/*
 *  Project   : tin - a Usenet reader
 *  Module    : trace.c
 *  Author    : Thomas Dickey <dickey@invisible-island.net>
 *  Created   : 1997-03-22
 *  Updated   : 2001-07-22
 *  Notes     : debugging support via TRACE macro.
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


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef included_trace_h
#	include "trace.h"
#endif /* !included_trace_h */

#ifndef HAVE__TRACEF
void
_tracef(
	const char *fmt,
	...)
{
	static FILE *fp;
	va_list ap;

	if (!fp)
		fp = fopen("trace.out", "w");
	if (!fp)
		abort();

	va_start(ap, fmt);
	if (fmt != 0) {
		vfprintf(fp, fmt, ap);
		fputc('\n', fp);
		(void)fflush(fp);
	} else {
		(void)fclose(fp);
		(void)fflush(stdout);
		(void)fflush(stderr);
	}
	va_end(ap);
}


char *
_nc_visbuf(const char *s)
{
	return (char *) s;
}
#endif /* !HAVE__TRACEF */

char *
tin_tracechar(
	int ch)
{
	static char result[2];
	result[0] = ch;
	return result;
}
