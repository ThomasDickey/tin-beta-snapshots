/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc1524.c
 *  Author    : Urs Janssen <urs@tin.org>, Jason Faultless <jason@radar.tele2.co.uk>
 *  Created   : 2000-05-15
 *  Updated   : 2000-06-13
 *  Notes     : mailcap parsing as defined in RFC 1524
 *
 * Copyright (c) 2000 Urs Janssen <urs@tin.org>, Jason Faultless <jason@radar.tele2.co.uk>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS
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
#endif /* RFC2046_H */

#define DEFAULT_MAILCAPS "~/.mailcap:/etc/mailcap:/usr/etc/mailcap:/usr/local/etc/mailcap"

/* maximum number of mailcap fields */
#define MAILCAPFIELDS 13

/* local prototypes */
static char *expand_mailcap_meta (const char *mailcap, t_part *part);
static char *get_mailcap_field (char *mailcap);
static t_mailcap *parse_mailcap_line (const char *mailcap, t_part *part);


/*
 * mainloop, find a mailcap file, look for a matching entry
 * extract fields, expand metas
 *
 * TODO: don't used fixed length buffers
 */
t_mailcap *
get_mailcap_entry (
	t_part *part)
{
	FILE *fp = (FILE *) 0;
	char *ptr, *ptr2;
	char buf[LEN];
	char mailcap[LEN];	/* full match */
	char mailcaps[LEN];	/* possible mailcap files */
	char wildcap[LEN];	/* basetype match */
	t_mailcap *foo = (t_mailcap *) 0;

	ptr = (char *) get_val("MAILCAPS", DEFAULT_MAILCAPS);
	STRCPY(mailcaps, ptr);
	ptr = strtok(mailcaps, ":");
	while (ptr != (char *) 0) {
		/* expand ~ and/or $HOME etc. */
		if (strfpath (ptr, mailcap, sizeof (mailcap), homedir, (char *) 0, (char *) 0, (char *) 0)) {
			if ((fp = fopen(mailcap, "r")) != (FILE *) 0)
				break;
			ptr = strtok(NULL, ":");
		}
	}
	if (!fp) /* no mailcaps file */
		return ((t_mailcap *) 0);

	/* (re)init some vars missued above */
	mailcap[0] = '\0';
	wildcap[0] = '\0';
	ptr = buf;

	while ((fgets (ptr, sizeof(buf) - strlen(buf), fp)) != NULL) {
		if (*ptr == '#' || *ptr == '\n')		/* skip comments & blank lines */
			continue;

		ptr = buf + strlen(buf) - 1;

		if (*ptr == '\n')		/* remove linebreaks */
			*ptr-- = '\0';

		if (*ptr == '\\')		/* continuation line */
			continue;			/* append */
		else
			ptr = buf;

		if ((ptr2 = strchr(buf, '/')) != (char *) 0) {
			if (!strncmp(ptr, content_types[part->type], strlen(ptr) - strlen(ptr2))) {
				if (!strncmp(ptr + strlen(content_types[part->type]) + 1, part->subtype, strlen(part->subtype))) {
					/* full match, so parse line and evaluate test if given. */
					STRCPY(mailcap, ptr);
					foo = parse_mailcap_line (mailcap, part);
					if (foo != (t_mailcap *) 0)
						break; /* perfect match with test succeded (if given) */
				} else {
					if ((*(ptr2 + 1) == '*') || (*(ptr2 + 1) == ';')) { /* wildmat match */
						if (!strlen(wildcap)) { /* we don't already have a wildmat match */
							STRCPY(wildcap, buf);
							foo = parse_mailcap_line (wildcap, part);
							if (foo == (t_mailcap *) 0) /* test failed */
								wildcap[0] = '\0'; /* ignore match */
						}
					} /* else subtype missmatch, no action required */
				}
			} /* else no match, no action required */
		} /* else invalid mailcap line (no /), no action required */
	}
	fclose (fp);
	return foo;
}


/*
 * extract fields, expand metas - called from get_mailcap_entry()
 */
