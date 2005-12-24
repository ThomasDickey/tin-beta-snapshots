/*
 *  Project   : tin - a Usenet reader
 *  Module    : save.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2005-07-02
 *  Notes     :
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

#ifdef HAVE_UUDEVIEW_H
#	ifndef __UUDEVIEW_H__
#		include <uudeview.h>
#	endif /* !__UUDEVIEW_H__ */
#endif /* HAVE_UUDEVIEW_H */

#undef OFF

enum state { INITIAL, MIDDLE, OFF, END };

/*
 * Local prototypes
 */
static FILE *open_save_filename(const char *path, t_bool mbox);
static int match_content_type(t_part *part, char *type);
static t_bool check_save_mime_type(t_part *part, const char *mime_types);
static t_bool decode_save_one(t_part *part, FILE *rawfp, t_bool postproc);
static void generate_filename(char *buf, int buflen, const char *suffix);
static void post_process_uud(void);
static void post_process_sh(void);
static void start_viewer(t_part *part, const char *path);
static void uudecode_line(const char *buf, FILE *fp);
static void view_file(const char *path, const char *file);
#ifndef HAVE_LIBUU
	static void sum_file(const char *path, const char *file);
#endif /* !HAVE_LIBUU */


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
	FILE *artfp, *savefp;
	FILE *fp_log = (FILE *) 0;
	char *line;
	char buf[LEN];
	char group_path[PATH_LEN];
	char path[PATH_LEN];
	char logfile[PATH_LEN], savefile[PATH_LEN];
	char subject[HEADER_LEN];
	int group_count = 0;
	int i, j;
	int art_count, hot_count;
	int saved_arts = 0;					/* Total # saved arts */
	struct t_group *group;
	t_bool log_opened = TRUE;
	t_bool print_first = verbose;
	t_bool unread_news = FALSE;
	time_t epoch;

	switch (function) {
		case CHECK_ANY_NEWS:
		case START_ANY_NEWS:
			if (verbose)
				wait_message(0, _(txt_checking_for_news));
			break;

		case MAIL_ANY_NEWS:
			joinpath(savefile, TMPDIR, "tin");
#ifdef APPEND_PID
			snprintf(savefile + strlen(savefile), sizeof(savefile) - strlen(savefile), ".%d", (int) process_id);
#endif /* APPEND_PID */
			/* FALLTHROUGH */

		case SAVE_ANY_NEWS:
			joinpath(logfile, rcdir, "log");

			if (no_write || (fp_log = fopen(logfile, "w" FOPEN_OPTS)) == NULL) {
				perror_message(_(txt_cannot_open), logfile);
				fp_log = stdout;
				verbose = FALSE;
				log_opened = FALSE;
			}
			fprintf(fp_log, "To: %s\n", userid);
			(void) time(&epoch);
			snprintf(subject, sizeof(subject), "Subject: NEWS LOG %s", ctime(&epoch));
			fprintf(fp_log, "%s\n", subject);	/* ctime() includes a \n too */
			break;

		default:
			break;
	}

	/*
	 * For each group we subscribe to...
	 */
	for (i = 0; i < selmenu.max; i++) {
		art_count = hot_count = 0;
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

		if (!index_group(group))
			continue;

		if (function == MAIL_ANY_NEWS || function == SAVE_ANY_NEWS) {
			if (!group->attribute->batch_save)
				continue;

			group_count++;
			snprintf(buf, sizeof(buf), _(txt_saved_groupname), group->name);
			fprintf(fp_log, buf);
			if (verbose)
				wait_message(0, buf);

			if (function == SAVE_ANY_NEWS) {
				char tmp[PATH_LEN];

				if (!strfpath(group->attribute->savedir, tmp, sizeof(tmp), group))
					joinpath(tmp, homedir, DEFAULT_SAVEDIR);

				make_group_path(group->name, group_path);
				joinpath(path, tmp, group_path);
				create_path(path);	/* TODO error handling */
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
					if (print_first) {
						my_fputc('\n', stdout);
						print_first = FALSE;
					}
					if (!verbose && !catchup) /* we don't need details */
						return 2;
					art_count++;
					if (arts[j].score >= tinrc.score_select)
						hot_count++;
					if (catchup)
						art_mark(group, &arts[j], ART_READ);
					break;

				case START_ANY_NEWS:
					return i;	/* return first group with unread news */
					/* NOTREACHED */

				case MAIL_ANY_NEWS:
				case SAVE_ANY_NEWS:
					if ((artfp = open_art_fp(group, arts[j].artnum)) == NULL)
						continue;

					if (function == SAVE_ANY_NEWS) {
						snprintf(buf, sizeof(buf), "%ld", arts[j].artnum);
						joinpath(savefile, path, buf);
					}

					if ((savefp = fopen(savefile, "w" FOPEN_OPTS)) == NULL) {
						fprintf(fp_log, _(txt_cannot_open), savefile);
						if (verbose)
							perror_message(_(txt_cannot_open), savefile);
						TIN_FCLOSE(artfp);
						continue;
					}

					if ((function == MAIL_ANY_NEWS) && ((INTERACTIVE_NONE == tinrc.interactive_mailer) || (INTERACTIVE_WITH_HEADERS == tinrc.interactive_mailer))) {
						fprintf(savefp, "To: %s\n", mail_news_user);
						fprintf(savefp, "Subject: %s\n", arts[j].subject);
						/*
						 * Reuse some headers to allow threading in mail reader
						 */
						if (arts[j].msgid)
							fprintf(savefp, "Message-ID: %s\n", arts[j].msgid);
						/* fprintf(savefp, "References: %s\n", arts[j].refs); */
						/*
						 * wrap article in appropriate MIME type
						 */
						fprintf(savefp, "MIME-Version: 1.0\n");
						fprintf(savefp, "Content-Type: message/rfc822\n");
						/*
						 * CTE should be 7bit if the article is in pure
						 * US-ASCII, but this requires previous parsing
						 */
						fprintf(savefp, "Content-Transfer-Encoding: 8bit\n\n");
					}

					snprintf(buf, sizeof(buf), "[%5ld]  %s\n", arts[j].artnum, arts[j].subject);
					fprintf(fp_log, "%s", buf);		/* buf may contain % */
					if (verbose)
						wait_message(0, buf);

					while ((line = tin_fgets(artfp, FALSE)) != NULL)
						fprintf(savefp, "%s\n", line);		/* TODO: error handling */

					TIN_FCLOSE(artfp);
					fclose(savefp);
					saved_arts++;

					if (function == MAIL_ANY_NEWS) {
						strfmailer(mailer, arts[j].subject, mail_news_user, savefile, buf, sizeof(buf), tinrc.mailer_format);
						invoke_cmd(buf);		/* Keep trying after errors */
						unlink(savefile);
					}
					if (catchup)
						art_mark(group, &arts[j], ART_READ);
					break;

				default:
					break;
			}
		}

		if (art_count) {
			unread_news = TRUE;
			if (verbose)
				wait_message(0, _(txt_saved_group), art_count, hot_count,
					PLURAL(art_count, txt_article), group->name);
		}
	}

	switch (function) {
		case CHECK_ANY_NEWS:
			/*
			 * TODO: shall we return 2 or 0 in the -cZ case?
			 */
			if (unread_news && !catchup)
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
			snprintf(buf, sizeof(buf), _(txt_saved_summary), (function == MAIL_ANY_NEWS ? _(txt_mailed) : _(txt_saved)),
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
	struct stat st;
	t_function func;

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
/* TODO: will this get called every art? Should only be done once/batch */
/* TODO: or add an option for defaulting on all future queries */
/* TODO: 'truncate' path if query exceeds screen-width */
		func = prompt_slk_response((tinrc.default_save_mode == 'a' ? SAVE_APPEND_FILE : SAVE_OVERWRITE_FILE),
				save_append_overwrite_keys,
				_(txt_append_overwrite_quit), path,
				printascii(keyappend, func_to_key(SAVE_APPEND_FILE, save_append_overwrite_keys)),
				printascii(keyoverwrite, func_to_key(SAVE_OVERWRITE_FILE, save_append_overwrite_keys)),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, save_append_overwrite_keys)));

		switch (func) {
			case SAVE_OVERWRITE_FILE:
				strcpy(mode, "w");
				break;

			case GLOBAL_ABORT:
			case GLOBAL_QUIT:
				wait_message(1, _(txt_art_not_saved));
				return NULL;

			default:	/* SAVE_APPEND_FILE */
				break;
		}
		if (func == SAVE_OVERWRITE_FILE)
			tinrc.default_save_mode = 'o';
		else
			tinrc.default_save_mode = 'a';
	}

	if ((fp = fopen(path, mode)) == NULL) {
		perror_message("%s (%s)", _(txt_art_not_saved), path);
		return NULL;
	}

	return fp;
}


