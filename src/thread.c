/*
 *  Project   : tin - a Usenet reader
 *  Module    : thread.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2006-07-20
 *  Notes     :
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>
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


#define IS_EXPIRED(a) ((a)->article == ART_UNAVAILABLE || arts[(a)->article].thread == ART_EXPIRED)

int thread_basenote = 0;				/* Index in base[] of basenote */
static int thread_respnum = 0;			/* Index in arts[] of basenote ie base[thread_basenote] */
t_bool show_subject;

/*
 * Local prototypes
 */
static char get_art_mark(struct t_article *art);
static int enter_pager(int art, t_bool ignore_unavail, int level);
static int mark_art_read(struct t_group *group);
static int thread_catchup(t_function func);
static int thread_tab_pressed(void);
static t_bool find_unexpired(struct t_msgid *ptr);
static t_bool has_sibling(struct t_msgid *ptr);
static t_function thread_left(void);
static t_function thread_right(void);
static void build_tline(int l, struct t_article *art);
static void draw_thread_arrow(void);
static void draw_thread_item(int item);
static void make_prefix(struct t_msgid *art, char *prefix, int maxlen);
static void show_thread_page(void);
static void update_thread_page(void);


/*
 * thdmenu.curr		Current screen cursor position in thread
 * thdmenu.max		Essentially = # threaded arts in current thread
 * thdmenu.first	Response # at top of screen
 */
static t_menu thdmenu = {0, 0, 0, show_thread_page, draw_thread_arrow, draw_thread_item };

/* TODO: find a better solution */
static int ret_code = 0;		/* Set to < 0 when it is time to leave this menu */

/*
 * returns the mark which should be used for this article
 */
static char
get_art_mark(
	struct t_article *art)
{
	if (art->inrange) {
		return tinrc.art_marked_inrange;
	} else if (art->status == ART_UNREAD) {
		return (art->selected ? tinrc.art_marked_selected : (tinrc.recent_time && ((time((time_t) 0) - art->date) < (tinrc.recent_time * DAY))) ? tinrc.art_marked_recent : tinrc.art_marked_unread);
	} else if (art->status == ART_WILL_RETURN) {
		return tinrc.art_marked_return;
	} else if (art->killed && tinrc.kill_level != KILL_NOTHREAD) {
		return tinrc.art_marked_killed;
	} else {
		if (/* tinrc.kill_level != KILL_UNREAD && */ art->score >= tinrc.score_select)
			return tinrc.art_marked_read_selected; /* read hot chil^H^H^H^H article */
		else
			return tinrc.art_marked_read;
	}
}


/*
 * Build one line of the thread page display. Looks long winded, but
 * there are a lot of variables in the format for the output
 *
 * WARNING: some other code expects to find the article mark (ART_MARK_READ,
 * ART_MARK_SELECTED, etc) at MARK_OFFSET from beginning of the line.
 * So, if you change the format used in this routine, be sure to check that
 * the value of MARK_OFFSET (tin.h) is still correct.
 * Yes, this is somewhat kludgy.
 */
static void
build_tline(
	int l,
	struct t_article *art)
{
	char mark;
	int gap, fill, i;
	int rest_of_line = cCOLS;
	int len_from, len_subj;
	struct t_msgid *ptr;
	char *buffer;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp, *wtmp2;
	char tmp[BUFSIZ];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

#ifdef USE_CURSES
	/*
	 * Allocate line buffer
	 * make it the same size like in !USE_CURSES case to simplify some code
	 */
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		buffer = my_malloc(cCOLS * MB_CUR_MAX + 2);
#	else
		buffer = my_malloc(cCOLS + 2);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#else
	buffer = screen[INDEX2SNUM(l)].col;
#endif /* USE_CURSES */

	/*
	 * Start with 2 spaces for ->
	 * then index number of the message and whitespace (2+4+1 chars)
	 */
#if 0 /* usefull? see also group.c:build_sline() */
	if (!tinrc.draw_arrow)
		sprintf(buffer, "%s ", tin_ltoa(l + 1, 6));
	else
#endif /* 0 */
	sprintf(buffer, "  %s ", tin_ltoa(l + 1, 4));
	rest_of_line -= 7;

	/*
	 * Add the article flags, tag number, or whatever (3 chars)
	 */
	rest_of_line -= 3;
	if (art->tagged) {
		strcat(buffer, tin_ltoa(art->tagged, 3));
		mark = '\0';
	} else {
		strcat(buffer, "   ");
		mark = get_art_mark(art);
		buffer[MARK_OFFSET] = mark;			/* insert mark */
	}

	strcat(buffer, "  ");					/* 2 more spaces */
	rest_of_line -= 2;

	/*
	 * Add the number of lines and/or the score if enabled
	 * (inside "[,]", 1+4[+1+6]+1+2 chars total)
	 */
	if (curr_group->attribute->show_info != SHOW_INFO_NOTHING) { /* add [ */
		strcat(buffer, "[");
		rest_of_line--;
	}

	if (curr_group->attribute->show_info == SHOW_INFO_LINES || curr_group->attribute->show_info == SHOW_INFO_BOTH) { /* add lines */
		strcat(buffer, ((art->line_count != -1) ? tin_ltoa(art->line_count, 4): "   ?"));
		rest_of_line -= 4;
	}

	if (curr_group->attribute->show_info == SHOW_INFO_SCORE || curr_group->attribute->show_info == SHOW_INFO_BOTH) {
		if (tinrc.show_info == SHOW_INFO_BOTH) { /* insert a separator if show lines and score */
			strcat(buffer, ",");
			rest_of_line--;
		}
		strcat(buffer, tin_ltoa(art->score, 6));
		rest_of_line -= 6;
	}

