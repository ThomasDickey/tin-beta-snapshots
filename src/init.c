/*
 *  Project   : tin - a Usenet reader
 *  Module    : init.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2025-02-20
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
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */
#ifndef included_trace_h
#	include "trace.h"
#endif /* !included_trace_h */
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */
#ifndef BUGREP_H
#	include "bugrep.h"
#endif /* !BUGREP_H */

/*
 * local prototypes
 */
static int read_site_config(void);
#ifdef HAVE_COLOR
	static void preinit_colors(void);
#endif /* HAVE_COLOR */

#ifndef NNTP_ONLY
char active_times_file[PATH_LEN];
char overviewfmt_file[PATH_LEN];	/* full path to overview.fmt */
char subscriptions_file[PATH_LEN];	/* full path to subscriptions */
#endif /* !NNTP_ONLY */

char article_name[PATH_LEN];			/* ~/TIN_ARTICLE_NAME file */
char *backup_article_name;			/* ~/TIN_ARTICLE_NAME[.pid].b[ak] file */
#ifdef NNTP_ABLE
	char bug_nntpserver1[PATH_LEN];		/* welcome message of NNTP server used */
	char bug_nntpserver2[PATH_LEN];		/* welcome message of NNTP server used */
#endif /* NNTP_ABLE */
char cvers[LEN];
char dead_article[PATH_LEN];		/* ~/dead.article file */
char dead_articles[PATH_LEN];		/* ~/dead.articles file */
char *default_filter_kill_global;
char *default_filter_select_global;
char *default_organization;			/* Organization: */
char *default_mime_types_to_save;
char default_signature[PATH_LEN];
char domain_name[MAXHOSTNAMELEN + 1];
char global_attributes_file[PATH_LEN];
char global_config_file[PATH_LEN];
char global_defaults_file[PATH_LEN];
char homedir[PATH_LEN];
char index_maildir[PATH_LEN];
char index_newsdir[PATH_LEN];	/* directory for private overview data */
char index_savedir[PATH_LEN];
char inewsdir[PATH_LEN];
char filter_file[PATH_LEN];
char keymap_file[PATH_LEN];
char local_attributes_file[PATH_LEN];
char local_config_file[PATH_LEN];
char local_input_history_file[PATH_LEN];
char local_motd_file[PATH_LEN];	/* local copy of NNTP MOTD message */
char local_newsgroups_file[PATH_LEN];	/* local copy of NNTP newsgroups file */
char newsrctable_file[PATH_LEN];
char lock_file[PATH_LEN];		/* contains name of index lock file */
char mail_news_user[LEN];		/* mail new news to this user address */
char mailbox[PATH_LEN];			/* system mailbox for each user */
char mailer[PATH_LEN];			/* mail program */
char newnewsrc[PATH_LEN];
char news_active_file[PATH_LEN];
char newsgroups_file[PATH_LEN];
char newsrc[PATH_LEN];
char page_header[LEN];			/* page header of pgm name and version */
char posted_info_file[PATH_LEN];
char postponed_articles_file[PATH_LEN];	/* ~/.tin/postponed.articles file */
char rcdir[PATH_LEN];
char save_active_file[PATH_LEN];
char spooldir[PATH_LEN];		/* directory where news is */
char *tin_progname;		/* program name */
const char *tmpdir;
char txt_help_bug_report[LEN];		/* address to send bug reports to */
char userid[LOGIN_NAME_MAX];
#ifdef HAVE_MH_MAIL_HANDLING
	char mail_active_file[PATH_LEN];
	char mailgroups_file[PATH_LEN];
#endif /* HAVE_MH_MAIL_HANDLING */
#ifndef NNTP_ONLY
	char novfilename[NAME_LEN + 1];		/* file name of a single nov index file */
	char novrootdir[PATH_LEN];		/* root directory of nov index files */
#endif /* !NNTP_ONLY */

t_function last_search = GLOBAL_SEARCH_REPEAT;	/* for repeated search */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	int art_mark_width = 1;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
int hist_last[HIST_MAXNUM + 1];
int hist_pos[HIST_MAXNUM + 1];
int iso2asc_supported;			/* Convert ISO-Latin1 to Ascii */
int system_status;
int xmouse, xrow, xcol;			/* xterm button pressing information */

long motd_hash = 0L;

pid_t process_id;			/* Useful to have around for .suffixes */

t_bool batch_mode;			/* update index files only mode */
t_bool check_for_new_newsgroups;	/* don't check for new newsgroups */
t_bool cmd_line;			/* batch / interactive mode */
t_bool created_rcdir;			/* checks if first time tin is started */
t_bool dangerous_signal_exit;		/* no get_respcode() in nntp_command when dangerous signal exit */
t_bool disable_gnksa_domain_check;	/* disable checking TLD in From: etc. */
t_bool disable_sender;			/* disable generation of Sender: header */
#ifdef NO_POSTING
	t_bool force_no_post = TRUE;		/* force no posting mode */
#else
	t_bool force_no_post = FALSE;	/* don't force no posting mode */
#endif /* NO_POSTING */
#if defined(NNTP_ABLE) && defined(INET6)
	t_bool force_ipv4 = FALSE;
	t_bool force_ipv6 = FALSE;
#endif /* NNTP_ABLE && INET6 */
t_bool list_active;
t_bool newsrc_active;
t_bool no_write = FALSE;		/* do not write newsrc on quit (-X cmd-line flag) */
t_bool post_article_and_exit;		/* quick post of an article then exit (elm like) */
t_bool post_postponed_and_exit;		/* post postponed articles and exit */
t_bool range_active;		/* Set if a range is defined */
t_bool reread_active_for_posted_arts;
t_bool read_local_newsgroups_file;	/* read newsgroups file locally or via NNTP */
t_bool read_news_via_nntp = FALSE;	/* read news locally or via NNTP */
t_bool read_saved_news = FALSE;		/* tin -R read saved news from tin -S */
t_bool show_description = TRUE;		/* current copy of tinrc flag */
t_bool use_nntps = FALSE;		/* use NNTPS */
t_bool insecure_nntps = FALSE;		/* allow insecure NNTPS, i.e. disable peer cert verification */
int verbose = 0;			/* batch or debug mode */
t_bool word_highlight;		/* word highlighting on/off */
t_bool xref_supported = TRUE;
#ifdef HAVE_COLOR
	t_bool use_color;		/* enables/disables ansi-color support under linux-console and color-xterm */
#endif /* HAVE_COLOR */
#ifdef NNTP_ABLE
	t_bool force_auth_on_conn_open = FALSE;	/* authenticate on connection startup */
	unsigned short nntp_tcp_port = 0;
	unsigned short nntp_tcp_default_port;
#ifdef USE_ZLIB
	t_bool use_compress = FALSE;
#endif /* USE_ZLIB */
#ifdef NNTPS_ABLE
	unsigned short nntps_tcp_default_port;
#endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */

/* Currently active menu parameters */
t_menu *currmenu;

/* History entries */
char *input_history[HIST_MAXNUM + 1][HIST_SIZE + 1];

#ifdef HAVE_SYS_UTSNAME_H
	struct utsname system_info;
#endif /* HAVE_SYS_UTSNAME_H */

