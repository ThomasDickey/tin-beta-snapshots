/*
 *  Project   : tin - a Usenet reader
 *  Module    : lock.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 1998-07-27
 *  Updated   : 2003-05-16
 *  Notes     :
 *
 * Copyright (c) 1998-2006 Urs Janssen <urs@tin.org>
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

#if !defined(USE_FLOCK) && !defined(USE_LOCKF) && !defined(USE_FCNTL)
#	ifdef HAVE_FCNTL
#		define USE_FCNTL 1
#	else
#		ifdef HAVE_LOCKF
#			define USE_LOCKF 1
#		else
#			ifdef HAVE_FLOCK
#				define USE_FLOCK 1
#			endif /* HAVE_FLOCK */
#		endif /* HAVE_LOCKF */
#	endif /* HAVE_FCNTL */
#endif /* !USE_FLOCK && !USE_LOCKF && !USE_FCNTL */

/*
 * TODO: add support for $LOCKEXT
 */
#define LOCK_SUFFIX ".lock"

/*
 * fd_lock(fd, block)
 *
 * try to lock a file descriptor with fcntl(), flock() or lockf()
 *
 * return codes:
 *  0 = file locked successfully
 * -1 = some error occured
 */
int
fd_lock(
	int fd,
	t_bool block)
{
	int rval = -1; /* assume an error */

#ifdef USE_FCNTL
	struct flock flk;

	flk.l_type = F_WRLCK;
	flk.l_whence = SEEK_SET;
	flk.l_start = 0;
	flk.l_len = 0;
	if ((rval = fcntl(fd, block ? F_SETLKW : F_SETLK, &flk)))
		return rval; /* fcntl locking failed */
#else
#	ifdef USE_LOCKF
	if ((rval = lockf(fd, block ? F_LOCK : F_TLOCK, 0L)))
		return rval; /* lockf locking failed */
#	else
#		ifdef USE_FLOCK
	if ((rval = flock(fd, block ? LOCK_EX : (LOCK_EX | LOCK_NB))))
		return rval; /* flock locking failed */
#		endif /* USE_FLOCK */
#	endif /* USE_LOCKF */
#endif /* USE_FCNTL */

	return rval;	/* available lock successfully applied or no locking available */
}


#if 0 /* unused */
/*
 * test_fd_lock(fd)
 *
 * check for an existing lock on file descriptor with fcntl(), lockf()
 * or flock()
 *
 * return codes:
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
#ifdef USE_FCNTL
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
#else
#	ifdef USE_LOCKF
	if (lockf(fd, F_TEST, 0L)) {
		if (errno == EACCES)
			return 1;	/* file is locked */
		else
			return -1;	/* some error occured */
	} else
		rval = 0;	/* file is not lockf locked */
#	else
#		ifdef USE_FLOCK
	if (flock(fd, (LOCK_EX|LOCK_NB))) {
		if (errno == EWOULDBLOCK)
			return 1;	/* file is locked */
		else
			return -1;	/* some error occured */
	} else
		rval = 0; /* file is not flock locked */

#		endif /* USE_FLOCK */
#	endif /* USE_LOCKF */
#endif /* USE_FCNTL */

	return rval;	/* file wasn't locked or no locking available */
}
#endif /* 0 */


/*
 * fd_unlock(fd)
 *
 * try to unlock a file descriptor with fcntl(), lockf() or flock()
 *
 * return codes:
 *  0 = file unlocked successfully
 * -1 = some error occured
 */
int
fd_unlock(
	int fd)
{
	int rval = -1; /* assume an error */

#ifdef USE_FCNTL
	{
		struct flock flk;

		flk.l_type = F_UNLCK;
		flk.l_whence = SEEK_SET;
		flk.l_start = 0;
		flk.l_len = 0;
		if ((rval = fcntl(fd, F_SETLK, &flk)))
			return rval; /* couldn't release fcntl lock */
	}
#else
#	ifdef USE_LOCKF
	if ((rval = lockf(fd, F_ULOCK, 0L)))
		return rval; /* couldn't release lockf lock */
#	else
#		ifdef USE_FLOCK
	if ((rval = flock(fd, LOCK_UN)))
		return rval; /* couldn't release flock lock */
#		endif /* USE_FLOCK */
#	endif /* USE_LOCKF */
#endif /* USE_FCNTL */

	return rval;	/* file successfully unlocked or no locking available */
}


/*
 * dot_lock(filename)
 *
 * try to lock filename via dotfile locking
 *
 * return codes:
 *  TRUE  = file locked successfully
 *  FALSE = some error occured
 */
t_bool dot_lock(
	const char *filename)
{
	char tempfile[PATH_LEN];
	char lockfile[PATH_LEN];
	char base_dir[PATH_LEN];
	int dot_fd;
	struct stat statbuf;
	t_bool rval = FALSE;

	dir_name(filename, base_dir);
	if (!strcmp(filename, base_dir)) /* no filename portion */
		return rval;
	if ((dot_fd = my_tmpfile(tempfile, sizeof(tempfile) - 1, TRUE, base_dir)) == -1)
		return rval;
	snprintf(lockfile, sizeof(lockfile), "%s%s", filename, LOCK_SUFFIX);

#ifdef HAVE_LINK
	if (stat(lockfile, &statbuf)) {				/* lockfile doesn't exist */
		if (!link(tempfile, lockfile)) {			/* link succsessfull */
			if (!stat(tempfile, &statbuf)) {	/* tempfile exist */
				if (statbuf.st_nlink == 2)			/* link count ok */
					rval = TRUE;
			}
		}
	}
#endif /* HAVE_LINK */

	close(dot_fd);
	(void) unlink(tempfile);

	if (!stat(lockfile, &statbuf)) {			/* lockfile still here */
		if (statbuf.st_nlink != 1)					/* link count wrong? */
			rval = FALSE;								/* shouldn't happen */
	}
	return rval;
}


/*
 * try to remove a dotlock for filename
 *
 * return codes:
 *  TRUE  = file unlocked successfully
 *  FALSE = some error occured
 */
t_bool dot_unlock(
	const char *filename)
{
	char *lockfile;
	t_bool rval = FALSE;

	lockfile = my_malloc(strlen(filename) + strlen(LOCK_SUFFIX) + 2);
	strcpy(lockfile, filename);
	strcat(lockfile, LOCK_SUFFIX);
	if (!unlink(lockfile))
		rval = TRUE;
	free(lockfile);
	return rval;
}
