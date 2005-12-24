/*
 *  Project   : tin - a Usenet reader
 *  Module    : lang.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2005-10-19
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

constext txt_1_resp[] = N_("1 Response");
constext txt_7bit[] = "7bit";
constext txt_8bit[] = "8bit";

constext txt_active_file_is_empty[] = N_("\n%s contains no newsgroups. Exiting.");
constext txt_all[] = N_("all ");
constext txt_all_groups[] = N_("All groups");
constext txt_append_overwrite_quit[] = N_("File %s exists. %s=append, %s=overwrite, %s=quit: ");
constext txt_art_cancel[] = N_("Article cancelled (deleted).");
constext txt_art_cannot_cancel[] = N_("Article cannot be cancelled (deleted).");
constext txt_art_deleted[] = N_("Article deleted.");
constext txt_art_newsgroups[] = N_("\nYour article:\n  \"%s\"\nwill be posted to the following %s:\n");
constext txt_art_not_posted[] = N_("Article not posted!");
constext txt_art_not_saved[] = N_("Article not saved");
constext txt_art_pager_com[] = N_("Article Level Commands");
constext txt_art_parent_none[] = N_("Article has no parent");
constext txt_art_parent_killed[] = N_("Parent article has been killed");
constext txt_art_parent_unavail[] = N_("Parent article is unavailable");
constext txt_art_posted[] = N_("Article posted: %s");
constext txt_art_rejected[] = N_("Article rejected (saved to %s)");
constext txt_art_thread_regex_tag[] = N_("%s=article, %s=thread, %s=hot, %s=pattern, %s=tagged articles, %s=quit: ");
constext txt_art_unavailable[] = N_("Article unavailable");
constext txt_art_undeleted[] = N_("Article undeleted.");
constext txt_article_cancelled[] = "Article cancelled by author.\n";
constext txt_article_plural[] = N_("articles");
constext txt_article_reposted[] = N_("This is a repost of the following article:");
constext txt_article_singular[] = N_("article");
constext txt_article_upper[] = N_("Article");
constext txt_articles_mailed[] = N_("-- %d %s mailed --");
constext txt_at_s[] = N_(" at %s");
constext txt_attach[] = N_("%*s[-- %s/%s, encoding %s%s%s, %d lines%s%s --]\n");
constext txt_attach_charset[] = N_(", charset: ");
constext txt_attach_description[] = N_("%*s[-- Description: %s --]\n");
constext txt_auth_failed[] = N_("%d Authentication failed");
constext txt_auth_needed[] = N_("Server expects authentication.\n");
constext txt_auth_pass[] = N_("    Please enter password: ");
constext txt_auth_user[] = N_("    Please enter username: ");
constext txt_author_search_backwards[] = N_("Author search backwards [%s]> ");
constext txt_author_search_forwards[] = N_("Author search forwards [%s]> ");
constext txt_authorization_ok[] = N_("Authorized for user: %s\n");
constext txt_authorization_fail[] = N_("Authorization failed for user: %s\n");
constext txt_autosubscribed[] = N_("\nAutosubscribed to %s");
constext txt_autosubscribing_groups[] = N_("Autosubscribing groups...\n");
constext txt_autoselecting_articles[] = N_("Autoselecting articles (use '%s' to see all unread) ...");

constext txt_bad_active_file[] = N_("Active file corrupt - %s");
constext txt_bad_article[] = N_("Article to be posted resulted in errors/warnings. %s=quit, %s=Menu, %s=edit: ");
constext txt_bad_attrib[] = N_("Unrecognized attribute: %s");
constext txt_bad_command[] = N_("Bad command. Type '%s' for help.");
constext txt_base64[] = "base64";
constext txt_base_article[] = N_("Base article");
constext txt_base_article_range[] = N_("Base article range");
constext txt_batch_update_unavail[] = N_("%s: Updating of index files not supported\n");
constext txt_begin_of_art[] = N_("*** Beginning of article ***");

constext txt_cancel_article[] = N_("Cancel (delete) or supersede (overwrite) article [%%s]? (%s/%s/%s): ");
constext txt_cancelling_art[] = N_("Cancelling article...");
constext txt_cannot_create_uniq_name[] = "Can't create unique tempfile-name";
constext txt_cannot_create[] = N_("Cannot create %s");
constext txt_cannot_find_base_art[] = N_("Can't find base article %d");
constext txt_cannot_open[] = N_("Can't open %s");
constext txt_cannot_open_for_saving[] = N_("Couldn't open %s for saving");
constext txt_cannot_post[] = N_("*** Posting not allowed ***");
constext txt_cannot_post_group[] = N_("Posting is not allowed to %s");
constext txt_cannot_retrieve[] = N_("Can't retrieve %s");
constext txt_cannot_write_to_directory[] = N_("%s is a directory");
constext txt_catchup[] = N_("Catchup");
constext txt_catchup_group[] = N_("Catchup %s...");
constext txt_catchup_all_read_groups[] = N_("Catchup all groups entered during this session?");
constext txt_catchup_despite_tags[] = N_("You have tagged articles in this group - catchup anyway?");
constext txt_catchup_update_info[] = N_("%s %d %s in %lu seconds\n");
constext txt_caughtup[] = N_("Caughtup");
constext txt_check_article[] = N_("Check Prepared Article");
constext txt_checking_new_groups[] = N_("Checking for new groups... ");
constext txt_checking_for_news[] = N_("Checking for news...");
constext txt_choose_post_process_type[] = N_("Post-process %s=no, %s=yes, %s=shar, %s=quit: ");
#ifdef HAVE_COLOR
	constext txt_color_off[] = N_("ANSI color disabled");
	constext txt_color_on[] = N_("ANSI color enabled");
#endif /* HAVE_COLOR */
constext txt_command_failed[] = N_("Command failed: %s");
constext txt_confirm_select_on_exit[] = N_("Mark not selected articles read?");
constext txt_connecting[] = N_("Connecting to %s...");
constext txt_cr[] = N_("<CR>");
constext txt_creating_active[] = N_("Creating active file for saved groups...\n");
constext txt_creating_newsrc[] = N_("Creating newsrc file...\n");

constext txt_default[] = N_("Default");
constext txt_delete_processed_files[] = N_("Delete saved files that have been post processed?");
constext txt_deleting[] = N_("Deleting temporary files...");

constext txt_end_of_art[] = N_("*** End of article ***");
constext txt_end_of_arts[] = N_("*** End of articles ***");
constext txt_end_of_groups[] = N_("*** End of groups ***");
constext txt_end_of_thread[] = N_("*** End of thread ***");
constext txt_enter_getart_limit[] = N_("Enter limit of articles to get> ");
constext txt_enter_message_id[] = N_("Enter Message-ID to go to> ");
constext txt_enter_next_thread[] = N_(" and enter next unread thread");
constext txt_enter_next_unread_group[] = N_(" and enter next unread group");
constext txt_enter_option_num[] = N_("Enter option number> ");
constext txt_enter_range[] = N_("Enter range [%s]> ");
constext txt_error_approved[] = N_("\nWarning: Approved: header used.\n");
constext txt_error_asfail[] = "%s: assertion failure: %s (%d): %s\n";
constext txt_error_bad_approved[] = N_("\nError: Bad address in Approved: header.\n");
constext txt_error_bad_from[] = N_("\nError: Bad address in From: header.\n");
constext txt_error_bad_replyto[] = N_("\nError: Bad address in Reply-To: header.\n");
constext txt_error_bad_msgidfqdn[] = N_("\nError: Bad FQDN in Message-ID: header.\n");
constext txt_error_cant_unlock[] = N_("Can't unlock %s");
constext txt_error_copy_fp[] = "copy_fp() failed";
constext txt_error_corrupted_file[] = N_("Corrupted file %s");
constext txt_error_couldnt_dotlock[] = N_("Couldn't dotlock %s - article not appended!");
constext txt_error_couldnt_lock[] = N_("Couldn't lock %s - article not appended!");
constext txt_error_fseek[] = "fseek() error on [%s]";
constext txt_error_gnksa_internal[] = N_("Internal error in GNKSA routine - send bug report.\n");
constext txt_error_gnksa_langle[] = N_("Left angle bracket missing in route address.\n");
constext txt_error_gnksa_lparen[] = N_("Left parenthesis missing in old-style address.\n");
constext txt_error_gnksa_rparen[] = N_("Right parenthesis missing in old-style address.\n");
constext txt_error_gnksa_atsign[] = N_("At-sign missing in mail address.\n");
constext txt_error_gnksa_sgl_domain[] = N_("Single component FQDN is not allowed. Add your domain.\n");
constext txt_error_gnksa_inv_domain[] = N_("Invalid domain. Send bug report if your top level domain really exists.\nUse .invalid as top level domain for munged addresses.\n");
constext txt_error_gnksa_ill_domain[] = N_("Illegal domain. Send bug report if your top level domain really exists.\nUse .invalid as top level domain for munged addresses.\n");
constext txt_error_gnksa_unk_domain[] = N_("Unknown domain. Send bug report if your top level domain really exists.\nUse .invalid as top level domain for munged addresses.\n");
constext txt_error_gnksa_fqdn[] = N_("Illegal character in FQDN.\n");
constext txt_error_gnksa_zero[] = N_("Zero length FQDN component not allowed.\n");
constext txt_error_gnksa_length[] = N_("FQDN component exceeds maximum allowed length (63 chars).\n");
constext txt_error_gnksa_hyphen[] = N_("FQDN component may not start or end with hyphen.\n");
constext txt_error_gnksa_begnum[] = N_("FQDN component may not start with digit.\n");
constext txt_error_gnksa_bad_lit[] = N_("Domain literal has impossible numeric value.\n");
constext txt_error_gnksa_local_lit[] = N_("Domain literal is for private use only and not allowed for global use.\n");
constext txt_error_gnksa_rbracket[] = N_("Right bracket missing in domain literal.\n");
constext txt_error_gnksa_lp_missing[] = N_("Missing localpart of mail address.\n");
constext txt_error_gnksa_lp_invalid[] = N_("Illegal character in localpart of mail address.\n");
constext txt_error_gnksa_lp_zero[] = N_("Zero length localpart component not allowed.\n");
constext txt_error_gnksa_rn_unq[] = N_("Illegal character in realname.\nUnquoted words may not contain '!()<>@,;:\\.[]' in route addresses.\n");
constext txt_error_gnksa_rn_qtd[] = N_("Illegal character in realname.\nQuoted words may not contain '()<>\\'.\n");
constext txt_error_gnksa_rn_enc[] = N_("Illegal character in realname.\nEncoded words may not contain '!()<>@,;:\"\\.[]/=' in parameter.\n");
constext txt_error_gnksa_rn_encsyn[] = N_("Bad syntax in encoded word used in realname.\n");
constext txt_error_gnksa_rn_paren[] = N_("Illegal character in realname.\nUnquoted words may not contain '()<>\\' in old-style addresses.\n");
constext txt_error_gnksa_rn_invalid[] = N_("Illegal character in realname.\nControl characters and unencoded 8bit characters > 127 are not allowed.\n");
constext txt_error_header_and_body_not_separate[] = N_("\nError: No blank line found after header.\n");
/* TODO: fixme, US-ASCII is not the only 7bit charset we know about */
constext txt_error_header_line_bad_charset[] = N_("\n\
Error: Posting contains non-ASCII characters but MM_CHARSET is set to\n\
       US-ASCII  - please change this setting to a suitable value for\n\
       your language  using the  M)enu of configurable  options or by\n\
       editing tinrc.\n");
constext txt_error_header_line_bad_encoding[] = N_("\n\
Error: Posting contains  non-ASCII characters  but the  MIME encoding\n\
       for news  messages  is set  to \"7bit\"  -  please change this\n\
       setting to \"8bit\" or \"quoted-printable\" depending  on what\n\
       is more common  in your part  of the world.  This can  be done\n\
       using the M)enu of configurable options or by editing tinrc.\n");
constext txt_error_header_line_blank[] = N_("\nError: Article starts with blank line instead of header\n");
constext txt_error_header_line_colon[] = N_("\nError: Header on line %d does not have a colon after the header name:\n%s\n");
constext txt_error_header_line_empty[] = N_("\nError: The \"%s:\" line is empty.\n");
constext txt_error_header_line_missing[] = N_("\nError: The \"%s:\" line is missing from the article header.\n");
constext txt_error_header_line_space[] = N_("\nError: Header on line %d does not have a space after the colon:\n%s\n");
constext txt_error_header_duplicate[] = N_("\nError: There are multiple (%d) \"%s:\" lines in the header.\n");
constext txt_error_insecure_permissions[] = N_("Insecure permissions of %s (%o)");
constext txt_error_invalid_response_to_group[] = N_("Invalid response to GROUP command, %s");
constext txt_error_locale[] = "Can't set the specified locale!";
constext txt_error_mime_end[] = N_("MIME parse error: Unexpected end of %s/%s article");
constext txt_error_mime_start[] = N_("MIME parse error: Start boundary whilst reading headers");
constext txt_error_no_domain_name[] = N_("Can't get a (fully-qualified) domain-name!");
constext txt_error_no_enter_permission[] = N_("No permissions to go into %s\n");
constext txt_error_no_from[] = N_("\nError: From: line missing.\n");
constext txt_error_no_read_permission[] = N_("No read permissions for %s\n");
constext txt_error_no_such_file[] = N_("File %s does not exist\n");
constext txt_error_no_write_permission[] = N_("No write permissions for %s\n");
constext txt_error_passwd_missing[] = N_("Can't get user information (/etc/passwd missing?)");
constext txt_error_plural[] = N_("errors");
constext txt_error_sender_in_header_not_allowed[] = N_("\nError on line %d: \"Sender:\" header not allowed (it will be added for you)\n");
constext txt_error_server_has_no_listed_groups[] = N_("Server has non of the groups listed in %s");
constext txt_error_singular[] = N_("error");
constext txt_error_unknown_dlevel[] = N_("Unknown display level");
constext txt_esc[] = N_("<ESC>");
constext txt_exiting[] = N_("Exiting...");
constext txt_external_mail_done[] = N_("leaving external mail-reader");
constext txt_extracting_shar[] = N_("Extracting %s...");