struct regex_cache
		strip_re_regex, strip_was_regex,
		uubegin_regex, uubody_regex,
		verbatim_begin_regex, verbatim_end_regex,
		url_regex, mail_regex, news_regex,
		shar_regex,
		slashes_regex, stars_regex, underscores_regex, strokes_regex
#ifdef HAVE_COLOR
		, extquote_regex, quote_regex, quote_regex2, quote_regex3
#endif /* HAVE_COLOR */
	= REGEX_CACHE_INITIALIZER;

struct t_cmdlineopts cmdline = { 0, 0, NULL, NULL, NULL, NULL, 0 };

struct t_config tinrc = {
	NULL,	/* default_goto_group */
	NULL,	/* default_mail_address */
#ifndef DONT_HAVE_PIPING
	NULL,	/* default_pipe_command */
#endif /* !DONT_HAVE_PIPING */
	NULL,	/* default_post_newsgroups */
	NULL,	/* default_post_subject */
	NULL,	/* default_range_group */
	NULL,	/* default_range_select */
	NULL,	/* default_range_thread */
	NULL,	/* default_pattern */
	NULL,	/* default_repost_group */
	NULL,	/* default_save_file */
	NULL,	/* default_search_art */
	NULL,	/* default_search_author */
	NULL,	/* default_search_config */
	NULL,	/* default_search_group */
	NULL,	/* default_search_subject */
	NULL,	/* default_select_pattern */
	NULL,	/* default_shell_command */
	NULL,	/* editor_format */
	NULL,	/* select_format */
	NULL,	/* group_format */
	NULL,	/* thread_format */
	NULL,	/* attachment_format */
	NULL,	/* page_mime_format */
	NULL,	/* page_uue_format */
	NULL,	/* date_format */
	NULL,	/* mail_quote_format */
	NULL,	/* mailer_format */
	NULL,	/* news_quote_format */
	NULL,	/* xpost_quote_format */
	NULL,	/* quote_chars */
	NULL,	/* inews_prog */
	NULL,	/* mail_address */
	NULL,	/* maildir */
	NULL,	/* metamail_prog */
	NULL,	/* mm_local_charset, display charset */
	NULL,	/* news_headers_to_display */
	NULL,	/* news_headers_to_not_display */
	NULL,	/* posted_articles_file */
#ifndef DISABLE_PRINTING
	NULL,	/* printer */
#endif /* !DISABLE_PRINTING */
	NULL,	/* slashes_regex */
	NULL,	/* stars_regex */
	NULL,	/* strokes_regex */
	NULL,	/* strip_re_regex */
	NULL,	/* strip_was_regex */
#ifdef HAVE_COLOR
	NULL,	/* quote_regex */
	NULL,	/* quote_regex 2nd level */
	NULL,	/* quote_regex >= 3rd level */
	NULL,	/* extquote_regex */
#endif /* HAVE_COLOR */
	NULL,	/* underscores_regex */
	NULL,	/* verbatim_begin_regex */
	NULL,	/* verbatim_end_regex */
	NULL,	/* savedir */
	NULL,	/* sigfile */
	NULL,	/* spamtrap_warning_addresses */
#ifdef NNTPS_ABLE
	NULL,	/* tls_ca_cert_file */
#endif /* NNTPS_ABLE */
	NULL,	/* url_handler */
#ifndef CHARSET_CONVERSION
	NULL,	/* mm_charset, defaults to $MM_CHARSET */
#else
	-1,		/* mm_network_charset, defaults to $MM_CHARSET */
#endif /* !CHARSET_CONVERSION */
#ifdef SCO_UNIX
	2,		/* mailbox_format = MMDF */
#else
	0,		/* mailbox_format = MBOXO */
#endif /* SCO_UNIX */
	0,		/* auto_cc_bcc */
#ifdef USE_CANLOCK
	1,		/* cancel_lock_algo, sha1 */
#endif /* USE_CANLOCK */
#ifdef HAVE_COLOR
	0,		/* col_back (initialised later) */
	0,		/* col_from (initialised later) */
	0,		/* col_head (initialised later) */
	0,		/* col_help (initialised later) */
	0,		/* col_invers_bg (initialised later) */
	0,		/* col_invers_fg (initialised later) */
	0,		/* col_minihelp (initialised later) */
	0,		/* col_normal (initialised later) */
	0,		/* col_markdash (initialised later) */
	0,		/* col_markstar (initialised later) */
	0,		/* col_markslash (initialised later) */
	0,		/* col_markstroke (initialised later) */
	0,		/* col_message (initialised later) */
	0,		/* col_newsheaders (initialised later) */
	0,		/* col_quote (initialised later) */
	0,		/* col_quote2 (initialised later) */
	0,		/* col_quote3 (initialised later) */
	0,		/* col_extquote (initialised later) */
	0,		/* col_response (initialised later) */
	0,		/* col_signature (initialised later) */
	0,		/* col_score_neg (initialised later) */
	0,		/* col_score_pos (initialised later) */
	0,		/* col_urls (initialised later) */
	0,		/* col_verbatim (initialised later) */
	0,		/* col_subject (initialised later) */
	0,		/* col_text (initialised later) */
	0,		/* col_title (initialised later) */
#endif /* HAVE_COLOR */
	4,		/* confirm_choice */
	FILTER_SUBJ_CASE_SENSITIVE,	/* default_filter_kill_header */
	FILTER_SUBJ_CASE_SENSITIVE,	/* default_filter_select_header */
	DEFAULT_FILTER_DAYS,		/* filter_days */
	0,		/* default_move_group */
	'a',	/* default_save_mode */
	0,		/* getart_limit */
	GOTO_NEXT_UNREAD_TAB,		/* goto_next_unread */
	UUE_NO,	/* hide_uue */
	INTERACTIVE_NONE,			/* interactive_mailer */
	KILL_UNREAD,				/* kill_level */
	2,		/* mono_markdash */
	6,		/* mono_markstar */
	5,		/* mono_markslash */
	3,		/* mono_markstroke */
#if defined(HAVE_ALARM) && defined(SIGALRM)
	120,	/* nntp_read_timeout_secs */
#endif /* HAVE_ALARM && SIGALRM */
#ifdef HAVE_UNICODE_NORMALIZATION
	DEFAULT_NORMALIZE,	/* normalization_form */
#endif /* HAVE_UNICODE_NORMALIZATION */
	MIME_ENCODING_QP,			/* mail_mime_encoding */
	MIME_ENCODING_8BIT,			/* post_mime_encoding */
	POST_PROC_NO,				/* post_process_type */
	QUOTE_COMPRESS|QUOTE_EMPTY,	/* quote_style */
	2,		/* recent_time */
	REREAD_ACTIVE_FILE_SECS,	/* reread_active_file_secs */
	-50,	/* score_limit_kill */
	50,		/* score_limit_select */
	-100,	/* score_kill */
	100,	/* score_select */
	1,		/* scroll_lines */
	SHOW_FROM_NAME,	/* show_author */
	SHOW_SIGN_BOTH,	/* show help/mail sign in level title */
	SORT_ARTICLES_BY_DATE_ASCEND,	/* sort_article_type */
	SORT_THREADS_BY_SCORE_DESCEND,	/* sort_threads_type */
#ifdef USE_HEAPSORT
	0,		/* sort_function default qsort */
#endif /* USE_HEAPSORT */
	BOGUS_SHOW,				/* strip_bogus */
	THREAD_BOTH,			/* thread_articles */
	THREAD_PERC_DEFAULT,	/* thread_perc */
	THREAD_SCORE_MAX,		/* thread_score */
	0,		/* trim_article_body */
	VERBATIM_SHOW_ALL,		/* verbatim_handling */
	0,		/* Default to wildmat, not regex */
	2,		/* word_h_display_marks */
	0,		/* wrap_column */
#ifdef HAVE_COLOR
	FALSE,	/* use_color */
#endif /* HAVE_COLOR */
	FALSE,	/* abbreviate_groupname */
	TRUE,	/* add_posted_to_filter */
	TRUE,	/* advertising */
	TRUE,	/* alternative_handling */
	FALSE,	/* ask_for_metamail */
	TRUE,	/* auto_list_thread */
	FALSE,	/* auto_reconnect */
	TRUE,	/* batch_save */
	TRUE,	/* beginner_level */
	FALSE,	/* cache_overview_files */
	FALSE,	/* catchup_read_groups */
#ifdef USE_ZLIB
	FALSE,	/* compress_overview_files */
#endif /* USE_ZLIB */
#ifdef USE_INVERSE_HACK
	TRUE,	/* draw_arrow */
#else
	FALSE,	/* draw_arrow */
#endif /* USE_INVERSE_HACK */
	FALSE,	/* dont_break_words */
	FALSE,	/* force_screen_redraw */
	TRUE,	/* group_catchup_on_exit */
	FALSE,	/* info_in_last_line */
#ifdef USE_INVERSE_HACK
	FALSE,	/* inverse_okay */
#else
	TRUE,	/* inverse_okay */
#endif /* USE_INVERSE_HACK */
	TRUE,	/* keep_dead_articles */
	FALSE,	/* mail_8bit_header */
	FALSE,	/* mark_ignore_tags */
	TRUE,	/* mark_saved_read */
	TRUE,	/* pos_first_unread */
	FALSE,	/* post_8bit_header */
	TRUE,	/* post_process_view */
#ifndef DISABLE_PRINTING
	FALSE,		/* print_header */
#endif /* !DISABLE_PRINTING */
	FALSE,	/* process_only_unread */
	FALSE,	/* prompt_followupto */
	TRUE,	/* show_description */
	TRUE,	/* show_only_unread_arts */
	FALSE,	/* show_only_unread_groups */
	TRUE,	/* show_signatures */
	FALSE,	/* show_art_score */
	TRUE,	/* sigdashes */
	TRUE,	/* signature_repost */
#ifndef USE_CURSES
	TRUE,	/* strip_blanks */
#endif /* !USE_CURSES */
	FALSE,	/* strip_newsrc */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	FALSE,	/* suppress_soft_hyphens */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	FALSE,	/* tex2iso_conv */
	TRUE,	/* thread_catchup_on_exit */
#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
	FALSE,	/* translit */
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */
	TRUE,	/* unlink_article */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	FALSE,	/* utf8_graphics */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#ifdef HAVE_COLOR
	FALSE,	/* extquote_handling */
#endif /* HAVE_COLOR */
	FALSE,	/* use_mouse */
#ifdef HAVE_KEYPAD
	FALSE,	/* use_keypad */
#endif /* HAVE_KEYPAD */
	FALSE,	/* default_filter_kill_case */
	FALSE,	/* default_filter_kill_expire */
	TRUE,	/* default_filter_kill_global */
	FALSE,	/* default_filter_select_case */
	FALSE,	/* default_filter_select_expire */
	TRUE,	/* default_filter_select_global */
#ifdef XFACE_ABLE
	FALSE,	/* use_slrnface */
#endif /* XFACE_ABLE */
#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
	FALSE,	/* render_bidi */
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
	TRUE,	/* url_highlight */
	TRUE,	/* word_highlight */
	TRUE,	/* wrap_on_next_unread */
	ART_MARK_DELETED,		/* art_marked_deleted */
	MARK_INRANGE,			/* art_marked_inrange */
	ART_MARK_RETURN,		/* art_marked_return */
	ART_MARK_SELECTED,		/* art_marked_selected */
	ART_MARK_RECENT,		/* art_marked_recent */
	ART_MARK_UNREAD,		/* art_marked_unread */
	ART_MARK_READ,			/* art_marked_read */
	ART_MARK_KILLED,		/* art_marked_killed */
	ART_MARK_READ_SELECTED,	/* art_marked_read_selected */
	NULL,	/* attrib_editor_format */
	NULL,	/* attrib_fcc */
	NULL,	/* attrib_maildir */
	NULL,	/* attrib_from */
	NULL,	/* attrib_mailing_list */
	NULL,	/* attrib_organization */
	NULL,	/* attrib_followup_to */
	NULL,	/* attrib_mime_types_to_save */
	NULL,	/* attrib_news_headers_to_display */
	NULL,	/* attrib_news_headers_to_not_display */
	NULL,	/* attrib_news_quote_format */
	NULL,	/* attrib_quote_chars */
	NULL,	/* attrib_sigfile */
	NULL,	/* attrib_savedir */
	NULL,	/* attrib_savefile */
	NULL,	/* attrib_x_body */
	NULL,	/* attrib_x_headers */
#ifdef HAVE_ISPELL
	NULL,	/* attrib_ispell */
#endif /* HAVE_ISPELL */
	NULL,	/* attrib_quick_kill_scope */
	NULL,	/* attrib_quick_select_scope */
	NULL,	/* attrib_group_format */
	NULL,	/* attrib_thread_format */
	NULL,	/* attrib_date_format */
#ifdef CHARSET_CONVERSION
	NULL,	/* attrib_undeclared_charset */
	-1,		/* attrib_mm_network_charset, defaults to $MM_CHARSET */
#	ifdef USE_ICU_UCSDET
	FALSE,
#	endif /* USE_ICU_UCSDET */
#endif /* CHARSET_CONVERSION */
	0,		/* attrib_trim_article_body */
	VERBATIM_SHOW_ALL,	/* attrib_verbatim_handling */
	0,		/* attrib_auto_cc_bcc */
	FILTER_SUBJ_CASE_SENSITIVE,		/* attrib_quick_kill_header */
	FILTER_SUBJ_CASE_SENSITIVE,		/* attrib_quick_select_header */
	MIME_ENCODING_QP,		/* attrib_mail_mime_encoding */
	MIME_ENCODING_8BIT,		/* attrib_post_mime_encoding */
	POST_PROC_NO,			/* attrib_post_process_type */
	SHOW_FROM_NAME,			/* attrib_show_author */
	SORT_ARTICLES_BY_DATE_ASCEND,	/* attrib_sort_article_type */
	SORT_THREADS_BY_SCORE_DESCEND,	/* attrib_sort_threads_type */
	THREAD_BOTH,			/* attrib_thread_articles */
	THREAD_PERC_DEFAULT,	/* attrib_thread_perc */
	TRUE,	/* attrib_add_posted_to_filter */
	TRUE,	/* attrib_advertising */
	TRUE,	/* attrib_alternative_handling */
	TRUE,	/* attrib_auto_list_thread */
	FALSE,	/* attrib_auto_select */
	TRUE,	/* attrib_batch_save */
	TRUE,	/* attrib_delete_tmp_files */
	TRUE,	/* attrib_group_catchup_on_exit */
	FALSE,	/* attrib_mail_8bit_header */
	FALSE,	/* attrib_mime_forward */
	FALSE,	/* attrib_mark_ignore_tags */
	TRUE,	/* attrib_mark_saved_read */
	TRUE,	/* attrib_pos_first_unread */
	FALSE,	/* attrib_post_8bit_header */
	TRUE,	/* attrib_post_process_view */
#ifndef DISABLE_PRINTING
	FALSE,	/* attrib_print_header */
#endif /* !DISABLE_PRINTING */
	FALSE,	/* attrib_process_only_unread */
	FALSE,	/* attrib_prompt_followupto */
	TRUE,	/* attrib_show_only_unread_arts */
	TRUE,	/* attrib_show_signatures */
	FALSE,	/* attrib_show_art_score */
	TRUE,	/* attrib_sigdashes */
	TRUE,	/* attrib_signature_repost */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	FALSE,	/* suppress_soft_hyphens */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	FALSE,	/* attrib_tex2iso_conv */
	TRUE,	/* attrib_thread_catchup_on_exit */
#ifdef HAVE_COLOR
	FALSE,	/* attrib_extquote_handling */
#endif /* HAVE_COLOR */
	FALSE,	/* attrib_x_comment_to */
	TRUE,	/* attrib_wrap_on_next_unread */
	FALSE,	/* attrib_ask_for_metamail */
	FALSE,	/* attrib_quick_kill_case */
	FALSE,	/* attrib_quick_kill_expire */
	FALSE,	/* attrib_quick_select_case */
	FALSE	/* attrib_quick_select_expire */
};

