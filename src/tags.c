/*
 *  Project   : tin - a Usenet reader
 *  Module    : tags.c
 *  Author    : Jason Faultless <jason@radar.tele2.co.uk>
 *  Created   : 1999-12-06
 *  Updated   : 1999-12-06
 *  Notes     : Split out from other modules
 *
 * Copyright (c) 1999-2002 Jason Faultless <jason@radar.tele2.co.uk>
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
 *    This product includes software developed by Jason Faultless.
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

/* Local prototypes */
static int get_multipart_info (int base_index, MultiPartInfo *setme);
static int get_multiparts (int base_index, MultiPartInfo **malloc_and_setme_info);
static int look_for_multipart_info (int base_index, MultiPartInfo* setme, char start, char stop, int *offset);
static t_bool bParseRange (char *pcRange, int iNumMin, int iNumMax, int iNumCur, int *piRngMin, int *piRngMax);
static void vDelRange (int iLevel, int iNumMax);

int num_of_tagged_arts = 0;

/*
 * Parses a subject header of the type "multipart message subject (01/42)"
 * into a MultiPartInfo struct, or fails if the message subject isn't in the
 * right form.
 *
 * @return nonzero on success
 */
static int
get_multipart_info (
	int base_index,
	MultiPartInfo *setme)
{
	int i, j, offi, offj;
	MultiPartInfo setmei, setmej;

	i = look_for_multipart_info(base_index, &setmei, '[', ']', &offi);
	j = look_for_multipart_info(base_index, &setmej, '(', ')', &offj);

	/* Ok i hits first */
	if (offi > offj) {
		*setme = setmei;
		return i;
	}

	/* Its j or they are both the same (which must be zero!) so we don't care */
	*setme = setmej;
	return j;
}


static int
look_for_multipart_info (
	int base_index,
	MultiPartInfo* setme,
	char start,
	char stop,
	int *offset)
{
	MultiPartInfo tmp;
	char *subj = (char *) 0;
	char *pch = (char *) 0;

	*offset = 0;

	/* entry assertions */
	assert (0 <= base_index && base_index < grpmenu.max && "invalid base_index");
	assert (setme != NULL && "setme must not be NULL");

	/* parse the message */
	subj = arts[base[base_index]].subject;
	pch = strrchr (subj, start);
	if (!pch)
		return 0;
	if (!isdigit((int)pch[1]))
		return 0;
	tmp.base_index = base_index;
	tmp.subject_compare_len = pch - subj;
	tmp.part_number = (int) strtol(pch + 1, &pch, 10);
	if (*pch != '/' && *pch != '|')
		return 0;
	if (!isdigit((int)pch[1]))
		return 0;
	tmp.total = (int) strtol (pch + 1, &pch, 10);
	if (*pch != stop)
		return 0;
	tmp.subject = subj;
	*setme = tmp;
	*offset = pch-subj;
	return 1;
}


/*
 * Tries to find all the parts to the multipart message pointed to by
 * base_index.
 *
 * Weakness(?): only walks through the base messages.
 *
 * @return on success, the number of parts found.  On failure, zero if not a
 * multipart or the negative value of the first missing part.
 * @param base_index index pointing to one of the messages in a multipart
 * message.
 * @param malloc_and_setme_info on success, set to a malloced array the
 * parts found.  Untouched on failure.
 */
static int
get_multiparts (
	int base_index,
	MultiPartInfo **malloc_and_setme_info)
{
	MultiPartInfo tmp, tmp2;
	MultiPartInfo *info = 0;
	int i = 0;
	int part_index;

	/* entry assertions */
	assert (0 <= base_index && base_index < grpmenu.max && "Invalid base index");
	assert (malloc_and_setme_info != NULL && "malloc_and_setme_info must not be NULL");

	/* make sure this is a multipart message... */
	if (!get_multipart_info(base_index, &tmp) || tmp.total < 1)
		return 0;

	/* make a temporary buffer to hold the multipart info... */
	info = my_malloc (sizeof(MultiPartInfo) * tmp.total);

	/* zero out part-number for the repost check below */
	for (i = 0; i < tmp.total; ++i)
		info[i].part_number = -1;

	/* try to find all the multiparts... */
	for (i = 0; i < grpmenu.max; ++i) {
		if (strncmp (arts[base[i]].subject, tmp.subject, tmp.subject_compare_len))
			continue;
		if (!get_multipart_info (i, &tmp2))
			continue;

		part_index = tmp2.part_number - 1;

		/* skip the "blah (00/102)" info messages... */
		if (part_index < 0)
			continue;

		/* skip insane "blah (103/102) subjects... */
		if (part_index >= tmp.total)
			continue;

		/* repost check: do we already have this part? */
		if (info[part_index].part_number != -1) {
			assert (info[part_index].part_number == tmp2.part_number && "bookkeeping error");
			continue;
		}

		/* we have a match, hooray! */
		info[part_index] = tmp2;
	}

	/* see if we got them all. */
	for (i = 0; i < tmp.total; ++i) {
		if (info[i].part_number != i + 1) {
			free (info);
			return -(i + 1); /* missing part #(i+1) */
		}
	}

	/* looks like a success .. */
	*malloc_and_setme_info = info;
	return tmp.total;
}


