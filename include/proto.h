/*
 *  Project   : tin - a Usenet reader
 *  Module    : proto.h
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   :
 *  Updated   : 2003-03-14
 *  Notes     :
 *
 * Copyright (c) 1997-2003 Urs Janssen <urs@tin.org>
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


#ifndef PROTO_H
#	define PROTO_H 1

#	ifndef RFC2046_H
#		include <rfc2046.h>
#	endif /* !RFC2046_H */

/* This fixes ambiguities on platforms that don't distinguish extern case */
#ifdef CASE_PROBLEM
#	define Raw tin_raw
#	define EndWin tin_EndWin
#endif /* CASE_PROBLEM */

/* active.c */
extern char group_flag(char ch);
extern int get_active_num(void);
extern t_bool match_group_list(const char *group, const char *group_list);
extern t_bool parse_active_line(char *line, long *max, long *min, char *moderated);
extern t_bool process_bogus(char *name);
extern t_bool need_reread_active_file(void);
extern t_bool resync_active_file(void);
extern void create_save_active_file(void);
extern void load_newnews_info(char *info);
extern void read_news_active_file(void);

/* art.c */
extern char *find_nov_file(struct t_group *group, int mode);
extern int global_get_multipart_info(int aindex, MultiPartInfo *setme);
extern t_bool index_group(struct t_group *group);
extern void do_update(t_bool catchup);
extern void find_base(struct t_group *group);
extern void make_threads(struct t_group *group, t_bool rethread);
extern void set_article(struct t_article *art);
extern void show_art_msg(const char *group);
extern void sort_arts(unsigned /* int */ sort_art_type);
extern void write_nov_file(struct t_group *group);

/* attrib.c */
extern void read_attributes_file(t_bool global_file);
extern void write_attributes_file(const char *file);

/* auth.c */
#ifdef NNTP_ABLE
	extern t_bool authenticate(char *server, char *user, t_bool startup);
#endif /* NNTP_ABLE */

/* charset.c */
extern char *convert_to_printable(char *buf);
extern t_bool is_art_tex_encoded(FILE *fp);
extern void convert_body2printable(char* buf);
extern void convert_iso2asc(char *iso, char **asc_buffer, int *max_line_len, int t);
extern void convert_tex2iso(char *from, char *to);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern wchar_t *wconvert_to_printable(wchar_t *wbuf);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

/* color.c */
#ifdef HAVE_COLOR
	extern void bcol(int color);
	extern void fcol(int color);
#endif /* HAVE_COLOR */
extern void draw_pager_line(const char *str, int flags);

/* config.c */
extern char **ulBuildArgv(char *cmd, int *new_argc);
extern char *fmt_option_prompt(char *dst, int len, t_bool editing, int option);
extern char *quote_space_to_dash(char *str);
extern const char *print_boolean(t_bool value);
extern int change_config_file(struct t_group *group);
extern int option_row(int option);
extern t_bool match_boolean(char *line, const char *pat, t_bool *dst);
extern t_bool match_integer(char *line, const char *pat, int *dst, int maxval);
extern t_bool match_list(char *line, constext *pat, constext *const *table, size_t tablelen, int *dst);
extern t_bool match_long(char *line, const char *pat, long *dst);
extern t_bool match_string(char *line, const char *pat, char *dst, size_t dstlen);
extern t_bool read_config_file(char *file, t_bool global_file);
extern void quote_dash_to_space(char *str);
extern void refresh_config_page(int act_option);
extern void show_menu_help(const char *help_message);
extern void write_config_file(char *file);

/* cook.c */
extern const char *get_filename(t_param *ptr);
extern t_bool cook_article(t_bool wrap_lines, t_openartinfo *artinfo, int tabs, t_bool uue);
extern t_bool expand_ctrl_chars(char *to, const char *from, int length, size_t cook_width);

/* curses.c */
extern OUTC_RETTYPE outchar(OUTC_ARGS);
extern int InitScreen(void);
extern int RawState(void);
extern int ReadCh(void);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern wint_t ReadWch(void);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
extern int get_arrow_key(int prech);
extern int get_termcaps(void);
extern void ClearScreen(void);
extern void CleartoEOLN(void);
extern void CleartoEOS(void);
extern void ScrollScreen(int lines_to_scroll);
extern void SetScrollRegion(int topline, int bottomline);
extern void EndInverse(void);
extern void EndWin(void);
extern void InitWin(void);
#ifndef USE_CURSES
	extern void MoveCursor(int row, int col);
