/*
 *  Project   : tin - a Usenet reader
 *  Module    : init.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2005-08-14
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
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	static t_bool utf8_pcre(void);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#ifdef HAVE_COLOR
	static void preinit_colors(void);
#endif /* HAVE_COLOR */


char **news_headers_to_display_array;	/* array of which headers to display */
char **news_headers_to_not_display_array;	/* array of which headers to not display */
char active_times_file[PATH_LEN];
char article_name[PATH_LEN];			/* ~/TIN_ARTICLE_NAME file */
char bug_nntpserver1[PATH_LEN];		/* welcome message of NNTP server used */
char bug_nntpserver2[PATH_LEN];		/* welcome message of NNTP server used */
char cmdline_nntpserver[PATH_LEN];
char cvers[LEN];
char dead_article[PATH_LEN];		/* ~/dead.article file */
char dead_articles[PATH_LEN];		/* ~/dead.articles file */
char default_organization[PATH_LEN];	/* Organization: */
char default_signature[PATH_LEN];
char domain_name[MAXHOSTNAMELEN];
char global_attributes_file[PATH_LEN];
char global_config_file[PATH_LEN];
char homedir[PATH_LEN];
char index_maildir[PATH_LEN];
char index_newsdir[PATH_LEN];	/* directory for private overview data */
char index_savedir[PATH_LEN];
char inewsdir[PATH_LEN];
char libdir[PATH_LEN];			/* directory where news config files are (ie. active) */
char local_attributes_file[PATH_LEN];
char local_config_file[PATH_LEN];
char local_input_history_file[PATH_LEN];
char local_newsgroups_file[PATH_LEN];	/* local copy of NNTP newsgroups file */
char local_newsrctable_file[PATH_LEN];
char lock_file[PATH_LEN];		/* contains name of index lock file */
char filter_file[PATH_LEN];
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
char subscriptions_file[PATH_LEN];
char tin_progname[PATH_LEN];		/* program name */
char txt_help_bug_report[LEN];		/* address to send bug reports to */
char userid[PATH_LEN];
#ifdef HAVE_MH_MAIL_HANDLING
	char mail_active_file[PATH_LEN];
	char mailgroups_file[PATH_LEN];
#endif /* HAVE_MH_MAIL_HANDLING */
#ifndef NNTP_ONLY
	char novfilename[PATH_LEN];		/* file name of a single nov index file */
	char novrootdir[PATH_LEN];		/* root directory of nov index files */
#endif /* !NNTP_ONLY */

#ifdef VMS
	char rcdir_asfile[PATH_LEN];	/* rcdir expressed as dev:[dir]tin.dir, for stat() */
#endif /* VMS */

t_function last_search;	/* for repeated search */
int hist_last[HIST_MAXNUM + 1];
int hist_pos[HIST_MAXNUM + 1];
int iso2asc_supported;			/* Convert ISO-Latin1 to Ascii */
int num_headers_to_display;		/* num headers to display -- swp */
int num_headers_to_not_display;		/* num headers to not display -- swp */
int system_status;
int xmouse, xrow, xcol;			/* xterm button pressing information */

pid_t process_id;			/* Useful to have around for .suffixes */

t_bool batch_mode;			/* update index files only mode */
t_bool check_for_new_newsgroups;	/* don't check for new newsgroups */
t_bool cmd_line;			/* batch / interactive mode */
t_bool created_rcdir;			/* checks if first time tin is started */
t_bool dangerous_signal_exit;		/* no get_respcode() in nntp_command when dangerous signal exit */
t_bool disable_gnksa_domain_check;	/* disable checking TLD in From: etc. */
t_bool disable_sender;			/* disable generation of Sender: header */
t_bool filtered_articles;		/* locally killed / auto-selected articles */
#ifdef NO_POSTING
	t_bool force_no_post = TRUE;		/* force no posting mode */
#else
	t_bool force_no_post = FALSE;	/* don't force no posting mode */
#endif /* NO_POSTING */
t_bool got_sig_pipe = FALSE;
t_bool list_active;
t_bool newsrc_active;
t_bool no_write = FALSE;		/* do not write newsrc on quit (-X cmd-line flag) */
t_bool post_article_and_exit;		/* quick post of an article then exit(elm like) */
t_bool post_postponed_and_exit;		/* post postponed articles and exit */
t_bool reread_active_for_posted_arts;
t_bool read_local_newsgroups_file;	/* read newsgroups file locally or via NNTP */
t_bool read_news_via_nntp = FALSE;	/* read news locally or via NNTP */
t_bool read_saved_news = FALSE;		/* tin -R read saved news from tin -S */
t_bool show_description = TRUE;		/* current copy of tinrc flag */
t_bool verbose = FALSE;			/* update index files only mode */
t_bool word_highlight;		/* word highlighting on/off */
t_bool xref_supported = TRUE;
#ifdef HAVE_COLOR
	t_bool use_color;		/* enables/disables ansi-color support under linux-console and color-xterm */
