/*
 *  Project   : tin - a Usenet reader
 *  Module    : config.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2004-06-06
 *  Notes     : Configuration file routines
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
#ifndef TINTBL_H
#	include "tincfg.h"
#endif /* !TINTBL_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* TNNTP_H */

/*
 * local prototypes
 */
static int get_option_num(int act_option);
static int set_option_num(int option);
static t_bool OptionOnPage(int option);
static t_bool match_item(char *line, const char *pat, char *dst, size_t dstlen);
static t_bool rc_update(FILE *fp);
static void RepaintOption(int option);
static void check_score_defaults(void);
static void highlight_option(int option);
static void print_any_option(int act_option);
static void print_option(enum option_enum the_option);
static void redraw_screen(int option);
static void show_config_page(void);
static void unhighlight_option(int option);
static void write_server_config(void);
#ifdef HAVE_COLOR
	static t_bool match_color(char *line, const char *pat, int *dst, int max);
#endif /* HAVE_COLOR */
#ifdef USE_CURSES
	static void DoScroll(int jump);
#endif /* USE_CURSES */


#define DASH_TO_SPACE(mark)	((char) (mark == '_' ? ' ' : mark))
#define SPACE_TO_DASH(mark)	((char) (mark == ' ' ? '_' : mark))


/*
 * read local & global configuration defaults
 */
