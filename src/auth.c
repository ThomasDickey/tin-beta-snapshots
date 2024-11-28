/*
 *  Project   : tin - a Usenet reader
 *  Module    : auth.c
 *  Author    : Dirk Nimmich <nimmich@muenster.de>
 *  Created   : 1997-04-05
 *  Updated   : 2024-11-25
 *  Notes     : Routines to authenticate to a news server via NNTP.
 *              DON'T USE get_respcode() THROUGHOUT THIS CODE.
 *
 * Copyright (c) 1997-2025 Dirk Nimmich <nimmich@muenster.de>
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
static int do_authinfo_user(char *server, char *authuser, const char *authpass);
static t_bool read_newsauth_file(const char *server, char *authuser, char *authpass);
static t_bool authinfo_plain(char *server, char *authuser, t_bool startup);
static char *prompt_for_authid(const char *authuser);
static char *prompt_for_password(void);
#	ifdef USE_GSASL
	static int sasl_auth(char *user, char *pass);
	static int do_authinfo_sasl(char *authuser, char *authpass);
	/* static int callback(Gsasl *ctx, Gsasl_session *sctx, Gsasl_property prop); */
#	endif /* USE_GSASL */


/*
 * Read the ${TIN_HOMEDIR:-"$HOME"}/.newsauth file and put authentication
 * username and password for the specified server in the given strings.
 * Returns TRUE if at least a password was found, FALSE if there was
 * no .newsauth file or no matching server.
 */
static t_bool
read_newsauth_file(
	const char *server,
	char *authuser,
	char *authpass)
{
	FILE *fp;
	char *_authpass;
	char *ptr;
	char filename[PATH_LEN];
	char line[PATH_LEN];
	int found = 0;
#	ifndef FILE_MODE_BROKEN
	int fd;
	struct stat statbuf;
#	endif /* !FILE_MODE_BROKEN */

	joinpath(filename, sizeof(filename), homedir, ".newsauth");

	if ((fp = tin_fopen(filename, "r")) == NULL)
		return FALSE;

#	ifndef FILE_MODE_BROKEN
	if ((fd = fileno(fp)) == -1) {
		fclose(fp);
		return FALSE;
	}

	if (fstat(fd, &statbuf) == -1) {
		fclose(fp);
		return FALSE;
	}

	if (S_ISREG(statbuf.st_mode) && (statbuf.st_mode|S_IRUSR|S_IWUSR) != (S_IRUSR|S_IWUSR|S_IFREG)) {
		error_message(4, _(txt_error_insecure_permissions), filename, statbuf.st_mode);
		/*
		 * TODO: fix permissions?
		 * #ifdef HAVE_FCHMOD
		 *  fchmod(fd, S_IRUSR|S_IWUSR);
		 * #else
		 * #	ifdef HAVE_CHMOD
		 *  chmod(filename, S_IRUSR|S_IWUSR);
		 * #	endif
		 * #endif
		 */
	}
#	endif /* !FILE_MODE_BROKEN */

	/*
	 * Search through authorization file for correct NNTP server
	 * File has format: 'nntp-server' 'password' ['username']
	 */
	while (fgets(line, sizeof(line), fp) != NULL) {
		/* strip trailing newline character */
		if ((ptr = strchr(line, '\n')) != NULL)
			*ptr = '\0';

		/* Get server from 1st part of the line */
		ptr = strpbrk(line, " \t");

		if (ptr == NULL || *line == '#')		/* comment or no passwd, no auth, skip */
			continue;

		*ptr++ = '\0';		/* cut off server part */

		/* allow ":port" suffix in .newsauth */
		{
			char *p;
			char hn[262]; /* [^\W_]{1,255}(:\d{,5})? */

			if ((p = strchr(line, ':')) != NULL) {
				if (*line != '[' && strrchr(line, ':') == p) { /* exact 1 x ':' must be [name|ipv4]:port */
					snprintf(hn, sizeof(hn), "%s:%u", server, nntp_tcp_port);
					if ((strcasecmp(line, hn)))
						continue;
				} else { /* "[ipv6]"[:port] */
					char *q;

					if (*line == '[' && (q = strrchr(line, ']')) != NULL) {
						if ((p = strchr(line, ':')) != NULL) {
							if (p > q) /* not an IPv6 */
								continue;
						}
						if ((p = strchr(q, ':')) != NULL) {
							if (p == q + 1) {
								snprintf(hn, sizeof(hn), "[%s]:%u", server, nntp_tcp_port);
								if ((strcasecmp(line, hn)))
									continue;
							}
						}
					}
				}
			} else {
				if ((strcasecmp(line, server)))
					continue;
			}
		}

		if (!(strcasecmp(line, server))) {
			/* Get password from 2nd part of the line */
			while (*ptr == ' ' || *ptr == '\t')
				++ptr;	/* skip any blanks */

			_authpass = ptr;

			if (*_authpass == '"') {	/* skip "embedded" password string */
				ptr = strrchr(_authpass, '"');
				if ((ptr != NULL) && (ptr > _authpass)) {
					_authpass++;
					*ptr++ = '\0';	/* cut off trailing " */
				} else	/* no matching ", proceed as normal */
					ptr = _authpass;
			}

			/* Get user from 3rd part of the line */
			ptr = strpbrk(ptr, " \t");	/* find next separating blank */

			if (ptr != NULL) {	/* a 3rd argument follows */
				while (*ptr == ' ' || *ptr == '\t')	/* skip any blanks */
					*ptr++ = '\0';
				if (*ptr != '\0')	/* if it is not just empty */
					strcpy(authuser, ptr);	/* so will replace default user */
			}
			strcpy(authpass, _authpass);
			++found;
			break;	/* if we end up here, everything seems OK */
		}
	}
	fclose(fp);
	return (found > 0);
}