constext txt_filesystem_full[] = N_("Error writing %s file. Filesystem full? File reset to previous state.");
constext txt_filesystem_full_backup[] = N_("Error making backup of %s file. Filesystem full?");
constext txt_filter_global_rules[] = N_("Filtering global rules (%d/%d)...");
constext txt_filter_rule_created[] = N_("Rule created by: ");
constext txt_feed_pattern[] = N_("Enter pattern [%s]> ");
constext txt_followup_newsgroups[] = N_("\nYou requested followups to your article to go to the following %s:\n");
constext txt_followup_poster[] = N_("  %s\t Answers will be directed to you by mail.\n");
constext txt_forwarded[] = N_("-- forwarded message --\n");
constext txt_forwarded_end[] = N_("-- end of forwarded message --\n");
constext txt_filter_file[] = N_("# Format:\n\
#   comment=STRING    Optional. Multiple lines allowed. Comments must be placed\n\
#                     at the beginning of a rule, or they will be moved to the\n\
#                     next rule. '#' is not a valid keyword for a comment!\n\
#   group=STRING      Mandatory. Newsgroups list (e.g. comp.*,!*sources*).\n\
#   case=NUM          Mandatory. Compare=0 / ignore=1 case when filtering.\n\
#   score=NUM|STRING  Mandatory. Score to give. Either:\n\
#     score=NUM         A number (e.g. 70). Or:\n\
#     score=STRING      One of the two keywords: 'hot' or 'kill'.\n\
#   subj=STRING       Optional. Subject: line (e.g. How to be a wizard).\n\
#   from=STRING       Optional. From: line (e.g. *Craig Shergold*).\n\
#   msgid=STRING      Optional. Message-ID: line (e.g. <123@ether.net>) with\n\
#                     full references.\n\
#   msgid_last=STRING Optional. Like above, but with last reference only.\n\
#   msgid_only=STRING Optional. Like above, but without references.\n\
#   refs_only=STRING  Optional. References: line (e.g. <123@ether.net>) without\n\
#                     Message-ID:\n\
#   lines=[<>]?NUM    Optional. Lines: line. '<' or '>' are optional.\n\
#   gnksa=[<>]?NUM    Optional. GNKSA parse_from() return code. '<' or '>' opt.\n\
#   xref=PATTERN      Optional. Kill pattern (e.g. alt.flame*)\n\
#   time=NUM          Optional. time_t value when rule expires\n#\n");
constext txt_filter_score[] = N_("Enter score for rule (default=%d): ");
constext txt_filter_score_help[] = N_("Enter the score weight (range 0 < score <= %d)"); /* SCORE_MAX */
constext txt_full[] = N_("Full");
constext txt_filter_comment[] = N_("Comment (optional)  : ");
constext txt_filter_text_type[] = N_("Apply pattern to    : ");
constext txt_from_line_only[] = N_("From: line (ignore case)      ");
constext txt_from_line_only_case[] = N_("From: line (case sensitive)   ");

constext txt_gethostbyname[] = N_("%s%s: Unknown host.\n");
constext txt_global[] = N_("global ");
constext txt_group_aliased[] = N_("Please use %.100s instead");
constext txt_group_bogus[] = N_("%s is bogus");
constext txt_group_is_moderated[] = N_("Group %s is moderated. Continue?");
constext txt_group_plural[] = N_("groups");
constext txt_group_rereading[] = N_("Rereading %s...");
constext txt_group_select_com[] = N_("Top Level Commands");
constext txt_group_selection[] = N_("Group Selection");
constext txt_group_singular[] = N_("group");

