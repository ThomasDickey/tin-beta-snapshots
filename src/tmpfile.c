/*
 *  Project   : tin - a Usenet reader
 *  Module    : tmpfile.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 2001-03-11
 *  Updated   : 2001-03-12
 *  Notes     :
 *
 * Copyright (c) 2001 Urs Janssen <urs@tin.org>
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
 * my_tmpfile(buffer, size)
 *
 * try to create a uniq tmp-file descriptor
 *
 * returncodes:
 * >0 = file descriptor of tmpfile
        if we have to unlink the file ourself buffer if set to the
        name of the file
 * -1 = some error occured
 */
int
my_tmpfile(
	char *buffer,
	size_t size)
{
	int fd = -1;

	errno = 0;

	if (buffer != (char *) 0 && size > 0)
#ifdef HAVE_TMPFILE
	{
		FILE *fp = (FILE *) 0;
		if ((fp = tmpfile()) != (FILE *) 0)
			fd = fileno(fp);
#	ifdef DEBUG
		else
			wait_message(5, "%s", strerror(errno));
#	endif /* DEBUG */
		buffer[0] = '\0';
	}
#else
	{
		snprintf (buffer, size, "%stin_XXXXXX", TMPDIR);
#	ifdef HAVE_MKSTEMP
		fd = mkstemp(buffer);
#		ifdef DEBUG
		if (errno)
			wait_message(5, "%s", strerror(errno));
#		endif /* DEBUG */
#	else
#		ifdef HAVE_MKTEMP
		fd = open(mktemp(buffer), (O_WRONLY|O_CREAT|O_EXCL), (mode_t)(S_IRUSR|S_IWUSR));
#			ifdef DEBUG
		if (errno)
			wait_message(5, "%s", strerror(errno));
#			endif /* DEBUG */
#		endif /* HAVE_MKTEMP */
#	endif /* HAVE_MKSTEMP */
	}
#endif /* HAVE_TMPFILE */
	if (fd == -1)
		error_message (_(txt_cannot_create_uniq_name));
	return fd;
}
