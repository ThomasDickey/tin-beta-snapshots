/*
 *  Project   : tin - a Usenet reader
 *  Module    : regex.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1997-02-21
 *  Updated   : 2025-07-09
 *  Notes     : Regular expression subroutines
 *  Credits   :
 *
 * Copyright (c) 1997-2025 Jason Faultless <jason@altarstone.com>
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
	struct regex_cache tmp_cache = REGEX_CACHE_INITIALIZER;
	struct regex_cache *ptr_cache;

	if (!tinrc.wildcard)	/* wildmat matching */
		return wildmat(string, pattern, icase);

	/* full regexes */
	if (cache != NULL && cache->re != NULL)
		ptr_cache = cache;	/* use the provided regex cache */
	else {
		/* compile the regex internally */
		if (!compile_regex(pattern, &tmp_cache, (icase ? REGEX_CASELESS : 0)))
			return FALSE;

		ptr_cache = &tmp_cache;
	}

	error = match_regex_ex(string, (REGEX_SIZE) strlen(string), 0, 0, ptr_cache);
	if (error >= 0) {
		regex_cache_destroy(&tmp_cache);
		return TRUE;
	}

	/*
	 * match_regex() is mostly used within loops and we don't want to display
	 * an error message on each call
	 */
#if 0
	if (error != REGEX_ERROR_NOMATCH)
		error_message(2, _(txt_pcre_error_num), error);
#endif /* 0 */

	regex_cache_destroy(&tmp_cache);
	return FALSE;
}


/*
 * See if pattern is matched in string. Return the number of captured strings,
 * if so, like pcre and pcre2, or a negative error.
 *
 * A precompiled regex MUST be provided.
 *
 */
int
match_regex_ex(
	const char *string,
	REGEX_SIZE length,
	REGEX_SIZE offset,
	REGEX_OPTIONS options,
	struct regex_cache *regex)
{
#ifndef HAVE_LIB_PCRE2
	int error = pcre_exec(regex->re, regex->extra, string, length, offset, options, regex->ovector, regex->ovecalloc);

	if (error >= 0) {
		/* error == 0 means 'matched, but not enough space in ovector' */
		regex->oveccount = error;
		if (regex->oveccount == 0 && regex->ovecmax > 0)
			regex->oveccount = 1;
		/* should not happen ... */
		if (regex->oveccount > regex->ovecmax)
			regex->oveccount = regex->ovecmax;

	} else
		regex->oveccount = 0;

	return error;
#else
	return pcre2_match_8(regex->re, (const PCRE2_UCHAR8 *) string, length, offset, options, regex->match, NULL);
#endif /* !HAVE_LIB_PCRE2 */
}


REGEX_NOFFSET
regex_get_ovector_count(
	struct regex_cache *regex)
{
#ifdef HAVE_LIB_PCRE2
	return pcre2_get_ovector_count_8(regex->match);
#else
	return regex->oveccount;
#endif /* HAVE_LIB_PCRE2 */
}


REGEX_SIZE
*regex_get_ovector_pointer(
	struct regex_cache *regex)
{
#ifdef HAVE_LIB_PCRE2
	return pcre2_get_ovector_pointer_8(regex->match);
#else
	return regex->ovector;
#endif /* HAVE_LIB_PCRE2 */
}


/*
 * Compile and optimise 'regex'. Return TRUE if all went well
 */
