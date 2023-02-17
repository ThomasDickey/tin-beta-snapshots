/*
 *  Project   : tin - a Usenet reader
 *  Module    : tinrc.h
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1999-04-13
 *  Updated   : 2022-09-19
 *  Notes     :
 *
 * Copyright (c) 1999-2023 Jason Faultless <jason@altarstone.com>
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
	/*
	 * Chars used to show article status
	 */
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		wchar_t art_marked_deleted;
		wchar_t art_marked_inrange;	/* misnomer, as it's also used for other ranges */
		wchar_t art_marked_return;
		wchar_t art_marked_selected;
		wchar_t art_marked_recent;
		wchar_t art_marked_unread;
		wchar_t art_marked_read;
		wchar_t art_marked_killed;
		wchar_t art_marked_read_selected;
#	else
		char art_marked_deleted;
		char art_marked_inrange;	/* misnomer, as it's also used for other ranges */
		char art_marked_return;
		char art_marked_selected;
		char art_marked_recent;
		char art_marked_unread;
		char art_marked_read;
		char art_marked_killed;
		char art_marked_read_selected;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	char editor_format[PATH_LEN];		/* editor + parameters  %E +%N %F */
	char default_goto_group[HEADER_LEN];		/* default for the 'g' command */
	char default_mail_address[HEADER_LEN];
	char mailer_format[PATH_LEN];		/* mailer + parameters  %M %S %T %F */
#	ifndef DONT_HAVE_PIPING
		char default_pipe_command[LEN];
#	endif /* DONT_HAVE_PIPING */
	char default_post_newsgroups[HEADER_LEN];	/* default newsgroups to post to */
	char default_post_subject[LEN];	/* default subject when posting */
#	ifndef DISABLE_PRINTING
		char printer[LEN];					/* printer program specified from tinrc */
#	endif /* !DISABLE_PRINTING */
	char default_range_group[LEN];
	char default_range_select[LEN];
	char default_range_thread[LEN];
	char default_pattern[LEN];
	char default_repost_group[LEN];		/* default group to repost to */
	char default_save_file[PATH_LEN];
	char default_search_art[LEN];		/* default when searching in article */
	char default_search_author[HEADER_LEN];	/* default when searching for author */
	char default_search_config[LEN];	/* default when searching config menu */
	char default_search_group[HEADER_LEN];		/* default when searching select screen */
	char default_search_subject[LEN];	/* default when searching by subject */
	char default_select_pattern[LEN];
	char default_shell_command[LEN];
	char mail_quote_format[LEN];
	char maildir[PATH_LEN];				/* mailbox dir where = saves are stored */
	int mailbox_format;					/* format of the mailbox (mboxo, mboxrd, mmdf, ...) */
	char mail_address[HEADER_LEN];				/* user's mail address */
	char metamail_prog[PATH_LEN];				/* name of MIME message viewer */
#	ifndef CHARSET_CONVERSION
		char mm_charset[LEN];				/* MIME charset */
#	else
		int mm_network_charset;				/* MIME charset */
#	endif /* !CHARSET_CONVERSION */
	char mm_local_charset[LEN];		/* display charset, not a rc/Menu-option anymore -> should be moved elsewhere */
#	ifdef HAVE_ICONV_OPEN_TRANSLIT
		t_bool translit;						/* use //TRANSLIT */
#	endif /* HAVE_ICONV_OPEN_TRANSLIT */
	char news_headers_to_display[LEN];	/* which headers to display */
	char news_headers_to_not_display[LEN];	/* which headers to not display */
	char news_quote_format[LEN];
	char quote_chars[LEN];			/* quote chars for posting/mails ": " (size matches prefixbuf in copy_body() */
#	ifdef HAVE_COLOR
		char quote_regex[LEN];				/* regex used to determine quoted lines */
		char quote_regex2[LEN];				/* regex used to determine twice quoted lines */
		char quote_regex3[LEN];				/* regex used to determine >=3 times quoted lines */
		char extquote_regex[LEN];			/* regex used to determine quoted lines from external sources */
