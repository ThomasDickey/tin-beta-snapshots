/*
 *  Project   : tin - a Usenet reader
 *  Module    : search.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1997-12-27
 *  Notes     :
 *
 * Copyright (c) 1991-2000 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
 *    This product includes software developed by Iain Lea, Rich Skrenta.
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

static char tmpbuf[LEN];

/*
 * local prototypes
 */
static char *get_search_pattern (t_bool forward, const char *fwd_msg, const char *bwd_msg, char *def, int which_hist);

/*
 * The search function may place error text into mesg
 */
#define MATCH_MSG	(mesg[0] ? mesg : _(txt_no_match))

/*
 * Kludge to maintain counters for body search
 */
static int total_cnt = 0, curr_cnt = 0;


/*
 * Obtain the search pattern, save it in the default buffer.
 * Return NULL if no pattern could be found
 */
static char *
get_search_pattern (
	t_bool forward,
	const char *fwd_msg,
	const char *bwd_msg,
	char *def,
	int which_hist)
{
	sprintf (tmpbuf, (forward ? fwd_msg : bwd_msg), def);

	if (!prompt_string_default(tmpbuf, def, _(txt_no_search_string), which_hist))
		return (NULL);

	wait_message (0, _(txt_searching));
	stow_cursor();

	if (tinrc.wildcard) {			/* ie, not wildmat() */
		strcpy(def, quote_wild_whitespace (def));
		return(def);
	}

	/*
	 * A gross hack to simulate substrings with wildmat()
	 */
/* TODO somehow use REGEX_FMT here ? */
	sprintf(tmpbuf, "*%s*", def);
	return(tmpbuf);
}


/*
 * Called by config.c
 */
int
search_config (
	t_bool forward,
	int current,
	int last)
{
	int n;
	int incr = forward ? 1 : -1;
	int result = current;
	char *buf;

	if (!(buf = get_search_pattern(
				forward,
				_(txt_search_forwards),
				_(txt_search_backwards),
				tinrc.default_search_config,
				HIST_CONFIG_SEARCH
	))) return result;

	current += incr;
	n = current;
	do {
		if (n < 0)
			n = last;
		else if (n > last)
			n = 0;
		if (REGEX_MATCH (option_table[n].txt->opt, buf, TRUE)) {
			result = n;
			break;
		}
		n += incr;
	} while (n != current);
	clear_message ();
	return result;
}


/*
 * Search active[] looking for a groupname
 * Called by select.c
 * Return index into active of matching groupname or -1
 */
int
search_active (
	t_bool forward)
{
	char *buf;
	char buf2[LEN];
	char *ptr = buf2;
	int i;

	if (!selmenu.max) {
		info_message (_(txt_no_groups));
		return -1;
	}

	if (!(buf = get_search_pattern(forward, _(txt_search_forwards), _(txt_search_backwards), tinrc.default_search_group, HIST_GROUP_SEARCH)))
		return -1;

	i = selmenu.curr;

	do {
		if (forward) {
			i++;
			if (i >= selmenu.max)
				i = 0;
		} else {
			i--;
			if (i < 0)
				i = selmenu.max - 1;
		}

		/*
		 * Get the group name & description into buf2
		 */
		if (show_description && active[my_group[i]].description) {
			sprintf (buf2, "%s %s", active[my_group[i]].name, active[my_group[i]].description);
			ptr = buf2;
		} else
			ptr = active[my_group[i]].name;

		if (REGEX_MATCH (ptr, buf, TRUE)) {
			return i;
		}
	} while (i != selmenu.curr);

	info_message (MATCH_MSG);
	return -1;
}


/*
 * Called by help.c
 */
int
search_help (
	t_bool forward,
	int current,
	int last)
{
	char *buf;
	int incr = forward ? 1 : -1;
	int result = current;
	int n;

	if (!(buf = get_search_pattern(
				forward,
				_(txt_search_forwards),
				_(txt_search_backwards),
				tinrc.default_search_group,
				HIST_HELP_SEARCH
	))) return result;

	current += incr;
	n = current;
	do {
		if (n < 0)
			n = last;
		else if (n > last)
			n = 0;
		if (REGEX_MATCH (info_help[n], buf, TRUE)) {
			result = n;
			break;
		}
		n += incr;
	} while (n != current);
	clear_message ();
	return result;
}


