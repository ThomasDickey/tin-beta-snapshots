/*
 *  Project   : tin - a Usenet reader
 *  Module    : curses.c
 *  Author    : D. Taylor & I. Lea
 *  Created   : 1986-01-01
 *  Updated   : 2000-04-14
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

#ifdef USE_CURSES

#define ReadCh cmdReadCh

void my_dummy(void) { }	/* ANSI C requires non-empty file */
t_bool have_linescroll = TRUE;	/* USE_CURSES always allows line scrolling */

#else	/* !USE_CURSES */

#ifdef M_AMIGA
#	undef BSD
#endif /* M_AMIGA */

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
int cCOLS  = DEFAULT_COLUMNS_ON_TERMINAL;
static int _inraw = FALSE;	/* are we IN rawmode? */
int _hp_glitch = FALSE;		/* standout not erased by overwriting on HP terms */
t_bool have_linescroll = FALSE;

static int xclicks=FALSE;	/* do we have an xterm? */

#define BACKSPACE        '\b'
#define VERY_LONG_STRING 2500
#define TTYIN            0

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
#			else
				please-fix-me(thanks)
#			endif /* HAVE_SGTTY_H */
#		endif /* HAVE_TERMIO_H */
#	endif /* HAVE_TERMIOS_H && HAVE_TCGETATTR && HAVE_TCSETATTR */

static TTY _raw_tty, _original_tty;

#else	/* FIXME: prune the non-autoconf'd stuff */

#	if (defined(M_AMIGA) && !defined(__SASC)) || defined(COHERENT) || defined(BSD)
#		ifdef BSD4_1
#			include <termio.h>
#			define USE_TERMIO 1
#		else
#			include <sgtty.h>
#			define USE_SGTTY 1
#		endif /* BSD4_1 */
#	else
#		if !defined(SYSV) && !defined(M_AMIGA)
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
#		endif /* !SYSV && !M_AMIGA */
#	endif /* (M_AMIGA && !__SASC) || COHERENT || BSD */

#	ifndef VMS
#		if (defined(M_AMIGA) && !defined(__SASC)) || defined(BSD) || defined(MINIX)
#			define USE_SGTTY 1
struct sgttyb _raw_tty, _original_tty;
#		else
#			if !defined(M_AMIGA) && !defined(M_OS2)
#				if defined(HAVE_TERMIOS_H) || defined(sinix)
#					define USE_POSIX_TERMIOS 1
struct termios _raw_tty, _original_tty;
#				else
#					define USE_TERMIO 1
struct termio _raw_tty, _original_tty;
#				endif /* HAVE_TERMIOS_H || sinix */
#			endif /* !M_AMIGA && ! M_OS2 */
#		endif /* (M_AMIGA && !__SASC) || BSD || MINIX */
#	endif /* !VMS */
#endif /* HAVE_CONFIG_H */

#ifndef USE_SGTTY
#	define USE_SGTTY 0
#endif /* !USE_SGTTY */

#ifndef USE_TERMIO
#	define USE_TERMIO 0
#endif /* !USE_TERMIO */

#ifndef USE_POSIX_TERMIOS
#	define USE_POSIX_TERMIOS 0
#endif /* !USE_POSIX_TERMIOS */

static char *_clearscreen, *_moveto, *_cleartoeoln, *_cleartoeos,
			*_setinverse, *_clearinverse, *_setunderline, *_clearunderline,
			*_xclickinit, *_xclickend, *_cursoron, *_cursoroff,
			*_terminalinit, *_terminalend, *_keypadlocal, *_keypadxmit,
			*_scrollregion, *_scrollfwd, *_scrollback;

#ifdef M_AMIGA
static char *_getwinsize;
#endif /* M_AMIGA */

static int _columns, _line, _lines;

#ifdef M_UNIX

#	if USE_POSIX_TERMIOS
#		define SET_TTY(arg) tcsetattr (TTYIN, TCSANOW, arg)
#		define GET_TTY(arg) tcgetattr (TTYIN, arg)
#	else
#		if USE_TERMIO
#			define SET_TTY(arg) ioctl (TTYIN, TCSETAW, arg)
#			define GET_TTY(arg) ioctl (TTYIN, TCGETA, arg)
#		else
#			if USE_SGTTY
#				define SET_TTY(arg) stty(TTYIN, arg)
#				define GET_TTY(arg) gtty(TTYIN, arg)
#			else
				please-fix-me(thanks)
