/*
 *  Project   : tin - a Usenet reader
 *  Module    : string.c
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 1997-01-20
 *  Updated   : 2003-12-28
 *  Notes     :
 *
 * Copyright (c) 1997-2004 Urs Janssen <urs@tin.org>
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
#ifdef HAVE_UNICODE_NORMALIZATION
#	ifdef HAVE_LIBICUUC
#		if defined(HAVE_UNICODE_UNORM_H) && !defined(UNORM_H)
#			include <unicode/unorm.h>
#		endif /* HAVE_UNICODE_UNORM_H && !UNORM_H */
#		if defined(HAVE_UNICODE_USTRING_H) && !defined(USTRING_H)
#			include <unicode/ustring.h>
#		endif /* HAVE_UNICODE_USTRING_H && !USTRING_H */
#	else
#		if defined(HAVE_LIBIDN) && defined(HAVE_STRINGPREP_H) && !defined(_STRINGPREP_H)
#			include <stringprep.h>
#		endif /* HAVE_LIBIDN && HAVE_STRINGPREP_H && !_STRINGPREP_H */
#	endif /* HAVE_LIBICUUC */
#endif /* HAVE_UNICODE_NORMALIZATION */

/*
 * this file needs some work
 */

/*
 * special ltoa()
 * converts value into a string with a len of digits
 * last char may be one of the following
 * Kilo, Mega, Giga, Terra
 */
char *
tin_ltoa(
	long value,
	int digits)
{
	static char buffer[256];
	static const char power[] = " KMGT";
	int len;
	int i = 0;

	snprintf(buffer, sizeof(buffer), "%ld", value);
	len = (int) strlen(buffer);

	while (len > digits) {
		len -= 3;
		i++;
	}

	if (i) {
		while (len < (digits - 1))
			buffer[len++] = ' ';

		buffer[digits - 1] = power[i];
		buffer[digits] = '\0';
	} else
		snprintf(buffer, sizeof(buffer), "%*ld", digits, value);

	return buffer;
}


/*
 * Handrolled version of strdup(), presumably to take advantage of
 * the enhanced error detection in my_malloc
 *
 * also, strdup is not mandatory in ANSI-C
 */
char *
my_strdup(
	const char *str)
{
	size_t len = strlen(str) + 1;
	void *ptr = my_malloc(len);

#if 0 /* as my_malloc exits on error, ptr can't be NULL */
	if (ptr == NULL)
		return NULL;
#endif /* 0 */

	memcpy(ptr, str, len);
	return (char *) ptr;
}


/*
 * strtok that understands empty tokens
 * ie 2 adjacent delims count as two delims around a \0
 */
char *
tin_strtok(
	char *str,
	const char *delim)
{
	static char *buff;
	char *oldbuff, *ptr;

	/*
	 * First call, setup static ptr
	 */
	if (str)
		buff = str;

	/*
	 * If not at end of string find ptr to next token
	 * If delim found, break off token
	 */
	if (buff && (ptr = strpbrk(buff, delim)) != NULL)
		*ptr++ = '\0';
	else
		ptr = NULL;

	/*
	 * Advance position in string to next token
	 * return current token
	 */
	oldbuff = buff;
	buff = ptr;
	return oldbuff;
}


/*
 * strncpy that stops at a newline and null terminates
 */
void
my_strncpy(
	char *p,
	const char *q,
	size_t n)
{
	while (n--) {
		if (!*q || *q == '\n')
			break;
		*p++ = *q++;
	}
	*p = '\0';
}


#ifndef HAVE_STRCASESTR
/*
 * case-insensitive version of strstr()
 */
const char *
strcasestr(
	const char *haystack,
	const char *needle)
{
	const char *h;
	const char *n;

	h = haystack;
	n = needle;
	while (*haystack) {
		if (tolower((unsigned char) *h) == tolower((unsigned char) *n)) {
			h++;
			n++;
			if (!*n)
				return haystack;
		} else {
			h = ++haystack;
			n = needle;
		}
	}
	return NULL;
}
#endif /* !HAVE_STRCASESTR */