constext txt_help_filter_comment[] = N_("One or more lines of comment. <CR> to add a line or proceed if line is empty.");
constext txt_help_filter_from[] = N_("From: line to add to filter file. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_lines[] = N_("Linecount of articles to be filtered. < for less, > for more, = for equal.");
constext txt_help_filter_msgid[] = N_("Message-ID: line to add to filter file. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_subj[] = N_("Subject: line to add to filter file. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_text[] = N_("Enter text pattern to filter if Subject: & From: lines are not what you want.");
constext txt_help_filter_text_type[] = N_("Select where text pattern should be applied. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_time[] = N_("Expiration time in days for the entered filter. <SPACE> toggles & <CR> sets.");
constext txt_help_kill_scope[] = N_("Apply kill only to current group or all groups. <SPACE> toggles & <CR> sets.");
constext txt_help_select_scope[] = N_("Apply select to current group or all groups. <SPACE> toggles & <CR> sets.");
constext txt_help_article_autokill[] = N_("kill an article via a menu");
constext txt_help_article_autoselect[] = N_("auto-select (hot) an article via a menu");
constext txt_help_article_browse_urls[] = N_("Browse URLs in article");
constext txt_help_article_by_num[] = N_("0 - 9\t  display article by number in current thread");
constext txt_help_article_cancel[] = N_("cancel (delete) or supersede (overwrite) current article");
constext txt_help_article_edit[] = N_("edit article (mail-groups only)");
constext txt_help_article_first_in_thread[] = N_("display first article in current thread");
constext txt_help_article_first_page[] = N_("display first page of article");
constext txt_help_article_followup[] = N_("post followup to current article");
constext txt_help_article_followup_no_quote[] = N_("post followup (don't copy text) to current article");
constext txt_help_article_followup_with_header[] = N_("post followup to current article quoting complete headers");
constext txt_help_article_last_in_thread[] = N_("display last article in current thread");
constext txt_help_article_last_page[] = N_("display last page of article");
constext txt_help_article_mark_thread_read[] = N_("mark rest of thread as read and advance to next unread");
constext txt_help_article_next[] = N_("display next article");
constext txt_help_article_next_thread[] = N_("display first article in next thread");
constext txt_help_article_next_unread[] = N_("display next unread article");
constext txt_help_article_parent[] = N_("go to the article that this one followed up");
constext txt_help_article_prev[] = N_("display previous article");
constext txt_help_article_prev_unread[] = N_("display previous unread article");
constext txt_help_article_quick_kill[] = N_("quickly kill an article using defaults");
constext txt_help_article_quick_select[] = N_("quickly auto-select (hot) an article using defaults");
constext txt_help_article_quit_to_select_level[] = N_("return to group selection level");
constext txt_help_article_reply[] = N_("reply through mail to author");
constext txt_help_article_reply_no_quote[] = N_("reply through mail (don't copy text) to author");
constext txt_help_article_reply_with_header[] = N_("reply through mail to author quoting complete headers");
constext txt_help_article_repost[] = N_("repost chosen article to another group");
constext txt_help_article_search_backwards[] = N_("search backwards within this article");
constext txt_help_article_search_forwards[] = N_("search forwards within this article");
constext txt_help_article_show_raw[] = N_("show article in raw-mode (including all headers)");
constext txt_help_article_skip_quote[] = N_("skip next block of included text");
constext txt_help_article_toggle_formfeed[] = N_("toggle display of sections hidden by a form-feed (^L) on/off");
constext txt_help_article_toggle_highlight[] = N_("toggle word highlighting on/off");
constext txt_help_article_toggle_rot13[] = N_("toggle ROT-13 (basic decode) for current article");
constext txt_help_article_toggle_tabwidth[] = N_("toggle tabwidth 4 <-> 8");
constext txt_help_article_toggle_tex2iso[] = N_("toggle german TeX style decoding for current article");
constext txt_help_article_toggle_uue[] = N_("toggle display of uuencoded sections");
constext txt_help_article_view_attachments[] = N_("View/save multimedia attachments");
constext txt_help_bug[] = N_("report bug or comment via mail to %s");
constext txt_help_global_article_range[] = N_("choose range of articles to be affected by next command");
constext txt_help_global_esc[] = N_("escape from command prompt");
constext txt_help_global_edit_filter[] = N_("edit filter file");
constext txt_help_global_help[] = N_("get help");
constext txt_help_global_last_art[] = N_("display last article viewed");
constext txt_help_global_line_down[] = N_("down one line");
constext txt_help_global_line_up[] = N_("up one line");
constext txt_help_global_lookup_art[] = N_("go to article chosen by Message-ID");
constext txt_help_global_mail[] = N_("mail article/thread/hot/pattern/tagged articles to someone");
constext txt_help_global_option_menu[] = N_("menu of configurable options");
constext txt_help_global_page_down[] = N_("down one page");
constext txt_help_global_page_up[] = N_("up one page");
constext txt_help_global_post[] = N_("post (write) article to current group");
constext txt_help_global_post_postponed[] = N_("post postponed articles");
constext txt_help_global_posting_history[] = N_("list articles posted by you (from posted file)");
constext txt_help_global_previous_menu[] = N_("return to previous menu");
constext txt_help_global_quit_tin[] = N_("quit tin immediately");
constext txt_help_global_redraw_screen[] = N_("redraw page");
constext txt_help_global_save[] = N_("save article/thread/hot/pattern/tagged articles to file");
constext txt_help_global_auto_save[] = N_("save marked articles automatically without user prompts");
constext txt_help_global_scroll_down[] = N_("scroll the screen one line down");
constext txt_help_global_scroll_up[] = N_("scroll the screen one line up");
constext txt_help_global_search_auth_backwards[] = N_("search for articles by author backwards");
constext txt_help_global_search_auth_forwards[] = N_("search for articles by author forwards");
constext txt_help_global_search_body[] = N_("search all articles for a given string (this may take some time)");
constext txt_help_global_search_body_comment[] = N_(" \t  (searches are case-insensitive and wrap around to all articles)");
constext txt_help_global_search_subj_backwards[] = N_("search for articles by Subject line backwards");
constext txt_help_global_search_subj_forwards[] = N_("search for articles by Subject line forwards");
constext txt_help_global_search_repeat[] = N_("repeat last search");
constext txt_help_global_tag[] = N_("tag current article for reposting/mailing/piping/printing/saving");
constext txt_help_global_toggle_info_line[] = N_("toggle info message in last line (subject/description)");
constext txt_help_global_toggle_inverse_video[] = N_("toggle inverse video");
constext txt_help_global_toggle_mini_help[] = N_("toggle mini help menu display");
constext txt_help_global_toggle_subj_display[] = N_("cycle the display of authors email address, real name, both or neither");
constext txt_help_global_version[] = N_("show version information");
constext txt_help_group_catchup[] = N_("mark all articles as read and return to group selection menu");
constext txt_help_group_catchup_next[] = N_("mark all articles as read and enter next group with unread articles");
constext txt_help_group_first_thread[] = N_("choose first thread in list");
constext txt_help_group_last_thread[] = N_("choose last thread in list");
constext txt_help_group_list_thread[] = N_("list articles within current thread (bring up Thread sub-menu)");
constext txt_help_group_mark_article_unread[] = N_("mark article as unread");
constext txt_help_group_mark_thread_read[] = N_("mark current thread or tagged threads as read");
constext txt_help_group_mark_thread_unread[] = N_("mark thread as unread");
constext txt_help_group_mark_unsel_art_read[] = N_("toggle display of all/selected articles");
constext txt_help_group_next[] = N_("display next group");
constext txt_help_group_prev[] = N_("display previous group");
constext txt_help_group_reverse_thread_selection[] = N_("toggle all selections (all articles)");
constext txt_help_group_select_all[] = N_("select group (make \"hot\")");
constext txt_help_group_select_thread[] = N_("select thread");
constext txt_help_group_select_thread_if_unread_selected[] = N_("select threads if at least one unread article is selected");
constext txt_help_group_select_thread_pattern[] = N_("select threads that match user specified pattern");
constext txt_help_group_tag_parts[] = N_("tag all parts of current multipart-message in order");
constext txt_help_group_thread_by_num[] = N_("0 - 9\t  choose thread by number");
constext txt_help_group_toggle_getart_limit[] = N_("toggle limit number of articles to get, and reload");
constext txt_help_group_toggle_read_articles[] = N_("toggle display of all/unread articles");
constext txt_help_group_toggle_thread_selection[] = N_("toggle selection of thread");
constext txt_help_group_toggle_threading[] = N_("cycle through threading options available");
constext txt_help_group_undo_thread_selection[] = N_("undo all selections (all articles)");
constext txt_help_group_untag_thread[] = N_("untag all tagged threads");
constext txt_help_select_catchup[] = N_("mark all articles in group as read");
constext txt_help_select_catchup_next_unread[] = N_("mark all articles in group as read and move to next unread group");
constext txt_help_select_first_group[] = N_("choose first group in list");
constext txt_help_select_goto_group[] = N_("choose group by name");
constext txt_help_select_group_by_num[] = N_("0 - 9\t  choose group by number");
constext txt_help_select_group_range[] = N_("choose range of groups to be affected by next command");
constext txt_help_select_last_group[] = N_("choose last group in list");
constext txt_help_select_mark_group_unread[] = N_("mark all articles in chosen group unread");
constext txt_help_select_move_group[] = N_("move chosen group within list");
constext txt_help_select_next_unread_group[] = N_("choose next group with unread news");
constext txt_help_select_quit[] = N_("quit");
constext txt_help_select_quit_no_write[] = N_("quit without saving configuration changes");
constext txt_help_select_read_group[] = N_("read chosen group");
constext txt_help_select_reset_newsrc[] = N_("reset .newsrc (all available articles in groups marked unread)");
constext txt_help_select_search_group_backwards[] = N_("search backwards for a group name");
constext txt_help_select_search_group_comment[] = N_(" \t  (all searches are case-insensitive and wrap around)");
constext txt_help_select_search_group_forwards[] = N_("search forwards for a group name");
constext txt_help_select_subscribe[] = N_("subscribe to chosen group");
constext txt_help_select_subscribe_pattern[] = N_("subscribe to groups that match pattern");
constext txt_help_select_sync_with_active[] = N_("reread active file to check for any new news");
constext txt_help_select_toggle_descriptions[] = N_("toggle display of group name only or group name plus description");
constext txt_help_select_toggle_read_groups[] = N_("toggle display to show all/unread subscribed groups");
constext txt_help_select_unsubscribe[] = N_("unsubscribe from chosen group");
constext txt_help_select_unsubscribe_pattern[] = N_("unsubscribe from groups that match pattern");
constext txt_help_select_sort_active[] = N_("sort the list of groups");
constext txt_help_select_yank_active[] = N_("toggle display to show all/subscribed groups");
constext txt_help_thread_article_by_num[] = N_("0 - 9\t  choose article by number");
constext txt_help_thread_catchup[] = N_("mark thread as read and return to group index page");
constext txt_help_thread_catchup_next_unread[] = N_("mark thread as read and enter next unread thread or group");
constext txt_help_thread_first_article[] = N_("choose first article in list");
constext txt_help_thread_last_article[] = N_("choose last article in list");
constext txt_help_thread_mark_article_read[] = N_("mark article or tagged articles as read and move cursor to next unread article");
constext txt_help_thread_read_article[] = N_("read chosen article");
constext txt_help_title_disp[] = N_("Display properties\n------------------");
constext txt_help_title_misc[] = N_("Miscellaneous\n-------------");
constext txt_help_title_navi[] = N_("Moving around\n-------------");
constext txt_help_title_ops[] = N_("Group/thread/article operations\n-------------------------------");

constext txt_index_page_com[] = N_("Group Level Commands");
constext txt_info_add_kill[] = N_("Kill filter added");
constext txt_info_add_select[] = N_("Auto-selection filter added");
constext txt_info_all_parts_tagged[] = N_("All parts tagged");
constext txt_info_do_postpone[] = N_("Storing article for later posting");
constext txt_info_enter_valid_character[] = N_("Please enter a valid character");
constext txt_info_missing_part[] = N_("Missing part #%d");
constext txt_info_nopostponed[] = N_("*** No postponed articles ***");
constext txt_info_not_multipart_message[] = N_("Not a multi-part message");
constext txt_info_not_subscribed[] = N_("You are not subscribed to this group");
constext txt_info_no_previous_expression[] = N_("No previous expression");
constext txt_info_no_write[] = N_("Operation disabled in no-overwrite mode");
/* TODO: replace hardcoded key-name in txt_info_postponed */
constext txt_info_postponed[] = N_("%d postponed %s, reuse with ^O...\n");
constext txt_info_x_conversion_note[] = N_("X-Conversion-Note: multipart/alternative contents have been removed.\n\
  To get the whole article, turn alternative handling OFF in the Option Menu\n");
constext txt_is_mailbox[] = N_("Save filename for %s/%s is a mailbox. Attachment not saved");
constext txt_is_tex_encoded[] = N_("TeX2Iso encoded article");
constext txt_incomplete[] = N_("incomplete ");
/* TODO: replace hardcoded key-names */
constext txt_intro_page[] = N_("\nWelcome to %s, a full screen threaded Netnews reader. It can read news locally\n\
(ie. <spool>/news) or remotely (-r option)  from a NNTP (Network News Transport\n\
Protocol) server. -h lists the available command line options.\n\n\
%s  has four newsreading levels, the newsgroup selection page, the group index\n\
page, the thread listing page and the article viewer. Help is available at each\n\
level by pressing the 'h' command.\n\n\
Move up/down by using the terminal arrow keys or 'j' and 'k'.  Use PgUp/PgDn or\n\
Ctrl-U and Ctrl-D to page up/down. Enter a newsgroup by pressing RETURN/TAB.\n\n\
Articles, threads, tagged articles or articles matching a pattern can be mailed\n\
('m' command), printed ('o' command), saved ('s' command), piped ('|' command).\n\
Use  the 'w' command  to post a  news article,  the 'f'/'F' commands  to post a\n\
follow-up to  an existing  news article  and the 'r'/'R' commands  to reply via\n\
mail to an  existing news article author.  The 'M' command allows the operation\n\
of %s to be configured via a menu.\n\n\
For more information read the manual page, README, INSTALL, TODO and FTP files.\n\
Please send bug-reports/comments to %s with the 'R' command.\n");
constext txt_invalid_from[] = N_("Invalid  From: %s  line. Read the INSTALL file again.");
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	constext txt_invalid_multibyte_sequence[] = N_("Invalid multibyte sequence found\n");
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
constext txt_invalid_sender[] = N_("Invalid  Sender:-header %s");
constext txt_inverse_off[] = N_("Inverse video disabled");
constext txt_inverse_on[] = N_("Inverse video enabled");

constext txt_keymap_missing_key[] = N_("Missing definition for %s\n");
constext txt_keymap_invalid_key[] = N_("Invalid key definition '%s'\n");
constext txt_keymap_invalid_name[] = N_("Invalid keyname '%s'\n");
constext txt_keymap_upgraded[] = N_("Keymap file was upgraded to version %s\n");
constext txt_kill_from[] = N_("Kill From:     [%s] (y/n): ");
constext txt_kill_lines[] = N_("Kill Lines: (</>num): ");
constext txt_kill_menu[] = N_("Kill Article Menu");
constext txt_kill_msgid[] = N_("Kill Msg-Id:   [%s] (f/l/o/n): ");
constext txt_kill_scope[] = N_("Kill pattern scope  : ");
constext txt_kill_subj[] = N_("Kill Subject:  [%s] (y/n): ");
constext txt_kill_text[] = N_("Kill text pattern   : ");
constext txt_kill_time[] = N_("Kill time in days   : ");

constext txt_last[] = N_("Last");
constext txt_last_resp[] = N_("-- Last response --");
constext txt_lines[] = N_("Lines %s  ");

constext txt_msgid_line_only[] = N_("Message-ID: line              ");
constext txt_mail[] = N_("Mail");
constext txt_mailbox[] = N_("mailbox ");
constext txt_mail_art_to[] = N_("Mail article(s) to [%.*s]> ");
constext txt_mail_log_to[] = N_("Mailing log to %s\n");
constext txt_mail_bug_report[] = N_("Mail bug report...");
constext txt_mail_bug_report_confirm[] = N_("Mail BUG REPORT to %s?");
constext txt_mailed[] = N_("Mailed");
constext txt_mailing_to[] = N_("Mailing to %s...");
constext txt_mail_save_active_head[] = N_("# [Mail/Save] active file. Format is like news active file:\n\
#   groupname  max.artnum  min.artnum  /dir\n\
# The 4th field is the basedir (ie. ~/Mail or ~/News)\n#\n");
constext txt_marked_as_unread[] = N_("%s marked as unread");
constext txt_marked_tagged_arts_as_read[] = N_("Marked %d of %d tagged %s as read");
constext txt_mark_arts_read[] = N_("Mark all articles as read%s?");
constext txt_mark_art_read_tagged_current[] = N_("Mark %s=tagged articles, %s=current article, %s=quit: ");
constext txt_mark_group_read[] = N_("Mark group %s as read?");
constext txt_mark_thread_read[] = N_("Mark thread as read%s?");
constext txt_mark_thread_read_tagged_current[] = N_("Mark %s=tagged articles/threads, %s=current thread, %s=quit: ");
constext txt_matching_cmd_line_groups[] = N_("Matching %s groups...");
constext txt_mini_group_1[] = N_("<n>=set current to n; %s=next unread; %s=search pattern; %s=kill/select");
constext txt_mini_group_2[] = N_("%s=author search; %s=catchup; %s=line down; %s=line up; %s=mark read; %s=list thread");
constext txt_mini_info_1[] = N_("%s=line up; %s=line down; %s=page up; %s=page down; %s=top; %s=bottom");
constext txt_mini_info_2[] = N_("%s=search forwards; %s=search backwards; %s=quit");
constext txt_mini_page_1[] = N_("<n>=set current to n; %s=next unread; %s=search pattern; %s=kill/select");
constext txt_mini_page_2[] = N_("%s=author search; %s=body search; %s=catchup; %s=followup; %s=mark read");
constext txt_mini_select_1[] = N_("<n>=set current to n; %s=next unread; %s,%s=search pattern; %s=catchup");
constext txt_mini_select_2[] = N_("%s=line down; %s=line up; %s=help; %s=move; %s=quit; %s=toggle all/unread");
constext txt_mini_select_3[] = N_("%s=subscribe; %s=sub pattern; %s=unsubscribe; %s=unsub pattern; %s=yank in/out");
constext txt_mini_thread_1[] = N_("<n>=set current to n; %s=next unread; %s=catchup; %s=display toggle");
constext txt_mini_thread_2[] = N_("%s=help; %s=line down; %s=line up; %s=quit; %s=tag; %s=mark unread");
constext txt_more[] = N_("--More--");
constext txt_moving[] = N_("Moving %s...");

constext txt_name[] = N_(", name: ");
constext txt_newsgroup[] = N_("Goto newsgroup [%s]> ");
constext txt_newsgroup_plural[] = N_("newsgroups");
constext txt_newsgroup_position[] = N_("Position %s in group list (1,2,..,$) [%d]> ");
constext txt_newsgroup_singular[] = N_("newsgroup");
constext txt_newsrc_again[] = N_("Try and save newsrc file again?");
constext txt_newsrc_nogroups[] = N_("Warning: No newsgroups were written to your newsrc file. Save aborted.");
constext txt_newsrc_saved[] = N_("newsrc file saved successfully.\n");
constext txt_next_resp[] = N_("-- Next response --");
constext txt_nntp_authorization_failed[] = N_("NNTP authorization password not found for %s");
constext txt_no[] = N_("No  ");
constext txt_no_arts[] = N_("*** No articles ***");
constext txt_no_arts_posted[] = N_("No articles have been posted");
constext txt_no_description[] = N_("*** No description ***");
constext txt_no_filename[] = N_("No filename");
constext txt_no_group[] = N_("No group");
constext txt_no_groups[] = N_("*** No groups ***");
constext txt_no_groups_to_read[] = N_("No more groups to read");
constext txt_no_last_message[] = N_("No last message");
constext txt_no_mail_address[] = N_("No mail address");
constext txt_no_marked_arts[] = N_("No articles marked for saving");
constext txt_no_match[] = N_("No match");
constext txt_no_more_groups[] = N_("No more groups");
constext txt_no_newsgroups[] = N_("No newsgroups");
constext txt_no_next_unread_art[] = N_("No next unread article");
constext txt_no_prev_group[] = N_("No previous group");
constext txt_no_prev_unread_art[] = N_("No previous unread article");
constext txt_no_responses[] = N_("No responses");
constext txt_no_resps_in_thread[] = N_("No responses to list in current thread");
constext txt_no_search_string[] = N_("No search string");
constext txt_no_subject[] = N_("No subject");
#ifndef USE_CURSES
	constext txt_no_term_clear_eol[] = N_("%s: Terminal must have clear to end-of-line (ce)\n");
	constext txt_no_term_clear_eos[] = N_("%s: Terminal must have clear to end-of-screen (cd)\n");
	constext txt_no_term_clearscreen[] = N_("%s: Terminal must have clearscreen (cl) capability\n");
	constext txt_no_term_cursor_motion[] = N_("%s: Terminal must have cursor motion (cm)\n");
	constext txt_no_term_set[] = N_("%s: TERM variable must be set to use screen capabilities\n");
#endif /* !USE_CURSES */
constext txt_no_viewer_found[] = N_("No viewer found for %s/%s\n");
constext txt_not_exist[] = N_("Newsgroup does not exist on this server");
constext txt_not_in_active_file[] = N_("Group %s not found in active file");
constext txt_nrctbl_create[] = N_("c)reate it, use a)lternative name, use d)efault .newsrc, q)uit tin: ");
constext txt_nrctbl_default[] = N_("use a)lternative name, use d)efault .newsrc, q)uit tin: ");
constext txt_nrctbl_info[] = N_("# NNTP-server -> newsrc translation table and NNTP-server\n\
# shortname list for %s %s\n#\n# the format of this file is\n\
#   <FQDN of NNTP-server> <newsrc file> <shortname> ...\n#\n\
# if <newsrc file> is given without path, $HOME is assumed as its location\n\
#\n# examples:\n#   news.tin.org  .newsrc-tin.org  tinorg\n\
#   news.ka.nu    /tmp/nrc-nu      kanu    nu\n#\n");

constext txt_only[] = N_("Only");
constext txt_option_not_enabled[] = N_("Option not enabled. Recompile with %s.");
constext txt_options_menu[] = N_("Options Menu");
constext txt_out_of_memory[] = "%s: memory exhausted trying to allocate %d bytes in file %s line %d";

constext txt_pcre_error_at[] = N_("Error in regex: %s at pos. %d '%s'");
constext txt_pcre_error_num[] = N_("Error in regex: pcre internal error %d");
constext txt_pcre_error_text[] = N_("Error in regex: study - pcre internal error %s");
constext txt_post_a_followup[] = N_("Post a followup...");
/* TODO: replace hardcoded key-name in txt_post_error_ask_postpone */
constext txt_post_error_ask_postpone[] = N_("An error has occurred while posting the article. If you think that this\n\
error is temporary or otherwise correctable, you can postpone the article\n\
and pick it up again with ^O later.\n");
constext txt_post_history_menu[] = N_("Posted articles history");
constext txt_post_newsgroups[] = N_("Post to newsgroup(s) [%s]> ");
constext txt_post_processing[] = N_("-- post processing started --");
constext txt_post_processing_finished[] = N_("-- post processing completed --");
constext txt_post_subject[] = N_("Post subject [%s]> ");
constext txt_posted_info_file[] = N_("# Summary of mailed/posted messages viewable by 'W' command from within tin.\n");
constext txt_posting[] = N_("Posting article...");
constext txt_postpone_repost[] = N_("Post postponed articles [%%s]? (%s/%s/%s/%s/%s): ");
constext txt_prefix_hot[] = N_("Hot %s");
constext txt_prefix_tagged[] = N_("Tagged %s");
constext txt_prefix_untagged[] = N_("Untagged %s");
constext txt_processing_mail_arts[] = N_("Processing mail messages marked for deletion.");
constext txt_processing_saved_arts[] = N_("Processing saved articles marked for deletion.");
constext txt_prompt_fup_ignore[] = N_("Accept Followup-To? %s=post, %s=ignore, %s=quit: ");
constext txt_prompt_unchanged_mail[] = N_("Article unchanged, abort mailing?");
constext txt_prompt_see_postponed[] = N_("Do you want to see postponed articles (%d)?");

constext txt_quick_filter_kill[] = N_("Add quick kill filter?");
constext txt_quick_filter_select[] = N_("Add quick selection filter?");
constext txt_quit[] = N_("Do you really want to quit?");
constext txt_quit_cancel[] = N_("%s=edit cancel message, %s=quit, %s=delete (cancel) [%%s]: ");
constext txt_quit_despite_tags[] = N_("You have tagged articles in this group - quit anyway?");
constext txt_quit_edit_postpone[] = N_("%s=quit, %s=edit, %s=postpone: ");
constext txt_quit_edit_save_kill[] = N_("%s=quit %s=edit %s=save kill description: ");
constext txt_quit_edit_save_select[] = N_("%s=quit %s=edit %s=save select description: ");
constext txt_quit_no_write[] = N_("Do you really want to quit without saving your configuration?");
constext txt_quoted_printable[] = "quoted-printable";

constext txt_range_invalid[] = N_("Invalid range - valid are '0-9.$' eg. 1-$");
constext txt_read_abort[] = N_("Do you want to abort this operation?");
constext txt_read_exit[] = N_("Do you want to exit tin immediately?");
constext txt_read_resp[] = N_("Read response> ");
constext txt_reading_article[] = N_("Reading ('q' to quit)...");
constext txt_reading_arts[] = N_("Reading %sarticles...");
constext txt_reading_attributes_file[] = N_("Reading %sattributes file...\n");
constext txt_reading_config_file[] = N_("Reading %sconfig file...\n");
constext txt_reading_filter_file[] = N_("Reading filter file...\n");
constext txt_reading_groups[] = N_("Reading %s groups...");
constext txt_reading_input_history_file[] = N_("Reading input history file...\n");
constext txt_reading_keymap_file[] = N_("Reading keymap file...\n");
constext txt_reading_news_active_file[] = N_("Reading groups from active file... ");
constext txt_reading_news_newsrc_file[] = N_("Reading groups from newsrc file... ");
constext txt_reading_newsgroups_file[] = N_("Reading newsgroups file... ");
constext txt_reading_newsrc[] = N_("Reading newsrc file...");
#ifdef HAVE_GETTIMEOFDAY
	constext txt_remaining[] = N_("(%d:%02d remaining)");
#endif /* HAVE_GETTIMEOFDAY */
constext txt_remove_bogus[] = N_("Bogus group %s removed.");
constext txt_rename_error[] = N_("Error: rename %s to %s");
constext txt_reply_to_author[] = N_("Reply to author...");
constext txt_repost[] = N_("Repost");
constext txt_repost_an_article[] = N_("Reposting article...");
constext txt_repost_group[] = N_("Repost article(s) to group(s) [%s]> ");
constext txt_reset_newsrc[] = N_("Reset newsrc?");
constext txt_resp_redirect[] = N_("Responses have been directed to the following newsgroups");
constext txt_resp_to_poster[] = N_("Responses have been directed to poster. %s=mail, %s=post, %s=quit: ");
constext txt_resp_x_of_n[] = N_("RespNo %4d of %4d");
constext txt_return_key[] = N_("Press <RETURN> to continue...");

constext txt_select_from[] = N_("Select From    [%s] (y/n): ");
constext txt_select_lines[] = N_("Select Lines: (</>num): ");
constext txt_select_menu[] = N_("Auto-select Article Menu");
constext txt_select_msgid[] = N_("Select Msg-Id  [%s] (f/l/o/n): ");
constext txt_select_scope[] = N_("Select pattern scope: ");
constext txt_select_subj[] = N_("Select Subject [%s] (y/n): ");
constext txt_select_text[] = N_("Select text pattern : ");
constext txt_select_time[] = N_("Select time in days   : ");
constext txt_serverconfig_header[] = N_("# %s server configuration file\n\
# This file was automatically saved by %s %s %s (\"%s\")\n#\n\
# Do not edit while %s is running, since all your changes to this file\n\
# will be overwritten when you leave %s.\n\
# Do not edit at all if you don't know what you do.\n\
############################################################################\n\n");
constext txt_show_unread[] = N_("Showing unread groups only");
constext txt_subj_line_only[] = N_("Subject: line (ignore case)   ");
constext txt_subj_line_only_case[] = N_("Subject: line (case sensitive)");
constext txt_save[] = N_("Save");
constext txt_save_attachment[] = N_("Save '%s' (%s/%s)?");
constext txt_save_config[] = N_("Save configuration before continuing?");
constext txt_save_filename[] = N_("Save filename> ");
constext txt_saved[] = N_("Saved");
constext txt_saved_group[] = N_("%4d unread (%4d hot) %s in %s\n");
constext txt_saved_groupname[] = N_("Saved %s...\n");
constext txt_saved_nothing[] = N_("Nothing was saved");
constext txt_saved_summary[] = N_("\n%s %d %s from %d %s\n");
constext txt_saved_to[] = N_("-- %s saved to %s%s --");
constext txt_saved_to_range[] = N_("-- %s saved to %s - %s --");
constext txt_saving[] = N_("Saving...");
constext txt_screen_init_failed[] = N_("%s: Screen initialization failed");
#ifndef USE_CURSES
	constext txt_screen_too_small[] = N_("%s: screen is too small\n");
#endif /* !USE_CURSES */
constext txt_screen_too_small_exiting[] = N_("screen is too small, %s is exiting\n");
constext txt_search_backwards[] = N_("Search backwards [%s]> ");
constext txt_search_body[] = N_("Search body [%s]> ");
constext txt_search_forwards[] = N_("Search forwards [%s]> ");
constext txt_searching[] = N_("Searching...");
constext txt_searching_body[] = N_("Searching article %d of %d ('q' to abort)...");
constext txt_select_art[] = N_("Select article> ");
constext txt_select_config_file_option[] = N_("Select option number before text or use arrow keys and <CR>. 'q' to quit.");
constext txt_select_group[] = N_("Select group> ");
constext txt_select_pattern[] = N_("Enter selection pattern [%s]> ");
constext txt_select_thread[] = N_("Select thread > ");
constext txt_send_bugreport[] = N_("%s %s %s (\"%s\") [%s]: send a DETAILED bug report to %s\n");
constext txt_servers_active[] = N_("servers active-file");
constext txt_skipping_newgroups[] = N_("Cannot move into new newsgroups. Subscribe first...");
constext txt_space[] = N_("<SPACE>");
constext txt_starting_command[] = N_("Starting: (%s)");
constext txt_stp_list_thread[] = N_("List Thread (%d of %d)");
constext txt_stp_thread[] = N_("Thread (%.*s)");
constext txt_subscribe_pattern[] = N_("Enter wildcard subscribe pattern> ");
constext txt_subscribed_num_groups[] = N_("subscribed to %d groups");
constext txt_subscribed_to[] = N_("Subscribed to %s");
constext txt_subscribing[] = N_("Subscribing... ");
constext txt_supersede_article[] = N_("Repost or supersede article(s) [%%s]? (%s/%s/%s): ");
constext txt_supersede_group[] = N_("Supersede article(s) to group(s) [%s]> ");
constext txt_superseding_art[] = N_("Superseding article ...");
constext txt_suspended_message[] = N_("\nStopped. Type 'fg' to restart %s\n");

constext txt_time_default_days[] = N_("%d days");
constext txt_tab[] = N_("<TAB>");
constext txt_tex[] = N_("TeX ");
constext txt_tinrc_defaults[] = N_("# Default action/prompt strings\n");
constext txt_tinrc_filter[] = N_("# Defaults for quick (1 key) kill & auto-selection filters\n\
# header=NUM  0,1=Subject: 2,3=From: 4=Message-ID: & full References: line\n\
#             5=Message-ID: & last References: entry only\n\
#             6=Message-ID: entry only 7=Lines:\n\
# global=ON/OFF  ON=apply to all groups OFF=apply to current group\n\
# case=ON/OFF    ON=filter case sensitive OFF=ignore case\n\
# expire=ON/OFF  ON=limit to default_filter_days OFF=don't ever expire\n");
/* do NOT localize the next string! */
constext txt_tinrc_header[] = "# %s configuration file V%s\n\
# This file was automatically saved by %s %s %s (\"%s\")\n#\n\
# Do not edit while tin is running, since all your changes to this file\n\
# will be overwritten when you leave tin.\n#\n\
############################################################################\n\n";
constext txt_tinrc_info_in_last_line[] = N_("# If ON use print current subject or newsgroup description in the last line\n");
constext txt_tinrc_newnews[] = N_("# Host & time info used for detecting new groups (don't touch)\n");
constext txt_there_is_no_news[] = N_("There is no news\n");
constext txt_thread_upper[] = N_("Thread");
constext txt_thread_com[] = N_("Thread Level Commands");
constext txt_thread_marked_as_deselected[] = N_("Thread deselected");
constext txt_thread_marked_as_selected[] = N_("Thread selected");
#if 0 /* unused */
	constext txt_thread_plural[] = N_("threads");
#endif /* 0 */
constext txt_thread_range[] = N_("Thread range");
constext txt_thread_singular[] = N_("thread");
constext txt_thread_x_of_n[] = N_("Thread %4s of %4s");
constext txt_threading_arts[] = N_("Threading articles...");
constext txt_toggled_high[] = N_("Toggled word highlighting %s");
constext txt_toggled_rot13[] = N_("Toggled rot13 encoding");
constext txt_toggled_tex2iso[] = N_("Toggled german TeX encoding %s");
constext txt_toggled_tabwidth[] = N_("Toggled tab-width to %d");
constext txt_trying_dotlock[] = N_("%d Trying to dotlock %s");
constext txt_trying_lock[] = N_("%d Trying to lock %s");
constext txt_type_h_for_help[] = N_("           h=help\n");

constext txt_unlimited_time[] = N_("Unlimited");
constext txt_unsubscribe_pattern[] = N_("Enter wildcard unsubscribe pattern> ");
constext txt_uu_error_decode[] = N_("Error decoding %s : %s");
constext txt_uu_error_no_end[] = N_("No end.");
constext txt_uu_success[] = N_("%s successfully decoded.");
constext txt_uue[] = N_("%*s[-- %s/%s, %suuencoded file, %d lines, name: %s --]\n\n");
constext txt_unread[] = N_("unread ");
constext txt_unsubscribed_num_groups[] = N_("unsubscribed from %d groups");
constext txt_unsubscribed_to[] = N_("Unsubscribed from %s");
constext txt_unsubscribing[] = N_("Unsubscribing... ");
constext txt_unthreading_arts[] = N_("Unthreading articles...");
constext txt_updated[] = N_("Updated");
constext txt_updating[] = N_("Updating");
constext txt_url_open[] = N_("Opening %s\n");
constext txt_url_done[] = N_("No more URL's in this article");
constext txt_use_mime[] = N_("Use MIME display program for this message?");
constext txt_usage_catchup[] = N_("  -c       mark all news as read in subscribed newsgroups (batch mode)");
constext txt_usage_check_for_unread_news[] = N_("  -Z       return status indicating if any unread news (batch mode)");
constext txt_usage_dont_check_new_newsgroups[] = N_("  -q       don't check for new newsgroups");
constext txt_usage_dont_save_files_on_quit[] = N_("  -X       don't save any files on quit");
constext txt_usage_dont_show_descriptions[] = N_("  -d       don't show newsgroup descriptions");
constext txt_usage_getart_limit[] = N_("  -G limit get only limit articles/group");
constext txt_usage_help_information[] = N_("  -H       help information about %s");
constext txt_usage_help_message[] = N_("  -h       this help message");
constext txt_usage_index_newsdir[] = N_("  -I dir   news index file directory [default=%s]");
constext txt_usage_update_index_files[] = N_("  -u       update index files (batch mode)");
constext txt_usage_maildir[] = N_("  -m dir   mailbox directory [default=%s]");
constext txt_usage_mail_bugreport[] = N_("\nMail bug reports/comments to %s");
constext txt_usage_mail_new_news[] = N_("  -N       mail new news to your posts (batch mode)");
constext txt_usage_mail_new_news_to_user[] = N_("  -M user  mail new news to specified user (batch mode)");
constext txt_usage_newsrc_file[] = N_("  -f file  subscribed to newsgroups file [default=%s]");
constext txt_usage_no_posting[] = N_("  -x       no posting mode");
constext txt_usage_post_article[] = N_("  -w       post an article and exit");
constext txt_usage_post_postponed_arts[] = N_("  -o       post all postponed articles and exit");
constext txt_usage_read_news_remotely[] = N_("  -r       read news remotely from default NNTP server");
constext txt_usage_read_saved_news[] = N_("  -R       read news saved by -S option");
constext txt_usage_savedir[] = N_("  -s dir   save news directory [default=%s]");
constext txt_usage_save_new_news[] = N_("  -S       save new news for later reading (batch mode)");
constext txt_usage_start_if_unread_news[] = N_("  -z       start if any unread news");
constext txt_usage_tin[] = N_("A Usenet reader.\n\nUsage: %s [options] [newsgroup[,...]]");
constext txt_usage_verbose[] = N_("  -v       verbose output for batch mode options");
constext txt_usage_version[] = N_("  -V       print version & date information");
constext txt_useful_without_batch_mode[] = N_("%s only useful without batch mode operations\n");
constext txt_useful_with_batch_mode[] = N_("%s only useful for batch mode operations\n");

constext txt_value_out_of_range[] = N_("\n%s%d out of range (0 - %d). Reset to 0");
constext txt_view_attachment[] = N_("View '%s' (%s/%s)?");

constext txt_warn_art_line_too_long[] = N_("\nWarning: posting exceeds %d columns. Line %d is the first long one:\n%-100s\n");
constext txt_warn_article_unchanged[] = N_("\nWarning: article unchanged after editing\n");
constext txt_warn_blank_subject[] = N_("\nWarning: \"Subject:\" contains only whitespaces.\n");
constext txt_warn_re_but_no_references[] = N_("\n\
Warning: \"Subject:\" begins with \"Re: \" but there are no \"References:\".\n");
constext txt_warn_references_but_no_re[] = N_("\n\
Warning: Article has \"References:\" but \"Subject:\" does not begin\n\
         with \"Re: \" and does not contain \"(was:\".\n");
constext txt_warn_cancel[] = N_("Read carefully!\n\n\
  You are about to cancel an article seemingly written by you. This will wipe\n\
  the article from most  news servers  throughout the world,  but there is no\n\
  guarantee that it will work.\n\nThis is the article you are about to cancel:\n\n");
constext txt_warn_encoding_and_external_inews[] = N_("\n\
Warning: You are using a non-plain transfer encoding (such as base64 or\n\
         quoted-printable) and an external inews program to submit your\n\
         article. If a signature is appended by that inews program it will\n\
         not be encoded properly.\n");
constext txt_warn_update[] = N_("\n\nYou are upgrading to tin %s from an earlier version.\n\
Some values in your %s file have changed!\nRead WHATSNEW, etc...\n");
constext txt_warn_downgrade[] = N_("\n\nYou are downgrading to tin %s from a more recent version!\n\
Some values in your %s file may be ignored, others might have changed!\n");
constext txt_warn_unrecognized_version[] = "\n\nUnrecognized version string!\n";
constext txt_warn_newsrc[] = N_("Warning: tin wrote fewer groups to your\n\t%s\n\
than it read at startup. If you didn't unsubscribe from %ld %s during\n\
this session this indicates an error and you should backup your %s\n\
before you start tin once again!\n");
constext txt_warn_multiple_sigs[] = N_("\nWarning: Found %d '-- \\n' lines, this may confuse some people.\n");
constext txt_warn_not_all_arts_saved[] = N_("Warning: Only %d out of %d articles were saved");
constext txt_warn_sig_too_long[] = N_("\n\
Warning: Your signature  is longer than %d lines.  Since signatures usually do\n\
         not  transport any  useful information,  they should be as  short as\n\
         possible.\n");
constext txt_warn_suspicious_mail[] = N_("Warning: this mail address may contain a spamtrap. %s=continue, %s=abort? ");
constext txt_warn_wrong_sig_format[] = N_("\nWarning: Signatures should start with '-- \\n' not with '--\\n'.\n");
constext txt_writing_attributes_file[] = N_("Writing attributes file...");

constext txt_x_resp[] = N_("%d Responses");

constext txt_yanked_groups[] = N_("Added %d %s");
constext txt_yanked_none[] = N_("No unsubscribed groups to show");
constext txt_yanked_sub_groups[] = N_("Showing subscribed to groups only");
constext txt_yes[] = N_("Yes ");
constext txt_you_have_mail[] = N_("    You have mail\n");


/* TODO: cleanup */
#ifdef CHARSET_CONVERSION
	constext txt_warn_charset_conversion[] = N_("\n\
Warning: Posting is in %s and contains characters which are not\n\
         in your selected MM_NETWORK_CHARSET: %s.\n\
         These characters will be replaced by '?' if you post this\n\
         article unchanged. To avoid garbling your article please either\n\
         edit it and remove those characters or change the setting of\n\
         MM_NETWORK_CHARSET to a suitable value for your posting via the\n\
         M)enu option.\n");
#endif /* CHARSET_CONVERSION */

#ifdef DEBUG
	constext txt_usage_debug[] = N_("  -D       debug mode 1=NNTP 2=ALL");
#endif /* DEBUG */

#ifdef FORGERY
	constext txt_warn_cancel_forgery[] = N_("Read carefully!\n\n\
  You are about to cancel an article seemingly not written by you.  This will\n\
  wipe the article from lots of news servers throughout the world;\n\
  Usenet's majority  considers this  rather inappropriate,  to say the least.\n\
  Only press 'd'  if you are  absolutely positive  that you are ready to take\n\
  the rap.\n\nThis is the article you are about to cancel:\n\n");
#endif /* FORGERY */

#ifdef HAVE_COLOR
	constext txt_help_global_toggle_color[] = N_("toggle color");
	constext txt_tinrc_colors[] = N_("# Changing colors of several screen parts\n\
# Possible values are:\n\
#  -1 = default (white for foreground and black for background)\n\
#   0 = black\n\
#   1 = red\n\
#   2 = green\n\
#   3 = brown\n\
#   4 = blue\n\
#   5 = pink\n\
#   6 = cyan\n\
#   7 = white\n\
# These are *only* for foreground:\n\
#   8 = gray\n\
#   9 = light red\n\
#  10 = light green\n\
#  11 = yellow\n\
#  12 = light blue\n\
#  13 = light pink\n\
#  14 = light cyan\n\
#  15 = light white\n\n");
	constext txt_usage_toggle_color[] = N_("  -a       toggle color flag");
#endif /* HAVE_COLOR */

#ifdef HAVE_FASCIST_NEWSADMIN
	constext txt_error_followup_to_several_groups[] = N_("\nError: Followup-To set to more than one newsgroup!\n");
	constext txt_error_missing_followup_to[] = N_("\nError: cross-posting to %d newsgroups and no Followup-To line!\n");
	constext txt_error_not_valid_newsgroup[] = N_("\nError: \"%s\" is not a valid newsgroup!\n");
#else
	constext txt_warn_followup_to_several_groups[] = N_("\nWarning: Followup-To set to more than one newsgroup!\n");
	constext txt_warn_missing_followup_to[] = N_("\nWarning: cross-posting to %d newsgroups and no Followup-To line!\n");
	constext txt_warn_not_in_newsrc[] = N_("\nWarning: \"%s\" is not in your newsrc, it may be invalid at this site!\n");
	constext txt_warn_not_valid_newsgroup[] = N_("\nWarning: \"%s\" is not a valid newsgroup at this site!\n");
#endif /* HAVE_FASCIST_NEWSADMIN */

#ifdef HAVE_LIBUU
	constext txt_libuu_saved[] = N_("%d files successfully written from %d articles. %d %s occurred.");
	constext txt_libuu_error_missing[] = N_("Missing parts.");
	constext txt_libuu_error_no_begin[] = N_("No beginning.");
	constext txt_libuu_error_no_data[] = N_("No data.");
	constext txt_libuu_error_unknown[] = N_("Unknown error.");
#else
#	if defined(M_UNIX) && defined(HAVE_SUM) && !defined(DONT_HAVE_PIPING)
		constext txt_checksum_of_file[] = N_("\tChecksum of %s (%ld %s)");
#	endif /* M_UNIX && HAVE_SUM && !DONT_HAVE_PIPING */
#endif /* HAVE_LIBUU */

#ifdef HAVE_MH_MAIL_HANDLING
	constext txt_reading_mail_active_file[] = N_("Reading mail active file... ");
	constext txt_reading_mailgroups_file[] = N_("Reading mailgroups file... ");
#endif /* HAVE_MH_MAIL_HANDLING */

#ifdef HAVE_PGP_GPG
	constext txt_help_article_pgp[] = N_("perform PGP operations on article");
	constext txt_pgp_add[] = N_("Add key(s) to public keyring?");
	constext txt_pgp_mail[] = N_("%s=encrypt, %s=sign, %s=both, %s=quit: ");
	constext txt_pgp_news[] = N_("%s=sign, %s=sign & include public key, %s=quit: ");
	constext txt_pgp_not_avail[] = N_("PGP has not been set up (can't open %s)");
	constext txt_pgp_nothing[] = N_("Article not signed and no public keys found");
#	ifdef HAVE_ISPELL
		constext txt_quit_edit_post[] = N_("%s=quit, %s=edit, %s=ispell, %s=pgp, %s=menu, %s=post, %s=postpone: ");
		constext txt_quit_edit_send[] = N_("%s=quit, %s=edit, %s=ispell, %s=pgp, %s=send [%%s]: ");
		constext txt_quit_edit_xpost[] = N_("%s=quit, %s=edit, %s=ispell, %s=pgp, %s=menu, %s=post, %s=postpone [%%s]: ");
#	else
	constext txt_quit_edit_post[] = N_("%s=quit, %s=edit, %s=pgp, %s=menu, %s=post, %s=postpone: ");
	constext txt_quit_edit_send[] = N_("%s=quit, %s=edit, %s=pgp, %s=send [%%s]: ");
	constext txt_quit_edit_xpost[] = N_("%s=quit, %s=edit, %s=pgp, %s=menu, %s=post, %s=postpone [%%s]: ");
#	endif /* HAVE_ISPELL */
#else
#	ifdef HAVE_ISPELL
		constext txt_quit_edit_post[] = N_("%s=quit, %s=edit, %s=ispell, %s=menu, %s=post, %s=postpone: ");
		constext txt_quit_edit_send[] = N_("%s=quit, %s=edit, %s=ispell, %s=send [%%s]: ");
		constext txt_quit_edit_xpost[] = N_("%s=quit, %s=edit, %s=ispell, %s=menu, %s=post, %s=postpone [%%s]: ");
#	else
		constext txt_quit_edit_post[] = N_("%s=quit, %s=edit, %s=menu, %s=post, %s=postpone: ");
		constext txt_quit_edit_send[] = N_("%s=quit, %s=edit, %s=send [%%s]: ");
		constext txt_quit_edit_xpost[] = N_("%s=quit, %s=edit, %s=menu, %s=post, %s=postpone [%%s]: ");
#	endif /* HAVE_ISPELL */
#endif /* HAVE_PGP_GPG */

#ifdef M_UNIX
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2006 Iain Lea.";
#endif /* M_UNIX */

#ifdef NNTP_ABLE
	constext txt_caching_off[] = N_("Try cache_overview_files to speed up things.\n");
	constext txt_caching_on[] = N_("Tin will use local index files instead.\n");
	constext txt_cannot_get_nntp_server_name[] = N_("Cannot find NNTP server name");
	constext txt_connecting_port[] = N_("Connecting to %s:%d...");
	constext txt_disconnecting[] = N_("Disconnecting from server...\n");
	constext txt_error_wrong_newsgroupname_in_group_response[] = N_("Wrong newsgroup name in response of GROUP command, %s for %s");
	constext txt_failed_to_connect_to_server[] = N_("Failed to connect to NNTP server %s. Exiting...");
	constext txt_nntp_ok_goodbye[] = N_("205  Closing connection");
	constext txt_no_xover_support[] = N_("Your server does not support the NNTP XOVER or OVER command.\n");
	constext txt_reconnect_to_news_server[] = N_("Connection to news server has timed out. Reconnect?");
	constext txt_server_name_in_file_env_var[] = N_("Put the server name in the file %s,\nor set the environment variable NNTPSERVER");
	constext txt_usage_force_authentication[] = N_("  -A       force authentication on connect");
	constext txt_usage_newsserver[] = N_("  -g serv  read news from NNTP server serv [default=%s]");
	constext txt_usage_port[] = N_("  -p port  use port as NNTP port [default=%d]");
	constext txt_usage_quickstart[] = N_("  -Q       quick start. Same as -nqd");
	constext txt_usage_read_only_active[] = N_("  -l       use only LIST instead of GROUP (-n) command");
	constext txt_usage_read_only_subscribed[] = N_("  -n       only read subscribed .newsrc groups from NNTP server");
#	ifdef HAVE_GETSERVBYNAME
		constext txt_error_unknown_service[] = N_("%s/tcp: Unknown service.\n");
#	endif /* HAVE_GETSERVBYNAME */
#	ifdef INET6
		constext txt_error_socket_or_connect_problem[] = N_("\nsocket or connect problem\n");
#	else
		constext txt_connection_to[] = N_("\nConnection to %s: ");
		constext txt_giving_up[] = N_("Giving up...\n");
#	endif /* INET6 */
#	ifdef XHDR_XREF
		constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its XOVER information.\n\
Tin will try to use XHDR XREF instead (slows down things a bit).\n");
#	else
		constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its XOVER information.\n");
#	endif /* XHDR_XREF */
#	ifndef NNTP_ONLY
		constext txt_cannot_open_active_file[] = N_("Can't open %s. Try %s -r to read news via NNTP.\n");
#	endif /* !NNTP_ONLY */
#else
	constext txt_usage_quickstart[] = N_("  -Q       quick start. Same as -qd");
	constext txt_usage_read_only_active[] = N_("  -l       read only active file instead of scanning spool (-n) command");
	constext txt_usage_read_only_subscribed[] = N_("  -n       only read subscribed .newsrc groups from spool");
	constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its NOV-files.\n");
#endif /* NNTP_ABLE */

#ifdef NNTP_INEWS
	constext txt_post_via_builtin_inews[] = N_("Posting using external inews failed. Use built in inews instead?");
	constext txt_post_via_builtin_inews_only[] = N_("It worked! Should I always use my built in inews from now on?");
#endif /* NNTP_INEWS */

#ifdef VMS
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2006 Iain Lea & Tod McQuillin & other.";
#endif /* VMS */

#ifndef DISABLE_PRINTING
	constext txt_articles_printed[] = N_("%d %s printed");
	constext txt_help_global_print[] = N_("output article/thread/hot/pattern/tagged articles to printer");
	constext txt_print[] = N_("Print");
	constext txt_printing[] = N_("Printing...");
#endif /* !DISABLE_PRINTING */

#ifndef DONT_HAVE_PIPING
	constext txt_help_global_pipe[] = N_("pipe article/thread/hot/pattern/tagged articles into command");
	constext txt_no_command[] = N_("No command");
	constext txt_pipe[] = N_("Pipe");
	constext txt_pipe_to_command[] = N_("Pipe to command [%.*s]> ");
	constext txt_piping[] = N_("Piping...");
#else
	constext txt_piping_not_enabled[] = N_("Piping not enabled.");
#endif /* !DONT_HAVE_PIPING */

#ifndef FOLLOW_USEFOR_DRAFT
	constext txt_error_header_line_comma[] = N_("\n\
Error: The \"%s:\" line has spaces  in it that MUST be removed.\n\
       The only allowable  space is the one  separating the colon (:)\n\
       from  the  contents.  Use a  comma  (,)  to separate  multiple\n\
       newsgroup names.\n");
	constext txt_error_header_line_groups_contd[] = N_("\n\
Error: The \"%s:\" line is  continued in  the next line.  Since\n\
       the line  may not  contain  whitespace,  this is  not allowed.\n\
       Please write all newsgroups into a single line.\n");
#else
	constext txt_warn_header_line_groups_contd[] = N_("\n\
Warning: The \"%s:\" line is continued in the next line.\n\
         This is a very new feature and may not be accepted by all servers.\n\
         To avoid trouble please write all newsgroups into a single line.\n");
	constext txt_warn_header_line_comma[] = N_("\n\
Warning: The \"%s:\" line has spaces in it that SHOULD be removed.\n");
#endif /* !FOLLOW_USEFOR_DRAFT */

#ifndef NO_ETIQUETTE
	constext txt_warn_posting_etiquette[] = N_("\n\
  If your article contains quoted text  please take some time to pare it down\n\
  to just the  key points to which you are  responding,  or people will think\n\
  you are a dweeb!  Many people have the habit of skipping any article  whose\n\
  first page is largely  quoted material.  Format your article to fit in less\n\
  then 80 chars,  since that's the conventional size  (72 is a good choice as\n\
  it allows quoting without exceeding the limit).  If your lines are too long\n\
  they'll wrap  around  ugly and  people won't  read what you  write.  If you\n\
  aren't  careful  and considerate  in  formatting  your posting, people  are\n\
  likely to ignore it completely.  It's a crowded net out there.\n");
#endif /* !NO_ETIQUETTE */

#ifndef NO_SHELL_ESCAPE
	constext txt_help_global_shell_escape[] = N_("shell escape");
	constext txt_shell_command[] = N_("Shell Command (%s)");
	constext txt_shell_escape[] = N_("Enter shell command [%s]> ");
#endif /* !NO_SHELL_ESCAPE */

#if !defined(USE_CURSES) && defined(M_UNIX) && !defined(USE_TERMINFO)
	constext txt_cannot_get_term_entry[] = N_("%s: Can't get entry for TERM\n");
#endif /* !USE_CURSES && M_UNIX && !USE_TERMINFO */

#if defined(HAVE_POLL) || defined(HAVE_SELECT)
	constext txt_group[] = N_("Group %.*s ('q' to quit)...");
#else
	constext txt_group[] = N_("Group %.*s...");
#endif /* HAVE_POLL || HAVE_SELECT */

#if defined(NNTP_ABLE) && defined(TLI) && !defined(INET6)
	constext txt_error_server_unavailable[] = N_("Server unavailable\n");
	constext txt_error_topen[] = "t_open: can't t_open /dev/tcp";
#endif /* NNTP_ABLE && TLI && !INET6 */

#ifndef DISABLE_PRINTING
#	ifndef DONT_HAVE_PIPING
		constext txt_mini_group_3[] = N_("%s=pipe; %s=mail; %s=print; %s=quit; %s=toggle all/unread; %s=save; %s=tag; %s=post");
		constext txt_mini_page_3[] = N_("%s=pipe; %s=mail; %s=print; %s=quit; %s=reply mail; %s=save; %s=tag; %s=post");
#	else
		constext txt_mini_group_3[] = N_("%s=mail; %s=print; %s=quit; %s=toggle all/unread; %s=save; %s=tag; %s=post");
		constext txt_mini_page_3[] = N_("%s=mail; %s=print; %s=quit; %s=reply mail; %s=save; %s=tag; %s=post");
#	endif /* !DONT_HAVE_PIPING */
#else
#	ifndef DONT_HAVE_PIPING
		constext txt_mini_group_3[] = N_("%s=pipe; %s=mail; %s=quit; %s=toggle all/unread; %s=save; %s=tag; %s=post");
		constext txt_mini_page_3[] = N_("%s=pipe; %s=mail; %s=quit; %s=reply mail; %s=save; %s=tag; %s=post");
#	else
		constext txt_mini_group_3[] = N_("%s=mail; %s=quit; %s=toggle all/unread; %s=save; %s=tag; %s=post");
		constext txt_mini_page_3[] = N_("%s=mail; %s=quit; %s=reply mail; %s=save; %s=tag; %s=post");
#	endif /* !DONT_HAVE_PIPING */
#endif /* !DISABLE_PRINTING */

#ifdef HAVE_COLOR
#	ifdef USE_CURSES
		constext txt_no_colorterm[] = N_("Terminal does not support color");
#	endif /* USE_CURSES */
#endif /* HAVE_COLOR */

#if defined(NNTP_ABLE) && defined(HAVE_INET_NTOA)
	constext txt_trying[] = N_("Trying %s");
#endif /* NNTP_ABLE && HAVE_INET_NTOA */


/*
 * OFF ~ FALSE, ON ~ TRUE
 */
constext *txt_onoff[] = { "OFF", "ON" };

/*
 * NB: All the following arrays must match corresponding ordering in tin.h
 * Threading types
 */
constext *txt_threading[] = {
	N_("None"),
	N_("Subject"),
	N_("References"),
	N_("Both Subject and References"),
	N_("Multipart Subject"),
	N_("Percentage Match")
};

/*
 * Whether to use wildmat() or regexec() for matching strings
 */
constext *txt_wildcard_type[] = { "WILDMAT", "REGEX" };

/*
 * Handling of uuencoded data in pager
 */
constext *txt_hide_uue_type[] = {
	N_("No"),
	N_("Yes"),
	N_("Hide All")
};

/*
 * How the From: line is displayed.
 */
constext *txt_show_from[] = {
	N_("None"),
	N_("Address"),
	N_("Full Name"),
	N_("Address and Name")
};

/*
 * How the score of a thread is computed
 */
constext *txt_thread_score_type[] = {
	N_("Max"),
	N_("Sum"),
	N_("Average")
};

constext *txt_show_info_type[] = {
	N_("None"),
	N_("Lines"),
	N_("Score"),
	N_("Lines & Score")
};

#ifdef HAVE_COLOR
	/*
	 * Which colors can be used.
	 */
	constext *txt_colors[] = {
		N_("Default"),
		N_("Black"),
		N_("Red"),
		N_("Green"),
		N_("Brown"),
		N_("Blue"),
		N_("Pink"),
		N_("Cyan"),
		N_("White"),
		N_("Gray"),
		N_("Light Red"),
		N_("Light Green"),
		N_("Yellow"),
		N_("Light Blue"),
		N_("Light Pink"),
		N_("Light Cyan"),
		N_("Light White")
	};
#endif /* HAVE_COLOR */

/*
 * Which mark types can be used.
 */
constext *txt_marks[] = {
	N_("Nothing"),
	N_("Mark"),
	N_("Space")
};

/*
 * Which attributes can be used for highlighting
 */
constext *txt_attrs[] = {
	N_("Normal"),
	N_("Best highlighting"),
	N_("Underline"),
	N_("Reverse video"),
	N_("Blinking"),
	N_("Half bright"),
	N_("Bold")
};

/* different confirm choices */
constext *txt_confirm_choices[] = {
	N_("none"),
	N_("commands"),
	N_("select"),
	N_("quit"),
	N_("commands & quit"),
	N_("commands & select"),
	N_("quit & select"),
	N_("commands & quit & select")
};

/*
 * MIME-Content-Transfer-Encodings.
 */
/* TODO: can any of this go away? */
constext *txt_mime_encodings[] = {
	txt_8bit, txt_base64, txt_quoted_printable, txt_7bit
};

constext *content_encodings[] = {
	"7bit", "quoted-printable", "base64", "8bit", "binary", "x-uuencode"
};

const char *content_types[] = {
	"text", "multipart", "application", "message", "image", "audio", "video"
};

/*
 * Array of possible post processing descriptions and short-keys
 * This must match the ordering of the defines in tin.h
 */
constext *txt_post_process_type[] = {
		N_("No"),
		N_("Shell archive"),
		N_("Yes")
};

constext *txt_sort_a_type[] = {
		N_("Nothing"),
		N_("Subject: (descending)"),
		N_("Subject: (ascending)"),
		N_("From: (descending)"),
		N_("From: (ascending)"),
		N_("Date: (descending)"),
		N_("Date: (ascending)"),
		N_("Score (descending)"),
		N_("Score (ascending)"),
		N_("Lines: (descending)"),
		N_("Lines: (ascending)")
};

constext *txt_sort_t_type[] = {
		N_("Nothing"),
		N_("Score (descending)"),
		N_("Score (ascending)")
};

/* Ways of handling bogus groups */
constext *txt_strip_bogus_type[] = {
		N_("Always Keep"),
		N_("Always Remove"),
		N_("Mark with D on selection screen")
};

/* Ways of handling killed articles */
constext *txt_kill_level_type[] = {
		N_("Kill only unread arts"),
		N_("Kill all arts & show with K"), /* TODO: s/K/art_marked_killed/ */
		N_("Kill all arts and never show")
};

/* Various quoting styles */
constext *txt_quote_style_type[] = {
		N_("Nothing special"),
		N_("Compress quotes"),
		N_("Quote signatures"),
		N_("Compress quotes, quote sigs"),
		N_("Quote empty lines"),
		N_("Compress quotes, quote empty lines"),
		N_("Quote sigs & empty lines"),
		N_("Comp. q., quote sigs & empty lines")
};

#ifdef CHARSET_CONVERSION
/* supported charsets */
constext *txt_mime_charsets[] = {
	"US-ASCII",
	"ISO-8859-1", "ISO-8859-2", "ISO-8859-3", "ISO-8859-4", "ISO-8859-5",
	"ISO-8859-7", "ISO-8859-9", "ISO-8859-10", "ISO-8859-13", "ISO-8859-14",
	"ISO-8859-15", "ISO-8859-16",
	"KOI8-RU", "KOI8-R", "KOI8-U",
	"EUC-CN", "EUC-JP", "EUC-KR", "EUC-TW",
	"ISO-2022-CN", "ISO-2022-CN-EXT", "ISO-2022-JP", "ISO-2022-JP-1",
	"ISO-2022-JP-2",
	"Big5",
	"UTF-8"
};
#endif /* CHARSET_CONVERSION */

/* 7bit charsets, US-ASCII must be the first entry */
constext *txt_mime_7bit_charsets[] = {
	"US-ASCII",
	"ISO-2022-CN", "ISO-2022-CN-EXT", "ISO-2022-JP", "ISO-2022-JP-1",
	"ISO-2022-JP-2", "ISO-2022-KR",
	"HZ-GB-2312",
	/* sentinel */
	""
};

/* different mailbox formats */
constext *txt_mailbox_formats[] = {
	"MBOXO",
	"MBOXRD",
	"MMDF"
};

/* interactive mailers */
constext *txt_interactive_mailers[] = {
	N_("no"),
	N_("with headers"),
	N_("without headers")
};

#ifdef HAVE_UNICODE_NORMALIZATION
constext *txt_normalization_forms[] = {
	N_("None"),
#	ifdef HAVE_LIBICUUC
	N_("NFKC"),
	N_("NFKD"),
	N_("NFC"),
	N_("NFD")
#	else
#		ifdef HAVE_LIBIDN
	N_("NFKC")
#		endif /* HAVE_LIBIDN */
#	endif /* HAVE_LIBICUUC */
};
#endif /* HAVE_UNICODE_NORMALIZATION */

struct opttxt txt_display_options = {
	NULL,
	N_("Display Options"),
	NULL
};

#ifdef HAVE_COLOR
struct opttxt txt_color_options = {
	NULL,
	N_("Color Options"),
	NULL
};
#endif /* HAVE_COLOR */

struct opttxt txt_getart_limit_options = {
	NULL,
	N_("Article-Limiting Options"),
	NULL
};

struct opttxt txt_posting_options = {
	NULL,
	N_("Posting/Mailing Options"),
	NULL
};

struct opttxt txt_saving_options = {
	NULL,
	N_("Saving/Printing Options"),
	NULL
};

struct opttxt txt_expert_options = {
	NULL,
	N_("Expert Options"),
	NULL
};

struct opttxt txt_filtering_options = {
	NULL,
	N_("Filtering Options"),
	NULL
};

struct opttxt txt_beginner_level = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show mini menu & posting etiquette"),
	N_("# If ON show a mini menu of useful commands at each level\n\
# and posting etiquette after composing an article\n")
};

struct opttxt txt_show_description = {
	N_("Show short description for each newsgroup. <SPACE> toggles & <CR> sets."),
	N_("Show description of each newsgroup"),
	N_("# If ON show group description text after newsgroup name at\n\
# group selection level\n")
};

struct opttxt txt_show_author = {
	N_("Show Subject & From (author) fields in group menu. <SPACE> toggles & <CR> sets."),
	N_("In group menu, show author by"),
	N_("# Part of from field to display\n\
# Possible values are (the default is marked with *):\n\
#   0 = none\n\
#   1 = address\n\
# * 2 = full name\n\
#   3 = both\n")
};

struct opttxt txt_draw_arrow = {
	N_("Draw -> or highlighted bar for selection. <SPACE> toggles & <CR> sets."),
	N_("Draw -> instead of highlighted bar"),
	N_("# If ON use -> otherwise highlighted bar for selection\n")
};

struct opttxt txt_inverse_okay = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use inverse video for page headers"),
	N_("# If ON use inverse video for page headers at different levels\n")
};

struct opttxt txt_thread_articles = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Thread articles by"),
	N_("# Thread articles by ...\n\
# Possible values are (the default is marked with *):\n\
#   0 = nothing\n\
#   1 = Subject\n\
#   2 = References\n\
# * 3 = Both (Subject and References)\n\
#   4 = Multipart Subject\n\
#   5 = Percentage Match\n")
};

