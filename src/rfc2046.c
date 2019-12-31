/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2046.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 2000-02-18
 *  Updated   : 2003-06-09
 *  Notes     : RFC 2046 MIME article parsing
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
#ifndef RFC2046_H
#	include "rfc2046.h"
#endif /* !RFC2046_H */


/*
 * local prototypes
 */
static int boundary_cmp(const char *line, const char *boundary);
static int count_lines(char *line);
static int parse_multipart_article(FILE *infile, t_openartinfo *artinfo, t_part *part, int depth, t_bool show_progress_meter);
static int parse_normal_article(FILE *in, t_openartinfo *artinfo, t_bool show_progress_meter);
static int parse_rfc2045_article(FILE *infile, int line_count, t_openartinfo *artinfo, t_bool show_progress_meter);
static unsigned int parse_content_encoding(const char *encoding);
static void free_list(t_param *list);
static void parse_content_type(char *type, t_part *content);
static void parse_content_disposition(char *disp, t_part *part);
static void parse_params(char *params, t_part *content);
static void progress(int line_count);
#ifdef DEBUG_ART
	static void dump_art(t_openartinfo *art);
#endif /* DEBUG_ART */


/*
 * Local variables
 */
static int art_lines = 0;		/* lines in art on spool */

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
	if (line_count && line_count % MODULO_COUNT_NUM == 0)
		show_progress(mesg, line_count, art_lines);
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

	for (i = 0; i < NUM_CONTENT_TYPES; ++i) {
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
	int nl;

	if ((len = strlen(line)) == 0)
		return BOUND_NONE;

	nl = line[len - 1] == '\n';

	if (len != blen + 2 + nl && len != blen + 4 + nl)
		return BOUND_NONE;

	if (line[0] != '-' || line[1] != '-')
		return BOUND_NONE;

	if (strncmp(line + 2, boundary, blen) != 0)
		return BOUND_NONE;

	if (line[blen + 2] != '-') {
		if (nl ? line[blen + 2] == '\n' : line[blen + 2] == '\0')
			return BOUND_START;
		else
			return BOUND_NONE;
	}

	if (line[blen + 3] != '-')
		return BOUND_NONE;

	if (nl ? line[blen + 4] == '\n' : line[blen + 4] == '\0')
		return BOUND_END;
	else
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
	char *eql, *param;
	t_param *ptr;

	for (param = strtok(params, ";"); param; param = strtok(NULL, PARAM_SEP)) {
		if ((eql = strchr(param, '=')) == NULL)
			continue;						/* No =, Malformed param */

		*eql++ = '\0';						/* Split at = */

		ptr = my_malloc(sizeof(t_param));
		str_trim(param);
		ptr->name = my_strdup(param);

		str_trim(eql);						/* See if in "" */
		if (*eql == '"' && (param = strrchr(eql, '"')) != NULL) {
			eql++;
			*param = '\0';
		}
		ptr->value = my_strdup(rfc1522_decode(eql));
		ptr->next = content->params;		/* Push onto start of list */
		content->params = ptr;
	}
}


/*
 * Free up a generic list object
 */
static void
free_list(
	t_param *list)
{
	while (list->next != NULL) {
		free_list(list->next);
		list->next = NULL;
	}

	free(list->name);
	free(list->value);
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
	for (; list != NULL; list = list->next) {
		if (strcasecmp(name, list->name) == 0)
			return list->value;
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
#ifndef CHARSET_CONVERSION
		char defparms[] = CT_DEFPARMS;	/* must be writeable */
#endif /* !CHARSET_CONVERSION */

		free_list(content->params);
		content->params = NULL;
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
				char defparms[] = CT_DEFPARMS;	/* must be writeable */

				parse_params(defparms, content);
			}
#endif /* !CHARSET_CONVERSION */
		}
	}
}


static unsigned int
parse_content_encoding(
	const char *encoding)
{
	unsigned int i;

	for (i = 0; i < NUM_ENCODINGS; ++i) {
		if (strcasecmp(encoding, content_encodings[i]) == 0)
		return i;
	}

	/*
	 * TODO: check rfc - may need to switch Content-Type to
	 * application/octet-steam where this header exists but is unparseable.
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
	char defparms[] = CT_DEFPARMS;	/* must be writeable */
#endif /* !CHARSET_CONVERSION */

	ptr->type = TYPE_TEXT;					/* Defaults per RFC */
	ptr->subtype = my_strdup("plain");
	ptr->description = NULL;
	ptr->encoding = ENCODING_7BIT;
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
		char defparms[] = CT_DEFPARMS;	/* must be writeable */

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
 *
 * TODO: plug mem leak: ptr->uue is not entirely freed
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
	t_bool structured)
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
		return ptr;

	if (decode) {
		if (structured) {
			char addr[HEADER_LEN];
			char name[HEADER_LEN];
			int type;

			if (gnksa_split_from(ptr, addr, name, &type) == GNKSA_OK) {
				if (*name) {
					if (type == GNKSA_ADDRTYPE_OLDSTYLE)
						sprintf(ptr, "%s (%s)", addr, rfc1522_decode(name));
					else
						sprintf(ptr, "%s <%s>", rfc1522_decode(name), addr);
				} else
					sprintf(ptr, "%s", addr);
			} else
				return ptr;
		} else
			return (rfc1522_decode(ptr));
	}

	return ptr;
}


