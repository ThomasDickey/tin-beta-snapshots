/*
 *  Project   : tin - a Usenet reader
 *  Module    : cook.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-08
 *  Updated   : 2025-07-04
 *  Notes     : Split from page.c
 *
 * Copyright (c) 2000-2025 Jason Faultless <jason@altarstone.com>
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
#ifdef HAVE_LIBURIPARSER
#	include <uriparser/Uri.h>
#	define CHECK_URI(r,f)	do { \
		if (MATCH_REGEX(r, line, len)) { \
			offsets = regex_get_ovector_pointer(&r); \
			if ((l = offsets[1] - offsets[0])) { \
				u = my_strndup(line + offsets[0], l); \
				if (*(u + l - 1) == '\n') \
					*(u + l - 1) = '\0'; \
				if (*u) { \
					state.uri = &uri; \
					if (uriParseUriA(&state, u) == URI_SUCCESS) { \
						if (uriNormalizeSyntaxA(&uri) == URI_SUCCESS) \
							flags |= f; \
						uriFreeUriMembersA(&uri); \
					} \
				} \
				FreeAndNull(u); \
			} \
		} \
	} while (0)
#else
#ifdef HAVE_LIBCURL
#	include <curl/curl.h>
#	define CHECK_URI(r,f)	do { \
		if (MATCH_REGEX(r, line, len)) { \
			offsets = regex_get_ovector_pointer(&r); \
			if ((l = offsets[1] - offsets[0])) { \
				u = my_strndup(line + offsets[0], l); \
				if (*(u + l - 1) == '\n') \
					*(u + l - 1) = '\0'; \
				if (*u) { \
					if ((curl = curl_url())) { \
						if (curl_url_set(curl, CURLUPART_URL, u, CURLU_URLENCODE) == CURLUE_OK) { \
							if (curl_url_get(curl, CURLUPART_URL, &nu, 0) == CURLUE_OK) { \
								curl_free(nu); \
								flags |= f; \
							} \
						} \
						curl_url_cleanup(curl); \
					} \
				} \
				FreeAndNull(u); \
			} \
		} \
	} while (0)
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */
/*
 * We malloc() this many t_lineinfo's at a time
 */
#define CHUNK		50

#define TEXT_SECTION			0
#define SIGNATURE_SECTION	(1 << 0)
#define VERBATIM_BEGIN		(1 << 1)
#define VERBATIM_SECTION		(1 << 2)
#define VERBATIM_END		(1 << 3)
#define UUE_BEGIN			(1 << 4)
#define UUE_SECTION			(1 << 5)
#define YENC_SECTION			(1 << 6)
#define SHAR_SECTION		(1 << 7)
#define PGP_KEY_SECTION		(1 << 8)
#define PGP_SIG_SECTION		(1 << 9)
#define PGP_ANY_SECTION		(PGP_KEY_SECTION | PGP_SIG_SECTION)

#define STRIP_ALTERNATIVE(x) \
			(curr_group->attribute->alternative_handling && \
			(x)->hdr.ext->type == TYPE_MULTIPART && \
			strcasecmp("alternative", (x)->hdr.ext->subtype) == 0)


static char *ltobi(unsigned long i);
static struct t_attach_item *add_attach_line_item(struct t_attach_item **item);
static t_bool header_wanted(const char *line);
static t_bool shorten_attach_line(struct t_attach_item *item);
static t_part *new_uue(t_part **part, char *name);
static void process_text_body_part(t_bool wrap_lines, FILE *in, const char *charset, t_part *part, int hide_inline_data);
static void put_attach(t_bool wrap_lines, t_part *part, int depth, enum section_type section, const char *name, const char *charset);
static void put_cooked(size_t buf_len, t_bool wrap_lines, unsigned int flags, const char *fmt, ...);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	static t_bool wexpand_ctrl_chars(wchar_t **wline, size_t *length, size_t lcook_width);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
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
	size_t *length,
	size_t lcook_width)
{
	t_bool ctrl_L = FALSE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wline = char2wchar_t(*line);
	size_t wlen;

	/*
	 * remove the assert() before release
	 * it should help us find problems with wide-char strings
	 * in the development branch
	 */
	assert(((void) "wline must not be NULL", wline != NULL));
	wlen = wcslen(wline) + 1; /* add one to make coverity happy */
	ctrl_L = wexpand_ctrl_chars(&wline, &wlen, lcook_width);
	free(*line);
	*line = wchar_t2char(wline);
	free(wline);
	assert(((void) "line must not be NULL", line != NULL));
	*length = strlen(*line);
#else
	int curr_len = LEN;
	unsigned int i = 0, j, ln = 0;
	char *buf = my_malloc(curr_len);
	unsigned char *c;

	c = (unsigned char *) *line;
	while (*c) {
		if (i > curr_len - (lcook_width + 1)) {
			curr_len <<= 1;
			buf = my_realloc(buf, curr_len);
		}
		if (*c == '\n')
			ln = i + 1;
		if (*c == '\t') { /* expand tabs */
			j = i + lcook_width - ((i - ln) % lcook_width);
			for (; i < j; i++)
				buf[i] = ' ';
		} else if (((*c) & 0xFF) < ' ' && *c != '\n' && (!IS_LOCAL_CHARSET("Big5") || *c != 27)) {	/* literal ctrl chars */
			buf[i++] = '^';
			buf[i++] = ((*c) & 0xFF) + '@';
			if (*c == '\f')		/* ^L detected */
				ctrl_L = TRUE;
		} else {
			if (!my_isprint(*c) && *c != '\n')
				buf[i++] = '?';
			else
				buf[i++] = *c;
		}
		++c;
	}
	buf[i] = '\0';
	*length = i + 1;
	*line = my_realloc(*line, *length);
	strcpy(*line, buf);
	free(buf);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	return ctrl_L;
}


#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
static t_bool
wexpand_ctrl_chars(
	wchar_t **wline,
	size_t *length,
	size_t lcook_width)
{
	size_t cur_len = LEN, i = 0, j, ln = 0;
	wchar_t *wbuf = my_malloc(cur_len * sizeof(wchar_t));
	wchar_t *wc;
	t_bool ctrl_L = FALSE;

	wc = *wline;
	while (*wc) {
		if (i > cur_len - (lcook_width + 1)) {
			cur_len <<= 1;
			wbuf = my_realloc(wbuf, cur_len * sizeof(wchar_t));
		}
		if (*wc == '\n')
			ln = i + 1;
		if (*wc == '\t') {		/* expand_tabs */
			j = i + lcook_width - ((i - ln) % lcook_width);
			for (; i < j; i++)
				wbuf[i] = ' ';
		} else if (*wc < ' ' && *wc != '\n' && (!IS_LOCAL_CHARSET("Big5") || *wc != 27)) {	/* literal ctrl chars */
			wbuf[i++] = '^';
			wbuf[i++] = *wc + '@';
			if (*wc == '\f')	/* ^L detected */
				ctrl_L = TRUE;
		} else {
			if (!iswprint((wint_t) *wc) && *wc != '\n')
				wbuf[i++] = '?';
			else
				wbuf[i++] = *wc;
		}
		++wc;
	}
	wbuf[i] = '\0';
	*length = i + 1;
	*wline = my_realloc(*wline, *length * sizeof(wchar_t));
	wcscpy(*wline, wbuf);
	free(wbuf);
	return ctrl_L;
}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */


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
	unsigned int flags,
	const char *fmt,
	...)
{
	char *p, *bufp, *buf, *last_space;
	int wrap_column;
	int space;
	static unsigned int saved_flags = 0;
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
	else {
		if (tinrc.dont_break_words)
			wrap_column = (((tinrc.wrap_column > 0) && (tinrc.wrap_column < cCOLS)) ? tinrc.wrap_column : cCOLS);
		else
			wrap_column = ((tinrc.wrap_column > 0) ? tinrc.wrap_column : cCOLS);
	}
	p = bufp = buf;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wp = my_malloc((MB_CUR_MAX + 1) * sizeof(wint_t));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (art->cooked_lines == 0) {
		art->cookl = my_malloc(sizeof(t_lineinfo) * CHUNK);
		art->cookl[0].offset = 0;
	}

	while (*p) {
		if (wrap_lines) {
			space = wrap_column;
			last_space = NULL;
			while (space > 0 && *p && *p != '\n') {
				if (tinrc.dont_break_words && !(flags & (C_VERBATIM | C_HEADER)) && (*p == ' ' || *p == '\t'))
					last_space = p;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				if ((bytes = mbtowc((wchar_t *) wp, p, MB_CUR_MAX)) > 0) {
					if ((space -= wcwidth((wchar_t) *wp)) < 0)
						break;
					p += bytes;
				} else
					++p;
#else
				++p;
				--space;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}
			if (tinrc.dont_break_words && !(flags & (C_VERBATIM | C_HEADER)) && space <= 0 && last_space && p > last_space)
				p = last_space + 1;
		} else {
			while (*p && *p != '\n')
				++p;
		}
		fwrite(bufp, 1, (size_t) (p - bufp), art->cooked);
		fputs("\n", art->cooked);
		if (*p == '\n')
			++p;
		bufp = p;

		/*
		 * Pick up flags from a previous partial write
		 */
		art->cookl[art->cooked_lines++].flags = flags | saved_flags;
		saved_flags = 0;

		/*
		 * Grow the array of lines if needed - we resize it properly at the end
		 */
		if (art->cooked_lines % CHUNK == 0)
			art->cookl = my_realloc(art->cookl, sizeof(t_lineinfo) * CHUNK * (size_t) ((art->cooked_lines / CHUNK) + 1));

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
	}

	va_end(ap);
	free(buf);
}


