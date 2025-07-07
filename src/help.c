/*
 *  Project   : tin - a Usenet reader
 *  Module    : help.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2025-06-14
 *  Notes     :
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>
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


typedef struct thp {
	constext *helptext;
	t_function func;
} t_help_page;

/*
 * local prototypes
 */
static void make_help_page(FILE *fp, const t_help_page *helppage, const struct keylist keys);


static t_help_page attachment_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_attachment_first, GLOBAL_FIRST_PAGE },
	{ txt_help_attachment_last, GLOBAL_LAST_PAGE },
	{ txt_help_attachment_goto, NOT_ASSIGNED },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_attachment_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_attachment_ops, NOT_ASSIGNED },
	{ txt_help_attachment_select, ATTACHMENT_SELECT },
#ifndef DONT_HAVE_PIPING
	{ txt_help_attachment_pipe, ATTACHMENT_PIPE },
	{ txt_help_attachment_pipe_raw, GLOBAL_PIPE },
#endif /* !DONT_HAVE_PIPING */
	{ txt_help_attachment_save, ATTACHMENT_SAVE },
	{ txt_help_attachment_tag, ATTACHMENT_TAG },
	{ txt_help_attachment_tag_pattern, ATTACHMENT_TAG_PATTERN },
	{ txt_help_attachment_toggle_tagged, ATTACHMENT_TOGGLE_TAGGED },
	{ txt_help_attachment_untag, ATTACHMENT_UNTAG },
	{ "", NOT_ASSIGNED },
	{ txt_help_attachment_search_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_attachment_search_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page attrib_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_attrib_first_opt, GLOBAL_FIRST_PAGE },
	{ txt_help_attrib_last_opt, GLOBAL_LAST_PAGE },
	{ txt_help_attrib_goto_opt, NOT_ASSIGNED },
	{ "", NOT_ASSIGNED },
	{ txt_help_attrib_search_opt_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_attrib_search_opt_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_select_search_group_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_attrib_ops, NOT_ASSIGNED },
	{ txt_help_attrib_reset_attrib, CONFIG_RESET_ATTRIB },
	{ txt_help_attrib_select, CONFIG_SELECT },
	{ txt_help_attrib_toggle_attrib, CONFIG_TOGGLE_ATTRIB },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_select_quit_no_write, CONFIG_NO_SAVE },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page config_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_config_first_opt, GLOBAL_FIRST_PAGE },
	{ txt_help_config_last_opt, GLOBAL_LAST_PAGE },
	{ txt_help_config_goto_opt, NOT_ASSIGNED },
	{ "", NOT_ASSIGNED },
	{ txt_help_config_search_opt_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_config_search_opt_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_select_search_group_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_config_ops, NOT_ASSIGNED },
	{ txt_help_config_select, CONFIG_SELECT },
	{ txt_help_config_toggle_attrib, CONFIG_TOGGLE_ATTRIB },
	{ txt_help_config_scope_menu, CONFIG_SCOPE_MENU },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_select_quit_no_write, CONFIG_NO_SAVE },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page scope_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_scope_first_scope, GLOBAL_FIRST_PAGE },
	{ txt_help_scope_last_scope, GLOBAL_LAST_PAGE },
	{ txt_help_scope_goto_scope, NOT_ASSIGNED },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_scope_ops, NOT_ASSIGNED },
	{ txt_help_scope_add, SCOPE_ADD },
	{ txt_help_scope_move, SCOPE_MOVE },
	{ txt_help_scope_rename, SCOPE_RENAME },
	{ txt_help_scope_del, SCOPE_DELETE },
	{ txt_help_scope_select, SCOPE_SELECT },
	{ "", NOT_ASSIGNED },
	{ txt_help_scope_edit_attrib_file, SCOPE_EDIT_ATTRIBUTES_FILE },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ NULL, NOT_ASSIGNED }
};

static t_help_page select_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_select_first_group, GLOBAL_FIRST_PAGE },
	{ txt_help_select_last_group, GLOBAL_LAST_PAGE },
	{ txt_help_select_group_by_num, NOT_ASSIGNED },
	{ txt_help_select_goto_group, SELECT_GOTO },
	{ txt_help_select_next_unread_group, SELECT_NEXT_UNREAD_GROUP },
#ifdef NNTP_ABLE
	{ txt_help_select_lookup_group, GLOBAL_LOOKUP_MESSAGEID },
	{ txt_help_select_lookup_group_comment, NOT_ASSIGNED },
#endif /* NNTP_ABLE */
	{ "", NOT_ASSIGNED },
	{ txt_help_select_search_group_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_select_search_group_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_select_search_group_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_select_toggle_read_groups, SELECT_TOGGLE_READ_DISPLAY },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_select_toggle_descriptions, SELECT_TOGGLE_DESCRIPTIONS },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ "", NOT_ASSIGNED },
	{ txt_help_select_sort_active, SELECT_SORT_ACTIVE },
	{ txt_help_select_yank_active, SELECT_YANK_ACTIVE },
	{ txt_help_select_sync_with_active, SELECT_SYNC_WITH_ACTIVE },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
	{ txt_help_select_read_group, SELECT_ENTER_GROUP },
	{ txt_help_select_next_unread_group, SELECT_ENTER_NEXT_UNREAD_GROUP },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
