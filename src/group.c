/*
 *  Project   : tin - a Usenet reader
 *  Module    : group.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2002-11-11
 *  Notes     :
 *
 * Copyright (c) 1991-2002 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

#define INDEX2SNUM(i)	((i) % NOTESLINES)
#define INDEX2LNUM(i)	(INDEX_TOP + INDEX2SNUM(i))

/* 3+1+3; width(art_mark) + space + width(unread count) */
#define MAGIC	7

char *glob_group;

int max_from = 0;
int max_subj = 0;

static const char *spaces = "XXXX";
static int len_from;
static int len_subj;
static int thread_depth;			/* Starting depth in threads we enter */

/*
 * Local prototypes
 */
static int do_search(int type, t_bool forward);
static int enter_pager(int art, t_bool ignore_unavail);
static int enter_thread(int depth, t_pagerinfo *page);
static int group_catchup(int ch);
static int group_left(void);
static int group_right(void);
static int tab_pressed(void);
static int prompt_getart_limit(void);
static int recent_responses(int thread);
static void build_sline(int i);
static void draw_subject_arrow(void);
static void show_group_title(t_bool clear_title);
static void show_tagged_lines(void);
static void toggle_read_unread(t_bool force);
static void update_group_page(void);
static void build_multipart_header(char *dest, int maxlen, const char *src, int cmplen, int have, int total);

/*
 * grpmenu.curr is an index into base[] and so equates to the cursor location
 * (thread number) on group page
 * grpmenu.first, last are static here
 */
t_menu grpmenu = { 0, 0, 0, 0, show_group_page, draw_subject_arrow };

static void
show_tagged_lines(
	void)
{
	int i;

	for (i = grpmenu.first; i < grpmenu.last; ++i) {
		if ((i != grpmenu.curr) && line_is_tagged(base[i])) {
			build_sline(i);
			draw_line(i, MAGIC);
		}
	}
}


static int
group_left(
	void)
{
	if (tinrc.group_catchup_on_exit)
		return iKeyCatchupLeft;		/* ie, not via 'c' or 'C' */
	else
		return iKeyQuit;
}


static int
group_right(
	void)
{
	if (grpmenu.curr >= 0 && HAS_FOLLOWUPS(grpmenu.curr)) {
		if (tinrc.auto_list_thread)
			return iKeyGroupListThd;
		else {
			int n = next_unread((int) base[grpmenu.curr]);

			if (grpmenu.curr == which_thread(n) && n >= 0)
				return enter_pager(n, TRUE);
		}
	}
	return iKeyGroupReadBasenote;
}


/*
 * Return Codes:
 * GRP_EXIT			'Normal' return to selection menu
 * GRP_RETURN		We are en route from pager to the selection screen
 * GRP_QUIT			User has done a 'Q'
 * GRP_NEXT			User wants to move onto next group
 * GRP_NEXTUNREAD	User did a 'C'atchup
 * GRP_ENTER		'g' command has been used to set group to enter
 */
int
group_page(
	struct t_group *group)
{
	char buf[128];
	char key[MAXKEYLEN];
	char pat[128];
	int ch = 0;
	int i, n;
	int filter_state;
	int old_selected_arts;
	int old_top = 0;
	int old_group_top;
	int ret_code = 0;			/* Set to < 0 when it is time to leave this menu */
	long old_artnum = 0L;
	struct t_art_stat sbuf;
	t_bool flag;
	t_bool range_active = FALSE;		/* Set if a range is defined */
	t_bool xflag = FALSE;	/* 'X'-flag */

	/*
	 * Set the group attributes
	 */
	group->read_during_session = TRUE;

	proc_ch_default = POST_PROC_TYPE(group->attribute->post_proc_type);

	glob_group = group->name;			/* For global access to the current group */
	num_of_tagged_arts = 0;

	last_resp = -1;
	this_resp = -1;

	/*
	 * update index file. quit group level if user aborts indexing
	 */
	if (!index_group(group))
		return GRP_RETURN;

	/*
	 * Position 'grpmenu.curr' accordingly
	 */
	pos_first_unread_thread();

	set_subj_from_size(cCOLS);
	clear_note_area();

	if (group->attribute->auto_select) {
		error_message(_(txt_autoselecting_articles), printascii(key, map_to_local(iKeyGroupMarkUnselArtRead, &menukeymap.group_nav)));
		do_auto_select_arts();						/* 'X' command */
		xflag = TRUE;
	}

	show_group_page();

#	ifdef DEBUG_NEWSRC
	debug_print_bitmap(group, NULL);
#	endif /* DEBUG_NEWSRC */

