/*
 *  Project   : tin - a Usenet reader
 *  Module    : help.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2001-11-10
 *  Notes     :
 *
 * Copyright (c) 1991-2002 Iain Lea <iain@bricbrac.de>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Iain Lea.
 * 4. The name of the author may not be used to endorse or promote
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

typedef struct thp {
	constext *helptext;
	char key;
} t_help_page;

const char **info_help;
static constext txt_help_empty_line[] = "";

static t_help_page select_help_page[] = {
	{ txt_help_title_navi, 0 },
	{ txt_help_global_page_down, iKeyPageDown },
	{ txt_help_global_page_down, iKeyPageDown2 },
	{ txt_help_global_page_down, iKeyPageDown3 },
	{ txt_help_global_page_up, iKeyPageUp2 },
	{ txt_help_global_page_up, iKeyPageUp },
	{ txt_help_global_page_up, iKeyPageUp3 },
	{ txt_help_global_line_down, iKeyDown2 },
	{ txt_help_global_line_down, iKeyDown },
	{ txt_help_global_line_up, iKeyUp2 },
	{ txt_help_global_line_up, iKeyUp },
	{ txt_help_empty_line, 0 },
	{ txt_help_select_first_group, iKeyFirstPage },
	{ txt_help_select_last_group, iKeyLastPage },
	{ txt_help_select_group_by_num, 0 },
	{ txt_help_select_goto_group, iKeySelectGoto },
	{ txt_help_select_next_unread_group, iKeySelectNextUnreadGrp },
	{ txt_help_empty_line, 0 },
	{ txt_help_select_search_group_forwards, iKeySearchSubjF },
	{ txt_help_select_search_group_backwards, iKeySearchSubjB },
	{ txt_help_select_search_group_comment, 0 },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_disp, 0 },
	{ txt_help_select_toggle_read_groups, iKeySelectToggleReadDisplay },
	{ txt_help_global_toggle_info_line, iKeyToggleInfoLastLine },
	{ txt_help_select_toggle_descriptions, iKeySelectToggleDescriptions },
	{ txt_help_global_toggle_inverse_video, iKeyToggleInverseVideo },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, iKeyToggleColor },
#endif /* HAVE_COLOR */
	{ txt_help_empty_line, 0 },
	{ txt_help_select_sort_active, iKeySelectSortActive },
	{ txt_help_select_yank_active, iKeySelectYankActive },
	{ txt_help_select_sync_with_active, iKeySelectSyncWithActive },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_ops, 0 },
	{ txt_help_select_read_group, iKeySelectReadGrp },
	{ txt_help_select_next_unread_group, iKeySelectEnterNextUnreadGrp2 },
	{ txt_help_select_next_unread_group, iKeySelectEnterNextUnreadGrp },
	{ txt_help_global_post, iKeyPost },
	{ txt_help_global_post_postponed, iKeyPostponed2 },
	{ txt_help_global_post_postponed, iKeyPostponed },
	{ txt_help_empty_line, 0 },
	{ txt_help_select_group_range, iKeySetRange },
	{ txt_help_empty_line, 0 },
	{ txt_help_select_catchup, iKeySelectCatchup },
	{ txt_help_select_catchup_next_unread, iKeySelectCatchupNextUnread },
	{ txt_help_select_mark_group_unread, iKeySelectMarkGrpUnread },
	{ txt_help_select_mark_group_unread, iKeySelectMarkGrpUnread2 },
	{ txt_help_select_subscribe, iKeySelectSubscribe },
	{ txt_help_select_unsubscribe, iKeySelectUnsubscribe },
	{ txt_help_select_subscribe_pattern, iKeySelectSubscribePat },
	{ txt_help_select_unsubscribe_pattern, iKeySelectUnsubscribePat },
	{ txt_help_select_move_group, iKeySelectMoveGrp },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_misc, 0 },
	{ txt_help_select_quit, iKeyQuit },
	{ txt_help_select_quit, iKeyQuitTin },
	{ txt_help_select_quit_no_write, iKeySelectQuitNoWrite },
	{ txt_help_global_help, iKeyHelp },
	{ txt_help_global_toggle_mini_help, iKeyToggleHelpDisplay },
	{ txt_help_global_option_menu, iKeyOptionMenu },
	{ txt_help_global_esc, iKeyAbort },
	{ txt_help_global_redraw_screen, iKeyRedrawScr },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, iKeyShellEscape },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, iKeyDisplayPostHist },
	{ txt_help_select_reset_newsrc, iKeySelectResetNewsrc },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_version, iKeyVersion },
	{ txt_help_bug_report, iKeySelectBugReport },
	{ NULL, 0 }
};

