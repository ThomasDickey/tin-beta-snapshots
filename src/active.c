/*
 *  Project   : tin - a Usenet reader
 *  Module    : active.c
 *  Author    : I. Lea
 *  Created   : 1992-02-16
 *  Updated   : 2024-10-31
 *  Notes     :
 *
 * Copyright (c) 1992-2024 Iain Lea <iain@bricbrac.de>
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
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
#ifndef STPWATCH_H
#	include "stpwatch.h"
#endif /* !STPWATCH_H */

/*
 * List of allowed separator chars in active file
 * unused in parse_active_line()
 */
#define ACTIVE_SEP	" \n"

#ifdef NNTP_ABLE
#	ifdef DISABLE_PIPELINING
#		define NUM_SIMULTANEOUS_GROUP_COMMAND 1
#	else
#		define NUM_SIMULTANEOUS_GROUP_COMMAND 50
#	endif /* DISABLE_PIPELINING */
#endif /* NNTP_ABLE */

t_bool force_reread_active_file = FALSE;
static time_t active_timestamp;	/* time active file read (local) */


/*
 * Local prototypes
 */
static FILE *open_newgroups_fp(int idx);
static FILE *open_news_active_fp(void);
static int check_for_any_new_groups(void);
static void active_add(struct t_group *ptr, t_artnum count, t_artnum max, t_artnum min, const char *moderated);
static void append_group_line(const char *active_file, const char *group_path, t_artnum art_max, t_artnum art_min, char *base_dir);
static void make_group_list(char *active_file, char *base_dir, char *fixed_base, char *group_path);
static void read_active_file(void);
static void read_newsrc_active_file(void);
static void subscribe_new_group(char *group, const char *autosubscribe, const char *autounsubscribe);
#ifdef NNTP_ABLE
	static t_bool do_read_newsrc_active_file(FILE *fp);
	static t_bool parse_count_line(char *line, t_artnum *max, t_artnum *min, t_artnum *count, char *moderated);
	static void read_active_counts(void);
#else
	static void do_read_newsrc_active_file(FILE *fp);
#endif /* NNTP_ABLE */


t_bool
need_reread_active_file(
	void)
{
	return (force_reread_active_file || (tinrc.reread_active_file_secs != 0 &&
		(int) (time(NULL) - active_timestamp) >= tinrc.reread_active_file_secs));
}


/*
 * Resync active file when reread_active_file_secs have passed or
 * force_reread_active_file is set.
 * Return TRUE if a reread was performed
 */
t_bool
resync_active_file(
	void)
{
	char *old_group = NULL;
	t_bool command_line = FALSE;

	if (!need_reread_active_file())
		return FALSE;

	reread_active_for_posted_arts = FALSE;

	if (selmenu.curr >= 0 && selmenu.max)
		old_group = my_strdup(CURR_GROUP.name);

	write_newsrc();
	read_news_active_file(FALSE);

#ifdef HAVE_MH_MAIL_HANDLING
	read_mail_active_file();
#endif /* HAVE_MH_MAIL_HANDLING */

	if (read_cmd_line_groups())
		command_line = TRUE;

	read_newsrc(newsrc, bool_not(command_line));

	if (command_line)		/* Can't show only unread groups with cmd line groups */
		tinrc.show_only_unread_groups = FALSE;
	else
		toggle_my_groups(old_group);

	FreeIfNeeded(old_group);
	show_selection_page();

	return TRUE;
}


/*
 * Populate a slot in the active[] array
 * TODO: 1) Have a preinitialised default slot and block assign it for speed
 * TODO: 2) Lump count/max/min/moderat into a t_active, big patch but much cleaner throughout tin
 */
static void
active_add(
	struct t_group *ptr,
	t_artnum count,
	t_artnum max,
	t_artnum min,
	const char *moderated)
{
	/* name - pre-initialised when group is made */
	ptr->aliasedto = ((moderated[0] == '=') ? my_strdup(moderated + 1) : NULL);
	ptr->description = NULL;
	/* spool - see below */
	ptr->moderated = moderated[0];
	ptr->count = count;
	ptr->xmax = max;
	ptr->xmin = min;
	/* type - see below */
	ptr->inrange = FALSE;
	ptr->read_during_session = FALSE;
	ptr->art_was_posted = FALSE;
	ptr->subscribed = FALSE;			/* not in my_group[] yet */
	ptr->newgroup = FALSE;
	ptr->bogus = FALSE;
	ptr->next = -1;						/* hash chain */
	ptr->newsrc.xbitmap = (t_bitmap *) 0;
	ptr->attribute = (struct t_attribute *) 0;
	ptr->glob_filter = &glob_filter;
	set_default_bitmap(ptr);

	if (moderated[0] == '/') {
		ptr->type = GROUP_TYPE_SAVE;
		ptr->spooldir = my_strdup(moderated);
	} else {
		ptr->type = GROUP_TYPE_NEWS;
		ptr->spooldir = spooldir;		/* another global - sigh */
	}
}


/*
 * Decide how to handle a bogus groupname.
 * If we process them interactively, create an empty active[] for this
 * group and mark it bogus for display in the group selection page
 * Otherwise, bogus groups are not displayed and are dealt with when newsrc
 * is written.
 */
