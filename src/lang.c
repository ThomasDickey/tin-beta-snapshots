/*
 *  Project   : tin - a Usenet reader
 *  Module    : lang.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 1998-10-17
 *  Notes     :
 *
 * Copyright (c) 1991-2000 Iain Lea <iain@bricbrac.de>
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

constext txt_1_resp[] = N_("1 Response%s");
constext txt_7bit[] = "7bit";
constext txt_8bit[] = "8bit";
constext txt_active_file_is_empty[] = N_("\n%s contains no newsgroups. Exiting.");
constext txt_added_groups[] = N_("Added %d %s");
constext txt_append_overwrite_quit[] = N_("File %s exists. a)ppend, o)verwrite, q)uit: ");
constext txt_all[] = N_("all ");
constext txt_art_cancel[] = N_("Article cancelled (deleted).");
constext txt_art_cannot_cancel[] = N_("Article cannot be cancelled (deleted).");
constext txt_art_newsgroups[] = N_("\nYour article:\n  \"%s\"\nwill be posted to the following %s:\n");
constext txt_art_not_posted[] = N_("Article not posted!");
constext txt_art_not_saved[] = N_("Article not saved");
constext txt_art_pager_com[] = N_("Article Level Commands (page %d of %d)");
constext txt_art_posted[] = N_("Article posted");
constext txt_art_rejected[] = N_("Article rejected (saved to %s)");
constext txt_art_saved_to[] = N_("Article saved to %s");
constext txt_art_thread_regex_tag[] = N_("a)rticle, t)hread, h)ot, p)attern, T)agged articles, q)uit: ");
constext txt_art_unavailable[] = N_("Article unavailable");
constext txt_art_parent_none[] = N_("Article has no parent");
constext txt_art_parent_killed[] = N_("Parent article has been killed");
constext txt_art_parent_unavail[] = N_("Parent article is unavailable");
constext txt_article_cancelled[] = "Article cancelled by author\n";
constext txt_article_plural[] = N_("articles");
constext txt_article_reposted[] = N_("This is a repost of the following article:");
constext txt_article_singular[] = N_("article");
constext txt_articles_mailed[] = N_("-- %d %s mailed --");
#ifndef DISABLE_PRINTING
	constext txt_articles_printed[] = N_("%d %s printed");
#endif /* !DISABLE_PRINTING */
constext txt_attach[] = N_("%*s[-- %s/%s, encoding %s, %d lines%s%s --]\n\n");
constext txt_uue[] = N_("[-- %suuencoded file, %d lines, name: %s --]\n\n");
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
constext txt_autoselecting_articles[] = N_("Autoselecting articles (use 'X' to see all unread) ...");
constext txt_bad_active_file[] = N_("Active file corrupt - %s");
constext txt_bad_article[] = N_("Article to be posted resulted in errors/warnings. q)uit, M)enu, e)dit: ");
constext txt_bad_command[] = N_("Bad command. Type 'h' for help.");
constext txt_base64[] = "base64";
constext txt_batch_update_unavail[] = N_("%s: Updating of index files not supported\n");
constext txt_batch_update_failed[] = N_("Failed to start background indexing process");
constext txt_begin_of_art[] = N_("*** Beginning of article ***");
constext txt_caching_disabled[] = N_("Overview caching not supported; %s is setuid");
constext txt_cancel_article[] = N_("Cancel (delete) or supersede (overwrite) article [%.*s]? (d/s/q): ");
constext txt_cancelling_art[] = N_("Cancelling article...");
constext txt_cannot_create_uniq_name[] = "Can't create unique tempfile-name";
constext txt_cannot_create[] = N_("Cannot create %s");
constext txt_cannot_find_base_art[] = N_("Can't find base article %d");
constext txt_cannot_get_nntp_server_name[] = N_("Cannot find NNTP server name");
constext txt_cannot_get_term_entry[] = N_("%s: Can't get entry for TERM\n");
constext txt_cannot_open[] = N_("Can't open %s");
#if defined(NNTP_ABLE) || defined(NNTP_ONLY)
	constext txt_cannot_open_active_file[] = N_("Can't open %s. Try %s -r to read news via NNTP.\n");
#endif /* NNTP_ABLE || NNTP_ONLY */
constext txt_cannot_open_for_saving[] = N_("Couldn't open %s for saving");
constext txt_cannot_post[] = N_("*** Posting not allowed ***");
constext txt_cannot_post_group[] = N_("Posting is not allowed to %s");
constext txt_cannot_write_index[] = N_("Can't write index %s");
constext txt_cannot_write_to_directory[] = N_("%s is a directory");
constext txt_catchup_all_read_groups[] = N_("Catchup all groups entered during this session? (y/n): ");
constext txt_catchup_update_info[] = N_("%s %d %s in %lu seconds\n");
constext txt_caughtup[] = N_("Caughtup");
constext txt_check_article[] = N_("Check Prepared Article");
constext txt_checking_new_groups[] = N_("Checking for new groups... ");
constext txt_checking_for_news[] = N_("Checking for news...");
constext txt_checksum_of_file[] = N_("Checksum of %s");
constext txt_return_key[] = N_("Press <RETURN> to continue...");
constext txt_color_off[] = N_("ANSI color disabled");
constext txt_color_on[] = N_("ANSI color enabled");
constext txt_command_failed[] = N_("Command failed: %s");
constext txt_connecting[] = N_("Connecting to %s...");
constext txt_connecting_port[] = N_("Connecting to %s:%d...");

#ifdef M_AMIGA
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2000 Iain Lea & Mark Tomlinson.";
#endif /* M_AMIGA */

#ifdef M_OS2
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2000 Iain Lea & Andreas Wrede.";
#endif /* M_OS2 */

#ifdef M_UNIX
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2000 Iain Lea.";
#endif /* M_UNIX */

#ifdef VMS
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2000 Iain Lea & Tod McQuillin & other.";
#endif /* VMS */

#ifdef WIN32
	constext txt_copyright_notice[] = "%s (c) Copyright 1991-2000 Iain Lea & Nigel Ellis.";
#endif /* WIN32 */

constext txt_creating_active[] = N_("Creating active file for saved groups...\n");
constext txt_creating_newsrc[] = N_("Creating newsrc file...\n");
constext txt_delete_processed_files[] = N_("Delete saved files that have been post processed? (y/n): ");
constext txt_deleting[] = N_("Deleting...");
constext txt_end_of_art[] = N_("*** End of article ***");
constext txt_end_of_arts[] = N_("*** End of articles ***");
constext txt_end_of_groups[] = N_("*** End of groups ***");
constext txt_end_of_thread[] = N_("*** End of thread ***");
constext txt_enter_message_id[] = N_("Enter Message-ID to goto> ");
constext txt_enter_next_thread[] = N_(" and enter next unread thread");
constext txt_enter_option_num[] = N_("Enter option number> ");
constext txt_enter_range[] = N_("Enter range [%s]> ");
constext txt_error_approved[] = N_("\nWarning: Approved: header used.\n");
constext txt_error_asfail[] = "%s: assertion failure: %s (%d): %s\n";
constext txt_error_bad_approved[] = N_("\nError: Bad address in Approved: header.\n");
constext txt_error_bad_from[] = N_("\nError: Bad address in From: header.\n");
constext txt_error_bad_replyto[] = N_("\nError: Bad address in Reply-To: header.\n");
constext txt_error_bad_msgidfqdn[] = N_("\nError: Bad FQDN in Message-ID: header.\n");
constext txt_error_copy_fp[] = "copy_fp() failed";
constext txt_error_corrupted_file[] = N_("Corrupted file %s");
constext txt_error_gnksa_internal[] = N_("Internal error in GNKSA routine - send bug report.\n");
constext txt_error_gnksa_langle[] = N_("Left angle bracket missing in route address.\n");
constext txt_error_gnksa_lparen[] = N_("Left parenthese missing in old-style address.\n");
constext txt_error_gnksa_rparen[] = N_("Right parenthese missing in old-style address.\n");
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
constext txt_error_gnksa_rn_paren[] = N_("Illegal character in realname.\nUnquoted words may not contain '()<>\\' in oldstyle addresses.\n");
constext txt_error_gnksa_rn_invalid[] = N_("Illegal character in realname.\nControl characters and unencoded 8bit characters > 127 are not allowed.\n");
constext txt_error_header_and_body_not_separate[] = N_("\nError: No blank line found after header.\n");
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
constext txt_error_header_line_comma[] = N_("\n\
Error: the \"%s:\" line has spaces  in it that MUST be removed.\n\
       The only allowable  space is the one  separating the colon (:)\n\
       from  the  contents.  Use a  comma  (,)  to separate  multiple\n\
       newsgroup names.\n");
constext txt_error_header_line_empty_subject[] = N_("\nError: the \"Subject:\" line is empty.\n");
constext txt_error_header_line_empty_newsgroups[] = N_("\nError: the \"Newsgroups:\" line lists no newsgroups.\n");
constext txt_error_header_line_groups_contd[] = N_("\n\
Error: The \"%s:\" line is  continued in  the next line.  Since\n\
       the line  may not  contain  whitespace,  this is  not allowed.\n\
       Please write all newsgroups into a single line.\n");
constext txt_error_header_line_missing_newsgroups[] = N_("\nError: the \"Newsgroups:\" line is missing from the article header.\n");
constext txt_error_header_line_missing_subject[] = N_("\nError: the \"Subject:\" line is missing from the article header.\n");
constext txt_error_header_line_missing_target[] = N_("\nError: the \"To:\" line is missing from the article header.\n");
constext txt_error_header_line_space[] = N_("\nError: Header on line %d does not have a space after the colon:\n%s\n");
constext txt_error_header_duplicate[] = N_("\nError: there are multiple (%d) \"%s\" lines in the header.\n");
#ifdef HAVE_METAMAIL
	constext txt_error_metamail_failed[] = N_("metamail failed: %s");
#endif /* HAVE_METAMAIL */
constext txt_error_no_domain_name[] = N_("Can't get a (fully-qualified) domain-name!\n");
constext txt_error_no_enter_permission[] = N_("No permissions to go into %s\n");
constext txt_error_no_from[] = N_("\nError: From: line missing.\n");
constext txt_error_no_read_permission[] = N_("No read permissions for %s\n");
constext txt_error_no_such_file[] = N_("File %s does not exist\n");
constext txt_error_no_write_permission[] = N_("No write permissions for %s\n");
constext txt_error_plural[] = N_("errors");
constext txt_error_sender_in_header_not_allowed[] = N_("\nError on line %d: \"Sender:\" header not allowed (it will be added for you)\n");
constext txt_error_singular[] = N_("error");
constext txt_exiting[] = N_("Exiting...");
constext txt_external_mail_done[] = N_("leaving external mail-reader");
constext txt_extracting_archive[] = N_("Extracting %s archive...");
constext txt_extracting_shar[] = N_("Extracting %s...");
constext txt_failed_to_connect_to_server[] = N_("Failed to connect to NNTP server %s. Exiting...");
constext txt_filesystem_full[] = N_("Error writing %s file. Filesystem full? File reset to previous state.");
constext txt_filesystem_full_backup[] = N_("Error making backup of %s file. Filesystem full?");
constext txt_filter_global_rules[] = N_("Filtering global rules (%d/%d)...");
constext txt_feed_pattern[] = N_("Enter wildcard pattern [%s]> ");
constext txt_followup_newsgroups[] = N_("\nYou requested followups to your article to go to the following %s:\n");
constext txt_followup_poster[] = N_("  %s\t Answers will be directed to you by mail.\n");
constext txt_global[] = N_("global ");

#if defined(HAVE_POLL) || defined(HAVE_SELECT)
	constext txt_group[] = N_("Group %.*s ('q' to quit)...");