static t_help_page group_help_page[] = {
	{ txt_help_title_navi, 0 },
	{ txt_help_global_page_down, iKeyPageDown },
	{ txt_help_global_page_down, iKeyPageDown2 },
	{ txt_help_global_page_down, iKeyPageDown3 },
	{ txt_help_global_page_up, iKeyPageUp2 },
	{ txt_help_global_page_up, iKeyPageUp },
	{ txt_help_global_page_up, iKeyPageUp3 },
	{ txt_help_global_line_down, iKeyDown2 },
	{ txt_help_global_line_down, iKeyDown },
	{ txt_help_global_line_up, iKeyUp2 },
	{ txt_help_global_line_up, iKeyUp },
	{ txt_help_empty_line, 0 },
	{ txt_help_group_first_thread, iKeyFirstPage },
	{ txt_help_group_last_thread, iKeyLastPage },
	{ txt_help_group_thread_by_num, 0 },
	{ txt_help_group_goto_group, iKeyGroupGoto },
	{ txt_help_group_next, iKeyGroupNextGroup },
	{ txt_help_group_prev, iKeyGroupPrevGroup },
	{ txt_help_group_next_unread_art, iKeyGroupNextUnreadArt },
	{ txt_help_group_prev_unread_art, iKeyGroupPrevUnreadArt },
	{ txt_help_global_last_art, iKeyLastViewed },
	{ txt_help_global_lookup_art, iKeyLookupMessage },
	{ txt_help_group_list_thread, iKeyGroupListThd },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_search_subj_forwards, iKeySearchSubjF },
	{ txt_help_global_search_subj_backwards, iKeySearchSubjB },
	{ txt_help_global_search_auth_forwards, iKeySearchAuthF },
	{ txt_help_global_search_auth_backwards, iKeySearchAuthB },
	{ txt_help_global_search_body, iKeySearchBody },
	{ txt_help_global_search_body_comment, 0 },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_disp, 0 },
	{ txt_help_group_toggle_read_articles, iKeyGroupToggleReadUnread },
	{ txt_help_global_toggle_info_line, iKeyToggleInfoLastLine },
	{ txt_help_group_toggle_subj_display, iKeyGroupToggleSubjDisplay },
	{ txt_help_global_toggle_inverse_video, iKeyToggleInverseVideo },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, iKeyToggleColor },
#endif /* HAVE_COLOR */
	{ txt_help_empty_line, 0 },
	{ txt_help_group_toggle_threading, iKeyGroupToggleThreading },
	{ txt_help_group_mark_unsel_art_read, iKeyGroupMarkUnselArtRead },
	{ txt_help_group_toggle_getart_limit, iKeyGroupToggleGetartLimit },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_ops, 0 },
	{ txt_help_group_read_article, iKeyGroupReadBasenote },
	{ txt_help_group_next_unread_article, iKeyGroupNextUnreadArtOrGrp },
	{ txt_help_global_post, iKeyPost },
	{ txt_help_global_post_postponed, iKeyPostponed2 },
	{ txt_help_global_post_postponed, iKeyPostponed },
	{ txt_help_group_repost, iKeyGroupRepost },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_article_range, iKeySetRange },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_mail, iKeyGroupMail },
	{ txt_help_global_save, iKeyGroupSave },
	{ txt_help_global_save_tagged, iKeyGroupAutoSaveTagged },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, iKeyPipe },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, iKeyPrint },
