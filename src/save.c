/*
 *  Project   : tin - a Usenet reader
 *  Module    : save.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1997-12-31
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
#ifndef MENUKEYS_H
#	include  "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef RFC2045_H
#	include  "rfc2045.h"
#endif /* !RFC2045_H */

#ifdef HAVE_UUDEVIEW_H
#	ifndef __UUDEVIEW_H__
#		include <uudeview.h>
#	endif /* !__UUDEVIEW_H__ */
#endif /* HAVE_UUDEVIEW_H */

#undef OFF

#define INITIAL		1
#define MIDDLE		2
#define OFF		3
#define END		4

#ifndef HAVE_LIBUU
	/*
	 * types of archive programs
	 */
	static struct archiver_t {
		constext *name;
		constext *ext;
		constext *test;
		constext *list;
		constext *extract;
	} archiver[] = {
		{ "",		"",		"",		"",		""	},
		{ "",		"",		"",		"",		""	},
		{ "",		"",		"",		"",		""	},
#	ifdef M_AMIGA
		{ "lha",	"lha",		"t",		"l",		"x" },
#	else
		{ "zoo",	"zoo",		"-test",	"-list",	"-extract" },
#	endif /* M_AMIGA */
		{ "unzip",	"zip",		"-t",		"-l",		"-o"	},
		{ (char *) 0,	(char *) 0,	(char *) 0,	(char *) 0,	(char *) 0 }
	};
#endif /* !HAVE_LIBUU */


/*
 * Local prototypes
 */
static t_bool any_saved_files (void);
static char *get_first_savefile (void);
static char *save_filename (int i);
static char *get_last_savefile (void);
static void delete_processed_files (t_bool auto_delete);
static void post_process_sh (t_bool auto_delete);
static void post_process_uud (
#ifndef HAVE_LIBUU
	int pp,
#endif /* !HAVE_LIBUU */
	t_bool auto_delete);
#ifndef HAVE_LIBUU
	static void uudecode_file (int pp, char *file_out_dir, char *file_out, char *uudname);
#endif /* !HAVE_LIBUU */


/*
 *  Check for articles and say how many new/unread in each group.
 *  or
 *  Start if new/unread articles and return first group with new/unread.
 *  or
 *  Save any new articles to savedir and mark arts read and mail user
 *  and inform how many arts in which groups were saved.
 *  or
 *  Mail any new articles to specified user and mark arts read and mail
 *  user and inform how many arts in which groups were mailed.
 *  Return codes:
 *  CHECK_ANY_NEWS	- code to pass to exit() - see manpage for list
 *  START_ANY_NEWS	- index in my_group of first group with unread news or -1
 *  MAIL_ANY_NEWS	- not checked
 *  SAVE_ANY_NEWS	- not checked
 */
int
check_start_save_any_news (
	int function,
	t_bool catchup)
{
	FILE *fp;
	FILE *fp_log = (FILE *) 0;
	char *ich;
	char buf[LEN], logfile[LEN];
	char group_path[PATH_LEN];
	char path[PATH_LEN];
	char savefile[PATH_LEN];
	char subject[HEADER_LEN];
	int art_count, hot_count, group_count;
	int i, j;
	int saved_arts = 0;					/* Total # saved arts */
	struct t_group *group;
	t_bool log_opened = TRUE;
	t_bool print_first = TRUE;
	t_bool unread_news = FALSE;
	time_t epoch;

	switch (function) {
		case CHECK_ANY_NEWS:
		case START_ANY_NEWS:
			if (verbose)
				wait_message (0, _(txt_checking_for_news));
			break;
		case MAIL_ANY_NEWS:
		case SAVE_ANY_NEWS:
#	ifdef VMS
			joinpath (logfile, rcdir, "log.");
#	else
			sprintf (logfile, "%s/log", rcdir);
#	endif /* VMS */
			if (no_write || (fp_log = fopen (logfile, "w" FOPEN_OPTS)) == NULL) {
				perror_message (_(txt_cannot_open), logfile);
				fp_log = stdout;
				verbose = FALSE;
				log_opened = FALSE;
			}
			fprintf (fp_log, "To: %s\n", userid);
			(void) time (&epoch);
			sprintf (subject, "Subject: NEWS LOG %s", ctime (&epoch));
			/* Remove trailing \n introduced by ctime() */
			if ((ich = strrchr(subject, '\n')) != 0)
				*ich = '\0';
			fprintf (fp_log, "%s\n\n", subject);
			break;
		default:
			break;
	}

	group_count = 0;

	for (i = 0; i < selmenu.max; i++) {
		group = &active[my_group[i]];
		if (!index_group (group))
			continue;

		art_count = hot_count = 0;

		make_group_path (group->name, group_path);

		if (function == MAIL_ANY_NEWS || function == SAVE_ANY_NEWS) {
			if (!group->attribute->batch_save)
				continue;

			group_count++;
			sprintf (buf, _(txt_saved_groupname), group->name);
			fprintf (fp_log, buf);
			if (verbose)
				wait_message (0, buf);

			if (function == SAVE_ANY_NEWS) {
#	ifdef VMS
				sprintf (buf, "[.%s]dummy", group_path);
#	else
				sprintf (buf, "%s/dummy", group_path);
#	endif /* VMS */
				create_path (buf);
			}
		}

		for (j = 0; j < top_art; j++) {
			FILE *artfp;

			if (arts[j].status != ART_UNREAD)
				continue;

			switch (function) {
				case CHECK_ANY_NEWS:
					if (print_first && verbose) {
						my_fputc ('\n', stdout);
						print_first = FALSE;
					}
					art_count++;
					if (arts[j].score >= SCORE_SELECT)
						hot_count++;
					break;

				case START_ANY_NEWS:
					return i;	/* return first group with unread news */
					/* NOTREACHED */

				case MAIL_ANY_NEWS:
				case SAVE_ANY_NEWS:
					if ((artfp = open_art_fp (group_path, arts[j].artnum)) == NULL)
						continue;

					if (function == MAIL_ANY_NEWS)
						sprintf (savefile, "%stin.%d", TMPDIR, (int) process_id);
					else {
						if (!strfpath (group->attribute->savedir, path, sizeof (path), homedir, NULL, NULL, group->name))
							joinpath (path, homedir, DEFAULT_SAVEDIR);
#	ifdef VMS
						sprintf (savefile, "%s.%s]%ld", path, group_path, arts[j].artnum);
#	else
						sprintf (savefile, "%s/%s/%ld", path, group_path, arts[j].artnum);
#	endif /* VMS */
					}

					if ((fp = fopen (savefile, "w" FOPEN_OPTS)) == (FILE *) 0) {
						fprintf (fp_log, _(txt_cannot_open), savefile);
						if (verbose)
							perror_message (_(txt_cannot_open), savefile);
						TIN_FCLOSE (artfp);
						continue;
					}

					if (function == MAIL_ANY_NEWS)
						fprintf (fp, "To: %s\n", mail_news_user);

					sprintf (buf, "[%5ld]  %s\n", arts[j].artnum, arts[j].subject);
					fprintf (fp_log, "%s", buf);		/* buf may contain % */
					if (verbose)
						wait_message (0, buf);

					copy_fp (artfp, fp);
					TIN_FCLOSE (artfp);
					fclose (fp);
					saved_arts++;

					if (function == MAIL_ANY_NEWS) {
						strfmailer (mailer, arts[j].subject, mail_news_user, savefile, buf, sizeof (buf), tinrc.mailer_format);
						invoke_cmd (buf);		/* Keep trying after errors */
						unlink (savefile);
					}
					if (catchup)
						art_mark_read (group, &arts[j]);
					break;

				default:
					break;
			}
		}

		if (art_count) {
			if (verbose)
				wait_message (0, _(txt_saved_group), art_count, hot_count,
					PLURAL(art_count, txt_article), group->name);
			unread_news = TRUE;
		}
	}

	switch (function) {
		case CHECK_ANY_NEWS:
			if (unread_news)
				return 2;
			else {
				if (verbose)
					wait_message (1, _(txt_there_is_no_news));
				return 0;
			}
			/* NOTREACHED */

		case START_ANY_NEWS:
			wait_message (1, _(txt_there_is_no_news));
			return -1;
			/* NOTREACHED */

		case MAIL_ANY_NEWS:
		case SAVE_ANY_NEWS:
			sprintf (buf, _(txt_saved_summary), (function == MAIL_ANY_NEWS ? _(txt_mailed) : _(txt_saved)),
					PLURAL(saved_arts, txt_article), PLURAL(group_count, txt_group));
			fprintf (fp_log, "%s", buf);
			if (verbose)
				wait_message (0, buf);

			if (log_opened) {
				fclose (fp_log);
				if (verbose)
					wait_message (0, _("Mailing log to %s\n"), (function == MAIL_ANY_NEWS ? mail_news_user : userid)); /* FIXME: -> lang.c */
				strfmailer (mailer, subject, (function == MAIL_ANY_NEWS ? mail_news_user : userid), logfile, buf, sizeof (buf), tinrc.mailer_format);
				if (invoke_cmd (buf))
					unlink (logfile);
			}
			break;

		default:
			break;
	}

	return 0;
}