struct opttxt txt_thread_perc = {
	N_("Enter percentage match required to thread together. <CR> sets."),
	N_("Thread percentage match"),
	N_("# Thread percentage match...\n\
# the percentage of characters in the subject of an article that must match\n\
# a base article for both those articles to be considered to belong to the\n\
# same thread. This option is an integer percentage, eg. 80, no decimals may\n\
# follow. If 80 is used here, then 80%% of the characters must match exactly,\n\
# no insertion of a character, for the two articles to be put in the same\n\
# thread. eg. 'happy' and 'harpy' would match, but 'harpie', 'happie' and\n\
# 'harppy' would be threaded separately from 'happy'\n")
};

struct opttxt txt_thread_score = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Score of a thread"),
	N_("# Thread score\n\
# Possible values are (the default is marked with *):\n\
# * 0 = max\n\
#   1 = sum\n\
#   2 = average\n")
};

struct opttxt txt_sort_article_type = {
	N_("Sort articles by Subject, From, Date or Score. <SPACE> toggles & <CR> sets."),
	N_("Sort articles by"),
	N_("# Sort articles by ...\n\
# Possible values are (the default is marked with *):\n\
#   0 = nothing\n\
#   1 = Subject descending\n\
#   2 = Subject ascending\n\
#   3 = From descending\n\
#   4 = From ascending\n\
#   5 = Date descending\n\
# * 6 = Date ascending\n\
#   7 = Score descending\n\
#   8 = Score ascending\n\
#   9 = Lines descending\n\
#  10 = Lines ascending\n")
};