size_t
mystrcat(
	char **t,
	const char *s)
{
	size_t len = 0;

	while (*s) {
		*((*t)++) = *s++;
		len++;
	}
	**t = 0;
	return len;
}


void
str_lwr(
	char *str)
{
	char *dst = str;

	while (*str) {
		*dst++ = (char) tolower((unsigned char) *str);
		str++;
	}
	*dst = '\0';
}


/*
 * normal systems come with these...
 */

#ifndef HAVE_STRPBRK
/*
 * find first occurrence of any char from str2 in str1
 */
char *
strpbrk(
	char *str1,
	char *str2)
{
	char *ptr1;
	char *ptr2;

	for (ptr1 = str1; *ptr1 != '\0'; ptr1++) {
		for (ptr2 = str2; *ptr2 != '\0'; ) {
			if (*ptr1 == *ptr2++)
				return ptr1;
		}
	}
	return (char *) 0;
}
#endif /* !HAVE_STRPBRK */

#ifndef HAVE_STRSTR
/*
 * ANSI C strstr() - Uses Boyer-Moore algorithm.
 */
char *
strstr(
	char *text,
	char *pattern)
{
	unsigned char *p, *t;
	int i, j, *delta;
	int deltaspace[256];
	size_t p1;
	size_t textlen;
	size_t patlen;

	textlen = strlen(text);
	patlen = strlen(pattern);

	/* algorithm fails if pattern is empty */
	if ((p1 = patlen) == 0)
		return text;

	/* code below fails (whenever i is unsigned) if pattern too long */
	if (p1 > textlen)
		return NULL;

	/* set up deltas */
	delta = deltaspace;
	for (i = 0; i <= 255; i++)
		delta[i] = p1;
	for (p = (unsigned char *) pattern, i = p1; --i > 0; )
		delta[*p++] = i;

	/*
	 * From now on, we want patlen - 1.
	 * In the loop below, p points to the end of the pattern,
	 * t points to the end of the text to be tested against the
	 * pattern, and i counts the amount of text remaining, not
	 * including the part to be tested.
	 */
	p1--;
	p = (unsigned char *) pattern + p1;
	t = (unsigned char *) text + p1;
	i = textlen - patlen;
	forever {
		if (*p == *t && memcmp ((p - p1), (t - p1), p1) == 0)
			return ((char *) t - p1);
		j = delta[*t];
		if (i < j)
			break;
		i -= j;
		t += j;
	}
	return NULL;
}
#endif /* !HAVE_STRSTR */

#ifndef HAVE_ATOL
/*
 * handrolled atol
 */
long
atol(
	const char *s)
{
	long ret = 0;

	/* skip leading whitespace(s) */
	while (*s && isspace((unsigned char) *s))
		s++;

	while (*s) {
		if (isdigit(*s))
			ret = ret * 10 + (*s - '0');
		else
			return -1;
		s++;
	}
	return ret;
}
#endif /* !HAVE_ATOL */

#ifndef HAVE_STRTOL
/* fix me - put me in tin.h */
#	define DIGIT(x) (isdigit((unsigned char) x) ? ((x) - '0') : (10 + tolower((unsigned char) x) - 'a'))
#	define MBASE 36
long
strtol(
	const char *str,
	char **ptr,
	int use_base)
{
	long val = 0L;
	int xx = 0, sign = 1;

	if (use_base < 0 || use_base > MBASE)
		goto OUT;
	while (isspace((unsigned char) *str))
		++str;
	if (*str == '-') {
		++str;
		sign = -1;
	} else if (*str == '+')
		++str;
	if (use_base == 0) {
		if (*str == '0') {
			++str;
			if (*str == 'x' || *str == 'X') {
				++str;
				use_base = 16;
			} else
				use_base = 8;
		} else
			use_base = 10;
	} else if (use_base == 16)
		if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
			str += 2;
	/*
	 * for any base > 10, the digits incrementally following
	 * 9 are assumed to be "abc...z" or "ABC...Z"
	 */
	while (isalnum((unsigned char) *str) && (xx = DIGIT(*str)) < use_base) {
		/* accumulate neg avoids surprises near maxint */
		val = use_base * val - xx;
		++str;
	}
OUT:
	if (ptr != NULL)
		*ptr = str;

	return (sign * (-val));
}
#	undef DIGIT(x)
#	undef MBASE
#endif /* !HAVE_STRTOL */

