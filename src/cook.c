/*
 *  Project   : tin - a Usenet reader
 *  Module    : cook.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-08
 *  Updated   : 2003-04-25
 *  Notes     : Split from page.c
 *
 * Copyright (c) 2000-2003 Jason Faultless <jason@altarstone.com>
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
#ifndef RFC2046_H
#	include "rfc2046.h"
#endif /* !RFC2046_H */

/*
 * We malloc() this many t_lineinfo's at a time
 */
#define CHUNK		50

#define STRIP_ALTERNATIVE(x) \
			(tinrc.alternative_handling && \
			(x)->hdr.ext->type == TYPE_MULTIPART && \
			strcasecmp("alternative", (x)->hdr.ext->subtype) == 0)

#define MATCH_REGEX(x,y,z)	(pcre_exec(x.re, x.extra, y, z, 0, 0, NULL, 0) >= 0)


static int put_rest(char **rest, char *dest, int max_line_len);
static int read_decoded_base64_line(FILE *file, char *line, const int max_line_len, const int max_lines_to_read, char **rest);
static int read_decoded_qp_line(FILE *file, char *line, const int max_line_len, const int max_lines_to_read, char **rest);
static t_bool header_wanted(const char *line);
static t_part *new_uue(t_part **part, char *name);
static void process_text_body_part(t_bool wrap_lines, FILE *in, t_part *part, int hide_uue);
static void put_cooked(t_bool wrap_lines, int flags, const char *fmt, ...);
static void set_rest(char **rest, const char *ptr);
#ifdef DEBUG_ART
	static void dump_cooked(void);
#endif /* DEBUG_ART */


/*
 * These are used globally within this module for access to the context
 * currently being built. They must not leak outside.
 */
static int cook_width;
static t_openartinfo *art;


/*
 * Rewrite frombuf into tobuf to a maximum length
 * Handle backspace, expand tabs, expand control chars to a literal ^[A-Z]
 * Allows \n through
 * Return TRUE if line contains a ^L (form-feed)
 * Anything after the (lenght-2)th character in the output string is
 * dropped, if the resultinglast character is not \n, \n is added
 * afterwards.
 */
t_bool
expand_ctrl_chars(
	char *to,
	const char *from,
	int length,
	size_t lcook_width)
{
	const char *p, *end;
	char *q;
	int i, j, space;
	t_bool ctrl_L = FALSE;

	for (p = from, q = to, end = p, space = length - 2; space && *p; end = p, p++) {
		if (*p == '\t') {			/* Expand tabs */
			i = q - to;
			j = ((i + lcook_width) / lcook_width) * lcook_width;
/*			j = (i | (tabwidth - 1)) + 1; TODO: more efficient? */

			while (space && i++ < j) {
				*q++ = ' ';
				space--;
			}
		} else if (((*p) & 0xFF) < ' ' && *p != '\n' && (!IS_LOCAL_CHARSET("Big5") || *p != 27)) {	/* Literal ctrl chars */
			*q++ = '^';
			if (--space) {
				*q++ = ((*p) & 0xFF) + '@';
				space--;
			}
			if (*p == '\f')					/* ^L detected */
				ctrl_L = TRUE;
		} else {
			*q++ = *p;
			space--;
		}
	}
	if (*end != '\n')
		*q++ = '\n';	/* Force last char of string to be \n */

	*q = '\0';
	return ctrl_L;
}


/*
 * Output text to the cooked stream. Wrap lines as necessary.
 * Update the line count and the array of line offsets
 * Extend the lineoffset array as needed in CHUNK amounts.
 * flags are 'hints' to the pager about line content
 */
