/*
 *  Project   : tin - a Usenet reader
 *  Module    : mail.c
 *  Author    : I. Lea
 *  Created   : 1992-10-02
 *  Updated   : 2001-07-22
 *  Notes     : Mail handling routines for creating pseudo newsgroups
 *
 * Copyright (c) 1992-2002 Iain Lea <iain@bricbrac.de>
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
 *    This product includes software developed by Iain Lea.
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

static void read_groups_descriptions (FILE *fp, FILE *fp_save);


/*
 * Load the mail active file into active[]
 */
#ifdef HAVE_MH_MAIL_HANDLING
void
read_mail_active_file (
	void)
{
	FILE *fp;
	char buf[LEN];
	char my_spooldir[PATH_LEN];
	long count = -1L;
	long min, max;
	struct t_group *ptr;

	if (!batch_mode)
		wait_message (0, _(txt_reading_mail_active_file));

	/*
	 * Open the mail active file
	 */
	if ((fp = open_mail_active_fp ("r")) == (FILE *) 0) {
		if (cmd_line)
			my_fputc ('\n', stderr);

		error_message (_(txt_cannot_open), mail_active_file);
		/*
		 * FIXME - maybe do an autoscan of maildir, create & do a reopen ?
		 */
		write_mail_active_file ();
		return;
	}

	while (fgets (buf, (int) sizeof (buf), fp) != (char *) 0) {
		if (!parse_active_line (buf, &max, &min, my_spooldir) || *buf == '\0')
			continue;

		/*
		 * Update mailgroup info
		 */
		if ((ptr = group_find (buf)) != NULL) {
			if (strcmp(ptr->spooldir, my_spooldir) != 0) {
				free(ptr->spooldir);
				ptr->spooldir = my_strdup(my_spooldir);
			}
			ptr->xmax = max;
			ptr->xmin = min;
			continue;
		}

		/*
		 * Load mailgroup into group hash table
		 */
		if ((ptr = group_add (buf)) == NULL)
			continue;

		/*
		 * Load group info. TODO - integrate with active_add()
		 */
		ptr->aliasedto = (char *) 0;
		ptr->description = (char *) 0;
		ptr->spooldir = my_strdup (my_spooldir);
		ptr->moderated = 'y';
		ptr->count = count;
		ptr->xmax = max;
		ptr->xmin = min;
		ptr->type = GROUP_TYPE_MAIL;
		ptr->inrange = FALSE;
		ptr->read_during_session = FALSE;
		ptr->art_was_posted = FALSE;
		ptr->subscribed = FALSE;		/* not in my_group[] yet */
		ptr->newgroup = FALSE;
		ptr->bogus = FALSE;
		ptr->next = -1;			/* hash chaining */
		ptr->newsrc.xbitmap = (t_bitmap *) 0;
		ptr->attribute = (struct t_attribute *) 0;
		ptr->glob_filter = &glob_filter;
		set_default_bitmap (ptr);
	}
	fclose (fp);

	if (!batch_mode)
		my_fputs("\n", stdout);
}


/*
 * Write out mailgroups from active[] to ~/.tin/active.mail
 */
void
write_mail_active_file (
	void)
{
	FILE *fp;
	char *file_tmp;
	char group_path[PATH_LEN];
	register int i;
	struct t_group *group;

	if (no_write && file_size (mail_active_file) != -1L)
		return;

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(mail_active_file);

	if (!backup_file (mail_active_file, file_tmp)) {
		error_message (_(txt_filesystem_full_backup), mail_active_file);
		/* free memory for tmp-filename */
		free (file_tmp);
		return;
	}

	print_active_head (mail_active_file);

	if ((fp = open_mail_active_fp ("a+")) != (FILE *) 0) {
		for (i = 0; i < num_active; i++) {
			group = &active[i];
			if (group->type == GROUP_TYPE_MAIL) {
				make_base_group_path (group->spooldir, group->name, group_path);
				find_art_max_min (group_path, &group->xmax, &group->xmin);
				print_group_line (fp, group->name, group->xmax, group->xmin, group->spooldir);
			}
		}
		if (ferror (fp) || fclose (fp)) {
			error_message (_(txt_filesystem_full), mail_active_file);
			rename (file_tmp, mail_active_file);
		} else
			unlink (file_tmp);
	}

	/* free memory for tmp-filename */
	free (file_tmp);
}


