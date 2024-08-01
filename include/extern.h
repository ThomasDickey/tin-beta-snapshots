/*
 *  Project   : tin - a Usenet reader
 *  Module    : extern.h
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2024-07-29
 *  Notes     :
 *
 * Copyright (c) 1997-2024 Iain Lea <iain@bricbrac.de>
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


#ifndef EXTERN_H
#	define EXTERN_H 1

/*
 * Library prototypes
 */

#ifndef RFC2046_H
#	include <rfc2046.h>
#endif /* !RFC2046_H */
#ifndef KEYMAP_H
#	include "keymap.h"
#endif /* !KEYMAP_H */
#ifndef TNNTPS_H
#	include "tnntps.h"
#endif /* !TNNTPS_H */

/*
 * The prototypes bracketed by DECL_xxxx ifdef's are used to get moderately
 * clean compiles on systems with pre-ANSI/POSIX headers when compiler
 * warnings are enabled. (Not all of the functions are ANSI or POSIX).
 */
#ifdef DECL__FLSBUF
	extern int _flsbuf(int, FILE *);
#endif /* DECL__FLSBUF */
#ifdef DECL_ALARM
	extern unsigned alarm(unsigned);
#endif /* DECL_ALARM */
#ifdef DECL_ATOI
	extern int atoi(const char *);
#endif /* DECL_ATOI */
#ifdef DECL_ATOL
	extern long atol(const char *);
#endif /* DECL_ATOL */
#ifdef HAVE_ATOLL
#	ifdef DECL_ATOLL
		extern long long atoll(const char *);
#	endif /* DECL_ATOLL */
#else
#	if defined(HAVE_ATOQ) && defined(DECL_ATOQ)
		 extern long long atoq(const char *);
#	endif /* HAVE_ATOQ && DECL_ATOQ */
#endif /* HAVE_ATOLL */
#ifndef HAVE_MEMCPY
#	ifdef DECL_BCOPY
		extern int bcopy(char *, char *, int);
#	endif /* DECL_BCOPY */
#endif /* !HAVE_MEMCPY */

#if 0 /* trouble on Linux/gcc 3.1 */
#	ifdef DECL_BZERO /* FD_ZERO() might use this */
		extern void bzero(char *, int);
#	endif /* DECL_BZERO */
#endif /* 0 */

#ifdef DECL_CALLOC
	extern void *calloc(size_t, size_t);
#endif /* DECL_CALLOC */
#ifdef DECL_FCHMOD
	extern int fchmod(int, mode_t);
#endif /* DECL_FCHMOD */
#ifdef DECL_FCLOSE
	extern int fclose(FILE *);
#endif /* DECL_FCLOSE */
#ifdef DECL_FDOPEN
	extern FILE *fdopen(int, const char *);
#endif /* DECL_FDOPEN */
#ifdef DECL_FFLUSH
	extern int fflush(FILE *);
#endif /* DECL_FFLUSH */
#ifdef DECL_FGETC
	extern int fgetc(FILE *);
#endif /* DECL_FGETC */
#if defined(DECL_FILENO) && !defined(fileno)
	extern int fileno(FILE *);
#endif /* DECL_FILENO && !fileno */
#ifdef DECL_FPRINTF
	extern int fprintf(FILE *, const char *, ...);
#endif /* DECL_FPRINTF */
#ifdef DECL_FPUTC
	extern int fputc(int, FILE *);
#endif /* DECL_FPUTC */
#ifdef DECL_FPUTS
	extern int fputs(const char *, FILE *);
#endif /* DECL_FPUTS */
#ifdef DECL_FREAD
	extern size_t fread(void *, size_t, size_t, FILE *);
#endif /* DECL_FREAD */
#ifdef DECL_FREE
	extern void free(void *);
#endif /* DECL_FREE */
#ifdef DECL_FSEEK
	extern int fseek(FILE *, long, int);
#endif /* DECL_FSEEK */
#ifdef DECL_FWRITE
	extern size_t fwrite(void *, size_t, size_t, FILE *);
#endif /* DECL_FWRITE */
#ifdef DECL_GETCWD
	extern char *getcwd(char *, size_t);
#endif /* DECL_GETCWD */
#ifdef DECL_GETENV
	extern char *getenv(const char *);
#endif /* DECL_GETENV */
#ifdef DECL_GETHOSTBYNAME
	extern struct hostent *gethostbyname(const char *);
#endif /* DECL_GETHOSTBYNAME */
#ifdef DECL_GETHOSTNAME
	extern int gethostname(char *, size_t);
#endif /* DECL_GETHOSTNAME */
#ifdef DECL_GETLOGIN
	extern char *getlogin(void);
#endif /* DECL_GETLOGIN */
#ifdef DECL_GETOPT
	extern int getopt(int, char * const*, const char *);
#endif /* DECL_GETOPT */
#ifdef DECL_GETPWNAM
	extern struct passwd *getpwnam(const char *);
#endif /* DECL_GETPWNAM */
#ifdef DECL_GETSERVBYNAME
	extern struct servent *getservbyname(const char *, const char *);
#endif /* DECL_GETSERVBYNAME */
#ifdef DECL_GETWD
	extern char *getwd(char *);
#endif /* DECL_GETWD */

#if 0 /* doesn't match prototype in proto.h */
#	ifdef DECL_HEAPSORT
	extern int heapsort(void *, size_t, size_t, int (*)(t_comptype*, t_comptype*));
#	endif /* DECL_HEAPSORT */
#endif /* 0 */

#ifdef DECL_INET_ADDR
	extern unsigned long inet_addr(const char *);
#endif /* DECL_INET_ADDR */

#if 0 /* breaks gcc 3.0 -std=c89 on SuSE 7.1 */
#	ifdef DECL_INET_ATON
		extern int inet_aton(const char *, struct in_addr *);
#	endif /* DECL_INET_ATON */
#endif /* 0 */

#ifdef DECL_IOCTL
	extern int ioctl(int, int, void *);
#endif /* DECL_IOCTL */
#if defined(DECL_ISASCII) && !defined(isascii)
	extern int isascii(int);
#endif /* DECL_ISASCII && !isascii */
#ifdef DECL_KILL
	extern int kill(pid_t, int);
#endif /* DECL_KILL */
#ifdef DECL_LRAND48
	extern long lrand48(void);
#endif /* DECL_LRAND48 */
#ifdef DECL_MALLOC
	extern void *malloc(size_t);
#endif /* DECL_MALLOC */
#ifdef DECL_MEMSET
	extern void *memset(void *, int, size_t);
#endif /* DECL_MEMSET */
#ifdef DECL_MKSTEMP
	extern int mkstemp(char *);
#endif /* DECL_MKSTEMP */
#ifdef DECL_MKTEMP
	extern char *mktemp(char *);
#endif /* DECL_MKTEMP */
#ifdef DECL_PCLOSE
	extern int pclose(FILE *);
#endif /* DECL_PCLOSE */
#ifdef DECL_PERROR
	extern void perror(const char *);
#endif /* DECL_PERROR */
#ifdef DECL_POPEN
	extern FILE *popen(const char *, const char *);
#endif /* DECL_POPEN */
#ifdef DECL_PRINTF
	extern int printf(const char *, ...);
#endif /* DECL_PRINTF */
#ifdef DECL_PUTENV
	extern int putenv(char *);
#endif /* DECL_PUTENV */
#ifdef DECL_QSORT
	extern void qsort(void *, size_t, size_t, int (*)(t_comptype*, t_comptype*));
#endif /* DECL_QSORT */
#ifdef DECL_REALLOC
	extern void *realloc(void *, size_t);
#endif /* DECL_REALLOC */
#ifdef DECL_RENAME
	extern int rename(const char *, const char *);
#endif /* DECL_RENAME */
#ifdef DECL_REWIND
	extern void rewind(FILE *);
#endif /* DECL_REWIND */
#ifdef DECL_SELECT
	extern int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
#endif /* DECL_SELECT */
#ifdef DECL_SETENV
	extern int setenv(const char *, const char *, int);
#endif /* DECL_SETENV */
#ifdef DECL_SOCKET
	extern int socket(int, int, int);
#endif /* DECL_SOCKET */
#ifdef DECL_SNPRINTF
	extern int snprintf(char *, size_t, const char *, ...);
#endif /* DECL_SNPRINTF */
#ifdef DECL_SRAND48
	extern void srand48(long seedval);
#endif /* DECL_SRAND48 */
#ifdef DECL_SSCANF
	extern int sscanf(const char *, const char *, ...);
#endif /* DECL_SSCANF */
#ifdef DECL_STRCASECMP
	extern int strcasecmp(const char *, const char *);
#endif /* DECL_STRCASECMP */
#ifdef DECL_STRCHR
	extern char *strchr(const char *, int);
#endif /* DECL_STRCHR */
#ifdef DECL_STRRCHR
	extern char *strrchr(const char *, int);
#endif /* DECL_STRRCHR */
#ifdef DECL_STRFTIME
	extern int strftime(char *, int, char *, struct tm *);
#endif /* DECL_STRFTIME */
#ifdef DECL_STRNCASECMP
	extern int strncasecmp(const char *, const char *, size_t);
#endif /* DECL_STRNCASECMP */
#ifdef DECL_STRSEP
	extern char *strsep(char **, const char *);
#endif /* DECL_STRSEP */
#ifdef DECL_STRTOL
	extern long strtol(const char *, char **, int);
#endif /* DECL_STRTOL */
#ifdef DECL_STRTOLL
	extern long long strtoll(const char *, char **, int);
#endif /* DECL_STRTOLL */
#ifdef DECL_SYSTEM
	extern int system(const char *);
#endif /* DECL_SYSTEM */
#ifdef DECL_TGETENT
	extern int tgetent(char *, char *);
#endif /* DECL_TGETENT */
#ifdef DECL_TGETFLAG
	extern int tgetflag(char *);
#endif /* DECL_TGETFLAG */
#if defined(DECL_TGETNUM)
	extern int tgetnum(char *);
#endif /* DECL_TGETNUM */
#ifdef DECL_TGETSTR
	extern char *tgetstr(char *, char **);
#endif /* DECL_TGETSTR */
#ifdef DECL_TGOTO
	extern char *tgoto(char *, int, int);
#endif /* DECL_TGOTO */
#ifdef DECL_TIGETFLAG
	extern int tigetflag(char *);
#endif /* DECL_TIGETFLAG */
#if defined(DECL_TIGETNUM)
	extern int tigetnum(char *);
#endif /* DECL_TGETNUM */
#ifdef DECL_TIGETSTR
	extern char *tigetstr(char *);
#endif /* DECL_TIGETSTR */
#ifdef DECL_TIME
	extern time_t time(time_t *);
#endif /* DECL_TIME */
#if defined(DECL_TOLOWER) && !defined(tolower)
	extern int tolower(int);
#endif /* DECL_TOLOWER && !tolower */
#if defined(DECL_TOUPPER) && !defined(toupper)
	extern int toupper(int);
#endif /* DECL_TOUPPER && !toupper */
#ifdef DECL_TPARM
	extern char *tparm(const char *, ...);
#endif /* DECL_TPARM */
#ifdef DECL_TPUTS
	extern int tputs(char *, int, OutcPtr);
#endif /* DECL_TPUTS */
#ifdef DECL_UNGETC
	extern int ungetc(int, FILE *);
#endif /* DECL_UNGETC */
#ifdef DECL_WCSWIDTH
	extern int wcswidth(const wchar_t *, size_t);
#endif /* DECL_WCSWIDTH */
#ifdef DECL_WCWIDTH
	extern int wcwidth(wchar_t);
#endif /* DECL_WCWIDTH */

#if 0 /* SUSv2 changed that to "int usleep(unsigned long)" */
#	ifdef DECL_USLEEP
		extern void usleep(unsigned long);
#	endif /* DECL_USLEEP */
#endif /* 0 */

#ifdef DECL_VSNPRINTF
	extern int vsnprintf(char *, size_t, const char *, va_list);
#endif /* DECL_VSNPRINTF */
#if 0 /* some (most?) systems have "int vsprintf(char *, const char *, va_list)" */
#	ifdef DECL_VSPRINTF
	extern int vsprintf(char *, char *, va_list);
