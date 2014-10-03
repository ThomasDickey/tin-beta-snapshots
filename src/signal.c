/*
 *  Project   : tin - a Usenet reader
 *  Module    : signal.c
 *  Author    : I.Lea
 *  Created   : 1991-04-01
 *  Updated   : 2011-09-07
 *  Notes     : signal handlers for different modes and window resizing
 *
 * Copyright (c) 1991-2014 Iain Lea <iain@bricbrac.de>
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
#ifndef included_trace_h
#	include "trace.h"
#endif /* !included_trace_h */
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */

/*
 * Needed for resizing under an xterm
 */
#ifdef HAVE_TERMIOS_H
#	include <termios.h>
#else
#	ifdef HAVE_TERMIO_H
#		include <termio.h>
#	endif /* HAVE_TERMIO_H */
#endif /* HAVE_TERMIOS_H */

#ifdef NEED_PTEM_H
#	include <sys/stream.h>
#	include <sys/ptem.h>
#endif /* NEED_PTEM_H */

#if defined(SIGWINCH) && !defined(DONT_HAVE_SIGWINCH)
#	if !defined(TIOCGWINSZ) && !defined(TIOCGSIZE)
#		ifdef HAVE_SYS_STREAM_H
#			include <sys/stream.h>
#		endif /* HAVE_SYS_STREAM_H */
#		ifdef HAVE_SYS_PTY_H
#			if !defined(_h_BSDTYPES) && defined(HAVE_SYS_BSDTYPES_H)
#				include <sys/bsdtypes.h>
#			endif /* !_h_BSDTYPES && HAVE_SYS_BSDTYPES_H */
#			include <sys/pty.h>
#		endif /* HAVE_SYS_PTY_H */
#	endif /* !TIOCGWINSZ && !TIOCGSIZE */
#endif /* SIGWINCH && !DONT_HAVE_SIGWINCH */

#ifdef MINIX
#	undef SIGTSTP
#endif /* MINIX */

/*
 * local prototypes
 */
static const char *signal_name(int code);
#ifdef SIGTSTP
	static void handle_suspend(void);
#endif /* SIGTSTP */
static void _CDECL signal_handler(SIG_ARGS);


#ifdef SIGTSTP
	static t_bool do_sigtstp = FALSE;
#endif /* SIGTSTP */
#if defined(SIGWINCH) || defined(SIGTSTP)
	static t_bool redraw_after_suspend;
#endif /* SIGWINCH || SIGTSTP */

int signal_context = cMain;
int input_context = cNone;
int need_resize = cNo;
/*
 * # lines of non-static data available for display
 */
int NOTESLINES;


#ifndef __LCLINT__ /* lclint doesn't like it */
static const struct {
	int code;
	const char *name;
} signal_list[] = {
#	ifdef SIGINT
	{ SIGINT,	"SIGINT" },	/* ctrl-C */
#	endif /* SIGINT */
#	ifdef SIGQUIT
	{ SIGQUIT,	"SIGQUIT" },	/* ctrl-\ */
#	endif /* SIGQUIT */
#	ifdef SIGILL
	{ SIGILL,	"SIGILL" },	/* illegal instruction */
#	endif /* SIGILL */
#	ifdef SIGFPE
	{ SIGFPE,	"SIGFPE" },	/* floating point exception */
#	endif /* SIGFPE */
#	ifdef SIGBUS
	{ SIGBUS,	"SIGBUS" },	/* bus error */
#	endif /* SIGBUS */
#	ifdef SIGSEGV
	{ SIGSEGV,	"SIGSEGV" },	/* segmentation violation */
#	endif /* SIGSEGV */
#	ifdef SIGPIPE
	{ SIGPIPE,	"SIGPIPE" },	/* broken pipe */
#	endif /* SIGPIPE */
#	ifdef SIGALRM
	{ SIGALRM,	"SIGALRM" },	/* real-time timer expired */
#	endif /* SIGALRM */
#	ifdef SIGCHLD
	{ SIGCHLD,	"SIGCHLD" },	/* death of a child process */
#	endif /* SIGCHLD */
#	ifdef SIGPWR
	{ SIGPWR,	"SIGPWR" },	/* powerfail */
#	endif /* SIGPWR */
#	ifdef SIGTSTP
	{ SIGTSTP,	"SIGTSTP" },	/* terminal-stop */
#	endif /* SIGTSTP */
#	ifdef SIGHUP
	{ SIGHUP,	"SIGHUP" },	/* hang up */
#	endif /* SIGHUP */
#	ifdef SIGUSR1
	{ SIGUSR1,	"SIGUSR1" },	/* User-defined signal 1 */
#	endif /* SIGUSR1 */
#	ifdef SIGUSR2
	{ SIGUSR2,	"SIGUSR2" },	/* User-defined signal 2 */
#	endif /* SIGUSR2 */
#	ifdef SIGTERM
	{ SIGTERM,	"SIGTERM" },	/* termination */
#	endif /* SIGTERM */
#	if defined(SIGWINCH) && !(defined(USE_CURSES) && defined(KEY_RESIZE))
	{ SIGWINCH,	"SIGWINCH" },	/* window-size change */
#	endif /* SIGWINCH && !(USE_CURSES && KEY_RESIZE) */
};
#endif /* !__LCLINT__ */


