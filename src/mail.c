/*
 *  Project   : tin - a Usenet reader
 *  Module    : mail.c
 *  Author    : I. Lea
 *  Created   : 1992-10-02
 *  Updated   : 2024-03-21
 *  Notes     : Mail handling routines for creating pseudo newsgroups
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
#ifdef NNTP_ABLE
#	ifndef TNNTP_H
#		include "tnntp.h"
#	endif /* !TNNTP_H */
#endif /* NNTP_ABLE */
/*
 * local prototypes
 */
static FILE *open_newsgroups_fp(void);
static void read_groups_descriptions(FILE *fp, FILE *fp_save);
static void read_newsgroups_file(t_bool verb);
#ifdef HAVE_MH_MAIL_HANDLING
	static FILE *open_mail_active_fp(const char *mode);
	static FILE *open_mailgroups_fp(void);
	static void read_mailgroups_file(t_bool verb);
#endif /* HAVE_MH_MAIL_HANDLING */


#ifdef HAVE_MH_MAIL_HANDLING
/*
 * Open the mail active file locally
 */
static FILE *
open_mail_active_fp(
	const char *mode)
{
	return fopen(mail_active_file, mode);
}


/*
 * Open mail groups description file locally
 */
static FILE *
open_mailgroups_fp(
	void)
{
	return fopen(mailgroups_file, "r");
}


/*
 * Load the mail active file into active[]
 */
void
read_mail_active_file(
	void)
{
	FILE *fp;
	char *buf;
	char my_spooldir[PATH_LEN];
	char buf2[PATH_LEN];
	t_artnum min, max;
	struct t_group *ptr;

	if (!batch_mode)
		wait_message(0, _(txt_reading_mail_active_file));

	/*
	 * Open the mail active file
	 */
	if ((fp = open_mail_active_fp("r")) == NULL) {
		if (cmd_line)
			my_fputc('\n', stderr);

		if (!created_rcdir) /* no error on first start */
			error_message(2, _(txt_cannot_open), mail_active_file);
		/*
		 * TODO: do an autoscan of maildir, create & reopen?
		 */
		write_mail_active_file();
		return;
	}

	while ((buf = tin_fgets(fp, FALSE))) {
		if (!parse_active_line(buf, &max, &min, my_spooldir) || *buf == '\0')
			continue;

		/*
		 * Update mailgroup info
		 */
		if ((ptr = group_find(buf, FALSE)) != NULL) {
			if (strcmp(ptr->spooldir, my_spooldir) != 0) {
				free(ptr->spooldir);
				strfpath(my_spooldir, buf2, sizeof(buf2) - 1, ptr, FALSE);
				ptr->spooldir = my_strdup(buf2);
			}
			ptr->xmax = max;
			ptr->xmin = min;
			continue;
		}

		/*
		 * Load mailgroup into group hash table
		 */
		if ((ptr = group_add(buf)) == NULL)
			continue;

		/*
		 * Load group info. TODO: integrate with active_add()
		 */
		strfpath(my_spooldir, buf2, sizeof(buf2) - 1, ptr, FALSE);
		ptr->spooldir = my_strdup(buf2);
		group_get_art_info(ptr->spooldir, buf, GROUP_TYPE_MAIL, &ptr->count, &ptr->xmax, &ptr->xmin);
		ptr->aliasedto = NULL;
		ptr->description = NULL;
		ptr->moderated = 'y';
		ptr->type = GROUP_TYPE_MAIL;
		ptr->inrange = FALSE;
		ptr->read_during_session = FALSE;
		ptr->art_was_posted = FALSE;
		ptr->subscribed = FALSE;		/* not in my_group[] yet */
		ptr->newgroup = FALSE;
		ptr->bogus = FALSE;
		ptr->next = -1;			/* hash chaining */
		ptr->newsrc.xbitmap = (t_bitmap *) 0;
		ptr->attribute = (struct t_attribute *) 0;
		ptr->glob_filter = &glob_filter;
		set_default_bitmap(ptr);
	}
	fclose(fp);

	if (!batch_mode)
		my_fputs("\n", stdout);
}