#	endif /* HAVE_COLOR */
	char slashes_regex[LEN];			/* regex used to highlight /slashes/ */
	char stars_regex[LEN];				/* regex used to highlight *stars* */
	char underscores_regex[LEN];			/* regex used to highlight _underscores_ */
	char strokes_regex[LEN];			/* regex used to highlight -strokes- */
	char sigfile[PATH_LEN];
	char strip_re_regex[LEN];			/* regex used to find and remove 'Re:'-like strings */
	char strip_was_regex[LEN];			/* regex used to find and remove '(was:.*'-like strings */
	char verbatim_begin_regex[LEN];			/* regex used to find the begin of a verbatim block */
	char verbatim_end_regex[LEN];			/* regex used to find the end of a verbatim block */
	char savedir[PATH_LEN];				/* directory to save articles to */
	char spamtrap_warning_addresses[LEN];
#	ifdef NNTPS_ABLE
		char tls_ca_cert_file[PATH_LEN];	/* file containing trusted CA certificates */
#	endif /* NNTPS_ABLE */
	char url_handler[LEN];				/* Helper app for opening URL's */
	char xpost_quote_format[LEN];
	int filter_days;					/* num of days an article filter can be active */
	int default_filter_kill_header;
	int default_filter_select_header;
	int default_move_group;
	int default_save_mode;				/* Append/Overwrite existing file when saving */
	int getart_limit;					/* number of article to get */
	int recent_time;				/* Time limit when article is "fresh" */
	int goto_next_unread;				/* jump to next unread article with SPACE|PGDN|TAB */
	int hide_uue;						/* treatment of uuencoded data in pager */
	int kill_level;						/* Define how killed articles are shown */
	int mail_mime_encoding;
	int post_mime_encoding;
	int post_process_type;				/* type of post processing to be performed */
	int reread_active_file_secs;		/* reread active file interval in seconds */
	int scroll_lines;					/* # lines to scroll by in pager */
	int show_author;					/* show_author value from 'M' menu in tinrc */
	int sort_article_type;				/* method used to sort arts[] */
	int sort_threads_type;				/* method used to sort base[] */
#	ifdef USE_HEAPSORT
		int sort_function;				/* index into sort_function[] */
#	endif /* USE_HEAPSORT */
	int strip_bogus;
	int thread_articles;				/* threading system for viewing articles */
	int thread_perc;				/* how close the match needs to be for THREAD_PERC to recognize two articles as the same thread */
	int thread_score;				/* how the score for threads is computed */
	int wildcard;						/* 0=wildmat, 1=regex */
	int score_limit_kill;					/* score limit to kill articles */
	int score_limit_select;					/* score limit to select articles */
	int score_kill;						/* default score for "kill" filter rules */
	int score_select;					/* default score for "hot" filter rules */
	int trim_article_body;				/* remove unnecessary blank lines */
	int show_help_mail_sign;			/* show help/mail sign in level title */
#	ifdef HAVE_COLOR
		int col_back;						/* standard background color */
		int col_from;						/* color of sender (From:) */
		int col_head;						/* color of headerlines */
		int col_help;						/* color of help pages */
		int col_invers_bg;					/* color of inverse text (background) */
		int col_invers_fg;					/* color of inverse text (foreground) */
		int col_minihelp;					/* color of mini help menu */
		int col_normal;						/* standard foreground color */
		int col_markdash;					/* text highlighting with _underdashes_ */
		int col_markstar;					/* text highlighting with *stars* */
		int col_markslash;					/* text highlighting with /slashes/ */
		int col_markstroke;					/* text highlighting with -strokes- */
		int col_message;					/* color of message lines at bottom */
		int col_newsheaders;				/* color of actual news header fields */
		int col_quote;						/* color of quotelines */
		int col_quote2;						/* color of twice quoted lines */
		int col_quote3;						/* color of >=3 times quoted lines */
		int col_extquote;					/* color of quoted external text */
		int col_response;					/* color of response counter */
		int col_signature;					/* color of signature */
		int col_urls;						/* color of urls highlight */
		int col_verbatim;					/* color of verbatim blocks */
		int col_subject;					/* color of article subject */
		int col_text;						/* color of textlines */
		int col_title;						/* color of Help/Mail-Sign */
