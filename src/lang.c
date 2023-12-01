/*
 *  Project   : tin - a Usenet reader
 *  Module    : lang.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2023-11-28
 *  Notes     :
 *
 * Copyright (c) 1991-2024 Iain Lea <iain@bricbrac.de>
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

constext txt_1_resp[] = N_("1 Response");
constext txt_7bit[] = "7bit";
constext txt_8bit[] = "8bit";

constext txt_active_file_is_empty[] = N_("\n%s contains no newsgroups. Exiting.");
#if defined(NNTP_ABLE) && defined(HAVE_SELECT)
	constext txt_abort_reading[] = N_("Aborting read, please wait...");
#endif /* defined(NNTP_ABLE) && defined(HAVE_SELECT) */
constext txt_all[] = N_("all");
constext txt_all_groups[] = N_("All groups");
constext txt_append_overwrite_quit[] = N_("File %s exists. %s=append, %s=overwrite, %s=quit: ");
constext txt_art_cancel[] = N_("Article cancelled (deleted).");
#ifndef FORGERY
	constext txt_art_cannot_cancel[] = N_("Article cannot be cancelled (deleted).");
#endif /* !FORGERY */
constext txt_art_deleted[] = N_("Article deleted.");
constext txt_art_mailgroups[] = N_("\nYour article:\n  \"%s\"\nwill be mailed to the following address:\n  %s");
constext txt_art_newsgroups[] = N_("\nYour article:\n  \"%s\"\nwill be posted to the following %s:\n");
constext txt_art_not_posted[] = N_("Article not posted!");
constext txt_art_not_saved[] = N_("Article not saved");
constext txt_art_pager_com[] = N_("Article Level Commands");
constext txt_art_parent_none[] = N_("Article has no parent");
constext txt_art_parent_killed[] = N_("Parent article has been killed");
constext txt_art_parent_unavail[] = N_("Parent article is unavailable");
constext txt_art_posted[] = N_("Article posted: %s");
constext txt_art_rejected[] = N_("Article rejected (saved to %s)");
constext txt_art_thread_regex_tag[] = N_("%s=article, %s=thread, %s=range, %s=hot, %s=pattern, %s=tagged, %s=quit: ");
constext txt_art_unavailable[] = N_("Article unavailable");
constext txt_art_undeleted[] = N_("Article undeleted.");
constext txt_art_x_of_n[] = N_("Article %4d of %4d");
constext txt_article_cancelled[] = "Article cancelled by author.\n";
constext txt_article_plural[] = N_("articles");
constext txt_article_reposted[] = N_("This is a repost of the following article:");
constext txt_article_singular[] = N_("article");
constext txt_article_upper[] = N_("Article");
constext txt_articles_mailed[] = N_("-- %d %s mailed --");
constext txt_at_s[] = N_(" at %s");
constext txt_mime_boundary[] = "--%s\n";
constext txt_mime_boundary_end[] = "--%s--\n";
constext txt_mime_charset[] = N_("charset %s");
constext txt_mime_content_subtype[] = N_("content subtype %s");
constext txt_mime_content_type[] = N_("content type %s");
constext txt_mime_unsup_charset[] = N_("%*s[-- charset %s not supported --]\n");
constext txt_mime_description[] = N_("%*s[-- Description: %s --]\n");
constext txt_mime_encoding[] = N_("encoding %s");
constext txt_mime_hdr_c_disposition_inline[] = "Content-Disposition: inline\n";
constext txt_mime_hdr_c_transfer_encoding[] = "Content-Transfer-Encoding: %s\n";
constext txt_mime_hdr_c_type_msg_rfc822[] = "Content-Type: message/rfc822\n";
constext txt_mime_hdr_c_type_multipart_mixed[] = "Content-Type: multipart/mixed; boundary=\"%s\"\n";
constext txt_mime_hdr_c_type_text_plain_charset[] = "Content-Type: text/plain; charset=%s\n";
constext txt_mime_lang[] = N_("lang %s");
constext txt_mime_lines[] = N_("%s lines");
constext txt_mime_name[] = N_("name %s");
constext txt_mime_sep[] = N_(", ");
constext txt_mime_size[] = N_("size %s");
constext txt_mime_preamble_multipart_mixed[] = N_("This message has been composed in the 'multipart/mixed' MIME-format. If you\n\
are reading this prefix, your mail reader probably has not yet been modified\n\
to understand the new format, and some of what follows may look strange.\n\n");
constext txt_mime_version[] = "MIME-Version: %s\n";
constext txt_attachment_menu[] = N_("Attachment Menu");
constext txt_attachment_menu_com[] = N_("Attachment Menu Commands");
constext txt_attachment_no_name[] = N_("<no name>");
constext txt_attachment_saved[] = N_("Attachment saved successfully. (%s)");
constext txt_attachments_saved[] = N_("%d of %d attachments saved successfully.");
constext txt_attachment_select[] = N_("Select attachment> ");
constext txt_attachment_tagged[] = N_("Tagged attachment");
constext txt_attachments_tagged[] = N_("%d attachments tagged");
constext txt_attachment_untagged[] = N_("Untagged attachment");
/* do NOT localize the next string! */
constext txt_attrib_file_version[] = "# Group attributes file V%s for the TIN newsreader\n";
constext txt_attrib_file_header[] = N_("# Do not edit this comment block\n#\n");
constext txt_attrib_file_scope[] = N_("#  scope=STRING (eg. alt.*,!alt.bin*) [mandatory]\n");
constext txt_attrib_file_posted_to_filter[] = N_("#  add_posted_to_filter=ON/OFF\n");
constext txt_attrib_file_advertising[] = N_("#  advertising=ON/OFF\n");
constext txt_attrib_file_alt_handling[] = N_("#  alternative_handling=ON/OFF\n");
constext txt_attrib_file_metamail[] = N_("#  ask_for_metamail=ON/OFF\n");
constext txt_attrib_file_auto_cc_bcc[] = N_("#  auto_cc_bcc=NUM\n");
constext txt_attrib_file_auto_cc_bcc_opts[] = N_("#    0=No, 1=Cc, 2=Bcc, 3=Cc and Bcc\n");
constext txt_attrib_file_auto_list_thrd[] = N_("#  auto_list_thread=ON/OFF\n");
constext txt_attrib_file_auto_select[] = N_("#  auto_select=ON/OFF\n");
constext txt_attrib_file_batch_save[] = N_("#  batch_save=ON/OFF\n");
constext txt_attrib_file_date_fmt[] = N_("#  date_format=STRING (eg. %a, %d %b %Y %H:%M:%S)\n");
constext txt_attrib_file_delete_tmp[] = N_("#  delete_tmp_files=ON/OFF\n");
constext txt_attrib_file_editor_fmt[] = N_("#  editor_format=STRING (eg. %E +%N %F)\n");
constext txt_attrib_file_fcc[] = N_("#  fcc=STRING (eg. =mailbox)\n");
constext txt_attrib_file_followup_to[] = N_("#  followup_to=STRING\n");
constext txt_attrib_file_from[] = N_("#  from=STRING (just append wanted From:-line, don't use quotes)\n");
constext txt_attrib_file_grp_catchup[] = N_("#  group_catchup_on_exit=ON/OFF\n");
constext txt_attrib_file_grp_fmt[] = N_("#  group_format=STRING (eg. %n %m %R %L  %s  %F)\n");
constext txt_attrib_file_mail_8bit_hdr[] = N_("#  mail_8bit_header=ON/OFF\n");
constext txt_attrib_file_mail_mime_enc[] = N_("#  mail_mime_encoding=supported_encoding");
#ifdef HAVE_ISPELL
	constext txt_attrib_file_ispell[] = N_("#  ispell=STRING\n");
#endif /* HAVE_ISPELL */
constext txt_attrib_file_maildir[] = N_("#  maildir=STRING (eg. ~/Mail)\n");
constext txt_attrib_file_mailing_list[] = N_("#  mailing_list=STRING (eg. majordomo@example.org)\n");
constext txt_attrib_file_mime_types_to_save[] = N_("#  mime_types_to_save=STRING (eg. image/*,!image/bmp)\n");
constext txt_attrib_file_mark_ignore_tags[] = N_("#  mark_ignore_tags=ON/OFF\n");
constext txt_attrib_file_mark_saved_read[] = N_("#  mark_saved_read=ON/OFF\n");
constext txt_attrib_file_mime_forward[] = N_("#  mime_forward=ON/OFF\n");
#ifdef CHARSET_CONVERSION
	constext txt_attrib_file_mm_network_charset[] = N_("#  mm_network_charset=supported_charset");
	constext txt_attrib_file_undeclared_charset[] = N_("#  undeclared_charset=STRING (default is US-ASCII)\n");
#endif /* CHARSET_CONVERSION */
constext txt_attrib_file_hdr_to_disp[] = N_("#  news_headers_to_display=STRING\n");
constext txt_attrib_file_hdr_to_not_disp[] = N_("#  news_headers_to_not_display=STRING\n");
constext txt_attrib_file_quote_fmt[] = N_("#  news_quote_format=STRING\n");
constext txt_attrib_file_organization[] = N_("#  organization=STRING (if beginning with '/' read from file)\n");
constext txt_attrib_file_pos_first_unread[] = N_("#  pos_first_unread=ON/OFF\n");
constext txt_attrib_file_post_8bit_hdr[] = N_("#  post_8bit_header=ON/OFF\n");
constext txt_attrib_file_post_mime_enc[] = N_("#  post_mime_encoding=supported_encoding");
constext txt_attrib_file_post_proc_type[] = N_("#  post_process_type=NUM\n");
constext txt_attrib_file_post_proc_view[] = N_("#  post_process_view=ON/OFF\n");
constext txt_attrib_file_quick_kill_scope[] = N_("#  quick_kill_scope=STRING (e.g. talk.*)\n");
constext txt_attrib_file_quick_kill_expire[] = N_("#  quick_kill_expire=ON/OFF\n");
constext txt_attrib_file_quick_kill_case[] = N_("#  quick_kill_case=ON/OFF\n");
constext txt_attrib_file_quick_kill_hdr[] = N_("#  quick_kill_header=NUM\n");
constext txt_attrib_file_quick_kill_hdr_0_1[] = N_("#    0=Subject: (case sensitive)  1=Subject: (ignore case)\n");
constext txt_attrib_file_quick_kill_hdr_2_3[] = N_("#    2=From: (case sensitive)     3=From: (ignore case)\n");
constext txt_attrib_file_quick_kill_hdr_4[] = N_("#    4=Message-ID: & full References: line\n");
constext txt_attrib_file_quick_kill_hdr_5[] = N_("#    5=Message-ID: & last References: entry only\n");
constext txt_attrib_file_quick_kill_hdr_6[] = N_("#    6=Message-ID: entry only     7=Lines:\n");
constext txt_attrib_file_quick_select_scope[] = N_("#  quick_select_scope=STRING\n");
constext txt_attrib_file_quick_select_expire[] = N_("#  quick_select_expire=ON/OFF\n");
constext txt_attrib_file_quick_select_case[] = N_("#  quick_select_case=ON/OFF\n");
constext txt_attrib_file_quick_select_hdr[] = N_("#  quick_select_header=NUM\n");
constext txt_attrib_file_quick_select_hdr_0_1[] = N_("#    0=Subject: (case sensitive)  1=Subject: (ignore case)\n");
constext txt_attrib_file_quick_select_hdr_2_3[] = N_("#    2=From: (case sensitive)     3=From: (ignore case)\n");
constext txt_attrib_file_quick_select_hdr_4[] = N_("#    4=Message-ID: & full References: line\n");
constext txt_attrib_file_quick_select_hdr_5[] = N_("#    5=Message-ID: & last References: entry only\n");
constext txt_attrib_file_quick_select_hdr_6[] = N_("#    6=Message-ID: entry only     7=Lines:\n");
constext txt_attrib_file_quote_chars[] = N_("#  quote_chars=STRING (%I for initials)\n");
#ifndef DISABLE_PRINTING
	constext txt_attrib_file_print_hdr[] = N_("#  print_header=ON/OFF\n");
