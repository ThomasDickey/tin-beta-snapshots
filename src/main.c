/*
 *  Project   : tin - a Usenet reader
 *  Module    : main.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2003-02-15
 *  Notes     :
 *
 * Copyright (c) 1991-2003 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */

#if defined(M_AMIGA) && defined(__SASC_650)
	extern int _WBArg;
	extern char	**_WBArgv;
	char __stdiowin[] = "con:0/12/640/200/TIN " VERSION;
	char __stdiov37[] = "/AUTO/NOCLOSE";
#else
#	ifdef VMS
	int debug;
#	endif /* VMS */
#endif /* M_AMIGA && __SASC_650 */

signed long int read_newsrc_lines = -1;

static char **cmdargs;
static int num_cmdargs;
static int max_cmdargs;

static t_bool catchup = FALSE;		/* mark all arts read in all subscribed groups */
static t_bool check_any_unread = FALSE;	/* print/return status if any unread */
static t_bool mail_news = FALSE;		/* mail all arts to specified user */
static t_bool save_news = FALSE;		/* save all arts to savedir structure */
static t_bool start_any_unread = FALSE;	/* only start if unread news */


/*
 * Local prototypes
 */
static void read_cmd_line_options(int argc, char *argv[]);
static void show_intro_page(void);
static void update_index_files(void);
static void usage(char *theProgname);


/*
 * OK lets start the ball rolling...
 */
