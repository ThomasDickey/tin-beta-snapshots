/*
 *  Project   : tin - a Usenet reader
 *  Module    : attrib.c
 *  Author    : I. Lea
 *  Created   : 1993-12-01
 *  Updated   : 2002-06-09
 *  Notes     : Group attribute routines
 *
 * Copyright (c) 1993-2002 Iain Lea <iain@bricbrac.de>
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
	ATTRIB_SHOW_AUTHOR,
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
	ATTRIB_AUTO_SAVE_MSG,
	ATTRIB_X_COMMENT_TO,
	ATTRIB_NEWS_QUOTE,
	ATTRIB_QUOTE_CHARS,
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
static void do_set_attrib (struct t_group *group, int type, const char *data);
static void set_attrib (int type, const char *scope, const char *data);
static void set_default_attributes (struct t_attribute *attributes);
#if 0 /* unused */
#	ifdef DEBUG
	static void dump_attributes (void);
#	endif /* DEBUG */
#endif /* 0 */

/*
 * Global attributes. This is attached to all groups that have no
 * specific attributes.
 */
static struct t_attribute glob_attributes;
extern char global_attributes_file[PATH_LEN];

/*
 * Per group attributes. This fills out a basic template of defaults
 * before the attributes in the current scope are applied.
 */
static void
set_default_attributes (
	struct t_attribute *attributes)
{
	attributes->global = FALSE;	/* global/group specific */
	attributes->maildir = tinrc.maildir;
	attributes->savedir = tinrc.savedir;
	attributes->savefile = (char *) 0;
	attributes->sigfile = tinrc.sigfile;
	attributes->organization = (*default_organization ? default_organization : (char *) 0);
	attributes->followup_to = (char *) 0;
	attributes->mailing_list = (char *) 0;
	attributes->x_headers = (char *) 0;
	attributes->x_body = (char *) 0;
	attributes->from = tinrc.mail_address;
	attributes->news_quote_format = tinrc.news_quote_format;
	attributes->quote_chars = tinrc.quote_chars;
#ifdef HAVE_ISPELL
	attributes->ispell = (char *) 0;
#endif /* HAVE_ISPELL */
	attributes->quick_kill_scope = (tinrc.default_filter_kill_global ? my_strdup("*") : (char *) 0);
	attributes->quick_kill_header = tinrc.default_filter_kill_header;
	attributes->quick_kill_case = tinrc.default_filter_kill_case;
	attributes->quick_kill_expire = tinrc.default_filter_kill_expire;
	attributes->quick_select_scope = (tinrc.default_filter_select_global ? my_strdup("*") : (char *) 0);
	attributes->quick_select_header = tinrc.default_filter_select_header;
	attributes->quick_select_case = tinrc.default_filter_select_case;
	attributes->quick_select_expire = tinrc.default_filter_select_expire;
	attributes->show_only_unread = tinrc.show_only_unread_arts;
	attributes->thread_arts = tinrc.thread_articles;
	attributes->sort_art_type = tinrc.sort_article_type;
	attributes->sort_threads_type = tinrc.sort_threads_type;
	attributes->show_author = tinrc.show_author;
	attributes->auto_save = tinrc.auto_save;
	attributes->auto_select = FALSE;
	attributes->batch_save = tinrc.batch_save;
	attributes->delete_tmp_files = FALSE;
	attributes->post_proc_type = tinrc.post_process;
	attributes->x_comment_to = FALSE;
	attributes->tex2iso_conv = tinrc.tex2iso_conv;
#ifdef CHARSET_CONVERSION
	attributes->mm_network_charset = tinrc.mm_network_charset;
	attributes->undeclared_charset = (char *) 0;
#endif /* CHARSET_CONVERSION */
}


/*
 * Load global & local attributes into active[].attribute
 */

#define MATCH_BOOLEAN(pattern, type) \
	if (match_boolean (line, pattern, &flag)) { \
		num = (flag != FALSE); \
		set_attrib (type, scope, (char *)&num); \
		found = TRUE; \
		break; \
	}