/*
 * Scan the body of an arts[i] for searchbuf
 * used only by search_body()
 * Returns:	1	String found
 *          0	Not found
 *			-1	User aborted search
 */
static int
body_search (
	int i,
	char *searchbuf)
{
	FILE *fp;
	char *line;
	char group_path[PATH_LEN];
	int  code;
	struct t_article *art = &arts[i];

	if (!read_news_via_nntp || CURR_GROUP.type != GROUP_TYPE_NEWS)
		make_group_path (CURR_GROUP.name, group_path);

	/*
	 * open_art_fp() will display 'mesg' if not null instead of the default progress counter
	 */
	sprintf(mesg, _(txt_searching_body), ++curr_cnt, total_cnt);

#if 1 /* see also screen.c show_progress ()*/
	if ((fp = open_art_fp (group_path, art->artnum, -art->lines, TRUE)) == (FILE *) 0)
#else
	if ((fp = open_art_fp (group_path, art->artnum, art->lines, TRUE)) == (FILE *) 0)
#endif /* 1 */
		return ((tin_errno != 0) ? -1 : 0);

	/*
	 * Skip the header
	 */
	while ((line = tin_fgets (fp, TRUE)) != (char *) 0) {
		if (*line == '\0')
			break;
	}

	if (tin_errno != 0) {			/* User aborted search */
		code = -1;
		goto exit_search;
	}

	/*
	 * Now search the body
	 */
	while ((line = tin_fgets (fp, FALSE)) != (char *) 0) {
		if (REGEX_MATCH (line, searchbuf, TRUE)) {
			code = 1;
			goto exit_search;
		}
	}

	if (tin_errno != 0) {			/* User abort */
		code = -1;
		goto exit_search;
	}

	code = 0;						/* Didn't find it */
exit_search:
	fclose (fp);					/* open_art_fp() returns a real fd */
	return code;
}


/*
 * Match searchbuff against the From: information in arts[i]
 * 1 = found, 0 = not found
 */
static int
author_search (
	int i,
	char *searchbuf)
{
	char buf[LEN];
	char *ptr = buf;

	if (arts[i].name == (char *) 0)
		ptr = arts[i].from;
	else
		sprintf (buf, "%s <%s>", arts[i].name, arts[i].from);

	return (REGEX_MATCH (ptr, searchbuf, TRUE)) ? 1 : 0;
}


/*
 * Match searchbuff against the Subject: information in arts[i]
 * 1 = found, 0 = not found
 */
static int
subject_search (
	int i,
	char *searchbuf)
{
	return (REGEX_MATCH (arts[i].subject, searchbuf, TRUE)) ? 1 : 0;
}


/*
 * Returns index into arts[] of matching article or -1
 */
static int
search_group (
	t_bool forward,
	int current_art,
	char *searchbuff,
	int (*search_func) (int i, char *searchbuff))
{
	char group_path[PATH_LEN];
	int i;

	if (grpmenu.curr < 0) {
		info_message (_(txt_no_arts));
		return 0;
	}

	if (!read_news_via_nntp || CURR_GROUP.type != GROUP_TYPE_NEWS)
		make_group_path (CURR_GROUP.name, group_path);

	i = current_art;

	do {
		if (forward) {
			if ((i = next_response (i)) < 0)
				i = base[0];
		} else {
			if ((i = prev_response (i)) < 0)
				i = find_response(grpmenu.max - 1, num_of_responses (grpmenu.max - 1));
		}

		/* Only search displayed articles */
		if (CURR_GROUP.attribute->show_only_unread && arts[i].status != ART_UNREAD)
			continue;

		switch (search_func (i, searchbuff)) {
			case 1:								/* Found */
				clear_message ();
				return i;
			case -1:							/* User abort */
				return -1;
		}

	} while (i != current_art);

	info_message (MATCH_MSG);
	return -1;
}