#	endif /* DECL_VSPRINTF */
#endif /* 0 */

#ifdef __CYGWIN__
	extern int __declspec(dllimport) optind;
	extern char __declspec(dllimport) *optarg;
#else
	extern int optind;
	extern char *optarg;
#endif /* __CYGWIN__ */

/*
 * Local variables
 */

/* This fixes ambiguities on platforms that don't distinguish extern case */
#ifdef CASE_PROBLEM
#	define txt_help_B	txt_help_BIGB
#	define txt_help_D	txt_help_BIGD
#	define txt_help_I	txt_help_BIGI
#	define txt_help_K	txt_help_BIGK
#	define txt_help_L	txt_help_BIGL
#	define txt_help_M	txt_help_BIGM
#	define txt_help_Q	txt_help_BIGQ
#	define txt_help_S	txt_help_BIGS
#	define txt_help_T	txt_help_BIGT
#	define txt_help_U	txt_help_BIGU
#	define txt_help_W	txt_help_BIGW
#	define txt_help_X	txt_help_BIGX
#	define txt_help_p_S	txt_help_p_BIGS
#	define txt_help_t_K	txt_help_t_BIGK
#	define txt_help_p_U	txt_help_p_BIGU
#	define txt_help_thd_C	txt_help_thd_BIGC
#endif /* CASE_PROBLEM */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern wchar_t *OPT_CHAR_list[];
#else
	extern char *OPT_CHAR_list[];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
extern char *OPT_STRING_list[];
extern char *nntp_server;
extern char *tin_progname;
extern const char *tmpdir;
extern char active_times_file[PATH_LEN];
extern char article_name[PATH_LEN];
extern char *backup_article_name;
extern char bug_addr[LEN];
#ifdef NNTP_ABLE
	extern char bug_nntpserver1[PATH_LEN];
	extern char bug_nntpserver2[PATH_LEN];
#endif /* NNTP_ABLE */
extern char cvers[LEN];
extern char dead_article[PATH_LEN];
extern char dead_articles[PATH_LEN];
extern char default_organization[PATH_LEN];
extern char default_signature[PATH_LEN];
extern char global_attributes_file[PATH_LEN];
extern char global_config_file[PATH_LEN];
extern char homedir[PATH_LEN];
extern char index_maildir[PATH_LEN];
extern char index_newsdir[PATH_LEN];
extern char index_savedir[PATH_LEN];
extern char inewsdir[PATH_LEN];
extern char local_attributes_file[PATH_LEN];
extern char local_config_file[PATH_LEN];
extern char filter_file[PATH_LEN];
extern char local_input_history_file[PATH_LEN];
extern char local_motd_file[PATH_LEN];
extern char local_newsgroups_file[PATH_LEN];
extern char local_newsrctable_file[PATH_LEN];
extern char lock_file[PATH_LEN];
extern char mail_news_user[LEN];
extern char mailbox[PATH_LEN];
extern char mailer[PATH_LEN];
#ifdef HAVE_MH_MAIL_HANDLING
	extern char mail_active_file[PATH_LEN];
	extern char mailgroups_file[PATH_LEN];
#endif /* HAVE_MH_MAIL_HANDLING */
extern char newnewsrc[PATH_LEN];
extern char news_active_file[PATH_LEN];
extern char newsgroups_file[PATH_LEN];
extern char newsrc[PATH_LEN];
#ifndef NNTP_ONLY
	extern char novrootdir[PATH_LEN];
	extern char novfilename[NAME_LEN + 1];
#endif /* !NNTP_ONLY */
extern char page_header[LEN];
extern char posted_info_file[PATH_LEN];
extern char postponed_articles_file[PATH_LEN];
extern char rcdir[PATH_LEN];
extern char save_active_file[PATH_LEN];
extern char spooldir[PATH_LEN];
extern char subscriptions_file[PATH_LEN];
extern char overviewfmt_file[PATH_LEN];
extern char txt_help_bug_report[LEN];
extern char userid[PATH_LEN];

extern char domain_name[];

extern const char base64_alphabet[64];

#ifdef USE_CANLOCK
	extern constext *txt_cancel_lock_algos[];
#endif /* USE_CANLOCK */
extern constext *content_disposition[];
extern constext *content_encodings[];
extern constext *content_types[];
extern constext *txt_attrs[];
extern constext *txt_auto_cc_bcc_options[];
#ifdef HAVE_COLOR
	extern constext *txt_colors[];
	extern constext *txt_backcolors[];
#endif /* HAVE_COLOR */
extern constext *txt_confirm_choices[];
extern constext *txt_goto_next_unread_options[];
extern constext *txt_hide_uue_type[];
extern constext *txt_interactive_mailers[];
extern constext *txt_kill_level_type[];
#ifdef CHARSET_CONVERSION
	extern constext *txt_mime_charsets[];	/* supported charsets */
#endif /* CHARSET_CONVERSION */
extern constext *txt_mime_7bit_charsets[]; /* 7bit charsets */
extern constext *txt_mailbox_formats[];
extern constext *txt_marks[];
extern constext *txt_mime_encodings[];
#ifdef HAVE_UNICODE_NORMALIZATION
	extern constext *txt_normalization_forms[];
#endif /* HAVE_UNICODE_NORMALIZATION */
extern constext *txt_onoff[];
extern constext *txt_post_process_types[];
extern constext *txt_quick_ks_header_options[];
extern constext *txt_quote_style_type[];
extern constext *txt_show_from[];
extern constext *txt_show_help_mail_sign_options[];
extern constext *txt_sort_a_type[];	/* a=articles */
extern constext *txt_sort_t_type[];	/* t=threads */
extern constext *txt_strip_bogus_type[];
extern constext *txt_threading[];
extern constext *txt_thread_score_type[];
extern constext *txt_trim_article_body_options[];
extern constext *txt_wildcard_type[];
extern constext txt_1_resp[];
#if defined(NNTP_ABLE) && defined(HAVE_SELECT)
	extern constext txt_abort_reading[];
#endif /* NNTP_ABLE && HAVE_SELECT */
extern constext txt_active_file_is_empty[];
extern constext txt_all[];
extern constext txt_all_groups[];
extern constext txt_append_overwrite_quit[];
extern constext txt_art_cancel[];
extern constext txt_art_mailgroups[];
extern constext txt_art_newsgroups[];
extern constext txt_art_not_posted[];
extern constext txt_art_not_saved[];
extern constext txt_art_pager_com[];
extern constext txt_art_parent_killed[];
extern constext txt_art_parent_none[];
extern constext txt_art_parent_unavail[];
extern constext txt_art_posted[];
extern constext txt_art_rejected[];
extern constext txt_art_score[];
extern constext txt_art_thread_regex_tag[];
extern constext txt_art_unavailable[];
extern constext txt_art_x_of_n[];
extern constext txt_article_cancelled[];
extern constext txt_article_info_page[];
extern constext txt_article_plural[];
extern constext txt_article_reposted[];
extern constext txt_article_singular[];
extern constext txt_article_upper[];
extern constext txt_articles_mailed[];
#ifndef DISABLE_PRINTING
	extern constext txt_articles_printed[];
#endif /* !DISABLE_PRINTING */
#ifndef DONT_HAVE_PIPING
	extern constext txt_articles_piped[];
#endif /* !DONT_HAVE_PIPING */
extern constext txt_mime_boundary[];
extern constext txt_mime_boundary_end[];
extern constext txt_mime_charset[];
extern constext txt_mime_content_subtype[];
extern constext txt_mime_content_type[];
extern constext txt_mime_description[];
extern constext txt_mime_encoding[];
extern constext txt_mime_hdr_c_disposition[];
extern constext txt_mime_hdr_c_transfer_encoding[];
extern constext txt_mime_hdr_c_type_msg_rfc822[];
extern constext txt_mime_hdr_c_type_multipart_mixed[];
extern constext txt_mime_hdr_c_type_text_plain_charset[];
extern constext txt_mime_lang[];
extern constext txt_mime_lines[];
extern constext txt_mime_name[];
extern constext txt_mime_sep[];
extern constext txt_mime_size[];
extern constext txt_mime_preamble_multipart_mixed[];
extern constext txt_mime_unsup_charset[];
extern constext txt_mime_version[];
extern constext txt_attachment_menu[];
extern constext txt_attachment_menu_com[];
extern constext txt_attachment_no_name[];
extern constext txt_attachment_saved[];
extern constext txt_attachments_saved[];
extern constext txt_attachment_select[];
extern constext txt_attachment_tagged[];
extern constext txt_attachments_tagged[];
extern constext txt_attachment_untagged[];
extern constext txt_attrib_file_version[];
extern constext txt_attrib_file_header[];
extern constext txt_attrib_file_scope[];
extern constext txt_attrib_file_posted_to_filter[];
extern constext txt_attrib_file_advertising[];
extern constext txt_attrib_file_alt_handling[];
extern constext txt_attrib_file_metamail[];
extern constext txt_attrib_file_auto_cc_bcc[];
extern constext txt_attrib_file_auto_cc_bcc_opts[];
extern constext txt_attrib_file_auto_list_thrd[];
extern constext txt_attrib_file_auto_select[];
extern constext txt_attrib_file_batch_save[];
extern constext txt_attrib_file_date_fmt[];
extern constext txt_attrib_file_delete_tmp[];
extern constext txt_attrib_file_editor_fmt[];
extern constext txt_attrib_file_fcc[];
extern constext txt_attrib_file_followup_to[];
extern constext txt_attrib_file_from[];
extern constext txt_attrib_file_grp_catchup[];
extern constext txt_attrib_file_grp_fmt[];
extern constext txt_attrib_file_mail_8bit_hdr[];
extern constext txt_attrib_file_mail_mime_enc[];
#ifdef HAVE_ISPELL
	extern constext txt_attrib_file_ispell[];
#endif /* HAVE_ISPELL */
extern constext txt_attrib_file_maildir[];
extern constext txt_attrib_file_mailing_list[];
extern constext txt_attrib_file_mime_types_to_save[];
extern constext txt_attrib_file_mark_ignore_tags[];
extern constext txt_attrib_file_mark_saved_read[];
extern constext txt_attrib_file_mime_forward[];
#ifdef CHARSET_CONVERSION
	extern constext txt_attrib_file_mm_network_charset[];
	extern constext txt_attrib_file_undeclared_charset[];
#	ifdef USE_ICU_UCSDET
	extern constext txt_attrib_file_undeclared_cs_guess[];
#	endif /* USE_ICU_UCSDET */
#endif /* CHARSET_CONVERSION */
extern constext txt_attrib_file_hdr_to_disp[];
extern constext txt_attrib_file_hdr_to_not_disp[];
extern constext txt_attrib_file_quote_fmt[];
extern constext txt_attrib_file_organization[];
extern constext txt_attrib_file_pos_first_unread[];
extern constext txt_attrib_file_post_8bit_hdr[];
extern constext txt_attrib_file_post_mime_enc[];
extern constext txt_attrib_file_post_proc_type[];
extern constext txt_attrib_file_post_proc_view[];
extern constext txt_attrib_file_quick_kill_scope[];
extern constext txt_attrib_file_quick_kill_expire[];
extern constext txt_attrib_file_quick_kill_case[];
extern constext txt_attrib_file_quick_kill_hdr[];
extern constext txt_attrib_file_quick_kill_hdr_0_1[];
extern constext txt_attrib_file_quick_kill_hdr_2_3[];
extern constext txt_attrib_file_quick_kill_hdr_4[];
extern constext txt_attrib_file_quick_kill_hdr_5[];
extern constext txt_attrib_file_quick_kill_hdr_6[];
extern constext txt_attrib_file_quick_select_scope[];
extern constext txt_attrib_file_quick_select_expire[];
extern constext txt_attrib_file_quick_select_case[];
extern constext txt_attrib_file_quick_select_hdr[];
extern constext txt_attrib_file_quick_select_hdr_0_1[];
extern constext txt_attrib_file_quick_select_hdr_2_3[];
extern constext txt_attrib_file_quick_select_hdr_4[];
extern constext txt_attrib_file_quick_select_hdr_5[];
extern constext txt_attrib_file_quick_select_hdr_6[];
extern constext txt_attrib_file_quote_chars[];
#ifndef DISABLE_PRINTING
	extern constext txt_attrib_file_print_hdr[];
