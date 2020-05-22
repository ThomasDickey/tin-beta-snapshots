/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2046.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 2000-02-18
 *  Updated   : 2020-05-01
 *  Notes     : RFC 2046 MIME article parsing
 *
 * Copyright (c) 2000-2020 Jason Faultless <jason@altarstone.com>
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


/*
 * local prototypes
 */
static char *get_charset(char *value);
static char *get_quoted_string(char *source, char **dest);
static char *get_token(const char *source);
static char *strip_charset(char **value);
static char *skip_equal_sign(char *source);
static char *skip_space(char *source);
static int boundary_cmp(const char *line, const char *boundary);
static int count_lines(char *line);
static int parse_multipart_article(FILE *infile, t_openartinfo *artinfo, t_part *part, int depth, t_bool show_progress_meter);
static int parse_normal_article(FILE *in, t_openartinfo *artinfo, t_bool show_progress_meter);
static int parse_rfc2045_article(FILE *infile, int line_count, t_openartinfo *artinfo, t_bool show_progress_meter);
static unsigned int parse_content_encoding(char *encoding);
static void decode_value(const char *charset, t_param *part);
static void parse_content_type(char *type, t_part *content);
static void parse_content_disposition(char *disp, t_part *part);
static void parse_params(char *params, t_part *content);
static void progress(int line_count);
static void remove_cwsp(char *source);
#ifdef DEBUG_ART
	static void dump_art(t_openartinfo *art);
#endif /* DEBUG_ART */


/*
 * Local variables
 */
static int art_lines = 0;		/* lines in art on spool */
static const char *progress_mesg = NULL;	/* message progress() should display */
/* RFC 2231 decoding table */
static const char xtbl[] = {
/*        0  1  2  3    4  5  6  7    8  9  a  b    c  d  e  f */
/* 0 */  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,
/* 1 */  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,
/* 2 */  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,
/* 3 */   0, 1, 2, 3,   4, 5, 6, 7,   8, 9,-1,-1,  -1,-1,-1,-1,
/* 4 */  -1,10,11,12,  13,14,15,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,
/* 5 */  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,
/* 6 */  -1,10,11,12,  13,14,15,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,
/* 7 */  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1
};

#define XVAL(c) (xtbl[(unsigned int) (c)])
/* C90: isxdigit(3) */
#define IS_XDIGIT(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define PARAM_SEP	"; \n"
/* default parameters for Content-Type */
#define CT_DEFPARMS	"charset=US-ASCII"

/*
 * Use the default message if one hasn't been supplied
 * Body search is currently the only function that has a different message
 */
static void
progress(
	int line_count)
{
	if (progress_mesg != NULL && art_lines > 0 && line_count && line_count % MODULO_COUNT_NUM == 0)
		show_progress(progress_mesg, line_count, art_lines);
}


/*
 * Lookup content type in content_types[] array and return matching
 * index or -1
 */
int
content_type(
	char *type)
{
	int i;

	if (type == NULL)
		return -1;

	for (i = 0; content_types[i] != NULL; ++i) {
		if (strcasecmp(type, content_types[i]) == 0)
			return i;
	}

	return -1;
}


/*
 * check if a line is a MIME boundary
 * returns BOUND_NONE if it is not, BOUND_START if normal boundary and
 * BOUND_END if closing boundary
 */
static int
boundary_cmp(
	const char *line,
	const char *boundary)
{
	size_t blen = strlen(boundary);
	size_t len;
	char *e, *l;
	int nl;

	if ((len = strlen(line)) == 0)
		return BOUND_NONE;

	if (blen + 2 > len)
		return BOUND_NONE;

	/* remove trailing whites as per RFC 2046 5.1.1 */
	l = my_strdup(line);
	e = l + len - 1;
	while(e > l + blen + 1 && isspace(*e))
		*e-- = '\0';

	len = strlen(l);

	nl = l[len - 1] == '\n';

	if (len != blen + 2 + nl && len != blen + 4 + nl) {
		free(l);
		return BOUND_NONE;
	}
	if (l[0] != '-' || l[1] != '-') {
		free(l);
		return BOUND_NONE;
	}

	if (strncmp(l + 2, boundary, blen) != 0) {
		free(l);
		return BOUND_NONE;
	}

	if (l[blen + 2] != '-') {
		if (nl ? l[blen + 2] == '\n' : l[blen + 2] == '\0') {
			free(l);
			return BOUND_START;
		} else {
			free(l);
			return BOUND_NONE;
		}
	}

	if (l[blen + 3] != '-') {
		free(l);
		return BOUND_NONE;
	}

	if (nl ? l[blen + 4] == '\n' : l[blen + 4] == '\0') {
		free(l);
		return BOUND_END;
	}
	free(l);
	return BOUND_NONE;
}


/*
 * RFC2046 5.1.2 says that we are required to check for all possible
 * boundaries, not only the one that is expected. Iterate through all
 * the parts.
 */
static int
boundary_check(
	const char *line,
	t_part *part)
{
	const char *boundary;
	int bnd = BOUND_NONE;

	for (; part != NULL; part = part->next) {
		/* We may not have even parsed a boundary for this part yet */
		if ((boundary = get_param(part->params, "boundary")) == NULL)
			continue;
		if ((bnd = boundary_cmp(line, boundary)) != BOUND_NONE)
			break;
	}

	return bnd;
}


#define ATTRIBUTE_DELIMS "()<>@,;:\\\"/[]?="

static char *
skip_space(
	char *source)
{
	while ((*source) && ((*source == ' ') || (*source == '\t')))
		source++;
	return *source ? source : NULL;
}


