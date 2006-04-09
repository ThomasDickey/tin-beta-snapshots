/*
 *  Project   : tin - a Usenet reader
 *  Module    : trace.h
 *  Author    : Thomas Dickey <dickey@invisible-island.net>
 *  Created   : 1997-03-22
 *  Updated   : 2002-11-10
 *  Notes     : Interface of trace.c
 *
 * Copyright (c) 1997-2006 Thomas Dickey <dickey@invisible-island.net>
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


#ifndef included_trace_h
#	define included_trace_h 1

#	ifdef USE_TRACE
#		ifdef NCURSES_VERSION

extern char *_nc_visbuf(const char *s);

#		else

#			define _nc_visbuf(s) s

extern void	_tracef (const char *, ...)
#			if defined(__GNUC__) && !defined(printf)
	__attribute__ ((format(printf,1,2)))
#			endif /* __GNUC__ && !printf */
	;
#		endif /* NCURSES_VERSION */

#		define TRACE(p) _tracef p

extern char *tin_tracechar(int c);

#	else
#		define TRACE(p) /* nothing */

#	endif /* USE_TRACE */

#endif /* included_trace_h */