t_bool
process_bogus(
	const char *name) /* return value is always ignored */
{
	struct t_group *ptr;

	if (read_saved_news || tinrc.strip_bogus != BOGUS_SHOW)
		return FALSE;

	if ((ptr = group_add(name)) == NULL)
		return FALSE;

	active_add(ptr, T_ARTNUM_CONST(0), T_ARTNUM_CONST(1), T_ARTNUM_CONST(0), "n");
	ptr->bogus = TRUE;		/* Mark it bogus */
	/*
	 * Set pointer to default attributes
	 */
	if (ptr->attribute && !ptr->attribute->global)
		free(ptr->attribute);
	ptr->attribute = scopes[0].attribute;

	if (my_group_add(name, FALSE) < 0)
		return TRUE;

	return FALSE;		/* Nothing was printed yet */
}


/*
 * Parse line from news or mail active files
 */
t_bool
parse_active_line(
	char *line,
	t_artnum *max,
	t_artnum *min,
	char *moderated)
{
	char *p = NULL, *q = NULL, *r = NULL;
#ifdef DEBUG
	char *l;
#endif /* DEBUG */
	t_bool lineok = FALSE;

	if (!*line || *line == '#')
		return lineok;

#ifdef DEBUG
	l = my_strdup(line);
#endif /* DEBUG */

	if (strtok(line, ACTIVE_SEP)) {		/* skip group name */
		if ((p = strtok(NULL, ACTIVE_SEP))) {	/* group max count */
			if ((q = strtok(NULL, ACTIVE_SEP))) {	/* group min count */
				if ((r = strtok(NULL, ACTIVE_SEP)))	/* mod status or path to mailgroup */
					lineok = TRUE;
			}
		}
	}

	if (!lineok) {
#ifdef DEBUG
		/* TODO: This also logs broken non NNTP active lines (i.e. mail.active) to NNTP */
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "Active file corrupt - %s", l);

#endif /* DEBUG */
		if (!p || !q) {
#ifdef DEBUG
			free(l);
#endif /* DEBUG */
			return lineok;
		}
	}

	*max = atoartnum(p);
	*min = atoartnum(q);

	if (!lineok) { /* missing moderation flag - seen on usenet.farm */
		strcpy(moderated, "y");	/* guess posting is fine */
		lineok = TRUE;
	} else
		strcpy(moderated, r);

#ifdef DEBUG
	free(l);
#endif /* DEBUG */

	return lineok;
}


#ifdef NNTP_ABLE
/*
 * Parse line from "LIST COUNTS" (RFC 6048 2.2.)
 * group high low count status
 */
static t_bool
parse_count_line(
	char *line,
	t_artnum *max,
	t_artnum *min,
	t_artnum *count,
	char *moderated)
{
	char *p = NULL, *q = NULL, *r = NULL, *s = NULL;
	t_bool lineok = FALSE;

	if (!*line || *line == '#')
		return FALSE;

	if (strtok(line, ACTIVE_SEP)) {		/* skip group name */
		if ((p = strtok(NULL, ACTIVE_SEP))) {	/* group max */
			if ((q = strtok(NULL, ACTIVE_SEP))) {	/* group min */
				if ((r = strtok(NULL, ACTIVE_SEP))) { /* group count */
					s = strtok(NULL, ACTIVE_SEP);	/* mod status or path to mailgroup */
					lineok = TRUE;
				}
			}
		}
	}

	if (!p || !q || !r || !s || !lineok) {
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "unparsable \"LIST COUNTS\" line: \"%s\"", line);
#	endif /* DEBUG */
		return FALSE;
	}

	*max = atoartnum(p);
	*min = atoartnum(q);
	*count = atoartnum(r);
	strcpy(moderated, s);

	return TRUE;
}
#endif /* NNTP_ABLE */


/*
 * Load the active information into active[] by counting the min/max/count
 * for each news group.
 * Parse a line from the .newsrc file
 * Send GROUP command to NNTP server directly to keep window.
 * We can't know the 'moderator' status and always return 'y'
 * But we don't change if the 'moderator' status is already checked by
 * read_active_file()
 * Returns TRUE if NNTP is enabled and authentication is needed
 */
