/*
 *  Project   : tin - a Usenet reader
 *  Module    : select.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2022-10-13
 *  Notes     :
 *
 * Copyright (c) 1991-2022 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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


static struct t_fmt sel_fmt;

/*
 * Local prototypes
 */
static t_function select_left(void);
static t_function select_right(void);
static int active_comp(t_comptype p1, t_comptype p2);
static int reposition_group(struct t_group *group, int default_num);
static int save_restore_curr_group(t_bool saving);
static t_bool pos_next_unread_group(t_bool redraw);
static t_bool yanked_out = TRUE;
static void build_gline(int i);
static void catchup_group(struct t_group *group, t_bool goto_next_unread_group);
static void draw_group_arrow(void);
static void read_groups(void);
static void select_done(void);
_Noreturn static void select_quit(void);
static void select_read_group(void);
static void sort_active_file(void);
static void subscribe_pattern(const char *prompt, const char *message, const char *result, t_bool state);
static void sync_active_file(void);
static void yank_active_file(void);
#ifdef NNTP_ABLE
	static char *lookup_msgid(char *msgid);
	static struct t_group *get_group_from_list(char *newsgroups);
#endif /* NNTP_ABLE */


/*
 * selmenu.curr = index (start at 0) of cursor position on menu,
 *                or -1 when no groups visible on screen
 * selmenu.max = Total # of groups in my_group[]
 * selmenu.first is static here
 */
t_menu selmenu = { 1, 0, 0, show_selection_page, draw_group_arrow, build_gline };

static int groupname_len;	/* max. group name length */
static int flags_offset;
static int ucnt_offset;


static t_function
select_left(
	void)
{
	return GLOBAL_QUIT;
}


static t_function
select_right(
	void)
{
	return SELECT_ENTER_GROUP;
}


