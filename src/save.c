/*
 *  Project   : tin - a Usenet reader
 *  Module    : save.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2002-10-01
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
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef RFC2046_H
/* #	include "rfc2046.h" */
#endif /* !RFC2046_H */

#ifdef HAVE_UUDEVIEW_H
#	ifndef __UUDEVIEW_H__
#		include <uudeview.h>
#	endif /* !__UUDEVIEW_H__ */
#endif /* HAVE_UUDEVIEW_H */

#undef OFF

enum state { INITIAL, MIDDLE, OFF, END };
static const char DIRSEP = SEPDIR;		/* Seperator between dir + filename */

/*
 * Local prototypes
 */
static FILE *open_save_filename(const char *path, t_bool mbox);
static const char *get_first_savefile(void);
static const char *get_last_savefile(void);
static int save_arts(const char *group_path);
static t_bool any_saved_files(void);
static t_bool get_save_filename(char *outpath, const char *path);
static void decode_save_one(t_part *part, FILE *rawfp, t_bool postproc);
static void delete_processed_files(t_bool auto_delete);
static void post_process_sh(t_bool auto_delete);
static void post_process_uud(t_bool auto_delete);
static void start_viewer(t_part *part, const char *path);
static void uudecode_line(const char *buf, FILE *fp);
#ifndef HAVE_LIBUU
	static void sum_and_view(const char *path, const char *file);
#endif /* !HAVE_LIBUU */
#if 0
	static int save_comp(t_comptype p1, t_comptype p2);
#endif /* 0 */


/*
 * Check for articles and say how many new/unread in each group.
 * or
 * Start if new/unread articles and return first group with new/unread.
 * or
 * Save any new articles to savedir and mark arts read and mail user
 * and inform how many arts in which groups were saved.
 * or
 * Mail any new articles to specified user and mark arts read and mail
 * user and inform how many arts in which groups were mailed.
 * Return codes:
 * 	CHECK_ANY_NEWS	- code to pass to exit() - see manpage for list
 * 	START_ANY_NEWS	- index in my_group of first group with unread news or -1
 * 	MAIL_ANY_NEWS	- not checked
 * 	SAVE_ANY_NEWS	- not checked
 */
