/*
 *  Project   : tin - a Usenet reader
 *  Module    : art.c
 *  Author    : I.Lea & R.Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2024-11-25
 *  Notes     :
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef NEWSRC_H
#	include "newsrc.h"
#endif /* !NEWSRC_H */
#ifndef STPWATCH_H
#	include "stpwatch.h"
#endif /* !STPWATCH_H */


/*
 * TODO: fixup to remove CURR_GROUP dependency in all sort funcs
 */
#define SortBy(func)	tin_sort(arts, (size_t) top_art, sizeof(struct t_article), func);

int top_art = 0;				/* # of articles in arts[] */

/*
 * Local prototypes
 */
static FILE *open_art_header(const char *groupname, t_artnum art, t_artnum *next);
static FILE *open_xover_fp(struct t_group *group, const char *mode, t_artnum min, t_artnum max, t_bool local);
static char *find_nov_file(struct t_group *group, int mode);
static char *print_from(struct t_group *group, struct t_article *article, int charset);
static int artnum_comp(t_comptype p1, t_comptype p2);
static int base_comp(t_comptype p1, t_comptype p2);
static int date_comp_asc(t_comptype p1, t_comptype p2);
static int date_comp_desc(t_comptype p1, t_comptype p2);
static int from_comp_asc(t_comptype p1, t_comptype p2);
static int from_comp_desc(t_comptype p1, t_comptype p2);
static int global_look_for_multipart_info(int aindex, MultiPartInfo *setme, char start, char stop, int *offset);
static int last_date_comp_base_asc(t_comptype p1, t_comptype p2);
static int last_date_comp_base_desc(t_comptype p1, t_comptype p2);
static int lines_comp_asc(t_comptype p1, t_comptype p2);
static int lines_comp_desc(t_comptype p1, t_comptype p2);
static int read_art_headers(struct t_group *group, int total, t_artnum top);
static int read_overview(struct t_group *group, t_artnum min, t_artnum max, t_artnum *top, t_bool local, t_bool *rebuild_cache);
static int score_comp_asc(t_comptype p1, t_comptype p2);
static int score_comp_desc(t_comptype p1, t_comptype p2);
static int score_comp_base(t_comptype p1, t_comptype p2);
static int subj_comp_asc(t_comptype p1, t_comptype p2);
static int subj_comp_desc(t_comptype p1, t_comptype p2);
static int valid_artnum(t_artnum art);
static t_artnum find_first_unread(struct t_group *group);
static t_artnum setup_hard_base(struct t_group *group);
static t_bool parse_headers(FILE *fp, struct t_article *h);
static t_compfunc eval_sort_arts_func(unsigned int sort_art_type);
static time_t get_last_posting_date(t_artnum n);
static void build_mailbox_list(struct t_article *art, const char *hdr);
static void sort_base(unsigned int sort_threads_type);
static void thread_by_multipart(void);
static void thread_by_percentage(unsigned int percentage);
static void thread_by_subject(void);
static void write_overview(struct t_group *group);
#ifdef NNTP_ABLE
	static struct t_article_range *build_range_list(t_artnum min, t_artnum max, int *range_cnt);
	static t_bool get_path_header(int cur, int cnt, struct t_group *group, t_artnum min, t_artnum max);
#endif /* NNTP_ABLE */
static struct t_mailbox *add_mailbox(struct t_article *art);


/*
 * Display a suitable 'entering group' message if screen needs redrawing
 * Allow for the non-printing %s, and the %-age counter
 */
void
show_art_msg(
	const char *group)
{
	wait_message(0, _(txt_group), cCOLS - (strwidth(_(txt_group)) > cCOLS ? cCOLS : strwidth(_(txt_group)) + 2 - 3), group);
}


/*
 * Construct the pointers to the first (base) article in each thread.
 * If we are showing only unread, then point to the first unread. I have
 * no idea why this should be so, it causes problems elsewhere [which_response]
 */
void
find_base(
	struct t_group *group)
{
	int i, j;

	grpmenu.max = 0;

#ifdef DEBUG
	if (debug & DEBUG_FILTER)
		debug_print_arts();
#endif /* DEBUG */

	for_each_art(i) {
		/*
		 * .prev will be set on each article that is after the first article in
		 * the thread. Invalid articles which have been expired will have
		 * .thread set to ART_EXPIRED
		 */
		if (arts[i].prev >= 0 || arts[i].thread == ART_EXPIRED || (arts[i].killed && tinrc.kill_level == KILL_NOTHREAD))
			continue;

		if (grpmenu.max >= max_base)
			expand_base();

		if (group->attribute && group->attribute->show_only_unread_arts) {
			if (arts[i].status != ART_READ || arts[i].keep_in_base)
				base[grpmenu.max++] = i;
			else {
				/* Find 1st unread art in thread */
				for (j = i; j >= 0; j = arts[j].thread) {
					if (arts[j].status != ART_READ || arts[j].keep_in_base) {
						base[grpmenu.max++] = i;
						break;
					}
				}
			}
		} else
			base[grpmenu.max++] = i;
	}
	/* sort base[] */
	if (group->attribute && group->attribute->sort_threads_type > SORT_THREADS_BY_NOTHING)
		sort_base(group->attribute->sort_threads_type);
}


/*
 * Longword comparison routine for the tin_sort()
 */
static int
base_comp(
	t_comptype p1,
	t_comptype p2)
{
	const t_artnum *a = (const t_artnum *) p1;
	const t_artnum *b = (const t_artnum *) p2;

	if (*a < *b)
		return -1;

	if (*a > *b)
		return 1;

	return 0;
}


/*
 * via NNTP:
 *   Issue a LISTGROUP command
 *   Read the article numbers existing in the group into base[]
 *   If the LISTGROUP failed, issue a GROUP command. Use the results to
 *   create a less accurate version of base[]
 *   This data will already be sorted
 *
 * on local spool:
 *   Read the spool dir to populate base[] as above. Sort it.
 *
 * Grow the arts[] and bitmaps as needed.
 * NB: the output will be sorted on artnum
 *
 * grpmenu.max is one past top.
 * Returns total number of articles in group, or -1 on error
 */
static t_artnum
setup_hard_base(
	struct t_group *group)
{
	t_artnum total = T_ARTNUM_CONST(0);

	grpmenu.max = 0;

	/*
	 * If reading with NNTP, issue a LISTGROUP
	 */
	if (read_news_via_nntp && !read_saved_news && group->type == GROUP_TYPE_NEWS) {
#ifdef NNTP_ABLE
		char buf[NNTP_STRLEN];
		char line[NNTP_STRLEN];
		int getart_limit = (cmdline.args & CMDLINE_GETART_LIMIT) ? cmdline.getart_limit : tinrc.getart_limit;
		FILE *fp;
		t_artnum last, start, count = T_ARTNUM_CONST(0), j = T_ARTNUM_CONST(0);
		static t_bool skip_listgroup = FALSE;

		/*
		 * Some nntp servers are broken and need an extra GROUP command
		 * (reported by reorx@irc.pl). This affects (old?) versions of
		 * nntpcache, leafnode and SurgeNews. Usually this should not be
		 * needed.
		 *
		 * For getart_limit recheck lowwatermark as at least giganews gives
		 * very different results for LIST ACTIVE (3 year retention for all)
		 * and GROUP (based on the clients contract).
		 * Calculate range and prepare base[] not to lose unread arts.
		 */
		if (nntp_caps.broken_listgroup || (!skip_listgroup && getart_limit && nntp_caps.type == CAPABILITIES && nntp_caps.reader)) {
			snprintf(buf, sizeof(buf), "GROUP %s", group->name);
			if (nntp_command(buf, OK_GROUP, line, sizeof(line)) == NULL)
				return -1;

			if (sscanf(line, "%"T_ARTNUM_SFMT" %"T_ARTNUM_SFMT, &count, &start) != 2)
				return -1;

			if (getart_limit > 0) {
				j = group->xmax - getart_limit;
				count = MAX(find_first_unread(group), start);
			}
			if (getart_limit < 0) {
				j = getart_limit + find_first_unread(group);
				count = group->xmin;
			}
			if (j < group->xmin)
				j = group->xmin;

			for (; count < j; count++) {
				if (grpmenu.max >= max_base)
					expand_base();
				base[grpmenu.max++] = count;
			}
		}

		/*
		 * See if LISTGROUP works
		 */
		if (!skip_listgroup && getart_limit != 0) { /* try to avoid traffic */
			if (nntp_caps.type == CAPABILITIES && nntp_caps.reader) {
				/* RFC 3977 allows ranges in LISTGROUP */
				if (getart_limit > 0)
					snprintf(buf, sizeof(buf), "LISTGROUP %s %"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT"", group->name, j, group->xmax);
				else /* getart_limit < 0; fetch till newest art */
					snprintf(buf, sizeof(buf), "LISTGROUP %s %"T_ARTNUM_PFMT"-", group->name, j);

			} else /* for RFC 977 just use GROUP */
				skip_listgroup = TRUE;

		} else /* get all article numbers */
			snprintf(buf, sizeof(buf), "LISTGROUP %s", group->name);

		if (!skip_listgroup) {
			if ((fp = nntp_command(buf, OK_GROUP, NULL, 0)) != NULL) {
				char *ptr;

#	ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose > 1)
					debug_print_file("NNTP", "setup_hard_base(%s)", buf);
#	endif /* DEBUG */

				while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
#	ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose)
					debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
					if (grpmenu.max >= max_base)
						expand_base();
					base[grpmenu.max++] = atoartnum(ptr);
					++total;
				}
#	ifdef DEBUG
				/* log end of multiline response to get timing data */
				if ((debug & DEBUG_NNTP) && !verbose)
					debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG */

				if (tin_errno)
					return -1;
			} else
				skip_listgroup = TRUE;
		}

		if (skip_listgroup) { /* LISTGROUP was skipped or failed */
			/*
			 * Handle the obscure case that the user aborted before the LISTGROUP
			 * had a chance to respond
			 */
			if (tin_errno)
				return -1;

			snprintf(buf, sizeof(buf), "GROUP %s", group->name);
			if (nntp_command(buf, OK_GROUP, line, sizeof(line)) == NULL)
				return -1;

			if (sscanf(line, "%"T_ARTNUM_SFMT" %"T_ARTNUM_SFMT" %"T_ARTNUM_SFMT, &count, &start, &last) != 3)
				return -1;

#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "setup_hard_base(%s)", buf);
#	endif /* DEBUG */
			total = count;
			grpmenu.max = 0;
			if (getart_limit > 0) {
				if ((j = find_first_unread(group)) > start) {
					if (group->xmax > getart_limit) {
						start = MIN(j, group->xmax - getart_limit);
						total = getart_limit;
					} else
						start = j;
				}
			}
			if (getart_limit < 0) {
				if ((j = (getart_limit + find_first_unread(group))) > start)
					start = j;
			}
			while (start <= last) {
				if (grpmenu.max >= max_base)
					expand_base();
				base[grpmenu.max++] = start++;
			}
		}
#endif /* NNTP_ABLE */
	/*
	 * Reading off local spool, read the directory files
	 */
	} else {
		DIR *d;
		DIR_BUF *e;
		char group_path[PATH_LEN];
		t_artnum art;

		make_base_group_path(group->spooldir, group->name, group_path, sizeof(group_path));

		if ((d = opendir(group_path)) != NULL) {
			while ((e = readdir(d)) != NULL) {
				art = atoartnum(e->d_name);
				if (art >= 1) {
					++total;
					if (grpmenu.max >= max_base)
						expand_base();
					base[grpmenu.max++] = art;
				}
			}
			CLOSEDIR(d);
			tin_sort((char *) base, (size_t) grpmenu.max, sizeof(t_artnum), base_comp);
		} else {
			perror_message(_(txt_cannot_open), group_path);
#if 0
			if (access(group_path, R_OK) != 0)
				error_message(2, _(txt_not_exist));
#endif /* 0 */
			return -1;
		}
	}

	if (grpmenu.max) {
		if (base[grpmenu.max - 1] > group->xmax)
			group->xmax = base[grpmenu.max - 1];
		expand_bitmap(group, base[0]);
	}

	return total;
}


/*
 * Main group indexing routine.
 *
 * Will read any existing index, create or incrementally update
 * the index by looking at the articles in the spool directory,
 * and attempt to write a new index if necessary.
 *
 * Returns FALSE if the user aborted the indexing, otherwise TRUE
 */
t_bool
index_group(
	struct t_group *group)
{
	int i;
	int changed;				/* Count of articles whose overview has changed */
	int getart_limit;
	int respnum;
	int total;
	t_artnum last_read_article;
	t_artnum min, new_min, max;
	t_bool caching_xover;
	t_bool filtered;
	t_bool path_in_nov = FALSE;
	t_bool rebuild_cache = FALSE;

	if (group == NULL)
		return TRUE;

	if (!batch_mode)
		show_art_msg(group->name);
	else {
		if (verbose > 1)
			wait_message(0, _(txt_reading_group), group->name);
	}

