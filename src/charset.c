/*
 *  Project   : tin - a Usenet reader
 *  Module    : charset.c
 *  Author    : M. Kuhn, T. Burmester
 *  Created   : 1993-12-10
 *  Updated   : 2025-01-31
 *  Notes     : ISO to ascii charset conversion routines
 *
 * Copyright (c) 1993-2025 Markus Kuhn <mgk25@cl.cam.ac.uk>
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

#ifdef CHARSET_CONVERSION
#	ifdef USE_ICU_UCSDET
#		include <unicode/utypes.h>
#		include <unicode/uenum.h>
#		include <unicode/ucsdet.h>
#	endif /* USE_ICU_UCSDET */
#endif /* CHARSET_CONVERSION */

/*
 *  Table for the iso2asc conversion
 *  iso2asc  by  (unrza3@cd4680fs.rrze.uni-erlangen.de)
 *  included by  (root@aspic.han.de)
 */

#define SUB	"?"
#define ISO_EXTRA	0xa0 /* beginning of second range of printable chars */

/*
 * TABSTOP(x) is the column of the character after the TAB
 * at column x. First column is 0, of course.
 */

#define TABSTOP(x)	(((x) - ((x)&7)) + 8)

static constext *const iso2asc[NUM_ISO_TABLES][256-ISO_EXTRA] =
{
	/* universal table for many languages */
	{
	" ","!","c",SUB,SUB,"Y","|",SUB,"\"","(c)","a","<<","-","-","(R)","-",
	" ","+/-","2","3","'","u","P",".",",","1","o",">>"," 1/4"," 1/2"," 3/4","?",
	"A","A","A","A","A","A","AE","C","E","E","E","E","I","I","I","I",
	"D","N","O","O","O","O","O","x","O","U","U","U","U","Y","Th","ss",
	"a","a","a","a","a","a","ae","c","e","e","e","e","i","i","i","i",
	"d","n","o","o","o","o","o",":","o","u","u","u","u","y","th","y"
	},
	/* single-spacing universal table */
	{
	" ","!","c",SUB,SUB,"Y","|",SUB,"\"","c","a","<","-","-","R","-",
	" ",SUB,"2","3","'","u","P",".",",","1","o",">",SUB,SUB,SUB,"?",
	"A","A","A","A","A","A","A","C","E","E","E","E","I","I","I","I",
	"D","N","O","O","O","O","O","x","O","U","U","U","U","Y","T","s",
	"a","a","a","a","a","a","a","c","e","e","e","e","i","i","i","i",
	"d","n","o","o","o","o","o",":","o","u","u","u","u","y","t","y"
	},
	/* table for Danish, Dutch, German, Norwegian and Swedish */
	{
	" ","!","c",SUB,SUB,"Y","|",SUB,"\"","(c)","a","<<","-","-","(R)","-",
	" ","+/-","2","3","'","u","P",".",",","1","o",">>"," 1/4"," 1/2"," 3/4","?",
	"A","A","A","A","Ae","Aa","AE","C","E","E","E","E","I","I","I","I",
	"D","N","O","O","O","O","Oe","x","Oe","U","U","U","Ue","Y","Th","ss",
	"a","a","a","a","ae","aa","ae","c","e","e","e","e","i","i","i","i",
	"d","n","o","o","o","o","oe",":","oe","u","u","u","ue","y","th","ij"
	},
	/* table for Danish, Finnish, Norwegian and Swedish, ISO 646 variant */
	{
	" ","!","c",SUB,"$","Y","|",SUB,"\"","(c)","a","<<","-","-","(R)","-",
	" ","+/-","2","3","'","u","P",".",",","1","o",">>"," 1/4"," 1/2"," 3/4","?",
	"A","A","A","A","[","]","[","C","E","@","E","E","I","I","I","I",
	"D","N","O","O","O","O","\\","x","\\","U","U","U","^","Y","Th","ss",
	"a","a","a","a","{","}","{","c","e","`","e","e","i","i","i","i",
	"d","n","o","o","o","o","|",":","|","u","u","u","~","y","th","y"
	},
	/* table with RFC 1345 codes in brackets */
	{
	"[NS]","[!I]","[Ct]","[Pd]","[Cu]","[Ye]","[BB]","[SE]",
	"[':]","[Co]","[-a]","[<<]","[NO]","[--]","[Rg]","['-]",
	"[DG]","[+-]","[2S]","[3S]","['']","[My]","[PI]","[.M]",
	"[',]","[1S]","[-o]","[>>]","[14]","[12]","[34]","[?I]",
	"[A!]","[A']","[A>]","[A?]","[A:]","[AA]","[AE]","[C,]",
	"[E!]","[E']","[E>]","[E:]","[I!]","[I']","[I>]","[I:]",
	"[D-]","[N?]","[O!]","[O']","[O>]","[O?]","[O:]","[*X]",
	"[O/]","[U!]","[U']","[U>]","[U:]","[Y']","[TH]","[ss]",
	"[a!]","[a']","[a>]","[a?]","[a:]","[aa]","[ae]","[c,]",
	"[e!]","[e']","[e>]","[e:]","[i!]","[i']","[i>]","[i:]",
	"[d-]","[n?]","[o!]","[o']","[o>]","[o?]","[o:]","[-:]",
	"[o/]","[u!]","[u']","[u>]","[u:]","[y']","[th]","[y:]"
	},
	/* table for printers that allow overstriking with backspace */
	{
	" ","!","c\b|","L\b-","o\bX","Y\b=","|",SUB,
	"\"","(c)","a\b_","<<","-\b,","-","(R)","-",
	" ","+\b_","2","3","'","u","P",".",
	",","1","o\b_",">>"," 1/4"," 1/2"," 3/4","?",
	"A\b`","A\b'","A\b^","A\b~","A\b\"","Aa","AE","C\b,",
	"E\b`","E\b'","E\b^","E\b\"","I\b`","I\b'","I\b^","I\b\"",
	"D\b-","N\b~","O\b`","O\b'","O\b^","O\b~","O\b\"","x",
	"O\b/","U\b`","U\b'","U\b^","U\b\"","Y\b'","Th","ss",
	"a\b`","a\b'","a\b^","a\b~","a\b\"","aa","ae","c\b,",
	"e\b`","e\b'","e\b^","e\b\"","i\b`","i\b'","i\b^","i\b\"",
	"d\b-","n\b~","o\b`","o\b'","o\b^","o\b~","o\b\"","-\b:",
	"o\b/","u\b`","u\b'","u\b^","u\b\"","y\b'","th","y\b\""
	},
	/* table for IBM PC character set (code page 437) */
	{
	"\377","\255","\233","\234",SUB,"\235","|","\25",
	"\"","(c)","\246","\256","\252","-","(R)","-",
	"\370","\361","\375","3","'","\346","\24","\371",
	",","1","\247","\257","\254","\253"," 3/4","\250",
	"A","A","A","A","\216","\217","\222","\200",
	"E","\220","E","E","I","I","I","I",
	"D","\245","O","O","O","O","\231","x",
	"\355","U","U","U","\232","Y","T","\341",
	"\205","\240","\203","a","\204","\206","\221","\207",
	"\212","\202","\210","\211","\215","\241","\214","\213",
	"d","\244","\225","\242","\223","o","\224","\366",
	"\355","\227","\243","\226","\201","y","t","\230"
	}
};