/*
 * Add a new UUE-uuencode attachment description to the current part
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
	ptr->params = new_params();
	ptr->params->name = my_strdup("name");
	ptr->params->value = my_strdup(str_trim(name)); /* TODO: sanitize filename */

	ptr->encoding = ENCODING_UUE;	/* treat as x-uuencode */

	ptr->offset = ftell(art->cooked);
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
 * 'filename' supersedes Content-Type 'name'.
 * In addition to 'filename', RFC 6266 also defines 'filename*'. Both can
 * occur simultaneously. If both are present, 'filename*' should be used.
 * We must also remove path information.
 * If 'filename*' ends with '/' we fall back to 'filename', if that also ends
 * with '/', we fall back to 'name' and if 'name ends with '/' we return NULL.
 */
const char *
get_filename(
	t_param *ptr)
{
	const char *name;
	char *p;

	if ((name = get_param(ptr, "filename*"))) {
		if ((p = strrchr(name, '/')))
			name = *++p ? p : NULL;
	}

	if (!name && (name = get_param(ptr, "filename"))) {
		if ((p = strrchr(name, '/')))
			name = *++p ? p : NULL;
	}

	if (!name && (name = get_param(ptr, "name"))) {
		if ((p = strrchr(name, '/')))
			name = *++p ? p : NULL;
	}

	return name;
}


#define SMALL_LETTER_CONDITIONALS() do { \
		curr->flags |= ATTACH_SHOW_BOTH; \
		if (excl_seen) \
			curr->flags |= ATTACH_OMIT_BOTH; \
		else if (star_seen) \
			curr->flags |= ATTACH_OMIT_DESC; \
	} while (0)

#define CAPITAL_LETTER_CONDITIONALS() do { \
		curr->flags |= ATTACH_SHOW_CONTENT; \
		if (excl_seen) \
			curr->flags |= ATTACH_OMIT_BOTH; \
	} while (0)

#define INSERT_SEP() do { \
		if (curr->prev && (curr->flags & (ATTACH_SHOW_CONTENT | ATTACH_SHOW_BOTH)) && (space_left -= strlen(_(txt_mime_sep)) > 0)) { \
			strcat(attach_line, _(txt_mime_sep)); \
			al_ptr = attach_line + strlen(attach_line); \
		} \
	} while (0)

#define INSERT_SLASH() do { \
		if ((curr->flags & (ATTACH_SHOW_CONTENT | ATTACH_SHOW_BOTH)) && (space_left -= 2 > 0)) { \
			*al_ptr++ = '/'; \
			*al_ptr = '\0'; \
		} \
	} while (0)

#define BUILD_ATTACH_ITEM() do { \
		if (curr->flags & (ATTACH_SHOW_CONTENT | ATTACH_SHOW_BOTH)) { \
			if (curr->flags & ATTACH_SHOW_BOTH) \
				snprintf(buf, blen, curr->description, curr->content); \
			else \
				snprintf(buf, blen, curr->fmt, curr->content); \
			if ((space_left -= strlen(buf) > 0)) \
				strcat(attach_line, buf); \
		} \
		curr = curr->next; \
		al_ptr = attach_line + strlen(attach_line); \
	} while (0)


static struct t_attach_item *
add_attach_line_item(
		struct t_attach_item **item)
{
	struct t_attach_item *curr;
	struct t_attach_item *prev = NULL;

	if (!*item)
		curr = *item = my_malloc(sizeof(struct t_attach_item));
	else {
		curr = (*item)->next;
		prev = *item;
		while (curr) {
			prev = curr;
			curr = curr->next;
		}
		curr = my_malloc(sizeof(struct t_attach_item));
		prev->next = curr;
	}
	curr->content = NULL;
	curr->description = NULL;
	curr->fmt = "%s";
	curr->flags = 0;
	curr->prev = prev;
	curr->next = NULL;
	return curr;
}


static t_bool
shorten_attach_line(
		struct t_attach_item *item)
{
	struct t_attach_item *curr = item;

	while (curr) {
		if ((curr->flags & ATTACH_SHOW_BOTH) && (curr->flags & (ATTACH_OMIT_DESC | ATTACH_OMIT_BOTH))) {
			curr->flags ^= ATTACH_SHOW_BOTH;
			curr->flags |= ATTACH_SHOW_CONTENT;
			return TRUE;
		}
		curr = curr->prev;
	}
	curr = item;
	while (curr) {
		if ((curr->flags & ATTACH_SHOW_CONTENT) && (curr->flags & ATTACH_OMIT_BOTH)) {
			curr->flags ^= ATTACH_SHOW_CONTENT;
			return TRUE;
		}
		curr = curr->prev;
	}
	return FALSE;
}