/*
 * Read main article headers into a blank header structure.
 * Pass the data 'from' -> 'to'
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
		if (to)
			fprintf(to, "%s\n", line);		/* Put raw data */

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
		if ((ptr = parse_header(line, "From", TRUE, TRUE))) {
			FreeIfNeeded(hdr->from);
			hdr->from = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "To", TRUE, TRUE))) {
			FreeIfNeeded(hdr->to);
			hdr->to = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Cc", TRUE, TRUE))) {
			FreeIfNeeded(hdr->cc);
			hdr->cc = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Bcc", TRUE, TRUE))) {
			FreeIfNeeded(hdr->bcc);
			hdr->bcc = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Date", FALSE, FALSE))) {
			FreeIfNeeded(hdr->date);
			hdr->date = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Subject", TRUE, FALSE))) {
			FreeIfNeeded(hdr->subj);
			hdr->subj = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Organization", TRUE, FALSE))) {
			FreeIfNeeded(hdr->org);
			hdr->org = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Reply-To", TRUE, TRUE))) {
			FreeIfNeeded(hdr->replyto);
			hdr->replyto = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Newsgroups", FALSE, FALSE))) {
			FreeIfNeeded(hdr->newsgroups);
			hdr->newsgroups = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Message-ID", FALSE, FALSE))) {
			FreeIfNeeded(hdr->messageid);
			hdr->messageid = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "References", FALSE, FALSE))) {
			FreeIfNeeded(hdr->references);
			hdr->references = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Distribution", TRUE, FALSE))) {
			FreeIfNeeded(hdr->distrib);
			hdr->distrib = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Keywords", TRUE, FALSE))) {
			FreeIfNeeded(hdr->keywords);
			hdr->keywords = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Summary", TRUE, FALSE))) {
			FreeIfNeeded(hdr->summary);
			hdr->summary = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Followup-To", FALSE, FALSE))) {
			FreeIfNeeded(hdr->followup);
			hdr->followup = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "X-Comment-To", TRUE, TRUE))) {
			FreeIfNeeded(hdr->ftnto);
			hdr->ftnto = my_strdup(ptr);
			continue;
		}
#ifdef XFACE_ABLE
		if ((ptr = parse_header(line, "X-Face", FALSE, FALSE))) {
			FreeIfNeeded(hdr->xface);
			hdr->xface = my_strdup(ptr);
			continue;
		}