	signal_context = cArt;			/* Set this only once curr_group is valid */

	hash_reclaim();
	free_art_array();
	free_msgids();

	BegStopWatch();
	/*
	 * Get list of valid article numbers
	 */
	if (setup_hard_base(group) < 0)
		return FALSE;

	EndStopWatch("setup_hard_base()");

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC) {
		debug_print_comment("Before read_overview");
		debug_print_bitmap(group, NULL);
	}
#endif /* DEBUG */

	min = grpmenu.max ? base[0] : group->xmin;
	max = grpmenu.max ? base[grpmenu.max - 1] : min - 1;

	getart_limit = (cmdline.args & CMDLINE_GETART_LIMIT) ? cmdline.getart_limit : tinrc.getart_limit;

	if (getart_limit > 0) {
		if (grpmenu.max && (grpmenu.max > getart_limit))
			min = base[grpmenu.max - getart_limit];
		else
			getart_limit = 0;
	} else if (getart_limit < 0) {
		t_artnum first_unread = find_first_unread(group);

		if (min - first_unread < getart_limit)
			min = first_unread + getart_limit;
		else
			getart_limit = 0;
	}

	/*
	 * Quit now if no articles
	 */
	if (max < 0)
		return FALSE;

	top_art = 0;
	last_read_article = T_ARTNUM_CONST(0);

	/*
	 * Read in the existing overview data for min..max
	 * This read has local=TRUE set if locally caching XOVER records to ensure
	 * we pull in any private overview caches in preference to using OVER
	 *
	 * When reading local spool, this will pull in the system wide overview
	 * cache (if found) otherwise the private overview cache will be read
	 */
	caching_xover = (tinrc.cache_overview_files && nntp_caps.over_cmd && group->type == GROUP_TYPE_NEWS);
	if ((changed = read_overview(group, min, max, &last_read_article, caching_xover, &rebuild_cache)) == -1)
		return FALSE;	/* user aborted indexing */

	/*
	 * Fill in the range last_read_article...max using XOVER
	 * Only do this if the previous read_overview() was against private cache
	 */
	if ((last_read_article < max) && caching_xover) {
		new_min = (last_read_article >= min) ? last_read_article + 1 : min;

		if ((i = read_overview(group, new_min, max, &last_read_article, FALSE, &rebuild_cache)) == -1)
			return FALSE;	/* user aborted indexing */
		else
			changed += i;
	} else
		caching_xover = FALSE;

	/*
	 * Mark as UNTHREADED all articles that have been verified as valid
	 * Get num of new arts to index so the user will have an idea of index time
	 */
	for (i = 0, total = 0; i < grpmenu.max; i++) {
		if ((respnum = valid_artnum(base[i])) >= 0) {
			arts[respnum].thread = ART_UNTHREADED;
			continue;
		}
		if (base[i] <= last_read_article)		/* It is vital this test be done last */
			continue;
		++total;
	}

	/*
	 * Add any articles to arts[] that are new or were killed
	 */
	if (total > 0) {
		new_min = (getart_limit != 0 && last_read_article < min) ? min - 1 : last_read_article;

		if ((i = read_art_headers(group, total, new_min)) == -1)
			return FALSE;		/* user aborted indexing */
		else
			changed += i;
	}

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC) {
		debug_print_comment("Before parse_unread_arts()");
		debug_print_bitmap(group, NULL);
	}
#endif /* DEBUG */
	/*
	 * Do this before calling art_mark(,, ART_READ) if you want
	 * the unread count to be correct.
	 */
	min = getart_limit > 0 ? min : T_ARTNUM_CONST(0);
	parse_unread_arts(group, min);
#ifdef DEBUG
	if (debug & DEBUG_NEWSRC) {
		debug_print_comment("After parse_unread_arts()");
		debug_print_bitmap(group, NULL);
	}
#endif /* DEBUG */

	/*
	 * Stat all articles to see if any have expired
	 */
	for_each_art(i) {
		if (arts[i].thread == ART_EXPIRED) {
			++changed;
#ifdef DEBUG
			if (debug & DEBUG_NEWSRC)
				debug_print_comment("art.c: index_group() purging...");
#endif /* DEBUG */
			art_mark(group, &arts[i], ART_READ);
			if (group->attribute && group->attribute->show_only_unread_arts)
				arts[i].keep_in_base = FALSE;
		}
		if (!path_in_nov && arts[i].path && *arts[i].path != '\0')
			path_in_nov = TRUE;
	}

	/*
	 * Only rewrite the index if it has changed
	 * TODO review the exact logic behind "|| caching_xover"
	 */
	if (changed || caching_xover || rebuild_cache)
		write_overview(group);

	/*
	 * Create the reference tree. The msgid and ref ptrs will
	 * be free()d now that the NovFile has been written.
	 */
	BegStopWatch();
	build_references(group);
	EndStopWatch("build_references()");

	/*
	 * Needs access to the reference tree
	 */
	BegStopWatch();
	filtered = filter_articles(group);
	EndStopWatch("filter_articles()");

	/*
	 * Thread the group
	 */
	BegStopWatch();
	make_threads(group, FALSE);
	EndStopWatch("make_threads()");

	if ((changed > 0 || filtered) && !batch_mode)
		clear_message();

	return TRUE;
}


/*
 * Returns number of first unread article
 */
static t_artnum
find_first_unread(
	struct t_group *group)
{
	unsigned char *p;
	unsigned char *end = group->newsrc.xbitmap;
	t_artnum first = group->newsrc.xmin; /* initial value */

	if ((p = group->newsrc.xbitmap)) {
		end += group->newsrc.xbitlen / NBITS;
		for (; p < end && *p == '\0'; p++, first += NBITS)
			;
	}
	return first;
}


/*
 * Open an article for reading just the header
 * 'NEXT' is used/updated with the next article number
 * to optimise the number of 'HEAD' commands issued on
 * groups with holes.
 */
static FILE *
open_art_header(
	const char *groupname,
	t_artnum art,
	t_artnum *next)
{
	char buf[NNTP_STRLEN];
#ifdef NNTP_ABLE
	FILE *fp;
	int i;

	if (read_news_via_nntp && CURR_GROUP.type == GROUP_TYPE_NEWS) {
		static t_bool no_next = FALSE; /* TODO: move to t_capabilities ? */
		/*
		 * Don't bother requesting if we have not got there yet.
		 * This is a big win if the group has got holes in it (ie. if 000's
		 * of articles have expired between active files min & max values).
		 */
		if (art < *next)
			return NULL;

		snprintf(buf, sizeof(buf), "HEAD %"T_ARTNUM_PFMT, art);
		if ((fp = nntp_command(buf, OK_HEAD, NULL, 0)) != NULL)
			return fp;

		/*
		 * TODO:
		 * shall we stop on 5xx?, i.e JamNNTPd/2 1.3 responds with
		 * "503 Access denied" instead of 480 but NEXT still works,
		 * so tin loops over all articles without getting useful data
		 *
		 * usenet.farm may return ERR_GOODBYE and NEXT then also fails
		 * we could flag that like no_next ...
		 */

		if (!no_next) { /* usenet.farm doesn't do NEXT */
			/*
			 * HEAD failed, try to find NEXT
			 * Should return "223 artno message-id more text...."
			 */
			i = new_nntp_command("NEXT", OK_NOTEXT, buf, sizeof(buf));
			switch (i) {
				case OK_NOTEXT:
					*next = atoartnum(buf);		/* Set next art number */
					break;

#	ifndef BROKEN_LISTGROUP
				/*
				 * might happen if LISTGROUP doesn't select group, but
				 * we are not -DBROKEN_LISTGROUP
				 */
				case ERR_NCING:
					nntp_caps.broken_listgroup = TRUE;
					snprintf(buf, sizeof(buf), "GROUP %s", groupname);
					if (nntp_command(buf, OK_GROUP, NULL, 0) == NULL)
						return NULL;
					snprintf(buf, sizeof(buf), "HEAD %"T_ARTNUM_PFMT, art);
					if ((fp = nntp_command(buf, OK_HEAD, NULL, 0)) != NULL)
						return fp;
					if (nntp_command("NEXT", OK_NOTEXT, buf, sizeof(buf)))
						*next = atoartnum(buf);
					break;
#	endif /* !BROKEN_LISTGROUP */

				case ERR_COMMAND:	/* TODO: abort loop over all arts */
					no_next = TRUE;
#	ifdef DEBUG
					if ((debug & DEBUG_NNTP) && verbose > 1)
						debug_print_file("NNTP", "!!! NEXT disabled after %d response", ERR_COMMAND);
#	endif /* DEBUG */
					break;

				default:
					/*
					 * TODO: abort loop over all arts on ERR_NONEXT
					 */
#	ifndef BROKEN_LISTGROUP
					/*
					 * to avoid out of sync responses
					 * (listgroup seems to work, but didn't select new group,
					 *  so xover seems to work but returns old data)
					 * we set listgroup_broken = TRUE; once we saw a
					 * ERR_NOARTIG / ERR_NONEXT or the like - even if
					 * ERR_NOARTIG may occur on servers where listgroup
					 * isn't broken...
					 */
					nntp_caps.broken_listgroup = TRUE;
#	endif /* !BROKEN_LISTGROUP */
					break;
			}
		}
		return NULL;
	}
	/* silence compiler warning (unused parameter) */
#	ifdef BROKEN_LISTGROUP
	(void) groupname;
#	endif /* BROKEN_LISTGROUP */
#else
	(void) groupname;
	(void) next;
#endif /* NNTP_ABLE */

	snprintf(buf, sizeof(buf), "%"T_ARTNUM_PFMT, art);
	return (fopen(buf, "r"));
}


/*
 * Called after XOVER/local/private overview databases have been loaded
 * Read and parse in headers for any arts not already found (usually
 * new articles that have not been indexed yet)
 * Any new articles that are added have ->thread set to ART_UNTHREADED
 * 'top' is the current highest artnum read
 *
 * Return values are:
 *   >0   Number of additional articles read in
 *    0   No additional (new) articles were found
 *   -1   user aborted during read
 */
static int
read_art_headers(
	struct t_group *group,
	int total,
	t_artnum top)
{
	FILE *fp;
	char dir[PATH_LEN];
	char group_msg[LEN];
	int i;
	int modified = 0;
	t_artnum art;
	t_artnum head_next = T_ARTNUM_CONST(-1); /* Reset the next article number index (for when HEAD fails) */
	t_bool res;

	/*
	 * Change to groups spooldir to optimize fopen()'s on local articles
	 * NB open_art_header() requires this
	 */
	if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS) {
		char buf[PATH_LEN];

		get_cwd(dir);
		make_base_group_path(group->spooldir, group->name, buf, sizeof(buf));
		if (chdir(buf) != 0) {
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				fprintf(stderr, "read_art_headers(chdir(%s))", strerror(errno));
#endif /* DEBUG */
			return -1;
		}
	}

	snprintf(group_msg, sizeof(group_msg), _(txt_group), cCOLS - MIN(cCOLS - 1, strwidth(_(txt_group))) + 2 - 3, group->name);

	/* TODO: add progress meter? HEAD/NEXT is slow */
	for (i = 0; i < grpmenu.max; i++) {	/* for each article number */
		art = base[i];

		/*
		 * Skip articles that are below the low water mark or are
		 * already present
		 */
		if (valid_artnum(art) >= 0)
			continue;
		if (art <= top)
			continue;

		/*
		 * Try and open the article
		 */
		if ((fp = open_art_header(group->name, art, &head_next)) == NULL)
			continue;

		/*
		 * Add article to arts[]
		 */
		if (top_art >= max_art)
			expand_art();

		set_article(&arts[top_art]);
		arts[top_art].artnum = art;
		arts[top_art].thread = ART_UNTHREADED;

		res = parse_headers(fp, &arts[top_art]);

		TIN_FCLOSE(fp);
		if (tin_errno) {
			modified = -1;
			break;
		}

		if (!res) {
#ifdef DEBUG
			if (debug & DEBUG_FILTER) { /* we currently have no "local spool" debug level */
				char buf[PATH_LEN];

				snprintf(buf, sizeof(buf), "FAILED parse_headers(%"T_ARTNUM_PFMT")", art);
				debug_print_file("ARTS", "read_art_headers() %s", buf);
			}
#endif /* DEBUG */
			arts[top_art].artnum = T_ARTNUM_CONST(0);
			arts[top_art].date = (time_t) 0;
			FreeAndNull(arts[top_art].xref);
			FreeAndNull(arts[top_art].path);
			FreeAndNull(arts[top_art].refs);
			FreeAndNull(arts[top_art].msgid);
			free_mailbox_list(arts[top_art].mailbox.next);
			arts[top_art].tagged = 0;
			arts[top_art].thread = ART_EXPIRED;
			arts[top_art].prev = ART_NORMAL;
			arts[top_art].status = ART_UNREAD;
			arts[top_art].killed = ART_NOTKILLED;
			arts[top_art].selected = FALSE;
			continue;
		}

		top = arts[top_art].artnum;	/* used if arts are killed */
		++top_art;

		if (++modified % (MODULO_COUNT_NUM * 20) == 0)
			show_progress(group_msg, modified, total);
	}

	/*
	 * Change back to previous dir before indexing started
	 */
	if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS) {
		if (chdir(dir) == -1) {
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				perror_message("chdir(%s)", dir);
#endif /* DEBUG */
		}
	}

	return modified;
}