/*
 * Write out mailgroups from active[] to ~/.tin/active.mail
 */
void
write_mail_active_file(
	void)
{
	FILE *fp;
	char *file_tmp;
	char group_path[PATH_LEN];
	int i;
	struct t_group *group;

	if (no_write && file_size(mail_active_file) != -1L)
		return;

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(mail_active_file);

	if (!backup_file(mail_active_file, file_tmp)) {
		error_message(2, _(txt_filesystem_full_backup), mail_active_file);
		/* free memory for tmp-filename */
		free(file_tmp);
		return;
	}

	print_active_head(mail_active_file);

	if ((fp = open_mail_active_fp("a+")) != NULL) {
		for_each_group(i) {
			group = &active[i];
			if (group->type == GROUP_TYPE_MAIL) {
				make_base_group_path(group->spooldir, group->name, group_path, sizeof(group_path));
				find_art_max_min(group_path, &group->xmax, &group->xmin);
				print_group_line(fp, group->name, group->xmax, group->xmin, group->spooldir);
			}
		}
		if ((i = ferror(fp)) || fclose(fp)) {
			error_message(2, _(txt_filesystem_full), mail_active_file);
			if (i) {
				clearerr(fp);
				fclose(fp);
			}
			rename_file(file_tmp, mail_active_file);
		} else
			unlink(file_tmp);
	}

	/* free memory for tmp-filename */
	free(file_tmp);
}


/*
 * Load the text description from ~/.tin/mailgroups for each mail group into
 * the active[] array.
 */
static void
read_mailgroups_file(
	t_bool verb)
{
	FILE *fp;

	if ((fp = open_mailgroups_fp()) != NULL) {
		if (!batch_mode && verb)
			wait_message(0, _(txt_reading_mailgroups_file));

		read_groups_descriptions(fp, (FILE *) 0);

		fclose(fp);

		if (!batch_mode && verb) {
			my_fputs("\n", stdout);
			cursoroff();
		}
	}
}
#endif /* HAVE_MH_MAIL_HANDLING */


/*
 * If reading via NNTP the newsgroups file will be saved to
 * ~/.tin/$NNTPSERVER/newsgroups so that any subsequent rereads on the
 * active file will not have to waste net bandwidth and the local copy
 * of the newsgroups file can be accessed.
 *
 * in the newsrc_active case (-n cmd-line switch) we use "LIST NEWSGROUPS grp"
 * instead of "LIST NEWSGROUPS" if we have just a few groups in the newsrc,
 * due to pipelining the code is a bit complex.
 */