t_bool
read_config_file(
	char *file,
	t_bool global_file) /* return value is always ignored */
{
	FILE *fp;
	char buf[LEN], tmp[LEN];
	int upgrade = RC_CHECK;

	if ((fp = fopen(file, "r")) == NULL)
		return FALSE;

#if 0 /* batch_mode is not set at this stage, so checking for it is useless */
	if (!batch_mode)
#endif /* 0 */
		wait_message(0, _(txt_reading_config_file), (global_file) ? _(txt_global) : "");

	while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
		if (buf[0] == '#' || buf[0] == '\n') {
			if (upgrade == RC_CHECK && !global_file) {
				upgrade = check_upgrade(buf, "# tin configuration file V", TINRC_VERSION);
				if (upgrade != RC_IGNORE)
					upgrade_prompt_quit(upgrade, CONFIG_FILE);
				if (upgrade == RC_UPGRADE)
					rc_update(fp);
			}
			continue;
		}

		switch (tolower((unsigned char) buf[0])) {
		case 'a':
			if (match_boolean(buf, "add_posted_to_filter=", &tinrc.add_posted_to_filter))
				break;

			if (match_boolean(buf, "advertising=", &tinrc.advertising))
				break;

			if (match_boolean(buf, "alternative_handling=", &tinrc.alternative_handling))
				break;

			if (match_string(buf, "art_marked_deleted=", tmp, sizeof(tmp))) {
				tinrc.art_marked_deleted = !tmp[0] ? ART_MARK_DELETED : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_inrange=", tmp, sizeof(tmp))) {
				tinrc.art_marked_inrange = !tmp[0] ? MARK_INRANGE : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_killed=", tmp, sizeof(tmp))) {
				tinrc.art_marked_killed = !tmp[0] ? ART_MARK_KILLED : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_read=", tmp, sizeof(tmp))) {
				tinrc.art_marked_read = !tmp[0] ? ART_MARK_READ : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_read_selected=", tmp, sizeof(tmp))) {
				tinrc.art_marked_read_selected = !tmp[0] ? ART_MARK_READ_SELECTED : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_recent=", tmp, sizeof(tmp))) {
				tinrc.art_marked_recent = !tmp[0] ? ART_MARK_RECENT : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_return=", tmp, sizeof(tmp))) {
				tinrc.art_marked_return = !tmp[0] ? ART_MARK_RETURN : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_selected=", tmp, sizeof(tmp))) {
				tinrc.art_marked_selected = !tmp[0] ? ART_MARK_SELECTED : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_string(buf, "art_marked_unread=", tmp, sizeof(tmp))) {
				tinrc.art_marked_unread = !tmp[0] ? ART_MARK_UNREAD : DASH_TO_SPACE(tmp[0]);
				break;
			}

			if (match_boolean(buf, "ask_for_metamail=", &tinrc.ask_for_metamail))
				break;

			if (match_boolean(buf, "auto_bcc=", &tinrc.auto_bcc))
				break;

			if (match_boolean(buf, "auto_cc=", &tinrc.auto_cc))
				break;

			if (match_boolean(buf, "auto_list_thread=", &tinrc.auto_list_thread))
				break;

			if (match_boolean(buf, "auto_reconnect=", &tinrc.auto_reconnect))
				break;

			if (match_boolean(buf, "auto_save=", &tinrc.auto_save))
				break;

			break;

		case 'b':
			if (match_boolean(buf, "batch_save=", &tinrc.batch_save))
				break;

			if (match_boolean(buf, "beginner_level=", &tinrc.beginner_level))
				break;

			break;

		case 'c':
			if (match_boolean(buf, "cache_overview_files=", &tinrc.cache_overview_files))
				break;

			if (match_boolean(buf, "catchup_read_groups=", &tinrc.catchup_read_groups))
				break;

#ifdef HAVE_COLOR
			if (match_color(buf, "col_back=", &tinrc.col_back, MAX_COLOR))
				break;

			if (match_color(buf, "col_invers_bg=", &tinrc.col_invers_bg, MAX_COLOR))
				break;

			if (match_color(buf, "col_invers_fg=", &tinrc.col_invers_fg, MAX_COLOR))
				break;

			if (match_color(buf, "col_text=", &tinrc.col_text, MAX_COLOR))
				break;

			if (match_color(buf, "col_minihelp=", &tinrc.col_minihelp, MAX_COLOR))
				break;

			if (match_color(buf, "col_help=", &tinrc.col_help, MAX_COLOR))
				break;

			if (match_color(buf, "col_message=", &tinrc.col_message, MAX_COLOR))
				break;

			if (match_color(buf, "col_quote=", &tinrc.col_quote, MAX_COLOR))
				break;

			if (match_color(buf, "col_quote2=", &tinrc.col_quote2, MAX_COLOR))
				break;

			if (match_color(buf, "col_quote3=", &tinrc.col_quote3, MAX_COLOR))
				break;

			if (match_color(buf, "col_head=", &tinrc.col_head, MAX_COLOR))
				break;

			if (match_color(buf, "col_newsheaders=", &tinrc.col_newsheaders, MAX_COLOR))
				break;

			if (match_color(buf, "col_subject=", &tinrc.col_subject, MAX_COLOR))
				break;

			if (match_color(buf, "col_response=", &tinrc.col_response, MAX_COLOR))
				break;

			if (match_color(buf, "col_from=", &tinrc.col_from, MAX_COLOR))
				break;

			if (match_color(buf, "col_normal=", &tinrc.col_normal, MAX_COLOR))
				break;

			if (match_color(buf, "col_title=", &tinrc.col_title, MAX_COLOR))
				break;

			if (match_color(buf, "col_signature=", &tinrc.col_signature, MAX_COLOR))
				break;

			if (match_color(buf, "col_urls=", &tinrc.col_urls, MAX_COLOR))
				break;

			if (match_color(buf, "col_markstar=", &tinrc.col_markstar, MAX_COLOR))
				break;

			if (match_color(buf, "col_markdash=", &tinrc.col_markdash, MAX_COLOR))
				break;

			if (match_color(buf, "col_markslash=", &tinrc.col_markslash, MAX_COLOR))
				break;

			if (match_color(buf, "col_markstroke=", &tinrc.col_markstroke, MAX_COLOR))
				break;
#endif /* HAVE_COLOR */
			if (match_list(buf, "confirm_choice=", txt_confirm_choices, NUM_CONFIRM_CHOICES, &tinrc.confirm_choice))
				break;

			break;

		case 'd':
			if (match_string(buf, "date_format=", tinrc.date_format, sizeof(tinrc.date_format)))
				break;

			if (match_string(buf, "default_editor_format=", tinrc.editor_format, sizeof(tinrc.editor_format)))
				break;

			if (match_string(buf, "default_mailer_format=", tinrc.mailer_format, sizeof(tinrc.mailer_format)))
				break;

			if (match_string(buf, "default_savedir=", tinrc.savedir, sizeof(tinrc.savedir))) {
				if (tinrc.savedir[0] == '.' && strlen(tinrc.savedir) == 1) {
					get_cwd(buf);
					my_strncpy(tinrc.savedir, buf, sizeof(tinrc.savedir) - 1);
				}
				break;
			}

			if (match_string(buf, "default_maildir=", tinrc.maildir, sizeof(tinrc.maildir)))
				break;

#ifndef DISABLE_PRINTING
			if (match_string(buf, "default_printer=", tinrc.printer, sizeof(tinrc.printer)))
				break;
#endif /* !DISABLE_PRINTING */

			if (match_string(buf, "default_sigfile=", tinrc.sigfile, sizeof(tinrc.sigfile)))
				break;

			if (match_integer(buf, "default_filter_days=", &tinrc.filter_days, 0)) {
				if (tinrc.filter_days <= 0)
					tinrc.filter_days = 1;
				break;
			}

			if (match_integer(buf, "default_filter_kill_header=", &tinrc.default_filter_kill_header, FILTER_LINES))
				break;

			if (match_boolean(buf, "default_filter_kill_global=", &tinrc.default_filter_kill_global))
				break;

			if (match_boolean(buf, "default_filter_kill_case=", &tinrc.default_filter_kill_case)) {
				/* ON=false, OFF=true */
				tinrc.default_filter_kill_case = bool_not(tinrc.default_filter_kill_case);
				break;
			}

			if (match_boolean(buf, "default_filter_kill_expire=", &tinrc.default_filter_kill_expire))
				break;

			if (match_integer(buf, "default_filter_select_header=", &tinrc.default_filter_select_header, FILTER_LINES))
				break;

			if (match_boolean(buf, "default_filter_select_global=", &tinrc.default_filter_select_global))
				break;

			if (match_boolean(buf, "default_filter_select_case=", &tinrc.default_filter_select_case)) {
				/* ON=false, OFF=true */
				tinrc.default_filter_select_case = bool_not(tinrc.default_filter_select_case);
				break;
			}

			if (match_boolean(buf, "default_filter_select_expire=", &tinrc.default_filter_select_expire))
				break;

			if (match_string(buf, "default_save_mode=", tmp, sizeof(tmp))) {
				tinrc.default_save_mode = tmp[0];
				break;
			}

			if (match_string(buf, "default_author_search=", tinrc.default_search_author, sizeof(tinrc.default_search_author)))
				break;

			if (match_string(buf, "default_goto_group=", tinrc.default_goto_group, sizeof(tinrc.default_goto_group)))
				break;

			if (match_string(buf, "default_config_search=", tinrc.default_search_config, sizeof(tinrc.default_search_config)))
				break;

			if (match_string(buf, "default_group_search=", tinrc.default_search_group, sizeof(tinrc.default_search_group)))
				break;

			if (match_string(buf, "default_subject_search=", tinrc.default_search_subject, sizeof(tinrc.default_search_subject)))
				break;

			if (match_string(buf, "default_art_search=", tinrc.default_search_art, sizeof(tinrc.default_search_art)))
				break;

			if (match_string(buf, "default_repost_group=", tinrc.default_repost_group, sizeof(tinrc.default_repost_group)))
				break;

			if (match_string(buf, "default_mail_address=", tinrc.default_mail_address, sizeof(tinrc.default_mail_address)))
				break;

			if (match_integer(buf, "default_move_group=", &tinrc.default_move_group, 0))
				break;

#ifndef DONT_HAVE_PIPING
			if (match_string(buf, "default_pipe_command=", tinrc.default_pipe_command, sizeof(tinrc.default_pipe_command)))
				break;
#endif /* !DONT_HAVE_PIPING */

			if (match_string(buf, "default_post_newsgroups=", tinrc.default_post_newsgroups, sizeof(tinrc.default_post_newsgroups)))
				break;

			if (match_string(buf, "default_post_subject=", tinrc.default_post_subject, sizeof(tinrc.default_post_subject)))
				break;

			if (match_string(buf, "default_pattern=", tinrc.default_pattern, sizeof(tinrc.default_pattern)))
				break;

			if (match_string(buf, "default_range_group=", tinrc.default_range_group, sizeof(tinrc.default_range_group)))
				break;

			if (match_string(buf, "default_range_select=", tinrc.default_range_select, sizeof(tinrc.default_range_select)))
				break;

			if (match_string(buf, "default_range_thread=", tinrc.default_range_thread, sizeof(tinrc.default_range_thread)))
				break;

			if (match_string(buf, "default_save_file=", tinrc.default_save_file, sizeof(tinrc.default_save_file)))
				break;

			if (match_string(buf, "default_select_pattern=", tinrc.default_select_pattern, sizeof(tinrc.default_select_pattern)))
				break;

			if (match_string(buf, "default_shell_command=", tinrc.default_shell_command, sizeof(tinrc.default_shell_command)))
				break;

			if (match_boolean(buf, "draw_arrow=", &tinrc.draw_arrow))
				break;

			break;

		case 'f':
			if (match_boolean(buf, "force_screen_redraw=", &tinrc.force_screen_redraw))
				break;

			break;

		case 'g':
			if (match_integer(buf, "getart_limit=", &tinrc.getart_limit, 0))
				break;

			if (match_integer(buf, "groupname_max_length=", &tinrc.groupname_max_length, 132))
				break;

			if (match_boolean(buf, "group_catchup_on_exit=", &tinrc.group_catchup_on_exit))
				break;

			break;

		case 'h':
			if (match_integer(buf, "hide_uue=", &tinrc.hide_uue, UUE_ALL))
				break;

			break;

		case 'i':
			if (match_boolean(buf, "info_in_last_line=", &tinrc.info_in_last_line))
				break;

			if (match_boolean(buf, "inverse_okay=", &tinrc.inverse_okay))
				break;

			if (match_string(buf, "inews_prog=", tinrc.inews_prog, sizeof(tinrc.inews_prog)))
				break;

			if (match_integer(buf, "interactive_mailer=", &tinrc.interactive_mailer, INTERACTIVE_NONE))
				break;
			break;

		case 'k':
			if (match_boolean(buf, "keep_dead_articles=", &tinrc.keep_dead_articles))
				break;

			if (match_integer(buf, "kill_level=", &tinrc.kill_level, KILL_NOTHREAD))
				break;

			break;

		case 'm':
			if (match_list(buf, "mail_mime_encoding=", txt_mime_encodings, NUM_MIME_ENCODINGS, &tinrc.mail_mime_encoding))
				break;

			if (match_boolean(buf, "mail_8bit_header=", &tinrc.mail_8bit_header)) {
				if (strcasecmp(txt_mime_encodings[tinrc.mail_mime_encoding], txt_8bit))
					tinrc.mail_8bit_header = FALSE;
				break;
			}

#ifndef CHARSET_CONVERSION
			if (match_string(buf, "mm_charset=", tinrc.mm_charset, sizeof(tinrc.mm_charset)))
				break;
#else
			if (match_list(buf, "mm_charset=", txt_mime_charsets, NUM_MIME_CHARSETS, &tinrc.mm_network_charset))
				break;
			if (match_list(buf, "mm_network_charset=", txt_mime_charsets, NUM_MIME_CHARSETS, &tinrc.mm_network_charset))
				break;
#endif /* !CHARSET_CONVERSION */

			if (match_boolean(buf, "mark_ignore_tags=", &tinrc.mark_ignore_tags))
				break;

			if (match_boolean(buf, "mark_saved_read=", &tinrc.mark_saved_read))
				break;

			if (match_string(buf, "mail_address=", tinrc.mail_address, sizeof(tinrc.mail_address)))
				break;

			if (match_string(buf, "mail_quote_format=", tinrc.mail_quote_format, sizeof(tinrc.mail_quote_format)))
				break;

			if (match_list(buf, "mailbox_format=", txt_mailbox_formats, NUM_MAILBOX_FORMATS, &tinrc.mailbox_format))
				break;

			if (match_string(buf, "metamail_prog=", tinrc.metamail_prog, sizeof(tinrc.metamail_prog)))
				break;

			if (match_integer(buf, "mono_markdash=", &tinrc.mono_markdash, MAX_ATTR))
				break;

			if (match_integer(buf, "mono_markstar=", &tinrc.mono_markstar, MAX_ATTR))
				break;

			if (match_integer(buf, "mono_markslash=", &tinrc.mono_markslash, MAX_ATTR))
				break;

			if (match_integer(buf, "mono_markstroke=", &tinrc.mono_markstroke, MAX_ATTR))
				break;

			break;

		case 'n':
			if (match_string(buf, "newnews=", tmp, sizeof(tmp))) {
				load_newnews_info(tmp);
				break;
			}

			/* pick which news headers to display */
			if (match_string(buf, "news_headers_to_display=", tinrc.news_headers_to_display, sizeof(tinrc.news_headers_to_display))) {
				if (news_headers_to_display_array)
					FreeIfNeeded(*news_headers_to_display_array);
				FreeIfNeeded(news_headers_to_display_array);
				news_headers_to_display_array = ulBuildArgv(tinrc.news_headers_to_display, &num_headers_to_display);
				break;
			}

			/* pick which news headers to NOT display */
			if (match_string(buf, "news_headers_to_not_display=", tinrc.news_headers_to_not_display, sizeof(tinrc.news_headers_to_not_display))) {
				if (news_headers_to_not_display_array)
					FreeIfNeeded(*news_headers_to_not_display_array);
				FreeIfNeeded(news_headers_to_not_display_array);
				news_headers_to_not_display_array = ulBuildArgv(tinrc.news_headers_to_not_display, &num_headers_to_not_display);
				break;
			}

			if (match_string(buf, "news_quote_format=", tinrc.news_quote_format, sizeof(tinrc.news_quote_format)))
				break;

#ifdef HAVE_UNICODE_NORMALIZATION
#	ifdef HAVE_LIBICUUC
			if (match_integer(buf, "normalization_form=", &tinrc.normalization_form, NORMALIZE_NFD))
				break;
#	else
#		ifdef HAVE_LIBIDN
			if (match_integer(buf, "normalization_form=", &tinrc.normalization_form, NORMALIZE_NFKC))
				break;
#		endif /* HAVE_LIBIDN */
#	endif /* HAVE_LIBICUUC */
#endif /* HAVE_UNICODE_NORMALIZATION */

			break;

		case 'p':
			if (match_list(buf, "post_mime_encoding=", txt_mime_encodings, NUM_MIME_ENCODINGS, &tinrc.post_mime_encoding))
				break;

			if (match_boolean(buf, "post_8bit_header=", &tinrc.post_8bit_header)) {
				/* if post_mime_encoding != 8bit, post_8bit_header is disabled */
				if (strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_8bit))
					tinrc.post_8bit_header = FALSE;
				break;
			}

#ifndef DISABLE_PRINTING
			if (match_boolean(buf, "print_header=", &tinrc.print_header))
				break;
#endif /* !DISABLE_PRINTING */

			if (match_boolean(buf, "pos_first_unread=", &tinrc.pos_first_unread))
				break;

			if (match_integer(buf, "post_process_type=", &tinrc.post_process, POST_PROC_YES))
				break;

			if (match_boolean(buf, "post_process_view=", &tinrc.post_process_view))
				break;

			if (match_string(buf, "posted_articles_file=", tinrc.posted_articles_file, sizeof(tinrc.posted_articles_file)))
				break;

			if (match_boolean(buf, "process_only_unread=", &tinrc.process_only_unread))
				break;

			if (match_boolean(buf, "prompt_followupto=", &tinrc.prompt_followupto))
				break;

			if (match_boolean(buf, "pgdn_goto_next=", &tinrc.pgdn_goto_next))
				break;

			break;

		case 'q':
			if (match_string(buf, "quote_chars=", tinrc.quote_chars, sizeof(tinrc.quote_chars))) {
				quote_dash_to_space(tinrc.quote_chars);
				break;
			}

			if (match_integer(buf, "quote_style=", &tinrc.quote_style, (QUOTE_COMPRESS|QUOTE_SIGS|QUOTE_EMPTY)))
				break;

#ifdef HAVE_COLOR
			if (match_string(buf, "quote_regex=", tinrc.quote_regex, sizeof(tinrc.quote_regex)))
				break;

			if (match_string(buf, "quote_regex2=", tinrc.quote_regex2, sizeof(tinrc.quote_regex2)))
				break;

			if (match_string(buf, "quote_regex3=", tinrc.quote_regex3, sizeof(tinrc.quote_regex3)))
				break;
#endif /* HAVE_COLOR */

			break;

		case 'r':
			if (match_integer(buf, "recent_time=", &tinrc.recent_time, 16383)) /* use INT_MAX? */
				break;

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
			if (match_boolean(buf, "render_bidi=", &tinrc.render_bidi))
				break;
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */

			if (match_integer(buf, "reread_active_file_secs=", &tinrc.reread_active_file_secs, 16383)) /* use INT_MAX? */
				break;

			break;

		case 's':
			if (match_integer(buf, "score_limit_kill=", &tinrc.score_limit_kill, 0))
				break;

			if (match_integer(buf, "score_limit_select=", &tinrc.score_limit_select, 0))
				break;

			if (match_integer(buf, "score_kill=", &tinrc.score_kill, 0)) {
				check_score_defaults();
				break;
			}

			if (match_integer(buf, "score_select=", &tinrc.score_select, 0)) {
				check_score_defaults();
				break;
			}

			if (match_integer(buf, "show_author=", &tinrc.show_author, SHOW_FROM_BOTH))
				break;

			if (match_boolean(buf, "show_description=", &tinrc.show_description)) {
				show_description = tinrc.show_description;
				break;
			}

			if (match_boolean(buf, "show_only_unread=", &tinrc.show_only_unread_arts))
				break;

			if (match_boolean(buf, "show_only_unread_groups=", &tinrc.show_only_unread_groups))
				break;

			if (match_boolean(buf, "sigdashes=", &tinrc.sigdashes))
				break;

			if (match_boolean(buf, "signature_repost=", &tinrc.signature_repost))
				break;

			if (match_string(buf, "spamtrap_warning_addresses=", tinrc.spamtrap_warning_addresses, sizeof(tinrc.spamtrap_warning_addresses)))
				break;

			if (match_boolean(buf, "start_editor_offset=", &tinrc.start_editor_offset))
				break;

			if (match_integer(buf, "sort_article_type=", &tinrc.sort_article_type, SORT_ARTICLES_BY_LINES_ASCEND))
				break;

			if (match_integer(buf, "sort_threads_type=", &tinrc.sort_threads_type, SORT_THREADS_BY_SCORE_ASCEND))
				break;

			if (match_integer(buf, "scroll_lines=", &tinrc.scroll_lines, 0))
				break;

			if (match_integer(buf, "show_info=", &tinrc.show_info, SHOW_INFO_BOTH))
				break;

			if (match_boolean(buf, "show_signatures=", &tinrc.show_signatures))
				break;

			if (match_string(buf, "slashes_regex=", tinrc.slashes_regex, sizeof(tinrc.slashes_regex)))
				break;

			if (match_string(buf, "stars_regex=", tinrc.stars_regex, sizeof(tinrc.stars_regex)))
				break;

			if (match_string(buf, "strokes_regex=", tinrc.strokes_regex, sizeof(tinrc.strokes_regex)))
				break;

			if (match_boolean(buf, "strip_blanks=", &tinrc.strip_blanks))
				break;

			if (match_integer(buf, "strip_bogus=", &tinrc.strip_bogus, BOGUS_SHOW))
				break;

			if (match_boolean(buf, "strip_newsrc=", &tinrc.strip_newsrc))
				break;

			/* Regexp used to strip "Re: "s and similar */
			if (match_string(buf, "strip_re_regex=", tinrc.strip_re_regex, sizeof(tinrc.strip_re_regex)))
				break;

			if (match_string(buf, "strip_was_regex=", tinrc.strip_was_regex, sizeof(tinrc.strip_was_regex)))
				break;

			if (match_boolean(buf, "space_goto_next_unread=", &tinrc.space_goto_next_unread))
				break;

			break;

		case 't':
			if (match_integer(buf, "thread_articles=", &tinrc.thread_articles, THREAD_MAX))
				break;

			if (match_integer(buf, "thread_score=", &tinrc.thread_score, THREAD_SCORE_WEIGHT))
				break;

			if (match_boolean(buf, "tab_goto_next_unread=", &tinrc.tab_goto_next_unread))
				break;

			if (match_boolean(buf, "tex2iso_conv=", &tinrc.tex2iso_conv))
				break;

			if (match_boolean(buf, "thread_catchup_on_exit=", &tinrc.thread_catchup_on_exit))
				break;

#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
			if (match_boolean(buf, "translit=", &tinrc.translit))
				break;
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */

			break;

		case 'u':
			if (match_string(buf, "underscores_regex=", tinrc.underscores_regex, sizeof(tinrc.underscores_regex)))
				break;

			if (match_boolean(buf, "unlink_article=", &tinrc.unlink_article))
				break;

			if (match_string(buf, "url_handler=", tinrc.url_handler, sizeof(tinrc.url_handler)))
				break;

			if (match_boolean(buf, "url_highlight=", &tinrc.url_highlight))
				break;

			if (match_boolean(buf, "use_mouse=", &tinrc.use_mouse))
				break;

#ifdef HAVE_KEYPAD
			if (match_boolean(buf, "use_keypad=", &tinrc.use_keypad))
				break;
#endif /* HAVE_KEYPAD */

#ifdef HAVE_COLOR
			if (match_boolean(buf, "use_color=", &tinrc.use_color)) {
				use_color = tinrc.use_color;
				break;
			}
#endif /* HAVE_COLOR */

#ifdef XFACE_ABLE
			if (match_boolean(buf, "use_slrnface=", &tinrc.use_slrnface))
				break;
#endif /* XFACE_ABLE */

			break;

		case 'w':
			if (match_integer(buf, "wildcard=", &tinrc.wildcard, 2))
				break;

			if (match_boolean(buf, "word_highlight=", &tinrc.word_highlight)) {
				word_highlight = tinrc.word_highlight;
				break;
			}

			if (match_integer(buf, "wrap_column=", &tinrc.wrap_column, 0))
				break;

			if (match_boolean(buf, "wrap_on_next_unread=", &tinrc.wrap_on_next_unread))
				break;

			if (match_integer(buf, "word_h_display_marks=", &tinrc.word_h_display_marks, MAX_MARK))
				break;

			break;

		case 'x':
			if (match_string(buf, "xpost_quote_format=", tinrc.xpost_quote_format, sizeof(tinrc.xpost_quote_format)))
				break;

			break;

		default:
			break;
		}
	}
	fclose(fp);

	/*
	 * sort out conflicting settings
	 */

	/* nobody likes to navigate blind */
	if (!(tinrc.draw_arrow || tinrc.inverse_okay))
		tinrc.draw_arrow = TRUE;

	/*
	 * determine local charset, or in case of NO_LOCALE use
	 * mm{_network}_charset
	 */
	{
#ifndef NO_LOCALE
		const char *p;

		if ((p = tin_nl_langinfo(CODESET)) != NULL) {
			if (!strcmp(p, "ANSI_X3.4-1968"))
				strcpy(tinrc.mm_local_charset, "US-ASCII");
			else
				strcpy(tinrc.mm_local_charset, p);
		} else
#endif /* !NO_LOCALE */
			if (!*tinrc.mm_local_charset)
#ifndef CHARSET_CONVERSION
				strcpy(tinrc.mm_local_charset, tinrc.mm_charset);
#else
				strcpy(tinrc.mm_local_charset, txt_mime_charsets[tinrc.mm_network_charset]);
#endif /* !CHARSET_CONVERSION */
	}
	return TRUE;
}


