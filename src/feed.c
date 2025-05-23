/*
 *  Project   : tin - a Usenet reader
 *  Module    : feed.c
 *  Author    : I. Lea
 *  Created   : 1991-08-31
 *  Updated   : 2025-04-04
 *  Notes     : provides same interface to mail,pipe,print,save & repost commands
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>
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


static t_bool confirm;					/* only used for FEED_MAIL */
static t_bool is_mailbox = FALSE;
static t_bool redraw_screen = FALSE;
static t_bool supersede = FALSE;		/* for reposting only */
static t_function pproc_func;			/* Post-processing type when saving */
#ifndef DONT_HAVE_PIPING
	static FILE *pipe_fp = (FILE *) 0;
	static t_bool got_epipe = FALSE;
#endif /* !DONT_HAVE_PIPING */


struct t_counters {
	int success;		/* # arts fed okay */
	int total;			/* # arts fed */
	int max;			/* initial guesstimate of total */
};

/*
 * Local prototypes
 */
static char *get_save_filename(const struct t_group *group, int function, char *filename, int filelen);
static t_bool expand_feed_filename(char *outpath, size_t outpath_len, const char *path);
static t_bool feed_article(int art, int function, struct t_counters *counter, t_bool use_current, const char *data, const struct t_group *group);
static t_function get_feed_key(int function, int level, struct t_art_stat *thread);
static t_function get_post_proc_type(void);
static void print_save_summary(t_function type, int fed);
#ifndef DISABLE_PRINTING
	static t_bool print_file(const char *command, int respnum, t_openartinfo *artinfo);
#endif /* !DISABLE_PRINTING */

#ifndef DONT_HAVE_PIPING
#	define handle_EPIPE()	if (got_epipe) goto got_epipe_while_piping
#else
#	define handle_EPIPE()	do {} while (0)	/* nothing */
#endif /* !DONT_HAVE_PIPING */

/*
 * 'filename' holds 'filelen' amount of storage in which to place the
 * filename to save-to. The filename is also returned after basic syntax
 * checking. We default to the global save filename or group specific
 * filename if it exists
 */
static char *
get_save_filename(
	const struct t_group *group,
	int function,
	char *filename,
	int filelen)
{
	char default_savefile[PATH_LEN];

	filename[0] = '\0';

	/*
	 * Group attribute savefile overrides tinrc default savefile
	 */
	my_strncpy(default_savefile, (group->attribute->savefile && *group->attribute->savefile ? *group->attribute->savefile : BlankIfNull(tinrc.default_save_file)), sizeof(default_savefile) - 1);

	/*
	 * We don't ask when auto'S'aving
	 */
	if (!(function == FEED_AUTOSAVE)) {
		if (!prompt_default_string(_(txt_save_filename), filename, filelen, default_savefile, HIST_SAVE_FILE)) {
			clear_message();
			return NULL;
		}
		str_trim(filename);
	}

	/*
	 * Update tinrc.default_save_file if changed
	 */
	if (*filename) {
		FreeIfNeeded(tinrc.default_save_file);
		tinrc.default_save_file = my_strdup(filename);
	} else {
		/*
		 * None chosen (or AUTOSAVING), use tinrc default
		 */
		if (*default_savefile)
			my_strncpy(filename, default_savefile, (size_t) (filelen - 1));
		else {									/* No default either */
			info_message(_(txt_no_filename));
			return NULL;
		}
	}

	/*
	 * Punt invalid expansions
	 */
	if ((filename[0] == '~' || filename[0] == '+') && filename[1] == '\0') {
		info_message(_(txt_no_filename));
		return NULL;
	}
	return filename;
}


/*
 * Generate a path/filename to save to, using 'path' as input.
 * The pathname is stored in 'outpath', which should be PATH_LEN in size
 * Expand metacharacters and use defaults as needed.
 * Return TRUE if the path is a mailbox, or FALSE otherwise.
 */
static t_bool
expand_feed_filename(
	char *outpath,
	size_t outpath_len,
	const char *path)
{
	int ret = strfpath(path, outpath, PATH_LEN, curr_group, TRUE);

	/*
	 * If no path exists or the above failed in some way, use sensible defaults
	 * Put the generic path into 'outpath'
	 */
	if ((ret == 0) || !(strrchr(outpath, '/'))) {
		char buf[PATH_LEN];

		if (!strfpath(cmdline.savedir ? cmdline.savedir : *curr_group->attribute->savedir, buf, sizeof(buf), curr_group, FALSE))
			joinpath(buf, sizeof(buf), homedir, DEFAULT_SAVEDIR);
		joinpath(outpath, outpath_len, buf, path);
		return FALSE;
	}
	return (ret == 1);
}


