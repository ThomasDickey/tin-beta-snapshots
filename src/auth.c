/*
 *  Project   : tin - a Usenet reader
 *  Module    : auth.c
 *  Author    : Dirk Nimmich <nimmich@muenster.de>
 *  Created   : 1997-04-05
 *  Updated   : 2013-12-06
 *  Notes     : Routines to authenticate to a news server via NNTP.
 *              DON'T USE get_respcode() THROUGHOUT THIS CODE.
 *
 * Copyright (c) 1997-2015 Dirk Nimmich <nimmich@muenster.de>
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
static int do_authinfo_user(char *server, char *authuser, char *authpass);
static t_bool read_newsauth_file(char *server, char *authuser, char *authpass);
static t_bool authinfo_plain(char *server, char *authuser, t_bool startup);
#	ifdef USE_SASL
	static char *sasl_auth_plain(char *user, char *pass);
	static int do_authinfo_sasl_plain(char *authuser, char *authpass);
#	endif /* USE_SASL */


/*
 * Read the ${TIN_HOMEDIR:-"$HOME"}/.newsauth file and put authentication
 * username and password for the specified server in the given strings.
 * Returns TRUE if at least a password was found, FALSE if there was
 * no .newsauth file or no matching server.
 */
static t_bool
read_newsauth_file(
	char *server,
	char *authuser,
	char *authpass)
{
	FILE *fp;
	char *_authpass;
	char *ptr;
	char filename[PATH_LEN];
	char line[PATH_LEN];
	int found = 0;
	int fd;
	struct stat statbuf;

	joinpath(filename, sizeof(filename), homedir, ".newsauth");

	if ((fp = fopen(filename, "r"))) {
		if ((fd = fileno(fp)) == -1) {
			fclose(fp);
			return FALSE;
		}
		if (fstat(fd, &statbuf) == -1) {
			fclose(fp);
			return FALSE;
		}

#	ifndef FILE_MODE_BROKEN
		if (S_ISREG(statbuf.st_mode) && (statbuf.st_mode|S_IRUSR|S_IWUSR) != (S_IRUSR|S_IWUSR|S_IFREG)) {
			error_message(4, _(txt_error_insecure_permissions), filename, statbuf.st_mode);
			/*
			 * TODO: fix permssions?
			 * fchmod(fd, S_IRUSR|S_IWUSR);
			 */
		}
#	endif /* !FILE_MODE_BROKEN */

		/*
		 * Search through authorization file for correct NNTP server
		 * File has format: 'nntp-server' 'password' ['username']
		 */
		while (fgets(line, sizeof(line), fp) != NULL) {
			/* strip trailing newline character */
			ptr = strchr(line, '\n');
			if (ptr != NULL)
				*ptr = '\0';

			/* Get server from 1st part of the line */
			ptr = strpbrk(line, " \t");

			if (ptr == NULL)		/* no passwd, no auth, skip */
				continue;

			*ptr++ = '\0';		/* cut off server part */

			if ((strcasecmp(line, server)))
				continue;		/* wrong server, keep on */

			/* Get password from 2nd part of the line */
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;	/* skip any blanks */

			_authpass = ptr;

			if (*_authpass == '"') {	/* skip "embedded" password string */
				ptr = strrchr(_authpass, '"');
				if ((ptr != NULL) && (ptr > _authpass)) {
					_authpass++;
					*ptr++ = '\0';	/* cut off trailing " */
				} else	/* no matching ", proceede as normal */
					ptr = _authpass;
			}

			/* Get user from 3rd part of the line */
			ptr = strpbrk(ptr, " \t");	/* find next separating blank */

			if (ptr != NULL) {	/* a 3rd argument follows */
				while (*ptr == ' ' || *ptr == '\t')	/* skip any blanks */
					*ptr++ = '\0';
				if (*ptr != '\0')	/* if its not just empty */
					strcpy(authuser, ptr);	/* so will replace default user */
			}
			strcpy(authpass, _authpass);
			found++;
			break;	/* if we end up here, everything seems OK */
		}
		fclose(fp);
		return (found > 0);
	}
	return FALSE;
}


/*
 * Perform authentication with AUTHINFO USER method. Return response
 * code from server.
 *
 * we don't handle ERR_ENCRYPT right now
 */
