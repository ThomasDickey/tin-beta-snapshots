/*
 *  Project   : tin - a Usenet reader
 *  Module    : main.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2024-10-19
 *  Notes     :
 *
 * Copyright (c) 1991-2024 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */
#if defined(INET6) && defined(HAVE_INET_PTON)
#	ifndef TNNTP_H
#		include "tnntp.h"
#	endif /* !TNNTP_H */
#endif /* INET6 && HAVE_INET_PTON */


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
static char **read_cmd_line_options(int argc, char *argv[]);
static void create_mail_save_dirs(void);
static void show_intro_page(void);
_Noreturn static void update_index_files(void);
static void usage(char *theProgname);


#define FREE_ARGV_IF_NEEDED(orig, new) do { \
		if (orig != new) { \
			free(*(new + 1)); \
			FreeAndNull(new); \
		} \
	} while (0)


/*
 * OK lets start the ball rolling...
 */
int
main(
	int argc,
	char *argv[])
{
	char **argv_orig = argv;
	int count, start_groupnum;
	int num_cmd_line_groups = 0;
	t_bool tmp_no_write;

	cmd_line = TRUE;

	/* initialize locale support */
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
	if (setlocale(LC_ALL, "")) {
#	ifdef ENABLE_NLS
		bindtextdomain(NLS_TEXTDOMAIN, LOCALEDIR);
		textdomain(NLS_TEXTDOMAIN);
#	endif /* ENABLE_NLS */
	} else {
		my_fprintf(stderr, "%s\n", txt_error_locale);
		if (!batch_mode)
			sleep(4);
	}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */

	/*
	 * determine local charset
	 */
#ifndef NO_LOCALE
	{
		const char *p;

		if ((p = tin_nl_langinfo(CODESET)) != NULL) {
			if (strcasecmp(p, "ANSI_X3.4-1968")) { /* !US-ASCII */
				FreeIfNeeded(tinrc.mm_local_charset);
				tinrc.mm_local_charset = my_strdup(p);
			}
		}
	}
#endif /* !NO_LOCALE */
	/* always set a default value */
	if (!tinrc.mm_local_charset || !*tinrc.mm_local_charset) {
		FreeIfNeeded(tinrc.mm_local_charset);
		tinrc.mm_local_charset = my_strdup("US-ASCII");
	}

	set_signal_handlers();

	debug = 0;	/* can't use if (debug) before read_cmd_line_options() */

	tin_progname = my_malloc(strlen(argv[0]) + 1);
	base_name(argv[0], tin_progname);

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
		my_fprintf(stderr, _(txt_option_not_enabled), "-DNNTP_ABLE");
		my_fprintf(stderr, "\n");
		if (!batch_mode)
			sleep(2);

		free(tin_progname);
		giveup();
#	endif /* NNTP_ABLE */
	}
#endif /* NNTP_ONLY */

#ifdef USE_SOCKS5
	/* override the the generic "SOCKSclient" log-name */
	if (read_news_via_nntp)
		SOCKSinit(tin_progname);