/*
 * Load the text description from ~/.tin/mailgroups for each mail group into
 * the active[] array.
 */
void
read_mailgroups_file (
	void)
{
	FILE *fp;

	if ((fp = open_mailgroups_fp ()) != (FILE *) 0) {
		if (!batch_mode)
			wait_message (0, _(txt_reading_mailgroups_file));

		read_groups_descriptions (fp, (FILE *) 0);

		fclose (fp);

		if (!batch_mode)
			my_fputs("\n", stdout);
	}
}
#endif /* HAVE_MH_MAIL_HANDLING */


/*
 * Load the text description from NEWSLIBDIR/newsgroups for each group into the
 * active[] array. Save a copy locally if reading via NNTP to save bandwidth.
 */
void
read_newsgroups_file (
	void)
{
	FILE *fp;
	FILE *fp_save = (FILE *) 0;

	if ((fp = open_newsgroups_fp ()) != (FILE *) 0) {
		if (!batch_mode)
			wait_message (0, _(txt_reading_newsgroups_file));


		if (read_news_via_nntp && !read_local_newsgroups_file && !no_write)
			fp_save = fopen (local_newsgroups_file, "w" FOPEN_OPTS);

		read_groups_descriptions (fp, fp_save);

		if (fp_save != (FILE *) 0) {
			fclose (fp_save);
			read_local_newsgroups_file = TRUE;
		}

		TIN_FCLOSE (fp);

		if (!batch_mode)
			my_fputs("\n", stdout);
	}
}


/*
 * Read groups descriptions from opened file & make local backup copy
 * of all groups that don't have a 'x' in the active file moderated
 * field & if reading groups of type GROUP_TYPE_NEWS.
 * Aborting this early won't have any adverse affects, just some missing
 * descriptions.
 */
static void
read_groups_descriptions (
	FILE *fp,
	FILE *fp_save)
{
	char *p, *q, *ptr;
	char *groupname = (char *) 0;
	int count = 0;
	size_t space = 0;
	struct t_group *group;

	while ((ptr = tin_fgets (fp, FALSE)) != (char *) 0) {
		if (*ptr == '#' || *ptr == '\0')
			continue;

		/*
		 * This was moved from below and simplified. I can't test here for the
		 * type of group being read, because that requires having found the
		 * group in the active file, and that truncates the local copy of the
		 * newsgroups file to only subscribed-to groups when tin is called
		 * with the "-q" option.
		 */
		if ((fp_save != (FILE *) 0) && read_news_via_nntp && !read_local_newsgroups_file)
			fprintf (fp_save, "%s\n", ptr);

		if (!space) { /* initial malloc */
			space = strlen(ptr) + 1;
			groupname = my_malloc(space);
		} else {
			while (strlen(ptr) > space) { /* realloc needed? */
				space <<= 1; /* double size */
				groupname = (char *) my_realloc((void *) groupname, space);
			}
		}

		for (p = ptr, q = groupname ; *p && *p != ' ' && *p != '\t' ; p++, q++)
			*q = *p;

		*q = '\0';

		while (*p == '\t' || *p == ' ')
			p++;

		group = group_find (groupname);

		if (group != (struct t_group *) 0 && group->description == (char *) 0) {
			q = p;
			while ((q = strchr (q, '\t')) != (char *) 0)
				*q = ' ';

			group->description = my_strdup (p);

#	if 0 /* not useful for cache_overview_files */
			if (group->type == GROUP_TYPE_NEWS) {
				if (fp_save != (FILE *) 0 && read_news_via_nntp && !read_local_newsgroups_file)
					fprintf (fp_save, "%s\n", ptr);
			}
#	endif /* 0 */
		}

		if (++count % 100 == 0)
			spin_cursor ();
	}
	free(groupname);
}


void
print_active_head (
	char *active_file)
{
	FILE *fp;

	if (no_write && file_size (active_file) != -1L)
		return;

	if ((fp = fopen (active_file, "w")) != (FILE *) 0) {
		/* FIXME: -> lang.c */
		fprintf (fp, _("# [Mail/Save] active file. Format is like news active file:\n"));
		fprintf (fp, _("#   groupname  max.artnum  min.artnum  /dir\n"));
		fprintf (fp, _("# The 4th field is the basedir (ie. ~/Mail or ~/News)\n#\n"));
		fclose (fp);
	}
}