static void
put_cooked(
	t_bool wrap_lines,
	int flags,
	const char *fmt,
	...)
{
	char *p, *bufp;
	char buf[LEN];
	int wrap_column;
	int space;
	static int overflow = 0;
	static int saved_flags = 0;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	int bytes;
	wint_t *wp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);

	if (tinrc.wrap_column < 0)
		wrap_column = ((tinrc.wrap_column > -cCOLS) ? cCOLS + tinrc.wrap_column : cCOLS);
	else
		wrap_column = ((tinrc.wrap_column > 0) ? tinrc.wrap_column : cCOLS);

	p = bufp = buf;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wp = my_malloc((MB_CUR_MAX + 1) * sizeof(wint_t));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	while (*p) {
		if (wrap_lines) {
			space = wrap_column;
			while (space > 0 && *p && *p != '\n') {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if ((bytes = mbtowc((wchar_t *) wp, p, MB_CUR_MAX)) > 0) {
					if ((space -= wcwidth(*wp)) < 0)
						break;
					p += bytes;
				} else
					p++;
#else
				p++;
				space--;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}
		} else {
			while (*p && *p != '\n')
				p++;
		}
		fwrite(bufp, 1, p - bufp, art->cooked);
		fputs("\n", art->cooked);
		if (*p == '\n')
			p++;
		bufp = p;
		overflow = 0;

		if (art->cooked_lines == 0) {
			art->cookl = my_malloc(sizeof(t_lineinfo) * CHUNK);
			art->cookl[0].offset = 0;
		}

		/*
		 * Pick up flags from a previous partial write
		 */
		art->cookl[art->cooked_lines].flags = flags | saved_flags;
		saved_flags = 0;
		art->cooked_lines++;

		/*
		 * Grow the array of lines if needed - we resize it properly at the end
		 */
		if (art->cooked_lines % CHUNK == 0)
			art->cookl = my_realloc(art->cookl, sizeof(t_lineinfo) * CHUNK * ((art->cooked_lines / CHUNK) + 1));

		art->cookl[art->cooked_lines].offset = ftell(art->cooked);
	}

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	free(wp);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	/*
	 * If there is anything left over, then it must be a non \n terminated
	 * partial line from base64 decoding etc.. Dump it now and the rest of
	 * the line (with the \n) will fill in the t_lineinfo
	 * We must save the flags now as the rest of the line may not have the same properties
	 * We need to keep the length for accounting purposes
	 */
	if (*bufp != '\0') {
		fputs(bufp, art->cooked);
		saved_flags = flags;
		overflow += strlen(bufp);
	}

	va_end(ap);
}


/*
 * FIXME: should go into rfc1521.c
 *
 * Set everything in ptr as the rest of a physical line to be processed
 * later.
 */
static void
set_rest(
	char **rest,
	const char *ptr)
{
	char *new_rest;
	size_t new_rest_len;

	new_rest_len = strlen(ptr);
	if (new_rest_len) {
		new_rest = my_malloc(new_rest_len + 1);
		strcpy(new_rest, ptr);
		*rest = new_rest;
	} else
		*rest = NULL;	/* no rest anymore */
}


/*
 * FIXME: should go into rfc1521.c
 *
 * Copy things that were left over from the last decoding into the new line.
 * If there's a newline in the rest, copy everything up to and including
 * that newline in the expected buffer, adjust rest and return. If there's
 * no newline in the rest, copy all of it (modulo length of the buffer) to
 * the expected buffer and return.
 *
 * This function returns the number of character written to the line buffer.
 * The buffer is NULL terminated if a complete line was written or the line
 * buffer was filled up to its end. The buffer is NOT NULL terminated if
 * there was no newline in the rest.
 */
