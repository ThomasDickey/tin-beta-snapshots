/*
 *  Project   : tin - a Usenet reader
 *  Module    : debug.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2005-05-04
 *  Notes     : debug routines
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>
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

#if defined(DEBUG) || defined(DEBUG_NEWSRC)
#	ifndef NEWSRC_H
#		include "newsrc.h"
#	endif /* !NEWSRC_H */
#	ifndef TCURSES_H
#		include "tcurses.h"
#	endif /* !TCURSES_H */
#endif /* DEBUG || DEBUG_NEWSRC */

int debug;

#ifdef DEBUG
/*
 * Local prototypes
 */
static void debug_print_attributes(struct t_attribute *attr, FILE *fp);
static void debug_print_filter(FILE *fp, int num, struct t_filter *the_filter);


/*
 * nntp specific debug routines
 */
void
debug_delete_files(
	void)
{
	char file[PATH_LEN];

	if (debug) {
		joinpath(file, TMPDIR, "NNTP");
		unlink(file);
		joinpath(file, TMPDIR, "ARTS");
		unlink(file);
		joinpath(file, TMPDIR, "SAVE_COMP");
		unlink(file);
		joinpath(file, TMPDIR, "BASE");
		unlink(file);
		joinpath(file, TMPDIR, "ACTIVE");
		unlink(file);
		joinpath(file, TMPDIR, "BITMAP");
		unlink(file);
		joinpath(file, TMPDIR, "MALLOC");
		unlink(file);
		joinpath(file, TMPDIR, "FILTER");
		unlink(file);
	}
}


/*
 * nntp specific debug routines
 */
void
debug_nntp(
	const char *func,
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

	joinpath(file, TMPDIR, "NNTP");

	if ((fp = fopen(file, "a+")) != NULL) {
		fprintf(fp,"%s: %s\n", func, buf);
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
		fclose(fp);
	}
	free(buf);
	va_end(ap);
}


/*
 * tin specific debug routines
 */
void
debug_print_arts(
	void)
{
	int i;

	if (debug != 2)
		return;

	for_each_art(i)
		debug_print_header(&arts[i]);
}


void
debug_print_header(
	struct t_article *s)
{
	FILE *fp;
	char file[PATH_LEN];

	if (debug != 2)
		return;

	joinpath(file, TMPDIR, "ARTS");

	if ((fp = fopen(file, "a+")) != NULL) {
		fprintf(fp,"art=[%5ld] tag=[%s] kill=[%s] selected=[%s]\n", s->artnum,
			bool_unparse(s->tagged),
			bool_unparse(s->killed),
			bool_unparse(s->selected));
		fprintf(fp,"subj=[%-38s]\n", s->subject);
		fprintf(fp,"date=[%ld]  from=[%s]  name=[%s]\n", (long) s->date, s->from,
			BlankIfNull(s->name));
		fprintf(fp,"msgid=[%s]  refs=[%s]\n",
			BlankIfNull(s->msgid),
			BlankIfNull(s->refs));

		if (s->archive) {
			fprintf(fp, "archive.name=[%-38s]  ", s->archive->name);
			if (s->archive->partnum)
				fprintf(fp, "archive.partnum=[%s]  ", s->archive->partnum);
			if (s->archive->ispart)
				fprintf(fp, "archive.ispart=[%s]\n", bool_unparse(s->archive->ispart));
		}
		fprintf(fp,"thread=[%d]  prev=[%d]  status=[%d]\n\n", s->thread, s->prev, s->status);
		fflush(fp);
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
		fclose(fp);
	}
}


void
debug_print_active(
	void)
{
	FILE *fp;
	char file[PATH_LEN];
	int i;
	struct t_group *group;

	if (debug != 2)
		return;

	joinpath(file, TMPDIR, "ACTIVE");

	if ((fp = fopen(file, "w")) != NULL) {
		for_each_group(i) {
			group = &active[i];
			fprintf(fp, "[%4d]=[%s] type=[%s] spooldir=[%s]\n",
				i, group->name,
				(group->type == GROUP_TYPE_NEWS ? "NEWS" : "MAIL"),
				group->spooldir);
			fprintf(fp, "count=[%4ld] max=[%4ld] min=[%4ld] mod=[%c]\n",
				group->count, group->xmax, group->xmin, group->moderated);
			fprintf(fp, " nxt=[%4d] hash=[%ld]  description=[%s]\n", group->next,
				hash_groupname(group->name), BlankIfNull(group->description));
#	ifdef DEBUG_NEWSRC
			debug_print_newsrc(&group->newsrc, fp);
#	endif /* DEBUG_NEWSRC */
			debug_print_attributes(group->attribute, fp);
		}
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
		fclose(fp);
	}
}


