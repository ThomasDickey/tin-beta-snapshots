/*
 *  Project   : tin - a Usenet reader
 *  Module    : feed.c
 *  Author    : I. Lea
 *  Created   : 1991-08-31
 *  Updated   : 2000-02-08
 *  Notes     : provides same interface to mail,pipe,print,save & repost commands
 *
 * Copyright (c) 1991-2002 Iain Lea <iain@bricbrac.de>
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

/*
 * Post-processing type when saving
 * Set to <0 before post proc type is selected
 * Set to 0 if user aborted save process
 * We do this hackery to honour the "don't postprocess mailboxes" rule
 */
static signed char proc_ch;

#ifndef DONT_HAVE_PIPING
	static FILE *pipe_fp = (FILE *) 0;
#endif /* !DONT_HAVE_PIPING */

t_bool is_mailbox = FALSE;
static t_bool confirm;
static t_bool redraw_screen = FALSE;
static t_bool supersede = FALSE;			/* for reposting only */

/*
 * Local prototypes
 */
#ifndef DISABLE_PRINTING
	static t_bool print_file (const char *command, int respnum, t_openartinfo *artinfo);
#endif /* !DISABLE_PRINTING */

#ifndef DONT_HAVE_PIPING
#	define handle_SIGPIPE() if (got_sig_pipe) goto got_sig_pipe_while_piping
#else
#	define handle_SIGPIPE() /*nothing*/
#endif /* DONT_HAVE_PIPING */

/*
 * 'filename' holds 'filelen' amount of storage in which to place the filename
 * to save-to. The filename is also returned after basic syntax checking.
 * We default to the global save filename or group specific filename if it exists
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

	strcpy (default_savefile, (group->attribute->savefile ? group->attribute->savefile : tinrc.default_save_file));

	if (function != FEED_AUTOSAVE_TAGGED) {			/* ie, FEED_SAVE */
		if (!prompt_default_string (_(txt_save_filename), filename, filelen, default_savefile, HIST_SAVE_FILE)) {
			clear_message ();
			return NULL;
		}
		str_trim (filename);
	}

	if (*filename) {
		if (group->attribute->savefile) {
			free (group->attribute->savefile);
			group->attribute->savefile = my_strdup (filename);
		}
		my_strncpy (tinrc.default_save_file, filename, sizeof (tinrc.default_save_file));
	} else {									/* No file was specified, try default */
		if (*default_savefile)
			my_strncpy (filename, default_savefile, LEN);
		else {									/* No default either */
			info_message (_(txt_no_filename));
			return NULL;
		}
	}

	/*
	 * Punt invalid expansions
	 */
	if ((filename[0] == '~' || filename[0] == '+') && filename[1] == '\0') {
		info_message (_(txt_no_filename));
		return NULL;
	}

	return filename;
}


/*
 * Find out what post-processing to perform.
 * This is not used when saving to mailboxes
 * Also not used when using the auto-save feature as a default value is taken
 * from the group attributes
 * Return a post_proc char or 0 if aborting the save process
 */
static int
get_post_proc_type(
	void)
{
	char ch;
	char keynone[MAXKEYLEN], keyquit[MAXKEYLEN], keyshar[MAXKEYLEN];
	char keyuud[MAXKEYLEN];

	ch = (char) prompt_slk_response (proc_ch_default,
				&menukeymap.feed_post_process_type,
				_(txt_choose_post_process_type),
				printascii (keynone, map_to_local(iKeyPProcNone, &menukeymap.feed_post_process_type)),
				printascii (keyshar, map_to_local(iKeyPProcShar, &menukeymap.feed_post_process_type)),
				printascii (keyuud, map_to_local(iKeyPProcUUDecode, &menukeymap.feed_post_process_type)),
				printascii (keyquit, map_to_local(iKeyQuit, &menukeymap.feed_post_process_type)));

	if (ch == iKeyQuit || ch == iKeyAbort) {			/* exit */
		clear_message ();
		return 0;
	}

	return ch;
}


/*
 * This is the handler that processes a single article for all the
 * various FEED_ functions. 'art' is the index in arts[]
 * Assumes no article is open when we enter -  opens and closes the art being
 * processed. As a performance hack this is not done if 'use_current' is set.
 * Returns TRUE if the article was processed okay
 */
