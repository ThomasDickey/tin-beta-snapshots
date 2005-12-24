/*
 *  Project   : tin - a Usenet reader
 *  Module    : main.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2005-08-14
 *  Notes     :
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

signed long int read_newsrc_lines = -1;

static char **cmdargs;
static int num_cmdargs;
static int max_cmdargs;

static t_bool catchup = FALSE;		/* mark all arts read in all subscribed groups */
static t_bool update_index = FALSE;		/* update local overviews */
static t_bool check_any_unread = FALSE;	/* print/return status if any unread */
static t_bool mail_news = FALSE;		/* mail all arts to specified user */
static t_bool save_news = FALSE;		/* save all arts to savedir structure */
static t_bool start_any_unread = FALSE;	/* only start if unread news */


/*
 * Local prototypes
 */
static t_bool create_mail_save_dirs(void);
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

	cmd_line = TRUE;

	/* initialize locale support */
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
	if (setlocale(LC_ALL, "")) {
#	ifdef ENABLE_NLS
		bindtextdomain(PACKAGE, LOCALEDIR);
		textdomain(PACKAGE);
#	endif /* ENABLE_NLS */
	} else {
		error_message(txt_error_locale);
		sleep(2);
	}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */

	set_signal_handlers();

	debug = 0;	/* debug OFF */

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
	setup_default_keys(); /* preinit keybindings */

	/*
	 * Read user local & global config files
	 * These override the compiled in defaults
	 */
	read_config_file(global_config_file, TRUE);
	read_config_file(local_config_file, FALSE);

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

	if (!batch_mode || verbose)
		wait_message(0, "%s\n", cvers);

	/*
	 * Connect to nntp server?
	 */
	nntp_server = getserverbyfile(NNTP_SERVER_FILE);
	if (read_news_via_nntp && !read_saved_news && nntp_open())
		giveup();

	read_server_config();

	/*
	 * exit early - unfortunately we can't do that in read_cmd_line_options()
	 * as nntp_caps.over_cmd is set in nntp_open()
	 */
	if (update_index && nntp_caps.over_cmd && !tinrc.cache_overview_files) {
		error_message(_(txt_batch_update_unavail), tin_progname);
		giveup();
	}

	/*
	 * Check if overview indexes contain Xref: lines
	 */
	if (nntp_caps.over_cmd)
		xref_supported = overview_xref_support();

#ifdef DEBUG_NEWSRC
	{
		char file[PATH_LEN];

		joinpath(file, TMPDIR, "BITMAP");
		unlink(file);
#	if 0
		newsrc_test_harness();
#	endif /* 0 */
	}
#endif /* DEBUG_NEWSRC */

	/*
	 * avoid empty regexp, we also need to do this in batch_mode
	 * as read_overview() calls eat_re() which uses a regexp to
	 * modify the subject *sigh*
	 */
	postinit_regexp();

	if (!(batch_mode || post_postponed_and_exit)) {
		/*
		 * Read user specific keybindings and input history
		 */
		wait_message(0, _(txt_reading_keymap_file));
		read_keymap_file();
		read_input_history_file();

		/*
		 * Load the mail & news active files into active[]
		 *
		 * create_save_active_file cannot write to active.save
		 * if no_write != FALSE, so restore original value temporarily
		 */
		if (read_saved_news) {
			no_write = tmp_no_write;
			create_save_active_file();
			no_write = TRUE;
		}
	}

#ifdef HAVE_MH_MAIL_HANDLING
	read_mail_active_file();
#endif /* HAVE_MH_MAIL_HANDLING */

	/*
	 * Initialise active[] and add new newsgroups to start of my_group[]
	 * also reads attributes
	 */
	selmenu.max = 0;
	/*
	 * we need to restore the original no_write mode to be able to handle
	 * $AUTOSUBSCRIBE groups
	 */
	no_write = tmp_no_write;
	read_news_active_file();
