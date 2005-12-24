/*
 *  Project   : tin - a Usenet reader
 *  Module    : color.c
 *  Original  : Olaf Kaluza <olaf@criseis.ruhr.de>
 *  Author    : Roland Rosenfeld <roland@spinnaker.rhein.de>
 *              Giuseppe De Marco <gdm@rebel.net> (light-colors)
 *              Julien Oster <fuzzy@cu8.cum.de> (word highlighting)
 *              T.Dickey <dickey@invisible-island.net> (curses support)
 *  Created   : 1995-06-02
 *  Updated   : 2004-06-07
 *  Notes     : This are the basic function for ansi-color
 *              and word highlighting
 *
 * Copyright (c) 1995-2006 Roland Rosenfeld <roland@spinnaker.rhein.de>
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

#ifdef HAVE_COLOR

#	define MIN_COLOR -1	/* -1 is default, otherwise 0-7 or 0-15 */

int default_fcol = 7;
int default_bcol = 0;

#	ifdef USE_CURSES
	static int current_fcol = 7;
#	endif /* USE_CURSES */
static int current_bcol = 0;


/*
 * local prototypes
 */
#	ifdef USE_CURSES
	static void set_colors(int fcolor, int bcolor);
#	endif /* USE_CURSES */


#	ifdef USE_CURSES
static void
set_colors(
	int fcolor,
	int bcolor)
{
	static struct LIST {
		struct LIST *link;
		int pair;
		int fg;
		int bg;
	} *list;
	static int nextpair;

#		ifndef HAVE_USE_DEFAULT_COLORS
	if (fcolor < 0)
		fcolor = default_fcol;
	if (bcolor < 0)
		bcolor = default_bcol;
#		endif /* !HAVE_USE_DEFAULT_COLORS */
	if (cmd_line || !use_color || !has_colors()) {
		current_fcol = default_fcol;
		current_bcol = default_bcol;
	} else if (COLORS > 1 && COLOR_PAIRS > 1) {
		chtype attribute = A_NORMAL;
		int pair = 0;

		TRACE(("set_colors(%d, %d)", fcolor, bcolor));

		/*
		 * fcolor/bcolor may be negative, if we're using ncurses
		 * function use_default_colors().
		 */
		if (fcolor > COLORS - 1) {
			attribute |= A_BOLD;
			fcolor %= COLORS;
		}
		if (bcolor > 0)
			bcolor %= COLORS;

		/* curses assumes white/black */
		if (fcolor != COLOR_WHITE || bcolor != COLOR_BLACK) {
			struct LIST *p;
			t_bool found = FALSE;

			for (p = list; p != 0; p = p->link) {
				if (p->fg == fcolor && p->bg == bcolor) {
					found = TRUE;
					break;
				}
			}
			if (found)
				pair = p->pair;
			else if (++nextpair < COLOR_PAIRS) {
				p = my_malloc(sizeof(struct LIST)); /* TODO: plug mem leak */
				p->fg = fcolor;
				p->bg = bcolor;
				p->pair = pair = nextpair;
				p->link = list;
				list = p;
				init_pair(pair, fcolor, bcolor);
			} else
				pair = 0;
		}

		bkgdset(attribute | COLOR_PAIR(pair) | ' ');
	}
}


void
refresh_color(
	void)
{
	set_colors(current_fcol, current_bcol);
}
#	endif /* USE_CURSES */


/* setting foreground-color */
void
fcol(
	int color)
{
	TRACE(("fcol(%d) %s", color, txt_colors[color - MIN_COLOR]));
	if (use_color) {
		if (color >= MIN_COLOR && color <= MAX_COLOR) {
#	ifdef USE_CURSES
			set_colors(color, current_bcol);
			current_fcol = color;
#	else
			int bold;
			if (color < 0)
				color = default_fcol;
			bold = (color >> 3); /* bitwise operation on signed value? ouch */
			my_printf("\033[%d;%dm", bold, ((color & 7) + 30));
			if (!bold)
				bcol(current_bcol);
#	endif /* USE_CURSES */
		}
	}
#	ifdef USE_CURSES
	else
		set_colors(default_fcol, default_bcol);
#	endif /* USE_CURSES */
}


