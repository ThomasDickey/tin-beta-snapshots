/*
 *  Project   : tin - a Usenet reader
 *  Module    : open.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1999-07-17
 *  Notes     : Routines to make reading news locally (ie. /var/spool/news)
 *              or via NNTP transparent
 *
 * Copyright (c) 1991-2002 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Iain Lea, Rich Skrenta.
 * 4. The name of the author may not be used to endorse or promote
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */


/*
 * local prototypes
 */
#if 0 /* currently unused */
	static FILE * open_xhdr_fp (char *header, long min, long max);
#endif /* 0 */


long head_next;

#ifdef NO_POSTING
	t_bool can_post = FALSE;
#else
	t_bool can_post = TRUE;
#endif /* NO_POSTING */

char *nntp_server = (char *)0;
#ifdef NNTP_ABLE
	static char txt_xover_string[] = "XOVER";
	static char *txt_xover = txt_xover_string;
#endif /* NNTP_ABLE */


/*
 * Open a connection to the NNTP server. Authenticate if necessary or
 * desired, and test if the server supports XOVER.
 * Returns: 0	success
 *        > 0	NNTP error response code
 *        < 0	-errno from system call or similar error
 */
int
nntp_open (
	void)
{
#ifdef NNTP_ABLE
	char *linep;
	char line[NNTP_STRLEN];
	int ret;
	t_bool sec = FALSE;
	static t_bool is_reconnect = FALSE;

	if (!read_news_via_nntp)
		return 0;

#	ifdef DEBUG
	debug_nntp ("nntp_open", "BEGIN");
#	endif /* DEBUG */

	/* do this only once at start-up */
	if (!is_reconnect)
		nntp_server = getserverbyfile (NNTP_SERVER_FILE);

	if (nntp_server == (char *) 0) {
		error_message (_(txt_cannot_get_nntp_server_name));
		error_message (_(txt_server_name_in_file_env_var), NNTP_SERVER_FILE);
		return -EHOSTUNREACH;
	}

	if (!batch_mode) {
		if (nntp_tcp_port != 119)
			wait_message (0, _(txt_connecting_port), nntp_server, nntp_tcp_port);
		else
			wait_message (0, _(txt_connecting), nntp_server);
	}

#	ifdef DEBUG
	debug_nntp ("nntp_open", nntp_server);
#	endif /* DEBUG */

	ret = server_init (nntp_server, NNTP_TCP_NAME, nntp_tcp_port, line);
	DEBUG_IO((stderr, "server_init returns %d,%s\n", ret, line));

	if (!batch_mode && ret >= 0 && cmd_line)
		my_fputc ('\n', stdout);

#	ifdef DEBUG
	debug_nntp ("nntp_open", line);
#	endif /* DEBUG */

	switch (ret) {
		/*
		 * ret < 0 : some error from system call
		 * ret > 0 : NNTP response code
		 *
		 * According to the ietf-nntp mailinglist:
		 *   200 you may (try to) do anything
		 *   201 you may not POST
		 *  (202 you may not IHAVE)
		 *  (203 you may not do EITHER)
		 *   All unrecognised 200 series codes should be assumed as success.
		 *   All unrecognised 300 series codes should be assumed as notice to continue.
		 *   All unrecognised 400 series codes should be assumed as temporary error.
		 *   All unrecognised 500 series codes should be assumed as error.
		 */

		case OK_CANPOST:
/*		case OK_NOIHAVE: */
#	ifndef NO_POSTING
			can_post = TRUE;
#	endif /* !NO_POSTING */
			break;

		case OK_NOPOST:
/*		case OK_NOPOSTIHAVE: */
			can_post = FALSE;
			break;

		default:
			if (ret >= 200 && ret <= 299) {
#	ifndef NO_POSTING
				can_post = TRUE;
#	endif /* !NO_POSTING */
				break;
			}
			if (ret < 0) {
				error_message (_(txt_failed_to_connect_to_server), nntp_server);
			} else {
				error_message (line);
			}
			return ret;
	}
	if (!is_reconnect) {
		/* remove leading whitespace and save server's initial response */
		linep = line;
		while (isspace((int) *linep))
			linep++;

		STRCPY(bug_nntpserver1, linep);
	}

	/*
	 * Switch INN into NNRP mode with 'mode reader'
	 */

#	ifdef DEBUG
	debug_nntp ("nntp_open", "mode reader");
#	endif /* DEBUG */
	DEBUG_IO((stderr, "nntp_command(MODE READER)\n"));
	put_server ("MODE READER");

	/*
	 * According to the latest NNTP draft (Jan 2002), MODE READER may only
	 * return the following response codes:
	 *
	 *   200 (OK_CANPOST)     Hello, you can post
	 *   201 (OK_NOPOST)      Hello, you can't post
	 *  (202 (OK_NOIHAVE)     discussed on the itef mailinglist)
	 *  (203 (OK_NOPOSTIHAVE) discussed on the itef mailinglist)
	 *   400 (ERR_GOODBYE)    Service temporarily unavailable
	 *   502 (ERR_ACCESS)     Service unavailable
	 *
	 * However, there may be old servers out there that do not implement this
	 * command and therefore return ERR_COMMAND (500). Unfortunately there
	 * are some new servers out there (i.e. INN 2.4.0 (20020220 prerelease)
	 * which do return ERR_COMMAND if they are feed only servers.
	 */

	ret = get_respcode(line);
	switch (ret) {
		case OK_CANPOST:
/*		case OK_NOIHAVE: */
#	ifndef NO_POSTING
			can_post = TRUE;
#	endif /* !NO_POSTING */
			sec = TRUE;
			break;

		case OK_NOPOST:
/*		case OK_NOPOSTIHAVE: */
			can_post = FALSE;
			sec = TRUE;
			break;

		case ERR_GOODBYE:
		case ERR_ACCESS:
			error_message (line);
			return ret;

		case ERR_COMMAND:
		default:
			break;

	}

	/*
	 * NOTE: Latest NNTP draft (Jan 2002) states that LIST EXTENSIONS should
	 *       be used to find out what commands are supported.
	 *
	 * TODO: Implement LIST EXTENSIONS here. Get this list before issuing
	 *       authentication because the authentication method required may be
	 *       mentioned in the list of extensions. (For details about
	 *       authentication methods, see draft-newman-nntpext-auth-01.txt.)
	 */

	/*
	 * If the user wants us to authenticate on connection startup, do it now.
	 * Some news servers return "201 no posting" first, but after successful
	 * authentication you get a "200 posting allowed". To find out if we are
	 * allowed to post after authentication issue a "MODE READER" again and
	 * interpret the response code.
	 */

	if (force_auth_on_conn_open) {
#	ifdef DEBUG
		debug_nntp ("nntp_open", "authenticate");
#	endif /* DEBUG */
		authenticate (nntp_server, userid, TRUE);
		put_server ("MODE READER");
		ret = get_respcode (line);
		switch (ret) {
			case OK_CANPOST:
/*			case OK_NOIHAVE: */
#	ifndef NO_POSTING
				can_post = TRUE;
#	endif /* !NO_POSTING */
				sec = TRUE;
				break;

			case OK_NOPOST:
/*			case OK_NOPOSTIHAVE: */
				can_post = FALSE;
				sec = TRUE;
				break;

			case ERR_GOODBYE:
			case ERR_ACCESS:
				error_message (line);
				return ret;

			case ERR_COMMAND:	/* Uh-oh ... now we don't know if posting */
			default:				/* is allowed or not ... so use last 200 */
				break;			/* or 201 response to decide. */

		}
	}

	if (!is_reconnect) {
		/* Inform user if he cannot post */
		if (!can_post)
			wait_message(0, "%s\n", _(txt_cannot_post));

		/* Remove leading white space and save server's second response */
		linep = line;
		while (isspace((int) *linep))
			linep++;

		STRCPY(bug_nntpserver2, linep);

		/*
		 * Show user last server response line, do some nice formatting if
		 * response is longer than a screen wide.
		 *
		 * TODO: This only breaks the line once, but the response could be
		 * longer than two lines ...
		 */
		{
			char *chr1, *chr2;
			int j;

			j = atoi (get_val ("COLUMNS", "80"));
			chr1 = my_strdup ((sec ? bug_nntpserver2 : bug_nntpserver1));

			if (((int) strlen (chr1)) >= j) {
				chr2 = chr1 + strlen (chr1) - 1;
				while (chr2 - chr1 >= j)
					chr2--;
				while (chr2 > chr1 && *chr2 != ' ')
					chr2--;
				if (chr2 != chr1)
					*chr2 = '\n';
			}

			wait_message (0, "%s\n", chr1);
			free (chr1);
		}
	}

	/*
	 * Check if NNTP supports XOVER or OVER (successor of XOVER as of latest
	 * NNTP Draft (Jan 2002) command; ie, we _don't_ get an ERR_COMMAND
	 *
	 * TODO: Don't try (X)OVER if listed in LIST EXTENSIONS.
	 */

	if (!nntp_command(txt_xover_string, ERR_COMMAND, NULL)) {
		xover_supported = TRUE;
		txt_xover = txt_xover_string;
		/* TODO issue warning if old index files found ? */
	} else {
		if (!nntp_command(&txt_xover_string[1], ERR_COMMAND, NULL)) {
			xover_supported = TRUE;
			txt_xover = &txt_xover_string[1];
			/* TODO issue warning if old index files found ? */
		} else {
			if (!is_reconnect) {
				wait_message(2, _(txt_no_xover_support));
			}
		}
	}

#	if 0 /* TODO */
	/* if we're using -n, check for LIST NEWSGROUPS <wildmat> */
	if (newsrc_active && !list_active) { /* -n */
		/* code goes here */
	}
#	endif /* 0 */

	is_reconnect = TRUE;

#endif /* NNTP_ABLE */

	DEBUG_IO((stderr, "nntp_open okay\n"));
	return 0;
}


