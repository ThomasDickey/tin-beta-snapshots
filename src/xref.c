/*
 *  Project   : tin - a Usenet reader
 *  Module    : xref.c
 *  Author    : I. Lea & H. Brugge
 *  Created   : 1993-07-01
 *  Updated   : 2003-02-06
 *  Notes     :
 *
 * Copyright (c) 1993-2003 Iain Lea <iain@bricbrac.de>
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

/* dbmalloc checks memset() parameters, so we'll use it to check the assignments */
#ifdef USE_DBMALLOC
#	define BIT_OR(n, b, mask)	memset(n + NOFFSET(b), n[NOFFSET(b)] | (mask), 1)
#	define BIT_AND(n, b, mask)	memset(n + NOFFSET(b), n[NOFFSET(b)] & (mask), 1)
#else
#	define BIT_OR(n, b, mask)	n[NOFFSET(b)] |= mask
#	define BIT_AND(n, b, mask)	n[NOFFSET(b)] &= mask
#endif /* USE_DBMALLOC */

/*
 * local prototypes
 */
#if defined(NNTP_ABLE) && defined(XHDR_XREF)
	static void read_xref_header(struct t_article *art);
#endif /* NNTP_ABLE && XHDR_XREF */


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

			if (STRNCASECMPEQ(ptr, "Xref:full", 9)) {
				supported = TRUE;
#ifdef NNTP_ABLE
				drain_buffer(fp);
#endif /* NNTP_ABLE */
				break;
			}
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
	/* xref_supported means already supported in xover record */
	if (!xref_supported && read_news_via_nntp && art && !art->xref) {
		FILE *fp;
		char *ptr, *q;
		char buf[HEADER_LEN];
		long artnum = 0L;

		snprintf(buf, sizeof(buf) - 1, "XHDR XREF %ld", art->artnum);
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
				q = strchr(ptr, ' ');	/* skip article number */
				if (q == NULL)
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
	char *group;
	char *ptr, c;
	long artnum;
	struct t_group *psGrp;

#if defined(NNTP_ABLE) && defined(XHDR_XREF)
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

		group = xref_ptr;
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
		psGrp = group_find(group);

#ifdef DEBUG
		if (debug == 3) {
			sprintf(mesg, "LOOKUP Xref: [%s:%ld] active=[%s] num_unread=[%ld]",
				group, artnum,
				(psGrp ? psGrp->name : ""),
				(psGrp ? psGrp->newsrc.num_unread : 0));
#	ifdef DEBUG_NEWSRC
			debug_print_comment(mesg);
			debug_print_bitmap(psGrp, NULL);
#	endif /* DEBUG_NEWSRC */
			error_message(mesg);
		}
#endif /* DEBUG */

		if (psGrp && psGrp->newsrc.xbitmap) {
			if (artnum >= psGrp->newsrc.xmin && artnum <= psGrp->xmax) {
				if (!((NTEST(psGrp->newsrc.xbitmap, artnum - psGrp->newsrc.xmin) == ART_READ) ? TRUE : FALSE)) {
					NSET0(psGrp->newsrc.xbitmap, artnum - psGrp->newsrc.xmin);
					if (psGrp->newsrc.num_unread > 0)
						psGrp->newsrc.num_unread--;
#ifdef DEBUG
					if (debug == 3) {
						sprintf(mesg, "FOUND!Xref: [%s:%ld] marked READ num_unread=[%ld]",
							group, artnum, psGrp->newsrc.num_unread);
#	ifdef DEBUG_NEWSRC
						debug_print_comment(mesg);
						debug_print_bitmap(psGrp, NULL);
#	endif /* DEBUG_NEWSRC */
						wait_message(2, mesg);
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
	register long i;

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
	register long i;

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