#endif /* NO_POSTING */
	{ "", NOT_ASSIGNED },
	{ txt_help_select_group_range, GLOBAL_SET_RANGE },
	{ "", NOT_ASSIGNED },
	{ txt_help_select_catchup, CATCHUP },
	{ txt_help_select_catchup_next_unread, CATCHUP_NEXT_UNREAD },
	{ txt_help_select_mark_group_unread, SELECT_MARK_GROUP_UNREAD },
	{ txt_help_select_subscribe, SELECT_SUBSCRIBE },
	{ txt_help_select_unsubscribe, SELECT_UNSUBSCRIBE },
	{ txt_help_select_subscribe_pattern, SELECT_SUBSCRIBE_PATTERN },
	{ txt_help_select_unsubscribe_pattern, SELECT_UNSUBSCRIBE_PATTERN },
	{ txt_help_select_move_group, SELECT_MOVE_GROUP },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER},
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_global_quit_tin, GLOBAL_QUIT_TIN },
	{ txt_help_select_quit_no_write, SELECT_QUIT_NO_WRITE },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_option_menu, GLOBAL_OPTION_MENU },
	{ txt_help_global_esc, GLOBAL_ABORT },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, GLOBAL_DISPLAY_POST_HISTORY },
	{ txt_help_select_reset_newsrc, SELECT_RESET_NEWSRC },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ txt_help_global_connection_info, GLOBAL_CONNECTION_INFO },
	{ txt_help_bug_report, GLOBAL_BUGREPORT },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page group_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_group_first_thread, GLOBAL_FIRST_PAGE },
	{ txt_help_group_last_thread, GLOBAL_LAST_PAGE },
	{ txt_help_group_thread_by_num, NOT_ASSIGNED },
	{ txt_help_select_goto_group, GROUP_GOTO },
	{ txt_help_group_next, GROUP_NEXT_GROUP },
	{ txt_help_group_prev, GROUP_PREVIOUS_GROUP },
	{ txt_help_article_next_unread, GROUP_NEXT_UNREAD_ARTICLE },
	{ txt_help_article_prev_unread, GROUP_PREVIOUS_UNREAD_ARTICLE },
	{ txt_help_global_last_art, GLOBAL_LAST_VIEWED },
	{ txt_help_global_lookup_art, GLOBAL_LOOKUP_MESSAGEID },
	{ txt_help_group_list_thread, GROUP_LIST_THREAD },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_search_subj_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_global_search_subj_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_auth_forwards, GLOBAL_SEARCH_AUTHOR_FORWARD },
	{ txt_help_global_search_auth_backwards, GLOBAL_SEARCH_AUTHOR_BACKWARD },
	{ txt_help_global_search_body, GLOBAL_SEARCH_BODY },
	{ txt_help_global_search_body_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_group_toggle_read_articles, GROUP_TOGGLE_READ_UNREAD },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_select_toggle_descriptions, SELECT_TOGGLE_DESCRIPTIONS },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_toggle_subj_display, GROUP_TOGGLE_SUBJECT_DISPLAY},
	{ txt_help_group_toggle_threading, GROUP_TOGGLE_THREADING },
	{ txt_help_group_mark_unsel_art_read, GROUP_MARK_UNSELECTED_ARTICLES_READ },
	{ txt_help_group_toggle_getart_limit, GROUP_TOGGLE_GET_ARTICLES_LIMIT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
	{ txt_help_thread_read_article, GROUP_READ_BASENOTE },
	{ txt_help_article_next_unread, GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
	{ txt_help_article_repost, GROUP_REPOST },
	{ txt_help_article_cancel, GROUP_CANCEL },
#endif /* NO_POSTING */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_article_range, GLOBAL_SET_RANGE },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_mail, GROUP_MAIL },
	{ txt_help_global_save, GROUP_SAVE },
	{ txt_help_global_auto_save, GROUP_AUTOSAVE },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, GLOBAL_PIPE },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, GLOBAL_PRINT },
