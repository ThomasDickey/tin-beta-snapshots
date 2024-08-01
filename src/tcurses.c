/*
 *  Project   : tin - a Usenet reader
 *  Module    : tcurses.c
 *  Author    : Thomas Dickey <dickey@invisible-island.net>
 *  Created   : 1997-03-02
 *  Updated   : 2024-05-13
 *  Notes     : This is a set of wrapper functions adapting the termcap
 *	             interface of tin to use SVr4 curses (e.g., ncurses).
 *
 * Copyright (c) 1997-2024 Thomas Dickey <dickey@invisible-island.net>
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

#ifdef USE_CURSES

#	ifndef KEY_MIN
#		define KEY_MIN KEY_BREAK	/* SVr3 curses */
#	endif /* !KEY_MIN */

#	ifndef KEY_CODE_YES
#		define KEY_CODE_YES (KEY_MIN - 1)	/* PDCurses */
#	endif /* !KEY_CODE_YES */

#	include "trace.h"

int cLINES;
int cCOLS;

static int my_innstr(char *str, int n);


#	if defined(HAVE_XCURSES) && !defined(HAVE_VW_PRINTW)
static int
vw_printw(
	WINDOW *w,
	char *fmt,
	va_list ap)
{
	char buffer[BUFSIZ];	/* FIXME */
	char *string = buffer;
	int y, x, code;

	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	getyx(w, y, x);
	TRACE(("vw_printw[%d/%d,%d/%d]:%s", y, cLINES, x, cCOLS, buffer));
	while (*string == '\b')
		string++;
	code = waddstr(w, string);
	if (string != buffer) {
		wmove(w, y, x);
		refresh();
	}
	return code;
}
#	endif /* HAVE_XCURSES && !HAVE_VW_PRINTW */


/*
 * Most of the logic corresponding to the termcap version is done in InitScreen.
 */
void
setup_screen(
	void)
{
	cmd_line = FALSE;
#	ifdef HAVE_COLOR
	bcol(tinrc.col_back);
#	endif /* HAVE_COLOR */
	scrollok(stdscr, TRUE);
	set_win_size(&cLINES, &cCOLS);
}


/*
 */
int
InitScreen(
	void)
{
#	ifdef NCURSES_VERSION
#		ifdef USE_TRACE
#			ifdef TRACE_CCALLS
	trace(TRACE_CALLS|TRACE_CCALLS);
#			else
	trace(TRACE_CALLS);
#			endif /* TRACE_CCALLS */
#		endif /* USE_TRACE */
#	endif /* NCURSES_VERSION */
	TRACE(("InitScreen"));
	initscr();
	cCOLS = COLS;
	cLINES = LINES - 1;
	TRACE(("screen size %d rows by %d cols", LINES, COLS));
/*	set_win_size(&cLINES, &cCOLS); */ /* will be done in setup_screen() */
/*	raw(); */ /* breaks serial terminal using software flow control and cbreak() below does most of the stuff raw() does */
	noecho();
	cbreak();
	cmd_line = FALSE;	/* ...so fcol/bcol will succeed */

	set_keypad_on();
#	ifdef HAVE_COLOR
	if (has_colors()) {
		start_color();
#		ifdef HAVE_USE_DEFAULT_COLORS
		if (use_default_colors() != ERR) {
			fcol(default_fcol = -1);
			bcol(default_bcol = -1);
		}
#		endif /* HAVE_USE_DEFAULT_COLORS */
		postinit_colors(MAX(COLORS, MAX_COLOR + 1)); /* postinit_colors(COLORS) would be correct */
	} else {
		use_color = FALSE;
		postinit_colors(MAX_COLOR + 1);
	}

#	endif /* HAVE_COLOR */
	set_xclick_on();
	return TRUE;
}


/*
 */
void
InitWin(
	void)
{
	TRACE(("InitWin"));
	Raw(TRUE);
	cmd_line = FALSE;
	set_keypad_on();
}


/*
 */
void
EndWin(
	void)
{
	TRACE(("EndWin(%d)", cmd_line));
	if (!cmd_line) {
		Raw(FALSE);
		endwin();
		cmd_line = TRUE;
	}
}


static int _inraw;

/*
 */
void
Raw(
	int state)
{
	if (state && !_inraw) {
		TRACE(("reset_prog_mode"));
		reset_prog_mode();
		_inraw = TRUE;
	} else if (!state && _inraw) {
		TRACE(("reset_shell_mode"));
		reset_shell_mode();
		_inraw = FALSE;
	}
}


/*
 */
int
RawState(
	void)
{
	return _inraw;
}


/*
 */