struct t_capabilities nntp_caps = {
	NONE, /* type (NONE, CAPABILITIES, BROKEN) */
	0, /* CAPABILITIES version */
	FALSE, /* MODE-READER: "MODE READER" */
	FALSE, /* READER: "ARTICLE", "BODY" */
	FALSE, /* POST */
	FALSE, /* LIST: "LIST ACTIVE" */
	FALSE, /* LIST: "LIST ACTIVE.TIMES" */
	FALSE, /* LIST: "LIST DISTRIB.PATS" */
	FALSE, /* LIST: "LIST HEADERS" */
	NULL, /* list of headers by range */
	NULL, /* list of headers by id */
	FALSE, /* LIST: "LIST NEWSGROUPS" */
	FALSE, /* LIST: "LIST OVERVIEW.FMT" */
	FALSE, /* LIST: "LIST MOTD" */
	FALSE, /* LIST: "LIST SUBSCRIPTIONS" */
	FALSE, /* LIST: "LIST DISTRIBUTIONS" */
	FALSE, /* LIST: "LIST MODERATORS" */
	FALSE, /* LIST: "LIST COUNTS" */
	FALSE, /* XPAT */
	FALSE, /* HDR: "HDR", "LIST HEADERS" */
	NULL, /* [X]HDR */
	FALSE, /* OVER: "OVER", "LIST OVERVIEW.FMT" */
	FALSE, /* OVER: "OVER mid" */
	NULL, /* [X]OVER */
	FALSE, /* NEWNEWS */
	NULL, /* IMPLEMENTATION */
	FALSE, /* STARTTLS */
	FALSE, /* AUTHINFO USER/PASS */
	FALSE, /* AUTHINFO SASL */
	FALSE, /* AUTHINFO available but not in current state */
	NULL, /* SASL mechs implemented on both sides */
	NULL, /* SASL mech used after handshake */
	FALSE, /* COMPRESS */
	COMPRESS_NONE, /* COMPRESS_NONE, COMPRESS_DEFLATE */
#if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
	T_ARTNUM_CONST(0), /* MAXARTNUM */
#endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
#if 0
	FALSE, /* STREAMING: "MODE STREAM", "CHECK", "TAKETHIS" */
	FALSE /* IHAVE */
#endif /* 0 */
#ifndef BROKEN_LISTGROUP
	FALSE /* LISTGROUP doesn't select group */
#else
	TRUE
#endif /* !BROKEN_LISTGROUP */
};