#			endif /* USE_SGTTY */
#		endif /* USE_TERMIO */
#	endif /* USE_POSIX_TERMIOS */

#endif /* M_UNIX */

static int in_inverse;			/* 1 when in inverse, 0 otherwise */

/*
 * Local prototypes
 */
static void ScreenSize (int *num_lines, int *num_columns);
static int input_pending (int delay);


/*
 *  returns the number of lines and columns on the display.
 */
static void
ScreenSize (
	int *num_lines,
	int *num_columns)
{
	if (!_lines)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (!_columns)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;

	*num_lines = _lines - 1;		/* assume index from zero*/
	*num_columns = _columns;		/* assume index from one */
}


/*
 * get screen size from termcap entry & setup sizes
 */
void
setup_screen (void)
{
	_line = 1;
	ScreenSize (&cLINES, &cCOLS);
	cmd_line = FALSE;
	Raw (TRUE);
	set_win_size (&cLINES, &cCOLS);
}

#ifdef M_UNIX

#ifdef USE_TERMINFO
#	define TGETSTR(a,b, bufp) tigetstr(b, bufp)
#	define TGETNUM(a,b)       tigetnum(b) /* may be tigetint() */
#	define TGETFLAG(a,b)      tigetflag(b)
#	define NO_CAP(s)           (s == 0 || s == (char *)-1)
#	if !defined(HAVE_TIGETNUM) && defined(HAVE_TIGETINT)
#		define tigetnum tigetint
#	endif /* !HAVE_TIGETNUM && HAVE_TIGETINT */
#else /* USE_TERMCAP */
#	undef USE_TERMCAP
#	define USE_TERMCAP 1
#	define TGETSTR(a,b, bufp) tgetstr(a, bufp)
#	define TGETNUM(a,b)       tgetnum(a)
#	define TGETFLAG(a,b)      tgetflag(a)
#	define NO_CAP(s)           (s == 0)
#endif /* USE_TERMINFO */

#ifdef HAVE_TPARM
#define TFORMAT(fmt, a, b) tparm(fmt, b, a)
#else
#define TFORMAT(fmt, a, b) tgoto(fmt, b, a)
#endif /* HAVE_TPARM */

#ifdef HAVE_EXTERN_TCAP_PC
extern char PC;			/* used in 'tputs()' */
#endif /* HAVE_EXTERN_TCAP_PC */

