/*
 *  Project   : tin - a Usenet reader
 *  Module    : select.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2003-07-20
 *  Notes     :
 *
 * Copyright (c) 1991-2003 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

/*
 * Local prototypes
 */
static int active_comp(t_comptype p1, t_comptype p2);
static int reposition_group(struct t_group *group, int default_num);
static int save_restore_curr_group(t_bool saving);
static int select_left(void);
static int select_right(void);
static t_bool pos_next_unread_group(t_bool redraw);
static t_bool yanked_out = TRUE;
static void catchup_group(struct t_group *group, t_bool goto_next_unread_group);
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
 * selmenu.first, selmenu.last are static here
 */
t_menu selmenu = { 1, 0, 0, 0, show_selection_page, draw_group_arrow };


static int
select_left(
	void)
{
	return iKeyQuit;
}


static int
select_right(
	void)
{
	return iKeySelectReadGrp;
}


void
selection_page(
	int start_groupnum,
	int num_cmd_line_groups)
{
	char buf[LEN];
	char key[MAXKEYLEN];
	int i, n, ch;

	selmenu.curr = start_groupnum;

#ifdef READ_CHAR_HACK
	setbuf(stdin, 0);
#endif /* READ_CHAR_HACK */

	ClearScreen();
	set_groupname_len(FALSE);	/* find longest subscribed to groupname */

	/*
	 * If user specified only 1 cmd line groupname (eg. tin -r alt.sources)
	 * then go there immediately.
	 */
	if (num_cmd_line_groups == 1)
		select_read_group();

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

		switch ((ch = handle_keypad(select_left, select_right, &menukeymap.select_nav))) {
			case iKeyAbort:		/* Abort */
				break;

			case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				if (selmenu.max)
					prompt_item_num(ch, _(txt_select_group));
				else
					info_message(_(txt_no_groups));
				break;

#ifndef NO_SHELL_ESCAPE
			case iKeyShellEscape:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case iKeyFirstPage:	/* show first page of groups */
				top_of_list();
				break;

			case iKeyLastPage:	/* show last page of groups */
				end_of_list();
				break;

			case iKeyPageUp:		/* page up */
			case iKeyPageUp2:
			case iKeyPageUp3:
				page_up();
				break;

			case iKeyPageDown:		/* page down */
			case iKeyPageDown2:
			case iKeyPageDown3:
				page_down();
				break;

			case iKeyUp:		/* line up */
			case iKeyUp2:
				move_up();
				break;

			case iKeyDown:		/* line down */
			case iKeyDown2:
				move_down();
				break;

			case iKeySelectSortActive:	/* Sort active groups */
				sort_active_file();
				break;

			case iKeySetRange:	/* set range */
				if (selmenu.max) {
					if (set_range(SELECT_LEVEL, 1, selmenu.max, selmenu.curr + 1))
						show_selection_page();
				} else
					info_message(_(txt_no_groups));
				break;

			case iKeySearchSubjF:	/* search forward */
			case iKeySearchSubjB:	/* search backward */
			case iKeySearchRepeat:
				if (ch == iKeySearchRepeat && i_key_search_last != iKeySearchSubjF && i_key_search_last != iKeySearchSubjB)
					info_message(_(txt_bad_command), printascii(key, map_to_local(iKeyHelp, &menukeymap.select_nav)));
				else {
					if ((i = search_active((ch == iKeySearchSubjF), (ch == iKeySearchRepeat))) != -1) {
						move_to_item(i);
						clear_message();
					}
				}
				break;

			case iKeySelectReadGrp:	/* go into group */
			case iKeySelectReadGrp2:
				select_read_group();
				break;

			case iKeySelectEnterNextUnreadGrp:	/* enter next group containing unread articles */
			case iKeySelectEnterNextUnreadGrp2:
				if (pos_next_unread_group(FALSE))
					read_groups();
				break;							/* Nothing more to do at the moment */

			case iKeyRedrawScr:		/* redraw */
				my_retouch();					/* TODO: not done elsewhere, maybe should be in show_selection_page */
				set_xclick_off();
				show_selection_page();
				break;

			case iKeySelectResetNewsrc:	/* reset .newsrc */
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				if (prompt_yn(cLINES, _(txt_reset_newsrc), FALSE) == 1) {
					reset_newsrc();
					sync_active_file();
					selmenu.curr = 0;
					show_selection_page();
				}
				break;

			case iKeySelectCatchup:	/* catchup - mark all articles as read */
			case iKeySelectCatchupNextUnread:	/* and goto next unread group */
				if (selmenu.max)
					catchup_group(&CURR_GROUP, (ch == iKeySelectCatchupNextUnread));
				else
					info_message(_(txt_no_groups));
				break;

			case iKeySelectToggleDescriptions:	/* toggle newsgroup descriptions */
				show_description = bool_not(show_description);
				if (show_description)
					read_descriptions(TRUE);
				set_groupname_len(FALSE);
				show_selection_page();
				break;

			case iKeySelectGoto:			/* prompt for a new group name */
				if ((n = choose_new_group()) >= 0) {
					set_groupname_len(FALSE);
					move_to_item(n);
				}
				break;

			case iKeyHelp:					/* help */
				show_help_page(SELECT_LEVEL, _(txt_group_select_com));
				show_selection_page();
				break;

			case iKeyToggleHelpDisplay:		/* toggle mini help menu */
				toggle_mini_help(SELECT_LEVEL);
				show_selection_page();
				break;

			case iKeyToggleInverseVideo:	/* toggle inverse video */
				toggle_inverse_video();
				show_selection_page();
				show_inverse_video_status();
				break;

#ifdef HAVE_COLOR
			case iKeyToggleColor:			/* toggle color */
				if (toggle_color()) {
					show_selection_page();
					show_color_status();
				}
				break;
#endif /* HAVE_COLOR */

			case iKeyToggleInfoLastLine:	/* display group description */
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				show_selection_page();
				break;

			case iKeySelectMoveGrp:	/* reposition group within group list */
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
				if (selmenu.curr < selmenu.first || selmenu.curr >= selmenu.last || selmenu.curr != n)
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

			case iKeyOptionMenu:	/* option menu */
				(void) change_config_file(NULL);
				read_attributes_files();
				show_selection_page();
				break;

			case iKeySelectNextUnreadGrp:	/* goto next unread group */
				pos_next_unread_group(TRUE);
				break;

			case iKeyQuit:	/* quit */
				select_done();
				break;

			case iKeyQuitTin:	/* quit, no ask */
				select_quit();
				break;

			case iKeySelectQuitNoWrite:	/* quit, but don't save configuration */
				if (prompt_yn(cLINES, _(txt_quit_no_write), TRUE) == 1)
					tin_done(EXIT_SUCCESS);
				show_selection_page();
				break;

			case iKeySelectToggleReadDisplay:
				/*
				 * If in tinrc.show_only_unread_groups mode toggle all
				 * subscribed to groups and only groups that contain unread
				 * articles
				 */
				tinrc.show_only_unread_groups = bool_not(tinrc.show_only_unread_groups);
				wait_message(0, _(txt_reading_groups), (tinrc.show_only_unread_groups) ? _("unread") : _("all"));

				toggle_my_groups(NULL);
				set_groupname_len(FALSE);
				show_selection_page();
				if (tinrc.show_only_unread_groups)
					info_message(_(txt_show_unread));
				break;

			case iKeySelectBugReport:
				bug_report();
				break;

			case iKeySelectSubscribe:	/* subscribe to current group */
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

			case iKeySelectSubscribePat:	/* subscribe to groups matching pattern */
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				subscribe_pattern(_(txt_subscribe_pattern), _(txt_subscribing), _(txt_subscribed_num_groups), TRUE);
				break;

			case iKeySelectUnsubscribe:	/* unsubscribe to current group */
				if (!selmenu.max) {
					info_message(_(txt_no_groups));
					break;
				}
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				if (CURR_GROUP.subscribed) {
					mark_screen(SELECT_LEVEL, selmenu.curr - selmenu.first, 2, CURR_GROUP.newgroup ? "N" : "u");
					subscribe(&CURR_GROUP, UNSUBSCRIBED, TRUE);
					info_message(_(txt_unsubscribed_to), CURR_GROUP.name);
					move_down();
				} else if (CURR_GROUP.bogus && tinrc.strip_bogus == BOGUS_SHOW) {
					/* Bogus groups aren't subscribed to avoid confusion */
					/* Note that there is no way to remove the group from active[] */
					sprintf(buf, _(txt_remove_bogus), CURR_GROUP.name);
					write_newsrc();					/* save current newsrc */
					delete_group(CURR_GROUP.name);		/* remove bogus group */
					read_newsrc(newsrc, TRUE);			/* reload newsrc */
					toggle_my_groups(NULL);			/* keep current display-state */
					show_selection_page();				/* redraw screen */
					info_message(buf);
				}
				break;

			case iKeySelectUnsubscribePat:	/* unsubscribe to groups matching pattern */
				if (no_write) {
					info_message(_(txt_info_no_write));
					break;
				}
				subscribe_pattern(_(txt_unsubscribe_pattern),
								_(txt_unsubscribing), _(txt_unsubscribed_num_groups), FALSE);
				break;

			case iKeyVersion:	/* show tin version */
				info_message(cvers);
				break;

			case iKeyPost:	/* post a basenote */
				if (!selmenu.max) {
					sprintf(buf, _(txt_post_newsgroups), tinrc.default_post_newsgroups);
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

			case iKeyPostponed:
			case iKeyPostponed2:	/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles(FALSE, FALSE))
						show_selection_page();
				} else
					info_message(_(txt_cannot_post));
				break;

			case iKeyDisplayPostHist:	/* display messages posted by user */
				if (user_posted_messages())
					show_selection_page();
				break;

			case iKeySelectYankActive:	/* yank in/out rest of groups from active */
				yank_active_file();
				break;

			case iKeySelectSyncWithActive:	/* Re-read active file to see if any new news */
				sync_active_file();
				if (!yanked_out)
					yank_active_file();			/* yank out if yanked in */
				break;

			case iKeySelectMarkGrpUnread:
			case iKeySelectMarkGrpUnread2:	/* mark group unread */
				if (!selmenu.max) {
					info_message(_(txt_no_groups));
					break;
				}
				grp_mark_unread(&CURR_GROUP);
				if (CURR_GROUP.newsrc.num_unread)
					strcpy(mesg, tin_ltoa(CURR_GROUP.newsrc.num_unread, 5));
				else
					strcpy(mesg, "     ");
				mark_screen(SELECT_LEVEL, selmenu.curr - selmenu.first, 9, mesg);
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, map_to_local(iKeyHelp, &menukeymap.select_nav)));
		}
	}
}


