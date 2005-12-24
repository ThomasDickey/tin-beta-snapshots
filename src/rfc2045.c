/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2045.c
 *  Author    : Chris Blum <chris@resolution.de>
 *  Created   : 1995-09-01
 *  Updated   : 2005-05-10
 *  Notes     : RFC 2045/2047 encoding
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

/*
 * local prototypes
 */
static int put_rest(char **rest, char **line, int *max_line_len, const int offset);
static unsigned char bin2hex(unsigned int x);
static void set_rest(char **rest, const char *ptr);


static unsigned char
bin2hex(
	unsigned int x)
{
	if (x < 10)
		return x + '0';
	return x - 10 + 'A';
}


#define HI4BITS(c) ((unsigned char) (*EIGHT_BIT(c) >> 4))
#define LO4BITS(c) ((unsigned char) (*c & 0xf))

/*
 * A MIME replacement for fputs. e can be 'b' for base64, 'q' for
 * quoted-printable, or 8 (default) for 8bit. Long lines get broken in
 * encoding modes. If line is the null pointer, flush internal buffers.
 * NOTE: Use only with text encodings, because line feed characters (0x0A)
 *       will be encoded as CRLF line endings when using base64! This will
 *       certainly break any binary format ...
 */
void
rfc1521_encode(
	char *line,
	FILE *f,
	int e)
{
	int i;
	static char *b = NULL;	/* they must be static for base64 */
	static char buffer[80];
	static int bits = 0;
	static int xpos = 0;
	static unsigned long pattern = 0;

	if (e == 'b') {
		if (!b) {
			b = buffer;
			*buffer = '\0';
		}
		if (!line) {		/* flush */
			if (bits) {
				if (xpos >= 73) {
					*b++ = '\n';
					*b = 0;
					fputs(buffer, f);
					b = buffer;
					xpos = 0;
				}
				pattern <<= 24 - bits;
				for (i = 0; i < 4; i++) {
					if (bits >= 0) {
						*b++ = base64_alphabet[(pattern & 0xfc0000) >> 18];
						pattern <<= 6;
						bits -= 6;
					} else
						*b++ = '=';
					xpos++;
				}
				pattern = 0;
				bits = 0;
			}
			if (xpos) {
				*b = 0;
				fputs(buffer, f);
				xpos = 0;
			}
			b = NULL;
		} else {
			char *line_crlf = line;
			size_t len = strlen(line);
			char tmpbuf[2050]; /* FIXME: this is sizeof(buffer)+2 from rfc15211522_encode() */

			/*
			 * base64 requires CRLF line endings in text types
			 * convert LF to CRLF if not CRLF already (Windows?)
			 */
			if ((len > 0) && (line[len - 1] == '\n') &&
					((len == 1) || (line[len - 2] != '\r'))) {
				STRCPY(tmpbuf, line);
				line_crlf = tmpbuf;
				line_crlf[len - 1] = '\r';
				line_crlf[len] = '\n';
				line_crlf[len + 1] = '\0';
			}

			while (*line_crlf) {
				pattern <<= 8;
				pattern |= *EIGHT_BIT(line_crlf)++;
				bits += 8;
				if (bits >= 24) {
					if (xpos >= 73) {
						*b++ = '\n';
						*b = 0;
						b = buffer;
						xpos = 0;
						fputs(buffer, f);
					}
					for (i = 0; i < 4; i++) {
						*b++ = base64_alphabet[(pattern >> (bits - 6)) & 0x3f];
						xpos++;
						bits -= 6;
					}
					pattern = 0;
				}
			}
		}
	} else if (e == 'q') {
		if (!line) {
			/*
			 * we don't really flush anything in qp mode, just set
			 * xpos to 0 in case the last line wasn't terminated by
			 * \n.
			 */
			xpos = 0;
			b = NULL;
			return;
		}
		b = buffer;
		while (*line) {
			if (isspace((unsigned char) *line) && *line != '\n') {
				char *l = line + 1;

				while (*l) {
					if (!isspace((unsigned char) *l)) {		/* it's not trailing whitespace, no encoding needed */
						*b++ = *line++;
						xpos++;
						break;
					}
					l++;
				}
				if (!*l) {		/* trailing whitespace must be encoded */
					*b++ = '=';
					*b++ = bin2hex(HI4BITS(line));
					*b++ = bin2hex(LO4BITS(line));
					xpos += 3;
					line++;
				}
			} else if ((!is_EIGHT_BIT(line) && *line != '=')
						  || (*line == '\n')) {
				*b++ = *line++;
				xpos++;
				if (*(line - 1) == '\n')
					break;
			} else {
				*b++ = '=';
				*b++ = bin2hex(HI4BITS(line));
				*b++ = bin2hex(LO4BITS(line));
				xpos += 3;
				line++;
			}
			if (xpos > 72 && *line != '\n') {	/* 72 +3 [worst case] + equal sign = 76 :-) */
				*b++ = '=';		/* break long lines with a 'soft line break' */
				*b++ = '\n';
				*b++ = '\0';
				fputs(buffer, f);
				b = buffer;
				xpos = 0;
			}
		}
		*b = 0;
		if (b != buffer)
			fputs(buffer, f);
		if (b != buffer && b[-1] == '\n')
			xpos = 0;
	} else if (line)
		fputs(line, f);
}


