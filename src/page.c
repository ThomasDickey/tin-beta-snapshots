/*
 *  Project   : tin - a Usenet reader
 *  Module    : page.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1995-07-26
 *  Notes     :
 *
 * Copyright (c) 1991-2000 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Iain Lea, Rich Skrenta.
 * 4. The name of the author may not be used to endorse or promote
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
#	include  "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef RFC2046_H
#	include  "rfc2046.h"
#endif /* !RFC2046_H */

#define PAGE_HEADER	4

/*
 * size of page header over and above INDEX_TOP
 * [ Used because NOTESLINES only specifies a 2 line header ]
 */
#define HDR_ADJUST	(PAGE_HEADER-INDEX_TOP)

/*
 * The number of lines available to display actual article text
 */
#define ARTLINES	(NOTESLINES-HDR_ADJUST)

FILE *note_fp;			/* active stream (raw or cooked) */
int artlines;			/* active # of lines in pager */
int curr_line;			/* current line in art (indexed from 0) */
t_lineinfo *artline;	/* active 'lineinfo' data */

t_openartinfo pgart =	/* Global context of article open in the pager */
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FALSE, 0 },
		FALSE, 0,
		NULL, NULL, NULL, NULL,
	};

int MORE_POS;			/* set in set_win_size () */
int RIGHT_POS;			/* set in set_win_size () */

int last_resp;			/* previous & current article # in arts[] for '-' command */
int this_resp;

int tabwidth = 8;

struct t_header *note_h = &pgart.hdr;	/* Easy access to article headers */

static int rotate;				/* 0=normal, 13=rot13 decode */
static int search_line;			/* Line to commence next search from */

static t_bool show_all_headers;	/* CTRL-H with headers specified */
static t_bool reveal_ctrl_l;	/* set when ^L hiding is off */
static t_bool hide_uue;			/* set when uuencoded sections are 'hidden' */

/*
 * Local prototypes
 */
static int load_article (int new_respnum);
static int prompt_response (int ch, int curr_respnum);
static void process_search (void);
static void process_url (void);
static void show_first_header (const char *group);
#ifdef HAVE_METAMAIL
	static void show_mime_article (FILE *fp);
#endif /* HAVE_METAMAIL */


/*
 * Scroll visible article part of display down (+ve) or up (-ve) by 'i'
 * lines.
 */
static void
scroll_page (
	int i)
{
#ifdef USE_CURSES
	scrollok(stdscr, TRUE);
#endif /* USE_CURSES */
	SetScrollRegion(INDEX_TOP + HDR_ADJUST, ARTLINES + HDR_ADJUST +1);
	ScrollScreen(i);
	SetScrollRegion(0, cLINES);
#ifdef USE_CURSES
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */
}


/*
 * Map keypad codes to standard keyboard characters
 */