#if !defined(HAVE_STRCASECMP) || !defined(HAVE_STRNCASECMP)
	/* fix me - put me in tin.h */
#	define FOLD_TO_UPPER(a)	(toupper((unsigned char) (a)))
#endif /* !HAVE_STRCASECMP || !HAVE_STRNCASECMP */
/*
 * strcmp that ignores case
 */
#ifndef HAVE_STRCASECMP
int
strcasecmp(
	const char *p,
	const char *q)
{
	int r;
	for (; (r = FOLD_TO_UPPER (*p) - FOLD_TO_UPPER (*q)) == 0; ++p, ++q) {
		if (*p == '\0')
			return 0;
	}

	return r;
}
#endif /* !HAVE_STRCASECMP */

#ifndef HAVE_STRNCASECMP
int
strncasecmp(
	const char *p,
	const char *q,
	size_t n)
{
	int r = 0;
	for (; n && (r = (FOLD_TO_UPPER(*p) - FOLD_TO_UPPER(*q))) == 0; ++p, ++q, --n) {
		if (*p == '\0')
			return 0;
	}
	return n ? r : 0;
}
#endif /* !HAVE_STRNCASECMP */


/*
 * str_trim - leading and trailing whitespace
 *
 * INPUT:  string  - string to trim
 *
 * OUTPUT: string  - trimmed string
 *
 * RETURN: trimmed string
 */
char *
str_trim(
	char *string)
{
	char *rp;		/* reading string pointer */
	char *wp;		/* writing string pointer */
	char *ls;		/* last space */

	for (rp = wp = ls = string; isspace((int) *rp); rp++)		/* Skip leading space */
		;

	while (*rp) {
		if (isspace((int) *rp)) {
			if (ls == NULL)		/* Remember last written space */
				ls = wp;
		} else
			ls = NULL;			/* It wasn't the last space */
		*wp++ = *rp++;
	}

	if (ls)						/* ie, there is trailing space */
		*ls = '\0';
	else
		*wp = '\0';

	return string;
}


/*
 * Return a pointer into s eliminating any TAB, CR and LF.
 */
char *
eat_tab(
	char *s)
{
	char *p1 = s;
	char *p2 = s;

	while (*p1) {
		if (*p1 == '\t' || *p1 == '\r' || *p1 == '\n') {
			p1++;
		} else if (p1 != p2) {
			*p2++ = *p1++;
		} else {
			p1++;
			p2++;
		}
	}
	if (p1 != p2)
		*p2 = '\0';

	return s;
}


/*
 * Format a shell command, escaping blanks and other awkward characters that
 * appear in the string arguments. Replaces sprintf, except that we pass in
 * the buffer limit, so we can refrain from trashing memory on very long
 * pathnames.
 *
 * Returns the number of characters written (not counting null), or -1 if there
 * is not enough room in the 'dst' buffer.
 */

#define SH_FORMAT(c)	if (++result >= (int) len) \
				break; \
			*dst++ = c

#define SH_SINGLE "\\\'"
#define SH_DOUBLE "\\\'\"`$"
#define SH_META   "\\\'\"`$*%?()[]{}|<>^&;#~"

