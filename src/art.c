/*
 *  Project   : tin - a Usenet reader
 *  Module    : art.c
 *  Author    : I.Lea & R.Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2003-08-25
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */

#ifndef STPWATCH_H
#	include "stpwatch.h"
#endif /* !STPWATCH_H */

/*
 * TODO: fixup to remove CURR_GROUP dependency in all sort funcs
 */
#define SortBy(func)	qsort(arts, (size_t) top_art, sizeof(struct t_article), func);

static long last_read_article;
int top_art = 0;				/* # of articles in arts[] */

/*
 * Local prototypes
 */
static FILE *open_xover_fp(struct t_group *group, const char *mode, long min, long max, t_bool local);
static char *find_nov_file(struct t_group *group, int mode);
static char *print_date(time_t secs);
static char *print_from(struct t_article *article);
static int artnum_comp(t_comptype p1, t_comptype p2);
static int date_comp(t_comptype p1, t_comptype p2);
static int from_comp(t_comptype p1, t_comptype p2);
static int global_get_multiparts(int aindex, MultiPartInfo **malloc_and_setme_info);
static int global_look_for_multipart_info(int aindex, MultiPartInfo *setme, char start, char stop, int *offset);
static int lines_comp(t_comptype p1, t_comptype p2);
static int read_nov_file(struct t_group *group, long min, long max, int *expired, t_bool local);
static int read_group(struct t_group *group, int *pcount);
static int score_comp(t_comptype p1, t_comptype p2);
static int score_comp_base(t_comptype p1, t_comptype p2);
static int subj_comp(t_comptype p1, t_comptype p2);
static int valid_artnum(long art);
static long find_first_unread(struct t_group *group);
static t_bool parse_headers(FILE *fp, struct t_article *h);
static t_compfunc eval_sort_arts_func(unsigned int sort_art_type);
static void sort_base(unsigned int sort_threads_type);
static void thread_by_subject(void);
static void thread_by_multipart(void);


/*
 * Display a suitable 'entering group' message if screen needs redrawing
 * Allow for the non-printing %s, and the %-age counter
 */
void
show_art_msg(
	const char *group)
{
/* what if cCOLS < (strlen)+18? */
	wait_message(0, _(txt_group), cCOLS - strlen(_(txt_group)) + 2 - 3, group);
}


/*
 * Construct the pointers to the base article in each thread.
 * If we are showing only unread, then point to the first unread. I have
 * no idea why this should be so, it causes problems elsewhere [which_response]
 * .prev is set on each article that is after the first article in the
 * thread and points to the previous article. Articles which have been expired
 * have their .thread set to ART_EXPIRED
 */
void
find_base(
	struct t_group *group)
{
	int i;
	int j;

	grpmenu.max = 0;

#ifdef DEBUG
	debug_print_arts();
#endif /* DEBUG */

	if (group->attribute->show_only_unread) {
		for_each_art(i) {
			if (arts[i].prev >= 0 || arts[i].thread == ART_EXPIRED || (arts[i].killed && tinrc.kill_level == KILL_NOTHREAD))
				continue;

			if (grpmenu.max >= max_art)
				expand_art();

			if (arts[i].status != ART_READ)
				base[grpmenu.max++] = i;
			else {
				for (j = i; j >= 0; j = arts[j].thread) {
					if (arts[j].status != ART_READ) {
						base[grpmenu.max++] = i;
						break;
					}
				}
			}
		}
	} else {
		for_each_art(i) {
			if (arts[i].prev >= 0 || arts[i].thread == ART_EXPIRED || (arts[i].killed && tinrc.kill_level == KILL_NOTHREAD))
				continue;

			if (grpmenu.max >= max_art)
				expand_art();

			base[grpmenu.max++] = i;
		}
	}
	/* sort base[] */
	if (group->attribute->sort_threads_type > SORT_THREADS_BY_NOTHING)
		sort_base(group->attribute->sort_threads_type);
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
	int count;
	int expired;
	int modified;
	long min;
	long max;
	t_bool caching_xover;
	t_bool filtered;

	if (group == NULL)
		return TRUE;

	if (!batch_mode)
		show_art_msg(group->name);

	signal_context = cArt;			/* Set this only once curr_group is valid */

	hash_reclaim();
	free_art_array();
	free_msgids();

	/*
	 * Load articles within min..max from xover index file if it exists
	 * and then create base[] article numbers from loaded articles.
	 * If nov file does not exist then create base[] with setup_hard_base().
	 */
	BegStopWatch("setup_hard_base");

	if (setup_hard_base(group) < 0)
		return FALSE;

	EndStopWatch();
	PrintStopWatch();

#ifdef DEBUG_NEWSRC
	debug_print_comment("Before read_nov_file");
	debug_print_bitmap(group, NULL);
#endif /* DEBUG_NEWSRC */

	min = grpmenu.max ? base[0] : group->xmin;
	max = grpmenu.max ? base[grpmenu.max - 1] : min - 1;

	if (tinrc.getart_limit > 0) {
		if (grpmenu.max && (grpmenu.max > tinrc.getart_limit))
			min = base[grpmenu.max - tinrc.getart_limit];
	} else if (tinrc.getart_limit < 0) {
		long first_unread = find_first_unread(group);

		if (min - first_unread < tinrc.getart_limit)
			min = first_unread + tinrc.getart_limit;
	}

