/*
 *  Project   : tin - a Usenet reader
 *  Module    : mailcap.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-31
 *  Updated   :
 *  Notes     : mailcap and mime.types handling per RFC1524
 *
 * Copyright (c) 2000 Jason Faultless <jason@radar.tele2.co.uk>
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
#if 0
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef MENUKEYS_H
#	include  "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef RFC2045_H
#	include  "rfc2045.h"
#endif /* !RFC2045_H */
#endif

#define DEFAULT_MAILCAP		"/etc/mailcap"


/*
 * Parse a mailcap line for matching type and subtype
 * Return 'body' of entry if match or NULL
 * If the match was on a wildcard entry, set wild
 */
static char *
parse_line(
	char *line,
	int type,
	const char *subtype,
	t_bool *wild)
{
	char *command;
	char *ptr;
	int i;
	t_bool quote = FALSE;
	t_bool backquote = FALSE;

	strtok (line, ";");
	command = strtok (NULL, "\n");		/* Get the command info */

	/*
	 * Split off the command name - don't split on backquoted ; or
	 * ; in " "
	 */
	for (ptr = command; *ptr; ptr++) {
		if (*ptr == '\"')
			quote = !quote;
		else if (*ptr == '\\')
			backquote = TRUE;
		else if (*ptr == ';') {
			if (!(quote || backquote))
				break;
		} else if (backquote)
			backquote = FALSE;
	}
	*ptr = '\0';

	strtok (line, "/");					/* Split the type+subtype */

	if ((i = content_type (line)) == -1)
		return NULL;					/* Not a known content-type */

	if (i != type)						/* Not the type we're looking for */
		return NULL;

	/*
	 * Match on implied wild, or wild, or exact match
	 */
	if (((line = strtok(NULL, "\n")) == NULL) || (strcmp(line, "*") == 0)) {
		*wild = TRUE;
		return command;
	}

	if (strcasecmp (line, subtype) == 0) {
		*wild = FALSE;
		return command;
	}

	return NULL;
}


/*
 * Parse the mailcap file to find a matching MIME type/subtype
 * The returned string in on the heap and should be free()d
 */
char *
lookup_mailcap(
	int type,
	const char *subtype)
{
	FILE *fp;
	char buf[LEN];
	char mailcap[LEN];
	char mailcap_file[PATH_LEN];
	char *command;
	char *ptr = buf;
	t_bool wild;

	strcpy (mailcap_file, get_val ("MAILCAP", DEFAULT_MAILCAP));	

	if ((fp = fopen(mailcap_file, "r")) == NULL)
		return NULL;

	mailcap[0] = '\0';

	while ((fgets (ptr, sizeof(buf) - strlen(buf), fp)) != NULL) {

		if (*ptr == '#' || *ptr == '\n')		/* Skip comments & blank lines */
			continue;

		ptr = buf + strlen(buf) - 1;

		if (*ptr == '\n')
			*ptr-- = '\0';

		if (*ptr == '\\')						/* Continuation line */
			continue;							/* So keep appending */
		else
			ptr = buf;

		if ((command = parse_line (ptr, type, subtype, &wild)) != NULL) {
			strcpy (mailcap, command);
			if (!wild)							/* Perfect match, exit now */
				break;
		}
	}
#ifdef DEBUG
	if (debug == 2)
		fprintf(stderr, "Best match: (%s) char 0 %d\n", mailcap, mailcap[0]);
#endif /* DEBUG */
	fclose (fp);

	if (mailcap[0] != '\0')
		return my_strdup(mailcap);
	else
		return NULL;
}


/*
 * Match a filename extension to a content-type / subtype pair in mime.types
 * Use this as input to lookup_mailcap() and return an appropriate command
 */
char *
lookup_mimetype (
	const char *ext)
{
	FILE *fp;
	char buf[LEN];
	char *exts;

	if ((fp = fopen("/etc/mime.types", "r")) == NULL)
		return NULL;

	while ((fgets (buf, sizeof(buf), fp)) != NULL) {

		if (buf[0] == '#' || buf[0] == '\n')		/* Skip comments & blank lines */
			continue;

		strtok(buf, " \t\n");
		while ((exts = strtok (NULL, " \t\n")) != NULL) {	/* Work through list of extensions */
			if (strcasecmp (ext, exts) == 0) {
				int i;

				if ((i = content_type (strtok(buf, "/"))) != -1) {
					char *ptr;
#ifdef DEBUG
					if (debug == 2)
						fprintf(stderr, "Matched type %d\n", i);
#endif /* DEBUG */
					if ((ptr = lookup_mailcap(i, strtok(NULL, "\n"))) != NULL) {
#ifdef DEBUG
						if (debug == 2)
							fprintf(stderr, "looked up %s\n", ptr);
#endif /* DEBUG */
						fclose (fp);
						return ptr;
					}
				}
			}
		}
	}

	fclose (fp);

	return NULL;
}