#ifdef NNTP_ABLE
static t_bool
#else
static void
#endif /* NNTP_ABLE */
do_read_newsrc_active_file(
	FILE *fp)
{
	char *ptr;
	char *p;
	char moderated[PATH_LEN];
	int window = 0;
	long processed = 0L;
	t_artnum count = T_ARTNUM_CONST(-1), min = T_ARTNUM_CONST(1), max = T_ARTNUM_CONST(0);
	static char ngname[NNTP_GRPLEN + 1]; /* RFC 3977 3.1 limits group names to 497 octets */
	struct t_group *grpptr;
	t_bool changed;
#ifdef NNTP_ABLE
	t_bool need_auth = FALSE;
	char *ngnames[NUM_SIMULTANEOUS_GROUP_COMMAND] = { NULL };
	char fmt[25];
	char buf[NNTP_STRLEN];
	char line[NNTP_STRLEN];
	int index_i = 0, index_o = 0;
	int respcode, i, j;

	snprintf(fmt, sizeof(fmt), "%%"T_ARTNUM_SFMT" %%"T_ARTNUM_SFMT" %%"T_ARTNUM_SFMT" %%%ds", NNTP_GRPLEN);
#endif /* NNTP_ABLE */

	rewind(fp);

	if (!batch_mode || verbose)
		wait_message(0, _(txt_reading_news_newsrc_file));

	while ((ptr = tin_fgets(fp, FALSE)) != NULL || window != 0) {
		if (ptr) {
			p = strpbrk(ptr, ":!");

			if (!p || *p != SUBSCRIBED)	/* Invalid line or unsubscribed */
				continue;
			*p = '\0';			/* Now ptr is the group name */

			/*
			 * 128 should be enough for a groupname, >256 and we overflow buffers
			 * later on
			 * TODO: check RFCs for possible max. size
			 */
			my_strncpy(ngname, ptr, 128);
			ptr = ngname;
		}

		if (read_news_via_nntp && !read_saved_news) { /* this should be limited to GROUP_TYPE_NEWS, but ... */
#ifdef NNTP_ABLE
			if (window < NUM_SIMULTANEOUS_GROUP_COMMAND && ptr && (!list_active || (newsrc_active && list_active && group_find(ptr, FALSE)))) {
				ngnames[index_i] = my_strdup(ptr);
				snprintf(buf, sizeof(buf), "GROUP %s", ngnames[index_i]);
#	ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose > 1)
					debug_print_file("NNTP", "read_newsrc_active_file() %s", buf);
#	endif /* DEBUG */
				put_server(buf, FALSE);
				index_i = (index_i + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
				window++;
			}
			if (window == NUM_SIMULTANEOUS_GROUP_COMMAND || ptr == NULL) {
				respcode = get_only_respcode(line, sizeof(line));

				if (reconnected_in_last_get_server) {
					/*
					 * If tin reconnected, last output is resended to server.
					 * So received data is for ngnames[last window_i].
					 * We resend all buffered command except for last window_i.
					 * And rotate buffer to use data received.
					 */
					j = index_o;

					for (i = 0; i < window - 1; i++) {
						snprintf(buf, sizeof(buf), "GROUP %s", ngnames[j]);
#	ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "read_newsrc_active_file() %s", buf);
#	endif /* DEBUG */
						put_server(buf, FALSE);
						j = (j + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
					}
					if (--index_o < 0)
						index_o = NUM_SIMULTANEOUS_GROUP_COMMAND - 1;
					if (--index_i < 0)
						index_i = NUM_SIMULTANEOUS_GROUP_COMMAND - 1;
					if (index_i != index_o)
						ngnames[index_o] = ngnames[index_i];
				}

				switch (respcode) {
					case OK_GROUP:
						{
							if (sscanf(line, fmt, &count, &min, &max, ngname) != 4) {
#	ifdef DEBUG
								if ((debug & DEBUG_NNTP) && verbose > 1) {
									debug_print_file("NNTP", "Invalid response to \"GROUP %s\": \"%s\"", ngnames[index_o], line);
									if (strcmp(ngname, ngnames[index_o]) != 0)
										debug_print_file("NNTP", "Groupname mismatch in response to \"GROUP %s\": \"%s\"", ngnames[index_o], line);
								}
#	endif /* DEBUG */
							}
							ptr = ngname;
							free(ngnames[index_o]);
							index_o = (index_o + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
							window--;
							break;
						}

					case ERR_NOAUTH:
					case NEED_AUTHINFO:
						need_auth = TRUE; /* delay auth till end of loop */
						/* keep lint quiet: */
						/* FALLTHROUGH */

					case ERR_NOGROUP:
						free(ngnames[index_o]);
						index_o = (index_o + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
						window--;
						continue;

					case ERR_ACCESS:
						for (index_i = 0; index_i < NUM_SIMULTANEOUS_GROUP_COMMAND - 1; index_i++) {
							FreeIfNeeded(ngnames[index_i]);
						}
						tin_done(NNTP_ERROR_EXIT, "%s", line);
						/* keep lint quiet: */
						/* FALLTHROUGH */

					default:
#	ifdef DEBUG
						if ((debug & DEBUG_NNTP) && verbose > 1)
							debug_print_file("NNTP", "NOT_OK %s", line);
#	endif /* DEBUG */
						free(ngnames[index_o]);
						index_o = (index_o + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
						window--;
						continue;
				}
			} else
				continue;
#endif /* NNTP_ABLE */
		} else {
			if (group_get_art_info(spooldir, ptr, GROUP_TYPE_NEWS, &count, &max, &min))
				continue;
		}

		strcpy(moderated, "y");

		if (++processed % 5 == 0)
			spin_cursor();

		/*
		 * Load group into group hash table
		 * NULL means group already present, so we just fixup the counters
		 * This call may implicitly ++num_active
		 */
		if ((grpptr = group_add(ptr)) == NULL) {
			changed = FALSE;

			if ((grpptr = group_find(ptr, FALSE)) == NULL)
				continue;

			if (max > grpptr->xmax) {
				grpptr->xmax = max;
				changed = TRUE;
			}
			if (min > grpptr->xmin) {
				grpptr->xmin = min;
				changed = TRUE;
			}
			if (changed) {
				grpptr->count = count;
				expand_bitmap(grpptr, 0); /* TODO: expand_bitmap(grpptr,grpptr->xmin) should be enough */
			}
			continue;
		}

		/*
		 * Load the new group in active[]
		 */
		active_add(grpptr, count, max, min, moderated);
	}
#ifdef NNTP_ABLE
	return need_auth;
#endif /* NNTP_ABLE */
}


/*
 * Wrapper for do_read_newsrc_active_file() to handle
 * missing authentication
 */
static void
read_newsrc_active_file(
	void)
{
	FILE *fp;

	/*
	 * return immediately if no .newsrc can be found or .newsrc is empty
	 * when function asked to use .newsrc
	 */
	if ((fp = tin_fopen(newsrc, "r")) == NULL)
		return;

#ifndef NNTP_ABLE
	do_read_newsrc_active_file(fp);
#else
	if (do_read_newsrc_active_file(fp)) {	/* delayed auth */
		if (!authenticate(nntp_server, userid, FALSE)) {
			fclose(fp);
			tin_done(EXIT_FAILURE, _(txt_auth_failed), ERR_ACCESS);
		}
#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
		set_maxartnum(FALSE);
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
		if (do_read_newsrc_active_file(fp)) {
			fclose(fp);
			tin_done(EXIT_FAILURE, _(txt_auth_failed), ERR_ACCESS);
		}
	}
#endif /* !NNTP_ABLE */

	fclose(fp);

	/*
	 * Exit if active file wasn't read correctly or is empty
	 */
	if (tin_errno || !num_active) {
		if (newsrc_active && !num_active)
			tin_done(EXIT_FAILURE, _(txt_error_server_has_no_listed_groups), newsrc);
		else
			tin_done(EXIT_FAILURE, _(txt_active_file_is_empty), (read_news_via_nntp ? (read_saved_news ? news_active_file : _(txt_servers_active)) : news_active_file));
	}

	if (!batch_mode || verbose)
		my_fputc('\n', stdout);
}


/*
 * Open the news active file locally or send the LIST command
 */
static FILE *
open_news_active_fp(
	void)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news)
		return (nntp_command("LIST", OK_GROUPS, NULL, 0));
#endif /* NNTP_ABLE */
	return (fopen(news_active_file, "r"));
}


/*
 * Load the active file into active[]
 */
static void
read_active_file(
	void)
{
	FILE *fp;
	char *ptr;
	char moderated[PATH_LEN];
	long processed = 0L;
	struct t_group *grpptr;
	t_artnum count = T_ARTNUM_CONST(-1), min = T_ARTNUM_CONST(1), max = T_ARTNUM_CONST(0);

	if (!batch_mode || verbose)
		wait_message(0, _(txt_reading_news_active_file));

	if ((fp = open_news_active_fp()) == NULL) {
		if ((cmd_line && !batch_mode) || verbose)
			my_fputc('\n', stderr);

#ifdef NNTP_ABLE
		if (read_news_via_nntp)
			tin_done(EXIT_FAILURE, _(txt_cannot_retrieve), ACTIVE_FILE);
#	ifndef NNTP_ONLY
		else {
			perror_message("%s", news_active_file);
			tin_done(EXIT_FAILURE, _(txt_cannot_open_active_file), news_active_file, tin_progname);
		}
#	endif /* !NNTP_ONLY */
#else
		perror_message("%s", news_active_file);
		tin_done(EXIT_FAILURE, _(txt_cannot_open), news_active_file);
#endif /* NNTP_ABLE */
	}

	while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
#if defined(DEBUG) && defined(NNTP_ABLE)
		if ((debug & DEBUG_NNTP) && verbose) /* long multiline response */
			debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#endif /* DEBUG && NNTP_ABLE */

		if (++processed % MODULO_COUNT_NUM == 0)
			spin_cursor();

		if (!parse_active_line(ptr, &max, &min, moderated))
			continue;

		/*
		 * Load group into group hash table
		 * NULL means group already present, so we just fixup the counters
		 * This call may implicitly ++num_active
		 */
		if ((grpptr = group_add(ptr)) == NULL) {
			if ((grpptr = group_find(ptr, FALSE)) == NULL)
				continue;

			if (max > grpptr->xmax) {
				grpptr->xmax = max;
				grpptr->count = count;
			}

			if (min > grpptr->xmin) {
				grpptr->xmin = min;
				grpptr->count = count;
			}

			continue;
		}

		/*
		 * Load the new group in active[]
		 */
		active_add(grpptr, count, max, min, moderated);
	}
#	if defined(DEBUG) && defined(NNTP_ABLE)
	if ((debug & DEBUG_NNTP) && !verbose)
		debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG && NNTP_ABLE */

	TIN_FCLOSE(fp);

	/*
	 * Exit if active file wasn't read correctly or is empty
	 */
	if (tin_errno || !num_active)
		tin_done(EXIT_FAILURE, _(txt_active_file_is_empty), (read_news_via_nntp ? (read_saved_news ? news_active_file : _(txt_servers_active)) : news_active_file));

	if (!batch_mode || verbose)
		my_fputc('\n', stdout);
}


#ifdef NNTP_ABLE
/*
 * Load the active file into active[] via LIST COUNTS
 */
static void
read_active_counts(
	void)
{
	FILE *fp;
	char *ptr;
	char moderated[PATH_LEN];
	long processed = 0L;
	struct t_group *grpptr;
	t_artnum count = T_ARTNUM_CONST(-1), min = T_ARTNUM_CONST(1), max = T_ARTNUM_CONST(0);

	if (!batch_mode || verbose)
		wait_message(0, _(txt_reading_news_active_file));

	if ((fp = nntp_command("LIST COUNTS", OK_GROUPS, NULL, 0)) == NULL) {
		if (cmd_line && !batch_mode)
			my_fputc('\n', stderr);

		tin_done(EXIT_FAILURE, _(txt_cannot_retrieve), ACTIVE_FILE);
	}

	while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose) /* long multiline response */
			debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */

		if (++processed % MODULO_COUNT_NUM == 0)
			spin_cursor();

		if (!parse_count_line(ptr, &max, &min, &count, moderated))
			continue;

		/*
		 * Load group into group hash table
		 * NULL means group already present, so we just fixup the counters
		 * This call may implicitly ++num_active
		 */
		if ((grpptr = group_add(ptr)) == NULL) {
			if ((grpptr = group_find(ptr, FALSE)) == NULL)
				continue;

			if (max > grpptr->xmax) {
				grpptr->xmax = max;
				grpptr->count = count;
			}

			if (min > grpptr->xmin) {
				grpptr->xmin = min;
				grpptr->count = count;
			}

			continue;
		}

		/*
		 * Load the new group in active[]
		 */
		active_add(grpptr, count, max, min, moderated);
	}
#	ifdef DEBUG
	/* log end of multiline response to get timing data */
	if ((debug & DEBUG_NNTP) && !verbose)
		debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG */

	/*
	 * Exit if active file wasn't read correctly or is empty
	 */
	if (tin_errno || !num_active)
		tin_done(EXIT_FAILURE, _(txt_active_file_is_empty), _(txt_servers_active));

	if (!batch_mode || verbose)
		my_fputc('\n', stdout);
}
#endif /* NNTP_ABLE */