#endif /* !DISABLE_PRINTING */
extern constext txt_attrib_file_process_only_unread[];
extern constext txt_attrib_file_prompt_followup[];
extern constext txt_attrib_file_savedir[];
extern constext txt_attrib_file_savefile[];
extern constext txt_attrib_file_sigfile[];
extern constext txt_attrib_file_show_author[];
extern constext txt_attrib_file_show_signatures[];
extern constext txt_attrib_file_show_art_score[];
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern constext txt_attrib_file_suppress_soft_hyphens[];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
extern constext txt_attrib_file_show_only_unread[];
extern constext txt_attrib_file_sigdashes[];
extern constext txt_attrib_file_signature_repost[];
extern constext txt_attrib_file_sort_art_type[];
extern constext txt_attrib_file_sort_thrd_type[];
extern constext txt_attrib_file_tex2iso[];
extern constext txt_attrib_file_thrd_catchup[];
extern constext txt_attrib_file_thrd_arts[];
extern constext txt_attrib_file_thrd_fmt[];
extern constext txt_attrib_file_thrd_perc[];
extern constext txt_attrib_file_trim_art_body[];
extern constext txt_attrib_file_trim_art_body_0[];
extern constext txt_attrib_file_trim_art_body_1[];
extern constext txt_attrib_file_trim_art_body_2[];
extern constext txt_attrib_file_trim_art_body_3[];
extern constext txt_attrib_file_trim_art_body_4[];
extern constext txt_attrib_file_trim_art_body_5[];
extern constext txt_attrib_file_trim_art_body_6[];
extern constext txt_attrib_file_trim_art_body_7[];
extern constext txt_attrib_file_verbatim_handling[];
#ifdef HAVE_COLOR
	extern constext txt_attrib_file_extquote_handling[];
#endif /* HAVE_COLOR */
extern constext txt_attrib_file_wrap_on_unread[];
extern constext txt_attrib_file_x_body[];
extern constext txt_attrib_file_x_comment[];
extern constext txt_attrib_file_x_headers[];
extern constext txt_attrib_file_note_1[];
extern constext txt_attrib_file_note_2[];
extern constext txt_attrib_file_footer[];
extern constext txt_attrib_menu_com[];
extern constext txt_attrib_no_scope[];
extern constext txt_at_s[];
#ifdef NNTP_ABLE
	extern constext txt_auth_failed[];
	extern constext txt_auth_failed_nopass[];
	extern constext txt_auth_pass[];
	extern constext txt_auth_user[];
	extern constext txt_auth_needed[];
	extern constext txt_authorization_fail[];
	extern constext txt_authorization_ok[];
	extern constext txt_authorization_unavail[];
#endif /* NNTP_ABLE */
extern constext txt_author_search_backwards[];
extern constext txt_author_search_forwards[];
extern constext txt_autoselecting_articles[];
extern constext txt_autosubscribed[];
extern constext txt_autosubscribing_groups[];
extern constext txt_bad_article[];
extern constext txt_bad_attrib[];
extern constext txt_bad_command[];
extern constext txt_base_article[];
extern constext txt_base_article_range[];
extern constext txt_batch_update_unavail[];
extern constext txt_begin_of_art[];
extern constext txt_begin_of_page[];
#ifdef NNTP_ABLE
	extern constext txt_caching_off[];
	extern constext txt_caching_on[];
#endif /* NNTP_ABLE */
extern constext txt_cancel_article[];
extern constext txt_cancelling_art[];
extern constext txt_cannot_create[];
extern constext txt_cannot_create_uniq_name[];
#ifdef DEBUG
	extern constext txt_cannot_find_base_art[];
#endif /* DEBUG */
extern constext txt_cannot_filter_on_path[];
#ifdef NNTP_ABLE
	extern constext txt_cannot_get_nntp_server_name[];
#endif /* NNTP_ABLE */
#if !defined(USE_CURSES) && !defined(USE_TERMINFO)
	extern constext txt_cannot_get_term_entry[];
#endif /* !USE_CURSES && !USE_TERMINFO */
extern constext txt_cannot_open[];
extern constext txt_cannot_open_for_saving[];
extern constext txt_cannot_post[];
extern constext txt_cannot_post_group[];
#ifdef NNTP_ABLE
	extern constext txt_cannot_retrieve[];
#endif /* NNTP_ABLE */
extern constext txt_cannot_supersede_mailgroups[];
extern constext txt_cannot_write_to_directory[];
#ifdef NNTP_ABLE
	extern constext txt_capabilities_without_reader[];
#endif /* NNTP_ABLE */
extern constext txt_catchup[];
extern constext txt_catchup_group[];
extern constext txt_catchup_all_read_groups[];
extern constext txt_catchup_despite_tags[];
extern constext txt_catchup_update_info[];
extern constext txt_caughtup[];
extern constext txt_check_article[];
extern constext txt_checking_for_news[];
extern constext txt_checking_new_groups[];
extern constext txt_choose_post_process_type[];
#ifdef HAVE_COLOR
	extern constext txt_color_off[];
	extern constext txt_color_on[];
#endif /* HAVE_COLOR */
extern constext txt_command_failed[];
extern constext txt_cook_article_failed_exiting[];
extern constext txt_confirm_select_on_exit[];
#ifdef NNTP_ABLE
	extern constext txt_connecting_port[];
	extern constext txt_connection_error[];
#endif /* NNTP_ABLE */
#if defined(NNTP_ABLE) && !defined(INET6)
	extern constext txt_connection_to[];
#endif /* NNTP_ABLE && !INET6 */
extern constext txt_connection_info[];
extern constext txt_conninfo_local_spool[];
extern constext txt_conninfo_saved_news[];
#ifndef NNTP_ONLY
	extern constext txt_conninfo_active_file[];
	extern constext txt_conninfo_active_times_file[];
	extern constext txt_conninfo_newsgroups_file[];
	extern constext txt_conninfo_novrootdir[];
	extern constext txt_conninfo_overview_file[];
	extern constext txt_conninfo_overview_fmt[];
	extern constext txt_conninfo_spool_config[];
	extern constext txt_conninfo_spooldir[];
	extern constext txt_conninfo_subscriptions_file[];
#endif /* !NNTP_ONLY */
#ifdef NNTP_ABLE
	extern constext txt_conninfo_compress[];
	extern constext txt_conninfo_conn_details[];
#	ifdef USE_ZLIB
	extern constext txt_conninfo_deflate[];
	extern constext txt_conninfo_enabled[];
	extern constext txt_conninfo_inactive[];
#	else
	extern constext txt_conninfo_deflate_unsupported[];
#	endif /* USE_ZLIB */
#	if defined(HAVE_ALARM) && defined(SIGALRM)
	extern constext txt_conninfo_disabled[];
	extern constext txt_conninfo_timeout[];
#	endif /* HAVE_ALARM && SIGALRM */
	extern constext txt_conninfo_implementation[];
#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
	extern constext txt_conninfo_maxartnum[];
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
	extern constext txt_conninfo_nntp[];
	extern constext txt_conninfo_port[];
	extern constext txt_conninfo_ro[];
	extern constext txt_conninfo_rw[];
	extern constext txt_conninfo_server[];
#	ifdef NNTPS_ABLE
	extern constext txt_conninfo_nntps[];
	extern constext txt_conninfo_trusted[];
	extern constext txt_conninfo_untrusted[];
	extern constext txt_conninfo_cert[];
	extern constext txt_conninfo_fmt_error[];
	extern constext txt_conninfo_issuer[];
	extern constext txt_conninfo_server_cert_info[];
	extern constext txt_conninfo_subject[];
	extern constext txt_conninfo_tls_info[];
#		if defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_OPENSSL)
		extern constext txt_conninfo_error_unexpected[];
		extern constext txt_conninfo_error_tolerated[];
		extern constext txt_conninfo_verify_failed[];
		extern constext txt_conninfo_verify_successful[];
#		endif /* HAVE_LIB_GNUTLS || HAVE_LIB_OPENSSL */
#		ifdef HAVE_LIB_GNUTLS
		extern constext txt_conninfo_gnutls[];
		extern constext txt_conninfo_verify_failed_no_reason[];
#		endif /* HAVE_LIB_GNUTLS */
#		ifdef HAVE_LIB_LIBTLS
		extern constext txt_conninfo_libressl[];
		extern constext txt_conninfo_libtls_info[];
#		endif /* HAVE_LIB_LIBTLS */
#		ifdef HAVE_LIB_OPENSSL
		extern constext txt_conninfo_openssl[];
#		endif /* HAVE_LIB_OPENSSL */
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */
extern constext txt_copyright_notice[];
extern constext txt_cr[];
extern constext txt_creating_active[];
extern constext txt_creating_newsrc[];
extern constext txt_default[];
extern constext txt_delete_processed_files[];
extern constext txt_deleting[];
#ifdef NNTP_ABLE
	extern constext txt_disconnecting[];
#endif /* NNTP_ABLE */
extern constext txt_end_of_art[];
extern constext txt_end_of_arts[];
extern constext txt_end_of_attachments[];
extern constext txt_end_of_groups[];
extern constext txt_end_of_page[];
extern constext txt_end_of_posted[];
extern constext txt_end_of_scopes[];
extern constext txt_end_of_thread[];
extern constext txt_end_of_urls[];
extern constext txt_enter_getart_limit[];
extern constext txt_enter_message_id[];
extern constext txt_enter_next_thread[];
extern constext txt_enter_next_unread_art[];
extern constext txt_enter_next_unread_group[];
extern constext txt_enter_option_num[];
extern constext txt_enter_range[];
extern constext txt_enter_append[];
extern constext txt_error_approved[];
#ifndef NDEBUG
	extern constext txt_error_asfail[];
#endif /* !NDEBUG */
extern constext txt_error_bad_address_in[];
extern constext txt_error_bad_msgidfqdn[];
#ifndef NO_LOCKING
	extern constext txt_error_cant_unlock[];
	extern constext txt_error_couldnt_dotlock[];
	extern constext txt_error_couldnt_lock[];
#endif /* NO_LOCKING */
#if defined(NNTP_ABLE) && defined(USE_ZLIB)
	extern constext txt_error_compression_auth[];
#endif /* NNTP_ABLE && USE_ZLIB */
extern constext txt_error_copy_fp[];
extern constext txt_error_corrupted_file[];
#ifdef NNTP_ABLE
	extern constext txt_error_couldnt_expand[];
#endif /* NNTP_ABLE */
extern constext txt_error_fseek[];
extern constext txt_error_followup_poster[];
extern constext txt_error_format_string[];
extern constext txt_error_gnksa_internal[];
extern constext txt_error_gnksa_langle[];
extern constext txt_error_gnksa_lparen[];
extern constext txt_error_gnksa_rparen[];
extern constext txt_error_gnksa_atsign[];
extern constext txt_error_gnksa_sgl_domain[];
extern constext txt_error_gnksa_inv_domain[];
extern constext txt_error_gnksa_ill_domain[];
extern constext txt_error_gnksa_unk_domain[];
extern constext txt_error_gnksa_fqdn[];
extern constext txt_error_gnksa_zero[];
extern constext txt_error_gnksa_length[];
extern constext txt_error_gnksa_hyphen[];
extern constext txt_error_gnksa_begnum[];
extern constext txt_error_gnksa_bad_lit[];
extern constext txt_error_gnksa_local_lit[];
extern constext txt_error_gnksa_rbracket[];
extern constext txt_error_gnksa_lp_missing[];
extern constext txt_error_gnksa_lp_invalid[];
extern constext txt_error_gnksa_lp_zero[];
extern constext txt_error_gnksa_rn_unq[];
extern constext txt_error_gnksa_rn_qtd[];
extern constext txt_error_gnksa_rn_enc[];
extern constext txt_error_gnksa_rn_encsyn[];
extern constext txt_error_gnksa_rn_paren[];
extern constext txt_error_gnksa_rn_invalid[];
extern constext txt_error_header_and_body_not_separate[];
extern constext txt_error_header_distribution_all[];
extern constext txt_error_header_duplicate[];
extern constext txt_error_header_format[];
extern constext txt_error_header_line_bad_charset[];
extern constext txt_error_header_line_bad_encoding[];
extern constext txt_error_header_line_blank[];
extern constext txt_error_header_line_colon[];
extern constext txt_error_header_line_empty[];
extern constext txt_error_header_line_missing[];
extern constext txt_error_header_line_not_7bit[];
extern constext txt_error_header_line_space[];
extern constext txt_error_header_no_name[];
extern constext txt_error_mailgroup_no_recipient[];
#ifndef FILE_MODE_BROKEN
	extern constext txt_error_insecure_permissions[];