int
sh_format(
	char *dst,
	size_t len,
	const char *fmt,
	...)
{
	int result = 0;
	char *src;
	char temp[20];
	va_list ap;
	int quote = 0;

	va_start(ap, fmt);

	while (*fmt != 0) {
		int ch;

		ch = *fmt++;

		if (ch == '\\') {
			SH_FORMAT(ch);
			if (*fmt != '\0') {
				SH_FORMAT(*fmt++);
			}
			continue;
		} else if (ch == '%') {
			if (*fmt == '\0') {
				SH_FORMAT('%');
				break;
			}

			switch (*fmt++) {
			case '%':
				src = strcpy(temp, "%");
				break;

			case 's':
				src = va_arg(ap, char *);
				break;

			case 'd':
				snprintf(temp, sizeof(temp), "%d", va_arg(ap, int));
				src = temp;
				break;

			default:
				src = strcpy(temp, "");
				break;
			}

			while (*src != '\0') {
				t_bool fix;

				/*
				 * This logic works for Unix. Non-Unix systems may require a
				 * different set of problem chars, and may need quotes around
				 * the whole string rather than escaping individual chars.
				 */
				if (quote == '"') {
					fix = (strchr(SH_DOUBLE, *src) != 0);
				} else if (quote == '\'') {
					fix = (strchr(SH_SINGLE, *src) != 0);
				} else
					fix = (strchr(SH_META, *src) != 0);
				if (fix) {
					SH_FORMAT('\\');
				}
				SH_FORMAT(*src++);
			}
		} else {
			if (quote) {
				if (ch == quote)
					quote = 0;
			} else {
				if (ch == '"' || ch == '\'')
					quote = ch;
			}
			SH_FORMAT(ch);
		}
	}
	va_end(ap);

	if (result + 1 >= (int) len)
		result = -1;
	else
		*dst = '\0';

	return result;
}


#ifndef HAVE_STRERROR
#	ifdef HAVE_SYS_ERRLIST
		extern int sys_nerr;
#	endif /* HAVE_SYS_ERRLIST */
char *
my_strerror(
	int n)
{
	static char temp[32];

#	ifdef HAVE_SYS_ERRLIST
	if (n >= 0 && n < sys_nerr)
		return sys_errlist[n];
#	endif /* HAVE_SYS_ERRLIST */
	snprintf(temp, sizeof(temp), "Errno: %d", n);
	return temp;
}
#endif /* !HAVE_STRERROR */


/* strrstr() based on Lars Wirzenius' <lars.wirzenius@helsinki.fi> code */
#ifndef HAVE_STRRSTR
char *
strrstr(
	const char *str,
	const char *pat)
{
	const char *ptr;
	size_t slen, plen;

	if ((str != 0) && (pat != 0)) {
		slen = strlen(str);
		plen = strlen(pat);

		if ((plen != 0) && (plen <= slen)) {
			for (ptr = str + (slen - plen); ptr > str; --ptr) {
				if (*ptr == *pat && strncmp(ptr, pat, plen) == 0)
					return (char *) ptr;
			}
		}
	}
	return NULL;
}
#endif /* !HAVE_STRRSTR */


#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
/*
 * copy wide-chars from '*from' to '*to' until 'columns' columns are filled
 * pad with spaces if necessary
 */
void
wcspart(
	wchar_t *to,
	const wchar_t *from,
	int columns,
	int size_to,
	t_bool pad)
{
	int n, i = 0;
	wchar_t *ptr, *wbuf;

	/* make sure all characters in from are printable */
	wbuf = my_wcsdup(from);
	ptr = wconvert_to_printable(wbuf);

	to[0] = (wint_t) '\0';
	while (*ptr && i < size_to && wcswidth(to, size_to - 1) + wcwidth(*ptr) <= columns) {
		to[i] = *ptr;
		ptr++;
		to[++i] = (wint_t) '\0';
	}

	/* pad with spaces */
	if (pad) {
		n = columns - wcswidth(to, size_to - 1) + (int) wcslen(to);
		for (; i < MIN(n, size_to - 1); i++)
			to[i] = (wint_t) ' ';
		to[i] = (wint_t) '\0';
	}

	free(wbuf);
}
#endif /* MULTIBYTE_ABLE && !NOLOCALE */