static FILE *
open_newsgroups_fp(
	void)
{
#ifdef NNTP_ABLE
	FILE *result;
	static int no_more_wildmat = 0;

	if (read_news_via_nntp && !read_saved_news) {
		if (read_local_newsgroups_file) {
			if ((result = fopen(local_newsgroups_file, "r")) != NULL) {
				struct stat buf;

#	ifdef DEBUG
				if ((debug & DEBUG_NNTP) && verbose > 1)
					debug_print_file("NNTP", "open_newsgroups_fp Using local copy of newsgroups file");
#	endif /* DEBUG */

				if (!fstat(fileno(result), &buf)) {
					if (buf.st_size > 0)
						return result;
				}
				fclose(result);
				unlink(local_newsgroups_file);
			}
		}
		/*
		 * TODO: test me, find a useful limit,
		 *       optimize more than n groups (e.g. 5) of the same
		 *       subhierarchy to a wildmat?
		 */
		if (((nntp_caps.type == CAPABILITIES && nntp_caps.list_newsgroups) || nntp_caps.type != CAPABILITIES) && newsrc_active && !list_active && !no_more_wildmat && (PIPELINE_LIMIT > MAX(1, num_active))) {
			char *ptr;
			char buff[NNTP_STRLEN];
			char line[NNTP_STRLEN];
			char file[PATH_LEN];
			char serverdir[PATH_LEN];
			struct t_group *group;
			int resp, i, j = 0;

			if (nntp_tcp_port != IPPORT_NNTP)
				snprintf(file, sizeof(file), "%s:%u", nntp_server, nntp_tcp_port);
			else
				STRCPY(file, quote_space_to_dash(nntp_server));

			joinpath(serverdir, sizeof(serverdir), rcdir, file);
			joinpath(file, sizeof(file), serverdir, NEWSGROUPS_FILE".tmp");
			*buff = '\0';
			if ((result = fopen(file, "w")) != NULL) {
				for_each_group(i) {
					if ((group = group_find(active[i].name, FALSE)) != NULL) {
						if (group->type == GROUP_TYPE_NEWS) {
							if (nntp_caps.type == CAPABILITIES && nntp_caps.list_newsgroups) {
								if (*buff) {
									if (strlen(buff) + strlen(active[i].name) + 1 < NNTP_GRPLEN) {
										snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), ",%s", active[i].name);
										continue;
									} else {
										put_server(buff, FALSE);
										*buff = '\0';
										j++;
									}
								}
								if (!*buff) {
									snprintf(buff, sizeof(buff), "LIST NEWSGROUPS %s", active[i].name);
									continue;
								}
							} else
								snprintf(buff, sizeof(buff), "LIST NEWSGROUPS %s", active[i].name);
#		ifdef DISABLE_PIPELINING
							if ((resp = new_nntp_command(buff, OK_GROUPS, line, sizeof(line))) != OK_GROUPS) {
								no_more_wildmat = resp;
								*buff = '\0';
								break;
							}
							while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#			if defined(DEBUG) && defined(NNTP_ABLE)
								if ((debug & DEBUG_NNTP) && verbose)
									debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#			endif /* DEBUG && NNTP_ABLE */
								fprintf(result, "%s\n", str_trim(ptr));
							}
#			if defined(DEBUG) && defined(NNTP_ABLE)
							if ((debug & DEBUG_NNTP) && !verbose)
								debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#			endif /* DEBUG && NNTP_ABLE */

#		else
							put_server(buff, FALSE);
							*buff = '\0';
							j++;
#		endif /* DISABLE_PIPELINING */
						}
					}
				}
				if (*buff) {
					put_server(buff, FALSE);
					j++;
				}
#		ifndef DISABLE_PIPELINING
				while (j--) {
					if ((resp = get_only_respcode(line, sizeof(line))) != OK_GROUPS) {
						if (!no_more_wildmat)
							no_more_wildmat = resp;
						continue;
					}
					while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#			if defined(DEBUG) && defined(NNTP_ABLE)
						if ((debug & DEBUG_NNTP) && verbose)
							debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#			endif /* DEBUG && NNTP_ABLE */
						fprintf(result, "%s\n", str_trim(ptr));
					}
#			if defined(DEBUG) && defined(NNTP_ABLE)
					if ((debug & DEBUG_NNTP) && !verbose)
						debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#			endif /* DEBUG && NNTP_ABLE */
				}
				/* TODO: add 483 (RFC 3977) support */
				if (no_more_wildmat == ERR_NOAUTH || no_more_wildmat == NEED_AUTHINFO) {
					if (!authenticate(nntp_server, userid, FALSE)) {
						fclose(result);
						tin_done(EXIT_FAILURE, _(txt_auth_failed), nntp_caps.type == CAPABILITIES ? ERR_AUTHFAIL : ERR_ACCESS);
					}
				}
#		endif /* !DISABLE_PIPELINING */
				fclose(result);
				result = fopen(file, "r");
				unlink(file); /* unlink on close */
			}

			if (result != NULL) {
				if (!no_more_wildmat)
					return result;
				else /* AUTH request while pipelining or some error */
					fclose(result);
			}
		}
		return (nntp_command("LIST NEWSGROUPS", OK_GROUPS, NULL, 0));
	}
