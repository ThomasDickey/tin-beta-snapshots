/*
 *  Project   : tin - a Usenet reader
 *  Module    : cook.c
 *  Author    : J. Faultless
 *  Created   : 2000-03-08
 *  Updated   : 2024-10-13
 *  Notes     : Split from page.c
 *
 * Copyright (c) 2000-2024 Jason Faultless <jason@altarstone.com>
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
 * We malloc() this many t_lineinfo's at a time
 */
#define CHUNK		50

#define STRIP_ALTERNATIVE(x) \
			(curr_group->attribute->alternative_handling && \
			(x)->hdr.ext->type == TYPE_MULTIPART && \
			strcasecmp("alternative", (x)->hdr.ext->subtype) == 0)

#define MATCH_REGEX(x,y,z)	(match_regex_ex(y, (REGEX_SIZE) z, 0, 0, &(x)) >= 0)


static char *ltobi(unsigned long i);
static struct t_attach_item *add_attach_line_item(struct t_attach_item **item);
static t_bool header_wanted(const char *line);
static t_bool shorten_attach_line(struct t_attach_item *item);
static t_part *new_uue(t_part **part, char *name);
static void process_text_body_part(t_bool wrap_lines, FILE *in, const char *charset, t_part *part, int hide_uue);
static void put_attach(t_bool wrap_lines, t_part *part, int depth, int is_uue, const char *name, const char *charset);
static void put_cooked(size_t buf_len, t_bool wrap_lines, int flags, const char *fmt, ...);
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
		c++;
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
		wc++;
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
	int flags,
	const char *fmt,
	...)
{
	char *p, *bufp, *buf, *last_space;
	int wrap_column;
	int space;
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
					p++;
#else
				p++;
				space--;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}
			if (tinrc.dont_break_words && !(flags & (C_VERBATIM | C_HEADER)) && space <= 0 && last_space && p > last_space)
				p = last_space + 1;
		} else {
			while (*p && *p != '\n')
				p++;
		}
		fwrite(bufp, 1, (size_t) (p - bufp), art->cooked);
		fputs("\n", art->cooked);
		if (*p == '\n')
			p++;
		bufp = p;

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
	ptr->params->value = my_strdup(str_trim(name));

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

	if ((p = strrchr(name, '/')))
		return p + 1;

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
	t_part *part,
	int depth,
	int max_len,
	int is_uue,
	const char *name,
	const char *charset)
{
	char *attach_line;
	char *al_ptr;
	char *fmt_ptr;
	char *line_cnt_str = NULL;
	char *buf;
	char *fmt;
	int i, line_cnt_str_len;
	size_t blen;
	ssize_t space_left;
	struct t_attach_item *curr = NULL;
	struct t_attach_item *items = NULL;
	struct t_attach_item *last;
	t_bool init = TRUE;
	t_bool excl_seen = FALSE;
	t_bool star_seen = FALSE;

#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	buf = my_malloc(MB_CUR_MAX * (size_t) (cCOLS + 1));
	blen = MB_CUR_MAX * (size_t) cCOLS;
#	else
	buf = my_malloc(cCOLS + 1);
	blen = (size_t) cCOLS;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	if (is_uue)
		fmt = tinrc.page_uue_format;
	else if (signal_context == cAttachment)
		fmt = tinrc.attachment_format;
	else
		fmt = tinrc.page_mime_format;

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
				if (charset) {
					curr = add_attach_line_item(&items);
					curr->content = charset;
					curr->description = _(txt_mime_charset);
					SMALL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'd':
				curr = add_attach_line_item(&items);
				if (!line_cnt_str) {
					line_cnt_str_len = snprintf(NULL, 0, "%d", part->line_count);
					line_cnt_str = my_malloc(line_cnt_str_len + 1);
					snprintf(line_cnt_str, line_cnt_str_len + 1, "%d", part->line_count);
				}
				curr->content = line_cnt_str;
				curr->description = _(txt_mime_lines);
				SMALL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'e':
				curr = add_attach_line_item(&items);
				curr->content = content_encodings[part->encoding];
				curr->description = _(txt_mime_encoding);
				SMALL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'l':
				if (!is_uue && part->language) {
					curr = add_attach_line_item(&items);
					curr->content = part->language;
					curr->description = _(txt_mime_lang);
					SMALL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'n':
				if (name) {
					curr = add_attach_line_item(&items);
					curr->content = name;
					curr->description = _(txt_mime_name);
					SMALL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 's':
				curr = add_attach_line_item(&items);
				curr->content = part->subtype;
				curr->description = _(txt_mime_content_subtype);
				curr->flags |= ATTACH_ITEM_IS_SUBTYPE;
				SMALL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 't':
				curr = add_attach_line_item(&items);
				curr->content = content_types[part->type];
				curr->description = _(txt_mime_content_type);
				curr->flags |= ATTACH_ITEM_IS_TYPE;
				SMALL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'z':
				curr = add_attach_line_item(&items);
				curr->content = ltobi(part->bytes);
				curr->description = _(txt_mime_size);
				SMALL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'C':
				if (charset) {
					curr = add_attach_line_item(&items);
					curr->content = charset;
					curr->description = _(txt_mime_charset);
					CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'D':
				curr = add_attach_line_item(&items);
				if (!line_cnt_str) {
					line_cnt_str_len = snprintf(NULL, 0, "%d", part->line_count);
					line_cnt_str = my_malloc(line_cnt_str_len + 1);
					snprintf(line_cnt_str, line_cnt_str_len + 1, "%d", part->line_count);
				}
				curr->content = line_cnt_str;
				curr->description = _(txt_mime_lines);
				CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'E':
				curr = add_attach_line_item(&items);
				curr->content = content_encodings[part->encoding];
				curr->description = _(txt_mime_encoding);
				CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'I':
				if (is_uue) {
					curr = add_attach_line_item(&items);
					curr->content = is_uue == UUE_COMPLETE ? _(txt_uue_complete) : _(txt_uue_incomplete);
					CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'L':
				if (!is_uue && part->language) {
					curr = add_attach_line_item(&items);
					curr->content = part->language;
					curr->description = _(txt_mime_lang);
					CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'N':
				if (name) {
					curr = add_attach_line_item(&items);
					curr->content = name;
					curr->description = _(txt_mime_name);
					CAPITAL_LETTER_CONDITIONALS();
				}
				excl_seen = star_seen = FALSE;
				break;

			case 'S':
				curr = add_attach_line_item(&items);
				curr->content = part->subtype;
				curr->description = _(txt_mime_content_subtype);
				curr->flags |= ATTACH_ITEM_IS_SUBTYPE;
				CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'T':
				curr = add_attach_line_item(&items);
				curr->content = content_types[part->type];
				curr->description = _(txt_mime_content_type);
				curr->flags |= ATTACH_ITEM_IS_TYPE;
				CAPITAL_LETTER_CONDITIONALS();
				excl_seen = star_seen = FALSE;
				break;

			case 'Z':
				curr = add_attach_line_item(&items);
				curr->content = ltobi(part->bytes);
				curr->description = _(txt_mime_size);
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
					if (!is_uue && part->language && curr) {
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

				case 'I':
					if (is_uue && curr) {
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

	FreeIfNeeded(line_cnt_str);

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
	int is_uue,
	const char *name,
	const char *charset)
{
	char *attach_line = build_attach_line(part, depth, cCOLS - 1, is_uue, name, charset);

	if (is_uue)
		put_cooked(LEN, wrap_lines, C_UUE, "%s", attach_line);
	else
		put_cooked(LEN, wrap_lines, C_ATTACH, "%s", attach_line);

	FreeIfNeeded(attach_line);

	if (!is_uue && part->description)
		put_cooked(LEN, wrap_lines, C_ATTACH, _(txt_mime_description), depth, "", part->description);

	if (part->next != NULL || IS_PLAINTEXT(part)) {
		if (is_uue)
			put_cooked(1, wrap_lines, C_UUE, "\n");
		else
			put_cooked(1, wrap_lines, C_ATTACH, "\n");
	}
}


/*
 * Decodes text bodies, remove sig's, detects uuencoded sections
 */
static void
process_text_body_part(
	t_bool wrap_lines,
	FILE *in,
	const char *charset,
	t_part *part,
	int hide_uue)
{
	char *rest = NULL;
	char *line = NULL, *buf, *tmpline;
	size_t max_line_len = 0, len, len_blank;
	int flags, lines_left;
	unsigned int lines_skipped = 0;
	t_bool in_sig = FALSE;			/* Set when in sig portion */
	t_bool in_uue = FALSE;			/* Set when in uuencoded section */
	t_bool in_verbatim = FALSE;		/* Set when in verbatim section */
	t_bool verbatim_begin = FALSE;	/* Set when verbatim_begin_regex matches */
	t_bool is_uubegin;				/* Set when current line starts a uue part */
	t_bool is_uubody;				/* Set when current line looks like a uuencoded line */
	t_bool first_line_blank = TRUE;	/* Unset when first non-blank line is reached */
	t_bool put_blank_lines = FALSE;	/* Set when previously skipped lines needs to put */
	t_part *curruue = NULL;

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
				if (max_line_len < strlen(buf) + 2 || !line) {
					max_line_len = strlen(buf) + 2;
					line = my_realloc(line, max_line_len);
				}
				strcpy(line, buf);

				/*
				 * FIXME: Some code in cook.c expects a '\n' at the end
				 * of the line. As tin_fgets() strips trailing '\n', re-add it.
				 * This should probably be fixed in that other code.
				 */
				strcat(line, "\n");

				lines_left--;
				break;
		}
		if (!(line && strlen(line))) {
			FreeIfNeeded(rest);
			break;	/* premature end of file, file error etc. */
		}

		process_charsets(&line, &max_line_len, charset, tinrc.mm_local_charset, curr_group->attribute->tex2iso_conv && art->tex2iso);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (IS_LOCAL_CHARSET("UTF-8")) {
			utf8_valid(line);

			if (!in_verbatim && curr_group->attribute->suppress_soft_hyphens && !strcasecmp(charset, "UTF-8"))
				remove_soft_hyphens(line);
		}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		len = strlen(line);

		/*
		 * trim article body and sig (not verbatim blocks):
		 * - skip leading blank lines
		 * - replace multiple blank lines with one empty line
		 * - skip tailing blank lines, keep one if an
		 *   attachment follows
		 */
		if (curr_group->attribute->trim_article_body && !in_uue && !in_verbatim && !verbatim_begin) {
			len_blank = 1;
			tmpline = line;
			/* check if line contains only whitespace */
			while ((*tmpline == ' ') || (*tmpline == '\t')) {
				len_blank++;
				tmpline++;
			}
			if (len_blank == len) {		/* line is blank */
				if (lines_left == 0 && (curr_group->attribute->trim_article_body & SKIP_TRAILING)) {
					if (!(part->next == NULL || (STRIP_ALTERNATIVE(art) && !IS_PLAINTEXT(part->next))))
						put_cooked(1, TRUE, in_sig ? C_SIG : C_BODY, "\n");
					continue;
				}
				if (first_line_blank) {
					if (curr_group->attribute->trim_article_body & SKIP_LEADING)
						continue;
				} else if ((curr_group->attribute->trim_article_body & (COMPACT_MULTIPLE | SKIP_TRAILING)) && (!in_sig || curr_group->attribute->show_signatures)) {
					lines_skipped++;
					if (lines_left == 0 && !(curr_group->attribute->trim_article_body & SKIP_TRAILING)) {
						for (; lines_skipped > 0; lines_skipped--)
							put_cooked(1, TRUE, in_sig ? C_SIG : C_BODY, "\n");
					}
					continue;
				}
			} else {	/* line is not blank */
				if (first_line_blank)
					first_line_blank = FALSE;
				if (lines_skipped && (!in_sig || curr_group->attribute->show_signatures)) {
					if (strcmp(line, SIGDASHES) != 0 || curr_group->attribute->show_signatures) {
						if (curr_group->attribute->trim_article_body & COMPACT_MULTIPLE)
							put_cooked(1, TRUE, in_sig ? C_SIG : C_BODY, "\n");
						else
							put_blank_lines = TRUE;
					} else if (!(curr_group->attribute->trim_article_body & SKIP_TRAILING))
						put_blank_lines = TRUE;
					if (put_blank_lines) {
						for (; lines_skipped > 0; lines_skipped--)
							put_cooked(1, TRUE, in_sig ? C_SIG : C_BODY, "\n");
					}
					put_blank_lines = FALSE;
					lines_skipped = 0;
				}
			}
		} /* if (tinrc.trim_article_body...) */

		/* look for verbatim marks, set in_verbatim only for lines in between */
		if (curr_group->attribute->verbatim_handling) {
			if (verbatim_begin) {
				in_verbatim = TRUE;
				verbatim_begin = FALSE;
			} else if (!in_sig && !in_uue && !in_verbatim && MATCH_REGEX(verbatim_begin_regex, line, len))
				verbatim_begin = TRUE;
			if (in_verbatim && MATCH_REGEX(verbatim_end_regex, line, len))
				in_verbatim = FALSE;
		}

		if (!in_verbatim) {
			/*
			 * Detect and skip signatures if necessary
			 */
			if (!in_sig) {
				if (STRCMPEQ(line, SIGDASHES)) {
					in_sig = TRUE;
					if (in_uue) {
						in_uue = FALSE;
						if (hide_uue)
							put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
					}
				}
			}

			if (in_sig && !(curr_group->attribute->show_signatures))
				continue;					/* No further processing needed */

			/*
			 * Detect and process uuencoded sections
			 * Look for the start or the end of a uuencoded section
			 *
			 * TODO: look for a tailing size line after end (non standard
			 *       extension)?
			 *       do we want to cook uue-parts in signatures?
			 */

			is_uubegin = FALSE;

			if (MATCH_REGEX(uubegin_regex, line, len)) {
				REGEX_SIZE *ovector = regex_get_ovector_pointer(&uubegin_regex);

				if (in_uue) { /* previous uue part incomplete and the current one follows without gap */
					if (hide_uue)
						put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
				} else
					in_uue = TRUE;

				is_uubegin = TRUE;
				curruue = new_uue(&part, line + ovector[1]);
				if (hide_uue)
					continue;				/* Don't cook the 'begin' line */
			} else if (STRNCMPEQ(line, "end\n", 4)) {
				if (in_uue) {
					in_uue = FALSE;
					if (hide_uue) {
						put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, UUE_COMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
						continue;			/* Don't cook the 'end' line */
					}
				}
			}

			/*
			 * See if this line looks like a uuencoded 'body' line
			 */
			is_uubody = FALSE;

			if (MATCH_REGEX(uubody_regex, line, len)) {
				int sum = (((*line) - ' ') & 077) * 4 / 3;		/* uuencode octet checksum */

				/* sum = 0 in a uubody only on the last line, a single ` */
				if (sum == 0 && len == 1 + 1)			/* +1 for the \n */
					is_uubody = TRUE;
				else if (len == (unsigned) (sum + 1 + 1))
					is_uubody = TRUE;
#ifdef DEBUG_ART
				if (debug & DEBUG_MISC)
					fprintf(stderr, "%s sum=%d len=%lu (%s)\n", bool_unparse(is_uubody), sum, len, line);
#endif /* DEBUG_ART */
			}

			if (in_uue) {
				if (is_uubody) {
					curruue->line_count++;
					curruue->bytes += len;
				} else {
					if (line[0] == '\n') {		/* Blank line in a uubody - definitely a failure */
						/* fprintf(stderr, "not a uue line while reading a uue body?\n"); */
						in_uue = FALSE;
						if (hide_uue)
							/* don't continue here, so we see the line that 'broke' in_uue */
							put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);
					} else {
						if (!is_uubegin) { /* xxencoding or the like */
							curruue->line_count++;
							curruue->bytes += len;
						}
					}
				}
			} else {
				/*
				 * UUE_ALL = 'Try harder' - we never saw a begin line, but useful
				 * when uue sections are split across > 1 article
				 */
				if (is_uubody && hide_uue == UUE_ALL) {
					/* _(txt_unknown) cannot be used directly in new_uue() due to str_trim() there */
					char *name = my_strdup(_(txt_unknown));

					in_uue = TRUE;
					curruue = new_uue(&part, name);
					curruue->line_count++;
					curruue->bytes += len;
					free(name);
					continue;
				}
			}

			/*
			 * Skip output if we're hiding uue or the sig
			 */
			if (in_uue && hide_uue)
				continue;	/* No further processing needed */
		}

		flags = in_verbatim ? C_VERBATIM : in_sig ? C_SIG : C_BODY;

		/*
		 * Don't do any further handling of uue || verbatim lines
		 */
		if (in_uue) {
			put_cooked(max_line_len, wrap_lines, flags, "%s", line);
			continue;
		} else if (in_verbatim) {
			expand_ctrl_chars(&line, &max_line_len, 8);
			put_cooked(max_line_len, wrap_lines, flags, "%s", line);
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

		if (MATCH_REGEX(url_regex, line, len))
			flags |= C_URL;
		if (MATCH_REGEX(mail_regex, line, len))
			flags |= C_MAIL;
		if (MATCH_REGEX(news_regex, line, len))
			flags |= C_NEWS;

		if (expand_ctrl_chars(&line, &max_line_len, tabwidth))
			flags |= C_CTRLL;				/* Line contains form-feed */

		buf = line;

		/*
		 * Skip over the first space in case of Format=Flowed (space-stuffing)
		 */
		if (part->format == FORMAT_FLOWED) {
			if (line[0] == ' ')
				++buf;
		}

		put_cooked(max_line_len, wrap_lines && (!IS_LOCAL_CHARSET("Big5")), flags, "%s", buf);
	} /* while */

	/*
	 * Were we reading uue and ran off the end ?
	 */
	if (in_uue && hide_uue)
		put_attach(wrap_lines, curruue, (curruue->depth - 1) * 4, UUE_INCOMPLETE, get_filename(curruue->params), content_encodings[curruue->encoding]);

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
 *
 * TODO:
 *      give an error-message on at least disk-full
 */
/* set INLINE_DEBUG_MIME to 1 to inline report some errors/warnings,
 * use PAGE_ARTICLE_INFO ('\'') for a more detailed info.
 * strings -> lang.c?
 */
t_bool
cook_article(
	t_bool wrap_lines,
	t_openartinfo *artinfo,
	int hide_uue,
	t_bool show_all_headers)
{
	const char *charset = NULL;
	const char *name;
	char *line;
	struct t_header *hdr = &artinfo->hdr;
	static const char *struct_header[] = {
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
			do {
				if (!strncasecmp(line, *strptr, strlen(*strptr))) {
					foo = my_strdup(*strptr);
					if ((ptr = strchr(foo, ':'))) {
						*ptr = '\0';
						unfold_header(line);
						if ((ptr = parse_mb_list_header(line, foo))) {
#if 0
							/*
							 * TODO:
							 * idna_decode() currently expects just a FQDN
							 * or a mailaddress (with all comments stripped).
							 *
							 * we need to look for something like
							 * (?i)((?:\S+\.)?xn--[a-z0-9\.\-]{3,}\S+)\b
							 * and just decode $1
							 * maybe also in process_text_body_part()
							 */
							bar = idna_decode(ptr);
#else
							bar = my_strdup(ptr);
#endif /* 0 */
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

			/* unstructured but must not be decoded */
			if (l == NULL && (!strncasecmp(line, "References: ", 12) || !strncasecmp(line, "Message-ID: ", 12) || !strncasecmp(line, "Date: ", 6) || !strncasecmp(line, "Newsgroups: ", 12) || !strncasecmp(line, "Distribution: ", 14) || !strncasecmp(line, "Followup-To: ", 13) || !strncasecmp(line, "X-Face: ", 8) || !strncasecmp(line, "Cancel-Lock: ", 13) || !strncasecmp(line, "Cancel-Key: ", 12) || !strncasecmp(line, "Supersedes: ", 12)))
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

			put_attach(wrap_lines, ptr, (ptr->depth - 1) * 4, 0, name, charset);

			/* Try to view anything of type text, may need to review this */
			if (IS_PLAINTEXT(ptr)) {
				if (hdr->ext->mime_hints.flags & MIME_CHARSET_UNSUPPORTED) {
					put_cooked(LEN, wrap_lines, C_ATTACH, _(txt_mime_unsup_charset), (ptr->depth - 1) * 4, "", hdr->ext->mime_hints.charset);
					if (ptr->next)
						put_cooked(1, wrap_lines, C_ATTACH, "\n");
				} else {
					charset = validate_charset(charset);
					process_text_body_part(wrap_lines, artinfo->raw, charset ? charset : "US-ASCII", ptr, hide_uue);
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
		if (hdr->ext->type == TYPE_APPLICATION && hdr->ext->encoding == ENCODING_BINARY && artinfo->hdr.ext->mime_hints.flags & MIME_TRANSFER_ENCODING_UNKNOWN) {
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
				process_text_body_part(wrap_lines, artinfo->raw, charset ? charset : "US-ASCII", hdr->ext, hide_uue);
		} else {
			/*
			 * Non-textual main body
			 */
			name = get_filename(hdr->ext->params);
			put_attach(wrap_lines, hdr->ext, 0, 0, name, BlankIfNull(charset));
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
	unsigned d = 0, e = 0;

	while (i >= BI_BASE) {
		d = (unsigned) (i % BI_BASE * 10 / BI_BASE);
		i /= BI_BASE;
		e++;
	}

	if (e) {
		if (e >= sizeof(power) - 1) /* any 128bit systems around? ,-) */
			sprintf(buffer, "%u.%u%c", (unsigned) i, d, power[0]); /* omit value and use a better error string? */
		else
			sprintf(buffer, "%u.%u%c", (unsigned) i, d, power[e]);
	} else
		sprintf(buffer, "0.%u%c", (unsigned) (i * 10 / BI_BASE), power[1]);

	return buffer;
}
