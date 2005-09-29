/*
 *  Project   : tin - a Usenet reader
 *  Module    : curses.c
 *  Author    : D. Taylor & I. Lea
 *  Created   : 1986-01-01
 *  Updated   : 2005-04-18
 *  Notes     : This is a screen management library borrowed with permission
 *              from the Elm mail system. This library was hacked to provide
 *              what tin needs.
 *  Copyright : Copyright (c) 1986-99 Dave Taylor & Iain Lea
 *              The Elm Mail System  -  @Revision: 2.1 $   $State: Exp @
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

/* local prototype */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE) && defined(M_UNIX)
	static wint_t convert_c2wc (const char *input);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE && M_UNIX */

#ifdef USE_CURSES

#define ReadCh cmdReadCh
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE) && defined(M_UNIX)
#	define ReadWch cmdReadWch
#endif /* MULTIBYTE_ABLE && !NO_LOCALE && M_UNIX */

void my_dummy(void) { }	/* ANSI C requires non-empty file */
t_bool have_linescroll = TRUE;	/* USE_CURSES always allows line scrolling */

#else	/* !USE_CURSES */

#ifndef ns32000
#	undef	sinix
#endif /* !ns32000 */

#ifdef VMS
#	include <descrip.h>
#	include <iodef.h>
#	include <ssdef.h>
#	include <dvidef.h>
#	ifdef __GNUC__ /* M.St. 22.01.98 */
#		include <vms/sys$routines.h>
#		include <vms/lib$routines.h>
#	endif /* __GNUC__ */
#endif /* VMS */

#define DEFAULT_LINES_ON_TERMINAL	24
#define DEFAULT_COLUMNS_ON_TERMINAL	80

int cLINES = DEFAULT_LINES_ON_TERMINAL - 1;
int cCOLS = DEFAULT_COLUMNS_ON_TERMINAL;
int _hp_glitch = FALSE;		/* stdout not erased by overwriting on HP terms */
static int _inraw = FALSE;	/* are we IN rawmode? */
static int xclicks = FALSE;	/* do we have an xterm? */
t_bool have_linescroll = FALSE;

#define TTYIN	0

#ifdef HAVE_CONFIG_H

#	if defined(HAVE_TERMIOS_H) && defined(HAVE_TCGETATTR) && defined(HAVE_TCSETATTR)
#		ifdef HAVE_IOCTL_H
#			include <ioctl.h>
#		else
#			ifdef HAVE_SYS_IOCTL_H
#				include <sys/ioctl.h>
#			endif /* HAVE_SYS_IOCTL_H */
#		endif /* HAVE_IOCTL_H */
#		if !defined(sun) || !defined(NL0)
#			include <termios.h>
#		endif /* !sun || !NL0 */
#		define USE_POSIX_TERMIOS 1
#		define TTY struct termios
#	else
#		ifdef HAVE_TERMIO_H
#			include <termio.h>
#			define USE_TERMIO 1
#			define TTY struct termio
#		else
#			ifdef HAVE_SGTTY_H
#				include <sgtty.h>
#				define USE_SGTTY 1
#				define TTY struct sgttyb
/*
	#			else
	#				error "No termios.h, termio.h or sgtty.h found"
*/
#			endif /* HAVE_SGTTY_H */
#		endif /* HAVE_TERMIO_H */
#	endif /* HAVE_TERMIOS_H && HAVE_TCGETATTR && HAVE_TCSETATTR */

static TTY _raw_tty, _original_tty;

#else	/* FIXME: prune the non-autoconf'd stuff */

#	if defined(COHERENT) || defined(BSD)
#		ifdef BSD4_1
#			include <termio.h>
#			define USE_TERMIO 1
#		else
#			include <sgtty.h>
#			define USE_SGTTY 1
#		endif /* BSD4_1 */
#	else
#		if !defined(SYSV)
#			ifdef MINIX
#				include <sgtty.h>
#				define USE_SGTTY 1
#			else
#				ifndef QNX42
#					ifdef sinix
#						include <termios.h>
#						define USE_POSIX_TERMIOS 1
#					else
#						ifdef VMS
#							include <curses.h>
#						else
#							include <termio.h>
#							define USE_TERMIO 1
#						endif /* VMS */
#					endif /* sinix */
#				endif /* !QNX42 */
#			endif /* MINIX */
#		endif /* !SYSV */
#	endif /* COHERENT || BSD */

#	ifndef VMS
#		if defined(BSD) || defined(MINIX)
#			define USE_SGTTY 1
struct sgttyb _raw_tty, _original_tty;
#		else
#			if defined(HAVE_TERMIOS_H) || defined(sinix)
#				define USE_POSIX_TERMIOS 1
struct termios _raw_tty, _original_tty;
#			else
#				define USE_TERMIO 1
struct termio _raw_tty, _original_tty;
#			endif /* HAVE_TERMIOS_H || sinix */
#		endif /* BSD || MINIX */
#	endif /* !VMS */
#endif /* HAVE_CONFIG_H */

#ifndef USE_SGTTY
#	define USE_SGTTY 0
#endif /* !USE_SGTTY */