#endif /* USE_CURSES */
extern void Raw(int state);
extern void StartInverse(void);
extern void cursoroff(void);
extern void cursoron(void);
extern void highlight_string(int row, int col, int size);
extern void set_keypad_off(void);
extern void set_keypad_on(void);
extern void set_xclick_off(void);
extern void set_xclick_on(void);
extern void setup_screen(void);
extern void word_highlight_string(int row, int col, int size, int color);
#if 0
	extern void ToggleInverse(void);
#endif /* 0 */

/* debug.c */
#ifdef DEBUG
	extern void debug_delete_files(void);
	extern void debug_nntp(const char *func, const char *line);
	extern void debug_print_active(void);
	extern void debug_print_arts(void);
	extern void debug_print_filters(void);
	extern void debug_print_header(struct t_article *s);
	extern void debug_save_comp(void);
	extern void vDbgPrintMalloc(int iIsMalloc, const char *pcFile, int iLine, size_t iSize);
#endif /* DEBUG */
#ifdef DEBUG_NEWSRC
	extern void debug_print_newsrc(struct t_newsrc *NewSrc, FILE *fp);
#endif /* DEBUG_NEWSRC */
#if defined(DEBUG) || defined(DEBUG_NEWSRC)
	extern void debug_print_bitmap(struct t_group *group, struct t_article *art);
	extern void debug_print_comment(const char *comment);
#endif /* DEBUG || DEBUG_NEWSRC */

/* envarg.c */
extern void envargs(int *Pargc, char ***Pargv, const char *envstr);

/* feed.c */
extern void feed_articles(int function, int level, struct t_group *group, int respnum);

/* filter.c */
extern int unfilter_articles(void);
extern t_bool filter_articles(struct t_group *group);
extern t_bool filter_menu(int type, struct t_group *group, struct t_article *art);
extern t_bool quick_filter(int type, struct t_group *group, struct t_article *art);
extern t_bool quick_filter_select_posted_art(struct t_group *group, const char *subj, const char *a_message_id);
extern t_bool read_filter_file(const char *file);
extern void free_filter_array(struct t_filters *ptr);
extern void refresh_filter_menu(void);
extern void write_filter_file(const char *filename);

/* getline.c */
extern char *tin_getline(const char *prompt, int number_only, const char *str, int max_chars, t_bool passwd, int which_hist);

/* global.c */
extern void bug_report(void);
extern void move_up(void);
extern void move_down(void);
extern void page_up(void);
extern void page_down(void);
extern void top_of_list(void);
extern void end_of_list(void);
extern void move_to_item(int n);
extern void prompt_item_num(int ch, const char *prompt);
extern void set_first_screen_item(void);

/* group.c */
extern int find_new_pos(int old_top, long old_artnum, int cur_pos);
extern int group_page(struct t_group *group);
extern void clear_note_area(void);
extern void mark_screen(int level, int screen_row, int screen_col, const char *value);
extern void pos_first_unread_thread(void);
extern void set_subj_from_size(int num_cols);
extern void show_group_page(void);
extern void toggle_subject_from(void);

/* hashstr.c */
extern char *hash_str(const char *s);
extern void hash_init(void);
extern void hash_reclaim(void);

/* help.c */
extern void show_help_page(const int level, const char *title);
extern void show_mini_help(int level);
extern void toggle_mini_help(int level);
#ifdef USE_CURSES
	extern void display_help_page(t_bool first);
#else
	extern void display_help_page(void);
#endif /* USE_CURSES */

/* header.c */
extern const char *get_domain_name(void);
extern const char *get_fqdn(const char *host);
extern const char *get_host_name(void);
#ifndef FORGERY
	extern char *build_sender(void);
#endif /* !FORGERY */

/* inews.c */
extern t_bool submit_news_file(char *name, struct t_group *group, char *a_message_id);
extern void get_from_name(char *from_name, struct t_group *thisgrp);
extern void get_user_info(char *user_name, char *full_name);

/* init.c */
extern t_bool(*wildcard_func)(const char *str, char *patt, t_bool icase);		/* Wildcard matching function */
extern t_bool create_mail_save_dirs(void);
extern void init_selfinfo(void);
extern void postinit_regexp(void);
extern void set_up_private_index_cache(void);
#ifdef HAVE_COLOR
	extern void postinit_colors(void);