int
get_termcaps (void)
{
	static char _terminal[1024];		/* Storage for terminal entry */
	static char _capabilities[1024];	/* String for cursor motion */
	static char *ptr = _capabilities;	/* for buffering */

	char the_termname[40], *p;

	if ((p = getenv ("TERM")) == (char *) 0) {
		my_fprintf (stderr, _(txt_no_term_set), tin_progname);
		return (FALSE);
	}
	STRCPY(the_termname, p);
	if (tgetent (_terminal, the_termname) != 1) {
		my_fprintf (stderr, _(txt_cannot_get_term_entry), tin_progname);
		return (FALSE);
	}

	/* load in all those pesky values */
	_clearscreen    = TGETSTR ("cl", "clear", &ptr);
	_moveto         = TGETSTR ("cm", "cup",   &ptr);
	_cleartoeoln    = TGETSTR ("ce", "el",    &ptr);
	_cleartoeos     = TGETSTR ("cd", "ed",    &ptr);
	_lines          = TGETNUM ("li", "lines");
	_columns        = TGETNUM ("co", "cols");
	_setinverse     = TGETSTR ("so", "smso",  &ptr);
	_clearinverse   = TGETSTR ("se", "rmso",  &ptr);
	_setunderline   = TGETSTR ("us", "smul",  &ptr);
	_clearunderline = TGETSTR ("ue", "rmul",  &ptr);
	_scrollregion   = TGETSTR ("cs", "csr",   &ptr);
	_scrollfwd      = TGETSTR ("sf", "ind",   &ptr);
	_scrollback     = TGETSTR ("sr", "ri",    &ptr);
	_hp_glitch      = TGETFLAG("xs", "xhp");
#ifdef HAVE_BROKEN_TGETSTR
	_terminalinit   = "";
	_terminalend    = "";
	_keypadlocal    = "";
	_keypadxmit     = "";
#else
	_terminalinit   = TGETSTR ("ti", "smcup", &ptr);
	_terminalend    = TGETSTR ("te", "rmcup", &ptr);
	_keypadlocal    = TGETSTR ("ke", "rmkx",  &ptr);
	_keypadxmit     = TGETSTR ("ks", "smkx",  &ptr);
#endif /* HAVE_BROKEN_TGETSTR */
	_cursoron       = TGETSTR ("ve", "cnorm", &ptr);
	_cursoroff      = TGETSTR ("vi", "civis", &ptr);

#ifdef USE_TERMCAP
#	ifdef HAVE_EXTERN_TCAP_PC
	t = TGETSTR("pc", "pad", &p);
	if (t != 0)
		PC = *t;
#	endif /* HAVE_EXTERN_TCAP_PC */
#endif /* USE_TERMCAP */

	if (STRCMPEQ(the_termname, "xterm")) {
		static char x_init[] = "\033[?9h";
		static char x_end[]  = "\033[?9l";
		xclicks = TRUE;
		_xclickinit	= x_init;
		_xclickend	= x_end;
	}

	if (NO_CAP(_clearscreen)) {
		my_fprintf (stderr, _(txt_no_term_clearscreen), tin_progname);
		return (FALSE);
	}
	if (NO_CAP(_moveto)) {
		my_fprintf (stderr, _(txt_no_term_cursor_motion), tin_progname);
		return (FALSE);
	}
	if (NO_CAP(_cleartoeoln)) {
		my_fprintf (stderr, _(txt_no_term_clear_eol), tin_progname);
		return (FALSE);
	}
	if (NO_CAP(_cleartoeos)) {
		my_fprintf (stderr, _(txt_no_term_clear_eos), tin_progname);
		return (FALSE);
	}
	if (_lines == -1)
		_lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == -1)
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;

	if (_lines < MIN_LINES_ON_TERMINAL || _columns < MIN_COLUMNS_ON_TERMINAL) {
		my_fprintf(stderr, _(txt_screen_too_small), tin_progname);
		return (FALSE);
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
	return (TRUE);
}

int
InitScreen (void)
{
	InitWin ();
#ifdef HAVE_COLOR
	postinit_colors();
#endif /* HAVE_COLOR */
	return (TRUE);
}

#else	/* !M_UNIX */

