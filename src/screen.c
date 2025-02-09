/*
 *  Project   : tin - a Usenet reader
 *  Module    : screen.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2025-02-05
 *  Notes     :
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

int mark_offset = 0;

static FILE* msglog = NULL;

#ifndef USE_CURSES
	struct t_screen *screen;
#endif /* !USE_CURSES */

static void log_formatted_msg(const char *tag, const char *msg);

/*
 * Move the cursor to the lower-left of the screen, where it won't be annoying
 */
void
stow_cursor(
	void)
{
	if (!cmd_line)
		MoveCursor(cLINES, 0);
}


/*
 * helper for the various *_message() functions
 * returns a pointer to an allocated buffer with the formatted message
 * must be freed if not needed anymore
 */
char *
fmt_message(
	const char *fmt,
	va_list ap)
{
	size_t size = LEN;
	char *msg = my_malloc(size);
	int used;
	va_list aq;

	while (1) {
		begin_va_copy(aq, ap);
		used = vsnprintf(msg, size, fmt, aq);
		end_va_copy(aq);

		if (used >= 0 && used < (int) size)
			break;

		size <<= 1;
		msg = my_realloc(msg, size);
	}

	return msg;
}


/*
 * Centre a formatted colour message at the bottom of the screen
 */
void
info_message(
	const char *fmt,
	...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);

	clear_message();
#ifdef HAVE_COLOR
	fcol(tinrc.col_message);
#endif /* HAVE_COLOR */

	buf = fmt_message(fmt, ap);
	center_line(cLINES, FALSE, buf);	/* center the message at screen bottom */
	free(buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
	stow_cursor();

	va_end(ap);
}


/*
 * Print a formatted colour message at the bottom of the screen, wait a while
 */