#endif /* HAVE_COLOR */
#ifdef USE_INN_NNTPLIB
	extern char *GetConfigValue(const char *name);
#endif /* USE_INN_NNTPLIB */

/* joinpath.c */
extern void joinpath(char *result, const char *dir, const char *file);

/* keymap.c */
extern char *printascii(char *buf, int ch);
extern t_bool read_keymap_file(void);
extern void build_keymaps(void);
extern void free_keymaps(void);

/* langinfo.c */
extern char *tin_nl_langinfo(nl_item item);

/* list.c */
extern char *random_organization(char *in_org);
extern int find_group_index(const char *group);
extern struct t_group *group_add(const char *group);
extern struct t_group *group_find(const char *group_name);
extern unsigned long hash_groupname(const char *group);
extern void group_rehash(t_bool yanked_out);
extern void init_group_hash(void);

/* lock.c */
extern int fd_lock(int fd, t_bool block);
extern int fd_unlock(int fd);
extern t_bool dot_lock(const char *filename);
extern t_bool dot_unlock(const char *filename);
#if 0 /* unused */
	extern int test_fd_lock(int fd);
#endif /* 0 */

/* mail.c */
extern t_bool art_edit(struct t_group *group, struct t_article *article);
extern void find_art_max_min(const char *group_path, long *art_max, long *art_min);
extern void print_active_head(const char *active_file);
extern void print_group_line(FILE *fp, const char *group_name, long art_max, long art_min, const char *base_dir);
extern void read_descriptions(t_bool verb);
extern void grp_del_mail_arts(struct t_group *group);
extern void grp_del_mail_art(struct t_article *article);
#ifdef HAVE_MH_MAIL_HANDLING
	extern void read_mail_active_file(void);
	extern void write_mail_active_file(void);
#endif /* HAVE_MH_MAIL_HANDLING */

/* mimetypes.c */
extern void lookup_mimetype(const char *ext, t_part *part);

/* main.c */
extern int main(int argc, char *argv[]);
extern int read_cmd_line_groups(void);
extern void giveup(void);

/* memory.c */
extern void expand_active(void);
extern void expand_art(void);
extern void expand_newnews(void);
extern void expand_save(void);
extern void init_alloc(void);
extern void free_all_arrays(void);
extern void free_art_array(void);
extern void free_attributes_array(void);
extern void free_save_array(void);
extern void *my_malloc1(const char *file, int line, size_t size);
extern void *my_calloc1(const char *file, int line, size_t nmemb, size_t size);
extern void *my_realloc1(const char *file, int line, void *p, size_t size);
#ifndef USE_CURSES
	extern void init_screen_array(t_bool allocate);
#endif /* !USE_CURSES */

/* misc.c */
extern const char *eat_re(char *s, t_bool eat_was);
extern char *escape_shell_meta(const char *source, int quote_area);
extern char *get_tmpfilename(const char *filename);
extern char *quote_wild(char *str);
extern char *quote_wild_whitespace(char *str);
extern char *strip_line(char *line);
extern const char *get_val(const char *env, const char *def);
extern const char *gnksa_strerror(int errcode);
extern int gnksa_check_from(char *from);
extern int gnksa_split_from(const char *from, char *address, char *realname, int *addrtype);
extern int get_initials(int respnum, char *s, int maxsize);
extern int gnksa_do_check_from(const char *from, char *address, char *realname);
extern int my_chdir(char *path);
extern int my_isprint(int c);
extern int my_mkdir(char *path, mode_t mode);
extern int strfmailer(char *the_mailer, char *subject, char *to, const char *filename, char *s, size_t maxsize, char *format);
extern int strfpath(const char *format, char *str, size_t maxsize, struct t_group *group);
extern int strfquote(const char *group, int respnum, char *s, size_t maxsize, char *format);
extern long file_mtime(const char *file);
extern long file_size(const char *file);
extern t_bool backup_file(const char *filename, const char *backupname);
extern t_bool copy_fp(FILE *fp_ip, FILE *fp_op);
extern t_bool invoke_cmd(const char *nam);
extern t_bool invoke_editor(const char *filename, int lineno);
extern t_bool mail_check(void);
extern void append_file(char *old_filename, char *new_filename);
extern void asfail(const char *file, int line, const char *cond);
extern void base_name(char *fullpath, char *program);
extern void cleanup_tmp_files(void);
extern void copy_body(FILE *fp_ip, FILE *fp_op, char *prefix, char *initl, t_bool raw_data);
extern void create_index_lock_file(char *the_lock_file);
extern void draw_percent_mark(long cur_num, long max_num);
extern void get_author(t_bool thread, struct t_article *art, char *str, size_t len);
extern void get_cwd(char *buf);
extern void make_base_group_path(const char *base_dir, const char *group_name, char *group_path);
extern void make_group_path(const char *name, char *path);
extern void read_input_history_file(void);
extern void rename_file(const char *old_filename, const char *new_filename);
extern void show_inverse_video_status(void);
extern void strip_double_ngs(char *ngs_list);
extern void strip_name(char *the_address, char *stripped_address);
extern void tin_done(int ret);
extern void toggle_inverse_video(void);
extern int parse_from(const char *from, char *address, char *realname);
#if defined(LOCAL_CHARSET) || defined(CHARSET_CONVERSION)
	extern void buffer_to_network(char *line, int mmnwcharset);