#endif /* USE_SOCKS5 */

	/*
	 * Set up initial array sizes, char *'s: homedir, newsrc, etc.
	 */
	init_alloc();
	hash_init();
	init_selfinfo();
	init_group_hash();

	/*
	 * Set cCOLS temporarily to trim (localized) messages
	 * It does not matter if this value is greater than the actual
	 * terminal size
	 *
	 * cCOLS will be set later to the real terminal width
	 */
	cCOLS = 80;

	/*
	 * Process envargs & command line options
	 * These override the configured in values
	 */
	cmdargs = read_cmd_line_options(argc, argv);
	if (cmdargs != argv_orig)
		handle_cmdargs(TRUE);

	/* preinit keybindings if interactive */
	if (!batch_mode)
		setup_default_keys();

	/*
	 * Read user local & global config files
	 * These override the compiled in defaults
	 *
	 * must be called before setup_screen()
	 */
	if (!read_config_file(global_config_file, TRUE))
		global_config_file[0] = '\0';
	read_config_file(local_config_file, FALSE);

	tmp_no_write = no_write; /* keep no_write */
	no_write = TRUE;		/* don't allow any writing back during startup */

	if (!batch_mode) {
#ifndef USE_CURSES
		if (!get_termcaps()) {
			my_fprintf(stderr, _(txt_screen_init_failed), tin_progname);
			my_fprintf(stderr, "\n");
			if (!batch_mode)
				sleep(2);
			FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
			free_all_arrays();
			giveup();
		}
#endif /* !USE_CURSES */

		/* exit early at -o without postponed articles */
		if (post_postponed_and_exit && !count_postponed_articles()) {
			no_write = TRUE;
			/* TODO: looks ugly */
			error_message(0, txt_info_nopostponed);
			FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
			tin_done(EXIT_SUCCESS, NULL);
		}

		/*
		 * Init curses emulation
		 */
		if (!InitScreen()) {
			error_message(2, _(txt_screen_init_failed), tin_progname);
			FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
			free_all_arrays();
			giveup();
		}

		EndInverse();

		/*
		 * This depends on various things in tinrc
		 */
		setup_screen();
	}

	if (!batch_mode || verbose) {
		if (!batch_mode && (cLINES < MIN_LINES_ON_TERMINAL || cCOLS < MIN_COLUMNS_ON_TERMINAL)) {
			ring_bell();
			FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
			tin_done(EXIT_FAILURE, _(txt_screen_too_small_exiting), tin_progname);
		}
		wait_message(0, "%s\n", cvers);
	}

	/*
	 * Connect to nntp server?
	 */
	if (!nntp_server || !*nntp_server)
		nntp_server = getserverbyfile(NNTP_SERVER_FILE);

	open_msglog(); /* depends on nntp_server */
	read_server_config();

	if (read_news_via_nntp && !read_saved_news) {
		if (use_nntps && tintls_init()) {
			tintls_exit();
			FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
			free_all_arrays();
			giveup();
		}

		if (nntp_open()) {
			nntp_close(FALSE);
			if (use_nntps)
				tintls_exit();
			FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
			free_all_arrays();
			giveup();
		}
	}

	/*
	 * exit early - unfortunately we can't do that in read_cmd_line_options()
	 * as nntp_caps.over_cmd is set in nntp_open()
	 *
	 * without nntp_caps.over_cmd we used HEAD to fetch the data
	 * which is MUCH slower, inform the user about this (but he can't
	 * do much about it anyway)?
	 *
	 * why evaluate tinrc.cache_overview_files in error_message, we know
	 * it is false -> txt_batch_update_unavail could hold a fixed string.
	 */
	if (update_index && !tinrc.cache_overview_files) {
		error_message(2, _(txt_batch_update_unavail), tin_progname, print_boolean(tinrc.cache_overview_files));
		FREE_ARGV_IF_NEEDED(argv_orig, cmdargs);
		free_all_arrays();
		giveup();
	}

	/*
	 * Check if overview indexes contain Xref: lines
	 */
#ifdef NNTP_ABLE
	if ((read_news_via_nntp && nntp_caps.over_cmd) || !read_news_via_nntp)
#endif /* NNTP_ABLE */
		xref_supported = overview_xref_support();

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
		keymap_file[0] = '\0';
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
	 * also reads global/local attributes
	 */
	selmenu.max = 0;
	/*
	 * we need to restore the original no_write mode to be able to handle
	 * $AUTOSUBSCRIBE groups
	 */
	no_write = tmp_no_write;
	if (!read_attributes_file(TRUE))
		global_attributes_file[0] = '\0';
	read_attributes_file(FALSE);
	start_groupnum = read_news_active_file();
#ifdef DEBUG
	if (debug & DEBUG_MISC)
		debug_print_active();
#endif /* DEBUG */

	/*
	 * Read in users filter preferences file. This has to be done before
	 * quick post because the filters might be updated.
	 */
	read_filter_file(filter_file);

	no_write = TRUE;
#ifdef DEBUG
	if (debug & DEBUG_FILTER)
		debug_print_filters();
