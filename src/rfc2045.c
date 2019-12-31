/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2045.c
 *  Author    : Chris Blum <chris@resolution.de>
 *  Created   : 1995-09-01
 *  Updated   : 2003-02-01
 *  Notes     : RFC 2045/2047 encoding
 *
 * Copyright (c) 1995-2003 Chris Blum <chris@resolution.de>
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
static unsigned char bin2hex(unsigned int x);


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
