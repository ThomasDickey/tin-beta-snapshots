/*
 *  Project   : tin - a Usenet reader
 *  Module    : version.c
 *  Author    : U. Janssen
 *  Created   : 2003-05-11
 *  Updated   : 2005-07-02
 *  Notes     :
 *
 * Copyright (c) 2003-2006 Urs Janssen <urs@tin.org>
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
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */


/*
 * line     is the entire line we should check
 * skip     must be the leading portion of the version line not including the
 *          version number (which must be a dotted triple)
 * version  is the version number (dotted triple) we expect to match
 *
 * returns RC_IGNORE     1st args dotted triple matches 3rd arg
 *         RC_UPGRADE    1st args dotted triple is older than 3rd arg
 *         RC_DOWNGRADE  1st args dotted triple is newer than 3rd arg
 *         RC_ERROR      3rd arg is not a dotted triple (usage error)
 *
 * Don't make the arguments to sscanf() consts, as some old systems require
 * them to writable (but do not change them)
 */
int
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
	int c_majorv, c_minorv, c_subv;	/* version numbers we require */
	size_t len;

	rc_majorv = rc_minorv = rc_subv = c_majorv = c_minorv = c_subv = -1;
	strcpy(fmt, "%d.%d.%d"); /* we are expecting dotted triples */
	len = strlen(lskip) + strlen(fmt) + 1; /* format buffer len */
	format = my_malloc(len + 1);
	snprintf(format, len, "%s%s", lskip, fmt);
	sscanf(line, format, &rc_majorv, &rc_minorv, &rc_subv);
	free(format);
	free(lskip);

	/* we can't parse our own version number - should never happen */
	if (sscanf(lversion, fmt, &c_majorv, &c_minorv, &c_subv) != 3) {
		free(lversion);
		return RC_ERROR;
	}
	free(lversion);

	if (c_majorv == rc_majorv && c_minorv == rc_minorv && c_subv == rc_subv)
		return RC_IGNORE;

	if (rc_majorv > c_majorv ||
		(rc_majorv == c_majorv && rc_minorv > c_minorv) ||
		(rc_majorv == c_majorv && rc_minorv == c_minorv && rc_subv > c_subv))
		return RC_DOWNGRADE;

	return RC_UPGRADE;
}


void
upgrade_prompt_quit(
	int reason,
	const char *file)
{
	switch (reason) {
		case RC_UPGRADE:
			error_message(_(txt_warn_update), VERSION, file);
			break;

		case RC_DOWNGRADE:
			error_message(_(txt_warn_downgrade), VERSION, file);
			break;

		case RC_ERROR: /* can't parse internal version string, should not happen */
			error_message(txt_warn_unrecognized_version);
			giveup();
			/* NOTREACHED */
			break;

		default:	/* should no happen */
			return;
	}

	error_message(_(txt_return_key));

	/*
	 * TODO: document, use something unbuffered here
	 * NOTE: these keys can not be remapped
	 */
	switch (getchar()) {
		case 'q':
		case 'Q':
		case ESC:
			giveup();
			/* NOTREACHED */
			break;

		default:
			break;
	}
}