	if (curr_group->attribute->show_info != SHOW_INFO_NOTHING) { /* add closing ] and two spaces */
		strcat(buffer, "]  ");
		rest_of_line -= 3;
	}

	/*
	 * There are two formats for the rest of the line:
	 * 1) subject + optional author info
	 * 2) mandatory author info (eg, if subject threading)
	 *
	 * Add the subject and author information if required
	 */
	if (show_subject) {
		if (curr_group->attribute->show_author == SHOW_FROM_NONE)
				len_from = 0;
		else {
			len_from = rest_of_line;

			if (curr_group->attribute->show_author == SHOW_FROM_BOTH)
				len_from /= 2; /* if SHOW_FROM_BOTH use 50% for author info */
			else
				len_from /= 3; /* otherwise use 33% for author info */

			if (len_from < 0) /* security check - small screen? */
				len_from = 0;
		}
		rest_of_line -= len_from;
		len_subj = rest_of_line - (len_from ? 2 : 0);

		/*
		 * Mutt-like thread tree. by sjpark@sparcs.kaist.ac.kr
		 * Insert tree-structure strings "`->", "+->", ...
		 */

		make_prefix(art->refptr, buffer + strlen(buffer), len_subj);

		/*
		 * Copy in the subject up to where the author (if any) starts
		 */
		gap = cCOLS - strlen(buffer) - len_from; /* gap = gap (no. of chars) between tree and author/border of window */

		if (len_from)	/* Leave gap before author */
			gap -= 2;

		/*
		 * Mutt-like thread tree. by sjpark@sparcs.kaist.ac.kr
		 * Hide subject if same as parent's.
		 */
		if (gap > 0) {
			size_t len = strlen(buffer);

			for (ptr = art->refptr->parent; ptr && IS_EXPIRED(ptr); ptr = ptr->parent)
				;
			if (!(ptr && arts[ptr->article].subject == art->subject)) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if ((wtmp = char2wchar_t(art->subject)) != NULL) {
					wtmp2 = wcspart(wtmp, gap, TRUE);
					if (wcstombs(tmp, wtmp2, sizeof(tmp) - 1) != (size_t) -1)
						strncat(buffer, tmp, cCOLS * MB_CUR_MAX - len - 1);

					free(wtmp);
					free(wtmp2);
				}
			}
#else
				strncat(buffer, art->subject, gap);
			}
			buffer[len + gap] = '\0';	/* Just in case */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}

		/*
		 * If we need to show the author, pad out to the start of the author field,
		 */
		if (len_from) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if ((wtmp = char2wchar_t(buffer)) != NULL) {
				fill = cCOLS - len_from - wcswidth(wtmp, wcslen(wtmp) + 1);
				free(wtmp);
			} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				fill = cCOLS - len_from - strlen(buffer);

			gap = strlen(buffer);
			for (i = 0; i < fill; i++)
				buffer[gap + i] = ' ';
			buffer[gap + fill] = '\0';

			/*
			 * Now add the author info at the end. This will be 0 terminated
			 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			get_author(TRUE, art, tmp, sizeof(tmp) - 1);

			if ((wtmp = char2wchar_t(tmp)) != NULL) {
				wtmp2 = wcspart(wtmp, len_from, TRUE);
				if (wcstombs(tmp, wtmp2, sizeof(tmp) - 1) != (size_t) -1)
					strncat(buffer, tmp, cCOLS * MB_CUR_MAX - strlen(buffer) - 1);

				free(wtmp);
				free(wtmp2);
			}
#else
			get_author(TRUE, art, buffer + strlen(buffer), len_from);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}

	} else { /* Add the author info. This is always shown if subject is not */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		get_author(TRUE, art, tmp, sizeof(tmp) - 1);

		if ((wtmp = char2wchar_t(tmp)) != NULL) {
			wtmp2 = wcspart(wtmp, cCOLS - strlen(buffer), TRUE);
			if (wcstombs(tmp, wtmp2, sizeof(tmp) - 1) != (size_t) -1)
				strncat(buffer, tmp, cCOLS * MB_CUR_MAX - strlen(buffer) - 1);

			free(wtmp);
			free(wtmp2);
		}
#else
		get_author(TRUE, art, buffer + strlen(buffer), cCOLS - strlen(buffer));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	}

	/* protect display from non-displayable characters (e.g., form-feed) */
	convert_to_printable(buffer);

	if (!tinrc.strip_blanks) {
		/*
		 * Pad to end of line so that inverse bar looks 'good'
		 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wtmp = char2wchar_t(buffer)) != NULL) {
			fill = cCOLS - wcswidth(wtmp, wcslen(wtmp) + 1);
			free(wtmp);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			fill = cCOLS - strlen(buffer);

		gap = strlen(buffer);
		for (i = 0; i < fill; i++)
			buffer[gap + i] = ' ';

		buffer[gap + fill] = '\0';
	}

	WriteLine(INDEX2LNUM(l), buffer);

#ifdef USE_CURSES
	free(buffer);
#endif /* USE_CURSES */

	if (mark == tinrc.art_marked_selected)
		draw_mark_selected(l);
}


static void
draw_thread_item(
	int item)
{
	build_tline(item, &arts[find_response(thread_basenote, item)]);
	return;
}


static t_function
thread_left(
	void)
{
	if (tinrc.thread_catchup_on_exit)
		return SPECIAL_CATCHUP_LEFT;			/* ie, not via 'c' or 'C' */
	else
		return GLOBAL_QUIT;
}


static t_function
thread_right(
	void)
{
	return THREAD_READ_ARTICLE;
}