#endif /* DEBUG */

	/*
	 * Preloads active[] with command line groups. They will follow any
	 * new newsgroups
	 */
	if (!post_postponed_and_exit)
		num_cmd_line_groups = read_cmd_line_groups();

	/*
	 * Quick post an article and exit if -w or -o specified
	 */
	if (post_article_and_exit || post_postponed_and_exit) {
		no_write = tmp_no_write; /* restore original value */
		quick_post_article(post_postponed_and_exit, num_cmd_line_groups);
		wait_message(2, _(txt_exiting));
		no_write = TRUE; /* disable newsrc updates */
		tin_done(EXIT_SUCCESS, NULL);
	}

	/* TODO: replace hard coded key-name in txt_info_postponed */
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

	/* what about "if (!no_write)" here? */
	create_mail_save_dirs();
	if (created_rcdir) /* first start */
		write_config_file(local_config_file);

	if (!tmp_no_write)	/* do not (over)write oldnewsrc with -X */
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
		tin_done(check_start_save_any_news(CHECK_ANY_NEWS, catchup, num_cmd_line_groups), NULL);

	if (start_any_unread) {
		if ((start_groupnum = check_start_save_any_news(START_ANY_NEWS, catchup, num_cmd_line_groups)) == -1)
			tin_done(EXIT_SUCCESS, NULL);
	}

	/*
	 * Mail any new articles to specified user
	 * or
	 * Save any new articles to savedir structure for later reading
	 *
	 * TODO: should we temporarily set
	 *       getart_limit=-1,thread_articles=0,sort_article_type=0
	 *       for speed reasons?
	 */
	if (mail_news || save_news) {
		check_start_save_any_news(mail_news ? MAIL_ANY_NEWS : SAVE_ANY_NEWS, catchup, num_cmd_line_groups);
		tin_done(EXIT_SUCCESS, NULL);
	}

	/*
	 * Catchup newsrc file (-c option)
	 */
	if (batch_mode && catchup && !update_index) {
		catchup_newsrc_file();
		tin_done(EXIT_SUCCESS, NULL);
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
 * [01235789beEFijJKOyY] are unused
 * [W] is reserved
 * [BPU] have been in use at some time, but now are unused:
 *   B BBS mode (M_AMIGA only)
 *   P purge group index files of articles that no longer exist
 *   U update index files in background
 * reused with different function:
 *   C was count articles, now is activate COMPRESS DEFLATE
 */
#define OPTIONS "46aAcCdD:f:g:G:hHI:klL:m:M:nNop:qQrRs:St:TuvVwxXzZ"
#define TRCVAL	(envtinrc && *envtinrc ? _(txt_useless_comb_tinrcval) : "\n")
static char **
read_cmd_line_options(
	int argc,
	char *argv[])
{
	const char *envtinrc = getenv("TINRC");
	char **argv_orig = argv;
	int ch;
#if defined(NNTP_ABLE) || defined(DEBUG)
	int i;
#endif /* NNTP_ABLE || DEBUG */
#ifdef NNTP_ABLE
	t_bool newsrc_set = FALSE;
#endif /* NNTP_ABLE */

	envargs(&argc, &argv, "TINRC");

	while ((ch = getopt(argc, argv, OPTIONS)) != -1) {
		switch (ch) {
			case '4':
#if defined(NNTP_ABLE) && defined(INET6)
				force_ipv4 = TRUE;
				read_news_via_nntp = TRUE;
#else
#	ifdef NNTP_ABLE
				error_message(0, _(txt_option_not_enabled), "-DENABLE_IPV6");
#	else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
#	endif /* NNTP_ABLE */
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE && INET6 */
				break;

			case '6':
#if defined(NNTP_ABLE) && defined(INET6)
				force_ipv6 = TRUE;
				read_news_via_nntp = TRUE;
#	else
#	ifdef NNTP_ABLE
				error_message(0, _(txt_option_not_enabled), "-DENABLE_IPV6");
#	else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
#	endif /* NNTP_ABLE */
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE && INET6 */
				break;

			case 'a':
#ifdef HAVE_COLOR
				cmdline.args |= CMDLINE_USE_COLOR;
#else
				error_message(0, _(txt_option_not_enabled), "-DHAVE_COLOR");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* HAVE_COLOR */
				break;

			case 'A':
#ifdef NNTP_ABLE
				force_auth_on_conn_open = TRUE;
				read_news_via_nntp = TRUE;
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'c':
				batch_mode = TRUE;
				catchup = TRUE;
				break;

			case 'C':
#ifdef NNTP_ABLE
#	ifdef USE_ZLIB
				use_compress = TRUE;
#	else
				error_message(0, _(txt_option_not_enabled), "-DUSE_ZLIB");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#	endif /* USE_ZLIB */
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'd':
				cmdline.args |= CMDLINE_NO_DESCRIPTION;
				show_description = FALSE;
				break;

			case 'D':		/* debug mode */
#ifdef DEBUG
				i = s2i(optarg, 0, 2 * DEBUG_REMOVE - 1);
				switch (errno) {
					case EINVAL: /* TODO: extra error message */
					case ERANGE:
						error_message(0, _(txt_value_out_of_range), "-D ", (int) strtol(optarg, NULL, 10), 2 * DEBUG_REMOVE - 1);
						debug = 0;
						if (!batch_mode)
							sleep(2);
						break;

					default:
						break;
				}
				if ((debug = (unsigned short) i))
					debug_delete_files();
#else
				error_message(0, _(txt_option_not_enabled), "-DDEBUG");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* DEBUG */
				break;

			case 'f':	/* newsrc file */
				my_strncpy(newsrc, optarg, sizeof(newsrc) - 1);
#ifdef NNTP_ABLE
				newsrc_set = TRUE;
#endif /* NNTP_ABLE */
				break;

			case 'G':
				cmdline.getart_limit = s2i(optarg, INT_MIN, INT_MAX);
				/* TODO: add EINVAL/ERANGE error messages here */
				if (cmdline.getart_limit != 0)
					cmdline.args |= CMDLINE_GETART_LIMIT;
				break;

			case 'g':	/* select alternative NNTP-server, implies -r */
#ifdef NNTP_ABLE
				cmdline.nntpserver = my_strdup(optarg);
				str_trim(cmdline.nntpserver);
				{
					char *p;

					if ((p = strchr(cmdline.nntpserver, ':')) != NULL) {
						if (*cmdline.nntpserver != '[' && strrchr(cmdline.nntpserver, ':') == p) { /* exact 1 x ':' must be name:port or ipv4:port */
							*p++ = '\0';
							i = s2i(p, 0, 65535);
							switch (errno) {
								case EINVAL:
									error_message(0, _(txt_port_not_numeric), cmdline.nntpserver, p);
									FREE_ARGV_IF_NEEDED(argv_orig, argv);
									free_all_arrays();
									giveup();
									/* keep lint quiet: */
									/* FALLTHROUGH */
									break;

								case ERANGE:
									error_message(0, _(txt_value_out_of_range), "-g ", (int) strtol(optarg, NULL, 10), 65535);
									if (!batch_mode)
										sleep(2);
									break;

								default:
									nntp_tcp_port = (unsigned short) i;
									break;
							}
						} else /* "[ipv6]"[:port] */
#	ifndef INET6
						{
							error_message(0, _(txt_option_not_enabled), "-DENABLE_IPV6");
							FREE_ARGV_IF_NEEDED(argv_orig, argv);
							free_all_arrays();
							giveup();
						}
#	else
						{
							char *q;
							int j = 0;

							if (*cmdline.nntpserver == '[' && (q = strrchr(cmdline.nntpserver, ']')) != NULL) {
								if ((p = strchr(cmdline.nntpserver, ':')) != NULL) {
									if (p > q) { /* not an IPv6 literal (e.g. [host.name]:port or [i.p.v.4]:port, must not be in []) */
										error_message(0, _(txt_error_not_ipv6_literal), cmdline.nntpserver);
										FREE_ARGV_IF_NEEDED(argv_orig, argv);
										free_all_arrays();
										giveup();
									}
								}
								if ((p = strchr(q, ':')) != NULL) {
									if (p == q + 1) {
										i = s2i(++p, 0, 65535);
										switch (errno) {
											case EINVAL:
												*++q = '\0'; /* remove tailing ':' */
												error_message(0, _(txt_port_not_numeric), cmdline.nntpserver, p);
												FREE_ARGV_IF_NEEDED(argv_orig, argv);
												free_all_arrays();
												giveup();
												/* keep lint quiet: */
												/* FALLTHROUGH */
												break;

											case ERANGE:
												error_message(0, _(txt_value_out_of_range), "-g ", (int) strtol(optarg, NULL, 10), 65535);
												if (!batch_mode)
													sleep(2);
												break;

											default:
												nntp_tcp_port = (unsigned short) i;
												break;
										}
									}
								}
								*q = '\0'; /* remove tailing ']' */
								for (i = 1; cmdline.nntpserver[i] != '\0'; i++) { /* remove leading '[' */
									cmdline.nntpserver[i - 1] = cmdline.nntpserver[i];
									/* minimal IPv6 syntax checking - just if we didn't find inet_pton() */
									if (cmdline.nntpserver[i] == ':')
										j++;
									else {
										if (!IS_XDIGIT(cmdline.nntpserver[i]))
											j += 1000;
									}
								}
								cmdline.nntpserver[i - 1] = '\0';
								str_trim(cmdline.nntpserver);
								{
#		if defined(HAVE_INET_PTON) && defined(AF_INET6)
									struct in_addr inaddr;

									(void) j; /* silence unused-but-set */
									if (inet_pton(AF_INET6, cmdline.nntpserver, &inaddr) != 1)
#		else
									if (j < 2 || j > 7)
#		endif /* HAVE_INET_PTON && AF_INET6 */
									{
										error_message(0, _(txt_error_not_ipv6_literal), cmdline.nntpserver);
										FREE_ARGV_IF_NEEDED(argv_orig, argv);
										free_all_arrays();
										giveup();
									}
								}
							}
						}
#	endif /* !INET6 */
					}
				}
				if (!*cmdline.nntpserver)
					FreeAndNull(cmdline.nntpserver);
				read_news_via_nntp = TRUE;
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'H':
				show_intro_page();
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				exit(EXIT_SUCCESS);
				/* keep lint quiet: */
				/* NOTREACHED */
				break;

			case 'I':
				joinpath(index_newsdir, sizeof(index_newsdir), optarg, INDEX_NEWSDIR);
				break;

			case 'k':
#ifdef NNTPS_ABLE
				insecure_nntps = TRUE;
				use_nntps = TRUE;
#else
				error_message(0, _(txt_option_not_enabled), "--with-nntps");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTPS_ABLE */
				break;

			case 'l':
				list_active = TRUE;
				break;

			case 'L':
#ifdef NNTP_ABLE
				cmdline.msgid = my_strdup(optarg);
				str_trim(cmdline.msgid);
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE */
				break;

			case 'm':
				cmdline.maildir = my_strdup(optarg);
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
				error_message(0, _(txt_option_not_enabled), "-UNO_POSTING");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* !NO_POSTING */
				break;

			case 'p': /* implies -r */
#ifdef NNTP_ABLE
				i = s2i(optarg, 0, 65535);

				switch (errno) {
					case EINVAL:
						error_message(0, _(txt_port_not_numeric), "", optarg);
						FREE_ARGV_IF_NEEDED(argv_orig, argv);
						free_all_arrays();
						giveup();
						/* keep lint quiet: */
						/* FALLTHROUGH */
						break;

					case ERANGE:
						error_message(0, _(txt_value_out_of_range), "-p ", (int) strtol(optarg, NULL, 10), 65535);
						if (!batch_mode)
							sleep(2);
						break;

					default:
						nntp_tcp_port = (unsigned short) i;
						break;
				}
				read_news_via_nntp = TRUE;
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
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
				cmdline.args |= CMDLINE_NO_DESCRIPTION;
				break;

			case 'r':	/* read news remotely from default NNTP server */
#ifdef NNTP_ABLE
				read_news_via_nntp = TRUE;
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
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
				cmdline.savedir = my_strdup(optarg);
				break;

			case 'S':	/* save new news to dir structure */
				save_news = TRUE;
				batch_mode = TRUE;
				break;

			case 't':
#if defined(NNTP_ABLE) && defined(HAVE_ALARM) && defined(SIGALRM)
				cmdline.nntp_timeout = s2i(optarg, 0, TIN_NNTP_TIMEOUT_MAX);
				switch (errno) {
					case EINVAL: /* TODO: extra error message */
					case ERANGE:
						error_message(0, _(txt_value_out_of_range), "-t ", (int) strtol(optarg, NULL, 10), TIN_NNTP_TIMEOUT_MAX);
						if (!batch_mode)
							sleep(2);
						break;

					default:
						break;
				}
				if (cmdline.nntp_timeout)
					cmdline.args |= CMDLINE_NNTP_TIMEOUT;
#else
				error_message(0, _(txt_option_not_enabled), "-DNNTP_ABLE");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTP_ABLE && HAVE_ALARM && SIGALRM */
				break;

			case 'T':
#ifdef NNTPS_ABLE
				use_nntps = TRUE;
#else
				error_message(0, _(txt_option_not_enabled), "--with-nntps");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				giveup();
				/* keep lint quiet: */
				/* NOTREACHED */
#endif /* NNTPS_ABLE */
				break;

			case 'u':	/* update index files */
				batch_mode = TRUE;
				update_index = TRUE;
				break;

			case 'v':	/* verbose mode, can be used multiple times */
				verbose++;
				break;

			case 'V':
				tin_version_info(stderr, verbose);
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				exit(EXIT_SUCCESS);
				/* keep lint quiet: */
				/* NOTREACHED */
				break;

			case 'w':	/* post article & exit */
#ifndef NO_POSTING
				post_article_and_exit = TRUE;
				check_for_new_newsgroups = FALSE;
#else
				error_message(0, _(txt_option_not_enabled), "-UNO_POSTING");
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
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
				FREE_ARGV_IF_NEEDED(argv_orig, argv);
				free_all_arrays();
				exit(EXIT_SUCCESS);
		}
	}

#ifdef NNTP_ABLE
	if (nntp_tcp_port == 0) {
#	ifdef NNTPS_ABLE
		if (use_nntps)
			nntp_tcp_port = nntps_tcp_default_port;
		else
#	endif /* NNTPS_ABLE */
			nntp_tcp_port = nntp_tcp_default_port;
	}
#endif /* NNTP_ABLE */

	/* cmdargs = argv; */
	num_cmdargs = optind;
	max_cmdargs = argc;

#ifdef NNTP_ABLE
	if (!newsrc_set) { /* this gives "-f" a higher priority (in contrast to -p) */
		if (read_news_via_nntp) {
			nntp_server = getserverbyfile(NNTP_SERVER_FILE);
			get_newsrcname(newsrc, sizeof(newsrc), nntp_server);
		} else {
			const char *nodenamebuf = get_host_name();

			get_newsrcname(newsrc, sizeof(newsrc), nodenamebuf);
		}
	}
#endif /* NNTP_ABLE */

	/*
	 * Sort out option conflicts
	 */
	if (!batch_mode) {
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
	if (!(batch_mode || debug)) {
		if (verbose) {
			wait_message(2, _(txt_useful_with_batch_or_debug_mode), "-v");
			verbose = FALSE;
		}
	}
	/*
	 * TODO: also disallow
	 *       -NM
	 *       -oN, -oM (at this stage we no longer know if -N or -M was given)
	 *       -wN, -wM (at this stage we no longer know if -N or -M was given)
	 *       -NZ, -MZ (at this stage we no longer know if -N or -M was given)
	 *       -LM, -LN (at this stage we no longer know if -N or -M was given)
	 *       (extend t_cmdlineopts with a flag indicating which of -M/-N was given)
	 *       -uo, -uw, -uX, ...
	 *
	 *      if combination comes from $TINRC (+ cmd.-line), mention $TINRC value.
	 */
	if (post_postponed_and_exit && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-o", "-L", "-L", TRCVAL);
		FreeAndNull(cmdline.msgid);
	}
	if (post_postponed_and_exit && force_no_post) {
		wait_message(2, _(txt_useless_combination), "-o", "-x", "-x", TRCVAL);
		force_no_post = FALSE;
	}
	if (post_postponed_and_exit && start_any_unread) {
		wait_message(2, _(txt_useless_combination), "-o", "-z", "-z", TRCVAL);
		start_any_unread = FALSE;
	}
	if (post_postponed_and_exit && post_article_and_exit) {
		wait_message(2, _(txt_useless_combination), "-o", "-w", "-w", TRCVAL);
		post_article_and_exit = FALSE;
	}
	if (post_article_and_exit && force_no_post) {
		wait_message(2, _(txt_useless_combination), "-w", "-x", "-x", TRCVAL);
		force_no_post = FALSE;
	}
	if (post_article_and_exit && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-w", "-L", "-w", TRCVAL);
		post_article_and_exit = FALSE;
	}
	if (post_article_and_exit && start_any_unread) {
		wait_message(2, _(txt_useless_combination), "-w", "-z", "-z", TRCVAL);
		start_any_unread = FALSE;
	}
	if (catchup && start_any_unread) {
		wait_message(2, _(txt_useless_combination), "-c", "-z", "-c", TRCVAL);
		catchup = FALSE;
	}
	if (catchup && no_write) {
		wait_message(2, _(txt_useless_combination), "-c", "-X", "-c", TRCVAL);
		catchup = FALSE;
	}
	if (catchup && check_any_unread) {
		wait_message(2, _(txt_useless_combination), "-c", "-Z", "-c", TRCVAL);
		catchup = FALSE;
	}
	if (catchup && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-c", "-L", "-c", TRCVAL);
		catchup = FALSE;
	}
	if (update_index && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-u", "-L", "-u", TRCVAL);
		update_index = FALSE;
	}
	if (newsrc_active && read_saved_news) {
		wait_message(2, _(txt_useless_combination), "-n", "-R", "-n", TRCVAL);
		newsrc_active = read_news_via_nntp = FALSE;
	}
	if (read_saved_news && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-R", "-L", "-R", TRCVAL);
		read_saved_news = FALSE;
	}
	if (save_news && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-S", "-L", "-S", TRCVAL);
		save_news = FALSE;
	}
	if (start_any_unread && save_news) {
		wait_message(2, _(txt_useless_combination), "-z", "-S", "-z", TRCVAL);
		start_any_unread = FALSE;
	}
	if (start_any_unread && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-z", "-L", "-z", TRCVAL);
		start_any_unread = FALSE;
	}
	if (save_news && check_any_unread) {
		wait_message(2, _(txt_useless_combination), "-S", "-Z", "-S", TRCVAL);
		save_news = FALSE;
	}
	if (check_any_unread && start_any_unread) {
		wait_message(2, _(txt_useless_combination), "-Z", "-z", "-Z", TRCVAL);
		check_any_unread = FALSE;
	}
	if (check_any_unread && cmdline.msgid) {
		wait_message(2, _(txt_useless_combination), "-Z", "-L", "-Z", TRCVAL);
		check_any_unread = FALSE;
	}
#ifdef DEBUG
#	ifdef NNTP_ABLE
	if ((debug & DEBUG_NNTP) && !read_news_via_nntp)
#		else
	if (debug & DEBUG_NNTP)
#	endif /* NNTP_ABLE */
	{ /* TODO: fix translation unfriendly construct */
		wait_message(3, _(txt_useless_combination), _(txt_reading_from_spool), "-D nntp", "-D nntp", TRCVAL);
		debug ^= DEBUG_NNTP;
	}
#endif /* DEBUG */

#if defined(NNTP_ABLE) && defined(INET6)
	if (force_ipv4 && force_ipv6) {
		wait_message(2, _(txt_useless_combination), "-4", "-6", "-6", TRCVAL); /* TODO: -> lang.c */
		force_ipv6 = FALSE;
	}
#endif /* NNTP_ABLE && INET6 */

	if (mail_news || save_news || update_index || check_any_unread || catchup)
		batch_mode = TRUE;
	else
		batch_mode = FALSE;
	if (batch_mode && (post_article_and_exit || post_postponed_and_exit))
		batch_mode = FALSE;

	/*
	 * When updating index files set getart_limit to 0 in order to get overview
	 * information for all article; this overwrites '-G limit' and disables
	 * tinrc.getart_limit temporary
	 */
	if (update_index) {
		cmdline.getart_limit = 0;
		cmdline.args |= CMDLINE_GETART_LIMIT;
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

	return argv;
}
#undef TRCVAL


/*
 * usage
 */
static void
usage(
	char *theProgname)
{
	error_message(0, _(txt_usage_tin), theProgname);

#if defined(NNTP_ABLE) && defined(INET6)
	error_message(0, _(txt_usage_force_ipv4));
	error_message(0, _(txt_usage_force_ipv6));
#endif /* NNTP_ABLE && INET6 */

#ifdef HAVE_COLOR
	error_message(0, _(txt_usage_toggle_color));
#endif /* HAVE_COLOR */
#ifdef NNTP_ABLE
	error_message(0, _(txt_usage_force_authentication));
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_catchup));
#if defined(NNTP_ABLE) && defined(USE_ZLIB)
	error_message(0, _(txt_usage_compress));
#endif /* NNTP_ABLE && USE_ZLIB */
	error_message(0, _(txt_usage_dont_show_descriptions));

#ifdef DEBUG
	error_message(0, _(txt_usage_debug));
#endif /* DEBUG */

	error_message(0, _(txt_usage_newsrc_file), newsrc);
	error_message(0, _(txt_usage_getart_limit));

#ifdef NNTP_ABLE
#	ifdef NNTP_DEFAULT_SERVER
	error_message(0, _(txt_usage_newsserver), get_val("NNTPSERVER", NNTP_DEFAULT_SERVER));
#	else
	error_message(0, _(txt_usage_newsserver), get_val("NNTPSERVER", "news"));
#	endif /* NNTP_DEFAULT_SERVER */
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_help_message));
	error_message(0, _(txt_usage_help_information), theProgname);
	error_message(0, _(txt_usage_index_newsdir), index_newsdir);