#ifndef USE_POSIX_TERMIOS
#	define USE_POSIX_TERMIOS 0
#	ifndef USE_TERMIO
#		define USE_TERMIO 0
#	endif /* !USE_TERMIO */
#endif /* !USE_POSIX_TERMIOS */

static char *_clearscreen, *_moveto, *_cleartoeoln, *_cleartoeos,
			*_setinverse, *_clearinverse, *_setunderline, *_clearunderline,
			*_xclickinit, *_xclickend, *_cursoron, *_cursoroff,
			*_terminalinit, *_terminalend, *_keypadlocal, *_keypadxmit,
			*_scrollregion, *_scrollfwd, *_scrollback,
			*_reset, *_reversevideo, *_blink, *_dim, *_bold;

static int _columns, _line, _lines;

#ifdef M_UNIX
#	undef SET_TTY
#	undef GET_TTY
#	if USE_POSIX_TERMIOS
#		define SET_TTY(arg) tcsetattr(TTYIN, TCSANOW, arg)
#		define GET_TTY(arg) tcgetattr(TTYIN, arg)
#	else
#		if USE_TERMIO
#			define SET_TTY(arg) ioctl(TTYIN, TCSETAW, arg)
#			define GET_TTY(arg) ioctl(TTYIN, TCGETA, arg)
#		else
#			if USE_SGTTY
#				define SET_TTY(arg) stty(TTYIN, arg)
#				define GET_TTY(arg) gtty(TTYIN, arg)
/*
	#			else
	#				error "No termios.h, termio.h or sgtty.h found"
*/
#			endif /* USE_SGTTY */
#		endif /* USE_TERMIO */
#	endif /* USE_POSIX_TERMIOS */
#endif /* M_UNIX */

#if 0
	static int in_inverse;			/* 1 when in inverse, 0 otherwise */
#endif /* 0 */

/*
 * Local prototypes
 */
static int input_pending(int delay);
static void ScreenSize(int *num_lines, int *num_columns);
static void xclick(int state);


/*
 * returns the number of lines and columns on the display.
 */
static void
ScreenSize(
	int *num_lines,
	int *num_columns)
{
	if (!_lines)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (!_columns)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;

	*num_lines = _lines - 1;		/* assume index from zero */
	*num_columns = _columns;		/* assume index from one */
}


/*
 * get screen size from termcap entry & setup sizes
 */
void
setup_screen(
	void)
{
	_line = 1;
	ScreenSize(&cLINES, &cCOLS);
	cmd_line = FALSE;
	Raw(TRUE);
	set_win_size(&cLINES, &cCOLS);
}

#ifdef M_UNIX

#	ifdef USE_TERMINFO
#		define CAPNAME(a,b)       b
#		define dCAPNAME(a,b)      strcpy(_terminal, b)
#		define TGETSTR(b,bufp)    tigetstr(b)
#		define TGETNUM(b)         tigetnum(b) /* may be tigetint() */
#		define TGETFLAG(b)        tigetflag(b)
#		define NO_CAP(s)          (s == 0 || s == (char *) -1)
#		if !defined(HAVE_TIGETNUM) && defined(HAVE_TIGETINT)
#			define tigetnum tigetint
#		endif /* !HAVE_TIGETNUM && HAVE_TIGETINT */
#	else /* USE_TERMCAP */
#		undef USE_TERMCAP
#		define USE_TERMCAP 1
#		define CAPNAME(a,b)       a
#		define dCAPNAME(a,b)      a
#		define TGETSTR(a, bufp)   tgetstr(a, bufp)
#		define TGETNUM(a)         tgetnum(a)
#		define TGETFLAG(a)        tgetflag(a)
#		define NO_CAP(s)          (s == 0)
#	endif /* USE_TERMINFO */

#	ifdef HAVE_TPARM
#		define TFORMAT(fmt, a, b) tparm(fmt, b, a)
#	else
#		define TFORMAT(fmt, a, b) tgoto(fmt, b, a)
#	endif /* HAVE_TPARM */

#	ifdef HAVE_EXTERN_TCAP_PC
extern char PC;			/* used in 'tputs()' */
#	endif /* HAVE_EXTERN_TCAP_PC */

