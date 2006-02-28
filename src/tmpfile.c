/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#	if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "$OpenBSD: tmpfile.c,v 1.6 1998/09/18 22:06:49 deraadt Exp $";
#	endif /* LIBC_SCCS and not lint */
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <unistd.h>
#	include <signal.h>
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	include <paths.h>
#else
#	include "tin.h"
#endif /* 0 */

#ifndef HAVE_TMPFILE
#	define TRAILER "tmp.XXXXXXXXXX"

FILE *
tmpfile(
	void)
{
	FILE *fp;
	char buf[sizeof(_PATH_TMP) + sizeof(TRAILER)];
	int sverrno, fd = -1;
	sigset_t set, oset;

	(void) memcpy(buf, _PATH_TMP, sizeof(_PATH_TMP) - 1);
	(void) memcpy(buf + sizeof(_PATH_TMP) - 1, TRAILER, sizeof(TRAILER));

	/* TODO: use portable signal blocking/unblocking */
	sigfillset(&set);
	(void) sigprocmask(SIG_BLOCK, &set, &oset);

#	ifdef HAVE_MKSTEMP
	fd = mkstemp(buf);
#	else
#		ifdef HAVE_MKTEMP
	fd = open(mktemp(buf), (O_WRONLY|O_CREAT|O_EXCL), (mode_t) (S_IRUSR|S_IWUSR));
#		endif /* HAVE_MKTEMP */
#	endif /* HAVE_MKSTEMP */

	if (fd != -1) {
		mode_t u;

		(void) unlink(buf);
		u = umask(0);
		(void) umask(u);
		(void) fchmod(fd, (S_IRUGO|S_IWUGO) & ~u);
	}

	(void) sigprocmask(SIG_SETMASK, &oset, NULL);

	if (fd == -1)
		return NULL;

	if ((fp = fdopen(fd, "w+")) == NULL) {
		sverrno = errno;
		(void) close(fd);
		errno = sverrno;
		return NULL;
	}
	return fp;
}
#endif /* !HAVE_TMPFILE */