#endif /* !FILE_MODE_BROKEN */
#ifdef MIME_BREAK_LONG_LINES
	extern constext txt_error_should_be_folded[];
#else
	extern constext txt_error_should_be_shortened[];
#endif /* MIME_BREAK_LONG_LINES */
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
	extern constext txt_error_locale[];
#endif /* HAVE_SETLOCALE && !NO_LOCALE */
extern constext txt_error_mime_end[];
extern constext txt_error_mime_start[];
extern constext txt_error_no_domain_name[];
#ifdef NNTP_INEWS
	extern constext txt_error_no_from[];
#endif /* NNTP_INEWS */
#ifdef NNTP_ABLE
	extern constext txt_error_no_enter_permission[];
	extern constext txt_error_no_read_permission[];
	extern constext txt_error_no_such_file[];
	extern constext txt_error_no_write_permission[];
#	ifdef INET6
		extern constext txt_error_not_ipv6_literal[];
#	endif /* INET6 */
#endif /* NNTP_ABLE */
extern constext txt_error_newsgroups_poster[];
extern constext txt_error_passwd_missing[];
#ifdef HAVE_LIBUU
	extern constext txt_error_plural[];
	extern constext txt_error_singular[];
#endif /* HAVE_LIBUU */
extern constext txt_error_server_has_no_listed_groups[];
#if defined(NNTP_ABLE) && defined(INET6)
	extern constext txt_error_socket_or_connect_problem[];
#endif /* NNTP_ABLE && INET6 */
#if defined(NNTP_ABLE) && defined(TLI) && !defined(INET6)
	extern constext txt_error_server_unavailable[];
	extern constext txt_error_topen[];
#endif /* NNTP_ABLE && TLI && !INET6 */
extern constext txt_error_unlink[];
extern constext txt_error_unknown_dlevel[];
#if defined(NNTP_ABLE) && defined(HAVE_GETSERVBYNAME) && !defined(INET6)
	extern constext txt_error_unknown_service[];
#endif /* NNTP_ABLE && HAVE_GETSERVBYNAME && !INET6 */
#ifndef NNTP_ABLE
	extern constext txt_error_unreachable[];
#endif /* !NNTP_ABLE */
extern constext txt_esc[];
extern constext txt_exiting[];
extern constext txt_external_mail_done[];
extern constext txt_extracting_shar[];
#ifdef NNTP_ABLE
	extern constext txt_failed_to_connect_to_server[];
#endif /* NNTP_ABLE */
extern constext txt_feed_pattern[];
extern constext txt_filesystem_full[];
extern constext txt_filesystem_full_backup[];
extern constext txt_filter_comment[];
#ifdef DEBUG
	extern constext txt_filter_error_overview_no_servername[];
	extern constext txt_filter_error_overview_xref[];
	extern constext txt_filter_error_skipping_xref_filter[];
#endif /* DEBUG */
extern constext txt_filter_file[];
extern constext txt_filter_file_version[];
extern constext txt_filter_global_rules[];
extern constext txt_filter_rule_created[];
extern constext txt_filter_score[];
extern constext txt_filter_score_help[];
extern constext txt_filter_text_type[];
extern constext txt_followup_newsgroups[];
extern constext txt_followup_poster[];
extern constext txt_forwarded[];
extern constext txt_forwarded_end[];
extern constext txt_from_line_only[];
extern constext txt_from_line_only_case[];
extern constext txt_full[];
#ifdef NNTP_ABLE
	extern constext txt_gethostbyname[];
#endif /* NNTP_ABLE */
#if defined(NNTP_ABLE) && !defined(INET6)
	extern constext txt_giving_up[];
#endif /* NNTP_ABLE && !INET6 */
extern constext txt_global[];
extern constext txt_group[];
extern constext txt_group_aliased[];
extern constext txt_group_bogus[];
extern constext txt_group_is_moderated[];
extern constext txt_group_plural[];
extern constext txt_group_rereading[];
extern constext txt_group_select_com[];
extern constext txt_group_selection[];
extern constext txt_group_singular[];
extern constext txt_grpdesc_disabled[];
extern constext txt_help_article_autokill[];
extern constext txt_help_article_autoselect[];
extern constext txt_help_article_browse_urls[];
extern constext txt_help_article_by_num[];
#ifndef NO_POSTING
	extern constext txt_help_article_cancel[];
	extern constext txt_help_article_followup[];
	extern constext txt_help_article_followup_no_quote[];
	extern constext txt_help_article_followup_with_header[];
	extern constext txt_help_article_repost[];
#endif /* !NO_POSTING */
extern constext txt_help_article_edit[];
extern constext txt_help_article_first_in_thread[];
extern constext txt_help_article_first_page[];
extern constext txt_help_article_last_in_thread[];
extern constext txt_help_article_last_page[];
extern constext txt_help_article_mark_thread_read[];
extern constext txt_help_article_next[];
extern constext txt_help_article_next_thread[];
extern constext txt_help_article_next_unread[];
extern constext txt_help_article_parent[];
#ifdef HAVE_PGP_GPG
	extern constext txt_help_article_pgp[];
#endif /* HAVE_PGP_GPG */
extern constext txt_help_article_prev[];
extern constext txt_help_article_prev_unread[];
extern constext txt_help_article_quick_kill[];
extern constext txt_help_article_quick_select[];
extern constext txt_help_article_quit_to_select_level[];
extern constext txt_help_article_reply[];
extern constext txt_help_article_reply_no_quote[];
extern constext txt_help_article_reply_with_header[];
extern constext txt_help_article_search_backwards[];
extern constext txt_help_article_search_forwards[];
extern constext txt_help_article_show_raw[];
extern constext txt_help_article_skip_quote[];
extern constext txt_help_article_toggle_formfeed[];
extern constext txt_help_article_toggle_headers[];
extern constext txt_help_article_toggle_highlight[];
extern constext txt_help_article_toggle_rot13[];
extern constext txt_help_article_toggle_tabwidth[];
extern constext txt_help_article_toggle_tex2iso[];
extern constext txt_help_article_toggle_uue[];
extern constext txt_help_article_view_attachments[];
extern constext txt_help_attachment_first[];
extern constext txt_help_attachment_goto[];
extern constext txt_help_attachment_last[];
#ifndef DONT_HAVE_PIPING
	extern constext txt_help_attachment_pipe[];
	extern constext txt_help_attachment_pipe_raw[];
#endif /* !DONT_HAVE_PIPING */
extern constext txt_help_attachment_save[];
extern constext txt_help_attachment_search_forwards[];
extern constext txt_help_attachment_search_backwards[];
extern constext txt_help_attachment_select[];
extern constext txt_help_attachment_tag[];
extern constext txt_help_attachment_tag_pattern[];
extern constext txt_help_attachment_toggle_tagged[];
extern constext txt_help_attachment_untag[];
extern constext txt_help_attachment_toggle_info_line[];
extern constext txt_help_attrib_first_opt[];
extern constext txt_help_attrib_goto_opt[];
extern constext txt_help_attrib_last_opt[];
extern constext txt_help_attrib_reset_attrib[];
extern constext txt_help_attrib_search_opt_backwards[];
extern constext txt_help_attrib_search_opt_forwards[];
extern constext txt_help_attrib_select[];
extern constext txt_help_attrib_toggle_attrib[];
extern constext txt_help_bug[];
extern constext txt_help_config_first_opt[];
extern constext txt_help_config_goto_opt[];
extern constext txt_help_config_last_opt[];
extern constext txt_help_config_scope_menu[];
extern constext txt_help_config_search_opt_backwards[];
extern constext txt_help_config_search_opt_forwards[];
extern constext txt_help_config_select[];
extern constext txt_help_config_toggle_attrib[];
extern constext txt_help_filter_comment[];
extern constext txt_help_filter_from[];
extern constext txt_help_filter_lines[];
extern constext txt_help_filter_msgid[];
extern constext txt_help_filter_subj[];
extern constext txt_help_filter_text[];
extern constext txt_help_filter_text_type[];
extern constext txt_help_filter_time[];
extern constext txt_help_global_article_range[];
extern constext txt_help_global_edit_filter[];
extern constext txt_help_global_esc[];
extern constext txt_help_global_help[];
extern constext txt_help_global_last_art[];
extern constext txt_help_global_line_down[];
extern constext txt_help_global_line_up[];
extern constext txt_help_global_lookup_art[];
extern constext txt_help_global_mail[];
extern constext txt_help_global_option_menu[];
extern constext txt_help_global_page_down[];
extern constext txt_help_global_page_up[];
#ifndef DONT_HAVE_PIPING
	extern constext txt_help_global_pipe[];
#endif /* !DONT_HAVE_PIPING */
#ifndef NO_POSTING
	extern constext txt_help_global_post[];
	extern constext txt_help_global_post_postponed[];
#endif /* !NO_POSTING */
extern constext txt_help_global_posting_history[];
extern constext txt_help_global_previous_menu[];
#ifndef DISABLE_PRINTING
	extern constext txt_help_global_print[];
#endif /* !DISABLE_PRINTING */
extern constext txt_help_global_quit_tin[];
extern constext txt_help_global_redraw_screen[];
extern constext txt_help_global_save[];
extern constext txt_help_global_auto_save[];
extern constext txt_help_global_scroll_down[];
extern constext txt_help_global_scroll_up[];
extern constext txt_help_global_search_auth_backwards[];
extern constext txt_help_global_search_auth_forwards[];
extern constext txt_help_global_search_body[];
extern constext txt_help_global_search_body_comment[];
extern constext txt_help_global_search_repeat[];
extern constext txt_help_global_search_subj_backwards[];
extern constext txt_help_global_search_subj_forwards[];
#ifndef NO_SHELL_ESCAPE
	extern constext txt_help_global_shell_escape[];
#endif /* !NO_SHELL_ESCAPE */
extern constext txt_help_global_tag[];
#ifdef HAVE_COLOR
	extern constext txt_help_global_toggle_color[];
#endif /* HAVE_COLOR */
extern constext txt_help_global_toggle_info_line[];
extern constext txt_help_global_toggle_inverse_video[];
extern constext txt_help_global_toggle_mini_help[];
extern constext txt_help_global_toggle_subj_display[];
extern constext txt_help_global_version[];
extern constext txt_help_group_catchup[];
extern constext txt_help_group_catchup_next[];
extern constext txt_help_group_first_thread[];
extern constext txt_help_group_last_thread[];
extern constext txt_help_group_list_thread[];
extern constext txt_help_group_mark_article_unread[];
extern constext txt_help_group_mark_thread_read[];
extern constext txt_help_group_mark_thread_unread[];
extern constext txt_help_mark_feed_read[];
extern constext txt_help_mark_feed_unread[];
extern constext txt_help_group_mark_unsel_art_read[];
extern constext txt_help_group_next[];
extern constext txt_help_group_prev[];
extern constext txt_help_group_reverse_thread_selection[];
extern constext txt_help_group_select_all[];
extern constext txt_help_group_select_thread[];
extern constext txt_help_group_select_thread_if_unread_selected[];
extern constext txt_help_group_select_thread_pattern[];
extern constext txt_help_group_thread_by_num[];
extern constext txt_help_group_toggle_getart_limit[];
extern constext txt_help_group_toggle_read_articles[];
extern constext txt_help_group_toggle_thread_selection[];
extern constext txt_help_group_toggle_threading[];
extern constext txt_help_group_undo_thread_selection[];
extern constext txt_help_group_untag_thread[];
extern constext txt_help_kill_scope[];
extern constext txt_help_post_hist_search_forwards[];
extern constext txt_help_post_hist_search_backwards[];
extern constext txt_help_post_hist_toggle_info_line[];
extern constext txt_help_post_hist_select[];
extern constext txt_help_scope_add[];
extern constext txt_help_scope_del[];
extern constext txt_help_scope_edit_attrib_file[];
extern constext txt_help_scope_first_scope[];
extern constext txt_help_scope_goto_scope[];
extern constext txt_help_scope_last_scope[];
extern constext txt_help_scope_move[];
extern constext txt_help_scope_rename[];
extern constext txt_help_scope_select[];
extern constext txt_help_select_catchup[];
extern constext txt_help_select_catchup_next_unread[];
extern constext txt_help_select_first_group[];
extern constext txt_help_select_goto_group[];
extern constext txt_help_select_group_by_num[];
extern constext txt_help_select_group_range[];
extern constext txt_help_select_last_group[];
#ifdef NNTP_ABLE
	extern constext txt_help_select_lookup_group[];
	extern constext txt_help_select_lookup_group_comment[];