/*
 * Show current thread.
 * If threaded on Subject: show
 *   <respnum> <name>
 * If threaded on References: or Archive-name: show
 *   <respnum> <subject> <name>
 * Return values:
 *		GRP_RETSELECT	Return to selection screen
 *		GRP_QUIT		'Q'uit all the way out
 *		GRP_NEXT		Catchup goto next group
 *		GRP_NEXTUNREAD	Catchup enter next unread thread
 *		GRP_KILLED		Thread was killed at art level?
 *		GRP_EXIT		Return to group menu
 */
int
thread_page(
	struct t_group *group,
	int respnum,				/* base[] article of thread to view */
	int thread_depth,			/* initial depth in thread */
	t_pagerinfo *page)			/* !NULL if we must go direct to the pager */
{
	char key[MAXKEYLEN];
	char mark[] = { '\0', '\0' };
	int i, n;
	t_bool repeat_search;
	t_function func;

	thread_respnum = respnum;		/* Bodge to make this variable global */

	if ((n = which_thread(thread_respnum)) >= 0)
		thread_basenote = n;
	if ((thdmenu.max = num_of_responses(thread_basenote) + 1) <= 0) {
		info_message(_(txt_no_resps_in_thread));
		return GRP_EXIT;
	}

	/*
	 * Set the cursor to the last response unless pos_first_unread is on
	 * or an explicit thread_depth has been specified
	 */
	thdmenu.curr = thdmenu.max;
	/* reset the first item on screen to 0 */
	thdmenu.first = 0;

	if (thread_depth)
		thdmenu.curr = thread_depth;
	else {
		if (tinrc.pos_first_unread) {
			if ((i = new_responses(thread_basenote))) {
				for (n = 0, i = (int) base[thread_basenote]; i >= 0; i = arts[i].thread, n++) {
					if (arts[i].status == ART_UNREAD || arts[i].status == ART_WILL_RETURN) {
						if (arts[i].thread == ART_EXPIRED)
							art_mark(group, &arts[i], ART_READ);
						else
							thdmenu.curr = n;
						break;
					}
				}
			}
		}
	}

	if (thdmenu.curr < 0)
		thdmenu.curr = 0;

	/*
	 * See if we're on a direct call from the group menu to the pager
	 */
	if (page) {
		if ((ret_code = enter_pager(page->art, page->ignore_unavail, GROUP_LEVEL)) != 0)
			return ret_code;
		/* else fall through to stay in thread level */
	}

	/* Now we know where the cursor is, actually put something on the screen */
	show_thread_page();

	/* reset ret_code */
	ret_code = 0;
	while (ret_code >= 0) {
		set_xclick_on();
		if ((func = handle_keypad(thread_left, thread_right, global_mouse_action, thread_keys)) == GLOBAL_SEARCH_REPEAT) {
			func = last_search;
			repeat_search = TRUE;
		} else
			repeat_search = FALSE;

		switch (func) {
			case GLOBAL_ABORT:			/* Abort */
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
				if (thdmenu.max == 1)
					info_message(_(txt_no_responses));
				else
					prompt_item_num(func_to_key(func, thread_keys), _(txt_select_art));
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_FIRST_PAGE:		/* show first page of articles */
				top_of_list();
				break;

			case GLOBAL_LAST_PAGE:		/* show last page of articles */
				end_of_list();
				break;

			case GLOBAL_LAST_VIEWED:	/* show last viewed article */
				if (this_resp < 0 || (which_thread(this_resp) == -1)) {
					info_message(_(txt_no_last_message));
					break;
				}
				ret_code = enter_pager(this_resp, FALSE, THREAD_LEVEL);
				break;

			case GLOBAL_SET_RANGE:		/* set range */
				if (set_range(THREAD_LEVEL, 1, thdmenu.max, thdmenu.curr + 1))
					show_thread_page();
				break;

			case GLOBAL_PIPE:			/* pipe article(s) to command */
				if (thread_basenote >= 0)
					feed_articles(FEED_PIPE, THREAD_LEVEL, group, find_response(thread_basenote, thdmenu.curr));
				break;

#ifndef DISABLE_PRINTING
			case GLOBAL_PRINT:			/* print article(s) */
				if (thread_basenote >= 0)
					feed_articles(FEED_PRINT, THREAD_LEVEL, group, find_response(thread_basenote, thdmenu.curr));
				break;
#endif /* !DISABLE_PRINTING */

			case THREAD_MAIL:	/* mail article(s) to somebody */
				if (thread_basenote >= 0)
					feed_articles(FEED_MAIL, THREAD_LEVEL, group, find_response(thread_basenote, thdmenu.curr));
				break;

			case THREAD_SAVE:	/* save articles with prompting */
				if (thread_basenote >= 0)
					feed_articles(FEED_SAVE, THREAD_LEVEL, group, find_response(thread_basenote, thdmenu.curr));
				break;

			case THREAD_AUTOSAVE:	/* Auto-save articles without prompting */
				if (thread_basenote >= 0)
					feed_articles(FEED_AUTOSAVE, THREAD_LEVEL, group, (int) base[grpmenu.curr]);
				break;

			case GLOBAL_MENU_FILTER_SELECT:
			case GLOBAL_MENU_FILTER_KILL:
				/*
				 * FIXME: a filter which kills all remaining messages in a thread
				 *        is 'troublesome'
				 */
				n = find_response(thread_basenote, thdmenu.curr);
				filter_menu(func, group, &arts[n]);
				if (filter_articles(group)) {
					make_threads(group, FALSE);
					if ((n = next_unread(n)) == -1) {
						ret_code = GRP_EXIT;
						break;
					}
					fixup_thread(n, TRUE);
				}
				show_thread_page();
				break;

			case GLOBAL_EDIT_FILTER:
				if (!invoke_editor(filter_file, FILTER_FILE_OFFSET))
					break;
				unfilter_articles();
				(void) read_filter_file(filter_file);
				if (filter_articles(group)) {
					make_threads(group, FALSE);
					if ((n = next_unread(n)) == -1) {
						ret_code = GRP_EXIT;
						break;
					}
					fixup_thread(n, TRUE);
				}
				show_thread_page();
				break;

			case THREAD_READ_ARTICLE:	/* read current article within thread */
				ret_code = enter_pager(find_response(thread_basenote, thdmenu.curr), FALSE, THREAD_LEVEL);
				break;

			case THREAD_READ_NEXT_ARTICLE_OR_THREAD:
				ret_code = thread_tab_pressed();
				break;

			case GLOBAL_POST:		/* post a basenote */
				if (post_article(group->name))
					show_thread_page();
				break;

			case GLOBAL_REDRAW_SCREEN:	/* redraw screen */
				my_retouch();
				set_xclick_off();
				show_thread_page();
				break;

			case GLOBAL_LINE_DOWN:
				move_down();
				break;

			case GLOBAL_LINE_UP:
				move_up();
				break;

			case GLOBAL_PAGE_UP:
				page_up();
				break;

			case GLOBAL_PAGE_DOWN:
				page_down();
				break;

			case GLOBAL_SCROLL_DOWN:
				scroll_down();
				break;

			case GLOBAL_SCROLL_UP:
				scroll_up();
				break;

			case SPECIAL_CATCHUP_LEFT:				/* come here when exiting thread via <- */
			case CATCHUP:				/* catchup thread, move to next one */
			case CATCHUP_NEXT_UNREAD:	/* -> next with unread arts */
				ret_code = thread_catchup(func);
				break;

			case THREAD_MARK_ARTICLE_READ: /* mark article as read */
				ret_code = mark_art_read(group);
				break;

			case THREAD_TOGGLE_SUBJECT_DISPLAY:	/* toggle display of subject & subj/author */
				if (show_subject) {
					if (++curr_group->attribute->show_author > SHOW_FROM_BOTH)
						curr_group->attribute->show_author = SHOW_FROM_NONE;
					show_thread_page();
				}
				break;

			case GLOBAL_OPTION_MENU:
				change_config_file(group);
				n = find_response(thread_basenote, thdmenu.curr);
				if (which_thread(n) == -1) /* We have lost the thread */
					ret_code = GRP_EXIT;
				else {
					fixup_thread(n, FALSE);
					thdmenu.curr = which_response(n);
					show_thread_page();
				}
				break;

			case GLOBAL_HELP:					/* help */
				show_help_page(THREAD_LEVEL, _(txt_thread_com));
				show_thread_page();
				break;

			case GLOBAL_LOOKUP_MESSAGEID:
				if ((n = prompt_msgid()) != ART_UNAVAILABLE)
					ret_code = enter_pager(n, FALSE, THREAD_LEVEL);
				break;

			case GLOBAL_SEARCH_BODY:			/* search article body */
				if ((n = search_body(group, find_response(thread_basenote, thdmenu.curr), repeat_search)) != -1) {
					fixup_thread(n, FALSE);
					ret_code = enter_pager(n, FALSE, THREAD_LEVEL);
				}
				break;

			case GLOBAL_SEARCH_AUTHOR_FORWARD:			/* author search */
			case GLOBAL_SEARCH_AUTHOR_BACKWARD:
			case GLOBAL_SEARCH_SUBJECT_FORWARD:			/* subject search */
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
				if ((n = search(func, find_response(thread_basenote, thdmenu.curr), repeat_search)) != -1)
					fixup_thread(n, TRUE);
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:		/* toggle mini help menu */
				toggle_mini_help(THREAD_LEVEL);
				show_thread_page();
				break;

			case GLOBAL_TOGGLE_INVERSE_VIDEO:	/* toggle inverse video */
				toggle_inverse_video();
				show_thread_page();
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:		/* toggle color */
				if (toggle_color()) {
					show_thread_page();
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case GLOBAL_QUIT:			/* return to previous level */
				ret_code = GRP_EXIT;
				break;

			case GLOBAL_QUIT_TIN:			/* quit */
				ret_code = GRP_QUIT;
				break;

			case THREAD_TAG:			/* tag/untag article */
				/* Find index of current article */
				if ((n = find_response(thread_basenote, thdmenu.curr)) < 0)
					break;
				else {
					t_bool tagged;

					if ((tagged = tag_article(n)))
						mark_screen(thdmenu.curr, MARK_OFFSET - 2, tin_ltoa((&arts[n])->tagged, 3));
					else
						update_thread_page();						/* Must update whole page */

					/* Automatically advance to next art if not at end of thread */
					if (thdmenu.curr + 1 < thdmenu.max)
						move_down();
					else
						draw_thread_arrow();

					info_message(tagged ? _(txt_prefix_tagged) : _(txt_prefix_untagged), txt_article_singular);
				}
				break;

			case GLOBAL_BUGREPORT:
				bug_report();
				break;

			case THREAD_UNTAG:			/* untag all articles */
				if (grpmenu.curr >= 0 && untag_all_articles())
					update_thread_page();
				break;

			case GLOBAL_VERSION:			/* version */
				info_message(cvers);
				break;

			case MARK_ARTICLE_UNREAD:	/* mark article as unread */
				n = find_response(thread_basenote, thdmenu.curr);
				art_mark(group, &arts[n], ART_WILL_RETURN);
				mark[0] = get_art_mark(&arts[n]);
				mark_screen(thdmenu.curr, MARK_OFFSET, mark);
				draw_thread_arrow();
				info_message(_(txt_marked_as_unread), _(txt_article_upper));
				break;

			case MARK_THREAD_UNREAD:		/* mark thread as unread */
				thd_mark_unread(group, base[thread_basenote]);
				update_thread_page();
				info_message(_(txt_marked_as_unread), _(txt_thread_upper));
				break;

			case THREAD_SELECT_ARTICLE:		/* mark article as selected */
			case THREAD_TOGGLE_ARTICLE_SELECTION:		/* toggle article as selected */
				if ((n = find_response(thread_basenote, thdmenu.curr)) < 0)
					break;
				arts[n].selected = (!(func == THREAD_TOGGLE_ARTICLE_SELECTION && arts[n].selected));	/* TODO: optimise? */
/*				update_thread_page(); */
				mark[0] = get_art_mark(&arts[n]);
				mark_screen(thdmenu.curr, MARK_OFFSET, mark);
				if (thdmenu.curr + 1 < thdmenu.max)
					move_down();
				else
					draw_thread_arrow();
				break;

			case THREAD_REVERSE_SELECTIONS:		/* reverse selections */
				for_each_art_in_thread(i, thread_basenote)
					arts[i].selected = bool_not(arts[i].selected);
				update_thread_page();
				break;

			case THREAD_UNDO_SELECTIONS:		/* undo selections */
				for_each_art_in_thread(i, thread_basenote)
					arts[i].selected = FALSE;
				update_thread_page();
				break;

			case GLOBAL_POSTPONED:		/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles(FALSE, FALSE))
						show_thread_page();
				} else
					info_message(_(txt_cannot_post));
				break;

			case GLOBAL_DISPLAY_POST_HISTORY:	/* display messages posted by user */
				if (user_posted_messages())
					show_thread_page();
				break;

			case GLOBAL_TOGGLE_INFO_LAST_LINE:		/* display subject in last line */
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				show_thread_page();
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, func_to_key(GLOBAL_HELP, thread_keys)));
		}
	} /* ret_code >= 0 */

	set_xclick_off();
	clear_note_area();

	return ret_code;
}