static t_bool
feed_article(
	int art,
	int function,
	int *curr,				/* Current # being processed */
	int max,				/* Total # items being processed, if known */
	t_bool use_current,		/* Use already open pager article */
	const char *data,		/* Extra data if needed, print command or save filename */
	const char *path)
{
	t_bool ok = TRUE;		/* Assume success */
	t_openartinfo openart;
	t_openartinfo *openartptr = &openart;

	/*
	 * The save function add_to_save_list() only queues up a batch and doesn't
	 * need an open article. The actual transfer of data comes later
	 */
	if (use_current)
		openartptr = &pgart;			/* Use art open in pager */
	else {
		if (function == FEED_SAVE || function == FEED_AUTOSAVE_TAGGED) {
			/* When saving, just check that it exists for now */
			if (!stat_article (arts[art].artnum, path))
				return FALSE;
		} else {
			memset (openartptr, 0, sizeof(t_openartinfo));
			if (art_open (FALSE, &arts[art], path, openartptr) == ART_UNAVAILABLE)
				return FALSE;
		}
	}

	switch (function) {
		case FEED_MAIL:
			switch (mail_to_someone (tinrc.default_mail_address, confirm, openartptr)) {
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
			/* TODO - looks odd because screen mode is raw */
			if (max)
				show_progress (_(txt_piping), (*curr) + 1, max);
			else
				wait_message (0, "%s %d", _(txt_piping), (*curr) + 1);

			rewind (openartptr->raw);
			ok = copy_fp (openartptr->raw, pipe_fp);	/* Check for SIGPIPE on return */
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			if (max)
				show_progress (_(txt_printing), (*curr) + 1, max);
			else
				wait_message (0, "%s %d", _(txt_printing), (*curr) + 1);

			ok = print_file (data /*print_command*/, art, openartptr);
			break;
#endif /* !DISABLE_PRINTING */

		/* add_to_save_list() is instant, the actual saving happens later */
		case FEED_SAVE:
			if (!(is_mailbox = add_to_save_list (&arts[art], data /*filename*/))) {
				if (proc_ch < 0 && (proc_ch = get_post_proc_type()) == 0)
					ok = FALSE;
			}
			break;

		case FEED_AUTOSAVE_TAGGED:
			add_to_save_list (&arts[art], data /*filename*/);
			break;

		case FEED_REPOST:
			if (repost_article (tinrc.default_repost_group, art, supersede, openartptr) == POSTED_NONE)
				ok = FALSE;
			else			/* POSTED_REDRAW, POSTED_OK */
				redraw_screen = TRUE;
			break;

		default:
			break;
	}
	if (ok)
		(*curr)++;

	/*
	 * Don't mark read when saving - this is only done by save_art_to_file()
	 * Likewise, no art is open at this stage when saving
	 */
	if (!(function == FEED_SAVE || function == FEED_AUTOSAVE_TAGGED)) {
		if (!use_current)
			art_close (openartptr);
		if (ok && tinrc.mark_saved_read)
			art_mark_read (&CURR_GROUP, &arts[art]);
	}

	return ok;
}


void
feed_articles (
	int function,
	int level,
	struct t_group *group,
	int respnum)
{
	char group_path[PATH_LEN];
	char output[PATH_LEN];
	constext *prompt;
	int ch, ch_default;
	int i, j;
	int count = 0;
	int thread_base;
	int saved_curr_line = -1;
	t_bool use_current = FALSE;
	t_bool processed_ok = TRUE;
	t_bool ret1 = FALSE;
	t_bool ret2 = FALSE;

#ifdef DONT_HAVE_PIPING
	if (function == FEED_PIPE) {
		error_message (_(txt_piping_not_enabled));
		clear_message ();
		return;
	}
#endif /* DONT_HAVE_PIPING */

	set_xclick_off ();

	thread_base = which_thread (respnum);

	proc_ch = -1;			/* No selection made yet */

	/*
	 * try and work out what default the user wants
	 * This is dumb. If you have _any_ selected arts, then it picks 'h'
	 * No it's not, that's exactly what you want (in binary groups)
	 */
	ch_default = (num_of_tagged_arts ? iKeyFeedTag :
					(level == GROUP_LEVEL && HAS_FOLLOWUPS (thread_base) ? iKeyFeedThd :
						(num_of_selected_arts ? iKeyFeedHot :
							iKeyFeedArt)));

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
				return;
			}
			prompt = txt_repost;
			break;
		default:
			prompt = "";
			break;
	}

	make_group_path (group->name, group_path);

	/*
	 * If not automatic, ask what the user wants to save
	 */
	if ((!(group->attribute->auto_save && arts[respnum].archive) || (group->attribute->auto_save && function != FEED_SAVE) || ch_default == iKeyFeedTag) && function != FEED_AUTOSAVE_TAGGED) {
		char buf[LEN];
		char keyart[MAXKEYLEN], keythread[MAXKEYLEN], keyhot[MAXKEYLEN];
		char keypat[MAXKEYLEN], keytag[MAXKEYLEN], keyquit[MAXKEYLEN];

		snprintf (buf, sizeof(buf) - 1, _(txt_art_thread_regex_tag),
			printascii (keyart, map_to_local (iKeyFeedArt, &menukeymap.feed_art_thread_regex_tag)),
			printascii (keythread, map_to_local (iKeyFeedThd, &menukeymap.feed_art_thread_regex_tag)),
			printascii (keyhot, map_to_local (iKeyFeedHot, &menukeymap.feed_art_thread_regex_tag)),
			printascii (keypat, map_to_local (iKeyFeedPat, &menukeymap.feed_art_thread_regex_tag)),
			printascii (keytag, map_to_local (iKeyFeedTag, &menukeymap.feed_art_thread_regex_tag)),
			printascii (keyquit, map_to_local (iKeyQuit, &menukeymap.feed_art_thread_regex_tag)));
		ch = prompt_slk_response (ch_default,
				&menukeymap.feed_art_thread_regex_tag,
				"%s %s", _(prompt), buf);
	} else
		ch = ch_default;

	switch (ch) {
		case iKeyQuit:
		case iKeyAbort:
			clear_message ();
			return;

		case iKeyFeedPat:
			sprintf (mesg, _(txt_feed_pattern), tinrc.default_regex_pattern);
			if (!(prompt_string_default(mesg, tinrc.default_regex_pattern, _(txt_no_match), HIST_REGEX_PATTERN)))
				return;
			break;

		default:
			break;
	}

	switch (function) {
		/* Setup mail - get address to mail to */
		case FEED_MAIL:
			sprintf (mesg, _(txt_mail_art_to), cCOLS - (strlen(_(txt_mail_art_to)) + 30), tinrc.default_mail_address);
			if (!(prompt_string_default(mesg, tinrc.default_mail_address, _(txt_no_mail_address), HIST_MAIL_ADDRESS)))
				return;
			break;

#ifndef DONT_HAVE_PIPING
		/* Setup pipe - get pipe-to command and open the pipe */
		case FEED_PIPE:
			sprintf (mesg, _(txt_pipe_to_command), cCOLS - (strlen(_(txt_pipe_to_command)) + 30), tinrc.default_pipe_command);
			if (!(prompt_string_default (mesg, tinrc.default_pipe_command, _(txt_no_command), HIST_PIPE_COMMAND)))
				return;

			got_sig_pipe = FALSE;

			/* Turn off curses/windowing */
			EndWin();
			Raw(FALSE);
			fflush(stdout);

			if ((pipe_fp = popen (tinrc.default_pipe_command, "w")) == (FILE *) 0) {
				perror_message (_(txt_command_failed), tinrc.default_pipe_command);
				Raw(TRUE);
				InitWin();
				return;
			}
			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			/* Setup print - get print command line */
			sprintf (output, "%s %s", tinrc.printer, REDIRECT_OUTPUT);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:		/* ask user for filename to save to */
		case FEED_AUTOSAVE_TAGGED:
			if (!group->attribute->auto_save || (arts[respnum].archive == NULL)) {
				if (get_save_filename (group, function, output, sizeof(output)) == NULL)
					return;
			}
			break;

		case FEED_REPOST:	/* repost article */
			{
#ifndef FORGERY
			char from_name[PATH_LEN];

			get_from_name (from_name, (struct t_group *) 0);

			if (strstr (from_name, arts[respnum].from)) {
#endif /* !FORGERY */
				char buf[LEN];
				char keyrepost[MAXKEYLEN], keysupersede[MAXKEYLEN];
				char keyquit[MAXKEYLEN];
				char option;

				/* repost or supersede ? */
				snprintf (buf, sizeof(buf), _(txt_supersede_article),
							printascii (keyrepost, map_to_local(iKeyFeedRepost, &menukeymap.feed_supersede_article)),
							printascii (keysupersede, map_to_local(iKeyFeedSupersede, &menukeymap.feed_supersede_article)),
							printascii (keyquit, map_to_local(iKeyQuit, &menukeymap.feed_supersede_article)));
				option = (char) prompt_slk_response (iKeyFeedSupersede,
										&menukeymap.feed_supersede_article, "%s",
										sized_message(buf, arts[respnum].subject));

				switch (option) {
					case iKeyFeedSupersede:
						sprintf (mesg, _(txt_supersede_group), tinrc.default_repost_group);
						supersede = TRUE;
						break;
					case iKeyFeedRepost:
						sprintf (mesg, _(txt_repost_group), tinrc.default_repost_group);
						supersede = FALSE;
						break;
					default:
						clear_message ();
						return;
				}
#ifndef FORGERY
			} else {
				sprintf (mesg, _(txt_repost_group), tinrc.default_repost_group);
				supersede = FALSE;
			}
#endif /* !FORGERY */
			if (!(prompt_string_default (mesg, tinrc.default_repost_group, _(txt_no_group), HIST_REPOST_GROUP)))
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
	if (level == PAGE_LEVEL && ch == iKeyFeedArt) {
		saved_curr_line = curr_line;		/* Save where we were in pager */
		use_current = TRUE;
	}

	switch (ch) {
		case iKeyFeedArt:		/* article */
			if (!feed_article (respnum, function, &count, 1, use_current, output, group_path)) {
				handle_SIGPIPE();
				/* No point testing proc_ch when handling only a single art */
				break;
			}

			if (proc_ch && function == FEED_SAVE) {
				if (use_current) {
					if (create_path (save[0].path))
						processed_ok = save_art_to_file (0, &pgart);
				} else
					processed_ok = save_batch (ch, group_path);
			}

			break;

		case iKeyFeedThd:		/* thread */
			/* Could stat_thread() and pass in 'max' to feed_article() */
			for (i = (int) base[thread_base]; i >= 0; i = arts[i].thread) {
				if (tinrc.process_only_unread && arts[i].status == ART_READ)
					continue;

				/* Ignore errors */
				feed_article (i, function, &count, 0, use_current, output, group_path);
				if (proc_ch == 0)
					break;

				handle_SIGPIPE();
			}
			if (proc_ch && function == FEED_SAVE) {
/* Can't see any need for this with real threading */
/*				sort_save_list ();*/
				processed_ok = save_batch (ch, group_path);
			}
			break;

		case iKeyFeedTag:		/* tagged articles */
			for (i = 1; i <= num_of_tagged_arts; i++) {
				for (j = 0; j < top_art; j++) {
					if (arts[j].tagged == i) {
						/* Ignore errors */
						feed_article(j, function, &count, num_of_tagged_arts, use_current, output, group_path);
						if (proc_ch == 0) {
							i = num_of_tagged_arts + 1;		/* Force break out of outer-loop */
							break;
						}

						handle_SIGPIPE();
					}
				}
			}

			if (proc_ch && (function == FEED_SAVE || function == FEED_AUTOSAVE_TAGGED))
				processed_ok = save_batch (ch, group_path);

			untag_all_articles ();
			break;

		case iKeyFeedHot:		/* hot (auto-selected) articles */
		case iKeyFeedPat:		/* regex pattern matched articles */
			for (i = 0; i < grpmenu.max; i++) {
				for (j = (int) base[i]; j >= 0; j = arts[j].thread) {
					if (ch == iKeyFeedPat) {
						if (!REGEX_MATCH(arts[j].subject, tinrc.default_regex_pattern, TRUE))
							continue;
					} else if (!arts[j].selected)
						continue;

					if (tinrc.process_only_unread && arts[j].status == ART_READ)
						continue;

					if (feed_article(j, function, &count, 0, use_current, output, group_path)) {
						if (ch == iKeyFeedHot) {
							arts[j].selected = FALSE;
							num_of_selected_arts--;
						}
					} else {
						if (proc_ch == 0) {
							i = grpmenu.max;		/* Force break out of outer-loop */
							break;
						}

						handle_SIGPIPE();
					}
				}
			}
			if (proc_ch && function == FEED_SAVE)
				processed_ok = save_batch (ch, group_path);

			break;

		default:			/* Should never get here */
			break;
	} /* switch (ch) */

	/*
	 * Now work out what (if anything) needs to be redrawn
	 */
	if (!tinrc.use_mailreader_i)
		redraw_screen |= mail_check ();	/* in case of sending to oneself */

	switch (function) {
#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
got_sig_pipe_while_piping:
#	if 1
			if (got_sig_pipe)
				perror_message (_(txt_command_failed), tinrc.default_pipe_command);
#	endif /* 1 */
			got_sig_pipe = FALSE;
			fflush (pipe_fp);
			(void) pclose (pipe_fp);
			Raw (TRUE);
			InitWin();
			prompt_continue ();
			redraw_screen = TRUE;
			break;
#endif /* !DONT_HAVE_PIPING */

		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			if (proc_ch != 'n' && !is_mailbox && processed_ok) {
				if (proc_ch < 0) {
					if (proc_ch_default != 'n')
					  ret2 = post_process_files (proc_ch_default, (function == FEED_SAVE ? FALSE : TRUE));
				} else
					ret2 = post_process_files (proc_ch, (function == FEED_SAVE ? FALSE : TRUE));
			}
			free_save_array ();		/* NB: This is where num_save etc.. gets purged */
			break;

		default:
			break;
	}

	if (level != PAGE_LEVEL)
		ret1 = tinrc.mark_saved_read;

	if (ret1 || ret2)
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
			draw_page (group->name, 0);
		else {
			if (function == FEED_PIPE)
				clear_message ();
		}
	} else {
		if (redraw_screen)
			currmenu->redraw();
	}

	switch (function) {
		case FEED_MAIL:
			if (tinrc.use_mailreader_i)
				info_message (_(txt_external_mail_done));
			else
				info_message (_(txt_articles_mailed), count, PLURAL(count, txt_article));
			break;

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			info_message (_(txt_articles_printed), count, PLURAL(count, txt_article));
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:		/* Reporting handled by save code */
		case FEED_AUTOSAVE_TAGGED:
		default:
			break;
	}
}

