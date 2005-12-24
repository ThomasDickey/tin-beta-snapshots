/*
 *  Project   : tin - a Usenet reader
 *  Module    : strftime.c
 *  Author    : A. Robbins & I. Lea
 *  Created   : 1991-02-01
 *  Updated   : 1993-08-15
 *  Notes     : Relatively quick-and-dirty implemenation of ANSI library
 *              routine for System V Unix systems.
 *              If target system already has strftime() call the #define
 *              HAVE_STRFTIME can be set to use it.
 *  Example   : time(&secs);
 *              tm = localtime(&secs);
 *              num = strftime(buf, sizeof(buf), "%a %d-%m-%y %H:%M:%S", tm);
 *
 * Copyright (c) 1991-2006 Arnold Robbins <arnold@skeeve.com>
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

#ifdef SYSV
extern int daylight;
#endif /* SYSV */

#ifndef HAVE_STRFTIME
#	define SYSV_EXT	1	/* stuff in System V ascftime routine */
#endif /* !HAVE_STRFTIME */

/*
 * strftime --- produce formatted time
 */

size_t
my_strftime(
	char *s,
	size_t maxsize,
	const char *format,
	struct tm *timeptr)
{
#ifdef HAVE_STRFTIME
	return strftime(s, maxsize, format, timeptr);
#else
	char *endp = s + maxsize;
	char *start = s;
	char tbuf[100];
	int i;

	/*
	 * various tables, useful in North America
	 */
	static const char *days_a[] = {
		"Sun", "Mon", "Tue", "Wed",
		"Thu", "Fri", "Sat",
	};
	static const char *days_l[] = {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday",
	};
	static const char *months_a[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};
	static const char *months_l[] = {
		"January", "February", "March", "April",
		"May", "June", "July", "August", "September",
		"October", "November", "December",
	};
	static const char *ampm[] = { "AM", "PM", };

	if (s == NULL || format == NULL || timeptr == NULL || maxsize == 0)
		return 0;

	if (strchr(format, '%') == NULL && strlen(format) + 1 >= maxsize)
		return 0;

#ifdef HAVE_TZSET
	tzset();
#else
#	ifdef HAVE_SETTZ
	settz();
#	endif /* HAVE_SETTZ */
#endif /* HAVE_TZSET */

	for (; *format && s < endp - 1; format++) {
		tbuf[0] = '\0';
		if (*format != '%') {
			*s++ = *format;
			continue;
		}
		switch (*++format) {
		case '\0':
			*s++ = '%';
			goto out;

		case '%':
			*s++ = '%';
			continue;

		case 'a':	/* abbreviated weekday name */
			strcpy(tbuf, days_a[timeptr->tm_wday]);
			break;

		case 'A':	/* full weekday name */
			strcpy(tbuf, days_l[timeptr->tm_wday]);
			break;

#	ifdef SYSV_EXT
		case 'h':	/* abbreviated month name */
#	endif /* SYSV_EXT */
		case 'b':	/* abbreviated month name */
			strcpy(tbuf, months_a[timeptr->tm_mon]);
			break;

		case 'B':	/* full month name */
			strcpy(tbuf, months_l[timeptr->tm_mon]);
			break;

		case 'c':	/* appropriate date and time representation */
			snprintf(tbuf, sizeof(tbuf), "%s %s %2d %02d:%02d:%02d %d",
				days_a[timeptr->tm_wday],
				months_a[timeptr->tm_mon],
				timeptr->tm_mday,
				timeptr->tm_hour,
				timeptr->tm_min,
				timeptr->tm_sec,
				timeptr->tm_year + 1900);
			break;

		case 'd':	/* day of the month, 01 - 31 */
			snprintf(tbuf, sizeof(tbuf), "%02d", timeptr->tm_mday);
			break;

		case 'H':	/* hour, 24-hour clock, 00 - 23 */
			snprintf(tbuf, sizeof(tbuf), "%02d", timeptr->tm_hour);
			break;

		case 'I':	/* hour, 12-hour clock, 01 - 12 */
			i = timeptr->tm_hour;
			if (i == 0)
				i = 12;
			else if (i > 12)
				i -= 12;
			snprintf(tbuf, sizeof(tbuf), "%02d", i);
			break;

		case 'j':	/* day of the year, 001 - 366 */
			snprintf(tbuf, sizeof(tbuf), "%03d", timeptr->tm_yday + 1);
			break;

		case 'm':	/* month, 01 - 12 */
			snprintf(tbuf, sizeof(tbuf), "%02d", timeptr->tm_mon + 1);
			break;

		case 'M':	/* minute, 00 - 59 */
			snprintf(tbuf, sizeof(tbuf), "%02d", timeptr->tm_min);
			break;

		case 'p':	/* am or pm based on 12-hour clock */
			strcpy(tbuf, ampm[((timeptr->tm_hour < 12) ? 0 : 1)]);
			break;

		case 'S':	/* second, 00 - 61 */
			snprintf(tbuf, sizeof(tbuf), "%02d", timeptr->tm_sec);
			break;

		case 'w':	/* weekday, Sunday == 0, 0 - 6 */
			snprintf(tbuf, sizeof(tbuf), "%d", timeptr->tm_wday);
			break;

		case 'x':	/* appropriate date representation */
			snprintf(tbuf, sizeof(tbuf), "%s %s %2d %d",
				days_a[timeptr->tm_wday],
				months_a[timeptr->tm_mon],
				timeptr->tm_mday,
				timeptr->tm_year + 1900);
			break;

		case 'X':	/* appropriate time representation */
			snprintf(tbuf, sizeof(tbuf), "%02d:%02d:%02d",
				timeptr->tm_hour,
				timeptr->tm_min,
				timeptr->tm_sec);
			break;

		case 'y':	/* year without a century, 00 - 99 */
			i = timeptr->tm_year % 100;
			snprintf(tbuf, sizeof(tbuf), "%d", i);
			break;

		case 'Y':	/* year with century */
			snprintf(tbuf, sizeof(tbuf), "%d", timeptr->tm_year + 1900);
			break;

#	ifdef SYSV_EXT
		case 'n':	/* same as \n */
			tbuf[0] = '\n';
			tbuf[1] = '\0';
			break;

		case 't':	/* same as \t */
			tbuf[0] = '\t';
			tbuf[1] = '\0';
			break;

		case 'D':	/* date as %m/%d/%y */
			my_strftime(tbuf, sizeof(tbuf), "%m/%d/%y", timeptr);
			break;

		case 'e':	/* day of month, blank padded */
			snprintf(tbuf, sizeof(tbuf), "%2d", timeptr->tm_mday);
			break;

		case 'r':	/* time as %I:%M:%S %p */
			my_strftime(tbuf, sizeof(tbuf), "%I:%M:%S %p", timeptr);
			break;

		case 'R':	/* time as %H:%M */
			my_strftime(tbuf, sizeof(tbuf), "%H:%M", timeptr);
			break;

		case 'T':	/* time as %H:%M:%S */
			my_strftime(tbuf, sizeof(tbuf), "%H:%M:%S", timeptr);
			break;
#	endif /* SYSV_EXT */

		default:
			tbuf[0] = '%';
			tbuf[1] = *format;
			tbuf[2] = '\0';
			break;
		}
		if ((i = strlen(tbuf))) {
			if (s + i < endp - 1) {
				strcpy(s, tbuf);
				s += i;
			} else
				return 0;
		}
	}
out:
	if (s < endp && *format == '\0') {
		*s = '\0';
		return (size_t) (s - start);
	} else
		return 0;

#endif /* HAVE_STRFTIME */
}
