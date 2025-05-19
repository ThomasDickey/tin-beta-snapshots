/*
 *  Project   : tin - a Usenet reader
 *  Module    : group.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2025-04-04
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


/*
 * Globally accessible pointer to currently active group
 * Any functionality accessed from group level or below can use this pointer.
 * Any code invoked from selection level that requires a group context will
 * need to manually fix this up
 */
struct t_group *curr_group;
static struct t_fmt grp_fmt;

/*
 * Local prototypes
 */
static int do_search(t_function func, t_bool repeat);
static int enter_pager(int art, t_bool ignore_unavail);
static int enter_thread(int depth, t_pagerinfo *page);
static int find_new_pos(t_artnum old_artnum, int cur_pos);
static int group_catchup(t_function func);
static int tab_pressed(void);
static t_bool prompt_getart_limit(void);
static t_function group_left(void);
static t_function group_right(void);
static void build_sline(int i);
static void build_multipart_header(char *dest, int maxlen, const char *src, int cmplen, int have, int total);
static void draw_subject_arrow(void);
static void show_group_title(t_bool clear_title);
static void show_tagged_lines(void);
static void toggle_read_unread(t_bool force);
static void update_group_page(void);

/*
 * grpmenu.curr is an index into base[] and so equates to the cursor location
 * (thread number) on group page
 * grpmenu.first is static here
 */
t_menu grpmenu = { 0, 0, 0, show_group_page, draw_subject_arrow, build_sline };

/* TODO: find a better solution */
static int ret_code = 0;		/* Set to < 0 when it is time to leave the group level */

static void
show_tagged_lines(
	void)
{
	int i, j;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	for (i = grpmenu.first; i < grpmenu.first + NOTESLINES && i < grpmenu.max; ++i) {
		if ((i != grpmenu.curr) && (j = line_is_tagged((int) base[i]))) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if ((wtmp = char2wchar_t(tin_ltoa(j, 3)))) {
				mark_screen(i, mark_offset - (3 - art_mark_width), wtmp);
				free(wtmp);
			}
#else
			mark_screen(i, mark_offset - 2, tin_ltoa(j, 3));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}
	}
}


static t_function
group_left(
	void)
{
	if (curr_group->attribute->group_catchup_on_exit)
		return SPECIAL_CATCHUP_LEFT;		/* ie, not via 'c' or 'C' */

	return GLOBAL_QUIT;
}


static t_function
group_right(
	void)
{
	if (grpmenu.curr >= 0 && HAS_FOLLOWUPS(grpmenu.curr)) {
		if (curr_group->attribute->auto_list_thread)
			return GROUP_LIST_THREAD;
		else {
			int n = next_unread((int) base[grpmenu.curr]);

			if (n >= 0 && grpmenu.curr == which_thread(n)) {
				ret_code = enter_pager(n, TRUE);
				return GLOBAL_ABORT;	/* TODO: should we return something else? */
			}
		}
	}
	return GROUP_READ_BASENOTE;
}


/*
 * Return Codes:
 * GRP_EXIT			'Normal' return to selection menu
 * GRP_RETSELECT	We are en route from pager to the selection screen
 * GRP_QUIT			User has done a 'Q'
 * GRP_NEXT			User wants to move onto next group
 * GRP_NEXTUNREAD	User did a 'C'atchup
 * GRP_ENTER		'g' command has been used to set group to enter
 */
int
group_page(
	struct t_group *group)
{
	char key[MAXKEYLEN];
	int i, n, ii;
	int thread_depth;	/* Starting depth in threads we enter */
	t_artnum old_artnum;
	struct t_art_stat sbuf;
	struct t_article *art;
	t_bool flag;
	t_bool xflag = FALSE;	/* 'X'-flag */
	t_bool repeat_search;
	t_function func;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	/*
	 * Set the group attributes
	 */
	group->read_during_session = TRUE;

	curr_group = group;					/* For global access to the current group */
	num_of_tagged_arts = 0;
	range_active = FALSE;

	last_resp = -1;
	this_resp = -1;

	/*
	 * update index file. quit group level if user aborts indexing
	 */
	if (!index_group(group)) {
		for_each_art(i) {
			art = &arts[i];
			FreeAndNull(art->refs);
			FreeAndNull(art->msgid);
		}
		curr_group = NULL;
		tin_errno = 0;
		return GRP_RETSELECT;
	}

	/*
	 * Position 'grpmenu.curr' accordingly
	 */
	pos_first_unread_thread();
	/* reset grpmenu.first */
	grpmenu.first = 0;

	clear_note_area();

	if (group->attribute->auto_select) {
		error_message(2, _(txt_autoselecting_articles), PrintFuncKey(key, GROUP_MARK_UNSELECTED_ARTICLES_READ, group_keys));
		do_auto_select_arts();						/* 'X' command */
		xflag = TRUE;
	}

	show_group_page();

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_bitmap(group, NULL);
#endif /* DEBUG */