#else
	constext txt_group[] = N_("Group %.*s...");
#endif /* HAVE_POLL || HAVE_SELECT */

constext txt_group_is_moderated[] = N_("Group %s is moderated. Continue? (y/n): ");
constext txt_group_plural[] = N_("groups");
constext txt_group_select_com[] = N_("Top Level Commands (page %d of %d)");
constext txt_group_selection[] = N_("Group Selection");
constext txt_group_singular[] = N_("group");

/* Multi-line strings won't work on help pager */
constext txt_help_disp[]  = N_("Display properties");
constext txt_help_disp_[] = N_("------------------");
constext txt_help_navi[]  = N_("Moving around");
constext txt_help_navi_[] = N_("-------------");
constext txt_help_ops[]   = N_("Group/thread/article operations");
constext txt_help_ops_[]  = N_("-------------------------------");
constext txt_help_misc [] = N_("Miscellaneous");
constext txt_help_misc_[] = N_("-------------");

#ifndef NO_SHELL_ESCAPE
	constext txt_help_shell[] = N_("!\t  shell escape");
#endif /* !NO_SHELL_ESCAPE */

#ifdef CASE_PROBLEM
	constext txt_help_BIGB[] = N_("B\t  search all articles for a given string (this may take some time)") N_("\t  (searches are case-insensitive and wrap around to all articles)");
	constext txt_help_BIGD[] = N_("D\t  cancel (delete) current article; must have been posted by you");
	constext txt_help_BIGI[] = N_("I\t  toggle inverse video");
	constext txt_help_BIGK[] = N_("K\t  mark current thread as read");
	constext txt_help_BIGL[] = N_("L\t  goto article chosen by Message-ID");
	constext txt_help_BIGM[] = N_("M\t  menu of configurable options");
	constext txt_help_BIGQ[] = N_("Q\t  quit");
	constext txt_help_BIGS[] = N_("S U\t  subscribe (U = unsubscribe) to groups that match pattern");
	constext txt_help_BIGT[] = N_("T\t  return to group selection level");
	constext txt_help_BIGU[] = N_("U\t  untag all tagged threads");
	constext txt_help_BIGW[] = N_("W\t  list articles posted by you (from posted file)");
	constext txt_help_BIGX[] = N_("X\t  toggle display of all/selected articles");
	constext txt_help_p_BIGU[] = N_("U\t  Browse URL's in article");
	constext txt_help_p_BIGV[] = N_("V\t  View/save multimedia attachments");
#else
	constext txt_help_B[] = N_("B\t  search all articles for a given string (this may take some time)");
	constext txt_help_B_[] = N_("\t  (searches are case-insensitive and wrap around to all articles)");
	constext txt_help_D[] = N_("D\t  cancel (delete) current article; must have been posted by you");
	constext txt_help_I[] = N_("I\t  toggle inverse video");
	constext txt_help_K[] = N_("K\t  mark current thread as read");
	constext txt_help_L[] = N_("L\t  goto article chosen by Message-ID");
	constext txt_help_M[] = N_("M\t  menu of configurable options");
	constext txt_help_Q[] = N_("Q\t  quit");
	constext txt_help_S[] = N_("S U\t  subscribe (U = unsubscribe) to groups that match pattern");
	constext txt_help_T[] = N_("T\t  return to group selection level");
	constext txt_help_U[] = N_("U\t  untag all tagged threads");
	constext txt_help_W[] = N_("W\t  list articles posted by you (from posted file)");
	constext txt_help_X[] = N_("X\t  toggle display of all/selected articles");
	constext txt_help_p_U[] = N_("U\t  Browse URL's in article");
	constext txt_help_p_V[] = N_("V\t  View/save multimedia attachments");
#endif /* CASE_PROBLEM */

constext txt_help_a[] = N_("a A\t  search for articles by author (a = forward, A = backward)");

constext txt_help_b[] = N_("<SPACE> b down (b = up) one page");
constext txt_help_bug[] = N_("R\t  report bug or comment via mail to %s");
constext txt_help_cC[] = N_("C\t  mark all articles as read and enter next group with unread articles");
constext txt_help_c[] = N_("c\t  mark all articles as read and return to group selection menu");
constext txt_help_sel_c[] = N_("c C\t  mark all articles in chosen group read (C = and enter next unread group)");
constext txt_help_thd_c[] = N_("c\t  mark thread as read and return to group index page");
constext txt_help_thd_C[] = N_("C\t  mark thread as read and enter next unread thread or group");
constext txt_help_thd_K[] = N_("K\t  mark article as read and move cursor to next unread article");

#ifdef HAVE_COLOR
	constext txt_help_color[] = N_("&\t  toggle color");
#endif /* HAVE_COLOR */

constext txt_help_colon[] = N_(":\t  skip next block of included text");
constext txt_help_ctrl_a[] = N_("^A\t  auto-select (hot) an article via a menu");
constext txt_help_ctrl_d[] = N_("^D ^U\t  down (^U = up) one page");
constext txt_help_ctrl_f[] = N_("^F ^B\t  down (^B = up) one page");

#ifdef HAVE_PGP_GPG
	constext txt_help_ctrl_g[] = N_("^G\t  perform PGP operations on article");
#endif /* HAVE_PGP_GPG */

constext txt_help_ctrl_h[] = N_("^H\t  show article headers");
constext txt_help_ctrl_k[] = N_("^K\t  kill an article via a menu");
constext txt_help_ctrl_l[] = N_("^L\t  redraw page");
constext txt_help_ctrl_n[] = N_("^N ^P\t  down (^P = up) one line");
constext txt_help_ctrl_o[] = N_("O ^O\t  post postponed articles");
constext txt_help_ctrl_t[] = N_("^T\t  toggle tabwidth 4 <-> 8");
constext txt_help_d[] = N_("d\t  cycle the display of authors email address, real name, both or neither");
constext txt_help_dash[] = N_("-\t  display last article viewed");
constext txt_help_e[] =  N_("e\t  edit article (mail-groups only)");
constext txt_help_equal[] = N_("=\t  select threads that match user specified pattern");
constext txt_help_esc[] = N_("<ESC>\t  escape from command prompt");
constext txt_help_g[] = N_("g\t  choose group by name");
constext txt_help_g_caret_dollar[] = N_("^ $\t  choose first ($ = last) group in list");
constext txt_help_g_cr[] = N_("<CR>\t  read chosen group");
constext txt_help_g_G[] = N_("G\t  toggle limit number of articles to get, and reload");
constext txt_help_g_ctrl_r[] = N_("^R\t  reset .newsrc (all available articles in groups marked unread)");
constext txt_help_g_d[] = N_("d\t  toggle display of group name only or group name plus description");
constext txt_help_g_hash[] = N_("#\t  choose range of groups to be affected by next command");
constext txt_help_g_num[] = N_("0 - 9\t  choose group by number");
constext txt_help_g_q[] = N_("q Q\t  quit");
constext txt_help_g_r[] = N_("r\t  toggle display to show all/unread subscribed groups");
constext txt_help_g_search[] = N_("/ ?\t  search forwards for a group name (? = backwards)");
constext txt_help_g_search_[] = N_("\t  (all searches are case-insensitive and wrap around)");
constext txt_help_g_T[] = N_("T\t  tag all parts of current multipart-message in order");
constext txt_help_g_tab[] = N_("n <TAB>\t  enter next group with unread news");
constext txt_help_g_x[] = N_("X\t  quit without saving configuration changes");
constext txt_help_g_y[] = N_("y\t  toggle display to show all/subscribed groups");
constext txt_help_g_z[] = N_("z Z\t  mark all articles in chosen group unread");
constext txt_help_h[] = N_("h H\t  get help (H = toggle mini help menu display)");
constext txt_help_hash[] = N_("#\t  choose range of articles to be affected by next command");
constext txt_help_i[] = N_("i\t  toggle info message in last line (subject/description)");
constext txt_help_i_caret_dollar[] = N_("^ $\t  choose first ($ = last) thread in list");
constext txt_help_i_at[] = N_("@\t  toggle all selections (all articles)");
constext txt_help_i_cr[] = N_("<CR>\t  read chosen article");
constext txt_help_i_dot[] = N_(".\t  toggle selection of thread");
constext txt_help_i_n[] = N_("n p\t  display next (p = previous) group");
constext txt_help_i_num[] = N_("0 - 9\t  choose thread by number");
constext txt_help_i_p[] = N_("N P\t  display next (P = previous) unread article");
constext txt_help_i_search[] = N_("/ ?\t  search for articles by Subject line (/ = forward, ? = backward)");
constext txt_help_i_star[] = N_("*\t  select thread");
constext txt_help_i_tab[] = N_("<TAB>\t  display next unread article");
constext txt_help_i_tilda[] = N_("~\t  undo all selections (all articles)");
constext txt_help_j[] = N_("j k\t  down (k = up) one line");
constext txt_help_quick_kill[] = N_("]\t  quickly kill an article using defaults");
constext txt_help_quick_select[] = N_("[\t  quickly auto-select (hot) an article using defaults");
constext txt_help_l[] = N_("l\t  list articles within current thread (bring up Thread sub-menu)");
constext txt_help_m[] = N_("m\t  move chosen group within list");
constext txt_help_n[] = N_("N\t  choose next group with unread news");

#ifndef DISABLE_PRINTING
	constext txt_help_o[] = N_("o\t  output article/thread/hot/pattern/tagged articles to printer");
#endif /* !DISABLE_PRINTING */

constext txt_help_p_caret_dollar[] = N_("^ $\t  display first ($ = last) page of article");
constext txt_help_p_at[] = N_("@\t  reverse article selections");
constext txt_help_p_ctrl_e[] = N_("^E\t  reply through mail to author quoting complete headers");
constext txt_help_p_ctrl_w[] = N_("^W\t  post followup to current article quoting complete headers");
constext txt_help_p_cr[] = N_("<CR>\t  display first article in next thread");
constext txt_help_p_d[] = N_("% d\t  toggle ROT-13 (basic decode) for current article");
constext txt_help_p_dot[] = N_(".\t  toggle article selection");
constext txt_help_p_f[] = N_("f F\t  post followup (F = don't copy text) to current article");
constext txt_help_p_g[] = N_("g G\t  display first (G = last) page of article");
/* constext txt_help_p_k[] = N_("k K\t  mark article (K = thread) as read and advance to next unread"); */
constext txt_help_p_k[] = N_("K\t  mark thread as read and advance to next unread");
constext txt_help_p_lb[] = N_("(\t  toggle display of uuencoded sections on/off");
constext txt_help_p_rb[] = N_(")\t  toggle display of sections hidden by a form-feed (^L) on/off");
constext txt_help_p_m[] = N_("m\t  mail article/thread/hot/pattern/tagged articles to someone");
constext txt_help_p_n[] = N_("n N\t  display next (N = next unread) article");
constext txt_help_p_num[] = N_("0 - 9\t  display article by number in current thread");
constext txt_help_p_p[] = N_("p P\t  display previous (P = previous unread) article");
constext txt_help_p_r[] = N_("r R\t  reply through mail (R = don't copy text) to author");
constext txt_help_p_s[] = N_("s\t  save article/thread/hot/pattern/tagged articles to file");

#ifdef CASE_PROBLEM
	constext txt_help_p_BIGS[] = N_("S\t  save tagged articles automatically without user prompts");
#else
	constext txt_help_p_S[] = N_("S\t  save tagged articles automatically without user prompts");
#endif /* CASE_PROBLEM */

