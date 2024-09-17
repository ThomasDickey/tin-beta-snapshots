/*
 *  Project   : tin - a Usenet reader
 *  Module    : debug.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2024-09-10
 *  Notes     : debug routines
 *
 * Copyright (c) 1991-2024 Iain Lea <iain@bricbrac.de>
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

#ifdef DEBUG
#	ifndef NEWSRC_H
#		include "newsrc.h"
#	endif /* !NEWSRC_H */
#endif /* DEBUG */

unsigned short debug;

#ifdef DEBUG
/*
 * Local prototypes
 */
static void debug_print_attributes(struct t_attribute *attr, FILE *fp);
static void debug_print_filter(FILE *fp, int num, struct t_filter *the_filter);
static void debug_print_newsrc(struct t_newsrc *lnewsrc, FILE *fp);


/*
 * remove debug files
 */
void
debug_delete_files(
	void)
{
	char file[PATH_LEN];

	if (debug & (DEBUG_NNTP | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "NNTP");
		unlink(file);
	}

	if (debug & (DEBUG_FILTER | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "ARTS");
		unlink(file);
		joinpath(file, sizeof(file), tmpdir, "FILTER");
		unlink(file);
	}

	if (debug & (DEBUG_NEWSRC | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "BITMAP");
		unlink(file);
	}

	if (debug & (DEBUG_REFS | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "REFS.dump");
		unlink(file);
		joinpath(file, sizeof(file), tmpdir, "REFS.info");
		unlink(file);
	}

	if (debug & (DEBUG_MEM | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "MALLOC");
		unlink(file);
	}

	if (debug & (DEBUG_ATTRIB | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "ATTRIBUTES");
		unlink(file);
		joinpath(file, sizeof(file), tmpdir, "SCOPES-R");
		unlink(file);
		joinpath(file, sizeof(file), tmpdir, "SCOPES-W");
		unlink(file);
	}

	if (debug & (DEBUG_MISC | DEBUG_REMOVE)) {
		joinpath(file, sizeof(file), tmpdir, "ACTIVE");
		unlink(file);
		joinpath(file, sizeof(file), tmpdir, "GNKSA");
		unlink(file);
	}
}


/*
 * tin specific debug routines
 */
void
debug_print_arts(
	void)
{
	int i;

	for_each_art(i) /* fopen/close() horror */
		debug_print_header(&arts[i]);
}


void
debug_print_header(
	struct t_article *s)
{
	static char file[PATH_LEN] = { '\0' };
	FILE *fp;

	if (!(debug & DEBUG_FILTER))
		return;

	if (!*file)
		joinpath(file, sizeof(file), tmpdir, "ARTS");

	if ((fp = fopen(file, "a")) != NULL) {
		fprintf(fp, "art=[%5"T_ARTNUM_PFMT"] tag=[%s] kill=[%s] selected=[%s]\n", s->artnum,
			bool_unparse(s->tagged),
			bool_unparse(s->killed),
			bool_unparse(s->selected));
		fprintf(fp, "subj=[%-38s]\n", s->subject);
		fprintf(fp, "date=[%ld]  from=[%s]  name=[%s]\n", (long) s->date, s->mailbox.from,
			BlankIfNull(s->mailbox.name));

#if 0	/* msgid and refs are only retained until the reference tree is built */
		if (s->msgid || s->refs)
			fprintf(fp, "msgid=[%s]  refs=[%s]\n", BlankIfNull(s->msgid), BlankIfNull(s->refs));
#endif /* 0 */

		if (s->score != 0)
			fprintf(fp, "score=[%d] gnksa=[%d] lines=[%d]\n", s->score, s->mailbox.gnksa_code, s->line_count);

		fprintf(fp, "thread=[%d]  prev=[%d]  status=[%u]\n\n", s->thread, s->prev, (unsigned) s->status);
		fflush(fp);
#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
#	else
#		ifdef HAVE_CHMOD
		chmod(file, (S_IRUGO|S_IWUGO));
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		fclose(fp);
	}
}


void
debug_print_active(
	void)
{
	static char file[PATH_LEN] = { '\0' };
	FILE *fp;

	if (!(debug & DEBUG_MISC))
		return;

	if (!*file)
		joinpath(file, sizeof(file), tmpdir, "ACTIVE");

	if ((fp = fopen(file, "w")) != NULL) {
		int i;
		struct t_group *group;

		for_each_group(i) {
			group = &active[i];
			fprintf(fp, "[%4d]=[%s] type=[%s] spooldir=[%s]\n",
				i, group->name,
				(group->type == GROUP_TYPE_NEWS ? "NEWS" : "MAIL"),
				group->spooldir);
			fprintf(fp, "count=[%4"T_ARTNUM_PFMT"] max=[%4"T_ARTNUM_PFMT"] min=[%4"T_ARTNUM_PFMT"] mod=[%c]\n",
				group->count, group->xmax, group->xmin, group->moderated);
			fprintf(fp, " nxt=[%4d] hash=[%lu]  description=[%s]\n", group->next,
				hash_groupname(group->name), BlankIfNull(group->description));
			if (debug & DEBUG_NEWSRC)
				debug_print_newsrc(&group->newsrc, fp);
			if (debug & DEBUG_ATTRIB)
				debug_print_attributes(group->attribute, fp);
		}
#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
#	else
#		ifdef HAVE_CHMOD
		chmod(file, (S_IRUGO|S_IWUGO));
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		fclose(fp);
	}
}


