/*
 *  Project   : tin - a Usenet reader
 *  Module    : filter.c
 *  Author    : I. Lea
 *  Created   : 1992-12-28
 *  Updated   : 2004-09-04
 *  Notes     : Filter articles. Kill & auto selection are supported.
 *
 * Copyright (c) 1991-2004 Iain Lea <iain@bricbrac.de>
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
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */


#define IS_READ(i)	(arts[i].status == ART_READ)
#define IS_KILLED(i)	(arts[i].killed)
#define IS_SELECTED(i)	(arts[i].selected)

/*
 * SET_FILTER in group grp, current article arts[i], with rule ptr[j]
 *
 * filtering is now done this way:
 * a. set score for all articles and rules
 * b. check each article if the score is above or below the limit
 *
 * SET_FILTER is now somewhat shorter, as the real filtering is done
 * at the end of filter_articles()
 */

#define SET_FILTER(grp, i, j)	\
	if (ptr[j].score > 0) { \
		arts[i].score = (SCORE_MAX - ptr[j].score >= arts[i].score) ? \
		(arts[i].score + ptr[j].score) : SCORE_MAX ; \
	} else { \
		arts[i].score = (-SCORE_MAX - ptr[j].score <= arts[i].score) ? \
		(arts[i].score + ptr[j].score) : -SCORE_MAX ; }

/*
 * These will probably go away when filtering is rewritten
 * Easier access to hashed msgids. Note that in REFS(), y must be free()d
 * msgid is mandatory in an article and cannot be NULL
 */
#define MSGID(x)			(x->refptr->txt)
#define REFS(x,y)			((y = get_references(x->refptr->parent)) ? y : "")

/*
 * global filter array
 */
struct t_filters glob_filter = { 0, 0, (struct t_filter *) 0 };


/*
 * Local prototypes
 */
static int get_choice(int x, const char *help, const char *prompt, char *list[], int list_size);
static int set_filter_scope(struct t_group *group);
static struct t_filter_comment *add_filter_comment(struct t_filter_comment *ptr, char *text);
static struct t_filter_comment *free_filter_comment(struct t_filter_comment *ptr);
static struct t_filter_comment *copy_filter_comment(struct t_filter_comment *from, struct t_filter_comment *to);
static t_bool add_filter_rule(struct t_group *group, struct t_article *art, struct t_filter_rule *rule, t_bool quick_filter_rule);
static int test_regex(const char *string, char *regex, t_bool nocase, struct regex_cache *cache);
static void expand_filter_array(struct t_filters *ptr);
static void fmt_filter_menu_prompt(char *dest, size_t dest_len, const char *fmt_str, int len, const char *text);
static void free_filter_item(struct t_filter *ptr);
static void print_filter_menu(void);
static void set_filter(struct t_filter *ptr);
static void write_filter_array(FILE *fp, struct t_filters *ptr);
#if 0 /* currently unused */
	static FILE *open_xhdr_fp(char *header, long min, long max);
#endif /* 0 */


/*
 * Add one more entry to the filter-comment-list.
 * If ptr == NULL the list will be created.
 */
static struct t_filter_comment *
add_filter_comment(
	struct t_filter_comment *ptr,
	char *text)
{
	if (ptr == NULL) {
		ptr = my_malloc(sizeof(struct t_filter_comment));
		ptr->text = my_strdup(text);
		ptr->next = (struct t_filter_comment *) 0;
	} else
		ptr->next = add_filter_comment(ptr->next, text);

	return ptr;
}


/*
 * Free all entries in a filter-comment-list.
 * Set ptr to NULL and return it.
 */
static struct t_filter_comment *
free_filter_comment(
	struct t_filter_comment *ptr)
{
	struct t_filter_comment *tmp, *next;

	tmp = ptr;
	while (tmp != NULL) {
		next = tmp->next;
		free(tmp->text);
		free(tmp);
		tmp = next;
	}

	return tmp;
}


/*
 * Copy the filter-comment-list 'from' into the list 'to'.
 */
static struct t_filter_comment *
copy_filter_comment(
	struct t_filter_comment *from,
	struct t_filter_comment *to)
{
	if (from != NULL) {
		to = my_malloc(sizeof(struct t_filter_comment));
		to->text = my_strdup(from->text);
		to->next = copy_filter_comment(from->next, NULL);
	}

	return to;
}


static void
expand_filter_array(
	struct t_filters *ptr)
{
	int num;
	size_t block;

	num = ++ptr->max;

	block = num * sizeof(struct t_filter);

	if (num == 1)	/* allocate */
		ptr->filter = my_malloc(block);
	else	/* reallocate */
		ptr->filter = my_realloc(ptr->filter, block);
}


/*
 * Looks for a matching filter hit (wildmat or pcre regex) in the supplied string
 * If the cache is not yet initialised, compile and optimise the regex
 * Returns 1 if we hit the rule
 * Returns 0 if we had no match
 * In case of error prints an error message and returns -1
 */
static int
test_regex(
	const char *string,
	char *regex,
	t_bool nocase,
	struct regex_cache *cache)
{
	int regex_errpos;

	if (!tinrc.wildcard) {
		if (wildmat(string, regex, nocase))
			return 1;
	} else {
		if (!cache->re)
			compile_regex(regex, cache, (nocase ? PCRE_CASELESS : 0));
		if (cache->re) {
			regex_errpos = pcre_exec(cache->re, cache->extra, string, strlen(string), 0, 0, NULL, 0);
			if (regex_errpos >= 0)
				return 1;
			else if (regex_errpos != PCRE_ERROR_NOMATCH) {
				error_message(_(txt_pcre_error_num), regex_errpos);
				return -1;
			}
		}
	}
	return 0;
}


/*
 * set_filter() initialises a struct t_filter with default values
 */
static void
set_filter(
	struct t_filter *ptr)
{
	if (ptr != NULL) {
		ptr->comment = (struct t_filter_comment *) 0;
		ptr->scope = NULL;
		ptr->inscope = TRUE;
		ptr->icase = FALSE;
		ptr->fullref = FILTER_MSGID;
		ptr->subj = NULL;
		ptr->from = NULL;
		ptr->msgid = NULL;
		ptr->lines_cmp = FILTER_LINES_NO;
		ptr->lines_num = 0;
		ptr->gnksa_cmp = FILTER_LINES_NO;
		ptr->gnksa_num = 0;
		ptr->score = 0;
		ptr->xref = NULL;
		ptr->time = (time_t) 0;
		ptr->next = (struct t_filter *) 0;
	}
}


/*
 * free_filter_item() frees all filter data (char *)
 */
static void
free_filter_item(
	struct t_filter *ptr)
{
	ptr->comment = free_filter_comment(ptr->comment);
	FreeAndNull(ptr->scope);
	FreeAndNull(ptr->subj);
	FreeAndNull(ptr->from);
	FreeAndNull(ptr->msgid);
	FreeAndNull(ptr->xref);
}


/*
 * free_filter_array() frees t_filter structs t_filters contains pointers to
 */
void
free_filter_array(
	struct t_filters *ptr)
{
	int i;

	if (ptr != NULL) {
		for (i = 0; i < ptr->num; i++)
			free_filter_item(ptr->filter + i);

		FreeAndNull(ptr->filter);
		ptr->num = 0;
		ptr->max = 0;
	}
}


/*
 * read ~/.tin/filter file contents into filter array
 */