t_bool
compile_regex(
	const char *regex,
	struct regex_cache *cache,
	REGEX_OPTIONS options)
{
	char curr_error[8192];
	static char last_error[8192] = { '\0' };
	t_bool different;
#ifdef HAVE_LIB_PCRE2
	int regex_errcode;
	PCRE2_SIZE regex_errpos;

	if (regex_use_utf8())
		options |= PCRE2_UTF; /* TODO: add PCRE2_UCP? */

	cache->re = pcre2_compile_8((const PCRE2_UCHAR8 *) regex, PCRE2_ZERO_TERMINATED, options,
			&regex_errcode, &regex_errpos, NULL);
	if (cache->re == NULL) {
		PCRE2_UCHAR8 regex_errmsg[256];

		pcre2_get_error_message_8(regex_errcode, regex_errmsg, sizeof(regex_errmsg));
		snprintf(curr_error, sizeof(curr_error), _(txt_pcre_error_at), regex_errmsg, regex_errpos, regex);
		different = (strcmp(last_error, curr_error) != 0);
		error_message(different ? 2 : 0, "%s", curr_error);
		if (different)
			strcpy(last_error, curr_error);
	} else {
		cache->match = pcre2_match_data_create_from_pattern_8(cache->re, NULL);
		if (cache->match == NULL) /* out of memory ... */
			regex_cache_destroy(cache);
		else
			return TRUE;
	}

	return FALSE;

#else
	const char *regex_errmsg = NULL;
	int regex_errpos;

	if (regex_use_utf8())
		options |= PCRE_UTF8;

	if ((cache->re = pcre_compile(regex, options, &regex_errmsg, &regex_errpos, NULL)) == NULL) {
		snprintf(curr_error, sizeof(curr_error), _(txt_pcre_error_at), regex_errmsg, regex_errpos, regex);
		different = (strcmp(last_error, curr_error) != 0);
		error_message(different ? 2 : 0, "%s", curr_error);
		if (different)
			strcpy(last_error, curr_error);
	} else {
		cache->extra = pcre_study(cache->re, 0, &regex_errmsg);
		if (regex_errmsg != NULL) {
			/* we failed, clean up */
			regex_cache_destroy(cache);
			error_message(2, _(txt_pcre_error_text), regex_errmsg);
		} else {
			int n;
			int error = pcre_fullinfo(cache->re, cache->extra, PCRE_INFO_CAPTURECOUNT, &n);

			if (error != 0)
				error_message(2, _(txt_pcre_error_num), error);
			else {
				if (n <= 0)
					n = 1;

				cache->ovecalloc = (n + 1) * 3;
				cache->ovecmax = n;
				cache->oveccount = 0;
				cache->ovector = my_malloc(cache->ovecalloc * sizeof(int));
				return TRUE;
			}
		}
	}

	return FALSE;

#endif /* HAVE_LIB_PCRE2 */
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
	char *buf;

	/* Get contents of line from the screen */
#ifdef USE_CURSES
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	buf = my_malloc(MB_CUR_MAX * (size_t) (cCOLS + 1));
#	else
	buf = my_malloc(cCOLS + 1);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	screen_contents(row, 0, buf);
#else
	buf = screen[row].col;
#endif /* USE_CURSES */
	ptr = buf;

	/* also check for 0 as offsets[] might be too small to hold all captured subpatterns */
	while (match_regex_ex(ptr, (REGEX_SIZE) strlen(ptr), 0, 0, regex) >= 0) {
		REGEX_SIZE *offsets = regex_get_ovector_pointer(regex);
		/* we have a match */
		if (color >= 0) /* color the matching text */
			word_highlight_string(row, (int) ((ptr - buf) + offsets[0]), (int) (offsets[1] - offsets[0]), color);
		else
			/* inverse the matching text */
			highlight_string(row, (int) ((ptr - buf) + offsets[0]), (int) (offsets[1] - offsets[0]));

		if (!tinrc.word_h_display_marks) {
#ifdef USE_CURSES
			screen_contents(row, 0, buf);
#endif /* USE_CURSES */
			ptr += offsets[1] - 2;
		} else
			ptr += offsets[1];
	}
#ifdef USE_CURSES
	free(buf);
#endif /* USE_CURSES */
}


void
regex_cache_init(
	struct regex_cache *regex)
{
	regex->re = NULL;
#ifdef HAVE_LIB_PCRE2
	regex->match = NULL;
#else
	regex->extra = NULL;
	regex->ovector = NULL;
	regex->ovecalloc = 0;
	regex->ovecmax = 0;
	regex->oveccount = 0;
#endif /* HAVE_LIB_PCRE2 */
}


void
regex_cache_destroy(
	struct regex_cache *regex)
{
#ifdef HAVE_LIB_PCRE2
	pcre2_code_free_8(regex->re);
	regex->re = NULL;
	pcre2_match_data_free_8(regex->match);
	regex->match = NULL;
#else
	FreeAndNull(regex->re);
	FreeAndNull(regex->extra);
	FreeAndNull(regex->ovector);
	regex->ovecalloc = 0;
	regex->ovecmax = 0;
	regex->oveccount = 0;
#endif /* HAVE_LIB_PCRE2 */
}


/*
 * returns freshly allocated mem which holds the matched portion
 * of the named subpattern on success and NULL on error
 */
char *
regex_get_substring_by_name(
	struct regex_cache *re,		/* precompiled regex */
	const char *sname,			/* name of subpattern */
	char *subject)				/* data to match against */
{
	char *ms;
	int snum;

#ifdef HAVE_LIB_PCRE2
	PCRE2_UCHAR8 *buf = NULL;
	PCRE2_SIZE buflen = 0;

	(void) subject;
	if ((snum = pcre2_substring_number_from_name_8(re->re, (PCRE2_SPTR8) sname)) < 1)
		return NULL;

	if (pcre2_substring_get_bynumber_8(re->match, snum, &buf, &buflen) < 0) {
		pcre2_substring_free_8(buf);
		return NULL;
	}
	ms = my_strdup((const char *) buf);
	pcre2_substring_free_8(buf);
	return ms;

#else
	const char *buf = NULL;

	regex_get_ovector_pointer(re);
	if ((snum = pcre_get_stringnumber(re->re, sname)) < 1)
		return NULL;

	if (pcre_get_substring(subject, re->ovector, re->oveccount, snum, &buf) < 1)
		return NULL;

	ms = my_strdup(buf);
	pcre_free_substring(buf);
	return ms;
#endif /* HAVE_LIB_PCRE2 */
}


t_bool
regex_use_utf8(
	void)
{
	/* TODO: clarify PCRE_MAJOR, as it does not seem to be set by any
	 * configure variant anymore */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	int i = 0;

#	ifdef HAVE_LIB_PCRE2
	(void) pcre2_config_8(PCRE2_CONFIG_UNICODE, &i);
#	else
#		if defined(PCRE_MAJOR) && PCRE_MAJOR >= 4
			(void) pcre_config(PCRE_CONFIG_UTF8, &i);
#		else
			/* nothing */
#		endif /* PCRE_MAJOR && PCRE_MAJOR >= 4 */
#	endif /* HAVE_LIB_PCRE2 */

	return (IS_LOCAL_CHARSET("UTF-8") && i ? TRUE : FALSE);

#else

	return FALSE;

#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
}