int
check_start_save_any_news(
	int function,
	t_bool catchup)
{
	FILE *artfp;
	FILE *fp;
	FILE *fp_log = (FILE *) 0;
	char buf[LEN], logfile[LEN];
	char group_path[PATH_LEN];
	char path[PATH_LEN];
	char savefile[PATH_LEN];
	char subject[HEADER_LEN];
	int group_count;
	int i, j;
	int art_count, hot_count;
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
				wait_message(0, _(txt_checking_for_news));
			break;

		case MAIL_ANY_NEWS:
		case SAVE_ANY_NEWS:
#ifdef VMS
			joinpath(logfile, rcdir, "log.");
#else
			joinpath(logfile, rcdir, "log");
#endif /* VMS */
			if (no_write || (fp_log = fopen(logfile, "w" FOPEN_OPTS)) == NULL) {
				perror_message(_(txt_cannot_open), logfile);
				fp_log = stdout;
				verbose = FALSE;
				log_opened = FALSE;
			}
			fprintf(fp_log, "To: %s\n", userid);
			(void) time(&epoch);
			snprintf(subject, sizeof(subject) - 1, "Subject: NEWS LOG %s", ctime(&epoch));
			fprintf(fp_log, "%s\n", subject);	/* ctime() includes a \n too */
			break;

		default:
			break;
	}

	group_count = 0;

	/*
	 * For each group we subscribe to...
	 */
	for (i = 0; i < selmenu.max; i++) {
		art_count = hot_count = 0;
		group = &active[my_group[i]];

		if (!index_group(group))
			continue;

		make_group_path(group->name, group_path);

		if (function == MAIL_ANY_NEWS || function == SAVE_ANY_NEWS) {
			if (!group->attribute->batch_save)
				continue;

			group_count++;
			snprintf(buf, sizeof(buf) - 1, _(txt_saved_groupname), group->name);
			fprintf(fp_log, buf);
			if (verbose)
				wait_message(0, buf);

			if (function == SAVE_ANY_NEWS) {
				char tmp[PATH_LEN];

				if (!strfpath(group->attribute->savedir, buf, sizeof(buf), group))
					joinpath(buf, homedir, DEFAULT_SAVEDIR);
				joinpath(tmp, buf, group_path);
#if 0
fprintf(stderr, "start_save: create_path(%s)\n", tmp);
#endif /* 0 */
				create_path(tmp);
			}
		}

		/*
		 * For each article in this group...
		 */
		for_each_art(j) {
			if (arts[j].status != ART_UNREAD)
				continue;

			switch (function) {
				case CHECK_ANY_NEWS:
					if (print_first && verbose) {
						my_fputc('\n', stdout);
						print_first = FALSE;
					}
					art_count++;
					if (arts[j].score >= tinrc.score_select)
						hot_count++;
					break;

				case START_ANY_NEWS:
					return i;	/* return first group with unread news */
					/* NOTREACHED */

				case MAIL_ANY_NEWS:
				case SAVE_ANY_NEWS:
					/*
					 * TODO: open_art_fp() returns FAKE_NNTP_FP in case of
					 *       reading via NNTP, artfp later is used in
					 *       copy_fp() which badly fails in that case.
					 */
					artfp = open_art_fp(group_path, arts[j].artnum);
					/* FIXME! (i.e. use t_openartinfo->raw) */
					if (artfp == FAKE_NNTP_FP || artfp == NULL)
						continue;

					if (function == MAIL_ANY_NEWS)
						snprintf(savefile, sizeof(savefile) - 1, "%stin.%d", TMPDIR, (int) process_id);
					else {
						if (!strfpath(group->attribute->savedir, path, sizeof(path), group))
							joinpath(path, homedir, DEFAULT_SAVEDIR);

						/* TODO: use joinpath() */
#ifdef VMS
						snprintf(savefile, sizeof(savefile) - 1, "%s.%s]%ld", path, group_path, arts[j].artnum);
#else
						snprintf(savefile, sizeof(savefile) - 1, "%s/%s/%ld", path, group_path, arts[j].artnum);
#endif /* VMS */
					}

					if ((fp = fopen(savefile, "w" FOPEN_OPTS)) == NULL) {
						fprintf(fp_log, _(txt_cannot_open), savefile);
						if (verbose)
							perror_message(_(txt_cannot_open), savefile);
						TIN_FCLOSE(artfp);
						continue;
					}

					if (function == MAIL_ANY_NEWS)
						fprintf(fp, "To: %s\n", mail_news_user);

					snprintf(buf, sizeof(buf) - 1, "[%5ld]  %s\n", arts[j].artnum, arts[j].subject);
					fprintf(fp_log, "%s", buf);		/* buf may contain % */
					if (verbose)
						wait_message(0, buf);

					copy_fp(artfp, fp);
					TIN_FCLOSE(artfp);
					fclose(fp);
					saved_arts++;

					/* TODO: if article already contains To: Cc: Bcc: it get's 'relayed' */
					if (function == MAIL_ANY_NEWS) {
						strfmailer(mailer, arts[j].subject, mail_news_user, savefile, buf, sizeof(buf), tinrc.mailer_format);
						invoke_cmd(buf);		/* Keep trying after errors */
						unlink(savefile);
					}
					if (catchup)
						art_mark_read(group, &arts[j]);
					break;

				default:
					break;
			}
		}

		if (art_count) {
			if (verbose)
				wait_message(0, _(txt_saved_group), art_count, hot_count,
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
					wait_message(1, _(txt_there_is_no_news));
				return 0;
			}
			/* NOTREACHED */

		case START_ANY_NEWS:
			wait_message(1, _(txt_there_is_no_news));
			return -1;
			/* NOTREACHED */

		case MAIL_ANY_NEWS:
		case SAVE_ANY_NEWS:
			snprintf(buf, sizeof(buf) - 1, _(txt_saved_summary), (function == MAIL_ANY_NEWS ? _(txt_mailed) : _(txt_saved)),
					saved_arts, PLURAL(saved_arts, txt_article),
					group_count, PLURAL(group_count, txt_group));
			fprintf(fp_log, "%s", buf);
			if (verbose)
				wait_message(0, buf);

			if (log_opened) {
				fclose(fp_log);
				if (verbose)
					wait_message(0, _(txt_mail_log_to), (function == MAIL_ANY_NEWS ? mail_news_user : userid));
				strfmailer(mailer, subject, (function == MAIL_ANY_NEWS ? mail_news_user : userid), logfile, buf, sizeof(buf), tinrc.mailer_format);
				if (invoke_cmd(buf))
					unlink(logfile);
			}
			break;

		default:
			break;
	}
	return 0;
}


/*
 * Do basic validation of a save-to path, handle append/overwrite semantics
 * and return an opened file handle or NULL if user aborted etc..
 */
static FILE *
open_save_filename(
	const char *path,
	t_bool mbox)
{
	FILE *fp;
	char keyappend[MAXKEYLEN], keyoverwrite[MAXKEYLEN], keyquit[MAXKEYLEN];
	char mode[3];
	int ch;
	struct stat st;

	strcpy(mode, "a+");

	/*
	 * Mailboxes will always be appended to
	 */
	if (!mbox && stat(path, &st) != -1) {
		/*
		 * Admittedly a special case hack, but it saves failing later on
		 */
		if (S_ISDIR(st.st_mode)) {
			wait_message(2, _(txt_cannot_write_to_directory), path);
			return NULL;
		}

		ch = prompt_slk_response(tinrc.default_save_mode,
				&menukeymap.save_append_overwrite_quit,
				_(txt_append_overwrite_quit), path,
				printascii(keyappend, map_to_local(iKeySaveAppendFile, &menukeymap.save_append_overwrite_quit)),
				printascii(keyoverwrite, map_to_local(iKeySaveOverwriteFile, &menukeymap.save_append_overwrite_quit)),
				printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.save_append_overwrite_quit)));

		switch (ch) {
			case iKeySaveOverwriteFile:
				strcpy(mode, "w");
				break;

			case iKeyAbort:
			case iKeyQuit:
				wait_message(1, _(txt_art_not_saved));
				return NULL;

			default:	/* iKeySaveAppendFile */
				break;
		}
		tinrc.default_save_mode = ch;
	}

	if ((fp = fopen(path, mode)) == NULL) {
		perror_message("%s (%s)", _(txt_art_not_saved), path);
		return NULL;
	}

	return fp;
}


