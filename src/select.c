/*
 *  Project   : tin - a Usenet reader
 *  Module    : select.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1994-12-21
 *  Notes     :
 *  Copyright : (c) Copyright 1991-99 by Iain Lea & Rich Skrenta
 *              You may  freely  copy or  redistribute  this software,
 *              so  long as there is no profit made from its use, sale
 *              trade or  reproduction.  You may not change this copy-
 *              right notice, and it must be included in any copy made
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

/*
 * Local prototypes
 */
static int handle_keypad (int ch, int ch1);
static int reposition_group (struct t_group *group, int default_num);
static int select_left (void);
static int select_right (void);
static t_bool continual_key (int ch, int ch1);
static t_bool pos_next_unread_group (t_bool redraw);
static void catchup_group (struct t_group *group, t_bool goto_next_unread_group);
static void erase_group_arrow (void);
static void read_groups (void);
static void select_quit (void);
static void select_read_group (void);
static void select_done (void);
static void subscribe_pattern (const char *prompt, const char *message, const char *result, t_bool state);
static void sync_active_file (void);
static void yank_active_file (void);


/*
 * selmenu.curr is always >= 0
 * selmenu.max = Total # of groups in my_group[]
 * selmenu.first, selmenu.last are static here
 */
t_menu selmenu = { 1, 0, 0, 0, show_selection_page, erase_group_arrow, draw_group_arrow };

/*
 *  TRUE, if we should check whether it's time to reread the active file
 *  after this keypress.
 */
static t_bool
continual_key (
	int ch,
	int ch1)
{
	switch(ch) {
#ifndef NO_SHELL_ESCAPE
		case iKeyShellEscape:
#endif /* !NO_SHELL_ESCAPE */
		/* case iKeyLookupMessage: */
		case iKeyOptionMenu:
		case iKeyQuit:
		case iKeyQuitTin:
		case iKeyPostponed:
		case iKeySelectResetNewsrc:
		case iKeySelectBugReport:
		case iKeyDisplayPostHist:
		case iKeySelectQuitNoWrite:
		case iKeySelectSyncActive:
		case iKeySelectHelp:
		case iKeySelectPost:
			return FALSE;

#ifndef WIN32
		case ESC:
#	ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
			switch(ch1) {
#endif /* !WIN32 */
				case KEYMAP_LEFT:
					return FALSE;
#ifndef WIN32
			}
			nobreak;	/* FALLTHROUGH */
#endif /* !WIN32 */

		default:
			return TRUE;
	}
}


static int
select_left (
	void)
{
	return iKeyQuit;
}


static int
select_right (
	void)
{
	return iKeySelectReadGrp;
}