/*
 * Load the active file into active[]
 * Check and preload any new newgroups into my_group[]
 */
int
read_news_active_file(
	t_bool check_any_unread)
{
	FILE *fp;
	int newgrps = 0;
	t_bool do_group_cmds = !nntp_caps.list_counts;
#if defined(NNTP_ABLE) && !defined(DISABLE_PIPELINING)
	t_bool did_list_cmd = FALSE;
#endif /* NNTP_ABLE && !DISABLE_PIPELINING */

	/*
	 * Ignore -n if no .newsrc can be found or .newsrc is empty
	 */
	if (newsrc_active) {
		if ((fp = tin_fopen(newsrc, "r")) == NULL) {
			list_active = TRUE;
			newsrc_active = FALSE;
		} else
			fclose(fp);
	}

	/* Read an active file if it is allowed */
	if (list_active) {
#ifdef NNTP_ABLE
#	ifndef DISABLE_PIPELINING
		did_list_cmd = TRUE;
#	endif /* !DISABLE_PIPELINING */
		if (read_news_via_nntp && nntp_caps.list_counts)
			read_active_counts();
		else
#endif /* NNTP_ABLE */
			read_active_file();
	}

	/* Read .newsrc and check each group */
	if (newsrc_active) {
#ifdef NNTP_ABLE
#	ifndef DISABLE_PIPELINING
		/*
		 * prefer LIST COUNTS, otherwise use LIST ACTIVE (-l) or GROUP (-n)
		 * or both (-ln); LIST COUNTS/ACTIVE grplist is used up to
		 * PIPELINE_LIMIT groups in newsrc
		 */
		if (read_news_via_nntp && (list_active || nntp_caps.list_counts) && !did_list_cmd) {
			char buff[NNTP_STRLEN];
			char *ptr, *q;
			char moderated[PATH_LEN];
			int r = 0, j = 0;
			int i;
			struct t_group *grpptr;
			t_artnum count = T_ARTNUM_CONST(-1), min = T_ARTNUM_CONST(1), max = T_ARTNUM_CONST(0);
			t_bool need_auth = FALSE;

			*buff = '\0';
			/* we can't use for_each_group(i) yet, so we have to parse the newsrc */
			if ((fp = tin_fopen(newsrc, "r")) != NULL) {
				while (tin_fgets(fp, FALSE) != NULL)
					j++;
				rewind(fp);
				if (j < PIPELINE_LIMIT) {
					while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
						if (!(q = strpbrk(ptr, ":!")))
							continue;
						*q = '\0';
						if (nntp_caps.type == CAPABILITIES && (nntp_caps.list_active || nntp_caps.list_counts)) {
							/* LIST ACTIVE or LIST COUNTS takes wildmats */
							if (*buff && ((strlen(buff) + strlen(ptr)) < (NNTP_GRPLEN - 1))) { /* append group name */
								snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), ",%s", ptr);
							} else {
								if (*buff) {
									put_server(buff, FALSE);
									r++;
								}
								snprintf(buff, sizeof(buff), "LIST %s %s", nntp_caps.list_counts ? "COUNTS" : "ACTIVE", ptr);
							}
							continue;
						} else
							snprintf(buff, sizeof(buff), "LIST ACTIVE %s", ptr);
						put_server(buff, FALSE);
						r++;
						*buff = '\0';
					}
					if (*buff) {
						put_server(buff, FALSE);
						r++;
					}
				} else
					do_group_cmds = TRUE;

				fclose(fp);

				if (j < PIPELINE_LIMIT) {
					for (i = 0; i < r && !did_reconnect; i++) {
						if ((j = get_only_respcode(buff, sizeof(buff))) != OK_GROUPS) {
							/* TODO: add 483 (RFC 3977) code */
							if (j == ERR_NOAUTH || j == NEED_AUTHINFO)
								need_auth = TRUE;
#		if 0 /* do we need something like this? */
							if (j == ERR_CMDSYN)
								list_active = TRUE;
#		endif /* 0 */
							continue;
						} else {
							while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#		ifdef DEBUG
								if ((debug & DEBUG_NNTP) && verbose) /* long multiline response */
									debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#		endif /* DEBUG */
								if (nntp_caps.type == CAPABILITIES && nntp_caps.list_counts) {
									if (!parse_count_line(ptr, &max, &min, &count, moderated))
										continue;
								} else {
									if (!parse_active_line(ptr, &max, &min, moderated))
										continue;
								}

								if ((grpptr = group_add(ptr)) == NULL) {
									if ((grpptr = group_find(ptr, FALSE)) == NULL)
										continue;

									if (max > grpptr->xmax) {
										grpptr->xmax = max;
										grpptr->count = count;
									}
									if (min > grpptr->xmin) {
										grpptr->xmin = min;
										grpptr->count = count;
									}
									continue;
								}
								active_add(grpptr, count, max, min, moderated);
							}
#		ifdef DEBUG
							/* log end of multiline response to get timing data */
							if ((debug & DEBUG_NNTP) && !verbose)
								debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#		endif /* DEBUG */
						}
					}
					if (need_auth) { /* retry after auth is overkill here, so just auth */
						if (!authenticate(nntp_server, userid, FALSE))
							tin_done(EXIT_FAILURE, _(txt_auth_failed), nntp_caps.type == CAPABILITIES ? ERR_AUTHFAIL : ERR_ACCESS);
						/* no set_maxartnum() here after auth as we're pipelining ... */
					}
				}
				did_reconnect = FALSE;
			}
		}