/*
 * Removes comments and white space
 */
static void
remove_cwsp(
	char *source)
{
	char *from, *to, src;
	int c_cnt = 0;
	t_bool inquotes = FALSE;

	from = to = source;

	while ((src = *from++) && c_cnt >= 0) {
		if (src == '"' && c_cnt == 0)
			inquotes = bool_not(inquotes);

		if (inquotes && src == '\\' && *from) {
			*to++ = src;
			*to++ = *from++;
			continue;
		}

		if (!inquotes) {
			/* skip over quoted pairs */
			if (c_cnt && src == '\\') {
				++from;
				continue;
			}
			if (src == '(') {
				++c_cnt;
				continue;
			}
			if (src == ')') {
				--c_cnt;
				continue;
			}
			if (c_cnt > 0 || src == ' ' || src == '\t')
				continue;
		}

		*to++ = src;
	}

	/*
	 * Setting *source = '\0' might be the right thing
	 * because the header is damaged. Anyway, we let the
	 * rest of the code pick up usable pieces.
	 */
#if 0
	if (c_cnt != 0)
		/* unbalanced parenthesis, header damaged */
		*source = '\0';
	else
#endif /* 0 */
		*to = '\0';
}


static char *
get_token(
	const char *source)
{
	char *dest = my_strdup(source);
	char *ptr = dest;

	while (isascii((int) *ptr) && isprint((int) *ptr) && *ptr != ' ' && !strchr(ATTRIBUTE_DELIMS, *ptr))
		ptr++;
	*ptr = '\0';

	return my_realloc(dest, strlen(dest) + 1);
}


static char *
get_quoted_string(
	char *source,
	char **dest)
{
	char *ptr;
	t_bool quote = FALSE;

	*dest = my_malloc(strlen(source) + 1);
	ptr = *dest;
	source++; /* skip over double quote */
	while (*source) {
		if (*source == '\\') {
			quote = TRUE;	/* next char as-is */
			if (*++source == '\\') {
				*ptr++ = *source++;
				quote = FALSE;
			}
			continue;
		}
		if ((*source == '"') && !quote)
			break;	/* end of quoted-string */
		*ptr++ = *source++;
		quote = FALSE;
	}
	*ptr = '\0';
	*dest = my_realloc(*dest, strlen(*dest) + 1);
	return *source ? source + 1 : source;
}


/*
 * RFC 2231: Extract character set from parameter value
 */
static char *
get_charset(
	char *value)
{
	char *charset, *ptr;

	/* no charset information present */
	if (!strchr(value, '\''))
		return NULL;

	/* no charset given -> fall back to us-ascii */
	if (*value == '\'')
		return my_strdup("US-ASCII");

	charset = my_strdup(value);

	if ((ptr = strchr(charset, '\'')))
		*ptr = '\0';

	return charset;
}


/*
 * RFC 2231: Decode parameter value according to the given
 *           character set
 */
static void
decode_value(
	const char *charset,
	t_param *part)
{
	char *rptr, *wptr;
	const char *cset;
	size_t max_line_len = strlen(part->value);

	/*
	 * we prefer part->charset if present, even if rfc 2231
	 * forbids different charsets for each part
	 */
	cset = part->charset ? part->charset : charset;
	rptr = wptr = part->value;

	while (*rptr) {
		if (*rptr == '%' && IS_XDIGIT(*(rptr + 1)) && IS_XDIGIT(*(rptr + 2))) {
			*wptr++ = XVAL(*(rptr + 1)) << 4 | XVAL(*(rptr + 2));
			rptr += 3;
		} else
			*wptr++ = *rptr++;
	}
	*wptr = '\0';

	process_charsets(&(part->value), &max_line_len, cset, tinrc.mm_local_charset, FALSE);
	part->encoded = FALSE;
	FreeAndNull(part->charset);
}


/*
 * RFC 2231: Remove character set (and language information)
 *           from parameter value
 */
static char *
strip_charset(
	char **value)
{
	char *newval, *ptr;

	if ((ptr = strrchr(*value, '\''))) {
		newval = my_strdup(ptr + 1);
		free(*value);
		*value = my_realloc(newval, strlen(newval) + 1);
	}

	return *value;
}


/*
 * Skip equal sign and (non compliant) white space around it
 */
static char *
skip_equal_sign(
	char *source)
{
	if (!(source = skip_space(source)))
		return NULL;

	if (*source++ != '=')
		/* no equal sign, invalid header, stop parsing here */
		return NULL;

	return skip_space(source);
}


/*
 * Parse a Content-* parameter list into a linked list
 * Ensure the ->params element is correctly initialised before calling
 * TODO: may still not catch everything permitted in the RFC
 */
