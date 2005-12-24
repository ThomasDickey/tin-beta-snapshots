/*
 *  Project   : tin - a Usenet reader
 *  Module    : xref.c
 *  Author    : I. Lea & H. Brugge
 *  Created   : 1993-07-01
 *  Updated   : 2005-05-04
 *  Notes     :
 *
 * Copyright (c) 1993-2006 Iain Lea <iain@bricbrac.de>
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
#ifndef NEWSRC_H
#	include "newsrc.h"
#endif /* !NEWSRC_H */

/*
 * local prototypes
 */
#if defined(NNTP_ABLE) && defined(XHDR_XREF)
	static void read_xref_header(struct t_article *art);
#endif /* NNTP_ABLE && XHDR_XREF */
static FILE *open_overview_fmt_fp(void);


/*
 * Open the NEWSLIBDIR/overview.fmt file locally or send LIST OVERVIEW.FMT
 */
static FILE *
open_overview_fmt_fp(
	void)
{
	char line[NNTP_STRLEN];

#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news) {
		if (!nntp_caps.over_cmd)
			return (FILE *) 0;

		snprintf(line, sizeof(line), "LIST %s", OVERVIEW_FMT);
		return (nntp_command(line, OK_GROUPS, NULL, 0));
	} else {
#endif /* NNTP_ABLE */
		joinpath(line, libdir, OVERVIEW_FMT);
		return (fopen(line, "r"));
#ifdef NNTP_ABLE
	}
#endif /* NNTP_ABLE */
}


/*
 * Read NEWSLIBDIR/overview.fmt file to check if Xref:full is enabled/disabled
 */
t_bool
overview_xref_support(
	void)
{
	FILE *fp;
	char *ptr;
	t_bool supported = FALSE;

	if ((fp = open_overview_fmt_fp()) != NULL) {
		while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
#if defined(DEBUG) && defined(NNTP_ABLE)
			debug_nntp("<<<", ptr);
#endif /* DEBUG && NNTP_ABLE */
			if (!supported && STRNCASECMPEQ(ptr, "Xref:full", 9))
				supported = TRUE;
		}
		TIN_FCLOSE(fp);
		/*
		 * If user aborted with 'q', then we continue regardless. If Xref was
		 * found, then fair enough. If not, tough. No real harm done
		 */
	}

	if (!supported)
		wait_message(2, _(txt_warn_xref_not_supported));

	return supported;
}


/*
 * read xref reference for current article
 * This enables crosspost marking even if the xref records are not
 * part of the xover record.
 */
#if defined(NNTP_ABLE) && defined(XHDR_XREF)
static void
read_xref_header(
	struct t_article *art)
{
	FILE *fp;
	char *ptr, *q;
	char buf[HEADER_LEN];
	long artnum;

	snprintf(buf, sizeof(buf), "XHDR XREF %ld", art->artnum);
	if ((fp = nntp_command(buf, OK_HEAD, NULL, 0)) == NULL)
		return;

	while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
		while (*ptr && isspace((int) *ptr))
			ptr++;
		if (*ptr == '.')
			break;
		/*
		 * read the article number
		 */
		artnum = atol(ptr);
		if ((artnum == art->artnum) && !art->xref && !strstr(ptr, "(none)")) {
			if ((q = strchr(ptr, ' ')) == NULL)	/* skip article number */
				continue;
			ptr = q;
			while (*ptr && isspace((int) *ptr))
				ptr++;
			q = strchr(ptr, '\n');
			if (q)
				*q = '\0';
			art->xref = my_strdup(ptr);
		}
	}
	return;
}
#endif /* NNTP_ABLE && XHDR_XREF */


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
	long artnum;
	struct t_group *group;
#ifdef DEBUG
	char *debug_mesg;
#endif /* DEBUG */

#if defined(NNTP_ABLE) && defined(XHDR_XREF)
	/* xref_supported => xref info was already read in xover record */
	if (!xref_supported && read_news_via_nntp && art && !art->xref)
		read_xref_header(art);
#endif /* NNTP_ABLE && XHDR_XREF */

	if (art->xref == NULL)
		return;

	xref_ptr = art->xref;

	/*
	 * check sitename matches nodename of current machine (ignore for now!)
	 */
	while (*xref_ptr != ' ' && *xref_ptr)
		xref_ptr++;

	/*
	 * tokenize each pair and update that newsgroup if it is in my_group[].
	 */
	forever {
		while (*xref_ptr == ' ')
			xref_ptr++;

		groupname = xref_ptr;
		while (*xref_ptr != ':' && *xref_ptr)
			xref_ptr++;

		if (*xref_ptr != ':')
			break;

		ptr = xref_ptr++;
		artnum = atol(xref_ptr);
		while (isdigit((int) *xref_ptr))
			xref_ptr++;

		if (&ptr[1] == xref_ptr)
			break;

		c = *ptr;
		*ptr = '\0';
		group = group_find(groupname);

#ifdef DEBUG
		if (debug == 3) {
			debug_mesg = fmt_string("LOOKUP Xref: [%s:%ld] active=[%s] num_unread=[%ld]",
				groupname, artnum,
				(group ? group->name : ""),
				(group ? group->newsrc.num_unread : 0));
#	ifdef DEBUG_NEWSRC
			debug_print_comment(debug_mesg);
			debug_print_bitmap(group, NULL);
#	endif /* DEBUG_NEWSRC */
			error_message(debug_mesg);
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
					if (debug == 3) {
						debug_mesg = fmt_string("FOUND!Xref: [%s:%ld] marked READ num_unread=[%ld]",
							groupname, artnum, group->newsrc.num_unread);
#	ifdef DEBUG_NEWSRC
						debug_print_comment(debug_mesg);
						debug_print_bitmap(group, NULL);
#	endif /* DEBUG_NEWSRC */
						wait_message(2, debug_mesg);
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
	long low,
	long high)
{
	long i;

	if (bitmap == NULL) {
		error_message("NSETRNG1() failed. Bitmap == NULL");
		return;
	}

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
	long low,
	long high)
{
	long i;

	if (bitmap == NULL) {
		error_message("NSETRNG0() failed. Bitmap == NULL");
		return;
	}

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