#	endif /* !DISABLE_PIPELINING */
#endif /* NNTP_ABLE */
		if (!nntp_caps.list_counts || do_group_cmds)
			read_newsrc_active_file();
	}

	(void) time(&active_timestamp);
	force_reread_active_file = FALSE;

	/*
	 * check_for_any_new_groups() also does $AUTOSUBSCRIBE
	 */
	if (check_for_new_newsgroups)
		newgrps = check_for_any_new_groups();

	/*
	 * finally we have a list of all groups and can set the attributes
	 * if required
	 */
	if (!check_any_unread) {
	BegStopWatch();
	assign_attributes_to_groups();
	EndStopWatch("assign_attributes_to_groups()");
	}

	return newgrps;
}


/*
 * Open the active.times file locally or send the NEWGROUPS command
 * "NEWGROUPS yymmdd hhmmss" (or "NEWGROUPS yyymmdd hhmmss GMT" if
 * server knows CAPABILITIES (=RFC 3977)).
 */
static FILE *
open_newgroups_fp(
	int idx)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news) {
		char line[NNTP_STRLEN];
		struct tm *ngtm;

		if (idx >= 0) {
			if (nntp_caps.type == CAPABILITIES)
				ngtm = gmtime(&newnews[idx].time);
			else
				ngtm = localtime(&newnews[idx].time);

			if (ngtm == NULL)
				return (FILE *) 0;

			if (nntp_caps.type == CAPABILITIES) {
			/*
			 * not checking for caps_type == CAPABILITIES && reader as some
			 * servers do not support it even if advertising READER so we must
			 * handle errors anyway and just issue the cmd. but we use
			 * GMT and 4 didigit year on RFC 3977 servers. 4 digit year
			 * was introduced before Y2K ...
			 */
				snprintf(line, sizeof(line), "NEWGROUPS %04d%02d%02d %02d%02d%02d GMT",
				ngtm->tm_year + 1900, ngtm->tm_mon + 1, ngtm->tm_mday,
				ngtm->tm_hour, ngtm->tm_min, ngtm->tm_sec);
			} else {
				/*
				 * RFC 3977 states that we SHOULD use 4 digit year but some servers
				 * still do not support it.
				 */
				snprintf(line, sizeof(line), "NEWGROUPS %02d%02d%02d %02d%02d%02d",
				ngtm->tm_year % 100, ngtm->tm_mon + 1, ngtm->tm_mday,
				ngtm->tm_hour, ngtm->tm_min, ngtm->tm_sec);
			}
		} else
			return (FILE *) 0;

		return (nntp_command(line, OK_NEWGROUPS, NULL, 0));
	}