/*
 * write config defaults to file
 */
void
write_config_file(
	char *file)
{
	FILE *fp;
	char *file_tmp;
	int i;

	if ((no_write || post_article_and_exit || post_postponed_and_exit) && file_size(file) != -1L)
		return;

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(file);

	if ((fp = fopen(file_tmp, "w")) == NULL) {
		error_message(_(txt_filesystem_full_backup), CONFIG_FILE);
		free(file_tmp);
		return;
	}

	if (!cmd_line)
		wait_message(0, _(txt_saving));

	if (!*tinrc.editor_format)
		strcpy(tinrc.editor_format, TIN_EDITOR_FMT_ON);

	fprintf(fp, txt_tinrc_header, PRODUCT, TINRC_VERSION, tin_progname, VERSION, RELEASEDATE, RELEASENAME);

	fprintf(fp, _(txt_savedir.tinrc));
	fprintf(fp, "default_savedir=%s\n\n", tinrc.savedir);

	fprintf(fp, _(txt_auto_save.tinrc));
	fprintf(fp, "auto_save=%s\n\n", print_boolean(tinrc.auto_save));

	fprintf(fp, _(txt_mark_saved_read.tinrc));
	fprintf(fp, "mark_saved_read=%s\n\n", print_boolean(tinrc.mark_saved_read));

	fprintf(fp, _(txt_post_process.tinrc));
	fprintf(fp, "post_process_type=%d\n\n", tinrc.post_process);

	fprintf(fp, _(txt_post_process_view.tinrc));
	fprintf(fp, "post_process_view=%s\n\n", print_boolean(tinrc.post_process_view));

	fprintf(fp, _(txt_process_only_unread.tinrc));
	fprintf(fp, "process_only_unread=%s\n\n", print_boolean(tinrc.process_only_unread));

	fprintf(fp, _(txt_prompt_followupto.tinrc));
	fprintf(fp, "prompt_followupto=%s\n\n", print_boolean(tinrc.prompt_followupto));

	fprintf(fp, _(txt_confirm_choice.tinrc));
	fprintf(fp, "confirm_choice=%s\n\n", txt_confirm_choices[tinrc.confirm_choice]);

	fprintf(fp, _(txt_mark_ignore_tags.tinrc));
	fprintf(fp, "mark_ignore_tags=%s\n\n", print_boolean(tinrc.mark_ignore_tags));

	fprintf(fp, _(txt_auto_reconnect.tinrc));
	fprintf(fp, "auto_reconnect=%s\n\n", print_boolean(tinrc.auto_reconnect));

	fprintf(fp, _(txt_draw_arrow.tinrc));
	fprintf(fp, "draw_arrow=%s\n\n", print_boolean(tinrc.draw_arrow));

	fprintf(fp, _(txt_inverse_okay.tinrc));
	fprintf(fp, "inverse_okay=%s\n\n", print_boolean(tinrc.inverse_okay));

	fprintf(fp, _(txt_pos_first_unread.tinrc));
	fprintf(fp, "pos_first_unread=%s\n\n", print_boolean(tinrc.pos_first_unread));

	fprintf(fp, _(txt_show_only_unread_arts.tinrc));
	fprintf(fp, "show_only_unread=%s\n\n", print_boolean(tinrc.show_only_unread_arts));

	fprintf(fp, _(txt_show_only_unread_groups.tinrc));
	fprintf(fp, "show_only_unread_groups=%s\n\n", print_boolean(tinrc.show_only_unread_groups));

	fprintf(fp, _(txt_kill_level.tinrc));
	fprintf(fp, "kill_level=%d\n\n", tinrc.kill_level);

	fprintf(fp, _(txt_tab_goto_next_unread.tinrc));
	fprintf(fp, "tab_goto_next_unread=%s\n\n", print_boolean(tinrc.tab_goto_next_unread));

	fprintf(fp, _(txt_space_goto_next_unread.tinrc));
	fprintf(fp, "space_goto_next_unread=%s\n\n", print_boolean(tinrc.space_goto_next_unread));

	fprintf(fp, _(txt_pgdn_goto_next.tinrc));
	fprintf(fp, "pgdn_goto_next=%s\n\n", print_boolean(tinrc.pgdn_goto_next));

	fprintf(fp, _(txt_scroll_lines.tinrc));
	fprintf(fp, "scroll_lines=%d\n\n", tinrc.scroll_lines);

	fprintf(fp, _(txt_catchup_read_groups.tinrc));
	fprintf(fp, "catchup_read_groups=%s\n\n", print_boolean(tinrc.catchup_read_groups));

	fprintf(fp, _(txt_group_catchup_on_exit.tinrc));
	fprintf(fp, "group_catchup_on_exit=%s\n", print_boolean(tinrc.group_catchup_on_exit));
	fprintf(fp, "thread_catchup_on_exit=%s\n\n", print_boolean(tinrc.thread_catchup_on_exit));

	fprintf(fp, _(txt_thread_articles.tinrc));
	fprintf(fp, "thread_articles=%d\n\n", tinrc.thread_articles);

	fprintf(fp, _(txt_show_description.tinrc));
	fprintf(fp, "show_description=%s\n\n", print_boolean(tinrc.show_description));

	fprintf(fp, _(txt_show_author.tinrc));
	fprintf(fp, "show_author=%d\n\n", tinrc.show_author);

	fprintf(fp, _(txt_news_headers_to_display.tinrc));
	fprintf(fp, "news_headers_to_display=");
	for (i = 0; i < num_headers_to_display; i++)
		fprintf(fp, "%s ", news_headers_to_display_array[i]);
	fprintf(fp, "\n\n");

	fprintf(fp, _(txt_news_headers_to_not_display.tinrc));
	fprintf(fp, "news_headers_to_not_display=");
	for (i = 0; i < num_headers_to_not_display; i++)
		fprintf(fp, "%s ", news_headers_to_not_display_array[i]);
	fprintf(fp, "\n\n");

	fprintf(fp, _(txt_tinrc_info_in_last_line));
	fprintf(fp, "info_in_last_line=%s\n\n", print_boolean(tinrc.info_in_last_line));

	fprintf(fp, _(txt_sort_article_type.tinrc));
	fprintf(fp, "sort_article_type=%d\n\n", tinrc.sort_article_type);

	fprintf(fp, _(txt_sort_threads_type.tinrc));
	fprintf(fp, "sort_threads_type=%d\n\n", tinrc.sort_threads_type);

	fprintf(fp, _(txt_maildir.tinrc));
	fprintf(fp, "default_maildir=%s\n\n", tinrc.maildir);

	fprintf(fp, _(txt_mailbox_format.tinrc));
	fprintf(fp, "mailbox_format=%s\n\n", txt_mailbox_formats[tinrc.mailbox_format]);

#ifndef DISABLE_PRINTING
	fprintf(fp, _(txt_print_header.tinrc));
	fprintf(fp, "print_header=%s\n\n", print_boolean(tinrc.print_header));

	fprintf(fp, _(txt_printer.tinrc));
	fprintf(fp, "default_printer=%s\n\n", tinrc.printer);
#endif /* !DISABLE_PRINTING */

	fprintf(fp, _(txt_batch_save.tinrc));
	fprintf(fp, "batch_save=%s\n\n", print_boolean(tinrc.batch_save));

	fprintf(fp, _(txt_start_editor_offset.tinrc));
	fprintf(fp, "start_editor_offset=%s\n\n", print_boolean(tinrc.start_editor_offset));

	fprintf(fp, _(txt_editor_format.tinrc));
	fprintf(fp, "default_editor_format=%s\n\n", tinrc.editor_format);

	fprintf(fp, _(txt_mailer_format.tinrc));
	fprintf(fp, "default_mailer_format=%s\n\n", tinrc.mailer_format);

	fprintf(fp, _(txt_interactive_mailer.tinrc));
	fprintf(fp, "interactive_mailer=%d\n\n", tinrc.interactive_mailer);

	fprintf(fp, _(txt_show_info.tinrc));
	fprintf(fp, "show_info=%d\n\n", tinrc.show_info);

	fprintf(fp, _(txt_thread_score.tinrc));
	fprintf(fp, "thread_score=%d\n\n", tinrc.thread_score);

	fprintf(fp, _(txt_unlink_article.tinrc));
	fprintf(fp, "unlink_article=%s\n\n", print_boolean(tinrc.unlink_article));

	fprintf(fp, _(txt_keep_dead_articles.tinrc));
	fprintf(fp, "keep_dead_articles=%s\n\n", print_boolean(tinrc.keep_dead_articles));

	fprintf(fp, _(txt_posted_articles_file.tinrc));
	fprintf(fp, "posted_articles_file=%s\n\n", tinrc.posted_articles_file);

	fprintf(fp, _(txt_add_posted_to_filter.tinrc));
	fprintf(fp, "add_posted_to_filter=%s\n\n", print_boolean(tinrc.add_posted_to_filter));

	fprintf(fp, _(txt_sigfile.tinrc));
	fprintf(fp, "default_sigfile=%s\n\n", tinrc.sigfile);

	fprintf(fp, _(txt_sigdashes.tinrc));
	fprintf(fp, "sigdashes=%s\n\n", print_boolean(tinrc.sigdashes));

	fprintf(fp, _(txt_signature_repost.tinrc));
	fprintf(fp, "signature_repost=%s\n\n", print_boolean(tinrc.signature_repost));

	fprintf(fp, _(txt_spamtrap_warning_addresses.tinrc));
	fprintf(fp, "spamtrap_warning_addresses=%s\n\n", tinrc.spamtrap_warning_addresses);

	fprintf(fp, _(txt_url_handler.tinrc));
	fprintf(fp, "url_handler=%s\n\n", tinrc.url_handler);

	fprintf(fp, _(txt_advertising.tinrc));
	fprintf(fp, "advertising=%s\n\n", print_boolean(tinrc.advertising));

	fprintf(fp, _(txt_reread_active_file_secs.tinrc));
	fprintf(fp, "reread_active_file_secs=%d\n\n", tinrc.reread_active_file_secs);

	fprintf(fp, _(txt_quote_chars.tinrc));
	fprintf(fp, "quote_chars=%s\n\n", quote_space_to_dash(tinrc.quote_chars));

	fprintf(fp, _(txt_quote_style.tinrc));
	fprintf(fp, "quote_style=%d\n\n", tinrc.quote_style);

#ifdef HAVE_COLOR
	fprintf(fp, _(txt_quote_regex.tinrc));
	fprintf(fp, "quote_regex=%s\n\n", tinrc.quote_regex);
	fprintf(fp, _(txt_quote_regex2.tinrc));
	fprintf(fp, "quote_regex2=%s\n\n", tinrc.quote_regex2);
	fprintf(fp, _(txt_quote_regex3.tinrc));
	fprintf(fp, "quote_regex3=%s\n\n", tinrc.quote_regex3);
#endif /* HAVE_COLOR */

	fprintf(fp, _(txt_slashes_regex.tinrc));
	fprintf(fp, "slashes_regex=%s\n\n", tinrc.slashes_regex);
	fprintf(fp, _(txt_stars_regex.tinrc));
	fprintf(fp, "stars_regex=%s\n\n", tinrc.stars_regex);
	fprintf(fp, _(txt_strokes_regex.tinrc));
	fprintf(fp, "strokes_regex=%s\n\n", tinrc.strokes_regex);
	fprintf(fp, _(txt_underscores_regex.tinrc));
	fprintf(fp, "underscores_regex=%s\n\n", tinrc.underscores_regex);

	fprintf(fp, _(txt_strip_re_regex.tinrc));
	fprintf(fp, "strip_re_regex=%s\n\n", tinrc.strip_re_regex);
	fprintf(fp, _(txt_strip_was_regex.tinrc));
	fprintf(fp, "strip_was_regex=%s\n\n", tinrc.strip_was_regex);

	fprintf(fp, _(txt_show_signatures.tinrc));
	fprintf(fp, "show_signatures=%s\n\n", print_boolean(tinrc.show_signatures));

	fprintf(fp, _(txt_tex2iso_conv.tinrc));
	fprintf(fp, "tex2iso_conv=%s\n\n", print_boolean(tinrc.tex2iso_conv));

	fprintf(fp, _(txt_hide_uue.tinrc));
	fprintf(fp, "hide_uue=%d\n\n", tinrc.hide_uue);

	fprintf(fp, _(txt_news_quote_format.tinrc));
	fprintf(fp, "news_quote_format=%s\n", tinrc.news_quote_format);
	fprintf(fp, "mail_quote_format=%s\n", tinrc.mail_quote_format);
	fprintf(fp, "xpost_quote_format=%s\n\n", tinrc.xpost_quote_format);

	fprintf(fp, _(txt_auto_cc.tinrc));
	fprintf(fp, "auto_cc=%s\n\n", print_boolean(tinrc.auto_cc));

	fprintf(fp, _(txt_auto_bcc.tinrc));
	fprintf(fp, "auto_bcc=%s\n\n", print_boolean(tinrc.auto_bcc));

	fprintf(fp, _(txt_art_marked_deleted.tinrc));
	fprintf(fp, "art_marked_deleted=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_deleted));

	fprintf(fp, _(txt_art_marked_inrange.tinrc));
	fprintf(fp, "art_marked_inrange=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_inrange));

	fprintf(fp, _(txt_art_marked_return.tinrc));
	fprintf(fp, "art_marked_return=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_return));

	fprintf(fp, _(txt_art_marked_selected.tinrc));
	fprintf(fp, "art_marked_selected=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_selected));

	fprintf(fp, _(txt_art_marked_recent.tinrc));
	fprintf(fp, "art_marked_recent=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_recent));

	fprintf(fp, _(txt_art_marked_unread.tinrc));
	fprintf(fp, "art_marked_unread=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_unread));

	fprintf(fp, _(txt_art_marked_read.tinrc));
	fprintf(fp, "art_marked_read=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_read));

	fprintf(fp, _(txt_art_marked_killed.tinrc));
	fprintf(fp, "art_marked_killed=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_killed));

	fprintf(fp, _(txt_art_marked_read_selected.tinrc));
	fprintf(fp, "art_marked_read_selected=%c\n\n", SPACE_TO_DASH(tinrc.art_marked_read_selected));

	fprintf(fp, _(txt_force_screen_redraw.tinrc));
	fprintf(fp, "force_screen_redraw=%s\n\n", print_boolean(tinrc.force_screen_redraw));

	fprintf(fp, _(txt_inews_prog.tinrc));
	fprintf(fp, "inews_prog=%s\n\n", tinrc.inews_prog);

	fprintf(fp, _(txt_auto_list_thread.tinrc));
	fprintf(fp, "auto_list_thread=%s\n\n", print_boolean(tinrc.auto_list_thread));

	fprintf(fp, _(txt_wrap_on_next_unread.tinrc));
	fprintf(fp, "wrap_on_next_unread=%s\n\n", print_boolean(tinrc.wrap_on_next_unread));

	fprintf(fp, _(txt_use_mouse.tinrc));
	fprintf(fp, "use_mouse=%s\n\n", print_boolean(tinrc.use_mouse));

	fprintf(fp, _(txt_strip_blanks.tinrc));
	fprintf(fp, "strip_blanks=%s\n\n", print_boolean(tinrc.strip_blanks));

	fprintf(fp, _(txt_groupname_max_length.tinrc));
	fprintf(fp, "groupname_max_length=%d\n\n", tinrc.groupname_max_length);

	fprintf(fp, _(txt_beginner_level.tinrc));
	fprintf(fp, "beginner_level=%s\n\n", print_boolean(tinrc.beginner_level));

	fprintf(fp, _(txt_filter_days.tinrc));
	fprintf(fp, "default_filter_days=%d\n\n", tinrc.filter_days);

	fprintf(fp, _(txt_cache_overview_files.tinrc));
	fprintf(fp, "cache_overview_files=%s\n\n", print_boolean(tinrc.cache_overview_files));

	fprintf(fp, _(txt_getart_limit.tinrc));
	fprintf(fp, "getart_limit=%d\n\n", tinrc.getart_limit);

	fprintf(fp, _(txt_recent_time.tinrc));
	fprintf(fp, "recent_time=%d\n\n", tinrc.recent_time);

	fprintf(fp, _(txt_score_limit_kill.tinrc));
	fprintf(fp, "score_limit_kill=%d\n\n", tinrc.score_limit_kill);

	fprintf(fp, _(txt_score_kill.tinrc));
	fprintf(fp, "score_kill=%d\n\n", tinrc.score_kill);

	fprintf(fp, _(txt_score_limit_select.tinrc));
	fprintf(fp, "score_limit_select=%d\n\n", tinrc.score_limit_select);

	fprintf(fp, _(txt_score_select.tinrc));
	fprintf(fp, "score_select=%d\n\n", tinrc.score_select);

#ifdef HAVE_COLOR
	fprintf(fp, _(txt_use_color.tinrc));
	fprintf(fp, "use_color=%s\n\n", print_boolean(tinrc.use_color));

	fprintf(fp, _(txt_tinrc_colors));

	fprintf(fp, _(txt_col_normal.tinrc));
	fprintf(fp, "col_normal=%d\n\n", tinrc.col_normal);

	fprintf(fp, _(txt_col_back.tinrc));
	fprintf(fp, "col_back=%d\n\n", tinrc.col_back);

	fprintf(fp, _(txt_col_invers_bg.tinrc));
	fprintf(fp, "col_invers_bg=%d\n\n", tinrc.col_invers_bg);

	fprintf(fp, _(txt_col_invers_fg.tinrc));
	fprintf(fp, "col_invers_fg=%d\n\n", tinrc.col_invers_fg);

	fprintf(fp, _(txt_col_text.tinrc));
	fprintf(fp, "col_text=%d\n\n", tinrc.col_text);

	fprintf(fp, _(txt_col_minihelp.tinrc));
	fprintf(fp, "col_minihelp=%d\n\n", tinrc.col_minihelp);

	fprintf(fp, _(txt_col_help.tinrc));
	fprintf(fp, "col_help=%d\n\n", tinrc.col_help);

	fprintf(fp, _(txt_col_message.tinrc));
	fprintf(fp, "col_message=%d\n\n", tinrc.col_message);

	fprintf(fp, _(txt_col_quote.tinrc));
	fprintf(fp, "col_quote=%d\n\n", tinrc.col_quote);

	fprintf(fp, _(txt_col_quote2.tinrc));
	fprintf(fp, "col_quote2=%d\n\n", tinrc.col_quote2);

	fprintf(fp, _(txt_col_quote3.tinrc));
	fprintf(fp, "col_quote3=%d\n\n", tinrc.col_quote3);

	fprintf(fp, _(txt_col_head.tinrc));
	fprintf(fp, "col_head=%d\n\n", tinrc.col_head);

	fprintf(fp, _(txt_col_newsheaders.tinrc));
	fprintf(fp, "col_newsheaders=%d\n\n", tinrc.col_newsheaders);

	fprintf(fp, _(txt_col_subject.tinrc));
	fprintf(fp, "col_subject=%d\n\n", tinrc.col_subject);

	fprintf(fp, _(txt_col_response.tinrc));
	fprintf(fp, "col_response=%d\n\n", tinrc.col_response);

	fprintf(fp, _(txt_col_from.tinrc));
	fprintf(fp, "col_from=%d\n\n", tinrc.col_from);

	fprintf(fp, _(txt_col_title.tinrc));
	fprintf(fp, "col_title=%d\n\n", tinrc.col_title);

	fprintf(fp, _(txt_col_signature.tinrc));
	fprintf(fp, "col_signature=%d\n\n", tinrc.col_signature);

	fprintf(fp, _(txt_col_urls.tinrc));
	fprintf(fp, "col_urls=%d\n\n", tinrc.col_urls);
#endif /* HAVE_COLOR */

#ifdef XFACE_ABLE
	fprintf(fp, _(txt_use_slrnface.tinrc));
	fprintf(fp, "use_slrnface=%s\n\n", print_boolean(tinrc.use_slrnface));
#endif /*XFACE_ABLE */

	fprintf(fp, _(txt_url_highlight.tinrc));
	fprintf(fp, "url_highlight=%s\n\n", print_boolean(tinrc.url_highlight));

	fprintf(fp, _(txt_word_highlight.tinrc));
	fprintf(fp, "word_highlight=%s\n\n", print_boolean(tinrc.word_highlight));

	fprintf(fp, _(txt_wrap_column.tinrc));
	fprintf(fp, "wrap_column=%d\n\n", tinrc.wrap_column);

	fprintf(fp, _(txt_word_h_display_marks.tinrc));
	fprintf(fp, "word_h_display_marks=%d\n\n", tinrc.word_h_display_marks);

#ifdef HAVE_COLOR
	fprintf(fp, _(txt_col_markstar.tinrc));
	fprintf(fp, "col_markstar=%d\n", tinrc.col_markstar);
	fprintf(fp, "col_markdash=%d\n", tinrc.col_markdash);
	fprintf(fp, "col_markslash=%d\n", tinrc.col_markslash);
	fprintf(fp, "col_markstroke=%d\n\n", tinrc.col_markstroke);
#endif /* HAVE_COLOR */

	fprintf(fp, _(txt_mono_markstar.tinrc));
	fprintf(fp, "mono_markstar=%d\n", tinrc.mono_markstar);
	fprintf(fp, "mono_markdash=%d\n", tinrc.mono_markdash);
	fprintf(fp, "mono_markslash=%d\n", tinrc.mono_markslash);
	fprintf(fp, "mono_markstroke=%d\n\n", tinrc.mono_markstroke);

	fprintf(fp, _(txt_mail_address.tinrc));
	fprintf(fp, "mail_address=%s\n\n", tinrc.mail_address);

#ifndef CHARSET_CONVERSION
	fprintf(fp, _(txt_mm_charset.tinrc));
	fprintf(fp, "mm_charset=%s\n\n", tinrc.mm_charset);
#else
	fprintf(fp, _(txt_mm_network_charset.tinrc));
	fprintf(fp, "mm_network_charset=%s\n\n", txt_mime_charsets[tinrc.mm_network_charset]);

#	ifdef HAVE_ICONV_OPEN_TRANSLIT
	fprintf(fp, _(txt_translit.tinrc));
	fprintf(fp, "translit=%s\n\n", print_boolean(tinrc.translit));
#	endif /* HAVE_ICONV_OPEN_TRANSLIT */
#endif /* !CHARSET_CONVERSION */

	fprintf(fp, _(txt_post_mime_encoding.tinrc));
	fprintf(fp, "post_mime_encoding=%s\n", txt_mime_encodings[tinrc.post_mime_encoding]);
	fprintf(fp, "mail_mime_encoding=%s\n\n", txt_mime_encodings[tinrc.mail_mime_encoding]);

	fprintf(fp, _(txt_post_8bit_header.tinrc));
	fprintf(fp, "post_8bit_header=%s\n\n", print_boolean(tinrc.post_8bit_header));

	fprintf(fp, _(txt_mail_8bit_header.tinrc));
	fprintf(fp, "mail_8bit_header=%s\n\n", print_boolean(tinrc.mail_8bit_header));

	fprintf(fp, _(txt_metamail_prog.tinrc));
	fprintf(fp, "metamail_prog=%s\n\n", tinrc.metamail_prog);

	fprintf(fp, _(txt_ask_for_metamail.tinrc));
	fprintf(fp, "ask_for_metamail=%s\n\n", print_boolean(tinrc.ask_for_metamail));

#ifdef HAVE_KEYPAD
	fprintf(fp, _(txt_use_keypad.tinrc));
	fprintf(fp, "use_keypad=%s\n\n", print_boolean(tinrc.use_keypad));
#endif /* HAVE_KEYPAD */

	fprintf(fp, _(txt_alternative_handling.tinrc));
	fprintf(fp, "alternative_handling=%s\n\n", print_boolean(tinrc.alternative_handling));

	fprintf(fp, _(txt_strip_newsrc.tinrc));
	fprintf(fp, "strip_newsrc=%s\n\n", print_boolean(tinrc.strip_newsrc));

	fprintf(fp, _(txt_strip_bogus.tinrc));
	fprintf(fp, "strip_bogus=%d\n\n", tinrc.strip_bogus);

	fprintf(fp, _(txt_date_format.tinrc));
	fprintf(fp, "date_format=%s\n\n", tinrc.date_format);

	fprintf(fp, _(txt_wildcard.tinrc));
	fprintf(fp, "wildcard=%d\n\n", tinrc.wildcard);

#ifdef HAVE_UNICODE_NORMALIZATION
	fprintf(fp, _(txt_normalization_form.tinrc));
	fprintf(fp, "normalization_form=%d\n\n", tinrc.normalization_form);
#endif /* HAVE_UNICODE_NORMALIZATION */

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
	fprintf(fp, _(txt_render_bidi.tinrc));
	fprintf(fp, "render_bidi=%s\n\n", print_boolean(tinrc.render_bidi));
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */

	fprintf(fp, _(txt_tinrc_filter));
	fprintf(fp, "default_filter_kill_header=%d\n", tinrc.default_filter_kill_header);
	fprintf(fp, "default_filter_kill_global=%s\n", print_boolean(tinrc.default_filter_kill_global));
	/* ON=false, OFF=true */
	fprintf(fp, "default_filter_kill_case=%s\n", print_boolean(!tinrc.default_filter_kill_case));
	fprintf(fp, "default_filter_kill_expire=%s\n", print_boolean(tinrc.default_filter_kill_expire));
	fprintf(fp, "default_filter_select_header=%d\n", tinrc.default_filter_select_header);
	fprintf(fp, "default_filter_select_global=%s\n", print_boolean(tinrc.default_filter_select_global));
	/* ON=false, OFF=true */
	fprintf(fp, "default_filter_select_case=%s\n", print_boolean(!tinrc.default_filter_select_case));
	fprintf(fp, "default_filter_select_expire=%s\n\n", print_boolean(tinrc.default_filter_select_expire));

	fprintf(fp, _(txt_tinrc_defaults));
	fprintf(fp, "default_save_mode=%c\n", tinrc.default_save_mode);
	fprintf(fp, "default_author_search=%s\n", tinrc.default_search_author);
	fprintf(fp, "default_goto_group=%s\n", tinrc.default_goto_group);
	fprintf(fp, "default_config_search=%s\n", tinrc.default_search_config);
	fprintf(fp, "default_group_search=%s\n", tinrc.default_search_group);
	fprintf(fp, "default_subject_search=%s\n", tinrc.default_search_subject);
	fprintf(fp, "default_art_search=%s\n", tinrc.default_search_art);
	fprintf(fp, "default_repost_group=%s\n", tinrc.default_repost_group);
	fprintf(fp, "default_mail_address=%s\n", tinrc.default_mail_address);
	fprintf(fp, "default_move_group=%d\n", tinrc.default_move_group);
#ifndef DONT_HAVE_PIPING
	fprintf(fp, "default_pipe_command=%s\n", tinrc.default_pipe_command);
#endif /* !DONT_HAVE_PIPING */
	fprintf(fp, "default_post_newsgroups=%s\n", tinrc.default_post_newsgroups);
	fprintf(fp, "default_post_subject=%s\n", tinrc.default_post_subject);
	fprintf(fp, "default_range_group=%s\n", tinrc.default_range_group);
	fprintf(fp, "default_range_select=%s\n", tinrc.default_range_select);
	fprintf(fp, "default_range_thread=%s\n", tinrc.default_range_thread);
	fprintf(fp, "default_pattern=%s\n", tinrc.default_pattern);
	fprintf(fp, "default_save_file=%s\n", tinrc.default_save_file);
	fprintf(fp, "default_select_pattern=%s\n", tinrc.default_select_pattern);
	fprintf(fp, "default_shell_command=%s\n\n", tinrc.default_shell_command);

	fprintf(fp, _(txt_tinrc_newnews));
	{
		char timestring[30];
		int j = find_newnews_index(nntp_server);

		/*
		 * Newnews timestamps in tinrc are bogus as of tin 1.5.19 because they
		 * are now stored in a separate file to prevent overwriting them from
		 * another instance running concurrently. Except for the current server,
		 * however, we must remember them because otherwise we would lose them
		 * after the first start of a tin 1.5.19 (or later) version.
		 */
		for (i = 0; i < num_newnews; i++) {
			if (i == j)
				continue;
			if (my_strftime(timestring, sizeof(timestring) - 1, "%Y-%m-%d %H:%M:%S UTC", gmtime(&(newnews[i].time))))
				fprintf(fp, "newnews=%s %lu (%s)\n", newnews[i].host, (unsigned long int) newnews[i].time, timestring);
		}
	}

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR)); /* rename_file() preserves mode */

	if (ferror(fp) || fclose(fp))
		error_message(_(txt_filesystem_full), CONFIG_FILE);
	else
		rename_file(file_tmp, file);

	free(file_tmp);
	write_server_config();
}


static int first_option_on_screen;
static int actual_top_option = 0;

#define option_lines_per_page (cLINES - INDEX_TOP - 3)
#define TopOfPage(option) option_lines_per_page \
			* ((option) / option_lines_per_page)
#define OptionInPage(option)	((option) - first_option_on_screen)
#define OptionIndex(option)	(OptionInPage(option) % option_lines_per_page)


int
option_row(
	int option)
{
	return (INDEX_TOP + OptionIndex(option));
}


static int
get_option_num(
	int act_option)
{
	int result = 0;

	if (option_table[act_option].var_type != OPT_TITLE) {
		while (act_option >= 0) {
			if (option_table[act_option--].var_type != OPT_TITLE)
				++result;
		}
	}
	return result;
}


static int
set_option_num(
	int option)
{
	int result = 0;

	while (option >= 0 && result < LAST_OPT) {
		while (result < LAST_OPT && option_table[result].var_type == OPT_TITLE)
			result++;
		if (result < LAST_OPT) {
			if (--option < 0)
				break;
			++result;
		}
	}
	return result;
}


char *
fmt_option_prompt(
	char *dst,
	size_t len,
	t_bool editing,
	int option)
{
	char *buf;
	int num = get_option_num(option);
	size_t option_width = MAX(35, cCOLS / 2 - 9);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wbuf, *wbuf2;

	/* convert the option text to wchar_t */
	wbuf = char2wchar_t(_(option_table[option].txt->opt));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (num) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (wbuf != NULL) {
			wbuf2 = my_malloc(sizeof(wchar_t) * (option_width + 1));
			wstrunc(wbuf, wbuf2, option_width + 1, option_width);

			if ((buf = wchar_t2char(wbuf2)) == NULL) {
				/* conversion failed, truncate original string */
				buf = my_malloc(option_width + 1);
				strunc(_(option_table[option].txt->opt), buf, option_width + 1, option_width);
				snprintf(dst, len, "%s %3d. %-*.*s: ", editing ? "->" : "  ", num, option_width, option_width, buf);
			} else {
				snprintf(dst, len, "%s %3d. %-*.*s: ", editing ? "->" : "  ", num,
					strlen(buf) + option_width - wcswidth(wbuf2, option_width + 1),
					strlen(buf) + option_width - wcswidth(wbuf2, option_width + 1), buf);
			}
			free(wbuf2);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		{
			/* truncate original string */
			buf = my_malloc(option_width + 1);
			strunc(_(option_table[option].txt->opt), buf, option_width + 1, option_width);
			snprintf(dst, len, "%s %3d. %-*.*s: ", editing ? "->" : "  ", num, option_width, option_width, buf);
		}
	} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (wbuf != NULL) {
			wbuf2 = my_malloc(sizeof(wchar_t) * (cCOLS - 3 + 1));
			wstrunc(wbuf, wbuf2, cCOLS - 3 + 1, cCOLS - 3);
			if ((buf = wchar_t2char(wbuf2)) == NULL) {
				/* conversion failed, truncate original string */
				buf = my_malloc(cCOLS - 3 + 1);
				strunc(_(option_table[option].txt->opt), buf, cCOLS - 3 + 1, cCOLS - 3);
			}
			free(wbuf2);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		{
			/* truncate original string */
			buf = my_malloc(cCOLS - 3 + 1);
			strunc(_(option_table[option].txt->opt), buf, cCOLS - 3 + 1, cCOLS - 3);
		}
		snprintf(dst, len, "  %s", buf);
	}

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	FreeIfNeeded(wbuf);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	FreeIfNeeded(buf);
	return dst;
}


static void
print_any_option(
	int act_option)
{
	constext **list;
	char temp[LEN], *ptr, *ptr2;
	int row = option_row(act_option);
	size_t len = sizeof(temp) - 1;

	MoveCursor(row, 0);

	ptr = fmt_option_prompt(temp, len, FALSE, act_option);
	ptr += strlen(temp);
	len -= strlen(temp);

	switch (option_table[act_option].var_type) {
		case OPT_ON_OFF:
			/* tailing space to overwrite any left over F from OFF */
			snprintf(ptr, len, "%s ", print_boolean(*OPT_ON_OFF_list[option_table[act_option].var_index]));
			break;

		case OPT_LIST:
			list = option_table[act_option].opt_list;
			ptr2 = my_strdup(list[*(option_table[act_option].variable) + ((strcasecmp(_(list[0]), _(txt_default)) == 0) ? 1 : 0)]);
			snprintf(ptr, len, "%s", _(ptr2));
			free(ptr2);
			break;

		case OPT_STRING:
			snprintf(ptr, len, "%s", OPT_STRING_list[option_table[act_option].var_index]);
			break;

		case OPT_NUM:
			snprintf(ptr, len, "%d", *(option_table[act_option].variable));
			break;

		case OPT_CHAR:
			snprintf(ptr, len, "%c", *OPT_CHAR_list[option_table[act_option].var_index]);
			break;

		default:
			break;
	}
#ifdef USE_CURSES
	my_printf("%.*s", cCOLS - 1, temp);
	{
		int y, x;

		getyx(stdscr, y, x);
		if (x < cCOLS)
			clrtoeol();
	}
#else
	my_printf("%.*s", cCOLS - 1, temp);
	/* draw_arrow_mark() will read this back for repainting */
	strncpy(screen[row - INDEX_TOP].col, temp, cCOLS);
#endif /* USE_CURSES */
}


static void
print_option(
	enum option_enum the_option)
{
	print_any_option((int) the_option);
}


static t_bool
OptionOnPage(
	int option)
{
	if ((option >= first_option_on_screen) && (option < first_option_on_screen + option_lines_per_page))
		return TRUE;
	return FALSE;
}


static void
RepaintOption(
	int option)
{
	if (OptionOnPage(option))
		print_any_option(option);
}


#ifdef USE_CURSES
static void
DoScroll(
	int jump)
{
	MoveCursor(INDEX_TOP, 0);
	SetScrollRegion(INDEX_TOP, INDEX_TOP + option_lines_per_page - 1);
	ScrollScreen(jump);
	SetScrollRegion(0, LINES - 1);
}
#endif /* USE_CURSES */


static void
highlight_option(
	int option)
{
	if (!OptionOnPage(option)) {
#ifdef USE_CURSES
		if (option > 0 && OptionOnPage(option - 1)) {
			DoScroll(1);
			first_option_on_screen++;
		} else if (option < LAST_OPT && OptionOnPage(option + 1)) {
			DoScroll(-1);
			first_option_on_screen--;
		} else
#endif /* USE_CURSES */
		{
			first_option_on_screen = TopOfPage(option);
			ClearScreen();
		}
	}

	refresh_config_page(option);
	draw_arrow_mark(option_row(option));
	info_message("%s", _(option_table[option].txt->opt));
}


static void
unhighlight_option(
	int option)
{
	/* Astonishing hack */
	t_menu *savemenu = currmenu;
	t_menu cfgmenu = { 0, 1, 0, 0, NULL, NULL };

	currmenu = &cfgmenu;
	currmenu->curr = option_row(option) - INDEX_TOP;
	erase_arrow();
	currmenu = savemenu;
	clear_message();
}


/*
 * Refresh the config page which holds the actual option. If act_option is
 * smaller zero fall back on the last given option (first option if there was
 * no last option) and refresh the screen.
 */
void
refresh_config_page(
	int act_option)
{
	static int last_option = 0;
	t_bool force_redraw = FALSE;

	signal_context = cConfig;

	if (act_option < 0) {
		force_redraw = TRUE;
		act_option = last_option;
		ClearScreen();
	}

	if ((first_option_on_screen != actual_top_option) || force_redraw) {
		show_config_page();
		actual_top_option = first_option_on_screen;
	}
	last_option = act_option;
}


static void
redraw_screen(
	int option)
{
	my_retouch();
	set_xclick_off();
	ClearScreen();
	show_config_page();
	highlight_option(option);
}


/*
 * options menu so that the user can dynamically change parameters
 *
 * TODO: - why do we use ret_code when we never modify it?  what about calling
 *         code which checks the return value?
 *       - when we change something we need to update the related attributes
 *         as well (see line 2009).
 */
int
change_config_file(
	struct t_group *group)
{
	int ch = 0;
	int option, old_option;
	int ret_code = NO_FILTERING;
	int mime_encoding = MIME_ENCODING_7BIT;
	t_bool change_option = FALSE;

	actual_top_option = -1;
	option = 1;

	ClearScreen();
	set_xclick_off();
	forever {
		highlight_option(option);
		stow_cursor();
		ch = ReadCh();

		/*
		 * convert arrow key codes to "normal" codes
		 */
		switch (ch) {
			case ESC:	/* common arrow keys */
#	ifdef HAVE_KEY_PREFIX
			case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
				switch (get_arrow_key(ch)) {
					case KEYMAP_UP:
						ch = map_to_local(iKeyUp, &menukeymap.config_change);
						break;

					case KEYMAP_DOWN:
						ch = map_to_local(iKeyDown, &menukeymap.config_change);
						break;

					case KEYMAP_HOME:
						ch = map_to_local(iKeyFirstPage, &menukeymap.config_change);
						break;

					case KEYMAP_END:
						ch = map_to_local(iKeyLastPage, &menukeymap.config_change);
						break;

					case KEYMAP_PAGE_UP:
						ch = map_to_local(iKeyPageUp, &menukeymap.config_change);
						break;

					case KEYMAP_PAGE_DOWN:
						ch = map_to_local(iKeyPageDown, &menukeymap.config_change);
						break;

					default:
						break;
				} /* switch (get_arrow_key()) */
				break;

			default:
				break;
		}	/* switch (ch) */

		switch (map_to_default(ch, &menukeymap.config_change)) {
			case iKeyQuit:
				write_config_file(local_config_file);
				/* FALLTHROUGH */
			case iKeyConfigNoSave:
				clear_note_area();
				return ret_code;

			case iKeyUp:
			case iKeyUp2:
				unhighlight_option(option);
				if (!get_option_num(--option))	/* skip over titles */
					option--;
				if (option < 0)
					option = LAST_OPT;
				highlight_option(option);
				break;

			case iKeyDown:
			case iKeyDown2:
				unhighlight_option(option);
				if (!get_option_num(++option))	/* skip over titles */
					option++;
				if (option > LAST_OPT)
					option = 1;
				highlight_option(option);
				break;

			case iKeyFirstPage:
			case iKeyConfigFirstPage2:
				unhighlight_option(option);
				option = 1;
				highlight_option(option);
				break;

			case iKeyLastPage:
			case iKeyConfigLastPage2:
				unhighlight_option(option);
				option = LAST_OPT;
				highlight_option(option);
				break;

			case iKeyPageUp:
			case iKeyPageUp2:
			case iKeyPageUp3:
				unhighlight_option(option);
				if (OptionInPage(option)) {
					option = first_option_on_screen;
				} else if (!first_option_on_screen) {
					option = LAST_OPT;
					first_option_on_screen = TopOfPage(option);
					ClearScreen();
					show_config_page();
				} else if ((option -= option_lines_per_page) < 0) {
					option = 1;
					first_option_on_screen = 0;
				} else {
					first_option_on_screen -= (tinrc.scroll_lines == -2) ? option_lines_per_page / 2 : option_lines_per_page;
					ClearScreen();
					show_config_page();
				}
				highlight_option(option);
				break;

			case iKeyPageDown:
			case iKeyPageDown2:
			case iKeyPageDown3:
				unhighlight_option(option);
				first_option_on_screen += (tinrc.scroll_lines == -2) ? option_lines_per_page / 2 : option_lines_per_page;
				if (first_option_on_screen > LAST_OPT)
					first_option_on_screen = 0;

				option = first_option_on_screen;
				ClearScreen();
				show_config_page();
				highlight_option(option);
				break;

			case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				unhighlight_option(option);
				old_option = option;
				option = prompt_num(ch, _(txt_enter_option_num)) - 1;
				option = set_option_num(option);
				if (option < 0 || option > LAST_OPT) {
					option = old_option;
					break;
				}
				highlight_option(option);
				break;

			case iKeySearchSubjF:
			case iKeySearchSubjB:
			case iKeySearchRepeat:
				if (ch == iKeySearchRepeat && i_key_search_last != iKeySearchSubjF && i_key_search_last != iKeySearchSubjB)
					break;

				old_option = option;
				option = search_config((ch == iKeySearchSubjF), (ch == iKeySearchRepeat), option, LAST_OPT);
				if (option != old_option) {
					unhighlight_option(old_option);
					highlight_option(option);
				}
				break;

			case iKeyConfigSelect:
			case iKeyConfigSelect2:
				change_option = TRUE;
				break;

			case iKeyRedrawScr:	/* redraw screen */
				redraw_screen(option);
				break;

			default:
				break;
		} /* switch (ch) */

		if (change_option) {
			switch (option_table[option].var_type) {
				case OPT_ON_OFF:
					switch (option) {
						case OPT_ADD_POSTED_TO_FILTER:
						case OPT_ADVERTISING:
						case OPT_ALTERNATIVE_HANDLING:
						case OPT_ASK_FOR_METAMAIL:
						case OPT_AUTO_BCC:
						case OPT_AUTO_CC:
						case OPT_AUTO_LIST_THREAD:
						case OPT_AUTO_RECONNECT:
						case OPT_AUTO_SAVE:
						case OPT_BATCH_SAVE:
						case OPT_CACHE_OVERVIEW_FILES:
						case OPT_CATCHUP_READ_GROUPS:
						case OPT_FORCE_SCREEN_REDRAW:
						case OPT_GROUP_CATCHUP_ON_EXIT:
						case OPT_KEEP_DEAD_ARTICLES:
						case OPT_MARK_IGNORE_TAGS:
						case OPT_MARK_SAVED_READ:
						case OPT_PGDN_GOTO_NEXT:
						case OPT_POS_FIRST_UNREAD:
						case OPT_POST_PROCESS_VIEW:
						case OPT_PRINT_HEADER:
						case OPT_PROCESS_ONLY_UNREAD:
						case OPT_PROMPT_FOLLOWUPTO:
						case OPT_SHOW_ONLY_UNREAD_GROUPS:
						case OPT_SHOW_SIGNATURES:
						case OPT_SIGDASHES:
						case OPT_SIGNATURE_REPOST:
						case OPT_SPACE_GOTO_NEXT_UNREAD:
						case OPT_START_EDITOR_OFFSET:
						case OPT_STRIP_BLANKS:
						case OPT_STRIP_NEWSRC:
						case OPT_TAB_GOTO_NEXT_UNREAD:
						case OPT_TEX2ISO_CONV:
						case OPT_THREAD_CATCHUP_ON_EXIT:
#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
						case OPT_TRANSLIT:
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */
						case OPT_UNLINK_ARTICLE:
						case OPT_URL_HIGHLIGHT:
#ifdef HAVE_KEYPAD
						case OPT_USE_KEYPAD:
#endif /* HAVE_KEYPAD */
						case OPT_USE_MOUSE:
						case OPT_WRAP_ON_NEXT_UNREAD:
							prompt_option_on_off(option);
							break;

						/* show mini help menu */
						case OPT_BEGINNER_LEVEL:
							if (prompt_option_on_off(option))
								set_noteslines(cLINES);
							break;

						/* show all arts or just new/unread arts */
						case OPT_SHOW_ONLY_UNREAD_ARTS:
							if (prompt_option_on_off(option) && group != NULL) {
								make_threads(group, TRUE);
								pos_first_unread_thread();
							}
							break;

						/* draw -> / highlighted bar */
						case OPT_DRAW_ARROW:
							prompt_option_on_off(option);
							unhighlight_option(option);
							if (!tinrc.draw_arrow && !tinrc.inverse_okay) {
								tinrc.inverse_okay = TRUE;
								RepaintOption(OPT_INVERSE_OKAY);
							}
							break;

						/* draw inversed screen header lines */
						/* draw inversed group/article/option line if draw_arrow is OFF */
						case OPT_INVERSE_OKAY:
							prompt_option_on_off(option);
							unhighlight_option(option);
							if (!tinrc.draw_arrow && !tinrc.inverse_okay) {
								tinrc.draw_arrow = TRUE;	/* we don't want to navigate blindly */
								RepaintOption(OPT_DRAW_ARROW);
							}
							break;

						case OPT_MAIL_8BIT_HEADER:
							prompt_option_on_off(option);
							if (strcasecmp(txt_mime_encodings[tinrc.mail_mime_encoding], txt_8bit)) {
								tinrc.mail_8bit_header = FALSE;
								print_option(OPT_MAIL_8BIT_HEADER);
							}
							break;

						case OPT_POST_8BIT_HEADER:
							prompt_option_on_off(option);
							/* if post_mime_encoding != 8bit, post_8bit_header is disabled */
							if (strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_8bit)) {
								tinrc.post_8bit_header = FALSE;
								print_option(OPT_POST_8BIT_HEADER);
							}
							break;

						/* show newsgroup description text next to newsgroups */
						case OPT_SHOW_DESCRIPTION:
							prompt_option_on_off(option);
							show_description = tinrc.show_description;
							if (show_description)			/* force reread of newgroups file */
								read_descriptions(FALSE);
							break;

#ifdef HAVE_COLOR
						/* use ANSI color */
						case OPT_USE_COLOR:
							prompt_option_on_off(option);
#	ifdef USE_CURSES
							if (!has_colors())
								use_color = FALSE;
							else
#	endif /* USE_CURSES */
								use_color = tinrc.use_color;
							break;
#endif /* HAVE_COLOR */

#ifdef XFACE_ABLE
						/* use slrnface */
						case OPT_USE_SLRNFACE:
							if (prompt_option_on_off(option)) {
								if (!tinrc.use_slrnface)
									slrnface_stop();
								else
									slrnface_start();
							}
							break;
#endif /* XFACE_ABLE */

						/* word_highlight */
						case OPT_WORD_HIGHLIGHT:
							prompt_option_on_off(option);
							word_highlight = tinrc.word_highlight;
							break;

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
						case OPT_RENDER_BIDI:
							prompt_option_on_off(option);
							break;
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_LIST:
					switch (option) {
#ifdef HAVE_COLOR
						case OPT_COL_BACK:
						case OPT_COL_FROM:
						case OPT_COL_HEAD:
						case OPT_COL_HELP:
						case OPT_COL_INVERS_BG:
						case OPT_COL_INVERS_FG:
						case OPT_COL_MESSAGE:
						case OPT_COL_MINIHELP:
						case OPT_COL_NEWSHEADERS:
						case OPT_COL_NORMAL:
						case OPT_COL_QUOTE:
						case OPT_COL_QUOTE2:
						case OPT_COL_QUOTE3:
						case OPT_COL_RESPONSE:
						case OPT_COL_SIGNATURE:
						case OPT_COL_SUBJECT:
						case OPT_COL_TEXT:
						case OPT_COL_TITLE:
						case OPT_COL_MARKSTAR:
						case OPT_COL_MARKDASH:
						case OPT_COL_MARKSLASH:
						case OPT_COL_MARKSTROKE:
						case OPT_COL_URLS:
#endif /* HAVE_COLOR */
						case OPT_HIDE_UUE:
						case OPT_INTERACTIVE_MAILER:
						case OPT_WORD_H_DISPLAY_MARKS:
						case OPT_MONO_MARKSTAR:
						case OPT_MONO_MARKDASH:
						case OPT_MONO_MARKSLASH:
						case OPT_MONO_MARKSTROKE:
						case OPT_CONFIRM_CHOICE:
						case OPT_KILL_LEVEL:
						case OPT_MAILBOX_FORMAT:
						case OPT_SHOW_INFO:
						case OPT_SORT_ARTICLE_TYPE:
						case OPT_STRIP_BOGUS:
#ifdef HAVE_UNICODE_NORMALIZATION
						case OPT_NORMALIZATION_FORM:
#endif /* HAVE_UNICODE_NORMALIZATION */
						case OPT_QUOTE_STYLE:
						case OPT_WILDCARD:
							prompt_option_list(option);
							break;

						case OPT_THREAD_ARTICLES:
							/*
							 * If the threading strategy has changed, fix things
							 * so that rethreading will occur
							 */
							if (prompt_option_list(option) && group != NULL) {
								int n, old_base_art = base[grpmenu.curr];

								group->attribute->thread_arts = tinrc.thread_articles;
								make_threads(group, TRUE);
								/* in non-empty groups update cursor position */
								if (grpmenu.max > 0) {
									if ((n = which_thread(old_base_art)) >= 0)
										grpmenu.curr = n;
								}
							}
							clear_message();
							break;

						case OPT_SORT_THREADS_TYPE:
							/*
							 * If the sorting strategy of threads has changed, fix things
							 * so that resorting will occur
							 */
							if (prompt_option_list(option) && group != NULL) {
								group->attribute->sort_threads_type = tinrc.sort_threads_type;
								make_threads(group, TRUE);
							}
							clear_message();
							break;

						case OPT_THREAD_SCORE:
							/*
							 * If the scoring of a thread has changed,
							 * resort base[]
							 */
							if (prompt_option_list(option) && group != NULL)
								find_base(group);
							clear_message();
							break;

						case OPT_POST_PROCESS:
							prompt_option_list(option);
							glob_attributes.post_proc_type = tinrc.post_process;
							if (group != NULL)
								group->attribute->post_proc_type = tinrc.post_process;
							break;

						case OPT_SHOW_AUTHOR:
							prompt_option_list(option);
							if (group != NULL)
								group->attribute->show_author = tinrc.show_author;
							break;

						case OPT_MAIL_MIME_ENCODING:
						case OPT_POST_MIME_ENCODING:
							prompt_option_list(option);
							mime_encoding = *(option_table[option].variable);
							/* do not use 8 bit headers if mime encoding is not 8bit */
							if (strcasecmp(txt_mime_encodings[mime_encoding], txt_8bit)) {
								if (option == (int) OPT_POST_MIME_ENCODING) {
									tinrc.post_8bit_header = FALSE;
									RepaintOption(OPT_POST_8BIT_HEADER);
								} else {
									tinrc.mail_8bit_header = FALSE;
									RepaintOption(OPT_MAIL_8BIT_HEADER);
								}
							}
							break;

#ifdef CHARSET_CONVERSION
						case OPT_MM_NETWORK_CHARSET:
							if (prompt_option_list(option)) {
								glob_attributes.mm_network_charset = tinrc.mm_network_charset;
								if (group)
									group->attribute->mm_network_charset = tinrc.mm_network_charset;
#	ifdef NO_LOCALE
								strcpy(tinrc.mm_local_charset, txt_mime_charsets[tinrc.mm_network_charset]);
#	endif /* NO_LOCALE */
							}
							/*
							 * check if we have selected a 7bit charset, otherwise
							 * update encoding
							 * we always do this (even if we did not change the
							 * charset) to fixup flaws in the tinrc - once we do
							 * the same while reading the tinrc this can go into
							 * the != original_list_value case.
							 */
							{
								int i;
								t_bool change;

								if (!strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_7bit)) {
									change = TRUE;
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											change = FALSE;
											break;
										}
									}
									if (change) {
										tinrc.post_mime_encoding = MIME_ENCODING_8BIT;
										RepaintOption(OPT_POST_MIME_ENCODING);
									}
								} else { /* and vice versa, if we have a 7bit chaset but a !7bit encoding, fix that */
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											tinrc.mail_mime_encoding = tinrc.post_mime_encoding = MIME_ENCODING_7BIT;
											tinrc.mail_8bit_header = tinrc.post_8bit_header = FALSE;
											RepaintOption(OPT_POST_MIME_ENCODING);
											RepaintOption(OPT_MAIL_MIME_ENCODING);
											RepaintOption(OPT_POST_8BIT_HEADER);
											break;
										}
									}
								}

								if (!strcasecmp(txt_mime_encodings[tinrc.mail_mime_encoding], txt_7bit)) {
									change = TRUE;
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											change = FALSE;
											break;
										}
									}
									if (change) {
										tinrc.mail_mime_encoding = MIME_ENCODING_QP;
										RepaintOption(OPT_MAIL_MIME_ENCODING);
									}
								} else { /* and vice versa, if we have a 7bit chaset but a !7bit encoding, fix that */
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											tinrc.mail_mime_encoding = tinrc.post_mime_encoding = MIME_ENCODING_7BIT;
											tinrc.mail_8bit_header = tinrc.post_8bit_header = FALSE;
											RepaintOption(OPT_POST_MIME_ENCODING);
											RepaintOption(OPT_MAIL_MIME_ENCODING);
											RepaintOption(OPT_POST_8BIT_HEADER);
											break;
										}
									}
								}
							}
							break;
#endif /* CHARSET_CONVERSION */

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_STRING:
					switch (option) {
						case OPT_EDITOR_FORMAT:
						case OPT_INEWS_PROG:
						case OPT_MAILER_FORMAT:
						case OPT_MAIL_ADDRESS:
						case OPT_MAIL_QUOTE_FORMAT:
						case OPT_METAMAIL_PROG:
#ifndef CHARSET_CONVERSION
						case OPT_MM_CHARSET:
#endif /* !CHARSET_CONVERSION */
						case OPT_NEWS_QUOTE_FORMAT:
						case OPT_QUOTE_CHARS:
						case OPT_SPAMTRAP_WARNING_ADDRESSES:
						case OPT_URL_HANDLER:
						case OPT_XPOST_QUOTE_FORMAT:
							prompt_option_string(option);
							break;

						case OPT_NEWS_HEADERS_TO_DISPLAY:
							prompt_option_string(option);
							if (news_headers_to_display_array)
								FreeIfNeeded(*news_headers_to_display_array);
							FreeIfNeeded(news_headers_to_display_array);
							news_headers_to_display_array = ulBuildArgv(tinrc.news_headers_to_display, &num_headers_to_display);
							break;

						case OPT_NEWS_HEADERS_TO_NOT_DISPLAY:
							prompt_option_string(option);
							if (news_headers_to_not_display_array)
								FreeIfNeeded(*news_headers_to_not_display_array);
							FreeIfNeeded(news_headers_to_not_display_array);
							news_headers_to_not_display_array = ulBuildArgv(tinrc.news_headers_to_not_display, &num_headers_to_not_display);
							break;

#ifndef DISABLE_PRINTING
						case OPT_PRINTER:
#endif /* !DISABLE_PRINTING */
						case OPT_MAILDIR:
						case OPT_SAVEDIR:
						case OPT_SIGFILE:
						case OPT_POSTED_ARTICLES_FILE:
							if (prompt_option_string(option)) {
								char buf[PATH_LEN];

								strfpath(tinrc.posted_articles_file, buf, sizeof(buf), &CURR_GROUP);
								STRCPY(tinrc.posted_articles_file, buf);
							}
							break;

#ifdef HAVE_COLOR
						case OPT_QUOTE_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(quote_regex.re);
							FreeIfNeeded(quote_regex.extra);
							if (!strlen(tinrc.quote_regex))
								STRCPY(tinrc.quote_regex, DEFAULT_QUOTE_REGEX);
							compile_regex(tinrc.quote_regex, &quote_regex, PCRE_CASELESS);
							break;

						case OPT_QUOTE_REGEX2:
							prompt_option_string(option);
							FreeIfNeeded(quote_regex2.re);
							FreeIfNeeded(quote_regex2.extra);
							if (!strlen(tinrc.quote_regex2))
								STRCPY(tinrc.quote_regex2, DEFAULT_QUOTE_REGEX2);
							compile_regex(tinrc.quote_regex2, &quote_regex2, PCRE_CASELESS);
							break;

						case OPT_QUOTE_REGEX3:
							prompt_option_string(option);
							FreeIfNeeded(quote_regex3.re);
							FreeIfNeeded(quote_regex3.extra);
							if (!strlen(tinrc.quote_regex3))
								STRCPY(tinrc.quote_regex3, DEFAULT_QUOTE_REGEX3);
							compile_regex(tinrc.quote_regex3, &quote_regex3, PCRE_CASELESS);
							break;
#endif /* HAVE_COLOR */

						case OPT_SLASHES_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(slashes_regex.re);
							FreeIfNeeded(slashes_regex.extra);
							if (!strlen(tinrc.slashes_regex))
								STRCPY(tinrc.slashes_regex, DEFAULT_SLASHES_REGEX);
							compile_regex(tinrc.slashes_regex, &slashes_regex, PCRE_CASELESS);
							break;

						case OPT_STARS_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(stars_regex.re);
							FreeIfNeeded(stars_regex.extra);
							if (!strlen(tinrc.stars_regex))
								STRCPY(tinrc.stars_regex, DEFAULT_STARS_REGEX);
							compile_regex(tinrc.stars_regex, &stars_regex, PCRE_CASELESS);
							break;

						case OPT_STROKES_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(strokes_regex.re);
							FreeIfNeeded(strokes_regex.extra);
							if (!strlen(tinrc.strokes_regex))
								STRCPY(tinrc.strokes_regex, DEFAULT_STROKES_REGEX);
							compile_regex(tinrc.strokes_regex, &strokes_regex, PCRE_CASELESS);
							break;

						case OPT_UNDERSCORES_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(underscores_regex.re);
							FreeIfNeeded(underscores_regex.extra);
							if (!strlen(tinrc.underscores_regex))
								STRCPY(tinrc.underscores_regex, DEFAULT_UNDERSCORES_REGEX);
							compile_regex(tinrc.underscores_regex, &underscores_regex, PCRE_CASELESS);
							break;

						case OPT_STRIP_RE_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(strip_re_regex.re);
							FreeIfNeeded(strip_re_regex.extra);
							if (!strlen(tinrc.strip_re_regex))
								STRCPY(tinrc.strip_re_regex, DEFAULT_STRIP_RE_REGEX);
							compile_regex(tinrc.strip_re_regex, &strip_re_regex, PCRE_ANCHORED);
							break;

						case OPT_STRIP_WAS_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(strip_was_regex.re);
							FreeIfNeeded(strip_was_regex.extra);
							if (!strlen(tinrc.strip_was_regex)) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
								if (IS_LOCAL_CHARSET("UTF-8")) {
#	if (defined(PCRE_MAJOR) && PCRE_MAJOR >= 4)
									int i;

									pcre_config(PCRE_CONFIG_UTF8, &i);
									if (i)
										STRCPY(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX);
									else
#	endif /* PCRE_MAJOR && PCRE_MAJOR >=4 */
										STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
								} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
									STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
							}
							compile_regex(tinrc.strip_was_regex, &strip_was_regex, 0);
							break;

						case OPT_DATE_FORMAT:
							prompt_option_string(option);
							if (!strlen(tinrc.date_format)) {
								STRCPY(tinrc.date_format, DEFAULT_DATE_FORMAT);
							}
							break;

						default:
							break;
					} /* switch (option) */

					break;

				case OPT_NUM:
					switch (option) {
						case OPT_GETART_LIMIT:
						case OPT_SCROLL_LINES:
							prompt_option_num(option);
							break;

						case OPT_REREAD_ACTIVE_FILE_SECS:
							prompt_option_num(option);
							if (tinrc.reread_active_file_secs < 0)
								tinrc.reread_active_file_secs = 0;
							break;

						case OPT_RECENT_TIME:
							prompt_option_num(option);
							if (tinrc.recent_time < 0)
								tinrc.recent_time = 0;
							break;

						case OPT_GROUPNAME_MAX_LENGTH:
							prompt_option_num(option);
							if (tinrc.groupname_max_length < 0)
								tinrc.groupname_max_length = 0;
							break;

						case OPT_FILTER_DAYS:
							prompt_option_num(option);
							if (tinrc.filter_days <= 0)
								tinrc.filter_days = 1;
							break;

						case OPT_SCORE_LIMIT_KILL:
						case OPT_SCORE_KILL:
						case OPT_SCORE_LIMIT_SELECT:
						case OPT_SCORE_SELECT:
							prompt_option_num(option);
							check_score_defaults();
							if (group != NULL) {
								unfilter_articles();
								read_filter_file(filter_file);
								if (filter_articles(group))
									make_threads(group, FALSE);
							}
							redraw_screen(option);
							break;

						case OPT_WRAP_COLUMN:
							prompt_option_num(option);
							/* recook if in an article is open */
							if (pgart.raw)
								resize_article(TRUE, &pgart);
							break;

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_CHAR:
					switch (option) {
						/*
						 * TODO: do DASH_TO_SPACE/SPACE_TO_DASH conversion here?
						 */
						case OPT_ART_MARKED_DELETED:
						case OPT_ART_MARKED_INRANGE:
						case OPT_ART_MARKED_RETURN:
						case OPT_ART_MARKED_SELECTED:
						case OPT_ART_MARKED_RECENT:
						case OPT_ART_MARKED_UNREAD:
						case OPT_ART_MARKED_READ:
						case OPT_ART_MARKED_KILLED:
						case OPT_ART_MARKED_READ_SELECTED:
							prompt_option_char(option);
							break;

						default:
							break;
					} /* switch (option) */
					break;

				default:
					break;
			} /* switch (option_table[option].var_type) */
			change_option = FALSE;
			show_menu_help(txt_select_config_file_option);
			RepaintOption(option);
		} /* if (change_option) */
	} /* forever */
	/* NOTREACHED */
	return ret_code;
}