int
get_termcaps(
	void)
{
	static struct {
		char **value;
		char capname[6];
	} table[] = {
		{ &_clearinverse,	CAPNAME("se", "rmso") },
		{ &_clearscreen,	CAPNAME("cl", "clear") },
		{ &_cleartoeoln,	CAPNAME("ce", "el") },
		{ &_cleartoeos,		CAPNAME("cd", "ed") },
		{ &_clearunderline,	CAPNAME("ue", "rmul") },
		{ &_cursoroff,		CAPNAME("vi", "civis") },
		{ &_cursoron,		CAPNAME("ve", "cnorm") },
		{ &_keypadlocal,	CAPNAME("ke", "rmkx") },
		{ &_keypadxmit,		CAPNAME("ks", "smkx") },
		{ &_moveto,		CAPNAME("cm", "cup") },
		{ &_scrollback,		CAPNAME("sr", "ri") },
		{ &_scrollfwd,		CAPNAME("sf", "ind") },
		{ &_scrollregion,	CAPNAME("cs", "csr") },
		{ &_setinverse,		CAPNAME("so", "smso") },
		{ &_setunderline,	CAPNAME("us", "smul") },
		{ &_terminalend,	CAPNAME("te", "rmcup") },
		{ &_terminalinit,	CAPNAME("ti", "smcup") },
		/* extra caps needed for word highlighting */
		{ &_reset,		CAPNAME("me", "sgr0") },
		{ &_reversevideo,	CAPNAME("mr", "rev") },
		{ &_blink,		CAPNAME("mb", "blink") },
		{ &_dim,		CAPNAME("mh", "dim") },
		{ &_bold,		CAPNAME("mb", "bold") },
	};

	static char _terminal[1024];		/* Storage for terminal entry */

#	if defined(USE_TERMCAP)
	static char _capabilities[1024];	/* String for cursor motion */
	static char *ptr = _capabilities;	/* for buffering */
#		if defined(HAVE_EXTERN_TCAP_PC)
	char *t;
#		endif /* HAVE_EXTERN_TCAP_PC */
#	endif /* USE_TERMCAP */

	char the_termname[40], *p;
	unsigned n;

	if ((p = getenv("TERM")) == NULL) {
		my_fprintf(stderr, _(txt_no_term_set), tin_progname);
		return FALSE;
	}

	my_strncpy(the_termname, p, sizeof(the_termname) - 1);

#	ifdef USE_TERMINFO
	setupterm(the_termname, fileno(stdout), (int *) 0);
#	else
	if (tgetent(_terminal, the_termname) != 1) {
		my_fprintf(stderr, _(txt_cannot_get_term_entry), tin_progname);
		return FALSE;
	}
#	endif /* USE_TERMINFO */

	/* load in all those pesky values */
	for (n = 0; n < ARRAY_SIZE(table); n++) {
		*(table[n].value) = TGETSTR(table[n].capname, &ptr);
	}
	_lines = TGETNUM(dCAPNAME("li", "lines"));
	_columns = TGETNUM(dCAPNAME("co", "cols"));
	_hp_glitch = TGETFLAG(dCAPNAME("xs", "xhp"));

#	if defined(USE_TERMCAP) && defined(HAVE_EXTERN_TCAP_PC)
	t = TGETSTR(CAPNAME("pc", "pad"), &p);
	if (t != 0)
		PC = *t;
#	endif /* USE_TERMCAP && HAVE_EXTERN_TCAP_PC */

	if (STRCMPEQ(the_termname, "xterm")) {
		static char x_init[] = "\033[?9h";
		static char x_end[] = "\033[?9l";
		xclicks = TRUE;
		_xclickinit = x_init;
		_xclickend = x_end;
	}

	if (NO_CAP(_clearscreen)) {
		my_fprintf(stderr, _(txt_no_term_clearscreen), tin_progname);
		return FALSE;
	}
	if (NO_CAP(_moveto)) {
		my_fprintf(stderr, _(txt_no_term_cursor_motion), tin_progname);
		return FALSE;
	}
	if (NO_CAP(_cleartoeoln)) {
		my_fprintf(stderr, _(txt_no_term_clear_eol), tin_progname);
		return FALSE;
	}
	if (NO_CAP(_cleartoeos)) {
		my_fprintf(stderr, _(txt_no_term_clear_eos), tin_progname);
		return FALSE;
	}
	if (_lines == -1)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == -1)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;

	if (_lines < MIN_LINES_ON_TERMINAL || _columns < MIN_COLUMNS_ON_TERMINAL) {
		my_fprintf(stderr, _(txt_screen_too_small), tin_progname);
		return FALSE;
	}
	/*
	 * kludge to workaround no inverse
	 */
	if (NO_CAP(_setinverse)) {
		_setinverse = _setunderline;
		_clearinverse = _clearunderline;
		if (NO_CAP(_setinverse))
			tinrc.draw_arrow = 1;
	}
	if (NO_CAP(_scrollregion) || NO_CAP(_scrollfwd) || NO_CAP(_scrollback))
		have_linescroll = FALSE;
	else
		have_linescroll = TRUE;
	return TRUE;
}


int
InitScreen(
	void)
{
	InitWin();
#	ifdef HAVE_COLOR
	postinit_colors();
#	endif /* HAVE_COLOR */
	return TRUE;
}

#else	/* !M_UNIX */