static int
do_authinfo_user(
	char *server,
	char *authuser,
	char *authpass)
{
	char line[PATH_LEN];
	int ret;

	snprintf(line, sizeof(line), "AUTHINFO USER %s", authuser);
#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "authorization %s", line);
#	endif /* DEBUG */
	put_server(line);
	if ((ret = get_only_respcode(NULL, 0)) != NEED_AUTHDATA)
		return ret;

	if ((authpass == NULL) || (*authpass == '\0')) {
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "authorization failed: no password");
#	endif /* DEBUG */
		error_message(2, _(txt_auth_failed_nopass), server);
		return ERR_AUTHBAD;
	}

	snprintf(line, sizeof(line), "AUTHINFO PASS %s", authpass);
#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "authorization %s", line);
#	endif /* DEBUG */
	put_server(line);
	ret = get_only_respcode(line, sizeof(line));
	if (!batch_mode || verbose || ret != OK_AUTH)
		wait_message(2, (ret == OK_AUTH ? _(txt_authorization_ok) : _(txt_authorization_fail)), authuser);
	return ret;
}


/*
 * NNTP user authorization. Returns TRUE if authorization succeeded,
 * FALSE if not.
 *
 * tries AUTHINFO SASL PLAIN (if available) fist and if not succcessfull
 * AUTHINFO USER/PASS
 *
 * If username/passwd already given, and server wasn't changed, retry those.
 * Otherwise, read password from ~/.newsauth or, if not present or no matching
 * server found, from console.
 *
 * The ~/.newsauth authorization file has the format:
 *   nntpserver1 password [user]
 *   nntpserver2 password [user]
 *   etc.
 */
