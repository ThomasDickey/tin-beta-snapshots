/*
 *  Project   : tin - a Usenet reader
 *  Module    : tinrc.h
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1999-04-13
 *  Updated   : 2025-05-09
 *  Notes     :
 *
 * Copyright (c) 1999-2025 Jason Faultless <jason@altarstone.com>
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


/*
 * These are the tin defaults read from the tinrc file
 * If you change this, ensure you change the initial values in init.c
 *
 * FIXME: most default_* could/should be stored in the .inputhistory
 *        and could be nuked if tin comes with a prefilled .inputhistory
 *        which is installed automatically if no .inputhistory is found.
 *
 * TODO:  sort in a useful order (also needs reordering in init.c)
 */

#ifndef TINRC_H
#	define TINRC_H 1

struct t_config {
	char *default_goto_group;			/* default for the 'g' command */
	char *default_mail_address;
#	ifndef DONT_HAVE_PIPING
		char *default_pipe_command;
#	endif /* DONT_HAVE_PIPING */
	char *default_post_newsgroups;		/* default newsgroups to post to */
	char *default_post_subject;			/* default subject when posting */
	char *default_range_group;
	char *default_range_select;
	char *default_range_thread;
	char *default_pattern;
	char *default_repost_group;			/* default group to repost to */
	char *default_save_file;
	char *default_search_art;			/* default when searching in article */
	char *default_search_author;		/* default when searching for author */
	char *default_search_config;		/* default when searching config menu */
	char *default_search_group;			/* default when searching select screen */
	char *default_search_subject;		/* default when searching by subject */
	char *default_select_pattern;
	char *default_shell_command;
	char *editor_format;				/* editor + parameters  %E +%N %F */
	char *select_format;				/* format string for the selection level */
	char *group_format;					/* format string for the group level */
	char *thread_format;				/* format string for the thread level */
	char *attachment_format;			/* format string for the attachment level */
	char *page_mime_format;				/* format string for the mime header in page level */
	char *page_uue_format;				/* format string for the uue header in page level */
	char *page_yenc_format;				/* format string for the yenc header in page level */
	char *date_format;					/* format string for the date display in the page header */
	char *mail_quote_format;
	char *mailer_format;				/* mailer + parameters  %M %S %T %F */
	char *news_quote_format;
	char *xpost_quote_format;
	char *quote_chars;					/* quote chars for posting/mails ": " (size matches prefixbuf in copy_body() */
	char *inews_prog;
	char *mail_address;					/* user's mail address */
	char *maildir;						/* mailbox dir where = saves are stored */
	char *metamail_prog;				/* name of MIME message viewer */
	char *mm_local_charset;				/* display charset, not a rc/Menu-option anymore -> should be moved elsewhere */
	char *news_headers_to_display;		/* which headers to display */
	char *news_headers_to_not_display;	/* which headers to not display */
	char *posted_articles_file;			/* if set, file in which to keep posted articles */
#	ifndef DISABLE_PRINTING
		char *printer;					/* printer program specified from tinrc */
#	endif /* !DISABLE_PRINTING */
	char *slashes_regex;				/* regex used to highlight /slashes/ */
	char *stars_regex;					/* regex used to highlight *stars* */
	char *strokes_regex;				/* regex used to highlight -strokes- */
	char *strip_re_regex;				/* regex used to find and remove 'Re:'-like strings */
	char *strip_was_regex;				/* regex used to find and remove '(was:.*'-like strings */
#	ifdef HAVE_COLOR
		char *quote_regex;				/* regex used to determine quoted lines */
		char *quote_regex2;				/* regex used to determine twice quoted lines */
		char *quote_regex3;				/* regex used to determine >=3 times quoted lines */
		char *extquote_regex;			/* regex used to determine quoted lines from external sources */
#	endif /* HAVE_COLOR */
	char *underscores_regex;			/* regex used to highlight _underscores_ */
	char *verbatim_begin_regex;			/* regex used to find the begin of a verbatim block */
	char *verbatim_end_regex;			/* regex used to find the end of a verbatim block */
	char *hideline_regex;				/* regex to skip over matching lines */
	char *savedir;						/* directory to save articles to */
	char *sigfile;
	char *spamtrap_warning_addresses;
#	ifdef NNTPS_ABLE
		char *tls_ca_cert_file;			/* file containing trusted CA certificates */
#	endif /* NNTPS_ABLE */
	char *url_handler;					/* Helper app for opening URL's */
#	ifndef CHARSET_CONVERSION
		char *mm_charset;				/* MIME charset */
#	else
		int mm_network_charset;			/* MIME charset */
#	endif /* !CHARSET_CONVERSION */
	int mailbox_format;					/* format of the mailbox (mboxo, mboxrd, mmdf, ...) */
	int auto_cc_bcc;					/* add your name to cc/bcc automatically */
#	ifdef USE_CANLOCK
		int cancel_lock_algo;			/* algorithm used for Cancel-Lock/Cancel-Key */
#	endif /* USE_CANLOCK */
#	ifdef HAVE_COLOR
		int col_back;					/* standard background color */
		int col_from;					/* color of sender (From:) */
		int col_head;					/* color of headerlines */
		int col_help;					/* color of help pages */
		int col_invers_bg;				/* color of inverse text (background) */
		int col_invers_fg;				/* color of inverse text (foreground) */
		int col_minihelp;				/* color of mini help menu */
		int col_normal;					/* standard foreground color */
		int col_markdash;				/* text highlighting with _underdashes_ */
		int col_markstar;				/* text highlighting with *stars* */
		int col_markslash;				/* text highlighting with /slashes/ */
		int col_markstroke;				/* text highlighting with -strokes- */
		int col_message;				/* color of message lines at bottom */
		int col_newsheaders;			/* color of actual news header fields */
		int col_quote;					/* color of quotelines */
		int col_quote2;					/* color of twice quoted lines */
		int col_quote3;					/* color of >=3 times quoted lines */
		int col_extquote;				/* color of quoted external text */
		int col_response;				/* color of response counter */
		int col_signature;				/* color of signature */
		int col_score_neg;				/* color of negative article score */
		int col_score_pos;				/* color of positive article score */
		int col_urls;					/* color of urls highlight */
		int col_verbatim;				/* color of verbatim blocks */
		int col_subject;				/* color of article subject */
		int col_text;					/* color of textlines */
		int col_title;					/* color of Help/Mail-Sign */
#	endif /* HAVE_COLOR */
	int confirm_choice;					/* what has to be confirmed */
	int default_filter_kill_header;
	int default_filter_select_header;
	int filter_days;					/* num of days an article filter can be active */
	int default_move_group;
	int default_save_mode;				/* Append/Overwrite existing file when saving */
	int getart_limit;					/* number of article to get */
	int goto_next_unread;				/* jump to next unread article with SPACE|PGDN|TAB */
	int hide_uue;						/* treatment of uuencoded data in pager */
	int interactive_mailer;				/* invoke user's mailreader */
	int kill_level;						/* Define how killed articles are shown */
	int mono_markdash;					/* attribute for text highlighting with _underdashes_ */
	int mono_markstar;					/* attribute for text highlighting with *stars* */
	int mono_markslash;					/* attribute for text highlighting with /slashes/ */
	int mono_markstroke;				/* attribute for text highlighting with -strokes- */
#	if defined(HAVE_ALARM) && defined(SIGALRM)
		/*
		 * # seconds after which a read from the NNTP will timeout
		 * NB: This is different from the NNTP server timing us out due to inactivity
		 */
		int nntp_read_timeout_secs;
#	endif /* HAVE_ALARM && SIGALRM */
#	ifdef HAVE_UNICODE_NORMALIZATION
		int normalization_form;
#	endif /* HAVE_UNICODE_NORMALIZATION */
	int mail_mime_encoding;
	int post_mime_encoding;
	int post_process_type;				/* type of post processing to be performed */
	int quote_style;					/* quoting behaviour */
	int recent_time;					/* Time limit when article is "fresh" */
	int reread_active_file_secs;		/* reread active file interval in seconds */
	int score_limit_kill;				/* score limit to kill articles */
	int score_limit_select;				/* score limit to select articles */
	int score_kill;						/* default score for "kill" filter rules */
	int score_select;					/* default score for "hot" filter rules */
	int scroll_lines;					/* # lines to scroll by in pager */
	int show_author;					/* show_author value from 'M' menu in tinrc */
	int show_help_mail_sign;			/* show help/mail sign in level title */
	int sort_article_type;				/* method used to sort arts[] */
	int sort_threads_type;				/* method used to sort base[] */
#	ifdef USE_HEAPSORT
		int sort_function;				/* index into sort_function[] */
#	endif /* USE_HEAPSORT */
	int strip_bogus;
	int thread_articles;				/* threading system for viewing articles */
	int thread_perc;					/* how close the match needs to be for THREAD_PERC to recognize two articles as the same thread */
	int thread_score;					/* how the score for threads is computed */
	int trim_article_body;				/* remove unnecessary blank lines */
	int verbatim_handling;				/* Detection of verbatim blocks */
	int wildcard;						/* 0=wildmat, 1=regex */
	int word_h_display_marks;			/* display * or _ when highlighting or space or nothing */
	int wrap_column;					/* screen column to wrap of text messages */
#	ifdef HAVE_COLOR
		t_bool use_color;				/* like use_color but stored in tinrc */
#	endif /* HAVE_COLOR */
	t_bool abbreviate_groupname;		/* abbreviate groupnames like n.s.readers */
	t_bool add_posted_to_filter;
	t_bool advertising;
	t_bool alternative_handling;
	t_bool ask_for_metamail;			/* enables/disables the viewer query if a MIME message is going to be displayed */
	t_bool auto_list_thread;			/* list thread when entering it using right arrow */
	t_bool auto_reconnect;				/* automatically reconnect to news server */
	t_bool batch_save;					/* save arts if -M/-S command line switch specified */
	t_bool beginner_level;				/* beginner level (shows mini help a la elm) */
	t_bool cache_overview_files;		/* create local index files for NNTP overview files */
	t_bool catchup_read_groups;			/* ask if read groups are to be marked read */
#ifdef USE_ZLIB
	t_bool compress_overview_files;		/* compress local index files (overview) with gz */
#endif /* USE_ZLIB */
	t_bool draw_arrow;					/* draw -> or highlighted bar */
	t_bool dont_break_words;			/* don't break words when wrapping long lines */
	t_bool force_screen_redraw;			/* force screen redraw after external (shell) commands */
	t_bool group_catchup_on_exit;		/* catchup group with left arrow key or not */
	t_bool info_in_last_line;
	t_bool inverse_okay;
	t_bool keep_dead_articles;			/* keep all dead articles in dead.articles */
	t_bool mail_8bit_header;			/* allow 8bit chars. in header of mail message */
	t_bool mark_ignore_tags;			/* Ignore tags for GROUP_MARK_THREAD_READ/THREAD_MARK_ARTICLE_READ */
	t_bool mark_saved_read;				/* mark saved article/thread as read */
	t_bool pos_first_unread;			/* position cursor at first/last unread article */
	t_bool post_8bit_header;			/* allow 8bit chars. in header when posting to newsgroup */
	t_bool post_process_view;			/* set TRUE to invoke mailcap viewer app */
#	ifndef DISABLE_PRINTING
		t_bool print_header;			/* print all of mail header or just Subject: & From lines */
#	endif /* !DISABLE_PRINTING */
	t_bool process_only_unread;			/* save/print/mail/pipe unread/all articles */
	t_bool prompt_followupto;			/* display empty Followup-To header in editor */
	t_bool show_description;
	t_bool show_only_unread_arts;		/* show only new/unread arts or all arts */
	t_bool show_only_unread_groups;		/* set TRUE to see only subscribed groups with new news */
	t_bool show_signatures;				/* show signatures when displaying articles */
	t_bool show_art_score;				/* show article score when displaying articles */
	t_bool sigdashes;					/* set TRUE to prepend every signature with dashes */
	t_bool signature_repost;			/* set TRUE to add signature when reposting articles */
#	ifndef USE_CURSES
		t_bool strip_blanks;
#	endif /* !USE_CURSES */
	t_bool strip_newsrc;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		t_bool suppress_soft_hyphens;	/* set TRUE to remove soft hyphens (U+00AD) from articles */
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_bool tex2iso_conv;				/* convert "a to Umlaut-a */
	t_bool thread_catchup_on_exit;		/* catchup thread with left arrow key or not */
#	if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
		t_bool translit;				/* use //TRANSLIT */
#	endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */
	t_bool unlink_article;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		t_bool utf8_graphics;			/* use utf-8 characters for line drawing */
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#	ifdef HAVE_COLOR
		t_bool extquote_handling;		/* Detection of quoted text from external sources */
#	endif /* HAVE_COLOR */
	t_bool use_mouse;					/* enables/disables mouse support under xterm */
#	ifdef HAVE_KEYPAD
		t_bool use_keypad;
#	endif /* HAVE_KEYPAD */
	t_bool default_filter_kill_case;
	t_bool default_filter_kill_expire;
	t_bool default_filter_kill_global;
	t_bool default_filter_select_case;
	t_bool default_filter_select_expire;
	t_bool default_filter_select_global;
#	ifdef XFACE_ABLE
		t_bool use_slrnface;			/* Use the slrnface program to display 'X-Face:'s */
#	endif /* XFACE_ABLE */
#	if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
		t_bool render_bidi;
#	endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
	t_bool url_highlight;				/* highlight urls in text bodies */
	t_bool word_highlight;				/* like word_highlight but stored in tinrc */
	t_bool wrap_on_next_unread;			/* Wrap around threads when searching next unread article */
	/*
	 * Chars used to show article status
	 */
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		wchar_t art_marked_deleted;
		wchar_t art_marked_inrange;		/* misnomer, as it's also used for other ranges */
		wchar_t art_marked_return;
		wchar_t art_marked_selected;
		wchar_t art_marked_recent;
		wchar_t art_marked_unread;
		wchar_t art_marked_read;
		wchar_t art_marked_killed;
		wchar_t art_marked_read_selected;
#	else
		char art_marked_deleted;
		char art_marked_inrange;		/* misnomer, as it's also used for other ranges */
		char art_marked_return;
		char art_marked_selected;
		char art_marked_recent;
		char art_marked_unread;
		char art_marked_read;
		char art_marked_killed;
		char art_marked_read_selected;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	char *attrib_editor_format;
	char *attrib_fcc;
	char *attrib_maildir;
	char *attrib_from;
	char *attrib_mailing_list;
	char *attrib_organization;
	char *attrib_followup_to;
	char *attrib_mime_types_to_save;
	char *attrib_news_headers_to_display;
	char *attrib_news_headers_to_not_display;
	char *attrib_news_quote_format;
	char *attrib_quote_chars;
	char *attrib_sigfile;
	char *attrib_savedir;
	char *attrib_savefile;
	char *attrib_x_body;
	char *attrib_x_headers;
#	ifdef HAVE_ISPELL
		char *attrib_ispell;
#	endif /* HAVE_ISPELL */
	char *attrib_quick_kill_scope;
	char *attrib_quick_select_scope;
	char *attrib_group_format;
	char *attrib_thread_format;
	char *attrib_date_format;
#	ifdef CHARSET_CONVERSION
		char *attrib_undeclared_charset;
		int attrib_mm_network_charset;
#		ifdef USE_ICU_UCSDET
		t_bool attrib_undeclared_cs_guess;
#		endif /* USE_ICU_UCSDET */
#	endif /* !CHARSET_CONVERSION */
	int attrib_trim_article_body;
	int attrib_verbatim_handling;
	int attrib_auto_cc_bcc;
	int attrib_quick_kill_header;
	int attrib_quick_select_header;
	int attrib_mail_mime_encoding;
	int attrib_post_mime_encoding;
	int attrib_post_process_type;
	int attrib_show_author;
	int attrib_sort_article_type;
	int attrib_sort_threads_type;
	int attrib_thread_articles;
	int attrib_thread_perc;
	t_bool attrib_add_posted_to_filter;
	t_bool attrib_advertising;
	t_bool attrib_alternative_handling;
	t_bool attrib_auto_list_thread;
	t_bool attrib_auto_select;
	t_bool attrib_batch_save;
	t_bool attrib_delete_tmp_files;
	t_bool attrib_group_catchup_on_exit;
	t_bool attrib_mail_8bit_header;
	t_bool attrib_mime_forward;
	t_bool attrib_mark_ignore_tags;
	t_bool attrib_mark_saved_read;
	t_bool attrib_pos_first_unread;
	t_bool attrib_post_8bit_header;
	t_bool attrib_post_process_view;
#	ifndef DISABLE_PRINTING
		t_bool attrib_print_header;
#	endif /* !DISABLE_PRINTING */
	t_bool attrib_process_only_unread;
	t_bool attrib_prompt_followupto;
	t_bool attrib_show_only_unread_arts;
	t_bool attrib_show_signatures;
	t_bool attrib_show_art_score;
	t_bool attrib_sigdashes;
	t_bool attrib_signature_repost;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		t_bool attrib_suppress_soft_hyphens;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_bool attrib_tex2iso_conv;
	t_bool attrib_thread_catchup_on_exit;
#	ifdef HAVE_COLOR
		t_bool attrib_extquote_handling;
#	endif /* HAVE_COLOR */
	t_bool attrib_x_comment_to;
	t_bool attrib_wrap_on_next_unread;
	t_bool attrib_ask_for_metamail;
	t_bool attrib_quick_kill_case;
	t_bool attrib_quick_kill_expire;
	t_bool attrib_quick_select_case;
	t_bool attrib_quick_select_expire;
};

#endif /* !TINRC_H */