#ifdef HAVE_NESTED_PARAMS
	RETSIGTYPE (*sigdisp(int signum, RETSIGTYPE (_CDECL *func)(SIG_ARGS)))(SIG_ARGS)
#else
	RETSIGTYPE (*sigdisp(signum, func))(SIG_ARGS)
	int signum;
	RETSIGTYPE (_CDECL *func)(SIG_ARGS);
#endif /* HAVE_NESTED_PARAMS */
{
#ifdef HAVE_POSIX_JC
#	define RESTORE_HANDLER(x, y)
	struct sigaction sa, osa;

	sa.sa_handler = func;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
#	ifdef SA_RESTART
		sa.sa_flags |= SA_RESTART;
#	endif /* SA_RESTART */
	if (sigaction(signum, &sa, &osa) < 0)
		return SIG_ERR;
	return (osa.sa_handler);
#else
#	define RESTORE_HANDLER(x, y)	signal(x, y)
	return (signal(signum, func));
#endif /* HAVE_POSIX_JC */
}


/*
 * Block/unblock SIGWINCH/SIGTSTP restarting syscalls
 */
void
allow_resize(
	t_bool allow)
{
#ifdef HAVE_POSIX_JC
	struct sigaction sa, osa;

	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
#	ifdef SA_RESTART
	if (!allow)
		sa.sa_flags |= SA_RESTART;
#	endif /* SA_RESTART */
#	if defined(SIGWINCH) && !(defined(USE_CURSES) && defined(KEY_RESIZE))
	sigaction(SIGWINCH, &sa, &osa);
#	endif /* SIGWINCH && !(USE_CURSES && KEY_RESIZE) */
#	ifdef SIGTSTP
	sigaction(SIGTSTP, &sa, &osa);
#	endif /* SIGTSTP */
#endif /* HAVE_POSIX_JC */
}


static const char *
signal_name(
	int code)
{
	size_t n;
	const char *name = "unknown";

	for (n = 0; n < ARRAY_SIZE(signal_list); n++) {
		if (signal_list[n].code == code) {
			name = signal_list[n].name;
			break;
		}
	}
	return name;
}


/*
 * Rescale the display buffer and redraw the contents according to
 * the current context
 * This should NOT be called from an interrupt context
 */
void
handle_resize(
	t_bool repaint)
{
#if defined(SIGWINCH) || defined(SIGTSTP)
#	ifdef SIGWINCH
	repaint |= set_win_size(&cLINES, &cCOLS);
#	endif /* SIGWINCH */

	if (cLINES < MIN_LINES_ON_TERMINAL || cCOLS < MIN_COLUMNS_ON_TERMINAL) {
		ring_bell();
		wait_message(3, _(txt_screen_too_small_exiting), tin_progname);
		tin_done(EXIT_FAILURE);
	}

	TRACE(("handle_resize(%d:%d)", signal_context, repaint));

	if (!repaint)
		return;

#	ifdef USE_CURSES
#		ifdef HAVE_RESIZETERM
	resizeterm(cLINES + 1, cCOLS);
	my_retouch();					/* seems necessary if win size unchanged */
#		else
	my_retouch();
#		endif /* HAVE_RESIZETERM */
#	endif /* USE_CURSES */

	switch (signal_context) {
		case cArt:
			ClearScreen();
			show_art_msg(CURR_GROUP.name);
			break;

		case cAttrib:
		case cConfig:
			refresh_config_page(SIGNAL_HANDLER);
			break;

		case cFilter:
			refresh_filter_menu();
			break;

		case cInfopager:
			display_info_page(0);
			break;

		case cAttachment:
		case cGroup:
		case cScope:
		case cSelect:
		case cThread:
		case cURL:
			ClearScreen();
			currmenu->redraw();
			break;

		case cPage:
			resize_article(TRUE, &pgart);
			draw_page(curr_group->name, 0);
			break;

		case cPost:
		case cPostCancel:
			refresh_post_screen(signal_context);
			break;

		case cPostFup:
			resize_article(TRUE, &pgart);
			draw_page(curr_group->name, 0);
			/*
			 * Reset signal_context because draw_page()
			 * sets signal_context to cPage.
			 */
			signal_context = cPostFup;
			refresh_post_screen(signal_context);
			break;

		case cReconnect:
			ClearScreen();
			show_title(tin_progname);
			break;

		case cMain:
			break;
	}
	switch (input_context) {
		case cGetline:
			gl_redraw();
			break;

		case cPromptCONT:
			if (redraw_after_suspend)
				info_message(_(txt_return_key));
			break;

		case cPromptSLK:
			prompt_slk_redraw();
			break;

		case cPromptYN:
			prompt_yn_redraw();
			break;

		default:
			break;
	}
	my_fflush(stdout);
	redraw_after_suspend = FALSE;
#endif /* SIGWINCH || SIGTSTP */
}