/*
 * This is where an article is actually copied to disk and processed
 * We only need to copy the art to disk if we are doing post-processing
 * 'artinfo' is parsed/cooked article to be saved
 * 'artptr' points to the article in arts[]
 * 'mailbox' is set if we are saving to a =mailbox
 * 'inpath' is the template save path/file to save to
 * 'max' is the number of articles we are saving
 * 'post_process' is set if we want post-processing
 * Expand the path appropriately, taking account of multiple file
 * extensions and the auto-save with Archive-Name: headers
 *
 * Extract binary attachments if !LIBUU
 * Start viewer if requested
 * If successful, add entry to the save[] array
 * Returns:
 *     TRUE or FALSE depending on whether article was saved okay.
 *
 * TODO: could we use append_mail() here
 */
t_bool
save_and_process_art(
	t_openartinfo *artinfo,
	struct t_article *artptr,
	t_bool is_mailbox,
	const char *inpath,
	int max,
	t_bool post_process)
{
	FILE *fp;
	char from[HEADER_LEN];
	char path[PATH_LEN];
	time_t epoch;
	t_bool mmdf = (is_mailbox && !strcasecmp(txt_mailbox_formats[tinrc.mailbox_format], "MMDF"));

	if (fseek(artinfo->raw, 0L, SEEK_SET) == -1) {
		perror_message(txt_error_fseek, artinfo->hdr.subj);
		return FALSE;
	}

	/* The first task is to fixup the filename to be saved too. This is context dependent */
	strncpy(path, inpath, sizeof(path) - 1);
/*fprintf(stderr, "save_and_process_art max=%d num_save=%d starting path=(%s) postproc=%s\n", max, num_save, path, bool_unparse(post_process));*/

	/*
	 * If using the auto-save feature on an article with Archive-Name,
	 * the path will be: <original-path>/<archive-name>/<part|patch><part#>
	 */
	if (!is_mailbox && curr_group->attribute->auto_save && artptr->archive) {
		const char *partprefix;
		char *ptr;
		char archpath[PATH_LEN];
		char filename[NAME_LEN];

		/*
		 * We need either a part or a patch number, part takes precedence
		 */
		if (artptr->archive->ispart)
			partprefix = PATH_PART;
		else
			partprefix = PATH_PATCH;

		/*
		 * Strip off any existing filename
		 */
		if ((ptr = strrchr(path, DIRSEP)) != NULL)
			*(ptr + 1) = '\0';

		/* Add on the archive name as a directory */
		/* TODO: maybe a s!/!.! on archive-name would be better */
		joinpath(archpath, path, artptr->archive->name);

		/* Generate the filename part and append it */
		snprintf(filename, sizeof(filename), "%s%s", partprefix, artptr->archive->partnum);
		joinpath(path, archpath, filename);
/*fprintf(stderr, "save_and_process_art archive-name mangled path=(%s)\n", path);*/
		if (!create_path(path))
			return FALSE;
	} else {
		/*
		 * Mailbox saves are by definition to a single file as are single file
		 * saves. Multiple file saves append a .NNN sequence number to the path
		 * This is backward-contemptibility with older versions of tin
		 */
		if (!is_mailbox && max > 1) {
#ifdef VMS
			const char suffixsep = '-';		/* Suffix seperator for .001 type extensions */
#else
			const char suffixsep = '.';
#endif /* VMS */

			sprintf(&path[strlen(path)], "%c%03d", suffixsep, num_save + 1);
		}
	}

/*fprintf(stderr, "save_and_process_art expanded path now=(%s)\n", path);*/

	if ((fp = open_save_filename(path, is_mailbox)) == NULL)
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

	if (copy_fp(artinfo->raw, fp)) /* Write tailing newline or MMDF-mailbox seperator */
		print_art_seperator_line(fp, is_mailbox);
	else {
		fclose(fp);
		unlink(path);
		return FALSE;
	}

	fclose(fp);

	/*
	 * Saved ok, so fill out a save[] record
	 */
	if (num_save == max_save - 1)
		expand_save();
	save[num_save].path = my_strdup(path);
	save[num_save].file = strrchr(save[num_save].path, DIRSEP) + 1;	/* ptr to filename portion */
	save[num_save].mailbox = is_mailbox;
/*fprintf(stderr, "SAPA (%s) (%s) mbox=%s\n", save[num_save].path, save[num_save].file, bool_unparse(save[num_save].mailbox));*/
	num_save++;			/* NB: num_save is bumped here only */

	/*
	 * Extract/view parts from multipart articles if required
	 * libuu does this as part of it's own processing
	 */
#ifndef HAVE_LIBUU
	if (post_process) {
#	ifdef USE_CURSES
		scrollok(stdscr, TRUE);
#	endif /* USE_CURSES */
		decode_save_mime(artinfo, TRUE);
#	ifdef USE_CURSES
		scrollok(stdscr, FALSE);
#	endif /* USE_CURSES */
	}
#endif /* !HAVE_LIBUU */

	return TRUE;
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
#ifndef VMS
	char buf[PATH_LEN];
	int i, j, len;
	struct stat st;

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
	if (my_mkdir(path, (mode_t) (S_IRWXU|S_IRUGO|S_IXUGO)) == -1) {
		if (errno != EEXIST) {
			perror_message(_(txt_cannot_create), buf);
			return FALSE;
		}
	}
#endif /* !VMS */

	return TRUE;
}


