/*
 *  Project   : tin - a Usenet reader
 *  Module    : attrib.c
 *  Author    : I. Lea
 *  Created   : 1993-12-01
 *  Updated   : 1997-12-20
 *  Notes     : Group attribute routines
 *
 * Copyright (c) 1993-2001 Iain Lea <iain@bricbrac.de>
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

/*
 * Defines used in setting attributes switch
 */
#define ATTRIB_MAILDIR			0
#define ATTRIB_SAVEDIR			1
#define ATTRIB_SAVEFILE			2
#define ATTRIB_ORGANIZATION		3
#define ATTRIB_FROM			4
#define ATTRIB_SIGFILE			5
#define ATTRIB_FOLLOWUP_TO		6
#ifndef DISABLE_PRINTING
#	define ATTRIB_PRINTER		7
#endif /* !DISABLE_PRINTING */
#define ATTRIB_AUTO_SELECT		8
#define ATTRIB_AUTO_SAVE		9
#define ATTRIB_BATCH_SAVE		10
#define ATTRIB_DELETE_TMP_FILES		11
#define ATTRIB_SHOW_ONLY_UNREAD		12
#define ATTRIB_THREAD_ARTS		13
#define ATTRIB_SHOW_AUTHOR		14
#define ATTRIB_SORT_ART_TYPE		15
#define ATTRIB_POST_PROC_TYPE		16
#define ATTRIB_QUICK_KILL_HEADER	17
#define ATTRIB_QUICK_KILL_SCOPE		18
#define ATTRIB_QUICK_KILL_EXPIRE	19
#define ATTRIB_QUICK_KILL_CASE		20
#define ATTRIB_QUICK_SELECT_HEADER	21
#define ATTRIB_QUICK_SELECT_SCOPE	22
#define ATTRIB_QUICK_SELECT_EXPIRE	23
#define ATTRIB_QUICK_SELECT_CASE	24
#define ATTRIB_MAILING_LIST		25
#define ATTRIB_X_HEADERS		26
#define ATTRIB_X_BODY			27
#define ATTRIB_AUTO_SAVE_MSG		28
#define ATTRIB_X_COMMENT_TO		29
#define ATTRIB_NEWS_QUOTE		30
#define ATTRIB_QUOTE_CHARS		31
#ifdef HAVE_ISPELL
#	define ATTRIB_ISPELL		32
#endif /* HAVE_ISPELL */

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
#ifndef DISABLE_PRINTING
	attributes->printer = tinrc.printer;
#endif /* !DISABLE_PRINTING */
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
	attributes->show_author = tinrc.show_author;
	attributes->auto_save = tinrc.auto_save;
	attributes->auto_select = FALSE;
	attributes->batch_save = tinrc.batch_save;
	attributes->delete_tmp_files = FALSE;
	attributes->post_proc_type = tinrc.post_process;
	attributes->x_comment_to = FALSE;
}


/*
 *  Load global & local attributes into active[].attribute
 */

#define MATCH_BOOLEAN(pattern, type) \
	if (match_boolean (line, pattern, &flag)) { \
		num = (flag != FALSE); \
		set_attrib (type, scope, (char *)&num); \
		break; \
	}
#define MATCH_INTEGER(pattern, type, maxval) \
	if (match_integer (line, pattern, &num, maxval)) { \
		set_attrib (type, scope, (char *)&num); \
		break; \
	}
	/* FIXME: the code always modifies 'scope' -- does it ??? */
#define MATCH_STRING(pattern, type) \
	if (match_string (line, pattern, buf, sizeof (buf))) { \
		set_attrib (type, scope, buf); \
		break; \
	}