void
selection_page(
	int start_groupnum,
	int num_cmd_line_groups)
{
	char buf[LEN];
	char key[MAXKEYLEN];
	int i, n;
	t_function func;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	selmenu.curr = start_groupnum;

#ifdef READ_CHAR_HACK
	setbuf(stdin, 0);
#endif /* READ_CHAR_HACK */

	Raw(TRUE);
	ClearScreen();

	/*
	 * If user specified only 1 cmd line groupname (eg. tin -r alt.sources)
	 * then go there immediately.
	 */
	if (num_cmd_line_groups == 1)
		select_read_group();

	cursoroff();
	show_selection_page();	/* display group selection page */

	forever {
		if (!resync_active_file()) {
			if (reread_active_after_posting()) /* reread active file if necessary */
				show_selection_page();
		} else {
			if (!yanked_out)
				yanked_out = TRUE; /* yank out if yanked in */
		}

		set_xclick_on();

		switch ((func = handle_keypad(select_left, select_right, global_mouse_action, select_keys))) {
			case GLOBAL_ABORT:		/* Abort */
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
				if (selmenu.max)
					prompt_item_num(func_to_key(func, select_keys), _(txt_select_group));
				else
					info_message(_(txt_no_groups));
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_FIRST_PAGE:		/* show first page of groups */
				top_of_list();
				break;

			case GLOBAL_LAST_PAGE:		/* show last page of groups */
				end_of_list();
				break;

			case GLOBAL_PAGE_UP:
				page_up();
				break;

			case GLOBAL_PAGE_DOWN:
				page_down();
				break;

			case GLOBAL_LINE_UP:
				move_up();
				break;

			case GLOBAL_LINE_DOWN:
				move_down();
				break;

			case GLOBAL_SCROLL_DOWN:
				scroll_down();
				break;

			case GLOBAL_SCROLL_UP:
				scroll_up();
				break;

			case SELECT_SORT_ACTIVE:	/* sort active groups */
				sort_active_file();
				break;

			case GLOBAL_SET_RANGE:
				if (selmenu.max) {
					if (set_range(SELECT_LEVEL, 1, selmenu.max, selmenu.curr + 1))
						show_selection_page();
				} else
					info_message(_(txt_no_groups));
				break;

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					info_message(_(txt_no_prev_search));
				else {
					if ((i = search_active((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT))) != -1) {
						move_to_item(i);
						clear_message();
					}
				}
				break;

			case SELECT_ENTER_GROUP:		/* go into group */
				select_read_group();
				break;

			case SELECT_ENTER_NEXT_UNREAD_GROUP:	/* enter next group containing unread articles */
				if (pos_next_unread_group(FALSE))
					read_groups();
				break;							/* Nothing more to do at the moment */

			case GLOBAL_REDRAW_SCREEN:
				my_retouch();					/* TODO: not done elsewhere, maybe should be in show_selection_page */
				set_xclick_off();
				show_selection_page();
				break;

			case SELECT_RESET_NEWSRC:		/* reset .newsrc */
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				if (prompt_yn(_(txt_reset_newsrc), FALSE) == 1) {
					reset_newsrc();
					sync_active_file();
					selmenu.curr = 0;
					show_selection_page();
				}
				break;

			case CATCHUP:			/* catchup - mark all articles as read */
			case CATCHUP_NEXT_UNREAD:	/* and goto next unread group */
				if (selmenu.max)
					catchup_group(&CURR_GROUP, (func == CATCHUP_NEXT_UNREAD));
				else
					info_message(_(txt_no_groups));
				break;

			case GLOBAL_EDIT_FILTER:
				if (invoke_editor(filter_file, filter_file_offset, NULL))
					(void) read_filter_file(filter_file);
				show_selection_page();
				break;

			case SELECT_TOGGLE_DESCRIPTIONS:	/* toggle newsgroup descriptions */
				if (sel_fmt.show_grpdesc) {
					show_description = bool_not(show_description);
					if (show_description)
						read_descriptions(TRUE);
					show_selection_page();
				} else
					info_message(_(txt_grpdesc_disabled));
				break;

			case SELECT_GOTO:			/* prompt for a new group name */
				{
					int oldmax = selmenu.max;

					if ((n = choose_new_group()) >= 0) {
						/*
						 * If a new group was added and it is on the actual screen
						 * draw it. If it is off screen the redraw will handle it.
						 */
						if (oldmax != selmenu.max && n >= selmenu.first && n < selmenu.first + NOTESLINES)
							build_gline(n);
						move_to_item(n);
					}
				}
				break;

			case GLOBAL_HELP:
				show_help_page(SELECT_LEVEL, _(txt_group_select_com));
				show_selection_page();
				break;

#ifdef NNTP_ABLE
			case GLOBAL_LOOKUP_MESSAGEID:
				switch (show_article_by_msgid(NULL)) {
					case LOOKUP_OK:
						show_selection_page();
						break;

					case LOOKUP_UNAVAIL:
						info_message("%s %s", _(txt_lookup_func_not_available), _(txt_lookup_func_not_nntp));
						break;

					case LOOKUP_QUIT:
						select_quit();
						break;

					default:
						break;
				}
				break;
#endif /* NNTP_ABLE */

			case GLOBAL_TOGGLE_HELP_DISPLAY:	/* toggle mini help menu */
				toggle_mini_help(SELECT_LEVEL);
				show_selection_page();
				break;

			case GLOBAL_TOGGLE_INVERSE_VIDEO:
				toggle_inverse_video();
				show_selection_page();
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case GLOBAL_TOGGLE_COLOR:
				if (toggle_color()) {
					show_selection_page();
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case GLOBAL_TOGGLE_INFO_LAST_LINE:	/* display group description */
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				show_selection_page();
				break;

			case SELECT_MOVE_GROUP:			/* reposition group within group list */
				/* TODO: move all this to reposition_group() */
				if (!selmenu.max) {
					info_message(_(txt_no_groups));
					break;
				}

				if (!CURR_GROUP.subscribed) {
					info_message(_(txt_info_not_subscribed));
					break;
				}

				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}

				n = selmenu.curr;
				selmenu.curr = reposition_group(&active[my_group[n]], n);
				HpGlitch(erase_arrow());
				if (selmenu.curr < selmenu.first || selmenu.curr >= selmenu.first + NOTESLINES - 1 || selmenu.curr != n)
					show_selection_page();
				else {
					i = selmenu.curr;
					selmenu.curr = n;
					erase_arrow();
					selmenu.curr = i;
					clear_message();
					draw_group_arrow();
				}
				break;

			case GLOBAL_OPTION_MENU:
				config_page(selmenu.max ? CURR_GROUP.name : NULL, signal_context);
				show_selection_page();
				break;

			case SELECT_NEXT_UNREAD_GROUP:		/* goto next unread group */
				pos_next_unread_group(TRUE);
				break;

			case GLOBAL_QUIT:			/* quit */
				select_done();
				break;

			case GLOBAL_QUIT_TIN:			/* quit, no ask */
				select_quit();
				break;

			case SELECT_QUIT_NO_WRITE:		/* quit, but don't save configuration */
				if (prompt_yn(_(txt_quit_no_write), TRUE) == 1)
					tin_done(EXIT_SUCCESS, NULL);
				show_selection_page();
				break;

			case SELECT_TOGGLE_READ_DISPLAY:
				/*
				 * If in tinrc.show_only_unread_groups mode toggle all
				 * subscribed to groups and only groups that contain unread
				 * articles
				 */
				tinrc.show_only_unread_groups = bool_not(tinrc.show_only_unread_groups);
				/*
				 * as we effectively do a yank out on each change, set yanked_out accordingly
				 */
				yanked_out = TRUE;
				wait_message(0, _(txt_reading_groups), (tinrc.show_only_unread_groups) ? _("unread") : _("all"));

				toggle_my_groups(NULL);
				show_selection_page();
				if (tinrc.show_only_unread_groups)
					info_message(_(txt_show_unread));
				else
					clear_message();
				break;

			case GLOBAL_BUGREPORT:
				bug_report();
				break;

			case SELECT_SUBSCRIBE:			/* subscribe to current group */
				if (!selmenu.max) {
					info_message(_(txt_no_groups));
					break;
				}
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				if (!CURR_GROUP.subscribed && !CURR_GROUP.bogus) {
					subscribe(&CURR_GROUP, SUBSCRIBED, TRUE);
					show_selection_page();
					info_message(_(txt_subscribed_to), CURR_GROUP.name);
					move_down();
				}
				break;

			case SELECT_SUBSCRIBE_PATTERN:		/* subscribe to groups matching pattern */
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				subscribe_pattern(_(txt_subscribe_pattern), _(txt_subscribing), _(txt_subscribed_num_groups), TRUE);
				break;

			case SELECT_UNSUBSCRIBE:		/* unsubscribe to current group */
				if (!selmenu.max) {
					info_message(_(txt_no_groups));
					break;
				}
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				if (CURR_GROUP.subscribed) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					mark_screen(selmenu.curr, flags_offset, CURR_GROUP.newgroup ? L"N" : L"u");
#else
					mark_screen(selmenu.curr, flags_offset, CURR_GROUP.newgroup ? "N" : "u");
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					subscribe(&CURR_GROUP, UNSUBSCRIBED, TRUE);
					info_message(_(txt_unsubscribed_to), CURR_GROUP.name);
					move_down();
				} else if (CURR_GROUP.bogus && tinrc.strip_bogus == BOGUS_SHOW) {
					/* Bogus groups aren't subscribed to avoid confusion */
					/* Note that there is no way to remove the group from active[] */
					snprintf(buf, sizeof(buf), _(txt_remove_bogus), CURR_GROUP.name);
					write_newsrc();					/* save current newsrc */
					delete_group(CURR_GROUP.name);		/* remove bogus group */
					read_newsrc(newsrc, TRUE);			/* reload newsrc */
					toggle_my_groups(NULL);			/* keep current display-state */
					show_selection_page();				/* redraw screen */
					info_message(buf);
				}
				break;

			case SELECT_UNSUBSCRIBE_PATTERN:	/* unsubscribe to groups matching pattern */
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				subscribe_pattern(_(txt_unsubscribe_pattern),
								_(txt_unsubscribing), _(txt_unsubscribed_num_groups), FALSE);
				break;

			case GLOBAL_VERSION:			/* show tin version */
				info_message(cvers);
				break;

			case GLOBAL_POST:			/* post a basenote */
				if (!selmenu.max) {
					if (!can_post) {
						info_message(_(txt_cannot_post));
						break;
					}
					snprintf(buf, sizeof(buf), _(txt_post_newsgroups), tinrc.default_post_newsgroups);
					if (!prompt_string_default(buf, tinrc.default_post_newsgroups, _(txt_no_newsgroups), HIST_POST_NEWSGROUPS))
						break;
					if (group_find(tinrc.default_post_newsgroups, FALSE) == NULL) {
						error_message(2, _(txt_not_in_active_file), tinrc.default_post_newsgroups);
						break;
					} else {
						strcpy(buf, tinrc.default_post_newsgroups);
#if 1 /* TODO: fix the rest of the code so we don't need this anymore */
						/*
						 * this is a gross hack to avoid a crash in the
						 * CHARSET_CONVERSION conversion case in new_part()
						 * which currently relies on CURR_GROUP
						 */
						selmenu.curr = my_group_add(buf, FALSE);
						/*
						 * and the next hack to avoid a grabbled selection
						 * screen after the posting
						 */
						toggle_my_groups(NULL);
						toggle_my_groups(NULL);
#endif /* 1 */
					}
				} else
					STRCPY(buf, CURR_GROUP.name);
				if (!can_post && !CURR_GROUP.bogus && !CURR_GROUP.attribute->mailing_list) {
					info_message(_(txt_cannot_post));
					break;
				}
				if (post_article(buf))
					show_selection_page();
				break;

			case GLOBAL_POSTPONED:			/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles(FALSE, FALSE))
						show_selection_page();
				} else
					info_message(_(txt_cannot_post));
				break;

			case GLOBAL_DISPLAY_POST_HISTORY:	/* display messages posted by user */
				if (post_hist_page())
					show_selection_page();
				break;

			case SELECT_YANK_ACTIVE:		/* yank in/out rest of groups from active */
				yank_active_file();
				break;

			case SELECT_SYNC_WITH_ACTIVE:		/* Re-read active file to see if any new news */
				sync_active_file();
				if (!yanked_out)
					yank_active_file();			/* yank out if yanked in */
				break;

			case SELECT_MARK_GROUP_UNREAD:
				if (!selmenu.max) {
					info_message(_(txt_no_groups));
					break;
				}
				grp_mark_unread(&CURR_GROUP);
				if (CURR_GROUP.newsrc.num_unread)
					STRCPY(buf, tin_ltoa(CURR_GROUP.newsrc.num_unread, (int) sel_fmt.len_ucnt));
				else {
					size_t j = 0;

					while (j < sel_fmt.len_ucnt)
						buf[j++] = ' ';
					buf[j] = '\0';
				}
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if ((wtmp = char2wchar_t(buf))) {
					mark_screen(selmenu.curr, ucnt_offset, wtmp);
					free(wtmp);
				}
