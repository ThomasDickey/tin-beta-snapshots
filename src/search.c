/*
 *  Project   : tin - a Usenet reader
 *  Module    : search.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2003-05-15
 *  Notes     :
 *
 * Copyright (c) 1991-2003 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */

/*
 * local prototypes
 */
static char *get_search_pattern(t_bool *forward, t_bool repeat, const char *fwd_msg, const char *bwd_msg, char *def, int which_hist);
static int author_search(int i, char *searchbuf);
static int body_search(int i, char *searchbuf);
static int subject_search(int i, char *searchbuf);
static int search_group(t_bool forward, int current_art, char *searchbuff, int (*search_func) (int i, char *searchbuff));


/*
 * The search function may place error text into mesg
 */
#define MATCH_MSG	(mesg[0] ? mesg : _(txt_no_match))

/*
 * Kludge to maintain some internal state for body search
 */
static int total_cnt = 0, curr_cnt = 0;
int srch_lineno = -1;

/*
 * Used by article and body search - this saves passing around large numbers
 * of parameters all the time
 */
static int srch_offsets[6];
static int srch_offsets_size = ARRAY_SIZE(srch_offsets);
static struct regex_cache srch_regex;


/*
 * Obtain the search pattern, save it in the default buffer.
 * Return NULL if no pattern could be found
 */
static char *
get_search_pattern(
	t_bool *forward,
	t_bool repeat,
	const char *fwd_msg,
	const char *bwd_msg,
	char *def,
	int which_hist)
{
	static char tmpbuf[LEN];	/* Hold the last pattern used */
	static char last_pattern[LEN];	/* last search pattern used; for repeated search */
	static t_bool last_forward;

	if (repeat) {
		*forward = last_forward;
		my_strncpy(def, last_pattern, LEN);
	}
	else {
		sprintf(tmpbuf, (*forward ? fwd_msg : bwd_msg), def);

		if (!prompt_string_default(tmpbuf, def, _(txt_no_search_string), which_hist))
			return NULL;

		last_forward = *forward;
		my_strncpy(last_pattern, def, LEN);

		/* HIST_BODY_SEARCH doesn't exist, hence i_key_search_last is set directly in search_body() */
		if (which_hist == HIST_AUTHOR_SEARCH)
			i_key_search_last = *forward ? iKeySearchAuthF : iKeySearchAuthB;
		else
			i_key_search_last = *forward ? iKeySearchSubjF : iKeySearchSubjB;
	}

	wait_message(0, _(txt_searching));

	stow_cursor();

	if (tinrc.wildcard) {			/* ie, not wildmat() */
		strcpy(def, quote_wild_whitespace(def));
		return def;
	}

	/*
	 * A gross hack to simulate substrings with wildmat()
	 */
/* TODO: somehow use REGEX_FMT here? */
	sprintf(tmpbuf, "*%s*", def);
	return tmpbuf;
}


/*
 * called by config.c
 */
int
search_config(
	t_bool forward,
	t_bool repeat,
	int current,
	int last)
{
	char *buf;
	int n;
	int incr;
	int result = current;

	if (!(buf = get_search_pattern(&forward, repeat, _(txt_search_forwards), _(txt_search_backwards), tinrc.default_search_config, HIST_CONFIG_SEARCH)))
		return result;

	incr = forward ? 1 : -1;

	current += incr;
	n = current;
	do {
		if (n < 0)
			n = last;
		else {
			if (n > last)
				n = 0;
		}
		if (REGEX_MATCH(_(option_table[n].txt->opt), buf, TRUE)) {
			result = n;
			break;
		}
		n += incr;
	} while (n != current);
	clear_message();
	return result;
}


/*
 * Search active[] looking for a groupname
 * Called by select.c
 * Return index into active of matching groupname or -1
 */
int
search_active(
	t_bool forward,
	t_bool repeat)
{
	char *buf;
	char buf2[LEN];
	char *ptr = buf2;
	int i;

	if (!selmenu.max) {
		info_message(_(txt_no_groups));
		return -1;
	}

	if (!(buf = get_search_pattern(&forward, repeat, _(txt_search_forwards), _(txt_search_backwards), tinrc.default_search_group, HIST_GROUP_SEARCH)))
		return -1;

	i = selmenu.curr;

	do {
		if (forward) {
			if (++i >= selmenu.max)
				i = 0;
		} else {
			if (--i < 0)
				i = selmenu.max - 1;
		}

		/*
		 * Get the group name & description into buf2
		 */
		if (show_description && active[my_group[i]].description) {
			snprintf(buf2, sizeof(buf2), "%s %s", active[my_group[i]].name, active[my_group[i]].description);
			ptr = buf2;
		} else
			ptr = active[my_group[i]].name;

		if (REGEX_MATCH(ptr, buf, TRUE)) {
			return i;
		}
	} while (i != selmenu.curr);

	info_message(MATCH_MSG);
	return -1;
}


