/*
 *  Project   : tin - a Usenet reader
 *  Module    : art.c
 *  Author    : I.Lea & R.Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1995-04-19
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */

#ifdef PROFILE
#	ifndef STPWATCH_H
#		include "stpwatch.h"
#	endif /* !STPWATCH_H */
#endif /* PROFILE */

#define SortBy(func) qsort (arts, (size_t)top_art, sizeof (struct t_article), func);

static long last_read_article;
static t_bool overview_index_filename = FALSE;
int top_art = 0;				/* # of articles in arts[] */

/*
 * Local prototypes
 */
static char *print_date (time_t secs);
static char *print_from (struct t_article *article);
static int artnum_comp (t_comptype p1, t_comptype p2);
static int date_comp (t_comptype p1, t_comptype p2);
static int from_comp (t_comptype p1, t_comptype p2);
static int read_nov_file (struct t_group *group, long min, long max, int *expired);
static int read_group (struct t_group *group, char *group_path, int *pcount);
static int score_comp (t_comptype p1, t_comptype p2);
static int subj_comp (t_comptype p1, t_comptype p2);
static int valid_artnum (long art);
static long find_first_unread (struct t_group *group);
static t_bool parse_headers (FILE *fp, struct t_article *h);
static void print_expired_arts (int num_expired);
static void thread_by_subject (void);


/*
 * Display a suitable 'entering group' message if screen needs redrawing
 * Allow for the non-printing %s, and the %-age counter
 */
void
show_art_msg(
	char *group)
{
/* what if cCOLS < (strlen)+18 ? */
	wait_message(0, _(txt_group), cCOLS - strlen(_(txt_group)) + 2 - 3, group);
}

/*
 *  Construct the pointers to the base article in each thread.
 *  If we are showing only unread, then point to the first unread. I have
 *  no idea why this should be so, it causes problems elsewhere [which_response]
 *  .inthread is set on each article that is after the first article in the
 *  thread.  Articles which have been expired have their .thread set to
 *  ART_EXPIRED
 */
void
find_base (
	struct t_group *group)
{
	register int i;
	register int j;

	grpmenu.max = 0;

#ifdef DEBUG
	debug_print_arts ();
#endif /* DEBUG */

	if (group->attribute && group->attribute->show_only_unread) {
		for (i = 0; i < top_art; i++) {
			if (IGNORE_ART(i) || arts[i].inthread)
				continue;

			if (grpmenu.max >= max_art)
				expand_art ();

			if (arts[i].status == ART_UNREAD)
				base[grpmenu.max++] = i;
			else {
				for (j = i; j >= 0; j = arts[j].thread) {
					if (arts[j].status == ART_UNREAD) {
						base[grpmenu.max++] = i;
						break;
					}
				}
			}
		}
	} else {
		for (i = 0; i < top_art; i++) {
			if (IGNORE_ART(i) || arts[i].inthread)
				continue;

			if (grpmenu.max >= max_art)
				expand_art ();

			base[grpmenu.max++] = i;
		}
	}
}


/*
 *  Main group indexing routine.
 *
 *  Will read any existing index, create or incrementally update
 *  the index by looking at the articles in the spool directory,
 *  and attempt to write a new index if necessary.
 *
 *  Returns FALSE if the user aborted the indexing, otherwise TRUE
 */
t_bool
index_group (
	struct t_group *group)
{
	char group_path[PATH_LEN];
	int count;
	int expired;
	int modified;
	long min;
	long max;
	register int i;
	t_bool filtered;

	if (group == (struct t_group *) 0)
		return TRUE;

	if (!batch_mode)
		show_art_msg (group->name);

	make_group_path (group->name, group_path);
	signal_context = cArt;			/* Set this once glob_group is valid */

	hash_reclaim ();
	free_art_array ();
	free_msgids ();

	/*
	 *  Load articles within min..max from xover index file if it exists
	 *  and then create base[] article numbers from loaded articles.
	 *  If nov file does not exist then create base[] with setup_base().
	 */
#ifdef PROFILE
	BegStopWatch("setup_base");
#endif /* PROFILE */

	if (setup_hard_base (group, group_path) < 0)
		return FALSE;

#ifdef PROFILE
	EndStopWatch();
	PrintStopWatch();
#endif /* PROFILE */

#ifdef DEBUG_NEWSRC
	debug_print_comment ("Before read_nov_file");
	debug_print_bitmap (group, NULL);
#endif /* DEBUG_NEWSRC */

	min = grpmenu.max ? base[0] : group->xmin;
	max = grpmenu.max ? base[grpmenu.max-1] : min - 1;