#endif /* !DISABLE_PRINTING */
constext txt_attrib_file_process_only_unread[] = N_("#  process_only_unread=ON/OFF\n");
constext txt_attrib_file_prompt_followup[] = N_("#  prompt_followupto=ON/OFF\n");
constext txt_attrib_file_savedir[] = N_("#  savedir=STRING (eg. ~user/News)\n");
constext txt_attrib_file_savefile[] = N_("#  savefile=STRING (eg. =linux)\n");
constext txt_attrib_file_sigfile[] = N_("#  sigfile=STRING (eg. $var/sig)\n");
constext txt_attrib_file_show_author[] = N_("#  show_author=NUM\n");
constext txt_attrib_file_show_signatures[] = N_("#  show_signatures=ON/OFF\n");
constext txt_attrib_file_show_art_score[] = N_("#  show_art_score=ON/OFF\n");
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	constext txt_attrib_file_suppress_soft_hyphens[] = N_("#  suppress_soft_hyphens=ON/OFF\n");
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
constext txt_attrib_file_show_only_unread[] = N_("#  show_only_unread_arts=ON/OFF\n");
constext txt_attrib_file_sigdashes[] = N_("#  sigdashes=ON/OFF\n");
constext txt_attrib_file_signature_repost[] = N_("#  signature_repost=ON/OFF\n");
constext txt_attrib_file_sort_art_type[] = N_("#  sort_article_type=NUM\n");
constext txt_attrib_file_sort_thrd_type[] = N_("#  sort_threads_type=NUM\n");
constext txt_attrib_file_tex2iso[] = N_("#  tex2iso_conv=ON/OFF\n");
constext txt_attrib_file_thrd_catchup[] = N_("#  thread_catchup_on_exit=ON/OFF\n");
constext txt_attrib_file_thrd_arts[] = N_("#  thread_articles=NUM");
constext txt_attrib_file_thrd_fmt[] = N_("#  thread_format=STRING (eg. %n %m [%L]  %T  %F)\n");
constext txt_attrib_file_thrd_perc[] = N_("#  thread_perc=NUM\n");
constext txt_attrib_file_trim_art_body[] = N_("#  trim_article_body=NUM\n");
constext txt_attrib_file_trim_art_body_0[] = N_("#    0 = Don't trim article body\n");
constext txt_attrib_file_trim_art_body_1[] = N_("#    1 = Skip leading blank lines\n");
constext txt_attrib_file_trim_art_body_2[] = N_("#    2 = Skip trailing blank lines\n");
constext txt_attrib_file_trim_art_body_3[] = N_("#    3 = Skip leading and trailing blank lines\n");
constext txt_attrib_file_trim_art_body_4[] = N_("#    4 = Compact multiple blank lines between text blocks\n");
constext txt_attrib_file_trim_art_body_5[] = N_("#    5 = Compact multiple blank lines between text blocks and skip\n#        leading blank lines\n");
constext txt_attrib_file_trim_art_body_6[] = N_("#    6 = Compact multiple blank lines between text blocks and skip\n#        trailing blank lines\n");
constext txt_attrib_file_trim_art_body_7[] = N_("#    7 = Compact multiple blank lines between text blocks and skip\n#        leading and trailing blank lines\n");
constext txt_attrib_file_verbatim_handling[] = N_("#  verbatim_handling=ON/OFF\n");
#ifdef HAVE_COLOR
	constext txt_attrib_file_extquote_handling[] = N_("#  extquote_handling=ON/OFF\n");
#endif /* HAVE_COLOR */
constext txt_attrib_file_wrap_on_unread[] = N_("#  wrap_on_next_unread=ON/OFF\n");
constext txt_attrib_file_x_body[] = N_("#  x_body=STRING (eg. ~/.tin/extra-body-text)\n");
constext txt_attrib_file_x_comment[] = N_("#  x_comment_to=ON/OFF\n");
constext txt_attrib_file_x_headers[] = N_("#  x_headers=STRING (eg. ~/.tin/extra-headers)\n");
constext txt_attrib_file_note_1[] = N_("#\n# Note that it is best to put general (global scoping)\n");
constext txt_attrib_file_note_2[] = N_("# entries first followed by group specific entries.\n#\n");
constext txt_attrib_file_footer[] = N_("############################################################################\n");
constext txt_attrib_menu_com[] = N_("Attributes Menu Commands");
constext txt_attrib_no_scope[] = N_("attribute with no scope: %s");
#ifdef NNTP_ABLE
	constext txt_auth_failed[] = N_("%d Authentication failed");
	constext txt_auth_failed_nopass[] = N_("NNTP authorization password not found for %s");
	constext txt_auth_needed[] = N_("Server expects authentication.\n");
	constext txt_auth_pass[] = N_("    Please enter password: ");
	constext txt_auth_user[] = N_("    Please enter username: ");
	constext txt_authorization_ok[] = N_("Authorized for user: %s\n");
	constext txt_authorization_fail[] = N_("Authorization failed for user: %s\n");
#endif /* NNTP_ABLE */
constext txt_author_search_backwards[] = N_("Author search backwards [%s]> ");
constext txt_author_search_forwards[] = N_("Author search forwards [%s]> ");
constext txt_autosubscribed[] = N_("\nAutosubscribed to %s");
constext txt_autosubscribing_groups[] = N_("Autosubscribing groups...\n");
constext txt_autoselecting_articles[] = N_("Autoselecting articles (use '%s' to see all unread) ...");

constext txt_bad_article[] = N_("Article to be posted resulted in errors/warnings. %s=quit, %s=Menu, %s=edit: ");
constext txt_bad_attrib[] = N_("Unrecognized attribute: %s");
constext txt_bad_command[] = N_("Bad command. Type '%s' for help.");
constext txt_base64[] = "base64";
constext txt_base_article[] = N_("Base article");
constext txt_base_article_range[] = N_("Base article range");
constext txt_batch_update_unavail[] = N_("%s: Updating of index files not supported: cache_overview_files=%s");
constext txt_begin_of_art[] = N_("*** Beginning of article ***");
constext txt_begin_of_page[] = N_("*** Beginning of page ***");
#if !defined(HAVE_LIBUU) && defined(HAVE_SUM) && !defined(DONT_HAVE_PIPING)
	constext txt_bytes[] = N_("bytes");
#endif /* !defined(HAVE_LIBUU) && defined(HAVE_SUM) && !defined(DONT_HAVE_PIPING) */

constext txt_cancel_article[] = N_("Cancel (delete) or supersede (overwrite) article [%%s]? (%s/%s/%s): ");
constext txt_cancelling_art[] = N_("Cancelling article...");
constext txt_cannot_create_uniq_name[] = "Can't create unique tempfile-name";
constext txt_cannot_create[] = N_("Cannot create %s");
constext txt_cannot_filter_on_path[] = "Can't filter on Path";
#ifdef DEBUG
	constext txt_cannot_find_base_art[] = N_("Can't find base article %d");
#endif /* DEBUG */
constext txt_cannot_open[] = N_("Can't open %s");
constext txt_cannot_open_for_saving[] = N_("Couldn't open %s for saving");
constext txt_cannot_post[] = N_("*** Posting not allowed ***");
constext txt_cannot_post_group[] = N_("Posting is not allowed to %s");
#ifdef NNTP_ABLE
	constext txt_cannot_retrieve[] = N_("Can't retrieve %s");
#endif /* NNTP_ABLE */
constext txt_cannot_supersede_mailgroups[] = N_("Can't supersede in mailgroups, try repost instead.");
constext txt_cannot_write_to_directory[] = N_("%s is a directory");
constext txt_catchup[] = N_("Catchup");
constext txt_catchup_group[] = N_("Catchup %s...");
constext txt_catchup_all_read_groups[] = N_("Catchup all groups entered during this session?");
constext txt_catchup_despite_tags[] = N_("You have tagged articles in this group - catchup anyway?");
constext txt_catchup_update_info[] = N_("%s %d %s in %lu seconds\n");
constext txt_caughtup[] = N_("Caughtup");
constext txt_check_article[] = N_("Check Prepared Article");
constext txt_checking_new_groups[] = N_("Checking for new groups... ");
constext txt_checking_for_news[] = N_("Checking for news...\n");
constext txt_choose_post_process_type[] = N_("Post-process %s=no, %s=yes, %s=shar, %s=quit: ");
#ifdef HAVE_COLOR
	constext txt_color_off[] = N_("ANSI color disabled");
	constext txt_color_on[] = N_("ANSI color enabled");
#endif /* HAVE_COLOR */
constext txt_command_failed[] = N_("Command failed: %s");
constext txt_copyright_notice[] = "%s (c) Copyright 1991-2024 Iain Lea.";
constext txt_confirm_select_on_exit[] = N_("Mark not selected articles read?");
constext txt_connection_info[] = N_("Connection Info");
constext txt_conninfo_local_spool[] = N_("Reading from local spool.\n");
constext txt_conninfo_saved_news[] = N_("Reading saved news.\n");
#ifndef NNTP_ONLY
	constext txt_conninfo_active_file[] = "ACTIVE_FILE       : %s\n";
	constext txt_conninfo_active_times_file[] = "ACTIVE_TIMES_FILE : %s\n";
	constext txt_conninfo_newsgroups_file[] = "NEWSGROUPS_FILE   : %s\n";
	constext txt_conninfo_novrootdir[] = "NOVROOTDIR        : %s\n";
	constext txt_conninfo_overview_file[] = "OVERVIEW_FILE     : %s\n";
	constext txt_conninfo_overview_fmt[] = "OVERVIEW_FMT      : %s\n";
	constext txt_conninfo_spool_config[] = N_("\nLocal spool config:\n-------------------\n");
	constext txt_conninfo_spooldir[] = "SPOOLDIR          : %s\n";
	constext txt_conninfo_subscriptions_file[] = "SUBSCRIPTIONS_FILE: %s\n";
#endif /* !NNTP_ONLY */
#ifdef NNTP_ABLE
	constext txt_conninfo_compress[] = N_("COMPRESS      :");
	constext txt_conninfo_conn_details[] = N_("\nConnection details:\n-------------------\n");
#	ifdef USE_ZLIB
	constext txt_conninfo_deflate[] = N_(" DEFLATE %s\n");
	constext txt_conninfo_enabled[] = N_("(enabled)");
	constext txt_conninfo_inactive[] = N_("(inactive)");
#	else
	constext txt_conninfo_deflate_unsupported[] = N_(" DEFLATE (not supported)\n");
#	endif /* USE_ZLIB */
#	if defined(HAVE_ALARM) && defined(SIGALRM)
	constext txt_conninfo_disabled[] = N_("(disabled)");
	constext txt_conninfo_timeout[] = N_("NNTP TIMEOUT  : %d seconds %s\n");
#	endif /* HAVE_ALARM && SIGALRM */
	constext txt_conninfo_implementation[] = N_("IMPLEMENTATION: %s\n");
#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
	constext txt_conninfo_maxartnum[] = N_("MAXARTNUM     : %s\n");
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
	constext txt_conninfo_nntp[] = N_("Reading via NNTP (%s).\n");
	constext txt_conninfo_port[] = N_("NNTPPORT      : %u\n");
	constext txt_conninfo_ro[] = N_("read only");
	constext txt_conninfo_rw[] = N_("read/write");
	constext txt_conninfo_server[] = N_("NNTPSERVER    : %s\n");
#	ifdef NNTPS_ABLE
	constext txt_conninfo_nntps[] = N_("Reading %s via NNTPS (%s; ");
#		if defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_LIBTLS) || defined(HAVE_LIB_OPENSSL)
		constext txt_conninfo_fmt_error[] = "<formatting error>";
		constext txt_conninfo_issuer[] = N_("Issuer : %s\n");
		constext txt_conninfo_server_cert_info[] = N_("\nServer certificate information:\n-------------------------------\n");
		constext txt_conninfo_subject[] = N_("Subject: %s\n");
		constext txt_conninfo_tls_info[] = N_("\nTLS information:\n----------------\n");
#		endif /* defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_LIBTLS) || defined(HAVE_LIB_OPENSSL) */
#		if defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_OPENSSL)
		constext txt_conninfo_cert[] = N_("Certificate #%d\n");
		constext txt_conninfo_error_unexpected[] = N_("UNEXPECTED, possible BUG");
		constext txt_conninfo_error_tolerated[] = N_("tolerated as \"-k\" (insecure) requested");
		constext txt_conninfo_verify_failed[] = N_("Server certificate verification FAILED:\n\t%s (%s)\n");
		constext txt_conninfo_verify_successful[] = N_("Server certificate verified successfully.\n");
#		endif /* defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_OPENSSL) */
	constext txt_conninfo_trusted[] = N_("trusted");
	constext txt_conninfo_untrusted[] = N_("untrusted");
#		ifdef HAVE_LIB_GNUTLS
		constext txt_conninfo_gnutls[] = "GnuTLS %s).\n";
		constext txt_conninfo_verify_failed_no_reason[] = N_("Server certificate verification FAILED: <can't get reason>\n");
#		endif /* HAVE_LIB_GNUTLS */
#		ifdef HAVE_LIB_LIBTLS
		constext txt_conninfo_libressl[] = "LibreSSL %d).\n";
		constext txt_conninfo_libtls_info[] = N_("%s %s (strength %d)\n");
#		endif /* HAVE_LIB_LIBTLS */
#		ifdef HAVE_LIB_OPENSSL
		constext txt_conninfo_openssl[] = "%s).\n";
#		endif /* HAVE_LIB_OPENSSL */
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */
constext txt_cook_article_failed_exiting[] = N_("Cook article failed, %s is exiting");
constext txt_cr[] = N_("<CR>");
constext txt_creating_active[] = N_("Creating active file for saved groups...\n");
constext txt_creating_newsrc[] = N_("Creating newsrc file...\n");

constext txt_default[] = N_("Default");
constext txt_delete_processed_files[] = N_("Delete saved files that have been post processed?");
constext txt_deleting[] = N_("Deleting temporary files...");

constext txt_end_of_art[] = N_("*** End of article ***");
constext txt_end_of_arts[] = N_("*** End of articles ***");
constext txt_end_of_attachments[] = N_("*** End of attachments ***");
constext txt_end_of_groups[] = N_("*** End of groups ***");
constext txt_end_of_page[] = N_("*** End of page ***");
constext txt_end_of_posted[] = N_("*** End of posted articles ***");
constext txt_end_of_scopes[] = N_("*** End of scopes ***");
constext txt_end_of_thread[] = N_("*** End of thread ***");
constext txt_end_of_urls[] = N_("*** End of URLs ***");
constext txt_enter_getart_limit[] = N_("Enter limit of articles to get> ");
constext txt_enter_message_id[] = N_("Enter Message-ID to go to> ");
constext txt_enter_next_thread[] = N_(" and enter next unread thread");
constext txt_enter_next_unread_art[] = N_(" and enter next unread article");
constext txt_enter_next_unread_group[] = N_(" and enter next unread group");
constext txt_enter_option_num[] = N_("Enter option number> ");
constext txt_enter_range[] = N_("Enter range [%s]> ");
constext txt_error_approved[] = N_("\nWarning: Approved: header used.\n");
#ifndef NDEBUG
	constext txt_error_asfail[] = "%s: assertion failure: %s (%d): %s\n";