int
InitScreen(
	void)
{
	char *ptr;

	/*
	 * we're going to assume a terminal here...
	 */

	_clearscreen = "\033[1;1H\033[J";
	_moveto = "\033[%d;%dH";	/* not a termcap string! */
	_cleartoeoln = "\033[K";
	_setinverse = "\033[7m";
	_clearinverse = "\033[0m";
	_setunderline = "\033[4m";
	_clearunderline = "\033[0m";
	_keypadlocal = "";
	_keypadxmit = "";
#	ifdef VMS
	_cleartoeos = "\033[J";
	_terminalinit = NULL;
	_terminalend = "";
#	endif /* VMS */
	/* needed for word highlighting */
	_reset = "\033[0m";
	_reversevideo = "\033[7m";
	_blink = "\033[5m";
	_dim = "\033[2m";
	_bold = "\033[1m";

	_lines = _columns = -1;

	if ((ptr = getenv("LINES")) != 0)
		_lines = atol(ptr);

	if ((ptr = getenv("COLUMNS")) != 0)
		_columns = atol(ptr);

	/*
	 * If that failed, try get a response from the console itself
	 */
#	ifdef VMS /* moved from below InitWin() M.St. 22.01.98 */
	{
		int input_chan, status;
		int item_code, eightbit;
		struct sensemode {
			short status;
			unsigned char xmit_baud;
			unsigned char rcv_baud;
			unsigned char crfill;
			unsigned char lffill;
			unsigned char parity;
			unsigned char unused;
			char class;
			char type;
			short scr_wid;
			unsigned long tt_char : 24, scr_len : 8;
			unsigned long tt2_char;
		} tty;
		$DESCRIPTOR(input_dsc, "TT");

		status = SYS$ASSIGN(&input_dsc, &input_chan, 0, 0);
		if (!(status & 1))
			LIB$STOP(status);
		SYS$QIOW(0, input_chan, IO$_SENSEMODE, &tty, 0, 0, &tty.class, 12, 0, 0, 0, 0);
		item_code = DVI$_TT_EIGHTBIT;
		status = LIB$GETDVI(&item_code, &input_chan, 0, &eightbit, 0, 0);
		_columns = tty.scr_wid;
		_lines = tty.scr_len;

		if (eightbit) { /* if using eightbit then use CSI (octal 233) rather than ESC "[" */
			_clearscreen = "\2331;1H\233J";
			_moveto = "\233%d;%dH"; /* not a termcap string! */
			_cleartoeoln = "\233K";
			_cleartoeos = "\233J";
			_setinverse = "\2337m";
			_clearinverse = "\2330m";
			_setunderline = "\2334m";
			_clearunderline = "\2330m";
			_keypadlocal = "";
			_keypadxmit = "";
		} else {
			_clearscreen = "\033[1;1H\033[J";
			_moveto = "\033[%d;%dH";	/* not a termcap string! */
			_cleartoeoln = "\033[K";
			_cleartoeos = "\033[J";
			_setinverse = "\033[7m";
			_clearinverse = "\033[0m";
			_setunderline = "\033[4m";
			_clearunderline = "\033[0m";
			_keypadlocal = "";
			_keypadxmit = "";
		}
#		ifdef HAVE_IS_XTERM
		if (is_xterm()) {
			xclicks = TRUE;
			if (!eightbit) {
				_xclickinit = "\033[?9h";
				_xclickend = "\033[?9l";
			}
#			if 0
			else {
				/*
				 * These are the settings for a DECterm but the reply can't easily be parsed
				 * Reply is of the form - CSI Pe ; Pb ; Pr ; Pc & w
				 * Where Pe is the event, Pb the button, Pr and Pc the row and column
				 */
				_xclickinit = "\2331;2'z";
				_xclickend = "\2330;0'z";
			}
#			endif /* 0 */
		}
#		endif /* HAVE_IS_XTERM */
	}
#	endif /* VMS */

	if (_lines < MIN_LINES_ON_TERMINAL || _columns < MIN_COLUMNS_ON_TERMINAL) {
		my_fprintf(stderr, _(txt_screen_too_small), tin_progname);
		return FALSE;
	}

	InitWin();

	Raw(FALSE);

	have_linescroll = FALSE;
	return TRUE;
}

#endif /* M_UNIX */


void
InitWin(
	void)
{
	if (_terminalinit) {
		tputs(_terminalinit, 1, outchar);
		my_flush();
	}
	set_keypad_on();
	set_xclick_on();
}


void
EndWin(
	void)
{
	if (_terminalend) {
		tputs(_terminalend, 1, outchar);
		my_flush();
	}
	set_keypad_off();
	set_xclick_off();
}


void
set_keypad_on(
	void)
{
#	ifdef HAVE_KEYPAD
	if (tinrc.use_keypad && _keypadxmit) {
		tputs(_keypadxmit, 1, outchar);
		my_flush();
	}
#	endif /* HAVE_KEYPAD */
}


void
set_keypad_off(
	void)
{
#	ifdef HAVE_KEYPAD
	if (tinrc.use_keypad && _keypadlocal) {
		tputs(_keypadlocal, 1, outchar);
		my_flush();
	}
#	endif /* HAVE_KEYPAD */
}


/*
 * clear the screen
 */
void
ClearScreen(
	void)
{
	tputs(_clearscreen, 1, outchar);
	my_flush();		/* clear the output buffer */
	_line = 1;
}


/*
 *  move cursor to the specified row column on the screen.
 *  0,0 is the top left!
 */
#ifdef M_UNIX
void
MoveCursor(
	int row,
	int col)
{
	char *stuff;

	stuff = tgoto(_moveto, col, row);
	tputs(stuff, 1, outchar);
	my_flush();
	_line = row + 1;
}

#else	/* !M_UNIX */

void
MoveCursor(
	int row,
	int col)
{
	char stuff[12];

	if (_moveto) {
		snprintf(stuff, sizeof(stuff), _moveto, row + 1, col + 1);
		tputs(stuff, 1, outchar);
		my_flush();
		_line = row + 1;
	}
}
#endif /* M_UNIX */