constext txt_help_p_search[] = N_("/ ?\t  search forwards within this article (? = backwards)");
constext txt_help_p_star[] = N_("*\t  select article");
constext txt_help_p_tab[] = N_("<TAB>\t  display next unread article");
constext txt_help_p_u[] = N_("u\t  goto the article that this one followed up");
constext txt_help_p_tilda[] = N_("~\t  undo all selections in thread");
constext txt_help_p_z[] = N_("z Z\t  mark article (Z = thread) as unread");
#ifndef DONT_HAVE_PIPING
	constext txt_help_pipe[] = N_("|\t  pipe article/thread/hot/pattern/tagged articles into command");
#endif /* !DONT_HAVE_PIPING */
constext txt_help_plus[] = N_("+\t  select group (make \"hot\")");

constext txt_help_q[] = N_("q\t  return to previous menu");

constext txt_help_r[] = N_("r\t  toggle display of all/unread articles");
constext txt_help_s[] = N_("s u\t  subscribe (u = unsubscribe) to chosen group");
constext txt_help_semicolon[] = N_(");\t  select threads if at least one unread article is selected");
constext txt_help_t[] = N_("t\t  tag current article for reposting/mailing/piping/printing/saving");
constext txt_help_tex[] = N_("\"\t  toggle german TeX style decoding for current article");
constext txt_help_t_caret_dollar[] = N_("^ $\t  choose first ($ = last) article in list");
constext txt_help_t_cr[] = N_("<CR>\t  read chosen article");
constext txt_help_t_num[] = N_("0 - 9\t  choose article by number");
constext txt_help_thread[] = N_("< >\t  display first (> = last) article in current thread");
constext txt_help_u[] = N_("u\t  cycle through threading options available");

constext txt_help_v[] = N_("v\t  show version information");
constext txt_help_w[] = N_("w\t  post (write) article to current group");
constext txt_help_x[] = N_("x\t  repost chosen article to another group");
constext txt_help_y[] = N_("Y\t  reread active file to check for any new news");

#ifdef HAVE_COLOR
	constext txt_help__[] = N_("_\t  toggle word highlighting on/off");
#endif /* HAVE_COLOR */

constext txt_hit_space_for_more[] = N_("PgDn,End,<SPACE>,^D - page down. PgUp,Home,b,^U - page up. <CR>,q - quit");
constext txt_index_page_com[] = N_("Group Level Commands (page %d of %d)");
constext txt_info_add_kill[] = N_("Kill filter added");
constext txt_info_add_select[] = N_("Auto-selection filter added");
constext txt_info_do_postpone[] = N_("Storing article for later posting");
constext txt_info_postponed[] = N_("%d postponed %s, reuse with ^O...\n");
constext txt_info_nopostponed[] = N_("*** No postponed articles ***");
constext txt_info_not_subscribed[] = N_("You are not subscribed to this group");
constext txt_info_no_write[] = N_("Operation disabled in no-overwrite mode");
constext txt_info_x_conversion_note[] = N_("\
X-Conversion-Note: multipart/alternative contents have been removed.\n\
\tTo get the whole article, turn alternative handling OFF in the Option Menu\n");
constext txt_is_tex_encoded[]= N_("TeX2Iso encoded article");
constext txt_intro_page[] = N_("\nWelcome to tin, a full screen threaded Netnews reader. It can read news locally\n\
(ie. <spool>/news) or remotely (-r option)  from a NNTP (Network News Transport\n\
Protocol) server. tin -h lists the available command line options.\n\n\
Tin  has four newsreading levels, the newsgroup selection page, the group index\n\
page, the thread listing page and the article viewer. Help is available at each\n\
level by pressing the 'h' command.\n\n\
Move up/down by using the terminal arrow keys or 'j' and 'k'.  Use PgUp/PgDn or\n\
Ctrl-U and Ctrl-D to page up/down. Enter a newsgroup by pressing RETURN/TAB.\n\n\
Articles, threads, tagged articles or articles matching a pattern can be mailed\n\
('m' command), printed ('o' command), saved ('s' command), piped ('|' command).\n\
Use  the 'w' command  to post a  news article,  the 'f'/'F' commands  to post a\n\
follow-up to  an existing  news article  and the 'r'/'R' commands  to reply via\n\
mail to an existing news article author.  The 'M' command allows the operation\n\
of tin to be configured via a menu.\n\n\
For more information read the manual page, README, INSTALL, TODO and FTP files.\n\
Please send bug-reports/comments to %s with the 'R' command.\n");
constext txt_invalid_from[] = N_("Invalid  From: %s  line. Read the INSTALL file again.");
constext txt_invalid_sender[] = N_("Invalid  Sender:-header %s");
constext txt_inverse_off[] = N_("Inverse video disabled");
constext txt_inverse_on[] = N_("Inverse video enabled");
constext txt_keymap_missing_key[] = N_("Missing definition for %s\n");
constext txt_keymap_invalid_key[] = N_("Invalid key definition '%s'\n");
constext txt_keymap_invalid_name[] = N_("Invalid keyname '%s'\n");
constext txt_keymap_redef[] = N_("Redefined key %s '%s' -> '%s'\n");
constext txt_keymap_conflict[] = N_("Key '%s' is defined for both %s%s and %s%s\n");
constext txt_last_resp[] = N_("-- Last response --");

#ifdef HAVE_LIBUU
	constext txt_libuu_saved[] = N_("%d files successfully written from %d articles. %d %s occurred.");
	constext txt_libuu_success[] = N_("%s successfully decoded.");
	constext txt_libuu_error_decode[] = N_("Error decoding %s : ");
	constext txt_libuu_error_missing[] = N_("Missing parts.");
	constext txt_libuu_error_no_begin[] = N_("No beginning.");
	constext txt_libuu_error_no_end[] = N_("No end.");
	constext txt_libuu_error_no_data[] = N_("No data.");
	constext txt_libuu_error_unknown[] = N_("Unknown error.");
#endif /* HAVE_LIBUU */

constext txt_lines[] = N_("Lines %s  ");
constext txt_listing_archive[] = N_("Listing %s archive...");
constext txt_mail[] = "Mail";
constext txt_mail_art_to[] = N_("Mail article(s) to [%.*s]> ");
constext txt_mail_bug_report[] = N_("Mail bug report...");
constext txt_mail_bug_report_confirm[] = N_("Mail BUG REPORT to %s? (y/n): ");
constext txt_mailed[] = N_("Mailed");
constext txt_mailing_to[] = N_("Mailing to %s...");
constext txt_marked_as_unread[] = N_("%s marked as unread");
constext txt_mark_arts_read[] = N_("Mark all articles as read%s? (y/n): ");
constext txt_mark_group_read[] = N_("Mark group %.*s as read? (y/n): ");
constext txt_mark_thread_read[] = N_("Mark thread as read%s? (y/n): ");
constext txt_matching_cmd_line_groups[] = N_("Matching %s groups...");
constext txt_mini_group_1[] = N_("<n>=set current to n; TAB=next unread; /=search pattern; ^K)ill/select;");
constext txt_mini_group_2[] = N_("a)uthor search; c)atchup; j=line down; k=line up; K=mark read; l)ist thread;");
constext txt_mini_page_1[] = N_("<n>=set current to n; TAB=next unread; /=search pattern; ^K)ill/select;");
constext txt_mini_page_2[] = N_("a)uthor search; B)ody search; c)atchup; f)ollowup; K=mark read;");

#ifndef DISABLE_PRINTING
#	ifndef DONT_HAVE_PIPING
		constext txt_mini_group_3[] = N_("|=pipe; m)ail; o=print; q)uit; r=toggle all/unread; s)ave; t)ag; w=post");
		constext txt_mini_page_3[] = N_("|=pipe; m)ail; o=print; q)uit; r)eply mail; s)ave; t)ag; w=post");
#	else
		constext txt_mini_group_3[] = N_("m)ail; o=print; q)uit; r=toggle all/unread; s)ave; t)ag; w=post");
		constext txt_mini_page_3[] = N_("m)ail; o=print; q)uit; r)eply mail; s)ave; t)ag; w=post");
#	endif /* DONT_HAVE_PIPING */
#else
#	ifndef DONT_HAVE_PIPING
		constext txt_mini_group_3[] = N_("|=pipe; m)ail; q)uit; r=toggle all/unread; s)ave; t)ag; w=post");
		constext txt_mini_page_3[] = N_("|=pipe; m)ail; q)uit; r)eply mail; s)ave; t)ag; w=post");
#	else
		constext txt_mini_group_3[] = N_("m)ail; q)uit; r=toggle all/unread; s)ave; t)ag; w=post");
		constext txt_mini_page_3[] = N_("m)ail; q)uit; r)eply mail; s)ave; t)ag; w=post");
#	endif /* DONT_HAVE_PIPING */
#endif /* DISABLE_PRINTING */

constext txt_mini_select_1[] = N_("<n>=set current to n; TAB=next unread; g,/=search pattern; c)atchup;");
constext txt_mini_select_2[] = N_("j=line down; k=line up; h)elp; m)ove; q)uit; r=toggle all/unread;");
constext txt_mini_select_3[] = N_("s)ubscribe; S)ub pattern; u)nsubscribe; U)nsub pattern; y)ank in/out");
constext txt_mini_thread_1[] = N_("<n>=set current to n; TAB=next unread; c)atchup; d)isplay toggle;");
constext txt_mini_thread_2[] = N_("h)elp; j=line down; k=line up; q)uit; t)ag; z=mark unread");
constext txt_more[] = N_("--More--");
constext txt_moving[] = N_("Moving %s...");
constext txt_newsgroup[] = N_("Goto newsgroup [%s]> ");
constext txt_newsgroup_plural[] = N_("newsgroups");
constext txt_newsgroup_position[] = N_("Position %s in group list (1,2,..,$) [%d]> ");
constext txt_newsgroup_singular[] = N_("newsgroup");
constext txt_newsrc_again[] = N_("Try and save newsrc file again? (y/n): ");
constext txt_newsrc_nogroups[] = N_("Warning: No newsgroups were written to your newsrc file. Save aborted.");
constext txt_newsrc_saved[] = N_("newsrc file saved successfully.\n");
constext txt_next_resp[] = N_("-- Next response --");
constext txt_nntp_authorization_failed[] = N_("NNTP authorization password not found for %s");

#ifdef NNTP_ABLE
	constext txt_nntp_ok_goodbye[] = N_("205  Closing connection");
#endif /* NNTP_ABLE */

constext txt_no[] = N_("No ");
constext txt_no_arts[] = N_("*** No articles ***");
constext txt_no_arts_posted[] = N_("No articles have been posted");

#ifndef DONT_HAVE_PIPING
	constext txt_no_command[] = N_("No command");
#endif /* !DONT_HAVE_PIPING */

#ifdef HAVE_COLOR
#	ifdef USE_CURSES
		constext txt_no_colorterm[] = N_("Terminal does not support color");
#	endif /* USE_CURSES */
#endif /* HAVE_COLOR */

constext txt_no_description[] = N_("*** No description ***");
constext txt_no_filename[] = N_("No filename");
constext txt_no_group[] = N_("No group");
constext txt_no_groups[] = N_("*** No groups ***");
constext txt_no_groups_to_read[] = N_("No more groups to read");
constext txt_no_groups_to_yank_in[] = N_("No more groups to yank in");
constext txt_no_last_message[] = N_("No last message");
constext txt_no_mail_address[] = N_("No mail address");
constext txt_no_match[] = N_("No match");
constext txt_no_more_groups[] = N_("No more groups");
constext txt_no_newsgroups[] = N_("No newsgroups");
constext txt_no_next_unread_art[] = N_("No next unread article");
constext txt_no_prev_group[] = N_("No previous group");
constext txt_no_prev_unread_art[] = N_("No previous unread article");
constext txt_no_resp[] = N_("No responses%s");
constext txt_no_responses[] = N_("No responses");
constext txt_no_resps_in_thread[] = N_("No responses to list in current thread");
constext txt_no_search_string[] = N_("No search string");
constext txt_no_subject[] = N_("No subject");
constext txt_no_tagged_arts_to_save[] = N_("No articles tagged for saving");
constext txt_no_term_clear_eol[] = N_("%s: Terminal must have clear to end-of-line (ce)\n");
constext txt_no_term_clear_eos[] = N_("%s: Terminal must have clear to end-of-screen (cd)\n");
constext txt_no_term_clearscreen[] = N_("%s: Terminal must have clearscreen (cl) capability\n");
constext txt_no_term_cursor_motion[] = N_("%s: Terminal must have cursor motion (cm)\n");
constext txt_no_term_set[] = N_("%s: TERM variable must be set to use screen capabilities\n");
constext txt_no_viewer_found[] = N_("No viewer found for %s/%s\n");
constext txt_no_xover_support[] = N_("Your server does not support the NNTP XOVER or OVER command.\nTin will use local index files instead.\n");
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