/*
 * Scan the body of an arts[i] for searchbuf
 * used only by search_body()
 * Returns:	1	String found
 *          0	Not found
 *	        -1	User aborted search
 */
static int
body_search(
	int i,
	char *searchbuf)
{
	char *line;
	t_openartinfo artinfo;

	switch (art_open(TRUE, &arts[i], curr_group, &artinfo, FALSE)) {
		case ART_ABORT:					/* User 'q'uit */
			art_close(&artinfo);
			return -1;

		case ART_UNAVAILABLE:			/* Treat as string not present */
			art_close(&artinfo);
			info_message(_(txt_no_match));
			return 0;
	}

	/*
	 * Skip the header - is this right ?
	 */
	for (i = 0; artinfo.cookl[i].flags & C_HEADER; ++i)
		;
	fseek(artinfo.cooked, artinfo.cookl[i].offset, SEEK_SET);

	/*
	 * Now search the body
	 */
	sprintf(mesg, _(txt_searching_body), ++curr_cnt, total_cnt);
	show_progress(mesg, curr_cnt, total_cnt);
	while ((line = tin_fgets(artinfo.cooked, FALSE)) != NULL) {
		if (tinrc.wildcard) {
			if (pcre_exec(srch_regex.re, srch_regex.extra, line, strlen(line), 0, 0, srch_offsets, srch_offsets_size) != PCRE_ERROR_NOMATCH) {
				srch_lineno = i;
				art_close(&pgart);		/* Switch the pager over to matched art */
				pgart = artinfo;
#ifdef DEBUG
				if (debug == 2)
					fprintf(stderr, "art_switch(%p = %p)\n", (void *) &pgart, (void *) &artinfo);
#endif /* DEBUG */

				return 1;
			}
		} else {
			if (wildmatpos(line, searchbuf, TRUE, srch_offsets, srch_offsets_size)) {
				srch_lineno = i;
				art_close(&pgart);		/* Switch the pager over to matched art */
				pgart = artinfo;
				return 1;
			}
		}
		i++;
	}

	if (tin_errno != 0) {			/* User abort */
		art_close(&artinfo);
		return -1;
	}

	art_close(&artinfo);
/*	info_message(MATCH_MSG); */
	return 0;
}


/*
 * Match searchbuff against the From: information in arts[i]
 * 1 = found, 0 = not found
 */
static int
author_search(
	int i,
	char *searchbuf)
{
	char buf[LEN];
	char *ptr = buf;

	if (arts[i].name == NULL)
		ptr = arts[i].from;
	else
		snprintf(buf, sizeof(buf), "%s <%s>", arts[i].name, arts[i].from);

	return (REGEX_MATCH(ptr, searchbuf, TRUE)) ? 1 : 0;
}


/*
 * Match searchbuff against the Subject: information in arts[i]
 * 1 = found, 0 = not found
 */
static int
subject_search(
	int i,
	char *searchbuf)
{
	return (REGEX_MATCH(arts[i].subject, searchbuf, TRUE)) ? 1 : 0;
}


/*
 * Returns index into arts[] of matching article or -1
 */
static int
search_group(
	t_bool forward,
	int current_art,
	char *searchbuff,
	int (*search_func) (int i, char *searchbuff))
{
	int i;

	if (grpmenu.curr < 0) {
		info_message(_(txt_no_arts));
		return -1;
	}

	i = current_art;

	do {
		if (forward) {
			if ((i = next_response(i)) < 0)
				i = base[0];
		} else {
			if ((i = prev_response(i)) < 0)
				i = find_response(grpmenu.max - 1, num_of_responses(grpmenu.max - 1));
		}

		/* Only search displayed articles */
		if (CURR_GROUP.attribute->show_only_unread && arts[i].status != ART_UNREAD)
			continue;

		switch (search_func(i, searchbuff)) {
			case 1:								/* Found */
				clear_message();
				return i;

			case -1:							/* User abort */
				return -1;
		}
	} while (i != current_art);

	info_message(_(txt_no_match));
	return -1;
}


/*
 * Generic entry point to search for fields in arts[]
 * Returns index into arts[] of matching article or -1
 */