/*
 * clear to end of line
 */
void
CleartoEOLN(
	void)
{
	tputs(_cleartoeoln, 1, outchar);
	my_flush();	/* clear the output buffer */
}


/*
 * clear to end of screen
 */
void
CleartoEOS(
	void)
{
	int i;

	if (_cleartoeos) {
		tputs(_cleartoeos, 1, outchar);
	} else {
		for (i = _line - 1; i < _lines; i++) {
			MoveCursor(i, 0);
			CleartoEOLN();
		}
	}
	my_flush();	/* clear the output buffer */
}


static int _topscrregion, _bottomscrregion;

void
SetScrollRegion(
	int topline,
	int bottomline)
{
	char *stuff;

	if (!have_linescroll)
		return;
	if (_scrollregion) {
		stuff = TFORMAT(_scrollregion, topline, bottomline);
		tputs(stuff, 1, outchar);
		_topscrregion = topline;
		_bottomscrregion = bottomline;
	}
	my_flush();
}


void
ScrollScreen(
	int lines_to_scroll)
{
	int i;

	if (!have_linescroll || (lines_to_scroll == 0))
		return;
	if (lines_to_scroll < 0) {
		if (_scrollback) {
			i = lines_to_scroll;
			while (i++) {
				MoveCursor(_topscrregion, 0);
				tputs(_scrollback, 1, outchar);
			}
		}
	} else
		if (_scrollfwd) {
			i = lines_to_scroll;
			while (i--) {
				MoveCursor(_bottomscrregion, 0);
				tputs(_scrollfwd, 1, outchar);
			}
		}
	my_flush();
}


/*
 * set inverse video mode
 */
void
StartInverse(
	void)
{
/*	in_inverse = 1; */
	if (_setinverse && tinrc.inverse_okay) {
#	ifdef HAVE_COLOR
		if (use_color) {
			bcol(tinrc.col_invers_bg);
			fcol(tinrc.col_invers_fg);
		} else {
			tputs(_setinverse, 1, outchar);
		}
#	else
		tputs(_setinverse, 1, outchar);
#	endif /* HAVE_COLOR */
	}
	my_flush();
}


/*
 * compliment of startinverse
 */
void
EndInverse(
	void)
{
/*	in_inverse = 0; */
	if (_clearinverse && tinrc.inverse_okay) {
#	ifdef HAVE_COLOR
		if (use_color) {
			fcol(tinrc.col_normal);
			bcol(tinrc.col_back);
		} else {
			tputs(_clearinverse, 1, outchar);
		}
#	else
		tputs(_clearinverse, 1, outchar);
#	endif /* HAVE_COLOR */
	}
	my_flush();
}


#if 0 /* doesn't work correct with ncurses4.x */
/*
 * toggle inverse video mode
 */
void
ToggleInverse(
	void)
{
	if (!in_inverse)
		StartInverse();
	else
		EndInverse();
}
#endif /* 0 */


/*
 * returns either 1 or 0, for ON or OFF
 */
int
RawState(
	void)
{
	return _inraw;
}


/*
 * state is either TRUE or FALSE, as indicated by call
 */
void
Raw(
	int state)
{
#ifdef VMS
	if (!state && _inraw) {
/*		vmsnoraw(); */
		_inraw = 0;
	} else if (state && !_inraw) {
/*		vmsraw(); */
		_inraw = 1;
	}
#else
	if (!state && _inraw) {
		SET_TTY(&_original_tty);
		_inraw = 0;
	} else if (state && !_inraw) {
		GET_TTY(&_original_tty);
		GET_TTY(&_raw_tty);
#		if USE_SGTTY
		_raw_tty.sg_flags &= ~(ECHO | CRMOD);	/* echo off */
		_raw_tty.sg_flags |= CBREAK;		/* raw on */
#		else
#			ifdef __FreeBSD__
		cfmakeraw(&_raw_tty);
		_raw_tty.c_lflag |= ISIG;		/* for ^Z */
#			else
		_raw_tty.c_lflag &= ~(ICANON | ECHO);	/* noecho raw mode */
		_raw_tty.c_cc[VMIN] = '\01';	/* minimum # of chars to queue */
		_raw_tty.c_cc[VTIME] = '\0';	/* minimum time to wait for input */
#			endif /* __FreeBSD__ */
#		endif /* USE_SGTTY */
		SET_TTY(&_raw_tty);
		_inraw = 1;
	}
#endif /* !VMS */
}


/*
 *  output a character. From tputs... (Note: this CANNOT be a macro!)
 */
OUTC_FUNCTION(
	outchar)
{
#ifdef OUTC_RETURN
	return fputc(c, stdout);
#else
	(void) fputc(c, stdout);
#endif /* OUTC_RETURN */
}


/*
 * setup to monitor mouse buttons if running in a xterm
 */
static void
xclick(
	int state)
{
	static int prev_state = 999;

	if (xclicks && prev_state != state) {
		if (state) {
			tputs(_xclickinit, 1, outchar);
		} else {
			tputs(_xclickend, 1, outchar);
		}
		my_flush();
		prev_state = state;
	}
}


/*
 * switch on monitoring of mouse buttons
 */
