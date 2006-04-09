/*
 *  Project   : tin - a Usenet reader
 *  Module    : regex.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1997-02-21
 *  Updated   : 2003-12-17
 *  Notes     : Regular expression subroutines
 *  Credits   :
 *
 * Copyright (c) 1997-2006 Jason Faultless <jason@altarstone.com>
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */

/*
 * See if pattern is matched in string. Return TRUE or FALSE
 * if icase=TRUE then ignore case in the compare
 * if a precompiled regex is provided it will be used instead of pattern
 *
 * If you use match_regex() with full regexes within a loop you should always
 * provide a precompiled error because if the compilation of the regex fails
 * an error message will be display on each execution of match_regex()
 */
t_bool
match_regex(
	const char *string,
	char *pattern,
	struct regex_cache *cache,
	t_bool icase)
{
	int error;
	struct regex_cache tmp_cache = { NULL, NULL };
	struct regex_cache *ptr_cache;

	if (!tinrc.wildcard)	/* wildmat matching */
		return wildmat(string, pattern, icase);

	/* full regexes */
	if (cache != NULL && cache->re != NULL)
		ptr_cache = cache;	/* use the provided regex cache */
	else {
		/* compile the regex internally */
		if (!compile_regex(pattern, &tmp_cache, (icase ? PCRE_CASELESS : 0)))
			return FALSE;
		ptr_cache = &tmp_cache;
	}

	if ((error = pcre_exec(ptr_cache->re, ptr_cache->extra, string, strlen(string), 0, 0, NULL, 0)) >= 0) {
		FreeIfNeeded(tmp_cache.re);
		FreeIfNeeded(tmp_cache.extra);

		return TRUE;
	}

#if 0
	/*
	 * match_regex() is mostly used within loops and we don't want to display
	 * an error message on each call
	 */
	if (error != PCRE_ERROR_NOMATCH)
		error_message(_(txt_pcre_error_num), error);
#endif /* 0 */

	FreeIfNeeded(tmp_cache.re);
	FreeIfNeeded(tmp_cache.extra);

	return FALSE;
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
	int regex_errpos, my_options = options;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE) && (defined(PCRE_MAJOR) && PCRE_MAJOR >= 4)
	if (IS_LOCAL_CHARSET("UTF-8")) {
		int i;

		pcre_config(PCRE_CONFIG_UTF8, &i);
		if (i)
			my_options |= PCRE_UTF8;
	}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE && PCRE_MAJOR && PCRE_MAJOR >= 4*/

	if ((cache->re = pcre_compile(regex, my_options, &regex_errmsg, &regex_errpos, NULL)) == NULL)
		error_message(_(txt_pcre_error_at), regex_errmsg, regex_errpos, regex);
	else {
		cache->extra = pcre_study(cache->re, 0, &regex_errmsg);
		if (regex_errmsg != NULL) {
			/* we failed, clean up */
			FreeAndNull(cache->re);
			error_message(_(txt_pcre_error_text), regex_errmsg);
		} else
			return TRUE;
	}
	return FALSE;
}


/*
 * Highlight any string on 'row' that match 'regex'
 */
void
highlight_regexes(
	int row,
	struct regex_cache *regex,
	int color)
{
	char *ptr;
	int offsets[6];
	int offsets_size = ARRAY_SIZE(offsets);
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

	while (pcre_exec(regex->re, regex->extra, ptr, strlen(ptr), 0, 0, offsets, offsets_size) > 0) {
		/* we have a match */
		if (color >= 0) /* color the matching text */
			word_highlight_string(row, (ptr - buf) + offsets[0], offsets[1] - offsets[0], color);
		else
			/* inverse the matching text */
			highlight_string(row, (ptr - buf) + offsets[0], offsets[1] - offsets[0]);

		if (!tinrc.word_h_display_marks) {
#ifdef USE_CURSES
			screen_contents(row, 0, buf);
#endif /* USE_CURSES */
			ptr += offsets[1] - 2;
		} else
			ptr += offsets[1];
	}
}
