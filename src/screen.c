/*
 *  Project   : tin - a Usenet reader
 *  Module    : screen.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2005-02-11
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

#ifndef USE_CURSES
	struct t_screen *screen;
#endif /* !USE_CURSES */


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
 * helper for the varius *_message() functions
 * returns a pointer to an allocated buffer with the formated message
 * must be freed if not needed anymore
 */
char *
fmt_message(
	const char *fmt,
	va_list ap)
{
	char *msg;
#ifdef HAVE_VASPRINTF

	if (vasprintf(&msg, fmt, ap) == -1)	/* something went wrong */
#endif /* HAVE_VASPRINTF */
	{
		size_t size = LEN;

		msg = my_malloc(size);
		/* TODO: realloc msg if necessary */
		vsnprintf(msg, size, fmt, ap);
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
	char *buf;
	va_list ap;

	va_start(ap, fmt);

	clear_message();
#ifdef HAVE_COLOR
	fcol(tinrc.col_message);
#endif /* HAVE_COLOR */

	buf = fmt_message(fmt, ap);
	my_fputs(buf, stdout);
	free(buf);

#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
	cursoron();
	my_flush();

	(void) sleep(sdelay);
/*	clear_message(); would be nice, but tin doesn't expect this yet */
	va_end(ap);
}


/*
 * Print a formatted message to stderr, no colour is added.
 * Interesting - this function implicitly clears 'errno'
 */
void
error_message(
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
	free(buf);

	if (cmd_line) {
		my_fputc('\n', stderr);
		fflush(stderr);
	} else {
		stow_cursor();
		(void) sleep(2);
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
		error_message("%s: Error: %s", buf, strerror(err));
		free(buf);
	}

	va_end(ap);

	return;
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
	int pos;
	int len;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	int width;
	wchar_t *wbuffer;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	len = strlen(str);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	if ((wbuffer = char2wchar_t(str)) != NULL) {
		if ((width = wcswidth(wbuffer, wcslen(wbuffer) + 1)) > 0)
			len = width;
		free(wbuffer);
	}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

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
		char *buffer;

		buffer = strunc(str, cCOLS - 2);
		my_fputs(buffer, stdout);
		free(buffer);
	} else
		my_fputs(str, stdout);

	if (cmd_line)
		my_flush();
	else {
		if (inverse)
			EndInverse();
	}
}


void
draw_arrow_mark(
	int line)
{
	MoveCursor(line, 0);

	if (tinrc.draw_arrow)
		my_fputs("->", stdout);
	else {
#ifdef USE_CURSES
		char buffer[BUFSIZ];
		char *s = screen_contents(line, 0, buffer);
#else
		char *s = screen[line - INDEX_TOP].col;
#endif /* USE_CURSES */
		StartInverse();
		my_fputs(s, stdout);
		EndInverse();
		if (s[MARK_OFFSET] == tinrc.art_marked_selected) {
			MoveCursor(line, MARK_OFFSET);
			EndInverse();
			my_fputc(s[MARK_OFFSET], stdout);
		}
	}
	stow_cursor();
}


void
erase_arrow(
	void)
{
	int line = INDEX_TOP + currmenu->curr - currmenu->first;

	if (!currmenu->max)
		return;

	MoveCursor(line, 0);

	if (tinrc.draw_arrow)
		my_fputs("  ", stdout);
	else {
#ifdef USE_CURSES
		char buffer[BUFSIZ];
		char *s = screen_contents(line, 0, buffer);
#else
		char *s;

		if (line - INDEX_TOP < 0) /* avoid underruns */
			line = INDEX_TOP;

		s = screen[line - INDEX_TOP].col;
#endif /* USE_CURSES */
		EndInverse();
		my_fputs(s, stdout);
		if (s[MARK_OFFSET] == tinrc.art_marked_selected) {
			MoveCursor(line, MARK_OFFSET);
			StartInverse();
			my_fputc(s[MARK_OFFSET], stdout);
			EndInverse();
		}
	}
}