void
nntp_close (
	void)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp) {
#	ifdef DEBUG
		debug_nntp ("nntp_close", "END");
#	endif /* DEBUG */
		close_server ();
	}
#endif /* NNTP_ABLE */
}


/*
 * Get a response code from the server.
 * Returns:
 * 	+ve NNTP return code
 * 	-1  on an error or user abort. We don't differentiate.
 * If 'message' is not NULL, then any trailing text after the response
 * code is copied into it.
 * Does not perform authentication if required; use get_respcode()
 * instead.
 */
int
get_only_respcode (
	char *message)
{
	int respcode = 0;
#ifdef NNTP_ABLE
	char *ptr, *end;

	ptr = tin_fgets (FAKE_NNTP_FP, FALSE);

	if (tin_errno || ptr == NULL)
		return -1;

	respcode = (int) strtol(ptr, &end, 10);
DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));

	if ((respcode == ERR_FAULT /* || respcode == ERR_GOODBYE ??? */) &&
	    last_put[0] != '\0') {
		/*
		 * Maybe server timed out.
		 * If so, retrying will force a reconnect.
		 */
#	ifdef DEBUG
		debug_nntp ("get_only_respcode", "timeout");
#	endif /* DEBUG */
		put_server (last_put);
		ptr = tin_fgets (FAKE_NNTP_FP, FALSE);

		if (tin_errno)
			return -1;

		respcode = (int) strtol(ptr, &end, 10);
DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));
	}
	if (message != NULL)				/* Pass out the rest of the text */
		strcpy(message, end);

