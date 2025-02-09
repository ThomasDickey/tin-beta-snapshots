/*
 *  Project   : tin - a Usenet reader
 *  Module    : newsrc.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2025-02-06
 *  Notes     : ArtCount = (ArtMax - ArtMin) + 1  [could have holes]
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */
#ifndef NEWSRC_H
#	include "newsrc.h"
#endif /* !NEWSRC_H */

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	static mode_t newsrc_mode = 0;
#endif /* HAVE_FCHMOD || HAVE_CHMOD */


/*
 * Local prototypes
 */
static FILE *open_subscription_fp(void);
static char *parse_newsrc_line(char *line, int *sub);
static char *parse_subseq(struct t_group *group, char *seq, t_artnum *low, t_artnum *high, t_artnum *sum);
static char *parse_get_seq(char *seq, t_artnum *low, t_artnum *high);
static int write_newsrc_line(FILE *fp, char *line);
static t_bool create_newsrc(char *newsrc_file);
static void auto_subscribe_groups(const char *newsrc_file);
static void get_subscribe_info(struct t_group *grp);
static void parse_bitmap_seq(struct t_group *group, char *seq);
static void print_bitmap_seq(FILE *fp, struct t_group *group);


/*
 * Read .newsrc into my_group[]. my_group[] ints point to active[] entries.
 * If allgroups is set, then my_group[] is completely overwritten,
 * otherwise, groups are appended. Any bogus groups will be handled
 * accordingly. Bogus groups will _not_ be subscribed to as a design
 * principle.
 *
 * Returns the number of lines read(useful for a check newsrc >= oldnewsrc)
 * 	< 0 error
 * 	>=0 number of lines read
 */
signed long int
read_newsrc(
	char *newsrc_file,
	t_bool allgroups)
{
	FILE *fp;
	char *grp, *seq;
	int sub, i;
	signed long line_count = 0;
	struct stat statbuf;

	if (allgroups)
		selmenu.max = skip_newgroups();

	/*
	 * make a .newsrc if none exist & auto subscribe to set groups
	 */
	if ((fp = fopen(newsrc_file, "r")) == NULL) {
		if (errno != ENOENT) { /* ENOENT is "ok" here */
			perror_message(_(txt_cannot_open), newsrc_file);
			if (!batch_mode)
				sleep(2);
		}
		switch (errno) {
			case EACCES:
				return -1L;

			default:	/* e.g. ENOENT */
				if (!create_newsrc(newsrc_file))
					return -1L;
				break;
		}
		auto_subscribe_groups(newsrc_file);
	}
	/* see if create_newsrc() created the file for us */
	if (!fp) {
		if ((fp = fopen(newsrc_file, "r")) == NULL) {
			perror_message(_(txt_cannot_open), newsrc_file);
			if (!batch_mode)
				sleep(2);
			return -1L;
		}
	}

	if (fstat(fileno(fp), &statbuf) == -1) {
		perror_message(_(txt_cannot_open), newsrc_file);
		if (!batch_mode)
			sleep(2);
		fclose(fp);
		return -1L;
	}
#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	else
		newsrc_mode = statbuf.st_mode;
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	if (S_ISDIR(statbuf.st_mode)) {	/* TODO: die hard? */
		errno = EISDIR;
		perror_message(_(txt_cannot_open), newsrc_file);
		if (!batch_mode)
			sleep(2);
		fclose(fp);
		return -1L;
	}

	if (!batch_mode || verbose)
		wait_message(0, _(txt_reading_newsrc), newsrc_file);

	while ((grp = tin_fgets(fp, FALSE)) != NULL) {
		strip_line(grp);

		/* mixed up -f / -F ? */
		if (*grp && !line_count && !strncmp(grp, txt_filter_file_version, 15)) { /* version number starts after pos. 15 */
			fclose(fp);
			tin_done(EXIT_FAILURE, _(txt_error_mixed_up_opt), "-f", newsrc_file);
		}
		if (*grp == '#' || *grp == '\0')	/* skip comments and empty lines */
			continue;

		++line_count;	/* but count all other lines (incl. bogus ones) */
		seq = parse_newsrc_line(grp, &sub);

		if (sub == SUBSCRIBED) {
			if ((i = my_group_add(grp, FALSE)) >= 0) {
				if (!active[my_group[i]].bogus) {
					active[my_group[i]].subscribed = SUB_BOOL(sub);
					parse_bitmap_seq(&active[my_group[i]], seq);
				}
			} else
				process_bogus(grp);
		}
	}
	fclose(fp);
	/* If you aborted with 'q', then you get what you get. */

	if (!batch_mode || verbose)
		my_fputc('\n', stdout);

	if (!cmd_line && !batch_mode)
		clear_message();

	return line_count;
}


/*
 * Parse a line from the newsrc file and write it back out with updated
 * sequence information. Return number of lines written (ie, 0 or 1)
 */
static int
write_newsrc_line(
	FILE *fp,
	char *line)
{
	char *seq;
	int sub;
	struct t_group *group;

	seq = parse_newsrc_line(line, &sub);

	if (line[0] == '\0' || sub == 0)		/* Insurance against blank line */
		return 0;

	if (seq == NULL) {		/* line has no ':' or '!' in it */
		if (tinrc.strip_bogus == BOGUS_REMOVE)
			wait_message(2, _(txt_remove_bogus), line);
		return 0;
	}

	/*
	 * Find the group in active. If we cannot, then junk it if bogus groups
	 * are set to auto removal. Also check for bogus flag just in case
	 * strip_bogus was changed since tin started
	 */
	group = group_find(line, FALSE);

	if (tinrc.strip_bogus == BOGUS_REMOVE) {
		if (group == NULL || group->bogus) { /* group doesn't exist */
			wait_message(2, _(txt_remove_bogus), line);
			return 0;
		}
	}

	if ((group && group->newsrc.present) && (group->subscribed || !tinrc.strip_newsrc)) {
		fprintf(fp, "%s%c ", group->name, SUB_CHAR(group->subscribed));
		print_bitmap_seq(fp, group);
		return 1;
	} else {
		if (sub == SUBSCRIBED || !tinrc.strip_newsrc) {
			fprintf(fp, "%s%c %s\n", line, sub, seq);
			return 1;
		}
	}
	return 0;
}


/*
 * Read in the users newsrc file and write a new file with all the changes
 * changes from the current session. If this works, replace the original
 * newsrc file.
 * Returns < 0 on error
 *         >=0 number of lines written
 */