#endif /* !DISABLE_PRINTING */
	{ txt_help_empty_line, 0 },
	{ txt_help_global_tag, iKeyGroupTag },
	{ txt_help_group_tag_parts, iKeyGroupTagParts },
	{ txt_help_group_untag_thread, iKeyGroupUntag },
	{ txt_help_empty_line, 0 },
	{ txt_help_group_mark_thread_read, iKeyGroupMarkThdRead },
	{ txt_help_group_catchup, iKeyGroupCatchup },
	{ txt_help_group_catchup_next, iKeyGroupCatchupNextUnread },
	{ txt_help_group_mark_article_unread, iKeyGroupMarkArtUnread },
	{ txt_help_group_mark_thread_unread, iKeyGroupMarkThdUnread },
	{ txt_help_empty_line, 0 },
	{ txt_help_group_select_all, iKeyGroupDoAutoSel },
	{ txt_help_group_select_thread, iKeyGroupSelThd },
	{ txt_help_group_select_thread_pattern, iKeyGroupSelPattern },
	{ txt_help_group_select_thread_if_unread_selected, iKeyGroupSelThdIfUnreadSelected },
	{ txt_help_group_toggle_thread_selection, iKeyGroupToggleThdSel },
	{ txt_help_group_reverse_thread_selection, iKeyGroupReverseSel },
	{ txt_help_group_undo_thread_selection, iKeyGroupUndoSel },
	{ txt_help_empty_line, 0 },
	{ txt_help_article_autoselect, iKeyGroupAutoSel },
	{ txt_help_article_autokill, iKeyGroupKill },
	{ txt_help_article_quick_select, iKeyGroupQuickAutoSel },
	{ txt_help_article_quick_kill, iKeyGroupQuickKill },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_misc, 0 },
	{ txt_help_global_previous_menu, iKeyQuit },
	{ txt_help_global_quit_tin, iKeyQuitTin },
	{ txt_help_global_help, iKeyHelp },
	{ txt_help_global_toggle_mini_help, iKeyToggleHelpDisplay },
	{ txt_help_global_option_menu, iKeyOptionMenu },
	{ txt_help_global_esc, iKeyAbort },
	{ txt_help_global_redraw_screen, iKeyRedrawScr },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, iKeyShellEscape },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, iKeyDisplayPostHist },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_version, iKeyVersion },
	{ txt_help_bug_report, iKeyGroupBugReport },
	{ NULL, 0 }
};

static t_help_page thread_help_page[] = {
	{ txt_help_title_navi, 0 },
	{ txt_help_global_page_down, iKeyPageDown },
	{ txt_help_global_page_down, iKeyPageDown2 },
	{ txt_help_global_page_down, iKeyPageDown3 },
	{ txt_help_global_page_up, iKeyPageUp2 },
	{ txt_help_global_page_up, iKeyPageUp },
	{ txt_help_global_page_up, iKeyPageUp3 },
	{ txt_help_global_line_down, iKeyDown2 },
	{ txt_help_global_line_down, iKeyDown },
	{ txt_help_global_line_up, iKeyUp2 },
	{ txt_help_global_line_up, iKeyUp },
	{ txt_help_empty_line, 0 },
	{ txt_help_thread_first_article, iKeyFirstPage },
	{ txt_help_thread_last_article, iKeyLastPage },
	{ txt_help_thread_article_by_num, 0 },
	{ txt_help_global_last_art, iKeyLastViewed },
	{ txt_help_global_lookup_art, iKeyLookupMessage },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_search_subj_forwards, iKeySearchSubjF },
	{ txt_help_global_search_subj_backwards, iKeySearchSubjB },
	{ txt_help_global_search_auth_forwards, iKeySearchAuthF },
	{ txt_help_global_search_auth_backwards, iKeySearchAuthB },
	{ txt_help_global_search_body, iKeySearchBody },
	{ txt_help_global_search_body_comment, 0 },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_disp, 0 },
	{ txt_help_global_toggle_info_line, iKeyToggleInfoLastLine },
	{ txt_help_thread_toggle_subj_display, iKeyThreadToggleSubjDisplay },
	{ txt_help_global_toggle_inverse_video, iKeyToggleInverseVideo },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, iKeyToggleColor },