#endif /* !DISABLE_PRINTING */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_tag, GROUP_TAG },
	{ txt_help_tag_parts, GROUP_TAG_PARTS },
	{ txt_help_group_untag_thread, GROUP_UNTAG },
	{ "", NOT_ASSIGNED },
	{ txt_help_group_mark_thread_read, GROUP_MARK_THREAD_READ },
	{ txt_help_group_catchup, CATCHUP },
	{ txt_help_group_catchup_next, CATCHUP_NEXT_UNREAD },
	{ txt_help_group_mark_article_unread, MARK_ARTICLE_UNREAD },
	{ txt_help_group_mark_thread_unread, MARK_THREAD_UNREAD },
	{ txt_help_mark_feed_read, MARK_FEED_READ },
	{ txt_help_mark_feed_unread, MARK_FEED_UNREAD },
	{ "", NOT_ASSIGNED },
	{ txt_help_group_select_all, GROUP_DO_AUTOSELECT },
	{ txt_help_group_select_thread, GROUP_SELECT_THREAD },
	{ txt_help_group_select_thread_pattern, GROUP_SELECT_PATTERN },
	{ txt_help_group_select_thread_if_unread_selected, GROUP_SELECT_THREAD_IF_UNREAD_SELECTED },
	{ txt_help_group_toggle_thread_selection, GROUP_TOGGLE_SELECT_THREAD },
	{ txt_help_group_reverse_thread_selection, GROUP_REVERSE_SELECTIONS },
	{ txt_help_group_undo_thread_selection, GROUP_UNDO_SELECTIONS },
	{ "", NOT_ASSIGNED },
	{ txt_help_article_autoselect, GLOBAL_MENU_FILTER_SELECT },
	{ txt_help_article_autokill, GLOBAL_MENU_FILTER_KILL },
	{ txt_help_article_quick_select, GLOBAL_QUICK_FILTER_SELECT },
	{ txt_help_article_quick_kill, GLOBAL_QUICK_FILTER_KILL },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER},
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_global_previous_menu, GLOBAL_QUIT },
	{ txt_help_global_quit_tin, GLOBAL_QUIT_TIN },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_option_menu, GLOBAL_OPTION_MENU },
	{ txt_help_global_esc, GLOBAL_ABORT },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, GLOBAL_DISPLAY_POST_HISTORY },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ txt_help_global_connection_info, GLOBAL_CONNECTION_INFO },
	{ txt_help_bug_report, GLOBAL_BUGREPORT },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page thread_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_thread_first_article, GLOBAL_FIRST_PAGE },
	{ txt_help_thread_last_article, GLOBAL_LAST_PAGE },
	{ txt_help_thread_article_by_num, NOT_ASSIGNED },
	{ txt_help_global_last_art, GLOBAL_LAST_VIEWED },
	{ txt_help_global_lookup_art, GLOBAL_LOOKUP_MESSAGEID },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_search_subj_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_global_search_subj_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_auth_forwards, GLOBAL_SEARCH_AUTHOR_FORWARD },
	{ txt_help_global_search_auth_backwards, GLOBAL_SEARCH_AUTHOR_BACKWARD },
	{ txt_help_global_search_body, GLOBAL_SEARCH_BODY },
	{ txt_help_global_search_body_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_global_toggle_subj_display, THREAD_TOGGLE_SUBJECT_DISPLAY},
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
	{ txt_help_thread_read_article, THREAD_READ_ARTICLE },
	{ txt_help_article_next_unread, THREAD_READ_NEXT_ARTICLE_OR_THREAD },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_article_followup, THREAD_FOLLOWUP_QUOTE },
	{ txt_help_article_followup_no_quote, THREAD_FOLLOWUP },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
	{ txt_help_article_cancel, THREAD_CANCEL },
#endif /* NO_POSTING */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_article_range, GLOBAL_SET_RANGE },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_mail, THREAD_MAIL },
	{ txt_help_global_save, THREAD_SAVE },
	{ txt_help_global_auto_save, THREAD_AUTOSAVE },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, GLOBAL_PIPE },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, GLOBAL_PRINT },