	/* reset ret_code */
	ret_code = 0;
	while (ret_code >= 0) {
		set_xclick_on();
		if ((func = handle_keypad(group_left, group_right, global_mouse_action, group_keys)) == GLOBAL_SEARCH_REPEAT) {
			func = last_search;
			repeat_search = TRUE;
		} else
			repeat_search = FALSE;

		switch (func) {
			case GLOBAL_ABORT:
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
				if (grpmenu.max)
					prompt_item_num(func_to_key(func, group_keys), group->attribute->thread_articles == THREAD_NONE ? _(txt_select_art) : _(txt_select_thread));
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_FIRST_PAGE:		/* show first page of threads */
				top_of_list();
				break;

			case GLOBAL_LAST_PAGE:		/* show last page of threads */
				end_of_list();
				break;

			case GLOBAL_LAST_VIEWED:	/* go to last viewed article */
				/*
				 * If the last art is no longer in a thread then we can't display it
				 */
				if (this_resp < 0 || (which_thread(this_resp) == -1))
					info_message(_(txt_no_last_message));
				else
					ret_code = enter_pager(this_resp, FALSE);
				break;

			case GLOBAL_PIPE:		/* pipe article/thread/tagged arts to command */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_PIPE, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;

			case GROUP_MAIL:	/* mail article to somebody */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_MAIL, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;

#ifndef DISABLE_PRINTING
			case GLOBAL_PRINT:	/* output art/thread/tagged arts to printer */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_PRINT, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;
#endif /* !DISABLE_PRINTING */

			case GROUP_REPOST:	/* repost current article */
				if (can_post) {
					if (grpmenu.curr >= 0)
						feed_articles(FEED_REPOST, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				} else
					info_message(_(txt_cannot_post));
				break;

			case GROUP_SAVE:	/* save articles with prompting */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_SAVE, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;

			case GROUP_AUTOSAVE:	/* Auto-save articles without prompting */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_AUTOSAVE, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;

			case GLOBAL_SET_RANGE:	/* set range */
				if (grpmenu.curr >= 0 && set_range(GROUP_LEVEL, 1, grpmenu.max, grpmenu.curr + 1)) {
					range_active = TRUE;
					show_group_page();
				}
				break;

			case GLOBAL_SEARCH_REPEAT:
				info_message(_(txt_no_prev_search));
				break;

			case GLOBAL_SEARCH_AUTHOR_FORWARD:
			case GLOBAL_SEARCH_AUTHOR_BACKWARD:
			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
				if ((thread_depth = do_search(func, repeat_search)) != 0)
					ret_code = enter_thread(thread_depth, NULL);
				break;

			case GLOBAL_SEARCH_BODY:	/* search article body */
				if (grpmenu.curr >= 0) {
					if ((n = search_body(group, (int) base[grpmenu.curr], repeat_search)) != -1)
						ret_code = enter_pager(n, FALSE);
				} else
					info_message(_(txt_no_arts));
				break;

			case GROUP_READ_BASENOTE:	/* read current basenote */
				if (grpmenu.curr >= 0)
					ret_code = enter_pager((int) base[grpmenu.curr], FALSE /*TRUE*/);
				else
					info_message(_(txt_no_arts));
				break;

			case GROUP_CANCEL:	/* cancel current basenote */
				if (grpmenu.curr >= 0) {
					if (can_post || (group->attribute->mailing_list && *group->attribute->mailing_list)) {
						n = (int) base[grpmenu.curr];
						ii = art_open(TRUE, &arts[n], group, &pgart, TRUE, _(txt_reading_article));
						if (ii != ART_UNAVAILABLE && ii != ART_ABORT && cancel_article(group, &arts[n], n))
							show_group_page();
						art_close(&pgart);
					} else
						info_message(_(txt_cannot_post));
				} else
					info_message(_(txt_no_arts));
				break;

			case GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP:	/* goto next unread article/group */
				ret_code = tab_pressed();
				break;

			case GLOBAL_PAGE_DOWN:
				page_down();
				break;

			case GLOBAL_MENU_FILTER_SELECT:		/* auto-select article menu */
			case GLOBAL_MENU_FILTER_KILL:		/* kill article menu */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}
				n = (int) base[grpmenu.curr];
				if (filter_menu(func, group, &arts[n])) {
					old_artnum = arts[n].artnum;
					unfilter_articles(group);
					filter_articles(group);
					make_threads(group, FALSE);
					grpmenu.curr = find_new_pos(old_artnum, grpmenu.curr);
				}
				show_group_page();
				break;

			case GLOBAL_EDIT_FILTER:
				if (invoke_editor(filter_file, filter_file_offset, NULL)) {
					old_artnum = grpmenu.max > 0 ? arts[(int) base[grpmenu.curr]].artnum : T_ARTNUM_CONST(-1);
					unfilter_articles(group);
					if (read_filter_file(filter_file))
						filter_articles(group);
					make_threads(group, FALSE);
					grpmenu.curr = old_artnum >= T_ARTNUM_CONST(0) ? find_new_pos(old_artnum, grpmenu.curr) : grpmenu.max - 1;
				}
				show_group_page();
				break;

			case GLOBAL_QUICK_FILTER_SELECT:		/* quickly auto-select article */
			case GLOBAL_QUICK_FILTER_KILL:			/* quickly kill article */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}
				if ((!TINRC_CONFIRM_ACTION) || prompt_yn((func == GLOBAL_QUICK_FILTER_KILL) ? _(txt_quick_filter_kill) : _(txt_quick_filter_select), TRUE) == 1) {
					n = (int) base[grpmenu.curr]; /* should this depend on show_only_unread_arts? */
					if (quick_filter(func, group, &arts[n])) {
						old_artnum = arts[n].artnum;
						unfilter_articles(group);
						filter_articles(group);
						make_threads(group, FALSE);
						grpmenu.curr = find_new_pos(old_artnum, grpmenu.curr);
						show_group_page();
						info_message((func == GLOBAL_QUICK_FILTER_KILL) ? _(txt_info_add_kill) : _(txt_info_add_select));
					}
				}
				break;

			case GLOBAL_REDRAW_SCREEN:
				my_retouch();
				set_xclick_off();
				show_group_page();
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

			case GLOBAL_SCROLL_DOWN:
				scroll_down();
				break;

			case GLOBAL_SCROLL_UP:
				scroll_up();
				break;

			case SPECIAL_CATCHUP_LEFT:
			case CATCHUP:
			case CATCHUP_NEXT_UNREAD:
				ret_code = group_catchup(func);
				break;

			case GROUP_TOGGLE_SUBJECT_DISPLAY:	/* toggle display of subject & subj/author */
				if (++curr_group->attribute->show_author > SHOW_FROM_BOTH)
					curr_group->attribute->show_author = SHOW_FROM_NONE;
				show_group_page();
				break;

			case GROUP_GOTO:	/* choose a new group by name */
				n = choose_new_group();
				if (n >= 0 && n != selmenu.curr) {
					selmenu.curr = n;
					ret_code = GRP_ENTER;
				}
				break;

			case GLOBAL_HELP:
				show_help_page(GROUP_LEVEL, _(txt_index_page_com));
				show_group_page();
				break;

			case GLOBAL_CONNECTION_INFO:
				show_connection_page();
				show_group_page();
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:		/* toggle mini help menu */
				toggle_mini_help(GROUP_LEVEL);
				show_group_page();
				break;

			case GLOBAL_TOGGLE_INVERSE_VIDEO:		/* toggle inverse video */
				toggle_inverse_video();
				need_parse_fmt |= GROUP_LEVEL;
				show_group_page();
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:
				if (toggle_color()) {
					show_group_page();
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case GROUP_MARK_THREAD_READ:			/* mark current thread/range/tagged threads as read */
			case MARK_THREAD_UNREAD:				/* or unread */
				if (grpmenu.curr < 0)
					info_message(_(txt_no_arts));
				else {
					t_function function, type;

					function = func == GROUP_MARK_THREAD_READ ? (t_function) FEED_MARK_READ : (t_function) FEED_MARK_UNREAD;
					type = range_active ? FEED_RANGE : (num_of_tagged_arts && !group->attribute->mark_ignore_tags) ? NOT_ASSIGNED : FEED_THREAD;
					feed_articles(function, GROUP_LEVEL, type, group, (int) base[grpmenu.curr]);
				}
				break;

			case GROUP_LIST_THREAD:				/* list articles within current thread */
				ret_code = enter_thread(0, NULL);	/* Enter thread at the top */
				break;

			case GLOBAL_LOOKUP_MESSAGEID:
				{
					t_bool only_unread = group->attribute->show_only_unread_arts;

					if (only_unread)
						toggle_read_unread(FALSE);
					if ((i = prompt_msgid()) != ART_UNAVAILABLE)
						ret_code = enter_pager(i, FALSE);
#ifdef NNTP_ABLE
					else {
						const char *eyde = input_history[HIST_MESSAGE_ID][(hist_pos[HIST_MESSAGE_ID] - 1 + HIST_SIZE) % HIST_SIZE];
						int sc = selmenu.curr;

						if (*eyde) {
							switch (show_article_by_msgid(eyde)) {
								case LOOKUP_OK:
									selmenu.curr = sc;
									if (!index_group(curr_group))
										ret_code = GRP_RETSELECT; /* TODO: return GRP_RETSELECT; ? */
									break;

								case LOOKUP_UNAVAIL: /* check for !reading_saved_news? */
									wait_message(2, _(txt_lookup_func_not_available));
									break;

								default:
									wait_message(2, _(txt_art_unavailable));
									break;
							}
						}
					}
#endif /* NNTP_ABLE */
					if (ret_code == 0) {
						if (only_unread)
							toggle_read_unread(FALSE);
						show_group_page();
					}
				}
				break;

			case GLOBAL_OPTION_MENU:			/* option menu */
				old_artnum = grpmenu.max > 0 ? arts[(int) base[grpmenu.curr]].artnum : T_ARTNUM_CONST(-1);
				config_page(group->name, signal_context);
				grpmenu.curr = old_artnum >= T_ARTNUM_CONST(0) ? find_new_pos(old_artnum, grpmenu.curr) : grpmenu.max - 1;
				show_group_page();
				break;

			case GROUP_NEXT_GROUP:			/* goto next group */
				clear_message();
				if (selmenu.curr + 1 >= selmenu.max)
					info_message(_(txt_no_more_groups));
				else {
					if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(_(txt_confirm_select_on_exit), FALSE) != 1)) {
						undo_auto_select_arts();
						xflag = FALSE;
					}
					selmenu.curr++;
					ret_code = GRP_NEXTUNREAD;
				}
				break;