#endif /* NNTP_ABLE */
	return respcode;
}


/*
 * Get a response code from the server.
 * Returns:
 * 	+ve NNTP return code
 * 	-1  on an error
 * If 'message' is not NULL, then any trailing text after the response
 *	code is copied into it.
 * Performs authentication if required and repeats the last command if
 * necessary after a timeout.
 */
int
get_respcode (
	char *message)
{
	int respcode = 0;
#ifdef NNTP_ABLE
	char savebuf[NNTP_STRLEN];
	char *ptr, *end;

	respcode = get_only_respcode (message);
	if ((respcode == ERR_NOAUTH) || (respcode == NEED_AUTHINFO)) {
		/*
		 * Server requires authentication.
		 */
#	ifdef DEBUG
		debug_nntp ("get_respcode", "authentication");
#	endif /* DEBUG */
		strncpy (savebuf, last_put, NNTP_STRLEN);		/* Take copy, as authenticate() will clobber this */

		if (authenticate (nntp_server, userid, FALSE)) {
			strcpy (last_put, savebuf);

			put_server (last_put);
			ptr = tin_fgets (FAKE_NNTP_FP, FALSE);

			if (tin_errno)
				return -1;

			respcode = (int) strtol(ptr, &end, 10);
			if (message != NULL)				/* Pass out the rest of the text */
				strcpy(message, end);

		} else {
			error_message (_(txt_auth_failed), ERR_ACCESS);
			/*	return -1; */
			tin_done (EXIT_FAILURE);
		}
	}
#endif /* NNTP_ABLE */
	return respcode;
}