	while (ret_code >= 0) {
		set_xclick_on();
		switch (ch = handle_keypad(group_left, group_right, &menukeymap.group_nav)) {
			case iKeyAbort:		/* Abort */
				break;

			case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				if (grpmenu.max)
					prompt_item_num(ch, _(txt_select_thread));
				break;

#	ifndef NO_SHELL_ESCAPE
			case iKeyShellEscape:
				do_shell_escape();
				break;
#	endif /* !NO_SHELL_ESCAPE */

			case iKeyFirstPage: /* show first page of threads */
				top_of_list();
				break;

			case iKeyLastPage:	/* show last page of threads */
				end_of_list();
				break;

			case iKeyLastViewed:	/* go to last viewed article */
				/*
				 * If the last art is no longer in a thread then we can't display it
				 */
				if (this_resp < 0 || (which_thread(this_resp) == -1))
					info_message(_(txt_no_last_message));
				else
					ret_code = enter_pager(this_resp, FALSE);
				break;

			case iKeyPipe:		/* pipe article/thread/tagged arts to command */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_PIPE, GROUP_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
				break;

			case iKeyGroupMail:	/* mail article to somebody */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_MAIL, GROUP_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
				break;

#ifndef DISABLE_PRINTING
			case iKeyPrint:	/* output art/thread/tagged arts to printer */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_PRINT, GROUP_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
				break;
#endif /* !DISABLE_PRINTING */

			case iKeyGroupRepost:	/* repost current article */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_REPOST, GROUP_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
				break;

			case iKeyGroupSave:	/* save articles with prompting */
				if (grpmenu.curr >= 0)
					feed_articles(FEED_SAVE, GROUP_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
				break;

			case iKeyGroupAutoSaveTagged:	/* Auto-save tagged articles without prompting */
				if (grpmenu.curr >= 0) {
					if (num_of_tagged_arts)
						feed_articles(FEED_AUTOSAVE_TAGGED, GROUP_LEVEL, &CURR_GROUP, (int) base[grpmenu.curr]);
					else
						info_message(_(txt_no_tagged_arts_to_save));
				}
				break;

			case iKeySetRange:	/* set range */
				if (set_range(GROUP_LEVEL, 1, grpmenu.max, grpmenu.curr + 1)) {
					range_active = TRUE;
					show_group_page();
				}
				break;

			case iKeySearchAuthF:	/* author forward/backward  search */
			case iKeySearchAuthB:
				if ((thread_depth = do_search(SEARCH_AUTH, (ch == iKeySearchAuthF))) != 0)
					ret_code = enter_thread(thread_depth, NULL);
				break;

			case iKeySearchSubjF:	/* subject forward/backward search */
			case iKeySearchSubjB:
				if ((thread_depth = do_search(SEARCH_SUBJ, (ch == iKeySearchSubjF))) != 0)
					ret_code = enter_thread(thread_depth, NULL);
				break;

			case iKeySearchBody:	/* search article body */
				if (grpmenu.curr >= 0) {
					if ((n = search_body((int) base[grpmenu.curr])) != -1)
						ret_code = enter_pager(n, FALSE);
				} else
					info_message(_(txt_no_arts));
				break;

			case iKeyGroupReadBasenote:
			case iKeyGroupReadBasenote2:	/* read current basenote */
				if (grpmenu.curr >= 0)
					ret_code = enter_pager((int) base[grpmenu.curr], FALSE /*TRUE*/);
				else
					info_message(_(txt_no_arts));
				break;

			case iKeyGroupNextUnreadArtOrGrp:	/* goto next unread article/group */
				ret_code = tab_pressed();
				break;

			case iKeyPageDown:		/* page down */
			case iKeyPageDown2:
			case iKeyPageDown3:
				page_down();
				break;

			case iKeyGroupAutoSel:		/* auto-select article menu */
			case iKeyGroupKill:		/* kill article menu */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}
				old_top = top_art;
				n = (int) base[grpmenu.curr];
				old_artnum = arts[n].artnum;
				if (filter_menu((ch == iKeyGroupKill) ? FILTER_KILL : FILTER_SELECT, group, &arts[n])) {
					if (filter_articles(group)) {
						make_threads(group, FALSE);
						grpmenu.curr = find_new_pos(old_top, old_artnum, grpmenu.curr);
					}
				}
				show_group_page();
				break;

			case iKeyGroupEditFilter:
				if (!invoke_editor(filter_file, FILTER_FILE_OFFSET))
					break;
				unfilter_articles();
				(void) read_filter_file(filter_file);
				if (filter_articles(group)) {
					make_threads(group, FALSE);
					grpmenu.curr = find_new_pos(old_top, old_artnum, grpmenu.curr);
				}
				show_group_page();
				break;

			case iKeyGroupQuickAutoSel:		/* quickly auto-select article */
			case iKeyGroupQuickKill:		/* quickly kill article */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}
				if ((!TINRC_CONFIRM_ACTION) || prompt_yn(cLINES, (ch == iKeyGroupQuickKill) ? _(txt_quick_filter_kill) : _(txt_quick_filter_select), TRUE) == 1) {
					old_top = top_art;
					n = (int) base[grpmenu.curr];
					old_artnum = arts[n].artnum;
					if (quick_filter((ch == iKeyGroupQuickKill) ? FILTER_KILL : FILTER_SELECT, group, &arts[n])) {
						info_message((ch == iKeyGroupQuickKill) ? _(txt_info_add_kill) : _(txt_info_add_select));
						if (filter_articles(group)) {
							make_threads(group, FALSE);
							grpmenu.curr = find_new_pos(old_top, old_artnum, grpmenu.curr);
						}
					}
					show_group_page();
				}
				break;

			case iKeyRedrawScr:	/* redraw screen */
				my_retouch();
				set_xclick_off();
				show_group_page();
				break;

			case iKeyDown:		/* line down */
			case iKeyDown2:
				move_down();
				break;

			case iKeyUp:		/* line up */
			case iKeyUp2:
				move_up();
				break;

			case iKeyPageUp:		/* page up */
			case iKeyPageUp2:
			case iKeyPageUp3:
				page_up();
				break;

			case iKeyCatchupLeft:
			case iKeyGroupCatchup:
			case iKeyGroupCatchupNextUnread:
				ret_code = group_catchup(ch);
				break;

			case iKeyGroupToggleSubjDisplay:	/* toggle display of subject & subj/author */
				toggle_subject_from();
				show_group_page();
				break;

			case iKeyGroupGoto:	/* choose a new group by name */
				old_group_top = selmenu.max;
				n = choose_new_group();
				if (n >= 0 && n != selmenu.curr) {
					/*
					 * if we added a group, set the length as appropriate
					 * for the group selection display
					 */
					if (old_group_top != selmenu.max)
						set_groupname_len(FALSE);
					selmenu.curr = n;
					ret_code = GRP_ENTER;
				}
				break;

			case iKeyHelp:					/* help */
				show_help_page(GROUP_LEVEL, _(txt_index_page_com));
				show_group_page();
				break;

			case iKeyToggleHelpDisplay:		/* toggle mini help menu */
				toggle_mini_help(GROUP_LEVEL);
				show_group_page();
				break;

			case iKeyToggleInverseVideo:	/* toggle inverse video */
				toggle_inverse_video();
				show_group_page();
				show_inverse_video_status();
				break;

#	ifdef HAVE_COLOR
			case iKeyToggleColor:
				if (toggle_color()) {
					show_group_page();
					show_color_status();
				}
				break;
#	endif /* HAVE_COLOR */

			case iKeyGroupMarkThdRead:	/* mark thread as read */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_next_unread_art));
					break;
				}
				old_selected_arts = num_of_selected_arts;
				/*
				 * If a range is active, use it.
				 */
				if (range_active) {
					/*
					 * We check all arts, in case the user did something clever like
					 * change the threading mode on us since the range was created
					 */
					for_each_art(n) {
						if (arts[n].inrange) {
							arts[n].inrange = FALSE;	/* Clear the range */
							art_mark_read(&CURR_GROUP, &arts[n]);
						}
					}
				} else
					thd_mark_read(&CURR_GROUP, base[grpmenu.curr]);

				/*
				 * If # of 'hot' articles changed, update the header
				 */
				if (num_of_selected_arts != old_selected_arts)
					show_group_title(TRUE);

				build_sline(grpmenu.curr);
				draw_line(grpmenu.curr, MAGIC);
				/*
				 * Move cursor to next unread
				 */
				if ((n = next_unread(next_response((int) base[grpmenu.curr]))) < 0) {
					draw_subject_arrow();
					info_message(_(txt_no_next_unread_art));
					break;
				}
				/*
				 * If range defined, we have to redraw whole page anyway.
				 */
				if (range_active) {
					range_active = FALSE;			/* Range has gone now */
					show_group_page();
				}
				if ((n = which_thread(n)) < 0) {
					error_message("Internal error: which_thread(%d) < 0", n);
					break;
				}
				move_to_item(n);
				break;

			case iKeyGroupListThd:				/* list articles within current thread */
				ret_code = enter_thread(0, NULL);	/* Enter thread at the top */
				break;

			case iKeyLookupMessage:
				if ((i = prompt_msgid()) != ART_UNAVAILABLE)
					ret_code = enter_pager(i, FALSE);
				break;

			case iKeyOptionMenu:	/* option menu */
				if (grpmenu.max > 0) {
					old_top = top_art;
					old_artnum = arts[(int) base[grpmenu.curr]].artnum;
				}
				n = tinrc.sort_article_type;
				filter_state = change_config_file(group);
				if (filter_state == NO_FILTERING && n != tinrc.sort_article_type)
					make_threads(&CURR_GROUP, TRUE);
				set_subj_from_size(cCOLS);
				grpmenu.curr = find_new_pos(old_top, old_artnum, grpmenu.curr);
				show_group_page();
				break;

			case iKeyGroupNextGroup:	/* goto next group */
				clear_message();
				if (selmenu.curr + 1 >= selmenu.max)
					info_message(_(txt_no_more_groups));
				else {
					if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(cLINES, txt_confirm_select_on_exit, FALSE) != 1)) {
						undo_auto_select_arts();
						xflag = FALSE;
					}
					selmenu.curr++;
					ret_code = GRP_NEXTUNREAD;
				}
				break;

			case iKeyGroupNextUnreadArt:	/* goto next unread article */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_next_unread_art));
					break;
				}
				if ((n = next_unread((int) base[grpmenu.curr])) == -1)
					info_message(_(txt_no_next_unread_art));
				else
					ret_code = enter_pager(n, FALSE);
				break;

			case iKeyGroupPrevUnreadArt:	/* go to previous unread article */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_prev_unread_art));
					break;
				}

				if ((n = prev_unread(prev_response((int) base[grpmenu.curr]))) == -1)
					info_message(_(txt_no_prev_unread_art));
				else
					ret_code = enter_pager(n, FALSE);
				break;

			case iKeyGroupPrevGroup:	/* previous group */
				clear_message();
				for (i = selmenu.curr - 1; i >= 0; i--) {
					if (UNREAD_GROUP(i))
						break;
				}
				if (i < 0)
					info_message(_(txt_no_prev_group));
				else {
					if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(cLINES, txt_confirm_select_on_exit, FALSE) != 1)) {
						undo_auto_select_arts();
						xflag = FALSE;
					}
					selmenu.curr = i;
					ret_code = GRP_NEXTUNREAD;
				}
				break;

			case iKeyQuit:	/* return to group selection page */
				if (num_of_tagged_arts && prompt_yn(cLINES, _(txt_quit_despite_tags), TRUE) != 1)
					break;
				if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(cLINES, txt_confirm_select_on_exit, FALSE) != 1)) {
					undo_auto_select_arts();
					xflag = FALSE;
				}
				ret_code = GRP_EXIT;
				break;

			case iKeyQuitTin:		/* quit */
				if (num_of_tagged_arts && prompt_yn(cLINES, _(txt_quit_despite_tags), TRUE) != 1)
					break;
				if (xflag && TINRC_CONFIRM_SELECT && (prompt_yn(cLINES, txt_confirm_select_on_exit, FALSE) != 1)) {
					undo_auto_select_arts();
					xflag = FALSE;
				}
				ret_code = GRP_QUIT;
				break;

			case iKeyGroupToggleReadUnread:
				toggle_read_unread(FALSE);
				show_group_page();
				break;

			case iKeyGroupToggleGetartLimit:
				tinrc.getart_limit = prompt_getart_limit();
				ret_code = GRP_NEXTUNREAD;
				break;

			case iKeyGroupBugReport:
				bug_report();
				break;

			case iKeyGroupTagParts: /* tag all in order */
				if (0 <= grpmenu.curr) {
					int new_tag_qty = tag_multipart(grpmenu.curr);
					/*
					 * on success, move the pointer to the next
					 * untagged article just for ease of use's sake
					 */
					if (new_tag_qty != 0) {
						int k = grpmenu.curr;
						update_group_page();
						do {
							k++;
							k %= grpmenu.max;
							if (arts[base[k]].tagged == 0) {
								move_to_item(k);
								break;
							}
						} while (k != grpmenu.curr);
					}
				}
				break;

			case iKeyGroupTag:	/* tag/untag threads for mailing/piping/printing/saving */
				if (grpmenu.curr >= 0) {
					int ii;
					t_bool tagged = TRUE;

					n = (int) base[grpmenu.curr];

					/*
					 * This loop looks for any article in the thread that
					 * isn't already tagged.
					 */
					for (ii = n; ii != -1 && tagged; ii = arts[ii].thread) {
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
						info_message(_(txt_untagged_thread));
						for (ii = n; ii != -1; ii = arts[ii].thread) {
							if (arts[ii].tagged != 0) {
								tagged = TRUE;
								remove_tag(ii);
							}
						}
					} else {
						info_message(_(txt_tagged_thread));
						for (ii = n; ii != -1; ii = arts[ii].thread) {
							if (arts[ii].tagged == 0)
								arts[ii].tagged = ++num_of_tagged_arts;
						}
					}
					build_sline(grpmenu.curr);
					draw_line(grpmenu.curr, MAGIC);
					if (tagged)
						show_tagged_lines();
					if (grpmenu.curr + 1 < grpmenu.max) {
						move_down();
						break;
					}
					draw_subject_arrow();
				}
				break;

			case iKeyGroupToggleThreading:	/* Cycle through the threading types */
				CURR_GROUP.attribute->thread_arts = (CURR_GROUP.attribute->thread_arts + 1) % (THREAD_MAX + 1);
				if (grpmenu.curr >= 0) {
					i = base[grpmenu.curr];								/* Save a copy of current thread */
					make_threads(&CURR_GROUP, TRUE);
					find_base(&CURR_GROUP);
					if ((grpmenu.curr = which_thread(i)) < 0)			/* Restore current position in group */
						grpmenu.curr = 0;
				}
				show_group_page();
				break;

			case iKeyGroupUntag:	/* untag all articles */
				if (grpmenu.curr >= 0) {
					if (untag_all_articles())
						update_group_page();
				}
				break;

			case iKeyVersion:
				info_message(cvers);
				break;

			case iKeyPost:	/* post an article */
				if (post_article(group->name))
					show_group_page();
				break;

			case iKeyPostponed:
			case iKeyPostponed2:	/* post postponed article */
				if (can_post) {
					if (pickup_postponed_articles(FALSE, FALSE))
						show_group_page();
				} else
					info_message(_(txt_cannot_post));
				break;

			case iKeyDisplayPostHist:	/* display messages posted by user */
				if (user_posted_messages())
					show_group_page();
				break;

			case iKeyGroupMarkArtUnread:	/* mark base article of thread unread */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}
				if (range_active) {
					int ii;

					/*
					 * We are tied to following base[] here, not arts[], as we operate on
					 * the base articles by definition.
					 */
					for (ii = 0; ii < grpmenu.max; ++ii) {
						if (arts[base[ii]].inrange) {
							arts[base[ii]].inrange = FALSE;
							art_mark_will_return(&CURR_GROUP, &arts[base[ii]]);
							for_each_art_in_thread(i, ii)
								arts[i].inrange = FALSE;
						}
					}
					range_active = FALSE;
					show_group_page();
					strcpy(buf, "Base article range"); /* FIXME: -> lang.c */
				} else {
					art_mark_will_return(&CURR_GROUP, &arts[base[grpmenu.curr]]);
					strcpy(buf, "Base article"); /* FIXME: -> lang.c */
				}

				show_group_title(TRUE);
				build_sline(grpmenu.curr);
				draw_line(grpmenu.curr, MAGIC);
				draw_subject_arrow();
				info_message(_(txt_marked_as_unread), buf);
				break;

			case iKeyGroupMarkThdUnread:	/* mark whole thread as unread */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}

				/*
				 * We process all articles in case the threading changed since
				 * the range was created
				 */
				if (range_active) {
					int ii;

					for_each_art(ii) {
						if (arts[ii].inrange) {
							arts[ii].inrange = FALSE;
							art_mark_will_return(&CURR_GROUP, &arts[ii]);
						}
					}
					range_active = FALSE;
					show_group_page();
					strcpy(buf, "Thread range"); /* FIXME: -> lang.c */
				} else {
					thd_mark_unread(&CURR_GROUP, base[grpmenu.curr]);
					strcpy(buf, "Thread");
				}

				show_group_title(TRUE);
				build_sline(grpmenu.curr);
				draw_line(grpmenu.curr, MAGIC);
				draw_subject_arrow();
				info_message(_(txt_marked_as_unread), buf);
				break;

			case iKeyGroupSelThd:	/* mark thread as selected */
			case iKeyGroupToggleThdSel:	/* toggle thread */
				if (grpmenu.curr < 0) {
					info_message(_(txt_no_arts));
					break;
				}

				flag = TRUE;
				if (ch == iKeyGroupToggleThdSel) {
					stat_thread(grpmenu.curr, &sbuf);
					if (sbuf.selected_unread == sbuf.unread)
						flag = FALSE;
				}
				n = 0;
				for_each_art_in_thread(i, grpmenu.curr) {
					arts[i].selected = flag;
					++n;
				}
				assert(n > 0);
				build_sline(grpmenu.curr);
				draw_line(grpmenu.curr, MAGIC);

				info_message(flag
					      ? _(txt_thread_marked_as_selected)
					      : _(txt_thread_marked_as_deselected));

				if (grpmenu.curr + 1 < grpmenu.max) {
					move_down();
					break;
				}
				draw_subject_arrow();
				break;

			case iKeyGroupReverseSel:	/* reverse selections */
				for_each_art(i)
					arts[i].selected = bool_not(arts[i].selected);
				update_group_page();
				break;

			case iKeyGroupUndoSel:	/* undo selections */
				undo_selections();
				xflag = FALSE;
				update_group_page();
				break;

			case iKeyGroupSelPattern:	/* select matching patterns */
				sprintf(mesg, _(txt_select_pattern), tinrc.default_select_pattern);
				if (!prompt_string(mesg, buf, HIST_SELECT_PATTERN))
					break;

				if (buf[0] == '\0') {				/* TODO -> prompt_string_default ?? */
					if (tinrc.default_select_pattern[0] == '\0') {
						info_message("No previous expression"); /* FIXME: -> lang.c */
						break;
					}
					sprintf(pat, REGEX_FMT, tinrc.default_select_pattern);
				} else if (STRCMPEQ(buf, "*")) {	/* all */
					strcpy(pat, buf);
					strcpy(tinrc.default_select_pattern, pat);
				} else {
					strcpy(tinrc.default_select_pattern, buf);
					sprintf(pat, REGEX_FMT, tinrc.default_select_pattern);
				}

				flag = FALSE;
				for (n = 0; n < grpmenu.max; n++) {
					if (!REGEX_MATCH(arts[base[n]].subject, pat, TRUE))
						continue;

					for_each_art_in_thread(i, n)
						arts[i].selected = TRUE;

					build_sline(n);
					flag = TRUE;
				}
				if (flag)
					update_group_page();

				break;

			case iKeyGroupSelThdIfUnreadSelected:	/* select all unread arts in thread hot if 1 is hot */
				for (n = 0; n < grpmenu.max; n++) {
					stat_thread(n, &sbuf);
					if (!sbuf.selected_unread || sbuf.selected_unread == sbuf.unread)
						continue;

					for_each_art_in_thread(i, n)
						arts[i].selected = TRUE;
				}
				/* no screen update needed */
				break;

			case iKeyGroupMarkUnselArtRead:	/* mark read all unselected arts */
				if (!xflag) {
					do_auto_select_arts();
					xflag = TRUE;
				} else {
					undo_auto_select_arts();
					xflag = FALSE;
				}
				break;

			case iKeyGroupDoAutoSel:		/* perform auto-selection on group */
				undo_auto_select_arts();
				xflag = FALSE;
				break;

			case iKeyToggleInfoLastLine:
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				show_group_page();
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, map_to_local(iKeyHelp, &menukeymap.group_nav)));
				break;
		} /* switch(ch) */
	} /* ret_code >= 0 */

	set_xclick_off();

	clear_note_area();
	vGrpDelMailArts(&CURR_GROUP);

	art_close(&pgart);				/* Close any open art */

	return ret_code;
}


