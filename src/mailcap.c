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
 *    This product includes software developed by Jason Faultless
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
#	ifndef TCURSES_H
#		include "tcurses.h"
#	endif /* !TCURSES_H */
#	ifndef MENUKEYS_H
#		include  "menukeys.h"
#	endif /* !MENUKEYS_H */
#	ifndef RFC2046_H
#		include  "rfc2046.h"
#	endif /* !RFC2046_H */
#endif /* 0 */

#define DEFAULT_MAILCAPS "~/.mailcap:/etc/mailcap:/usr/etc/mailcap:/usr/local/etc/mailcap"

#if 0
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
	int i;

	strtok (line, ";");
	command = strtok (NULL, "\n");		/* Get the command info */

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
	FILE *fp = (FILE *) 0;
	char buf[LEN];
	char mailcap[LEN];
	char mailcaps[LEN];
	char *command;
	char *ptr;
	t_bool wild;

	/*
	 * lookup all files given in $MAILCAPS, if $MAILCAPS is unset
	 * use DEFAULT_MAILCAPS as fallback serachpath
	 *
	 * TODO: append DEFAULT_MAILCAPS to $MAILCAPS?
	 */
	ptr = get_val("MAILCAPS", DEFAULT_MAILCAPS);
	strncpy(mailcaps, ptr, sizeof(mailcaps) - 1);
	ptr = strtok(mailcaps, ":");
	while (ptr != (char *) 0) {
		/* expand ~ and/or $HOME etc. */
		strfpath (ptr, mailcap, sizeof (mailcap), homedir, (char *) 0, (char *) 0, (char *) 0);
		if ((fp = fopen(mailcap, "r")) != (FILE *) 0)
			break;
		ptr = strtok(NULL, ":");
	}
	if (!fp)	/* no mailcap file */
		return NULL;

	mailcap[0] = '\0';
	ptr = buf;

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
	fprintf(stderr, "Best match: (%s) char 0 %d\n", mailcap, mailcap[0]);
#endif /* DEBUG */
	fclose (fp);

	if (mailcap[0] != '\0')
		return my_strdup(mailcap);
	else
		return NULL;
}
#endif /* 0 */

/*
 * Match a filename extension to a content-type / subtype pair in mime.types
 * Update the passed-in attachment structure with type/subtype if found
 */
void
lookup_mimetype (
	const char *ext,
	t_part *part)
{
	FILE *fp = (FILE *) 0;
	char buf[LEN];

	/*
	 * check $HOME/.mime.types first, then /etc/mime.types and
	 * TIN_DEFAULTS_DIR/mime.types
	 */
	snprintf(buf, sizeof(buf), "%s/.mime.types", homedir);
	fp = fopen (buf, "r");
	if (!fp)
		fp = fopen("/etc/mime.types", "r");
#ifdef TIN_DEFAULTS_DIR
	if (!fp) {
		snprintf(buf, sizeof(buf), "%s/mime.types", TIN_DEFAULTS_DIR);
		fp = fopen(buf, "r");
	}
#endif /* TIN_DEFAULTS_DIR */

	if (!fp)
		return;

	while ((fgets (buf, sizeof(buf), fp)) != NULL) {
		char *exts;

		if (buf[0] == '#' || buf[0] == '\n')		/* Skip comments & blank lines */
			continue;

		strtok(buf, " \t\n");

		while ((exts = strtok (NULL, " \t\n")) != NULL) {	/* Work through list of extensions */
			if (strcasecmp (ext, exts) == 0) {
				int i;

				if ((i = content_type (strtok(buf, "/"))) != -1) {
					char *ptr;

					if ((ptr = strtok(NULL, "\n")) != NULL) {
						part->type = i;
						part->subtype = my_strdup(ptr);

						fclose (fp);
						return;
					}
				}
			}
		}
	}

	fclose (fp);
	return;
}
