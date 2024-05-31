/*
 *  Project   : tin - a Usenet reader
 *  Module    : tags.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1999-12-06
 *  Updated   : 2024-01-10
 *  Notes     : Split out from other modules
 *
 * Copyright (c) 1999-2024 Jason Faultless <jason@altarstone.com>
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

/* Local prototypes */
static t_bool parse_range(char *range, int min, int max, int curr, int *range_start, int *range_end);

int num_of_tagged_arts = 0;

/*
 * Tags all parts of a multipart index if base_index points
 * to a multipart message and all its parts can be found.
 *
 * @param base_index points to one message in a multipart message.
 * @return number of messages tagged, or zero on failure
 */
int
tag_multipart(
	int arts_index)
{
	MultiPartInfo *info = NULL;
	int i;
	int qty;
	t_bool untagging = FALSE;

	for_each_art(i) {
		if (!global_look_for_multipart(i, '[', ']'))
			global_look_for_multipart(i, '(', ')');
	}

	qty = global_get_multiparts(arts_index, &info, TRUE);

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
	 * if any are already tagged, untag 'em
	 */
	for (i = 0; i < qty; ++i) {
		if (arts[info[i].arts_index].tagged != 0) {
			untagging = TRUE;
			while (i < qty)
				untag_article(info[i++].arts_index);
		}
	}

	/*
	 * get_multiparts() sorts info by part number,
	 * so a simple for loop tags in the right order
	 *
	 * only tag if we are not untagging
	 */
	if (!untagging) {
		for (i = 0; i < qty; ++i)
			arts[info[i].arts_index].tagged = ++num_of_tagged_arts;
	}

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
	int i, code = 0;

	if (curr_group->attribute->thread_articles) {
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
 *   1-23    mark grp/art 1 through 23
 *   1-.     mark grp/art 1 through current
 *   1-$     mark grp/art 1 through last
 *   .-$     mark grp/art current through last
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
	int range_min;
	int range_max;
	t_bool only_clear = FALSE;

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

		default:	/* should no happen */
			return FALSE;
	}

#if 0
	error_message(2, "Min=[%d] Max=[%d] Cur=[%d] DefRng=[%s]", min, max, curr, range);
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
		if (range_min == 0 && range_max == -1)
			only_clear = TRUE;
		else {
			info_message(_(txt_range_invalid));
			return FALSE;
		}
	}

	switch (level) {
		case SELECT_LEVEL:
			for (i = 0; i < max; i++)			/* Clear existing range */
				active[my_group[i]].inrange = FALSE;

			if (!only_clear) {
				for (i = range_min - 1; i < range_max; i++)
					active[my_group[i]].inrange = TRUE;
			}
			break;

		case GROUP_LEVEL:
			for (i = 0; i < max; i++) {			/* Clear existing range */
				for_each_art_in_thread(artnum, i)
					arts[artnum].inrange = FALSE;
			}

			if (!only_clear) {
				for (i = range_min - 1; i < range_max; i++) {
					for_each_art_in_thread(artnum, i)
						arts[artnum].inrange = TRUE;
				}
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

			if (!only_clear) {
				i = 1;
				for_each_art_in_thread(artnum, thread_basenote) {
					if (i > range_max)
						break;
					if (i >= range_min)
						arts[artnum].inrange = TRUE;
					i++;
				}
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
		if (isdigit((unsigned char) *ptr)) {
			if (state == FINDMAX) {
				*range_end = atoi(ptr);
				state = DONE;
			} else
				*range_start = atoi(ptr);
			while (isdigit((unsigned char) *ptr))
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
#ifdef DEBUG
			if (debug & DEBUG_NEWSRC)
				debug_print_comment("group.c: X command");
#endif /* DEBUG */
			art_mark(curr_group, &arts[i], ART_READ);
			arts[i].zombie = TRUE;
		}
		if (curr_group->attribute->show_only_unread_arts)
			arts[i].keep_in_base = FALSE;
	}
	if (curr_group->attribute->show_only_unread_arts)
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
#ifdef DEBUG
			if (debug & DEBUG_NEWSRC)
				debug_print_comment("group.c: + command");
#endif /* DEBUG */
			art_mark(curr_group, &arts[i], ART_UNREAD);
			arts[i].zombie = FALSE;
		}
	}
	if (curr_group->attribute->show_only_unread_arts)
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
