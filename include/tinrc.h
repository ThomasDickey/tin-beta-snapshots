/*
 *  Project   : tin - a Usenet reader
 *  Module    : tinrc.h
 *  Author    : Jason Faultless <jason@radar.tele2.co.uk>
 *  Created   :
 *  Updated   : 2000-01-03
 *  Notes     :
 *
 * Copyright (c) 1999-2002 Jason Faultless <jason@radar.tele2.co.uk>
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
 *    This product includes software developed by Jason Faultless.
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


/*
 * These are the tin defaults read from the tinrc file
 * If you change this, ensure you change the initial values in init.c
 *
 * FIXME: most default_* could/should be stored in the .inputhistory
 *        and could be nuked if tin comes with a prefilled .inputhistory
 *        which is installed automatically if no .inputhistory is found.
 */

#ifndef TINRC_H
#	define TINRC_H 1

struct t_config {
	/*
	 * Chars used to show article status
	 */
	char art_marked_deleted;
	char art_marked_inrange;
	char art_marked_return;
	char art_marked_selected;
	char art_marked_recent;
	char art_marked_unread;
	char art_marked_read;
	char art_marked_killed;
	char art_marked_read_selected;
	char editor_format[PATH_LEN];		/* editor + parameters  %E +%N %F */
	char default_goto_group[HEADER_LEN];		/* default for the 'g' command */
	char default_mail_address[HEADER_LEN];
	char mailer_format[PATH_LEN];		/* mailer + parameters  %M %S %T %F */
#ifndef DONT_HAVE_PIPING
	char default_pipe_command[LEN];
#endif /* DONT_HAVE_PIPING */
	char default_post_newsgroups[HEADER_LEN];	/* default newsgroups to post to */
	char default_post_subject[LEN];	/* default subject when posting */
#ifndef DISABLE_PRINTING
	char printer[LEN];					/* printer program specified from tinrc */
#endif /* !DISABLE_PRINTING */
	char default_range_group[LEN];
	char default_range_select[LEN];
	char default_range_thread[LEN];
	char default_regex_pattern[LEN];
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
#ifndef CHARSET_CONVERSION
	char mm_charset[LEN];				/* MIME charset */
#else
	int mm_network_charset;				/* MIME charset */
#endif /* !CHARSET_CONVERSION */
	char mm_local_charset[LEN];		/* display charset, not a rc/Menu-option anymore -> should be moved elsewhere */
#ifdef HAVE_ICONV_OPEN_TRANSLIT
	t_bool translit;						/* use //TRANSLIT */
#endif /* HAVE_ICONV_OPEN_TRANSLIT */
	char news_headers_to_display[LEN];	/* which headers to display */
	char news_headers_to_not_display[LEN];	/* which headers to not display */
	char news_quote_format[LEN];
	char quote_chars[LEN];			/* quote chars for posting/mails ": " (size matches prefixbuf in copy_body() */
#ifdef HAVE_COLOR
	char quote_regex[LEN];				/* regex used to determine quoted lines */
	char quote_regex2[LEN];				/* regex used to determine twice quoted lines */
	char quote_regex3[LEN];				/* regex used to determine >=3 times quoted lines */
#endif /* HAVE_COLOR */
	char sigfile[PATH_LEN];
	char strip_re_regex[LEN];			/* regex used to find and remove 'Re:'-like strings */
	char strip_was_regex[LEN];			/* regex used to find and remove '(was:.*'-like strings */
	char savedir[PATH_LEN];				/* directory to save articles to */
	char spamtrap_warning_addresses[LEN];
	char xpost_quote_format[LEN];
	int filter_days;					/* num of days an article filter can be active */
	int default_filter_kill_header;
	int default_filter_select_header;
	int default_move_group;
	int default_save_mode;				/* Append/Overwrite existing file when saving */
	int getart_limit;					/* number of article to get */
	int recent_time;				/* Time limit when article is "fresh" */
	int groupname_max_length;			/* max len of group names to display on screen */
	int kill_level;						/* Define how killed articles are shown */
	int mail_mime_encoding;
	int post_mime_encoding;
	int post_process;					/* type of post processing to be performed */
	int reread_active_file_secs;		/* reread active file interval in seconds */
	int scroll_lines;					/* # lines to scroll by in pager */
	int show_author;					/* show_author value from 'M' menu in tinrc */
	int sort_article_type;				/* method used to sort arts[] */
	int sort_threads_type;				/* method used to sort base[] */
	int strip_bogus;
	int thread_articles;				/* threading system for viewing articles */
	int wildcard;						/* 0=wildmat, 1=regex */
#ifdef HAVE_COLOR
	int col_back;						/* standard background color */
	int col_from;						/* color of sender (From:) */
	int col_head;						/* color of headerlines */
	int col_help;						/* color of help pages */
	int col_invers_bg;					/* color of inverse text (background) */
	int col_invers_fg;					/* color of inverse text (foreground) */
	int col_minihelp;					/* color of mini help menu*/
	int col_normal;						/* standard foreground color */
	int col_markdash;					/* text highlighting with _underdashes_ */
	int col_markstar;					/* text highlighting with *stars* */
	int col_message;					/* color of message lines at bottom */
	int col_newsheaders;				/* color of actual news header fields */
	int col_quote;						/* color of quotelines */
	int col_quote2;						/* color of twice quoted lines */
	int col_quote3;						/* color of >=3 times quoted lines */
	int col_response;					/* color of respone counter */
	int col_signature;					/* color of signature */
	int col_subject;					/* color of article subject */
	int col_text;						/* color of textlines*/
	int col_title;						/* color of Help/Mail-Sign */
	int word_h_display_marks;			/* display * or _ when highlighting or space or nothing*/
	t_bool word_highlight;				/* like word_highlight but stored in tinrc */
	t_bool use_color;					/* like use_color but stored in tinrc */
#endif /* HAVE_COLOR */
	t_bool add_posted_to_filter;
	t_bool advertising;
	t_bool alternative_handling;
	t_bool auto_bcc;					/* add your name to bcc automatically */
	t_bool auto_cc;						/* add your name to cc automatically */
	t_bool auto_list_thread;			/* list thread when entering it using right arrow */
	t_bool auto_reconnect;				/* automatically reconnect to news server */
	t_bool auto_save;					/* save thread with name from Archive-name: field */
	t_bool batch_save;					/* save arts if -M/-S command line switch specified */
	t_bool beginner_level;				/* beginner level (shows mini help a la elm) */
	t_bool cache_overview_files;		/* create local index files for NNTP overview files */
	t_bool catchup_read_groups;			/* ask if read groups are to be marked read */
	t_bool confirm_action;
	t_bool confirm_to_quit;
	t_bool draw_arrow;					/* draw -> or highlighted bar */
	t_bool force_screen_redraw;			/* force screen redraw after external (shell) commands */
	t_bool group_catchup_on_exit;		/* catchup group with left arrow key or not */
	t_bool info_in_last_line;
	t_bool inverse_okay;
	t_bool keep_dead_articles;			/* keep all dead articles in dead.articles */
	t_bool keep_posted_articles;		/* keep all posted articles in ~/Mail/posted */
	char keep_posted_articles_file[LEN];		/* file, to keep posted articles */
	t_bool mail_8bit_header;			/* allow 8bit chars. in header of mail message */
	t_bool mark_saved_read;				/* mark saved article/thread as read */
	t_bool pgdn_goto_next;
	t_bool pos_first_unread;			/* position cursor at first/last unread article */
	t_bool post_8bit_header;			/* allow 8bit chars. in header when posting to newsgroup */
	t_bool post_process_view;			/* set TRUE to invoke mailcap viewer app */
#ifndef DISABLE_PRINTING
	t_bool print_header;				/* print all of mail header or just Subject: & From lines */
#endif /* !DISABLE_PRINTING */
	t_bool process_only_unread;			/* save/print//mail/pipe unread/all articles */
	t_bool prompt_followupto;			/* display empty Followup-To header in editor */
	t_bool quote_empty_lines;			/* quote empty lines, too */
	t_bool quote_signatures;			/* quote signatures */
	t_bool show_description;
	t_bool show_lines;
	t_bool show_only_unread_arts;		/* show only new/unread arts or all arts */
	t_bool show_only_unread_groups;		/* set TRUE to see only subscribed groups with new news */
	t_bool show_score;
	t_bool show_signatures;				/* show signatures when displaying articles */
	t_bool hide_uue;					/* set TRUE to treat uuencoded data as an attachment */
	t_bool sigdashes;					/* set TRUE to prepend every signature with dashes */
	t_bool signature_repost;			/* set TRUE to add signature when reposting articles */
	t_bool space_goto_next_unread;
	t_bool start_editor_offset;
	t_bool strip_blanks;
	t_bool strip_newsrc;
	t_bool tab_after_X_selection;		/* set TRUE if you want auto TAB after X */
	t_bool tab_goto_next_unread;
	t_bool tex2iso_conv;			/* convert "a to Umlaut-a */
	t_bool thread_catchup_on_exit;		/* catchup thread with left arrow key or not */
	t_bool unlink_article;
	char inews_prog[PATH_LEN];
	t_bool use_getart_limit;
	t_bool use_mailreader_i;			/* invoke user's mailreader earlier to use more of its features (i = interactive) */
	t_bool use_mouse;					/* enables/disables mouse support under xterm */
#ifdef HAVE_KEYPAD
	t_bool use_keypad;
#endif /* HAVE_KEYPAD */
#ifdef HAVE_METAMAIL
	t_bool ask_for_metamail;			/* enables/disables the metamail query if a MIME message is going to be displayed */
	t_bool use_metamail;				/* enables/disables metamail on MIME messages */
#endif /* HAVE_METAMAIL */
	t_bool default_filter_kill_case;
	t_bool default_filter_kill_expire;
	t_bool default_filter_kill_global;
	t_bool default_filter_select_case;
	t_bool default_filter_select_expire;
	t_bool default_filter_select_global;
};

#endif /* !TINRC_H */