	if (tinrc.use_getart_limit) {
		if (tinrc.getart_limit > 0) {
			if (grpmenu.max && (grpmenu.max > tinrc.getart_limit))
				min = base[grpmenu.max-tinrc.getart_limit];
		} else if (tinrc.getart_limit < 0) {
			long first_unread = find_first_unread(group);
			if (min - first_unread < tinrc.getart_limit)
				min = first_unread + tinrc.getart_limit;
		}
	}

	/*
	 * Quit now if no articles
	 */
	if (max < 0)
		return FALSE;

	/*
	 * Read in the existing index via XOVER or the index file
	 */
	if (read_nov_file (group, min, max, &expired) == -1)
		return FALSE;	/* user aborted indexing */

#if 0	/* IMHO this is a bit to verbose (urs) */
	/*
	 * Prints 'P' for each expired article if verbose
	 */
	if (expired)
		print_expired_arts (expired);
#endif /* 0 */

	/*
	 * Add any articles to arts[] that are new or were killed
	 */
	if ((modified = read_group (group, group_path, &count)) == -1)
		return FALSE;	/* user aborted indexing */

	/*
	 * Do this before calling art_mark_read if you want
	 * the unread count to be correct.
	 */
#ifdef DEBUG_NEWSRC
	debug_print_comment ("Before parse_unread_arts()");
	debug_print_bitmap (group, NULL);
#endif /* DEBUG_NEWSRC */
	parse_unread_arts (group);
#ifdef DEBUG_NEWSRC
	debug_print_comment ("After parse_unread_arts()");
	debug_print_bitmap (group, NULL);
#endif /* DEBUG_NEWSRC */

	/*
	 * Stat all articles to see if any have expired
	 */
	for (i = 0; i < top_art; i++) {
		if (arts[i].thread == ART_EXPIRED) {
			expired = 1;
#ifdef DEBUG_NEWSRC
			debug_print_comment ("art.c: index_group() purging...");
#endif /* DEBUG_NEWSRC */
			art_mark_read (group, &arts[i]);
			print_expired_arts (expired);
		}
	}

	if (expired || modified || tinrc.cache_overview_files)
		write_nov_file (group);

	/*
	 * Create the reference tree. The msgid and ref ptrs will
	 * be free()d now that the NovFile has been written.
	 */
	build_references (group);

	/*
	 * Needs access to the reference tree
	 */
	filtered = filter_articles (group);

	if ((expired || count) && cmd_line && verbose) {
		my_fputc ('\n', stdout);
		my_flush ();
	}

#ifdef PROFILE
	BegStopWatch("make_thread");
#endif /* PROFILE */

	make_threads (group, FALSE);

#ifdef PROFILE
	EndStopWatch();
	PrintStopWatch();
#endif /* PROFILE */

	if ((modified || filtered) && !batch_mode)
		clear_message ();

	return TRUE;
}


/*
 * Returns number of first unread article
 */
static long
find_first_unread (
	struct t_group *group)
{
	unsigned char *p;
	unsigned char *end = group->newsrc.xbitmap;
	long first = group->newsrc.xmin; /* initial value */

	if ((p = group->newsrc.xbitmap)) {
		end += group->newsrc.xbitlen / 8;
		for (; *p == '\0' && p < end; p++, first += 8);
	}
	return first;
}


/*
 * Index a group.  Assumes any existing NOV index has already been loaded.
 * Return values are:
 *    1   loaded index and modified it
 *    0   loaded index but not modified
 *   -1   user aborted indexing operation
 */
static int
read_group (
	struct t_group *group,
	char *group_path,
	int *pcount)
{
	FILE *fp;
	char buf[PATH_LEN];
	int count = 0;
	int respnum, total = 0;
	int modified = 0;
	long art;
	register int i;
	t_bool res;
	static char dir[PATH_LEN] = "";

	/*
	 * change to groups spooldir to optimize fopen()'s on local articles
	 */
	if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS) {
		get_cwd (dir);
		joinpath (buf, group->spooldir, group_path);
		my_chdir (buf);
	}

	/*
	 *  Count num of arts to index so the user has an idea of index time
	 */
	for (i = 0; i < grpmenu.max; i++) {
		if (base[i] <= last_read_article || valid_artnum (base[i]) >= 0)
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
		 *  Do we already have this article in our index?  Change
		 *  arts[].thread from ART_EXPIRED to ART_NORMAL and skip
		 *  reading the header.
		 */
		if ((respnum = valid_artnum (art)) >= 0 || art <= last_read_article) {
			if (respnum >= 0)
				arts[respnum].thread = ART_NORMAL;

			continue;
		}

		/*
		 * Try and open the article
		 */
		if ((fp = open_art_header (art)) == (FILE *) 0)
			continue;

		/*
		 * we've modified the index so it will need to be re-written
		 */
		modified = 1;

		/*
		 *  Add article to arts[]
		 */
		if (top_art >= max_art)
			expand_art();

		set_article (&arts[top_art]);
		arts[top_art].artnum = art;
		arts[top_art].thread = ART_NORMAL;

		res = parse_headers (fp, &arts[top_art]);

		TIN_FCLOSE(fp);
		if (tin_errno) {
			if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS)
				my_chdir (dir);

			return -1;
		}

		if (!res) {
			sprintf (buf, "FAILED parse_header(%ld)", art);
#ifdef DEBUG
			debug_nntp ("read_group", buf);
#endif /* DEBUG */
			continue;
		}

		last_read_article = arts[top_art].artnum;	/* used if arts are killed */
		top_art++;

		if (++count % MODULO_COUNT_NUM == 0)
			show_progress (mesg, count, total);