constext txt_option_not_enabled[] = N_("Option not enabled. Recompile with %s.");
constext txt_options_menu[] = N_("Options Menu");
constext txt_out_of_memory[] = "%s: memory exhausted trying to allocate %d bytes in file %s line %d";

constext txt_pcre_error_at[] = N_("Error in regex: %s at pos. %d");
constext txt_pcre_error_num[] = N_("Error in regex: pcre internal error %d");
constext txt_pcre_error_text[] = N_("Error in regex: study - pcre internal error %s");

#ifdef HAVE_PGP_GPG
	constext txt_pgp_add[] = N_("Add key(s) to public keyring? ");
	constext txt_pgp_mail[] = N_("e)ncrypt, s)ign, b)oth, q)uit: ");
	constext txt_pgp_news[] = N_("s)ign, i) sign & include public key, q)uit: ");
	constext txt_pgp_not_avail[] = N_("PGP has not been set up (can't open %s)");
	constext txt_pgp_nothing[] = N_("Article not signed and no public keys found");
#endif /* HAVE_PGP_GPG */

#ifndef DONT_HAVE_PIPING
	constext txt_pipe[] = N_("Pipe");
	constext txt_pipe_to_command[] = N_("Pipe to command [%.*s]> ");
	constext txt_piping[] = N_("Piping...");
#else
#	ifdef VMS
		constext txt_pipe_to_command[] = N_("");
		constext txt_piping[] = N_("");
#	endif /* VMS */
	constext txt_piping_not_enabled[] = N_("Piping not enabled.");
#endif /* !DONT_HAVE_PIPING */

constext txt_posted_info_file[] = N_("# Summary of mailed/posted messages viewable by 'W' command from within tin.\n");
constext txt_prompt_fup_ignore[] = N_("Accept Followup-To? (p/i/q) ");
constext txt_prompt_unchanged_art[] = N_("Article unchanged, abort posting? (Y/n) ");
constext txt_prompt_unchanged_mail[] = N_("Article unchanged, abort mailing? (Y/n) ");
constext txt_prompt_see_postponed[] = N_("Do you want to see postponed articles (%d)? ");
constext txt_reading_keymap_file[] = N_("Reading keymap file...\n");
constext txt_repost[] = N_("Repost");
constext txt_save_attachment[] = N_("Save '%s' (%s/%s)? (y/n): ");
constext txt_screen_too_small[] = N_("%s: screen is too small\n");
constext txt_screen_too_small_exiting[] = N_("screen is too small, %s is exiting\n");
constext txt_servers_active[] = N_("servers active-file");
constext txt_starting_command[] = N_("Starting: (%s)");
constext txt_subscribe_pattern[] = N_("Enter wildcard subscribe pattern> ");
constext txt_superseding_art[] = N_("Superseding article ...");

#ifdef HAVE_COLOR
	constext txt_tinrc_colors[] = N_("# For color-adjust use the following numbers\n\
#  0-black       1-red         2-green        3-brown\n\
#  4-blue        5-pink        6-cyan         7-white\n\
# These are *only* for foreground:\n\
#  8-gray        9-lightred   10-lightgreen  11-yellow\n\
# 12-lightblue  13-lightpink  14-lightcyan   15-lightwhite\n\
# A '-1' is interpreted as default (foreground normally is white, and\n\
# background black)\n\n");
#endif /* HAVE_COLOR */

constext txt_tinrc_defaults[] = N_("# Default action/prompt strings\n");
constext txt_tinrc_filter[] = N_("# Defaults for quick (1 key) kill & auto-selection filters\n\
# header=NUM  0,1=Subject: 2,3=From: 4=Message-Id: & full References: line\n\
#             5=Message-Id: & last References: entry only\n\
#             6=Message-Id: entry only 7=Lines:\n\
# global=ON/OFF  ON=apply to all groups OFF=apply to current group\n\
# case=ON/OFF    ON=filter case sensitive OFF=ignore case\n\
# expire=ON/OFF  ON=limit to default_filter_days OFF=don't ever expire\n");
constext txt_tinrc_header[] = "# %s configuration file V%s\n\
# This file was automatically saved by %s %s %s (\"%s\")\n#\n\
# Do not edit while tin is running, since all your changes to this file\n\
# will be overwritten when you leave tin.\n#\n\
############################################################################\n\n";
constext txt_tinrc_info_in_last_line[] = N_("# If ON use print current subject or newsgroup description in the last line\n");

#ifdef LOCAL_CHARSET
	constext txt_tinrc_local_charset[] = N_("# Whether or not to automatically convert to a local charset that is\n\
# different from the one defined in mm_charset. Currently only NeXTstep is\n\
# supported. Set to OFF when logged in from a iso-8859-1 environment.\n");
#endif /* LOCAL_CHARSET */

constext txt_tinrc_newnews[] = N_("# Host & time info used for detecting new groups (don't touch)\n");
constext txt_tinrc_post_process_command[] = N_("# If set, command to be run after a successful uudecode\n");

constext txt_unsubscribe_pattern[] = N_("Enter wildcard unsubscribe pattern> ");
constext txt_uuencode_not_supported[] = N_("x-uuencode not supported yet");
constext txt_view_attachment[] = N_("View '%s' (%s/%s)? (y/n): ");

/*
 * Special value used to override option-list behavior
 */
constext txt_default[] = N_("Default");

/*
 * OFF ~ FALSE, ON ~ TRUE
 */
constext *txt_onoff[] = { "OFF", "ON" };

/*
 * NB: All the following arrays must match corresponding ordering in tin.h
 * Threading types
 */
constext *txt_thread[] = { "None", "Subject", "References", "Both Subject and References" };

/*
 * Whether to use wildmat() or regexec() for matching strings
 */
constext *txt_wildcard_type[] = { "WILDMAT", "REGEX" };

/*
 * How the From: line is displayed.
 */
constext *txt_show_from[] = { "None", "Address", "Full Name", "Address and Name" };

#ifdef HAVE_COLOR
	/*
	 * Which colors can be used.
	 */
	constext *txt_colors[] = {
	N_("Default"),
	N_("Black"),      N_("Red"),        N_("Green"),       N_("Brown"),
	N_("Blue"),       N_("Pink"),       N_("Cyan"),        N_("White"),
	N_("Gray"),       N_("Light Red"),  N_("Light Green"), N_("Yellow"),
	N_("Light Blue"), N_("Light Pink"), N_("Light Cyan"),  N_("Light White") };

	/*
	 * Which mark types can be used.
	 */
	constext *txt_marks[] = { N_("Nothing"), N_("Mark"), N_("Space"), N_("Space in Sigs") };
#endif /* HAVE_COLOR */

/*
 * MIME-Content-Transfer-Encodings.
 */
/* TODO can any of this go away ? */
constext *txt_mime_encodings[] = { txt_8bit, txt_base64, txt_quoted_printable, txt_7bit };

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
		N_("None"),
		N_("Shell archive"),
		N_("Uudecode"),
#ifdef M_AMIGA
		N_("Uudecode & list lharc archive"),
		N_("Uudecode & extract lharc archive"),
#else
		N_("Uudecode & list zoo archive"),
		N_("Uudecode & extract zoo archive"),
#endif /* M_AMIGA */
		N_("Uudecode & list zip archive"),
		N_("Uudecode & extract zip archive")
};

#if 0
/* moved to keymap.c because they may be localized */
char ch_post_process[] = { 'n', 's', 'u', 'l', 'e', 'L', 'E' };
#endif /* 0 */

constext *txt_sort_type[] = {
		N_("Nothing"),
		N_("Subject: field (descending)"),
		N_("Subject: field (ascending)"),
		N_("From: field (descending)"),
		N_("From: field (ascending)"),
		N_("Date: field (descending)"),
		N_("Date: field (ascending)"),
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
		N_("Kill all arts & show with K"),
		N_("Kill all arts and never show")
};

#ifdef M_AMIGA
	constext txt_choose_post_process_type[] = N_("Process n)one, s)har, u)ud, q)uit: ");
#else
	constext txt_choose_post_process_type[] = N_("Process n)one, s)har, u)ud, l)ist zoo, e)xt zoo, L)ist zip, E)xt zip, q)uit: ");
#endif /* M_AMIGA */

constext txt_post_a_followup[] = N_("Post a followup...");
constext txt_post_an_article[] = N_("Post an article...");
constext txt_post_error_ask_postpone[]=
N_("An error has occurred while posting the article. If you think that this\n\
error is temporary or otherwise correctable, you can postpone the article\n\
and pick it up again with ^O later.\n");
constext txt_post_history_menu[] = N_("Posted articles history (page %d of %d)");
constext txt_post_newsgroups[] = N_("Post to newsgroup(s) [%s]> ");
constext txt_post_processing[] = N_("Post processing...");
constext txt_post_processing_failed[] = N_("Post processing failed");
constext txt_post_processing_finished[] = N_("-- post processing completed --");
constext txt_post_subject[] = N_("Post subject [%s]> ");
constext txt_posting[] = N_("Posting article...");
constext txt_postpone_repost[] = N_("Post postponed articles [%.*s]? (y/Y/A/n/q): ");
constext txt_quick_filter_kill[] = N_("Add quick kill filter (y/n): ");
constext txt_quick_filter_select[] = N_("Add quick selection filter (y/n): ");
constext txt_quit[] = N_("Do you really want to quit? (y/n): ");
constext txt_quit_cancel[] = N_("e)dit cancel message, q)uit, d)elete (cancel) [%.*s]: ");
constext txt_quit_no_write[] = N_("Do you really want to quit without saving your configuration? (y/n): ");

#ifndef DISABLE_PRINTING
	constext txt_print[] = N_("Print");
	constext txt_printing[] = N_("Printing...");
#endif /* !DISABLE_PRINTING */

#ifdef HAVE_PGP_GPG
#	ifdef HAVE_ISPELL
		constext txt_quit_edit_post[] = N_("q)uit, e)dit, i)spell, g) pgp, p)ost, p(o)stpone: ");
		constext txt_quit_edit_send[] = N_("q)uit, e)dit, i)spell, g) pgp, s)end [%.*s]: ");
		constext txt_quit_edit_xpost[] = N_("q)uit, e)dit, i)spell, g) pgp, p)ost, p(o)stpone [%.*s]: ");
#	else
	constext txt_quit_edit_post[] = N_("q)uit, e)dit, g) pgp, p)ost, p(o)stpone: ");
	constext txt_quit_edit_send[] = N_("q)uit, e)dit, g) pgp, s)end [%.*s]: ");
	constext txt_quit_edit_xpost[] = N_("q)uit, e)dit, g) pgp, p)ost, p(o)stpone [%.*s]: ");
