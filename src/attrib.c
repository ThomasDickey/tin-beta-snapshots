/*
 *  Project   : tin - a Usenet reader
 *  Module    : attrib.c
 *  Author    : I. Lea
 *  Created   : 1993-12-01
 *  Updated   : 2005-06-30
 *  Notes     : Group attribute routines
 *
 * Copyright (c) 1993-2006 Iain Lea <iain@bricbrac.de>
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

#ifdef DEBUG
#	ifndef TCURSES_H
#		include "tcurses.h"
#	endif /* !TCURSES_H */
#endif /* DEBUG */

/*
 * Defines used in setting attributes switch
 */
enum {
	ATTRIB_MAILDIR,
	ATTRIB_SAVEDIR,
	ATTRIB_SAVEFILE,
	ATTRIB_ORGANIZATION,
	ATTRIB_FROM,
	ATTRIB_SIGFILE,
	ATTRIB_FOLLOWUP_TO,
	ATTRIB_AUTO_SELECT,
	ATTRIB_AUTO_SAVE,
	ATTRIB_BATCH_SAVE,
	ATTRIB_DELETE_TMP_FILES,
	ATTRIB_SHOW_ONLY_UNREAD,
	ATTRIB_THREAD_ARTS,
	ATTRIB_THREAD_PERC,
	ATTRIB_SHOW_AUTHOR,
	ATTRIB_SHOW_INFO,
	ATTRIB_SORT_ART_TYPE,
	ATTRIB_POST_PROC_TYPE,
	ATTRIB_QUICK_KILL_HEADER,
	ATTRIB_QUICK_KILL_SCOPE,
	ATTRIB_QUICK_KILL_EXPIRE,
	ATTRIB_QUICK_KILL_CASE,
	ATTRIB_QUICK_SELECT_HEADER,
	ATTRIB_QUICK_SELECT_SCOPE,
	ATTRIB_QUICK_SELECT_EXPIRE,
	ATTRIB_QUICK_SELECT_CASE,
	ATTRIB_MAILING_LIST,
	ATTRIB_X_HEADERS,
	ATTRIB_X_BODY,
	ATTRIB_AUTO_SAVE_MSG, /* TODO: what is this? dublicate of ATTRIB_AUTO_SAVE? */
	ATTRIB_X_COMMENT_TO,
	ATTRIB_FCC,
	ATTRIB_NEWS_QUOTE,
	ATTRIB_QUOTE_CHARS,
	ATTRIB_MIME_TYPES_TO_SAVE,
	ATTRIB_MIME_FORWARD,
#ifdef HAVE_ISPELL
		ATTRIB_ISPELL,
#endif /* HAVE_ISPELL */
	ATTRIB_SORT_THREADS_TYPE,
	ATTRIB_TEX2ISO_CONV
#ifdef CHARSET_CONVERSION
	,ATTRIB_MM_NETWORK_CHARSET
	,ATTRIB_UNDECLARED_CHARSET
#endif /* CHARSET_CONVERSION */
};

/*
 * Local prototypes
 */
static void do_set_attrib(struct t_group *group, int type, const char *data);
static void read_attributes_file(t_bool global_file, t_bool startup);
static void set_attrib(int type, const char *scope, const char *data);
static void set_default_attributes(struct t_attribute *attributes);
#if 0 /* unused */
#	ifdef DEBUG
	static void dump_attributes(void);
#	endif /* DEBUG */
#endif /* 0 */

/*
 * Global attributes. This is attached to all groups that have no
 * specific attributes.
 */
struct t_attribute glob_attributes;

/*
 * Per group attributes. This fills out a basic template of defaults
 * before the attributes in the current scope are applied.
 */
static void
set_default_attributes(
	struct t_attribute *attributes)
{
	attributes->global = FALSE;	/* global/group specific */
	attributes->maildir = tinrc.maildir;
	attributes->savedir = tinrc.savedir;
	attributes->savefile = NULL;
	attributes->sigfile = tinrc.sigfile;
	attributes->organization = (*default_organization ? default_organization : NULL);
	attributes->followup_to = NULL;
	attributes->mailing_list = NULL;
	attributes->x_headers = NULL;
	attributes->x_body = NULL;
	attributes->from = tinrc.mail_address;
	attributes->news_quote_format = tinrc.news_quote_format;
	attributes->quote_chars = tinrc.quote_chars;
	attributes->mime_types_to_save = my_strdup("*/*");
#ifdef HAVE_ISPELL
	attributes->ispell = NULL;
#endif /* HAVE_ISPELL */
	attributes->quick_kill_scope = (tinrc.default_filter_kill_global ? my_strdup("*") : NULL);
	attributes->quick_kill_header = tinrc.default_filter_kill_header;
	attributes->quick_kill_case = tinrc.default_filter_kill_case;
	attributes->quick_kill_expire = tinrc.default_filter_kill_expire;
	attributes->quick_select_scope = (tinrc.default_filter_select_global ? my_strdup("*") : NULL);
	attributes->quick_select_header = tinrc.default_filter_select_header;
	attributes->quick_select_case = tinrc.default_filter_select_case;
	attributes->quick_select_expire = tinrc.default_filter_select_expire;
	attributes->show_only_unread = tinrc.show_only_unread_arts;
	attributes->thread_arts = tinrc.thread_articles;
	attributes->thread_perc = tinrc.thread_perc;
	attributes->sort_art_type = tinrc.sort_article_type;
	attributes->sort_threads_type = tinrc.sort_threads_type;
	attributes->show_info = tinrc.show_info;
	attributes->show_author = tinrc.show_author;
	attributes->auto_save = tinrc.auto_save;
	attributes->auto_select = FALSE;
	attributes->batch_save = tinrc.batch_save;
	attributes->delete_tmp_files = FALSE;
	attributes->post_proc_type = tinrc.post_process;
	attributes->x_comment_to = FALSE;
	attributes->tex2iso_conv = tinrc.tex2iso_conv;
	attributes->mime_forward = FALSE;
	attributes->fcc = NULL;
#ifdef CHARSET_CONVERSION
	attributes->mm_network_charset = tinrc.mm_network_charset;
	attributes->undeclared_charset = NULL;
#endif /* CHARSET_CONVERSION */
}