static char libdir[PATH_LEN];			/* directory where news config files are (ie. active) */
static mode_t real_umask;

#ifdef HAVE_COLOR

#	define DFT_FORE -1
#	define DFT_BACK -2
#	define DFT_INIT -3

static const struct {
	int *colorp;
	int color_dft;	/* -2 back, -1 fore, >=0 normal */
} our_colors[] = {
	{ &tinrc.col_back,       DFT_BACK },
	{ &tinrc.col_from,        2 },
	{ &tinrc.col_head,        2 },
	{ &tinrc.col_help,       DFT_FORE },
	{ &tinrc.col_invers_bg,   4 },
	{ &tinrc.col_invers_fg,   7 },
	{ &tinrc.col_markdash,   13 },
	{ &tinrc.col_markstar,   11 },
	{ &tinrc.col_markslash,  14 },
	{ &tinrc.col_markstroke, 12 },
	{ &tinrc.col_message,     6 },
	{ &tinrc.col_minihelp,    3 },
	{ &tinrc.col_newsheaders, 9 },
	{ &tinrc.col_normal,     DFT_FORE },
	{ &tinrc.col_quote,       2 },
	{ &tinrc.col_quote2,      3 },
	{ &tinrc.col_quote3,      4 },
	{ &tinrc.col_extquote,    5 },
	{ &tinrc.col_response,    2 },
	{ &tinrc.col_signature,   4 },
	{ &tinrc.col_score_neg,   1 },
	{ &tinrc.col_score_pos,   2 },
	{ &tinrc.col_urls,       DFT_FORE },
	{ &tinrc.col_verbatim,    5 },
	{ &tinrc.col_subject,     6 },
	{ &tinrc.col_text,       DFT_FORE },
	{ &tinrc.col_title,       4 },
};


static void
preinit_colors(
	void)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(our_colors); n++)
		*(our_colors[n].colorp) = DFT_INIT;
}


void
postinit_colors(
	int last_color)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(our_colors); n++) {
		if (*(our_colors[n].colorp) == DFT_INIT
		 || *(our_colors[n].colorp) >= last_color) {
			switch (our_colors[n].color_dft) {
				case DFT_FORE:
					*(our_colors[n].colorp) = default_fcol;
					break;

				case DFT_BACK:
					*(our_colors[n].colorp) = default_bcol;
					break;

				default:
					*(our_colors[n].colorp) = our_colors[n].color_dft;
					break;
			}
		}
		TRACE(("postinit_colors [%d] = %d", n, *(our_colors[n].colorp)));
	}
}
#endif /* HAVE_COLOR */


/*
 * Get users home directory, userid, and a bunch of other stuff!
 */