static void
show_thread_page(
	void)
{
	char *title;
	int i, art;

	signal_context = cThread;
	currmenu = &thdmenu;

	ClearScreen();
	set_first_screen_item();

	/*
	 * If threading by Refs, it helps to see the subject line
	 */
	show_subject = ((arts[thread_respnum].archive != NULL) || (curr_group->attribute->thread_arts == THREAD_REFS) || (curr_group->attribute->thread_arts == THREAD_BOTH));

	if (show_subject)
		title = fmt_string(_(txt_stp_list_thread), grpmenu.curr + 1, grpmenu.max);
	else
		title = fmt_string(_(txt_stp_thread), cCOLS - 23, arts[thread_respnum].subject);
	show_title(title);
	free(title);

	art = find_response(thread_basenote, thdmenu.first);
	for (i = thdmenu.first; i < thdmenu.first + NOTESLINES && i < thdmenu.max; ++i) {
		build_tline(i, &arts[art]);
		art = next_response(art);
	}

	show_mini_help(THREAD_LEVEL);
	draw_thread_arrow();
}


static void
update_thread_page(
	void)
{
	char mark[] = { '\0', '\0' };
	int i, the_index;

	the_index = find_response(thread_basenote, thdmenu.first);
	assert(thdmenu.first != 0 || the_index == thread_respnum);

	for (i = thdmenu.first; i < thdmenu.first + NOTESLINES && i < thdmenu.max; ++i) {
		if ((&arts[the_index])->tagged)
			mark_screen(i, MARK_OFFSET - 2, tin_ltoa((&arts[the_index])->tagged, 3));
		else {
			mark[0] = get_art_mark(&arts[the_index]);
			mark_screen(i, MARK_OFFSET - 2, "  ");	/* clear space used by tag numbering */
			mark_screen(i, MARK_OFFSET, mark);
			if (mark[0] == tinrc.art_marked_selected)
				draw_mark_selected(i);
		}
		if ((the_index = next_response(the_index)) == -1)
			break;
	}

	draw_thread_arrow();
}


