/*
 *  Project   : tin - a Usenet reader
 *  Module    : page.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2004-01-11
 *  Notes     :
 *
 * Copyright (c) 1991-2004 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef RFC2046_H
#	include "rfc2046.h"
#endif /* !RFC2046_H */

#if defined(HAVE_IDNA_H) && !defined(_IDNA_H)
#	include <idna.h>
#endif /* HAVE_IDNA_H && !_IDNA_H */
#if defined(HAVE_STRINGPREP_H) && !defined(_STRINGPREP_H)
#	include <stringprep.h>
#endif /* HAVE_STRINGPREP_H & !_STRINGPREP_H */

/*
 * PAGE_HEADER is the size in lines of the article page header
 * ARTLINES is the number of lines available to display actual article text.
 */
#define PAGE_HEADER	4
#define ARTLINES	(NOTESLINES - (PAGE_HEADER - INDEX_TOP))

int curr_line;			/* current line in art (indexed from 0) */
static FILE *note_fp;			/* active stream (raw or cooked) */
static int artlines;			/* active # of lines in pager */
static t_lineinfo *artline;	/* active 'lineinfo' data */

t_openartinfo pgart =	/* Global context of article open in the pager */
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FALSE, 0 },
		FALSE, 0,
		NULL, NULL, NULL, NULL,
	};

int MORE_POS;			/* set in set_win_size() */

int last_resp;			/* previous & current article # in arts[] for '-' command */
int this_resp;

static struct t_header *note_h = &pgart.hdr;	/* Easy access to article headers */

static FILE *info_file;
static const char *info_title;
static int curr_info_line;
static int hide_uue;			/* set when uuencoded sections are 'hidden' */
static int num_info_lines;
static int reveal_ctrl_l_lines;	/* number of lines (from top) with de-activated ^L */
static int rotate;				/* 0=normal, 13=rot13 decode */
static int tabwidth = 8;
static int scroll_region_top;	/* first screen line for displayed message */
static int search_line;			/* Line to commence next search from */
static t_lineinfo *infoline = (t_lineinfo *) 0;

static t_bool show_all_headers;	/* CTRL-H with headers specified */
static t_bool reveal_ctrl_l;	/* set when ^L hiding is off */

/*
 * Local prototypes
 */
static int handle_pager_keypad(t_menukeys *menukeys);
static int load_article(int new_respnum, struct t_group *group);
static int prompt_response(int ch, int curr_respnum);
static int scroll_page(int dir);
static t_bool deactivate_next_ctrl_l(void);
static t_bool activate_last_ctrl_l(void);
static void draw_page_header(const char *group);
static void preprocess_info_message(FILE *info_fh);
static void print_message_page(FILE *file, t_lineinfo *messageline, size_t messagelines, size_t base_line, size_t begin, size_t end, int help_level);
static void process_search(int *lcurr_line, size_t message_lines, size_t screen_lines, int help_level);
static void process_url(void);
static void invoke_metamail(FILE *fp);

#ifdef XFACE_ABLE
#	define XFACE_SHOW()	if (tinrc.use_slrnface) \
								slrnface_show_xface()
#	define XFACE_CLEAR()	if (tinrc.use_slrnface) \
								slrnface_clear_xface()
#	define XFACE_SUPPRESS()	if (tinrc.use_slrnface) \
										slrnface_suppress_xface()
#else
#	define XFACE_SHOW()	/*nothing*/
#	define XFACE_CLEAR()	/*nothing*/
#	define XFACE_SUPPRESS()	/*nothing*/
#endif /* XFACE_ABLE */

/*
 * Scroll visible article part of display down (+ve) or up (-ve)
 * according to 'dir' (KEYMAP_UP or KEYMAP_DOWN) and tinrc.scroll_lines
 * >= 1  line count
 * 0     full page scroll
 * -1    full page but retain last line of prev page when scrolling
 *       down. Obviously only applies when scrolling down.
 * -2    half page scroll
 * Return the offset we scrolled by so that redrawing can be done
 */
static int
scroll_page(
	int dir)
{
	int i;

	if (tinrc.scroll_lines >= 1)
		i = tinrc.scroll_lines;
	else {
		i = (signal_context == cPage) ? ARTLINES : NOTESLINES;
		switch (tinrc.scroll_lines) {
			case 0:
				break;

			case -1:
				i--;
				break;

			case -2:
				i >>= 1;
				break;
		}
	}

	if (dir == KEYMAP_UP)
		i = -i;

#ifdef USE_CURSES
	scrollok(stdscr, TRUE);
#endif /* USE_CURSES */
	SetScrollRegion(scroll_region_top, NOTESLINES + 1);
	ScrollScreen(i);
	SetScrollRegion(0, cLINES);
#ifdef USE_CURSES
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */

	return i;
}


/*
 * Map keypad codes to standard keyboard characters
 */
static int
handle_pager_keypad(
	t_menukeys *menukeys)
{
	int ch = ReadCh();

	switch (ch) {
		case ESC:
#	ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
			switch (get_arrow_key(ch)) {
				case KEYMAP_UP:
					ch = iKeyUp;
					break;

				case KEYMAP_DOWN:
					ch = iKeyDown;
					break;

				case KEYMAP_LEFT:
					ch = iKeyQuit;
					break;

				case KEYMAP_RIGHT:
					ch = iKeyPageNextUnread;
					break;

				case KEYMAP_PAGE_UP:
					ch = iKeyPageUp;
					break;

				case KEYMAP_PAGE_DOWN:
					ch = iKeyPageDown;
					break;

				case KEYMAP_HOME:
					ch = iKeyFirstPage;
					break;

				case KEYMAP_END:
					ch = iKeyLastPage;
					break;

				case KEYMAP_MOUSE:
					switch (xmouse) {
						case MOUSE_BUTTON_1:
							if (xrow < PAGE_HEADER || xrow >= cLINES - 1)
								ch = iKeyPageDown;
							else
								ch = iKeyPageNextUnread;
							break;

						case MOUSE_BUTTON_2:
							if (xrow < PAGE_HEADER || xrow >= cLINES - 1)
								ch = iKeyPageUp;
							else
								ch = iKeyQuit;
							break;

						case MOUSE_BUTTON_3:
							ch = iKeyMouseToggle;
							break;

						default:
							break;
					}
					break;

				default:
					break;
			}
			break;

		default:
			ch = map_to_default(ch, menukeys);
			break;
	}
	return ch;
}


/*
 * Make hidden part of article after ^L visible.
 * Returns:
 *    FALSE no ^L found, no changes
 *    TRUE  ^L found and displayed page must be updated
 *          (draw_page must be called)
 */
static t_bool
deactivate_next_ctrl_l(
	void)
{
	int i;
	int end = curr_line + ARTLINES;

	if (reveal_ctrl_l)
		return FALSE;
	if (end > artlines)
		end = artlines;
	for (i = reveal_ctrl_l_lines + 1; i < end; i++)
		if (artline[i].flags & C_CTRLL) {
			reveal_ctrl_l_lines = i;
			return TRUE;
		}
	reveal_ctrl_l_lines = end - 1;
	return FALSE;
}


/*
 * Re-hide revealed part of article after last ^L
 * that is currently displayed.
 * Returns:
 *    FALSE no ^L found, no changes
 *    TRUE  ^L found and displayed page must be updated
 *          (draw_page must be called)
 */
static t_bool
activate_last_ctrl_l(
	void)
{
	int i;

	if (reveal_ctrl_l)
		return FALSE;
	for (i = reveal_ctrl_l_lines; i >= curr_line; i--)
		if (artline[i].flags & C_CTRLL) {
			reveal_ctrl_l_lines = i - 1;
			return TRUE;
		}
	reveal_ctrl_l_lines = curr_line - 1;
	return FALSE;
}


/*
 * The main routine for viewing articles
 * Returns:
 *    >=0	normal exit - return a new base[] note
 *    <0	indicates some unusual condition. See GRP_* in tin.h
 *			GRP_QUIT		User is doing a 'Q'
 *			GRP_RETSELECT	Back to selection level due to 'T' command
 *			GRP_ARTUNAVAIL	We didn't make it into the art
 *							don't bother fixing the screen up
 *			GRP_ARTABORT	User 'q'uit load of article
 *			GRP_GOTOTHREAD	To thread menu due to 'l' command
 *			GRP_NEXT		Catchup with 'c'
 *			GRP_NEXTUNREAD	   "      "  'C'
 */