static int
put_rest(
	char **rest,
	char *dest,
	int max_line_len)
{
	char *my_rest = *rest;
	char *ptr;
	int put_chars = 0;

	ptr = my_rest;
	if (my_rest) {
		if (strlen(my_rest)) {
			char c;

			while ((c = *ptr++) && (c != '\n') && (put_chars < (max_line_len - 2))) {
				if ((c == '\r') && (*ptr == '\n'))
					continue;	/* step over CRLF */
				*dest++ = c;
				put_chars++;
			}
			if ((c == '\n') || (put_chars >= max_line_len - 2)) {
				/*
				 * FIXME: Adding a newline may be not correct. At least it may
				 * be not what the author of that article intended.
				 * Unfortunately, a newline is expected at the end of a line by
				 * some other code in cook.c and even those functions invoking
				 * this one rely on it.
				 */
				*dest++ = '\n';
				put_chars++;	/* don't count the termining NULL! */
				*dest = '\0';
				if (c != '\n') {
					/*
					 * Line buffer was too small -- "ungetc" last character,
					 * because we didn't put it into the line but left the loop.
					 */
					ptr--;
				}
				set_rest(rest, ptr);
				FreeIfNeeded(my_rest);
			} else {
				/* rest is now empty */
				*rest = NULL;
				FreeIfNeeded(my_rest);
			}
		} else {
			/* empty rest, clean up */
			free(my_rest);
			*rest = NULL;
		}
	}
	return put_chars;
}


/*
 * FIXME: should go into rfc1521.c
 *
 * Read a logical base64 encoded line into the specified line buffer.
 * Logical lines can be split over several physical base64 encoded lines and
 * a single physical base64 encoded line can contain serveral logical lines.
 * This function keeps track of all these cases and always copies only one
 * decoded line to the line buffer.
 *
 * This function returns the number of physical lines read or a negative
 * value on error.
 */
static int
read_decoded_base64_line(
	FILE *file,
	char *line,
	const int max_line_len,
	const int max_lines_to_read,
	char **rest)
{
	char *dest = line;
	char *ptr;
	char c;
	char buf2[HEADER_LEN];	/* holds the entire decoded line */
	char buf[HEADER_LEN];	/* holds the entire encoded line; allowed are 76 char + CRLF (cf. RFC 2045, section 6.8), so "this should be enough for everyone" */
	int count = 0;
	int lines_read = 0;
	int put_chars = 0;

	/*
	 * First of all, catch everything that is left over from the last decoding.
	 * If there's a newline in that rest, copy everything up to and including
	 * that newline in the expected buffer, adjust rest and return. If there's
	 * no newline in the rest, copy all of it (modulo length of the buffer) to
	 * the expected buffer and continue as if there was no rest.
	 */
	put_chars = put_rest(rest, dest, max_line_len);
	if (put_chars && (line[put_chars - 1] == '\n'))
		return 0;	/* we didn't read any new lines but filled the line */
	dest = &line[put_chars];

	/*
	 * At this point, either there was no rest or there was no newline in the
	 * rest. In any case, we need to read further encoded lines and decode
	 * them until we find a newline or the line buffer is filled up
	 * completely or there are no more (encoded or physical) lines in this
	 * part of the posting.
	 */
	/*
	 * max_lines_to_read==0 occurs at end of an encoded part and if there was
	 * no trailing newline in the encoded text. So we put one there and exit.
	 * FIXME: Adding a newline may be not correct. At least it may be not
	 * what the author of that article intended. Unfortunately, a newline is
	 * expected at the end of a line by some other code in cook.c.
	 */
	if (max_lines_to_read <= 0) {
		if (put_chars) {
			*dest++ = '\n';
			*dest++ = '\0';	/* TODO: why do we a post increment here? */
		}
		return max_lines_to_read;
	}
	/*
	 * Ok, now read a new line from the original article.
	 */
	do {
		if (fgets(buf, sizeof(buf), file) == NULL) {
			/*
			 * Premature end of file (or file error), leave loop. To prevent
			 * re-invoking of this function, set the numbers of read lines to
			 * the expected maximum that should be read at most.
			 *
			 * FIXME: Adding a newline may be not correct. At least it may be
			 * not what the author of that article intended. Unfortunately, a
			 * newline is expected at the end of a line by some other code in
			 * cook.c.
			 */
			*dest++ = '\n';
			*dest = '\0';
			return max_lines_to_read;
		}
		lines_read++;
		count = mmdecode(buf, 'b', '\0', buf2);
		buf2[count] = '\0';
		ptr = &buf2[0];
		while ((c = *ptr++) && (c != '\n') && (put_chars < (max_line_len - 2))) {
			if ((c == '\r') && (*ptr == '\n'))
				continue;	/* step over CRLF */
			*dest++ = c;
			put_chars++;
		}
		if ((c == '\n') || (put_chars >= max_line_len - 2)) {
			if (put_chars && (line[put_chars - 1] == '\r') && (c == '\n'))
				dest--;	/* remove CR before LF */
			*dest++ = '\n';
			*dest = '\0';
			if (c != '\n')
				ptr--;
			set_rest(rest, ptr);
			return lines_read;
		}
	} while ((lines_read < max_lines_to_read) && (put_chars < max_line_len));
	set_rest(rest, ptr);	/* should not be needed */
	return lines_read;
}