static void
draw_thread_arrow(
	void)
{
	draw_arrow_mark(INDEX_TOP + thdmenu.curr - thdmenu.first);

	if (tinrc.info_in_last_line)
		info_message("%s", arts[find_response(thread_basenote, thdmenu.curr)].subject);
	else if (thdmenu.curr == thdmenu.max - 1)
		info_message(_(txt_end_of_thread));
}


/*
 * Fix all the internal pointers if the current thread/response has
 * changed.
 */
void
fixup_thread(
	int respnum,
	t_bool redraw)
{
	int basenote = which_thread(respnum);
	int old_thread_basenote = thread_basenote;

	if (basenote >= 0) {
		thread_basenote = basenote;
		thdmenu.max = num_of_responses(thread_basenote) + 1;
		thread_respnum = base[thread_basenote];
		grpmenu.curr = basenote;
		if (redraw && basenote != old_thread_basenote)
			show_thread_page();
	}

	if (redraw)
		move_to_item(which_response(respnum));		/* Redraw screen etc.. */
}


/*
 * Return the number of unread articles there are within a thread
 */
int
new_responses(
	int thread)
{
	int i;
	int sum = 0;

	for_each_art_in_thread(i, thread) {
		if (arts[i].status != ART_READ)
			sum++;
	}

	return sum;
}


/*
 * Which base note (an index into base[]) does a respnum (an index into
 * arts[]) correspond to?
 *
 * In other words, base[] points to an entry in arts[] which is the head of
 * a thread, linked with arts[].thread. For any q: arts[q], find i such that
 * base[i]->arts[n]->arts[o]->...->arts[q]
 *
 * Note that which_thread() can return -1 if in show_read_only mode and the
 * article of interest has been read as well as all other articles in the
 * thread, thus resulting in no base[] entry for it.
 */
int
which_thread(
	int n)
{
	int i, j;

	/* Move to top of thread */
	for (i = n; arts[i].prev >= 0; i = arts[i].prev)
		;
	/* Find in base[] */
	for (j = 0; j < grpmenu.max; j++) {
		if (base[j] == i)
			return j;
	}

	error_message(_(txt_cannot_find_base_art), n);
	return -1;
}