#define MATCH_INTEGER(pattern, type, maxval) \
	if (match_integer (line, pattern, &num, maxval)) { \
		set_attrib (type, scope, (char *)&num); \
		found = TRUE; \
		break; \
	}
#define MATCH_STRING(pattern, type) \
	if (match_string (line, pattern, buf, sizeof (buf))) { \
		set_attrib (type, scope, buf); \
		found = TRUE; \
		break; \
	}
#define MATCH_LIST(pattern, type, table, tablelen) \
	if (match_list (line, pattern, table, tablelen, &num)) { \
		set_attrib (type, scope, (char *)&num); \
		found = TRUE; \
		break; \
	}


void
read_attributes_file (
	t_bool global_file)
{
	FILE *fp;
	char buf[LEN];
	char line[LEN];
	char scope[LEN];
	char *file;
	int num;
	register int i;
	t_bool flag, found = FALSE;

	/*
	 * Initialize global attributes even if there is no global file
	 * These setting are used as the default for all groups unless overridden
	 */
	if (global_file) {
		set_default_attributes (&glob_attributes);
		glob_attributes.global = TRUE;
		file = global_attributes_file;
	} else
		file = local_attributes_file;

	if ((fp = fopen (file, "r")) != NULL) {
		scope[0] = '\0';
		while (fgets (line, (int) sizeof (line), fp) != NULL) {
			if (line[0] == '#' || line[0] == '\n')
				continue;

			switch(tolower((unsigned char)line[0])) {
				case 'a':
					MATCH_BOOLEAN ("auto_save=", ATTRIB_AUTO_SAVE);
					MATCH_BOOLEAN ("auto_select=", ATTRIB_AUTO_SELECT);
					break;

				case 'b':
					MATCH_BOOLEAN ("batch_save=", ATTRIB_BATCH_SAVE);
					break;

				case 'd':
					MATCH_BOOLEAN ("delete_tmp_files=", ATTRIB_DELETE_TMP_FILES);
					break;

				case 'f':
					MATCH_STRING ("followup_to=", ATTRIB_FOLLOWUP_TO);
					MATCH_STRING ("from=", ATTRIB_FROM);
					break;

				case 'i':
#ifdef HAVE_ISPELL
					MATCH_STRING ("ispell=", ATTRIB_ISPELL);
#endif /* HAVE_ISPELL */
					break;

				case 'm':
					MATCH_STRING ("maildir=", ATTRIB_MAILDIR);
					MATCH_STRING ("mailing_list=", ATTRIB_MAILING_LIST);
#ifdef CHARSET_CONVERSION
					MATCH_LIST("mm_network_charset=", ATTRIB_MM_NETWORK_CHARSET,txt_mime_charsets, NUM_MIME_CHARSETS);
#endif /* CHARSET_CONVERSION */
					break;

				case 'n':
					MATCH_STRING ("news_quote_format=", ATTRIB_NEWS_QUOTE);
					break;

				case 'o':
					MATCH_STRING ("organization=", ATTRIB_ORGANIZATION);
					break;

				case 'p':
					MATCH_INTEGER ("post_proc_type=", ATTRIB_POST_PROC_TYPE, POST_PROC_UUDECODE);
					break;

				case 'q':
					MATCH_INTEGER ("quick_kill_header=", ATTRIB_QUICK_KILL_HEADER, FILTER_LINES);
					MATCH_STRING ("quick_kill_scope=", ATTRIB_QUICK_KILL_SCOPE);
					MATCH_BOOLEAN ("quick_kill_case=", ATTRIB_QUICK_KILL_CASE);
					MATCH_BOOLEAN ("quick_kill_expire=", ATTRIB_QUICK_KILL_EXPIRE);
					MATCH_INTEGER ("quick_select_header=", ATTRIB_QUICK_SELECT_HEADER, FILTER_LINES);
					MATCH_STRING ("quick_select_scope=", ATTRIB_QUICK_SELECT_SCOPE);
					MATCH_BOOLEAN ("quick_select_case=", ATTRIB_QUICK_SELECT_CASE);
					MATCH_BOOLEAN ("quick_select_expire=", ATTRIB_QUICK_SELECT_EXPIRE);
					if (match_string (line, "quote_chars=", buf, sizeof (buf))) {
						quote_dash_to_space (buf);
						set_attrib (ATTRIB_QUOTE_CHARS, scope, buf);
						found = TRUE;
						break;
					}
					break;

				case 's':
					MATCH_STRING ("savedir=", ATTRIB_SAVEDIR);
					MATCH_STRING ("savefile=", ATTRIB_SAVEFILE);
					if (match_string (line, "scope=", scope, sizeof (scope))) {
						found = TRUE;
						break;
					}
					MATCH_STRING ("sigfile=", ATTRIB_SIGFILE);
					MATCH_INTEGER ("show_author=", ATTRIB_SHOW_AUTHOR, SHOW_FROM_BOTH);
					MATCH_BOOLEAN ("show_only_unread=", ATTRIB_SHOW_ONLY_UNREAD);
					MATCH_INTEGER ("sort_art_type=", ATTRIB_SORT_ART_TYPE, SORT_ARTICLES_BY_LINES_ASCEND);
					MATCH_INTEGER ("sort_threads_type=", ATTRIB_SORT_THREADS_TYPE, SORT_THREADS_BY_SCORE_DESCEND);
					break;

				case 't':
					MATCH_BOOLEAN ("tex2iso_conv=", ATTRIB_TEX2ISO_CONV);
					MATCH_INTEGER ("thread_arts=", ATTRIB_THREAD_ARTS, THREAD_MAX);
					break;

				case 'u':
#ifdef CHARSET_CONVERSION
					MATCH_STRING ("undeclared_charset=", ATTRIB_UNDECLARED_CHARSET);
#endif /* CHARSET_CONVERSION */
					break;

				case 'x':
					MATCH_STRING ("x_body=", ATTRIB_X_BODY);
					MATCH_BOOLEAN ("x_comment_to=", ATTRIB_X_COMMENT_TO);
					MATCH_STRING ("x_headers=", ATTRIB_X_HEADERS);
					break;

				default:
					break;
			}

			if (found)
				found = FALSE;
			else
				error_message (_(txt_bad_attrib), line);
		}
		fclose (fp);
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
set_attrib (
	int type,
	const char *scope,
	const char *data)
{
	struct t_group *group;

	if (scope == NULL || *scope == '\0')	/* No active scope set yet */
		return;
#if 0
	fprintf(stderr, "set_attrib #%d %s %s(%d)\n", type, scope, data, (int)*data);
#endif /* 0 */

	/*
	 * Does scope refer to 1 or more than 1 group
	 */
	if (!strchr (scope, '*')) {
		if ((group = group_find (scope)) != NULL)
			do_set_attrib (group, type, data);
	} else {
		int i;
/* TODO Can we get out of doing this per group for .global case */
		for_each_group(i) {
			group = &active[i];
			if (match_group_list (group->name, scope))
				do_set_attrib (group, type, data);
		}
	}
}


#define SET_STRING(string) \
	group->attribute->string = my_strdup (data); \
	break
#define SET_INTEGER(integer) \
	group->attribute->integer = *data; \
	break

static void
do_set_attrib (
	struct t_group *group,
	int type,
	const char *data)
{
	/*
	 * Setup default attributes for this group if none already set
	 */
	if (group->attribute == NULL) {
		group->attribute = my_malloc (sizeof (struct t_attribute));
		set_default_attributes (group->attribute);
	}

	/*
	 * Now set the required attribute
	 */
	switch (type) {
		case ATTRIB_MAILDIR:
			SET_STRING(maildir);
		case ATTRIB_SAVEDIR:
			SET_STRING(savedir);
		case ATTRIB_SAVEFILE:
			SET_STRING(savefile);
		case ATTRIB_ORGANIZATION:
			SET_STRING(organization);
		case ATTRIB_FROM:
			SET_STRING(from);
		case ATTRIB_SIGFILE:
			SET_STRING(sigfile);
		case ATTRIB_FOLLOWUP_TO:
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
		case ATTRIB_SHOW_AUTHOR:
			SET_INTEGER(show_author);
		case ATTRIB_SORT_ART_TYPE:
			SET_INTEGER(sort_art_type);
		case ATTRIB_SORT_THREADS_TYPE:
			SET_INTEGER(sort_threads_type);
		case ATTRIB_POST_PROC_TYPE:
			SET_INTEGER(post_proc_type);
		case ATTRIB_QUICK_KILL_HEADER:
			SET_INTEGER(quick_kill_header);
		case ATTRIB_QUICK_KILL_SCOPE:
			SET_STRING(quick_kill_scope);
		case ATTRIB_QUICK_KILL_EXPIRE:
			SET_INTEGER(quick_kill_expire);
		case ATTRIB_QUICK_KILL_CASE:
			SET_INTEGER(quick_kill_case);
		case ATTRIB_QUICK_SELECT_HEADER:
			SET_INTEGER(quick_select_header);
		case ATTRIB_QUICK_SELECT_SCOPE:
			SET_STRING(quick_select_scope);
		case ATTRIB_QUICK_SELECT_EXPIRE:
			SET_INTEGER(quick_select_expire);
		case ATTRIB_QUICK_SELECT_CASE:
			SET_INTEGER(quick_select_case);
		case ATTRIB_MAILING_LIST:
			SET_STRING(mailing_list);
#ifdef CHARSET_CONVERSION
		case ATTRIB_MM_NETWORK_CHARSET:
			SET_INTEGER(mm_network_charset);
		case ATTRIB_UNDECLARED_CHARSET:
			SET_STRING(undeclared_charset);
#endif /* CHARSET_CONVERSION */
		case ATTRIB_X_HEADERS:
			SET_STRING(x_headers);
		case ATTRIB_X_BODY:
			SET_STRING(x_body);
		case ATTRIB_X_COMMENT_TO:
			SET_INTEGER(x_comment_to);
		case ATTRIB_NEWS_QUOTE:
			SET_STRING(news_quote_format);
		case ATTRIB_QUOTE_CHARS:
			SET_STRING(quote_chars);
#ifdef HAVE_ISPELL
		case ATTRIB_ISPELL:
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
 */
void
write_attributes_file (
	const char *file)
{
	FILE *fp;
	char *file_tmp;
	int i;
#if 0
	struct t_group *group;
#endif /* 0 */

	if (no_write && file_size (file) != -1L)
		return;

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(file);

	if ((fp = fopen (file_tmp, "w" FOPEN_OPTS)) == NULL) {
		error_message (_(txt_filesystem_full_backup), ATTRIBUTES_FILE);
		free (file_tmp);	/* free memory for tmp-filename */
		return;
	}

	if (!cmd_line && !batch_mode)
		wait_message (0, _(txt_writing_attributes_file));

	/*
	 * TODO: - sort in a usefull order
	 *       - move strings to lang.c
	 */
	fprintf (fp, _("# Group attributes file for the TIN newsreader\n#\n"));
	fprintf (fp, _("#  scope=STRING (ie. alt.*,!alt.bin*) [mandatory]\n"));
	fprintf (fp, _("#  maildir=STRING (ie. ~/Mail)\n"));
	fprintf (fp, _("#  savedir=STRING (ie. ~user/News)\n"));
	fprintf (fp, _("#  savefile=STRING (ie. =linux)\n"));
	fprintf (fp, _("#  sigfile=STRING (ie. $var/sig)\n"));
	fprintf (fp, _("#  organization=STRING (if beginning with '/' read from file)\n"));
	fprintf (fp, _("#  followup_to=STRING\n"));
	fprintf (fp, _("#  mailing_list=STRING (ie. majordomo@list.org)\n"));
	fprintf (fp, _("#  x_headers=STRING (ie. ~/.tin/extra-headers)\n"));
	fprintf (fp, _("#  x_body=STRING (ie. ~/.tin/extra-body-text)\n"));
	fprintf (fp, _("#  from=STRING (just append wanted From:-line, don't use quotes)\n"));
	fprintf (fp, _("#  news_quote_format=STRING\n"));
	fprintf (fp, _("#  quote_chars=STRING (%%s, %%S for initials)\n"));
#ifdef HAVE_ISPELL
	fprintf (fp, _("#  ispell=STRING\n"));
#endif /* HAVE_ISPELL */
	fprintf (fp, _("#  auto_select=ON/OFF\n"));
	fprintf (fp, _("#  auto_save=ON/OFF\n"));
	fprintf (fp, _("#  batch_save=ON/OFF\n"));
	fprintf (fp, _("#  delete_tmp_files=ON/OFF\n"));
	fprintf (fp, _("#  show_only_unread=ON/OFF\n"));
	fprintf (fp, _("#  thread_arts=NUM"));
	for (i = 0; i <= THREAD_MAX; i++) {
		if (!(i % 2))
			fprintf (fp, "\n#    ");
		fprintf (fp, "%d=%s, ", i, _(txt_thread[i]));
	}
	fprintf (fp, "\n");
	fprintf (fp, _("#  show_author=NUM\n"));
	fprintf (fp, "#    %d=%s, %d=%s, %d=%s, %d=%s\n",
		SHOW_FROM_NONE, _(txt_show_from[SHOW_FROM_NONE]),
		SHOW_FROM_ADDR, _(txt_show_from[SHOW_FROM_ADDR]),
		SHOW_FROM_NAME, _(txt_show_from[SHOW_FROM_NAME]),
		SHOW_FROM_BOTH, _(txt_show_from[SHOW_FROM_BOTH]));
	fprintf (fp, _("#  sort_art_type=NUM\n"));
	fprintf (fp, "#    %d=%s,\n",
		SORT_ARTICLES_BY_NOTHING, _(txt_sort_a_type[SORT_ARTICLES_BY_NOTHING]));
	fprintf (fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_SUBJ_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SUBJ_DESCEND]),
		SORT_ARTICLES_BY_SUBJ_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SUBJ_ASCEND]));
	fprintf (fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_FROM_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_FROM_DESCEND]),
		SORT_ARTICLES_BY_FROM_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_FROM_ASCEND]));
	fprintf (fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_DATE_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_DATE_DESCEND]),
		SORT_ARTICLES_BY_DATE_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_DATE_ASCEND]));
	fprintf (fp, "#    %d=%s, %d=%s,\n",
		SORT_ARTICLES_BY_SCORE_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SCORE_DESCEND]),
		SORT_ARTICLES_BY_SCORE_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_SCORE_ASCEND]));
	fprintf (fp, "#    %d=%s, %d=%s\n",
		SORT_ARTICLES_BY_LINES_DESCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_LINES_DESCEND]),
		SORT_ARTICLES_BY_LINES_ASCEND, _(txt_sort_a_type[SORT_ARTICLES_BY_LINES_ASCEND]));
	fprintf (fp, _("#  sort_threads_type=NUM\n"));
	fprintf (fp, "#    %d=%s, %d=%s, %d=%s\n",
		SORT_THREADS_BY_NOTHING, _(txt_sort_t_type[SORT_THREADS_BY_NOTHING]),
		SORT_THREADS_BY_SCORE_DESCEND, _(txt_sort_t_type[SORT_THREADS_BY_SCORE_DESCEND]),
		SORT_THREADS_BY_SCORE_ASCEND, _(txt_sort_t_type[SORT_THREADS_BY_SCORE_ASCEND]));
	fprintf (fp, _("#  post_proc_type=NUM\n"));
	fprintf (fp, "#    %d=%s, %d=%s, %d=%s\n",
		POST_PROC_NONE, _(txt_post_process_type[POST_PROC_NONE]),
		POST_PROC_SHAR, _(txt_post_process_type[POST_PROC_SHAR]),
		POST_PROC_UUDECODE, _(txt_post_process_type[POST_PROC_UUDECODE]));
	fprintf (fp, _("#  quick_kill_scope=STRING (ie. talk.*)\n"));
	fprintf (fp, _("#  quick_kill_expire=ON/OFF\n"));
	fprintf (fp, _("#  quick_kill_case=ON/OFF\n"));
	fprintf (fp, _("#  quick_kill_header=NUM\n"));
	fprintf (fp, _("#    0=subj (case sensitive) 1=subj (ignore case)\n"));
	fprintf (fp, _("#    2=from (case sensitive) 3=from (ignore case)\n"));
	fprintf (fp, _("#    4=msgid 5=lines\n"));
	fprintf (fp, _("#  quick_select_scope=STRING\n"));
	fprintf (fp, _("#  quick_select_expire=ON/OFF\n"));
	fprintf (fp, _("#  quick_select_case=ON/OFF\n"));
	fprintf (fp, _("#  quick_select_header=NUM\n"));
	fprintf (fp, _("#    0=subj (case sensitive) 1=subj (ignore case)\n"));
	fprintf (fp, _("#    2=from (case sensitive) 3=from (ignore case)\n"));
	fprintf (fp, _("#    4=msgid 5=lines\n"));
	fprintf (fp, _("#  x_comment_to=ON/OFF\n"));
	fprintf (fp, _("#  tex2iso_conv=ON/OFF\n"));