/*
 * Perform authentication with AUTHINFO USER method. Return response
 * code from server.
 *
 * we don't handle ERR_ENCRYPT right now
 *
 * we don't convert authuser and authpass to UTF-8 as required by 3977
 * and we do in do_authinfo_sasl(); if we want to do so it should
 * likely be done in authinfo_plain() instead.
 */
static int
do_authinfo_user(
	char *server,
	char *authuser,
	const char *authpass)
{
	char line[NNTP_STRLEN];
	int ret;

	snprintf(line, sizeof(line), "AUTHINFO USER %.*s", NNTP_GRPLEN, authuser); /* RFC 3977 3.1 */
#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "authorization %s", line);
#	endif /* DEBUG */
	put_server(line, FALSE);
	if ((ret = get_only_respcode(NULL, 0)) != NEED_AUTHDATA)
		return ret;

	if ((authpass == NULL) || (*authpass == '\0')) {
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "authorization failed: no password");
#	endif /* DEBUG */
		error_message(2, _(txt_auth_failed_nopass), server);
		return ERR_AUTHBAD;
	}

	snprintf(line, sizeof(line), "AUTHINFO PASS %.*s", NNTP_GRPLEN, authpass); /* RFC 3977 3.1 */
#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "authorization %s", line);
#	endif /* DEBUG */
	put_server(line, TRUE);
	ret = get_only_respcode(line, sizeof(line));
	if (!batch_mode || verbose || ret != OK_AUTH)
		wait_message(2, (ret == OK_AUTH ? _(txt_authorization_ok) : _(txt_authorization_fail)), authuser);
	return ret;
}


