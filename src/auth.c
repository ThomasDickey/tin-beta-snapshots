/*
 *  Project   : tin - a Usenet reader
 *  Module    : auth.c
 *  Author    : Dirk Nimmich <nimmich@uni-muenster.de>
 *  Created   : 1997-04-05
 *  Updated   : 2000-01-22
 *  Notes     : Routines to authenticate to a news server via NNTP.
 *              DON'T USE get_respcode() THROUGHOUT THIS CODE.
 *
 * Copyright (c) 1997-2002 Dirk Nimmich <nimmich@uni-muenster.de>
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
 *    This product includes software developed by Dirk Nimmich.
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
 * we don't need authentication stuff at all if we don't have NNTP support
 */
#ifdef NNTP_ABLE

/*
 * local prototypes
 */
static int do_authinfo_original (char *server, char *authuser, char *authpass);
static t_bool authinfo_generic (void);
static t_bool read_newsauth_file (char *server, char *authuser, char *authpass);
static t_bool authinfo_original (char *server, char *authuser, t_bool startup);


/*
 * Process AUTHINFO GENERIC method.
 * TRUE means succeeded.
 * FALSE means failed
 */
static t_bool
authinfo_generic (
	void)
{
	char *authcmd;
	char authval[NNTP_STRLEN];
	char tmpbuf[NNTP_STRLEN];
	static int cookiefd = -1;
	t_bool builtinauth = FALSE;
#ifdef HAVE_PUTENV
	char *new_env;
	static char *old_env = 0;
#endif /* HAVE_PUTENV */

#ifdef DEBUG
	debug_nntp ("authorization", "authinfo generic");
#endif /* DEBUG */

	/*
	 * If we have authenticated before, NNTP_AUTH_FDS already
	 * exists, pull out the cookiefd. Just in case we've nested.
	 */
	if (cookiefd == -1 && (authcmd = getenv ("NNTP_AUTH_FDS")))
		(void) sscanf (authcmd, "%*d.%*d.%d", &cookiefd);

	if (cookiefd == -1) {
		char tempfile[PATH_LEN];
		if ((cookiefd = my_tmpfile_only(tempfile)) == -1) {
#	ifdef DEBUG
			debug_nntp ("authorization", txt_cannot_create_uniq_name);
#	endif /* DEBUG */
			return FALSE;
		} else {
			close(cookiefd);
			if (tempfile[0] != '\0')
				(void) unlink (tempfile);
		}
	}

	strcpy (tmpbuf, "AUTHINFO GENERIC ");
	STRCPY(authval, get_val ("NNTPAUTH", ""));
	if (strlen (authval))
		strcat (tmpbuf, authval);
	else {
		strcat (tmpbuf, "ANY ");
		strcat (tmpbuf, userid);
		builtinauth = TRUE;
	}
	put_server (tmpbuf);

#ifdef HAVE_PUTENV
	sprintf (tmpbuf, "NNTP_AUTH_FDS=%d.%d.%d",
			fileno (get_nntp_fp(FAKE_NNTP_FP)),
			fileno (get_nntp_wr_fp(FAKE_NNTP_FP)), cookiefd);
	new_env = (char *) my_malloc (strlen (tmpbuf) + 1);
	strcpy (new_env, tmpbuf);
	(void) putenv (new_env);
	FreeIfNeeded (old_env);
	old_env = new_env;
#else
#	ifdef HAVE_SETENV
	sprintf (tmpbuf, "%d.%d.%d",
			fileno (get_nntp_fp(FAKE_NNTP_FP)),
			fileno (get_nntp_wr_fp(FAKE_NNTP_FP)), cookiefd);
	setenv ("NNTP_AUTH_FDS", tmpbuf, 1);
#	endif /* HAVE_SETENV */
#endif /* HAVE_PUTENV */

	/* TODO - is it possible that we should have drained server here ? */
	return (builtinauth ? (get_only_respcode(NULL) == OK_AUTH) : (invoke_cmd (authval) ? TRUE : FALSE));
}


/*
 * Read the $HOME/.newsauth file and put authentication username
 * and password for the specified server in the given strings.
 * Returns TRUE if at least a password was found, FALSE if there was
 * no .newsauth file or no matching server.
 */