#endif /* NNTP_ABLE */
	return fopen(newsgroups_file, "r");
}


/*
 * Load the text description from NEWSLIBDIR/newsgroups for each group into the
 * active[] array. Save a copy locally if reading via NNTP to save bandwidth.
 */
static void
read_newsgroups_file(
	t_bool verb)
{
	FILE *fp;
	FILE *fp_save = (FILE *) 0;

	if ((fp = open_newsgroups_fp()) != NULL) {
		if (!batch_mode && verb)
			wait_message(0, _(txt_reading_newsgroups_file));

		if (read_news_via_nntp && !no_write && !read_local_newsgroups_file)
			fp_save = fopen(local_newsgroups_file, "w");

		read_groups_descriptions(fp, fp_save);

		if (fp_save != NULL) {
			fclose(fp_save);
			read_local_newsgroups_file = TRUE;
		}

		TIN_FCLOSE(fp);

		if (!batch_mode && verb) {
			my_fputs("\n", stdout);
			cursoroff();
		}
	}
}


/*
 * read group descriptions for news (and mailgroups)
 */
void
read_descriptions(
	t_bool verb)
{
#ifdef HAVE_MH_MAIL_HANDLING
	read_mailgroups_file(verb);
#endif /* HAVE_MH_MAIL_HANDLING */
	read_newsgroups_file(verb);
}


/*
 * Read groups descriptions from opened file & make local backup copy
 * of all groups if reading groups of type GROUP_TYPE_NEWS.
 * Aborting this early won't have any adverse affects, just some missing
 * descriptions.
 */
static void
read_groups_descriptions(
	FILE *fp,
	FILE *fp_save)
{
	char *p, *q, *ptr;
	char *groupname = NULL;
	int count = 0;
	size_t space = 0;
	struct t_group *group;

	while ((ptr = tin_fgets(fp, FALSE)) != NULL) {
#if defined(DEBUG) && defined(NNTP_ABLE)
		if ((debug & DEBUG_NNTP) && fp == FAKE_NNTP_FP && verbose)
			debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#endif /* DEBUG && NNTP_ABLE */
		if (*ptr == '#' || *ptr == '\0')
			continue;

		/*
		 * This was moved from below and simplified. We can't test here for
		 * the type of group being read, because that requires having found
		 * the group in the active file, and that truncates the local copy
		 * of the newsgroups file to only subscribed-to groups when tin is
		 * called with the "-q" option.
		 */
		if ((fp_save != NULL) && read_news_via_nntp)
			fprintf(fp_save, "%s\n", str_trim(ptr));

		if (!space) { /* initial malloc */
			space = strlen(ptr) + 1;
			groupname = my_malloc(space);
		} else {
			while (space < strlen(ptr) + 1) { /* realloc needed? */
				space <<= 1; /* double size */
				groupname = my_realloc(groupname, space);
			}
		}

		for (p = ptr, q = groupname; *p && *p != ' ' && *p != '\t'; p++, q++)
			*q = *p;

		*q = '\0';

		while (*p == '\t' || *p == ' ')
			p++;

		group = group_find(groupname, FALSE);

		if (group != NULL && group->description == NULL) {
			char *r;
			size_t r_len;

			q = p;
			while ((q = strchr(q, '\t')) != NULL)
				*q = ' ';

			r = my_strdup(p);
			r_len = strlen(r);
			/*
			 * Protect against invalid character sequences.
			 */
			process_charsets(&r, &r_len, "UTF-8", tinrc.mm_local_charset, FALSE);
			group->description = convert_to_printable(r, FALSE);
		}

		if (++count % 100 == 0)
			spin_cursor();
	}
#	if defined(DEBUG) && defined(NNTP_ABLE)
	if ((debug & DEBUG_NNTP) && !verbose)
		debug_print_file("NNTP", "<<<%s%s", logtime(), txt_log_data_hidden);
#	endif /* DEBUG && NNTP_ABLE */
	FreeIfNeeded(groupname);
}


