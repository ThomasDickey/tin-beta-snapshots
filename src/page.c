/*
 *  Project   : tin - a Usenet reader
 *  Module    : page.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2025-06-04
 *  Notes     :
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifdef HAVE_LIBURIPARSER
#	include <uriparser/Uri.h>
#else
#	ifdef HAVE_LIBCURL
#		include <curl/curl.h>
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */


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

static t_url *url_list;

t_openartinfo pgart =	/* Global context of article open in the pager */
	{
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL },
		FALSE, 0,
		NULL, NULL, NULL, NULL, NULL,
	};

int last_resp;			/* previous & current article # in arts[] for '-' command */
int this_resp;

size_t tabwidth = 8;

static struct t_header *note_h = &pgart.hdr;	/* Easy access to article headers */

static FILE *info_file;
static const char *info_title;
static int curr_info_line;
static int hide_inline_data;	/* set when inline data sections are 'hidden' */
static int num_info_lines;
static int reveal_ctrl_l_lines;	/* number of lines (from top) with de-activated ^L */
static int rotate;				/* 0=normal, 13=rot13 decode */
static int scroll_region_top;	/* first screen line for displayed message */
static int search_line;			/* Line to commence next search from */
static void show_article_info_page(void);
static t_lineinfo *infoline = (t_lineinfo *) 0;

static t_bool show_all_headers;	/* all headers <-> headers in news_headers_to[_not]_display */
static t_bool show_raw_article;	/* CTRL-H raw <-> cooked article */
static t_bool reveal_ctrl_l;	/* set when ^L hiding is off */

/*
 * Local prototypes
 */
static char *build_from_line(void);
static int build_url_list(void);
static int load_article(int new_respnum, struct t_group *group);
static int prompt_response(int ch, int curr_respnum);
static int scroll_page(int dir);
static t_bool deactivate_next_ctrl_l(void);
static t_bool activate_last_ctrl_l(void);
static t_bool process_url(int n);
static t_bool url_page(void);
static t_function page_left(void);
static t_function page_right(void);
static t_function page_mouse_action(t_function (*left_action) (void), t_function (*right_action) (void));
static t_function url_left(void);
static t_function url_right(void);
static void build_url_line(int i);
static void draw_page_header(t_bool tex2iso_conv);
static void draw_percent_mark(long cur_num, long max_num);
static void draw_url_arrow(void);
static void free_url_list(void);
static void preprocess_info_message(FILE *info_fh);
static void print_message_page(FILE *file, t_lineinfo *messageline, size_t messagelines, size_t base_line, size_t begin, size_t end, int help_level);
static void process_search(int *lcurr_line, size_t message_lines, size_t screen_lines, int help_level);
static void show_url_page(void);
static void invoke_metamail(FILE *fp);