void
show_selection_page(
	void)
{
	char buf[LEN];
	char tmp[10];
	char active_name[255];
	char group_descript[255];
	char subs;
	int i, j, n;
	int blank_len;

	signal_context = cSelect;
	currmenu = &selmenu;

	if (read_news_via_nntp)
		snprintf(buf, sizeof(buf), "%s (%s  %d%s)", _(txt_group_selection), nntp_server, selmenu.max, (tinrc.show_only_unread_groups ? _(" R") : ""));
	else
		snprintf(buf, sizeof(buf), "%s (%d%s)", _(txt_group_selection), selmenu.max, (tinrc.show_only_unread_groups ? _(" R") : ""));

	MoveCursor(0, 0);		/* top left corner */
	CleartoEOLN();
	show_title(buf);
	MoveCursor(1, 0);
	CleartoEOLN();
	MoveCursor(INDEX_TOP, 0);

	if (selmenu.curr < 0)
		selmenu.curr = 0;

	set_first_screen_item();

	blank_len = (MIN(cCOLS, (int) sizeof(group_descript)) - (groupname_len + SELECT_MISC_COLS)) + (show_description ? 2 : 4);

	for (j = 0, i = selmenu.first; i < selmenu.last; i++, j++) {
#ifdef USE_CURSES
		char sptr[BUFSIZ];
#else
		char *sptr = screen[j].col;
#endif /* USE_CURSES */
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

		if (blank_len > (int) (sizeof(group_descript) - 1))
			blank_len = sizeof(group_descript) - 1;

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

		CleartoEOLN();
		my_fputs(sptr, stdout);
	}

	CleartoEOS();
	show_mini_help(SELECT_LEVEL);

	if (selmenu.max <= 0) {
		info_message(_(txt_no_groups));
		return;
	} else if (selmenu.last == selmenu.max)
		info_message(_(txt_end_of_groups));

	draw_group_arrow();
}