#define MATCH_BOOLEAN(pattern, type) \
	if (match_boolean(line, pattern, &flag)) { \
		num = (flag != FALSE); \
		set_attrib(type, scope, (char *) &num); \
		found = TRUE; \
		break; \
	}
#define MATCH_INTEGER(pattern, type, maxval) \
	if (match_integer(line, pattern, &num, maxval)) { \
		set_attrib(type, scope, (char *) &num); \
		found = TRUE; \
		break; \
	}
#define MATCH_STRING(pattern, type) \
	if (match_string(line, pattern, buf, sizeof(buf) - strlen(pattern))) { \
		set_attrib(type, scope, buf); \
		found = TRUE; \
		break; \
	}
#ifdef CHARSET_CONVERSION
#	define MATCH_LIST(pattern, type, table, tablelen) \
		if (match_list(line, pattern, table, tablelen, &num)) { \
			set_attrib(type, scope, (char *) &num); \
			found = TRUE; \
			break; \
		}
#endif /* CHARSET_CONVERSION */
#if !defined(CHARSET_CONVERSION) || !defined(HAVE_ISPELL)
#	define SKIP_ITEM(pattern) \
		if (!strncmp(line, pattern, strlen(pattern))) { \
			found = TRUE; \
			break; \
		}
#endif /* !CHARSET_CONVERSION || !HAVE_ISPELL */


/*
 * (re)read global/local attributes file
 */
void
read_attributes_files(
	void)
{
	static t_bool startup = TRUE;

	if (!startup) { /* reinit attributes */
		free_attributes_array();
		read_attributes_file(TRUE, startup);
		read_attributes_file(FALSE, startup);
	} else {
		if (!batch_mode || verbose)
			wait_message(0, _(txt_reading_attributes_file), _(txt_global));
		read_attributes_file(TRUE, startup);
		if (!batch_mode || verbose)
			wait_message(0, _(txt_reading_attributes_file), "");
		read_attributes_file(FALSE, startup);
		startup = FALSE;
	}
}


