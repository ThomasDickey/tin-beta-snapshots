/*
 *  Project   : tin - a Usenet reader
 *  Module    : page.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2005-10-19
 *  Notes     :
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
static int load_article(int new_respnum, struct t_group *group);
static int prompt_response(int ch, int curr_respnum);
static int scroll_page(int dir);
static t_bool deactivate_next_ctrl_l(void);
static t_bool activate_last_ctrl_l(void);
static t_function page_left(void);
static t_function page_right(void);
static t_function page_mouse_action(t_function (*left_action) (void), t_function (*right_action) (void));
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
static t_function
page_left(
	void)
{
	return GLOBAL_QUIT;
}


static t_function
page_right(
	void)
{
	return PAGE_NEXT_UNREAD;
}


static t_function
page_mouse_action(
	t_function (*left_action) (void),
	t_function (*right_action) (void))
{
	t_function func = NOT_ASSIGNED;

	switch (xmouse) {
		case MOUSE_BUTTON_1:
			if (xrow < PAGE_HEADER || xrow >= cLINES - 1)
				func = GLOBAL_PAGE_DOWN;
			else
				func = right_action();
			break;

		case MOUSE_BUTTON_2:
			if (xrow < PAGE_HEADER || xrow >= cLINES - 1)
				func = GLOBAL_PAGE_UP;
			else
				func = left_action();
			break;

		case MOUSE_BUTTON_3:
			func = SPECIAL_MOUSE_TOGGLE;
			break;

		default:
			break;
	}
	return func;
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
	int i, n = 0;
	int filter_state = NO_FILTERING;
	int old_sort_art_type = tinrc.sort_article_type;
	int art_type = GROUP_TYPE_NEWS;
	t_bool mouse_click_on = TRUE;
	t_bool repeat_search;
	t_function func;

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
		if ((func = handle_keypad(page_left, page_right, page_mouse_action, page_keys)) == GLOBAL_SEARCH_REPEAT) {
			func = last_search;
			repeat_search = TRUE;
		} else
			repeat_search = FALSE;

		switch (func) {
			case GLOBAL_ABORT:       /* Abort */
				break;

			case DIGIT_0:
			case DIGIT_1:
			case DIGIT_2:
			case DIGIT_3:
			case DIGIT_4:
			case DIGIT_5:
			case DIGIT_6:
			case DIGIT_7:
			case DIGIT_8:
			case DIGIT_9:
				if (!HAS_FOLLOWUPS(which_thread(this_resp)))
					info_message(_(txt_no_responses));
				else {
					if ((n = prompt_response(func_to_key(func, page_keys), this_resp)) != -1) {
						XFACE_CLEAR();
						if ((i = load_article(n, group)) < 0)
							return i;
					}
				}
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				XFACE_CLEAR();
				shell_escape();
				draw_page(group->name, 0);
				break;