#endif /* !DISABLE_PRINTING */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_tag, THREAD_TAG },
	{ txt_help_tag_parts, THREAD_TAG_PARTS },
	{ txt_help_group_untag_thread, THREAD_UNTAG },
	{ "", NOT_ASSIGNED },
	{ txt_help_thread_mark_article_read, THREAD_MARK_ARTICLE_READ },
	{ txt_help_thread_catchup, CATCHUP },
	{ txt_help_thread_catchup_next_unread, CATCHUP_NEXT_UNREAD },
	{ txt_help_thread_mark_article_unread, MARK_ARTICLE_UNREAD },
	{ txt_help_thread_mark_thread_unread, MARK_THREAD_UNREAD },
	{ txt_help_mark_feed_read, MARK_FEED_READ },
	{ txt_help_mark_feed_unread, MARK_FEED_UNREAD },
	{ "", NOT_ASSIGNED },
	{ txt_help_group_select_thread, THREAD_SELECT_ARTICLE },
	{ txt_help_group_toggle_thread_selection, THREAD_TOGGLE_ARTICLE_SELECTION },
	{ txt_help_group_reverse_thread_selection, THREAD_REVERSE_SELECTIONS },
	{ txt_help_group_undo_thread_selection, THREAD_UNDO_SELECTIONS },
	{ "", NOT_ASSIGNED },
	{ txt_help_article_autoselect, GLOBAL_MENU_FILTER_SELECT },
	{ txt_help_article_autokill, GLOBAL_MENU_FILTER_KILL },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_global_previous_menu, GLOBAL_QUIT },
	{ txt_help_global_quit_tin, GLOBAL_QUIT_TIN },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_option_menu, GLOBAL_OPTION_MENU },
	{ txt_help_global_esc, GLOBAL_ABORT },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, GLOBAL_DISPLAY_POST_HISTORY },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ txt_help_global_connection_info, GLOBAL_CONNECTION_INFO },
	{ txt_help_bug_report, GLOBAL_BUGREPORT },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page page_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_article_first_page, GLOBAL_FIRST_PAGE },
	{ txt_help_article_last_page, GLOBAL_LAST_PAGE },
	{ "", NOT_ASSIGNED },
	{ txt_help_article_by_num, NOT_ASSIGNED },
	{ txt_help_article_next_thread, PAGE_NEXT_THREAD },
	{ txt_help_article_next_unread, PAGE_NEXT_UNREAD },
	{ txt_help_article_next, PAGE_NEXT_ARTICLE },
	{ txt_help_article_next_unread, PAGE_NEXT_UNREAD_ARTICLE },
	{ txt_help_article_prev, PAGE_PREVIOUS_ARTICLE },
	{ txt_help_article_prev_unread, PAGE_PREVIOUS_UNREAD_ARTICLE },
	{ txt_help_article_first_in_thread, PAGE_TOP_THREAD },
	{ txt_help_article_last_in_thread, PAGE_BOTTOM_THREAD },
	{ txt_help_global_last_art, GLOBAL_LAST_VIEWED },
	{ txt_help_group_list_thread, PAGE_LIST_THREAD },
	{ txt_help_article_parent, PAGE_GOTO_PARENT },
	{ txt_help_global_lookup_art, GLOBAL_LOOKUP_MESSAGEID },
	{ txt_help_article_quit_to_select_level, PAGE_GROUP_SELECT },
	{ txt_help_article_skip_quote, PAGE_SKIP_INCLUDED_TEXT },
	{ "", NOT_ASSIGNED },
	{ txt_help_article_search_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_article_search_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_auth_forwards, GLOBAL_SEARCH_AUTHOR_FORWARD },
	{ txt_help_global_search_auth_backwards, GLOBAL_SEARCH_AUTHOR_BACKWARD },
	{ txt_help_global_search_body, GLOBAL_SEARCH_BODY },
	{ txt_help_global_search_body_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_article_toggle_rot13, PAGE_TOGGLE_ROT13 },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
	{ txt_help_article_show_raw, PAGE_TOGGLE_RAW },
	{ txt_help_article_toggle_headers, PAGE_TOGGLE_HEADERS },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ txt_help_article_toggle_highlight, PAGE_TOGGLE_HIGHLIGHTING },
	{ txt_help_article_toggle_inline_data, PAGE_TOGGLE_INLINE_DATA },
	{ txt_help_article_toggle_verbatim, PAGE_TOGGLE_VERBATIM },
	{ txt_help_article_toggle_tex2iso, PAGE_TOGGLE_TEX2ISO },
	{ txt_help_article_toggle_tabwidth, PAGE_TOGGLE_TABS },
	{ txt_help_article_toggle_formfeed, PAGE_REVEAL },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
	{ txt_help_article_followup, PAGE_FOLLOWUP_QUOTE },
	{ txt_help_article_followup_no_quote, PAGE_FOLLOWUP },
	{ txt_help_article_followup_with_header, PAGE_FOLLOWUP_QUOTE_HEADERS },
	{ txt_help_article_repost, PAGE_REPOST },
	{ txt_help_article_cancel, PAGE_CANCEL },
#endif /* NO_POSTING */
	{ txt_help_article_reply, PAGE_REPLY_QUOTE },
	{ txt_help_article_reply_no_quote, PAGE_REPLY },
	{ txt_help_article_reply_with_header, PAGE_REPLY_QUOTE_HEADERS },
	{ txt_help_article_edit, PAGE_EDIT_ARTICLE },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_mail, PAGE_MAIL },
	{ txt_help_global_save, PAGE_SAVE },
	{ txt_help_global_auto_save, PAGE_AUTOSAVE },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, GLOBAL_PIPE },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, GLOBAL_PRINT },
#endif /* !DISABLE_PRINTING */
	{ txt_help_article_view_attachments, PAGE_VIEW_ATTACHMENTS },
	{ "", NOT_ASSIGNED },
	{ txt_help_global_tag, PAGE_TAG },
	{ "", NOT_ASSIGNED },
	{ txt_help_article_mark_thread_read, PAGE_MARK_THREAD_READ },
	{ txt_help_thread_catchup, CATCHUP },
	{ txt_help_thread_catchup_next_unread, CATCHUP_NEXT_UNREAD },
	{ txt_help_group_mark_article_unread, MARK_ARTICLE_UNREAD },
	{ txt_help_thread_mark_thread_unread, MARK_THREAD_UNREAD },
	{ "", NOT_ASSIGNED },
	{ txt_help_article_autoselect, GLOBAL_MENU_FILTER_SELECT },
	{ txt_help_article_autokill, GLOBAL_MENU_FILTER_KILL },
	{ txt_help_article_quick_select, GLOBAL_QUICK_FILTER_SELECT },
	{ txt_help_article_quick_kill, GLOBAL_QUICK_FILTER_KILL },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_article_browse_urls, PAGE_VIEW_URL },
	{ txt_help_global_previous_menu, GLOBAL_QUIT },
	{ txt_help_global_quit_tin, GLOBAL_QUIT_TIN },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_option_menu, GLOBAL_OPTION_MENU },
	{ txt_help_global_esc, GLOBAL_ABORT },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, GLOBAL_DISPLAY_POST_HISTORY },
#ifdef HAVE_PGP_GPG
	{ "", NOT_ASSIGNED },
	{ txt_help_article_pgp, PAGE_PGP_CHECK_ARTICLE },