static void
read_attributes_file(
	t_bool global_file,
	t_bool startup)
{
	FILE *fp;
	char *file;
	char buf[LEN];
	char line[LEN];
	char scope[LEN];
	int num;
	int i;
	int upgrade = RC_CHECK;
	t_bool flag, found = FALSE;

	/*
	 * Initialize global attributes even if there is no global file
	 * These setting are used as the default for all groups unless overridden
	 */
	if (global_file) {
		set_default_attributes(&glob_attributes);
		glob_attributes.global = TRUE;
		file = global_attributes_file;
	} else
		file = local_attributes_file;

	if ((fp = fopen(file, "r")) != NULL) {
		scope[0] = '\0';
		while (fgets(line, (int) sizeof(line), fp) != NULL) {
			if (line[0] == '#' || line[0] == '\n') {
				if (!global_file && startup && upgrade == RC_CHECK)
					upgrade = check_upgrade(line, "# Group attributes file V", ATTRIBUTES_VERSION);
				continue;
			}

			switch (tolower((unsigned char) line[0])) {
				case 'a':
					MATCH_BOOLEAN("auto_save=", ATTRIB_AUTO_SAVE);
					MATCH_BOOLEAN("auto_select=", ATTRIB_AUTO_SELECT);
					break;

				case 'b':
					MATCH_BOOLEAN("batch_save=", ATTRIB_BATCH_SAVE);
					break;

				case 'd':
					MATCH_BOOLEAN("delete_tmp_files=", ATTRIB_DELETE_TMP_FILES);
					break;

				case 'f':
					MATCH_STRING("fcc=", ATTRIB_FCC);
					MATCH_STRING("followup_to=", ATTRIB_FOLLOWUP_TO);
					MATCH_STRING("from=", ATTRIB_FROM);
					break;

				case 'i':
#ifdef HAVE_ISPELL
					MATCH_STRING("ispell=", ATTRIB_ISPELL);
#else
					SKIP_ITEM("ispell=");
#endif /* HAVE_ISPELL */
					break;

				case 'm':
					MATCH_STRING("maildir=", ATTRIB_MAILDIR);
					MATCH_STRING("mailing_list=", ATTRIB_MAILING_LIST);
					MATCH_BOOLEAN("mime_forward=", ATTRIB_MIME_FORWARD);
					MATCH_STRING("mime_types_to_save=", ATTRIB_MIME_TYPES_TO_SAVE);
#ifdef CHARSET_CONVERSION
					MATCH_LIST("mm_network_charset=", ATTRIB_MM_NETWORK_CHARSET, txt_mime_charsets, NUM_MIME_CHARSETS);
#else
					SKIP_ITEM("mm_network_charset=");
#endif /* CHARSET_CONVERSION */
					break;

				case 'n':
					MATCH_STRING("news_quote_format=", ATTRIB_NEWS_QUOTE);
					break;

				case 'o':
					MATCH_STRING("organization=", ATTRIB_ORGANIZATION);
					break;

				case 'p':
					MATCH_INTEGER("post_proc_type=", ATTRIB_POST_PROC_TYPE, POST_PROC_YES);
					break;

				case 'q':
					MATCH_BOOLEAN("quick_kill_case=", ATTRIB_QUICK_KILL_CASE);
					MATCH_BOOLEAN("quick_kill_expire=", ATTRIB_QUICK_KILL_EXPIRE);
					MATCH_INTEGER("quick_kill_header=", ATTRIB_QUICK_KILL_HEADER, FILTER_LINES);
					MATCH_STRING("quick_kill_scope=", ATTRIB_QUICK_KILL_SCOPE);
					MATCH_BOOLEAN("quick_select_case=", ATTRIB_QUICK_SELECT_CASE);
					MATCH_BOOLEAN("quick_select_expire=", ATTRIB_QUICK_SELECT_EXPIRE);
					MATCH_INTEGER("quick_select_header=", ATTRIB_QUICK_SELECT_HEADER, FILTER_LINES);
					MATCH_STRING("quick_select_scope=", ATTRIB_QUICK_SELECT_SCOPE);
					if (match_string(line, "quote_chars=", buf, sizeof(buf))) {
						quote_dash_to_space(buf);
						set_attrib(ATTRIB_QUOTE_CHARS, scope, buf);
						found = TRUE;
						break;
					}
					break;

				case 's':
					MATCH_STRING("savedir=", ATTRIB_SAVEDIR);
					MATCH_STRING("savefile=", ATTRIB_SAVEFILE);
					if (match_string(line, "scope=", scope, sizeof(scope))) {
						found = TRUE;
						break;
					}
					MATCH_INTEGER("show_author=", ATTRIB_SHOW_AUTHOR, SHOW_FROM_BOTH);
					MATCH_INTEGER("show_info=", ATTRIB_SHOW_INFO, SHOW_INFO_BOTH);
					MATCH_BOOLEAN("show_only_unread=", ATTRIB_SHOW_ONLY_UNREAD);
					MATCH_STRING("sigfile=", ATTRIB_SIGFILE);
					MATCH_INTEGER("sort_art_type=", ATTRIB_SORT_ART_TYPE, SORT_ARTICLES_BY_LINES_ASCEND);
					MATCH_INTEGER("sort_threads_type=", ATTRIB_SORT_THREADS_TYPE, SORT_THREADS_BY_SCORE_DESCEND);
					break;

				case 't':
					MATCH_BOOLEAN("tex2iso_conv=", ATTRIB_TEX2ISO_CONV);
					MATCH_INTEGER("thread_arts=", ATTRIB_THREAD_ARTS, THREAD_MAX);
					MATCH_INTEGER("thread_perc=", ATTRIB_THREAD_PERC, 100);
					break;

				case 'u':
#ifdef CHARSET_CONVERSION
					MATCH_STRING("undeclared_charset=", ATTRIB_UNDECLARED_CHARSET);
#else
					SKIP_ITEM("undeclared_charset=");
#endif /* CHARSET_CONVERSION */
					break;

				case 'x':
					MATCH_STRING("x_body=", ATTRIB_X_BODY);
					MATCH_BOOLEAN("x_comment_to=", ATTRIB_X_COMMENT_TO);
					MATCH_STRING("x_headers=", ATTRIB_X_HEADERS);
					break;

				default:
					break;
			}

			if (found)
				found = FALSE;
			else /* TODO: surpress error messages on non intial reads? */
				error_message(_(txt_bad_attrib), line);
		}
		fclose(fp);

		/*
		 * TODO: do something usfull for the other cases
		 */
		if (upgrade == RC_UPGRADE && !global_file)
			write_attributes_file(file);
	}

	/*
	 * Now setup the rest of the groups to use the default attributes
	 */
	if (!global_file) {
		for_each_group(i) {
			if (!active[i].attribute)
				active[i].attribute = &glob_attributes;
		}
	}
/* debug_print_filter_attributes(); */
}


static void
set_attrib(
	int type,
	const char *scope,
	const char *data)
{
	struct t_group *group;

	if (scope == NULL || *scope == '\0') {	/* No active scope set yet */
		/* TODO: include full line in error-message */
		error_message("attribute with no scope: %s", data);
		return;
	}
#if 0
	fprintf(stderr, "set_attrib #%d %s %s(%d)\n", type, scope, data, (int) *data);
#endif /* 0 */

	/*
	 * Does scope refer to 1 or more than 1 group
	 */
	if (!strpbrk(scope, "*,")) {
		if ((group = group_find(scope)) != NULL)
			do_set_attrib(group, type, data);
	} else {
		int i;

		/* TODO: Can we get out of doing this per group for .global case */
		for_each_group(i) {
			group = &active[i];
			if (match_group_list(group->name, scope))
				do_set_attrib(group, type, data);
		}
	}
}