#else
				mark_screen(selmenu.curr, ucnt_offset, buf);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				break;

			default:
				info_message(_(txt_bad_command), PrintFuncKey(key, GLOBAL_HELP, select_keys));
		}
	}
}


void
show_selection_page(
	void)
{
	char buf[LEN], keyhelp[MAXKEYLEN];
	char *title;
	int i, keyhelplen;
	size_t len;
	const char *secflag = "";

	signal_context = cSelect;
	currmenu = &selmenu;
	parse_format_string(tinrc.select_format, &sel_fmt);
	groupname_len = 0;
	flags_offset = 0;
	mark_offset = 0;
	ucnt_offset = 0;

	if (use_nntps) {
		if (insecure_nntps)
			secflag=_("[k]");
		else
			secflag=_("[T]");
	}

	if (read_news_via_nntp)
		snprintf(buf, sizeof(buf), "%s (%s%s  %d%s)", _(txt_group_selection), nntp_server, secflag, selmenu.max, (tinrc.show_only_unread_groups ? _(" R") : ""));
	else
		snprintf(buf, sizeof(buf), "%s (%d%s)", _(txt_group_selection), selmenu.max, (tinrc.show_only_unread_groups ? _(" R") : ""));

	if (selmenu.curr < 0)
		selmenu.curr = 0;

	ClearScreen();
	set_first_screen_item();

	/*
	 * determine max. length for centered title
	 */
	if (tinrc.show_help_mail_sign != SHOW_SIGN_NONE) {
		if (tinrc.show_help_mail_sign == SHOW_SIGN_MAIL)
			len = cCOLS - (2 * strwidth(_(txt_you_have_mail))) - 2;
		else {
			PrintFuncKey(keyhelp, GLOBAL_HELP, select_keys);
			keyhelplen = strwidth(keyhelp);
			if (tinrc.show_help_mail_sign == SHOW_SIGN_HELP)
				len = cCOLS - (2 * (strwidth(_(txt_type_h_for_help)) - 2 + keyhelplen)) - 2;
			else
				len = cCOLS - (2 * MAX(strwidth(_(txt_type_h_for_help)) - 2 + keyhelplen, strwidth(_(txt_you_have_mail)))) - 2;
		}
	} else
		len = cCOLS - 2;

	title = strunc(buf, len);
	show_title(title);
	free(title);

	if (sel_fmt.len_grpname_max && !sel_fmt.len_grpname) {
		/*
		 * calculate max length of groupname field
		 * if yanked in (yanked_out == FALSE) check all groups in active file
		 * otherwise just subscribed to groups
		 */
		if (yanked_out) {
			for (i = 0; i < selmenu.max; i++) {
				if ((len = (size_t) strwidth(active[my_group[i]].name)) > sel_fmt.len_grpname)
					sel_fmt.len_grpname = len;
			}
		} else {
			for_each_group(i) {
				if ((len = (size_t) strwidth(active[i].name)) > sel_fmt.len_grpname)
					sel_fmt.len_grpname = len;
			}
		}
	}

	groupname_len = (sel_fmt.show_grpdesc && show_description) ? (int) sel_fmt.len_grpname_dsc : (int) sel_fmt.len_grpname;

	if (groupname_len > (int) sel_fmt.len_grpname_max)
		groupname_len = (int) sel_fmt.len_grpname_max;
	if (groupname_len < 0)
		groupname_len = 0;

	if (!sel_fmt.len_grpdesc)
		sel_fmt.len_grpdesc = (sel_fmt.len_grpname_max - (size_t) groupname_len);
	else {
		if (sel_fmt.len_grpdesc > (sel_fmt.len_grpname_max - (size_t) groupname_len))
			sel_fmt.len_grpdesc = (sel_fmt.len_grpname_max - (size_t) groupname_len);
	}

	flags_offset = (int) (sel_fmt.flags_offset + (size_t) (sel_fmt.g_before_f ? groupname_len : 0) + (sel_fmt.d_before_f ? sel_fmt.len_grpdesc : 0));
	ucnt_offset = (int) (sel_fmt.ucnt_offset + (size_t) (sel_fmt.g_before_u ? groupname_len : 0) + (sel_fmt.d_before_u ? sel_fmt.len_grpdesc : 0));

	for (i = selmenu.first; i < selmenu.first + NOTESLINES && i < selmenu.max; i++)
		build_gline(i);

	show_mini_help(SELECT_LEVEL);

#ifdef NNTP_ABLE
	did_reconnect = FALSE;
#endif /* NNTP_ABLE */

	if (selmenu.max <= 0) {
		info_message(_(txt_no_groups));
		return;
	}

	draw_group_arrow();
}