/*
 * Find out what post-processing to perform.
 * This is not used when saving to mailboxes (we don't postprocess mailboxes)
 * Also not used when using the auto-save feature because a default value is
 * taken from the group attributes
 * Returns POSTPROCESS_{NO,SHAR,YES} or GLOBAL_ABORT if aborting the save process
 */
static t_function
get_post_proc_type(
	void)
{
	char keyno[MAXKEYLEN], keyyes[MAXKEYLEN], keyquit[MAXKEYLEN];
	char keyshar[MAXKEYLEN];
	t_function default_func, func;

	switch (curr_group->attribute->post_process_type) {
		case POST_PROC_YES:
			default_func = POSTPROCESS_YES;
			break;

		case POST_PROC_SHAR:
			default_func = POSTPROCESS_SHAR;
			break;

		case POST_PROC_NO:
		default:
			default_func = POSTPROCESS_NO;
			break;
	}

	func = prompt_slk_response(default_func, feed_post_process_keys, _(txt_choose_post_process_type),
				PrintFuncKey(keyno, POSTPROCESS_NO, feed_post_process_keys),
				PrintFuncKey(keyyes, POSTPROCESS_YES, feed_post_process_keys),
				PrintFuncKey(keyshar, POSTPROCESS_SHAR, feed_post_process_keys),
				PrintFuncKey(keyquit, GLOBAL_QUIT, feed_post_process_keys));

	if (func == GLOBAL_QUIT || func == GLOBAL_ABORT) {			/* exit */
		clear_message();
		return GLOBAL_ABORT;
	}
	return func;
}


/*
 * Return the key mapping for what we are intending to process or
 * GLOBAL_ABORT if save process is being aborted
 * Key can be (current) article, (current) thread, tagged articles,
 * hot articles, or articles matching a pattern
 * This is automatic in the various auto-save cases, in other
 * cases this is prompted for based on a chosen default
 */
static t_function
get_feed_key(
	int function,
	int level,
	struct t_art_stat *thread)
{
	constext *prompt;
	t_function default_func, func;

	switch (function) {
		case FEED_MAIL:
			prompt = txt_mail;
			break;

		case FEED_MARK_READ:
		case FEED_MARK_UNREAD:
			prompt = txt_mark;
			break;

#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
			prompt = txt_pipe;
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			prompt = txt_print;
			break;
#endif /* !DISABLE_PRINTING */

		/* FEED_AUTOSAVE doesn't prompt */
		case FEED_SAVE:
			prompt = txt_save;
			break;

		case FEED_REPOST:
			if (!can_post) {				/* Get this over with before asking any Q's */
				info_message(_(txt_cannot_post));
				return NOT_ASSIGNED;
			}
			prompt = txt_repost;
			break;

		default:
			prompt = "";
			break;
	}

	/*
	 * Try and work out what default the user wants
	 * thread->total = # arts in thread
	 */
	default_func = (range_active ? FEED_RANGE :
					num_of_tagged_arts ? FEED_TAGGED :
					(arts_selected() ? FEED_HOT :
					((level == GROUP_LEVEL && thread->total > 1) ? FEED_THREAD :
					(thread->selected_total ? FEED_HOT :
					FEED_ARTICLE))));

	/*
	 * Don't bother querying when:
	 *  auto'S'aving and there are tagged or selected(hot) articles
	 */
	if ((function == FEED_AUTOSAVE && (range_active || num_of_tagged_arts || arts_selected())))
		func = default_func;
	else {
		char buf[LEN];
		char keyart[MAXKEYLEN], keythread[MAXKEYLEN], keyrange[MAXKEYLEN], keyhot[MAXKEYLEN];
		char keypat[MAXKEYLEN], keytag[MAXKEYLEN], keyquit[MAXKEYLEN];

		snprintf(buf, sizeof(buf), _(txt_art_thread_regex_tag),
			PrintFuncKey(keyart, FEED_ARTICLE, feed_type_keys),
			PrintFuncKey(keythread, FEED_THREAD, feed_type_keys),
			PrintFuncKey(keyrange, FEED_RANGE, feed_type_keys),
			PrintFuncKey(keyhot, FEED_HOT, feed_type_keys),
			PrintFuncKey(keypat, FEED_PATTERN, feed_type_keys),
			PrintFuncKey(keytag, FEED_TAGGED, feed_type_keys),
			PrintFuncKey(keyquit, GLOBAL_QUIT, feed_type_keys));

		func = prompt_slk_response(default_func, feed_type_keys, "%s %s", _(prompt), buf);
	}

