/*
 *  Project   : tin - a Usenet reader
 *  Module    : attrib.c
 *  Author    : I. Lea
 *  Created   : 1993-12-01
 *  Updated   : 2008-12-15
 *  Notes     : Group attribute routines
 *
 * Copyright (c) 1993-2009 Iain Lea <iain@bricbrac.de>
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
	ATTRIB_ADD_POSTED_TO_FILTER,
	ATTRIB_ADVERTISING,
	ATTRIB_ALTERNATIVE_HANDLING,
	ATTRIB_ASK_FOR_METAMAIL,
	ATTRIB_AUTO_BCC,
	ATTRIB_AUTO_CC,
	ATTRIB_AUTO_LIST_THREAD,
	ATTRIB_AUTO_SELECT,
	ATTRIB_AUTO_SAVE,
	ATTRIB_BATCH_SAVE,
	ATTRIB_DATE_FORMAT,
	ATTRIB_EDITOR_FORMAT,
	ATTRIB_DELETE_TMP_FILES,
	ATTRIB_GROUP_CATCHUP_ON_EXIT,
	ATTRIB_MARK_IGNORE_TAGS,
	ATTRIB_MARK_SAVED_READ,
	ATTRIB_NEWS_HEADERS_TO_DISPLAY,
	ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY,
	ATTRIB_POS_FIRST_UNREAD,
	ATTRIB_POST_PROCESS_VIEW,
#ifndef DISABLE_PRINTING
	ATTRIB_PRINT_HEADER,
#endif /* !DISABLE_PRINTING */
	ATTRIB_PROCESS_ONLY_UNREAD,
	ATTRIB_PROMPT_FOLLOWUPTO,
	ATTRIB_SHOW_ONLY_UNREAD_ARTS,
	ATTRIB_SIGDASHES,
	ATTRIB_SIGNATURE_REPOST,
	ATTRIB_START_EDITOR_OFFSET,
	ATTRIB_THREAD_ARTICLES,
	ATTRIB_THREAD_CATCHUP_ON_EXIT,
	ATTRIB_THREAD_PERC,
	ATTRIB_SHOW_AUTHOR,
	ATTRIB_SHOW_INFO,
	ATTRIB_SHOW_SIGNATURES,
	ATTRIB_TRIM_ARTICLE_BODY,
	ATTRIB_VERBATIM_HANDLING,
	ATTRIB_WRAP_ON_NEXT_UNREAD,
	ATTRIB_SORT_ARTICLE_TYPE,
	ATTRIB_POST_PROCESS_TYPE,
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
static void set_attrib(int type, const char *scope, const char *data, t_bool global_file);
static void set_default_attributes(struct t_attribute *attributes, struct t_attribute *scope, t_bool global);
#ifdef DEBUG
	static void dump_attributes(void);
	static void dump_scopes(void);
#	if 0 /* unused */
		static void debug_print_filter_attributes(void);
#	endif /* 0 */
#endif /* DEBUG */

/*
 * Per group attributes. This fills out a basic template of defaults
 * before the attributes in the current scope are applied.
 */
static void
set_default_attributes(
	struct t_attribute *attributes,
	struct t_attribute *scope,
	t_bool global)
{
	attributes->scope = NULL;
	attributes->global = (global ? TRUE : FALSE);	/* global/group specific */
	attributes->temp = FALSE;
	attributes->maildir = (scope ? scope->maildir : (global ? tinrc.maildir : NULL));
	attributes->savedir = (scope ? scope->savedir : (global ? tinrc.savedir : NULL));
	attributes->savefile = NULL;
	attributes->sigfile = (scope ? scope->sigfile : (global ? tinrc.sigfile : NULL));
	attributes->date_format = (scope ? scope->date_format : (global ? tinrc.date_format : NULL));
	attributes->editor_format = (scope ? scope->editor_format : (global ? tinrc.editor_format : NULL));
	attributes->organization = (scope ? scope->organization : (global ? (*default_organization ? default_organization : NULL) : NULL));
	attributes->followup_to = NULL;
	attributes->mailing_list = NULL;
	attributes->x_headers = NULL;
	attributes->x_body = NULL;
	attributes->from = (scope ? scope->from :(global ? tinrc.mail_address : NULL));
	attributes->news_quote_format = (scope ? scope->news_quote_format : (global ? tinrc.news_quote_format : NULL));
	attributes->quote_chars = (scope ? scope->quote_chars : (global ? tinrc.quote_chars : NULL));
	attributes->mime_types_to_save = (scope ? scope->mime_types_to_save : (global ? my_strdup("*/*") : NULL));
#ifdef HAVE_ISPELL
	attributes->ispell = NULL;
#endif /* HAVE_ISPELL */
	attributes->quick_kill_scope = (scope ? scope->quick_kill_scope : (global ? (tinrc.default_filter_kill_global ? my_strdup("*") : NULL) : NULL));
	attributes->quick_kill_header = tinrc.default_filter_kill_header;
	attributes->quick_kill_case = tinrc.default_filter_kill_case;
	attributes->quick_kill_expire = tinrc.default_filter_kill_expire;
	attributes->quick_select_scope = (scope ? scope->quick_select_scope : (global ? (tinrc.default_filter_select_global ? my_strdup("*") : NULL) : NULL));
	attributes->quick_select_header = tinrc.default_filter_select_header;
	attributes->quick_select_case = tinrc.default_filter_select_case;
	attributes->quick_select_expire = tinrc.default_filter_select_expire;
	attributes->show_only_unread_arts = tinrc.show_only_unread_arts;
	attributes->thread_articles = tinrc.thread_articles;
	attributes->thread_catchup_on_exit = tinrc.thread_catchup_on_exit;
	attributes->thread_perc = tinrc.thread_perc;
	attributes->sort_article_type = tinrc.sort_article_type;
	attributes->sort_threads_type = tinrc.sort_threads_type;
	attributes->show_info = tinrc.show_info;
	attributes->show_author = tinrc.show_author;
	attributes->show_signatures = tinrc.show_signatures;
	attributes->trim_article_body = tinrc.trim_article_body;
	attributes->verbatim_handling = tinrc.verbatim_handling;
	attributes->wrap_on_next_unread = tinrc.wrap_on_next_unread;
	attributes->add_posted_to_filter = tinrc.add_posted_to_filter;
	attributes->advertising = tinrc.advertising;
	attributes->alternative_handling = tinrc.alternative_handling;
	attributes->ask_for_metamail = tinrc.ask_for_metamail;
	attributes->auto_bcc = tinrc.auto_bcc;
	attributes->auto_cc = tinrc.auto_cc;
	attributes->auto_list_thread = tinrc.auto_list_thread;
	attributes->auto_save = tinrc.auto_save;
	attributes->auto_select = FALSE;
	attributes->batch_save = tinrc.batch_save;
	attributes->delete_tmp_files = FALSE;
	attributes->group_catchup_on_exit = tinrc.group_catchup_on_exit;
	attributes->mark_ignore_tags = tinrc.mark_ignore_tags;
	attributes->mark_saved_read = tinrc.mark_saved_read;
	attributes->news_headers_to_display = (global ? tinrc.news_headers_to_display : NULL);
	attributes->headers_to_display = (scope ? (scope->headers_to_display ? scope->headers_to_display : NULL) : NULL);
	attributes->news_headers_to_not_display = (global ? tinrc.news_headers_to_not_display : NULL);
	attributes->headers_to_not_display = (scope ? (scope->headers_to_not_display ? scope->headers_to_not_display : NULL) : NULL);
	attributes->pos_first_unread = tinrc.pos_first_unread;
	attributes->post_process_view = tinrc.post_process_view;
	attributes->post_process_type = tinrc.post_process_type;
#ifndef DISABLE_PRINTING
	attributes->print_header = tinrc.print_header;
#endif /* !DISABLE_PRINTING */
	attributes->process_only_unread = tinrc.process_only_unread;
	attributes->prompt_followupto = tinrc.prompt_followupto;
	attributes->sigdashes = tinrc.sigdashes;
	attributes->signature_repost = tinrc.signature_repost;
	attributes->start_editor_offset = tinrc.start_editor_offset;
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
		set_attrib(type, scope, (const char *) &num, global_file); \
		found = TRUE; \
		break; \
	}
#define MATCH_INTEGER(pattern, type, maxval) \
	if (match_integer(line, pattern, &num, maxval)) { \
		set_attrib(type, scope, (const char *) &num, global_file); \
		found = TRUE; \
		break; \
	}
#define MATCH_STRING(pattern, type) \
	if (match_string(line, pattern, buf, sizeof(buf) - strlen(pattern))) { \
		set_attrib(type, scope, buf, global_file); \
		found = TRUE; \
		break; \
	}