#if 0 /* again too verbose */
		if (batch_mode && verbose) {
			my_fputc ('.', stdout);
			my_flush();
		}
#endif /* 0 */

	}

	/*
	 * Update number of article we 'read'
	 */
	*pcount = count;

	/*
	 * if !nntp change to previous dir before indexing started
	 */
	if (!read_news_via_nntp || group->type != GROUP_TYPE_NEWS)
		my_chdir (dir);

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
 */
static void
thread_by_subject (
	void)
{
	int i, j;
	struct t_hashnode *h;

	for (i = 0; i < top_art; i++) {

		if (arts[i].thread != ART_NORMAL || IGNORE_ART(i))
			continue;

		/*
		 * Get the contents of the magic marker in the hashnode
		 */
		h = (struct t_hashnode *)(arts[i].subject - sizeof(int) - sizeof(void *)); /* FIXME: cast increases required alignment of target type */

		j = h->aptr;

		if (j != -1 && j < i) {
			if (!arts[i].inthread && ((arts[i].subject == arts[j].subject) || ((arts[i].part || arts[i].patch) && arts[i].archive == arts[j].archive))) {
				arts[j].thread = i;
				arts[i].inthread = TRUE;
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
	for (i = 0; i < top_art; i++) {
		fprintf(stderr, "%3d %3d %3d %3d : %3d %3d : %.50s %s\n", i,
			(arts[i].refptr->parent)  ? arts[i].refptr->parent->article : -2,
			(arts[i].refptr->sibling) ? arts[i].refptr->sibling->article : -2,
			(arts[i].refptr->child)   ? arts[i].refptr->child->article : -2,
			arts[i].inthread, arts[i].thread, arts[i].refptr->txt, arts[i].subject);
	}
#endif /* 0 */
}


/*
 *  Go through the articles in arts[] and create threads. There are
 *  4 strategies currently defined :
 *
 *	THREAD_NONE		No threading
 *	THREAD_SUBJ		Threads are created using like Subject lines
 *	THREAD_REFS		Threads are created using the References headers
 *	THREAD_BOTH		Threads created using References and then Subject
 *
 *  Apart from THREAD_NONE, .thread and .inthread are used, the
 *  first article in a thread should have .inthread set to FALSE, the
 *  rest TRUE.  Only do unexprired articles we haven't visited yet
 *  (arts[].thread == -1 ART_NORMAL).
 *
 *  The rethread parameter is a misnomer. Its only effect (if set) is
 *  to delete all threading information, not to rethread
 *
 */
void
make_threads (
	struct t_group *group,
	t_bool rethread)
{
	int i;

	if (!cmd_line && !batch_mode)
		info_message (((group->attribute && group->attribute->thread_arts == THREAD_NONE) ? _(txt_unthreading_arts) : _(txt_threading_arts)));

#ifdef DEBUG
	if (debug == 2)
		error_message ("rethread=[%d]  thread_arts=[%d]  attr_thread_arts=[%d]",
				rethread, tinrc.thread_articles, group->attribute->thread_arts);
#endif /* DEBUG */

	/*
	 * Sort all the articles using the preferred method
	 * When find_base() is called, the bases are created ordered
	 * on arts[] and so the base messages under all threading systems
	 * will be sorted in this way.
	 */
	if (group->attribute)
		sort_arts (group->attribute->sort_art_type);

	/*
	 * Reset all the ptrs to articles following the above sort
	 */
	clear_art_ptrs();

	/*
	 *  The threading pointers need to be reset if re-threading
	 *	If using ref threading, revector the links back to the articles
	 */
	if (rethread || (group->attribute && group->attribute->thread_arts)) {

		for (i = 0; i < top_art; i++) {

			if (arts[i].thread != ART_EXPIRED)
				arts[i].thread = ART_NORMAL;

			arts[i].inthread = FALSE;

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
	if (group->attribute)
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

		default: /* not reached */
			break;
	}

	find_base (group);
}


void
sort_arts (
	unsigned int sort_art_type)
{
	switch (sort_art_type) {
		case SORT_BY_NOTHING:		/* don't sort at all */
			SortBy(artnum_comp);
			break;
		case SORT_BY_SUBJ_DESCEND:
		case SORT_BY_SUBJ_ASCEND:
			SortBy(subj_comp);
			break;
		case SORT_BY_FROM_DESCEND:
		case SORT_BY_FROM_ASCEND:
			SortBy(from_comp);
			break;
		case SORT_BY_DATE_DESCEND:
		case SORT_BY_DATE_ASCEND:
			SortBy(date_comp);
			break;
		case SORT_BY_SCORE_DESCEND:
		case SORT_BY_SCORE_ASCEND:
			SortBy(score_comp);
			break;
		default:
			break;
	}
}


/*
 * TODO
 * Seems to be only called when reading local spool and no overview files exist
 * Code reads (max_lineno) lines of article, presumably to catch headers like
 * Archive-name: which are not normally included in XOVER or even the normal
 * block of headers. How this is supposed to be useful when 99% of the time we'll
 * have overview data I don't know...
 */
static t_bool
parse_headers (
	FILE *fp,
	struct t_article *h)
{
	char art_from_addr[HEADER_LEN];
	char art_full_name[HEADER_LEN];
	char *hdr, *ptr, *s;
	int lineno = 0;
	int max_lineno = 25;
	t_bool got_from, got_lines, got_received;

	got_from = got_lines = got_received = FALSE;

	while ((ptr = tin_fgets(fp, TRUE)) != NULL) {
		/*
		 * Look for the end of information which tin wants to get.
		 * Applies when reading local spool and via NNTP.
		 */
		if (lineno++ > max_lineno || h->archive)
			break;

		switch (toupper((unsigned char)*ptr)) {
			case 'A':	/* Archive-name:  optional */
				if ((hdr = parse_header (ptr+1, "rchive-name", FALSE))) {
					/* TODO - what if header of form news/group/name/part01 ? */
					if ((s = strrchr (hdr, '/')) != NULL) {
						if (STRNCASECMPEQ(s+1, "part", 4)) {
							h->part = my_strdup (s+5);
							strtok(h->part, "\n");
						} else if (STRNCASECMPEQ(s+1, "patch", 5)) {
							h->patch = my_strdup (s+6);
							strtok(h->patch, "\n");
						} else
							continue;

						strtok(hdr, "/");
						h->archive = hash_str (hdr);
					}
				}
				break;
			case 'D':	/* Date:  mandatory */
				if (!h->date) {
					if ((hdr = parse_header (ptr+1, "ate", FALSE)))
						h->date = parsedate (hdr, (struct _TIMEINFO *) 0);
				}
				break;
			case 'F':	/* From:  mandatory */
			case 'T':	/* To:    mandatory (mailbox) */
				if (!got_from) {
					if ((hdr = parse_header (ptr+1, "rom", FALSE)) ||
									(hdr = parse_header (ptr+1, "o", FALSE))) {
						h->gnksa_code = parse_from (hdr, art_from_addr, art_full_name);
						h->from = hash_str (art_from_addr);
						if (*art_full_name)
							h->name = hash_str (eat_tab(rfc1522_decode(art_full_name)));
						got_from = TRUE;
					}
				}
				break;
			case 'L':	/* Lines:  optional */
				if (!got_lines) {
					if ((hdr = parse_header (ptr+1, "ines", FALSE))) {
						h->line_count = atoi (hdr);
						got_lines = TRUE;
					}
				}
				break;
			case 'M':	/* Message-ID:  mandatory */
				if (!h->msgid) {
					if ((hdr = parse_header (ptr+1, "essage-ID", FALSE)))
						h->msgid = my_strdup (hdr);
				}
				break;
			case 'R':	/* References:  optional */
				if (!h->refs) {
					if ((hdr = parse_header (ptr+1, "eferences", FALSE)))
						h->refs = my_strdup (hdr);
				}

				/* Received:  If found it's probably a mail article */
				if (!got_received) {
					if ((hdr = parse_header (ptr+1, "eceived", FALSE))) {
						max_lineno = 50;		/* Get extra lines for some reason */
						got_received = TRUE;
					}
				}
				break;
			case 'S':	/* Subject:  mandatory */
				if (!h->subject) {
					if ((hdr = parse_header (ptr+1, "ubject", FALSE))) {
						s = eat_re (eat_tab(rfc1522_decode(hdr)), FALSE);
						h->subject = hash_str (s);
					}
				}
				break;
			case 'X':	/* Xref:  optional */
				if (!h->xref) {
					if ((hdr = parse_header (ptr+1, "ref", FALSE)))
						h->xref = my_strdup (hdr);
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
			h->subject = hash_str ("<No subject>");

#ifdef DEBUG
		debug_print_header (h);
#endif /* DEBUG */
		return TRUE;
	}

	return FALSE;
}


/*
 *  Read in an Nov/Xover index file. Fields are separated by TAB.
 *  return the new value of 'top_art' or -1 if user quit partway.
 *
 *  Format:
 *    1.  article number (ie. 183)                [mandatory]
 *    2.  Subject: line  (ie. Which newsreader?)  [mandatory]
 *    3.  From: line     (ie. iain@ecrc.de)       [mandatory]
 *    4.  Date: line     (rfc822 format)          [mandatory]
 *    5.  MessageID:     (ie. <123@ether.net>)    [mandatory]
 *    6.  References:    (ie. <message-id> ....)  [mandatory]
 *    7.  Byte count     (Skipped - not used)     [mandatory]
 *    8.  Lines: line    (ie. 23)                 [optional]
 *    9.  Xref: line     (ie. alt.test:389)       [optional]
 *   10.  Archive-name:  (ie. widget/part01)      [optional]
 */
static int
read_nov_file (
	struct t_group *group,
	long min,
	long max,
	int *expired)
{
	FILE *fp;
	char *p, *q;
	char *buf;
	char art_full_name[HEADER_LEN];
	char art_from_addr[HEADER_LEN];
	long artnum;

	top_art = 0;
	last_read_article = 0L;
	*expired = 0;

	/*
	 *  Call ourself recursively to read the cached overview file, if we are
	 *  supposed to be doing NNTP caching and we aren't already the recursive
	 *  instance.  (Turn off read_news_via_nntp while we're recursing so we
	 *  will know we're recursing while we're doing it.)  If there aren't
	 *  any new articles, just return, without going on to read the NNTP
	 *  overview file.  If we're going to read from NNTP, adjust min to the
	 *  next article past last_read_article; there's no reason to read them
	 *  from NNTP if they're cached locally.
	 */
	if (tinrc.cache_overview_files && read_news_via_nntp && xover_supported && group->type == GROUP_TYPE_NEWS) {
		read_news_via_nntp = FALSE;
		read_nov_file (group, min, max, expired);
		read_news_via_nntp = TRUE;
		if (last_read_article >= max)
			return top_art;
		if (last_read_article >= min)
			min = last_read_article + 1;
	}
	/*
	 * open the overview file (whether it be local or via nntp)
	 */
	if ((fp = open_xover_fp (group, "r", min, max)) == (FILE *) 0)
		return top_art;

	if (group->xmax > max)
		group->xmax = max;

	while ((buf = tin_fgets (fp, FALSE)) != (char *) 0) {
		if (need_resize) {
			handle_resize ((need_resize == cRedraw) ? TRUE : FALSE);
			need_resize = cNo;
		}

#ifdef DEBUG
		debug_nntp ("read_nov_file", buf);
#endif /* DEBUG */

		if (top_art >= max_art)
			expand_art ();

		p = buf;

		/*
		 * read the article number
		 */
		artnum = atol (p);

		/* catches case of 1st line being groupname */
		if (artnum <= 0)
			continue;

		/*
		 * Check to make sure article in nov file has not expired in group
		 */
#if 0
	my_printf ("artnum=[%ld] xmin=[%ld] xmax=[%ld]\n", artnum, group->xmin, group->xmax);
	my_flush();
	(void) sleep(1);
#endif /* 0 */

		if (artnum < group->xmin) {
			(*expired)++;
			continue;
		}
		set_article (&arts[top_art]);
		arts[top_art].artnum = last_read_article = artnum;

		if ((q = strchr (p, '\t')) == (char *) 0) {
#ifdef DEBUG
			error_message ("Bad overview record (Artnum) '%s'", buf);
			debug_nntp ("read_nov_file", "Bad overview record (Artnum)");
#endif /* DEBUG */
			continue;
		} else
			p = q + 1;

		/*
		 * READ subject
		 */
		if ((q = strchr (p, '\t')) == (char *) 0) {
#ifdef DEBUG
			error_message ("Bad overview record (Subject) [%s]", p);
			debug_nntp ("read_nov_file", "Bad overview record (Subject)");
#endif /* DEBUG */
			continue;
		} else
			*q = '\0';

		arts[top_art].subject = hash_str (eat_re(eat_tab(rfc1522_decode(p)), FALSE));
		p = q + 1;

		/*
		 * READ author
		 */
		if ((q = strchr (p, '\t')) == (char *) 0) {
#ifdef DEBUG
			error_message ("Bad overview record (From) [%s]", p);
			debug_nntp ("read_nov_file", "Bad overview record (From)");
#endif /* DEBUG */
			continue;
		} else
			*q = '\0';

		arts[top_art].gnksa_code = parse_from (p, art_from_addr, art_full_name);
		arts[top_art].from = hash_str (art_from_addr);

		if (*art_full_name)
			arts[top_art].name = hash_str (eat_tab(rfc1522_decode(art_full_name)));

		p = q + 1;
		/*
		 * READ article date
		 */
		if ((q = strchr (p, '\t')) == (char *) 0) {
#ifdef DEBUG
			error_message ("Bad overview record (Date) [%s]", p);
			debug_nntp ("read_nov_file", "Bad overview record (Date)");
#endif /* DEBUG */
			continue;
		} else
			*q = '\0';

		arts[top_art].date = parsedate (p, (TIMEINFO*)0);
		p = q + 1;

		/*
		 * READ article message id
		 */
		q = strchr (p, '\t');
		if (q == (char *) 0 || p == q) {	/* Empty msgid's */
#ifdef DEBUG
			error_message ("Bad overview record (Msg-id) [%s]", p);
			debug_nntp ("read_nov_file", "Bad overview record (Msg-id)");
#endif /* DEBUG */
			continue;
		} else
			*q = '\0';

		/* TODO is no mesg-id allowed in rfc ? */
		arts[top_art].msgid = ((*p) ? (my_strdup (p)) : ((char *) '\0'));

		p = q + 1;

		/*
		 * READ article references
		 */
		if ((q = strchr (p, '\t')) == (char *) 0) {
#ifdef DEBUG
			error_message ("Bad overview record (References) [%s]", p);
			debug_nntp ("read_nov_file", "Bad overview record (References)");
#endif /* DEBUG */
			continue;
		} else
			*q = '\0';

		arts[top_art].refs = ((*p) ? (my_strdup (p)) : ((char *) '\0'));

		p = q + 1;

		/*
		 * SKIP article bytes
		 */
		if ((q = strchr (p, '\t')) == (char *) 0) {
#ifdef DEBUG
			error_message ("Bad overview record (Bytes) [%s]", p);
			debug_nntp ("read_nov_file", "Bad overview record (Bytes)");
#endif /* DEBUG */
			continue;
		} else
			*q = '\0';

		p = (q == (char *) 0 ? (char *) 0 : q + 1);

		/*
		 * READ article lines
		 */
		if (p != (char *) 0) {
			if ((q = strchr (p, '\t')) != (char *) 0)
				*q = '\0';

			if (isdigit((unsigned char)*p))
				arts[top_art].line_count = atoi (p);

			p = (q == (char *) 0 ? (char *) 0 : q + 1);
		}

		/*
		 * READ article xrefs
		 */
		if (p != (char *) 0 && xref_supported) {
			if ((q = strstr (p, "Xref: ")) == (char *) 0)
				q = strstr (p, "xref: ");

			if (q != (char *) 0) {
				p = q + 6;
				q = p;
				while (*q && *q != '\t')
					q++;

				*q = '\0';
				q = strrchr (p, '\n');
				if (q != (char *) 0)
					*q = '\0';

				q = p;
				while (*q && *q == ' ')
					q++;

				arts[top_art].xref = my_strdup (q);
			}
		}

		/*
		 * end of overview line processing
		 */
#ifdef DEBUG
		debug_print_header (&arts[top_art]);
#endif /* DEBUG */

		/* we might loose accuracy here, but that shouldn't hurt */
		if (artnum % MODULO_COUNT_NUM == 0)
			show_progress(mesg, artnum - min, max - min);

		top_art++;
	}

	TIN_FCLOSE (fp);

	if (tin_errno)
		return(-1);

	return top_art;
}


/*
 *  Write an Nov/Xover index file. Fields are separated by TAB.
 *
 *  Format:
 *    1.  article number (ie. 183)                [mandatory]
 *    2.  Subject: line  (ie. Which newsreader?)  [mandatory]
 *    3.  From: line     (ie. iain@ecrc.de)       [mandatory]
 *    4.  Date: line     (rfc822 format)          [mandatory]
 *    5.  MessageID:     (ie. <123@ether.net>)    [optional]
 *    6.  References:    (ie. <message-id> ....)  [mandatory]
 *    7.  Byte count     (Skipped - not used)     [mandatory]
 *    8.  Lines: line    (ie. 23)                 [mandatory]
 *    9.  Xref: line     (ie. alt.test:389)       [optional]
 */
void
write_nov_file (
	struct t_group *group)
{
	FILE *fp;
	char *nov_file;
	char tmp[PATH_LEN];
	int i;
	struct t_article *article;

	if (no_write)
		return;

	/*
	 * Don't write local index if we have XOVER, unless the user has
	 * asked for caching.
	 */
	if (xover_supported && !tinrc.cache_overview_files)
		return;

	/*
	 * setup the overview file (local only)
	 */

	/*
	 * don't write an overview file if R_OK returns a different name
	 * than W_OK, since we won't read it anyway.
	 */

	STRCPY(tmp, ((((nov_file = find_nov_file (group, R_OK)) != 0)) ? nov_file : ""));
	nov_file = find_nov_file (group, W_OK);

	if (strcmp(tmp, nov_file) != 0)
		return;

#ifdef DEBUG
	if (debug)
		error_message ("WRITE file=[%s]", nov_file);
#endif /* DEBUG */

	fp = open_xover_fp (group, "w", 0L, 0L);

	if (fp == (FILE *) 0)
		error_message (_(txt_cannot_write_index), nov_file);
	else {
		if (group->attribute && group->attribute->sort_art_type != SORT_BY_NOTHING)
			SortBy(artnum_comp);

		if (!overview_index_filename)
			fprintf (fp, "%s\n", group->name);

		for (i = 0; i < top_art; i++) {
			article = &arts[i];

			if (article->thread != ART_EXPIRED && article->artnum >= group->xmin) {
				fprintf (fp, "%ld\t%s\t%s\t%s\t%s\t%s\t%d\t%d",
					article->artnum,
					article->subject,
					print_from (article),
					print_date (article->date),
					(article->msgid ? article->msgid : ""),
					(article->refs ? article->refs : ""),
					0,	/* bytes */
					article->line_count);

				if (article->xref)
					fprintf (fp, "\tXref: %s", article->xref);

				fprintf (fp, "\n");
			}
		}

		fclose (fp);
		chmod (nov_file, (mode_t)(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));

	}
}


/*
 *  A complex little function to determine where to read the index file
 *  from and where to write it.
 *
 *  GROUP_TYPE_MAIL index files are read/written in ~/.tin/.mail
 *
 *  GROUP_TYPE_SAVE index files are read/written in ~/.tin/.save
 *
 *  GROUP_TYPE_NEWS index files are a little bit more complex :(
 *  READ:
 *    if  reading via NNTP
 *       path = TMPDIR/num.idx
 *       hash = FALSE
 *    else if  SPOOLDIR/group/name/.overview exists
 *       path = SPOOLDIR/group/name/.overview
 *    else if  SPOOLDIR/.news exists
 *       path = SPOOLDIR/.news
 *       hash = TRUE
 *    else
 *       path = ~/.tin/.news
 *       hash = TRUE
 *
 *  WRITE:
 *    if  SPOOLDIR/group/name writable
 *       path = SPOOLDIR/group/name/.overview
 *       hash = FALSE
 *    else if  SPOOLDIR/.news exists AND writable
 *       path = SPOOLDIR/.news
 *       hash = TRUE
 *    else
 *       path = ~/.tin/.news
 *       hash = TRUE
 *
 *  If hash = TRUE the index filename will be in format number.number.
 *  Hashing the groupname gets a number. See if that #.1 file exists;
 *  if so, read first line. Is this the group we want? If no, try #.2.
 *  Repeat until no such file or we find an existing file that matches
 *  our group. Return pointer to path or NULL if not found.
 */
char *
find_nov_file (
	struct t_group *group,
	int mode)
{
	char *ptr;
	const char *dir;
	char buf[PATH_LEN];
	FILE *fp;
	int i;
	static char nov_file[PATH_LEN];
	t_bool hash_filename;
	unsigned long hash;

	if (group == (struct t_group *) 0)
		return (char *) 0;

	overview_index_filename = FALSE;	/* Write groupname in nov file ? */

	hash_filename = FALSE;
	dir = "";

	switch (group->type) {
		case GROUP_TYPE_MAIL:
			dir = index_maildir;
			hash_filename = TRUE;
			break;
		case GROUP_TYPE_SAVE:
			dir = index_savedir;
			hash_filename = TRUE;
			break;
		case GROUP_TYPE_NEWS:
			if (read_news_via_nntp && xover_supported && ! tinrc.cache_overview_files)
				snprintf (nov_file, sizeof(nov_file)-1, "%s%d.idx", TMPDIR, (int) process_id);
			else {
				make_base_group_path (novrootdir, group->name, buf);
				snprintf (nov_file, sizeof(nov_file)-1, "%s/%s", buf, novfilename);
				if (mode == R_OK || mode == W_OK) {
					if (!access (nov_file, mode))
						overview_index_filename = TRUE;
				}
				if (!overview_index_filename) {
					dir = index_newsdir;
					hash_filename = TRUE;
				}
			}
			break;
		default: /* not reached */
			break;
	}

	if (hash_filename) {
		hash = hash_groupname (group->name);

		for (i = 1; ; i++) {

			snprintf (nov_file, sizeof(nov_file)-1, "%s/%lu.%d", dir, hash, i);

			if ((fp = fopen (nov_file, "r")) == (FILE *) 0)
				return nov_file;

			/*
			 * Don't follow, why should a zero length index file
			 * cause the write to fail ?
			 */
			if (fgets (buf, (int) sizeof (buf), fp) == (char *) 0) {
				fclose (fp);
				return nov_file;
			}
			fclose (fp);

			ptr = strrchr (buf, '\n');
			if (ptr != (char *) 0)
				*ptr = '\0';

			if (STRCMPEQ(buf, group->name))
				return nov_file;

		}
	}

	return nov_file;
}


/*
 *  Run the index file updater only for the groups we've loaded.
 */
void
do_update (
	t_bool catchup)
{
	char group_path[PATH_LEN];
	register int i, j;
	time_t beg_epoch;
	struct t_group *group;

	if (verbose)
		(void) time (&beg_epoch);

	/*
	 * loop through groups and update any required index files
	 */
	for (i = 0; i < selmenu.max; i++) {
		group = &active[my_group[i]];
		make_group_path (group->name, group_path);

		if (verbose) {
			my_printf ("%s %s\n", (catchup ? "Catchup" : "Updating"), group->name);
			my_flush();
		}
		if (!index_group (group))
			continue;

		if (catchup) {
			for (j = 0; j < top_art; j++)
				art_mark_read (group, &arts[j]);
		}
	}

	if (verbose) {
		wait_message (0, _(txt_catchup_update_info),
			(catchup ? _(txt_caughtup) : _(txt_updated)), selmenu.max,
			PLURAL(selmenu.max, txt_group), (unsigned long int) (time(NULL) - beg_epoch));
	}
}


static int
artnum_comp (
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
subj_comp (
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * return result of strcmp (reversed for descending)
	 */
	return (CURR_GROUP.attribute->sort_art_type == SORT_BY_SUBJ_ASCEND
			? (retval = strcasecmp (s1->subject, s2->subject))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1
			: (retval = strcasecmp (s2->subject, s1->subject))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1);
}


static int
from_comp (
	t_comptype p1,
	t_comptype p2)
{
	int retval;
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	/*
	 * return result of strcmp (reversed for descending)
	 */
	return (CURR_GROUP.attribute->sort_art_type == SORT_BY_FROM_ASCEND
			? (retval = strcasecmp (s1->from, s2->from))
				? retval : ((s1->date - s2->date) > 0) ? 1 : -1
			: (retval = strcasecmp (s2->from, s1->from))
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
date_comp (
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (CURR_GROUP.attribute->sort_art_type == SORT_BY_DATE_ASCEND) {
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
score_comp (
	t_comptype p1,
	t_comptype p2)
{
	const struct t_article *s1 = (const struct t_article *) p1;
	const struct t_article *s2 = (const struct t_article *) p2;

	if (CURR_GROUP.attribute->sort_art_type == SORT_BY_SCORE_ASCEND) {
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

	return 0;
}


void
set_article (
	struct t_article *art)
{
	art->subject	= (char *) 0;
	art->from	= (char *) 0;
	art->name	= (char *) 0;
	art->date	= (time_t) 0;
	art->xref	= (char *) 0;
	art->msgid	= (char *) 0;
	art->refs	= (char *) 0;
	art->refptr	= (struct t_msgid *) 0;
	art->line_count	= -1;
	art->archive	= (char *) 0;
	art->part	= (char *) 0;
	art->patch	= (char *) 0;
	art->thread	= ART_EXPIRED;
	art->status	= ART_UNREAD;
	art->inthread	= FALSE;
	art->killed	= FALSE;
	art->tagged	= FALSE;
	art->selected	= FALSE;
	art->zombie	= FALSE;
	art->delete_it	= FALSE;
	art->inrange	= FALSE;
}


/*
 * Do a binary chop to see if 'art' (an article number) exists in arts[]
 * Return index into arts[] or -1
 */
static int
valid_artnum (
	long art)
{
	register int prev, range;
	register int dctop = top_art;
	register int cur = 1;

	while ((dctop /= 2))
		cur *= 2;

	range = cur / 2;
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

		range /= 2;
	}
	return -1;
}


static void
print_expired_arts (
	int num_expired)
{
	int i;

	if (cmd_line && verbose) {
#ifdef DEBUG
		if (debug)
			my_printf ("Expired Index Arts=[%d]", num_expired);
#endif /* DEBUG */
		for (i = 0; i < num_expired; i++)
			my_fputc ('P', stdout);

		if (num_expired)
			my_flush();
	}
}


static char *
print_date (
	time_t secs)
{
	static char date[25];
	struct tm *tm;

	static const char *const months_a[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};

	tm = localtime (&secs);
	sprintf(date, "%02d %s %04d %02d:%02d:%02d",
			tm->tm_mday,
			months_a[tm->tm_mon],
			tm->tm_year + 1900,
			tm->tm_hour, tm->tm_min, tm->tm_sec);

	return date;
}


static char *
print_from (
	struct t_article *article)
{
	static char from[PATH_LEN];

	*from = '\0';

	if (article->name != (char *) 0)
		snprintf (from, sizeof(from)-1, "%s <%s>", rfc1522_encode(article->name, FALSE), article->from);
	else
		STRCPY (from, article->from);

	return from;
}