#define SET_STRING(string) \
	group->attribute->string = my_strdup(data); \
	break
#define SET_INTEGER(integer) \
	group->attribute->integer = *data; \
	break

static void
do_set_attrib(
	struct t_group *group,
	int type,
	const char *data)
{
	/*
	 * Setup default attributes for this group if none already set
	 */
	if (group->attribute == NULL) {
		group->attribute = my_malloc(sizeof(struct t_attribute));
		set_default_attributes(group->attribute);
	}

	/*
	 * Now set the required attribute
	 */
	switch (type) {
		case ATTRIB_MAILDIR:
			free_if_not_default(&group->attribute->maildir, tinrc.maildir);
			SET_STRING(maildir);

		case ATTRIB_SAVEDIR:
			free_if_not_default(&group->attribute->savedir, tinrc.savedir);
			SET_STRING(savedir);

		case ATTRIB_SAVEFILE:
			FreeIfNeeded(group->attribute->savefile);
			SET_STRING(savefile);

		case ATTRIB_ORGANIZATION:
			free_if_not_default(&group->attribute->organization, default_organization);
			SET_STRING(organization);

		case ATTRIB_FROM:
			free_if_not_default(&group->attribute->from, tinrc.mail_address);
			SET_STRING(from);

		case ATTRIB_SIGFILE:
			free_if_not_default(&group->attribute->sigfile, tinrc.sigfile);
			SET_STRING(sigfile);

		case ATTRIB_FOLLOWUP_TO:
			FreeIfNeeded(group->attribute->followup_to);
			SET_STRING(followup_to);

		case ATTRIB_AUTO_SELECT:
			SET_INTEGER(auto_select);

		case ATTRIB_AUTO_SAVE:
			SET_INTEGER(auto_save);

		case ATTRIB_BATCH_SAVE:
			SET_INTEGER(batch_save);

		case ATTRIB_DELETE_TMP_FILES:
			SET_INTEGER(delete_tmp_files);

		case ATTRIB_SHOW_ONLY_UNREAD:
			SET_INTEGER(show_only_unread);

		case ATTRIB_THREAD_ARTS:
			SET_INTEGER(thread_arts);

		case ATTRIB_THREAD_PERC:
			SET_INTEGER(thread_perc);

		case ATTRIB_SHOW_AUTHOR:
			SET_INTEGER(show_author);

		case ATTRIB_SHOW_INFO:
			SET_INTEGER(show_info);

		case ATTRIB_SORT_ART_TYPE:
			SET_INTEGER(sort_art_type);

		case ATTRIB_SORT_THREADS_TYPE:
			SET_INTEGER(sort_threads_type);

		case ATTRIB_POST_PROC_TYPE:
			SET_INTEGER(post_proc_type);

		case ATTRIB_QUICK_KILL_HEADER:
			SET_INTEGER(quick_kill_header);

		case ATTRIB_QUICK_KILL_SCOPE:
			FreeIfNeeded(group->attribute->quick_kill_scope);
			SET_STRING(quick_kill_scope);

		case ATTRIB_QUICK_KILL_EXPIRE:
			SET_INTEGER(quick_kill_expire);

		case ATTRIB_QUICK_KILL_CASE:
			SET_INTEGER(quick_kill_case);

		case ATTRIB_QUICK_SELECT_HEADER:
			SET_INTEGER(quick_select_header);

		case ATTRIB_QUICK_SELECT_SCOPE:
			FreeIfNeeded(group->attribute->quick_select_scope);
			SET_STRING(quick_select_scope);

		case ATTRIB_QUICK_SELECT_EXPIRE:
			SET_INTEGER(quick_select_expire);

		case ATTRIB_QUICK_SELECT_CASE:
			SET_INTEGER(quick_select_case);

		case ATTRIB_MAILING_LIST:
			FreeIfNeeded(group->attribute->mailing_list);
			SET_STRING(mailing_list);

#ifdef CHARSET_CONVERSION
		case ATTRIB_MM_NETWORK_CHARSET:
			SET_INTEGER(mm_network_charset);

		case ATTRIB_UNDECLARED_CHARSET:
			FreeIfNeeded(group->attribute->undeclared_charset);
			SET_STRING(undeclared_charset);
#endif /* CHARSET_CONVERSION */

		case ATTRIB_X_HEADERS:
			FreeIfNeeded(group->attribute->x_headers);
			SET_STRING(x_headers);

		case ATTRIB_X_BODY:
			FreeIfNeeded(group->attribute->x_body);
			SET_STRING(x_body);

		case ATTRIB_X_COMMENT_TO:
			SET_INTEGER(x_comment_to);

		case ATTRIB_FCC:
			FreeIfNeeded(group->attribute->fcc);
			SET_STRING(fcc);

		case ATTRIB_NEWS_QUOTE:
			free_if_not_default(&group->attribute->news_quote_format, tinrc.news_quote_format);
			SET_STRING(news_quote_format);

		case ATTRIB_QUOTE_CHARS:
			free_if_not_default(&group->attribute->quote_chars, tinrc.quote_chars);
			SET_STRING(quote_chars);

		case ATTRIB_MIME_TYPES_TO_SAVE:
			FreeIfNeeded(group->attribute->mime_types_to_save);
			SET_STRING(mime_types_to_save);

		case ATTRIB_MIME_FORWARD:
			SET_INTEGER(mime_forward);

#ifdef HAVE_ISPELL
		case ATTRIB_ISPELL:
			FreeIfNeeded(group->attribute->ispell);
			SET_STRING(ispell);
#endif /* HAVE_ISPELL */

		case ATTRIB_TEX2ISO_CONV:
			SET_INTEGER(tex2iso_conv);

		default:
			break;
	}
}


