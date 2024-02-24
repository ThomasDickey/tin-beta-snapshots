/*
 *  Project   : tin - a Usenet reader
 *  Module    : my_tmpfile.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 2001-03-11
 *  Updated   : 2024-01-18
 *  Notes     :
 *
 * Copyright (c) 2001-2024 Urs Janssen <urs@tin.org>
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
 * try to create a unique tmp-file descriptor
 *
 * return codes:
 * >0 = file descriptor of tmpfile
 *      if need_name is set to true and/or we have to unlink the file
 *      ourself filename is set to the name of the tmp file located in
 *      base_dir
 * -1 = some error occurred
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
#ifdef DEBUG
	int sverrno;
#endif /* DEBUG */

	if (filename != NULL && name_size > 0) {
		int len;
#ifdef HAVE_POSIX_SIGNALS
		sigset_t set, oset;
#else
#	ifdef HAVE_BSD_SIGNALS
		int smask;
#	endif	/* HAVE_BSD_SIGNALS */
#endif /* HAVE_POSIX_SIGNALS */

		if (base_dir) {
#ifdef HAVE_LONG_FILE_NAMES
			/*
			 * base_dir may reside on a shared network device
			 * so add hostname/pid if allowed.
			 */
			if ((len = snprintf(NULL, 0, "tin-%s-%ld-XXXXXX", get_host_name(), (long) process_id)) < 0)
				goto error_out;

			buf = my_malloc(++len);
			if (snprintf(buf, len, "tin-%s-%ld-XXXXXX", get_host_name(), (long) process_id) != len - 1) {
				free(buf);
				goto error_out;
			}
#else
			if ((len = snprintf(NULL, 0, "tin-XXXXXX")) < 0)
				goto error_out;

			buf = my_malloc(++len);
			if (snprintf(buf, len, "tin-XXXXXX") != len - 1) {
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
			if ((len = snprintf(NULL, 0, "tin_XXXXXX")) < 0)
				goto error_out;

			buf = my_malloc(++len);
			if (snprintf(buf, len, "tin_XXXXXX") != len - 1) {
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
#	ifdef DEBUG
		sverrno = errno;
		if (fd == -1 && sverrno)
			wait_message(5, "HAVE_MKSTEMP %s: %s", filename, strerror(sverrno));
#	endif /* DEBUG */
#else
#	ifdef HAVE_MKTEMP
		if ((buf = mktemp(filename)) != NULL)
			fd = open(buf, (O_WRONLY|O_CREAT|O_EXCL), (mode_t) (S_IRUSR|S_IWUSR));
#		ifdef DEBUG
		sverrno = errno;
		if (sverrno)
			wait_message(5, "HAVE_MKTEMP %s: %s", filename, strerror(sverrno));
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

error_out:
	if (fd == -1)
		error_message(2, _(txt_cannot_create_uniq_name));

	return fd;
}


/* tmpfile() replacement */
FILE *
my_tmpfile(
	void)
{
	char f[PATH_LEN];
	int fd;
	size_t s= sizeof(f);
	FILE *fp;

	if ((fd = my_mktmp(f, s, NULL)) != -1) {
		if ((fp = fdopen(fd, "w+")) != NULL) {
			unlink(f);
			return(fp);
		}
		unlink(f);
	}

	return NULL;
}