#ifdef CHARSET_CONVERSION
#	define MATCH_LIST(pattern, type, table, tablelen) \
		if (match_list(line, pattern, table, tablelen, &num)) { \
			set_attrib(type, scope, (const char *) &num, global_file); \
			found = TRUE; \
			break; \
		}
#endif /* CHARSET_CONVERSION */
#if !defined(CHARSET_CONVERSION) || !defined(HAVE_ISPELL) || defined(DISABLE_PRINTING)
#	define SKIP_ITEM(pattern) \
		if (!strncmp(line, pattern, strlen(pattern))) { \
			found = TRUE; \
			break; \
		}
#endif /* !CHARSET_CONVERSION || !HAVE_ISPELL || DISABLE_PRINTING */

/*
 * read global/local attributes file
 */
void
read_attributes_file(
	t_bool global_file)
{
	FILE *fp;
	char *file;
	char buf[LEN];
	char line[LEN];
	char scope[LEN];
	int num;
	enum rc_state upgrade = RC_CHECK;
	t_bool flag, found = FALSE;

	if (!batch_mode)
		wait_message(0, _(txt_reading_attributes_file), (global_file ? _(txt_global) : ""));
	/*
	 * Initialize global attributes even if there is no global file
	 * These setting are used as the default for all groups unless overridden
	 */
	if (global_file) {
		if ((num_scope >= max_scope) || (num_scope < 0) || (scopes == NULL))
			expand_scope();
		set_default_attributes(&scopes[num_scope], NULL, TRUE);
		build_news_headers_array(&scopes[num_scope], TRUE);
		build_news_headers_array(&scopes[num_scope], FALSE);
		num_scope++;
		file = global_attributes_file;
	} else
		file = local_attributes_file;

	if ((fp = fopen(file, "r")) != NULL) {
		scope[0] = '\0';
		while (fgets(line, (int) sizeof(line), fp) != NULL) {
			if (line[0] == '\n')
				continue;
			if (line[0] == '#') {
				if (!global_file && upgrade == RC_CHECK && match_string(line, "# Group attributes file V", NULL, 0)) {
					upgrade = check_upgrade(line, "# Group attributes file V", ATTRIBUTES_VERSION);
					if (upgrade != RC_IGNORE)
						upgrade_prompt_quit(upgrade, file); /* TODO: do something (more) useful here */
				}
				continue;
			}

			switch (tolower((unsigned char) line[0])) {
				case 'a':
					MATCH_BOOLEAN("add_posted_to_filter=", ATTRIB_ADD_POSTED_TO_FILTER);
					MATCH_BOOLEAN("advertising=", ATTRIB_ADVERTISING);
					MATCH_BOOLEAN("alternative_handling=", ATTRIB_ALTERNATIVE_HANDLING);
					MATCH_BOOLEAN("ask_for_metamail=", ATTRIB_ASK_FOR_METAMAIL);
					MATCH_BOOLEAN("auto_bcc=", ATTRIB_AUTO_BCC);
					MATCH_BOOLEAN("auto_cc=", ATTRIB_AUTO_CC);
					MATCH_BOOLEAN("auto_list_thread=", ATTRIB_AUTO_LIST_THREAD);
					MATCH_BOOLEAN("auto_save=", ATTRIB_AUTO_SAVE);
					MATCH_BOOLEAN("auto_select=", ATTRIB_AUTO_SELECT);
					break;

				case 'b':
					MATCH_BOOLEAN("batch_save=", ATTRIB_BATCH_SAVE);
					break;

				case 'd':
					MATCH_STRING("date_format=", ATTRIB_DATE_FORMAT);
					MATCH_BOOLEAN("delete_tmp_files=", ATTRIB_DELETE_TMP_FILES);
					break;

				case 'e':
					MATCH_STRING("editor_format=", ATTRIB_EDITOR_FORMAT);
					break;

				case 'f':
					MATCH_STRING("fcc=", ATTRIB_FCC);
					MATCH_STRING("followup_to=", ATTRIB_FOLLOWUP_TO);
					MATCH_STRING("from=", ATTRIB_FROM);
					break;

				case 'g':
					MATCH_BOOLEAN("group_catchup_on_exit=", ATTRIB_GROUP_CATCHUP_ON_EXIT);
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
					MATCH_BOOLEAN("mark_ignore_tags=", ATTRIB_MARK_IGNORE_TAGS);
					MATCH_BOOLEAN("mark_saved_read=", ATTRIB_MARK_SAVED_READ);
					MATCH_BOOLEAN("mime_forward=", ATTRIB_MIME_FORWARD);
					MATCH_STRING("mime_types_to_save=", ATTRIB_MIME_TYPES_TO_SAVE);
#ifdef CHARSET_CONVERSION
					MATCH_LIST("mm_network_charset=", ATTRIB_MM_NETWORK_CHARSET, txt_mime_charsets, NUM_MIME_CHARSETS);
#else
					SKIP_ITEM("mm_network_charset=");
#endif /* CHARSET_CONVERSION */
					break;

				case 'n':
					MATCH_STRING("news_headers_to_display=", ATTRIB_NEWS_HEADERS_TO_DISPLAY);
					MATCH_STRING("news_headers_to_not_display=", ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY);
					MATCH_STRING("news_quote_format=", ATTRIB_NEWS_QUOTE);
					break;

				case 'o':
					MATCH_STRING("organization=", ATTRIB_ORGANIZATION);
					break;

				case 'p':
					MATCH_BOOLEAN("pos_first_unread=", ATTRIB_POS_FIRST_UNREAD);
					MATCH_BOOLEAN("post_process_view=", ATTRIB_POST_PROCESS_VIEW);
					MATCH_INTEGER("post_process_type=", ATTRIB_POST_PROCESS_TYPE, POST_PROC_YES);
#ifndef DISABLE_PRINTING
					MATCH_BOOLEAN("print_header=", ATTRIB_PRINT_HEADER);
#else
					SKIP_ITEM("print_header=");
#endif /* !DISABLE_PRINTING */
					MATCH_BOOLEAN("process_only_unread=", ATTRIB_PROCESS_ONLY_UNREAD);
					MATCH_BOOLEAN("prompt_followupto=", ATTRIB_PROMPT_FOLLOWUPTO);
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
						set_attrib(ATTRIB_QUOTE_CHARS, scope, buf, global_file);
						found = TRUE;
						break;
					}
					break;

				case 's':
					MATCH_STRING("savedir=", ATTRIB_SAVEDIR);
					MATCH_STRING("savefile=", ATTRIB_SAVEFILE);
					if (match_string(line, "scope=", scope, sizeof(scope))) {
						if (!global_file) {
							/* Add a new entry to the list of editable attributes */
							if (num_scope >= max_scope) /*|| (num_scope < 1) || (scopes == NULL))*/
								expand_scope();
							set_default_attributes(&scopes[num_scope], NULL, FALSE);
							scopes[num_scope].scope = my_strdup(scope);
							num_scope++;
						}
						found = TRUE;
						break;
					}
					MATCH_INTEGER("show_author=", ATTRIB_SHOW_AUTHOR, SHOW_FROM_BOTH);
					MATCH_INTEGER("show_info=", ATTRIB_SHOW_INFO, SHOW_INFO_BOTH);
					MATCH_BOOLEAN("show_only_unread_arts=", ATTRIB_SHOW_ONLY_UNREAD_ARTS);
					MATCH_BOOLEAN("show_signatures=", ATTRIB_SHOW_SIGNATURES);
					MATCH_BOOLEAN("sigdashes=", ATTRIB_SIGDASHES);
					MATCH_BOOLEAN("signature_repost=", ATTRIB_SIGNATURE_REPOST);
					MATCH_BOOLEAN("start_editor_offset=", ATTRIB_START_EDITOR_OFFSET);
					MATCH_STRING("sigfile=", ATTRIB_SIGFILE);
					MATCH_INTEGER("sort_article_type=", ATTRIB_SORT_ARTICLE_TYPE, SORT_ARTICLES_BY_LINES_ASCEND);
					MATCH_INTEGER("sort_threads_type=", ATTRIB_SORT_THREADS_TYPE, SORT_THREADS_BY_LAST_POSTING_DATE_ASCEND);
					break;

				case 't':
					MATCH_BOOLEAN("tex2iso_conv=", ATTRIB_TEX2ISO_CONV);
					MATCH_INTEGER("thread_articles=", ATTRIB_THREAD_ARTICLES, THREAD_MAX);
					MATCH_BOOLEAN("thread_catchup_on_exit=", ATTRIB_THREAD_CATCHUP_ON_EXIT);
					MATCH_INTEGER("thread_perc=", ATTRIB_THREAD_PERC, 100);
					MATCH_INTEGER("trim_article_body=", ATTRIB_TRIM_ARTICLE_BODY, 7);
					break;

				case 'u':
#ifdef CHARSET_CONVERSION
					MATCH_STRING("undeclared_charset=", ATTRIB_UNDECLARED_CHARSET);
#else
					SKIP_ITEM("undeclared_charset=");
#endif /* CHARSET_CONVERSION */
					break;

				case 'v':
					MATCH_BOOLEAN("verbatim_handling=", ATTRIB_VERBATIM_HANDLING);
					break;

				case 'w':
					MATCH_BOOLEAN("wrap_on_next_unread=", ATTRIB_WRAP_ON_NEXT_UNREAD);
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
			else {
				if (upgrade == RC_UPGRADE && !global_file) {
					switch (tolower((unsigned char) line[0])) {
						case 'p':
							MATCH_INTEGER("post_proc_type=", ATTRIB_POST_PROCESS_TYPE, POST_PROC_YES);
							break;

						case 's':
							MATCH_BOOLEAN("show_only_unread=", ATTRIB_SHOW_ONLY_UNREAD_ARTS);
							MATCH_INTEGER("sort_art_type=", ATTRIB_SORT_ARTICLE_TYPE, SORT_ARTICLES_BY_LINES_ASCEND);
							break;

						case 't':
							MATCH_INTEGER("thread_arts=", ATTRIB_THREAD_ARTICLES, THREAD_MAX);
							break;

						default:
							break;
					}
					if (found)
						found = FALSE;
					else
						error_message(1, _(txt_bad_attrib), line);
				} else	/* TODO: surpress error messages on non intial reads? */
					error_message(1, _(txt_bad_attrib), line);
			}
		}
		fclose(fp);

		/*
		 * TODO: do something useful for the other cases
		 */
		if (upgrade == RC_UPGRADE && !global_file)
			write_attributes_file(file);
	} else if (!global_file) {
		/* no local attributes file, add some useful defaults and write file */

		add_scope("*", FALSE);
		set_attrib(ATTRIB_X_HEADERS, "*", "~/.tin/headers", global_file);

		add_scope("*sources*", FALSE);
		num = POST_PROC_SHAR;
		set_attrib(ATTRIB_POST_PROCESS_TYPE, "*sources*", (char *) &num, global_file);

		add_scope("*binaries*", FALSE);
		num = POST_PROC_YES;
		set_attrib(ATTRIB_POST_PROCESS_TYPE, "*binaries*", (char *) &num, global_file);
		num = FALSE;
		set_attrib(ATTRIB_TEX2ISO_CONV, "*binaries*", (char *) &num, global_file);
		num = TRUE;
		set_attrib(ATTRIB_DELETE_TMP_FILES, "*binaries*", (char *) &num, global_file);
		set_attrib(ATTRIB_FOLLOWUP_TO, "*binaries*", "poster", global_file);

		write_attributes_file(file);
	}
#ifdef DEBUG
	if (!global_file)
		dump_scopes();
#endif /* DEBUG */
}


