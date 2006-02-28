/*
 *  Project   : tin - a Usenet reader
 *  Module    : tags.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1999-12-06
 *  Updated   : 2005-05-10
 *  Notes     : Split out from other modules
 *
 * Copyright (c) 1999-2006 Jason Faultless <jason@altarstone.com>
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

/* Local prototypes */
static int get_multipart_info(int base_index, MultiPartInfo *setme);
static int get_multiparts(int base_index, MultiPartInfo **malloc_and_setme_info);
static int look_for_multipart_info(int base_index, MultiPartInfo *setme, char start, char stop, int *offset);
static t_bool parse_range(char *range, int min, int max, int curr, int *range_start, int *range_end);

int num_of_tagged_arts = 0;

/*
 * Parses a subject header of the type "multipart message subject (01/42)"
 * into a MultiPartInfo struct, or fails if the message subject isn't in the
 * right form.
 *
 * @return nonzero on success
 */
static int
get_multipart_info(
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
look_for_multipart_info(
	int base_index,
	MultiPartInfo* setme,
	char start,
	char stop,
	int *offset)
{
	MultiPartInfo tmp;
	char *subj;
	char *pch;

	*offset = 0;

	/* entry assertions */
	assert(0 <= base_index && base_index < grpmenu.max && "invalid base_index");
	assert(setme != NULL && "setme must not be NULL");

	/* parse the message */
	subj = arts[base[base_index]].subject;
	if (!(pch = strrchr(subj, start)))
		return 0;
	if (!isdigit((int) pch[1]))
		return 0;
	tmp.base_index = base_index;
	tmp.subject_compare_len = pch - subj;
	tmp.part_number = (int) strtol(pch + 1, &pch, 10);
	if (*pch != '/' && *pch != '|')
		return 0;
	if (!isdigit((int) pch[1]))
		return 0;
	tmp.total = (int) strtol(pch + 1, &pch, 10);
	if (*pch != stop)
		return 0;
	tmp.subject = subj;
	*setme = tmp;
	*offset = pch - subj;
	return 1;
}


/*
 * Tries to find all the parts to the multipart message pointed to by
 * base_index.
 *
 * Weakness(?): only walks through the base messages.
 *
 * @return on success, the number of parts found. On failure, zero if not a
 * multipart or the negative value of the first missing part.
 * @param base_index index pointing to one of the messages in a multipart
 * message.
 * @param malloc_and_setme_info on success, set to a malloced array the
 * parts found. Untouched on failure.
 */
static int
get_multiparts(
	int base_index,
	MultiPartInfo **malloc_and_setme_info)
{
	MultiPartInfo tmp, tmp2;
	MultiPartInfo *info = 0;
	int i;
	int part_index;

	/* entry assertions */
	assert(0 <= base_index && base_index < grpmenu.max && "Invalid base index");
	assert(malloc_and_setme_info != NULL && "malloc_and_setme_info must not be NULL");

	/* make sure this is a multipart message... */
	if (!get_multipart_info(base_index, &tmp) || tmp.total < 1)
		return 0;

	/* make a temporary buffer to hold the multipart info... */
	info = my_malloc(sizeof(MultiPartInfo) * tmp.total);

	/* zero out part-number for the repost check below */
	for (i = 0; i < tmp.total; ++i)
		info[i].part_number = -1;

	/* try to find all the multiparts... */
	for (i = 0; i < grpmenu.max; ++i) {
		if (strncmp(arts[base[i]].subject, tmp.subject, tmp.subject_compare_len))
			continue;
		if (!get_multipart_info(i, &tmp2))
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
			assert(info[part_index].part_number == tmp2.part_number && "bookkeeping error");
			continue;
		}

		/* we have a match, hooray! */
		info[part_index] = tmp2;
	}

	/* see if we got them all. */
	for (i = 0; i < tmp.total; ++i) {
		if (info[i].part_number != i + 1) {
			free(info);
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
tag_multipart(
	int base_index)
{
	MultiPartInfo *info = 0;
	int i;
	const int qty = get_multiparts(base_index, &info);

	/* check for failure... */
	if (qty == 0) {
		info_message(_(txt_info_not_multipart_message));
		return 0;
	}
	if (qty < 0) {
		info_message(_(txt_info_missing_part), -qty);
		return 0;
	}

	/*
	 * if any are already tagged, untag 'em first
	 * so num_of_tagged_arts doesn't get corrupted
	 */
	for (i = 0; i < qty; ++i) {
		if (arts[base[info[i].base_index]].tagged != 0)
			untag_article(base[info[i].base_index]);
	}

	/*
	 * get_multiparts() sorts info by part number,
	 * so a simple for loop tags in the right order
	 */
	for (i = 0; i < qty; ++i)
		arts[base[info[i].base_index]].tagged = ++num_of_tagged_arts;

	free(info);

	return qty;
}


/*
 * Return the highest tag number of any article in thread
 * rooted at base[n]
 */
int
line_is_tagged(
	int n)
{
	int code = 0;

	if (curr_group->attribute->thread_arts) {
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
tag_article(
	int art)
{
	if (arts[art].tagged != 0) {
		untag_article(art);
		info_message(_(txt_prefix_untagged), txt_article_singular);
		return FALSE;
	} else {
		arts[art].tagged = ++num_of_tagged_arts;
		info_message(_(txt_prefix_tagged), txt_article_singular);
		return TRUE;
	}
}


/*
 * Remove the tag from an article
 * Work through all the threads and decrement the tag counter on all arts
 * greater than 'tag', fixup counters
 */
void
untag_article(
	long art)
{
	int i, j;

	for (i = 0; i < grpmenu.max; ++i) {
		for_each_art_in_thread(j, i) {
			if (arts[j].tagged > arts[art].tagged)
				--arts[j].tagged;
		}
	}
	arts[art].tagged = 0;
	--num_of_tagged_arts;
}


/*
 * Clear tag status of all articles. If articles were untagged, return TRUE
 */
t_bool
untag_all_articles(
	void)
{
	int i;
	t_bool untagged = FALSE;

	for_each_art(i) {
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
 * min/max/curr are the lowest/highest and current positions on the
 * menu from which this was called; used as defaults if needed
 * Return TRUE if a range was successfully read, parsed and set
 *
 * Allowed syntax is 0123456789-.$ (blanks are ignored):
 *   1-23    mark grp/art 1 thru 23
 *   1-.     mark grp/art 1 thru current
 *   1-$     mark grp/art 1 thru last
 *   .-$     mark grp/art current thru last
 */
t_bool
set_range(
	int level,
	int min,
	int max,
	int curr)
{
	char *range;
	char *prompt;
	int artnum;
	int i;
	int depth;
	int range_min;
	int range_max;

	switch (level) {
		case SELECT_LEVEL:
			range = tinrc.default_range_select;
			break;

		case GROUP_LEVEL:
			range = tinrc.default_range_group;
			break;

		case THREAD_LEVEL:
			range = tinrc.default_range_thread;
			break;

		default:
			return FALSE;
	}

#if 0
	error_message("Min=[%d] Max=[%d] Cur=[%d] DefRng=[%s]", min, max, curr, range);
#endif /* 0 */
	prompt = fmt_string(_(txt_enter_range), range);

	if (!(prompt_string_default(prompt, range, _(txt_range_invalid), HIST_OTHER))) {
		free(prompt);
		return FALSE;
	}
	free(prompt);

	/*
	 * Parse range string
	 */
	if (!parse_range(range, min, max, curr, &range_min, &range_max)) {
		info_message(_(txt_range_invalid));
		return FALSE;
	}

	switch (level) {
		case SELECT_LEVEL:
			for (i = 0; i < max; i++)			/* Clear existing range */
				active[my_group[i]].inrange = FALSE;

			for (i = range_min - 1; i < range_max; i++)
				active[my_group[i]].inrange = TRUE;
			break;

		case GROUP_LEVEL:
			for (i = 0; i < max; i++) {			/* Clear existing range */
				for_each_art_in_thread(artnum, i)
					arts[artnum].inrange = FALSE;
			}

			for (i = range_min - 1; i < range_max; i++) {
				for_each_art_in_thread(artnum, i)
					arts[artnum].inrange = TRUE;
			}
			break;

		case THREAD_LEVEL:
			/*
			 * Debatably should clear all of arts[] depending on how you
			 * interpret the (non)spec
			 */
			for (i = 0; i < grpmenu.max; i++) {			/* Clear existing range */
				for_each_art_in_thread(artnum, i)
					arts[artnum].inrange = FALSE;
			}

			depth = 1;
			for_each_art_in_thread(artnum, thread_basenote) {
				if (depth > range_max)
					break;
				if (depth >= range_min)
					arts[artnum].inrange = TRUE;
				depth++;
			}
			break;

		default:
			return FALSE;
			/* NOTREACHED */
			break;
	}
	return TRUE;
}


/*
 * Parse 'range', return the range start and end values in range_start and range_end
 * min/max/curr are used to select defaults when n explicit start/end are given
 */
static t_bool
parse_range(
	char *range,
	int min,
	int max,
	int curr,
	int *range_start,
	int *range_end)
{
	char *ptr = range;
	enum states { FINDMIN, FINDMAX, DONE };
	int state = FINDMIN;
	t_bool ret = FALSE;

	*range_start = -1;
	*range_end = -1;

	while (*ptr && state != DONE) {
		if (isdigit((int) *ptr)) {
			if (state == FINDMAX) {
				*range_end = atoi(ptr);
				state = DONE;
			} else
				*range_start = atoi(ptr);
			while (isdigit((int) *ptr))
				ptr++;
		} else {
			switch (*ptr) {
				case '-':
					state = FINDMAX;
					break;

				case '.':
					if (state == FINDMAX) {
						*range_end = curr;
						state = DONE;
					} else
						*range_start = curr;
					break;

				case '$':
					if (state == FINDMAX) {
						*range_end = max;
						state = DONE;
					}
					break;

				default:
					break;
			}
			ptr++;
		}
	}

	if (*range_start >= min && *range_end >= *range_start && *range_end <= max)
		ret = TRUE;

	return ret;
}


/*
 * SELECTED CODE
 */
void
do_auto_select_arts(
	void)
{
	int i;

	for_each_art(i) {
		if (arts[i].status == ART_UNREAD && !arts[i].selected) {
#	ifdef DEBUG_NEWSRC
			debug_print_comment("group.c: X command");
#	endif /* DEBUG_NEWSRC */
			art_mark(curr_group, &arts[i], ART_READ);
			arts[i].zombie = TRUE;
		}
	}
	if (curr_group->attribute->show_only_unread)
		find_base(curr_group);

	grpmenu.curr = 0;
	show_group_page();
}


/* selection already happened in filter_articles() */
void
undo_auto_select_arts(
	void)
{
	int i;

	for_each_art(i) {
		if (arts[i].status == ART_READ && arts[i].zombie) {
#	ifdef DEBUG_NEWSRC
			debug_print_comment("group.c: + command");
#	endif /* DEBUG_NEWSRC */
			art_mark(curr_group, &arts[i], ART_UNREAD);
			arts[i].zombie = FALSE;
		}
	}
	if (curr_group->attribute->show_only_unread)
		find_base(curr_group);

	grpmenu.curr = 0;	/* do we want this? */
	show_group_page();
}


void
undo_selections(
	void)
{
	int i;

	for_each_art(i) {
		arts[i].selected = FALSE;
		arts[i].zombie = FALSE;
	}
}


/*
 * Return TRUE if there are any selected arts
 */
t_bool
arts_selected(
	void)
{
	int i;

	for_each_art(i) {
		if (arts[i].selected)
			return TRUE;
	}

	return FALSE;
}


/*
 * Mark all tagged articles as read.
 * Return number of articles that have been marked.
 */
int
mark_tagged_read(
	struct t_group *group)
{
	int i, cnt = 0;

	for_each_art(i) {
		if (arts[i].tagged && (arts[i].status == ART_UNREAD || arts[i].status == ART_WILL_RETURN)) {
			art_mark(group, &arts[i], ART_READ);
			cnt++;
		}
	}
	untag_all_articles();

	return cnt;
}


/*
 * Check if there is at least one tagged _and_ unread article.
 */
t_bool
got_tagged_unread_arts(
	void)
{
	int i;

	if (num_of_tagged_arts > 0) {
		for_each_art(i) {
			if (arts[i].tagged && (arts[i].status == ART_UNREAD || arts[i].status == ART_WILL_RETURN))
				return TRUE;
		}
	}

	return FALSE;
}