#endif /* HAVE_PGP_GPG */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_connection_info, GLOBAL_CONNECTION_INFO },
	{ txt_help_article_info, PAGE_ARTICLE_INFO },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page post_hist_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_thread_first_article, GLOBAL_FIRST_PAGE },
	{ txt_help_thread_last_article, GLOBAL_LAST_PAGE },
	{ txt_help_thread_article_by_num, NOT_ASSIGNED },
	{ "", NOT_ASSIGNED },
	{ txt_help_post_hist_search_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_post_hist_search_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_post_hist_ops, NOT_ASSIGNED },
	{ txt_help_post_hist_select, POSTED_SELECT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_post_hist_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_esc, GLOBAL_ABORT },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page url_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ "", NOT_ASSIGNED },
	{ txt_help_url_first_url, GLOBAL_FIRST_PAGE },
	{ txt_help_url_last_url, GLOBAL_LAST_PAGE },
	{ txt_help_url_goto_url, NOT_ASSIGNED },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_url_ops, NOT_ASSIGNED },
	{ txt_help_url_select, URL_SELECT },
	{ "", NOT_ASSIGNED },
	{ txt_help_url_search_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_url_search_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_url_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ "\n", NOT_ASSIGNED },
	{ txt_help_title_misc, NOT_ASSIGNED },
	{ txt_help_select_quit, GLOBAL_QUIT },
	{ txt_help_global_help, GLOBAL_HELP },
	{ txt_help_global_toggle_mini_help, GLOBAL_TOGGLE_HELP_DISPLAY },
	{ txt_help_global_esc, GLOBAL_ABORT },
	{ txt_help_global_redraw_screen, GLOBAL_REDRAW_SCREEN },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, GLOBAL_SHELL_ESCAPE },
#endif /* !NO_SHELL_ESCAPE */
	{ "", NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};


static void
make_help_page(
	FILE *fp,
	const t_help_page *helppage,
	const struct keylist keys)
{
	char *buf;
	char key[MAXKEYLEN];
	size_t length; /* length is only needed to pass it to expand_ctrl_chars() */
	size_t i;

	if (!helppage)
		return;

	buf = my_malloc(LEN);

	while (helppage->helptext) {
		if (helppage->func == NOT_ASSIGNED) {
			buf[0]='\0';

			/* no translation of empty strings */
			if (*helppage->helptext) {
				/*
				 * as expand_ctrl_chars() may has shrunk buf
				 * make sure buf is large enough to contain the helpline
				 */
				buf = my_realloc(buf, LEN);
				if (helppage->helptext[0] == '\n' && helppage->helptext[1] == '\0')
					strncpy(buf, helppage->helptext, LEN);
				else
					strncpy(buf, _(helppage->helptext), LEN);
				buf[LEN - 1] = '\0';
			}
			expand_ctrl_chars(&buf, &length, 8);
			fprintf(fp, "%s\n", BlankIfNull(buf));
		} else {
			for (i = 0; i < keys.used; i++) {
				if (keys.list[i].function == helppage->func && keys.list[i].key) {
					buf = my_realloc(buf, LEN);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					snprintf(buf, LEN, "%s\t  %s", printascii(key, (wint_t) keys.list[i].key), _(helppage->helptext));
#else
					snprintf(buf, LEN, "%s\t  %s", printascii(key, keys.list[i].key), _(helppage->helptext));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					expand_ctrl_chars(&buf, &length, 8);
					fprintf(fp, "%s\n", buf);
				}
			}
		}
		++helppage;
	}
	free(buf);
}


void
show_help_page(
	const int level,
	const char *title)
{
	FILE *fp;

	if (!(fp = my_tmpfile()))
		return;

	switch (level) {
		case ATTACHMENT_LEVEL:
			make_help_page(fp, attachment_help_page, attachment_keys);
			break;

		case ATTRIB_LEVEL:
			make_help_page(fp, attrib_help_page, option_menu_keys);
			break;

		case CONFIG_LEVEL:
			make_help_page(fp, config_help_page, option_menu_keys);
			break;

		case SCOPE_LEVEL:
			make_help_page(fp, scope_help_page, scope_keys);
			break;

		case SELECT_LEVEL:
			make_help_page(fp, select_help_page, select_keys);
			break;

		case GROUP_LEVEL:
			make_help_page(fp, group_help_page, group_keys);
			break;

		case THREAD_LEVEL:
			make_help_page(fp, thread_help_page, thread_keys);
			break;

		case PAGE_LEVEL:
			make_help_page(fp, page_help_page, page_keys);
			break;

		case POSTED_LEVEL:
			make_help_page(fp, post_hist_help_page, post_hist_keys);
			break;

		case URL_LEVEL:
			make_help_page(fp, url_help_page, url_keys);
			break;

		case INFO_PAGER:
		default: /* should not happen */
			error_message(2, _(txt_error_unknown_dlevel));
			fclose(fp);
			return;
	}

	info_pager(fp, title, TRUE);
	fclose(fp);
	info_pager(NULL, NULL, TRUE); /* free mem */
}