static int
handle_pager_keypad(
	void)
{
	int ch = ReadCh ();

	switch (ch) {
#ifndef WIN32
		case ESC:
#	ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
			switch (get_arrow_key (ch)) {
#endif /* !WIN32 */
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
#ifndef WIN32
				case KEYMAP_MOUSE:
					switch (xmouse) {
						case MOUSE_BUTTON_1:
							if (xrow < PAGE_HEADER || xrow >= cLINES-1)
								ch = iKeyPageDown;
							else
								ch = iKeyPageNextUnread;
							break;
						case MOUSE_BUTTON_2:
							if (xrow < PAGE_HEADER || xrow >= cLINES-1)
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
#endif /* !WIN32 */
		default:
			ch = map_to_default (ch, &menukeymap.page_nav);
			break;
	}
	return ch;
}


/*
 * The main routine for viewing articles
 * Returns:
 *    >=0	normal exit - return a new base[] note
 *    <0	indicates some unusual condition. See GRP_* in tin.h
 *			GRP_QUIT		User is doing a 'Q'
 *			GRP_RETURN		Back to selection level due to 'T' command
 *			GRP_ARTFAIL		We didn't make it into the art
 *							don't bother fixing the screen up
 *			GRP_GOTOTHREAD	To thread menu due to 'l' command
 *			GRP_NEXT		Catchup with 'c'
 *			GRP_NEXTUNREAD	   "      "  'C'
 */
int
show_page (
	struct t_group *group,	/* A bit useless since it is always CURR_GROUP */
	int start_respnum,		/* index into arts[] */
	int *threadnum)			/* to allow movement in thread mode */
{
	char buf[LEN];
	char group_path[LEN];
	char key[MAXKEYLEN];
	int ch, i, n = 0;
	int filter_state = NO_FILTERING;
	int old_sort_art_type = tinrc.sort_article_type;
	t_bool mouse_click_on = TRUE;

	filtered_articles = FALSE;	/* used in thread level */
	make_group_path (group->name, group_path);

	/*
	 * Peek to see if the pager started due to a body search
	 * Stop load_article() changing context again
	 */
	if (srch_lineno != -1)
		this_resp = start_respnum;

	if (load_article(start_respnum) < 0)
		return GRP_ARTFAIL;

	if (srch_lineno != -1)
		process_search();

	forever {
		switch ((ch = handle_pager_keypad())) {
#ifndef WIN32
			case ESC:       /* Abort */
				break;
#endif /* !WIN32 */

			case '0': case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				if (!HAS_FOLLOWUPS (which_thread (this_resp)))
					info_message (_(txt_no_responses));
				else {
					if ((n = prompt_response (ch, this_resp)) != -1) {
						if (load_article(n) < 0)
							return GRP_ARTFAIL;
					}
				}
				break;

#ifndef NO_SHELL_ESCAPE
			case iKeyShellEscape:
				shell_escape ();
				draw_page (group->name, 0);
				break;
#endif /* !NO_SHELL_ESCAPE */

			case iKeyMouseToggle:
				if (mouse_click_on)
					set_xclick_off ();
				else
					set_xclick_on ();
				mouse_click_on = !mouse_click_on;
				break;

			case iKeyPageUp:		/* page up */
			case iKeyPageUp2:
			case iKeyPageUp3:
				if (curr_line == 0)
					info_message (_(txt_begin_of_art));
				else {
					curr_line -= (tinrc.full_page_scroll) ? ARTLINES : ARTLINES/2;
					draw_page (group->name, 0);
				}
				break;

			case iKeyPageDown:		/* page down or next response */
			case iKeyPageDown2:
			case iKeyPageDown3:
			case iKeyPageNextUnread:
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
					info_message (_(txt_end_of_art));
				} else {
					if ((ch == iKeyPageNextUnread) && tinrc.tab_goto_next_unread)
						goto page_goto_next_unread;

					curr_line += (tinrc.full_page_scroll) ? ARTLINES : ARTLINES/2;

					if (tinrc.show_last_line_prev_page)
						curr_line--;
					draw_page (group->name, 0);
				}
				break;

page_goto_next_unread:
				if ((n = next_unread (next_response (this_resp))) == -1)
					return (which_thread (this_resp));
				if (load_article(n) < 0)
					return GRP_ARTFAIL;
				break;

			case iKeyFirstPage:		/* beginning of article */
			case iKeyPageFirstPage2:
				if (curr_line != 0) {
					curr_line = 0;
					draw_page (group->name, 0);
				}
				break;

			case iKeyLastPage:		/* end of article */
			case iKeyPageLastPage2:
				if (curr_line + ARTLINES != artlines) {
					/* Display a full last page for neatness */
					curr_line = artlines - ARTLINES;
					draw_page (group->name, 0);
				}
				break;

			case iKeyUp:		/* line up */
			case iKeyUp2:
				if (curr_line == 0) {
					info_message (_(txt_begin_of_art));
					break;
				}

				curr_line--;
				if (have_linescroll) {
					scroll_page (-1);
					draw_page (group->name, -1);
				} else
					draw_page (group->name, 0);
				break;

			case iKeyDown:		/* line down */
			case iKeyDown2:
				if (curr_line + ARTLINES >= artlines) {
					info_message (_(txt_end_of_art));
					break;
				}

				curr_line++;
				if (have_linescroll) {
					scroll_page (1);
					draw_page (group->name, 1);
				} else
					draw_page (group->name, 0);
				break;

			case iKeyLastViewed:	/* show last viewed article */
				if (last_resp < 0 || (which_thread(last_resp) == -1)) {
					info_message (_(txt_no_last_message));
					break;
				}
				if (load_article(last_resp) < 0)
					return GRP_ARTFAIL;
				break;

			case iKeyLookupMessage:			/* Goto article by Message-ID */

				if ((i = prompt_msgid ()) != ART_UNAVAILABLE) {
					if (load_article(i) < 0)
						return GRP_ARTFAIL;
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

				if (arts[parent->article].killed) {
					info_message(_(txt_art_parent_killed));
					break;
				}

				if (load_article(parent->article) < 0)
					return GRP_ARTFAIL;

				break;
			}

			case iKeyPipe:		/* pipe article/thread/tagged arts to command */
				feed_articles (FEED_PIPE, PAGE_LEVEL, group, this_resp);
				break;

			case iKeyPageMail:	/* mail article/thread/tagged articles to somebody */
				feed_articles (FEED_MAIL, PAGE_LEVEL, group, this_resp);
				break;

#ifndef DISABLE_PRINTING
			case iKeyPagePrint:	/* output art/thread/tagged arts to printer */
				feed_articles (FEED_PRINT, PAGE_LEVEL, group, this_resp);
				break;
#endif /* !DISABLE_PRINTING */

			case iKeyPageRepost:	/* repost current article */
				feed_articles (FEED_REPOST, PAGE_LEVEL, group, this_resp);
				break;

			case iKeyPageSave:	/* save article/thread/tagged articles */
				feed_articles (FEED_SAVE, PAGE_LEVEL, group, this_resp);
				break;

			case iKeyPageAutoSaveTagged:	/* Auto-save tagged articles without prompting */
				if (grpmenu.curr >= 0) {
					if (num_of_tagged_arts)
						feed_articles (FEED_AUTOSAVE_TAGGED, PAGE_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
					else
						info_message (_(txt_no_tagged_arts_to_save));
				}
				break;

			case iKeySearchSubjF:	/* search in article */
			case iKeySearchSubjB:
				if ((i = search_article ((ch == iKeySearchSubjF), search_line, artlines, artline, reveal_ctrl_l, note_fp)) == -1)
					break;

				process_search();
				break;

			case iKeySearchBody:	/* article body search */
				if ((n = search_body (this_resp)) != -1) {
					this_resp = n;			/* Stop load_article() changing context again */
					if (load_article(n) < 0)
						return GRP_ARTFAIL;
					process_search();
				}
				break;

			case iKeyPageTopThd:	/* first article in current thread */
				if (arts[this_resp].inthread) {
					if ((n = which_thread (this_resp)) >= 0 && base[n] != this_resp) {
						assert (n < grpmenu.max);
						if (load_article(base[n]) < 0)
							return GRP_ARTFAIL;
					}
				}
				break;

			case iKeyPageBotThd:	/* last article in current thread */
				for (i = this_resp; i >= 0; i = arts[i].thread)
					n = i;

				if (n != this_resp) {
					if (load_article(n) < 0)
						return GRP_ARTFAIL;
				}
				break;

			case iKeyPageNextThd:
			case iKeyPageNextThd2:	/* start of next thread */
				if ((n = next_thread (this_resp)) == -1)
					return (which_thread (this_resp));
				if (load_article(n) < 0)
					return GRP_ARTFAIL;
				break;

#ifdef HAVE_PGP_GPG
			case iKeyPagePGPCheckArticle:
				if (pgp_check_article(&pgart))
					draw_page (group->name, 0);
				break;
#endif /* HAVE_PGP_GPG */

			case iKeyPageToggleHeaders:	/* toggle display of whole 'raw' article */
				if (show_all_headers) {
					artline = pgart.cookl;
					artlines = pgart.cooked_lines;
					note_fp = pgart.cooked;
				} else {
					int j = 0;
					/*
					 * We do this on the fly, since most of the time it won't be used
					 */
					if (!pgart.rawl) {
						rewind (pgart.raw);
						/* Need one extra as the last \n in the file will be accounted too */
						pgart.rawl = my_malloc(sizeof(t_lineinfo) * (note_h->ext->line_count+1));
						do {
							pgart.rawl[j].offset = ftell(pgart.raw);
							pgart.rawl[j].flags = 0;
							j++;
						} while ((tin_fgets(pgart.raw, FALSE)) != NULL);
						pgart.rawl = my_realloc((char *)pgart.rawl, sizeof(t_lineinfo) * note_h->ext->line_count);
					}
					artline = pgart.rawl;
					artlines = note_h->ext->line_count;
					note_fp = pgart.raw;
				}
				curr_line = 0;
				show_all_headers = !show_all_headers;
				draw_page (group->name, 0);
				break;

			case iKeyPageToggleTex2iso:		/* toggle german TeX to ISO latin1 style conversion */
				if ((tex2iso_supported = !tex2iso_supported))
					pgart.tex2iso = is_art_tex_encoded (pgart.raw);

				resize_article (&pgart);	/* Also recooks it.. */
				draw_page (group->name, 0);
				info_message (_(txt_toggled_tex2iso), (tex2iso_supported) ? "on" : "off");
				break;

			case iKeyPageToggleTabs:		/* toggle tab stops 8 vs 4 */
				tabwidth = ((tabwidth == 8) ? 4 : 8);
				resize_article (&pgart);	/* Also recooks it.. */
				draw_page (group->name, 0);
				break;

			case iKeyPageToggleUue:			/* toggle display off uuencoded sections */
				hide_uue = !hide_uue;
				resize_article (&pgart);	/* Also recooks it.. */

				/*
				 * If we hid uue and are off the end of the article, reposition to
				 * show last page for neatness
				 */
				if (hide_uue && curr_line + ARTLINES > artlines)
					curr_line = artlines - ARTLINES;
				draw_page (group->name, 0);
				break;

			case iKeyPageReveal:			/* toggle hiding after ^L */
				reveal_ctrl_l = !reveal_ctrl_l;
				draw_page (group->name, 0);
				break;

			case iKeyPageQuickAutoSel:	/* quickly auto-select article */
			case iKeyPageQuickKill:		/* quickly kill article */
				if ((filtered_articles = quick_filter (
						(ch == iKeyPageQuickKill) ? FILTER_KILL : FILTER_SELECT,
						group, &arts[this_resp])))
					goto return_to_index;

				draw_page (group->name, 0);
				break;

			case iKeyPageAutoSel:		/* auto-select article menu */
			case iKeyPageAutoKill:		/* kill article menu */
				if (filter_menu ((ch == iKeyPageAutoKill) ? FILTER_KILL : FILTER_SELECT, group, &arts[this_resp])) {
					if ((filtered_articles = filter_articles (group)))
						goto return_to_index;
				}
				draw_page (group->name, 0);
				break;

			case iKeyPageEditFilter:
				if (!invoke_editor (filter_file, 25)) /* FIXME: is 25 correct offset? */
					break;
				unfilter_articles ();
				(void) read_filter_file (filter_file);
				if ((filtered_articles = filter_articles (group)))
					goto return_to_index;
				draw_page (group->name, 0);
				break;

			case iKeyRedrawScr:		/* redraw current page of article */
				my_retouch();
				draw_page (group->name, 0);
				break;

			case iKeyPageToggleRot:	/* toggle rot-13 mode */
				rotate = (rotate ? 0 : 13);
				draw_page (group->name, 0);
				info_message (_(txt_toggled_rot13));
				break;

			case iKeySearchAuthF:	/* author search forward */
			case iKeySearchAuthB:	/* author search backward */
				if ((n = search (SEARCH_AUTH, this_resp, (ch == iKeySearchAuthF))) < 0)
					break;
				if (load_article(n) < 0)
					return GRP_ARTFAIL;
				break;

			case iKeyPageCatchup:			/* catchup - mark read, goto next */
			case iKeyPageCatchupNextUnread:	/* goto next unread */
				snprintf(buf, sizeof(buf)-1, _(txt_mark_thread_read), (ch == iKeyPageCatchupNextUnread) ? _(txt_enter_next_thread) : "");
				if (!tinrc.confirm_action || prompt_yn (cLINES, buf, TRUE) == 1) {
					thd_mark_read (group, base[which_thread(this_resp)]);
					return (ch == iKeyPageCatchupNextUnread) ? GRP_NEXTUNREAD : GRP_NEXT;
				}
				break;

			case iKeyPageMarkThdUnread:
				thd_mark_unread (group, base[which_thread(this_resp)]);
				/* FIXME: replace 'Thread' by 'Article' if THREAD_NONE */
				info_message(_(txt_marked_as_unread), _("Thread"));
				break;

			case iKeyPageCancel:			/* cancel an article */
				if (can_post) {
					if (cancel_article (group, &arts[this_resp], this_resp))
						draw_page (group->name, 0);
				} else
					info_message (_(txt_cannot_post));
				break;

			case iKeyPageEditArticle:		/* edit an article (mailgroup only) */
				if (art_edit (group, &arts[this_resp]))
					draw_page (group->name, 0);
				break;

			case iKeyPageFollowupQuote:		/* post a followup to this article */
			case iKeyPageFollowupQuoteHeaders:
			case iKeyPageFollowup:
				if (!can_post) {
					info_message (_(txt_cannot_post));
					break;
				}
				(void) post_response (group->name, this_resp,
				  (ch == iKeyPageFollowupQuote || ch == iKeyPageFollowupQuoteHeaders) ? TRUE : FALSE,
				  ch == iKeyPageFollowupQuoteHeaders ? TRUE : FALSE);
				draw_page (group->name, 0);
				break;

			case iKeyHelp:	/* help */
				show_info_page (HELP_INFO, help_page, _(txt_art_pager_com));
				draw_page (group->name, 0);
				break;

			case iKeyToggleHelpDisplay:	/* toggle mini help menu */
				toggle_mini_help (PAGE_LEVEL);
				draw_page (group->name, 0);
				break;

			case iKeyQuit:	/* return to index page */
return_to_index:
				if (filter_state == NO_FILTERING && tinrc.sort_article_type != old_sort_art_type) {
					make_threads (group, TRUE);
				}

				i = which_thread (this_resp);
				if (threadnum)
					*threadnum = which_response (this_resp);

				if (filter_state == FILTERING || filtered_articles) {
					int old_top = top_art;
					long old_artnum = arts[this_resp].artnum;
					filter_articles (group);
					make_threads (group, FALSE);
					i = find_new_pos (old_top, old_artnum, i);
				}
				return i;

			case iKeyToggleInverseVideo:	/* toggle inverse video */
				toggle_inverse_video ();
				draw_page (group->name, 0);
				show_inverse_video_status ();
				break;

#ifdef HAVE_COLOR
			case iKeyToggleColor:		/* toggle color */
				if (toggle_color ()) {
					draw_page (group->name, 0);
					show_color_status ();
				}
				break;
#endif /* HAVE_COLOR */

			case iKeyPageListThd:	/* -> thread page that this article is in */
				fixup_thread (this_resp, FALSE);
				return GRP_GOTOTHREAD;

			case iKeyOptionMenu:	/* option menu */
				if (change_config_file (group) == FILTERING)
					filter_state = FILTERING;
				set_subj_from_size (cCOLS);
				draw_page (group->name, 0);
				break;

			case iKeyPageNextArt:	/* skip to next article */
				if ((n = next_response (this_resp)) == -1)
					return (which_thread(this_resp));

				if (load_article(n) < 0)
					return GRP_ARTFAIL;
				break;

			case iKeyPageKillThd:	/* mark rest of thread as read */
				thd_mark_read (group, this_resp);
				if ((n = next_unread (next_response (this_resp))) == -1)
					goto return_to_index;
				if (load_article(n) < 0)
					return GRP_ARTFAIL;
				break;

			case iKeyPageNextUnreadArt:	/* next unread article */
				goto page_goto_next_unread;

			case iKeyPagePrevArt:	/* previous article */
				if ((n = prev_response (this_resp)) == -1)
					return (this_resp);

				if (load_article(n) < 0)
					return GRP_ARTFAIL;
				break;

			case iKeyPagePrevUnreadArt:	/* previous unread article */
				if ((n = prev_unread (prev_response (this_resp))) == -1)
					info_message (_(txt_no_prev_unread_art));
				else {
					if (load_article(n) < 0)
						return GRP_ARTFAIL;
				}
				break;

			case iKeyQuitTin:	/* quit */
				return GRP_QUIT;

			case iKeyPageReplyQuote:	/* reply to author through mail */
			case iKeyPageReplyQuoteHeaders:
			case iKeyPageReply:
				mail_to_author (group->name, this_resp, (ch == iKeyPageReplyQuote || ch == iKeyPageReplyQuoteHeaders) ? TRUE : FALSE, ch == iKeyPageReplyQuoteHeaders ? TRUE : FALSE);
				draw_page (group->name, 0);
				break;

			case iKeyPageTag:	/* tag/untag article for saving */
				tag_article (this_resp);
				break;

			case iKeyPageGroupSel:	/* return to group selection page */
				if (filter_state == FILTERING) {
					filter_articles (group);
					make_threads (group, FALSE);
				}
				return GRP_RETURN;

			case iKeyVersion:
				info_message (cvers);
				break;

			case iKeyPost:	/* post a basenote */
				if (post_article (group->name))
					draw_page (group->name, 0);
				break;

			case iKeyPostponed:
			case iKeyPostponed2:	/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles (FALSE, FALSE))
						draw_page (group->name, 0);
				} else
					info_message(_(txt_cannot_post));
				break;

			case iKeyDisplayPostHist:	/* display messages posted by user */
				if (user_posted_messages ())
					draw_page (group->name, 0);
				break;

			case iKeyPageMarkArtUnread:	/* mark article as unread (to return) */
				art_mark_will_return (group, &arts[this_resp]);
				info_message (_(txt_marked_as_unread), _("Article"));
				break;

			case iKeyPageSkipIncludedText:	/* skip included text */
				for (i=curr_line; i < artlines; i++) {
					if (!(artline[i].flags & (C_QUOTE1|C_QUOTE2|C_QUOTE3)))
						break;
				}

				if (i != curr_line) {
					curr_line = i;
					draw_page (group->name, 0);
				}
				break;

			case iKeyToggleInfoLastLine: /* this is _not_ correct, we do not toggle status here */
				info_message ("%s", arts[this_resp].subject);
				break;

#ifdef HAVE_COLOR
			case iKeyPageToggleHighlight:
				if (use_color) { /* make sure we have color turned on */
					word_highlight = !word_highlight;
					draw_page (group->name, 0);
					info_message(_(txt_toggled_high), (word_highlight) ? "on" : "off");
				}
				break;
#endif /* HAVE_COLOR */

			case iKeyPageViewAttach:
				decode_save_mime (&pgart, FALSE);
				break;

			case iKeyPageViewUrl:
				process_url();
				break;

			default:
				info_message(_(txt_bad_command), printascii (key, map_to_local (iKeyHelp, &menukeymap.page_nav)));
		}
	}
	/* NOTREACHED */
	return GRP_ARTFAIL; /* default-value - I don't think we should get here */
}