/*
 * All article saves use this function
 * Save the article indexed via i and pointed to by artinfo
 * A non-blank 'filename' seems to force a mailbox save
 * 'indexnum' is index into save[]
 * Returns:
 *     TRUE or FALSE depending on whether article was saved okay.
 */
t_bool
save_art_to_file (
	int indexnum,
	t_bool the_mailbox,
	const char *filename,
	t_openartinfo *artinfo)
{
	FILE *fp;
	char *file;
	char mode[3];
	char save_art_info[LEN];
	int ch;
	int i = 0;
	struct stat st;
	t_bool is_mailbox = FALSE;

	if (strlen (filename)) {		/* TODO can this be NULL ? see end of this func */
		is_mailbox = the_mailbox;
		i = indexnum;
	}

	file = save_filename (i);
	strcpy (mode, "a+");

	if (!save[i].is_mailbox) {
		if (stat (file, &st) != -1) {
			if (S_ISDIR(st.st_mode)) {
				wait_message (2, _(txt_cannot_write_to_directory), file);
				return FALSE;
			}

			ch = prompt_slk_response(tinrc.default_save_mode, "aoq\033", _(txt_append_overwrite_quit), file);
			switch (ch) {
				case iKeySaveAppendFile:
					strcpy (mode, "a+");
					break;

				case iKeySaveOverwriteFile:
					strcpy (mode, "w");
					break;

				case iKeyAbort:
				case iKeySaveDontSaveFile2:
/*					save[i].saved = FALSE;*/
					wait_message (1, _(txt_art_not_saved));
					return FALSE;

				default:
					break;
			}
			tinrc.default_save_mode = ch;
		}
	}

#	ifdef DEBUG
	if (debug == 2)
		error_message(_("Save index=[%d] mbox=[%d] filename=[%s] file=[%s] mode=[%s]"), indexnum, the_mailbox, filename, file, mode);
#	endif /* DEBUG */

	if ((fp = fopen (file, mode)) == (FILE *) 0) {
		info_message (_(txt_art_not_saved));
		return FALSE;
	}

	{
		/*
		 * place "^From from date" line as mailbox-seperator
		 * on top of each article
		 */
		char from[HEADER_LEN];
		time_t epoch;

		if (artinfo->hdr.from)
			strip_name (artinfo->hdr.from, from);
		(void) time (&epoch);
		fprintf (fp, "From %s %s", from, ctime (&epoch));
	}

	if (fseek (artinfo->raw, 0L, SEEK_SET) == -1)
		perror_message (_("fseek() error on [%s]"), save[i].subject); /* FIXME: -> lang.c */

	if (copy_fp (artinfo->raw, fp))
		print_art_seperator_line (fp, the_mailbox); /* write tailing newline or MDF-mailbox seperator */

	fclose (fp);

	save[i].saved = TRUE;
	if (tinrc.mark_saved_read)
		art_mark_read (&CURR_GROUP, &arts[save[i].index]);

	if (filename == 0) {
		char *first_savefile;

		sprintf (save_art_info, (is_mailbox ? _(txt_saved_to_mailbox) : _(txt_art_saved_to)), first_savefile = get_first_savefile ());
		FreeIfNeeded(first_savefile);
		info_message (save_art_info);
	}
	return TRUE;
}


/*
 * We return TRUE if any of the articles saved successfully
 */
static t_bool
save_arts (
	t_bool is_mailbox,
	char *group_path)
{
	char file[PATH_LEN];
	int i;
	t_bool ret_code = FALSE;
	t_openartinfo artinfo;

	for (i = 0 ; i < num_save ; i++) {
		/* the tailing spaces are needed for the progress-meter */
		wait_message (0, "%s%d  ", _(txt_saving), i+1);

/* see the TODO file, this bit decides to write to seperate files or not - make it configurable */
		if (is_mailbox)
			file[0] = 0;
		else {
#	ifdef HAVE_LONG_FILE_NAMES
			char tbuf[31]; /* big enough to hold 2^64 + fmt + \0 */
			size_t mlen;

			sprintf(tbuf, "%d", num_save);
			mlen = strlen(tbuf);
			sprintf(tbuf, "%%s.%%0%dd", (int)mlen);
			sprintf(file, tbuf, save[i].file, i+1);
#	else
			sprintf (file, "%s.%03d", save[i].file, i+1);
#	endif /* HAVE_LONG_FILE_NAMES */
		}

		memset (&artinfo, 0, sizeof(t_openartinfo));
		switch (art_open (&arts[save[i].index], group_path, do_rfc1521_decoding, &artinfo)) {

			case ART_ABORT:					/* User 'q'uit */
				return ret_code;

			case ART_UNAVAILABLE:			/* Ignore, just keep going */
				wait_message (1, _(txt_art_unavailable));
				art_mark_read (&CURR_GROUP, &arts[save[i].index]);
				continue;

			default:
				ret_code |= save_art_to_file (i, is_mailbox, file, &artinfo);
				art_close (&artinfo);
		}
	}

	return ret_code;
}


