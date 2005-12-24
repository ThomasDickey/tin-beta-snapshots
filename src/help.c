/*
 *  Project   : tin - a Usenet reader
 *  Module    : help.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2005-07-02
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


typedef struct thp {
	constext *helptext;
	t_function func;
} t_help_page;

/*
 * local prototypes
 */
static void make_help_page(FILE *fp, const t_help_page *helppage, const struct keylist keys);


static constext txt_help_empty_line[] = "";

static t_help_page select_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_global_scroll_down, GLOBAL_SCROLL_DOWN },
	{ txt_help_global_scroll_up, GLOBAL_SCROLL_UP },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_select_first_group, GLOBAL_FIRST_PAGE },
	{ txt_help_select_last_group, GLOBAL_LAST_PAGE },
	{ txt_help_select_group_by_num, NOT_ASSIGNED },
	{ txt_help_select_goto_group, SELECT_GOTO },
	{ txt_help_select_next_unread_group, SELECT_NEXT_UNREAD_GROUP },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_select_search_group_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_select_search_group_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_select_search_group_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_select_toggle_read_groups, SELECT_TOGGLE_READ_DISPLAY },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_select_toggle_descriptions, SELECT_TOGGLE_DESCRIPTIONS },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_select_sort_active, SELECT_SORT_ACTIVE },
	{ txt_help_select_yank_active, SELECT_YANK_ACTIVE },
	{ txt_help_select_sync_with_active, SELECT_SYNC_WITH_ACTIVE },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
	{ txt_help_select_read_group, SELECT_ENTER_GROUP },
	{ txt_help_select_next_unread_group, SELECT_ENTER_NEXT_UNREAD_GROUP },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
#endif /* NO_POSTING */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_select_group_range, GLOBAL_SET_RANGE },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_select_catchup, CATCHUP },
	{ txt_help_select_catchup_next_unread, CATCHUP_NEXT_UNREAD },
	{ txt_help_select_mark_group_unread, SELECT_MARK_GROUP_UNREAD },
	{ txt_help_select_subscribe, SELECT_SUBSCRIBE },
	{ txt_help_select_unsubscribe, SELECT_UNSUBSCRIBE },
	{ txt_help_select_subscribe_pattern, SELECT_SUBSCRIBE_PATTERN },
	{ txt_help_select_unsubscribe_pattern, SELECT_UNSUBSCRIBE_PATTERN },
	{ txt_help_select_move_group, SELECT_MOVE_GROUP },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER},
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_search_subj_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_global_search_subj_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_auth_forwards, GLOBAL_SEARCH_AUTHOR_FORWARD },
	{ txt_help_global_search_auth_backwards, GLOBAL_SEARCH_AUTHOR_BACKWARD },
	{ txt_help_global_search_body, GLOBAL_SEARCH_BODY },
	{ txt_help_global_search_body_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_group_toggle_read_articles, GROUP_TOGGLE_READ_UNREAD },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_select_toggle_descriptions, SELECT_TOGGLE_DESCRIPTIONS },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_toggle_subj_display, GROUP_TOGGLE_SUBJECT_DISPLAY},
	{ txt_help_group_toggle_threading, GROUP_TOGGLE_THREADING },
	{ txt_help_group_mark_unsel_art_read, GROUP_MARK_UNSELECTED_ARTICLES_READ },
	{ txt_help_group_toggle_getart_limit, GROUP_TOGGLE_GET_ARTICLES_LIMIT },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
	{ txt_help_thread_read_article, GROUP_READ_BASENOTE },
	{ txt_help_article_next_unread, GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
	{ txt_help_article_repost, GROUP_REPOST },
#endif /* NO_POSTING */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_article_range, GLOBAL_SET_RANGE },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_mail, GROUP_MAIL },
	{ txt_help_global_save, GROUP_SAVE },
	{ txt_help_global_auto_save, GROUP_AUTOSAVE },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, GLOBAL_PIPE },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, GLOBAL_PRINT },
