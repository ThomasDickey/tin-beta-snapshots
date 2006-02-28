/*
 *  Project   : tin - a Usenet reader
 *  Module    : stpwatch.h
 *  Author    : I. Lea
 *  Created   : 1993-08-03
 *  Updated   : 1997-04-05
 *  Notes     : Simple stopwatch routines for timing code using timeb
 *	             or gettimeofday structs
 *
 * Copyright (c) 1993-2006 Iain Lea <iain@bricbrac.de>
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


#ifndef STPWATCH_H
#	define STPWATCH_H 1

#	ifdef PROFILE

#		if defined(HAVE_SYS_TIMEB_H) && defined(HAVE_FTIME)
#			include <sys/timeb.h>

char msg_tb[LEN];
char tmp_tb[LEN];
struct timeb beg_tb;
struct timeb end_tb;

#			define LSECS 700000000

#			define BegStopWatch(msg)	{strcpy (msg_tb, msg); ftime (&beg_tb);}

#			define EndStopWatch()		{ftime (&end_tb);}

#			define PrintStopWatch()	{sprintf (tmp_tb, "%s: Beg=[%ld.%d] End=[%ld.%d] Elapsed=[%ld]", \
				 msg_tb, beg_tb.time, beg_tb.millitm, \
				 end_tb.time, end_tb.millitm, \
				 (((end_tb.time - LSECS) * 1000) + end_tb.millitm) - \
				 (((beg_tb.time - LSECS) * 1000) + beg_tb.millitm)); \
				 error_message (tmp_tb, "");}

#		else	/* HAVE_SYS_TIMEB_H && HAVE_FTIME */

#		ifdef	HAVE_SYS_TIME_H
#			include <sys/time.h>

char msg_tb[LEN], tmp_tb[LEN];
struct timeval beg_tb, end_tb;
float d_time;

#			define BegStopWatch(msg)	{strcpy (msg_tb, msg); \
				 (void) gettimeofday (&beg_tb, NULL);}

#			define EndStopWatch()		{(void) gettimeofday (&end_tb, NULL); \
				if ((end_tb.tv_usec -= beg_tb.tv_usec) < 0) { \
					end_tb.tv_sec--; \
					end_tb.tv_usec += 1000000; \
				 } \
				 end_tb.tv_sec -= beg_tb.tv_sec; \
				 d_time = (end_tb.tv_sec*1000.0 + ((float)end_tb.tv_usec)/1000.0);}

#			define PrintStopWatch()	{sprintf (tmp_tb, "StopWatch(%s): %6.3f ms", msg_tb, d_time); \
				 error_message (tmp_tb, "");}

#		endif /* HAVE_SYS_TIME_H */
#	endif /* HAVE_SYS_TIMEB_H && HAVE_FTIME */

#	else	/* PROFILE */

#		define BegStopWatch(msg)
#		define EndStopWatch()
#		define PrintStopWatch()

#	endif /* PROFILE */
#endif /* !STPWATCH_H */