#endif /* LOCAL_CHARSET || CHARSET_CONVERSION */

extern void process_charsets(char **line, int *max_line_len, const char *network_charset, const char *local_charset, t_bool conv_tex2iso);
#ifdef HAVE_COLOR
	extern t_bool toggle_color(void);
	extern void show_color_status(void);
#endif /* HAVE_COLOR */
#ifdef HAVE_ISPELL
	extern t_bool invoke_ispell(const char *nam, struct t_group *group);
#endif /* HAVE_ISPELL */
#ifndef M_UNIX
	extern void make_post_process_cmd(char *cmd, char *dir, char *file);
#endif /* !M_UNIX */
#ifndef NO_SHELL_ESCAPE
	extern void shell_escape(void);
	extern void do_shell_escape(void);
#endif /* !NO_SHELL_ESCAPE */

/* newsrc.c */
extern int pos_group_in_newsrc(struct t_group *group, int pos);
extern signed long int read_newsrc(char *newsrc_file, t_bool allgroups);
extern signed long int write_newsrc(void);
extern void art_mark(struct t_group *group, struct t_article *art, int flag);
extern void backup_newsrc(void);
extern void catchup_newsrc_file(void);
extern void delete_group(char *group);
extern void expand_bitmap(struct t_group *group, long min);
extern void grp_mark_read(struct t_group *group, struct t_article *art);
extern void grp_mark_unread(struct t_group *group);
extern void parse_unread_arts(struct t_group *group);
extern void reset_newsrc(void);
extern void subscribe(struct t_group *group, int sub_state);
extern void thd_mark_read(struct t_group *group, long thread);
extern void thd_mark_unread(struct t_group *group, long thread);
extern void set_default_bitmap(struct t_group *group);
extern void art_mark_deleted(struct t_article *art);
extern void art_mark_undeleted(struct t_article *art);
#ifdef DEBUG_NEWSRC
	extern void vNewsrcTestHarness(void);
#endif /* DEBUG_NEWSRC */

/* nntplib.c */
extern FILE *get_nntp_fp(FILE *fp);
extern FILE *get_nntp_wr_fp(FILE *fp);
extern char *getserverbyfile(const char *file);
extern char *get_server(char *string, int size);
extern int server_init(char *machine, const char *service, int port, char *text, size_t mlen);
extern void close_server(void);
extern void put_server(const char *string);
extern void u_put_server(const char *string);
#ifdef DEBUG
	extern const char *nntp_respcode(int respcode);
#endif /* DEBUG */

/* nrctbl.c */
extern int get_newsrcname(char *newsrc_name, const char *nntpserver_name);
extern void get_nntpserver(char *nntpserver_name, char *nick_name);