/*
 * Find how deep in its' thread arts[n] is. Start counting at zero
 */
int
which_response(
	int n)
{
	int i, j;
	int num = 0;

	if ((i = which_thread(n)) == -1)
		return 0;

	for_each_art_in_thread(j, i) {
		if (j == n)
			break;
		else
			num++;
	}

	return num;
}


/*
 * Given an index into base[], find the number of responses for
 * that basenote
 */
int
num_of_responses(
	int n)
{
	int i;
	int oldi = -3;
	int sum = 0;

	assert(n < grpmenu.max && n >= 0);

	for_each_art_in_thread(i, n) {
		assert(i != ART_EXPIRED);
		assert(i != oldi);
		oldi = i;
		sum++;
	}

	return sum - 1;
}


/*
 * Calculating the score of a thread has been extracted from stat_thread()
 * because we need it also in art.c to sort base[].
 * get_score_of_thread expects the number of the first article of a thread.
 */
int
get_score_of_thread(
	int n)
{
	int i;
	int j = 0;
	int score = 0;

	for (i = n; i >= 0; i = arts[i].thread) {
		/*
		 * TODO: do we want to take the score of read articles into account?
		 */
		if (arts[i].status != ART_READ || arts[i].killed == ART_KILLED_UNREAD /* || tinrc.kill_level == KILL_THREAD */) {
			if (tinrc.thread_score == THREAD_SCORE_MAX) {
				/* we use the maximum article score for the complete thread */
				if ((arts[i].score > score) && (arts[i].score > 0))
					score = arts[i].score;
				else {
					if ((arts[i].score < score) && (score <= 0))
						score = arts[i].score;
				}
			} else { /* tinrc.thread_score >= THREAD_SCORE_SUM */
				/* sum scores of unread arts and count num. arts */
				score += arts[i].score;
				j++;
			}
		}
	}
	if (j && tinrc.thread_score == THREAD_SCORE_WEIGHT)
		score /= j;

	return score;
}


/*
 * Given an index into base[], return relevant statistics
 */
int
stat_thread(
	int n,
	struct t_art_stat *sbuf) /* return value is always ignored */
{
	int i;
	MultiPartInfo minfo;

	sbuf->total = 0;
	sbuf->unread = 0;
	sbuf->seen = 0;
	sbuf->deleted = 0;
	sbuf->inrange = 0;
	sbuf->selected_total = 0;
	sbuf->selected_unread = 0;
	sbuf->selected_seen = 0;
	sbuf->killed = 0;
	sbuf->art_mark = tinrc.art_marked_read;
	sbuf->score = 0 /* -(SCORE_MAX) */;
	sbuf->time = 0;
	sbuf->multipart_compare_len = 0;
	sbuf->multipart_total = 0;
	sbuf->multipart_have = 0;

	for_each_art_in_thread(i, n) {
		++sbuf->total;
		if (arts[i].inrange)
			++sbuf->inrange;

		if (arts[i].delete_it)
			++sbuf->deleted;

		if (arts[i].status == ART_UNREAD) {
			++sbuf->unread;

			if (arts[i].date > sbuf->time)
				sbuf->time = arts[i].date;
		} else if (arts[i].status == ART_WILL_RETURN)
			++sbuf->seen;

		if (arts[i].selected) {
			++sbuf->selected_total;
			if (arts[i].status == ART_UNREAD)
				++sbuf->selected_unread;
			else if (arts[i].status == ART_WILL_RETURN)
				++sbuf->selected_seen;
		}

		if (arts[i].killed)
			++sbuf->killed;

		if ((curr_group->attribute->thread_arts == THREAD_MULTI) && global_get_multipart_info(i, &minfo) && (minfo.total >= 1)) {
			sbuf->multipart_compare_len = minfo.subject_compare_len;
			sbuf->multipart_total = minfo.total;
			sbuf->multipart_have++;
		}
	}

	sbuf->score = get_score_of_thread((int) base[n]);

	if (sbuf->inrange)
		sbuf->art_mark = tinrc.art_marked_inrange;
	else if (sbuf->deleted)
		sbuf->art_mark = tinrc.art_marked_deleted;
	else if (sbuf->selected_unread)
		sbuf->art_mark = tinrc.art_marked_selected;
	else if (sbuf->unread) {
		if (tinrc.recent_time && (time((time_t) 0) - sbuf->time) < (tinrc.recent_time * DAY))
			sbuf->art_mark = tinrc.art_marked_recent;
		else
			sbuf->art_mark = tinrc.art_marked_unread;
	}
	else if (sbuf->seen)
		sbuf->art_mark = tinrc.art_marked_return;
	else if (sbuf->selected_total)
		sbuf->art_mark = tinrc.art_marked_read_selected;
	else if (sbuf->killed == sbuf->total)
		sbuf->art_mark = tinrc.art_marked_killed;
	else
		sbuf->art_mark = tinrc.art_marked_read;
	return sbuf->total;
}


/*
 * Find the next response to arts[n]. Go to the next basenote if there
 * are no more responses in this thread
 */
int
next_response(
	int n)
{
	int i;

	if (arts[n].thread >= 0)
		return arts[n].thread;

	i = which_thread(n) + 1;

	if (i >= grpmenu.max)
		return -1;

	return (int) base[i];
}


/*
 * Given a respnum (index into arts[]), find the respnum of the
 * next basenote
 */
int
next_thread(
	int n)
{
	int i;

	i = which_thread(n) + 1;
	if (i >= grpmenu.max)
		return -1;

	return (int) base[i];
}