t_bool
save_thread_to_file (
	t_bool is_mailbox,
	char *group_path)
{
	char *first_savefile;
	char buf[LEN];
	t_bool ret_code;

	if (num_save == 0) {
		wait_message(2, _(txt_saved_nothing));
		return FALSE;
	}

	if ((ret_code = save_arts(is_mailbox, group_path))) {
		first_savefile = get_first_savefile ();
		if (first_savefile[0] == '\0')
			info_message (_(txt_thread_not_saved));
		else {
			if (is_mailbox)
				sprintf (buf, _(txt_saved_to_mailbox), first_savefile);
			else {
				if (num_save == 1)
					sprintf (buf, _(txt_art_saved_to), first_savefile);
				else {
					char *last_savefile;

					sprintf (buf, _(txt_thread_saved_to_many), first_savefile, last_savefile = get_last_savefile ());
					FreeIfNeeded(last_savefile);
				}
			}
			wait_message (2, buf);
		}
		FreeIfNeeded(first_savefile);
	}
	return ret_code;
}


t_bool
save_regex_arts_to_file (
	t_bool is_mailbox,
	char *group_path)
{
	char *first_savefile;
	char buf[LEN];
	t_bool ret_code;

	if (num_save == 0) {
		info_message (_(txt_no_match));
		return FALSE;
	}

	if ((ret_code = save_arts(is_mailbox, group_path))) {
		first_savefile = get_first_savefile ();
		if (is_mailbox)
			sprintf (buf, _(txt_saved_to_mailbox), first_savefile);
		else {
			char *last_savefile;

			sprintf (buf, _(txt_saved_pattern_to), first_savefile, last_savefile = get_last_savefile ());
			FreeIfNeeded(last_savefile);
		}

		wait_message (2, "%s", buf);
		FreeIfNeeded(first_savefile);
	}
	return ret_code;
}


/*
 * Parse and expand the supplied path name. Attempt to create
 * the generated path.
 * Return TRUE if it corresponds to a mailbox
 */
t_bool
create_path (
	char *path)
{
	t_bool mbox_format = FALSE;
	char tmp[PATH_LEN];
	char buf[PATH_LEN];
	int i, j, len;
	struct stat st;
	struct t_group currgrp;

	currgrp = CURR_GROUP;

	/*
	 * expand "$var..." first, so variables starting with
	 * '+', '$' or '=' will be processed correctly later
	 */
	if (path[0] == '$') {
		if (strfpath (path, buf, sizeof (buf), homedir, (char *) 0, (char *) 0, currgrp.name))
			my_strncpy (path, buf, PATH_LEN);
	}

	/*
	 * save in mailbox format to ~/Mail/<group.name> or
	 * attribute->maildir for current group
	 */
	if (path[0] == '=') {
		mbox_format = TRUE;
		strcpy (tmp, path);
		if (!strfpath (currgrp.attribute->maildir, buf, sizeof (buf),
		    homedir, (char *) 0, (char *) 0, currgrp.name)) {
#	ifdef VMS
			joindir (buf, homedir, DEFAULT_MAILDIR);
#	else
			joinpath (buf, homedir, DEFAULT_MAILDIR);
#	endif /* VMS */
		}
#	ifdef VMS
		joinpath (path, buf, "dummy");
#	else
		sprintf (path, "%s/dummy", buf);
#	endif /* VMS */
	} else {
		if (!strchr ("~$=+/.", path[0])) {
			if (!strfpath (currgrp.attribute->savedir, buf, sizeof (buf),
			    homedir, (char *) 0, (char *) 0, currgrp.name)) {
#	ifdef VMS
				joindir (buf, homedir, DEFAULT_SAVEDIR);
#	else
				joinpath (buf, homedir, DEFAULT_SAVEDIR);
#	endif /* VMS */
			}
			joinpath (tmp, buf, path);
			my_strncpy (path, tmp, PATH_LEN);
		}
		if (strfpath (path, buf, sizeof (buf), homedir, (char *) 0, currgrp.attribute->savedir, currgrp.name))
			my_strncpy (path, buf, PATH_LEN);
	}

#	ifndef VMS			/* no good answer to this yet XXX */
	/*
	 *  create any directories, otherwise check
	 *  errno and give appropiate error message
	 */
	len = (int) strlen (path);

	for (i = 0, j = 0; i < len; i++, j++) {
		buf[j] = path[i];
		if (i+1 < len && path[i+1] == '/') {
			buf[j+1] = '\0';
			if (stat (buf, &st) == -1) {
				if (my_mkdir (buf, (mode_t)(S_IRWXU|S_IRUGO|S_IXUGO)) == -1) {
					if (errno != EEXIST) {
						perror_message (_(txt_cannot_create), buf);
						return FALSE;
					}
				}
			}
		}
	}
#	else
	if (my_mkdir (buf, (mode_t)(S_IRWXU|S_IRUGO|S_IXUGO)) == -1) {
		if (errno != EEXIST) {
			perror_message (_(txt_cannot_create), buf);
			return FALSE;
		}
	}
#	endif /* !VMS */

	if (mbox_format)
		strcpy (path, tmp);

	return mbox_format;
}


static t_bool
create_sub_dir (
	int i)
{
	char dir[LEN];
	struct stat st;

	if (!save[i].is_mailbox && save[i].archive) {
		joinpath (dir, save[i].dir, save[i].archive);
		if (stat (dir, &st) == -1) {
			my_mkdir (dir, (mode_t)(S_IRWXU|S_IRUGO|S_IXUGO));
			return TRUE;
		}
		return ((S_ISDIR(st.st_mode)) ? TRUE : FALSE);
	}
	return FALSE;
}


/*
 * Add files to be saved to save array
 */