#define SET_STRING(string) \
	FreeIfNeeded(attributes->string); \
	attributes->string = my_strdup(data); \
	break
#define SET_INTEGER(integer) \
	attributes->integer = *((const int *) data); \
	break

static void
set_attrib(
	int type,
	const char *scope,
	const char *data,
	t_bool global_file)
{
	struct t_attribute *attributes;

	if (scope == NULL || *scope == '\0') {	/* No active scope set yet */
		/* TODO: include full line in error-message */
		error_message(2, _("attribute with no scope: %s"), data); /* TODO: -> lang.c */
		return;
	}

	if (!global_file && (num_scope > 1)) {
		attributes = &scopes[num_scope - 1];
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

			case ATTRIB_ADD_POSTED_TO_FILTER:
				SET_INTEGER(add_posted_to_filter);

			case ATTRIB_ADVERTISING:
				SET_INTEGER(advertising);

			case ATTRIB_ALTERNATIVE_HANDLING:
				SET_INTEGER(alternative_handling);

			case ATTRIB_ASK_FOR_METAMAIL:
				SET_INTEGER(ask_for_metamail);

			case ATTRIB_AUTO_BCC:
				SET_INTEGER(auto_bcc);

			case ATTRIB_AUTO_CC:
				SET_INTEGER(auto_cc);

			case ATTRIB_AUTO_LIST_THREAD:
				SET_INTEGER(auto_list_thread);

			case ATTRIB_AUTO_SELECT:
				SET_INTEGER(auto_select);

			case ATTRIB_AUTO_SAVE:
				SET_INTEGER(auto_save);

			case ATTRIB_BATCH_SAVE:
				SET_INTEGER(batch_save);

			case ATTRIB_DATE_FORMAT:
				SET_STRING(date_format);

			case ATTRIB_DELETE_TMP_FILES:
				SET_INTEGER(delete_tmp_files);

			case ATTRIB_EDITOR_FORMAT:
				SET_STRING(editor_format);

			case ATTRIB_GROUP_CATCHUP_ON_EXIT:
				SET_INTEGER(group_catchup_on_exit);

			case ATTRIB_MARK_IGNORE_TAGS:
				SET_INTEGER(mark_ignore_tags);

			case ATTRIB_MARK_SAVED_READ:
				SET_INTEGER(mark_saved_read);

			case ATTRIB_NEWS_HEADERS_TO_DISPLAY:
				FreeIfNeeded(attributes->news_headers_to_display);
				attributes->news_headers_to_display = my_strdup(data);
				build_news_headers_array(attributes, TRUE);
				break;

			case ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY:
				FreeIfNeeded(attributes->news_headers_to_not_display);
				attributes->news_headers_to_not_display = my_strdup(data);
				build_news_headers_array(attributes, FALSE);
				break;

			case ATTRIB_POS_FIRST_UNREAD:
				SET_INTEGER(pos_first_unread);

			case ATTRIB_POST_PROCESS_VIEW:
				SET_INTEGER(post_process_view);

#ifndef DISABLE_PRINTING
			case ATTRIB_PRINT_HEADER:
				SET_INTEGER(print_header);
#endif /* !DISABLE_PRINTING */

			case ATTRIB_PROCESS_ONLY_UNREAD:
				SET_INTEGER(process_only_unread);

			case ATTRIB_PROMPT_FOLLOWUPTO:
				SET_INTEGER(prompt_followupto);

			case ATTRIB_SHOW_ONLY_UNREAD_ARTS:
				SET_INTEGER(show_only_unread_arts);

			case ATTRIB_SIGDASHES:
				SET_INTEGER(sigdashes);

			case ATTRIB_SIGNATURE_REPOST:
				SET_INTEGER(signature_repost);

			case ATTRIB_START_EDITOR_OFFSET:
				SET_INTEGER(start_editor_offset);

			case ATTRIB_THREAD_ARTICLES:
				SET_INTEGER(thread_articles);

			case ATTRIB_THREAD_CATCHUP_ON_EXIT:
				SET_INTEGER(thread_catchup_on_exit);

			case ATTRIB_THREAD_PERC:
				SET_INTEGER(thread_perc);

			case ATTRIB_SHOW_AUTHOR:
				SET_INTEGER(show_author);

			case ATTRIB_SHOW_INFO:
				SET_INTEGER(show_info);

			case ATTRIB_SHOW_SIGNATURES:
				SET_INTEGER(show_signatures);

			case ATTRIB_TRIM_ARTICLE_BODY:
				SET_INTEGER(trim_article_body);

			case ATTRIB_VERBATIM_HANDLING:
				SET_INTEGER(verbatim_handling);

			case ATTRIB_WRAP_ON_NEXT_UNREAD:
				SET_INTEGER(wrap_on_next_unread);

			case ATTRIB_SORT_ARTICLE_TYPE:
				SET_INTEGER(sort_article_type);

			case ATTRIB_SORT_THREADS_TYPE:
				SET_INTEGER(sort_threads_type);

			case ATTRIB_POST_PROCESS_TYPE:
				SET_INTEGER(post_process_type);

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

			case ATTRIB_FCC:
				SET_STRING(fcc);

			case ATTRIB_NEWS_QUOTE:
				SET_STRING(news_quote_format);

			case ATTRIB_QUOTE_CHARS:
				SET_STRING(quote_chars);

			case ATTRIB_MIME_TYPES_TO_SAVE:
				SET_STRING(mime_types_to_save);

			case ATTRIB_MIME_FORWARD:
				SET_INTEGER(mime_forward);

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
}


/*
 * Insert a new [temporary] scope entry into scopes[]
 */
int
add_scope(
	const char *scope,
	t_bool tmpscope)
{
	int i = 0;

	if (!scope || !*scope)
		return i;

	if (tmpscope) {
		for (i = 1; i < num_scope; i++) {
			if ((strcasecmp(scope, scopes[i].scope) == 0) && scopes[i].temp)
				return i;
		}
	}
	if (num_scope >= max_scope)
		expand_scope();
	set_default_attributes(&scopes[num_scope], NULL, FALSE);
	scopes[num_scope].scope = my_strdup(scope);
	if (tmpscope)
		scopes[num_scope].temp = TRUE;
	return num_scope++;
}


#define SET_STRING_ATTRIB(attr) do { \
		if (curr_scope->attr) \
			group->attribute->attr = curr_scope->attr; \
	} while (0)