char *
build_attach_line(
	const t_part *part,
	int depth,
	int max_len,
	enum section_type section,
	const char *name,
	const char *charset)
{
	char *attach_line;
	char *al_ptr;
	char *fmt_ptr;
	char *bytes_str = NULL;
	char *crc_str = NULL;
	char *line_cnt_str = NULL;
	char *part_size_str = NULL;
	char *part_str = NULL;
	char *total_size_str = NULL;
	char *total_str = NULL;
	char *buf;
	char *fmt;
	int i, str_len;
	size_t blen;
	ssize_t space_left;
	struct t_attach_item *curr = NULL;
	struct t_attach_item *items = NULL;
	struct t_attach_item *last;
	t_bool init = TRUE;
	t_bool is_uue = (section == SECTION_UUE_COMPLETE || section == SECTION_UUE_INCOMPLETE);
	t_bool is_yenc = (section >= SECTION_YENC_COMPLETE && section <= SECTION_YENC_CORRUPT);
	t_bool excl_seen = FALSE;
	t_bool star_seen = FALSE;

#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	buf = my_malloc(MB_CUR_MAX * (size_t) (cCOLS + 1));
	blen = MB_CUR_MAX * (size_t) cCOLS;
#	else
	buf = my_malloc(cCOLS + 1);
	blen = (size_t) cCOLS;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (signal_context == cAttachment)
		section = SECTION_ATTACHMENT;

	switch (section) {
		case SECTION_UUE_COMPLETE:
		case SECTION_UUE_INCOMPLETE:
			fmt = tinrc.page_uue_format;
			break;

		case SECTION_YENC_COMPLETE:
		case SECTION_YENC_INCOMPLETE:
		case SECTION_YENC_PARTIAL:
		case SECTION_YENC_CORRUPT:
			fmt = tinrc.page_yenc_format;
			break;

		case SECTION_ATTACHMENT:
			fmt = tinrc.attachment_format;
			break;

		default:
			fmt = tinrc.page_mime_format;
			break;
	}

	fmt_ptr = fmt;
	for (; *fmt_ptr; fmt_ptr++) {
		if (*fmt_ptr != '%' && !(excl_seen || star_seen))
			continue;

		switch (*++fmt_ptr) {
			case '\0':
			case '%':
				break;

			case '!':
				excl_seen = TRUE;
				--fmt_ptr;
				break;

			case '*':
				star_seen = TRUE;
				--fmt_ptr;
				break;

			case 'c':
			case 'C':
				if (charset) {
					curr = add_attach_line_item(&items);
					curr->content = charset;
					curr->description = _(txt_mime_charset);
					if (*fmt_ptr == 'c')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'd':
			case 'D':
				curr = add_attach_line_item(&items);
				if (!line_cnt_str) {
					str_len = snprintf(NULL, 0, "%d", part->line_count);
					line_cnt_str = my_malloc(str_len + 1);
					snprintf(line_cnt_str, str_len + 1, "%d", part->line_count);
				}
				curr->content = line_cnt_str;
				curr->description = P_(txt_mime_line_sp[0], txt_mime_line_sp[1], part->line_count);
				if (*fmt_ptr == 'd')
					SMALL_LETTER_CONDITIONALS();
				else
					CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'e':
			case 'E':
				curr = add_attach_line_item(&items);
				curr->content = content_encodings[part->encoding];
				curr->description = _(txt_mime_encoding);
				if (*fmt_ptr == 'e')
					SMALL_LETTER_CONDITIONALS();
				else
					CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'f':
			case 'F':
				if (is_yenc) {
					curr = add_attach_line_item(&items);
					if (!part_str) {
						str_len = snprintf(NULL, 0, "%d", part->yenc_part);
						part_str = my_malloc(str_len + 1);
						snprintf(part_str, str_len + 1, "%d", part->yenc_part);
					}
					curr->content = part_str;
					curr->description = _(txt_yenc_part);
					curr->flags |= ATTACH_ITEM_IS_YENC_PART;
					if (*fmt_ptr == 'f')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'g':
			case 'G':
				if (is_yenc) {
					curr = add_attach_line_item(&items);
					if (!total_str) {
						str_len = snprintf(NULL, 0, "%d", part->yenc_total);
						total_str = my_malloc(str_len + 1);
						snprintf(total_str, str_len + 1, "%d", part->yenc_total);
					}
					curr->content = total_str;
					curr->description = _(txt_yenc_total);
					curr->flags |= ATTACH_ITEM_IS_YENC_TOTAL;
					if (*fmt_ptr == 'g')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'I':
				if (is_uue || is_yenc) {
					curr = add_attach_line_item(&items);
					switch (section) {
						case SECTION_UUE_COMPLETE:
							curr->content = _(txt_uue_complete);
							break;

						case SECTION_UUE_INCOMPLETE:
							curr->content = _(txt_uue_incomplete);
							break;

						case SECTION_YENC_PARTIAL:
							curr->content = _(txt_yenc_partial);
							break;

						case SECTION_YENC_CORRUPT:
							curr->content = _(txt_yenc_corrupt);
							break;

						case SECTION_YENC_COMPLETE:
							curr->content = _(txt_yenc_complete);
							break;

						case SECTION_YENC_INCOMPLETE:
							curr->content = _(txt_yenc_incomplete);
							break;

						default:
							break;
					}
					CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'l':
			case 'L':
				if (!is_uue && !is_yenc && part->language) {
					curr = add_attach_line_item(&items);
					curr->content = part->language;
					curr->description = _(txt_mime_lang);
					if (*fmt_ptr == 'l')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'n':
			case 'N':
				if (name) {
					curr = add_attach_line_item(&items);
					curr->content = name;
					curr->description = _(txt_mime_name);
					if (*fmt_ptr == 'n')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 's':
			case 'S':
				curr = add_attach_line_item(&items);
				curr->content = part->subtype;
				curr->description = _(txt_mime_content_subtype);
				curr->flags |= ATTACH_ITEM_IS_SUBTYPE;
				if (*fmt_ptr == 's')
					SMALL_LETTER_CONDITIONALS();
				else
					CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 't':
			case 'T':
				curr = add_attach_line_item(&items);
				curr->content = content_types[part->type];
				curr->description = _(txt_mime_content_type);
				curr->flags |= ATTACH_ITEM_IS_TYPE;
				if (*fmt_ptr == 't')
					SMALL_LETTER_CONDITIONALS();
				else
					CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'v':
			case 'V':
				if (is_yenc) {
					curr = add_attach_line_item(&items);
					if (!part_size_str) {
						if (part->yenc_part_size)
							part_size_str = my_strdup(ltobi(part->yenc_part_size));
						else
							part_size_str = my_strdup("?");
					}
					curr->content = part_size_str;
					curr->description = _(txt_yenc_part_size);
					curr->flags |= ATTACH_ITEM_IS_YENC_PART_SIZE;
					if (*fmt_ptr == 'v')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'w':
			case 'W':
				if (is_yenc) {
					curr = add_attach_line_item(&items);
					if (!total_size_str)
						total_size_str = my_strdup(ltobi(part->yenc_total_size));
					curr->content = total_size_str;
					curr->description = _(txt_yenc_total_size);
					curr->flags |= ATTACH_ITEM_IS_YENC_TOTAL_SIZE;
					if (*fmt_ptr == 'w')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'x':
			case 'X':
				if (is_yenc) {
					curr = add_attach_line_item(&items);
					if (!crc_str) {
						str_len = snprintf(NULL, 0, "0x%08lx", (unsigned long) part->yenc_crc);
						crc_str = my_malloc(str_len + 1);
						snprintf(crc_str, str_len + 1, "0x%08lx", (unsigned long) part->yenc_crc);
					}
					curr->content = crc_str;
					curr->description = _(txt_yenc_crc);
					if (*fmt_ptr == 'x')
						SMALL_LETTER_CONDITIONALS();
					else
						CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'z':
			case 'Z':
				curr = add_attach_line_item(&items);
				if (!bytes_str)
					bytes_str = my_strdup(ltobi(part->bytes));
				curr->content = bytes_str;
				curr->description = _(txt_mime_size);
				if (*fmt_ptr == 'z')
					SMALL_LETTER_CONDITIONALS();
				else
					CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			default:
				break;
		}
	}

	last = curr;
	star_seen = excl_seen = FALSE;
	attach_line = my_malloc(LEN);
	space_left = LEN - 2;
	*attach_line = '\0';

	while (space_left > 0 && (init || ((strwidth(attach_line) > max_len && shorten_attach_line(last))))) {
		init = FALSE;
		fmt_ptr = fmt;
		curr = items;
		al_ptr = attach_line;
		for (i = 0; i < depth; i++)
			*al_ptr++ = ' ';

		*al_ptr = '\0';
		for (; *fmt_ptr; fmt_ptr++) {
			if (*fmt_ptr != '%' && !(excl_seen || star_seen)) {
				*al_ptr++ = *fmt_ptr;
				*al_ptr = '\0';
				space_left -= 2;
				continue;
			}
			switch (*++fmt_ptr) {
				case '\0':
					break;

				case '%':
					*al_ptr++ = *fmt_ptr;
					*al_ptr = '\0';
					break;

				case '!':
					excl_seen = TRUE;
					--fmt_ptr;
					break;

				case '*':
					star_seen = TRUE;
					--fmt_ptr;
					break;

				case 'c':
				case 'C':
					if (charset && curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'd':
				case 'D':
					if (curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'l':
				case 'L':
					if (!is_uue && !is_yenc && part->language && curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'n':
				case 'N':
				case 'e':
				case 'E':
					if (curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'f':
				case 'F':
					if (is_yenc && curr) {
						if (curr->prev && (curr->prev->flags & ATTACH_ITEM_IS_YENC_TOTAL))
							INSERT_SLASH();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'g':
				case 'G':
					if (is_yenc && curr) {
						if (curr->prev && (curr->prev->flags & ATTACH_ITEM_IS_YENC_PART))
							INSERT_SLASH();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'I':
					if ((is_uue || is_yenc) && curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 's':
				case 'S':
					if (curr) {
						if (curr->prev && (curr->prev->flags & ATTACH_ITEM_IS_TYPE))
							INSERT_SLASH();
						else
							INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 't':
				case 'T':
					if (curr) {
						if (curr->prev && (curr->prev->flags & ATTACH_ITEM_IS_SUBTYPE))
							INSERT_SLASH();
						else
							INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'v':
				case 'V':
					if (is_yenc && curr) {
						if (curr->prev && (curr->prev->flags & ATTACH_ITEM_IS_YENC_TOTAL_SIZE))
							INSERT_SLASH();
						else
							INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'w':
				case 'W':
					if (is_yenc && curr) {
						if (curr->prev && (curr->prev->flags & ATTACH_ITEM_IS_YENC_PART_SIZE))
							INSERT_SLASH();
						else
							INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'x':
				case 'X':
					if (is_yenc && curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				case 'z':
				case 'Z':
					if (curr) {
						INSERT_SEP();
						BUILD_ATTACH_ITEM();
					}
					excl_seen = star_seen = FALSE;
					break;

				default:
					break;
			}
		}
	}

	FreeIfNeeded(bytes_str);
	FreeIfNeeded(line_cnt_str);
	if (is_yenc) {
		FreeIfNeeded(crc_str);
		FreeIfNeeded(part_size_str);
		FreeIfNeeded(part_str);
		FreeIfNeeded(total_size_str);
		FreeIfNeeded(total_str);
	}

	if (items) {
		while (last) {
			curr = last;
			last = last->prev;
			free(curr);
		}
	}

	free(buf);
	return (attach_line);
}


static void
put_attach(
	t_bool wrap_lines,
	t_part *part,
	int depth,
	enum section_type section,
	const char *name,
	const char *charset)
{
	char *attach_line;
	t_bool is_uue = section == SECTION_UUE_COMPLETE || section == SECTION_UUE_INCOMPLETE;
	t_bool is_yenc = section >= SECTION_YENC_COMPLETE && section <= SECTION_YENC_CORRUPT;

	assert(((void) "put_attach() failed. part == NULL", part != NULL));

	attach_line = build_attach_line(part, depth, cCOLS - 1, section, name, charset);

	if (is_uue || is_yenc)
		put_cooked(LEN, wrap_lines, is_uue ? C_UUE : C_YENC, "%s", attach_line);
	else
		put_cooked(LEN, wrap_lines, C_ATTACH, "%s", attach_line);

	FreeIfNeeded(attach_line);

	if (!is_uue && !is_yenc && part->description)
		put_cooked(LEN, wrap_lines, C_ATTACH, _(txt_mime_description), depth, "", part->description);

	if (part->next != NULL || IS_PLAINTEXT(part)) {
		if (is_uue || is_yenc)
			put_cooked(1, wrap_lines, is_uue ? C_UUE : C_YENC, "\n");
		else
			put_cooked(1, wrap_lines, C_ATTACH, "\n");
	}
}


#define GET_YENC_SECTION_TYPE() do { \
		/* as we don't have the full crc of multiparts we must compare against pcrc if set */ \
		if (y_epcrc) { \
			if (y_ccrc != y_epcrc) { \
				section_type = SECTION_YENC_CORRUPT; \
				break; \
			} \
		} else { \
			if (y_ecrc && (y_ccrc != y_ecrc)) { \
				section_type = SECTION_YENC_CORRUPT; \
				break; \
			} \
		} \
		if (y_part > y_total || y_total > 1) { \
			if (curruue->bytes != y_esize) { \
				section_type = SECTION_YENC_CORRUPT; \
				break; \
			} else { \
				section_type = SECTION_YENC_PARTIAL; \
				break; \
			} \
		} else { \
			if (curruue->bytes != y_bsize || curruue->bytes != y_esize) { \
				section_type = SECTION_YENC_CORRUPT; \
				break; \
			} \
		} \
		section_type = SECTION_YENC_COMPLETE; \
	} while (0)

#define PUT_YENC_HEADER() do { \
		curruue->yenc_part = y_part ? y_part : 1; /* fixup display when no =part was found */ \
		curruue->yenc_total = y_total ? y_total : 1; /* fixup display when no =total was found */ \
		curruue->yenc_part_size = y_pend && (y_pend - y_pbegin) ? y_pend - y_pbegin : y_bsize == y_esize ? y_bsize : 0; \
		curruue->yenc_total_size = y_bsize; \
		curruue->yenc_crc = y_ccrc; \
		put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, section_type, get_filename(curruue->params), content_encodings[curruue->encoding]); \
	} while (0)

#ifdef INLINE_DEBUG_MIME
#	define DEBUG_PUT_YENC_INVALID_ITEM(what, value, where) do { \
		put_cooked(LEN, wrap_lines, C_YENC, "[-- Error: invalid %s \"%s\" in %s --]", (what), (value), (where)); \
	} while (0)
#	define DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM(what, where) do { \
		put_cooked(LEN, wrap_lines, C_YENC, "[-- Error: missing/empty %s parameter in %s --]", (what), (where)); \
	} while (0)
#	define DEBUG_PUT_YENC_UNEXPECTED_ITEM(what) do { \
		put_cooked(LEN, wrap_lines, C_YENC, "[-- Error: unexpected %s --]", (what)); \
	} while (0)
#	define DEBUG_PUT_YENC_CRC32_MISMATCH(computed, expected) do { \
		put_cooked(LEN, wrap_lines, C_YENC, "[-- Error: crc32 mismatch, computed 0x%08lx != 0x%08lx expected --]", (computed), (expected)); \
	} while (0)
#else
#	define DEBUG_PUT_YENC_INVALID_ITEM(what, value, where) do {} while (0)
#	define DEBUG_PUT_YENC_UNEXPECTED_ITEM(what) do {} while (0)
/* #	define DEBUG_PUT_YENC_CRC32_MISMATCH(computed, expected) do {} while (0) */
#endif /* INLINE_DEBUG_MIME */


/*
 * Decodes text bodies, remove sig's, detects uuencoded sections
 */
static void
process_text_body_part(
	t_bool wrap_lines,
	FILE *in,
	const char *charset,
	t_part *part,
	int hide_inline_data)
{
	char *rest = NULL;
	char *line = NULL, *buf;
	char *raw_line = NULL;
	char *y_name = NULL;			/* yenc, filename */
	char *cp;
	char  *ncs;						/* named capture */
	static const char xxenc[] = {	/* xxdecode table */
		0x00, 0x7f, 0x01, 0x7f, 0x7f,
		0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
		0x0a, 0x0b, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
		0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
		0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22,
		0x23, 0x24, 0x25, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c,
		0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
		0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c,
		0x3d, 0x3e, 0x3f };
	int lines_left;
	int y_part = 0;					/* yenc, current part */
	int y_total = 0;				/* yenc, total parts */
	int is_uubody;					/* Set when current line looks like a uuencoded=1/xxencoded=2 line */
	int sum;
	unsigned int flags;
	unsigned int section = TEXT_SECTION;
	unsigned int lines_skipped = 0;
	unsigned long y_bsize = 0L;		/* yenc, size in begin line */
	unsigned long y_esize = 0L;
	unsigned long y_pbegin = 0L;		/* yenc, part start offset */
	unsigned long y_pend = 0L;		/* yenc, part end offset */
	unsigned long lines_hidden = 0L;
	size_t max_line_len = 0, len, raw_len;
	uint32_t y_ccrc = 0L;			/* computed crc */
	t_bool first_line_blank = TRUE;	/* Unset when first non-blank line is reached */
	t_bool put_blank_lines = FALSE;	/* Set when previously skipped lines needs to put */
	t_part *curruue = NULL;
#if defined(HAVE_LIBURIPARSER) || defined(HAVE_LIBCURL)
	REGEX_SIZE *offsets;
	char *u;
	size_t l;
#endif /* HAVE_LIBURIPARSER || HAVE_LIBCURL */
#ifdef HAVE_LIBURIPARSER
	UriUriA uri;
	UriParserStateA state;
#else
#	ifdef HAVE_LIBCURL
	CURLU *curl;
	char *nu;
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */
	unsigned int non_attach_lines = 0;

	if (part->uue) {				/* These are redone each time we recook/resize etc.. */
		free_parts(part->uue);
		part->uue = NULL;
	}

	if (fseek(in, part->offset, SEEK_SET) == -1) { /* should not happen */
#ifdef DEBUG
		/*
		 * TODO: always show to user?
		 *       then use something less technical and move to lang.c
		 */
		perror_message("%s:%d process_text_body_part(fseek(in)) failed", __FILE__, __LINE__);
#endif /* DEBUG */
		return;
	}

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
				if (max_line_len <= strlen(buf) + 1 || !line) {
					max_line_len = strlen(buf) + 1;
					line = my_realloc(line, max_line_len + 1);
				}
				strcpy(line, buf);

				/*
				 * FIXME: Some code in cook.c expects a '\n' at the end
				 * of the line. As tin_fgets() strips trailing '\n', re-add it.
				 * This should probably be fixed in that other code.
				 */
				strcat(line, "\n");

				--lines_left;
				break;
		}
		if (!(line && strlen(line))) {
			FreeIfNeeded(rest);
			break;	/* premature end of file, file error etc. */
		}

		/*
		 * former is_art_tex_encoded(), may have false positives.
		 * unfortinatly we decode on the fly, so we can't validate
		 * via e.g. non blank lines vs. umlaut lines ratio (like
		 * 20:1; typical in german is ~4:1 with lines around 70 chars).
		 */
		if (!art->tex2iso && (section == TEXT_SECTION || section == SIGNATURE_SECTION) && MATCH_REGEX(latex_regex, line, strlen(line)))
			art->tex2iso = TRUE;

		FreeAndNull(raw_line);
		raw_line = my_strdup(line); /* we need the raw data for yenc decoding */
		process_charsets(&line, &max_line_len, charset, tinrc.mm_local_charset, curr_group->attribute->tex2iso_conv && art->tex2iso && !(section & (VERBATIM_SECTION | VERBATIM_BEGIN)));
		len = strlen(line);

		/*
		 * assume that yenc never shows up in encoded articles (cause then
		 * MIME could have been used), lines are between 128 or 130 chars
		 * (incl. '\n') and lines are not all ascii (unlikely for yenc).
		 * we use raw_len as a flag.
		 */
		if ((hide_inline_data & UUE_ALL) && section == TEXT_SECTION && (part->encoding == ENCODING_7BIT || part->encoding == ENCODING_8BIT) && !strcasecmp(charset, "US-ASCII") &&  (*raw_line != '=' || *(raw_line + 1) != 'y')) {
			raw_len = strlen(raw_line);
			/* skip check after 100 yenc-lins; yenc should run till ^=yend or EOF */
			/* TODO: use defines instead of magic numbers */
			if (lines_hidden <= 100 && (raw_len < 128 || raw_len > 130))
				raw_len = 0;
		} else
			raw_len = 0;

		/* look for verbatim marks, set in_verbatim only for lines in between */
		if (curr_group->attribute->verbatim_handling > VERBATIM_NONE) {
			if (section & VERBATIM_BEGIN) {
				section &= ~VERBATIM_BEGIN;
				section |= VERBATIM_SECTION;
			} else {
				if (section == TEXT_SECTION && MATCH_REGEX(verbatim_begin_regex, line, len))
					section = VERBATIM_BEGIN;
			}
			if ((section & VERBATIM_SECTION) && MATCH_REGEX(verbatim_end_regex, line, len)) {
				section &= ~VERBATIM_SECTION;
				section |= VERBATIM_END;
			}
		} /* else section &= ~(VERBATIM_BEGIN | VERBATIM_SECTION | VERBATIM_END); */

		if (!(section & VERBATIM_SECTION)) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			/*
			 * UTF-8 and all ISO-8859 charsets but ISO-8859-11 do have SHY at 0xAD
			 * also KOI8-[ET] and Windows-125[0-8] do have SHY at 0xAD
			 */
			if (strcasecmp(tinrc.mm_local_charset, "ISO-8859-11") && (IS_LOCAL_CHARSET("ISO-8859") || IS_LOCAL_CHARSET("UTF-8")) && curr_group->attribute->suppress_soft_hyphens) {
				remove_soft_hyphens(line);
				len = strlen(line);
			}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

			/*
			 * trim article body and sig (not verbatim blocks):
			 * - skip leading blank lines
			 * - replace multiple blank lines with one empty line
			 * - skip tailing blank lines, keep one if an
			 *   attachment follows
			 */
			if (curr_group->attribute->trim_article_body && !(section & (UUE_SECTION | YENC_SECTION))) {
				/* skip all consecutive blanks */
				for (cp = line; (*cp == ' ') || (*cp == '\t'); cp++)
					;
				if (*cp == '\n') {	/* line was all blank */
					if (section & (SHAR_SECTION | PGP_ANY_SECTION)) {
						++non_attach_lines;
						continue;
					}
					if (lines_left == 0 && (curr_group->attribute->trim_article_body & SKIP_TRAILING)) {
						if (!(part->next == NULL || (STRIP_ALTERNATIVE(art) && !IS_PLAINTEXT(part->next))))
							put_cooked(1, TRUE, (section & SIGNATURE_SECTION) ? C_SIG : C_BODY, "\n");
						continue;
					}
					if (first_line_blank) {
						if (curr_group->attribute->trim_article_body & SKIP_LEADING)
							continue;
					} else if ((curr_group->attribute->trim_article_body & (COMPACT_MULTIPLE | SKIP_TRAILING)) && (!(section & SIGNATURE_SECTION) || curr_group->attribute->show_signatures)) {
						++lines_skipped;
						if (lines_left == 0 && !(curr_group->attribute->trim_article_body & SKIP_TRAILING)) {
							for (; lines_skipped > 0; lines_skipped--)
								put_cooked(1, TRUE, (section & SIGNATURE_SECTION) ? C_SIG : C_BODY, "\n");
						}
						continue;
					}
				} else {	/* line is not blank */
					if (first_line_blank)
						first_line_blank = FALSE;
					if (lines_skipped && (!(section & SIGNATURE_SECTION) || curr_group->attribute->show_signatures)) {
						if (strcmp(line, SIGDASHES) != 0 || curr_group->attribute->show_signatures) {
							if (curr_group->attribute->trim_article_body & COMPACT_MULTIPLE)
								put_cooked(1, TRUE, (section & SIGNATURE_SECTION) ? C_SIG : C_BODY, "\n");
							else
								put_blank_lines = TRUE;
						} else if (!(curr_group->attribute->trim_article_body & SKIP_TRAILING))
							put_blank_lines = TRUE;
						if (put_blank_lines) {
							for (; lines_skipped > 0; lines_skipped--)
								put_cooked(1, TRUE, (section & SIGNATURE_SECTION) ? C_SIG : C_BODY, "\n");
						}
						put_blank_lines = FALSE;
						lines_skipped = 0;
					}
				}
			} /* if (tinrc.trim_article_body...) */

			/*
			 * Detect and skip signatures if necessary
			 */
			if (!(section & SIGNATURE_SECTION) && STRCMPEQ(line, SIGDASHES)) {
				section |= SIGNATURE_SECTION;
				if (section & UUE_SECTION) {
					section &= ~UUE_SECTION;
					if (hide_inline_data & UUE_ALL)
						put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
				}
				if (section & YENC_SECTION) {
					section &= ~YENC_SECTION;
					if (hide_inline_data & UUE_ALL)
						put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_YENC_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
				}
			}

			if ((section & SIGNATURE_SECTION) && !(curr_group->attribute->show_signatures))
				continue;					/* No further processing needed */

			/*
			 * TODO:
			 *       - convert single part yenc data into base64 encoded
			 *         mime attachments on the fly? attachment level could
			 *         make use of it...
			 *       - do the same for shar parts
			 *       - move strings to lang.c (and wrap into _()?)
			 *       - we should use raw_line instead of line (but we
			 *         abuse raw_len as a flag)
			 */
			if (!(section & (SIGNATURE_SECTION | UUE_SECTION | PGP_ANY_SECTION | SHAR_SECTION))) { /* yenc? */
				if (*line == '=' && *(line + 1) == 'y') { /* can't show up in yenc-bodies */
					if (MATCH_REGEX(yencbegin_regex, line, len)) {
#	ifdef INLINE_DEBUG_MIME
						if (section & YENC_SECTION) {
							DEBUG_PUT_YENC_UNEXPECTED_ITEM("=ybegin");
						} else
#	endif /* INLINE_DEBUG_MIME */
							section |= YENC_SECTION;

						ncs = regex_get_substring_by_name(&yencbegin_regex, "y_bsize", line);
						if (ncs && *ncs) {
							y_bsize = strtol(ncs, &cp, 10);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("size", ncs, "=ybegin");
								y_bsize = 0;
							}
						}
#	ifdef INLINE_DEBUG_MIME
						else
							DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("size", "=ybegin");
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						/* part / total are optional */
						ncs = regex_get_substring_by_name(&yencbegin_regex, "y_part", line);
						if (ncs && *ncs) {
							y_part = strtol(ncs, &cp, 10);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("part", ncs, "=ybegin");
								y_part = 0;
							}
						}
						FreeAndNull(ncs);

						ncs = regex_get_substring_by_name(&yencbegin_regex, "y_total", line);
						if (ncs && *ncs) {
							y_total = strtol(ncs, &cp, 10);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("total", ncs, "=ybegin");
								y_total = 0;
							}
						}
						FreeAndNull(ncs);

						ncs = regex_get_substring_by_name(&yencbegin_regex, "y_line", line);
						if (ncs && *ncs) {
#	ifdef INLINE_DEBUG_MIME
							if (!strtol(ncs, &cp, 10) || cp == ncs || *ncs == '-')
								DEBUG_PUT_YENC_INVALID_ITEM("line", ncs, "=ybegin");
#	endif /* INLINE_DEBUG_MIME */
						}
#	ifdef INLINE_DEBUG_MIME
						else
							DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("line", "=ybegin");
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						ncs = regex_get_substring_by_name(&yencbegin_regex, "y_name", line);
						if (ncs && *ncs)
							y_name = my_strdup(ncs);
#	ifdef INLINE_DEBUG_MIME
						else
							DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("name", "=ybegin");
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						if (!y_name)
							y_name = my_strdup(txt_unknown);

						curruue = new_uue(&part, y_name);
						curruue->encoding = ENCODING_YENC;
						FreeAndNull(y_name);
						y_ccrc = tin_crc32(0L, NULL, 0);
					} else if (MATCH_REGEX(yencpart_regex, line, len)) {
						ncs = regex_get_substring_by_name(&yencpart_regex, "y_pbegin", line);
						if (ncs && *ncs) {
							y_pbegin = strtol(ncs, &cp, 10);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("begin", ncs, "=ypart");
								y_pbegin = 0;
							}
						}
#	ifdef INLINE_DEBUG_MIME
						else
							DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("begin", "=ypart");
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						ncs = regex_get_substring_by_name(&yencpart_regex, "y_pend", line);
						if (ncs && *ncs) {
							y_pend = strtol(ncs, &cp, 10);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("end", ncs, "=ypart");
								y_pend = 0;
							}
						}
#	ifdef INLINE_DEBUG_MIME
						else
							DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("end", "=ypart");
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						if (y_pbegin > y_pend)
							y_pend = 0;
						/* guess the # parts if no total= was found */
						if (!y_total) {
							if (y_pend && y_pend < y_bsize)
								y_total = y_bsize / (y_pend - y_pbegin) + ((y_bsize % (y_pend - y_pbegin)) ? 1 : 0);
							else
								y_total = y_part;
						}
#	ifdef INLINE_DEBUG_MIME
						if (!(section & YENC_SECTION))
							DEBUG_PUT_YENC_UNEXPECTED_ITEM("=ypart");
#	endif /* INLINE_DEBUG_MIME */
					} else if (MATCH_REGEX(yencend_regex, line, len)) {
						unsigned long y_ecrc = 0L; /* crc32 mentioned in end line */
						unsigned long y_epcrc = 0L; /* pcrc32 mentioned in end line */

						if (!(section & YENC_SECTION) && lines_hidden) {
							put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], lines_hidden), _(txt_yenc_partial), lines_hidden);
							lines_hidden = 0;
						}

						y_esize = 0L;
						ncs = regex_get_substring_by_name(&yencend_regex, "y_esize", line);
						if (ncs && *ncs) {
							y_esize = strtol(ncs, &cp, 10);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("size", ncs, "=yend");
								y_esize = 0;
							}
						}
#	ifdef INLINE_DEBUG_MIME
						else
							DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("size", "=yend");
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						ncs = regex_get_substring_by_name(&yencend_regex, "y_epcrc", line);
						if (ncs && *ncs) {
							y_epcrc = strtol(ncs, &cp, 16);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("pcrc32", ncs, "=yend");
								y_epcrc = 0L;
							}
						}
#	ifdef INLINE_DEBUG_MIME
						else {
							if (y_part)
								DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("pcrc32", "=yend");
						}
#	endif /* INLINE_DEBUG_MIME */
						FreeAndNull(ncs);

						ncs = regex_get_substring_by_name(&yencend_regex, "y_ecrc", line);
						if (ncs && *ncs) {
							y_ecrc = strtol(ncs, &cp, 16);
							if (cp == ncs || *ncs == '-') {
								DEBUG_PUT_YENC_INVALID_ITEM("crc32", ncs, "=yend");
								y_ecrc = 0L;
							}
						}
#	ifdef INLINE_DEBUG_MIME
						else {
							if (y_part && y_part == y_total)
								DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM("crc32", "=yend");
						}
#	endif /* INLINE_DEBUG_MIME */

						FreeAndNull(ncs);
						if (!(section & YENC_SECTION)) {
							DEBUG_PUT_YENC_UNEXPECTED_ITEM("=yend");
							if ((hide_inline_data & UUE_ALL))
								continue;
						} else {
							enum section_type section_type;

							section &= ~YENC_SECTION;
							GET_YENC_SECTION_TYPE();
							PUT_YENC_HEADER();
#	if defined(INLINE_DEBUG_MIME)
							/* as we don't have the full crc of multiparts we must compare against pcrc if set */
							if (y_epcrc) {
								if (y_ccrc != y_epcrc)
									DEBUG_PUT_YENC_CRC32_MISMATCH((unsigned long) y_ccrc, y_epcrc);
							} else {
								if (y_ecrc && (y_ccrc != y_ecrc))
									DEBUG_PUT_YENC_CRC32_MISMATCH((unsigned long) y_ccrc, y_ecrc);
							}
#	endif /* INLINE_DEBUG_MIME */
							y_bsize = 0L;
							y_total = y_part = 0;
							continue;
						}
					}
#	ifdef INLINE_DEBUG_MIME
					else {
						put_cooked(LEN, wrap_lines, C_YENC, "[-- Error: missing =yend --]");
					}
#	endif /* INLINE_DEBUG_MIME */
				} else {
					if (section & YENC_SECTION) {
						unsigned char ch;
						unsigned char *dl = my_calloc(strlen(raw_line), 1);
						int cnt = 0;

						cp = raw_line;
						while (*cp) {
							if (*cp == '\n' || *cp == '\r')
								break;
							if (*cp == '=') { /* escaped? */
								++cp;
								if (!*cp) /* unexpected EOL */
									break;
								else
									ch = (*cp) - 64 - 42;
							} else
								ch = (*cp) - 42;
							dl[cnt++] = ch;
							++cp;
						}
						curruue->bytes += cnt;
						curruue->line_count++;
						dl[cnt] = '\0';
						y_ccrc = tin_crc32(y_ccrc, dl, cnt);
#if 0
						if (hide_inline_data & UUE_ALL) {
							cp = my_calloc((strlen(raw_line) + 1) * 4 / 3 + 1, 1);

							/* process_charsets(&dl, &cnt, charset, tinrc.mm_local_charset, FALSE); */
							put_cooked(LEN, wrap_lines, C_ATTACH, "%s", dl); /* decoded data */
							str2b64((const char*) dl, cp);
							put_cooked(LEN, wrap_lines, C_ATTACH, "%s", cp); /* quick hack, base64 encoded data, would need proper splitting for MIME */
							free(cp);
						}
#endif /* 0 */
						free(dl);
					}
				}
				if ((section & YENC_SECTION) && (hide_inline_data & UUE_ALL))
					continue;
			}

			/*
			 * Detect and process uuencoded sections
			 * Look for the start or the end of a uuencoded section
			 *
			 * TODO: look for a tailing size line after end (non standard
			 *       extension)?
			 *       do we want to cook uue-parts in signatures?
			 */

			section &= ~UUE_BEGIN;

			if (MATCH_REGEX(uubegin_regex, line, len)) {
				REGEX_SIZE *ovector = regex_get_ovector_pointer(&uubegin_regex);

				if (section & UUE_SECTION) { /* previous uue part incomplete and the current one follows without gap */
					if (hide_inline_data & UUE_ALL)
						put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
				} else
					section |= UUE_SECTION;

				section |= UUE_BEGIN;
				if (*(line + ovector[0] + 5) != '-')
					curruue = new_uue(&part, line + ovector[1]);
				else { /* must be "begin-encoded" */
					cp = my_malloc(strlen(line + ovector[1]));

					if (b642str(line + ovector[1], cp) == -1)
						strcpy(cp, line + ovector[1]);
					curruue = new_uue(&part, cp);
					free(cp);
				}
				if (hide_inline_data & UUE_ALL)
					continue;				/* Don't cook the 'begin' line */
			} else if (STRNCMPEQ(line, "end\n", 4)) {
				if (section & UUE_SECTION) {
					section &= ~UUE_SECTION;
					if (hide_inline_data & UUE_ALL) {
						put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_UUE_COMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
						continue;			/* Don't cook the 'end' line */
					}
				}
			}

			/*
			 * See if this line looks like a uu-or xx-encoded 'body' line
			 */
			is_uubody = 0;

			if (MATCH_REGEX(uubody_regex, line, len)) {
				sum = (((*line) - ' ') & 077) * 4 / 3;		/* uuencode octet checksum */
				/*
				 * sum = 0 in a uubody only on the last line, a single `
				 * +1 for the \n
				 */
				if ((sum == 0 && len == 1 + 1) || len == (unsigned) (sum + 1 + 1))
					is_uubody = 1;

#ifdef DEBUG_ART
				if (debug & DEBUG_MISC)
					fprintf(stderr, "%d sum=%d len=%lu %s", is_uubody, sum, len, line);
#endif /* DEBUG_ART */
			} else {
				if (MATCH_REGEX(xxbody_regex, line, len)) {
					/*
					 * we don't bother to sum != 0x7f as it will be
					 * caught be the check against len (limited via
					 * the regex) anyway
					 */
					if (*line >= '+' && *line <= 'z' && (sum = xxenc[(*line - '+')]) >= 0) {
						sum = sum * 4 / 3;
						if ((sum == 0 && len == 1 + 1) || len == (unsigned) (sum + 1 + 1))
							is_uubody = 2;
					}
#ifdef DEBUG_ART
					else
						sum = 0;

					if (debug & DEBUG_MISC)
						fprintf(stderr, "%d sum=%d len=%lu %s", is_uubody, sum, len, line);
#endif /* DEBUG_ART */
				}
			}

			if (section & UUE_SECTION) {
				if (is_uubody) {
					curruue->line_count++;
					curruue->bytes += len;
				} else {
					if (line[0] == '\n') {		/* Blank line in a uubody - definitely a failure */
						/* fprintf(stderr, "not a uue line while reading a uue body?\n"); */
						section &= ~UUE_SECTION;
						if (hide_inline_data & UUE_ALL)
							/* don't continue here, so we see the line that 'broke' in_uue */
							put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
					} else {
						if (!(section & UUE_BEGIN)) { /* xxencoding or the like */
							curruue->line_count++;
							curruue->bytes += len;
						}
					}
				}
			} else {
				/*
				 * UUE_ALLi = 'Try harder' - we never saw a begin line, but useful
				 * when uue sections are split across > 1 article
				 * requires at least one full uu/xx line to fire
				 */
				if (is_uubody && (hide_inline_data & UUE_ALL) && section == TEXT_SECTION && len == 62) {
					/* _(txt_unknown) cannot be used directly in new_uue() due to str_trim() there */
					char *name = my_strdup(_(txt_unknown));

					section |= UUE_SECTION;
					curruue = new_uue(&part, name);
					curruue->line_count++;
					curruue->bytes += len;
					free(name);
#ifdef INLINE_DEBUG_MIME
					put_cooked(LEN, wrap_lines, C_UUE, "[-- Error: unexpected %s --]", is_uubody == 1 ? "uu-encoced data" : "xx-encoced data");
#endif /* INLINE_DEBUG_MIME */
					continue;
				}
			}

			/*
			 * Skip output if we're hiding uue or the sig
			 */
			if ((section & UUE_SECTION) && (hide_inline_data & UUE_ALL))
				continue;	/* No further processing needed */
		}

		flags = (section & VERBATIM_SECTION) ? C_VERBATIM : (section & SIGNATURE_SECTION) ? C_SIG : C_BODY;

		/*
		 * Don't do any further handling of uue || verbatim lines
		 */
		if (section & (UUE_SECTION | YENC_SECTION)) {
			put_cooked(max_line_len, wrap_lines, flags, "%s", line);
			continue;
		} else if ((section & VERBATIM_BEGIN) && curr_group->attribute->verbatim_handling >= VERBATIM_HIDE_MARKS) {
			if (curr_group->attribute->verbatim_handling == VERBATIM_HIDE_ALL)
				put_cooked(strlen(_(txt_verbatim_block_hidden)), wrap_lines, C_VERBATIM, "%s", _(txt_verbatim_block_hidden));
			continue;
		} else if (section & VERBATIM_SECTION) {
			if (curr_group->attribute->verbatim_handling < VERBATIM_HIDE_ALL) {
				expand_ctrl_chars(&line, &max_line_len, 8);
				put_cooked(max_line_len, wrap_lines, flags, "%s", line);
			}
			continue;
		} else if (section & VERBATIM_END) {
			section &= ~VERBATIM_END;
			if (curr_group->attribute->verbatim_handling >= VERBATIM_HIDE_MARKS)
				continue;
		}

#ifdef HAVE_COLOR
		/* keep order in sync with color.c:draw_pager_line() */
		if (quote_regex3.re) {
			if (MATCH_REGEX(quote_regex3, line, len))
				flags |= C_QUOTE3;
			else if (quote_regex2.re) {
				if (MATCH_REGEX(quote_regex2, line, len))
					flags |= C_QUOTE2;
				else if (curr_group->attribute->extquote_handling && extquote_regex.re) {
					if (MATCH_REGEX(extquote_regex, line, len))
						flags |= C_EXTQUOTE;
					else if (quote_regex.re) {
						if (MATCH_REGEX(quote_regex, line, len))
							flags |= C_QUOTE1;
					}
				} else if (quote_regex.re) {
					if (MATCH_REGEX(quote_regex, line, len))
						flags |= C_QUOTE1;
				}
			}
		}
#endif /* HAVE_COLOR */

#ifdef HAVE_LIBURIPARSER
		/* find and validate URIs */
		CHECK_URI(url_regex, C_URL);
		CHECK_URI(mail_regex, C_MAIL);
		CHECK_URI(news_regex, C_NEWS);
#else
#	ifdef HAVE_LIBCURL
		/* find and validate URIs */
		CHECK_URI(url_regex, C_URL);
		CHECK_URI(mail_regex, C_MAIL);
		CHECK_URI(news_regex, C_NEWS);
#	else
		if (MATCH_REGEX(url_regex, line, len))
			flags |= C_URL;
		if (MATCH_REGEX(mail_regex, line, len))
			flags |= C_MAIL;
		if (MATCH_REGEX(news_regex, line, len))
			flags |= C_NEWS;
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */

		if (expand_ctrl_chars(&line, &max_line_len, tabwidth))
			flags |= C_CTRLL;				/* Line contains form-feed */

		buf = line;

		/*
		 * Skip over the first space in case of Format=Flowed
		 * (RFC 3676 4.4 Space-Stuffing)
		 * #if 1
		 * _except_ for lines starting with " ", " >" or " From "
		 * (this is NOT what the RFC says).
		 * #endif
		 * quote depth adjustment, 1st level only.
		 */
		if (part->format == FORMAT_FLOWED && line[0] == ' ') {
			if (line[1] == '>' && (flags & C_QUOTE1))
				flags &= ~C_QUOTE1;

#if 0
			if (line[1] != '>' && line[1] != ' ' && strncmp(line + 1, "From ", 5))
#endif /* 0 */
				++buf;
		}

		/*
		 * skip over what "looks" like an unexpected yenc line.
		 * in contrast to partial uue we don't store the data
		 */
		if (raw_len) {
#ifdef HAVE_COLOR
			flags &= ~(C_CTRLL | C_QUOTE1 | C_QUOTE2 | C_QUOTE3 | C_EXTQUOTE);
#else
			flags &= ~(C_CTRLL | C_QUOTE1 | C_QUOTE2 | C_QUOTE3);
#endif /* HAVE_COLOR */
			if (flags == C_BODY && (*line != '=' || *(line + 1) != 'y') && (strcmp(raw_line, line) || lines_hidden > 100)) {
				if (!(section & (SHAR_SECTION | PGP_ANY_SECTION))) {
					++lines_hidden;
					continue;
				}
			}
		}

		/* TODO: shall we also look for PGP_PRIVATE_KEY_TAG? */
		if ((hide_inline_data & PGP_ALL) && section == TEXT_SECTION && !strcmp(line, PGP_PUBLIC_KEY_TAG)) {
			section |= PGP_KEY_SECTION;
			++non_attach_lines;
			continue;
		}
		if ((hide_inline_data & PGP_ALL) && section == PGP_KEY_SECTION) {
			if (!strcmp(line, PGP_PUBLIC_KEY_END_TAG)) { /* see above, PGP_PRIVATE_KEY_END_TAG */
				++non_attach_lines;
				section &= ~PGP_KEY_SECTION;
				/* -> lang.c && _("OpenPGP public key block") */
				put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], non_attach_lines), "OpenPGP pubic key block", non_attach_lines);
				non_attach_lines = 0;
				continue;
			} else
				++non_attach_lines;
			continue;
		}

		if ((hide_inline_data & PGP_ALL) && section == TEXT_SECTION && !strcmp(line, PGP_SIG_TAG)) {
			section |= PGP_SIG_SECTION;
			++non_attach_lines;
			continue;
		}
		if ((hide_inline_data & PGP_ALL) && section == PGP_SIG_SECTION) {
			if (!strcmp(line, PGP_SIG_END_TAG)) {
				++non_attach_lines;
				section &= ~PGP_SIG_SECTION;
				/* -> lang.c && _("OpenPGP signature block") */
				put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], non_attach_lines), "OpenPGP signature block", non_attach_lines);
				non_attach_lines = 0;
				continue;
			} else
				++non_attach_lines;
			continue;
		}

		if ((hide_inline_data & SHAR_ALL) && section == SHAR_SECTION) {
			++non_attach_lines;
			if (MATCH_REGEX(shar_end_regex, line, len)) {
				/* -> lang.c && _("shar archive") */
				put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], non_attach_lines), "shar archive", non_attach_lines);
				non_attach_lines = 0;
				section &= ~SHAR_SECTION;
			}
			continue;
		} else {
			if ((hide_inline_data & SHAR_ALL) && section == TEXT_SECTION && MATCH_REGEX(shar_regex, line, len)) {
				++non_attach_lines;
				section |= SHAR_SECTION;
				continue;
			}
		}

		/*
		 * TODO: add a key on/off toggle (e.g. '{' or '}')
		 *
		 * be aware that trim_article_body may have kicked in before!
		 */
		if (MATCH_REGEX(hideline_regex, buf, len))
			continue;

		put_cooked(max_line_len, wrap_lines && (!IS_LOCAL_CHARSET("Big5")), flags, "%s", buf);
	} /* while */

	/*
	 * Were we reading uue/yenc and ran off the end ?
	 */
	if ((section & UUE_SECTION) && (hide_inline_data & UUE_ALL))
		put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
	else
		if ((section & YENC_SECTION) && hide_inline_data) {
			curruue->yenc_part = y_part ? y_part : 1; /* fixup display when no =part was found */
			curruue->yenc_total = y_total ? y_total : 1; /* fixup display when no =total was found */
			curruue->yenc_part_size = y_pend && (y_pend - y_pbegin) ? y_pend - y_pbegin : y_bsize == y_esize ? y_bsize : 0;
			curruue->yenc_total_size = y_bsize;
			curruue->yenc_crc = y_ccrc;
			put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, SECTION_YENC_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
			FreeAndNull(y_name);
		}

	if (!(section & YENC_SECTION) && lines_hidden && (hide_inline_data & UUE_ALL))
		put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], lines_hidden), _(txt_yenc_partial), lines_hidden);

	if (section != TEXT_SECTION && non_attach_lines) {
		if ((section & SHAR_SECTION) && (hide_inline_data & SHAR_ALL))
			put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], non_attach_lines), "shar archive", non_attach_lines);

		if ((section & PGP_KEY_SECTION) && (hide_inline_data & PGP_ALL))
			put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], non_attach_lines), "OpenPGP public key block", non_attach_lines);

		if ((section & PGP_SIG_SECTION) && (hide_inline_data & PGP_ALL))
			put_cooked(LEN, wrap_lines, C_BODY, P_(txt_cook_lines_hidden_sp[0], txt_cook_lines_hidden_sp[1], non_attach_lines), "OpenPGP signature block", non_attach_lines);