int
main(
	int argc,
	char *argv[])
{
	int count;
	int num_cmd_line_groups;
	int start_groupnum = 0;
	t_bool tmp_no_write;

	/* initialize locale support */
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
	if (setlocale(LC_ALL, "")) {
#	ifdef ENABLE_NLS
		bindtextdomain(PACKAGE, LOCALEDIR);
		textdomain(PACKAGE);
#	endif /* ENABLE_NLS */
	} else { /* EMPTY */
	/*
	 * TODO: issue a warning here like
	 *       "Can't set the specified locale! Check $LANG, $LC_CTYPE, $LC_ALL"
	 */
	}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */

	set_signal_handlers();

	cmd_line = TRUE;
	debug = 0;	/* debug OFF */

#if defined(M_AMIGA) && defined(__SASC)
	/* Call tzset() here! */
	_TZ = "GMT0";
	tzset();
	if (argc == 0) { /* we are running from the Workbench */
		argc = _WBArgc;
		argv = _WBArgv;
	}
#endif /* M_AMIGA && __SASC */

	base_name(argv[0], tin_progname);
#ifdef VMS
	argv[0] = tin_progname;
#endif /* VMS */

#ifdef NNTP_ONLY
	read_news_via_nntp = TRUE;
#else
	/*
	 * If called as rtin, read news remotely via NNTP
	 */
	if (tin_progname[0] == 'r') {
#	ifdef NNTP_ABLE
		read_news_via_nntp = TRUE;
#	else
		error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
		giveup();
#	endif /* NNTP_ABLE */
	}
#endif /* NNTP_ONLY */

	/*
	 * Set up initial array sizes, char *'s: homedir, newsrc, etc.
	 */
	init_alloc();
	hash_init();
	init_selfinfo();
	init_group_hash();

	/*
	 * Read user local & global config files
	 * These override the compiled in defaults
	 */
	read_config_file(global_config_file, TRUE);
	read_config_file(local_config_file, FALSE);

	/*
	 * avoid empty regexp
	 */
	postinit_regexp();

	/*
	 * Process envargs & command line options
	 * These override the configured in values
	 */
	read_cmd_line_options(argc, argv);

	tmp_no_write = no_write; /* keep no_write */
	no_write = TRUE;		/* don't allow any writing back during startup */

	if (!batch_mode) {
#ifdef M_UNIX
#	ifndef USE_CURSES
		if (!get_termcaps()) {
			error_message(_(txt_screen_init_failed), tin_progname);
			giveup();
		}
#	endif /* !USE_CURSES */
#endif /* M_UNIX */

		/*
		 * Init curses emulation
		 */
		if (!InitScreen()) {
			error_message(_(txt_screen_init_failed), tin_progname);
			giveup();
		}

		EndInverse();

		/*
		 * This depends on various things in tinrc
		 */
		setup_screen();
	}

	if (!batch_mode || (batch_mode && verbose))
		wait_message(0, "%s\n", cvers);

	set_up_private_index_cache();

	/*
	 * Connect to nntp server?
	 */
	if (read_news_via_nntp) {
		if (!read_saved_news) {
			if (nntp_open())
				giveup();
		} else { /* set nntp_server if reading via -R as it's used in select.c */
			nntp_server = getserverbyfile(NNTP_SERVER_FILE);
			if (nntp_server == NULL)
				nntp_server = my_strdup("reading saved news"); /* mem-leak ,-) */
		}
	}

	/*
	 * Check if overview indexes contain Xref: lines
	 */
	if (xover_supported)
		xref_supported = overview_xref_support();

#ifdef DEBUG_NEWSRC
	unlink("/tmp/BITMAP");
/*	vNewsrcTestHarness(); */
#endif /* DEBUG_NEWSRC */

	/*
	 * Read user specific keybindings and input history
	 */
	if (!batch_mode) {
		wait_message(0, _(txt_reading_keymap_file));
		read_keymap_file();
		read_input_history_file();
	}

	/*
	 * Load the mail & news active files into active[]
	 *
	 * create_save_active_file cannot write to active.save
	 * if no_write != FALSE, so restore original value temporarily
	 */
	no_write = tmp_no_write;
	if (read_saved_news)
		create_save_active_file();
	no_write = TRUE;

#ifdef HAVE_MH_MAIL_HANDLING
	read_mail_active_file();
#endif /* HAVE_MH_MAIL_HANDLING */

	/*
	 * Initialise active[] and add new newsgroups to start of my_group[]
	 */
	selmenu.max = 0;
	read_news_active_file();
#ifdef DEBUG
	debug_print_active();
#endif /* DEBUG */

	/*
	 * Load the local & global group specific attribute files
	 */
	if (!batch_mode || (batch_mode && verbose))
		wait_message(0, _(txt_reading_attributes_file), _(txt_global));
	read_attributes_file(TRUE);
	if (!batch_mode || (batch_mode && verbose))
		wait_message(0, _(txt_reading_attributes_file), "");
	read_attributes_file(FALSE);

	/*
	 * Read in users filter preferences file. This has to be done before
	 * quick post because the filters might be updated.
	 */
	filtered_articles = read_filter_file(filter_file);

#ifdef DEBUG
	debug_print_filters();
#endif /* DEBUG */

	/*
	 * Quick post an article and exit if -w or -o specified
	 */
	if (post_article_and_exit || post_postponed_and_exit) {
		no_write = tmp_no_write; /* restore original value */
		quick_post_article(post_postponed_and_exit);
		wait_message(2, _(txt_exiting));
		no_write = TRUE; /* disable tinrc updates */
		tin_done(EXIT_SUCCESS);
	}

	/* TODO: replace hardcoded key-name in txt_info_postponed */
	if ((count = count_postponed_articles()))
		wait_message(3, _(txt_info_postponed), count, PLURAL(count, txt_article));

	/*
	 * Read text descriptions for mail and/or news groups
	 */
	if (show_description && !batch_mode)
		read_descriptions(TRUE);

	if (create_mail_save_dirs())
		write_config_file(local_config_file);

	/*
	 * Preloads active[] with command line groups. They will follow any
	 * new newsgroups
	 */
	num_cmd_line_groups = read_cmd_line_groups();

	backup_newsrc();

	/*
	 * Load my_groups[] from the .newsrc file. We append these groups to any
	 * new newsgroups and command line newsgroups already loaded
	 */

	/*
	 * TODO:
	 * if (num_cmd_line_groups != 0 && check_any_unread)
	 * don't read newsrc.
	 * This makes -Z handle command line newsgroups. Test & document
	 */
	read_newsrc_lines = read_newsrc(newsrc, FALSE);
	no_write = tmp_no_write; /* restore old value */

	/*
	 * We have to show all groups with command line groups
	 */
	if (num_cmd_line_groups)
		tinrc.show_only_unread_groups = FALSE;
	else
		toggle_my_groups(NULL);

	/*
	 * Check/start if any new/unread articles
	 */
	if (check_any_unread)
		exit(check_start_save_any_news(CHECK_ANY_NEWS, catchup));

	if (start_any_unread) {
		batch_mode = TRUE;			/* Suppress some unwanted on-screen garbage */
		if ((start_groupnum = check_start_save_any_news(START_ANY_NEWS, catchup)) == -1)
			giveup();				/* No new/unread news so exit */
		batch_mode = FALSE;
	}

	/*
	 * Mail any new articles to specified user
	 * or
	 * Save any new articles to savedir structure for later reading
	 *
	 * FIXME: this currentyl doen't work, see comments in
	 *        check_start_save_any_news()
	 * TODO: should we temporarely set
	 *       getart_limit=-1,thread_arts=0,sort_art_type=0
	 *       for speed reasons?
	 */
	if (mail_news || save_news) {
		do_update(FALSE);
		check_start_save_any_news(mail_news ? MAIL_ANY_NEWS : SAVE_ANY_NEWS, catchup);
		tin_done(EXIT_SUCCESS);
	}

	/*
	 * Catchup newsrc file (-c option)
	 */
	if (catchup && batch_mode) {
		catchup_newsrc_file();
		tin_done(EXIT_SUCCESS);
	}

	/*
	 * Update index files
	 * Only the -u batch_mode case will get this far
	 */
	if (batch_mode)
		update_index_files();

	/*
	 * If first time print welcome screen and auto-subscribe
	 * to groups specified in /usr/lib/news/subscribe locally
	 * or via NNTP if reading news remotely (LIST SUBSCRIBE)
	 */
	if (created_rcdir && !batch_mode)
		show_intro_page();

#ifdef USE_CURSES
	/* Turn scrolling off now the startup messages have been displayed */
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */

	/*
	 * Work loop
	 */
	selection_page(start_groupnum, num_cmd_line_groups);
	return 0; /* not reached */
}