void
show_group_page(
	void)
{
	int i;

	signal_context = cGroup;
	currmenu = &grpmenu;

	MoveCursor(0, 0);
	CleartoEOLN();

	show_group_title(FALSE);

	MoveCursor(1, 0);
	CleartoEOLN();
	MoveCursor(INDEX_TOP, 0);

	set_first_screen_item();

	if (tinrc.draw_arrow)
		CleartoEOS();

	for (i = grpmenu.first; i < grpmenu.last; ++i) {
		build_sline(i);
		draw_line(i, 0);
	}

	CleartoEOS();
	show_mini_help(GROUP_LEVEL);

	if (grpmenu.max <= 0) {
		info_message(_(txt_no_arts));
		return;
	} else if (grpmenu.last == grpmenu.max)
		info_message(_(txt_end_of_arts));

	draw_subject_arrow();

}


static void
update_group_page(
	void)
{
	register int i;

	for (i = grpmenu.first; i < grpmenu.last; ++i) {
		build_sline(i);
		draw_line(i, MAGIC);
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
		struct t_art_stat statbuf;

		stat_thread(grpmenu.curr, &statbuf);
		info_message("%s", arts[(statbuf.unread ? next_unread(base[grpmenu.curr]) : base[grpmenu.curr])].subject);
	} else {
		if (grpmenu.last == grpmenu.max)
			info_message(_(txt_end_of_arts));
	}
}