void
StartInverse(
	void)
{
	if (tinrc.inverse_okay) {
#	ifdef HAVE_COLOR
		if (use_color) {
			bcol(tinrc.col_invers_bg);
			fcol(tinrc.col_invers_fg);
		} else
#	endif /* HAVE_COLOR */
		{
			attrset(A_REVERSE);
		}
	}
}


#	if 0 /* not used */
static int
isInverse(
	void)
{
#		ifdef HAVE_COLOR
	if (use_color) {
		short pair = PAIR_NUMBER(getattrs(stdscr));
		short fg, bg;
		pair_content(pair, &fg, &bg);
		return (fg == tinrc.col_invers_fg) && (bg == tinrc.col_invers_bg);
	}
#		endif /* HAVE_COLOR */

	return (getattrs(stdscr) & A_REVERSE);
}
#	endif /* 0 */


#	if 0 /* doesn't work correct with ncurses4.x */
/*
 */
void
ToggleInverse(
	void)
{
	if (isInverse())
		EndInverse();
	else
		StartInverse();
}
#	endif /* 0 */


/*
 */
void
EndInverse(
	void)
{
	if (tinrc.inverse_okay && !cmd_line) {
#	ifdef HAVE_COLOR
		fcol(tinrc.col_normal);
		bcol(tinrc.col_back);
#	endif /* HAVE_COLOR */
		attroff(A_REVERSE);
	}
}


/*
 */
void
cursoron(
	void)
{
	if (!cmd_line)
		curs_set(1);
}


/*
 */
void
cursoroff(
	void)
{
	if (!cmd_line)
		curs_set(0);
}


/*
 */
void
set_keypad_on(
	void)
{
	if (!cmd_line)
		keypad(stdscr, TRUE);
}


/*
 */
void
set_keypad_off(
	void)
{
	if (!cmd_line)
		keypad(stdscr, FALSE);
}


/*
 * Ncurses mouse support is turned on/off when the keypad code is on/off,
 * as well as when we enable/disable the mousemask.
 */
void
set_xclick_on(
	void)
{
#	ifdef NCURSES_MOUSE_VERSION
	if (tinrc.use_mouse)
		mousemask(
			(BUTTON1_CLICKED|BUTTON2_CLICKED|BUTTON3_CLICKED),
			(mmask_t *) 0);
#	endif /* NCURSES_MOUSE_VERSION */
}


/*
 */
void
set_xclick_off(
	void)
{
#	ifdef NCURSES_MOUSE_VERSION
	(void) mousemask(0, (mmask_t *) 0);
#	endif /* NCURSES_MOUSE_VERSION */
}


void
MoveCursor(
	int row,
	int col)
{
	TRACE(("MoveCursor %d,%d", row, col));
	if (!cmd_line)
		move(row, col);
}


/*
 * Inverse 'size' chars at (row,col)
 */
void
highlight_string(
	int row,
	int col,
	int size)
{
	char tmp[LEN];

#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	/*
	 * In a multibyte locale we get byte offsets instead of character
	 * offsets calculate now the correct starting column
	 */
	if (col > 0 && col < (LEN / 2)) {
		wchar_t *wtmp;

		MoveCursor(row, 0);
		my_innstr(tmp, MIN(cCOLS, (LEN / 2) - 1));
		tmp[col] = '\0';
		if ((wtmp = char2wchar_t(tmp)) != NULL) {
			col = wcswidth(wtmp, wcslen(wtmp) + 1);
			free(wtmp);
		}
	}
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	if (row < 0)
		row = 0;
	if (col < 0)
		col = 0;

	MoveCursor(row, col);
	my_innstr(tmp, MIN(size, (LEN / 2) - 1));
	tmp[MIN(size, LEN - 1)] = '\0';
	StartInverse();
	my_fputs(tmp, stdout);
	EndInverse();
	my_flush();
	stow_cursor();
}


/*
 * Color 'size' chars at (row,col) with 'color' and handle marks
 */