/*
 * This is where articles are actually copied to disk
 * All save functions use this function eventually
 * Save the article opened in 'artinfo'
 * 'indexnum' is index into save[] for the save path information etc..
 * Returns:
 *     TRUE or FALSE depending on whether article was saved okay.
 *
 * TODO: use append_mail() here
 */
t_bool
save_art_to_file(
	int indexnum,
	t_openartinfo *artinfo)
{
	FILE *fp;
	char from[HEADER_LEN];
	time_t epoch;
	t_bool mmdf = (save[indexnum].is_mailbox && !strcasecmp(txt_mailbox_formats[tinrc.mailbox_format], "MMDF"));

	if ((fp = open_save_filename(save[indexnum].path, save[indexnum].is_mailbox)) == NULL)
		return FALSE;

	if (mmdf)
		fprintf(fp, "%s", MMDFHDRTXT);
	else {
		if (artinfo->hdr.from)
			strip_name(artinfo->hdr.from, from);
		(void) time(&epoch);
		fprintf(fp, "From %s %s", from, ctime(&epoch));
		/*
		 * TODO: add Content-Length: header when using MBOXO
		 *       so tin actually write MBOXCL instead of MBOXO?
		 */
	}

	if (fseek(artinfo->raw, 0L, SEEK_SET) == -1)
		perror_message("fseek() error on [%s]", save[indexnum].artptr->subject); /* FIXME: -> lang.c */

	if (copy_fp(artinfo->raw, fp))
		/* Write tailing newline or MMDF-mailbox seperator */
		print_art_seperator_line(fp, save[indexnum].is_mailbox);

	fclose(fp);

	save[indexnum].saved = TRUE;
	if (tinrc.mark_saved_read)
		art_mark_read(&CURR_GROUP, save[indexnum].artptr);

#ifdef USE_CURSES
	scrollok(stdscr, TRUE);
#endif /* USE_CURSES */
#ifndef HAVE_LIBUU		/* libuu decodes base64 internally */
	decode_save_mime(artinfo, TRUE);
#endif /* !HAVE_LIBUU */
#ifdef USE_CURSES
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */

	return TRUE;
}


/*
 * We return the number of articles saved successfully
 */
static int
save_arts(
	const char *group_path)
{
	int i;
	int count = 0;
	t_openartinfo artinfo;

	/*
	 * Create the path to save the arts into. There is no way that there
	 * can be different paths in the same batch so we do it once against the
	 * first pathname
	 */
#if 0
fprintf(stderr, "save_arts, create_path(%s)\n", save[0].path);
#endif /* 0 */
	if (!create_path(save[0].path))
		return count;

	for (i = 0 ; i < num_save ; i++) {
		/* the tailing spaces are needed for the progress-meter */
		wait_message(0, "%s%d  ", _(txt_saving), i + 1);

		memset(&artinfo, 0, sizeof(t_openartinfo));
		switch (art_open(FALSE, save[i].artptr, group_path, &artinfo, TRUE)) {
			case ART_ABORT:					/* User 'q'uit */
				return count;

			case ART_UNAVAILABLE:			/* Ignore, just keep going */
				wait_message(1, _(txt_art_unavailable));
				art_mark_read(&CURR_GROUP, save[i].artptr);
				continue;

			default:
				if (save_art_to_file(i, &artinfo))
					++count;
				art_close(&artinfo);
		}
	}
	return count;
}


/*
 * Save everything batched up in save[]
 * Print a message like:
 * -- [Article|Thread|Tagged Articles] saved to [mailbox] [filenames] --
 * TODO => lang.c
 */
t_bool
save_batch(
	char type,
	const char *group_path)
{
	const char *first;
	char buf[LEN];
	char what[LEN];
	int count;

	if (num_save == 0) {
		/* TODO maybe print something more context dependent here? -> lang.c */
		wait_message(1, _("No articles to save"));
		return FALSE;
	}

	if ((count = save_arts(group_path)) == 0) {
		wait_message(2, _(txt_saved_nothing));
		return FALSE;
	}

	if (count != num_save)	/* FIMME: -> lang.c */
		wait_message(2, _("Warning: Only %d out of %d articles were saved"), count, num_save);

	first = get_first_savefile();

	switch (type) {	/* FIMME: -> lang.c */
		case iKeyFeedHot:
			snprintf(what, sizeof(what), _("Hot %s"), PLURAL(count, txt_article));
			break;

		case iKeyFeedTag:
			snprintf(what, sizeof(what), _("Tagged %s"), PLURAL(count, txt_article));
			break;

		case iKeyFeedThd:
			STRCPY(what, _("Thread"));
			break;

		case iKeyFeedArt:
		case iKeyFeedPat:
		default:
			snprintf(what, sizeof(what), "%s", PLURAL(count, txt_article));
			break;
	}

	/*
	 * We report the range of saved-to files for regular saves of > 1 articles
	 * TODO: "mailbox " -> lang.c
	 */
	if (num_save == 1 || save[0].is_mailbox)
		snprintf(buf, sizeof(buf), _("-- %s saved to %s%s --"),
			what, (save[0].is_mailbox ? _("mailbox ") : ""), first);
	else
		snprintf(buf, sizeof(buf), _("-- %s saved to %s%s - %s --"),
			what, (save[0].is_mailbox ? _("mailbox ") :  ""),
			first, get_last_savefile());

	wait_message((tinrc.beginner_level) ? 2 : 1, buf);

	return (count != 0);
}


