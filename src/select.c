/*
 *  Project   : tin - a Usenet reader
 *  Module    : select.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2005-07-02
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
static void select_quit(void);
static void select_read_group(void);
static void sort_active_file(void);
static void subscribe_pattern(const char *prompt, const char *message, const char *result, t_bool state);
static void sync_active_file(void);
static void yank_active_file(void);


/*
 * selmenu.curr = index (start at 0) of cursor position on menu,
 *                or -1 when no groups visible on screen
 * selmenu.max = Total # of groups in my_group[]
 * selmenu.first is static here
 */
t_menu selmenu = { 1, 0, 0, show_selection_page, draw_group_arrow, build_gline };

static int groupname_len;	/* max. group name length */


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

	selmenu.curr = start_groupnum;

#ifdef READ_CHAR_HACK
	setbuf(stdin, 0);
#endif /* READ_CHAR_HACK */

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
				yanked_out = bool_not(yanked_out); /* yank out if yanked in */
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
					info_message(_(txt_bad_command), printascii(key, key_to_func(GLOBAL_HELP, select_keys)));
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
				if (!invoke_editor(filter_file, FILTER_FILE_OFFSET))
					break;
				(void) read_filter_file(filter_file);
				break;

			case SELECT_TOGGLE_DESCRIPTIONS:	/* toggle newsgroup descriptions */
				show_description = bool_not(show_description);
				if (show_description)
					read_descriptions(TRUE);
				show_selection_page();
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
				(void) change_config_file(NULL);
				read_attributes_files();
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
					tin_done(EXIT_SUCCESS);
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
					mark_screen(selmenu.curr, 2, CURR_GROUP.newgroup ? "N" : "u");
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
					snprintf(buf, sizeof(buf), _(txt_post_newsgroups), tinrc.default_post_newsgroups);
					if (!prompt_string_default(buf, tinrc.default_post_newsgroups, _(txt_no_newsgroups), HIST_POST_NEWSGROUPS))
						break;
					if (group_find(tinrc.default_post_newsgroups) == NULL) {
						error_message(_(txt_not_in_active_file), tinrc.default_post_newsgroups);
						break;
					} else {
						strcpy(buf, tinrc.default_post_newsgroups);
#if 1 /* TODO: fix the rest of the code so we don't need this anymore */
						/*
						 * this is a gross hack to avoid a crash in the
						 * CHARSET_CONVERSION conversion case in new_part()
						 * which relies currently relies on CURR_GROUP
						 */
						selmenu.curr = my_group_add(buf);
						/*
						 * and the next hack to avoid a grabbled selection
						 * screen after the posting
						 */
						toggle_my_groups(NULL);
						toggle_my_groups(NULL);
#endif /* 1 */
					}
				} else
					strcpy(buf, CURR_GROUP.name);
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
				if (user_posted_messages())
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
				{
					char tmp[6];

					if (CURR_GROUP.newsrc.num_unread)
						STRCPY(tmp, tin_ltoa(CURR_GROUP.newsrc.num_unread, 5));
					else
						STRCPY(tmp, "     ");
					mark_screen(selmenu.curr, 9, tmp);
				}
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, func_to_key(GLOBAL_HELP, select_keys)));
		}
	}
}