void
init_selfinfo(
	void)
{
	FILE *fp;
	const char *p;
	char tmp[PATH_LEN];
	size_t space;
	const struct passwd *myentry;
	struct stat sb;
#if !defined(NNTP_ONLY) || defined(CHARSET_CONVERSION)
	char *ptr;
#endif /* !NNTP_ONLY || CHARSET_CONVERSION */

	process_id = getpid();

	real_umask = umask(0);
	(void) umask(real_umask);

	if ((myentry = getpwuid(getuid())) == NULL) {
		my_fprintf(stderr, "%s\n", _(txt_error_passwd_missing));
		free(tin_progname);
		giveup();
	} else
		my_strncpy(userid, myentry->pw_name, sizeof(userid) - 1);

	*domain_name = '\0';

#ifdef HAVE_SYS_UTSNAME_H
#	ifdef HAVE_UNAME
	if (uname(&system_info) == -1)
#	endif /* HAVE_UNAME */
	{
		strcpy(system_info.sysname, txt_unknown);
		*system_info.machine = '\0';
		*system_info.release = '\0';
		*system_info.nodename = '\0';
	}
#endif /* HAVE_SYS_UTSNAME_H */

#ifdef DOMAIN_NAME
	if ((p = get_domain_name()) != NULL)
		my_strncpy(domain_name, p, MAXHOSTNAMELEN);
#endif /* DOMAIN_NAME */

#ifdef HAVE_GETHOSTBYNAME
	if (!*domain_name) {
		if ((p = get_fqdn(get_host_name())) != NULL)
			my_strncpy(domain_name, p, MAXHOSTNAMELEN);
	}
#endif /* HAVE_GETHOSTBYNAME */

	tmpdir = get_val("TMPDIR", _PATH_TMP);

	if ((p = get_val("TIN_HOMEDIR", get_val("HOME", NULL))) != NULL) {
		my_strncpy(homedir, p, sizeof(homedir) - 1);
	} else if (strlen(myentry->pw_dir)) {
		strncpy(homedir, myentry->pw_dir, sizeof(homedir) - 1);
	} else
		strncpy(homedir, tmpdir, sizeof(homedir) - 1);

	created_rcdir = FALSE;
	dangerous_signal_exit = FALSE;
	disable_gnksa_domain_check = TRUE;
	disable_sender = FALSE;	/* we set force_no_post=TRUE later on if we don't have a valid FQDN */
	p = get_val("ISO2ASC", DEFAULT_ISO2ASC);
	iso2asc_supported = s2i(p, -1, NUM_ISO_TABLES - 1);
	switch (errno) {
		case EINVAL:
			error_message(0, _(txt_arg_not_numeric), "$ISO2ASC", p);
			if (!batch_mode)
				sleep(2);
			iso2asc_supported = -1;
			break;

		case ERANGE:
			error_message(0, _(txt_val_out_of_range_ignored), "$ISO2ASC", p, -1, NUM_ISO_TABLES - 1);
			if (!batch_mode)
				sleep(2);
			iso2asc_supported = -1;
			break;

		default:
			break;
	}
	list_active = FALSE;
	newsrc_active = FALSE;
	post_article_and_exit = FALSE;
	post_postponed_and_exit = FALSE;
	read_local_newsgroups_file = FALSE;
	force_reread_active_file = TRUE;
	reread_active_for_posted_arts = TRUE;
	batch_mode = FALSE;
	check_for_new_newsgroups = TRUE;

#ifdef HAVE_COLOR
	preinit_colors();
	use_color = FALSE;
#endif /* HAVE_COLOR */

	word_highlight = TRUE;

	index_maildir[0] = '\0';
	index_newsdir[0] = '\0';
	index_savedir[0] = '\0';
	newsrc[0] = '\0';

	snprintf(page_header, sizeof(page_header), "%s %s release %s (\"%s\")%s",
		PRODUCT, VERSION, RELEASEDATE, RELEASENAME,
		(iso2asc_supported >= 0 ? " [ISO2ASC]" : ""));
	snprintf(cvers, sizeof(cvers), txt_copyright_notice, page_header);

	default_mime_types_to_save = my_strdup(DEFAULT_MIME_TYPES_TO_SAVE);

	strncpy(bug_addr, BUG_REPORT_ADDRESS, sizeof(bug_addr) - 1);

#ifdef NNTP_ABLE
	bug_nntpserver1[0] = '\0';
	bug_nntpserver2[0] = '\0';
#endif /* NNTP_ABLE */

#ifdef INEWSDIR
	strncpy(inewsdir, INEWSDIR, sizeof(inewsdir) - 1);
#else
	inewsdir[0] = '\0';
#endif /* INEWSDIR */

	default_organization = NULL;
#ifdef apollo
	default_organization = my_strdup(get_val("NEWSORG", ""));
#else
	default_organization = my_strdup(get_val("ORGANIZATION", ""));
#endif /* apollo */

	default_filter_kill_global = my_strdup(DEFAULT_FILTER);
	default_filter_select_global = my_strdup(DEFAULT_FILTER);

#ifndef NNTP_ONLY
	my_strncpy(libdir, get_val("TIN_LIBDIR", NEWSLIBDIR), sizeof(libdir) - 1);
	my_strncpy(novrootdir, get_val("TIN_NOVROOTDIR", NOVROOTDIR), sizeof(novrootdir) - 1);
	my_strncpy(novfilename, get_val("TIN_NOVFILENAME", OVERVIEW_FILE), sizeof(novfilename) - 1);
	my_strncpy(spooldir, get_val("TIN_SPOOLDIR", SPOOLDIR), sizeof(spooldir) - 1);
	active_times_file[0] = '\0';
	overviewfmt_file[0] = '\0';
	subscriptions_file[0] = '\0';
#endif /* !NNTP_ONLY */
	/* clear news_active_file, newsgroups_file */
	news_active_file[0] = '\0';
	newsgroups_file[0] = '\0';

	/*
	 * read the global site config file to override some default
	 * values given at compile time
	 */
	(void) read_site_config();

	/*
	 * the site_config-file was the last chance to set the domainname
	 * if it's still unset fall into no posting mode.
	 */
	if (!*domain_name) {
		force_no_post = TRUE;
		my_fprintf(stderr, "%s\n%s\n", _(txt_error_no_domain_name), _(txt_cannot_post));
		if (!batch_mode)
			sleep(4);
	}

#ifndef NNTP_ONLY
	/*
	 * only set the following variables if they weren't set from within
	 * read_site_config()
	 *
	 * what if !*libdir - should be use some fallback other than '/' ?
	 *
	 * TODO: do we really want that read_site_config() overwrites
	 * values given in env-vars? ($MM_CHARSET, $TIN_ACTIVEFILE)
	 */
	if (!*news_active_file) {
		p = get_val("TIN_ACTIVEFILE", ACTIVE_FILE);
		if (*p != '/')
			joinpath(news_active_file, sizeof(news_active_file), libdir, p);
		else
			my_strncpy(news_active_file, p, sizeof(news_active_file) - 1);
	}
#ifndef NNTP_ONLY
	if (!*active_times_file)
		joinpath(active_times_file, sizeof(active_times_file), libdir, ACTIVE_TIMES_FILE);
	if (!*overviewfmt_file)
		joinpath(overviewfmt_file, sizeof(overviewfmt_file), libdir, OVERVIEW_FMT);
	if (!*subscriptions_file)
		joinpath(subscriptions_file, sizeof(subscriptions_file), libdir, SUBSCRIPTIONS_FILE);
#endif /* !NNTP_ONLY */
	if (!*newsgroups_file)
		joinpath(newsgroups_file, sizeof(newsgroups_file), libdir, NEWSGROUPS_FILE);
	if (!default_organization || !*default_organization) {
		joinpath(tmp, sizeof(tmp), libdir, "organization");
		if ((fp = tin_fopen(tmp, "r")) != NULL) {
			char buf[LEN];

			if (fgets(buf, (int) sizeof(buf), fp) != NULL) {
				if ((ptr = strrchr(buf, '\n')) != NULL)
					*ptr = '\0';
			}
			fclose(fp);
			my_strncpy(default_organization, buf, sizeof(default_organization) - 1);
			FreeIfNeeded(default_organization);
			default_organization = my_strdup(buf);
		}
	}
#endif /* NNTP_ONLY */

	/*
	 * Formerly get_mm_charset(), read_site_config() may set mm_charset
	 * No need to set tinrc.mm_local_charset here, main.c:main() always sets a default value
	 */
#ifndef CHARSET_CONVERSION
	if (!tinrc.mm_charset || !*tinrc.mm_charset) {
		FreeIfNeeded(tinrc.mm_charset);
		tinrc.mm_charset = my_strdup(get_val("MM_CHARSET", MM_CHARSET));
	}
#else
	if (tinrc.mm_network_charset < 0) {
		space = 127;
		ptr = my_malloc(space + 1);
		/* check $MM_CHARSET */
		if ((p = validate_charset(get_val("MM_CHARSET", NULL)))) {
			snprintf(ptr, space, "mm_network_charset=%s\n", p);
			if (!match_list(ptr, "mm_network_charset=", txt_mime_charsets, &tinrc.mm_network_charset)) {
				/* fallback to MM_CHARSET */
				snprintf(ptr, space, "mm_network_charset=%s\n", MM_CHARSET);
				if (!match_list(ptr, "mm_network_charset=", txt_mime_charsets, &tinrc.mm_network_charset))
					/* fallback to US-ASCII */
					tinrc.mm_network_charset = 0;
			}
		} else {
				/* fallback to MM_CHARSET */
				snprintf(ptr, space, "mm_network_charset=%s\n", MM_CHARSET);
				if (!match_list(ptr, "mm_network_charset=", txt_mime_charsets, &tinrc.mm_network_charset))
					/* fallback to US-ASCII */
					tinrc.mm_network_charset = 0;
		}
		free(ptr);
	}
#endif /* !CHARSET_CONVERSION */

#ifdef TIN_DEFAULTS_DIR
	joinpath(global_attributes_file, sizeof(global_attributes_file), TIN_DEFAULTS_DIR, ATTRIBUTES_FILE);
	joinpath(global_config_file, sizeof(global_config_file), TIN_DEFAULTS_DIR, CONFIG_FILE);
#else
	/* read_site_config() might have changed the value of libdir */
	joinpath(global_attributes_file, sizeof(global_attributes_file), libdir, ATTRIBUTES_FILE);
	joinpath(global_config_file, sizeof(global_config_file), libdir, CONFIG_FILE);
#endif /* TIN_DEFAULTS_DIR */

	joinpath(rcdir, sizeof(rcdir), homedir, RCDIR);
	if (stat(rcdir, &sb) == -1) {
		if (my_mkdir(rcdir, (mode_t) (S_IRWXU)) == -1) {
			my_fprintf(stderr, _(txt_cannot_create), rcdir);
			my_fprintf(stderr, "\n");
			free_all_arrays();
			giveup();
		} else
			created_rcdir = TRUE;
	}
	tinrc.mailer_format = my_strdup(MAILER_FORMAT);
	my_strncpy(mailer, get_val(ENV_VAR_MAILER, DEFAULT_MAILER), sizeof(mailer) - 1);
	tinrc.editor_format = my_strdup(TIN_EDITOR_FMT);
	tinrc.attrib_editor_format = NULL;
#ifndef DISABLE_PRINTING
	tinrc.printer = my_strdup(DEFAULT_PRINTER);
#endif /* !DISABLE_PRINTING */
	tinrc.default_range_group = my_strdup(DEFAULT_RANGE);
	tinrc.default_range_select = my_strdup(DEFAULT_RANGE);
	tinrc.default_range_thread = my_strdup(DEFAULT_RANGE);
	tinrc.default_save_file = my_strdup(DEFAULT_SAVE_FILE);
	tinrc.mail_quote_format = my_strdup(MAIL_QUOTE_FORMAT);
#ifdef HAVE_METAMAIL
	tinrc.metamail_prog = my_strdup(METAMAIL_CMD);
#else
	tinrc.metamail_prog = my_strdup(INTERNAL_CMD);
#endif /* HAVE_METAMAIL */
	tinrc.news_headers_to_display = my_strdup(DEFAULT_NEWS_HEADERS_TO_DISPLAY);
	tinrc.news_quote_format = my_strdup(DEFAULT_NEWS_QUOTE_FORMAT);
	tinrc.quote_chars = my_strdup(DEFAULT_COMMENT);
	tinrc.url_handler = my_strdup(DEFAULT_URL_HANDLER);
	tinrc.posted_articles_file = my_strdup(POSTED_FILE);
	tinrc.xpost_quote_format = my_strdup(DEFAULT_XPOST_QUOTE_FORMAT);
	tinrc.inews_prog = my_strdup(PATH_INEWS);
	tinrc.select_format = my_strdup(DEFAULT_SELECT_FORMAT);
	tinrc.group_format = my_strdup(DEFAULT_GROUP_FORMAT);
	tinrc.thread_format = my_strdup(DEFAULT_THREAD_FORMAT);
	tinrc.attachment_format = my_strdup(DEFAULT_ATTACHMENT_FORMAT);
	tinrc.page_mime_format = my_strdup(DEFAULT_PAGE_MIME_FORMAT);
	tinrc.page_uue_format = my_strdup(DEFAULT_PAGE_UUE_FORMAT);
	tinrc.date_format = my_strdup(DEFAULT_DATE_FORMAT);
	joinpath(article_name, sizeof(article_name), homedir, TIN_ARTICLE_NAME);
#ifdef APPEND_PID
	snprintf(article_name + strlen(article_name), sizeof(article_name) - strlen(article_name), ".%ld", (long) process_id);
#endif /* APPEND_PID */

#ifdef HAVE_LONG_FILE_NAMES
	space = snprintf(NULL, 0, "%s.bak", article_name);
#else
	space = snprintf(NULL, 0, "%s.b", article_name);
#endif /* HAVE_LONG_FILE_NAMES */
	backup_article_name = my_malloc(++space);
#ifdef HAVE_LONG_FILE_NAMES
	snprintf(backup_article_name, space, "%s.bak", article_name);
#else
	snprintf(backup_article_name, space, "%s.b", article_name);
#endif /* HAVE_LONG_FILE_NAMES */

	joinpath(dead_article, sizeof(dead_article), homedir, "dead.article");
	joinpath(dead_articles, sizeof(dead_articles), homedir, "dead.articles");
	joinpath(tmp, sizeof(tmp), homedir, DEFAULT_MAILDIR);
	tinrc.maildir = my_strdup(tmp);
	joinpath(tmp, sizeof(tmp), homedir, DEFAULT_SAVEDIR);
	tinrc.savedir = my_strdup(tmp);
	joinpath(tmp, sizeof(tmp), homedir, DEFAULT_SIGFILE);
	tinrc.sigfile = my_strdup(tmp);
	joinpath(default_signature, sizeof(default_signature), homedir, ".signature");

	if (!index_newsdir[0])
		joinpath(index_newsdir, sizeof(index_newsdir), get_val("TIN_INDEX_NEWSDIR", rcdir), INDEX_NEWSDIR);
	joinpath(index_maildir, sizeof(index_maildir), get_val("TIN_INDEX_MAILDIR", rcdir), INDEX_MAILDIR);
	if (stat(index_maildir, &sb) == -1) {
		if (my_mkdir(index_maildir, (mode_t) S_IRWXU) == -1) {
			my_fprintf(stderr, _(txt_cannot_create), index_maildir);
			my_fprintf(stderr, "\n");
			if (!batch_mode)
				sleep(2);
		}
	}
	joinpath(index_savedir, sizeof(index_savedir), get_val("TIN_INDEX_SAVEDIR", rcdir), INDEX_SAVEDIR);
	if (stat(index_savedir, &sb) == -1) {
		if (my_mkdir(index_savedir, (mode_t) S_IRWXU) == -1) {
			my_fprintf(stderr, _(txt_cannot_create), index_savedir);
			my_fprintf(stderr, "\n");
			if (!batch_mode)
				sleep(2);
		}
	}
	joinpath(local_attributes_file, sizeof(local_attributes_file), rcdir, ATTRIBUTES_FILE);
	joinpath(local_config_file, sizeof(local_config_file), rcdir, CONFIG_FILE);
	joinpath(filter_file, sizeof(filter_file), rcdir, FILTER_FILE);
	joinpath(local_input_history_file, sizeof(local_input_history_file), rcdir, INPUT_HISTORY_FILE);
	joinpath(newsrctable_file, sizeof(newsrctable_file), rcdir, NEWSRCTABLE_FILE);
#ifdef HAVE_MH_MAIL_HANDLING
	joinpath(mail_active_file, sizeof(mail_active_file), rcdir, ACTIVE_MAIL_FILE);
#endif /* HAVE_MH_MAIL_HANDLING */
	if ((p = get_val("MAILPATH", get_val("MAIL", NULL))) != NULL) {
		STRCPY(mailbox, p);
	} else
		joinpath(mailbox, sizeof(mailbox), DEFAULT_MAILBOX, userid);
#ifdef HAVE_MH_MAIL_HANDLING
	joinpath(mailgroups_file, sizeof(mailgroups_file), rcdir, MAILGROUPS_FILE);
#endif /* HAVE_MH_MAIL_HANDLING */
	joinpath(newsrc, sizeof(newsrc), homedir, NEWSRC_FILE);
	joinpath(newnewsrc, sizeof(newnewsrc), homedir, NEWNEWSRC_FILE);
#ifdef APPEND_PID
	snprintf(newnewsrc + strlen(newnewsrc), sizeof(newnewsrc) - strlen(newnewsrc), ".%d", (int) process_id);
#endif /* APPEND_PID */
	joinpath(posted_info_file, sizeof(posted_info_file), rcdir, POSTED_FILE);
	joinpath(postponed_articles_file, sizeof(postponed_articles_file), rcdir, POSTPONED_FILE);
	joinpath(save_active_file, sizeof(save_active_file), rcdir, ACTIVE_SAVE_FILE);

#ifdef HAVE_LONG_FILE_NAMES
	snprintf(tmp, sizeof(tmp), "tin.%.*s.LCK", MIN((int) (sizeof(tmp) - 9), LOGIN_NAME_MAX - 1), userid);
#else
	snprintf(tmp, sizeof(tmp), "%.10s.LCK", userid);
#endif /* HAVE_LONG_FILE_NAMES */
	/*
	 * TODO: rcdir is better than tmpdir, but serverdir (or
	 *       index_newsdir"-"servername) would make most sense as that
	 *       would allow simultaneous updates from different servers,
	 *       unfortunately we don't know the servername yet
	 */
	joinpath(lock_file, sizeof(lock_file), rcdir, tmp);

#ifdef NNTP_ABLE
	nntp_tcp_default_port = (unsigned short) s2i(get_val("NNTPPORT", NNTP_TCP_PORT), 0, 65535);
	if (errno)
		nntp_tcp_default_port = IPPORT_NNTP;
#	ifdef NNTPS_ABLE
	nntps_tcp_default_port = (unsigned short) s2i(get_val("NNTPSPORT", NNTPS_TCP_PORT), 0, 65535);
	if (errno)
		nntps_tcp_default_port = IPPORT_NNTPS;
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */

	if ((fp = fopen(posted_info_file, "a")) != NULL) {
		if (!fstat(fileno(fp), &sb)) {
			if (sb.st_size == 0) {
				fprintf(fp, "%s", _(txt_posted_info_file));
#ifdef HAVE_FCHMOD
				fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));
#else
#	ifdef HAVE_CHMOD
				chmod(posted_info_file, (mode_t) (S_IRUSR|S_IWUSR));
#	endif /* HAVE_CHMOD */
#endif /* HAVE_FCHMOD */
			}
		}
		fclose(fp);
	}

	init_postinfo();
	snprintf(txt_help_bug_report, sizeof(txt_help_bug_report), _(txt_help_bug), bug_addr);