#ifdef NNTP_ABLE
/*
 * Do an NNTP command. Send command to server, and read the reply.
 * If the reply code matches success, then return an open file stream
 * Return NULL if we did not see the response we wanted.
 * If message is not NULL, then the trailing text of the reply string is
 * copied into it for the caller to process.
 */
FILE *
nntp_command (
	const char *command,
	int success,
	char *message)
{
DEBUG_IO((stderr, "nntp_command (%s)\n", command));
#	ifdef DEBUG
	debug_nntp ("nntp command", command);
#	endif /* DEBUG */
	put_server (command);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if ((get_respcode (message)) != success) {
#	ifdef DEBUG
			debug_nntp (command, "NOT_OK");
#	endif /* DEBUG */
			/* error_message ("%s", message); */
			return (FILE *) 0;
		}
	}
#	ifdef DEBUG
	debug_nntp (command, "OK");
#	endif /* DEBUG */
	return FAKE_NNTP_FP;
}
#endif /* NNTP_ABLE */


/*
 * Open the news active file locally or send the LIST command
 */
FILE *
open_news_active_fp (
	void)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news)
		return (nntp_command ("LIST", OK_GROUPS, NULL));
	else
#endif /* NNTP_ABLE */
		return (fopen (news_active_file, "r"));
}


/*
 * Open the NEWSLIBDIR/overview.fmt file locally or send LIST OVERVIEW.FMT
 */
FILE *
open_overview_fmt_fp (
	void)
{
	char line[NNTP_STRLEN];

#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news) {
		if (!xover_supported)
			return (FILE *) 0;

		sprintf (line, "LIST %s", OVERVIEW_FMT);
		return (nntp_command (line, OK_GROUPS, NULL));
	} else {
#endif /* NNTP_ABLE */
		joinpath (line, libdir, OVERVIEW_FMT);
		return (fopen (line, "r"));
#ifdef NNTP_ABLE
	}
#endif /* NNTP_ABLE */
}


/*
 * Open the active.times file locally or send the NEWGROUPS command
 *
 * NEWGROUPS yymmdd hhmmss
 */
FILE *
open_newgroups_fp (
	int idx)
{
#ifdef NNTP_ABLE
	char line[NNTP_STRLEN];
	struct tm *ngtm;

	if (read_news_via_nntp && !read_saved_news) {
		if (idx == -1)
			return (FILE *) 0;

		ngtm = localtime (&newnews[idx].time);
	/*
	 * in the current draft NEWGROUPS is allowed to take a 4 digit year
	 * componennt - but even with a 2 digit year componennt it is y2k
	 * compilant... we should switch over to ngtm->tm_year + 1900
	 * after most of the server could handle the new format
	 */
		sprintf (line, "NEWGROUPS %02d%02d%02d %02d%02d%02d",
			ngtm->tm_year % 100, ngtm->tm_mon + 1, ngtm->tm_mday,
			ngtm->tm_hour, ngtm->tm_min, ngtm->tm_sec);

		return (nntp_command (line, OK_NEWGROUPS, NULL));
	} else
#endif /* NNTP_ABLE */
		return (fopen (active_times_file, "r"));
}


/*
 * Get a list of default groups to subscribe to
 */
/* TODO: fixme/checkme
 *      - logic seems to be wrong, NNTP_ABLE && read_saved_news
 *        looks for a local subscriptions_file, but read_saved_news
 *        doesn't require a local server...
 *        open_newgroups_fp() uses the same logic.
 */
FILE *
open_subscription_fp (
	void)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news)
		return (nntp_command ("LIST SUBSCRIPTIONS", OK_GROUPS, NULL));
	else
#endif /* NNTP_ABLE */
		return (fopen (subscriptions_file, "r"));
}


#ifdef HAVE_MH_MAIL_HANDLING
/*
 * Open the mail active file locally
 */
FILE *
open_mail_active_fp (
	const char *mode)
{
	return fopen (mail_active_file, mode);
}


/*
 *  Open mail groups description file locally
 */
FILE *
open_mailgroups_fp (
	void)
{
	return fopen (mailgroups_file, "r");
}
#endif /* HAVE_MH_MAIL_HANDLING */


/*
 * If reading via NNTP the newsgroups file will be saved to ~/.tin/newsgroups
 * so that any subsequent rereads on the active file will not have to waste
 * net bandwidth and the local copy of the newsgroups file can be accessed.
 */