static void
parse_params(
	char *params,
	t_part *content)
{
	char *name, *param, *value, *contp;
	int idx;
	t_bool encoded;
	t_param *ptr;

	param = params;
	while (*param) {
		idx = -1;
		encoded = FALSE;
		/* Skip over white space */
		if (!(param = skip_space(param)))
			break;

		/* catch parameter name */
		name = get_token(param);
		param += strlen(name);

		if (!*param) {
			/* Nothing follows, invalid, stop here */
			FreeIfNeeded(name);
			break;
		}

		/* RFC 2231 Character set and language information */
		if ((contp = strrchr(name, '*')) && !*(contp + 1)) {
			encoded = TRUE;
			*contp = '\0';
		}

		/* RFC 2231 Parameter Value Continuations */
		if ((contp = strchr(name, '*')) && *(contp + 1) >= '0' && *(contp + 1) <= '9') {
			idx = atoi(contp + 1);
			*contp = '\0';
		}

		if (!(param = skip_equal_sign(param))) {
			FreeIfNeeded(name);
			break;
		}

		/* catch parameter value; may be surrounded by double quotes */
		if (*param == '"')	/* parse quoted-string */
			param = get_quoted_string(param, &value);
		else {
			/* parse token */
			value = get_token(param);
			param += strlen(value);
		}

		ptr = new_params();
		ptr->name = name;
		if (encoded) {
			ptr->encoded = TRUE;
			ptr->charset = get_charset(value);
			ptr->value = strip_charset(&value);
		} else
			ptr->value = value;

		ptr->part = idx;
		ptr->next = content->params;		/* Push onto start of list */
		content->params = ptr;

		/* advance pointer to next parameter */
		while ((*param) && (*param != ';'))
			param++;
		if (*param == ';')
			param++;
	}
}


/*
 * Return a freshly allocated and initialised t_param structure
 */
t_param *
new_params(
	void)
{
	t_param *ptr;

	ptr = my_malloc(sizeof(t_param));
	ptr->name = NULL;
	ptr->value = NULL;
	ptr->charset = NULL;
	ptr->part = -1;
	ptr->encoded = FALSE;
	ptr->enc_fallback = TRUE;
	ptr->next = NULL;

	return ptr;
}


/*
 * Free up a generic list object
 */
void
free_list(
	t_param *list)
{
	while (list->next != NULL) {
		free_list(list->next);
		list->next = NULL;
	}

	free(list->name);
	free(list->value);
	FreeIfNeeded(list->charset);
	free(list);
}


/*
 * Return a parameter value from a param list or NULL
 */
const char *
get_param(
	t_param *list,
	const char *name)
{
	char *tmpval, *charset = NULL;
	int i, j;
	size_t newlen;
	t_param *p_list, *c_list;

	for (p_list = list; p_list != NULL; p_list = p_list->next) {
		/*
		 * RFC 2231 Parameter Value Continuations + Character Set
		 *
		 * part == 0,1,2...: parameter has several parts, must be concatenated
		 * part == -1      : parameter has only one part
		 * part == -2      : part has already been concatenated, main part has
		 *                   part == -1
		 *
		 * charset         : character set if present
		 */
		if (strcasecmp(name, p_list->name) == 0 && p_list->part > -2) {
			if (p_list->part == -1 && p_list->encoded && p_list->charset) {
				decode_value(p_list->charset, p_list);
				p_list->encoded = FALSE;
				p_list->enc_fallback = FALSE;
			}
			if (p_list->part >= 0) {
				newlen = 0;
				if (p_list->charset) {
					FreeIfNeeded(charset);
					charset = my_strdup(p_list->charset);
				}
				for (j = 0, c_list = list; c_list != NULL; c_list = c_list->next) {
					if (strcasecmp(name, c_list->name) == 0) {
						if (c_list->part < 0)
							continue;
						if (c_list->part < p_list->part) {
							if (c_list->charset) {
								FreeIfNeeded(charset);
								charset = my_strdup(c_list->charset);
							}
							p_list = c_list;
						}

						if (j < c_list->part)
							j = c_list->part;

						newlen += strlen(c_list->value);
					}
				}
				p_list->value = my_realloc(p_list->value, newlen + 1);
				if (charset)
					decode_value(charset, p_list);
				for (i = p_list->part + 1; i <= j; ++i) {
					for (c_list = list; c_list != NULL; c_list = c_list->next) {
						if (strcasecmp(name, c_list->name) == 0) {
							if (c_list->part == i) {
								if (c_list->encoded && charset)
									decode_value(charset, c_list);
								strcat(p_list->value, c_list->value);
								c_list->part = -2;
							}
						}
					}
				}
				p_list->part = -1;
				p_list->encoded = FALSE;
				p_list->enc_fallback = FALSE;
				FreeAndNull(charset);
			}
			/*
			 * RFC 2047 'encoded-word' is not allowed at this place but
			 * some clients use this nevertheless -> we try to decode that
			 */
			if (p_list->enc_fallback) {
				tmpval = p_list->value;
				if (*tmpval == '=' && *(++tmpval) == '?') {
					if ((tmpval = rfc1522_decode(p_list->value))) {
						free(p_list->value);
						p_list->value = my_strdup(tmpval);
					}
				}
				p_list->enc_fallback = FALSE;
			}
			return p_list->value;
		}
	}

	return NULL;
}


/*
 * Split a Content-Type header into a t_part structure
 */