struct opttxt txt_sort_threads_type = {
	N_("Sort threads by Nothing or Score. <SPACE> toggles & <CR> sets."),
	N_("Sort threads by"),
	N_("# Sort thread by ...\n\
# Possible values are (the default is marked with *):\n\
#   0 = nothing\n\
# * 1 = Score descending\n\
#   2 = Score ascending\n")
};

struct opttxt txt_pos_first_unread = {
	N_("Put cursor at first/last unread art in groups. <SPACE> toggles & <CR> sets."),
	N_("Goto first unread article in group"),
	N_("# If ON put cursor at first unread art in group otherwise last art\n")
};

struct opttxt txt_show_only_unread_arts = {
	N_("Show all articles or only unread articles. <SPACE> toggles & <CR> sets."),
	N_("Show only unread articles"),
	N_("# If ON show only new/unread articles otherwise show all.\n")
};

struct opttxt txt_show_only_unread_groups = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show only groups with unread arts"),
	N_("# If ON show only subscribed to groups that contain unread articles.\n")
};

struct opttxt txt_kill_level = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Filter which articles"),
	N_("# Filter which articles\n\
# Possible values are (the default is marked with *):\n\
# * 0 = only kill unread articles\n\
#   1 = kill all articles and show in threads marked with K\n\
#   2 = kill all articles and never show them\n")
};