void
find_art_max_min (
	char *group_path,
	long *art_max,
	long *art_min)
{
	DIR *dir;
	DIR_BUF *direntry;
	long art_num;

	*art_min = *art_max = 0L;

	if (access (group_path, R_OK) != 0) {
		*art_min = 1L;
		return;
	}

	dir = opendir (group_path);
	if (dir != (DIR *) 0) {
		while ((direntry = readdir (dir)) != (DIR_BUF *) 0) {
			art_num = atol (direntry->d_name);
			if (art_num >= 1) {
				if (art_num > *art_max) {
					*art_max = art_num;
					if (*art_min == 0)
						*art_min = art_num;
				} else if (art_num < *art_min)
					*art_min = art_num;
			}
		}
		CLOSEDIR(dir);
	}
	if (*art_min == 0)
		*art_min = 1;
}


void
print_group_line (
	FILE *fp,
	char *group_name,
	long art_max,
	long art_min,
	char *base_dir)
{
	fprintf (fp, "%s %05ld %05ld %s\n",
		group_name, art_max, art_min, base_dir);
}


/*
 * Given a base pathname & a newsgroup name build an absolute pathname.
 * base = /usr/spool/news
 * newsgroup = alt.sources
 * absolute path = /usr/spool/news/alt/sources
 */
void
make_base_group_path (
	char *base_dir,
	char *group_name,
	char *group_path)
{
	char *ptr;

	joinpath (group_path, base_dir, group_name);

	ptr = group_path + strlen (base_dir);
	while ((ptr = strchr (ptr, '.')) != (char *) 0)
		*ptr = '/';
}


void
vGrpDelMailArt (
	struct t_article *article)
{

	if (article->delete_it) {
		art_mark_undeleted (article);
		info_message (_(txt_art_undeleted));
	} else {
		art_mark_deleted (article);
		info_message (_(txt_art_deleted));
	}
}


void
vGrpDelMailArts (
	struct t_group *group)
{
	char article_filename[PATH_LEN];
	char group_path[PATH_LEN];
	int i;
	struct t_article *article;
#if 0 /* see comment below */
	t_bool update_index_file = FALSE;
#endif /* 0 */

	if (group->type == GROUP_TYPE_MAIL || group->type == GROUP_TYPE_SAVE) {
		wait_message (1, (group->type == GROUP_TYPE_MAIL) ? _(txt_processing_mail_arts) : _(txt_processing_saved_arts));
		make_base_group_path (group->spooldir, group->name, group_path);
		for (i = 0; i < top_art; i++) {
			article = &arts[i];
			if (article->delete_it) {
				sprintf (article_filename, "%s/%ld", group_path, article->artnum);
				unlink (article_filename);
				article->thread = ART_EXPIRED;
#if 0 /* see comment below */
				update_index_file = TRUE;
#endif /* 0 */
			}
		}

#if 0
/*
 * current tin's build_references() is changed to free msgid and refs,
 * therefore we cannot call write_nov_file after it. I simply commented
 * out this codes, NovFile will update at next time.
 */
/*
 * MAYBE also check if min / max article was deleted. If so then update
 * the active[] entry for the group and rewrite the mail.active file
 */
		if (update_index_file)
			write_nov_file (group);
#endif /* 0 */
	}
}


t_bool
art_edit (
	struct t_group *group,
	struct t_article *article)
{
	char article_filename[PATH_LEN];
	char temp_filename[PATH_LEN];
	t_bool ret = FALSE;

	/*
	 * Check if news / mail group
	 */
	if (group->type == GROUP_TYPE_NEWS)
		return FALSE;

	make_base_group_path (group->spooldir, group->name, temp_filename);
	snprintf (article_filename, sizeof(article_filename) - 1, "%s/%ld", temp_filename, article->artnum);
	snprintf (temp_filename, sizeof(temp_filename) - 1, "%s%d.art", TMPDIR, (int) process_id);

	if (!backup_file (article_filename, temp_filename))
		return FALSE;

	if (invoke_editor (temp_filename, 1)) {
		rename_file (temp_filename, article_filename);
		ret = TRUE;
	} else {
		unlink (temp_filename);
		ret = FALSE;
	}

	return ret;
}