void
show_selection_page(
	void)
{
	char buf[LEN];
	int i, len;

	signal_context = cSelect;
	currmenu = &selmenu;

	if (read_news_via_nntp)
		snprintf(buf, sizeof(buf), "%s (%s  %d%s)", _(txt_group_selection), nntp_server, selmenu.max, (tinrc.show_only_unread_groups ? _(" R") : ""));
	else
		snprintf(buf, sizeof(buf), "%s (%d%s)", _(txt_group_selection), selmenu.max, (tinrc.show_only_unread_groups ? _(" R") : ""));

	if (selmenu.curr < 0)
		selmenu.curr = 0;

	ClearScreen();
	set_first_screen_item();
	show_title(buf);

	/*
	 * calculate max length of groupname field
	 * if yanked in (yanked_out == FALSE) check all groups in active file
	 * otherwise just subscribed to groups
	 */
	if (yanked_out) {
		for (i = 0; i < selmenu.max; i++) {
			if ((len = strlen(active[my_group[i]].name)) > groupname_len)
				groupname_len = len;
			if (show_description && groupname_len > tinrc.groupname_max_length) {
				/* no need to search further, we have reached max length */
				groupname_len = tinrc.groupname_max_length;
				break;
			}
		}
	} else {
		for_each_group(i) {
			if ((len = strlen(active[i].name)) > groupname_len)
				groupname_len = len;
			if (show_description && groupname_len > tinrc.groupname_max_length) {
				/* no need to search further, we have reached max length */
				groupname_len = tinrc.groupname_max_length;
				break;
			}
		}
	}
	if (groupname_len >= (cCOLS - SELECT_MISC_COLS))
		groupname_len = cCOLS - SELECT_MISC_COLS - 1;
	if (groupname_len < 0)
		groupname_len = 0;

	for (i = selmenu.first; i < selmenu.first + NOTESLINES && i < selmenu.max; i++)
		build_gline(i);

	show_mini_help(SELECT_LEVEL);

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
	char tmp[10];
	char active_name[255];
	char group_descript[255];
	char subs;
#ifdef USE_CURSES
	char sptr[BUFSIZ];
#else
	char *sptr = screen[INDEX2SNUM(i)].col;
#endif /* USE_CURSES */
	int n, blank_len;

	blank_len = (MIN(cCOLS, (int) sizeof(group_descript)) - (groupname_len + SELECT_MISC_COLS)) + (show_description ? 2 : 4);

	if (active[my_group[i]].inrange)
		strcpy(tmp, "    #");
	else if (active[my_group[i]].newsrc.num_unread) {
		strcpy(tmp, tin_ltoa(active[my_group[i]].newsrc.num_unread, 5));
	} else
		strcpy(tmp, "     ");

	n = my_group[i];

	/*
	 * Display a flag for this group if needed
	 * . Bogus groups are dumped immediately
	 * . Normal subscribed groups may be
	 *   ' ' normal, 'X' not postable, 'M' moderated, '=' renamed
	 * . Newgroups are 'N'
	 * . Unsubscribed groups are 'u'
	 */
	if (active[n].bogus)					/* Group is not in active list */
		subs = 'D';
	else if (active[n].subscribed)			/* Important that this preceeds Newgroup */
		subs = group_flag(active[n].moderated);
	else
		subs = ((active[n].newgroup) ? 'N' : 'u'); /* New (but unsubscribed) group or unsubscribed group */

	strncpy(active_name, active[n].name, groupname_len);
	active_name[groupname_len] = '\0';

	if (show_description) {
		if (active[n].description) {
			strncpy(group_descript, active[n].description, blank_len);
			group_descript[blank_len] = '\0';
			sprintf(sptr, "  %c %s %s  %-*.*s  %-*.*s%s",
				 subs, tin_ltoa(i + 1, 4), tmp,
				 groupname_len, groupname_len, active_name,
				 blank_len, blank_len, group_descript, cCRLF);
		} else
			sprintf(sptr, "  %c %s %s  %-*.*s  %s",
				 subs, tin_ltoa(i + 1, 4), tmp,
				 (groupname_len + blank_len),
				 (groupname_len + blank_len), active[n].name, cCRLF);
	} else {
		if (tinrc.draw_arrow)
			sprintf(sptr, "  %c %s %s  %-*.*s%s", subs, tin_ltoa(i + 1, 4), tmp, groupname_len, groupname_len, active_name, cCRLF);
		else
			sprintf(sptr, "  %c %s %s  %-*.*s%*s%s", subs, tin_ltoa(i + 1, 4), tmp, groupname_len, groupname_len, active_name, blank_len, " ", cCRLF);
	}
	if (tinrc.strip_blanks)
		strcat(strip_line(sptr), cCRLF);

	WriteLine(INDEX2LNUM(i), sptr);
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
		info_message(_(txt_yanked_groups), selmenu.max-prevmax, PLURAL(selmenu.max-prevmax, txt_group));
	} else {							/* Yank out */
		toggle_my_groups(NULL);
		HpGlitch(erase_arrow());
		yanked_out = bool_not(yanked_out);
		show_selection_page();
		info_message(_(txt_yanked_sub_groups));
	}
}


/*
 * Sort active[] and associated qsort() helper function
 */
static int
active_comp(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_group *s1 = (const struct t_group *)p1;
	const struct t_group *s2 = (const struct t_group *)p2;

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
	static int oldmax;
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

	qsort(active, (size_t) num_active, sizeof(struct t_group), active_comp);
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

	if ((idx = my_group_add(tinrc.default_goto_group)) == -1)
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
 * NOTE: can't be static due to my_group_add() marco
 */
int
add_my_group(
	const char *group,
	t_bool add)
{
	int i, j;

	if ((i = find_group_index(group)) < 0)
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

	if (strlen(pos))
		pos_num = ((pos[0] == '$') ? selmenu.max: atoi(pos));
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
		error_message(_(txt_skipping_newgroups));
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

	if ((!TINRC_CONFIRM_ACTION) || prompt_yn(sized_message(&smsg, _(txt_mark_group_read), group->name), TRUE) == 1) {
		grp_mark_read(group, NULL);
		mark_screen(selmenu.curr, 9, "     ");

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

	return;
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
	if ((fp = fopen(newsrc, "r")) == NULL)
		return;

	while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
		if ((ptr = strchr(buf, SUBSCRIBED)) != NULL) {
			*ptr = '\0';

			if ((i = find_group_index(buf)) < 0)
				continue;

			if (tinrc.show_only_unread_groups) {
				if (active[i].newsrc.num_unread || (active[i].bogus && tinrc.strip_bogus == BOGUS_SHOW))
					my_group_add(buf);
			} else
				my_group_add(buf);
		}
	}
	fclose(fp);
#endif /* 0 */
	selmenu.curr = save_restore_curr_group(FALSE);			/* Restore saved group position */
}


/*
 * Subscribe or unsubscribe from a list of groups. List can be full list as supported
 * by match_group_list()
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

	wait_message(0, message);

	/*
	 * TODO: why do we do a pass over my_group[] before another one
	 *       over active[]? AFAICS the first loop can go away.
	 */
	for (i = 0; i < selmenu.max; i++) {
		if (match_group_list(active[my_group[i]].name, buf)) {
			if (active[my_group[i]].subscribed != (state != FALSE)) {
				spin_cursor();
				subscribe(&active[my_group[i]], SUB_CHAR(state), TRUE);
				subscribe_num++;
			}
		}
	}

	if (num_active > selmenu.max) {			/* ie, there are groups yanked out */
		for_each_group(i) {
			if (match_group_list(active[i].name, buf)) {
				if (active[i].subscribed != (state != FALSE)) {
					spin_cursor();
					/* If found and group is not subscribed add it to end of my_group[]. */
					subscribe(&active[i], SUB_CHAR(state), TRUE);
					if (state) {
						my_group_add(active[i].name);
						/*
						 * TODO: grp_mark_unread() or something needs to do a
						 *       group_get_art_info() to get initial count right
						 */
						grp_mark_unread(&active[i]);
					}
					subscribe_num++;
				}
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
static void
select_quit(
	void)
{
	write_config_file(local_config_file);
	tin_done(EXIT_SUCCESS);	/* Tin END */
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