			case GROUP_NEXT_UNREAD_ARTICLE:	/* goto next unread article */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_next_unread_art));
					break;
				}
				if ((n = next_unread((int) base[grpmenu.curr])) == -1)
					info_message(_(txt_no_next_unread_art));
				else
					ret_code = enter_pager(n, FALSE);
				break;

			case GROUP_PREVIOUS_UNREAD_ARTICLE:	/* go to previous unread article */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_prev_unread_art));
					break;
				}

				if ((n = prev_unread(prev_response((int) base[grpmenu.curr]))) == -1)
					info_message(_(txt_no_prev_unread_art));
				else
					ret_code = enter_pager(n, FALSE);
				break;

			case GROUP_PREVIOUS_GROUP:	/* previous group */
				clear_message();
				for (i = selmenu.curr - 1; i >= 0; i--) {
					if (UNREAD_GROUP(i))
						break;
				}
				if (i < 0)
					info_message(_(txt_no_prev_group));
				else {
					if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(_(txt_confirm_select_on_exit), FALSE) != 1)) {
						undo_auto_select_arts();
						xflag = FALSE;
					}
					selmenu.curr = i;
					ret_code = GRP_NEXTUNREAD;
				}
				break;

			case GLOBAL_QUIT:	/* return to group selection page */
				/* TODO: add %d-num_of_tagged_arts counter in prompt? */
				if (num_of_tagged_arts && prompt_yn(P_(txt_quit_despite_tags_sp[0], txt_quit_despite_tags_sp[1], num_of_tagged_arts), TRUE) != 1)
					break;
				if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(_(txt_confirm_select_on_exit), FALSE) != 1)) {
					undo_auto_select_arts();
					xflag = FALSE;
				}
				ret_code = GRP_EXIT;
				break;

			case GLOBAL_QUIT_TIN:		/* quit */
				/* TODO: add %d-num_of_tagged_arts counter in prompt? */
				if (num_of_tagged_arts && prompt_yn(P_(txt_quit_despite_tags_sp[0], txt_quit_despite_tags_sp[1], num_of_tagged_arts), TRUE) != 1)
					break;
				if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(_(txt_confirm_select_on_exit), FALSE) != 1)) {
					undo_auto_select_arts();
					xflag = FALSE;
				}
				ret_code = GRP_QUIT;
				break;

			case GROUP_TOGGLE_READ_UNREAD:
				toggle_read_unread(FALSE);
				show_group_page();
				break;

			case GROUP_TOGGLE_GET_ARTICLES_LIMIT:
				if (prompt_getart_limit()) {
					/*
					 * if getart limit was given via cmd-line
					 * make it inactive now in order to use
					 * tinrc.getart_limit
					 */
					if (cmdline.args & CMDLINE_GETART_LIMIT)
						cmdline.args ^= CMDLINE_GETART_LIMIT;
					ret_code = GRP_NEXTUNREAD;
				}
				break;

			case GLOBAL_BUGREPORT:
				bug_report();
				break;

			case GROUP_TAG_PARTS: /* tag all in order */
				if (0 <= grpmenu.curr) {
					if (tag_multipart((int) base[grpmenu.curr]) != 0) {
						int old_num = num_of_tagged_arts;

						/*
						 * on success, move the pointer to the next
						 * untagged article just for ease of use's sake
						 */
						n = grpmenu.curr;
						update_group_page();
						do {
							++n;
							n %= grpmenu.max;
							if (arts[base[n]].tagged == 0) {
								move_to_item(n);
								break;
							}
						} while (n != grpmenu.curr);
						if (old_num < num_of_tagged_arts)
							info_message(_(txt_info_all_parts_tagged));
						else
							info_message(_(txt_info_all_parts_untagged));
					}
				}
				break;

			case GROUP_TAG:		/* tag/untag threads for mailing/piping/printing/saving */
				if (grpmenu.curr >= 0) {
					t_bool tagged = TRUE;

					n = (int) base[grpmenu.curr];

					/*
					 * This loop looks for any article in the thread that
					 * isn't already tagged.
					 */
					for (ii = n; ii != -1; ii = arts[ii].thread) {
						if (arts[ii].tagged == 0) {
							tagged = FALSE;
							break;
						}
					}

					/*
					 * If the whole thread is tagged, untag it. Otherwise, tag
					 * any untagged articles
					 */
					if (tagged) {
						/*
						 * Here we repeat the tagged test in both blocks
						 * to leave the choice of tagged/untagged
						 * determination politic in the previous lines.
						 */
						for (ii = n; ii != -1; ii = arts[ii].thread) {
							if (arts[ii].tagged != 0) {
								tagged = TRUE;
								untag_article(ii);
							}
						}
					} else {
						for (ii = n; ii != -1; ii = arts[ii].thread) {
							if (arts[ii].tagged == 0)
								arts[ii].tagged = ++num_of_tagged_arts;
						}
					}
					if ((ii = line_is_tagged(n))) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
						if ((wtmp = char2wchar_t(tin_ltoa(ii, 3)))) {
							mark_screen(grpmenu.curr, mark_offset - (3 - art_mark_width), wtmp);
							free(wtmp);
						}
#else
						mark_screen(grpmenu.curr, mark_offset - 2, tin_ltoa(ii, 3));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
						wchar_t mark[] = { L'\0', L'\0' };
#else
						char mark[] = { '\0', '\0' };
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

						stat_thread(grpmenu.curr, &sbuf);
						mark[0] = sbuf.art_mark;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
						mark_screen(grpmenu.curr, mark_offset - (3 - art_mark_width), (const wchar_t *) L"   "); /* clear space used by tag numbering */
						mark_screen(grpmenu.curr, mark_offset + (art_mark_width - wcwidth(mark[0])), mark);
#else
						mark_screen(grpmenu.curr, mark_offset - 2, "  "); /* clear space used by tag numbering */
						mark_screen(grpmenu.curr, mark_offset, mark);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					}
					if (tagged)
						show_tagged_lines();

					if (grpmenu.curr + 1 < grpmenu.max)
						move_down();
					else
						draw_subject_arrow();

					info_message(tagged ? _(txt_untagged_thread) : _(txt_tagged_thread));
				}
				break;

			case GROUP_TOGGLE_THREADING:		/* Cycle through the threading types */
				group->attribute->thread_articles = CAST_BITS((group->attribute->thread_articles + 1) % (THREAD_MAX + 1), thread_articles);
				if (grpmenu.curr >= 0) {
					i = (int) base[grpmenu.curr];								/* Save a copy of current thread */
					make_threads(group, TRUE);
					find_base(group);
					if ((grpmenu.curr = which_thread(i)) < 0)			/* Restore current position in group */
						grpmenu.curr = 0;
				}
				show_group_page();
				break;

			case GROUP_UNTAG:	/* untag all articles */
				if (grpmenu.curr >= 0) {
					if (untag_all_articles())
						update_group_page();
				}
				break;

			case GLOBAL_VERSION:
				info_message(cvers);
				break;

			case GLOBAL_POST:	/* post an article */
				if (post_article(group->name))
					show_group_page();
				break;

			case GLOBAL_POSTPONED:	/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles(FALSE, FALSE))
						show_group_page();
				} else
					info_message(_(txt_cannot_post));
				break;

			case GLOBAL_DISPLAY_POST_HISTORY:	/* display messages posted by user */
				if (post_hist_page()) {
					if (grpmenu.curr == -1 && grpmenu.max > 0)
						grpmenu.curr = 0;
					show_group_page();
				}
				break;

			case MARK_ARTICLE_UNREAD:		/* mark base article of thread unread */
				if (grpmenu.curr < 0)
					info_message(_(txt_no_arts));
				else {
					if (range_active) {
						/*
						 * We are tied to following base[] here, not arts[], as we operate on
						 * the base articles by definition.
						 */
						for (ii = 0; ii < grpmenu.max; ++ii) {
							if (arts[base[ii]].inrange) {
								arts[base[ii]].inrange = FALSE;
								art_mark(group, &arts[base[ii]], ART_WILL_RETURN);
								for_each_art_in_thread(i, ii)
									arts[i].inrange = FALSE;
							}
						}
						range_active = FALSE;
						show_group_page();
					} else
						art_mark(group, &arts[base[grpmenu.curr]], ART_WILL_RETURN);

					show_group_title(TRUE);
					build_sline(grpmenu.curr);
					draw_subject_arrow();
					info_message(range_active ? _(txt_base_article_range_marked_read) : _(txt_base_article_marked_read));
				}
				break;

			case MARK_FEED_READ:	/* mark selected articles as read */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_MARK_READ, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;

			case MARK_FEED_UNREAD:	/* mark selected articles as unread */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_MARK_UNREAD, GROUP_LEVEL, NOT_ASSIGNED, group, (int) base[grpmenu.curr]);
				break;

			case GROUP_SELECT_THREAD:	/* mark thread as selected */
			case GROUP_TOGGLE_SELECT_THREAD:	/* toggle thread */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}

				flag = TRUE;
				if (func == GROUP_TOGGLE_SELECT_THREAD) {
					stat_thread(grpmenu.curr, &sbuf);
					if (sbuf.selected_unread == sbuf.unread)
						flag = FALSE;
				}
				n = 0;
				for_each_art_in_thread(i, grpmenu.curr) {
					arts[i].selected = CAST_BOOL(flag);
					++n;
				}
				assert(n > 0);
				{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					wchar_t mark[] = { L'\0', L'\0' };
#else
					char mark[] = { '\0', '\0' };
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

					stat_thread(grpmenu.curr, &sbuf);
					mark[0] = sbuf.art_mark;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					mark_screen(grpmenu.curr, mark_offset + (art_mark_width - wcwidth(mark[0])), mark);
#else
					mark_screen(grpmenu.curr, mark_offset, mark);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				}

				show_group_title(TRUE);

				if (grpmenu.curr + 1 < grpmenu.max)
					move_down();
				else
					draw_subject_arrow();

				info_message(flag ? _(txt_thread_marked_as_selected) : _(txt_thread_marked_as_deselected));
				break;

			case GROUP_REVERSE_SELECTIONS:	/* reverse selections */
				for_each_art(i)
					arts[i].selected = bool_not(arts[i].selected);
				update_group_page();
				show_group_title(TRUE);
				break;

			case GROUP_UNDO_SELECTIONS:	/* undo selections */
				undo_selections();
				xflag = FALSE;
				show_group_title(TRUE);
				update_group_page();
				break;

			case GROUP_SELECT_PATTERN:	/* select matching patterns */
				if (grpmenu.curr >= 0) {
					char pat[LEN];
					char *prompt;
					char *spat;
					struct regex_cache cache = REGEX_CACHE_INITIALIZER;

					prompt = fmt_string(_(txt_select_pattern), BlankIfNull(tinrc.default_select_pattern));
					if (!(spat = prompt_string_default(prompt, tinrc.default_select_pattern, _(txt_info_no_previous_expression), HIST_SELECT_PATTERN))) {
						FreeIfNeeded(spat);
						free(prompt);
						break;
					}
					tinrc.default_select_pattern = spat;
					free(prompt);

					if (STRCMPEQ(tinrc.default_select_pattern, "*")) {	/* all */
						if (tinrc.wildcard)
							STRCPY(pat, ".*");
						else
							STRCPY(pat, tinrc.default_select_pattern);
					} else
						snprintf(pat, sizeof(pat), REGEX_FMT, tinrc.default_select_pattern);

					if (tinrc.wildcard && !(compile_regex(pat, &cache, REGEX_CASELESS)))
						break;

					flag = FALSE;
					for (n = 0; n < grpmenu.max; n++) {
						if (!match_regex(arts[base[n]].subject, pat, &cache, TRUE))
							continue;

						for_each_art_in_thread(i, n)
							arts[i].selected = TRUE;

						flag = TRUE;
					}
					if (flag) {
						show_group_title(TRUE);
						update_group_page();
					}
					if (tinrc.wildcard)
						regex_cache_destroy(&cache);
				}
				break;

			case GROUP_SELECT_THREAD_IF_UNREAD_SELECTED:	/* select all unread arts in thread hot if 1 is hot */
				for (n = 0; n < grpmenu.max; n++) {
					stat_thread(n, &sbuf);
					if (!sbuf.selected_unread || sbuf.selected_unread == sbuf.unread)
						continue;

					for_each_art_in_thread(i, n)
						arts[i].selected = TRUE;
				}
				show_group_title(TRUE);
				break;

			case GROUP_MARK_UNSELECTED_ARTICLES_READ:	/* mark read all unselected arts */
				if (!xflag) {
					do_auto_select_arts();
					xflag = TRUE;
				} else {
					undo_auto_select_arts();
					xflag = FALSE;
				}
				break;

			case GROUP_DO_AUTOSELECT:		/* perform auto-selection on group */
				for (n = 0; n < grpmenu.max; n++) {
					for_each_art_in_thread(i, n)
						arts[i].selected = TRUE;
				}
				update_group_page();
				show_group_title(TRUE);
				break;

			case GLOBAL_TOGGLE_INFO_LAST_LINE:
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				clear_message();
				draw_subject_arrow();
				break;

			default:
				info_message(_(txt_bad_command), PrintFuncKey(key, GLOBAL_HELP, group_keys));
				break;
		} /* switch(ch) */
	} /* ret_code >= 0 */

	set_xclick_off();

	clear_note_area();
	grp_del_mail_arts(group);

	art_close(&pgart);				/* Close any open art */

	FreeAndNull(grp_fmt.str);
	FreeAndNull(grp_fmt.date_str);

	curr_group = NULL;

	return ret_code;
}