/*
 * Tags all parts of a multipart index if base_index points
 * to a multipart message and all its parts can be found.
 *
 * @param base_index points to one message in a multipart message.
 * @return number of messages tagged, or zero on failure
 */
int
tag_multipart (
	int base_index)
{
	MultiPartInfo *info = 0;
	int i = 0;
	const int qty = get_multiparts(base_index, &info);

	/* check for failure... */
	if (qty == 0) {
		info_message (_("Not a multi-part message")); /* FIXME: -> lang.c */
		return 0;
	}
	if (qty < 0) {
		info_message(_("Missing part #%d"), -qty); /* FIXME: -> lang.c? */
		return 0;
	}

	/*
	 * if any are already tagged, untag 'em first
	 * so num_of_tagged_arts doesn't get corrupted
	 */
	for (i = 0; i < qty; ++i) {
		if (arts[base[info[i].base_index]].tagged != 0)
			remove_tag(base[info[i].base_index]);
	}

	/*
	 * get_multiparts() sorts info by part number,
	 * so a simple for loop tags in the right order
	 */
	for (i = 0; i < qty; ++i)
		arts[base[info[i].base_index]].tagged = ++num_of_tagged_arts;

	free (info);

	info_message (_("All parts tagged")); /* FIXME -> lang.c */
	return qty;
}


int
line_is_tagged (
	int n)
{
	int code = 0;

	if (CURR_GROUP.attribute->thread_arts) {
		int i;
		for (i = n; i >= 0; i = arts[i].thread) {
			if (arts[i].tagged > code)
				code = arts[i].tagged;
		}
	} else
		code = arts[n].tagged;

	return code;
}


/*
 * Toggle tag status of an article. Returns TRUE if we tagged the article
 * FALSE if we untagged it.
 */
t_bool
tag_article (
	int art)
{
	if (arts[art].tagged != 0) {
		remove_tag(art);
		info_message (_(txt_untagged_art));
		return FALSE;
	} else {
		arts[art].tagged = ++num_of_tagged_arts;
		info_message (_(txt_tagged_art));
		return TRUE;
	}
}


/*
 * Remove the tag from an article
 * Work through all the threads and decrement the tag counter on all arts
 * greater than 'tag', fixup counters
 */
void
remove_tag (
	long art)
{
	int i, j;

	for (i = 0; i < grpmenu.max; ++i) {
		for (j = (int) base[i]; j != -1; j = arts[j].thread)
			if (arts[j].tagged > arts[art].tagged)
				--arts[j].tagged;
	}
	arts[art].tagged = 0;
	--num_of_tagged_arts;
}


/*
 * Clear tag status of all articles. If articles were untagged, return TRUE
 */
t_bool
untag_all_articles (
	void)
{
	int i;
	t_bool untagged = FALSE;

	for (i = 0; i < top_art; i++) {
		if (arts[i].tagged != 0) {
			arts[i].tagged = 0;
			untagged = TRUE;
		}
	}
	num_of_tagged_arts = 0;

	return untagged;
}


/*
 * RANGE CODE
 */
/*
 * Allows user to specify an group/article range that a followup
 * command will operate on (eg. catchup articles 1-56) # 1-56 K
 *
 * Allowed syntax is 0123456789-.$ (blanks are ignored):
 *   1-23    mark grp/art 1 thru 23
 *   1-.     mark grp/art 1 thru current
 *   1-$     mark grp/art 1 thru last
 *   .-$     mark grp/art current thru last
 */
t_bool
bSetRange (
	int iLevel,
	int iNumMin,
	int iNumMax,
	int iNumCur)
{
	char *pcPtr;
	int iIndex;
	int iNum;
	int iRngMin;
	int iRngMax;
	t_bool bRetCode = FALSE;

	switch (iLevel) {
		case SELECT_LEVEL:
			pcPtr = tinrc.default_range_select;
			break;
		case GROUP_LEVEL:
			pcPtr = tinrc.default_range_group;
			break;
		case THREAD_LEVEL:
			pcPtr = tinrc.default_range_thread;
			break;
		default:
			return bRetCode;
	}

#if 0
	error_message ("Min=[%d] Max=[%d] Cur=[%d] DefRng=[%s]", iNumMin, iNumMax, iNumCur, pcPtr);
#endif /* 0 */
	sprintf (mesg, _(txt_enter_range), pcPtr);

	if (!(prompt_string_default(mesg, pcPtr, _(txt_range_invalid), HIST_OTHER)))
		return bRetCode;

	/*
	 * Parse range string
	 */
	if (!bParseRange (pcPtr, iNumMin, iNumMax, iNumCur, &iRngMin, &iRngMax))
		info_message (_(txt_range_invalid));
	else {
		bRetCode = TRUE;
		switch (iLevel) {
			case SELECT_LEVEL:
				vDelRange (iLevel, iNumMax);
				for (iIndex = iRngMin - 1; iIndex < iRngMax; iIndex++)
					active[my_group[iIndex]].inrange = TRUE;
				break;
			case GROUP_LEVEL:
				vDelRange (iLevel, iNumMax);
				for (iIndex = iRngMin - 1; iIndex < iRngMax; iIndex++) {
					for (iNum = (int) base[iIndex]; iNum != -1; iNum = arts[iNum].thread)
						arts[iNum].inrange = TRUE;
				}
				break;
			case THREAD_LEVEL:
				vDelRange (iLevel, selmenu.max);
				for (iNum = 0, iIndex = base[thread_basenote]; iIndex >= 0; iIndex = arts[iIndex].thread, iNum++) {
					if (iNum >= iRngMin && iNum <= iRngMax)
						arts[iIndex].inrange = TRUE;
				}
				break;
			default:
				bRetCode = FALSE;
				break;
		}
	}
	return bRetCode;
}