#	endif /* HAVE_ISPELL */
#else
#	ifdef HAVE_ISPELL
		constext txt_quit_edit_post[] = N_("q)uit, e)dit, i)spell, p)ost, p(o)stpone: ");
		constext txt_quit_edit_send[] = N_("q)uit, e)dit, i)spell, s)end [%.*s]: ");
		constext txt_quit_edit_xpost[] = N_("q)uit, e)dit, i)spell, p)ost, p(o)stpone [%.*s]: ");
#	else
		constext txt_quit_edit_post[] = N_("q)uit, e)dit, p)ost, p(o)stpone: ");
		constext txt_quit_edit_send[] = N_("q)uit, e)dit, s)end [%.*s]: ");
		constext txt_quit_edit_xpost[] = N_("q)uit, e)dit, p)ost, p(o)stpone [%.*s]: ");
#	endif /* HAVE_ISPELL */
#endif /* HAVE_PGP_GPG */

constext txt_quit_edit_postpone[] = N_("q)uit, e)dit, p(o)stpone: ");
constext txt_catchup_despite_tags[] = N_("You have tagged articles in this group - catchup anyway? (y/n): ");
constext txt_quit_despite_tags[] = N_("You have tagged articles in this group - quit anyway? (y/n): ");
constext txt_quoted_printable[] = "quoted-printable";
constext txt_range_invalid[] = N_("Invalid range - valid are '0-9.$' eg. 1-$");
constext txt_read_abort[] = N_("Do you want to abort this operation? (y/n): ");
constext txt_read_exit[] = N_("Do you want to exit tin immediately ? (y/n): ");
constext txt_read_resp[] = N_("Read response> ");
constext txt_reading_article[] = N_("Reading ('q' to quit)...");
constext txt_reading_arts[] = N_("Reading %sarticles...");
constext txt_reading_attributes_file[] = N_("Reading %sattributes file...\n");
constext txt_reading_config_file[] = N_("Reading %sconfig file...\n");
constext txt_reading_filter_file[] = N_("Reading filter file...\n");
constext txt_reading_input_history_file[] = N_("Reading input history file...\n");

#ifdef HAVE_MH_MAIL_HANDLING
	constext txt_reading_mail_active_file[] = N_("Reading mail active file... ");
	constext txt_reading_mailgroups_file[] = N_("Reading mailgroups file... ");
#endif /* HAVE_MH_MAIL_HANDLING */

constext txt_reading_groups[] = N_("Reading %s groups...");
constext txt_reading_news_active_file[] = N_("Reading groups from active file... ");
constext txt_reading_news_newsrc_file[] = N_("Reading groups from newsrc file... ");
constext txt_reading_newsgroups_file[] = N_("Reading newsgroups file... ");
constext txt_reading_newsrc[] = N_("Reading newsrc file...");
constext txt_reconnect_to_news_server[] = N_("Connection to news server has timed out. Reconnect? (y/n): ");
constext txt_remove_bogus[] = N_("Bogus group %s removed.");
constext txt_rename_error[] = N_("Error: rename %s to %s");
constext txt_reply_to_author[] = N_("Reply to author...");
constext txt_repost_an_article[] = N_("Reposting article...");
constext txt_repost_group[] = N_("Repost article(s) to group(s) [%s]> ");
constext txt_reset_newsrc[] = N_("Reset newsrc? (y/n): ");
constext txt_resp_redirect[] = N_("Responses have been directed to the following newsgroups");
constext txt_resp_to_poster[] = N_("Responses have been directed to poster. Mail/Post/Quit (m/p/q): ");
constext txt_resp_x_of_n[] = N_("RespNo %3d of %3d%s");
constext txt_at_s[] = N_(" at %s");
constext txt_save[] = N_("Save");
constext txt_save_config[] = N_("Save configuration before continuing? (y/n): ");
constext txt_save_filename[] = N_("Save filename [%s]> ");
constext txt_saved[] = N_("Saved");
constext txt_saved_arts[] = N_("-- %d %s saved --");
constext txt_saved_group[] = N_("%4d unread (%4d hot) %s in %s\n");
constext txt_saved_groupname[] = N_("Saved %s...\n");
constext txt_saved_nothing[] = N_("-- No unread articles: nothing saved! --");
constext txt_saved_pattern_to[] = N_("-- Saved pattern to %s - %s --");
constext txt_saved_summary[] = N_("\n%s %d %s from %d %s\n");
constext txt_saved_to_mailbox[] = N_("-- Saved to mailbox %s --");
constext txt_saving[] = N_("Saving...");
constext txt_screen_init_failed[] = N_("%s: Screen initialization failed");
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
constext txt_server_name_in_file_env_var[] = N_("Put the server name in the file %s,\nor set the environment variable NNTPSERVER");
#ifndef NO_SHELL_ESCAPE
	constext txt_shell_escape[] = N_("Enter shell command [%s]> ");
#endif /* !NO_SHELL_ESCAPE */
constext txt_skipping_newgroups[] = N_("Cannot move into new newsgroups. Subscribe first...");
constext txt_subscribed_num_groups[] = N_("subscribed to %d groups");
constext txt_subscribed_to[] = N_("Subscribed to %s");
constext txt_subscribing[] = N_("Subscribing... ");
constext txt_supersede_article[] = N_("Repost or supersede article(s) [%.*s]? (r/s/q): ");
constext txt_supersede_group[] = N_("Supersede article(s) to group(s) [%s]> ");
constext txt_suspended_message[] = N_("\nStopped. Type 'fg' to restart %s\n");
constext txt_tagged_art[] = N_("Tagged article");
constext txt_tagged_thread[] = N_("Tagged thread");
constext txt_testing_archive[] = N_("Testing %s archive...");
constext txt_there_is_no_news[] = N_("There is no news\n");
constext txt_thread_com[] = N_("Thread Level Commands (page %d of %d)");
constext txt_thread_marked_as_deselected[] = N_("Thread deselected");
constext txt_thread_marked_as_selected[] = N_("Thread selected");
constext txt_thread_not_saved[] = N_("Thread not saved");
constext txt_thread_page[] = N_("T %d of %d (p %d), %s:  %s");
constext txt_thread_resp_page[] = N_("T %d of %d, R %d/%d (p %d), %s:  %s");
constext txt_thread_saved_to_many[] = N_("Thread saved to %s - %s");
constext txt_thread_x_of_n[] = N_("%sThread %4s of %4s%s");
constext txt_threading_arts[] = N_("Threading articles...");

#ifdef HAVE_COLOR
	constext txt_toggled_high[] = N_("Toggled word highlighting %s");
#endif /* HAVE_COLOR */

constext txt_toggled_rot13[] = N_("Toggled rot13 encoding");
constext txt_toggled_tex2iso[] = N_("Toggled german TeX encoding %s");
constext txt_type_h_for_help[] = N_("           h=help\n");
constext txt_unread[] = N_("unread ");
constext txt_unsubscribed_num_groups[] = N_("unsubscribed from %d groups");
constext txt_unsubscribed_to[] = N_("Unsubscribed from %s");
constext txt_unsubscribing[] = N_("Unsubscribing... ");
constext txt_untagged_art[] = N_("Untagged article");
constext txt_untagged_thread[] = N_("Untagged thread");
constext txt_unthreading_arts[] = N_("Unthreading articles...");
constext txt_updated[] = "Updated";

#ifdef HAVE_METAMAIL
	constext txt_use_mime[] = N_("Use MIME display program for this message? (y/n): ");
#endif /* HAVE_METAMAIL */

constext txt_uudecoding[] = N_("Uudecoding %s");
constext txt_value_out_of_range[] = N_("\n%s%d out of range (0 - %d). Reset to 0");
constext txt_warn_art_line_too_long[] = N_("\nWarning: posting exceeds %d columns. Line %d is the first long one:\n%-100s\n");
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
         article.  If a signature is appended by that inews program it will\n\
         not be encoded properly.\n");
constext txt_warn_update[] = N_("\n\nYou are upgrading to tin %s from an earlier version.\n\
Some values in your configuration file have changed!\nRead WHATSNEW, etc...\n");

constext txt_warn_newsrc[] = N_("Warning: tin wrote fewer groups to your\n\t%s\n\
than it read at startup. If you didn't unsubscribe from %ld %s during\n\
this session this indicates an error and you should backup your %s\n\
before you start tin once again!\n");

#ifdef FORGERY
	constext txt_warn_cancel_forgery[] = N_("Read carefully!\n\n\
  You are about to cancel an article seemingly not written by you.  This will\n\
  wipe the article from lots of news servers throughout the world;\n\
  Usenet's majority  considers this  rather inappropriate,  to say the least.\n\
  Only press 'd'  if you are  absolutely positive  that you are ready to take\n\
  the rap.\n\nThis is the article you are about to cancel:\n\n");
#endif /* FORGERY */

#ifndef HAVE_FASCIST_NEWSADMIN
	constext txt_warn_followup_to_several_groups[] = N_("\nWarning: Followup-To set to more than one newsgroup!\n");
	constext txt_warn_missing_followup_to[] = N_("\nWarning: cross-posting to %d newsgroups and no Followup-To line!\n");
	constext txt_warn_not_in_newsrc[] = N_("\nWarning: \"%s\" is not in your newsrc, it may be invalid at this site!\n");
	constext txt_warn_not_valid_newsgroup[] = N_("\nWarning: \"%s\" is not a valid newsgroup at this site!\n");
#endif /* !HAVE_FASCIST_NEWSADMIN */

constext txt_warn_suspicious_mail[] = N_("Warning: this mail address may contain a spamtrap. Continue or Abort? (c/a) ");

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

constext txt_warn_sig_too_long[] = N_("\n\
Warning: Your signature  is longer than %d lines.  Since signatures usually do\n\
         not  transport any  useful information,  they should be as  short as\n\
         possible.\n");