signed long int
write_newsrc(
	void)
{
	FILE *fp_ip;
	FILE *fp_op;
	char *line;
	signed long int tot = 0L;
	t_bool write_ok = FALSE;
	int err;

	if (no_write)
		return 0L;

	if ((fp_ip = tin_fopen(newsrc, "r")) == NULL) {
		if (errno == ENOENT)
			perror_message(_(txt_cannot_open), newsrc);
		if (!batch_mode && errno)
			sleep(2);
		return (errno ? -1L : 0L);
	}

	if ((fp_op = fopen(newnewsrc, "w")) != NULL) {
#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
		if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
			fchmod(fileno(fp_op), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
			chmod(newnewsrc, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

		while ((line = tin_fgets(fp_ip, FALSE)) != NULL)
			tot += write_newsrc_line(fp_op, line);

		/*
		 * Don't rename if either fclose() fails or ferror() is set
		 */
		if ((err = ferror(fp_op)) || fclose(fp_op)) {
			error_message(2, _(txt_filesystem_full), newsrc);
			unlink(newnewsrc);
			if (err) {
				clearerr(fp_op);
				fclose(fp_op);
			}
		} else
			write_ok = TRUE;
	} else {
		perror_message(_(txt_cannot_open_for_saving), newnewsrc);
		fclose(fp_ip);
		return 0L; /* o we don't get prompted to try again */
	}

	fclose(fp_ip);

	if (tot < 1L) {
		error_message(2, _(txt_newsrc_nogroups), newsrc);
		unlink(newnewsrc);
		return 0L;		/* So we don't get prompted to try again */
	}

	if (write_ok)
		rename_file(newnewsrc, newsrc);

	return tot;
}


/*
 * Create a newsrc from active[] groups. Subscribe to all groups.
 */
static t_bool
create_newsrc(
	char *newsrc_file)
{
	FILE *fp;
	int i;

	if ((fp = fopen(newsrc_file, "w")) != NULL) {
		wait_message(0, _(txt_creating_newsrc), newsrc);

		for_each_group(i)
			fprintf(fp, "%s!\n", active[i].name);

		if ((i = ferror(fp)) || fclose(fp)) {
			error_message(2, _(txt_filesystem_full), newsrc);
			if (i) {
				clearerr(fp);
				fclose(fp);
			}
			return FALSE;
		}
		return TRUE; /* newsrc created */
	}

	perror_message(_(txt_cannot_open), newsrc_file);
	if (!batch_mode)
		sleep(2);

	return FALSE;
}


/*
 * Get a list of default groups to subscribe to
 */
static FILE *
open_subscription_fp(
	void)
{
	if (read_saved_news)
		return (FILE *) 0;
	else {
#ifdef NNTP_ABLE
		if (read_news_via_nntp) {
			/* RFC 6048 2.6 */
			if (nntp_caps.type == CAPABILITIES && !nntp_caps.list_subscriptions)
				return (FILE *) 0;
			else
				return (nntp_command("LIST SUBSCRIPTIONS", OK_GROUPS, NULL, 0));
		}
#endif /* NNTP_ABLE */
#ifndef NNTP_ONLY
		return (fopen(subscriptions_file, "r"));
#else
		return (FILE *) 0;
#endif /* !NNTP_ONLY */
	}
}


/*
 * Automatically subscribe user to newsgroups specified in
 * NEWSLIBDIR/subscriptions (locally) or same file but from NNTP
 * server (LIST SUBSCRIPTIONS) and create .newsrc
 */
static void
auto_subscribe_groups(
	const char *newsrc_file)
{
	FILE *fp_newsrc;
	FILE *fp_subs;
	char *ptr;
	int err;

	/*
	 * If subscription file exists then first unsubscribe to all groups
	 * and then subscribe to just the auto specified groups.
	 */
	if ((fp_subs = open_subscription_fp()) == NULL)
		return;

	if (!batch_mode)
		wait_message(0, _(txt_autosubscribing_groups));

	if ((fp_newsrc = fopen(newsrc_file, "w")) == NULL) {
		TIN_FCLOSE(fp_subs);
		return;
	}

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp_newsrc), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
		chmod(newsrc_file, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	/* TODO: test me! */
	while ((ptr = tin_fgets(fp_subs, FALSE)) != NULL) {
		if (ptr[0] != '#') {
			if (group_find(ptr, FALSE) != NULL)
				fprintf(fp_newsrc, "%s:\n", ptr);
		}
	}

	/* We ignore user 'q'uits here. They will get them next time in any case */

	if ((err = ferror(fp_newsrc)) || fclose(fp_newsrc)) {
		error_message(2, _(txt_filesystem_full), newsrc);
		if (err) {
			clearerr(fp_newsrc);
			fclose(fp_newsrc);
		}
	}

	TIN_FCLOSE(fp_subs);
}


/*
 * make a backup of users .newsrc in case of the bogie man
 */
void
backup_newsrc(
	void)
{
	char dirbuf[PATH_LEN];
	char filebuf[PATH_LEN];
	struct stat statbuf;

#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news && nntp_tcp_port != IPPORT_NNTP)
		snprintf(filebuf, sizeof(filebuf), "%s:%u", nntp_server, nntp_tcp_port);
	else
#endif /* NNTP_ABLE */
	{
		STRCPY(filebuf, quote_space_to_dash(nntp_server));
	}
	joinpath(dirbuf, sizeof(dirbuf), rcdir, filebuf);
	joinpath(filebuf, sizeof(filebuf), dirbuf, OLDNEWSRC_FILE);

	if (stat(dirbuf, &statbuf) == -1) {
		if (my_mkdir(dirbuf, (mode_t) (S_IRWXU)) == -1)
			/* Can't create directory: Fall back on Homedir */
			joinpath(filebuf, sizeof(filebuf), homedir, OLDNEWSRC_FILE);
	}

	if (!backup_file(newsrc, filebuf))
		error_message(2, _(txt_filesystem_full_backup), newsrc);
}


/*
 * Find the total, max & min articles number for specified group
 * Use nntp GROUP command or read local spool
 * Return 0, or -error
 */
int
group_get_art_info(
	const char *tin_spooldir,
	const char *groupname,
	int grouptype,
	t_artnum *art_count,
	t_artnum *art_max,
	t_artnum *art_min)
{
	t_artnum artnum;

	if (read_news_via_nntp && grouptype == GROUP_TYPE_NEWS) {
#ifdef NNTP_ABLE
		char line[NNTP_STRLEN];

		snprintf(line, sizeof(line), "GROUP %s", groupname);
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "group_get_art_info %s", line);
#	endif /* DEBUG */
		put_server(line, FALSE);

		switch (get_respcode(line, sizeof(line))) {
			case OK_GROUP:
				if (sscanf(line, "%"T_ARTNUM_SFMT" %"T_ARTNUM_SFMT" %"T_ARTNUM_SFMT, art_count, art_min, art_max) != 3) {
#	ifdef DEBUG
					if ((debug & DEBUG_NNTP) && verbose > 1)
						debug_print_file("NNTP", "Invalid response to GROUP command, %s", line);
#	endif /* DEBUG */
				}
				break;

			case ERR_NOGROUP:
				*art_count = T_ARTNUM_CONST(0);
				*art_min = T_ARTNUM_CONST(1);
				*art_max = T_ARTNUM_CONST(0);
				return -ERR_NOGROUP;

			case ERR_ACCESS:
				tin_done(NNTP_ERROR_EXIT, "%s", line);
				/* keep lint quiet: */
				/* NOTREACHED */
				break;

			default:
#	ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose > 1)
					debug_print_file("NNTP", "NOT_OK %s", line);
#	endif /* DEBUG */
				return -1;
		}
#else
		my_fprintf(stderr, "%s", _(txt_error_unreachable));
		return 0;
#endif /* NNTP_ABLE */
	} else {
		char group_path[PATH_LEN];
		DIR *dir;
		DIR_BUF *direntry;

		*art_count = T_ARTNUM_CONST(0);
		*art_min = T_ARTNUM_CONST(0);
		*art_max = T_ARTNUM_CONST(0);

		make_base_group_path(tin_spooldir, groupname, group_path, sizeof(group_path));

		if ((dir = opendir(group_path)) != NULL) {
			while ((direntry = readdir(dir)) != NULL) {
				artnum = atoartnum(direntry->d_name); /* should be '\0' terminated... */
				if (artnum >= T_ARTNUM_CONST(1)) {
					if (artnum > *art_max) {
						*art_max = artnum;
						if (*art_min == T_ARTNUM_CONST(0))
							*art_min = artnum;
					} else if (artnum < *art_min)
						*art_min = artnum;
					(*art_count)++;
				}
			}
			CLOSEDIR(dir);
			if (*art_min == T_ARTNUM_CONST(0))
				*art_min = T_ARTNUM_CONST(1);
		} else {
			*art_min = T_ARTNUM_CONST(1);
			return -1;
		}
	}

	return 0;
}


/*
 * Get and fixup (if needed) the counters for a newly subscribed group
 */
static void
get_subscribe_info(
	struct t_group *grp)
{
	t_artnum oldmin = grp->xmin;
	t_artnum oldmax = grp->xmax;

	group_get_art_info(grp->spooldir, grp->name, grp->type, &grp->count, &grp->xmax, &grp->xmin);

	if (grp->newsrc.num_unread > grp->count) {
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC) { /* TODO: is this the right debug-level? */
			my_printf(cCRLF "Unread WRONG %s unread=[%"T_ARTNUM_PFMT"] count=[%"T_ARTNUM_PFMT"]", grp->name, grp->newsrc.num_unread, grp->count);
			my_flush();
		}
#endif /* DEBUG */
		grp->newsrc.num_unread = grp->count;
	}

	if (grp->xmin != oldmin || grp->xmax != oldmax) {
		expand_bitmap(grp, 0);
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC) { /* TODO: is this the right debug-level? */
			my_printf(cCRLF "Min/Max DIFF %s old=[%"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT"] new=[%"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT"]", grp->name, oldmin, oldmax, grp->xmin, grp->xmax);
			my_flush();
		}
#endif /* DEBUG */
	}
}