t_bool
read_filter_file(
	const char *file)
{
	FILE *fp;
	char buf[HEADER_LEN];
	char scope[HEADER_LEN];
	char comment_line[LEN];	/* one line of comment */
	char subj[HEADER_LEN];
	char from[HEADER_LEN];
	char msgid[HEADER_LEN];
	char buffer[HEADER_LEN];
	char gnksa[HEADER_LEN];
	char xref[HEADER_LEN];
	char scbuf[PATH_LEN];
	int i = 0;
	int icase = 0;
	int score = 0;
	long secs = 0L;
	struct t_filter_comment *comment = NULL;
	struct t_filter *ptr = NULL;
	t_bool expired = FALSE;
	t_bool expired_time = FALSE;
	time_t current_secs = (time_t) 0;
	static t_bool first_read = TRUE;
	int upgrade = RC_CHECK;

	if ((fp = fopen(file, "r")) == NULL)
		return FALSE;

	if (!batch_mode || verbose)
		wait_message(0, _(txt_reading_filter_file));

	(void) time(&current_secs);

	/*
	 * Reset all filter arrays if doing a reread of the active file
	 */
	if (!first_read)
		free_filter_array(&glob_filter);

	while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
		if (*buf == '#' || *buf == '\n') {
			if (upgrade == RC_CHECK && first_read) {
				first_read = FALSE;
				upgrade = check_upgrade(buf, "# Filter file V", FILTER_VERSION);
				if (upgrade != RC_IGNORE)
					upgrade_prompt_quit(upgrade, FILTER_FILE);
				/* TODO: do something (more) useful here */
			}
			continue;
		}

		switch (tolower((unsigned char) buf[0])) {
			case 'c':
				if (match_integer(buf + 1, "ase=", &icase, 1)) {
					if (ptr && !expired_time)
						ptr[i].icase = (unsigned) icase;

					break;
				}
				if (match_string(buf + 1, "omment=", comment_line, sizeof(comment_line)))
					comment = add_filter_comment(comment, comment_line);
				break;

			case 'f':
				if (match_string(buf + 1, "rom=", from, sizeof(from))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].from != NULL) {
							/* merge with already read value */
							ptr[i].from = my_realloc(ptr[i].from, strlen(ptr[i].from) + strlen(from) + 2);
							strcat(ptr[i].from, "|");
							strcat(ptr[i].from, from);
						} else {
							FreeIfNeeded(ptr[i].from);
							ptr[i].from = my_strdup(from);
						}
					}
				}
				break;

			case 'g':
				if (match_string(buf + 1, "roup=", scope, sizeof(scope))) {
#ifdef DEBUG
					if (debug) {
						my_printf("group=[%s] num=[%d]\n", scope, glob_filter.num);
						my_flush();
					}
#endif /* DEBUG */
					if (glob_filter.num >= glob_filter.max)
						expand_filter_array(&glob_filter);

					ptr = glob_filter.filter;
					i = glob_filter.num++;
					set_filter(&ptr[i]);
					expired_time = FALSE;
					ptr[i].scope = my_strdup(scope);
					if (comment != NULL) {
						ptr[i].comment = copy_filter_comment(comment, ptr[i].comment);
						comment = free_filter_comment(comment);
					}
					subj[0] = '\0';
					from[0] = '\0';
					msgid[0] = '\0';
					buffer[0] = '\0';
					xref[0] = '\0';
					icase = 0;
					secs = 0L;
					break;
				}
				if (match_string(buf + 1, "nksa=", gnksa, sizeof(gnksa))) {
					if (ptr && !expired_time) {
						if (gnksa[0] == '<') {
							ptr[i].gnksa_cmp = FILTER_LINES_LT;
							ptr[i].gnksa_num = atoi(&gnksa[1]);
						} else if (gnksa[0] == '>') {
							ptr[i].gnksa_cmp = FILTER_LINES_GT;
							ptr[i].gnksa_num = atoi(&gnksa[1]);
						} else {
							ptr[i].gnksa_cmp = FILTER_LINES_EQ;
							ptr[i].gnksa_num = atoi(gnksa);
						}
					}
				}
			break;

			case 'l':
				if (match_string(buf + 1, "ines=", buffer, sizeof(buffer))) {
					if (ptr && !expired_time) {
						if (buffer[0] == '<') {
							ptr[i].lines_cmp = FILTER_LINES_LT;
							ptr[i].lines_num = atoi(&buffer[1]);
						} else if (buffer[0] == '>') {
							ptr[i].lines_cmp = FILTER_LINES_GT;
							ptr[i].lines_num = atoi(&buffer[1]);
						} else {
							ptr[i].lines_cmp = FILTER_LINES_EQ;
							ptr[i].lines_num = atoi(buffer);
						}
					}
				}
				break;

			case 'm':
				if (match_string(buf + 1, "sgid=", msgid, sizeof(msgid))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].msgid != NULL && ptr[i].fullref == FILTER_MSGID) {
							/* merge with already read value */
							ptr[i].msgid = my_realloc(ptr[i].msgid, strlen(ptr[i].msgid) + strlen(msgid) + 2);
							strcat(ptr[i].msgid, "|");
							strcat(ptr[i].msgid, msgid);
						} else {
							FreeIfNeeded(ptr[i].msgid);
							ptr[i].msgid = my_strdup(msgid);
							ptr[i].fullref = FILTER_MSGID;
						}
					}
					break;
				}
				if (match_string(buf + 1, "sgid_last=", msgid, sizeof(msgid))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].msgid != NULL && ptr[i].fullref == FILTER_MSGID_LAST) {
							/* merge with already read value */
							ptr[i].msgid = my_realloc(ptr[i].msgid, strlen(ptr[i].msgid) + strlen(msgid) + 2);
							strcat(ptr[i].msgid, "|");
							strcat(ptr[i].msgid, msgid);
						} else {
							FreeIfNeeded(ptr[i].msgid);
							ptr[i].msgid = my_strdup(msgid);
							ptr[i].fullref = FILTER_MSGID_LAST;
						}
					}
					break;
				}
				if (match_string(buf + 1, "sgid_only=", msgid, sizeof(msgid))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].msgid != NULL && ptr[i].fullref == FILTER_MSGID_ONLY) {
							/* merge with already read value */
							ptr[i].msgid = my_realloc(ptr[i].msgid, strlen(ptr[i].msgid) + strlen(msgid) + 2);
							strcat(ptr[i].msgid, "|");
							strcat(ptr[i].msgid, msgid);
						} else {
							FreeIfNeeded(ptr[i].msgid);
							ptr[i].msgid = my_strdup(msgid);
							ptr[i].fullref = FILTER_MSGID_ONLY;
						}
					}
				}
				break;

			case 'r':
				if (match_string(buf + 1, "efs_only=", msgid, sizeof(msgid))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].msgid != NULL && ptr[i].fullref == FILTER_REFS_ONLY) {
							/* merge with already read value */
							ptr[i].msgid = my_realloc(ptr[i].msgid, strlen(ptr[i].msgid) + strlen(msgid) + 2);
							strcat(ptr[i].msgid, "|");
							strcat(ptr[i].msgid, msgid);
						} else {
							FreeIfNeeded(ptr[i].msgid);
							ptr[i].msgid = my_strdup(msgid);
							ptr[i].fullref = FILTER_REFS_ONLY;
						}
					}
				}
				break;

			case 's':
				if (match_string(buf + 1, "ubj=", subj, sizeof(subj))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].subj != NULL) {
							/* merge with already read value */
							ptr[i].subj = my_realloc(ptr[i].subj, strlen(ptr[i].subj) + strlen(subj) + 2);
							strcat(ptr[i].subj, "|");
							strcat(ptr[i].subj, subj);
						} else {
							FreeIfNeeded(ptr[i].subj);
							ptr[i].subj = my_strdup(subj);
						}
					}

#ifdef DEBUG
					if (debug) {
						my_printf("6. buf=[%s]  Gsubj=[%s]\n", ptr[i].subj, glob_filter.filter[i].subj);
						my_flush();
					}