/*
 * show_menu_help
 */
void
show_menu_help(
	const char *help_message)
{
	MoveCursor(cLINES - 2, 0);
	CleartoEOLN();
	center_line(cLINES - 2, FALSE, _(help_message));
}


t_bool
match_boolean(
	char *line,
	const char *pat,
	t_bool *dst)
{
	size_t patlen = strlen(pat);

	if (STRNCASECMPEQ(line, pat, patlen)) {
		*dst = (t_bool) (STRNCASECMPEQ(&line[patlen], "ON", 2) ? TRUE : FALSE);
		return TRUE;
	}
	return FALSE;
}


#ifdef HAVE_COLOR
static t_bool
match_color(
	char *line,
	const char *pat,
	int *dst,
	int max)
{
	int n;
	size_t patlen = strlen(pat);

	if (STRNCMPEQ(line, pat, patlen)) {
		t_bool found = FALSE;
		for (n = 0; n < MAX_COLOR + 1; n++) {
			if (!strcasecmp(&line[patlen], txt_colors[n])) {
				found = TRUE;
				*dst = n;
				if (*dst > max)
					*dst = -1;
			}
		}

		if (!found)
			*dst = atoi(&line[patlen]);

		if (max) {
			if ((*dst < -1) || (*dst > max)) {
				my_fprintf(stderr, _(txt_value_out_of_range), pat, *dst, max);
				*dst = 0;
			}
		}
		return TRUE;
	}
	return FALSE;
}
#endif /* HAVE_COLOR */