void
show_group_page(
	void)
{
	int i;

	signal_context = cGroup;
	currmenu = &grpmenu;

	if (!grp_fmt.str || (need_parse_fmt & GROUP_LEVEL)) {
		parse_format_string(curr_group->attribute->group_format ? BlankIfNull(*curr_group->attribute->group_format) : "", &grp_fmt);
		need_parse_fmt &= ~GROUP_LEVEL;
		mark_offset = 0;
	}

	ClearScreen();
	set_first_screen_item();
	show_group_title(FALSE);

	for (i = grpmenu.first; i < grpmenu.first + NOTESLINES && i < grpmenu.max; ++i)
		build_sline(i);

	show_mini_help(GROUP_LEVEL);

	if (grpmenu.max <= 0) {
		info_message(_(txt_no_arts));
		return;
	}

	draw_subject_arrow();
}


static void
update_group_page(
	void)
{
	int i, j;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t mark[] = { L'\0', L'\0' };
	wchar_t *wtmp;
#else
	char mark[] = { '\0', '\0' };
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	struct t_art_stat sbuf;

	for (i = grpmenu.first; i < grpmenu.first + NOTESLINES && i < grpmenu.max; ++i) {
		if ((j = line_is_tagged((int) base[i]))) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if ((wtmp = char2wchar_t(tin_ltoa(j, 3)))) {
				mark_screen(i, mark_offset - (3 - art_mark_width), wtmp);
				free(wtmp);
			}
#else
			mark_screen(i, mark_offset - 2, tin_ltoa(j, 3));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		} else {
			stat_thread(i, &sbuf);
			mark[0] = sbuf.art_mark;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			mark_screen(i, mark_offset - (3 - art_mark_width), (const wchar_t *) L"   ");	/* clear space used by tag numbering */
			mark_screen(i, mark_offset + (art_mark_width - wcwidth(mark[0])), mark);
#else
			mark_screen(i, mark_offset - 2, "  ");	/* clear space used by tag numbering */
			mark_screen(i, mark_offset, mark);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			if (sbuf.art_mark == tinrc.art_marked_selected)
				draw_mark_selected(i);
		}
	}

	if (grpmenu.max <= 0)
		return;

	draw_subject_arrow();
}