static int
handle_keypad (
	int ch,
	int ch1)
{
/* fprintf(stderr, "KP %d %d\n", ch, ch1); */
#ifndef WIN32
	switch (ch1) {
#else
	switch (ch) {
#endif /* !WIN32 */
		case KEYMAP_UP:
			ch = iKeyUp;
			break;
		case KEYMAP_DOWN:
			ch = iKeyDown;
			break;
		case KEYMAP_LEFT:
			ch = select_left();
			break;
		case KEYMAP_RIGHT:
			ch = select_right();
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
/* fprintf(stderr, "Mouse event\n"); */
			ch = mouse_action(ch, select_left, select_right);
			break;
#endif /* !WIN32 */
		default:
			break;
	}
	return ch;
}


void
selection_page (
	int start_groupnum,
	int num_cmd_line_groups)
{
	char buf[LEN];
	int i, n, ch, ch1 = 0;

	selmenu.curr = start_groupnum;

#ifdef READ_CHAR_HACK
	setbuf (stdin, 0);
#endif /* READ_CHAR_HACK */

	ClearScreen();
	set_groupname_len (FALSE);	/* find longest subscribed to groupname */

	/*
	 * If user specified only 1 cmd line groupname (eg. tin -r alt.sources)
	 * then go there immediately.
	 */
	if (num_cmd_line_groups == 1)
		select_read_group();

	show_selection_page ();	/* display group selection page */

	forever {
		if (!resync_active_file () && reread_active_after_posting ())	/* reread active file if necessary */
			show_selection_page ();
		set_xclick_on ();
		ch = ReadCh ();
		ch1 = KEYMAP_UNKNOWN;
#ifndef WIN32
		switch(ch) {
			case ESC:	/* (ESC) common arrow keys */
#	ifdef HAVE_KEY_PREFIX
			case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
				ch1 = get_arrow_key (ch);
/* fprintf(stderr, "get_arrow_key=%d (%d)\n", ch1, ch); */
				ch = handle_keypad(ch, ch1);
/* fprintf(stderr, "handle_keypad = %d\n", ch); */
			default:
				break;
		}
#endif /* !WIN32 */

		if (continual_key (ch, ch1))
			(void) resync_active_file ();

		switch (ch) {

#ifndef WIN32
			case ESC:	/* (ESC) common arrow keys */
				break;
#endif /* !WIN32 */

			case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				if (selmenu.max)
					prompt_item_num (ch, txt_select_group);
				break;

#ifndef NO_SHELL_ESCAPE
			case iKeyShellEscape:
				shell_escape ();
				show_selection_page ();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case iKeyFirstPage:	/* show first page of groups */
				top_of_list();
				break;

			case iKeyLastPage:	/* show last page of groups */
				end_of_list();
				break;

			case iKeySetRange:	/* set range */
				if (bSetRange (SELECT_LEVEL, 1, selmenu.max, selmenu.curr+1))
					show_selection_page ();
				break;

			case iKeySearchSubjF:	/* search forward */
			case iKeySearchSubjB:	/* search backward */
				if ((i = search_active (ch == iKeySearchSubjF)) != -1) {
					move_to_item (i);
					clear_message ();
				}
				break;

			case iKeySelectReadGrp:	/* go into group */
			case iKeySelectReadGrp2:
				select_read_group();
				break;

			case iKeySelectEnterNextUnreadGrp:	/* enter next group containing unread articles */
			case iKeySelectEnterNextUnreadGrp2:
				if (pos_next_unread_group (FALSE))
					read_groups();
				break;							/* Nothing more to do at the moment */

			case iKeyPageDown:		/* page down */
			case iKeyPageDown2:
			case iKeyPageDown3:
				page_down();
				break;

			case iKeyRedrawScr:		/* redraw */
				my_retouch ();					/* TODO not done elsewhere, maybe should be in show_selection_page */
				set_xclick_off ();
				show_selection_page ();
				break;

			case iKeyDown:		/* line down */
			case iKeyDown2:
				move_down();
				break;

			case iKeyUp:		/* line up */
			case iKeyUp2:
				move_up();
				break;

			case iKeySelectResetNewsrc:	/* reset .newsrc */
				if (no_write) {
					wait_message(0, _(txt_info_no_write));
					break;
				}
				if (prompt_yn (cLINES, _(txt_reset_newsrc), FALSE) == 1) {
					reset_newsrc ();
					sync_active_file ();
					selmenu.curr = 0;
					show_selection_page ();
				}
				break;

			case iKeyPageUp:		/* page up */
			case iKeyPageUp2:
			case iKeyPageUp3:
				page_up();
				break;

			case iKeySelectCatchup:	/* catchup - mark all articles as read */
			case iKeySelectCatchupNextUnread:	/* and goto next unread group */
				if (!selmenu.max)
					break;
				catchup_group (&CURR_GROUP, (ch == iKeySelectCatchupNextUnread));
				break;

			case iKeySelectToggleDescriptions:	/* toggle newsgroup descriptions */
				show_description = !show_description;
				if (show_description)
					read_newsgroups_file ();
				set_groupname_len (FALSE);
				show_selection_page ();
				break;

			case iKeySelectGoto:	/* prompt for a new group name */
				if ((n = choose_new_group ()) >= 0) {
					set_groupname_len (FALSE);
					move_to_item (n);
				}
				break;

			case iKeySelectHelp:	/* help */
				show_info_page (HELP_INFO, help_select, _(txt_group_select_com));
				show_selection_page ();
				break;

			case iKeySelectToggleHelpDisplay:	/* toggle mini help menu */
				toggle_mini_help (SELECT_LEVEL);
				show_selection_page ();
				break;

			case iKeySelectToggleInverseVideo:	/* toggle inverse video */
				toggle_inverse_video ();
				show_selection_page ();
				show_inverse_video_status ();
				break;

#ifdef HAVE_COLOR
			case iKeyToggleColor:		/* toggle color */
				if (toggle_color ()) {
					show_selection_page ();
					show_color_status ();
				}
				break;
#endif /* HAVE_COLOR */

			case iKeyToggleInfoLastLine:	/* display group description */
				tinrc.info_in_last_line = !tinrc.info_in_last_line;
				show_selection_page ();
				break;

			case iKeySelectMoveGrp:	/* reposition group within group list */
				if (no_write) {
					wait_message(0, _(txt_info_no_write));
					break;
				}
				if (!CURR_GROUP.subscribed) {
					wait_message(0, _(txt_info_not_subscribed));
					break;
				}

				n = selmenu.curr;
				selmenu.curr = reposition_group (&active[my_group[n]], n);
				HpGlitch(erase_group_arrow ());
				if (selmenu.curr < selmenu.first ||
					selmenu.curr >= selmenu.last ||
					selmenu.curr != n) {
					show_selection_page ();
				} else {
					i = selmenu.curr;
					selmenu.curr = n;
					erase_group_arrow ();
					selmenu.curr = i;
					clear_message ();
					draw_group_arrow ();
				}
				break;

			case iKeyOptionMenu:	/* option menu */
				(void) change_config_file(NULL);
				free_attributes_array ();
				read_attributes_file (global_attributes_file, TRUE);
				read_attributes_file (local_attributes_file, FALSE);
				show_selection_page ();
				break;

			case iKeySelectNextUnreadGrp:	/* goto next unread group */
				pos_next_unread_group (TRUE);
				break;

			case iKeyQuit:	/* quit */
				select_done();
				break;

			case iKeyQuitTin:	/* quit, no ask */
				select_quit();

			case iKeySelectQuitNoWrite:	/* quit, but don't save configuration */
				if (prompt_yn (cLINES, _(txt_quit_no_write), TRUE) == 1)
					tin_done (EXIT_SUCCESS);
				show_selection_page ();
				break;

			case iKeySelectToggleReadDisplay:
				/*
				 * If in tinrc.show_only_unread_groups mode toggle
				 * all subscribed to groups and only groups
				 * that contain unread articles
				 */
				tinrc.show_only_unread_groups = !tinrc.show_only_unread_groups;
				wait_message (0, _(txt_reading_groups), (tinrc.show_only_unread_groups) ? _("unread") : _("all"));

				toggle_my_groups (tinrc.show_only_unread_groups, "");
				set_groupname_len (FALSE);
				show_selection_page ();
				break;

			case iKeySelectBugReport:
				bug_report ();
				break;

			case iKeySelectSubscribe:	/* subscribe to current group */
				if (!selmenu.max)
					break;
				if (no_write) {
					wait_message(0, _(txt_info_no_write));
					break;
				}
				if (!CURR_GROUP.subscribed && !CURR_GROUP.bogus) {
					subscribe (&CURR_GROUP, SUBSCRIBED);
					show_selection_page();
					info_message (_(txt_subscribed_to), CURR_GROUP.name);
				}
				break;

			case iKeySelectSubscribePat:	/* subscribe to groups matching pattern */
				if (no_write) {
					wait_message(0, _(txt_info_no_write));
					break;
				}
				subscribe_pattern (_(txt_subscribe_pattern), _(txt_subscribing), _(txt_subscribed_num_groups), TRUE);
				break;

			case iKeySelectUnsubscribe:	/* unsubscribe to current group */
				if (!selmenu.max)
					break;
				if (no_write) {
					wait_message(0, _(txt_info_no_write));
					break;
				}

				if (CURR_GROUP.subscribed && !no_write) {
					mark_screen (SELECT_LEVEL, selmenu.curr - selmenu.first, 2, CURR_GROUP.newgroup ? "N" : "u");
					subscribe (&CURR_GROUP, UNSUBSCRIBED);
					info_message(_(txt_unsubscribed_to), CURR_GROUP.name);
					move_to_item (selmenu.curr + 1);
				} else if (CURR_GROUP.bogus && tinrc.strip_bogus == BOGUS_ASK) {
					/* Bogus groups aren't subscribed to avoid confusion */
					sprintf (buf, _(txt_remove_bogus), CURR_GROUP.name);
					vWriteNewsrc ();		/* save current newsrc */
					delete_group(CURR_GROUP.name);		/* remove bogus group */
					read_newsrc(newsrc, TRUE);		/* reload newsrc */
					toggle_my_groups (tinrc.show_only_unread_groups, "");		/* keep current display-state */
					show_selection_page();		/* reddraw screen */
					info_message (buf);
				}
				break;

			case iKeySelectUnsubscribePat:	/* unsubscribe to groups matching pattern */
				if (no_write) {
					wait_message(0, _(txt_info_no_write));
					break;
				}
				subscribe_pattern (_(txt_unsubscribe_pattern),
								_(txt_unsubscribing), _(txt_unsubscribed_num_groups), FALSE);
				break;

			case iKeyVersion:	/* show tin version */
				info_message (cvers);
				break;

			case iKeySelectPost:	/* post a basenote */
				if (!can_post) {
					info_message(_(txt_cannot_post));
					break;
				}
				if (!selmenu.max) {
					sprintf (buf, _(txt_post_newsgroups), tinrc.default_post_newsgroups);
					if (!(prompt_string_default (buf, tinrc.default_post_newsgroups, _(txt_no_newsgroups), HIST_POST_NEWSGROUPS)))
						break;

					if (find_group_index (buf) == -1) {
						error_message (_(txt_not_in_active_file), buf);
						break;
					}
				} else
					strcpy (buf, CURR_GROUP.name);
				if (post_article (buf))
					show_selection_page ();
				break;

			case iKeyPostponed:
			case iKeyPostponed2:	/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles(FALSE, FALSE))
						show_selection_page ();
				} else
					info_message(_(txt_cannot_post));
				break;

			case iKeyDisplayPostHist:	/* display messages posted by user */
				if (user_posted_messages ())
					show_selection_page ();
				break;

			case iKeySelectYankActive:	/* yank in/out rest of groups from active */
				yank_active_file();
				break;

			case iKeySelectSyncActive:  /* Re-read active file to see if any new news */
				sync_active_file ();
				break;

			case iKeySelectMarkGrpUnread:
			case iKeySelectMarkGrpUnread2:	/* mark group unread */
				if (!selmenu.max)
					break;
				grp_mark_unread (&CURR_GROUP);
				if (CURR_GROUP.newsrc.num_unread)
					strcpy (mesg, tin_ltoa(CURR_GROUP.newsrc.num_unread, 5));
				else
					strcpy (mesg, "     ");
				mark_screen (SELECT_LEVEL, selmenu.curr - selmenu.first, 9, mesg);
				break;

			default:
				info_message(_(txt_bad_command));
		}
	}
}