/*
 * If pat matches the start of line, convert rest of line to an integer, dst
 * If maxval is set, constrain value to 0 <= dst <= maxlen and return TRUE.
 * If no match is made, return FALSE.
 */
t_bool
match_integer(
	char *line,
	const char *pat,
	int *dst,
	int maxval)
{
	size_t patlen = strlen(pat);

	if (STRNCMPEQ(line, pat, patlen)) {
		*dst = atoi(&line[patlen]);

		if (maxval) {
			if ((*dst < 0) || (*dst > maxval)) {
				my_fprintf(stderr, _(txt_value_out_of_range), pat, *dst, maxval);
				*dst = 0;
			}
		}
		return TRUE;
	}
	return FALSE;
}


t_bool
match_long(
	char *line,
	const char *pat,
	long *dst)
{
	size_t patlen = strlen(pat);

	if (STRNCMPEQ(line, pat, patlen)) {
		*dst = atol(&line[patlen]);
		return TRUE;
	}
	return FALSE;
}


/*
 * If the 'pat' keyword matches, lookup & return an index into the table
 */
t_bool
match_list(
	char *line,
	constext *pat,
	constext *const *table,
	size_t tablelen,
	int *dst)
{
	size_t patlen = strlen(pat);
	size_t n;
	char temp[LEN];

	if (STRNCMPEQ(line, pat, patlen)) {
		line += patlen;
		*dst = 0;	/* default, if no match */
		for (n = 0; n < tablelen; n++) {
			if (match_item(line, table[n], temp, sizeof(temp))) {
				*dst = (int) n;
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}


t_bool
match_string(
	char *line,
	const char *pat,
	char *dst,
	size_t dstlen)
{
	char *ptr;
	size_t patlen = strlen(pat);

	if (STRNCMPEQ(line, pat, patlen) && (strlen(line) > patlen /* + 1 */)) {
		strncpy(dst, &line[patlen], dstlen);
		if ((ptr = strrchr(dst, '\n')) != NULL)
			*ptr = '\0';

		return TRUE;
	}
	return FALSE;
}


/* like mach_string() but looks for 100% exact matches */
static t_bool
match_item(
	char *line,
	const char *pat,
	char *dst,
	size_t dstlen)
{
	char *ptr;
	char *nline = my_strdup(line);
	size_t patlen = strlen(pat);

	nline[strlen(nline) - 1] = '\0'; /* remove tailing \n */

	if (!strcasecmp(nline, pat)) {
		strncpy(dst, &nline[patlen], dstlen);
		if ((ptr = strrchr(dst, '\n')) != NULL)
			*ptr = '\0';

		free(nline);
		return TRUE;
	}
	free(nline);
	return FALSE;
}


const char *
print_boolean(
	t_bool value)
{
	return txt_onoff[value != FALSE ? 1 : 0];
}


/*
 * convert underlines to spaces in a string
 */
void
quote_dash_to_space(
	char *str)
{
	char *ptr;

	for (ptr = str; *ptr; ptr++) {
		if (*ptr == '_')
			*ptr = ' ';
	}
}


/*
 * convert spaces to underlines in a string
 */
char *
quote_space_to_dash(
	char *str)
{
	char *ptr, *dst;
	static char buf[PATH_LEN];

	dst = buf;
	for (ptr = str; *ptr; ptr++) {
		if (*ptr == ' ')
			*dst = '_';
		else
			*dst = *ptr;
		dst++;
	}
	*dst = '\0';

	return buf;
}


/*
 * display current configuration page
 */
static void
show_config_page(
	void)
{
	int i, lines_to_print = option_lines_per_page;

	center_line(0, TRUE, _(txt_options_menu));

	/*
	 * on last page, there need not be option_lines_per_page options
	 */
	if (first_option_on_screen + option_lines_per_page > LAST_OPT)
		lines_to_print = LAST_OPT + 1 - first_option_on_screen;

	for (i = 0; i < lines_to_print; i++)
		print_any_option(first_option_on_screen + i);
	CleartoEOS();

	show_menu_help(txt_select_config_file_option);
	my_flush();
	stow_cursor();
}


/*
 * Written by: Brad Viviano and Scott Powers (bcv & swp)
 *
 * Takes a 1d string and turns it into a 2d array of strings.
 *
 * Watch out for the frees! You must free(*argv) and then free(argv)!
 * NOTHING ELSE! Do _NOT_ free the individual args of argv.
 */
char **
ulBuildArgv(
	char *cmd,
	int *new_argc)
{
	char **new_argv = NULL;
	char *buf, *tmp;
	int i = 0;

	if (!cmd || !*cmd) {
		*new_argc = 0;
		return NULL;
	}

	for (tmp = cmd; isspace((int) *tmp); tmp++)
		;

	buf = my_strdup(tmp);
	if (!buf) {
		*new_argc = 0;
		return NULL;
	}

	new_argv = my_calloc(1, sizeof(char *));
	if (!new_argv) {
		free(buf);
		*new_argc = 0;
		return NULL;
	}

	tmp = buf;
	new_argv[0] = NULL;

	while (*tmp) {
		if (!isspace((int) *tmp)) { /* found the begining of a word */
			new_argv[i] = tmp;
			for (; *tmp && !isspace((int) *tmp); tmp++)
				;
			if (*tmp) {
				*tmp = '\0';
				tmp++;
			}
			i++;
			new_argv = my_realloc(new_argv, ((i + 1) * sizeof(char *)));
			new_argv[i] = NULL;
		} else
			tmp++;
	}
	*new_argc = i;
	return new_argv;
}


/*
 * Check if score_kill is <= score_limit_kill and if score_select >= score_limit_select
 */
static void
check_score_defaults(
	void)
{
	if (tinrc.score_kill > tinrc.score_limit_kill)
		tinrc.score_kill = tinrc.score_limit_kill;

	if (tinrc.score_select < tinrc.score_limit_select)
		tinrc.score_select = tinrc.score_limit_select;
}


/*
 * auto update tinrc
 */
static t_bool
rc_update(
	FILE *fp)
{
	char buf[1024];
	const char *env;
	t_bool confirm_to_quit = FALSE;
	t_bool confirm_action = FALSE;
	t_bool compress_quotes = FALSE;
	t_bool hide_uue = FALSE;
	t_bool keep_posted_articles = FALSE;
	t_bool quote_empty_lines = FALSE;
	t_bool quote_signatures = FALSE;
	t_bool save_to_mmdf_mailbox = FALSE;
	t_bool show_last_line_prev_page = FALSE;
	t_bool show_lines = FALSE;
	t_bool show_score = FALSE;
	t_bool thread_articles = FALSE;
	t_bool use_builtin_inews = FALSE;
	t_bool use_getart_limit = FALSE;
	t_bool use_mailreader_i = FALSE;
	t_bool use_metamail = FALSE;

	if (!fp)
		return FALSE;

	/* rewind(fp); */
	while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
		if (buf[0] == '#' || buf[0] == '\n')
			continue;

		switch (tolower((unsigned char) buf[0])) {
			case 'c':
				if (match_boolean(buf, "confirm_action=", &confirm_action))
					break;
				if (match_boolean(buf, "confirm_to_quit=", &confirm_to_quit))
					break;
				if (match_boolean(buf, "compress_quotes=", &compress_quotes))
					break;
				break;

			case 'd':
				/* simple rename */
				if (match_string(buf, "default_regex_pattern=", tinrc.default_pattern, sizeof(tinrc.default_pattern)))
					break;
				break;

			case 'h':
				if (match_boolean(buf, "hide_uue=", &hide_uue))
					break;
				break;

			case 'k':
				if (match_boolean(buf, "keep_posted_articles=", &keep_posted_articles))
					break;
				break;

			case 'q':
				if (match_boolean(buf, "quote_signatures=" , &quote_signatures))
					break;
				if (match_boolean(buf, "quote_empty_lines=" , &quote_empty_lines))
					break;
				break;

			case 's':
				if (match_boolean(buf, "save_to_mmdf_mailbox=", &save_to_mmdf_mailbox))
					break;
				if (match_boolean(buf, "show_last_line_prev_page=", &show_last_line_prev_page))
					break;
				if (match_boolean(buf, "show_lines=", &show_lines))
					break;
				if (match_boolean(buf, "show_score=", &show_score))
					break;
				break;

			case 't':
				if (match_boolean(buf, "thread_articles=", &thread_articles))
					break;
				break;

			case 'u':
				if (match_boolean(buf, "use_builtin_inews=", &use_builtin_inews))
					break;
				if (match_boolean(buf, "use_getart_limit=", &use_getart_limit))
					break;
				if (match_boolean(buf, "use_mailreader_i=", &use_mailreader_i))
					break;
				if (match_boolean(buf, "use_metamail=", &use_metamail))
					break;
				break;

			default:
				break;
		}
	}

	/* update the values */
	tinrc.confirm_choice = (confirm_action ? 1 : 0 ) + (confirm_to_quit ? 3 : 0);

	if (!use_getart_limit)
		tinrc.getart_limit = 0;

	if (hide_uue)
		tinrc.hide_uue = 1;

	if (keep_posted_articles)
		strncpy(tinrc.posted_articles_file, "posted", sizeof(tinrc.posted_articles_file) - 1);

	tinrc.quote_style = (compress_quotes ? QUOTE_COMPRESS : 0) + (quote_empty_lines ? QUOTE_EMPTY : 0) + (quote_signatures ? QUOTE_SIGS : 0);

	tinrc.mailbox_format = (save_to_mmdf_mailbox ? 2 : 0);

	tinrc.show_info = (show_lines ? SHOW_INFO_LINES : 0) + (show_score ? SHOW_INFO_SCORE : 0);

	if (show_last_line_prev_page)
		tinrc.scroll_lines = -1;

	if (thread_articles)
		tinrc.thread_articles = THREAD_BOTH;

	if (use_builtin_inews)
		strncpy(tinrc.inews_prog, INTERNAL_CMD, sizeof(tinrc.inews_prog) - 1);

	if (use_mailreader_i)
		tinrc.interactive_mailer = INTERACTIVE_WITHOUT_HEADERS;

	env = getenv("NOMETAMAIL");
	if (!use_metamail || (NULL == env))
		strncpy(tinrc.metamail_prog, INTERNAL_CMD, sizeof(tinrc.metamail_prog) - 1);
	else
		my_strncpy(tinrc.metamail_prog, METAMAIL_CMD, sizeof(tinrc.metamail_prog) - 1);

	rewind(fp);
	return TRUE;
}


void
read_server_config(
	void)
{
	FILE *fp;
	char *line;
	char file[PATH_LEN];
	char newnews_info[LEN];
	char serverdir[PATH_LEN];
	char version[LEN];
	int upgrade = RC_CHECK;

#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news && nntp_tcp_port != IPPORT_NNTP)
		snprintf(file, sizeof(file), "%s:%d", nntp_server, nntp_tcp_port);
	else
#endif /* NNTP_ABLE */
	{
		STRCPY(file, quote_space_to_dash(nntp_server));
	}
	JOINPATH(serverdir, rcdir, file);
	joinpath(file, serverdir, SERVERCONFIG_FILE);

	if ((fp = fopen(file, "r")) == NULL)
		return;
	while (NULL != (line = tin_fgets(fp, FALSE))) {
		if (('#' == *line) || ('\0' == *line))
			continue;

		if (match_string(line, "last_newnews=", newnews_info, sizeof(newnews_info))) {
			int tmp_len = strlen(nntp_server) + strlen(newnews_info) + 2;
			char *tmp_info = my_malloc(tmp_len);

			snprintf(tmp_info, tmp_len, "%s %s", nntp_server, newnews_info);
			load_newnews_info(tmp_info);
			free(tmp_info);
			continue;
		}
		if (match_string(line, "version=", version, sizeof(version))) {
			if (RC_CHECK != upgrade)
				/* ignore duplicate version lines; last match counts */
				continue;
			upgrade = check_upgrade(line, "version=", SERVERCONFIG_VERSION);
			if (RC_IGNORE == upgrade)
				/* Expected version number; nothing to do -> continue */
				continue;

			/* Nothing to do yet for RC_UPGRADE and RC_DOWNGRADE */
			continue;
		}
	}
	fclose(fp);
}


static void
write_server_config(
	void)
{
	FILE *fp;
	char *file_tmp;
	char file[PATH_LEN];
	char timestring[30];
	char serverdir[PATH_LEN];
	int i;
	struct stat statbuf;

	if (read_saved_news)
		/* don't update server files while reading locally stored articles */
		return;
#ifdef NNTP_ABLE
	if (read_news_via_nntp && nntp_tcp_port != IPPORT_NNTP)
		snprintf(file, sizeof(file), "%s:%d", nntp_server, nntp_tcp_port);
	else
#endif /* NNTP_ABLE */
	{
		STRCPY(file, nntp_server);
	}
	JOINPATH(serverdir, rcdir, file);
	joinpath(file, serverdir, SERVERCONFIG_FILE);

	if ((no_write || post_article_and_exit || post_postponed_and_exit) && file_size(file) != -1L)
		return;

	if (-1 == stat(serverdir, &statbuf)) {
		if (-1 == my_mkdir(serverdir, (mode_t) (S_IRWXU)))
			/* Can't create directory TODO: Add error handling */
			return;
	}

	/* generate tmp-filename */
	file_tmp = get_tmpfilename(file);

	if ((fp = fopen(file_tmp, "w")) == NULL) {
		error_message(_(txt_filesystem_full_backup), SERVERCONFIG_FILE);
		free(file_tmp);
		return;
	}

	fprintf(fp, _(txt_serverconfig_header), PRODUCT, tin_progname, VERSION, RELEASEDATE, RELEASENAME, PRODUCT, PRODUCT);
	fprintf(fp, "version=%s\n", SERVERCONFIG_VERSION);

	if ((i = find_newnews_index(nntp_server)) >= 0)
		if (my_strftime(timestring, sizeof(timestring) - 1, "%Y-%m-%d %H:%M:%S UTC", gmtime(&(newnews[i].time))))
			fprintf(fp, "last_newnews=%lu (%s)\n", (unsigned long int) newnews[i].time, timestring);

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR)); /* rename_file() preserves mode */

	if (ferror(fp) || fclose(fp))
		error_message(_(txt_filesystem_full), SERVERCONFIG_FILE);
	else
		rename_file(file_tmp, file);

	free(file_tmp);
}
