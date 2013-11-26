/*
 *  Project   : tin - a Usenet reader
 *  Module    : save.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2013-11-21
 *  Notes     :
 *
 * Copyright (c) 1991-2013 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

#ifndef HAVE_LIBUU
#undef OFF
enum state { INITIAL, MIDDLE, OFF, END };
#endif /* !HAVE_LIBUU */

enum action {
	VIEW,
	SAVE,
	SAVE_TAGGED
#ifndef DONT_HAVE_PIPING
	, PIPE_RAW
	, PIPE
#endif /* !DONT_HAVE_PIPING */
};

/*
 * Local prototypes
 */
static FILE *open_save_filename(const char *path, t_bool mbox);
static char *build_tree(int depth, int maxlen, int i);
static char *generate_savepath(t_part *part);
static int build_part_list(t_openartinfo *art);
static int get_tagged(int n);
static int match_content_type(t_part *part, char *type);
static t_bool check_save_mime_type(t_part *part, const char *mime_types);
static t_bool decode_save_one(t_part *part, FILE *rawfp, t_bool postproc);
static t_bool expand_save_filename(char *outpath, size_t outpath_len, const char *path);
static t_bool tag_part(int n);
static t_function attachment_left(void);
static t_function attachment_right(void);
static t_partl *find_part(int n);
static void build_attachment_line(int i);
static void draw_attachment_arrow(void);
static void free_part_list(t_partl *list);
static void generate_filename(char *buf, int buflen, const char *suffix);
#ifndef DONT_HAVE_PIPING
	static void pipe_part(const char *savepath);
#endif /* !DONT_HAVE_PIPING */
static void post_process_uud(void);
static void post_process_sh(void);
static void process_part(t_part *part,	FILE *rawfp, FILE *outfile, const char *savepath, enum action what);
static void process_parts(t_part *part,	FILE *rawfp, enum action what);
static void show_attachment_page(void);
static void start_viewer(t_part *part, const char *path);
static void tag_pattern(void);
static void untag_all_parts(void);
static void untag_part(int n);
static void uudecode_line(const char *buf, FILE *fp);
static void view_file(const char *path, const char *file);
#ifndef HAVE_LIBUU
	static void sum_file(const char *path, const char *file);
#endif /* !HAVE_LIBUU */

static int num_of_tagged_parts, info_len;
static t_menu attmenu = { 0, 0, 0, show_attachment_page, draw_attachment_arrow, build_attachment_line };
static t_partl *part_list;

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
			joinpath(savefile, sizeof(savefile), TMPDIR, "tin");
#ifdef APPEND_PID
			snprintf(savefile + strlen(savefile), sizeof(savefile) - strlen(savefile), ".%ld", (long) process_id);
#endif /* APPEND_PID */
			/* FALLTHROUGH */

		case SAVE_ANY_NEWS:
			joinpath(logfile, sizeof(logfile), rcdir, "log");

			if (no_write || (fp_log = fopen(logfile, "w")) == NULL) {
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

		if (function == MAIL_ANY_NEWS || function == SAVE_ANY_NEWS) {
			if (!group->attribute->batch_save)
				continue;

			group_count++;
			snprintf(buf, sizeof(buf), _(txt_saved_groupname), group->name);
			fprintf(fp_log, "%s", buf);
			if (verbose)
				wait_message(0, buf);

			if (function == SAVE_ANY_NEWS) {
				char tmp[PATH_LEN];

				if (!strfpath(cmdline.args & CMDLINE_SAVEDIR ? cmdline.savedir : tinrc.savedir, tmp, sizeof(tmp), group, FALSE))
					joinpath(tmp, sizeof(tmp), homedir, DEFAULT_SAVEDIR);

				make_group_path(group->name, group_path);
				joinpath(path, sizeof(path), tmp, group_path);
				create_path(path);	/* TODO error handling */
			}
		}

		if (!index_group(group))
			continue;

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
						return NEWS_AVAIL_EXIT;
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
						snprintf(buf, sizeof(buf), "%"T_ARTNUM_PFMT, arts[j].artnum);
						joinpath(savefile, sizeof(savefile), path, buf);
					}

					if ((savefp = fopen(savefile, "w")) == NULL) {
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

					snprintf(buf, sizeof(buf), "[%5"T_ARTNUM_PFMT"]  %s\n", arts[j].artnum, arts[j].subject);
					fprintf(fp_log, "%s", buf);	/* buf may contain % */
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
				return NEWS_AVAIL_EXIT;
			else {
				if (verbose)
					wait_message(1, _(txt_there_is_no_news));
				return EXIT_SUCCESS;
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
/* fprintf(stderr, "save_and_process_art max=%d num_save=%d starting path=(%s) postproc=%s\n", max, num_save, path, bool_unparse(post_process)); */

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
		joinpath(archpath, sizeof(archpath), path, artptr->archive->name);

		/* Generate the filename part and append it */
		snprintf(filename, sizeof(filename), "%s%s", partprefix, artptr->archive->partnum);
		joinpath(path, sizeof(path), archpath, filename);
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
			const char suffixsep = '.';

			sprintf(&path[strlen(path)], "%c%03d", suffixsep, num_save + 1);
		}
	}