struct opttxt txt_tab_goto_next_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Tab goes to next unread article"),
	N_("# If ON the TAB command will go to next unread article at article viewer level\n")
};

struct opttxt txt_space_goto_next_unread = {
	N_("Scrolling with <SPACE> past the end of an art. jumps to the next unread one."),
	N_("Space goes to next unread article"),
	N_("# If ON the SPACE command will go to next unread article at article viewer\n\
# level when the end of the article is reached (rn-style pager)\n")
};

struct opttxt txt_pgdn_goto_next = {
	N_("Scrolling with <PGDN>/<DOWN> past the end of an art. jumps to the unread one."),
	N_("PgDn goes to next unread article"),
	N_("# If ON the PGDN or DOWN command will go to next unread article when\n\
# pressed at end of message\n")
};

struct opttxt txt_auto_list_thread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("List thread using right arrow key"),
	N_("# If ON automatically list thread when entering it using right arrow key.\n")
};

struct opttxt txt_art_marked_deleted = {
	N_("Enter character to indicate deleted articles. <CR> sets, <ESC> cancels."),
	N_("Character to show deleted articles"),
	N_("# Character used to show that an art was deleted (default 'D')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_inrange = {
	N_("Enter character to indicate articles in a range. <CR> sets, <ESC> cancels."),
	N_("Character to show inrange articles"),
	N_("# Character used to show that an art is in a range (default '#')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_return = {
	N_("Enter character to indicate that article will return. <CR> sets, <ESC> cancels."),
	N_("Character to show returning arts"),
	N_("# Character used to show that an art will return (default '-')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_selected = {
	N_("Enter character to indicate selected articles. <CR> sets, <ESC> cancels."),
	N_("Character to show selected articles"),
	N_("# Character used to show that an art was auto-selected (default '*')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_recent = {
	N_("Enter character to indicate recent articles. <CR> sets, <ESC> cancels."),
	N_("Character to show recent articles"),
	N_("# Character used to show that an art is recent (default 'o')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_unread = {
	N_("Enter character to indicate unread articles. <CR> sets, <ESC> cancels."),
	N_("Character to show unread articles"),
	N_("# Character used to show that an art is unread (default '+')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_read = {
	N_("Enter character to indicate read articles. <CR> sets, <ESC> cancels."),
	N_("Character to show read articles"),
	N_("# Character used to show that an art was read (default ' ')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_killed = {
	N_("Enter character to indicate killed articles. <CR> sets, <ESC> cancels."),
	N_("Character to show killed articles"),
	N_("# Character used to show that an art was killed (default 'K')\n\
# kill_level must be set accordingly, _ is turned into ' '\n")
};

struct opttxt txt_art_marked_read_selected = {
	N_("Enter character to indicate read selected articles. <CR> sets, <ESC> cancels."),
	N_("Character to show readselected arts"),
	N_("# Character used to show that an art was selected before read (default ':')\n\
# kill_level must be set accordingly, _ is turned into ' '\n")
};

struct opttxt txt_groupname_max_length = {
	N_("Enter maximum length of newsgroup names displayed. <CR> sets."),
	N_("Max. length of group names shown"),
	N_("# Maximum length of the names of newsgroups displayed\n")
};

struct opttxt txt_show_info = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show lines/score in listings"),
	N_("# What informations should be displayed in article/thread listing\n\
# Possible values are (the default is marked with *):\n\
#   0 = nothing\n\
# * 1 = lines\n\
#   2 = score\n\
#   3 = lines & score\n")
};

struct opttxt txt_scroll_lines = {
	N_("0 = full page scrolling, -1 = show previous last line as first on next page, -2 = half page"),
	N_("Number of lines to scroll in pager"),
	N_("# Number of lines that cursor-up/down will scroll in article pager\n\
# Possible values are (the default is marked with *):\n\
#  -2 = half-page scrolling\n\
#  -1 = the top/bottom line is carried over onto the next page\n\
#   0 = page-by-page (traditional behavior)\n\
# * 1 = line-by-line\n\
#   2 or greater = scroll by 2 or more lines (only in the pager)\n")
};

struct opttxt txt_show_signatures = {
	N_("Display signatures. <SPACE> toggles & <CR> sets."),
	N_("Display signatures"),
	N_("# If OFF don't show signatures when displaying articles\n")
};

struct opttxt txt_hide_uue = {
	N_("Display uuencoded data as tagged attachments. <SPACE> toggles & <CR> sets."),
	N_("Display uue data as an attachment"),
	N_("# Handling of uuencoded data in the pager\n\
# Possible values are (the default is marked with *):\n\
# * 0 = no, display raw uuencoded data\n\
#   1 = yes, uuencoded data will be condensed to a single tag line showing\n\
#       size and filename, similar to how MIME attachments are displayed\n\
#   2 = hide all, as for 1, but any line that looks like uuencoded data will\n\
#       be folded into a tag line.\n")
};

struct opttxt txt_tex2iso_conv = {
	N_("Decode German style TeX umlaut codes to ISO. <SPACE> toggles & <CR> sets."),
	N_("Display \"a as Umlaut-a"),
	N_("# If ON decode German style TeX umlaut codes to ISO and\n\
# show \"a as Umlaut-a, etc.\n")
};

struct opttxt txt_news_headers_to_display = {
	N_("Space separated list of header fields"),
	N_("Display these header fields (or *)"),
	N_("# Which news headers you wish to see. If you want to see _all_ the headers,\n\
# place an '*' as this value. This is the only way a wildcard can be used.\n\
# If you enter 'X-' as the value, you will see all headers beginning with\n\
# 'X-' (like X-Alan or X-Pape). You can list more than one by delimiting with\n\
# spaces. Not defining anything turns off this option.\n")
};

struct opttxt txt_news_headers_to_not_display = {
	N_("Space separated list of header fields"),
	N_("Do not display these header fields"),
	N_("# Same as 'news_headers_to_display' except it denotes the opposite.\n\
# An example of using both options might be if you thought X- headers were\n\
# A Good Thing(tm), but thought Alan and Pape were miscreants...well then you\n\
# would do something like this:\n\
# news_headers_to_display=X-\n\
# news_headers_to_not_display=X-Alan X-Pape\n\
# Not defining anything turns off this option.\n")
};

struct opttxt txt_alternative_handling = {
	N_("Do you want to enable automatic handling of multipart/alternative articles?"),
	N_("Skip multipart/alternative parts"),
	N_("# If ON strip multipart/alternative messages automatically\n")
};

#ifdef HAVE_COLOR
struct opttxt txt_quote_regex = {
	N_("A regex used to decide which lines to show in col_quote."),
	N_("Regex used to show quoted lines"),
	N_("# A regular expression that tin will use to decide which lines are\n\
# quoted when viewing articles. Quoted lines are shown in col_quote.\n\
# If you leave this blank, tin will use a built in default.\n")
};

struct opttxt txt_quote_regex2 = {
	N_("A regex used to decide which lines to show in col_quote2."),
	N_("Regex used to show twice quoted l."),
	N_("# A regular expression that tin will use to decide which lines are\n\
# quoted twice. Twice quoted lines are shown in col_quote2.\n\
# If you leave this blank, tin will use a built in default.\n")
};

struct opttxt txt_quote_regex3 = {
	N_("A regex used to decide which lines to show in col_quote3."),
	N_("Regex used to show >= 3 times q.l."),
	N_("# A regular expression that tin will use to decide which lines are\n\
# quoted >=3 times. >=3 times quoted lines are shown in col_quote3.\n\
# If you leave this blank, tin will use a built in default.\n")
};
#endif /* HAVE_COLOR */

struct opttxt txt_slashes_regex = {
	N_("A regex used to decide which words to show in col_markslashes."),
	N_("Regex used to highlight /slashes/"),
	N_("# A regular expression that tin will use to decide which words\n\
# bounded by '/' are to be shown in col_markslashes.\n\
# If you leave this blank, tin will use a built in default.\n")
};

struct opttxt txt_stars_regex = {
	N_("A regex used to decide which words to show in col_markstars."),
	N_("Regex used to highlight *stars*"),
	N_("# A regular expression that tin will use to decide which words\n\
# bounded by '*' are to be shown in col_markstars.\n\
# If you leave this blank, tin will use a built in default.\n")
};

struct opttxt txt_strokes_regex = {
	N_("A regex used to decide which words to show in col_markstroke."),
	N_("Regex used to highlight -strokes-"),
	N_("# A regular expression that tin will use to decide which words\n\
# bounded by '-' are to be shown in col_markstroke.\n\
# If you leave this blank, tin will use a built in default.\n")
};

struct opttxt txt_underscores_regex = {
	N_("A regex used to decide which words to show in col_markdash."),
	N_("Regex used to highlight _underline_"),
	N_("# A regular expression that tin will use to decide which words\n\
# bounded by '_' are to be shown in col_markdash.\n\
# If you leave this blank, tin will use a built in default.\n")
};

struct opttxt txt_strip_re_regex = {
	N_("A regex used to find Subject prefixes to remove.  Use '|' as separator."),
	N_("Regex with Subject prefixes"),
	N_("# A regular expression that tin will use to find Subject prefixes\n\
# which will be removed before showing the header.\n")
};

struct opttxt txt_strip_was_regex = {
	N_("A regex used to find Subject suffixes to remove.  Use '|' as separator."),
	N_("Regex with Subject suffixes"),
	N_("# A regular expression that tin will use to find Subject suffixes\n\
# which will be removed when replying or posting followup.\n")
};

struct opttxt txt_metamail_prog = {
	N_("Enter name and options for external MIME viewer, --internal for built-in viewer"),
	N_("MIME binary content viewer"),
	N_("# If --internal automatically use the built in MIME viewer for non-text\n\
# parts of articles.\n\
# Otherwise specify an external viewer program (eg, metamail) or leave blank\n\
# for no automatic viewing\n")
};

struct opttxt txt_ask_for_metamail = {
	N_("Confirm before starting non-text viewing program"),
	N_("Ask before using MIME viewer"),
	N_("# If ON tin will ask before using metamail to display MIME messages\n\
# this only happens if metamail_prog is set to something\n")
};

struct opttxt txt_catchup_read_groups = {
	N_("Ask to mark groups read when quitting. <SPACE> toggles & <CR> sets."),
	N_("Catchup read groups when quitting"),
	N_("# If ON ask user if read groups should all be marked read\n")
};

struct opttxt txt_group_catchup_on_exit = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Catchup group using left key"),
	N_("# If ON catchup group/thread when leaving with the left arrow key.\n")
};

struct opttxt txt_thread_catchup_on_exit = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Catchup thread by using left key"),
	""
};

struct opttxt txt_confirm_choice = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Which actions require confirmation"),
	N_("# What should we ask confirmation for.\n")
};

struct opttxt txt_mark_ignore_tags = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("'Mark article read' ignores tags"),
	N_("# If ON the 'Mark article read' function marks only the current article.\n")
};

struct opttxt txt_url_handler = {
	N_("Program to run to open URL's, <CR> sets, <ESC> cancels."),
	N_("Program that opens URL's"),
	N_("# The program used to open URL's. The actual URL will be appended\n")
};

struct opttxt txt_use_mouse = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use mouse in xterm"),
	N_("# If ON enable mouse key support on xterm terminals\n")
};

#ifdef HAVE_KEYPAD
struct opttxt txt_use_keypad = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use scroll keys on keypad"),
	N_("# If ON enable scroll keys on terminals that support it\n")
};
#endif /* HAVE_KEYPAD */