#endif /* HAVE_COLOR */
	{ txt_help_empty_line, 0 },
	{ txt_help_title_ops, 0 },
	{ txt_help_thread_read_article, iKeyThreadReadArt },
	{ txt_help_article_read_next_unread, iKeyThreadReadNextArtOrThread },
	{ txt_help_global_post, iKeyPost },
	{ txt_help_global_post_postponed, iKeyPostponed2 },
	{ txt_help_global_post_postponed, iKeyPostponed },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_article_range, iKeySetRange },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_mail, iKeyThreadMail },
	{ txt_help_global_save, iKeyThreadSave },
	{ txt_help_global_save_tagged, iKeyThreadAutoSaveTagged },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_tag, iKeyThreadTag },
	{ txt_help_group_untag_thread, iKeyThreadUntag },
	{ txt_help_empty_line, 0 },
	{ txt_help_thread_mark_article_read, iKeyThreadMarkArtRead },
	{ txt_help_thread_catchup, iKeyThreadCatchup },
	{ txt_help_thread_catchup_next_unread, iKeyThreadCatchupNextUnread },
	{ txt_help_group_mark_article_unread, iKeyGroupMarkArtUnread },
	{ txt_help_group_mark_thread_unread, iKeyGroupMarkThdUnread },
	{ txt_help_empty_line, 0 },
	{ txt_help_group_select_thread, iKeyThreadSelArt },
	{ txt_help_group_toggle_thread_selection, iKeyThreadToggleArtSel },
	{ txt_help_group_reverse_thread_selection, iKeyThreadReverseSel },
	{ txt_help_group_undo_thread_selection, iKeyThreadUndoSel },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_misc, 0 },
	{ txt_help_global_previous_menu, iKeyQuit },
	{ txt_help_global_quit_tin, iKeyQuitTin },
	{ txt_help_global_help, iKeyHelp },
	{ txt_help_global_toggle_mini_help, iKeyToggleHelpDisplay },
	{ txt_help_global_option_menu, iKeyOptionMenu },
	{ txt_help_global_esc, iKeyAbort },
	{ txt_help_global_redraw_screen, iKeyRedrawScr },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, iKeyShellEscape },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, iKeyDisplayPostHist },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_version, iKeyVersion },
	{ txt_help_bug_report, iKeyThreadBugReport },
	{ NULL, 0 }
};

static t_help_page page_help_page[] = {
	{ txt_help_title_navi, 0 },
	{ txt_help_global_page_down, iKeyPageDown },
	{ txt_help_global_page_down, iKeyPageDown2 },
	{ txt_help_global_page_down, iKeyPageDown3 },
	{ txt_help_global_page_up, iKeyPageUp2 },
	{ txt_help_global_page_up, iKeyPageUp },
	{ txt_help_global_page_up, iKeyPageUp3 },
	{ txt_help_global_line_down, iKeyDown2 },
	{ txt_help_global_line_down, iKeyDown },
	{ txt_help_global_line_up, iKeyUp2 },
	{ txt_help_global_line_up, iKeyUp },
	{ txt_help_article_first_page, iKeyFirstPage },
	{ txt_help_article_last_page, iKeyLastPage },
	{ txt_help_article_first_page, iKeyPageFirstPage2 },
	{ txt_help_article_last_page, iKeyPageLastPage2 },
	{ txt_help_empty_line, 0 },
	{ txt_help_article_by_num, 0 },
	{ txt_help_article_next_thread, iKeyPageNextThd },
	{ txt_help_article_read_next_unread, iKeyPageNextUnread },
	{ txt_help_article_next, iKeyPageNextArt },
	{ txt_help_article_next_unread, iKeyPageNextUnreadArt },
	{ txt_help_article_prev, iKeyPagePrevArt },
	{ txt_help_article_prev_unread, iKeyPagePrevUnreadArt },
	{ txt_help_article_first_in_thread, iKeyPageTopThd },
	{ txt_help_article_last_in_thread, iKeyPageBotThd },
	{ txt_help_global_last_art, iKeyLastViewed },
	{ txt_help_group_list_thread, iKeyPageListThd },
	{ txt_help_article_parent, iKeyPageGotoParent },
	{ txt_help_global_lookup_art, iKeyLookupMessage },
	{ txt_help_article_quit_to_select_level, iKeyPageGroupSel },
	{ txt_help_article_skip_quote, iKeyPageSkipIncludedText },
	{ txt_help_empty_line, 0 },
	{ txt_help_article_search_forwards, iKeySearchSubjF },
	{ txt_help_article_search_backwards, iKeySearchSubjB },
	{ txt_help_global_search_auth_forwards, iKeySearchAuthF },
	{ txt_help_global_search_auth_backwards, iKeySearchAuthB },
	{ txt_help_global_search_body, iKeySearchBody },
	{ txt_help_global_search_body_comment, 0 },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_disp, 0 },
	{ txt_help_global_toggle_info_line, iKeyToggleInfoLastLine },
	{ txt_help_article_toggle_rot13, iKeyPageToggleRot },
	{ txt_help_global_toggle_inverse_video, iKeyToggleInverseVideo },
	{ txt_help_article_show_raw, iKeyPageToggleHeaders },