/* fprintf(stderr, "save_and_process_art expanded path now=(%s)\n", path); */

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

	if (copy_fp(artinfo->raw, fp)) /* Write tailing newline or MMDF-mailbox separator */
		print_art_separator_line(fp, is_mailbox);
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
/* fprintf(stderr, "SAPA (%s) (%s) mbox=%s\n", save[num_save].path, save[num_save].file, bool_unparse(save[num_save].mailbox)); */
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
 * Generate /save/to/path name.
 *
 * Return pointer to allocated memory which the caller must free or
 * NULL if something went wrong.
 */
static char *
generate_savepath(
	t_part *part)
{
	char buf[2048];
	char *savepath;
	const char *name;
	t_bool mbox;

	savepath = my_malloc(PATH_LEN + 1);
	/*
	 * Get the filename to save to in 'savepath'
	 */
	if ((name = get_filename(part->params)) == NULL) {
		char extension[NAME_LEN + 1];

		lookup_extension(extension, sizeof(extension), content_types[part->type], part->subtype);
		generate_filename(buf, sizeof(buf), extension);
		mbox = expand_save_filename(savepath, PATH_LEN, buf);
	} else
		mbox = expand_save_filename(savepath, PATH_LEN, name);

	/*
	 * Not a good idea to dump attachments over a mailbox
	 */
	if (mbox) {
		wait_message(2, _(txt_is_mailbox), content_types[part->type], part->subtype);
		free(savepath);
		return NULL;
	}

	if (!(create_path(savepath))) {
		error_message(2, _(txt_cannot_open_for_saving), savepath);
		free(savepath);
		return NULL;
	}

	return savepath;
}


/*
 * Generate a path/filename to save to, using 'path' as input.
 * The pathname is stored in 'outpath', which should be PATH_LEN in size
 * Expand metacharacters and use defaults as needed.
 * Return TRUE if the path is a mailbox, or FALSE otherwise.
 */