static void
draw_subject_arrow(
	void)
{
	draw_arrow_mark(INDEX_TOP + grpmenu.curr - grpmenu.first);

	if (tinrc.info_in_last_line) {
		int n;
		struct t_art_stat statbuf;

		stat_thread(grpmenu.curr, &statbuf);
		if (!statbuf.unread || (n = next_unread((int) base[grpmenu.curr])) == -1)
			n = (int) base[grpmenu.curr];
		info_message("%s", arts[n].subject);
	} else if (grpmenu.curr == grpmenu.max - 1)
		info_message(_(txt_end_of_arts));
}


void
clear_note_area(
	void)
{
	MoveCursor(INDEX_TOP, 0);
	CleartoEOS();
}


/*
 * If in show_only_unread_arts mode or there are unread articles we know this
 * thread will exist after toggle. Otherwise we find the next closest to
 * return to. 'force' can be set to force tin to show all messages
 */
static void
toggle_read_unread(
	t_bool force)
{
	int n, i = -1;

	/*
	 * Clear art->keep_in_base if switching to !show_only_unread_arts
	 */
	if (curr_group->attribute->show_only_unread_arts) {
		for_each_art(n)
			arts[n].keep_in_base = FALSE;
	}

	/* force currently is always false */
	if (force)
		curr_group->attribute->show_only_unread_arts = TRUE;	/* Yes - really, we change it in a bit */

	wait_message(0, (curr_group->attribute->show_only_unread_arts) ? _(txt_reading_all_arts) : _(txt_reading_unread_arts));

	if (grpmenu.curr >= 0) {
		if (curr_group->attribute->show_only_unread_arts || new_responses(grpmenu.curr))
			i = (int) base[grpmenu.curr];
		else if ((n = prev_unread((int) base[grpmenu.curr])) >= 0)
			i = n;
		else if ((n = next_unread((int) base[grpmenu.curr])) >= 0)
			i = n;
	}

	if (!force)
		curr_group->attribute->show_only_unread_arts = bool_not(curr_group->attribute->show_only_unread_arts);

	find_base(curr_group);
	if (i >= 0 && (n = which_thread(i)) >= 0)
		grpmenu.curr = n;
	else if (grpmenu.max > 0)
		grpmenu.curr = grpmenu.max - 1;
	clear_message();
}


/*
 * Find new index position after a kill or unkill. Because kill can work on
 * author it is impossible to know which, if any, articles will be left
 * afterwards. So we make a "best attempt" to find a new index point.
 */
static int
find_new_pos(
	t_artnum old_artnum,
	int cur_pos)
{
	int i, pos;

	if ((i = find_artnum(old_artnum)) >= 0 && (pos = which_thread(i)) >= 0)
		return pos;

	return ((cur_pos < grpmenu.max) ? cur_pos : (grpmenu.max - 1));
}


/*
 * Set grpmenu.curr to the first unread or the last thread depending on
 * the value of pos_first_unread
 */
void
pos_first_unread_thread(
	void)
{
	int i;

	if (curr_group->attribute->pos_first_unread) {
		for (i = 0; i < grpmenu.max; i++) {
			if (new_responses(i))
				break;
		}
		grpmenu.curr = ((i < grpmenu.max) ? i : (grpmenu.max - 1));
	} else
		grpmenu.curr = grpmenu.max - 1;
}


void
mark_screen(
	int screen_row,
	int screen_col,
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	const wchar_t *value)
#else
	const char *value)
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
{
	if (tinrc.draw_arrow) {
		MoveCursor(INDEX2LNUM(screen_row), screen_col);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		my_fputws(value, stdout);
#else
		my_fputs(value, stdout);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		stow_cursor();
		my_flush();
	} else {
#ifdef USE_CURSES
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		char *tmp;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		int y, x;

		getyx(stdscr, y, x);
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((tmp = wchar_t2char(value))) {
			mvaddstr(INDEX2LNUM(screen_row), screen_col, tmp);
			free(tmp);
		}
#	else
		mvaddstr(INDEX2LNUM(screen_row), screen_col, value);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		MoveCursor(y, x);
#else
		int i;

		for (i = 0; value[i] != '\0'; i++)
			screen[INDEX2SNUM(screen_row)].col[screen_col + i] = value[i];

		MoveCursor(INDEX2LNUM(screen_row), screen_col);
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		my_fputws(value, stdout);
#	else
		my_fputs(value, stdout);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#endif /* USE_CURSES */
		currmenu->draw_arrow();
	}
}