constext txt_warn_multiple_sigs[] = N_("\nWarning: Found %d '-- \\n' lines, this may confuse some people.\n");
constext txt_warn_wrong_sig_format[] = N_("\nWarning: Signatures should start with '-- \\n' not with '--\\n'.\n");
constext txt_writing_attributes_file[] = N_("Writing attributes file...");
constext txt_x_resp[] = N_("%d Responses%s");
constext txt_yanking_all_groups[] = N_("Yanking in all groups...");
constext txt_yanking_sub_groups[] = N_("Yanking in subscribed to groups...");
constext txt_yes[] = N_("Yes");
constext txt_you_have_mail[] = N_("    You have mail\n");
constext txt_all_groups[] = N_("All groups");
constext txt_filter_text_type[] = N_("Apply pattern to    : ");
constext txt_from_line_only[] = N_("From: line (ignore case)        ");
constext txt_from_line_only_case[] = N_("From: line (case sensitive)     ");
constext txt_help_filter_from[] = N_("From: line to add to filter file. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_lines[] = N_("Linecount of articles to be filtered. < for less, > for more, = for equal.");
constext txt_help_filter_msgid[] = N_("Message-Id: line to add to filter file. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_subj[] = N_("Subject: line to add to filter file. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_text[] = N_("Enter text pattern to filter if Subject: & From: lines are not what you want.");
constext txt_help_filter_text_type[] = N_("Select where text pattern should be applied. <SPACE> toggles & <CR> sets.");
constext txt_help_filter_time[] = N_("Expiration time in days for the entered filter. <SPACE> toggles & <CR> sets.");
constext txt_help_kill_scope[] = N_("Apply kill only to current group or all groups. <SPACE> toggles & <CR> sets.");
constext txt_help_select_scope[] = N_("Apply select to current group or all groups. <SPACE> toggles & <CR> sets.");
constext txt_kill_from[] = N_("Kill From:     [%-*.*s] (y/n): ");
constext txt_kill_lines[] = N_("Kill Lines: (</>num): ");
constext txt_kill_menu[] = N_("Kill Article Menu");
constext txt_kill_msgid[] = N_("Kill Msg-Id:   [%-*.*s] (f/l/o/n): ");
constext txt_kill_scope[] = N_("Kill pattern scope  : ");
constext txt_kill_subj[] = N_("Kill Subject:  [%-*.*s] (y/n): ");
constext txt_kill_text[] = N_("Kill text pattern   : ");
constext txt_kill_time[] = N_("Kill time in days   : ");
constext txt_msgid_line_only[] = N_("Message-Id: line");
constext txt_quit_edit_save_kill[] = N_("q)uit e)dit s)ave kill description: ");
constext txt_quit_edit_save_select[] = N_("q)uit e)dit s)ave select description: ");
constext txt_select_from[] = N_("Select From    [%-*.*s] (y/n): ");
constext txt_select_lines[] = N_("Select Lines: (</>num): ");
constext txt_select_menu[] = N_("Auto-select Article Menu");
constext txt_select_msgid[] = N_("Select Msg-Id  [%-*.*s] (f/l/o/n): ");
constext txt_select_scope[] = N_("Select pattern scope: ");
constext txt_select_subj[] = N_("Select Subject [%-*.*s] (y/n): ");
constext txt_select_text[] = N_("Select text pattern : ");
constext txt_select_time[] = N_("Select time in days   : ");
constext txt_subj_line_only[] = N_("Subject: line (ignore case)     ");
constext txt_subj_line_only_case[] = N_("Subject: line (case sensitive)  ");
constext txt_time_default_days[] = N_("%d days");
constext txt_unlimited_time[] = N_("Unlimited");
constext txt_full[] = N_("Full");
constext txt_last[] = N_("Last");
constext txt_only[] = N_("Only");
constext txt_filter_file[] = N_("# Global & local filter file for the TIN newsreader\n#\n\
# Global format:\n\
#   group=STRING      Newsgroups list (e.g. comp.*,!*sources*)    [mandatory]\n\
#   type=NUM          0=kill 1=auto-select (hot) [mandatory]\n\
#   case=NUM          Compare=0 / ignore=1 case when filtering\n\
#   score=NUM         Score to give (e.g. 70)\n\
#   subj=STRING       Subject: line (e.g. How to be a wizard)\n\
#   from=STRING       From: line (e.g. *Craig Shergold*)\n\
#   msgid=STRING      Message-ID: line (e.g. <123@ether.net>) with full references\n\
#   msgid_last=STRING Message-ID: line (e.g. <123@ether.net>) with last reference only\n\
#   msgid_only=STRING Message-ID: line (e.g. <123@ether.net>) without references\n\
#   refs_only=STRING  References: line (e.g. <123@ether.net>) without Message-Id:\n\
#   lines=[<>]?NUM    Lines: line\n\
#   gnksa=[<>]?NUM    GNKSA parse_from() return code\n\
# either:\n\
#   xref_max=NUM      Maximum score (e.g. 5)\n\
#   xref_score=NUM,PATTERN score for pattern (e.g 0,*.answers)\n\
#   ...\n\
# or:\n\
#   xref=PATTERN      Kill pattern (e.g. alt.flame*)\n\
#   \n\
#   time=NUM          Filter period in days (default %d)\n#\n");
constext txt_filter_score[] = N_("Enter score for rule (default=100): ");
constext txt_filter_score_help[] = N_("Enter the score weight (range 0 < score <= 10000)");

constext txt_art_deleted[] = N_("Article deleted.");
constext txt_art_undeleted[] = N_("Article undeleted.");
constext txt_processing_mail_arts[] = N_("Processing mail messages marked for deletion.");
constext txt_processing_saved_arts[] = N_("Processing saved articles marked for deletion.");

#ifdef M_AMIGA
	constext txt_env_var_not_found[] = N_("Environment variable %s not found. Set and retry...");
#endif /* M_AMIGA */

#ifdef HAVE_FASCIST_NEWSADMIN
	constext txt_error_followup_to_several_groups[] = N_("\nError: Followup-To set to more than one newsgroup!\n");
	constext txt_error_missing_followup_to[] = N_("\nError: cross-posting to %d newsgroups and no Followup-To line!\n");
	constext txt_error_not_valid_newsgroup[] = N_("\nError: \"%s\" is not a valid newsgroup!\n");
#endif /* HAVE_FASCIST_NEWSADMIN */

#ifdef XHDR_XREF
	constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its XOVER information.\n\
Tin will try to use XHDR XREF instead (slows down things a bit).\n");
#else
	constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its XOVER information.\n");
#endif /* XHDR_XREF */

struct opttxt txt_beginner_level = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show mini menu & posting etiquette : "),
	N_("# If ON show a mini menu of useful commands at each level\n\
# and posting etiquette after composing an article\n")
};

struct opttxt txt_show_description = {
	N_("Show short description for each newsgroup. <SPACE> toggles & <CR> sets."),
	N_("Show description of each newsgroup : "),
	N_("# If ON show group description text after newsgroup name at\n\
# group selection level\n")
};

struct opttxt txt_show_author = {
	N_("Show Subject & From (author) fields in group menu. <SPACE> toggles & <CR> sets."),
	N_("In group menu, show author by      : "),
	N_("# Part of from field to display 0) none 1) address 2) full name 3) both\n")
};

struct opttxt txt_draw_arrow = {
	N_("Draw -> or highlighted bar for selection. <SPACE> toggles & <CR> sets."),
	N_("Draw -> instead of highlighted bar : "),
	N_("# If ON use -> otherwise highlighted bar for selection\n")
};

struct opttxt txt_inverse_okay = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use inverse video for page headers : "),
	N_("# If ON use inverse video for page headers at different levels\n")
};

struct opttxt txt_thread_articles = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Thread articles by                 : "),
	N_("# Thread articles on 0=(nothing) 1=(Subject) 2=(References) 3=(Both).\n")
};

struct opttxt txt_sort_article_type = {
	N_("Sort articles by Subject, From, Date or Score. <SPACE> toggles & <CR> sets."),
	N_("Sort article by                    : "),
	N_("# Sort articles by 0=(nothing) 1=(Subject descend) 2=(Subject ascend)\n\
# 3=(From descend) 4=(From ascend) 5=(Date descend) 6=(Date ascend)\n\
# 7=(Score descend) 8=(Score ascend).\n")
};

struct opttxt txt_pos_first_unread = {
	N_("Put cursor at first/last unread art in groups. <SPACE> toggles & <CR> sets."),
	N_("Goto first unread article in group : "),
	N_("# If ON put cursor at first unread art in group otherwise last art\n")
};

struct opttxt txt_show_only_unread_arts = {
	N_("Show all articles or only unread articles. <SPACE> toggles & <CR> sets."),
	N_("Show only unread articles          : "),
	N_("# If ON show only new/unread articles otherwise show all.\n")
};

struct opttxt txt_show_only_unread_groups = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show only groups with unread arts  : "),
	N_("# If ON show only subscribed to groups that contain unread articles.\n")
};

struct opttxt txt_kill_level = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Filter which articles              : "),
	N_("# 0=(Only kill unread articles)\n\
# 1=(Kill all articles and show in threads marked with K)\n\
# 2=(Kill all articles and never show them).\n")
};

struct opttxt txt_tab_goto_next_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Tab goes to next unread article    : "),
	N_("# If ON the TAB command will goto next unread article at article viewer level\n")
};

struct opttxt txt_space_goto_next_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Space goes to next unread article  : "),
	N_("# If ON the SPACE command will goto next unread article at article viewer\n\
# level when the end of the article is reached (rn-style pager)\n")
};

struct opttxt txt_pgdn_goto_next = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("PgDn goes to next article at EOF   : "),
	N_("# If ON the PGDN or DOWN command will goto next article when pressed\n\
# at end of message\n")
};

struct opttxt txt_auto_list_thread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("List thread using right arrow key  : "),
	N_("# If ON automatically list thread when entering it using right arrow key.\n")
};

struct opttxt txt_tab_after_X_selection = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Do tab after X automatically       : "),
	N_("# If ON a TAB command will be automatically done after the X command\n")
};

struct opttxt txt_art_marked_deleted = {
	N_("Enter character to indicate deleted articles. <CR> sets, <ESC> cancels."),
	N_("Character to show deleted articles : "),
	N_("# Character used to show that an art was deleted (default 'D')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_inrange = {
	N_("Enter character to indicate articles in a range. <CR> sets, <ESC> cancels."),
	N_("Character to show inrange articles : "),
	N_("# Character used to show that an art is in a range (default '#')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_return = {
	N_("Enter character to indicate that article will return. <CR> sets, <ESC> cancels."),
	N_("Character to show returning arts   : "),
	N_("# Character used to show that an art will return (default '-')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_selected = {
	N_("Enter character to indicate selected articles. <CR> sets, <ESC> cancels."),
	N_("Character to show selected articles: "),
	N_("# Character used to show that an art was auto-selected (default '*')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_recent = {
	N_("Enter character to indicate recent articles. <CR> sets, <ESC> cancels."),
	N_("Character to show recent articles  : "),
	N_("# Character used to show that an art is recent (default 'o')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_unread = {
	N_("Enter character to indicate unread articles. <CR> sets, <ESC> cancels."),
	N_("Character to show unread articles  : "),
	N_("# Character used to show that an art is unread (default '+')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_read = {
	N_("Enter character to indicate read articles. <CR> sets, <ESC> cancels."),
	N_("Character to show read articles    : "),
	N_("# Character used to show that an art was read (default ' ')\n\
# _ is turned into ' '\n")
};

struct opttxt txt_art_marked_killed = {
	N_("Enter character to indicate killed articles. <CR> sets, <ESC> cancels."),
	N_("Character to show killed articles  : "),
	N_("# Character used to show that an art was killed (default 'K')\n\
# kill_level must be set accordingly, _ is turned into ' '\n")
};

struct opttxt txt_art_marked_read_selected = {
	N_("Enter character to indicate read selected articles. <CR> sets, <ESC> cancels."),
	N_("Character to show readselected arts: "),
	N_("# Character used to show that an art was selected before read (default ':')\n\
# kill_level must be set accordingly, _ is turned into ' '\n")
};

struct opttxt txt_groupname_max_length = {
	N_("Enter maximum length of newsgroup names displayed. <CR> sets."),
	N_("Max. length of group names shown   : "),
	N_("# Maximum length of the names of newsgroups displayed\n")
};

struct opttxt txt_show_lines = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show no. of lines in thread listing: "),
	N_("# Show number of lines of first unread article in thread listing (ON/OFF)\n")
};

struct opttxt txt_show_score = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show score of article in listing   : "),
	N_("# Show score of article/thread in listing (ON/OFF)\n")
};

struct opttxt txt_full_page_scroll = {
	N_("Scroll half/full page of groups/articles. <SPACE> toggles & <CR> sets."),
	N_("Scroll full page (OFF=half page)   : "),
	N_("# If ON scroll full page of groups/articles otherwise half a page\n")
};

struct opttxt txt_show_last_line_prev_page = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show last line of previous page    : "),
	N_("# If ON show the last line of the previous page as first line of next page\n")
};

struct opttxt txt_show_signatures = {
	N_("Display signatures. <SPACE> toggles & <CR> sets."),
	N_("Display signatures                 : "),
	N_("# If OFF don't show signatures when displaying articles\n")
};

struct opttxt txt_hide_uue = {
	N_("Display uuencoded data as tagged attachments. <SPACE> toggles & <CR> sets."),
	N_("Display uue data as an attachment  : "),
	N_("# If ON uuencoded data will be condensed to a single tag line showing\n\
# size and filename, similar to how MIME attachments are displayed\n")
};

struct opttxt txt_news_headers_to_display = {
	N_("Space separated list of header fields"),
	N_("Display these header fields (or *) : "),
	N_("# Which news headers you wish to see. If you want to see _all_ the headers,\n\
# place an '*' as this value. This is the only way a wildcard can be used.\n\
# If you enter 'X-' as the value, you will see all headers beginning with\n\
# 'X-' (like X-Alan or X-Pape). You can list more than one by delimiting with\n\
# spaces. Not defining anything turns off this option.\n")
};

struct opttxt txt_news_headers_to_not_display = {
	N_("Space separated list of header fields"),
	N_("Do not display these header fields : "),
	N_("# Same as 'news_headers_to_display' except it denotes the opposite.\n\
# An example of using both options might be if you thought X- headers were\n\
# A Good Thing(tm), but thought Alan and Pape were miscreants...well then you\n\
# would do something like this:\n\
# news_headers_to_display=X-\n\
# news_headers_to_not_display=X-Alan X-Pape\n\
# Not defining anything turns off this option.\n")
};

struct opttxt txt_show_xcommentto = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Display X-Comment-To: header       : "),
	N_("# If ON, the realname in the X-Comment-To header is displayed in the\n\
# upper-right corner. You can use news_headers_to_display=X-Comment-To\n\
# instead of this\n")
};

struct opttxt txt_alternative_handling = {
	N_("Do you want to enable automatic handling of multipart/alternative articles?"),
	N_("Skip multipart/alternative parts   : "),
	N_("# If ON strip multipart/alternative messages automatically\n")
};

#ifdef HAVE_COLOR
struct opttxt txt_quote_regex = {
	N_("A regex used to decide which lines to show in col_quote."),
	N_("Regex used to show quoted lines    : "),
	N_("# A regular expression that tin will use to decide which lines are\n\
# quoted when viewing articles. Quoted lines are shown in col_quote.\n\
# If you leave this blank, tin will use a builtin default.\n")
};

struct opttxt txt_quote_regex2 = {
	N_("A regex used to decide which lines to show in col_quote2."),
	N_("Regex used to show twice quoted l. : "),
	N_("# A regular expression that tin will use to decide which lines are\n\
# quoted twice. Twice quoted lines are shown in col_quote2.\n\
# If you leave this blank, tin will use a builtin default.\n")
};

struct opttxt txt_quote_regex3 = {
	N_("A regex used to decide which lines to show in col_quote3."),
	N_("Regex used to show >= 3 times q.l. : "),
	N_("# A regular expression that tin will use to decide which lines are\n\
# quoted >=3 times. >=3 times quoted lines are shown in col_quote3.\n\
# If you leave this blank, tin will use a builtin default.\n")
};
#endif	/* HAVE_COLOR */

struct opttxt txt_strip_re_regex = {
	N_("A regex used to find Subject prefixes to remove.  Use '|' as separator."),
	N_("Regex with Subject prefixes        : "),
	N_("# A regular expression that tin will use to find Subject prefixes\n\
# which will be removed before showing the header.\n")
};

struct opttxt txt_strip_was_regex = {
	N_("A regex used to find Subject suffixes to remove.  Use '|' as separator."),
	N_("Regex with Subject suffixes        : "),
	N_("# A regular expression that tin will use to find Subject suffixes\n\
# which will be removed when replying or posting followup.\n")
};

#ifdef HAVE_METAMAIL
struct opttxt txt_use_metamail = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use metamail upon MIME articles    : "),
	N_("# If ON metamail can/will be used to display MIME articles\n")
};

struct opttxt txt_ask_for_metamail = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Ask before using metamail          : "),
	N_("# If ON tin will ask before using metamail to display MIME messages\n\
# this only happens if use_metamail is switched ON\n")
};
#endif	/* HAVE_METAMAIL */