/*
 * NNTP user authorization. Returns TRUE if authorization succeeded,
 * FALSE if not.
 *
 * tries AUTHINFO SASL PLAIN (if available) fist and if not successful
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
 *
 * TODO: - convert authuser and authpass to UTF-8 as required by 3977?
 *         (instead of doing so in do_authinfo_sasl())
 *       - prompt logic does NOT work with SASL mechs which do not
 *         require a username and/or password
 *       - rewrite from scratch like:
 *         - fill username/password from cache (reconnecting)
 *         - if still unset lookup user/password from file
 *         - if avail try sasl_auth (with callback if still unset user/password
 *           and _return_ username/password so it can be used with non sasl
 *           too)
 *         - if user/password are still empty prompt user
 *         - authinfo user/pass
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

	if ((changed = strcmp(server, last_server)))	/* do we need new auth values? */
		STRCPY(last_server, server);

	/*
	 * Let's try the previous auth pair first, if applicable.
	 * Else, proceed to the other mechanisms.
	 */
	if (initialized && !changed && !already_failed) {
#	ifdef USE_GSASL
		if (nntp_caps.authinfo_sasl && *nntp_caps.sasl_mechs)
			ret = do_authinfo_sasl(authusername, authpassword);
		if (ret != OK_AUTH_SASL && ret != OK_AUTH)
#	endif /* USE_GSASL */
		{
			if (nntp_caps.type != CAPABILITIES || nntp_caps.authinfo_user)
				ret = do_authinfo_user(server, authusername, authpassword);
		}
		return (ret == OK_AUTH || ret == OK_AUTH_SASL);
	}

	*authpassword = '\0';
	STRCPY(authusername, authuser);
	authuser = authusername;
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
#	ifdef USE_GSASL
			if (nntp_caps.authinfo_sasl && *nntp_caps.sasl_mechs)
				ret = do_authinfo_sasl(authuser, authpass);

			if (ret != OK_AUTH_SASL && ret != OK_AUTH)
#	endif /* USE_GSASL */
			{
				if (force_auth_on_conn_open || nntp_caps.type != CAPABILITIES || (nntp_caps.type == CAPABILITIES && nntp_caps.authinfo_user))
					ret = do_authinfo_user(server, authuser, authpass);
			}
			already_failed = (ret != OK_AUTH && ret != OK_AUTH_SASL);

			if (ret == OK_AUTH || ret == OK_AUTH_SASL) {
#	ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose > 1)
					debug_print_file("NNTP", "authorization succeeded");
#	endif /* DEBUG */
				initialized = TRUE;
				return TRUE;
			}
		}
#	ifdef DEBUG
		else {
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "read_newsauth_file(\"%s\", \"%s\", \"%s\") failed", server, authuser, authpass);
		}
#	endif /* DEBUG */
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
			return (ret == OK_AUTH || ret == OK_AUTH_SASL);
		}
		if (nntp_caps.type != CAPABILITIES || (nntp_caps.type == CAPABILITIES && !nntp_caps.authinfo_state && ((nntp_caps.authinfo_sasl && *nntp_caps.sasl_mechs) || nntp_caps.authinfo_user))) {
			char *u, *p;

			wait_message(0, _(txt_auth_needed));

			u = prompt_for_authid(authuser);
			p = prompt_for_password();

			if (p && *p) /* authpass points to authpassword */
				STRCPY(authpassword, p);
			else
				*authpassword = '\0';

			if (u && *u) /* authuser points to authusername */
				STRCPY(authusername, u);
			else
				*authusername = '\0';

			free(p);
			free(u);

#	ifdef USE_GSASL
			if (nntp_caps.authinfo_sasl && *nntp_caps.sasl_mechs)
				ret = do_authinfo_sasl(authuser, authpass);
			if (ret != OK_AUTH_SASL && ret != OK_AUTH)
#	endif /* USE_GSASL */
			{
				if (*authpass && *authuser && (nntp_caps.type != CAPABILITIES || (nntp_caps.authinfo_user || !nntp_caps.authinfo_sasl))) {
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
					 * if not advertised we simply go on but fully ignore
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
			 * can we change the state here? and if so how? SARTTLS (which
			 * we do not support, RFC 7525 3.2)? MODE READER?
			 */
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1) {
				debug_print_file("NNTP", "Authorization not allowed in current state");
				debug_print_file("NNTP", "\tCAPABILITIES: %s", nntp_caps.type ? (nntp_caps.type < 2 ? "CAPABILITIES" : "BROKEN") : "NONE");
				debug_print_file("NNTP", "\t%cREADER, %cMODE READER", nntp_caps.reader ? '+' : '-', nntp_caps.mode_reader ? '+' : '-');
				debug_print_file("NNTP", "\t%cSTARTTLS", nntp_caps.starttls ? '+' : '-');
				debug_print_file("NNTP", "\t%cAUTHINFO %s%s", nntp_caps.authinfo_state ? '-' : '+', nntp_caps.authinfo_user ? "USER " : "", nntp_caps.authinfo_sasl ? "SASL" : "");
				if (nntp_caps.authinfo_sasl) /* should not happen as we unset authinfo_sasl if we didn't find any mech we support */
					debug_print_file("NNTP", "\tSASL %s", BlankIfNull(nntp_caps.sasl_mechs));
			}
#	endif /* DEBUG */
			/*
			 * we return OK_AUTH here once so tin doesn't exit just because a
			 * single command requested auth ...
			 */
			if (!already_failed)
				ret = OK_AUTH;

			if (ret != OK_AUTH) /* TODO: also issue a hint for "-T" or to leave out "-A"? */
				wait_message(4, _(txt_authorization_unavail));
		}
	}