/*
 * Create the supplied path. Create intermediate directories as needed
 * Don't create the last component (which would be the filename) unless the
 * path is / terminated.
 * Return FALSE if it somehow fails.
 */
t_bool
create_path(
	const char *path)
{
	char buf[PATH_LEN];
	int i, j, len;
	struct stat st;

#ifndef VMS
	len = (int) strlen(path);

	for (i = 0, j = 0; i < len; i++, j++) {
		buf[j] = path[i];
		if (i + 1 < len && path[i + 1] == DIRSEP) {
			buf[j + 1] = '\0';
			if (stat(buf, &st) == -1) {
				if (my_mkdir(buf, (mode_t) (S_IRWXU|S_IRUGO|S_IXUGO)) == -1) {
					if (errno != EEXIST) {
						perror_message(_(txt_cannot_create), buf);
						return FALSE;
					}
				}
			}
		}
	}
#else
	if (my_mkdir(buf, (mode_t) (S_IRWXU|S_IRUGO|S_IXUGO)) == -1) {
		if (errno != EEXIST) {
			perror_message(_(txt_cannot_create), buf);
			return FALSE;
		}
	}
#endif /* !VMS */

	return TRUE;
}


/*
 * Get a path/filename to save to, using 'path' as input.
 * The pathname is stored in 'outpath', which should be PATH_LEN in size
 * Expand metacharacters and use defaults as needed.
 * Return TRUE if the path is a mailbox, or FALSE otherwise.
 */
static t_bool
get_save_filename(
	char *outpath,
	const char *path)
{
	int ret;
	struct t_group *group = &CURR_GROUP;

	ret = strfpath(path, outpath, PATH_LEN, group);

	/*
	 * If no path exists or the above failed in some way, use sensible defaults
	 * Put the generic path into 'outpath'
	 */
	if ((ret == 0) || !(strrchr(outpath, DIRSEP))) {
		char buf[PATH_LEN];

		if (!strfpath(group->attribute->savedir, buf, sizeof(buf), group))
			joinpath(buf, homedir, DEFAULT_SAVEDIR);
		joinpath(outpath, buf, path);
		return FALSE;
	} else
		return (ret == 1);
}


/*
 * Add a file to be saved to the save[] array
 * 'artptr' points to the article in arts[]
 * 'path' is the generic save path/file that was entered/default
 * Expand the path appropriately, taking account of multiple file
 * extensions and the auto-save with Archive-Name: headers
 *
 * Return TRUE if the generated path is a mailbox, FALSE otherwise.
 */
t_bool
add_to_save_list(
	struct t_article *artptr,
	const char *path)
{
	char tmp[PATH_LEN];

	if (num_save == max_save - 1)
		expand_save();

	save[num_save].is_mailbox = get_save_filename(tmp, path);

	/*
	 * If using the auto-save feature on an article with Archive-Name
	 * The path will be: (path+archive)/archive.part<part>
	 */
	if (!save[num_save].is_mailbox && CURR_GROUP.attribute->auto_save && artptr->archive && (artptr->part || artptr->patch)) {
		/* TODO the Archive-Name code is largely untested */
		const char *partprefix;
		char *partname;
		char *ptr;
		char archpath[PATH_LEN];
		char filename[PATH_LEN];

		/*
		 * We need either a part or a patch number, part takes precedence
		 */
		if (artptr->part) {
			partprefix = LONG_PATH_PART;
			partname = artptr->part;
		} else {
			partprefix = LONG_PATH_PATCH;
			partname = artptr->patch;
		}

		/*
		 * Strip off any default filename
		 */
		if ((ptr = strrchr(tmp, DIRSEP)) != NULL)
			*(ptr + 1) = '\0';

		/* Add on the archive name as a directory */
		joinpath(archpath, tmp, artptr->archive);

		/* Generate the filename part and append it */
		snprintf(filename, sizeof(filename) - 1, "%s.%s%s", artptr->archive, partname, partprefix);
		joinpath(tmp, archpath, filename);
	} else {
		/*
		 * Mailbox saves are by definition to a single file
		 * Otherwise append a .NNN sequence number to the path. We strip this
		 * later if there is only 1 part
		 */
		if (!save[num_save].is_mailbox && (num_save >= 1)) {
#ifdef VMS
			const char pathsep = '-';		/* Suffix seperator for .001 type extensions */
#else
			const char pathsep = '.';
#endif /* VMS */

			/*
			 * If this is the 2nd file we're saving, we must retrospectively
			 * add the extension to the 1st file. Hackish, but keeps code
			 * clean
			 */
			if (num_save == 1) {
				save[0].path = my_realloc(save[0].path, strlen(save[0].path) + 5);
				strcat(save[0].path, ".001");
				save[0].file = strrchr(save[0].path, DIRSEP) + 1;	/* ptr to filename portion */
			}

			sprintf(&tmp[strlen(tmp)], "%c%03d", pathsep, num_save + 1);
		}
	}

	save[num_save].path = my_strdup(tmp);
	save[num_save].file = strrchr(save[num_save].path, DIRSEP) + 1;	/* ptr to filename portion */
	save[num_save].artptr = artptr;
	save[num_save].saved = FALSE;
#if 0
fprintf(stderr, "ATSL (%s) (%s)\n", save[num_save].path, save[num_save].file);
#endif /* 0 */
	return save[num_save++].is_mailbox;		/* NB: num_save is bumped here */
}


