/*
 *  Project   : tin - a Usenet reader
 *  Module    : init.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2002-03-26
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
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */
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


char active_times_file[PATH_LEN];
char article[PATH_LEN];			/* ~/TIN_ARTICLE_NAME file */
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
char host_name[MAXHOSTNAMELEN];
char index_maildir[PATH_LEN];
char index_newsdir[PATH_LEN];
char index_savedir[PATH_LEN];
char inewsdir[PATH_LEN];
char libdir[PATH_LEN];			/* directory where news config files are (ie. active) */
char local_attributes_file[PATH_LEN];
char local_config_file[PATH_LEN];
char filter_file[PATH_LEN];
char local_input_history_file[PATH_LEN];
char local_newsgroups_file[PATH_LEN];	/* local copy of NNTP newsgroups file */
char local_newsrctable_file[PATH_LEN];
char lock_file[PATH_LEN];		/* contains name of index lock file */
char mail_active_file[PATH_LEN];
char mail_news_user[LEN];		/* mail new news to this user address */
char mailbox[PATH_LEN];			/* system mailbox for each user */
char mailer[PATH_LEN];			/* mail program */
char mailgroups_file[PATH_LEN];
char **news_headers_to_display_array;	/* array of which headers to display */
char **news_headers_to_not_display_array;	/* array of which headers to not display */
char newnewsrc[PATH_LEN];
char news_active_file[PATH_LEN];
char newsgroups_file[PATH_LEN];
char newsrc[PATH_LEN];
char novrootdir[PATH_LEN];		/* root directory of nov index files */
char novfilename[PATH_LEN];		/* file name of a single nov index files */
char page_header[LEN];			/* page header of pgm name and version */
char posted_info_file[PATH_LEN];
char posted_msgs_file[PATH_LEN];
char postponed_articles_file[PATH_LEN];	/* ~/.tin/postponed.articles file */
char tin_progname[PATH_LEN];		/* program name */
char rcdir[PATH_LEN];
char save_active_file[PATH_LEN];
char spooldir[PATH_LEN];		/* directory where news is */
char subscriptions_file[PATH_LEN];
char txt_help_bug_report[LEN];		/* address to send bug reports to */
char userid[PATH_LEN];

char proc_ch_default;			/* set in change_config_file () */

#ifdef VMS
	char rcdir_asfile[PATH_LEN];	/* rcdir expressed as dev:[dir]tin.dir, for stat() */
#endif /* VMS */

int groupname_len = 0;			/* 'runtime' copy of groupname_max_len */
int hist_last[HIST_MAXNUM + 1];
int hist_pos[HIST_MAXNUM + 1];
int iso2asc_supported;			/* Convert ISO-Latin1 to Ascii */
int num_headers_to_display;		/* num headers to display -- swp */
int num_headers_to_not_display;		/* num headers to not display -- swp */
int num_of_killed_arts;
int num_of_selected_arts;		/* num articles marked 'hot' */
int system_status;
int xmouse, xrow, xcol;			/* xterm button pressing information */

#ifdef HAVE_COLOR
	t_bool use_color;		/* enables/disables ansi-color support under linux-console and color-xterm */
	t_bool word_highlight;		/* word highlighting on/off */
#endif /* HAVE_COLOR */

pid_t process_id;			/* Useful to have around for .suffixes */
mode_t real_umask;