static void
build_gline(
	int i)
{
	char *sptr, *fmt, *buf;
	char subs;
	int n;
	size_t j;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	char *name_buf = NULL;
	char *desc_buf = NULL;
	wchar_t *active_name, *active_name2, *active_desc, *active_desc2;
#else
	char *active_name, *active_name2;
	size_t fill, len_start;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

#ifdef USE_CURSES
	/*
	 * Allocate line buffer
	 * make it the same size like in !USE_CURSES case to simplify the code
	 */
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		sptr = my_malloc(cCOLS * MB_CUR_MAX + 2);
#	else
		sptr = my_malloc(cCOLS + 2);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#else
	sptr = screen[INDEX2SNUM(i)].col;
#endif /* USE_CURSES */

	sptr[0] = '\0';
	fmt = sel_fmt.str;
	n = my_group[i];

	if (tinrc.draw_arrow)
		strcat(sptr, "  ");

	for (; *fmt; fmt++) {
		if (*fmt != '%') {
			strncat(sptr, fmt, 1);
			continue;
		}
		switch (*++fmt) {
			case '\0':
				break;

			case '%':
				strncat(sptr, fmt, 1);
				break;

			case 'd':
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if (show_description && active[n].description) {
					active_desc = char2wchar_t(active[n].description);
					if (active_desc) {
						if ((active_desc2 = wcspart(active_desc, (int) sel_fmt.len_grpdesc, TRUE)) != NULL) {
							desc_buf = wchar_t2char(active_desc2);
							free(active_desc);
							free(active_desc2);
						}
					}
					if (desc_buf) {
						strcat(sptr, desc_buf);
						FreeAndNull(desc_buf);
					}
				} else {
					buf = sptr + strlen(sptr);
					for (j = 0; j < sel_fmt.len_grpdesc; ++j)
						*buf++ = ' ';
					*buf = '\0';
				}
#else
				if (show_description && active[n].description) {
					len_start = strwidth(sptr);
					strncat(sptr, active[n].description, sel_fmt.len_grpdesc);
					fill = sel_fmt.len_grpdesc - (strwidth(sptr) - len_start);
				} else
					fill = sel_fmt.len_grpdesc;
				buf = sptr + strlen(sptr);
				for (j = 0; j < fill; ++j)
					*buf++ = ' ';
				*buf = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				break;

			case 'f':
				/*
				 * Display a flag for this group if needed
				 * . Bogus groups are dumped immediately 'D'
				 * . Normal subscribed groups may be
				 *   ' ' normal, 'X' not postable, 'M' moderated, '=' renamed
				 * . Newgroups are 'N'
				 * . Unsubscribed groups are 'u'
				 *
				 * TODO: make flags configurable via tinrc?
				 */
				if (active[n].bogus)					/* Group is not in active list */
					subs = 'D';
				else if (active[n].subscribed)			/* Important that this precedes Newgroup */
					subs = group_flag(active[n].moderated);
				else
					subs = ((active[n].newgroup) ? 'N' : 'u'); /* New (but unsubscribed) group or unsubscribed group */
				buf = sptr + strlen(sptr);
				*buf++ = subs;
				*buf = '\0';
				break;

			case 'G':
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if ((active_name = char2wchar_t(active[n].name)) == NULL) /* If char2wchar_t() fails try again after replacing unprintable characters */
					active_name = char2wchar_t(convert_to_printable(active[n].name, FALSE));

				if (active_name && tinrc.abbreviate_groupname) {
					active_name2 = abbr_wcsgroupname(active_name, groupname_len);
					free(active_name);
				} else
					active_name2 = active_name;

				if (active_name2 && (active_name = wcspart(active_name2, groupname_len, TRUE)) != NULL) {
					free(active_name2);
					name_buf = wchar_t2char(active_name);
					free(active_name);
				}
				if (name_buf) {
					strcat(sptr, name_buf);
					FreeAndNull(name_buf);
				}
#else
				if (tinrc.abbreviate_groupname)
					active_name = abbr_groupname(active[n].name, groupname_len);
				else
					active_name = my_strdup(active[n].name);

				active_name2 = my_malloc(groupname_len + 1);
				snprintf(active_name2, groupname_len + 1, "%-*s", groupname_len, active_name);
				strcat(sptr, active_name2);
				free(active_name);
				free(active_name2);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				break;

			case 'n':
				strcat(sptr, tin_ltoa(i + 1, (int) sel_fmt.len_linenumber));
				break;

			case 'U':
				if (active[my_group[i]].inrange) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					char tmp_buf[10];

					buf = sptr + strlen(sptr);
					for (j = 1; j <= sel_fmt.len_ucnt - (art_mark_width - (art_mark_width - wcwidth(tinrc.art_marked_inrange))); ++j)
						*buf++ = ' ';
					snprintf(tmp_buf, sizeof(tmp_buf), "%"T_CHAR_FMT, tinrc.art_marked_inrange);
					*buf-- = '\0';
					strcat(buf, tmp_buf);
#else
					buf = sptr + strlen(sptr);
					for (j = 1; j < sel_fmt.len_ucnt; ++j)
						*buf++ = ' ';
					*buf++ = tinrc.art_marked_inrange;
					*buf = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				} else if (active[my_group[i]].newsrc.num_unread) {
					int getart_limit;
					t_artnum num_unread;

					getart_limit = (cmdline.args & CMDLINE_GETART_LIMIT) ? cmdline.getart_limit : tinrc.getart_limit;
					num_unread = active[my_group[i]].newsrc.num_unread;
					if (getart_limit > 0 && getart_limit < num_unread)
						num_unread = getart_limit;
					strcat(sptr, tin_ltoa(num_unread, (int) sel_fmt.len_ucnt));
				} else {
					buf = sptr + strlen(sptr);
					for (j = 0; j < sel_fmt.len_ucnt; ++j)
						*buf++ = ' ';
					*buf = '\0';
				}
				break;

			default:
				break;
		}
	}