#if 0
/*
 * string comparison routine for the qsort()
 * ie. qsort(array, 5, 32, save_comp);
 */
static int
save_comp(
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
				if (strcmp(s1->part, s2->part) < 0)
					return -1;
				if (strcmp(s1->part, s2->part) > 0)
					return 1;
			} else
				return 0;
		} else if (s1->patch != 0) {
			if (s2->patch != 0) {
				if (strcmp(s1->patch, s2->patch) < 0)
					return -1;
				if (strcmp(s1->patch, s2->patch) > 0)
					return 1;
			} else
				return 0;
		}
	} else {
		if (strcmp(s1->artptr->subject, s2->artptr->subject) < 0)
			return -1;
		if (strcmp(s1->artptr->subject, s2->artptr->subject) > 0)
			return 1;
	}

	return 0;
}


/*
 * Print save array of files to be saved
 */
void
sort_save_list(
	void)
{
	qsort((char *) save, (size_t) num_save, sizeof(struct t_save), save_comp);
#ifdef DEBUG
	debug_save_comp();
#endif /* DEBUG */
}
#endif /* 0 */


/*
 * Return the name of the file to be saved from save[i]
 * The name is used for reporting purposes only, so whatever reads most
 * usefully is returned. for mailboxes we return the full path.
 */
static const char *
get_first_savefile(
	void)
{
	int i;
	static const char *dummy = "";

	for (i = 0; i < num_save; i++) {
		if (save[i].saved)
			return (save[i].is_mailbox ? save[i].path : save[i].file);
	}

	return dummy;
}


static const char *
get_last_savefile(
	void)
{
	int i;
	static const char *dummy = "";

	for (i = num_save - 1; i >= 0; i--) {
		if (save[i].saved)
			return (save[i].is_mailbox ? save[i].path : save[i].file);
	}

	return dummy;
}


/*
 * Post process the articles in save[] according to proc_type_ch
 * auto_delete is set in the AUTOSAVE_TAGGED case
 * This stage can produce a fair bit of output so we allow it to
 * scroll up the screen rather than waste time displaying it in the
 * message bar
 */
t_bool
post_process_files(
	int proc_type_ch,
	t_bool auto_delete)
{
	if (!any_saved_files())
		return FALSE;

	clear_message();
#ifdef USE_CURSES
	scrollok(stdscr, TRUE);
#endif /* USE_CURSES */
	my_printf("%s%s", _(txt_post_processing), cCRLF);

	switch (proc_type_ch) {
		case iKeyPProcShar:
			post_process_sh(auto_delete);
			break;

		case iKeyPProcUUDecode:		/* This is the default, eg, with AUTOSAVE_TAGGED */
		default:
			post_process_uud(auto_delete);
			break;
	}

	my_printf("%s%s%s", _(txt_post_processing_finished), cCRLF, cCRLF);
	my_flush();
	prompt_continue();
#ifdef USE_CURSES
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */

	return TRUE;
}


