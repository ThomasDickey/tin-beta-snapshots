/*
 *  Project   : tin - a Usenet reader
 *  Module    : envarg.c
 *  Author    : B. Davidson
 *  Created   : 1991-10-13
 *  Updated   : 1993-03-10
 *  Notes     : Adds default options from environment to command line
 *
 * Copyright (c) 1991-2006 Bill Davidson
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

static int count_args(char *s);

static int
count_args(
	char *s)
{
	int ch, count = 0;

	do {
		/*
		 * count and skip args
		 */
		++count;
		while ((ch = *s) != '\0' && ch != ' ')
			++s;
		while ((ch = *s) != '\0' && ch == ' ')
			++s;
	} while (ch);

	return count;
}


void
envargs(
	int *Pargc,
	char ***Pargv,
	const char *envstr)
{
	char *envptr;			/* value returned by getenv */
	char *bufptr;			/* copy of env info */
	int argc;			/* internal arg count */
	int ch;				/* spare temp value */
	char **argv;			/* internal arg vector */
	char **argvect;			/* copy of vector address */

	/*
	 * see if anything in the environment
	 */
	envptr = getenv(envstr);
	if (envptr == NULL || *envptr == 0)
		return;

	/*
	 * count the args so we can allocate room for them
	 */
	argc = count_args(envptr);
	bufptr = my_strdup(envptr);

	/*
	 * allocate a vector large enough for all args
	 */
	argv = my_malloc((argc + *Pargc + 1) * sizeof(char *));
	argvect = argv;

	/*
	 * copy the program name first, that's always true
	 */
	*(argv++) = *((*Pargv)++);

	/*
	 * copy the environment args first, may be changed
	 */
	do {
		*(argv++) = bufptr;
		/*
		 * skip the arg and any trailing blanks
		 */
		while ((ch = *bufptr) != '\0' && ch != ' ')
			++bufptr;
		if (ch == ' ')
			*(bufptr++) = '\0';
		while ((ch = *bufptr) != '\0' && ch == ' ')
			++bufptr;
	} while (ch);

	/*
	 * now save old argc and copy in the old args
	 */
	argc += *Pargc;
	while (--(*Pargc))
		*(argv++) = *((*Pargv)++);

	/*
	 * finally, add a NULL after the last arg, like UNIX
	 */
	*argv = NULL;

	/*
	 * save the values and return
	 */
	*Pargv = argvect;
	*Pargc = argc;
}
