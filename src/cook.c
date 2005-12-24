/*
 *  Project   : tin - a Usenet reader
 *  Module    : cook.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-08
 *  Updated   : 2005-07-20
 *  Notes     : Split from page.c
 *
 * Copyright (c) 2000-2006 Jason Faultless <jason@altarstone.com>
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
 * We malloc() this many t_lineinfo's at a time
 */
#define CHUNK		50

#define STRIP_ALTERNATIVE(x) \
			(tinrc.alternative_handling && \
			(x)->hdr.ext->type == TYPE_MULTIPART && \
			strcasecmp("alternative", (x)->hdr.ext->subtype) == 0)

#define MATCH_REGEX(x,y,z)	(pcre_exec(x.re, x.extra, y, z, 0, 0, NULL, 0) >= 0)


static t_bool header_wanted(const char *line);
static t_part *new_uue(t_part **part, char *name);
static void process_text_body_part(t_bool wrap_lines, FILE *in, t_part *part, int hide_uue, int tabs);
static void put_cooked(size_t buf_len, t_bool wrap_lines, int flags, const char *fmt, ...);
#ifdef DEBUG_ART
	static void dump_cooked(void);
#endif /* DEBUG_ART */


/*
 * These are used globally within this module for access to the context
 * currently being built. They must not leak outside.
 */
static t_openartinfo *art;


/*
 * Handle backspace, expand tabs, expand control chars to a literal ^[A-Z]
 * Allows \n through
 * Return TRUE if line contains a ^L (form-feed)
 */
t_bool
expand_ctrl_chars(
	char **line,
	int *length,
	size_t lcook_width)
{
	int curr_len = LEN;
	int i = 0, j;
	char *buf = my_malloc(curr_len);
	char *c;
	t_bool ctrl_L = FALSE, resize = FALSE;

	c = *line;
	while (*c) {
		if (resize) {
			curr_len <<= 1;
			buf = my_realloc(buf, curr_len);
			resize = FALSE;
		}
		if (*c == '\t') { /* expand tabs */
/*			j = ((i + lcook_width) / lcook_width) * lcook_width; */
			j = i + lcook_width - (i % lcook_width);
			if (j > curr_len - 2) {
				resize = TRUE;
				continue;
			}
			for (; i < j; i++)
				buf[i] = ' ';
		} else {
			if (((*c) & 0xFF) < ' ' && *c != '\n' && (!IS_LOCAL_CHARSET("Big5") || *c != 27)) {	/* literal ctrl chars */
				if (i > curr_len - 4) {
					resize = TRUE;
					continue;
				}
				buf[i++] = '^';
				buf[i++] = ((*c) & 0xFF) + '@';
				if (*c == '\f')	/* ^L detected */
					ctrl_L = TRUE;
			} else {
				if (i > curr_len - 3) {
					resize = TRUE;
					continue;
				}
				buf[i++] = *c;
			}
		}
		c++;
	}
	/* put_cooked() requires a newline at the end of the line */
	if (buf[i - 1] != '\n')
		buf[i++] = '\n';	/* Force last char of string to be \n */
	buf[i] = '\0';
	*length = i + 1;
	*line = my_realloc(*line, *length);
	strcpy(*line, buf);
	free(buf);
	return ctrl_L;
}


/*
 * Output text to the cooked stream. Wrap lines as necessary.
 * Update the line count and the array of line offsets
 * Extend the lineoffset array as needed in CHUNK amounts.
 * flags are 'hints' to the pager about line content.
 * buf_len is the size put_cooked should use for its buffer.
 */
static void
put_cooked(
	size_t buf_len,
	t_bool wrap_lines,
	int flags,
	const char *fmt,
	...)
{
	char *p, *bufp, *buf;
	int wrap_column;
	int space;
/*	static int overflow = 0; */ /* TODO: we don't use it (anymore?) */
	static int saved_flags = 0;
	va_list ap;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	int bytes;
	wint_t *wp;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	buf = my_malloc(buf_len + 1);

	va_start(ap, fmt);
	vsnprintf(buf, buf_len + 1, fmt, ap);

	if (tinrc.wrap_column < 0)
		wrap_column = ((tinrc.wrap_column > -cCOLS) ? cCOLS + tinrc.wrap_column : cCOLS);
	else
#if 1
		wrap_column = ((tinrc.wrap_column > 0) ? tinrc.wrap_column : cCOLS);
#else	/* never cut off long lines */
		wrap_column = (((tinrc.wrap_column > 0) && (tinrc.wrap_column < cCOLS)) ? tinrc.wrap_column : cCOLS);
#endif /* 1 */

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
/*		overflow = 0; */

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
/*		overflow += strlen(bufp); */
	}

	va_end(ap);
	free(buf);
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

	free_list(ptr->params);
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
	put_cooked(LEN, wrap_lines, C_UUE, _(txt_uue),			\
		part->depth * 4, "",							\
		content_types[part->type], part->subtype,		\
		qualifier_text, part->line_count, get_filename(part->params))

