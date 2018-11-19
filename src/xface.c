/*
 *  Project   : tin - a Usenet reader
 *  Module    : xface.c
 *  Author    : Joshua Crawford & Drazen Kacar
 *  Created   : 2003-04-27
 *  Updated   : 2013-11-06
 *  Notes     :
 *
 * Copyright (c) 2003-2019 Joshua Crawford <mortarn@softhome.net> & Drazen Kacar <dave@willfork.com>
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


/*
 * TODO: - document the used vars/files/dir in the manpage
 *       - move strings to lang.c
 */

#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */

#ifdef XFACE_ABLE

static int slrnface_fd = -1;


void
slrnface_start(
	void)
{
	char *fifo;
	const char *ptr;
	int status;
	pid_t pid, pidst;
	size_t pathlen;
	struct utsname u;

	if (tinrc.use_slrnface == FALSE)
		return;

#ifdef HAVE_IS_XTERM
	if (!is_xterm()) {
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, _("Can't run slrnface: Not running in a xterm."));
#	endif /* DEBUG */
		return;
	}
#endif /* HAVE_IS_XTERM */

	/*
	 * $DISPLAY holds the (default) display name
	 */
	if (!getenv("DISPLAY")) {
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, _("Can't run slrnface: Environment variable %s not found."), "DISPLAY");
#	endif /* DEBUG */
		return;
	}

	/*
	 * $WINDOWID holds the X window id number of the xterm window
	 */
	if (!getenv("WINDOWID")) {
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, _("Can't run slrnface: Environment variable %s not found."), "WINDOWID");
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
			error_message(2, _("Can't run slrnface: Environment variable %s not found."), "HOME");
#	endif /* DEBUG */
		return;
	}
	pathlen = strlen(ptr) + strlen("/.slrnfaces/") + strlen(u.nodename) + 30;
	fifo = my_malloc(pathlen);
	snprintf(fifo, pathlen, "%s/.slrnfaces", ptr);
	if (my_mkdir(fifo, (mode_t) S_IRWXU)) {
		if (errno != EEXIST) {
			perror_message(_("Can't run slrnface: failed to create %s"), fifo);
			free(fifo);
			return;
		}
	} else {
		FILE *fp;

		/* We abuse fifo filename memory here. It is long enough. */
		snprintf(fifo, pathlen, "%s/.slrnfaces/README", ptr);
		if ((fp = fopen(fifo, "w")) != NULL) {
			fputs(_("This directory is used to create named pipes for communication between\n"
"slrnface and its parent process. It should normally be empty because\n"
"the pipe is deleted right after it has been opened by both processes.\n\n"
"File names generated by slrnface have the form \"hostname.pid\". It is\n"
"probably an error if they linger here longer than a fraction of a second.\n\n"
"However, if the directory is mounted from an NFS server, you might see\n"
"special files created by your NFS server while slrnface is running.\n"
"Do not try to remove them.\n"), fp);
			fclose(fp);
		}
	}

	status = snprintf(fifo, pathlen, "%s/.slrnfaces/%s.%ld", ptr, u.nodename, (long) getpid());
	if (status <= 0 || status >= (int) pathlen) {
		error_message(2, _("Can't run slrnface: couldn't construct fifo name."));
		unlink(fifo);
		free(fifo);
		return;
	}

	unlink(fifo);
	if (mkfifo(fifo, (S_IRUSR|S_IWUSR)) < 0) {
		perror_message(_("Can't run slrnface: failed to create %s"), fifo);
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
			 *       execlp("slrnface", "slrnface",
			 *              "-xOffsetChar", tinrc.xfacex,
			 *              "-yOffsetChar", tinrc.xfacey,
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
				pidst = waitpid(pid, &status, 0);
			} while (pidst == -1 && errno == EINTR);
			if (!WIFEXITED(status))
				error_message(2, _("Slrnface abnormally exited, code %d."), status);
			else {
				const char *message;

				switch (WEXITSTATUS(status)) {
					case 0:	/* All fine, open the pipe */
						slrnface_fd = open(fifo, O_WRONLY, (S_IRUSR|S_IWUSR));
						if (slrnface_fd != -1) {
							write(slrnface_fd, "start\n", strlen("start\n"));
							message = NULL;
						} else
							message = "can't open FIFO";
						break;

					/* TODO: warp into _()? */
					case 1:
						message = "couldn't connect to display";
						break;

					case 2:
						message = "WINDOWID not found in environment";
						break;

					case 3:
						message = "couldn't find controlling terminal";
						break;

					case 4:
						message = "terminal doesn't export width and height";
						break;

					case 5:
						message = "can't open FIFO";
						break;

					case 6:
						message = "fork() failed";
						break;

					case 10:
						message = "executable not found";
						break;

					default:
						message = "unknown error";
				}
				if (message)
					error_message(2, _("Slrnface failed: %s."), message);
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
		write(slrnface_fd, "clear\n", strlen("clear\n"));
	else {
		char buf[2000];	/* slrnface will ignore X-Faces larger than approx. 2000 chars. */

		snprintf(buf, sizeof(buf), "xface %s\n", face);
		write(slrnface_fd, buf, strlen(buf));
	}
}


void
slrnface_clear_xface(
	void)
{
	if (slrnface_fd < 0)
		return;

	write(slrnface_fd, "clear\n", strlen("clear\n"));
}


void
slrnface_suppress_xface(
	void)
{
	if (slrnface_fd < 0)
		return;

	write(slrnface_fd, "suppress\n", strlen("suppress\n"));
}


void
slrnface_show_xface(
	void)
{
	if (slrnface_fd < 0)
		return;

	write(slrnface_fd, "show\n", strlen("show\n"));
}

#else
static void no_xface(void);	/* proto-type */
static void
no_xface(	/* ANSI C requires non-empty source file */
	void)
{
}
#endif /* XFACE_ABLE */