static void
parse_content_type(
	char *type,
	t_part *content)
{
	char *subtype, *params;
	int i;

	/* Remove comments and white space */
	remove_cwsp(type);

	/*
	 * Split the type/subtype
	 */
	if ((type = strtok(type, "/")) == NULL)
		return;

	/* Look up major type */

	/*
	 * Unrecognised type, treat according to RFC
	 */
	if ((i = content_type(type)) == -1) {
		content->type = TYPE_APPLICATION;
		free(content->subtype);
		content->subtype = my_strdup("octet-stream");
		return;
	} else
		content->type = i;

	subtype = strtok(NULL, PARAM_SEP);
	/* save new subtype, or use pre-initialised value "plain" */
	if (subtype != NULL) {				/* check for broken Content-Type: is header without a subtype */
		free(content->subtype);				/* Pre-initialised to plain */
		content->subtype = my_strdup(subtype);
		str_lwr(content->subtype);
	}

	/*
	 * Parse any parameters into a list
	 */
	if ((params = strtok(NULL, "\n")) != NULL) {
		const char *format;
#ifndef CHARSET_CONVERSION
		char defparms[] = CT_DEFPARMS;	/* must be writable */
#endif /* !CHARSET_CONVERSION */

		parse_params(params, content);
		if (!get_param(content->params, "charset")) {	/* add default charset if needed */
#ifndef CHARSET_CONVERSION
			parse_params(defparms, content);
#else
			if (curr_group->attribute->undeclared_charset) {
				char *charsetheader;

				charsetheader = my_malloc(strlen(curr_group->attribute->undeclared_charset) + 9); /* 9=len('charset=\0') */
				sprintf(charsetheader, "charset=%s", curr_group->attribute->undeclared_charset);
				parse_params(charsetheader, content);
				free(charsetheader);
			} else {
				char defparms[] = CT_DEFPARMS;	/* must be writable */

				parse_params(defparms, content);
			}
#endif /* !CHARSET_CONVERSION */
		}
		if ((format = get_param(content->params, "format"))) {
			if (!strcasecmp(format, "flowed"))
				content->format = FORMAT_FLOWED;
		}
	}
}


static unsigned int
parse_content_encoding(
	char *encoding)
{
	unsigned int i;

	/* Remove comments and white space */
	remove_cwsp(encoding);

	for (i = 0; content_encodings[i] != NULL; ++i) {
		if (strcasecmp(encoding, content_encodings[i]) == 0)
			return i;
	}

	/*
	 * TODO: check rfc - may need to switch Content-Type to
	 * application/octet-steam where this header exists but is unparsable.
	 *
	 * RFC 2045 6.2:
	 * Labelling unencoded data containing 8bit characters as "7bit" is not
	 * allowed, nor is labelling unencoded non-line-oriented data as anything
	 * other than "binary" allowed.
	 */
	return ENCODING_BINARY;
}


/*
 * We're only really interested in the filename parameter, which has
 * a higher precedence than the name parameter from Content-Type (RFC 1806)
 * Attach the parsed params to the part passed in 'part'
 */
static void
parse_content_disposition(
	char *disp,
	t_part *part)
{
	char *ptr;

	/* Remove comments and white space */
	remove_cwsp(disp);

	strtok(disp, PARAM_SEP);
	if ((ptr = strtok(NULL, "\n")) == NULL)
		return;

	parse_params(ptr, part);
}


/*
 * Return a freshly allocated and initialised part structure attached to the
 * end of the list of article parts given
 */
t_part *
new_part(
	t_part *part)
{
	t_part *p;
	t_part *ptr = my_malloc(sizeof(t_part));
#ifndef CHARSET_CONVERSION
	char defparms[] = CT_DEFPARMS;	/* must be writable */
#endif /* !CHARSET_CONVERSION */

	ptr->type = TYPE_TEXT;					/* Defaults per RFC */
	ptr->subtype = my_strdup("plain");
	ptr->description = NULL;
	ptr->encoding = ENCODING_7BIT;
	ptr->format = FORMAT_FIXED;
	ptr->params = NULL;

#ifndef CHARSET_CONVERSION
	parse_params(defparms, ptr);
#else
	if (curr_group && curr_group->attribute->undeclared_charset) {
		char *charsetheader;

		charsetheader = my_malloc(strlen(curr_group->attribute->undeclared_charset) + 9); /* 9=len('charset=\0') */
		sprintf(charsetheader, "charset=%s", curr_group->attribute->undeclared_charset);
		parse_params(charsetheader, ptr);
		free(charsetheader);
	} else {
		char defparms[] = CT_DEFPARMS;	/* must be writable */

		parse_params(defparms, ptr);
	}
#endif /* !CHARSET_CONVERSION */

	ptr->offset = 0;
	ptr->line_count = 0;
	ptr->depth = 0;							/* Not an embedded object (yet) */
	ptr->uue = NULL;
	ptr->next = NULL;

	if (part == NULL)						/* List head - we don't do this */
		return ptr;

	for (p = part; p->next != NULL; p = p->next)
		;
	p->next = ptr;

	return ptr;
}


/*
 * Free a linked list of t_part
 */
void
free_parts(
	t_part *ptr)
{
	while (ptr->next != NULL) {
		free_parts(ptr->next);
		ptr->next = NULL;
	}

	free(ptr->subtype);
	FreeAndNull(ptr->description);
	if (ptr->params)
		free_list(ptr->params);
	if (ptr->uue)
		free_parts(ptr->uue);
	free(ptr);
}


void
free_and_init_header(
	struct t_header *hdr)
{
	/*
	 * Initialise the header struct
	 */
	FreeAndNull(hdr->from);
	FreeAndNull(hdr->to);
	FreeAndNull(hdr->cc);
	FreeAndNull(hdr->bcc);
	FreeAndNull(hdr->date);
	FreeAndNull(hdr->subj);
	FreeAndNull(hdr->org);
	FreeAndNull(hdr->replyto);
	FreeAndNull(hdr->newsgroups);
	FreeAndNull(hdr->messageid);
	FreeAndNull(hdr->references);
	FreeAndNull(hdr->distrib);
	FreeAndNull(hdr->keywords);
	FreeAndNull(hdr->summary);
	FreeAndNull(hdr->followup);
	FreeAndNull(hdr->ftnto);
#ifdef XFACE_ABLE
	FreeAndNull(hdr->xface);
#endif /* XFACE_ABLE */
	hdr->mime = FALSE;