#endif /* ! NDEBUG */
constext txt_error_bad_approved[] = N_("\nError: Bad address in Approved: header.\n");
constext txt_error_bad_from[] = N_("\nError: Bad address in From: header.\n");
constext txt_error_bad_msgidfqdn[] = N_("\nError: Bad FQDN in Message-ID: header.\n");
constext txt_error_bad_replyto[] = N_("\nError: Bad address in Reply-To: header.\n");
constext txt_error_bad_to[] = N_("\nError: Bad address in To: header.\n");
#ifndef NO_LOCKING
	constext txt_error_cant_unlock[] = N_("Can't unlock %s");
	constext txt_error_couldnt_dotlock[] = N_("Couldn't dotlock %s - article not appended!");
	constext txt_error_couldnt_lock[] = N_("Couldn't lock %s - article not appended!");
#endif /* !NO_LOCKING */
#if defined(NNTP_ABLE) && defined(USE_ZLIB)
	constext txt_error_compression_auth[] = N_("Server requires authentication but compression (-C) is already active.\nRestart %s with -A cmd.-line switch in conjunction with -C.\n");
#endif /* NNTP_ABLE && USE_ZLIB */
constext txt_error_copy_fp[] = "copy_fp() failed";
constext txt_error_corrupted_file[] = N_("Corrupted file %s");
constext txt_error_couldnt_expand[] = N_("couldn't expand %s\n");
constext txt_error_fseek[] = "fseek() error on [%s]";
constext txt_error_followup_poster[] = N_("\nError: Followup-To \"poster\" and a newsgroup is not allowed!\n");
constext txt_error_format_string[] = N_("Error: Custom format exceeds screen width. Using default \"%s\".");
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
constext txt_error_header_format[] = N_("\nError: Illegal formatted %s.\n");
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
constext txt_error_header_line_not_7bit[] = N_("\nError: %s contains non 7bit chars.\n");
constext txt_error_header_line_space[] = N_("\nError: Header on line %d does not have a space after the colon:\n%s\n");
constext txt_error_header_duplicate[] = N_("\nError: There are multiple (%d) \"%s:\" lines in the header.\n");
constext txt_error_header_no_name[] = N_("\nError: Header on line %d has no name:\n%s\n");
# ifndef FILE_MODE_BROKEN
	constext txt_error_insecure_permissions[] = N_("Insecure permissions of %s (%o)");
#endif /* !FILE_MODE_BROKEN */
#ifdef MIME_BREAK_LONG_LINES
	constext txt_error_should_be_folded[] = N_("Line %d is longer than %d octets and should be folded.\n");
#else
	constext txt_error_should_be_shortened[] = N_("Line %d is longer than %d octets and should be shortened.\n");
#endif /* MIME_BREAK_LONG_LINES */
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
	constext txt_error_locale[] = "Can't set the specified locale!";
#endif /* HAVE_SETLOCALE && !NO_LOCALE */
#ifdef DEBUG
	constext txt_error_mime_end[] = N_("MIME parse error: Unexpected end of %s/%s article");
	constext txt_error_mime_start[] = N_("MIME parse error: Start boundary whilst reading headers");
#endif /* DEBUG */
constext txt_error_newsgroups_poster[] = N_("\nError: \"poster\" is not allowed in Newsgroups!\n");
constext txt_error_no_domain_name[] = N_("Can't get a (fully-qualified) domain-name!");
constext txt_error_no_enter_permission[] = N_("No permissions to go into %s\n");
#ifdef NNTP_INEWS
	constext txt_error_no_from[] = N_("\nError: From: line missing.\n");
#endif /* NNTP_INEWS */
constext txt_error_no_read_permission[] = N_("No read permissions for %s\n");
constext txt_error_no_such_file[] = N_("File %s does not exist\n");
constext txt_error_no_write_permission[] = N_("No write permissions for %s\n");
constext txt_error_passwd_missing[] = N_("Can't get user information (/etc/passwd missing?)");
#ifdef HAVE_LIBUU
	constext txt_error_plural[] = N_("errors");
	constext txt_error_singular[] = N_("error");
#endif /* HAVE_LIBUU */
#ifndef FORGERY
	constext txt_error_sender_in_header_not_allowed[] = N_("\nError on line %d: \"Sender:\" header not allowed (it will be added for you)\n");
#endif /* !FORGERY */
constext txt_error_server_has_no_listed_groups[] = N_("Server has non of the groups listed in %s");
constext txt_error_unlink[] = N_("Error: unlink %s");
constext txt_error_unknown_dlevel[] = N_("Unknown display level");
#ifndef NNTP_ABLE
	constext txt_error_unreachable[] = N_("Unreachable?\n");
#endif /* !NNTP_ABLE */
constext txt_esc[] = N_("<ESC>");
constext txt_exiting[] = N_("Exiting...");
constext txt_external_mail_done[] = N_("leaving external mail-reader");
constext txt_extracting_shar[] = N_("Extracting %s...");

constext txt_filesystem_full[] = N_("Error writing %s file. Filesystem full? File reset to previous state.");
constext txt_filesystem_full_backup[] = N_("Error making backup of %s file. Filesystem full?");
constext txt_filter_global_rules[] = N_("Filtering global rules (%d/%d) ('q' to quit)...");
constext txt_filter_rule_created[] = N_("Rule created by: ");
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
#   msgid=STRING      Optional. Message-ID: line (e.g. <123@example.net>) with\n\
#                     full references.\n\
#   msgid_last=STRING Optional. Like above, but with last reference only.\n\
#   msgid_only=STRING Optional. Like above, but without references.\n\
#   refs_only=STRING  Optional. References: line (e.g. <123@example.net>) without\n\
#                     Message-ID:\n\
#   lines=[<>]?NUM    Optional. Lines: line. '<' or '>' are optional.\n\
#   gnksa=[<>]?NUM    Optional. GNKSA parse_from() return code. '<' or '>' opt.\n\
#   xref=PATTERN      Optional. Kill pattern (e.g. alt.flame*)\n\
#   path=PATTERN      Optional. Kill pattern (e.g. news.example.org)\n\
#                     Be aware that filtering on Path: may significantly slow\n\
#                     down the process.\n\
#   time=NUM          Optional. time_t value when rule expires\n#\n");
/* do NOT localize the next string! */
constext txt_filter_file_version[] = "# Filter file V%s for the TIN newsreader\n#\n";
constext txt_filter_score[] = N_("Enter score for rule (default=%d): ");
constext txt_filter_score_help[] = N_("Enter the score weight (range 0 < score <= %d)"); /* SCORE_MAX */
constext txt_full[] = N_("Full");
constext txt_filter_comment[] = N_("Comment (optional)  : ");
#ifdef DEBUG
	constext txt_filter_error_overview_no_servername[] = N_("Malformed overview entry: servername missing.");
	constext txt_filter_error_overview_xref[] = N_("\t Xref: %s");
	constext txt_filter_error_skipping_xref_filter[] = N_("Skipping Xref filter");
#endif /* DEBUG */
constext txt_filter_text_type[] = N_("Apply pattern to    : ");
constext txt_feed_pattern[] = N_("Enter pattern [%s]> ");
constext txt_followup_newsgroups[] = N_("\nYou requested followups to your article to go to the following %s:\n");
constext txt_followup_poster[] = N_("  %s\t Answers will be directed to you by mail.\n");
constext txt_forwarded[] = N_("-- forwarded message --\n");
constext txt_forwarded_end[] = N_("-- end of forwarded message --\n");
constext txt_from_line_only[] = N_("From: line (ignore case)      ");
constext txt_from_line_only_case[] = N_("From: line (case sensitive)   ");

#ifdef NNTP_ABLE
	constext txt_gethostbyname[] = N_("%s%s: Unknown host.\n");
#endif /* NNTP_ABLE */
constext txt_global[] = N_("global ");
constext txt_group_aliased[] = N_("Please use %.100s instead");
constext txt_group_bogus[] = N_("%s is bogus");
constext txt_group_is_moderated[] = N_("Group %s is moderated. Continue?");
constext txt_group_plural[] = N_("groups");
constext txt_group_rereading[] = N_("Rereading %s...");
constext txt_group_select_com[] = N_("Top Level Commands");
constext txt_group_selection[] = N_("Group Selection");
constext txt_group_singular[] = N_("group");
constext txt_grpdesc_disabled[] = N_("*** Group descriptions are disabled according to current select_format ***");

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
#ifndef NO_POSTING
	constext txt_help_article_cancel[] = N_("cancel (delete) or supersede (overwrite) current article");
	constext txt_help_article_followup[] = N_("post followup to current article");
	constext txt_help_article_followup_no_quote[] = N_("post followup (don't copy text) to current article");
	constext txt_help_article_followup_with_header[] = N_("post followup to current article quoting complete headers");
	constext txt_help_article_repost[] = N_("repost chosen article to another group");
#endif /* !NO_POSTING */
constext txt_help_article_edit[] = N_("edit article (mail-groups only)");
constext txt_help_article_first_in_thread[] = N_("display first article in current thread");
constext txt_help_article_first_page[] = N_("display first page of article");
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
constext txt_help_article_search_backwards[] = N_("search backwards within this article");
constext txt_help_article_search_forwards[] = N_("search forwards within this article");
constext txt_help_article_show_raw[] = N_("show article in raw-mode (including all headers)");
constext txt_help_article_skip_quote[] = N_("skip next block of included text");
constext txt_help_article_toggle_formfeed[] = N_("toggle display of sections hidden by a form-feed (^L) on/off");
constext txt_help_article_toggle_headers[] = N_("toggle display of all headers");
constext txt_help_article_toggle_highlight[] = N_("toggle word highlighting on/off");
constext txt_help_article_toggle_rot13[] = N_("toggle ROT-13 (basic decode) for current article");
constext txt_help_article_toggle_tabwidth[] = N_("toggle tabwidth 4 <-> 8");
constext txt_help_article_toggle_tex2iso[] = N_("toggle German TeX style decoding for current article");
constext txt_help_article_toggle_uue[] = N_("toggle display of uuencoded sections");
constext txt_help_article_view_attachments[] = N_("View/pipe/save multimedia attachments");
constext txt_help_attachment_first[] = N_("choose first attachment in list");
constext txt_help_attachment_goto[] = N_("0 - 9\t  choose attachment by number");
constext txt_help_attachment_last[] = N_("choose last attachment in list");
#ifndef DONT_HAVE_PIPING
constext txt_help_attachment_pipe[] = N_("pipe attachment into command");
constext txt_help_attachment_pipe_raw[] = N_("pipe raw attachment into command");
#endif /* !DONT_HAVE_PIPING */
constext txt_help_attachment_save[] = N_("save attachment to disk");
constext txt_help_attachment_search_forwards[] = N_("search for attachments forwards");
constext txt_help_attachment_search_backwards[] = N_("search for attachments backwards");
constext txt_help_attachment_select[] = N_("view attachment");
constext txt_help_attachment_tag[] = N_("tag attachment");
constext txt_help_attachment_tag_pattern[] = N_("tag attachments that match user specified pattern");
constext txt_help_attachment_toggle_tagged[] = N_("reverse tagging on all attachments (toggle)");
constext txt_help_attachment_untag[] = N_("untag all tagged attachments");
constext txt_help_attachment_toggle_info_line[] = N_("toggle info message in last line (name/description of attachment)");
constext txt_help_attrib_first_opt[] = N_("choose first attribute in list");
constext txt_help_attrib_goto_opt[] = N_("0 - 9\t  choose attribute by number");
constext txt_help_attrib_last_opt[] = N_("choose last attribute in list");
constext txt_help_attrib_reset_attrib[] = N_("reset attribute to a default value");
constext txt_help_attrib_search_opt_backwards[] = N_("search forwards for an attribute");
constext txt_help_attrib_search_opt_forwards[] = N_("search backwards for an attribute");
constext txt_help_attrib_select[] = N_("select attribute");
constext txt_help_attrib_toggle_attrib[] = N_("toggle back to options menu when invoked from there");
constext txt_help_bug[] = N_("report bug or comment via mail to %s");
constext txt_help_config_first_opt[] = N_("choose first option in list");
constext txt_help_config_goto_opt[] = N_("0 - 9\t  choose option by number");
constext txt_help_config_last_opt[] = N_("choose last option in list");
constext txt_help_config_scope_menu[] = N_("start scopes menu");
constext txt_help_config_search_opt_backwards[] = N_("search forwards for an option");
constext txt_help_config_search_opt_forwards[] = N_("search backwards for an option");
constext txt_help_config_select[] = N_("select option");
constext txt_help_config_toggle_attrib[] = N_("toggle to attributes menu");
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
#ifndef NO_POSTING
	constext txt_help_global_post[] = N_("post (write) article to current group");
	constext txt_help_global_post_postponed[] = N_("post postponed articles");