static void
post_process_uud(
	t_bool auto_delete)
{
	FILE *fp_in;
	char file_out_dir[PATH_LEN];
	int i;
#ifdef HAVE_LIBUU
	const char *eptr;
	int count;
	int errors = 0;
	uulist *item;
#else
	FILE *fp_out = NULL;
	char *filename = NULL;
	char path[PATH_LEN];
	char s[LEN], t[LEN], u[LEN];
	int state;
	mode_t mode = (S_IRUSR|S_IWUSR);
#endif /* HAVE_LIBUU */

	/*
	 * Grab the dirname portion
	 */
	my_strncpy(file_out_dir, save[0].path, save[0].file - save[0].path);

#ifdef HAVE_LIBUU
	UUInitialize();

	UUSetOption(UUOPT_SAVEPATH, 0, file_out_dir);
	for (i = 0; i < num_save; i++) {
		if (!save[i].saved)
			continue;

		if ((fp_in = fopen(save[i].path, "r")) != NULL) {
			UULoadFile(save[i].path, NULL, 0);
			fclose(fp_in);
		}
	}

#	if 0 /* uudeview's "intelligent" multi-part detection */
	UUSmerge(0);
	UUSmerge(1);
	UUSmerge(99);
#	endif /* 0 */

	i = count = 0;
	item = UUGetFileListItem(i);
	my_printf(cCRLF);

	while (item != NULL) {
		if (UUDecodeFile(item, NULL) == UURET_OK) {
			count++;
			my_printf(_(txt_uu_success), item->filename);
			my_printf(cCRLF);
			/* TODO - insert viewing code here? */
		} else {
			errors++;
			if (item->state & UUFILE_MISPART)
				eptr = _(txt_libuu_error_missing);
			else if (item->state & UUFILE_NOBEGIN)
				eptr = _(txt_libuu_error_no_begin);
			else if (item->state & UUFILE_NOEND)
				eptr = _(txt_uu_error_no_end);
			else if (item->state & UUFILE_NODATA)
				eptr = _(txt_libuu_error_no_data);
			else
				eptr = _(txt_libuu_error_unknown);

			my_printf(_(txt_uu_error_decode), (item->filename) ? item->filename : item->subfname, eptr);
			my_printf(cCRLF);
		}
		i++;
		item = UUGetFileListItem(i);
		my_flush();
	}

	my_printf(_(txt_libuu_saved), count, num_save, errors, PLURAL(errors, txt_error));
	my_printf(cCRLF);
	UUCleanUp();
#else
	t[0] = '\0';
	u[0] = '\0';

	state = INITIAL;

	for (i = 0; i < num_save; i++) {
		if (!save[i].saved)
			continue;

		if ((fp_in = fopen(save[i].path, "r")) == NULL)
			continue;

		while (fgets(s, (int) sizeof(s), fp_in) != 0) {
			switch (state) {
				case INITIAL:
					if (strncmp("begin ", s, 6) == 0) {
						/* don't use PATH_LEN - we use an absolute value (128) below */
						char name[130];

						if (sscanf(s + 6, "%o %128c\n", &mode, name) != 2)		/* Get the real filename and the mode */
							name[0] = '\0';
						else
							strtok(name, "\n");

						name[sizeof(name) - 1] = '\0';
						str_trim(name);

						/* Remove pathname if present, it's dangerous */
						if ((filename = strrchr(name, DIRSEP)) != NULL)
							filename++;
						else
							filename = name;

						get_save_filename(path, filename);
						filename = strrchr(path, DIRSEP) + 1;  /* ptr to filename portion */
						if ((fp_out = fopen(path, "w")) == NULL) {
							perror_message(_(txt_cannot_open), path);
							return;
						}
						state = MIDDLE;
					}
					break;

				case MIDDLE:
					if (s[0] == 'M')
						uudecode_line(s, fp_out);
					else if (STRNCMPEQ("end", s, 3)) {
						state = END;
						if (u[0] != 'M')
							uudecode_line(u, fp_out);
						if (t[0] != 'M')
							uudecode_line(t, fp_out);
					} else                                  /* end */
						state = OFF;            /* OFF => a break in the uuencoded data */
					break;

				case OFF:
					if ((s[0] == 'M') && (t[0] == 'M') && (u[0] == 'M')) {
						uudecode_line(u, fp_out);
						uudecode_line(t, fp_out);
						uudecode_line(s, fp_out);
						state = MIDDLE;         /* Continue output of previously suspended data */
					} else if (STRNCMPEQ("end", s, 3)) {
						state = END;
						if (u[0] != 'M')
							uudecode_line(u, fp_out);
						if (t[0] != 'M')
							uudecode_line(t, fp_out);
					}
					break;

				case END:
				default:
					break;
			}	/* switch (state) */

			if (state == END) {
				/* set the mode after getting rid of dangerous bits */
				if (!(mode &= ~(S_ISUID|S_ISGID|S_ISVTX)))
					mode = (S_IRUSR|S_IWUSR);

				fchmod (fileno(fp_out), mode);

				fclose(fp_out);
				fp_out = NULL;

				my_printf(_(txt_uu_success), filename);
				my_printf(cCRLF);
				sum_and_view(path, filename);
				state = INITIAL;
				continue;
			}

			strcpy(u, t);	/* Keep tabs on the last two lines, which typically do not start with M */
			strcpy(t, s);

		}	/* while (fgets) ... */

		fclose(fp_in);

	} /* for i...num_save */

	/*
	 * Check if we ran out of data
	 */
	if (fp_out) {
		fclose(fp_out);
		my_printf(_(txt_uu_error_decode), filename, _(txt_uu_error_no_end));
		my_printf(cCRLF);
	}

#endif /* HAVE_LIBUU */
	delete_processed_files(auto_delete); /* TRUE = auto-delete files */
	return;
}


/*
 * Do whatever needs doing after a successful uudecode
 */
#ifndef HAVE_LIBUU
static void
sum_and_view(
	const char *path,
	const char *file)
{
	char *ext;
	t_part *part;
#	ifndef DONT_HAVE_PIPING
	FILE *fp_in;
	char buf[LEN];
#	endif /* !DONT_HAVE_PIPING */

	/*
	 * Sum file - TODO why do we bother to do this?
	 * nuke code or add DONT_HAVE_PIPING and !M_UNIX -tree
	 */
#	if defined(M_UNIX) && defined(HAVE_SUM) && !defined(DONT_HAVE_PIPING)
	sh_format(buf, sizeof(buf), "%s \"%s\"", DEFAULT_SUM, path);
	if ((fp_in = popen(buf, "r")) != NULL) {
		buf[0] = '\0';

		/*
		 * You can't do this with (fgets != NULL)
		 */
		while (!feof(fp_in)) {
			fgets(buf, (int) sizeof(buf), fp_in);
			if ((ext = strchr(buf, '\n')) != NULL)
				*ext = '\0';
		}
		fflush(fp_in);
		pclose(fp_in);

		my_printf(_(txt_checksum_of_file), file, file_size(path), _("bytes"));
		my_printf(cCRLF);
		my_printf("\t%s%s", buf, cCRLF);
	} else {
		my_printf(_(txt_command_failed), buf);
		my_printf(cCRLF);
	}
	my_flush();
#	endif /* M_UNIX && HAVE SUM && !DONT_HAVE_PIPING */

	/*
	 * If defined, invoke post processor command
	 * Create a part structure, with defaults, insert a parameter for the name
	 */
	part = new_part(NULL);

	if ((ext = strrchr(file, '.')) != NULL)
		lookup_mimetype(ext + 1, part);				/* Get MIME type/subtype */

	/*
	 * Needed for the mime-type processor
	 */
	part->params = my_malloc(sizeof(t_param));
	part->params->name = my_strdup("name");
	part->params->value = my_strdup(file);
	part->params->next = NULL;

	if (tinrc.post_process_view) {
		start_viewer(part, path);
		my_printf(cCRLF);
	}

	free_parts(part);
}
#endif /* !HAVE_LIBUU */