#endif /* NNTP_ABLE */
extern constext txt_help_select_mark_group_unread[];
extern constext txt_help_select_move_group[];
extern constext txt_help_select_next_unread_group[];
extern constext txt_help_select_quit[];
extern constext txt_help_select_quit_no_write[];
extern constext txt_help_select_read_group[];
extern constext txt_help_select_reset_newsrc[];
extern constext txt_help_select_scope[];
extern constext txt_help_select_search_group_backwards[];
extern constext txt_help_select_search_group_comment[];
extern constext txt_help_select_search_group_forwards[];
extern constext txt_help_select_subscribe[];
extern constext txt_help_select_subscribe_pattern[];
extern constext txt_help_select_sync_with_active[];
extern constext txt_help_global_connection_info[];
extern constext txt_help_select_toggle_descriptions[];
extern constext txt_help_select_toggle_read_groups[];
extern constext txt_help_select_unsubscribe[];
extern constext txt_help_select_unsubscribe_pattern[];
extern constext txt_help_select_sort_active[];
extern constext txt_help_select_yank_active[];
extern constext txt_help_tag_parts[];
extern constext txt_help_thread_article_by_num[];
extern constext txt_help_thread_catchup[];
extern constext txt_help_thread_catchup_next_unread[];
extern constext txt_help_thread_first_article[];
extern constext txt_help_thread_last_article[];
extern constext txt_help_thread_mark_article_read[];
extern constext txt_help_thread_mark_article_unread[];
extern constext txt_help_thread_mark_thread_unread[];
extern constext txt_help_thread_read_article[];
extern constext txt_help_title_disp[];
extern constext txt_help_title_misc[];
extern constext txt_help_title_navi[];
extern constext txt_help_title_ops[];
extern constext txt_help_title_attachment_ops[];
extern constext txt_help_title_attrib_ops[];
extern constext txt_help_title_config_ops[];
extern constext txt_help_title_post_hist_ops[];
extern constext txt_help_title_scope_ops[];
extern constext txt_help_title_url_ops[];
extern constext txt_help_url_first_url[];
extern constext txt_help_url_goto_url[];
extern constext txt_help_url_last_url[];
extern constext txt_help_url_search_forwards[];
extern constext txt_help_url_search_backwards[];
extern constext txt_help_url_select[];
extern constext txt_help_url_toggle_info_line[];
extern constext txt_index_page_com[];
extern constext txt_info_add_kill[];
extern constext txt_info_add_select[];
extern constext txt_info_all_parts_tagged[];
extern constext txt_info_all_parts_untagged[];
extern constext txt_info_building_ref_tree[];
extern constext txt_info_do_postpone[];
extern constext txt_info_enter_valid_character[];
extern constext txt_info_missing_part[];
extern constext txt_info_nopostponed[];
extern constext txt_info_not_multipart_message[];
extern constext txt_info_not_subscribed[];
extern constext txt_info_no_write[];
extern constext txt_info_no_previous_expression[];
extern constext txt_info_postponed[];
extern constext txt_info_x_conversion_note[];
extern constext txt_invalid_from[];
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern constext txt_invalid_multibyte_sequence[];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
extern constext txt_inverse_off[];
extern constext txt_inverse_on[];
extern constext txt_is_mailbox[];
extern constext txt_is_tex_encoded[];
extern constext txt_keymap_missing_key[];
extern constext txt_keymap_invalid_key[];
extern constext txt_keymap_invalid_name[];
extern constext txt_keymap_upgraded[];
extern constext txt_kill_from[];
extern constext txt_kill_lines[];
extern constext txt_kill_menu[];
extern constext txt_kill_msgid[];
extern constext txt_kill_scope[];
extern constext txt_kill_subj[];
extern constext txt_kill_text[];
extern constext txt_kill_time[];
extern constext txt_last[];
extern constext txt_last_resp[];
extern constext txt_lines[];
#if defined(NNTP_ABLE) && defined(DEBUG)
	extern constext txt_log_data_hidden[];
#endif /* NNTP_ABLE && DEBUG */
extern constext txt_lookup_func_not_available[];
extern constext txt_lookup_func_not_nntp[];
#ifdef NNTP_ABLE
	extern constext txt_lookup_show_group[];
	extern constext txt_lookup_show_groups[];
#endif /* NNTP_ABLE */
extern constext txt_mail[];
extern constext txt_mailbox[];
extern constext txt_mail_art_to[];
extern constext txt_mail_log_to[];
extern constext txt_mail_bug_report[];
extern constext txt_mail_bug_report_confirm[];
extern constext txt_mailed[];
extern constext txt_mailing_to[];
extern constext txt_mail_save_active_head[];
extern constext txt_mark[];
extern constext txt_mark_arts_read[];
extern constext txt_mark_art_read[];
extern constext txt_mark_group_read[];
extern constext txt_mark_thread_read[];
extern constext txt_marked_as_read[];
extern constext txt_marked_as_unread[];
extern constext txt_marked_arts_as_read[];
extern constext txt_marked_arts_as_unread[];
extern constext txt_matching_cmd_line_groups[];
extern constext txt_mini_attachment_1[];
extern constext txt_mini_attachment_2[];
extern constext txt_mini_attachment_3[];
extern constext txt_mini_group_1[];
extern constext txt_mini_group_2[];
extern constext txt_mini_group_3[];
extern constext txt_mini_info_1[];
extern constext txt_mini_info_2[];
extern constext txt_mini_page_1[];
extern constext txt_mini_page_2[];
extern constext txt_mini_page_3[];
extern constext txt_mini_post_hist_1[];
extern constext txt_mini_post_hist_2[];
extern constext txt_mini_scope_1[];
extern constext txt_mini_scope_2[];
extern constext txt_mini_select_1[];
extern constext txt_mini_select_2[];
extern constext txt_mini_select_3[];
extern constext txt_mini_thread_1[];
extern constext txt_mini_thread_2[];
extern constext txt_mini_url_1[];
extern constext txt_mini_url_2[];
extern constext txt_more[];
#ifdef NNTP_ABLE
	extern constext txt_motd[];
#endif /* NNTP_ABLE */
extern constext txt_moving[];
extern constext txt_msgid_line_last[];
extern constext txt_msgid_line_only[];
extern constext txt_msgid_refs_line[];
extern constext txt_newsgroup[];
extern constext txt_newsgroup_plural[];
extern constext txt_newsgroup_position[];
extern constext txt_newsgroup_singular[];
extern constext txt_newsrc_again[];
extern constext txt_newsrc_nogroups[];
extern constext txt_newsrc_saved[];
extern constext txt_next_resp[];
extern constext txt_no[];
extern constext txt_no_arts[];
extern constext txt_no_arts_posted[];
extern constext txt_no_attachments[];
extern constext txt_no_description[];
extern constext txt_no_filename[];
extern constext txt_no_group[];
extern constext txt_no_groups[];
extern constext txt_no_groups_to_read[];
extern constext txt_no_last_message[];
extern constext txt_no_mail_address[];
extern constext txt_no_marked_arts[];
extern constext txt_no_match[];
extern constext txt_no_more_groups[];
extern constext txt_no_newsgroups[];
extern constext txt_no_next_unread_art[];
extern constext txt_no_prev_group[];
extern constext txt_no_prev_search[];
extern constext txt_no_prev_unread_art[];
extern constext txt_no_responses[];
extern constext txt_no_resps_in_thread[];
extern constext txt_no_scopes[];
extern constext txt_no_search_string[];
extern constext txt_no_subject[];
#ifndef USE_CURSES
	extern constext txt_no_term_clear_eol[];
	extern constext txt_no_term_clear_eos[];
	extern constext txt_no_term_clearscreen[];
	extern constext txt_no_term_cursor_motion[];
	extern constext txt_no_term_set[];
#endif /* !USE_CURSES */
extern constext txt_no_viewer_found[];
#ifdef NNTP_ABLE
	extern constext txt_no_xover_support[];
#endif /* NNTP_ABLE */
extern constext txt_none[];
extern constext txt_not_exist[];
extern constext txt_not_in_active_file[];
#ifdef NNTP_ABLE
	extern constext txt_nrctbl_create[];
	extern constext txt_nrctbl_default[];
	extern constext txt_nrctbl_info[];
#endif /* NNTP_ABLE */
extern constext txt_null[];
extern constext txt_only[];
extern constext txt_option_not_enabled[];
extern constext txt_options_menu[];
extern constext txt_options_menu_com[];
extern constext txt_out_of_memory[];
extern constext txt_pcre_error_at[];
extern constext txt_pcre_error_num[];
#ifndef HAVE_LIB_PCRE2
	extern constext txt_pcre_error_text[];
#endif /* !HAVE_LIB_PCRE2 */
#ifdef NNTP_ABLE
	extern constext txt_port_not_numeric[];
#	ifdef DEBUG
	extern constext txt_port_not_numeric_in[];
#	endif /* DEBUG */
#endif /* NNTP_ABLE */
extern constext txt_post_a_followup[];
extern constext txt_post_error_ask_postpone[];
extern constext txt_post_history_menu[];
extern constext txt_post_history_menu_com[];
extern constext txt_post_history_lookup_failed[];
extern constext txt_post_history_op_unavail_for_reply[];
extern constext txt_post_history_recursion[];
extern constext txt_post_newsgroups[];
extern constext txt_post_processing[];
extern constext txt_post_processing_finished[];
extern constext txt_post_subject[];
#ifdef NNTP_INEWS
	extern constext txt_post_via_builtin_inews[];
	extern constext txt_post_via_builtin_inews_only[];
#endif /* NNTP_INEWS */
extern constext txt_posted_info_file[];
extern constext txt_posting[];
#ifdef NNTP_INEWS
	extern constext txt_posting_failed[];
#endif /* NNTP_INEWS */
extern constext txt_postpone_post[];
extern constext txt_postpone_repost[];
extern constext txt_prefix_hot[];
extern constext txt_prefix_tagged[];
extern constext txt_prefix_untagged[];
#ifdef NNTP_ABLE
	extern constext txt_prep_for_filter_on_path[];
#endif /* NNTP_ABLE */
extern constext txt_prompt_fup_ignore[];
extern constext txt_prompt_unchanged_mail[];
extern constext txt_prompt_see_postponed[];
extern constext txt_quick_filter_kill[];
extern constext txt_quick_filter_select[];
extern constext txt_quit[];
extern constext txt_quit_cancel[];
extern constext txt_quit_despite_tags[];
extern constext txt_quit_edit_post[];
extern constext txt_quit_edit_postpone[];
extern constext txt_quit_edit_save_kill[];
extern constext txt_quit_edit_save_select[];
extern constext txt_quit_edit_send[];
extern constext txt_quit_edit_xpost[];
extern constext txt_quit_no_write[];
extern constext txt_range_invalid[];
#ifdef HAVE_SELECT
	extern constext txt_read_abort[];
	extern constext txt_read_exit[];
#endif /* HAVE_SELECT */
extern constext txt_reading_article[];
extern constext txt_reading_arts[];
extern constext txt_reading_attributes_file[];
extern constext txt_reading_config_file[];
extern constext txt_reading_filter_file[];
#ifdef DEBUG
	extern constext txt_reading_from_spool[];
