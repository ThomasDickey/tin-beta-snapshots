/*
 *  Project   : tin - a Usenet reader
 *  Module    : version.c
 *  Author    : U. Janssen
 *  Created   : 2003-05-11
 *  Updated   : 2024-04-01
 *  Notes     :
 *
 * Copyright (c) 2003-2024 Urs Janssen <urs@tin.org>
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
 *              TODO: switch to rc_majorv << 16 + rc_minorv << 8 + rc_subv
 *                    (requires int_least32_t aka INT_WIDTH >= 32, but
 *                     I guess a lot of the code silently assumes that
 *                     anyway)
 *
 * usage:
 * if (upgrade == NULL && match_string(line, skip, NULL, 0)) {
 *     upgrade = check_upgrade(line, skip, version);
 *     upgrade_prompt_quit(upgrade, file);
 * }
 */
struct t_version *
check_upgrade(
	char *line,
	const char *skip,
	const char *version)
{
	const char *p;
	int rc_majorv, rc_minorv, rc_subv; /* version numbers in the file */
	int current_version, c_majorv, c_minorv, c_subv;	/* version numbers we require */
	struct t_version *fversion = my_malloc(sizeof(struct t_version));

	fversion->state = RC_ERROR;
	fversion->file_version = -1;

	/* we pre-checked that the beginning of line matches skip via
	   match_string() before calling check_upgrade(), no need to
	   strncmp() again.
	 */
	p = line + strlen(skip);
	if (!isdigit((unsigned char) *p))
		return fversion;
	rc_majorv = s2i(p, 0, 99);
	if (errno)
		return fversion;
	while (isdigit((unsigned char) *p))
		p++;
	if (*p != '.' || !isdigit((unsigned char) *++p))
		return fversion;
	rc_minorv = s2i(p, 0, 99);
	if (errno)
		return fversion;
	while (isdigit((unsigned char) *p))
		p++;
	if (*p != '.' || !isdigit((unsigned char) *++p))
		return fversion;
	rc_subv = s2i(p, 0, 99);
	if (errno)
		return fversion;

	fversion->file_version = rc_majorv * 10000 + rc_minorv * 100 + rc_subv;

	p = version;
	c_majorv = s2i(p, 0, 99);
	if (errno)
		return fversion;
	while (isdigit((unsigned char) *p))
		p++;
	if (*p != '.')
		return fversion;

	c_minorv = s2i(++p, 0, 99);
	if (errno)
		return fversion;
	while (isdigit((unsigned char) *p))
		p++;
	if (*p != '.')
		return fversion;

	c_subv = s2i(++p, 0, 99);
	if (errno)
		return fversion;
	/* we don't care about trailing text */

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
	const char *file,
	FILE *fp)
{
	switch (upgrade->state) {
		case RC_UPGRADE:
			error_message(2, _(txt_warn_update), VERSION, file);
			break;

		case RC_DOWNGRADE:
			error_message(2, _(txt_warn_downgrade), VERSION, file);
			break;

		case RC_ERROR: /* can't parse internal version string, should not happen */
			error_message(2, txt_warn_unrecognized_version, file);
			break;

		default:	/* should no happen */
			return;
	}

	error_message(2, _(txt_return_key)); /* TODO: mention 'q'/'Q' */

	/*
	 * TODO: document, use something unbuffered here
	 * NOTE: these keys can not be remapped
	 */
	switch (getchar()) {
		case 'q':
		case 'Q':
		case ESC:
			free(upgrade);
			if (fp)
				fclose(fp);
			handle_cmdargs(FALSE);
			no_write = TRUE;
			tin_done(EXIT_FAILURE, _(txt_exiting));
			/* NOTREACHED */
			break;

		default:
			break;
	}
}