#ifdef CHARSET_CONVERSION
	fprintf (fp, _("#  mm_network_charset=supported_charset"));
	for (i = 0; i < NUM_MIME_CHARSETS; i++) {
		if (!(i % 5)) /* start new line */
			fprintf (fp, "\n#    ");
		fprintf (fp, "%s, ", txt_mime_charsets[i]);
	}
	fprintf (fp, "\n");
	fprintf (fp, _("#  undeclared_charset=STRING (default is US-ASCII)\n"));
#endif /* CHARSET_CONVERSION */
	fprintf (fp, _("#\n# Note that it is best to put general (global scoping)\n"));
	fprintf (fp, _("# entries first followed by group specific entries.\n#\n"));
	fprintf (fp, _("############################################################################\n\n"));

/*
 * some useful defaults
 */
	fprintf (fp, _("# include extra headers\n"));
	fprintf (fp, "scope=*\n");
	/*
	 * ${TIN_HOMEDIR-HOME} would be correct, but tin doesn't expand it,
	 * so we take ~ instead
	 */
	fprintf (fp, "x_headers=~/.tin/headers\n\n");

	fprintf (fp, _("# in *sources* set post process type to shar\n"));
	fprintf (fp, "scope=*sources*\n");
	fprintf (fp, "post_proc_type=1\n\n");

	fprintf (fp, _("# in *binaries* set post process type to uudecode, remove tmp files\n"));
	fprintf (fp, _("# and set Followup-To: poster\n"));
	fprintf (fp, "scope=*binaries*\n");
	fprintf (fp, "post_proc_type=2\n");
	fprintf (fp, "delete_tmp_files=ON\n");
	fprintf (fp, "followup_to=poster\n\n");