/*
 * process command line options
 */
#ifndef M_AMIGA
#	define OPTIONS "aAcdD:f:G:g:hHI:lm:M:nNop:qQrRs:SuvVwXzZ"
#else
#	define OPTIONS "BcdD:f:G:hHI:lm:M:nNop:qQrRs:SuvVwXzZ"
#endif /* M_AMIGA */

static void
read_cmd_line_options(
	int argc,
	char *argv[])
{
	int ch;
	t_bool newsrc_set = FALSE;

	envargs(&argc, &argv, "TINRC");

	while ((ch = getopt(argc, argv, OPTIONS)) != -1) {
		switch (ch) {
#	ifndef M_AMIGA
			case 'a':
#		ifdef HAVE_COLOR
				use_color = bool_not(use_color);
#		else
				error_message(_(txt_option_not_enabled), "-DHAVE_COLOR");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#		endif /* HAVE_COLOR */
				break;

			case 'A':
#		ifdef NNTP_ABLE
				force_auth_on_conn_open = TRUE;
#		else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#		endif /* NNTP_ABLE */
				break;
#	else
			case 'B':
				tin_bbs_mode = TRUE;
				break;
#	endif /* !M_AMIGA */

			case 'c':
				catchup = TRUE;
				break;

			case 'd':
				show_description = FALSE;
				break;

			case 'D':		/* debug mode 1=NNTP 2=ALL */
#ifdef DEBUG
				debug = atoi(optarg);
				debug_delete_files();
#else
				error_message(_(txt_option_not_enabled), "-DDEBUG");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* DEBUG */
				break;

			case 'f':	/* newsrc (tin) file */
				my_strncpy(newsrc, optarg, sizeof(newsrc));
				newsrc_set = TRUE;
				break;

			case 'G':
				tinrc.getart_limit = atoi(optarg);
				break;

#	ifndef M_AMIGA
			case 'g':	/* select alternative NNTP-server, implies -r */
#		ifdef NNTP_ABLE
				my_strncpy(cmdline_nntpserver, optarg, sizeof(cmdline_nntpserver));
				read_news_via_nntp = TRUE;
#		else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#		endif /* NNTP_ABLE */
				break;
#	endif /* !M_AMIGA */

			case 'H':
				show_intro_page();
				exit(EXIT_SUCCESS);
				/* keep lint quiet: */
				/* FALLTHROUGH */

			case 'I':
#ifndef NNTP_ONLY
				my_strncpy(index_newsdir, optarg, sizeof(index_newsdir));
#else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* !NNTP_ONLY */
				break;

			case 'l':
				list_active = TRUE;
				break;

			case 'm':
				my_strncpy(tinrc.maildir, optarg, sizeof(tinrc.maildir));
				break;

			case 'M':	/* mail new news to specified user */
				my_strncpy(mail_news_user, optarg, sizeof(mail_news_user));
				mail_news = TRUE;
				batch_mode = TRUE;
				break;

			case 'n':
				newsrc_active = TRUE;
				break;

			case 'N':	/* mail new news to your posts */
				my_strncpy(mail_news_user, userid, sizeof(userid));
				mail_news = TRUE;
				batch_mode = TRUE;
				break;

			case 'o':	/* post postponed articles & exit */
#ifndef NO_POSTING
				post_postponed_and_exit = TRUE;
				check_for_new_newsgroups = FALSE;
#else
				error_message(_(txt_option_not_enabled), "-UNO_POSTING");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NO_POSTING */
				break;

#ifdef NNTP_ABLE
			case 'p': /* implies -r */
				read_news_via_nntp = TRUE;
				if (atoi(optarg) != 0)
					nntp_tcp_port = (unsigned short) atoi(optarg);
				break;
#endif /* NNTP_ABLE */

			case 'q':
				check_for_new_newsgroups = FALSE;
				break;

			case 'Q':
				newsrc_active = TRUE;
				check_for_new_newsgroups = FALSE;
				show_description = FALSE;
				break;

			case 'r':	/* read news remotely from default NNTP server */
#	ifdef NNTP_ABLE
				read_news_via_nntp = TRUE;
#	else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#	endif /* NNTP_ABLE */
				break;

			case 'R':	/* read news saved by -S option */
				read_saved_news = TRUE;
				list_active = TRUE;
				newsrc_active = FALSE;
				check_for_new_newsgroups = FALSE;
				my_strncpy(news_active_file, save_active_file, sizeof(news_active_file));
				break;

			case 's':
				my_strncpy(tinrc.savedir, optarg, sizeof(tinrc.savedir));
				break;

			case 'S':	/* save new news to dir structure */
				save_news = TRUE;
				batch_mode = TRUE;
				break;

			case 'u':	/* update index files */
#	ifndef NNTP_ONLY
				batch_mode = TRUE;
				show_description = FALSE;
#	else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#	endif /* !NNTP_ONLY */
				break;

			case 'v':	/* verbose mode */
				verbose = TRUE;
				break;

			case 'V':
#if defined(__DATE__) && defined(__TIME__)
				error_message(_("Version: %s release %s (\"%s\") %s %s"),
					VERSION, RELEASEDATE, RELEASENAME, __DATE__, __TIME__);
#else
				error_message(_("Version: %s release %s (\"%s\")"),
					VERSION, RELEASEDATE, RELEASENAME);
#endif /* __DATE__ && __TIME__ */
/*
 * FIXME: make the rest below a function (i.e. to fill a struct), so we
 * can also use it inside post.c:mail_bug_report()
 */
#ifdef SYSTEM_NAME
				error_message("Platform:");
				error_message("\tOS-Name  = \"%s\"", SYSTEM_NAME);
#endif /* SYSTEM_NAME */
#ifdef TIN_CC
				error_message("Compiler:");
				error_message("\tCC       = \"%s\"", TIN_CC);
#		ifdef TIN_CFLAGS
				error_message("\tCFLAGS   = \"%s\"", TIN_CFLAGS);
#		endif /* TIN_CFLAGS */
#	ifdef TIN_CPP
				error_message("\tCPP      = \"%s\"", TIN_CPP);
#	endif /* TIN_CPP */
#		ifdef TIN_CPPFLAGS
				error_message("\tCPPFLAGS = \"%s\"", TIN_CPPFLAGS);
#		endif /* TIN_CPPFLAGS */
#endif /* TIN_CC */

#ifdef TIN_LD
				error_message("Linker and Libraries:");
				error_message("\tLD       = \"%s\"", TIN_LD);
#		ifdef TIN_LDFLAGS
				error_message("\tLDFLAGS  = \"%s\"", TIN_LDFLAGS);
#		endif /* TIN_LDFLAGS */

#	ifdef TIN_LIBS
				error_message("\tLIBS     = \"%s\"", TIN_LIBS);
#	endif /* TIN_LIBS */
#endif /* TIN_LD */

				error_message("Characteristics:");
				error_message("\t"
/* TODO: complete list and do some useful grouping */
#ifdef NNTP_ONLY
				"+NNTP_ONLY "
#else
#	ifdef NNTP_ABLE
				"+NNTP_ABLE "
#	else
				"-NNTP_ABLE "
#	endif /* NNTP_ABLE */
#endif /* NNTP_ONLY */
#ifdef NO_POSTING
				"+NO_POSTING "
#else
				"-NO_POSTING "
#endif /* NO_POSTING */
#ifdef BROKEN_LISTGROUP
				"+BROKEN_LISTGROUP "
#else
				"-BROKEN_LISTGROUP "
#endif /* BROKEN_LISTGROUP */
#ifdef XHDR_XREF
				"+XHDR_XREF"
#else
				"-XHDR_XREF"
#endif /* XHDR_XREF */
				"\n\t"
#ifdef HAVE_FASCIST_NEWSADMIN
				"+HAVE_FASCIST_NEWSADMIN "
#else
				"-HAVE_FASCIST_NEWSADMIN "
#endif /* HAVE_FASCIST_NEWSADMIN */
#ifdef ENABLE_IPV6
				"+ENABLE_IPV6 "
#else
				"-ENABLE_IPV6 "
#endif /* ENABLE_IPV6 */
#ifdef HAVE_COREFILE
				"+HAVE_COREFILE"
#else
				"-HAVE_COREFILE"
#endif /* HAVE_COREFILE */
				"\n\t"
#ifdef NO_SHELL_ESCAPE
				"+NO_SHELL_ESCAPE "
#else
				"-NO_SHELL_ESCAPE "
#endif /* NO_SHELL_ESCAPE */
#ifdef DISABLE_PRINTING
				"+DISABLE_PRINTING "
#else
				"-DISABLE_PRINTING "
#endif /* DISABLE_PRINTING */
#ifdef DONT_HAVE_PIPING
				"+DONT_HAVE_PIPING "
#else
				"-DONT_HAVE_PIPING "
#endif /* DONT_HAVE_PIPING */
#ifdef NO_ETIQUETTE
				"+NO_ETIQUETTE"
#else
				"-NO_ETIQUETTE"
#endif /* NO_ETIQUETTE */
				"\n\t"
#ifdef HAVE_LONG_FILE_NAMES
				"+HAVE_LONG_FILE_NAMES "
#else
				"-HAVE_LONG_FILE_NAMES "
#endif /* HAVE_LONG_FILE_NAMES */
#ifdef APPEND_PID
				"+APPEND_PID "
#else
				"-APPEND_PID "
#endif /* APPEND_PID */
#ifdef HAVE_MH_MAIL_HANDLING
				"+HAVE_MH_MAIL_HANDLING"
#else
				"-HAVE_MH_MAIL_HANDLING"
#endif /* HAVE_MH_MAIL_HANDLING */
				"\n\t"
#ifdef HAVE_ISPELL
				"+HAVE_ISPELL "
#else
				"-HAVE_ISPELL "
#endif /* HAVE_ISPELL */
#ifdef HAVE_METAMAIL
				"+HAVE_METAMAIL "
#else
				"-HAVE_METAMAIL "
#endif /* HAVE_METAMAIL */
#ifdef HAVE_SUM
				"+HAVE_SUM "
#else
				"-HAVE_SUM "
#endif /* HAVE_SUM */
				"\n\t"
#ifdef HAVE_COLOR
				"+HAVE_COLOR "
#else
				"-HAVE_COLOR "
#endif /* HAVE_COLOR */
#ifdef HAVE_PGP
				"+HAVE_PGP "
#else
				"-HAVE_PGP "
#endif /* HAVE_PGP */
#ifdef HAVE_PGPK
				"+HAVE_PGPK "
#else
				"-HAVE_PGPK "
#endif /* HAVE_PGPK */
#ifdef HAVE_GPG
				"+HAVE_GPG "
#else
				"-HAVE_GPG "
#endif /* HAVE_GPG */
				"\n\t"
#ifdef MIME_BREAK_LONG_LINES
				"+MIME_BREAK_LONG_LINES "
#else
				"-MIME_BREAK_LONG_LINES "
#endif /* MIME_BREAK_LONG_LINES */
#ifdef MIME_STRICT_CHARSET
				"+MIME_STRICT_CHARSET "
#else
				"-MIME_STRICT_CHARSET "
#endif /* MIME_STRICT_CHARSET */
#ifdef CHARSET_CONVERSION
				"+CHARSET_CONVERSION "
#else
				"-CHARSET_CONVERSION "
#endif /* CHARSET_CONVERSION */
				"\n\t"
#ifdef LOCAL_CHARSET
				"+LOCAL_CHARSET "
#else
				"-LOCAL_CHARSET "
#endif /* LOCAL_CHARSET */
#ifdef NO_LOCALE
				"+NO_LOCALE "
#else
				"-NO_LOCALE "
#endif /* NO_LOCALE */
				"\n\t"
#ifdef USE_CANLOCK
				"+USE_CANLOCK "
#else
				"-USE_CANLOCK "
#endif /* USE_CANLOCK */
#ifdef EVIL_INSIDE
				"+EVIL_INSIDE "
#else
				"-EVIL_INSIDE "
#endif /* EVIL_INSIDE */
#ifdef FORGERY
				"+FORGERY "
#else
				"-FORGERY "
#endif /* FORGERY */
#ifdef TINC_DNS
				"+TINC_DNS "
#else
				"-TINC_DNS "
#endif /* TINC_DNS */
#ifdef ENFORCE_RFC1034
				"+ENFORCE_RFC1034"
#else
				"-ENFORCE_RFC1034"
#endif /* ENFORCE_RFC1034 */
				);
				error_message("\t"
#ifdef REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
				"+REQUIRE_BRACKETS_IN_DOMAIN_LITERAL "
#else
				"-REQUIRE_BRACKETS_IN_DOMAIN_LITERAL "
#endif /* REQUIRE_BRACKETS_IN_DOMAIN_LITERAL */
#ifdef FOLLOW_USEFOR_DRAFT
				"+FOLLOW_USEFOR_DRAFT"
#else
				"-FOLLOW_USEFOR_DRAFT"
#endif /* FOLLOW_USEFOR_DRAFT */
				);
				exit(EXIT_SUCCESS);
				/* keep lint quiet: */
				/* FALLTHROUGH */

			case 'w':	/* post article & exit */
#ifndef NO_POSTING
				post_article_and_exit = TRUE;
				check_for_new_newsgroups = FALSE;
#else
				error_message(_(txt_option_not_enabled), "-UNO_POSTING");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NO_POSTING */
				break;

#if 0
			case 'W':	/* reserved according to SUSV3 XDB Utility Syntax Guidelines, Guideline 3 */
				break;
#endif /* 0 */

			case 'X':	/* don't save ~/.newsrc on exit */
				no_write = TRUE;
				break;

			case 'z':
				start_any_unread = TRUE;
				break;

			case 'Z':
				check_any_unread = TRUE;
				batch_mode = TRUE;
				break;

			case 'h':
			case '?':
			default:
				usage(tin_progname);
				exit(EXIT_SUCCESS);
		}
	}
	cmdargs = argv;
	num_cmdargs = optind;
	max_cmdargs = argc;
	if (!newsrc_set) {
		if (read_news_via_nntp)
			get_newsrcname(newsrc, getserverbyfile(NNTP_SERVER_FILE));
		else {
#if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
			struct utsname uts;
			(void) uname(&uts);
			get_newsrcname(newsrc, uts.nodename);
#else
			char nodenamebuf[256]; /* SUSv2 limit; better use HOST_NAME_MAX */
#	if defined(HAVE_GETHOSTNAME) && !defined(M_AMIGA)
			(void) gethostname(nodenamebuf, sizeof(nodenamebuf));
#	else
			/* TODO: document $NodeName */
			my_strncpy(nodenamebuf, get_val("NodeName", "PROBLEM_WITH_NODE_NAME"), sizeof(nodenamebuf));
#	endif /* HAVE_GETHOSTNAME && !M_AMIGA */
			get_newsrcname(newsrc, nodenamebuf);
#endif /* HAVE_SYS_UTSNAME_H && HAVE_UNAME */
		}
	}

	/*
	 * Sort out conflicts of options....
	 */
	if (verbose && !batch_mode) {
		wait_message(2, _(txt_useful_with_batch_mode), "-v");
		verbose = FALSE;
	}
	if (catchup && !batch_mode) {
		wait_message(2, _(txt_useful_with_batch_mode), "-c");
		catchup = FALSE;
	}
	if (read_saved_news && batch_mode) {
		wait_message(2, _(txt_useful_without_batch_mode), "-R");
		read_saved_news = FALSE;
	}

#ifdef NNTP_ABLE
	/*
	 * If we're reading from an NNTP server and we've been asked not to look
	 * for new newsgroups, trust our cached copy of the newsgroups file.
	 */
	if (read_news_via_nntp)
		read_local_newsgroups_file = bool_not(check_for_new_newsgroups);
#endif /* NNTP_ABLE */
	/*
	 * If we use neither list_active nor newsrc_active,
	 * we use both of them.
	 */
	if (!list_active && !newsrc_active) {
		list_active = TRUE;
		newsrc_active = TRUE;
	}
}