#endif /* !DISABLE_PRINTING */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_tag, GROUP_TAG },
	{ txt_help_group_tag_parts, GROUP_TAG_PARTS },
	{ txt_help_group_untag_thread, GROUP_UNTAG },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_group_mark_thread_read, GROUP_MARK_THREAD_READ },
	{ txt_help_group_catchup, CATCHUP },
	{ txt_help_group_catchup_next, CATCHUP_NEXT_UNREAD },
	{ txt_help_group_mark_article_unread, MARK_ARTICLE_UNREAD },
	{ txt_help_group_mark_thread_unread, MARK_THREAD_UNREAD },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_group_select_all, GROUP_DO_AUTOSELECT },
	{ txt_help_group_select_thread, GROUP_SELECT_THREAD },
	{ txt_help_group_select_thread_pattern, GROUP_SELECT_PATTERN },
	{ txt_help_group_select_thread_if_unread_selected, GROUP_SELECT_THREAD_IF_UNREAD_SELECTED },
	{ txt_help_group_toggle_thread_selection, GROUP_TOGGLE_SELECT_THREAD },
	{ txt_help_group_reverse_thread_selection, GROUP_REVERSE_SELECTIONS },
	{ txt_help_group_undo_thread_selection, GROUP_UNDO_SELECTIONS },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_article_autoselect, GLOBAL_MENU_FILTER_SELECT },
	{ txt_help_article_autokill, GLOBAL_MENU_FILTER_KILL },
	{ txt_help_article_quick_select, GLOBAL_QUICK_FILTER_SELECT },
	{ txt_help_article_quick_kill, GLOBAL_QUICK_FILTER_KILL },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER},
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_thread_first_article, GLOBAL_FIRST_PAGE },
	{ txt_help_thread_last_article, GLOBAL_LAST_PAGE },
	{ txt_help_thread_article_by_num, NOT_ASSIGNED },
	{ txt_help_global_last_art, GLOBAL_LAST_VIEWED },
	{ txt_help_global_lookup_art, GLOBAL_LOOKUP_MESSAGEID },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_search_subj_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_global_search_subj_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_auth_forwards, GLOBAL_SEARCH_AUTHOR_FORWARD },
	{ txt_help_global_search_auth_backwards, GLOBAL_SEARCH_AUTHOR_BACKWARD },
	{ txt_help_global_search_body, GLOBAL_SEARCH_BODY },
	{ txt_help_global_search_body_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_global_toggle_subj_display, THREAD_TOGGLE_SUBJECT_DISPLAY},
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_ops, NOT_ASSIGNED },
	{ txt_help_thread_read_article, THREAD_READ_ARTICLE },
	{ txt_help_article_next_unread, THREAD_READ_NEXT_ARTICLE_OR_THREAD },
#ifndef NO_POSTING
	{ txt_help_global_post, GLOBAL_POST },
	{ txt_help_global_post_postponed, GLOBAL_POSTPONED },
#endif /* NO_POSTING */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_article_range, GLOBAL_SET_RANGE },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_mail, THREAD_MAIL },
	{ txt_help_global_save, THREAD_SAVE },
	{ txt_help_global_auto_save, THREAD_AUTOSAVE },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, GLOBAL_PIPE },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, GLOBAL_PRINT },
#endif /* !DISABLE_PRINTING */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_tag, THREAD_TAG },
	{ txt_help_group_untag_thread, THREAD_UNTAG },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_thread_mark_article_read, THREAD_MARK_ARTICLE_READ },
	{ txt_help_thread_catchup, CATCHUP },
	{ txt_help_thread_catchup_next_unread, CATCHUP_NEXT_UNREAD },
	{ txt_help_group_mark_article_unread, MARK_ARTICLE_UNREAD },
	{ txt_help_group_mark_thread_unread, MARK_THREAD_UNREAD },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_group_select_thread, THREAD_SELECT_ARTICLE },
	{ txt_help_group_toggle_thread_selection, THREAD_TOGGLE_ARTICLE_SELECTION },
	{ txt_help_group_reverse_thread_selection, THREAD_REVERSE_SELECTIONS },
	{ txt_help_group_undo_thread_selection, THREAD_UNDO_SELECTIONS },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_article_autoselect, GLOBAL_MENU_FILTER_SELECT },
	{ txt_help_article_autokill, GLOBAL_MENU_FILTER_KILL },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER },
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ txt_help_bug_report, GLOBAL_BUGREPORT },
	{ NULL, NOT_ASSIGNED }
};