struct opttxt txt_getart_limit = {
	N_("Enter maximum number of article to get. <CR> sets."),
	N_("Number of articles to get"),
	N_("# Number of articles to get (0=no limit), if negative sets maximum number\n\
# of already read articles to be read before first unread one\n")
};

struct opttxt txt_recent_time = {
	N_("Enter number of days article is considered recent. <CR> sets."),
	N_("Article recentness time limit"),
	N_("# Number of days in which article is considered recent, (0=OFF)\n")
};

struct opttxt txt_wildcard = {
	N_("WILDMAT for normal wildcards, REGEX for full regular expression matching."),
	N_("Wildcard matching"),
	N_("# Wildcard matching\n\
# Possible values are (the default is marked with *):\n\
# * 0 = wildmat\n\
#   1 = regex\n")
};

struct opttxt txt_score_limit_kill = {
	N_("Enter minimal score before an article is marked killed. <CR> sets."),
	N_("Score limit (kill)"),
	N_("# Score limit before an article is marked killed\n")
};

struct opttxt txt_score_kill = {
	N_("Enter default score to kill articles. <CR> sets."),
	N_("Default score to kill articles"),
	N_("# Default score to kill articles\n")
};

struct opttxt txt_score_limit_select = {
	N_("Enter minimal score before an article is marked hot. <CR> sets."),
	N_("Score limit (select)"),
	N_("# Score limit before an article is marked hot\n")
};

struct opttxt txt_score_select = {
	N_("Enter default score to select articles. <CR> sets."),
	N_("Default score to select articles"),
	N_("# Default score to select articles\n")
};

#ifdef XFACE_ABLE
struct opttxt txt_use_slrnface = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use slrnface to show ''X-Face:''s"),
	N_("# If ON using slrnface(1) to interpret the ''X-Face:'' header.\n\
# Only useful when running in an xterm.\n")
};
#endif /* XFACE_ABLE */

#ifdef HAVE_COLOR
struct opttxt txt_use_color = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use ANSI color"),
	N_("# If ON using ANSI-color\n")
};

struct opttxt txt_col_normal = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Standard foreground color"),
	N_("# Standard foreground color\n\
# Default: -1 (default color)\n")
};

struct opttxt txt_col_back = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Standard background color"),
	N_("# Standard background color\n\
# Default: -1 (default color)\n")
};

struct opttxt txt_col_invers_bg = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color for inverse text (background)"),
	N_("# Color of background for inverse text\n\
# Default: 4 (blue)\n")
};

struct opttxt txt_col_invers_fg = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color for inverse text (foreground)"),
	N_("# Color of foreground for inverse text\n\
# Default: 7 (white)\n")
};

struct opttxt txt_col_text = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of text lines"),
	N_("# Color of text lines\n\
# Default: -1 (default color)\n")
};

struct opttxt txt_col_minihelp = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of mini help menu"),
	N_("# Color of mini help menu\n\
# Default: 3 (brown)\n")
};

struct opttxt txt_col_help = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of help text"),
	N_("# Color of help pages\n\
# Default: -1 (default color)\n")
};

struct opttxt txt_col_message = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of status messages"),
	N_("# Color of messages in last line\n\
# Default: 6 (cyan)\n")
};

struct opttxt txt_col_quote = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of quoted lines"),
	N_("# Color of quote-lines\n\
# Default: 2 (green)\n")
};

struct opttxt txt_col_quote2 = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of twice quoted line"),
	N_("# Color of twice quoted lines\n\
# Default: 3 (brown)\n")
};

struct opttxt txt_col_quote3 = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of =>3 times quoted line"),
	N_("# Color of >=3 times quoted lines\n\
# Default: 4 (blue)\n")
};

struct opttxt txt_col_head = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of article header lines"),
	N_("# Color of header-lines\n\
# Default: 2 (green)\n")
};

struct opttxt txt_col_newsheaders = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of actual news header fields"),
	N_("# Color of actual news header fields\n\
# Default: 9 (light red)\n")
};

struct opttxt txt_col_subject = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of article subject lines"),
	N_("# Color of article subject\n\
# Default: 6 (cyan)\n")
};

struct opttxt txt_col_response = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of response counter"),
	N_("# Color of response counter\n\
# Default: 2 (green)\n")
};

struct opttxt txt_col_from = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of sender (From:)"),
	N_("# Color of sender (From:)\n\
# Default: 2 (green)\n")
};

struct opttxt txt_col_title = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of help/mail sign"),
	N_("# Color of Help/Mail-Sign\n\
# Default: 4 (blue)\n")
};

struct opttxt txt_col_signature = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of signatures"),
	N_("# Color of signature\n\
# Default: 4 (blue)\n")
};

struct opttxt txt_col_urls = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighted URLs"),
	N_("# Color of highlighted URLs\n\
# Default: -1 (default color)\n")
};

struct opttxt txt_col_markstar = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighting with *stars*"),
	N_("# Color of word highlighting with *stars*\n\
# Default: 11 (yellow)\n")
};

struct opttxt txt_col_markdash = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighting with _dash_"),
	N_("# Color of word highlighting with _dash_\n\
# Default: 13 (light pink)\n")
};

struct opttxt txt_col_markslash = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighting with /slash/"),
	N_("# Color of word highlighting with /slash/\n\
# Default: 14 (light cyan)\n")
};

struct opttxt txt_col_markstroke = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighting with -stroke-"),
	N_("# Color of word highlighting with -stroke-\n\
# Default: 12 (light blue)\n")
};
#endif /* HAVE_COLOR */

struct opttxt txt_mono_markstar = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Attr. of highlighting with *stars*"),
	N_("# Attributes of word highlighting on mono terminals\n\
# Possible values are:\n\
#   0 = Normal\n\
#   1 = Underline\n\
#   2 = Best highlighting\n\
#   3 = Reverse video\n\
#   4 = Blinking\n\
#   5 = Half bright\n\
#   6 = Bold\n\n\
# Attribute of word highlighting with *stars*\n\
# Default: 6 (bold)\n")
};

struct opttxt txt_mono_markdash = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Attr. of highlighting with _dash_"),
	N_("# Attribute of word highlighting with _dash_\n\
# Default: 2 (best highlighting)\n")
};

struct opttxt txt_mono_markslash = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Attr. of highlighting with /slash/"),
	N_("# Attribute of word highlighting with /slash/\n\
# Default: 5 (half bright)\n")
};

struct opttxt txt_mono_markstroke = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Attr. of highlighting with -stroke-"),
	N_("# Attribute of word highlighting with -stroke-\n\
# Default: 3 (reverse video)\n")
};

struct opttxt txt_url_highlight = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("URL highlighting in message body"),
	N_("# Enable URL highlighting?\n")
};

struct opttxt txt_word_highlight = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Word highlighting in message body"),
	N_("# Enable word highlighting?\n")
};