void
set_xclick_on(
	void)
{
	if (tinrc.use_mouse)
		xclick(TRUE);
}


/*
 * switch off monitoring of mouse buttons
 */
void
set_xclick_off(
	void)
{
	if (tinrc.use_mouse)
		xclick(FALSE);
}


void
cursoron(
	void)
{
	if (_cursoron)
		tputs(_cursoron, 1, outchar);
}


void
cursoroff(
	void)
{
	if (_cursoroff)
		tputs(_cursoroff, 1, outchar);
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
	char output[LEN];

	my_strncpy(output, &(screen[row].col[col]), size);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	/*
	 * In a multibyte locale we get byte offsets instead of character
	 * offsets; calculate now the correct starting column
	 */
	if (col > 0) {
		char tmp[LEN];
		wchar_t *wtmp;

		my_strncpy(tmp, &(screen[row].col[0]), sizeof(tmp) - 1);
		tmp[col] = '\0';
		if ((wtmp = char2wchar_t(tmp)) != NULL) {
			col = wcswidth(wtmp, wcslen(wtmp) + 1);
			free(wtmp);
		}
	}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	MoveCursor(row, col);
	StartInverse();
	my_fputs(output, stdout);
	my_flush();
	EndInverse();

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
	/*
	 * Mapping of the tinrc.mono_mark* values to the corresponding escape sequences
	 */
	char *attributes[MAX_ATTR + 1];
	char *dest, *src;
	char output[LEN];
	int byte_offset = col;
	int wsize = size;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	attributes[0] = _reset;	/* Normal */
	attributes[1] = _setinverse;	/* Best highlighting */
	attributes[2] = _setunderline;	/* Underline */
	attributes[3] = _reversevideo;	/* Reverse video */
	attributes[4] = _blink;	/* Blink */
	attributes[5] = _dim;	/* Dim */
	attributes[6] = _bold;	/* Bold */

	my_strncpy(output, &(screen[row].col[byte_offset]), size);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	/*
	 * In a multibyte locale we get byte offsets instead of character
	 * offsets; calculate now the correct starting column and
	 * width
	 */
	if (byte_offset > 0) {
		char tmp[LEN];
		my_strncpy(tmp, &(screen[row].col[0]), sizeof(tmp) - 1);
		tmp[byte_offset] = '\0';
		if ((wtmp = char2wchar_t(tmp)) != NULL) {
			col = wcswidth(wtmp, wcslen(wtmp) + 1);
			free(wtmp);
		}
	}
	if ((wtmp = char2wchar_t(output)) != NULL) {
		wsize = wcswidth(wtmp, wcslen(wtmp) + 1);
		free(wtmp);
	}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	MoveCursor(row, col);

	/* safeguard against bogus regexps */
	if ((output[0] == '*' && output[size - 1] == '*') ||
		 (output[0] == '/' && output[size - 1] == '/') ||
		 (output[0] == '_' && output[size - 1] == '_') ||
		 (output[0] == '-' && output[size - 1] == '-')) {

		switch (tinrc.word_h_display_marks) {
			case 0: /* remove marks */
				MoveCursor(row, col + wsize - 2);
				CleartoEOLN();
				my_fputs(&(screen[row].col[byte_offset + size]), stdout);
				output[0] = output[size - 1] = ' ';
				str_trim(output);
				strncpy(&(screen[row].col[byte_offset]), output, size - 2);
				src = &(screen[row].col[byte_offset + size]);
				dest = &(screen[row].col[byte_offset + size - 2]);
				while (*src)
					*dest++ = *src++;
				*dest++ = '\0';
				MoveCursor(row, col);
				break;

			case 2: /* print space */
				MoveCursor(row, col + wsize - 1);
				my_fputs(" ", stdout);
				MoveCursor(row, col);
				my_fputs(" ", stdout);
				output[0] = output[size - 1] = ' ';
				str_trim(output);
				screen[row].col[byte_offset] = ' ';
				screen[row].col[byte_offset + size - 1] = ' ';
				break;

			default:	/* print mark (case 1) */
				break;
		}
	}
#	ifdef HAVE_COLOR
	if (use_color)
		fcol(color);
	else
#	endif /* HAVE_COLOR */
		if (color > 0 && color <= MAX_ATTR && !NO_CAP(attributes[color]))
			tputs(attributes[color], 1, outchar);
	my_fputs(output, stdout);
	my_flush();
#	ifdef HAVE_COLOR
	if (use_color)
		fcol(tinrc.col_text);
	else
#	endif /* HAVE_COLOR */
		if (!NO_CAP(_reset))
			tputs(_reset, 1, outchar);
	stow_cursor();
}


/*
 * input_pending() waits for input during time given
 * by delay in msec. The original behavior of input_pending()
 * (in art.c's threading code) is delay=0
 */
