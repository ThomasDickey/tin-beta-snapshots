/*
 *  Project   : tin - a Usenet reader
 *  Module    : header.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 1997-03-10
 *  Updated   : 2003-09-19
 *
 * Copyright (c) 1997-2004 Urs Janssen <urs@tin.org>
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

static const char *get_full_name(void);
static const char *get_user_name(void);


/* find hostname */
const char *
get_host_name(
	void)
{
	char *ptr;
	static char hostname[MAXHOSTNAMELEN + 1]; /* need space for '\0' */

	hostname[0] = '\0';

#ifdef HAVE_GETHOSTNAME
	gethostname(hostname, sizeof(hostname) - 1);
#endif /* HAVE_GETHOSTNAME */
#ifdef HAVE_SYS_UTSNAME_H
	if (!*hostname)
		strncpy(hostname, system_info.nodename, sizeof(hostname) - 1);
#endif /* HAVE_SYS_UTSNAME_H */
	if (!*hostname) {
		if ((ptr = getenv("HOST")) != NULL)
			my_strncpy(hostname, ptr, sizeof(hostname) - 1);
		else {
			if ((ptr = getenv("HOSTNAME")) != NULL)
				my_strncpy(hostname, ptr, sizeof(hostname) - 1);
			else
				hostname[0] = '\0';
		}
	}
	hostname[MAXHOSTNAMELEN] = '\0';
	return hostname;
}


#ifdef DOMAIN_NAME
/*
 * find domainname - check DOMAIN_NAME
 * TODO: check /etc/defaultdomain as a last resort?
 *       there is M_UNIX specific code in here, but no VMS
 *       alternative
 */
const char *
get_domain_name(
	void)
{
	FILE *fp;
	char *ptr;
	char buff[MAXHOSTNAMELEN + 1];
	static char domain[8192];

	domain[0] = '\0';

	if (strlen(DOMAIN_NAME))
		strcpy(domain, DOMAIN_NAME);

	if (domain[0] == '/' && domain[1]) {
		/* read domainname from specified file */
		if ((fp = fopen(domain, "r")) != NULL) {
			while (fgets(buff, (int) sizeof(buff), fp) != NULL) {
				if (buff[0] == '#' || buff[0] == '\n')
					continue;

				if ((ptr = strrchr(buff, '\n'))) {
					*ptr = '\0';
					strcpy(domain, buff);
				}
			}
			if (domain[0] == '/')	/* '/' is not allowed in domainames -> file was empty */
				domain[0] = '\0';

			fclose(fp);
		} else
			domain[0] = '\0';
	}
	domain[MAXHOSTNAMELEN + 1] = '\0';
	return domain;
}
#endif /* DOMAIN_NAME */


#ifdef HAVE_GETHOSTBYNAME
#	define MAXLINELEN   1024
#	define WS	" \f\t\v"
/* find FQDN - gethostbyaddr() */
const char *
get_fqdn(
	const char *host)
{
	char *domain;
	char line[MAXLINELEN + 1];
	char name[MAXHOSTNAMELEN + 1];
	static char fqdn[1024];
	struct hostent *hp;
	struct in_addr in;

	*fqdn = '\0';
	domain = NULL;
	name[MAXHOSTNAMELEN] = '\0';

	if (host) {
		if (strchr(host, '.'))
			return host;
		strncpy(name, host, sizeof(name) - 1);
	} else {
#	ifdef HAVE_GETHOSTNAME
		if (gethostname(name, sizeof(name) - 1))
#	endif /* HAVE_GETHOSTNAME */
			return NULL;
	}

#	ifdef HAVE_INET_ADDR
	if ('0' <= *name && *name <= '9') {
		in.s_addr = inet_addr(name);
		if ((hp = gethostbyaddr((char *) &in.s_addr, 4, AF_INET)))
			in.s_addr = (*hp->h_addr);
		return (hp && strchr(hp->h_name, '.') ? hp->h_name : inet_ntoa(in));
	}
#	endif /* HAVE_INET_ADDR */
	if ((hp = gethostbyname(name)) && !strchr(hp->h_name, '.'))
		if ((hp = gethostbyaddr(hp->h_addr, hp->h_length, hp->h_addrtype)))
			in.s_addr = (*hp->h_addr);

	snprintf(fqdn, sizeof(fqdn), "%s", hp
		? strchr(hp->h_name, '.')
			? hp->h_name : inet_ntoa(in)
		: "");

	if (!*fqdn || (fqdn[strlen(fqdn) - 1] <= '9')) {
		FILE *inf;

		*fqdn = '\0';

		if ((inf = fopen("/etc/resolv.conf", "r")) != NULL) {
			char *eos;
			int j;

			while (fgets(line, MAXLINELEN, inf)) {
				if (line[0] == '#' || line[0] == '\n')
					continue;

				line[MAXLINELEN] = '\0';

				if ((eos = strpbrk(line, WS)) != NULL) {
					if ((j = eos - line)) {
						if (!strncmp(line, "domain", j) || !strncmp(line, "search", j)) {
							domain = strtok(eos, WS);
							break;
						}
					}
				}
			}
			if (domain)
				snprintf(fqdn, sizeof(fqdn), "%s.%s", name, strip_line(domain));

			fclose(inf);
		}
	}
	return fqdn;
}
#endif /* HAVE_GETHOSTBYNAME */