struct opttxt txt_word_h_display_marks = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("What to display instead of mark"),
	N_("# Should the leading and ending stars and dashes also be displayed,\n\
# even when they are highlighting marks?\n\
# Possible values are (the default is marked with *):\n\
#   0 = no\n\
#   1 = yes, display mark\n\
# * 2 = print a space instead\n")
};

struct opttxt txt_wrap_column = {
	N_("Enter column number to wrap article lines to in the pager. <CR> sets."),
	N_("Page line wrap column"),
	N_("# Wrap article lines at column\n")
};

struct opttxt txt_wrap_on_next_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Wrap around threads on next unread"),
	N_("# If ON wrap around threads on searching next unread article\n")
};

struct opttxt txt_mail_address = {
	N_("Enter default mail address (and fullname). <CR> sets."),
	N_("Mail address (and fullname)"),
	N_("# User's mail address (and fullname), if not username@host (fullname)\n")
};

struct opttxt txt_prompt_followupto = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show empty Followup-To in editor"),
	N_("# If ON show empty Followup-To header when editing an article\n")
};

struct opttxt txt_sigfile = {
	N_("Enter path/! command/--none to create your default signature. <CR> sets."),
	N_("Create signature from path/command"),
	N_("# Signature path (random sigs)/file to be used when posting/replying\n\
# default_sigfile=file       appends file as signature\n\
# default_sigfile=!command   executes external command to generate a signature\n\
# default_sigfile=--none     don't append a signature\n")
};

struct opttxt txt_sigdashes = {
	N_("Prepend signature with \"-- \" on own line. <SPACE> toggles & <CR> sets."),
	N_("Prepend signature with \"-- \""),
	N_("# If ON prepend the signature with dashes '\\n-- \\n'\n")
};

struct opttxt txt_signature_repost = {
	N_("Add signature when reposting articles. <SPACE> toggles & <CR> sets."),
	N_("Add signature when reposting"),
	N_("# If ON add signature to reposted articles\n")
};

struct opttxt txt_quote_chars = {
	N_("Enter quotation marks, %s or %S for author's initials."),
	N_("Characters used as quote-marks"),
	N_("# Characters used in quoting to followups and replies.\n\
# '_' is replaced by ' ', %%s, %%S are replaced by author's initials.\n")
};

struct opttxt txt_quote_style = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Quoting behavior"),
	N_("# How quoting should be handled when following up or replying.\n\
# Possible values are (the default is marked with *):\n\
#   0 = Nothing special\n\
#   1 = Compress quotes\n\
#   2 = Quote signatures\n\
#   3 = Compress quotes, quote signatures\n\
#   4 = Quote empty lines\n\
# * 5 = Compress quotes, quote empty lines\n\
#   6 = Quote signatures, quote empty lines\n\
#   7 = Compress quotes, quote signatures, quote empty lines\n")
};

struct opttxt txt_news_quote_format = {
	N_("%A Addr %D Date %F Addr+Name %G Groupname %M Message-ID %N Name %C First Name"),
	N_("Quote line when following up"),
	N_("# Format of quote line when mailing/posting/following-up an article\n\
# %%A Address    %%D Date   %%F Addr+Name   %%G Groupname   %%M Message-ID\n\
# %%N Full Name  %%C First Name   %%I Initials\n")
};

struct opttxt txt_xpost_quote_format = {
	N_("%A Addr %D Date %F Addr+Name %G Groupname %M Message-ID %N Name %C First Name"),
	N_("Quote line when cross-posting"),
	""
};

struct opttxt txt_mail_quote_format = {
	N_("%A Addr %D Date %F Addr+Name %G Groupname %M Message-ID %N Name %C First Name"),
	N_("Quote line when mailing"),
	""
};

struct opttxt txt_advertising = {
	N_("If ON, include User-Agent: header. <SPACE> toggles & <CR> sets."),
	N_("Insert 'User-Agent:'-header"),
	N_("# If ON include advertising User-Agent: header\n")
};

#ifndef CHARSET_CONVERSION
struct opttxt txt_mm_charset = {
	N_("Enter charset name for MIME (e.g. US-ASCII, ISO-8859-1, EUC-KR), <CR> to set."),
	N_("MM_CHARSET"),
	N_("# Charset supported locally which is also used for MIME header and\n\
# Content-Type header.\n\
# If not set, the value of the environment variable MM_CHARSET is used.\n\
# Set to US-ASCII or compile time default if neither of them is defined.\n\
# If MIME_STRICT_CHARSET is defined at compile-time, charset other than\n\
# mm_charset is considered not displayable and represented as '?'.\n")
};
#else
struct opttxt txt_mm_network_charset = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("MM_NETWORK_CHARSET"),
	N_("# Charset used for MIME (Content-Type) header in postings.\n")
};
#endif /* !CHARSET_CONVERSION */

struct opttxt txt_mailbox_format = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Mailbox format"),
	N_("# Format of the mailbox.\n")
};

struct opttxt txt_post_mime_encoding = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("MIME encoding in news messages"),
	N_("# MIME encoding (8bit, base64, quoted-printable, 7bit) of the body\n\
# for mails and posts, if necessary. QP is efficient for most European\n\
# character sets (ISO-8859-X) with small fraction of non-US-ASCII chars,\n\
# while Base64 is more efficient for most 8bit East Asian, Greek, and\n\
# Russian charsets with a lot of 8bit characters.\n")
};

struct opttxt txt_post_8bit_header = {
	N_("Don't change unless you know what you are doing. <ESC> cancels."),
	N_("Use 8bit characters in news headers"),
	N_("# If ON, 8bit characters in news headers are NOT encoded.\n\
# default is OFF. Thus 8bit characters are encoded by default.\n\
# 8bit chars in header are encoded regardless of the value of this\n\
# parameter unless post_mime_encoding is 8bit as well.\n")
};

struct opttxt txt_post_process_view = {
	N_("Auto-view post-processed files <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("View post-processed files"),
	N_("# If set, post processed files will be opened in a viewer\n")
};

struct opttxt txt_mail_mime_encoding = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("MIME encoding in mail messages"),
	""
};

struct opttxt txt_mail_8bit_header = {
	N_("Don't change unless you know what you are doing. <ESC> cancels."),
	N_("Use 8bit characters in mail headers"),
	N_("# If ON, 8bit characters in mail headers are NOT encoded.\n\
# default is OFF. Thus 8bit characters are encoded by default.\n\
# 8bit chars in headers are encoded regardless of the value of this parameter\n\
# unless mail_mime_encoding is 8bit as well. Note that RFC 2822\n\
# prohibits 8bit characters in mail headers so that you are advised NOT to\n\
# turn it ON unless you have some compelling reason.\n")
};

struct opttxt txt_strip_blanks = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Strip blanks from ends of lines"),
	N_("# If ON strip blanks from ends of lines for faster display on slow terminals.\n")
};

#ifdef HAVE_ICONV_OPEN_TRANSLIT
struct opttxt txt_translit = {
	N_("If ON, use transliteration. <SPACE> toggles & <CR> sets."),
	N_("Transliteration"),
	N_("# If ON, use //TRANSLIT extension. This means that when a character cannot\n\
# be represented in the in the target character set, it can be approximated\n\
# through one or several similarly looking characters.\n")
};
#endif /* HAVE_ICONV_OPEN_TRANSLIT */

struct opttxt txt_auto_cc = {
	N_("Send you a carbon copy automatically. <SPACE> toggles & <CR> sets."),
	N_("Send you a cc automatically"),
	N_("# If ON automatically put your name in the Cc: field when mailing an article\n")
};

struct opttxt txt_auto_bcc = {
	N_("Send you a blind carbon copy automatically. <SPACE> toggles & <CR> sets."),
	N_("Send you a blind cc automatically"),
	N_("# If ON automatically put your name in the Bcc: field when mailing an article\n")
};

struct opttxt txt_spamtrap_warning_addresses = {
	N_("Enter address elements about which you want to be warned. <CR> sets."),
	N_("Spamtrap warning address parts"),
	N_("# A comma-delimited list of address-parts you want to be warned\n\
# about when trying to reply by email.\n")
};

struct opttxt txt_filter_days = {
	N_("Enter default number of days a filter entry will be valid. <CR> sets."),
	N_("No. of days a filter entry is valid"),
	N_("# Number of days a short term filter will be active\n")
};

struct opttxt txt_add_posted_to_filter = {
	N_("Add posted articles to filter. <SPACE> toggles & <CR> sets."),
	N_("Add posted articles to filter"),
	N_("# If ON add posted articles which start a new thread to filter for\n# highlighting follow-ups\n")
};

struct opttxt txt_maildir = {
	N_("The directory where articles/threads are to be saved in mailbox format."),
	N_("Mail directory"),
	N_("# (-m) directory where articles/threads are saved in mailbox format\n")
};

struct opttxt txt_batch_save = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Save articles in batch mode (-S)"),
	N_("# If ON articles/threads will be saved in batch mode when save -S\n\
# or mail (-M/-N) is specified on the command line\n")
};

struct opttxt txt_savedir = {
	N_("The directory where you want articles/threads saved."),
	N_("Directory to save arts/threads in"),
	N_("# Directory where articles/threads are saved\n")
};

struct opttxt txt_auto_save = {
	N_("Auto save article/thread by Archive-name: header. <SPACE> toggles & <CR> sets."),
	N_("Use Archive-name: header for save"),
	N_("# If ON articles/threads with Archive-name: in mail header will\n\
# be automatically saved with the Archive-name & part/patch no.\n")
};

struct opttxt txt_mark_saved_read = {
	N_("Mark saved articles/threads as read. <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Mark saved articles/threads as read"),
	N_("# If ON mark articles that are saved as read\n")
};

struct opttxt txt_post_process = {
	N_("Do post processing (eg. extract attachments) for saved articles."),
	N_("Post process saved articles"),
	N_("# Perform post processing (saving binary attachments) from saved articles.\n\
# Possible values are (the default is marked with *):\n\
# * 0 = no\n\
#   1 = extract shell archives (shar) only\n\
#   2 = yes\n")
};

struct opttxt txt_process_only_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Process only unread articles"),
	N_("# If ON only save/print/pipe/mail unread articles (tagged articles excepted)\n")
};

#ifndef DISABLE_PRINTING
struct opttxt txt_print_header = {
	N_("Print all or just part of header. <SPACE> toggles & <CR> sets."),
	N_("Print all headers when printing"),
	N_("# If ON print all of article header otherwise just the important lines\n")
};

struct opttxt txt_printer = {
	N_("The printer program with options that is to be used to print articles/threads."),
	N_("Printer program with options"),
	N_("# Print program with parameters used to print articles/threads\n"),
};
#endif /* !DISABLE_PRINTING */

struct opttxt txt_force_screen_redraw = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Force redraw after certain commands"),
	N_("# If ON a screen redraw will always be done after certain external commands\n")
};

struct opttxt txt_start_editor_offset = {
	N_("Start editor with line offset. <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Start editor with line offset"),
	N_("# If ON editor will be started with cursor offset into the file\n\
# otherwise the cursor will be positioned at the first line\n")
};

struct opttxt txt_editor_format = {
	N_("Enter %E for editor, %F for filename, %N for line-number, <CR> to set."),
	N_("Invocation of your editor"),
	N_("# Format of editor line including parameters\n\
# %%E Editor  %%F Filename  %%N Linenumber\n")
};

struct opttxt txt_inews_prog = {
	N_("Enter name and options for external-inews, --internal for internal inews"),
	N_("External inews"),
	N_("# If --internal use the built in mini inews for posting via NNTP\n# otherwise use an external inews program\n"),
};

struct opttxt txt_mailer_format = {
	N_("Enter %M for mailer, %S for subject, %T for to, %F for filename, <CR> to set."),
	N_("Invocation of your mail command"),
	N_("# Format of mailer line including parameters\n\
# %%M Mailer  %%S Subject  %%T To  %%F Filename\n\
# ie. to use elm as your mailer:    elm -s \"%%S\" \"%%T\" < %%F\n\
# ie. elm interactive          :    elm -i %%F -s \"%%S\" \"%%T\"\n")
};

struct opttxt txt_interactive_mailer = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use interactive mail reader"),
	N_("# Interactive mailreader\n\
# Possible values are (the default is marked with *):\n\
# * 0 = no interactive mailreader\n\
#   1 = use interactive mailreader with headers in file\n\
#   2 = use interactive mailreader without headers in file\n")
};

struct opttxt txt_unlink_article = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Remove ~/.article after posting"),
	N_("# If ON remove ~/.article after posting.\n")
};

struct opttxt txt_posted_articles_file = {
	N_("Filename for all posted articles, <CR> sets, no filename=do not save."),
	N_("Filename for posted articles"),
	N_("# Filename where to keep all postings (default posted)\n\
# If no filename is set then postings will not be saved\n")
};

struct opttxt txt_keep_dead_articles = {
	N_("Keep all failed articles in ~/dead.articles. <SPACE> toggles & <CR> sets."),
	N_("Keep failed arts in ~/dead.articles"),
	N_("# If ON keep all failed postings in ~/dead.articles\n")
};

struct opttxt txt_strip_newsrc = {
	N_("Do you want to strip unsubscribed groups from .newsrc"),
	N_("No unsubscribed groups in newsrc"),
	N_("# If ON strip unsubscribed groups from newsrc\n")
};

struct opttxt txt_strip_bogus = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Remove bogus groups from newsrc"),
	N_("# What to do with bogus groups in newsrc file\n\
# Possible values are (the default is marked with *):\n\
# * 0 = keep\n\
#   1 = remove\n\
#   2 = highlight with D on selection screen\n")
};

struct opttxt txt_reread_active_file_secs = {
	N_("Enter number of seconds until active file will be reread. <CR> sets."),
	N_("Interval in secs to reread active"),
	N_("# Time interval in seconds between rereading the active file (0=never)\n")
};

struct opttxt txt_auto_reconnect = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Reconnect to server automatically"),
	N_("# If ON automatically reconnect to NNTP server if the connection is broken\n")
};

struct opttxt txt_cache_overview_files = {
	N_("Create local copies of NNTP overview files. <SPACE> toggles & <CR> sets."),
	N_("Cache NNTP overview files locally"),
	N_("# If ON, create local copies of NNTP overview files.\n")
};

struct opttxt txt_date_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for display of dates"),
	N_("# Format string for date representation\n")
};

#ifdef HAVE_UNICODE_NORMALIZATION
struct opttxt txt_normalization_form = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Unicode normalization form"),
	N_("# Unicode normalization form\n\
# Possible values are (the default is marked with *):\n\
#   0 = None\n\
# * 1 = NFKC\n\
#   2 = NFKD\n\
#   3 = NFC\n\
#   4 = NFD\n")
};
#endif /* HAVE_UNICODE_NORMALIZATION */
#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
struct opttxt txt_render_bidi = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Render BiDi"),
	N_("# If ON, bi-directional text is rendered by tin\n")
};
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