#ifdef NNTP_ABLE
#	ifdef NNTPS_ABLE
	error_message(0, _(txt_usage_use_insecure_nntps));
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_read_only_active));

#ifdef NNTP_ABLE
	error_message(0, _(txt_usage_lookup_id));
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_maildir), tinrc.maildir);
	error_message(0, _(txt_usage_mail_new_news_to_user));
	error_message(0, _(txt_usage_read_only_subscribed));
	error_message(0, _(txt_usage_mail_new_news));
	error_message(0, _(txt_usage_post_postponed_arts));

#ifdef NNTP_ABLE
#	ifdef NNTPS_ABLE
	error_message(0, _(txt_usage_port), use_nntps ? nntps_tcp_default_port : nntp_tcp_default_port);
#	else
	error_message(0, _(txt_usage_port), nntp_tcp_default_port);
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_dont_check_new_newsgroups));
	error_message(0, _(txt_usage_quickstart));

#ifdef NNTP_ABLE
	if (!read_news_via_nntp)
		error_message(0, _(txt_usage_read_news_remotely));
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_read_saved_news));
	error_message(0, _(txt_usage_savedir), tinrc.savedir);
	error_message(0, _(txt_usage_save_new_news));

#ifdef NNTP_ABLE
#	if defined(HAVE_ALARM) && defined(SIGALRM)
	error_message(0, _(txt_usage_nntp_timeout), TIN_NNTP_TIMEOUT);