static t_bool
authinfo_plain(
	char *server,
	char *authuser,
	t_bool startup)
{
	char *authpass;
	int ret = ERR_AUTHBAD, changed;
	static char authusername[PATH_LEN] = "";
	static char authpassword[PATH_LEN] = "";
	static char last_server[PATH_LEN] = "";
	static t_bool already_failed = FALSE;
	static t_bool initialized = FALSE;

	changed = strcmp(server, last_server);	/* do we need new auth values? */
	strncpy(last_server, server, PATH_LEN - 1);
	last_server[PATH_LEN - 1] = '\0';

	/*
	 * Let's try the previous auth pair first, if applicable.
	 * Else, proceed to the other mechanisms.
	 */
	if (initialized && !changed && !already_failed) {
#	ifdef USE_SASL
		if (nntp_caps.sasl & SASL_PLAIN)
			ret = do_authinfo_sasl_plain(authusername, authpassword);
		if (ret != OK_AUTH)
#	endif /* USE_SASL */
		{
			if (nntp_caps.type != CAPABILITIES || (nntp_caps.type == CAPABILITIES && nntp_caps.authinfo_user))
				ret = do_authinfo_user(server, authusername, authpassword);
		}
		return (ret == OK_AUTH);
	}

	authpassword[0] = '\0';
	authuser = strncpy(authusername, authuser, sizeof(authusername) - 1);
	authpass = authpassword;

	/*
	 * No username/password given yet.
	 * Read .newsauth only if we had not failed authentication yet for the
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
	if ((changed || !initialized) && !already_failed) {
		if (read_newsauth_file(server, authuser, authpass)) {
#	ifdef USE_SASL
			if (nntp_caps.sasl & SASL_PLAIN)
				ret = do_authinfo_sasl_plain(authuser, authpass);

			if (ret != OK_AUTH)
#	endif /* USE_SASL */
			{
				if (force_auth_on_conn_open || nntp_caps.type != CAPABILITIES || (nntp_caps.type == CAPABILITIES && nntp_caps.authinfo_user))
					ret = do_authinfo_user(server, authuser, authpass);
			}
			already_failed = (ret != OK_AUTH);

			if (ret == OK_AUTH) {
#	ifdef DEBUG
				if (debug & DEBUG_NNTP)
					debug_print_file("NNTP", "authorization succeeded");
#	endif /* DEBUG */
				initialized = TRUE;
				return TRUE;
			}
		}
	}

	/*
	 * At this point, either authentication with username/password pair from
	 * .newsauth has failed or there's no .newsauth file respectively no
	 * matching username/password for the current server. If we are not at
	 * startup we ask the user to enter such a pair by hand. Don't ask him
	 * at startup except if requested by -A option because if he doesn't need
	 * to authenticate (we don't know), the "Server expects authentication"
	 * messages are annoying (and even wrong).
	 * UNSURE: Maybe we want to make this decision configurable in the
	 * options menu, too, so that the user doesn't need -A.
	 * TODO: Put questions into do_authinfo_user() because it is possible
	 * that the server doesn't want a password; so only ask for it if needed.
	 */
	if (force_auth_on_conn_open || !startup) {
		if (batch_mode) { /* no interactive username/password prompting */
			error_message(0, _(txt_auth_needed));
			return (ret == OK_AUTH);
		}
		if (nntp_caps.type != CAPABILITIES || (nntp_caps.type == CAPABILITIES && !nntp_caps.authinfo_state && ((nntp_caps.sasl & SASL_PLAIN) || nntp_caps.authinfo_user || (!nntp_caps.authinfo_user && !(nntp_caps.sasl & SASL_PLAIN))))) {
#	ifdef USE_CURSES
			int state = RawState();
#	endif /* USE_CURSES */

			wait_message(0, _(txt_auth_needed));
#	ifdef USE_CURSES
			Raw(TRUE);
#	endif /* USE_CURSES */
			if (!prompt_default_string(_(txt_auth_user), authuser, PATH_LEN, authusername, HIST_NONE)) {
#	ifdef DEBUG
				if (debug & DEBUG_NNTP)
					debug_print_file("NNTP", "authorization failed: no username");
#	endif /* DEBUG */
				return FALSE;
			}

#	ifdef USE_CURSES
			Raw(state);
			my_printf("%s", _(txt_auth_pass));
			wgetnstr(stdscr, authpassword, sizeof(authpassword));
			Raw(TRUE);
#	else
			/*
			 * on some systems (i.e. Solaris) getpass(3) is limited to 8 chars ->
			 * we use tin_getline()
			 */
			authpass = strncpy(authpassword, tin_getline(_(txt_auth_pass), FALSE, NULL, PATH_LEN, TRUE, HIST_NONE), sizeof(authpassword) - 1);
#	endif /* USE_CURSES */

#	ifdef USE_SASL
			if (nntp_caps.sasl & SASL_PLAIN)
				ret = do_authinfo_sasl_plain(authuser, authpass);
			if (ret != OK_AUTH)
#	endif /* USE_SASL */
			{
				if (nntp_caps.type != CAPABILITIES || (nntp_caps.type == CAPABILITIES && (nntp_caps.authinfo_user || (!nntp_caps.authinfo_user && !nntp_caps.authinfo_sasl)))) {
#	ifdef DEBUG
					if (debug & DEBUG_NNTP) {
						if (nntp_caps.type == CAPABILITIES && !nntp_caps.authinfo_sasl && !nntp_caps.authinfo_user)
							debug_print_file("NNTP", "!!! No supported authmethod available, trying AUTHINFO USER/PASS");
					}
#	endif /* DEBUG */
					ret = do_authinfo_user(server, authuser, authpass);
					if (ret != OK_AUTH)
						already_failed = TRUE;
					/*
					 * giganews once responded to CAPABILITIES with just
					 * "VERSION 2", no mode-switching indication, no reader
					 * indication, no post indication, no authentication
					 * indication, ... so in case AUTHINFO USER/PASS succeeds
					 * if not advertized we simply go on but fully ignore
					 * CAPABILITIES
					 */
					if (nntp_caps.type == CAPABILITIES && !nntp_caps.authinfo_user && !nntp_caps.authinfo_sasl && ret == OK_AUTH)
						nntp_caps.type = BROKEN;
				}
			}
			initialized = TRUE;
			my_retouch();			/* Get rid of the chaff */
		} else {
			/*
			 * TODO:
			 * nntp_caps.type == CAPABILITIES && nntp_caps.authinfo_state
			 * can we change the sate here? and if so how? SARTTLS? MODE
			 * READER?
			 */
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "authorization not allowed in current sate");
#	endif /* DEBUG */
			/*
			 * we return OK_AUTH here once so tin doesn't exit just because a
			 * single command requested auth ...
			 */
			if (!already_failed)
				ret = OK_AUTH;
		}
	}

