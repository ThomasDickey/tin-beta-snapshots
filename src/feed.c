/*
 *  Project   : tin - a Usenet reader
 *  Module    : feed.c
 *  Author    : I. Lea
 *  Created   : 1991-08-31
 *  Updated   : 2000-02-08
 *  Notes     : provides same interface to mail,pipe,print,save & repost commands
 *
 * Copyright (c) 1991-2000 Iain Lea <iain@bricbrac.de>
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
#		include	"tcurses.h"
#	endif /* !TCURSES_H */
#endif /* DEBUG */

#ifndef MENUKEYS_H
#	include	"menukeys.h"
#endif /* !MENUKEYS_H */

#ifndef RFC2045_H
#	include	"rfc2045.h"
#endif /* !RFC2045_H */

char proc_ch;						/* Used for post-processing save queries */

#ifndef DONT_HAVE_PIPING
	FILE *pipe_fp = (FILE *) 0;
#endif /* !DONT_HAVE_PIPING */

t_bool confirm;
/* Nasty global kluge - needed for postprocessing saved arts */
t_bool do_rfc1521_decoding = FALSE;
t_bool is_mailbox = FALSE;
t_bool redraw_screen = FALSE;
t_bool supersede = FALSE;			/* for reposting only */

/*
 * Local prototypes
 */
#ifndef DISABLE_PRINTING
	static t_bool print_file (const char *command, int respnum, t_openartinfo *artinfo);
#endif /* !DISABLE_PRINTING */


/*
 * Return the filename to save to.
 * Sets the global 'is_mailbox' flag.
 * NB: When saving to anything other than a mailbox, the whole path is returned.
 * We default to the global save filename or group specific filename if it exists
 */
static char *
get_save_filename(
	struct t_group *group,
	int function,
	char *filename,
	int filelen)
{
	char save_file[PATH_LEN];

	filename[0] = '\0';

	strcpy (save_file, ((group->attribute->savefile != (char *) 0) ? group->attribute->savefile : tinrc.default_save_file));

	if (function != FEED_AUTOSAVE_TAGGED) {
		sprintf (mesg, _(txt_save_filename), save_file);

		if (!prompt_string (mesg, filename, HIST_SAVE_FILE)) {
			clear_message ();
			return NULL;
		}
		str_trim (filename);
	}

	if (*filename) {
		if (group->attribute->savefile != (char *) 0) {
			free (group->attribute->savefile);
			group->attribute->savefile = my_strdup (filename);
		}
		my_strncpy (tinrc.default_save_file, filename, sizeof (tinrc.default_save_file));
	} else {
		if (*save_file)
			my_strncpy (filename, save_file, LEN);
		else {
			info_message (_(txt_no_filename));
			return NULL;
		}
	}

	if ((filename[0] == '~' || filename[0] == '+') && filename[1] == '\0') {
		info_message (_(txt_no_filename));
		return NULL;
	}

	if (function != FEED_AUTOSAVE_TAGGED) {
		is_mailbox = create_path (filename);
		if (is_mailbox) {
			char my_mailbox[PATH_LEN];

			if ((int) strlen (filename) > 1)
				my_strncpy (my_mailbox, filename+1, sizeof (my_mailbox));
			else
				my_strncpy (my_mailbox, group->name, sizeof (my_mailbox));
			my_strncpy (filename, my_mailbox, filelen);
		} else {		/* ask for post processing type */
			proc_ch = (char) prompt_slk_response(proc_ch_default, "eElLnqsu\033", _(txt_choose_post_process_type));

			if (proc_ch == iKeyQuit || proc_ch == iKeyAbort) { /* exit */
				clear_message ();
				return NULL;
			}
			/* FIXME, ugly hack */
			/* check if rfc1521 decoding is needed */
			do_rfc1521_decoding = (proc_ch == 'n') ? FALSE : TRUE;
		}
	}

	return filename;
}