/*
 * German tex style to latin1 conversion (by root@aspic, 12/04/93)
 */

#define TEX_SUBST	16
#define SPACES		"                                                                                                         "

static const char *const tex_from[TEX_SUBST] =
{
	"\"a", "\\\"a",
	"\"o", "\\\"o",
	"\"u", "\\\"u",
	"\"A", "\\\"A",
	"\"O", "\\\"O",
	"\"U", "\\\"U",
	"\"s", "\\\"s", "\\3",
	NULL
};

/*
 *  Now the conversion function...
 */

void
convert_iso2asc(
	char *iso,
	char **asc_buffer,
	size_t *max_line_len,
	int t)
{
	constext *p;
	constext *const *tab;
	char *asc;
	int i, a;	/* column counters in iso and asc */
	t_bool first;	/* flag for first SPACE/TAB after other characters */

	asc = *asc_buffer;
	if (iso == NULL || asc == NULL)
		return;

	tab = iso2asc[t];
	first = TRUE;
	i = a = 0;
	while (*iso != '\0') {
		if (*(unsigned char *) (iso) >= ISO_EXTRA) {
			p = tab[*(unsigned char *) (iso) - ISO_EXTRA];
			++iso;
			++i;
			first = TRUE;
			while (*p) {
				*(asc++) = *(p++);
				if ((asc - *asc_buffer) >= (int) *max_line_len) {
					int offset = (int) (asc - *asc_buffer);
					*max_line_len += 64;
					*asc_buffer = my_realloc(*asc_buffer, *max_line_len);
					asc = *asc_buffer + offset;
				}
				++a;
			}
		} else {
			if (a > i && ((*iso == ' ') || (*iso == '\t'))) {
				/*
				 * spaces or TABS should be removed
				 */
				if (*iso == ' ') {
					/*
					 * only the first space after a letter must not be removed
					 */
					if (first) {
						*(asc++) = ' ';
						++a;
						first = FALSE;
					}
					++i;
				} else {	/* here: *iso == '\t' */
					if (a >= TABSTOP(i)) {
						/*
						 * remove TAB or replace it with SPACE if necessary
						 */
						if (first) {
							*(asc++) = ' ';
							++a;
							first = FALSE;
						}
					} else {
						/*
						 * TAB will correct the column difference
						 */
						*(asc++) = '\t';	/* = *iso */
						a = TABSTOP(a);	/* = TABSTOP(i), because i < a < TABSTOP(i) */
					}
					i = TABSTOP(i);
				}
				++iso;
			} else {
				/*
				 * just copy the characters and advance the column counters
				 */
				if (*iso == '\t') {
					a = i = TABSTOP(i);	/* = TABSTOP(a), because here a = i */
				} else if (*iso == '\b') {
					--a;
					--i;
				} else {
					++a;
					++i;
				}
				*(asc++) = *(iso++);
				first = TRUE;
			}
		}
		if ((asc - *asc_buffer) >= (int) *max_line_len) {
			int offset = (int) (asc - *asc_buffer);
			*max_line_len += 64;
			*asc_buffer = my_realloc(*asc_buffer, *max_line_len);
			asc = *asc_buffer + offset;
		}
	}
	*asc = '\0';
}