#ifdef HAVE_COLOR
	{ txt_help_global_toggle_color, iKeyToggleColor },
	{ txt_help_article_toggle_highlight, iKeyPageToggleHighlight },
#endif /* HAVE_COLOR */
	{ txt_help_article_toggle_tex2iso, iKeyPageToggleTex2iso },
	{ txt_help_article_toggle_tabwidth, iKeyPageToggleTabs },
	{ txt_help_article_toggle_uue, iKeyPageToggleUue },
	{ txt_help_article_toggle_formfeed, iKeyPageReveal },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_ops, 0 },
	{ txt_help_global_post, iKeyPost },
	{ txt_help_global_post_postponed, iKeyPostponed2 },
	{ txt_help_global_post_postponed, iKeyPostponed },
	{ txt_help_article_followup, iKeyPageFollowupQuote },
	{ txt_help_article_followup_no_quote, iKeyPageFollowup },
	{ txt_help_article_followup_with_header, iKeyPageFollowupQuoteHeaders },
	{ txt_help_article_repost, iKeyPageRepost },
	{ txt_help_article_reply, iKeyPageReplyQuote },
	{ txt_help_article_reply_no_quote, iKeyPageReply },
	{ txt_help_article_reply_with_header, iKeyPageReplyQuoteHeaders },
	{ txt_help_article_edit, iKeyPageEditArticle },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_mail, iKeyPageMail },
	{ txt_help_global_save, iKeyPageSave },
	{ txt_help_global_save_tagged, iKeyPageAutoSaveTagged },
#ifndef DONT_HAVE_PIPING
	{ txt_help_global_pipe, iKeyPipe },
#endif /* !DONT_HAVE_PIPING */
#ifndef DISABLE_PRINTING
	{ txt_help_global_print, iKeyPrint },
#endif /* !DISABLE_PRINTING */
	{ txt_help_article_view_attachments, iKeyPageViewAttach },
	{ txt_help_empty_line, 0 },
	{ txt_help_global_tag, iKeyPageTag },
	{ txt_help_empty_line, 0 },
	{ txt_help_article_mark_thread_read, iKeyPageKillThd },
	{ txt_help_group_catchup, iKeyPageCatchup },
	{ txt_help_group_catchup_next, iKeyPageCatchupNextUnread },
	{ txt_help_group_mark_article_unread, iKeyGroupMarkArtUnread },
	{ txt_help_group_mark_thread_unread, iKeyGroupMarkThdUnread },
	{ txt_help_empty_line, 0 },
	{ txt_help_article_autoselect, iKeyPageAutoSel },
	{ txt_help_article_autokill, iKeyPageAutoKill },
	{ txt_help_article_quick_select, iKeyPageQuickAutoSel },
	{ txt_help_article_quick_kill, iKeyPageQuickKill },
	{ txt_help_article_cancel, iKeyPageCancel },
	{ txt_help_empty_line, 0 },
	{ txt_help_title_misc, 0 },
	{ txt_help_article_browse_urls, iKeyPageViewUrl },
	{ txt_help_global_previous_menu, iKeyQuit },
	{ txt_help_global_quit_tin, iKeyQuitTin },
	{ txt_help_global_help, iKeyHelp },
	{ txt_help_global_toggle_mini_help, iKeyToggleHelpDisplay },
	{ txt_help_global_option_menu, iKeyOptionMenu },
	{ txt_help_global_esc, iKeyAbort },
	{ txt_help_global_redraw_screen, iKeyRedrawScr },