	switch (func) {
		case FEED_PATTERN:
			{
				char *tmp = fmt_string(_(txt_feed_pattern), tinrc.default_pattern);

				if (!(prompt_string_ptr_default(tmp, &tinrc.default_pattern, _(txt_no_match), HIST_REGEX_PATTERN))) {
					free(tmp);
					return GLOBAL_ABORT;
				}
				free(tmp);
			}
			break;

		case FEED_RANGE:
			if (!range_active) {
				if (set_range(level, 1, currmenu->max, currmenu->curr + 1))
					range_active = TRUE;
				else
					return GLOBAL_ABORT;
			}
			break;

		case GLOBAL_QUIT:
		case GLOBAL_ABORT:
			clear_message();
			return GLOBAL_ABORT;
			/* NOTREACHED */
			break;

		default:
			break;
	}

	return func;
}


/*
 * Print a message like:
 * -- [Article|Thread|Tagged Articles] saved to [mailbox] [filenames] --
 * 'fed' is the number of articles we tried to save
 *
 * FIXME: translatable/plural-forms and fixed length buffer
 */
static void
print_save_summary(
	t_function type,
	int fed)
{
	char buf[LEN];
	char what[LEN];

	if (fed != num_save)
		wait_message(2, P_(txt_warn_not_all_arts_saved_sp[0], txt_warn_not_all_arts_saved_sp[1], num_save), fed, num_save);

	switch (type) {
		case FEED_HOT:
			snprintf(what, sizeof(what), "%s", P_(txt_hot_article_sp[0], txt_hot_article_sp[1], fed));
			break;

		case FEED_TAGGED:
			snprintf(what, sizeof(what), "%s", P_(txt_tagged_article_sp[0], txt_tagged_article_sp[1], fed));
			break;

		case FEED_THREAD:
			STRCPY(what, _(txt_thread_upper));
			break;

		case FEED_ARTICLE:
		case FEED_PATTERN:
		default:
			snprintf(what, sizeof(what), "%s", P_(txt_article_sp[0], txt_article_sp[1], fed));
			break;
	}

	/*
	 * We report the range of saved-to files for regular saves of > 1 articles
	 */
	/* FIXME: translatable/plural-forms */
	if (num_save == 1 || save[0].mailbox)
		snprintf(buf, sizeof(buf), _(txt_saved_to),
			what, (save[0].mailbox ? _(txt_mailbox) : ""), save[0].path);
	else
		snprintf(buf, sizeof(buf), _(txt_saved_to_range),
			what, save[0].file, save[num_save - 1].file);

	wait_message((tinrc.beginner_level) ? 4 : 2, buf);
}


/*
 * This is the handler that processes a single article for all the various
 * FEED_ functions.
 * Assumes no article is open when we enter - opens and closes the art being
 * processed. As a performance hack this is not done if 'use_current' is set.
 * Returns TRUE or FALSE
 * TODO: option to mail/pipe/print/repost raw vs. cooked?
 *       (all currently raw only) or should we feed according to what
 *       is currently on screen?
 */