void
convert_tex2iso(
	char *from,
	char *to)
{
	const char *tex_to[TEX_SUBST];
	int i;
	size_t spaces = 0; /* spaces to add */
	size_t len, col = 0;	/* length of from, col counter */
	size_t subst_len;
	t_bool ex;

	/* initialize tex_to */
	memset(tex_to, '\0', sizeof(tex_to));

	/*
	 * Charsets which have German umlauts incl. sharp s at the same
	 * code position as ISO-8859-1
	 * DEC-MCS, Windows-1252
	 */
	if (
		!strcasecmp( /* ensure not to also match ISO-8859-{11,12} */
#ifdef CHARSET_CONVERSION
			tinrc.mm_local_charset
#else
			tinrc.mm_charset
#endif /* CHARSET_CONVERSION */
			, "ISO-8859-1") ||
		IS_LOCAL_CHARSET("ISO-8859-2") ||
		IS_LOCAL_CHARSET("ISO-8859-3") ||
		IS_LOCAL_CHARSET("ISO-8859-4") ||
		IS_LOCAL_CHARSET("ISO-8859-9") ||
		IS_LOCAL_CHARSET("ISO-8859-10") ||
		IS_LOCAL_CHARSET("ISO-8859-13") ||
		IS_LOCAL_CHARSET("ISO-8859-14") ||
		IS_LOCAL_CHARSET("ISO-8859-15") ||
		IS_LOCAL_CHARSET("ISO-8859-16") ||
		iso2asc_supported >= 0) {
		tex_to[1] = tex_to[0] = "\344";	/* auml */
		tex_to[3] = tex_to[2] = "\366";	/* ouml */
		tex_to[5] = tex_to[4] = "\374";	/* uuml */
		tex_to[7] = tex_to[6] = "\304";	/* Auml */
		tex_to[9] = tex_to[8] = "\326";	/* Ouml */
		tex_to[11] = tex_to[10] = "\334";	/* Uuml */
		tex_to[14] = tex_to[13] = tex_to[12] = "\337"; /* szlig */
	} else if (IS_LOCAL_CHARSET("UTF-8")) { /* locale charset is UTF-8 */
		tex_to[1] = tex_to[0] = "\303\244";	/* auml */
		tex_to[3] = tex_to[2] = "\303\266";	/* ouml */
		tex_to[5] = tex_to[4] = "\303\274";	/* uuml */
		tex_to[7] = tex_to[6] = "\303\204";	/* Auml */
		tex_to[9] = tex_to[8] = "\303\226";	/* Ouml */
		tex_to[11] = tex_to[10] = "\303\234";	/* Uuml */
		tex_to[14] = tex_to[13] = tex_to[12] = "\303\237";	/* szlig */
	} else {
		strcpy(to, from);
		return;
	}

	*to = '\0';
	len = strlen(from);
	while (col < len) {
		i = 0;
		ex = FALSE;
		while ((i < TEX_SUBST - 1) && !ex) {
			subst_len = strlen(tex_from[i]);
			if (!strncmp(from + col, tex_from[i], subst_len)) {
				strcat(to, tex_to[i]);
				spaces += subst_len - strlen(tex_to[i]);
				col += subst_len - 1;
				ex = TRUE;
			}
			++i;
		}
		if (!ex)
			strncat(to, from + col, 1);
		if (from[col] == ' ') {
			strncat(to, SPACES, spaces);
			spaces = 0;
		}
		++col;
	}
}