#endif /* HAVE_COLOR */
#ifdef NNTP_ABLE
	t_bool force_auth_on_conn_open = FALSE;	/* authenticate on connection startup */
	unsigned short nntp_tcp_port;
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
		url_regex, mail_regex, news_regex,
		shar_regex,
		slashes_regex, stars_regex, underscores_regex, strokes_regex
#ifdef HAVE_COLOR
		, quote_regex, quote_regex2, quote_regex3
#endif /* HAVE_COLOR */
	= {
		NULL,
		NULL
	};

struct t_config tinrc = {
	ART_MARK_DELETED,		/* art_marked_deleted */
	MARK_INRANGE,			/* art_marked_inrange */
	ART_MARK_RETURN,		/* art_marked_return */
	ART_MARK_SELECTED,		/* art_marked_selected */
	ART_MARK_RECENT,		/* art_marked_recent */
	ART_MARK_UNREAD,		/* art_marked_unread */
	ART_MARK_READ,			/* art_marked_read */
	ART_MARK_KILLED,		/* art_marked_killed */
	ART_MARK_READ_SELECTED,		/* art_marked_read_selected */
	"",		/* editor_format */
	"",		/* default_goto_group */
	"",		/* default_mail_address */
	"",		/* mailer_format */
#ifndef DONT_HAVE_PIPING
	"",		/* default_pipe_command */
#endif /* !DONT_HAVE_PIPING */
	"",		/* default_post_newsgroups */
	"",		/* default_post_subject */
#ifndef DISABLE_PRINTING
	"",		/* printer */
#endif /* !DISABLE_PRINTING */
	"1-.",	/* default_range_group */
	"1-.",	/* default_range_select */
	"1-.",	/* default_range_thread */
	"",		/* default_pattern */
	"",		/* default_repost_group */
	"savefile.tin",		/* default_save_file */
	"",		/* default_search_art */
	"",		/* default_search_author */
	"",		/* default_search_config */
	"",		/* default_search_group */
	"",		/* default_search_subject */
	"",		/* default_select_pattern */
	"",		/* default_shell_command */
	"In article %M you wrote:",		/* mail_quote_format */
	"",		/* maildir */
#ifdef SCO_UNIX
	2,			/* mailbox_format = MMDF */
#else
	0,			/* mailbox_format = MBOXO */
#endif /* SCO_UNIX */
	"",		/* mail_address */
#ifdef HAVE_METAMAIL
	METAMAIL_CMD,		/* metamail_prog */
#else
	INTERNAL_CMD,		/* metamail_prog */
#endif /* HAVE_METAMAIL */
#ifndef CHARSET_CONVERSION
	"",		/* mm_charset, defaults to $MM_CHARSET */
#else
	-1,		/* mm_network_charset, defaults to $MM_CHARSET */
#endif /* !CHARSET_CONVERSION */
	"",		/* mm_local_charset, display charset */
#ifdef HAVE_ICONV_OPEN_TRANSLIT
	FALSE,	/* translit */
#endif /* HAVE_ICONV_OPEN_TRANSLIT */
	"Newsgroups Followup-To Summary Keywords X-Comment-To",		/* news_headers_to_display */
	"",		/* news_headers_to_not_display */
	"%F wrote:",		/* news_quote_format */
	DEFAULT_COMMENT,	/* quote_chars */
#ifdef HAVE_COLOR
	"",		/* quote_regex */
	"",		/* quote_regex 2nd level */
	"",		/* quote_regex >= 3rd level */
#endif /* HAVE_COLOR */
	"",		/* slashes_regex */
	"",		/* stars_regex */
	"",		/* underscores_regex */
	"",		/* strokes_regex */
	"",		/* sigfile */
	"",		/* strip_re_regex */
	"",		/* strip_was_regex */
	"",		/* savedir */
	"",		/* spamtrap_warning_addresses */
	DEFAULT_URL_HANDLER,	/* url_handler */
	"In %G %F wrote:",			/* xpost_quote_format */
	DEFAULT_FILTER_DAYS,			/* filter_days */
	FILTER_SUBJ_CASE_SENSITIVE,		/* default_filter_kill_header */
	FILTER_SUBJ_CASE_SENSITIVE,		/* default_filter_select_header */
	0,		/* default_move_group */
	'a',		/* default_save_mode */
	0,		/* getart_limit */
	2,		/* recent_time */
	32,		/* groupname_max_length */
	UUE_NO,	/* hide_uue */
	KILL_UNREAD,		/* kill_level */
	MIME_ENCODING_7BIT,		/* mail_mime_encoding */
	MIME_ENCODING_7BIT,		/* post_mime_encoding */
	POST_PROC_NO,			/* post_process */
	REREAD_ACTIVE_FILE_SECS,	/* reread_active_file_secs */
	1,		/* scroll_lines */
	SHOW_FROM_NAME,				/* show_author */
	SORT_ARTICLES_BY_DATE_ASCEND,		/* sort_article_type */
	SORT_THREADS_BY_SCORE_DESCEND,		/* sort_threads_type */
	BOGUS_SHOW,		/* strip_bogus */
	THREAD_BOTH,		/* thread_articles */
	THREAD_PERC_DEFAULT,	/* thread_perc */
	THREAD_SCORE_MAX,	/* thread_score */
	0,		/* Default to wildmat, not regex */
	-50,		/* score_limit_kill */
	50,		/* score_limit_select */
	-100,		/* score_kill */
	100,		/* score_select */
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
	0,		/* col_response (initialised later) */
	0,		/* col_signature (initialised later) */
	0,		/* col_urls (initialised later) */
	0,		/* col_subject (initialised later) */
	0,		/* col_text (initialised later) */
	0,		/* col_title (initialised later) */
#endif /* HAVE_COLOR */
	2,		/* word_h_display_marks */
	2,		/* mono_markdash */
	6,		/* mono_markstar */
	5,		/* mono_markslash */
	3,		/* mono_markstroke */
	TRUE,		/* word_highlight */
	TRUE,		/* url_highlight */
	0,		/* wrap_column */
#ifdef HAVE_COLOR
	FALSE,		/* use_color */
#endif /* HAVE_COLOR */
	TRUE,		/* add_posted_to_filter */
	TRUE,		/* advertising */
	TRUE,		/* alternative_handling */
	FALSE,		/* auto_bcc */
	FALSE,		/* auto_cc */
	TRUE,		/* auto_list_thread */
	FALSE,		/* auto_reconnect */
	FALSE,		/* auto_save */
	TRUE,		/* batch_save */
	TRUE,		/* beginner_level */
	FALSE,		/* cache_overview_files */
	FALSE,		/* catchup_read_groups */
	4,		/* confirm_choice */
#ifdef USE_INVERSE_HACK
	TRUE,		/* draw_arrow */
#else
	FALSE,		/* draw_arrow */
#endif /* USE_INVERSE_HACK */
	FALSE,		/* force_screen_redraw */
	TRUE,		/* group_catchup_on_exit */
	FALSE,		/* info_in_last_line */
#ifdef USE_INVERSE_HACK
	FALSE,		/* inverse_okay */
#else
	TRUE,		/* inverse_okay */
#endif /* USE_INVERSE_HACK */
	TRUE,		/* keep_dead_articles */
	POSTED_FILE,	/* posted_articles_file */
	FALSE,		/* mail_8bit_header */
	FALSE,		/* mark_ignore_tags */
	TRUE,		/* mark_saved_read */
	TRUE,		/* pgdn_goto_next */
	TRUE,		/* pos_first_unread */
	FALSE,		/* post_8bit_header */
	TRUE,		/* post_process_view */
#ifndef DISABLE_PRINTING
	FALSE,		/* print_header */
#endif /* !DISABLE_PRINTING */
	FALSE,		/* process_only_unread */
	FALSE,		/* prompt_followupto */
	QUOTE_COMPRESS|QUOTE_EMPTY,	/* quote_style */
	TRUE,		/* show_description */
	SHOW_INFO_LINES,		/* show_info */
	TRUE,		/* show_only_unread_arts */
	FALSE,		/* show_only_unread_groups */
	TRUE,		/* show_signatures */
	TRUE,		/* sigdashes */
	TRUE,		/* signature_repost */
	FALSE,		/* space_goto_next_unread */
#ifdef M_UNIX
	TRUE,		/* start_editor_offset */
#else
	FALSE,		/* start_editor_offset */
#endif /* M_UNIX */
	TRUE,		/* strip_blanks */
	FALSE,		/* strip_newsrc */
	TRUE,		/* tab_goto_next_unread */
	FALSE,		/* tex2iso_conv */
	TRUE,		/* thread_catchup_on_exit */
	TRUE,		/* unlink_article */
	"",		/* inews_prog */
	INTERACTIVE_NONE,		/* interactive_mailer */
	FALSE,		/* use_mouse */
#ifdef HAVE_KEYPAD
	FALSE,		/* use_keypad */
#endif /* HAVE_KEYPAD */
	TRUE,		/* wrap_on_next_unread */
	FALSE,		/* ask_for_metamail */
	FALSE,		/* default_filter_kill_case */
	FALSE,		/* default_filter_kill_expire */
	TRUE,		/* default_filter_kill_global */
	FALSE,		/* default_filter_select_case */
	FALSE,		/* default_filter_select_expire */
#ifdef XFACE_ABLE
	FALSE,		/* use_slrnface */
#endif /* XFACE_ABLE */
	TRUE,		/* default_filter_select_global */
	DEFAULT_DATE_FORMAT,	/* date_format */
#ifdef HAVE_UNICODE_NORMALIZATION
	NORMALIZE_NFKC,		/* normalization form */
#endif /* HAVE_UNICODE_NORMALIZATION */
#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
	FALSE		/* render_bidi */
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
};