t_bool no_write = FALSE;		/* do not write newsrc on quit (-X cmd-line flag) */
t_bool check_for_new_newsgroups;	/* don't check for new newsgroups */
t_bool cmd_line;			/* batch / interactive mode */
t_bool created_rcdir;			/* checks if first time tin is started */
t_bool dangerous_signal_exit;		/* no get_respcode() in nntp_command when dangerous signal exit */
t_bool disable_gnksa_domain_check;	/* disable checking TLD in From: etc. */
t_bool disable_sender;			/* disable generation of Sender: header */
t_bool got_sig_pipe = FALSE;
t_bool filtered_articles;		/* locally killed / auto-selected articles */
t_bool list_active;
t_bool newsrc_active;
t_bool post_article_and_exit;		/* quick post of an article then exit (elm like) */
t_bool post_postponed_and_exit;		/* post postponed articles and exit */
t_bool reread_active_for_posted_arts;
t_bool read_local_newsgroups_file;	/* read newsgroups file locally or via NNTP */
t_bool read_news_via_nntp = FALSE;	/* read news locally or via NNTP */
t_bool read_saved_news = FALSE;		/* tin -R read saved news from tin -S */
t_bool show_description = TRUE;		/* current copy of tinrc flag */
t_bool batch_mode;			/* update index files only mode */
t_bool verbose = FALSE;			/* update index files only mode */
t_bool (*wildcard_func) (const char *str, char *patt, t_bool icase);		/* Wildcard matching function */
t_bool xover_supported = FALSE;
t_bool xref_supported = TRUE;
#ifdef LOCAL_CHARSET
	t_bool use_local_charset = TRUE;
#endif /* LOCAL_CHARSET */
#ifdef NNTP_ABLE
	t_bool force_auth_on_conn_open = FALSE;	/* authenticate on connection startup */
#endif /* NNTP_ABLE */

/* Currently active menu parameters */
t_menu *currmenu;

/* History entries */
char *input_history[HIST_MAXNUM + 1][HIST_SIZE + 1];

#ifdef HAVE_SYS_UTSNAME_H
	struct utsname system_info;
#endif /* HAVE_SYS_UTSNAME_H */

#ifndef M_AMIGA
	static struct passwd *myentry;
	static struct passwd pwdentry;
#endif /* !M_AMIGA */

struct regex_cache
					strip_re_regex, strip_was_regex,
					uubegin_regex, uubody_regex,
					url_regex, mail_regex, news_regex,
					shar_regex
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
	"0-.",	/* default_range_thread */
	"",		/* default_regex_pattern */
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
	0,			/* mailbox_format */
	"",		/* mail_address */
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
	iKeySaveAppendFile,		/* default_save_mode */
	0,		/* getart_limit */
	2,		/* recent_time */
	32,		/* groupname_max_length */
	KILL_READ,		/* kill_level */
	MIME_ENCODING_7BIT,		/* mail_mime_encoding */
	MIME_ENCODING_7BIT,		/* post_mime_encoding */
	POST_PROC_NONE,			/* post_process */
	REREAD_ACTIVE_FILE_SECS,	/* reread_active_file_secs */
	1,		/* scroll_lines */
	SHOW_FROM_NAME,				/* show_author */
	SORT_ARTICLES_BY_DATE_ASCEND,		/* sort_article_type */
	SORT_THREADS_BY_SCORE_DESCEND,		/* sort_threads_type */
	BOGUS_SHOW,		/* strip_bogus */
	THREAD_MAX,		/* thread_articles */
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
	0,		/* col_message (initialised later) */
	0,		/* col_newsheaders (initialised later) */
	0,		/* col_quote (initialised later) */
	0,		/* col_quote2 (initialised later) */
	0,		/* col_quote3 (initialised later) */
	0,		/* col_response (initialised later) */
	0,		/* col_signature (initialised later) */
	0,		/* col_subject (initialised later) */
	0,		/* col_text (initialised later) */
	0,		/* col_title (initialised later) */
	2,		/* word_h_display_marks */
	TRUE,		/* word_highlight */
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
	TRUE,		/* keep_posted_articles */
	POSTED_FILE,	/* keep_posted_articles_file */
	FALSE,		/* mail_8bit_header */
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
	TRUE,		/* show_lines */
	TRUE,		/* show_only_unread_arts */
	FALSE,		/* show_only_unread_groups */
	FALSE,		/* show_score */
	TRUE,		/* show_signatures */
	FALSE,		/* hide_uue */
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
	FALSE,		/* tab_after_X_selection */
	TRUE,		/* tab_goto_next_unread */
	FALSE,		/* tex2iso_conv */
	TRUE,		/* thread_catchup_on_exit */
	TRUE,		/* unlink_article */
	"",		/* inews_prog */
	FALSE,		/* use_mailreader_i */
	FALSE,		/* use_mouse */