static t_menu urlmenu = { 0, 0, 0, show_url_page, draw_url_arrow, build_url_line };

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
				--i;
				break;

			case -2:
				i >>= 1;
				break;

			default: /* should not happen */
				/* CONSTANTCONDITION */
				assert(0 != 0);
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
	for (i = reveal_ctrl_l_lines + 1; i < end; i++) {
		if (artline[i].flags & C_CTRLL) {
			reveal_ctrl_l_lines = i;
			return TRUE;
		}
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
	for (i = reveal_ctrl_l_lines; i >= curr_line; i--) {
		if (artline[i].flags & C_CTRLL) {
			reveal_ctrl_l_lines = i - 1;
			return TRUE;
		}
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
	int i, j, n = 0;
	int art_type = GROUP_TYPE_NEWS;
	int hide_inline_data_tmp;
	t_artnum old_artnum;
	t_bool mouse_click_on = TRUE;
	t_bool repeat_search;
	t_function func;

	if (group->attribute->mailing_list && *group->attribute->mailing_list)
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
		process_search(&curr_line, (size_t) artlines, (size_t) ARTLINES, PAGE_LEVEL);

	forever {
		if ((func = handle_keypad(page_left, page_right, page_mouse_action, page_keys)) == GLOBAL_SEARCH_REPEAT) {
			func = last_search;
			repeat_search = TRUE;
		} else
			repeat_search = FALSE;

		switch (func) {
			case GLOBAL_ABORT:	/* Abort */
				break;

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
				draw_page(0);
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
					draw_page(0);
				else {
					if (curr_line == 0)
						info_message(_(txt_begin_of_art));
					else {
						curr_line -= ((tinrc.scroll_lines == -2) ? ARTLINES / 2 : ARTLINES);
						draw_page(0);
					}
				}
				break;

			case GLOBAL_PAGE_DOWN:		/* page down or next response */
			case PAGE_NEXT_UNREAD:
				if (!((func == PAGE_NEXT_UNREAD) && (tinrc.goto_next_unread & GOTO_NEXT_UNREAD_TAB)) && deactivate_next_ctrl_l())
					draw_page(0);
				else {
					if (curr_line + ARTLINES >= artlines) {	/* End is already on screen */
						switch (func) {
							case PAGE_NEXT_UNREAD:	/* <TAB> */
								goto page_goto_next_unread;

							case GLOBAL_PAGE_DOWN:
								if (tinrc.goto_next_unread & GOTO_NEXT_UNREAD_PGDN)
									goto page_goto_next_unread;
								break;

							default:		/* to keep gcc quiet */
								break;
						}
						info_message(_(txt_end_of_art));
					} else {
						if ((func == PAGE_NEXT_UNREAD) && (tinrc.goto_next_unread & GOTO_NEXT_UNREAD_TAB))
							goto page_goto_next_unread;

						curr_line += ((tinrc.scroll_lines == -2) ? ARTLINES / 2 : ARTLINES);

						if (tinrc.scroll_lines == -1)		/* formerly show_last_line_prev_page */
							--curr_line;
						draw_page(0);
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
					draw_page(0);
				}
				break;

			case GLOBAL_LAST_PAGE:		/* end of article */
				if (reveal_ctrl_l_lines < artlines - 1 || curr_line + ARTLINES != artlines) {
					reveal_ctrl_l_lines = artlines - 1;
					/* Display a full last page for neatness */
					curr_line = artlines - ARTLINES;
					draw_page(0);
				}
				break;

			case GLOBAL_LINE_UP:
				if (activate_last_ctrl_l())
					draw_page(0);
				else {
					if (curr_line == 0) {
						info_message(_(txt_begin_of_art));
						break;
					}

					i = scroll_page(KEYMAP_UP);
					curr_line += i;
					draw_page(i);
				}
				break;

			case GLOBAL_LINE_DOWN:
				if (deactivate_next_ctrl_l())
					draw_page(0);
				else {
					if (curr_line + ARTLINES >= artlines) {
						info_message(_(txt_end_of_art));
						break;
					}

					i = scroll_page(KEYMAP_DOWN);
					curr_line += i;
					draw_page(i);
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
#ifdef NNTP_ABLE
				else {
					const char *eyde = input_history[HIST_MESSAGE_ID][(hist_pos[HIST_MESSAGE_ID] - 1 + HIST_SIZE) % HIST_SIZE];
					int tr = this_resp;

					if (*eyde) {
						switch (show_article_by_msgid(eyde)) {
							case LOOKUP_OK:
								art_close(&pgart);
								this_resp = tr;
								if (!index_group(curr_group))
									return GRP_RETSELECT;
								if (!art_open(TRUE, &arts[this_resp], group, &pgart, TRUE, _(txt_reading_article))) {
									if ((i = load_article(this_resp, group)) < 0)
										return i;
								} else {
									XFACE_CLEAR(); /* TODO: needed? */
									return ART_UNAVAILABLE;
								}
								break;

							case LOOKUP_UNAVAIL: /* TODO: check for !reading_saved_news for better error-message? */
								wait_message(2, _(txt_lookup_func_not_available));
								break;

							default:
								wait_message(2, _(txt_art_unavailable));
								break;
						}
					}
				}
#endif /* NNTP_ABLE */
				break;

			case PAGE_GOTO_PARENT:		/* Goto parent of this article */
			{
				const struct t_msgid *parent = arts[this_resp].refptr->parent;

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
				feed_articles(FEED_PIPE, PAGE_LEVEL, NOT_ASSIGNED, group, this_resp);
				XFACE_SHOW();
				break;

			case PAGE_MAIL:	/* mail article/thread/tagged articles to somebody */
				XFACE_SUPPRESS();
				feed_articles(FEED_MAIL, PAGE_LEVEL, NOT_ASSIGNED, group, this_resp);
				XFACE_SHOW();
				break;

#ifndef DISABLE_PRINTING
			case GLOBAL_PRINT:	/* output art/thread/tagged arts to printer */
				XFACE_SUPPRESS();
				feed_articles(FEED_PRINT, PAGE_LEVEL, NOT_ASSIGNED, group, this_resp);
				XFACE_SHOW();
				break;
#endif /* !DISABLE_PRINTING */

			case PAGE_REPOST:	/* repost current article */
				if (can_post) {
					XFACE_SUPPRESS();
					feed_articles(FEED_REPOST, PAGE_LEVEL, NOT_ASSIGNED, group, this_resp);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case PAGE_SAVE:	/* save article/thread/tagged articles */
				XFACE_SUPPRESS();
				feed_articles(FEED_SAVE, PAGE_LEVEL, NOT_ASSIGNED, group, this_resp);
				XFACE_SHOW();
				break;

			case PAGE_AUTOSAVE:	/* Auto-save articles without prompting */
				if (grpmenu.curr >= 0) {
					XFACE_SUPPRESS();
					feed_articles(FEED_AUTOSAVE, PAGE_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
					XFACE_SHOW();
				}
				break;

			case GLOBAL_SEARCH_REPEAT:
				info_message(_(txt_no_prev_search));
				break;

			case GLOBAL_SEARCH_SUBJECT_FORWARD:	/* search in article */
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
				if (search_article((func == GLOBAL_SEARCH_SUBJECT_FORWARD), repeat_search, search_line, artlines, artline, reveal_ctrl_l_lines, note_fp) == -1)
					break;

				if (func == GLOBAL_SEARCH_SUBJECT_BACKWARD && !reveal_ctrl_l) {
					reveal_ctrl_l_lines = curr_line + ARTLINES - 1;
					draw_page(0);
				}
				process_search(&curr_line, (size_t) artlines, (size_t) ARTLINES, PAGE_LEVEL);
				break;

			case GLOBAL_SEARCH_BODY:	/* article body search */
				if ((n = search_body(group, this_resp, repeat_search)) != -1) {
					this_resp = n;			/* Stop load_article() changing context again */
					if ((i = load_article(n, group)) < 0) {
						XFACE_CLEAR();
						return i;
					}
					process_search(&curr_line, (size_t) artlines, (size_t) ARTLINES, PAGE_LEVEL);
				}
				break;

			case PAGE_TOP_THREAD:	/* first article in current thread */
				if (arts[this_resp].prev >= 0) {
					n = which_thread(this_resp);
					if (n >= 0 && base[n] != this_resp) {
						assert(n < grpmenu.max);
						if ((i = load_article((int) base[n], group)) < 0) {
							XFACE_CLEAR();
							return i;
						}
					}
				}
				break;

			case PAGE_BOTTOM_THREAD:	/* last article in current thread */
				for (i = this_resp; i >= 0; i = arts[i].thread)
					n = i;

				if (n != this_resp && n >= 0) {
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
					draw_page(0);
				XFACE_SHOW();
				break;
#endif /* HAVE_PGP_GPG */

			case PAGE_TOGGLE_HEADERS:	/* toggle display of all headers */
				XFACE_CLEAR();
				show_all_headers = bool_not(show_all_headers);
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				curr_line = 0;
				draw_page(0);
				break;

			case PAGE_TOGGLE_RAW:	/* toggle display of whole 'raw' article */
				XFACE_CLEAR();
				toggle_raw();
				break;

			case PAGE_TOGGLE_TEX2ISO:		/* toggle German TeX to ISO latin1 style conversion */
				if ((group->attribute->tex2iso_conv = bool_not(group->attribute->tex2iso_conv)))
					;
				else
					pgart.tex2iso = FALSE;

				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(0);
				info_message(_(txt_toggled_tex2iso), txt_onoff[group->attribute->tex2iso_conv != FALSE ? 1 : 0]);
				break;

			case PAGE_TOGGLE_TABS:		/* toggle tab stops 8 vs 4 */
				tabwidth = (tabwidth == 8) ? 4 : 8;
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(0);
				info_message(_(txt_toggled_tabwidth), tabwidth);
				break;

			case PAGE_TOGGLE_INLINE_DATA:	/* toggle display of inline data sections */
				do {
					hide_inline_data = (hide_inline_data + 1) % (HIDE_ALL + 1);
				} while (!*txt_hide_inline_data_type[hide_inline_data]); /* a list with gaps? - skip over them */
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				/*
				 * If we hid uue and are off the end of the article, reposition to
				 * show last page for neatness
				 */
				if (hide_inline_data && curr_line + ARTLINES > artlines)
					curr_line = artlines - ARTLINES;
				draw_page(0);
				info_message("%s: %s", _(txt_hide_inline_data.opt), _(txt_hide_inline_data_type[hide_inline_data]));
				break;

			case PAGE_TOGGLE_VERBATIM:			/* toggle verbatim options */
				if (++group->attribute->verbatim_handling > VERBATIM_HIDE_ALL)
					group->attribute->verbatim_handling = VERBATIM_NONE;
				resize_article(TRUE, &pgart);	/* Also recooks it.. */
				draw_page(0);
				info_message(_(txt_toggled_verbatim), txt_verbatim_handling_options[group->attribute->verbatim_handling]);
				break;

			case PAGE_REVEAL:			/* toggle hiding after ^L */
				reveal_ctrl_l = bool_not(reveal_ctrl_l);
				if (!reveal_ctrl_l) {	/* switched back to active ^L's */
					reveal_ctrl_l_lines = -1;
					curr_line = 0;
				} else
					reveal_ctrl_l_lines = artlines - 1;
				draw_page(0);
				/* TODO: info_message()? */
				break;

			case GLOBAL_QUICK_FILTER_SELECT:	/* quickly auto-select article */
			case GLOBAL_QUICK_FILTER_KILL:		/* quickly kill article */
				if (quick_filter(func, group, &arts[this_resp])) {
					old_artnum = arts[this_resp].artnum;
					unfilter_articles(group);
					filter_articles(group);
					make_threads(group, FALSE);
					if ((n = find_artnum(old_artnum)) == -1 || which_thread(n) == -1) /* We have lost the thread */
						return GRP_KILLED;
					this_resp = n;
					draw_page(0);
					info_message((func == GLOBAL_QUICK_FILTER_KILL) ? _(txt_info_add_kill) : _(txt_info_add_select));
				}
				break;

			case GLOBAL_MENU_FILTER_SELECT:		/* auto-select article menu */
			case GLOBAL_MENU_FILTER_KILL:			/* kill article menu */
				XFACE_CLEAR();
				if (filter_menu(func, group, &arts[this_resp])) {
					old_artnum = arts[this_resp].artnum;
					unfilter_articles(group);
					filter_articles(group);
					make_threads(group, FALSE);
					if ((n = find_artnum(old_artnum)) == -1 || which_thread(n) == -1) /* We have lost the thread */
						return GRP_KILLED;
					this_resp = n;
				}
				draw_page(0);
				break;

			case GLOBAL_EDIT_FILTER:
				if (no_write)
					info_message(_(txt_info_no_write));
				else {
					XFACE_CLEAR();
					if (invoke_editor(filter_file, filter_file_offset, NULL)) {
						old_artnum = arts[this_resp].artnum;
						unfilter_articles(group);
						if (read_filter_file(filter_file))
							filter_articles(group);
						make_threads(group, FALSE);
						if ((n = find_artnum(old_artnum)) == -1 || which_thread(n) == -1) /* We have lost the thread */
							return GRP_KILLED;
						this_resp = n;
					}
					draw_page(0);
				}
				break;

			case GLOBAL_REDRAW_SCREEN:		/* redraw current page of article */
				my_retouch();
				draw_page(0);
				break;

			case PAGE_TOGGLE_ROT13:	/* toggle rot-13 mode */
				rotate = rotate ? 0 : 13;
				draw_page(0);
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
				if (group->attribute->thread_articles == THREAD_NONE)
					snprintf(buf, sizeof(buf), _(txt_mark_art_read), (func == CATCHUP_NEXT_UNREAD) ? _(txt_enter_next_unread_art) : "");
				else
					snprintf(buf, sizeof(buf), _(txt_mark_thread_read), (func == CATCHUP_NEXT_UNREAD) ? _(txt_enter_next_thread) : "");
				if ((!TINRC_CONFIRM_ACTION) || prompt_yn(buf, TRUE) == 1) {
					thd_mark_read(group, base[which_thread(this_resp)]);
					XFACE_CLEAR();
					return (func == CATCHUP_NEXT_UNREAD) ? GRP_NEXTUNREAD : GRP_NEXT;
				}
				break;

			case MARK_THREAD_UNREAD:
				thd_mark_unread(group, base[which_thread(this_resp)]);
				if (group->attribute->thread_articles != THREAD_NONE)
					info_message(_(txt_marked_thread_as_unread));
				else
					info_message(_(txt_marked_article_as_unread));
				break;

			case PAGE_CANCEL:			/* cancel an article */
				if (can_post || art_type != GROUP_TYPE_NEWS) {
					XFACE_SUPPRESS();
					if (cancel_article(group, &arts[this_resp], this_resp))
						draw_page(0);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case PAGE_EDIT_ARTICLE:		/* edit an article (mailgroup only) */
				XFACE_SUPPRESS();
				if (art_edit(group, &arts[this_resp]))
					draw_page(0);
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
				  func == PAGE_FOLLOWUP_QUOTE_HEADERS ? TRUE : FALSE, show_raw_article);
				draw_page(0);
				break;

			case GLOBAL_HELP:	/* help */
				XFACE_CLEAR();
				show_help_page(PAGE_LEVEL, _(txt_art_pager_com));
				draw_page(0);
				break;

			case GLOBAL_CONNECTION_INFO:
				XFACE_CLEAR();
				show_connection_page();
				draw_page(0);
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:	/* toggle mini help menu */
				toggle_mini_help(PAGE_LEVEL);
				draw_page(0);
				break;

			case GLOBAL_QUIT:	/* return to index page */
return_to_index:
				XFACE_CLEAR();
				i = which_thread(this_resp);
				if (threadnum)
					*threadnum = which_response(this_resp);

				return i;

			case GLOBAL_TOGGLE_INVERSE_VIDEO:	/* toggle inverse video */
				toggle_inverse_video();
				draw_page(0);
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:		/* toggle color */
				if (toggle_color()) {
					draw_page(0);
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
				old_artnum = arts[this_resp].artnum;
				config_page(group->name, signal_context);
				if ((this_resp = find_artnum(old_artnum)) == -1 || which_thread(this_resp) == -1) { /* We have lost the thread */
					pos_first_unread_thread();
					return GRP_EXIT;
				}
				fixup_thread(this_resp, FALSE);
				draw_page(0);
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
				mail_to_author(group->name, this_resp, (func == PAGE_REPLY_QUOTE || func == PAGE_REPLY_QUOTE_HEADERS) ? TRUE : FALSE, func == PAGE_REPLY_QUOTE_HEADERS ? TRUE : FALSE, show_raw_article);
				draw_page(0);
				break;

			case PAGE_TAG:	/* tag/untag article for saving */
				tag_article(this_resp);
				break;

			case PAGE_GROUP_SELECT:	/* return to group selection page */
				XFACE_CLEAR();
				return GRP_RETSELECT;

			case GLOBAL_VERSION:
				info_message(cvers);
				break;

			case GLOBAL_POST:	/* post a basenote */
				XFACE_SUPPRESS();
				if (post_article(group->name))
					draw_page(0);
				XFACE_SHOW();
				break;

			case GLOBAL_POSTPONED:	/* post postponed article */
				if (can_post || art_type != GROUP_TYPE_NEWS) {
					XFACE_SUPPRESS();
					if (pickup_postponed_articles(FALSE, FALSE))
						draw_page(0);
					XFACE_SHOW();
				} else
					info_message(_(txt_cannot_post));
				break;

			case GLOBAL_DISPLAY_POST_HISTORY:	/* display messages posted by user */
				XFACE_SUPPRESS();
				if (post_hist_page())
					return GRP_EXIT;
				else {
					XFACE_SHOW();
				}
				break;

			case MARK_ARTICLE_UNREAD:	/* mark article as unread(to return) */
				art_mark(group, &arts[this_resp], ART_WILL_RETURN);
				info_message(_(txt_marked_article_as_unread));
				break;

			case PAGE_SKIP_INCLUDED_TEXT:	/* skip included text */
				if (show_raw_article) {
					info_message(_(txt_info_not_available_in_raw));
					break;
				}
				for (i = j = curr_line; i < artlines; i++) {
					if (artline[i].flags & (C_QUOTE1 | C_QUOTE2 | C_QUOTE3)) {
						j = i;
						break;
					}
				}

				for (; j < artlines; j++) {
					if (!(artline[j].flags & (C_QUOTE1 | C_QUOTE2 | C_QUOTE3)))
						break;
				}

				if (j != curr_line) {
					curr_line = j;
					draw_page(0);
				}
				break;

			case GLOBAL_TOGGLE_INFO_LAST_LINE: /* this is _not_ correct, we do not toggle status here */
				info_message("%s", arts[this_resp].subject);
				break;

			case PAGE_TOGGLE_HIGHLIGHTING:
				word_highlight = bool_not(word_highlight);
				draw_page(0);
				info_message(_(txt_toggled_high), txt_onoff[word_highlight != FALSE ? 1 : 0]);
				break;

			case PAGE_VIEW_ATTACHMENTS:
				XFACE_SUPPRESS();
				hide_inline_data_tmp = hide_inline_data;
				hide_inline_data = UUE_NO;
				resize_article(TRUE, &pgart);
				attachment_page(&pgart);
				signal_context = cPage;
				hide_inline_data = hide_inline_data_tmp;
				resize_article(TRUE, &pgart);
				draw_page(0);
				XFACE_SHOW();
				break;

			case PAGE_VIEW_URL:
				if (!show_raw_article) { /* cooked mode? */
					t_bool success;

					XFACE_SUPPRESS();
					resize_article(FALSE, &pgart); /* unbreak long lines */
					success = url_page();
					resize_article(TRUE, &pgart); /* rebreak long lines */
					draw_page(0);
					if (!success)
						info_message(_(txt_url_done));
					XFACE_SHOW();
				}
				break;

			case PAGE_ARTICLE_INFO:
				XFACE_CLEAR();
				show_article_info_page();
				draw_page(0);
				break;

			default:
				info_message(_(txt_bad_command), PrintFuncKey(key, GLOBAL_HELP, page_keys));
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

		if (fseek(file, curr->offset, SEEK_SET) != 0)
			break;

		if ((line = tin_fgets(file, FALSE)) == NULL)
			break;	/* ran out of message */

		if ((help_level == INFO_PAGER) && (strwidth(line) >= cCOLS - 1))
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			{
				char *tmp, *f, *t;

				f = tmp = strunc(line, cCOLS - 1);
				t = line;
				while (*f)
					*t++ = *f++;
				*t = '\0';
				free(tmp);
			}
#else
			line[cCOLS - 1] = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		/*
		 * use the offsets gained while doing line wrapping to
		 * determine the correct position to truncate the line
		 */
		if ((help_level != INFO_PAGER) && (base_line + i < messagelines - 1)) {	/* not last line of message */
			bytes = (int) ((curr + 1)->offset - curr->offset);
			line[bytes] = '\0';
		}

		/*
		 * rotN encoding on body and sig data only
		 */
		if ((rotate != 0) && ((curr->flags & (C_BODY | C_SIG)) || show_raw_article)) {
			for (p = line; *p; p++) {
				if (*p >= 'A' && *p <= 'Z')
					*p = (char) ((*p - 'A' + rotate) % 26 + 'A');
				else if (*p >= 'a' && *p <= 'z')
					*p = (char) ((*p - 'a' + rotate) % 26 + 'a');
			}
		}

		strip_line(line);

#ifndef USE_CURSES
		snprintf(screen[i + (size_t) scroll_region_top].col, (size_t) cCOLS, "%s" cCRLF, line);
#endif /* !USE_CURSES */

		MoveCursor((int) (i + (size_t) scroll_region_top), 0);
		draw_pager_line(line, curr->flags, show_raw_article);

		/*
		 * Highlight URL's and mail addresses
		 */
		if (tinrc.url_highlight) {
			if (curr->flags & C_URL) {
#ifdef HAVE_COLOR
				highlight_regexes((int) (i + (size_t) scroll_region_top), &url_regex, use_color ? tinrc.col_urls : -1);
#else
				highlight_regexes((int) (i + (size_t) scroll_region_top), &url_regex, -1);
#endif /* HAVE_COLOR */
			}

			if (curr->flags & C_MAIL) {
#ifdef HAVE_COLOR
				highlight_regexes((int) (i + (size_t) scroll_region_top), &mail_regex, use_color ? tinrc.col_urls : -1);
#else
				highlight_regexes((int) (i + (size_t) scroll_region_top), &mail_regex, -1);
#endif /* HAVE_COLOR */
			}

			if (curr->flags & C_NEWS) {
#ifdef HAVE_COLOR
				highlight_regexes((int) (i + (size_t) scroll_region_top), &news_regex, use_color ? tinrc.col_urls : -1);
#else
				highlight_regexes((int) (i + (size_t) scroll_region_top), &news_regex, -1);
#endif /* HAVE_COLOR */
			}
		}

		/*
		 * Highlight /slashes/, *stars*, _underscores_ and -strokes-
		 */
		if (word_highlight && (curr->flags & C_BODY) && !(curr->flags & C_CTRLL)) {
#ifdef HAVE_COLOR
			highlight_regexes((int) (i + (size_t) scroll_region_top), &slashes_regex, use_color ? tinrc.col_markslash : tinrc.mono_markslash);
			highlight_regexes((int) (i + (size_t) scroll_region_top), &stars_regex, use_color ? tinrc.col_markstar : tinrc.mono_markstar);
			highlight_regexes((int) (i + (size_t) scroll_region_top), &underscores_regex, use_color ? tinrc.col_markdash : tinrc.mono_markdash);
			highlight_regexes((int) (i + (size_t) scroll_region_top), &strokes_regex, use_color ? tinrc.col_markstroke : tinrc.mono_markstroke);
#else
			highlight_regexes((int) (i + (size_t) scroll_region_top), &slashes_regex, tinrc.mono_markslash);
			highlight_regexes((int) (i + (size_t) scroll_region_top), &stars_regex, tinrc.mono_markstar);
			highlight_regexes((int) (i + (size_t) scroll_region_top), &underscores_regex, tinrc.mono_markdash);
			highlight_regexes((int) (i + (size_t) scroll_region_top), &strokes_regex, tinrc.mono_markstroke);
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
	if ((end - start >= ARTLINES) || (part == 0)) {
		ClearScreen();
		draw_page_header(CURR_GROUP.attribute->tex2iso_conv);
	} else
		MoveCursor(0, 0);

	print_message_page(note_fp, artline, (size_t) artlines, (size_t) curr_line, (size_t) start, (size_t) end, PAGE_LEVEL);

	/*
	 * Print an appropriate footer
	 */
	if (curr_line + ARTLINES >= artlines) {
		char buf[LEN], *buf2;
		int len;

		STRCPY(buf, (arts[this_resp].thread != -1) ? _(txt_next_resp) : _(txt_last_resp));
		buf2 = strunc(buf, cCOLS - 1);
		len = strwidth(buf2);
		clear_message();
		MoveCursor(cLINES, cCOLS - len - (1 + BLANK_PAGE_COLS));
#ifdef HAVE_COLOR
		fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
		StartInverse();
		my_fputs(buf2, stdout);
		EndInverse();
		my_flush();
		free(buf2);
	} else
		draw_percent_mark(curr_line + ARTLINES, artlines);

	if (CURR_GROUP.attribute->show_art_score && arts[this_resp].score != 0) {
		char *buf;
#ifdef HAVE_COLOR
		int tmp_col = tinrc.col_invers_bg;
#endif /* HAVE_COLOR */
		size_t buflen = strlen(_(txt_art_score)) + 3;

		buf = my_malloc(buflen + 1);
		snprintf(buf, buflen, _(txt_art_score), tin_ltoa(arts[this_resp].score, 4));
		MoveCursor(cLINES, 0);
#ifdef HAVE_COLOR
		fcol(tinrc.col_normal);
		if (tinrc.inverse_okay) {
			if (arts[this_resp].score < 0)
				tinrc.col_invers_bg = tinrc.col_score_neg;
			else
				tinrc.col_invers_bg = tinrc.col_score_pos;
		} else {
			if (arts[this_resp].score < 0)
				fcol(tinrc.col_score_neg);
			else
				fcol(tinrc.col_score_pos);
		}
#endif /* HAVE_COLOR */
		StartInverse();
		my_fputs(buf, stdout);
		EndInverse();
		my_flush();
#ifdef HAVE_COLOR
		if (tinrc.inverse_okay)
			tinrc.col_invers_bg = tmp_col;
		else
			fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
		free(buf);
	}

#ifdef XFACE_ABLE
	if (tinrc.use_slrnface && !show_raw_article)
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
	char *ptr = tinrc.metamail_prog;
	char buf[LEN];
	long offset;
	FILE *mime_fp;
#ifdef DONT_HAVE_PIPING
	char mimefile[PATH_LEN];
	int fd_mime;
#endif /* DONT_HAVE_PIPING */

	if ((*ptr == '\0') || (!strcmp(ptr, INTERNAL_CMD)) || (getenv("NOMETAMAIL") != NULL))
		return;

	if ((offset = ftell(fp)) == -1) {
		perror_message(_(txt_command_failed), ptr);
		return;
	}

	EndWin();
	Raw(FALSE);

#ifdef DONT_HAVE_PIPING
	if ((fd_mime = my_mktmp(mimefile, sizeof(mimefile) - 1, homedir)) == -1) {
		perror_message(_(txt_command_failed), ptr);
		return;
	}
	if ((mime_fp = fdopen(fd_mime, "w")))
#else
	if ((mime_fp = popen(ptr, "w")))
#endif /* DONT_HAVE_PIPING */
	{
		t_bool seek_error = FALSE;

		rewind(fp);
		while (fgets(buf, (int) sizeof(buf), fp) != NULL)
			fputs(buf, mime_fp);

		fflush(mime_fp);
		/* This is needed if we are viewing the raw art */
		if (fseek(fp, offset, SEEK_SET) != -1)	{ /* goto old position */
		}
#ifdef DEBUG
		else {
			int e = errno;
			/*
			 * TODO: always show to user?
			 *       then use something less technical and move to lang.c
			 */
			perror_message("%s:%d invoke_metamail(fseek(fp)) failed", __FILE__, __LINE__);
			seek_error = TRUE;
			errno = e;
		}
#endif /* DEBUG */

		if (!seek_error) {
#ifdef DONT_HAVE_PIPING
			char *pbuf;
			size_t len = snprintf(NULL, 0, "%s %s", tinrc.metamail_prog, mimefile);

			pbuf = my_malloc(++len);
			snprintf(pbuf, len, "%s %s", tinrc.metamail_prog, mimefile);
			invoke_cmd(pbuf);
			free(pbuf);
#endif /* DONT_HAVE_PIPING */
		}
#ifdef DEBUG
		else
			perror_message(_(txt_command_failed), ptr);
#endif /* DEBUG */

#ifdef DONT_HAVE_PIPING
		fclose(mime_fp);
		unlink(mimefile);
#else
		pclose(mime_fp);
#endif /* DONT_HAVE_PIPING */
	} else
		perror_message(_(txt_command_failed), ptr);

#ifdef USE_CURSES
	Raw(TRUE);
	InitWin();
#endif /* USE_CURSES */
	prompt_continue();
#ifndef USE_CURSES
	Raw(TRUE);
	InitWin();
#endif /* !USE_CURSES */
}


/* TODO: add BiDi handling (our layout expects LTR) */
static char *
build_from_line(
	void)
{
	char *tmp_from, *curr_from, *next_from, *p;
	char addr[HEADER_LEN];
	char name[HEADER_LEN];
	char single_from[HEADER_LEN + 5]; /*"\"\" <>"*/
	char *from = NULL;
	int type, c_needed = 0;

	if (!note_h->from) {
		from = my_malloc(3);
		strcpy(from, "<>");
		return from;
	}

	curr_from = tmp_from = my_strdup(note_h->from);

	do {
		next_from = split_mailbox_list(curr_from);
		if (gnksa_split_from(curr_from, addr, name, &type) == GNKSA_OK) {
			buffer_to_ascii(addr);
			p = idna_decode(addr);

			if (*name) {
				if (CHECK_RFC5322_SPECIALS(name))
					snprintf(single_from, sizeof(single_from), "\"%s\" <%s>", name, p);
				else
					snprintf(single_from, sizeof(single_from), "%s <%s>", name, p);
			} else {
				STRCPY(single_from, p);
			}
			free(p);
		} else {
			p = idna_decode(str_trim(curr_from));
			STRCPY(single_from, p);
			free(p);
		}
		if (c_needed++)
			from = append_to_string(from, ", ");
		from = append_to_string(from, single_from);
		curr_from = next_from;
	} while (curr_from);

	free(tmp_from);
	return from;
}


#define CCOLS_THIRD (cCOLS / 3 + 1)
/*
 * PAGE_HEADER defines the size in lines of this header
 *
 * TODO: what about page_header_format{1,PAGE_HEADER-1}
 *       to be able to customize the shown data and if all
 *       are set to NULL use the full screen for ARTLINES?
 *       (headers then could be seen via news_headers_to_display)
 */
static void
draw_page_header(
	t_bool tex2iso_conv)
{
	char *buf, *from, *tmp = NULL, *tmp2 = NULL;
	int i, n;
	int whichresp, x_resp;
	int len, right_len, center_pos, cur_pos;
	int tex_space = 0;
	size_t line_len, tlen;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp, *wtmp2;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	whichresp = which_response(this_resp);
	if ((i = which_thread(this_resp)) >= 0)
		x_resp = num_of_responses(i);
	else
		x_resp = 0;

	line_len = LEN + 1;
	buf = my_malloc(line_len);

	if (!my_strftime(buf, line_len, curr_group->attribute->date_format ? BlankIfNull(*curr_group->attribute->date_format) : "", localtime(&arts[this_resp].date))) {
		strncpy(buf, BlankIfNull(note_h->date), line_len);
		buf[line_len - 1] = '\0';
	}

#	ifdef HAVE_COLOR
	fcol(tinrc.col_head);
#	endif /* HAVE_COLOR */

	/* first line */
	cur_pos = 0;

	/*
	 * Determine the needed space for the text at the right hand margin.
	 * The formatting info (%4s) needs 3 positions but we need 4 positions
	 * on the screen for each counter.
	 */
	right_len = strwidth(_(txt_thread_x_of_n)) - 6 + 8;

	/*
	 * limit right_len to cCOLS / 3
	 */
	if (right_len > CCOLS_THIRD)
		right_len = CCOLS_THIRD;

	/* date */
	my_fputs(buf, stdout);
	cur_pos += strwidth(buf);

	/*
	 * determine max len for centered group name
	 * allow one space before and after group name
	 */
	if ((len = cCOLS - 2 * MAX(cur_pos, right_len) - 3) < 0)
		len = 0;

	/* group name */
	if (len > 0) {
		if (tinrc.abbreviate_groupname) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if ((wtmp = char2wchar_t(curr_group->name)) != NULL) {
				/* wconvert_to_printable(wtmp, FALSE); */
				wtmp2 = abbr_wcsgroupname(wtmp, len);
				tmp = wchar_t2char(wtmp2);
				free(wtmp);
				free(wtmp2);
			}
#else
			tmp = abbr_groupname(curr_group->name, len);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		} else
			tmp = strunc(curr_group->name, len);
	}
	if (!tmp)
		tmp = my_strdup("");

	if ((i = strwidth(tmp)) < len)
		len = i;

	center_pos = (cCOLS - len) / 2;

	/* pad out to left */
	for (; cur_pos < center_pos; cur_pos++)
		my_fputc(' ', stdout);

	my_fputs(tmp, stdout);
	cur_pos += strwidth(tmp);
	FreeAndNull(tmp);

	/* pad out to right */
	for (; cur_pos < cCOLS - right_len - 1; cur_pos++)
		my_fputc(' ', stdout);

	/* thread info */
	/* can't eval tin_ltoa() more than once in a statement due to statics */
	{
		char *buf2 = my_calloc(1, 5);

		strcpy(buf, tin_ltoa(which_thread(this_resp) + 1, 4));
		strcpy(buf2, tin_ltoa(grpmenu.max, 4));

		if ((n = snprintf(NULL, 0, _(txt_thread_x_of_n), buf, buf2)) < 0) {
			free(buf2);
			goto skip;
		}

		tlen = (size_t) n + 1;
		tmp2 = my_malloc(tlen);
		if (snprintf(tmp2, tlen, _(txt_thread_x_of_n), buf, buf2) != n) {
			FreeAndNull(tmp2);
			free(buf2);
			goto skip;
		}
		tmp = strunc(tmp2, CCOLS_THIRD);
		my_printf("%s", tmp);
		FreeAndNull(tmp);
		FreeAndNull(tmp2);
		free(buf2);
skip:
		my_fputs(cCRLF, stdout);
	}
#	if 0
	/* display a ruler for layout checking purposes */
	my_fputs("....|....3....|....2....|....1....|....0....|....1....|....2....|....3....|....\n", stdout);
#	endif /* 0 */

	/*
	 * second line
	 */
	cur_pos = 0;

	/*
	 * Determine the needed space for the text at the right hand margin
	 * the formatting info (%4s) needs 3 positions but we need 4 positions
	 * on the screen for each counter
	 */
	if (whichresp)
		right_len = strwidth(_(txt_art_x_of_n)) - 6 + 8;
	else {
		if (!x_resp)
			right_len = strwidth(_(txt_no_responses));
		else
			right_len = strwidth(P_(txt_x_resp_sp[0], txt_x_resp_sp[1], x_resp)) - 3 + 4;
	}

	/*
	 * limit right_len to cCOLS / 3
	 */
	if (right_len > CCOLS_THIRD)
		right_len = CCOLS_THIRD;

	/* line count */
	if (arts[this_resp].line_count < 0)
		strcpy(buf, "?");
	else
		snprintf(buf, line_len, "%-4d", arts[this_resp].line_count);

	if ((n = snprintf(NULL, 0, _(txt_lines), buf)) > 0) {
		tlen = (size_t) n + 1;
		tmp = my_malloc(tlen);
		if (snprintf(tmp, line_len, _(txt_lines), buf) == n) {
			tmp2 = strunc(tmp, CCOLS_THIRD);
			my_fputs(tmp2, stdout);
			cur_pos += strwidth(tmp2);
		}
		free(tmp);
		free(tmp2);
	}

	/* tex2iso */
	if (pgart.tex2iso && tex2iso_conv) {
		tex_space = strwidth(_(txt_tex));
		if (tex_space > CCOLS_THIRD - cur_pos)
			tex_space = CCOLS_THIRD - cur_pos;

		tmp = strunc(_(txt_tex), tex_space);
		tex_space = strwidth(tmp);
		my_fputs(tmp, stdout);
		cur_pos += tex_space;
		free(tmp);
	}

	/* subject */
#	ifdef HAVE_COLOR
	fcol(tinrc.col_subject);
#	endif /* HAVE_COLOR */
	/*
	 * TODO: why do we fall back to arts[this_resp].subject if !note_h->subj?
	 *       if !note_h->subj then the article just has no subject, no matter
	 *       what the overview says.
	 *
	 *       add BiDi handling (our layout expects LTR)
	 */
	strncpy(buf, (note_h->subj ? note_h->subj : arts[this_resp].subject), line_len);
	buf[line_len - 1] = '\0';

	tmp2 = expand_tab(buf, tabwidth);
	tmp = strunc(tmp2, cCOLS - 2 * (MAX(cur_pos, right_len)) - 4);

	n = strwidth(tmp);
	center_pos = (cCOLS - n) / 2;

	/* pad out to left */
	for (; cur_pos < center_pos; cur_pos++)
		my_fputc(' ', stdout);

	StartInverse();
	my_fputs(tmp, stdout);
	EndInverse();
	cur_pos += n;
	free(tmp);
	free(tmp2);

#	ifdef HAVE_COLOR
	fcol(tinrc.col_response);
#	endif /* HAVE_COLOR */

	/* pad out to right */
	for (; cur_pos < cCOLS - right_len - 1; cur_pos++)
		my_fputc(' ', stdout);

	if (whichresp) {
		if ((n = snprintf(NULL, 0, _(txt_art_x_of_n), whichresp + 1, x_resp + 1)) < 0)
			goto shrug;

		tlen = (size_t) n + 1;
		tmp2 = my_malloc(tlen);
		if (snprintf(tmp2, tlen, _(txt_art_x_of_n), whichresp + 1, x_resp + 1) != n) {
			free(tmp2);
			goto shrug;
		}
		tmp = strunc(tmp2, CCOLS_THIRD);
		my_printf("%s", tmp);
		free(tmp2);
		free(tmp);
	} else {
		if (!x_resp) {
			tmp = strunc(_(txt_no_responses), CCOLS_THIRD);
			my_printf("%s", tmp);
		} else {
			if ((n = snprintf(NULL, 0, P_(txt_x_resp_sp[0], txt_x_resp_sp[1], x_resp), x_resp)) < 0)
				goto shrug;

			tlen = (size_t) n + 1;
			tmp2 = my_malloc(tlen);
			if (snprintf(tmp2, tlen, P_(txt_x_resp_sp[0], txt_x_resp_sp[1], x_resp), x_resp) != n) {
				free(tmp2);
				goto shrug;
			}
			tmp = strunc(tmp2, CCOLS_THIRD);
			my_printf("%s", tmp);
			free(tmp2);
		}
		free(tmp);
	}
shrug:
	my_fputs(cCRLF, stdout);

	/*
	 * third line
	 */
	cur_pos = 0;

#	ifdef HAVE_COLOR
	fcol(tinrc.col_from);
#	endif /* HAVE_COLOR */

	/* from */
	from = build_from_line();
	snprintf(buf, line_len, "%s", from);
	free(from);

	tmp = strunc(buf, cCOLS - 1);
	my_fputs(tmp, stdout);
	cur_pos += strwidth(tmp);
	free(tmp);

	/*
	 * Organization
	 *
	 * TODO: - IDNA decoding, see also comment in
	 *         cook.c:cook_article()
	 *       - add BiDi handling (our layout expects LTR)
	 */
	if (note_h->org && cCOLS - cur_pos - 1 >= (int) strlen(_(txt_at_s)) - 2 + 3) {
		/* we have enough space to print at least " at ..." */
		snprintf(buf, line_len, _(txt_at_s), note_h->org);

		tmp2 = expand_tab(buf, tabwidth);
		tmp = strunc(tmp2, cCOLS - cur_pos - 1);
		len = cCOLS - (int) strwidth(tmp) - 1;
		for (; cur_pos < len; cur_pos++)
			my_fputc(' ', stdout);
		my_fputs(tmp, stdout);
		free(tmp);
		free(tmp2);
	}

	my_fputs(cCRLF, stdout);
	my_fputs(cCRLF, stdout);

	free(buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
}
#undef CCOLS_THIRD


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
	static t_bool art_closed = FALSE;

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		fprintf(stderr, "load_art %s(new=%d, curr=%d)\n", (new_respnum == this_resp && !art_closed) ? "ALREADY OPEN!" : "", new_respnum, this_resp);
#endif /* DEBUG */

	if (new_respnum != this_resp || art_closed) {
		art_close(&pgart);			/* close previously opened art in pager */
		switch (art_open(TRUE, &arts[new_respnum], group, &pgart, TRUE, _(txt_reading_article))) {
			case ART_UNAVAILABLE:
				art_mark(group, &arts[new_respnum], ART_READ);
				/* prevent retagging as unread in unfilter_articles() */
				if (arts[new_respnum].killed == ART_KILLED_UNREAD)
					arts[new_respnum].killed = ART_KILLED;
				art_closed = TRUE;
				wait_message(1, _(txt_art_unavailable));
				return GRP_ARTUNAVAIL;

			case ART_ABORT:
				art_close(&pgart);
				art_closed = TRUE;
				return GRP_ARTABORT;	/* special retcode to stop redrawing screen */

			default:					/* Normal case */
#if 0			/* Very useful debugging tool */
				if (prompt_yn("Fake art unavailable? ", FALSE) == 1) {
					art_close(&pgart);
					art_mark(group, &arts[new_respnum], ART_READ);
					art_closed = TRUE;
					return GRP_ARTUNAVAIL;
				}
#endif /* 0 */
				if (art_closed)
					art_closed = FALSE;
				if (new_respnum != this_resp) {
					/*
					 * Remember current & previous articles for '-' command
					 */
					last_resp = this_resp;
					this_resp = new_respnum;		/* Set new art globally */
				}
				break;
		}
	} else if (show_all_headers) {
		/*
		 * article is already opened with show_all_headers ON
		 * -> re-cook it
		 */
		show_all_headers = FALSE;
		resize_article(TRUE, &pgart);
	}

	art_mark(group, &arts[this_resp], ART_READ);

	/*
	 * Change status if art was unread before killing to
	 * prevent retagging as unread in unfilter_articles()
	 */
	if (arts[this_resp].killed == ART_KILLED_UNREAD)
		arts[this_resp].killed = ART_KILLED;

	if (pgart.cooked == NULL) { /* harmony corruption */
		wait_message(1, _(txt_art_unavailable));
		return GRP_ARTUNAVAIL;
	}

	/*
	 * Setup to start viewing cooked version
	 */
	show_raw_article = FALSE;
	show_all_headers = FALSE;
	curr_line = 0;
	note_fp = pgart.cooked;
	artline = pgart.cookl;
	artlines = pgart.cooked_lines;
	search_line = 0;
	/*
	 * Reset offsets only if not invoked during 'body search' (srch_lineno != -1)
	 * otherwise the found string will not be highlighted
	 */
	if (srch_lineno == -1)
		reset_srch_offsets();
	rotate = 0;			/* normal mode, not rot13 */
	reveal_ctrl_l = FALSE;
	reveal_ctrl_l_lines = -1;	/* all ^L's active */
	hide_inline_data = group->attribute->hide_inline_data;

	draw_page(0);

	/*
	 * Automatically invoke attachment viewing if requested
	 */
	if (!note_h->mime || IS_PLAINTEXT(note_h->ext))		/* Text only article */
		return 0;

	if (!tinrc.metamail_prog || !*tinrc.metamail_prog || getenv("NOMETAMAIL") != NULL)	/* Viewer turned off */
		return 0;

	if (group->attribute->ask_for_metamail) {
		if (prompt_yn(_(txt_use_mime), TRUE) != 1)
			return 0;
	}

	XFACE_SUPPRESS();
	if (STRCMPEQ(tinrc.metamail_prog, INTERNAL_CMD))	/* Use internal viewer */
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
	int i, num;

	clear_message();

	if ((num = prompt_num(ch, _(txt_select_art))) < 0) {
		clear_message();
		return -1;
	}

	if ((i = which_thread(curr_respnum)) >= 0)
		return find_response(i, num - 1);
	else
		return -1;
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
	int i;
	REGEX_SIZE start, end;

	if ((i = get_search_vectors(&start, &end)) == -1)
		return;

	/*
	 * Is matching line off the current view?
	 * Reposition within article if needed, try to get matched line
	 * in the middle of the screen
	 */
	if (i < *lcurr_line || i >= (int) ((size_t) *lcurr_line + screen_lines)) {
		*lcurr_line = (int) ((size_t) i - (screen_lines / 2));
		if (((size_t) *lcurr_line + screen_lines) > message_lines)	/* off the end */
			*lcurr_line = (int) (message_lines - screen_lines);
		/* else pos. is just fine */
	}

	switch (help_level) {
		case PAGE_LEVEL:
			draw_page(0);
			break;

		case INFO_PAGER:
			display_info_page(0);
			break;

		default: /* unknown level, should not happen */
			/* CONSTANTCONDITION */
			assert(0 != 0);
			break;
	}
	search_line = i;								/* draw_page() resets this to 0 */

	/*
	 * Highlight found string
	 */
	highlight_string(i - *lcurr_line + scroll_region_top, (int) start, (int) (end - start));
}


/*
 * Implement ^H toggle between cooked and raw views of article
 */
void
toggle_raw(
	void)
{
	if (show_raw_article) {
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
			pgart.rawl = my_malloc(sizeof(t_lineinfo) * (size_t) chunk);
			offset = ftell(pgart.raw);

			while ((line = tin_fgets(pgart.raw, FALSE)) != NULL) {
				int space;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				int num_bytes;
				wchar_t wc;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

				pgart.rawl[j].offset = offset;
				pgart.rawl[j].flags = 0;
				++j;
				if (j >= chunk) {
					chunk += 50;
					pgart.rawl = my_realloc(pgart.rawl, sizeof(t_lineinfo) * (size_t) chunk);
				}

				p = line;
				while (*p) {
					space = cCOLS - 1; /* -1 to work around differences in terminal wrapping. ANSI vs. VT100 and eat_newline_glitch / xenl / xn */

					while ((space > 0) && *p) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
						num_bytes = mbtowc(&wc, p, MB_CUR_MAX);
						if (num_bytes != -1 && iswprint((wint_t) wc)) {
							if ((space -= wcwidth(wc)) < 0)
								break;
							p += num_bytes;
							offset += num_bytes;
						}
#else
						if (my_isprint((unsigned char) *p)) {
							--space;
							++p;
							++offset;
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
							--space;
						} else {
							/*
							 * the current character can't be displayed print it as
							 * an octal value (needs 4 columns) see also
							 * color.c:draw_pager_line()
							 */
							if ((space -= 4) < 0)
								break;
							++offset;
							++p;
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
							pgart.rawl = my_realloc(pgart.rawl, sizeof(t_lineinfo) * (size_t) chunk);
						}
					}
				}

				/*
				 * only use ftell's return value here because we didn't
				 * take the \n into account above.
				 */
				offset = ftell(pgart.raw);
			}

			pgart.rawl = my_realloc(pgart.rawl, sizeof(t_lineinfo) * (size_t) j);
		}
		artline = pgart.rawl;
		artlines = j;
		note_fp = pgart.raw;
	}
	curr_line = 0;
	show_raw_article = bool_not(show_raw_article);
	draw_page(0);
}


void
update_hide_inline_data(
	void)
{
	if (hide_inline_data != curr_group->attribute->hide_inline_data)
		hide_inline_data = curr_group->attribute->hide_inline_data;
}


/*
 * Re-cook an article
 */
void
resize_article(
	t_bool wrap_lines,
	t_openartinfo *artinfo)
{
	free(artinfo->cookl);
	if (artinfo->cooked)
		fclose(artinfo->cooked);

	if (!cook_article(wrap_lines, artinfo, hide_inline_data, show_all_headers))
		tin_done(EXIT_FAILURE, _(txt_cook_article_failed_exiting), tin_progname);

	show_raw_article = FALSE;
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
	if (!info_fh)
		return;
	set_xclick_off();
	display_info_page(0);

	forever {
		switch (func = handle_keypad(page_left, page_right, page_mouse_action, info_keys)) {
			case GLOBAL_ABORT:	/* common arrow keys */
				break;

			case GLOBAL_LINE_UP:
				if (num_info_lines <= NOTESLINES) {
					info_message(_(txt_begin_of_page));
					break;
				}
				if (curr_info_line == 0) {
					if (!wrap_at_ends) {
						info_message(_(txt_begin_of_page));
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
					info_message(_(txt_end_of_page));
					break;
				}
				if (curr_info_line + NOTESLINES >= num_info_lines) {
					if (!wrap_at_ends) {
						info_message(_(txt_end_of_page));
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
					info_message(_(txt_end_of_page));
					break;
				}
				if (curr_info_line + NOTESLINES >= num_info_lines) {	/* End is already on screen */
					if (!wrap_at_ends) {
						info_message(_(txt_end_of_page));
						break;
					}
					curr_info_line = 0;
					display_info_page(0);
					break;
				}
				curr_info_line += ((tinrc.scroll_lines == -2) ? NOTESLINES / 2 : NOTESLINES);
				display_info_page(0);
				break;

			case GLOBAL_PAGE_UP:
				if (num_info_lines <= NOTESLINES) {
					info_message(_(txt_begin_of_page));
					break;
				}
				if (curr_info_line == 0) {
					if (!wrap_at_ends) {
						info_message(_(txt_begin_of_page));
						break;
					}
					curr_info_line = num_info_lines - NOTESLINES;
					display_info_page(0);
					break;
				}
				curr_info_line -= ((tinrc.scroll_lines == -2) ? NOTESLINES / 2 : NOTESLINES);
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

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:
				if (toggle_color()) {
					display_info_page(0);
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					break;

				if ((search_article((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT), search_line, num_info_lines, infoline, num_info_lines - 1, info_file)) == -1)
					break;

				process_search(&curr_info_line, (size_t) num_info_lines, (size_t) NOTESLINES, INFO_PAGER);
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

	print_message_page(info_file, infoline, (size_t) num_info_lines, (size_t) curr_info_line, (size_t) start, (size_t) end, INFO_PAGER);

	/* print footer */
	draw_percent_mark(curr_info_line + (curr_info_line + NOTESLINES < num_info_lines ? NOTESLINES : num_info_lines - curr_info_line), num_info_lines);
	stow_cursor();
}


static void
preprocess_info_message(
	FILE *info_fh)
{
	int chunk = 50;

	FreeAndNull(infoline);
	if (!info_fh)
		return;

	rewind(info_fh);
	infoline = my_malloc(sizeof(t_lineinfo) * (size_t) chunk);
	num_info_lines = 0;

	do {
		infoline[num_info_lines].offset = ftell(info_fh);
		infoline[num_info_lines].flags = 0;
		++num_info_lines;
		if (num_info_lines >= chunk) {
			chunk += 50;
			infoline = my_realloc(infoline, sizeof(t_lineinfo) * (size_t) chunk);
		}
	} while (tin_fgets(info_fh, FALSE) != NULL);

	--num_info_lines;
	infoline = my_realloc(infoline, sizeof(t_lineinfo) * (size_t) num_info_lines);
}


/*
 * URL menu
 */
static t_function
url_left(
	void)
{
	return GLOBAL_QUIT;
}


static t_function
url_right(
	void)
{
	return URL_SELECT;
}


static void
show_url_page(
	void)
{
	int i, prev_mark_offset = mark_offset;

	signal_context = cURL;
	currmenu = &urlmenu;
	mark_offset = 0;

	if (urlmenu.curr < 0)
		urlmenu.curr = 0;

	ClearScreen();
	set_first_screen_item();
	center_line(0, TRUE, _(txt_url_menu));

	for (i = urlmenu.first; i < urlmenu.first + NOTESLINES && i < urlmenu.max; ++i)
		build_url_line(i);

	show_mini_help(URL_LEVEL);

	draw_url_arrow();
	mark_offset = prev_mark_offset;
}


static t_bool
url_page(
	void)
{
	char key[MAXKEYLEN];
	t_function func;
	t_menu *oldmenu = NULL;

	if (currmenu)
		oldmenu = currmenu;
	urlmenu.curr = 0;
	urlmenu.max = build_url_list();
	if (urlmenu.max == 0)
		return FALSE;

	clear_note_area();
	show_url_page();
	set_xclick_off();

	forever {
		switch ((func = handle_keypad(url_left, url_right, NULL, url_keys))) {
			case GLOBAL_QUIT:
				free_url_list();
				if (oldmenu)
					currmenu = oldmenu;
				return TRUE;

			case DIGIT_1:
			case DIGIT_2:
			case DIGIT_3:
			case DIGIT_4:
			case DIGIT_5:
			case DIGIT_6:
			case DIGIT_7:
			case DIGIT_8:
			case DIGIT_9:
				if (urlmenu.max)
					prompt_item_num(func_to_key(func, url_keys), _(txt_url_select));
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_HELP:
				show_help_page(URL_LEVEL, _(txt_url_menu_com));
				show_url_page();
				break;

			case GLOBAL_FIRST_PAGE:
				top_of_list();
				break;

			case GLOBAL_LAST_PAGE:
				end_of_list();
				break;

			case GLOBAL_REDRAW_SCREEN:
				my_retouch();
				show_url_page();
				break;

			case GLOBAL_LINE_DOWN:
				move_down();
				break;

			case GLOBAL_LINE_UP:
				move_up();
				break;

			case GLOBAL_PAGE_DOWN:
				page_down();
				break;

			case GLOBAL_PAGE_UP:
				page_up();
				break;

			case GLOBAL_SCROLL_DOWN:
				scroll_down();
				break;

			case GLOBAL_SCROLL_UP:
				scroll_up();
				break;

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:
				if (toggle_color()) {
					show_url_page();
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case GLOBAL_TOGGLE_HELP_DISPLAY:
				toggle_mini_help(URL_LEVEL);
				show_url_page();
				break;

			case GLOBAL_TOGGLE_INFO_LAST_LINE:
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				clear_message();
				draw_url_arrow();
				break;

			case GLOBAL_TOGGLE_INVERSE_VIDEO:
				toggle_inverse_video();
				show_url_page();
				show_inverse_video_status();
				break;

			case GLOBAL_VERSION:
				info_message(cvers);
				break;

			case URL_SELECT:
				if (urlmenu.max) {
					if (process_url(urlmenu.curr))
						show_url_page();
					else
						draw_url_arrow();
				}
				break;

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					info_message(_(txt_no_prev_search));
				else if (urlmenu.max) {
					int new_pos, old_pos = urlmenu.curr;

					new_pos = generic_search((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT), urlmenu.curr, urlmenu.max - 1, URL_LEVEL);
					if (new_pos != old_pos)
						move_to_item(new_pos);
				}
				break;

			default:
				info_message(_(txt_bad_command), PrintFuncKey(key, GLOBAL_HELP, url_keys));
				break;
		}
	}
}


static void
draw_url_arrow(
	void)
{
	draw_arrow_mark(INDEX_TOP + urlmenu.curr - urlmenu.first);
	if (tinrc.info_in_last_line) {
		t_url *lptr;

		lptr = find_url(urlmenu.curr);
		info_message("%s", lptr->url);
	} else if (urlmenu.curr == urlmenu.max - 1)
		info_message(_(txt_end_of_urls));
}


t_url *
find_url(
	int n)
{
	t_url *lptr;

	lptr = url_list;
	while (n-- > 0 && lptr->next)
		lptr = lptr->next;

	return lptr;
}


static void
build_url_line(
	int i)
{
	char *sptr;
	int len = cCOLS - 9;
	t_url *lptr;

#ifdef USE_CURSES
	/*
	 * Allocate line buffer
	 * make it the same size like in !USE_CURSES case to simplify some code
	 */
	sptr = my_malloc((size_t) cCOLS + 2);
#else
	sptr = screen[INDEX2SNUM(i)].col;
#endif /* USE_CURSES */

	lptr = find_url(i);
	snprintf(sptr, (size_t) cCOLS, "  %s  %-*.*s%s", tin_ltoa(i + 1, 4), len, len, lptr->url, cCRLF);
	WriteLine(INDEX2LNUM(i), sptr);

#ifdef USE_CURSES
	free(sptr);
#endif /* USE_CURSES */
}


static t_bool
process_url(
	int n)
{
	char *url;
	const char *url_esc;
	int l;
	size_t len;
	t_url *lptr;
#if defined(HAVE_LIBURIPARSER) || defined(HAVE_LIBCURL)
	char *uri_norm;
	int ulen = -1;
#endif /* HAVE_LIBURIPARSER || HAVE_LIBCURL */
#ifdef HAVE_LIBURIPARSER
	UriParserStateA state;
	UriUriA uri;
#else
#	ifdef HAVE_LIBCURL
	CURLU *curl;
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */

	lptr = find_url(n);
	len = strlen(lptr->url) << 1; /* double size; room for editing URL */
	url = my_malloc(len + 1);
	if (prompt_default_string("URL:", url, (int) len, lptr->url, HIST_URL)) {
		if (!*url) {			/* Don't try and open nothing */
			free(url);
			return FALSE;
		}

#ifdef HAVE_LIBURIPARSER
		/*
		 * Syntax-Based Normalization RFC 3986 6.2.2
		 *
		 * We could keep the error code, but that would be likely
		 * always URI_ERROR_SYNTAX, so no big win.
		 * With -DDEBUG MISC|URI we would write some details
		 * (uri.scheme, uri.hostText, ...) to a log.
		 * And/or make use of uri(Une|E)scape* ...
		 */
		state.uri = &uri;
		if (uriParseUriA(&state, url) == URI_SUCCESS) {
			if (uriNormalizeSyntaxA(&uri) == URI_SUCCESS) {
				if (uriToStringCharsRequiredA(&uri, &ulen) == URI_SUCCESS) {
					uri_norm = my_malloc(++ulen);
					if (uriToStringA(uri_norm, &uri, ulen, NULL) == URI_SUCCESS) {
						free(url);
						url = uri_norm;
					} else {
						free(uri_norm);
						ulen = -1;
					}
				}
			}
			uriFreeUriMembersA(&uri);
		}
#else
#	ifdef HAVE_LIBCURL
		/* same as above but with libcurl, again no detailed error logging yet */
		if ((curl = curl_url())) {
			if (curl_url_set(curl, CURLUPART_URL, url, CURLU_URLENCODE) == CURLUE_OK) {
				if (curl_url_get(curl, CURLUPART_URL, &uri_norm, 0) == CURLUE_OK) {
					free(url);
					url = uri_norm;
					ulen = strlen(url);
				}
			}
			curl_url_cleanup(curl);
		}
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */
#if defined(HAVE_LIBURIPARSER) || defined(HAVE_LIBCURL)
		if (ulen < 0) { /* should only happen if the URL was modified in the prompt above */
			error_message(2, "URI Normalization failed: %s", url); /* TODO: -> lang.c; _()? */
			free(url);
			return FALSE;
		}
#endif /* HAVE_LIBURIPARSER || HAVE_LIBCURL */
		url_esc = escape_shell_meta(url, no_quote);
		if ((l = snprintf(NULL, 0, "%s %s", tinrc.url_handler, url_esc)) < 0) {
			free(url);
			return FALSE;
		}
		wait_message(2, _(txt_url_open), url); /* use url_esc in message? */
		len = (size_t) l + 1;
		url = my_realloc(url, len);
		if (snprintf(url, len, "%s %s", tinrc.url_handler, url_esc) != l) {
			free(url);
			return FALSE;
		}
		invoke_cmd(url);
		free(url);
		cursoroff();
		return TRUE;
	}
	free(url);
	return FALSE;
}


static int
build_url_list(
	void)
{
	char *ptr;
	int i, count = 0;
	REGEX_SIZE *offsets = NULL;
	t_url *lptr = NULL;

	for (i = 0; i < artlines; ++i) {
		if (!(artline[i].flags & (C_URL | C_NEWS | C_MAIL)))
			continue;

		/*
		 * Line contains a URL, so read it in
		 */
		if (fseek(pgart.cooked, artline[i].offset, SEEK_SET) == -1) /* skip on error */
			continue;
		if ((ptr = tin_fgets(pgart.cooked, FALSE)) == NULL)
			continue;

		/*
		 * Step through, finding URL's
		 */
		forever {
			/* any matches left? */
			if (MATCH_REGEX(url_regex, ptr, strlen(ptr))) {
				offsets = regex_get_ovector_pointer(&url_regex);
			} else if (MATCH_REGEX(mail_regex, ptr, strlen(ptr))) {
				offsets = regex_get_ovector_pointer(&mail_regex);
			} else if (MATCH_REGEX(news_regex, ptr, strlen(ptr))) {
				offsets = regex_get_ovector_pointer(&news_regex);
			} else
				break;

			*(ptr + offsets[1]) = '\0';

			if (!lptr)
				lptr = url_list = my_malloc(sizeof(t_url));
			else {
				lptr->next = my_malloc(sizeof(t_url));
				lptr = lptr->next;
			}
			lptr->url = my_strdup(ptr + offsets[0]);
			lptr->next = NULL;
			++count;

			ptr += offsets[1] + 1;
		}
	}
	return count;
}


static void
free_url_list(
	void)
{
	t_url *p, *q;

	for (p = url_list; p != NULL; p = q) {
		q = p->next;
		free(p->url);
		free(p);
	}
	url_list = NULL;
}


static void
show_article_info_page(
	void)
{
	if (!(pgart.log))
		return;

	info_pager(pgart.log, _(txt_article_info_page), FALSE); /* all other pagers do wrap */
	info_pager(NULL, NULL, TRUE); /* free mem */
}


#define MIME_FLAG(what) (ptr->mime_hints.flags & what)
/* TODO: strings to lang.c and _() ? */
void
log_article_info(
	t_openartinfo *artinfo)
{
	const char *charset;
	int i = 1;
	FILE *fp;
	t_part *ptr;
#ifdef DEBUG
	const char *filename_star;
	const char *filename;
	const char *name;
#endif /* DEBUG */

	if (!(fp = artinfo->log))
		return;

	fprintf(fp, "Message-ID: %s\n", BlankIfNull(artinfo->hdr.messageid));

	for (ptr = artinfo->hdr.ext; ptr; ptr = ptr->next, i++) {
		fprintf(fp, "\nPart %d:\n", i);

		if (i == 1) {
			fprintf(fp, "\tMIME Version:\n");
			fprintf(fp, "\t\tArticle:\t%s", MIME_FLAG(MIME_VERSION_MISSING) ? "Missing" : MIME_FLAG(MIME_VERSION_UNSUPPORTED) ? "Unsupported" : MIME_SUPPORTED_VERSION);
			fprintf(fp, "\n\t\tUsed:\t\t%s\n", MIME_SUPPORTED_VERSION);
		}

		fprintf(fp, "\tContent Type:\n");
		fprintf(fp, "\t\tArticle:\t");
		if (MIME_FLAG(MIME_TYPE_MISSING))
			fprintf(fp, "%s", "Missing");
		else if (MIME_FLAG(MIME_TYPE_UNKNOWN))
			fprintf(fp, "%s: %s", "Unsupported", ptr->mime_hints.type);
		else
			fprintf(fp, "%s", ptr->mime_hints.type);
		if (MIME_FLAG(MIME_SUBTYPE_MISSING))
			fprintf(fp, " / %s", "Missing");
		else if (MIME_FLAG(MIME_SUBTYPE_UNKNOWN))
			fprintf(fp, " / %s: %s", "Unsupported", ptr->mime_hints.subtype);
		else
			fprintf(fp, " / %s", ptr->mime_hints.subtype);
		if (ptr->format)
			fprintf(fp, "%s", " ; Format=Flowed");
		fprintf(fp, "\n\t\tUsed:\t\t%s / %s", content_types[ptr->type], ptr->subtype);

		charset = get_param(ptr->params, "charset");
		if (!charset || !*charset)
			charset = "US-ASCII";
		fprintf(fp, "\n\tCharset:\n");
		fprintf(fp, "\t\tArticle:\t");
		if (MIME_FLAG(MIME_CHARSET_MISSING)) {
			fprintf(fp, "%s", "Missing");
			if (MIME_FLAG(MIME_CHARSET_UNDECLARED))
				fprintf(fp, ", %s: %s", "undeclared_charset", charset);
#if defined(CHARSET_CONVERSION) && defined(USE_ICU_UCSDET)
			else if (MIME_FLAG(MIME_CHARSET_GUESSED)) {
				charset = get_param(ptr->params, "guessed_charset");
				fprintf(fp, ", %s: %s", "charset guessed", charset);
			}
#endif /* CHARSET_CONVERSION && USE_ICU_UCSDET */
		} else if (MIME_FLAG(MIME_CHARSET_UNSUPPORTED)) {
			fprintf(fp, "%s: %s", "Unsupported", ptr->mime_hints.charset);
			charset = "None";
		} else
			fprintf(fp, "%s", charset);
		if (MIME_FLAG(MIME_TRANSFER_ENCODING_UNKNOWN))
			charset = "None";
		fprintf(fp, "\n\t\tUsed:\t\t%s", charset);

		fprintf(fp, "\n\tContent Transfer Encoding:\n");
		fprintf(fp, "\t\tArticle:\t");
		if (MIME_FLAG(MIME_TRANSFER_ENCODING_MISSING))
			fprintf(fp, "%s", "Missing");
		else if (MIME_FLAG(MIME_TRANSFER_ENCODING_UNKNOWN))
			fprintf(fp, "%s: %s", "Unsupported", ptr->mime_hints.encoding);
		else
			fprintf(fp, "%s", content_encodings[ptr->encoding]);
		fprintf(fp, "\n\t\tUsed:\t\t%s", content_encodings[ptr->encoding]);

		if (artinfo->tex2iso)
			fprintf(fp, "\n\n\t\t%s\n", _(txt_tex));

#ifdef DEBUG
		if ((filename_star = get_param(ptr->params, "filename*")) != NULL) {
			fprintf(fp, "\n\tFilename*:\n");
			fprintf(fp, "\t\t%s", filename_star);
		}
		if ((filename = get_param(ptr->params, "filename")) != NULL) {
			fprintf(fp, "\n\tFilename:\n");
			fprintf(fp, "\t\t%s", filename);
		}
		if ((name = get_param(ptr->params, "name")) != NULL) {
			fprintf(fp, "\n\tName:\n");
			fprintf(fp, "\t\t%s", name);
		}
		if (ptr->description != NULL) {
			fprintf(fp, "\n\tContent Description:\n");
			fprintf(fp, "\t\t%s", ptr->description);
		}
		if (ptr->language != NULL) {
			fprintf(fp, "\n\tContent Language:\n");
			fprintf(fp, "\t\t%s", ptr->language);
		}
		if (ptr->disposition != DISP_NONE) {
			fprintf(fp, "\n\tContent Disposition:\n");
			fprintf(fp, "\t\t%s", ptr->disposition == DISP_INLINE ? content_disposition[DISP_INLINE] : content_disposition[DISP_ATTACHMENT]);
		}
		fprintf(fp, "\n");
		if (filename_star && *filename_star && filename && *filename && strcmp(filename_star, filename))
			fprintf(fp, "\n!!\tFilename*: %s != Filename: %s", filename_star, filename);
		if (filename_star && *filename_star && name && *name && strcmp(filename_star, name))
			fprintf(fp, "\n!!\tFilename*: %s != Name: %s", filename_star, name);
		if (filename && *filename && name && *name && strcmp(filename, name))
			fprintf(fp, "\n!!\tFilename: %s != Name: %s", filename, name);
#endif /* DEBUG */
		fprintf(fp, "\n");
	}
}


static void
draw_percent_mark(
	long cur_num,
	long max_num)
{
	char *buf;
	int len;
	size_t sz;

	if (NOTESLINES <= 0)
		return;

	if (cur_num <= 0 && max_num <= 0)
		return;

	if ((len = snprintf(NULL, 0, "%s(%d%%) [%ld/%ld]", _(txt_more), (int) (cur_num * 100 / max_num), cur_num, max_num)) < 0)
		return;

	sz = (size_t) len + 1;
	buf = my_malloc(sz);
	if (snprintf(buf, sz, "%s(%d%%) [%ld/%ld]", _(txt_more), (int) (cur_num * 100 / max_num), cur_num, max_num) != len) {
		free(buf);
		return;
	}

	len = strwidth(buf);
	clear_message();
	MoveCursor(cLINES, cCOLS - len - (1 + BLANK_PAGE_COLS));
#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
	StartInverse();
	my_fputs(buf, stdout);
	EndInverse();
	my_flush();
	free(buf);
}