int
search(
	int key,
	int current_art,
	t_bool forward,
	t_bool repeat)
{
	char *buf = NULL;
	int (*search_func) (int i, char *searchbuff) = author_search;

	switch (key) {
		case SEARCH_SUBJ:
			if (!(buf = get_search_pattern(&forward, repeat, _(txt_search_forwards), _(txt_search_backwards), tinrc.default_search_subject, HIST_SUBJECT_SEARCH)))
				return -1;
			search_func = subject_search;
			break;

		case SEARCH_AUTH:
		default:
			if (!(buf = get_search_pattern(&forward, repeat, _(txt_author_search_forwards), _(txt_author_search_backwards), tinrc.default_search_author, HIST_AUTHOR_SEARCH)))
				return -1;
			search_func = author_search;
			break;
	}
	return (search_group(forward, current_art, buf, search_func));
}


/*
 * page.c (search current article body)
 * Return line number that matches or -1
 * If using regex's return vector of character offsets
 */
int
search_article(
	t_bool forward,
	t_bool repeat,
	int start_line,
	int lines,
	t_lineinfo *line,
	int reveal_ctrl_l_lines,
	FILE *fp)
{
	char *pattern, *ptr;
	int i;
	struct regex_cache srch;

	if (!(pattern = get_search_pattern(&forward, repeat, _(txt_search_forwards), _(txt_search_backwards), tinrc.default_search_art, HIST_ART_SEARCH)))
		return FALSE;

	if (tinrc.wildcard && !(compile_regex(pattern, &srch, PCRE_CASELESS)))
		return -1;

	srch_lineno = -1;
	i = start_line;

	forever {
		if (forward) {
			if (++i == lines)
				break;
		} else {
			if (--i < 0)
				break;
		}

		/*
		 * TODO: consider not searching some line types?
		 * 'B'ody search skips hdrs, '/' inside article does not.
		 */
		fseek(fp, line[i].offset, SEEK_SET);

		/* Don't search beyond ^L if hiding is enabled */
		if ((line[i].flags&C_CTRLL) && i > reveal_ctrl_l_lines)
			break;

		ptr = tin_fgets(fp, FALSE);

		if (tinrc.wildcard) {
			if (pcre_exec(srch.re, srch.extra, ptr, strlen(ptr), 0, 0,
								srch_offsets, srch_offsets_size) != PCRE_ERROR_NOMATCH) {
				srch_lineno = i;
				return i;
			}
		} else {
			if (wildmatpos(ptr, pattern, TRUE, srch_offsets, srch_offsets_size)) {
				srch_lineno = i;
				return i;
			}
		}
	}

	info_message(_(txt_no_match));
	return -1;
}


/*
 * Search the bodies of all the articles in current group
 * Start the search at the current article
 * A match will replace the context of the article open in the pager
 * Save the line # that matched (and the start/end vector for regex)
 * for later retrieval
 * Return index in arts[] of article that matched or -1
 */
int
search_body(
	struct t_group *group,
	int current_art,
	t_bool repeat)
{
	char *buf;
	int i;
	t_bool forward_fake;

	if (!(buf = get_search_pattern(
			&forward_fake,				/* we pass a dummy var since body search has no `forward' */
			repeat,
			_(txt_search_body),
			_(txt_search_body),
			tinrc.default_search_art,
			HIST_ART_SEARCH
	))) return -1;

	i_key_search_last = iKeySearchBody;	/* store last search type for repeated search */
	total_cnt = curr_cnt = 0;			/* Reset global counter of articles done */

	/*
	 * Count up the articles to be processed for the progress meter
	 */
	if (group->attribute->show_only_unread) {
		for (i = 0; i < grpmenu.max; i++)
			total_cnt += new_responses(i);
	} else {
		for_each_art(i) {
			if (!IGNORE_ART(i))
				total_cnt++;
		}
	}

	/*
	 * Pre-compile if we're using full regex
	 */
	if (tinrc.wildcard && !(compile_regex(buf, &srch_regex, PCRE_CASELESS)))
		return -1;

	srch_lineno = -1;
	return search_group(1, current_art, buf, body_search);
}


/*
 * Return the saved line & start/end info from previous successful
 * regex search
 */
int
get_search_vectors(
	int *start,
	int *end)
{
	int i = srch_lineno;

	*start = srch_offsets[0];
	*end = srch_offsets[1];
	srch_lineno = -1;			/* We can only retrieve this info once */
	return i;
}