static t_help_page page_help_page[] = {
	{ txt_help_title_navi, NOT_ASSIGNED },
	{ txt_help_global_page_down, GLOBAL_PAGE_DOWN },
	{ txt_help_global_page_down, PAGE_PAGE_DOWN3 },
	{ txt_help_global_page_up, GLOBAL_PAGE_UP },
	{ txt_help_global_line_down, GLOBAL_LINE_DOWN },
	{ txt_help_global_line_up, GLOBAL_LINE_UP },
	{ txt_help_article_first_page, GLOBAL_FIRST_PAGE },
	{ txt_help_article_last_page, GLOBAL_LAST_PAGE },
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_article_search_forwards, GLOBAL_SEARCH_SUBJECT_FORWARD },
	{ txt_help_article_search_backwards, GLOBAL_SEARCH_SUBJECT_BACKWARD },
	{ txt_help_global_search_auth_forwards, GLOBAL_SEARCH_AUTHOR_FORWARD },
	{ txt_help_global_search_auth_backwards, GLOBAL_SEARCH_AUTHOR_BACKWARD },
	{ txt_help_global_search_body, GLOBAL_SEARCH_BODY },
	{ txt_help_global_search_body_comment, NOT_ASSIGNED },
	{ txt_help_global_search_repeat, GLOBAL_SEARCH_REPEAT },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_title_disp, NOT_ASSIGNED },
	{ txt_help_global_toggle_info_line, GLOBAL_TOGGLE_INFO_LAST_LINE },
	{ txt_help_article_toggle_rot13, PAGE_TOGGLE_ROT13 },
	{ txt_help_global_toggle_inverse_video, GLOBAL_TOGGLE_INVERSE_VIDEO },
	{ txt_help_article_show_raw, PAGE_TOGGLE_HEADERS },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, GLOBAL_TOGGLE_COLOR },
#endif /* HAVE_COLOR */
	{ txt_help_article_toggle_highlight, PAGE_TOGGLE_HIGHLIGHTING },
	{ txt_help_article_toggle_tex2iso, PAGE_TOGGLE_TEX2ISO },
	{ txt_help_article_toggle_tabwidth, PAGE_TOGGLE_TABS },
	{ txt_help_article_toggle_uue, PAGE_TOGGLE_UUE },
	{ txt_help_article_toggle_formfeed, PAGE_REVEAL },
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_tag, PAGE_TAG },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_article_mark_thread_read, PAGE_MARK_THREAD_READ },
	{ txt_help_thread_catchup, CATCHUP },
	{ txt_help_thread_catchup_next_unread, CATCHUP_NEXT_UNREAD },
	{ txt_help_group_mark_article_unread, MARK_ARTICLE_UNREAD },
	{ txt_help_group_mark_thread_unread, MARK_THREAD_UNREAD },
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_article_autoselect, GLOBAL_MENU_FILTER_SELECT },
	{ txt_help_article_autokill, GLOBAL_MENU_FILTER_KILL },
	{ txt_help_article_quick_select, GLOBAL_QUICK_FILTER_SELECT },
	{ txt_help_article_quick_kill, GLOBAL_QUICK_FILTER_KILL },
	{ txt_help_global_edit_filter, GLOBAL_EDIT_FILTER },
	{ txt_help_empty_line, NOT_ASSIGNED },
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
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_article_pgp, PAGE_PGP_CHECK_ARTICLE },
#endif /* HAVE_PGP_GPG */
	{ txt_help_empty_line, NOT_ASSIGNED },
	{ txt_help_global_version, GLOBAL_VERSION },
	{ NULL, NOT_ASSIGNED }
};


static void
make_help_page(
	FILE *fp,
	const t_help_page *helppage,
	const struct keylist keys)
{
	char *buf = my_malloc(LEN);
	char *last = my_malloc(LEN);
	char key[MAXKEYLEN];
	/*
	 * length is only needed to pass it to expand_ctrl_chars()
	 * we have no need for the value
	 */
	int length;
	size_t i;

	if (!helppage)
		return;

	last[0] = '\0';

	while (helppage->helptext) {
		if (helppage->func == NOT_ASSIGNED) {
			/*
			 * as expand_ctrl_chars() may has shrinked buf
			 * make sure buf is large enough to contain the helpline
			 */
			buf = my_realloc(buf, LEN);

			if (!strlen(helppage->helptext))	/* avoid translation of empty strings */
				snprintf(buf, LEN, "\n");
			else
				snprintf(buf, LEN, "%s", _(helppage->helptext));
			buf[LEN - 1] = '\0';
			expand_ctrl_chars(&buf, &length, 8);
			fprintf(fp, "%s", buf);
		} else {
			for (i = 0; i < keys.used; i++) {
				if (keys.list[i].function == helppage->func && keys.list[i].key) {
					buf = my_realloc(buf, LEN);
					snprintf(buf, LEN, "%s\t  %s", printascii(key, keys.list[i].key), _(helppage->helptext));
					buf[LEN - 1] = '\0';
					expand_ctrl_chars(&buf, &length, 8);
					if (strcmp(last, buf)) {
						fprintf(fp, "%s", buf);
						strncpy(last, buf, LEN);
					}
				}
			}
		}
		helppage++;
	}

	free(buf);
	free(last);
}