/*
 * Generate semi-meaningful filename based on sequence number and
 * Content-(sub)type
 */
static void
generate_filename(
	char *buf,
	int buflen,
	const char *suffix)
{
	static int seqno = 0;

	snprintf(buf, buflen, "%s-%03d.%s", SAVEFILE_PREFIX, seqno++, suffix);
}


/*
 * Generate a path/filename to save to, using 'path' as input.
 * The pathname is stored in 'outpath', which should be PATH_LEN in size
 * Expand metacharacters and use defaults as needed.
 * Return TRUE if the path is a mailbox, or FALSE otherwise.
 */
t_bool
expand_save_filename(
	char *outpath,
	const char *path)
{
	int ret = strfpath(path, outpath, PATH_LEN, curr_group);

	/*
	 * If no path exists or the above failed in some way, use sensible defaults
	 * Put the generic path into 'outpath'
	 */
	if ((ret == 0) || !(strrchr(outpath, DIRSEP))) {
		char buf[PATH_LEN];

		if (!strfpath(curr_group->attribute->savedir, buf, sizeof(buf), curr_group))
			joinpath(buf, homedir, DEFAULT_SAVEDIR);
		joinpath(outpath, buf, path);
		return FALSE;
	} else
		return (ret == 1);
}


/*
 * Post process the articles in save[] according to proc_type_ch
 * auto_delete is set if we should remove the saved files after processing
 * This stage can produce a fair bit of output so we allow it to
 * scroll up the screen rather than waste time displaying it in the
 * message bar
 */