static int
input_pending(
	int delay)
{
#if 0
	int ch;
	nodelay(stdscr, TRUE);
	if ((ch = getch()) != ERR)
		ungetch(ch);
	nodelay(stdscr, FALSE);
	return (ch != ERR);

#else

#	ifdef HAVE_SELECT
	int fd = STDIN_FILENO;
	fd_set fdread;
	struct timeval tvptr;

	FD_ZERO(&fdread);

	tvptr.tv_sec = 0;
	tvptr.tv_usec = delay * 100;

	FD_SET(fd, &fdread);

#		ifdef HAVE_SELECT_INTP
	if (select(1, (int *) &fdread, NULL, NULL, &tvptr))
#		else
	if (select(1, &fdread, NULL, NULL, &tvptr))
#		endif /* HAVE_SELECT_INTP */
	{
		if (FD_ISSET(fd, &fdread))
			return TRUE;
	}
#	endif /* HAVE_SELECT */

#	if defined(HAVE_POLL) && !defined(HAVE_SELECT)
	static int Timeout;
	static long nfds = 1;
	static struct pollfd fds[] = {{ STDIN_FILENO, POLLIN, 0 }};

	Timeout = delay;
	if (poll(fds, nfds, Timeout) < 0) /* Error on poll */
		return FALSE;

	switch (fds[0].revents) {
		case POLLIN:
			return TRUE;
		/*
		 * Other conditions on the stream
		 */
		case POLLHUP:
		case POLLERR:
		default:
			return FALSE;
	}
#	endif /* HAVE_POLL && !HAVE_SELECT */

#endif /* 0 */

	return FALSE;
}


int
get_arrow_key(
	int prech)
{
	int ch;
	int ch1;

#define wait_a_while(i) \
	while (!input_pending(0) \
		&& i < ((VT_ESCAPE_TIMEOUT * 1000) / SECOND_CHARACTER_DELAY))

#	ifndef VMS
	if (!input_pending(0)) {
#			ifdef HAVE_USLEEP
		int i = 0;

		wait_a_while(i) {
			usleep((unsigned long) (SECOND_CHARACTER_DELAY * 1000));
			i++;
		}
#			else	/* !HAVE_USLEEP */
#				ifdef HAVE_SELECT
		struct timeval tvptr;
		int i = 0;

		wait_a_while(i) {
			tvptr.tv_sec = 0;
			tvptr.tv_usec = SECOND_CHARACTER_DELAY * 1000;
			select(0, NULL, NULL, NULL, &tvptr);
			i++;
		}
#				else /* !HAVE_SELECT */
#					ifdef HAVE_POLL
		struct pollfd fds[1];
		int i = 0;

		wait_a_while(i) {
			poll(fds, 0, SECOND_CHARACTER_DELAY);
			i++;
		}
#					else /* !HAVE_POLL */
		(void) sleep(1);
#					endif /* HAVE_POLL */
#				endif /* HAVE_SELECT */
#			endif /* HAVE_USLEEP */
		if (!input_pending(0))
			return prech;
	}
#	endif /* !VMS */
	ch = ReadCh();
	if (ch == '[' || ch == 'O')
		ch = ReadCh();

	switch (ch) {
		case 'A':
		case 'i':
#	ifdef QNX42
		case 0xA1:
#	endif /* QNX42 */
			return KEYMAP_UP;

		case 'B':
#	ifdef QNX42
		case 0xA9:
#	endif /* QNX42 */
			return KEYMAP_DOWN;

		case 'D':
#	ifdef QNX42
		case 0xA4:
#	endif /* QNX42 */
			return KEYMAP_LEFT;

		case 'C':
#	ifdef QNX42
		case 0xA6:
#	endif /* QNX42 */
			return KEYMAP_RIGHT;

		case 'I':		/* ansi  PgUp */
		case 'V':		/* at386 PgUp */
		case 'S':		/* 97801 PgUp */
		case 'v':		/* emacs style */
#	ifdef QNX42
		case 0xA2:
#	endif /* QNX42 */
			return KEYMAP_PAGE_UP;

		case 'G':		/* ansi  PgDn */
		case 'U':		/* at386 PgDn */
		case 'T':		/* 97801 PgDn */
#	ifdef QNX42
		case 0xAA:
#	endif /* QNX42 */
			return KEYMAP_PAGE_DOWN;

		case 'H':		/* at386 Home */
#	ifdef QNX42
		case 0xA0:
#	endif /* QNX42 */
			return KEYMAP_HOME;

		case 'F':		/* ansi  End */
		case 'Y':		/* at386 End */
#	ifdef QNX42
		case 0xA8:
#	endif /* QNX42 */
			return KEYMAP_END;

		case '2':		/* vt200 Ins */
			(void) ReadCh();	/* eat the ~ */
			return KEYMAP_INS;

		case '3':		/* vt200 Del */
			(void) ReadCh();	/* eat the ~ */
			return KEYMAP_DEL;

		case '5':		/* vt200 PgUp */
			(void) ReadCh();	/* eat the ~ (interesting use of words :) */
			return KEYMAP_PAGE_UP;

		case '6':		/* vt200 PgUp */
			(void) ReadCh();	/* eat the ~ */
			return KEYMAP_PAGE_DOWN;

		case '1':		/* vt200 PgUp */
			ch = ReadCh(); /* eat the ~ */
			if (ch == '5') {	/* RS/6000 PgUp is 150g, PgDn is 154g */
				ch1 = ReadCh();
				(void) ReadCh();
				if (ch1 == '0')
					return KEYMAP_PAGE_UP;
				if (ch1 == '4')
					return KEYMAP_PAGE_DOWN;
			}
			return KEYMAP_HOME;

		case '4':		/* vt200 PgUp */
			(void) ReadCh();	/* eat the ~ */
			return KEYMAP_END;

		case 'M':		/* xterminal button press */
			xmouse = ReadCh() - ' ';	/* button */
			xcol = ReadCh() - '!';		/* column */
			xrow = ReadCh() - '!';		/* row */
			return KEYMAP_MOUSE;

		default:
			return KEYMAP_UNKNOWN;
	}
}
#endif /* USE_CURSES */


