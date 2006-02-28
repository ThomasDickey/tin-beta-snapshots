/*
 *  Project   : tin - a Usenet reader
 *  Module    : active.c
 *  Author    : I. Lea
 *  Created   : 1992-02-16
 *  Updated   : 2004-06-30
 *  Notes     :
 *
 * Copyright (c) 1992-2006 Iain Lea <iain@bricbrac.de>
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

/*
 * List of allowed seperator chars in active file
 * unsed in parse_active_line()
 */
#define ACTIVE_SEP	" \n"

/*
 * if you are using C-News nntpd set NUM_SIMULTANEOUS_GROUP_COMMAND to 1
 */
#ifdef NNTP_ABLE
#	define NUM_SIMULTANEOUS_GROUP_COMMAND 50
#endif /* NNTP_ABLE */

t_bool force_reread_active_file = FALSE;
static time_t active_timestamp;	/* time active file read (local) */


/*
 * Local prototypes
 */
static FILE *open_newgroups_fp(int idx);
static FILE *open_news_active_fp(void);
static void active_add(struct t_group *ptr, long count, long max, long min, const char *moderated);
static void append_group_line(char *active_file, char *group_path, long art_max, long art_min, char *base_dir);
static void check_for_any_new_groups(void);
static void make_group_list(char *active_file, char *base_dir, char *group_path);
static void make_group_name(char *base_dir, char *group_name, char *group_path);
static void read_active_file(void);
static void read_newsrc_active_file(void);
static void subscribe_new_group(char *group, char *autosubscribe, char *autounsubscribe);


t_bool
need_reread_active_file(
	void)
{
	return (force_reread_active_file || (tinrc.reread_active_file_secs != 0 &&
		(int) (time(NULL) - active_timestamp) >= tinrc.reread_active_file_secs));
}


/*
 * Resync active file when reread_active_file_secs have passed or
 * force_reread_actve_file is set.
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
	read_news_active_file();

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

	FreeAndNull(old_group);
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
	long count,
	long max,
	long min,
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
		ptr->spooldir = my_strdup(moderated); /* TODO: Unix'ism, other OSs need transformation */
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
	char *name) /* return value is always ignored */
{
	struct t_group *ptr;

	if (read_saved_news || tinrc.strip_bogus != BOGUS_SHOW)
		return FALSE;

	if ((ptr = group_add(name)) == NULL)
		return FALSE;

	active_add(ptr, 0L, 1L, 0L, "n");
	ptr->bogus = TRUE;		/* Mark it bogus */

	if (my_group_add(name) < 0)
		return TRUE;

	return FALSE;		/* Nothing was printed yet */
}


/*
 * Parse line from news or mail active files
 */
t_bool
parse_active_line(
	char *line,
	long *max,
	long *min,
	char *moderated)
{
	char *p, *q, *r;

	if (line[0] == '#' || line[0] == '\0')
		return FALSE;

	(void) strtok(line, ACTIVE_SEP);		/* skip group name */
	p = strtok(NULL, ACTIVE_SEP);	/* group max count */
	q = strtok(NULL, ACTIVE_SEP);	/* group min count */
	r = strtok(NULL, ACTIVE_SEP);	/* mod status or path to mailgroup */

	if (!p || !q || !r) {
		error_message(_(txt_bad_active_file), line);
		return FALSE;
	}

	*max = atol(p);
	*min = atol(q);
	strcpy(moderated, r);

	return TRUE;
}


/*
 * Load the active information into active[] by counting the min/max/count
 * for each news group.
 * Parse a line from the .newsrc file
 * Send GROUP command to NNTP server directly to keep window.
 * We can't know the 'moderator' status and always return 'y'
 * But we don't change if the 'moderator' status is already checked by
 * read_active_file()
 */