static void
debug_print_attributes(
	struct t_attribute *attr,
	FILE *fp)
{
	if (attr == NULL)
		return;

	fprintf(fp, "global=[%u] show=[%u] thread=[%u] sort=[%u] author=[%u] auto_select=[%u] batch_save=[%u] process=[%u]\n",
		(unsigned) attr->global,
		(unsigned) attr->show_only_unread_arts,
		(unsigned) attr->thread_articles,
		(unsigned) attr->sort_article_type,
		(unsigned) attr->show_author,
		(unsigned) attr->auto_select,
		(unsigned) attr->batch_save,
		(unsigned) attr->post_process_type);
	fprintf(fp, "select_header=[%u] select_global=[%s] select_expire=[%s]\n",
		(unsigned) attr->quick_select_header,
		attr->quick_select_scope ? attr->quick_select_scope ? BlankIfNull(*attr->quick_select_scope) : "" : "",
		bool_unparse(attr->quick_select_expire));
	fprintf(fp, "kill_header  =[%u] kill_global  =[%s] kill_expire  =[%s]\n",
		(unsigned) attr->quick_kill_header,
		attr->quick_kill_scope ? attr->quick_kill_scope ? BlankIfNull(*attr->quick_kill_scope) : "" : "",
		bool_unparse(attr->quick_kill_expire));
	fprintf(fp, "maildir=[%s] savedir=[%s] savefile=[%s]\n",
		attr->maildir ? BlankIfNull(*attr->maildir) : "",
		attr->savedir ? BlankIfNull(*attr->savedir) : "",
		attr->savefile ? BlankIfNull(*attr->savefile) : "");
	fprintf(fp, "sigfile=[%s] followup_to=[%s]\n\n",
		attr->sigfile ? BlankIfNull(*attr->sigfile) : "",
		attr->followup_to ? BlankIfNull(*attr->followup_to) : "");
	fflush(fp);
}


void
debug_print_malloc(
	t_bool is_malloc,
	const char *xfile,
	int line,
	size_t size)
{
	static char file[PATH_LEN] = { '\0' };
	static size_t total = 0;
	FILE *fp;

	if (!*file)
		joinpath(file, sizeof(file), tmpdir, "MALLOC");

	if ((fp = fopen(file, "a")) != NULL) {
		total += size;
		/* sometimes size_t is long */
		fprintf(fp, "%12s:%-4d %s(%6lu). Total %lu\n", xfile, line, is_malloc ? " malloc" : "realloc", (unsigned long) size, (unsigned long) total);
#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
#	else
#		ifdef HAVE_CHMOD
		chmod(file, (S_IRUGO|S_IWUGO));
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		fclose(fp);
	}
}


static void
debug_print_filter(
	FILE *fp,
	int num,
	struct t_filter *the_filter)
{
	static const char sign[] = { ' ', '=', '<', '>', '\0' };

	fprintf(fp, "[%3d]  group=[%s]\n       inscope=[%s] score=[%d] case=[%s]\n",
		num, BlankIfNull(the_filter->scope),
		(the_filter->inscope ? "TRUE" : "FILTER"),
		the_filter->score,
		the_filter->icase ? "I" : "C");

	if (the_filter->subj)
		fprintf(fp, "       subj=[%s]\n", the_filter->subj);
	if (the_filter->from)
		fprintf(fp, "       from=[%s]\n", the_filter->from);
	if (the_filter->msgid)
		fprintf(fp, "       msgid=[%s]\n", the_filter->msgid);
	if (the_filter->xref)
		fprintf(fp, "       xref=[%s]\n", the_filter->xref);
	if (the_filter->path)
		fprintf(fp, "       path=[%s]\n", the_filter->path);

	fprintf(fp, "       lines=[%c%d] gnksa=[%c%d]\n",
		sign[(int) the_filter->lines_cmp], the_filter->lines_num,
		sign[(int) the_filter->gnksa_cmp], the_filter->gnksa_num);

	if (the_filter->time)
		fprintf(fp, "       time=[%ld][%s]\n", (long) the_filter->time, BlankIfNull(str_trim(ctime(&the_filter->time))));
}