#else
	/* silence compiler warning (unused parameter) */
	(void) idx;
#endif /* NNTP_ABLE */

	return (fopen(active_times_file, "r"));
}


/*
 * Check for any newly created newsgroups.
 *
 * If reading news locally check the NEWSLIBDIR/active.times file.
 * Format:   Groupname Seconds Creator
 *
 * If reading news via NNTP issue a NEWGROUPS command.
 * Format:   (as active file) Groupname Maxart Minart moderated
 */
static int
check_for_any_new_groups(
	void)
{
	FILE *fp;
	const char *autosubscribe, *autounsubscribe;
	char *ptr, *line, buf[NNTP_STRLEN];
	char old_newnews_host[PATH_LEN];
	int newnews_index;
	int newgrps = 0;
	time_t old_newnews_time;
	time_t new_newnews_time;

	if (!batch_mode /* || verbose */)
		wait_message(0, _(txt_checking_new_groups));

	(void) time(&new_newnews_time);

	/*
	 * find out if we have read news from here before otherwise -1
	 */
	if ((newnews_index = find_newnews_index(nntp_server)) >= 0) {
		STRCPY(old_newnews_host, newnews[newnews_index].host);
		old_newnews_time = newnews[newnews_index].time;
	} else {
		STRCPY(old_newnews_host, "UNKNOWN");
		old_newnews_time = (time_t) 0;
	}

#ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "Newnews old=[%lu]  new=[%lu]", (unsigned long int) old_newnews_time, (unsigned long int) new_newnews_time);
#endif /* DEBUG */

	if ((fp = open_newgroups_fp(newnews_index)) != NULL) {
		/*
		 * Need these later. They list user-defined groups to be
		 * automatically subscribed or unsubscribed.
		 */
		autosubscribe = getenv("AUTOSUBSCRIBE");
		autounsubscribe = getenv("AUTOUNSUBSCRIBE");

		while ((line = tin_fgets(fp, FALSE)) != NULL) {
			/*
			 * Split the group name off and subscribe. If we're reading local,
			 * we must check the creation date manually
			 */
			if ((ptr = strchr(line, ' ')) != NULL) {
				if (!read_news_via_nntp && ((time_t) atol(ptr) < old_newnews_time || old_newnews_time == (time_t) 0))
					continue;

				*ptr = '\0';
			}
			subscribe_new_group(line, autosubscribe, autounsubscribe);
			newgrps++;
		}
		TIN_FCLOSE(fp);

		if (tin_errno)
			return 0;				/* Don't update the time if we quit */
	}

	/*
	 * Update (if already existing) or create (if new) the in-memory
	 * 'last time newgroups checked' slot for this server. It will be written
	 * out as part of tinrc.
	 */
	if (newnews_index >= 0)
		newnews[newnews_index].time = new_newnews_time;
	else {
		snprintf(buf, sizeof(buf), "%s %lu", nntp_server, (unsigned long int) new_newnews_time);
		load_newnews_info(buf);
	}

	if (!batch_mode)
		my_fputc('\n', stdout);

	return newgrps;
}