#endif /* !NO_POSTING */
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
constext txt_help_global_toggle_mini_help[] = N_("toggle mini help menu and posting etiquette display");
constext txt_help_global_toggle_subj_display[] = N_("cycle the display of authors email address, real name, both or neither");
constext txt_help_global_version[] = N_("show version information");
constext txt_help_group_catchup[] = N_("mark all articles as read and return to group selection menu");
constext txt_help_group_catchup_next[] = N_("mark all articles as read and enter next group with unread articles");
constext txt_help_group_first_thread[] = N_("choose first thread in list");
constext txt_help_group_last_thread[] = N_("choose last thread in list");
constext txt_help_group_list_thread[] = N_("list articles within current thread (bring up Thread sub-menu)");
constext txt_help_group_mark_article_unread[] = N_("mark article as unread");
constext txt_help_group_mark_thread_read[] = N_("mark current thread, range or tagged threads as read");
constext txt_help_group_mark_thread_unread[] = N_("mark current thread, range or tagged threads as unread");
constext txt_help_mark_feed_read[] = N_("mark current/range/selected/pattern/tagged as read after prompting");
constext txt_help_mark_feed_unread[] = N_("mark current/range/selected/pattern/tagged as unread after prompting");
constext txt_help_group_mark_unsel_art_read[] = N_("toggle display of all/selected articles");
constext txt_help_group_next[] = N_("display next group");
constext txt_help_group_prev[] = N_("display previous group");
constext txt_help_group_reverse_thread_selection[] = N_("toggle all selections (all articles)");
constext txt_help_group_select_all[] = N_("select group (make \"hot\")");
constext txt_help_group_select_thread[] = N_("select thread");
constext txt_help_group_select_thread_if_unread_selected[] = N_("select threads if at least one unread article is selected");
constext txt_help_group_select_thread_pattern[] = N_("select threads that match user specified pattern");
constext txt_help_group_thread_by_num[] = N_("0 - 9\t  choose thread by number");
constext txt_help_group_toggle_getart_limit[] = N_("toggle limit number of articles to get, and reload");
constext txt_help_group_toggle_read_articles[] = N_("toggle display of all/unread articles");
constext txt_help_group_toggle_thread_selection[] = N_("toggle selection of thread");
constext txt_help_group_toggle_threading[] = N_("cycle through threading options available");
constext txt_help_group_undo_thread_selection[] = N_("undo all selections (all articles)");
constext txt_help_group_untag_thread[] = N_("untag all tagged threads");
constext txt_help_post_hist_search_forwards[] = N_("search for articles forwards");
constext txt_help_post_hist_search_backwards[] = N_("search for articles backwards");
constext txt_help_post_hist_toggle_info_line[] = N_("toggle info message in last line (Message-ID)");
constext txt_help_post_hist_select[] = N_("Open article by Message-ID");
constext txt_help_scope_add[] = N_("add new scope");
constext txt_help_scope_del[] = N_("delete scope");
constext txt_help_scope_edit_attrib_file[] = N_("edit attributes file");
constext txt_help_scope_first_scope[] = N_("choose first scope in list");
constext txt_help_scope_goto_scope[] = N_("0 - 9\t  choose scope by number");
constext txt_help_scope_last_scope[] = N_("choose last scope in list");
constext txt_help_scope_move[] = N_("move scope");
constext txt_help_scope_rename[] = N_("rename scope");
constext txt_help_scope_select[] = N_("select scope");
constext txt_help_select_catchup[] = N_("mark all articles in group as read");
constext txt_help_select_catchup_next_unread[] = N_("mark all articles in group as read and move to next unread group");
constext txt_help_select_first_group[] = N_("choose first group in list");
constext txt_help_select_goto_group[] = N_("choose group by name");
constext txt_help_select_group_by_num[] = N_("0 - 9\t  choose group by number");
constext txt_help_select_group_range[] = N_("choose range of groups to be affected by next command");
constext txt_help_select_last_group[] = N_("choose last group in list");
#ifdef NNTP_ABLE
	constext txt_help_select_lookup_group[] = N_("list groups which an article has been posted to (by Message-ID)");
	constext txt_help_select_lookup_group_comment[] = N_(" \t  (go to article if at least one of the groups is available)");
#endif /* NNTP_ABLE */
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
constext txt_help_global_connection_info[] = N_("show NNTP[S] connection details");
constext txt_help_select_toggle_descriptions[] = N_("toggle display of group name only or group name plus description");
constext txt_help_select_toggle_read_groups[] = N_("toggle display to show all/unread subscribed groups");
constext txt_help_select_unsubscribe[] = N_("unsubscribe from chosen group");
constext txt_help_select_unsubscribe_pattern[] = N_("unsubscribe from groups that match pattern");
constext txt_help_select_sort_active[] = N_("sort the list of groups");
constext txt_help_select_yank_active[] = N_("toggle display to show all/subscribed groups");
constext txt_help_tag_parts[] = N_("tag/untag all parts of current multipart-message in order");
constext txt_help_thread_article_by_num[] = N_("0 - 9\t  choose article by number");
constext txt_help_thread_catchup[] = N_("mark thread as read and return to group index page");
constext txt_help_thread_catchup_next_unread[] = N_("mark thread as read and enter next unread thread or group");
constext txt_help_thread_first_article[] = N_("choose first article in list");
constext txt_help_thread_last_article[] = N_("choose last article in list");
constext txt_help_thread_mark_article_read[] = N_("mark art, range or tagged arts as read; move crsr to next unread art");
constext txt_help_thread_mark_article_unread[] = N_("mark article, range or tagged articles as unread");
constext txt_help_thread_mark_thread_unread[] = N_("mark current thread as unread");
constext txt_help_thread_read_article[] = N_("read chosen article");
constext txt_help_title_disp[] = N_("Display properties\n------------------");
constext txt_help_title_misc[] = N_("Miscellaneous\n-------------");
constext txt_help_title_navi[] = N_("Moving around\n-------------");
constext txt_help_title_ops[] = N_("Group/thread/article operations\n-------------------------------");
constext txt_help_title_attachment_ops[] = N_("Attachment operations\n---------------------");
constext txt_help_title_attrib_ops[] = N_("Attribute operations\n--------------------");
constext txt_help_title_config_ops[] = N_("Option operations\n-----------------");
constext txt_help_title_post_hist_ops[] = N_("Posted article operations\n-------------------------");
constext txt_help_title_scope_ops[] = N_("Scope operations\n----------------");
constext txt_help_title_url_ops[] = N_("URL operations\n--------------");
constext txt_help_url_first_url[] = N_("choose first URL in list");
constext txt_help_url_goto_url[] = N_("0 - 9\t  choose URL by number");
constext txt_help_url_last_url[] = N_("choose last URL in list");
constext txt_help_url_search_forwards[] = N_("search for URLs forwards");
constext txt_help_url_search_backwards[] = N_("search for URLs backwards");
constext txt_help_url_select[] = N_("Open URL in browser");
constext txt_help_url_toggle_info_line[] = N_("toggle info message in last line (URL)");

constext txt_index_page_com[] = N_("Group Level Commands");
constext txt_info_add_kill[] = N_("Kill filter added");
constext txt_info_add_select[] = N_("Auto-selection filter added");
constext txt_info_all_parts_tagged[] = N_("All parts tagged");
constext txt_info_all_parts_untagged[] = N_("All parts untagged");
constext txt_info_building_ref_tree[] = N_("Building References-trees (%d/%d)...");
constext txt_info_do_postpone[] = N_("Storing article for later posting");
constext txt_info_enter_valid_character[] = N_("Please enter a valid character");
constext txt_info_missing_part[] = N_("Missing part #%d");
constext txt_info_nopostponed[] = N_("*** No postponed articles ***");
constext txt_info_not_multipart_message[] = N_("Not a multi-part message");
constext txt_info_not_subscribed[] = N_("You are not subscribed to this group");
constext txt_info_no_previous_expression[] = N_("No previous expression");
constext txt_info_no_write[] = N_("Operation disabled in no-overwrite mode");
/* TODO: replace hard coded key-name in txt_info_postponed */
constext txt_info_postponed[] = N_("%d postponed %s, reuse with ^O...\n");
constext txt_info_x_conversion_note[] = N_("X-Conversion-Note: multipart/alternative contents have been removed.\n\
  To get the whole article, turn alternative handling OFF in the Option Menu\n");