	/*
	 * Quit now if no articles
	 */
	if (max < 0)
		return FALSE;

	top_art = 0;
	last_read_article = 0L;

	/*
	 * Read in the existing overview
	 * This read has local=TRUE set if locally caching XOVER records to ensure
	 * we pull in any private overview caches in preference to using using OVER
	 *
	 * When reading local spool, this will pull in the system wide overview
	 * cache (if found) otherwise the private overview cache will be read
	 */
	caching_xover = (tinrc.cache_overview_files && xover_cmd && group->type == GROUP_TYPE_NEWS);

	if (read_nov_file(group, min, max, &expired, caching_xover) == -1)
		return FALSE;	/* user aborted indexing */

	/*
	 * Fill in the range last_read_article...max using XOVER
	 * Only do this if the previous read_nov_file() was against private cache
	 */
	if ((last_read_article < max) && caching_xover) {
		if (read_nov_file(group, (last_read_article >= min) ? last_read_article + 1 : min, max, &expired, FALSE) == -1)
			return FALSE;	/* user aborted indexing */
	} else
		caching_xover = FALSE;

	/*
	 * Add any articles to arts[] that are new or were killed
	 */
	if ((modified = read_group(group, &count)) == -1)
		return FALSE;		/* user aborted indexing */

	/*
	 * Do this before calling art_mark(,, ART_READ) if you want
	 * the unread count to be correct.
	 */
#ifdef DEBUG_NEWSRC
	debug_print_comment("Before parse_unread_arts()");
	debug_print_bitmap(group, NULL);
#endif /* DEBUG_NEWSRC */
	parse_unread_arts(group);
#ifdef DEBUG_NEWSRC
	debug_print_comment("After parse_unread_arts()");
	debug_print_bitmap(group, NULL);
#endif /* DEBUG_NEWSRC */

	/*
	 * Stat all articles to see if any have expired
	 */
	for_each_art(i) {
		if (arts[i].thread == ART_EXPIRED) {
			expired = 1;
#ifdef DEBUG_NEWSRC
			debug_print_comment("art.c: index_group() purging...");
#endif /* DEBUG_NEWSRC */
			art_mark(group, &arts[i], ART_READ);
		}
	}

	/*
	 * Only rewrite the index if it has changed
	 */
	if (expired || modified || caching_xover)
		write_nov_file(group);

	/*
	 * Create the reference tree. The msgid and ref ptrs will
	 * be free()d now that the NovFile has been written.
	 */
	build_references(group);

	/*
	 * Needs access to the reference tree
	 */
	filtered = filter_articles(group);

	BegStopWatch("make_thread");

	make_threads(group, FALSE);

	EndStopWatch();
	PrintStopWatch();

	if ((modified || filtered) && !batch_mode)
		clear_message();

	return TRUE;
}


/*
 * Returns number of first unread article
 */
static long
find_first_unread(
	struct t_group *group)
{
	unsigned char *p;
	unsigned char *end = group->newsrc.xbitmap;
	long first = group->newsrc.xmin; /* initial value */

	if ((p = group->newsrc.xbitmap)) {
		end += group->newsrc.xbitlen / 8;
		for (; *p == '\0' && p < end; p++, first += 8)
			;
	}
	return first;
}


/*
 * Called after XOVER/local/private overview databases have been loaded
 * Read and parse in headers for any arts not already found (usually
 * new articles that have not been indexed yet)
 * Already present articles that pass valid_artnum() have their
 * ->thread set to ART_NORMAL, as do any new articles that are added.
 *
 * Return values are:
 *    1   Additional articles were read in
 *    0   No additional (new) articles were found
 *   -1   user aborted during read
 * TODO: think of a function name that sucks less
 * TODO: *pcount isn't used by the caller (index_group)
 */
static int
read_group(
	struct t_group *group,
	int *pcount)
{
	FILE *fp;
	char buf[PATH_LEN];
	int i;
	int count = 0;
	int respnum, total = 0;
	int modified = 0;
	long art;
	t_bool res;
	static char dir[PATH_LEN] = "";

	/*
	 * Change to groups spooldir to optimize fopen()'s on local articles
	 */
	if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS) {
		get_cwd(dir);
		make_base_group_path(group->spooldir, group->name, buf);
		my_chdir(buf);
	}

	/*
	 * Count num of arts to index so the user has an idea of index time
	 */
	for (i = 0; i < grpmenu.max; i++) {
		if (base[i] <= last_read_article || valid_artnum(base[i]) >= 0)
			continue;

		total++;
	}

	/*
	 * Reset the next article number index (for when HEAD fails)
	 */
	head_next = -1;

	for (i = 0; i < grpmenu.max; i++) {	/* for each article # */
		art = base[i];

		/*
		 * Do we already have this article in our index? Change
		 * arts[].thread from ART_EXPIRED to ART_NORMAL and skip
		 * reading the header.
		 */
		if ((respnum = valid_artnum(art)) >= 0) {
			arts[respnum].thread = ART_NORMAL;
			continue;
		}
		if (art <= last_read_article)
			continue;

		/*
		 * Try and open the article
		 */
		if ((fp = open_art_header(art)) == NULL)
			continue;

		/*
		 * we've modified the index so it will need to be re-written
		 */
		modified = 1;

		/*
		 * Add article to arts[]
		 */
		if (top_art >= max_art)
			expand_art();

		set_article(&arts[top_art]);
		arts[top_art].artnum = art;
		arts[top_art].thread = ART_NORMAL;

		res = parse_headers(fp, &arts[top_art]);

		TIN_FCLOSE(fp);
		if (tin_errno) {
			if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS)
				my_chdir(dir);

			return -1;
		}

		if (!res) {
			snprintf(buf, sizeof(buf), "FAILED parse_headers(%ld)", art);
#ifdef DEBUG
			debug_nntp("read_group", buf);
#endif /* DEBUG */
			continue;
		}

		last_read_article = arts[top_art].artnum;	/* used if arts are killed */
		top_art++;

		if (++count % MODULO_COUNT_NUM == 0)
			show_progress(mesg, count, total);
	}

	/*
	 * Update number of article we 'read'
	 */
	*pcount = count;

	/*
	 * if !nntp change to previous dir before indexing started
	 */
	if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS)
		my_chdir(dir);

	return modified;
}