/*
 * FIXME: should go into rfc1521.c
 *
 * Read a logical quoted-printable encoded line into the specified line
 * buffer. Quoted-printable lines can be split over several physical lines,
 * so this function collects all affected lines, concatenates and decodes
 * them.
 *
 * This function returns the number of physical lines read or a negative
 * value on error.
 */
static int
read_decoded_qp_line(
	FILE *file,
	char *line,							/* where to copy the decoded line */
	const int max_line_len,			/* maximum line length */
	const int max_lines_to_read,	/* don't read more physical lines than told here */
	char **rest)						/* rest of line if line is too small */
{
	char *buf, *buf2, *endptr;
	char *dest = line;
	char *ptr;
	char c;
	int buflen = 0;
	int chars_added = 0;
	int count = 0;
	int lines_read = 0;
	int max_buf_len = 3 * max_line_len;	/* worst case: every char is qp-encoded (3 chars) */
	int put_chars = 0;

	/*
	 * If there was anything left over from the last invokation, put it out
	 * now.
	 */
	put_chars = put_rest(rest, dest, max_line_len);
	if (put_chars && (line[put_chars - 1] == '\n'))
		return 0;	/* we didn't read any new lines but filled the line */

	dest = &line[put_chars];
	if ((max_lines_to_read < 1) || (max_line_len < 2))
		return max_lines_to_read;
	buf = my_malloc((size_t) max_buf_len);
	*buf = '\0';
	endptr = buf;
	do {
		if (fgets(endptr, max_buf_len - strlen(buf), file) == NULL) {
			/*
			 * Premature end of file (or file error), leave loop. To prevent
			 * re-invokation of this function, set the numbers of read lines to
			 * the expected maximum that should be read at most.
			 */
			/*
			 * FIXME: If the last line ended in '=' (soft break), skip that
			 * character because the QP decoder will happily strip the
			 * following newline that put_cooked() relies on. This should
			 * probably be fixed in put_cooked(), not here.
			 */
			if (*endptr == '=')
				endptr--;

			lines_read = max_lines_to_read;
			break;
		}
		lines_read++;
		chars_added = strlen(buf) - (endptr - buf);
		if (!(buflen = strlen(buf)))
			/*
			 * Empty line. Should not occur, at least newline should be there.
			 * We'll add it when we left the loop.
			 */
			break;
		endptr = &buf[buflen - 1];
		buf[buflen] = '\0';
		c = *endptr;
		/*
		 * Strip trailing white space at the end of the line, but no more than
		 * the number of characters that were newly read.
		 * See RFC 2045, section 6.7, #3
		 */
		while ((chars_added > 0) && ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))) {
			c = *--endptr;
			buflen--;
			chars_added--;
		}
		/*
		 * '=' at the end of a line indicates a soft break meaning
		 * that the following physical line "belongs" to this one.
		 * (See RFC 2045, section 6.7, #5)
		 *
		 * Skip that equal sign now; since c holds this char, the
		 * loop is not left but the next line is read and concatenated
		 * with this one while the '=' is overwritten.
		 */
	} while ((c == '=') && (lines_read < max_lines_to_read) && (buflen < max_buf_len));
	/*
	 * re-add newline and NULL termination at end of line
	 * FIXME: Adding a newline may be not correct. At least it may be not
	 * what the author of that article intended. Unfortunately, a newline is
	 * expected at the end of a line by some other code in cook.c.
	 */
	*++endptr = '\n';
	*++endptr = '\0';

	/*
	 * Now decode complete (logical) line from buf to buf2 and copy it to the
	 * buffer where the invoking function expects it. Don't decode directly
	 * to the buffer of the other function to prevent buffer overruns.
	 */
	buf2 = my_malloc(strlen(buf) + 1);
	count = mmdecode(buf, 'q', '\0', buf2);

	if (count >= 0) {
		buf2[count] = '\0';
		ptr = buf2;
	} else	/* error in encoding: copy raw line */
		ptr = buf;

	strncpy(dest, ptr, max_line_len - put_chars - 1);
	line[max_line_len - 1] = '\0'; /* be sure to terminate string */
	if ((signed int) (strlen(ptr) + put_chars) > max_line_len - 1)
		set_rest(rest, &ptr[max_line_len - put_chars - 1]);
	free(buf);
	free(buf2);
	return lines_read;
}