#	endif /* HAVE_ALARM && SIGALRM */
#	ifdef NNTPS_ABLE
	error_message(0, _(txt_usage_use_nntps));
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */

	error_message(0, _(txt_usage_update_index_files));
	error_message(0, _(txt_usage_verbose));
	error_message(0, _(txt_usage_version));
	error_message(0, _(txt_usage_post_article));
	error_message(0, _(txt_usage_no_posting));
	error_message(0, _(txt_usage_dont_save_files_on_quit));
	error_message(0, _(txt_usage_start_if_unread_news));
	error_message(0, _(txt_usage_check_for_unread_news));

	error_message(0, _(txt_usage_mail_bugreport), bug_addr);
}


/*
 * update index files
 */
_Noreturn static void
update_index_files(
	void)
{
	cCOLS = 132;							/* set because curses has not started */
	create_index_lock_file(lock_file);
	tinrc.thread_articles = THREAD_NONE;	/* stop threading to run faster */
	tinrc.sort_article_type = SORT_ARTICLES_BY_NOTHING;
	tinrc.sort_threads_type = SORT_THREADS_BY_NOTHING;
	do_update(catchup);
	tin_done(EXIT_SUCCESS, NULL);
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
					if (my_group_add(active[i].name, TRUE) != -1) {
						matched++;
						if (post_article_and_exit) {
							FreeIfNeeded(tinrc.default_post_newsgroups);
							tinrc.default_post_newsgroups = my_strdup(active[i].name);
							break;
						}
						active[i].read_during_session = TRUE; /* misuse for "-[zZMN] grp" */
					}
				}
			}
		}
	}
	return matched;
}