/*
 * Save the group attributes from active[].attribute to ~/.tin/attributes
 *
 */
void
write_attributes_file(
	const char *file)
{
	FILE *fp, *infp;
	char *new_file;
	int i;
	t_bool copy_ok = TRUE;

	if (no_write && file_size(file) != -1L)
		return;

	new_file = get_tmpfilename(file);

	if ((fp = fopen(new_file, "w" FOPEN_OPTS)) == NULL) {
		error_message(_(txt_filesystem_full_backup), ATTRIBUTES_FILE);
		free(new_file);
		return;
	}

	if (!cmd_line && !batch_mode)
		wait_message(0, _(txt_writing_attributes_file));

	/*
	 * TODO: sort in a useful order
	 *       move strings to lang.c
	 */
	fprintf(fp, "# Group attributes file V%s for the TIN newsreader\n", ATTRIBUTES_VERSION);
	fprintf(fp, _("# Do not edit this comment block\n#\n"));
	fprintf(fp, _("#  scope=STRING (ie. alt.*,!alt.bin*) [mandatory]\n"));
	fprintf(fp, _("#  maildir=STRING (ie. ~/Mail)\n"));
	fprintf(fp, _("#  savedir=STRING (ie. ~user/News)\n"));
	fprintf(fp, _("#  savefile=STRING (ie. =linux)\n"));
	fprintf(fp, _("#  sigfile=STRING (ie. $var/sig)\n"));
	fprintf(fp, _("#  organization=STRING (if beginning with '/' read from file)\n"));
	fprintf(fp, _("#  followup_to=STRING\n"));
	fprintf(fp, _("#  mailing_list=STRING (ie. majordomo@example.org)\n"));
	fprintf(fp, _("#  x_headers=STRING (ie. ~/.tin/extra-headers)\n"));
	fprintf(fp, _("#  x_body=STRING (ie. ~/.tin/extra-body-text)\n"));
	fprintf(fp, _("#  from=STRING (just append wanted From:-line, don't use quotes)\n"));
	fprintf(fp, _("#  news_quote_format=STRING\n"));
	fprintf(fp, _("#  quote_chars=STRING (%%s, %%S for initials)\n"));
	fprintf(fp, _("#  mime_types_to_save=STRING (eg. image/*,!image/bmp)\n"));
#ifdef HAVE_ISPELL
	fprintf(fp, _("#  ispell=STRING\n"));
#endif /* HAVE_ISPELL */
	fprintf(fp, _("#  auto_select=ON/OFF\n"));
	fprintf(fp, _("#  auto_save=ON/OFF\n"));
	fprintf(fp, _("#  batch_save=ON/OFF\n"));
	fprintf(fp, _("#  delete_tmp_files=ON/OFF\n"));
	fprintf(fp, _("#  show_only_unread=ON/OFF\n"));
	fprintf(fp, _("#  thread_arts=NUM"));
	for (i = 0; i <= THREAD_MAX; i++) {
		if (!(i % 2))
			fprintf(fp, "\n#    ");
		fprintf(fp, "%d=%s, ", i, _(txt_threading[i]));
	}
	fprintf(fp, "\n");
	fprintf(fp, _("#  thread_perc=NUM\n"));
	fprintf(fp, _("#  show_author=NUM\n"));
	fprintf(fp, "#    %d=%s, %d=%s, %d=%s, %d=%s\n",
		SHOW_FROM_NONE, _(txt_show_from[SHOW_FROM_NONE]),
		SHOW_FROM_ADDR, _(txt_show_from[SHOW_FROM_ADDR]),
		SHOW_FROM_NAME, _(txt_show_from[SHOW_FROM_NAME]),
		SHOW_FROM_BOTH, _(txt_show_from[SHOW_FROM_BOTH]));
	fprintf(fp, _("#  show_info=NUM\n"));
	fprintf(fp, "#    %d=%s, %d=%s, %d=%s, %d=%s\n",
		SHOW_INFO_NOTHING, _(txt_show_info_type[SHOW_INFO_NOTHING]),
		SHOW_INFO_LINES, _(txt_show_info_type[SHOW_INFO_LINES]),
		SHOW_INFO_SCORE, _(txt_show_info_type[SHOW_INFO_SCORE]),
		SHOW_INFO_BOTH, _(txt_show_info_type[SHOW_INFO_BOTH]));
	fprintf(fp, _("#  sort_art_type=NUM\n"));
	fprintf(fp, "#    %d=%s,\n",
		SORT_ARTICLES_BY_NOTHING, _(txt_sort_a_type[SORT_ARTICLES_BY_NOTHING]));
	fprintf(fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_SUBJ_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SUBJ_DESCEND]),
		SORT_ARTICLES_BY_SUBJ_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SUBJ_ASCEND]));
	fprintf(fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_FROM_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_FROM_DESCEND]),
		SORT_ARTICLES_BY_FROM_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_FROM_ASCEND]));
	fprintf(fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_DATE_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_DATE_DESCEND]),
		SORT_ARTICLES_BY_DATE_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_DATE_ASCEND]));
	fprintf(fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_SCORE_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SCORE_DESCEND]),
		SORT_ARTICLES_BY_SCORE_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SCORE_ASCEND]));
	fprintf(fp, "#    %d=%s, %d=%s\n",
		SORT_ARTICLES_BY_LINES_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_LINES_DESCEND]),
		SORT_ARTICLES_BY_LINES_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_LINES_ASCEND]));
	fprintf(fp, _("#  sort_threads_type=NUM\n"));
	fprintf(fp, "#    %d=%s, %d=%s, %d=%s\n",
		SORT_THREADS_BY_NOTHING, _(txt_sort_t_type[SORT_THREADS_BY_NOTHING]),
		SORT_THREADS_BY_SCORE_DESCEND, _(txt_sort_t_type[SORT_THREADS_BY_SCORE_DESCEND]),
		SORT_THREADS_BY_SCORE_ASCEND, _(txt_sort_t_type[SORT_THREADS_BY_SCORE_ASCEND]));
	fprintf(fp, _("#  post_proc_type=NUM\n"));
	fprintf(fp, "#    %d=%s, %d=%s, %d=%s\n",
		POST_PROC_NO, _(txt_post_process_type[POST_PROC_NO]),
		POST_PROC_SHAR, _(txt_post_process_type[POST_PROC_SHAR]),
		POST_PROC_YES, _(txt_post_process_type[POST_PROC_YES]));
	fprintf(fp, _("#  quick_kill_scope=STRING (ie. talk.*)\n"));
	fprintf(fp, _("#  quick_kill_expire=ON/OFF\n"));
	fprintf(fp, _("#  quick_kill_case=ON/OFF\n"));
	fprintf(fp, _("#  quick_kill_header=NUM\n"));
	fprintf(fp, _("#    0=subj (case sensitive) 1=subj (ignore case)\n"));
	fprintf(fp, _("#    2=from (case sensitive) 3=from (ignore case)\n"));
	fprintf(fp, _("#    4=msgid 5=lines\n"));
	fprintf(fp, _("#  quick_select_scope=STRING\n"));
	fprintf(fp, _("#  quick_select_expire=ON/OFF\n"));
	fprintf(fp, _("#  quick_select_case=ON/OFF\n"));
	fprintf(fp, _("#  quick_select_header=NUM\n"));
	fprintf(fp, _("#    0=subj (case sensitive) 1=subj (ignore case)\n"));
	fprintf(fp, _("#    2=from (case sensitive) 3=from (ignore case)\n"));
	fprintf(fp, _("#    4=msgid 5=lines\n"));
	fprintf(fp, _("#  x_comment_to=ON/OFF\n"));
	fprintf(fp, _("#  fcc=STRING (eg. =mailbox)\n"));
	fprintf(fp, _("#  tex2iso_conv=ON/OFF\n"));
	fprintf(fp, _("#  mime_forward=ON/OFF\n"));