/*
 * Add a new uuencode attachment description to the current part
 */
static t_part *
new_uue(
	t_part **part,
	char *name)
{
	t_part *ptr = new_part((*part)->uue);

	if (!(*part)->uue)			/* new_part() is simple and doesn't attach list heads */
		(*part)->uue = ptr;

	/*
	 * Load the name into the parameter list
	 */
	ptr->params = my_malloc(sizeof(t_param));
	ptr->params->name = my_strdup("name");
	ptr->params->value = my_strdup(str_trim(name));
	ptr->params->next = NULL;

	ptr->encoding = ENCODING_UUE;	/* treat as x-uuencode */

	ptr->offset = ftell(art->raw);
	ptr->depth = (*part)->depth;	/* uue is at the same depth as the envelope */

	/*
	 * If an extension is present, try and add a Content-Type
	 */
	if ((name = strrchr(name, '.')) != NULL)
		lookup_mimetype(name + 1, ptr);

	return ptr;
}


/*
 * Get the suggested filename for an attachment. RFC says Content-Disposition
 * 'filename' supersedes Content-Type 'name'. We must also remove path
 * information.
 */
const char *
get_filename(
	t_param *ptr)
{
	const char *name;
	char *p;

	if (!(name = get_param(ptr, "filename"))) {
		if (!(name = get_param(ptr, "name")))
			return NULL;
	}

	/* TODO: Use base_name()? or at least DIRSEP */
	if (((p = strrchr(name, '/'))) || ((p = strrchr(name, '\\'))))
		return p + 1;

	return name;
}


#define PUT_UUE(part, qualifier_text)					\
	put_cooked(wrap_lines, C_UUE, _(txt_uue),			\
		part->depth * 4, "",							\
		content_types[part->type], part->subtype,		\
		qualifier_text, part->line_count, get_filename(part->params))

#define PUT_ATTACH(part, depth, name, charset) \
	put_cooked(wrap_lines, C_ATTACH, _(txt_attach),	\
		depth, "",	\
		content_types[part->type], part->subtype,	\
		content_encodings[part->encoding],	\
		charset ? _(txt_attach_charset) : "", BlankIfNull(charset),	\
		part->line_count,	\
		name ? _(txt_name) : "", BlankIfNull(name));	\
		\
	if (part->description)	\
		put_cooked(wrap_lines, C_ATTACH,	\
			_(txt_attach_description),	\
			depth, "",	\
			part->description);	\
	put_cooked(wrap_lines, C_ATTACH, "\n")

/*
 * Decodes text bodies, remove sig's, detects uuencoded sections
 */