/*
 * Redraw the current page, curr_line will be the first line displayed
 * Everything that calls draw_page() just sets curr_line, this function must
 * ensure it is set to something sane
 * If part is !=0, then only draw the first (-ve) or last (+ve) few lines
 */
void
draw_page (
	const char *group,
	int part)
{
	char *buff;
	int i;
	int end;	/* last line to draw */

	signal_context = cPage;

	/*
	 * Ensure curr_line is in bounds
	 */
	if (curr_line < 0)
		curr_line = 0;			/* Oops - off the top */
	if (curr_line > artlines)
		curr_line = artlines;	/* Oops - off the end */

	search_line = curr_line;	/* Reset search to start from top of display */

	buff = my_malloc(cCOLS+1);	/* Need to account for \n */

	if (part == 0) {
		ClearScreen();
		show_first_header (group);
	} else
		MoveCursor (0, 0);

	/* Down-scroll, only redraw bottom 'part' lines of screen */
	i = (part > 0) ? ARTLINES-part : 0;

	/* Up-scroll, only redraw the top 'part' lines of screen */
	end = (part < 0) ? -part : ARTLINES;

	for (; i < end; i++) {
		t_lineinfo *curr;

		if (curr_line + i >= artlines)		/* ran out of article */
			break;

		curr = &artline[curr_line+i];
		fseek (note_fp, curr->offset, SEEK_SET);

		fgets (buff, cCOLS+1, note_fp);

		/*
		 * rotN encoding on body and sig data only
		 */
		if ((rotate != 0) && (curr->flags & (C_BODY | C_SIG))) {
			char *p = buff;

			for (p=buff; *p; p++) {
				if (*p >= 'A' && *p <= 'Z')
					*p = (*p - 'A' + rotate) % 26 + 'A';
				else if (*p >= 'a' && *p <= 'z')
					*p = (*p - 'a' + rotate) % 26 + 'a';
			}
		}

		strip_line(buff);

#ifndef USE_CURSES
		snprintf (screen[i+PAGE_HEADER].col, cCOLS, "%s" cCRLF, buff);
#endif /* !USE_CURSES */

		MoveCursor (i+PAGE_HEADER, 0);
		draw_pager_line (buff, curr->flags);

		/*
		 * Highlight URL's and mail addresses
		 */
		if (curr->flags & C_URL)
			highlight_regexes (i+PAGE_HEADER, &url_regex);

		if (curr->flags & C_MAIL)
			highlight_regexes (i+PAGE_HEADER, &mail_regex);

		/* Blank the screen after a ^L (only occurs when showing cooked) */
		if (!reveal_ctrl_l && (curr->flags & C_CTRLL)) {
			CleartoEOS();
			break;
		}
	}

#ifdef HAVE_COLOR
	fcol(tinrc.col_text);
#endif /* HAVE_COLOR */

	free(buff);

	show_mini_help (PAGE_LEVEL);

	/*
	 * Print an appropriate footer
	 */
	if (curr_line + ARTLINES >= artlines) {
		clear_message();
		MoveCursor (cLINES, MORE_POS - (5 + BLANK_PAGE_COLS));
		StartInverse ();

		my_fputs (((arts[this_resp].thread != -1) ? _(txt_next_resp) : _(txt_last_resp)), stdout);
		my_flush ();
		EndInverse ();
	} else
		draw_percent_mark (curr_line + ARTLINES, artlines);

	stow_cursor();
}