static t_bool
read_newsauth_file (
	char *server,
	char *authuser,
	char *authpass)
{
	FILE *fp;
	char *_authpass;
	char *ptr;
	char line[PATH_LEN];
	int found = 0;
	struct stat statbuf;

	joinpath (line, homedir, ".newsauth");

	if (stat (line, &statbuf) == -1)
		return FALSE;
	else {
		if (S_ISREG(statbuf.st_mode) && (statbuf.st_mode|S_IRUSR|S_IWUSR) != (S_IRUSR|S_IWUSR|S_IFREG)) {
			/* FIXME: -> lang.c */
			error_message (_("Insecure permissions of %s (%o)"), line, statbuf.st_mode);
			sleep(2);
		}
	}

	if ((fp = fopen (line, "r"))) {

		/*
		 * Search through authorization file for correct NNTP server
		 * File has format:  'nntp-server' 'password' ['username']
		 */
		while (fgets (line, PATH_LEN, fp) != (char *) 0) {

			/*
			 * strip trailing newline character
			 */

			ptr = strchr (line, '\n');
			if (ptr != (char *) 0)
				*ptr = '\0';

			/*
			 * Get server from 1st part of the line
			 */

			ptr = strpbrk (line, " \t");

			if (ptr == (char *) 0)		/* no passwd, no auth, skip */
				continue;

			*ptr++ = '\0';		/* cut off server part */

			if ((strcasecmp (line, server)))
				continue;		/* wrong server, keep on */

			/*
			 * Get password from 2nd part of the line
			 */

			while (*ptr == ' ' || *ptr == '\t')
				ptr++;	/* skip any blanks */

			_authpass = ptr;

			if (*_authpass == '"') {	/* skip "embedded" password string */
				ptr = strrchr (_authpass, '"');
				if ((ptr != (char *) 0) && (ptr > _authpass)) {
					_authpass++;
					*ptr++ = '\0';	/* cut off trailing " */
				} else	/* no matching ", proceede as normal */
					ptr = _authpass;
			}

			/*
			 * Get user from 3rd part of the line
			 */

			ptr = strpbrk (ptr, " \t");	/* find next separating blank */

			if (ptr != (char *) 0) {	/* a 3rd argument follows */
				while (*ptr == ' ' || *ptr == '\t')	/* skip any blanks */
					*ptr++ = '\0';
				if (*ptr != '\0')	/* if its not just empty */
					strcpy (authuser, ptr);	/* so will replace default user */
			}
			strcpy (authpass, _authpass);
			found++;
			break;	/* if we end up here, everything seems OK */
		}
		fclose (fp);
		return (found > 0);
	} else
		return FALSE;
}


/*
 * Perform authentication with ORIGINAL AUTHINFO method. Return response
 * code from server.
 */
static int
do_authinfo_original (
	char *server,
	char *authuser,
	char *authpass)
{
	char line[PATH_LEN];
	int ret;

	sprintf (line, "AUTHINFO USER %s", authuser);
#ifdef DEBUG
	debug_nntp ("authorization", line);
#endif /* DEBUG */
	put_server (line);
	if ((ret = get_only_respcode(NULL)) != NEED_AUTHDATA)
		return ret;

	if ((authpass == (char *) 0) || (*authpass == '\0')) {
#ifdef DEBUG
		debug_nntp ("authorization", "failed: no password");
#endif /* DEBUG */
		error_message (_(txt_nntp_authorization_failed), server);
		return ERR_AUTHBAD;
	}

	sprintf (line, "AUTHINFO PASS %s", authpass);
#ifdef DEBUG
	debug_nntp ("authorization", line);
#endif /* DEBUG */
	put_server (line);
	wait_message(2, (((ret = get_only_respcode(line)) == OK_AUTH) ? _(txt_authorization_ok) : _(txt_authorization_fail)), authuser);
	return ret;
}


/*
 * NNTP user authorization. Returns TRUE if authorization succeeded,
 * FALSE if not.
 * Password read from ~/.newsauth or, if not present or no matching
 * server found, from console.
 * The ~/.newsauth authorization file has the format:
 *   nntpserver1 password [user]
 *   nntpserver2 password [user]
 *   etc.
 */