#if 0 /* FIXME */
	for_each_group(i) {
		group = &active[i];
		fprintf (fp, "scope=%s\n", group->name);
		fprintf (fp, "maildir=%s\n", group->attribute->maildir);
		fprintf (fp, "savedir=%s\n", group->attribute->savedir);
		fprintf (fp, "savefile=%s\n", group->attribute->savefile);
		fprintf (fp, "sigfile=%s\n", group->attribute->sigfile);
		fprintf (fp, "organization=%s\n", group->attribute->organization);
		fprintf (fp, "followup_to=%s\n", group->attribute->followup_to);
		fprintf (fp, "mailing_list=%s\n", group->attribute->mailing_list);
		fprintf (fp, "x_headers=%s\n", group->attribute->x_headers);
		fprintf (fp, "x_body=%s\n", group->attribute->x_body);
		fprintf (fp, "from=%s\n", group->attribute->from);
		fprintf (fp, "news_quote_format=%s\n", group->attribute->news_quote_format);
		fprintf (fp, "quote_chars=%s\n",
			quote_space_to_dash (group->attribute->quote_chars));
#	ifdef HAVE_ISPELL
		fprintf (fp, "ispell=%s\n", group->attribute->ispell);
#	endif /* HAVE_ISPELL */
		fprintf (fp, "show_only_unread=%s\n",
			print_boolean (group->attribute->show_only_unread));
		fprintf (fp, "thread_arts=%d\n", group->attribute->thread_arts);
		fprintf (fp, "auto_select=%s\n",
			print_boolean (group->attribute->auto_select));
		fprintf (fp, "auto_save=%s\n",
			print_boolean (group->attribute->auto_save));
		fprintf (fp, "batch_save=%s\n",
			print_boolean (group->attribute->batch_save));
		fprintf (fp, "delete_tmp_files=%s\n",
			print_boolean (group->attribute->delete_tmp_files));
		fprintf (fp, "sort_art_type=%d\n", group->attribute->sort_art_type);
		fprintf (fp, "sort_threads_type=%d\n", group->attribute->sort_threads_type);
		fprintf (fp, "show_author=%d\n", group->attribute->show_author);
		fprintf (fp, "post_proc_type=%d\n", group->attribute->post_proc_type);
		fprintf (fp, "quick_kill_scope=%s\n",
			group->attribute->quick_kill_scope);
		fprintf (fp, "quick_kill_case=%s\n",
			print_boolean (group->attribute->quick_kill_case));
		fprintf (fp, "quick_kill_expire=%s\n",
			print_boolean (group->attribute->quick_kill_expire));
		fprintf (fp, "quick_kill_header=%d\n", group->attribute->quick_kill_header);
		fprintf (fp, "quick_select_scope=%s\n",
			group->attribute->quick_select_scope);
		fprintf (fp, "quick_select_case=%s\n",
			print_boolean (group->attribute->quick_select_case));
		fprintf (fp, "quick_select_expire=%s\n",
			print_boolean (group->attribute->quick_select_expire));
		fprintf (fp, "quick_select_header=%d\n", group->attribute->quick_select_header);
		fprintf (fp, "x_comment_to=%s\n",
			print_boolean (group->attribute->x_comment_to));
		fprintf (fp, "tex2iso_conv=%s\n",
			print_boolean (group->attribute->tex2iso_conv));
#	ifdef CHARSET_CONVERSION
		fprintf (fp, "mm_network_charset=%s\n", txt_mime_charsets[group->attribute->mm_charset]);
		fprintf (fp, "undeclared_charset=%s\n", group->attribute->undeclared_charset);
#	endif /* CHARSET_CONVERSION */
	}