/*
 * Subscribe to a new news group:
 * Handle the AUTOSUBSCRIBE/AUTOUNSUBSCRIBE env vars
 * They hold a wildcard list of groups that should be automatically
 * (un)subscribed when a new group is found
 * If a group is autounsubscribed, completely ignore it
 * If a group is autosubscribed, subscribe to it
 * Otherwise, mark it as New for inclusion in selection screen
 */
static void
subscribe_new_group(
	char *group,
	const char *autosubscribe,
	const char *autounsubscribe)
{
	int idx;
	struct t_group *ptr;

	/*
	 * If we explicitly don't auto subscribe to this group, then don't bother going on
	 */
	if ((autounsubscribe != NULL) && match_group_list(group, autounsubscribe))
		return;

	/*
	 * Try to add the group to our selection list. If this fails, we're
	 * probably using -n, so we fake an entry with no counts. The count will
	 * be properly updated when we enter the group. Otherwise there is some
	 * mismatch in the active.times data and we ignore the newgroup.
	 */
	if ((idx = my_group_add(group, FALSE)) < 0) {
		if (list_active) {
/*			my_fprintf(stderr, "subscribe_new_group: %s not in active[] && list_active\n", group); */
			return;
		}

		if ((ptr = group_add(group)) != NULL)
			active_add(ptr, T_ARTNUM_CONST(0), T_ARTNUM_CONST(1), T_ARTNUM_CONST(0), "y");

		if ((idx = my_group_add(group, FALSE)) < 0)
			return;
	}

	if (!no_write && (autosubscribe != NULL) && match_group_list(group, autosubscribe)) {
		if (!batch_mode || verbose)
			my_printf(_(txt_autosubscribed), group);

		/*
		 * as subscribe_new_group() is called from check_for_any_new_groups()
		 * which has pending data on the socket if reading via NNTP we are not
		 * allowed to issue any NNTP commands yet
		 */
		subscribe(&active[my_group[idx]], SUBSCRIBED, bool_not(read_news_via_nntp));
		/*
		 * Bad kluge to stop group later appearing in New newsgroups. This
		 * effectively loses the group, and it has now been subscribed to and
		 * so will be reread later by read_newsrc()
		 */
		selmenu.max--;
	} else
		active[my_group[idx]].newgroup = TRUE;
}


/*
 * See if group is a member of group_list, returning a boolean.
 * group_list is a comma separated list of newsgroups, ! implies NOT
 * The same degree of wildcarding as used elsewhere in tin is allowed
 */
t_bool
match_group_list(
	const char *group,
	const char *group_list)
{
	char *separator;
	char pattern[HEADER_LEN];
	char ngname[NNTP_GRPLEN + 1]; /* RFC 3977 3.1 limits group names to 497 octets */
	size_t group_len, list_len;
	t_bool negate, accept = FALSE;

	list_len = strlen(group_list);
	/*
	 * walk through comma-separated entries in list
	 */
	while (list_len != 0) {
		/*
		 * find end/length of this entry
		 */
		separator = strchr(group_list, ',');
		group_len = MIN(((separator == NULL) ? list_len : (size_t) (separator - group_list)), sizeof(pattern) - 1);

		if ((negate = (*group_list == '!'))) {
			/*
			 * a '!' before the pattern inverts sense of match
			 */
			group_list++;
			group_len--;
			list_len--;
		}
		/*
		 * copy out the entry and terminate it properly
		 */
		my_strncpy(pattern, group_list, group_len);
		my_strncpy(ngname, group, sizeof(ngname) - 1);
		str_lwr(pattern);
		str_lwr(ngname);

		/*
		 * "case-insensitive" (str_lwr(); avoid malloc()/free() in) wildcard match
		 */
		if (GROUP_MATCH(group, pattern, FALSE))
			accept = bool_not(negate);	/* matched! */

		/*
		 * now examine next entry if any
		 */
		if (group_list[group_len] != '\0')
			group_len++;	/* skip the separator */

		group_list += group_len;
		list_len -= group_len;
	}
	return accept;
}