void
show_selection_page (
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

	MoveCursor (0, 0);		/* top left corner */
	CleartoEOLN ();

	if (read_news_via_nntp)
		sprintf (buf, "%s (%s  %d%s)", _(txt_group_selection), nntp_server, selmenu.max, (tinrc.show_only_unread_groups ? " R" : ""));
	else
		sprintf (buf, "%s (%d%s)", _(txt_group_selection), selmenu.max, (tinrc.show_only_unread_groups ? " R" : ""));

	show_title (buf);
	MoveCursor (1, 0);
	CleartoEOLN ();
	MoveCursor (INDEX_TOP, 0);

	if (selmenu.curr < 0)
		selmenu.curr = 0;

	set_first_screen_item ();

	blank_len = (cCOLS - (groupname_len + SELECT_MISC_COLS)) + (show_description ? 2 : 4);

	for (j = 0, i = selmenu.first; i < selmenu.last; i++, j++) {
#ifdef USE_CURSES
		char sptr[BUFSIZ];
#else
		char *sptr = screen[j].col;
#endif /* USE_CURSES */
		if (active[my_group[i]].inrange)
			strcpy (tmp, "    #");
		else if (active[my_group[i]].newsrc.num_unread) {
			strcpy (tmp, tin_ltoa(active[my_group[i]].newsrc.num_unread, 5));
		} else
			strcpy (tmp, "     ");

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
			subs = group_flag (active[n].moderated);
		else
			subs = ((active[n].newgroup) ? 'N' : 'u'); /* New (but unsubscribed) group or unsubscribed group */

		strncpy(active_name, active[n].name, groupname_len);
		active_name[groupname_len] = '\0';
		if (blank_len > 254)
			blank_len = 254;
		/* copy of active[n].description fix some malloc bugs kg */
		strncpy(group_descript, active[n].description ? active[n].description : " ", blank_len);
		group_descript[blank_len] = '\0';

		if (show_description) {
			if (active[n].description)
				sprintf (sptr, "  %c %s %s  %-*.*s  %-*.*s" cCRLF,
				         subs, tin_ltoa(i+1, 4), tmp,
				         groupname_len, groupname_len, active_name,
				         blank_len, blank_len, group_descript);
			else
				sprintf (sptr, "  %c %s %s  %-*.*s  " cCRLF,
				         subs, tin_ltoa(i+1, 4), tmp,
				         (groupname_len+blank_len),
				         (groupname_len+blank_len), active[n].name);
		} else {
			if (tinrc.draw_arrow)
				sprintf (sptr, "  %c %s %s  %-*.*s" cCRLF, subs, tin_ltoa(i+1, 4), tmp, groupname_len, groupname_len, active_name);
			else
				sprintf (sptr, "  %c %s %s  %-*.*s%*s" cCRLF, subs, tin_ltoa(i+1, 4), tmp, groupname_len, groupname_len, active_name, blank_len, " ");
		}
		if (tinrc.strip_blanks) {
			strip_line (sptr);
			strcat (sptr, cCRLF);
		}
		CleartoEOLN ();
		my_fputs (sptr, stdout);
	}

	CleartoEOS ();
	show_mini_help (SELECT_LEVEL);

	if (selmenu.max <= 0) {
		info_message (_(txt_no_groups));
		return;
	} else if (selmenu.last == selmenu.max)
		info_message (_(txt_end_of_groups));

	draw_group_arrow ();
}


