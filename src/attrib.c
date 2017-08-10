/*
 *  Project   : tin - a Usenet reader
 *  Module    : attrib.c
 *  Author    : I. Lea
 *  Created   : 1993-12-01
 *  Updated   : 2017-08-02
 *  Notes     : Group attribute routines
 *
 * Copyright (c) 1993-2017 Iain Lea <iain@bricbrac.de>
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


/*
 * Global file offset for local_attributes_file
 */
int attrib_file_offset;


/*
 * Local prototypes
 */
static void set_attrib(int type, const char *scope, const char *line, void *data);
static void set_default_attributes(struct t_attribute *attributes, struct t_attribute *scope, t_bool global);
static void set_default_state(struct t_attribute_state *state);
#ifdef DEBUG
	static void dump_attributes(void);
	static void dump_scopes(const char *fname);
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
	attributes->global = (global ? TRUE : FALSE);	/* global/group specific */
	attributes->maildir = (scope ? scope->maildir : (global ? tinrc.maildir : NULL));
	attributes->savedir = (scope ? scope->savedir : (global ? tinrc.savedir : NULL));
	attributes->savefile = NULL;
	attributes->sigfile = (scope ? scope->sigfile : (global ? tinrc.sigfile : NULL));
	attributes->group_format = (scope ? scope->group_format : (global ? tinrc.group_format : NULL));
	attributes->thread_format = (scope ? scope->thread_format : (global ? tinrc.thread_format : NULL));
	attributes->date_format = (scope ? scope->date_format : (global ? tinrc.date_format : NULL));
	attributes->editor_format = (scope ? scope->editor_format : (global ? tinrc.editor_format : NULL));
	attributes->organization = (scope ? scope->organization : (global ? (*default_organization ? default_organization : NULL) : NULL));
	attributes->followup_to = NULL;
	attributes->mailing_list = NULL;
	attributes->x_headers = NULL;
	attributes->x_body = NULL;
	attributes->from = (scope ? scope->from : (global ? tinrc.mail_address : NULL));
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
	attributes->show_author = tinrc.show_author;
	attributes->show_signatures = tinrc.show_signatures;
	attributes->trim_article_body = tinrc.trim_article_body;
	attributes->verbatim_handling = tinrc.verbatim_handling;
#ifdef HAVE_COLOR
	attributes->extquote_handling = tinrc.extquote_handling;
#endif /* HAVE_COLOR */
	attributes->wrap_on_next_unread = tinrc.wrap_on_next_unread;
	attributes->add_posted_to_filter = tinrc.add_posted_to_filter;
	attributes->advertising = tinrc.advertising;
	attributes->alternative_handling = tinrc.alternative_handling;
	attributes->ask_for_metamail = tinrc.ask_for_metamail;
	attributes->auto_cc_bcc = tinrc.auto_cc_bcc;
	attributes->auto_list_thread = tinrc.auto_list_thread;
	attributes->auto_save = tinrc.auto_save;
	attributes->auto_select = FALSE;
	attributes->batch_save = tinrc.batch_save;
	attributes->delete_tmp_files = FALSE;
	attributes->group_catchup_on_exit = tinrc.group_catchup_on_exit;
	attributes->mail_8bit_header = tinrc.mail_8bit_header;
	attributes->mail_mime_encoding = tinrc.mail_mime_encoding;
	attributes->mark_ignore_tags = tinrc.mark_ignore_tags;
	attributes->mark_saved_read = tinrc.mark_saved_read;
	attributes->news_headers_to_display = (global ? tinrc.news_headers_to_display : NULL);
	attributes->headers_to_display = (scope ? (scope->headers_to_display ? scope->headers_to_display : NULL) : NULL);
	attributes->news_headers_to_not_display = (global ? tinrc.news_headers_to_not_display : NULL);
	attributes->headers_to_not_display = (scope ? (scope->headers_to_not_display ? scope->headers_to_not_display : NULL) : NULL);
	attributes->pos_first_unread = tinrc.pos_first_unread;
	attributes->post_8bit_header = tinrc.post_8bit_header;
	attributes->post_mime_encoding = tinrc.post_mime_encoding;
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


static void
set_default_state(
	struct t_attribute_state *state)
{
	state->add_posted_to_filter = FALSE;
	state->advertising = FALSE;
	state->alternative_handling = FALSE;
	state->ask_for_metamail = FALSE;
	state->auto_cc_bcc = FALSE;
	state->auto_list_thread = FALSE;
	state->auto_save = FALSE;
	state->auto_select = FALSE;
	state->batch_save = FALSE;
	state->date_format = FALSE;
	state->delete_tmp_files = FALSE;
	state->editor_format = FALSE;
	state->fcc = FALSE;
	state->followup_to = FALSE;
	state->from = FALSE;
	state->group_catchup_on_exit = FALSE;
	state->group_format = FALSE;
#ifdef HAVE_ISPELL
	state->ispell = FALSE;
#endif /* HAVE_ISPELL */
	state->mail_8bit_header = FALSE;
	state->mail_mime_encoding = FALSE;
	state->maildir = FALSE;
	state->mailing_list = FALSE;
	state->mark_ignore_tags = FALSE;
	state->mark_saved_read = FALSE;
	state->mime_forward = FALSE;
	state->mime_types_to_save = FALSE;
	state->news_headers_to_display = FALSE;
	state->news_headers_to_not_display = FALSE;
	state->news_quote_format = FALSE;
	state->organization = FALSE;
	state->pos_first_unread = FALSE;
	state->post_8bit_header = FALSE;
	state->post_mime_encoding = FALSE;
	state->post_process_type = FALSE;
	state->post_process_view = FALSE;
#ifndef DISABLE_PRINTING
	state->print_header = FALSE;
#endif /* !DISABLE_PRINTING */
	state->process_only_unread = FALSE;
	state->prompt_followupto = FALSE;
	state->quick_kill_case = FALSE;
	state->quick_kill_expire = FALSE;
	state->quick_kill_header = FALSE;
	state->quick_kill_scope = FALSE;
	state->quick_select_case = FALSE;
	state->quick_select_expire = FALSE;
	state->quick_select_header = FALSE;
	state->quick_select_scope = FALSE;
	state->quote_chars = FALSE;
	state->savedir = FALSE;
	state->savefile = FALSE;
	state->show_author = FALSE;
	state->show_only_unread_arts = FALSE;
	state->show_signatures = FALSE;
	state->sigdashes = FALSE;
	state->sigfile = FALSE;
	state->signature_repost = FALSE;
	state->sort_article_type = FALSE;
	state->sort_threads_type = FALSE;
	state->start_editor_offset = FALSE;
	state->tex2iso_conv = FALSE;
	state->thread_articles = FALSE;
	state->thread_catchup_on_exit = FALSE;
	state->thread_format = FALSE;
	state->thread_perc = FALSE;
	state->trim_article_body = FALSE;
#ifdef CHARSET_CONVERSION
	state->undeclared_charset = FALSE;
	state->mm_network_charset = FALSE;
#endif /* CHARSET_CONVERSION */
	state->verbatim_handling = FALSE;
#ifdef HAVE_COLOR
	state->extquote_handling = FALSE;
#endif /* HAVE_COLOR */
	state->wrap_on_next_unread = FALSE;
	state->x_body = FALSE;
	state->x_comment_to = FALSE;
	state->x_headers = FALSE;
}


#define MATCH_BOOLEAN(pattern, type) \
	if (match_boolean(line, pattern, &flag)) { \
		num = (flag != FALSE); \
		set_attrib(type, scope, line, &num); \
		found = TRUE; \
		break; \
	}
#define MATCH_INTEGER(pattern, type, maxval) \
	if (match_integer(line, pattern, &num, maxval)) { \
		set_attrib(type, scope, line, &num); \
		found = TRUE; \
		break; \
	}
#define MATCH_STRING(pattern, type) \
	if (match_string(line, pattern, buf, sizeof(buf) - strlen(pattern))) { \
		set_attrib(type, scope, line, buf); \
		found = TRUE; \
		break; \
	}