/*
 * Unpack /bin/sh archives
 */
static void
post_process_sh(
	t_bool auto_delete)
{
	FILE *fp_in, *fp_out;
	char buf[LEN];
	char file_out[PATH_LEN];
	char file_out_dir[PATH_LEN];
	int i;
	t_bool found_header;

	/*
	 * Grab the dirname portion
	 */
	my_strncpy(file_out_dir, save[0].path, save[0].file - save[0].path);

#ifdef VMS
	snprintf(file_out, sizeof(file_out) - 1, "%ssh.%05d", file_out_dir, (int) process_id);
#else
	snprintf(file_out, sizeof(file_out) - 1, "%ssh%05d", file_out_dir, (int) process_id);
#endif /* VMS */

	for (i = 0; i < num_save; i++) {
		if (!save[i].saved)
			continue;

		wait_message(1, _(txt_extracting_shar), save[i].path);

		found_header = FALSE;

		if ((fp_out = fopen(file_out, "w" FOPEN_OPTS)) != NULL) {
			if ((fp_in = fopen(save[i].path, "r")) != NULL) {

				while (!feof(fp_in)) {
					if (fgets(buf, (int) sizeof(buf), fp_in)) {
						/* find #!/bin/sh or #!/bin/sh pattern */
						if (!found_header && pcre_exec(shar_regex.re, shar_regex.extra, buf, strlen(buf), 0, 0, NULL, 0) >= 0)
							found_header = TRUE;

						/* write to temp file */
						if (found_header)
							fputs(buf, fp_out);
					}
				}
				fclose(fp_in);
			}
			fclose(fp_out);

#ifndef M_UNIX
			make_post_process_cmd(DEFAULT_UNSHAR, file_out_dir, file_out);
#else
			sh_format(buf, sizeof(buf), "cd %s; sh %s", file_out_dir, file_out);
			my_fputs(cCRLF, stdout);
			my_flush();
			Raw(FALSE);									/* TODO done in invoke_cmd()? */
			if (!invoke_cmd(buf))
				error_message(_(txt_command_failed), buf);	/* TODO not needed */
			Raw(TRUE);										/* TODO done in invoke_cmd()? */
#endif /* !M_UNIX */
			unlink(file_out);
		}
	}
	delete_processed_files(auto_delete);
}


static void
delete_processed_files(
	t_bool auto_delete)
{
	int i;
	t_bool delete_it = FALSE;

	if (any_saved_files()) {
		if (CURR_GROUP.attribute->delete_tmp_files && (auto_delete || prompt_yn(cLINES, _(txt_delete_processed_files), TRUE) == 1))
			delete_it = TRUE;

		my_printf(cCRLF);
		my_flush();

		if (delete_it) {
			my_printf("%s%s", _(txt_deleting), cCRLF);
			my_flush();

			for (i = 0; i < num_save; i++)
				unlink(save[i].path);
		}
	}
}


