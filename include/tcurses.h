/*
 *  Project   : tin - a Usenet reader
 *  Module    : tcurses.h
 *  Author    : Thomas Dickey <dickey@herndon4.his.com>
 *  Created   : 1997-03-02
 *  Updated   : 2001-07-22
 *  Notes     : curses #include files, #defines & struct's
 *
 * Copyright (c) 1997-2001 Thomas Dickey <dickey@herndon4.his.com>
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
 *    This product includes software developed by Thomas Dickey.
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


#ifndef TCURSES_H
#define TCURSES_H 1


#if defined(USE_CURSES) || defined(NEED_CURSES_H)
#	ifdef HAVE_XCURSES
#		include <xcurses.h>
#		define getattrs(w) (w)->_attrs
#	elif defined(HAVE_NCURSES_H)
#		include <ncurses.h>
#	elif defined(HAVE_NCURSES_NCURSES_H)
#		include <ncurses/ncurses.h>
#	else
#		undef TRUE
#		undef FALSE
#		include <curses.h>
#		ifndef FALSE
#			define FALSE	0
#		endif /* !FALSE */
#		ifndef TRUE
#			define TRUE	(!FALSE)
#		endif /* !TRUE */
#	endif /* HAVE_NCURSES_H */
#endif /* USE_CURSES || NEED_CURSES_H */

#ifdef USE_CURSES

#ifdef USE_TRACE
#	ifdef HAVE_NOMACROS_H
#		include <nomacros.h>
#	endif /* HAVE_NOMACROS_H */
#endif /* USE_TRACE */

#define cCRLF				"\n"
#define my_flush()			my_fflush(stdout)
#define ClearScreen()			my_erase()
#define CleartoEOLN()			clrtoeol()
#define CleartoEOS()			clrtobot()
#define ScrollScreen(lines_to_scroll)	scrl(lines_to_scroll)
#define SetScrollRegion(top,bottom)	setscrreg(top, bottom)
#define WriteLine(row,buffer)		write_line(row,buffer)

#define HpGlitch(func)			/*nothing*/

extern int cmdReadCh (void);

extern char *screen_contents(int row, int col, char *buffer);
extern void MoveCursor(int row,int col);
extern void my_erase(void);
extern void my_fflush(FILE *stream);
extern void my_fputc(int ch, FILE *stream);
extern void my_fputs(const char *str, FILE *stream);
extern void my_fprintf(FILE *stream, const char *fmt, ...)
#ifdef __GNUC__
	__attribute__((format(printf,2,3)))
#endif /* __GNUC__ */
	;
extern void my_printf(const char *fmt, ...)
#ifdef __GNUC__
	__attribute__((format(printf,1,2)))
#endif /* __GNUC__ */
	;
extern void my_retouch(void);
extern void refresh_color(void);
extern void write_line(int row, char *buffer);

#else	/* !USE_CURSES */

#ifdef NEED_TERM_H
#	include <curses.h>
#	include <term.h>
#else
#	ifdef HAVE_TERMCAP_H
#	include <termcap.h>
#	endif /* HAVE_TERMCAP_H */
#endif /* NEED_TERM_H */

#define cCRLF				"\r\n"

#define my_fputc(ch, stream)		fputc (ch, stream)
#define my_fputs(str, stream)		fputs (str, stream)

#define my_printf			printf
#define my_fprintf			fprintf
#define my_flush()			fflush(stdout)
#define my_fflush(stream)		fflush(stream)
#define my_retouch()			ClearScreen()
#define WriteLine(row,buffer)		/*nothing*/

#define HpGlitch(func)			if (_hp_glitch) func

#endif /* USE_CURSES/!USE_CURSES */

extern void my_dummy(void);

#endif /* !TCURSES_H */