FILE *
open_newsgroups_fp (
	void)
{
#ifdef NNTP_ABLE
	FILE *result;
	if (read_news_via_nntp && !read_saved_news) {
		if (read_local_newsgroups_file) {
			result = fopen (local_newsgroups_file, "r");
			if (result != NULL) {
#	ifdef DEBUG
				debug_nntp ("open_newsgroups_fp", "Using local copy of newsgroups file");
#	endif /* DEBUG */
				return result;
			}
			read_local_newsgroups_file = FALSE;
		}
#	if 0 /* TODO */
		if (list_newsgroups_wildmat_supported && newsrc_active
		    && !list_active
		    && num_active < some_useful_limit) {
			for (i = 0; i < num_active; i++) {
				sprintf(buff, "LIST NEWSGROUPS %s", active[i].name);
				nntp_command(buff, OK_LIST, NULL);
			}
		} else
#	endif /* 0 */
		return (nntp_command ("LIST NEWSGROUPS", OK_GROUPS, NULL));
	} else
#endif /* NNTP_ABLE */
		return fopen (newsgroups_file, "r");
}


/*
 * Open a group NOV/XOVER file
 */
FILE *
open_xover_fp (
	struct t_group *psGrp,
	const char *mode,
	long lMin,
	long lMax)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && xover_supported && *mode == 'r' && psGrp->type == GROUP_TYPE_NEWS) {
		char line[NNTP_STRLEN];

		sprintf (line, "%s %ld-%ld", txt_xover, lMin, lMax);
		return (nntp_command (line, OK_XOVER, NULL));
	} else {
#endif /* NNTP_ABLE */
		char *pcNovFile;

		pcNovFile = find_nov_file (psGrp, (*mode == 'r' ? R_OK : W_OK));
#ifdef DEBUG
		if (debug)
			error_message ("READ file=[%s]", pcNovFile);
#endif /* DEBUG */
		if (pcNovFile != (char *) 0)
			return fopen (pcNovFile, mode);

		return (FILE *) 0;
#ifdef NNTP_ABLE
	}
#endif /* NNTP_ABLE */
}


/*
 * Stat a mail/news article to see if it still exists
 */
t_bool
stat_article (
	long art,
	const char *group_path)
{
	char buf[NNTP_STRLEN];
	struct t_group currgrp;

	currgrp = CURR_GROUP;

#ifdef NNTP_ABLE
	if (read_news_via_nntp && currgrp.type == GROUP_TYPE_NEWS) {
		sprintf (buf, "STAT %ld", art);
		return(nntp_command (buf, OK_NOTEXT, NULL) != NULL);
	} else
#endif /* NNTP_ABLE */
	{
		struct stat sb;

		joinpath (buf, currgrp.spooldir, group_path);
		sprintf (&buf[strlen (buf)], "/%ld", art);

		return (stat (buf, &sb) != -1);
	}
}


/*
 * Open an article for reading just the header
 */
FILE *
open_art_header (
	long art)
{
	char buf[NNTP_STRLEN];
#ifdef NNTP_ABLE
	FILE *fp;

	if (read_news_via_nntp && CURR_GROUP.type == GROUP_TYPE_NEWS) {
		/*
		 * Don't bother requesting if we have not got there yet.
		 * This is a big win if the group has got holes in it (ie. if 000's
		 * of articles have expired between active files min & max values).
		 */
		if (art < head_next)
			return (FILE *) 0;

		sprintf (buf, "HEAD %ld", art);
		if ((fp = nntp_command(buf, OK_HEAD, NULL)) != NULL)
			return(fp);

		/*
		 * HEAD failed, try to find NEXT
		 *	Should return "223 artno message-id more text...."
		 */
		if (nntp_command("NEXT", OK_NOTEXT, buf))
			head_next = atoi (buf);		/* Set next art number */

		return (FILE *) 0;
	} else {
#endif /* NNTP_ABLE */
		sprintf (buf, "%ld", art);
		return(fopen (buf, "r"));
#ifdef NNTP_ABLE
	}
#endif /* NNTP_ABLE */
}


/*
 * Open a mail/news article. Do MIME decoding if necessary.
 * Return:
 *		A pointer to the open postprocessed file
 *		NULL pointer if article read fails in some way
 */