#endif /* XFACE_ABLE */
		/* TODO: check version */
		if (parse_header(line, "MIME-Version", FALSE, FALSE)) {
			hdr->mime = TRUE;
			continue;
		}
		if ((ptr = parse_header(line, "Content-Type", FALSE, FALSE))) {
			parse_content_type(ptr, hdr->ext);
			continue;
		}
		if ((ptr = parse_header(line, "Content-Transfer-Encoding", FALSE, FALSE))) {
			hdr->ext->encoding = parse_content_encoding(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Content-Description", TRUE, FALSE))) {
			FreeIfNeeded(hdr->ext->description);
			hdr->ext->description = my_strdup(ptr);
			continue;
		}
		if ((ptr = parse_header(line, "Content-Disposition", FALSE, FALSE))) {
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

	while ((c = *src++))
		if (c != '\n') *dst++ = c;
	*dst = c;
}


#define M_SEARCHING	1	/* Looking for boundary */
#define M_HDR		2	/* In MIME headers */
#define M_BODY		3	/* In MIME body */

#define TIN_EOF		0xf00	/* Used internally for error recovery */

/*
 * Handles multipart/ article types, write data to a raw stream
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
	t_part *curr_part = 0;

	while ((line = tin_fgets(infile, (state == M_HDR))) != NULL) {
/* fprintf(stderr, "%d---:%s\n", depth, line); */

		/*
		 * Check current line for boundary markers
		 */
		bnd = boundary_check(line, artinfo->hdr.ext);

		fprintf(artinfo->raw, "%s\n", line);

		artinfo->hdr.ext->line_count += count_lines(line);
		if (show_progress_meter)
			progress(artinfo->hdr.ext->line_count);		/* Overall line count */

		if (bnd == BOUND_END) {							/* End of this part detected */
			/*
			 * When we have reached the end boundary of the outermost envelope
			 * just log any trailing data for the raw article format.
			 */
			if (depth == 0)
				while ((line = tin_fgets(infile, FALSE)) != NULL)
					fprintf(artinfo->raw, "%s\n", line);
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
						error_message(_(txt_error_mime_start));
						continue;

					case BOUND_NONE:
						break;				/* Correct - No boundary */
				}

				if (*line == '\0') {		/* End of MIME headers */
					state = M_BODY;
					curr_part->offset = ftell(artinfo->raw);
					break;
				}

				/*
				 * Keep headers that interest us
				 */
/*fprintf(stderr, "HDR:%s\n", line);*/
				unfold_header(line);
				if ((ptr = parse_header(line, "Content-Type", FALSE, FALSE))) {
					parse_content_type(ptr, curr_part);

					if (curr_part->type == TYPE_MULTIPART) {	/* Complex multipart article */
						int ret;

						if ((ret = parse_multipart_article(infile, artinfo, curr_part, depth + 1, show_progress_meter)) != 0)
							return ret;							/* User abort or EOF reached */
						else
							break;
					}
				}
				if ((ptr = parse_header(line, "Content-Transfer-Encoding", FALSE, FALSE))) {
					curr_part->encoding = parse_content_encoding(ptr);
					break;
				}
				if ((ptr = parse_header(line, "Content-Disposition", FALSE, FALSE))) {
					parse_content_disposition(ptr, curr_part);
					break;
				}
				if ((ptr = parse_header(line, "Content-Description", TRUE, FALSE))) {
					FreeIfNeeded(curr_part->description);
					curr_part->description = my_strdup(ptr);
					break;
				}
				break;

			case M_BODY:
				switch (bnd) {
					case BOUND_NONE:
/*fprintf(stderr, "BOD:%s\n", line);*/
						curr_part->line_count++;
						break;

					case BOUND_START:		/* Start new attchment */
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
		fprintf(artinfo->raw, "%s\n", line);
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

	fprintf(stderr, "\nMain body\nMIME-Version: %d\n", note_h.mime);
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
 * Return NULL if we couldn't open an output stream
 */
static int
parse_rfc2045_article(
	FILE *infile,
	int line_count,
	t_openartinfo *artinfo,
	t_bool show_progress_meter)
{
	int ret;

	if (!infile || !(artinfo->raw = tmpfile()))
		return ART_ABORT;

	art_lines = line_count;

	if ((ret = parse_rfc822_headers(&artinfo->hdr, infile, artinfo->raw)) != 0)
		goto error;

	/*
	 * Is this a MIME article ?
	 * We don't bother to parse all plain text articles
	 */
	if (artinfo->hdr.mime && artinfo->hdr.ext->type == TYPE_MULTIPART) {
		if ((ret = parse_multipart_article(infile, artinfo, artinfo->hdr.ext, 0, show_progress_meter)) != 0) {
			/* Strip off EOF condition if present */
			if (ret & TIN_EOF) {
				ret ^= TIN_EOF;
				error_message(_(txt_error_mime_end), content_types[artinfo->hdr.ext->type], artinfo->hdr.ext->subtype);
				if (ret != 0)
					goto error;
			} else
				goto error;
		}
	} else {
		if ((ret = parse_normal_article(infile, artinfo, show_progress_meter)) != 0)
			goto error;
	}

	TIN_FCLOSE(infile);

	return 0;

error:
	TIN_FCLOSE(infile);
	art_close(artinfo);
	return ret;
}


/*----------- art_open() and art_close() are the only interface ---------*/
/*------------------------for accessing articles -------------------*/

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
	t_bool show_progress_meter)
{
	FILE *fp;

	memset(artinfo, 0, sizeof(t_openartinfo));

	if ((fp = open_art_fp(group, art->artnum)) == NULL)
		return ((tin_errno == 0) ? ART_UNAVAILABLE : ART_ABORT);

#ifdef DEBUG_ART
	fprintf(stderr, "art_open(%p)\n", (void *) artinfo);
#endif /* DEBUG_ART */

	if (parse_rfc2045_article(fp, art->line_count, artinfo, show_progress_meter) != 0)
		return ART_ABORT;

	/*
	 * TODO: compare art->msgid and artinfo->hdr.messageid and issue a
	 *       warinng (once) about broken overviews if they differ
	 */

	if ((artinfo->tex2iso = ((group->attribute->tex2iso_conv) ? is_art_tex_encoded(artinfo->raw) : FALSE)))
		wait_message(0, _(txt_is_tex_encoded));

	/* Maybe fix it so if this fails, we default to raw? */
	if (!cook_article(wrap_lines, artinfo, 8, tinrc.hide_uue))
		return ART_ABORT;

#ifdef DEBUG_ART
	dump_art(artinfo);
#endif /* DEBUG_ART */

	/*
	 * If Newsgroups is empty its a good bet the article is a mail article
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