static t_bool
any_saved_files(
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
 * write tailing (MMDF)-mailbox seperator
 */
void
print_art_seperator_line(
	FILE *fp,
	t_bool is_mailbox)
{
#ifdef DEBUG
	if (debug == 2)
		error_message("Mailbox=[%d], mailbox_format=[%s]", is_mailbox, txt_mailbox_formats[tinrc.mailbox_format]);
#endif /* DEBUG */

	fprintf(fp, "%s", (is_mailbox && !strcasecmp(txt_mailbox_formats[tinrc.mailbox_format], "MMDF")) ? MMDFHDRTXT : "\n");
}


static void
start_viewer(
	t_part *part,
	const char *path)
{
	t_mailcap *foo;

	if ((foo = get_mailcap_entry(part, path)) != NULL) {
		char buff[LEN];

		if (foo->nametemplate)	/* honor nametemplate */
			rename_file(path, foo->nametemplate);

		wait_message(0, _(txt_starting_command), foo->command);

		/* are the () needed if foo->command holds more than one cmd? */
		snprintf(buff, sizeof(buff) - 1, "(%s)", foo->command);
		if (foo->needsterminal) {
			set_xclick_off();
			EndWin();
			Raw(FALSE);
			fflush(stdout);
		} else {
			if (foo->description)
				info_message(foo->description);
		}
		invoke_cmd(foo->command);
		if (foo->needsterminal) {
			Raw(TRUE);
			InitWin();
			prompt_continue();
		}
		if (foo->nametemplate) /* undo nametemplate, needed as 'save'-prompt is done outside start_viewer */
			rename_file(foo->nametemplate, path);
		free_mailcap(foo);
	} else
		wait_message(0, _(txt_no_viewer_found), content_types[part->type], part->subtype);
}


/*
 * Decode and save the binary object pointed to in 'part'
 * Optionally launch a viewer for it
 */
static void
decode_save_one(
	t_part *part,
	FILE *rawfp,
	t_bool postproc)
{
	FILE *fp;
	char buf[2048], buf2[2048];
	char savepath[PATH_LEN];
	const char *name;
	int i;
	struct t_attribute *attr = CURR_GROUP.attribute;
	t_bool mbox;

	/*
	 * Get the filename to save to in 'savepath'
	 */
	if ((name = get_filename(part->params)) == NULL)
		mbox = get_save_filename(savepath, attr->savefile ? attr->savefile : tinrc.default_save_file);
	else
		mbox = get_save_filename(savepath, name);

	/*
	 * Not a good idea to dump attachments over a mailbox
	 */
	if (mbox) {
		wait_message(2, _(txt_is_mailbox), content_types[part->type], part->subtype);
		return;
	}

	if (!(create_path(savepath))) {
		error_message(_(txt_cannot_open_for_saving), savepath);
		return;
	}

	/*
	 * Decode/save the attachment
	 */
	if ((fp = open_save_filename(savepath, FALSE)) == NULL) {
		error_message(_(txt_cannot_open_for_saving), savepath);
		return;
	}

	if (part->encoding == ENCODING_BASE64)
		mmdecode(NULL, 'b', 0, NULL);				/* flush */

	fseek(rawfp, part->offset, SEEK_SET);

	for (i = 0; i < part->line_count; i++) {
		if ((fgets(buf, sizeof(buf), rawfp)) == NULL)
			break;

		/* This should catch cases where people illegally append text etc */
		if (buf[0] == '\0')
			break;

		switch (part->encoding) {
			int count;

			case ENCODING_QP:
			case ENCODING_BASE64:
				count = mmdecode(buf, part->encoding == ENCODING_QP ? 'q' : 'b', '\0', buf2);
				fwrite(buf2, count, 1, fp);
				break;

			case ENCODING_UUE:
				/* TODO - if postproc, don't decode these since the traditional uudecoder will get them */
				/*
				 * x-uuencode attachments have all the header info etc which we must ignore
				 */
				if (strncmp(buf, "begin ", 6) != 0 && strncmp(buf, "end\n", 4) != 0 && buf[0] != '\n')
					uudecode_line(buf, fp);
				break;

			default:
				fputs(buf, fp);
		}
	}
	fclose(fp);

	/*
	 * View the attachment
	 */
	if (postproc) {
		if (tinrc.post_process_view) {
			start_viewer(part, savepath);
			my_printf(cCRLF);
		}
	} else {
		snprintf(buf, sizeof(buf) - 1, _(txt_view_attachment), savepath, content_types[part->type], part->subtype);
		if (prompt_yn(cLINES, buf, TRUE) == 1)
			start_viewer(part, savepath);
	}

	/*
	 * Save the attachment
	 */
	if (postproc && tinrc.post_process_view) {
		my_printf(_(txt_uu_success), savepath);
		my_printf(cCRLF);
	}
	if (!postproc) {
		snprintf(buf, sizeof(buf) - 1, _(txt_save_attachment), savepath, content_types[part->type], part->subtype);
		if (prompt_yn(cLINES, buf, FALSE) != 1)
			unlink(savepath);
	}
}


/*
 * decode and save binary MIME attachments from an open article context
 * optionally locate and launch a viewer application
 * 'postproc' determines the mode of the operation and will be set to
 * TRUE when we're saving articles using 's/S' and FALSE otherwise (ie
 * when just viewing)
 * When it is FALSE then the view/save options will be queried
 * Otherwise the view option will depend on post_process_view and the
 * save is implicit. Feedback will also be printed.
 */
void
decode_save_mime(
	t_openartinfo *art,
	t_bool postproc)
{
	t_part *ptr;
	t_part *uueptr;

	/*
	 * Iterate over all the attachments
	 */
	for (ptr = art->hdr.ext; ptr != NULL; ptr = ptr->next) {
		/*
		 * Handle uuencoded sections in this message part
		 * We don't do this when postprocessing as the generic uudecode
		 * code already handles uuencoded data
		 */
		if (!postproc) {
			for (uueptr = ptr->uue; uueptr != NULL; uueptr = uueptr->next)
				decode_save_one(uueptr, art->raw, postproc);
		}

		/*
		 * Decode this message part if appropriate
		 */
		if (ptr->type == TYPE_MULTIPART || IS_PLAINTEXT(ptr))
			continue;

		decode_save_one(ptr, art->raw, postproc);
	}
}


/* Single character decode. */
#define DEC(Char) (((Char) - ' ') & 077)
/*
 * Decode 'buf' - write the uudecoded output to 'fp'
 */
static void
uudecode_line(
	const char *buf,
	FILE *fp)
{
	const char *p = buf;
	char ch;
	int n;

	n = DEC(*p);

	for (++p; n > 0; p += 4, n -= 3) {
		if (n >= 3) {
			ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
			fputc(ch, fp);
			ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
			fputc(ch, fp);
			ch = DEC(p[2]) << 6 | DEC(p[3]);
			fputc(ch, fp);
		} else {
			if (n >= 1) {
				ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
				fputc(ch, fp);
			}
			if (n >= 2) {
				ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
				fputc(ch, fp);
			}
		}
	}
	return;
}