#endif /* DEBUG */
extern constext txt_reading_group[];
extern constext txt_reading_groups[];
extern constext txt_reading_input_history_file[];
extern constext txt_reading_keymap_file[];
extern constext txt_reading_news_active_file[];
extern constext txt_reading_news_newsrc_file[];
extern constext txt_reading_newsgroups_file[];
extern constext txt_reading_newsrc[];
#ifdef NNTP_ABLE
#	ifdef DEBUG
		extern constext txt_reconnect_limit_reached[];
#	endif /* DEBUG */
	extern constext txt_reconnect_to_news_server[];
#endif /* NNTP_ABLE */
extern constext txt_refs_line_only[];
#if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
	extern constext txt_remaining[];
#endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */
extern constext txt_remove_bogus[];
extern constext txt_removed_rule[];
extern constext txt_rename_error[];
extern constext txt_reply_to_author[];
extern constext txt_repost[];
extern constext txt_repost_an_article[];
extern constext txt_repost_group[];
extern constext txt_reset_newsrc[];
extern constext txt_resp_redirect[];
extern constext txt_resp_to_poster[];
extern constext txt_return_key[];
extern constext txt_save[];
extern constext txt_save_attachment[];
extern constext txt_save_config[];
extern constext txt_save_filename[];
extern constext txt_saved[];
extern constext txt_saved_group[];
extern constext txt_saved_groupname[];
extern constext txt_saved_nothing[];
extern constext txt_saved_summary[];
extern constext txt_saved_to[];
extern constext txt_saved_to_range[];
extern constext txt_saving[];
extern constext txt_screen_init_failed[];
#ifndef USE_CURSES
	extern constext txt_screen_too_small[];
#endif /* !USE_CURSES */
extern constext txt_screen_too_small_exiting[];
extern constext txt_scope_delete[];
extern constext txt_scope_enter[];
extern constext txt_scope_new_position[];
extern constext txt_scope_new_position_is_global[];
extern constext txt_scope_operation_not_allowed[];
extern constext txt_scope_rename[];
extern constext txt_scope_select[];
extern constext txt_scopes_menu[];
extern constext txt_scopes_menu_com[];
extern constext txt_search_backwards[];
extern constext txt_search_body[];
extern constext txt_search_forwards[];
extern constext txt_searching[];
extern constext txt_searching_body[];
extern constext txt_select_art[];
extern constext txt_select_config_file_option[];
extern constext txt_select_from[];
extern constext txt_select_group[];
extern constext txt_select_lines[];
extern constext txt_select_menu[];
extern constext txt_select_msgid[];
extern constext txt_select_pattern[];
extern constext txt_select_scope[];
extern constext txt_select_subj[];
extern constext txt_select_text[];
extern constext txt_select_thread[];
extern constext txt_select_time[];
extern constext txt_selection_flag_insecure[];
extern constext txt_selection_flag_secure[];
extern constext txt_selection_flag_only_unread[];
extern constext txt_send_bugreport[];
#ifdef NNTP_ABLE
	extern constext txt_server_name_in_file_env_var[];
#endif /* NNTP_ABLE */
extern constext txt_serverconfig_header[];
extern constext txt_servers_active[];
extern constext txt_skipped_group[];
extern constext txt_skipping_newgroups[];
extern constext txt_space[];
extern constext txt_starting_command[];
extern constext txt_stp_list_thread[];
extern constext txt_stp_thread[];
extern constext txt_subj_line_only[];
extern constext txt_subj_line_only_case[];
extern constext txt_subscribe_pattern[];
extern constext txt_subscribed_num_groups[];
extern constext txt_subscribed_to[];
extern constext txt_subscribing[];
extern constext txt_supersede_article[];
extern constext txt_supersede_group[];
extern constext txt_superseding_art[];
extern constext txt_suspended_message[];
extern constext txt_tab[];
extern constext txt_tex[];
extern constext txt_there_is_no_news[];
extern constext txt_thread_upper[];
extern constext txt_thread_com[];
extern constext txt_thread_marked_as_deselected[];
extern constext txt_thread_marked_as_selected[];
extern constext txt_thread_singular[];
extern constext txt_thread_x_of_n[];
extern constext txt_threading_arts[];
extern constext txt_threading_by_multipart[];
extern constext txt_time_default_days[];
extern constext txt_tin_version[];
extern constext txt_tinrc_defaults[];
extern constext txt_tinrc_filter[];
extern constext txt_tinrc_header[];
extern constext txt_tinrc_info_in_last_line[];
extern constext txt_tinrc_newnews[];
#if defined(NNTP_ABLE) && defined(NNTPS_ABLE)
	extern constext txt_tls_handshake_done[];
#	ifdef HAVE_LIB_LIBTLS
	extern constext txt_retr_cipher_failed[];
	extern constext txt_retr_issuer_failed[];
	extern constext txt_retr_subject_failed[];
	extern constext txt_retr_version_failed[];
#	endif /* HAVE_LIB_LIBTLS */
#	ifdef HAVE_LIB_GNUTLS
	extern constext txt_tls_handshake_failed_with_err_num[];
	extern constext txt_tls_peer_verify_failed[];
	extern constext txt_tls_unable_to_get_status[];
	extern constext txt_tls_unexpected_status[];
#	endif /* HAVE_LIB_GNUTLS */
#	if defined(HAVE_LIB_GNUTLS) || defined(HAVE_LIB_OPENSSL)
	extern constext txt_tls_peer_verify_failed_continuing[];
#	endif /* HAVE_LIB_GNUTLS || HAVE_LIB_OPENSSL */
#	if defined(USE_LIBTLS) || defined(USE_OPENSSL)
	extern constext txt_tls_handshake_failed[];
#	endif /* USE_LIBTLS || USE_OPENSSL */
#endif /* NNTP_ABLE && NNTPS_ABLE */
extern constext txt_toggled_high[];
extern constext txt_toggled_rot13[];
extern constext txt_toggled_tex2iso[];
extern constext txt_toggled_tabwidth[];
#if defined(NNTP_ABLE) && defined(HAVE_INET_NTOA) && !defined(INET6)
	extern constext txt_trying[];
#endif /* NNTP_ABLE && HAVE_INET_NTOA && !INET6 */
#ifndef NO_LOCKING
	extern constext txt_trying_dotlock[];
	extern constext txt_trying_lock[];
#endif /* NO_LOCKING */
extern constext txt_type_h_for_help[];
extern constext txt_unlimited_time[];
extern constext txt_unchanged[];
extern constext txt_unknown[];
#if defined(XFACE_ABLE) || (defined(NNTPS_ABLE) && defined(HAVE_LIB_LIBTLS))
	extern constext txt_unknown_error[];
#endif /* XFACE_ABLE || (NNTPS_ABLE && HAVE_LIB_LIBTLS) */
extern constext txt_unread[];
extern constext txt_unsubscribe_pattern[];
extern constext txt_unsubscribed_num_groups[];
extern constext txt_unsubscribed_to[];
extern constext txt_unsubscribing[];
extern constext txt_unthreading_arts[];
extern constext txt_updated[];
extern constext txt_updating[];
extern constext txt_url_menu[];
extern constext txt_url_menu_com[];
extern constext txt_url_open[];
extern constext txt_url_select[];
extern constext txt_url_done[];
extern constext txt_usage_catchup[];
extern constext txt_usage_check_for_unread_news[];
#ifdef NNTP_ABLE
#	ifdef USE_GSASL
	extern constext txt_usable_sasl_mechs[];
	extern constext txt_used_sasl_mech[];
#	endif /* USE_GSASL */
#	ifdef USE_ZLIB
	extern constext txt_continuing[];
	extern constext txt_read_timeout_quit[];
	extern constext txt_usage_compress[];
#	endif /* USE_ZLIB */
#endif /* NNTP_ABLE */
#ifdef DEBUG
	extern constext txt_usage_debug[];
#endif /* DEBUG */
extern constext txt_usage_dont_check_new_newsgroups[];
extern constext txt_usage_dont_save_files_on_quit[];
extern constext txt_usage_dont_show_descriptions[];
#ifdef NNTP_ABLE
	extern constext txt_usage_force_authentication[];
	extern constext txt_usage_lookup_id[];
#	ifdef INET6
	extern constext txt_usage_force_ipv4[];
	extern constext txt_usage_force_ipv6[];
#	endif /* INET6 */
	extern constext txt_usage_newsserver[];
#if defined(HAVE_ALARM) && defined(SIGALRM)
	extern constext txt_usage_nntp_timeout[];
#endif /* HAVE_ALARM && SIGALRM */
	extern constext txt_usage_port[];
	extern constext txt_usage_read_news_remotely[];
#	ifdef NNTPS_ABLE
	extern constext txt_usage_use_insecure_nntps[];
	extern constext txt_usage_use_nntps[];
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */
extern constext txt_usage_getart_limit[];
extern constext txt_usage_help_information[];
extern constext txt_usage_help_message[];
extern constext txt_usage_index_newsdir[];
extern constext txt_usage_update_index_files[];
extern constext txt_usage_maildir[];
extern constext txt_usage_mail_bugreport[];
extern constext txt_usage_mail_new_news[];
extern constext txt_usage_mail_new_news_to_user[];
extern constext txt_usage_newsrc_file[];
extern constext txt_usage_no_posting[];
extern constext txt_usage_post_article[];
extern constext txt_usage_post_postponed_arts[];
extern constext txt_usage_read_saved_news[];
extern constext txt_usage_savedir[];
extern constext txt_usage_save_new_news[];
extern constext txt_usage_start_if_unread_news[];
extern constext txt_usage_tin[];
#ifdef HAVE_COLOR
	extern constext txt_usage_toggle_color[];
#endif /* HAVE_COLOR */
extern constext txt_usage_quickstart[];
extern constext txt_usage_read_only_subscribed[];
extern constext txt_usage_read_only_active[];
extern constext txt_usage_verbose[];
extern constext txt_usage_version[];
extern constext txt_useful_without_batch_mode[];
extern constext txt_useful_with_batch_mode[];
extern constext txt_useful_with_batch_or_debug_mode[];
extern constext txt_useless_combination[];
extern constext txt_use_mime[];
extern constext txt_uue_complete[];
extern constext txt_uue_incomplete[];
#if defined(NNTP_ABLE) && defined(NNTPS_ABLE)
	extern constext txt_valid_not_after[];
	extern constext txt_valid_not_before[];
#endif /* NNTP_ABLE && NNTPS_ABLE */
extern constext txt_value_out_of_range[];
extern constext txt_view_attachment[];
extern constext txt_warn_art_line_too_long[];
extern constext txt_warn_article_unchanged[];
extern constext txt_warn_blank_subject[];
extern constext txt_warn_cancel[];
#ifdef CHARSET_CONVERSION
	extern constext txt_warn_charset_conversion[];
#endif /* CHARSET_CONVERSION */
extern constext txt_warn_distribution_world[];
extern constext txt_warn_downgrade[];
extern constext txt_warn_encoding_and_external_inews[];
extern constext txt_warn_long_line_not_base[];
extern constext txt_warn_long_line_not_break[];
#ifdef MIME_BREAK_LONG_LINES
	extern constext txt_warn_long_line_not_qp[];
#endif /* MIME_BREAK_LONG_LINES */
#ifdef ALLOW_FWS_IN_NEWSGROUPLIST
	extern constext txt_warn_header_line_comma[];
	extern constext txt_warn_header_line_groups_contd[];
#endif /* ALLOW_FWS_IN_NEWSGROUPLIST */
extern constext txt_warn_example_hierarchy[];
extern constext txt_warn_multiple_addresses[];
extern constext txt_warn_multiple_sigs[];
extern constext txt_warn_newsrc[];
extern constext txt_warn_not_all_arts_saved[];
extern constext txt_warn_re_but_no_references[];
extern constext txt_warn_references_but_no_re[];
extern constext txt_warn_sig_too_long[];
extern constext txt_warn_suspicious_mail[];
extern constext txt_warn_update[];
extern constext txt_warn_unprintable_char[];
extern constext txt_warn_unrecognized_version[];
extern constext txt_warn_wrong_sig_format[];
extern constext txt_warn_xref_not_supported[];
extern constext txt_writing_attributes_file[];
extern constext txt_writing_group[];
extern constext txt_writing_overview[];
extern constext txt_x_resp[];
#ifdef XFACE_ABLE
	extern constext txt_xface_error_construct_fifo_name[];
	extern constext txt_xface_error_create_failed[];
	extern constext txt_xface_error_exited_abnormal[];
	extern constext txt_xface_error_finally_failed[];
	extern constext txt_xface_error_missing_env_var[];