	if (hdr->ext)
		free_parts(hdr->ext);
	hdr->ext = NULL;
}


/*
 * buf:         Article header
 * pat:         Text to match in header
 * decode:      RFC2047-decode the header
 * structured:  extract address-part before decoding the header
 *
 * Returns:
 *	(decoded) body of header if matched or NULL
 */
char *
parse_header(
	char *buf,
	const char *pat,
	t_bool decode,
	t_bool structured,
	t_bool keep_tab)
{
	size_t plen = strlen(pat);
	char *ptr = buf + plen;

	/*
	 * Does ': ' follow the header text?
	 */
	if (!(*ptr && *(ptr + 1) && *ptr == ':' && *(ptr + 1) == ' '))
		return NULL;

	/*
	 * If the header matches, skip past the ': ' and any leading whitespace
	 */
	if (strncasecmp(buf, pat, plen) != 0)
		return NULL;

	ptr += 2;

	str_trim(ptr);
	if (!*ptr)
		return NULL;

	if (decode) {
		if (structured) {
			char addr[HEADER_LEN];
			char name[HEADER_LEN];
			int type;

			if (gnksa_split_from(ptr, addr, name, &type) == GNKSA_OK) {
				buffer_to_ascii(addr);

				if (*name) {
					if (type == GNKSA_ADDRTYPE_OLDSTYLE)
						sprintf(ptr, "%s (%s)", addr, convert_to_printable(rfc1522_decode(name), keep_tab));
					else
						sprintf(ptr, "%s <%s>", convert_to_printable(rfc1522_decode(name), keep_tab), addr);
				} else
					strcpy(ptr, addr);
			} else
				return convert_to_printable(ptr, keep_tab);
		} else
			return (convert_to_printable(rfc1522_decode(ptr), keep_tab));
	}

	return ptr;
}


/*
 * Read main article headers into a blank header structure.
 * Pass the data 'from' -> 'to' when reading via NNTP
 * Return tin_errno (basically will be !=0 if reading was 'q'uit)
 * We have to guard against 'to' here since this function is exported
 */
int
parse_rfc822_headers(
	struct t_header *hdr,
	FILE *from,
	FILE *to)
{
	char *line;
	char *ptr;

	memset(hdr, 0, sizeof(struct t_header));
	hdr->mime = FALSE;
	hdr->ext = new_part(NULL);		/* Initialise MIME data */

	while ((line = tin_fgets(from, TRUE)) != NULL) {
		if (read_news_via_nntp && to) {
			fprintf(to, "%s\n", line);		/* Put raw data */
#ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "<<<%s%s", logtime(), line);
#endif /* DEBUG */
		}
		/*
		 * End of headers ?
		 */
		if (line[0] == '\0') {
			if (to)
				hdr->ext->offset = ftell(to);	/* Offset of main body */
			return 0;
		}

		/*
		 * FIXME: multiple headers of the same name could lead to information
		 *        loss (multiple Cc: lines are allowed, for example)
		 */
		unfold_header(line);
		if ((ptr = parse_header(line, "From", TRUE, TRUE, FALSE))) {
			FreeIfNeeded(hdr->from);
			hdr->from = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "To", TRUE, TRUE, FALSE))) {
			FreeIfNeeded(hdr->to);
			hdr->to = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Cc", TRUE, TRUE, FALSE))) {
			FreeIfNeeded(hdr->cc);
			hdr->cc = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Bcc", TRUE, TRUE, FALSE))) {
			FreeIfNeeded(hdr->bcc);
			hdr->bcc = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Date", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->date);
			hdr->date = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Subject", TRUE, FALSE, TRUE))) {
			FreeIfNeeded(hdr->subj);
			hdr->subj = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Organization", TRUE, FALSE, TRUE))) {
			FreeIfNeeded(hdr->org);
			hdr->org = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Reply-To", TRUE, TRUE, FALSE))) {
			FreeIfNeeded(hdr->replyto);
			hdr->replyto = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Newsgroups", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->newsgroups);
			hdr->newsgroups = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Message-ID", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->messageid);
			hdr->messageid = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "References", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->references);
			hdr->references = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Distribution", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->distrib);
			hdr->distrib = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Keywords", TRUE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->keywords);
			hdr->keywords = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Summary", TRUE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->summary);
			hdr->summary = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Followup-To", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->followup);
			hdr->followup = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "X-Comment-To", TRUE, TRUE, FALSE))) {
			FreeIfNeeded(hdr->ftnto);
			hdr->ftnto = my_strdup(ptr);
			continue;
		}
#ifdef XFACE_ABLE
		if ((ptr = parse_header(line, "X-Face", FALSE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->xface);
			hdr->xface = my_strdup(ptr);
			continue;
		}
#endif /* XFACE_ABLE */
		/* TODO: check version */
		if (parse_header(line, "MIME-Version", FALSE, FALSE, FALSE)) {
			hdr->mime = TRUE;
			continue;
		}
		if ((ptr = parse_header(line, "Content-Type", FALSE, FALSE, FALSE))) {
			parse_content_type(ptr, hdr->ext);
			continue;
		}
		if ((ptr = parse_header(line, "Content-Transfer-Encoding", FALSE, FALSE, FALSE))) {
			hdr->ext->encoding = parse_content_encoding(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Content-Description", TRUE, FALSE, FALSE))) {
			FreeIfNeeded(hdr->ext->description);
			hdr->ext->description = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Content-Disposition", FALSE, FALSE, FALSE))) {
			parse_content_disposition(ptr, hdr->ext);
			continue;
		}
	}

	return tin_errno;
}