static void
process_text_body_part(
	t_bool wrap_lines,
	FILE *in,
	t_part *part,
	int hide_uue)
{
	char *rest = NULL;
	char *line, *buf;
	char to[LEN];
	int max_line_len = LEN;
	int flags, len, lines_left;
	int offsets[6];
	int size_offsets = ARRAY_SIZE(offsets);
	t_bool in_sig = FALSE;			/* Set when in sig portion */
	t_bool in_uue = FALSE;			/* Set when in uuencoded section */
	t_bool is_uubody;				/* Set when current line looks like a uuencoded line */
	t_part *curruue = NULL;

	if (part->uue) {				/* These are redone each time we recook/resize etc.. */
		free_parts(part->uue);
		part->uue = NULL;
	}

	line = buf = my_malloc(max_line_len);
	fseek(in, part->offset, SEEK_SET);

	if (part->encoding == ENCODING_BASE64)
		(void) mmdecode(NULL, 'b', 0, NULL);		/* flush */

	lines_left = part->line_count;
	while ((lines_left > 0) || rest) {
		switch (part->encoding) {
			case ENCODING_BASE64:
				lines_left -= read_decoded_base64_line(in, buf, max_line_len, lines_left, &rest);
				line = buf;
				break;

			case ENCODING_QP:
				lines_left -= read_decoded_qp_line(in, buf, max_line_len, lines_left, &rest);
				line = buf;
				break;

			default:
				line = fgets(buf, max_line_len, in);
				lines_left--;
				break;
		}
		if (!(line && strlen(line)))
			break;	/* premature end of file, file error etc. */

		/* convert network to local charset, tex2iso, iso2asc etc. */
		process_charsets(&line, &max_line_len, get_param(part->params, "charset"), tinrc.mm_local_charset, CURR_GROUP.attribute->tex2iso_conv && art->tex2iso);

		/* we might have realloced line, so make sure we use this new buffer */
		buf = line;

		len = (int) strlen(line);

		/*
		 * Detect and skip signatures if necessary
		 */
		if (!in_sig) {
			if (strcmp(line, SIGDASHES) == 0) {
				in_sig = TRUE;
				if (in_uue) {
					in_uue = FALSE;
					if (hide_uue)
						PUT_UUE(curruue, _(txt_incomplete));
				}
			}
		}

		if (in_sig && !tinrc.show_signatures)
			continue;					/* No further processing needed */

		/*
		 * Detect and process uuencoded sections
		 * Look for the start or the end of a uuencoded section
		 */
		if (pcre_exec(uubegin_regex.re, uubegin_regex.extra, line, len, 0, 0, offsets, size_offsets) != PCRE_ERROR_NOMATCH) {
			in_uue = TRUE;
			curruue = new_uue(&part, line + offsets[1]);
			if (hide_uue)
				continue;				/* Don't cook the 'begin' line */
		} else if (strncmp(line, "end\n", 4) == 0) {
			if (in_uue) {
				in_uue = FALSE;
				if (hide_uue) {
					PUT_UUE(curruue, "");
					continue;			/* Don't cook the 'end' line */
				}
			}
		}

		/*
		 * See if this line looks like a uuencoded 'body' line
		 */
		is_uubody = FALSE;

		if (MATCH_REGEX (uubody_regex, line, len)) {
			int sum = (((*line) - ' ') & 077) * 4 / 3;		/* uuencode octet checksum */

			/* sum = 0 in a uubody only on the last line, a single ` */
			if (sum == 0 && len == 1 + 1)			/* +1 for the \n */
				is_uubody = TRUE;
			else if (len == sum + 1 + 1)
				is_uubody = TRUE;
#ifdef DEBUG_ART
			if (debug == 2)
				fprintf(stderr, "%s sum=%d len=%d (%s)\n", bool_unparse(is_uubody), sum, len, line);
#endif /* DEBUG_ART */
		}

		if (in_uue) {
			if (is_uubody)
				curruue->line_count++;
			else {
				if (line[0] == '\n') {		/* Blank line in a uubody - definitely a failure */
					/* fprintf(stderr, "not a uue line while reading a uue body?\n"); */
					in_uue = FALSE;
					if (hide_uue)
						/* don't continue here, so we see the line that 'broke' in_uue */
						PUT_UUE(curruue, _(txt_incomplete));
				}
			}
		} else {
			/*
			 * UUE_ALL = 'Try harder' - we never saw a begin line, but useful
			 * when uue sections are split across > 1 article
			 */
			if (is_uubody && hide_uue == UUE_ALL) {
				char name[] = N_("(continued)");

				curruue = new_uue(&part, name);
				curruue->line_count++;
				in_uue = TRUE;
				continue;
			}
		}

		/*
		 * Skip output if we're hiding uue or the sig
		 */
		if (in_uue && hide_uue)
			continue;						/* No further processing needed */

		flags = in_sig ? C_SIG : C_BODY;

		/*
		 * Don't do any further handling of uue lines - the data is binary after all
		 */
		if (in_uue) {
			put_cooked(wrap_lines, flags, "%s", line);
			continue;
		}

#ifdef HAVE_COLOR
		if (quote_regex3.re) {
			if (MATCH_REGEX(quote_regex3, line, len))
				flags |= C_QUOTE3;
			else if (quote_regex2.re) {
				if (MATCH_REGEX(quote_regex2, line, len))
					flags |= C_QUOTE2;
				else if (quote_regex.re) {
					if (MATCH_REGEX(quote_regex, line, len))
						flags |= C_QUOTE1;
				}
			}
		}
#endif /* HAVE_COLOR */

		if (MATCH_REGEX(url_regex, line, len))
			flags |= C_URL;
		if (MATCH_REGEX(mail_regex, line, len))
			flags |= C_MAIL;
		if (MATCH_REGEX(news_regex, line, len))
			flags |= C_NEWS;

		/*
		 * Basically, c_b2p() does: if (!(my_isprint(*c) || *c==8 || *c==9 || *c==12))
		 * It is only used here
		 * How about if !isprint() && !isctrl() - expand_ctrl_chars is done at display time.
		 * TODO: integrate into expand_ctrl_chars
		 */
		convert_body2printable(line);
		if (expand_ctrl_chars(to, line, sizeof(to) - 1, cook_width))
			flags |= C_CTRLL;				/* Line contains form-feed */

		put_cooked(wrap_lines && (!IS_LOCAL_CHARSET("Big5")), flags, "%s", to);
	} /* while */

	/*
	 * Were we reading uue and ran off the end ?
	 */
	if (in_uue && hide_uue)
		PUT_UUE(curruue, _(txt_incomplete));

	free(line);
}


