/*
 *  Project   : tin - a Usenet reader
 *  Module    : lock.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 1998-07-27
 *  Updated   : 2001-03-11
 *  Notes     :
 *
 * Copyright (c) 1997-2001 Urs Janssen <urs@tin.org>
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
 * fd_lock(fd, block)
 *
 * try to lock a file descriptor with fcntl(), flock() and lockf()
 * or a subset if OS doesn't support all of them.
 *
 * returncodes:
 *  0 = file locked successfully
 * -1 = some error occured
 */
int
fd_lock(
	int fd,
	t_bool block)
{
	int rval = -1; /* assume an error */

#ifdef HAVE_FCNTL
	struct flock flk;

	flk.l_type = F_WRLCK;
	flk.l_whence = SEEK_SET;
	flk.l_start = 0;
	flk.l_len = 0;
	if ((rval = fcntl(fd, block ? F_SETLKW : F_SETLK, &flk)))
		return rval; /* fcntl locking failed */
#endif /* HAVE_FCNTL */

#ifdef HAVE_FLOCK
	if ((rval = flock(fd, block ? LOCK_EX : (LOCK_EX | LOCK_NB))))
		return rval; /* flock locking failed */
#endif /* HAVE_FLOCK */

#ifdef HAVE_LOCKF
	if ((rval = lockf(fd, block ? F_LOCK : F_TLOCK, 0L)))
		return rval; /* lockf locking failed */
#endif /* HAVE_LOCKF */

	return rval;	/* all available locks successfully applied or no locking available */
}


/*
 * test_fd_lock(fd)
 *
 * check for a existing lock on file descriptor with fcntl(), flock()
 * and lockf() or a subset if OS doesn't support all of them.
 *
 * returncodes:
 *  0 = file is not locked
 *  1 = file is locked
 * -1 = some error occured
 */
int
test_fd_lock(
	int fd)
{
	int rval = -1; /* assume an error */

	errno = 0;
#ifdef HAVE_FCNTL
	{
		struct flock flk;

		flk.l_type = F_WRLCK;
		flk.l_whence = SEEK_SET;
		flk.l_start = 0;
		flk.l_len = 0;
		if (fcntl(fd, F_GETLK, &flk) < 0)
				return -1; /* some error occured */
		else {
			if (flk.l_type != F_UNLCK)
				return 1;	/* file is locked */
			else
				rval = 0; /* file is not fcntl locked */
		}
	}
#endif /* HAVE_FCNTL */

#ifdef HAVE_FLOCK
	if (flock(fd, (LOCK_EX|LOCK_NB))) {
		if (errno == EWOULDBLOCK)
			return 1;	/* file is locked */
		else
			return -1;	/* some error occured */
	} else
		rval = 0; /* file is not flock locked */
#endif /* HAVE_FLOCK */

#ifdef HAVE_LOCKF
	if (lockf(fd, F_TEST, 0L)) {
		if (errno == EACCES)
			return 1;	/* file is locked */
		else
			return -1;	/* some error occured */
	} else
		rval = 0;	/* file is not lockf locked */
#endif /* HAVE_LOCKF */

	return rval;	/* file wasn't locked or no locking available */
}


/*
 * fd_unlock(fd)
 *
 * try to unlock a file descriptor with lockf(), flock() and fcntl()
 * or a subset (reverse order of fd_lock()) if OS doesn't support
 * all of them.
 *
 * returncodes:
 *  0 = file unlocked successfully
 * -1 = some error occured
 */
int
fd_unlock(
	int fd)
{
	int rval = -1; /* assume an error */

#ifdef HAVE_LOCKF
	if ((rval = lockf(fd, F_ULOCK, 0L)))
		return rval; /* couldn't release lockf lock */
#endif /* HAVE_LOCKF */

#ifdef HAVE_FLOCK
	if ((rval = flock(fd, LOCK_UN)))
		return rval; /* couldn't release flock lock */
#endif /* HAVE_FLOCK */

#ifdef HAVE_FCNTL
	{
		struct flock flk;

		flk.l_type = F_UNLCK;
		flk.l_whence = SEEK_SET;
		flk.l_start = 0;
		flk.l_len = 0;
		if ((rval = fcntl(fd, F_SETLK, &flk)))
			return rval; /* couldn't release fcntl lock */
	}
#endif /* HAVE_FCNTL */

	return rval;	/* file successfully unlocked or no locking available */
}