/*
 *	Builds the correct header for multipart messages when sorting via
 *	THREAD_MULTI.
 */
static void
build_multipart_header(
	char *dest,
	int maxlen,
	const char *src,
	int cmplen,
	int have,
	int total)
{
	const char *mark = (have == total) ? "*" : "-";
	char *ss;

	if (cmplen > maxlen)
		strncpy(dest, src, (size_t) maxlen);
	else {
		strncpy(dest, src, (size_t) cmplen);
		ss = dest + cmplen;
		snprintf(ss, (size_t) (maxlen - cmplen), "(%s/%d)", mark, total);
	}
}


/*
 * Build subject line given an index into base[].
 *
 * WARNING: some other code expects to find the article mark (ART_MARK_READ,
 * ART_MARK_SELECTED, etc) at mark_offset from beginning of the line.
 * So, if you change the format used in this routine, be sure to check that
 * the value of mark_offset is still correct.
 * Yes, this is somewhat kludgy.
 */
static void
build_sline(
	int i)
{
	char *fmt, *buf;
	char *buffer;
	char arts_sub[HEADER_LEN];
	char tmp_buf[8];
	char tmp[LEN];
	int respnum;
	int j, k, n;
	size_t len;
	struct t_art_stat sbuf;
	t_bool tagged = FALSE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp, *wtmp2;
#else
	int fill, gap;
	size_t len_start;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

#ifdef USE_CURSES
	/*
	 * Allocate line buffer
	 * make it the same size like in !USE_CURSES case to simplify the code
	 */
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		buffer = my_malloc((size_t) cCOLS * MB_CUR_MAX + 2);
#	else
		buffer = my_malloc((size_t) cCOLS + 2);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#else
	buffer = screen[INDEX2SNUM(i)].col;
#endif /* USE_CURSES */

	buffer[0] = '\0';

	respnum = (int) base[i];

	stat_thread(i, &sbuf);

	/*
	 * Find index of first unread in this thread
	 */
	j = (sbuf.unread) ? next_unread(respnum) : respnum;

	fmt = grp_fmt.str;

	if (tinrc.draw_arrow)
		strcat(buffer, "  ");

	for (; *fmt; fmt++) {
		if (*fmt != '%') {
			strncat(buffer, fmt, 1);
			continue;
		}
		switch (*++fmt) {
			case '\0':
				break;

			case '%':
				strncat(buffer, fmt, 1);
				break;

			case 'D':	/* date */
				buf = my_malloc(LEN);
				if (my_strftime(buf, LEN - 1, grp_fmt.date_str, localtime((const time_t *) &arts[j].date))) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					if ((wtmp = char2wchar_t(buf)) != NULL) {
						wtmp2 = wcspart(wtmp, grp_fmt.len_date_max, TRUE);
						free(wtmp);
						if (wcstombs(tmp, wtmp2, sizeof(tmp) - 1) != (size_t) -1)
							strcat(buffer, tmp);

						free(wtmp2);
					}
#else
					strncat(buffer, buf, grp_fmt.len_date_max);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				}
				free(buf);
				break;

			case 'F':	/* from */ /* TODO: BiDi */
				if (curr_group->attribute->show_author != SHOW_FROM_NONE) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					get_author(FALSE, &arts[j], tmp, sizeof(tmp) - 1);

					if ((wtmp = char2wchar_t(tmp)) != NULL) {
						wtmp2 = wcspart(wtmp, grp_fmt.len_from, TRUE);
						free(wtmp);
					} else
						wtmp2 = wcspart(L"", grp_fmt.len_from, TRUE);
					if (wcstombs(tmp, wtmp2, sizeof(tmp) - 1) != (size_t) -1)
						strcat(buffer, tmp);

					free(wtmp2);
#else
					len_start = strwidth(buffer);
					get_author(FALSE, &arts[j], buffer + strlen(buffer), grp_fmt.len_from);
					fill = grp_fmt.len_from - (strwidth(buffer) - len_start);
					gap = strlen(buffer);
					for (k = 0; k < fill; k++)
						buffer[gap + k] = ' ';
					buffer[gap + fill] = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				}
				break;

			case 'I':	/* initials */
				len = MIN(grp_fmt.len_initials, sizeof(tmp) - 1);
				get_initials(&arts[j], tmp, (int) len);
				strcat(buffer, tmp);
				if ((k = (int) (len - (size_t) strwidth(tmp))) > 0) {
					buf = buffer + strlen(buffer);
					for (; k > 0; --k)
						*buf++ = ' ';
					*buf = '\0';
				}
				break;

			case 'L':	/* lines */
				if (arts[j].line_count != -1)
					strcat(buffer, tin_ltoa(arts[j].line_count, grp_fmt.len_linecnt));
				else {
					buf = buffer + strlen(buffer);
					for (k = (int) grp_fmt.len_linecnt; k > 1; --k)
						*buf++ = ' ';
					*buf++ = '?';
					*buf = '\0';
				}
				break;

			case 'm':	/* article flags, tag number, or whatever */
				if (!grp_fmt.mark_offset)
					grp_fmt.mark_offset = (size_t) (mark_offset = strwidth(buffer) + 2);
				if ((k = line_is_tagged(respnum))) {
					STRCPY(tmp_buf, tin_ltoa(k, 3));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					if (art_mark_width > 1)
						strcat(buffer, " ");
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					tagged = TRUE;
				} else
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					snprintf(tmp_buf, sizeof(tmp_buf), "%s%"T_CHAR_FMT, art_mark_width > wcwidth(sbuf.art_mark) ? "   " : "  ", (wint_t) sbuf.art_mark);
#else
					snprintf(tmp_buf, sizeof(tmp_buf), "  %c", sbuf.art_mark);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				strcat(buffer, tmp_buf);
				break;

			case 'M':	/* message-id */
				len = MIN(grp_fmt.len_msgid, sizeof(tmp) - 1);
				strncpy(tmp, arts[j].refptr ? arts[j].refptr->txt : "", len);
				tmp[len] = '\0';
				strcat(buffer, tmp);
				if ((k = (int) (len - (size_t) strwidth(tmp))) > 0) {
					buf = buffer + strlen(buffer);
					for (; k > 0; --k)
						*buf++ = ' ';
					*buf = '\0';
				}
				break;

			case 'n':
				strcat(buffer, tin_ltoa(i + 1, grp_fmt.len_linenumber));
				break;

			case 'R':
				n = ((curr_group->attribute->show_only_unread_arts) ? (sbuf.unread + sbuf.seen) : sbuf.total);
				if (n > 1)
					strcat(buffer, tin_ltoa(n, grp_fmt.len_respcnt));
				else {
					buf = buffer + strlen(buffer);
					for (k = (int) grp_fmt.len_respcnt; k > 0; --k)
						*buf++ = ' ';
					*buf = '\0';
				}
				break;

			case 'S':	/* score */
				strcat(buffer, tin_ltoa(sbuf.score, grp_fmt.len_score));
				break;

			case 's':	/* thread/subject */ /* TODO: BiDi */
				len = curr_group->attribute->show_author != SHOW_FROM_NONE ? grp_fmt.len_subj : grp_fmt.len_subj + grp_fmt.len_from;

				if (sbuf.multipart_have > 1) /* We have a multipart msg so lets built our new header info. */
					build_multipart_header(arts_sub, (int) len, arts[j].subject, sbuf.multipart_compare_len, sbuf.multipart_have, sbuf.multipart_total);
				else
					STRCPY(arts_sub, arts[j].subject);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if ((wtmp = char2wchar_t(arts_sub)) != NULL) {
					wtmp2 = wcspart(wtmp, len, TRUE);
					free(wtmp);
				} else
					wtmp2 = wcspart(L"", len, TRUE);

				if (wcstombs(tmp, wtmp2, sizeof(tmp) - 1) != (size_t) -1)
					strcat(buffer, tmp);

				free(wtmp2);
#else
				len_start = strwidth(buffer);
				strncat(buffer, arts_sub, len);
				fill = len - (strwidth(buffer) - len_start);
				gap = strlen(buffer);
				for (k = 0; k < fill; k++)
					buffer[gap + k] = ' ';
				buffer[gap + fill] = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				break;

			default:
				break;
		}
	}
	/* protect display from non-displayable characters (e.g., form-feed) */
	convert_to_printable(buffer, FALSE);