/*
 * Generic entry point to search for fields in arts[]
 * Returns index into arts[] of matching article or -1
 */
int
search (
	int key,
	int current_art,
	t_bool forward)
{
	char *buf = NULL;
	int (*search_func) (int i, char *searchbuff) = author_search;

	switch (key) {
		case SEARCH_SUBJ:
			if (!(buf = get_search_pattern(
					forward,
					_(txt_search_forwards),
					_(txt_search_backwards),
					tinrc.default_search_subject,
					HIST_SUBJECT_SEARCH
			))) return -1;

			search_func = subject_search;
			break;

		case SEARCH_AUTH:
		default:
			if (!(buf = get_search_pattern(
					forward,
					_(txt_author_search_forwards),
					_(txt_author_search_backwards),
					tinrc.default_search_author,
					HIST_AUTHOR_SEARCH
			))) return -1;

			search_func = author_search;
			break;
	}

	return (search_group (forward, current_art, buf, search_func));
}


/*
 * page.c (search current article body)
 * TODO - highlight located text ?
 */
t_bool
search_article (
	t_bool forward)
{
	char *pattern;
	char *p, *q;
	char buf[LEN];
	char buf2[LEN];
	int i, j;
	int local_note_page = note_page; /* copy current position in article */
	t_bool ctrl_L;
	t_bool local_note_end = note_end;

	if (!(pattern = get_search_pattern(
				forward,
				_(txt_search_forwards),
				_(txt_search_backwards),
				tinrc.default_search_art,
				HIST_ART_SEARCH
	))) return FALSE;

	while (!local_note_end) {
		ctrl_L = FALSE;
		note_line = (local_note_page < 1) ? 5 : 3;

		while (note_line < cLINES) {
			if (fgets (buf, (int) sizeof(buf), note_fp) == NULL) {
				local_note_end = TRUE;
				break;
			}
			buf[LEN-1] = '\0';
			/*
			 * Build the string to search in buf2. Preprocess in the following ways:
			 * ignore backspaces
			 * expand pagefeed into a literal '^L'
			 * expand tabs
			 * maps control chars to '^char'
			 */
			for (p = buf, q = buf2;	*p && *p != '\n' && q<&buf2[LEN]; p++) {
				if (*p == '\b' && q > buf2)
					q--;
				else if (*p == '\f') {		/* ^L */
					*q++ = '^';
					*q++ = 'L';
					ctrl_L = TRUE;
				} else if (*p == '\t') {
					i = q - buf2;
					j = (i|7) + 1;
					while (i++ < j)
						*q++ = ' ';

				} else if (((*p) & 0xFF) < ' ') {
					*q++ = '^';
					*q++ = ((*p) & 0xFF) + '@';
				} else
					*q++ = *p;
			}
			*q = '\0';

			if (REGEX_MATCH (buf2, pattern, TRUE)) {
				fseek (note_fp, note_mark[local_note_page], SEEK_SET);
				return TRUE;
			}

			note_line += ((int) strlen(buf2) / cCOLS) + 1;

			if (ctrl_L)
				break;
		}
		if (!local_note_end)
			note_mark[++local_note_page] = ftell (note_fp);
	}

	fseek (note_fp, note_mark[note_page], SEEK_SET);
	info_message (MATCH_MSG);
	return FALSE;
}


/*
 * Search the bodies of all the articles in current group
 */
int
search_body (
	int current_art)
{
	char *buf;
	int i;

	if (!(buf = get_search_pattern(TRUE, _(txt_search_body), _(txt_search_body), tinrc.default_search_art, HIST_ART_SEARCH)))
		return -1;

	total_cnt = curr_cnt = 0;			/* Reset global counter of articles done */

	/*
	 * Count up the articles to be processed for the progress meter
	 */
	if (CURR_GROUP.attribute->show_only_unread) {
		for (i = 0; i < grpmenu.max; i++)
			total_cnt += new_responses (i);
	} else {
		for (i = 0; i < top_art; i++) {
			if (!IGNORE_ART(i))
				total_cnt++;
		}
	}
	return (search_group (1, current_art, buf, body_search));
}