static void
read_newsrc_active_file(
	void)
{
	FILE *fp;
	char *ptr;
	char *p;
	char moderated[PATH_LEN];
	int window = 0;
	long count = -1L, min = 1L, max = 0L;
	long processed = 0L;
	static char ngname[NNTP_STRLEN];
	struct t_group *grpptr;
#ifdef NNTP_ABLE
	char *ngnames[NUM_SIMULTANEOUS_GROUP_COMMAND];
	int index_i = 0;
	int index_o = 0;
#endif /* NNTP_ABLE */

	/*
	 * return immediately if no .newsrc can be found or .newsrc is empty
	 * when function asked to use .newsrc
	 */
	if ((fp = fopen(newsrc, "r")) == NULL)
		return;

	if (file_size(newsrc) <= 0L) {
		fclose(fp);
		return;
	}

	if (!batch_mode)
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
			strncpy(ngname, ptr, 128);
			ptr = ngname;
		}

		if (read_news_via_nntp) {
#ifdef NNTP_ABLE
			char buf[NNTP_STRLEN];
			char line[NNTP_STRLEN];
			if (window < NUM_SIMULTANEOUS_GROUP_COMMAND && ptr) {
				ngnames[index_i] = my_strdup(ptr);
				snprintf(buf, sizeof(buf), "GROUP %s", ngnames[index_i]);
#	ifdef DEBUG
				debug_nntp("read_newsrc_active_file", buf);
#	endif /* DEBUG */
				put_server(buf);
				index_i = (index_i + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
				window++;
			}
			if (window == NUM_SIMULTANEOUS_GROUP_COMMAND || ptr == NULL) {
				int respcode = get_respcode(line, sizeof(line));

				if (reconnected_in_last_get_server) {
					/*
					 * If tin reconnected, last output is resended to server.
					 * So received data is for ngnames[last window_i].
					 * We resend all buffered command except for last window_i.
					 * And rotate buffer to use data received.
					 */
					int i;
					int j = index_o;
					for (i = 0; i < window - 1; i++) {
						snprintf(buf, sizeof(buf), "GROUP %s", ngnames[j]);
#	ifdef DEBUG
						debug_nntp("read_newsrc_active_file", buf);
#	endif /* DEBUG */
						put_server(buf);
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
							char fmt[20];

							snprintf(fmt, sizeof(fmt), "%%ld %%ld %%ld %%%ds", NNTP_STRLEN);
							if (sscanf(line, fmt, &count, &min, &max, ngname) != 4)
								error_message(_(txt_error_invalid_response_to_group), line);
							if (strcmp(ngname, ngnames[index_o]) != 0)
								error_message(_(txt_error_wrong_newsgroupname_in_group_response), line, ngnames[index_o]);
							ptr = ngname;
							free(ngnames[index_o]);
							index_o = (index_o + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
							window--;
							break;
						}

					case ERR_NOGROUP:
						free(ngnames[index_o]);
						index_o = (index_o + 1) % NUM_SIMULTANEOUS_GROUP_COMMAND;
						window--;
						continue;

					case ERR_ACCESS:
						error_message("%s%s", cCRLF, line);
						tin_done(NNTP_ERROR_EXIT);
						/* keep lint quiet: */
						/* FALLTHROUGH */

					default:
#	ifdef DEBUG
						debug_nntp("NOT_OK", line);
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
			t_bool changed = FALSE;

			if ((grpptr = group_find(ptr)) == NULL)
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

	fclose(fp);

	/*
	 * Exit if active file wasn't read correctly or is empty
	 */
	if (tin_errno || !num_active) {
		if (newsrc_active && !num_active)
			error_message(_(txt_error_server_has_no_listed_groups), newsrc);
		else
			error_message(_(txt_active_file_is_empty), (read_news_via_nntp ? _(txt_servers_active) : news_active_file));
		tin_done(EXIT_FAILURE);
	}

	if (!batch_mode)
		my_fputs("\n", stdout);
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
	else
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
	long count = -1L, min = 1L, max = 0L;
	long processed = 0L;
	struct t_group *grpptr;

	if (!batch_mode)
		wait_message(0, _(txt_reading_news_active_file));

	if ((fp = open_news_active_fp()) == NULL) {
		if (cmd_line && !batch_mode)
			my_fputc('\n', stderr);

#ifdef NNTP_ABLE
		if (read_news_via_nntp)
			error_message(_(txt_cannot_retrieve), ACTIVE_FILE);
#	ifndef NNTP_ONLY
		else
			error_message(_(txt_cannot_open_active_file), news_active_file, tin_progname);
#	endif /* !NNTP_ONLY */
#else
		error_message(_(txt_cannot_open), news_active_file);
#endif /* NNTP_ABLE */

		tin_done(EXIT_FAILURE);
	}

	while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
		if (!parse_active_line(ptr, &max, &min, moderated))
			continue;

		if (++processed % MODULO_COUNT_NUM == 0)
			spin_cursor();

		/*
		 * Load group into group hash table
		 * NULL means group already present, so we just fixup the counters
		 * This call may implicitly ++num_active
		 */
		if ((grpptr = group_add(ptr)) == NULL) {
			if ((grpptr = group_find(ptr)) == NULL)
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

	TIN_FCLOSE(fp);

	/*
	 * Exit if active file wasn't read correctly or is empty
	 */
	if (tin_errno || !num_active) {
		error_message(_(txt_active_file_is_empty), (read_news_via_nntp ? _(txt_servers_active) : news_active_file));
		tin_done(EXIT_FAILURE);
	}

	if (!batch_mode)
		my_fputs("\n", stdout);
}


/*
 * Load the active file into active[]
 * Check and preload any new newgroups into my_group[]
 */
void
read_news_active_file(
	void)
{
	FILE *fp;

	/*
	 * Ignore -n if no .newsrc can be found or .newsrc is empty
	 */
	if (newsrc_active) {
		if ((fp = fopen(newsrc, "r")) == NULL) {
			list_active = TRUE;
			newsrc_active = FALSE;
		} else {
			fclose(fp);
			if (file_size(newsrc) <= 0L) {
				list_active = TRUE;
				newsrc_active = FALSE;
			}
		}
	}

	/* Read an active file if it is allowed */
	if (list_active)
		read_active_file();

	/* Read .newsrc and check each group */
	if (newsrc_active)
		read_newsrc_active_file();

	(void) time(&active_timestamp);
	force_reread_active_file = FALSE;

	/*
	 * check_for_any_new_groups() also does $AUTOSUBSCRIBE
	 */
	if (check_for_new_newsgroups)
		check_for_any_new_groups();

	/*
	 * finally we have a list of all groups an can set the attributes
	 */
	read_attributes_files();
}


/*
 * Open the active.times file locally or send the NEWGROUPS command
 *
 * NEWGROUPS yymmdd hhmmss
 */
static FILE *
open_newgroups_fp(
	int idx)
{
#ifdef NNTP_ABLE
	char line[NNTP_STRLEN];
	struct tm *ngtm;

	if (read_news_via_nntp && !read_saved_news) {
		if (idx == -1)
			return (FILE *) 0;

		ngtm = localtime(&newnews[idx].time);
		/*
		 * in the current draft, NEWGROUPS is allowed to take a 4 digit year
		 * component - but even with a 2 digit year component it is y2k
		 * compliant... we should switch over to ngtm->tm_year + 1900
		 * when most servers can handle the new format
		 */
		snprintf(line, sizeof(line), "NEWGROUPS %02d%02d%02d %02d%02d%02d",
			ngtm->tm_year % 100, ngtm->tm_mon + 1, ngtm->tm_mday,
			ngtm->tm_hour, ngtm->tm_min, ngtm->tm_sec);

		return (nntp_command(line, OK_NEWGROUPS, NULL, 0));
	} else
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
static void
check_for_any_new_groups(
	void)
{
	FILE *fp;
	char *autosubscribe, *autounsubscribe;
	char *ptr, *line, buf[NNTP_STRLEN];
	char old_newnews_host[PATH_LEN];
	int newnews_index;
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
	if (debug == 2) {
		error_message("Newnews old=[%lu]  new=[%lu]", (unsigned long int) old_newnews_time, (unsigned long int) new_newnews_time);
		(void) sleep(2);
	}
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
		}
		TIN_FCLOSE(fp);

		if (tin_errno)
			return;				/* Don't update the time if we quit */
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
	char *autosubscribe,
	char *autounsubscribe)
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
	if ((idx = my_group_add(group)) < 0) {
		if (list_active) {
/*			my_fprintf(stderr, "subscribe_new_group: %s not in active[] && list_active\n", group); */
			return;
		}

		if ((ptr = group_add(group)) != NULL)
			active_add(ptr, 0L, 1L, 0L, "y");

		if ((idx = my_group_add(group)) < 0)
			return;
	}

	if (!no_write && (autosubscribe != NULL) && match_group_list(group, autosubscribe)) {
		if (!batch_mode || verbose)
			my_printf(_(txt_autosubscribed), group);

		/*
		 * as subscribe_new_group() is called from check_for_any_new_groups()
		 * which has pending data on the socket if reading via NNTP we are not
		 * allowed to issue any NNTP comands yet
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

		if ((negate = ('!' == *group_list))) {
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
		strncpy(pattern, group_list, group_len);
		pattern[group_len] = '\0';
		/*
		 * case-insensitive wildcard match
		 */
		if (GROUP_MATCH(group, pattern, TRUE))
			accept = bool_not(negate);	/* matched! */

		/*
		 * now examine next entry if any
		 */
		if ((char) 0 != group_list[group_len])
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
 *
 * Side effects:
 *   'info' is modified. Caller should not depend on it.
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
	if (debug == 2)
		error_message("ACTIVE host=[%s] time=[%lu]", newnews[i].host, (unsigned long int) newnews[i].time);
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
 *
 * TODO: what about 'j' groups? active(5) says:
 *       "local postings to that group should not be generated"
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
			return 'X';

		case '=':
			return '=';

		default:
			return ' ';
	}
}


/* ex actived.c functions */
void
create_save_active_file(
	void)
{
	char group_path[PATH_LEN];
	char local_save_active_file[PATH_LEN];

	joinpath(local_save_active_file, rcdir, ACTIVE_SAVE_FILE);

	if (no_write && file_size(local_save_active_file) != -1L)
		return;

	my_printf(_(txt_creating_active));

	print_active_head(local_save_active_file);
	strcpy(group_path, tinrc.savedir);
	make_group_list(local_save_active_file, tinrc.savedir, group_path);
}


static void
make_group_list(
	char *active_file,
	char *base_dir,
	char *group_path)
{
	DIR *dir;
	DIR_BUF *direntry;
	char *ptr;
	char filename[PATH_LEN];
	char path[PATH_LEN];
	long art_max;
	long art_min;
	struct stat stat_info;
	t_bool is_dir;

	if ((dir = opendir(group_path)) != NULL) {
		is_dir = FALSE;
		while ((direntry = readdir(dir)) != NULL) {
			STRCPY(filename, direntry->d_name);
			joinpath(path, group_path, filename);

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

				make_group_list(active_file, base_dir, group_path);
				find_art_max_min(group_path, &art_max, &art_min);
				append_group_line(active_file, group_path, art_max, art_min, base_dir);
				if ((ptr = strrchr(group_path, '/')) != NULL) /* TODO: Unix'ism */
					*ptr = '\0';
			}
		}
		CLOSEDIR(dir);
	}
}


static void
append_group_line(
	char *active_file,
	char *group_path,
	long art_max,
	long art_min,
	char *base_dir)
{
	FILE *fp;
	char *file_tmp;
	char group_name[PATH_LEN];

	if (art_max == 0 && art_min == 1)
		return;

	file_tmp = get_tmpfilename(active_file);

	if (!backup_file(active_file, file_tmp)) {
		free(file_tmp);
		return;
	}

	if ((fp = fopen(active_file, "a+")) != NULL) {
		make_group_name(base_dir, group_name, group_path);
		my_printf("Appending=[%s %ld %ld %s]\n", group_name, art_max, art_min, base_dir);
		print_group_line(fp, group_name, art_max, art_min, base_dir);
		if (ferror(fp) || fclose(fp)) /* TODO: issue warning? */
			rename(file_tmp, active_file);
	}
	unlink(file_tmp);
	free(file_tmp);
}


/*
 * Given an absolute pathname & a base pathname build a newsgroup name
 * base = /usr/spool/news
 * absolute path = /usr/spool/news/alt/sources
 * newsgroup = alt.sources
 */
static void
make_group_name(
	char *base_dir,
	char *group_name,
	char *group_path)
{
	char *base_ptr;
	char *name_ptr;
	char *path_ptr;

	base_ptr = base_dir;
	path_ptr = group_path;

	while (*base_ptr && (*base_ptr == *path_ptr)) {
		base_ptr++;
		path_ptr++;
	}
	strcpy(group_name, ++path_ptr);

	name_ptr = group_name;
	while ((name_ptr = strchr(name_ptr, '/')) != NULL)
		*name_ptr = '.';
}
