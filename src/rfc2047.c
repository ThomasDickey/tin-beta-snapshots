/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2047.c
 *  Author    : Chris Blum <chris@resolution.de>
 *  Created   : 1995-09-01
 *  Updated   : 2005-07-02
 *  Notes     : MIME header encoding/decoding stuff
 *
 * Copyright (c) 1995-2006 Chris Blum <chris@resolution.de>
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


#define isreturn(c) ((c) == '\r' || ((c) == '\n'))

/*
 * Modified to return TRUE for '(' and ')' only if
 * it's in structured header field. '(' and ')' are
 * NOT to be treated differently than other characters
 * in unstructured headers like Subject, Keyword and Summary
 * c.f. RFC 2047
 */
#define isbetween(c, s) (isspace((unsigned char) c) || ((s) && ((c) == '(' || (c) == ')' || (c) == '"')))

#define NOT_RANKED 255

#if 0
	/* inside a quoted word these 7bit chars need to be encoded too */
#	define RFC2047_ESPECIALS "[]<>.;@,=?_\"\\"
#endif /* 0 */

const char base64_alphabet[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static unsigned char base64_rank[256];
static int base64_rank_table_built;
static t_bool quoteflag;

/* fixed prefix and default part for tin-generated MIME boundaries */
static const char MIME_BOUNDARY_PREFIX[] = "=_tin=_";
static const char MIME_BOUNDARY_DEFAULT_PART[] = "====____====____====____";
/* required size of a buffer containing a MIME boundary, including the final '\0' */
enum {
	MIME_BOUNDARY_SIZE = sizeof(MIME_BOUNDARY_PREFIX) + sizeof(MIME_BOUNDARY_DEFAULT_PART) - 1
};

/*
 * local prototypes
 */
static FILE *compose_message_rfc822(FILE *articlefp, t_bool *is_8bit);
static FILE *compose_multipart_mixed(FILE *textfp, FILE *articlefp);
static int do_b_encode(char *w, char *b, int max_ewsize, t_bool isstruct_head);
static int sizeofnextword(char *w);
static int which_encoding(char *w);
static t_bool contains_8bit_characters(FILE *fp);
static t_bool contains_nonprintables(char *w, t_bool isstruct_head);
static t_bool contains_string(FILE *fp, const char *str);
static t_bool rfc1522_do_encode(char *what, char **where, const char *charset, t_bool break_long_line);
static t_bool split_mail(const char *filename, FILE **headerfp, FILE **textfp);
static unsigned hex2bin(int x);
static void build_base64_rank_table(void);
static void do_rfc15211522_encode(FILE *f, constext * mime_encoding, struct t_group *group, t_bool allow_8bit_header, t_bool ismail, t_bool contains_headers);
static void generate_mime_boundary(char *boundary, FILE *f, FILE *g);
static void generate_random_mime_boundary(char *boundary, size_t len);
static void str2b64(const char *from, char *to);


static void
build_base64_rank_table(
	void)
{
	int i;

	if (!base64_rank_table_built) {
		for (i = 0; i < 256; i++)
			base64_rank[i] = NOT_RANKED;
		for (i = 0; i < 64; i++)
			base64_rank[(int) base64_alphabet[i]] = i;
		base64_rank_table_built = TRUE;
	}
}


static unsigned
hex2bin(
	int x)
{
	if (x >= '0' && x <= '9')
		return (x - '0');
	if (x >= 'A' && x <= 'F')
		return (x - 'A') + 10;
	if (x >= 'a' && x <= 'f')
		return (x - 'a') + 10;
	return 255;
}


/*
 * Do B or Q decoding of a chunk of data in 'what' to 'where'
 * Return number of bytes decoded into 'where' or -1.
 */
int
mmdecode(
	const char *what,
	int encoding,
	int delimiter,
	char *where)
{
	char *t;

	t = where;
	encoding = tolower((unsigned char) encoding);
	if (encoding == 'q') {		/* quoted-printable */
		int x;
		unsigned hi, lo;

		if (!what || !where) /* should not happen with 'q'-encoding */
			return -1;

		while (*what != delimiter) {
			if (*what != '=') {
				if (!delimiter || *what != '_')
					*t++ = *what++;
				else
					*t++ = ' ', what++;
				continue;
			}
			what++;
			if (*what == delimiter)		/* failed */
				return -1;

			x = *what++;
			if (x == '\n')
				continue;
			if (*what == delimiter)
				return -1;

			hi = hex2bin(x);
			lo = hex2bin(*what);
			what++;
			if (hi == 255 || lo == 255)
				return -1;
			x = (hi << 4) + lo;
			*EIGHT_BIT(t)++ = x;
		}
		return t - where;
	} else if (encoding == 'b') {		/* base64 */
		static unsigned short pattern = 0;
		static int bits = 0;
		unsigned char x;

		if (!what || !where) {		/* flush */
			pattern = 0;
			bits = 0;
			return 0;
		}

		build_base64_rank_table();

		while (*what != delimiter) {
			x = base64_rank[(int) (*what++)];
			/* ignore everything not in the alphabet, including '=' */
			if (x == NOT_RANKED)
				continue;
			pattern <<= 6;
			pattern |= x;
			bits += 6;
			if (bits >= 8) {
				x = (pattern >> (bits - 8)) & 0xff;
				*t++ = x;
				bits -= 8;
			}
		}
		return t - where;
	}
	return -1;
}


/*
 * This routine decodes encoded headers in the
 * =?charset?encoding?coded text?=
 * format
 */
char *
rfc1522_decode(
	const char *s)
{
	char *c, *sc;
	const char *d;
	char *t;
#define BUFFER_LEN 2048
	static char buffer[BUFFER_LEN];
	int max_len;
	char charset[1024];
	char encoding;
	t_bool adjacentflag = FALSE;

	charset[0] = '\0';
	c = my_strdup(s);
	max_len = strlen(c) + 1;
	t = buffer;

	/*
	 * remove non-ASCII chars if MIME_STRICT_CHARSET is set
	 * must be changed if UTF-8 becomes default charset for headers:
	 *
	 * process_charsets(c, len, "UTF-8", tinrc.mm_local_charset, FALSE);
	 */
#ifndef CHARSET_CONVERSION
	process_charsets(&c, &max_len, "US-ASCII", tinrc.mm_local_charset, FALSE);
#else
	process_charsets(&c, &max_len, (CURR_GROUP.attribute->undeclared_charset) ? (CURR_GROUP.attribute->undeclared_charset) : "US-ASCII", tinrc.mm_local_charset, FALSE);
#endif /* !CHARSET_CONVERSION */
	sc = c;

	while (*c && t - buffer < BUFFER_LEN) {
		if (*c != '=') {
			if (adjacentflag && isspace((unsigned char) *c)) {
				const char *dd;

				dd = c + 1;
				while (isspace((unsigned char) *dd))
					dd++;
				if (*dd == '=') {		/* brute hack, makes mistakes under certain circumstances comp. 6.2 */
					c++;
					continue;
				}
			}
			adjacentflag = FALSE;
			*t++ = *c++;
			continue;
		}
		d = c++;
		if (*c == '?') {
			char *e;

			e = charset;
			c++;
			while (*c && *c != '?')
				*e++ = *c++;
			*e = 0;
			if (*c == '?') {
				c++;
				encoding = tolower((unsigned char)*c);
				if (encoding == 'b')
					(void) mmdecode(NULL, 'b', 0, NULL);	/* flush */
				c++;
				if (*c == '?') {
					c++;
					if ((e = strchr(c, '?'))) {
						int i;

						i = mmdecode(c, encoding, '?', t);
						if (i > 0) {
							int chars_to_copy;
							char *tmpbuf;

							max_len = i + 1;
							tmpbuf = my_malloc(max_len);
							strncpy(tmpbuf, t, i);
							*(tmpbuf + i) = '\0';
							process_charsets(&tmpbuf, &max_len, charset, tinrc.mm_local_charset, FALSE);
							chars_to_copy = strlen(tmpbuf);
							if (chars_to_copy > BUFFER_LEN - (t - buffer) - 1)
								chars_to_copy = BUFFER_LEN - (t - buffer) - 1;
							strncpy(t, tmpbuf, chars_to_copy);
							free(tmpbuf);
							t += chars_to_copy;
							e++;
							if (*e == '=')
								e++;
							d = c = e;
							adjacentflag = TRUE;
						}
					}
				}
			}
		}
		while (d != c && t - buffer < BUFFER_LEN - 1)
			*t++ = *d++;
	}
	*t = '\0';
	free(sc);

	return buffer;
}


/*
 * adopted by J. Shin(jshin@pantheon.yale.edu) from
 * Woohyung Choi's(whchoi@cosmos.kaist.ac.kr) sdn2ks and ks2sdn
 */
static void
str2b64(
	const char *from,
	char *to)
{
	short int i, count;
	unsigned long tmp;

	while (*from) {
		for (i = count = 0, tmp = 0; i < 3; i++)
			if (*from) {
				tmp = (tmp << 8) | (unsigned long) (*from++ & 0x0ff);
				count++;
			} else
				tmp = (tmp << 8) | (unsigned long) 0;

		*to++ = base64_alphabet[(0x0fc0000 & tmp) >> 18];
		*to++ = base64_alphabet[(0x003f000 & tmp) >> 12];
		*to++ = count >= 2 ? base64_alphabet[(0x0000fc0 & tmp) >> 6] : '=';
		*to++ = count >= 3 ? base64_alphabet[0x000003f & tmp] : '=';
	}

	*to = '\0';
	return;
}


static int
do_b_encode(
	char *w,
	char *b,
	int max_ewsize,
	t_bool isstruct_head)
{
	char tmp[60];				/* strings to be B encoded */
	char *t = tmp;
	int len8 = 0;				/* the number of trailing 8bit chars, which
								   should be even (i.e. the first and second byte
								   of wide_char should NOT be split into two
								   encoded words) in order to be compatible with
								   some CJK mail client */
	int count = max_ewsize / 4 * 3;
	t_bool isleading_between = TRUE;		/* are we still processing leading space */

	while (count-- > 0 && (!isbetween(*w, isstruct_head) || isleading_between) && *w) {
		len8 += is_EIGHT_BIT(w) ? 1 : (-len8);
		if (!isbetween(*w, isstruct_head))
			isleading_between = FALSE;
		*(t++) = *(w++);
	}

/* if (len8 & (unsigned long) 1 && !isbetween(*w,isstruct_head)) */
	if (len8 != len8 / 2 * 2 && !isbetween(*w, isstruct_head) && (*w))
		t--;

	*t = '\0';

	str2b64(tmp, b);

	return t - tmp;
}


/*
 * find out whether encoding is necessary and which encoding
 * to use if necessary by scanning the whole header field
 * instead of each fragment of it.
 * This will ensure that either Q or B encoding will be used in a single
 * header (i.e. two encoding won't be mixed in a single header line).
 * Mixing two encodings is not a violation of RFC 2047 but may break
 * some news/mail clients.
 *
 * mmnwcharset is ignored unless CHARSET_CONVERSION
 */
static int
which_encoding(
	char *w)
{
	int chars = 0;
	int schars = 0;
	int nonprint = 0;

	while (*w && isspace((unsigned char) *w))
		w++;
	while (*w) {
		if (is_EIGHT_BIT(w))
			nonprint++;
		if (!nonprint && *w == '=' && *(w + 1) == '?')
			nonprint = 1;
		if (*w == '=' || *w == '?' || *w == '_')
			schars++;
		chars++;
		w++;
	}
	if (nonprint) {
		if (chars + 2 * (nonprint + schars) /* QP size */ >
			 (chars * 4 + 3) / 3		/* B64 size */)
			return 'B';
		return 'Q';
	}
	return 0;
}


/* now only checks if there's any 8bit chars in a given "fragment" */
static t_bool
contains_nonprintables(
	char *w,
	t_bool isstruct_head)
{
	t_bool nonprint = FALSE;

	/* first skip all leading whitespaces */
	while (*w && isspace((unsigned char) *w))
		w++;

	/* then check the next word */
	while (!nonprint && *w && !isbetween(*w, isstruct_head)) {
		if (is_EIGHT_BIT(w))
			nonprint = TRUE;
		else if (*w == '=' && *(w + 1) == '?') {
			/*
			 * to be exact we must look for ?= in the same word
			 * not in the whole string and check ?B? or ?Q? in the word...
			 * best would be using a regexp like
			 * ^=\?\S+\?[qQbB]\?\S+\?=
			 */
			if (strstr(w, "?=") != NULL)
				nonprint = TRUE;
		}
		w++;
	}
	return nonprint;
}


/*
 * implement mandatory break-up of long lines in mail messages in accordance
 * with rfc 2047 (rfc 1522)
 */
static int
sizeofnextword(
	char *w)
{
	char *x;

	x = w;
	while (*x && isspace((unsigned char) *x))
		x++;
	while (*x && !isspace((unsigned char) *x))
		x++;
	return x - w;
}


static t_bool
rfc1522_do_encode(
	char *what,
	char **where,
	const char *charset,
	t_bool break_long_line)
{
	/*
	 * We need to meet several partly contradictional requirements here.
	 * First of all, a line containing MIME encodings must not be longer
	 * than 76 chars (including delimiters, charset, encoding). Second,
	 * we should not encode more than necessary. Third, we should not
	 * produce more overhead than absolutely necessary; this means we
	 * should extend chunks over several words if there are more
	 * characters-to-quote to come. This means we have to rely on some
	 * heuristics. We process whole words, checking if it contains
	 * characters to be quoted. If not, the word is output 'as is',
	 * previous quoting being terminated before. If two adjoining words
	 * contain non-printable characters, they are encoded together (up
	 * to 60 characters). If a resulting encoded word would break the
	 * 76 characters boundary, we 'break' the line, output a SPACE, then
	 * output the encoded word. Note that many wide-spread news applications,
	 * notably INN's xover support, does not understand multiple-lines,
	 * so it's a compile-time feature with default off.
	 *
	 * To make things a bit easier, we do all processing in two stages;
	 * first we build all encoded words without any bells and whistles
	 * (just checking that they don get longer than 76 characters),
	 * then, in a second pass, we replace all SPACEs inside encoded
	 * words by '_', break long lines, etc.
	 *
	 * *group is passed for evaluation of group-attributes,
	 * set to NULL for e-mails
	 */
	char *c;
	char *t;
	char buf[2048];				/* buffer for encoded stuff */
	char buf2[80];					/* buffer for this and that */
	int encoding;					/* which encoding to use ('B' or 'Q') */
	int ew_taken_len;
	int column = 0;				/* current column */
	int word_cnt = 0;
	size_t ewsize = 0;			/* size of current encoded-word */
	t_bool quoting = FALSE;		/* currently inside quote block? */
	t_bool any_quoting_done = FALSE;
	t_bool isbroken_within = FALSE;	/* is word broken due to length restriction on encoded of word? */
	t_bool isstruct_head = FALSE;		/* are we dealing with structured header? */
	t_bool rightafter_ew = FALSE;
/*
 * the list of structured header fields where '(' and ')' are
 * treated specially in rfc 1522 encoding
 */
	static const char *struct_header[] = {
		"Approved: ", "From: ", "Originator: ",
		"Reply-To: ", "Sender: ", "X-Cancelled-By: ", "X-Comment-To: ",
		"X-Submissions-To: ", "To: ", "Cc: ", "Bcc: ", "X-Originator: ", 0 };
	const char **strptr = struct_header;

	do {
		if (!strncasecmp(what, *strptr, strlen(*strptr))) {
			isstruct_head = TRUE;
			break;
		}
	} while (*(++strptr) != 0);

	t = buf;
	encoding = which_encoding(what);
	ew_taken_len = strlen(charset) + 7 /* =?c?E?d?= */;
	while (*what) {
		if (break_long_line)
			word_cnt++;
/*
 * if a word with 8bit chars is broken in the middle, whatever follows
 * after the point where it's split should be encoded (i.e. even if
 * they are made of only 7bit chars)
 */
		if (contains_nonprintables(what, isstruct_head) || isbroken_within) {
			if (encoding == 'Q') {
				if (!quoting) {
					snprintf(buf2, sizeof(buf2), "=?%s?%c?", charset, encoding);
					ewsize = mystrcat(&t, buf2);
					if (break_long_line) {
						if (word_cnt == 2) {
							/*
							 * Make sure we fit the first encoded
							 * word in with the header keyword,
							 * since we cannot break the line
							 * directly after the keyword.
							 */
							ewsize = t - buf;
						}
					}
					quoting = TRUE;
					any_quoting_done = TRUE;
				}
				isbroken_within = FALSE;
				while (*what && !isbetween(*what, isstruct_head)) {
#if 0
					if (is_EIGHT_BIT(what) || (strchr(RFC2047_ESPECIALS, *what)))
#else
					if (is_EIGHT_BIT(what) || !isalnum((int)(unsigned char) *what))
#endif /* 0 */
					{
						snprintf(buf2, sizeof(buf2), "=%2.2X", *EIGHT_BIT(what));
						*t++ = buf2[0];
						*t++ = buf2[1];
						*t++ = buf2[2];
						ewsize += 3;
					} else {
						*t++ = *what;
						ewsize++;
					}
					what++;
					/*
					 * Be sure to encode at least one char, even if
					 * that overflows the line limit, otherwise, we
					 * will be stuck in a loop (if this were in the
					 * while condition above). (Can only happen in
					 * the first line, if we have a very long
					 * header keyword, I think).
					 */
					if (ewsize >= 71) {
						isbroken_within = TRUE;
						break;
					}
				}
				if (!contains_nonprintables(what, isstruct_head) || ewsize >= 70 - strlen(charset)) {
					/* next word is 'clean', close encoding */
					*t++ = '?';
					*t++ = '=';
					ewsize += 2;
/*
 * if a word with 8bit chars is broken in the middle, whatever follows
 * after the point where it's split should be encoded (i.e. even if
 * they are made of only 7bit chars)
 */
					if (ewsize >= 70 - strlen(charset) && (contains_nonprintables(what, isstruct_head) || isbroken_within)) {
						*t++ = ' ';
						ewsize++;
					}
					quoting = FALSE;
				} else {
					/* process whitespace in-between by quoting it properly */
					while (*what && isspace((unsigned char) *what)) {
						if (*what == 32 /* not ' ', compare chapter 4! */ ) {
							*t++ = '_';
							ewsize++;
						} else {
							snprintf(buf2, sizeof(buf2), "=%2.2X", *EIGHT_BIT(what));
							*t++ = buf2[0];
							*t++ = buf2[1];
							*t++ = buf2[2];
							ewsize += 3;
						}
						what++;
					}					/* end of while */
				}						/* end of else */
			} else {					/* end of Q encoding and beg. of B encoding */
				/*
				 * if what immediately precedes the current fragment with 8bit
				 * char is encoded word, the leading spaces should be encoded
				 * together with 8bit chars following them. No need to worry
				 * about '(',')' and '"' as they're already excluded with
				 * contain_nonprintables used in outer if-clause
				 */
				while (*what && (!isbetween(*what, isstruct_head) || rightafter_ew)) {
					snprintf(buf2, sizeof(buf2), "=?%s?%c?", charset, encoding);
					ewsize = mystrcat(&t, buf2);

					if (word_cnt == 2)
						ewsize = t - buf;
					what += do_b_encode(what, buf2, 75 - ew_taken_len, isstruct_head);
					ewsize += mystrcat(&t, buf2);
					*t++ = '?';
					*t++ = '=';
					*t++ = ' ';
					ewsize += 3;
					if (break_long_line)
						word_cnt++;
					rightafter_ew = FALSE;
					any_quoting_done = TRUE;
				}
				rightafter_ew = TRUE;
				word_cnt--;		/* compensate double counting */
				/*
				 * if encoded word is followed by 7bit-only fragment, we need to
				 * eliminate ' ' inserted in while-block above
				 */
				if (!contains_nonprintables(what, isstruct_head)) {
					t--;
					ewsize--;
				}
			}		/* end of B encoding */
		} else {
			while (*what && !isbetween(*what, isstruct_head))
				*t++ = *what++;		/* output word unencoded */
			while (*what && isbetween(*what, isstruct_head))
				*t++ = *what++;		/* output trailing whitespace unencoded */
			rightafter_ew = FALSE;
		}
	}		/* end of pass 1 while loop */
	*t = 0;
	/* Pass 2: break long lines if there are MIME-sequences in the result */
	c = buf;
	if (break_long_line) {
		column = 0;
		if (any_quoting_done) {
			word_cnt = 1;			/*
									 * note, if the user has typed a continuation
									 * line, we will consider the initial
									 * whitespace to be delimiting word one (well,
									 * just assume an empty word).
									 */
			while (*c) {
				if (isspace((unsigned char) *c)) {
					/*
					 * According to rfc1522, header lines containing encoded
					 * words are limited to 76 chars, but if the first line is
					 * too long (due to a long header keyword), we cannot stick
					 * to that, since we would break the line directly after the
					 * keyword's colon, which is not allowed. The same is
					 * necessary for a continuation line with an unencoded word
					 * that is too long.
					 */
					if (sizeofnextword(c) + column > 76 && word_cnt != 1) {
						*((*where)++) = '\n';
						column = 0;
					}
					if (c > buf && !isspace((unsigned char) *(c - 1)))
						word_cnt++;
					*((*where)++) = *c++;
					column++;
				} else
					while (*c && !isspace((unsigned char) *c)) {
						*((*where)++) = *c++;
						column++;
					}
			}
		} else {
			while (*c)
				*((*where)++) = *c++;
		}
	} else {		/* !break_long_line */
		while (*c)
			*((*where)++) = *c++;
	}

	**where = 0;
	return any_quoting_done;
}


/*
 * calling code must free() the result if it's no longer needed
 */
char *
rfc1522_encode(
	char *s,
	const char *charset,
	t_bool ismail)
{
	char *b, *buf;
	t_bool x;
/*
 * break_long_line is FALSE for news posting unless MIME_BREAK_LONG_LINES is
 * defined, but it's TRUE for mail messages regardless of whether or not
 * MIME_BREAK_LONG_LINES is defined
 */
#ifdef MIME_BREAK_LONG_LINES
	t_bool break_long_line = TRUE;
#else
/*
 * Even if MIME_BREAK_LONG_LINES is NOT defined, long headers in mail
 * messages should be broken up in accordance with RFC 2047(1522)
 */
	t_bool break_long_line = ismail;
#endif /* MIME_BREAK_LONG_LINES */

	b = buf = my_malloc(2048);
	x = rfc1522_do_encode(s, &b, charset, break_long_line);
	quoteflag = quoteflag || x;
	return buf;
}


/*
 * Helper function doing the hard work for rfc15211522_encode().
 * Code moved from rfc15211522_encode(), with some adjustments to work on a
 * file specified by a FILE* instead of a filename.
 */
static void
do_rfc15211522_encode(
	FILE *f,
	constext * mime_encoding,
	struct t_group *group,
	t_bool allow_8bit_header,
	t_bool ismail,
	t_bool contains_headers)
{
	FILE *g;
	char *c;
	char *header;
	char encoding;
	char buffer[2048];
	t_bool mime_headers_needed = FALSE;
	BodyPtr body_encode;
	int i;
#ifdef CHARSET_CONVERSION
	int mmnwcharset;

	if (group) /* Posting */
		mmnwcharset = group->attribute->mm_network_charset;
	else /* E-Mail */
		mmnwcharset = tinrc.mm_network_charset;
#endif /* CHARSET_CONVERSION */

	if ((g = tmpfile()) == NULL)
		return;

	quoteflag = FALSE;

	while (contains_headers && (header = tin_fgets(f, TRUE))) {
#ifdef CHARSET_CONVERSION
		buffer_to_network(header, mmnwcharset);
#endif /* CHARSET_CONVERSION */
		if (*header == '\0')
			break;

		if (allow_8bit_header)
			fputs(header, g);
		else {
			char *p;

#ifdef CHARSET_CONVERSION
			p = rfc1522_encode(header, txt_mime_charsets[mmnwcharset], ismail);
#else
			p = rfc1522_encode(header, tinrc.mm_charset, ismail);
#endif /* CHARSET_CONVERSION */

			fputs(p, g);
			free(p);
		}
		fputc('\n', g);
	}

	fputc('\n', g);

	while (fgets(buffer, 2048, f)) {
#ifdef CHARSET_CONVERSION
		buffer_to_network(buffer, mmnwcharset);
#endif /* CHARSET_CONVERSION */
		fputs(buffer, g);
		if (!allow_8bit_header) {
			/* see if there are any 8bit chars in the body... */
			for (c = buffer; *c && !isreturn(*c); c++) {
				if (is_EIGHT_BIT(c)) {
					mime_headers_needed = TRUE;
					break;
				}
			}
		}
	}

	rewind(g);
	rewind(f);
#ifdef HAVE_FTRUNCATE
	ftruncate(fileno(f), 0);
#endif /* HAVE_FTRUNCATE */

	/* copy header */
	while (fgets(buffer, 2048, g) && !isreturn(buffer[0]))
		fputs(buffer, f);

	if (!allow_8bit_header) {
		/*
		 * 7bit charsets except US-ASCII also need mime headers
		 */
		for (i = 1; *txt_mime_7bit_charsets[i]; i++) {
#ifdef CHARSET_CONVERSION
			if (!strcasecmp(txt_mime_charsets[mmnwcharset], txt_mime_7bit_charsets[i])) {
				mime_headers_needed = TRUE;
				break;
			}
#else
			if (!strcasecmp(tinrc.mm_charset, txt_mime_7bit_charsets[i])) {
				mime_headers_needed = TRUE;
				break;
			}
#endif /* CHARSET_CONVERSION */
		}

		/*
		 * now add MIME headers as necessary
		 */
		if (mime_headers_needed) {
			if (contains_headers)
				fprintf(f, "MIME-Version: %s\n", MIME_SUPPORTED_VERSION);
#ifdef CHARSET_CONVERSION
			fprintf(f, "Content-Type: text/plain; charset=%s\n", txt_mime_charsets[mmnwcharset]);
#else
			fprintf(f, "Content-Type: text/plain; charset=%s\n", tinrc.mm_charset);
#endif /* CHARSET_CONVERSION */
			fprintf(f, "Content-Transfer-Encoding: %s\n", mime_encoding);
		}
	}
	fputc('\n', f);

	if (!allow_8bit_header) {
		if (!strcasecmp(mime_encoding, txt_base64))
			encoding = 'b';
		else if (!strcasecmp(mime_encoding, txt_quoted_printable))
			encoding = 'q';
		else if (!strcasecmp(mime_encoding, txt_7bit))
			encoding = '7';
		else
			encoding = '8';

		/* avoid break of long lines for US-ASCII/quoted-printable */
		if (!mime_headers_needed)
			encoding = '8';

		body_encode = rfc1521_encode;

		while (fgets(buffer, 2048, g))
			body_encode(buffer, f, encoding);

		if (encoding == 'b' || encoding == 'q' || encoding == '7')
			body_encode(NULL, f, encoding);	/* flush */
	} else {
		while (fgets(buffer, 2048, g))
			fputs(buffer, f);
	}

	fclose(g);
}


void
rfc15211522_encode(
	const char *filename,
	constext * mime_encoding,
	struct t_group *group,
	t_bool allow_8bit_header,
	t_bool ismail)
{
	FILE *fp;

	if ((fp = fopen(filename, "r+")) == NULL)
		return;

	do_rfc15211522_encode(fp, mime_encoding, group, allow_8bit_header, ismail, TRUE);

	fclose(fp);
}


/*
 * Generate a MIME boundary being unique with high probability, consisting
 * of len - 1 random characters.
 * This function is used as a last resort if anything else failed to
 * generate a truly unique boundary.
 */
static void
generate_random_mime_boundary(
	char *boundary,
	size_t len)
{
	size_t i;

	srand((unsigned int) time(NULL));
	for (i = 0; i < len - 1; i++)
		boundary[i] = base64_alphabet[rand() % sizeof(base64_alphabet)];
	boundary[len - 1] = '\0';
}


/*
 * Generate a unique MIME boundary.
 * boundary must have enough space for at least MIME_BOUNDARY_SIZE characters.
 */
static void
generate_mime_boundary(
	char *boundary,
	FILE *f,
	FILE *g)
{
	const char nice_chars[] = { '-', '_', '=' };
	const size_t prefix_len = sizeof(MIME_BOUNDARY_PREFIX) - 1;
	char *s;
	size_t i = 0;
	t_bool unique = FALSE;

	/*
	 * Choose MIME boundary as follows:
	 *   - Always start with MIME_BOUNDARY_PREFIX.
	 *   - Append MIME_BOUNDARY_DEFAULT_PART.
	 *   - If necessary, change it from right to left, choosing from a set of
	 *     `nice_chars' characters.
	 *   - After that, if it is still not unique, replace MIME_BOUNDARY_DEFAULT_PART
	 *     with random characters and hope the best.
	 */

	strcpy(boundary, MIME_BOUNDARY_PREFIX);
	strcat(boundary, MIME_BOUNDARY_DEFAULT_PART);

	s = boundary + MIME_BOUNDARY_SIZE - 2; /* set s to last character before '\0' */
	do {
		/*
		 * Scan for entire boundary in both f and g.
		 * When found: modify and redo.
		 */
		if (contains_string(f, boundary) || contains_string(g, boundary)) {
			*s = nice_chars[i];
			if ((i = (i + 1) % sizeof(nice_chars)) == 0)
				--s;
		} else
			unique = TRUE;
	} while (!unique && s >= boundary + prefix_len);

	if (!unique)
		generate_random_mime_boundary(boundary + prefix_len, sizeof(MIME_BOUNDARY_DEFAULT_PART));
}


/*
 * Split mail into header and (optionally) body.
 *
 * If textfp is not NULL, everything behind the header is stored in it.
 * Whenever an error is encountered, all files are closed and FALSE is returned.
 */
static t_bool
split_mail(
	const char *filename,
	FILE **headerfp,
	FILE **textfp)
{
	FILE *fp;
	char *line;

	if ((fp = fopen(filename, "r")) == NULL)
		return FALSE;

	/* Header */
	if ((*headerfp = tmpfile()) == NULL) {
		fclose(fp);
		return FALSE;
	}

	while ((line = tin_fgets(fp, TRUE))) {
		if (*line == '\0')
			break;
		else
			fprintf(*headerfp, "%s\n", line);
	}

	/* Body */
	if (textfp != NULL) {
		if ((*textfp = tmpfile()) == NULL) {
			fclose(fp);
			fclose(*headerfp);
			return FALSE;
		}

		while ((line = tin_fgets(fp, FALSE)))
			fprintf(*textfp, "%s\n", line);
	}

	fclose(fp);
	return TRUE;
}


/*
 * Compose a mail consisting of a sole text/plain MIME part.
 */
void
compose_mail_text_plain(
	const char *filename,
	struct t_group *group)
{
	rfc15211522_encode(filename, txt_mime_encodings[tinrc.mail_mime_encoding], group, tinrc.mail_8bit_header, TRUE);
}


/*
 * Compose a mail consisting of an optional text/plain and a message/rfc822
 * part.
 *
 * At this point, the file denoted by `filename' contains some common headers
 * and any text the user entered. The file `articlefp' contains the forwarded
 * article in raw form.
 */
void
compose_mail_mime_forwarded(
	const char *filename,
	FILE *articlefp,
	t_bool include_text,
	struct t_group *group)
{
	FILE *fp;
	FILE *headerfp;
	FILE *textfp = NULL;
	FILE *entityfp;
	char *line;
	constext* encoding = txt_mime_encodings[tinrc.mail_mime_encoding];
	t_bool allow_8bit_header = tinrc.mail_8bit_header;
	t_bool _8bit;

	/* Split mail into headers and text */
	if (!split_mail(filename, &headerfp, include_text ? &textfp : NULL))
		return;

	/* Encode header and text */
	rewind(headerfp);
	do_rfc15211522_encode(headerfp, encoding, group, allow_8bit_header, TRUE, TRUE);

	if (textfp) {
		rewind(textfp);
		do_rfc15211522_encode(textfp, encoding, group, allow_8bit_header, TRUE, FALSE);
	}

	/* Compose top-level MIME entity */
	if (include_text)
		entityfp = compose_multipart_mixed(textfp, articlefp);
	else
		entityfp = compose_message_rfc822(articlefp, &_8bit);

	if (entityfp == NULL) {
		fclose(headerfp);
		if (textfp)
			fclose(textfp);
		return;
	}

	/* Put it all together */
	if ((fp = fopen(filename, "w")) == NULL)
		return;

	rewind(headerfp);
	while ((line = tin_fgets(headerfp, TRUE))) {
		if (*line != '\0')
			fprintf(fp, "%s\n", line);
	}
	fprintf(fp, "MIME-Version: %s\n", MIME_SUPPORTED_VERSION);
	rewind(entityfp);
	copy_fp(entityfp, fp);

	/* Clean up */
	fclose(fp);
	fclose(headerfp);
	fclose(entityfp);
	if (textfp)
		fclose(textfp);
}


/*
 * Compose a message/rfc822 MIME entity containing articlefp.
 */
static FILE *
compose_message_rfc822(
	FILE *articlefp,
	t_bool *is_8bit)
{
	FILE *fp;

	if ((fp = tmpfile()) == NULL)
		return NULL;

	*is_8bit = contains_8bit_characters(articlefp);

	/* Header: CT, CD, CTE */
	fprintf(fp, "Content-Type: message/rfc822\n");
	fprintf(fp, "Content-Disposition: inline\n");
	fprintf(fp, "Content-Transfer-Encoding: %s\n", *is_8bit ? txt_8bit : txt_7bit);
	fputc('\n', fp);

	/* Body: articlefp */
	rewind(articlefp);
	copy_fp(articlefp, fp);

	return fp;
}


/*
 * Compose a multipart/mixed MIME entity consisting of a text/plain and a
 * message/rfc822 part.
 */
static FILE *
compose_multipart_mixed(
	FILE *textfp,
	FILE *articlefp)
{
	FILE *fp;
	FILE *messagefp;
	char boundary[MIME_BOUNDARY_SIZE];
	t_bool requires_8bit;

	if ((fp = tmpfile()) == NULL)
		return NULL;

	/* First compose message/rfc822 part (needed for choosing the appropriate CTE) */
	if ((messagefp = compose_message_rfc822(articlefp, &requires_8bit)) == NULL) {
		fclose(fp);
		return NULL;
	}

	requires_8bit = (requires_8bit || contains_8bit_characters(textfp));

	/* Header: CT with multipart boundary, CTE */
	generate_mime_boundary(boundary, textfp, articlefp);
	fprintf(fp, "Content-Type: multipart/mixed; boundary=\"%s\"\n", boundary);
	fprintf(fp, "Content-Transfer-Encoding: %s\n\n", requires_8bit ? txt_8bit : txt_7bit);

	/*
	 * preamble
	 * TODO: -> lang.c
	 */
	fprintf(fp, _("This message has been composed in the 'multipart/mixed' MIME-format. If you\n\
are reading this prefix, your mail reader probably has not yet been modified\n\
to understand the new format, and some of what follows may look strange.\n\n"));

	/*
	 * Body: boundary+text, message/rfc822 part, closing boundary
	 */
	/* text */
	fprintf(fp, "--%s\n", boundary);
	rewind(textfp);
	copy_fp(textfp, fp);
	fputc('\n', fp);

	/* message/rfc822 part */
	fprintf(fp, "--%s\n", boundary);
	rewind(messagefp);
	copy_fp(messagefp, fp);
	fputc('\n', fp);

	/* closing boundary */
	fprintf(fp, "--%s--\n", boundary);
	/* TODO: insert an epilogue here? */

	return fp;
}


/*
 * Determines whether the file denoted by fp contains 8bit characters.
 */
static t_bool
contains_8bit_characters(
	FILE *fp)
{
	char *line;

	rewind(fp);
	while ((line = tin_fgets(fp, FALSE))) {
		for ( ; *line != '\0'; line++) {
			if (is_EIGHT_BIT(line))
				return TRUE;
		}
	}

	return FALSE;
}


/*
 * Determines whether any line of the file denoted by fp contains str.
 */
static t_bool
contains_string(
	FILE *fp,
	const char *str)
{
	char *line;

	rewind(fp);
	while ((line = tin_fgets(fp, FALSE))) {
		if (strstr(line, str))
			return TRUE;
	}

	return FALSE;
}