FILE *
open_art_fp (
	const char *group_path,
	long art)
{
	char buf[NNTP_STRLEN];
	FILE *art_fp = (FILE *) 0;

#ifdef NNTP_ABLE
	if (read_news_via_nntp && CURR_GROUP.type == GROUP_TYPE_NEWS) {
		snprintf (buf, sizeof(buf) - 1, "ARTICLE %ld", art);
		art_fp = nntp_command(buf, OK_ARTICLE, NULL);
	} else {
#endif /* NNTP_ABLE */
		joinpath (buf, CURR_GROUP.spooldir, group_path);
		sprintf (&buf[strlen (buf)], "/%ld", art);

		art_fp = fopen (buf, "r");
#ifdef NNTP_ABLE
	}
#endif /* NNTP_ABLE */

	return art_fp;
}



/*
 *  Longword comparison routine for the qsort()
 */
static int
base_comp (
	t_comptype p1,
	t_comptype p2)
{
	const long *a = (const long *) p1;
	const long *b = (const long *) p2;

	if (*a < *b)
		return -1;

	if (*a > *b)
		return 1;

	return 0;
}


/*
 * via NNTP:
 *   Issue a LISTGROUP command
 *   Read the article numbers existing in the group into base[]
 *   If the LISTGROUP failed, issue a GROUP command. Use the results to
 *   create a less accurate version of base[]
 *	 This data will already be sorted
 *
 * on local spool:
 *   Read the spool dir to populate base[] as above. Sort it.
 *
 * Grow the arts[] and bitmaps as needed.
 * NB: the output will be sorted on artnum
 *
 * grpmenu.max is one past top.
 * Returns total number of articles in group, or -1 on error
 */
long
setup_hard_base (
	struct t_group *group,
	const char *group_path)
{
	char buf[NNTP_STRLEN];
	long art;
	long total = 0;

	grpmenu.max = 0;

	/*
	 * If reading with NNTP, issue a LISTGROUP
	 */
	if (read_news_via_nntp && group->type == GROUP_TYPE_NEWS) {
#ifdef NNTP_ABLE

#	ifdef BROKEN_LISTGROUP
		/*
		 * Some nntp servers are broken and need an extra GROUP command
		 * (reported by reorx@irc.pl). This affects (old?) versions of
		 * nntpcache and leafnode. Usually this should not be needed.
		 */
		sprintf (buf, "GROUP %s", group->name);
		if (nntp_command(buf, OK_GROUP, NULL) == NULL)
			return(-1);
#	endif /* BROKEN_LISTGROUP */

		/*
		 * See if LISTGROUP works
		 */
		sprintf (buf, "LISTGROUP %s", group->name);
		if (nntp_command(buf, OK_GROUP, NULL) != NULL) {
			char *ptr;

#	ifdef DEBUG
			debug_nntp ("setup_base", buf);
#	endif /* DEBUG */

			while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
				if (grpmenu.max >= max_art)
					expand_art ();

				base[grpmenu.max++] = atoi (ptr);
			}

			if (tin_errno)
				return(-1);

		} else {
			long start, last, count;
			char line[NNTP_STRLEN];

			/*
			 * Handle the obscure case that the user aborted before the LISTGROUP
			 * had a chance to respond
			 */
			if (tin_errno)
				return(-1);

			/*
			 * LISTGROUP failed, try a GROUP command instead
			 */
			sprintf (buf, "GROUP %s", group->name);
			if (nntp_command(buf, OK_GROUP, line) == NULL)
				return(-1);

			if (sscanf (line, "%ld %ld %ld", &count, &start, &last) != 3)
				return(-1);

			total = count;

			if (last - count > start)
				count = last - start;

			while (start <= last) {
				if (grpmenu.max >= max_art)
					expand_art();
				base[grpmenu.max++] = start++;
			}
		}
#endif /* NNTP_ABLE */
	/*
	 * Reading off local spool, read the directory files
	 */
	} else {
		DIR *d;
		DIR_BUF *e;

		joinpath (buf, group->spooldir, group_path);

		if (access (buf, R_OK) != 0) {
			error_message(_(txt_not_exist));
			return (-1);
		}

		if ((d = opendir (buf)) != NULL) {
			while ((e = readdir (d)) != NULL) {
				art = atol (e->d_name);
				if (art >= 1) {
					total++;
					if (grpmenu.max >= max_art)
						expand_art ();
					base[grpmenu.max++] = art;
				}
			}
			CLOSEDIR(d);
			qsort ((char *) base, (size_t)grpmenu.max, sizeof (long), base_comp);
		}
	}

	if (grpmenu.max) {
		if (base[grpmenu.max - 1] > group->xmax)
			group->xmax = base[grpmenu.max - 1];
		expand_bitmap (group, base[0]);
	}

	return total;
}