void
add_to_save_list (
	int the_index,
	t_bool is_mailbox,
	const char *path)
{
	char tmp[PATH_LEN];
	char dir[PATH_LEN];
	char file[PATH_LEN];
	struct t_article *artptr = &arts[the_index];

	dir[0] = '\0';
	file[0] = '\0';

	if (num_save == max_save-1)
		expand_save ();

	save[num_save].subject = my_strdup (artptr->subject);
	save[num_save].dir = NULL;
	save[num_save].file = NULL;
	save[num_save].archive = NULL;
	save[num_save].part = NULL;
	save[num_save].patch = NULL;
	save[num_save].index = the_index;
	save[num_save].saved = FALSE;
	save[num_save].is_mailbox = is_mailbox;

	if (CURR_GROUP.attribute->auto_save && artptr->archive) {
		save[num_save].archive = my_strdup (artptr->archive);
		if (artptr->part)
			save[num_save].part = my_strdup (artptr->part);
		if (artptr->patch)
			save[num_save].patch = my_strdup (artptr->patch);
	}

	if (is_mailbox) {
		my_strncpy (file, ((strlen (path) > 1) ? ((path[0] == '=') ? (path+1) : path) : glob_group), sizeof (file));

		if (!strfpath (CURR_GROUP.attribute->maildir, tmp, sizeof (tmp),
		    homedir, (char *) 0, (char *) 0, CURR_GROUP.name)) {
#	ifdef VMS
			joindir (tmp, homedir, DEFAULT_MAILDIR);
#	else
			joinpath (tmp, homedir, DEFAULT_MAILDIR);
#	endif /* VMS */
		}
		save[num_save].dir = my_strdup (tmp);
		save[num_save].file = my_strdup (file);
	} else {
		if (*path) {
#	ifdef VMS
#	include "parse.h"
			struct filespec *spec;

			spec = sysparse(path);
			sprintf(dir, "%s%s", spec->dev, spec->dir);
			strcpy(file, spec->filename);
#	else
			int i;

			for (i = strlen (path); i; i--) {
#		ifdef WIN32
				/*
				 * Under WIN32, paths can be in form D:\a\b\c\file. Optionally,
				 * User can override the default, so we need to deal with input in
				 * Form: D:\a\b\c\F:\x\y\article or D:\a\b\c\\x\y\article.
				 * In these cases, we want to use F:\x\y\article or \x\y\article
				 * as the basepath.
				 */
				if (path[i] == '\\') {
					int j;
					for (j = i - 1; j; j--) {
						if (path[j] == '\\' && path[j+1] == '\\') {
							if (i-j-1 == 0)
								strcpy(dir, "\\");
							else {
								strncpy (dir, &path[j+1], (size_t)(i-j-1));
								dir[i-j-1] = '\0';
							}
							break;
						}
						else if (path[j] == ':') {
							strncpy (dir, &path[j-1], (size_t)(i-j+1));
							dir[i-j+1] = '\0';
							break;
						}
					}
					strcpy (file, path+i+1);
					break;
				}
#		else
				if (path[i] == '/') {
					strncpy (dir, path, (size_t)i);
					dir[i] = '\0';
					strcpy (file, path+i+1);
					break;
				}
#		endif /* WIN32 */
			}
#	endif /* VMS */
		}

#	ifdef M_AMIGA
		if (tin_bbs_mode)
			dir[0] = 0;
#	endif /* M_AMIGA */

		if (*dir)
			save[num_save].dir = my_strdup (dir);
		else {
			if (!strfpath (CURR_GROUP.attribute->savedir, tmp, sizeof (tmp),
			    homedir, (char *) 0, (char *) 0, CURR_GROUP.name)) {
#	ifdef VMS
				joindir (tmp, homedir, DEFAULT_SAVEDIR);
#	else
				joinpath (tmp, homedir, DEFAULT_SAVEDIR);
#	endif /* VMS */
			}
			save[num_save].dir = my_strdup (tmp);
		}

		save[num_save].file = my_strdup (*file ? file : (*path ? path : save[num_save].archive));
	}
/*fprintf(stderr, "Really save in !%s!\n", save[num_save].file);*/
	num_save++;
}


/*
 *  string comparison routine for the qsort()
 *  ie. qsort(array, 5, 32, save_comp);
 */
static int
save_comp (
	t_comptype p1,
	t_comptype p2)
{
	const struct t_save *s1 = (const struct t_save *) p1;
	const struct t_save *s2 = (const struct t_save *) p2;

	/*
	 * Sort on Archive-name: part & patch otherwise Subject:
	 */
	if (s1->archive != 0) {
		if (s1->part != 0) {
			if (s2->part != 0) {
				if (strcmp (s1->part, s2->part) < 0)
					return -1;
				if (strcmp (s1->part, s2->part) > 0)
					return 1;
			} else
				return 0;
		} else if (s1->patch != 0) {
			if (s2->patch != 0) {
				if (strcmp (s1->patch, s2->patch) < 0)
					return -1;
				if (strcmp (s1->patch, s2->patch) > 0)
					return 1;
			} else
				return 0;
		}
	} else {
		if (strcmp (s1->subject, s2->subject) < 0)
			return -1;
		if (strcmp (s1->subject, s2->subject) > 0)
			return 1;
	}

	return 0;
}


/*
 *  Print save array of files to be saved
 */
void
sort_save_list (
	void)
{
	qsort ((char *) save, (size_t)num_save, sizeof (struct t_save), save_comp);
#ifdef DEBUG
	debug_save_comp ();
#endif /* DEBUG */
}


/*
 * Get the full path/filename of corresponding to save[i]
 */
static char *
save_filename (
	int i)
{
	char *p;
	static char *filename;

	FreeIfNeeded(filename);	/* leak only the last instance */
#	ifdef DOALLOC
	if (i < 0)
		return 0;
#	endif /* DOALLOC */
	filename = (char *) my_malloc(PATH_LEN);

	if (save[i].is_mailbox) {
		joinpath (filename, save[i].dir, save[i].file);
		return filename;
	}

	if (!tinrc.auto_save || (!(save[i].part || save[i].patch))) {
		joinpath (filename, save[i].dir, save[i].file);
		if (num_save != 1) {
#	ifdef VMS
			sprintf (&filename[strlen(filename)], "-%03d", i+1);
#	else
#		ifdef HAVE_LONG_FILE_NAMES
			char tbuf[31]; /* big enough to hold 2^64 + fmt + \0 */
			size_t mlen;

			sprintf(tbuf, "%d", num_save);
			mlen = strlen(tbuf);
			sprintf(tbuf, ".%%0%dd", (int)mlen);
			sprintf(&filename[strlen(filename)], tbuf, i+1);
#		else
			sprintf (&filename[strlen(filename)], ".%03d", i+1);
#		endif /* HAVE_LONG_FILE_NAMES */
#	endif /* VMS */
		}
	} else if ((p = save[i].part) || (p = save[i].patch)) {
		joinpath (filename, save[i].dir, save[i].archive);
		if (create_sub_dir (i)) {
#	ifdef VMS
			sprintf (&filename[strlen(filename)], "%s.%s%s", save[i].archive, LONG_PATH_PART, p);
#	else
			sprintf (&filename[strlen(filename)], "/%s.%s%s", save[i].archive, LONG_PATH_PART, p);
#	endif /* VMS */
		} else {
#	ifdef VMS
			sprintf (&filename[strlen(filename)], "%s%s", LONG_PATH_PART, p);
#	else
			sprintf (&filename[strlen(filename)], ".%s%s", LONG_PATH_PART, p);
#	endif /* VMS */
		}
	}

	return filename;
}