void
show_mini_help(
	int level)
{
	char buf[LEN];
	char key[20][MAXKEYLEN];
	int line;
	size_t bufs;

	if (!tinrc.beginner_level)
		return;

	line = NOTESLINES + MINI_HELP_LINES - 2;
	bufs = sizeof(buf) - 1;

#ifdef HAVE_COLOR
	fcol(tinrc.col_minihelp);
#endif /* HAVE_COLOR */

	switch (level) {
		case ATTACHMENT_LEVEL:
			snprintf(buf, bufs, _(txt_mini_attachment_1),
				PrintFuncKey(key[0], GLOBAL_LINE_DOWN, attachment_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_UP, attachment_keys),
				PrintFuncKey(key[2], GLOBAL_HELP, attachment_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, attachment_keys));
			center_line(line, FALSE, buf);
#ifndef DONT_HAVE_PIPING
			snprintf(buf, bufs, _(txt_mini_attachment_2),
				PrintFuncKey(key[0], ATTACHMENT_SELECT, attachment_keys),
				PrintFuncKey(key[5], ATTACHMENT_PIPE, attachment_keys),
				PrintFuncKey(key[6], GLOBAL_PIPE, attachment_keys),
				PrintFuncKey(key[1], ATTACHMENT_SAVE, attachment_keys),
				PrintFuncKey(key[2], ATTACHMENT_TAG, attachment_keys),
				PrintFuncKey(key[3], ATTACHMENT_TAG_PATTERN, attachment_keys),
				PrintFuncKey(key[4], ATTACHMENT_UNTAG, attachment_keys));
#else
			snprintf(buf, bufs, _(txt_mini_attachment_2),
				PrintFuncKey(key[0], ATTACHMENT_SELECT, attachment_keys),
				PrintFuncKey(key[1], ATTACHMENT_SAVE, attachment_keys),
				PrintFuncKey(key[2], ATTACHMENT_TAG, attachment_keys),
				PrintFuncKey(key[3], ATTACHMENT_TAG_PATTERN, attachment_keys),
				PrintFuncKey(key[4], ATTACHMENT_UNTAG, attachment_keys));
#endif /* !DONT_HAVE_PIPING */
			center_line(line + 1, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_attachment_3),
				PrintFuncKey(key[1], ATTACHMENT_TOGGLE_TAGGED, attachment_keys),
				PrintFuncKey(key[2], GLOBAL_SEARCH_SUBJECT_FORWARD, attachment_keys),
				PrintFuncKey(key[3], GLOBAL_SEARCH_SUBJECT_BACKWARD, attachment_keys),
				PrintFuncKey(key[4], GLOBAL_SEARCH_REPEAT, attachment_keys));
			center_line(line + 2, FALSE, buf);
			break;

		case SCOPE_LEVEL:
			snprintf(buf, bufs, _(txt_mini_scope_1),
				PrintFuncKey(key[0], SCOPE_ADD, scope_keys),
				PrintFuncKey(key[1], SCOPE_MOVE, scope_keys),
				PrintFuncKey(key[2], SCOPE_RENAME, scope_keys),
				PrintFuncKey(key[3], SCOPE_DELETE, scope_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_scope_2),
				PrintFuncKey(key[0], GLOBAL_LINE_DOWN, scope_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_UP, scope_keys),
				PrintFuncKey(key[2], GLOBAL_HELP, scope_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, scope_keys));
			center_line(line + 1, FALSE, buf);
			break;

		case SELECT_LEVEL:
			snprintf(buf, bufs, _(txt_mini_select_1),
				PrintFuncKey(key[0], SELECT_ENTER_NEXT_UNREAD_GROUP, select_keys),
				PrintFuncKey(key[1], SELECT_GOTO, select_keys),
				PrintFuncKey(key[2], GLOBAL_SEARCH_SUBJECT_FORWARD, select_keys),
				PrintFuncKey(key[3], CATCHUP, select_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_select_2),
				PrintFuncKey(key[0], GLOBAL_LINE_DOWN, select_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_UP, select_keys),
				PrintFuncKey(key[2], GLOBAL_HELP, select_keys),
				PrintFuncKey(key[3], SELECT_MOVE_GROUP, select_keys),
				PrintFuncKey(key[4], GLOBAL_QUIT, select_keys),
				PrintFuncKey(key[5], SELECT_TOGGLE_READ_DISPLAY, select_keys));
			center_line(line + 1, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_select_3),
				PrintFuncKey(key[0], SELECT_SUBSCRIBE, select_keys),
				PrintFuncKey(key[1], SELECT_SUBSCRIBE_PATTERN, select_keys),
				PrintFuncKey(key[2], SELECT_UNSUBSCRIBE, select_keys),
				PrintFuncKey(key[3], SELECT_UNSUBSCRIBE_PATTERN, select_keys),
				PrintFuncKey(key[4], SELECT_YANK_ACTIVE, select_keys));
			center_line(line + 2, FALSE, buf);
			break;

		case GROUP_LEVEL:
			snprintf(buf, bufs, _(txt_mini_group_1),
				PrintFuncKey(key[0], GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP, group_keys),
				PrintFuncKey(key[1], GLOBAL_SEARCH_SUBJECT_FORWARD, group_keys),
				PrintFuncKey(key[2], GLOBAL_MENU_FILTER_KILL, group_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_group_2),
				PrintFuncKey(key[0], GLOBAL_SEARCH_AUTHOR_FORWARD, group_keys),
				PrintFuncKey(key[1], CATCHUP, group_keys),
				PrintFuncKey(key[2], GLOBAL_LINE_DOWN, group_keys),
				PrintFuncKey(key[3], GLOBAL_LINE_UP, group_keys),
				PrintFuncKey(key[4], GROUP_MARK_THREAD_READ, group_keys),
				PrintFuncKey(key[5], GROUP_LIST_THREAD, group_keys));
			center_line(line + 1, FALSE, buf);