#ifdef CHARSET_CONVERSION
	fprintf(fp, _("#  mm_network_charset=supported_charset"));
	for (i = 0; i < NUM_MIME_CHARSETS; i++) {
		if (!(i % 5)) /* start new line */
			fprintf(fp, "\n#    ");
		fprintf(fp, "%s, ", txt_mime_charsets[i]);
	}
	fprintf(fp, "\n");
	fprintf(fp, _("#  undeclared_charset=STRING (default is US-ASCII)\n"));
#endif /* CHARSET_CONVERSION */
	fprintf(fp, _("#\n# Note that it is best to put general (global scoping)\n"));
	fprintf(fp, _("# entries first followed by group specific entries.\n#\n"));
	fprintf(fp, _("############################################################################\n\n"));

	/*
	 * Add some useful defaults if no attributes file currently exists
	 */
	if ((infp = fopen(file, "r" FOPEN_OPTS)) == NULL) {
		fprintf(fp, _("# include extra headers\n"));
		fprintf(fp, "scope=*\n");
		/*
		 * ${TIN_HOMEDIR-HOME} would be correct, but tin doesn't expand it,
		 * so we take ~ instead
		 */
		fprintf(fp, "x_headers=~/.tin/headers\n\n");

		fprintf(fp, _("# in *sources* set post process type to shar only\n"));
		fprintf(fp, "scope=*sources*\n");
		fprintf(fp, "post_proc_type=%d\n\n", POST_PROC_SHAR);

		fprintf(fp, _("# in *binaries* do full post processing but no TeX2ISO conversion,\n"));
		fprintf(fp, _("# remove tmp files and set Followup-To: poster\n"));
		fprintf(fp, "scope=*binaries*\n");
		fprintf(fp, "post_proc_type=%d\n", POST_PROC_YES);
		fprintf(fp, "tex2iso_conv=OFF\n");
		fprintf(fp, "delete_tmp_files=ON\n");
		fprintf(fp, "followup_to=poster\n\n");
	} else {
		char *ptr;
		char buf[LEN];

		/*
		 * Until the write code is fixed, just copy the existing settings after
		 * the updated header
		 */
		while ((ptr = fgets(buf, sizeof(buf), infp)) != NULL) {
			if (ptr[0] != '#' )				/* End of headers */
				break;
		}
		copy_ok = copy_fp(infp, fp);
		fclose(infp);
	}

	/* rename_file() preserves mode, so this is safe */
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	if (ferror(fp) || fclose(fp))
		error_message(_(txt_filesystem_full), ATTRIBUTES_FILE);
	else if (copy_ok)
		rename_file(new_file, file);
	else
		unlink (new_file);

	free(new_file);
	return;