/*
 * This is the handler that processes a single article for all the
 * various FEED_ functions. 'art' is the index in arts[]
 * Assumes no article is open when we enter -  opens and closes the art being
 * processed. As a performance hack this is not done if 'use_current'.
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
	char *path)
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
			if (!stat_article (arts[art].artnum, path))
				return FALSE;
		} else {
			if (art_open (&arts[art], path, TRUE, &openart) == ART_UNAVAILABLE)
				return FALSE;
		}
	}

	switch (function) {
		case FEED_MAIL:
			if (mail_to_someone (tinrc.default_mail_address, confirm, openartptr) == POSTED_NONE)
				ok = FALSE;
			else			/* POSTED_REDRAW, POSTED_OK */
				redraw_screen = TRUE;

			confirm = bool_not(ok);		/* Only confirm the next one after a failure */
			break;

#ifndef DONT_HAVE_PIPING
		case FEED_PIPE:
			/* TODO - looks odd because screen mode is raw */
			if (max)
				show_progress (_(txt_piping), (*curr)+1, max);
			else
				wait_message (0, "%s %d", _(txt_piping), (*curr)+1);

			rewind (openartptr->raw);
			ok = copy_fp (openartptr->raw, pipe_fp);	/* Check for SIGPIPE on return */

			break;