int
InitScreen (void)
{
	char *ptr;

	/*
	 * we're going to assume a terminal here...
	 */

	_clearscreen	= "\033[1;1H\033[J";
	_moveto		= "\033[%d;%dH";	/* not a termcap string! */
	_cleartoeoln	= "\033[K";
	_setinverse	= "\033[7m";
	_clearinverse	= "\033[0m";
	_setunderline	= "\033[4m";
	_clearunderline	= "\033[0m";
	_keypadlocal	= "";
	_keypadxmit	= "";
#ifdef M_AMIGA
	_terminalinit	= "\033[12{\033[0 p";
	_terminalend	= "\033[12}\033[ p";
	_cursoron	= "\033[ p";
	_cursoroff	= "\033[0 p";
	_cleartoeos	= "\033[J";
	_getwinsize	= "\2330 q";
#endif /* M_AMIGA */
#ifdef M_OS2
	_cleartoeos	= NULL;
	_terminalinit	= NULL;
	_terminalend	= "";
	initscr ();
#endif /* M_OS2 */
#ifdef VMS
	_cleartoeos	= "\033[J";
	_terminalinit	= NULL;
	_terminalend	= "";
#endif /* VMS */

	_lines = _columns = -1;

	/*
	 * Get lines and columns from environment settings - useful when
	 * you're using something other than an Amiga window
	 */

	if ((ptr = getenv ("LINES")) != 0) {
		_lines = atol (ptr);
	}
	if ((ptr = getenv ("COLUMNS")) != 0) {
		_columns = atol (ptr);
	}

	/*
	 * If that failed, try get a response from the console itself
	 */
#ifdef M_AMIGA
	if (_lines == -1 || _columns == -1) {
		_lines = DEFAULT_LINES_ON_TERMINAL;
		_columns = DEFAULT_COLUMNS_ON_TERMINAL;
	} else {
		_terminalinit = NULL;		/* don't do fancy things on a non-amiga console */
		_terminalend = NULL;
		_cursoroff = NULL;
		_cursoron = NULL;
		_getwinsize = NULL;
	}
#endif /* M_AMIGA */
#ifdef M_OS2
	if (_lines == -1 || _columns == -1) {
		_lines = LINES;
		_columns = COLS;
	}
#endif /* M_OS2 */
#ifdef VMS /* moved from below InitWin () M.St. 22.01.98 */
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
		$DESCRIPTOR (input_dsc, "TT");

		status = SYS$ASSIGN (&input_dsc, &input_chan, 0, 0);
		if (!(status & 1))
			LIB$STOP (status);
		SYS$QIOW (0, input_chan, IO$_SENSEMODE, &tty, 0, 0, &tty.class, 12, 0, 0, 0, 0);
		item_code = DVI$_TT_EIGHTBIT;
		status = LIB$GETDVI(&item_code, &input_chan, 0, &eightbit, 0, 0);
		_columns = tty.scr_wid;
		_lines = tty.scr_len;

		if (eightbit) { /* if using eightbit then use CSI (octal 233) rather than ESC "[" */
			_clearscreen = "\2331;1H\233J";
			_moveto = "\233%d;%dH"; /* not a termcap string !*/
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
#ifdef HAVE_IS_XTERM
		if (is_xterm()) {
			xclicks = TRUE;
			if (!eightbit) {
				_xclickinit = "\033[?9h";
				_xclickend  = "\033[?9l";
			}
#if 0
			else {
				/*
				 * These are the settings for a DECterm but the reply can't easily be parsed
				 * Reply is of the form - CSI Pe ; Pb ; Pr ; Pc & w
				 * Where Pe is the event, Pb the button, Pr and Pc the row and column
				 */
				_xclickinit = "\2331;2'z";
				_xclickend = "\2330;0'z";
			}
#endif /* 0 */
		}
#endif /* HAVE_IS_XTERM */
	}
#endif /* VMS */

	if (_lines < MIN_LINES_ON_TERMINAL || _columns < MIN_COLUMNS_ON_TERMINAL) {
		my_fprintf(stderr, _(txt_screen_too_small), tin_progname);
		return (FALSE);
	}

	InitWin ();

	Raw (FALSE);

	have_linescroll = FALSE;
	return (TRUE);
}

#endif /* M_UNIX */


void
InitWin (void)
{
	if (_terminalinit) {
		tputs (_terminalinit, 1, outchar);
		my_flush();
	}
	set_keypad_on ();
	set_xclick_on ();
}


void
EndWin (void)
{
	if (_terminalend) {
		tputs (_terminalend, 1, outchar);
		my_flush();
	}
	set_keypad_off ();
	set_xclick_off ();
}


void
set_keypad_on (void)
{
#	ifdef HAVE_KEYPAD
	if (tinrc.use_keypad && _keypadxmit) {
		tputs (_keypadxmit, 1, outchar);
		my_flush();
	}
#	endif /* HAVE_KEYPAD */
}


void
set_keypad_off (void)
{
#	ifdef HAVE_KEYPAD
	if (tinrc.use_keypad && _keypadlocal) {
		tputs (_keypadlocal, 1, outchar);
		my_flush();
	}
#	endif /* HAVE_KEYPAD */
}

/*
 *  clear the screen
 */

void
ClearScreen (void)
{
	tputs (_clearscreen, 1, outchar);
	my_flush ();		/* clear the output buffer */
	_line = 1;
}

/*
 *  move cursor to the specified row column on the screen.
 *  0,0 is the top left!
 */

#ifdef M_UNIX

void
MoveCursor (
	int row,
	int col)
{
	char *stuff;

	stuff = tgoto(_moveto, col, row);
	tputs (stuff, 1, outchar);
	my_flush ();
	_line = row + 1;
}