#ifdef DEBUG
	debug_print_active();
#endif /* DEBUG */

	/*
	 * Read in users filter preferences file. This has to be done before
	 * quick post because the filters might be updated.
	 */
	filtered_articles = read_filter_file(filter_file);

	no_write = TRUE;
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
		no_write = TRUE; /* disable newsrc updates */
		tin_done(EXIT_SUCCESS);
	}

	/* TODO: replace hardcoded key-name in txt_info_postponed */
	if ((count = count_postponed_articles()))
		wait_message(3, _(txt_info_postponed), count, PLURAL(count, txt_article));

	/*
	 * Read text descriptions for mail and/or news groups
	 */
	if (show_description && !batch_mode) {
		no_write = tmp_no_write; /* restore original value */
		read_descriptions(TRUE);
		no_write = TRUE; /* disable newsrc updates */
	}

	/*
	 * TODO: what has write_config_file() to do with create_mail_save_dirs ()
	 */
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
	 * new newsgroups and command line newsgroups already loaded. Also does
	 * auto-subscribe to groups specified in /usr/lib/news/subscriptions
	 * locally or via NNTP if reading news remotely (LIST SUBSCRIPTIONS)
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
		tin_done(check_start_save_any_news(CHECK_ANY_NEWS, catchup));

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
	 * TODO: should we temporarely set
	 *       getart_limit=-1,thread_arts=0,sort_art_type=0
	 *       for speed reasons?
	 */
	if (mail_news || save_news) {
		check_start_save_any_news(mail_news ? MAIL_ANY_NEWS : SAVE_ANY_NEWS, catchup);
		tin_done(EXIT_SUCCESS);
	}

	/*
	 * Catchup newsrc file (-c option)
	 */
	if (batch_mode && catchup && !update_index) {
		catchup_newsrc_file();
		tin_done(EXIT_SUCCESS);
	}

	/*
	 * Update index files (-u option), also does catchup if requested
	 */
	if (update_index)
		update_index_files();

	/*
	 * the code below this point can't be reached in batch mode
	 */

	/*
	 * If first time print welcome screen
	 */
	if (created_rcdir)
		show_intro_page();

#ifdef XFACE_ABLE
	if (tinrc.use_slrnface && !batch_mode)
		slrnface_start();
#endif /* XFACE_ABLE */

#ifdef USE_CURSES
	/* Turn scrolling off now the startup messages have been displayed */
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */

	/*
	 * Work loop
	 */
	selection_page(start_groupnum, num_cmd_line_groups);
	/* NOTREACHED */
	return 0;
}


/*
 * process command line options
 */
#define OPTIONS "aAcdD:f:g:G:hHI:lm:M:nNop:qQrRs:SuvVwxXzZ"

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
			case 'a':
#ifdef HAVE_COLOR
				use_color = bool_not(use_color);
#else
				error_message(_(txt_option_not_enabled), "-DHAVE_COLOR");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* HAVE_COLOR */
				break;

			case 'A':
#ifdef NNTP_ABLE
				force_auth_on_conn_open = TRUE;
#else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'c':
				batch_mode = TRUE;
				catchup = TRUE;
				break;

			case 'd':
				show_description = FALSE;
				break;

			case 'D':		/* debug mode 1=NNTP, 2=ALL, 3=newsrc, 4=malloc */
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

			case 'f':	/* newsrc file */
				my_strncpy(newsrc, optarg, sizeof(newsrc) - 1);
				newsrc_set = TRUE;
				break;

			case 'G':
				tinrc.getart_limit = atoi(optarg);
				break;

			case 'g':	/* select alternative NNTP-server, implies -r */
#ifdef NNTP_ABLE
				my_strncpy(cmdline_nntpserver, optarg, sizeof(cmdline_nntpserver) - 1);
				read_news_via_nntp = TRUE;
