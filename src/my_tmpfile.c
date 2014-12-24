/*
 *  Project   : tin - a Usenet reader
 *  Module    : my_tmpfile.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 2001-03-11
 *  Updated   : 2014-05-13
 *  Notes     :
 *
 * Copyright (c) 2001-2015 Urs Janssen <urs@tin.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
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


/*
 * my_tmpfile(filename, name_size, base_dir)
 *
 * try to create a uniq tmp-file descriptor
 *
 * return codes:
 * >0 = file descriptor of tmpfile
 *      if need_name is set to true and/or we have to unlink the file
 *      ourself filename is set to the name of the tmp file located in
 *      base_dir
 * -1 = some error occurred
 */
int
my_tmpfile(
	char *filename,
	size_t name_size,
	const char *base_dir)
{
	int fd = -1;
	char buf[PATH_LEN];
	mode_t mask;
#if defined(HAVE_MKTEMP) && !defined(HAVE_MKSTEMP)
	char *t;
#endif /* HAVE_MKTEMP && !HAVE_MKSTEMP */
#ifdef DEBUG
	int sverrno;
#endif /* DEBUG */

	errno = 0;

	if (filename != NULL && name_size > 0) {
		if (base_dir) {
#ifdef HAVE_LONG_FILE_NAMES
			snprintf(buf, MIN(name_size, (sizeof(buf) - 1)), "tin-%s-%ld-XXXXXX", get_host_name(), (long) process_id);
#else
			snprintf(buf, MIN(name_size, (sizeof(buf) - 1)), "tin-XXXXXX");
#endif /* HAVE_LONG_FILE_NAMES */
			joinpath(filename, name_size, base_dir, buf);
		} else {
			snprintf(buf, MIN(name_size, (sizeof(buf) - 1)), "tin_XXXXXX");
			joinpath(filename, name_size, TMPDIR, buf);
		}
		mask = umask((mode_t) (S_IRWXO|S_IRWXG));
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
		if ((t = mktemp(filename)) != NULL)
			fd = open(t, (O_WRONLY|O_CREAT|O_EXCL), (mode_t) (S_IRUSR|S_IWUSR));
#		ifdef DEBUG
		sverrno = errno;
		if (sverrno)
			wait_message(5, "HAVE_MKTEMP %s: %s", filename, strerror(sverrno));
#		endif /* DEBUG */
#	endif /* HAVE_MKTEMP */
#endif /* HAVE_MKSTEMP */
		umask(mask);
	}
	if (fd == -1)
		error_message(2, _(txt_cannot_create_uniq_name));
	return fd;
}