#ifdef HAVE_KEYPAD
	FALSE,		/* use_keypad */
#endif /* HAVE_KEYPAD */
#ifdef HAVE_METAMAIL
	FALSE,		/* ask_for_metamail */
	FALSE,		/* use_metamail */
#endif /* HAVE_METAMAIL */
	FALSE,		/* default_filter_kill_case */
	FALSE,		/* default_filter_kill_expire */
	TRUE,		/* default_filter_kill_global */
	FALSE,		/* default_filter_select_case */
	FALSE,		/* default_filter_select_expire */
	TRUE		/* default_filter_select_global */
};

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
	{ &tinrc.col_message,     6 },
	{ &tinrc.col_minihelp,    3 },
	{ &tinrc.col_newsheaders, 9 },
	{ &tinrc.col_normal,     DFT_FORE },
	{ &tinrc.col_quote,       2 },
	{ &tinrc.col_quote2,      3 },
	{ &tinrc.col_quote3,      4 },
	{ &tinrc.col_response,    2 },
	{ &tinrc.col_signature,   4 },
	{ &tinrc.col_subject,     6 },
	{ &tinrc.col_text,       DFT_FORE },
	{ &tinrc.col_title,       4 },
};

static void
preinit_colors (
	void)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(our_colors); n++)
		*(our_colors[n].colorp) = DFT_INIT;
}