void
show_title(
	const char *title)
{
	int col;

	col = (cCOLS - (int) strlen(_(txt_type_h_for_help))) + 1;
	if (col) {
		MoveCursor(0, col);
#ifdef HAVE_COLOR
		fcol(tinrc.col_title);
#endif /* HAVE_COLOR */
		/* you have mail message in */
		my_fputs((mail_check() ? _(txt_you_have_mail) : _(txt_type_h_for_help)), stdout);

#ifdef HAVE_COLOR
		fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
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


#define DISPLAY_FMT "%s %3d%% "
/*
 * progressmeter in %
 */
void
show_progress(
	const char *txt,
	long count,
	long total)
{
	char display[LEN];
	int ratio;
	time_t curr_time;
	static char last_display[LEN];
	static const char *last_txt;
	static int last_ratio;
	static long last_total;
	static time_t last_update;
#ifdef HAVE_GETTIMEOFDAY
	static long last_count;
	static int average;
	static int samples;
	static int sum;
	static struct timeval last_time;
	static struct timeval this_time;
	char *display_format;
	int time_diff;
	int secs_left;
	long count_diff;
#endif /* HAVE_GETTIMEOFDAY */

	if (batch_mode || count <= 0 || total == 0)
		return;

	/* If this is a new progress meter, start recalculating */
	if ((last_txt != txt) || (last_total != total)) {
		last_ratio = -1;
		last_display[0] = '\0';
		last_update = time(NULL) - 2;
	}

	curr_time = time(NULL);
	ratio = (int) ((count * 100) / total);
	if ((ratio == last_ratio) && (curr_time - last_update < 2))
		/*
		 * return if ratio did not change and less than 1-2 seconds since last
		 * update to reduce output
		 */
		return;

	last_update = curr_time;

#ifdef HAVE_GETTIMEOFDAY
	display_format = my_malloc(strlen(DISPLAY_FMT) + strlen(_(txt_remaining)) + 1);
	strcpy(display_format, DISPLAY_FMT);

	if (last_ratio == -1) {
		/* Don't print a "time remaining" this time */
		snprintf(display, sizeof(display), display_format, txt, ratio);

		/* Reset the variables */
		sum = average = samples = 0;
	} else {
		/* Get the current time */
		gettimeofday(&this_time, NULL);
		time_diff = (this_time.tv_sec - last_time.tv_sec) * 1000000;
		time_diff += (this_time.tv_usec - last_time.tv_usec);

		count_diff = (count - last_count);

		if (!count_diff) /* avoid div by zero */
			count_diff++;

		/*
		 * Calculate a running average based on the last 20 samples. For the
		 * first 19 samples just add all and divide by the number of samples.
		 * From the 20th sample on use only the last 20 samples to calculate
		 * the running averave. To make things easier we don't want to store
		 * and keep track of all of them, so we assume that the first sample
		 * was close to the current average and substract it from sum. Then,
		 * the new sample is added to the sum and the sum is divided by 20 to
		 * get the new average.
		 */
		if (samples == 20) {
			sum -= average;
			sum += (time_diff / count_diff);
			average = sum / 20;
		} else {
			sum += (time_diff / count_diff);
			average = sum / ++samples;
		}

		if (average >= 1000000)
			secs_left = (total - count) * (average / 1000000);
		else
			secs_left = ((total - count) * average) / 1000000;

		if (secs_left < 0)
			secs_left = 0;

		strcat(display_format, _(txt_remaining));
		snprintf(display, sizeof(display), display_format, txt, ratio, secs_left / 60, secs_left % 60);
	}
	free(display_format);

	last_count = count;
	gettimeofday(&last_time, NULL);

#else
	snprintf(display, sizeof(display), "%s %3d%%", txt, ratio);
#endif /* HAVE_GETTIMEOFDAY */

	/* Only display text if it changed from last time */
	if (strcmp(display, last_display)) {
		char *tmp;

		clear_message();
		MoveCursor(cLINES, 0);

#	ifdef HAVE_COLOR
		fcol(tinrc.col_message);
#	endif /* HAVE_COLOR */

		/*
		 * TODO: depending on the length of the newsgroup name
		 * it's possible to cut away a great part of the progress meter
		 * perhaps we should shorten the newsgroup name instead?
		 */
		my_printf("%s", sized_message(&tmp, "%s", display));
		free(tmp);

#	ifdef HAVE_COLOR
		fcol(tinrc.col_normal);
#	endif /* HAVE_COLOR */

		my_flush();
		STRCPY(last_display, display);
	}

	last_txt = txt;
	last_total = total;
	last_ratio = ratio;
}