/*
 * Count lines in a continuated header.
 * line MUST NOT end in a newline.
 */
static int
count_lines(
	char *line)
{
	char *src = line;
	char c;
	int lines = 1;

	while ((c = *src++))
		if (c == '\n')
			lines++;
	return lines;
}


/*
 * Unfold header, i.e. strip any newline off it. Don't strip other
 * whitespace, it depends on the header if this is legal (structured
 * headers) or not (unstructured headers, e.g. Subject)
 */
void
unfold_header(
	char *line)
{
	char *src = line, *dst = line;
	char c;

	while ((c = *src++)) {
		if (c != '\n')
			*dst++ = c;
	}
	*dst = c;
}


#define M_SEARCHING	1	/* Looking for boundary */
#define M_HDR		2	/* In MIME headers */
#define M_BODY		3	/* In MIME body */

#define TIN_EOF		0xf00	/* Used internally for error recovery */

/*
 * Handles multipart/ article types, write data to a raw stream when reading via NNTP
 * artinfo is used for generic article pointers
 * part contains content info about the attachment we're parsing
 * depth is the number of levels by which the current part is embedded
 * Returns a tin_errno value which is '&'ed with TIN_EOF if the end of the
 * article is reached (to prevent broken articles from hanging the NNTP socket)
 */
static int
parse_multipart_article(
	FILE *infile,
	t_openartinfo *artinfo,
	t_part *part,
	int depth,
	t_bool show_progress_meter)
{
	char *line;
	char *ptr;
	int bnd;
	int state = M_SEARCHING;
	t_bool is_rfc822 = FALSE;
	t_part *curr_part = NULL, *rfc822_part = NULL;

	while ((line = tin_fgets(infile, (state == M_HDR))) != NULL) {
/* fprintf(stderr, "%d---:%s\n", depth, line); */

		/*
		 * Check current line for boundary markers
		 */
		bnd = boundary_check(line, artinfo->hdr.ext);

		if (read_news_via_nntp) {
			fprintf(artinfo->raw, "%s\n", line);
#ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "<<<%s%s", logtime(), line);
#endif /* DEBUG */
		}

		artinfo->hdr.ext->line_count += count_lines(line);
		if (show_progress_meter)
			progress(artinfo->hdr.ext->line_count);		/* Overall line count */

		if (part && part != artinfo->hdr.ext)
			part->line_count += count_lines(line);

		if (is_rfc822 && rfc822_part)
			rfc822_part->line_count += count_lines(line);

		if (bnd == BOUND_END) {							/* End of this part detected */
			if (is_rfc822 && rfc822_part)
				rfc822_part->line_count -= count_lines(line);
			/*
			 * When we have reached the end boundary of the outermost envelope
			 * just log any trailing data for the raw article format.
			 */
			if (boundary_cmp(line, get_param(artinfo->hdr.ext->params, "boundary")) == BOUND_END)
				depth = 0;
#if 0 /* doesn't count tailing lines after envelop mime part - correct but confusing */
			if (read_news_via_nntp && depth == 0)
				while ((line = tin_fgets(infile, FALSE)) != NULL)
					fprintf(artinfo->raw, "%s\n", line);
#else
			if (depth == 0) {
				while ((line = tin_fgets(infile, FALSE)) != NULL) {
					if (read_news_via_nntp)
						fprintf(artinfo->raw, "%s\n", line);
					artinfo->hdr.ext->line_count++;
				}
			}
#endif /* 0 */
			return tin_errno;
		}

		switch (state) {
			case M_SEARCHING:
				switch (bnd) {
					case BOUND_NONE:
						break;				/* Keep looking */

					case BOUND_START:
						state = M_HDR;		/* Now parsing headers of a part */
						curr_part = new_part(part);
						curr_part->depth = depth;
						break;
				}
				break;

			case M_HDR:
				switch (bnd) {
					case BOUND_START:
#ifdef DEBUG
						if (debug & DEBUG_MISC)
							error_message(2, _(txt_error_mime_start));
#endif /* DEBUG */
						continue;

					case BOUND_NONE:
						break;				/* Correct - No boundary */
				}

				if (*line == '\0') {		/* End of MIME headers */
					state = M_BODY;
					curr_part->offset = ftell(artinfo->raw);

					if (curr_part->type == TYPE_MULTIPART) {	/* Complex multipart article */
						int ret, old_line_count;

						old_line_count = curr_part->line_count;
						if ((ret = parse_multipart_article(infile, artinfo, curr_part, depth + 1, show_progress_meter)) != 0)
							return ret;							/* User abort or EOF reached */
						if (part && part != artinfo->hdr.ext)
							part->line_count += curr_part->line_count - old_line_count;
						if (is_rfc822 && rfc822_part)
							rfc822_part->line_count += curr_part->line_count - old_line_count;
					} else if (curr_part->type == TYPE_MESSAGE && !strcasecmp("RFC822", curr_part->subtype)) {
						is_rfc822 = TRUE;
						rfc822_part = curr_part;
						state = M_HDR;
						curr_part = new_part(part);
						curr_part->depth = ++depth;
					}
					break;
				}

				/*
				 * Keep headers that interest us
				 */
/* fprintf(stderr, "HDR:%s\n", line); */
				unfold_header(line);
				if ((ptr = parse_header(line, "Content-Type", FALSE, FALSE, FALSE))) {
					parse_content_type(ptr, curr_part);
					break;
				}
				if ((ptr = parse_header(line, "Content-Transfer-Encoding", FALSE, FALSE, FALSE))) {
					curr_part->encoding = parse_content_encoding(ptr);
					break;
				}
				if ((ptr = parse_header(line, "Content-Disposition", FALSE, FALSE, FALSE))) {
					parse_content_disposition(ptr, curr_part);
					break;
				}
				if ((ptr = parse_header(line, "Content-Description", TRUE, FALSE, FALSE))) {
					FreeIfNeeded(curr_part->description);
					curr_part->description = my_strdup(ptr);
					break;
				}
				break;

			case M_BODY:
				switch (bnd) {
					case BOUND_NONE:
/* fprintf(stderr, "BOD:%s\n", line); */
						curr_part->line_count++;
						break;

					case BOUND_START:		/* Start new attachment */
						if (is_rfc822) {
							--depth;
							rfc822_part->line_count--;
							rfc822_part = NULL;
							is_rfc822 = FALSE;
						}
						state = M_HDR;
						curr_part = new_part(part);
						curr_part->depth = depth;
						break;
				}
				break;
		} /* switch (state) */
	} /* while() */

	/*
	 * We only reach this point when we (unexpectedly) reach the end of the
	 * article
	 */
	return tin_errno | TIN_EOF;		/* Flag EOF */
}


