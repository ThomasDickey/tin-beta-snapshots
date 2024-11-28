/*
 *  Project   : tin - a Usenet reader
 *  Module    : header.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 1997-03-10
 *  Updated   : 2024-11-26
 *
 * Copyright (c) 1997-2025 Urs Janssen <urs@tin.org>
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
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */

static const char *get_full_name(void);


/* find hostname */
const char *
get_host_name(
	void)
{
	const char *ptr;
	static char hostname[MAXHOSTNAMELEN + 1]; /* need space for '\0' */

	hostname[0] = '\0';

#ifdef HAVE_GETHOSTNAME
	gethostname(hostname, sizeof(hostname) - 1);
#endif /* HAVE_GETHOSTNAME */
#ifdef HAVE_SYS_UTSNAME_H
	if (!*hostname)
		my_strncpy(hostname, system_info.nodename, sizeof(hostname) - 1);
#endif /* HAVE_SYS_UTSNAME_H */
	if (!*hostname) {
		ptr = get_val("HOST", get_val("HOSTNAME", ""));
		if (*ptr)
			my_strncpy(hostname, ptr, sizeof(hostname) - 1);
		else
			hostname[0] = '\0';
	}
	hostname[MAXHOSTNAMELEN] = '\0';
	return hostname;
}


#ifdef DOMAIN_NAME
/*
 * find domainname - check DOMAIN_NAME
 * TODO: check /etc/defaultdomain as a last resort?
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
		if ((fp = tin_fopen(domain, "r")) != NULL) {
			while (fgets(buff, (int) sizeof(buff), fp) != NULL) {
				if (buff[0] == '#' || buff[0] == '\n')
					continue;

				if ((ptr = strrchr(buff, '\n'))) {
					*ptr = '\0';
					strcpy(domain, buff);
				}
			}
			fclose(fp);
			str_trim(domain);
			if (domain[0] == '/')	/* '/' is not allowed in domainames -> file was empty */
				domain[0] = '\0';
		} else
			domain[0] = '\0';
	}
	domain[MAXHOSTNAMELEN + 1] = '\0';
	return domain;
}
#endif /* DOMAIN_NAME */


#ifdef HAVE_GETHOSTBYNAME
#	define MAXLINELEN	1024
#	define WSP	" \f\t\v"
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
	struct in_addr in = { 0 };

	*fqdn = '\0';
	domain = NULL;
	name[MAXHOSTNAMELEN] = '\0';

	if (host) {
		if (strchr(host, '.'))
			return host;
		my_strncpy(name, host, sizeof(name) - 1);
	} else {
#	ifdef HAVE_GETHOSTNAME
		if (gethostname(name, sizeof(name) - 1))
#	endif /* HAVE_GETHOSTNAME */
			return NULL;
	}

	/* FIXME: this does not do IPv6 only */
#	ifdef HAVE_INET_ADDR
	if (*name >= '0' && *name <= '9') {
		in_addr_t addr = inet_addr(name);

		if ((hp = gethostbyaddr((char *) &addr, 4, AF_INET))) {
#		ifdef HAVE_HOSTENT_H_ADDR_LIST
			in.s_addr = (*hp->h_addr_list[0]);
#		else
			in.s_addr = (*hp->h_addr);
#		endif /* HAVE_HOSTENT_H_ADDR_LIST */
		}
		return (
			hp && strchr(hp->h_name, '.') ? hp->h_name :
#		ifdef HAVE_INET_NTOA
			inet_ntoa(in)
#		else
			""
#		endif /* HAVE_INET_NTOA */
			);
	}
