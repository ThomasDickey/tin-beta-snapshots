/*
 *  Project   : tin - a Usenet reader
 *  Module    : regex.c
 *  Author    : Jason Faultless <jason@radar.tele2.co.uk>
 *  Created   : 1997-02-21
 *  Updated   : 1997-02-25
 *  Notes     : Regular expression subroutines
 *  Credits   :
 *
 * Copyright (c) 1997-2002 Jason Faultless <jason@radar.tele2.co.uk>
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
 *    This product includes software developed by Jason Faultless.
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */

/*
 * See if pattern is matched in string. Return TRUE or FALSE
 * if icase=TRUE then ignore case in the compare
 */
t_bool
match_regex (
	const char *string,
	char *pattern,
	t_bool icase)
{
	const char *errmsg;
	int flags = PCRE_EXTENDED;
	int error;
	pcre *re;
	t_bool ret = FALSE;

	mesg[0] = '\0';

	if (icase)
		flags |= PCRE_CASELESS;

	/*
	 * Compile the expression internally.
	 */
	if ((re = pcre_compile(pattern, flags, &errmsg, &error, NULL)) == NULL) {
		sprintf(mesg, _(txt_pcre_error_at), errmsg, error);
		return FALSE;
	}

	/*
	 * Since we are running the the compare only once,
	 * we don't need to use pcre_study() to improve
	 * performance
	 */

	/*
	 * Only a single compare is needed to see if a match exists
	 *
	 * pcre_exec(precompile pattern, hints pointer, string to match,
	 *           length of string (string may contain '\0', but not in
	 *           out case), startoffset, options,
	 *           vector of offsets to be filled,
	 *           number of elements in offsets);
	 *
	 */
	error = pcre_exec(re, NULL, string, strlen(string), 0, 0, NULL, 0);
	if (error >= 0)
		ret = TRUE;
	else if (error == -1)
		ret = FALSE;
	else
		sprintf(mesg, _(txt_pcre_error_num), error);

	free(re);
	return ret;
}


/*
 * Compile and optimise 'regex'. Return TRUE if all went well
 */
t_bool
compile_regex(
	const char *regex,
	struct regex_cache *cache,
	int options)
{
	const char *regex_errmsg = 0;
	int regex_errpos;

	if ((cache->re = pcre_compile (regex, PCRE_EXTENDED | options, &regex_errmsg, &regex_errpos, NULL)) == NULL)
		error_message (_(txt_pcre_error_at), regex_errmsg, regex_errpos);
	else {
		cache->extra = pcre_study (cache->re, 0, &regex_errmsg);
		if (regex_errmsg != NULL)
			error_message (_(txt_pcre_error_text), regex_errmsg);
		else
			return TRUE;
	}
	return FALSE;
}


/*
 * Highlight (in inverse text) any string on 'row' that match 'regex'
 */
void
highlight_regexes(
	int row,
	struct regex_cache *regex)
{
	char *ptr;
	int offsets[6];
	int offsets_size = sizeof(offsets)/sizeof(int);
#ifdef USE_CURSES
	char buf[LEN];
#else
	char *buf;
#endif /* USE_CURSES */

	/* Get contents of line from the screen */
#ifdef USE_CURSES
	screen_contents(row, 0, buf);
#else
	buf = screen[row].col;
#endif /* USE_CURSES */
	ptr = buf;

	while (pcre_exec (regex->re, regex->extra, ptr, strlen(ptr), 0, 0, offsets, offsets_size) != PCRE_ERROR_NOMATCH) {
		highlight_string (row, (ptr - buf) + offsets[0], offsets[1] - offsets[0]);
		ptr += offsets[1];
	}
}
