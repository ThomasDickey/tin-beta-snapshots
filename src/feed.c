/*
 *  Project   : tin - a Usenet reader
 *  Module    : feed.c
 *  Author    : I. Lea
 *  Created   : 1991-08-31
 *  Updated   : 2003-04-25
 *  Notes     : provides same interface to mail,pipe,print,save & repost commands
 *
 * Copyright (c) 1991-2003 Iain Lea <iain@bricbrac.de>
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

#ifdef DEBUG
#	ifndef TCURSES_H
#		include "tcurses.h"
#	endif /* !TCURSES_H */
#endif /* DEBUG */

#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */

#ifndef RFC2046_H
#	include "rfc2046.h"
#endif /* !RFC2046_H */


static char proc_ch;					/* Post-processing type when saving */
static t_bool confirm;					/* only used for FEED_MAIL */
static t_bool is_mailbox = FALSE;
static t_bool redraw_screen = FALSE;
static t_bool supersede = FALSE;		/* for reposting only */
#ifndef DONT_HAVE_PIPING
	static FILE *pipe_fp = (FILE *) 0;
#endif /* !DONT_HAVE_PIPING */


struct t_counters {
	int success;		/* # arts fed okay */
	int total;			/* # arts fed */
	int max;			/* initial guesstimate of total */
};

/*
 * Local prototypes
 */
static char *get_save_filename(struct t_group *group, int function, char *filename, int filelen);
static int get_feed_key(int function, int level, struct t_group *group, struct t_art_stat *thread, int respnum);
static int get_post_proc_type(void);
static t_bool feed_article(int art, int function, struct t_counters *counter, t_bool use_current, const char *data, const char *path);
static void print_save_summary(char type, int fed);
#ifndef DISABLE_PRINTING
	static t_bool print_file(const char *command, int respnum, t_openartinfo *artinfo);
#endif /* !DISABLE_PRINTING */

#ifndef DONT_HAVE_PIPING
#	define handle_SIGPIPE()	if (got_sig_pipe) goto got_sig_pipe_while_piping
#else
#	define handle_SIGPIPE() /*nothing*/
#endif /* DONT_HAVE_PIPING */

/*
 * 'filename' holds 'filelen' amount of storage in which to place the
 * filename to save-to. The filename is also returned after basic syntax
 * checking. We default to the global save filename or group specific
 * filename if it exists
 */