void
clear_note_area(
	void)
{
	MoveCursor(INDEX_TOP, 0);
	CleartoEOS();
}


/*
 * If in show_only_unread mode or there are unread articles we know this
 * thread will exist after toggle. Otherwise we find the next closest to
 * return to. 'force' can be set to force tin to show all messages
 */
static void
toggle_read_unread(
	t_bool force)
{
	int n, i = -1;

	if (force)
		CURR_GROUP.attribute->show_only_unread = TRUE;	/* Yes - really, we change it in a bit */

	wait_message(0, _(txt_reading_arts),
		(CURR_GROUP.attribute->show_only_unread) ? _(txt_all) : _(txt_unread));

	if (grpmenu.curr >= 0) {
		if (CURR_GROUP.attribute->show_only_unread || new_responses(grpmenu.curr))
			i = base[grpmenu.curr];
		else if ((n = prev_unread((int) base[grpmenu.curr])) >= 0)
			i = n;
		else if ((n = next_unread((int) base[grpmenu.curr])) >= 0)
			i = n;
	}

	CURR_GROUP.attribute->show_only_unread = bool_not(CURR_GROUP.attribute->show_only_unread);

	find_base(&CURR_GROUP);
	if (i >= 0 && (n = which_thread(i)) >= 0)
		grpmenu.curr = n;
	else if (grpmenu.max > 0)
		grpmenu.curr = grpmenu.max - 1;
}