#if defined(DONT_HAVE_PIPING) && defined(DISABLE_PRINTING)
			snprintf(buf, bufs, _(txt_mini_group_3),
				PrintFuncKey(key[3], GLOBAL_QUIT, group_keys),
				PrintFuncKey(key[4], GROUP_TOGGLE_READ_UNREAD, group_keys),
				PrintFuncKey(key[5], GROUP_SAVE, group_keys),
				PrintFuncKey(key[6], GROUP_TAG, group_keys),
				PrintFuncKey(key[7], GLOBAL_POST, group_keys));
#else
#	ifdef DONT_HAVE_PIPING
			snprintf(buf, bufs, _(txt_mini_group_3),
				PrintFuncKey(key[1], GROUP_MAIL, group_keys),
				PrintFuncKey(key[2], GLOBAL_PRINT, group_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, group_keys),
				PrintFuncKey(key[4], GROUP_TOGGLE_READ_UNREAD, group_keys),
				PrintFuncKey(key[5], GROUP_SAVE, group_keys),
				PrintFuncKey(key[6], GROUP_TAG, group_keys),
				PrintFuncKey(key[7], GLOBAL_POST, group_keys));
#	else
#		ifdef DISABLE_PRINTING
			snprintf(buf, bufs, _(txt_mini_group_3),
				PrintFuncKey(key[0], GLOBAL_PIPE, group_keys),
				PrintFuncKey(key[1], GROUP_MAIL, group_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, group_keys),
				PrintFuncKey(key[4], GROUP_TOGGLE_READ_UNREAD, group_keys),
				PrintFuncKey(key[5], GROUP_SAVE, group_keys),
				PrintFuncKey(key[6], GROUP_TAG, group_keys),
				PrintFuncKey(key[7], GLOBAL_POST, group_keys));
#		else
			snprintf(buf, bufs, _(txt_mini_group_3),
				PrintFuncKey(key[0], GLOBAL_PIPE, group_keys),
				PrintFuncKey(key[1], GROUP_MAIL, group_keys),
				PrintFuncKey(key[2], GLOBAL_PRINT, group_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, group_keys),
				PrintFuncKey(key[4], GROUP_TOGGLE_READ_UNREAD, group_keys),
				PrintFuncKey(key[5], GROUP_SAVE, group_keys),
				PrintFuncKey(key[6], GROUP_TAG, group_keys),
				PrintFuncKey(key[7], GLOBAL_POST, group_keys));
#		endif /* DISABLE_PRINTING */
#	endif /* DONT_HAVE_PIPING */
#endif /* DONT_HAVE_PIPING && DISABLE_PRINTING */

			center_line(line + 2, FALSE, buf);
			break;

		case THREAD_LEVEL:
			snprintf(buf, bufs, _(txt_mini_thread_1),
				PrintFuncKey(key[0], THREAD_READ_NEXT_ARTICLE_OR_THREAD, thread_keys),
				PrintFuncKey(key[1], CATCHUP, thread_keys),
				PrintFuncKey(key[2], THREAD_TOGGLE_SUBJECT_DISPLAY, thread_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_thread_2),
				PrintFuncKey(key[0], GLOBAL_HELP, thread_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_DOWN, thread_keys),
				PrintFuncKey(key[2], GLOBAL_LINE_UP, thread_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, thread_keys),
				PrintFuncKey(key[4], THREAD_TAG, thread_keys),
				PrintFuncKey(key[5], MARK_ARTICLE_UNREAD, thread_keys));
			center_line(line + 1, FALSE, buf);
			break;

		case PAGE_LEVEL:
			snprintf(buf, bufs, _(txt_mini_page_1),
				PrintFuncKey(key[0], PAGE_NEXT_UNREAD, page_keys),
				PrintFuncKey(key[1], GLOBAL_SEARCH_SUBJECT_FORWARD, page_keys),
				PrintFuncKey(key[2], GLOBAL_MENU_FILTER_KILL, page_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_page_2),
				PrintFuncKey(key[0], GLOBAL_SEARCH_AUTHOR_FORWARD, page_keys),
				PrintFuncKey(key[1], GLOBAL_SEARCH_BODY, page_keys),
				PrintFuncKey(key[2], CATCHUP, page_keys),
				PrintFuncKey(key[3], PAGE_FOLLOWUP_QUOTE, page_keys),
				PrintFuncKey(key[4], PAGE_MARK_THREAD_READ, page_keys));
			center_line(line + 1, FALSE, buf);