#else	/* !M_UNIX */

void
MoveCursor (
	int row,
	int col)
{
	char stuff[12];

	if (_moveto) {
		sprintf (stuff, _moveto, row+1, col+1);
		tputs (stuff, 1, outchar);
		my_flush ();
		_line = row + 1;
	}
}

#endif /* M_UNIX */

/*
 *  clear to end of line
 */
void
CleartoEOLN (void)
{
	tputs (_cleartoeoln, 1, outchar);
	my_flush ();	/* clear the output buffer */
}

/*
 *  clear to end of screen
 */
void
CleartoEOS (void)
{
	int i;

	if (_cleartoeos) {
		tputs (_cleartoeos, 1, outchar);
	} else {
		for (i=_line - 1 ; i < _lines ; i++) {
			MoveCursor (i, 0);
			CleartoEOLN ();
		}
	}
	my_flush ();	/* clear the output buffer */
}

static int _topscrregion, _bottomscrregion;

void
setscrreg (
	int topline,
	int bottomline)
{
	char *stuff;

	if (!have_linescroll)
		return;
	if (_scrollregion) {
		stuff = TFORMAT(_scrollregion, topline, bottomline);
		tputs (stuff, 1, outchar);
		_topscrregion = topline;
		_bottomscrregion = bottomline;
	}
	my_flush ();
}

void
scrl (
	int lines)
{
	int i;

	if (!have_linescroll || (lines == 0))
		return;
	if (lines < 0) {
		if (_scrollback) {
			i = lines;
			while (i++) {
				MoveCursor (_topscrregion, 0);
				tputs (_scrollback, 1, outchar);
			}
		}
	} else
		if (_scrollfwd) {
			i = lines;
			while (i--) {
				MoveCursor (_bottomscrregion, 0);
				tputs (_scrollfwd, 1, outchar);
			}
		}
	my_flush ();
}


/*
 *  set inverse video mode
 */
void
StartInverse (
	void)
{
	in_inverse = 1;
	if (_setinverse && tinrc.inverse_okay) {
#	ifdef HAVE_COLOR
		if (use_color) {
			bcol(tinrc.col_invers_bg);
			fcol(tinrc.col_invers_fg);
		} else {
			tputs (_setinverse, 1, outchar);
		}
#	else
		tputs (_setinverse, 1, outchar);
#	endif /* HAVE_COLOR */
	}
	my_flush ();
}


/*
 *  compliment of startinverse
 */
void
EndInverse (
	void)
{
	in_inverse = 0;
	if (_clearinverse && tinrc.inverse_okay) {
#	ifdef HAVE_COLOR
		if (use_color) {
			fcol(tinrc.col_normal);
			bcol(tinrc.col_back);
		} else {
			tputs (_clearinverse, 1, outchar);
		}
#	else
		tputs (_clearinverse, 1, outchar);
#	endif /* HAVE_COLOR */
	}
	my_flush ();
}


/*
 *  toggle inverse video mode
 */
void
ToggleInverse (
	void)
{
	if (!in_inverse)
		StartInverse();
	else
		EndInverse();
}


/*
 *  returns either 1 or 0, for ON or OFF
 */
int
RawState(
	void)
{
	return (_inraw);
}


/*
 *  state is either TRUE or FALSE, as indicated by call
 */
void
Raw (
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
#	ifndef M_OS2

#		if defined(M_AMIGA) && defined(__SASC)
	_inraw = state;
	rawcon (state);
#		else
	if (!state && _inraw) {
		SET_TTY (&_original_tty);
		_inraw = 0;
	} else if (state && !_inraw) {
		GET_TTY (&_original_tty);
		GET_TTY (&_raw_tty);
#			if USE_SGTTY
		_raw_tty.sg_flags &= ~(ECHO | CRMOD);	/* echo off */
		_raw_tty.sg_flags |= CBREAK;		/* raw on */
#				ifdef M_AMIGA
		_raw_tty.sg_flags |= RAW; /* Manx-C 5.2 does not support CBREAK */
#				endif /* M_AMIGA */
#			else
#				ifdef __FreeBSD__
		cfmakeraw(&_raw_tty);
		_raw_tty.c_lflag |= ISIG;		/* for ^Z */
#				else
		_raw_tty.c_lflag &= ~(ICANON | ECHO);	/* noecho raw mode */
		_raw_tty.c_cc[VMIN] = '\01';	/* minimum # of chars to queue */
		_raw_tty.c_cc[VTIME] = '\0';	/* minimum time to wait for input */
#				endif /* __FreeBSD__ */
#			endif /* USE_SGTTY */
		SET_TTY (&_raw_tty);
		_inraw = 1;
	}
#		endif /* M_AMIGA && __SASC */

#	endif /* !M_OS2 */
#endif /* !VMS */
}