#if 0 /* FIXME */
	for_each_group(i) {
		struct t_attribute *attr = active[i].attribute;

		/* TODO: only write out none empty && no defaut entrys */
		fprintf(fp, "scope=%s\n", active[i].name);
		fprintf(fp, "maildir=%s\n", attr->maildir);
		fprintf(fp, "savedir=%s\n", attr->savedir);
		fprintf(fp, "savefile=%s\n", attr->savefile);
		fprintf(fp, "sigfile=%s\n", attr->sigfile);
		fprintf(fp, "organization=%s\n", attr->organization);
		fprintf(fp, "followup_to=%s\n", attr->followup_to);
		fprintf(fp, "mailing_list=%s\n", attr->mailing_list);
		fprintf(fp, "x_headers=%s\n", attr->x_headers);
		fprintf(fp, "x_body=%s\n", attr->x_body);
		fprintf(fp, "from=%s\n", attr->from);
		fprintf(fp, "news_quote_format=%s\n", attr->news_quote_format);
		fprintf(fp, "quote_chars=%s\n", quote_space_to_dash(attr->quote_chars));
		fprintf(fp, "mime_types_to_save=%s\n", attr->mime_types_to_save);
#	ifdef HAVE_ISPELL
		fprintf(fp, "ispell=%s\n", attr->ispell);
#	endif /* HAVE_ISPELL */
		fprintf(fp, "show_only_unread=%s\n", print_boolean(attr->show_only_unread));
		fprintf(fp, "thread_arts=%d\n", attr->thread_arts);
		fprintf(fp, "thread_perc=%d\n", attr->thread_perc);
		fprintf(fp, "auto_select=%s\n", print_boolean(attr->auto_select));
		fprintf(fp, "auto_save=%s\n", print_boolean(attr->auto_save));
		fprintf(fp, "batch_save=%s\n", print_boolean(attr->batch_save));
		fprintf(fp, "delete_tmp_files=%s\n", print_boolean(attr->delete_tmp_files));
		fprintf(fp, "sort_art_type=%d\n", attr->sort_art_type);
		fprintf(fp, "sort_threads_type=%d\n", attr->sort_threads_type);
		fprintf(fp, "show_author=%d\n", attr->show_author);
		fprintf(fp, "show_info=%d\n", attr->show_info);
		fprintf(fp, "post_proc_type=%d\n", attr->post_proc_type);
		fprintf(fp, "quick_kill_scope=%s\n", attr->quick_kill_scope);
		fprintf(fp, "quick_kill_case=%s\n", print_boolean(attr->quick_kill_case));
		fprintf(fp, "quick_kill_expire=%s\n", print_boolean(attr->quick_kill_expire));
		fprintf(fp, "quick_kill_header=%d\n", attr->quick_kill_header);
		fprintf(fp, "quick_select_scope=%s\n", attr->quick_select_scope);
		fprintf(fp, "quick_select_case=%s\n", print_boolean(attr->quick_select_case));
		fprintf(fp, "quick_select_expire=%s\n", print_boolean(attr->quick_select_expire));
		fprintf(fp, "quick_select_header=%d\n", attr->quick_select_header);
		fprintf(fp, "x_comment_to=%s\n", print_boolean(attr->x_comment_to));
		fprintf(fp, "fcc=%s\n", attr->fcc);
		fprintf(fp, "tex2iso_conv=%s\n", print_boolean(attr->tex2iso_conv));
		fprintf(fp, "mime_forward=%s\n", print_boolean(attr->mime_forward));
#	ifdef CHARSET_CONVERSION
		fprintf(fp, "mm_network_charset=%s\n", txt_mime_charsets[attr->mm_network_charset]);
		fprintf(fp, "undeclared_charset=%s\n", attr->undeclared_charset);
#	endif /* CHARSET_CONVERSION */
	}
#endif /* 0 */
}