static void
erase_group_arrow (
	void)
{
	if (selmenu.max)
		erase_arrow_mark (INDEX_TOP + selmenu.curr - selmenu.first);
}


void
draw_group_arrow (
	void)
{
	if (!selmenu.max)
		info_message (_(txt_no_groups));
	else {
		draw_arrow_mark (INDEX_TOP + selmenu.curr - selmenu.first);
		if (tinrc.info_in_last_line)
			info_message ("%s", CURR_GROUP.description ? CURR_GROUP.description : _(txt_no_description));
	}
}


static void
sync_active_file (
	void)
{
	force_reread_active_file = TRUE;
	resync_active_file ();
}


static void
yank_active_file (
	void)
{
	char *oldgroup = (char *) 0;
	int i;
	int oldmax = selmenu.max;
	static t_bool yank_in_active_file = TRUE;

	if (oldmax)
		oldgroup = strdup(CURR_GROUP.name);

	if (yank_in_active_file) {
		wait_message (0, _(txt_yanking_all_groups));

		/*
		 * Reset counter and load all the groups in active[] into my_group[]
		 */
		selmenu.max = 0;
		for (i = 0; i < num_active; i++)
			my_group[selmenu.max++] = i;

		/*
		 * If there are now more groups than before, we did yank something
		 */
		if (oldmax < selmenu.max) {
			if (oldmax)					/* Keep us positioned on the group we were before */
				selmenu.curr = my_group_add (oldgroup);
			set_groupname_len (yank_in_active_file);
			show_selection_page ();
			info_message (_(txt_added_groups), selmenu.max - oldmax, (selmenu.max - oldmax) == 1 ? "" : _(txt_plural));
		} else
			info_message (_(txt_no_groups_to_yank_in));
	} else {												/* Yank out */
		wait_message (0, _(txt_yanking_sub_groups));

		toggle_my_groups(tinrc.show_only_unread_groups, "");
		HpGlitch(erase_group_arrow ());

		selmenu.curr = -1;
		if (oldmax)					/* Keep us positioned on the group we were before */
			selmenu.curr = my_group_find (oldgroup);

		if (selmenu.curr == -1) {
			if (selmenu.max > 0)
				selmenu.curr = selmenu.max - 1;
			else
				selmenu.curr = 0;
		}

		set_groupname_len (yank_in_active_file);
		show_selection_page ();
	}

	yank_in_active_file = !yank_in_active_file;

	if (oldmax)
		FreeAndNull (oldgroup);
}