#ifndef USE_CURSES
	if (tinrc.strip_blanks)
		strcat(strip_line(buffer), cCRLF);
#endif /* !USE_CURSES */

	WriteLine(INDEX2LNUM(i), buffer);

#ifdef USE_CURSES
	free(buffer);
#endif /* USE_CURSES */
	if (!tagged && sbuf.art_mark == tinrc.art_marked_selected)
		draw_mark_selected(i);
}


static void
show_group_title(
	t_bool clear_title)
{
	char buf[LEN], tmp[LEN], keyhelp[MAXKEYLEN], flag;
	char *grpname;
	int i, len, keyhelplen, art_cnt = 0, recent_art_cnt = 0, selected_art_cnt = 0, read_selected_art_cnt = 0, killed_art_cnt = 0;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp, *wtmp2;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	for_each_art(i) {
		if (arts[i].thread == ART_EXPIRED)
			continue;

		if (curr_group->attribute->show_only_unread_arts) {
			if (arts[i].status != ART_READ) {
				++art_cnt;
				if (tinrc.recent_time && ((time((time_t *) 0) - arts[i].date) < (tinrc.recent_time * DAY)))
					++recent_art_cnt;
			}
			if (arts[i].killed == ART_KILLED_UNREAD)
				++killed_art_cnt;
		} else {
			++art_cnt;
			if (tinrc.recent_time && ((time((time_t *) 0) - arts[i].date) < (tinrc.recent_time * DAY)))
				++recent_art_cnt;

			if (arts[i].killed)
				++killed_art_cnt;
		}
		if (arts[i].selected) {
			if (arts[i].status != ART_READ)
				++selected_art_cnt;
			else
				++read_selected_art_cnt;
		}
	}

	/*
	 * build the group title
	 */
	/* thread count */
	snprintf(buf, sizeof(buf), " (%d%c",
		grpmenu.max, *txt_threading[curr_group->attribute->thread_articles]);

	/* article count */
	if ((cmdline.args & CMDLINE_GETART_LIMIT) ? cmdline.getart_limit : tinrc.getart_limit)
		snprintf(tmp, sizeof(tmp), " %d/%d%"T_CHAR_FMT,
			(cmdline.args & CMDLINE_GETART_LIMIT) ? cmdline.getart_limit : tinrc.getart_limit, art_cnt,
			(wint_t) (curr_group->attribute->show_only_unread_arts ? tinrc.art_marked_unread : tinrc.art_marked_read));
	else
		snprintf(tmp, sizeof(tmp), " %d%"T_CHAR_FMT,
			art_cnt,
			(wint_t) (curr_group->attribute->show_only_unread_arts ? tinrc.art_marked_unread : tinrc.art_marked_read));
	if (sizeof(buf) > strlen(buf) + strlen(tmp))
		strcat(buf, tmp);

	/* selected articles */
	if (curr_group->attribute->show_only_unread_arts)
		snprintf(tmp, sizeof(tmp), " %d%"T_CHAR_FMT,
			selected_art_cnt, (wint_t) tinrc.art_marked_selected);
	else
		snprintf(tmp, sizeof(tmp), " %d%"T_CHAR_FMT" %d%"T_CHAR_FMT,
			selected_art_cnt, (wint_t) tinrc.art_marked_selected,
			read_selected_art_cnt, (wint_t) tinrc.art_marked_read_selected);
	if (sizeof(buf) > strlen(buf) + strlen(tmp))
		strcat(buf, tmp);

	/* recent articles */
	if (tinrc.recent_time) {
		snprintf(tmp, sizeof(tmp), " %d%"T_CHAR_FMT,
			recent_art_cnt, (wint_t) tinrc.art_marked_recent);

		if (sizeof(buf) > strlen(buf) + strlen(tmp))
			strcat(buf, tmp);
	}

	/* killed articles */
	snprintf(tmp, sizeof(tmp), " %d%"T_CHAR_FMT,
		killed_art_cnt, (wint_t) tinrc.art_marked_killed);

	if (sizeof(buf) > strlen(buf) + strlen(tmp))
		strcat(buf, tmp);

	/* group flag */
	if ((flag = group_flag(curr_group->moderated)) == ' ')
		snprintf(tmp, sizeof(tmp), ")");
	else
		snprintf(tmp, sizeof(tmp), ") %c", flag);
	if (sizeof(buf) > strlen(buf) + strlen(tmp))
		strcat(buf, tmp);

	/*
	 * determine max len for centered group name
	 */
	if (tinrc.show_help_mail_sign != SHOW_SIGN_NONE) {
		if (tinrc.show_help_mail_sign == SHOW_SIGN_MAIL)
			len = cCOLS - (2 * strwidth(_(txt_you_have_mail))) - 2;
		else {
			PrintFuncKey(keyhelp, GLOBAL_HELP, group_keys);
			keyhelplen = strwidth(keyhelp);
			if (tinrc.show_help_mail_sign == SHOW_SIGN_HELP)
				len = cCOLS - (2 * (strwidth(_(txt_type_h_for_help)) - 2 + keyhelplen)) - 2;
			else
				len = cCOLS - (2 * MAX(strwidth(_(txt_type_h_for_help)) - 2 + keyhelplen, strwidth(_(txt_you_have_mail)))) - 2;
		}
	} else
		len = cCOLS - strwidth(buf) - 2;

	if (len < 0)
		len = 0;
	/* group name */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	if ((wtmp = char2wchar_t(curr_group->name)) != NULL) {
		if (tinrc.abbreviate_groupname)
			wtmp2 = abbr_wcsgroupname(wtmp, len);
		else
			wtmp2 = wstrunc(wtmp, len);
		free(wtmp);
		grpname = wchar_t2char(wtmp2);
		free(wtmp2);
		if (grpname) {
			STRCPY(tmp, grpname);
			strcat(tmp, buf);
			free(grpname);
		} else {
			STRCPY(tmp, buf);
		}
	} else
		STRCPY(tmp, buf);
#else
	if (tinrc.abbreviate_groupname)
		grpname = abbr_groupname(curr_group->name, len);
	else
		grpname = strunc(curr_group->name, len);
	STRCPY(tmp, grpname);
	strcat(tmp, buf);
	free(grpname);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (clear_title) {
		MoveCursor(0, 0);
		CleartoEOLN();
	}
	show_title(tmp);
}