/*
 * The algorithm is elegant, using the fact that identical Subject lines
 * are hashed to the same node in table[] (see hashstr.c)
 *
 * Mark i as being in j's thread list if
 * . The article is _not_ being ignored
 * . The article is not already threaded
 * . The subject lines are the same
 */
static void
thread_by_subject(
	void)
{
	int i, j;
	struct t_hashnode *h;

	for_each_art(i) {
		if (IGNORE_ART_THREAD(i))
			continue;

		/*
		 * Get the contents of the magic marker in the hashnode
		 */
		h = (void *) (arts[i].subject - sizeof(int) - sizeof(void *)); /* FIXME: cast increases required alignment of target type */

		j = h->aptr;

		if (j != -1 && j < i) {
			if (arts[i].prev == ART_NORMAL && (arts[i].subject == arts[j].subject)) {
				arts[j].thread = i;
				arts[i].prev = j;
			}
		}

		/*
		 * Update the magic marker with the highest numbered mesg in
		 * arts[] that has been used in this thread so far
		 */
		h->aptr = i;
	}

#if 0
	fprintf(stderr, "Subj dump\n");
	fprintf(stderr, "%3s %3s %3s %3s : %3s %3s\n", "#", "Par", "Sib", "Chd", "In", "Thd");
	for_each_art(i) {
		fprintf(stderr, "%3d %3d %3d %3d : %3d %3d : %.50s %s\n", i,
			(arts[i].refptr->parent) ? arts[i].refptr->parent->article : -2,
			(arts[i].refptr->sibling) ? arts[i].refptr->sibling->article : -2,
			(arts[i].refptr->child) ? arts[i].refptr->child->article : -2,
			arts[i].prev, arts[i].thread, arts[i].refptr->txt, arts[i].subject);
	}
#endif /* 0 */
}


/*
 * This Threading algorithm threads articles into 'buckets' where each bucket
 * contains all the articles which match the root article's subject line to
 * the configured percentage. Eg, if the root article had the subject "asdf"
 * and the match percentage was configured to be 75% then any article would
 * match if its subject was no different in more than a single character.
 */
static void
thread_by_percentage(
	unsigned int percentage)
{
	int i, j, k;
	int root_num = 0; /* The index number of the root we are currently working on. */
	unsigned int unmatched; /* This is the number of characters that don't match between the two strings */
	size_t slen;

	/* First we need to sort art[] to simplify and speed up the matching. */
	SortBy(subj_comp_asc);

	/*
	 * Now we put all the articles which match enough into the thread. If
	 * an article doesn't match enough we create a new thread and then add
	 * to that and so on.
	 */
	base[0] = 0;
	arts[0].prev = ART_NORMAL;
	for_each_art(i) {
		if (i == 0)
			continue;

		/* Check each character to see if it matched enough */
		k = 0;
		unmatched = 0;
		for (j = 0; arts[base[root_num]].subject[j] != '\0' && arts[i].subject[k] != '\0'; j++, k++) {
			if (arts[base[root_num]].subject[j] != arts[i].subject[k])
				++unmatched;
		}

		/*
		 * By getting here we have a number of unmatched characters
		 * between the two strings. We also have the length of the
		 * strings available to us easily.
		 * All we need to do is see if the match is good enough, but
		 * we count differences in the length of the strings against
		 * them matching.
		 */
		if (!(slen = strlen(arts[base[root_num]].subject)))
			++slen;
		unmatched += (unsigned) (slen - strlen(arts[i].subject));
		if (unmatched * 100 / slen > percentage) {
			/*
			 * If there is less greater than percentage% different start a
			 * new thread.
			 */
			base[++root_num] = i;
			arts[i].prev = ART_NORMAL;
			continue;
		} else {
			/*
			 * The subject lines match enough to consider them part of a single
			 * thread, so add the current article to the thread.
			 */
			if (arts[base[root_num]].thread < 0)
				arts[base[root_num]].thread = i;
			arts[i].prev = i - 1;
			arts[i - 1].thread = i;
			continue;
		}
	}
}


/*
 * Parses a subject header of the type "multipart message subject (01/42)"
 * into a MultiPartInfo struct, or fails if the message subject isn't in the
 * right form.
 *
 * @return nonzero on success
 */