#define SET_INT_ATTRIB(attr) do { \
		group->attribute->attr = curr_scope->attr; \
	} while (0)

/*
 * Set the attributes of all groups
 */
void
assign_attributes_to_groups(
	void)
{
	struct t_group *group;
	struct t_attribute *global_scope, *curr_scope;
	t_bool found;
	int i, j;

	if (!cmd_line && !batch_mode)
		wait_message(0, _("Processing attributes... ")); /* TODO: -> lang.c */

	global_scope = &scopes[0];
	for_each_group(i) {
		group = &active[i];
		found = FALSE;
		for (j = 1; j < num_scope; j++) {
			curr_scope = &scopes[j];
			if (match_group_list(group->name, curr_scope->scope)) {
				if (group->attribute == NULL)
					group->attribute = my_malloc(sizeof(struct t_attribute));
				if (!found)
					set_default_attributes(group->attribute, global_scope, FALSE);
				found = TRUE;
				SET_STRING_ATTRIB(maildir);
				SET_STRING_ATTRIB(savedir);
				SET_STRING_ATTRIB(savefile);
				SET_STRING_ATTRIB(sigfile);
				SET_STRING_ATTRIB(date_format);
				SET_STRING_ATTRIB(editor_format);
				SET_STRING_ATTRIB(organization);
				SET_STRING_ATTRIB(fcc);
				SET_STRING_ATTRIB(followup_to);
				SET_STRING_ATTRIB(mailing_list);
				SET_STRING_ATTRIB(x_headers);
				SET_STRING_ATTRIB(x_body);
				SET_STRING_ATTRIB(from);
				SET_STRING_ATTRIB(news_quote_format);
				SET_STRING_ATTRIB(quote_chars);
				SET_STRING_ATTRIB(mime_types_to_save);
#ifdef HAVE_ISPELL
				SET_STRING_ATTRIB(ispell);
#endif /* HAVE_ISPELL */
#ifdef CHARSET_CONVERSION
				SET_INT_ATTRIB(mm_network_charset);
				SET_STRING_ATTRIB(undeclared_charset);
#endif /* CHARSET_CONVERSION */
				SET_STRING_ATTRIB(quick_kill_scope);
				SET_INT_ATTRIB(quick_kill_header);
				SET_INT_ATTRIB(quick_kill_case);
				SET_INT_ATTRIB(quick_kill_expire);
				SET_STRING_ATTRIB(quick_select_scope);
				SET_INT_ATTRIB(quick_select_header);
				SET_INT_ATTRIB(quick_select_case);
				SET_INT_ATTRIB(quick_select_expire);
				SET_INT_ATTRIB(show_only_unread_arts);
				SET_INT_ATTRIB(thread_articles);
				SET_INT_ATTRIB(thread_catchup_on_exit);
				SET_INT_ATTRIB(thread_perc);
				SET_INT_ATTRIB(sort_article_type);
				SET_INT_ATTRIB(sort_threads_type);
				SET_INT_ATTRIB(show_info);
				SET_INT_ATTRIB(show_author);
				SET_INT_ATTRIB(show_signatures);
				SET_INT_ATTRIB(trim_article_body);
				SET_INT_ATTRIB(verbatim_handling);
				SET_INT_ATTRIB(wrap_on_next_unread);
				SET_INT_ATTRIB(add_posted_to_filter);
				SET_INT_ATTRIB(advertising);
				SET_INT_ATTRIB(alternative_handling);
				SET_INT_ATTRIB(ask_for_metamail);
				SET_INT_ATTRIB(auto_bcc);
				SET_INT_ATTRIB(auto_cc);
				SET_INT_ATTRIB(auto_list_thread);
				SET_INT_ATTRIB(auto_save);
				SET_INT_ATTRIB(auto_select);
				SET_INT_ATTRIB(batch_save);
				SET_INT_ATTRIB(delete_tmp_files);
				SET_INT_ATTRIB(group_catchup_on_exit);
				SET_INT_ATTRIB(mark_ignore_tags);
				SET_INT_ATTRIB(mark_saved_read);
				SET_STRING_ATTRIB(headers_to_display);
				SET_STRING_ATTRIB(headers_to_not_display);
				SET_INT_ATTRIB(pos_first_unread);
				SET_INT_ATTRIB(post_process_view);
				SET_INT_ATTRIB(post_process_type);
#ifndef DISABLE_PRINTING
				SET_INT_ATTRIB(print_header);
#endif /* !DISABLE_PRINTING */
				SET_INT_ATTRIB(process_only_unread);
				SET_INT_ATTRIB(prompt_followupto);
				SET_INT_ATTRIB(sigdashes);
				SET_INT_ATTRIB(signature_repost);
				SET_INT_ATTRIB(start_editor_offset);
				SET_INT_ATTRIB(x_comment_to);
				SET_INT_ATTRIB(tex2iso_conv);
				SET_INT_ATTRIB(mime_forward);
			}
		}
		if (!found)
			group->attribute = global_scope;
	}
#ifdef DEBUG
	dump_attributes();
#	if 0
	debug_print_filter_attributes();
#	endif /* 0 */
#endif /* DEBUG */
}


/*
 * (re)build scope->headers_to_[not_]display array
 */
void
build_news_headers_array(
	struct t_attribute *scope,
	t_bool header_to_display)
{
	if (header_to_display) {
		if (scope->headers_to_display == NULL)
			scope->headers_to_display = my_malloc(sizeof(struct t_newsheader));
		else {
			if (scope->headers_to_display->header)
				FreeIfNeeded(*scope->headers_to_display->header);
			FreeIfNeeded(scope->headers_to_display->header);
		}
		scope->headers_to_display->header = ulBuildArgv(scope->news_headers_to_display, &scope->headers_to_display->num);
	} else {
		if (scope->headers_to_not_display == NULL)
			scope->headers_to_not_display = my_malloc(sizeof(struct t_newsheader));
		else {
			if (scope->headers_to_not_display->header)
				FreeIfNeeded(*scope->headers_to_not_display->header);
			FreeIfNeeded(scope->headers_to_not_display->header);
		}
		scope->headers_to_not_display->header = ulBuildArgv(scope->news_headers_to_not_display, &scope->headers_to_not_display->num);
	}
}


/*
 * Save the scope attributes from scopes[] to ~/.tin/attributes
 */
