/*
 *  Project   : tin - a Usenet reader
 *  Module    : color.c
 *  Original  : Olaf Kaluza <olaf@criseis.ruhr.de>
 *  Author    : Roland Rosenfeld <roland@spinnaker.rhein.de>
 *              Giuseppe De Marco <gdm@rebel.net> (light-colors)
 *              Julien Oster <fuzzy@cu8.cum.de> (word highlighting)
 *              T.Dickey <dickey@herndon4.his.com> (curses support)
 *  Created   : 1995-06-02
 *  Updated   : 1996-12-15
 *  Notes     : This are the basic function for ansi-color
 *              and word highlighting
 *
 * Copyright (c) 1995-2000 Roland Rosenfeld <roland@spinnaker.rhein.de>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Roland Rosenfeld.
 * 4. The name of the author may not be used to endorse or promote
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

static int current_fcol = 7;
static int current_bcol = 0;


/*
 * local prototypes
 */
static t_bool check_valid_mark (const char *s);
static void color_fputs (const char *s, FILE *stream, int color, t_bool signature);
static void put_mark_char (int c, FILE *stream, t_bool signature);


#	ifdef USE_CURSES
static void
set_colors (
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

		TRACE(("set_colors (%d, %d)", fcolor, bcolor));

		/* fcolor/bcolor may be negative, if we're using ncurses
		 * function use_default_colors().
		 */
		if (fcolor > COLORS-1) {
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
				p = (struct LIST *) my_malloc(sizeof(struct LIST));
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
refresh_color (
	void)
{
	set_colors(current_fcol, current_bcol);
}
#	endif /* USE_CURSES */

/* setting foreground-color */
void
fcol (
	int color)
{
	TRACE(("fcol(%d) %s", color, txt_colors[color-MIN_COLOR]));
	if (use_color) {
		if (color >= MIN_COLOR && color <= MAX_COLOR) {
#	ifdef USE_CURSES
			set_colors(color, current_bcol);
#	else
			int bold;
			if (color < 0)
				color = default_fcol;
			bold = (color >> 3); /* bitwise operation on signed value? ouch */
			my_printf("\033[%d;%dm", bold, ((color & 7) + 30));
			if (!bold)
				bcol(current_bcol);
#	endif /* USE_CURSES */
			current_fcol = color;
		}
	}
#	ifdef USE_CURSES
	else
		set_colors(default_fcol, default_bcol);
#	endif /* USE_CURSES */
}

/* setting background-color */
void
bcol (
	int color)
{
	TRACE(("bcol(%d) %s", color, txt_colors[color-MIN_COLOR]));
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


/*
 * Lookahead to find matching closing highlight character
 */
static t_bool
check_valid_mark (
	const char *s)
{
	const char *p;
	int c = *s;

	if (s[1] == '\0' || s[1] == c || !isgraph ((unsigned char)s[1]))
		return FALSE;
	p = strpbrk(s+2, "*_");

	return (p != NULL && *p == c &&
			((isalnum ((unsigned char)p[-1]) && !isalnum ((unsigned char)p[1])) ||
			 (ispunct ((unsigned char)p[-1]) && !isgraph ((unsigned char)p[1])))
			);
}


static void
put_mark_char (
	int c,
	FILE *stream,
	t_bool signature)
{
	switch (tinrc.word_h_display_marks) {
		case 1: /* print mark */
			my_fputc(c, stream);
			break;
		case 2: /* print space */
			my_fputc(' ', stream);
			break;
		case 3: /* print space, but only in signatures */
			if (signature)
				my_fputc(' ', stream);
			break;
		default: /* print nothing */
			break;
	}
}


/*
 * Like fputs(), but highlights words denoted by * and _ in colour
 */
static void
color_fputs (
	const char *s,
	FILE *stream,
	int color,
	t_bool signature)
{
	const char *p;
	t_bool hilite = FALSE;

	for (p = s; *p; p++) {
		if (*p == '*' || *p == '_') {
			if (! hilite) {
				if ((p == s || !isgraph((unsigned char)p[-1]))
						&& check_valid_mark(p)) {
					hilite = TRUE;
					fcol(*p == '*' ? tinrc.col_markstar : tinrc.col_markdash);
					put_mark_char(*p, stream, signature);
				} else /* print normal character */
					my_fputc(*p, stream);
			} else {
				hilite = FALSE;
				put_mark_char(*p, stream, signature);
				fcol(color);
			}
		} else {
			my_fputc(*p, stream);
		}
	}
}
#endif /* HAVE_COLOR */


/*
 * Output a line of text to the screen with colour if needed
 * word highlights, signatures etc will be highlighted
 */
void
draw_pager_line (
	const char *str,
	int flags)
{
#ifdef HAVE_COLOR
	int color = tinrc.col_text;

	if (use_color) {
		if (flags & C_SIG) {
			fcol (tinrc.col_signature);
			color = tinrc.col_signature;
		} else if (flags & (C_HEADER | C_ATTACH | C_UUE)) {
			color = tinrc.col_newsheaders;
			fcol (tinrc.col_newsheaders);
		} else {
			if (flags & C_QUOTE3) {
				fcol (tinrc.col_quote3);
				color = tinrc.col_quote3;
			} else if (flags & C_QUOTE2) {
				fcol (tinrc.col_quote2);
				color = tinrc.col_quote2;
			} else if (flags & C_QUOTE1) {
				fcol (tinrc.col_quote);
				color = tinrc.col_quote;
			} else
				fcol (tinrc.col_text);
		}
	}

	if (word_highlight && use_color)
		color_fputs(str, stdout, color, (flags&C_SIG));
	else
#endif /* HAVE_COLOR */
		my_fputs(str, stdout);

#ifndef USE_CURSES
	my_fputs(cCRLF, stdout);
#endif /* !USE_CURSES */
}
