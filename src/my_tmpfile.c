/*
 *  Project   : tin - a Usenet reader
 *  Module    : my_tmpfile.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 2001-03-11
 *  Updated   : 2024-12-03
 *  Notes     :
 *
 * Copyright (c) 2001-2025 Urs Janssen <urs@tin.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
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

#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */


/*
 * my_mktmp(filename, name_size, base_dir)
 *
 * try to create a unique tmp-file descriptor (in base_dir if set)
 *
 * return codes:
 * >0 = file descriptor of tmpfile
 *      filename is set to the name of the tmp file
 * -1 = some error occurred, errno should hold the details
 *
 * TODO: add/verify code for
 *       HAVE_BSD_SIGNALS
 *       HAVE_SYSV_SIGNALS
 */
int
my_mktmp(
	char *filename,
	size_t name_size,
	const char *base_dir)
{
	char *buf;
	int fd = -1;
	mode_t mask;
	int sverrno = errno;

	if (filename != NULL && name_size > 0) {
		int n;
		size_t len;
#ifdef HAVE_POSIX_SIGNALS
		sigset_t set, oset;
#else
#	ifdef HAVE_BSD_SIGNALS
		int smask;
#	endif /* HAVE_BSD_SIGNALS */
#endif /* HAVE_POSIX_SIGNALS */

		if (base_dir) {
#ifdef HAVE_LONG_FILE_NAMES
			const char *nodenamebuf = BlankIfNull(get_host_name());

			/*
			 * base_dir may reside on a shared network device
			 * so add hostname/pid if allowed.
			 */
			if ((n = snprintf(NULL, 0, "tin-%s-%ld-XXXXXX", nodenamebuf, (long) process_id)) < 0)
				goto error_out;

			len = (size_t) n + 1;
			buf = my_malloc(len);
			if (snprintf(buf, len, "tin-%s-%ld-XXXXXX", nodenamebuf, (long) process_id) != n) {
				free(buf);
				goto error_out;
			}
#else
			if ((n = snprintf(NULL, 0, "tin-XXXXXX")) < 0)
				goto error_out;

			len = (size_t) n + 1;
			buf = my_malloc(len);
			if (snprintf(buf, len, "tin-XXXXXX") != n) {
				free(buf);
				goto error_out;
			}
#endif /* HAVE_LONG_FILE_NAMES */
		} else {
			/*
			 * tmpdir is unlikely to reside on a shared device
			 * so don't bother to distinguish between HAVE_LONG_FILE_NAMES
			 * or not.
			 */
			if ((n = snprintf(NULL, 0, "tin_XXXXXX")) < 0) {
#ifdef DEBUG
wait_message(5, "snprintf(NULL, 0, \"tin_XXXXXX\")=%d", n);
#endif /* DEBUG */
				goto error_out;
			}

			len = (size_t) n + 1;
			buf = my_malloc(len);
			if (snprintf(buf, len, "tin_XXXXXX") != n) {
#ifdef DEBUG
wait_message(5, "snprintf(%s, %d, \"tin_XXXXXX\")=%d", BlankIfNull(buf), len, n);
#endif /* DEBUG */
				free(buf);
				goto error_out;
			}
		}
		joinpath(filename, name_size, base_dir ? base_dir: tmpdir, buf);
		free(buf);

		mask = umask((mode_t) (S_IRWXO|S_IRWXG));

#ifdef HAVE_POSIX_SIGNALS
		if (!sigfillset(&set))
			(void) sigprocmask(SIG_BLOCK, &set, &oset);
		else
			(void) sigprocmask(SIG_BLOCK, NULL, &oset);
#else
#	ifdef HAVE_BSD_SIGNALS
		smask = sigblock(0); /* siggetmask(); */
		(void) sigblock(~0);
#	else
#		ifdef HAVE_SYSV_SIGNALS
		sighold(SIGINT);
#		endif /* HAVE_SYSV_SIGNALS */
#	endif /* HAVE_BSD_SIGNALS */
#endif /* HAVE_POSIX_SIGNALS */

#ifdef DEBUG
		errno = 0;
#endif /* DEBUG */

#ifdef HAVE_MKSTEMP
		fd = mkstemp(filename);
		sverrno = errno;
#	ifdef DEBUG
		if (fd == -1)
			perror_message("HAVE_MKSTEMP mkstemp(%s)==%d", filename, fd);
#	endif /* DEBUG */
#else
#	ifdef HAVE_MKTEMP
		buf = mktemp(filename);
		sverrno = errno;
		if (!sverrno) {
			if ((fd = open(buf, (O_RDWR|O_CREAT|O_EXCL), (mode_t) (S_IRUSR|S_IWUSR))) == -1) {
				sverrno = errno;
#		ifdef DEBUG
				if (fd == -1)
					perror_message("HAVE_MKTEMP open(%s)==%d", filename, fd);
#		endif /* DEBUG */
			}
		}
#		ifdef DEBUG
		else
			perror_message("HAVE_MKTEMP mktemp(%s)", filename);
#		endif /* DEBUG */
#	endif /* HAVE_MKTEMP */
#endif /* HAVE_MKSTEMP */

		(void) umask(mask);

#ifdef HAVE_POSIX_SIGNALS
		(void) sigprocmask(SIG_SETMASK, &oset, NULL);
#else
#	ifdef HAVE_BSD_SIGNALS
		(void) sigsetmask(smask);
#	else
#		ifdef HAVE_SYSV_SIGNALS
		(void) sigrelse(SIGINT);
#		endif /* HAVE_SYSV_SIGNALS */
#	endif /* HAVE_BSD_SIGNALS */
#endif /* HAVE_POSIX_SIGNALS */
	}
#ifdef DEBUG
	else
		wait_message(5, "my_mktmp(NULL, %lu, ...)", name_size);
#endif /* DEBUG */

error_out:
	if (fd == -1)
		error_message(2, _(txt_cannot_create_uniq_name));

	errno = sverrno;
	return fd;
}


/* tmpfile() replacement */
FILE *
my_tmpfile(
	void)
{
	char f[PATH_LEN];
	int fd, sverrno;
	size_t s = sizeof(f);
	FILE *fp;

	if ((fd = my_mktmp(f, s, NULL)) != -1) {
		if ((fp = fdopen(fd, "w+")) != NULL) {
			unlink(f);
			return(fp);
		}
		sverrno = errno;
#ifdef DEBUG
		perror_message("fdopen(%d, \"w+\")", fd);
#endif /* DEBUG */
		unlink(f);
		close(fd);
		errno = sverrno;
	}
#ifdef DEBUG
	perror_message("my_mktmp(f, %lu, NULL)==%d", s, fd);
#endif /* DEBUG */
	return NULL;
}