/*
 *  read a character with Raw mode set!
 */

#ifndef VMS
#	ifdef M_OS2

int
ReadCh (void)
{
	char ch;
	KBDKEYINFO os2key;
	int rc;
	register int result = 0;
	static secondkey = 0;

	if (secondkey) {
		result = secondkey;
		secondkey = 0;
	} else {
		rc = KbdCharIn((PKBDKEYINFO)&os2key, IO_WAIT, 0);
		result = os2key.chChar;
		if (result == 0xe0) {
			result = 0x1b;
			switch (os2key.chScan) {
				case 'H':
					secondkey = 'A';
					break;
				case 'P':
					secondkey = 'B';
					break;
				case 'K':
					secondkey = 'D';
					break;
				case 'M':
					secondkey = 'C';
					break;
				case 'I':
					secondkey = 'I';
					break;
				case 'Q':
					secondkey = 'G';
					break;
				case 'G':
					secondkey = 'H';
					break;
				case 'O':
					secondkey = 'F';
					break;
				default:
					secondkey = '?';
					break;
			}
		} else if (result == 0x0d) {
			result = 0x0a;
		}
	}
	return ((result == EOF) ? EOF : result & 0xFF);
}

#	endif /* M_OS2 */

#	ifdef M_AMIGA
#		include <sprof.h>

static int new_lines, new_columns;

static int
AmiReadCh (
	int getscrsize)
{
	int result;
	static int buflen = 0, bufp = 0;
	static unsigned char buf[128];
	unsigned char ch;

	while (getscrsize || !buflen) {
PROFILE_OFF();
		result = read (0, (char *)&buf[buflen], 1);
PROFILE_ON();
		if (result <= 0) return EOF;
		buflen++;
		if (buf[bufp] == KEY_PREFIX) {
			do {
				result = read (0, (char *)&buf[buflen], 1);
				if (result <= 0) return EOF;
			} while (buf[buflen++] < 0x40);

			switch (buf[buflen-1])
			{	char *ptr;
				long class;

				case 'r':		/* Window bounds report */
					ptr = (char *)&buf[bufp+1];
					strtol(ptr, &ptr, 10);
					ptr++;
					strtol(ptr, &ptr, 10);
					ptr++;
					new_lines = strtol(ptr, &ptr, 10);
					ptr++;
					new_columns = strtol(ptr, &ptr, 10);
					buflen = bufp;
					if (getscrsize)
						return 0;
					break;

				case '|':		/* Raw Input Events */
					ptr = (char *)&buf[bufp+1];
					class = strtol(ptr, &ptr, 10);
					ptr++;
					switch (class) {
						int x, y;

						case 12:	/* Window resized */
							buflen = bufp; /* Must do this before raise() */
							raise(SIGWINCH);
							break;
#			ifdef notdef
						case 2:	/* Mouse event */
					/*
					 * At this point we know what button was pressed
					 * but we don't really know where the mouse is.
					 * The <x> and <y> parameters don't help.
					 * Perhaps looking directly in the window's structure
					 * is the easiest thing to do (after finding out where
					 * the window's structure is! Sending an ACTION_INFO
					 * packet to the handler gives us this. I don't know
					 * if there is an easier way.
					 */
							buflen = bufp;
							break;
#			endif /* notdef */
						default:
							buflen = bufp;
							break;
					}
					break;

				default:
					break;
			}
		}
	}

	ch = buf[bufp++];
	if (bufp >= buflen) buflen = bufp = 0;
	return ch;
}