constext txt_is_mailbox[] = N_("Save filename for %s/%s is a mailbox. Attachment not saved");
constext txt_is_tex_encoded[] = N_("TeX2Iso encoded article");
/* TODO: replace hard coded key-names */
constext txt_intro_page[] = N_("\nWelcome to %s, a full screen threaded Netnews reader. It can read news locally\n\
(i.e. <spool>/news) or remotely (-r option) from an NNTP (Network News Transport\n\
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
#if defined(NNTP_INEWS) && !defined(FORGERY)
	constext txt_invalid_sender[] = N_("Invalid  Sender:-header %s");
#endif /* NNTP_INEWS && !FORGERY */
constext txt_inverse_off[] = N_("Inverse video disabled");
constext txt_inverse_on[] = N_("Inverse video enabled");

constext txt_keymap_missing_key[] = N_("Missing definition for %s\n");
constext txt_keymap_invalid_key[] = N_("Invalid key definition '%s'\n");
constext txt_keymap_invalid_name[] = N_("Invalid keyname '%s'\n");
constext txt_keymap_upgraded[] = N_("Keymap file was upgraded to version %s\n");
constext txt_kill_from[] = N_("Kill From:     [%s] (y/n): ");
constext txt_kill_lines[] = N_("Kill Lines: (</>num): ");
constext txt_kill_menu[] = N_("Kill Article Menu");
constext txt_kill_msgid[] = N_("Kill Msg-ID:   [%s] (f/l/o/n): ");
constext txt_kill_scope[] = N_("Kill pattern scope  : ");
constext txt_kill_subj[] = N_("Kill Subject:  [%s] (y/n): ");
constext txt_kill_text[] = N_("Kill text pattern   : ");
constext txt_kill_time[] = N_("Kill time in days   : ");

constext txt_last[] = N_("Last");
constext txt_last_resp[] = N_("-- Last response --");
constext txt_lines[] = N_("Lines %s  ");

#if defined(NNTP_ABLE) && defined(DEBUG)
	constext txt_log_data_hidden[] = ". [full data hidden, rerun with -v]";
#endif /* NNTP_ABLE && DEBUG */

constext txt_lookup_func_not_available[] = N_("Function not available.");
constext txt_lookup_func_not_nntp[] = N_("Not reading via NNTP.");
#ifdef NNTP_ABLE
	constext txt_lookup_show_group[] = N_("Group: %s");
	constext txt_lookup_show_groups[] = N_("Groups: %s");
#endif /* NNTP_ABLE */

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
# The 4th field is the basedir (i.e. ~/Mail or ~/News)\n#\n");
constext txt_marked_as_read[] = N_("%s marked as read");
constext txt_marked_as_unread[] = N_("%s marked as unread");
constext txt_marked_arts_as_read[] = N_("Marked %d of %d %s as read");
constext txt_marked_arts_as_unread[] = N_("Marked %d of %d %s as unread");
constext txt_mark[] = N_("Mark");
constext txt_mark_arts_read[] = N_("Mark all articles as read%s?");
constext txt_mark_art_read[] = N_("Mark article as read%s?");
constext txt_mark_group_read[] = N_("Mark group %s as read?");
constext txt_mark_thread_read[] = N_("Mark thread as read%s?");
constext txt_matching_cmd_line_groups[] = N_("Matching %s groups...");
constext txt_mini_attachment_1[] = N_("<n>=set current to n; %s=line down; %s=line up; %s=help; %s=quit");
#ifndef DONT_HAVE_PIPING
constext txt_mini_attachment_2[] = N_("%s=view; %s=pipe; %s=pipe raw; %s=save; %s=tag; %s=tag pattern; %s=untag all");
#else
constext txt_mini_attachment_2[] = N_("%s=view; %s=save; %s=tag; %s=tag pattern; %s=untag all");
#endif /* !DONT_HAVE_PIPING */
constext txt_mini_attachment_3[] = N_("%s=reverse tagging; %s=search forwards; %s=search backwards; %s=repeat search");
constext txt_mini_group_1[] = N_("<n>=set current to n; %s=next unread; %s=search pattern; %s=kill/select");
constext txt_mini_group_2[] = N_("%s=author search; %s=catchup; %s=line down; %s=line up; %s=mark read; %s=list thread");
constext txt_mini_info_1[] = N_("%s=line up; %s=line down; %s=page up; %s=page down; %s=top; %s=bottom");
constext txt_mini_info_2[] = N_("%s=search forwards; %s=search backwards; %s=quit");
constext txt_mini_page_1[] = N_("<n>=set current to n; %s=next unread; %s=search pattern; %s=kill/select");
constext txt_mini_page_2[] = N_("%s=author search; %s=body search; %s=catchup; %s=followup; %s=mark read");
constext txt_mini_post_hist_1[] = N_("<n>=set current to n; %s=line down; %s=line up; %s=help; %s=quit");
constext txt_mini_post_hist_2[] = N_("%s=search forwards; %s=search backwards; %s=repeat search");
constext txt_mini_scope_1[] = N_("%s=add; %s=move; %s=rename; %s=delete");
constext txt_mini_scope_2[] = N_("<n>=set current to n; %s=line down; %s=line up; %s=help; %s=quit");
constext txt_mini_select_1[] = N_("<n>=set current to n; %s=next unread; %s,%s=search pattern; %s=catchup");
constext txt_mini_select_2[] = N_("%s=line down; %s=line up; %s=help; %s=move; %s=quit; %s=toggle all/unread");
constext txt_mini_select_3[] = N_("%s=subscribe; %s=sub pattern; %s=unsubscribe; %s=unsub pattern; %s=yank in/out");
constext txt_mini_thread_1[] = N_("<n>=set current to n; %s=next unread; %s=catchup; %s=display toggle");
constext txt_mini_thread_2[] = N_("%s=help; %s=line down; %s=line up; %s=quit; %s=tag; %s=mark unread");
constext txt_mini_url_1[] = N_("<n>=set current to n; %s=line down; %s=line up; %s=help; %s=quit");
constext txt_mini_url_2[] = N_("%s=search forwards; %s=search backwards; %s=repeat search");
constext txt_more[] = N_("--More--");
#ifdef NNTP_ABLE
	constext txt_motd[] = N_("MOTD: %s\n");
#endif /* NNTP_ABLE */
constext txt_moving[] = N_("Moving %s...");
constext txt_msgid_line_last[] = N_("Message-ID: & last Reference  ");
constext txt_msgid_line_only[] = N_("Message-ID: line              ");
constext txt_msgid_refs_line[] = N_("Message-ID: & References: line");

constext txt_newsgroup[] = N_("Go to newsgroup [%s]> ");
constext txt_newsgroup_plural[] = N_("newsgroups");
constext txt_newsgroup_position[] = N_("Position %s in group list (1,2,..,$) [%d]> ");
constext txt_newsgroup_singular[] = N_("newsgroup");
constext txt_newsrc_again[] = N_("Try and save newsrc file again?");
constext txt_newsrc_nogroups[] = N_("Warning: No newsgroups were written to your newsrc file. Save aborted.");
constext txt_newsrc_saved[] = N_("newsrc file saved successfully.\n");
constext txt_next_resp[] = N_("-- Next response --");
constext txt_no[] = N_("No  ");
constext txt_no_arts[] = N_("*** No articles ***");
constext txt_no_arts_posted[] = N_("No articles have been posted");
constext txt_no_attachments[] = N_("*** No attachments ***");
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
constext txt_no_prev_search[] = N_("No previous search, nothing to repeat");
constext txt_no_prev_unread_art[] = N_("No previous unread article");
constext txt_no_responses[] = N_("No responses");
constext txt_no_resps_in_thread[] = N_("No responses to list in current thread");
constext txt_no_scopes[] = N_("*** No scopes ***");
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
constext txt_none[] = N_("None");
constext txt_not_exist[] = N_("Newsgroup does not exist on this server");
constext txt_not_in_active_file[] = N_("Group %s not found in active file");
constext txt_nrctbl_create[] = N_("c)reate it, use a)lternative name, use d)efault .newsrc, q)uit tin: ");
constext txt_nrctbl_default[] = N_("use a)lternative name, use d)efault .newsrc, q)uit tin: ");
constext txt_nrctbl_info[] = N_("# NNTP-server -> newsrc translation table and NNTP-server\n\
# shortname list for %s %s\n#\n# the format of this file is\n\
#   <FQDN of NNTP-server> <newsrc file> <shortname> ...\n#\n\
# if <newsrc file> is given without path, $HOME is assumed as its location\n\
#\n# examples:\n#   news.tin.org      .newsrc-tin.org  tinorg\n\
#   news.example.org  /tmp/nrc-ex      example    ex\n#\n");
constext txt_null[] = N_("NULL");

constext txt_only[] = N_("Only");
constext txt_option_not_enabled[] = N_("Option not enabled. Recompile with %s.");
constext txt_options_menu[] = N_("Options Menu");
constext txt_options_menu_com[] = N_("Options Menu Commands");
constext txt_out_of_memory[] = "%s: memory exhausted trying to allocate %lu bytes in file %s line %d";

constext txt_pcre_error_at[] = N_("Error in regex: %s at pos. %d '%s'");
constext txt_pcre_error_num[] = N_("Error in regex: pcre internal error %d");
#ifndef HAVE_LIB_PCRE2
	constext txt_pcre_error_text[] = N_("Error in regex: study - pcre internal error %s");
#endif /* !HAVE_LIB_PCRE2 */
constext txt_post_a_followup[] = N_("Post a followup...");
/* TODO: replace hard coded key-name in txt_post_error_ask_postpone */
constext txt_post_error_ask_postpone[] = N_("An error has occurred while posting the article. If you think that this\n\
error is temporary or otherwise correctable, you can postpone the article\n\
and pick it up again with ^O later.\n");
constext txt_post_history_menu[] = N_("Posted articles history");
constext txt_post_history_menu_com[] = N_("Posted Articles Menu Commands");
constext txt_post_history_lookup_failed[] = N_("Lookup failed");
constext txt_post_history_op_unavail_for_reply[] = N_("Operation not available for replies by mail");
constext txt_post_history_recursion[] = N_("Already in posted articles history level");
constext txt_post_newsgroups[] = N_("Post to newsgroup(s) [%s]> ");
constext txt_post_processing[] = N_("-- post processing started --");
constext txt_post_processing_finished[] = N_("-- post processing completed --");
constext txt_post_subject[] = N_("Post subject [%s]> ");
constext txt_posted_info_file[] = N_("# Summary of mailed/posted messages viewable by 'W' command from within tin.\n");
constext txt_posting[] = N_("Posting article...");
#ifdef NNTP_INEWS
	constext txt_posting_failed[] = N_("Posting failed (%s)");
#endif /* NNTP_INEWS */
constext txt_postpone_post[] = N_("Posting: %.*s ...");
constext txt_postpone_repost[] = N_("Post postponed articles [%%s]? (%s/%s/%s/%s/%s): ");
constext txt_prefix_hot[] = N_("Hot %s");
constext txt_prefix_tagged[] = N_("Tagged %s");
constext txt_prefix_untagged[] = N_("Untagged %s");
#ifdef NNTP_ABLE
	constext txt_prep_for_filter_on_path[] = N_("Preparing for filtering on Path header (%d/%d)...");
#endif /* NNTP_ABLE */
constext txt_processing_attributes[] = N_("Processing attributes...");
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
constext txt_quit_edit_save_kill[] = N_("%s=quit, %s=edit, %s=save kill description: ");
constext txt_quit_edit_save_select[] = N_("%s=quit, %s=edit, %s=save select description: ");
constext txt_quit_no_write[] = N_("Do you really want to quit without saving your configuration?");
constext txt_quoted_printable[] = "quoted-printable";

constext txt_range_invalid[] = N_("Invalid range - valid are '0-9.$' e.g. 1-$");
#ifdef HAVE_SELECT
	constext txt_read_abort[] = N_("Do you want to abort this operation?");
	constext txt_read_exit[] = N_("Do you want to exit tin immediately?");
#endif /* HAVE_SELECT */
constext txt_reading_article[] = N_("Reading ('q' to quit)...");
constext txt_reading_arts[] = N_("Reading %s articles...");
constext txt_reading_attributes_file[] = N_("Reading %sattributes file...\n");
constext txt_reading_config_file[] = N_("Reading %sconfig file...\n");
constext txt_reading_filter_file[] = N_("Reading filter file...\n");
#ifdef DEBUG
	constext txt_reading_from_spool[] = N_("reading from local spool");
#endif /* DEBUG */
constext txt_reading_group[] = N_("Reading %s\n");
constext txt_reading_groups[] = N_("Reading %s groups...");
constext txt_reading_input_history_file[] = N_("Reading input history file...\n");
constext txt_reading_keymap_file[] = N_("Reading keymap file: %s\n");
constext txt_reading_news_active_file[] = N_("Reading groups from active file... ");
constext txt_reading_news_newsrc_file[] = N_("Reading groups from newsrc file... ");
constext txt_reading_newsgroups_file[] = N_("Reading newsgroups file... ");
constext txt_reading_newsrc[] = N_("Reading newsrc file...");
constext txt_refs_line_only[] = N_("References: line              ");
#if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
	constext txt_remaining[] = N_("(%d:%02d remaining)");
#endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */
constext txt_remove_bogus[] = N_("Bogus group %s removed.");
constext txt_removed_rule[] = N_("Removed from this rule: ");
constext txt_rename_error[] = N_("Error: rename %s to %s");
constext txt_reply_to_author[] = N_("Reply to author...");
constext txt_repost[] = N_("Repost");
constext txt_repost_an_article[] = N_("Reposting article...");
constext txt_repost_group[] = N_("Repost article(s) to group(s) [%s]> ");
constext txt_reset_newsrc[] = N_("Reset newsrc?");
constext txt_resp_redirect[] = N_("Responses have been directed to the following newsgroups");
constext txt_resp_to_poster[] = N_("Responses have been directed to poster. %s=mail, %s=post, %s=quit: ");
constext txt_return_key[] = N_("Press <RETURN> to continue...");

constext txt_art_score[] = N_("Score: %s");
constext txt_select_from[] = N_("Select From    [%s] (y/n): ");
constext txt_select_lines[] = N_("Select Lines: (</>num): ");
constext txt_select_menu[] = N_("Auto-select Article Menu");
constext txt_select_msgid[] = N_("Select Msg-ID  [%s] (f/l/o/n): ");
constext txt_select_scope[] = N_("Select pattern scope: ");
constext txt_select_subj[] = N_("Select Subject [%s] (y/n): ");
constext txt_select_text[] = N_("Select text pattern : ");
constext txt_select_time[] = N_("Select time in days   : ");
constext txt_selection_flag_insecure[] = N_("[k]");
constext txt_selection_flag_secure[] = N_("[T]");
constext txt_selection_flag_only_unread[] = N_(" R");
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
constext txt_screen_too_small_exiting[] = N_("screen is too small, %s is exiting");
constext txt_scope_delete[] = N_("Delete scope?");
constext txt_scope_enter[] = N_("Enter scope> ");
constext txt_scope_new_position[] = N_("Select new position> ");
constext txt_scope_new_position_is_global[] = N_("New position cannot be a global scope");
constext txt_scope_operation_not_allowed[] = N_("Global scope, operation not allowed");
constext txt_scope_rename[] = N_("Rename scope> ");
constext txt_scope_select[] = N_("Select scope> ");
constext txt_scopes_menu[] = N_("Scopes Menu");
constext txt_scopes_menu_com[] = N_("Scopes Menu Commands");
constext txt_search_backwards[] = N_("Search backwards [%s]> ");
constext txt_search_body[] = N_("Search body [%s]> ");
constext txt_search_forwards[] = N_("Search forwards [%s]> ");
constext txt_searching[] = N_("Searching...");
constext txt_searching_body[] = N_("Searching article %d of %d ('q' to abort)...");
constext txt_select_art[] = N_("Select article> ");
constext txt_select_config_file_option[] = N_("Select option number before text or use arrow keys and <CR>. 'q' to quit.");
constext txt_select_group[] = N_("Select group> ");
constext txt_select_pattern[] = N_("Enter selection pattern [%s]> ");
constext txt_select_thread[] = N_("Select thread> ");
constext txt_send_bugreport[] = N_("%s %s %s (\"%s\"): send a DETAILED bug report to %s\n");
constext txt_servers_active[] = N_("servers active-file");
constext txt_skipped_group[] = N_("Skipped %s");
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
constext txt_tin_version[] = N_("Version: %s %s release %s (\"%s\")");
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
constext txt_thread_singular[] = N_("thread");
constext txt_thread_x_of_n[] = N_("Thread %4s of %4s");
constext txt_threading_arts[] = N_("Threading articles...");
constext txt_threading_by_multipart[] = N_("Threading by multipart");
#if defined(NNTP_ABLE) && defined(NNTPS_ABLE)
	constext txt_tls_handshake_failed[] = N_("TLS handshake failed: %s\n");
#	ifdef HAVE_LIB_LIBTLS
	constext txt_retr_cipher_failed[] = N_("<failed to retrieve cipher>");
	constext txt_retr_issuer_failed[] = N_("<failed to retrieve issuer>");
	constext txt_retr_subject_failed[] = N_("<failed to retrieve subject>");
	constext txt_retr_version_failed[] = N_("<failed to retrieve version>");
	constext txt_tls_handshake_done[] = N_("%s handshake done: %s\n");
	constext txt_tls_unknown_error[] = N_("unknown error");
#	endif /* HAVE_LIB_LIBTLS */
#	ifdef HAVE_LIB_GNUTLS
	constext txt_tls_handshake_failed_with_err_num[] = N_("TLS handshake failed: %s (%d)\n");
	constext txt_tls_peer_verify_failed[] = N_("TLS peer verification failed: %s\n");
	constext txt_tls_peer_verify_failed_continuing[] = N_("TLS peer verification failed, continuing anyway as requested: %s\n");
	constext txt_tls_unable_to_get_status[] = N_("<unable to retrieve status>");
	constext txt_tls_unexpected_status[] = N_("unexpected certificate verification status!");
#	endif /* HAVE_LIB_GNUTLS */
#	ifdef HAVE_LIB_OPENSSL
	constext txt_tls_peer_verify_failed_continuing[] = N_("TLS peer verification failed: %s.\nContinuing anyway as requested.\n");
#	endif /* HAVE_LIB_OPENSSL */
#	if defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_OPENSSL)
	constext txt_tls_handshake_done[] = N_("TLS handshake done: %s\n");
#	endif /* defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_OPENSSL) */
#endif /* NNTP_ABLE && NNTPS_ABLE */
constext txt_toggled_high[] = N_("Toggled word highlighting %s");
constext txt_toggled_rot13[] = N_("Toggled rot13 encoding");
constext txt_toggled_tex2iso[] = N_("Toggled German TeX encoding %s");
constext txt_toggled_tabwidth[] = N_("Toggled tab-width to %d");
#ifndef NO_LOCKING
	constext txt_trying_dotlock[] = N_("%d Trying to dotlock %s");
	constext txt_trying_lock[] = N_("%d Trying to lock %s");
#endif /* !NO_LOCKING */
constext txt_type_h_for_help[] = N_("%s=help");

constext txt_unlimited_time[] = N_("Unlimited");
constext txt_unsubscribe_pattern[] = N_("Enter wildcard unsubscribe pattern> ");
constext txt_uu_error_decode[] = N_("Error decoding %s : %s");
constext txt_uu_error_no_end[] = N_("No end.");
constext txt_uu_success[] = N_("%s successfully decoded.");
constext txt_unchanged[] = N_("unchanged");
constext txt_unknown[] = N_("(unknown)");
constext txt_unread[] = N_("unread");
constext txt_unsubscribed_num_groups[] = N_("unsubscribed from %d groups");
constext txt_unsubscribed_to[] = N_("Unsubscribed from %s");
constext txt_unsubscribing[] = N_("Unsubscribing... ");
constext txt_unthreading_arts[] = N_("Unthreading articles...");
constext txt_updated[] = N_("Updated");
constext txt_updating[] = N_("Updating");
constext txt_url_menu[] = N_("URL Menu");
constext txt_url_menu_com[] = N_("URL Menu Commands");
constext txt_url_open[] = N_("Opening %s");
constext txt_url_select[] = N_("Select URL> ");
constext txt_url_done[] = N_("No URLs in this article");
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
constext txt_usage_no_posting[] = N_("  -x       no-posting mode");
constext txt_usage_post_article[] = N_("  -w       post an article and exit");
constext txt_usage_post_postponed_arts[] = N_("  -o       post all postponed articles and exit");
constext txt_usage_read_saved_news[] = N_("  -R       read news saved by -S option");
constext txt_usage_savedir[] = N_("  -s dir   save news directory [default=%s]");
constext txt_usage_save_new_news[] = N_("  -S       save new news for later reading (batch mode)");
constext txt_usage_start_if_unread_news[] = N_("  -z       start if any unread news");
constext txt_usage_tin[] = N_("A Usenet reader.\n\nUsage: %s [options] [newsgroup[,...]]");
constext txt_usage_verbose[] = N_("  -v       verbose output for batch mode options");
constext txt_usage_version[] = N_("  -V       print version & date information");
constext txt_useful_without_batch_mode[] = N_("%s only useful without batch mode operations\n");
constext txt_useful_with_batch_mode[] = N_("%s only useful for batch mode operations\n");
constext txt_useful_with_batch_or_debug_mode[] = N_("%s only useful for batch or debug mode operations\n");
constext txt_useless_combination[] = N_("Useless combination %s and %s. Ignoring %s.\n");
constext txt_uue_complete[] = N_("uuencoded file");
constext txt_uue_incomplete[] = N_("incomplete uuencoded file");

#if defined(NNTP_ABLE) && defined(NNTPS_ABLE)
	constext txt_valid_not_after[] = N_("Valid not after : %s\n");
	constext txt_valid_not_before[] = N_("Valid not before: %s\n");
#endif /* NNTP_ABLE && NNTPS_ABLE */
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
#ifdef MIME_BREAK_LONG_LINES
	constext txt_warn_long_line_not_qp[] = N_("\n\
Line %d is longer than %d octets and should be folded, but encoding\n\
is neither set to %s nor to %s.\n");
#endif /* MIME_BREAK_LONG_LINES */
constext txt_warn_long_line_not_break[] = N_("\n\
Line %d is longer than %d octets and should be folded, but encoding\n\
is set to %s without enabling MIME_BREAK_LONG_LINES or\n\
posting doesn't contain any 8bit chars and thus folding won't happen.\n");
constext txt_warn_long_line_not_base[] = N_("\n\
Line %d is longer than %d octets and should be folded, but encoding\n\
is not set to %s.\n");
constext txt_warn_example_hierarchy[] = N_("\nWarning: \"example\" is a reserved hierarchy!\n");
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
constext txt_warn_unprintable_char[] = N_("Warning: line %d contains unprintable chars:\n%s\n");
constext txt_warn_wrong_sig_format[] = N_("\nWarning: Signatures should start with '-- \\n' not with '--\\n'.\n");
constext txt_writing_attributes_file[] = N_("Writing attributes file...");
constext txt_writing_group[] = N_("Writing %s\n");
constext txt_writing_overview[] = N_("Writing overview cache...");

constext txt_x_resp[] = N_("%4d Responses");
#ifdef XFACE_ABLE
	constext txt_xface_error_construct_fifo_name[] = N_("Can't run slrnface: couldn't construct fifo name.");
	constext txt_xface_error_create_failed[] = N_("Can't run slrnface: failed to create %s");
	constext txt_xface_error_exited_abnormal[] = N_("Slrnface abnormally exited, code %d.");
	constext txt_xface_error_finally_failed[] = N_("Slrnface failed: %s.");
	constext txt_xface_error_missing_env_var[] = N_("Can't run slrnface: Environment variable %s not found.");
	constext txt_xface_error_no_xterm[] = N_("Can't run slrnface: Not running in an xterm.");
	constext txt_xface_msg_cannot_connect_display[] = N_("couldn't connect to display");
	constext txt_xface_msg_cannot_open_fifo[] = N_("can't open FIFO");
	constext txt_xface_msg_executable_not_found[] = N_("executable not found");
	constext txt_xface_msg_fork_failed[] = N_("fork() failed");
	constext txt_xface_msg_no_controlling_terminal[] = N_("couldn't find controlling terminal");
	constext txt_xface_msg_no_width_and_height_avail[] = N_("terminal doesn't export width and height");
	constext txt_xface_msg_unknown_error[] = N_("unknown error");
	constext txt_xface_msg_windowid_not_found[] = N_("WINDOWID not found in environment");
	constext txt_xface_readme[] = N_("This directory is used to create named pipes for communication between\n\
slrnface and its parent process. It should normally be empty because\n\
the pipe is deleted right after it has been opened by both processes.\n\n\
File names generated by slrnface have the form \"hostname.pid\". It is\n\
probably an error if they linger here longer than a fraction of a second.\n\n\
However, if the directory is mounted from an NFS server, you might see\n\
special files created by your NFS server while slrnface is running.\n\
Do not try to remove them.\n");
#endif /* XFACE_ABLE */
#if defined(NNTP_ABLE) && defined(XHDR_XREF)
	constext txt_xref_loop[] = "%s XREF loop";
#endif /* NNTP_ABLE && XHDR_XREF */

constext txt_yanked_groups[] = N_("Added %d %s");
constext txt_yanked_none[] = N_("No unsubscribed groups to show");
constext txt_yanked_sub_groups[] = N_("Showing subscribed to groups only");
constext txt_yes[] = N_("Yes ");
constext txt_you_have_mail[] = N_("You have mail");


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
	constext txt_usage_debug[] = N_("  -D mode  debug mode");
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
	constext txt_error_grp_renamed[] = N_("\nError: \"%s\" is renamed, use \"%s\" instead!\n");
	constext txt_error_missing_followup_to[] = N_("\nError: cross-posting to %d newsgroups and no Followup-To line!\n");
	constext txt_error_not_valid_newsgroup[] = N_("\nError: \"%s\" is not a valid newsgroup!\n");
#else
	constext txt_warn_followup_to_several_groups[] = N_("\nWarning: Followup-To set to more than one newsgroup!\n");
	constext txt_warn_grp_renamed[] = N_("\nWarning: \"%s\" is renamed, you should use \"%s\" instead!\n");
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
#	if defined(HAVE_SUM) && !defined(DONT_HAVE_PIPING)
		constext txt_checksum_of_file[] = N_("\tChecksum of %s (%ld %s)");
#	endif /* HAVE_SUM && !DONT_HAVE_PIPING */
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


#ifdef NNTP_ABLE
	constext txt_caching_off[] = N_("Try cache_overview_files to speed up things.\n");
	constext txt_caching_on[] = N_("Tin will use local index files instead.\n");
	constext txt_cannot_get_nntp_server_name[] = N_("Cannot find NNTP server name");
	constext txt_capabilities_without_reader[] = N_("CAPABILITIES did not announce READER");
	constext txt_connecting_port[] = N_("Connecting to %s:%u...");
	constext txt_connection_error[] = N_("NNTP connection error. Exiting...");
	constext txt_disconnecting[] = N_("Disconnecting from server...");
	constext txt_failed_to_connect_to_server[] = N_("Failed to connect to NNTP server %s. Exiting...");
	constext txt_nntp_ok_goodbye[] = N_("205  Closing connection");
	constext txt_no_xover_support[] = N_("Your server does not support the NNTP XOVER or OVER command.\n");
#	ifdef DEBUG
		constext txt_port_not_numeric[] = N_("Port isn't numeric: %s:%s\n");
		constext txt_port_not_numeric_in[] = N_("Port in %s isn't numeric: %s:%s\n");
		constext txt_reconnect_limit_reached[] = N_("reconnect (%d) limit %d reached, giving up.");
#	endif /* DEBUG */
	constext txt_reconnect_to_news_server[] = N_("Connection to news server has timed out. Reconnect?");
	constext txt_server_name_in_file_env_var[] = N_("Put the server name in the file %s,\nor set the environment variable NNTPSERVER");
#ifdef USE_ZLIB
		constext txt_continuing[] = N_("Continuing...");
		constext txt_read_timeout_quit[] = N_("Read timeout from server (%d seconds) - quit tin?");
		constext txt_usage_compress[] = N_("  -C       try COMPRESS NNTP extension");
#endif /* USE_ZLIB */
	constext txt_usage_force_authentication[] = N_("  -A       force authentication on connect");
	constext txt_usage_newsserver[] = N_("  -g serv  read news from NNTP server serv [default=%s]");
	constext txt_usage_port[] = N_("  -p port  use port as NNTP port [default=%d]");
#if defined(NNTP_ABLE) && defined(NNTPS_ABLE)
	constext txt_usage_use_insecure_nntps[] = N_("  -k       skip verification for NNTPS");
	constext txt_usage_use_nntps[] = N_("  -T       enable NNTPS");
#endif /* NNTP_ABLE && NNTPS_ABLE */
	constext txt_usage_quickstart[] = N_("  -Q       quick start. Same as -dnq");
	constext txt_usage_read_news_remotely[] = N_("  -r       read news remotely from default NNTP server");
	constext txt_usage_read_only_active[] = N_("  -l       use only LIST instead of GROUP (-n) command");
	constext txt_usage_read_only_subscribed[] = N_("  -n       only read subscribed .newsrc groups from NNTP server");
#	ifdef INET6
		constext txt_usage_force_ipv4[] = N_("  -4       force connecting via IPv4");
		constext txt_usage_force_ipv6[] = N_("  -6       force connecting via IPv6");
		constext txt_error_socket_or_connect_problem[] = N_("\nsocket or connect problem\n");
#	else
		constext txt_connection_to[] = N_("\nConnection to %s: ");
		constext txt_giving_up[] = N_("Giving up...\n");
#		ifdef HAVE_GETSERVBYNAME
			constext txt_error_unknown_service[] = N_("%s/tcp: Unknown service.\n");
#		endif /* HAVE_GETSERVBYNAME */
#	endif /* INET6 */
#	ifdef XHDR_XREF
		constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its XOVER information.\n\
Tin will try to use XHDR XREF instead (slows down things a bit).\n");
#	else
		constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its XOVER information.\n");
#	endif /* XHDR_XREF */
#	ifndef NNTP_ONLY
		constext txt_cannot_open_active_file[] = N_("Can't open %s. Try %s -r to read news via NNTP.");
#	endif /* !NNTP_ONLY */
#else
	constext txt_usage_quickstart[] = N_("  -Q       quick start. Same as -dq");
	constext txt_usage_read_only_active[] = N_("  -l       read only active file instead of scanning spool (-n) command");
	constext txt_usage_read_only_subscribed[] = N_("  -n       only read subscribed .newsrc groups from spool");
	constext txt_warn_xref_not_supported[] = N_("Your server does not have Xref: in its NOV-files.\n");
#endif /* NNTP_ABLE */

#ifdef NNTP_INEWS
	constext txt_post_via_builtin_inews[] = N_("Posting using external inews failed. Use built in inews instead?");
	constext txt_post_via_builtin_inews_only[] = N_("It worked! Should I always use my built in inews from now on?");
#endif /* NNTP_INEWS */

#ifndef DISABLE_PRINTING
	constext txt_articles_printed[] = N_("%d %s printed");
	constext txt_help_global_print[] = N_("output article/thread/hot/pattern/tagged articles to printer");
	constext txt_print[] = N_("Print");
	constext txt_printing[] = N_("Printing...");
#endif /* !DISABLE_PRINTING */

#ifndef DONT_HAVE_PIPING
	constext txt_articles_piped[] = N_("%d %s piped to \"%s\"");
	constext txt_help_global_pipe[] = N_("pipe article/thread/hot/pattern/tagged articles into command");
	constext txt_no_command[] = N_("No command");
	constext txt_pipe[] = N_("Pipe");
	constext txt_pipe_to_command[] = N_("Pipe to command [%.*s]> ");
	constext txt_piping[] = N_("Piping...");
#else
	constext txt_piping_not_enabled[] = N_("Piping not enabled.");
#endif /* !DONT_HAVE_PIPING */

#ifndef ALLOW_FWS_IN_NEWSGROUPLIST
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
#endif /* !ALLOW_FWS_IN_NEWSGROUPLIST */

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

#if !defined(USE_CURSES) && !defined(USE_TERMINFO)
	constext txt_cannot_get_term_entry[] = N_("%s: Can't get entry for TERM\n");
#endif /* !USE_CURSES && !USE_TERMINFO */

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

#if defined(NNTP_ABLE) && defined(HAVE_INET_NTOA) && !defined(INET6)
	constext txt_trying[] = N_("Trying %s");
#endif /* NNTP_ABLE && HAVE_INET_NTOA && !INET6 */


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
	N_("Percentage Match"),
	NULL
};