#ifndef USE_CURSES
	if (tinrc.strip_blanks)
		strcat(strip_line(sptr), cCRLF);
#endif /* !USE_CURSES */

	WriteLine(INDEX2LNUM(i), sptr);

#ifdef USE_CURSES
	free(sptr);
#endif /* USE_CURSES */
}


static void
draw_group_arrow(
	void)
{
	if (!selmenu.max)
		info_message(_(txt_no_groups));
	else {
		draw_arrow_mark(INDEX_TOP + selmenu.curr - selmenu.first);
		if (CURR_GROUP.aliasedto)
			info_message(_(txt_group_aliased), CURR_GROUP.aliasedto);
		else if (tinrc.info_in_last_line)
			info_message("%s", CURR_GROUP.description ? CURR_GROUP.description : _(txt_no_description));
		else if (selmenu.curr == selmenu.max - 1)
			info_message(_(txt_end_of_groups));
	}
}


static void
sync_active_file(
	void)
{
	wait_message(0, _(txt_reading_news_newsrc_file));
	force_reread_active_file = TRUE;
	resync_active_file();
}


static void
yank_active_file(
	void)
{
	if (yanked_out && selmenu.max == num_active) {			/* All groups currently present? */
		info_message(_(txt_yanked_none));
		return;
	}

	if (yanked_out) {						/* Yank in */
		int i;
		int prevmax = selmenu.max;

		save_restore_curr_group(TRUE);				/* Save group position */

		/*
		 * Reset counter and load all the groups in active[] into my_group[]
		 */
		selmenu.max = 0;
		for_each_group(i)
			my_group[selmenu.max++] = i;

		selmenu.curr = save_restore_curr_group(FALSE);	/* Restore previous group position */
		yanked_out = bool_not(yanked_out);
		show_selection_page();
		info_message(_(txt_yanked_groups), selmenu.max - prevmax, PLURAL(selmenu.max - prevmax, txt_group));
	} else {							/* Yank out */
		toggle_my_groups(NULL);
		HpGlitch(erase_arrow());
		yanked_out = bool_not(yanked_out);
		show_selection_page();
		info_message(_(txt_yanked_sub_groups));
	}
}


/*
 * Sort active[] and associated tin_sort() helper function
 */
static int
active_comp(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_group *s1 = (const struct t_group *) p1;
	const struct t_group *s2 = (const struct t_group *) p2;

	return strcasecmp(s1->name, s2->name);
}


/*
 * Call with TRUE to file away the current cursor position
 * Call again with FALSE to return a suggested value to restore the
 * current cursor (selmenu.curr) position
 */
static int
save_restore_curr_group(
	t_bool saving)
{
	static char *oldgroup;
	static int oldmax = 0;
	int ret;

	/*
	 * Take a copy of the current groupname, if present
	 */
	if (saving) {
		oldmax = selmenu.max;
		if (oldmax)
			oldgroup = my_strdup(CURR_GROUP.name);
		return 0;
	}

	/*
	 * Find & return the new screen position of the group
	 */
	ret = -1;

	if (oldmax) {
		ret = my_group_find(oldgroup);
		FreeAndNull(oldgroup);
	}

	if (ret == -1) {		/* Group not present, return something semi-useful */
		if (selmenu.max > 0)
			ret = selmenu.max - 1;
		else
			ret = 0;
	}
	return ret;
}


static void
sort_active_file(
	void)
{
	save_restore_curr_group(TRUE);

	tin_sort(active, (size_t) num_active, sizeof(struct t_group), active_comp);
	group_rehash(yanked_out);

	selmenu.curr = save_restore_curr_group(FALSE);

	show_selection_page();
}


int
choose_new_group(
	void)
{
	char *prompt;
	int idx;

	prompt = fmt_string(_(txt_newsgroup), tinrc.default_goto_group);

	if (!(prompt_string_default(prompt, tinrc.default_goto_group, "", HIST_GOTO_GROUP))) {
		free(prompt);
		return -1;
	}
	free(prompt);

	str_trim(tinrc.default_goto_group);

	if (tinrc.default_goto_group[0] == '\0')
		return -1;

	clear_message();

	if ((idx = my_group_add(tinrc.default_goto_group, TRUE)) == -1)
		info_message(_(txt_not_in_active_file), tinrc.default_goto_group);

	return idx;
}


/*
 * Return new value for selmenu.max, skipping any new newsgroups that have been
 * found
 */
int
skip_newgroups(
	void)
{
	int i = 0;

	if (selmenu.max) {
		while (i < selmenu.max && active[my_group[i]].newgroup)
			i++;
	}

	return i;
}


/*
 * Find a group in the users selection list, my_group[].
 * If 'add' is TRUE, then add the supplied group return the index into
 * my_group[] if group is added or was already there. Return -1 if group
 * is not in active[]
 *
 * NOTE: can't be static due to my_group_add() macro
 */