#ifndef NO_SHELL_ESCAPE
	{ txt_help_global_shell_escape, iKeyShellEscape },
#endif /* !NO_SHELL_ESCAPE */
	{ txt_help_global_posting_history, iKeyDisplayPostHist },
#ifdef HAVE_PGP_GPG
	{ txt_help_empty_line, 0 },
	{ txt_help_article_pgp, iKeyPagePGPCheckArticle },
#endif /* HAVE_PGP_GPG */
	{ txt_help_empty_line, 0 },
	{ txt_help_global_version, iKeyVersion },
	{ NULL, 0 }
};


static void
make_help_page (
	FILE *fp,
	const t_help_page *helppage,
	const t_menukeys *menukeys)
{
	char buf[LEN];
	char helpline[LEN];
	char key[MAXKEYLEN];

	if (!helppage)
		return;

	while (helppage->helptext) {
		if (helppage->key == 0) {
			if (strlen(helppage->helptext))
				snprintf (buf, sizeof(buf), "%s", _(helppage->helptext));
			else /* avoid zero length translations */
				snprintf (buf, sizeof(buf), "%s", helppage->helptext);
		} else
			snprintf (buf, sizeof(buf), "%s\t  %s",
				printascii (key, map_to_local (helppage->key, menukeys)),
				_(helppage->helptext));
		buf[sizeof(buf) - 1] = '\0';
		expand_ctrl_chars (helpline, buf, sizeof(helpline), 8);
		fprintf (fp, "%s\n", helpline);
		helppage++;
	}
}

void
show_help_page (
	const int level,
	const char *title)
{
	FILE *fp;

	if (!(fp = tmpfile ()))
		return;

	switch (level) {
		case SELECT_LEVEL:
			make_help_page (fp, select_help_page, &menukeymap.select_nav);
			break;

		case GROUP_LEVEL:
			make_help_page (fp, group_help_page, &menukeymap.group_nav);
			break;

		case THREAD_LEVEL:
			make_help_page (fp, thread_help_page, &menukeymap.thread_nav);
			break;

		case PAGE_LEVEL:
			make_help_page (fp, page_help_page, &menukeymap.page_nav);
			break;

		case INFO_PAGER:
		default: /* should not happen */
			error_message (_(txt_error_unknown_dlevel));
			fclose (fp);
			return;
	}

	info_pager (fp, title, TRUE);
	fclose (fp);
	return;
}