#	endif /* HAVE_COLOR */
	int word_h_display_marks;			/* display * or _ when highlighting or space or nothing */
	int mono_markdash;				/* attribute for text highlighting with _underdashes_ */
	int mono_markstar;				/* attribute for text highlighting with *stars* */
	int mono_markslash;				/* attribute for text highlighting with /slashes/ */
	int mono_markstroke;				/* attribute for text highlighting with -strokes- */
	t_bool word_highlight;				/* like word_highlight but stored in tinrc */
	t_bool url_highlight;				/* highlight urls in text bodies */
	int wrap_column;				/* screen column to wrap of text messages */
#	ifdef HAVE_COLOR
		t_bool use_color;					/* like use_color but stored in tinrc */
#	endif /* HAVE_COLOR */
	t_bool abbreviate_groupname;		/* abbreviate groupnames like n.s.readers */
	t_bool add_posted_to_filter;
	t_bool advertising;
	t_bool alternative_handling;
	int auto_cc_bcc;					/* add your name to cc/bcc automatically */
	t_bool auto_list_thread;			/* list thread when entering it using right arrow */
	t_bool auto_reconnect;				/* automatically reconnect to news server */
	t_bool batch_save;					/* save arts if -M/-S command line switch specified */
	t_bool beginner_level;				/* beginner level (shows mini help a la elm) */
	t_bool cache_overview_files;		/* create local index files for NNTP overview files */
	t_bool catchup_read_groups;			/* ask if read groups are to be marked read */
	int confirm_choice;				/* what has to be confirmed */
	t_bool draw_arrow;					/* draw -> or highlighted bar */
	t_bool force_screen_redraw;			/* force screen redraw after external (shell) commands */
	t_bool group_catchup_on_exit;		/* catchup group with left arrow key or not */
	t_bool info_in_last_line;
	t_bool inverse_okay;
	t_bool keep_dead_articles;			/* keep all dead articles in dead.articles */
	char posted_articles_file[PATH_LEN];		/* if set, file in which to keep posted articles */
	t_bool mail_8bit_header;			/* allow 8bit chars. in header of mail message */
	t_bool mark_ignore_tags;			/* Ignore tags for GROUP_MARK_THREAD_READ/THREAD_MARK_ARTICLE_READ */
	t_bool mark_saved_read;				/* mark saved article/thread as read */
	t_bool pos_first_unread;			/* position cursor at first/last unread article */
	t_bool post_8bit_header;			/* allow 8bit chars. in header when posting to newsgroup */
	t_bool post_process_view;			/* set TRUE to invoke mailcap viewer app */
#	ifndef DISABLE_PRINTING
		t_bool print_header;				/* print all of mail header or just Subject: & From lines */
#	endif /* !DISABLE_PRINTING */
	t_bool process_only_unread;			/* save/print/mail/pipe unread/all articles */
	t_bool prompt_followupto;			/* display empty Followup-To header in editor */
	int quote_style;					/* quoting behaviour */
	t_bool show_description;
	t_bool show_only_unread_arts;		/* show only new/unread arts or all arts */
	t_bool show_only_unread_groups;		/* set TRUE to see only subscribed groups with new news */
	t_bool show_signatures;				/* show signatures when displaying articles */
	t_bool sigdashes;					/* set TRUE to prepend every signature with dashes */
	t_bool signature_repost;			/* set TRUE to add signature when reposting articles */
#	ifndef USE_CURSES
		t_bool strip_blanks;
#	endif /* !USE_CURSES */
	t_bool strip_newsrc;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		t_bool suppress_soft_hyphens;	/* set TRUE to remove soft hyphens (U+00AD) from articles */
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_bool tex2iso_conv;			/* convert "a to Umlaut-a */
	t_bool thread_catchup_on_exit;		/* catchup thread with left arrow key or not */
	t_bool unlink_article;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		t_bool utf8_graphics;				/* use utf-8 characters for line drawing */
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_bool verbatim_handling;			/* Detection of verbatim blocks */
#	ifdef HAVE_COLOR
		t_bool extquote_handling;		/* Detection of quoted text from external sources */