int
add_my_group(
	const char *group,
	t_bool add,
	t_bool ignore_case)
{
	int i, j;

	if ((i = find_group_index(group, ignore_case)) < 0)
		return -1;

	for (j = 0; j < selmenu.max; j++) {
		if (my_group[j] == i)
			return j;
	}

	if (add) {
		my_group[selmenu.max++] = i;
		return (selmenu.max - 1);
	}

	return -1;
}


static int
reposition_group(
	struct t_group *group,
	int default_num)
{
	char buf[LEN];
	char pos[LEN];
	int pos_num, newgroups;

	/* Have already trapped no_write at this point */

	snprintf(buf, sizeof(buf), _(txt_newsgroup_position), group->name,
		(tinrc.default_move_group ? tinrc.default_move_group : default_num + 1));

	if (!prompt_string(buf, pos, HIST_MOVE_GROUP))
		return default_num;

	if (*pos)
		pos_num = ((pos[0] == '$') ? selmenu.max : atoi(pos));
	else {
		if (tinrc.default_move_group)
			pos_num = tinrc.default_move_group;
		else
			return default_num;
	}

	if (pos_num > selmenu.max)
		pos_num = selmenu.max;
	else if (pos_num <= 0)
		pos_num = 1;

	newgroups = skip_newgroups();

	/*
	 * Can't move into newgroups, they aren't in .newsrc
	 */
	if (pos_num <= newgroups) {
		error_message(2, _(txt_skipping_newgroups));
		return default_num;
	}

	wait_message(0, _(txt_moving), group->name);

	/*
	 * seems to have the side effect of rearranging
	 * my_groups, so tinrc.show_only_unread_groups has to be updated
	 */
	tinrc.show_only_unread_groups = FALSE;

	/*
	 * New newgroups aren't in .newsrc so we need to offset to
	 * get the right position
	 */
	if (pos_group_in_newsrc(group, pos_num - newgroups)) {
		read_newsrc(newsrc, TRUE);
		tinrc.default_move_group = pos_num;
		return (pos_num - 1);
	} else {
		tinrc.default_move_group = default_num + 1;
		return default_num;
	}
}


static void
catchup_group(
	struct t_group *group,
	t_bool goto_next_unread_group)
{
	char *smsg = NULL;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if ((!TINRC_CONFIRM_ACTION) || prompt_yn(sized_message(&smsg, _(txt_mark_group_read), group->name), TRUE) == 1) {
		grp_mark_read(group, NULL);
		{
			char buf[LEN];
			size_t i = 0;

			while (i < sel_fmt.len_ucnt)
				buf[i++] = ' ';
			buf[i] = '\0';
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if ((wtmp = char2wchar_t(buf))) {
				mark_screen(selmenu.curr, ucnt_offset, wtmp);
				free(wtmp);
			}
#else
			mark_screen(selmenu.curr, ucnt_offset, buf);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}

		if (goto_next_unread_group)
			pos_next_unread_group(TRUE);
		else
			move_down();
	}
	FreeIfNeeded(smsg);
}


/*
 * Set selmenu.curr to next group with unread arts
 * If the current group has unread arts, it will be found first !
 * If redraw is set, update the selection menu appropriately
 * Return FALSE if no groups left to read
 *        TRUE  at all other times
 */
static t_bool
pos_next_unread_group(
	t_bool redraw)
{
	int i;
	t_bool all_groups_read = TRUE;

	if (!selmenu.max)
		return FALSE;

	for (i = selmenu.curr; i < selmenu.max; i++) {
		if (UNREAD_GROUP(i)) {
			all_groups_read = FALSE;
			break;
		}
	}

	if (all_groups_read) {
		for (i = 0; i < selmenu.curr; i++) {
			if (UNREAD_GROUP(i)) {
				all_groups_read = FALSE;
				break;
			}
		}
	}

	if (all_groups_read) {
		info_message(_(txt_no_groups_to_read));
		return FALSE;
	}

	if (redraw)
		move_to_item(i);
	else
		selmenu.curr = i;

	return TRUE;
}


/*
 * This is the main loop that cycles through, reading groups.
 * We keep going until we return to the selection screen or exit tin
 */
static void
read_groups(
	void)
{
	t_bool done = FALSE;

	clear_message();

	while (!done) {		/* if xmin > xmax the newsservers active is broken */
		switch (group_page(&CURR_GROUP)) {
			case GRP_QUIT:
				select_quit();
				break;

			case GRP_NEXT:
				if (selmenu.curr + 1 < selmenu.max)
					selmenu.curr++;
				done = TRUE;
				break;

			case GRP_NEXTUNREAD:
				if (!pos_next_unread_group(FALSE))
					done = TRUE;
				break;

			case GRP_ENTER:		/* group_page() has already set selmenu.curr */
				break;

			case GRP_RETSELECT:
			case GRP_EXIT:
			default:
				done = TRUE;
				break;
		}
	}

	if (!need_reread_active_file())
		show_selection_page();
}


/*
 * Toggle my_group[] between all groups / only unread groups
 * We make a special case for Newgroups (always appear, at the top)
 * and Bogus groups if tinrc.strip_bogus = BOGUS_SHOW
 */