int
global_get_multipart_info(
	int aindex,
	MultiPartInfo *setme)
{
	int i, j, offi, offj;
	MultiPartInfo setmei, setmej;

	i = global_look_for_multipart_info(aindex, &setmei, '[', ']', &offi);
	j = global_look_for_multipart_info(aindex, &setmej, '(', ')', &offj);

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
global_look_for_multipart_info(
	int aindex,
	MultiPartInfo* setme,
	char start,
	char stop,
	int *offset)
{
	char *subj;
	char *pch;
	MultiPartInfo tmp;

	*offset = 0;

	/* entry assertions */
	assert(((void) "invalid index", 0 <= aindex && aindex < top_art));
	assert(((void) "setme must not be NULL", setme != NULL));

	/* parse the message */
	subj = arts[aindex].subject;
	pch = strrchr(subj, start);
	if (!pch || !isdigit((unsigned char) pch[1]))
		return 0;

	tmp.arts_index = aindex;
	tmp.subject_compare_len = (int) (pch - subj);
	tmp.part_number = (int) strtol(pch + 1, &pch, 10);
	if (*pch != '/' && *pch != '|')
		return 0;

	if (!isdigit((unsigned char) pch[1]))
		return 0;

	tmp.total = (int) strtol(pch + 1, &pch, 10);
	if (*pch != stop)
		return 0;

	/*
	 * skip "blah (00/102)" or "blah (103/102)" subjects
	 */
	if (tmp.part_number < 1 || tmp.part_number > tmp.total)
		return 0;

	tmp.subject = subj;
	*setme = tmp;
	*offset = (int) (pch - subj);
	return 1;
}


t_bool
global_look_for_multipart(
	int aindex,
	char start,
	char stop)
{
	char *pch = strrchr(arts[aindex].subject, start);

	if (!pch || !isdigit((unsigned char) pch[1]))
		return FALSE;

	strtol(pch + 1, &pch, 10);
	if (*pch != '/' && *pch != '|')
		return FALSE;

	if (!isdigit((unsigned char) pch[1]))
		return FALSE;

	strtol(pch + 1, &pch, 10);
	if (*pch != stop)
		return FALSE;

	arts[aindex].multipart_subj = TRUE;
	return TRUE;
}


/*
 * Tries to find all the parts to the multipart message pointed to by
 * aindex.
 *
 * @return on success, the number of parts found. On failure, zero if not
 * a multipart or the negative value of the first missing part in case of
 * tagging.
 * @param aindex index pointing to one of the messages in a multipart
 * message.
 * @param malloc_and_setme_info on success, set to a malloced array the
 * parts found.
 */
int
global_get_multiparts(
	int aindex,
	MultiPartInfo **malloc_and_setme_info,
	t_bool tagging)
{
	int i, part_index, part_cnt = 0;
	MultiPartInfo tmp, tmp2;
	MultiPartInfo *info;

	/* entry assertions */
	assert(((void) "Invalid index", 0 <= aindex && aindex < top_art));
	assert(((void) "malloc_and_setme_info must not be NULL", malloc_and_setme_info != NULL));

	/* make sure this is a multipart message... */
	if (!global_get_multipart_info(aindex, &tmp) || tmp.total < 1)
		return 0;

	/* make a temporary buffer to hold the multipart info... */
	info = my_malloc(sizeof(MultiPartInfo) * (size_t) tmp.total);

	/* zero out part-number for the repost check below */
	for (i = 0; i < tmp.total; ++i) {
		info[i].total = tmp.total; /* Added this for thread_by_multipart */
		info[i].part_number = -1;
	}

	/* try to find all the multiparts... */
	for (i = (tagging ? 0 : aindex); i < top_art; i++) {
		if (!arts[i].multipart_subj || strncmp(arts[i].subject, tmp.subject, (size_t) tmp.subject_compare_len))
			continue;

		if (!global_get_multipart_info(i, &tmp2))
			continue;

		/* 'test (1/5)' is not the same as 'test (1/15)' */
		if (tmp.total != tmp2.total)
			continue;

		part_index = tmp2.part_number - 1;

		/* repost check: do we already have this part? */
		if (info[part_index].part_number != -1) {
			assert(((void) "bookkeeping error", info[part_index].part_number == tmp2.part_number));
			continue;
		}

		/* we have a match, hooray! */
		info[part_index] = tmp2;

		arts[i].multipart_subj = FALSE;

		/* all parts found? */
		if (++part_cnt == tmp.total)
			break;
	}

	/* see if we got them all. */
	if (tagging) {
		for (i = 0; i < tmp.total; ++i) {
			if (info[i].part_number != i + 1) {
				free(info);
				return -(i + 1); /* missing part #(i+1) */
			}
		}
	}

	/* looks like a success .. */
	*malloc_and_setme_info = info;
	return tmp.total;
}


/*
 *	The algorithm uses the tag multipart searches to thread articles together.
 */
static void
thread_by_multipart(
	void)
{
	int i, j, threadNum;
	MultiPartInfo *minfo = NULL;

	for_each_art(i) {
		if (!global_look_for_multipart(i, '[', ']'))
			global_look_for_multipart(i, '(', ')');
	}

	for_each_art(i) {
		if (!arts[i].multipart_subj)
			continue;

		if (IGNORE_ART_THREAD(i) || arts[i].prev >= 0 || !global_get_multiparts(i, &minfo, FALSE)) {
			FreeAndNull(minfo);
			arts[i].multipart_subj = FALSE;
			continue;
		}

		threadNum = -1;
		for (j = minfo[0].total - 1; j >= 0; j--) {
			if (minfo[j].part_number != -1) {
				if (threadNum != -1) {
					arts[minfo[j].arts_index].thread = threadNum;
					arts[threadNum].prev = minfo[j].arts_index;
				}
				threadNum = minfo[j].arts_index;
			}
		}
		FreeAndNull(minfo);
		arts[i].multipart_subj = FALSE;
		if (i % MODULO_COUNT_NUM == 0)
			show_progress(_(txt_threading_by_multipart), i, top_art);
	}
}


/*
 * Go through the articles in arts[] and create threads. There are
 * 5 strategies currently defined :
 *
 *	THREAD_NONE		No threading
 *	THREAD_SUBJ		Threads are created using like Subject lines
 *	THREAD_REFS		Threads are created using the References headers
 *	THREAD_BOTH		Threads created using References and then Subject
 *	THREAD_MULTI	Threads created using Subject to search for Multiparts
 *	THREAD_PERC		Threads based upon a char for char match of greater than x%
 *
 * .thread and .prev are used to hold the threading information, see tin.h for
 * more information
 * Only process valid (unexpired) articles we haven't visited yet
 * (ie arts[].thread == ART_UNTHREADED)
 *
 * The rethread parameter is used to force the deletion of existing threading
 * information before threading which happens anyway expect when using
 * THREAD_NONE (I don't immediately see how this is useful)
 */
/* TODO: rewrite that user can easily combine different 'threading'
 *       methods, i.e:
 *       - thread_by_multipart() + collate_subjects()
 */
void
make_threads(
	struct t_group *group,
	t_bool rethread)
{
	if (!cmd_line && !batch_mode) {
		info_message((group->attribute->thread_articles == THREAD_NONE ? _(txt_unthreading_arts) : _(txt_threading_arts)));
		my_flush();
	}

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		error_message(2, "rethread=[%d]  thread_articles=[%d]  attr_thread_articles=[%d]",
			rethread, tinrc.thread_articles, group->attribute->thread_articles);
#endif /* DEBUG */

	/*
	 * Sort all the articles using the preferred method
	 * When find_base() is called, the bases are created ordered
	 * on arts[] and so the base messages under all threading systems
	 * will be sorted in this way.
	 */
	sort_arts(group->attribute ? group->attribute->sort_article_type : SORT_ARTICLES_BY_NOTHING);

	/*
	 * Reset all the ptrs to articles following the above sort
	 */
	clear_art_ptrs();

	/*
	 * The threading pointers need to be reset if re-threading
	 * If using ref threading, revector the links back to the articles
	 */
	if (rethread || (group->attribute && group->attribute->thread_articles)) {
		int i;

		for_each_art(i) {
			if (arts[i].thread >= 0)
				arts[i].thread = ART_UNTHREADED;

			arts[i].prev = ART_NORMAL;

			/* Should never happen if tree is built properly */
			if (arts[i].refptr == NULL) {
#ifdef DEBUG
				if (debug & DEBUG_REFS) {
					my_fprintf(stderr, "\nError  : art->refptr is NULL\n");
					my_fprintf(stderr, "Artnum : %"T_ARTNUM_PFMT"\n", arts[i].artnum);
					my_fprintf(stderr, "Subject: %s\n", arts[i].subject);
					my_fprintf(stderr, "From   : %s\n", arts[i].mailbox.from);
					assert(arts[i].refptr != NULL);
				} else
#endif /* DEBUG */
					continue;
			}
			arts[i].refptr->article = i;
		}
	}

	/*
	 * Do the right thing according to the threading strategy
	 */
	switch (group->attribute ? group->attribute->thread_articles : THREAD_NONE) {
		case THREAD_NONE:
			break;

		case THREAD_SUBJ:
			thread_by_subject();
			break;

		case THREAD_REFS:
			thread_by_reference();
			break;

		case THREAD_BOTH:
			thread_by_reference();
			collate_subjects();
			break;

		case THREAD_MULTI:
			thread_by_multipart();
			break;

		case THREAD_PERC:
			thread_by_percentage((unsigned) (100 - group->attribute->thread_perc));
			break;

		default: /* not reached */
			break;
	}

	/*
	 * Rebuild base[]
	 */
	find_base(group);
}


static t_compfunc
eval_sort_arts_func(
	unsigned int sort_art_type)
{
	switch (sort_art_type) {
		case SORT_ARTICLES_BY_NOTHING:		/* don't sort at all */
			return artnum_comp;

		case SORT_ARTICLES_BY_SUBJ_DESCEND:
			return subj_comp_desc;

		case SORT_ARTICLES_BY_SUBJ_ASCEND:
			return subj_comp_asc;

		case SORT_ARTICLES_BY_FROM_DESCEND:
			return from_comp_desc;

		case SORT_ARTICLES_BY_FROM_ASCEND:
			return from_comp_asc;

		case SORT_ARTICLES_BY_DATE_DESCEND:
			return date_comp_desc;

		case SORT_ARTICLES_BY_DATE_ASCEND:
			return date_comp_asc;

		case SORT_ARTICLES_BY_SCORE_DESCEND:
			return score_comp_desc;

		case SORT_ARTICLES_BY_SCORE_ASCEND:
			return score_comp_asc;

		case SORT_ARTICLES_BY_LINES_DESCEND:
			return lines_comp_desc;

		case SORT_ARTICLES_BY_LINES_ASCEND:
			return lines_comp_asc;

		default:
			break;
	}
	return NULL;
}


void
sort_arts(
	unsigned int sort_art_type)
{
	t_compfunc comp_func = eval_sort_arts_func(sort_art_type);

	if (comp_func)
		SortBy(comp_func);
}


static void
sort_base(
	unsigned int sort_threads_type)
{
	switch (sort_threads_type) {
		case SORT_THREADS_BY_SCORE_DESCEND:
		case SORT_THREADS_BY_SCORE_ASCEND:
			tin_sort(base, (size_t) grpmenu.max, sizeof(t_artnum), score_comp_base);
			break;

		case SORT_THREADS_BY_LAST_POSTING_DATE_DESCEND:
			tin_sort(base, (size_t) grpmenu.max, sizeof(t_artnum), last_date_comp_base_desc);
			break;

		case SORT_THREADS_BY_LAST_POSTING_DATE_ASCEND:
			tin_sort(base, (size_t) grpmenu.max, sizeof(t_artnum), last_date_comp_base_asc);
			break;

		/* should not happen */
		case SORT_THREADS_BY_NOTHING:
		default:
			/* CONSTANTCONDITION */
			assert(0 != 0);
			break;
	}
}


/*
 * This is called to get header info for articles not already found in the
 * overview files.
 */
static t_bool
parse_headers(
	FILE *fp,
	struct t_article *h)
{
	char *s, *hdr, *ptr;
	t_bool got_from = FALSE, got_lines = FALSE;

	while ((ptr = tin_fgets(fp, TRUE)) != NULL) {
		/*
		 * Look for the end of information which tin wants to get.
		 * Applies when reading local spool and via NNTP.
		 */

		/*
		 * End of headers ?
		 */
		if (!*ptr)
			break;

		unfold_header(ptr);
		switch (my_toupper((unsigned char) *ptr)) {
			case 'D':	/* Date:  mandatory */
				if (!h->date) {
					if ((hdr = parse_header(ptr + 1, "ate", FALSE, FALSE, FALSE))) {
						str_trim(hdr);
						if ((h->date = parsedate(hdr, (struct _TIMEINFO *) 0)) <= 0) {
							/* date parsing failed, cut off at last ' ' and try again */
							if ((s = strrchr(hdr, ' ')) != NULL) {
								*s = '\0';
								h->date = parsedate(hdr, (struct _TIMEINFO *) 0);
							}
						}
					}
				}
				break;

			case 'F':	/* From:  mandatory */
				if (!got_from) {
					if ((hdr = parse_header(ptr + 1, "rom", FALSE, FALSE, FALSE))) {
						build_mailbox_list(h, hdr);
						got_from = TRUE;
					}
				}
				break;

			case 'L':	/* Lines:  optional */
				if (!got_lines) {
					if ((hdr = parse_header(ptr + 1, "ines", FALSE, FALSE, FALSE))) {
						h->line_count = s2i(hdr, 0, INT_MAX);
						got_lines = TRUE;
					}
				}
				break;

			case 'M':	/* Message-ID:  mandatory; be aware that build_references() later on clears it! */
				if (!h->msgid) {
					if ((hdr = parse_header(ptr + 1, "essage-ID", FALSE, FALSE, FALSE)))
						h->msgid = my_strdup(hdr);
				}
				break;

			/* for Path:-filter when reading from local spool */
			case 'P':	/* Path: */
				if (!h->path) {
					if ((hdr = parse_header(ptr + 1, "ath", FALSE, FALSE, FALSE)))
						h->path = my_strdup(hdr);
				}
				break;

			case 'R':	/* References:  optional */
				if (!h->refs) {
					if ((hdr = parse_header(ptr + 1, "eferences", FALSE, FALSE, FALSE)))
						h->refs = my_strdup(hdr);
				}
				break;

			case 'S':	/* Subject:  mandatory */
				if (!h->subject) {
					if ((hdr = parse_header(ptr + 1, "ubject", FALSE, FALSE, FALSE))) {
#ifdef HAVE_UNICODE_NORMALIZATION
						if (IS_LOCAL_CHARSET("UTF-8"))
							s = normalize(eat_re(convert_to_printable(rfc1522_decode(hdr), FALSE), FALSE));
						else
#endif /* HAVE_UNICODE_NORMALIZATION */
							s = my_strdup(eat_re(convert_to_printable(rfc1522_decode(hdr), FALSE), FALSE));

						h->subject = hash_str(s);
						free(s);
					}
				}
				break;

			case 'X':	/* Xref:  optional */
				if (!h->xref) {
					if ((hdr = parse_header(ptr + 1, "ref", FALSE, FALSE, FALSE)))
						h->xref = my_strdup(hdr);
				}
				break;

			default:
				break;
		} /* switch */
	} /* while */

#ifdef NNTP_ABLE
	if (ptr)
		drain_buffer(fp);
#endif /* NNTP_ABLE */

	if (tin_errno)
		return FALSE;

	/*
	 * The son of RFC 1036 states that the following hdrs are mandatory. It
	 * also states that Subject, Newsgroups and Path are too. Ho hum.
	 *
	 * What about reading mail from local spool via ~/.tin/active.mail,
	 * they might not have a Message-ID.
	 */
	if (got_from && h->date && h->msgid) {
		if (!h->subject)
			h->subject = hash_str("");

#ifdef DEBUG
		if (debug & DEBUG_FILTER)
			debug_print_header(h);
#endif /* DEBUG */
		return TRUE;
	}

	return FALSE;
}


#ifdef NNTP_ABLE
/*
 * Loop over arts[] and find ranges without Path: header
 * If there are any try to optimize the ranges regarding traffic consumption
 * Start optimization if at least MIN_CNT ranges exist
 * If there are more than MAX_CNT ranges after optimization, fetch all in one
 * big range
 */
#define MIN_CNT 10
#define MAX_CNT 50
static struct t_article_range *
build_range_list(
	t_artnum min,
	t_artnum max,
	int *range_cnt)
{
	int i, gap_cnt = 0;
	struct t_article_range *res = NULL, *gap_list, *curr, *from;
	t_artnum new_end = T_ARTNUM_CONST(0);

	gap_list = my_malloc(sizeof(struct t_article_range));
	curr = gap_list;
	curr->start = min;
	curr->end = max;
	curr->cnt = T_ARTNUM_CONST(0);
	curr->next = NULL;

	for_each_art(i) {
		if (arts[i].artnum < min)
			continue;
		if (arts[i].artnum > max)
			break;
		if (arts[i].path) {
			for (; i < top_art && arts[i].path; i++)
				;
			/*
			 * the current art has no path -> we use this one
			 * if we reached top_art all arts have path
			 * so we use max
			 */
			curr->start = ((i == top_art) ? max : arts[i--].artnum);
		} else {
			for (; i < top_art && !arts[i].path; i++)
				;
			/* the current art has path -> we use the last one */
			new_end = curr->end = arts[--i].artnum;
		}
		if (new_end) {
			curr->cnt = curr->end - curr->start + 1;
			curr->next = my_malloc(sizeof(struct t_article_range));
			curr = curr->next;
			curr->start = new_end;
			curr->end = max;
			curr->cnt = T_ARTNUM_CONST(0);
			curr->next = NULL;
			new_end = T_ARTNUM_CONST(0);
		}
	}

	curr = gap_list;
	while (curr && curr->cnt) {
		++gap_cnt;
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "range #%d without path in overview cache: start: %"T_ARTNUM_PFMT" end: %"T_ARTNUM_PFMT" cnt: %"T_ARTNUM_PFMT"", gap_cnt, curr->start, curr->end, curr->cnt);
#	endif /* DEBUG */
		curr = curr->next;
	}

	/*
	 * Optimize only if there are at least MIN_CNT ranges
	 */
	if (gap_cnt >= MIN_CNT) {
		res = my_malloc(sizeof(struct t_article_range));
		res->start = T_ARTNUM_CONST(0);
		res->end = T_ARTNUM_CONST(0);
		res->cnt = T_ARTNUM_CONST(0);
		res->next = NULL;

		from = gap_list;
		curr = res;
		while (from) {
			curr->start = from->start;
			curr->end = from->end;
			curr->cnt = from->cnt;
			if ((from = from->next)) {
				/*
				 * If the next range is grater then the gap between the current
				 * one and the next one we build a new range including the
				 * current one, the next one and the gap between
				 */
				while (from && from->cnt >= from->start - curr->end - 1) {
					curr->end = from->end;
					from = from->next;
				}
				curr->cnt = curr->end - curr->start + 1;
				curr->next = my_malloc(sizeof(struct t_article_range));
				curr = curr->next;
				curr->start = T_ARTNUM_CONST(0);
				curr->end = T_ARTNUM_CONST(0);
				curr->cnt = T_ARTNUM_CONST(0);
				curr->next = NULL;
			}
		}
	}

	/*
	 * If there are less then MIN_CNT ranges
	 * no res is build -> return the original list
	 */
	if (res) {
		while (gap_list) {
			curr = gap_list;
			gap_list = curr->next;
			free(curr);
		}
	} else
		res = gap_list;

	curr = res;
	gap_cnt = 0;
	while (curr && curr->cnt) {
		++gap_cnt;
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "optimized range #%d: start: %"T_ARTNUM_PFMT" end: %"T_ARTNUM_PFMT" cnt: %"T_ARTNUM_PFMT"", gap_cnt, curr->start, curr->end, curr->cnt);
#	endif /* DEBUG */
		curr = curr->next;
	}

	if (gap_cnt >= MAX_CNT) {
		curr = res;
		while (curr->next && curr->next->cnt) {
			res->end = curr->next->end;
			curr->next->cnt = 0;
			curr = curr->next;
		}
		res->cnt = res->end - res->start + 1;
		gap_cnt = 1;
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "more then %d ranges after optimization, fetch all at once instead: start: %"T_ARTNUM_PFMT" end: %"T_ARTNUM_PFMT" cnt: %"T_ARTNUM_PFMT"", MAX_CNT, res->start, res->end, res->cnt);
#	endif /* DEBUG */
	}
	*range_cnt = gap_cnt;

	return res;
}


/*
 * Fetch the Path header in case we want to filter on that in the given group
 *
 * Try [X]HDR first, then XPAT
 */