static t_bool
feed_article(
	int art,				/* index in arts[] */
	int function,
	struct t_counters *counter,	/* Accounting */
	t_bool use_current,		/* Use already open pager article */
	const char *data,		/* Extra data if needed, print command or save filename */
	const struct t_group *group)
{
	char *progress_mesg = NULL;
	t_bool ok = TRUE;		/* Assume success */
	t_openartinfo openart;
	t_openartinfo *openartptr = &openart;

	counter->total++;

	/*
	 * Update the on-screen progress before art_open(), which is the bottleneck
	 * timewise
	 */
	switch (function) {
#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
			progress_mesg = fmt_string("%s (%d/%d)", _(txt_piping), counter->total, counter->max);
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			progress_mesg = fmt_string("%s (%d/%d)", _(txt_printing), counter->total, counter->max);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:
		case FEED_AUTOSAVE:
			progress_mesg = fmt_string("%s (%d/%d)", _(txt_saving), counter->total, counter->max);
			break;
	}

	if (progress_mesg != NULL) {
		if (!use_current)
			show_progress(progress_mesg, counter->total, counter->max);
		FreeAndNull(progress_mesg);
	}

	if (use_current)
		openartptr = &pgart;			/* Use art already open in pager */
	else {
		if (art_open(FALSE, &arts[art], group, openartptr, FALSE, NULL) < 0)
			/* User abort or an error */
			return FALSE;
	}

	switch (function) {
		case FEED_MAIL:
			switch (mail_to_someone(tinrc.default_mail_address, confirm, openartptr, group)) {
				case POSTED_REDRAW:
					redraw_screen = TRUE;
					/* FALLTHROUGH */
				case POSTED_NONE:
					ok = FALSE;
					break;

				case POSTED_OK:
				default:
					break;
			}
			confirm = bool_not(ok);		/* Only confirm the next one after a failure */
			break;

		case FEED_MARK_READ:
			if (arts[art].status == ART_UNREAD || arts[art].status == ART_WILL_RETURN)
				art_mark(curr_group, &arts[art], ART_READ);
			else
				ok = FALSE;
			break;

		case FEED_MARK_UNREAD:
			if (arts[art].status == ART_READ)
				art_mark(curr_group, &arts[art], ART_WILL_RETURN);
			else
				ok = FALSE;
			break;

#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
			rewind(openartptr->raw);
			if (copy_fp(openartptr->raw, pipe_fp) == EPIPE) {	/* broken pipe in copy_fp() */
				got_epipe = TRUE;
				ok = FALSE;
			}
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			ok = print_file(data /*print_command*/, art, openartptr);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:
		case FEED_AUTOSAVE:
			ok = save_and_process_art(openartptr, is_mailbox, data /*filename*/, counter->max, (pproc_func != POSTPROCESS_NO));
			if (ok && curr_group->attribute->mark_saved_read)
				art_mark(curr_group, &arts[art], ART_READ);
			break;

		case FEED_REPOST:
			if (repost_article(tinrc.default_repost_group, art, supersede, openartptr) == POSTED_NONE)
				ok = FALSE;
			else			/* POSTED_REDRAW, POSTED_OK */
				redraw_screen = TRUE;
			break;

		default:
			break;
	}
	if (ok)
		counter->success++;

	if (!use_current)
		art_close(openartptr);

	return ok;
}


/*
 * Single entry point for 'feed'ing article(s) to a backend
 * Function:
 *	FEED_PIPE, FEED_MAIL, FEED_PRINT, FEED_REPOST
 *	FEED_SAVE, FEED_AUTOSAVE, FEED_MARK_READ, FEED_MARK_UNREAD
 * Level:
 *	GROUP_LEVEL, THREAD_LEVEL, PAGE_LEVEL
 * Type:
 *  default feed_type; if NOT_ASSIGNED, query what to do
 * Respnum:
 *	Index in arts[] of starting article
 *
 * The following 'groups' of article can be processed:
 *	Single (current) article
 *	Current thread
 *	Range of articles
 *	Tagged articles
 *	Hot articles
 *	Articles matching a pattern
 *
 * The selection of Function depends on the key used to get here.
 * The selection of which article 'group' to process is managed
 * inside here, or by defaults.
 *
 * Returns:
 *   1	if there are no more unread arts in this group (FEED_MARK_READ)
 *   0	on success
 *  -1	on failure/abort
 */