/*
 * Whether to use wildmat() or regexec() for matching strings
 */
constext *txt_wildcard_type[] = { "WILDMAT", "REGEX", NULL };

/*
 * Handling of uuencoded data in pager
 */
constext *txt_hide_uue_type[] = {
	N_("No"),
	N_("Yes"),
	N_("Hide all"),
	NULL
};

/*
 * How the From: line is displayed.
 */
constext *txt_show_from[] = {
	N_("None"),
	N_("Address"),
	N_("Full Name"),
	N_("Address and Name"),
	NULL
};

/*
 * How the score of a thread is computed
 */
constext *txt_thread_score_type[] = {
	N_("Max"),
	N_("Sum"),
	N_("Average"),
	NULL
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
		N_("Light White"),
		NULL
	};
	constext *txt_backcolors[] = {
		N_("Default"),
		N_("Black"),
		N_("Red"),
		N_("Green"),
		N_("Brown"),
		N_("Blue"),
		N_("Pink"),
		N_("Cyan"),
		N_("White"),
		NULL
	};
#endif /* HAVE_COLOR */

/*
 * Which mark types can be used.
 */
constext *txt_marks[] = {
	N_("Nothing"),
	N_("Mark"),
	N_("Space"),
	NULL
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
	N_("Bold"),
	NULL
};