/*
 * Search for type SUBJ/AUTH in direction (TRUE = forwards)
 * Return 0 if all is done, or a >0 thread_depth to enter the thread menu
 */
static int
do_search(
	t_function func,
	t_bool repeat)
{
	int start, n;

	if (grpmenu.curr < 0)
		return 0;

	/*
	 * Not intuitive to search current thread in fwd search
	 */
	start = ((func == GLOBAL_SEARCH_SUBJECT_FORWARD || func == GLOBAL_SEARCH_AUTHOR_FORWARD)
		&& grpmenu.curr < grpmenu.max - 1) ? prev_response((int) base[grpmenu.curr + 1]) : (int) base[grpmenu.curr];

	if (start >= 0 && ((n = search(func, start, repeat)) != -1)) {
		grpmenu.curr = which_thread(n);

		/*
		 * If the search found something deeper in a thread(not the base art)
		 * then enter the thread
		 */
		if ((n = which_response(n)) != 0)
			return n;

		show_group_page();
	}
	return 0;
}


/*
 * We don't directly invoke the pager, but pass through the thread menu
 * to keep navigation sane.
 * 'art' is the arts[art] we wish to read
 * ignore_unavail should be set if we wish to 'keep going' after 'article unavailable'
 * Return a -ve ret_code if we must exit the group menu on return
 */
static int
enter_pager(
	int art,
	t_bool ignore_unavail)
{
	t_pagerinfo page;

	page.art = art;
	page.ignore_unavail = CAST_BOOL(ignore_unavail);

	return enter_thread(0, &page);
}


/*
 * Handle entry/exit with the thread menu
 * Return -ve ret_code if we must exit the group menu on return
 */
static int
enter_thread(
	int depth,
	t_pagerinfo *page)
{
	int i, n;

	if (grpmenu.curr < 0) {
		info_message(_(txt_no_arts));
		return 0;
	}

	forever {
		switch (i = thread_page(curr_group, (int) base[grpmenu.curr], depth, page)) {
			case GRP_QUIT:						/* 'Q'uit */
			case GRP_RETSELECT:					/* Back to selection screen */
				return i;
				/* NOTREACHED */
				break;

			case GRP_NEXT:						/* 'c'atchup */
				show_group_page();
				move_down();
				return 0;
				/* NOTREACHED */
				break;

			case GRP_NEXTUNREAD:				/* 'C'atchup */
				if ((n = next_unread((int) base[grpmenu.curr])) >= 0) {
					if (page)
						page->art = n;
					if ((n = which_thread(n)) >= 0) {
						grpmenu.curr = n;
						depth = 0;
						break;		/* Drop into next thread with unread */
					}
				}
				/* No more unread threads in this group, enter next group */
				grpmenu.curr = 0;
				return GRP_NEXTUNREAD;
				/* NOTREACHED */
				break;

			case GRP_KILLED:
				grpmenu.curr = 0;
				/* FALLTHROUGH */

			case GRP_EXIT:
			/* case GRP_GOTOTHREAD will never make it up this far */
			default:		/* ie >= 0 Shouldn't happen any more? */
				clear_note_area();
				show_group_page();
				return 0;
				/* NOTREACHED */
				break;
		}
	}
	/* NOTREACHED */
	return 0;
}


/*
 * Return a ret_code
 */
static int
tab_pressed(
	void)
{
	int n;

	if ((n = ((grpmenu.curr < 0) ? -1 : next_unread((int) base[grpmenu.curr]))) < 0)
		return GRP_NEXTUNREAD;			/* => Enter next unread group */

	/* We still have unread arts in the current group ... */
	return enter_pager(n, TRUE);
}


/*
 * There are three ways this is called
 * catchup & return to group menu
 * catchup & go to next group with unread articles
 * group exit via left arrow if auto-catchup is set
 * Return a -ve ret_code if we're done with the group menu
 */
static int
group_catchup(
	t_function func)
{
	char buf[LEN];
	int pyn = 1;

	/* TODO: add %d-num_of_tagged_arts counter in prompt? */
	if (num_of_tagged_arts && prompt_yn(P_(txt_catchup_despite_tags_sp[0], txt_catchup_despite_tags_sp[1], num_of_tagged_arts), TRUE) != 1)
		return 0;

	snprintf(buf, sizeof(buf), _(txt_mark_arts_read), (func == CATCHUP_NEXT_UNREAD) ? _(txt_enter_next_unread_group) : "");

	if (!curr_group->newsrc.num_unread || (!TINRC_CONFIRM_ACTION) || (pyn = prompt_yn(buf, TRUE)) == 1)
		grp_mark_read(curr_group, arts);

	switch (func) {
		case CATCHUP:				/* 'c' */
			if (pyn == 1)
				return GRP_NEXT;
			break;

		case CATCHUP_NEXT_UNREAD:			/* 'C' */
			if (pyn == 1)
				return GRP_NEXTUNREAD;
			break;

		case SPECIAL_CATCHUP_LEFT:				/* <- group catchup on exit */
			switch (pyn) {
				case -1:					/* ESCAPE - do nothing */
					break;

				case 1:						/* We caught up - advance group */
					return GRP_NEXT;

				default:					/* Just leave the group */
					return GRP_EXIT;
			}
			/* FALLTHROUGH */
		default:							/* Should not be here */
			break;
	}
	return 0;								/* Stay in this menu by default */
}


static t_bool
prompt_getart_limit(
	void)
{
	const char *p;
	t_bool ret = FALSE;

	clear_message();
	if ((p = tin_getline(_(txt_enter_getart_limit), 2, NULL, 0, FALSE, HIST_OTHER)) != NULL) {
		tinrc.getart_limit = s2i(p, INT_MIN, INT_MAX);
		if (!errno)
			ret = TRUE;
	}
	clear_message();
	return ret;
}


/*
 * Redraw all necessary parts of the screen after FEED_MARK_(UN)READ
 * Move cursor to next unread item if needed
 *
 * Returns TRUE when no next unread art, FALSE otherwise
 */
t_bool
group_mark_postprocess(
	int function,
	t_function feed_type,
	int respnum)
{
	int n;

	show_group_title(TRUE);
	switch (function) {
		case (FEED_MARK_READ):
			if (feed_type == FEED_THREAD || feed_type == FEED_ARTICLE)
				build_sline(grpmenu.curr);
			else
				show_group_page();

			if ((n = next_unread(next_response(respnum))) == -1) {
				draw_subject_arrow();
				return TRUE;
			}

			move_to_item(which_thread(n));
			break;

		case (FEED_MARK_UNREAD):
			if (feed_type == FEED_THREAD || feed_type == FEED_ARTICLE)
				build_sline(grpmenu.curr);
			else
				show_group_page();

			draw_subject_arrow();
			break;

		default:
			break;
	}
	return FALSE;
}
