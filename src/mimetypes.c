/*
 *  Project   : tin - a Usenet reader
 *  Module    : mimetypes.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-31
 *  Updated   : 2000-06-05
 *  Notes     : mime.types handling
 *
 * Copyright (c) 2000-2002 Jason Faultless <jason@radar.tele2.co.uk>
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
	char *exts;
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