int
feed_articles(
	int function,
	int level,
	t_function type,
	struct t_group *group,
	int respnum)
{
	char *prompt;
	char outpath[PATH_LEN]; /* FIXME: don't used fixed length */
	int art;
	int i;
	int saved_curr_line = -1;
	int thread_base;
	struct t_art_stat sbuf;
	struct t_counters counter = { 0, 0, 0 };
	t_bool feed_mark_function = function == FEED_MARK_READ || function == FEED_MARK_UNREAD;
	t_bool mark_saved = FALSE;
	t_bool no_next_unread = FALSE;
	t_bool post_processed_ok = FALSE;
	t_bool use_current = FALSE;
	t_function feed_type;

#ifdef DONT_HAVE_PIPING
	if (function == FEED_PIPE) {
		error_message(2, _(txt_piping_not_enabled));
		clear_message();
		return -1;
	}
#endif /* DONT_HAVE_PIPING */

	if (function == FEED_AUTOSAVE) {
		if (!range_active && num_of_tagged_arts == 0 && !arts_selected()) {
			info_message(_(txt_no_marked_arts));
			return -1;
		}
	}

	set_xclick_off();		/* TODO: there is no corresponding set_xclick_on()? */
	if ((thread_base = which_thread(respnum)) >= 0)
		stat_thread(thread_base, &sbuf);
	else /* TODO: error message? */
		return -1;

	switch (type) {
		case FEED_ARTICLE:
		case FEED_THREAD:
		case FEED_RANGE:
			feed_type = type;
			break;

		default:
			if ((feed_type = get_feed_key(function, level, &sbuf)) == GLOBAL_ABORT)
				return -1;
			break;
	}

	/*
	 * Get whatever information is needed to proceed
	 */
	switch (function) {
		/* Setup mail - get address to mail to */
		case FEED_MAIL: /* FIXME: translatable/plural-forms */
			prompt = fmt_string(_(txt_mail_art_to), cCOLS - (strwidth(_(txt_mail_art_to)) > cCOLS - 30 ? cCOLS - 30 : strwidth(_(txt_mail_art_to)) + 30), tinrc.default_mail_address);
			if (!(prompt_string_ptr_default(prompt, &tinrc.default_mail_address, _(txt_no_mail_address), HIST_MAIL_ADDRESS))) {
				free(prompt);
				return -1;
			}
			free(prompt);
			break;

#ifndef DONT_HAVE_PIPING
		/* Setup pipe - get pipe-to command and open the pipe */
		case FEED_PIPE:
			prompt = fmt_string(_(txt_pipe_to_command), cCOLS - (strwidth(_(txt_pipe_to_command)) > cCOLS - 30 ? cCOLS - 30 : strwidth(_(txt_pipe_to_command)) + 30), tinrc.default_pipe_command);
			if (!(prompt_string_ptr_default(prompt, &tinrc.default_pipe_command, _(txt_no_command), HIST_PIPE_COMMAND))) {
				free(prompt);
				return -1;
			}
			free(prompt);

			got_epipe = FALSE;
			EndWin(); /* Turn off curses/windowing */
			Raw(FALSE);
			fflush(stdout);
			set_signal_catcher(FALSE);
			if ((pipe_fp = popen(tinrc.default_pipe_command, "w")) == NULL) {
				perror_message(_(txt_command_failed), tinrc.default_pipe_command);
				set_signal_catcher(TRUE);
				Raw(TRUE);
				InitWin();
				return -1;
			}
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		/* Setup printing - get print command line */
		/* FIXME: don't use fixed length buffer */
		case FEED_PRINT:
			{
				int l = (int) (sizeof(outpath) - strlen(tinrc.printer) - strlen(REDIRECT_OUTPUT) - 2);

				if (l > 0)
					snprintf(outpath, sizeof(outpath), "%.*s %s", l, tinrc.printer, REDIRECT_OUTPUT);
				else
					*outpath = '\0';
			}
			break;
#endif /* !DISABLE_PRINTING */

		/*
		 * Setup saving, some of these are generated automatically
		 *	Determine path/file to save to
		 *	Determine post-processing type
		 *	Determine if post processed file deletion required
		 */
		case FEED_SAVE:
		case FEED_AUTOSAVE:
			{
				char savefile[PATH_LEN];

				/* This will force automatic selection unless changed by user */
				savefile[0] = '\0';

				if (get_save_filename(group, function, savefile, sizeof(savefile)) == NULL)
					return -1;

				switch (curr_group->attribute->post_process_type) {
					case POST_PROC_YES:
						pproc_func = POSTPROCESS_YES;
						break;

					case POST_PROC_SHAR:
						pproc_func = POSTPROCESS_SHAR;
						break;

					case POST_PROC_NO:
					default:
						pproc_func = POSTPROCESS_NO;
						break;
				}

				/* We don't postprocess mailboxen */
				if ((is_mailbox = expand_feed_filename(outpath, sizeof(outpath), savefile)) == TRUE)
					pproc_func = POSTPROCESS_NO;
				else {
					if (function != FEED_AUTOSAVE && (pproc_func = get_post_proc_type()) == GLOBAL_ABORT)
						return -1;
				}
				if (create_path(outpath) != 0)
					return -1;
			}
			break;

		/* repost (or supersede) article */
		case FEED_REPOST:
			{
				char *tmp;
				char *smsg;
#ifndef FORGERY
				char from_name[PATH_LEN];

				get_from_name(from_name, (struct t_group *) 0);

				if (strstr(from_name, arts[respnum].mailbox.from)) {
#endif /* !FORGERY */
					char buf[LEN];
					char keyrepost[MAXKEYLEN], keysupersede[MAXKEYLEN];
					char keyquit[MAXKEYLEN];
					t_function func;

					/* repost or supersede? */
					snprintf(buf, sizeof(buf), _(txt_supersede_article),
							PrintFuncKey(keyrepost, FEED_KEY_REPOST, feed_supersede_article_keys),
							PrintFuncKey(keysupersede, FEED_SUPERSEDE, feed_supersede_article_keys),
							PrintFuncKey(keyquit, GLOBAL_QUIT, feed_supersede_article_keys));
					func = prompt_slk_response(FEED_SUPERSEDE,
								feed_supersede_article_keys, "%s",
								sized_message(&smsg, buf, arts[respnum].subject));
					free(smsg);

					switch (func) {
						case FEED_SUPERSEDE: /* TODO: plural-forms? strip_double_ngs()? */
							tmp = fmt_string(_(txt_supersede_group), BlankIfNull(tinrc.default_repost_group));
							supersede = TRUE;
							break;

						case FEED_KEY_REPOST: /* TODO: plural-forms? strip_double_ngs()? */
							tmp = fmt_string(_(txt_repost_group), BlankIfNull(tinrc.default_repost_group));
							supersede = FALSE;
							break;

						default:
							clear_message();
							return -1;
					}
#ifndef FORGERY
				} else { /* TODO: plural-forms? strip_double_ngs()? */
					tmp = fmt_string(_(txt_repost_group), BlankIfNull(tinrc.default_repost_group));
					supersede = FALSE;
				}
#endif /* !FORGERY */
				if (!(smsg = prompt_string_default(tmp, tinrc.default_repost_group, _(txt_no_group), HIST_REPOST_GROUP))) {
					FreeIfNeeded(smsg);
					free(tmp);
					return -1;
				}
				tinrc.default_repost_group = smsg;
				free(tmp);
			}
			break;

		default:
			break;
	} /* switch (function) */

	confirm = TRUE;				/* Always confirm the first time */
	clear_message();

	/*
	 * Performance hack - If we feed a single art from the pager then we can
	 * reuse the currently open article
	 * Also no need to fetch articles just to mark them (un)read
	 */
	if (feed_mark_function || (level == PAGE_LEVEL && (feed_type == FEED_ARTICLE || feed_type == FEED_THREAD))) {
		saved_curr_line = curr_line;		/* Save where we were in pager */
		use_current = TRUE;
	}

	/*
	 * This is the main loop
	 * The general idea is to feed_article() for every article to be processed
	 */
	switch (feed_type) {
		case FEED_ARTICLE:		/* article */
			counter.max = 1;
			if (!feed_article(respnum, function, &counter, use_current, outpath, group))
				handle_EPIPE();
			break;

		case FEED_THREAD:		/* thread */
			/* Get accurate count first */
			for_each_art_in_thread(art, which_thread(respnum)) {
				if (feed_mark_function || !(curr_group->attribute->process_only_unread && arts[art].status == ART_READ))
					counter.max++;
			}

			for_each_art_in_thread(art, which_thread(respnum)) {
				if (feed_mark_function || !(curr_group->attribute->process_only_unread && arts[art].status == ART_READ)) {
					/* Keep going - don't abort on errors */
					if (!feed_article(art, function, &counter, use_current, outpath, group))
						handle_EPIPE();
				}
			}
			break;

		case FEED_RANGE:
			/* Get accurate count first */
			for_each_art(art) {
				if (arts[art].inrange)
					counter.max++;
			}

			for_each_art(art) {
				if (arts[art].inrange) {
					arts[art].inrange = FALSE;
					if (!feed_article(art, function, &counter, use_current, outpath, group))
						handle_EPIPE();
				}
			}
			range_active = FALSE;
			redraw_screen = TRUE;
			break;

		case FEED_TAGGED:		/* tagged articles */
			counter.max = num_of_tagged_arts;
			for (i = 1; i <= num_of_tagged_arts; i++) {
				for_each_art(art) {
					/* process_only_unread does NOT apply on tagged arts */
					if (arts[art].tagged == i) {
						/* Keep going - don't abort on errors */
						if (!feed_article(art, function, &counter, use_current, outpath, group))
							handle_EPIPE();
					}
				}
			}
			untag_all_articles();	/* TODO: this will untag even on partial failure */
			redraw_screen = TRUE;
			break;

		case FEED_HOT:		/* hot (auto-selected) articles */
		case FEED_PATTERN:	/* pattern matched articles */
			{
				struct regex_cache cache = REGEX_CACHE_INITIALIZER;

				if ((feed_type == FEED_PATTERN) && tinrc.wildcard && !(compile_regex(tinrc.default_pattern, &cache, REGEX_CASELESS)))
					break;

				for_each_art(art) {
					if (feed_type == FEED_PATTERN) {
						if (!match_regex(arts[art].subject, tinrc.default_pattern, &cache, TRUE))
							continue;
					} else if (!arts[art].selected)
						continue;

					if (!feed_mark_function && (curr_group->attribute->process_only_unread && arts[art].status == ART_READ))
						continue;

					arts[art].matched = TRUE;
					counter.max++;
				}

				if (tinrc.wildcard)
					regex_cache_destroy(&cache);
			}

			/* I think we nest like this to preserve any 'ordering' of the arts */
			for (i = 0; i < grpmenu.max; i++) {
				for_each_art_in_thread(art, i) {
					if (!arts[art].matched)
						continue;
					arts[art].matched = FALSE;

					/* Keep going - don't abort on errors */
					if (feed_article(art, function, &counter, use_current, outpath, group)) {
						if (feed_type == FEED_HOT)
							arts[art].selected = FALSE;
					} else
						handle_EPIPE();
				}
			}
			redraw_screen = TRUE;
			break;

		default:			/* Should never get here */
			break;
	} /* switch (feed_type) */

	/*
	 * Invoke post-processing if needed
	 * Work out what (if anything) needs to be redrawn
	 */
	if (tinrc.interactive_mailer == INTERACTIVE_NONE)
		redraw_screen |= mail_check(mailbox);	/* in case of sending to oneself */

	switch (function) {
		case FEED_MARK_READ:
		case FEED_MARK_UNREAD:
			redraw_screen = FALSE;
			if (level == GROUP_LEVEL) {
				no_next_unread = group_mark_postprocess(function, feed_type, respnum);
				break;
			}
			if (level == THREAD_LEVEL)
				no_next_unread = thread_mark_postprocess(function, feed_type, respnum);
			break;

#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
got_epipe_while_piping:
			if (got_epipe)
				perror_message(_(txt_command_failed), tinrc.default_pipe_command);
			got_epipe = FALSE;
			fflush(pipe_fp);
			(void) pclose(pipe_fp);
			set_signal_catcher(TRUE);
			my_printf(cCRLF);
#	ifdef USE_CURSES
			Raw(TRUE);
			InitWin();
#	endif /* USE_CURSES */
			prompt_continue();
#	ifndef USE_CURSES
			Raw(TRUE);
			InitWin();
#	endif /* !USE_CURSES */
			redraw_screen = TRUE;
			break;
#endif /* !DONT_HAVE_PIPING */

		case FEED_SAVE:
		case FEED_AUTOSAVE:
			if (num_save == 0) {
				wait_message(1, _(txt_saved_nothing));
				break;
			}

			if (redraw_screen) {
				currmenu->redraw();
				redraw_screen = FALSE;
			}

			print_save_summary(feed_type, counter.total);
			if (pproc_func != POSTPROCESS_NO) {
				t_bool delete_post_proc = FALSE;

				if (curr_group->attribute->delete_tmp_files)
					delete_post_proc = TRUE;
				else {
					if (function != FEED_AUTOSAVE) {
						if (prompt_yn(_(txt_delete_processed_files), TRUE) == 1)
							delete_post_proc = TRUE;
					}
				}
				post_processed_ok = post_process_files(pproc_func, delete_post_proc);
			}
			free_save_array();		/* NB: This is where num_save etc.. gets purged */

			if (level != PAGE_LEVEL)
				mark_saved = curr_group->attribute->mark_saved_read;
			break;

		default:
			break;
	}

	if (mark_saved || post_processed_ok)
		redraw_screen = TRUE;

	if (level == PAGE_LEVEL && !feed_mark_function) {
		if (tinrc.force_screen_redraw)
			redraw_screen = TRUE;

		/*
		 * If we were using the paged art return to our former position
		 */
		if (use_current)
			curr_line = saved_curr_line;

		if (redraw_screen)
			draw_page(0);
		else {
			if (function == FEED_PIPE)
				clear_message();
		}
	} else {
		if (redraw_screen) {
			currmenu->redraw();
			redraw_screen = FALSE;
		}
	}

	/*
	 * Finally print a status message
	 */
	switch (function) {
		case FEED_MAIL:
			if (tinrc.interactive_mailer != INTERACTIVE_NONE)
				info_message(_(txt_external_mail_done));
			else
				info_message(P_(txt_article_mailed_sp[0], txt_article_mailed_sp[1], counter.success), counter.success);
			break;

		case FEED_MARK_READ:
		case FEED_MARK_UNREAD:
			if (no_next_unread)
				info_message(_(txt_no_next_unread_art));
			else {
				if (counter.success && level != PAGE_LEVEL) {
					switch (feed_type) {
						case FEED_THREAD:
							info_message((function == FEED_MARK_READ) ?
								_(txt_marked_thread_as_read) :
								_(txt_marked_thread_as_unread));
							break;

						case FEED_ARTICLE:
							info_message((function == FEED_MARK_READ) ?
								_(txt_marked_article_as_read) :
								_(txt_marked_article_as_unread));
							break;

						default:
							/* FIXME: translatable/plural-forms */
							info_message((function == FEED_MARK_READ) ?
								_(txt_marked_arts_as_read) :
								_(txt_marked_arts_as_unread),
								counter.success, counter.max, P_(txt_article_sp[0], txt_article_sp[1], counter.max));
							break;
					}
				}
			}
			break;

#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
			info_message(P_(txt_article_piped_sp[0], txt_article_piped_sp[1], counter.success), counter.success, tinrc.default_pipe_command);
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			info_message(P_(txt_article_printed_sp[0], txt_article_printed_sp[1], counter.success), counter.success);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:		/* Reporting done earlier */
		case FEED_AUTOSAVE:
		default:
			break;
	}
	return no_next_unread ? 1 : 0;
}