#	endif /* HAVE_INET_ADDR */

	if ((hp = gethostbyname(name)) && !strchr(hp->h_name, '.')) {
#	ifdef HAVE_HOSTENT_H_ADDR_LIST
		if ((hp = gethostbyaddr(hp->h_addr_list[0], hp->h_length, hp->h_addrtype)))
			in.s_addr = (*hp->h_addr_list[0]);
#	else
		if ((hp = gethostbyaddr(hp->h_addr, hp->h_length, hp->h_addrtype)))
			in.s_addr = (*hp->h_addr);
#	endif /* HAVE_HOSTENT_H_ADDR_LIST */
	}
	snprintf(fqdn, sizeof(fqdn), "%s", hp
		? strchr(hp->h_name, '.')
			? hp->h_name :
#	ifdef HAVE_INET_NTOA
			inet_ntoa(in)
#	else
			""
#	endif /* HAVE_INET_NTOA */
		: "");

	if (!*fqdn || (fqdn[strlen(fqdn) - 1] <= '9')) { /* see FIXME above about IPv6 */
		FILE *inf;

		*fqdn = '\0';

		if ((inf = tin_fopen("/etc/resolv.conf", "r")) != NULL) {
			char *eos;
			int j;

			while (fgets(line, MAXLINELEN, inf)) {
				if (line[0] == '#' || line[0] == '\n')
					continue;

				line[MAXLINELEN] = '\0';

				if ((eos = strpbrk(line, WSP)) != NULL) {
					if ((j = (int) (eos - line))) {
						if (!strncmp(line, "domain", (size_t) j) || !strncmp(line, "search", (size_t) j)) {
							domain = strtok(eos, WSP);
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
	return *fqdn ? fqdn : get_val("HOST", get_val("HOSTNAME", ""));
}
#endif /* HAVE_GETHOSTBYNAME */


static const char *
get_full_name(
	void)
{
	const char *q;
	char *p;
	static char fullname[128];
#ifndef DONT_HAVE_PW_GECOS
	char buf[128];
	char tmp[128];
	struct passwd *pw;
#endif /* !DONT_HAVE_PW_GECOS */

	fullname[0] = '\0';

	/*
	 * ignore set but empty env-var.
	 * give mail_address in tinrc without a name instead if you
	 * want this behaviour.
	 */
	if ((q = get_val("NAME", get_val("REALNAME", NULL))) != NULL) {
		my_strncpy(fullname, q, sizeof(fullname) - 1);
		return fullname;
	}

#ifndef DONT_HAVE_PW_GECOS
	if ((pw = getpwuid(getuid())) != NULL) {
		STRCPY(buf, pw->pw_gecos);
		if ((p = strchr(buf, ',')))
			*p = '\0';
		if ((p = strchr(buf, '&'))) {
			int ret;

			*p++ = '\0';
			STRCPY(tmp, pw->pw_name);
			if (*tmp && isalpha((unsigned char) *tmp) && islower((unsigned char) *tmp))
				*tmp = (char) my_toupper((unsigned char) *tmp);
			ret = snprintf(fullname, sizeof(fullname), "%s%s%s", buf, tmp, p);
			if (ret == -1 || ret > (int) sizeof(fullname))
				error_message(2, "Fullname truncated"); /* -> lang.c */
		} else
			STRCPY(fullname, buf);
	}
#endif /* !DONT_HAVE_PW_GECOS */
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
	const char *fromhost = domain_name;

	if (thisgrp && thisgrp->attribute->from && *thisgrp->attribute->from) {
		strcpy(from_name, *thisgrp->attribute->from);
		return;
	}

	sprintf(from_name, ((strpbrk(get_full_name(), "!()<>@,;:\\\".[]")) ? "\"%s\" <%s@%s>" : "%s <%s@%s>"), BlankIfNull(get_full_name()), userid, BlankIfNull(fromhost));

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		error_message(2, "FROM=[%s] USER=[%s] HOST=[%s] NAME=[%s]", from_name, userid, domain_name, BlankIfNull(get_full_name()));
#endif /* DEBUG */
}


/*
 * build_sender()
 * returns *(Full_Name <user@fq.domainna.me>)
 */
#if !defined(FORGERY) && defined(NNTP_ABLE)
char *
build_sender(
	void)
{
	const char *ptr;
	static char sender[HEADER_LEN];

	sender[0] = '\0';

	if ((ptr = get_full_name())) /* TODO: rfc2047 encode */
		snprintf(sender, sizeof(sender), ((strpbrk(ptr, "\".:;<>@[]()\\")) ? "\"%s\"" : "%s "), ptr);

	snprintf(sender + strlen(sender), sizeof(sender) - strlen(sender), "<%.*s@", LOGIN_NAME_MAX, userid);

#	ifdef HAVE_GETHOSTBYNAME
	ptr = get_fqdn(get_host_name());
#	else
	ptr = get_host_name();
#	endif /* HAVE_GETHOSTBYNAME */

		/* intentionally do not fall back to *domain_name */

	if (*ptr)
		snprintf(sender + strlen(sender), sizeof(sender) - strlen(sender), "%s>", ptr);
	else
		return NULL;

	return sender;
}
#endif /* !FORGERY && NNTP_ABLE */