#	endif /* HAVE_COLOR */
	char inews_prog[PATH_LEN];
#	ifdef USE_CANLOCK
		int cancel_lock_algo;			/* algorithm used for Cancel-Lock/Cancel-Key */
#	endif /* USE_CANLOCK */
	int interactive_mailer;			/* invoke user's mailreader */
	t_bool use_mouse;					/* enables/disables mouse support under xterm */
#	ifdef HAVE_KEYPAD
		t_bool use_keypad;
#	endif /* HAVE_KEYPAD */
	t_bool wrap_on_next_unread;		/* Wrap around threads when searching next unread article */
	t_bool ask_for_metamail;			/* enables/disables the viewer query if a MIME message is going to be displayed */
	t_bool default_filter_kill_case;
	t_bool default_filter_kill_expire;
	t_bool default_filter_kill_global;
	t_bool default_filter_select_case;
	t_bool default_filter_select_expire;
#	ifdef XFACE_ABLE
		t_bool use_slrnface;			/* Use the slrnface program to display 'X-Face:'s */
#	endif /* XFACE_ABLE */
	t_bool default_filter_select_global;
	char select_format[LEN];		/* format string for the selection level */
	char group_format[LEN];			/* format string for the group level */
	char thread_format[LEN];		/* format string for the thread level */
	char date_format[LEN];			/* format string for the date display in the page header */
#	ifdef HAVE_UNICODE_NORMALIZATION
		int normalization_form;
#	endif /* HAVE_UNICODE_NORMALIZATION */
#	if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
		t_bool render_bidi;
#	endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
#	ifdef CHARSET_CONVERSION
		int attrib_mm_network_charset;
		char attrib_undeclared_charset[LEN];
#	endif /* !CHARSET_CONVERSION */
	char attrib_editor_format[PATH_LEN];
	char attrib_fcc[PATH_LEN];
	char attrib_maildir[PATH_LEN];
	char attrib_from[HEADER_LEN];
	char attrib_mailing_list[HEADER_LEN];
	char attrib_organization[LEN];
	char attrib_followup_to[LEN];
	char attrib_mime_types_to_save[LEN];
	char attrib_news_headers_to_display[LEN];
	char attrib_news_headers_to_not_display[LEN];
	char attrib_news_quote_format[LEN];
	char attrib_quote_chars[LEN];
	char attrib_sigfile[PATH_LEN];
	char attrib_savedir[PATH_LEN];
	char attrib_savefile[PATH_LEN];
	char attrib_x_body[LEN];
	char attrib_x_headers[HEADER_LEN];
#	ifdef HAVE_ISPELL
		char attrib_ispell[PATH_LEN];
#	endif /* HAVE_ISPELL */
	char attrib_quick_kill_scope[LEN];
	char attrib_quick_select_scope[LEN];
	char attrib_group_format[LEN];
	char attrib_thread_format[LEN];
	char attrib_date_format[LEN];
	int attrib_trim_article_body;
	int attrib_auto_cc_bcc;
	int attrib_quick_kill_header;
	int attrib_quick_select_header;
	int attrib_mail_mime_encoding;
#	if defined(HAVE_ALARM) && defined(SIGALRM)
		/*
		 * # seconds after which a read from the NNTP will timeout
		 * NB: This is different from the NNTP server timing us out due to inactivity
		 */
		int nntp_read_timeout_secs;
#	endif /* HAVE_ALARM && SIGALRM */
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
	t_bool attrib_sigdashes;
	t_bool attrib_signature_repost;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		t_bool attrib_suppress_soft_hyphens;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_bool attrib_tex2iso_conv;
	t_bool attrib_thread_catchup_on_exit;
	t_bool attrib_verbatim_handling;
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