#ifdef INLINE_DEBUG_MIME
		put_cooked(LEN, wrap_lines, C_BODY, "[-- Error: unexpected %s --]", "end. No mark found.");
#endif /* INLINE_DEBUG_MIME */
	}

	free(line);
	FreeIfNeeded(raw_line);
}


#undef GET_YENC_SECTION_TYPE
#undef PUT_YENC_HEADER
#undef DEBUG_PUT_YENC_INVALID_ITEM
#undef DEBUG_PUT_YENC_MISSING_OR_EMPTY_PARAM
#undef DEBUG_PUT_YENC_UNEXPECTED_ITEM
#undef DEBUG_PUT_YENC_CRC32_MISMATCH


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

	if (curr_group->attribute->headers_to_display->num && (curr_group->attribute->headers_to_display->header[0][0] == '*'))
		ret = TRUE; /* wild do */
	else {
		for (i = 0; i < curr_group->attribute->headers_to_display->num; i++) {
			if (!strncasecmp(line, curr_group->attribute->headers_to_display->header[i], strlen(curr_group->attribute->headers_to_display->header[i]))) {
				ret = TRUE;
				break;
			}
		}
	}

	if (curr_group->attribute->headers_to_not_display->num && (curr_group->attribute->headers_to_not_display->header[0][0] == '*'))
		ret = FALSE; /* wild don't: doesn't make sense! */
	else {
		for (i = 0; i < curr_group->attribute->headers_to_not_display->num; i++) {
			if (!strncasecmp(line, curr_group->attribute->headers_to_not_display->header[i], strlen(curr_group->attribute->headers_to_not_display->header[i]))) {
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


/* set INLINE_DEBUG_MIME to 1 to inline report some errors/warnings,
 * use PAGE_ARTICLE_INFO ('\'') for a more detailed info.
 * strings -> lang.c?
 */
t_bool
cook_article(
	t_bool wrap_lines,
	t_openartinfo *artinfo,
	int hide_inline_data,
	t_bool show_all_headers)
{
	const char *charset;
	const char *name;
	char *line;
	struct t_header *hdr = &artinfo->hdr;
	static const char *struct_header[] = { /* as in rfc2047.c:rfc1522_do_encode() */
		"Approved: ", "From: ", "Originator: ", "Reply-To: ",
		"Sender: ", "To: ", "Cc: ", "Bcc: ", "X-Cancelled-By: ",
		"X-Comment-To: ", "X-Submissions-To: ", "X-Originator: ",
		NULL };
	t_bool header_put = FALSE;
#ifdef INLINE_DEBUG_MIME
	t_bool stripped = FALSE;
#endif /* INLINE_DEBUG_MIME */

	art = artinfo;				/* Global saves lots of passing artinfo around */

	if (!(art->cooked = my_tmpfile()))
		return FALSE;

	art->cooked_lines = 0;

	rewind(artinfo->raw);

	/*
	 * Put down just the headers we want
	 */
	while ((line = tin_fgets(artinfo->raw, TRUE)) != NULL) {
		if (!*line) {				/* End of headers? */
			if (STRIP_ALTERNATIVE(artinfo)) {
#ifdef INLINE_DEBUG_MIME
				stripped = TRUE;
#endif /* INLINE_DEBUG_MIME */
				if (header_wanted(_(txt_info_x_conversion_note))) {
					header_put = TRUE;
					put_cooked(LEN, wrap_lines, C_HEADER, _(txt_info_x_conversion_note));
				}
			}
			if (header_put) {
				put_cooked(1, TRUE, 0, "\n");		/* put a newline after headers */
#ifdef INLINE_DEBUG_MIME
				if (stripped)
					put_cooked(LEN, wrap_lines, C_ATTACH, "[-- non text/plain parts from multipart/alternative stripped --]\n");
#endif /* INLINE_DEBUG_MIME */
			}
			break;
		}

		if (show_all_headers || header_wanted(line)) {	/* Put cooked data */
			const char **strptr = struct_header;
			char *l = NULL, *ptr, *foo, *bar;
			size_t i = LEN;
			t_bool found = FALSE;

			/* structured headers */
			/*
			 * TODO: should we look for URLs in headers too?
			 * (make this configurable?)
			 */
			do {
				if (!strncasecmp(line, *strptr, strlen(*strptr))) {
					foo = my_strdup(*strptr);
					if ((ptr = strchr(foo, ':'))) {
						*ptr = '\0';
						unfold_header(line);
						if ((ptr = parse_mb_list_header(line, foo))) {
#if 0
							char *idnp;
							/*
							 * TODO:
							 * idna_decode() currently expects just a FQDN
							 * or a mailaddress (with all comments stripped).
							 *
							 * we need to look for something like
							 * (?i)((?:\S+\.)?xn--[a-z0-9\.\-]{3,}\S+)\b
							 * and just decode $1
							 * maybe also in process_text_body_part()
							 * (after looking for url_regex & co)
							 *
							 * the following is quick'n'dirty and
							 * can't cope with multiple addresses.
							 * it needs a rewrite/cleanup to
							 * some idna_decode_line() which just
							 * decodes all the idna-parts in it and
							 * keeps he rest of the line as is
							 */

							/* something which looks like IDNA? */
							if ((idnp = strcasestr(ptr, "xn--")) != NULL) {
								char *t = NULL;
								char *ep = idnp;
								char k;

								/* look for fqdn start */
								while (idnp > ptr && (*(idnp - 1) == '+' || *(idnp - 1) == '-' || *(idnp - 1) == '.' || isalnum((int) *(idnp - 1))))
									--idnp;

								/* keep beginng of the line before idna part */
								bar = my_calloc(strlen(ptr) - strlen(idnp) + 1, 1);
								snprintf(bar, strlen(ptr) - strlen(idnp) + 1, "%s", ptr);

								/* look for fqdn end */
								while (*ep != '\0' && (*ep == '+' || *ep == '-' || *ep == '.' || isalnum((int) *ep)))
									++ep;
								/* remember "end" = first non fqdn char */
								k = *ep;
								/* terminate fqdn */
								*ep = '\0';
								if (idnp > ptr && *(idnp - 1) == '@' && strlen(idnp) >= 10 && strchr(idnp, '.')) { /* follows an '@', has idna min. len and at least one '.' */
									/* decode */
									t = idna_decode(idnp);
									/* append idn data */
									bar = append_to_string(bar, t);
									free(t);
									/* restore end char */
									*ep = k;
									if (ep <= (ptr + strlen(ptr))) /* append tail */
										bar = append_to_string(bar, ep);
								} else { /* use original data */
									*ep = k;
									free(bar);
									bar = my_strdup(ptr);
								}
							} else
#endif /* 0 */
								bar = my_strdup(ptr);

							l = my_calloc(1, strlen(bar) + strlen(*strptr) + 1);
							strncpy(l, line, strlen(*strptr));
							strcat(l, bar);
							free(bar);
						}
					}
					free(foo);
					found = TRUE;
				}
			} while (!found && *(++strptr) != NULL);

			/* unstructured but must not be decoded (see also rfc2047.c:do_rfc15211522_encode()) */
			if (l == NULL && (!strncasecmp(line, "References: ", 12) || !strncasecmp(line, "Message-ID: ", 12) || !strncasecmp(line, "Date: ", 6) || !strncasecmp(line, "Newsgroups: ", 12) || !strncasecmp(line, "Distribution: ", 14) || !strncasecmp(line, "Followup-To: ", 13) || !strncasecmp(line, "X-Face: ", 8) || !strncasecmp(line, "Cancel-Lock: ", 13) || !strncasecmp(line, "Cancel-Key: ", 12) || !strncasecmp(line, "Supersedes: ", 12) || !strncasecmp(line, "Path: ", 6)))
				l = my_strdup(line);

			if (l == NULL)
				l = my_strdup(rfc1522_decode(line));

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if (IS_LOCAL_CHARSET("UTF-8"))
				utf8_valid(l);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			header_put = TRUE;
			expand_ctrl_chars(&l, &i, tabwidth);
			put_cooked(i, wrap_lines, C_HEADER, "%s", l);
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

#ifdef INLINE_DEBUG_MIME
		if (hdr->ext->mime_hints.flags & MIME_VERSION_MISSING)
			put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"MIME-Version:\"-header missing --]\n");
		else {
			if (hdr->ext->mime_hints.flags & MIME_VERSION_UNSUPPORTED)
				put_cooked(LEN, wrap_lines, C_ATTACH, "[-- Unsupported \"MIME-Version:\" --]\n");
		}
#endif /* INLINE_DEBUG_MIME */

		for (ptr = hdr->ext->next; ptr != NULL; ptr = ptr->next) {
			/*
			 * Ignore non text/plain sections with alternative handling
			 */
			if (STRIP_ALTERNATIVE(artinfo) && !IS_PLAINTEXT(ptr))
				continue;

			name = get_filename(ptr->params);
			if (ptr->type == TYPE_TEXT)
				charset = get_param(ptr->params, "charset");
			else
				charset = NULL;

			put_attach(wrap_lines, ptr, (ptr->depth - 1) * 4, SECTION_DEFAULT, name, charset);

			/* Try to view anything of type text, may need to review this */
			if (IS_PLAINTEXT(ptr)) {
				if (hdr->ext->mime_hints.flags & MIME_CHARSET_UNSUPPORTED) {
					put_cooked(LEN, wrap_lines, C_ATTACH, _(txt_mime_unsup_charset), (ptr->depth - 1) * 4, "", hdr->ext->mime_hints.charset);
					if (ptr->next)
						put_cooked(1, wrap_lines, C_ATTACH, "\n");
				} else {
					charset = validate_charset(charset);
					process_text_body_part(wrap_lines, artinfo->raw, charset ? charset : "US-ASCII", ptr, hide_inline_data);
				}
			}
		}
	} else {
		charset = get_param(hdr->ext->params, "charset");

#ifdef INLINE_DEBUG_MIME
		if (hdr->ext->type != TYPE_TEXT || (hdr->ext->type == TYPE_TEXT && hdr->ext->encoding != ENCODING_7BIT)) { /* don't warn on pure ascii non mime */
			if (hdr->ext->mime_hints.flags & MIME_VERSION_MISSING)
				put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"MIME-Version:\"-header missing --]\n");
			else {
				if (hdr->ext->mime_hints.flags & MIME_VERSION_UNSUPPORTED)
					put_cooked(LEN, wrap_lines, C_ATTACH, "[-- Unsupported \"MIME-Version:\" --]\n");
			}
		}
#endif /* INLINE_DEBUG_MIME */

#ifdef CHARSET_CONVERSION
#	ifdef USE_ICU_UCSDET
		if (hdr->ext->type == TYPE_APPLICATION && hdr->ext->encoding == ENCODING_BINARY && (artinfo->hdr.ext->mime_hints.flags & MIME_TRANSFER_ENCODING_UNKNOWN)) {
			put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"Content-Type: application/octet-stream\" forced --]\n");
			put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"Content-Type-Encodig: %s\" forced --]\n", content_encodings[hdr->ext->encoding]);
		}
		if (hdr->ext->mime_hints.flags & MIME_CHARSET_GUESSED) {
			charset = get_param(hdr->ext->params, "guessed_charset");
#		ifdef INLINE_DEBUG_MIME
			put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"charset=%s\" guessed --]\n", charset);
			if (hdr->ext->type == TYPE_TEXT && hdr->ext->encoding != ENCODING_7BIT && ((hdr->ext->mime_hints.flags & MIME_TRANSFER_ENCODING_MISSING) || (artinfo->hdr.ext->mime_hints.flags & MIME_TRANSFER_ENCODING_UNKNOWN)))
				put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"Content-Type-Encodig: %s\" forced --]\n", content_encodings[hdr->ext->encoding]);
#		endif /* INLINE_DEBUG_MIME */
		}