int
ReadCh (
	void)
{
	return AmiReadCh(0);
}


void
AmiGetWinSize (
	int *lines,
	int *columns)
{
	if (_getwinsize) {
		tputs (_getwinsize, 1, outchar);	/* identify yourself */
		my_flush ();
		AmiReadCh(1);		/* Look for the identification */
		*lines = new_lines;
		*columns = new_columns;
	}
}

#	endif /* M_AMIGA */
#endif /* !VMS */

/*
 *  output a character. From tputs... (Note: this CANNOT be a macro!)
 */
OUTC_FUNCTION (
	outchar)
{
#ifdef OUTC_RETURN
	return fputc (c, stdout);
#else
	(void) fputc (c, stdout);
#endif /* OUTC_RETURN */
}


/*
 *  setup to monitor mouse buttons if running in a xterm
 */
static void
xclick (
	int state)
{
	static int prev_state = 999;

	if (xclicks && prev_state != state) {
		if (state) {
			tputs (_xclickinit, 1, outchar);
		} else {
			tputs (_xclickend, 1, outchar);
		}
		my_flush ();
		prev_state = state;
	}
}


/*
 *  switch on monitoring of mouse buttons
 */
void
set_xclick_on (void)
{
	if (tinrc.use_mouse) xclick (TRUE);
}


/*
 *  switch off monitoring of mouse buttons
 */
void
set_xclick_off (void)
{
	if (tinrc.use_mouse) xclick (FALSE);
}


void
cursoron (void)
{
	if (_cursoron)
		tputs (_cursoron, 1, outchar);
}


void
cursoroff (void)
{
	if (_cursoroff)
		tputs (_cursoroff, 1, outchar);
}


/*
 * Inverse 'size' chars at (row,col)
 */
void
highlight_string (
	int row,
	int col,
	int size)
{
	char tmp[LEN];

	MoveCursor (row, col);
	strncpy (tmp, &(screen[row].col[col]), size);	/* TODO: broken in pager - it doesn't use screen[] */
	StartInverse ();
#if 0
	my_fputs (tmp, stdout);
	my_flush ();
#endif /* 0 */
	EndInverse ();
	stow_cursor();
}


/*
 * input_pending() waits for input during time given
 * by delay in msec. The original behaviour of input_pending()
 * (in art.c's threading code) is delay=0
 */