/*
 * usage
 */
static void
usage(
	char *theProgname)
{
	error_message(_(txt_usage_tin), theProgname);

#	ifndef M_AMIGA
#		ifdef HAVE_COLOR
			error_message(_(txt_usage_toggle_color));
#		endif /* HAVE_COLOR */
#		ifdef NNTP_ABLE
			error_message(_(txt_usage_force_authentication));
#		endif /* NNTP_ABLE */
#	else
		error_message(_(txt_usage_bbs_mode));
#	endif /* !M_AMIGA */

	error_message(_(txt_usage_catchup));
	error_message(_(txt_usage_dont_show_descriptions));

#	ifdef DEBUG
		error_message(_(txt_usage_debug));
#	endif /* DEBUG */

	error_message(_(txt_usage_newsrc_file), newsrc);
	error_message(_(txt_usage_getart_limit));

#	ifndef M_AMIGA
#		ifdef NNTP_ABLE
			/* FIXME, default should be $NNTPSERVER if set ... */
			error_message(_(txt_usage_newsserver), NNTP_DEFAULT_SERVER);
#		endif /* NNTP_ABLE */
#	endif /* !M_AMIGA */

	error_message(_(txt_usage_help_message));
	error_message(_(txt_usage_help_information), theProgname);

#	ifndef NNTP_ONLY
		error_message(_(txt_usage_index_newsdir), index_newsdir);
#	endif /* !NNTP_ONLY */

	error_message(_(txt_usage_use_listgroup));
	error_message(_(txt_usage_maildir), tinrc.maildir);
	error_message(_(txt_usage_mail_new_news_to_user));
	error_message(_(txt_usage_read_only_subscribed));
	error_message(_(txt_usage_mail_new_news));
	error_message(_(txt_usage_post_postponed_arts));

#	ifdef NNTP_ABLE
		error_message(_(txt_usage_port), nntp_tcp_port);
#	endif /* NNTP_ABLE */

	error_message(_(txt_usage_dont_check_new_newsgroups));
	error_message(_(txt_usage_quickstart));

#	ifdef NNTP_ABLE
		if (!read_news_via_nntp)
			error_message(_(txt_usage_read_news_remotely));
#	endif /* NNTP_ABLE */

	error_message(_(txt_usage_read_saved_news));
	error_message(_(txt_usage_savedir), tinrc.savedir);
	error_message(_(txt_usage_save_new_news));

#	ifndef NNTP_ONLY
		error_message(_(txt_usage_update_index_files));
#	endif /* !NNTP_ONLY */

	error_message(_(txt_usage_verbose));
	error_message(_(txt_usage_version));
	error_message(_(txt_usage_post_article));
	error_message(_(txt_usage_dont_save_files_on_quit));
	error_message(_(txt_usage_start_if_unread_news));
	error_message(_(txt_usage_check_for_unread_news));

	error_message(_(txt_usage_mail_bugreport), bug_addr);
}