void
toggle_my_groups(
	const char *group)
{
#if 1
	FILE *fp;
	char buf[NEWSRC_LINE];
	char *ptr;
#endif /* 1 */
	int i;

	/*
	 * Save current or next group with unread arts for later use
	 */
	if (selmenu.max) {
		int old_curr_group_idx = 0;

		if (group != NULL) {
			if ((i = my_group_find(group)) >= 0)
				old_curr_group_idx = i;
		} else
			old_curr_group_idx = (selmenu.curr == -1) ? 0 : selmenu.curr;

		if (tinrc.show_only_unread_groups) {
			for (i = old_curr_group_idx; i < selmenu.max; i++) {
				if (UNREAD_GROUP(i) || active[my_group[i]].newgroup) {
					old_curr_group_idx = i;
					break;
				}
			}
		}
		selmenu.curr = old_curr_group_idx;	/* Set current group to save */
	} else
		selmenu.curr = 0;

	save_restore_curr_group(TRUE);

	selmenu.max = skip_newgroups();			/* Reposition after any newgroups */

	/* TODO: why re-read .newsrc here, instead of something like this... */
#if 0
	for_each_group(i) {
		if (active[i].subscribed) {
			if (tinrc.show_only_unread_groups) {
				if (active[i].newsrc.num_unread > 0 || (active[i].bogus && tinrc.strip_bogus == BOGUS_SHOW))
					my_group[selmenu.max++] = i;
			} else
				my_group[selmenu.max++] = i;
		}
	}
#else
	/* preserve group ordering based on newsrc */
	if ((fp = fopen(newsrc, "r")) == NULL)
		return;

	while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
		if ((ptr = strchr(buf, SUBSCRIBED)) != NULL) {
			*ptr = '\0';

			if ((i = find_group_index(buf, FALSE)) < 0)
				continue;

			if (tinrc.show_only_unread_groups) {
				if (active[i].newsrc.num_unread || (active[i].bogus && tinrc.strip_bogus == BOGUS_SHOW))
					my_group_add(buf, FALSE);
			} else
				my_group_add(buf, FALSE);
		}
	}
	fclose(fp);
#endif /* 0 */
	selmenu.curr = save_restore_curr_group(FALSE);			/* Restore saved group position */
}


/*
 * Subscribe or unsubscribe from a list of groups. List can be full list as
 * supported by match_group_list()
 */
static void
subscribe_pattern(
	const char *prompt,
	const char *message,
	const char *result,
	t_bool state)
{
	char buf[LEN];
	int i, subscribe_num = 0;

	if (!num_active || no_write)
		return;

	if (!prompt_string(prompt, buf, HIST_OTHER) || !*buf) {
		clear_message();
		return;
	}

	wait_message(0, "%s", message);

	for_each_group(i) {
		if (match_group_list(active[i].name, buf)) {
			if (active[i].subscribed != (state != FALSE)) {
				spin_cursor();
				/* If found and group is not subscribed add it to end of my_group[]. */
				subscribe(&active[i], SUB_CHAR(state), TRUE);
				if (state) {
					my_group_add(active[i].name, FALSE);
					grp_mark_unread(&active[i]);
				}
				subscribe_num++;
			}
		}
	}

	if (subscribe_num) {
		toggle_my_groups(NULL);
		show_selection_page();
		info_message(result, subscribe_num);
	} else
		info_message(_(txt_no_match));
}


/*
 * Does NOT return
 */
_Noreturn static void
select_quit(
	void)
{
	write_config_file(local_config_file);
	ClearScreen();
	tin_done(EXIT_SUCCESS, NULL);	/* Tin END */
}


static void
select_done(
	void)
{
	if (!TINRC_CONFIRM_TO_QUIT || prompt_yn(_(txt_quit), TRUE) == 1)
		select_quit();
	if (!no_write && prompt_yn(_(txt_save_config), TRUE) == 1) {
		write_config_file(local_config_file);
		write_newsrc();
	}
	show_selection_page();
}


static void
select_read_group(
	void)
{
	struct t_group *currgrp;

	if (!selmenu.max || selmenu.curr == -1) {
		info_message(_(txt_no_groups));
		return;
	}

	currgrp = &CURR_GROUP;

	if (currgrp->bogus) {
		info_message(_(txt_not_exist));
		return;
	}

	if (currgrp->xmax > 0 && (currgrp->xmin <= currgrp->xmax))
		read_groups();
	else
		info_message(_(txt_no_arts));
}


#ifdef NNTP_ABLE
/*
 * Try to fetch articles Nesgroups-header via [X]HDR or XPAT.
 */
static char *
lookup_msgid(
	char *msgid)
{
	if (read_news_via_nntp && !read_saved_news) {
		if (!nntp_caps.hdr_cmd && !nntp_caps.xpat) {
			info_message(_(txt_lookup_func_not_available));
			return NULL;
		}
		if (msgid) {
			char *ptr, *r = NULL;
			static char *x;
			char buf[NNTP_STRLEN];
			int ret;

			if (nntp_caps.hdr_cmd) {
				snprintf(buf, sizeof(buf), "%s Newsgroups %s", nntp_caps.hdr_cmd, msgid);
				ret = new_nntp_command(buf, (nntp_caps.type == CAPABILITIES) ? OK_HDR : OK_HEAD, NULL, 0);

				switch (ret) {
					case OK_HEAD:
					case OK_HDR:
						x = NULL;
						while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#		ifdef DEBUG
							if (debug & DEBUG_NNTP)
								debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#		endif /* DEBUG */

							if (ret == OK_HEAD) { /* RFC 2980 ("%s %s", id, grp) */
								if (!strncmp(ptr, msgid, strlen(msgid))) { /* INN, MPNews, Leafnode, Cnews nntpd */
									r = ptr + strlen(msgid) + 1;
								} else { /* DNEWS ("%d %s", num, grp) */
									r = ptr;
									while (*r && *r != ' ' && *r != '\t')
										r++;
									while (*r && (*r == ' ' || *r == '\t'))
										r++;
								}
							}

							if (ret == OK_HDR) { /* RFC 3977 ("0 %s", grp) */
								if (*ptr == '0' && (*(ptr + 1) == ' ' || *(ptr + 1) == '\t'))
									r = ptr + 2;
							}

							if (r) {
								FreeIfNeeded(x);	/* only required on bogus multi responses, just to be safe */
								x = my_strdup(r);
							}
						}

						if (x)
							return x;

						if (!r) {
#		ifdef DEBUG
								if ((debug & DEBUG_NNTP) && verbose > 1)
									debug_print_file("NNTP", "lookup_msgid(%s) response empty or not recognized", buf);
#		endif /* DEBUG */
								if (!nntp_caps.xpat)
									info_message(_(txt_lookup_func_not_available));
						}
						if (r || !nntp_caps.xpat)
							return NULL;
						break;

					case ERR_NOART:
						info_message(_(txt_art_unavailable));
						return NULL;

					default:
						if (!nntp_caps.xpat) { /* try only once */
							info_message(_(txt_lookup_func_not_available));
							return NULL;
						}
						break;
				}
			}

			if (nntp_caps.xpat) {
				snprintf(buf, sizeof(buf), "XPAT Newsgroups %s *", msgid);
				ret = new_nntp_command(buf, OK_HEAD, NULL, 0);
				x = NULL;
				switch (ret) {
					case OK_HEAD:
						while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#		ifdef DEBUG
							if (debug & DEBUG_NNTP)
								debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#		endif /* DEBUG */
							if (!strncmp(ptr, msgid, strlen(msgid)))
								r = ptr + strlen(msgid) + 1;

							if (r) {
								FreeIfNeeded(x); /* only required on bogus multi responses, just to be safe */
								x = my_strdup(r);
							}
						}

						if (x)
							return x;

						if (!r) {
#		ifdef DEBUG
								if ((debug & DEBUG_NNTP) && verbose > 1)
									debug_print_file("NNTP", "lookup_msgid(%s) response empty or not recognized", buf);
#		endif /* DEBUG */
								info_message(_(txt_lookup_func_not_available));
								/* nntp_caps.xpat = FALSE; */ /* ? */
						}
						return NULL;

					case ERR_NOART:
						info_message(_(txt_art_unavailable));
						return NULL;

					default:
						nntp_caps.xpat = FALSE;
						break;
				}
			}
			info_message(_(txt_lookup_func_not_available));
		}
	} else
		info_message("%s %s", _(txt_lookup_func_not_available), _(txt_lookup_func_not_nntp));

	return NULL;
}