/*
 * Set everything in ptr as the rest of a physical line to be processed
 * later.
 */
static void
set_rest(
	char **rest,
	const char *ptr)
{
	char *old_rest = *rest;

	if (ptr == NULL || strlen(ptr) == 0) {
		FreeAndNull(*rest);
		return;
	}
	*rest = my_strdup(ptr);
	FreeIfNeeded(old_rest);
}


/*
 * Copy things that were left over from the last decoding into the new line.
 * If there's a newline in the rest, copy everything up to and including that
 * newline into the expected buffer, adjust rest and return. If there's no
 * newline in the rest, copy all of it to the expected buffer and return.
 *
 * Side effects: resizes line if necessary, adjusts max_line_len
 * accordingly.
 *
 * This function returns the number of characters written to the line buffer.
 */
static int
put_rest(
	char **rest,
	char **line,
	int *max_line_len,
	const int offset)
{
	char *my_rest = *rest;
	char *ptr;
	char c;
	int put_chars = offset;

	if ((ptr = my_rest) == NULL)
		return put_chars;
	if (strlen(my_rest) == 0) {
		FreeAndNull(*rest);
		return put_chars;
	}

	while ((c = *ptr++) && (c != '\n')) {
		if ((c == '\r') && (*ptr == '\n'))
			continue;	/* step over CRLF */
		/*
		 * Resize line if necessary. Keep in mind that we add LF and \0 later.
		 */
		if (put_chars >= *max_line_len - 2) {
			if (*max_line_len == 0)
				*max_line_len = LEN;
			else
				*max_line_len <<= 1;
			*line = my_realloc(*line, *max_line_len);
		}
		(*line)[put_chars++] = c;
	}
	if (c == '\n') {
		/*
		 * FIXME: Adding a newline may be not correct. At least it may
		 * be not what the author of that article intended.
		 * Unfortunately, a newline is expected at the end of a line by
		 * some other code in cook.c and even those functions invoking
		 * this one rely on it.
		 */
		(*line)[put_chars++] = '\n';
		set_rest(rest, ptr);
	} else /* c == 0 */
		/* rest is now empty */
		FreeAndNull(*rest);

	(*line)[put_chars] = '\0';	/* don't count the termining NULL! */
	return put_chars;
}


/*
 * Read a logical base64 encoded line into the specified line buffer.
 * Logical lines can be split over several physical base64 encoded lines and
 * a single physical base64 encoded line can contain serveral logical lines.
 * This function keeps track of all these cases and always copies only one
 * decoded line to the line buffer.
 *
 * Side effects: resizes line if necessary, adjusts max_line_len
 * accordingly.
 *
 * This function returns the number of physical lines read or a negative
 * value on error.
 */
int
read_decoded_base64_line(
	FILE *file,
	char **line,
	int *max_line_len,
	const int max_lines_to_read,
	char **rest)
{
	char *buf2;	/* holds the entire decoded line */
	char *buf;	/* holds the entire encoded line*/
	int count;
	int lines_read = 0;
	int put_chars;

	/*
	 * First of all, catch everything that is left over from the last decoding.
	 * If there's a newline in that rest, copy everything up to and including
	 * that newline in the expected buffer, adjust rest and return. If there's
	 * no newline in the rest, copy all of it (modulo length of the buffer) to
	 * the expected buffer and continue as if there was no rest.
	 */
	put_chars = put_rest(rest, line, max_line_len, 0);
	if (put_chars && ((*line)[put_chars - 1] == '\n'))
		return 0;	/* we didn't read any new lines but filled the line */

	/*
	 * At this point, either there was no rest or there was no newline in the
	 * rest. In any case, we need to read further encoded lines and decode
	 * them until we find a newline or there are no more (encoded or physical)
	 * lines in this part of the posting. To be sure, now allocate memory for
	 * the output if it wasn't already done.
	 */
	if (*max_line_len == 0) {
		*max_line_len = LEN;
		*line = my_malloc(*max_line_len);
	}

	/*
	 * max_lines_to_read==0 occurs at end of an encoded part and if there was
	 * no trailing newline in the encoded text. So we put one there and exit.
	 * FIXME: Adding a newline may be not correct. At least it may be not
	 * what the author of that article intended. Unfortunately, a newline is
	 * expected at the end of a line by some other code in cook.c.
	 */
	if (max_lines_to_read <= 0) {
		if (put_chars) {
			(*line)[put_chars++] = '\n';
			(*line)[put_chars] = '\0';
		}
		return max_lines_to_read;
	}
	/*
	 * Ok, now read a new line from the original article.
	 */
	do {
		if ((buf = tin_fgets(file, FALSE)) == NULL) {
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
			if (put_chars > *max_line_len - 2) {
				*max_line_len <<= 1;
				*line = my_realloc(*line, *max_line_len);
			}
			(*line)[put_chars++] = '\n';
			(*line)[put_chars] = '\0';
			return max_lines_to_read;
		}
		lines_read++;
		buf2 = my_malloc(strlen(buf) + 1); /* decoded string is always shorter than encoded string, so this is safe */
		count = mmdecode(buf, 'b', '\0', buf2);
		buf2[count] = '\0';
		FreeIfNeeded(*rest);
		*rest = buf2;
		put_chars = put_rest(rest, line, max_line_len, put_chars);
		if (put_chars && ((*line)[put_chars - 1] == '\n')) /* end of logical line reached */
			return lines_read;
	} while (lines_read < max_lines_to_read);
	/*
	 * FIXME: Adding a newline may be not correct. At least it may be
	 * not what the author of that article intended. Unfortunately, a
	 * newline is expected at the end of a line by some other code in
	 * cook.c.
	 */
	if (put_chars > *max_line_len - 2) {
		*max_line_len <<= 1;
		*line = my_realloc(*line, *max_line_len);
	}
	if ((0 == put_chars) || ('\n' != (*line)[put_chars - 1]))
			(*line)[put_chars++] = '\n';
	(*line)[put_chars] = '\0';
	return lines_read;
}