static t_bool
expand_save_filename(
	char *outpath,
	size_t outpath_len,
	const char *path)
{
	char base_filename[PATH_LEN];
	char buf[PATH_LEN];
	char buf_path[PATH_LEN];
	int ret;

	/*
	 * Make sure that externally supplied filename is a filename only and fits
	 * into buffer
	 */
	STRCPY(buf_path, path);
	base_name(buf_path, base_filename);

	/* Build default path to save to */
	if (!(ret = strfpath(cmdline.args & CMDLINE_SAVEDIR ? cmdline.savedir : curr_group->attribute->savedir, buf, sizeof(buf), curr_group, FALSE)))
		joinpath(buf, sizeof(buf), homedir, DEFAULT_SAVEDIR);

	/* Join path and filename */
	joinpath(outpath, outpath_len, buf, base_filename);

	return (ret == 1);	/* should now always evaluate to FALSE */
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

	EndWin();
	Raw(FALSE);
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
#ifdef USE_CURSES
	Raw(TRUE);
	InitWin();
#endif /* USE_CURSES */
	prompt_continue();
#ifndef USE_CURSES
	Raw(TRUE);
	InitWin();
#endif /* !USE_CURSES */

	/*
	 * Remove the post-processed files if required
	 */
	if (auto_delete) {
		int i;

		wait_message((tinrc.beginner_level) ? 2 : 1, "%s", _(txt_deleting));
		cursoroff();

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
			if (curr_group->attribute->post_process_view) {
				joinpath(path, sizeof(path), file_out_dir, item->filename);
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
						expand_save_filename(path, sizeof(path), filename);
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
					} else	/* end */
						state = OFF;	/* OFF => a break in the uuencoded data */
					break;

				case OFF:
					if ((s[0] == 'M') && (t[0] == 'M') && (u[0] == 'M')) {
						uudecode_line(u, fp_out);
						uudecode_line(t, fp_out);
						uudecode_line(s, fp_out);
						state = MIDDLE;	/* Continue output of previously suspended data */
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
				if (curr_group->attribute->post_process_view)
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
 * the start marker. This is why shar is handled separately.
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
	snprintf(file_out, sizeof(file_out), "%ssh%ld", file_out_dir, (long) process_id);

	for (i = 0; i < num_save; i++) {
		if ((fp_in = fopen(save[i].path, "r")) == NULL)
			continue;

		wait_message(1, _(txt_extracting_shar), save[i].path);

		while (fgets(buf, (int) sizeof(buf), fp_in) != NULL) {
			/* find #!/bin/sh style patterns */
			if ((fp_out == NULL) && pcre_exec(shar_regex.re, shar_regex.extra, buf, strlen(buf), 0, 0, NULL, 0) >= 0)
				fp_out = fopen(file_out, "w");

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
 * write tailing (MMDF)-mailbox separator
 */
void
print_art_separator_line(
	FILE *fp,
	t_bool is_mailbox)
{
#ifdef DEBUG
	if (debug & DEBUG_MISC)
		error_message(2, "Mailbox=[%d], mailbox_format=[%s]", is_mailbox, txt_mailbox_formats[tinrc.mailbox_format]);
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
		if (foo->nametemplate)	/* honor nametemplate */
			rename_file(path, foo->nametemplate);

		wait_message(0, _(txt_starting_command), foo->command);
		if (foo->needsterminal) {
			set_xclick_off();
			fflush(stdout);
		} else {
			if (foo->description)
				info_message(foo->description);
		}
		invoke_cmd(foo->command);
		if (foo->needsterminal) {
#ifndef USE_CURSES
			EndWin();
			Raw(FALSE);
#endif /* !USE_CURSES */
			prompt_continue();
#ifndef USE_CURSES
			Raw(TRUE);
			InitWin();
#endif /* !USE_CURSES */
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
	char *savepath;
	int count;
	int i;

	/*
	 * Decode this message part if appropriate
	 */
	if (!(check_save_mime_type(part, curr_group->attribute->mime_types_to_save))) {
		/* TODO: skip message if saving multiple files (e.g. save 't'agged) */
		wait_message(1, "Skipped %s/%s", content_types[part->type], part->subtype);	/* TODO: better msg */
		return TRUE;
	}

	if ((savepath = generate_savepath(part)) == NULL)
		return FALSE;

	/*
	 * Decode/save the attachment
	 */
	if ((fp = open_save_filename(savepath, FALSE)) == NULL) {
		error_message(2, _(txt_cannot_open_for_saving), savepath);
		free(savepath);
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
	if ((postproc && curr_group->attribute->post_process_view) || !curr_group->attribute->ask_for_metamail) {
			start_viewer(part, savepath);
			my_printf(cCRLF);
	} else {
		snprintf(buf, sizeof(buf), _(txt_view_attachment), savepath, content_types[part->type], part->subtype);
		if ((i = prompt_yn(buf, TRUE)) == 1)
			start_viewer(part, savepath);
		else if (i == -1) {	/* Skip rest of attachments */
			unlink(savepath);
			free(savepath);
			return FALSE;
		}
	}

	/*
	 * Save the attachment
	 */
	if (postproc && curr_group->attribute->post_process_view) {
		my_printf(_(txt_uu_success), savepath);
		my_printf(cCRLF);
	}
	if (!postproc) {
		snprintf(buf, sizeof(buf), _(txt_save_attachment), savepath, content_types[part->type], part->subtype);
		if ((i = prompt_yn(buf, FALSE)) != 1) {
			unlink(savepath);
			if (i == -1) {	/* Skip rest of attachments */
				free(savepath);
				return FALSE;
			}
		}
	}
	free(savepath);
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
		 /* check_save_mime_type() is done in decode_save_one() and the check for ptr->uue must be done unconditionally */
		if (ptr->type == TYPE_MULTIPART || (NULL != ptr->uue /* && !check_save_mime_type(ptr, curr_group->attribute->mime_types_to_save) */ ))
			continue;

		if (!(decode_save_one(ptr, art->raw, postproc)))
			break;
	}
}


/*
 * Attachment menu
 */
static void
show_attachment_page(
	void)
{
	char buf[BUFSIZ];
	const char *charset;
	int i, tmp_len, max_depth;
	t_part *part;

	signal_context = cAttachment;
	currmenu = &attmenu;
	mark_offset = 0;

	if (attmenu.curr < 0)
		attmenu.curr = 0;

	info_len = max_depth = 0;
	for (i = 0; i < attmenu.max; ++i) {
		part = get_part(i);
		snprintf(buf, sizeof(buf), _(txt_attachment_lines), part->line_count);
		tmp_len = strwidth(buf);
		charset = get_param(part->params, "charset");
		snprintf(buf, sizeof(buf), "  %s/%s, %s, %s%s", content_types[part->type], part->subtype, content_encodings[part->encoding], charset ? charset : "", charset ? ", " : "");
		tmp_len += strwidth(buf);
		if (tmp_len > info_len)
			info_len = tmp_len;

		tmp_len = part->depth;
		if (tmp_len > max_depth)
			max_depth = tmp_len;
	}
	tmp_len = cCOLS - 13 - MIN((cCOLS - 13) / 2 + 10, max_depth * 2 + 1 + strwidth(_(txt_attachment_no_name)));
	if (info_len > tmp_len)
		info_len = tmp_len;

	ClearScreen();
	set_first_screen_item();
	center_line(0, TRUE, _(txt_attachment_menu));

	for (i = attmenu.first; i < attmenu.first + NOTESLINES && i < attmenu.max; ++i)
		build_attachment_line(i);

	show_mini_help(ATTACHMENT_LEVEL);

	if (attmenu.max <= 0) {
		info_message(_(txt_no_attachments));
		return;
	}

	draw_attachment_arrow();
}


void
attachment_page(
	t_openartinfo *art)
{
	char key[MAXKEYLEN];
	t_function func;
	t_menu *oldmenu = NULL;
	t_part *part;

	if (currmenu)
		oldmenu = currmenu;
	num_of_tagged_parts = 0;
	attmenu.curr = 0;
	attmenu.max = build_part_list(art);
	clear_note_area();
	show_attachment_page();
	set_xclick_off();

	forever {
		switch ((func = handle_keypad(attachment_left, attachment_right, NULL, attachment_keys))) {
			case GLOBAL_QUIT:
				free_part_list(part_list);
				if (oldmenu)
					currmenu = oldmenu;
				return;

			case DIGIT_1:
			case DIGIT_2:
			case DIGIT_3:
			case DIGIT_4:
			case DIGIT_5:
			case DIGIT_6:
			case DIGIT_7:
			case DIGIT_8:
			case DIGIT_9:
				if (attmenu.max)
					prompt_item_num(func_to_key(func, attachment_keys), _(txt_attachment_select));
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_HELP:
				show_help_page(ATTACHMENT_LEVEL, _(txt_attachment_menu_com));
				show_attachment_page();
				break;

			case GLOBAL_FIRST_PAGE:
				top_of_list();
				break;

			case GLOBAL_LAST_PAGE:
				end_of_list();
				break;

			case GLOBAL_REDRAW_SCREEN:
				my_retouch();
				show_attachment_page();
				break;

			case GLOBAL_LINE_DOWN:
				move_down();
				break;

			case GLOBAL_LINE_UP:
				move_up();
				break;

			case GLOBAL_PAGE_DOWN:
				page_down();
				break;

			case GLOBAL_PAGE_UP:
				page_up();
				break;

			case GLOBAL_SCROLL_DOWN:
				scroll_down();
				break;

			case GLOBAL_SCROLL_UP:
				scroll_up();
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:
				toggle_mini_help(ATTACHMENT_LEVEL);
				show_attachment_page();
				break;

			case GLOBAL_TOGGLE_INFO_LAST_LINE:
				tinrc.info_in_last_line = bool_not(tinrc.info_in_last_line);
				show_attachment_page();
				break;

			case ATTACHMENT_SAVE:
				if (attmenu.max) {
					part = get_part(attmenu.curr);
					process_parts(part, art->raw, num_of_tagged_parts ? SAVE_TAGGED : SAVE);
					show_attachment_page();
				}
				break;

			case ATTACHMENT_SELECT:
				if (attmenu.max) {
					part = get_part(attmenu.curr);
					process_parts(part, art->raw, VIEW);
					show_attachment_page();
				}
				break;

			case ATTACHMENT_TAG:
				if (attmenu.max) {
					t_bool tagged;

					tagged = tag_part(attmenu.curr);
					show_attachment_page();
					if (attmenu.curr + 1 < attmenu.max)
						move_down();
					info_message(tagged ? _(txt_attachment_tagged) : _(txt_attachment_untagged));
				}
				break;

			case ATTACHMENT_UNTAG:
				if (attmenu.max && num_of_tagged_parts) {
					untag_all_parts();
					show_attachment_page();
				}
				break;

			case ATTACHMENT_TAG_PATTERN:
				if (attmenu.max) {
					tag_pattern();
					show_attachment_page();
					info_message(_(txt_attachments_tagged), num_of_tagged_parts);
				}
				break;

			case ATTACHMENT_TOGGLE_TAGGED:
				if (attmenu.max) {
					int i;

					for (i = attmenu.first; i < attmenu.max; ++i)
						tag_part(i);
					show_attachment_page();
					info_message(_(txt_attachments_tagged), num_of_tagged_parts);
				}
				break;

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					info_message(_(txt_no_prev_search));
				else if (attmenu.max) {
					int new_pos, old_pos = attmenu.curr;

					new_pos = generic_search((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT), attmenu.curr, attmenu.max - 1, ATTACHMENT_LEVEL);
					if (new_pos != old_pos)
						move_to_item(new_pos);
				}
				break;

#ifndef DONT_HAVE_PIPING
			case ATTACHMENT_PIPE:
			case GLOBAL_PIPE:
				if (attmenu.max) {
					part = get_part(attmenu.curr);
					process_parts(part, art->raw, func == GLOBAL_PIPE ? PIPE_RAW : PIPE);
					show_attachment_page();
				}
				break;
#endif /* !DONT_HAVE_PIPING */

			default:
				info_message(_(txt_bad_command), printascii(key, func_to_key(GLOBAL_HELP, attachment_keys)));
				break;
		}
	}
}


static t_function
attachment_left(
	void)
{
	return GLOBAL_QUIT;
}


static t_function
attachment_right(
	void)
{
	return ATTACHMENT_SELECT;
}


static void
draw_attachment_arrow(
	void)
{
	draw_arrow_mark(INDEX_TOP + attmenu.curr - attmenu.first);
	if (tinrc.info_in_last_line) {
		const char *name;
		t_part *part;

		part = get_part(attmenu.curr);
		name = get_filename(part->params);
		info_message("%s  %s", name ? name : _(txt_attachment_no_name), BlankIfNull(part->description));
	} else if (attmenu.curr == attmenu.max - 1)
		info_message(_(txt_end_of_attachments));
}


static void
build_attachment_line(
	int i)
{
	char *sptr;
	const char *name;
	const char *charset;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	char *tmpname;
	char *tmpbuf;
#endif /* MULTIBYTE_ABLE && !NOLOCALE */
	char buf[BUFSIZ];
	char buf2[BUFSIZ];
	char *tree = NULL;
	int len, namelen, tagged;
	int treelen = 0;
	t_part *part;

#ifdef USE_CURSES
	/*
	 * Allocate line buffer
	 * make it the same size like in !USE_CURSES case to simplify some code
	 */
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		sptr = my_malloc(cCOLS * MB_CUR_MAX + 2);
#	else
		sptr = my_malloc(cCOLS + 2);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#else
	sptr = screen[INDEX2SNUM(i)].col;
#endif /* USE_CURSES */

	part = get_part(i);
	namelen = strwidth(_(txt_attachment_no_name));
	tagged = get_tagged(i);

	if (!(name = get_filename(part->params))) {
		if (!(name = part->description))
			name = _(txt_attachment_no_name);
	}

	charset = get_param(part->params, "charset");
	snprintf(buf2, sizeof(buf2), _(txt_attachment_lines), part->line_count);
	snprintf(buf, sizeof(buf), "  %s/%s, %s, %s%s%s", content_types[part->type], part->subtype, content_encodings[part->encoding], charset ? charset : "", charset ? ", " : "", buf2);
	if (part->depth > 0) {
		treelen = cCOLS - 13 - info_len - namelen;
		tree = build_tree(part->depth, treelen, i);
	}
	snprintf(buf2, sizeof(buf2), "%s  %s", tagged ? tin_ltoa(tagged, 3) : "   ", BlankIfNull(tree));
	FreeIfNeeded(tree);
	len = strwidth(buf2);
	if (namelen + len + info_len + 8 <= cCOLS)
		namelen = cCOLS - 8 - info_len - len;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	tmpname = spart(name, namelen, TRUE);
	tmpbuf = spart(buf, info_len, TRUE);
	snprintf(sptr, cCOLS * MB_CUR_MAX, "  %s %s%*s%*s%s", tin_ltoa(i + 1, 4), buf2, namelen, BlankIfNull(tmpname), info_len, BlankIfNull(tmpbuf), cCRLF);
	FreeIfNeeded(tmpname);
	FreeIfNeeded(tmpbuf);
#else
	snprintf(sptr, cCOLS, "  %s %s%-*.*s%*.*s%s", tin_ltoa(i + 1, 4), buf2, namelen, namelen, name, info_len, info_len, buf, cCRLF);
#endif /* MULTIBYTE_ABLE && !NOLOCALE */

	WriteLine(INDEX2LNUM(i), sptr);

#ifdef USE_CURSES
	free(sptr);
#endif /* USE_CURSES */
}


/*
 * Build attachment tree. Code adopted
 * from thread.c:make_prefix().
 */
static char *
build_tree(
	int depth,
	int maxlen,
	int i)
{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	char *result;
	wchar_t *tree;
#else
	char *tree;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	int prefix_ptr, tmpdepth;
	int depth_level = 0;
	t_bool found = FALSE;
	t_partl *lptr, *lptr2;

	lptr2 = find_part(i);
	prefix_ptr = depth * 2 - 1;
	if (prefix_ptr > maxlen - 1 - !(maxlen % 2)) {
		int odd = ((maxlen % 2) ? 0 : 1);

		prefix_ptr -= maxlen - ++depth_level - 2 - odd;
		while (prefix_ptr > maxlen - 2 - odd) {
			if (depth_level < maxlen / 5)
				depth_level++;

			prefix_ptr -= maxlen - depth_level - 2 - odd;
			odd = (odd ? 0 : 1);
		}
	}
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	tree = my_malloc(sizeof(wchar_t) * prefix_ptr + 3 * sizeof(wchar_t));
	tree[prefix_ptr + 2] = (wchar_t) '\0';
#else
	tree = my_malloc(prefix_ptr + 3);
	tree[prefix_ptr + 2] = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	tree[prefix_ptr + 1] = TREE_ARROW;
	tree[prefix_ptr] = TREE_HORIZ;
	for (lptr = lptr2->next; lptr != NULL; lptr = lptr->next) {
		if (lptr->part->depth == depth) {
			found = TRUE;
			break;
		}
		if (lptr->part->depth < depth)
			break;
	}
	tree[--prefix_ptr] = found ? TREE_VERT_RIGHT : TREE_UP_RIGHT;
	found = FALSE;
	for (tmpdepth = depth - 1; prefix_ptr > 1; --tmpdepth) {
		for (lptr = lptr2->next; lptr != NULL; lptr = lptr->next) {
			if (lptr->part->depth == tmpdepth) {
				found = TRUE;
				break;
			}
			if (lptr->part->depth < tmpdepth)
				break;
		}
		tree[--prefix_ptr] = TREE_BLANK;
		tree[--prefix_ptr] = found ? TREE_VERT : TREE_BLANK;
		found = FALSE;
	}
	while (depth_level)
		tree[--depth_level] = TREE_ARROW_WRAP;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	result = wchar_t2char(tree);
	free(tree);
	return result;
#else
	return tree;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
}


/*
 * Find nth attachment in part_list.
 * Return pointer to that part.
 */
static t_partl *
find_part(
	int n)
{
	t_partl *lptr;

	lptr = part_list;
	if (attmenu.max >= 1)
		lptr = lptr->next;

	while (n-- > 0 && lptr->next)
		lptr = lptr->next;

	return lptr;
}


t_part *
get_part(
	int n)
{
	t_partl *lptr;

	lptr = find_part(n);
	return lptr->part;
}


static void
tag_pattern(
	void)
{
	char buf[BUFSIZ];
	char pat[128];
	char *prompt;
	const char *name;
	const char *charset;
	struct regex_cache cache = { NULL, NULL };
	t_part *part;
	t_partl *lptr;

#if 0
	if (num_of_tagged_parts)
		untag_all_parts();
#endif /* 0 */

	prompt = fmt_string(_(txt_select_pattern), tinrc.default_select_pattern);
	if (!(prompt_string_default(prompt, tinrc.default_select_pattern, _(txt_info_no_previous_expression), HIST_SELECT_PATTERN))) {
		free(prompt);
		return;
	}
	free(prompt);

	if (STRCMPEQ(tinrc.default_select_pattern, "*")) {	/* all */
		if (tinrc.wildcard)
			STRCPY(pat, ".*");
		else
			STRCPY(pat, tinrc.default_select_pattern);
	} else
		snprintf(pat, sizeof(pat), REGEX_FMT, tinrc.default_select_pattern);

	if (tinrc.wildcard && !(compile_regex(pat, &cache, PCRE_CASELESS)))
		return;

	lptr = find_part(0);

	for (; lptr != NULL; lptr = lptr->next) {
		part = lptr->part;
		if (!(name = get_filename(part->params))) {
			if (!(name = part->description))
				name = _(txt_attachment_no_name);
		}
		charset = get_param(part->params, "charset");

		snprintf(buf, sizeof(buf), "%s %s/%s %s, %s", name, content_types[part->type], part->subtype, content_encodings[part->encoding], charset ? charset : "");

		if (!match_regex(buf, pat, &cache, TRUE)) {
			continue;
		}
		if (!lptr->tagged)
			lptr->tagged = ++num_of_tagged_parts;
	}

	if (tinrc.wildcard) {
		FreeIfNeeded(cache.re);
		FreeIfNeeded(cache.extra);
	}
	return;
}

static int
get_tagged(
	int n)
{
	t_partl *lptr;

	lptr = find_part(n);
	return lptr->tagged;
}


static t_bool
tag_part(
	int n)
{
	t_partl *lptr;

	lptr = find_part(n);
	if (lptr->tagged) {
		untag_part(n);
		return FALSE;
	} else {
		lptr->tagged = ++num_of_tagged_parts;
		return TRUE;
	}
}


static void
untag_part(
	int n)
{
	int i;
	t_partl *curr_part, *lptr;

	lptr = find_part(0);
	curr_part = find_part(n);
	i = attmenu.max;

	while (i-- > 0 && lptr) {
		if (lptr->tagged > curr_part->tagged)
			--lptr->tagged;
		lptr = lptr->next;
	}

	curr_part->tagged = 0;
	--num_of_tagged_parts;
}


static void
untag_all_parts(
	void)
{
	t_partl *lptr = part_list;

	while (lptr) {
		if (lptr->tagged) {
			lptr->tagged = 0;
		}
		lptr = lptr->next;
	}
	num_of_tagged_parts = 0;
}


/*
 * Build a linked list which holds pointers to the parts we want deal with.
 */
static int
build_part_list(
	t_openartinfo *art)
{
	int i = 0;
	t_part *ptr, *uueptr;
	t_partl *lptr;

	part_list = my_malloc(sizeof(t_partl));
	lptr = part_list;
	lptr->part = art->hdr.ext;
	lptr->next = NULL;
	lptr->tagged = 0;
	for (ptr = art->hdr.ext; ptr != NULL; ptr = ptr->next) {
		if ((uueptr = ptr->uue) != NULL) {
			lptr->next = my_malloc(sizeof(t_partl));
			lptr->next->part = ptr;
			lptr->next->next = NULL;
			lptr->next->tagged = 0;
			lptr = lptr->next;
			++i;
			for (; uueptr != NULL; uueptr = uueptr->next) {
				lptr->next = my_malloc(sizeof(t_partl));
				lptr->next->part = uueptr;
				lptr->next->next = NULL;
				lptr->next->tagged = 0;
				lptr = lptr->next;
				++i;
			}
		}

		if (ptr->uue)
			continue;

		lptr->next = my_malloc(sizeof(t_partl));
		lptr->next->part = ptr;
		lptr->next->next = NULL;
		lptr->next->tagged = 0;
		lptr = lptr->next;
		++i;
	}
	return i;
}


static void
free_part_list(
	t_partl *list)
{
	while (list->next != NULL) {
		free_part_list(list->next);
		list->next = NULL;
	}
	free(list);
}


static void
process_parts(
	t_part *part,
	FILE *rawfp,
	enum action what)
{
	FILE *fp;
	char *savepath = NULL, *tmppath;
	int i, saved_parts = 0;
	t_partl *lptr;

	switch (what) {
		case SAVE_TAGGED:
			for (i = 1; i <= num_of_tagged_parts; i++) {
				lptr = part_list;

				while (lptr) {
					if (lptr->tagged == i) {
						if ((savepath = generate_savepath(lptr->part)) == NULL)
							return;

						if ((fp = open_save_filename(savepath, FALSE)) == NULL) {
							error_message(2, _(txt_cannot_open_for_saving), savepath);
							free(savepath);
							return;
						}
						process_part(lptr->part, rawfp, fp, NULL, SAVE);
						free(savepath);
						++saved_parts;
					}
					lptr = lptr->next;
				}
			}
			break;

		default:
			if ((tmppath = generate_savepath(part)) == NULL)
				return;

			if (what == SAVE)
				savepath = tmppath;
			else {
				savepath = get_tmpfilename(tmppath);
				free(tmppath);
			}
			if ((fp = open_save_filename(savepath, FALSE)) == NULL) {
				error_message(2, _(txt_cannot_open_for_saving), savepath);
				free(savepath);
				return;
			}
			process_part(part, rawfp, fp, savepath, what);
			break;
	}
	switch (what) {
		case SAVE_TAGGED:
			wait_message(2, _(txt_attachments_saved), saved_parts, num_of_tagged_parts);
			break;

		case SAVE:
			wait_message(2, _(txt_attachment_saved), savepath);
			free(savepath);
			break;

		default:
			unlink(savepath);
			free(savepath);
			break;
	}
	cursoroff();
}


/*
 * VIEW/PIPE/SAVE the given part.
 *
 * PIPE_RAW uses the raw part, otherwise the part is decoded first.
 */
static void
process_part(
	t_part *part,
	FILE *rawfp,
	FILE *outfile,
	const char *savepath,
	enum action what)
{
	char buf[2048], buf2[2048];
	int count;
	int i;

	if (what != PIPE_RAW && part->encoding == ENCODING_BASE64)
		mmdecode(NULL, 'b', 0, NULL);				/* flush */

	fseek(rawfp, part->offset, SEEK_SET);

	for (i = 0; i < part->line_count; i++) {
		if ((fgets(buf, sizeof(buf), rawfp)) == NULL)
			break;

		/* This should catch cases where people illegally append text etc */
		if (buf[0] == '\0')
			break;

		if (what != PIPE_RAW) {
			switch (part->encoding) {
				case ENCODING_QP:
				case ENCODING_BASE64:
					if ((count = mmdecode(buf, part->encoding == ENCODING_QP ? 'q' : 'b', '\0', buf2)) > 0)
						fwrite(buf2, count, 1, outfile);
					break;

				case ENCODING_UUE:
					/* TODO: if postproc, don't decode these since the traditional uudecoder will get them */
					/*
					 * x-uuencode attachments have all the header info etc which we must ignore
					 */
					if (strncmp(buf, "begin ", 6) != 0 && strncmp(buf, "end\n", 4) != 0 && buf[0] != '\n')
						uudecode_line(buf, outfile);
					break;

				default:
					fputs(buf, outfile);
			}
		} else
			fputs(buf, outfile);
	}

	fclose(outfile);

	switch (what) {
		case VIEW:
			start_viewer(part, savepath);
			break;

#ifndef DONT_HAVE_PIPING
		case PIPE:
		case PIPE_RAW:
			pipe_part(savepath);
			break;
#endif /* !DONT_HAVE_PIPING */

		default:
			break;
	}
}


#ifndef DONT_HAVE_PIPING
static void
pipe_part(
	const char *savepath)
{
	FILE *fp, *pipe_fp = (FILE *) 0;
	char *prompt;

	prompt = fmt_string(_(txt_pipe_to_command), cCOLS - (strlen(_(txt_pipe_to_command)) + 30), tinrc.default_pipe_command);
	if (!(prompt_string_default(prompt, tinrc.default_pipe_command, _(txt_no_command), HIST_PIPE_COMMAND))) {
		free(prompt);
		return;
	}
	free(prompt);
	if ((fp = fopen(savepath, "r")) == NULL)
		/* TODO: error message? */
		return;
	EndWin();
	Raw(FALSE);
	fflush(stdout);
	set_signal_catcher(FALSE);
	if ((pipe_fp = popen(tinrc.default_pipe_command, "w")) == NULL) {
		perror_message(_(txt_command_failed), tinrc.default_pipe_command);
		set_signal_catcher(TRUE);
		Raw(TRUE);
		InitWin();
		fclose(fp);
		return;
	}
	copy_fp(fp, pipe_fp);
	if (errno == EPIPE)
		perror_message(_(txt_command_failed), tinrc.default_pipe_command);
	fflush(pipe_fp);
	(void) pclose(pipe_fp);
	set_signal_catcher(TRUE);
	fclose(fp);
#	ifdef USE_CURSES
	Raw(TRUE);
	InitWin();
#	endif /* USE_CURSES */
	prompt_continue();
#	ifndef USE_CURSES
	Raw(TRUE);
	InitWin();
#	endif /* !USE_CURSES */
}
#endif /* !DONT_HAVE_PIPING */