struct opttxt txt_catchup_read_groups = {
	N_("Ask to mark groups read when quitting. <SPACE> toggles & <CR> sets."),
	N_("Catchup read groups when quitting  : "),
	N_("# If ON ask user if read groups should all be marked read\n")
};

struct opttxt txt_group_catchup_on_exit = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Catchup group using left key       : "),
	N_("# If ON catchup group/thread when leaving with the left arrow key.\n")
};

struct opttxt txt_thread_catchup_on_exit = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Catchup thread by using left key   : "),
	""
};

struct opttxt txt_confirm_action = {
	N_("Ask for command confirmation. <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Confirm commands before executing  : "),
	N_("# If ON confirm certain commands with y/n before executing\n")
};

struct opttxt txt_confirm_to_quit = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Confirm before quitting            : "),
	N_("# If ON confirm with y/n before quitting ('Q' never asks)\n")
};

struct opttxt txt_use_mouse = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use mouse in xterm                 : "),
	N_("# If ON enable mouse key support on xterm terminals\n")
};

#ifdef HAVE_KEYPAD
struct opttxt txt_use_keypad = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use scroll keys on keypad          : "),
	N_("# If ON enable scroll keys on terminals that support it\n")
};
#endif	/* HAVE_KEYPAD */

struct opttxt txt_use_getart_limit = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use getart_limit                   : "),
	N_("# If ON limit the number of articles to get\n")
};

struct opttxt txt_getart_limit = {
	N_("Enter maximum number of article to get. <CR> sets."),
	N_("Number of articles to get          : "),
	N_("# Number of articles to get (0=no limit), if negative sets maximum number\n\
# of already read articles to be read before first unread one\n")
};

struct opttxt txt_recent_time = {
	N_("Enter number of days article is considered recent. <CR> sets."),
	N_("Article recentness time limit      : "),
	N_("# Number of days in which article is considered recent, (0=OFF)\n")
};

#ifdef HAVE_COLOR
struct opttxt txt_use_color = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use ANSI color                     : "),
	N_("# If ON using ANSI-color\n")
};

struct opttxt txt_col_normal = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Standard foreground color          : "),
	N_("# Standard foreground color\n")
};

struct opttxt txt_col_back = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Standard background color          : "),
	N_("# Standard-Background-Color\n")
};

struct opttxt txt_col_invers_bg = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color for inverse text (background): "),
	N_("# Color of background for inverse text\n")
};

struct opttxt txt_col_invers_fg = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color for inverse text (foreground): "),
	N_("# Color of foreground for inverse text\n")
};

struct opttxt txt_col_text = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of text lines                : "),
	N_("# Color of text-lines\n")
};

struct opttxt txt_col_minihelp = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of mini help menu            : "),
	N_("# Color of mini help menu\n")
};

struct opttxt txt_col_help = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of help text                 : "),
	N_("# Color of help pages\n")
};

struct opttxt txt_col_message = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of status messages           : "),
	N_("# Color of messages in last line\n")
};

struct opttxt txt_col_quote = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of quoted lines              : "),
	N_("# Color of quote-lines\n")
};

struct opttxt txt_col_quote2 = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of twice quoted line         : "),
	N_("# Color of twice quoted lines\n")
};

struct opttxt txt_col_quote3 = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of =>3 times quoted line     : "),
	N_("# Color of >=3 times quoted lines\n")
};

struct opttxt txt_col_head = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of article header lines      : "),
	N_("# Color of header-lines\n")
};

struct opttxt txt_col_newsheaders = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of actual news header fields : "),
	N_("# Color of actual news header fields\n")
};

struct opttxt txt_col_subject = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of article subject lines     : "),
	N_("# Color of article subject\n")
};

struct opttxt txt_col_response = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of response counter          : "),
	N_("# Color of response counter\n")
};

struct opttxt txt_col_from = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of sender (From:)            : "),
	N_("# Color of sender (From:)\n")
};

struct opttxt txt_col_title = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of help/mail sign            : "),
	N_("# Color of Help/Mail-Sign\n")
};

struct opttxt txt_col_signature = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of signatures                : "),
	N_("# Color of signature\n")
};

struct opttxt txt_word_highlight = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Word highlighting in message body  : "),
	N_("# Enable word highlighting?\n")
};

struct opttxt txt_word_h_display_marks = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("What to display instead of mark    : "),
	N_("# Should the leading and ending stars and dashes also be displayed,\n\
# even when they are highlighting marks?\n\
# 0 - no    1 - yes, display mark    2 - print a space instead\n\
# 3 - print a space, but only in signatures\n")
};

struct opttxt txt_col_markstar = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighting with *stars* : "),
	N_("# Color of word highlighting. There are two possibilities for\n\
# in Articles: *stars* and _underdashes_\n")
};

struct opttxt txt_col_markdash = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighting with _dash_  : "),
	""
};
#endif	/* HAVE_COLOR */

struct opttxt txt_mail_address = {
	N_("Enter default mail address. <CR> sets."),
	N_("Mail address                       : "),
	N_("# User's mail address, if not username@host\n")
};

struct opttxt txt_prompt_followupto = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show empty Followup-To in editor   : "),
	N_("# If ON show empty Followup-To header when editing an article\n")
};

struct opttxt txt_sigfile = {
	N_("Enter path/! command/--none to create your default signature. <CR> sets."),
	N_("Create signature from path/command : "),
	N_("# Signature path (random sigs)/file to be used when posting/replying\n\
# default_sigfile=file       appends file as signature\n\
# default_sigfile=!command   executes external command to generate a signature\n\
# default_sigfile=--none     don't append a signature\n")
};

struct opttxt txt_sigdashes = {
	N_("Prepend signature with \"-- \" on own line. <SPACE> toggles & <CR> sets."),
	N_("Prepend signature with \"-- \"       : "),
	N_("# If ON prepend the signature with dashes '\\n-- \\n'\n")
};

struct opttxt txt_signature_repost = {
	N_("Add signature when reposting articles. <SPACE> toggles & <CR> sets."),
	N_("Add signature when reposting       : "),
	N_("# If ON add signature to reposted articles\n")
};

struct opttxt txt_quote_chars = {
	N_("Enter quotation marks, %s or %S for author's initials."),
	N_("Characters used as quote-marks     : "),
	N_("# Characters used in quoting to followups and replys.\n\
# '_' is replaced by ' ', %%s, %%S are replaced by author's initials.\n")
};

struct opttxt txt_quote_empty_lines = {
	N_("Quote empty lines. <SPACE> toggles & <CR> sets."),
	N_("Quote empty lines                  : "),
	N_("# If ON quote empty lines, too\n")
};

struct opttxt txt_quote_signatures = {
	N_("Quote signatures. <SPACE> toggles & <CR> sets."),
	N_("Quote signatures                   : "),
	N_("# If ON quote signatures, too\n")
};

struct opttxt txt_news_quote_format = {
	N_("%A Addr %D Date %F Addr+Name %G Groupname %M Message-Id %N Name %C First Name"),
	N_("Quote line when following up       : "),
	N_("# Format of quote line when mailing/posting/following-up an article\n\
# %%A Address    %%D Date   %%F Addr+Name   %%G Groupname   %%M Message-Id\n\
# %%N Full Name  %%C First Name\n")
};

struct opttxt txt_xpost_quote_format = {
	N_("%A Addr %D Date %F Addr+Name %G Groupname %M Message-ID %N Name %C First Name"),
	N_("Quote line when cross-posting      : "),
	""
};

struct opttxt txt_mail_quote_format = {
	N_("%A Addr %D Date %F Addr+Name %G Groupname %M Message-ID %N Name %C First Name"),
	N_("Quote line when mailing            : "),
	""
};

struct opttxt txt_advertising = {
	N_("If ON, include User-Agent: header. <SPACE> toggles & <CR> sets."),
	N_("Insert 'User-Agent:'-header        : "),
	N_("# If ON include advertising User-Agent: header\n")
};