void
debug_print_filters(
	void)
{
	int i, num;
	static char file[PATH_LEN] = { '\0' };
	struct t_filter *the_filter;
	FILE *fp;

	if (!*file)
		joinpath(file, sizeof(file), tmpdir, "FILTER");

	if ((fp = fopen(file, "w")) != NULL) {
		/*
		 * print global filter
		 */
		num = glob_filter.num;
		the_filter = glob_filter.filter;
		fprintf(fp, "*** BEG GLOBAL FILTER=[%3d] ***\n", num);
		for (i = 0; i < num; i++) {
			debug_print_filter(fp, i, &the_filter[i]);
			fprintf(fp, "\n");
		}
		fprintf(fp, "*** END GLOBAL FILTER ***\n");

#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
#	else
#		ifdef HAVE_CHMOD
		chmod(file, (S_IRUGO|S_IWUGO));
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		fclose(fp);
	}
}


void
debug_print_file(
	const char *fname,
	const char *fmt,
	...)
{
	FILE *fp;
	char *buf;
	char file[PATH_LEN];
	va_list ap;

	if (!debug)
		return;

	va_start(ap, fmt);
	buf = fmt_message(fmt, ap);
	va_end(ap);

	joinpath(file, sizeof(file), tmpdir, fname);

	if ((fp = fopen(file, "a")) != NULL) {
		fprintf(fp, "%s\n", buf);
#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
#	else
#		ifdef HAVE_CHMOD
		chmod(file, (S_IRUGO|S_IWUGO));
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		fclose(fp);
	}
	free(buf);
}


void
debug_print_comment(
	const char *comment)
{
	if (!(debug & DEBUG_NEWSRC))
		return;

	debug_print_file("BITMAP", comment);
}


void
debug_print_bitmap(
	struct t_group *group,
	struct t_article *art)
{
	FILE *fp;
	char file[PATH_LEN];

	if (!(debug & DEBUG_NEWSRC))
		return;

	joinpath(file, sizeof(file), tmpdir, "BITMAP");
	if (group != NULL) {
		if ((fp = fopen(file, "a")) != NULL) {
			fprintf(fp, "\nActive: Group=[%s] sub=[%c] min=[%"T_ARTNUM_PFMT"] max=[%"T_ARTNUM_PFMT"] count=[%"T_ARTNUM_PFMT"] num_unread=[%"T_ARTNUM_PFMT"]\n",
				group->name, SUB_CHAR(group->subscribed),
				group->xmin, group->xmax, group->count,
				group->newsrc.num_unread);
			if (art != NULL) {
				fprintf(fp, "art=[%5"T_ARTNUM_PFMT"] tag=[%s] kill=[%s] selected=[%s] subj=[%s]\n",
					art->artnum,
					bool_unparse(art->tagged),
					bool_unparse(art->killed),
					bool_unparse(art->selected),
					art->subject);
				fprintf(fp, "thread=[%d]  prev=[%d]  status=[%s]\n",
					art->thread, art->prev,
					(art->status == ART_READ ? "READ" : "UNREAD"));
			}
			debug_print_newsrc(&group->newsrc, fp);
#	ifdef HAVE_FCHMOD
			fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
#	else
#		ifdef HAVE_CHMOD
			chmod(file, (S_IRUGO|S_IWUGO));
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
			fclose(fp);
		}
	}
}


static void
debug_print_newsrc(
	struct t_newsrc *lnewsrc,
	FILE *fp)
{
	int j;
	t_artnum i;

	fprintf(fp, "Newsrc: min=[%"T_ARTNUM_PFMT"] max=[%"T_ARTNUM_PFMT"] bitlen=[%"T_ARTNUM_PFMT"] num_unread=[%"T_ARTNUM_PFMT"] present=[%d]\n",
		lnewsrc->xmin, lnewsrc->xmax, lnewsrc->xbitlen,
		lnewsrc->num_unread, (lnewsrc->present ? 1 : 0));

	fprintf(fp, "bitmap=[");
	if (lnewsrc->xbitlen && lnewsrc->xbitmap) {
		for (j = 0, i = lnewsrc->xmin; i <= lnewsrc->xmax; i++) {
			fprintf(fp, "%d",
				(NTEST(lnewsrc->xbitmap, i - lnewsrc->xmin) == ART_READ ?
				ART_READ : ART_UNREAD));
			if ((j++ % 8) == 7 && i < lnewsrc->xmax)
				fprintf(fp, " ");
		}
	}
	fprintf(fp, "]\n");
	fflush(fp);
}


#	ifdef NNTP_ABLE
const char *
logtime(
	void)
{
#		if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
	static struct t_tintime log_time;
	static char out[40];

	if (tin_gettime(&log_time) == 0) {
		if (my_strftime(out, 39, " [%H:%M:%S.", gmtime(&(log_time.tv_sec)))) {
			snprintf(out + 11, sizeof(out) - 11, "%09ld", log_time.tv_nsec); /* strlen(" [hh:mm:ss.") */
			out[17] = '\0'; /* strlen(" [hh:mm:ss.uuuuuu") */
			strcat(out, "] ");
			return out;
		}
	}
#		endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */
	return " ";
}
#	endif /* NNTP_ABLE */
#endif /* DEBUG */