static t_bool
get_path_header(
	int cur,
	int cnt,
	struct t_group *group,
	t_artnum min,
	t_artnum max)
{
	FILE *fp = NULL;
	char *buf, *ptr;
	char cmd[NNTP_STRLEN];
	t_bool found = FALSE;
	static t_bool supported = TRUE; /* assume HDR || XPAT works */

	if (!read_news_via_nntp || !supported || group->type != GROUP_TYPE_NEWS)
		return FALSE;

#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "%s: Filtering on Path header requested.", group->name);
#	endif /* DEBUG */

	if (nntp_caps.type == CAPABILITIES && nntp_caps.list_headers && !*nntp_caps.headers_range && nntp_caps.hdr_cmd[0] != 'X') {
		int j = new_nntp_command("LIST HEADERS RANGE", 215, cmd, sizeof(cmd));
		switch (j) {
			case 215:
				while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#	ifdef DEBUG
					if (debug & DEBUG_NNTP)
						debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
					nntp_caps.headers_range = my_realloc(nntp_caps.headers_range, strlen(nntp_caps.headers_range) + strlen(ptr) + 2);
					strcat(nntp_caps.headers_range, ptr);
					strcat(nntp_caps.headers_range, "\n");
				}
				break;

			default:
				break;
		}
	}

	/* does HDR return Path? */
	if (nntp_caps.headers_range && (ptr = strtok(nntp_caps.headers_range, "\n")) != NULL) {
		do {
			if ((*ptr == ':' && *(ptr + 1) == '\0') || !strncasecmp(ptr, "Path", 4))
				found = TRUE;
		} while (!found && *ptr && (ptr = strtok(NULL, "\n")) != NULL);
	}

	if ((nntp_caps.hdr || nntp_caps.hdr_cmd) && (!(nntp_caps.type == CAPABILITIES) || found)) {
		if (min == max)
			snprintf(cmd, sizeof(cmd), "%s Path %"T_ARTNUM_PFMT, nntp_caps.hdr_cmd, min);
		else
			snprintf(cmd, sizeof(cmd), "%s Path %"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT, nntp_caps.hdr_cmd, min, MAX(min, max));
		fp = nntp_command(cmd, nntp_caps.hdr_cmd[0] == 'X' ? OK_XHDR : OK_HDR, NULL, 0);
		if (!nntp_caps.hdr && fp)
			nntp_caps.hdr = TRUE;
	} else if (nntp_caps.xpat) {
		if (min == max)
			snprintf(cmd, sizeof(cmd), "XPAT Path %"T_ARTNUM_PFMT" *", min);
		else
			snprintf(cmd, sizeof(cmd), "XPAT Path %"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT" *", min, MAX(min, max));
		fp = nntp_command(cmd, OK_XPAT, NULL, 0);
	}

	if (fp) {
		t_artnum artnum, i, j = T_ARTNUM_CONST(0);
		char *prep_msg = fmt_string(_(txt_prep_for_filter_on_path), cur, cnt);

		while ((buf = tin_fgets(fp, FALSE)) != NULL && buf[0] != '.') {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose)
				debug_print_file("NNTP", "<<<%s%s", logtime(), buf);
#	endif /* DEBUG */
			if ((ptr = tin_strtok(buf, " ")) == NULL)
				continue;
			artnum = atoartnum(ptr);
			if ((ptr = tin_strtok(NULL, " ")) == NULL)
				continue;
			for (i = j; i < top_art; i++) {
				if (arts[i].artnum == artnum) {
					FreeIfNeeded(arts[i].path);
					arts[i].path = my_strdup(ptr);
					j = i;
					break;
				}
			}
			if (++artnum % MODULO_COUNT_NUM == 0)
				show_progress(prep_msg, artnum - min, max - min);
		}
#	ifdef DEBUG
		/* log end of multiline response to get timing data */
		if ((debug & DEBUG_NNTP) && !verbose)
			debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG */
		free(prep_msg);
		return supported;
	}

	/* !fp */
	supported = FALSE;
	if (nntp_caps.xpat)
		nntp_caps.xpat = FALSE;
	/* as nntp_caps.hdr may work with other headers we don't disable it */

#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "%s: Neither \"[X]HDR Path\" nor \"XPAT Path\" are supported.", group->name);
#	endif /* DEBUG */
	return supported;
}
#endif /* NNTP_ABLE */


static struct t_mailbox *
add_mailbox(
	struct t_article *art)
{
	struct t_mailbox *mb;

	if (!art)
		return NULL;

	if (!art->mailbox.from)
		return &art->mailbox;

	if (!(mb = art->mailbox.next)) {
		art->mailbox.next = my_malloc(sizeof(struct t_mailbox));
		mb = art->mailbox.next;
	} else {
		while (mb->next)
			mb = mb->next;
		mb->next = my_malloc(sizeof(struct t_mailbox));
		mb = mb->next;
	}

	mb->from = NULL;
	mb->name = NULL;
	mb->next = NULL;

	return mb;
}


static void
build_mailbox_list(
	struct t_article *art,
	const char *hdr)
{
	char art_from_addr[HEADER_LEN];
	char art_full_name[HEADER_LEN];
	char *tmp_from, *curr_from, *next_from;
	struct t_mailbox *mb;

	tmp_from = my_strdup(hdr);
	curr_from = tmp_from;

	do {
		if ((mb = add_mailbox(art)) == NULL)
			break;

		while (*curr_from == ' ')
			++curr_from;

		next_from = split_mailbox_list(curr_from);
		mb->gnksa_code = parse_from(curr_from, art_from_addr, art_full_name);
		mb->from = hash_str(buffer_to_ascii(art_from_addr));
		if (*art_full_name)
			mb->name = hash_str(eat_tab(convert_to_printable(rfc1522_decode(art_full_name), FALSE)));
		curr_from = next_from;
	} while (curr_from);

	free(tmp_from);
}


void
free_mailbox_list(
	struct t_mailbox *mb)
{
	if (!mb)
		return;

	while (mb->next != NULL) {
		free_mailbox_list(mb->next);
		mb->next = NULL;
	}

	free(mb);
}


/*
 * Read in an overview index file. Fields are separated by TAB.
 * return the number of expired articles encountered or -1 if the user aborted
 * the read
 * 'top' is set to the highest artnum read
 * If 'local' is set then always open local overview cache in preference to
 * using NNTP XOVER
 *
 * Format (mandatory as far as line count [RFC 2980]):
 *	1. article number (ie. 183)                [mandatory]
 *	2. Subject: line  (ie. Which newsreader?)  [mandatory]
 *	3. From: line     (ie. iain@ecrc.de)       [mandatory]
 *	4. Date: line     (RFC 822 format)         [mandatory]
 *	5. MessageID:     (ie. <123@example.net>)  [mandatory]
 *	6. References:    (ie. <message-id> ....)  [optional]
 *	7. Byte count     (Skipped - not used)     [mandatory]
 *	8. Line count     (ie. 23)                 [mandatory]
 *	9. Xref: line     (ie. alt.test:389)       [optional]
 */