#endif /* 0 */

	if (ferror (fp) || fclose (fp))
		error_message (_(txt_filesystem_full), ATTRIBUTES_FILE);
	else {
		rename_file (file_tmp, file);
		chmod (file, (mode_t)(S_IRUSR|S_IWUSR));
	}
	free (file_tmp);	/* free memory for tmp-filename */
}


#if 0
void
debug_print_filter_attributes (
	void)
{
	register int i;
	struct t_group *group;

	my_printf("\nBEG ***\n");

	for_each_group(i) {
		group = &active[i];
		my_printf ("Grp=[%s] KILL   header=[%d] scope=[%s] case=[%s] expire=[%s]\n",
			group->name, group->attribute->quick_kill_header,
			(group->attribute->quick_kill_scope ?
				group->attribute->quick_kill_scope : ""),
			txt_onoff[group->attribute->quick_kill_case != FALSE ? 1 : 0],
			txt_onoff[group->attribute->quick_kill_expire != FALSE ? 1 : 0]);
		my_printf ("Grp=[%s] SELECT header=[%d] scope=[%s] case=[%s] expire=[%s]\n",
			group->name, group->attribute->quick_select_header,
			(group->attribute->quick_select_scope ?
				group->attribute->quick_select_scope: ""),
			txt_onoff[group->attribute->quick_select_case != FALSE ? 1 : 0],
			txt_onoff[group->attribute->quick_select_expire != FALSE ? 1 : 0]);
	}

	my_printf("END ***\n");
}