/*
 * Check for German TeX encoding in file open on fp
 */
t_bool
is_art_tex_encoded(
	FILE *fp)
{
	char *line;
	int i, len;
	t_bool body = FALSE;

	rewind(fp);
	while ((line = tin_fgets(fp, FALSE)) != NULL) {
		if (!body) {
			if (!*line)
				body = TRUE;

			continue;
		}

		i = 0;
		while (line[i++] == ' ')
			;	/* search for first non blank */

		--i;
		if (!isalnum((unsigned char) line[i]) && line[i] != '\"')
			continue;	/* quoting char */

		len = (int) strlen(line) - 1;
		for (i = 1; i < len; i++) {
			if (((line[i] == '\\') || (line[i] == '\"')) &&
							(isalnum((unsigned char) line[i - 1])) &&
							(isalnum((unsigned char) line[i + 1])))
				return TRUE;
		}
	}
	return FALSE;
}


/*
 * Replace all non printable characters by '?'
 */
char *
convert_to_printable(
	char *buf,
	t_bool keep_tab)
{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	char *buffer;
	wchar_t *wbuffer;
	size_t len = strlen(buf) + 1;

	if (IS_LOCAL_CHARSET("UTF-8"))
		utf8_valid(buf);

	if ((wbuffer = char2wchar_t(buf)) != NULL) {
		wconvert_to_printable(wbuffer, keep_tab);
		if ((buffer = wchar_t2char(wbuffer)) != NULL) {
			strncpy(buf, buffer, len);
			buf[len - 1] = '\0';
			free(buffer);
		}
		free(wbuffer);
	}
#else
	unsigned char *c;

	for (c = (unsigned char *) buf; *c; c++) {
		if (!my_isprint(*c) && !(keep_tab && *c == '\t'))
			*c = '?';
	}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	return buf;
}


#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
/*
 * NOTES: don't make wc a wint_t as libutf8 (at least version 0.8)
 *        sometimes fails to proper convert (wchar_t) 0 to (wint_t) 0
 *        and thus loop termination fails.
 */
wchar_t *
wconvert_to_printable(
	wchar_t *wbuf,
	t_bool keep_tab)
{
	wchar_t *wc;

	for (wc = wbuf; *wc; wc++) {
		if (!iswprint((wint_t) *wc) && !(keep_tab && *wc == (wchar_t) '\t'))
			*wc = (wchar_t) '?';
	}
	return wbuf;
}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */


/*
 * Check for charsets which may contain NULL bytes and thus break string
 * functions. Possibly incomplete.
 *
 * TODO: fix the other code to handle those charsets properly.
 */