#ifdef HAVE_METAMAIL
static void
show_mime_article (
	FILE *fp)
{
	FILE *mime_fp;
	char *mm;
	char buf[PATH_LEN];
	long offset;

	offset = ftell (fp);
	rewind (fp);

	if ((mm = getenv("METAMAIL"))) {
		if (strcmp (mm, "(internal)") == 0) {	/* Special hack - use internal viewer */
			draw_page (CURR_GROUP.name, 0);
			decode_save_mime (&pgart, FALSE);
			return;
		}
		snprintf (buf, sizeof(buf)-1, mm);
	} else
		snprintf (buf, sizeof(buf)-1, METAMAIL_CMD, PATH_METAMAIL);

	EndWin();
	Raw(FALSE);
	if ((mime_fp = popen (buf, "w"))) {
		while (fgets (buf, (int) sizeof(buf), fp) != (char *) 0)
			fputs (buf, mime_fp);

		fflush (mime_fp);
		pclose (mime_fp);
	} else
		info_message (_(txt_error_metamail_failed), strerror(errno));

	Raw(TRUE);
	InitWin ();
	prompt_continue ();

	/* This is redundant, but harmless, unless we are viewing the raw art */
	fseek (fp, offset, SEEK_SET);	/* goto old position */

	MoveCursor (cLINES, MORE_POS-(5+BLANK_PAGE_COLS));
	StartInverse ();

	my_flush ();
	EndInverse ();
}
#endif /* HAVE_METAMAIL */