/*
 * Parse a non-multipart article, merely a passthrough and bean counter
 */
static int
parse_normal_article(
	FILE *in,
	t_openartinfo *artinfo,
	t_bool show_progress_meter)
{
	char *line;

	while ((line = tin_fgets(in, FALSE)) != NULL) {
		if (read_news_via_nntp) {
			fprintf(artinfo->raw, "%s\n", line);
#ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "<<<%s%s", logtime(), line);
#endif /* DEBUG */
		}

		++artinfo->hdr.ext->line_count;

		if (show_progress_meter)
			progress(artinfo->hdr.ext->line_count);
	}
	return tin_errno;
}


#ifdef DEBUG_ART
/* DEBUG dump of what we got */
static void
dump_uue(
	t_part *ptr,
	t_openartinfo *art)
{
	if (ptr->uue != NULL) {
		t_part *uu;
		for (uu = ptr->uue; uu != NULL; uu = uu->next) {
			fprintf(stderr, "UU: %s\n", get_param(uu->params, "name"));
			fprintf(stderr, "    Content-Type: %s/%s\n    Content-Transfer-Encoding: %s\n",
				content_types[uu->type], uu->subtype,
				content_encodings[uu->encoding]);
			fprintf(stderr, "    Offset: %ld  Lines: %d\n", uu->offset, uu->line_count);
			fprintf(stderr, "    Depth: %d\n", uu->depth);
			fseek(art->raw, uu->offset, SEEK_SET);
			fprintf(stderr, "[%s]\n\n", tin_fgets(art->raw, FALSE));
		}
	}
}


static void
dump_art(
	t_openartinfo *art)
{
	t_part *ptr;
	t_param *pptr;
	struct t_header note_h = art->hdr;

	fprintf(stderr, "\nMain body\nMIME-Version: %u\n", note_h.mime);
	fprintf(stderr, "Content-Type: %s/%s\nContent-Transfer-Encoding: %s\n",
		content_types[note_h.ext->type], note_h.ext->subtype,
		content_encodings[note_h.ext->encoding]);
	if (note_h.ext->description)
		fprintf(stderr, "Content-Description: %s\n", note_h.ext->description);
	fprintf(stderr, "Offset: %ld\nLines: %d\n", note_h.ext->offset, note_h.ext->line_count);
	for (pptr = note_h.ext->params; pptr != NULL; pptr = pptr->next)
		fprintf(stderr, "P: %s = %s\n", pptr->name, pptr->value);
	dump_uue(note_h.ext, art);
	fseek(art->raw, note_h.ext->offset, SEEK_SET);
	fprintf(stderr, "[%s]\n\n", tin_fgets(art->raw, FALSE));
	fprintf(stderr, "\n");

	for (ptr = note_h.ext->next; ptr != NULL; ptr = ptr->next) {
		fprintf(stderr, "Attachment:\n");
		fprintf(stderr, "\tContent-Type: %s/%s\n\tContent-Transfer-Encoding: %s\n",
			content_types[ptr->type], ptr->subtype,
			content_encodings[ptr->encoding]);
		if (ptr->description)
			fprintf(stderr, "\tContent-Description: %s\n", ptr->description);
		fprintf(stderr, "\tOffset: %ld\n\tLines: %d\n", ptr->offset, ptr->line_count);
		fprintf(stderr, "\tDepth: %d\n", ptr->depth);
		for (pptr = ptr->params; pptr != NULL; pptr = pptr->next)
			fprintf(stderr, "\tP: %s = %s\n", pptr->name, pptr->value);
		dump_uue(ptr, art);
		fseek(art->raw, ptr->offset, SEEK_SET);
		fprintf(stderr, "[%s]\n\n", tin_fgets(art->raw, FALSE));
	}
}
#endif /* DEBUG_ART */


/*
 * Core parser for all article types
 * Return NULL if we couldn't open an output stream when reading via NNTP
 * When reading from local spool we assign the filehandle of the on-spool
 * article directly to artinfo->raw
 */