/* open.c */
extern FILE *open_art_fp(const char *group_path, long art);
extern FILE *open_newgroups_fp(int the_index);
extern FILE *open_news_active_fp(void);
extern FILE *open_newsgroups_fp(void);
extern FILE *open_overview_fmt_fp(void);
extern FILE *open_subscription_fp(void);
extern FILE *open_xover_fp(struct t_group *group, const char *mode, long min, long max);
extern FILE *open_art_header(long art);
extern int get_respcode(char *, size_t);
extern int get_only_respcode(char *, size_t);
extern int nntp_open(void);
extern int group_get_art_info(char *tin_spooldir, char *groupname, int grouptype, long *art_count, long *art_max, long *art_min);
extern long setup_hard_base(struct t_group *group);
extern t_bool stat_article(long art, const char *group_path);
extern void nntp_close(void);
extern void vGet1GrpArtInfo(struct t_group *grp);
#ifdef HAVE_MH_MAIL_HANDLING
	extern FILE *open_mail_active_fp(const char *mode);
	extern FILE *open_mailgroups_fp(void);
#endif /* HAVE_MH_MAIL_HANDLING */
#ifdef NNTP_ABLE
	extern FILE *nntp_command(const char *, int, char *, size_t);
#endif /* NNTP_ABLE */

/* page.c */
extern int show_page(struct t_group *group, int respnum, int *threadnum);
extern void display_info_page(int part);
extern void draw_page(const char *group, int part);
extern void info_pager(FILE *info_fh, const char *title, t_bool wrap_at_ends);
extern void resize_article(t_bool wrap_lines, t_openartinfo *artinfo);
extern void toggle_raw(struct t_group *group);

/* parsdate.y */
extern time_t parsedate(char *p, TIMEINFO *now);

/* pgp.c */
#ifdef HAVE_PGP_GPG
	extern t_bool pgp_check_article(t_openartinfo *artinfo);
	extern void init_pgp(void);
	extern void invoke_pgp_mail(const char *nam, char *mail_to);
	extern void invoke_pgp_news(char *the_article);
#endif /* HAVE_PGP_GPG */

/* plp_snprintf.c */
#ifndef HAVE_SNPRINTF
	extern int plp_snprintf(char *, size_t, const char *, ...);
#endif /* !HAVE_SNPRINTF */
#ifndef HAVE_VSNPRINTF
	extern int plp_vsnprintf(char *, size_t, const char *, va_list);
#endif /* !HAVE_VSNPRINTF */

/* post.c */
extern int count_postponed_articles(void);
extern int mail_to_author(const char *group, int respnum, t_bool copy_text, t_bool with_headers, t_bool raw_data);
extern int mail_to_someone(const char *address, t_bool confirm_to_mail, t_openartinfo *artinfo);
extern int post_response(const char *group, int respnum, t_bool copy_text, t_bool with_headers, t_bool raw_data);
extern int repost_article(const char *group, int respnum, t_bool supersede, t_openartinfo *artinfo);
extern t_bool cancel_article(struct t_group *group, struct t_article *art, int respnum);
extern t_bool mail_bug_report(void);
extern t_bool pickup_postponed_articles(t_bool ask, t_bool all);
extern t_bool post_article(const char *group);
extern t_bool reread_active_after_posting(void);
extern t_bool user_posted_messages(void);
extern void checknadd_headers(const char *infile);
extern void init_postinfo(void);
extern void quick_post_article(t_bool postponed_only);
#ifdef USE_CANLOCK
	extern const char *build_cankey(const char *messageid, const char *secret);
	extern const char *build_canlock(const char *messageid, const char *secret);
	extern char *get_secret(void);
#endif /* USE_CANLOCK */

/* prompt.c */
extern char *prompt_string_default(const char *prompt, char *def, const char *failtext, int history);
extern char *sized_message(const char *format, const char *subject);
extern int prompt_list(int row, int col, int var, constext *help_text, constext *prompt_text, constext *list[], int size);
extern int prompt_num(int ch, const char *prompt);
extern int prompt_yn(int line, const char *prompt, t_bool default_answer);
extern int prompt_msgid(void);
extern t_bool prompt_default_string(const char *prompt, char *buf, int buf_len, char *default_prompt, int which_hist);
extern t_bool prompt_menu_string(int line, const char *prompt, char *var);
extern t_bool prompt_option_char(int option);
extern t_bool prompt_option_num(int option);
extern t_bool prompt_option_string(int option);
extern t_bool prompt_string(const char *prompt, char *buf, int which_hist);
extern void prompt_continue(void);
extern void prompt_on_off(int row, int col, t_bool *var, constext *help_text, constext *prompt_text);