#if defined(DONT_HAVE_PIPING) && defined(DISABLE_PRINTING)
			snprintf(buf, bufs, _(txt_mini_page_3),
				PrintFuncKey(key[1], PAGE_MAIL, page_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, page_keys),
				PrintFuncKey(key[4], PAGE_REPLY_QUOTE, page_keys),
				PrintFuncKey(key[5], PAGE_SAVE, page_keys),
				PrintFuncKey(key[6], PAGE_TAG, page_keys),
				PrintFuncKey(key[7], GLOBAL_POST, page_keys));
#else
#	ifdef DONT_HAVE_PIPING
			snprintf(buf, bufs, _(txt_mini_page_3),
				PrintFuncKey(key[1], PAGE_MAIL, page_keys),
				PrintFuncKey(key[2], GLOBAL_PRINT, page_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, page_keys),
				PrintFuncKey(key[4], PAGE_REPLY_QUOTE, page_keys),
				PrintFuncKey(key[5], PAGE_SAVE, page_keys),
				PrintFuncKey(key[6], PAGE_TAG, page_keys),
				PrintFuncKey(key[7], GLOBAL_POST, page_keys));
#	else
#		ifdef DISABLE_PRINTING
			snprintf(buf, bufs, _(txt_mini_page_3),
				PrintFuncKey(key[0], GLOBAL_PIPE, page_keys),
				PrintFuncKey(key[1], PAGE_MAIL, page_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, page_keys),
				PrintFuncKey(key[4], PAGE_REPLY_QUOTE, page_keys),
				PrintFuncKey(key[5], PAGE_SAVE, page_keys),
				PrintFuncKey(key[6], PAGE_TAG, page_keys),
				PrintFuncKey(key[7], GLOBAL_POST, page_keys));
#		else
			snprintf(buf, bufs, _(txt_mini_page_3),
				PrintFuncKey(key[0], GLOBAL_PIPE, page_keys),
				PrintFuncKey(key[1], PAGE_MAIL, page_keys),
				PrintFuncKey(key[2], GLOBAL_PRINT, page_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, page_keys),
				PrintFuncKey(key[4], PAGE_REPLY_QUOTE, page_keys),
				PrintFuncKey(key[5], PAGE_SAVE, page_keys),
				PrintFuncKey(key[6], PAGE_TAG, page_keys),
				PrintFuncKey(key[7], GLOBAL_POST, page_keys));
#		endif /* DISABLE_PRINTING */
#	endif /* DONT_HAVE_PIPING */
#endif /* DONT_HAVE_PIPING && DISABLE_PRINTING */

			center_line(line + 2, FALSE, buf);
			break;

		case POSTED_LEVEL:
			snprintf(buf, bufs, _(txt_mini_post_hist_1),
				PrintFuncKey(key[0], GLOBAL_LINE_DOWN, post_hist_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_UP, post_hist_keys),
				PrintFuncKey(key[2], GLOBAL_HELP, post_hist_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, post_hist_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_post_hist_2),
				PrintFuncKey(key[2], GLOBAL_SEARCH_SUBJECT_FORWARD, post_hist_keys),
				PrintFuncKey(key[3], GLOBAL_SEARCH_SUBJECT_BACKWARD, post_hist_keys),
				PrintFuncKey(key[4], GLOBAL_SEARCH_REPEAT, post_hist_keys));
			center_line(line + 1, FALSE, buf);
			break;

		case URL_LEVEL:
			snprintf(buf, bufs, _(txt_mini_url_1),
				PrintFuncKey(key[0], GLOBAL_LINE_DOWN, url_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_UP, url_keys),
				PrintFuncKey(key[2], GLOBAL_HELP, url_keys),
				PrintFuncKey(key[3], GLOBAL_QUIT, url_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_url_2),
				PrintFuncKey(key[2], GLOBAL_SEARCH_SUBJECT_FORWARD, url_keys),
				PrintFuncKey(key[3], GLOBAL_SEARCH_SUBJECT_BACKWARD, url_keys),
				PrintFuncKey(key[4], GLOBAL_SEARCH_REPEAT, url_keys));
			center_line(line + 1, FALSE, buf);
			break;

		case INFO_PAGER:
			snprintf(buf, bufs, _(txt_mini_info_1),
				PrintFuncKey(key[0], GLOBAL_LINE_UP, info_keys),
				PrintFuncKey(key[1], GLOBAL_LINE_DOWN, info_keys),
				PrintFuncKey(key[2], GLOBAL_PAGE_UP, info_keys),
				PrintFuncKey(key[3], GLOBAL_PAGE_DOWN, info_keys),
				PrintFuncKey(key[4], GLOBAL_FIRST_PAGE, info_keys),
				PrintFuncKey(key[5], GLOBAL_LAST_PAGE, info_keys));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_info_2),
				PrintFuncKey(key[0], GLOBAL_SEARCH_SUBJECT_FORWARD, info_keys),
				PrintFuncKey(key[1], GLOBAL_SEARCH_SUBJECT_BACKWARD, info_keys),
				PrintFuncKey(key[2], GLOBAL_QUIT, info_keys));
			center_line(line + 1, FALSE, buf);
			break;

		default: /* should not happen */
			error_message(2, _(txt_error_unknown_dlevel));
			break;
	}
#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
}


void
toggle_mini_help(
	int level)
{
	tinrc.beginner_level = bool_not(tinrc.beginner_level);
	set_noteslines(cLINES);
	show_mini_help(level);
}