void
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
	if (yanked_out) {										/* Yank in */
		if (selmenu.max == num_active)						/* All groups currently present? */
			info_message(_(txt_yanked_none));
		else {
			int i;
			int prevmax = selmenu.max;

			save_restore_curr_group(TRUE);					/* Save group position */

			/*
			 * Reset counter and load all the groups in active[] into my_group[]
			 */
			selmenu.max = 0;
			for_each_group(i)
				my_group[selmenu.max++] = i;

			selmenu.curr = save_restore_curr_group(FALSE);	/* Restore previous group position */
			set_groupname_len(yanked_out);
			show_selection_page();
			info_message(_(txt_yanked_groups), selmenu.max-prevmax, PLURAL(selmenu.max-prevmax, txt_group));
		}
	} else {												/* Yank out */
		toggle_my_groups(NULL);
		HpGlitch(erase_arrow());
		set_groupname_len(yanked_out);
		show_selection_page();
		info_message(_(txt_yanked_sub_groups));
	}

	yanked_out = bool_not(yanked_out);
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
	int idx;

	sprintf(mesg, _(txt_newsgroup), tinrc.default_goto_group);

	if (!(prompt_string_default(mesg, tinrc.default_goto_group, "", HIST_GOTO_GROUP)))
		return -1;

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

	sprintf(buf, _(txt_newsgroup_position), group->name,
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
	if ((!TINRC_CONFIRM_ACTION) || prompt_yn(cLINES, sized_message(_(txt_mark_group_read), group->name), TRUE) == 1) {
		grp_mark_read(group, NULL);
		mark_screen(SELECT_LEVEL, selmenu.curr - selmenu.first, 9, "     ");

		if (goto_next_unread_group)
			pos_next_unread_group(TRUE);
		else
			move_down();
	}
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
 * Calculate max length of groupname field for group selection level.
 * If all_groups is TRUE check all groups in active file otherwise
 * just subscribed to groups.
 */
void
set_groupname_len(
	t_bool all_groups)
{
	int len;
	int i;

	groupname_len = 0;

	if (all_groups) {
		for_each_group(i) {
			if ((len = strlen(active[i].name)) > groupname_len)
				groupname_len = len;
		}
	} else {
		for (i = 0; i < selmenu.max; i++) {
			if ((len = strlen(active[my_group[i]].name)) > groupname_len)
				groupname_len = len;
		}
	}

	if (groupname_len >= (cCOLS - SELECT_MISC_COLS)) {
		groupname_len = cCOLS - SELECT_MISC_COLS - 1;
		if (groupname_len < 0)
			groupname_len = 0;
	}

	/*
	 * If newsgroups descriptions are ON then cut off groupnames
	 * to specified max. length otherwise display full length
	 */
	if (show_description && groupname_len > tinrc.groupname_max_length)
		groupname_len = tinrc.groupname_max_length;
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
		set_groupname_len(FALSE);
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
	if (!TINRC_CONFIRM_TO_QUIT || prompt_yn(cLINES, _(txt_quit), TRUE) == 1)
		select_quit();
	if (!no_write && prompt_yn(cLINES, _(txt_save_config), TRUE) == 1) {
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