/*
 * Read a logical quoted-printable encoded line into the specified line
 * buffer. Quoted-printable lines can be split over several physical lines,
 * so this function collects all affected lines, concatenates and decodes
 * them.
 *
 * Side effects: resizes line if necessary, adjusts max_line_len
 * accordingly.
 *
 * This function returns the number of physical lines read or a negative
 * value on error.
 */
int
read_decoded_qp_line(
	FILE *file,
	char **line,					/* where to copy the decoded line */
	int *max_line_len,				/* (maximum) line length */
	const int max_lines_to_read)	/* don't read more physical lines than told here */
{
	char *buf, *buf2;
	char *ptr;
	char c;
	int buflen = LEN;
	int count;
	int lines_read = 0;
	size_t chars_to_add;

	buf = my_malloc(buflen); /* initial internal line buffer */
	*buf = '\0';
	do {
		if ((buf2 = tin_fgets(file, FALSE)) == NULL) {
			/*
			 * Premature end of file (or file error, leave loop. To prevent
			 * re-invokation of this function, set the numbers of read lines
			 * to the expected maximum that should be read at most.
			 */
			lines_read = max_lines_to_read;
			break;
		}
		lines_read++;
		if ((chars_to_add = strlen(buf2)) == 0) /* Empty line, leave loop. */
			break;

		/*
		 * Strip trailing white space at the end of the line.
		 * See RFC 2045, section 6.7, #3
		 */
		c = buf2[chars_to_add - 1];
		while ((chars_to_add > 0) && ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))) {
			--chars_to_add;
			c = (chars_to_add > 0 ? buf2[chars_to_add - 1] : '\0');
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
		if (c == '=') /* c is 0 when chars_to_add is 0 so this is safe */
			buf2[--chars_to_add] = '\0';

		/*
		 * Join physical lines to a logical one; keep in mind that a LF is
		 * added afterwards.
		 */
		if (chars_to_add > buflen - strlen(buf) - 2) {
			buflen <<= 1;
			buf = my_realloc(buf, buflen);
		}
		strncat(buf, buf2, buflen);
	} while ((c == '=') && (lines_read < max_lines_to_read));
	/*
	 * re-add newline and NULL termination at end of line
	 * FIXME: Adding a newline may be not correct. At least it may be not
	 * what the author of that article intended. Unfortunately, a newline is
	 * expected at the end of a line by some other code in cook.c.
	 */
	strcat(buf, "\n");

	/*
	 * Now decode complete (logical) line from buf to buf2 and copy it to the
	 * buffer where the invoking function expects it. Don't decode directly
	 * to the buffer of the other function to prevent buffer overruns and to
	 * decide if the encoding was ok.
	 */
	buf2 = my_malloc(strlen(buf) + 1); /* Don't use realloc here, tin_fgets relies on its internal state! */
	count = mmdecode(buf, 'q', '\0', buf2);

	if (count >= 0) {
		buf2[count] = '\0';
		ptr = buf2;
	} else	/* error in encoding: copy raw line */
		ptr = buf;

	if (*max_line_len < (int) strlen(ptr) + 1) {
		*max_line_len = strlen(ptr) + 1;
		*line = my_realloc(*line, *max_line_len);
	}
	strncpy(*line, ptr, *max_line_len);
	(*line)[*max_line_len - 1] = '\0'; /* be sure to terminate string */
	free(buf);
	free(buf2);
	return lines_read;
}