t_bool
post_process_files(
	t_function proc_type_func,
	t_bool auto_delete)
{
	if (num_save < 1)
		return FALSE;

	clear_message();
#ifdef USE_CURSES
	scrollok(stdscr, TRUE);
#endif /* USE_CURSES */
	my_printf("%s%s", _(txt_post_processing), cCRLF);

	switch (proc_type_func) {
		case POSTPROCESS_SHAR:
			post_process_sh();
			break;

		/* This is the default, eg, with AUTOSAVE */
		case POSTPROCESS_YES:
		default:
			post_process_uud();
			break;
	}

	my_printf("%s%s%s", _(txt_post_processing_finished), cCRLF, cCRLF);
	my_flush();
	prompt_continue();
#ifdef USE_CURSES
	scrollok(stdscr, FALSE);
#endif /* USE_CURSES */

	/*
	 * Remove the post-processed files if required
	 */
	my_printf(cCRLF);
	my_flush();

	if (auto_delete) {
		int i;

		my_printf("%s%s", _(txt_deleting), cCRLF);
		my_flush();

		for (i = 0; i < num_save; i++)
			unlink(save[i].path);
	}

	return TRUE;
}


/*
 * Two implementations .....
 * The LIBUU case performs multi-file decoding for uue, base64
 * binhex, qp. This is handled entirely during the post processing phase
 *
 * The !LIBUU case only handles multi-file uudecoding, the other MIME
 * types were handled using the internal MIME parser when the articles
 * were originally saved
 */