#define PUT_ATTACH(part, depth, name, charset) \
	put_cooked(LEN, wrap_lines, C_ATTACH, _(txt_attach),	\
		depth, "",	\
		content_types[part->type], part->subtype,	\
		content_encodings[part->encoding],	\
		charset ? _(txt_attach_charset) : "", BlankIfNull(charset),	\
		part->line_count,	\
		name ? _(txt_name) : "", BlankIfNull(name));	\
		\
	if (part->description)	\
		put_cooked(LEN, wrap_lines, C_ATTACH,	\
			_(txt_attach_description),	\
			depth, "",	\
			part->description);	\
	put_cooked(1, wrap_lines, C_ATTACH, "\n")

/*
 * Decodes text bodies, remove sig's, detects uuencoded sections
 */
static void
process_text_body_part(
	t_bool wrap_lines,
	FILE *in,
	t_part *part,
	int hide_uue,
	int tabs)
{
	char *rest = NULL;
	char *line = NULL, *buf;
	int max_line_len = 0;
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

	fseek(in, part->offset, SEEK_SET);

	if (part->encoding == ENCODING_BASE64)
		(void) mmdecode(NULL, 'b', 0, NULL);		/* flush */

	lines_left = part->line_count;
	while ((lines_left > 0) || rest) {
		switch (part->encoding) {
			case ENCODING_BASE64:
				lines_left -= read_decoded_base64_line(in, &line, &max_line_len, lines_left, &rest);
				break;

			case ENCODING_QP:
				lines_left -= read_decoded_qp_line(in, &line, &max_line_len, lines_left);
				break;

			default:
				if ((buf = tin_fgets(in, FALSE)) == NULL) {
					FreeAndNull(line);
					break;
				}

				/*
				 * tin_fgets() uses the returned space also internally
				 * so it's not advisable to use it for our own purposes
				 * especially if we must resize it.
				 * So copy buf to line (and resize line if necessary).
				 */
				if (max_line_len < (int) strlen(buf) + 2) {
					max_line_len = strlen(buf) + 2;
					line = my_realloc(line, max_line_len);
				};
				strcpy(line, buf);

				/*
				 * FIXME: Some code in cook.c expects a '\n' at the end
				 * of the line. As tin_fgets() strips trailing '\n', re-add it.
				 * This should problably be fixed in that other code.
				 */
				strcat(line, "\n");

				lines_left--;
				break;
		}
		if (!(line && strlen(line)))
			break;	/* premature end of file, file error etc. */

		/* convert network to local charset, tex2iso, iso2asc etc. */
		process_charsets(&line, &max_line_len, get_param(part->params, "charset"), tinrc.mm_local_charset, curr_group->attribute->tex2iso_conv && art->tex2iso);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (IS_LOCAL_CHARSET("UTF-8"))
			utf8_valid(line);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

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
		 *
		 * TODO: look for a tailing size line after end (non standard
		 *       extension)?
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
				char name[] = N_("(unknown)");

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
			continue;	/* No further processing needed */

		flags = in_sig ? C_SIG : C_BODY;

		/*
		 * Don't do any further handling of uue lines - the data is binary after all
		 */
		if (in_uue) {
			put_cooked(max_line_len, wrap_lines, flags, "%s", line);
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
		if (expand_ctrl_chars(&line, &max_line_len, tabs))
			flags |= C_CTRLL;				/* Line contains form-feed */
		put_cooked(max_line_len, wrap_lines && (!IS_LOCAL_CHARSET("Big5")), flags, "%s", line);
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
	char *line;
	int i;

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
	const char *charset;
	const char *name;
	char *line;
	struct t_header *hdr = &artinfo->hdr;
	t_bool header_put = FALSE;

	art = artinfo;				/* Global saves lots of passing artinfo around */

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
					put_cooked(LEN, wrap_lines, C_HEADER, _(txt_info_x_conversion_note));
				}
			}
			if (header_put)
				put_cooked(1, TRUE, 0, "\n");		/* put a newline after headers */
			break;
		}

		if (header_wanted(line)) {	/* Put cooked data */
			int i = LEN;
			char *l = my_strdup(convert_body2printable(rfc1522_decode(line)));	/* FIXME: don't decode addr-part of From:/Cc:/ etc.pp. */

			header_put = TRUE;
			expand_ctrl_chars(&l, &i, tabs);
			put_cooked(LEN, wrap_lines, C_HEADER, "%s", l);
			free(l);
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
				process_text_body_part(wrap_lines, artinfo->raw, ptr, hide_uue, tabs);
		}
	} else {
		/*
		 * A regular single-body article
		 */
		if (IS_PLAINTEXT(hdr->ext))
			process_text_body_part(wrap_lines, artinfo->raw, hdr->ext, hide_uue, tabs);
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
	return (tin_errno != 0) ? FALSE : TRUE;
}