#endif /* !DONT_HAVE_PIPING */

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			if (max)
				show_progress (_(txt_printing), (*curr)+1, max);
			else
				wait_message (0, "%s %d", _(txt_printing), (*curr)+1);

			ok = print_file (data /*print_command*/, art, openartptr);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			/* This is instant, the actual saving happens later */
			add_to_save_list (art, is_mailbox, data /*filename*/);
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

	/*
	 * try and work out what default the user wants
	 * This is dumb. If you have _any_ selected arts, then it picks 'h'
	 * No it's not, that's exactly whyt you want (in binarie groups)
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
	if ((!(group->attribute->auto_save && arts[respnum].archive) || (group->attribute->auto_save && function != FEED_SAVE) || ch_default == iKeyFeedTag) && function != FEED_AUTOSAVE_TAGGED)
		ch = prompt_slk_response (ch_default, "ahpqtT\033", "%s %s", _(prompt), _(txt_art_thread_regex_tag));
	else
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
			sprintf (mesg, _(txt_mail_art_to), cCOLS-(strlen(_(txt_mail_art_to))+30), tinrc.default_mail_address);
			if (!(prompt_string_default(mesg, tinrc.default_mail_address, _(txt_no_mail_address), HIST_MAIL_ADDRESS)))
				return;
			break;

#ifndef DONT_HAVE_PIPING
		/* Setup pipe - get pipe-to command and open the pipe */
		case FEED_PIPE:
			sprintf (mesg, _(txt_pipe_to_command), cCOLS-(strlen(_(txt_pipe_to_command))+30), tinrc.default_pipe_command);
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
			sprintf (output, "%s %s", group->attribute->printer, REDIRECT_OUTPUT);
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:		/* ask user for filename to save to */
		case FEED_AUTOSAVE_TAGGED:
			/* TODO test for the attrib instead - also in save.c */
			if (!(tinrc.auto_save && arts[respnum].archive)) {
				if (get_save_filename (group, function, output, sizeof(output)) == NULL)
					return;
			}
			wait_message (0, _(txt_saving));
			break;

		case FEED_REPOST:	/* repost article */
			{
#ifndef FORGERY
			char from_name[PATH_LEN];

			get_from_name (from_name, (struct t_group *) 0);

			if (strstr (from_name, arts[respnum].from)) {
#endif /* !FORGERY */
				char option;

				/* repost or supersede ? */
				option = (char) prompt_slk_response (iKeyFeedSupersede, "\033qrs", sized_message(_(txt_supersede_article), arts[respnum].subject));

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
	 * Performance hack - If we feed a single art from the pager then we re-use
	 * the currently open article
	 */
	if (level == PAGE_LEVEL && ch == iKeyFeedArt) {
		saved_curr_line = curr_line;		/* Save where we were in pager */
		use_current = TRUE;
	}

	switch (ch) {
		case iKeyFeedArt:		/* article */
			if (!feed_article (respnum, function, &count, 1, use_current, output, group_path)) {
				if (got_sig_pipe)
					goto got_sig_pipe_while_piping;
				break;
			}

			if (function == FEED_SAVE) {
				if (level == PAGE_LEVEL)
					processed_ok = save_art_to_file (0, FALSE, "", &pgart);
				else {
					t_openartinfo openart;

					if (art_open (&arts[respnum], group_path, do_rfc1521_decoding, &openart) != 0)
						break;
					processed_ok = save_art_to_file (0, FALSE, "", &openart);
					art_close(&openart);
				}
			}

			break;

		case iKeyFeedThd:		/* thread */
			/* Could stat_thread() and pass in 'max' to feed_article() */
			for (i = (int) base[thread_base]; i >= 0; i = arts[i].thread) {
				if (tinrc.process_only_unread && arts[i].status == ART_READ)
					continue;

				/* Ignore errors */
				feed_article (i, function, &count, 0, use_current, output, group_path);
				if (got_sig_pipe)
					goto got_sig_pipe_while_piping;
			}
			if (function == FEED_SAVE) {
				sort_save_list ();
				processed_ok = save_thread_to_file (is_mailbox, group_path);
			}
			break;

		case iKeyFeedTag:		/* tagged articles */
			for (i = 1; i <= num_of_tagged_arts; i++) {
				for (j = 0; j < top_art; j++) {
					if (arts[j].tagged == i) {
						/* Ignore errors */
						feed_article(j, function, &count, num_of_tagged_arts, use_current, output, group_path);
						if (got_sig_pipe)
							goto got_sig_pipe_while_piping;
					}
				}
			}
			if (function == FEED_SAVE || function == FEED_AUTOSAVE_TAGGED)
				processed_ok = save_regex_arts_to_file (is_mailbox, group_path);

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

					if (feed_article(j, function, &count, 0, use_current, output, group_path) && tinrc.mark_saved_read) {
						if (ch == iKeyFeedHot) {
							arts[j].selected = FALSE;
							num_of_selected_arts--;
						}
					} else {
						if (got_sig_pipe)
							goto got_sig_pipe_while_piping;
					}
				}
			}
			if (function == FEED_SAVE)
				processed_ok = save_regex_arts_to_file (is_mailbox, group_path);

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
#if 1
			if (got_sig_pipe)
				perror_message (_(txt_command_failed), tinrc.default_pipe_command);
#endif
			got_sig_pipe = FALSE;
			fflush (pipe_fp);
			(void) pclose (pipe_fp);
			Raw (TRUE);
			InitWin();
			continue_prompt ();
			redraw_screen = TRUE;
			break;
#endif /* !DONT_HAVE_PIPING */

		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			if (proc_ch != 'n' && !is_mailbox && processed_ok)
				ret2 = post_process_files (proc_ch, (function == FEED_SAVE ? FALSE : TRUE));
			free_save_array ();
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
		if (ch == iKeyFeedArt)
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
				info_message (_(txt_articles_mailed), count,
					(count > 1 ? _(txt_article_plural) : _(txt_article_singular)));
			break;

#ifndef DISABLE_PRINTING
		case FEED_PRINT:
			info_message (_(txt_articles_printed), count,
				(count > 1 ? _(txt_article_plural) : _(txt_article_singular)));
			break;
#endif /* !DISABLE_PRINTING */

		case FEED_SAVE:
		case FEED_AUTOSAVE_TAGGED:
			if (ch == iKeyFeedArt)
				info_message (_(txt_saved_arts), count,
					(count > 1 ? _(txt_article_plural) : _(txt_article_singular)));
			break;
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
	sprintf(file, TIN_PRINTFILE, respnum);
	if ((fp = fopen(file,"w")) == (FILE *) 0)
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
	strcat(cmd, file);
	invoke_cmd(cmd);
	unlink(file);
#	else
	fflush (fp);
	pclose (fp);
#	endif /* DONT_HAVE_PIPING */

	return ok;
}
#endif /* !DISABLE_PRINTING */
