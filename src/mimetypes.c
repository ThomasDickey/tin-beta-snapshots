/*
 *  Project   : tin - a Usenet reader
 *  Module    : mimetypes.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-31
 *  Updated   : 2024-05-05
 *  Notes     : mime.types handling
 *
 * Copyright (c) 2000-2025 Jason Faultless <jason@altarstone.com>
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


/*
 * local prototypes
 */
static t_bool _lookup_mimetype(const char *file, const char *ext, t_part *part);
static t_bool _lookup_extension(char *extension, size_t ext_len, const char *file, const char *type);


/*
 * Match a filename extension to a content-type / subtype pair in mime.types
 * Update the passed-in attachment structure with type/subtype if found
 * Return TRUE if found
 */
static t_bool
_lookup_mimetype(
	const char *file,
	const char *ext,
	t_part *part)
{
	FILE *fp;
	const char *exts;
	char *ptr;
	char buf[PATH_LEN];
	int i;

	if ((fp = tin_fopen(file, "r")) == NULL)
		return FALSE;

	while ((fgets(buf, sizeof(buf), fp)) != NULL) {
		if (buf[0] == '#' || buf[0] == '\n')		/* Skip comments & blank lines */
			continue;

		if (strtok(buf, " \t\n")) {
			while ((exts = strtok(NULL, " \t\n")) != NULL) {	/* Work through list of extensions */
				if (strcasecmp(ext, exts) == 0) {
					if ((i = content_type(strtok(buf, "/"))) != -1) {
						if ((ptr = strtok(NULL, "\n")) != NULL) {
							part->type = (unsigned int) i;
							FreeIfNeeded(part->subtype);
							part->subtype = my_strdup(ptr);
							fclose(fp);
							return TRUE;
						}
					}
				}
			}
		}
	}

	fclose(fp);
	return FALSE;
}


/*
 * Check:
 *	$HOME/.mime.types
 *	/etc/mime.types
 *	TIN_DEFAULTS_DIR/mime.types
 */
void
lookup_mimetype(
	const char *ext,
	t_part *part)
{
	char buf[PATH_LEN];

	joinpath(buf, sizeof(buf), homedir, ".mime.types");
	if (_lookup_mimetype(buf, ext, part))
		return;

	if (_lookup_mimetype("/etc/mime.types", ext, part))
		return;

#ifdef TIN_DEFAULTS_DIR
	joinpath(buf, sizeof(buf), TIN_DEFAULTS_DIR, "mime.types");
	_lookup_mimetype(buf, ext, part);
#endif /* TIN_DEFAULTS_DIR */
}


/*
 * look for a filename extension in file for the specified
 * type ("major/minor"), the result is stored in extension
 */
static t_bool
_lookup_extension(
	char *extension,
	size_t ext_len,
	const char *file,
	const char *type)
{
	FILE *fp;
	char *p;
	char buf[8192];

	if ((fp = tin_fopen(file, "r")) == NULL)
		return FALSE;

	while ((fgets(buf, sizeof(buf), fp)) != NULL) {
		if (buf[0] == '#' || buf[0] == '\n' || strncmp(buf, type, strlen(type)))
			continue;
		if ((p = strtok(buf, " \t\n"))) {
			if (strlen(p) != strlen(type))
				continue;
			if ((p = strtok(NULL, " \t\n")) != NULL) {
				my_strncpy(extension, p, ext_len - 1);
				fclose(fp);
				return TRUE;
			}
		}
	}
	fclose(fp);
	return FALSE;
}


/*
 * look for a filename extension for the specified
 * major minor, the result is stored in extension
 * files checked are:
 *	$HOME/.mime.types
 *	/etc/mime.types
 *	TIN_DEFAULTS_DIR/mime.types
 */
t_bool
lookup_extension(
	char *extension,
	size_t ext_len,
	const char *major,
	const char *minor)
{
	char *type;
	char buf[PATH_LEN];
	int n;
	size_t tlen;

	if (!major || !minor) {
		*extension = '\0';
		return FALSE;
	}

	if ((n = snprintf(NULL, 0, "%s/%s", major, minor)) < 0) {
		*extension = '\0';
		return FALSE;
	}
	tlen = (size_t) n + 1;
	type = my_malloc(tlen);
	if (snprintf(type, tlen, "%s/%s", major, minor) != n) {
		*extension = '\0';
		free(type);
		return FALSE;
	}

	joinpath(buf, sizeof(buf), homedir, ".mime.types");
	if (_lookup_extension(extension, ext_len, buf, type)) {
		free(type);
		return TRUE;
	}

	if (_lookup_extension(extension, ext_len, "/etc/mime.types", type)) {
		free(type);
		return TRUE;
	}

#ifdef TIN_DEFAULTS_DIR
	joinpath(buf, sizeof(buf), TIN_DEFAULTS_DIR, "mime.types");
	if (_lookup_extension(extension, ext_len, buf, type)) {
		free(type);
		return TRUE;
	}
#endif /* TIN_DEFAULTS_DIR */
	free(type);
	my_strncpy(extension, minor, ext_len - 1);
	return FALSE;
}