static char *
get_save_filename(
	struct t_group *group,
	int function,
	char *filename,
	int filelen)
{
	char default_savefile[PATH_LEN];

	filename[0] = '\0';

	/*
	 * Group attribute savefile overrides tinrc default savefile
	 */
	strncpy(default_savefile, (group->attribute->savefile ? group->attribute->savefile : tinrc.default_save_file), sizeof(default_savefile) - 1);

	if (function == FEED_SAVE) {
		if (!prompt_default_string(_(txt_save_filename), filename, filelen, default_savefile, HIST_SAVE_FILE)) {
			clear_message();
			return NULL;
		}
		str_trim(filename);
	}

	/*
	 * Update attribute/tinrc default savefiles if changed
	 */
	if (*filename) {
		if (group->attribute->savefile) {
			free(group->attribute->savefile);
			group->attribute->savefile = my_strdup(filename);
		}
		my_strncpy(tinrc.default_save_file, filename, sizeof(tinrc.default_save_file) - 1);
	} else {									/* No file was specified, try default */
		/*
		 * None chosen (or AUTOSAVING), use tinrc default
		 */
		if (*default_savefile)
			my_strncpy(filename, default_savefile, filelen - 1);
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
 * Find out what post-processing to perform.
 * This is not used when saving to mailboxes (we don't postprocess mailboxen)
 * Also not used when using the auto-save feature because a default value is
 * taken from the group attributes
 * Return a post_proc_char or 0 if aborting the save process
 */
static int
get_post_proc_type(
	void)
{
	char ch;
	char keyno[MAXKEYLEN], keyyes[MAXKEYLEN], keyquit[MAXKEYLEN];
	char keyshar[MAXKEYLEN];

	ch = (char) prompt_slk_response(proc_ch_default,
				&menukeymap.feed_post_process_type,
				_(txt_choose_post_process_type),
				printascii(keyno, map_to_local(iKeyPProcNo, &menukeymap.feed_post_process_type)),
				printascii(keyyes, map_to_local(iKeyPProcYes, &menukeymap.feed_post_process_type)),
				printascii(keyshar, map_to_local(iKeyPProcShar, &menukeymap.feed_post_process_type)),
				printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.feed_post_process_type)));

	if (ch == iKeyQuit || ch == iKeyAbort) {			/* exit */
		clear_message();
		return 0;
	}
	return ch;
}


/*
 * Return the key mapping for what we are intending to process or 0 if save
 * process is being aborted
 * Key can be (current) article, (current) thread, tagged articles,
 * hot articles, or articles matching a pattern
 * This is automatic in the various auto-save cases, in other
 * cases this is prompted for based on a chosen default
 */
static int
get_feed_key(
	int function,
	int level,
	struct t_group *group,
	struct t_art_stat *thread,
	int respnum)
{
	constext *prompt;
	int ch, ch_default;

	/*
	 * Try and work out what default the user wants
	 * thread->total = # arts in thread
	 */
	ch_default = (num_of_tagged_arts ? iKeyFeedTag :
					(level == GROUP_LEVEL && thread->total > 1 ? iKeyFeedThd :
						(thread->selected_total ? iKeyFeedHot : iKeyFeedArt)));

	switch (function) {
		case FEED_MAIL:
			prompt = txt_mail;
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

		case FEED_SAVE:
			prompt = txt_save;
			break;

		case FEED_REPOST:
			if (!can_post) {				/* Get this over with before asking any Q's */
				info_message(_(txt_cannot_post));
				return 0;
			}
			prompt = txt_repost;
			break;

		default:
			prompt = "";
			break;
	}

	/*
	 * TODO: someone please rewrite this in a way that is easily understandable
	 * We only query for what to feed when:
	 *	We are NOT auto'S'aving
	 *  auto_save is set and we did not 's'ave (ie, we're print/pipe/mailing?)
	 *	?????
	 */
	if (
		(
			!(group->attribute->auto_save && arts[respnum].archive) ||
			 (group->attribute->auto_save && function != FEED_SAVE) ||
			  ch_default == iKeyFeedTag
		)
			&& function != FEED_AUTOSAVE_TAGGED
	) {
		char buf[LEN];
		char keyart[MAXKEYLEN], keythread[MAXKEYLEN], keyhot[MAXKEYLEN];
		char keypat[MAXKEYLEN], keytag[MAXKEYLEN], keyquit[MAXKEYLEN];

		snprintf(buf, sizeof(buf), _(txt_art_thread_regex_tag),
			printascii(keyart, map_to_local(iKeyFeedArt, &menukeymap.feed_art_thread_regex_tag)),
			printascii(keythread, map_to_local(iKeyFeedThd, &menukeymap.feed_art_thread_regex_tag)),
			printascii(keyhot, map_to_local(iKeyFeedHot, &menukeymap.feed_art_thread_regex_tag)),
			printascii(keypat, map_to_local(iKeyFeedPat, &menukeymap.feed_art_thread_regex_tag)),
			printascii(keytag, map_to_local(iKeyFeedTag, &menukeymap.feed_art_thread_regex_tag)),
			printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.feed_art_thread_regex_tag)));

		ch = prompt_slk_response(ch_default, &menukeymap.feed_art_thread_regex_tag, "%s %s", _(prompt), buf);
	} else
		ch = ch_default;

	switch (ch) {
		case iKeyQuit:
		case iKeyAbort:
			clear_message();
			return 0;

		case iKeyFeedPat:
			sprintf(mesg, _(txt_feed_pattern), tinrc.default_pattern);
			if (!(prompt_string_default(mesg, tinrc.default_pattern, _(txt_no_match), HIST_REGEX_PATTERN)))
				return 0;
			break;

		default:
			break;
	}

	return ch;
}


/*
 * Print a message like:
 * -- [Article|Thread|Tagged Articles] saved to [mailbox] [filenames] --
 * 'fed' is the number of articles we tried to save
 */