static int
parse_rfc2045_article(
	FILE *infile,
	int line_count,
	t_openartinfo *artinfo,
	t_bool show_progress_meter)
{
	int ret = ART_ABORT;

	if (read_news_via_nntp && !(artinfo->raw = tmpfile()))
		goto error;

	if (!read_news_via_nntp)
		artinfo->raw = infile;

	art_lines = line_count;

	if ((ret = parse_rfc822_headers(&artinfo->hdr, infile, artinfo->raw)) != 0)
		goto error;

	/*
	 * Is this a MIME article ?
	 * We don't bother to parse all plain text articles
	 */
	if (artinfo->hdr.mime && artinfo->hdr.ext->type == TYPE_MULTIPART) {
		if ((ret = parse_multipart_article(infile, artinfo, artinfo->hdr.ext, 1, show_progress_meter)) != 0) {
			/* Strip off EOF condition if present */
			if (ret & TIN_EOF) {
				ret ^= TIN_EOF;
#ifdef DEBUG
				if (debug & DEBUG_MISC)
					error_message(2, _(txt_error_mime_end), content_types[artinfo->hdr.ext->type], artinfo->hdr.ext->subtype);
#endif /* DEBUG */
				if (ret != 0)
					goto error;
			} else
				goto error;
		}
	} else {
		if ((ret = parse_normal_article(infile, artinfo, show_progress_meter)) != 0)
			goto error;
	}

	if (read_news_via_nntp)
		TIN_FCLOSE(infile);

	return 0;

error:
	if (read_news_via_nntp)
		TIN_FCLOSE(infile);
	art_close(artinfo);
	return ret;
}


/*
 * Open a mail/news article using NNTP ARTICLE command
 * or directly off local spool
 * Return:
 *		A pointer to the open postprocessed file
 *		NULL pointer if article open fails in some way
 */
FILE *
open_art_fp(
	struct t_group *group,
	t_artnum art)
{
	FILE *art_fp;

#ifdef NNTP_ABLE
	if (read_news_via_nntp && group->type == GROUP_TYPE_NEWS) {
		char buf[NNTP_STRLEN];
		snprintf(buf, sizeof(buf), "ARTICLE %"T_ARTNUM_PFMT, art);
		art_fp = nntp_command(buf, OK_ARTICLE, NULL, 0);
	} else {
#endif /* NNTP_ABLE */
		char buf[PATH_LEN];
		char pbuf[PATH_LEN];
		char fbuf[NAME_LEN + 1];
		char *group_path = my_malloc(strlen(group->name) + 2); /* tailing "/\0" */;

		make_group_path(group->name, group_path);
		joinpath(buf, sizeof(buf), group->spooldir, group_path);
		free(group_path);
		snprintf(fbuf, sizeof(fbuf), "%"T_ARTNUM_PFMT, art);
		joinpath(pbuf, sizeof(pbuf), buf, fbuf);

		art_fp = fopen(pbuf, "r");
#ifdef NNTP_ABLE
	}
#endif /* NNTP_ABLE */

	return art_fp;
}


/* ----------- art_open() and art_close() are the only interface --------- */
/* ------------------------for accessing articles ------------------- */

/*
 * Open's and postprocesses and article
 * Populates the passed in artinfo structure if successful
 *
 * Returns:
 *		0				Art opened successfully
 *		ART_UNAVAILABLE	Couldn't find article
 *		ART_ABORT		User aborted during read of article
 */
int
art_open(
	t_bool wrap_lines,
	struct t_article *art,
	struct t_group *group,
	t_openartinfo *artinfo,
	t_bool show_progress_meter,
	const char *pmesg)
{
	FILE *fp;

	memset(artinfo, 0, sizeof(t_openartinfo));

	if ((fp = open_art_fp(group, art->artnum)) == NULL)
		return ((tin_errno == 0) ? ART_UNAVAILABLE : ART_ABORT);

#ifdef DEBUG_ART
	fprintf(stderr, "art_open(%p)\n", (void *) artinfo);
#endif /* DEBUG_ART */

	progress_mesg = pmesg;
	if (parse_rfc2045_article(fp, art->line_count, artinfo, show_progress_meter) != 0) {
		progress_mesg = NULL;
		return ART_ABORT;
	}
	progress_mesg = NULL;

	/*
	 * TODO: compare art->msgid and artinfo->hdr.messageid and issue a
	 *       warning (once) about broken overviews if they differ
	 */

	if ((artinfo->tex2iso = ((group->attribute->tex2iso_conv) ? is_art_tex_encoded(artinfo->raw) : FALSE)))
		wait_message(0, _(txt_is_tex_encoded));

	/* Maybe fix it so if this fails, we default to raw? */
	if (!cook_article(wrap_lines, artinfo, tinrc.hide_uue, FALSE))
		return ART_ABORT;

#ifdef DEBUG_ART
	dump_art(artinfo);
#endif /* DEBUG_ART */

	/*
	 * If Newsgroups is empty it is a good bet the article is a mail article
	 * TODO: Why do this ?
	 */
	if (!artinfo->hdr.newsgroups)
		artinfo->hdr.newsgroups = my_strdup(group->name);

	return 0;
}


/*
 * Close an open article identified by an 'artinfo' handle
 */
void
art_close(
	t_openartinfo *artinfo)
{
#ifdef DEBUG_ART
	fprintf(stderr, "art_close(%p)\n", (void *) artinfo);
#endif /* DEBUG_ART */

	if (artinfo == NULL)
		return;

	free_and_init_header(&artinfo->hdr);

	artinfo->tex2iso = FALSE;

	if (artinfo->raw) {
		fclose(artinfo->raw);
		artinfo->raw = NULL;
	}

	if (artinfo->cooked) {
		fclose(artinfo->cooked);
		artinfo->cooked = NULL;
	}

	FreeAndNull(artinfo->rawl);
	FreeAndNull(artinfo->cookl);
}