void
read_attributes_file (
	const char *file,
	t_bool global_file)
{
	FILE *fp;
	char buf[LEN];
	char line[LEN];
	char scope[LEN];
	int num;
	register int i;
	t_bool flag;

	/*
	 * Initialize global attributes even if there is no global file
	 * These setting are used as the default for all groups unless overridden
	 */
	if (global_file) {
		set_default_attributes (&glob_attributes);
		glob_attributes.global = TRUE;
	}

	if ((fp = fopen (file, "r")) != (FILE *) 0) {
		scope[0] = '\0';
		while (fgets (line, (int) sizeof (line), fp) != (char *) 0) {
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
				break;

			case 'n':
				MATCH_STRING ("news_quote_format=", ATTRIB_NEWS_QUOTE);
				break;

			case 'o':
				MATCH_STRING ("organization=", ATTRIB_ORGANIZATION);
				break;

			case 'p':
#ifndef DISABLE_PRINTING
				MATCH_STRING ("printer=", ATTRIB_PRINTER);
#endif /* !DISABLE_PRINTING */
				MATCH_INTEGER ("post_proc_type=",
					ATTRIB_POST_PROC_TYPE,
					POST_PROC_UUDECODE);
				break;

			case 'q':
				MATCH_INTEGER ("quick_kill_header=",
					ATTRIB_QUICK_KILL_HEADER,
					FILTER_LINES);
				MATCH_STRING ("quick_kill_scope=", ATTRIB_QUICK_KILL_SCOPE);
				MATCH_BOOLEAN ("quick_kill_case=", ATTRIB_QUICK_KILL_CASE);
				MATCH_BOOLEAN ("quick_kill_expire=", ATTRIB_QUICK_KILL_EXPIRE);
				MATCH_INTEGER ("quick_select_header=",
					ATTRIB_QUICK_SELECT_HEADER,
					FILTER_LINES);
				MATCH_STRING ("quick_select_scope=", ATTRIB_QUICK_SELECT_SCOPE);
				MATCH_BOOLEAN ("quick_select_case=", ATTRIB_QUICK_SELECT_CASE);
				MATCH_BOOLEAN ("quick_select_expire=", ATTRIB_QUICK_SELECT_EXPIRE);
				if (match_string (line, "quote_chars=", buf, sizeof (buf))) {
					quote_dash_to_space (buf);
					set_attrib (ATTRIB_QUOTE_CHARS, scope, buf);
					break;
				}
				break;

			case 's':
				MATCH_STRING ("savedir=", ATTRIB_SAVEDIR);
				MATCH_STRING ("savefile=", ATTRIB_SAVEFILE);
				if (match_string (line, "scope=", scope, sizeof (scope)))
					break;
				MATCH_STRING ("sigfile=", ATTRIB_SIGFILE);
				MATCH_BOOLEAN ("show_only_unread=", ATTRIB_SHOW_ONLY_UNREAD);
				MATCH_INTEGER ("sort_art_type=",
					ATTRIB_SORT_ART_TYPE,
					SORT_BY_SCORE_ASCEND);
				MATCH_INTEGER ("show_author=",
					ATTRIB_SHOW_AUTHOR,
					SHOW_FROM_BOTH);
				break;

			case 't':
				MATCH_INTEGER ("thread_arts=", ATTRIB_THREAD_ARTS, THREAD_MAX);
				break;

			case 'x':
				MATCH_STRING ("x_headers=", ATTRIB_X_HEADERS);
				MATCH_STRING ("x_body=", ATTRIB_X_BODY);
				MATCH_BOOLEAN ("x_comment_to=", ATTRIB_X_COMMENT_TO);
				break;

			default:
				break;
			}
/* TODO report syntax errors */
		}
		fclose (fp);
	}

	/*
	 * Now setup the rest of the groups to use the default attributes
	 */
	if (!global_file) {
		for (i = 0; i < num_active; i++) {
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

	if (scope == (char *) 0 || *scope == '\0')	/* No active scope set yet */
		return;
#if 0
	fprintf(stderr, "set_attrib #%d %s %s(%d)\n", type, scope, data, (int)*data);
#endif /* 0 */

	/*
	 * Does scope refer to 1 or more than 1 group
	 * TODO If scope=*  change glob_attributes and just those groups with structs already
	 */
	if (!strchr (scope, '*')) {
		if ((group = group_find (scope)) != (struct t_group *) 0)
			do_set_attrib (group, type, data);
	} else {
		int i;

		for (i = 0; i < num_active; i++) {
			group = &active[i];
			if (match_group_list (group->name, scope))
				do_set_attrib (group, type, data);
		}
	}
}


static void
do_set_attrib (
	struct t_group *group,
	int type,
	const char *data)
{
	/*
	 * Setup default attributes for this group if none already set
	 */
	if (group->attribute == (struct t_attribute *) 0) {
		group->attribute = (struct t_attribute *) my_malloc (sizeof (struct t_attribute));
		set_default_attributes (group->attribute);
	}

	/*
	 * Now set the required attribute
	 */
	switch (type) {
		case ATTRIB_MAILDIR:
			group->attribute->maildir = my_strdup (data);
			break;
		case ATTRIB_SAVEDIR:
			group->attribute->savedir = my_strdup (data);
			break;
		case ATTRIB_SAVEFILE:
			group->attribute->savefile = my_strdup (data);
			break;
		case ATTRIB_ORGANIZATION:
			group->attribute->organization = my_strdup (data);
			break;
		case ATTRIB_FROM:
			group->attribute->from = my_strdup (data);
			break;
		case ATTRIB_SIGFILE:
			group->attribute->sigfile = my_strdup (data);
			break;
		case ATTRIB_FOLLOWUP_TO:
			group->attribute->followup_to = my_strdup (data);
			break;
#ifndef DISABLE_PRINTING
		case ATTRIB_PRINTER:
			group->attribute->printer = my_strdup (data);
			break;
#endif /* !DISABLE_PRINTING */
		case ATTRIB_AUTO_SELECT:
			group->attribute->auto_select = *data;
			break;
		case ATTRIB_AUTO_SAVE:
			group->attribute->auto_save = *data;
			break;
		case ATTRIB_BATCH_SAVE:
			group->attribute->batch_save = *data;
			break;
		case ATTRIB_DELETE_TMP_FILES:
			group->attribute->delete_tmp_files = *data;
			break;
		case ATTRIB_SHOW_ONLY_UNREAD:
			group->attribute->show_only_unread = *data;
			break;
		case ATTRIB_THREAD_ARTS:
			group->attribute->thread_arts = *data;
			break;
		case ATTRIB_SHOW_AUTHOR:
			group->attribute->show_author = *data;
			break;
		case ATTRIB_SORT_ART_TYPE:
			group->attribute->sort_art_type = *data;
			break;
		case ATTRIB_POST_PROC_TYPE:
			group->attribute->post_proc_type = *data;
			break;
		case ATTRIB_QUICK_KILL_HEADER:
			group->attribute->quick_kill_header = *data;
			break;
		case ATTRIB_QUICK_KILL_SCOPE:
			group->attribute->quick_kill_scope = my_strdup (data);
			break;
		case ATTRIB_QUICK_KILL_EXPIRE:
			group->attribute->quick_kill_expire = *data;
			break;
		case ATTRIB_QUICK_KILL_CASE:
			group->attribute->quick_kill_case = *data;
			break;
		case ATTRIB_QUICK_SELECT_HEADER:
			group->attribute->quick_select_header = *data;
			break;
		case ATTRIB_QUICK_SELECT_SCOPE:
			group->attribute->quick_select_scope = my_strdup (data);
			break;
		case ATTRIB_QUICK_SELECT_EXPIRE:
			group->attribute->quick_select_expire = *data;
			break;
		case ATTRIB_QUICK_SELECT_CASE:
			group->attribute->quick_select_case = *data;
			break;
		case ATTRIB_MAILING_LIST:
			group->attribute->mailing_list = my_strdup (data);
			break;
		case ATTRIB_X_HEADERS:
			group->attribute->x_headers = my_strdup (data);
			break;
		case ATTRIB_X_BODY:
			group->attribute->x_body = my_strdup (data);
			break;
		case ATTRIB_X_COMMENT_TO:
			group->attribute->x_comment_to = *data;
			break;
		case ATTRIB_NEWS_QUOTE:
			group->attribute->news_quote_format = my_strdup (data);
			break;
		case ATTRIB_QUOTE_CHARS:
			group->attribute->quote_chars = my_strdup (data);
			break;
#ifdef HAVE_ISPELL
		case ATTRIB_ISPELL:
			group->attribute->ispell = my_strdup (data);
			break;
#endif /* HAVE_ISPELL */
		default:
			break;
	}

}


/*
 *  Save the group attributes from active[].attribute to ~/.tin/attributes
 */
void
write_attributes_file (
	const char *file)
{
	FILE *fp;
	char *file_tmp;
#if 0
	register int i;
	struct t_group *group;
#endif /* 0 */

	if (no_write && file_size (file) != -1L)
		return;

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(file);

	if ((fp = fopen (file_tmp, "w" FOPEN_OPTS)) == (FILE *) 0) {
		error_message (_(txt_filesystem_full_backup), ATTRIBUTES_FILE);
		free (file_tmp);	/* free memory for tmp-filename */
		return;
	}

	if (!cmd_line && !batch_mode)
		wait_message (0, _(txt_writing_attributes_file));

	/* FIXME - move strings to lang.c */
	fprintf (fp, _("# Group attributes file for the TIN newsreader\n#\n"));
	fprintf (fp, _("#  scope=STRING (ie. alt.sources, alt.*,!alt.bin* etc..) [mandatory]\n"));
	fprintf (fp, _("#  maildir=STRING (ie. ~/Mail)\n"));
	fprintf (fp, _("#  savedir=STRING (ie. ~user/News)\n"));
	fprintf (fp, _("#  savefile=STRING (ie. =linux)\n"));
	fprintf (fp, _("#  sigfile=STRING (ie. $var/sig)\n"));
	fprintf (fp, _("#  organization=STRING (if beginning with '/' read from file)\n"));
	fprintf (fp, _("#  followup_to=STRING\n"));
#ifndef DISABLE_PRINTING
	fprintf (fp, _("#  printer=STRING\n"));
#endif /* !DISABLE_PRINTING */
	fprintf (fp, _("#  mailing_list=STRING (ie. majordomo@list.org)\n"));
	fprintf (fp, _("#  x_headers=STRING (ie. ~/.tin/extra-headers)\n"));
	fprintf (fp, _("#  x_body=STRING (ie. ~/.tin/extra-body-text)\n"));
	fprintf (fp, _("#  from=STRING (just append wanted From:-line, don't use quotes)\n"));
	fprintf (fp, _("#  news_quote_format=STRING\n"));
	fprintf (fp, _("#  quote_chars=STRING (%%s, %%S for initials)\n"));
#ifdef HAVE_ISPELL
	fprintf (fp, _("#  ispell = STRING\n"));
#endif /* HAVE_ISPELL */
	fprintf (fp, _("#  auto_select=ON/OFF\n"));
	fprintf (fp, _("#  auto_save=ON/OFF\n"));
	fprintf (fp, _("#  batch_save=ON/OFF\n"));
	fprintf (fp, _("#  delete_tmp_files=ON/OFF\n"));
	fprintf (fp, _("#  show_only_unread=ON/OFF\n"));
	fprintf (fp, _("#  thread_arts=NUM\n"));
	fprintf (fp, _("#    0=none, 1=subj, 2=refs, 3=both\n"));
	fprintf (fp, _("#  show_author=NUM\n"));
	fprintf (fp, _("#    0=none, 1=name, 2=addr, 3=both\n"));
	fprintf (fp, _("#  sort_art_type=NUM\n"));
	fprintf (fp, _("#    0=none, 1=subj descend, 2=subj ascend,\n"));
	fprintf (fp, _("#    3=from descend, 4=from ascend,\n"));
	fprintf (fp, _("#    5=date descend, 6=date ascend\n"));
	fprintf (fp, _("#    7=score descend, 8=score ascend\n"));
	fprintf (fp, _("#  post_proc_type=NUM\n"));
	fprintf (fp, _("#    0=none, 1=unshar, 2=uudecode\n"));
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
	for (i = 0; i < num_active; i++) {
		group = &active[i];
		fprintf (fp, "scope=%s\n", group->name);
		fprintf (fp, "maildir=%s\n", group->attribute->maildir);
		fprintf (fp, "savedir=%s\n", group->attribute->savedir);
		fprintf (fp, "savefile=%s\n", group->attribute->savefile);
		fprintf (fp, "sigfile=%s\n", group->attribute->sigfile);
		fprintf (fp, "organization=%s\n", group->attribute->organization);
		fprintf (fp, "followup_to=%s\n", group->attribute->followup_to);
#	ifndef DISABLE_PRINTING
		fprintf (fp, "printer=%s\n", group->attribute->printer);
#	endif /* !DISABLE_PRINTING */
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
		fprintf (fp, "quick_select_header=%d\n\n", group->attribute->quick_select_header);
		fprintf (fp, "x_comment_to=%s\n",
			print_boolean (group->attribute->x_comment_to));
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

	for (i = 0; i < num_active; i++) {
		group = &active[i];
		my_printf ("Grp=[%s] KILL   header=[%d] scope=[%s] case=[%s] expire=[%s]\n",
			group->name, group->attribute->quick_kill_header,
			(group->attribute->quick_kill_scope ?
				group->attribute->quick_kill_scope : ""),
			(group->attribute->quick_kill_case ? "ON" : "OFF"),
			(group->attribute->quick_kill_expire ? "ON" : "OFF"));
		my_printf ("Grp=[%s] SELECT header=[%d] scope=[%s] case=[%s] expire=[%s]\n",
			group->name, group->attribute->quick_select_header,
			(group->attribute->quick_select_scope ?
				group->attribute->quick_select_scope: ""),
			(group->attribute->quick_select_case ? "ON" : "OFF"),
			(group->attribute->quick_select_expire ? "ON" : "OFF"));
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

	for (i = 0; i < num_active; i++) {
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
#		ifndef DISABLE_PRINTING
		fprintf (stderr, "printer=%s\n", group->attribute->printer);
#		endif /* !DISABLE_PRINTING */
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
		fprintf (stderr, "x_comment_to=%s\n\n",
			print_boolean (group->attribute->x_comment_to));
	}
}
#	endif /* DEBUG */
#endif /* 0 */