void
wait_message(
	unsigned int sdelay,
	const char *fmt,
	...)
{
	char *buf, *msg;
	size_t len = cCOLS - 1;
	va_list ap;

	va_start(ap, fmt);

	clear_message();
#ifdef HAVE_COLOR
	fcol(tinrc.col_message);
#endif /* HAVE_COLOR */

	buf = fmt_message(fmt, ap);
	/* test for multiline messages */
	if (strrchr(buf, '\n')) {
		char *from, *to, *tmp;

		for (from = buf; *from && (to = strchr(from, '\n')); from = ++to) {
			*to = '\0';
			/* expand tabs in multiline msg, as we wrap anyway */
			tmp = expand_tab(from, tabwidth);
			/* and strunc would replace them */
			msg = strunc(tmp, len);
			free(tmp);
			my_fputs(msg, stdout);
			my_fputc('\n', stdout);
			free(msg);
		}
	} else {
		msg = strunc(buf, len);
		my_fputs(msg, stdout);
		free(msg);
	}
	log_formatted_msg(NULL, buf);
	free(buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
	cursoron();
	my_flush();
	va_end(ap);

#ifdef HAVE_SELECT
	/* allow to skip sleep time via key-press */
	{
		int nfds;
		fd_set readfds;
		struct timeval tv;

		forever {
			FD_ZERO(&readfds);
			FD_SET(STDIN_FILENO, &readfds);
			tv.tv_sec = sdelay;
			tv.tv_usec = 0;

#	ifdef HAVE_SELECT_INTP
			nfds = select(STDIN_FILENO + 1, (int *) &readfds, NULL, NULL, &tv);
#	else
			nfds = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
#	endif /* HAVE_SELECT_INTP */
			if (nfds == -1) {

#	ifdef EINTR
				if (errno != EINTR)
#	else
				if (errno != 0)
#	endif /* EINTR */
				{
					perror_message("wait_message(select()) failed");
					free(tin_progname);
					giveup();
				} else
					return;
			} else
				break;
		}

		if (nfds > 0) {
			if (FD_ISSET(STDIN_FILENO, &readfds)) {
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				ReadWch();
#	else
				ReadCh();
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}
		}
	}
#else
	(void) sleep(sdelay);
#endif /* HAVE_SELECT */
}


/*
 * Print a formatted message to stderr, no colour is added.
 * Interesting - this function implicitly clears 'errno'
 */
void
error_message(
	unsigned int sdelay,
	const char *fmt,
	...)
{
	char *buf;
	va_list ap;

	va_start(ap, fmt);

	errno = 0;
	clear_message();

	buf = fmt_message(fmt, ap);
	my_fputs(buf, stderr);	/* don't use my_fprintf() here due to %format chars */
	my_fflush(stderr);
	log_formatted_msg("ERR", buf);
	free(buf);

	if (cmd_line) {
		my_fputc('\n', stderr);
		fflush(stderr);
	} else {
		stow_cursor();
		(void) sleep(sdelay);
		clear_message();
	}

	va_end(ap);
}


/*
 * Print a formatted error message to stderr, no colour is added.
 * This function implicitly clears 'errno'
 */
void
perror_message(
	const char *fmt,
	...)
{
	char *buf;
	int err;
	va_list ap;

	err = errno;
	va_start(ap, fmt);

	clear_message();

	if ((buf = fmt_message(fmt, ap)) != NULL) {
		error_message(2, "%s: Error: %s", buf, strerror(err));
		free(buf);
	}

	va_end(ap);
}


void
clear_message(
	void)
{
	if (!cmd_line) {
		MoveCursor(cLINES, 0);
		CleartoEOLN();
		cursoroff();
#ifndef USE_CURSES
		my_flush();
#endif /* !USE_CURSES */
	}
}


void
center_line(
	int line,
	t_bool inverse,
	const char *str)
{
	char *ln;
	int pos;
	int len = strwidth(str);

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
	if (tinrc.render_bidi && IS_LOCAL_CHARSET("UTF-8") && len > 1) {
		t_bool is_rtl;

		if ((ln = render_bidi(str, &is_rtl)) == NULL)
			ln = my_strdup(str);
	} else
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
		ln = my_strdup(str);

	if (!cmd_line) {
		if (cCOLS >= len)
			pos = (cCOLS - len) / 2;
		else
			pos = 1;

		MoveCursor(line, pos);
		if (inverse) {
			StartInverse();
			my_flush();
		}
	}

	if (len >= cCOLS) {
		char *buffer = strunc(ln, cCOLS - 2);

		if (buffer != ln)
			free(ln);

		ln = buffer;
	}
	my_fputs(ln, stdout);

	if (cmd_line)
		my_flush();
	else {
		if (inverse)
			EndInverse();
	}
	free(ln);
}


void
draw_arrow_mark(
	int line)
{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	MoveCursor(line, 0);

	if (tinrc.draw_arrow) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (tinrc.utf8_graphics) {
			my_fputwc(CURSOR_HORIZ, stdout);
			my_fputwc(CURSOR_ARROW, stdout);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			my_fputs("->", stdout);
	} else {
		char *s;
#ifdef USE_CURSES
		char *buffer;

#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		buffer = my_malloc(MB_CUR_MAX * (size_t) (cCOLS + 1));
#	else
		buffer = my_malloc(cCOLS + 1);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		s = screen_contents(line, 0, buffer);
#else
		s = screen[line - INDEX_TOP].col;
#endif /* USE_CURSES */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wtmp = char2wchar_t(s)) != NULL) {
			StartInverse();
			my_fputws(wtmp, stdout);
			EndInverse();
			if (mark_offset && wtmp[mark_offset + (art_mark_width - wcwidth(tinrc.art_marked_selected))] == tinrc.art_marked_selected) {
				MoveCursor(line, mark_offset + (art_mark_width - wcwidth(tinrc.art_marked_selected)));
/*				EndInverse(); */ /* needed? */
				my_fputwc((wint_t) wtmp[mark_offset + (art_mark_width - wcwidth(tinrc.art_marked_selected))], stdout);
			}
			free(wtmp);
		}
#else
		StartInverse();
		my_fputs(s, stdout);
		EndInverse();
		if (mark_offset && s[mark_offset] == tinrc.art_marked_selected) {
			MoveCursor(line, mark_offset);
/*			EndInverse(); */ /* needed? */
			my_fputc(s[mark_offset], stdout);
		}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#ifdef USE_CURSES
		free(buffer);
#endif /* USE_CURSES */
	}
	stow_cursor();
}


void
erase_arrow(
	void)
{
	int line = INDEX_TOP + currmenu->curr - currmenu->first;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (!currmenu->max)
		return;

	MoveCursor(line, 0);

	if (tinrc.draw_arrow)
		my_fputs("  ", stdout);
	else {
		char *s;
#ifdef USE_CURSES
		char *buffer;

#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		buffer = my_malloc(MB_CUR_MAX * (size_t) (cCOLS + 1));
#	else
		buffer = my_malloc(cCOLS + 1);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		s = screen_contents(line, 0, buffer);
#else
		if (line - INDEX_TOP < 0) /* avoid underruns */
			line = INDEX_TOP;

		s = screen[line - INDEX_TOP].col;
#endif /* USE_CURSES */
		EndInverse();
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wtmp = char2wchar_t(s)) != NULL) {
			my_fputws(wtmp, stdout);
			if (mark_offset && wtmp[mark_offset + (art_mark_width - wcwidth(tinrc.art_marked_selected))] == tinrc.art_marked_selected) {
				MoveCursor(line, mark_offset + (art_mark_width - wcwidth(tinrc.art_marked_selected)));
				StartInverse();
				my_fputwc((wint_t) wtmp[mark_offset + (art_mark_width - wcwidth(tinrc.art_marked_selected))], stdout);
				EndInverse();
			}
			free(wtmp);
		}