/*
 * Return TRUE if this header should be printed as per
 * news_headers_to_[not_]display
 */
static t_bool
header_wanted(
	const char *line)
{
	int i;
	t_bool ret = FALSE;

	if (num_headers_to_display && (news_headers_to_display_array[0][0] == '*'))
		ret = TRUE; /* wild do */
	else {
		for (i = 0; i < num_headers_to_display; i++) {
			if (!strncasecmp(line, news_headers_to_display_array[i], strlen(news_headers_to_display_array[i]))) {
				ret = TRUE;
				break;
			}
		}
	}

	if (num_headers_to_not_display && (news_headers_to_not_display_array[0][0] == '*'))
		ret = FALSE; /* wild don't: doesn't make sense! */
	else {
		for (i = 0; i < num_headers_to_not_display; i++) {
			if (!strncasecmp(line, news_headers_to_not_display_array[i], strlen(news_headers_to_not_display_array[i]))) {
				ret = FALSE;
				break;
			}
		}
	}

	return ret;
}


/* #define DEBUG_ART	1 */
#ifdef DEBUG_ART
static void
dump_cooked(
	void)
{
	int i;
	char *line;

	for (i = 0; i < art->cooked_lines; i++) {
		fseek(art->cooked, art->cookl[i].offset, SEEK_SET);
		line = tin_fgets(art->cooked, FALSE);
		fprintf(stderr, "[%3d] %4ld %3x [%s]\n", i, art->cookl[i].offset, art->cookl[i].flags, line);
	}
	fprintf(stderr, "%d lines cooked\n", art->cooked_lines);
}
#endif /* DEBUG_ART */