#define MATCH_LIST(pattern, type, table) \
	if (match_list(line, pattern, table, &num)) { \
		set_attrib(type, scope, line, &num); \
		found = TRUE; \
		break; \
	}
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
	int i, num;
	enum rc_state upgrade = RC_CHECK;
	static t_bool startup = TRUE;
	t_bool flag, found = FALSE;

	if (!batch_mode || verbose)
		wait_message(0, _(txt_reading_attributes_file), (global_file ? _(txt_global) : ""));
	/*
	 * Initialize global attributes even if there is no global file
	 * These setting are used as the default for all groups unless overridden
	 */
	if (global_file) {
		i = add_scope("_default_");
		scopes[i].global = TRUE;
		set_default_attributes(scopes[i].attribute, NULL, TRUE);
		build_news_headers_array(scopes[i].attribute, TRUE);
		build_news_headers_array(scopes[i].attribute, FALSE);
		file = global_attributes_file;
	} else {
		attrib_file_offset = 1;
		file = local_attributes_file;
	}

	if ((fp = fopen(file, "r")) != NULL) {
		scope[0] = '\0';
		while (fgets(line, (int) sizeof(line), fp) != NULL) {
			if (line[0] == '\n')
				continue;
			if (line[0] == '#') {
				if (!global_file) {
					if (scope[0] == '\0')
						attrib_file_offset++;
					if (startup && upgrade == RC_CHECK && match_string(line, "# Group attributes file V", NULL, 0)) {
						upgrade = check_upgrade(line, "# Group attributes file V", ATTRIBUTES_VERSION);
						if (upgrade != RC_IGNORE)
							upgrade_prompt_quit(upgrade, file); /* TODO: do something (more) useful here */
					}
				}
				continue;
			}

			switch (tolower((unsigned char) line[0])) {
				case 'a':
					MATCH_BOOLEAN("add_posted_to_filter=", OPT_ATTRIB_ADD_POSTED_TO_FILTER);
					MATCH_BOOLEAN("advertising=", OPT_ATTRIB_ADVERTISING);
					MATCH_BOOLEAN("alternative_handling=", OPT_ATTRIB_ALTERNATIVE_HANDLING);
					MATCH_BOOLEAN("ask_for_metamail=", OPT_ATTRIB_ASK_FOR_METAMAIL);
					MATCH_INTEGER("auto_cc_bcc=", OPT_ATTRIB_AUTO_CC_BCC, AUTO_CC_BCC);
					MATCH_BOOLEAN("auto_list_thread=", OPT_ATTRIB_AUTO_LIST_THREAD);
					MATCH_BOOLEAN("auto_save=", OPT_ATTRIB_AUTO_SAVE);
					MATCH_BOOLEAN("auto_select=", OPT_ATTRIB_AUTO_SELECT);
					break;

				case 'b':
					MATCH_BOOLEAN("batch_save=", OPT_ATTRIB_BATCH_SAVE);
					break;

				case 'd':
					MATCH_STRING("date_format=", OPT_ATTRIB_DATE_FORMAT);
					MATCH_BOOLEAN("delete_tmp_files=", OPT_ATTRIB_DELETE_TMP_FILES);
					break;

				case 'e':
					MATCH_STRING("editor_format=", OPT_ATTRIB_EDITOR_FORMAT);
#ifdef HAVE_COLOR
					MATCH_BOOLEAN("extquote_handling=", OPT_ATTRIB_EXTQUOTE_HANDLING);
#endif /* HAVE_COLOR */

					break;

				case 'f':
					MATCH_STRING("fcc=", OPT_ATTRIB_FCC);
					MATCH_STRING("followup_to=", OPT_ATTRIB_FOLLOWUP_TO);
					MATCH_STRING("from=", OPT_ATTRIB_FROM);
					break;

				case 'g':
					MATCH_BOOLEAN("group_catchup_on_exit=", OPT_ATTRIB_GROUP_CATCHUP_ON_EXIT);
					MATCH_STRING("group_format=", OPT_ATTRIB_GROUP_FORMAT);
					break;

				case 'i':
#ifdef HAVE_ISPELL
					MATCH_STRING("ispell=", OPT_ATTRIB_ISPELL);
#else
					SKIP_ITEM("ispell=");
#endif /* HAVE_ISPELL */
					break;

				case 'm':
					MATCH_BOOLEAN("mail_8bit_header=", OPT_ATTRIB_MAIL_8BIT_HEADER);
					MATCH_LIST("mail_mime_encoding=", OPT_ATTRIB_MAIL_MIME_ENCODING, txt_mime_encodings);
					MATCH_STRING("maildir=", OPT_ATTRIB_MAILDIR);
					MATCH_STRING("mailing_list=", OPT_ATTRIB_MAILING_LIST);
					MATCH_BOOLEAN("mark_ignore_tags=", OPT_ATTRIB_MARK_IGNORE_TAGS);
					MATCH_BOOLEAN("mark_saved_read=", OPT_ATTRIB_MARK_SAVED_READ);
					MATCH_BOOLEAN("mime_forward=", OPT_ATTRIB_MIME_FORWARD);
					MATCH_STRING("mime_types_to_save=", OPT_ATTRIB_MIME_TYPES_TO_SAVE);
#ifdef CHARSET_CONVERSION
					MATCH_LIST("mm_network_charset=", OPT_ATTRIB_MM_NETWORK_CHARSET, txt_mime_charsets);
#else
					SKIP_ITEM("mm_network_charset=");
#endif /* CHARSET_CONVERSION */
					break;

				case 'n':
					MATCH_STRING("news_headers_to_display=", OPT_ATTRIB_NEWS_HEADERS_TO_DISPLAY);
					MATCH_STRING("news_headers_to_not_display=", OPT_ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY);
					MATCH_STRING("news_quote_format=", OPT_ATTRIB_NEWS_QUOTE_FORMAT);
					break;

				case 'o':
					MATCH_STRING("organization=", OPT_ATTRIB_ORGANIZATION);
					break;

				case 'p':
					MATCH_BOOLEAN("pos_first_unread=", OPT_ATTRIB_POS_FIRST_UNREAD);
					MATCH_BOOLEAN("post_8bit_header=", OPT_ATTRIB_POST_8BIT_HEADER);
					MATCH_LIST("post_mime_encoding=", OPT_ATTRIB_POST_MIME_ENCODING, txt_mime_encodings);
					MATCH_BOOLEAN("post_process_view=", OPT_ATTRIB_POST_PROCESS_VIEW);
					MATCH_INTEGER("post_process_type=", OPT_ATTRIB_POST_PROCESS_TYPE, POST_PROC_YES);
#ifndef DISABLE_PRINTING
					MATCH_BOOLEAN("print_header=", OPT_ATTRIB_PRINT_HEADER);
#else
					SKIP_ITEM("print_header=");
#endif /* !DISABLE_PRINTING */
					MATCH_BOOLEAN("process_only_unread=", OPT_ATTRIB_PROCESS_ONLY_UNREAD);
					MATCH_BOOLEAN("prompt_followupto=", OPT_ATTRIB_PROMPT_FOLLOWUPTO);
					break;

				case 'q':
					MATCH_BOOLEAN("quick_kill_case=", OPT_ATTRIB_QUICK_KILL_CASE);
					MATCH_BOOLEAN("quick_kill_expire=", OPT_ATTRIB_QUICK_KILL_EXPIRE);
					MATCH_INTEGER("quick_kill_header=", OPT_ATTRIB_QUICK_KILL_HEADER, FILTER_LINES);
					MATCH_STRING("quick_kill_scope=", OPT_ATTRIB_QUICK_KILL_SCOPE);
					MATCH_BOOLEAN("quick_select_case=", OPT_ATTRIB_QUICK_SELECT_CASE);
					MATCH_BOOLEAN("quick_select_expire=", OPT_ATTRIB_QUICK_SELECT_EXPIRE);
					MATCH_INTEGER("quick_select_header=", OPT_ATTRIB_QUICK_SELECT_HEADER, FILTER_LINES);
					MATCH_STRING("quick_select_scope=", OPT_ATTRIB_QUICK_SELECT_SCOPE);
					if (match_string(line, "quote_chars=", buf, sizeof(buf))) {
						quote_dash_to_space(buf);
						set_attrib(OPT_ATTRIB_QUOTE_CHARS, scope, line, buf);
						found = TRUE;
						break;
					}
					break;

				case 's':
					MATCH_STRING("savedir=", OPT_ATTRIB_SAVEDIR);
					MATCH_STRING("savefile=", OPT_ATTRIB_SAVEFILE);
					if (match_string(line, "scope=", scope, sizeof(scope))) {
						i = add_scope(scope);
						scopes[i].global = global_file ? TRUE : FALSE;
						found = TRUE;
						break;
					}
					MATCH_INTEGER("show_author=", OPT_ATTRIB_SHOW_AUTHOR, SHOW_FROM_BOTH);
					MATCH_BOOLEAN("show_only_unread_arts=", OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS);
					MATCH_BOOLEAN("show_signatures=", OPT_ATTRIB_SHOW_SIGNATURES);
					MATCH_BOOLEAN("sigdashes=", OPT_ATTRIB_SIGDASHES);
					MATCH_BOOLEAN("signature_repost=", OPT_ATTRIB_SIGNATURE_REPOST);
					MATCH_BOOLEAN("start_editor_offset=", OPT_ATTRIB_START_EDITOR_OFFSET);
					MATCH_STRING("sigfile=", OPT_ATTRIB_SIGFILE);
					MATCH_INTEGER("sort_article_type=", OPT_ATTRIB_SORT_ARTICLE_TYPE, SORT_ARTICLES_BY_LINES_ASCEND);
					MATCH_INTEGER("sort_threads_type=", OPT_ATTRIB_SORT_THREADS_TYPE, SORT_THREADS_BY_LAST_POSTING_DATE_ASCEND);
					break;

				case 't':
					MATCH_BOOLEAN("tex2iso_conv=", OPT_ATTRIB_TEX2ISO_CONV);
					MATCH_INTEGER("thread_articles=", OPT_ATTRIB_THREAD_ARTICLES, THREAD_MAX);
					MATCH_BOOLEAN("thread_catchup_on_exit=", OPT_ATTRIB_THREAD_CATCHUP_ON_EXIT);
					MATCH_STRING("thread_format=", OPT_ATTRIB_THREAD_FORMAT);
					MATCH_INTEGER("thread_perc=", OPT_ATTRIB_THREAD_PERC, 100);
					MATCH_INTEGER("trim_article_body=", OPT_ATTRIB_TRIM_ARTICLE_BODY, 7);
					break;

				case 'u':
#ifdef CHARSET_CONVERSION
					MATCH_STRING("undeclared_charset=", OPT_ATTRIB_UNDECLARED_CHARSET);
#else
					SKIP_ITEM("undeclared_charset=");
#endif /* CHARSET_CONVERSION */
					break;

				case 'v':
					MATCH_BOOLEAN("verbatim_handling=", OPT_ATTRIB_VERBATIM_HANDLING);
					break;

				case 'w':
					MATCH_BOOLEAN("wrap_on_next_unread=", OPT_ATTRIB_WRAP_ON_NEXT_UNREAD);
					break;

				case 'x':
					MATCH_STRING("x_body=", OPT_ATTRIB_X_BODY);
					MATCH_BOOLEAN("x_comment_to=", OPT_ATTRIB_X_COMMENT_TO);
					MATCH_STRING("x_headers=", OPT_ATTRIB_X_HEADERS);
					break;

				default:
					break;
			}

			if (!global_file && upgrade == RC_UPGRADE) {
				int auto_cc_bcc;
				int show_info;
				t_bool auto_bcc = FALSE;
				t_bool auto_cc = FALSE;

				switch (tolower((unsigned char) line[0])) {
					case 'a':
						if (match_boolean(line, "auto_bcc=", &auto_bcc)) {
							if (scopes[num_scope -1].attribute->auto_cc_bcc & AUTO_CC)
								auto_cc_bcc = (auto_bcc ? AUTO_CC_BCC : AUTO_CC);
							else
								auto_cc_bcc = (auto_bcc ? AUTO_BCC : 0);
							set_attrib(OPT_ATTRIB_AUTO_CC_BCC, scope, line, &auto_cc_bcc);
							found = TRUE;
							break;
						}
						if (match_boolean(line, "auto_cc=", &auto_cc)) {
							if (scopes[num_scope -1].attribute->auto_cc_bcc & AUTO_BCC)
								auto_cc_bcc = (auto_cc ? AUTO_CC_BCC : AUTO_BCC);
							else
								auto_cc_bcc = (auto_cc ? AUTO_CC : 0);
							set_attrib(OPT_ATTRIB_AUTO_CC_BCC, scope, line, &auto_cc_bcc);
							found = TRUE;
							break;
						}
						break;

					case 'p':
						MATCH_INTEGER("post_proc_type=", OPT_ATTRIB_POST_PROCESS_TYPE, POST_PROC_YES);
						break;

					case 's':
						if (match_integer(line, "show_info=", &show_info, 3)) {
							char *gbuf = my_malloc(23);
							char *tbuf = my_malloc(23);

							switch (show_info) {
								case 0:
									strcpy(gbuf, "%n %m %R  %s  %F");
									strcpy(tbuf, "%n %m  %T  %F");
									break;

								case 2:
									strcpy(gbuf, "%n %m %R %S  %s  %F");
									strcpy(tbuf, "%n %m  [%S]  %T  %F");
									break;

								case 3:
									strcpy(gbuf, "%n %m %R %L %S  %s  %F");
									strcpy(tbuf, "%n %m  [%L,%S]  %T  %F");
									break;

								default:
									strcpy(gbuf, DEFAULT_GROUP_FORMAT);
									strcpy(tbuf, DEFAULT_THREAD_FORMAT);
									break;
							}
							set_attrib(OPT_ATTRIB_GROUP_FORMAT, scope, line, gbuf);
							set_attrib(OPT_ATTRIB_THREAD_FORMAT, scope, line, tbuf);
							free(gbuf);
							free(tbuf);
							found = TRUE;
						}
						/*
						 * previous versions has always passed groupname to external
						 * commands, now we look for %G
						 *
						 * 8 == sizeof("sigfile=")
						 */
						if (match_string(line, "sigfile=", buf, sizeof(buf) - 8) && buf[0] == '!') {
							char *newbuf = my_malloc(sizeof(buf) - 8 + 4);

							sprintf(newbuf, "%s %s", buf, "%G");
							set_attrib(OPT_ATTRIB_SIGFILE, scope, line, newbuf);
							free(newbuf);
						}
						MATCH_BOOLEAN("show_only_unread=", OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS);
						MATCH_INTEGER("sort_art_type=", OPT_ATTRIB_SORT_ARTICLE_TYPE, SORT_ARTICLES_BY_LINES_ASCEND);
						break;

					case 't':
						MATCH_INTEGER("thread_arts=", OPT_ATTRIB_THREAD_ARTICLES, THREAD_MAX);
						break;

					default:
						break;
				}
			}

			if (found)
				found = FALSE;
			else
				error_message(1, _(txt_bad_attrib), line);
		}
		fclose(fp);

		/*
		 * TODO: do something useful for the other cases
		 */
		if (!global_file && upgrade == RC_UPGRADE)
			write_attributes_file(file);
	} else if (!global_file && startup) {
		/* no local attributes file, add some useful defaults and write file */

		add_scope("*");
		snprintf(buf, sizeof(buf), "%s", "~/.tin/headers");
		set_attrib(OPT_ATTRIB_X_HEADERS, "*", "", buf);

		add_scope("*sources*");
		num = POST_PROC_SHAR;
		set_attrib(OPT_ATTRIB_POST_PROCESS_TYPE, "*sources*", "", &num);

		add_scope("*binaries*");
		num = POST_PROC_YES;
		set_attrib(OPT_ATTRIB_POST_PROCESS_TYPE, "*binaries*", "", &num);
		num = FALSE;
		set_attrib(OPT_ATTRIB_TEX2ISO_CONV, "*binaries*", "", &num);
		num = TRUE;
		set_attrib(OPT_ATTRIB_DELETE_TMP_FILES, "*binaries*", "", &num);
		snprintf(buf, sizeof(buf), "%s", "poster");
		set_attrib(OPT_ATTRIB_FOLLOWUP_TO, "*binaries*", "", buf);

		write_attributes_file(file);
	}
	if (!global_file && startup)
		startup = FALSE;