void
postinit_colors (
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

#ifdef NNTP_ABLE
	unsigned short nntp_tcp_port;
#endif /* NNTP_ABLE */


/*
 * Get users home directory, userid, and a bunch of other stuff!
 */
void
init_selfinfo (
	void)
{
	char *ptr;
	const char *cptr;
	FILE *fp;
	struct stat sb;

	host_name[0] = '\0';
	domain_name[0] = '\0';

#ifndef M_AMIGA
#	ifdef HAVE_SYS_UTSNAME_H
	if (uname(&system_info) < 0) {
		strcpy(system_info.sysname, "unknown");
		*system_info.machine = '\0';
		*system_info.release = '\0';
	}
#	endif /* HAVE_SYS_UTSNAME_H */
#endif /* !M_AMIGA */

	if ((cptr = get_host_name()) != NULL)
		strcpy (host_name, cptr);

#ifdef DOMAIN_NAME
	if ((cptr = get_domain_name()) != NULL)
		strcpy (domain_name, cptr);
#endif /* DOMAIN_NAME */

#ifdef HAVE_GETHOSTBYNAME
	if (domain_name[0] == '\0') {
		cptr = ((host_name[0] == '\0') ? get_fqdn((char *) 0) : get_fqdn(host_name));
		if (cptr != (char *)NULL)
			strcpy (domain_name, cptr);
	}
#endif /* HAVE_GETHOSTBYNAME */

	process_id = getpid ();

	real_umask = umask (0);
	(void) umask (real_umask);

#if defined(HAVE_SETLOCALE) && defined(LC_ALL) && !defined(NO_LOCALE)
	if (!setlocale(LC_ALL, "")) {
		/*
		 * TODO: issue a warning here like
		 *       "Can't set the specified locale! Check $LANG, $LC_CTYPE, $LC_ALL"
		 */
		;
	}
#endif /* HAVE_SETLOCALE && LC_ALL && !NO_LOCALE */

/* FIXME: move to get_user_name() [header.c] */
#ifndef M_AMIGA
	if ((myentry = getpwuid (getuid ())) != NULL) {
		memcpy (&pwdentry, myentry, sizeof (struct passwd));
		myentry = &pwdentry;
	}
#	ifdef VMS
	/* TODO: get_user_name() entirely reles on $USER */
	if (((ptr = getlogin ()) != NULL) && strlen (ptr))
		myentry = getpwnam (ptr);
#	endif /* VMS */

	if (myentry != NULL)
		strcpy (userid, myentry->pw_name);
	else {
		error_message (_(txt_error_passwd_missing));
		tin_done (EXIT_FAILURE);
	}

#	ifdef VMS
	lower (userid);
#	endif /* VMS */

	if ((ptr = getenv ("TIN_HOMEDIR")) != NULL) {
		my_strncpy (homedir, ptr, sizeof (homedir));
	} else if ((ptr = getenv ("HOME")) != NULL) {
		my_strncpy (homedir, ptr, sizeof (homedir));
	} else if (!myentry) {
		strcpy (homedir, "/tmp");
	} else
		my_strncpy (homedir, myentry->pw_dir, sizeof (homedir));

#else
	/* TODO: get_user_name() uses $USER, not $USERNAME */
	if ((ptr = getenv ("USERNAME")) != NULL) {
		my_strncpy (userid, ptr, sizeof (userid));
	} else {
		error_message (_(txt_env_var_not_found), "USERNAME");
		tin_done (EXIT_FAILURE);
	}
	/* TODO: why not also check for TIN_HOME? */
	if ((ptr = getenv ("HOME")) != NULL) {
		my_strncpy (homedir, ptr, sizeof (homedir));
	} else {
		error_message (_(txt_env_var_not_found), "HOME");
		tin_done (EXIT_FAILURE);
	}
#endif /* !M_AMIGA */

	cmdline_nntpserver[0] = '\0';
	created_rcdir = FALSE;
	dangerous_signal_exit = FALSE;
	disable_gnksa_domain_check = FALSE;
#ifdef MAC_OS_X	/* usualy they don't have a valid FQDN */
	disable_sender = TRUE;
#else
	disable_sender = FALSE;
#endif /* MAC_OS_X */
	filtered_articles = FALSE;
	iso2asc_supported = atoi (get_val ("ISO2ASC", DEFAULT_ISO2ASC));
	if (iso2asc_supported > NUM_ISO_TABLES)
		iso2asc_supported = 0;
	list_active = FALSE;
	newsrc_active = FALSE;
	num_headers_to_display = 0;
	num_headers_to_not_display = 0;
	num_of_selected_arts = 0;
	num_of_killed_arts = 0;
	post_article_and_exit = FALSE;
	post_postponed_and_exit = FALSE;
	read_local_newsgroups_file = FALSE;
	force_reread_active_file = TRUE;
	reread_active_for_posted_arts = TRUE;
	batch_mode = FALSE;
	check_for_new_newsgroups = !batch_mode;
	wildcard_func = wildmat;
#ifdef HAVE_METAMAIL
#	ifdef M_AMIGA
		/* for all those AmigaElm users ... ;-) */
		tinrc.use_metamail = (getenv ("NoMetaMail") != NULL) ? TRUE : FALSE;
#	endif /* M_AMIGA */
#endif /* HAVE_METAMAIL */

#ifdef HAVE_COLOR
	preinit_colors();
	use_color = FALSE;
	word_highlight = TRUE;
#endif /* HAVE_COLOR */

	index_maildir[0] = '\0';
	index_newsdir[0] = '\0';
	index_savedir[0] = '\0';
	newsrc[0] = '\0';

	sprintf (page_header, "%s %s release %s (\"%s\") [%s%s]",
		tin_progname, VERSION, RELEASEDATE, RELEASENAME, OSNAME,
		(iso2asc_supported >= 0 ? " ISO2ASC" : ""));
	sprintf (cvers, txt_copyright_notice, page_header);

	default_organization[0] = '\0';
	proc_ch_default = 'n';
	news_headers_to_display_array = ulBuildArgv(tinrc.news_headers_to_display, &num_headers_to_display);
	news_headers_to_not_display_array = NULL;

	/* TODO: nuke $BUG_ADDRESS entirely? */
	strcpy (bug_addr, get_val("BUG_ADDRESS", BUG_REPORT_ADDRESS));

	bug_nntpserver1[0] = '\0';
	bug_nntpserver2[0] = '\0';

	/*
	 * Amiga uses assigns which end in a ':' and won't work with a '/'
	 * tacked on after them: e.g. we want UULIB:active, and not
	 * UULIB:/active. For this reason I have changed the sprintf calls
	 * to joinpath. This is defined to sprintf(result,"%s/%s",dir,file)
	 * on all UNIX systems.
	 */

#ifdef INEWSDIR
	strcpy (inewsdir, INEWSDIR);
#else
	inewsdir[0] = '\0';
#endif /* INEWSDIR */

#ifdef apollo
	strcpy(default_organization, get_val("NEWSORG", ""));
#else
	strcpy(default_organization, get_val("ORGANIZATION", ""));
#endif /* apollo */

#ifdef USE_INN_NNTPLIB
	ptr = GetConfigValue (_CONF_ORGANIZATION);
	if (ptr != NULL)
		my_strncpy (default_organization, ptr, sizeof (default_organization));
#endif /* USE_INN_NNTPLIB */

#ifndef NNTP_ONLY
	strcpy (libdir, get_val("TIN_LIBDIR", NEWSLIBDIR)); /* moved inside ifdef */
	strcpy (novrootdir, get_val ("TIN_NOVROOTDIR", NOVROOTDIR));
	strcpy (novfilename, get_val ("TIN_NOVFILENAME", OVERVIEW_FILE));
	strcpy (spooldir, get_val ("TIN_SPOOLDIR", SPOOLDIR));
#endif /* !NNTP_ONLY */
	/* clear news_active_file, active_time_file, newsgroups_file */
	news_active_file[0] = '\0';
	active_times_file[0] = '\0';
	newsgroups_file[0] = '\0';
	subscriptions_file[0] = '\0';

	/*
	 * Setup default keymaps
	 */
	build_keymaps ();

	/*
	 * read the global site config file to override some default
	 * values given at compile time
	 */
	(void) read_site_config();

	/*
	 * the site_confog-file was the last chance to set the domainname
	 * if it's still unset exit tin.
	 */
	if (domain_name[0] == '\0') {
		error_message (txt_error_no_domain_name);
		tin_done(EXIT_FAILURE);
	}

	/*
	 * only set the following variables if they weren't set from within
	 * read_site_config()
	 *
	 * TODO do we really want that read_site_config() overwrites
	 * values given in env-vars? ($MM_CHARSET, $TIN_ACTIVEFILE)
	 */
	if (!*news_active_file) /* TODO: really prepend libdir here in case of $TIN_ACTIVEFILE is set? */
		joinpath (news_active_file, libdir, get_val ("TIN_ACTIVEFILE", ACTIVE_FILE));
	if (!*active_times_file)
		joinpath (active_times_file, libdir, ACTIVE_TIMES_FILE);
	if (!*newsgroups_file)
		joinpath (newsgroups_file, libdir, NEWSGROUPS_FILE);
	if (!*subscriptions_file)
		joinpath (subscriptions_file, libdir, SUBSCRIPTIONS_FILE);
	if (!*default_organization) {
		char buf[LEN];

		joinpath (buf, libdir, "organization");
		if ((fp = fopen(buf, "r")) != NULL) {
			if (fgets (buf, (int) sizeof (buf), fp) != NULL) {
				ptr = strrchr (buf, '\n');
				if (ptr != NULL)
					*ptr = '\0';
			}
			fclose (fp);
			my_strncpy (default_organization, buf, sizeof (default_organization));
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
		strcpy (ptr, "mm_network_charset=");
		space -= strlen(ptr);
		strncat(ptr, get_val("MM_CHARSET", MM_CHARSET), space);
		if ((space -= strlen(ptr)) > 0) {
			strncat(ptr, "\n", space);
			match_list(ptr, "mm_network_charset=", txt_mime_charsets, NUM_MIME_CHARSETS, &tinrc.mm_network_charset);
		}
		free(ptr);
	}
#endif /* !CHARSET_CONVERSION */
	/* read_site_config() might have changed the value of libdir */
	/* FIXME: we'd better use TIN_DEFAULTS_DIR instead of TIN_LIBDIR here */
	joinpath (global_attributes_file, libdir, ATTRIBUTES_FILE);
	joinpath (global_config_file, libdir, CONFIG_FILE);

#ifdef VMS
	joindir (rcdir, homedir, RCDIR); /* we're naming a directory here */
	joinpath (rcdir_asfile, homedir, RCDIR);	/* for stat() */
	strcat(rcdir_asfile, ".DIR");
	if (stat (rcdir_asfile, &sb) == -1)
#else
	joinpath (rcdir, homedir, RCDIR);
	if (stat (rcdir, &sb) == -1)
#endif /* VMS */
	{
		created_rcdir = TRUE;
		my_mkdir (rcdir, (mode_t)(S_IRWXU));
	}
#if defined(M_UNIX) || defined (M_AMIGA) || defined(VMS)
	strcpy (tinrc.mailer_format, MAILER_FORMAT);
#else
	strcpy (tinrc.mailer_format, mailer);
#endif /* M_UNIX || M_AMIGA || VMS */
#ifndef DISABLE_PRINTING
	strcpy (tinrc.printer, DEFAULT_PRINTER);
#	ifdef M_AMIGA
	if (tin_bbs_mode)
		strcpy(tinrc.printer, DEFAULT_BBS_PRINTER);
#	endif /* M_AMIGA */
#endif /* !DISABLE_PRINTING */
#if defined(NNTP_ABLE) || defined(NNTP_ONLY)
	strcpy (tinrc.inews_prog, DEFAULT_INEWS_PROG);
#endif /* NNTP_ABLE) || NNTP_ONLY */
	strcpy (mailer, get_val (ENV_VAR_MAILER, DEFAULT_MAILER));
	joinpath (article, homedir, TIN_ARTICLE_NAME);
#ifdef APPEND_PID
	sprintf (article + strlen(article), ".%d", (int) process_id);
#endif /* APPEND_PID */
	joinpath (dead_article, homedir, "dead.article");
	joinpath (dead_articles, homedir, "dead.articles");
	JOINPATH(tinrc.maildir, homedir, DEFAULT_MAILDIR);
	JOINPATH(tinrc.savedir, homedir, DEFAULT_SAVEDIR);
	joinpath (tinrc.sigfile, homedir, ".Sig");
	joinpath (default_signature, homedir, ".signature");

	if (!index_newsdir[0])
		JOINPATH(index_newsdir, get_val ("TIN_INDEX_NEWSDIR", rcdir), INDEX_NEWSDIR);
#if 0
	 else {
		if (stat (index_newsdir, &sb) == -1)
			my_mkdir (index_newsdir, (mode_t)S_IRWXUGO);
	}
#endif /* 0 */

	JOINPATH(index_maildir, get_val ("TIN_INDEX_MAILDIR", rcdir), INDEX_MAILDIR);
	if (stat (index_maildir, &sb) == -1)
		my_mkdir (index_maildir, (mode_t)S_IRWXUGO);
	joinpath (index_savedir, get_val ("TIN_INDEX_SAVEDIR", rcdir), INDEX_SAVEDIR);

	if (stat (index_savedir, &sb) == -1)
		my_mkdir (index_savedir, (mode_t)S_IRWXUGO);
	joinpath (local_attributes_file, rcdir, ATTRIBUTES_FILE);
	joinpath (local_config_file, rcdir, CONFIG_FILE);
	joinpath (filter_file, rcdir, FILTER_FILE);
	joinpath (local_input_history_file, rcdir, INPUT_HISTORY_FILE);
	joinpath (local_newsrctable_file, rcdir, NEWSRCTABLE_FILE);
	joinpath (local_newsgroups_file, rcdir, NEWSGROUPS_FILE);
	joinpath (mail_active_file, rcdir, ACTIVE_MAIL_FILE);
#ifdef VMS
	joinpath (mailbox, DEFAULT_MAILBOX, "MAIL.TXT");
#else
	joinpath (mailbox, DEFAULT_MAILBOX, userid);
#endif /* VMS */
	joinpath (mailgroups_file, rcdir, MAILGROUPS_FILE);
	joinpath (newsrc, homedir, NEWSRC_FILE);
	joinpath (newnewsrc, homedir, NEWNEWSRC_FILE);
#ifdef APPEND_PID
	sprintf(newnewsrc + strlen(newnewsrc), "%d", (int) process_id);
#endif /* APPEND_PID */
	joinpath (posted_info_file, rcdir, POSTED_FILE);
	joinpath (posted_msgs_file, tinrc.maildir, POSTED_FILE);
	joinpath (postponed_articles_file, rcdir, POSTPONED_FILE);
	joinpath (save_active_file, rcdir, ACTIVE_SAVE_FILE);

#ifdef HAVE_LONG_FILE_NAMES
	sprintf (lock_file, "%stin.%s.LCK", TMPDIR, userid);
#else
	sprintf (lock_file, "%s%s.LCK", TMPDIR, userid);
#endif /* HAVE_LONG_FILE_NAMES */

#ifdef NNTP_ABLE
	nntp_tcp_port = (unsigned short) atoi(get_val("NNTPPORT", NNTP_TCP_PORT));
#endif /* NNTP_ABLE */

	if (stat (posted_info_file, &sb) == -1) {
		if ((fp = fopen (posted_info_file, "w")) != NULL) {
			fprintf (fp, txt_posted_info_file);
			fclose (fp);
		}
	}

	if (stat (local_attributes_file, &sb) == -1)
		write_attributes_file (local_attributes_file);

	init_postinfo();
	sprintf (txt_help_bug_report, _(txt_help_bug), bug_addr);

#ifdef HAVE_PGP_GPG
	init_pgp();
#endif /* HAVE_PGP_GPG */
}

/*
 * If we're caching overview files and the user specified an NNTP server
 * with the '-g' option, make the directory name specific to the NNTP server
 * and make sure the directory exists.
 */
void
set_up_private_index_cache (
	void)
{
	struct stat sb;

	if (xover_supported && !tinrc.cache_overview_files)
		return;

	if (cmdline_nntpserver[0] != 0) {
		char *from;
		char *to;
		int c;

		to = index_newsdir + strlen (index_newsdir);
		*(to++) = '-';
		for (from = cmdline_nntpserver; (c = *from) != 0; ++from)
			*(to++) = tolower(c);
		*to = 0;
	}

	if (stat (index_newsdir, &sb) == -1)
		my_mkdir (index_newsdir, (mode_t)S_IRWXU); /* was S_IRWXUGO */

#	ifdef DEBUG
	debug_nntp ("set_up_private_index_cache", index_newsdir);
#	endif /* DEBUG */
	joinpath (local_newsgroups_file, index_newsdir, NEWSGROUPS_FILE);
	return;
}


/*
 * Create default mail & save directories if they do not exist
 */
t_bool
create_mail_save_dirs (
	void)
{
	t_bool created = FALSE;
	char path[PATH_LEN];
	struct stat sb;

	if (!strfpath (tinrc.maildir, path, sizeof (path), NULL))
		joinpath (path, homedir, DEFAULT_MAILDIR);

	if (stat (path, &sb) == -1) {
		my_mkdir (path, (mode_t)(S_IRWXU|S_IRUGO|S_IXUGO));
		created = TRUE;
	}

	if (!strfpath (tinrc.savedir, path, sizeof (path), NULL))
		joinpath (path, homedir, DEFAULT_SAVEDIR);

	if (stat (path, &sb) == -1) {
		my_mkdir (path, (mode_t)(S_IRWXU|S_IRUGO|S_IXUGO));
		created = TRUE;
	}

	return created;
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
read_site_config (
	void)
{
	FILE *fp = (FILE *)0;
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
		if (match_string (buf, "spooldir=", spooldir, sizeof (spooldir)))
			continue;
		if (match_string (buf, "overviewdir=", novrootdir, sizeof (novrootdir)))
			continue;
		if (match_string (buf, "overviewfile=", novfilename, sizeof (novfilename)))
			continue;
#endif /* !NNTP_ONLY */
		if (match_string (buf, "activefile=", news_active_file, sizeof (news_active_file)))
			continue;
		if (match_string (buf, "activetimesfile=", active_times_file, sizeof (active_times_file)))
			continue;
		if (match_string (buf, "newsgroupsfile=", newsgroups_file, sizeof (newsgroups_file)))
			continue;
		if (match_string (buf, "newslibdir=", libdir, sizeof (libdir)))
			continue;
		if (match_string (buf, "subscriptionsfile=", subscriptions_file, sizeof (subscriptions_file)))
			continue;
		if (match_string (buf, "domainname=", domain_name, sizeof (domain_name)))
			continue;
		if (match_string (buf, "inewsdir=", inewsdir, sizeof (inewsdir)))
			continue;
		if (match_string (buf, "bugaddress=", bug_addr, sizeof (bug_addr)))
			continue;
		if (match_string (buf, "organization=", default_organization, sizeof (default_organization)))
			continue;
#ifndef CHARSET_CONVERSION
		if (match_string (buf, "mm_charset=", tinrc.mm_charset, sizeof (tinrc.mm_charset)))
			continue;
#else
		if (match_list (buf, "mm_charset=", txt_mime_charsets, NUM_MIME_CHARSETS, &tinrc.mm_network_charset))
			continue;
#endif /* !CHARSET_CONVERSION */
		if (match_list (buf, "post_mime_encoding=", txt_mime_encodings, NUM_MIME_ENCODINGS, &tinrc.post_mime_encoding))
			continue;
		if (match_list (buf, "mail_mime_encoding=", txt_mime_encodings, NUM_MIME_ENCODINGS, &tinrc.mail_mime_encoding))
			continue;
		if (match_boolean (buf, "disable_gnksa_domain_check=", &disable_gnksa_domain_check))
			continue;
		if (match_boolean (buf, "disable_sender=", &disable_sender))
			continue;
	}

	fclose(fp);

	return 0;
}


/*
 * set defaults if needed to avoid empty regexp
 */
void
postinit_regexp (
	void)
{
	if (!strlen(tinrc.strip_re_regex))
		STRCPY(tinrc.strip_re_regex, DEFAULT_STRIP_RE_REGEX);
	compile_regex (tinrc.strip_re_regex, &strip_re_regex, PCRE_ANCHORED);
	if (!strlen(tinrc.strip_was_regex))
		STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
	compile_regex (tinrc.strip_was_regex, &strip_was_regex, 0);
#ifdef HAVE_COLOR
	if (!strlen(tinrc.quote_regex))
		STRCPY(tinrc.quote_regex, DEFAULT_QUOTE_REGEX);
	compile_regex (tinrc.quote_regex, &quote_regex, PCRE_CASELESS);
	if (!strlen(tinrc.quote_regex2))
		STRCPY(tinrc.quote_regex2, DEFAULT_QUOTE_REGEX2);
	compile_regex (tinrc.quote_regex2, &quote_regex2, PCRE_CASELESS);
	if (!strlen(tinrc.quote_regex3))
		STRCPY(tinrc.quote_regex3, DEFAULT_QUOTE_REGEX3);
	compile_regex (tinrc.quote_regex3, &quote_regex3, PCRE_CASELESS);
#endif /* HAVE_COLOR */

	compile_regex (UUBEGIN_REGEX, &uubegin_regex, PCRE_CASELESS|PCRE_ANCHORED);
	compile_regex (UUBODY_REGEX, &uubody_regex, PCRE_ANCHORED);

	compile_regex (URL_REGEX, &url_regex, PCRE_CASELESS);
	compile_regex (MAIL_REGEX, &mail_regex, PCRE_CASELESS);
	compile_regex (NEWS_REGEX, &news_regex, PCRE_CASELESS);

	compile_regex (SHAR_REGEX, &shar_regex, PCRE_ANCHORED);
}