void
show_mini_help (
	int level)
{
	char buf[LEN];
	char key[20][MAXKEYLEN];
	int line;
	size_t bufs;

	if (!tinrc.beginner_level)
		return;

	line = NOTESLINES + (MINI_HELP_LINES - 2);
	bufs = (size_t) MIN((unsigned)cCOLS, (sizeof(buf) - 1));

#ifdef HAVE_COLOR
	fcol(tinrc.col_minihelp);
#endif /* HAVE_COLOR */

	switch (level) {
		case SELECT_LEVEL:
			snprintf (buf, bufs, _(txt_mini_select_1),
				printascii (key[0], map_to_local (iKeySelectEnterNextUnreadGrp, &menukeymap.select_nav)),
				printascii (key[1], map_to_local (iKeySelectGoto, &menukeymap.select_nav)),
				printascii (key[2], map_to_local (iKeySearchSubjF, &menukeymap.select_nav)),
				printascii (key[3], map_to_local (iKeySelectCatchup, &menukeymap.select_nav)));
			center_line (line, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_select_2),
				printascii (key[0], map_to_local (iKeyDown2, &menukeymap.select_nav)),
				printascii (key[1], map_to_local (iKeyUp2, &menukeymap.select_nav)),
				printascii (key[2], map_to_local (iKeyHelp, &menukeymap.select_nav)),
				printascii (key[3], map_to_local (iKeySelectMoveGrp, &menukeymap.select_nav)),
				printascii (key[4], map_to_local (iKeyQuit, &menukeymap.select_nav)),
				printascii (key[5], map_to_local (iKeySelectToggleReadDisplay, &menukeymap.select_nav)));
			center_line (line + 1, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_select_3),
				printascii (key[0], map_to_local (iKeySelectSubscribe, &menukeymap.select_nav)),
				printascii (key[1], map_to_local (iKeySelectSubscribePat, &menukeymap.select_nav)),
				printascii (key[2], map_to_local (iKeySelectUnsubscribe, &menukeymap.select_nav)),
				printascii (key[3], map_to_local (iKeySelectUnsubscribePat, &menukeymap.select_nav)),
				printascii (key[4], map_to_local (iKeySelectYankActive, &menukeymap.select_nav)));
			center_line (line + 2, FALSE, buf);
			break;
		case GROUP_LEVEL:
			snprintf (buf, bufs, _(txt_mini_group_1),
				printascii (key[0], map_to_local (iKeyGroupNextUnreadArtOrGrp, &menukeymap.group_nav)),
				printascii (key[1], map_to_local (iKeySearchSubjF, &menukeymap.group_nav)),
				printascii (key[2], map_to_local (iKeyGroupKill, &menukeymap.group_nav)));
			center_line (line, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_group_2),
				printascii (key[0], map_to_local (iKeySearchAuthF, &menukeymap.group_nav)),
				printascii (key[1], map_to_local (iKeyGroupCatchup, &menukeymap.group_nav)),
				printascii (key[2], map_to_local (iKeyDown2, &menukeymap.group_nav)),
				printascii (key[3], map_to_local (iKeyUp2, &menukeymap.group_nav)),
				printascii (key[4], map_to_local (iKeyGroupMarkThdRead, &menukeymap.group_nav)),
				printascii (key[5], map_to_local (iKeyGroupListThd, &menukeymap.group_nav)));
			center_line (line + 1, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_group_3),
#ifndef DONT_HAVE_PIPING
				printascii (key[0], map_to_local (iKeyPipe, &menukeymap.group_nav)),
#endif /* !DONT_HAVE_PIPING */
				printascii (key[1], map_to_local (iKeyGroupMail, &menukeymap.group_nav)),
#ifndef DISABLE_PRINTING
				printascii (key[2], map_to_local (iKeyPrint, &menukeymap.group_nav)),