#ifdef DEBUG
	if (!global_file)
		dump_scopes("SCOPES-R");
#endif /* DEBUG */
}


#define SET_STRING(string) \
	FreeIfNeeded(curr_scope->attribute->string); \
	curr_scope->attribute->string = my_strdup((char *) data); \
	curr_scope->state->string = TRUE; \
	break
#define SET_INTEGER(integer) \
	curr_scope->attribute->integer = *((int *) data); \
	curr_scope->state->integer = TRUE; \
	break


static void
set_attrib(
	int type,
	const char *scope,
	const char *line,
	void *data)
{
	struct t_scope *curr_scope;

	if (scope == NULL || *scope == '\0') {	/* No active scope set yet */
		error_message(2, _(txt_attrib_no_scope), line);
#ifdef DEBUG
		if (debug & DEBUG_ATTRIB)
			debug_print_file("ATTRIBUTES", txt_attrib_no_scope, line);
#endif /* DEBUG */
		return;
	}

	if (num_scope > 1) {
		curr_scope = &scopes[num_scope - 1];
		/*
		 * Now set the required attribute
		 */
		switch (type) {
			case OPT_ATTRIB_MAILDIR:
				SET_STRING(maildir);

			case OPT_ATTRIB_SAVEDIR:
				SET_STRING(savedir);

			case OPT_ATTRIB_SAVEFILE:
				SET_STRING(savefile);

			case OPT_ATTRIB_ORGANIZATION:
				SET_STRING(organization);

			case OPT_ATTRIB_FROM:
				SET_STRING(from);

			case OPT_ATTRIB_SIGFILE:
				SET_STRING(sigfile);

			case OPT_ATTRIB_FOLLOWUP_TO:
				SET_STRING(followup_to);

			case OPT_ATTRIB_ADD_POSTED_TO_FILTER:
				SET_INTEGER(add_posted_to_filter);

			case OPT_ATTRIB_ADVERTISING:
				SET_INTEGER(advertising);

			case OPT_ATTRIB_ALTERNATIVE_HANDLING:
				SET_INTEGER(alternative_handling);

			case OPT_ATTRIB_ASK_FOR_METAMAIL:
				SET_INTEGER(ask_for_metamail);

			case OPT_ATTRIB_AUTO_CC_BCC:
				SET_INTEGER(auto_cc_bcc);

			case OPT_ATTRIB_AUTO_LIST_THREAD:
				SET_INTEGER(auto_list_thread);

			case OPT_ATTRIB_AUTO_SELECT:
				SET_INTEGER(auto_select);

			case OPT_ATTRIB_AUTO_SAVE:
				SET_INTEGER(auto_save);

			case OPT_ATTRIB_BATCH_SAVE:
				SET_INTEGER(batch_save);

			case OPT_ATTRIB_DATE_FORMAT:
				SET_STRING(date_format);

			case OPT_ATTRIB_DELETE_TMP_FILES:
				SET_INTEGER(delete_tmp_files);

			case OPT_ATTRIB_EDITOR_FORMAT:
				SET_STRING(editor_format);

			case OPT_ATTRIB_GROUP_CATCHUP_ON_EXIT:
				SET_INTEGER(group_catchup_on_exit);

			case OPT_ATTRIB_GROUP_FORMAT:
				SET_STRING(group_format);

			case OPT_ATTRIB_MAIL_8BIT_HEADER:
				SET_INTEGER(mail_8bit_header);

			case OPT_ATTRIB_MAIL_MIME_ENCODING:
				SET_INTEGER(mail_mime_encoding);

			case OPT_ATTRIB_MARK_IGNORE_TAGS:
				SET_INTEGER(mark_ignore_tags);

			case OPT_ATTRIB_MARK_SAVED_READ:
				SET_INTEGER(mark_saved_read);

			case OPT_ATTRIB_NEWS_HEADERS_TO_DISPLAY:
				FreeIfNeeded(curr_scope->attribute->news_headers_to_display);
				curr_scope->attribute->news_headers_to_display = my_strdup((char *) data);
				build_news_headers_array(curr_scope->attribute, TRUE);
				curr_scope->state->news_headers_to_display = TRUE;
				break;

			case OPT_ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY:
				FreeIfNeeded(curr_scope->attribute->news_headers_to_not_display);
				curr_scope->attribute->news_headers_to_not_display = my_strdup((char *) data);
				build_news_headers_array(curr_scope->attribute, FALSE);
				curr_scope->state->news_headers_to_not_display = TRUE;
				break;

			case OPT_ATTRIB_POS_FIRST_UNREAD:
				SET_INTEGER(pos_first_unread);

			case OPT_ATTRIB_POST_8BIT_HEADER:
				SET_INTEGER(post_8bit_header);

			case OPT_ATTRIB_POST_MIME_ENCODING:
				SET_INTEGER(post_mime_encoding);

			case OPT_ATTRIB_POST_PROCESS_VIEW:
				SET_INTEGER(post_process_view);

#ifndef DISABLE_PRINTING
			case OPT_ATTRIB_PRINT_HEADER:
				SET_INTEGER(print_header);
#endif /* !DISABLE_PRINTING */

			case OPT_ATTRIB_PROCESS_ONLY_UNREAD:
				SET_INTEGER(process_only_unread);

			case OPT_ATTRIB_PROMPT_FOLLOWUPTO:
				SET_INTEGER(prompt_followupto);

#ifdef HAVE_COLOR
			case OPT_ATTRIB_EXTQUOTE_HANDLING:
				SET_INTEGER(extquote_handling);
#endif /* HAVE_COLOR */

			case OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS:
				SET_INTEGER(show_only_unread_arts);

			case OPT_ATTRIB_SIGDASHES:
				SET_INTEGER(sigdashes);

			case OPT_ATTRIB_SIGNATURE_REPOST:
				SET_INTEGER(signature_repost);

			case OPT_ATTRIB_START_EDITOR_OFFSET:
				SET_INTEGER(start_editor_offset);

			case OPT_ATTRIB_THREAD_ARTICLES:
				SET_INTEGER(thread_articles);

			case OPT_ATTRIB_THREAD_CATCHUP_ON_EXIT:
				SET_INTEGER(thread_catchup_on_exit);

			case OPT_ATTRIB_THREAD_FORMAT:
				SET_STRING(thread_format);

			case OPT_ATTRIB_THREAD_PERC:
				SET_INTEGER(thread_perc);

			case OPT_ATTRIB_SHOW_AUTHOR:
				SET_INTEGER(show_author);

			case OPT_ATTRIB_SHOW_SIGNATURES:
				SET_INTEGER(show_signatures);

			case OPT_ATTRIB_TRIM_ARTICLE_BODY:
				SET_INTEGER(trim_article_body);

			case OPT_ATTRIB_VERBATIM_HANDLING:
				SET_INTEGER(verbatim_handling);

			case OPT_ATTRIB_WRAP_ON_NEXT_UNREAD:
				SET_INTEGER(wrap_on_next_unread);

			case OPT_ATTRIB_SORT_ARTICLE_TYPE:
				SET_INTEGER(sort_article_type);

			case OPT_ATTRIB_SORT_THREADS_TYPE:
				SET_INTEGER(sort_threads_type);

			case OPT_ATTRIB_POST_PROCESS_TYPE:
				SET_INTEGER(post_process_type);

			case OPT_ATTRIB_QUICK_KILL_HEADER:
				SET_INTEGER(quick_kill_header);

			case OPT_ATTRIB_QUICK_KILL_SCOPE:
				SET_STRING(quick_kill_scope);

			case OPT_ATTRIB_QUICK_KILL_EXPIRE:
				SET_INTEGER(quick_kill_expire);

			case OPT_ATTRIB_QUICK_KILL_CASE:
				SET_INTEGER(quick_kill_case);

			case OPT_ATTRIB_QUICK_SELECT_HEADER:
				SET_INTEGER(quick_select_header);

			case OPT_ATTRIB_QUICK_SELECT_SCOPE:
				SET_STRING(quick_select_scope);

			case OPT_ATTRIB_QUICK_SELECT_EXPIRE:
				SET_INTEGER(quick_select_expire);

			case OPT_ATTRIB_QUICK_SELECT_CASE:
				SET_INTEGER(quick_select_case);

			case OPT_ATTRIB_MAILING_LIST:
				SET_STRING(mailing_list);

#ifdef CHARSET_CONVERSION
			case OPT_ATTRIB_MM_NETWORK_CHARSET:
				SET_INTEGER(mm_network_charset);

			case OPT_ATTRIB_UNDECLARED_CHARSET:
				SET_STRING(undeclared_charset);
#endif /* CHARSET_CONVERSION */

			case OPT_ATTRIB_X_HEADERS:
				SET_STRING(x_headers);

			case OPT_ATTRIB_X_BODY:
				SET_STRING(x_body);

			case OPT_ATTRIB_X_COMMENT_TO:
				SET_INTEGER(x_comment_to);

			case OPT_ATTRIB_FCC:
				SET_STRING(fcc);

			case OPT_ATTRIB_NEWS_QUOTE_FORMAT:
				SET_STRING(news_quote_format);

			case OPT_ATTRIB_QUOTE_CHARS:
				SET_STRING(quote_chars);

			case OPT_ATTRIB_MIME_TYPES_TO_SAVE:
				SET_STRING(mime_types_to_save);

			case OPT_ATTRIB_MIME_FORWARD:
				SET_INTEGER(mime_forward);

#ifdef HAVE_ISPELL
			case OPT_ATTRIB_ISPELL:
				SET_STRING(ispell);
#endif /* HAVE_ISPELL */

			case OPT_ATTRIB_TEX2ISO_CONV:
				SET_INTEGER(tex2iso_conv);

			default:
				break;
		}
	}
}