void
word_highlight_string(
	int row,
	int col,
	int size,
	int color)
{
	char tmp[LEN] = { '\0' };
	/*
	 * Mapping of the tinrc.mono_mark* values to the ncurses attributes
	 */
	int attributes[] = {
		A_NORMAL,
		A_STANDOUT,
		A_UNDERLINE,
		A_REVERSE,
		A_BLINK,
		A_DIM,
		A_BOLD
	};
	int wsize = size;
#		if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;

	/*
	 * In a multibyte locale we get byte offsets instead of character offsets
	 * calculate now the correct starting column
	 */
	if (col > 0 && col < (LEN / 2)) {
		MoveCursor(row, 0);
		my_innstr(tmp, MIN(cCOLS, (LEN / 2) - 1));
		tmp[col] = '\0';
		if ((wtmp = char2wchar_t(tmp)) != NULL) {
			col = wcswidth(wtmp, wcslen(wtmp) + 1);
			free(wtmp);
		}
	}
#		endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	if (row < 0)
		row = 0;
	if (col < 0)
		col = 0;
	MoveCursor(row, col);
	my_innstr(tmp, MIN(size, (LEN / 2) - 1));

#		if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	tmp[MIN(size, LEN - 1)] = '\0';
	if ((wtmp = char2wchar_t(tmp)) != NULL) {
		wsize = wcswidth(wtmp, wcslen(wtmp) + 1);
		free(wtmp);
	}
#		endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	/* safegurad against bogus regexps */
	if (size > 1 && ((tmp[0] == '*' && tmp[size - 1] == '*') ||
		 (tmp[0] == '/' && tmp[size - 1] == '/') ||
		 (tmp[0] == '_' && tmp[size - 1] == '_') ||
		 (tmp[0] == '-' && tmp[size - 1] == '-'))) {

		switch (tinrc.word_h_display_marks) {
			case 0:
				delch();
				mvdelch(row, col + wsize - 2);
				MoveCursor(row, col);
				tmp[0] = tmp[size - 1] = ' ';
				str_trim(tmp);
				break;

			case 2: /* print space */
				MoveCursor(row, col + wsize - 1);
				my_fputs(" ", stdout);
				MoveCursor(row, col);
				my_fputs(" ", stdout);
				tmp[0] = tmp[size - 1] = ' ';
				str_trim(tmp);
				break;

			default: /* print mark (case 1) */
				break;
		}
	}
#	ifdef HAVE_COLOR
	if (use_color)
		fcol(color);
	else
#	endif /* HAVE_COLOR */
		if (color > 0 && color <= MAX_ATTR)
			attron(attributes[color]);
	my_fputs(tmp, stdout);
	my_flush();
#	ifdef HAVE_COLOR
	if (use_color)
		fcol(tinrc.col_text);
	else
#	endif /* HAVE_COLOR */
		if (color > 0 && color <= MAX_ATTR)
			attroff(attributes[color]);
	stow_cursor();
}