static t_mailcap*
parse_mailcap_line(
	const char *mailcap,
	t_part *part)
{
	char *ptr, *optr, *buf;
	int i = MAILCAPFIELDS - 2; /* max MAILCAPFIELDS - required fileds */
	size_t blen;
	t_mailcap *tmailcap;

	/* malloc and init */
	tmailcap = (t_mailcap *) my_malloc (sizeof(t_mailcap));
	tmailcap->type = (char *) 0;
	tmailcap->command = (char *) 0;
	tmailcap->needsterminal = FALSE;
	tmailcap->copiousoutput = FALSE;
	tmailcap->textualnewlines = 0;
	tmailcap->description = (char *) 0;
	tmailcap->test = (char *) 0;
	tmailcap->nametemplate = (char *) 0;
	tmailcap->compose = (char *) 0;
	tmailcap->composetyped = (char *) 0;
	tmailcap->edit = (char *) 0;
	tmailcap->print = (char *) 0;
	tmailcap->x11bitmap = (char *) 0;

	optr = ptr = my_strdup(mailcap);

	/* get required entrys */
	ptr = get_mailcap_field(ptr);
	blen = strlen(content_types[part->type]) + strlen(part->subtype) + 2;
	buf = (char *) my_malloc(sizeof(char) * blen);
	memset(buf, 0, blen);
	sprintf (buf, "%s/%s", content_types[part->type], part->subtype);
	tmailcap->type = buf;
	ptr += strlen(ptr) + 1;
	if ((ptr = get_mailcap_field(ptr)) != (char *) 0) {
		tmailcap->command = expand_mailcap_meta(ptr, part);
		ptr += strlen(ptr) + 1;
	}

	while ((ptr = get_mailcap_field(ptr)) != (char *) 0) {
		if (i-- <= 0) /* number of possible fields exhausted */
			break;
		if (!strncasecmp(ptr, "needsterminal", 13)) {
			tmailcap->needsterminal = TRUE;
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "copiousoutput", 13)) {
			tmailcap->copiousoutput = TRUE;
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "description=", 12)) {
			tmailcap->description = expand_mailcap_meta(ptr + 12, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "nametemplate=", 13)) {
			tmailcap->nametemplate = expand_mailcap_meta(ptr + 13, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "test=", 5)) {
			tmailcap->test = expand_mailcap_meta(ptr + 5, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "textualnewlines=", 16)) {
			tmailcap->textualnewlines = atoi(ptr + 16);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "compose=", 8)) {
			tmailcap->compose = expand_mailcap_meta(ptr + 8, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "composetyped=", 13)) {
			tmailcap->composetyped = expand_mailcap_meta(ptr + 13, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "edit=", 5)) {
			tmailcap->edit = expand_mailcap_meta(ptr + 5, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "print=", 6)) {
			tmailcap->print = expand_mailcap_meta(ptr + 6, part);
			ptr += strlen(ptr) + 1;
		}
		if (!strncasecmp(ptr, "x11-bitmap=", 11)) {
			tmailcap->x11bitmap = expand_mailcap_meta(ptr + 11, part);
			ptr += strlen(ptr) + 1;
		}
	}

	free (optr);

	if (tmailcap->test != (char *) 0) { /* test field given */
		/* argh! system() */
		if (system(tmailcap->test) != 0) { /* test failed ? */
			free_mailcap (tmailcap);
			return ((t_mailcap *) 0);
		}
	}
	return tmailcap;
}


/*
 * extract fields - called from parse_mailcap_line()
 *
 * TODO: add handling for signlequotes
 */
static char *
get_mailcap_field(
	char *mailcap)
{
	char *ptr;
	t_bool backquote = FALSE;
	t_bool doublequote = FALSE;

	ptr = str_trim(mailcap);

	while(*ptr != '\0') {
		switch (*ptr) {
			case '\\':
				backquote = !backquote;
				break;

			case '"':
				if (!backquote)
					doublequote = !doublequote;
				backquote = FALSE;
				break;

			case ';':
				if (!backquote && !doublequote) { /* field seperator (plain ;) */
					*ptr = '\0';
					return mailcap;
				}
				if (backquote && !doublequote) /* remove \ in \; if not inside "" or '' */
					*(ptr-1) = ' ';
				backquote = FALSE;
				break;

			default:
				backquote = FALSE;
				break;
		}
		ptr++;
	}
	return mailcap;
}


/*
 * expand metas - called from parse_mailcap_line()
 *
 * TODO: expand %F, %n, %{...}
 */