/*
 * Find new index position after a kill or unkill. Because kill can work on
 * author it is impossible to know which, if any, articles will be left
 * afterwards. So we make a "best attempt" to find a new index point.
 */
int
find_new_pos(
	int old_top,
	long old_artnum,
	int cur_pos)
{
	int i, pos;

	if (top_art == old_top)
		return cur_pos;

	for_each_art(i) {
		if (arts[i].artnum == old_artnum) {
			pos = which_thread(arts[i].artnum);
			if (pos >= 0)
				return pos;
		}
	}
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

	if (tinrc.pos_first_unread) {
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
	int level,	/* Always SELECT_LEVEL - TODO move to select.c or use this everywhere */
	int screen_row,
	int screen_col,
	const char *value)
{
	if (tinrc.draw_arrow) {
		MoveCursor(INDEX_TOP + screen_row, screen_col);
		my_fputs(value, stdout);
		stow_cursor();
		my_flush();
	} else {
#ifdef USE_CURSES
		int y, x;
		getyx(stdscr, y, x);
		mvaddstr(INDEX_TOP + screen_row, screen_col, value);
		MoveCursor(y, x);
#else
		int i;
		for (i = 0; value[i] != '\0'; i++)
			screen[screen_row].col[screen_col + i] = value[i];
#endif /* USE_CURSES */
		if (level == SELECT_LEVEL)
			draw_group_arrow();
		else
			draw_subject_arrow();
	}
}


void
set_subj_from_size(
	int num_cols)
{
	int show_author;

	/*
	 * This function is called early during startup when we only have
	 * very limited information loaded.
	 */
	show_author = ((selmenu.max && CURR_GROUP.attribute) ? CURR_GROUP.attribute->show_author : tinrc.show_author);
	max_subj = ((show_author == SHOW_FROM_BOTH) ? ((num_cols / 2) - 4): ((num_cols / 2) + 3));
	max_from = (num_cols - max_subj) - 17;

	if (show_author != SHOW_FROM_BOTH) {
		if (max_from > 25) {
			max_subj += max_from - 25;
			max_from = 25;
		}
	}

	if (show_author != SHOW_FROM_NONE) {
		len_from = max_from - BLANK_GROUP_COLS;
		len_subj = max_subj;
		spaces = "  ";
	} else {
		len_from = 0;
		len_subj = (max_subj + max_from + 2) - BLANK_GROUP_COLS;
		spaces = "";
	}

	if (!tinrc.show_lines)
		len_subj += 5;

	if (!tinrc.show_score)
		len_subj += 7;
}


void
toggle_subject_from(
	void)
{
	if (++CURR_GROUP.attribute->show_author > SHOW_FROM_BOTH)
		CURR_GROUP.attribute->show_author = SHOW_FROM_NONE;

	set_subj_from_size(cCOLS);
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
		strncpy(dest, src, maxlen);
	else {
		strncpy(dest, src, cmplen);
		ss = dest + cmplen;
		snprintf(ss, maxlen - cmplen, "(%s/%d)", mark, total);
	}
}


/*
 * Build subject line given an index into base[].
 *
 * WARNING: the routine is tightly coupled with draw_line() in the sense
 * that draw_line() expects build_sline() to place the article mark
 * (ART_MARK_READ, ART_MARK_SELECTED, etc) at MARK_OFFSET in the
 * screen[].col.
 * So, if you change the format used in this routine, be sure to check
 * that the value of MARK_OFFSET (tin.h) is still correct.
 * Yes, this is somewhat kludgy.
 */
static void
build_sline(
	int i)
{
	char from[HEADER_LEN];
	char new_resps[8];
	char art_cnt[10];
	char arts_sub[255];
	int respnum;
	int n, j;
	struct t_art_stat sbuf;
#ifdef USE_CURSES
	char buffer[BUFSIZ];	/* FIXME: allocate? */
#else
	char *buffer;
#endif /* USE_CURSES */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t format[32];
	wchar_t wbuffer[LEN];
	wchar_t tmp_subj[256], tmp_subj2[256];
	wchar_t tmp_from[HEADER_LEN], tmp_from2[HEADER_LEN];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	from[0] = '\0';
	respnum = (int) base[i];

	stat_thread(i, &sbuf);

	/*
	 * n is number of articles in this thread
	 */
	n = ((CURR_GROUP.attribute->show_only_unread) ? (sbuf.unread + sbuf.seen) : sbuf.total);
	/*
	 * if you like to see the number of responses excluding the first
	 *	art in thread - add the following:
	 *	n--;
	 */

	if ((j = line_is_tagged(respnum)))
		strcpy(new_resps, tin_ltoa(j, 3));
	else
		sprintf(new_resps, "  %c", sbuf.art_mark);

	/*
	 * Find index of first unread in this thread
	 */
	j = (sbuf.unread) ? next_unread(respnum) : respnum;

	if (tinrc.show_lines) {
		if (n > 1) { /* change this to (n > 0) if you do a n-- above */
			if (arts[j].line_count != -1) {
				char tmp_buffer[4];
				strcpy(tmp_buffer, tin_ltoa(n, 3));
				sprintf(art_cnt, "%s %s ", tmp_buffer, tin_ltoa(arts[j].line_count, 4));
			} else
				sprintf(art_cnt, "%s    ? ", tin_ltoa(n, 3));
		} else {
			if (arts[j].line_count != -1)
				sprintf(art_cnt, "    %s ", tin_ltoa(arts[j].line_count, 4));
			else
				strcpy(art_cnt, "       ? ");
		}
	} else {
		if (n > 1) /* change this to (n > 0) if you do a n-- above */
			sprintf(art_cnt, "%s ", tin_ltoa(n, 3));
		else
			strcpy(art_cnt, "    ");
	}

	if (CURR_GROUP.attribute->show_author != SHOW_FROM_NONE)
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		/* ignore len_from for now, we truncate it later */
		get_author(FALSE, &arts[j], from, sizeof(from) - 1);
#else
		get_author(FALSE, &arts[j], from, len_from);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (sbuf.multipart_have > 1) /* We have a multipart msg so lets built our new header info. */
		build_multipart_header(arts_sub, len_subj, arts[j].subject, sbuf.multipart_compare_len, sbuf.multipart_have, sbuf.multipart_total);
	else
		strncpy(arts_sub, arts[j].subject, sizeof(arts_sub) - 1);

#ifndef USE_CURSES
	buffer = screen[INDEX2SNUM(i)].col;
#endif /* !USE_CURSES */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	/* wcswidth() in wcspart() depends that all characters are printable */
	convert_to_printable(arts_sub);
	convert_to_printable(from);

	mbstowcs(tmp_subj2, arts_sub, ARRAY_SIZE(tmp_subj2) - 1);
	mbstowcs(tmp_from2, from, ARRAY_SIZE(tmp_from2) - 1);

	/* format subject and from  */
	wcspart(tmp_subj, tmp_subj2, len_subj - 12, ARRAY_SIZE(tmp_subj));
	wcspart(tmp_from, tmp_from2, len_from, ARRAY_SIZE(tmp_from));

	if (tinrc.show_score) {
		mbstowcs(format, "  %s %s %s%6d %-ls%s%-ls", ARRAY_SIZE(format) - 1);
		swprintf(wbuffer, ARRAY_SIZE(wbuffer) - 1, format,
			 tin_ltoa(i + 1, 4), new_resps, art_cnt, sbuf.score, tmp_subj,
			 spaces, tmp_from);
	} else {
		mbstowcs(format, "  %s %s %s %-ls%s%-ls", ARRAY_SIZE(format) - 1);
		swprintf(wbuffer, ARRAY_SIZE(wbuffer) - 1, format,
			 tin_ltoa(i + 1, 4), new_resps, art_cnt, tmp_subj,
			 spaces, tmp_from);
	}

	wcstombs(buffer, wbuffer, BUFSIZ);
#else
	arts_sub[len_subj - 12 + 1] = '\0';

	if (tinrc.show_score)
		sprintf(buffer, "  %s %s %s%6d %-*.*s%s%-*.*s",
			 tin_ltoa(i + 1, 4), new_resps, art_cnt, sbuf.score,
			 len_subj - 12, len_subj - 12, arts_sub,
			 spaces, len_from, len_from, from);
	else
		sprintf(buffer, "  %s %s %s%-*.*s%s%-*.*s",
			 tin_ltoa(i + 1, 4), new_resps, art_cnt,
			 len_subj - 12, len_subj - 12, arts_sub,
			 spaces, len_from, len_from, from);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	/*
	 * protect display from non-displayable characters (e.g., form-feed)
	 * and write line.
	 */
	WriteLine(INDEX2LNUM(i), convert_to_printable(buffer));
}


static void
show_group_title(
	t_bool clear_title)
{
	char buf[LEN];
	int i, art_cnt = 0, recent_art_cnt = 0;
	struct t_group currgrp;

	currgrp = CURR_GROUP;
	if (currgrp.attribute->show_only_unread) {
		for (i = 0; i < grpmenu.max; i++) {
			art_cnt += new_responses(i);
			recent_art_cnt += recent_responses(i);
		}
	} else {
		for_each_art(i) {
			if (!IGNORE_ART(i)) {
				++art_cnt;
				if (tinrc.recent_time && ((time((time_t) 0) - arts[i].date) < (tinrc.recent_time * DAY)))
					recent_art_cnt++;
			}
		}
	}

	/*
	 * TODO: clean up
	 *       really count read_selected into num_of_selected_arts? (kill_level > 1)
	 */
	if (tinrc.getart_limit && tinrc.recent_time)
		snprintf(buf, sizeof(buf) - 1, "%s (%d%c %d/%d%c %d%c %d%c %d%c) %c",
			currgrp.name, grpmenu.max,
			*txt_thread[currgrp.attribute->thread_arts],
			tinrc.getart_limit,
			art_cnt, (currgrp.attribute->show_only_unread ? tinrc.art_marked_unread : tinrc.art_marked_read),
			num_of_selected_arts, tinrc.art_marked_selected,
			recent_art_cnt, tinrc.art_marked_recent,
			num_of_killed_arts, tinrc.art_marked_killed,
			group_flag(currgrp.moderated));
	else if (tinrc.getart_limit)
		snprintf(buf, sizeof(buf) - 1, "%s (%d%c %d/%d%c %d%c %d%c) %c",
			currgrp.name, grpmenu.max,
			*txt_thread[currgrp.attribute->thread_arts],
			tinrc.getart_limit,
			art_cnt, (currgrp.attribute->show_only_unread ? tinrc.art_marked_unread : tinrc.art_marked_read),
			num_of_selected_arts, tinrc.art_marked_selected,
			num_of_killed_arts, tinrc.art_marked_killed,
			group_flag(currgrp.moderated));
	else if (tinrc.recent_time)
		snprintf(buf, sizeof(buf) - 1, "%s (%d%c %d%c %d%c %d%c %d%c) %c",
			currgrp.name, grpmenu.max,
			*txt_thread[currgrp.attribute->thread_arts],
			art_cnt, (currgrp.attribute->show_only_unread ? tinrc.art_marked_unread : tinrc.art_marked_read),
			num_of_selected_arts, tinrc.art_marked_selected,
			recent_art_cnt, tinrc.art_marked_recent,
			num_of_killed_arts, tinrc.art_marked_killed,
			group_flag(currgrp.moderated));
	else
		snprintf(buf, sizeof(buf) - 1, "%s (%d%c %d%c %d%c %d%c) %c",
			currgrp.name, grpmenu.max,
			*txt_thread[currgrp.attribute->thread_arts],
			art_cnt, (currgrp.attribute->show_only_unread ? tinrc.art_marked_unread : tinrc.art_marked_read),
			num_of_selected_arts, tinrc.art_marked_selected,
			num_of_killed_arts, tinrc.art_marked_killed,
			group_flag(currgrp.moderated));

	if (clear_title) {
		MoveCursor(0, 0);
		CleartoEOLN();
	}
	show_title(buf);
}


/*
 * Search for type SUBJ/AUTH in direction (TRUE = forwards)
 * Return 0 if all is done, or a >0 thread_depth to enter the thread menu
 */
static int
do_search(
	int type,
	t_bool forward)
{
	int start, n;

	if (grpmenu.curr < 0)
		return 0;

	/*
	 * Not intuitive to search current thread in fwd search
	 */
	start = (forward && grpmenu.curr < grpmenu.max - 1) ? prev_response((int) base[grpmenu.curr + 1]) : (int) base[grpmenu.curr];

	if ((n = search(type, start, forward)) != -1) {
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
	page.ignore_unavail = ignore_unavail;

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
		switch (i = thread_page(&CURR_GROUP, (int) base[grpmenu.curr], depth, page)) {
			case GRP_QUIT:						/* 'Q'uit */
			case GRP_RETURN:					/* Back to selection screen */
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
					if ((n = which_thread(n)) >= 0) {
						grpmenu.curr = n;
						depth = 0;
						break;		/* Drop into next thread with unread */
					}
				}
				/* No more unread threads in this group */
				/* FALLTHROUGH */

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
	int ch)
{
	char buf[LEN];
	int pyn = 1;

	if (num_of_tagged_arts && prompt_yn(cLINES, _(txt_catchup_despite_tags), TRUE) != 1)
		return 0;

	/* FIXME: -> lang.c */
	snprintf(buf, sizeof(buf) - 1, _(txt_mark_arts_read), (ch == iKeyGroupCatchupNextUnread) ? _(" and enter next unread group") : "");

	if (!CURR_GROUP.newsrc.num_unread || (!TINRC_CONFIRM_ACTION) || (pyn = prompt_yn(cLINES, buf, TRUE)) == 1)
		grp_mark_read(&CURR_GROUP, arts);

	switch (ch) {
		case iKeyGroupCatchup:				/* 'c' */
			if (pyn == 1)
				return GRP_NEXT;
			break;

		case iKeyGroupCatchupNextUnread:	/* 'C' */
			if (pyn == 1)
				return GRP_NEXTUNREAD;
			break;

		case iKeyCatchupLeft:				/* <- group catchup on exit */
			switch (pyn) {
				case -1:					/* ESCAPE - do nothing */
					break;

				case 1:						/* We caught up - advance group */
					return GRP_NEXT;
					/* NOTREACHED */
					break;

				default:					/* Just leave the group */
					return GRP_EXIT;
					/* NOTREACHED */
					break;
			}
			/* FALLTHROUGH */
		default:							/* Should not be here */
			break;
	}
	return 0;								/* Stay in this menu by default */
}


/*
 * Return the number of recent articles there are within a thread
 */
static int
recent_responses(
	int thread)
{
	int i;
	int sum = 0;

	for_each_art_in_thread(i, thread) {
		if (tinrc.recent_time && ((time((time_t) 0) - arts[i].date) < (tinrc.recent_time * DAY)) && arts[i].status != ART_READ)
			sum++;
	}

	return sum;
}


static int
prompt_getart_limit(
	void)
{
	char *p;
	int num = 0;

	clear_message();
	if ((p = tin_getline(_(txt_enter_getart_limit), 2, 0, 0, FALSE, HIST_OTHER)) != NULL)
		num = atoi(p);

	clear_message();
	return num;
}