/*
 * Get the filename portion only of save[i]
 * (for reporting purposes only)
 */
static char *
get_save_filename (
	int i)
{
	char *file = (char *) my_malloc(PATH_LEN);

	if (save[i].is_mailbox) {
#	ifdef VMS
		joinpath (file, save[i].dir, save[i].file);
#	else
		sprintf (file, "%s/%s", save[i].dir, save[i].file);
#	endif /* VMS */
		return file;
	}

	if (save[i].archive && tinrc.auto_save) {
		if (save[i].part) {
#	ifdef VMS
			char fbuf[256];
			sprintf(fbuf, "%s.%s%s", save[i].archive, LONG_PATH_PART, save[i].part);
			joinpath(file, save[i].archive, fbuf);
#	else
			sprintf (file, "%s/%s.%s%s", save[i].archive, save[i].archive, LONG_PATH_PART, save[i].part);
#	endif /* VMS */
		} else {
#	ifdef VMS
			char fbuf[256];
			sprintf(fbuf, "%s.%s%s", save[i].archive, LONG_PATH_PATCH, save[i].patch);
			joinpath(file, save[i].archive, fbuf);
#	else
			sprintf (file, "%s/%s.%s%s", save[i].archive, save[i].archive, LONG_PATH_PATCH, save[i].patch);
#	endif /* VMS */
		}
	} else {
		if (num_save == 1)
			sprintf (file, "%s", save[i].file);
		else {
#	ifdef VMS
			sprintf (file, "%s-%03d", save[i].file, i+1);
#	else
#		ifdef HAVE_LONG_FILE_NAMES
			char tbuf[31]; /* big enough to hold 2^64 + fmt + \0 */
			size_t mlen;

			sprintf(tbuf, "%d", num_save);
			mlen = strlen(tbuf);
			sprintf(tbuf, "%%s.%%0%dd", (int)mlen);
			sprintf(file, tbuf, save[i].file, i+1);
#		else
			sprintf (file, "%s.%03d", save[i].file, i+1);
#		endif /* HAVE_LONG_FILE_NAMES */
#	endif /* VMS */
		}
	}
	return file;
}


static char *
get_first_savefile (
	void)
{
	static char empty[] = "";
	int i;

	for (i = 0; i < num_save; i++) {
		if (save[i].saved)
			return (get_save_filename(i));
	}
	return empty;
}


static char *
get_last_savefile (
	void)
{
	static char empty[] = "";
	int i;

	for (i = num_save-1; i >= 0; i--) {
		if (save[i].saved)
			return (get_save_filename(i));
	}
	return empty;
}


t_bool
post_process_files (
	int proc_type_ch,
	t_bool auto_delete)
{
	if (!any_saved_files ())
		return FALSE;

	wait_message (0, _(txt_post_processing));

	switch (proc_type_ch) {
		case iKeyPProcShar:
			post_process_sh (auto_delete);
			break;

		case iKeyPProcUUDecode:
			post_process_uud (
#	ifndef HAVE_LIBUU
			POST_PROC_UUDECODE,
#	endif /* !HAVE_LIBUU */
			auto_delete);
			break;

		case iKeyPProcListZoo:
			post_process_uud (
#	ifndef HAVE_LIBUU
			POST_PROC_UUD_LST_ZOO,
#	endif /* !HAVE_LIBUU */
			auto_delete);
			break;

		case iKeyPProcExtractZoo:
			post_process_uud (
#	ifndef HAVE_LIBUU
			POST_PROC_UUD_EXT_ZOO,
#	endif /* !HAVE_LIBUU */
			auto_delete);
			break;

		case iKeyPProcListZip:
			post_process_uud (
#	ifndef HAVE_LIBUU
			POST_PROC_UUD_LST_ZIP,
#	endif /* !HAVE_LIBUU */
			auto_delete);
			break;

		case iKeyPProcExtractZip:
			post_process_uud (
#	ifndef HAVE_LIBUU
			POST_PROC_UUD_EXT_ZIP,
#	endif /* !HAVE_LIBUU */
			auto_delete);
			break;

		default:
			break;
	}

	wait_message (1, _(txt_post_processing_finished));
	return TRUE;
}