#ifndef DISABLE_PRINTING
static t_bool
print_file (
	const char *command,
	int respnum,
	t_openartinfo *artinfo)
{
	FILE *fp;
	t_bool ok = TRUE;
	struct t_header *hdr = &artinfo->hdr;
#	ifdef DONT_HAVE_PIPING
	char cmd[255], file[255];
#	endif /* DONT_HAVE_PIPING */

#	ifdef DONT_HAVE_PIPING
	snprintf(file, sizeof(file) - 1, TIN_PRINTFILE, respnum);
	if ((fp = fopen(file, "w")) == (FILE *) 0)
#	else
	if ((fp = popen (command, "w")) == (FILE *) 0)
#	endif /* DONT_HAVE_PIPING */
	{
		perror_message (_(txt_command_failed), command);
		return FALSE;
	}

	if (tinrc.print_header)
		rewind (artinfo->raw);
	else {
		if (hdr->newsgroups)
			fprintf (fp, "Newsgroups: %s\n", hdr->newsgroups);
		if (arts[respnum].from == arts[respnum].name || arts[respnum].name == (char *) 0)
			fprintf (fp, "From: %s\n", arts[respnum].from);
		else
			fprintf (fp, "From: %s <%s>\n", arts[respnum].name, arts[respnum].from);
		if (hdr->subj)
			fprintf (fp, "Subject: %s\n", hdr->subj);
		if (hdr->date)
			fprintf (fp, "Date: %s\n\n", hdr->date);
		fseek (artinfo->raw, hdr->ext->offset, SEEK_SET);	/* -> start of body */
	}

	ok = copy_fp (artinfo->raw, fp);

#	ifdef DONT_HAVE_PIPING
	fclose(fp);
	strncpy(cmd, command, sizeof(cmd));
	strcat(cmd, " ");
	strncat(cmd, file, sizeof(cmd) - strlen(cmd) - 1);
	cmd[254] = '\0';
	invoke_cmd(cmd);
	unlink(file);
#	else
	fflush (fp);
	pclose (fp);
#	endif /* DONT_HAVE_PIPING */

	return ok;
}
#endif /* !DISABLE_PRINTING */