void
print_active_head(
	const char *active_file)
{
	FILE *fp;

	if (no_write && file_size(active_file) != -1L)
		return;

	if ((fp = fopen(active_file, "w")) != NULL) {
		fprintf(fp, "%s", _(txt_mail_save_active_head));
		fclose(fp);
	}
}


void
find_art_max_min(
	const char *group_path,
	t_artnum *art_max,
	t_artnum *art_min)
{
	DIR *dir;
	DIR_BUF *direntry;
	t_artnum art_num;

	*art_min = *art_max = T_ARTNUM_CONST(0);

	if ((dir = opendir(group_path)) != NULL) {
		while ((direntry = readdir(dir)) != NULL) {
			art_num = atoartnum(direntry->d_name);
			if (art_num >= T_ARTNUM_CONST(1)) {
				if (art_num > *art_max) {
					*art_max = art_num;
					if (*art_min == T_ARTNUM_CONST(0))
						*art_min = art_num;
				} else if (art_num < *art_min)
					*art_min = art_num;
			}
		}
		CLOSEDIR(dir);
	}
	if (*art_min == T_ARTNUM_CONST(0))
		*art_min = T_ARTNUM_CONST(1);
}


void
print_group_line(
	FILE *fp,
	const char *group_name,
	t_artnum art_max,
	t_artnum art_min,
	const char *base_dir)
{
	fprintf(fp, "%s %05"T_ARTNUM_PFMT" %05"T_ARTNUM_PFMT" %s\n",
		group_name, art_max, art_min, base_dir);
}


void
grp_del_mail_art(
	struct t_article *article)
{
	if (article->delete_it)
		info_message(_(txt_art_undeleted));
	else
		info_message(_(txt_art_deleted));

	article->delete_it = bool_not(article->delete_it);
}


void
grp_del_mail_arts(
	struct t_group *group)
{
	char article_filename[PATH_LEN];
	char group_path[PATH_LEN];
	char artnum[LEN];
	int i;
	struct t_article *article;

	if (group->type == GROUP_TYPE_MAIL || group->type == GROUP_TYPE_SAVE) {
		/*
		 * at least for GROUP_TYPE_SAVE a wait is annoying - nuke the message?
		 */
		wait_message(0, (group->type == GROUP_TYPE_MAIL) ? _(txt_processing_mail_arts) : _(txt_processing_saved_arts));
		cursoroff();
		make_base_group_path(group->spooldir, group->name, group_path, sizeof(group_path));
		for_each_art(i) {
			article = &arts[i];
			if (article->delete_it) {
				snprintf(artnum, sizeof(artnum), "%"T_ARTNUM_PFMT, article->artnum);
				joinpath(article_filename, sizeof(article_filename), group_path, artnum);
				unlink(article_filename);
				article->thread = ART_EXPIRED;
			}
		}

		/*
		 * current tin's build_references() is changed to free msgid and
		 * refs, therefore we cannot call write_overview after it. NovFile
		 * will update at next time.
		 */
	}
}


t_bool
art_edit(
	struct t_group *group,
	struct t_article *article)
{
	char article_filename[PATH_LEN];
	char temp_filename[PATH_LEN];
	char buf[PATH_LEN];

	/*
	 * Check if news / mail group
	 */
	if (group->type != GROUP_TYPE_MAIL)
		return FALSE;

	make_base_group_path(group->spooldir, group->name, temp_filename, sizeof(temp_filename));
	snprintf(buf, sizeof(buf), "%"T_ARTNUM_PFMT, article->artnum);
	joinpath(article_filename, sizeof(article_filename), temp_filename, buf);
	snprintf(buf, sizeof(buf), "%ld.art", (long) process_id);
	joinpath(temp_filename, sizeof(temp_filename), tmpdir, buf);

	if (!backup_file(article_filename, temp_filename))
		return FALSE;

	if (!invoke_editor(temp_filename, 1, group)) {
		unlink(temp_filename);
		return FALSE;
	}

	rename_file(temp_filename, article_filename);
	return TRUE;
}