/*
 * Find username & fullname
 */
void
get_user_info(
	char *user_name,
	char *full_name)
{
	const char *ptr;

	user_name[0] = '\0';
	full_name[0] = '\0';

	if ((ptr = get_full_name()))
		strcpy(full_name, ptr);
	if ((ptr = get_user_name()))
		strcpy(user_name, ptr);
}


static const char *
get_user_name(
	void)
{
	static char username[128];
	struct passwd *pw;
#ifdef VMS
	char *p;
#endif /* VMS */

	username[0] = '\0';

#ifndef VMS
	if ((pw = getpwuid(getuid())) != NULL)
		strcpy(username, pw->pw_name);
	else
#else
	if (((p = getlogin()) != NULL) && strlen(p)) {
		if ((pw = getpwnam(p)) != NULL)
			strcpy(username, pw->pw_name);
	}
#endif /* !VMS */

	if (!*username) {
		error_message(_(txt_error_passwd_missing));
		tin_done(EXIT_FAILURE);
	}

#	ifdef VMS
	lower(username);
#	endif /* VMS */

	return username;
}


static const char *
get_full_name(
	void)
{
	char *p;
	static char fullname[128];
#	if !defined(VMS) && !defined(DONT_HAVE_PW_GECOS)
	char buf[128];
	char tmp[128];
	struct passwd *pw;
#	endif /* !VMS && !DONT_HAVE_PW_GECOS */

	fullname[0] = '\0';

	if ((p = getenv("NAME")) != NULL) {
		my_strncpy(fullname, p, sizeof(fullname) - 1);
		return fullname;
	}
	if ((p = getenv("REALNAME")) != NULL) {
		my_strncpy(fullname, p, sizeof(fullname) - 1);
		return fullname;
	}

#	ifdef VMS
	STRCPY(fullname, fix_fullname(get_uaf_fullname()));
#	else
#		ifndef DONT_HAVE_PW_GECOS
	if ((pw = getpwuid(getuid())) != NULL) {
		STRCPY(buf, pw->pw_gecos);
		if ((p = strchr(buf, ',')))
			*p = '\0';
		if ((p = strchr(buf, '&'))) {
			*p++ = '\0';
			STRCPY(tmp, pw->pw_name);
			if (*tmp && isalpha((int)(unsigned char) *tmp) && islower((int)(unsigned char) *tmp))
				*tmp = toupper((int)(unsigned char) *tmp);
			snprintf(fullname, sizeof(fullname), "%s%s%s", buf, tmp, p);
		} else
			STRCPY(fullname, buf);
	}
#		endif /* !DONT_HAVE_PW_GECOS */
#	endif /* VMS */
	return fullname;
}


/*
 * FIXME to:
 * char *build_from(full_name, user_name, domain_name)
 */
/*
 * build From: in 'name <user@host.doma.in>' format
 */
void
get_from_name(
	char *from_name,
	struct t_group *thisgrp)
{
#	ifdef USE_INN_NNTPLIB
	char *fromhost = GetConfigValue(_CONF_FROMHOST);
#	else
	char *fromhost = NULL;
#	endif /* USE_INN_NNTPLIB */

	if (!(fromhost && *fromhost))
		fromhost = domain_name;

	if (thisgrp && *thisgrp->attribute->from != '\0') {
		strcpy(from_name, thisgrp->attribute->from);
		return;
	}

	sprintf(from_name, ((strpbrk(get_full_name(), "!()<>@,;:\\\".[]")) ? "\"%s\" <%s@%s>" : "%s <%s@%s>"), get_full_name(), get_user_name(), fromhost);

#	ifdef DEBUG
	if (debug == 2)
		error_message("FROM=[%s] USER=[%s] HOST=[%s] NAME=[%s]", from_name, get_user_name(), domain_name, get_full_name());
#	endif /* DEBUG */

}


/*
 * build_sender()
 * returns *(Full_Name <user@fq.domainna.me>)
 */
#ifndef FORGERY
char *
build_sender(
	void)
{
	const char *ptr;
	static char sender[8192];

	sender[0] = '\0';

	if ((ptr = get_full_name()))
		snprintf(sender, sizeof(sender), ((strpbrk(ptr, "\".:;<>@[]()\\")) ? "\"%s\"" : "%s "), ptr);
	if ((ptr = get_user_name())) {
		strcat(sender, "<");
		strcat(sender, ptr);
		strcat(sender, "@");

#	ifdef HAVE_GETHOSTBYNAME
		if ((ptr = get_fqdn(get_host_name())))
#	else
		if ((ptr = get_host_name()))
#	endif /* HAVE_GETHOSTBYNAME */
		{
			strcat(sender, ptr);
			strcat(sender, ">");
		} else
			return (char *) 0;
	} else
		return (char *) 0;

	return sender;
}
#endif /* !FORGERY */
