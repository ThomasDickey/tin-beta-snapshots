/*
 *  Project   : tin - a Usenet reader
 *  Module    : xref.c
 *  Author    : I. Lea & H. Brugge
 *  Created   : 1993-07-01
 *  Updated   : 2024-01-31
 *  Notes     :
 *
 * Copyright (c) 1993-2024 Iain Lea <iain@bricbrac.de>
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
#ifndef NEWSRC_H
#	include "newsrc.h"
#endif /* !NEWSRC_H */

/*
 * local prototypes
 */
static FILE *open_overview_fmt_fp(void);

struct t_overview_fmt *ofmt;
t_bool expensive_over_parse = FALSE;

/*
 * Open the NEWSLIBDIR/overview.fmt file locally or send LIST OVERVIEW.FMT
 */
static FILE *
open_overview_fmt_fp(
	void)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news) {
		if (!*nntp_caps.over_cmd)
			return (FILE *) 0;

		if ((nntp_caps.type == CAPABILITIES && nntp_caps.list_overview_fmt) || nntp_caps.type != CAPABILITIES)
			return (nntp_command("LIST OVERVIEW.FMT", OK_GROUPS, NULL, 0));

		return (FILE *) 0;
	}
#endif /* NNTP_ABLE */
	return (fopen(overviewfmt_file, "r"));
}


/*
 * Read overview.fmt file to check if Xref:full is enabled/disabled
 */
t_bool
overview_xref_support(
	void)
{
	FILE *fp;
	char *ptr, *p;
	t_bool supported = FALSE;
	size_t res_fields = 9; /* initial number of overview fields */
	size_t fields = 0;
	size_t i;

	ofmt[0].type = OVER_T_INT;
	ofmt[0].name = my_strdup("Artnum:");

	if ((fp = open_overview_fmt_fp()) != NULL) {
		while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
			if (*ptr == '#' || *ptr == '\n')	/* skip comments and empty lines */
				continue;

#if defined(DEBUG) && defined(NNTP_ABLE)
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#endif /* DEBUG && NNTP_ABLE */

			fields++;

			/* expand overview fmt array */
			if (fields >= res_fields) {
				res_fields <<= 1;
				ofmt = my_realloc(ofmt, sizeof(struct t_overview_fmt) * res_fields);
			}

			if ((p = strchr(ptr, ':'))) {
				if (p == ptr) { /* metadata items start with : */
					/* currently only :lines and :bytes are reserved */
					if (!strcasecmp(ptr, ":lines")) {
						ofmt[fields].type = OVER_T_INT;
						ofmt[fields].name = my_strdup("Lines:");
						if (fields != 7) {
							expensive_over_parse = TRUE;
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 7);
#endif /* DEBUG */
						}
						continue;
					}

					if (!strcasecmp(ptr, ":bytes")) {
						ofmt[fields].type = OVER_T_INT;
						ofmt[fields].name = my_strdup("Bytes:");
						if (fields != 6) {
							expensive_over_parse = TRUE;
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 6);
#endif /* DEBUG */
						}
						continue;
					}
					/* unknown metadata item */
				}

				/* non metadata items end with : or :full */
				/* optional items require :full */
				if (!strcasecmp(p, ":full")) {
					ofmt[fields].type = OVER_T_FSTRING;
					*(++p) = '\0';
					ofmt[fields].name = my_strdup(ptr);
					if (fields < 7) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected > %d", ptr, fields, 7);
#endif /* DEBUG */
					}
#ifdef DEBUG
					else {
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d", ptr, fields);
					}
#endif /* DEBUG */

					continue;
				}

				/* mandatory items */
				if (!strcasecmp(ptr, "Subject:")) {
					ofmt[fields].type = OVER_T_STRING;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 1) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 1);
#endif /* DEBUG */
					}
					continue;
				}

				if (!strcasecmp(ptr, "From:")) {
					ofmt[fields].type = OVER_T_STRING;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 2) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 2);
#endif /* DEBUG */
					}
					continue;
				}

				if (!strcasecmp(ptr, "Date:")) {
					ofmt[fields].type = OVER_T_STRING;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 3) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 3);
#endif /* DEBUG */
					}
					continue;
				}

				if (!strcasecmp(ptr, "Message-ID:")) {
					ofmt[fields].type = OVER_T_STRING;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 4) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 4);