#endif /* DEBUG */
					break;
				}

				/*
				 * read score for rule
				 */
				if (match_string(buf + 1, "core=", scbuf, PATH_LEN)) {
					score = atoi(scbuf);
#ifdef DEBUG
					if (debug) {
						my_printf("score=[%d]\n", score);
						my_flush();
					}
#endif /* DEBUG */
					if (ptr && !expired_time) {
						if (score > SCORE_MAX)
							score = SCORE_MAX;
						else {
							if (score < -SCORE_MAX)
								score = -SCORE_MAX;
							else {
								if (!score) {
									if (!strncasecmp(scbuf, "kill", 4))
										score = tinrc.score_kill;
									else {
										if (!strncasecmp(scbuf, "hot", 3))
											score = tinrc.score_select;
									}
								}
							}
						}
						ptr[i].score = score;
					}
				}
				break;

			case 't':
				if (match_long(buf + 1, "ime=", &secs)) {
					if (ptr && !expired_time) {
						ptr[i].time = (time_t) secs;
						/* rule expired? */
						if (secs && current_secs > (time_t) secs) {
#ifdef DEBUG
							if (debug) {
								my_printf("EXPIRED  secs=[%lu]  current_secs=[%lu]\n", (unsigned long int) secs, (unsigned long int) current_secs);
								my_flush();
							}
#endif /* DEBUG */
							glob_filter.num--;
							expired_time = TRUE;
							expired = TRUE;
						}
					}
				}
				break;

			case 'x':
				/*
				 * TODO: fromat has changed in FILTER_VERSION 1.0.0,
				 *       should we comment out older xref rules like below?
				 */
				if (match_string(buf + 1, "ref=", xref, sizeof(xref))) {
					if (ptr && !expired_time) {
						if (tinrc.wildcard && ptr[i].xref != NULL) {
							/* merge with already read value */
							ptr[i].xref = my_realloc(ptr[i].xref, strlen(ptr[i].xref) + strlen(xref) + 2);
							strcat(ptr[i].xref, "|");
							strcat(ptr[i].xref, xref);
						} else {
							FreeIfNeeded(ptr[i].xref);
							ptr[i].xref = my_strdup(xref);
						}
					}
					break;
				}
				if (upgrade == RC_UPGRADE) {
					char foo[HEADER_LEN];

					if (match_string(buf + 1, "ref_max=", foo, LEN - 1)) {
						/*
						 * TODO: add to the right rule, give better explanation, -> lang.c
						 */
						snprintf(foo, HEADER_LEN, "%s%s", _("Removed from the previous rule: "), str_trim(buf));
						comment = add_filter_comment(comment, foo);
						break;
					}
					if (match_string(buf + 1, "ref_score=", foo, LEN - 1)) {
						/*
						 * TODO: add to the right rule, give better explanation, -> lang.c
						 */
						snprintf(foo, HEADER_LEN, "%s%s", _("Removed from the previous rule: "), str_trim(buf));
						comment = add_filter_comment(comment, foo);
					}
				}
				break;

			default:
				break;
		}
	}
	fclose(fp);

	if (expired || upgrade == RC_UPGRADE) {
		write_filter_file(file);
	}

	if (cmd_line && !batch_mode)
		printf("\r\n");

	if (!batch_mode)
		clear_message();

	return TRUE;
}


/*
 * write filter strings to ~/.tin/filter
 */
void
write_filter_file(
	const char *filename)
{
	FILE *fp;
	char *file_tmp;

	if (no_write)
		return;

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(filename);

	if (!backup_file(filename, file_tmp)) {
		error_message(_(txt_filesystem_full_backup), filename);
		free(file_tmp);
		return;
	}

	if ((fp = fopen(filename, "w")) == NULL)
		return;

	/* TODO: -> lang.c */
	fprintf(fp, "# Filter file V%s for the TIN newsreader\n#\n", FILTER_VERSION);
	fprintf(fp, _(txt_filter_file));
	fflush(fp);

	/*
	 * Save global filters
	 */
	write_filter_array(fp, &glob_filter);

	if (ferror(fp) || fclose(fp)) {
		error_message(_(txt_filesystem_full), filename);
		rename_file(file_tmp, filename);
	} else
		unlink(file_tmp);

	free(file_tmp);
}


static void
write_filter_array(
	FILE *fp,
	struct t_filters *ptr)
{
	int i;
	struct t_filter_comment *comment;
	time_t theTime = time(NULL);

	if (ptr == NULL)
		return;

	for (i = 0; i < ptr->num; i++) {
/* my_printf("WRITE i=[%d] subj=[%s] from=[%s]\n", i, BlankIfNull(ptr->filter[i].subj), BlankIfNull(ptr->filter[i].from)); */

		if (ptr->filter[i].time && theTime > ptr->filter[i].time)
				continue;
/*
my_printf("Scope=[%s]" cCRLF, (ptr->filter[i].scope != NULL ? ptr->filter[i].scope : "*"));
my_flush();
*/
		fprintf(fp, "\n");		/* makes filter file more readable */

		/* comments appear always first, if there are any... */
		if (ptr->filter[i].comment != NULL) {
			/*
			 * Save the start of the list, in case write_filter_array is
			 * called multiple times. Otherwise the list would get lost.
			 */
			comment = ptr->filter[i].comment;
			while (ptr->filter[i].comment != NULL) {
				fprintf(fp, "comment=%s\n", ptr->filter[i].comment->text);
				ptr->filter[i].comment = ptr->filter[i].comment->next;
			}
			ptr->filter[i].comment = comment;
		}

		fprintf(fp, "group=%s\n", (ptr->filter[i].scope != NULL ? ptr->filter[i].scope : "*"));

		fprintf(fp, "case=%u\n", ptr->filter[i].icase);

		if (ptr->filter[i].score == tinrc.score_kill)
			fprintf(fp, "score=kill\n");
		else if (ptr->filter[i].score == tinrc.score_select)
			fprintf(fp, "score=hot\n");
		else
			fprintf(fp, "score=%d\n", ptr->filter[i].score);

		if (ptr->filter[i].subj != NULL)
			fprintf(fp, "subj=%s\n", ptr->filter[i].subj);

		if (ptr->filter[i].from != NULL)
			fprintf(fp, "from=%s\n", ptr->filter[i].from);

		if (ptr->filter[i].msgid != NULL) {
			switch (ptr->filter[i].fullref) {
				case FILTER_MSGID:
					fprintf(fp, "msgid=%s\n", ptr->filter[i].msgid);
					break;

				case FILTER_MSGID_LAST:
					fprintf(fp, "msgid_last=%s\n", ptr->filter[i].msgid);
					break;

				case FILTER_MSGID_ONLY:
					fprintf(fp, "msgid_only=%s\n", ptr->filter[i].msgid);
					break;

				case FILTER_REFS_ONLY:
					fprintf(fp, "refs_only=%s\n", ptr->filter[i].msgid);
					break;

				default:
					break;
			}
		}

		if (ptr->filter[i].lines_cmp != FILTER_LINES_NO) {
			switch (ptr->filter[i].lines_cmp) {
				case FILTER_LINES_EQ:
					fprintf(fp, "lines=%d\n", ptr->filter[i].lines_num);
					break;

				case FILTER_LINES_LT:
					fprintf(fp, "lines=<%d\n", ptr->filter[i].lines_num);
					break;

				case FILTER_LINES_GT:
					fprintf(fp, "lines=>%d\n", ptr->filter[i].lines_num);
					break;

				default:
					break;
			}
		}

		if (ptr->filter[i].gnksa_cmp != FILTER_LINES_NO) {
			switch (ptr->filter[i].gnksa_cmp) {
				case FILTER_LINES_EQ:
					fprintf(fp, "gnksa=%d\n", ptr->filter[i].gnksa_num);
					break;

				case FILTER_LINES_LT:
					fprintf(fp, "gnksa=<%d\n", ptr->filter[i].gnksa_num);
					break;

				case FILTER_LINES_GT:
					fprintf(fp, "gnksa=>%d\n", ptr->filter[i].gnksa_num);
					break;

				default:
					break;
			}
		}

		if (ptr->filter[i].xref != NULL)
			fprintf(fp, "xref=%s\n", ptr->filter[i].xref);

		if (ptr->filter[i].time) {
			char timestring[25];
			if (my_strftime(timestring, sizeof(timestring) - 1, "%Y-%m-%d %H:%M:%S UTC", gmtime(&(ptr->filter[i].time))))
				fprintf(fp, "time=%lu (%s)\n", (unsigned long int) ptr->filter[i].time, timestring);
		}
	}
	fflush(fp);
}


/*
 * Interactive filter menu
 */