void
show_help_page(
	const int level,
	const char *title)
{
	FILE *fp;

	if (!(fp = tmpfile()))
		return;

	switch (level) {
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

		case INFO_PAGER:
		default: /* should not happen */
			error_message(_(txt_error_unknown_dlevel));
			fclose(fp);
			return;
	}

	info_pager(fp, title, TRUE);
	fclose(fp);
	return;
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
	bufs = (size_t) MIN((unsigned) cCOLS, (sizeof(buf) - 1));

#ifdef HAVE_COLOR
	fcol(tinrc.col_minihelp);
#endif /* HAVE_COLOR */

	switch (level) {
		case SELECT_LEVEL:
			snprintf(buf, bufs, _(txt_mini_select_1),
				printascii(key[0], func_to_key(SELECT_ENTER_NEXT_UNREAD_GROUP, select_keys)),
				printascii(key[1], func_to_key(SELECT_GOTO, select_keys)),
				printascii(key[2], func_to_key(GLOBAL_SEARCH_SUBJECT_FORWARD, select_keys)),
				printascii(key[3], func_to_key(CATCHUP, select_keys)));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_select_2),
				printascii(key[0], func_to_key(GLOBAL_LINE_DOWN, select_keys)),
				printascii(key[1], func_to_key(GLOBAL_LINE_UP, select_keys)),
				printascii(key[2], func_to_key(GLOBAL_HELP, select_keys)),
				printascii(key[3], func_to_key(SELECT_MOVE_GROUP, select_keys)),
				printascii(key[4], func_to_key(GLOBAL_QUIT, select_keys)),
				printascii(key[5], func_to_key(SELECT_TOGGLE_READ_DISPLAY, select_keys)));
			center_line(line + 1, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_select_3),
				printascii(key[0], func_to_key(SELECT_SUBSCRIBE, select_keys)),
				printascii(key[1], func_to_key(SELECT_SUBSCRIBE_PATTERN, select_keys)),
				printascii(key[2], func_to_key(SELECT_UNSUBSCRIBE, select_keys)),
				printascii(key[3], func_to_key(SELECT_UNSUBSCRIBE_PATTERN, select_keys)),
				printascii(key[4], func_to_key(SELECT_YANK_ACTIVE, select_keys)));
			center_line(line + 2, FALSE, buf);
			break;

		case GROUP_LEVEL:
			snprintf(buf, bufs, _(txt_mini_group_1),
				printascii(key[0], func_to_key(GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP, group_keys)),
				printascii(key[1], func_to_key(GLOBAL_SEARCH_SUBJECT_FORWARD, group_keys)),
				printascii(key[2], func_to_key(GLOBAL_MENU_FILTER_KILL, group_keys)));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_group_2),
				printascii(key[0], func_to_key(GLOBAL_SEARCH_AUTHOR_FORWARD, group_keys)),
				printascii(key[1], func_to_key(CATCHUP, group_keys)),
				printascii(key[2], func_to_key(GLOBAL_LINE_DOWN, group_keys)),
				printascii(key[3], func_to_key(GLOBAL_LINE_UP, group_keys)),
				printascii(key[4], func_to_key(GROUP_MARK_THREAD_READ, group_keys)),
				printascii(key[5], func_to_key(GROUP_LIST_THREAD, group_keys)));
			center_line(line + 1, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_group_3),
#ifndef DONT_HAVE_PIPING
				printascii(key[0], func_to_key(GLOBAL_PIPE, group_keys)),
#endif /* !DONT_HAVE_PIPING */
				printascii(key[1], func_to_key(GROUP_MAIL, group_keys)),
#ifndef DISABLE_PRINTING
				printascii(key[2], func_to_key(GLOBAL_PRINT, group_keys)),
