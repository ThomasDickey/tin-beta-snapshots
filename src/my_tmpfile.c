/*
 *  Project   : tin - a Usenet reader
 *  Module    : my_tmpfile.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 2001-03-11
 *  Updated   : 2004-06-30
 *  Notes     :
 *
 * Copyright (c) 2001-2006 Urs Janssen <urs@tin.org>
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
 * my_tmpfile(filename, name_size, need_name, base_dir)
 *
 * try to create a uniq tmp-file descriptor
 *
 * return codes:
 * >0 = file descriptor of tmpfile
 *      if need_name is set to true and/or we have to unlink the file
 *      ourself filename is set to the name of the tmp file located in
 *      base_dir
 * -1 = some error occured
 */
int
my_tmpfile(
	char *filename,
	size_t name_size,
	t_bool need_name,
	const char *base_dir)
{
	int fd = -1;
	char buf[PATH_LEN];

	errno = 0;

	if (filename != NULL && name_size > 0) {
		if (!need_name) {
			FILE *fp;

			if ((fp = tmpfile()) != NULL)
				fd = fileno(fp);
#ifdef DEBUG
			else
				wait_message(5, "HAVE_TMPFILE %s", strerror(errno));
#endif /* DEBUG */
			*filename = '\0';
			if (fd == -1)
				error_message(_(txt_cannot_create_uniq_name));
			return fd;
		}

		if (base_dir) {
			snprintf(buf, MIN(name_size, (sizeof(buf) - 1)), "tin-%s-%d-XXXXXX", get_host_name(), process_id);
			joinpath(filename, base_dir, buf);
		} else {
			snprintf(buf, MIN(name_size, (sizeof(buf) - 1)), "tin_XXXXXX");
			joinpath(filename, TMPDIR, buf);
		}
#ifdef HAVE_MKSTEMP
		fd = mkstemp(filename);
#	ifdef DEBUG
		if (errno)
			wait_message(5, "HAVE_MKSTEMP %s: %s", filename, strerror(errno));
#	endif /* DEBUG */
#else
#	ifdef HAVE_MKTEMP
		fd = open(mktemp(filename), (O_WRONLY|O_CREAT|O_EXCL), (mode_t) (S_IRUSR|S_IWUSR));
#		ifdef DEBUG
		if (errno)
			wait_message(5, "HAVE_MKTEMP %s: %s", filename, strerror(errno));
#		endif /* DEBUG */
#	endif /* HAVE_MKTEMP */
#endif /* HAVE_MKSTEMP */
		}
	if (fd == -1)
		error_message(_(txt_cannot_create_uniq_name));
	return fd;
}