struct t_capabilities nntp_caps = {
	0, /* type (none, LIST EXTENSIONS, CAPABILITIES) */
	0, /* CAPABILITIES version */
	FALSE, /* MODE-READER: "MODE READER" */
	FALSE, /* READER: "ARTICLE", "BODY" */
	FALSE, /* POST */
	FALSE, /* LIST: "LIST ACTIVE" */
	FALSE, /* LIST: "LIST ACTIVE.TIMES" */
	FALSE, /* LIST: "LIST DISTRIB.PATS" */
	FALSE, /* LIST: "LIST HEADERS" */
	FALSE, /* LIST: "LIST NEWSGROUPS" */
	FALSE, /* LIST: "LIST OVERVIEW.FMT" */
	FALSE, /* LIST: "LIST MOTD" */
	FALSE, /* LIST: "LIST SUBSCRIPTIONS" */
	FALSE, /* LIST: "LIST DISTRIBUTIONS" */
	FALSE, /* LIST: "LIST MODERATORS" */
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
#if 0
	FALSE, /* SASL CRAM-MD5 */
	FALSE, /* SASL DIGEST-MD5 */
	FALSE, /* SASL PLAIN */
	FALSE, /* SASL GSSAPI */
	FALSE, /* SASL EXTERNAL */
	FALSE, /* STREAMING: "MODE STREAM", "CHECK", "TAKETHIS" */
	FALSE /* IHAVE */
#endif /* 0 */
};

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
	{ &tinrc.col_response,    2 },
	{ &tinrc.col_signature,   4 },
	{ &tinrc.col_urls,       -1 },
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
	void)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(our_colors); n++) {
		if (*(our_colors[n].colorp) == DFT_INIT) {
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
	char *ptr;
	const char *cptr;
	struct stat sb;
	struct passwd *myentry;

	domain_name[0] = '\0';

#ifdef HAVE_SYS_UTSNAME_H
#	ifdef HAVE_UNAME
	if (uname(&system_info) != -1)
		;
	else
#	endif /* HAVE_UNAME */
	{
		strcpy(system_info.sysname, "unknown");
		*system_info.machine = '\0';
		*system_info.release = '\0';
		*system_info.nodename = '\0';
	}
#endif /* HAVE_SYS_UTSNAME_H */

#ifdef DOMAIN_NAME
	if ((cptr = get_domain_name()) != NULL)
		strcpy(domain_name, cptr);
#endif /* DOMAIN_NAME */

#ifdef HAVE_GETHOSTBYNAME
	if (domain_name[0] == '\0') {
		cptr = get_fqdn(get_host_name());
		if (cptr != NULL)
			strcpy(domain_name, cptr);
	}
#endif /* HAVE_GETHOSTBYNAME */

	process_id = getpid();

	real_umask = umask(0);
	(void) umask(real_umask);

#ifndef VMS
	if ((myentry = getpwuid(getuid())) == NULL) {
		error_message(_(txt_error_passwd_missing));
		giveup();
	}
#else
	if (((ptr = getlogin()) != NULL) && strlen(ptr))
		myentry = getpwnam(ptr);
	else {
		error_message(_(txt_error_passwd_missing));
		giveup();
	}
#endif /* !VMS */

	strcpy(userid, myentry->pw_name);

#ifdef VMS
	lower(userid);
#endif /* VMS */

	if (((ptr = getenv("TIN_HOMEDIR")) != NULL) && strlen(ptr)) {
		my_strncpy(homedir, ptr, sizeof(homedir) - 1);
	} else if (((ptr = getenv("HOME")) != NULL) && strlen(ptr)) {
		my_strncpy(homedir, ptr, sizeof(homedir) - 1);
	} else if (strlen(myentry->pw_dir)) {
		strncpy(homedir, myentry->pw_dir, sizeof(homedir) - 1);
	} else
		strncpy(homedir, TMPDIR, sizeof(homedir) - 1);

	cmdline_nntpserver[0] = '\0';
	created_rcdir = FALSE;
	dangerous_signal_exit = FALSE;
	disable_gnksa_domain_check = FALSE;
#ifdef MAC_OS_X	/* usualy they don't have a valid FQDN */
	/*
	 * TODO: check if gnksa_check_domain(get_fqdn()) returns ok
	 *       and if it does, don't disable Sender
	 */
	disable_sender = TRUE;
#else
	disable_sender = FALSE;
#endif /* MAC_OS_X */
	filtered_articles = FALSE;
	iso2asc_supported = atoi(get_val("ISO2ASC", DEFAULT_ISO2ASC));
	if (iso2asc_supported > NUM_ISO_TABLES || iso2asc_supported < 0) /* TODO: issue a warning here? */
		iso2asc_supported = -1;
	list_active = FALSE;
	newsrc_active = FALSE;
	num_headers_to_display = 0;
	num_headers_to_not_display = 0;
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

	snprintf(page_header, sizeof(page_header), "%s %s release %s (\"%s\") [%s%s]",
		PRODUCT, VERSION, RELEASEDATE, RELEASENAME, OSNAME,
		(iso2asc_supported >= 0 ? " ISO2ASC" : ""));
	snprintf(cvers, sizeof(cvers), txt_copyright_notice, page_header);

	default_organization[0] = '\0';
	news_headers_to_display_array = ulBuildArgv(tinrc.news_headers_to_display, &num_headers_to_display);
	news_headers_to_not_display_array = NULL;

	strncpy(bug_addr, BUG_REPORT_ADDRESS, sizeof(bug_addr) - 1);

	bug_nntpserver1[0] = '\0';
	bug_nntpserver2[0] = '\0';

#ifdef INEWSDIR
	strncpy(inewsdir, INEWSDIR, sizeof(inewsdir) - 1);
#else
	inewsdir[0] = '\0';
#endif /* INEWSDIR */

#ifdef apollo
	my_strncpy(default_organization, get_val("NEWSORG", ""), sizeof(default_organization) - 1);
#else
	my_strncpy(default_organization, get_val("ORGANIZATION", ""), sizeof(default_organization) - 1);
#endif /* apollo */

#ifdef USE_INN_NNTPLIB
	if ((ptr = GetConfigValue(_CONF_ORGANIZATION)) != NULL)
		my_strncpy(default_organization, ptr, sizeof(default_organization) - 1);
#endif /* USE_INN_NNTPLIB */

#ifndef NNTP_ONLY
	my_strncpy(libdir, get_val("TIN_LIBDIR", NEWSLIBDIR), sizeof(libdir) - 1);
	my_strncpy(novrootdir, get_val("TIN_NOVROOTDIR", NOVROOTDIR), sizeof(novrootdir) - 1);
	my_strncpy(novfilename, get_val("TIN_NOVFILENAME", OVERVIEW_FILE), sizeof(novfilename) - 1);
	my_strncpy(spooldir, get_val("TIN_SPOOLDIR", SPOOLDIR), sizeof(spooldir) - 1);
#endif /* !NNTP_ONLY */
	/* clear news_active_file, active_time_file, newsgroups_file */
	news_active_file[0] = '\0';
	active_times_file[0] = '\0';
	newsgroups_file[0] = '\0';
	subscriptions_file[0] = '\0';

	/*
	 * read the global site config file to override some default
	 * values given at compile time
	 */
	(void) read_site_config();

	/*
	 * the site_config-file was the last chance to set the domainname
	 * if it's still unset fall into no posting mode.
	 */
	if (domain_name[0] == '\0') {
		error_message(_(txt_error_no_domain_name));
		sleep(2);
		force_no_post = TRUE;
	}

	/*
	 * only set the following variables if they weren't set from within
	 * read_site_config()
	 *
	 * TODO: do we really want that read_site_config() overwrites
	 * values given in env-vars? ($MM_CHARSET, $TIN_ACTIVEFILE)
	 */
	if (!*news_active_file) /* TODO: really prepend libdir here in case of $TIN_ACTIVEFILE is set? */
		joinpath(news_active_file, libdir, get_val("TIN_ACTIVEFILE", ACTIVE_FILE));
	if (!*active_times_file)
		joinpath(active_times_file, libdir, ACTIVE_TIMES_FILE);
	if (!*newsgroups_file)
		joinpath(newsgroups_file, libdir, NEWSGROUPS_FILE);
	if (!*subscriptions_file)
		joinpath(subscriptions_file, libdir, SUBSCRIPTIONS_FILE);
	if (!*default_organization) {
		char buf[LEN];

		joinpath(buf, libdir, "organization");
		if ((fp = fopen(buf, "r")) != NULL) {
			if (fgets(buf, (int) sizeof(buf), fp) != NULL) {
				ptr = strrchr(buf, '\n');
				if (ptr != NULL)
					*ptr = '\0';
			}
			fclose(fp);
			my_strncpy(default_organization, buf, sizeof(default_organization) - 1);
		}
	}

	/*
	 * Formerly get_mm_charset(), read_site_config() may set mm_charset
	 */
#ifndef CHARSET_CONVERSION
	if (!*tinrc.mm_charset)
		STRCPY(tinrc.mm_charset, get_val("MM_CHARSET", MM_CHARSET));
#else
	if (tinrc.mm_network_charset < 0) {
		size_t space = 255;

		ptr = my_malloc(space + 1);
		strcpy(ptr, "mm_network_charset=");
		space -= strlen(ptr);
		strncat(ptr, get_val("MM_CHARSET", MM_CHARSET), space);
		if ((space -= strlen(ptr)) > 0) {
			strncat(ptr, "\n", space);
			match_list(ptr, "mm_network_charset=", txt_mime_charsets, NUM_MIME_CHARSETS, &tinrc.mm_network_charset);
		}
		free(ptr);
	}
#endif /* !CHARSET_CONVERSION */

#ifdef TIN_DEFAULTS_DIR
	joinpath(global_attributes_file, TIN_DEFAULTS_DIR, ATTRIBUTES_FILE);
	joinpath(global_config_file, TIN_DEFAULTS_DIR, CONFIG_FILE);
#else
	/* read_site_config() might have changed the value of libdir */
	joinpath(global_attributes_file, libdir, ATTRIBUTES_FILE);
	joinpath(global_config_file, libdir, CONFIG_FILE);
#endif /* TIN_DEFAULTS_DIR */

#ifdef VMS
	joindir(rcdir, homedir, RCDIR); /* we're naming a directory here */
	joinpath(rcdir_asfile, homedir, RCDIR);	/* for stat() */
	strcat(rcdir_asfile, ".DIR");
	if (stat(rcdir_asfile, &sb) == -1)
#else
	joinpath(rcdir, homedir, RCDIR);
	if (stat(rcdir, &sb) == -1)
#endif /* VMS */
	{
		created_rcdir = TRUE;
		my_mkdir(rcdir, (mode_t) (S_IRWXU));
	}
	strcpy(tinrc.mailer_format, MAILER_FORMAT);
	my_strncpy(mailer, get_val(ENV_VAR_MAILER, DEFAULT_MAILER), sizeof(mailer) - 1);
#ifndef DISABLE_PRINTING
	strcpy(tinrc.printer, DEFAULT_PRINTER);
#endif /* !DISABLE_PRINTING */
	strcpy(tinrc.inews_prog, PATH_INEWS);
	joinpath(article_name, homedir, TIN_ARTICLE_NAME);
#ifdef APPEND_PID
	snprintf(article_name + strlen(article_name), sizeof(article_name) - strlen(article_name), ".%d", (int) process_id);
#endif /* APPEND_PID */
	joinpath(dead_article, homedir, "dead.article");
	joinpath(dead_articles, homedir, "dead.articles");
	JOINPATH(tinrc.maildir, homedir, DEFAULT_MAILDIR);
	JOINPATH(tinrc.savedir, homedir, DEFAULT_SAVEDIR);
	joinpath(tinrc.sigfile, homedir, ".Sig");
	joinpath(default_signature, homedir, ".signature");

	if (!index_newsdir[0])
		JOINPATH(index_newsdir, get_val("TIN_INDEX_NEWSDIR", rcdir), INDEX_NEWSDIR);
	JOINPATH(index_maildir, get_val("TIN_INDEX_MAILDIR", rcdir), INDEX_MAILDIR);
	if (stat(index_maildir, &sb) == -1)
		my_mkdir(index_maildir, (mode_t) S_IRWXU);
	JOINPATH(index_savedir, get_val("TIN_INDEX_SAVEDIR", rcdir), INDEX_SAVEDIR);
	if (stat(index_savedir, &sb) == -1)
		my_mkdir(index_savedir, (mode_t) S_IRWXU);
	joinpath(local_attributes_file, rcdir, ATTRIBUTES_FILE);
	joinpath(local_config_file, rcdir, CONFIG_FILE);
	joinpath(filter_file, rcdir, FILTER_FILE);
	joinpath(local_input_history_file, rcdir, INPUT_HISTORY_FILE);
	joinpath(local_newsrctable_file, rcdir, NEWSRCTABLE_FILE);
#ifdef HAVE_MH_MAIL_HANDLING
	joinpath(mail_active_file, rcdir, ACTIVE_MAIL_FILE);
#endif /* HAVE_MH_MAIL_HANDLING */
#ifdef VMS
	joinpath(mailbox, DEFAULT_MAILBOX, "MAIL.TXT");
#else
	joinpath(mailbox, DEFAULT_MAILBOX, userid);
#endif /* VMS */
#ifdef HAVE_MH_MAIL_HANDLING
	joinpath(mailgroups_file, rcdir, MAILGROUPS_FILE);
#endif /* HAVE_MH_MAIL_HANDLING */
	joinpath(newsrc, homedir, NEWSRC_FILE);
	joinpath(newnewsrc, homedir, NEWNEWSRC_FILE);
#ifdef APPEND_PID
	snprintf(newnewsrc + strlen(newnewsrc), sizeof(newnewsrc) - strlen(newnewsrc), ".%d", (int) process_id);
#endif /* APPEND_PID */
	joinpath(posted_info_file, rcdir, POSTED_FILE);
	joinpath(postponed_articles_file, rcdir, POSTPONED_FILE);
	joinpath(save_active_file, rcdir, ACTIVE_SAVE_FILE);

	snprintf(lock_file, sizeof(lock_file), INDEX_LOCK, TMPDIR, userid);

#ifdef NNTP_ABLE
	nntp_tcp_port = (unsigned short) atoi(get_val("NNTPPORT", NNTP_TCP_PORT));
#endif /* NNTP_ABLE */

	if (stat(posted_info_file, &sb) == -1) {
		if ((fp = fopen(posted_info_file, "w")) != NULL) {
			fprintf(fp, _(txt_posted_info_file));
			fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));
			fclose(fp);
		}
	}

	if (stat(local_attributes_file, &sb) == -1)
		write_attributes_file(local_attributes_file);

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
	char buf[LEN];
	static const char *tin_defaults[] = { TIN_DEFAULTS };
	int i = 0;

	/*
	 * try to find tin.defaults in some different locations
	 */
	while (tin_defaults[i] != NULL) {
		joinpath(buf, tin_defaults[i++], "tin.defaults");
		if ((fp = fopen(buf, "r")) != NULL)
			break;
	}

	if (!fp)
		return -1;

	while (fgets(buf, (int) sizeof(buf), fp)) {
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
#endif /* !NNTP_ONLY */
		if (match_string(buf, "activefile=", news_active_file, sizeof(news_active_file)))
			continue;
		if (match_string(buf, "activetimesfile=", active_times_file, sizeof(active_times_file)))
			continue;
		if (match_string(buf, "newsgroupsfile=", newsgroups_file, sizeof(newsgroups_file)))
			continue;
		if (match_string(buf, "newslibdir=", libdir, sizeof(libdir)))
			continue;
		if (match_string(buf, "subscriptionsfile=", subscriptions_file, sizeof(subscriptions_file)))
			continue;
		if (match_string(buf, "domainname=", domain_name, sizeof(domain_name)))
			continue;
		if (match_string(buf, "inewsdir=", inewsdir, sizeof(inewsdir)))
			continue;
		if (match_string(buf, "bugaddress=", bug_addr, sizeof(bug_addr)))
			continue;
		if (match_string(buf, "organization=", default_organization, sizeof(default_organization)))
			continue;
#ifndef CHARSET_CONVERSION
		if (match_string(buf, "mm_charset=", tinrc.mm_charset, sizeof(tinrc.mm_charset)))
			continue;
#else
		if (match_list(buf, "mm_charset=", txt_mime_charsets, NUM_MIME_CHARSETS, &tinrc.mm_network_charset))
			continue;
#endif /* !CHARSET_CONVERSION */
		if (match_list(buf, "post_mime_encoding=", txt_mime_encodings, NUM_MIME_ENCODINGS, &tinrc.post_mime_encoding))
			continue;
		if (match_list(buf, "mail_mime_encoding=", txt_mime_encodings, NUM_MIME_ENCODINGS, &tinrc.mail_mime_encoding))
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
	if (!strlen(tinrc.strip_re_regex))
		STRCPY(tinrc.strip_re_regex, DEFAULT_STRIP_RE_REGEX);
	compile_regex(tinrc.strip_re_regex, &strip_re_regex, PCRE_ANCHORED);

	if (strlen(tinrc.strip_was_regex)) {
		/*
		 * try to be clever, if we still use the initial default value
		 * convert it to our needs
		 *
		 * TODO: a global soultion
		 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if (IS_LOCAL_CHARSET("UTF-8") && utf8_pcre()) {
				if (!strcmp(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX))
					STRCPY(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX);
			} else {
				if (!strcmp(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX))
					STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
			}
#else
			if (!strcmp(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX))
				STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (IS_LOCAL_CHARSET("UTF-8") && utf8_pcre())
			STRCPY(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX);
		else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
	}
	compile_regex(tinrc.strip_was_regex, &strip_was_regex, 0);

#ifdef HAVE_COLOR
	if (!strlen(tinrc.quote_regex))
		STRCPY(tinrc.quote_regex, DEFAULT_QUOTE_REGEX);
	compile_regex(tinrc.quote_regex, &quote_regex, PCRE_CASELESS);
	if (!strlen(tinrc.quote_regex2))
		STRCPY(tinrc.quote_regex2, DEFAULT_QUOTE_REGEX2);
	compile_regex(tinrc.quote_regex2, &quote_regex2, PCRE_CASELESS);
	if (!strlen(tinrc.quote_regex3))
		STRCPY(tinrc.quote_regex3, DEFAULT_QUOTE_REGEX3);
	compile_regex(tinrc.quote_regex3, &quote_regex3, PCRE_CASELESS);
#endif /* HAVE_COLOR */

	if (!strlen(tinrc.slashes_regex))
		STRCPY(tinrc.slashes_regex, DEFAULT_SLASHES_REGEX);
	compile_regex(tinrc.slashes_regex, &slashes_regex, PCRE_CASELESS);
	if (!strlen(tinrc.stars_regex))
		STRCPY(tinrc.stars_regex, DEFAULT_STARS_REGEX);
	compile_regex(tinrc.stars_regex, &stars_regex, PCRE_CASELESS);
	if (!strlen(tinrc.strokes_regex))
		STRCPY(tinrc.strokes_regex, DEFAULT_STROKES_REGEX);
	compile_regex(tinrc.strokes_regex,&strokes_regex, PCRE_CASELESS);
	if (!strlen(tinrc.underscores_regex))
		STRCPY(tinrc.underscores_regex, DEFAULT_UNDERSCORES_REGEX);
	compile_regex(tinrc.underscores_regex, &underscores_regex, PCRE_CASELESS);

	compile_regex(UUBEGIN_REGEX, &uubegin_regex, PCRE_ANCHORED);
	compile_regex(UUBODY_REGEX, &uubody_regex, PCRE_ANCHORED);

	compile_regex(URL_REGEX, &url_regex, PCRE_CASELESS);
	compile_regex(MAIL_REGEX, &mail_regex, PCRE_CASELESS);
	compile_regex(NEWS_REGEX, &news_regex, PCRE_CASELESS);

	compile_regex(SHAR_REGEX, &shar_regex, PCRE_ANCHORED);
}


#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
static t_bool
utf8_pcre(
	void)
{
	int i = 0;

#	if (defined(PCRE_MAJOR) && PCRE_MAJOR >= 4)
	(void) pcre_config(PCRE_CONFIG_UTF8, &i);
#	endif /* PCRE_MAJOR && PCRE_MAJOR >= $*/

	return (i ? TRUE : FALSE);
}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