static int
read_overview(
	struct t_group *group,
	t_artnum min,
	t_artnum max,
	t_artnum *top,
	t_bool local,
	t_bool *rebuild_cache)
{
	FILE *fp;
	char *ptr;
	char *q;
	char *buf;
	char *group_msg;
	unsigned int count;
	int expired = 0;
	t_artnum artnum;
	t_bool path_found = FALSE, path_in_ofmt = FALSE;
	struct t_article *art;
	size_t over_fields = 1;

	/*
	 * open the overview file (whether it be local or via nntp)
	 */
	if ((fp = open_xover_fp(group, "r", min, max, local)) == NULL)
		return expired;

	BegStopWatch();

	if (group->xmax > max)
		group->xmax = max;

	group_msg = fmt_string(_(txt_group), cCOLS - MIN(cCOLS - 1, strwidth(_(txt_group))) + 2 - 3, group->name);

	/* get the number of fields per over-record as announced by LIST OVERVIEW.FMT */
	if (ofmt) {
		for (; ofmt[over_fields].name; over_fields++) {
			if (local && !path_in_ofmt && !strcasecmp(ofmt[over_fields].name, "Path:"))
				path_in_ofmt = TRUE;
		}
	}

	if (!--over_fields) { /* e.g. nntp_caps.type == CAPABILITIES && !nntp_caps.list_overview_fmt -> assume defaults */
		ofmt = my_realloc(ofmt, sizeof(struct t_overview_fmt) * (8 + 1));
		ofmt[0].type = OVER_T_INT;
		ofmt[0].name = my_strdup("Artnum:");
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
		over_fields = 7;
	}

	while ((buf = tin_fgets(fp, FALSE)) != NULL) {
#if defined(DEBUG) && defined(NNTP_ABLE)
		if ((debug & DEBUG_NNTP) && fp == FAKE_NNTP_FP && verbose)
			debug_print_file("NNTP", "<<<%s%s", logtime(), buf);
#endif /* DEBUG && NNTP_ABLE */

		if (need_resize) {
			handle_resize((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
		}

		/*
		 * Read artnum
		 */
		if ((ptr = tin_strtok(buf, "\t")) == NULL)
			continue;

		/*
		 * read the article number, guaranteed to be the first field
		 */
		artnum = atoartnum(ptr);

		/*
		 * artnum field invalid/corrupt or is 1st line of local cached overview
		 * (group name)
		 */
		if (artnum <= 0)
			continue;

		/*
		 * skip artnums below the given minimum (getart_limit)
		 */
		if (artnum < min)
			continue;

		/*
		 * Check to make sure article in nov file has not expired in group
		 */
		if (artnum < group->xmin) {
			++expired;
			continue;
		}

		/*
		 * artnum in overview data higher than groups high mark
		 *
		 * TODO: - warn user about broken overviews?
		 *       - try to parse the Xref:-line to get the correct artnum
		 *       - see also parse_unread_arts()
		 */
		if (artnum > group->xmax)
			continue;

		if (top_art >= max_art)
			expand_art();

		art = &arts[top_art];
		set_article(art);
		art->artnum = *top = artnum;

		/*
		 * Note: Fields after line count are not mandatory, use "LIST OVERVIEW.FMT"
		 *       to check for additions like we do with xref_supported
		 */
		for (count = 1; (ptr = tin_strtok(NULL, "\t")) != NULL; count++) {
			/* skip unexpected tailing fields */
			if (count > over_fields) {
#ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose > 1)
					debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") Unexpected overview-field %d of %d: %s", nntp_caps.over_cmd, artnum, count, over_fields, ptr);
#endif /* DEBUG */

				/* "common error" Xref:full in overview-data but not in OVERVIEW.FMT */
				if (count == over_fields + 1) {
					if (!strncasecmp(ptr, "Xref: ", 6)) {
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "%s: found unexpected Xref: on semi std. position", nntp_caps.over_cmd);
#endif /* DEBUG */
						++over_fields;
						ofmt = my_realloc(ofmt, sizeof(struct t_overview_fmt) * (over_fields + 2)); /* + 2 = artnum and end-marker */
						ofmt[over_fields].type = OVER_T_FSTRING;
						ofmt[over_fields].name = my_strdup("Xref:");
						ofmt[over_fields + 1].type = OVER_T_ERROR;
						ofmt[over_fields + 1].name = NULL;
						xref_supported = TRUE;
					} else if (local && !strncasecmp(ptr, "Path: ", 6)) {
#ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "%s: found Path:", nntp_caps.over_cmd);
#endif /* DEBUG */
						++over_fields;
						ofmt = my_realloc(ofmt, sizeof(struct t_overview_fmt) * (over_fields + 2)); /* + 2 = artnum and end-marker */
						ofmt[over_fields].type = OVER_T_FSTRING;
						ofmt[over_fields].name = my_strdup("Path:");
						ofmt[over_fields + 1].type = OVER_T_ERROR;
						ofmt[over_fields + 1].name = NULL;
						xref_supported = TRUE;
					} else
						continue;
				} else
					continue;
			}

			/* for duplicated headers this is last match counts, INN >= 2.5.3 does first match counts */
			if (expensive_over_parse) { /* strange order */
				/* mandatory fields */
				if (ofmt[count].type == OVER_T_STRING) {
					if (!strcasecmp(ofmt[count].name, "Subject:")) {
						if (*ptr) {
#ifdef HAVE_UNICODE_NORMALIZATION
							if (IS_LOCAL_CHARSET("UTF-8"))
								q = normalize(eat_re(eat_tab(convert_to_printable(rfc1522_decode(ptr), FALSE)), FALSE));
							else
#endif /* HAVE_UNICODE_NORMALIZATION */
								q = my_strdup(eat_re(eat_tab(convert_to_printable(rfc1522_decode(ptr), FALSE)), FALSE));

							art->subject = hash_str(q);
							free(q);
						} else {
							art->subject = hash_str("");
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") empty overview-field %s", nntp_caps.over_cmd, artnum, ofmt[count].name);
#endif /* DEBUG */
						}
						continue;
					}

					if (!strcasecmp(ofmt[count].name, "From:")) {
						if (*ptr)
							build_mailbox_list(art, ptr);
						else {
							struct t_mailbox *mb;

							if ((mb = add_mailbox(art)) != NULL)
								mb->from = hash_str("");
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") empty overview-field %s", nntp_caps.over_cmd, artnum, ofmt[count].name);
#endif /* DEBUG */
						}
						continue;
					}

					if (!strcasecmp(ofmt[count].name, "Date:")) {
						str_trim(ptr);
						if ((art->date = parsedate(ptr, (TIMEINFO *) 0)) <= 0) {
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") bogus overview-field %s %s", nntp_caps.over_cmd, artnum, ofmt[count].name, ptr);
#endif /* DEBUG */
							/* date parsing failed, cut off at last ' ' and try again */
							if ((q = strrchr(ptr, ' ')) != NULL) {
								*q = '\0';
								art->date = parsedate(ptr, (TIMEINFO *) 0);
							}
						}
						continue;
					}

					if (!strcasecmp(ofmt[count].name, "Message-ID:")) {
						if (*ptr) {
							FreeIfNeeded(art->msgid); /* if field is listed more than once in overview.fmt */
							art->msgid = my_strdup(ptr);
						} else {
							art->msgid = NULL;
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") empty overview-field %s", nntp_caps.over_cmd, artnum, ofmt[count].name);
#endif /* DEBUG */
						}
						continue;
					}

					if (!strcasecmp(ofmt[count].name, "References:")) {
						if (*ptr) {
							FreeIfNeeded(art->refs); /* if field is listed more than once in overview.fmt */
							art->refs = my_strdup(ptr);
						} else
							art->refs = NULL;
						continue;
					}

					/*
					 * non std. fields when doing
					 * expensive overview parsing (very
					 * rare, just happens if RFC 3977
					 * 8.4.2 is violated) go here
					 */
					/* for Path:-filter */
					if (!strcasecmp(ofmt[count].name, "Path:")) {
						if (!path_found)
							path_found = TRUE;
						if (*ptr) {
							FreeIfNeeded(art->path); /* if field is listed more than once in overview.fmt */
							art->path = my_strdup(ptr);
						} else
							art->path = NULL;
						continue;
					}
				}
				/* metadata fields */
				if (ofmt[count].type == OVER_T_INT) {
					if (!strcasecmp(ofmt[count].name, "Bytes:")) {
						if (*ptr) {
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1 && !isdigit((unsigned char) *ptr))
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") overview field %d (%s) mismatch: %s", nntp_caps.over_cmd, artnum, count, ofmt[count].name, ptr);
#endif /* DEBUG */
						}
						continue;
					}

					if (!strcasecmp(ofmt[count].name, "Lines:")) {
						if (*ptr) {
							if (isdigit((unsigned char) *ptr))
								art->line_count = s2i(ptr, 0, INT_MAX);
							else {
								art->line_count = 0;
#ifdef DEBUG
								if ((debug & DEBUG_NNTP) && verbose > 1)
									debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") overview field %d (%s) mismatch: %s", nntp_caps.over_cmd, artnum, count, ofmt[count].name, ptr);
#endif /* DEBUG */
							}
						} else
							art->line_count = 0;
						continue;
					}
				}
			} else { /* first 7 fields are in RFC 3977 order */
				switch (count) {
					case 1: /* Subject: */
						if (*ptr) {
#ifdef HAVE_UNICODE_NORMALIZATION
							if (IS_LOCAL_CHARSET("UTF-8"))
								q = normalize(eat_re(eat_tab(convert_to_printable(rfc1522_decode(ptr), FALSE)), FALSE));
							else
#endif /* HAVE_UNICODE_NORMALIZATION */
								q = my_strdup(eat_re(eat_tab(convert_to_printable(rfc1522_decode(ptr), FALSE)), FALSE));

							art->subject = hash_str(q);
							free(q);
						} else {
							art->subject = hash_str("");
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") empty overview-field %s", nntp_caps.over_cmd, artnum, ofmt[count].name);
#endif /* DEBUG */
						}
						break;

					case 2:	/* From: */
						if (*ptr)
							build_mailbox_list(art, ptr);
						else {
							struct t_mailbox *mb;

							if ((mb = add_mailbox(art)) != NULL)
								mb->from = hash_str("");
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") empty overview-field %s", nntp_caps.over_cmd, artnum, ofmt[count].name);
#endif /* DEBUG */
						}
						break;

					case 3:	/* Date: */
						str_trim(ptr);
						if ((art->date = parsedate(ptr, (TIMEINFO *) 0)) <= 0) {
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") bogus overview-field %s %s", nntp_caps.over_cmd, artnum, ofmt[count].name, ptr);
#endif /* DEBUG */
							/* date parsing failed, cut off at last ' ' and try again */
							if ((q = strrchr(ptr, ' ')) != NULL) {
								*q = '\0';
								art->date = parsedate(ptr, (TIMEINFO *) 0);
							}
						}
						break;

					case 4:	/* Message-ID: */
						if (*ptr)
							art->msgid = my_strdup(ptr);
						else {
							art->msgid = NULL;
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") empty overview-field %s", nntp_caps.over_cmd, artnum, ofmt[count].name);
#endif /* DEBUG */
						}
						break;

					case 5:	/* References: */
						if (*ptr)
							art->refs = my_strdup(ptr);
						else
							art->refs = NULL;
						break;

					case 6:	/* :bytes || Bytes: */
						if (*ptr) {
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1 && !isdigit((unsigned char) *ptr))
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") overview field %d (%s) mismatch: %s", nntp_caps.over_cmd, artnum, count, ofmt[count].name, ptr);
#endif /* DEBUG */
						}
						break;

					case 7:	/* :lines || Lines: */
						if (*ptr) {
							if (isdigit((unsigned char) *ptr))
								art->line_count = s2i(ptr, 0, INT_MAX);
							else {
								art->line_count = 0;
#ifdef DEBUG
								if ((debug & DEBUG_NNTP) && verbose > 1)
									debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") overview field %d (%s) mismatch: %s", nntp_caps.over_cmd, artnum, count, ofmt[count].name, ptr);
#endif /* DEBUG */
							}
						} else
							art->line_count = 0;
						break;

					default:
						break;
				}
			}

			/* optional fields; for duplicated headers: last match counts, INN >= 2.5.3 does first match counts */
			if (ofmt[count].type == OVER_T_FSTRING) {
				if (*ptr) {
					if (!strcasecmp(ofmt[count].name, "Xref:")) {
						if ((q = parse_header(ptr, "Xref", FALSE, FALSE, FALSE)) != NULL) {
							FreeIfNeeded(art->xref); /* if field is listed more than once in overview.fmt */
							art->xref = my_strdup(q);
						}
#ifdef DEBUG
						else {
							if ((debug & DEBUG_NNTP) && verbose > 1)
								debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") bogus overview-field %s %s", nntp_caps.over_cmd, artnum, ofmt[count].name, ptr);
						}
#endif /* DEBUG */
						continue;
					}
					/*
					 * handling of addition overview fields
					 * goes here
					 */
#ifdef DEBUG
					if ((debug & DEBUG_NNTP) && verbose > 1)
						debug_print_file("NNTP", "%s(%"T_ARTNUM_PFMT") extra overview-field \"%s\" at position %d %s", nntp_caps.over_cmd, artnum, ofmt[count].name, count, ptr);
#endif /* DEBUG */
					/* if we're lucky we've Path in NOV */
					/*
					 * if reading locally cached overview data try
					 * path regardless of the servers OVERVIEW.FMT
					 */
					if (local || !strcasecmp(ofmt[count].name, "Path:")) {
						if ((q = parse_header(ptr, "Path", FALSE, FALSE, FALSE)) != NULL) {
							if (!path_found)
								path_found = TRUE;
							FreeIfNeeded(art->path);
							art->path = my_strdup(q);
#ifdef DEBUG
							if ((debug & DEBUG_NNTP) && verbose > 1 && strcasecmp(ofmt[count].name, "Path:"))
								debug_print_file("NNTP", "\tUsing as \"Path:\" not \"%s\"", ofmt[count].name);
#endif /* DEBUG */
						}
						continue;
					}
				}
				continue;
			}
		}

		/*
		 * RFC says Message-ID is mandatory in newsgroups (but not in
		 * mailgroups etc..) NB. a NULL Message-ID would abort if we ever do
		 * threading in mailgroups
		 */
		if (!art->msgid && group->type == GROUP_TYPE_NEWS)
			continue;

		/* we might lose accuracy here, but that shouldn't hurt */
		if (artnum % (MODULO_COUNT_NUM * 20) == 0)
			show_progress(group_msg, artnum - min, max - min);

		++top_art;				/* Basically this statement commits the article */
	}
#	if defined(DEBUG) && defined(NNTP_ABLE)
	/* log end of multiline response to get timing data */
	if ((debug & DEBUG_NNTP) && fp == FAKE_NNTP_FP && !verbose)
		debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG && NNTP_ABLE */

	free(group_msg);
	TIN_FCLOSE(fp);

	if (tin_errno)
		return -1;

#if defined(NNTP_ABLE) && defined(XHDR_XREF)
	if (read_news_via_nntp && !read_saved_news && !xref_supported && nntp_caps.hdr_cmd) {
		char cbuf[HEADER_LEN];
		int i;
		static t_bool found;
		static t_bool first = TRUE;

		if (first) {
			found = TRUE;
			/*
			 * TODO: if "LIST HEADERS RANGE" failed try "LIST HEADERS"?
			 */
			if (nntp_caps.type == CAPABILITIES && nntp_caps.list_headers) {
				if (!*nntp_caps.headers_range) {
					i = new_nntp_command("LIST HEADERS RANGE", 215, cbuf, sizeof(cbuf));

					found = FALSE;
					switch (i) {
						case 215:
							while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#	ifdef DEBUG
								if (debug & DEBUG_NNTP)
									debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
								if (!found && ((*ptr == ':' && *(ptr + 1) == '\0') || !strncasecmp(ptr, "Xref", 4)))
									found = TRUE;
								nntp_caps.headers_range = my_realloc(nntp_caps.headers_range, strlen(nntp_caps.headers_range) + strlen(ptr) + 2);
								strcat(nntp_caps.headers_range, ptr);
								strcat(nntp_caps.headers_range, "\n");
							}
							break;

						default:
							break;
					}
					first = FALSE;
				} else {
					found = FALSE;
					if ((ptr = strtok(nntp_caps.headers_range, "\n")) != NULL) {
						do {
							if ((*ptr == ':' && *(ptr + 1) == '\0') || !strncasecmp(ptr, "Xref", 4))
								found = TRUE;
						} while (!found && *ptr && (ptr = strtok(NULL, "\n")) != NULL);
					}
				}
			}
		}

		if (found) {
			snprintf(cbuf, sizeof(cbuf), "%s XREF %"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT, nntp_caps.hdr_cmd, min, MAX(min, max));
			group_msg = fmt_string(txt_xref_loop, nntp_caps.hdr_cmd); /* TODO: find a better message */
			if ((fp = nntp_command(cbuf, nntp_caps.hdr ? OK_HDR : OK_HEAD, NULL, 0)) != NULL) { /* RFC 2980 (XHDR) uses 221; RFC 3977 (HDR) uses 225 */
				top_art = 0;
				while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
#	ifdef DEBUG
					if ((debug & DEBUG_NNTP) && verbose)
						debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */

					artnum = atoartnum(ptr);
					if (artnum <= 0 || artnum < group->xmin || artnum > group->xmax)
						continue;
					art = &arts[top_art];
					if (artnum != art->artnum) /* try harder to find a match? while (&arts[i++].artnum != artnum) ...? */
						continue;
					FreeAndNull(art->xref);
					if (!strstr(ptr, "(none)")) {
						if ((q = strchr(ptr, ' ')) == NULL) /* skip article number */
							continue;
						ptr = q;
						while (*ptr && isspace((unsigned char) *ptr))
							++ptr;
						if ((q = strchr(ptr, '\n')) != NULL)
							*q = '\0';
						art->xref = my_strdup(ptr);
					}
					/* we might lose accuracy here, but that shouldn't hurt */
					if (artnum % (MODULO_COUNT_NUM * 20) == 0)
						show_progress(group_msg, artnum - min, max - min);

					++top_art;
				}
#	ifdef DEBUG
				/* log end of multiline response to get timing data */
				if ((debug & DEBUG_NNTP) && !verbose)
					debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG */
			}
			free(group_msg);
		}
	}
#endif /* NNTP_ABLE && XHDR_XREF */

	if (local) {
#ifdef NNTP_ABLE
		if (filter_on_path(group)) {
			int curr_range, range_cnt;
			struct t_article_range *ranges, *curr;
			t_bool supported = TRUE;

			/*
			 * Get the ranges without Path: header and try to fetch the
			 * headers
			 */
			if ((ranges = build_range_list(min, *top, &range_cnt))) {
				curr = ranges;
				curr_range = 1;
				while (curr && supported) {
					if (curr->cnt)
						supported = get_path_header(curr_range++, range_cnt, group, curr->start, curr->end);
					curr = curr->next;
				}
				if (!supported && path_in_ofmt) {
					/*
					 * fetching Path: headers via [X]HDR or XPAT has failed
					 * Path: is in the servers overview so let the next
					 * read_overview() fetch them
					 */
					free_art_array();
					free_msgids();
					top_art = 0;
					*top = T_ARTNUM_CONST(0);
					expired = 0;
				}
				*rebuild_cache = TRUE;
				while (ranges) {
					curr = ranges;
					ranges = curr->next;
					free(curr);
				}
			}
		}
#endif /* NNTP_ABLE */
	} else
		if (!path_found && filter_on_path(group) && !batch_mode) {
#ifdef NNTP_ABLE
			if (!get_path_header(1, 1, group, min, *top))
#endif /* NNTP_ABLE */
				wait_message(2, _(txt_cannot_filter_on_path));
		}