void
write_attributes_file(
	const char *file)
{
	FILE *fp;
	char *new_file;
	int i;

	if (file_size(file) != -1L && (no_write || num_scope <= 1))
		return;

	new_file = get_tmpfilename(file);

	if ((fp = fopen(new_file, "w")) == NULL) {
		error_message(2, _(txt_filesystem_full_backup), ATTRIBUTES_FILE);
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
	fprintf(fp, _("#  scope=STRING (eg. alt.*,!alt.bin*) [mandatory]\n"));
	fprintf(fp, _("#  add_posted_to_filter=ON/OFF\n"));
	fprintf(fp, _("#  advertising=ON/OFF\n"));
	fprintf(fp, _("#  alternative_handling=ON/OFF\n"));
	fprintf(fp, _("#  ask_for_metamail=ON/OFF\n"));
	fprintf(fp, _("#  auto_bcc=ON/OFF\n"));
	fprintf(fp, _("#  auto_cc=ON/OFF\n"));
	fprintf(fp, _("#  auto_list_thread=ON/OFF\n"));
	fprintf(fp, _("#  auto_save=ON/OFF\n"));
	fprintf(fp, _("#  auto_select=ON/OFF\n"));
	fprintf(fp, _("#  batch_save=ON/OFF\n"));
	fprintf(fp, _("#  date_format=STRING (eg. %%a, %%d %%b %%Y %%H:%%M:%%S)\n"));
	fprintf(fp, _("#  delete_tmp_files=ON/OFF\n"));
	fprintf(fp, _("#  editor_format=STRING (eg. %%E +%%N %%F)\n"));
	fprintf(fp, _("#  fcc=STRING (eg. =mailbox)\n"));
	fprintf(fp, _("#  followup_to=STRING\n"));
	fprintf(fp, _("#  from=STRING (just append wanted From:-line, don't use quotes)\n"));
	fprintf(fp, _("#  group_catchup_on_exit=ON/OFF\n"));
#ifdef HAVE_ISPELL
	fprintf(fp, _("#  ispell=STRING\n"));
#endif /* HAVE_ISPELL */
	fprintf(fp, _("#  maildir=STRING (eg. ~/Mail)\n"));
	fprintf(fp, _("#  mailing_list=STRING (eg. majordomo@example.org)\n"));
	fprintf(fp, _("#  mime_types_to_save=STRING (eg. image/*,!image/bmp)\n"));
	fprintf(fp, _("#  mark_ignore_tags=ON/OFF\n"));
	fprintf(fp, _("#  mark_saved_read=ON/OFF\n"));
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
	fprintf(fp, _("#  news_headers_to_display=STRING\n"));
	fprintf(fp, _("#  news_headers_to_not_display=STRING\n"));
	fprintf(fp, _("#  news_quote_format=STRING\n"));
	fprintf(fp, _("#  organization=STRING (if beginning with '/' read from file)\n"));
	fprintf(fp, _("#  pos_first_unread=ON/OFF\n"));
	fprintf(fp, _("#  post_process_type=NUM\n"));
	fprintf(fp, "#    %d=%s, %d=%s, %d=%s\n",
		POST_PROC_NO, _(txt_post_process_types[POST_PROC_NO]),
		POST_PROC_SHAR, _(txt_post_process_types[POST_PROC_SHAR]),
		POST_PROC_YES, _(txt_post_process_types[POST_PROC_YES]));
	fprintf(fp, _("#  post_process_view=ON/OFF\n"));
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
	fprintf(fp, _("#  quote_chars=STRING (%%s, %%S for initials)\n"));
#ifndef DISABLE_PRINTING
	fprintf(fp, _("#  print_header=ON/OFF\n"));
#endif /* !DISABLE_PRINTING */
	fprintf(fp, _("#  process_only_unread=ON/OFF\n"));
	fprintf(fp, _("#  prompt_followupto=ON/OFF\n"));
	fprintf(fp, _("#  savedir=STRING (eg. ~user/News)\n"));
	fprintf(fp, _("#  savefile=STRING (eg. =linux)\n"));
	fprintf(fp, _("#  sigfile=STRING (eg. $var/sig)\n"));
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
	fprintf(fp, _("#  show_signatures=ON/OFF\n"));
	fprintf(fp, _("#  show_only_unread_arts=ON/OFF\n"));
	fprintf(fp, _("#  sigdashes=ON/OFF\n"));
	fprintf(fp, _("#  signature_repost=ON/OFF\n"));
	fprintf(fp, _("#  sort_article_type=NUM\n"));
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
	fprintf(fp, "#    %d=%s, %d=%s\n",
		SORT_THREADS_BY_LAST_POSTING_DATE_DESCEND, _(txt_sort_t_type[SORT_THREADS_BY_LAST_POSTING_DATE_DESCEND]),
		SORT_THREADS_BY_LAST_POSTING_DATE_ASCEND, _(txt_sort_t_type[SORT_THREADS_BY_LAST_POSTING_DATE_ASCEND]));
	fprintf(fp, _("#  start_editor_offset=ON/OFF\n"));
	fprintf(fp, _("#  tex2iso_conv=ON/OFF\n"));
	fprintf(fp, _("#  thread_catchup_on_exit=ON/OFF\n"));
	fprintf(fp, _("#  thread_articles=NUM"));
	for (i = 0; i <= THREAD_MAX; i++) {
		if (!(i % 2))
			fprintf(fp, "\n#    ");
		fprintf(fp, "%d=%s, ", i, _(txt_threading[i]));
	}
	fprintf(fp, "\n");
	fprintf(fp, _("#  thread_perc=NUM\n"));
	fprintf(fp, _("#  trim_article_body=NUM\n"));
	fprintf(fp, _("#    0 = Don't trim article body\n"));
	fprintf(fp, _("#    1 = Skip leading blank lines\n"));
	fprintf(fp, _("#    2 = Skip trailing blank lines\n"));
	fprintf(fp, _("#    3 = Skip leading and trailing blank lines\n"));
	fprintf(fp, _("#    4 = Compact multiple blank lines between textblocks\n"));
	fprintf(fp, _("#    5 = Compact multiple blank lines between textblocks and skip\n#        leading blank lines\n"));
	fprintf(fp, _("#    6 = Compact multiple blank lines between textblocks and skip\n#        trailing blank lines\n"));
	fprintf(fp, _("#    7 = Compact multiple blank lines between textblocks and skip\n#        leading and trailing blank lines\n"));
	fprintf(fp, _("#  verbatim_handling=ON/OFF\n"));
	fprintf(fp, _("#  wrap_on_next_unread=ON/OFF\n"));
	fprintf(fp, _("#  x_body=STRING (eg. ~/.tin/extra-body-text)\n"));
	fprintf(fp, _("#  x_comment_to=ON/OFF\n"));
	fprintf(fp, _("#  x_headers=STRING (eg. ~/.tin/extra-headers)\n"));
	fprintf(fp, _("#\n# Note that it is best to put general (global scoping)\n"));
	fprintf(fp, _("# entries first followed by group specific entries.\n#\n"));
	fprintf(fp, _("############################################################################\n"));

	if ((num_scope > 0) && (scopes != NULL)) {
		struct t_attribute *attr;
		struct t_attribute *default_attr;

		default_attr = &scopes[0];

		for (i = 1; i < num_scope; i++) {
			attr = &scopes[i];
			if (!attr->temp) {
				fprintf(fp, "\nscope=%s\n", attr->scope);
				if (attr->add_posted_to_filter != default_attr->add_posted_to_filter)
					fprintf(fp, "add_posted_to_filter=%s\n", print_boolean(attr->add_posted_to_filter));
				if (attr->advertising != default_attr->advertising)
					fprintf(fp, "advertising=%s\n", print_boolean(attr->advertising));
				if (attr->alternative_handling != default_attr->alternative_handling)
					fprintf(fp, "alternative_handling=%s\n", print_boolean(attr->alternative_handling));
				if (attr->ask_for_metamail != default_attr->ask_for_metamail)
					fprintf(fp, "ask_for_metamail=%s\n", print_boolean(attr->ask_for_metamail));
				if (attr->auto_bcc != default_attr->auto_bcc)
					fprintf(fp, "auto_bcc=%s\n", print_boolean(attr->auto_bcc));
				if (attr->auto_cc != default_attr->auto_cc)
					fprintf(fp, "auto_cc=%s\n", print_boolean(attr->auto_cc));
				if (attr->auto_list_thread != default_attr->auto_list_thread)
					fprintf(fp, "auto_list_thread=%s\n", print_boolean(attr->auto_list_thread));
				if (attr->auto_select != default_attr->auto_select)
					fprintf(fp, "auto_select=%s\n", print_boolean(attr->auto_select));
				if (attr->auto_save != default_attr->auto_save)
					fprintf(fp, "auto_save=%s\n", print_boolean(attr->auto_save));
				if (attr->batch_save != default_attr->batch_save)
					fprintf(fp, "batch_save=%s\n", print_boolean(attr->batch_save));
				if (attr->date_format)
					fprintf(fp, "date_format=%s\n", attr->date_format);
				if (attr->delete_tmp_files != default_attr->delete_tmp_files)
					fprintf(fp, "delete_tmp_files=%s\n", print_boolean(attr->delete_tmp_files));
				if (attr->editor_format)
					fprintf(fp, "editor_format=%s\n", attr->editor_format);
				if (attr->fcc)
					fprintf(fp, "fcc=%s\n", attr->fcc);
				if (attr->followup_to)
					fprintf(fp, "followup_to=%s\n", attr->followup_to);
				if (attr->from)
					fprintf(fp, "from=%s\n", attr->from);
				if (attr->group_catchup_on_exit != default_attr->group_catchup_on_exit)
					fprintf(fp, "group_catchup_on_exit=%s\n", print_boolean(attr->group_catchup_on_exit));
#ifdef HAVE_ISPELL
				if (attr->ispell)
					fprintf(fp, "ispell=%s\n", attr->ispell);
#endif /* HAVE_ISPELL */
				if (attr->maildir)
					fprintf(fp, "maildir=%s\n", attr->maildir);
				if (attr->mailing_list)
					fprintf(fp, "mailing_list=%s\n", attr->mailing_list);
				if (attr->mark_ignore_tags != default_attr->mark_ignore_tags)
					fprintf(fp, "mark_ignore_tags=%s\n", print_boolean(attr->mark_ignore_tags));
				if (attr->mark_saved_read != default_attr->mark_saved_read)
					fprintf(fp, "mark_saved_read=%s\n", print_boolean(attr->mark_saved_read));
				if (attr->mime_forward != default_attr->mime_forward)
					fprintf(fp, "mime_forward=%s\n", print_boolean(attr->mime_forward));
				if (attr->mime_types_to_save)
					fprintf(fp, "mime_types_to_save=%s\n", attr->mime_types_to_save);
#ifdef CHARSET_CONVERSION
				if (attr->mm_network_charset && (attr->mm_network_charset != default_attr->mm_network_charset))
					fprintf(fp, "mm_network_charset=%s\n", txt_mime_charsets[attr->mm_network_charset]);
				if (attr->undeclared_charset)
					fprintf(fp, "undeclared_charset=%s\n", attr->undeclared_charset);
#endif /* CHARSET_CONVERSION */
				if (attr->news_headers_to_display)
					fprintf(fp, "news_headers_to_display=%s\n", attr->news_headers_to_display);
				if (attr->news_headers_to_not_display)
					fprintf(fp, "news_headers_to_not_display=%s\n", attr->news_headers_to_not_display);
				if (attr->news_quote_format)
					fprintf(fp, "news_quote_format=%s\n", attr->news_quote_format);
				if (attr->organization)
					fprintf(fp, "organization=%s\n", attr->organization);
				if (attr->pos_first_unread != default_attr->pos_first_unread)
					fprintf(fp, "pos_first_unread=%s\n", print_boolean(attr->pos_first_unread));
				if (attr->post_process_view != default_attr->post_process_view)
					fprintf(fp, "post_process_view=%s\n", print_boolean(attr->post_process_view));
				if (attr->post_process_type != default_attr->post_process_type)
					fprintf(fp, "post_process_type=%d\n", attr->post_process_type);
#ifndef DISABLE_PRINTING
				if (attr->print_header != default_attr->print_header)
					fprintf(fp, "print_header=%s\n", print_boolean(attr->print_header));
#endif /* !DISABLE_PRINTING */
				if (attr->process_only_unread != default_attr->process_only_unread)
					fprintf(fp, "process_only_unread=%s\n", print_boolean(attr->process_only_unread));
				if (attr->prompt_followupto != default_attr->prompt_followupto)
					fprintf(fp, "prompt_followupto=%s\n", print_boolean(attr->prompt_followupto));
				if (attr->quick_kill_scope)
					fprintf(fp, "quick_kill_scope=%s\n", attr->quick_kill_scope);
				if (attr->quick_kill_case != default_attr->quick_kill_case)
					fprintf(fp, "quick_kill_case=%s\n", print_boolean(attr->quick_kill_case));
				if (attr->quick_kill_expire != default_attr->quick_kill_expire)
					fprintf(fp, "quick_kill_expire=%s\n", print_boolean(attr->quick_kill_expire));
				if (attr->quick_kill_header != default_attr->quick_kill_header)
					fprintf(fp, "quick_kill_header=%d\n", attr->quick_kill_header);
				if (attr->quick_select_scope)
					fprintf(fp, "quick_select_scope=%s\n", attr->quick_select_scope);
				if (attr->quick_select_case != default_attr->quick_select_case)
					fprintf(fp, "quick_select_case=%s\n", print_boolean(attr->quick_select_case));
				if (attr->quick_select_expire != default_attr->quick_select_expire)
					fprintf(fp, "quick_select_expire=%s\n", print_boolean(attr->quick_select_expire));
				if (attr->quick_select_header != default_attr->quick_select_header)
					fprintf(fp, "quick_select_header=%d\n", attr->quick_select_header);
				if (attr->quote_chars)
					fprintf(fp, "quote_chars=%s\n", quote_space_to_dash(attr->quote_chars));
				if (attr->savedir)
					fprintf(fp, "savedir=%s\n", attr->savedir);
				if (attr->savefile)
					fprintf(fp, "savefile=%s\n", attr->savefile);
				if (attr->show_author != default_attr->show_author)
					fprintf(fp, "show_author=%d\n", attr->show_author);
				if (attr->show_info != default_attr->show_info)
					fprintf(fp, "show_info=%d\n", attr->show_info);
				if (attr->show_only_unread_arts != default_attr->show_only_unread_arts)
					fprintf(fp, "show_only_unread_arts=%s\n", print_boolean(attr->show_only_unread_arts));
				if (attr->show_signatures != default_attr->show_signatures)
					fprintf(fp, "show_signatures=%s\n", print_boolean(attr->show_signatures));
				if (attr->sigdashes != default_attr->sigdashes)
					fprintf(fp, "sigdashes=%s\n", print_boolean(attr->sigdashes));
				if (attr->sigfile)
					fprintf(fp, "sigfile=%s\n", attr->sigfile);
				if (attr->signature_repost != default_attr->signature_repost)
					fprintf(fp, "signature_repost=%s\n", print_boolean(attr->signature_repost));
				if (attr->sort_article_type != default_attr->sort_article_type)
					fprintf(fp, "sort_article_type=%d\n", attr->sort_article_type);
				if (attr->sort_threads_type != default_attr->sort_threads_type)
					fprintf(fp, "sort_threads_type=%d\n", attr->sort_threads_type);
				if (attr->start_editor_offset != default_attr->start_editor_offset)
					fprintf(fp, "start_editor_offset=%s\n", print_boolean(attr->start_editor_offset));
				if (attr->tex2iso_conv != default_attr->tex2iso_conv)
					fprintf(fp, "tex2iso_conv=%s\n", print_boolean(attr->tex2iso_conv));
				if (attr->thread_articles != default_attr->thread_articles)
					fprintf(fp, "thread_articles=%d\n", attr->thread_articles);
				if (attr->thread_catchup_on_exit != default_attr->thread_catchup_on_exit)
					fprintf(fp, "thread_catchup_on_exit=%s\n", print_boolean(attr->thread_catchup_on_exit));
				if (attr->thread_perc != default_attr->thread_perc)
					fprintf(fp, "thread_perc=%d\n", attr->thread_perc);
				if (attr->trim_article_body != default_attr->trim_article_body)
					fprintf(fp, "trim_article_body=%d\n", attr->trim_article_body);
				if (attr->verbatim_handling != default_attr->verbatim_handling)
					fprintf(fp, "verbatim_handling=%s\n", print_boolean(attr->verbatim_handling));
				if (attr->wrap_on_next_unread != default_attr->wrap_on_next_unread)
					fprintf(fp, "wrap_on_next_unread=%s\n", print_boolean(attr->wrap_on_next_unread));
				if (attr->x_headers)
					fprintf(fp, "x_headers=%s\n", attr->x_headers);
				if (attr->x_body)
					fprintf(fp, "x_body=%s\n", attr->x_body);
				if (attr->x_comment_to != default_attr->x_comment_to)
					fprintf(fp, "x_comment_to=%s\n", print_boolean(attr->x_comment_to));
			}
		}
	}

	/* rename_file() preserves mode, so this is safe */
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	if (ferror(fp) || fclose(fp)) {
		error_message(2, _(txt_filesystem_full), ATTRIBUTES_FILE);
		unlink(new_file);
	} else
		rename_file(new_file, file);

	free(new_file);
	return;
}


#ifdef DEBUG
#	if 0
static void
debug_print_filter_attributes(
	void)
{
	if (debug & DEBUG_ATTRIB) {
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
}
#	endif /*0 */


static void
dump_attributes(
	void)
{
	if (debug & DEBUG_ATTRIB) {
		int i, j;
		struct t_group *group;

		for_each_group(i) {
			group = &active[i];
			if (!group->attribute)
				continue;
			debug_print_file("ATTRIBUTES", "group=%s", group->name);
			debug_print_file("ATTRIBUTES", "\tGlobal=%d", group->attribute->global);
			debug_print_file("ATTRIBUTES", "\tmaildir=%s", group->attribute->maildir);
			debug_print_file("ATTRIBUTES", "\tsavedir=%s", group->attribute->savedir);
			debug_print_file("ATTRIBUTES", "\tsavefile=%s", group->attribute->savefile);
			debug_print_file("ATTRIBUTES", "\tsigfile=%s", group->attribute->sigfile);
			debug_print_file("ATTRIBUTES", "\torganization=%s", group->attribute->organization);
			debug_print_file("ATTRIBUTES", "\tfollowup_to=%s", group->attribute->followup_to);
			debug_print_file("ATTRIBUTES", "\tmailing_list=%s", group->attribute->mailing_list);
			debug_print_file("ATTRIBUTES", "\tx_headers=%s", group->attribute->x_headers);
			debug_print_file("ATTRIBUTES", "\tx_body=%s", group->attribute->x_body);
			debug_print_file("ATTRIBUTES", "\tfrom=%s", group->attribute->from);
			debug_print_file("ATTRIBUTES", "\tnews_quote_format=%s", group->attribute->news_quote_format);
			debug_print_file("ATTRIBUTES", "\tquote_chars=%s", quote_space_to_dash(group->attribute->quote_chars));
			debug_print_file("ATTRIBUTES", "\tmime_types_to_save=%s", group->attribute->mime_types_to_save);
#	ifdef HAVE_ISPELL
			debug_print_file("ATTRIBUTES", "\tispell=%s", group->attribute->ispell);
#	endif /* HAVE_ISPELL */
			debug_print_file("ATTRIBUTES", "\tshow_only_unread_arts=%s", print_boolean(group->attribute->show_only_unread_arts));
			debug_print_file("ATTRIBUTES", "\tthread_articles=%d", group->attribute->thread_articles);
			debug_print_file("ATTRIBUTES", "\tthread_perc=%d", group->attribute->thread_perc);
			debug_print_file("ATTRIBUTES", "\tadd_posted_to_filter=%s", print_boolean(group->attribute->add_posted_to_filter));
			debug_print_file("ATTRIBUTES", "\tadvertising=%s", print_boolean(group->attribute->advertising));
			debug_print_file("ATTRIBUTES", "\talternative_handling=%s", print_boolean(group->attribute->alternative_handling));
			debug_print_file("ATTRIBUTES", "\task_for_metamail=%s", print_boolean(group->attribute->ask_for_metamail));
			debug_print_file("ATTRIBUTES", "\tauto_bcc=%s", print_boolean(group->attribute->auto_bcc));
			debug_print_file("ATTRIBUTES", "\tauto_cc=%s", print_boolean(group->attribute->auto_cc));
			debug_print_file("ATTRIBUTES", "\tauto_list_thread=%s", print_boolean(group->attribute->auto_list_thread));
			debug_print_file("ATTRIBUTES", "\tauto_select=%s", print_boolean(group->attribute->auto_select));
			debug_print_file("ATTRIBUTES", "\tauto_save=%s", print_boolean(group->attribute->auto_save));
			debug_print_file("ATTRIBUTES", "\tbatch_save=%s", print_boolean(group->attribute->batch_save));
			debug_print_file("ATTRIBUTES", "\tdate_format=%s", group->attribute->date_format);
			debug_print_file("ATTRIBUTES", "\tdelete_tmp_files=%s", print_boolean(group->attribute->delete_tmp_files));
			debug_print_file("ATTRIBUTES", "\teditor_format=%s", group->attribute->editor_format);
			debug_print_file("ATTRIBUTES", "\tgroup_catchup_on_exit=%s", print_boolean(group->attribute->group_catchup_on_exit));
			debug_print_file("ATTRIBUTES", "\tmark_ignore_tags=%s", print_boolean(group->attribute->mark_ignore_tags));
			debug_print_file("ATTRIBUTES", "\tmark_saved_read=%s", print_boolean(group->attribute->mark_saved_read));
			debug_print_file("ATTRIBUTES", "\tnews_headers_to_display=%s", group->attribute->news_headers_to_display);
			if (group->attribute->headers_to_display) {
				debug_print_file("ATTRIBUTES", "\theaders_to_display->num=%d", group->attribute->headers_to_display->num);
				for (j = 0; j < group->attribute->headers_to_display->num; j++)
					debug_print_file("ATTRIBUTES", "\theaders_to_display->header[%d]=%s", j, group->attribute->headers_to_display->header[j]);
			} else
				debug_print_file("ATTRIBUTES", "\theaders_to_display=(null)");
			debug_print_file("ATTRIBUTES", "\tnews_headers_to_not_display=%s", group->attribute->news_headers_to_not_display);
			if (group->attribute->headers_to_not_display) {
				debug_print_file("ATTRIBUTES", "\theaders_to_not_display->num=%d", group->attribute->headers_to_not_display->num);
				for (j = 0; j < group->attribute->headers_to_not_display->num; j++)
					debug_print_file("ATTRIBUTES", "\theaders_to_not_display->header[%d]=%s", j, group->attribute->headers_to_not_display->header[j]);
			} else
				debug_print_file("ATTRIBUTES", "\theaders_to_not_display=(null)");
			debug_print_file("ATTRIBUTES", "\tpos_first_unread=%s", print_boolean(group->attribute->pos_first_unread));
			debug_print_file("ATTRIBUTES", "\tpost_process_view=%s", print_boolean(group->attribute->post_process_view));
#	ifndef DISABLE_PRINTING
			debug_print_file("ATTRIBUTES", "\tprint_header=%s", print_boolean(group->attribute->print_header));
#	endif /* !DISABLE_PRINTING */
			debug_print_file("ATTRIBUTES", "\tprocess_only_unread=%s", print_boolean(group->attribute->process_only_unread));
			debug_print_file("ATTRIBUTES", "\tprompt_followupto=%s", print_boolean(group->attribute->prompt_followupto));
			debug_print_file("ATTRIBUTES", "\tsort_article_type=%d", group->attribute->sort_article_type);
			debug_print_file("ATTRIBUTES", "\tsort_threads_type=%d", group->attribute->sort_threads_type);
			debug_print_file("ATTRIBUTES", "\tshow_author=%d", group->attribute->show_author);
			debug_print_file("ATTRIBUTES", "\tshow_info=%d", group->attribute->show_info);
			debug_print_file("ATTRIBUTES", "\tshow_signatures=%s", print_boolean(group->attribute->show_signatures));
			debug_print_file("ATTRIBUTES", "\tsigdashes=%s", print_boolean(group->attribute->sigdashes));
			debug_print_file("ATTRIBUTES", "\tsignature_repost=%s", print_boolean(group->attribute->signature_repost));
			debug_print_file("ATTRIBUTES", "\tstart_editor_offset=%s", print_boolean(group->attribute->start_editor_offset));
			debug_print_file("ATTRIBUTES", "\tthread_catchup_on_exit=%s", print_boolean(group->attribute->thread_catchup_on_exit));
			debug_print_file("ATTRIBUTES", "\ttrim_article_body=%d", group->attribute->trim_article_body);
			debug_print_file("ATTRIBUTES", "\tverbatim_handling=%s", print_boolean(group->attribute->verbatim_handling));
			debug_print_file("ATTRIBUTES", "\twrap_on_next_unread=%s", print_boolean(group->attribute->wrap_on_next_unread));
			debug_print_file("ATTRIBUTES", "\tpost_process_type=%d", group->attribute->post_process_type);
			debug_print_file("ATTRIBUTES", "\tquick_kill_scope=%s", group->attribute->quick_kill_scope);
			debug_print_file("ATTRIBUTES", "\tquick_kill_case=%s", print_boolean(group->attribute->quick_kill_case));
			debug_print_file("ATTRIBUTES", "\tquick_kill_expire=%s", print_boolean(group->attribute->quick_kill_expire));
			debug_print_file("ATTRIBUTES", "\tquick_kill_header=%d", group->attribute->quick_kill_header);
			debug_print_file("ATTRIBUTES", "\tquick_select_scope=%s", group->attribute->quick_select_scope);
			debug_print_file("ATTRIBUTES", "\tquick_select_case=%s", print_boolean(group->attribute->quick_select_case));
			debug_print_file("ATTRIBUTES", "\tquick_select_expire=%s", print_boolean(group->attribute->quick_select_expire));
			debug_print_file("ATTRIBUTES", "\tquick_select_header=%d", group->attribute->quick_select_header);
			debug_print_file("ATTRIBUTES", "\tx_comment_to=%s", print_boolean(group->attribute->x_comment_to));
			debug_print_file("ATTRIBUTES", "\tfcc=%s", group->attribute->fcc);
			debug_print_file("ATTRIBUTES", "\ttex2iso_conv=%s", print_boolean(group->attribute->tex2iso_conv));
			debug_print_file("ATTRIBUTES", "\tmime_forward=%s", print_boolean(group->attribute->mime_forward));
#	ifdef CHARSET_CONVERSION
			debug_print_file("ATTRIBUTES", "\tmm_network_charset=%s", txt_mime_charsets[group->attribute->mm_network_charset]);
			debug_print_file("ATTRIBUTES", "\tundeclared_charset=%s", group->attribute->undeclared_charset);
#	endif /* CHARSET_CONVERSION */
			debug_print_file("ATTRIBUTES", "");
		}
	}
}


static void
dump_scopes(
	void)
{
	if (!scopes)
		return;

	if (debug & DEBUG_ATTRIB) {
		int i, j;
		struct t_attribute *scope;

		for (i = 0; i < num_scope; i++) {
			scope = &scopes[i];
			debug_print_file("SCOPES", "scopes[%d]", i);
			debug_print_file("SCOPES", "scope=%s", scope->scope);
			debug_print_file("SCOPES", "\tglobal=%d", scope->global);
			debug_print_file("SCOPES", "\tmaildir=%s", scope->maildir);
			debug_print_file("SCOPES", "\tsavedir=%s", scope->savedir);
			debug_print_file("SCOPES", "\tsavefile=%s", scope->savefile);
			debug_print_file("SCOPES", "\tsigfile=%s", scope->sigfile);
			debug_print_file("SCOPES", "\torganization=%s", scope->organization);
			debug_print_file("SCOPES", "\tfollowup_to=%s", scope->followup_to);
			debug_print_file("SCOPES", "\tmailing_list=%s", scope->mailing_list);
			debug_print_file("SCOPES", "\tx_headers=%s", scope->x_headers);
			debug_print_file("SCOPES", "\tx_body=%s", scope->x_body);
			debug_print_file("SCOPES", "\tfrom=%s", scope->from);
			debug_print_file("SCOPES", "\tnews_quote_format=%s", scope->news_quote_format);
			if (scope->quote_chars)
				debug_print_file("SCOPES", "\tquote_chars=%s", quote_space_to_dash(scope->quote_chars));
			else
				debug_print_file("SCOPES", "\tquote_chars=%s", scope->quote_chars);
			debug_print_file("SCOPES", "\tmime_types_to_save=%s", scope->mime_types_to_save);
#	ifdef HAVE_ISPELL
			debug_print_file("SCOPES", "\tispell=%s", scope->ispell);
#	endif /* HAVE_ISPELL */
			debug_print_file("SCOPES", "\tshow_only_unread_arts=%s", print_boolean(scope->show_only_unread_arts));
			debug_print_file("SCOPES", "\tthread_articles=%d", scope->thread_articles);
			debug_print_file("SCOPES", "\tthread_perc=%d", scope->thread_perc);
			debug_print_file("SCOPES", "\tadd_posted_to_filter=%s", print_boolean(scope->add_posted_to_filter));
			debug_print_file("SCOPES", "\tadvertising=%s", print_boolean(scope->advertising));
			debug_print_file("SCOPES", "\talternative_handling=%s", print_boolean(scope->alternative_handling));
			debug_print_file("SCOPES", "\task_for_metamail=%s", print_boolean(scope->ask_for_metamail));
			debug_print_file("SCOPES", "\tauto_bcc=%s", print_boolean(scope->auto_bcc));
			debug_print_file("SCOPES", "\tauto_cc=%s", print_boolean(scope->auto_cc));
			debug_print_file("SCOPES", "\tauto_list_thread=%s", print_boolean(scope->auto_list_thread));
			debug_print_file("SCOPES", "\tauto_select=%s", print_boolean(scope->auto_select));
			debug_print_file("SCOPES", "\tauto_save=%s", print_boolean(scope->auto_save));
			debug_print_file("SCOPES", "\tbatch_save=%s", print_boolean(scope->batch_save));
			debug_print_file("SCOPES", "\tdate_format=%s", scope->date_format);
			debug_print_file("SCOPES", "\tdelete_tmp_files=%s", print_boolean(scope->delete_tmp_files));
			debug_print_file("SCOPES", "\teditor_format=%s", scope->editor_format);
			debug_print_file("SCOPES", "\tgroup_catchup_on_exit=%s", print_boolean(scope->group_catchup_on_exit));
			debug_print_file("SCOPES", "\tmark_ignore_tags=%s", print_boolean(scope->mark_ignore_tags));
			debug_print_file("SCOPES", "\tmark_saved_read=%s", print_boolean(scope->mark_saved_read));
			debug_print_file("SCOPES", "\tnews_headers_to_display=%s", scope->news_headers_to_display);
			if (scope->headers_to_display) {
				debug_print_file("SCOPES", "\theaders_to_display->num=%d", scope->headers_to_display->num);
				for (j = 0; j < scope->headers_to_display->num; j++)
					debug_print_file("SCOPES", "\theaders_to_display->header[%d]=%s", j, scope->headers_to_display->header[j]);
			} else
				debug_print_file("SCOPES", "\theaders_to_display=(null)");
			debug_print_file("SCOPES", "\tnews_headers_to_not_display=%s", scope->news_headers_to_not_display);
			if (scope->headers_to_not_display) {
				debug_print_file("SCOPES", "\theaders_to_not_display->num=%d", scope->headers_to_not_display->num);
				for (j = 0; j < scope->headers_to_not_display->num; j++)
					debug_print_file("SCOPES", "\theaders_to_not_display->header[%d]=%s", j, scope->headers_to_not_display->header[j]);
			} else
				debug_print_file("SCOPES", "\theaders_to_not_display=(null)");
			debug_print_file("SCOPES", "\tpos_first_unread=%s", print_boolean(scope->pos_first_unread));
			debug_print_file("SCOPES", "\tpost_process_view=%s", print_boolean(scope->post_process_view));
#	ifndef DISABLE_PRINTING
			debug_print_file("SCOPES", "\tprint_header=%s", print_boolean(scope->print_header));
#	endif /* !DISABLE_PRINTING */
			debug_print_file("SCOPES", "\tprocess_only_unread=%s", print_boolean(scope->process_only_unread));
			debug_print_file("SCOPES", "\tprompt_followupto=%s", print_boolean(scope->prompt_followupto));
			debug_print_file("SCOPES", "\tsort_article_type=%d", scope->sort_article_type);
			debug_print_file("SCOPES", "\tsort_threads_type=%d", scope->sort_threads_type);
			debug_print_file("SCOPES", "\tshow_author=%d", scope->show_author);
			debug_print_file("SCOPES", "\tshow_info=%d", scope->show_info);
			debug_print_file("SCOPES", "\tshow_signatures=%s", print_boolean(scope->show_signatures));
			debug_print_file("SCOPES", "\tsigdashes=%s", print_boolean(scope->sigdashes));
			debug_print_file("SCOPES", "\tsignature_repost=%s", print_boolean(scope->signature_repost));
			debug_print_file("SCOPES", "\tstart_editor_offset=%s", print_boolean(scope->start_editor_offset));
			debug_print_file("SCOPES", "\tthread_catchup_on_exit=%s", print_boolean(scope->thread_catchup_on_exit));
			debug_print_file("SCOPES", "\ttrim_article_body=%d", scope->trim_article_body);
			debug_print_file("SCOPES", "\tverbatim_handling=%s", print_boolean(scope->verbatim_handling));
			debug_print_file("SCOPES", "\twrap_on_next_unread=%s", print_boolean(scope->wrap_on_next_unread));
			debug_print_file("SCOPES", "\tpost_process_type=%d", scope->post_process_type);
			debug_print_file("SCOPES", "\tquick_kill_scope=%s", scope->quick_kill_scope);
			debug_print_file("SCOPES", "\tquick_kill_case=%s", print_boolean(scope->quick_kill_case));
			debug_print_file("SCOPES", "\tquick_kill_expire=%s", print_boolean(scope->quick_kill_expire));
			debug_print_file("SCOPES", "\tquick_kill_header=%d", scope->quick_kill_header);
			debug_print_file("SCOPES", "\tquick_select_scope=%s", scope->quick_select_scope);
			debug_print_file("SCOPES", "\tquick_select_case=%s", print_boolean(scope->quick_select_case));
			debug_print_file("SCOPES", "\tquick_select_expire=%s", print_boolean(scope->quick_select_expire));
			debug_print_file("SCOPES", "\tquick_select_header=%d", scope->quick_select_header);
			debug_print_file("SCOPES", "\tx_comment_to=%s", print_boolean(scope->x_comment_to));
			debug_print_file("SCOPES", "\tfcc=%s", scope->fcc);
			debug_print_file("SCOPES", "\ttex2iso_conv=%s", print_boolean(scope->tex2iso_conv));
			debug_print_file("SCOPES", "\tmime_forward=%s", print_boolean(scope->mime_forward));
#	ifdef CHARSET_CONVERSION
			debug_print_file("SCOPES", "\tmm_network_charset=%s", txt_mime_charsets[scope->mm_network_charset]);
			debug_print_file("SCOPES", "\tundeclared_charset=%s", scope->undeclared_charset);
#	endif /* CHARSET_CONVERSION */
			debug_print_file("SCOPES", "");
		}
	}
}
#endif /* DEBUG */