#ifdef SIGTSTP
static void
handle_suspend(
	void)
{
	t_bool save_cmd_line = cmd_line;
	t_bool save_state = (!batch_mode || !cmd_line);

	TRACE(("handle_suspend(%d)", signal_context));

	set_keypad_off();
	if (!cmd_line)
		set_xclick_off();

	if (save_state) {
		EndWin();
		Raw(FALSE);
	}

	wait_message(0, _(txt_suspended_message), tin_progname);

	kill(0, SIGSTOP);				/* Put ourselves to sleep */

	RESTORE_HANDLER(SIGTSTP, signal_handler);

	if (save_state) {
		Raw(TRUE);
		InitWin();
		cmd_line = save_cmd_line;
		if (!cmd_line)
			my_retouch();
		need_resize = cRedraw;		/* Flag a redraw */
		redraw_after_suspend = TRUE;
	}
	set_keypad_on();
	if (!cmd_line)
		set_xclick_on();
}
#endif /* SIGTSTP */


static void _CDECL
signal_handler(
	int sig)
{
#ifdef SIGCHLD
#	ifdef HAVE_TYPE_UNIONWAIT
	union wait wait_status;
#	else
	int wait_status = 1;
#	endif /* HAVE_TYPE_UNIONWAIT */
#endif /* SIGCHLD */

	/* In this case statement, we handle only the non-fatal signals */
	switch (sig) {
#ifdef SIGINT
		case SIGINT:
			RESTORE_HANDLER(sig, signal_handler);
			return;
#endif /* SIGINT */

/*
 * fatal error but we don't want the "signal handler caught signal"
 * message here
 */
#if defined(HAVE_ALARM) && defined(SIGALRM)
		case SIGALRM:
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "get_server() %d sec elapsed without response", tinrc.nntp_read_timeout_secs);
#	endif /* DEBUG */
			error_message(2, "NNTP connection error. Exiting...");
			tin_done(NNTP_ERROR_EXIT);
			return;
#endif /* HAVE_ALARM && SIGALRM */

#ifdef SIGCHLD
		case SIGCHLD:
			wait(&wait_status);
			RESTORE_HANDLER(sig, signal_handler);	/* death of a child */
			system_status = WIFEXITED(wait_status) ? WEXITSTATUS(wait_status) : 0;
			return;
#endif /* SIGCHLD */

#ifdef SIGTSTP
		case SIGTSTP:
			handle_suspend();
			return;
#endif /* SIGTSTP */

#ifdef SIGWINCH
		case SIGWINCH:
			need_resize = cYes;
			RESTORE_HANDLER(sig, signal_handler);
			return;
#endif /* SIGWINCH */

#ifdef SIGUSR2
		case SIGUSR2:
			if (!no_write) /* TODO: add more config-files to be saved */
				write_newsrc();
			RESTORE_HANDLER(sig, signal_handler);
			return;
#endif /* SIGUSR2 */

		default:
			break;
	}

	fprintf(stderr, "\n%s: signal handler caught %s signal (%d).\n", tin_progname, signal_name(sig), sig);

	switch (sig) {
#ifdef SIGHUP
		case SIGHUP:
#endif /* SIGHUP */
#ifdef SIGUSR1
		case SIGUSR1:
#endif /* SIGUSR1 */
#ifdef SIGTERM
		case SIGTERM:
#endif /* SIGTERM */
#if defined(SIGHUP) || defined(SIGUSR1) || defined(SIGTERM)
			dangerous_signal_exit = TRUE;
			tin_done(-sig);
			/* NOTREACHED */
			break;
#endif /* SIGHUP || SIGUSR1 || SIGTERM */

#ifdef SIGSEGV
		case SIGSEGV:
#	if defined(SIGBUS) && (SIGSEGV != SIGBUS) /* on Haiku SIGSEGV == SIGBUS */
		case SIGBUS:
#	endif /* SIGBUS && SIGSEGV != SIGBUS */
#else
#	ifdef SIGBUS
		case SIGBUS:
#	endif /* SIGBUS */
#endif /* SIGSEGV */

#if defined(SIGBUS) || defined(SIGSEGV)
			my_fprintf(stderr, _(txt_send_bugreport), tin_progname, VERSION, RELEASEDATE, RELEASENAME, OSNAME, bug_addr);
			my_fflush(stderr);
			break;
#endif /* SIGBUS || SIGSEGV */

		default:
			break;
	}

	cleanup_tmp_files();