#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "authorization %s", (ret == OK_AUTH || ret == OK_AUTH_SASL) ? "succeeded" : "failed");
#	endif /* DEBUG */

	return (ret == OK_AUTH || ret == OK_AUTH_SASL);
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
	char line[NNTP_STRLEN];
	t_bool ret = authinfo_plain(server, user, startup);

	if (ret && nntp_caps.type == CAPABILITIES) {
		/* resend CAPABILITIES, but "manually" to avoid AUTH loop */
		snprintf(line, sizeof(line), "%s", "CAPABILITIES");
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "authenticate(%s)", line);
#	endif /* DEBUG */
		put_server(line, FALSE);

		check_extensions(get_only_respcode(line, sizeof(line)));
	}

	return ret;
}


#	ifdef USE_GSASL
static int
do_authinfo_sasl(
	char *authuser,
	char *authpass)
{
	char *utf8user = NULL;
	char *utf8pass = NULL;
	int ret;

	if (authuser && *authuser)
		utf8user = my_strdup(authuser);

	if (authpass && *authpass)
		utf8pass = my_strdup(authpass);

#		ifdef CHARSET_CONVERSION
	/* RFC 4616 */
	if (!IS_LOCAL_CHARSET("UTF-8")) {
		char *cp;
		int i, c = 0;
		t_bool contains_8bit = FALSE;

		/*
		 * if authuser or authpass contains non ASCII-chars
		 * convert both to UTF-8 even if this is a noop for
		 * one of them
		 */
		if (utf8user && *utf8user) {
			for (cp = utf8user; *cp; cp++) {
				if (!isascii((unsigned char) *cp)) {
					contains_8bit = TRUE;
					break;
				}
			}
		}
		if (!contains_8bit && utf8pass && *utf8pass) {
			for (cp = utf8pass; *cp; cp++) {
				if (!isascii((unsigned char) *cp)) {
					contains_8bit = TRUE;
					break;
				}
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
				if (utf8user && !buffer_to_network(&utf8user, c)) {
					free(utf8user);
					utf8user = my_strdup(authuser);
				}
				if (utf8pass && !buffer_to_network(&utf8pass, c)) {
					free(utf8pass);
					utf8pass = my_strdup(authpass);
				}
			}
		}
	}
#		endif /* CHARSET_CONVERSION */

#		ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "do_authinfo_sasl(\"%s\", \"%s\")", BlankIfNull(authuser), BlankIfNull(authpass));
#		endif /* DEBUG */

	ret = sasl_auth(utf8user, utf8pass);
	FreeIfNeeded(utf8user);
	FreeIfNeeded(utf8pass);

	if (!batch_mode || verbose || (ret != OK_AUTH_SASL && ret != OK_AUTH))
		wait_message(2, ((ret == OK_AUTH_SASL || ret == OK_AUTH) ? _(txt_authorization_ok) : _(txt_authorization_fail)), BlankIfNull(authuser));
	return ret;
}