/*
 * Insert a new scope entry into scopes[] and return the index.
 */
int
add_scope(
	const char *scope)
{
	if (!scope || !*scope)
		return 0;

	if ((num_scope >= max_scope) || (num_scope < 0) || (scopes == NULL))
		expand_scope();
	scopes[num_scope].scope = my_strdup(scope);
	scopes[num_scope].attribute = my_malloc(sizeof(struct t_attribute));
	set_default_attributes(scopes[num_scope].attribute, NULL, FALSE);
	scopes[num_scope].state = my_malloc(sizeof(struct t_attribute_state));
	set_default_state(scopes[num_scope].state);
	scopes[num_scope].global = FALSE;
	return num_scope++;
}


#define SET_ATTRIB(attr) do { \
		if (curr_scope->state->attr) \
			group->attribute->attr = curr_scope->attribute->attr; \
	} while (0)


/*
 * Set the attributes of all groups
 */
void
assign_attributes_to_groups(
	void)
{
	struct t_group *group;
	struct t_scope *default_scope, *curr_scope;
	t_bool found;
	int i, j;
	long processed = 0L;
#ifdef CHARSET_CONVERSION
	t_bool is_7bit;
#endif /* CHARSET_CONVERSION */

	if (!batch_mode || verbose)
		wait_message(0, _("Processing attributes... ")); /* TODO: -> lang.c */

	default_scope = &scopes[0];
	for_each_group(i) {
		if (++processed % (MODULO_COUNT_NUM) == 0)
			spin_cursor();
		group = &active[i];
		found = FALSE;
		for (j = 1; j < num_scope; j++) {
			curr_scope = &scopes[j];
			if (match_group_list(group->name, curr_scope->scope)) {
				if (group->attribute == NULL || group->attribute == default_scope->attribute) {
					group->attribute = my_malloc(sizeof(struct t_attribute));
					set_default_attributes(group->attribute, default_scope->attribute, FALSE);
					found = TRUE;
				}
				if (!found) {
					set_default_attributes(group->attribute, default_scope->attribute, FALSE);
					found = TRUE;
				}
				SET_ATTRIB(maildir);
				SET_ATTRIB(savedir);
				SET_ATTRIB(savefile);
				SET_ATTRIB(sigfile);
				SET_ATTRIB(group_format);
				SET_ATTRIB(thread_format);
				SET_ATTRIB(date_format);
				SET_ATTRIB(editor_format);
				SET_ATTRIB(organization);
				SET_ATTRIB(fcc);
				SET_ATTRIB(followup_to);
				SET_ATTRIB(mailing_list);
				SET_ATTRIB(x_headers);
				SET_ATTRIB(x_body);
				SET_ATTRIB(from);
				SET_ATTRIB(news_quote_format);
				SET_ATTRIB(quote_chars);
				SET_ATTRIB(mime_types_to_save);
#ifdef HAVE_ISPELL
				SET_ATTRIB(ispell);
#endif /* HAVE_ISPELL */
#ifdef CHARSET_CONVERSION
				SET_ATTRIB(mm_network_charset);
				SET_ATTRIB(undeclared_charset);
#endif /* CHARSET_CONVERSION */
				SET_ATTRIB(quick_kill_scope);
				SET_ATTRIB(quick_kill_header);
				SET_ATTRIB(quick_kill_case);
				SET_ATTRIB(quick_kill_expire);
				SET_ATTRIB(quick_select_scope);
				SET_ATTRIB(quick_select_header);
				SET_ATTRIB(quick_select_case);
				SET_ATTRIB(quick_select_expire);
				SET_ATTRIB(show_only_unread_arts);
				SET_ATTRIB(thread_articles);
				SET_ATTRIB(thread_catchup_on_exit);
				SET_ATTRIB(thread_perc);
				SET_ATTRIB(sort_article_type);
				SET_ATTRIB(sort_threads_type);
				SET_ATTRIB(show_author);
				SET_ATTRIB(show_signatures);
				SET_ATTRIB(trim_article_body);
				SET_ATTRIB(verbatim_handling);
#ifdef HAVE_COLOR
				SET_ATTRIB(extquote_handling);
#endif /* HAVE_COLOR */
				SET_ATTRIB(wrap_on_next_unread);
				SET_ATTRIB(add_posted_to_filter);
				SET_ATTRIB(advertising);
				SET_ATTRIB(alternative_handling);
				SET_ATTRIB(ask_for_metamail);
				SET_ATTRIB(auto_cc_bcc);
				SET_ATTRIB(auto_list_thread);
				SET_ATTRIB(auto_save);
				SET_ATTRIB(auto_select);
				SET_ATTRIB(batch_save);
				SET_ATTRIB(delete_tmp_files);
				SET_ATTRIB(group_catchup_on_exit);
				SET_ATTRIB(mail_8bit_header);
				SET_ATTRIB(mail_mime_encoding);
				SET_ATTRIB(mark_ignore_tags);
				SET_ATTRIB(mark_saved_read);
				if (curr_scope->state->news_headers_to_display)
					group->attribute->headers_to_display = curr_scope->attribute->headers_to_display;
				if (curr_scope->state->news_headers_to_not_display)
					group->attribute->headers_to_not_display = curr_scope->attribute->headers_to_not_display;
				SET_ATTRIB(pos_first_unread);
				SET_ATTRIB(post_8bit_header);
				SET_ATTRIB(post_mime_encoding);
				SET_ATTRIB(post_process_view);
				SET_ATTRIB(post_process_type);
#ifndef DISABLE_PRINTING
				SET_ATTRIB(print_header);
#endif /* !DISABLE_PRINTING */
				SET_ATTRIB(process_only_unread);
				SET_ATTRIB(prompt_followupto);
				SET_ATTRIB(sigdashes);
				SET_ATTRIB(signature_repost);
				SET_ATTRIB(start_editor_offset);
				SET_ATTRIB(x_comment_to);
				SET_ATTRIB(tex2iso_conv);
				SET_ATTRIB(mime_forward);
			}
		}
		if (found) {
			/*
			 * sort out conflicting settings
			 */
#ifdef CHARSET_CONVERSION
			/*
			 * check if we have a 7bit charset but a !7bit encoding
			 * or a 8bit charset but a !8bit encoding, update encoding if needed
			 */
			is_7bit = FALSE;
			for (j = 0; txt_mime_7bit_charsets[j] != NULL; j++) {
				if (!strcasecmp(txt_mime_charsets[group->attribute->mm_network_charset], txt_mime_7bit_charsets[j])) {
					is_7bit = TRUE;
					break;
				}
			}
			if (is_7bit) {
				if (group->attribute->mail_mime_encoding != MIME_ENCODING_7BIT)
					group->attribute->mail_mime_encoding = MIME_ENCODING_7BIT;
				if (group->attribute->post_mime_encoding != MIME_ENCODING_7BIT)
					group->attribute->post_mime_encoding = MIME_ENCODING_7BIT;
			} else {
				if (group->attribute->mail_mime_encoding == MIME_ENCODING_7BIT)
					group->attribute->mail_mime_encoding = MIME_ENCODING_QP;
				if (group->attribute->post_mime_encoding == MIME_ENCODING_7BIT)
					group->attribute->post_mime_encoding = MIME_ENCODING_8BIT;
			}
#endif /* CHARSET_CONVERSION */
			/* do not use 8 bit headers if mime encoding is not 8bit */
			if (group->attribute->mail_mime_encoding != MIME_ENCODING_8BIT)
				group->attribute->mail_8bit_header = FALSE;
			if (group->attribute->post_mime_encoding != MIME_ENCODING_8BIT)
				group->attribute->post_8bit_header = FALSE;
		} else {
			if (group->attribute && !group->attribute->global)
				free(group->attribute);
			group->attribute = default_scope->attribute;
		}
	}
#ifdef DEBUG
	dump_attributes();
#	if 0
	debug_print_filter_attributes();
#	endif /* 0 */
#endif /* DEBUG */

	if (!batch_mode || verbose)
		my_fputc('\n', stdout);

	if (!cmd_line && !batch_mode)
		clear_message();
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
	long fpos;

	if ((batch_mode || no_write || num_scope < 1) && file_size(file) != -1L)
		return;

	new_file = get_tmpfilename(file);

	if ((fp = fopen(new_file, "w+")) == NULL) {
		error_message(2, _(txt_filesystem_full_backup), ATTRIBUTES_FILE);
		free(new_file);
		return;
	}

	wait_message(0, _(txt_writing_attributes_file));

#ifdef DEBUG
	dump_scopes("SCOPES-W");
#endif /* DEBUG */

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
	fprintf(fp, _("#  auto_cc_bcc=NUM\n"));
	fprintf(fp, _("#    0=No, 1=Cc, 2=Bcc, 3=Cc and Bcc\n"));
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
	fprintf(fp, _("#  group_format=STRING (eg. %%n %%m %%R %%L  %%s  %%F)\n"));
	fprintf(fp, _("#  mail_8bit_header=ON/OFF\n"));
	fprintf(fp, _("#  mail_mime_encoding=supported_encoding"));
	for (i = 0; txt_mime_encodings[i] != NULL; i++) {
		if (!(i % 5))
			fprintf(fp, "\n#    ");
		fprintf(fp, "%s, ", txt_mime_encodings[i]);
	}
	fprintf(fp, "\n");
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
	for (i = 0; txt_mime_charsets[i] != NULL; i++) {
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
	fprintf(fp, _("#  post_8bit_header=ON/OFF\n"));
	fprintf(fp, _("#  post_mime_encoding=supported_encoding"));
	for (i = 0; txt_mime_encodings[i] != NULL; i++) {
		if (!(i % 5))
			fprintf(fp, "\n#    ");
		fprintf(fp, "%s, ", txt_mime_encodings[i]);
	}
	fprintf(fp, "\n");
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
	fprintf(fp, _("#    0=Subject: (case sensitive)  1=Subject: (ignore case)\n"));
	fprintf(fp, _("#    2=From: (case sensitive)     3=From: (ignore case)\n"));
	fprintf(fp, _("#    4=Message-ID: & full References: line\n"));
	fprintf(fp, _("#    5=Message-ID: & last References: entry only\n"));
	fprintf(fp, _("#    6=Message-ID: entry only     7=Lines:\n"));
	fprintf(fp, _("#  quick_select_scope=STRING\n"));
	fprintf(fp, _("#  quick_select_expire=ON/OFF\n"));
	fprintf(fp, _("#  quick_select_case=ON/OFF\n"));
	fprintf(fp, _("#  quick_select_header=NUM\n"));
	fprintf(fp, _("#    0=Subject: (case sensitive)  1=Subject: (ignore case)\n"));
	fprintf(fp, _("#    2=From: (case sensitive)     3=From: (ignore case)\n"));
	fprintf(fp, _("#    4=Message-ID: & full References: line\n"));
	fprintf(fp, _("#    5=Message-ID: & last References: entry only\n"));
	fprintf(fp, _("#    6=Message-ID: entry only     7=Lines:\n"));
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
	fprintf(fp, "#    %d=%s\n",
		SORT_THREADS_BY_LAST_POSTING_DATE_DESCEND, _(txt_sort_t_type[SORT_THREADS_BY_LAST_POSTING_DATE_DESCEND]));
	fprintf(fp, "#    %d=%s\n",
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
	fprintf(fp, _("#  thread_format=STRING (eg. %%n %%m [%%L]  %%T  %%F)\n"));
	fprintf(fp, _("#  thread_perc=NUM\n"));
	fprintf(fp, _("#  trim_article_body=NUM\n"));
	fprintf(fp, _("#    0 = Don't trim article body\n"));
	fprintf(fp, _("#    1 = Skip leading blank lines\n"));
	fprintf(fp, _("#    2 = Skip trailing blank lines\n"));
	fprintf(fp, _("#    3 = Skip leading and trailing blank lines\n"));
	fprintf(fp, _("#    4 = Compact multiple blank lines between text blocks\n"));
	fprintf(fp, _("#    5 = Compact multiple blank lines between text blocks and skip\n#        leading blank lines\n"));
	fprintf(fp, _("#    6 = Compact multiple blank lines between text blocks and skip\n#        trailing blank lines\n"));
	fprintf(fp, _("#    7 = Compact multiple blank lines between text blocks and skip\n#        leading and trailing blank lines\n"));
	fprintf(fp, _("#  verbatim_handling=ON/OFF\n"));
#ifdef HAVE_COLOR
	fprintf(fp, _("#  extquote_handling=ON/OFF\n"));
#endif /* HAVE_COLOR */
	fprintf(fp, _("#  wrap_on_next_unread=ON/OFF\n"));
	fprintf(fp, _("#  x_body=STRING (eg. ~/.tin/extra-body-text)\n"));
	fprintf(fp, _("#  x_comment_to=ON/OFF\n"));
	fprintf(fp, _("#  x_headers=STRING (eg. ~/.tin/extra-headers)\n"));
	fprintf(fp, _("#\n# Note that it is best to put general (global scoping)\n"));
	fprintf(fp, _("# entries first followed by group specific entries.\n#\n"));
	fprintf(fp, _("############################################################################\n"));

	/*
	 * determine the file offset
	 * this is nesessary because a changed locale setting
	 * may lead to an invalid offset
	 */
	if ((fpos = ftell(fp)) <= 0) {
		clearerr(fp);
		fclose(fp);
		unlink(new_file);
		free(new_file);
		error_message(2, _(txt_filesystem_full), ATTRIBUTES_FILE);
		return;
	}
	rewind(fp);
	attrib_file_offset = 1;
	while ((i = fgetc(fp)) != EOF) {
		if (i == '\n')
			attrib_file_offset++;
	}
	if (fseek(fp, fpos, SEEK_SET)) {
		clearerr(fp);
		fclose(fp);
		unlink(new_file);
		free(new_file);
		error_message(2, _(txt_filesystem_full), ATTRIBUTES_FILE);
		return;
	}

	if ((num_scope > 0) && (scopes != NULL)) {
		struct t_scope *scope;

		for (i = 1; i < num_scope; i++) {
			scope = &scopes[i];
			if (!scope->global) {
				fprintf(fp, "\nscope=%s\n", scope->scope);
				if (scope->state->add_posted_to_filter)
					fprintf(fp, "add_posted_to_filter=%s\n", print_boolean(scope->attribute->add_posted_to_filter));
				if (scope->state->advertising)
					fprintf(fp, "advertising=%s\n", print_boolean(scope->attribute->advertising));
				if (scope->state->alternative_handling)
					fprintf(fp, "alternative_handling=%s\n", print_boolean(scope->attribute->alternative_handling));
				if (scope->state->ask_for_metamail)
					fprintf(fp, "ask_for_metamail=%s\n", print_boolean(scope->attribute->ask_for_metamail));
				if (scope->state->auto_cc_bcc)
					fprintf(fp, "auto_cc_bcc=%u\n", scope->attribute->auto_cc_bcc);
				if (scope->state->auto_list_thread)
					fprintf(fp, "auto_list_thread=%s\n", print_boolean(scope->attribute->auto_list_thread));
				if (scope->state->auto_select)
					fprintf(fp, "auto_select=%s\n", print_boolean(scope->attribute->auto_select));
				if (scope->state->auto_save)
					fprintf(fp, "auto_save=%s\n", print_boolean(scope->attribute->auto_save));
				if (scope->state->batch_save)
					fprintf(fp, "batch_save=%s\n", print_boolean(scope->attribute->batch_save));
				if (scope->state->date_format && scope->attribute->date_format)
					fprintf(fp, "date_format=%s\n", scope->attribute->date_format);
				if (scope->state->delete_tmp_files)
					fprintf(fp, "delete_tmp_files=%s\n", print_boolean(scope->attribute->delete_tmp_files));
				if (scope->state->editor_format && scope->attribute->editor_format)
					fprintf(fp, "editor_format=%s\n", scope->attribute->editor_format);
#ifdef HAVE_COLOR
				if (scope->state->extquote_handling)
					fprintf(fp, "extquote_handling=%s\n", print_boolean(scope->attribute->extquote_handling));
#endif /* HAVE_COLOR */
				if (scope->state->fcc && scope->attribute->fcc)
					fprintf(fp, "fcc=%s\n", scope->attribute->fcc);
				if (scope->state->followup_to && scope->attribute->followup_to)
					fprintf(fp, "followup_to=%s\n", scope->attribute->followup_to);
				if (scope->state->from && scope->attribute->from)
					fprintf(fp, "from=%s\n", scope->attribute->from);
				if (scope->state->group_catchup_on_exit)
					fprintf(fp, "group_catchup_on_exit=%s\n", print_boolean(scope->attribute->group_catchup_on_exit));
				if (scope->state->group_format && scope->attribute->group_format)
					fprintf(fp, "group_format=%s\n", scope->attribute->group_format);
				if (scope->state->mail_8bit_header)
					fprintf(fp, "mail_8bit_header=%s\n", print_boolean(scope->attribute->mail_8bit_header));
				if (scope->state->mail_mime_encoding)
					fprintf(fp, "mail_mime_encoding=%s\n", txt_mime_encodings[scope->attribute->mail_mime_encoding]);
#ifdef HAVE_ISPELL
				if (scope->state->ispell && scope->attribute->ispell)
					fprintf(fp, "ispell=%s\n", scope->attribute->ispell);
#endif /* HAVE_ISPELL */
				if (scope->state->maildir && scope->attribute->maildir)
					fprintf(fp, "maildir=%s\n", scope->attribute->maildir);
				if (scope->state->mailing_list && scope->attribute->mailing_list)
					fprintf(fp, "mailing_list=%s\n", scope->attribute->mailing_list);
				if (scope->state->mark_ignore_tags)
					fprintf(fp, "mark_ignore_tags=%s\n", print_boolean(scope->attribute->mark_ignore_tags));
				if (scope->state->mark_saved_read)
					fprintf(fp, "mark_saved_read=%s\n", print_boolean(scope->attribute->mark_saved_read));
				if (scope->state->mime_forward)
					fprintf(fp, "mime_forward=%s\n", print_boolean(scope->attribute->mime_forward));
				if (scope->state->mime_types_to_save && scope->attribute->mime_types_to_save)
					fprintf(fp, "mime_types_to_save=%s\n", scope->attribute->mime_types_to_save);
#ifdef CHARSET_CONVERSION
				if (scope->state->mm_network_charset)
					fprintf(fp, "mm_network_charset=%s\n", txt_mime_charsets[scope->attribute->mm_network_charset]);
				if (scope->state->undeclared_charset && scope->attribute->undeclared_charset)
					fprintf(fp, "undeclared_charset=%s\n", scope->attribute->undeclared_charset);
#endif /* CHARSET_CONVERSION */
				if (scope->state->news_headers_to_display && scope->attribute->news_headers_to_display)
					fprintf(fp, "news_headers_to_display=%s\n", scope->attribute->news_headers_to_display);
				if (scope->state->news_headers_to_not_display && scope->attribute->news_headers_to_not_display)
					fprintf(fp, "news_headers_to_not_display=%s\n", scope->attribute->news_headers_to_not_display);
				if (scope->state->news_quote_format && scope->attribute->news_quote_format)
					fprintf(fp, "news_quote_format=%s\n", scope->attribute->news_quote_format);
				if (scope->state->organization && scope->attribute->organization)
					fprintf(fp, "organization=%s\n", scope->attribute->organization);
				if (scope->state->pos_first_unread)
					fprintf(fp, "pos_first_unread=%s\n", print_boolean(scope->attribute->pos_first_unread));
				if (scope->state->post_8bit_header)
					fprintf(fp, "post_8bit_header=%s\n", print_boolean(scope->attribute->post_8bit_header));
				if (scope->state->post_mime_encoding)
					fprintf(fp, "post_mime_encoding=%s\n", txt_mime_encodings[scope->attribute->post_mime_encoding]);
				if (scope->state->post_process_view)
					fprintf(fp, "post_process_view=%s\n", print_boolean(scope->attribute->post_process_view));
				if (scope->state->post_process_type)
					fprintf(fp, "post_process_type=%u\n", scope->attribute->post_process_type);
#ifndef DISABLE_PRINTING
				if (scope->state->print_header)
					fprintf(fp, "print_header=%s\n", print_boolean(scope->attribute->print_header));
#endif /* !DISABLE_PRINTING */
				if (scope->state->process_only_unread)
					fprintf(fp, "process_only_unread=%s\n", print_boolean(scope->attribute->process_only_unread));
				if (scope->state->prompt_followupto)
					fprintf(fp, "prompt_followupto=%s\n", print_boolean(scope->attribute->prompt_followupto));
				if (scope->state->quick_kill_scope && scope->attribute->quick_kill_scope)
					fprintf(fp, "quick_kill_scope=%s\n", scope->attribute->quick_kill_scope);
				if (scope->state->quick_kill_case)
					fprintf(fp, "quick_kill_case=%s\n", print_boolean(scope->attribute->quick_kill_case));
				if (scope->state->quick_kill_expire)
					fprintf(fp, "quick_kill_expire=%s\n", print_boolean(scope->attribute->quick_kill_expire));
				if (scope->state->quick_kill_header)
					fprintf(fp, "quick_kill_header=%u\n", scope->attribute->quick_kill_header);
				if (scope->state->quick_select_scope && scope->attribute->quick_select_scope)
					fprintf(fp, "quick_select_scope=%s\n", scope->attribute->quick_select_scope);
				if (scope->state->quick_select_case)
					fprintf(fp, "quick_select_case=%s\n", print_boolean(scope->attribute->quick_select_case));
				if (scope->state->quick_select_expire)
					fprintf(fp, "quick_select_expire=%s\n", print_boolean(scope->attribute->quick_select_expire));
				if (scope->state->quick_select_header)
					fprintf(fp, "quick_select_header=%u\n", scope->attribute->quick_select_header);
				if (scope->state->quote_chars && scope->attribute->quote_chars)
					fprintf(fp, "quote_chars=%s\n", quote_space_to_dash(scope->attribute->quote_chars));
				if (scope->state->savedir && scope->attribute->savedir)
					fprintf(fp, "savedir=%s\n", scope->attribute->savedir);
				if (scope->state->savefile && scope->attribute->savefile)
					fprintf(fp, "savefile=%s\n", scope->attribute->savefile);
				if (scope->state->show_author)
					fprintf(fp, "show_author=%u\n", scope->attribute->show_author);
				if (scope->state->show_only_unread_arts)
					fprintf(fp, "show_only_unread_arts=%s\n", print_boolean(scope->attribute->show_only_unread_arts));
				if (scope->state->show_signatures)
					fprintf(fp, "show_signatures=%s\n", print_boolean(scope->attribute->show_signatures));
				if (scope->state->sigdashes)
					fprintf(fp, "sigdashes=%s\n", print_boolean(scope->attribute->sigdashes));
				if (scope->state->sigfile && scope->attribute->sigfile)
					fprintf(fp, "sigfile=%s\n", scope->attribute->sigfile);
				if (scope->state->signature_repost)
					fprintf(fp, "signature_repost=%s\n", print_boolean(scope->attribute->signature_repost));
				if (scope->state->sort_article_type)
					fprintf(fp, "sort_article_type=%u\n", scope->attribute->sort_article_type);
				if (scope->state->sort_threads_type)
					fprintf(fp, "sort_threads_type=%u\n", scope->attribute->sort_threads_type);
				if (scope->state->start_editor_offset)
					fprintf(fp, "start_editor_offset=%s\n", print_boolean(scope->attribute->start_editor_offset));
				if (scope->state->tex2iso_conv)
					fprintf(fp, "tex2iso_conv=%s\n", print_boolean(scope->attribute->tex2iso_conv));
				if (scope->state->thread_articles)
					fprintf(fp, "thread_articles=%u\n", scope->attribute->thread_articles);
				if (scope->state->thread_catchup_on_exit)
					fprintf(fp, "thread_catchup_on_exit=%s\n", print_boolean(scope->attribute->thread_catchup_on_exit));
				if (scope->state->thread_format && scope->attribute->thread_format)
					fprintf(fp, "thread_format=%s\n", scope->attribute->thread_format);
				if (scope->state->thread_perc)
					fprintf(fp, "thread_perc=%u\n", scope->attribute->thread_perc);
				if (scope->state->trim_article_body)
					fprintf(fp, "trim_article_body=%u\n", scope->attribute->trim_article_body);
				if (scope->state->verbatim_handling)
					fprintf(fp, "verbatim_handling=%s\n", print_boolean(scope->attribute->verbatim_handling));
				if (scope->state->wrap_on_next_unread)
					fprintf(fp, "wrap_on_next_unread=%s\n", print_boolean(scope->attribute->wrap_on_next_unread));
				if (scope->state->x_headers && scope->attribute->x_headers)
					fprintf(fp, "x_headers=%s\n", scope->attribute->x_headers);
				if (scope->state->x_body && scope->attribute->x_body)
					fprintf(fp, "x_body=%s\n", scope->attribute->x_body);
				if (scope->state->x_comment_to)
					fprintf(fp, "x_comment_to=%s\n", print_boolean(scope->attribute->x_comment_to));
			}
		}
	}

	/* rename_file() preserves mode, so this is safe */
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	if ((i = ferror(fp)) || fclose(fp)) {
		error_message(2, _(txt_filesystem_full), ATTRIBUTES_FILE);
		unlink(new_file);
		if (i) {
			clearerr(fp);
			fclose(fp);
		}
	} else
		rename_file(new_file, file);

	free(new_file);
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
#	endif /* 0 */


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
			debug_print_file("ATTRIBUTES", "group=%s", BlankIfNull(group->name));
			debug_print_file("ATTRIBUTES", "\tGlobal=%d", group->attribute->global);
			debug_print_file("ATTRIBUTES", "\tmaildir=%s", BlankIfNull(group->attribute->maildir));
			debug_print_file("ATTRIBUTES", "\tsavedir=%s", BlankIfNull(group->attribute->savedir));
			debug_print_file("ATTRIBUTES", "\tsavefile=%s", BlankIfNull(group->attribute->savefile));
			debug_print_file("ATTRIBUTES", "\tsigfile=%s", BlankIfNull(group->attribute->sigfile));
			debug_print_file("ATTRIBUTES", "\torganization=%s", BlankIfNull(group->attribute->organization));
			debug_print_file("ATTRIBUTES", "\tfollowup_to=%s", BlankIfNull(group->attribute->followup_to));
			debug_print_file("ATTRIBUTES", "\tmailing_list=%s", BlankIfNull(group->attribute->mailing_list));
			debug_print_file("ATTRIBUTES", "\tx_headers=%s", BlankIfNull(group->attribute->x_headers));
			debug_print_file("ATTRIBUTES", "\tx_body=%s", BlankIfNull(group->attribute->x_body));
			debug_print_file("ATTRIBUTES", "\tfrom=%s", BlankIfNull(group->attribute->from));
			debug_print_file("ATTRIBUTES", "\tnews_quote_format=%s", BlankIfNull(group->attribute->news_quote_format));
			debug_print_file("ATTRIBUTES", "\tquote_chars=%s", quote_space_to_dash(BlankIfNull(group->attribute->quote_chars)));
			debug_print_file("ATTRIBUTES", "\tmime_types_to_save=%s", BlankIfNull(group->attribute->mime_types_to_save));
#	ifdef HAVE_ISPELL
			debug_print_file("ATTRIBUTES", "\tispell=%s", BlankIfNull(group->attribute->ispell));
#	endif /* HAVE_ISPELL */
			debug_print_file("ATTRIBUTES", "\tshow_only_unread_arts=%s", print_boolean(group->attribute->show_only_unread_arts));
			debug_print_file("ATTRIBUTES", "\tthread_articles=%d", group->attribute->thread_articles);
			debug_print_file("ATTRIBUTES", "\tthread_perc=%d", group->attribute->thread_perc);
			debug_print_file("ATTRIBUTES", "\tadd_posted_to_filter=%s", print_boolean(group->attribute->add_posted_to_filter));
			debug_print_file("ATTRIBUTES", "\tadvertising=%s", print_boolean(group->attribute->advertising));
			debug_print_file("ATTRIBUTES", "\talternative_handling=%s", print_boolean(group->attribute->alternative_handling));
			debug_print_file("ATTRIBUTES", "\task_for_metamail=%s", print_boolean(group->attribute->ask_for_metamail));
			debug_print_file("ATTRIBUTES", "\tauto_cc_bcc=%d", group->attribute->auto_cc_bcc);
			debug_print_file("ATTRIBUTES", "\tauto_list_thread=%s", print_boolean(group->attribute->auto_list_thread));
			debug_print_file("ATTRIBUTES", "\tauto_select=%s", print_boolean(group->attribute->auto_select));
			debug_print_file("ATTRIBUTES", "\tauto_save=%s", print_boolean(group->attribute->auto_save));
			debug_print_file("ATTRIBUTES", "\tbatch_save=%s", print_boolean(group->attribute->batch_save));
			debug_print_file("ATTRIBUTES", "\tdate_format=%s", BlankIfNull(group->attribute->date_format));
			debug_print_file("ATTRIBUTES", "\tdelete_tmp_files=%s", print_boolean(group->attribute->delete_tmp_files));
			debug_print_file("ATTRIBUTES", "\teditor_format=%s", BlankIfNull(group->attribute->editor_format));
#	ifdef HAVE_COLOR
			debug_print_file("ATTRIBUTES", "\textquote_handling=%s", print_boolean(group->attribute->extquote_handling));
#	endif /* HAVE_COLOR */
			debug_print_file("ATTRIBUTES", "\tgroup_catchup_on_exit=%s", print_boolean(group->attribute->group_catchup_on_exit));
			debug_print_file("ATTRIBUTES", "\tgroup_format=%s", BlankIfNull(group->attribute->group_format));
			debug_print_file("ATTRIBUTES", "\tmail_8bit_header=%s", print_boolean(group->attribute->mail_8bit_header));
			debug_print_file("ATTRIBUTES", "\tmail_mime_encoding=%s", txt_mime_encodings[group->attribute->mail_mime_encoding]);
			debug_print_file("ATTRIBUTES", "\tmark_ignore_tags=%s", print_boolean(group->attribute->mark_ignore_tags));
			debug_print_file("ATTRIBUTES", "\tmark_saved_read=%s", print_boolean(group->attribute->mark_saved_read));
			debug_print_file("ATTRIBUTES", "\tnews_headers_to_display=%s", BlankIfNull(group->attribute->news_headers_to_display));
			if (group->attribute->headers_to_display) {
				debug_print_file("ATTRIBUTES", "\theaders_to_display->num=%d", group->attribute->headers_to_display->num);
				for (j = 0; j < group->attribute->headers_to_display->num; j++)
					debug_print_file("ATTRIBUTES", "\theaders_to_display->header[%d]=%s", j, group->attribute->headers_to_display->header[j]);
			}
			debug_print_file("ATTRIBUTES", "\tnews_headers_to_not_display=%s", BlankIfNull(group->attribute->news_headers_to_not_display));
			if (group->attribute->headers_to_not_display) {
				debug_print_file("ATTRIBUTES", "\theaders_to_not_display->num=%d", group->attribute->headers_to_not_display->num);
				for (j = 0; j < group->attribute->headers_to_not_display->num; j++)
					debug_print_file("ATTRIBUTES", "\theaders_to_not_display->header[%d]=%s", j, group->attribute->headers_to_not_display->header[j]);
			}
			debug_print_file("ATTRIBUTES", "\tpos_first_unread=%s", print_boolean(group->attribute->pos_first_unread));
			debug_print_file("ATTRIBUTES", "\tpost_8bit_header=%s", print_boolean(group->attribute->post_8bit_header));
			debug_print_file("ATTRIBUTES", "\tpost_mime_encoding=%s", txt_mime_encodings[group->attribute->post_mime_encoding]);
			debug_print_file("ATTRIBUTES", "\tpost_process_view=%s", print_boolean(group->attribute->post_process_view));
#	ifndef DISABLE_PRINTING
			debug_print_file("ATTRIBUTES", "\tprint_header=%s", print_boolean(group->attribute->print_header));
#	endif /* !DISABLE_PRINTING */
			debug_print_file("ATTRIBUTES", "\tprocess_only_unread=%s", print_boolean(group->attribute->process_only_unread));
			debug_print_file("ATTRIBUTES", "\tprompt_followupto=%s", print_boolean(group->attribute->prompt_followupto));
			debug_print_file("ATTRIBUTES", "\tsort_article_type=%d", group->attribute->sort_article_type);
			debug_print_file("ATTRIBUTES", "\tsort_threads_type=%d", group->attribute->sort_threads_type);
			debug_print_file("ATTRIBUTES", "\tshow_author=%d", group->attribute->show_author);
			debug_print_file("ATTRIBUTES", "\tshow_signatures=%s", print_boolean(group->attribute->show_signatures));
			debug_print_file("ATTRIBUTES", "\tsigdashes=%s", print_boolean(group->attribute->sigdashes));
			debug_print_file("ATTRIBUTES", "\tsignature_repost=%s", print_boolean(group->attribute->signature_repost));
			debug_print_file("ATTRIBUTES", "\tstart_editor_offset=%s", print_boolean(group->attribute->start_editor_offset));
			debug_print_file("ATTRIBUTES", "\tthread_catchup_on_exit=%s", print_boolean(group->attribute->thread_catchup_on_exit));
			debug_print_file("ATTRIBUTES", "\tthread_format=%s", BlankIfNull(group->attribute->thread_format));
			debug_print_file("ATTRIBUTES", "\ttrim_article_body=%d", group->attribute->trim_article_body);
			debug_print_file("ATTRIBUTES", "\tverbatim_handling=%s", print_boolean(group->attribute->verbatim_handling));
			debug_print_file("ATTRIBUTES", "\twrap_on_next_unread=%s", print_boolean(group->attribute->wrap_on_next_unread));
			debug_print_file("ATTRIBUTES", "\tpost_process_type=%d", group->attribute->post_process_type);
			debug_print_file("ATTRIBUTES", "\tquick_kill_scope=%s", BlankIfNull(group->attribute->quick_kill_scope));
			debug_print_file("ATTRIBUTES", "\tquick_kill_case=%s", print_boolean(group->attribute->quick_kill_case));
			debug_print_file("ATTRIBUTES", "\tquick_kill_expire=%s", print_boolean(group->attribute->quick_kill_expire));
			debug_print_file("ATTRIBUTES", "\tquick_kill_header=%d", group->attribute->quick_kill_header);
			debug_print_file("ATTRIBUTES", "\tquick_select_scope=%s", BlankIfNull(group->attribute->quick_select_scope));
			debug_print_file("ATTRIBUTES", "\tquick_select_case=%s", print_boolean(group->attribute->quick_select_case));
			debug_print_file("ATTRIBUTES", "\tquick_select_expire=%s", print_boolean(group->attribute->quick_select_expire));
			debug_print_file("ATTRIBUTES", "\tquick_select_header=%d", group->attribute->quick_select_header);
			debug_print_file("ATTRIBUTES", "\tx_comment_to=%s", print_boolean(group->attribute->x_comment_to));
			debug_print_file("ATTRIBUTES", "\tfcc=%s", BlankIfNull(group->attribute->fcc));
			debug_print_file("ATTRIBUTES", "\ttex2iso_conv=%s", print_boolean(group->attribute->tex2iso_conv));
			debug_print_file("ATTRIBUTES", "\tmime_forward=%s", print_boolean(group->attribute->mime_forward));
#	ifdef CHARSET_CONVERSION
			debug_print_file("ATTRIBUTES", "\tmm_network_charset=%s", txt_mime_charsets[group->attribute->mm_network_charset]);
			debug_print_file("ATTRIBUTES", "\tundeclared_charset=%s", BlankIfNull(group->attribute->undeclared_charset));
#	endif /* CHARSET_CONVERSION */
			debug_print_file("ATTRIBUTES", "");
		}
	}
}