/*
 * Find the previous response. Go to the last response in the previous
 * thread if we go past the beginning of this thread.
 * Return -1 if we are at the start of the group
 */
int
prev_response(
	int n)
{
	int i;

	if (arts[n].prev >= 0)
		return arts[n].prev;

	i = which_thread(n) - 1;

	if (i < 0)
		return -1;

	return find_response(i, num_of_responses(i));
}


/*
 * return index in arts[] of the 'n'th response in thread base 'i'
 */
int
find_response(
	int i,
	int n)
{
	int j;

	j = (int) base[i];

	while (n-- > 0 && arts[j].thread >= 0)
		j = arts[j].thread;

	return j;
}


/*
 * Find the next unread response to art[n] in this group. If no response is
 * found from current point to the end restart from beginning of articles.
 * If no more responses can be found, return -1
 */
int
next_unread(
	int n)
{
	int cur_base_art = n;

	while (n >= 0) {
		if (((arts[n].status == ART_UNREAD) || (arts[n].status == ART_WILL_RETURN)) && arts[n].thread != ART_EXPIRED)
			return n;

		n = next_response(n);
	}

	if (tinrc.wrap_on_next_unread) {
		n = base[0];
		while (n != cur_base_art) {
			if (((arts[n].status == ART_UNREAD) || (arts[n].status == ART_WILL_RETURN)) && arts[n].thread != ART_EXPIRED)
				return n;

			n = next_response(n);
		}
	}

	return -1;
}


/*
 * Find the previous unread response in this thread
 */
int
prev_unread(
	int n)
{
	while (n >= 0) {
		if (arts[n].status != ART_READ && arts[n].thread != ART_EXPIRED)
			return n;

		n = prev_response(n);
	}

	return -1;
}


static t_bool
find_unexpired(
	struct t_msgid *ptr)
{
	return ptr && (!IS_EXPIRED(ptr) || find_unexpired(ptr->child) || find_unexpired(ptr->sibling));
}


static t_bool
has_sibling(
	struct t_msgid *ptr)
{
	do {
		if (find_unexpired(ptr->sibling))
			return TRUE;
		ptr = ptr->parent;
	} while (ptr && IS_EXPIRED(ptr));
	return FALSE;
}


/*
 * mutt-like subject according. by sjpark@sparcs.kaist.ac.kr
 * string in prefix will be overwritten up to length len prefix will always
 * be terminated with \0
 * make sure prefix is at least len+1 bytes long (to hold the terminating
 * null byte)
 */
static void
make_prefix(
	struct t_msgid *art,
	char *prefix,
	int maxlen)
{
	char *buf;
	int prefix_ptr;
	int depth = 0;
	int depth_level = 0;
	struct t_msgid *ptr;

	for (ptr = art->parent; ptr; ptr = ptr->parent)
		depth += (!IS_EXPIRED(ptr) ? 1 : 0);

	if ((depth == 0) || (maxlen < 1)) {
		prefix[0] = '\0';
		return;
	}

	prefix_ptr = depth * 2 - 1;

	if (prefix_ptr > maxlen - 1 - !(maxlen % 2)) {
		int odd = ((maxlen % 2) ? 0 : 1);

		prefix_ptr -= maxlen - ++depth_level - 2 - odd;

		while (prefix_ptr > maxlen - 2 - odd) {
			if (depth_level < maxlen / 5)
				depth_level++;
			prefix_ptr -= maxlen - depth_level - 2 - odd;
			odd = (odd ? 0 : 1);
		}
	}

	buf = my_malloc(prefix_ptr + 3);
	strcpy(&buf[prefix_ptr], "->");
	buf[--prefix_ptr] = (has_sibling(art) ? '+' : '`');

	for (ptr = art->parent; prefix_ptr > 1; ptr = ptr->parent) {
		if (IS_EXPIRED(ptr))
			continue;
		buf[--prefix_ptr] = ' ';
		buf[--prefix_ptr] = (has_sibling(ptr) ? '|' : ' ');
	}

	while (depth_level)
		buf[--depth_level] = '>';

	strncpy(prefix, buf, maxlen);
	prefix[maxlen] = '\0'; /* just in case strlen(buf) > maxlen */
	free(buf);
	return;
}


/*
 * There are 3 catchup methods:
 * When exiting thread via <-
 * Catchup thread, move to next one
 * Catchup thread and enter next one with unread arts
 * Return a suitable ret_code
 */
static int
thread_catchup(
	t_function func)
{
	char buf[LEN];
	int i, n;
	int pyn = 1;

	/* Find first unread art in this thread */
	n = ((thdmenu.curr == 0) ? thread_respnum : find_response(thread_basenote, 0));
	for (i = n; i != -1; i = arts[i].thread) {
		if ((arts[i].status == ART_UNREAD) || (arts[i].status == ART_WILL_RETURN))
			break;
	}

	if (i != -1) {				/* still unread arts in this thread */
		/*
		 * TODO: if (group->attribute->thread_arts == THREAD_NONE)
		 *          snprintf(buf, sizeof(buf), _("Mark article as read%s?"), (func == CATCHUP_NEXT_UNREAD) ? _(" and enter next unread article") : "");
		 *       else
		 */
		snprintf(buf, sizeof(buf), _(txt_mark_thread_read), (func == CATCHUP_NEXT_UNREAD) ? _(txt_enter_next_thread) : "");
		if ((!TINRC_CONFIRM_ACTION) || (pyn = prompt_yn(buf, TRUE)) == 1)
			thd_mark_read(curr_group, base[thread_basenote]);
	}

	switch (func) {
		case CATCHUP:				/* 'c' */
			if (pyn == 1)
				return GRP_NEXT;
			break;

		case CATCHUP_NEXT_UNREAD:	/* 'C' */
			if (pyn == 1)
				return GRP_NEXTUNREAD;
			break;

		case SPECIAL_CATCHUP_LEFT:			/* <- thread catchup on exit */
			switch (pyn) {
				case -1:				/* ESC from prompt, stay in group */
					break;

				case 1:					/* We caught up - advance group */
					return GRP_NEXT;

				default:				/* Just leave the group */
					return GRP_EXIT;
			}
			/* FALLTHROUGH */
		default:
			break;
	}
	return 0;							/* Default is to stay in current screen */
}