/* read.c */
extern char *tin_fgets(FILE *fp, t_bool header);
#ifdef NNTP_ABLE
	extern void drain_buffer(FILE *fp);
#endif /* NNTP_ABLE */

/* refs.c */
extern char *get_references(struct t_msgid *refptr);
extern struct t_msgid *find_msgid(const char *msgid);
extern void build_references(struct t_group *group);
extern void clear_art_ptrs(void);
extern void collate_subjects(void);
extern void free_msgids(void);
extern void thread_by_reference(void);

/* regex.c */
extern t_bool compile_regex(const char *regex, struct regex_cache *cache, int options);
extern t_bool match_regex(const char *string, char *pattern, t_bool icase);
extern void highlight_regexes(int row, struct regex_cache *regex, int color);

/* rfc1524.c */
extern t_mailcap *get_mailcap_entry(t_part *part, const char *path);
extern void free_mailcap(t_mailcap *tmailcap);

/* rfc2045.c */
extern void rfc1521_encode(char *line, FILE *f, int e);

/* rfc2046.c */
extern const char *get_param(t_param *list, const char *name);
extern char *parse_header(char *buf, const char *pat, t_bool decode);
extern int art_open(t_bool wrap_lines, struct t_article *art, const char *group_path, t_openartinfo *artinfo, t_bool show_progress_meter);
extern int content_type(char *type);
extern int parse_rfc822_headers(struct t_header *hdr, FILE *from, FILE *to);
extern t_part *new_part(t_part *part);
extern void art_close(t_openartinfo *artinfo);
extern void free_and_init_header(struct t_header *hdr);
extern void free_parts(t_part *ptr);
extern void unfold_header(char *line);

/* rfc2047.c */
extern char *rfc1522_decode(const char *s);
extern char *rfc1522_encode(char *s, const char *charset, t_bool ismail);
extern int mmdecode(const char *what, int encoding, int delimiter, char *where);
extern void rfc15211522_encode(const char *filename, constext *mime_encoding, struct t_group *group, t_bool allow_8bit_header, t_bool ismail);

/* save.c */
extern int check_start_save_any_news(int function, t_bool catchup);
extern t_bool add_to_save_list(struct t_article *art, const char *path);
extern t_bool create_path(const char *path);
extern t_bool post_process_files(int proc_type_ch, t_bool auto_delete);
extern t_bool save_art_to_file(int indexnum, t_openartinfo *artinfo);
extern t_bool save_batch(char type, const char *group_path);
extern void decode_save_mime(t_openartinfo *art, t_bool postproc);
extern void print_art_seperator_line(FILE *fp, t_bool is_mailbox);
extern void sort_save_list(void);

/* screen.c */
extern void center_line(int line, t_bool inverse, const char *str);
extern void clear_message(void);
extern void draw_arrow_mark(int line);
extern void erase_arrow(void);
extern void error_message(const char *fmt, ...);
extern void info_message(const char *fmt, ...);
extern void perror_message(const char *fmt, ...);
extern void ring_bell(void);
extern void show_progress(const char *txt, long count, long total);
extern void show_title(const char *title);
extern void spin_cursor(void);
extern void stow_cursor(void);
extern void wait_message(unsigned int sdelay, const char *fmt, ...);

/* search.c */
extern int get_search_vectors(int *start, int *end);
extern int search(int key, int current_art, t_bool forward, t_bool repeat);
extern int search_active(t_bool forward, t_bool repeat);
extern int search_article(t_bool forward, t_bool repeat, int start_line, int lines, t_lineinfo *line, int reveal_ctrl_l_lines, FILE *fp);
extern int search_config(t_bool forward, t_bool repeat, int current, int last);
extern int search_body(int current_art, t_bool repeat);

/* select.c */
extern int add_my_group(const char *group, t_bool add);
extern int choose_new_group(void);
extern int skip_newgroups(void);
extern void draw_group_arrow(void);
extern void selection_page(int start_groupnum, int num_cmd_line_groups);
extern void set_groupname_len(t_bool all_groups);
extern void show_selection_page(void);
extern void toggle_my_groups(const char *group);

/* sigfile.c */
extern void msg_write_signature(FILE *fp, t_bool flag, struct t_group *thisgroup);