/*
 * Create default mail & save directories if they do not exist
 */
static void
create_mail_save_dirs(
	void)
{
	char path[PATH_LEN];
	struct stat sb;

	if (!strfpath(tinrc.maildir, path, sizeof(path), NULL, FALSE))
		joinpath(path, sizeof(path), homedir, DEFAULT_MAILDIR);

	if (stat(path, &sb) == -1) {
		if (my_mkdir(path, (mode_t) (S_IRWXU)) == -1)
			error_message(2, _(txt_cannot_create), path);
	}

	if (!strfpath(tinrc.savedir, path, sizeof(path), NULL, FALSE))
		joinpath(path, sizeof(path), homedir, DEFAULT_SAVEDIR);

	if (stat(path, &sb) == -1) {
		if (my_mkdir(path, (mode_t) (S_IRWXU)) == -1)
			error_message(2, _(txt_cannot_create), path);
	}
}


/*
 * TODO: find a better solution to free() cmdargs from outside main.c
 */
void
handle_cmdargs(
	t_bool init)
{
	static t_bool argv_modified = FALSE;

	if (init)
		argv_modified = TRUE;
	else if (argv_modified) {
		if (cmdargs) {
			free(*(cmdargs + 1));
			free(cmdargs);
			cmdargs = NULL;
		}
	}
}


/*
 * we don't try do free() any previously malloc()ed mem here as exit via
 * giveup() indicates a serious error and keeping track of what we've
 * already malloc()ed would be a PITA.
 */
/* coverity[+kill] */
_Noreturn void
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
	close_msglog();

	exit(EXIT_FAILURE);
}