/*
 * Subscribe/unsubscribe to a group in .newsrc.
 * This involves rewriting the .newsrc with the new info
 * If get_info is set we are allowed to issue NNTP commands if needed
 */
void
subscribe(
	struct t_group *group,
	int sub_state,
	t_bool get_info)
{
	FILE *fp;
	FILE *newfp;
	char *line;
	char *seq;
	int sub;
	t_bool found = FALSE;

	if (no_write || (newfp = fopen(newnewsrc, "w")) == NULL)
		return;

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
		fchmod(fileno(newfp), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
		chmod(newnewsrc, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	if ((fp = fopen(newsrc, "r")) != NULL) {
		while ((line = tin_fgets(fp, FALSE)) != NULL) {
			if ((seq = parse_newsrc_line(line, &sub))) {
				if (STRCMPEQ(line, group->name)) {
					fprintf(newfp, "%s%c %s\n", line, sub_state, seq);
					group->subscribed = SUB_BOOL(sub_state);

					/* If previously subscribed to in .newsrc, load up any existing information */
					if (sub_state == SUBSCRIBED)
						parse_bitmap_seq(group, seq);

					found = TRUE;
				} else
					fprintf(newfp, "%s%c %s\n", line, sub, seq);
			}
		}

		fclose(fp);

		if (!found) {
			wait_message(0, _(txt_subscribing));
			group->subscribed = SUB_BOOL(sub_state);
			if (sub_state == SUBSCRIBED) {
				fprintf(newfp, "%s%c ", group->name, sub_state);
				if (get_info) {
					get_subscribe_info(group);
					print_bitmap_seq(newfp, group);
				} else /* we are not allowed to issue NNTP cmds during AUTOSUBSCRIBE loop */
					fprintf(newfp, "1\n");
			} else
				fprintf(newfp, "%s%c\n", group->name, sub_state);
		}
	}

	if ((sub = ferror(newfp)) || fclose(newfp)) {
		error_message(2, _(txt_filesystem_full), newsrc);
		if (sub) {
			clearerr(newfp);
			fclose(newfp);
		}
		unlink(newnewsrc);
	} else
		rename_file(newnewsrc, newsrc);
}


/*
 * Like subscribe() but for a bunch of groups.
 */
void
bulk_subscribe(
	struct t_group **groups,
	int groups_cnt,
	int sub_state,
	t_bool get_info)
{
	FILE *fp;
	FILE *newfp;
	char *line;
	char *seq;
	int i, sub;
	t_bool found;

	if (no_write || (newfp = fopen(newnewsrc, "w")) == NULL)
		return;

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
		fchmod(fileno(newfp), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
		chmod(newnewsrc, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	if ((fp = fopen(newsrc, "r")) != NULL) {
		while ((line = tin_fgets(fp, FALSE)) != NULL) {
			if ((seq = parse_newsrc_line(line, &sub))) {
				found = FALSE;
				for (i = 0; i < groups_cnt; i++) {
					if (groups[i] && STRCMPEQ(line, groups[i]->name)) {
						fprintf(newfp, "%s%c %s\n", line, sub_state, seq);
						groups[i]->subscribed = SUB_BOOL(sub_state);

						/* If previously subscribed to in .newsrc, load up any existing information */
						if (sub_state == SUBSCRIBED)
							parse_bitmap_seq(groups[i], seq);

						groups[i] = NULL;
						found = TRUE;
						break;
					}
				}
				if (!found)
					fprintf(newfp, "%s%c %s\n", line, sub, seq);
			}
		}

		fclose(fp);

		for (i = 0; i < groups_cnt; i++) {
			if (groups[i]) {
				wait_message(0, _(txt_subscribing));
				spin_cursor();
				groups[i]->subscribed = SUB_BOOL(sub_state);
				if (sub_state == SUBSCRIBED) {
					fprintf(newfp, "%s%c ", groups[i]->name, sub_state);
					if (get_info) {
						get_subscribe_info(groups[i]);
						print_bitmap_seq(newfp, groups[i]);
					} else /* we are not allowed to issue NNTP cmds during AUTOSUBSCRIBE loop */
						fprintf(newfp, "1\n");
				} else
					fprintf(newfp, "%s%c\n", groups[i]->name, sub_state);
			}
		}
	}

	if ((sub = ferror(newfp)) || fclose(newfp)) {
		error_message(2, _(txt_filesystem_full), newsrc);
		if (sub) {
			clearerr(newfp);
			fclose(newfp);
		}
		unlink(newnewsrc);
	} else
		rename_file(newnewsrc, newsrc);
}


void
reset_newsrc(
	void)
{
	FILE *fp;
	FILE *newfp;
	char *line;
	int sub, i;

	if (!no_write && (newfp = fopen(newnewsrc, "w")) != NULL) {
#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
		if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
			fchmod(fileno(newfp), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
			chmod(newnewsrc, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
		}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

		if ((fp = fopen(newsrc, "r")) != NULL) {
			while ((line = tin_fgets(fp, FALSE)) != NULL) {
				(void) parse_newsrc_line(line, &sub);
				fprintf(newfp, "%s%c\n", line, sub);
			}
			fclose(fp);
		}
		if ((sub = ferror(newfp)) || fclose(newfp)) {
			error_message(2, _(txt_filesystem_full), newsrc);
			if (sub) {
				clearerr(newfp);
				fclose(newfp);
			}
			unlink(newnewsrc);
		} else
			rename_file(newnewsrc, newsrc);
	}

	for (i = 0; i < selmenu.max; i++)
		set_default_bitmap(&active[my_group[i]]);
}


/*
 * Rewrite the newsrc file, without the specified group
 */
void
delete_group(
	const char *group)
{
	FILE *fp;
	FILE *newfp;
	char *line;
	char *seq;
	int sub;

	if (no_write)
		return;

	if ((newfp = fopen(newnewsrc, "w")) != NULL) {
#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
		if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
			fchmod(fileno(newfp), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
			chmod(newnewsrc, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

		if ((fp = fopen(newsrc, "r")) != NULL) {
			while ((line = tin_fgets(fp, FALSE)) != NULL) {
				if ((seq = parse_newsrc_line(line, &sub))) {
					if (!STRCMPEQ(line, group))
						fprintf(newfp, "%s%c %s\n", line, sub, seq);
				}
			}
			fclose(fp);
		}

		if ((sub = ferror(newfp)) || fclose(newfp)) {
			error_message(2, _(txt_filesystem_full), newsrc);
			if (sub) {
				clearerr(newfp);
				fclose(newfp);
			}
			unlink(newnewsrc);
		} else
			rename_file(newnewsrc, newsrc);
	}
}


/*
 * Mark a group as read
 * If art != NULL then we explicitly process each article thus
 * catching crossposts as well, otherwise we simply scrub the
 * bitmap and adjust the highwater mark.
 */
void
grp_mark_read(
	struct t_group *group,
	struct t_article *art)
{
	int i;

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_comment("c/C command");
#endif /* DEBUG */

	if (art != NULL) {
		for_each_art(i)
			art_mark(group, &art[i], ART_READ);
	} else {
		FreeAndNull(group->newsrc.xbitmap);
		group->newsrc.xbitlen = 0;
		if (group->xmax > group->newsrc.xmax)
			group->newsrc.xmax = group->xmax;
		group->newsrc.xmin = group->newsrc.xmax + 1;
		group->newsrc.num_unread = 0;
	}
}


void
grp_mark_unread(
	struct t_group *group)
{
	t_artnum bitlength;
	t_bitmap *newbitmap = (t_bitmap *) 0;

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_comment("Z command");
#endif /* DEBUG */

	group_get_art_info(group->spooldir, group->name, group->type, &group->count, &group->xmax, &group->xmin);

	group->newsrc.num_unread = group->count;
	if (group->xmax > group->newsrc.xmax)
		group->newsrc.xmax = group->xmax;
	if (group->xmin > 0)
		group->newsrc.xmin = group->xmin;

	bitlength = MAX(0, group->newsrc.xmax - group->newsrc.xmin + 1);

	if (bitlength > 0)
		newbitmap = my_malloc(BITS_TO_BYTES(bitlength));

	FreeIfNeeded(group->newsrc.xbitmap);
	group->newsrc.xbitmap = newbitmap;
	group->newsrc.xbitlen = bitlength;

	if (bitlength)
		NSETRNG1(group->newsrc.xbitmap, T_ARTNUM_CONST(0), bitlength - T_ARTNUM_CONST(1));

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_bitmap(group, NULL);
#endif /* DEBUG */
}


void
thd_mark_read(
	struct t_group *group,
	long thread)
{
	int i;

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_comment("Mark thread read K command");
#endif /* DEBUG */

	for (i = (int) thread; i >= 0; i = arts[i].thread)
		art_mark(group, &arts[i], ART_READ);
}


void
thd_mark_unread(
	struct t_group *group,
	long thread)
{
	int i;

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_comment("Mark thread unread Z command");
#endif /* DEBUG */

	for (i = (int) thread; i >= 0; i = arts[i].thread)
		art_mark(group, &arts[i], ART_WILL_RETURN);
}


/*
 * Parse the newsrc sequence for the specified group
 */
static void
parse_bitmap_seq(
	struct t_group *group,
	char *seq)
{
	char *ptr;
	t_artnum sum = T_ARTNUM_CONST(0);
	t_artnum low = T_ARTNUM_CONST(0);
	t_artnum high = T_ARTNUM_CONST(0);
	t_artnum min, max;
	t_bool gotseq = FALSE;

	/*
	 * Skip possible non-numeric prefix
	 */
	ptr = seq;
	while (ptr && *ptr && (*ptr < '0' || *ptr > '9'))
		++ptr;

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC) {
		char buf[NEWSRC_LINE];

		snprintf(buf, sizeof(buf), "Parsing [%s%c %.*s]", group->name, SUB_CHAR(group->subscribed), (int) (NEWSRC_LINE - strlen(group->name) - 14), BlankIfNull(ptr));
		debug_print_comment(buf);
		debug_print_bitmap(group, NULL);
	}
#endif /* DEBUG */

	if (ptr) {
		gotseq = TRUE;
		ptr = parse_get_seq(ptr, &low, &high);

		if (high < group->xmin - 1)
			high = group->xmin - 1;

		min = ((low <= 1) ? (high + 1) : 1);

		if (group->xmin > min)
			min = group->xmin;

		if (group->xmax > high)
			max = group->xmax;
		else
			max = high;		/* trust newsrc's max */

		FreeAndNull(group->newsrc.xbitmap);
		group->newsrc.xmax = max;
		group->newsrc.xmin = min;
		group->newsrc.xbitlen = (max - min) + 1;
		if (group->newsrc.xbitlen > 0) {
			group->newsrc.xbitmap = my_malloc(BITS_TO_BYTES(group->newsrc.xbitlen));
			NSETRNG1(group->newsrc.xbitmap, T_ARTNUM_CONST(0), group->newsrc.xbitlen - T_ARTNUM_CONST(1));
		}

		if (min <= high) {
			if (low > min)
				sum = low - min;
			else
				low = min;
			NSETRNG0(group->newsrc.xbitmap, low - min, high - min);
		}

		/*
		 * Pick up any additional articles/ranges after the first
		 */
		while (*ptr)
			ptr = parse_subseq(group, ptr, &low, &high, &sum);
	} else {
		FreeAndNull(group->newsrc.xbitmap);
		group->newsrc.xmax = group->xmax;
		if (group->xmin > 0)
			group->newsrc.xmin = group->xmin;
		else
			group->newsrc.xmin = 1;
		group->newsrc.xbitlen = (group->newsrc.xmax - group->newsrc.xmin) + 1;
		if (group->newsrc.xbitlen > 0) {
			group->newsrc.xbitmap = my_malloc(BITS_TO_BYTES(group->newsrc.xbitlen));
			NSETRNG1(group->newsrc.xbitmap, T_ARTNUM_CONST(0), group->newsrc.xbitlen - T_ARTNUM_CONST(1));
		}
/*
wait_message(2, "BITMAP Grp=[%s] MinMax=[%"T_ARTNUM_PFMT"-%"T_ARTNUM_PFMT"] Len=[%"T_ARTNUM_PFMT"]\n",
	group->name, group->xmin, group->xmax, group->newsrc.xbitlen);
*/
	}

	group->newsrc.present = TRUE;

	if (gotseq) {
		if (group->newsrc.xmax > high)
			sum += group->newsrc.xmax - high;
	} else
		sum = (group->count >= 0) ? group->count : ((group->newsrc.xmax - group->newsrc.xmin) + 1);

	group->newsrc.num_unread = sum;
#ifdef DEBUG
	if (debug & DEBUG_NEWSRC)
		debug_print_bitmap(group, NULL);
#endif /* DEBUG */
}


/*
 * Parse a subsection of the newsrc sequencer ie., 1-34,23-90,93,97-99
 * would parse the sequence if called in a loop in the following way:
 *   1st call would parse  1-34 and return 23-90,93,97-99
 *   2nd call would parse 23-90 and return 93,97-99
 *   3rd call would parse    93 and return 97-99
 *   4th call would parse 97-99 and return NULL
 */
static char *
parse_subseq(
	struct t_group *group,
	char *seq,
	t_artnum *low,
	t_artnum *high,
	t_artnum *sum)
{
	t_artnum bitmin;
	t_artnum bitmax;
	t_artnum last_high = *high;

	seq = parse_get_seq(seq, low, high);

	/*
	 * Bitmap index
	 */
	bitmin = *low - group->newsrc.xmin;

	/*
	 * check that seq is not out of order
	 */
	if (*low > last_high)
		*sum += (*low - last_high) - 1;

	if (*high == *low) {
		if (bitmin >= 0) {
			if (*high > group->newsrc.xmax) {
				/* We trust .newsrc's max. */
				t_artnum bitlen;
				t_bitmap *newbitmap;

				group->newsrc.xmax = *high;
				bitlen = group->newsrc.xmax - group->newsrc.xmin + 1;
				newbitmap = my_malloc(BITS_TO_BYTES(bitlen));

				/* Copy over old bitmap */
				memcpy(newbitmap, group->newsrc.xbitmap, BITS_TO_BYTES(group->newsrc.xbitlen));

				/* Mark high numbered articles as unread */
				NSETRNG1(newbitmap, group->newsrc.xbitlen, bitlen - 1);

				free(group->newsrc.xbitmap);
				group->newsrc.xbitmap = newbitmap;
				group->newsrc.xbitlen = bitlen;
			}
			NSET0(group->newsrc.xbitmap, bitmin);
		}
	} else if ((*low < *high) && (*high >= group->newsrc.xmin)) {
		/*
		 * Restrict the range to min..max
		 */
		if (bitmin < 0)
			bitmin = 0;

		bitmax = *high;

		if (bitmax > group->newsrc.xmax) {
			/* We trust .newsrc's max. */
			t_artnum bitlen;
			t_bitmap *newbitmap;

			group->newsrc.xmax = bitmax;
			bitlen = group->newsrc.xmax - group->newsrc.xmin + 1;
			newbitmap = my_malloc(BITS_TO_BYTES(bitlen));

			/* Copy over old bitmap */
			memcpy(newbitmap, group->newsrc.xbitmap, BITS_TO_BYTES(group->newsrc.xbitlen));

			/* Mark high numbered articles as unread */
			NSETRNG1(newbitmap, group->newsrc.xbitlen, bitlen - 1);

			free(group->newsrc.xbitmap);
			group->newsrc.xbitmap = newbitmap;
			group->newsrc.xbitlen = bitlen;
		}

		bitmax -= group->newsrc.xmin;

		/*
		 * Fill in the whole range as read
		 */
		NSETRNG0(group->newsrc.xbitmap, bitmin, bitmax);
	}
	return seq;
}


static char *
parse_get_seq(
	char *seq,
	t_artnum *low,
	t_artnum *high)
{
	*low = strtoartnum(seq, &seq, 10);

	if (*seq == '-') {	/* Range of articles */
		++seq;
		*high = strtoartnum(seq, &seq, 10);
	} else	/* Single article */
		*high = *low;

	while (*seq && (*seq < '0' || *seq > '9'))
		++seq;

	return seq;
}


/*
 * Loop through arts[] array marking state of each article READ/UNREAD
 */
void
parse_unread_arts(
	struct t_group *group,
	t_artnum min)
{
	int i;
	t_artnum unread = T_ARTNUM_CONST(0);
	t_artnum bitmin, bitmax;
	t_bitmap *newbitmap = (t_bitmap *) 0;

	bitmin = group->newsrc.xmin;
	bitmax = group->newsrc.xmax;

	/*
	 * TODO
	 * what about group->newsrc.xmax > group->xmax?
	 * that should indicate an artnum 'reset' on the server
	 * (or using the "wrong" newsrc for that server)
	 */

	if (group->xmax > group->newsrc.xmax)
		group->newsrc.xmax = group->xmax;

	if (group->newsrc.xmax >= bitmin) {
		newbitmap = my_malloc(BITS_TO_BYTES(group->newsrc.xmax - bitmin + 1));
		NSETRNG0(newbitmap, T_ARTNUM_CONST(0), group->newsrc.xmax - bitmin);
	}

	/*
	 * if getart_limit > 0 preserve read/unread state
	 * of all articles below the new minimum
	 */
	if (min > 0 && newbitmap) {
		t_artnum j, tmp_bitmax;

		tmp_bitmax = (bitmax < min) ? bitmax : min;
		for (j = bitmin; j < tmp_bitmax; j++) {
			if (NTEST(group->newsrc.xbitmap, j - bitmin) != ART_READ)
				NSET1(newbitmap, j - bitmin);
		}
		while (j < min) {
			NSET1(newbitmap, j - bitmin);
			++j;
		}
	}

	for_each_art(i) {
		if (arts[i].artnum < bitmin)
			arts[i].status = ART_READ;
		else if (arts[i].artnum > bitmax)
			arts[i].status = ART_UNREAD;
		else if (NTEST(group->newsrc.xbitmap, arts[i].artnum - bitmin) == ART_READ)
			arts[i].status = ART_READ;
		else
			arts[i].status = ART_UNREAD;

		/* TODO: logic correct? */
		if (newbitmap != NULL && arts[i].status == ART_UNREAD && arts[i].artnum >= bitmin) {
#if 0
		/*
		 * check for wrong article numbers in the overview
		 *
		 * TODO: check disabled as we currently catch the artnum > high_mark
		 *       case in read_overview() where we might be able to
		 *       fix the broken artnum (via xref:-parsing). currently
		 *       we just skip the art there.
		 */
			if (arts[i].artnum <= group->xmax)
#endif /* 0 */
				NSET1(newbitmap, arts[i].artnum - bitmin);
			++unread;
		}
	}

	group->newsrc.xbitlen = group->newsrc.xmax - bitmin + 1;

	FreeIfNeeded(group->newsrc.xbitmap);

	group->newsrc.xbitmap = newbitmap;
	group->newsrc.num_unread = unread;
}


static void
print_bitmap_seq(
	FILE *fp,
	struct t_group *group)
{
	t_artnum artnum;
	t_artnum i;
	t_bool flag = FALSE;

#ifdef DEBUG
	if (debug & DEBUG_NEWSRC) {
		debug_print_comment("print_bitmap_seq()");
		debug_print_bitmap(group, NULL);
	}
#endif /* DEBUG */

	if (group->count == 0 || group->xmin > group->xmax) {
		if (group->newsrc.xmax > 1)
			fprintf(fp, "1-%"T_ARTNUM_PFMT, group->newsrc.xmax);

		fprintf(fp, "\n");
		fflush(fp);
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC)
			debug_print_comment("print_bitmap_seq(): group->count == 0");
#endif /* DEBUG */
		return;
	}

	i = group->newsrc.xmin;
	if (i <= group->newsrc.xmax) {
		forever {
			if (group->newsrc.xbitmap && NTEST(group->newsrc.xbitmap, i - group->newsrc.xmin) == ART_READ) {
				if (flag) {
					artnum = i;
					fprintf(fp, ",%"T_ARTNUM_PFMT, i);
				} else {
					artnum = 1;
					flag = TRUE;
					fprintf(fp, "1");
				}
				while (i < group->newsrc.xmax && NTEST(group->newsrc.xbitmap, (i + 1) - group->newsrc.xmin) == ART_READ)
					++i;

				if (artnum != i)
					fprintf(fp, "-%"T_ARTNUM_PFMT, i);

			} else if (!flag) {
				flag = TRUE;
				if (group->newsrc.xmin > 1) {
					fprintf(fp, "1");

					if (group->newsrc.xmin > 2)
						fprintf(fp, "-%"T_ARTNUM_PFMT, group->newsrc.xmin - 1);
				}
			}
			if (group->newsrc.xmax == i)
				break;

			++i;
		}
	}

	if (!flag && group->newsrc.xmin > 1) {
		fprintf(fp, "1");

		if (group->newsrc.xmin > 2)
			fprintf(fp, "-%"T_ARTNUM_PFMT, group->newsrc.xmin - 1);

#ifdef DEBUG
		if (debug & DEBUG_NEWSRC)
			debug_print_comment("print_bitmap_seq(): !flag && group->newsrc.xmin > 1");
#endif /* DEBUG */
	}

	fprintf(fp, "\n");
	fflush(fp);
}


/*
 * rewrite .newsrc and position group at specified position
 */
t_bool
pos_group_in_newsrc(
	const struct t_group *group,
	int pos)
{
	FILE *fp_in = NULL, *fp_out = NULL;
	FILE *fp_sub = NULL, *fp_unsub = NULL;
	char *newsgroup = NULL;
	char *line;
	char filename[PATH_LEN];
	char *sub = NULL;
	char *unsub = NULL;
	int subscribed_pos = 1;
	int err, n;
	size_t len;
	t_bool found = FALSE;
	t_bool newnewsrc_created = FALSE;
	t_bool option_line = FALSE;
	t_bool repositioned = FALSE;
	t_bool ret_code = FALSE;
	t_bool sub_created = FALSE;
	t_bool unsub_created = FALSE;
	t_bool fs_error = FALSE;

	if (no_write || (fp_in = fopen(newsrc, "r")) == NULL)
		goto rewrite_group_done;

	if ((fp_out = fopen(newnewsrc, "w")) == NULL)
		goto rewrite_group_done;

	newnewsrc_created = TRUE;

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	if (newsrc_mode) {
#	ifdef HAVE_FCHMOD
		fchmod(fileno(fp_out), newsrc_mode);
#	else
#		ifdef HAVE_CHMOD
		chmod(newnewsrc, newsrc_mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	joinpath(filename, sizeof(filename), tmpdir, ".subrc");
	if ((n = snprintf(NULL, 0, "%s.%ld", filename, (long) process_id)) < 0)
		goto rewrite_group_done;

	len = (size_t) n + 1;
	sub = my_malloc(len);
	if (snprintf(sub, len, "%s.%ld", filename, (long) process_id) != n)
		goto rewrite_group_done;

	joinpath(filename, sizeof(filename), tmpdir, ".unsubrc");
	if ((n = snprintf(NULL, 0, "%s.%ld", filename, (long) process_id)) < 0)
		goto rewrite_group_done;

	len = (size_t) n + 1;
	unsub = my_malloc(len);
	if (snprintf(unsub, len, "%s.%ld", filename, (long) process_id) != n)
		goto rewrite_group_done;

	if ((fp_sub = fopen(sub, "w")) == NULL)
		goto rewrite_group_done;

	sub_created = TRUE;

	if ((fp_unsub = fopen(unsub, "w")) == NULL)
		goto rewrite_group_done;

	unsub_created = TRUE;

	/*
	 * split newsrc into subscribed and unsubscribed to files
	 */
	len = strlen(group->name);

	while ((line = tin_fgets(fp_in, FALSE)) != NULL) {
		if (STRNCMPEQ(group->name, line, len) && line[len] == SUBSCRIBED) {
			FreeIfNeeded(newsgroup);
			newsgroup = my_strdup(line);		/* Take a copy of this line */
			found = TRUE;
			continue;
		} else if (strchr(line, SUBSCRIBED) != NULL) {
			write_newsrc_line(fp_sub, line);
		} else if (strchr(line, UNSUBSCRIBED) != NULL) {
			write_newsrc_line(fp_unsub, line);
		} else {								/* options line at beginning of .newsrc */
			fprintf(fp_sub, "%s\n", line);
			option_line = TRUE;
		}
	}

	if ((err = ferror(fp_sub)) || fclose(fp_sub)) {
		error_message(2, _(txt_filesystem_full), newsrc);
		if (err) {
			clearerr(fp_sub);
			fclose(fp_sub);
		}
		fs_error = TRUE;
	}
	if ((err = ferror(fp_unsub)) || fclose(fp_unsub)) {
		if (!fs_error) /* avoid repeatd error message */
			error_message(2, _(txt_filesystem_full), newsrc);
		if (err) {
			clearerr(fp_unsub);
			fclose(fp_unsub);
		}
		fs_error = TRUE;
	}
	fp_sub = fp_unsub = NULL;

	if (fs_error)
		goto rewrite_group_done;

	fclose(fp_in);
	fp_in = NULL;

	/*
	 * The group to be moved cannot be found, so give up now
	 */
	if (!found)
		goto rewrite_group_done;

	/*
	 * write subscribed groups & repositioned group to newnewsrc
	 */
	if ((fp_sub = fopen(sub, "r")) == NULL)
		goto rewrite_group_done;

	while ((line = tin_fgets(fp_sub, FALSE)) != NULL) {
		if (option_line) {
			if (strchr(line, SUBSCRIBED) == NULL && strchr(line, UNSUBSCRIBED) == NULL) {
				fprintf(fp_out, "%s\n", line);
				continue;
			} else
				option_line = FALSE;
		}

		if (pos == subscribed_pos) {
			write_newsrc_line(fp_out, newsgroup);
			repositioned = TRUE;
		}

		fprintf(fp_out, "%s\n", line);

		++subscribed_pos;
	}

	if (!repositioned)
		write_newsrc_line(fp_out, newsgroup);

	/*
	 * append unsubscribed groups file to newnewsrc
	 */
	if ((fp_unsub = fopen(unsub, "r")) == NULL)
		goto rewrite_group_done;

	while ((line = tin_fgets(fp_unsub, FALSE)) != NULL)
		fprintf(fp_out, "%s\n", line);

	/*
	 * Try and cleanly close out the newnewsrc file
	 */
	if ((err = ferror(fp_out)) || fclose(fp_out)) {
		error_message(2, _(txt_filesystem_full), newsrc);
		if (err) {
			clearerr(fp_out);
			fclose(fp_out);
		}
	} else {
		rename_file(newnewsrc, newsrc);
		ret_code = TRUE;
	}
	fp_out = NULL;
	newnewsrc_created = FALSE;

rewrite_group_done:
	if (fp_in != NULL)
		fclose(fp_in);

	if (fp_out != NULL)
		fclose(fp_out);

	if (fp_sub != NULL)
		fclose(fp_sub);

	if (fp_unsub != NULL)
		fclose(fp_unsub);

	if (newnewsrc_created)
		unlink(newnewsrc);

	if (sub_created)
		unlink(sub);

	if (unsub_created)
		unlink(unsub);

	FreeIfNeeded(newsgroup);
	FreeIfNeeded(sub);
	FreeIfNeeded(unsub);

	return ret_code;
}


/*
 * catchup all groups in .newsrc
 */
void
catchup_newsrc_file(
	void)
{
	int i;
	struct t_group *group;

	for (i = 0; i < selmenu.max; i++) {
		group = &active[my_group[i]];
		group->newsrc.present = TRUE;
		FreeAndNull(group->newsrc.xbitmap);
		if (group->xmax > group->newsrc.xmax)
			group->newsrc.xmax = group->xmax;
		group->newsrc.xmin = group->newsrc.xmax + 1;
		group->newsrc.num_unread = 0;
		group->newsrc.xbitlen = 0;
	}
}


/*
 * Break down a line of .newsrc file
 * The sequence information [ eg; 1-3,10,12 ] is returned, line is truncated to
 * just the group name and the subscription flag is copied to sub.
 */
static char *
parse_newsrc_line(
	char *line,
	int *sub)
{
	char *ptr, *tmp;

	*sub = UNSUBSCRIBED;				/* Default to no entry */

	if ((ptr = strpbrk(line, "!:")) == NULL)			/* space|SUBSCRIBED|UNSUBSCRIBED */
		return NULL;

	*sub = *ptr;						/* Save the subscription status */
	tmp = ptr;							/* Keep this blank for later */
	*(ptr++) = '\0';					/* Terminate the group name */

#if 0
	if (ptr == NULL)					/* No seq info, so return a blank */
		return tmp;
#endif /* 0 */

	if ((ptr = strpbrk(ptr, " \t")) == NULL)
		return tmp;

	return (ptr + 1);	/* Return pointer to sequence info. At worst this will be \0 */
}


/*
 * expand group->newsrc information if group->xmax is larger than
 * group->newsrc.xmax or min is smaller than group->newsrc.xmin.
 */
void
expand_bitmap(
	struct t_group *group,
	t_artnum min)
{
	t_artnum bitlen;
	t_artnum first;
	t_artnum tmp;
	t_artnum max;
	t_bool need_full_copy = FALSE;

	/* calculate new max */
	if (group->newsrc.xmax > group->xmax)
		max = group->newsrc.xmax;
	else
		max = group->xmax;

	/* adjust min */
	if (!min)
		min = group->newsrc.xmin;

	/* calculate first */
	if (min >= group->newsrc.xmin)
		first = group->newsrc.xmin;
	else
		first = group->newsrc.xmin - ((group->newsrc.xmin - min + (NBITS - 1)) & ~(NBITS - 1));

	/* adjust first */
	if (group->newsrc.xmax < first - 1)
		first = first - ((first - (group->newsrc.xmax + 1) + (NBITS - 1)) & ~(NBITS - 1));

	/* check first */
	if (first < 1) {
		need_full_copy = TRUE;
		first = 1;
	}

	bitlen = max - first + 1;

	if (bitlen <= 0) {
		bitlen = 0;
		FreeIfNeeded(group->newsrc.xbitmap);
		group->newsrc.xbitmap = (t_bitmap *) 0;
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC)
			debug_print_comment("expand_bitmap: group->newsrc.bitlen == 0");
#endif /* DEBUG */
	} else if (group->newsrc.xbitmap == NULL) {
		group->newsrc.xbitmap = my_malloc(BITS_TO_BYTES(bitlen));
		if (group->newsrc.xmin > first)
			NSETRNG0(group->newsrc.xbitmap, T_ARTNUM_CONST(0), group->newsrc.xmin - first - T_ARTNUM_CONST(1));
		if (bitlen > group->newsrc.xmin - first)
			NSETRNG1(group->newsrc.xbitmap, group->newsrc.xmin - first, bitlen - 1);
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC)
			debug_print_comment("expand_bitmap: group->newsrc.xbitmap == NULL");
#endif /* DEBUG */
	} else if (need_full_copy) {
		t_bitmap *newbitmap = my_malloc(BITS_TO_BYTES(bitlen));

		/* Copy over old bitmap */
		/* TODO: change to use shift */
		for (tmp = group->newsrc.xmin; tmp <= group->newsrc.xmax; tmp++) {
			if (NTEST(group->newsrc.xbitmap, tmp - group->newsrc.xmin) == ART_READ)
				NSET0(newbitmap, tmp - first);
			else
				NSET1(newbitmap, tmp - first);
		}

		/* Mark earlier articles as read, updating num_unread */

		if (first < group->newsrc.xmin) {
			NSETRNG0(newbitmap, T_ARTNUM_CONST(0), group->newsrc.xmin - first - T_ARTNUM_CONST(1));
		}

		for (tmp = group->newsrc.xmin; tmp < min; tmp++) {
			if (NTEST(newbitmap, tmp - first) != ART_READ) {
				NSET0(newbitmap, tmp - first);
				if (group->newsrc.num_unread)
					group->newsrc.num_unread--;
			}
		}

		/* Mark high numbered articles as unread */

		if (group->newsrc.xmin - first + group->newsrc.xbitlen < bitlen) {
			tmp = group->newsrc.xmin - first + group->newsrc.xbitlen;
			NSETRNG1(newbitmap, tmp, bitlen - 1);
		}

		free(group->newsrc.xbitmap);
		group->newsrc.xbitmap = newbitmap;
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC)
			debug_print_comment("expand_bitmap: group->newsrc.bitlen != (group->max-group->min)+1 and need full copy");
#endif /* DEBUG */
	} else if (max != group->newsrc.xmax || first != group->newsrc.xmin) {
		t_bitmap *newbitmap = my_malloc(BITS_TO_BYTES(bitlen));

		/* Copy over old bitmap */

		assert((group->newsrc.xmin - first) / NBITS + BITS_TO_BYTES(group->newsrc.xbitlen) <= BITS_TO_BYTES(bitlen));

		memcpy(newbitmap + (group->newsrc.xmin - first) / NBITS, group->newsrc.xbitmap, BITS_TO_BYTES(group->newsrc.xbitlen));

		/* Mark earlier articles as read, updating num_unread */

		if (first < group->newsrc.xmin) {
			NSETRNG0(newbitmap, T_ARTNUM_CONST(0), group->newsrc.xmin - first - T_ARTNUM_CONST(1));
		}

		for (tmp = group->newsrc.xmin; tmp < min; tmp++) {
			if (NTEST(newbitmap, tmp - first) != ART_READ) {
				NSET0(newbitmap, tmp - first);
				if (group->newsrc.num_unread)
					group->newsrc.num_unread--;
			}
		}

		/* Mark high numbered articles as unread */

		if (group->newsrc.xmin - first + group->newsrc.xbitlen < bitlen) {
			tmp = group->newsrc.xmin - first + group->newsrc.xbitlen;
			NSETRNG1(newbitmap, tmp, bitlen - 1);
		}

		free(group->newsrc.xbitmap);
		group->newsrc.xbitmap = newbitmap;
#ifdef DEBUG
		if (debug & DEBUG_NEWSRC)
			debug_print_comment("expand_bitmap: group->newsrc.bitlen != (group->max-group->min)+1");
#endif /* DEBUG */
	}
	group->newsrc.xmin = first;
	if (group->newsrc.xmax < max)
		group->newsrc.num_unread += max - group->newsrc.xmax;
	group->newsrc.xmax = max;
	group->newsrc.xbitlen = bitlen;
	group->newsrc.present = TRUE;
}


void
art_mark(
	struct t_group *group,
	struct t_article *art,
	int flag)
{
	if (art == NULL)
		return;

	switch (flag) {
		case ART_READ:
			if (group != NULL) {
				if (art->artnum >= group->newsrc.xmin && art->artnum <= group->newsrc.xmax)
					NSET0(group->newsrc.xbitmap, art->artnum - group->newsrc.xmin);
#ifdef DEBUG
				if (debug & DEBUG_NEWSRC)
					debug_print_bitmap(group, art);
#endif /* DEBUG */
			}
			if ((art->status == ART_UNREAD) || (art->status == ART_WILL_RETURN)) {
				art_mark_xref_read(art);

				if (group != NULL) {
					if (group->newsrc.num_unread)
						group->newsrc.num_unread--;

					if (group->attribute && group->attribute->show_only_unread_arts)
						art->keep_in_base = TRUE;
				}

				art->status = ART_READ;
			}
			break;

		case ART_UNREAD:
		case ART_WILL_RETURN:
			if (art->status == ART_READ) {
				if (group != NULL) {
					group->newsrc.num_unread++;

					if (group->attribute && group->attribute->show_only_unread_arts)
						art->keep_in_base = FALSE;
				}

				art->status = flag;
			}
			if (group != NULL) {
				if (art->artnum < group->newsrc.xmin)
					expand_bitmap(group, art->artnum);
				else {
					NSET1(group->newsrc.xbitmap, art->artnum - group->newsrc.xmin);
#ifdef DEBUG
					if (debug & DEBUG_NEWSRC)
						debug_print_bitmap(group, art);
#endif /* DEBUG */
				}
			}
			break;

		default:
			break;
	}
}


void
set_default_bitmap(
	struct t_group *group)
{
	if (group != NULL) {
		group->newsrc.num_unread = 0;
		group->newsrc.present = FALSE;

		FreeIfNeeded(group->newsrc.xbitmap);

		group->newsrc.xbitmap = (t_bitmap *) 0;
		group->newsrc.xbitlen = 0;
		if (group->xmin > 0)
			group->newsrc.xmin = group->xmin;
		else
			group->newsrc.xmin = 1;
		group->newsrc.xmax = group->newsrc.xmin - 1;
	}
}