/*
 * Add or update an entry to the in-memory newnews[] array (The times newgroups
 * were last checked for a particular news server)
 * If this is first time we've been called, zero out the array.
 */
void
load_newnews_info(
	char *info)
{
	char *ptr;
	int i;
	time_t new_time;

	/*
	 * initialize newnews[] if no entries
	 */
	if (!num_newnews) {
		for (i = 0; i < max_newnews; i++) {
			newnews[i].host = NULL;
			newnews[i].time = (time_t) 0;
		}
	}

	/*
	 * Split 'info' into hostname and time
	 */
	if ((ptr = strchr(info, ' ')) == NULL)
		return;

	*ptr++ = '\0';
	new_time = (time_t) atol(ptr);

	/*
	 * If this is a new host entry, set it up
	 */
	if ((i = find_newnews_index(info)) == -1) {
		i = num_newnews++;

		if (i >= max_newnews)
			expand_newnews();
		newnews[i].host = my_strdup(info);
	}

	newnews[i].time = new_time;

#ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "ACTIVE host=[%s] time=[%lu]", newnews[i].host, (unsigned long int) newnews[i].time);
#endif /* DEBUG */
}


/*
 * Return the index of cur_newnews_host in newnews[] or -1 if not found
 */
int
find_newnews_index(
	const char *cur_newnews_host)
{
	int i;

	for (i = 0; i < num_newnews; i++) {
		if (STRCMPEQ(cur_newnews_host, newnews[i].host))
			return i;
	}

	return -1;
}


/*
 * Get a single status char from the moderated field. Used on selection screen
 * and in header of group screen
 */
char
group_flag(
	char ch)
{
	switch (ch) {
		case 'm':
			return 'M';

		case 'x':
		case 'n':
		case 'j':
			return 'X';

		case '=':
			return '=';

		default:
			return ' ';
	}
}


void
create_save_active_file(
	void)
{
	char *fb;
	char group_path[PATH_LEN];
	char local_save_active_file[PATH_LEN];

	joinpath(local_save_active_file, sizeof(local_save_active_file), rcdir, ACTIVE_SAVE_FILE);

	if (no_write && file_size(local_save_active_file) != -1L)
		return;

	if (strfpath(cmdline.savedir ? cmdline.savedir : tinrc.savedir, group_path, sizeof(group_path), NULL, FALSE)) {
		wait_message(0, _(txt_creating_active));
		print_active_head(local_save_active_file);

		while (strlen(group_path) && group_path[strlen(group_path) - 1] == '/')
			group_path[strlen(group_path) - 1] = '\0';

		fb = my_strdup(group_path);
		make_group_list(local_save_active_file, cmdline.savedir ? cmdline.savedir : tinrc.savedir, fb, group_path);
		free(fb);
	}
}


static void
make_group_list(
	char *active_file,
	char *base_dir,
	char *fixed_base,
	char *group_path)
{
	DIR *dir;
	DIR_BUF *direntry;
	char *ptr;
	char filename[PATH_LEN];
	char path[PATH_LEN];
	t_artnum art_max;
	t_artnum art_min;
	struct stat stat_info;
	t_bool is_dir;

	if ((dir = opendir(group_path)) != NULL) {
		is_dir = FALSE;
		while ((direntry = readdir(dir)) != NULL) {
			STRCPY(filename, direntry->d_name);
			joinpath(path, sizeof(path), group_path, filename);
			if (!(filename[0] == '.' && filename[1] == '\0') &&
				!(filename[0] == '.' && filename[1] == '.' && filename[2] == '\0')) {
				if (stat(path, &stat_info) != -1) {
					if (S_ISDIR(stat_info.st_mode))
						is_dir = TRUE;
				}
			}
			if (is_dir) {
				is_dir = FALSE;
				strcpy(group_path, path);

				make_group_list(active_file, base_dir, fixed_base, group_path);
				find_art_max_min(group_path, &art_max, &art_min);
				append_group_line(active_file, group_path + strlen(fixed_base) + 1, art_max, art_min, fixed_base);
				if ((ptr = strrchr(group_path, '/')) != NULL)
					*ptr = '\0';
			}
		}
		CLOSEDIR(dir);
	}
}


static void
append_group_line(
	const char *active_file,
	const char *group_path,
	t_artnum art_max,
	t_artnum art_min,
	char *base_dir)
{
	FILE *fp;
	char *file_tmp;

	if (art_max == 0 && art_min == 1)
		return;

	if ((file_tmp = get_tmpfilename(active_file)) == NULL)
		return;

	if (!backup_file(active_file, file_tmp)) {
		free(file_tmp);
		return;
	}

	if ((fp = fopen(active_file, "a+")) != NULL) {
		char *ptr;
		char *group_name;
		int err;

		ptr = group_name = my_strdup(group_path);
		ptr++;
		while ((ptr = strchr(ptr, '/')) != NULL)
			*ptr = '.';

		wait_message(0, "Appending=[%s %"T_ARTNUM_PFMT" %"T_ARTNUM_PFMT" %s]\n", group_name, art_max, art_min, base_dir);
		print_group_line(fp, group_name, art_max, art_min, base_dir);
		if ((err = ferror(fp)) || fclose(fp)) { /* TODO: issue warning? */
			if (err) {
				clearerr(fp);
				fclose(fp);
			}
			rename_file(file_tmp, active_file);
		}
		free(group_name);
	}
	unlink(file_tmp);
	free(file_tmp);
}