#endif /* !NO_SHELL_ESCAPE */

			case SPECIAL_MOUSE_TOGGLE:
				if (mouse_click_on)
					set_xclick_off();
				else
					set_xclick_on();
				mouse_click_on = bool_not(mouse_click_on);
				break;

			case GLOBAL_PAGE_UP:
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

			case GLOBAL_PAGE_DOWN:		/* page down or next response */
			case PAGE_NEXT_UNREAD:
			case PAGE_PAGE_DOWN3:
				if (!((func == PAGE_NEXT_UNREAD) && tinrc.tab_goto_next_unread) && deactivate_next_ctrl_l())
					draw_page(group->name, 0);
				else {
					if (curr_line + ARTLINES >= artlines) {	/* End is already on screen */
						switch (func) {
							case PAGE_NEXT_UNREAD:	/* <TAB> */
								goto page_goto_next_unread;

							case GLOBAL_PAGE_DOWN:
								if (tinrc.pgdn_goto_next)
									goto page_goto_next_unread;
								break;

							case PAGE_PAGE_DOWN3:			/* <SPACE> */
								if (tinrc.space_goto_next_unread)
									goto page_goto_next_unread;
								break;

							default:		/* to keep gcc quiet */
								break;
						}
						info_message(_(txt_end_of_art));
					} else {
						if ((func == PAGE_NEXT_UNREAD) && tinrc.tab_goto_next_unread)
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

			case GLOBAL_FIRST_PAGE:		/* beginning of article */
				if (reveal_ctrl_l_lines > -1 || curr_line != 0) {
					reveal_ctrl_l_lines = -1;
					curr_line = 0;
					draw_page(group->name, 0);
				}
				break;

			case GLOBAL_LAST_PAGE:		/* end of article */
				if (reveal_ctrl_l_lines < artlines - 1 || curr_line + ARTLINES != artlines) {
					reveal_ctrl_l_lines = artlines - 1;
					/* Display a full last page for neatness */
					curr_line = artlines - ARTLINES;
					draw_page(group->name, 0);
				}
				break;

			case GLOBAL_LINE_UP:
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

			case GLOBAL_LINE_DOWN:
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

			case GLOBAL_LAST_VIEWED:	/* show last viewed article */
				if (last_resp < 0 || (which_thread(last_resp) == -1)) {
					info_message(_(txt_no_last_message));
					break;
				}
				if ((i = load_article(last_resp, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}
				break;

			case GLOBAL_LOOKUP_MESSAGEID:			/* Goto article by Message-ID */
				if ((n = prompt_msgid()) != ART_UNAVAILABLE) {
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
				}
				break;

			case PAGE_GOTO_PARENT:		/* Goto parent of this article */
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

			case GLOBAL_PIPE:		/* pipe article/thread/tagged arts to command */
				XFACE_SUPPRESS();
				feed_articles(FEED_PIPE, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

			case PAGE_MAIL:	/* mail article/thread/tagged articles to somebody */
				XFACE_SUPPRESS();
				feed_articles(FEED_MAIL, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

#ifndef DISABLE_PRINTING
			case GLOBAL_PRINT:	/* output art/thread/tagged arts to printer */
				XFACE_SUPPRESS();
				feed_articles(FEED_PRINT, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;
#endif /* !DISABLE_PRINTING */

			case PAGE_REPOST:	/* repost current article */
				XFACE_SUPPRESS();
				feed_articles(FEED_REPOST, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

			case PAGE_SAVE:	/* save article/thread/tagged articles */
				XFACE_SUPPRESS();
				feed_articles(FEED_SAVE, PAGE_LEVEL, group, this_resp);
				XFACE_SHOW();
				break;

			case PAGE_AUTOSAVE:	/* Auto-save articles without prompting */
				if (grpmenu.curr >= 0) {
					XFACE_SUPPRESS();
					feed_articles(FEED_AUTOSAVE, PAGE_LEVEL, group, (int) base[grpmenu.curr]);
					XFACE_SHOW();
				}
				break;

			case GLOBAL_SEARCH_SUBJECT_FORWARD:	/* search in article */
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
				if ((i = search_article((func == GLOBAL_SEARCH_SUBJECT_FORWARD), repeat_search, search_line, artlines, artline, reveal_ctrl_l_lines, note_fp)) == -1)
					break;

				if (func == GLOBAL_SEARCH_SUBJECT_BACKWARD && !reveal_ctrl_l) {
					reveal_ctrl_l_lines = curr_line + ARTLINES - 1;
					draw_page(group->name, 0);
				}
				process_search(&curr_line, artlines, ARTLINES, PAGE_LEVEL);
				break;

			case GLOBAL_SEARCH_BODY:	/* article body search */
				if ((n = search_body(group, this_resp, repeat_search)) != -1) {
					this_resp = n;			/* Stop load_article() changing context again */
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
					process_search(&curr_line, artlines, ARTLINES, PAGE_LEVEL);
				}
				break;

			case PAGE_TOP_THREAD:	/* first article in current thread */
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

			case PAGE_BOTTOM_THREAD:	/* last article in current thread */
				for (i = this_resp; i >= 0; i = arts[i].thread)
					n = i;

				if (n != this_resp) {
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
				}
				break;

			case PAGE_NEXT_THREAD:	/* start of next thread */
				XFACE_CLEAR();
				if ((n = next_thread(this_resp)) == -1)
					return (which_thread(this_resp));
				if ((i = load_article(n, group)) < 0)
					return i;
				break;

#ifdef HAVE_PGP_GPG
			case PAGE_PGP_CHECK_ARTICLE:
				XFACE_SUPPRESS();
				if (pgp_check_article(&pgart))
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;
#endif /* HAVE_PGP_GPG */

			case PAGE_TOGGLE_HEADERS:	/* toggle display of whole 'raw' article */
				XFACE_CLEAR();
				toggle_raw(group);
				break;

			case PAGE_TOGGLE_TEX2ISO:		/* toggle german TeX to ISO latin1 style conversion */
				if (((group->attribute->tex2iso_conv) = !(group->attribute->tex2iso_conv)))
					pgart.tex2iso = is_art_tex_encoded(pgart.raw);
				else
					pgart.tex2iso = FALSE;

				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(group->name, 0);
				info_message(_(txt_toggled_tex2iso), txt_onoff[group->attribute->tex2iso_conv != FALSE ? 1 : 0]);
				break;

			case PAGE_TOGGLE_TABS:		/* toggle tab stops 8 vs 4 */
				tabwidth = (tabwidth == 8) ? 4 : 8;
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(group->name, 0);
				info_message(_(txt_toggled_tabwidth), tabwidth);
				break;

			case PAGE_TOGGLE_UUE:			/* toggle display of uuencoded sections */
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

			case PAGE_REVEAL:			/* toggle hiding after ^L */
				reveal_ctrl_l = bool_not(reveal_ctrl_l);
				if (!reveal_ctrl_l) {	/* switched back to active ^L's */
					reveal_ctrl_l_lines = -1;
					curr_line = 0;
				} else
					reveal_ctrl_l_lines = artlines - 1;
				draw_page(group->name, 0);
				/* TODO: info_message()? */
				break;

			case GLOBAL_QUICK_FILTER_SELECT:	/* quickly auto-select article */
			case GLOBAL_QUICK_FILTER_KILL:		/* quickly kill article */
				if ((filtered_articles = quick_filter(func, group, &arts[this_resp])))
					goto return_to_index;

				draw_page(group->name, 0);
				break;

			case GLOBAL_MENU_FILTER_SELECT:		/* auto-select article menu */
			case GLOBAL_MENU_FILTER_KILL:			/* kill article menu */
				XFACE_CLEAR();
				if (filter_menu(func, group, &arts[this_resp])) {
					if ((filtered_articles = filter_articles(group)))
						goto return_to_index;
				}
				draw_page(group->name, 0);
				break;

			case GLOBAL_EDIT_FILTER:
				XFACE_CLEAR();
				if (!invoke_editor(filter_file, FILTER_FILE_OFFSET))
					break;
				unfilter_articles();
				(void) read_filter_file(filter_file);
				if ((filtered_articles = filter_articles(group)))
					goto return_to_index;
				draw_page(group->name, 0);
				break;

			case GLOBAL_REDRAW_SCREEN:		/* redraw current page of article */
				my_retouch();
				draw_page(group->name, 0);
				break;

			case PAGE_TOGGLE_ROT13:	/* toggle rot-13 mode */
				rotate = rotate ? 0 : 13;
				draw_page(group->name, 0);
				info_message(_(txt_toggled_rot13));
				break;

			case GLOBAL_SEARCH_AUTHOR_FORWARD:	/* author search forward */
			case GLOBAL_SEARCH_AUTHOR_BACKWARD:	/* author search backward */
				if ((n = search(func, this_resp, repeat_search)) < 0)
					break;
				if ((i = load_article(n, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}
				break;

			case CATCHUP:			/* catchup - mark read, goto next */
			case CATCHUP_NEXT_UNREAD:	/* goto next unread */
				/*
				 * TODO: if (group->attribute->thread_arts == THREAD_NONE)
				 *       	snprintf(buf, sizeof(buf), _("Mark article as read%s?"), (func == CATCHUP_NEXT_UNREAD) ? _(" and enter next unread article") : "");
				 *       else
				 */
				snprintf(buf, sizeof(buf), _(txt_mark_thread_read), (func == CATCHUP_NEXT_UNREAD) ? _(txt_enter_next_thread) : "");
				if ((!TINRC_CONFIRM_ACTION) || prompt_yn(buf, TRUE) == 1) {
					thd_mark_read(group, base[which_thread(this_resp)]);
					XFACE_CLEAR();
					return (func == CATCHUP_NEXT_UNREAD) ? GRP_NEXTUNREAD : GRP_NEXT;
				}
				break;

			case MARK_THREAD_UNREAD:
				thd_mark_unread(group, base[which_thread(this_resp)]);
				if (group->attribute->thread_arts != THREAD_NONE)
					info_message(_(txt_marked_as_unread), _(txt_thread_upper));
				else
					info_message(_(txt_marked_as_unread), _(txt_article_upper));
				break;

			case PAGE_CANCEL:			/* cancel an article */
				if (can_post || art_type != GROUP_TYPE_NEWS) {
					XFACE_SUPPRESS();
					if (cancel_article(group, &arts[this_resp], this_resp))
						draw_page(group->name, 0);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case PAGE_EDIT_ARTICLE:		/* edit an article (mailgroup only) */
				XFACE_SUPPRESS();
				if (art_edit(group, &arts[this_resp]))
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case PAGE_FOLLOWUP_QUOTE:		/* post a followup to this article */
			case PAGE_FOLLOWUP_QUOTE_HEADERS:
			case PAGE_FOLLOWUP:
				if (!can_post && art_type == GROUP_TYPE_NEWS) {
					info_message(_(txt_cannot_post));
					break;
				}
				XFACE_CLEAR();
				(void) post_response(group->name, this_resp,
				  (func == PAGE_FOLLOWUP_QUOTE || func == PAGE_FOLLOWUP_QUOTE_HEADERS) ? TRUE : FALSE,
				  func == PAGE_FOLLOWUP_QUOTE_HEADERS ? TRUE : FALSE, show_all_headers);
				draw_page(group->name, 0);
				break;

			case GLOBAL_HELP:	/* help */
				XFACE_CLEAR();
				show_help_page(PAGE_LEVEL, _(txt_art_pager_com));
				draw_page(group->name, 0);
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:	/* toggle mini help menu */
				toggle_mini_help(PAGE_LEVEL);
				draw_page(group->name, 0);
				break;

			case GLOBAL_QUIT:	/* return to index page */
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

			case GLOBAL_TOGGLE_INVERSE_VIDEO:	/* toggle inverse video */
				toggle_inverse_video();
				draw_page(group->name, 0);
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:		/* toggle color */
				if (toggle_color()) {
					draw_page(group->name, 0);
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case PAGE_LIST_THREAD:	/* -> thread page that this article is in */
				XFACE_CLEAR();
				fixup_thread(this_resp, FALSE);
				return GRP_GOTOTHREAD;

			case GLOBAL_OPTION_MENU:	/* option menu */
				XFACE_CLEAR();
				if (change_config_file(group) == FILTERING)
					filter_state = FILTERING;
				draw_page(group->name, 0);
				break;

			case PAGE_NEXT_ARTICLE:	/* skip to next article */
				XFACE_CLEAR();
				if ((n = next_response(this_resp)) == -1)
					return (which_thread(this_resp));

				if ((i = load_article(n, group)) < 0)
					return i;
				break;

			case PAGE_MARK_THREAD_READ:	/* mark rest of thread as read */
				thd_mark_read(group, this_resp);
				if ((n = next_unread(next_response(this_resp))) == -1)
					goto return_to_index;
				if ((i = load_article(n, group)) < 0) {
					XFACE_CLEAR();
					return i;
				}
				break;

			case PAGE_NEXT_UNREAD_ARTICLE:	/* next unread article */
				goto page_goto_next_unread;

			case PAGE_PREVIOUS_ARTICLE:	/* previous article */
				XFACE_CLEAR();
				if ((n = prev_response(this_resp)) == -1)
					return this_resp;

				if ((i = load_article(n, group)) < 0)
					return i;
				break;

			case PAGE_PREVIOUS_UNREAD_ARTICLE:	/* previous unread article */
				if ((n = prev_unread(prev_response(this_resp))) == -1)
					info_message(_(txt_no_prev_unread_art));
				else {
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
				}
				break;

			case GLOBAL_QUIT_TIN:	/* quit */
				XFACE_CLEAR();
				return GRP_QUIT;

			case PAGE_REPLY_QUOTE:	/* reply to author through mail */
			case PAGE_REPLY_QUOTE_HEADERS:
			case PAGE_REPLY:
				XFACE_CLEAR();
				mail_to_author(group->name, this_resp, (func == PAGE_REPLY_QUOTE || func == PAGE_REPLY_QUOTE_HEADERS) ? TRUE : FALSE, func == PAGE_REPLY_QUOTE_HEADERS ? TRUE : FALSE, show_all_headers);
				draw_page(group->name, 0);
				break;

			case PAGE_TAG:	/* tag/untag article for saving */
				tag_article(this_resp);
				break;

			case PAGE_GROUP_SELECT:	/* return to group selection page */
				if (filter_state == FILTERING) {
					filter_articles(group);
					make_threads(group, FALSE);
				}
				XFACE_CLEAR();
				return GRP_RETSELECT;

			case GLOBAL_VERSION:
				info_message(cvers);
				break;

			case GLOBAL_POST:	/* post a basenote */
				XFACE_SUPPRESS();
				if (post_article(group->name))
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case GLOBAL_POSTPONED:	/* post postponed article */
				if (can_post || art_type != GROUP_TYPE_NEWS) {
					XFACE_SUPPRESS();
					if (pickup_postponed_articles(FALSE, FALSE))
						draw_page(group->name, 0);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case GLOBAL_DISPLAY_POST_HISTORY:	/* display messages posted by user */
				XFACE_SUPPRESS();
				if (user_posted_messages())
					draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case MARK_ARTICLE_UNREAD:	/* mark article as unread(to return) */
				art_mark(group, &arts[this_resp], ART_WILL_RETURN);
				info_message(_(txt_marked_as_unread), _(txt_article_upper));
				break;

			case PAGE_SKIP_INCLUDED_TEXT:	/* skip included text */
				for (i = curr_line; i < artlines; i++) {
					if (!(artline[i].flags & (C_QUOTE1 | C_QUOTE2 | C_QUOTE3)))
						break;
				}

				if (i != curr_line) {
					curr_line = i;
					draw_page(group->name, 0);
				}
				break;

			case GLOBAL_TOGGLE_INFO_LAST_LINE: /* this is _not_ correct, we do not toggle status here */
				info_message("%s", arts[this_resp].subject);
				break;

			case PAGE_TOGGLE_HIGHLIGHTING:
				word_highlight = bool_not(word_highlight);
				draw_page(group->name, 0);
				info_message(_(txt_toggled_high), txt_onoff[word_highlight != FALSE ? 1 : 0]);
				break;

			case PAGE_VIEW_ATTACHMENTS:
				XFACE_SUPPRESS();
				decode_save_mime(&pgart, FALSE);
				draw_page(group->name, 0);
				XFACE_SHOW();
				break;

			case PAGE_VIEW_URL:
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
				info_message(_(txt_bad_command), printascii(key, func_to_key(GLOBAL_HELP, page_keys)));
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
		if (base_line + i < messagelines - 1) {	/* not last line of message */
			bytes = (curr + 1)->offset - curr->offset;
			line[bytes] = '\0';
		}

		/*
		 * rotN encoding on body and sig data only
		 */
		if ((rotate != 0) && (curr->flags & (C_BODY | C_SIG))) {
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
		wchar_t *wbuf;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		STRCPY(buf, (arts[this_resp].thread != -1) ? _(txt_next_resp) : _(txt_last_resp));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wbuf = char2wchar_t(buf)) != NULL) {
			len = wcswidth(wbuf, wcslen(wbuf) + 1);
			free(wbuf);
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

	/* FIXME: values do differ for different languages */
	MoveCursor(cLINES, cCOLS - 20 - BLANK_PAGE_COLS);
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
	int len, right_len, center_pos, cur_pos;
	size_t line_len;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *fmt_resp, *fmt_thread, *wtmp, *wtmp2;
#else
	char *tmp;
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
	/* convert to wide-char format strings */
	fmt_thread = char2wchar_t(_(txt_thread_x_of_n));
	fmt_resp = char2wchar_t(_(txt_resp_x_of_n));

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
	FreeIfNeeded(fmt_thread);
	FreeIfNeeded(fmt_resp);

	/*
	 * first line
	 */
	cur_pos = 0;

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	/* date */
	if ((wtmp = char2wchar_t(buf)) != NULL) {
		my_fputws(wtmp, stdout);
		cur_pos += wcswidth(wtmp, wcslen(wtmp));
		free(wtmp);
	}

	/*
	 * determine max len for centered group name
	 * allow one space before and after group name
	 */
	len = cCOLS - 2 * MAX(cur_pos, right_len) - 3;

	/* group name */
	if ((wtmp = char2wchar_t(group)) != NULL) {
		/* wconvert_to_printable(wtmp); */
		if ((i = wcswidth(wtmp, wcslen(wtmp))) < len)
			len = i;

		center_pos = (cCOLS - len) / 2;

		/* pad out to left */
		for (; cur_pos < center_pos; cur_pos++)
			my_fputc(' ', stdout);

		wtmp2 = wstrunc(wtmp, len);
		my_fputws(wtmp2, stdout);
		cur_pos += wcswidth(wtmp2, wcslen(wtmp2));
		free(wtmp2);
		free(wtmp);
	}

	/* pad out to right */
	for (; cur_pos < cCOLS - right_len - 1; cur_pos++)
		my_fputc(' ', stdout);

	/* thread info */
	/* can't eval tin_ltoa() more than once in a statement due to statics */
	strcpy(buf, tin_ltoa(which_thread(this_resp) + 1, 4));
	my_printf(_(txt_thread_x_of_n), buf, tin_ltoa(grpmenu.max, 4));

	my_fputs(cCRLF, stdout);

#	if 0
	/* display a ruler for layout checking purposes */
	my_fputs("....|....3....|....2....|....1....|....0....|....1....|....2....|....3....|....\n", stdout);
#	endif /* 0 */

	/*
	 * second line
	 */
	cur_pos = 0;

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	/* line count */
	if (arts[this_resp].line_count < 0)
		strcpy(buf, "?");
	else
		snprintf(buf, line_len, "%-4d", arts[this_resp].line_count);

	{
		wchar_t *fmt;

		if ((fmt = char2wchar_t(_(txt_lines))) != NULL) {
			wtmp = my_malloc(sizeof(wchar_t) * line_len);
			swprintf(wtmp, line_len, fmt, buf);
			my_fputws(wtmp, stdout);
			cur_pos += wcswidth(wtmp, wcslen(wtmp));
			free(fmt);
			free(wtmp);
		}
	}

#	ifdef HAVE_COLOR
	fcol(tinrc.col_subject);
#	endif /* HAVE_COLOR */

	/* tex2iso */
	if (pgart.tex2iso) {
		wtmp = char2wchar_t(_(txt_tex));
		my_fputws(wtmp, stdout);
		cur_pos += wcswidth(wtmp, wcslen(wtmp));
		free(wtmp);
	}

	/* subject */
	/*
	 * TODO: why do we fall back to arts[this_resp].subject if !note_h->subj?
	 *       if !note_h->subj then the article just has no subject, no matter
	 *       what the overview says.
	 */
	strncpy(buf, (note_h->subj ? note_h->subj : arts[this_resp].subject), line_len);
	buf[line_len - 1] = '\0';
	if ((wtmp = char2wchar_t(buf)) != NULL) {
		wtmp2 = wstrunc(wtmp, cCOLS - 2 * right_len - 3);

		center_pos = (cCOLS - wcswidth(wtmp2, wcslen(wtmp2))) / 2;

		/* pad out to left */
		for (; cur_pos < center_pos; cur_pos++)
			my_fputc(' ', stdout);

		StartInverse();
		my_fputws(wtmp2, stdout);
		EndInverse();
		cur_pos += wcswidth(wtmp2, wcslen(wtmp2));
		free(wtmp2);
		free(wtmp);
	}

#	ifdef HAVE_COLOR
	fcol(tinrc.col_response);
#	endif /* HAVE_COLOR */

	/* pad out to right */
	for (; cur_pos < cCOLS - right_len - 1; cur_pos++)
		my_fputc(' ', stdout);

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
	cur_pos = 0;

#	ifdef HAVE_COLOR
	fcol(tinrc.col_from);
#	endif /* HAVE_COLOR */
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

	if ((wtmp = char2wchar_t(buf)) != NULL) {
		wtmp2 = wstrunc(wtmp, cCOLS - 1);
		my_fputws(wtmp2, stdout);
		cur_pos += wcswidth(wtmp2, wcslen(wtmp2));
		free(wtmp2);
		free(wtmp);
	}

	/* organization */
	if ((wtmp = char2wchar_t(_(txt_at_s))) != NULL) {
		len = wcswidth(wtmp, wcslen(wtmp));
		free(wtmp);
	} else
		len = 0;
	if (note_h->org && cCOLS - cur_pos - 1 >= len - 2 + 3) {
		/* we have enough space to print at least " at ..." */
		snprintf(buf, line_len, _(txt_at_s), note_h->org);

		if ((wtmp = char2wchar_t(buf)) != NULL) {
			wconvert_to_printable(wtmp);
			wtmp2 = wstrunc(wtmp, cCOLS - cur_pos - 1);

			i = cCOLS - wcswidth(wtmp2, wcslen(wtmp2)) - 1;
			for (; cur_pos < i; cur_pos++)
				my_fputc(' ', stdout);

			my_fputws(wtmp2, stdout);
			free(wtmp2);
			free(wtmp);
		}
	}

	my_fputs(cCRLF, stdout);
	my_fputs(cCRLF, stdout);

#else /* !MULTIBYTE_ABLE || NO_LOCALE */
	/*
	 * determine the needed space for the text at the right hand margin
	 * the formating info (%4s) needs 3 positions but we need 4 positions
	 * on the screen for each counter
	 */
	right_len = MAX((strlen(_(txt_thread_x_of_n)) - 6 + 8), (strlen(_(txt_resp_x_of_n)) - 6 + 8));

	/*
	 * first line
	 */
	cur_pos = 0;

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	/* date */
	my_fputs(buf, stdout);
	cur_pos += strlen(buf);

	/*
	 * determine max len for centered group name
	 * allow one space before and after group name
	 */
	len = cCOLS - 2 * MAX(cur_pos, right_len) - 3;

	/* group name */
	if ((i = strlen(group)) < len)
		len = i;

	center_pos = (cCOLS - len) / 2;

	/* pad out to left */
	for (; cur_pos < center_pos; cur_pos++)
		my_fputc(' ', stdout);

	tmp = strunc(group, len);
	my_fputs(tmp, stdout);
	cur_pos += strlen(tmp);
	free(tmp);

	/* pad out to right */
	for (; cur_pos < cCOLS - right_len - 1; cur_pos++)
		my_fputc(' ', stdout);

	/* thread info */
	/* can't eval tin_ltoa() more than once in a statement due to statics */
	strcpy(buf, tin_ltoa(which_thread(this_resp) + 1, 4));
	my_printf(_(txt_thread_x_of_n), buf, tin_ltoa(grpmenu.max, 4));

	my_fputs(cCRLF, stdout);

#	if 0
	/* display a ruler for layout checking purposes */
	my_fputs("....|....3....|....2....|....1....|....0....|....1....|....2....|....3....|....\n", stdout);
#	endif /* 0 */

	/*
	 * second line
	 */
	cur_pos = 0;

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	/* line count */
	/* an accurate line count will appear in the footer anymay */
	if (arts[this_resp].line_count < 0)
		strcpy(buf, "?");
	else
		snprintf(buf, line_len, "%-4d", arts[this_resp].line_count);

	tmp = my_malloc(line_len);
	snprintf(tmp, line_len, _(txt_lines), buf);
	my_fputs(tmp, stdout);
	cur_pos += strlen(tmp);
	free(tmp);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_subject);
#	endif /* HAVE_COLOR */

	/* tex2iso */
	if (pgart.tex2iso) {
		my_fputs(_(txt_tex), stdout);
		cur_pos += strlen(_(txt_tex));
	}

	/* subject */
	/*
	 * TODO: why do we fall back to arts[this_resp].subject if !note_h->subj?
	 *       if !note_h->subj then the article just has no subject, no matter
	 *       what the overview says.
	 */
	strncpy(buf, (note_h->subj ? note_h->subj : arts[this_resp].subject), line_len);
	buf[line_len - 1] = '\0';

	tmp = strunc(buf, cCOLS - 2 * right_len - 3);

	center_pos = (cCOLS - strlen(tmp)) / 2;

	/* pad out to left */
	for (; cur_pos < center_pos; cur_pos++)
		my_fputc(' ', stdout);

	StartInverse();
	my_fputs(tmp, stdout);
	EndInverse();
	cur_pos += strlen(tmp);
	free(tmp);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_response);
#	endif /* HAVE_COLOR */

	/* pad out to right */
	for (; cur_pos < cCOLS - right_len - 1; cur_pos++)
		my_fputc(' ', stdout);

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
	cur_pos = 0;

#ifdef HAVE_COLOR
	fcol(tinrc.col_from);
#endif /* HAVE_COLOR */
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

	tmp = strunc(buf, cCOLS - 1);
	my_fputs(tmp, stdout);
	cur_pos += strlen(tmp);
	free(tmp);

	if (note_h->org && cCOLS - cur_pos - 1 >= (int) strlen(_(txt_at_s)) - 2 + 3) {
		/* we have enough space to print at least " at ..." */
		snprintf(buf, line_len, _(txt_at_s), note_h->org);

		tmp = strunc(buf, cCOLS - cur_pos - 1);
		len = cCOLS - (int) strlen(tmp) - 1;
		for (; cur_pos < len; cur_pos++)
			my_fputc(' ', stdout);
		my_fputs(tmp, stdout);
		free(tmp);
	}

	my_fputs(cCRLF, stdout);
	my_fputs(cCRLF, stdout);
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
	if (debug == 2)
		fprintf(stderr, "load_art %s(new=%d, curr=%d)\n", (new_respnum == this_resp) ? "ALREADY OPEN!" : "", new_respnum, this_resp);
#endif /* DEBUG */

	if (new_respnum != this_resp) {
		char *progress_mesg = my_strdup(_(txt_reading_article));
		int ret;

		art_close(&pgart);			/* close previously opened art in pager */

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
	reset_srch_offsets();
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
		if (prompt_yn(_(txt_use_mime), TRUE) != 1)
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
							if ((space -= 4) < 0)
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
	int offset;
	t_function func;

	search_line = 0;
	reset_srch_offsets();
	info_file = info_fh;
	info_title = title;
	curr_info_line = 0;
	preprocess_info_message(info_fh);
	set_xclick_off();
	display_info_page(0);

	forever {
		switch (func = handle_keypad(page_left, page_right, page_mouse_action, info_keys)) {
			case GLOBAL_ABORT:	/* common arrow keys */
				break;

			case GLOBAL_LINE_UP:
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

			case GLOBAL_LINE_DOWN:
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

			case GLOBAL_PAGE_DOWN:
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

			case GLOBAL_PAGE_UP:
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

			case GLOBAL_FIRST_PAGE:
				if (curr_info_line) {
					curr_info_line = 0;
					display_info_page(0);
				}
				break;

			case GLOBAL_LAST_PAGE:
				if (curr_info_line + NOTESLINES != num_info_lines) {
					/* Display a full last page for neatness */
					curr_info_line = num_info_lines - NOTESLINES;
					display_info_page(0);
				}
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:
				toggle_mini_help(INFO_PAGER);
				display_info_page(0);
				break;

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					break;

				if ((search_article((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT), search_line, num_info_lines, infoline, num_info_lines - 1, info_file)) == -1)
					break;

				process_search(&curr_info_line, num_info_lines, NOTESLINES, INFO_PAGER);
				break;

			case GLOBAL_QUIT:	/* quit */
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