static void
post_process_uud (
#	ifndef HAVE_LIBUU
	int pp,
#	endif /* !HAVE_LIBUU */
	t_bool auto_delete)
{
	FILE *fp_in;
	char t[LEN], u[LEN];
	char buf[LEN];
	char file_out[PATH_LEN];
	char file_out_dir[PATH_LEN];
	int i, state;
	int open_out_file;
#	ifndef HAVE_LIBUU
	FILE *fp_out = NULL;
	char s[LEN];
#	else
	int errors = 0;
	uulist *item;
#	endif /* !HAVE_LIBUU */

	t[0] = '\0';
	u[0] = '\0';

	my_strncpy (file_out_dir, save_filename (0), sizeof (file_out_dir));
	for (i = strlen(file_out_dir); i > 0; i--) {
#	ifdef VMS
		if (file_out_dir[i] == ']') {
			file_out_dir[i+1] = '\0';
			break;
		}
#	else
		if (file_out_dir[i] == '/') {
			file_out_dir[i] = '\0';
			break;
		}
#	endif /* VMS */
	}

#	ifdef HAVE_LIBUU
	UUInitialize();

	/* Blatant unixism here! */
	sprintf(file_out, "%s/", file_out_dir);

	/* Clumsy, but file_out is already declared, so might as well use it */
	UUSetOption(UUOPT_SAVEPATH, 0, file_out);
	for (i = 0; i < num_save; i++) {
		if (!save[i].saved)
			continue;

		my_strncpy (buf, save_filename(i), sizeof (buf));
		if ((fp_in = fopen (buf, "r")) != (FILE *)NULL) {
			UULoadFile(buf, NULL, 0);
			fclose(fp_in);
		}
	}

#	if 0 /* uudeview's "intelligent" multi-part detection */
	UUSmerge (0);
	UUSmerge (1);
	UUSmerge (99);
#	endif /* 0 */

	i = open_out_file = 0;
	item = UUGetFileListItem(i);
	my_printf(cCRLF);
	while (item != NULL) {
		state = UUDecodeFile(item, NULL);
		if (state == UURET_OK) {
			/* open_out_file already declared, might as well use it */
			open_out_file++;
			my_printf(_(txt_libuu_success), item->filename);
			my_printf(cCRLF);

		} else {
			errors++;
			if (item->filename == NULL) {
				my_printf(_(txt_libuu_error_decode), item->subfname);
				my_printf(cCRLF);
			}
			else {
				my_printf(_(txt_libuu_error_decode), item->filename);
				my_printf(cCRLF);
			}

			if (item->state & UUFILE_MISPART) {
				my_printf(_(txt_libuu_error_missing));
				my_printf(cCRLF);
			} else if (item->state & UUFILE_NOBEGIN) {
				my_printf(_(txt_libuu_error_no_begin));
				my_printf(cCRLF);
			} else if (item->state & UUFILE_NOEND) {
				my_printf(_(txt_libuu_error_no_end));
				my_printf(cCRLF);
			} else if (item->state & UUFILE_NODATA) {
				my_printf(_(txt_libuu_error_no_data));
				my_printf(cCRLF);
			} else {
				my_printf(_(txt_libuu_error_unknown));
				my_printf(cCRLF);
			}
		}
		i++;
		item = UUGetFileListItem(i);
		my_flush();
	}
	my_printf(_(txt_libuu_saved), open_out_file, num_save, errors, PLURAL(errors, txt_error));
	my_printf(cCRLF);
	UUCleanUp();
	delete_processed_files (auto_delete); /* TRUE = auto-delete files */
	return;
#	else
#		ifdef VMS
	sprintf (file_out, "%suue.%05d", file_out_dir, (int) process_id);
#		else
	sprintf (file_out, "%s/uue%05d", file_out_dir, (int) process_id);
#		endif /* VMS */
	state = INITIAL;
	open_out_file = TRUE;

	for (i = 0; i < num_save; i++) {
		char realname[130]; /* don't use PATH_LEN if you use an absolut val. below as you can't say that PATH_LEN is big enought */

		if (!save[i].saved)
			continue;
		if (open_out_file) {
			if ((fp_out = fopen (file_out, "w")) == (FILE *) 0) {
				perror_message (_(txt_cannot_open), file_out);
				unlink (file_out);
				return;
			}
			open_out_file = FALSE;
		}

		my_strncpy (buf, save_filename (i), sizeof (buf));
		if ((fp_in = fopen (buf, "r")) != (FILE *) 0) {
			if (fgets (s, (int) sizeof(s), fp_in) == 0) {
				fclose (fp_in);
				continue;
			}
			while (state != END) {
				switch (state) {
					case INITIAL:
						if (!strncmp ("begin ", s, 6)) {
							if (sscanf (s+6, "%*o %128c\n", realname) != 1)	/* Get the real filename */
								realname[0] = '\0';
							else
								strtok (realname, "\n");
							realname[129] = '\0';
							state = MIDDLE;
							fprintf (fp_out, "%s", s);
						}
						break;

					case MIDDLE:
						if (s[0] == 'M') {
							fprintf (fp_out, "%s", s);
						} else if (strncmp("end", s, 3)) {
							state = OFF;
						} else { /* end */
							state = END;
							if (u[0] != 'M')
								fprintf (fp_out, "%s", u);
							if (t[0] != 'M')
								fprintf (fp_out, "%s", t);
							fprintf (fp_out, "%s\n", s);
						}
						break;

					case OFF:
						if ((s[0] == 'M') && (t[0] == 'M') && (u[0] == 'M')) {
							fprintf (fp_out, "%s", u);
							fprintf (fp_out, "%s", t);
							fprintf (fp_out, "%s", s);
							state = MIDDLE;
						} else if (STRNCMPEQ("end", s, 3)) {
							state = END;
							if (u[0] != 'M')
								fprintf (fp_out, "%s", u);
							if (t[0] != 'M')
								fprintf (fp_out, "%s", t);
							fprintf (fp_out, "%s\n", s);
						}
						break;

					case END:
						break;

					default:
						my_fprintf (stderr, "%s %s", cCRLF, _("Error: ASSERT - default state\n")); /* FIXME: -> lang.c */
						fclose (fp_in);
						fclose (fp_out);
						unlink (file_out);
						return;
				}
				strcpy (u, t);
				strcpy (t, s);

				/*
				 *  read next line & if error goto next file in save array
				 */
				if (fgets (s, (int) sizeof(s), fp_in) == 0)
					break;
			}
			fclose (fp_in);
		}
		if (state == END) {
			fclose (fp_out);
			uudecode_file (pp, file_out_dir, file_out, realname);
			state = INITIAL;
			open_out_file = TRUE;
		}
	}

	delete_processed_files (auto_delete); /* TRUE = auto-delete files */
	unlink (file_out);
#	endif /* HAVE_LIBUU */
}


/*
 *  uudecode a single file
 */