/*
 * PAGE_HEADER defines the size in lines of this header
 */
static void
show_first_header (
	const char *group)
{
	char buf[HEADER_LEN];
	char tmp[LEN];
	int whichresp;
	int x_resp;
	int pos, i, n;
	int grplen, maxlen;

	whichresp = which_response (this_resp);
	x_resp = num_of_responses (which_thread (this_resp));

	if (!my_strftime (buf, sizeof (buf), "%a, %d %b %Y %H:%M:%S",
							localtime (&arts[this_resp].date)))
		strcpy (buf, BlankIfNull(note_h->date));

	/*
	 * Work out how much room we have for group name, allow 1 space before and
	 * after it
	 */
	grplen = strlen(group);
	maxlen = RIGHT_POS - strlen(buf) - 2;

	if (grplen < maxlen)
		maxlen = grplen;

	/*
	 * Aesthetics - Add 3 to compensate for the fact that
	 * the left hand margin (date) is longer than the right hand margin
	 */
	pos = 3 + (cCOLS - maxlen) / 2;

	for (i = strlen(buf); i < pos; i++)		/* Pad out to left */
		buf[i] = ' ';

	buf[i] = '\0';

	if (maxlen != grplen) {					/* ie groupname was too long */
		strncat (buf, group, maxlen-3);
		strcat (buf, "...");
	} else
		strncat (buf, group, maxlen);

	for (i = strlen(buf); i < RIGHT_POS; i++)	/* Pad out to right */
		buf[i] = ' ';

	buf[i] = '\0';

#ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#endif /* HAVE_COLOR */

	/* Displaying the value of X-Comment-To header in the upper right corner */
	if (note_h->ftnto && tinrc.show_xcommentto) {
		char ftbuf[HEADER_LEN]; /* FTN-To aka X-Comment-To */
		my_fputs (buf, stdout);
		strip_address(note_h->ftnto, ftbuf);
		ftbuf[19] = '\0';
		convert_to_printable (ftbuf);
		StartInverse ();
		my_fputs (ftbuf, stdout);
		EndInverse ();
		my_fputs (cCRLF, stdout);
	} else {
		char x[5];

		/* Can't eval tin_ltoa() more than once in a statement due to statics */
		strcpy(x, tin_ltoa(which_thread(this_resp) + 1, 4));

		sprintf (tmp, _(txt_thread_x_of_n), buf, x, tin_ltoa(grpmenu.max, 4), cCRLF);
		my_fputs (tmp, stdout);
	}

	/*
	 * An accurate line count will appear in the footer anyway
	 */
	if (arts[this_resp].line_count < 0)
		strcpy (tmp, "?");
	else
		sprintf (tmp, "%-4d", arts[this_resp].line_count);

#ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#endif /* HAVE_COLOR */

	sprintf (buf, _(txt_lines), tmp);
	n = strlen (buf);
	my_fputs (buf, stdout);

#ifdef HAVE_COLOR
	fcol(tinrc.col_subject);
#endif /* HAVE_COLOR */

	if (pgart.tex2iso) {
		*buf = '\0';
		strcpy (buf, "TeX ");
		n += strlen (buf);
		my_fputs (buf, stdout);
	}

	strncpy (buf, (note_h->subj ? note_h->subj : arts[this_resp].subject), HEADER_LEN - 1);

	buf[RIGHT_POS - 5 - n] = '\0';

	pos = ((cCOLS - (int) strlen (buf)) / 2) - 2;

	MoveCursor (1, ((pos > n) ? pos : n));

	convert_to_printable (buf);

	StartInverse ();
	my_fputs (buf, stdout);
	EndInverse ();

#ifdef HAVE_COLOR
	fcol(tinrc.col_response);
#endif /* HAVE_COLOR */

	MoveCursor (1, RIGHT_POS);
	if (whichresp)
		my_printf (_(txt_resp_x_of_n), whichresp, x_resp, cCRLF);
	else {
		if (!x_resp)
			my_printf (_(txt_no_resp), cCRLF);
		else if (x_resp == 1)
			my_printf (_(txt_1_resp), cCRLF);
		else
			my_printf (_(txt_x_resp), x_resp, cCRLF);
	}

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */

	if (arts[this_resp].name)
		sprintf (buf, "%s <%s>", arts[this_resp].name, arts[this_resp].from);
	else
		strncpy (buf, arts[this_resp].from, cCOLS-1);
	buf[cCOLS-1] = '\0';

	if (note_h->org) {
		sprintf (tmp, _(txt_at_s), note_h->org);
		tmp[sizeof(tmp)-1] = '\0';

		if ((int) strlen (buf) + (int) strlen (tmp) >= cCOLS -1) {
			strncat (buf, tmp, cCOLS - 1 - strlen(buf));
			buf[cCOLS-1] = '\0';
		} else {
			pos = cCOLS - 1 - (int) strlen(tmp);
			for (i = strlen(buf); i < pos; i++)
				buf[i] = ' ';
			buf[i] = '\0';
			strcat (buf, tmp);
		}
	}

	strip_line (buf);

	convert_to_printable (buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_from);
#endif /* HAVE_COLOR */

	my_printf ("%s" cCRLF cCRLF, buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
}


/*
 * Change the pager article context to arts[new_respnum]
 * Return GRP_ARTFAIL if article could not be opened
 */
static int
load_article(
	int new_respnum)
{
	char group_path[LEN];

	if (read_news_via_nntp)
		wait_message (0, _(txt_reading_article));

#ifdef DEBUG
	fprintf(stderr, "load_art %s(new=%d, curr=%d)\n", (new_respnum == this_resp) ? "ALREADY OPEN!":"", new_respnum, this_resp);
#endif /* DEBUG */

	if (new_respnum == this_resp) {
#ifdef DEBUG
		fprintf(stderr, "ART %d already open\n", new_respnum);
#endif /* DEBUG */
		goto already_open;
	}

	art_close (&pgart);			/* close previously opened art in pager */

	make_group_path (CURR_GROUP.name, group_path);

	switch (art_open (&arts[new_respnum], group_path, &pgart)) {

		case ART_UNAVAILABLE:
			art_mark_read (&CURR_GROUP, &arts[new_respnum]);
			wait_message (1, _(txt_art_unavailable));
			nobreak;	/* FALLTHROUGH */

		case ART_ABORT:
			return GRP_ARTFAIL;	/* special retcode to stop redrawing screen */

		default:					/* Normal case */
#if 0
			if (prompt_yn(cLINES, "Fake art unavailable ? ", FALSE) == 1) {
				art_close(&pgart);
				art_mark_read (&CURR_GROUP, &arts[new_respnum]);
				return GRP_ARTFAIL;
			}
#endif /* 0 */
			/*
			 * Remember current & previous articles for '-' command
			 */
			last_resp = this_resp;
			this_resp = new_respnum;		/* Set new art globally */

			break;
	}

already_open:
	art_mark_read (&CURR_GROUP, &arts[this_resp]);

	/*
	 * Setup to start viewing cooked version
	 */
	show_all_headers = FALSE;
	curr_line = 0;
	note_fp = pgart.cooked;
	artline = pgart.cookl;
	artlines = pgart.cooked_lines;
	search_line = -1;

	rotate = 0;			/* normal mode, not rot13 */
	reveal_ctrl_l = FALSE;
	hide_uue = tinrc.hide_uue;

	/*
	 * Automatically invoke attachment viewing if requested
	 */
#ifdef HAVE_METAMAIL
	if (tinrc.use_metamail && note_h->mime && !(IS_PLAINTEXT(note_h->ext))) {
		if (tinrc.ask_for_metamail) {
			draw_page (CURR_GROUP.name, 0);
			if (prompt_yn (cLINES, _(txt_use_mime), TRUE) == 1)
				show_mime_article (pgart.raw);
		} else
			show_mime_article (pgart.raw);
	}
#endif /* HAVE_METAMAIL */

	draw_page (CURR_GROUP.name, 0);

	return 0;
}


static int
prompt_response (
	int ch,
	int curr_respnum)
{
	int num;

	clear_message ();

	if ((num = prompt_num (ch, _(txt_read_resp))) == -1) {
		clear_message ();
		return -1;
	}

	return find_response (which_thread (curr_respnum), num);
}


/*
 * Reposition within art as needed, highlight searched string
 * This is tied quite closely to the information stored by
 * get_search_vectors()
 */
static void
process_search(
	void)
{
	int i, start, end;

	if ((i = get_search_vectors (&start, &end)) == -1)
		return;

	/*
	 * Is matching line off the current view ?
	 * Reposition within article if needed, try to get matched line
	 * in the middle of the screen
	 */
	if (i < curr_line || i >= curr_line+ARTLINES) {
		int ideal_pos = i - (ARTLINES / 2);

		if (ideal_pos + ARTLINES > artlines)		/* Off the end */
			curr_line = artlines - ARTLINES;
		else										/* Pos is just fine */
			curr_line = ideal_pos;
	}

	draw_page (CURR_GROUP.name, 0);
	search_line = i;								/* draw_page() resets this to 0 */

	/*
	 * Highlight found string
	 */
	highlight_string (i - curr_line + HDR_ADJUST + 2, start, end - start);
}


static void
process_url(
	void)
{
	char buf[LEN];
	char *ptr;
	int i;
	int offsets[6];
	int offsets_size = sizeof(offsets)/sizeof(int);

	for (i = curr_line; i < artlines; ++i) {
		if (!artline[i].flags & C_URL)
			continue;

		/*
		 * Line contains a URL, so read it in
		 */
		fseek(pgart.cooked, artline[i].offset, SEEK_SET);
		ptr = fgets (buf, sizeof(buf), pgart.cooked);

		/*
		 * Step through, finding URL's
		 */
		while (pcre_exec (url_regex.re, url_regex.extra, ptr, strlen(ptr), 0, 0, offsets, offsets_size) != PCRE_ERROR_NOMATCH) {
			char url[LEN];

			*(ptr+offsets[1]) = '\0';
			if (prompt_default_string ("URL:", url, sizeof(url), ptr+offsets[0], HIST_NONE)) {
				char ubuf[LEN];

				wait_message(2, "Launching %s\n", url);
				strcpy(ubuf, "/usr/local/bin/url_handler.sh ");
				strncat(ubuf, url, LEN-32);
				invoke_cmd (ubuf);
			}

			ptr += offsets[1];
		}
	}

	info_message ("No more URL's");
}


/*
 * Re-cook an article
 */
void
resize_article(
	t_openartinfo *artinfo)
{
	free(artinfo->cookl);
	if (artinfo->cooked)
		fclose(artinfo->cooked);

	cook_article (artinfo, tabwidth, hide_uue);

	show_all_headers = FALSE;
	artline = pgart.cookl;
	artlines = pgart.cooked_lines;
	note_fp = pgart.cooked;
}