#else
		my_fputs(s, stdout);
		if (mark_offset && s[mark_offset] == tinrc.art_marked_selected) {
			MoveCursor(line, mark_offset);
			StartInverse();
			my_fputc(s[mark_offset], stdout);
			EndInverse();
		}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#ifdef USE_CURSES
		free(buffer);
#endif /* USE_CURSES */
	}
}


void
show_title(
	const char *title)
{
	char keyhelp[MAXKEYLEN];
	char *helps;
	const char *sign;
	size_t col;

	if (tinrc.show_help_mail_sign != SHOW_SIGN_NONE) {
		col = MAXKEYLEN + strlen(_(txt_type_h_for_help)) + 1;
		helps = my_malloc(col);

		switch (signal_context) {
			case cSelect:
				PrintFuncKey(keyhelp, GLOBAL_HELP, select_keys);
				break;

			case cGroup:
				PrintFuncKey(keyhelp, GLOBAL_HELP, group_keys);
				break;

			case cThread:
				PrintFuncKey(keyhelp, GLOBAL_HELP, thread_keys);
				break;

			default:
				keyhelp[0] = 'h';
				keyhelp[1] = '\0';
		}
		snprintf(helps, col, _(txt_type_h_for_help), keyhelp);

		switch (tinrc.show_help_mail_sign) {
			case SHOW_SIGN_MAIL:
				sign = mail_check(mailbox) ? _(txt_you_have_mail) : NULL;
				break;

			case SHOW_SIGN_BOTH:
				sign = mail_check(mailbox) ? _(txt_you_have_mail) : helps;
				break;

			default:
				sign = helps;
				break;
		}

		if (sign) {
			if (cCOLS > strwidth(sign)) {
				col = cCOLS - strwidth(sign);
				MoveCursor(0, (int) col);
#ifdef HAVE_COLOR
				fcol(tinrc.col_title);
#endif /* HAVE_COLOR */
				/* you have mail message in */
				my_fputs(sign, stdout);

#ifdef HAVE_COLOR
				fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
			}
		}
		free(helps);
	}
	center_line(0, TRUE, title); /* wastes some space on the left */
}


void
ring_bell(
	void)
{
#ifdef USE_CURSES
	if (!cmd_line)
		beep();
	else {
#endif /* USE_CURSES */
	my_fputc('\007', stdout);
	my_flush();
#ifdef USE_CURSES
	}
#endif /* USE_CURSES */
}


void
spin_cursor(
	void)
{
	static const char buf[] = "|/-\\|/-\\ "; /* don't remove the tailing space! */
	static unsigned short int i = 0;

	if (batch_mode)
		return;

	if (i > 7)
		i = 0;

#ifdef HAVE_COLOR
	fcol(tinrc.col_message);
#endif /* HAVE_COLOR */
	my_printf("\b%c", buf[i++]);
	my_flush();
#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
}


#if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
#	define DISPLAY_FMT "%s %3d%% "
#else
#	define DISPLAY_FMT "%s %3d%%"
#endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */
/*
 * progressmeter in %
 */