#	ifndef HAVE_LIBUU
static void
uudecode_file (
	int pp,
	char *file_out_dir,
	char *file_out,
	char *uudname)
{
	FILE *fp_in;
	char buf[LEN];

	wait_message (0, _(txt_uudecoding), file_out);

#		ifndef M_UNIX
	make_post_process_cmd (DEFAULT_UUDECODE, file_out_dir, file_out);
#		else
	chdir (file_out_dir);

/*	sh_format (buf, sizeof(buf), "uudecode %s -o \"%s\"", file_out, uudname);*/
	sh_format (buf, sizeof(buf), "uudecode %s", file_out);
	if (!invoke_cmd (buf))
		return;

	if (uudname[0] == '\0')					/* Unable to determine the file that was uudecoded */
		return;

	/*
	 *  Sum file
	 */
	sh_format (buf, sizeof(buf), "%s \"%s\"", DEFAULT_SUM, uudname);
	if ((fp_in = popen (buf, "r")) != (FILE *) 0) {
		if (fgets (buf, (int) sizeof(buf), fp_in) != 0) {
			char *ptr = strchr (buf, '\n');
			if (ptr != 0)
				*ptr = '\0';
		}
		pclose (fp_in);
		my_printf (cCRLF);
		my_printf (_(txt_checksum_of_file), uudname);
		my_printf (cCRLF);
		my_flush ();
		my_printf ("%s  %10ld %s %s %s", buf, file_size (uudname), _("bytes"), cCRLF, cCRLF);
	} else
		my_printf ("%s %s %s", _(txt_command_failed), buf, cCRLF);

	my_flush ();

	/*
	 * If defined, invoke post processor command
	 */
	if (*tinrc.post_process_command) {
		sh_format (buf, sizeof(buf), "%s '%s'", tinrc.post_process_command, uudname);
		invoke_cmd (buf);
		(void) sleep (1);
	}

	if (pp > POST_PROC_UUDECODE) {
		int i;
		/*
		 *  Test archive integrity
		 */
		if (pp > POST_PROC_UUDECODE && archiver[pp].test != 0) {
			i = (pp == POST_PROC_UUD_LST_ZOO || pp == POST_PROC_UUD_EXT_ZOO ? 3 : 4);
			sh_format (buf, sizeof(buf), "%s %s \"%s\"", archiver[i].name, archiver[i].test, uudname);
			my_printf(cCRLF cCRLF);
			my_printf (_(txt_testing_archive), uudname);
			my_printf(cCRLF);
			my_flush ();
			if (!invoke_cmd (buf))
				error_message (_(txt_post_processing_failed));
			(void) sleep (3);
		}
		/*
		 *  List archive
		 */
		if (pp == POST_PROC_UUD_LST_ZOO || pp == POST_PROC_UUD_LST_ZIP) {
			i = (pp == POST_PROC_UUD_LST_ZOO ? 3 : 4);
			sh_format (buf, sizeof(buf), "%s %s %s", archiver[i].name, archiver[i].list, uudname);
			my_printf (cCRLF cCRLF);
			my_printf (_(txt_listing_archive), uudname);
			my_printf (cCRLF);
			my_flush ();
			if (!invoke_cmd (buf))
				error_message (_(txt_post_processing_failed));
			(void) sleep (3);
		}
		/*
		 *  Extract archive
		 */
		if (pp == POST_PROC_UUD_EXT_ZOO || pp == POST_PROC_UUD_EXT_ZIP) {
			i = (pp == POST_PROC_UUD_EXT_ZOO ? 3 : 4);
			sh_format (buf, sizeof(buf), "%s %s %s", archiver[i].name, archiver[i].extract, uudname);
			my_printf (cCRLF cCRLF);
			my_printf (_(txt_extracting_archive), uudname);
			my_printf (cCRLF);
			my_flush ();
			if (!invoke_cmd (buf))
				error_message (_(txt_post_processing_failed));
			(void) sleep (3);
		}
	}
#		endif /* ! M_UNIX */
}
#	endif /* !HAVE_LIBUU */


/*
 *  Unpack /bin/sh archives
 */
static void
post_process_sh (
	t_bool auto_delete)
{
	FILE *fp_in, *fp_out;
	char *ptr1, *ptr2, *ptr3;
	char buf[LEN];
	char file_in[PATH_LEN];
	char file_out[PATH_LEN];
	char file_out_dir[PATH_LEN];
	char sh_pattern_1[16];
	char sh_pattern_2[16];
	char sh_pattern_3[64];
	int i, j;
	t_bool found_header;

	strcpy (sh_pattern_1, "# !/bin/sh");
	strcpy (sh_pattern_2, "#!/bin/sh");
	strcpy (sh_pattern_3, "# This is a shell archive.");

	my_strncpy (file_out_dir, save_filename (0), sizeof (file_out_dir));
	for (i = strlen(file_out_dir); i > 0; i--) {
#	ifdef VMS
		if (file_out_dir[i] == ']') {
			file_out_dir[i+1] = '\0';
			break;
		}
#	else
		if (file_out_dir[i] == '/') {
			file_out_dir[i] = '\0';
			break;
		}
#	endif /* VMS */
	}

#	ifdef VMS
	sprintf (file_out, "%ssh.%05d", file_out_dir, (int) process_id);
#	else
	sprintf (file_out, "%s/sh%05d", file_out_dir, (int) process_id);
#	endif /* VMS */

	for (j = 0; j < num_save; j++) {
		if (!save[j].saved)
			continue;
		my_strncpy (file_in, save_filename (j), sizeof (file_in));

		my_printf (cCRLF);
		my_printf (_(txt_extracting_shar), file_in);
		my_printf (cCRLF);
		my_flush ();

		found_header = FALSE;

		if ((fp_out = fopen (file_out, "w" FOPEN_OPTS)) != NULL) {
			if ((fp_in = fopen (file_in, "r")) != NULL) {
				ptr1 = sh_pattern_1;
				ptr2 = sh_pattern_2;
				ptr3 = sh_pattern_3;

				while (!feof (fp_in)) {
					if (fgets (buf, (int) sizeof(buf), fp_in)) {
						/*
						 *  find #!/bin/sh or #!/bin/sh pattern
						 */
						if (!found_header && (strstr (buf, ptr1) != 0 || strstr (buf, ptr2) != 0 || strstr (buf, ptr3) != 0))
							found_header = TRUE;

						/*
						 *  Write to temp file
						 */
						if (found_header)
							fputs (buf, fp_out);
					}
				}
				fclose (fp_in);
			}
			fclose (fp_out);

#	ifndef M_UNIX
			make_post_process_cmd (DEFAULT_UNSHAR, file_out_dir, file_out);
#	else
			sh_format (buf, sizeof(buf), "cd %s; sh %s", file_out_dir, file_out);
			my_fputs (cCRLF, stdout);
			my_flush ();
			Raw (FALSE);
			if (!invoke_cmd (buf))
				error_message (_(txt_command_failed), buf);		/* TODO not needed */
			Raw (TRUE);
#	endif /* ! M_UNIX */
			unlink (file_out);
		}
	}
	delete_processed_files (auto_delete);
}


static void
delete_processed_files (
	t_bool auto_delete)
{
	int i;
	t_bool delete_it = FALSE;

	if (any_saved_files ()) {
		if (CURR_GROUP.attribute->delete_tmp_files || auto_delete)
			delete_it = TRUE;
		else if (prompt_yn (cLINES, _(txt_delete_processed_files), TRUE) == 1)
			delete_it = TRUE;

		if (delete_it) {
			wait_message (0, "%s%s", _(txt_deleting), cCRLF);

			for (i = 0; i < num_save; i++)
				unlink (save_filename (i));
		}
	}
}


static t_bool
any_saved_files (
	void)
{
	int i;

	for (i = 0; i < num_save; i++) {
		if (save[i].saved)
			return TRUE;
	}

	return FALSE;
}


/*
 * write tailing (MDF)-mailbox seperator
 */