static int
input_pending (
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

#	ifdef WIN32
	return kbhit() ? TRUE : FALSE;
#	endif /* WIN32 */
#	ifdef M_AMIGA
	return (WaitForChar(Input(), 1000 * delay) == DOSTRUE) ? TRUE : FALSE;
#	endif /* M_AMIGA */

#	ifdef HAVE_SELECT
	int fd = STDIN_FILENO;
	fd_set fdread;
	struct timeval tvptr;

	FD_ZERO(&fdread);

	tvptr.tv_sec = 0;
	tvptr.tv_usec = delay * 100;

	FD_SET(fd, &fdread);

#		ifdef HAVE_SELECT_INTP
	if (select (1, (int *)&fdread, NULL, NULL, &tvptr))
#		else
	if (select (1, &fdread, NULL, NULL, &tvptr))
#		endif /* HAVE_SELECT_INTP */
	{
		if (FD_ISSET(fd, &fdread))
			return TRUE;
	}
#	endif /* HAVE_SELECT */

#	if defined(HAVE_POLL) && !defined(HAVE_SELECT)
	static int Timeout;
	static long nfds = 1;
	static struct pollfd fds[]= {{ STDIN_FILENO, POLLIN, 0 }};

	Timeout = delay;
	if (poll (fds, nfds, Timeout) < 0) /* Error on poll */
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
get_arrow_key (
	int prech)
{
	int ch;
	int ch1;

#define wait_a_while(i) \
	while (!input_pending(0) \
		&& i < ((VT_ESCAPE_TIMEOUT * 1000) / SECOND_CHARACTER_DELAY))

#	ifndef VMS
#		ifdef M_AMIGA
	if (WaitForChar(Input(), 1000) == DOSTRUE)
		return prech;
#		else	/* !M_AMIGA */
	if (!input_pending(0)) {
#			ifdef HAVE_USLEEP
		int i=0;

		wait_a_while(i) {
			usleep((unsigned long) (SECOND_CHARACTER_DELAY * 1000));
			i++;
		}
#			else	/* !HAVE_USLEEP */
#				ifdef HAVE_SELECT
		struct timeval tvptr;
		int i=0;

		wait_a_while(i) {
			tvptr.tv_sec = 0;
			tvptr.tv_usec = SECOND_CHARACTER_DELAY * 1000;
			select (0, NULL, NULL, NULL, &tvptr);
			i++;
		}
#				else /* !HAVE_SELECT */
#					ifdef HAVE_POLL
		struct pollfd fds[1];
		int i=0;

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
#		endif /* M_AMIGA */
#	endif /* !VMS */
	ch = ReadCh ();
	if (ch == '[' || ch == 'O')
		ch = ReadCh ();

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
#	ifdef M_AMIGA
			return KEYMAP_PAGE_DOWN;
#	else
			return KEYMAP_PAGE_UP;
#	endif /* M_AMIGA */

		case 'G':		/* ansi  PgDn */
		case 'U':		/* at386 PgDn */
		case 'T':		/* 97801 PgDn */
#	ifdef QNX42
		case 0xAA:
#	endif /* QNX42 */
#	ifdef M_AMIGA
			return KEYMAP_PAGE_UP;
#	else
			return KEYMAP_PAGE_DOWN;
#	endif /* M_AMIGA */

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
			(void) ReadCh ();	/* eat the ~ */
			return KEYMAP_INS;

		case '3':		/* vt200 Del */
			(void) ReadCh ();	/* eat the ~ */
			return KEYMAP_DEL;

		case '5':		/* vt200 PgUp */
			(void) ReadCh ();	/* eat the ~ (interesting use of words :) */
			return KEYMAP_PAGE_UP;

		case '6':		/* vt200 PgUp */
			(void) ReadCh ();	/* eat the ~ */
			return KEYMAP_PAGE_DOWN;

		case '1':		/* vt200 PgUp */
			ch = ReadCh (); /* eat the ~ */
			if (ch == '5') {	/* RS/6000 PgUp is 150g, PgDn is 154g */
				ch1 = ReadCh ();
				(void) ReadCh ();
				if (ch1 == '0')
					return KEYMAP_PAGE_UP;
				if (ch1 == '4')
					return KEYMAP_PAGE_DOWN;
			}
			return KEYMAP_HOME;

		case '4':		/* vt200 PgUp */
			(void) ReadCh ();	/* eat the ~ */
			return KEYMAP_END;

		case 'M':		/* xterminal button press */
			xmouse = ReadCh () - ' ';	/* button */
			xcol = ReadCh () - '!';		/* column */
			xrow = ReadCh () - '!';		/* row */
			return KEYMAP_MOUSE;

		default:
			return KEYMAP_UNKNOWN;
	}
}
#endif /* !USE_CURSES */


/*
 * The UNIX version of ReadCh, termcap version
 */
#ifdef M_UNIX
int
ReadCh (void)
{
	register int result;
#		ifndef READ_CHAR_HACK
	char ch;
#		endif /* READ_CHAR_HACK */

	fflush(stdout);
#		ifdef READ_CHAR_HACK
#			undef getc
	while ((result = getc(stdin)) == EOF) {
		if (feof(stdin))
			break;

#			ifdef EINTR
		if (ferror(stdin) && errno != EINTR)
#			else
		if (ferror(stdin))
#			endif /* EINTR */
			break;

		clearerr(stdin);
	}

	return ((result == EOF) ? EOF : result & 0xFF);

#		else
#			ifdef EINTR

	allow_resize (TRUE);
	while ((result = read (0, &ch, 1)) < 0 && errno == EINTR) {		/* spin on signal interrupts */
		if (need_resize) {
			handle_resize ((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
		}
	}
	allow_resize (FALSE);
#			else
	result = read (0, &ch, 1);
#			endif /* EINTR */

	return ((result <= 0) ? EOF : ch & 0xFF);

#		endif /* READ_CHAR_HACK */
}
#endif /* M_UNIX */