static void
print_save_summary(
	char type,
	int fed)
{
	const char *first, *last;
	char buf[LEN];
	char what[LEN];

	if (fed != num_save)
		wait_message(2, _(txt_warn_not_all_arts_saved), fed, num_save);

	switch (type) {
		case iKeyFeedHot:
			snprintf(what, sizeof(what), _(txt_prefix_hot), PLURAL(fed, txt_article));
			break;

		case iKeyFeedTag:
			snprintf(what, sizeof(what), _(txt_prefix_tagged), PLURAL(fed, txt_article));
			break;

		case iKeyFeedThd:
			STRCPY(what, _(txt_thread));
			break;

		case iKeyFeedArt:
		case iKeyFeedPat:
		default:
			snprintf(what, sizeof(what), "%s", PLURAL(fed, txt_article));
			break;
	}

	first = (save[0].mailbox) ? save[0].path : save[0].file;
	last = (save[num_save - 1].mailbox) ? save[num_save - 1].path : save[num_save - 1].file;

	/*
	 * We report the range of saved-to files for regular saves of > 1 articles
	 */
	if (num_save == 1 || save[0].mailbox)
		snprintf(buf, sizeof(buf), _(txt_saved_to),
			what, (save[0].mailbox ? _(txt_mailbox) : ""), first);
	else
		snprintf(buf, sizeof(buf), _(txt_saved_to_range),
			what, first, last);

	wait_message((tinrc.beginner_level) ? 2 : 1, buf);

	return;
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
	const char *path)
{
	t_bool ok = TRUE;		/* Assume success */
	t_openartinfo openart;
	t_openartinfo *openartptr = &openart;

	counter->total++;

	if (use_current)
		openartptr = &pgart;			/* Use art already open in pager */
	else {
		if (art_open(FALSE, &arts[art], path, openartptr, TRUE) < 0)	/* User abort or an error */
			return FALSE;
	}

	switch (function) {
		case FEED_MAIL:
			switch (mail_to_someone(tinrc.default_mail_address, confirm, openartptr)) {
				case POSTED_REDRAW:
					redraw_screen = TRUE;
					/* FALLTHROUGH */
				case POSTED_NONE:
					ok = FALSE;
					break;

				case POSTED_OK:
					break;
			}

			confirm = bool_not(ok);		/* Only confirm the next one after a failure */
			break;

#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
			/* TODO: looks odd because screen mode is raw */
			show_progress(_(txt_piping), counter->success + 1, counter->max);

			rewind(openartptr->raw);
			ok = copy_fp(openartptr->raw, pipe_fp);	/* Check for SIGPIPE on return */
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			/* TODO: looks odd because screen mode is raw */
			show_progress(_(txt_printing), counter->success + 1, counter->max);

			ok = print_file(data /*print_command*/, art, openartptr);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			wait_message(0, "%s %d  ", _(txt_saving), counter->success + 1);
			ok = save_and_process_art(openartptr, &arts[art], is_mailbox, data /*filename*/, counter->max, (proc_ch != iKeyPProcNo));
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

	/*
	 * Mark read for the SAVE cases (but not print/pipe etc..)
	 */
	if (function == FEED_SAVE || function == FEED_AUTOSAVE_TAGGED) {
		if (ok && tinrc.mark_saved_read)
			art_mark(&CURR_GROUP, &arts[art], ART_READ);
	}

	if (!use_current)
		art_close(openartptr);
	return ok;
}


/*
 * Single entry point for 'feed'ing article(s) to a backend
 * Function:
 *	FEED_PIPE, FEED_MAIL, FEED_PRINT, FEED_REPOST
 *	FEED_SAVE, FEED_AUTOSAVE_TAGGED
 * Level:
 *	GROUP_LEVEL, THREAD_LEVEL, PAGE_LEVEL
 * Respnum:
 *	Index in arts[] of starting article
 *
 * The following 'groups' of article can be processed:
 *	Single (current) article
 *	Current thread
 *	Tagged articles
 *	Hot articles
 *	Articles matching a pattern
 *
 * The selection of Function depends on the key used to get here.
 * The selection of which article 'group' to process is managed
 * inside here, or by defaults.
 */
void
feed_articles(
	int function,
	int level,
	struct t_group *group,
	int respnum)
{
	char group_path[PATH_LEN];
	char outpath[PATH_LEN];
	int art;
	int feed_type;
	int i;
	int saved_curr_line = -1;
	int thread_base;
	struct t_art_stat sbuf;
	struct t_counters counter = { 0, 0, 0 };
	t_bool use_current = FALSE;
	t_bool ret1 = FALSE;
	t_bool post_processed_ok = FALSE;

#ifdef DONT_HAVE_PIPING
	if (function == FEED_PIPE) {
		error_message(_(txt_piping_not_enabled));
		clear_message();
		return;
	}
#endif /* DONT_HAVE_PIPING */

	set_xclick_off();		/* TODO: there is no corresponding set_xclick_on()? */
	make_group_path(group->name, group_path);
	thread_base = which_thread(respnum);
	stat_thread(thread_base, &sbuf);

	if ((feed_type = get_feed_key(function, level, group, &sbuf, respnum)) == 0)
		return;

	/*
	 * Get whatever information is needed to proceed
	 */
	switch (function) {
		/* Setup mail - get address to mail to */
		case FEED_MAIL:
			sprintf(mesg, _(txt_mail_art_to), cCOLS - (strlen(_(txt_mail_art_to)) + 30), tinrc.default_mail_address);
			if (!(prompt_string_default(mesg, tinrc.default_mail_address, _(txt_no_mail_address), HIST_MAIL_ADDRESS)))
				return;
			break;

#ifndef DONT_HAVE_PIPING
		/* Setup pipe - get pipe-to command and open the pipe */
		case FEED_PIPE:
			sprintf(mesg, _(txt_pipe_to_command), cCOLS - (strlen(_(txt_pipe_to_command)) + 30), tinrc.default_pipe_command);
			if (!(prompt_string_default(mesg, tinrc.default_pipe_command, _(txt_no_command), HIST_PIPE_COMMAND)))
				return;

			got_sig_pipe = FALSE;
			EndWin(); /* Turn off curses/windowing */
			Raw(FALSE);
			fflush(stdout);
			if ((pipe_fp = popen(tinrc.default_pipe_command, "w")) == NULL) {
				perror_message(_(txt_command_failed), tinrc.default_pipe_command);
				Raw(TRUE);
				InitWin();
				return;
			}
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		/* Setup printing - get print command line */
		case FEED_PRINT:
			sprintf(outpath, "%s %s", tinrc.printer, REDIRECT_OUTPUT);
			break;
#endif /* !DISABLE_PRINTING */

		/*
		 * Setup saving, some of these are generated automatically
		 *	Determine path/file to save to
		 *	Determine post-processing type
		 *	Determine if post processed file deletion required
		 */
		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			{
				char savefile[PATH_LEN];

				/* This will force automatic selection unless changed by user */
				savefile[0] = '\0';

				/*
				 * Only explicitly ask in these cases, otherwise generation is
				 * automatic in expand_save_filename()
				 */
				if (!group->attribute->auto_save || (arts[respnum].archive == NULL)) {
					if (get_save_filename(group, function, savefile, sizeof(savefile)) == NULL)
						return;
				}

				proc_ch = proc_ch_default;

				/* We don't postprocess mailboxen */
				if ((is_mailbox = expand_save_filename(outpath, savefile)) == TRUE)
					proc_ch = iKeyPProcNo;
				else {
					if ((proc_ch = get_post_proc_type()) == 0)
						return;
				}
				if (!create_path(outpath))
					return;
			}
			break;

		/* repost (or supersede) article */
		case FEED_REPOST:
			{
#ifndef FORGERY
				char from_name[PATH_LEN];

				get_from_name(from_name, (struct t_group *) 0);

				if (strstr(from_name, arts[respnum].from)) {
#endif /* !FORGERY */
					char buf[LEN];
					char keyrepost[MAXKEYLEN], keysupersede[MAXKEYLEN];
					char keyquit[MAXKEYLEN];
					char option;

					/* repost or supersede? */
					snprintf(buf, sizeof(buf), _(txt_supersede_article),
							printascii(keyrepost, map_to_local(iKeyFeedRepost, &menukeymap.feed_supersede_article)),
							printascii(keysupersede, map_to_local(iKeyFeedSupersede, &menukeymap.feed_supersede_article)),
							printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.feed_supersede_article)));
					option = (char) prompt_slk_response(iKeyFeedSupersede,
										&menukeymap.feed_supersede_article, "%s",
										sized_message(buf, arts[respnum].subject));

					switch (option) {
						case iKeyFeedSupersede:
							sprintf(mesg, _(txt_supersede_group), tinrc.default_repost_group);
							supersede = TRUE;
							break;

						case iKeyFeedRepost:
							sprintf(mesg, _(txt_repost_group), tinrc.default_repost_group);
							supersede = FALSE;
							break;

						default:
							clear_message();
							return;
					}
#ifndef FORGERY
				} else {
					sprintf(mesg, _(txt_repost_group), tinrc.default_repost_group);
					supersede = FALSE;
				}
#endif /* !FORGERY */
				if (!(prompt_string_default(mesg, tinrc.default_repost_group, _(txt_no_group), HIST_REPOST_GROUP)))
					return;
			}
			break;

		default:
			break;
	} /* switch (function) */

	confirm = TRUE;				/* Always confirm the first time */
	clear_message();

	/*
	 * Performance hack - If we feed a single art from the pager then we can
	 * re-use the currently open article
	 */
	if (level == PAGE_LEVEL && feed_type == iKeyFeedArt) {
		saved_curr_line = curr_line;		/* Save where we were in pager */
		use_current = TRUE;
	}

	/*
	 * This is the main loop
	 * The general idea is to feed_article() for every article to be processed
	 */
	switch (feed_type) {
		case iKeyFeedArt:		/* article */
			counter.max = 1;
			if (!feed_article(respnum, function, &counter, use_current, outpath, group_path))
				handle_SIGPIPE();
			break;

		case iKeyFeedThd:		/* thread */
			counter.max = sbuf.total;
			for_each_art_in_thread(art, which_thread(respnum)) {
				if (tinrc.process_only_unread && arts[art].status == ART_READ)
					continue;
				/* Keep going - ignore errors */
				feed_article(art, function, &counter, use_current, outpath, group_path);
				handle_SIGPIPE();
			}
			break;

		case iKeyFeedTag:		/* tagged articles */
			counter.max = num_of_tagged_arts;
			for (i = 1; i <= num_of_tagged_arts; i++) {
				for_each_art(art) {
					/* process_only_unread does NOT apply on tagged arts */
					if (arts[art].tagged == i) {
						/* Keep going - ignore errors */
						feed_article(art, function, &counter, use_current, outpath, group_path);
						handle_SIGPIPE();
					}
				}
			}

			untag_all_articles();	/* TODO: this will untag even on partial failure */
			break;

		case iKeyFeedHot:		/* hot (auto-selected) articles */
		case iKeyFeedPat:		/* pattern matched articles */
			for_each_art(art) {
				if (feed_type == iKeyFeedPat) {
					if (!REGEX_MATCH(arts[art].subject, tinrc.default_pattern, TRUE))
						continue;
				} else if (!arts[art].selected)
					continue;

				if (tinrc.process_only_unread && arts[art].status == ART_READ)
					continue;

				arts[art].matched = TRUE;
				counter.max++;
			}

			/* I think we nest like this to preserve any 'ordering' of the arts */
			for (i = 0; i < grpmenu.max; i++) {
				for_each_art_in_thread(art, i) {
					if (!arts[art].matched)
						continue;
					arts[art].matched = FALSE;

					if (feed_article(art, function, &counter, use_current, outpath, group_path)) {
						if (feed_type == iKeyFeedHot)
							arts[art].selected = FALSE;
					} else
						handle_SIGPIPE();
				}
			}
			break;

		default:			/* Should never get here */
			break;
	} /* switch (ch) */

	/*
	 * Invoke post-processing if needed
	 * Work out what (if anything) needs to be redrawn
	 */
	if (!tinrc.use_mailreader_i)
		redraw_screen |= mail_check();	/* in case of sending to oneself */

	switch (function) {
#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
got_sig_pipe_while_piping:
			if (got_sig_pipe)
				perror_message(_(txt_command_failed), tinrc.default_pipe_command);
			got_sig_pipe = FALSE;
			fflush(pipe_fp);
			(void) pclose(pipe_fp);
			Raw(TRUE);
			InitWin();
			prompt_continue();
			redraw_screen = TRUE;
			break;
#endif /* !DONT_HAVE_PIPING */

		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			if (num_save == 0) {
				wait_message(1, _(txt_saved_nothing));
				break;
			}

			print_save_summary(feed_type, counter.total);
			if (proc_ch != iKeyPProcNo) {
				t_bool delete_post_proc = FALSE;

				if (CURR_GROUP.attribute->delete_tmp_files)
					delete_post_proc = TRUE;
				else {
					if (function != FEED_AUTOSAVE_TAGGED) {
						if (prompt_yn(cLINES, _(txt_delete_processed_files), TRUE) == 1)
							delete_post_proc = TRUE;
					}
				}
				post_processed_ok = post_process_files(proc_ch, delete_post_proc);
			}
			free_save_array();		/* NB: This is where num_save etc.. gets purged */
			break;

		default:
			break;
	}

	if (level != PAGE_LEVEL)
		ret1 = tinrc.mark_saved_read;

	if (ret1 || post_processed_ok)
		redraw_screen = TRUE;

	if (level == PAGE_LEVEL) {
		if (tinrc.force_screen_redraw)
			redraw_screen = TRUE;

		/*
		 * If we were using the paged art return to our former position
		 */
		if (use_current)
			curr_line = saved_curr_line;

		if (redraw_screen)
			draw_page(group->name, 0);
		else {
			if (function == FEED_PIPE)
				clear_message();
		}
	} else {
		if (redraw_screen)
			currmenu->redraw();
	}

	/*
	 * Finally print a status message
	 */
	switch (function) {
		case FEED_MAIL:
			if (tinrc.use_mailreader_i)
				info_message(_(txt_external_mail_done));
			else
				info_message(_(txt_articles_mailed), counter.success, PLURAL(counter.success, txt_article));
			break;

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			info_message(_(txt_articles_printed), counter.success, PLURAL(counter.success, txt_article));
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:		/* Reporting done earlier */
		case FEED_AUTOSAVE_TAGGED:
		default:
			break;
	}
}