static int
get_choice(
	int x,
	const char *help,
	const char *prompt,
	char *list[],
	int list_size)
{
	int ch, y, i = 0;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wbuf;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (help)
		show_menu_help(help);

	if (list == NULL || list_size < 1)
		return -1;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	if ((wbuf = char2wchar_t(prompt)) != NULL) {
		wconvert_to_printable(wbuf);
		if ((y = wcswidth(wbuf, wcslen(wbuf) + 1)) == -1) /* something went wrong, use wcslen() as fallback */
			y = wcslen(wbuf);

		free(wbuf);
	} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		y = (int) strlen(prompt);

	do {
		MoveCursor(x, y);
		my_fputs(list[i], stdout);
		my_flush();
		CleartoEOLN();
		ch = ReadCh();
		switch (ch) {
			case ' ':
				i++;
				i %= list_size;
				break;

			case ESC:	/* (ESC) common arrow keys */
#	ifdef HAVE_KEY_PREFIX
			case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
				switch (get_arrow_key(ch)) {
					case KEYMAP_UP:
						i--;
						if (i < 0)
							i = list_size - 1;
						ch = ' ';	/* don't exit the while loop yet */
						break;

					case KEYMAP_DOWN:
						i++;
						i %= list_size;
						ch = ' ';	/* don't exit the while loop yet */
						break;

					default:
						break;
				}
				break;

			default:
				break;
		}
	} while (ch != '\n' && ch != '\r' && ch != iKeyAbort); /* TODO: replace hardcoded keynames */

	if (ch == iKeyAbort)
		return -1;

	return i;
}


static const char *ptr_filter_comment;
static const char *ptr_filter_lines;
static const char *ptr_filter_menu;
static const char *ptr_filter_scope;
static const char *ptr_filter_text;
static const char *ptr_filter_time;
static const char *ptr_filter_groupname;
static char text_subj[PATH_LEN];
static char text_from[PATH_LEN];
static char text_msgid[PATH_LEN];
static char text_score[PATH_LEN];


static void
print_filter_menu(
	void)
{
	ClearScreen();

	center_line(0, TRUE, ptr_filter_menu);

	MoveCursor(INDEX_TOP, 0);
	my_printf("%s%s%s", ptr_filter_comment, cCRLF, cCRLF);
	my_printf("%s%s", ptr_filter_text, cCRLF);
	my_printf("%s%s%s", _(txt_filter_text_type), cCRLF, cCRLF);
	my_printf("%s%s", text_subj, cCRLF);
	my_printf("%s%s", text_from, cCRLF);
	my_printf("%s%s%s", text_msgid, cCRLF, cCRLF);
	my_printf("%s%s", ptr_filter_lines, cCRLF);
	my_printf("%s%s", text_score, cCRLF);
	my_printf("%s%s%s", ptr_filter_time, cCRLF, cCRLF);
	my_printf("%s%s", ptr_filter_scope, ptr_filter_groupname);
	my_flush();
}


void
refresh_filter_menu(
	void)
{
	print_filter_menu();

	/*
	 * TODO:
	 * - refresh already entered and accepted information (follow control
	 *   flow in filter_menu below)
	 * - refresh help line
	 * - set cursor into current input field
	 * - refresh already entered data or selected item in current input field
	 *   (not everywhere possible yet -- must change getline.c for refreshing
	 *    string input)
	 */
}


/*
 * a help function for filter_menu
 * formats a menu option in a multibyte-safe way
 *
 * this function in closely tight to the way how the filter menu is build
 */
static void
fmt_filter_menu_prompt(
	char *dest,		/* where to store the resulting string */
	size_t dest_len,	/* size of dest */
	const char *fmt_str,	/* format string */
	int len,		/* maximal len of the include string */
	const char *text)	/* the include string */
{
	char *buf;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wbuf, *wbuf2;

	if ((wbuf = char2wchar_t(text)) != NULL) {
		wbuf2 = wcspart(wbuf, len, TRUE);
		if ((buf = wchar_t2char(wbuf2)) == NULL) {
			/* conversion failed, truncate original string */
			buf = my_malloc(len + 1);
			snprintf(buf, len + 1, "%-*.*s", len, len, text);
		}

		free(wbuf);
		free(wbuf2);
	} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	{
		buf = my_malloc(len + 1);
		snprintf(buf, len + 1, "%-*.*s", len, len, text);
	}
	snprintf(dest, dest_len, fmt_str, buf);
	free(buf);

	return;
}


/*
 * Interactive filter menu so that the user can dynamically enter parameters.
 * Can be configured for kill or auto-selection screens.
 */
t_bool
filter_menu(
	int type,
	struct t_group *group,
	struct t_article *art)
{
	const char *ptr_filter_from;
	const char *ptr_filter_msgid;
	const char *ptr_filter_subj;
	const char *ptr_filter_help_scope;
	const char *ptr_filter_quit_edit_save;
	char *ptr;
	char **list;
	char comment_line[LEN];
	char buf[LEN];
	char keyedit[MAXKEYLEN], keyquit[MAXKEYLEN], keysave[MAXKEYLEN];
	char text_time[PATH_LEN];
	char double_time[PATH_LEN];
	char quat_time[PATH_LEN];
	char ch_default = iKeyFilterSave;
	int ch, i, len;
	struct t_filter_rule rule;
	t_bool proceed;
	t_bool ret;

	signal_context = cFilter;

	rule.comment = (struct t_filter_comment *) 0;
	rule.text[0] = '\0';
	rule.scope[0] = '\0';
	rule.counter = 0;
	rule.lines_cmp = FILTER_LINES_NO;
	rule.lines_num = 0;
	rule.from_ok = FALSE;
	rule.lines_ok = FALSE;
	rule.msgid_ok = FALSE;
	rule.fullref = FILTER_MSGID;
	rule.subj_ok = FALSE;
	rule.icase = FALSE;
	rule.score = 0;
	rule.expire_time = FALSE;
	rule.check_string = FALSE;

	comment_line[0] = '\0';

