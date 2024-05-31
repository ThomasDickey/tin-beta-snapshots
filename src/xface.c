/*
 *  Project   : tin - a Usenet reader
 *  Module    : xface.c
 *  Author    : Joshua Crawford & Drazen Kacar
 *  Created   : 2003-04-27
 *  Updated   : 2024-05-20
 *  Notes     :
 *
 * Copyright (c) 2003-2024 Joshua Crawford <mortarn@softhome.net> & Drazen Kacar <dave@willfork.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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


/*
 * TODO: - document the used files/dir in the manpage
 */

#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */

#ifdef XFACE_ABLE

static int slrnface_fd = -1;

#	define WRITE_FACE_FD(s)	if (write(slrnface_fd, s, strlen(s)) != (ssize_t) strlen(s)) {;}


void
slrnface_start(
	void)
{
	char *fifo;
	const char *ptr;
	int n;
	pid_t pid, pidst;
	size_t pathlen;
	struct utsname u;

	if (tinrc.use_slrnface == FALSE)
		return;

#	ifdef HAVE_IS_XTERM
	if (!is_xterm()) {
#		ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, "%s", _(txt_xface_error_no_xterm));
#		endif /* DEBUG */
		return;
	}
#	endif /* HAVE_IS_XTERM */

	/*
	 * $DISPLAY holds the (default) display name
	 */
	if (!getenv("DISPLAY")) {
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, _(txt_xface_error_missing_env_var), "DISPLAY");
#	endif /* DEBUG */
		return;
	}

	/*
	 * $WINDOWID holds the X window id number of the xterm window
	 */
	if (!getenv("WINDOWID")) {
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, _(txt_xface_error_missing_env_var), "WINDOWID");
#	endif /* DEBUG */
		return;
	}

	uname(&u);
	ptr = get_val("XDG_RUNTIME_DIR", get_val("HOME", ""));
	/*
	 * TODO:
	 * - check if $XDG_RUNTIME_DIR is on a local filesystem and has secure permissions
	 * <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
	 */
	if (!strlen(ptr)) { /* TODO: mention XDG_RUNTIME_DIR in error message? */
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, _(txt_xface_error_missing_env_var), "HOME");
#	endif /* DEBUG */
		return;
	}
	if ((n = snprintf(NULL, 0, "%s/.slrnfaces", ptr)) < 0)
		return;
	pathlen = (size_t) n + 1;
	fifo = my_malloc(pathlen);
	if (snprintf(fifo, pathlen, "%s/.slrnfaces", ptr) != n) {
		free(fifo);
		return;
	}
	if (my_mkdir(fifo, (mode_t) S_IRWXU)) {
		if (errno != EEXIST) {
			perror_message(_(txt_xface_error_create_failed), fifo);
			free(fifo);
			return;
		}
	} else {
		FILE *fp;

		free(fifo);
		if ((n = snprintf(NULL, 0, "%s/.slrnfaces/README", ptr)) < 0)
			return;
		pathlen = (size_t) n + 1;
		fifo = my_malloc(pathlen);
		if (snprintf(fifo, pathlen, "%s/.slrnfaces/README", ptr) != n) {
			free(fifo);
			return;
		}
		if ((fp = fopen(fifo, "w")) != NULL) {
			fputs(_(txt_xface_readme), fp);
			fclose(fp);
		}
	}
	free(fifo);
	pid = getpid();
	if ((n = snprintf(NULL, 0, "%s/.slrnfaces/%s.%ld", ptr, u.nodename, (long) pid)) < 0)
		return;
	pathlen = (size_t) n + 1;
	fifo = my_malloc(pathlen);
	if (snprintf(fifo, pathlen, "%s/.slrnfaces/%s.%ld", ptr, u.nodename, (long) pid) != n) {
		error_message(2, "%s", _(txt_xface_error_construct_fifo_name));
		unlink(fifo);
		free(fifo);
		return;
	}

	unlink(fifo);
	if (mkfifo(fifo, (S_IRUSR|S_IWUSR)) < 0) {
		perror_message(_(txt_xface_error_create_failed), fifo);
		unlink(fifo);
		free(fifo);
		return;
	}

	switch ((pid = fork())) {
		case -1:
			break;

		case 0:
			/*
			 * TODO: allow positioning, coloring, ...
			 *       execl(PATH_SLRNFACE, "slrnface",
			 *              "-xOffsetPix", tinrc.xfacex,
			 *              "-yOffsetPix", tinrc.xfacey,
			 *              "-ink", tinrc.xfacefg,
			 *              "-paper", tinrc.xfacebg,
			 *              fifo, NULL);
			 */
			execlp("slrnface", "slrnface", fifo, NULL);
			/* This is child, exit on error. */
			giveup();
			/* NOTREACHED */
			break;

		default:
			do {
				pidst = waitpid(pid, &n, 0);
			} while (pidst == -1 && errno == EINTR);
			if (!WIFEXITED(n))
				error_message(2, _(txt_xface_error_exited_abnormal), n);
			else {
				const char *message;

				switch (WEXITSTATUS(n)) {
					case 0:	/* All fine, open the pipe */
						if ((slrnface_fd = open(fifo, O_WRONLY, (S_IRUSR|S_IWUSR))) != -1) {
							WRITE_FACE_FD("start\n");
							message = NULL;
						} else
							message = _(txt_xface_msg_cannot_open_fifo);
						break;

					case 1:
						message = _(txt_xface_msg_cannot_connect_display);
						break;

					case 2:
						message = _(txt_xface_msg_windowid_not_found);
						break;

					case 3:
						message = _(txt_xface_msg_no_controlling_terminal);
						break;

					case 4:
						message = _(txt_xface_msg_no_width_and_height_avail);
						break;

					case 5:
						message = _(txt_xface_msg_cannot_open_fifo);
						break;

					case 6:
						message = _(txt_xface_msg_fork_failed);
						break;

					case 10:
						message = _(txt_xface_msg_executable_not_found);
						break;

					default:
						message = _(txt_unknown_error);
				}
				if (message)
					error_message(2, _(txt_xface_error_finally_failed), message);
			}
	}
	unlink(fifo);
	free(fifo);
}


void
slrnface_stop(
	void)
{
	if (slrnface_fd >= 0)
		close(slrnface_fd);

	slrnface_fd = -1;
	/* FIFO has been unlinked in the startup function. */
}


void
slrnface_display_xface(
	char *face)
{
	if (slrnface_fd < 0)
		return;

	if (!face || !*face)
		slrnface_clear_xface();
	else {
		char *buf = my_malloc(strlen(face) + 8);

		sprintf(buf, "xface %s\n", face);
		WRITE_FACE_FD(buf);
		free(buf);
	}
}


void
slrnface_clear_xface(
	void)
{
	if (slrnface_fd < 0)
		return;

	WRITE_FACE_FD("clear\n");
}


void
slrnface_suppress_xface(
	void)
{
	if (slrnface_fd < 0)
		return;

	WRITE_FACE_FD("suppress\n");
}


void
slrnface_show_xface(
	void)
{
	if (slrnface_fd < 0)
		return;

	WRITE_FACE_FD("show\n");
}

#else
static void no_xface(void);	/* proto-type */
static void
no_xface(	/* ANSI C requires non-empty source file */
	void)
{
}
#endif /* XFACE_ABLE */