#endif /* !DISABLE_PRINTING */
				printascii (key[3], map_to_local (iKeyQuit, &menukeymap.group_nav)),
				printascii (key[4], map_to_local (iKeyGroupToggleReadUnread, &menukeymap.group_nav)),
				printascii (key[5], map_to_local (iKeyGroupSave, &menukeymap.group_nav)),
				printascii (key[6], map_to_local (iKeyGroupTag, &menukeymap.group_nav)),
				printascii (key[7], map_to_local (iKeyPost, &menukeymap.group_nav)));
			center_line (line + 2, FALSE, buf);
			break;
		case THREAD_LEVEL:
			snprintf (buf, bufs, _(txt_mini_thread_1),
				printascii (key[0], map_to_local (iKeyThreadReadNextArtOrThread, &menukeymap.thread_nav)),
				printascii (key[1], map_to_local (iKeyThreadCatchup, &menukeymap.thread_nav)),
				printascii (key[2], map_to_local (iKeyThreadToggleSubjDisplay, &menukeymap.thread_nav)));
			center_line (line, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_thread_2),
				printascii (key[0], map_to_local (iKeyHelp, &menukeymap.thread_nav)),
				printascii (key[1], map_to_local (iKeyDown2, &menukeymap.thread_nav)),
				printascii (key[2], map_to_local (iKeyUp2, &menukeymap.thread_nav)),
				printascii (key[3], map_to_local (iKeyQuit, &menukeymap.thread_nav)),
				printascii (key[4], map_to_local (iKeyThreadTag, &menukeymap.thread_nav)),
				printascii (key[5], map_to_local (iKeyThreadMarkArtUnread, &menukeymap.thread_nav)));
			center_line (line + 1, FALSE, buf);
			break;
		case PAGE_LEVEL:
			snprintf (buf, bufs, _(txt_mini_page_1),
				printascii (key[0], map_to_local (iKeyPageNextUnread, &menukeymap.page_nav)),
				printascii (key[1], map_to_local (iKeySearchSubjF, &menukeymap.page_nav)),
				printascii (key[2], map_to_local (iKeyPageAutoKill, &menukeymap.page_nav)));
			center_line (line, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_page_2),
				printascii (key[0], map_to_local (iKeySearchAuthF, &menukeymap.page_nav)),
				printascii (key[1], map_to_local (iKeySearchBody, &menukeymap.page_nav)),
				printascii (key[2], map_to_local (iKeyPageCatchup, &menukeymap.page_nav)),
				printascii (key[3], map_to_local (iKeyPageFollowupQuote, &menukeymap.page_nav)),
				printascii (key[4], map_to_local (iKeyPageKillThd, &menukeymap.page_nav)));
			center_line (line + 1, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_page_3),
#ifndef DONT_HAVE_PIPING
				printascii (key[0], map_to_local (iKeyPipe, &menukeymap.page_nav)),
#endif /* !DONT_HAVE_PIPING */
				printascii (key[1], map_to_local (iKeyPageMail, &menukeymap.page_nav)),
#ifndef DISABLE_PRINTING
				printascii (key[2], map_to_local (iKeyPrint, &menukeymap.page_nav)),
#endif /* !DISABLE_PRINTING */
				printascii (key[3], map_to_local (iKeyQuit, &menukeymap.page_nav)),
				printascii (key[4], map_to_local (iKeyPageReplyQuote, &menukeymap.page_nav)),
				printascii (key[5], map_to_local (iKeyPageSave, &menukeymap.page_nav)),
				printascii (key[6], map_to_local (iKeyPageTag, &menukeymap.page_nav)),
				printascii (key[7], map_to_local (iKeyPost, &menukeymap.page_nav)));
			center_line (line + 2, FALSE, buf);
			break;

		case INFO_PAGER:
			snprintf (buf, bufs, _(txt_mini_info_1),
				printascii (key[0], map_to_local (iKeyUp, &menukeymap.info_nav)),
				printascii (key[1], map_to_local (iKeyDown, &menukeymap.info_nav)),
				printascii (key[2], map_to_local (iKeyPageUp, &menukeymap.info_nav)),
				printascii (key[3], map_to_local (iKeyPageDown, &menukeymap.info_nav)),
				printascii (key[4], map_to_local (iKeyFirstPage, &menukeymap.info_nav)),
				printascii (key[5], map_to_local (iKeyLastPage, &menukeymap.info_nav)));
			center_line (line, FALSE, buf);
			snprintf (buf, bufs, _(txt_mini_info_2),
				printascii (key[0], map_to_local (iKeySearchSubjF, &menukeymap.info_nav)),
				printascii (key[1], map_to_local (iKeySearchSubjB, &menukeymap.info_nav)),
				printascii (key[2], map_to_local (iKeyQuit, &menukeymap.info_nav)));
			center_line (line + 1, FALSE, buf);
			break;

		default: /* should not happen */
			error_message (_(txt_error_unknown_dlevel));
			break;
	}
#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
}


void
toggle_mini_help (
	int level)
{
	tinrc.beginner_level = bool_not(tinrc.beginner_level);
	set_noteslines (cLINES);
	show_mini_help (level);
}