#define TRUNC_TAIL	"..."
/*
 * shortens 'mesg' that it occupies at most 'len' screen positions.
 * If it was nessary to truncate 'mesg', " ..." is appended to the
 * resulting string (still 'len' screen positions wide).
 * The resulting string is stored in 'buf'.
 */
char *
strunc(
	const char *message,
	char *buf,
	size_t buf_len,
	int len)
{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wmessage, *wbuf;
	size_t mesg_len;

	mesg_len = mbstowcs(NULL, message, 0);
	if (mesg_len != (size_t) (-1)) {
		wmessage = my_malloc(sizeof(wchar_t) * (mesg_len + 1));
		wbuf = my_malloc(sizeof(wchar_t) * (mesg_len + 1));

		if (mbstowcs(wmessage, message, mesg_len + 1) != (size_t) (-1)) {
			wstrunc(wmessage, wbuf, mesg_len + 1, len);
			if (wcstombs(buf, wbuf, buf_len) != (size_t) (-1)) {
				buf[buf_len - 1] = '\0';

				free(wbuf);
				free(wmessage);

				return buf;
			}
		}

		free(wbuf);
		free(wmessage);
	}
	/* something went wrong using wide-chars, default back to normal chars */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if ((int) strlen(message) <= len && len < (int) buf_len)
		strcpy(buf, message);
	else
		snprintf(buf, buf_len, "%-.*s%s", len - 3, message, TRUNC_TAIL);

	return buf;
}

/*
 * if you use UTF-8 as local charset and want to use
 * U+2026 (HORIZONTAL_ELLIPSIS) instead of "..." uncomment
 * the following define
 */
/* #define USE_UTF8_HORIZONTAL_ELLIPSIS 1 */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
/* the wide-char equivalent of strunc() */
wchar_t *
wstrunc(
	const wchar_t *wmessage,
	wchar_t *wbuf,
	size_t wbuf_len,
	int len)
{
	wchar_t *wtmp;

	/* make sure all characters are printable */
	wtmp = my_wcsdup(wmessage);
	wconvert_to_printable(wtmp);

	if (wcswidth(wtmp, wcslen(wtmp)) <= len && wcslen(wtmp) < wbuf_len) {
		/* wtmp doesn't need to be truncated */
		wcscpy(wbuf, wtmp);
	} else {
		/* wtmp must be truncated */
#	ifdef USE_UTF8_HORIZONTAL_ELLIPSIS
		if (IS_LOCAL_CHARSET("UTF-8")) {
			/*
			 * use U+2026 (HORIZONTAL ELLIPSIS) instead of "..."
			 * we gain two additional screen positions
			 */
			wchar_t wtail[2] = {8230, 0};	/* \0-terminated U+2026 */

			wcspart(wbuf, wtmp, len - 1, wbuf_len - 1, FALSE);
			wcscat(wbuf, wtail);
		} else
#	endif /* USE_UTF8_HORIZONTAL_ELLIPSIS */
		{
			wchar_t tail[4];
			size_t i;

			i = mbstowcs(tail, TRUNC_TAIL, ARRAY_SIZE(tail));
			tail[3] = (wchar_t) '\0';
			assert(i != (size_t) (-1));

			wcspart(wbuf, wtmp, len - 3, wbuf_len - 3, FALSE);
			wcscat(wbuf, tail);
		}
	}
	free(wtmp);

	return wbuf;
}


/*
 * duplicates a wide-char string
 */
wchar_t *
my_wcsdup(
	const wchar_t *wstr)
{
	size_t len = wcslen(wstr) + 1;
	void *ptr = my_malloc(sizeof(wchar_t) * len);

	memcpy(ptr, wstr, sizeof(wchar_t) * len);
	return (wchar_t *) ptr;
}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */


#ifdef HAVE_UNICODE_NORMALIZATION
/*
 * unicode normalization
 *
 * str: the string to normalize (must be UTF-8)
 * returns the normalized string
 * if the normalization failed a copy of the original string will be returned
 *
 * don't forget to free() the allocated memory if not needed anymore
 */
char *
normalize(
	const char *str)
{
	char *buf, *tmp;

	/* make sure str is valid UTF8 */
	tmp = my_strdup(str);
	utf8_valid(tmp);

	if (tinrc.normalization_form == NORMALIZE_NONE) /* normalization is disabled */
		return tmp;

#	ifdef HAVE_LIBICUUC
	{ /* ICU */
		int32_t buf_len, ustr_len, needed, norm_len;
		UChar *ustr, *norm;
		UErrorCode status = U_ZERO_ERROR;
		UNormalizationMode mode;

		switch (tinrc.normalization_form) {
			case NORMALIZE_NFD:
				mode = UNORM_NFD;
				break;

			case NORMALIZE_NFC:
				mode = UNORM_NFC;
				break;

			case NORMALIZE_NFKD:
				mode = UNORM_NFKD;
				break;

			case NORMALIZE_NFKC:
			default:
				mode = UNORM_NFKC;
		}

		/* convert to UTF-16 which is used internally by ICU */
		u_strFromUTF8(NULL, 0, &needed, tmp, (int32_t) strlen(tmp), &status);
		status = U_ZERO_ERROR;		/* reset status */
		ustr_len = needed + 1;
		ustr = (UChar *) my_malloc(sizeof(UChar) * ustr_len);
		u_strFromUTF8(ustr, ustr_len, &needed, tmp, (int32_t) strlen(tmp), &status);
		if (U_FAILURE(status)) {
			/* something went wrong, return the original string (as valid UTF8) */
			free(ustr);
			return tmp;
		}

		needed = unorm_normalize(ustr, -1, mode, 0 , NULL, 0, &status);
		status = U_ZERO_ERROR;		/* reset status */
		norm_len = needed + 1;
		norm = (UChar *) my_malloc(sizeof(UChar) * norm_len);
		needed = unorm_normalize(ustr, -1, mode, 0 , norm, norm_len, &status);
		if (U_FAILURE(status)) {
			/* something went wrong, return the original string (as valid UTF8) */
			free(ustr);
			free(norm);
			return tmp;
		}

		/* convert back to UTF-8 */
		u_strToUTF8(NULL, 0, &needed, norm, -1, &status);
		status = U_ZERO_ERROR;		/* reset status */
		buf_len = needed + 1;
		buf = my_malloc(buf_len);
		u_strToUTF8(buf, buf_len, &needed, norm, -1, &status);
		if (U_FAILURE(status)) {
			/* something went wrong, return the original string (as valid UTF8) */
			free(buf);
			buf = tmp;
		}

		free(ustr);
		free(norm);
		return buf;
	}
#	else
#		ifdef HAVE_LIBIDN
	/* libidn */

	buf = stringprep_utf8_nfkc_normalize(tmp, -1);
	if (buf == NULL) /* normalization failed, return the original string (as valid UTF8) */
		buf = tmp;

	return buf;
#		endif /* HAVE_LIBIDN */
#	endif /* HAVE_LIBICUUC */
}
#endif /* HAVE_UNICODE_NORMALIZATION */


/*
 * returns a pointer to allocated buffer containing the formated string
 * must be freed if not needed anymore
 */
char *
fmt_string(
	const char *fmt,
	...) {
	char *str;
#ifdef HAVE_VASPRINTF
	int n;
#endif /* HAVE_VASPRINTF */
	va_list ap;

	va_start(ap, fmt);
#ifdef HAVE_VASPRINTF
	if ((n = vasprintf(&str, fmt, ap)) == -1)	/* something went wrong */
#endif /* HAVE_VASPRINTF */
	{
		size_t size = LEN;

		str = my_malloc(size);
		/* TODO: realloc str if necessary */
		vsnprintf(str, size, fmt, ap);
	}
	va_end(ap);

	return str;
}