/* signal.c */
extern RETSIGTYPE(*sigdisp (int sig, RETSIGTYPE (*func)(SIG_ARGS))) (SIG_ARGS);
extern t_bool set_win_size(int *num_lines, int *num_cols);
extern void allow_resize(t_bool allow);
extern void handle_resize(t_bool repaint);
extern void set_noteslines(int num_lines);
extern void set_signal_catcher(int flag);
extern void set_signal_handlers(void);

/* strftime.c */
extern size_t my_strftime(char *s, size_t maxsize, const char *format, struct tm *timeptr);

/* string.c */
extern char *eat_tab(char *s);
extern char *my_strdup(const char *str);
extern char *str_trim(char *string);
extern char *tin_ltoa(long value, int digits);
#if !defined(HAVE_STRCASESTR) || defined(DECL_STRCASESTR)
	extern const char *strcasestr(const char *haystack, const char *needle);
#endif /* !HAVE_STRCASESTR || DECL_STRCASESTR */
extern int sh_format(char *dst, size_t len, const char *fmt, ...);
extern size_t mystrcat(char **t, const char *s);
extern void my_strncpy(char *p, const char *q, size_t n);
extern void str_lwr(char *str);
#ifndef HAVE_STRPBRK
	extern char *strpbrk(char *str1, char *str2);
#endif /* !HAVE_STRPBRK */
#ifndef HAVE_STRSTR
	extern char *strstr(char *text, char *pattern);
#endif /* !HAVE_STRSTR */
#ifndef HAVE_STRCASECMP
	extern int strcasecmp(const char *p, const char *q);
#endif /* !HAVE STRCASECMP */
#ifndef HAVE_STRNCASECMP
	extern int strncasecmp(const char *p, const char *q, size_t n);
#endif /* !HAVE_STRNCASECMP */
#ifndef HAVE_ATOI
	extern int atoi(const char *s);
#endif /* !HAVE_ATOI */
#ifndef HAVE_ATOL
	extern long atol(const char *s);
#endif /* !HAVE_ATOL */
#ifndef HAVE_STRTOL
	extern long strtol(const char *str, char **ptr, int use_base);
#endif /* !HAVE STRTOL */
#ifndef HAVE_STRERROR
	extern char *my_strerror(int n);
#	define strerror(n) my_strerror(n)
#endif /* !HAVE_STRERROR */
#ifndef HAVE_STRRSTR
	extern char *my_strrstr(const char *str, const char *pat);
#	define strrstr(s,p)	my_strrstr(s,p)
#endif /* !HAVE_STRRSTR */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern void wcspart(wchar_t *to, const wchar_t *from, int columns, int size_to);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

/* tags.c */
extern int line_is_tagged(int n);
extern int tag_multipart(int base_index);
extern t_bool set_range(int level, int min, int max, int curr);
extern t_bool tag_article(int art);
extern t_bool untag_all_articles(void);
extern void do_auto_select_arts(void);
extern void remove_tag(long art);
extern void undo_auto_select_arts(void);
extern void undo_selections(void);

/* tmpfile.c */
#ifndef HAVE_TMPFILE
	extern FILE *tmpfile(void);
#endif /* !HAVE_TMPFILE */

/* my_tmpfile.c */
extern int my_tmpfile(char *filename, size_t name_size, t_bool need_name, const char *base_dir);

/* thread.c */
extern int find_response(int i, int n);
extern int get_score_of_thread(int n);
extern int new_responses(int thread);
extern int next_response(int n);
extern int next_thread(int n);
extern int next_unread(int n);
extern int num_of_responses(int n);
extern int prev_response(int n);
extern int prev_unread(int n);
extern int stat_thread(int n, struct t_art_stat *sbuf);
extern int which_response(int n);
extern int which_thread(int n);
extern int thread_page(struct t_group *group, int respnum, int thread_depth, t_pagerinfo *page);
extern void draw_line(int i, int magic);
extern void fixup_thread(int respnum, t_bool redraw);

/* wildmat.c */
extern t_bool wildmat(const char *text, char *p, t_bool icase);
extern t_bool wildmatpos(const char *text, char *p, t_bool icase, int *srch_offsets, int srch_offsets_size);

/* xref.c */
extern t_bool overview_xref_support(void);
extern void NSETRNG0(t_bitmap *bitmap, long low, long high);
extern void NSETRNG1(t_bitmap *bitmap, long low, long high);
extern void art_mark_xref_read(struct t_article *art);

#endif /* !PROTO_H */