/*
 * This is the single entry point into the article pager
 * art
 *		is the arts[art] we wish to read
 * ignore_unavail
 *		should be set if we wish to keep going after article unavailable
 * level
 *		is the menu from which we came. This should be only be GROUP or THREAD
 *		it is used to set the return code to go back to the calling menu when
 *		not explicitly set
 * Return:
 *	<0 to quit to group menu
 *	 0 to stay in thread menu
 *  >0 after normal exit from pager to return to previous menu level
 */
static int
enter_pager(
	int art,
	t_bool ignore_unavail,
	int level)
{
	int i;

again:
	switch ((i = show_page(curr_group, art, &thdmenu.curr))) {
		/* These exit to previous menu level */
		case GRP_QUIT:				/* 'Q' all the way out */
		case GRP_RETSELECT:			/* 'T' back to select menu */
		case GRP_NEXT:				/* 'c' Move to next thread on group menu */
		case GRP_NEXTUNREAD:		/* 'C' */
			break;

		case GRP_ARTABORT:			/* user 'q'uit load of article */
			/* break forces return to group menu */
			if (level == GROUP_LEVEL)
				break;
			/* else stay on thread menu */
			show_thread_page();
			return 0;

		/* Keeps us in thread menu */
		case GRP_ARTUNAVAIL:
			if (ignore_unavail && (art = next_unread(art)) != -1)
				goto again;
			else if (level == GROUP_LEVEL)
				return GRP_ARTABORT;
			/* back to thread menu */
			show_thread_page();
			return 0;

		case GRP_GOTOTHREAD:		/* 'l' from pager */
			show_thread_page();
			move_to_item(which_response(this_resp));
			return 0;

		default:					/* >=0 normal exit, new basenote */
			if (filtered_articles)
				return GRP_KILLED; /* ? set group cursor back to 0 and do nothing */
			fixup_thread(this_resp, FALSE);

			if (currmenu != &grpmenu)	/* group menu will redraw itself */
				currmenu->redraw();

			return 1;				/* Must return any +ve integer */
	}
	return i;
}


/*
 * Find index in arts[] of next unread article _IN_THIS_THREAD_
 * Page it or return GRP_NEXTUNREAD if thread is all read
 * (to tell group menu to skip to next thread)
 */
static int
thread_tab_pressed(
	void)
{
	int i, n;

	/*
	 * Find current position in thread
	 */
	n = ((thdmenu.curr == 0) ? thread_respnum : find_response(thread_basenote, thdmenu.curr));

	/*
	 * Find and display next unread
	 */
	for (i = n; i != -1; i = arts[i].thread) {
		if ((arts[i].status == ART_UNREAD) || (arts[i].status == ART_WILL_RETURN))
			return (enter_pager(i, TRUE, THREAD_LEVEL));
	}

	/*
	 * We ran out of thread, tell group.c to enter the next with unread
	 */
	return GRP_NEXTUNREAD;
}


/*
 * If there are any tagged and unread articles, prompt user to mark either
 * all tagged arts as read, or only current article, or cancel operation.
 * Otherwise, use current article.
 * Finally move to next unread article.
 *
 * Return GRP_EXIT if there are no more unread articles in this group,
 * else return 0.
 */
static int
mark_art_read(
	struct t_group *group)
{
	char keytagged[MAXKEYLEN], keycurrent[MAXKEYLEN], keyquit[MAXKEYLEN];
	int n, cnt = 0;
	int tmp_num_of_tagged_arts = num_of_tagged_arts;
	t_function func = MARK_READ_CURRENT;

	if (!tinrc.mark_ignore_tags && got_tagged_unread_arts()) {
		func = prompt_slk_response(MARK_READ_TAGGED,
				mark_read_keys,
				_(txt_mark_art_read_tagged_current),
				printascii(keytagged, func_to_key(MARK_READ_TAGGED, mark_read_keys)),
				printascii(keycurrent, func_to_key(MARK_READ_CURRENT, mark_read_keys)),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, mark_read_keys)));
	}

	switch (func) {
		case MARK_READ_TAGGED: /* mark tagged unread articles as read */
			cnt = mark_tagged_read(group);
			show_thread_page();
			n = find_response(thread_basenote, thdmenu.curr);
			break;

		case MARK_READ_CURRENT: /* mark current article as read */
			n = find_response(thread_basenote, thdmenu.curr);
			if ((arts[n].status == ART_UNREAD) || (arts[n].status == ART_WILL_RETURN)) {
				char mark[] = { '\0', '\0' };

				art_mark(group, &arts[n], ART_READ);
				mark[0] = get_art_mark(&arts[n]);
				mark_screen(thdmenu.curr, MARK_OFFSET, mark);
			}
			break;

		case GLOBAL_QUIT: /* cancel operation */
		case GLOBAL_ABORT:
		default:
			return 0;
			/* NOTREACHED */
			break;
	}

	if ((n = next_unread(n)) == -1)	/* no more unread articles */
		return GRP_EXIT;
	else
		fixup_thread(n, TRUE);	/* We may be in the next thread now */

	if (func == MARK_READ_TAGGED)
		info_message(_(txt_marked_tagged_arts_as_read), cnt, tmp_num_of_tagged_arts, PLURAL(tmp_num_of_tagged_arts, txt_article));

	return 0;
}
