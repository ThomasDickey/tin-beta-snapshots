/*
 *  Project   : tin - a Usenet reader
 *  Module    : cook.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-08
 *  Updated   :
 *  Notes     : Split from page.c
 *
 * Copyright (c) 2000 Jason Faultless <jason@radar.tele2.co.uk>
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
 *    This product includes software developed by Jason Faultless
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
#ifndef MENUKEYS_H
#	include  "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef RFC2045_H
#	include  "rfc2045.h"
#endif /* !RFC2045_H */

/*
 * We malloc() this many t_lineinfo's at a time
 */
#define CHUNK		50

#define STRIP_ALTERNATIVE(x) \
			(tinrc.alternative_handling && \
			(x)->hdr.ext->type == TYPE_MULTIPART && \
			strcmp("alternative", (x)->hdr.ext->subtype) == 0)

#define MATCH_REGEX(x,y,z)	(pcre_exec (x.re, x.extra, y, z, 0, 0, NULL, 0) >= 0)

/*
 * These are used globally within this module for access to the context currently
 * being built. They must not leak outside.
 */
int tabwidth;
t_bool hide_uue;
t_openartinfo *art;

/*
 * Rewrite frombuf into tobuf to a maximum length
 * Handle backspace, expand tabs, expand control chars to a literal ^[A-Z]
 * Allows \n through
 * Return TRUE if line contains a ^L (form-feed)
 */
static t_bool
expand_ctrl_chars (
	char *to,
	char *from,
	int length)
{
	char *p, *q;
	t_bool ctrl_L = FALSE;

	for (p = from, q = to; *p && q < &to[length]; p++) {
		if (*p == '\b' && q > to) {			/* Backspace */
			q--;
		} else if (*p == '\t') {			/* Expand tabs */
			int i, j;

			i = q - to;
			j = ((i+tabwidth)/tabwidth) * tabwidth;
/*			j = (i|(tabwidth-1)) + 1; TODO more efficient ? */

			while (i++ < j)
				*q++ = ' ';

		} else if (((*p) & 0xFF) < ' ' && *p != '\n') {	/* Literal ctrl chars */
			*q++ = '^';
			*q++ = ((*p) & 0xFF) + '@';
			if (*p == '\f')					/* ^L detected */
				ctrl_L = TRUE;
		} else
			*q++ = *p;
	}
	*q = '\0';

	return ctrl_L;
}


/*
 * Output text to the cooked stream. Wrap lines as necessary.
 * Update the line count and the array of line offsets
 * Extend the lineoffset array as needed in CHUNK amounts.
 * flags are 'hints' to the pager about line content
 * TODO do word wrap of long lines ?
 */
