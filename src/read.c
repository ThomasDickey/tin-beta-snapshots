/*
 *  Project   : tin - a Usenet reader
 *  Module    : read.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1997-04-10
 *  Updated   : 2005-07-02
 *
 * Copyright (c) 1997-2006 Jason Faultless <jason@altarstone.com>
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


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */


/*
 * The initial and expansion sizes to use for allocating read data
 */
#define INIT					512
#define RCHUNK					256

/*
 * Global error flag. Set if something abnormal happens during file I/O
 */
int tin_errno;

/* How many chars we read at last tin_read() */
static int offset = 0;

/*
 * local prototypes
 */
static char *tin_read(char *buffer, size_t len, FILE *fp, t_bool header);
#ifdef NNTP_ABLE
	static t_bool wait_for_input(void);
#endif /* NNTP_ABLE */


#ifdef NNTP_ABLE
/*
 * Used by the I/O read routine to look for keyboard input
 * Returns TRUE if user aborted with 'q' or 'z' (lynx-style)
 *         FALSE otherwise
 * TODO: document 'z' (, and allow it's remapping?) and 'Q'
 */
static t_bool
wait_for_input(
	void)
{
#	ifndef HAVE_SELECT
#		ifdef VMS
	int ch = ReadChNowait();

	if (ch == iKeyQuit || ch == 'z' || ch == iKeyAbort) {
		if (prompt_yn(cLINES, _(txt_read_abort), FALSE) == 1)
			return TRUE;
	}
	if (ch == iKeyQuitTin) {
		if (prompt_yn(cLINES, _(txt_read_exit), FALSE) == 1)
			tin_done(EXIT_SUCCESS);
	}
#		endif /* VMS */
#	else
	int nfds, ch;
	fd_set readfds;
	struct timeval tv;

	/*
	 * Main loop. Wait for input from keyboard or file or for a timeout.
	 */
	forever {
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
/*		FD_SET(fileno(NEED_REAL_NNTP_FD_HERE), &readfds); */

		tv.tv_sec = 0;		/* NNTP_READ_TIMEOUT; */
		tv.tv_usec = 0;

/*DEBUG_IO((stderr, "waiting on %d and %d...", STDIN_FILENO, fileno(fd)));*/
		if ((nfds = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv)) == -1) {
			if (errno != EINTR) {
				perror_message("select() failed");
				giveup();
			} else
				return FALSE;
		}

		/* No input pending */
		if (nfds == 0)
			return FALSE;

		/*
		 * Something is waiting. See what's cooking...
		 */
		if (nfds > 0) {

			/*
			 * User pressed something. If 'q'uit, then handle this. Process
			 * user input 1st so they get chance to quit on busy (or stalled)
			 * reads
			 */
			if (FD_ISSET(STDIN_FILENO, &readfds)) {
				if ((ch = ReadCh()) == EOF)
					return FALSE;

				/*
				 * check if keymap key was pressed. Unfortunately, keymap keys
				 * are encoded by a sequence of bytes starting with ESC, so we
				 * must first see if it was really an ESC or a keymap key before
				 * asking the user if he wants to abort.
				 */
				if (ch == iKeyAbort) {
					int keymap_ch = get_arrow_key(ch);

					if (keymap_ch != KEYMAP_UNKNOWN)
						ch = keymap_ch;
				}

				if (ch == iKeyQuit || ch == 'z' || ch == iKeyAbort) {
					if (prompt_yn(_(txt_read_abort), FALSE) == 1)
						return TRUE;
				}

				if (ch == iKeyQuitTin) {
					if (prompt_yn(_(txt_read_exit), FALSE) == 1)
						tin_done(EXIT_SUCCESS);
				}

			}

#		if 0
			/*
			 * Our file has something for us to read
			 */
			if (FD_ISSET(fileno(NEED_NNTP_FD_HERE), &readfds))
				return TRUE;
#		endif /* 0 */
		}

	}
#	endif /* !HAVE_SELECT */
/*
 * FIXME: insert !HAVE_SELECT code here - NB: we already have VMS specific
 *        code at the top of this function
 */
	return FALSE;
}
#endif /* NNTP_ABLE */


/*
 * Support routine to read a fixed size buffer. This does most of the
 * hard work for tin_fgets()
 */
static t_bool partial_read;