	/*
	 * setup correct text for user selected menu
	 */
	(void) printascii(keyedit, map_to_local(iKeyFilterEdit, &menukeymap.filter_quit_edit_save));
	(void) printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.filter_quit_edit_save));
	(void) printascii(keysave, map_to_local(iKeyFilterSave, &menukeymap.filter_quit_edit_save));

	if (type == FILTER_KILL) {
		ptr_filter_from = _(txt_kill_from);
		ptr_filter_lines = _(txt_kill_lines);
		ptr_filter_menu = _(txt_kill_menu);
		ptr_filter_msgid = _(txt_kill_msgid);
		ptr_filter_scope = _(txt_kill_scope);
		ptr_filter_subj = _(txt_kill_subj);
		ptr_filter_text = _(txt_kill_text);
		ptr_filter_time = _(txt_kill_time);
		ptr_filter_help_scope = _(txt_help_kill_scope);
		ptr_filter_quit_edit_save = _(txt_quit_edit_save_kill);
	} else {
		ptr_filter_from = _(txt_select_from);
		ptr_filter_lines = _(txt_select_lines);
		ptr_filter_menu = _(txt_select_menu);
		ptr_filter_msgid = _(txt_select_msgid);
		ptr_filter_scope = _(txt_select_scope);
		ptr_filter_subj = _(txt_select_subj);
		ptr_filter_text = _(txt_select_text);
		ptr_filter_time = _(txt_select_time);
		ptr_filter_help_scope = _(txt_help_select_scope);
		ptr_filter_quit_edit_save = _(txt_quit_edit_save_select);
	}

	ptr_filter_comment = _(txt_filter_comment);
	ptr_filter_groupname = group->name;

	len = cCOLS - 30;

	snprintf(text_time, sizeof(text_time), _(txt_time_default_days), tinrc.filter_days);
	fmt_filter_menu_prompt(text_subj, sizeof(text_subj), ptr_filter_subj, len, art->subject);
	snprintf(text_score, sizeof(text_score), _(txt_filter_score), (type == FILTER_KILL ? -tinrc.score_kill : tinrc.score_select));
	fmt_filter_menu_prompt(text_from, sizeof(text_from), ptr_filter_from, len, art->from);
	fmt_filter_menu_prompt(text_msgid, sizeof(text_msgid), ptr_filter_msgid, len - 4, MSGID(art));

	print_filter_menu();

	/*
	 * None, one or multiple lines of comment.
	 * Continue until an empty line is entered.
	 * The empty line is ignored.
	 */
	show_menu_help(_(txt_help_filter_comment));
	while ((proceed = prompt_menu_string(INDEX_TOP, ptr_filter_comment, comment_line)) && comment_line[0] != '\0') {
		rule.comment = add_filter_comment(rule.comment, comment_line);
		comment_line[0] = '\0';
	}
	if (!proceed) {
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	/*
	 * Text which might be used to filter on subj, from or msgid
	 */
	show_menu_help(_(txt_help_filter_text));
	if (!prompt_menu_string(INDEX_TOP + 2, ptr_filter_text, rule.text)) {
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	if (*rule.text) {
		list = my_malloc(sizeof(char *) * 5);
		list[0] = (char *) _(txt_subj_line_only_case);
		list[1] = (char *) _(txt_subj_line_only);
		list[2] = (char *) _(txt_from_line_only_case);
		list[3] = (char *) _(txt_from_line_only);
		list[4] = (char *) _(txt_msgid_line_only);

		i = get_choice(INDEX_TOP + 3, _(txt_help_filter_text_type),
			       _(txt_filter_text_type), list, 5);
		free(list);

		if (i == -1) {
			rule.comment = free_filter_comment(rule.comment);
			return FALSE;
		}

		rule.counter = i;
		switch (i) {
			case FILTER_SUBJ_CASE_IGNORE:
			case FILTER_FROM_CASE_IGNORE:
				rule.icase = TRUE;
				break;

			case FILTER_SUBJ_CASE_SENSITIVE:
			case FILTER_FROM_CASE_SENSITIVE:
			case FILTER_MSGID:
			case FILTER_MSGID_LAST:
			case FILTER_MSGID_ONLY:
				/* rule.icase is FALSE already, no assignment necessary */
				break;

			default: /* should not happen */
				/* CONSTANTCONDITION */
				assert(0 != 0);
				break;
		}
	}

	if (!*rule.text) {
		rule.check_string = TRUE;
		/*
		 * Subject:
		 */
		list = my_malloc(sizeof(char *) * 2);
		list[0] = (char *) _(txt_yes);
		list[1] = (char *) _(txt_no);
		i = get_choice(INDEX_TOP + 5, _(txt_help_filter_subj), text_subj, list, 2);
		free(list);

		if (i == -1) {
			rule.comment = free_filter_comment(rule.comment);
			return FALSE;
		} else
			rule.subj_ok = (i == 0);

		/*
		 * From:
		 */
		list = my_malloc(sizeof(char *) * 2);
		if (rule.subj_ok) {
			list[0] = (char *) _(txt_no);
			list[1] = (char *) _(txt_yes);
		} else {
			list[0] = (char *) _(txt_yes);
			list[1] = (char *) _(txt_no);
		}
		i = get_choice(INDEX_TOP + 6, _(txt_help_filter_from), text_from, list, 2);
		free(list);

		if (i == -1) {
			rule.comment = free_filter_comment(rule.comment);
			return FALSE;
		} else
			rule.from_ok = rule.subj_ok ? (i != 0) : (i == 0);

		/*
		 * Message-ID:
		 */
		list = my_malloc(sizeof(char *) * 4);
		if (rule.subj_ok || rule.from_ok) {
			list[0] = (char *) _(txt_no);
			list[1] = (char *) _(txt_full);
			list[2] = (char *) _(txt_last);
			list[3] = (char *) _(txt_only);
		} else {
			list[0] = (char *) _(txt_full);
			list[1] = (char *) _(txt_last);
			list[2] = (char *) _(txt_only);
			list[3] = (char *) _(txt_no);
		}
		i = get_choice(INDEX_TOP + 7, _(txt_help_filter_msgid), text_msgid, list, 4);
		free(list);

		if (i == -1) {
			rule.comment = free_filter_comment(rule.comment);
			return FALSE;
		} else {
			switch ((rule.subj_ok || rule.from_ok) ? i : i + 1) {
				case 0:
				case 4:
					rule.msgid_ok = FALSE;
					rule.fullref = FILTER_MSGID;
					break;

				case 1:
					rule.msgid_ok = TRUE;
					rule.fullref = FILTER_MSGID;
					break;

				case 2:
					rule.msgid_ok = TRUE;
					rule.fullref = FILTER_MSGID_LAST;
					break;

				case 3:
					rule.msgid_ok = TRUE;
					rule.fullref = FILTER_MSGID_ONLY;
					break;

				default: /* should not happen */
					/* CONSTANTCONDITION */
					assert(0 != 0);
					break;
			}
		}

	}

	/*
	 * Lines:
	 */
	show_menu_help(_(txt_help_filter_lines));

	buf[0] = '\0';

	if (!prompt_menu_string(INDEX_TOP + 9, ptr_filter_lines, buf)) {
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	/*
	 * Get the < > sign if any for the lines rule
	 */
	ptr = buf;
	while (ptr && *ptr == ' ')
		ptr++;

	if (ptr && *ptr == '>') {
		rule.lines_cmp = FILTER_LINES_GT;
		ptr++;
	} else if (ptr && *ptr == '<') {
		rule.lines_cmp = FILTER_LINES_LT;
		ptr++;
	} else if (ptr && *ptr == '=') {
		rule.lines_cmp = FILTER_LINES_EQ;
		ptr++;
	}
	rule.lines_num = atoi(ptr);

	if (rule.lines_cmp != FILTER_LINES_NO && rule.lines_num >= 0)
		rule.lines_ok = TRUE;

	/*
	 * Scoring value
	 */
	snprintf(buf, sizeof(buf), _(txt_filter_score_help), SCORE_MAX);
	show_menu_help(buf);

	buf[0] = '\0';
	if (!prompt_menu_string(INDEX_TOP + 10, text_score, buf)) {
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	/* check if a score has been entered */
	if (buf[0] != '\0')
		/* use entered score */
		rule.score = atoi(buf);
	else {
		/* use default score */
		if (type == FILTER_KILL)
			rule.score = tinrc.score_kill;
		else /* type == FILTER_SELECT */
			rule.score = tinrc.score_select;
	}

	if (!rule.score) { /* ignore 0 scores */
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	/*
	 * assure we are in range
	 */
	if (rule.score < 0)
		rule.score = abs(rule.score);
	if (rule.score > SCORE_MAX)
		rule.score = SCORE_MAX;

	/* get the right sign for the score */
	if (type == FILTER_KILL)
		rule.score = -rule.score;

	/*
	 * Expire time
	 */
	snprintf(double_time, sizeof(double_time), "2x %s", text_time);
	snprintf(quat_time, sizeof(double_time), "4x %s", text_time);
	list = my_malloc(sizeof(char *) * 4);
	list[0] = (char *) _(txt_unlimited_time);
	list[1] = text_time;
	list[2] = double_time;
	list[3] = quat_time;
	i = get_choice(INDEX_TOP + 11, _(txt_help_filter_time), ptr_filter_time, list, 4);
	free(list);

	if (i == -1) {
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	rule.expire_time = i;

	/*
	 * Scope
	 */
	if (*rule.text || rule.subj_ok || rule.from_ok || rule.msgid_ok || rule.lines_ok) {
		int j = 0;

		list = my_malloc(sizeof(char *) * 2); /* at least 2 scopes */
		list[j++] = my_strdup(group->name);
		list[j] = my_strdup(list[j - 1]);
		while ((ptr = strrchr(list[j], '.')) != NULL) {
			*(++ptr) = '*';
			*(++ptr) = '\0';
			j++;
			list = my_realloc(list, sizeof(char *) * (j + 1)); /* one element more */
			list[j] = my_strdup(list[j - 1]);
			list[j][strlen(list[j]) - 2] = '\0';
		}
		free(list[j]); /* this copy isn't needed anymore */
		list[j] = (char *) _(txt_all_groups);

		if ((i = get_choice(INDEX_TOP + 13, ptr_filter_help_scope, ptr_filter_scope, list, j + 1)) > 0)
			strncpy(rule.scope, i == j ? "*" : list[i], sizeof(rule.scope));

		for (j--; j >= 0; j--)
			free(list[j]);
		free(list);

		if (i == -1) {
			rule.comment = free_filter_comment(rule.comment);
			return FALSE;
		}
	} else {
		rule.comment = free_filter_comment(rule.comment);
		return FALSE;
	}

	forever {
		ch = prompt_slk_response(ch_default,
				&menukeymap.filter_quit_edit_save,
				ptr_filter_quit_edit_save,
				keyquit, keyedit, keysave);
		switch (ch) {

		case iKeyFilterEdit:
			add_filter_rule(group, art, &rule, FALSE); /* save the rule */
			rule.comment = free_filter_comment(rule.comment);
			if (!invoke_editor(filter_file, FILTER_FILE_OFFSET))
				return FALSE;
			unfilter_articles();
			(void) read_filter_file(filter_file);
			return TRUE;
			/* keep lint quiet: */
			/* FALLTHROUGH */

		case iKeyQuit:
		case iKeyAbort:
			rule.comment = free_filter_comment(rule.comment);
			return FALSE;
			/* keep lint quiet: */
			/* FALLTHROUGH */

		case iKeyFilterSave:
			/*
			 * Add the filter rule and save it to the filter file
			 */
			ret = add_filter_rule(group, art, &rule, FALSE);
			rule.comment = free_filter_comment(rule.comment);
			return ret;
			/* keep lint quiet: */
			/* FALLTHROUGH */

		default:
			break;
		}
	}
	/* NOTREACHED */
	return FALSE;
}


/*
 * Quick command to add an auto-select / kill filter to specified groups filter
 */
t_bool
quick_filter(
	int type,
	struct t_group *group,
	struct t_article *art)
{
	char *scope;
	char txt[LEN];
	int header, expire, icase;
	struct t_filter_rule rule;
	t_bool ret;

	if (type == FILTER_KILL) {
		header = group->attribute->quick_kill_header;
		expire = group->attribute->quick_kill_expire;
		icase = group->attribute->quick_kill_case;
		scope = group->attribute->quick_kill_scope;
	} else {
		header = group->attribute->quick_select_header;
		expire = group->attribute->quick_select_expire;
		icase = group->attribute->quick_select_case;
		scope = group->attribute->quick_select_scope;
	}

#ifdef DEBUG
	if (debug)
		error_message("%s header=[%d] scope=[%s] expire=[%s] case=[%d]",
			(type == FILTER_KILL) ? "KILL" : "SELECT", header,
			BlankIfNull(scope), txt_onoff[expire != FALSE ? 1 : 0], icase);
#endif /* DEBUG */

	/*
	 * Setup rules
	 */
	strcpy(rule.scope, BlankIfNull(scope));
	rule.counter = 0;
	rule.lines_cmp = FILTER_LINES_NO;
	rule.lines_num = 0;
	rule.lines_ok = (header == FILTER_LINES);
	rule.msgid_ok = (header == FILTER_MSGID) || (header == FILTER_MSGID_LAST);
	rule.fullref = header; /* value is directly used to select correct filter type */
	rule.from_ok = (header == FILTER_FROM_CASE_SENSITIVE || header == FILTER_FROM_CASE_IGNORE);
	rule.subj_ok = (header == FILTER_SUBJ_CASE_SENSITIVE || header == FILTER_SUBJ_CASE_IGNORE);

	/* create an auto-comment. */
	if (type == FILTER_KILL)
		snprintf(txt, sizeof(txt), "%s%s%c%s%s%s", _(txt_filter_rule_created), "'", iKeyGroupQuickKill, "' (", _(txt_help_article_quick_kill), ").");
	else
		snprintf(txt, sizeof(txt), "%s%s%c%s%s%s", _(txt_filter_rule_created), "'", iKeyGroupQuickAutoSel, "' (", _(txt_help_article_quick_select), ").");
	rule.comment = add_filter_comment(NULL, txt);

	rule.text[0] = '\0';
	rule.icase = icase;
	rule.expire_time = expire;
	rule.check_string = TRUE;
	rule.score = (type == FILTER_KILL) ? tinrc.score_kill : tinrc.score_select;

	ret = add_filter_rule(group, art, &rule, TRUE);
	rule.comment = free_filter_comment(rule.comment);
	return ret;
}


/*
 * Quick command to add an auto-select filter to the article that user
 * has just posted. Selects on Subject: line with limited expire time.
 * Don't process if GROUP_TYPE_MAIL || GROUP_TYPE_SAVE
 */
t_bool
quick_filter_select_posted_art(
	struct t_group *group,
	const char *subj,
	const char *a_message_id)	/* return value is always ignored */
{
	t_bool filtered = FALSE;
	char txt[LEN];

	if (group->type == GROUP_TYPE_NEWS) {
		struct t_article art;
		struct t_filter_rule rule;

#ifdef __cplusplus /* keep C++ quiet */
		rule.scope[0] = '\0';
#endif /* __cplusplus */

		if (strlen(group->name) > (sizeof(rule.scope) - 1)) /* groupname to long? */
			return FALSE;

		/*
		 * Setup rules
		 */
		rule.counter = 0;
		rule.lines_cmp = FILTER_LINES_NO;
		rule.lines_num = 0;
		rule.from_ok = FALSE;
		rule.lines_ok = FALSE;
		rule.msgid_ok = FALSE;
		rule.fullref = FILTER_MSGID;
		rule.subj_ok = TRUE;
		rule.text[0] = '\0';
		rule.icase = FALSE;
		rule.expire_time = TRUE;
		rule.check_string = TRUE;
		rule.score = tinrc.score_select;

		strcpy(rule.scope, group->name);

		/* create an auto-comment. */
		snprintf(txt, sizeof(txt), "%s%s", _(txt_filter_rule_created), "add_posted_to_filter=ON.");
		rule.comment = add_filter_comment(NULL, txt);

		/*
		 * Setup dummy article with posted articles subject
		 * xor Message-ID
		 */
		set_article(&art);
		if (*a_message_id) {
			/* initialize art->refptr */
			struct {
				struct t_msgid *next;
				struct t_msgid *parent;
				struct t_msgid *sibling;
				struct t_msgid *child;
				int article;
				char txt[HEADER_LEN];
			} refptr_dummyart;

			rule.subj_ok = FALSE;
			rule.msgid_ok = TRUE;
			refptr_dummyart.next = (struct t_msgid *) 0;
			refptr_dummyart.parent = (struct t_msgid *) 0;
			refptr_dummyart.sibling = (struct t_msgid *) 0;
			refptr_dummyart.child = (struct t_msgid *) 0;
			refptr_dummyart.article = ART_NORMAL;
			my_strncpy(refptr_dummyart.txt, a_message_id, HEADER_LEN);
			/* Hack */
			art.refptr = (struct t_msgid *) &refptr_dummyart;

			filtered = add_filter_rule(group, &art, &rule, FALSE);
		} else {
			art.subject = my_strdup(subj);
			filtered = add_filter_rule(group, &art, &rule, FALSE);
			FreeIfNeeded(art.subject);
		}
		rule.comment = free_filter_comment(rule.comment);
	}
	return filtered;
}


/*
 * API to add filter rule to the local or global filter array
 */
static t_bool
add_filter_rule(
	struct t_group *group,
	struct t_article *art,
	struct t_filter_rule *rule,
	t_bool quick_filter_rule)
{
	char acBuf[PATH_LEN];
	char sbuf[(sizeof(acBuf) / 2)]; /* half as big as acBuf so quote_wild(sbuf) fits into acBuf */
	int i = glob_filter.num;
	t_bool filtered = FALSE;
	time_t current_time;
	struct t_filter *ptr;

	if (glob_filter.num >= glob_filter.max)
		expand_filter_array(&glob_filter);

	ptr = glob_filter.filter;

	ptr[i].icase = FALSE;
	ptr[i].inscope = TRUE;
	ptr[i].fullref = FILTER_MSGID;
	ptr[i].comment = (struct t_filter_comment *) 0;
	ptr[i].scope = NULL;
	ptr[i].subj = NULL;
	ptr[i].from = NULL;
	ptr[i].msgid = NULL;
	ptr[i].lines_cmp = rule->lines_cmp;
	ptr[i].lines_num = rule->lines_num;
	ptr[i].gnksa_cmp = FILTER_LINES_NO;
	ptr[i].gnksa_num = 0;
	ptr[i].score = rule->score;
	ptr[i].xref = NULL;

	if (rule->comment != NULL)
		ptr[i].comment = copy_filter_comment(rule->comment, ptr[i].comment);

	if (rule->scope[0] == '\0') /* replace empty scope with current group name */
		ptr[i].scope = my_strdup(group->name);
	else {
		if ((rule->scope[0] != '*') && (rule->scope[1] != '\0')) /* copy non-global scope */
			ptr[i].scope = my_strdup(rule->scope);
	}

	(void) time(&current_time);
	switch (rule->expire_time) {
		case 1:
			ptr[i].time = current_time + (time_t) (tinrc.filter_days * DAY);
			break;

		case 2:
			ptr[i].time = current_time + (time_t) (tinrc.filter_days * DAY * 2);
			break;

		case 3:
			ptr[i].time = current_time + (time_t) (tinrc.filter_days * DAY * 4);
			break;

		default:
			ptr[i].time = (time_t) 0;
			break;
	}

	ptr[i].icase = rule->icase;
	if (*rule->text) {
		snprintf(acBuf, sizeof(acBuf), REGEX_FMT, quote_wild_whitespace(rule->text));

		switch (rule->counter) {
			case FILTER_SUBJ_CASE_IGNORE:
			case FILTER_SUBJ_CASE_SENSITIVE:
				ptr[i].subj = my_strdup(acBuf);
				break;

			case FILTER_FROM_CASE_IGNORE:
			case FILTER_FROM_CASE_SENSITIVE:
				ptr[i].from = my_strdup(acBuf);
				break;

			case FILTER_MSGID:
			case FILTER_MSGID_LAST:
			case FILTER_MSGID_ONLY:
			case FILTER_REFS_ONLY:
				ptr[i].msgid = my_strdup(acBuf);
				ptr[i].fullref = rule->counter;
				break;

			default: /* should not happen */
				/* CONSTANTCONDITION */
				assert(0 != 0);
				break;
		}
		filtered = TRUE;
		glob_filter.num++;
	} else {
		/*
		 * STRCPY() truncates subject/from/message-id so it fits
		 * into acBuf even after quote_wild()
		 */
		if (rule->subj_ok) {
			STRCPY(sbuf, art->subject);
			snprintf(acBuf, sizeof(acBuf), REGEX_FMT, (rule->check_string ? quote_wild(sbuf) : sbuf));
			ptr[i].subj = my_strdup(acBuf);
		}
		if (rule->from_ok) {
			STRCPY(sbuf, art->from);
			snprintf(acBuf, sizeof(acBuf), REGEX_FMT, quote_wild(sbuf));
			ptr[i].from = my_strdup(acBuf);
		}
		/*
		 * message-ids should be quoted
		 */
		if (rule->msgid_ok) {
			/*
			 * If threading by references is set, and a quick kill is applied
			 * (in group level), it is applied with the data of the root
			 * article of the thread built by tin.
			 * In case of threading by references, if tin's root is not the
			 * *real* root of thread (which is the first entry in references
			 * field) any applying of filtering for MSGID (or MSGID_LAST)
			 * doesn't work, because the filter is applied with the data of
			 * tin's root article which doesn't cover the other articles in
			 * the thread.
			 * So the thread remains open (in group level). To overcome this,
			 * the first msgid from references field is taken in this case.
			 */
			if (quick_filter_rule && group->attribute->thread_arts == THREAD_REFS &&
				(group->attribute->quick_kill_header == FILTER_MSGID ||
				 group->attribute->quick_kill_header == FILTER_REFS_ONLY) &&
				 art->refptr->parent != NULL)
			{
				/* not real parent, take first references entry as MID */
				struct t_msgid *xptr;

				for (xptr = art->refptr->parent; xptr->parent != NULL; xptr = xptr->parent)
					;
				STRCPY(sbuf, xptr->txt);
			} else {
				STRCPY(sbuf, MSGID(art));
			}
			snprintf(acBuf, sizeof(acBuf), REGEX_FMT, quote_wild(sbuf));
			ptr[i].msgid = my_strdup(acBuf);
			ptr[i].fullref = rule->fullref;
		}
		if (rule->subj_ok || rule->from_ok || rule->msgid_ok || rule->lines_ok) {
			filtered = TRUE;
			glob_filter.num++;
		}
	}

	if (filtered) {
#ifdef DEBUG
		if (debug)
			wait_message(2, "inscope=[%s] scope=[%s]  case=[%d] subj=[%s] from=[%s] msgid=[%s] fullref=[%d] line=[%d %d] time=[%lu]",
				bool_unparse(ptr[i].inscope),
				BlankIfNull(rule->scope),
				ptr[i].icase,
				BlankIfNull(ptr[i].subj),
				BlankIfNull(ptr[i].from),
				BlankIfNull(ptr[i].msgid),
				ptr[i].fullref, ptr[i].lines_cmp,
				ptr[i].lines_num, (unsigned long int) ptr[i].time);
#endif /* DEBUG */

		write_filter_file(filter_file);
	}
	return filtered;
}


/*
 * We assume that any articles which are tagged as killed are also
 * tagged as being read BECAUSE they were killed. So, we retag
 * them as being unread. Selected articles will be un"select"ed.
 */
void
unfilter_articles(
	void)
{
	int i;

	for_each_art(i) {
		arts[i].score = 0;
		if (IS_KILLED(i)) {
			arts[i].killed = ART_NOTKILLED;
			arts[i].status = ART_UNREAD;
		}
		if (IS_SELECTED(i))
			arts[i].selected = FALSE;
	}
}


/*
 * Filter any articles in specified group.
 * Apply global filter rules followed by group filter rules.
 * In global rules check if scope field set to determine if
 * filter applys to current group.
 */
t_bool
filter_articles(
	struct t_group *group)
{
	char buf[LEN];
	int num, inscope;
	int i, j;
	struct t_filter *ptr;
	struct regex_cache *regex_cache_subj = NULL;
	struct regex_cache *regex_cache_from = NULL;
	struct regex_cache *regex_cache_msgid = NULL;
	struct regex_cache *regex_cache_xref = NULL;
	t_bool filtered = FALSE;
	t_bool error = FALSE;

	/*
	 * check if there are any global filter rules
	 */
	if (group->glob_filter->num == 0)
		return filtered;

	/*
	 * Apply global filter rules first if there are any entries
	 */
	/*
	 * Check if any scope rules are active for this group
	 * ie. group=comp.os.linux.help  scope=comp.os.linux.*
	 */
	inscope = set_filter_scope(group);
	if (!cmd_line && !batch_mode)
		wait_message(0, _(txt_filter_global_rules), inscope, group->glob_filter->num);
	num = group->glob_filter->num;
	ptr = group->glob_filter->filter;

	/*
	 * set up cache tables for all types of filter rules
	 * (only for regexp matching)
	 */
	if (tinrc.wildcard) {
		size_t msiz;

		msiz = sizeof(struct regex_cache) * num;
		regex_cache_subj = my_malloc(msiz);
		regex_cache_from = my_malloc(msiz);
		regex_cache_msgid = my_malloc(msiz);
		regex_cache_xref = my_malloc(msiz);
		for (j = 0; j < num; j++) {
			regex_cache_subj[j].re = NULL;
			regex_cache_subj[j].extra = NULL;
			regex_cache_from[j].re = NULL;
			regex_cache_from[j].extra = NULL;
			regex_cache_msgid[j].re = NULL;
			regex_cache_msgid[j].extra = NULL;
			regex_cache_xref[j].re = NULL;
			regex_cache_xref[j].extra = NULL;
		}
	}

	/*
	 * loop thru all arts applying global & local filtering rules
	 */
	for (i = 0; (i < top_art) && !error; i++) {
		arts[i].score = 0;

		if (tinrc.kill_level == KILL_UNREAD && IS_READ(i)) /* skip only when the article is read */
			continue;

		for (j = 0; j < num && !error; j++) {
			if (ptr[j].inscope) {
				/*
				 * Filter on Subject: line
				 */
				if (ptr[j].subj != NULL) {
					switch (test_regex(arts[i].subject, ptr[j].subj, ptr[j].icase, &regex_cache_subj[j])) {
						case 1:
							SET_FILTER(group, i, j);
							break;

						case -1:
							error = TRUE;
							break;

						default:
							break;
					}
				}

				/*
				 * Filter on From: line
				 */
				if (ptr[j].from != NULL) {
					if (arts[i].name != NULL)
						snprintf(buf, sizeof(buf), "%s (%s)", arts[i].from, arts[i].name);
					else
						strcpy(buf, arts[i].from);

					switch (test_regex(buf, ptr[j].from, ptr[j].icase, &regex_cache_from[j])) {
						case 1:
							SET_FILTER(group, i, j);
							break;

						case -1:
							error = TRUE;
							break;

						default:
							break;
					}
				}

				/*
				 * Filter on Message-ID: line
				 * Apply to Message-ID: & References: lines or
				 * Message-ID: & last entry from References: line
				 * Case is important here
				 */
				if (ptr[j].msgid != NULL) {
					char *refs = NULL;
					const char *myrefs = NULL;
					const char *mymsgid = NULL;
					int x;
					struct t_article *art = &arts[i];
					/*
					 * TODO: nice idea del'd; better apply one rule on all
					 *       fitting articles, so we can switch to an appropriate
					 *       algorithm for each kind of rule, including the
					 *       deleted one.
					 */

					/* myrefs does not need to be freed */

					/* use full references header or just the last entry? */
					switch (ptr[j].fullref) {
						case FILTER_MSGID:
							myrefs = REFS(art, refs);
							mymsgid = MSGID(art);
							break;

						case FILTER_MSGID_LAST:
							myrefs = (art->refptr->parent) ? art->refptr->parent->txt : "";
							mymsgid = MSGID(art);
							break;

						case FILTER_MSGID_ONLY:
							myrefs = "";
							mymsgid = MSGID(art);
							break;

						case FILTER_REFS_ONLY:
							myrefs = REFS(art, refs);
							mymsgid = "";
							break;

						default: /* should not happen */
							/* CONSTANTCONDITION */
							assert(0 != 0);
							break;
					}

					if ((x = test_regex(myrefs, ptr[j].msgid, FALSE, &regex_cache_msgid[j])) == 0) /* no match */
						x = test_regex(mymsgid, ptr[j].msgid, FALSE, &regex_cache_msgid[j]);

					switch (x) {
						case 1:
							SET_FILTER(group, i, j);
							break;

						case -1:
							error = TRUE;
							break;

						default:
							break;
					}
					FreeIfNeeded(refs);
				}
				/*
				 * Filter on Lines: line
				 */
				if ((ptr[j].lines_cmp != FILTER_LINES_NO) && (arts[i].line_count >= 0)) {
					switch (ptr[j].lines_cmp) {
						case FILTER_LINES_EQ:
							if (arts[i].line_count == ptr[j].lines_num) {
/*
wait_message(1, "FILTERED Lines arts[%d] == [%d]", arts[i].line_count, ptr[j].lines_num);
*/
								SET_FILTER(group, i, j);
							}
							break;

						case FILTER_LINES_LT:
							if (arts[i].line_count < ptr[j].lines_num) {
/*
wait_message(1, "FILTERED Lines arts[%d] < [%d]", arts[i].line_count, ptr[j].lines_num);
*/
								SET_FILTER(group, i, j);
							}
							break;

						case FILTER_LINES_GT:
							if (arts[i].line_count > ptr[j].lines_num) {
/*
wait_message(1, "FILTERED Lines arts[%d] > [%d]", arts[i].line_count, ptr[j].lines_num);
*/
								SET_FILTER(group, i, j);
							}
							break;

						default:
							break;
					}
				}

				/*
				 * Filter on GNKSA code
				 */
				if ((ptr[j].gnksa_cmp != FILTER_LINES_NO) && (arts[i].gnksa_code >= 0)) {
					switch (ptr[j].gnksa_cmp) {
						case FILTER_LINES_EQ:
							if (arts[i].gnksa_code == ptr[j].gnksa_num) {
								SET_FILTER(group, i, j);
							}
							break;

						case FILTER_LINES_LT:
							if (arts[i].gnksa_code < ptr[j].gnksa_num) {
								SET_FILTER(group, i, j);
							}
							break;

						case FILTER_LINES_GT:
							if (arts[i].gnksa_code > ptr[j].gnksa_num) {
								SET_FILTER(group, i, j);
							}
							break;

						default:
							break;
					}
				}

				/*
				 * Filter on Xref: lines
				 *
				 * 	Xref: news.foo.bar foo.bar:666 bar.bar:999
				 * is turned into
				 * 	foo.bar,bar.bar
				 */
				if (arts[i].xref && *arts[i].xref) {
					if (ptr[j].score && ptr[j].xref != NULL) {
						char *s, *e, *k;
						t_bool skip = FALSE;

						s = arts[i].xref;
						while (*s && !isspace((int) *s))
							s++;
						while (*s && isspace((int) *s))
							s++;

						/* reformat */
						k = e = my_malloc(strlen(s));
						while (*s) {
							if (*s == ':') {
								*e++ = ',';
								skip = TRUE;
							}
							if (*s != ':' && !isspace((int) *s) && !skip)
								*e++ = *s;
							if (isspace((int) *s))
								skip = FALSE;
							s++;
						}
						*--e = '\0';

						if (ptr[j].xref != NULL) {
							switch (test_regex(k, ptr[j].xref, ptr[j].icase, &regex_cache_xref[j])) {
								case 1:
									SET_FILTER(group, i, j);
									break;

								case -1:
									error = TRUE;
									break;

								default:
									break;
							}
						}
						free(k);
					}
				}
			}
		}
	}

	/*
	 * throw away the contents of all regex_caches
	 */
	if (tinrc.wildcard) {
		for (j = 0; j < num; j++) {
			FreeIfNeeded(regex_cache_subj[j].re);
			FreeIfNeeded(regex_cache_subj[j].extra);
			FreeIfNeeded(regex_cache_from[j].re);
			FreeIfNeeded(regex_cache_from[j].extra);
			FreeIfNeeded(regex_cache_msgid[j].re);
			FreeIfNeeded(regex_cache_msgid[j].extra);
			FreeIfNeeded(regex_cache_xref[j].re);
			FreeIfNeeded(regex_cache_xref[j].extra);
		}
		free(regex_cache_subj);
		free(regex_cache_from);
		free(regex_cache_msgid);
		free(regex_cache_xref);
	}

	/*
	 * now entering the main filter loop:
	 * all articles have scored, so do kill & select
	 */
	if (!error) {
		for_each_art(i) {
			if (arts[i].score <= tinrc.score_limit_kill) {
				if (arts[i].status == ART_UNREAD)
					arts[i].killed = ART_KILLED_UNREAD;
				else
					arts[i].killed = ART_KILLED;
				filtered = TRUE;
				art_mark(group, &arts[i], ART_READ);
			} else if (arts[i].score >= tinrc.score_limit_select) {
				arts[i].selected = TRUE;
			}
		}
	}
	return filtered;
}


static int
set_filter_scope(
	struct t_group *group)
{
	int i, num, inscope;
	struct t_filter *ptr, *prev;

	inscope = num = group->glob_filter->num;
	prev = ptr = group->glob_filter->filter;

	for (i = 0; i < num; i++) {
		ptr[i].inscope = TRUE;
		ptr[i].next = (struct t_filter *) 0;
		if (ptr[i].scope != NULL) {
			if (!match_group_list(group->name, ptr[i].scope)) {
				ptr[i].inscope = FALSE;
				inscope--;
			}
		}
		if (i != 0 && ptr[i].inscope)
			prev = prev->next = &ptr[i];
	}
	return inscope;
}


/*
 * This will come in useful for filtering on non-overview hdr fields
 */
#if 0
static FILE *
open_xhdr_fp(
	char *header,
	long min,
	long max)
{
#	ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news && xhdr_cmd) {
		char buf[NNTP_STRLEN];

		snprintf(buf, sizeof(buf), "%s %s %ld-%ld", xhdr_cmd, header, min, max);
		return (nntp_command(buf, OK_HEAD, NULL, 0));
	} else
#	endif /* NNTP_ABLE */
		return (FILE *) 0;		/* Some trick implementation for local spool... */
}
#endif /* 0 */