#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "authorization %s", (ret == OK_AUTH ? "succeeded" : "failed"));
#	endif /* DEBUG */

	return (ret == OK_AUTH);
}


/*
 * Do authentication stuff. Return TRUE if authentication was successful,
 * FALSE otherwise.
 *
 * try ORIGINAL AUTHINFO method.
 * Other authentication methods can easily be added.
 */
t_bool
authenticate(
	char *server,
	char *user,
	t_bool startup)
{
	return authinfo_plain(server, user, startup);
}


#	ifdef USE_SASL
static int
do_authinfo_sasl_plain(
	char *authuser,
	char *authpass)
{
	char line[PATH_LEN];
	char *foo;
	char *utf8user;
	char *utf8pass;
	int ret;
#		ifdef CHARSET_CONVERSION
	char *cp;
	int i, c = 0;
	t_bool contains_8bit = FALSE;
#		endif /* CHARSET_CONVERSION */

	utf8user = my_strdup(authuser);
	utf8pass = my_strdup(authpass);
#		ifdef CHARSET_CONVERSION
	/* RFC 4616 */
	if (!IS_LOCAL_CHARSET("UTF-8")) {
		for (cp = utf8user; *cp && !contains_8bit; cp++) {
			if (!isascii(*cp)) {
				contains_8bit = TRUE;
				break;
			}
		}
		for (cp = utf8pass; *cp && !contains_8bit; cp++) {
			if (!isascii(*cp)) {
				contains_8bit = TRUE;
				break;
			}
		}
		if (contains_8bit) {
			for (i = 0; txt_mime_charsets[i] != NULL; i++) {
				if (!strcasecmp("UTF-8", txt_mime_charsets[i])) {
					c = i;
					break;
				}
			}
			if (c == i) { /* should never fail */
				if (!buffer_to_network(utf8user, c)) {
					free(utf8user);
					free(utf8pass);
					utf8user = my_strdup(authuser);
					utf8pass = my_strdup(authpass);
				} else {
					if (!buffer_to_network(utf8pass, c)) {
						free(utf8user);
						free(utf8pass);
						utf8user = my_strdup(authuser);
						utf8pass = my_strdup(authpass);
					}
				}
			}
		}
	}
#		endif /* CHARSET_CONVERSION */

#		ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "do_authinfo_sasl_plain(%s, %s)", BlankIfNull(authuser), BlankIfNull(authpass));
#		endif /* DEBUG */

	if ((foo = sasl_auth_plain(utf8user, utf8pass)) == NULL) {
		free(utf8user);
		free(utf8pass);
		return ERR_AUTHBAD;
	}

	free(utf8user);
	free(utf8pass);

	snprintf(line, sizeof(line), "AUTHINFO SASL PLAIN %s", foo);
	FreeIfNeeded(foo);
#		ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "authorization %s", line);
#		endif /* DEBUG */
	put_server(line);
	ret = get_only_respcode(line, sizeof(line));
	if (!batch_mode || verbose || ret != OK_AUTH)
		wait_message(2, (ret == OK_AUTH ? _(txt_authorization_ok) : _(txt_authorization_fail)), authuser);
	return ret;
}


static char *
sasl_auth_plain(
	char *user,
	char *pass)
{
	Gsasl *ctx = NULL;
	Gsasl_session *session;
	char *p = NULL;
	const char *mech = "PLAIN";

	if (gsasl_init(&ctx) != GSASL_OK) /* TODO: do this only once at startup */
		return p;
	if (gsasl_client_start(ctx, mech, &session) != GSASL_OK) {
		gsasl_done(ctx);
		return p;
	}
	gsasl_property_set(session, GSASL_AUTHID, user);
	gsasl_property_set(session, GSASL_PASSWORD, pass);
	if (gsasl_step64(session, NULL, &p) != GSASL_OK)
		FreeAndNull(p);
	gsasl_finish(session);
	gsasl_done(ctx);
	return p;
}
#	endif /* USE_SASL */

#else
static void no_authenticate(void);			/* proto-type */
static void
no_authenticate(					/* ANSI C requires non-empty source file */
	void)
{
}
#endif /* NNTP_ABLE */