#	if defined(DEBUG) && defined(HAVE_IS_XTERM)
	extern constext txt_xface_error_no_xterm[];
#	endif /* DEBUG && HAVE_IS_XTERM */
	extern constext txt_xface_msg_cannot_connect_display[];
	extern constext txt_xface_msg_cannot_open_fifo[];
	extern constext txt_xface_msg_executable_not_found[];
	extern constext txt_xface_msg_fork_failed[];
	extern constext txt_xface_msg_no_controlling_terminal[];
	extern constext txt_xface_msg_no_width_and_height_avail[];
	extern constext txt_xface_msg_windowid_not_found[];
	extern constext txt_xface_readme[];
#endif /* XFACE_ABLE */
#if defined(NNTP_ABLE) && defined(XHDR_XREF)
	extern constext txt_xref_loop[];
#endif /* NNTP_ABLE && XHDR_XREF */
extern constext txt_yanked_groups[];
extern constext txt_yanked_none[];
extern constext txt_yanked_sub_groups[];
extern constext txt_show_unread[];
extern constext txt_yes[];
extern constext txt_you_have_mail[];

#ifndef DISABLE_PRINTING
	extern constext txt_print[];
	extern constext txt_printing[];
#endif /* !DISABLE_PRINTING */

#ifndef DONT_HAVE_PIPING
	extern constext txt_no_command[];
#endif /* !DONT_HAVE_PIPING */

#ifndef HAVE_FASCIST_NEWSADMIN
	extern constext txt_warn_followup_to_several_groups[];
	extern constext txt_warn_grp_renamed[];
	extern constext txt_warn_missing_followup_to[];
	extern constext txt_warn_not_in_newsrc[];
	extern constext txt_warn_not_valid_newsgroup[];
#endif /* !HAVE_FASCIST_NEWSADMIN */

#ifdef HAVE_LIBUU
	extern constext txt_libuu_saved[];
	extern constext txt_libuu_error_missing[];
	extern constext txt_libuu_error_no_begin[];
	extern constext txt_libuu_error_no_data[];
	extern constext txt_libuu_error_unknown[];
#endif /* HAVE_LIBUU */
extern constext txt_uu_error_decode[];
extern constext txt_uu_error_no_end[];
extern constext txt_uu_success[];

#if defined(NNTP_ABLE) && !defined(NNTP_ONLY)
	extern constext txt_cannot_open_active_file[];
#endif /* NNTP_ABLE && !NNTP_ONLY */

#ifndef NO_SHELL_ESCAPE
	extern constext txt_shell_command[];
	extern constext txt_shell_escape[];
#endif /* !NO_SHELL_ESCAPE */

extern int *my_group;
extern int NOTESLINES;
extern int _hp_glitch;
extern int art_mark_width;
extern int attrib_file_offset;
extern int cCOLS;
extern int cLINES;
extern int curr_line;
extern int filter_file_offset;
extern int input_context;
extern int iso2asc_supported;
extern int last_resp;
extern int mark_offset;
extern int max_active;
extern int max_art;
extern int max_base;
extern int max_newnews;
extern int max_save;
extern int max_scope;
extern int need_resize;
extern int num_active;
extern int num_newnews;
extern int num_of_tagged_arts;
extern int num_save;
extern int num_scope;
extern int signal_context;
extern int srch_lineno;
extern int system_status;
extern int this_resp;
extern int thread_basenote;
extern int tin_errno;
extern int top_art;
extern int xcol;
extern int xmouse;
extern int xrow;

extern unsigned short debug;

extern t_artnum *base;

extern signed long int read_newsrc_lines;

extern size_t tabwidth;

extern pid_t process_id;

#ifdef USE_HEAPSORT
	extern int tin_sort(void *, size_t, size_t, t_compfunc);
	extern constext *txt_sort_functions[];
	extern struct opttxt txt_sort_function;
#endif /* USE_HEAPSORT */

extern struct regex_cache strip_re_regex;
extern struct regex_cache strip_was_regex;
extern struct regex_cache uubegin_regex;
extern struct regex_cache uubody_regex;
extern struct regex_cache verbatim_begin_regex;
extern struct regex_cache verbatim_end_regex;
extern struct regex_cache url_regex;
extern struct regex_cache mail_regex;
extern struct regex_cache news_regex;
extern struct regex_cache shar_regex;
extern struct regex_cache slashes_regex;
extern struct regex_cache stars_regex;
extern struct regex_cache underscores_regex;
extern struct regex_cache strokes_regex;
#ifdef HAVE_COLOR
	extern struct regex_cache extquote_regex;
	extern struct regex_cache quote_regex;
	extern struct regex_cache quote_regex2;
	extern struct regex_cache quote_regex3;
#endif /* HAVE_COLOR */

extern struct t_article *arts;
extern struct t_scope *scopes;
extern struct t_cmdlineopts cmdline;
extern struct t_config tinrc;
extern struct t_filters glob_filter;
extern struct t_group *active;
extern struct t_group *curr_group;
extern struct t_newnews *newnews;
extern struct t_option option_table[];
extern struct t_save *save;
extern struct t_capabilities nntp_caps;

extern t_bool *OPT_ON_OFF_list[];
extern t_bool can_post;
extern t_bool check_for_new_newsgroups;
extern t_bool cmd_line;
extern t_bool created_rcdir;
extern t_bool dangerous_signal_exit; /* TRUE if SIGHUP, SIGTERM, SIGUSR1 */
#ifdef NNTP_ABLE
	extern t_bool did_reconnect;
	extern t_bool reconnected_in_last_get_server;
#	ifdef USE_ZLIB
		extern t_bool use_compress;
#	endif /* USE_ZLIB */
#endif /* NNTP_ABLE */
extern t_bool disable_gnksa_domain_check;
extern t_bool disable_sender;
extern t_bool force_no_post;
extern t_bool force_reread_active_file;
#if defined(NNTP_ABLE) && defined(INET6)
	extern t_bool force_ipv4;
	extern t_bool force_ipv6;
#endif /* NNTP_ABLE && INET6 */
extern t_bool have_linescroll;
extern t_bool list_active;
extern t_bool newsrc_active;
extern t_bool no_write;
extern t_bool post_article_and_exit;
extern t_bool post_postponed_and_exit;
extern t_bool range_active;
extern t_bool read_local_newsgroups_file;
extern t_bool read_news_via_nntp;
extern t_bool read_saved_news;
extern t_bool reread_active_for_posted_arts;
extern t_bool show_description;
extern t_bool show_subject;
extern t_bool use_nntps;
extern t_bool insecure_nntps;
extern t_bool batch_mode;
extern int verbose;
extern t_bool xref_supported;
extern t_bool expensive_over_parse;

extern t_function last_search;

extern t_menu selmenu;
extern t_menu grpmenu;
extern t_menu *currmenu;

extern t_openartinfo pgart;

extern struct t_overview_fmt *ofmt;

enum {
	HIST_OTHER = 0,
	HIST_ART_SEARCH,
	HIST_AUTHOR_SEARCH,
	HIST_GOTO_GROUP,
	HIST_GROUP_SEARCH,
	HIST_MAIL_ADDRESS,
	HIST_MESSAGE_ID,
	HIST_MOVE_GROUP,
	HIST_PIPE_COMMAND,
	HIST_POST_NEWSGROUPS,
	HIST_POST_SUBJECT,
	HIST_REGEX_PATTERN,
	HIST_REPOST_GROUP,
	HIST_SAVE_FILE,
	HIST_SELECT_PATTERN,
	HIST_SHELL_COMMAND,
	HIST_SUBJECT_SEARCH,
	HIST_CONFIG_SEARCH,
	HIST_URL
};
/* must always be the same as the highest HIST_ value except HIST_NONE */
#define HIST_MAXNUM		HIST_URL
#define HIST_NONE		(HIST_MAXNUM + 1)
#define HIST_SIZE		15	/* # items in each history */

extern int hist_last[HIST_MAXNUM + 1];
extern int hist_pos[HIST_MAXNUM + 1];
extern char *input_history[HIST_MAXNUM + 1][HIST_SIZE + 1];
extern long motd_hash;

/* defines for GNKSA checking */
/* success/undefined failure */
#define GNKSA_OK			0
#define GNKSA_INTERNAL_ERROR		1
/* general syntax */
#define GNKSA_LANGLE_MISSING		100
#define GNKSA_LPAREN_MISSING		101
#define GNKSA_RPAREN_MISSING		102
#define GNKSA_ATSIGN_MISSING		103
#define GNKSA_RANGLE_MISSING		104
/* FQDN checks */
#define GNKSA_SINGLE_DOMAIN		200
#define GNKSA_INVALID_DOMAIN		201
#define GNKSA_ILLEGAL_DOMAIN		202
#define GNKSA_UNKNOWN_DOMAIN		203
#define GNKSA_INVALID_FQDN_CHAR		204
#define GNKSA_ZERO_LENGTH_LABEL		205
#define GNKSA_ILLEGAL_LABEL_LENGTH	206
#define GNKSA_ILLEGAL_LABEL_HYPHEN	207
#define GNKSA_ILLEGAL_LABEL_BEGNUM	208
#define GNKSA_BAD_DOMAIN_LITERAL	209
#define GNKSA_LOCAL_DOMAIN_LITERAL	210
#define GNKSA_RBRACKET_MISSING		211
/* localpart checks */
#define GNKSA_LOCALPART_MISSING		300
#define GNKSA_INVALID_LOCALPART		301
#define GNKSA_ZERO_LENGTH_LOCAL_WORD	302
/* realname checks */
#define GNKSA_ILLEGAL_UNQUOTED_CHAR	400
#define GNKSA_ILLEGAL_QUOTED_CHAR	401
#define GNKSA_ILLEGAL_ENCODED_CHAR	402
#define GNKSA_BAD_ENCODE_SYNTAX		403
#define GNKSA_ILLEGAL_PAREN_CHAR		404
#define GNKSA_INVALID_REALNAME		405
#define GNKSA_MISSING_REALNAME		406

/* address types */
#define GNKSA_ADDRTYPE_ROUTE	0
#define GNKSA_ADDRTYPE_OLDSTYLE	1

#ifndef DONT_HAVE_PIPING
	extern constext txt_pipe[];
	extern constext txt_pipe_to_command[];
	extern constext txt_piping[];
#else
	extern constext txt_piping_not_enabled[];
#endif /* !DONT_HAVE_PIPING */

#ifdef FORGERY
	extern constext txt_warn_cancel_forgery[];
#else
	extern constext txt_art_cannot_cancel[];
	extern constext txt_error_sender_in_header_not_allowed[];
#	ifdef NNTP_INEWS
	extern constext txt_invalid_sender[];
#	endif /* NNTP_INEWS */
#endif /* FORGERY */

extern t_bool word_highlight;
#ifdef HAVE_COLOR
	extern constext txt_tinrc_colors[];
	extern int default_bcol;
	extern int default_fcol;
	extern t_bool use_color;
#	ifdef USE_CURSES
		extern constext txt_no_colorterm[];
#	endif /* USE_CURSES */
#endif /* HAVE_COLOR */

#ifdef HAVE_FASCIST_NEWSADMIN
	extern constext txt_error_followup_to_several_groups[];
	extern constext txt_error_grp_renamed[];
	extern constext txt_error_missing_followup_to[];
	extern constext txt_error_not_valid_newsgroup[];
#endif /* HAVE_FASCIST_NEWSADMIN */


#ifndef ALLOW_FWS_IN_NEWSGROUPLIST
	extern constext txt_error_header_line_comma[];
	extern constext txt_error_header_line_groups_contd[];
#endif /* !ALLOW_FWS_IN_NEWSGROUPLIST */

#ifdef HAVE_PGP_GPG
	extern constext txt_pgp_add[];
	extern constext txt_pgp_mail[];
	extern constext txt_pgp_news[];
	extern constext txt_pgp_not_avail[];
	extern constext txt_pgp_nothing[];