#ifdef HAVE_PGP_GPG
	init_pgp();
#endif /* HAVE_PGP_GPG */
}


/*
 * read_site_config()
 *
 * This function permits the local administrator to override a few compile
 * time defined parameters, especially the concerning the place of a local
 * news spool. This has especially binary distributions of TIN in mind.
 *
 * Sven Paulus <sven@tin.org>, 26-Jan-'98
 */
static int
read_site_config(
	void)
{
	FILE *fp = (FILE *) 0;
	char *buf, filename[PATH_LEN];
	static const char *tin_defaults[] = { TIN_DEFAULTS };
	int i = 0;

	/*
	 * try to find tin.defaults in some different locations
	 */
	*global_defaults_file = '\0';
	while (tin_defaults[i] != NULL) {
		joinpath(filename, sizeof(filename), tin_defaults[i++], "tin.defaults");
		if ((fp = tin_fopen(filename, "r")) != NULL) {
			STRCPY(global_defaults_file, filename);
			break;
		}
	}

	if (!fp)
		return -1;

	/*
	 * no (_(txt_reading_config_file), _(txt_global), global_defaults_file)
	 * here as read_site_config() is called (from init_selfinfo()) before
	 * read_cmd_line_options() so we don't know if we're in (verbose)
	 * batch mode or not ...
	 */
	while ((buf = tin_fgets(fp, FALSE)) != NULL) {
		/* ignore comments */
		if (*buf == '#' || *buf == ';' || *buf == ' ')
			continue;
#ifndef NNTP_ONLY
		if (match_string(buf, "spooldir=", spooldir, sizeof(spooldir)))
			continue;
		if (match_string(buf, "overviewdir=", novrootdir, sizeof(novrootdir)))
			continue;
		if (match_string(buf, "overviewfile=", novfilename, sizeof(novfilename)))
			continue;
		if (match_string(buf, "activetimesfile=", active_times_file, sizeof(active_times_file)))
			continue;
		if (match_string(buf, "overviewfmtfile=", overviewfmt_file, sizeof(overviewfmt_file)))
			continue;
		if (match_string(buf, "subscriptionsfile=", subscriptions_file, sizeof(subscriptions_file)))
			continue;
#endif /* !NNTP_ONLY */
		if (match_string(buf, "activefile=", news_active_file, sizeof(news_active_file)))
			continue;
		if (match_string(buf, "newsgroupsfile=", newsgroups_file, sizeof(newsgroups_file)))
			continue;
		if (match_string(buf, "newslibdir=", libdir, sizeof(libdir)))
			continue;
		if (match_string(buf, "domainname=", domain_name, sizeof(domain_name)))
			continue;
		if (match_string(buf, "inewsdir=", inewsdir, sizeof(inewsdir)))
			continue;
		if (match_string(buf, "bugaddress=", bug_addr, sizeof(bug_addr)))
			continue;
		if (match_string_ptr(buf, "organization=", &default_organization))
			continue;
#ifndef CHARSET_CONVERSION
		if (match_string_ptr(buf, "mm_charset=", &tinrc.mm_charset))
			continue;
#else
		if (match_list(buf, "mm_charset=", txt_mime_charsets, &tinrc.mm_network_charset))
			continue;
#endif /* !CHARSET_CONVERSION */
		if (match_list(buf, "post_mime_encoding=", txt_mime_encodings, &tinrc.post_mime_encoding))
			continue;
		if (match_list(buf, "mail_mime_encoding=", txt_mime_encodings, &tinrc.mail_mime_encoding))
			continue;
		if (match_boolean(buf, "disable_gnksa_domain_check=", &disable_gnksa_domain_check))
			continue;
		if (match_boolean(buf, "disable_sender=", &disable_sender))
			continue;
	}

	fclose(fp);
	return 0;
}