#ifndef DISABLE_PRINTING
static t_bool
print_file(
	const char *command,
	int respnum,
	t_openartinfo *artinfo)
{
	FILE *fp;
	t_bool ok = TRUE;
	struct t_header *hdr = &artinfo->hdr;
#	ifdef DONT_HAVE_PIPING
	char cmd[PATH_LEN], file[PATH_LEN];
#	endif /* DONT_HAVE_PIPING */

#	ifdef DONT_HAVE_PIPING
	snprintf(file, sizeof(file), TIN_PRINTFILE, respnum);
	if ((fp = fopen(file, "w")) == NULL) /* TODO: issue a more correct error message here */
#	else
	if ((fp = popen(command, "w")) == NULL)
#	endif /* DONT_HAVE_PIPING */
	{
		perror_message(_(txt_command_failed), command);
		return FALSE;
	}

	if (tinrc.print_header)
		rewind(artinfo->raw);
	else {
		if (hdr->newsgroups)
			fprintf(fp, "Newsgroups: %s\n", hdr->newsgroups);
		if (arts[respnum].from == arts[respnum].name || arts[respnum].name == NULL)
			fprintf(fp, "From: %s\n", arts[respnum].from);
		else
			fprintf(fp, "From: %s <%s>\n", arts[respnum].name, arts[respnum].from);
		if (hdr->subj)
			fprintf(fp, "Subject: %s\n", hdr->subj);
		if (hdr->date)
			fprintf(fp, "Date: %s\n\n", hdr->date);
		fseek(artinfo->raw, hdr->ext->offset, SEEK_SET);	/* -> start of body */
	}

	ok = copy_fp(artinfo->raw, fp);

#	ifdef DONT_HAVE_PIPING
	fclose(fp);
	strncpy(cmd, command, sizeof(cmd) - 2);
	strcat(cmd, " ");
	strncat(cmd, file, sizeof(cmd) - strlen(cmd) - 1);
	cmd[sizeof(cmd) - 1] = '\0';
	invoke_cmd(cmd);
	unlink(file);
#	else
	fflush(fp);
	pclose(fp);
#	endif /* DONT_HAVE_PIPING */

	return ok;
}
#endif /* !DISABLE_PRINTING */