#endif /* HAVE_PGP_GPG */

#ifdef HAVE_SYS_UTSNAME_H
	extern struct utsname system_info;
#endif /* HAVE_SYS_UTSNAME_H */

extern constext txt_art_deleted[];
extern constext txt_art_undeleted[];
extern constext txt_intro_page[];
extern constext txt_processing_attributes[];
extern constext txt_processing_mail_arts[];
extern constext txt_processing_saved_arts[];

#ifdef HAVE_MH_MAIL_HANDLING
	extern constext txt_reading_mail_active_file[];
	extern constext txt_reading_mailgroups_file[];
#endif /* HAVE_MH_MAIL_HANDLING */

#ifndef NO_ETIQUETTE
	extern constext txt_warn_posting_etiquette[];
#endif /* NO_ETIQUETTE */

#if !defined(USE_CURSES)
	extern struct t_screen *screen;
#endif /* !USE_CURSES */

#ifdef NNTP_ABLE
	extern constext txt_nntp_ok_goodbye[];
	extern unsigned short nntp_tcp_port;
	extern unsigned short nntp_tcp_default_port;
#ifdef NNTPS_ABLE
	extern unsigned short nntps_tcp_default_port;
#endif /* NNTPS_ABLE */
	extern t_bool force_auth_on_conn_open;
#endif /* NNTP_ABLE */

extern struct opttxt txt_abbreviate_groupname;
extern struct opttxt txt_add_posted_to_filter;
extern struct opttxt txt_advertising;
extern struct opttxt txt_alternative_handling;
extern struct opttxt txt_art_marked_deleted;
extern struct opttxt txt_art_marked_inrange;
extern struct opttxt txt_art_marked_killed;
extern struct opttxt txt_art_marked_read;
extern struct opttxt txt_art_marked_read_selected;
extern struct opttxt txt_art_marked_recent;
extern struct opttxt txt_art_marked_return;
extern struct opttxt txt_art_marked_selected;
extern struct opttxt txt_art_marked_unread;
extern struct opttxt txt_ask_for_metamail;
extern struct opttxt txt_attachment_format;
extern struct opttxt txt_auto_cc_bcc;
extern struct opttxt txt_auto_list_thread;
extern struct opttxt txt_auto_reconnect;
extern struct opttxt txt_auto_select;
extern struct opttxt txt_delete_tmp_files;
extern struct opttxt txt_batch_save;
extern struct opttxt txt_beginner_level;
extern struct opttxt txt_cache_overview_files;
#ifdef USE_CANLOCK
	extern struct opttxt txt_cancel_lock_algo;
#endif /* USE_CANLOCK */
extern struct opttxt txt_catchup_read_groups;
#ifdef HAVE_COLOR
	extern struct opttxt txt_color_options;
#else
	extern struct opttxt txt_highlight_options;
#endif /* HAVE_COLOR */
extern struct opttxt txt_confirm_choice;
extern struct opttxt txt_date_format;
extern struct opttxt txt_display_options;
extern struct opttxt txt_dont_break_words;
extern struct opttxt txt_draw_arrow;
extern struct opttxt txt_editor_format;
extern struct opttxt txt_expert_options;
extern struct opttxt txt_fcc;
extern struct opttxt txt_filter_days;
extern struct opttxt txt_filtering_options;
extern struct opttxt txt_followup_to;
extern struct opttxt txt_force_screen_redraw;
extern struct opttxt txt_from;
extern struct opttxt txt_getart_limit;
extern struct opttxt txt_getart_limit_options;
extern struct opttxt txt_goto_next_unread;
extern struct opttxt txt_group_catchup_on_exit;
extern struct opttxt txt_group_format;
extern struct opttxt txt_hide_uue;
extern struct opttxt txt_inews_prog;
extern struct opttxt txt_interactive_mailer;
extern struct opttxt txt_inverse_okay;
#ifdef HAVE_ISPELL
	extern struct opttxt txt_ispell;
#endif /* HAVE_ISPELL */
extern struct opttxt txt_keep_dead_articles;
extern struct opttxt txt_kill_level;
extern struct opttxt txt_mail_8bit_header;
extern struct opttxt txt_mail_address;
extern struct opttxt txt_mail_mime_encoding;
extern struct opttxt txt_mail_quote_format;
extern struct opttxt txt_mailbox_format;
extern struct opttxt txt_maildir;
extern struct opttxt txt_mailing_list;
extern struct opttxt txt_mailer_format;
extern struct opttxt txt_mark_ignore_tags;
extern struct opttxt txt_mark_saved_read;
extern struct opttxt txt_mime_forward;
extern struct opttxt txt_mime_types_to_save;
extern struct opttxt txt_mono_markstar;
extern struct opttxt txt_mono_markdash;
extern struct opttxt txt_mono_markslash;
extern struct opttxt txt_mono_markstroke;
#ifndef CHARSET_CONVERSION
	extern struct opttxt txt_mm_charset;
#else
#	ifdef NO_LOCALE
		extern struct opttxt txt_mm_local_charset;
#	endif /* NO_LOCALE */
#endif /* CHARSET_CONVERSION */
extern struct opttxt txt_metamail_prog;
extern struct opttxt txt_news_headers_to_display;
extern struct opttxt txt_news_headers_to_not_display;
extern struct opttxt txt_news_quote_format;
#if defined(HAVE_ALARM) && defined(SIGALRM)
	extern struct opttxt txt_nntp_read_timeout_secs;
#endif /* HAVE_ALARM && SIGALRM */
extern struct opttxt txt_organization;
extern struct opttxt txt_page_mime_format;
extern struct opttxt txt_page_uue_format;
extern struct opttxt txt_pos_first_unread;
extern struct opttxt txt_post_8bit_header;
extern struct opttxt txt_post_mime_encoding;
extern struct opttxt txt_post_process_type;
extern struct opttxt txt_post_process_view;
extern struct opttxt txt_posted_articles_file;
extern struct opttxt txt_posting_options;
#ifndef DISABLE_PRINTING
	extern struct opttxt txt_print_header;
	extern struct opttxt txt_printer;
#endif /* !DISABLE_PRINTING */
extern struct opttxt txt_process_only_unread;
extern struct opttxt txt_prompt_followupto;
extern struct opttxt txt_quick_select_scope;
extern struct opttxt txt_quick_select_header;
extern struct opttxt txt_quick_select_case;
extern struct opttxt txt_quick_select_expire;
extern struct opttxt txt_quick_kill_scope;
extern struct opttxt txt_quick_kill_header;
extern struct opttxt txt_quick_kill_case;
extern struct opttxt txt_quick_kill_expire;
extern struct opttxt txt_quote_chars;
extern struct opttxt txt_quote_style;
extern struct opttxt txt_recent_time;
extern struct opttxt txt_reread_active_file_secs;
extern struct opttxt txt_savedir;
extern struct opttxt txt_savefile;
extern struct opttxt txt_saving_options;
extern struct opttxt txt_score_limit_kill;
extern struct opttxt txt_score_limit_select;
extern struct opttxt txt_score_kill;
extern struct opttxt txt_score_select;
extern struct opttxt txt_scroll_lines;
extern struct opttxt txt_select_format;
extern struct opttxt txt_show_author;
extern struct opttxt txt_show_description;
extern struct opttxt txt_show_help_mail_sign;
extern struct opttxt txt_show_only_unread_arts;
extern struct opttxt txt_show_only_unread_groups;
extern struct opttxt txt_show_signatures;
extern struct opttxt txt_show_art_score;
extern struct opttxt txt_sigdashes;
extern struct opttxt txt_sigfile;
extern struct opttxt txt_signature_repost;
extern struct opttxt txt_slashes_regex;
extern struct opttxt txt_sort_article_type;
extern struct opttxt txt_sort_threads_type;
extern struct opttxt txt_spamtrap_warning_addresses;
extern struct opttxt txt_stars_regex;
#ifndef USE_CURSES
	extern struct opttxt txt_strip_blanks;
#endif /* !USE_CURSES */
extern struct opttxt txt_strip_bogus;
extern struct opttxt txt_strip_newsrc;
extern struct opttxt txt_strip_re_regex;
extern struct opttxt txt_strip_was_regex;
extern struct opttxt txt_strokes_regex;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern struct opttxt txt_suppress_soft_hyphens;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
extern struct opttxt txt_tex2iso_conv;
extern struct opttxt txt_thread_articles;
extern struct opttxt txt_thread_perc;
extern struct opttxt txt_thread_catchup_on_exit;
extern struct opttxt txt_thread_format;
extern struct opttxt txt_thread_score;
extern struct opttxt txt_trim_article_body;
#ifdef NNTPS_ABLE
extern struct opttxt txt_tls_ca_cert_file;
#endif /* NNTPS_ABLE */
extern struct opttxt txt_underscores_regex;
extern struct opttxt txt_unlink_article;
extern struct opttxt txt_url_handler;
extern struct opttxt txt_url_highlight;
extern struct opttxt txt_use_mouse;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	extern struct opttxt txt_utf8_graphics;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
extern struct opttxt txt_verbatim_begin_regex;
extern struct opttxt txt_verbatim_end_regex;
extern struct opttxt txt_verbatim_handling;
extern struct opttxt txt_wildcard;
extern struct opttxt txt_word_highlight;
extern struct opttxt txt_word_h_display_marks;
extern struct opttxt txt_wrap_column;
extern struct opttxt txt_wrap_on_next_unread;
extern struct opttxt txt_x_body;
extern struct opttxt txt_x_comment_to;
extern struct opttxt txt_x_headers;
extern struct opttxt txt_xpost_quote_format;
#ifdef CHARSET_CONVERSION
	extern struct opttxt txt_mm_network_charset;
	extern struct opttxt txt_undeclared_charset;
#	ifdef USE_ICU_UCSDET
	extern struct opttxt txt_undeclared_cs_guess;
#	endif /* USE_ICU_UCSDET */
#endif /* CHARSET_CONVERSION */
#ifdef HAVE_COLOR
	extern struct opttxt txt_quote_regex;
	extern struct opttxt txt_quote_regex2;
	extern struct opttxt txt_quote_regex3;
	extern struct opttxt txt_extquote_handling;
	extern struct opttxt txt_extquote_regex;
	extern struct opttxt txt_use_color;
	extern struct opttxt txt_col_normal;
	extern struct opttxt txt_col_back;
	extern struct opttxt txt_col_invers_bg;
	extern struct opttxt txt_col_invers_fg;
	extern struct opttxt txt_col_text;
	extern struct opttxt txt_col_minihelp;
	extern struct opttxt txt_col_help;
	extern struct opttxt txt_col_message;
	extern struct opttxt txt_col_quote;
	extern struct opttxt txt_col_quote2;
	extern struct opttxt txt_col_quote3;
	extern struct opttxt txt_col_extquote;
	extern struct opttxt txt_col_head;
	extern struct opttxt txt_col_newsheaders;
	extern struct opttxt txt_col_subject;
	extern struct opttxt txt_col_response;
	extern struct opttxt txt_col_from;
	extern struct opttxt txt_col_title;
	extern struct opttxt txt_col_signature;
	extern struct opttxt txt_col_score_neg;
	extern struct opttxt txt_col_score_pos;
	extern struct opttxt txt_col_urls;
	extern struct opttxt txt_col_verbatim;
	extern struct opttxt txt_col_markstar;
	extern struct opttxt txt_col_markdash;
	extern struct opttxt txt_col_markslash;
	extern struct opttxt txt_col_markstroke;
#endif /* HAVE_COLOR */
#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
	extern struct opttxt txt_translit;
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */
#ifdef HAVE_KEYPAD
	extern struct opttxt txt_use_keypad;
#endif /* HAVE_KEYPAD */
#ifdef XFACE_ABLE
	extern struct opttxt txt_use_slrnface;
#endif /* XFACE_ABLE */
#ifdef HAVE_UNICODE_NORMALIZATION
	extern struct opttxt txt_normalization_form;
#endif /* HAVE_UNICODE_NORMALIZATION */
#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
	extern struct opttxt txt_render_bidi;
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */
#endif /* !EXTERN_H */