/* setting background-color */
void
bcol(
	int color)
{
	TRACE(("bcol(%d) %s", color, txt_colors[color - MIN_COLOR]));
	if (use_color) {
		if (color >= MIN_COLOR && color <= MAX_BACKCOLOR) {
#	ifdef USE_CURSES
			set_colors(current_fcol, color);
#	else
			if (color < 0)
				color = default_bcol;
			my_printf("\033[%dm", (color + 40));
#	endif /* USE_CURSES */
			current_bcol = color;
		}
	}
#	ifdef USE_CURSES
	else
		set_colors(default_fcol, default_bcol);
#	endif /* USE_CURSES */
}
#endif /* HAVE_COLOR */


/*
 * Output a line of text to the screen with colour if needed
 * word highlights, signatures etc will be highlighted
 */
void
draw_pager_line(
	const char *str,
	int flags,
	t_bool raw_data)
{
#ifdef HAVE_COLOR

	if (use_color) {
		if (flags & C_SIG) {
			fcol(tinrc.col_signature);
		} else if (flags & (C_HEADER | C_ATTACH | C_UUE)) {
			fcol(tinrc.col_newsheaders);
		} else {
			if (flags & C_QUOTE3) {
				fcol(tinrc.col_quote3);
			} else if (flags & C_QUOTE2) {
				fcol(tinrc.col_quote2);
			} else if (flags & C_QUOTE1) {
				fcol(tinrc.col_quote);
			} else
				fcol(tinrc.col_text);
		}
	}

#endif /* HAVE_COLOR */
	if (!raw_data) {
#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
		/*
		 * BiDi support
		 */
		/* don't run it on empty lines and lines containing only one char (which must be an ASCII one) */
		if (tinrc.render_bidi && IS_LOCAL_CHARSET("UTF-8") && strlen(str) > 1) {
			char *line;
			t_bool is_rtl;

			if ((line = render_bidi(str, &is_rtl)) != NULL) {
				if (is_rtl) { /* RTL */
					/* determine visual length and pad out so that line is right-aligned */
					wchar_t *wline;

					if ((wline = char2wchar_t(line)) != NULL) {
						int visual_len;

						wconvert_to_printable(wline);
						visual_len = wcswidth(wline, wcslen(wline) + 1);
						free(wline);

						if (visual_len > 0) {
							int i;

							for (i = 0; i < cCOLS - visual_len - 1; i++)
								my_fputc(' ', stdout);
						}
						my_fputs(line, stdout);
					} else /* fallback */
						my_fputs(line, stdout);

				} else	/* LTR */
					my_fputs(line, stdout);

				free(line);
			} else
				my_fputs(str, stdout);
		} else
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
			my_fputs(str, stdout);
	} else {
		/* in RAW-mode (show_all_headers) display non-printable chars as octals */
		const char *c;
		char octal[5];

		c = str;
		while (*c) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			int num_bytes;
			wchar_t wc;

			num_bytes = mbtowc(&wc, c, MB_CUR_MAX);
			if (num_bytes != -1 && iswprint(wc)) {
				my_fputwc((wint_t) wc, stdout);
				c += num_bytes;
			}
#else
			if (my_isprint((int) *c)) {
				my_fputc((int) *c, stdout);
				c++;
			}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			else if (IS_LOCAL_CHARSET("Big5") && (unsigned char) *c >= 0xa1 &&(unsigned char) *c <= 0xfe && *(c + 1)) {
				/*
				 * Big5: ASCII chars are handled by the normal code
				 * check only for 2-byte chars
				 * TODO: should we also check if the second byte is also valid?
				 */
				my_fputc((int) *c, stdout);
				c++;
				my_fputc((int) *c, stdout);
				c++;
			} else {
				/*
				 * non-printable char
				 * print as an octal value
				 */
				snprintf(octal, sizeof(octal), "\\%03o", (int) *c & 0xff);
				my_fputs(octal, stdout);
				c++;
			}
		}
	}

#ifndef USE_CURSES
	my_fputs(cCRLF, stdout);
#endif /* !USE_CURSES */
}