static void
debug_print_attributes(
	struct t_attribute *attr,
	FILE *fp)
{
	if (attr == 0)
		return;

	fprintf(fp, "global=[%d] show=[%d] thread=[%d] sort=[%d] author=[%d] auto_select=[%d] auto_save=[%d] batch_save=[%d] process=[%d]\n",
		attr->global,
		attr->show_only_unread,
		attr->thread_arts,
		attr->sort_art_type,
		attr->show_author,
		attr->auto_select,
		attr->auto_save,
		attr->batch_save,
		attr->post_proc_type);
	fprintf(fp, "select_header=[%d] select_global=[%s] select_expire=[%s]\n",
		attr->quick_select_header,
		BlankIfNull(attr->quick_select_scope),
		bool_unparse(attr->quick_select_expire));
	fprintf(fp, "kill_header  =[%d] kill_global  =[%s] kill_expire  =[%s]\n",
		attr->quick_kill_header,
		BlankIfNull(attr->quick_kill_scope),
		bool_unparse(attr->quick_kill_expire));
	fprintf(fp, "maildir=[%s] savedir=[%s] savefile=[%s]\n",
		BlankIfNull(attr->maildir),
		BlankIfNull(attr->savedir),
		BlankIfNull(attr->savefile));
	fprintf(fp, "sigfile=[%s] followup_to=[%s]\n\n",
		BlankIfNull(attr->sigfile),
		BlankIfNull(attr->followup_to));
	fflush(fp);
}


void
debug_print_malloc(
	int is_malloc,
	const char *xfile,
	int line,
	size_t size)
{
	FILE *fp;
	char file[PATH_LEN];
	static int total = 0;

	if (debug == 4) {
		joinpath(file, TMPDIR, "MALLOC");
		if ((fp = fopen(file, "a+")) != NULL) {
			total += size;
			/* sometimes size_t is long */
			if (is_malloc)
				fprintf(fp, "%10s:%-4d Malloc  %6ld. Total %d\n", xfile, line, (long) size, total);
			else
				fprintf(fp, "%10s:%-4d Realloc %6ld. Total %d\n", xfile, line, (long) size, total);
			fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
			fclose(fp);
		}
	}
}


static void
debug_print_filter(
	FILE *fp,
	int num,
	struct t_filter *the_filter)
{
	fprintf(fp, "[%3d]  group=[%s] inscope=[%s] score=[%d] case=[%d] lines=[%d %d]\n",
		num, BlankIfNull(the_filter->scope),
		(the_filter->inscope ? "TRUE" : "FILTER"),
		the_filter->score,
		the_filter->icase,
		the_filter->lines_cmp, the_filter->lines_num);

	fprintf(fp, "       subj=[%s] from=[%s] msgid=[%s]\n",
		BlankIfNull(the_filter->subj),
		BlankIfNull(the_filter->from),
		BlankIfNull(the_filter->msgid));

	if (the_filter->time)
		fprintf(fp, "       time=[%ld][%s", the_filter->time, (the_filter->time ? ctime(&the_filter->time) : "]\n"));
}


void
debug_print_filters(
	void)
{
	FILE *fp;
	char file[PATH_LEN];
	int i, num;
	struct t_filter *the_filter;

	if (debug < 2)
		return;

	joinpath(file, TMPDIR, "FILTER");

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

		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
		fclose(fp);
	}
}


/* TODO: print out all fields of t_capabilities */
#	ifdef NNTP_ABLE
void
debug_print_nntp_extensions(
	void)
{
	if (debug < 2)
		return;
	debug_nntp("###", "NNTP EXTENSIONS/CAPABILITIES");
	debug_nntp("###", "Type/Version : %d/%d", nntp_caps.type, nntp_caps.version);
	debug_nntp("###", "Command-names: %s %s", BlankIfNull(nntp_caps.over_cmd), BlankIfNull(nntp_caps.hdr_cmd));
	debug_nntp("###", "List         : %s", nntp_caps.list_motd ? "MOTD" : "");
}
#	endif /* NNTP_ABLE */
#endif /* DEBUG */


#ifdef DEBUG_NEWSRC
void
debug_print_comment(
	const char *comment)
{
	FILE *fp;
	char file[PATH_LEN];

	if (debug < 2)
		return;

	joinpath(file, TMPDIR, "BITMAP");

	if ((fp = fopen(file, "a+")) != NULL) {
		fprintf(fp,"\n%s\n", comment);
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
		fclose(fp);
	}
}


void
debug_print_bitmap(
	struct t_group *group,
	struct t_article *art)
{
	FILE *fp;
	char file[PATH_LEN];

	if (debug != 3)
		return;

	joinpath(file, TMPDIR, "BITMAP");
	if ((fp = fopen(file, "a+")) != NULL) {
		fprintf(fp, "\nActive: Group=[%s] sub=[%c] min=[%ld] max=[%ld] count=[%ld] num_unread=[%ld]\n",
			group->name, SUB_CHAR(group->subscribed),
			group->xmin, group->xmax, group->count,
			group->newsrc.num_unread);
		if (art != NULL) {
			fprintf(fp, "art=[%5ld] tag=[%s] kill=[%s] selected=[%s] subj=[%s]\n",
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
		fchmod(fileno(fp), (S_IRUGO|S_IWUGO));
		fclose(fp);
	}
}


void
debug_print_newsrc(
	struct t_newsrc *lnewsrc,
	FILE *fp)
{
	int i, j;

	fprintf(fp, "Newsrc: min=[%ld] max=[%ld] bitlen=[%ld] num_unread=[%ld] present=[%d]\n",
		lnewsrc->xmin, lnewsrc->xmax, lnewsrc->xbitlen,
		lnewsrc->num_unread, lnewsrc->present);

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
#endif /* DEBUG_NEWSRC */