void
show_progress(
	const char *txt,
	t_artnum count,
	t_artnum total)
{
	char display[LEN];
	int ratio;
	time_t curr_time;
	static char last_display[LEN];
	static int last_ratio;
	static t_artnum last_count;
	static t_artnum last_total;
	static time_t last_update;
#if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
	static int average;
	static int samples;
	static int last_secs_left;
	static int sum;
	char *display_format;
	int time_diff;
	int secs_left;
	t_artnum count_diff;
	static struct t_tintime last_time;
	static struct t_tintime this_time;
#endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */

	if (batch_mode || count <= 0 || total <= 1)
		return;

	/* If this is a new progress meter, start recalculating */
	if ((last_total != total) || (count <= last_count)) {
		last_ratio = -1;
		last_display[0] = '\0';
		last_update = time(NULL) - 2;
	}

	curr_time = time(NULL);
	ratio = (int) ((count * 100) / total);
	if ((ratio == last_ratio) && (curr_time - last_update < 2))
		/*
		 * return if ratio did not change and less than
		 * 2 seconds since last update to reduce output
		 */
		return;

	last_update = curr_time;

#if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
	display_format = my_malloc(strlen(DISPLAY_FMT) + strlen(_(txt_remaining)) + 1);
	strcpy(display_format, DISPLAY_FMT);

	if (last_ratio == -1) {
		/* Don't print the time remaining */
		snprintf(display, sizeof(display), display_format, txt, ratio);

		/* Reset the variables */
		sum = average = samples = last_secs_left = 0;
	} else {
		/* Get the current time */
		tin_gettime(&this_time);
		time_diff = (int) ((this_time.tv_sec - last_time.tv_sec) * 1000000);
		time_diff = (int) (time_diff + ((this_time.tv_nsec - last_time.tv_nsec) / 1000));
		count_diff = (count - last_count);

		if (!count_diff) /* avoid div by zero */
			++count_diff;

		/*
		 * Calculate a running average based on the last 20 samples. For the
		 * first 19 samples just add all and divide by the number of samples.
		 * From the 20th sample on use only the last 20 samples to calculate
		 * the running averave. To make things easier we don't want to store
		 * and keep track of all of them, so we assume that the first sample
		 * was close to the current average and subtract it from sum. Then,
		 * the new sample is added to the sum and the sum is divided by 20 to
		 * get the new average.
		 */
		if (samples == 20) {
			sum -= average;
			sum = (int) (sum + (time_diff / count_diff));
			average = sum / 20;
		} else {
			sum = (int) (sum + (time_diff / count_diff));
			average = sum / ++samples;
		}

		if (average >= 1000000)
			secs_left = (int) ((total - count) * (average / 1000000));
		else
			secs_left = (int) (((total - count) * average) / 1000000);

		if (secs_left < 0)
			secs_left = 0;

		if ((secs_left > 0) && (last_secs_left == 0))
			last_secs_left = secs_left;

		if (samples < 5)
			/* Don't print the time remaining */
			snprintf(display, sizeof(display), display_format, txt, ratio);
		else {
			/* Don't allow time remaining to increase by 1 or 2 seconds */
			if ((secs_left == last_secs_left + 1) || (secs_left == last_secs_left + 2))
				secs_left = last_secs_left;
			else if (secs_left < last_secs_left)
				last_secs_left = secs_left;
			strcat(display_format, _(txt_remaining));
			snprintf(display, sizeof(display), display_format, txt, ratio, secs_left / 60, secs_left % 60);
		}
	}
	free(display_format);

	tin_gettime(&last_time);
#else
	snprintf(display, sizeof(display), DISPLAY_FMT, txt, ratio);
#endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */

	/* Only display text if it changed from last time */
	if (strcmp(display, last_display)) {
		char *tmp;

		if (RawState()) {
			clear_message();
			MoveCursor(cLINES, 0);
		} else {
			my_printf("\r");
			my_flush();
			CleartoEOLN();
		}

#ifdef HAVE_COLOR
		if (RawState())
			fcol(tinrc.col_message);
#endif /* HAVE_COLOR */

		/*
		 * TODO: depending on the length of the newsgroup name
		 * it's possible to cut away a great part of the progress meter
		 * perhaps we should shorten the newsgroup name instead?
		 */
		my_printf("%s", sized_message(&tmp, "%s", display));
		free(tmp);

#ifdef HAVE_COLOR
		if (RawState())
			fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */

#ifndef USE_CURSES
		if (!RawState())
			MoveCursor(cLINES, 0);
#endif /* !USE_CURSES */

		my_flush();
		STRCPY(last_display, display);
	}

	last_count = count;
	last_total = total;
	last_ratio = ratio;
}


void
open_msglog(
	void)
{
	char logfile[PATH_LEN];
	char serverdir[PATH_LEN];

	if (msglog != NULL)
		return;

#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news && nntp_tcp_port != IPPORT_NNTP)
		snprintf(logfile, sizeof(logfile), "%s:%u", nntp_server, nntp_tcp_port);
	else
#endif /* NNTP_ABLE */
	{
		snprintf(logfile, sizeof(logfile), "%s", nntp_server);
	}
	joinpath(serverdir, sizeof(serverdir), rcdir, logfile);
	joinpath(logfile, sizeof(logfile), serverdir, "msglog");

	if ((msglog = fopen(logfile, "w")) != NULL) {
#ifdef HAVE_FCHMOD
		fchmod(fileno(msglog), (mode_t) (S_IRUSR|S_IWUSR));
#else
#	ifdef HAVE_CHMOD
		chmod(logfile, (mode_t) (S_IRUSR|S_IWUSR));
#	endif /* HAVE_CHMOD */
#endif /* HAVE_FCHMOD */
	}
}


void
close_msglog(
	void)
{
	if (msglog)
		fclose(msglog);
	msglog = NULL;
}


void
log_formatted_msg(
	const char *tag,
	const char *msg)
{
	size_t len;

	if (msglog == NULL || msg == NULL || (len = strlen(msg)) == 0)
		return;

	if (tag)
		fprintf(msglog, "%s: %s", tag, msg);
	else
		fprintf(msglog, "%s", msg);

	if (msg[len - 1] != '\n')
		fputc('\n', msglog);

	fflush(msglog);
}