#	endif /* USE_ICU_UCSDET */
#	ifdef INLINE_DEBUG_MIME
		if (hdr->ext->mime_hints.flags & MIME_CHARSET_UNSUPPORTED)
			put_cooked(LEN, wrap_lines, C_ATTACH, "[-- \"charset=%s\" unsupported --]\n", hdr->ext->mime_hints.charset);
#	endif /* INLINE_DEBUG_MIME */
#endif /* CHARSET_CONVERSION */

		/*
		 * A regular single-body article
		 */
		if (!hdr->mime || IS_PLAINTEXT(hdr->ext)) {
			if (hdr->ext->mime_hints.flags & MIME_CHARSET_UNSUPPORTED)
				put_cooked(LEN, wrap_lines, C_ATTACH, _(txt_mime_unsup_charset), 0, "", hdr->ext->mime_hints.charset);
			else
				process_text_body_part(wrap_lines, artinfo->raw, charset ? charset : "US-ASCII", hdr->ext, hide_inline_data);
		} else {
			/*
			 * Non-textual main body
			 */
			name = get_filename(hdr->ext->params);
			put_attach(wrap_lines, hdr->ext, 0, SECTION_DEFAULT, name, BlankIfNull(charset));
		}
	}

#ifdef DEBUG_ART
	dump_cooked();