struct opttxt txt_mm_charset = {
	N_("Enter charset name for MIME (e.g. US-ASCII, ISO-8859-1, EUC-KR), <CR> to set."),
	N_("MM_CHARSET                         : "),
	N_("# Charset supported locally which is also used for MIME header and\n\
# Content-Type header unless news and mail need to be encoded in other\n\
# charsets as in case of EUC-KR for Korean which needs to be converted to\n\
# ISO-2022-KR in mail message.\n\
# If not set, the value of the environment variable MM_CHARSET is used.\n\
# Set to US-ASCII or compile time default if neither of them is defined.\n\
# If MIME_STRICT_CHARSET is defined at compile-time, charset other than\n\
# mm_charset is considered not displayable and represented as '?'.\n")
};

struct opttxt txt_post_mime_encoding = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("MIME encoding in news messages     : "),
	N_("# MIME encoding (8bit, base64, quoted-printable, 7bit) of the body\n\
# for mails and posts, if necessary. QP is efficient for most European\n\
# character sets (ISO-8859-X) with small fraction of non-US-ASCII chars,\n\
# while Base64 is more efficient for most 8bit East Asian,Greek,and Russian\n\
# charsets with a lot of 8bit characters.\n\
# For EUC-KR, 7bit encoding specifies that EUC charsets be converted\n\
# to corresponding ISO-2022-KR. The same may be true of EUC-JP/CN.\n\
# Korean users should set post_mime_encoding to 8bit and mail_mime_encoding\n\
# to 7bit. With mm_charset to EUC-KR, post_mime_encoding set to 7bit does\n\
# NOT lead to conversion of EUC-KR into ISO-2022-KR in news-postings since\n\
# it's never meant to be used for Usenet news. Japanese always use\n\
# ISO-2022-JP for both news and mail. Automatic conversion of EUC-JP and\n\
# other 8bit Japanese encodings into ISO-2022-JP (even if 7bit is chosen)\n\
# is NOT yet implemented. (it may not be necessary at all as Japanese\n\
# terminal emulators and editors appear to have native support of\n\
# ISO-2022-JP). In case of Chinese, the situation seems to be more\n\
# complicated (different newsgroups and hierarchies for Chinese use different\n\
# MIME charsets and encodings) and no special handling is yet implemented.\n\
# Summing up 7bit does NOT have any effect on MIME charset other than EUC-KR\n")
};

struct opttxt txt_post_8bit_header = {
	N_("Don't change unless you know what you are doing. <ESC> cancels."),
	N_("Use 8bit characters in news headers: "),
	N_("# If ON, 8bit characters in news headers are NOT encoded.\n\
# default is OFF. Thus 8bit characters are encoded by default.\n\
# 8bit chars in header are encoded regardless of the value of this\n\
# parameter unless post_mime_encoding is 8bit as well.\n")
};

struct opttxt txt_mail_mime_encoding = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("MIME encoding in mail messages     : "),
	""
};

struct opttxt txt_mail_8bit_header = {
	N_("Don't change unless you know what you are doing. <ESC> cancels."),
	N_("Use 8bit characters in mail headers: "),
	N_("# If ON, 8bit characters in mail headers are NOT encoded.\n\
# default is OFF. Thus 8bit characters are encoded by default.\n\
# 8bit chars in headers are encoded regardless of the value of this parameter\n\
# unless mail_mime_encoding is 8bit as well. Note that RFC 1552/1651/1652\n\
# prohibit 8bit characters in mail headers so that you are advised NOT to\n\
# turn it ON unless you have some compelling reason.\n")
};

struct opttxt txt_strip_blanks = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Strip blanks of end of lines       : "),
	N_("# If ON strip blanks from end of lines to speedup display on slow terminals\n")
};

struct opttxt txt_auto_cc = {
	N_("Send you a carbon copy automatically. <SPACE> toggles & <CR> sets."),
	N_("Send you a cc automatically        : "),
	N_("# If ON automatically put your name in the Cc: field when mailing an article\n")
};

struct opttxt txt_auto_bcc = {
	N_("Send you a blind carbon copy automatically. <SPACE> toggles & <CR> sets."),
	N_("Send you a blind cc automatically  : "),
	N_("# If ON automatically put your name in the Bcc: field when mailing an article\n")
};

struct opttxt txt_spamtrap_warning_addresses = {
	N_("Enter address elements about which you want to be warned. <CR> sets."),
	N_("Spamtrap warning address parts     : "),
	N_("# A comma-delimited list of address-parts you want to be warned\n\
# about when trying to reply by email.\n")
};

struct opttxt txt_filter_days = {
	N_("Enter default number of days a filter entry will be valid. <CR> sets."),
	N_("No. of days a filter entry is valid: "),
	N_("# Num of days a short term filter will be active\n")
};

struct opttxt txt_add_posted_to_filter = {
	N_("Add subject of posted articles to filter. <SPACE> toggles & <CR> sets."),
	N_("Add posted articles to filter      : "),
	N_("# If ON add posted articles to filter for highlighting follow-ups\n")
};

struct opttxt txt_maildir = {
	N_("The directory where articles/threads are to be saved in mailbox format."),
	N_("Mail directory                     : "),
	N_("# (-m) directory where articles/threads are saved in mailbox format\n")
};

struct opttxt txt_save_to_mmdf_mailbox = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Save mail in MMDF style            : "),
	N_("# If ON save mail to a MMDF style mailbox (default is normal mbox format)\n")
};

struct opttxt txt_batch_save = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Save articles in batch mode (-S)   : "),
	N_("# If ON articles/threads will be saved in batch mode when save -S\n\
# or mail (-M/-N) is specified on the command line\n")
};

struct opttxt txt_savedir = {
	N_("The directory where you want articles/threads saved."),
	N_("Directory to save arts/threads in  : "),
	N_("# Directory where articles/threads are saved\n")
};

struct opttxt txt_auto_save = {
	N_("Auto save article/thread by Archive-name: header. <SPACE> toggles & <CR> sets."),
	N_("Use Archive-name: header for save  : "),
	N_("# If ON articles/threads with Archive-name: in mail header will\n\
# be automatically saved with the Archive-name & part/patch no.\n")
};

struct opttxt txt_mark_saved_read = {
	N_("Mark saved articles/threads as read. <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Mark saved articles/threads as read: "),
	N_("# If ON mark articles that are saved as read\n")
};

struct opttxt txt_post_process = {
	N_("Post process (ie. unshar) saved article/thread. <SPACE> toggles & <CR> sets."),
	N_("Post process saved art/thread with : "),
#ifdef M_AMIGA
	N_("# Type of post processing to perform after saving articles.\n\
# 0=(none) 1=(unshar) 2=(uudecode) 3=(uudecode & list lha)\n\
# 4=(uud & extract lha) 5=(uud & list zip) 6=(uud & extract zip)\n")
#else
	N_("# Type of post processing to perform after saving articles.\n\
# 0=(none) 1=(unshar) 2=(uudecode) 3=(uudecode & list zoo)\n\
# 4=(uud & extract zoo) 5=(uud & list zip) 6=(uud & extract zip)\n")
#endif /* M_AMIGA */
};

struct opttxt txt_process_only_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Process only unread articles       : "),
	N_("# If ON only save/print/pipe/mail unread articles (tagged articles excepted)\n")
};

#ifndef DISABLE_PRINTING
struct opttxt txt_print_header = {
	N_("Print all or just part of header. <SPACE> toggles & <CR> sets."),
	N_("Print all headers when printing    : "),
	N_("# If ON print all of article header otherwise just the important lines\n")
};

struct opttxt txt_printer = {
	N_("The printer program with options that is to be used to print articles/threads."),
	N_("Printer program with options       : "),
	N_("# Print program with parameters used to print articles/threads\n"),
};
#endif /* !DISABLE_PRINTING */

struct opttxt txt_wildcard = {
	N_("WILDMAT for normal wildcards, REGEX for full regular expression matching."),
	N_("Wildcard matching                  : "),
	N_("# Wildcard matching 0=(wildmat) 1=(regex)\n")
};

struct opttxt txt_force_screen_redraw = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Force redraw after certain commands: "),
	N_("# If ON a screen redraw will always be done after certain external commands\n")
};

struct opttxt txt_start_editor_offset = {
	N_("Start editor with line offset. <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Start editor with line offset      : "),
	N_("# If ON editor will be started with cursor offset into the file\n\
# otherwise the cursor will be positioned at the first line\n")
};

struct opttxt txt_editor_format = {
	N_("Enter %E for editor, %F for filename, %N for line-number, <CR> to set."),
	N_("Invocation of your editor          : "),
	N_("# Format of editor line including parameters\n\
# %%E Editor  %%F Filename  %%N Linenumber\n")
};

#if defined(NNTP_ABLE) || defined(NNTP_ONLY)
struct opttxt txt_use_builtin_inews = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use builtin inews                  : "),
	N_("# If ON use the builtin mini inews for posting via NNTP\n# otherwise use an external inews program\n"),
};
#endif /* NNTP_ABLE || NNTP_ONLY */

struct opttxt txt_mailer_format = {
	N_("Enter %M for mailer, %S for subject, %T for to, %F for filename, <CR> to set."),
	N_("Invocation of your mail command    : "),
	N_("# Format of mailer line including parameters\n\
# %%M Mailer  %%S Subject  %%T To  %%F Filename  %%U User (AmigaDOS)\n\
# ie. to use elm as your mailer:    elm -s \"%%S\" \"%%T\" < %%F\n\
# ie. elm interactive          :    elm -i %%F -s \"%%S\" \"%%T\"\n")
};

struct opttxt txt_use_mailreader_i = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use interactive mail reader        : "),
	N_("# Interactive mailreader: if ON mailreader will be invoked earlier for\n\
# reply so you can use more of its features (eg. MIME, pgp, ...)\n\
# this option has to suit default_mailer_format\n")
};

struct opttxt txt_unlink_article = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Remove ~/.article after posting    : "),
	N_("# If ON remove ~/.article after posting.\n")
};

struct opttxt txt_keep_posted_articles = {
	N_("Keep all posted articles in ~/Mail/file. <SPACE> toggles & <CR> sets."),
	N_("Keep posted articles in ~/Mail/file: "),
	N_("# If ON keep all postings in ~/Mail/file (see below)\n")
};

struct opttxt txt_keep_posted_articles_file = {
	N_("Filename for all posted articles. <CR> sets."),
	N_("Filename for posted articles       : "),
	N_("# Filename where to keep all postings (default posted)\n")
};

struct opttxt txt_keep_dead_articles = {
	N_("Keep all failed articles in ~/dead.articles. <SPACE> toggles & <CR> sets."),
	N_("Keep failed arts in ~/dead.articles: "),
	N_("# If ON keep all failed postings in ~/dead.articles\n")
};

struct opttxt txt_strip_newsrc = {
	N_("Do you want to strip unsubscribed groups from .newsrc"),
	N_("No unsubscribed groups in newsrc   : "),
	N_("# If ON strip unsubscribed groups from newsrc\n")
};

struct opttxt txt_strip_bogus = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Remove bogus groups from newsrc    : "),
	N_("# What to do with bogus groups in newsrc file\n# 0=(Keep) 1=(Remove) 2=(Highlight with D on selection screen).\n")
};

struct opttxt txt_reread_active_file_secs = {
	N_("Enter number of seconds until active file will be reread. <CR> sets."),
	N_("Interval in secs to reread active  : "),
	N_("# Time interval in seconds between rereading the active file (0=never)\n")
};

struct opttxt txt_auto_reconnect = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Reconnect to server automatically  : "),
	N_("# If ON automatically reconnect to NNTP server if the connection is broken\n")
};

struct opttxt txt_cache_overview_files = {
	N_("Create local copies of NNTP overview files. <SPACE> toggles & <CR> sets."),
	N_("Cache NNTP overview files locally  : "),
	N_("# If ON, create local copies of NNTP overview files.\n")
};