#endif /* DEBUG */
					}
					continue;
				}

				if (!strcasecmp(ptr, "References:")) {
					ofmt[fields].type = OVER_T_STRING;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 5) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 5);
#endif /* DEBUG */
					}
					continue;
				}

				if (!strcasecmp(ptr, "Bytes:")) {
					ofmt[fields].type = OVER_T_INT;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 6) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 6);
#endif /* DEBUG */
					}
					continue;
				}

				if (!strcasecmp(ptr, "Lines:")) {
					ofmt[fields].type = OVER_T_INT;
					ofmt[fields].name = my_strdup(ptr);
					if (fields != 7) {
						expensive_over_parse = TRUE;
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "OVERVIEW.FMT: %s at position %d expected %d", ptr, fields, 7);
#endif /* DEBUG */
					}
					continue;
				}
			}
			/* bogus entry */
			ofmt[fields].type = OVER_T_ERROR;
			ofmt[fields].name = my_strdup(ptr);
		}
		TIN_FCLOSE(fp);
	}

	fields++;
	/* resize */
	ofmt = my_realloc(ofmt, sizeof(struct t_overview_fmt) * (fields + 1));

	/* end marker */
	ofmt[fields].type = OVER_T_ERROR;
	ofmt[fields].name = NULL;

	if (fields < 2) {
#ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", fp ? "OVERVIEW.FMT: Empty response - using safe defaults" : "OVERVIEW.FMT: not advertised - using safe defaults");
#endif /* DEBUG */
		ofmt = my_realloc(ofmt, sizeof(struct t_overview_fmt) * (8 + 1));
		ofmt[1].type = OVER_T_STRING;
		ofmt[1].name = my_strdup("Subject:");
		ofmt[2].type = OVER_T_STRING;
		ofmt[2].name = my_strdup("From:");
		ofmt[3].type = OVER_T_STRING;
		ofmt[3].name = my_strdup("Date:");
		ofmt[4].type = OVER_T_STRING;
		ofmt[4].name = my_strdup("Message-ID:");
		ofmt[5].type = OVER_T_STRING;
		ofmt[5].name = my_strdup("References:");
		ofmt[6].type = OVER_T_INT;
		ofmt[6].name = my_strdup("Bytes:");
		ofmt[7].type = OVER_T_INT;
		ofmt[7].name = my_strdup("Lines:");
		ofmt[8].type = OVER_T_ERROR;
		ofmt[8].name = NULL;
		fields = 8;
	}

	for (i = 0; i <= fields; i++) {
		if (ofmt[i].type == OVER_T_FSTRING) {
			if (!strcasecmp(ofmt[i].name, "Xref:"))
				supported = TRUE;
		}
	}

	/*
	 * If user aborted with 'q', then we continue regardless. If Xref was
	 * found, then fair enough. If not, tough. No real harm done
	 */
	/*
	 * TODO: warning message is not correct
	 *       - in the NNTP_ABLE but !read_news_via_nntp case when
	 *         OVERVIEW.FMT-file wasn't found or didn't mention Xref:
	 *       - if the used command is OVER instead of XOVER
	 *         (use nntp_caps.over_cmd in txt_warn_xref_not_supported ?)
	 *       - if the used command is HDR XREF instead of XHDR XREF
	 *         (use nntp_caps.hdr_cmd in txt_warn_xref_not_supported ?)
	 *       - if server doesn't mention XREF in LIST HEADERS
	 */
	if (read_news_via_nntp && !supported)
		wait_message(2, _(txt_warn_xref_not_supported));

	return supported;
}


/*
 * mark all other Xref: crossposted articles as read when one article read
 * Xref: sitename newsgroup:artnum newsgroup:artnum [newsgroup:artnum ...]
 */
void
art_mark_xref_read(
	struct t_article *art)
{
	char *xref_ptr;
	char *groupname;
	char *ptr, c;
	t_artnum artnum;
	struct t_group *group;
#ifdef DEBUG
	char *debug_mesg;
#endif /* DEBUG */

	if (art->xref == NULL)
		return;

	xref_ptr = art->xref;

	/* skip sitename */
	while (*xref_ptr != ' ' && *xref_ptr != '\t' && *xref_ptr)
		xref_ptr++;