/*
 * The algorithm is elegant, using the fact that identical Subject lines
 * are hashed to the same node in table[] (see hashstr.c)
 *
 * Mark i as being in j's thread list if
 * . The article is _not_ being ignored
 * . The article is not already threaded
 * . One of the following is true:
 *    1) The subject lines are the same
 *    2) Both are part of the same archive (name's match and arch bit set)
 * IMHO the tests for archive name are redundant and have been for years
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
		h = (struct t_hashnode *) (arts[i].subject - sizeof(int) - sizeof(void *)); /* FIXME: cast increases required alignment of target type */

		j = h->aptr;

		if (j != -1 && j < i) {
			if (arts[i].prev == ART_NORMAL &&
					((arts[i].subject == arts[j].subject) /* ||
					 (arts[i].archive && arts[j].archive && (arts[i].archive->name == arts[j].archive->name)) */ )) {
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
 * This was brought over from tags.c, however this version doesn't not
 * opperate on base_index
 *
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


/*
 *	Again this was taken from tags.c, but works on global indicies into arts
 *	rather then on base_index.
 */
static int
global_look_for_multipart_info(
	int aindex,
	MultiPartInfo* setme,
	char start,
	char stop,
	int *offset)
{
	char *subj = (char *) 0;
	char *pch = (char *) 0;
	MultiPartInfo tmp;

	*offset = 0;

	/* entry assertions */
	assert(0 <= aindex && aindex < top_art && "invalid index");
	assert(setme != NULL && "setme must not be NULL");

	/* parse the message */
	subj = arts[aindex].subject;
	pch = strrchr(subj, start);
	if (!pch || !isdigit((int) pch[1]))
		return 0;

	tmp.base_index = aindex; /* This could be confusing because we are actually storing the index into arts, not the base_index. */
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
 * Taken from tags.c but changed to use indicies into arts[] instead of
 * base_index. Changed so that even when we don't have all the parts we
 * return a valid array.
 *
 * Tries to find all the parts to the multipart message pointed to by
 * base_index.
 *
 * @return on success, the number of parts found. On failure, zero if not a
 * multipart or the negative value of the first missing part.
 * @param base_index index pointing to one of the messages in a multipart
 * message.
 * @param malloc_and_setme_info on success, set to a malloced array the
 * parts found.
 */
static int
global_get_multiparts(
	int aindex,
	MultiPartInfo **malloc_and_setme_info)
{
	int i = 0;
	int part_index;
	MultiPartInfo tmp, tmp2;
	MultiPartInfo *info = 0;

	/* entry assertions */
	assert(0 <= aindex && aindex < top_art && "Invalid index");
	assert(malloc_and_setme_info != NULL && "malloc_and_setme_info must not be NULL");

	/* make sure this is a multipart message... */
	if (!global_get_multipart_info(aindex, &tmp) || tmp.total < 1)
		return 0;

	/* make a temporary buffer to hold the multipart info... */
	info = my_malloc(sizeof(MultiPartInfo) * tmp.total);

	/* zero out part-number for the repost check below */
	for (i = 0; i < tmp.total; ++i) {
		info[i].total = tmp.total; /* Added this for thread_by_multipart */
		info[i].part_number = -1;
	}

	/* try to find all the multiparts... */
	for_each_art(i) {
		if (strncmp(arts[i].subject, tmp.subject, tmp.subject_compare_len))
			continue;

		if (!global_get_multipart_info(i, &tmp2))
			continue;

		/* 'test (1/5)' is not the same as 'test (1/15)' */
		if (tmp.total != tmp2.total)
			continue;

		part_index = tmp2.part_number - 1;

		/*
		 * skip "blah (00/102)" or "blah (103/102)" subjects
		 */
		if (part_index < 0 || part_index >= tmp.total)
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
			*malloc_and_setme_info = info;
			return -(i + 1); /* missing part #(i+1) */
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

		if (IGNORE_ART_THREAD(i) || arts[i].prev >= 0 || !global_get_multiparts(i, &minfo))
			continue;

		threadNum = -1;
		for (j = minfo[0].total - 1; j >= 0; j--) {
			if (minfo[j].part_number != -1) {
				if (threadNum != -1) {
					arts[minfo[j].base_index].thread = threadNum;
					arts[threadNum].prev = minfo[j].base_index;
				}

				threadNum = minfo[j].base_index;
			}
		}
	}
	free(minfo);
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
 *
 * Apart from THREAD_NONE, .thread and .prev are used, the
 * first article in a thread should have .prev set to ART_NORMAL, the
 * rest >= 0. Only do unexpired articles we haven't visited yet
 * (arts[].thread == -1 ART_NORMAL).
 *
 * The rethread parameter is a misnomer. Its only effect (if set) is
 * to delete all threading information, not to rethread
 */
/* TODO: rewrite that user can easly combine different 'threading'
 *       methods, i.e:
 *       - thread_by_multipart() + collate_subjects()
 */
void
make_threads(
	struct t_group *group,
	t_bool rethread)
{
	int i;

	if (!cmd_line && !batch_mode)
		info_message((group->attribute->thread_arts == THREAD_NONE ? _(txt_unthreading_arts) : _(txt_threading_arts)));

#ifdef DEBUG
	if (debug == 2)
		error_message("rethread=[%d]  thread_arts=[%d]  attr_thread_arts=[%d]",
				rethread, tinrc.thread_articles, group->attribute->thread_arts);
#endif /* DEBUG */

	/*
	 * Sort all the articles using the preferred method
	 * When find_base() is called, the bases are created ordered
	 * on arts[] and so the base messages under all threading systems
	 * will be sorted in this way.
	 */
	sort_arts(group->attribute->sort_art_type);

	/*
	 * Reset all the ptrs to articles following the above sort
	 */
	clear_art_ptrs();

	/*
	 * The threading pointers need to be reset if re-threading
	 *	If using ref threading, revector the links back to the articles
	 */
	if (rethread || group->attribute->thread_arts) {
		for_each_art(i) {
			if (arts[i].thread != ART_EXPIRED)
				arts[i].thread = ART_NORMAL;

			arts[i].prev = ART_NORMAL;

			/* Should never happen if tree is built properly */
			if (arts[i].refptr == 0) {
				my_fprintf(stderr, "\nError  : art->refptr is NULL\n");
				my_fprintf(stderr, "Artnum : %ld\n", arts[i].artnum);
				my_fprintf(stderr, "Subject: %s\n", arts[i].subject);
				my_fprintf(stderr, "From   : %s\n", arts[i].from);
			}
			assert(arts[i].refptr != 0);

			arts[i].refptr->article = i;
		}
	}

	/*
	 * Do the right thing according to the threading strategy
	 */
	switch (group->attribute->thread_arts) {
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

		default: /* not reached */
			break;
	}
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
		case SORT_ARTICLES_BY_SUBJ_ASCEND:
			return subj_comp;

		case SORT_ARTICLES_BY_FROM_DESCEND:
		case SORT_ARTICLES_BY_FROM_ASCEND:
			return from_comp;

		case SORT_ARTICLES_BY_DATE_DESCEND:
		case SORT_ARTICLES_BY_DATE_ASCEND:
			return date_comp;

		case SORT_ARTICLES_BY_SCORE_DESCEND:
		case SORT_ARTICLES_BY_SCORE_ASCEND:
			return score_comp;

		case SORT_ARTICLES_BY_LINES_DESCEND:
		case SORT_ARTICLES_BY_LINES_ASCEND:
			return lines_comp;

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
			qsort(base, (size_t) grpmenu.max, sizeof(long), score_comp_base);
			break;
	}
}


/*
 * This is called to get header info for articles not already found in the
 * overview files.
 * Code reads (max_lineno) lines of article to catch headers like Archive-name:
 * which are not normally included in XOVER or even the normal block of headers.
 * How this is supposed to be useful when 99% of the time we'll have overview
 * data I don't know...
 * TODO: move Archive-name: parsing to article body parsing, remove the
 * TODO: max_lineno nonsense and parse just the hdrs. Only parse if
 * TODO: currgrp->auto_save is set, otherwise it is redundant info
 */
static t_bool
parse_headers(
	FILE *fp,
	struct t_article *h)
{
	char art_from_addr[HEADER_LEN];
	char art_full_name[HEADER_LEN];
	char art_trunc_subj[HEADER_LEN];
	char *hdr, *ptr;
	unsigned int lineno = 0;
	unsigned int max_lineno = 25;
	t_bool got_from, got_lines, got_received;

	got_from = got_lines = got_received = FALSE;

	while ((ptr = tin_fgets(fp, TRUE)) != NULL) {
		/*
		 * Look for the end of information which tin wants to get.
		 * Applies when reading local spool and via NNTP.
		 */

		/*
		 * as Archive-name: is placed in the body it's safe to exit
		 * the loop if it was found.
		 */
		if (lineno++ > max_lineno || h->archive)
			break;

		unfold_header(ptr);
		switch (toupper((unsigned char) *ptr)) {
			case 'A':	/* Archive-name:  optional */
				/*
				 * Archive-name: {name}/{part|patch}{number}
				 * eg, acorn/faq/part01
				 */
				if ((hdr = parse_header(ptr + 1, "rchive-name", FALSE, FALSE))) {
					char *s;

					if ((s = strrchr(hdr, '/')) != NULL) {
						struct t_archive *archptr = my_malloc(sizeof(struct t_article));

						if (STRNCASECMPEQ(s + 1, "part", 4)) {
							archptr->partnum = my_strdup(s + 5);
							archptr->ispart = TRUE;
						} else if (STRNCASECMPEQ(s + 1, "patch", 5)) {
							archptr->partnum = my_strdup(s + 6);
							archptr->ispart = FALSE;
						} else {		/* part or patch must be present */
							free(archptr);
							continue;
						}
						strtok(archptr->partnum, "\n");
						*s = '\0';
						archptr->name = hash_str(hdr);
						h->archive = archptr;
					}
				}
				break;

			case 'D':	/* Date:  mandatory */
				if (!h->date) {
					if ((hdr = parse_header(ptr + 1, "ate", FALSE, FALSE)))
						h->date = parsedate(hdr, (struct _TIMEINFO *) 0);
				}
				break;

			case 'F':	/* From:  mandatory */
				if (!got_from) {
					if ((hdr = parse_header(ptr + 1, "rom", FALSE, FALSE))) {
						h->gnksa_code = parse_from(hdr, art_from_addr, art_full_name);
						h->from = hash_str(art_from_addr);
						if (*art_full_name)
							h->name = hash_str(eat_tab(rfc1522_decode(art_full_name)));
						got_from = TRUE;
					}
				}
				break;

			case 'L':	/* Lines:  optional */
				if (!got_lines) {
					if ((hdr = parse_header(ptr + 1, "ines", FALSE, FALSE))) {
						h->line_count = atoi(hdr);
						got_lines = TRUE;
					}
				}
				break;

			case 'M':	/* Message-ID:  mandatory */
				if (!h->msgid) {
					if ((hdr = parse_header(ptr + 1, "essage-ID", FALSE, FALSE)))
						h->msgid = my_strdup(hdr);
				}
				break;

			case 'R':	/* References:  optional */
				if (!h->refs) {
					if ((hdr = parse_header(ptr + 1, "eferences", FALSE, FALSE)))
						h->refs = my_strdup(hdr);
				}

				/* Received:  If found it's probably a mail article */
				if (!got_received) {
					if ((hdr = parse_header(ptr + 1, "eceived", FALSE, FALSE))) {
						max_lineno <<= 1;		/* double the max number of line to read for mails */
						got_received = TRUE;
					}
				}
				break;

			/*
			 * FIXME: Subject: truncation is a HACK and it's not multibyte safe
			 *        the core problem are probably fixed length buffers
			 *        (i.e. in rfc1522_encode() called from write_nov_file()
			 *         with the data read in here).
			 */
			case 'S':	/* Subject:  mandatory */
				if (!h->subject) {
					if ((hdr = parse_header(ptr + 1, "ubject", FALSE, FALSE))) {
						strncpy(art_trunc_subj, eat_re(eat_tab(rfc1522_decode(hdr)), FALSE), sizeof(art_trunc_subj) - 1);
						h->subject = hash_str(art_trunc_subj);
					}
				}
				break;

			case 'X':	/* Xref:  optional */
				if (!h->xref) {
					if ((hdr = parse_header(ptr + 1, "ref", FALSE, FALSE)))
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
	 */
	if (got_from && h->date && h->msgid) {
		if (!h->subject)
			h->subject = hash_str("<No subject>");

#ifdef DEBUG
		debug_print_header(h);
#endif /* DEBUG */
		return TRUE;
	}

	return FALSE;
}


#ifdef DEBUG
#	define handle_overview_fmt_error()	else oerror += 1<<count
#else
#	define handle_overview_fmt_error()
#endif /* DEBUG */
/*
 * Read in an overview index file. Fields are separated by TAB.
 * return the new value of 'top_art' or -1 if user quit partway.
 * 'expired' is set to the # of expired arts
 * If 'local' is set then always open local overview cache in
 * preference to using NNTP XOVER
 *
 * Format (mandatory as far as line count [RFC2980]):
 *	1. article number (ie. 183)                [mandatory]
 *	2. Subject: line  (ie. Which newsreader?)  [mandatory]
 *	3. From: line     (ie. iain@ecrc.de)       [mandatory]
 *	4. Date: line     (rfc822 format)          [mandatory]
 *	5. MessageID:     (ie. <123@ether.net>)    [mandatory]
 *	6. References:    (ie. <message-id> ....)  [optional]
 *	7. Byte count     (Skipped - not used)     [mandatory]
 *	8. Lines: line    (ie. 23)                 [mandatory]
 *	9. Xref: line     (ie. alt.test:389)       [optional]
 */
static int
read_nov_file(
	struct t_group *group,
	long min,
	long max,
	int *expired,
	t_bool local)
{
	FILE *fp;
	char *ptr;
	char *buf;
	char art_full_name[HEADER_LEN];
	char art_from_addr[HEADER_LEN];
	unsigned int count;
	long artnum;
	struct t_article *art;
#ifdef DEBUG
	unsigned int oerror = 0;
#endif /* DEBUG */

	*expired = 0;

	/*
	 * open the overview file (whether it be local or via nntp)
	 */
	if ((fp = open_xover_fp(group, "r", min, max, local)) == NULL)
		return top_art;

	if (group->xmax > max)
		group->xmax = max;

	while ((buf = tin_fgets(fp, FALSE)) != NULL) {
		if (need_resize) {
			handle_resize((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
		}
#ifdef DEBUG
		debug_nntp("read_nov_file", buf);
#endif /* DEBUG */

		/*
		 * read the article number, guaranteed to be the first field
		 */
		artnum = atol(buf);

		/*
		 * 1st line of local cached overview is group name
		 */
		if (artnum <= 0)
			continue;

		/*
		 * Check to make sure article in nov file has not expired in group
		 */
		if (artnum < group->xmin) {
			(*expired)++;
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
		art->artnum = last_read_article = artnum;

		/*
		 * Note: Fields after line count are not mandatory, use "LIST OVERVIEW.FMT"
		 *       to check for additions like we do with xref_supported
		 */
		if ((ptr = tin_strtok(buf, "\t")) == NULL)		/* Skip past artnum */
			continue;

		for (count = 1; (ptr = tin_strtok(NULL, "\t")) != NULL; count++) {
			switch (count) {
				case 1:		/* Subject */
					art->subject = hash_str(eat_re(eat_tab(rfc1522_decode(ptr)), FALSE));
					break;

				case 2:		/* From */
					art->gnksa_code = parse_from(ptr, art_from_addr, art_full_name);
					art->from = hash_str(art_from_addr);

					if (*art_full_name)
						art->name = hash_str(eat_tab(rfc1522_decode(art_full_name)));
					break;

				case 3:		/* Date */
					art->date = parsedate(ptr, (TIMEINFO *) 0);
#ifdef DEBUG
					if (art->date == (time_t) -1)
						oerror += 1<<count;
#endif /* DEBUG */
					break;

				case 4:		/* Message-ID */
					if (*ptr)
						art->msgid = my_strdup(ptr);
					handle_overview_fmt_error();
					break;

				case 5:		/* References */
					if (*ptr)
						art->refs = my_strdup(ptr);
					break;

				case 6:		/* Bytes */
#ifdef DEBUG
					if (!isdigit((unsigned char) *ptr))
						oerror += 1<<count;
#endif /* DEBUG */
					break;

				case 7:		/* Lines */
					if (isdigit((unsigned char) *ptr))
						art->line_count = atoi(ptr);
					handle_overview_fmt_error();
					break;

				case 8:		/* Xref: */
					if (!xref_supported)
						continue;
					/* TODO: crosscheck artnum against Xref:-line (if Xref:full) */
					if ((ptr = parse_header(ptr, "Xref", FALSE, FALSE)) != NULL)
						art->xref = my_strdup(ptr);
					handle_overview_fmt_error();
					break;
			}
		}

#ifdef DEBUG
		/* Complain if incorrect # of fields */
		if (count < (xref_supported ? 8 : 7) || oerror) {
			char errbuf[LEN];

			error_message(_("%d Bad overview record (%d fields) '%s'"), oerror, count, BlankIfNull(ptr)); /* TODO move to lang.c */
			snprintf(errbuf, sizeof(errbuf), "%d Bad overview record (%d fields)", oerror, count);
			debug_nntp("read_nov_file", errbuf);
		}
		debug_print_header(art);
		oerror = 0;
#endif /* DEBUG */

		/*
		 * RFC says Message-ID is mandatory in newsgroups (but not in
		 * mailgroups etc..) NB. a NULL Message-ID would abort if we ever do
		 * threading in mailgroups
		 */
		if (!art->msgid && group->type == GROUP_TYPE_NEWS)
			continue;

		/* we might loose accuracy here, but that shouldn't hurt */
		if (artnum % MODULO_COUNT_NUM == 0)
			show_progress(mesg, artnum - min, max - min);

		top_art++;
	}

	TIN_FCLOSE(fp);

	if (tin_errno)
		return -1;

	return top_art;
}


/*
 * Write an Nov/Xover index file. Fields are separated by '\t'.
 *
 * Format:
 *	1. article number (ie. 183)                [mandatory]
 *	2. Subject: line  (ie. Which newsreader?)  [mandatory]
 *	3. From: line     (ie. iain@ecrc.de)       [mandatory]
 *	4. Date: line     (rfc822 format)          [mandatory]
 *	5. MessageID:     (ie. <123@ether.net>)    [mandatory]
 *	6. References:    (ie. <message-id> ....)  [optional]
 *	7. Byte count     (Skipped - not used)     [mandatory]
 *	8. Lines: line    (ie. 23)                 [mandatory]
 *	9. Xref: line     (ie. alt.test:389)       [optional]
 *
 * TODO: as we don't use the original data, we currently can't store
 *       the data (from/subject) in the original charset (we don't store
 *       that info). this has the advantage that we can avoid raw 8bit data
 *       in our overviews, but the disadvantage that we might store the data
 *       with a wrong charset and thus lose information. a simmiliar problem
 *       exists with the data for the from:-line, we don't store it in the
 *       original format, whenever our from-parser (partially) fails we'll
 *       lose informations in our overviews (but those couldn't be handeled
 *       by tin anyway, so this is not a real problem).
 *       long-term solution: store the original data in the overview
 *       (tin has to handle raw 8bit data and other ugly stuff in the
 *       overviews anyway and thus we preserver as much info as possible)
 *       this would require some changes in read_nov_file() and
 *       parse_headers(): don't do the decoding/unfolding there, but in a
 *       second pass right after write_nov_file(), or two additional fields
 *       which hold the raw data for from/subject. the latter has the
 *       disadvantage that it costs (much) more memory.
 */
void
write_nov_file(
	struct t_group *group)
{
	FILE *fp;
	int i;
	struct t_article *article;

	/*
	 * Can't write or caching is off
	 */
	if (no_write || !tinrc.cache_overview_files)
		return;

	if ((fp = open_xover_fp(group, "w", 0L, 0L, FALSE)) == NULL)
		return;

	if (group->attribute->sort_art_type != SORT_ARTICLES_BY_NOTHING)
		SortBy(artnum_comp);

	/*
	 * Needed to preserve uniqueness in hashed private overview files
	 */
	fprintf(fp, "%s\n", group->name);

	for_each_art(i) {
		article = &arts[i];

		if (article->thread != ART_EXPIRED && article->artnum >= group->xmin) {
			char *p;
			char *q = NULL, *ref = NULL;

			/*
			 * TODO: instead of tinrc.mm_local_charset we'd better use UTF-8
			 *       here and in print_from() in the CHARSET_CONVERSION case.
			 *       note that this requires something like
			 *          buffer_to_network(article->subject, "UTF-8");
			 *       right bfore the rfc1522_encode() call.
			 *
			 *       if we would cache the original undecoded data, we could
			 *       ignore stuff like this.
			 */
			p = rfc1522_encode(article->subject, tinrc.mm_local_charset, FALSE);
			/* as the subject might now be folded we have to unfold it */
			unfold_header(p);

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
					q++;
				}
			}

			fprintf(fp, "%ld\t%s\t%s\t%s\t%s\t%s\t%d\t%d",
				article->artnum,
				tinrc.post_8bit_header ? article->subject : p,
				print_from(article),
				print_date(article->date),
				BlankIfNull(article->msgid),
				BlankIfNull(ref),
				0,	/* bytes */
				article->line_count);

			if (article->xref)
				fprintf(fp, "\tXref: %s", article->xref);

			fprintf(fp, "\n");
			free(p);
			if (q != ref) {
				free(ref);
				ref = q = NULL;
			}
		}
	}
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
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
	int i;
	struct stat sb;
	unsigned long hash;
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
			 * xover_cmd is not an issue here, any gripes and warnings
			 * about XOVER are handled in nntp_open()
			 */

			/*
			 * When reading via NNTP, system wide overviews are irrelevent, of
			 * course, and the private overview filename will be the same for
			 * both reading and writing.
			 *
			 * When working locally, we only use a private cache for reading
			 * if requested and when system wide overviews don't already exist.
			 * When writing then only private overviews can be used since
			 * updating system wide overviews is not safe wrt locking etc.
			 *
			 * See if local overview file $SPOOLDIR/<groupname>/.overview exists
			 */
#ifndef NNTP_ONLY
			if (!read_news_via_nntp) {
				make_base_group_path(novrootdir, group->name, buf);
				joinpath(nov_file, buf, novfilename);
				if (access(nov_file, R_OK) == 0) {
					if (mode == R_OK)
						return nov_file;		/* Use system wide overviews */
					else
						return NULL;			/* Don't write cache in this case */
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
				const char *from;
				char *to;
				int c;

				to = index_newsdir + strlen(index_newsdir);
				*(to++) = '-';
				for (from = nntp_server; (c = *from) != 0; ++from)
					*(to++) = tolower(c);
				*to = '\0';
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
				joinpath(local_newsgroups_file, index_newsdir, NEWSGROUPS_FILE);

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

	for (i = 1; ; i++) {
		char *ptr;

		snprintf(buf, sizeof(buf), "%lu.%d", hash, i);
		joinpath(nov_file, dir, buf);

		if ((fp = fopen(nov_file, "r")) == NULL)
			return nov_file;

		/*
		 * No group name header, so not a valid index file => overwrite it
		 */
		if (fgets(buf, (int) sizeof(buf), fp) == NULL) {
			fclose(fp);
			return nov_file;
		}
		fclose(fp);

		if ((ptr = strrchr(buf, '\n')) != NULL)
			*ptr = '\0';

		if (strcmp(buf, group->name) == 0)
			return nov_file;

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
	time_t beg_epoch;
	struct t_group *group;

	if (verbose)
		(void) time(&beg_epoch);

	/*
	 * loop through groups and update any required index files
	 */
	for (i = 0; i < selmenu.max; i++) {
		group = &active[my_group[i]];

		if (group->bogus || !group->subscribed)
			continue;

		if (!index_group(group))
			continue;

		k++;

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


static int
subj_comp(
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * return result of strcmp(reversed for descending)
	 */
	return (CURR_GROUP.attribute->sort_art_type == SORT_ARTICLES_BY_SUBJ_ASCEND
			? (retval = strcasecmp(s1->subject, s2->subject))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1
			: (retval = strcasecmp(s2->subject, s1->subject))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1);
}


static int
from_comp(
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * return result of strcmp(reversed for descending)
	 */
	return (CURR_GROUP.attribute->sort_art_type == SORT_ARTICLES_BY_FROM_ASCEND
			? (retval = strcasecmp(s1->from, s2->from))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1
			: (retval = strcasecmp(s2->from, s1->from))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1);
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
date_comp(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (CURR_GROUP.attribute->sort_art_type == SORT_ARTICLES_BY_DATE_ASCEND) {
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
	} else {
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
	}
	return 0;
}


/*
 * Same again, but for art[].score
 */
static int
score_comp(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (CURR_GROUP.attribute->sort_art_type == SORT_ARTICLES_BY_SCORE_ASCEND) {
		if (s1->score < s2->score)
			return -1;

		if (s1->score > s2->score)
			return 1;
	} else {
		if (s2->score < s1->score)
			return -1;

		if (s2->score > s1->score)
			return 1;
	}
	return s1->date - s2->date > 0 ? 1 : -1;
}


/*
 * Same again, but for art[].line_count
 */
static int
lines_comp(
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (CURR_GROUP.attribute->sort_art_type == SORT_ARTICLES_BY_LINES_ASCEND) {
		if (s1->line_count < s2->line_count)
			return -1;

		if (s1->line_count > s2->line_count)
			return 1;
	} else {
		if (s2->line_count < s1->line_count)
			return -1;

		if (s2->line_count > s1->line_count)
			return 1;
	}
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
	int a = get_score_of_thread(*(const long *)p1);
	int b = get_score_of_thread(*(const long *)p2);

	/* If scores are equal, compare using the article sort order.
	 * This determines the order in a group of equally scored threads.
	 */
	if (a == b) {
		const struct t_article *s1 = &arts[*(const long *)p1];
		const struct t_article *s2 = &arts[*(const long *)p2];
		t_compfunc comp_func = eval_sort_arts_func(CURR_GROUP.attribute->sort_art_type);

		if (comp_func)
			return (*comp_func)(s1, s2);
		return 0;
	}

	if (CURR_GROUP.attribute->sort_threads_type == SORT_THREADS_BY_SCORE_ASCEND)
		return a > b ? 1 : -1;
	return a < b ? 1 : -1;
}


void
set_article(
	struct t_article *art)
{
	art->subject = NULL;
	art->from = NULL;
	art->name = NULL;
	art->date = (time_t) 0;
	art->xref = NULL;
	art->msgid = NULL;
	art->refs = NULL;
	art->refptr = NULL;
	art->line_count = -1;
	art->archive = NULL;
	art->tagged = FALSE;
	art->thread = ART_EXPIRED;
	art->prev = ART_NORMAL;
	art->status = ART_UNREAD;
	art->killed = ART_NOTKILLED;
	art->zombie = FALSE;
	art->delete_it = FALSE;
	art->selected = FALSE;
	art->inrange = FALSE;
	art->matched = FALSE;
}


/*
 * Do a binary chop to see if 'art' (an article number) exists in arts[]
 * Return index into arts[] or -1
 */
static int
valid_artnum(
	long art)
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
		cur += (arts[cur].artnum < art) ? range : -range;
		if (prev == cur)
			break;

		if (cur >= top_art)
			cur = top_art - 1;

		range >>= 1;
	}
	return -1;
}


/*----------------------------- Overview handling -----------------------*/

static char *
print_date(
	time_t secs)
{
	static char date[25];
	struct tm *tm;
	static const char *const months_a[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};

	tm = gmtime(&secs);
	snprintf(date, sizeof(date), "%02d %s %04d %02d:%02d:%02d GMT",
			tm->tm_mday,
			months_a[tm->tm_mon],
			tm->tm_year + 1900,
			tm->tm_hour, tm->tm_min, tm->tm_sec);

	return date;
}


static char *
print_from(
	struct t_article *article)
{
	static char from[PATH_LEN];
	char *p;

	*from = '\0';

	if (article->name != NULL) {
		p = rfc1522_encode(article->name, tinrc.mm_local_charset, FALSE);
		unfold_header(p);
		if (strpbrk(article->name, "\".:;<>@[]()\\") != NULL && article->name[0] != '"' && article->name[strlen(article->name)] != '"')
			snprintf(from, sizeof(from), "\"%s\" <%s>", tinrc.post_8bit_header ? article->name : p, article->from);
		else
			snprintf(from, sizeof(from), "%s <%s>", tinrc.post_8bit_header ? article->name : p, article->from);

		free(p);
	}
	else
		STRCPY(from, article->from);

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
	long min,
	long max,
	t_bool local)
{
#ifdef NNTP_ABLE
	if (!local && xover_cmd && *mode == 'r' && group->type == GROUP_TYPE_NEWS) {
		char line[NNTP_STRLEN];

		snprintf(line, sizeof(line), "%s %ld-%ld", xover_cmd, min, max);
		return (nntp_command(line, OK_XOVER, NULL, 0));
	} else
#endif /* NNTP_ABLE */
	{
		FILE *fp;
		char *nov_file = find_nov_file(group, (*mode == 'r') ? R_OK : W_OK);

		if (nov_file != NULL) {
			if ((fp = fopen(nov_file, mode)) != NULL)
				return fp;

			if (*mode != 'r')
				error_message(_(txt_cannot_open), nov_file);
		}
		return NULL;
	}
}