int
choose_new_group (
	void)
{
	char *p;
	int idx;

	sprintf (mesg, _(txt_newsgroup), tinrc.default_goto_group);

	if (!(prompt_string_default (mesg, tinrc.default_goto_group, "", HIST_GOTO_GROUP)))
		return -1;

	/*
	 * Skip leading whitespace, ignore blank strings
	 */
	for (p = tinrc.default_goto_group; *p && (*p == ' ' || *p == '\t'); p++)
		continue;

	if (*p == '\0')
		return -1;

	clear_message ();

	if ((idx = my_group_add (p)) == -1)
		info_message (_(txt_not_in_active_file), p);

	return idx;
}


/*
 * Return new value for selmenu.max, skipping any new newsgroups that have been
 * found
 */
int
skip_newgroups (
	void)
{
	int i = 0;

	if (selmenu.max) {
		while (i < selmenu.max && active[my_group[i]].newgroup)
			i++;
	}

	return(i);
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
add_my_group (
	const char *group,
	t_bool add)
{
	int i, j;

	if ((i = find_group_index (group)) < 0)
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
reposition_group (
	struct t_group *group,
	int default_num)
{
	char buf[LEN];
	char pos[LEN];
	int pos_num, newgroups;

	if (no_write)
		return (tinrc.default_move_group ? tinrc.default_move_group : default_num + 1);

	sprintf (buf, _(txt_newsgroup_position), group->name,
		(tinrc.default_move_group ? tinrc.default_move_group : default_num + 1));

	if (!prompt_string (buf, pos, HIST_MOVE_GROUP))
		return default_num;

	if (strlen (pos))
		pos_num = ((pos[0] == '$') ? selmenu.max: atoi (pos));
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
		return(default_num);
	}

	wait_message (0, _(txt_moving), group->name);

	/*
	 * seems to have the side effect of rearranging
	 * my_groups, so tinrc.show_only_unread_groups has to be updated
	 */
	tinrc.show_only_unread_groups = FALSE;

	/*
	 * New newgroups aren't in .newsrc so we need to offset to
	 * get the right position
	 */
	if (pos_group_in_newsrc (group, pos_num - newgroups)) {
		read_newsrc (newsrc, TRUE);
		tinrc.default_move_group = pos_num;
		return (pos_num-1);
	} else {
		tinrc.default_move_group = default_num + 1;
		return (default_num);
	}
}


static void
catchup_group (
	struct t_group *group,
	t_bool goto_next_unread_group)
{
	if (!tinrc.confirm_action || prompt_yn (cLINES, sized_message(_(txt_mark_group_read), group->name), TRUE) == 1) {
		grp_mark_read (group, NULL);
		mark_screen (SELECT_LEVEL, selmenu.curr - selmenu.first, 9, "     ");

		if (goto_next_unread_group)
			pos_next_unread_group (TRUE);
		else
			move_to_item (selmenu.curr + 1);
	}
}


/*
 * Set selmenu.curr to next group with unread arts
 * If the current group has unread arts, it will be counted. This is important !
 * If redraw is set, update the selection menu appropriately
 * Return FALSE if no groups left to read
 *        TRUE  at all other times
 */
static t_bool
pos_next_unread_group (
	t_bool redraw)
{
	int i;
	t_bool all_groups_read = TRUE;

	if (!selmenu.max)
		return FALSE;

	for (i = selmenu.curr; i < selmenu.max; i++) {
		if (UNREAD_GROUP (i)) {
			all_groups_read = FALSE;
			break;
		}
	}

	if (all_groups_read) {
		for (i = 0; i < selmenu.curr; i++) {
			if (UNREAD_GROUP (i)) {
				all_groups_read = FALSE;
				break;
			}
		}
	}

	if (all_groups_read) {
		info_message (_(txt_no_groups_to_read));
		return FALSE;
	}

	if (redraw)
		move_to_item (i);
	else
		selmenu.curr = i;

	return TRUE;
}


/*
 * This is the main loop that cycles through, reading groups.
 * We keep going until we return to the selection screen or exit tin
 */
static void
read_groups (
	void)
{
	t_bool done = FALSE;

	clear_message ();

	forever { /* if xmin > xmax the newsservers active is broken */
		if (done /* || CURR_GROUP.xmin > CURR_GROUP.xmax */)
			break;

		switch (group_page (&CURR_GROUP)) {

			case GRP_QUIT:
				select_quit();

			case GRP_NEXT:
				if (selmenu.curr + 1 < selmenu.max)
					selmenu.curr++;
				done = TRUE;
				break;

			case GRP_NEXTUNREAD:
				if (!pos_next_unread_group (FALSE))
					done = TRUE;
				break;

			case GRP_ENTER:			/* group_page() has set selmenu.curr, no need to change it */
				break;

			case GRP_RETURN:
			default:
				done = TRUE;
				break;
		}
	}

	if (!need_reread_active_file ())
		show_selection_page ();

	return;
}


/*
 * Calculate max length of groupname field for group selection level.
 * If all_groups is TRUE check all groups in active file otherwise
 * just subscribed to groups.
 */
void
set_groupname_len (
	t_bool all_groups)
{
	int len;
	register int i;

	groupname_len = 0;

	if (all_groups) {
		for (i = 0; i < num_active; i++) {
			if ((len = strlen (active[i].name)) > groupname_len)
				groupname_len = len;
		}
	} else {
		for (i = 0; i < selmenu.max; i++) {
			if ((len = strlen (active[my_group[i]].name)) > groupname_len)
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
 * and Bogus groups if tinrc.strip_bogus = BOGUS_ASK
 */
void
toggle_my_groups (
	t_bool only_unread_groups,
	const char *group)
{
	FILE *fp;
	char buf[NEWSRC_LINE];
	char old_curr_group[PATH_LEN];
	char *ptr;
	int old_curr_group_idx = 0;
	int group_num = (selmenu.curr == -1) ? 0 : selmenu.curr;
	int i;

	/*
	 * Save current or next group with unread arts for later use
	 */
	old_curr_group[0] = '\0';

	if (selmenu.max) {
		if (group[0] != '\0') {
			if ((i = my_group_find (group)) >= 0)
				old_curr_group_idx = i;
		} else if (group_num >= 0) {
			old_curr_group_idx = group_num;
		} else {
			old_curr_group_idx = 0;
		}
		if (only_unread_groups) {
			for (i = old_curr_group_idx; i < selmenu.max; i++) {
				if (active[my_group[i]].newsrc.num_unread || active[my_group[i]].newgroup) {
					my_strncpy (old_curr_group, active[my_group[i]].name, sizeof (old_curr_group));
					break;
				}
			}
		} else
			my_strncpy (old_curr_group, active[my_group[old_curr_group_idx]].name, sizeof (old_curr_group));
	}
	selmenu.max = skip_newgroups();			/* Reposition after any newgroups */

	if ((fp = fopen (newsrc, "r")) == (FILE *) 0)
		return;

	while (fgets (buf, (int) sizeof(buf), fp) != (char *) 0) {
		if ((ptr = strchr (buf, SUBSCRIBED)) != (char *) 0) {
			*ptr = '\0';

			if ((i = find_group_index (buf)) < 0)
				continue;

			if (only_unread_groups) {
				if (active[i].newsrc.num_unread || (active[i].bogus && tinrc.strip_bogus == BOGUS_ASK))
					my_group_add (buf);
			} else
				my_group_add (buf);
		}
	}
	fclose (fp);

	/*
	 * Try and reposition on same or next group before toggling
	 */
	if ((selmenu.curr = my_group_find(old_curr_group)) == -1)
		selmenu.curr = 0;

}


/*
 * Subscribe or unsubscribe from a list of groups. List can be full list as supported
 * by match_group_list()
 */
static void
subscribe_pattern (
	const char *prompt,
	const char *message,
	const char *result,
	t_bool state)
{
	char buf[LEN];
	int i, subscribe_num;

	if (!num_active || no_write)
		return;

	if (!prompt_string (prompt, buf, HIST_OTHER) || !*buf) {
		clear_message ();
		return;
	}

	wait_message (0, message);

	/* TODO - so why precisely do we need these 2 separate passes ? */
	for (subscribe_num = 0, i = 0; i < selmenu.max; i++) {
		if (match_group_list (active[my_group[i]].name, buf)) {
			if (active[my_group[i]].subscribed != (state != FALSE)) {
				spin_cursor ();
				subscribe (&active[my_group[i]], SUB_CHAR(state));
				subscribe_num++;
			}
		}
	}

	if (num_active > selmenu.max) {
		for (i = 0; i < num_active; i++) {
			if (match_group_list (active[i].name, buf)) {
				if (active[i].subscribed != (state != FALSE)) {
					spin_cursor ();
					/* If found and group is not subscribed add it to end of my_group[]. */
					subscribe (&active[i], SUB_CHAR(state));
					if (state) {
						my_group_add (active[i].name);
/* TODO grp_mark_unread() or something needs to do a GrpGetArtInfo to get initial count right */
						grp_mark_unread (&active[i]);
					}
					subscribe_num++;
				}
			}
		}
	}

	if (subscribe_num) {
		toggle_my_groups (tinrc.show_only_unread_groups, "");
		set_groupname_len (FALSE);
		show_selection_page ();
		info_message (result, subscribe_num);
	} else
		info_message (_(txt_no_match));
}


#if 0
/* TODO - work this back in again somewhere */
	if (CURR_GROUP.aliasedto) /* FIXME -> lang.c */
		info_message ("Please use %.100s instead", CURR_GROUP.aliasedto);
#endif /* 0 */


/*
 * Does NOT return
 */
static void
select_quit (
	void)
{
	write_config_file (local_config_file);
	tin_done (EXIT_SUCCESS);	/* Tin END */
}


static void
select_done (
	void)
{
	if (!tinrc.confirm_to_quit || prompt_yn (cLINES, _(txt_quit), TRUE) == 1)
		select_quit();
	if (!no_write && prompt_yn (cLINES, _(txt_save_config), TRUE) == 1) {
		write_config_file (local_config_file);
		vWriteNewsrc ();
	}
	show_selection_page ();
}


static void
select_read_group (
	void)
{
	struct t_group currgrp = CURR_GROUP;

	if (!selmenu.max) {
		info_message (_(txt_no_groups));
		return;
	}

	if (currgrp.bogus) {
		info_message (_(txt_not_exist));
		return;
	}

	if (currgrp.xmax > 0 && (currgrp.xmin <= currgrp.xmax))
		read_groups();
	else
		info_message (_(txt_no_arts));
}
