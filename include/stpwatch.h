/*
 *  Project   : tin - a Usenet reader
 *  Module    : stpwatch.h
 *  Author    : I. Lea
 *  Created   : 1993-08-03
 *  Updated   : 2023-08-02
 *  Notes     : Simple stopwatch routines for timing code; avoid nesting!
 *
 * Copyright (c) 1993-2025 Iain Lea <iain@bricbrac.de>
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


#ifndef STPWATCH_H
#	define STPWATCH_H 1
#	if defined(PROFILE) /* TODO: write to some log? */
#		if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
			static struct t_tintime start_time;
			static struct t_tintime stop_time;
#			define BegStopWatch()	tin_gettime(&start_time)
#			define EndStopWatch(msg)	{ \
				if (tin_gettime(&stop_time) == 0) \
					error_message(2, "%s:%d:%s: %.6f sec", __FILE__, __LINE__, msg, ((stop_time.tv_sec - start_time.tv_sec) * 1000000000.0 + stop_time.tv_nsec - start_time.tv_nsec) / 1000000000.0); \
			}
#		else
			/* no ftime() fallback (we've also dropped the configure check for ftime) */
#			define BegStopWatch()
#			define EndStopWatch(msg)
#		endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */
#	else
#		define BegStopWatch()
#		define EndStopWatch(msg)
#	endif /* PROFILE */
#endif /* !STPWATCH_H */