#	ifdef DEBUG
static void
dump_attributes (
	void)
{
	int i;
	struct t_group *group;

	fprintf(stderr, "DUMP attributes\n");

	for_each_group(i) {
		group = &active[i];
		if (!group->attribute)
			continue;
		fprintf (stderr, "scope=%s\n", group->name);
		fprintf (stderr, "Global=%d\n", group->attribute->global);
		fprintf (stderr, "maildir=%s\n", group->attribute->maildir);
		fprintf (stderr, "savedir=%s\n", group->attribute->savedir);
		fprintf (stderr, "savefile=%s\n", group->attribute->savefile);
		fprintf (stderr, "sigfile=%s\n", group->attribute->sigfile);
		fprintf (stderr, "organization=%s\n", group->attribute->organization);
		fprintf (stderr, "followup_to=%s\n", group->attribute->followup_to);
		fprintf (stderr, "mailing_list=%s\n", group->attribute->mailing_list);
		fprintf (stderr, "x_headers=%s\n", group->attribute->x_headers);
		fprintf (stderr, "x_body=%s\n", group->attribute->x_body);
		fprintf (stderr, "from=%s\n", group->attribute->from);
		fprintf (stderr, "news_quote_format=%s\n", group->attribute->news_quote_format);
		fprintf (stderr, "quote_chars=%s\n",
			quote_space_to_dash (group->attribute->quote_chars));
#		ifdef HAVE_ISPELL
		fprintf (stderr, "ispell=%s\n", group->attribute->ispell);
#		endif /* HAVE_ISPELL */
		fprintf (stderr, "show_only_unread=%s\n",
			print_boolean (group->attribute->show_only_unread));
		fprintf (stderr, "thread_arts=%d\n", group->attribute->thread_arts);
		fprintf (stderr, "auto_select=%s\n",
			print_boolean (group->attribute->auto_select));
		fprintf (stderr, "auto_save=%s\n",
			print_boolean (group->attribute->auto_save));
		fprintf (stderr, "batch_save=%s\n",
			print_boolean (group->attribute->batch_save));
		fprintf (stderr, "delete_tmp_files=%s\n",
			print_boolean (group->attribute->delete_tmp_files));
		fprintf (stderr, "sort_art_type=%d\n", group->attribute->sort_art_type);
		fprintf (stderr, "sort_threads_type=%d\n", group->attribute->sort_threads_type);
		fprintf (stderr, "show_author=%d\n", group->attribute->show_author);
		fprintf (stderr, "post_proc_type=%d\n", group->attribute->post_proc_type);
		fprintf (stderr, "quick_kill_scope=%s\n",
			group->attribute->quick_kill_scope);
		fprintf (stderr, "quick_kill_case=%s\n",
			print_boolean (group->attribute->quick_kill_case));
		fprintf (stderr, "quick_kill_expire=%s\n",
			print_boolean (group->attribute->quick_kill_expire));
		fprintf (stderr, "quick_kill_header=%d\n", group->attribute->quick_kill_header);
		fprintf (stderr, "quick_select_scope=%s\n",
			group->attribute->quick_select_scope);
		fprintf (stderr, "quick_select_case=%s\n",
			print_boolean (group->attribute->quick_select_case));
		fprintf (stderr, "quick_select_expire=%s\n",
			print_boolean (group->attribute->quick_select_expire));
		fprintf (stderr, "quick_select_header=%d\n", group->attribute->quick_select_header);
		fprintf (stderr, "x_comment_to=%s\n",
			print_boolean (group->attribute->x_comment_to));
		fprintf (stderr, "tex2iso_conv=%s\n",
			print_boolean (group->attribute->tex2iso_conv));
#		ifdef CHARSET_CONVERSION
		fprintf (stderr, "mm_network_charset=%s\n", txt_mime_charsets[group->attribute->mm_charset])
		fprintf (stderr, "undeclared_charset=%s\n", group->attribute->undeclared_charset);
#		endif /* CHARSET_CONVERSION */
	}
}
#	endif /* DEBUG */
#endif /* 0 */