/* different options for auto_cc_bcc */
constext *txt_auto_cc_bcc_options[] = {
	N_("No"),
	N_("Cc"),
	N_("Bcc"),
	N_("Cc and Bcc"),
	NULL
};

#ifdef USE_CANLOCK
/* hash algorithms for Cancel-Lock/Cancel-Key */
constext *txt_cancel_lock_algos[] = {
	"none",
	"sha1",
	"sha256",
	"sha512",
	NULL
};
#endif /* USE_CANLOCK */

/* different confirm choices */
constext *txt_confirm_choices[] = {
	N_("none"),
	N_("commands"),
	N_("select"),
	N_("quit"),
	N_("commands & quit"),
	N_("commands & select"),
	N_("quit & select"),
	N_("commands & quit & select"),
	NULL
};

/* different options for goto_next_unread */
constext *txt_goto_next_unread_options[] = {
	N_("none"),
	N_("PageDown"),
	N_("PageNextUnread"),
	N_("PageDown or PageNextUnread"),
	NULL
};

/* different options for quick_kill_header / quick_select_header */
constext *txt_quick_ks_header_options[] = {
	N_("Subject: (case sensitive)"),
	N_("Subject: (ignore case)"),
	N_("From: (case sensitive)"),
	N_("From: (ignore case)"),
	N_("Msg-ID: & full References: line"),
	N_("Msg-ID: & last References: only"),
	N_("Message-ID: entry only"),
	N_("Lines:"),
	NULL
};

/* different options for trim_article_body */
constext *txt_trim_article_body_options[] = {
	N_("Don't trim article body"),
	N_("Skip leading blank lines"),
	N_("Skip trailing blank lines"),
	N_("Skip leading and trailing blank l."),
	N_("Compact multiple between text"),
	N_("Compact multiple and skip leading"),
	N_("Compact multiple and skip trailing"),
	N_("Compact mltpl., skip lead. & trai."),
	NULL
};

/* different options for show_help_mail_sign */
constext *txt_show_help_mail_sign_options[] = {
	N_("Don't show help or mail sign"),
	N_("Show only help sign"),
	N_("Show only mail sign if new mail"),
	N_("Show mail if new mail else help s."),
	NULL
};

/*
 * MIME-Content-Transfer-Encodings.
 */
/* TODO: can any of this go away? */
constext *txt_mime_encodings[] = {
	txt_8bit, txt_base64, txt_quoted_printable, txt_7bit,
	NULL
};

constext *content_encodings[] = {
	"7bit", "quoted-printable", "base64", "8bit", "binary", "x-uuencode", "unknown",
	NULL
};

constext *content_types[] = {
	"text", "multipart", "application", "message", "image", "audio", "video",
	NULL
};

/*
 * Array of possible post processing descriptions and short-keys
 * This must match the ordering of the defines in tin.h
 */
constext *txt_post_process_types[] = {
		N_("No"),
		N_("Shell archive"),
		N_("Yes"),
		NULL
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
		N_("Lines: (ascending)"),
		NULL
};

constext *txt_sort_t_type[] = {
		N_("Nothing"),
		N_("Score (descending)"),
		N_("Score (ascending)"),
		N_("Last posting date (descending)"),
		N_("Last posting date (ascending)"),
		NULL
};

#ifdef USE_HEAPSORT
constext *txt_sort_functions[] = {
		N_("Quick-sort"),
		N_("Heap-sort"),
		NULL
};
#endif /* USE_HEAPSORT */

/* Ways of handling bogus groups */
constext *txt_strip_bogus_type[] = {
		N_("Always Keep"),
		N_("Always Remove"),
		N_("Mark with D on selection screen"),
		NULL
};

/* Ways of handling killed articles */
constext *txt_kill_level_type[] = {
		N_("Kill only unread arts"),
		N_("Kill all arts & show with K"), /* TODO: s/K/art_marked_killed/ */
		N_("Kill all arts and never show"),
		NULL
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
		N_("Comp. q., quote sigs & empty lines"),
		NULL
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
	"UTF-8",
	NULL
};
#endif /* CHARSET_CONVERSION */

/* 7bit charsets, US-ASCII must be the first entry */
constext *txt_mime_7bit_charsets[] = {
	"US-ASCII",
	"ISO-2022-CN", "ISO-2022-CN-EXT", "ISO-2022-JP", "ISO-2022-JP-1",
	"ISO-2022-JP-2", "ISO-2022-KR",
	"HZ-GB-2312",
	NULL
};

/* different mailbox formats */
constext *txt_mailbox_formats[] = {
	"MBOXO",
	"MBOXRD",
	"MMDF",
	NULL
};

/* interactive mailers */
constext *txt_interactive_mailers[] = {
	N_("No"),
	N_("With headers"),
	N_("Without headers"),
	NULL
};

#ifdef HAVE_UNICODE_NORMALIZATION
constext *txt_normalization_forms[] = {
	N_("None"),
	N_("NFKC"),
#	if (HAVE_UNICODE_NORMALIZATION >= 2)
	N_("NFKD"),
	N_("NFC"),
	N_("NFD"),
#		ifdef HAVE_UNICODE_UNORM2_H
	N_("NFKC case fold"),
#		endif /* HAVE_UNICODE_UNORM2_H */
#	endif /* HAVE_UNICODE_NORMALIZATION >= 2 */
	NULL
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
#else
struct opttxt txt_highlight_options = {
	NULL,
	N_("Highlight Options"),
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
	N_("Show From (author) fields in group & thread level. <SPACE> toggles & <CR> sets."),
	N_("In group and thread level, show author by"),
	N_("# Part of From field to display in group and thread level\n\
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
	/* xgettext:no-c-format */
	N_("# Thread percentage match...\n\
# the percentage of characters in the subject of an article that must match\n\
# a base article for both those articles to be considered to belong to the\n\
# same thread. This option is an integer percentage, e.g. 80, no decimals may\n\
# follow. If 80 is used here, then 80% of the characters must match exactly,\n\
# no insertion of a character, for the two articles to be put in the same\n\
# thread. e.g. 'happy' and 'harpy' would match, but 'harpie', 'happie' and\n\
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
#   2 = Score ascending\n\
#   3 = Last posting date descending\n\
#   4 = Last posting date ascending\n")
};

struct opttxt txt_pos_first_unread = {
	N_("Put cursor at first/last unread art in groups. <SPACE> toggles & <CR> sets."),
	N_("Go to first unread article in group"),
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

struct opttxt txt_goto_next_unread = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Go to the next unread article with"),
	N_("# Go to the unread article with following key(s)\n\
# Possible values are (the default is marked with *):\n\
#   0 = nothing\n\
#   1 = PAGE DOWN\n\
# * 2 = TAB\n\
#   3 = PAGE DOWN or TAB\n")
};

struct opttxt txt_trim_article_body = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("How to treat blank lines"),
	N_("# Trim the article body, remove unnecessary blank lines.\n\
# Possible values are (the default is marked with *):\n\
# * 0 = Nothing special\n\
#   1 = Skip leading blank lines\n\
#   2 = Skip trailing blank lines\n\
#   3 = Skip leading and trailing blank lines\n\
#   4 = Compact multiple blank lines between text blocks\n\
#   5 = Compact multiple blank lines between text blocks and skip\n\
#       leading blank lines\n\
#   6 = Compact multiple blank lines between text blocks and skip\n\
#       trailing blank lines\n\
#   7 = Compact multiple blank lines between text blocks and skip\n\
#       leading and trailing blank lines\n")
};

struct opttxt txt_show_help_mail_sign = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Show help/mail sign in level titles"),
	N_("# Show help sign, new mail sign, both or nothing in level titles.\n\
# Possible values are (the default is marked with *):\n\
#   0 = Don't show help or mail sign\n\
#   1 = Show only help sign\n\
#   2 = Show only mail sign if new mail have arrived\n\
# * 3 = Show mail sign if new mail has arrived else show help sign\n")
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

struct opttxt txt_abbreviate_groupname = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Abbreviate long newsgroup names"),
	N_("# If ON abbreviate (if necessary) long newsgroup names at group selection\n\
# level and article level like this:\n\
#   news.software.readers -> n.software.readers -> n.s.readers -> n.s.r.\n")
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

struct opttxt txt_show_art_score = {
	N_("Display article score. <SPACE> toggles & <CR> sets."),
	N_("Display article score"),
	N_("# If ON show article score when displaying articles\n")
};

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
struct opttxt txt_suppress_soft_hyphens = {
	N_("Remove soft hyphens. <SPACE> toggles & <CR> sets."),
	N_("Remove soft hyphens"),
	N_("# If ON remove soft hyphens when displaying articles\n")
};
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

struct opttxt txt_hide_uue = {
	N_("Display uuencoded data as tagged attachments. <SPACE> toggles & <CR> sets."),
	N_("Display uue data as an attachment"),
	N_("# Handling of uuencoded data in the pager\n\
# Possible values are (the default is marked with *):\n\
# * 0 = No, display raw uuencoded data\n\
#   1 = Yes, uuencoded data will be condensed to a single tag line showing\n\
#       size and filename, similar to how MIME attachments are displayed\n\
#   2 = Hide all, as for 1, but any line that looks like uuencoded data will\n\
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

struct opttxt txt_verbatim_handling = {
	N_("Enable detection of verbatim blocks? <SPACE> toggles & <CR> sets."),
	N_("Detection of verbatim blocks"),
	N_("# If ON detect verbatim blocks in articles\n")
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

struct opttxt txt_extquote_handling = {
	N_("Enable detection of external quotes? <SPACE> toggles & <CR> sets."),
	N_("Detection of external quotes"),
	N_("# If ON detect quoted text from external sources in articles\n")
};

struct opttxt txt_extquote_regex = {
	N_("A regex used to decide which lines to show in col_extquote."),
	N_("Regex used to show quotes from external sources"),
	N_("# A regular expression that tin will use to decide which lines are\n\
# external quotes. Text from external quotes is shown in col_extquote.\n\
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

struct opttxt txt_verbatim_begin_regex = {
	N_("A regex used to find the begin of a verbatim block."),
	N_("Regex for begin of a verbatim block"),
	N_("# A regular expression that tin will use to find the begin of\n\
# a verbatim block.\n")
};

struct opttxt txt_verbatim_end_regex = {
	N_("A regex used to find the end of a verbatim block."),
	N_("Regex for end of a verbatim block"),
	N_("# A regular expression that tin will use to find the end of\n\
# a verbatim block.\n")
};

struct opttxt txt_metamail_prog = {
	N_("Enter name and options for external MIME viewer, --internal for built-in viewer"),
	N_("MIME binary content viewer"),
	N_("# If --internal automatically use the built in MIME viewer for non-text\n\
# parts of articles.\n\
# Otherwise specify an external viewer program (e.g. metamail) or leave blank\n\
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
	N_("# What should we ask confirmation for.\n\
# Possible values are (the default is marked with *):\n\
#   0 = none\n\
#   1 = commands\n\
#   2 = select\n\
#   3 = quit\n\
# * 4 = commands & quit\n\
#   5 = commands & select\n\
#   6 = quit & select\n\
#   7 = commands & quit & select\n")
};

struct opttxt txt_mark_ignore_tags = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("'Mark as (un)read' ignores tags"),
	N_("# If ON the 'Mark as (un)read' function marks only the current article.\n")
};

struct opttxt txt_url_handler = {
	N_("Program to run to open URLs, <CR> sets, <ESC> cancels."),
	N_("Program that opens URLs"),
	N_("# The program used to open URLs. The actual URL will be appended\n")
};

struct opttxt txt_use_mouse = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use mouse in xterm"),
	N_("# If ON enable mouse button support on xterm terminals\n")
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

struct opttxt txt_col_extquote = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of external quotes"),
	N_("# Color of quoted text from external sources\n\
# Default: 5 (pink)\n")
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

struct opttxt txt_col_score_neg = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of negative score"),
	N_("# Color of negative score\n\
# Default: 1 (red)\n")
};

struct opttxt txt_col_score_pos = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of positive score"),
	N_("# Color of positive score\n\
# Default: 2 (green)\n")
};

struct opttxt txt_col_urls = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of highlighted URLs"),
	N_("# Color of highlighted URLs\n\
# Default: -1 (default color)\n")
};

struct opttxt txt_col_verbatim = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Color of verbatim blocks"),
	N_("# Color of verbatim blocks\n\
# Default: 5 (pink)\n")
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
# sigfile=file       appends file as signature\n\
# sigfile=!command   executes external command to generate a signature\n\
#                    (specify %G to pass name of current newsgroup)\n\
# sigfile=--none     don't append a signature\n")
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
	N_("Enter quotation marks, %I for author's initials."),
	N_("Characters used as quote-marks"),
	N_("# Characters used in quoting to followups and replies.\n\
# '_' is replaced by ' ', %I is replaced by author's initials.\n")
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
# %A Address    %D Date   %F Addr+Name   %G Groupname   %M Message-ID\n\
# %N Full Name  %C First Name   %I Initials\n")
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
	N_("Insert 'User-Agent:' header"),
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
#	ifdef NO_LOCALE
struct opttxt txt_mm_local_charset = {
	N_("Enter local charset name (e.g. US-ASCII, ISO-8859-1, EUC-KR), <CR> to set."),
	N_("MM_LOCAL_CHARSET"),
	N_("# Charset supported locally.\n")
};
#	endif /* NO_LOCALE */
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

#ifndef USE_CURSES
struct opttxt txt_strip_blanks = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Strip blanks from ends of lines"),
	N_("# If ON strip blanks from ends of lines for faster display on slow terminals.\n")
};
#endif /* !USE_CURSES */

#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
struct opttxt txt_translit = {
	N_("If ON, use transliteration. <SPACE> toggles & <CR> sets."),
	N_("Transliteration"),
	N_("# If ON, use //TRANSLIT extension. This means that when a character cannot\n\
# be represented in the in the target character set, it can be approximated\n\
# through one or several similarly looking characters.\n")
};
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */

struct opttxt txt_auto_cc_bcc = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Send you a Cc/Bcc automatically"),
	N_("# Put your name in the Cc: and/or Bcc: field when mailing an article.\n\
# Possible values are (the default is marked with *):\n\
# * 0 = No\n\
#   1 = Cc\n\
#   2 = Bcc\n\
#   3 = Cc and Bcc\n")
};

#ifdef USE_CANLOCK
struct opttxt txt_cancel_lock_algo = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Hash algorithm for Cancel-Lock/Cancel-Key"),
	N_("# Hash algorithm for Cancel-Lock/Cancel-Key (default 'sha1')\n\
# Use 'none' to not generate Cancel-Lock headers.\n")
};
#endif /* USE_CANLOCK */

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

struct opttxt txt_mark_saved_read = {
	N_("Mark saved articles/threads as read. <SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Mark saved articles/threads as read"),
	N_("# If ON mark articles that are saved as read\n")
};

struct opttxt txt_post_process_type = {
	N_("Do post processing (e.g. extract attachments) for saved articles."),
	N_("Post process saved articles"),
	N_("# Perform post processing (saving binary attachments) from saved articles.\n\
# Possible values are (the default is marked with *):\n\
# * 0 = No\n\
#   1 = extract shell archives (shar) only\n\
#   2 = Yes\n")
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

struct opttxt txt_editor_format = {
	N_("Enter %E for editor, %F for filename, %N for line-number, <CR> to set."),
	N_("Invocation of your editor"),
	N_("# Format of editor line including parameters\n\
# %E Editor  %F Filename  %N Linenumber\n")
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
# %M Mailer  %S Subject  %T To  %F Filename\n\
# e.g. to use mutt as your mailer:    mutt -s \"%S\" -- \"%T\" < %F\n\
# e.g. mutt interactive          :    mutt -H %F\n")
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

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
struct opttxt txt_utf8_graphics = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Use UTF-8 graphics (thread tree etc.)"),
	N_("# If ON use UTF-8 characters for indicator '->', tree and ellipsis '...'.\n")
};
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

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

#if defined(HAVE_ALARM) && defined(SIGALRM)
struct opttxt txt_nntp_read_timeout_secs = {
	N_("Enter number of seconds to wait for a response from the server. <CR> sets."),
	N_("NNTP read timeout in seconds"),
	N_("# Time in seconds to wait for a response from the server (0=no timeout)\n")
};
#endif /* HAVE_ALARM && SIGALRM */

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

struct opttxt txt_select_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for selection level"),
	N_("# Format string for selection level representation\n\
# Default: %f %n %U  %G  %d\n\
# Possible values are:\n\
#   %%              '%'\n\
#   %d              Description\n\
#   %f              Newsgroup flag: 'D' bogus, 'X' not postable,\n\
#                   'M' moderated, '=' renamed, 'N' new, 'u' unsubscribed\n\
#   %G              Group name\n\
#   %n              Number, linenumber on screen\n\
#   %U              Unread count\n")
};

struct opttxt txt_group_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for group level"),
	N_("# Format string for group level representation\n\
# Default: %n %m %R %L  %s  %F\n\
# Possible values are:\n\
#   %%              '%'\n\
#   %D              Date, like date_format\n\
#   %(formatstr)D   Date, formatstr gets passed to my_strftime()\n\
#   %F              From, name and/or address according to show_author\n\
#   %I              Initials\n\
#   %L              Line count\n\
#   %M              Message-ID\n\
#   %m              Article marks\n\
#   %n              Number, linenumber on screen\n\
#   %R              Count, number of responses in thread\n\
#   %s              Subject (only group level)\n\
#   %S              Score\n")
};