static t_bool
bParseRange (
	char *pcRange,
	int iNumMin,
	int iNumMax,
	int iNumCur,
	int *piRngMin,
	int *piRngMax)
{
	char *pcPtr;
	t_bool bRetCode = FALSE;
	t_bool bSetMax = FALSE;
	t_bool bDone = FALSE;

	pcPtr = pcRange;
	*piRngMin = -1;
	*piRngMax = -1;

	while (*pcPtr && !bDone) {
		if (*pcPtr >= '0' && *pcPtr <= '9') {
			if (bSetMax) {
				*piRngMax = atoi (pcPtr);
				bDone = TRUE;
			} else
				*piRngMin = atoi (pcPtr);
			while (*pcPtr >= '0' && *pcPtr <= '9')
				pcPtr++;
		} else {
			switch (*pcPtr) {
				case '-':
					bSetMax = TRUE;
					break;
				case '.':
					if (bSetMax) {
						*piRngMax = iNumCur;
						bDone = TRUE;
					} else
						*piRngMin = iNumCur;
					break;
				case '$':
					if (bSetMax) {
						*piRngMax = iNumMax;
						bDone = TRUE;
					}
					break;
				default:
					break;
			}
			pcPtr++;
		}
	}

	if (*piRngMin >= iNumMin && *piRngMax > iNumMin && *piRngMax <= iNumMax)
		bRetCode = TRUE;

	return bRetCode;
}


static void
vDelRange (
	int iLevel,
	int iNumMax)
{
	int iIndex;

	switch (iLevel) {
		case SELECT_LEVEL:
			for (iIndex = 0; iIndex < iNumMax - 1; iIndex++)
				active[iIndex].inrange = FALSE;
			break;
		case GROUP_LEVEL:
			{
				int iNum;
				for (iIndex = 0; iIndex < iNumMax - 1; iIndex++) {
					for (iNum = (int) base[iIndex]; iNum != -1; iNum = arts[iNum].thread)
						arts[iNum].inrange = FALSE;
				}
			}
			break;
		case THREAD_LEVEL:
			for (iIndex = 0; iIndex < iNumMax - 1; iIndex++)
				arts[iIndex].inrange = FALSE;
#if 0
			for (iIndex = base[thread_basenote]; iIndex >= 0; iIndex = arts[iIndex].thread)
				arts[iIndex].inrange = FALSE;
#endif /* 0 */
			break;
		default:
			break;
	}
}


/*
 * SELECTED CODE
 */
void
do_auto_select_arts (
	void)
{
	int i;

	for (i = 0; i < top_art; ++i) {
		if (arts[i].status == ART_UNREAD && arts[i].selected != 1) {
#	ifdef DEBUG_NEWSRC
			debug_print_comment ("group.c: X command");
#	endif /* DEBUG_NEWSRC */
			art_mark_read (&CURR_GROUP, &arts[i]);
			arts[i].zombie = TRUE;
		}
	}
	if (CURR_GROUP.attribute->show_only_unread)
		find_base (&CURR_GROUP);

	grpmenu.curr = 0;
	show_group_page ();
}


/* selection already happened in filter_articles() */
void
undo_auto_select_arts (
	void)
{
	int i;

	for (i = 0; i < top_art; ++i) {
		if (arts[i].status == ART_READ && arts[i].zombie) {
#	ifdef DEBUG_NEWSRC
			debug_print_comment ("group.c: + command");
#	endif /* DEBUG_NEWSRC */
			art_mark_unread (&CURR_GROUP, &arts[i]);
			arts[i].zombie = FALSE;
		}
	}
	if (CURR_GROUP.attribute->show_only_unread)
		find_base (&CURR_GROUP);

	grpmenu.curr = 0;	/* do we want this? */
	show_group_page ();
}


void
undo_selections (
	void)
{
	int i;

	for (i = 0; i < top_art; i++) {
		arts[i].selected = FALSE;
		arts[i].zombie = FALSE;
	}
}