	/*
	 * tokenize each pair and update that newsgroup if it is in my_group[].
	 */
	forever {
		while (*xref_ptr == ' ' || *xref_ptr == '\t')
			xref_ptr++;

		groupname = xref_ptr;
		while (*xref_ptr != ':' && *xref_ptr != ' ' && *xref_ptr != '\t' && *xref_ptr)
			xref_ptr++;

		if (*xref_ptr != ':')
			break;

		ptr = xref_ptr++;
		if ((artnum = atoartnum(xref_ptr)) <= 0)
			break;
		while (isdigit((unsigned char) *xref_ptr))
			xref_ptr++;

		if (&ptr[1] == xref_ptr)
			break;
		if (*xref_ptr && *xref_ptr != ' ' && *xref_ptr != '\t')
			break;

		c = *ptr;
		*ptr = '\0';
		group = group_find(groupname, FALSE);

#ifdef DEBUG
		if (debug & DEBUG_NEWSRC) {
			debug_mesg = fmt_string("LOOKUP Xref: [%s:%"T_ARTNUM_PFMT"] active=[%s] num_unread=[%"T_ARTNUM_PFMT"]",
				groupname, artnum,
				(group ? group->name : ""),
				(group ? group->newsrc.num_unread : 0));
			debug_print_comment(debug_mesg);
			debug_print_bitmap(group, NULL);
/*			error_message(2, debug_mesg); */
			free(debug_mesg);
		}
#endif /* DEBUG */

		if (group && group->newsrc.xbitmap) {
			if (artnum >= group->newsrc.xmin && artnum <= group->xmax) {
				if (!((NTEST(group->newsrc.xbitmap, artnum - group->newsrc.xmin) == ART_READ) ? TRUE : FALSE)) {
					NSET0(group->newsrc.xbitmap, artnum - group->newsrc.xmin);
					if (group->newsrc.num_unread > 0)
						group->newsrc.num_unread--;
#ifdef DEBUG
					if (debug & DEBUG_NEWSRC) {
						debug_mesg = fmt_string("FOUND!Xref: [%s:%"T_ARTNUM_PFMT"] marked READ num_unread=[%"T_ARTNUM_PFMT"]",
							groupname, artnum, group->newsrc.num_unread);
						debug_print_comment(debug_mesg);
						debug_print_bitmap(group, NULL);
/*						error_message(2, debug_mesg); */
						free(debug_mesg);
					}
#endif /* DEBUG */
				}
			}
		}
		*ptr = c;
	}
}


/*
 * Set bits [low..high] of 'bitmap' to 1's
 */
void
NSETRNG1(
	t_bitmap *bitmap,
	t_artnum low,
	t_artnum high)
{
	t_artnum i;

	assert(((void) "NSETRNG1() failed. bitmap == NULL", bitmap != NULL));

	if (high >= low) {
		if (NOFFSET(high) == NOFFSET(low)) {
			for (i = low; i <= high; i++) {
				NSET1(bitmap, i);
			}
		} else {
			BIT_OR(bitmap, low, (NBITSON << NBITIDX(low)));
			if (NOFFSET(high) > NOFFSET(low) + 1)
				memset(&bitmap[NOFFSET(low) + 1], NBITSON, (size_t) (NOFFSET(high) - NOFFSET(low) - 1));

			BIT_OR(bitmap, high, ~ (NBITNEG1 << NBITIDX(high)));
		}
	}
}


/*
 * Set bits [low..high] of 'bitmap' to 0's
 */
void
NSETRNG0(
	t_bitmap *bitmap,
	t_artnum low,
	t_artnum high)
{
	t_artnum i;

	assert(((void) "NSETRNG0() failed. bitmap == NULL", bitmap != NULL));

	if (high >= low) {
		if (NOFFSET(high) == NOFFSET(low)) {
			for (i = low; i <= high; i++) {
				NSET0(bitmap, i);
			}
		} else {
			BIT_AND(bitmap, low, ~(NBITSON << NBITIDX(low)));
			if (NOFFSET(high) > NOFFSET(low) + 1)
				memset(&bitmap[NOFFSET(low) + 1], 0, (size_t) (NOFFSET(high) - NOFFSET(low) - 1));

			BIT_AND(bitmap, high, NBITNEG1 << NBITIDX(high));
		}
	}
}