static char *
tin_read(
	char *buffer,
	size_t len,
	FILE *fp,
	t_bool header)
{
	char *ptr;
	signed char c;
	int i;
#ifdef NNTP_ABLE
	t_bool check_dot_only_line;

	/*
	 * We have to check '.' line when reading via NNTP and
	 * reading first line.
	 */
	check_dot_only_line = (header && fp == FAKE_NNTP_FP && partial_read == FALSE);
#endif /* NNTP_ABLE */

	partial_read = FALSE;

#ifdef NNTP_ABLE
	if (wait_for_input()) {			/* Check if okay to read */
		info_message(_("Aborting read, please wait..."));
		drain_buffer(fp);
		clear_message();
		tin_errno = TIN_ABORT;
		fflush(stdin);
		return NULL;
	}

	errno = 0;		/* To check errno after read, clear it here */

	/*
	 * Initially try and fit into supplied buffer
	 */
	if (fp == FAKE_NNTP_FP)
		ptr = get_server(buffer, len);
	else
		ptr = fgets(buffer, len, fp);
#else
	errno = 0;		/* To check errno after read, clear it here */

	ptr = fgets(buffer, len, fp);
#endif /* NNTP_ABLE */

/* TODO: develop this next line? */
#ifdef DEBUG
	if (errno)
		fprintf(stderr, "errno in tin_read %d\n", errno);
#endif /* DEBUG */

	if (ptr == 0)	/* End of data? */
		return NULL;

	/*
	 * Was this only a partial read?
	 * We strip trailing \r and \n here and here _only_
	 * 'offset' is the # of chars which we read now
	 */
	i = strlen(buffer);
	if (i >= 1 && buffer[i - 1] == '\n') {

		if (i >= 2 && buffer[i - 2] == '\r') {
			buffer[i - 2] = '\0';
			offset = i -= 2;
		} else {
			buffer[i - 1] = '\0';
			offset = --i;
		}

		/*
		 * If we're looking for continuation headers, mark this as a partial
		 * read and put back a newline. Unfolding (removing of this newline
		 * and whitespace, if necessary) must be done at a higher level --
		 * there are headers where whitespace is significant even in folded
		 * lines.
		 */
#ifdef NNTP_ABLE
		if (check_dot_only_line && i == 1 && buffer[0] == '.') { /* EMPTY */
			/* Find a terminator, don't check next line. */
		} else
#endif /* NNTP_ABLE */
		{
			if (header) {
				if (!i) { /* EMPTY */
					/* Find a header separator, don't check next line. */
				} else {
					if ((c = fgetc(get_nntp_fp(fp))) == ' ' || c == '\t') {
						partial_read = TRUE;
						/* This is safe because we removed at least one char above */
						buffer[offset++] = '\n';
						buffer[offset] = '\0';
					}

					/* Push back the 1st char of next line */
					if (c != EOF)
						ungetc(c, get_nntp_fp(fp));
				}
			}
		}
	} else {
		partial_read = TRUE;
		offset = i;
	}

	return buffer;
}


/*
 * This is the main routine for reading news data from local spool or NNTP.
 * It can handle arbitrary length lines of data, failed connections and
 * user initiated aborts (where possible)
 *
 * We simply request data from an fd and data is read up to the next \n
 * Any trailing \r and \n will be stripped.
 * If fp is FAKE_NNTP_FP, then we are reading via a socket to an NNTP
 * server. The required post-processing of the data will be done such that
 * we look like a local read to the calling function.
 *
 * Header lines: If header is TRUE, then we assume we're reading a news
 * article header. In some cases, article headers are split over multiple
 * lines. The rule is that if the next line starts with \t or ' ', then it
 * will be included as part of the current line. Line breaks are NOT
 * stripped (but replaced by \n) in continuated lines except the trailing
 * one; unfolding MUST be done at a higher level because it may be
 * significant or not.
 *
 * Dynamic read code based on code by <emcmanus@gr.osf.org>
 *
 * Caveat: We try to keep the code path for a trivial read as short as
 * possible.
 */
char *
tin_fgets(
	FILE *fp,
	t_bool header)
{
	static char *dynbuf = NULL;
	static int size = 0;

	char *temp, *ptr;
	int next;

	tin_errno = 0;					/* Clear errors */
	partial_read = FALSE;

#if 1
	if (fp == NULL) {
		FreeAndNull(dynbuf);
		return NULL;
	}
	/* Allocate initial buffer */
	if (dynbuf == NULL) {
		dynbuf = my_malloc(INIT * sizeof(*dynbuf));
		size = INIT;
	}
	/* Otherwise reuse last buffer */
	/* TODO: Should we free too large buffer? */
#else
	FreeIfNeeded(dynbuf);	/* Free any previous allocation */
	dynbuf = my_malloc(INIT * sizeof(*dynbuf));
	size = INIT;
#endif /* 1 */

	if ((ptr = tin_read(dynbuf, size, fp, header)) == NULL)
		return ptr;

	if (tin_errno != 0) {
		DEBUG_IO((stderr, _("Aborted read\n")));
		return NULL;
	}

	next = offset;

	while (partial_read) {
		if (next + RCHUNK > size)
			size = next + RCHUNK;
		temp = my_realloc(dynbuf, size * sizeof(*dynbuf));
		dynbuf = temp;
		temp = tin_read(dynbuf + next, size - next, fp, header); /* What if == 0? */
		next += offset;

		if (tin_errno != 0)
			return NULL;
	}

	/*
	 * Do processing of leading . for NNTP
	 * This only occurs at the start of lines
	 * At this point, dynbuf won't be NULL
	 */
#ifdef NNTP_ABLE
	if (fp == FAKE_NNTP_FP) {
		if (dynbuf[0] == '.') {			/* reduce leading .'s */
			if (dynbuf[1] == '\0') {
				DEBUG_IO((stderr, "tin_fgets(NULL)\n"));
				return NULL;
			}
			DEBUG_IO((stderr, "tin_fgets(%s)\n", dynbuf + 1));
			return (dynbuf + 1);
		}
	}
#endif /* NNTP_ABLE */

DEBUG_IO((stderr, "tin_fgets(%s)\n", (dynbuf) ? dynbuf : "NULL"));

	return dynbuf;
}


/*
 * We can't just stop reading a socket once we are through with it. This
 * drains out any pending data on the NNTP port
 */
#ifdef NNTP_ABLE
void
drain_buffer(
	FILE *fp)
{
	int i = 0;

	if (fp != FAKE_NNTP_FP)
		return;

DEBUG_IO((stderr, _("Draining\n")));
	while (tin_fgets(fp, FALSE) != NULL) {
		if (++i % MODULO_COUNT_NUM == 0)
			spin_cursor();
	}
}
#endif /* NNTP_ABLE */

/* end of read.c */