static void
put_cooked (
	int flags,
	const char *fmt,
	...)
{
	char *p, *bufp;
	char buf[LEN];
	static int overflow = 0;
	static int saved_flags = 0;
	va_list ap;

	va_start(ap, fmt);
	vsprintf (buf, fmt, ap);

	bufp = buf;

	for (p = bufp; *p; p++) {
		if (*p == '\n' || overflow+p-bufp >= cCOLS) {
			fwrite(bufp, p-bufp, 1, art->cooked);

			fputs("\n", art->cooked);
			bufp = p;
			overflow = 0;

			/* Skip over newline if that's what split the current buffer */
			if (*p == '\n')
				++bufp;

			if (art->cooked_lines == 0) {
				art->cookl = my_malloc(sizeof(t_lineinfo)*CHUNK);
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
				art->cookl = my_realloc ((char *)art->cookl, sizeof(t_lineinfo) * CHUNK * ((art->cooked_lines/CHUNK)+1));

			art->cookl[art->cooked_lines].offset = ftell(art->cooked);
		}
	}

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

	va_end (ap);
}


/*
 * Process Q and P encoded text strings
 */
static char *
decode_text_line(
	char *line,
	int encoding,
	const char *charset)
{
	char buf[HEADER_LEN];
	static char buf2[HEADER_LEN];
	int chs;
	t_bool last_char_was_cr = FALSE;	/* TODO - needs to be static ?? */

	chs = mmdecode(line, encoding == ENCODING_BASE64 ? 'b' : 'q', '\0', buf2, charset);
	if (chs >= 0)
		buf2[chs] = '\0';
	else
		strcpy(buf2, line);

	/* TODO reduce the copying around ? */
	if (encoding == ENCODING_BASE64) {
		/*
		 * base64 encoded text has CRLF line endings. Strip off
		 * all CRs if followed by LF but keep it if not followed
		 * by LF. (CR not followed by LF may be a part of valid
		 * encoding for some (multibyte) character sets.)
		 */
		char *src = buf2;
		char *dest = buf;
		char ch;

		if (last_char_was_cr && (buf2[0] != '\n')) {
			*dest++ = '\r';	/* keep CR from last loop if not followed by LF */
			last_char_was_cr = FALSE;
		}

		while ((ch = *src++)) {
			if ((ch == '\r') && (*src == '\n'))
				continue;	/* skip CR if followed by LF */
			if ((ch == '\r') && (*src == '\0')) {
				/*
				 * End of buffer is CR; we don't know what follows
				 * so remember this CR and leave it to next loop to
				 * keep it out or not
				 */
				last_char_was_cr = TRUE;
				break;
			}
			*dest++ = ch;
		}
		*dest = '\0';
		strcpy (buf2, buf);
	}

	if (encoding == ENCODING_BASE64 && last_char_was_cr)
		strcat (buf2, "\r");	/* if one CR was left over, keep it */

	return buf2;
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

	/*
	 * If an extension is present, try and add a Content-Type
	 */
	if ((name = strrchr(name, '.')) != NULL)
		lookup_mimetype (name+1, ptr);

	return ptr;
}


/*
 * Get the suggested filename for an attachment. RFC says Content-Disposition
 * 'filename' supersedes Content-Type 'name'. We must also remove path
 * information.
 */
char *
get_filename(
	t_param *ptr)
{
	char *name, *p;

	if (!(name = get_param(ptr, "filename"))) {
		if (!(name = get_param(ptr, "name")))
		return NULL;
	}

	if (((p = strrchr(name, '/'))) || ((p = strrchr(name, '\\'))))
		return p+1;

	return name;
}


/*
 * Decodes text bodies, remove sig's, detects uuencoded sections
 */
static void
process_text_body_part(
	FILE *in,
	t_part *part)
{
	const char *charset;
	char *line;
	char buf[LEN], to[LEN];
	int flags, i, len;
	t_bool decode = TRUE;
	t_bool in_sig = FALSE;			/* Set when in sig portion */
	t_bool in_uue = FALSE;			/* Set when in uuencoded section */
	t_part *curruue = NULL;

	fseek(in, part->offset, SEEK_SET);

#ifndef LOCAL_CHARSET
	/*
	 * if we have a different local charset, we also convert articles
	 * that do not have MIME headers, since e.g. quoted text may contain
	 * accented chars on non-MIME newsreaders.
	 */
	if (IS_PLAINTEXT(part))
		decode = FALSE;
#endif /* !LOCAL_CHARSET */

	if ((charset = get_param(part->params, "charset")) == NULL)
		decode = FALSE;				/* Impossible in practice, since it defaults */

	if (part->encoding == ENCODING_BASE64)
		(void) mmdecode(NULL, 'b', 0, NULL, NULL);		/* flush */

	for (i=0; i<part->lines; i++) {

		if ((line = fgets (buf, (int) sizeof (buf), in)) == NULL)
			break;					/* Premature end of body ? */

		switch (part->encoding) {
			case ENCODING_BASE64:
			case ENCODING_QP:
				line = decode_text_line (line, part->encoding, charset);
#ifdef LOCAL_CHARSET
				buffer_to_local(line);
#endif /* LOCAL_CHARSET */
				break;

			default:	/* 7bit, 8bit etc.. */
#ifdef LOCAL_CHARSET
				/*
				 * If we have a different local charset, we also have to convert
				 * 8bit articles (and we also convert 7bit articles thay may contain
				 * accented characters due to incorrectly configured newsreaders
				 */
				if (decode)
					buffer_to_local(line);
#endif /* LOCAL_CHARSET */
				break;
		}

		/*
		 * Detect and skip signatures if necessary
		 * Detect and process uuencoded sections
		 * Check quoting depth
		 */
		len = (int)strlen(line);

		if (!in_sig) {
			if (strcmp (line, SIGDASHES) == 0) {
				in_sig = TRUE;
				if (in_uue) {
					in_uue = FALSE;
					put_cooked (C_UUE, txt_uue, "incomplete ", curruue->lines, get_filename(curruue->params));
				}
			}
		}

		if (hide_uue) {
			int offsets[6];
			int size_offsets = sizeof(offsets)/sizeof(int);
			t_bool is_uubody = FALSE;		/* Set if this line looks like a uuencoded line */

			/*
			 * Look for the start or the end of a uuencoded section
			 */
			if (pcre_exec (uubegin_regex.re, uubegin_regex.extra, line, len, 0, 0, offsets, size_offsets) != PCRE_ERROR_NOMATCH) {
				in_uue = TRUE;
				curruue = new_uue(&part, line+offsets[1]);
				continue;
			} else if (strncmp (line, "end\n", 4) == 0) {
				if (in_uue) {
					in_uue = FALSE;
					put_cooked (C_UUE, txt_uue, "", curruue->lines, get_filename(curruue->params));
					continue;				/* To stop 'end' line appearing */
				}
			}

			/*
			 * See if this line looks like a uuencoded line
			 */
			if (MATCH_REGEX (uubody_regex, line, len)) {
				int sum = (((*line) - ' ') & 077) *4/3;		/* uuencode octet checksum */

				/*
				 * sum = 0 in a uubody only on the last line, a single `
				 */
				if (sum == 0 && len == 1+1)			/* +1 for the \n */
					is_uubody = TRUE;
				else if (len == sum+1+1)
					is_uubody = TRUE;
#ifdef DEBUG_ART
fprintf(stderr, "%s sum=%d len=%d (%s)\n", bool_unparse(is_uubody), sum, len, line);
#endif /* DEBUG_ART */
			}

			if (in_uue) {
				if (is_uubody)
					curruue->lines++;
				else {
					if (line[0] == '\n') {		/* Blank line in a uubody - definitely a failure */
#	ifdef DEBUG_ART
						fprintf(stderr, "not a uue line while reading a uue body?\n");
#	endif /* DEBUG_ART */
						in_uue = FALSE;
						put_cooked (C_UUE, txt_uue, "incomplete ", curruue->lines, get_filename(curruue->params));
					}
				}
			} else {
#if 0
				if (is_uubody) {
					char name[] = "(missing)";
					curruue = new_uue(&part, name);
					curruue->lines++;				/* We never saw a begin line */
					in_uue = TRUE;
					continue;
				}
#endif /* 0 */
			}
		}

		if (in_uue || (in_sig && !tinrc.show_signatures))
			continue;

		flags = (in_sig) ? C_SIG : C_BODY;

#ifdef HAVE_COLOR
		if (quote_regex3.re) {
			if (MATCH_REGEX (quote_regex3, line, len))
				flags |= C_QUOTE3;
			else if (quote_regex2.re) {
				if (MATCH_REGEX (quote_regex2, line, len))
					flags |= C_QUOTE2;
				else if (quote_regex.re) {
					if (MATCH_REGEX (quote_regex, line, len))
						flags |= C_QUOTE1;
				}
			}
		}
#endif /* HAVE_COLOR */

		if (MATCH_REGEX (url_regex, line, len))
			flags |= C_URL;
		if (MATCH_REGEX (mail_regex, line, len))
			flags |= C_MAIL;

		if (tex2iso_supported && art->tex2iso) {
			char texbuf[LEN];
			strcpy (texbuf, line);
			ConvertTeX2Iso (texbuf, line);
		}

		if (iso2asc_supported >= 0) {
			char isobuf[LEN];
			strcpy (isobuf, line);
			ConvertIso2Asc (isobuf, line, iso2asc_supported);
		}

#if 0	/* TODO */
/* Basically: if (!(my_isprint(*c) || *c==8 || *c==9 || *c==12)) */
/* How about if !isprint() && !isctrl() - expand_ctrl_chars is done at display time */
		ConvertBody2Printable (line);
#endif /* 0 */
/* TODO integrate above into expand_ctrl_chars */

		if (expand_ctrl_chars (to, line, sizeof(to)))
			flags |= C_CTRLL;				/* Line contains form-feed */

		put_cooked (flags, "%s", to);
	} /* for */

	/*
	 * Were we reading uue and ran off the end ?
	 */
	if (in_uue)
		put_cooked (C_UUE, txt_uue, "incomplete ", curruue->lines, get_filename(curruue->params));
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

	if (num_headers_to_display && (news_headers_to_display_array[0][0] == '*')) {
		ret = TRUE; /* wild do */
	} else {
		for (i = 0; i < num_headers_to_display; i++) {
			if (!strncasecmp (line, news_headers_to_display_array[i], strlen (news_headers_to_display_array[i]))) {
				ret = TRUE;
				break;
			}
		}
	}

	if (num_headers_to_not_display && (news_headers_to_not_display_array[0][0] == '*')) {
		ret = FALSE; /* wild don't: doesn't make sense! */
	} else {
		for (i = 0; i < num_headers_to_not_display; i++) {
			if (!strncasecmp (line, news_headers_to_not_display_array[i], strlen (news_headers_to_not_display_array[i]))) {
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

	for (i=0; i<art->cooked_lines; i++) {
		char *line;

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
 *		Rewound open cooked stream
 *		Fresh article context to write into
 * Handles:
 *		multipart articles
 *		stripping of non text sections if skip_alternative
 *		Q and B decoding of text sections
 *		detection of uuencoded sections
 *		stripping of sigs if !show_signatures
 * Returns:
 *		TRUE on success
 */
t_bool
cook_article(
	t_openartinfo *artinfo,
	int tabs,
	t_bool uue)
{
	char *line;
	int header_put = FALSE;
	struct t_header *hdr = &artinfo->hdr;

	art = artinfo;				/* Global saves lots of passing artinfo around */
	tabwidth = tabs;
	hide_uue = uue;

	if (!(art->cooked = tmpfile()))
		return FALSE;

	art->cooked_lines = 0;

	rewind(artinfo->raw);

	/*
	 * Put down just the headers we want
	 */
	while ((line = tin_fgets (artinfo->raw, TRUE)) != (char *) 0) {
		if (line[0] == '\0') {				/* End of headers ? */
			if (STRIP_ALTERNATIVE(artinfo)) {
				if (header_wanted(_(txt_info_x_conversion_note))) {
					header_put = TRUE;
					put_cooked (C_HEADER, _(txt_info_x_conversion_note));
				}
			}
			if (header_put)
				put_cooked (0, "\n");		/* put a newline after headers */
			break;
		}

		if (header_wanted(line)) {	/* Put cooked data */
			header_put = TRUE;
			put_cooked (C_HEADER, "%s\n", rfc1522_decode(line));
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
			char *name = get_filename(ptr->params);

			/*
			 * Ignore non text/plain sections with alternative handling
			 */
			if (STRIP_ALTERNATIVE(artinfo) && !IS_PLAINTEXT(ptr))
				continue;

			put_cooked (C_ATTACH, _(txt_attach),
				ptr->depth * 4, "",
				content_types[ptr->type], ptr->subtype,
				content_encodings[ptr->encoding], ptr->lines,
				(name)? ", name: " : "", (name) ? name: "");

			/* Try to view anything of type text, may need to review this */
			if (IS_PLAINTEXT(ptr))
				process_text_body_part(artinfo->raw, ptr);
		}
	} else {
		/*
		 * A regular single-body article
		 */
		if (IS_PLAINTEXT(hdr->ext))
			process_text_body_part(artinfo->raw, hdr->ext);
		else {						/* Non-textual main body */
			char *name = get_filename(hdr->ext->params);

			put_cooked (C_ATTACH, _(txt_attach),
					0, "",
					content_types[hdr->ext->type], hdr->ext->subtype,
					content_encodings[hdr->ext->encoding], hdr->ext->lines,
					(name)? ", name: " : "", (name) ? name : "");
		}
	}

#ifdef DEBUG_ART
	dump_cooked();
#endif /* DEBUG_ART */

	art->cookl = my_realloc ((char *)art->cookl, sizeof(t_lineinfo) * art->cooked_lines);

	rewind(art->cooked);

	return TRUE;
}