t_bool
charset_unsupported(
	const char *charset)
{
	static const char *charsets[] = {
		"csUnicode",	/* alias for ISO-10646-UCS-2 */
		"csUCS4",		/* alias for ISO-10646-UCS-4 */
		"ISO-10646-UCS-2", /* can't be shortened due to ISO-10646-UCS-Basic */
		"ISO-10646-UCS-4",
		"UTF-16",		/* covers also BE/LE */
		"UTF-32",		/* covers also BE/LE */
		"UNICODE-1-1",
		"SCSU",
		"csSCSU",
		NULL };
	const char **charsetptr = charsets;
	t_bool ret = FALSE;

	if (!charset)
		return ret;

	do {
		if (!strncasecmp(charset, *charsetptr, strlen(*charsetptr)))
			return TRUE;
	} while (*(++charsetptr) != NULL);

#ifdef CHARSET_CONVERSION
#	ifdef CHARSET_CONVERSION_ICONV
	{
		iconv_t cd;

		if ((cd = iconv_open("UCS-4", charset)) != (iconv_t) (-1)) {
			iconv_close(cd);
			return ret;
		}
	}
#	endif /* CHARSET_CONVERSION_ICONV */
#	ifdef CHARSET_CONVERSION_UCNV
	/* check if icu can handle it */
	{
		UErrorCode err = U_ZERO_ERROR;
		UConverter *test = ucnv_open(charset, &err);

		if (U_FAILURE(err)) {
#if defined(DEBUG) && defined(DEBUG_UCNV)
			wait_message(2, "UCNV: %s (%s:%d) %s", u_errorName(err), __FILE__, __LINE__, charset);
#endif /* DEBUG && DEBUG_UCNV */
			ret = TRUE;
		} else
			ucnv_close(test);
	}
#	endif /* CHARSET_CONVERSION_UCNV */
#endif /* CHARSET_CONVERSION */

	return ret;
}


#if defined(CHARSET_CONVERSION) && defined(USE_ICU_UCSDET)
char *
guess_charset(
		const char *sample,
		int32_t confidence)
{
	char *guessed_charset = NULL;
	const char *p_match;
	UCharsetDetector *detector;
	const UCharsetMatch *match;
	UErrorCode status = U_ZERO_ERROR;

	detector = ucsdet_open(&status);
	if (U_FAILURE(status))
		goto failure;

	ucsdet_setText(detector, sample, -1, &status);
	if (U_FAILURE(status))
		goto failure;

	/*
	 * TODO: use ucsdet_detectAll() and loop over the results?
	 * currently we just look at the best match and that might
	 * be a charset we can't handle (e.g. UTF-16) ...
	 */

	match = ucsdet_detect(detector, &status);
	if (match == NULL || U_FAILURE(status))
		goto failure;

	p_match = ucsdet_getName(match, &status);
	if (p_match == NULL || U_FAILURE(status))
		goto failure;

	if (charset_unsupported(p_match))
		goto failure;

	/* badguess = 0 ... perfect = 100 */
	if (ucsdet_getConfidence(match, &status) >= confidence)
		guessed_charset = my_strdup(p_match);

failure:
	if (detector)
		ucsdet_close(detector);

	return guessed_charset;
}
#endif /* CHARSET_CONVERSION && USE_ICU_UCSDET */


/*
 * restrict it to [a-zA-Z0-9_-]+
 */
const char *
validate_charset(
	const char *charset)
{
	const char *c = charset;

	if (!charset || strlen(charset) > 40) /* RFC 2978 2.3 */
		return NULL;

	while (*c) {
		if (*c < 45 || *c > 122 || *c == 46 || *c == 47 || (*c >= 58 && *c <= 64) || (*c >= 91 && *c <= 94) || *c == 96)
			return NULL;

		++c;
	}
	return charset;
}


/*
 * return pos. of charset in txt_mime_charsets or -1 if not found
 */
int
charset_name_to_num(
	const char *charset)
{
	int i;

	if (!charset || !*charset)
		return -1;

	for (i = 0; txt_mime_charsets[i] != NULL; i++) {
		if (!strcasecmp(charset, txt_mime_charsets[i]))
			return i;
	}

	return -1;
}