void
print_art_seperator_line (
	FILE *fp,
	t_bool is_mailbox)
{
	char sep = '\1';	/* Ctrl-A */
#ifdef DEBUG
	if (debug == 2)
		error_message (_("Mailbox=[%d]  MMDF=[%d]"), is_mailbox, tinrc.save_to_mmdf_mailbox);
#endif /* DEBUG */

	if (is_mailbox && tinrc.save_to_mmdf_mailbox)
		fprintf (fp, "%c%c%c%c\n", sep, sep, sep, sep);
	else
		my_fputc ('\n', fp);
}


static void
start_viewer(
	char *app,
	char *path)
{
	char *ptr, *toptr;
	char viewer[PATH_LEN];
	t_bool quote = FALSE;
	t_bool backquote = FALSE;
	t_bool percent = FALSE;

	/*
	 * Split off the command name - don't split on backquoted ; or ; in " "
	 * Substitute %s with the filename - again don't substitute if in \ or "
	 */
	for (ptr = app, toptr = viewer; *ptr; ptr++) {
		if (*ptr == '\"')
			quote = !quote;
		else if (*ptr == '\\')
			backquote = TRUE;
		else if (*ptr == '%' && !backquote && !quote)
			percent = TRUE;
		else if (*ptr == ';' && !backquote && !quote)
			break;
		else if (*ptr == 's' && percent) {
			sprintf (toptr-1, "\"%s\"", path);
			toptr += strlen(path) + 1;
		} else if (backquote)
			backquote = FALSE;
		else if (percent)
			percent = FALSE;

		if (!(*ptr == 's' && percent))
			*toptr++ = *ptr;
	}
	*toptr = '\0';

	wait_message(1, "Starting %s\n", viewer);
	system(viewer);
}


#define HAVE_UUDECODE 1
#ifdef HAVE_UUDECODE
	static void uudecode_line(char *buf, FILE *fp);
#endif /* HAVE_UUDECODE */

static void
decode_save_one(
	t_part *ptr,
	FILE *rawfp)
{
	char buf[2048], buf2[2048];
	char savepath[PATH_LEN];
	char *name;
	char *savefile;				/* ptr to filename portion of savepath */
	int i;
	struct t_attribute *attr = CURR_GROUP.attribute;
	FILE *fp;

	/*
	 * Determine the filename
	 * FIXME: use joinpath()
	 *        honor, that tinrc.default_save_file holds the filename
	 *        _and_ the path (-> no path prepending needed)
	 */
	strncpy (savepath, attr->savedir, sizeof(savepath));
	strcat (savepath, "/");
	savefile = savepath + strlen(savepath);
	if ((name = get_filename(ptr->params)) != NULL)
		strcat (savepath, name);
	else
		strcat (savepath, attr->savefile ? attr->savefile : tinrc.default_save_file);

	/*
	 * Decode/save the attachment
	 */
/* TODO don't overwrite by default */
	if ((fp = fopen (savepath, "w")) == NULL) {
		error_message ("Couldn't open %s for saving", savepath);
		return;
	}

	if (ptr->encoding == ENCODING_BASE64)
		mmdecode(NULL, 'b', 0, NULL, NULL);				/* flush */

	fseek(rawfp, ptr->offset, SEEK_SET);

	for (i = 0; i < ptr->lines ; i++) {
		if ((fgets(buf, sizeof(buf), rawfp)) == NULL)
			break;

		/* This should catch cases where people illegally append text etc */
		if (buf[0] == '\0')
			break;

		switch (ptr->encoding) {
			int count;

			case ENCODING_QP:
			case ENCODING_BASE64:
				count = mmdecode(buf, ptr->encoding == ENCODING_QP ? 'q' : 'b', '\0', buf2, NULL);
				fwrite(buf2, count, 1, fp);
				break;

			case ENCODING_UUE:
#ifdef HAVE_UUDECODE
				/*
				 * x-uuencode attatchments have all the header info etc which we must ignore
				 */
				if (strncmp(buf, "begin ", 6) != 0 && strncmp(buf, "end\n", 4) != 0 && buf[0] != '\n')
					uudecode_line (buf, fp);
				break;
#else
				wait_message(1, "x-uuencode not supported yet");
				/* FALLTHROUGH */
#endif /* HAVE_UUDECODE */
			default:
				fputs(buf, fp);
		}
	}
	fclose(fp);

	sprintf(buf, "View '%s' (%s/%s) ? (y/n): ", savefile, content_types[ptr->type], ptr->subtype);
	if (prompt_yn (cLINES, buf, FALSE) == 1) {
		char *app;

		if ((app = lookup_mailcap (ptr->type, ptr->subtype)) != NULL) {
			start_viewer (app, savepath);
			free (app);
			/* TODO redraw screen if (needstermainal) */
		} else
			wait_message (2, "No viewer found for %s/%s\n", content_types[ptr->type], ptr->subtype);
	}

	sprintf(buf, "Save '%s' (%s/%s) ? (y/n): ", savefile, content_types[ptr->type], ptr->subtype);
	if (prompt_yn (cLINES, buf, FALSE) != 1)
		unlink (savepath);
}


/*
 * decode and save a binary MIME attachment
 * optionally locate and launch a viewer application
 */
void
decode_save_mime(
	t_openartinfo *art)
{
	t_part *ptr;

	/*
	 * Iterate over all the attachments
	 */
	for (ptr = art->hdr.ext; ptr != NULL; ptr = ptr->next) {
		t_part *uueptr;

		/*
		 * Handle uuencoded sections in this message part
		 */
		for (uueptr = ptr->uue; uueptr != NULL; uueptr = uueptr->next)
			decode_save_one (uueptr, art->raw);

		/*
		 * Decode this message part if appropriate
		 */
		if (ptr->type == TYPE_MULTIPART || IS_PLAINTEXT(ptr))
			continue;

		decode_save_one (ptr, art->raw);
	}
}


#ifdef HAVE_UUDECODE
/* Single character decode.  */
#define	DEC(Char) (((Char) - ' ') & 077)
static void
uudecode_line(
	char *buf,
	FILE *fp) 
{
	char *p = buf;
	int n;

	n = DEC (*p);
#ifdef DEBUG_ART
	fprintf(stderr, "check=%2d of:    %s", n, p);
#endif /* DEBUG_ART */
	for (++p; n > 0; p += 4, n -= 3) {
		char ch;
		if (n >= 3) {
			ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
			fputc (ch, fp);
			ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
			fputc (ch, fp);
			ch = DEC (p[2]) << 6 | DEC (p[3]);
			fputc (ch, fp);
		} else {
			if (n >= 1) {
				ch = DEC (p[0]) << 2 | DEC (p[1]) >> 4;
				fputc (ch, fp);
			}
			if (n >= 2) {
				ch = DEC (p[1]) << 4 | DEC (p[2]) >> 2;
				fputc (ch, fp);
			}
		}			
	}
	return;
}
#endif /* HAVE_UUDECODE */