#endif /* !DISABLE_PRINTING */
				printascii(key[3], func_to_key(GLOBAL_QUIT, group_keys)),
				printascii(key[4], func_to_key(GROUP_TOGGLE_READ_UNREAD, group_keys)),
				printascii(key[5], func_to_key(GROUP_SAVE, group_keys)),
				printascii(key[6], func_to_key(GROUP_TAG, group_keys)),
				printascii(key[7], func_to_key(GLOBAL_POST, group_keys)));
			center_line(line + 2, FALSE, buf);
			break;

		case THREAD_LEVEL:
			snprintf(buf, bufs, _(txt_mini_thread_1),
				printascii(key[0], func_to_key(THREAD_READ_NEXT_ARTICLE_OR_THREAD, thread_keys)),
				printascii(key[1], func_to_key(CATCHUP, thread_keys)),
				printascii(key[2], func_to_key(THREAD_TOGGLE_SUBJECT_DISPLAY, thread_keys)));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_thread_2),
				printascii(key[0], func_to_key(GLOBAL_HELP, thread_keys)),
				printascii(key[1], func_to_key(GLOBAL_LINE_DOWN, thread_keys)),
				printascii(key[2], func_to_key(GLOBAL_LINE_UP, thread_keys)),
				printascii(key[3], func_to_key(GLOBAL_QUIT, thread_keys)),
				printascii(key[4], func_to_key(THREAD_TAG, thread_keys)),
				printascii(key[5], func_to_key(MARK_ARTICLE_UNREAD, thread_keys)));
			center_line(line + 1, FALSE, buf);
			break;

		case PAGE_LEVEL:
			snprintf(buf, bufs, _(txt_mini_page_1),
				printascii(key[0], func_to_key(PAGE_NEXT_UNREAD, page_keys)),
				printascii(key[1], func_to_key(GLOBAL_SEARCH_SUBJECT_FORWARD, page_keys)),
				printascii(key[2], func_to_key(GLOBAL_MENU_FILTER_KILL, page_keys)));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_page_2),
				printascii(key[0], func_to_key(GLOBAL_SEARCH_AUTHOR_FORWARD, page_keys)),
				printascii(key[1], func_to_key(GLOBAL_SEARCH_BODY, page_keys)),
				printascii(key[2], func_to_key(CATCHUP, page_keys)),
				printascii(key[3], func_to_key(PAGE_FOLLOWUP_QUOTE, page_keys)),
				printascii(key[4], func_to_key(PAGE_MARK_THREAD_READ, page_keys)));
			center_line(line + 1, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_page_3),
#ifndef DONT_HAVE_PIPING
				printascii(key[0], func_to_key(GLOBAL_PIPE, page_keys)),
#endif /* !DONT_HAVE_PIPING */
				printascii(key[1], func_to_key(PAGE_MAIL, page_keys)),
#ifndef DISABLE_PRINTING
				printascii(key[2], func_to_key(GLOBAL_PRINT, page_keys)),
#endif /* !DISABLE_PRINTING */
				printascii(key[3], func_to_key(GLOBAL_QUIT, page_keys)),
				printascii(key[4], func_to_key(PAGE_REPLY_QUOTE, page_keys)),
				printascii(key[5], func_to_key(PAGE_SAVE, page_keys)),
				printascii(key[6], func_to_key(PAGE_TAG, page_keys)),
				printascii(key[7], func_to_key(GLOBAL_POST, page_keys)));
			center_line(line + 2, FALSE, buf);
			break;

		case INFO_PAGER:
			snprintf(buf, bufs, _(txt_mini_info_1),
				printascii(key[0], func_to_key(GLOBAL_LINE_UP, info_keys)),
				printascii(key[1], func_to_key(GLOBAL_LINE_DOWN, info_keys)),
				printascii(key[2], func_to_key(GLOBAL_PAGE_UP, info_keys)),
				printascii(key[3], func_to_key(GLOBAL_PAGE_DOWN, info_keys)),
				printascii(key[4], func_to_key(GLOBAL_FIRST_PAGE, info_keys)),
				printascii(key[5], func_to_key(GLOBAL_LAST_PAGE, info_keys)));
			center_line(line, FALSE, buf);
			snprintf(buf, bufs, _(txt_mini_info_2),
				printascii(key[0], func_to_key(GLOBAL_SEARCH_SUBJECT_FORWARD, info_keys)),
				printascii(key[1], func_to_key(GLOBAL_SEARCH_SUBJECT_BACKWARD, info_keys)),
				printascii(key[2], func_to_key(GLOBAL_QUIT, info_keys)));
			center_line(line + 1, FALSE, buf);
			break;

		default: /* should not happen */
			error_message(_(txt_error_unknown_dlevel));
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