#endif /* DEBUG_ART */

	if (art->cooked_lines > 0)
		art->cookl = my_realloc(art->cookl, sizeof(t_lineinfo) * (size_t) art->cooked_lines);

	rewind(art->cooked);
	return (tin_errno != 0) ? FALSE : TRUE;
}


/*
 * tin_ltoa() like function; IEC binary notation (base 1024)
 * with one decimal point. positive numbers only.
 */
#define BI_BASE 1024
static char *
ltobi(
	unsigned long i)
{
	static const char power[] = { 'e', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'R', 'Q', '\0' };
	static char buffer[9];
	char r;
	unsigned d = 0, e = 0;

#if !defined(NO_LOCALE)
	r = tin_nl_langinfo(RADIXCHAR)[0];
	if (r == '\0')
#endif /* !NO_LOCALE */
		r = '.';

	while (i >= BI_BASE) {
		d = (unsigned) (i % BI_BASE * 10 / BI_BASE);
		i /= BI_BASE;
		++e;
	}

	if (e) {
		if (e >= sizeof(power) - 1) /* any 128bit systems around? ,-) */
			sprintf(buffer, "%u%c%u%c", (unsigned) i, r, d, power[0]); /* omit value and use a better error string? */
		else
			sprintf(buffer, "%u%c%u%c", (unsigned) i, r, d, power[e]);
	} else
		sprintf(buffer, "0%c%u%c", r, (unsigned) (i * 10 / BI_BASE), power[1]);

	return buffer;
}