void
vGet1GrpArtInfo (
	struct t_group *grp)
{
	long lMinOld = grp->xmin;
	long lMaxOld = grp->xmax;

	group_get_art_info (grp->spooldir, grp->name, grp->type, &grp->count, &grp->xmax, &grp->xmin);

	if (grp->newsrc.num_unread > grp->count) {
#ifdef DEBUG
		my_printf (cCRLF "Unread WRONG %s unread=[%ld] count=[%ld]", grp->name, grp->newsrc.num_unread, grp->count);
		my_flush ();
#endif /* DEBUG */
		grp->newsrc.num_unread = grp->count;
	}

	if (grp->xmin != lMinOld || grp->xmax != lMaxOld) {
		expand_bitmap(grp, 0);
#ifdef DEBUG
		my_printf (cCRLF "Min/Max DIFF %s old=[%ld-%ld] new=[%ld-%ld]", grp->name, lMinOld, lMaxOld, grp->xmin, grp->xmax);
		my_flush ();
#endif /* DEBUG */
	}
}


/*
 *  Find the total, max & min articles number for specified group
 *  Use nntp GROUP command or read local spool
 *  Return 0, or -error
 */
int
group_get_art_info (
	char *tin_spooldir,
	char *groupname,
	int grouptype,
	long *art_count,
	long *art_max,
	long *art_min)
{
	DIR *dir;
	DIR_BUF *direntry;
	char buf[NNTP_STRLEN];
	long artnum;
#ifdef M_AMIGA
	long artmin;
	long artmax;

	artmin = *art_min;
	artmax = *art_max;
#endif /* M_AMIGA */

	if (read_news_via_nntp && grouptype == GROUP_TYPE_NEWS) {
#ifdef NNTP_ABLE
		char line[NNTP_STRLEN];

		sprintf (buf, "GROUP %s", groupname);
#	ifdef DEBUG
		debug_nntp ("group_get_art_info", buf);
#	endif /* DEBUG */
		put_server (buf);

		switch (get_respcode(line)) {

			case OK_GROUP:
				if (sscanf (line, "%ld %ld %ld", art_count, art_min, art_max) != 3)
					error_message(_("Invalid response to GROUP command, %s"), line);
				break;

			case ERR_NOGROUP:
				*art_count = 0;
				*art_min = 1;
				*art_max = 0;
				return(-ERR_NOGROUP);

			case ERR_ACCESS:
				error_message (cCRLF "%s", line);
				tin_done (NNTP_ERROR_EXIT);
				/* keep lint quiet: */
				/* NOTREACHED */
				break;

			default:
#	ifdef DEBUG
				debug_nntp ("NOT_OK", line);
#	endif /* DEBUG */
				return(-1);
		}
#else
		my_fprintf(stderr, _("Unreachable ?\n"));
		return 0;
#endif /* NNTP_ABLE */
	} else {
#ifdef M_AMIGA
		if (!artmin)
			*art_min = 1;
		*art_max = artmax;
		*art_count = artmax - *art_min + 1;
#else
		*art_count = 0;
		*art_min = 1;
		*art_max = 0;

		make_base_group_path (tin_spooldir, groupname, buf);

		if ((dir = opendir (buf)) != (DIR *) 0) {
			while ((direntry = readdir (dir)) != (DIR_BUF *) 0) {
				artnum = atol(direntry->d_name); /* should be '\0' terminated... */
				if (artnum >= 1) {
					if (artnum > *art_max) {
						*art_max = artnum;
						if (*art_min == 0)
							*art_min = artnum;
					} else if (artnum < *art_min)
						*art_min = artnum;
					(*art_count)++;
				}
			}
			CLOSEDIR(dir);
		} else
			return(-1);
#endif /* M_AMIGA */
	}

	return 0;
}


/* This will come in useful for filtering on non-overview hdr fields */
#if 0
static FILE *
open_xhdr_fp (
	char *header,
	long min,
	long max)
{
#	ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news) {
		char buf[NNTP_STRLEN];

		sprintf(buf, "XHDR %s %ld-%ld", header, min, max);
		return(nntp_command(buf, OK_HEAD));
	} else
#	endif /* NNTP_ABLE */
		return (FILE *) 0;		/* Some trick implementation for local spool... */
}
#endif /* 0 */