int
ReadCh(
	void)
{
	int ch;

	if (cmd_line)
		ch = cmdReadCh();
	else {
#	if defined(KEY_RESIZE) && defined(USE_CURSES)
again:
#	endif /* KEY_RESIZE && USE_CURSES */
		allow_resize(TRUE);
#	if defined(KEY_RESIZE) && defined(USE_CURSES)
		if ((ch = getch()) == KEY_RESIZE)
			need_resize = cYes;
#		if 0
		/*
		 * disable checking for ERR until all callers of ReadCh() doesn't
		 * depend on ERR for redrawing
		 */
		if (ch == ERR)
			goto again;
#		endif /* 0 */
#	else
		ch = getch();
#	endif /* KEY_RESIZE && USE_CURSES */
		allow_resize(FALSE);
		if (need_resize) {
			handle_resize((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
#	if defined(KEY_RESIZE) && defined(USE_CURSES)
			if (ch == KEY_RESIZE)
				goto again;
#	endif /* KEY_RESIZE && USE_CURSES */

		}
		if (ch == KEY_BACKSPACE)
			ch = '\010';	/* fix for Ctrl-H - show headers */
		else if (ch == ESC || ch >= KEY_MIN) {
			ungetch(ch);
			ch = ESC;
		}
	}
	TRACE(("ReadCh(%s)", tin_tracechar(ch)));
	return ch;
}


#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
wint_t
ReadWch(
	void)
{
	wint_t wch;
	int res;

	if (cmd_line)
		wch = cmdReadWch();
	else {
again:
		allow_resize(TRUE);
#		ifdef HAVE_NCURSESW
#			if defined(KEY_RESIZE) && defined(USE_CURSES)
		if ((res = get_wch(&wch)) == KEY_CODE_YES && wch == KEY_RESIZE)
			need_resize = cYes;
		if (res == ERR)
			goto again;
#			else
		res = get_wch(&wch);
#			endif /* KEY_RESIZE && USE_CURSES */
#		else
		wch = (wint_t) getch();

		if (wch == (wint_t) ERR)
			goto again;

		if (wch < KEY_MIN) {
			/* read in the multibyte sequence */
			char *mbs = my_calloc(1, MB_CUR_MAX + 1);
			int i, ch;
			wchar_t wc;

			mbs[0] = (char) wch;
			nodelay(stdscr, TRUE);
			res = mbtowc(&wc, mbs, MB_CUR_MAX);
			for (i = 1; i < (int) MB_CUR_MAX && res == -1; i++) {
				if ((res = mbtowc(&wc, mbs, MB_CUR_MAX)) > 0)
					break;
				if ((ch = getch()) != ERR)
					mbs[i] = (char) ch;
				else
					break;
			}
			nodelay(stdscr, FALSE);

			free(mbs);
			if (res == -1)
				return WEOF; /* error */
			else {
				res = OK;
				wch = wc;
			}
		} else {
			res = KEY_CODE_YES;
#			if defined(KEY_RESIZE) && defined(USE_CURSES)
			if (wch == KEY_RESIZE)
				need_resize = cYes;
#			endif /* KEY_RESIZE && USE_CURSES */
		}
#		endif /* HAVE_NCURSESW */
		allow_resize(FALSE);
		if (need_resize) {
			handle_resize((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
#		if defined(KEY_RESIZE) && defined(USE_CURSES)
			if (wch == KEY_RESIZE)
				goto again;
#		endif /* KEY_RESIZE && USE_CURSES */
		}
		if (wch == KEY_BACKSPACE)
			wch = (wint_t) '\010';	/* fix for Ctrl-H - show headers */
		else if (wch == ESC || res == KEY_CODE_YES) {
			/* TODO:
			 * check out why using unget_wch() here causes problems at
			 * get_arrow_key()
			 */
			/* unget_wch(wch); */
			ungetch((int) wch);
			wch = ESC;
		}
	}
	return wch;
}
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */


void
my_printf(
	const char *fmt,
	...)
{
	va_list ap;

	va_start(ap, fmt);
	if (cmd_line) {
		int flag = _inraw;
		if (flag)
			Raw(FALSE);
		vprintf(fmt, ap);
		if (flag)
			Raw(TRUE);
	} else
		vw_printw(stdscr, fmt, ap);

	va_end(ap);
}


void
my_fprintf(
	FILE *stream,
	const char *fmt,
	...)
{
	va_list ap;

	va_start(ap, fmt);
	TRACE(("my_fprintf(%s)", fmt));
	if (cmd_line) {
		int flag = _inraw && isatty(fileno(stream));
		if (flag)
			Raw(FALSE);
		vfprintf(stream, fmt, ap);
		if (flag)
			Raw(TRUE);
	} else
		vw_printw(stdscr, fmt, ap);

	va_end(ap);
}


void
my_fputc(
	int ch,
	FILE *stream)
{
	TRACE(("my_fputc(%s)", tin_tracechar(ch)));
	if (cmd_line) {
		if (_inraw && ch == '\n')
			fputc('\r', stream);
		fputc(ch, stream);
	} else
		addch((unsigned char) ch);
}


void
my_fputs(
	const char *str,
	FILE *stream)
{
	TRACE(("my_fputs(%s)", _nc_visbuf(str)));
	if (cmd_line) {
		if (_inraw) {
			while (*str)
				my_fputc(*str++, stream);
		} else
			fputs(str, stream);
	} else {
		addstr(str);
	}
}


#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
void
my_fputwc(
	wint_t wc,
	FILE *fp)
{
	if (cmd_line) {
		if (_inraw && wc == (wint_t) '\n')
			fputwc((wint_t) '\r', fp);
		fputwc(wc, fp);
	} else {
#		ifdef HAVE_NCURSESW
		cchar_t cc;
		wchar_t wstr[2];

		wstr[0] = (wchar_t) wc;
		wstr[1] = (wchar_t) '\0';

		if (setcchar(&cc, wstr, A_NORMAL, 0, NULL) != ERR)
			add_wch(&cc);
		else
			addch('?');
#	else
		char *mbs;
		int len;

		mbs = my_malloc(MB_CUR_MAX + 1);

		if ((len = wctomb(mbs, wc)) != -1) {
			mbs[len] = '\0';
			addstr(mbs);
		} else
			addch('?');

		free(mbs);
#		endif /* HAVE_NCURSESW */
	}
}


void
my_fputws(
	const wchar_t *wstr,
	FILE *fp)
{
	if (cmd_line) {
		if (_inraw) {
			while (*wstr)
				my_fputwc(*wstr++, fp);
		} else
			fputws(wstr, fp);
	} else {
#		ifdef HAVE_NCURSESW
		addwstr(wstr);
#		else
		char *mbs;

		if ((mbs = wchar_t2char(wstr)) != NULL) {
			addstr(mbs);
			free(mbs);
		}
#		endif /* HAVE_NCURSESW */
	}
}
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */


void
my_erase(
	void)
{
	TRACE(("my_erase"));

	if (!cmd_line) {
		erase();

		/*
		 * Curses doesn't actually do an erase() until refresh() is called.
		 * Ncurses 4.0 (and lower) reset the background color when doing an
		 * erase(). So the only way to ensure we'll get the same background
		 * colors is to reset them here.
		 */
		refresh();
#	ifdef HAVE_COLOR
		refresh_color();
#	endif /* HAVE_COLOR */
	}
}


void
my_fflush(
	FILE *stream)
{
	if (cmd_line)
		fflush(stream);
	else {
		TRACE(("my_fflush"));
		refresh();
	}
}


/*
 * Needed if non-curses output has corrupted curses understanding of the screen
 */
void
my_retouch(
	void)
{
	TRACE(("my_retouch"));
	if (!cmd_line) {
		wrefresh(curscr);
#	ifdef HAVE_COLOR
		fcol(tinrc.col_normal);
		bcol(tinrc.col_back);
#	endif /* HAVE_COLOR */
	}
}


/*
 * innstr can't read multibyte chars
 * we use innwstr (if available) and convert to multibyte chars
 */
static int
my_innstr(
	char *str,
	int n)
{
#	if defined(HAVE_NCURSESW) && defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	size_t len = 0;
	wchar_t *buffer;

	if (n < 0)
		return len;

	buffer = my_malloc(sizeof(wchar_t) * (size_t) (n + 1));

	if (innwstr(buffer, n) != ERR) {
		if ((len = wcstombs(str, buffer, (size_t) (2 * n))) == (size_t) (-1))
			len = 0;
		str[len] = '\0';
	}

	free(buffer);
	return len;
#	else
	int len = innstr(str, n);
	return (len == ERR ? 0 : len);
#	endif /* HAVE_NCURSESW && MULTIBYTE_ABLE && !NO_LOCALE */
}


char *
screen_contents(
	int row,
	int col,
	char *buffer)
{
	int len = COLS - col;
#	ifdef USE_TRACE
	int y, x;

	getyx(stdscr, y, x);
#	endif /* USE_TRACE */

	move(row, col);
	TRACE(("screen_contents(%d,%d)", row, col));
	len = my_innstr(buffer, len);
	buffer[len] = '\0';
	TRACE(("...screen_contents(%d,%d) %s", y, x, _nc_visbuf(buffer)));
	return buffer;
}


void
write_line(
	int row,
	char *buffer)
{
	mvaddnstr(row, 0, buffer, -1);
}


int
get_arrow_key(
	int prech) /* unused */
{
#	ifdef NCURSES_MOUSE_VERSION
	MEVENT my_event;
#	endif /* NCURSES_MOUSE_VERSION */
	int ch;
	int code = KEYMAP_UNKNOWN;

	if (cmd_line)
		code = cmd_get_arrow_key(prech);
	else {
		ch = getch();
		switch (ch) {
			case KEY_DC:
				code = KEYMAP_DEL;
				break;

			case KEY_IC:
				code = KEYMAP_INS;
				break;

			case KEY_UP:
				code = KEYMAP_UP;
				break;

			case KEY_DOWN:
				code = KEYMAP_DOWN;
				break;

			case KEY_LEFT:
				code = KEYMAP_LEFT;
				break;

			case KEY_RIGHT:
				code = KEYMAP_RIGHT;
				break;

			case KEY_NPAGE:
				code = KEYMAP_PAGE_DOWN;
				break;

			case KEY_PPAGE:
				code = KEYMAP_PAGE_UP;
				break;

			case KEY_HOME:
				code = KEYMAP_HOME;
				break;

			case KEY_END:
				code = KEYMAP_END;
				break;

#	ifdef NCURSES_MOUSE_VERSION
			case KEY_MOUSE:
				if (getmouse(&my_event) != ERR) {
					switch ((int) my_event.bstate) {
						case BUTTON1_CLICKED:
							xmouse = MOUSE_BUTTON_1;
							break;

						case BUTTON2_CLICKED:
							xmouse = MOUSE_BUTTON_2;
							break;

						case BUTTON3_CLICKED:
							xmouse = MOUSE_BUTTON_3;
							break;
					}
					xcol = my_event.x;	/* column */
					xrow = my_event.y;	/* row */
					code = KEYMAP_MOUSE;
				}
				break;
#	endif /* NCURSES_MOUSE_VERSION */
		}
	}
	return code;
}


#else
void my_tcurses(void); /* proto-type */
void my_tcurses(void) { }	/* ANSI C requires non-empty file */
#endif /* USE_CURSES */