/*
 * Get a message ID for the 'L' command. Add <> if needed.
 * Try to enter an appropriate group and display the referenced article.
 * If no group from the Newsgroups:-header is available, display the
 * contents of the header.
 */
int
show_article_by_msgid(
	char *messageid)
{
	char id[NNTP_STRLEN];	/* still way too big; RFC 3977 3.6 & RFC 5536 3.1.3 limit Message-ID to max 250 octets */
	char *idptr = NULL;
	char *newsgroups = NULL;
	int i, ret = 0;
	struct t_article *art;
	struct t_group *group = NULL;
	struct t_group *tmp_group = NULL;
	struct t_msgid *msgid = NULL;
	t_bool tmp_cache_overview_files;
	t_bool tmp_show_only_unread_arts;

	if (!(read_news_via_nntp && !read_saved_news)) {
		return LOOKUP_UNAVAIL;
	}

	if (messageid) {
		idptr = messageid;
		newsgroups = lookup_msgid(idptr);
	} else {
		if (prompt_string(_(txt_enter_message_id), id + 1, HIST_MESSAGE_ID) && id[1]) {
			idptr = str_trim(id + 1);
			if (id[1] != '<') {
				id[0] = '<';
				strcat(id, ">");
				idptr = id;
			}
			newsgroups = lookup_msgid(idptr);
		}
	}

	if (!newsgroups)
		return LOOKUP_ART_UNAVAIL;

	if ((group = get_group_from_list(newsgroups)) == NULL) {
		info_message(strchr(newsgroups, ',') ? _(txt_lookup_show_groups) : _(txt_lookup_show_group), newsgroups);
		free(newsgroups);
		return LOOKUP_FAILED;
	}

	if (curr_group)
		tmp_group = curr_group;
	curr_group = group;
	num_of_tagged_arts = 0;
	range_active = FALSE;
	last_resp = -1;
	this_resp = -1;
	tmp_cache_overview_files = tinrc.cache_overview_files;
	tinrc.cache_overview_files = FALSE;
	tmp_show_only_unread_arts = curr_group->attribute->show_only_unread_arts;
	curr_group->attribute->show_only_unread_arts = FALSE;

	if (!index_group(group)) {
		for_each_art(i) {
			art = &arts[i];
			FreeAndNull(art->refs);
			FreeAndNull(art->msgid);
		}
		tin_errno = 0;
		ret = LOOKUP_FAILED;
	}

	if (!ret) {
		grpmenu.first = 0;

		if (*idptr == '\0')
			ret = LOOKUP_ART_UNAVAIL;

		if ((msgid = find_msgid(idptr)) == NULL)
			ret = LOOKUP_ART_UNAVAIL;

		if (!ret && msgid->article == ART_UNAVAILABLE)
			ret = LOOKUP_ART_UNAVAIL;

		if (!ret && which_thread(msgid->article) == -1)
			ret = LOOKUP_NO_LAST;
	}

	if (!ret) {
		switch (show_page(group, msgid->article, NULL)) {
			case GRP_QUIT:
				ret = LOOKUP_QUIT;
				break;

			default:
				break;
		}
	}

	free(newsgroups);
	art_close(&pgart);
	tinrc.cache_overview_files = tmp_cache_overview_files;
	curr_group->attribute->show_only_unread_arts = CAST_BOOL(tmp_show_only_unread_arts);
	if (tmp_group) {
		curr_group = tmp_group;
		if (!index_group(curr_group)) {
			for_each_art(i) {
				art = &arts[i];
				FreeAndNull(art->refs);
				FreeAndNull(art->msgid);
			}
			curr_group = NULL;
			tin_errno = 0;
			ret = LOOKUP_FAILED;
		}
	} else
		curr_group = NULL;

	this_resp = last_resp = -1;

	return ret;
}


/*
 * Takes a list of newsgroups and determines if one of them is available.
 */
static struct t_group *
get_group_from_list(
	char *newsgroups)
{
	char *ptr, *tr;
	t_bool found = FALSE;
	struct t_group *group = NULL;

	if (!newsgroups || (ptr = strtok(newsgroups, ",")) == NULL)
		return NULL;

	/* find first available group of type news */
	do {
		tr = str_trim(ptr);
		group = group_find(tr, TRUE);
		if (group && group->type == GROUP_TYPE_NEWS)
			found = TRUE;
	} while (!found && (ptr = strtok(NULL, ",")) != NULL);

	return found ? group : NULL;
}
#endif /* NNTP_ABLE */