/*
 * set defaults if needed to avoid empty regexp
 */
void
postinit_regexp(
	void)
{
	if (!tinrc.strip_re_regex || !*tinrc.strip_re_regex) {
		FreeIfNeeded(tinrc.strip_re_regex);
		tinrc.strip_re_regex = my_strdup(DEFAULT_STRIP_RE_REGEX);
	}
	compile_regex(tinrc.strip_re_regex, &strip_re_regex, REGEX_ANCHORED);

	if (tinrc.strip_was_regex && *tinrc.strip_was_regex) {
		/*
		 * try to be clever, if we still use the initial default value
		 * convert it to our needs
		 *
		 * TODO: a global solution
		 */
		if (regex_use_utf8()) {
			if (!strcmp(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX)) {
				free(tinrc.strip_was_regex);
				tinrc.strip_was_regex = my_strdup(DEFAULT_U8_STRIP_WAS_REGEX);
			}
		} else {
			if (!strcmp(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX)) {
				free(tinrc.strip_was_regex);
				tinrc.strip_was_regex = my_strdup(DEFAULT_STRIP_WAS_REGEX);
			}
		}
	} else {
		if (regex_use_utf8()) {
			FreeIfNeeded(tinrc.strip_was_regex);
			tinrc.strip_was_regex = my_strdup(DEFAULT_U8_STRIP_WAS_REGEX);
		} else {
			FreeIfNeeded(tinrc.strip_was_regex);
			tinrc.strip_was_regex = my_strdup(DEFAULT_STRIP_WAS_REGEX);
		}
	}
	compile_regex(tinrc.strip_was_regex, &strip_was_regex, 0);

#ifdef HAVE_COLOR
	if (!tinrc.extquote_regex || !*tinrc.extquote_regex) {
		FreeIfNeeded(tinrc.extquote_regex);
		tinrc.extquote_regex = my_strdup(DEFAULT_EXTQUOTE_REGEX);
	}
	compile_regex(tinrc.extquote_regex, &extquote_regex, REGEX_CASELESS);
	if (!tinrc.quote_regex || !*tinrc.quote_regex) {
		FreeIfNeeded(tinrc.quote_regex);
		tinrc.quote_regex = my_strdup(DEFAULT_QUOTE_REGEX);
	}
	compile_regex(tinrc.quote_regex, &quote_regex, REGEX_CASELESS);
	if (!tinrc.quote_regex2 || !*tinrc.quote_regex2) {
		FreeIfNeeded(tinrc.quote_regex2);
		tinrc.quote_regex2 = my_strdup(DEFAULT_QUOTE_REGEX2);
	}
	compile_regex(tinrc.quote_regex2, &quote_regex2, REGEX_CASELESS);
	if (!tinrc.quote_regex3 || !*tinrc.quote_regex3) {
		FreeIfNeeded(tinrc.quote_regex3);
		tinrc.quote_regex3 = my_strdup(DEFAULT_QUOTE_REGEX3);
	}
	compile_regex(tinrc.quote_regex3, &quote_regex3, REGEX_CASELESS);
#endif /* HAVE_COLOR */

	if (!tinrc.slashes_regex || !*tinrc.slashes_regex) {
		FreeIfNeeded(tinrc.slashes_regex);
		tinrc.slashes_regex = my_strdup(DEFAULT_SLASHES_REGEX);
	}
	compile_regex(tinrc.slashes_regex, &slashes_regex, REGEX_CASELESS);
	if (!tinrc.stars_regex || !*tinrc.stars_regex) {
		FreeIfNeeded(tinrc.stars_regex);
		tinrc.stars_regex = my_strdup(DEFAULT_STARS_REGEX);
	}
	compile_regex(tinrc.stars_regex, &stars_regex, REGEX_CASELESS);
	if (!tinrc.strokes_regex || !*tinrc.strokes_regex) {
		FreeIfNeeded(tinrc.strokes_regex);
		tinrc.strokes_regex = my_strdup(DEFAULT_STROKES_REGEX);
	}
	compile_regex(tinrc.strokes_regex, &strokes_regex, REGEX_CASELESS);
	if (!tinrc.underscores_regex || !*tinrc.underscores_regex) {
		FreeIfNeeded(tinrc.underscores_regex);
		tinrc.underscores_regex = my_strdup(DEFAULT_UNDERSCORES_REGEX);
	}
	compile_regex(tinrc.underscores_regex, &underscores_regex, REGEX_CASELESS);

	if (!tinrc.verbatim_begin_regex || !*tinrc.verbatim_begin_regex) {
		FreeIfNeeded(tinrc.verbatim_begin_regex);
		tinrc.verbatim_begin_regex = my_strdup(DEFAULT_VERBATIM_BEGIN_REGEX);
	}
	compile_regex(tinrc.verbatim_begin_regex, &verbatim_begin_regex, REGEX_ANCHORED);
	if (!tinrc.verbatim_end_regex || !*tinrc.verbatim_end_regex) {
		FreeIfNeeded(tinrc.verbatim_end_regex);
		tinrc.verbatim_end_regex = my_strdup(DEFAULT_VERBATIM_END_REGEX);
	}
	compile_regex(tinrc.verbatim_end_regex, &verbatim_end_regex, REGEX_ANCHORED);

	compile_regex(UUBEGIN_REGEX, &uubegin_regex, REGEX_ANCHORED);
	compile_regex(UUBODY_REGEX, &uubody_regex, REGEX_ANCHORED);

	compile_regex(URL_REGEX, &url_regex, REGEX_CASELESS);
	compile_regex(MAIL_REGEX, &mail_regex, REGEX_CASELESS);
	compile_regex(NEWS_REGEX, &news_regex, REGEX_CASELESS);

	compile_regex(SHAR_REGEX, &shar_regex, REGEX_ANCHORED);
}