#if 1
/* #if defined(apollo) || defined(HAVE_COREFILE) */
	/* do this so we can get a traceback (doesn't dump core) */
	abort();
#else
	giveup();
#endif /* 1 */ /* apollo || HAVE_COREFILE */
}


/*
 * Turn on (flag != FALSE) our signal handler for TSTP and WINCH
 * Otherwise revert to the default handler
 */
void
set_signal_catcher(
	int flag)
{
#ifdef SIGTSTP
	if (do_sigtstp)
		sigdisp(SIGTSTP, flag ? signal_handler : SIG_DFL);
#endif /* SIGTSTP */

#if defined(SIGWINCH) && !(defined(USE_CURSES) && defined(KEY_RESIZE))
	sigdisp(SIGWINCH, flag ? signal_handler : SIG_DFL);
#endif /* SIGWINCH && !(USE_CURSES && KEY_RESIZE) */
}


void
set_signal_handlers(
	void)
{
	size_t n;
	int code;
#ifdef SIGTSTP
	RETSIGTYPE (*ptr)(SIG_ARGS);
#endif /* SIGTSTP */

	for (n = 0; n < ARRAY_SIZE(signal_list); n++) {
		switch ((code = signal_list[n].code)) {
#ifdef SIGPIPE
		case SIGPIPE:
			sigdisp(code, SIG_IGN);
			break;
#endif /* SIGPIPE */
#ifdef SIGTSTP
		case SIGTSTP:
			ptr = sigdisp(code, SIG_DFL);
			sigdisp(code, ptr);
			if (ptr == SIG_IGN)
				break;
			/*
			 * SIGTSTP is ignored when starting from shells
			 * without job-control
			 */
			do_sigtstp = TRUE;
			/* FALLTHROUGH */
#endif /* SIGTSTP */

		default:
			sigdisp(code, signal_handler);
		}
	}
}


/*
 * Size the display at startup or rescale following a SIGWINCH etc.
 */
t_bool
set_win_size(
	int *num_lines,
	int *num_cols)
{
	int old_lines;
	int old_cols;
#ifdef TIOCGSIZE
	struct ttysize win;
#else
#	ifdef TIOCGWINSZ
	struct winsize win;
#	endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */

	old_lines = *num_lines;
	old_cols = *num_cols;

#ifdef HAVE_XCURSES
	*num_lines = LINES - 1;		/* FIXME */
	*num_cols = COLS;
#else /* curses/ncurses */

#	ifndef USE_CURSES
	init_screen_array(FALSE);		/* deallocate screen array */
#	endif /* !USE_CURSES */

#	ifdef TIOCGSIZE
	if (ioctl(0, TIOCGSIZE, &win) == 0) {
		if (win.ts_lines != 0)
			*num_lines = win.ts_lines - 1;
		if (win.ts_cols != 0)
			*num_cols = win.ts_cols;
	}
#	else
#		ifdef TIOCGWINSZ
	if (ioctl(0, TIOCGWINSZ, &win) == 0) {
		if (win.ws_row != 0)
			*num_lines = win.ws_row - 1;
		if (win.ws_col != 0)
			*num_cols = win.ws_col;
	}
#		else
#		endif /* TIOCGWINSZ */
#	endif /* TIOCGSIZE */

#	ifndef USE_CURSES
	init_screen_array(TRUE);		/* allocate screen array for resize */
#	endif /* !USE_CURSES */

#endif /* HAVE_XCURSES */

	set_noteslines(*num_lines);
	return (*num_lines != old_lines || *num_cols != old_cols);
}


void
set_noteslines(
	int num_lines)
{
	NOTESLINES = num_lines - INDEX_TOP - (tinrc.beginner_level ? MINI_HELP_LINES : 1);
	if (NOTESLINES <= 0)
		NOTESLINES = 1;
}