#ifdef HAVE_LIBUU
static void
post_process_uud(
	void)
{
	FILE *fp_in;
	char file_out_dir[PATH_LEN];
	const char *eptr;
	int i;
	int count;
	int errors = 0;
	uulist *item;

	/*
	 * Grab the dirname portion
	 */
	my_strncpy(file_out_dir, save[0].path, save[0].file - save[0].path);

	UUInitialize();

	UUSetOption(UUOPT_SAVEPATH, 0, file_out_dir);
	for (i = 0; i < num_save; i++) {
		if ((fp_in = fopen(save[i].path, "r")) != NULL) {
			UULoadFile(save[i].path, NULL, 0);	/* Scans file for encoded data */
			fclose(fp_in);
		}
	}

#	if 0
	/*
	 * uudeview's "intelligent" multi-part detection
	 * From the uudeview docs: This function is a bunch of heuristics, and I
	 * don't really trust them... should only be called as a last resort on
	 * explicit user request
	 */
	UUSmerge(0);
	UUSmerge(1);
	UUSmerge(99);
#	endif /* 0 */

	i = count = 0;
	item = UUGetFileListItem(i);
	my_printf(cCRLF);

	while (item != NULL) {
		if (UUDecodeFile(item, NULL) == UURET_OK) {
			char path[PATH_LEN];

/* TODO: test for multiple things per article decoded okay? */
			count++;
			my_printf(_(txt_uu_success), item->filename);
			my_printf(cCRLF);

			/* item->mimetype seems not to be available for uudecoded files etc */
			if (tinrc.post_process_view) {
				joinpath(path, file_out_dir, item->filename);
				view_file(path, strrchr(path, DIRSEP) + 1);
			}
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

	return;
}

#else

/*
 * Open and read all the files in save[]
 * Scan for uuencode BEGIN lines, decode input as we go along
 * uuencoded data can span multiple files, and multiple uuencoded
 * files are supported per batch
 */
static void
post_process_uud(
	void)
{
	FILE *fp_in;
	FILE *fp_out = NULL;
	char *filename = NULL;
	char file_out_dir[PATH_LEN];
	char path[PATH_LEN];
	char s[LEN], t[LEN], u[LEN];
	int state = INITIAL;
	int i;
	mode_t mode = 0;

	/*
	 * Grab the dirname portion
	 */
	my_strncpy(file_out_dir, save[0].path, save[0].file - save[0].path);

	t[0] = '\0';
	u[0] = '\0';

	for (i = 0; i < num_save; i++) {
		if ((fp_in = fopen(save[i].path, "r")) == NULL)
			continue;

		while (fgets(s, (int) sizeof(s), fp_in) != 0) {
			switch (state) {
				case INITIAL:
					if (strncmp("begin ", s, 6) == 0) {
						char fmt[15];
						char name[PATH_LEN];
						char buf[PATH_LEN];

						snprintf(fmt, sizeof(fmt), "%%o %%%dc\\n", PATH_LEN - 1);
						if (sscanf(s + 6, fmt, &mode, name) == 2) {
							strtok(name, "\n");
							my_strncpy(buf, name, sizeof(buf) - 1);
							str_trim(buf);
							base_name(buf, name);
						} else
							name[0] = '\0';

						if (!mode && !*name) { /* not a valid uu-file at all */
							state = INITIAL;
							continue;
						}

						if (!*name)
							generate_filename(name, sizeof(name), "uue");

						filename = name;
						expand_save_filename(path, filename);
						filename = strrchr(path, DIRSEP) + 1;	/* ptr to filename portion */
						if ((fp_out = fopen(path, "w")) == NULL) {
							perror_message(_(txt_cannot_open), path);
							return;
						}
						state = MIDDLE;
					}
					break;

				case MIDDLE:
					/*
					 * TODO: replace hardcoded length check (uue lines are not
					 *       required to be 60 chars long (45 encoded chars)
					 *       ('M' == 60 * 3 / 4 + ' ' == 77))
					 */
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

				fchmod(fileno(fp_out), mode);

				fclose(fp_out);
				fp_out = NULL;

				my_printf(_(txt_uu_success), filename);
				my_printf(cCRLF);
				sum_file(path, filename);
				if (tinrc.post_process_view)
					view_file(path, filename);
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
	return;
}


/*
 * Sum file - why do we bother to do this?
 * nuke code or add DONT_HAVE_PIPING and !M_UNIX -tree
 */
static void
sum_file(
	const char *path,
	const char *file)
{
#	if defined(M_UNIX) && defined(HAVE_SUM) && !defined(DONT_HAVE_PIPING)
	FILE *fp_in;
	char *ext;
	char buf[LEN];

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
}
#endif /* HAVE_LIBUU */


/*
 * If defined, invoke post processor command
 * Create a part structure, with defaults, insert a parameter for the name
 */
static void
view_file(
	const char *path,
	const char *file)
{
	char *ext;
	t_part *part;

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

	start_viewer(part, path);
	my_printf(cCRLF);

	free_parts(part);
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
			ch = ((DEC(p[0]) << 2) | (DEC(p[1]) >> 4));
			fputc(ch, fp);
			ch = ((DEC(p[1]) << 4) | (DEC(p[2]) >> 2));
			fputc(ch, fp);
			ch = ((DEC(p[2]) << 6) | DEC(p[3]));
			fputc(ch, fp);
		} else {
			if (n >= 1) {
				ch = ((DEC(p[0]) << 2) | (DEC(p[1]) >> 4));
				fputc(ch, fp);
			}
			if (n >= 2) {
				ch = ((DEC(p[1]) << 4) | (DEC(p[2]) >> 2));
				fputc(ch, fp);
			}
		}
	}
	return;
}


/*
 * Unpack /bin/sh archives
 * There is no end-of-shar marker so the code reads everything after
 * the start marker. This is why shar is handled seperately.
 * The code assumes shar archives do not span articles
 */
static void
post_process_sh(
	void)
{
	FILE *fp_in, *fp_out = NULL;
	char buf[LEN];
	char file_out[PATH_LEN];
	char file_out_dir[PATH_LEN];
	int i;

	/*
	 * Grab the dirname portion
	 */
	my_strncpy(file_out_dir, save[0].path, save[0].file - save[0].path);

#ifdef VMS
	snprintf(file_out, sizeof(file_out), "%ssh.%05d", file_out_dir, (int) process_id);
#else
	snprintf(file_out, sizeof(file_out), "%ssh%05d", file_out_dir, (int) process_id);
#endif /* VMS */

	for (i = 0; i < num_save; i++) {
		if ((fp_in = fopen(save[i].path, "r")) == NULL)
			continue;

		wait_message(1, _(txt_extracting_shar), save[i].path);

		while (fgets(buf, (int) sizeof(buf), fp_in) != NULL) {
			/* find #!/bin/sh style patterns */
			if ((fp_out == NULL) && pcre_exec(shar_regex.re, shar_regex.extra, buf, strlen(buf), 0, 0, NULL, 0) >= 0)
				fp_out = fopen(file_out, "w" FOPEN_OPTS);

			/* write to temp file */
			if (fp_out != NULL)
				fputs(buf, fp_out);
		}
		fclose(fp_in);

		if (fp_out == NULL)			/* Didn't extract any shar */
			continue;

		fclose(fp_out);
		fp_out = NULL;
#ifndef M_UNIX
		make_post_process_cmd(DEFAULT_UNSHAR, file_out_dir, file_out);
#else
		sh_format(buf, sizeof(buf), "cd %s; sh %s", file_out_dir, file_out);
		my_fputs(cCRLF, stdout);
		my_flush();
		invoke_cmd(buf);			/* Handles its own errors */
#endif /* !M_UNIX */
		unlink(file_out);
	}
	return;
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


/*
 * part needs to have at least content type/subtype and a filename
 * path = full path/file (used for substitution in mailcap entries)
 */
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
		snprintf(buff, sizeof(buff), "(%s)", foo->command);
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
		wait_message(1, _(txt_no_viewer_found), content_types[part->type], part->subtype);
}


/*
 * Decode and save the binary object pointed to in 'part'
 * Optionally launch a viewer for it
 * Return FALSE if Abort used to skip further viewing/saving
 * or other terminal error occurs
 */
static t_bool
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
	t_bool mbox;

	/*
	 * Decode this message part if appropriate
	 */
	if (!(check_save_mime_type(part, curr_group->attribute->mime_types_to_save))) {
		/* TODO: skip message if saving multiple files (e.g. save 't'agged) */
		wait_message(1, "Skipped %s/%s", content_types[part->type], part->subtype);	/* TODO: better msg */
		return TRUE;
	}

	/*
	 * Get the filename to save to in 'savepath'
	 */
	if ((name = get_filename(part->params)) == NULL) {
		char extension[NAME_LEN + 1];

		lookup_extension(extension, sizeof(extension), content_types[part->type], part->subtype);
		generate_filename(buf, sizeof(buf), extension);
		mbox = expand_save_filename(savepath, buf);
	} else
		mbox = expand_save_filename(savepath, name);

	/*
	 * Not a good idea to dump attachments over a mailbox
	 */
	if (mbox) {
		wait_message(2, _(txt_is_mailbox), content_types[part->type], part->subtype);
		return FALSE;
	}

	if (!(create_path(savepath))) {
		error_message(_(txt_cannot_open_for_saving), savepath);
		return FALSE;
	}

	/*
	 * Decode/save the attachment
	 */
	if ((fp = open_save_filename(savepath, FALSE)) == NULL) {
		error_message(_(txt_cannot_open_for_saving), savepath);
		return FALSE;
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
				/* TODO: if postproc, don't decode these since the traditional uudecoder will get them */
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
		snprintf(buf, sizeof(buf), _(txt_view_attachment), savepath, content_types[part->type], part->subtype);
		if ((i = prompt_yn(buf, TRUE)) == 1)
			start_viewer(part, savepath);
		else if (i == -1) {	/* Skip rest of attachments */
			unlink(savepath);
			return FALSE;
		}
	}

	/*
	 * Save the attachment
	 */
	if (postproc && tinrc.post_process_view) {
		my_printf(_(txt_uu_success), savepath);
		my_printf(cCRLF);
	}
	if (!postproc) {
		snprintf(buf, sizeof(buf), _(txt_save_attachment), savepath, content_types[part->type], part->subtype);
		if ((i = prompt_yn(buf, FALSE)) != 1) {
			unlink(savepath);
			if (i == -1)	/* Skip rest of attachments */
				return FALSE;
		}
	}
	return TRUE;
}


enum match { NO, MATCH, NOTMATCH };

/*
 * Match a single type/subtype Content pair
 * Returns:
 * NO = Not matched
 * MATCH = Matched
 * NOTMATCH = Matched, but !negated
 */
static int
match_content_type(
	t_part *part,
	char *type)
{
	char *subtype;
	int typeindex;
	t_bool found = FALSE;
	t_bool negate = FALSE;

	/* Check for negation */
	if (*type == '!') {
		negate = TRUE;
		++type;

		if (!*type)				/* Invalid type */
			return NO;
	}

	/* Split type and subtype */
	if ((subtype = strchr(type, '/')) == NULL)
		return NO;
	*(subtype++) = '\0';

	if (!*type || !*subtype)	/* Missing type or subtype */
		return NO;

	/* Try and match major */
	if (strcmp(type, "*") == 0)
		found = TRUE;
	else if (((typeindex = content_type(type)) != -1) && typeindex == part->type)
		found = TRUE;

	if (!found)
		return NO;

	/* Try and match subtype */
	found = FALSE;
	if (strcmp(subtype, "*") == 0)
		found = TRUE;
	else if (strcmp(subtype, part->subtype) == 0)
		found = TRUE;

	if (!found)
		return NO;

	/* We got a match */
	if (negate)
		return NOTMATCH;

	return MATCH;
}


/*
 * See if the mime type of this part matches the list of content types to save
 * or ignore. Return TRUE if there is a match
 * mime_types is a comma separated list of type/subtype pairs. type and/or
 * subtype can be a '*' to match any, and a pair can begin with a ! which
 * will negate the meaning. We eval all pairs, the rightmost match will
 * prevail
 */
static t_bool
check_save_mime_type(
	t_part *part,
	const char *mime_types)
{
	char *ptr, *pair;
	int found;
	int retcode;

	if (!mime_types)
		return FALSE;

	ptr = my_strdup(mime_types);

	pair = strtok(ptr, ",");
	retcode = match_content_type(part, pair);

	while ((pair = strtok(NULL, ",")) != NULL) {
		if ((found = match_content_type(part, pair)) != NO)
			retcode = found;
	}

	free(ptr);
	return (retcode == MATCH);
}


/*
 * decode and save binary MIME attachments from an open article context
 * optionally locate and launch a viewer application
 * 'postproc' determines the mode of the operation and will be set to
 * TRUE when we're called during a [Ss]ave operation and FALSE when
 * when just viewing
 * When it is TRUE the view option will depend on post_process_view and
 * the save is implicit. Feedback will also be printed.
 * When it is FALSE then the view/save options will be queried
 */
void
decode_save_mime(
	t_openartinfo *art,
	t_bool postproc)
{
	t_part *ptr, *uueptr;

	/*
	 * Iterate over all the attachments
	 */
	for (ptr = art->hdr.ext; ptr != NULL; ptr = ptr->next) {
		/*
		 * Handle uuencoded sections in this message part.
		 * Only works when the uuencoded file is entirely within the current
		 * article.
		 * We don't do this when postprocessing as the generic uudecode code
		 * already handles uuencoded data, but TODO: review this
		 */
		if (!postproc) {
			for (uueptr = ptr->uue; uueptr != NULL; uueptr = uueptr->next) {
				if (!(decode_save_one(uueptr, art->raw, postproc)))
					break;
			}
		}

		/*
		 * TYPE_MULTIPART is an envelope type, don't process it.
		 * If we had an UUE part, the "surrounding" text/plain plays
		 * the role of a multipart part. Check to see if we want to
		 * save text and if not, skip this part.
		 */
		if ((ptr->type == TYPE_MULTIPART || ((NULL != ptr->uue) && (!check_save_mime_type(ptr, curr_group->attribute->mime_types_to_save)))))
			continue;

		if (!(decode_save_one(ptr, art->raw, postproc)))
			break;
	}
}