#else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'H':
				show_intro_page();
				exit(EXIT_SUCCESS);
				/* keep lint quiet: */
				/* FALLTHROUGH */

			case 'I':
				my_strncpy(index_newsdir, optarg, sizeof(index_newsdir) - 1);
				break;

			case 'l':
				list_active = TRUE;
				break;

			case 'm':
				my_strncpy(tinrc.maildir, optarg, sizeof(tinrc.maildir) - 1);
				break;

			case 'M':	/* mail new news to specified user */
				my_strncpy(mail_news_user, optarg, sizeof(mail_news_user) - 1);
				mail_news = TRUE;
				batch_mode = TRUE;
				break;

			case 'n':
				newsrc_active = TRUE;
				break;

			case 'N':	/* mail new news to your posts */
				my_strncpy(mail_news_user, userid, sizeof(mail_news_user) - 1);
				mail_news = TRUE;
				batch_mode = TRUE;
				break;

			case 'o':	/* post postponed articles & exit */
#ifndef NO_POSTING
				/*
				 * TODO: autoposting currently does some screen output, so we
				 *       can't set batch_mode
				 */
				post_postponed_and_exit = TRUE;
				check_for_new_newsgroups = FALSE;
#else
				error_message(_(txt_option_not_enabled), "-UNO_POSTING");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* !NO_POSTING */
				break;

			case 'p': /* implies -r */
#ifdef NNTP_ABLE
				read_news_via_nntp = TRUE;
				if (atoi(optarg) != 0)
					nntp_tcp_port = (unsigned short) atoi(optarg);
#else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'q':
				check_for_new_newsgroups = FALSE;
				break;

			case 'Q':
				newsrc_active = TRUE;
				check_for_new_newsgroups = FALSE;
				show_description = FALSE;
				break;

			case 'r':	/* read news remotely from default NNTP server */
#ifdef NNTP_ABLE
				read_news_via_nntp = TRUE;
#else
				error_message(_(txt_option_not_enabled), "-DNNTP_ABLE");
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'R':	/* read news saved by -S option */
				read_saved_news = TRUE;
				list_active = TRUE;
				newsrc_active = FALSE;
				check_for_new_newsgroups = FALSE;
				my_strncpy(news_active_file, save_active_file, sizeof(news_active_file) - 1);
				break;

			case 's':
				my_strncpy(tinrc.savedir, optarg, sizeof(tinrc.savedir) - 1);
				break;

			case 'S':	/* save new news to dir structure */
				save_news = TRUE;
				batch_mode = TRUE;
				break;

			case 'u':	/* update index files */
				batch_mode = TRUE;
				update_index = TRUE;
				break;

			case 'v':	/* verbose mode */
				verbose = TRUE;
				break;

			case 'V':
				tin_version_info(stderr);
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
#endif /* !NO_POSTING */
				break;

#if 0
			case 'W':	/* reserved according to SUSV3 XDB Utility Syntax Guidelines, Guideline 3 */
				break;
#endif /* 0 */

			case 'x':	/* enter no_posting mode */
				force_no_post = TRUE;
				break;

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
#ifdef HAVE_GETHOSTNAME
			(void) gethostname(nodenamebuf, sizeof(nodenamebuf));
#endif /* HAVE_GETHOSTNAME */
			get_newsrcname(newsrc, nodenamebuf);
#endif /* HAVE_SYS_UTSNAME_H && HAVE_UNAME */
		}
	}

	/*
	 * Sort out conflicts of options....
	 */
	if (!batch_mode) {
		if (verbose) {
			wait_message(2, _(txt_useful_with_batch_mode), "-v");
			verbose = FALSE;
		}
		if (catchup) {
			wait_message(2, _(txt_useful_with_batch_mode), "-c");
			catchup = FALSE;
		}
	} else {
		if (read_saved_news) {
			wait_message(2, _(txt_useful_without_batch_mode), "-R");
			read_saved_news = FALSE;
		}
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
	if (!list_active && !newsrc_active)
		list_active = newsrc_active = TRUE;
}