/*
 * 'cooks' an article, ie, prepare what will actually appear on the screen
 * It is not easy to do this in the same pass as the initial read since
 * boundary conditions for multipart articles make it harder to do on the
 * fly decoding.
 * We could have cooked the headers whilst they were being read but we're
 * trying to keep this simple.
 *
 * Expects:
 *		Fresh article context to write into
 *		parse_uue is set only when the art is opened to create t_parts for
 *		uue sections found, when resizing this is not needed
 *		hide_uue determines the folding of uue sections
 * Handles:
 *		multipart articles
 *		stripping of non text sections if skip_alternative
 *		Q and B decoding of text sections
 *		handling of uuencoded sections
 *		stripping of sigs if !show_signatures
 * Returns:
 *		TRUE on success
 */
t_bool
cook_article(
	t_bool wrap_lines,
	t_openartinfo *artinfo,
	int tabs,
	int hide_uue)
{
	const char *name;
	const char *charset;
	char *line;
	struct t_header *hdr = &artinfo->hdr;
	t_bool header_put = FALSE;

	art = artinfo;				/* Global saves lots of passing artinfo around */
	cook_width = tabs;

	if (!(art->cooked = tmpfile()))
		return FALSE;

	art->cooked_lines = 0;

	rewind(artinfo->raw);

	/*
	 * Put down just the headers we want
	 */
	while ((line = tin_fgets(artinfo->raw, TRUE)) != NULL) {
		if (line[0] == '\0') {				/* End of headers? */
			if (STRIP_ALTERNATIVE(artinfo)) {
				if (header_wanted(_(txt_info_x_conversion_note))) {
					header_put = TRUE;
					put_cooked(wrap_lines, C_HEADER, _(txt_info_x_conversion_note));
				}
			}
			if (header_put)
				put_cooked(TRUE, 0, "\n");		/* put a newline after headers */
			break;
		}

		if (header_wanted(line)) {	/* Put cooked data */
			header_put = TRUE;
			/*
			 * FIXME: don't decode addr-part of From:/Cc:/ etc.pp.
			 */
			put_cooked(wrap_lines, C_HEADER, "%s\n", rfc1522_decode(line));
		}
	}

	if (tin_errno != 0)
		return FALSE;

	/*
	 * Process the attachments in turn, print a neato header, and process/decode
	 * the body if of text type
	 */
	if (hdr->mime && hdr->ext->type == TYPE_MULTIPART) {
		t_part *ptr;

		for (ptr = hdr->ext->next; ptr != NULL; ptr = ptr->next) {
			/*
			 * Ignore non text/plain sections with alternative handling
			 */
			if (STRIP_ALTERNATIVE(artinfo) && !IS_PLAINTEXT(ptr))
				continue;

			name = get_filename(ptr->params);
			if (!strcmp(content_types[ptr->type], "text"))
				charset = get_param(ptr->params, "charset");
			else
				charset = NULL;
			PUT_ATTACH(ptr, ptr->depth * 4, name, charset);

			/* Try to view anything of type text, may need to review this */
			if (IS_PLAINTEXT(ptr))
				process_text_body_part(wrap_lines, artinfo->raw, ptr, hide_uue);
		}
	} else {
		/*
		 * A regular single-body article
		 */
		if (IS_PLAINTEXT(hdr->ext))
			process_text_body_part(wrap_lines, artinfo->raw, hdr->ext, hide_uue);
		else {
			/*
			 * Non-textual main body
			 */
			name = get_filename(hdr->ext->params);
			if (!strcmp(content_types[hdr->ext->type], "text"))
				charset = get_param(hdr->ext->params, "charset");
			else
				charset = NULL;
			PUT_ATTACH(hdr->ext, 0, name, charset);
		}
	}

#ifdef DEBUG_ART
	dump_cooked();
#endif /* DEBUG_ART */

	if (art->cooked_lines > 0)
		art->cookl = my_realloc(art->cookl, sizeof(t_lineinfo) * art->cooked_lines);

	rewind(art->cooked);
	return TRUE;
}