int
show_page(
	struct t_group *group,
	int start_respnum,		/* index into arts[] */
	int *threadnum)			/* to allow movement in thread mode */
{
	char buf[LEN];
	char key[MAXKEYLEN];
	int ch, i, n = 0;
	int filter_state = NO_FILTERING;
	int old_sort_art_type = tinrc.sort_article_type;
	int art_type = GROUP_TYPE_NEWS;
	t_bool mouse_click_on = TRUE;
	t_bool repeat_search = FALSE;

	filtered_articles = FALSE;	/* used in thread level */

	if (group->attribute->mailing_list != NULL)
		art_type = GROUP_TYPE_MAIL;

	/*
	 * Peek to see if the pager started due to a body search
	 * Stop load_article() changing context again
	 */
	if (srch_lineno != -1)
		this_resp = start_respnum;

	if ((i = load_article(start_respnum, group)) < 0)
		return i;

	if (srch_lineno != -1)
		process_search(&curr_line, artlines, ARTLINES, PAGE_LEVEL);

	resize_article(TRUE, &pgart);

	forever {
		if ((ch = handle_pager_keypad(&menukeymap.page_nav)) == iKeySearchRepeat) {
			ch = i_key_search_last;
			repeat_search = TRUE;
		}
		else
			repeat_search = FALSE;

		switch (ch) {
			case iKeyAbort:       /* Abort */
				break;

			case '0': case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				if (!HAS_FOLLOWUPS(which_thread(this_resp)))
					info_message(_(txt_no_responses));
				else {
					if ((n = prompt_response(ch, this_resp)) != -1) {
						XFACE_CLEAR();
						if ((i = load_article(n, group)) < 0)
							return i;
					}
				}
				break;

#ifndef NO_SHELL_ESCAPE
			case iKeyShellEscape:
				XFACE_CLEAR();
				shell_escape();
				draw_page(group->name, 0);
				break;
#endif /* !NO_SHELL_ESCAPE */

			case iKeyMouseToggle:
				if (mouse_click_on)
					set_xclick_off();
				else
					set_xclick_on();
				mouse_click_on = bool_not(mouse_click_on);
				break;

			case iKeyPageUp:		/* page up */
			case iKeyPageUp2:
			case iKeyPageUp3:
				if (activate_last_ctrl_l())
					draw_page(group->name, 0);
				else {
					if (curr_line == 0)
						info_message(_(txt_begin_of_art));
					else {
						curr_line -= (tinrc.scroll_lines == -2) ? ARTLINES / 2 : ARTLINES;
						draw_page(group->name, 0);
					}
				}
				break;

			case iKeyPageDown:		/* page down or next response */
			case iKeyPageDown2:
			case iKeyPageDown3:
			case iKeyPageNextUnread:
				if (!((ch == iKeyPageNextUnread) && tinrc.tab_goto_next_unread) && deactivate_next_ctrl_l())
					draw_page(group->name, 0);
				else {
					if (curr_line + ARTLINES >= artlines) {	/* End is already on screen */
						switch (ch) {
							case iKeyPageNextUnread:	/* <TAB> */
								goto page_goto_next_unread;

							case iKeyPageDown:
							case iKeyPageDown2:
								if (tinrc.pgdn_goto_next)
									goto page_goto_next_unread;
								break;

							case iKeyPageDown3:			/* <SPACE> */
								if (tinrc.space_goto_next_unread)
									goto page_goto_next_unread;
								break;
						}
						info_message(_(txt_end_of_art));
					} else {
						if ((ch == iKeyPageNextUnread) && tinrc.tab_goto_next_unread)
							goto page_goto_next_unread;

						curr_line += (tinrc.scroll_lines == -2) ? ARTLINES / 2 : ARTLINES;

						if (tinrc.scroll_lines == -1)		/* formerly show_last_line_prev_page */
							curr_line--;
						draw_page(group->name, 0);
					}
				}
				break;

page_goto_next_unread:
				XFACE_CLEAR();
				if ((n = next_unread(next_response(this_resp))) == -1)
					return (which_thread(this_resp));
				if ((i = load_article(n, group)) < 0)
					return i;
				break;

			case iKeyFirstPage:		/* beginning of article */
			case iKeyPageFirstPage:
				if (reveal_ctrl_l_lines > -1 || curr_line != 0) {
					reveal_ctrl_l_lines = -1;
					curr_line = 0;
					draw_page(group->name, 0);
				}
				break;

			case iKeyLastPage:		/* end of article */
			case iKeyPageLastPage:
				if (reveal_ctrl_l_lines < artlines - 1 || curr_line + ARTLINES != artlines) {
					reveal_ctrl_l_lines = artlines - 1;
					/* Display a full last page for neatness */
					curr_line = artlines - ARTLINES;
					draw_page(group->name, 0);
				}
				break;

			case iKeyUp:		/* line up */
			case iKeyUp2:
				if (activate_last_ctrl_l())
					draw_page(group->name, 0);
				else {
					if (curr_line == 0) {
						info_message(_(txt_begin_of_art));
						break;
					}

					i = scroll_page(KEYMAP_UP);
					curr_line += i;
					draw_page(group->name, i);
				}
				break;

			case iKeyDown:		/* line down */
			case iKeyDown2:
				if (deactivate_next_ctrl_l())
					draw_page(group->name, 0);
				else {
					if (curr_line + ARTLINES >= artlines) {
						info_message(_(txt_end_of_art));
						break;
					}

					i = scroll_page(KEYMAP_DOWN);
					curr_line += i;
					draw_page(group->name, i);
				}
				break;

			case iKeyLastViewed:	/* show last viewed article */
				if (last_resp < 0 || (which_thread(last_resp) == -1)) {
					info_message(_(txt_no_last_message));
					break;
				}
				if ((i = load_article(last_resp, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}
				break;

			case iKeyLookupMessage:			/* Goto article by Message-ID */
				if ((n = prompt_msgid()) != ART_UNAVAILABLE) {
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
				}
				break;

			case iKeyPageGotoParent:		/* Goto parent of this article */
			{
				struct t_msgid *parent = arts[this_resp].refptr->parent;

				if (parent == NULL) {
					info_message(_(txt_art_parent_none));
					break;
				}

				if (parent->article == ART_UNAVAILABLE) {
					info_message(_(txt_art_parent_unavail));
					break;
				}

				if (arts[parent->article].killed && tinrc.kill_level == KILL_NOTHREAD) {
					info_message(_(txt_art_parent_killed));
					break;
				}

				if ((i = load_article(parent->article, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}

				break;
			}

			case iKeyPipe:		/* pipe article/thread/tagged arts to command */
				XFACE_SUPPRESS();
				feed_articles(FEED_PIPE, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

			case iKeyPageMail:	/* mail article/thread/tagged articles to somebody */
				XFACE_SUPPRESS();
				feed_articles(FEED_MAIL, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

#ifndef DISABLE_PRINTING
			case iKeyPrint:	/* output art/thread/tagged arts to printer */
				XFACE_SUPPRESS();
				feed_articles(FEED_PRINT, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;
#endif /* !DISABLE_PRINTING */

			case iKeyPageRepost:	/* repost current article */
				XFACE_SUPPRESS();
				feed_articles(FEED_REPOST, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

			case iKeyPageSave:	/* save article/thread/tagged articles */
				XFACE_SUPPRESS();
				feed_articles(FEED_SAVE, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

			case iKeyPageAutoSave:	/* Auto-save articles without prompting */
				if (grpmenu.curr >= 0) {
					XFACE_SUPPRESS();
					feed_articles(FEED_AUTOSAVE, PAGE_LEVEL, group, (int) base[grpmenu.curr]);
					XFACE_SHOW();
				}
				break;

			case iKeySearchSubjF:	/* search in article */
			case iKeySearchSubjB:
				if ((i = search_article((ch == iKeySearchSubjF), repeat_search, search_line, artlines, artline, reveal_ctrl_l_lines, note_fp)) == -1)
					break;

				if (ch == iKeySearchSubjB && !reveal_ctrl_l) {
					reveal_ctrl_l_lines = curr_line + ARTLINES - 1;
					draw_page(group->name, 0);
				}
				process_search(&curr_line, artlines, ARTLINES, PAGE_LEVEL);
				break;

			case iKeySearchBody:	/* article body search */
				if ((n = search_body(group, this_resp, repeat_search)) != -1) {
					this_resp = n;			/* Stop load_article() changing context again */
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
					process_search(&curr_line, artlines, ARTLINES, PAGE_LEVEL);
				}
				break;

			case iKeyPageTopThd:	/* first article in current thread */
				if (arts[this_resp].prev >= 0) {
					if ((n = which_thread(this_resp)) >= 0 && base[n] != this_resp) {
						assert(n < grpmenu.max);
						if ((i = load_article(base[n], group)) < 0) {
							XFACE_CLEAR();
							return i;
						}
					}
				}
				break;

			case iKeyPageBotThd:	/* last article in current thread */
				for (i = this_resp; i >= 0; i = arts[i].thread)
					n = i;

				if (n != this_resp) {
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
				}
				break;

			case iKeyPageNextThd:
			case iKeyPageNextThd2:	/* start of next thread */
				XFACE_CLEAR();
				if ((n = next_thread(this_resp)) == -1)
					return (which_thread(this_resp));
				if ((i = load_article(n, group)) < 0)
					return i;
				break;

#ifdef HAVE_PGP_GPG
			case iKeyPagePGPCheckArticle:
				XFACE_SUPPRESS();
				if (pgp_check_article(&pgart))
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;
#endif /* HAVE_PGP_GPG */

			case iKeyPageToggleHeaders:	/* toggle display of whole 'raw' article */
				XFACE_CLEAR();
				toggle_raw(group);
				break;

			case iKeyPageToggleTex2iso:		/* toggle german TeX to ISO latin1 style conversion */
				if (((group->attribute->tex2iso_conv) = !(group->attribute->tex2iso_conv)))
					pgart.tex2iso = is_art_tex_encoded(pgart.raw);
				else
					pgart.tex2iso = FALSE;

				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(group->name, 0);
				info_message(_(txt_toggled_tex2iso), txt_onoff[group->attribute->tex2iso_conv != FALSE ? 1 : 0]);
				break;

			case iKeyPageToggleTabs:		/* toggle tab stops 8 vs 4 */
				tabwidth = (tabwidth == 8) ? 4 : 8;
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(group->name, 0);
				info_message(_(txt_toggled_tabwidth), tabwidth);
				break;

			case iKeyPageToggleUue:			/* toggle display of uuencoded sections */
				hide_uue = (hide_uue + 1) % (UUE_ALL + 1);
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				/*
				 * If we hid uue and are off the end of the article, reposition to
				 * show last page for neatness
				 */
				if (hide_uue && curr_line + ARTLINES > artlines)
					curr_line = artlines - ARTLINES;
				draw_page(group->name, 0);
				/* TODO: info_message()? */
				break;

			case iKeyPageReveal:			/* toggle hiding after ^L */
				reveal_ctrl_l = bool_not(reveal_ctrl_l);
				if (!reveal_ctrl_l) {	/* switched back to active ^L's */
					reveal_ctrl_l_lines = -1;
					curr_line = 0;
				} else
					reveal_ctrl_l_lines = artlines - 1;
				draw_page(group->name, 0);
				/* TODO: info_message()? */
				break;

			case iKeyPageQuickAutoSel:	/* quickly auto-select article */
			case iKeyPageQuickKill:		/* quickly kill article */
				if ((filtered_articles = quick_filter((ch == iKeyPageQuickKill) ? FILTER_KILL : FILTER_SELECT, group, &arts[this_resp])))
					goto return_to_index;

				draw_page(group->name, 0);
				break;

			case iKeyPageAutoSel:		/* auto-select article menu */
			case iKeyPageAutoKill:		/* kill article menu */
				XFACE_CLEAR();
				if (filter_menu((ch == iKeyPageAutoKill) ? FILTER_KILL : FILTER_SELECT, group, &arts[this_resp])) {
					if ((filtered_articles = filter_articles(group)))
						goto return_to_index;
				}
				draw_page(group->name, 0);
				break;

			case iKeyPageEditFilter:
				XFACE_CLEAR();
				if (!invoke_editor(filter_file, FILTER_FILE_OFFSET))
					break;
				unfilter_articles();
				(void) read_filter_file(filter_file);
				if ((filtered_articles = filter_articles(group)))
					goto return_to_index;
				draw_page(group->name, 0);
				break;

			case iKeyRedrawScr:		/* redraw current page of article */
				my_retouch();
				draw_page(group->name, 0);
				break;

			case iKeyPageToggleRot:	/* toggle rot-13 mode */
				rotate = rotate ? 0 : 13;
				draw_page(group->name, 0);
				info_message(_(txt_toggled_rot13));
				break;

			case iKeySearchAuthF:	/* author search forward */
			case iKeySearchAuthB:	/* author search backward */
				if ((n = search(SEARCH_AUTH, this_resp, (ch == iKeySearchAuthF), repeat_search)) < 0)
					break;
				if ((i = load_article(n, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}
				break;

			case iKeyPageCatchup:			/* catchup - mark read, goto next */
			case iKeyPageCatchupNextUnread:	/* goto next unread */
				snprintf(buf, sizeof(buf), _(txt_mark_thread_read), (ch == iKeyPageCatchupNextUnread) ? _(txt_enter_next_thread) : "");
				if ((!TINRC_CONFIRM_ACTION) || prompt_yn(cLINES, buf, TRUE) == 1) {
					thd_mark_read(group, base[which_thread(this_resp)]);
					XFACE_CLEAR();
					return (ch == iKeyPageCatchupNextUnread) ? GRP_NEXTUNREAD : GRP_NEXT;
				}
				break;

			case iKeyPageMarkThdUnread:
				thd_mark_unread(group, base[which_thread(this_resp)]);
				/*
				 * FIXME: replace txt_thread by txt_article_upper
				 * if THREAD_NONE
				 */
				info_message(_(txt_marked_as_unread), _(txt_thread_upper));
				break;

			case iKeyPageCancel:			/* cancel an article */
				if (can_post || art_type != GROUP_TYPE_NEWS) {
					XFACE_SUPPRESS();
					if (cancel_article(group, &arts[this_resp], this_resp))
						draw_page(group->name, 0);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case iKeyPageEditArticle:		/* edit an article (mailgroup only) */
				XFACE_SUPPRESS();
				if (art_edit(group, &arts[this_resp]))
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case iKeyPageFollowupQuote:		/* post a followup to this article */
			case iKeyPageFollowupQuoteHeaders:
			case iKeyPageFollowup:
				if (!can_post && art_type == GROUP_TYPE_NEWS) {
					info_message(_(txt_cannot_post));
					break;
				}
				XFACE_CLEAR();
				(void) post_response(group->name, this_resp,
				  (ch == iKeyPageFollowupQuote || ch == iKeyPageFollowupQuoteHeaders) ? TRUE : FALSE,
				  ch == iKeyPageFollowupQuoteHeaders ? TRUE : FALSE, show_all_headers);
				draw_page(group->name, 0);
				break;

			case iKeyHelp:	/* help */
				XFACE_CLEAR();
				show_help_page(PAGE_LEVEL, _(txt_art_pager_com));
				draw_page(group->name, 0);
				break;

			case iKeyToggleHelpDisplay:	/* toggle mini help menu */
				toggle_mini_help(PAGE_LEVEL);
				draw_page(group->name, 0);
				break;

			case iKeyQuit:	/* return to index page */
return_to_index:
				XFACE_CLEAR();
				if (filter_state == NO_FILTERING && tinrc.sort_article_type != old_sort_art_type)
					make_threads(group, TRUE);

				i = which_thread(this_resp);
				if (threadnum)
					*threadnum = which_response(this_resp);

				if (filter_state == FILTERING || filtered_articles) {
					int old_top = top_art;
					long old_artnum = arts[this_resp].artnum;

					filter_articles(group);
					make_threads(group, FALSE);
					i = find_new_pos(old_top, old_artnum, i);
				}
				return i;

			case iKeyToggleInverseVideo:	/* toggle inverse video */
				toggle_inverse_video();
				draw_page(group->name, 0);
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case iKeyToggleColor:		/* toggle color */
				if (toggle_color()) {
					draw_page(group->name, 0);
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case iKeyPageListThd:	/* -> thread page that this article is in */
				XFACE_CLEAR();
				fixup_thread(this_resp, FALSE);
				return GRP_GOTOTHREAD;

			case iKeyOptionMenu:	/* option menu */
				XFACE_CLEAR();
				if (change_config_file(group) == FILTERING)
					filter_state = FILTERING;
				set_subj_from_size(cCOLS);
				draw_page(group->name, 0);
				break;

			case iKeyPageNextArt:	/* skip to next article */
				XFACE_CLEAR();
				if ((n = next_response(this_resp)) == -1)
					return (which_thread(this_resp));

				if ((i = load_article(n, group)) < 0)
					return i;
				break;

			case iKeyPageKillThd:	/* mark rest of thread as read */
				thd_mark_read(group, this_resp);
				if ((n = next_unread(next_response(this_resp))) == -1)
					goto return_to_index;
				if ((i = load_article(n, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}
				break;

			case iKeyPageNextUnreadArt:	/* next unread article */
				goto page_goto_next_unread;

			case iKeyPagePrevArt:	/* previous article */
				XFACE_CLEAR();
				if ((n = prev_response(this_resp)) == -1)
					return this_resp;

				if ((i = load_article(n, group)) < 0)
					return i;
				break;

			case iKeyPagePrevUnreadArt:	/* previous unread article */
				if ((n = prev_unread(prev_response(this_resp))) == -1)
					info_message(_(txt_no_prev_unread_art));
				else {
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
				}
				break;

			case iKeyQuitTin:	/* quit */
				XFACE_CLEAR();
				return GRP_QUIT;

			case iKeyPageReplyQuote:	/* reply to author through mail */
			case iKeyPageReplyQuoteHeaders:
			case iKeyPageReply:
				XFACE_CLEAR();
				mail_to_author(group->name, this_resp, (ch == iKeyPageReplyQuote || ch == iKeyPageReplyQuoteHeaders) ? TRUE : FALSE, ch == iKeyPageReplyQuoteHeaders ? TRUE : FALSE, show_all_headers);
				draw_page(group->name, 0);
				break;

			case iKeyPageTag:	/* tag/untag article for saving */
				tag_article(this_resp);
				break;

			case iKeyPageGroupSel:	/* return to group selection page */
				if (filter_state == FILTERING) {
					filter_articles(group);
					make_threads(group, FALSE);
				}
				XFACE_CLEAR();
				return GRP_RETSELECT;

			case iKeyVersion:
				info_message(cvers);
				break;

			case iKeyPost:	/* post a basenote */
				XFACE_SUPPRESS();
				if (post_article(group->name))
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case iKeyPostponed:
			case iKeyPostponed2:	/* post postponed article */
				if (can_post || art_type != GROUP_TYPE_NEWS) {
					XFACE_SUPPRESS();
					if (pickup_postponed_articles(FALSE, FALSE))
						draw_page(group->name, 0);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case iKeyDisplayPostHist:	/* display messages posted by user */
				XFACE_SUPPRESS();
				if (user_posted_messages())
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case iKeyPageMarkArtUnread:	/* mark article as unread(to return) */
				art_mark(group, &arts[this_resp], ART_WILL_RETURN);
				info_message(_(txt_marked_as_unread), _(txt_article_upper));
				break;

			case iKeyPageSkipIncludedText:	/* skip included text */
				for (i = curr_line; i < artlines; i++) {
					if (!(artline[i].flags & (C_QUOTE1 | C_QUOTE2 | C_QUOTE3)))
						break;
				}

				if (i != curr_line) {
					curr_line = i;
					draw_page(group->name, 0);
				}
				break;

			case iKeyToggleInfoLastLine: /* this is _not_ correct, we do not toggle status here */
				info_message("%s", arts[this_resp].subject);
				break;

			case iKeyPageToggleHighlight:
				word_highlight = bool_not(word_highlight);
				draw_page(group->name, 0);
				info_message(_(txt_toggled_high), txt_onoff[word_highlight != FALSE ? 1 : 0]);
				break;

			case iKeyPageViewAttach:
				XFACE_SUPPRESS();
				decode_save_mime(&pgart, FALSE);
				draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case iKeyPageViewUrl:
				if (!show_all_headers) { /* cooked mode? */
					XFACE_SUPPRESS();
					resize_article(FALSE, &pgart); /* umbreak long lines */
					process_url();
					resize_article(TRUE, &pgart); /* rebreak long lines */
					draw_page(group->name, 0);
					XFACE_SHOW();
				}
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, map_to_local(iKeyHelp, &menukeymap.page_nav)));
		}
	}
	/* NOTREACHED */
	return GRP_ARTUNAVAIL;
}


static void
print_message_page(
	FILE *file,
	t_lineinfo *messageline,
	size_t messagelines,
	size_t base_line,
	size_t begin,
	size_t end,
	int help_level)
{
	char *line;
	char *p;
	int bytes;
	size_t i = begin;
	t_lineinfo *curr;

	for (; i < end; i++) {
		if (base_line + i >= messagelines)		/* ran out of message */
			break;

		curr = &messageline[base_line + i];
		fseek(file, curr->offset, SEEK_SET);

		if ((line = tin_fgets(file, FALSE)) == NULL)
			break;	/* ran out of message */

		/*
		 * use the offsets gained while doing line wrapping to
		 * determine the correct position to truncate the line
		 */
		if (base_line + i < messagelines - 1) { 	/* not last line of message*/
			bytes = (curr + 1)->offset - curr->offset;
			line[bytes] = '\0';
		}

		/*
		 * rotN encoding on body and sig data only
		 */
		if ((rotate != 0) && (curr->flags & (C_BODY | C_SIG))) {
			p = line;
			for (p = line; *p; p++) {
				if (*p >= 'A' && *p <= 'Z')
					*p = (*p - 'A' + rotate) % 26 + 'A';
				else if (*p >= 'a' && *p <= 'z')
					*p = (*p - 'a' + rotate) % 26 + 'a';
			}
		}

		strip_line(line);

#ifndef USE_CURSES
		snprintf(screen[i + scroll_region_top].col, cCOLS, "%s" cCRLF, line);
#endif /* !USE_CURSES */

		MoveCursor(i + scroll_region_top, 0);
		draw_pager_line(line, curr->flags, show_all_headers);

		/*
		 * Highlight URL's and mail addresses
		 */
		if (tinrc.url_highlight) {
			if (curr->flags & C_URL)
#ifdef HAVE_COLOR
				highlight_regexes(i + scroll_region_top, &url_regex, use_color ? tinrc.col_urls : -1);
#else
				highlight_regexes(i + scroll_region_top, &url_regex, -1);
#endif /* HAVE_COLOR */

			if (curr->flags & C_MAIL)
#ifdef HAVE_COLOR
				highlight_regexes(i + scroll_region_top, &mail_regex, use_color ? tinrc.col_urls : -1);
#else
				highlight_regexes(i + scroll_region_top, &mail_regex, -1);
#endif /* HAVE_COLOR */

			if (curr->flags & C_NEWS)
#ifdef HAVE_COLOR
				highlight_regexes(i + scroll_region_top, &news_regex, use_color ? tinrc.col_urls : -1);
#else
				highlight_regexes(i + scroll_region_top, &news_regex, -1);
#endif /* HAVE_COLOR */
		}

		/*
		 * Highlight /slashes/, *stars*, _underscores_ and -strokes-
		 */
		if (word_highlight && (curr->flags & C_BODY) && !(curr->flags & C_CTRLL)) {
#ifdef HAVE_COLOR
			highlight_regexes(i + scroll_region_top, &slashes_regex, use_color ? tinrc.col_markslash : tinrc.mono_markslash);
			highlight_regexes(i + scroll_region_top, &stars_regex, use_color ? tinrc.col_markstar : tinrc.mono_markstar);
			highlight_regexes(i + scroll_region_top, &underscores_regex, use_color ? tinrc.col_markdash : tinrc.mono_markdash);
			highlight_regexes(i + scroll_region_top, &strokes_regex, use_color ? tinrc.col_markstroke : tinrc.mono_markstroke);
#else
			highlight_regexes(i + scroll_region_top, &slashes_regex, tinrc.mono_markslash);
			highlight_regexes(i + scroll_region_top, &stars_regex, tinrc.mono_markstar);
			highlight_regexes(i + scroll_region_top, &underscores_regex, tinrc.mono_markdash);
			highlight_regexes(i + scroll_region_top, &strokes_regex, tinrc.mono_markstroke);
#endif /* HAVE_COLOR */
		}

		/* Blank the screen after a ^L (only occurs when showing cooked) */
		if (!reveal_ctrl_l && (curr->flags & C_CTRLL) && (int) (base_line + i) > reveal_ctrl_l_lines) {
			CleartoEOS();
			break;
		}
	}

#ifdef HAVE_COLOR
	fcol(tinrc.col_text);
#endif /* HAVE_COLOR */

	show_mini_help(help_level);
}


/*
 * Redraw the current page, curr_line will be the first line displayed
 * Everything that calls draw_page() just sets curr_line, this function must
 * ensure it is set to something sane
 * If part is !=0, then only draw the first (-ve) or last (+ve) few lines
 */
void
draw_page(
	const char *group,
	int part)
{
	int start, end;	/* 1st, last line to draw */

	signal_context = cPage;

	/*
	 * Can't do partial draw if term can't scroll properly
	 */
	if (part && !have_linescroll)
		part = 0;

	/*
	 * Ensure curr_line is in bounds
	 */
	if (curr_line < 0)
		curr_line = 0;			/* Oops - off the top */
	else {
		if (curr_line > artlines)
			curr_line = artlines;	/* Oops - off the end */
	}

	search_line = curr_line;	/* Reset search to start from top of display */

	scroll_region_top = PAGE_HEADER;

	/* Down-scroll, only redraw bottom 'part' lines of screen */
	if ((start = (part > 0) ? ARTLINES - part : 0) < 0)
		start = 0;

	/* Up-scroll, only redraw the top 'part' lines of screen */
	if ((end = (part < 0) ? -part : ARTLINES) > ARTLINES)
		end = ARTLINES;

	/*
	 * ncurses doesn't clear the scroll area when you scroll by more than the
	 * window size - force full redraw
	 */
	if ((end-start >= ARTLINES) || (part == 0)) {
		ClearScreen();
		draw_page_header(group);
	} else
		MoveCursor(0, 0);

	print_message_page(note_fp, artline, artlines, curr_line, start, end, PAGE_LEVEL);

	/*
	 * Print an appropriate footer
	 */
	if (curr_line + ARTLINES >= artlines) {
		char buf[LEN];
		int len;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		wchar_t wbuf[LEN];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		STRCPY(buf, (arts[this_resp].thread != -1) ? _(txt_next_resp) : _(txt_last_resp));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (mbstowcs(wbuf, buf, ARRAY_SIZE(wbuf)) != (size_t) (-1)) {
			wbuf[ARRAY_SIZE(wbuf) - 1] = (wchar_t) '\0';
			wconvert_to_printable(wbuf);
			len = wcswidth(wbuf, ARRAY_SIZE(wbuf));
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			len = (int) strlen(buf);

		clear_message();
		MoveCursor(cLINES, cCOLS - len - (1 + BLANK_PAGE_COLS));
		StartInverse();
		my_fputs(buf, stdout);
		my_flush();
		EndInverse();
	} else
		draw_percent_mark(curr_line + ARTLINES, artlines);

#ifdef XFACE_ABLE
	if (tinrc.use_slrnface && !show_all_headers)
		slrnface_display_xface(note_h->xface);
#endif /* XFACE_ABLE */

	stow_cursor();
}


/*
 * Start external metamail program
 */
static void
invoke_metamail(
	FILE *fp)
{
	char *ptr;
	long offset;
#ifndef DONT_HAVE_PIPING
	FILE *mime_fp;
	char buf[LEN];
#endif /* !DONT_HAVE_PIPING */

	ptr = tinrc.metamail_prog;
	if (('\0' == *ptr) || (0 == strcmp(ptr, INTERNAL_CMD)) || (NULL != getenv("NOMETAMAIL")))
		return;

	offset = ftell(fp);
	rewind(fp);

	EndWin();
	Raw(FALSE);

	/* TODO: add DONT_HAVE_PIPING fallback code */
#ifndef DONT_HAVE_PIPING
	if ((mime_fp = popen(ptr, "w"))) {
		while (fgets(buf, (int) sizeof(buf), fp) != NULL)
			fputs(buf, mime_fp);

		fflush(mime_fp);
		pclose(mime_fp);
	} else
#endif /* !DONT_HAVE_PIPING */
		perror_message(_(txt_command_failed), ptr);

	Raw(TRUE);
	InitWin();
	prompt_continue();

	/* This is needed if we are viewing the raw art */
	fseek(fp, offset, SEEK_SET);	/* goto old position */

	MoveCursor(cLINES, MORE_POS - (5 + BLANK_PAGE_COLS));
	StartInverse();
	my_flush();
	EndInverse();
}


/*
 * PAGE_HEADER defines the size in lines of this header
 */
static void
draw_page_header(
	const char *group)
{
	char *buf;
	int i;
	int whichresp, x_resp;
	int right_len, center_pos;
	size_t len, line_len;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *fmt_resp, *fmt_thread, *line, *wbuf, *wtmp;
#else
	char *line, *tmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	whichresp = which_response(this_resp);
	x_resp = num_of_responses(which_thread(this_resp));

	line_len = LEN + 1;
	buf = my_malloc(line_len);

	if (!my_strftime(buf, line_len, tinrc.date_format, localtime(&arts[this_resp].date))) {
		strncpy(buf, BlankIfNull(note_h->date), line_len);
		buf[line_len - 1] = '\0';
	}

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	line = my_malloc(line_len * sizeof(wchar_t));
	wbuf = my_malloc(line_len * sizeof(wchar_t));
	wtmp = my_malloc(line_len * sizeof(wchar_t));

	/* convert to wide-char format strings */
	len = mbstowcs(NULL, _(txt_thread_x_of_n), 0) + 1;
	if (len > 0) {
		fmt_thread = my_malloc(len * sizeof(wchar_t));
		mbstowcs(fmt_thread, _(txt_thread_x_of_n), len);
	} else
		fmt_thread = NULL;

	len = mbstowcs(NULL, _(txt_resp_x_of_n), 0) + 1;
	if (len > 0) {
		fmt_resp = my_malloc(len * sizeof(wchar_t));
		mbstowcs(fmt_resp, _(txt_resp_x_of_n), len);
	} else
		fmt_resp = NULL;

	/*
	 * determine the needed space for the text at the right hand margin
	 * the formating info (%4s) needs 3 positions but we need 4 positions
	 * on the screen for each counter.
	 */
	if (fmt_thread && fmt_resp)
		right_len = MAX((wcswidth(fmt_thread, wcslen(fmt_thread)) - 6 + 8), (wcswidth(fmt_resp, wcslen(fmt_resp)) - 6 + 8));
	else if (fmt_thread)
		right_len = wcswidth(fmt_thread, wcslen(fmt_thread)) - 6 + 8;
	else if (fmt_resp)
		right_len = wcswidth(fmt_resp, wcslen(fmt_resp)) - 6 + 8;
	else
		right_len = 0;

	/*
	 * first line
	 */
	/* date */
	if (mbstowcs(wbuf, buf, line_len) == (size_t) (-1))
		wbuf[0] = (wchar_t) '\0'; /* conversion failed */
	else
		wbuf[line_len - 1] = (wchar_t) '\0';

	wcsncpy(line, wbuf, line_len);
	line[line_len - 1] = (wchar_t) '\0';

	/*
	 * determine max len for centered group name
	 * allow one space before and after group name
	 */
	len = cCOLS - 2 * MAX(wcswidth(wbuf, line_len), right_len) - 3;

	/* group name */
	if (mbstowcs(wbuf, group, line_len) == (size_t) (-1))
		wbuf[0] = (wchar_t) '\0'; /* conversion failed */
	else
		wbuf[line_len - 1] = (wchar_t) '\0';

	wconvert_to_printable(wbuf);
	if ((i = wcswidth(wbuf, line_len)) < (int) len)
		len = i;

	center_pos = (cCOLS - len) / 2;

	/* pad out to left */
	wtmp[0] = (wchar_t) ' ';
	wtmp[1] = (wchar_t) '\0';
	while (wcswidth(line, line_len) < center_pos && wcslen(line) < line_len - 1)
		wcscat(line, wtmp);

	wcsncat(line, wstrunc(wbuf, wtmp, line_len, len), line_len - wcslen(line) - 1);

	/* pad out to right */
	wtmp[0] = (wchar_t) ' ';
	wtmp[1] = (wchar_t) '\0';
	while (wcswidth(line, line_len) < cCOLS - right_len - 1 && wcslen(line) < line_len - 1)
		wcscat(line, wtmp);

	/* thread info */
	/* can't eval tin_ltoa() more than once in a statement due to statics */
	if (fmt_thread) {
		strcpy(buf, tin_ltoa(which_thread(this_resp) + 1, 4));
		swprintf(wbuf, line_len, fmt_thread, buf, tin_ltoa(grpmenu.max, 4));
	} else
		wbuf[0] = (wchar_t) '\0';

	wcsncat(line, wbuf, line_len - wcslen(line) - 1);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	my_fputws(line, stdout);
	my_fputs(cCRLF, stdout);

#if 0
	/* display a ruler for layout checking purposes */
	my_fputs("....|....3....|....2....|....1....|....0....|....1....|....2....|....3....|....\n", stdout);
#endif /* 0 */

	/*
	 * second line
	 */
	/* line count */
	if (arts[this_resp].line_count < 0)
		strcpy(buf, "?");
	else
		snprintf(buf, line_len, "%-4d", arts[this_resp].line_count);

	{
		wchar_t *fmt;

		len = mbstowcs(NULL, _(txt_lines), 0) + 1;
		fmt = my_malloc(len * sizeof(wchar_t));
		if (mbstowcs(fmt, _(txt_lines), len) == (size_t) (-1))
			wbuf[0] = (wchar_t) '\0';
		else {
			fmt[len - 1] = (wchar_t) '\0';
			swprintf(wbuf, line_len, fmt, buf);
		}

		free(fmt);
	}

	len = wcswidth(wbuf, line_len);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	my_fputws(wbuf, stdout);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_subject);
#	endif /* HAVE_COLOR */

	/* tex2iso */
	if (pgart.tex2iso) {
		mbstowcs(wtmp, "TeX ", line_len);
		len += wcswidth(wtmp, line_len);
		my_fputws(wtmp, stdout);
	}

	/* subject */
	/*
	 * TODO: why do we fall back to arts[this_resp].subject if !note_h->subj?
	 *       if !note_h->subj then the article just has no subject, no matter
	 *       what the overview says.
	 */
	strncpy(buf, (note_h->subj ? note_h->subj : arts[this_resp].subject), line_len);
	buf[line_len - 1] = '\0';
	if (IS_LOCAL_CHARSET("UTF-8"))
		utf8_valid(buf);
	if (mbstowcs(wtmp, buf, line_len) == (size_t) (-1))
		wtmp[0] = (wchar_t) '\0';	/* conversion failed */
	else
		wtmp[line_len - 1] = (wchar_t) '\0';
	wconvert_to_printable(wtmp);
	if (wcswidth(wtmp, line_len) > cCOLS - 2 * right_len - 3)
		wstrunc(wtmp, wbuf, line_len, cCOLS - 2 * right_len - 3);
	else
		wcscpy(wbuf, wtmp);

	center_pos = (cCOLS - wcswidth(wbuf, line_len)) / 2;

	/* pad out to left */
	line[0] = (wchar_t) '\0';
	wtmp[0] = (wchar_t) ' ';
	wtmp[1] = (wchar_t) '\0';
	while (wcswidth(line, line_len) < center_pos - (int) len && wcslen(line) < line_len - 1)
		wcscat(line, wtmp);

	len += wcswidth(line, line_len);
	my_fputws(line, stdout);

	StartInverse();
	my_fputws(wbuf, stdout);
	EndInverse();
	len += wcswidth(wbuf, line_len);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_response);
#	endif /* HAVE_COLOR */

	/* pad out to right */
	line[0] = (wchar_t) '\0';
	wtmp[0] = (wchar_t) ' ';
	wtmp[1] = (wchar_t) '\0';
	while (wcswidth(line, line_len) < cCOLS - right_len - (int) len - 1 && wcslen(line) < line_len - 1)
		wcscat(line, wtmp);

	my_fputws(line, stdout);

	if (whichresp) {
		if (fmt_resp) {
			swprintf(line, line_len, fmt_resp, whichresp, x_resp);
			my_fputws(line, stdout);
		}
	} else {
		if (!x_resp)
			my_printf(_(txt_no_responses));
		else if (x_resp == 1)
			my_printf(_(txt_1_resp));
		else
			my_printf(_(txt_x_resp), x_resp);
	}
	my_fputs(cCRLF, stdout);

	/*
	 * third line
	 */
	/* from */
	/*
	 * TODO: don't use arts[this_resp].name/arts[this_resp].from
	 *       split up note_h->from and use that instead as it might
	 *       be different _if_ the overviews are broken
	 */
	{
		char *p = idna_decode(arts[this_resp].from);

		if (arts[this_resp].name)
			snprintf(buf, line_len, "%s <%s>", arts[this_resp].name, p);
		else {
			strncpy(buf, p, line_len);
			buf[line_len - 1] = '\0';
		}
		free(p);
	}

	if (IS_LOCAL_CHARSET("UTF-8"))
		utf8_valid(buf);

	if (mbstowcs(wbuf, buf, line_len) == (size_t) (-1))
		line[0] = (wchar_t) '\0';
	wstrunc(wbuf, line, line_len, cCOLS - 1);

	/* organization */
	if (note_h->org) {
		snprintf(buf, line_len, _(txt_at_s), note_h->org);

		if (mbstowcs(wbuf, buf, line_len) == (size_t) (-1))
			wbuf[0] = (wchar_t) '\0';
		wconvert_to_printable(wbuf);

		if (wcswidth(line, line_len) + wcswidth(wbuf, line_len) >= cCOLS - 1) {
			wcsncat(line, wbuf, line_len - wcslen(line) - 1);
			wcscpy(wbuf, line);
			wstrunc(wbuf, line, line_len, cCOLS - 1);
		} else {
			i = cCOLS - 1 - wcswidth(wbuf, line_len);

			wtmp[0] = (wchar_t) ' ';
			wtmp[1] = (wchar_t) '\0';
			while (wcswidth(line, line_len) < i && wcslen(line) < line_len - 1)
				wcscat(line, wtmp);
			wcsncat(line, wbuf, line_len - wcslen(line) - 1);
		}
	}

#	ifdef HAVE_COLOR
	fcol(tinrc.col_from);
#	endif /* HAVE_COLOR */

	my_fputws(line, stdout);
	my_fputs(cCRLF, stdout);
	my_fputs(cCRLF, stdout);

	/* clean up */
	free(line);
	free(wbuf);
	free(wtmp);
	FreeIfNeeded(fmt_thread);
	FreeIfNeeded(fmt_resp);

#else /* !MULTIBYTE_ABLE || NO_LOCALE */

	line = my_malloc(line_len);
	tmp = my_malloc(line_len);

	/*
	 * determine the needed space for the text at the right hand margin
	 * the formating info (%4s) needs 3 positions but we need 4 positions
	 * on the screen for each counter
	 */
	right_len = MAX((strlen(_(txt_thread_x_of_n)) - 6 + 8), (strlen(_(txt_resp_x_of_n)) - 6 + 8));

	/*
	 * first line
	 */
	/* date */
	strncpy(line, buf, line_len);
	line[line_len - 1] = '\0';

	/*
	 * determine max len for centered group name
	 * allow one space before and after group name
	 */
	len = cCOLS - 2 * MAX((int) strlen(buf), right_len) - 3;

	/* group name */
	if ((i = strlen(group)) < (int) len)
		len = i;

	center_pos = (cCOLS - len) / 2;

	/* pad out to left */
	for (i = strlen(line); i < center_pos && i < (int) line_len - 1; i++)
		line[i] = ' ';
	line[i] = '\0';

	strunc(group, line + i, line_len - i, len);

	/* pad out to right */
	for (i = strlen(line); i < cCOLS - right_len - 1 && i < (int) line_len - 1; i++)
		line[i] = ' ';
	line[i] = '\0';

	/* thread info */
	/* can't eval tin_ltoa() more than once in a statement due to statics */
	strcpy(tmp, tin_ltoa(which_thread(this_resp) + 1, 4));
	snprintf(buf, line_len, _(txt_thread_x_of_n), tmp, tin_ltoa(grpmenu.max, 4));

	strncat(line, buf, line_len - strlen(line) - 1);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	my_fputs(line, stdout);
	my_fputs(cCRLF, stdout);

#if 0
	/* display a ruler for layout checking purposes */
	my_fputs("....|....3....|....2....|....1....|....0....|....1....|....2....|....3....|....\n", stdout);
#endif /* 0 */

	/*
	 * second line
	 */
	/* line count */
	/* an accurate line count will appear in the footer anymay */
	if (arts[this_resp].line_count < 0)
		strcpy(buf, "?");
	else
		snprintf(buf, line_len, "%-4d", arts[this_resp].line_count);

	snprintf(line, line_len, _(txt_lines), buf);

	len = strlen(line);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	my_fputs(line, stdout);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_subject);
#	endif /* HAVE_COLOR */

	/* tex2iso */
	if (pgart.tex2iso) {
		strcpy(buf, "TeX ");
		len += strlen(buf);
		my_fputs(buf, stdout);
	}

	/* subject */
	/*
	 * TODO: why do we fall back to arts[this_resp].subject if !note_h->subj?
	 *       if !note_h->subj then the article just has no subject, no matter
	 *       what the overview says.
	 */
	strncpy(buf, (note_h->subj ? note_h->subj : arts[this_resp].subject), line_len);
	buf[line_len - 1] = '\0';

	convert_to_printable(buf);
	if ((int) strlen(buf) > cCOLS - 2 * right_len - 3)
		strunc(buf, line, line_len, cCOLS - 2 * right_len - 3);
	else {
		strncpy(line, buf, line_len);
		buf[line_len - 1] = '\0';
	}

	center_pos = (cCOLS - strlen(line)) / 2;

	/* pad out to left */
	tmp[0] = '\0';
	for (i = 0; i < center_pos - (int) len && i < (int) line_len - 1; i++)
		tmp[i] = ' ';
	tmp[i] = '\0';
	len += strlen(tmp);
	my_fputs(tmp, stdout);

	StartInverse();
	my_fputs(line, stdout);
	EndInverse();
	len += strlen(line);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_response);
#	endif /* HAVE_COLOR */

	/* pad out to right */
	tmp[0] = '\0';
	for (i = 0; i < cCOLS - right_len - (int) len - 1 && i < (int) line_len - 1; i++)
		tmp[i] = ' ';
	tmp[i] = '\0';

	my_fputs(tmp, stdout);

	if (whichresp)
		my_printf(_(txt_resp_x_of_n), whichresp, x_resp);
	else {
		if (!x_resp)
			my_printf(_(txt_no_responses));
		else if (x_resp == 1)
			my_printf(_(txt_1_resp));
		else
			my_printf(_(txt_x_resp), x_resp);
	}
	my_fputs(cCRLF, stdout);

	/*
	 * third line
	 */
	/* from */
	/*
	 * TODO: don't use arts[this_resp].name/arts[this_resp].from
	 *       split up note_h->from and use that instead as it might
	 *       be different _if_ the overviews are broken
	 */
	if (arts[this_resp].name)
		snprintf(buf, line_len, "%s <%s>", arts[this_resp].name, arts[this_resp].from);
	else {
		strncpy(buf, arts[this_resp].from, line_len);
		buf[line_len - 1] = '\0';
	}

	strcpy(tmp, buf);
	strunc(tmp, buf, line_len, cCOLS - 1);

	if (note_h->org) {
		snprintf(tmp, line_len, _(txt_at_s), note_h->org);

		if ((int) strlen(buf) + (int) strlen(tmp) >= cCOLS - 1) {
			strncat(buf, tmp, line_len - strlen(buf) - 1);
			strcpy(tmp, buf);
			strunc(tmp, buf, line_len, cCOLS - 1);
		} else {
			len = cCOLS - 1 - (int) strlen(tmp);
			for (i = strlen(buf); i < (int) len; i++)
				buf[i] = ' ';
			buf[i] = '\0';
			strncat(buf, tmp, line_len - strlen(buf) - 1);
		}
	}
	convert_to_printable(buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_from);
#endif /* HAVE_COLOR */

	my_fputs(buf, stdout);
	my_fputs(cCRLF, stdout);
	my_fputs(cCRLF, stdout);

	/* clean up */
	free(line);
	free(tmp);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	free(buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
}


/*
 * Change the pager article context to arts[new_respnum]
 * Return GRP_ARTUNAVAIL if article could not be opened
 * or GRP_ARTABORT if load of article was interrupted
 * or 0 on success
 */
static int
load_article(
	int new_respnum,
	struct t_group *group)
{
#ifdef DEBUG
	fprintf(stderr, "load_art %s(new=%d, curr=%d)\n", (new_respnum == this_resp) ? "ALREADY OPEN!" : "", new_respnum, this_resp);
#endif /* DEBUG */

	if (new_respnum != this_resp) {
		char *progress_mesg;
		int ret;

		art_close(&pgart);			/* close previously opened art in pager */

		progress_mesg = my_strdup(_(txt_reading_article));
		ret = art_open(TRUE, &arts[new_respnum], group, &pgart, TRUE, progress_mesg);
		free(progress_mesg);
		switch (ret) {
			case ART_UNAVAILABLE:
				art_mark(group, &arts[new_respnum], ART_READ);
				wait_message(1, _(txt_art_unavailable));
				return GRP_ARTUNAVAIL;

			case ART_ABORT:
				art_close(&pgart);
				return GRP_ARTABORT;	/* special retcode to stop redrawing screen */

			default:					/* Normal case */
#if 0			/* Very useful debugging tool */
				if (prompt_yn(cLINES, "Fake art unavailable? ", FALSE) == 1) {
					art_close(&pgart);
					art_mark(group, &arts[new_respnum], ART_READ);
					return GRP_ARTUNAVAIL;
				}
#endif /* 0 */
				/*
				 * Remember current & previous articles for '-' command
				 */
				last_resp = this_resp;
				this_resp = new_respnum;		/* Set new art globally */
				break;
		}
	}

	art_mark(group, &arts[this_resp], ART_READ);

	if (pgart.cooked == NULL) { /* harmony corruption */
		wait_message(1, _(txt_art_unavailable));
		return GRP_ARTUNAVAIL;
	}

	/*
	 * Setup to start viewing cooked version
	 */
	show_all_headers = FALSE;
	curr_line = 0;
	note_fp = pgart.cooked;
	artline = pgart.cookl;
	artlines = pgart.cooked_lines;
	search_line = 0;
	rotate = 0;			/* normal mode, not rot13 */
	reveal_ctrl_l = FALSE;
	reveal_ctrl_l_lines = -1;	/* all ^L's active */
	hide_uue = tinrc.hide_uue;

	draw_page(group->name, 0);

	/*
	 * Automatically invoke attachment viewing if requested
	 */
	if (!note_h->mime || IS_PLAINTEXT(note_h->ext))		/* Text only article */
		return 0;

	if (*tinrc.metamail_prog == '\0' || getenv("NOMETAMAIL") != NULL)	/* Viewer turned off */
		return 0;

	if (tinrc.ask_for_metamail) {
		if (prompt_yn(cLINES, _(txt_use_mime), TRUE) != 1)
			return 0;
	}

	XFACE_SUPPRESS();
	if (strcmp(tinrc.metamail_prog, INTERNAL_CMD) == 0)	/* Use internal viewer */
		decode_save_mime(&pgart, FALSE);
	else
		invoke_metamail(pgart.raw);
	XFACE_SHOW();
	return 0;
}


static int
prompt_response(
	int ch,
	int curr_respnum)
{
	int num;

	clear_message();

	if ((num = prompt_num(ch, _(txt_read_resp))) == -1) {
		clear_message();
		return -1;
	}

	return find_response(which_thread(curr_respnum), num);
}


/*
 * Reposition within message as needed, highlight searched string
 * This is tied quite closely to the information stored by
 * get_search_vectors()
 */
static void
process_search(
	int *lcurr_line,
	size_t message_lines,
	size_t screen_lines,
	int help_level)
{
	int i, start, end;

	if ((i = get_search_vectors(&start, &end)) == -1)
		return;

	/*
	 * Is matching line off the current view?
	 * Reposition within article if needed, try to get matched line
	 * in the middle of the screen
	 */
	if (i < *lcurr_line || i >= (int) (*lcurr_line + screen_lines)) {
		*lcurr_line = i - (screen_lines / 2);
		if (*lcurr_line + screen_lines > message_lines)	/* off the end */
			*lcurr_line = message_lines - screen_lines;
		/* else pos. is just fine */
	}

	switch (help_level) {
		case PAGE_LEVEL:
			draw_page(curr_group->name, 0);
			break;

		case INFO_PAGER:
			display_info_page(0);
			break;

		default:
			break;
	}
	search_line = i;								/* draw_page() resets this to 0 */

	/*
	 * Highlight found string
	 */
	highlight_string(i - *lcurr_line + scroll_region_top, start, end - start);
}


/*
 * Implement ^H toggle between cooked and raw views of article
 */
void
toggle_raw(
	struct t_group *group)
{
	if (show_all_headers) {
		artline = pgart.cookl;
		artlines = pgart.cooked_lines;
		note_fp = pgart.cooked;
	} else {
		static int j;				/* Needed on successive invocations */
		int chunk = note_h->ext->line_count;

		/*
		 * We do this on the fly, since most of the time it won't be used
		 */
		if (!pgart.rawl) {			/* Already done this for this article? */
			char *line;
			char *p;
			long offset;

			j = 0;
			rewind(pgart.raw);
			pgart.rawl = my_malloc(sizeof(t_lineinfo) * chunk);
			offset = ftell(pgart.raw);

			while (NULL != (line = tin_fgets(pgart.raw, FALSE))) {
				int space;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				int num_bytes;
				wchar_t wc;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

				pgart.rawl[j].offset = offset;
				pgart.rawl[j].flags = 0;
				j++;
				if (j >= chunk) {
					chunk += 50;
					pgart.rawl = my_realloc(pgart.rawl, sizeof(t_lineinfo) * chunk);
				}

				p = line;
				while (*p) {
					space = cCOLS - 1;

					while ((space > 0) && *p) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
						num_bytes = mbtowc(&wc, p, MB_CUR_MAX);
						if (num_bytes != -1 && iswprint(wc)) {
							if ((space -= wcwidth(wc)) < 0)
								break;
							p += num_bytes;
							offset += num_bytes;
						}
#else
						if (my_isprint((int) *p)) {
							space--;
							p++;
							offset++;
						}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
						else if (IS_LOCAL_CHARSET("Big5") && (unsigned char) *p >= 0xa1 && (unsigned char) *p <= 0xfe && *(p + 1)) {
							/*
							 * Big5: ASCII chars are handled by the normal code
							 * check only for 2-byte chars
							 * TODO: should we also check if the second byte is
							 * also valid?
							 */
							p += 2;
							offset += 2;
							space--;
						} else {
							/*
							 * the current character can't be displayed print it as
							 * an octal value (needs 4 columns) see also
							 * color.c:draw_pager_line()
							 */
							if ((space -= 4) < 0 )
								break;
							offset++;
							p++;
						}
					}
					/*
					 * if we reached the end of the line we don't need to
					 * remember anything
					 */
					if (*p) {
						pgart.rawl[j].offset = offset;
						pgart.rawl[j].flags = 0;
						if (++j >= chunk) {
							chunk += 50;
							pgart.rawl = my_realloc(pgart.rawl, sizeof(t_lineinfo) * chunk);
						}
					}
				}

				/*
				 * only use ftell's return value here because we didn't
				 * take the \n into account above.
				 */
				offset = ftell(pgart.raw);
			}

			pgart.rawl = my_realloc(pgart.rawl, sizeof(t_lineinfo) * j);
		}
		artline = pgart.rawl;
		artlines = j;
		note_fp = pgart.raw;
	}
	curr_line = 0;
	show_all_headers = bool_not(show_all_headers);
	draw_page(group->name, 0);
}


static void
process_url(
	void)
{
	char *ptr;
	char buf[LEN];
	char ubuf[LEN];
	char url[LEN];
	int i;
	int offsets[6];
	int offsets_size = ARRAY_SIZE(offsets);

	/*
	 * TODO: handle mailto: and news: (not NNTP) URLs internally
	 */
	for (i = curr_line; i < artlines; ++i) {
		if (!(artline[i].flags & (C_URL | C_NEWS | C_MAIL)))
			continue;

		/*
		 * Line contains a URL, so read it in
		 */
		fseek(pgart.cooked, artline[i].offset, SEEK_SET);
		ptr = fgets(buf, sizeof(buf), pgart.cooked);

		/*
		 * Step through, finding URL's
		 */
		forever {
			/* any matches left? */
			if (pcre_exec(url_regex.re, url_regex.extra, ptr, strlen(ptr), 0, 0, offsets, offsets_size) == PCRE_ERROR_NOMATCH)
				if (pcre_exec(mail_regex.re, mail_regex.extra, ptr, strlen(ptr), 0, 0, offsets, offsets_size) == PCRE_ERROR_NOMATCH)
					if (pcre_exec(news_regex.re, news_regex.extra, ptr, strlen(ptr), 0, 0, offsets, offsets_size) == PCRE_ERROR_NOMATCH)
						break;

			*(ptr + offsets[1]) = '\0';

			if (prompt_default_string("URL:", url, sizeof(url), ptr + offsets[0], HIST_URL)) {
				if (!*url)			/* Don't try and open nothing */
					break;

				wait_message(2, _(txt_url_open), url);
				snprintf(ubuf, sizeof(ubuf), "%s %s", tinrc.url_handler, escape_shell_meta(url, 0));
				invoke_cmd(ubuf);
			}
			ptr += offsets[1] + 1;
		}
	}
	info_message(_(txt_url_done));
}


/*
 * Re-cook an article
 *
 * TODO: check cook_article()s return code
 */
void
resize_article(
	t_bool wrap_lines,
	t_openartinfo *artinfo)
{
	free(artinfo->cookl);
	if (artinfo->cooked)
		fclose(artinfo->cooked);

	cook_article(wrap_lines, artinfo, tabwidth, hide_uue);

	show_all_headers = FALSE;
	artline = pgart.cookl;
	artlines = pgart.cooked_lines;
	note_fp = pgart.cooked;
}


/*
 * Infopager: simply page files
 */
void
info_pager(
	FILE *info_fh,
	const char *title,
	t_bool wrap_at_ends)
{
	int ch;
	int offset;

	search_line = 0;
	info_file = info_fh;
	info_title = title;
	curr_info_line = 0;
	preprocess_info_message(info_fh);
	set_xclick_off();
	display_info_page(0);

	forever {
		switch (ch = handle_pager_keypad(&menukeymap.info_nav)) {
			case ESC:	/* common arrow keys */
				break;

			case iKeyUp:				/* line up */
			case iKeyUp2:
				if (num_info_lines <= NOTESLINES) {
					info_message(_(txt_begin_of_art));
					break;
				}
				if (curr_info_line == 0) {
					if (!wrap_at_ends) {
						info_message(_(txt_begin_of_art));
						break;
					}
					curr_info_line = num_info_lines - NOTESLINES;
					display_info_page(0);
					break;
				}
				offset = scroll_page(KEYMAP_UP);
				curr_info_line += offset;
				display_info_page(offset);
				break;

			case iKeyDown:				/* line down */
			case iKeyDown2:
				if (num_info_lines <= NOTESLINES) {
					info_message(_(txt_end_of_art));
					break;
				}
				if (curr_info_line + NOTESLINES >= num_info_lines) {
					if (!wrap_at_ends) {
						info_message(_(txt_end_of_art));
						break;
					}
					curr_info_line = 0;
					display_info_page(0);
					break;
				}
				offset = scroll_page(KEYMAP_DOWN);
				curr_info_line += offset;
				display_info_page(offset);
				break;

			case iKeyPageDown:			/* page down */
			case iKeyPageDown2:
			case iKeyPageDown3:
				if (num_info_lines <= NOTESLINES) {
					info_message(_(txt_end_of_art));
					break;
				}
				if (curr_info_line + NOTESLINES >= num_info_lines) {	/* End is already on screen */
					if (!wrap_at_ends) {
						info_message(_(txt_end_of_art));
						break;
					}
					curr_info_line = 0;
					display_info_page(0);
					break;
				}
				curr_info_line += (tinrc.scroll_lines == -2) ? NOTESLINES / 2 : NOTESLINES;
				display_info_page(0);
				break;

			case iKeyPageUp:			/* page up */
			case iKeyPageUp2:
			case iKeyPageUp3:
				if (num_info_lines <= NOTESLINES) {
					info_message(_(txt_begin_of_art));
					break;
				}
				if (curr_info_line == 0) {
					if (!wrap_at_ends) {
						info_message(_(txt_begin_of_art));
						break;
					}
					curr_info_line = num_info_lines - NOTESLINES;
					display_info_page(0);
					break;
				}
				curr_info_line -= (tinrc.scroll_lines == -2) ? NOTESLINES / 2 : NOTESLINES;
				display_info_page(0);
				break;

			case iKeyFirstPage:			/* Home */
			case iKeyHelpFirstPage2:
				if (curr_info_line) {
					curr_info_line = 0;
					display_info_page(0);
				}
				break;

			case iKeyLastPage:			/* End */
			case iKeyHelpLastPage2:
				if (curr_info_line + NOTESLINES != num_info_lines) {
					/* Display a full last page for neatness */
					curr_info_line = num_info_lines - NOTESLINES;
					display_info_page(0);
				}
				break;

			case iKeyToggleHelpDisplay:
				toggle_mini_help(INFO_PAGER);
				display_info_page(0);
				break;

			case iKeySearchSubjF:
			case iKeySearchSubjB:
			case iKeySearchRepeat:
				if (ch == iKeySearchRepeat && i_key_search_last != iKeySearchSubjF && i_key_search_last != iKeySearchSubjB)
					break;

				if ((search_article((ch == iKeySearchSubjF), (ch == iKeySearchRepeat), search_line, num_info_lines, infoline, num_info_lines - 1, info_file)) == -1)
					break;

				process_search(&curr_info_line, num_info_lines, NOTESLINES, INFO_PAGER);
				break;

			case iKeyQuit:	/* quit */
				ClearScreen();
				return;

			default:
				break;
		}
	}
}


/*
 * Redraw the current page, curr_info_line will be the first line displayed
 * If part is !=0, then only draw the first (-ve) or last (+ve) few lines
 */
void
display_info_page(
	int part)
{
	int start, end;	/* 1st, last line to draw */

	signal_context = cInfopager;

	/*
	 * Can't do partial draw if term can't scroll properly
	 */
	if (part && !have_linescroll)
		part = 0;

	if (curr_info_line < 0)
		curr_info_line = 0;
	if (curr_info_line >= num_info_lines)
		curr_info_line = num_info_lines - 1;

	scroll_region_top = INDEX_TOP;

	/* Down-scroll, only redraw bottom 'part' lines of screen */
	if ((start = (part > 0) ? NOTESLINES - part : 0) < 0)
		start = 0;

	/* Up-scroll, only redraw the top 'part' lines of screen */
	if ((end = (part < 0) ? -part : NOTESLINES) > NOTESLINES)
		end = NOTESLINES;

	/* Print title */
	if ((end - start >= NOTESLINES) || (part == 0)) {
		ClearScreen();
		center_line(0, TRUE, info_title);
	}

	print_message_page(info_file, infoline, num_info_lines, curr_info_line, start, end, INFO_PAGER);

	/* print footer */
	draw_percent_mark(curr_info_line + (curr_info_line + NOTESLINES < num_info_lines ? NOTESLINES : num_info_lines - curr_info_line), num_info_lines);
	stow_cursor();
}


/*
 * TODO: plug mem leak: malloced mem is not freed on exit
 */
static void
preprocess_info_message(
	FILE *info_fh)
{
	int chunk = 50;

	rewind(info_fh);
	FreeIfNeeded(infoline);
	infoline = my_malloc(sizeof(t_lineinfo) * chunk);
	num_info_lines = 0;

	do {
		infoline[num_info_lines].offset = ftell(info_fh);
		infoline[num_info_lines].flags = 0;
		num_info_lines++;
		if (num_info_lines >= chunk) {
			chunk += 50;
			infoline = my_realloc(infoline, sizeof(t_lineinfo) * chunk);
		}
	} while (tin_fgets(info_fh, FALSE) != NULL);

	num_info_lines--;
	infoline = my_realloc(infoline, sizeof(t_lineinfo) * num_info_lines);
}