static t_bool
authinfo_original (
	char *server,
	char *authuser,
	t_bool startup)
{
	char *authpass;
	char authusername[PATH_LEN];
	char authpassword[PATH_LEN];
	int ret = ERR_AUTHBAD, changed;
	static char last_server[PATH_LEN] = "";
	static t_bool already_failed = FALSE;

#ifdef DEBUG
	debug_nntp ("authorization", "original authinfo");
#endif /* DEBUG */

	authpassword[0] = '\0';
	authuser = strncpy (authusername, authuser, PATH_LEN);
	authpass = authpassword;

	/*
	 * read .newsauth only if we had not failed authentication yet for the
	 * current server (we don't want to try wrong username/password pairs
	 * more than once because this may lead to an infinite loop at connection
	 * startup: nntp_open tries to authenticate, it fails, server closes
	 * connection; next time tin tries to access the server it will do
	 * nntp_open again ...). This means, however, that if configuration
	 * changed on the server between two authentication attempts tin will
	 * prompt you the second time instead of reading .newsauth (except when
	 * at startup time; in this case, it will just leave); you have to leave
	 * and restart tin or change to another server and back in order to get
	 * it read again.
	 */
	if ((changed = strcmp (server, last_server)) || (!changed && !already_failed)) {
		already_failed = FALSE;
		if (read_newsauth_file (server, authuser, authpass)) {
			ret = do_authinfo_original (server, authuser, authpass);
			if (!(already_failed = (ret != OK_AUTH))) {
#ifdef DEBUG
				debug_nntp ("authorization", "succeeded");
#endif /* DEBUG */
				return TRUE;
			}
		}
	}

	/*
	 * At this point, either authentication with username/password pair from
	 * .newsauth has failed or there's no .newsauth file respectively no
	 * matching username/password for the current server. If we are not at
	 * startup we ask the user to enter such a pair by hand. Don't ask him
	 * startup except if requested by -A option because if he doesn't need
	 * authenticate (we don't know), the "Server expects authentication"
	 * messages are annoying (and even wrong).
	 * UNSURE: Maybe we want to make this decision configurable in the
	 * options menu, too, so that the user doesn't need -A.
	 * TODO: Put questions into do_authinfo_original because it is possible
	 * that the server doesn't want a password; so only ask for it if needed.
	 */
	if (force_auth_on_conn_open || !startup) {
#ifdef USE_CURSES
		int state = RawState();
#endif /* USE_CURSES */

		wait_message (0, _(txt_auth_needed));
#ifdef USE_CURSES
		Raw(TRUE);
#endif /* USE_CURSES */

		if (!prompt_default_string(_(txt_auth_user), authuser, PATH_LEN, authusername, HIST_NONE)) {
#ifdef DEBUG
			debug_nntp ("authorization", "failed: no username");
#endif /* DEBUG */
			return FALSE;
		}

#ifdef USE_CURSES
		Raw(state);
		my_printf ("%s", _(txt_auth_pass));
		wgetnstr (stdscr, authpassword, sizeof(authpassword));
		Raw(TRUE);
#else
#	if 0
		/*
		 * on some systems (i.e. Solaris) getpass(3) is limited to 8 chars ->
		 * we use tin_getline() till we have a config check
		 * for getpass() or our own getpass()
		 */
		authpass = strncpy (authpassword, getpass (_(txt_auth_pass)), PATH_LEN);
#	else
		authpass = strncpy (authpassword, tin_getline (_(txt_auth_pass), FALSE, (char *) 0, PATH_LEN, TRUE, HIST_NONE), PATH_LEN);
#	endif /* 0 */
#endif /* USE_CURSES */

		ret = do_authinfo_original (server, authuser, authpass);
		my_retouch();			/* Get rid of the chaff */
	}

#ifdef DEBUG
	debug_nntp ("authorization", (ret == OK_AUTH ? "succeeded" : "failed"));
#endif /* DEBUG */

	return (ret == OK_AUTH);
}


/*
 * Do authentication stuff. Return TRUE if authentication was successful,
 * FALSE otherwise.
 *
 * First try AUTHINFO GENERIC method, then, if that failed, ORIGINAL
 * AUTHINFO method. Other authentication methods can easily be added.
 */
t_bool
authenticate (
	char *server,
	char *user,
	t_bool startup)
{
	return (authinfo_generic () || authinfo_original (server, user, startup));
}

#else
static void no_authenticate (void);			/* proto-type */
static void
no_authenticate (					/* ANSI C requires non-empty source file */
	void)
{
}
#endif /* NNTP_ABLE */
