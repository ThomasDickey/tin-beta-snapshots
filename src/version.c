/*
 *  Project   : tin - a Usenet reader
 *  Module    : version.c
 *  Author    : U. Janssen
 *  Created   : 2003-05-11
 *  Updated   : 2019-02-04
 *  Notes     :
 *
 * Copyright (c) 2003-2022 Urs Janssen <urs@tin.org>
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
 * line     is the entire line we should check
 * skip     must be the leading portion of the version line not including the
 *          version number (which must be a dotted triple)
 * version  is the version number (dotted triple) we expect to match
 *
 * returns struct t_version*
 *         ->state
 *              RC_IGNORE     1st args dotted triple matches 3rd arg
 *              RC_UPGRADE    1st args dotted triple is older than 3rd arg
 *              RC_DOWNGRADE  1st args dotted triple is newer than 3rd arg
 *              RC_ERROR      3rd arg is not a dotted triple (usage error)
 *         ->file_version     version number in file as int
 *              (rc_majorv * 10000 + rc_minorv * 100 + rc_subv) or -1
 *
 * Don't make the arguments to sscanf() consts, as some old systems require
 * them to writable (but do not change them)
 */
struct t_version *
check_upgrade(
	char *line,
	const char *skip,
	const char *version)
{
	char *format;
	char *lskip = my_strdup(skip);
	char *lversion = my_strdup(version);
	char fmt[10];
	int rc_majorv, rc_minorv, rc_subv; /* version numbers in the file */
	int current_version, c_majorv, c_minorv, c_subv;	/* version numbers we require */
	size_t len;
	struct t_version *fversion = my_malloc(sizeof(struct t_version));

	fversion->state = RC_ERROR;
	fversion->file_version = -1;

	rc_majorv = rc_minorv = rc_subv = c_majorv = c_minorv = c_subv = -1;
	strcpy(fmt, "%d.%d.%d"); /* we are expecting dotted triples */
	len = strlen(lskip) + strlen(fmt) + 1; /* format buffer len */
	format = my_malloc(len + 1);
	snprintf(format, len, "%s%s", lskip, fmt);
	free(lskip);

	if (sscanf(line, format, &rc_majorv, &rc_minorv, &rc_subv) != 3) {
		free(format);
		free(lversion);
		return fversion;
	}
	free(format);

	fversion->file_version = rc_majorv * 10000 + rc_minorv * 100 + rc_subv;

	/* we can't parse our own version number - should never happen */
	if (sscanf(lversion, fmt, &c_majorv, &c_minorv, &c_subv) != 3) {
		free(lversion);
		return fversion;
	}
	free(lversion);

	current_version = c_majorv * 10000 + c_minorv * 100 + c_subv;

	if (fversion->file_version == current_version)
		fversion->state = RC_IGNORE;

	if (fversion->file_version > current_version)
		fversion->state = RC_DOWNGRADE;

	if (fversion->file_version < current_version)
		fversion->state = RC_UPGRADE;

	return fversion;
}


void
upgrade_prompt_quit(
	struct t_version *upgrade,
	const char *file)
{
	switch (upgrade->state) {
		case RC_UPGRADE:
			error_message(2, _(txt_warn_update), VERSION, file);
			break;

		case RC_DOWNGRADE:
			error_message(2, _(txt_warn_downgrade), VERSION, file);
			break;

		case RC_ERROR: /* can't parse internal version string, should not happen */
			error_message(2, txt_warn_unrecognized_version);
			free(upgrade);
			free(tin_progname);
			giveup();
			/* NOTREACHED */
			break;

		default:	/* should no happen */
			return;
	}

	error_message(2, _(txt_return_key));

	/*
	 * TODO: document, use something unbuffered here
	 * NOTE: these keys can not be remapped
	 */
	switch (getchar()) {
		case 'q':
		case 'Q':
		case ESC:
			free(upgrade);
			free(tin_progname);
			giveup();
			/* NOTREACHED */
			break;

		default:
			break;
	}
}