#ifndef DISABLE_PRINTING
static t_bool
print_file(
	const char *command,
	int respnum,
	t_openartinfo *artinfo)
{
	FILE *fp;
	const struct t_header *hdr = &artinfo->hdr;
	t_bool ok = FALSE;
#	ifdef DONT_HAVE_PIPING
	char cmd[PATH_LEN], file[PATH_LEN];
	int i;
#	endif /* DONT_HAVE_PIPING */

#	ifdef DONT_HAVE_PIPING
	snprintf(file, sizeof(file), TIN_PRINTFILE, respnum);
	if ((fp = fopen(file, "w")) == NULL) /* TODO: issue a more correct error message here */
#	else
	if (!command || !*command || (fp = popen(command, "w")) == NULL)
#	endif /* DONT_HAVE_PIPING */
	{
		perror_message(_(txt_command_failed), BlankIfNull(command));
		return ok;
	}

	rewind(artinfo->raw);
	if (!curr_group->attribute->print_header && !(fseek(artinfo->raw, hdr->ext->offset, SEEK_SET))) {	/* -> start of body */
		if (hdr->newsgroups)
			fprintf(fp, "Newsgroups: %s\n", hdr->newsgroups);
		if (arts[respnum].mailbox.from == arts[respnum].mailbox.name || arts[respnum].mailbox.name == NULL)
			fprintf(fp, "From: %s\n", arts[respnum].mailbox.from);
		else
			fprintf(fp, "From: %s <%s>\n", arts[respnum].mailbox.name, arts[respnum].mailbox.from);
		if (hdr->subj)
			fprintf(fp, "Subject: %s\n", hdr->subj);
		if (hdr->date)
			fprintf(fp, "Date: %s\n\n", hdr->date);
	}

	if (copy_fp(artinfo->raw, fp) == 0)
		ok = TRUE;

#	ifdef DONT_HAVE_PIPING
	fclose(fp);
	i = snprintf(cmd, sizeof(cmd), "%s %s", command, file);
	if (i > 0 && i < (int) sizeof(cmd))
		invoke_cmd(cmd);
	else {
		perror_message(_(txt_command_failed), cmd);
		unlink(file);
		return FALSE;
	}
	unlink(file);
#	else
	fflush(fp);
	pclose(fp);
#	endif /* DONT_HAVE_PIPING */

	return ok;
}
#endif /* !DISABLE_PRINTING */