/*
 * usage
 */
static void
usage(
	char *theProgname)
{
	error_message(_(txt_usage_tin), theProgname);

#ifdef HAVE_COLOR
		error_message(_(txt_usage_toggle_color));
#endif /* HAVE_COLOR */
#ifdef NNTP_ABLE
		error_message(_(txt_usage_force_authentication));
#endif /* NNTP_ABLE */

	error_message(_(txt_usage_catchup));
	error_message(_(txt_usage_dont_show_descriptions));

#ifdef DEBUG
	error_message(_(txt_usage_debug));
#endif /* DEBUG */

	error_message(_(txt_usage_newsrc_file), newsrc);
	error_message(_(txt_usage_getart_limit));

#ifdef NNTP_ABLE
	error_message(_(txt_usage_newsserver), get_val("NNTPSERVER", NNTP_DEFAULT_SERVER));
#endif /* NNTP_ABLE */

	error_message(_(txt_usage_help_message));
	error_message(_(txt_usage_help_information), theProgname);
	error_message(_(txt_usage_index_newsdir), index_newsdir);
	error_message(_(txt_usage_read_only_active));
	error_message(_(txt_usage_maildir), tinrc.maildir);
	error_message(_(txt_usage_mail_new_news_to_user));
	error_message(_(txt_usage_read_only_subscribed));
	error_message(_(txt_usage_mail_new_news));
	error_message(_(txt_usage_post_postponed_arts));

#ifdef NNTP_ABLE
	error_message(_(txt_usage_port), nntp_tcp_port);
#endif /* NNTP_ABLE */

	error_message(_(txt_usage_dont_check_new_newsgroups));
	error_message(_(txt_usage_quickstart));

#ifdef NNTP_ABLE
	if (!read_news_via_nntp)
		error_message(_(txt_usage_read_news_remotely));
#endif /* NNTP_ABLE */

	error_message(_(txt_usage_read_saved_news));
	error_message(_(txt_usage_savedir), tinrc.savedir);
	error_message(_(txt_usage_save_new_news));
	error_message(_(txt_usage_update_index_files));
	error_message(_(txt_usage_verbose));
	error_message(_(txt_usage_version));
	error_message(_(txt_usage_post_article));
	error_message(_(txt_usage_no_posting));
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
	cCOLS = 132;							/* set because curses has not started */
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

	snprintf(buf, sizeof(buf), _(txt_intro_page), PRODUCT, PRODUCT, PRODUCT, bug_addr);

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
			if (!batch_mode)
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


/*
 * Create default mail & save directories if they do not exist
 * TODO: return code not meaningful ?
 */
static t_bool
create_mail_save_dirs(
	void)
{
	t_bool created = FALSE;
	char path[PATH_LEN];
	struct stat sb;

	if (!strfpath(tinrc.maildir, path, sizeof(path), NULL))
		joinpath(path, homedir, DEFAULT_MAILDIR);

	if (stat(path, &sb) == -1) {
		my_mkdir(path, (mode_t) (S_IRWXU));
		created = TRUE;
	}

	if (!strfpath(tinrc.savedir, path, sizeof(path), NULL))
		joinpath(path, homedir, DEFAULT_SAVEDIR);

	if (stat(path, &sb) == -1) {
		my_mkdir(path, (mode_t) (S_IRWXU));
		created = TRUE;
	}

	return created;
}


/*
 * we don't try do free() any previously malloc()ed mem here as exit via
 * giveup() indicates a serious error and keeping track of what we've
 * already malloc()ed would be a PITA.
 */
void
giveup(
	void)
{
	static int nested;

#ifdef XFACE_ABLE
	slrnface_stop();
#endif /* XFACE_ABLE */

	if (!cmd_line && !nested++) {
		cursoron();
		EndWin();
		Raw(FALSE);
	}
	exit(EXIT_FAILURE);
}