/*
 * TODO: add cyrus-sasl variant
 *       support more mechs
 *       callbacks
 */
static int
sasl_auth(
	char *user,
	char *pass)
{
	Gsasl *ctx = NULL;
	Gsasl_session *session = NULL;
	char *out = NULL;
	char *in = NULL;
	const char *mech;
	char *suggest = nntp_caps.sasl_mechs;
	char line[8192]; /* hope that fits RFC 4643 2.4.1 */
	int rc, ret = ERR_AUTHFAIL;
	int sasl_prop;

	if (gsasl_init(&ctx) != GSASL_OK) /* TODO: do this only once at startup? */
		goto sasl_done;

	if ((mech = gsasl_client_suggest_mechanism(ctx, suggest)) == NULL)
		goto sasl_done;

	if (gsasl_client_start(ctx, mech, &session) != GSASL_OK)
		goto sasl_done;

	/* callback to prompt for missing data if needed */
	/* gsasl_callback_set(ctx, callback); */

	FreeIfNeeded(nntp_caps.sasl_mech_used);
	nntp_caps.sasl_mech_used = my_strdup(mech);
	sasl_prop = SASL_NEED_NONE;

	/* figure out required properties - align mechs with nntplib.c:check_extensions() */
	if (!strcasecmp(mech, "PLAIN") || !strcasecmp(mech, "LOGIN"))
		sasl_prop = (SASL_NEED_AUTHID | SASL_NEED_PASSWORD);
	if (!strcasecmp(mech, "ANONYMOUS"))
		sasl_prop = SASL_NEED_ANONYMOUS_TOKEN;

	/* set required props  ... see also callback() */
#if 0
	if (user && (sasl_prop & SASL_NEED_AUTHZID)) /* authorization identity, usually not used with NNTP, but GSSAPI? */
		gsasl_property_set(session, GSASL_AUTHZID, user);
#endif /* 0 */

	if (user && (sasl_prop & SASL_NEED_AUTHID))	/* authentication identity */
		gsasl_property_set(session, GSASL_AUTHID, user);

	if (pass && (sasl_prop & SASL_NEED_PASSWORD))	/* password of the authentication identity */
		gsasl_property_set(session, GSASL_PASSWORD, pass);

	if (sasl_prop & SASL_NEED_ANONYMOUS_TOKEN)
		gsasl_property_set(session, GSASL_ANONYMOUS_TOKEN, "dummy"); /* use a base64(randstr(len=whatever))?? */

	/* authenticate */
	snprintf(line, sizeof(line), "AUTHINFO SASL %s", mech);
	put_server(line, FALSE);

	do {
		ret = get_only_respcode(line, sizeof(line));
		rc = gsasl_step64(session, in, &out);

		if (ret != NEED_AUTHDATA_SASL || !out)
			break;

		if (rc == GSASL_NEEDS_MORE || ret == NEED_AUTHDATA_SASL)
			put_server(out, TRUE);

		if (rc == GSASL_NEEDS_MORE || rc == GSASL_OK)
			gsasl_free(out);
	} while (rc == GSASL_NEEDS_MORE);

	if (ret == NEED_AUTHDATA_SASL && rc == GSASL_OK)
		ret = get_only_respcode(line, sizeof(line));

	if (rc != GSASL_OK)
		error_message(batch_mode ? 0 : 4, "%s", gsasl_strerror(rc));

sasl_done:
	if (session)
		gsasl_finish(session);
	if (ctx)
		gsasl_done(ctx);

	if (ret != OK_AUTH_SASL && ret != OK_AUTH)
		FreeAndNull(nntp_caps.sasl_mech_used);

	return ret;
}