#ifndef NNTP_ABLE
	/* silence compiler warning (unused parameter) */
	(void) rebuild_cache;
#endif /* !NNTP_ABLE */

	EndStopWatch("read_overview()");

	return expired;
}


/*
 * Write an Nov/Xover index file. Fields are separated by '\t'.
 *
 * Format:
 *	1. article number (ie. 183)                [mandatory]
 *	2. Subject: line  (ie. Which newsreader?)  [mandatory]
 *	3. From: line     (ie. iain@ecrc.de)       [mandatory]
 *	4. Date: line     (RFC 822 format)         [mandatory]
 *	5. MessageID:     (ie. <123@example.net>)  [mandatory]
 *	6. References:    (ie. <message-id> ....)  [optional]
 *	7. Byte count     (Skipped - not used)     [mandatory]
 *	8. Line count     (ie. 23)                 [mandatory]
 *	9. Xref: line     (ie. alt.test:389)       [optional]
 *
 * TODO: as we don't use the original data, we currently can't store
 *       the data (from/subject) in the original charset (we don't store
 *       that info). this has the advantage that we can avoid raw 8bit data
 *       in our overviews, but the disadvantage that we might store the data
 *       with a wrong charset and thus lose information. a similar problem
 *       exists with the data for the from:-line, we don't store it in the
 *       original format, whenever our from-parser (partially) fails we'll
 *       lose information in our overviews (but those couldn't be handled
 *       by tin anyway, so this is not a real problem).
 *       long-term solution: store the original data in the overview
 *       (tin has to handle raw 8bit data and other ugly stuff in the
 *       overviews anyway and thus we preserver as much info as possible)
 *       this would require some changes in read_overview() and
 *       parse_headers(): don't do the decoding/unfolding there, but in a
 *       second pass right after write_overview(), or two additional fields
 *       which hold the raw data for from/subject. the latter has the
 *       disadvantage that it costs (much) more memory.
 */
static void
write_overview(
	struct t_group *group)
{
	FILE *fp;
	int i;
	struct t_article *article;
#ifdef CHARSET_CONVERSION
	int c = -1;
#endif /* CHARSET_CONVERSION */

	/*
	 * Can't write or caching is off or getart_limit is set
	 */
	if (no_write || !tinrc.cache_overview_files || ((cmdline.args & CMDLINE_GETART_LIMIT) ? cmdline.getart_limit : tinrc.getart_limit) != 0)
		return;

	if ((fp = open_xover_fp(group, "w", T_ARTNUM_CONST(0), T_ARTNUM_CONST(0), FALSE)) == NULL)
		return;

	if (group->attribute && group->attribute->sort_article_type != SORT_ARTICLES_BY_NOTHING)
		SortBy(artnum_comp);

	/*
	 * Needed to preserve uniqueness in hashed private overview files
	 */
	fprintf(fp, "%s\n", group->name);

#ifdef CHARSET_CONVERSION
	/* get undeclared_charset number if required */
	if (group->attribute && group->attribute->undeclared_charset && *group->attribute->undeclared_charset) {
		for (i = 0; txt_mime_charsets[i] != NULL; i++) {
			if (!strcasecmp(*group->attribute->undeclared_charset, txt_mime_charsets[i])) {
				c = i;
				break;
			}
		}
	}
#endif /* CHARSET_CONVERSION */

	if (batch_mode && verbose > 1)
		wait_message(0, _(txt_writing_group), group->name);

	for_each_art(i) {
		char *p, *q, *ref;

		article = &arts[i];

		if (article->thread != ART_EXPIRED && article->artnum >= group->xmin) {
			ref = NULL;

			if (group->attribute && !group->attribute->post_8bit_header) { /* write encoded data */
				/*
				 * TODO: instead of tinrc.mm_local_charset we'd better use UTF-8
				 *       here and in print_from() in the CHARSET_CONVERSION case.
				 *       note that this requires something like
				 *          buffer_to_network(&article->subject, "UTF-8");
				 *       right before the rfc1522_encode() call.
				 *
				 *       if we would cache the original undecoded data, we could
				 *       ignore stuff like this.
				 */
				p = rfc1522_encode(article->subject, tinrc.mm_local_charset, FALSE);
				/* as the subject might now be folded we have to unfold it */
				unfold_header(p);
			} else { /* raw data */
				p = my_strdup(article->subject);
#ifdef CHARSET_CONVERSION
				if (group->attribute && group->attribute->undeclared_charset && *group->attribute->undeclared_charset && c != -1) /* use undeclared_charset if set (otherwise local charset is used) */
					buffer_to_network(&p, c);
#endif /* CHARSET_CONVERSION */
			}

			/*
			 * replace any '\t's with ' ' in the references-data
			 *
			 * TODO: nntpext-draft might come up with a new scheme:
			 *       For all fields, the value is processed by first
			 *       removing all US-ASCII CRLF pairs and then replacing
			 *       each remaining US-ASCII NUL, TAB, CR, or LF character
			 *       with a single US-ASCII space (for example, CR LF LF TAB
			 *       will become two spaces).
			 */
			if (article->refs) {
				ref = q = my_strdup(article->refs);
				while (*q) {
					if (*q == '\t')
						*q = ' ';
					++q;
				}
			}

			{
				char date[30];
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
				char *old_lc_all = NULL, *old_lc_time = NULL;

				/* Unlocalized date-header */
				if (getenv("LC_ALL") != NULL) {
					old_lc_all = my_strdup(setlocale(LC_ALL, NULL));
					setlocale(LC_ALL, "POSIX");
				} else {
					old_lc_time = my_strdup(setlocale(LC_TIME, NULL));
					setlocale(LC_TIME, "POSIX");
				}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */

				if (!my_strftime(date, sizeof(date) - 1, "%d %b %Y %H:%M:%S GMT", gmtime(&article->date)))
					snprintf(date, sizeof(date) - 1, "01 Jan 1970 00:00:00 UTC");

				fprintf(fp, "%"T_ARTNUM_PFMT"\t%s\t%s\t%s\t%s\t%s\t%d\t%d",
					article->artnum,
					p,
#ifdef CHARSET_CONVERSION
					print_from(group, article, c),
#else
					print_from(group, article, -1),
#endif /* CHARSET_CONVERSION */
					date,
					BlankIfNull(article->msgid),
					BlankIfNull(ref),
					0,	/* bytes */
					article->line_count);
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
				/* change back LC_* */
				if (old_lc_all != NULL) {
					setlocale(LC_ALL, old_lc_all);
					free(old_lc_all);
				} else if (old_lc_time != NULL) {
					setlocale(LC_TIME, old_lc_time);
					free(old_lc_time);
				}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */
			}

			if (article->xref)
				fprintf(fp, "\tXref: %s", article->xref);

			if (article->path)
				fprintf(fp, "\tPath: %s", article->path);

			fprintf(fp, "\n");

			free(p);
			if (article->refs) {
				FreeIfNeeded(ref);
			}
		}
		if (i % (MODULO_COUNT_NUM * 20) == 0)
			show_progress(_(txt_writing_overview), i, top_art);
	}
#ifdef HAVE_FCHMOD
	fchmod(fileno(fp), (mode_t) (S_IWUSR|S_IRUGO));
#else
#	ifdef HAVE_CHMOD
		chmod(find_nov_file(group, R_OK), (mode_t) (S_IWUSR|S_IRUGO));
#	endif /* HAVE_CHMOD */
#endif /* HAVE_FCHMOD */
	fclose(fp);
}


/*
 * A complex little function to determine the correct overview index file
 * according to 'mode' (read or write)
 * NULL is returned if the current setup dictates otherwise
 *
 * GROUP_TYPE_MAIL index files are read/written in ~/.tin/.mail
 * GROUP_TYPE_SAVE index files are read/written in ~/.tin/.save
 *
 * Both of these are hashed
 *
 * GROUP_TYPE_NEWS index files are a little bit more complex
 *
 * When hashing the index filename will be in format number.number.
 * Hashing the groupname gets a number. See if that #.1 file exists;
 * if so, read first line. Is this the group we want? If no, try #.2.
 * Repeat until no such file or we find an existing file that matches
 * our group. Return pointer to path or NULL if not found.
 */
static char *
find_nov_file(
	struct t_group *group,
	int mode)
{
	FILE *fp;
	const char *dir;
	char buf[PATH_LEN];
	unsigned int i;
	unsigned long hash;
	struct stat sb;
	static char nov_file[PATH_LEN];
	static t_bool once_only = FALSE;	/* Trap things that are done only 1 time */

	if (group == NULL || (mode != R_OK && mode != W_OK))
		return NULL;

	switch (group->type) {
		case GROUP_TYPE_MAIL:
			dir = index_maildir;
			break;

		case GROUP_TYPE_SAVE:
			dir = index_savedir;
			break;

		case GROUP_TYPE_NEWS:
			/*
			 * nntp.caps.over_cmd is not an issue here, any gripes and warnings
			 * about [X]OVER are handled in nntp_open()
			 */

			/*
			 * When reading via NNTP, system wide overviews are irrelevant, of
			 * course, and the private overview filename will be the same for
			 * both reading and writing.
			 *
			 * When working locally, we only use a private cache for reading
			 * if requested and when system wide overviews don't already exist.
			 * When writing then only private overviews can be used since
			 * updating system wide overviews is not safe wrt locking etc.
			 *
			 * See if local overview file $SPOOLDIR/<groupname>/.overview exists
			 *
			 * INN >= 2.3.0 uses a new naming schemme with tradindexed;
			 * buffindexed and ovdb are not covered by the code at all.
			 */
#ifndef NNTP_ONLY
			if (!read_news_via_nntp) {
				make_base_group_path(novrootdir, group->name, buf, sizeof(buf));
				joinpath(nov_file, sizeof(nov_file), buf, novfilename);
				if (access(nov_file, R_OK) == 0) {
					if (mode == R_OK)
						return nov_file; /* system wide "classic" overviews */
					else
						return NULL;	/* Don't write to system wide overviews */
				} else { /* ugly hack for inn >= 2.3.0 with ovmethod tradindexed */
					char *gn = my_strdup(group->name);
					size_t t;
					t_bool w = FALSE;

					for (t = 1, i = 1; t < strlen(group->name); t++) {
						if (!w) {
							if (group->name[t] == '.') {
								gn[i++] = '/';
								w = TRUE;
							}
						} else { /* TODO: check against inns code */
							if (group->name[t] != '.') { /* illegal .. in name? */
								gn[i++] = group->name[t];
								w = FALSE;
							}
						}
					}
					gn[i] = '\0';

					joinpath(nov_file, sizeof(nov_file), novrootdir, gn);
					free(gn);
					snprintf(nov_file + strlen(nov_file), sizeof(nov_file) - strlen(nov_file), "/%s.DAT", group->name);
				}
				if (access(nov_file, R_OK) == 0) {
					if (mode == R_OK) {
						/* STRCPY(novfilename, ".DAT"); */ /* would be just to fix the name in make_connection_page() */
						return nov_file;		/* Use system wide overviews */
					} else
						return NULL;			/* Don't write to system wide overviews */
				}
			}
#endif /* !NNTP_ONLY */

			/*
			 * We only get here when private overviews are going to be used
			 * Go no further if they are explicitly turned off
			 */
			if (!tinrc.cache_overview_files)
				return NULL;

			/*
			 * Append -<nntpserver> to private cache dir
			 */
			if (!once_only && nntp_server) {
				size_t sp = sizeof(index_newsdir), ln = strlen(index_newsdir);

				if (sp > ln + 3) {
					char *srv = my_strdup(nntp_server);

					str_lwr(srv);
					strcat(index_newsdir, "-");
					my_strncpy(index_newsdir + ln + 1, srv, --sp);
					free(srv);
				}
				once_only = TRUE;
			}

			/*
			 * Only try to set up the private cache when writing. If it
			 * doesn't exist yet, then ergo we can't read from it.
			 * The cache will be checked/created on every write; a previous
			 * bug report complained that this was not the case
			 */
			if (stat(index_newsdir, &sb) == -1) {			/* Private cache doesn't exist */
				if (mode == R_OK)
					return NULL;
				if (my_mkdir(index_newsdir, (mode_t) S_IRWXU) != 0)
					return NULL;
			} else {
				if (!S_ISDIR(sb.st_mode))
					return NULL;
			}

			/*
			 * Update the newsgroups cache to point to the new location
			 * now that we know it is valid
			 */
			if (!once_only)
				joinpath(local_newsgroups_file, sizeof(local_newsgroups_file), index_newsdir, NEWSGROUPS_FILE);

			dir = index_newsdir;
			break;

		default: /* not reached */
			return NULL;
	}

	/*
	 * We only get here if writing to a private overview.
	 * These always have hashed filenames.
	 * Try <hash>.<seqno> and check the group name tagline until
	 * matching index file is found. If not found return next unused
	 * filename
	 */
	hash = hash_groupname(group->name);

	for (i = 1; i < INT_MAX; i++) {
		char *ptr;

		snprintf(buf, sizeof(buf), "%lu.%u", hash, i);
		joinpath(nov_file, sizeof(nov_file), dir, buf);

		if ((fp = tin_fopen(nov_file, "r")) == NULL) /* file not found or empty -> name can be used, leave loop */
			break;

		if ((ptr = tin_fgets(fp, FALSE)) != NULL) { /* grab 1st line */
			if (strcmp(ptr, group->name)) {/* name mismatch try next */
				fclose(fp);
				continue;
			}
		}
		/* match, leave loop */
		fclose(fp);
		break;
	}

	return nov_file;
}