#define DEBUG_PRINT_STATE(attrib) (scope->state->attrib ? "+ " : "  ")
#define DEBUG_PRINT_STRING(attrib) (scope->attribute->attrib ? scope->attribute->attrib : "NULL")


static void
dump_scopes(
	const char *fname)
{
	if (!scopes)
		return;

	if (debug & DEBUG_ATTRIB) {
		int i, j;
		struct t_scope *scope;

		for (i = 0; i < num_scope; i++) {
			scope = &scopes[i];
			debug_print_file(fname, "scopes[%d]", i);
			debug_print_file(fname, "scope=%s", scope->scope ? scope->scope : "NULL");
			debug_print_file(fname, "\t  global=%d", scope->global);
			debug_print_file(fname, "\t%smaildir=%s", DEBUG_PRINT_STATE(maildir), DEBUG_PRINT_STRING(maildir));
			debug_print_file(fname, "\t%ssavedir=%s", DEBUG_PRINT_STATE(savedir), DEBUG_PRINT_STRING(savedir));
			debug_print_file(fname, "\t%ssavefile=%s", DEBUG_PRINT_STATE(savefile), DEBUG_PRINT_STRING(savefile));
			debug_print_file(fname, "\t%ssigfile=%s", DEBUG_PRINT_STATE(sigfile), DEBUG_PRINT_STRING(sigfile));
			debug_print_file(fname, "\t%sorganization=%s", DEBUG_PRINT_STATE(organization), DEBUG_PRINT_STRING(organization));
			debug_print_file(fname, "\t%sfollowup_to=%s", DEBUG_PRINT_STATE(followup_to), DEBUG_PRINT_STRING(followup_to));
			debug_print_file(fname, "\t%smailing_list=%s", DEBUG_PRINT_STATE(mailing_list), DEBUG_PRINT_STRING(mailing_list));
			debug_print_file(fname, "\t%sx_headers=%s", DEBUG_PRINT_STATE(x_headers), DEBUG_PRINT_STRING(x_headers));
			debug_print_file(fname, "\t%sx_body=%s", DEBUG_PRINT_STATE(x_body), DEBUG_PRINT_STRING(x_body));
			debug_print_file(fname, "\t%sfrom=%s", DEBUG_PRINT_STATE(from), DEBUG_PRINT_STRING(from));
			debug_print_file(fname, "\t%snews_quote_format=%s", DEBUG_PRINT_STATE(news_quote_format), DEBUG_PRINT_STRING(news_quote_format));
			debug_print_file(fname, "\t%squote_chars=%s", DEBUG_PRINT_STATE(quote_chars), quote_space_to_dash(DEBUG_PRINT_STRING(quote_chars)));
			debug_print_file(fname, "\t%smime_types_to_save=%s", DEBUG_PRINT_STATE(mime_types_to_save), DEBUG_PRINT_STRING(mime_types_to_save));
#	ifdef HAVE_ISPELL
			debug_print_file(fname, "\t%sispell=%s", DEBUG_PRINT_STATE(ispell), DEBUG_PRINT_STRING(ispell));
#	endif /* HAVE_ISPELL */
			debug_print_file(fname, "\t%sshow_only_unread_arts=%s", DEBUG_PRINT_STATE(show_only_unread_arts), print_boolean(scope->attribute->show_only_unread_arts));
			debug_print_file(fname, "\t%sthread_articles=%d", DEBUG_PRINT_STATE(thread_articles), scope->attribute->thread_articles);
			debug_print_file(fname, "\t%sthread_perc=%d", DEBUG_PRINT_STATE(thread_perc), scope->attribute->thread_perc);
			debug_print_file(fname, "\t%sadd_posted_to_filter=%s", DEBUG_PRINT_STATE(add_posted_to_filter), print_boolean(scope->attribute->add_posted_to_filter));
			debug_print_file(fname, "\t%sadvertising=%s", DEBUG_PRINT_STATE(advertising), print_boolean(scope->attribute->advertising));
			debug_print_file(fname, "\t%salternative_handling=%s", DEBUG_PRINT_STATE(alternative_handling), print_boolean(scope->attribute->alternative_handling));
			debug_print_file(fname, "\t%sask_for_metamail=%s", DEBUG_PRINT_STATE(ask_for_metamail), print_boolean(scope->attribute->ask_for_metamail));
			debug_print_file(fname, "\t%sauto_cc_bcc=%d", DEBUG_PRINT_STATE(auto_cc_bcc), scope->attribute->auto_cc_bcc);
			debug_print_file(fname, "\t%sauto_list_thread=%s", DEBUG_PRINT_STATE(auto_list_thread), print_boolean(scope->attribute->auto_list_thread));
			debug_print_file(fname, "\t%sauto_select=%s", DEBUG_PRINT_STATE(auto_select), print_boolean(scope->attribute->auto_select));
			debug_print_file(fname, "\t%sauto_save=%s", DEBUG_PRINT_STATE(auto_save), print_boolean(scope->attribute->auto_save));
			debug_print_file(fname, "\t%sbatch_save=%s", DEBUG_PRINT_STATE(batch_save), print_boolean(scope->attribute->batch_save));
			debug_print_file(fname, "\t%sdate_format=%s", DEBUG_PRINT_STATE(date_format), DEBUG_PRINT_STRING(date_format));
			debug_print_file(fname, "\t%sdelete_tmp_files=%s", DEBUG_PRINT_STATE(delete_tmp_files), print_boolean(scope->attribute->delete_tmp_files));
			debug_print_file(fname, "\t%seditor_format=%s", DEBUG_PRINT_STATE(editor_format), DEBUG_PRINT_STRING(editor_format));
#	ifdef HAVE_COLOR
			debug_print_file(fname, "\t%sextquote_handling=%s", DEBUG_PRINT_STATE(extquote_handling), print_boolean(scope->attribute->extquote_handling));
#	endif /* HAVE_COLOR */
			debug_print_file(fname, "\t%sgroup_catchup_on_exit=%s", DEBUG_PRINT_STATE(group_catchup_on_exit), print_boolean(scope->attribute->group_catchup_on_exit));
			debug_print_file(fname, "\t%sgroup_format=%s", DEBUG_PRINT_STATE(group_format), DEBUG_PRINT_STRING(group_format));
			debug_print_file(fname, "\t%smail_8bit_header=%s", DEBUG_PRINT_STATE(mail_8bit_header), print_boolean(scope->attribute->mail_8bit_header));
			debug_print_file(fname, "\t%smail_mime_encoding=%s", DEBUG_PRINT_STATE(mail_mime_encoding), txt_mime_encodings[scope->attribute->mail_mime_encoding]);
			debug_print_file(fname, "\t%smark_ignore_tags=%s", DEBUG_PRINT_STATE(mark_ignore_tags), print_boolean(scope->attribute->mark_ignore_tags));
			debug_print_file(fname, "\t%smark_saved_read=%s", DEBUG_PRINT_STATE(mark_saved_read), print_boolean(scope->attribute->mark_saved_read));
			debug_print_file(fname, "\t%snews_headers_to_display=%s", DEBUG_PRINT_STATE(news_headers_to_display), DEBUG_PRINT_STRING(news_headers_to_display));
			if (scope->attribute->headers_to_display) {
				debug_print_file(fname, "\t    headers_to_display->num=%d", scope->attribute->headers_to_display->num);
				for (j = 0; j < scope->attribute->headers_to_display->num; j++)
					debug_print_file(fname, "\t    headers_to_display->header[%d]=%s", j, scope->attribute->headers_to_display->header[j]);
			}
			debug_print_file(fname, "\t%snews_headers_to_not_display=%s", DEBUG_PRINT_STATE(news_headers_to_not_display), DEBUG_PRINT_STRING(news_headers_to_not_display));
			if (scope->attribute->headers_to_not_display) {
				debug_print_file(fname, "\t    headers_to_not_display->num=%d", scope->attribute->headers_to_not_display->num);
				for (j = 0; j < scope->attribute->headers_to_not_display->num; j++)
					debug_print_file(fname, "\t    headers_to_not_display->header[%d]=%s", j, scope->attribute->headers_to_not_display->header[j]);
			}
			debug_print_file(fname, "\t%spos_first_unread=%s", DEBUG_PRINT_STATE(pos_first_unread), print_boolean(scope->attribute->pos_first_unread));
			debug_print_file(fname, "\t%spost_8bit_header=%s", DEBUG_PRINT_STATE(post_8bit_header), print_boolean(scope->attribute->post_8bit_header));
			debug_print_file(fname, "\t%spost_mime_encoding=%s", DEBUG_PRINT_STATE(post_mime_encoding), txt_mime_encodings[scope->attribute->post_mime_encoding]);
			debug_print_file(fname, "\t%spost_process_view=%s", DEBUG_PRINT_STATE(post_process_view), print_boolean(scope->attribute->post_process_view));
#	ifndef DISABLE_PRINTING
			debug_print_file(fname, "\t%sprint_header=%s", DEBUG_PRINT_STATE(print_header), print_boolean(scope->attribute->print_header));
#	endif /* !DISABLE_PRINTING */
			debug_print_file(fname, "\t%sprocess_only_unread=%s", DEBUG_PRINT_STATE(process_only_unread), print_boolean(scope->attribute->process_only_unread));
			debug_print_file(fname, "\t%sprompt_followupto=%s", DEBUG_PRINT_STATE(prompt_followupto), print_boolean(scope->attribute->prompt_followupto));
			debug_print_file(fname, "\t%ssort_article_type=%d", DEBUG_PRINT_STATE(sort_article_type), scope->attribute->sort_article_type);
			debug_print_file(fname, "\t%ssort_threads_type=%d", DEBUG_PRINT_STATE(sort_threads_type), scope->attribute->sort_threads_type);
			debug_print_file(fname, "\t%sshow_author=%d", DEBUG_PRINT_STATE(show_author), scope->attribute->show_author);
			debug_print_file(fname, "\t%sshow_signatures=%s", DEBUG_PRINT_STATE(show_signatures), print_boolean(scope->attribute->show_signatures));
			debug_print_file(fname, "\t%ssigdashes=%s", DEBUG_PRINT_STATE(sigdashes), print_boolean(scope->attribute->sigdashes));
			debug_print_file(fname, "\t%ssignature_repost=%s", DEBUG_PRINT_STATE(signature_repost), print_boolean(scope->attribute->signature_repost));
			debug_print_file(fname, "\t%sstart_editor_offset=%s", DEBUG_PRINT_STATE(start_editor_offset), print_boolean(scope->attribute->start_editor_offset));
			debug_print_file(fname, "\t%sthread_catchup_on_exit=%s", DEBUG_PRINT_STATE(thread_catchup_on_exit), print_boolean(scope->attribute->thread_catchup_on_exit));
			debug_print_file(fname, "\t%sthread_format=%s", DEBUG_PRINT_STATE(thread_format), DEBUG_PRINT_STRING(thread_format));
			debug_print_file(fname, "\t%strim_article_body=%d", DEBUG_PRINT_STATE(trim_article_body), scope->attribute->trim_article_body);
			debug_print_file(fname, "\t%sverbatim_handling=%s", DEBUG_PRINT_STATE(verbatim_handling), print_boolean(scope->attribute->verbatim_handling));
			debug_print_file(fname, "\t%swrap_on_next_unread=%s", DEBUG_PRINT_STATE(wrap_on_next_unread), print_boolean(scope->attribute->wrap_on_next_unread));
			debug_print_file(fname, "\t%spost_process_type=%d", DEBUG_PRINT_STATE(post_process_type), scope->attribute->post_process_type);
			debug_print_file(fname, "\t%squick_kill_scope=%s", DEBUG_PRINT_STATE(quick_kill_scope), DEBUG_PRINT_STRING(quick_kill_scope));
			debug_print_file(fname, "\t%squick_kill_case=%s", DEBUG_PRINT_STATE(quick_kill_case), print_boolean(scope->attribute->quick_kill_case));
			debug_print_file(fname, "\t%squick_kill_expire=%s", DEBUG_PRINT_STATE(quick_kill_expire), print_boolean(scope->attribute->quick_kill_expire));
			debug_print_file(fname, "\t%squick_kill_header=%d", DEBUG_PRINT_STATE(quick_kill_header), scope->attribute->quick_kill_header);
			debug_print_file(fname, "\t%squick_select_scope=%s", DEBUG_PRINT_STATE(quick_select_scope), DEBUG_PRINT_STRING(quick_select_scope));
			debug_print_file(fname, "\t%squick_select_case=%s", DEBUG_PRINT_STATE(quick_select_case), print_boolean(scope->attribute->quick_select_case));
			debug_print_file(fname, "\t%squick_select_expire=%s", DEBUG_PRINT_STATE(quick_select_expire), print_boolean(scope->attribute->quick_select_expire));
			debug_print_file(fname, "\t%squick_select_header=%d", DEBUG_PRINT_STATE(quick_select_header), scope->attribute->quick_select_header);
			debug_print_file(fname, "\t%sx_comment_to=%s", DEBUG_PRINT_STATE(x_comment_to), print_boolean(scope->attribute->x_comment_to));
			debug_print_file(fname, "\t%sfcc=%s", DEBUG_PRINT_STATE(fcc), DEBUG_PRINT_STRING(fcc));
			debug_print_file(fname, "\t%stex2iso_conv=%s", DEBUG_PRINT_STATE(tex2iso_conv), print_boolean(scope->attribute->tex2iso_conv));
			debug_print_file(fname, "\t%smime_forward=%s", DEBUG_PRINT_STATE(mime_forward), print_boolean(scope->attribute->mime_forward));
#	ifdef CHARSET_CONVERSION
			debug_print_file(fname, "\t%smm_network_charset=%s", DEBUG_PRINT_STATE(mm_network_charset), txt_mime_charsets[scope->attribute->mm_network_charset]);
			debug_print_file(fname, "\t%sundeclared_charset=%s", DEBUG_PRINT_STATE(undeclared_charset), DEBUG_PRINT_STRING(undeclared_charset));
#	endif /* CHARSET_CONVERSION */
			debug_print_file(fname, "");
		}
	}
}
#endif /* DEBUG */