static char *
expand_mailcap_meta(
	const char *mailcap,
	t_part *part)
{
	const char *ptr;
	char *line, *lptr;
	t_bool quote = FALSE;
	t_bool percent = FALSE;
	size_t linelen, space, olen;
	struct t_attribute *attr = CURR_GROUP.attribute;

 	if ((ptr = strchr(mailcap, '%')) == (char *) 0) /* nothing to expand */
		return my_strdup(mailcap); /* waste of mem, but simplyfies the frees */

	linelen = sizeof(char) * LEN * 2;		/* initial maxlen */
	space = linelen - 1;							/* available space in string */
	line = (char *) my_malloc (linelen); 	/* initial malloc */
	memset (line, 0, linelen);
	lptr = line;

	ptr = mailcap;

	while (*ptr != '\0') {
		/*
		 * to avoid reallocs() for the all the single char cases
		 * we do a check here
		 */
		if (space < (sizeof(char) * 10)) { /* 'worst'case are two chars ... */
			olen = strlen(line);		/* get current legth of string */
			space += linelen;			/* recalc available space */
			linelen *= 2;				/* double maxlen */
			line = (char *) my_realloc((void *) line, linelen);
			memset (line + olen, 0, linelen - olen); /* weed out junk */
			lptr = line + olen;		/* adjust pointer to current position */
		}

		switch (*ptr) {
			case '\\':
				quote = !quote;
				break;

			case '%':
				if (!quote)
					percent = TRUE;
				else {
					*lptr++ = '%';
					space -= sizeof(char);
					quote = FALSE;
				}
				break;

#if 0
			case '{':
				if (percent) {
					percent = FALSE;
					break;
				}
				/* FALLTHROUGH */

			case 'F':
				if (percent) {
					percent = FALSE;
					break;
				}
				/* FALLTHROUGH */

			case 'n':
				if (percent) {
					percent = FALSE;
					break;
				}
				/* FALLTHROUGH */
#endif /* 0 */

			case 's':
				if (percent) {
					char *nptr;

					if ((nptr = get_filename(part->params)) == (char *) 0)
						nptr = attr->savefile ? attr->savefile : tinrc.default_save_file;

					while (space <= (strlen(attr->savedir) + 1 + strlen(nptr) + 2)) { /* not enough space? */
						olen = strlen(line);		/* get current legth of string */
						space += linelen;			/* recalc available space */
						linelen *= 2;				/* double maxlen */
						line = (char *) my_realloc((void *) line, linelen);
						memset(line + olen, 0, linelen - olen);	/* weed out junk */
					}
					strcat(line, attr->savedir);
					strcat(line, "/");
					strcat(line, nptr);
					lptr = line + strlen(line);
					space -= strlen(line);
					percent = FALSE;
					break;
				}
				/* FALLTHROUGH */

			case 't':
				if (percent) {
					while (space <= (strlen(content_types[part->type]) + 1 + strlen(part->subtype))) { /* not enough space? */
						olen = strlen(line);		/* get current legth of string */
						space += linelen;       /* recalc available space */
						linelen *= 2;				/* double maxlen */
						line = (char *) my_realloc((void *) line, linelen);
						memset(line + olen, 0, linelen - olen);	/* weed out junk */
					}
					strcat(line, content_types[part->type]);
					strcat(line, "/");
					strcat(line, part->subtype);
					lptr = line + strlen(line);
					space -= strlen(line);
					percent = FALSE;
					break;
				}
				/* FALLTHROUGH */

			default:
				if (quote) { /* last char was \ */
					*lptr = '\\';
					lptr++;
					space -= sizeof(char);
					quote = FALSE;
				}
				if (percent) { /* unknow %x sequence */
					*lptr = '%';
					lptr++;
					space -= sizeof(char);
					percent = FALSE;
					quote = FALSE;
				}
				*lptr = *ptr;
				lptr++;
				space -= sizeof(char);
		}
		ptr++;
	}
	return line;
}


/*
 * frees the malloced space
 */
void
free_mailcap (
	t_mailcap *tmailcap)
{
	FreeIfNeeded(tmailcap->type);
	FreeIfNeeded(tmailcap->command);
	FreeIfNeeded(tmailcap->compose);
	FreeIfNeeded(tmailcap->composetyped);
	FreeIfNeeded(tmailcap->description);
	FreeIfNeeded(tmailcap->edit);
	FreeIfNeeded(tmailcap->nametemplate);
	FreeIfNeeded(tmailcap->print);
	FreeIfNeeded(tmailcap->test);
	FreeIfNeeded(tmailcap->x11bitmap);
	FreeIfNeeded((char *)tmailcap);
}