/*
 * Run the index file updater only for the groups we've loaded.
 */
void
do_update(
	t_bool catchup)
{
	int i, j, k = 0;
	time_t beg_epoch = 0;
	struct t_article *art;
	struct t_group *group;

	if (verbose)
		(void) time(&beg_epoch);

	/*
	 * loop through groups and update any required index files
	 */
	for (i = 0; i < selmenu.max; i++) {
		group = &active[my_group[i]];
		/*
		 * FIXME: workaround to get a valid CURR_GROUP
		 * it also points to the currently processed group so that
		 * the correct attributes are used
		 * The correct fix is to get rid of CURR_GROUP
		 */
		selmenu.curr = i;

		if (group->bogus || !group->subscribed)
			continue;

		if (!index_group(group)) {
			for_each_art(j) {
				art = &arts[j];
				FreeAndNull(art->refs);
				FreeAndNull(art->msgid);
			}
			continue;
		}

		++k;

		if (verbose) {
			my_printf("%s %s\n", (catchup ? _(txt_catchup) : _(txt_updating)), group->name);
			my_flush();
		}

		if (catchup) {
			for_each_art(j)
				art_mark(group, &arts[j], ART_READ);
		}
	}

	if (verbose) {
		wait_message(0, _(txt_catchup_update_info),
			(catchup ? _(txt_caughtup) : _(txt_updated)), k,
			PLURAL(selmenu.max, txt_group), (unsigned long int) (time(NULL) - beg_epoch));
	}
}


static int
artnum_comp(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * s1->artnum less than s2->artnum
	 */
	if (s1->artnum < s2->artnum)
		return -1;

	/*
	 * s1->artnum greater than s2->artnum
	 */
	if (s1->artnum > s2->artnum)
		return 1;

	return 0;
}


/*
 * return result of strcmp (reversed for descending)
 */
static int
subj_comp_asc(
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if ((retval = strcasecmp(s1->subject, s2->subject))) /* != 0 */
		return retval;

	return s1->date - s2->date > 0 ? 1 : -1;
}


static int
subj_comp_desc(
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if ((retval = strcasecmp(s2->subject, s1->subject))) /* != 0 */
		return retval;

	return s1->date - s2->date > 0 ? 1 : -1;
}


/*
 * return result of strcmp (reversed for descending)
 */
static int
from_comp_asc(
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if ((retval = strcasecmp(s1->mailbox.from, s2->mailbox.from))) /* != 0 */
		return retval;

	return s1->date - s2->date > 0 ? 1 : -1;
}


static int
from_comp_desc(
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if ((retval = strcasecmp(s2->mailbox.from, s1->mailbox.from))) /* != 0 */
		return retval;

	return s1->date - s2->date > 0 ? 1 : -1;
}


/*
 * Works like strcmp() for comparing time_t type values
 * Return codes:
 *  -1:		If p1 is before p2
 *   0:		If they are the same time
 *   1:		If p1 is after p2
 * If the sort order is _not_ DATE_ASCEND then the sense of the above
 * is reversed.
 */
static int
date_comp_asc(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * s1->date less than s2->date
	 */
	if (s1->date < s2->date)
		return -1;

	/*
	 * s1->date greater than s2->date
	 */
	if (s1->date > s2->date)
		return 1;

	return 0;
}


static int
date_comp_desc(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * s2->date less than s1->date
	 */
	if (s2->date < s1->date)
		return -1;

	/*
	 * s2->date greater than s1->date
	 */
	if (s2->date > s1->date)
		return 1;

	return 0;
}


/*
 * Same again, but for art[].score
 */
static int
score_comp_asc(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (s1->score < s2->score)
		return -1;

	if (s1->score > s2->score)
		return 1;

	return s1->date - s2->date > 0 ? 1 : -1;
}


static int
score_comp_desc(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (s2->score < s1->score)
		return -1;

	if (s2->score > s1->score)
		return 1;

	return s1->date - s2->date > 0 ? 1 : -1;
}


/*
 * Same again, but for art[].line_count
 */
static int
lines_comp_asc(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (s1->line_count < s2->line_count)
		return -1;

	if (s1->line_count > s2->line_count)
		return 1;

	return s1->date - s2->date > 0 ? 1 : -1;
}


static int
lines_comp_desc(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (s2->line_count < s1->line_count)
		return -1;

	if (s2->line_count > s1->line_count)
		return 1;

	return s1->date - s2->date > 0 ? 1 : -1;
}


/*
 * Compares the total score of two threads. Used for sorting base[].
 */
static int
score_comp_base(
	t_comptype p1,
	t_comptype p2)
{
	int a = get_score_of_thread((int) *(const t_artnum *) p1);
	int b = get_score_of_thread((int) *(const t_artnum *) p2);

	/* If scores are equal, compare using the article sort order.
	 * This determines the order in a group of equally scored threads.
	 */
	if (a == b) {
		t_comptype s1 = &arts[*(const t_artnum *) p1];
		t_comptype s2 = &arts[*(const t_artnum *) p2];
		t_compfunc comp_func = eval_sort_arts_func(CURR_GROUP.attribute->sort_article_type);

		if (comp_func)
			return (*comp_func)(s1, s2);

		return 0;
	}

	if (CURR_GROUP.attribute->sort_threads_type == SORT_THREADS_BY_SCORE_ASCEND)
		return a > b ? 1 : -1;

	return a < b ? 1 : -1;
}


/*
 * Compare the date of the last posted article of two threads.
 * Used for sorting base[].
 */
static int
last_date_comp_base_desc(
	t_comptype p1,
	t_comptype p2)
{
	time_t s1_last = get_last_posting_date(*(const t_artnum *) p1);
	time_t s2_last = get_last_posting_date(*(const t_artnum *) p2);

	if (s2_last < s1_last)
		return -1;

	if (s2_last > s1_last)
		return 1;

	return 0;
}


static int
last_date_comp_base_asc(
	t_comptype p1,
	t_comptype p2)
{
	time_t s1_last = get_last_posting_date(*(const t_artnum *) p1);
	time_t s2_last = get_last_posting_date(*(const t_artnum *) p2);

	if (s2_last > s1_last)
		return -1;

	if (s2_last < s1_last)
		return 1;

	return 0;
}


static time_t
get_last_posting_date(
	t_artnum n)
{
	t_artnum i;
	time_t last = (time_t) 0;

	for (i = n; i >= 0; i = arts[i].thread) {
		if (arts[i].date > last)
			last = arts[i].date;
	}

	return last;
}


void
set_article(
	struct t_article *art)
{
	art->subject = NULL;
	art->date = (time_t) 0;
	art->xref = NULL;
	art->msgid = NULL;
	art->refs = NULL;
	art->refptr = NULL;
	art->mailbox.from = NULL;
	art->mailbox.name = NULL;
	art->mailbox.next = NULL;
	art->line_count = -1;
	art->tagged = 0;
	art->thread = ART_EXPIRED;
	art->prev = ART_NORMAL;
	art->score = 0;
	art->status = ART_UNREAD;
	art->killed = ART_NOTKILLED;
	art->zombie = FALSE;
	art->delete_it = FALSE;
	art->selected = FALSE;
	art->inrange = FALSE;
	art->matched = FALSE;
	art->keep_in_base = FALSE;
	art->multipart_subj = FALSE;
}


/*
 * Do a binary chop to see if 'art' (an article number) exists in arts[]
 * Naturally arts[] must be sorted on artnum
 * Return index into arts[] or -1
 */
static int
valid_artnum(
	t_artnum art)
{
	int prev, range;
	int dctop = top_art;
	int cur = 1;

	while ((dctop >>= 1))
		cur <<= 1;

	range = cur >> 1;
	cur--;

	forever {
		if (arts[cur].artnum == art)
			return cur;

		prev = cur;
		cur += ((arts[cur].artnum < art) ? range : -range);
		if (prev == cur)
			break;

		if (cur >= top_art)
			cur = top_art - 1;

		range >>= 1;
	}
	return -1;
}


/*
 * Loop over arts[] to see if 'art' (an article number) exists in arts[]
 * Needed if arts[] is not sorted on artnum
 * Return index into arts[] or -1
 */
int
find_artnum(
	t_artnum art)
{
	int i;

	for_each_art(i) {
		if (arts[i].artnum == art)
			return i;
	}
	return -1;
}


static char *
print_from(
	struct t_group *group,
	struct t_article *article,
	int charset)
{
	char *p, *q;
	static char from[PATH_LEN];
	char single_from[PATH_LEN];
	int c_needed = 0;
	struct t_mailbox *mb = &article->mailbox;

/*	if (!mb)
		return from; */

	*from = *single_from = '\0';

	do {
		if (mb->name != NULL) {
			q = my_strdup(mb->name);
#ifdef CHARSET_CONVERSION
			if (charset != -1)
				buffer_to_network(&q, charset);
#else
			(void) charset;
#endif /* CHARSET_CONVERSION */
			p = rfc1522_encode(mb->name, tinrc.mm_local_charset, FALSE);
			unfold_header(p);
			if (CHECK_RFC5322_SPECIALS(mb->name)) {
				if (group->attribute)
					snprintf(single_from, sizeof(single_from), "\"%s\" <%s>", group->attribute->post_8bit_header ? q : p, mb->from);
				else
					snprintf(single_from, sizeof(single_from), "\"%s\" <%s>", tinrc.post_8bit_header ? q : p, mb->from);
			} else {
				if (group->attribute)
					snprintf(single_from, sizeof(single_from), "%s <%s>", group->attribute->post_8bit_header ? q : p, mb->from);
				else
					snprintf(single_from, sizeof(single_from), "%s <%s>", tinrc.post_8bit_header ? q : p, mb->from);
			}

			free(p);
			free(q);
		} else
			snprintf(single_from, sizeof(single_from), "<%s>", mb->from);

		if (strlen(from) + strlen(single_from) + /* strlen(", ") */ 2 < PATH_LEN) {
			if (c_needed++)
				strcat(from, ", ");
			strcat(from, single_from);
		}

		mb = mb->next;
	} while (mb);

	return from;
}


/*
 * Open a group news overview file
 * Use NNTP XOVER where possible unless 'local' is set
 */
static FILE *
open_xover_fp(
	struct t_group *group,
	const char *mode,
	t_artnum min,
	t_artnum max,
	t_bool local)
{
#ifdef NNTP_ABLE
	if (!local && nntp_caps.over_cmd && *mode == 'r' && group->type == GROUP_TYPE_NEWS) {
		char line[NNTP_STRLEN];

		if (!max)
			return NULL;

		if (min == max)
			snprintf(line, sizeof(line), "%s %"T_ARTNUM_PFMT, nntp_caps.over_cmd, min);
		else
			snprintf(line, sizeof(line), "%s %"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT, nntp_caps.over_cmd, min, MAX(min, max));

		return (nntp_command(line, OK_XOVER, NULL, 0));
	}
#else
	/* silence compiler warning (unused parameter) */
	(void) min;
	(void) max;
	(void) local;
#endif /* NNTP_ABLE */
	{
		FILE *fp;
		char *nov_file = find_nov_file(group, (*mode == 'r') ? R_OK : W_OK);

		if (nov_file != NULL) {
			if ((fp = fopen(nov_file, mode)) != NULL)
				return fp;

			if (*mode != 'r')
				perror_message(_(txt_cannot_open), nov_file);
		}
	}
	return NULL;
}


#ifdef USE_HEAPSORT
int
tin_sort(
	void *sbase,
	size_t nel,
	size_t width,
	t_compfunc compar)
{
	int rc;

	switch (tinrc.sort_function) {
		case 0:
			qsort(sbase, nel, width, compar);
			rc = 0;
			break;

		case 1:
			rc = heapsort(sbase, nel, width, compar);
			break;

		default:
			rc = -1;
			break;
	}
	return rc;
}
#endif /* USE_HEAPSORT */