#if 0
void
debug_print_filter_attributes(
	void)
{
	int i;
	struct t_group *group;

	my_printf("\nBEG ***\n");

	for_each_group(i) {
		group = &active[i];
		my_printf("Grp=[%s] KILL   header=[%d] scope=[%s] case=[%s] expire=[%s]\n",
			group->name, group->attribute->quick_kill_header,
			BlankIfNull(group->attribute->quick_kill_scope),
			txt_onoff[group->attribute->quick_kill_case != FALSE ? 1 : 0],
			txt_onoff[group->attribute->quick_kill_expire != FALSE ? 1 : 0]);
		my_printf("Grp=[%s] SELECT header=[%d] scope=[%s] case=[%s] expire=[%s]\n",
			group->name, group->attribute->quick_select_header,
			BlankIfNull(group->attribute->quick_select_scope),
			txt_onoff[group->attribute->quick_select_case != FALSE ? 1 : 0],
			txt_onoff[group->attribute->quick_select_expire != FALSE ? 1 : 0]);
	}

	my_printf("END ***\n");
}


#	ifdef DEBUG
static void
dump_attributes(
	void)
{
	int i;
	struct t_group *group;

	fprintf(stderr, "DUMP attributes\n");

	for_each_group(i) {
		group = &active[i];
		if (!group->attribute)
			continue;
		fprintf(stderr, "scope=%s\n", group->name);
		fprintf(stderr, "\tGlobal=%d\n", group->attribute->global);
		fprintf(stderr, "\tmaildir=%s\n", group->attribute->maildir);
		fprintf(stderr, "\tsavedir=%s\n", group->attribute->savedir);
		fprintf(stderr, "\tsavefile=%s\n", group->attribute->savefile);
		fprintf(stderr, "\tsigfile=%s\n", group->attribute->sigfile);
		fprintf(stderr, "\torganization=%s\n", group->attribute->organization);
		fprintf(stderr, "\tfollowup_to=%s\n", group->attribute->followup_to);
		fprintf(stderr, "\tmailing_list=%s\n", group->attribute->mailing_list);
		fprintf(stderr, "\tx_headers=%s\n", group->attribute->x_headers);
		fprintf(stderr, "\tx_body=%s\n", group->attribute->x_body);
		fprintf(stderr, "\tfrom=%s\n", group->attribute->from);
		fprintf(stderr, "\tnews_quote_format=%s\n", group->attribute->news_quote_format);
		fprintf(stderr, "\tquote_chars=%s\n", quote_space_to_dash(group->attribute->quote_chars));
		fprintf(stderr, "\tmime_types_to_save=%s\n", group->attribute->mime_types_to_save);
#		ifdef HAVE_ISPELL
		fprintf(stderr, "\tispell=%s\n", group->attribute->ispell);
#		endif /* HAVE_ISPELL */
		fprintf(stderr, "\tshow_only_unread=%s\n", print_boolean(group->attribute->show_only_unread));
		fprintf(stderr, "\tthread_arts=%d\n", group->attribute->thread_arts);
		fprintf(stderr, "\tthread_perc=%d\n", group->attribute->thread_perc);
		fprintf(stderr, "\tauto_select=%s\n", print_boolean(group->attribute->auto_select));
		fprintf(stderr, "\tauto_save=%s\n", print_boolean(group->attribute->auto_save));
		fprintf(stderr, "\tbatch_save=%s\n", print_boolean(group->attribute->batch_save));
		fprintf(stderr, "\tdelete_tmp_files=%s\n", print_boolean(group->attribute->delete_tmp_files));
		fprintf(stderr, "\tsort_art_type=%d\n", group->attribute->sort_art_type);
		fprintf(stderr, "\tsort_threads_type=%d\n", group->attribute->sort_threads_type);
		fprintf(stderr, "\tshow_author=%d\n", group->attribute->show_author);
		fprintf(stderr, "\tshow_info=%d\n", group->attribute->show_info);
		fprintf(stderr, "\tpost_proc_type=%d\n", group->attribute->post_proc_type);
		fprintf(stderr, "\tquick_kill_scope=%s\n", group->attribute->quick_kill_scope);
		fprintf(stderr, "\tquick_kill_case=%s\n", print_boolean(group->attribute->quick_kill_case));
		fprintf(stderr, "\tquick_kill_expire=%s\n", print_boolean(group->attribute->quick_kill_expire));
		fprintf(stderr, "\tquick_kill_header=%d\n", group->attribute->quick_kill_header);
		fprintf(stderr, "\tquick_select_scope=%s\n", group->attribute->quick_select_scope);
		fprintf(stderr, "\tquick_select_case=%s\n", print_boolean(group->attribute->quick_select_case));
		fprintf(stderr, "\tquick_select_expire=%s\n", print_boolean(group->attribute->quick_select_expire));
		fprintf(stderr, "\tquick_select_header=%d\n", group->attribute->quick_select_header);
		fprintf(stderr, "\tx_comment_to=%s\n", print_boolean(group->attribute->x_comment_to));
		fprintf(stderr, "\tfcc=%s\n", group->attribute->fcc);
		fprintf(stderr, "\ttex2iso_conv=%s\n", print_boolean(group->attribute->tex2iso_conv));
		fprintf(stderr, "\tmime_forward=%s\n", print_boolean(group->attribute->mime_forward));
#		ifdef CHARSET_CONVERSION
		fprintf(stderr, "\tmm_network_charset=%s\n", txt_mime_charsets[group->attribute->mm_network_charset]);
		fprintf(stderr, "\tundeclared_charset=%s\n", group->attribute->undeclared_charset);
#		endif /* CHARSET_CONVERSION */
	}
}
#	endif /* DEBUG */
#endif /* 0 */