/*
 * The UNIX version of ReadCh, termcap version
 */
#ifdef M_UNIX
int
ReadCh(
	void)
{
	int result;
#	ifndef READ_CHAR_HACK
	char ch;
#	endif /* !READ_CHAR_HACK */

	fflush(stdout);
#	ifdef READ_CHAR_HACK
#		undef getc
	while ((result = getc(stdin)) == EOF) {
		if (feof(stdin))
			break;

#		ifdef EINTR
		if (ferror(stdin) && errno != EINTR)
#		else
		if (ferror(stdin))
#		endif /* EINTR */
			break;

		clearerr(stdin);
	}

	return ((result == EOF) ? EOF : result & 0xFF);

#	else
#		ifdef EINTR

	allow_resize(TRUE);
	while ((result = read(0, &ch, 1)) < 0 && errno == EINTR) {		/* spin on signal interrupts */
		if (need_resize) {
			handle_resize((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
		}
	}
	allow_resize(FALSE);
#		else
	result = read(0, &ch, 1);
#		endif /* EINTR */

	return ((result <= 0) ? EOF : ch & 0xFF);

#	endif /* READ_CHAR_HACK */
}


#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
/*
 * A helper function for ReadWch()
 * converts the read input to a wide-char
 */
static wint_t
convert_c2wc(
	const char *input)
{
	int res;
	wchar_t wc;

	res = mbtowc(&wc, input, MB_CUR_MAX);
	if (res == -1)
		return WEOF;
	else
		return (wint_t) wc;
}


wint_t
ReadWch(
	void)
{
	char *mbs = my_malloc(MB_CUR_MAX + 1);
	int result, to_read;
	wchar_t wch = 0;

	fflush(stdout);

	/*
	 * Independent of the pressed key and the used charset we have to
	 * read at least one byte. The further processing is depending of
	 * the read byte and the used charset.
	 */
#		ifdef EINTR
	allow_resize(TRUE);
	while ((result = read(0, mbs, 1)) < 0 && errno == EINTR) { /* spin on signal interrupts */
		if (need_resize) {
			handle_resize((need_resize == cRedraw) ? TRUE :  FALSE);
			need_resize = cNo;
		}
	}
	allow_resize(FALSE);
#		else
	result = read(0, mbs, 1);
#		endif /* EINTR */

	if (result <= 0) {
		free(mbs);
		return WEOF;
	}

	/* Begin of an ESC-sequence. Let get_arrow_key() figure out which it is */
	if (mbs[0] == ESC) {
		free(mbs);
		return (wint_t) ESC;
	}

	/*
	 * In an one-byte charset we don't need to read further chars but
	 * we still need to convert the char to a correct wide-char
	 */
	if (MB_CUR_MAX == 1) {
		mbs[1] = '\0';
		wch = convert_c2wc(mbs);
		free(mbs);

		return (wint_t) wch;
	}

	/*
	 * we use a multi-byte charset
	 */

	/*
	 * UTF-8
	 */
	if (!strcasecmp(tinrc.mm_local_charset, "UTF-8")) {
		int ch = mbs[0] & 0xFF;

		/* determine the count of bytes we have still have to read */
		if (ch <= 0x7F) {
			/* ASCII char */
			to_read = 0;
		} else if (ch >= 0xC2 && ch <= 0xDF) {
			/* 2-byte sequence */
			to_read = 1;
		} else if (ch >= 0xE0 && ch <= 0xEF) {
			/* 3-byte sequence */
			to_read = 2;
		} else if (ch >= 0xF0 && ch <= 0xF4) {
			/* 4-byte sequence */
			to_read = 3;
		} else {
			/* invalid sequence */
			free(mbs);
			return WEOF;
		}

		/* read the missing bytes of the multi-byte sequence */
		if (to_read > 0) {
#		ifdef EINTR
			allow_resize(TRUE);
			while ((result = read(0, mbs + 1, to_read)) < 0 && errno == EINTR) { /* spin on signal interrupts */
				if (need_resize) {
					handle_resize((need_resize == cRedraw) ? TRUE :  FALSE);
					need_resize = cNo;
				}
			}
			allow_resize(FALSE);
#		else
			result = read(0, mbs + 1, to_read);
#		endif /* EINTR */
			if (result < 0) {
				free(mbs);

				return WEOF;
			}
		}
		mbs[to_read + 1] = '\0';
		wch = convert_c2wc(mbs);
		free (mbs);

		return (wint_t) wch;

	}

	/* FIXME: add support for other multi-byte charsets */

	return WEOF;
}
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#endif /* M_UNIX */