/*
 * update index files
 */
static void
update_index_files(
	void)
{
	if (!catchup && (read_news_via_nntp && xover_supported)) {
		error_message(_(txt_batch_update_unavail), tin_progname);
		tin_done(EXIT_FAILURE);
	}

	cCOLS = 132;							/* set because curses has not started */
	batch_mode = TRUE;					/* -u handling... */
	create_index_lock_file(lock_file);
	tinrc.thread_articles = THREAD_NONE;	/* stop threading to run faster */
	do_update(catchup);
	tin_done(EXIT_SUCCESS);
}


/*
 * display page of general info. for first time user.
 */
static void
show_intro_page(
	void)
{
	char buf[4096];

	if (!cmd_line) {
		ClearScreen();
		center_line(0, TRUE, cvers);
		Raw(FALSE);
		my_printf("\n");
	}

	snprintf(buf, sizeof(buf) - 1, _(txt_intro_page), PRODUCT, PRODUCT, PRODUCT, bug_addr);

	my_fputs(buf, stdout);
	my_flush();

	if (!cmd_line) {
		Raw(TRUE);
		prompt_continue();
	}
}


/*
 * Wildcard match any newsgroups on the command line. Sort of like a limited
 * yank at startup. Return number of groups that were matched.
 */
int
read_cmd_line_groups(
	void)
{
	int matched = 0;
	int num;
	int i;

	if (num_cmdargs < max_cmdargs) {
		selmenu.max = skip_newgroups();		/* Reposition after any newgroups */

		for (num = num_cmdargs; num < max_cmdargs; num++) {
			wait_message(0, _(txt_matching_cmd_line_groups), cmdargs[num]);

			for_each_group(i) {
				if (match_group_list(active[i].name, cmdargs[num])) {
					if (my_group_add(active[i].name) != -1)
						matched++;
				}
			}
		}
	}
	return matched;
}


void
giveup(
	void)
{
	static int nested;

	if (!cmd_line && !nested++) {
		cursoron();
		EndWin();
		Raw(FALSE);
	}
	exit(EXIT_FAILURE);
}