struct opttxt txt_attachment_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for attachment level"),
	N_("# Format string for attachment level representation\n\
# Default: %t%s%e%c%d\n\
# Possible values are:\n\
#   %%              '%'\n\
#   %C              Charset\n\
#   %c              Like %C but with description\n\
#   %D              Line count\n\
#   %d              Like %D but with description\n\
#   %E              Content encoding\n\
#   %e              Like %E but with description\n\
#   %L              Language\n\
#   %l              Like %L but with description\n\
#   %S              Content subtype\n\
#   %s              Like %S but with description\n\
#   %T              Content type\n\
#   %t              Like %T but with description\n\
#   %Z              Size in bytes\n\
#   %z              Like %Z but with description\n")
};

struct opttxt txt_page_mime_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for display of mime header"),
	N_("# Format string for mime header at article level\n\
# Default: [-- %T%S%*n%z%*l%!c%!d%*e --]\n\
# Possible values are:\n\
#   %%              '%'\n\
#   %C              Charset\n\
#   %c              Like %C but with description\n\
#   %D              Line count\n\
#   %d              Like %D but with description\n\
#   %E              Content encoding\n\
#   %e              Like %E but with description\n\
#   %L              Language\n\
#   %l              Like %L but with description\n\
#   %N              Name\n\
#   %n              Like %N but with description\n\
#   %S              Content subtype\n\
#   %s              Like %S but with description\n\
#   %T              Content type\n\
#   %t              Like %T but with description\n\
#   %Z              Size in bytes\n\
#   %z              Like %Z but with description\n")
};

struct opttxt txt_page_uue_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for display of uue header"),
	N_("# Format string for uue header at article level\n\
# Default: [-- %T%S%*n%I%!d%*e --]\n\
# Possible values are:\n\
#   %%              '%'\n\
#   %D              Line count\n\
#   %d              Like %D but with description\n\
#   %E              Content encoding\n\
#   %e              Like %E but with description\n\
#   %I              Complete/incomplete UUE part indicator\n\
#   %N              Name\n\
#   %n              Like %N but with description\n\
#   %S              Content subtype\n\
#   %s              Like %S but with description\n\
#   %T              Content type\n\
#   %t              Like %T but with description\n\
#   %Z              Size in bytes\n\
#   %z              Like %Z but with description\n")
};

struct opttxt txt_thread_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for thread level"),
	N_("# Format string for thread level representation\n\
# Default: %n %m  [%L]  %T  %F\n\
# Possible values are:\n\
#   %%              '%'\n\
#   %D              Date, like date_format\n\
#   %(formatstr)D   Date, formatstr gets passed to my_strftime()\n\
#   %F              From, name and/or address according to show_author\n\
#   %I              Initials\n\
#   %L              Line count\n\
#   %M              Message-ID\n\
#   %m              Article marks\n\
#   %n              Number, linenumber on screen\n\
#   %S              Score\n\
#   %T              Thread tree (only thread level)\n")
};

struct opttxt txt_date_format = {
	N_("Enter format string. <CR> sets, <ESC> cancels."),
	N_("Format string for display of dates"),
	N_("# Format string for date representation\n")
};

/*
 * TODO: needs different text for HAVE_UNICODE_NORMALIZATION < 2 and/or
 *       !HAVE_UNICODE_UNORM2_H
 */
#ifdef HAVE_UNICODE_NORMALIZATION
struct opttxt txt_normalization_form = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Unicode normalization form"),
	N_("# Unicode normalization form\n\
# Possible values are:\n\
#   0 = None\n\
#   1 = NFKC\n\
#   2 = NFKD\n\
#   3 = NFC\n\
#   4 = NFD\n\
#   5 = NFKC_CF\n")
};
#endif /* HAVE_UNICODE_NORMALIZATION */

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
struct opttxt txt_render_bidi = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Render BiDi"),
	N_("# If ON, bi-directional text is rendered by tin\n")
};
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */

#ifdef USE_HEAPSORT
struct opttxt txt_sort_function = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Function for sorting articles"),
	N_("# Function for sorting articles\n\
# Possible values are (the default is marked with *):\n\
# * 0 = qsort\n\
#   1 = heapsort\n")
};
#endif /* USE_HEAPSORT */

/*
 * structs for the attributes menu below,
 * no need for *tinrc text
 */
struct opttxt txt_auto_select = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Automatically GroupMarkUnselArtRead"),
	NULL
};

struct opttxt txt_delete_tmp_files = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Delete post-process files"),
	NULL
};

struct opttxt txt_fcc = {
	N_("Filename for all mailed articles, <CR> sets, no filename=do not save."),
	N_("Mailbox to save sent mails"),
	NULL
};

struct opttxt txt_followup_to = {
	N_("Set Followup-To: header to this group(s). <CR> sets, <ESC> cancels."),
	N_("Followup-To: header"),
	NULL
};

struct opttxt txt_from = {
	N_("Enter default mail address (and fullname). <CR> sets, <ESC> cancels."),
	N_("Mail address (and fullname)"),
	NULL
};

#ifdef HAVE_ISPELL
struct opttxt txt_ispell = {
	N_("Path and options for ispell-like spell-checker. <CR> sets, <ESC> cancels."),
	N_("Ispell program"),
	NULL
};
#endif /* HAVE_ISPELL */

struct opttxt txt_mailing_list = {
	N_("When group is a mailing list, send responses to this email address."),
	N_("Mailing list address"),
	NULL
};

struct opttxt txt_mime_forward = {
	N_("<SPACE> toggles, <CR> sets, <ESC> cancels."),
	N_("Forward articles as attachment"),
	NULL
};

struct opttxt txt_mime_types_to_save = {
	N_("A comma separated list of MIME major/minor Content-Types. <ESC> cancels."),
	N_("Which MIME types will be saved"),
	NULL
};

struct opttxt txt_organization = {
	N_("Value of the Organization: header. <CR> sets, <ESC> cancels."),
	N_("Organization: header"),
	NULL
};

struct opttxt txt_savefile = {
	N_("Filename for saved articles. <CR> sets, <ESC> cancels."),
	N_("savefile"),
	NULL
};

struct opttxt txt_quick_select_scope = {
	N_("Scope for the filter rule. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) select filter scope"),
	NULL
};

struct opttxt txt_quick_select_header = {
	N_("Header for filter rule. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) select filter header"),
	NULL
};

struct opttxt txt_quick_select_case = {
	N_("ON = case sensitive, OFF = ignore case. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) select filter case"),
	NULL
};

struct opttxt txt_quick_select_expire = {
	N_("ON = expire, OFF = don't ever expire. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) select filter expire"),
	NULL
};

struct opttxt txt_quick_kill_scope = {
	N_("Scope for the filter rule. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) kill filter scope"),
	NULL
};

struct opttxt txt_quick_kill_header = {
	N_("Header for filter rule. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) kill filter header"),
	NULL
};

struct opttxt txt_quick_kill_case = {
	N_("ON = case sensitive, OFF = ignore case. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) kill filter case"),
	NULL
};

struct opttxt txt_quick_kill_expire = {
	N_("ON = expire, OFF = don't ever expire. <CR> sets, <ESC> cancels."),
	N_("Quick (1 key) kill filter expire"),
	NULL
};

#ifdef CHARSET_CONVERSION
struct opttxt txt_undeclared_charset = {
	N_("Assume this charset if no charset declaration is present, <CR> to set."),
	N_("UNDECLARED_CHARSET"),
	NULL
};
#endif /* CHARSET_CONVERSION */

struct opttxt txt_x_body = {
	N_("Add this text at the start of the message body. <CR> sets, <ESC> cancels."),
	N_("X_Body"),
	NULL
};

struct opttxt txt_x_headers = {
	N_("Insert this header when posting. <CR> sets, <ESC> cancels."),
	N_("X_Headers"),
	NULL
};

struct opttxt txt_x_comment_to = {
	N_("Automatically insert an X-Comment-To: header? <SPACE> toggles & <CR> sets."),
	N_("Insert 'X-Comment-To:' header"),
	NULL
};

#ifdef NNTPS_ABLE
struct opttxt txt_tls_ca_cert_file = {
	N_("Enter name of file containing trusted CA certificates. <CR> sets."),
	N_("CA certificate file"),
	N_("# name of file containing all trusted CA certificates (empty = system default)\n")
};
#endif /* NNTPS_ABLE */