#if 0 /* prototype */
static int
callback(
	Gsasl *ctx,
	Gsasl_session *sctx,
	Gsasl_property prop
) {
	char *u, *p;
	int rc = GSASL_NO_CALLBACK;
	int c = 0, i = -1;

	(void) ctx;
	if (!IS_LOCAL_CHARSET("UTF-8")) { /* charset conversion likely (we don't check for 7bit only) needed? */
		for (i = 0; txt_mime_charsets[i] != NULL; i++) {
			if (!strcasecmp("UTF-8", txt_mime_charsets[i])) {
				c = i;
				break;
			}
		}
	}

	switch (prop) {
		case GSASL_AUTHID:
			if ((u = prompt_for_authid(NULL)) != NULL) {
				if (c == i) { /* convert to utf-8 */
					char *u8 = my_strdup(u);

					if (buffer_to_network(&u8, c))
						gsasl_property_set(sctx, prop, u8);
					else /* conversion failed, try plain string */
						gsasl_property_set(sctx, prop, u);

					free(u8);
				} else
					gsasl_property_set(sctx, prop, u);

				free(u);
				rc = GSASL_OK;
			}
			break;

		case GSASL_PASSWORD:
			if ((p = prompt_for_password()) != NULL) {
				if (c == i) { /* convert to utf-8 */
					char *p8 = my_strdup(p);

					if (buffer_to_network(&p8, c))
						gsasl_property_set(sctx, prop, p8);
					else /* conversion failed, try plain string */
						gsasl_property_set(sctx, prop, p);

					free(p8);
				} else
					gsasl_property_set(sctx, prop, p);

				free(p);
				rc = GSASL_OK;
			}
			break;

		GSASL_ANONYMOUS_TOKEN:
			gsasl_property_set(sctx, GSASL_ANONYMOUS_TOKEN, "dummy");
			rc = GSASL_OK;
			break;

		GSASL_SERVICE:
			gsasl_property_set(sctx, GSASL_SERVICE, "nntp");
			rc = GSASL_OK;
			break;

		case GSASL_HOSTNAME:
			gsasl_property_set(sctx, GSASL_HOSTNAME, nntp_server /* get_host_name() ? */);
			rc = GSASL_OK;
			break;

#if 0
		case GSASL_AUTHZID:
		case GSASL_PASSCODE:
		case GSASL_PIN:
		case GSASL_REALM:
#endif /* 0 */
		default: /* unhandled property */
			break;
	}
	return rc;
}
#endif /* 0 */
#	endif /* USE_GSASL */


/*
 * prompts for authid, suggests authuser
 * returns freshly allocated authid
 * caller must free result.
 */
static char *
prompt_for_authid(
	const char *authuser
) {
	char *authid;
	size_t maxlen = 255;
#	ifdef USE_CURSES
	int state = RawState();

	Raw(TRUE);
#	endif /* USE_CURSES */

	authid = my_malloc(maxlen--);
	if (!prompt_default_string(_(txt_auth_user), authid, maxlen, authuser, HIST_NONE)) {
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "authorization failed: no username");
#	endif /* DEBUG */

		*authid = '\0';
	}
#	ifdef USE_CURSES
	Raw(state);
#	endif /* USE_CURSES */

	return authid;
}


/*
 * prompts for authid related password
 * returns freshly allocated password
 * caller must free result.
 */
static char *
prompt_for_password(
	void
) {
	char *pass;
	size_t maxlen = 255;

#	ifdef USE_CURSES
	pass = my_malloc(maxlen--);
	my_printf("%s", _(txt_auth_pass));
	wgetnstr(stdscr, pass, maxlen);
	pass[maxlen] = '\0';
#	else
	/*
	 * on some systems (i.e. Solaris) getpass(3) is limited
	 * to 8 chars -> we use tin_getline()
	 */
	pass = my_strdup(tin_getline(_(txt_auth_pass), 0, NULL, maxlen, TRUE, HIST_NONE));
#	endif /* USE_CURSES */

	return pass;
}


#else
static void no_authenticate(void);			/* proto-type */
static void
no_authenticate(					/* ANSI C requires non-empty source file */
	void)
{
}
#endif /* NNTP_ABLE */
