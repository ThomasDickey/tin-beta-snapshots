/*
 *  Project   : tin - a Usenet reader
 *  Module    : misc.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2025-06-18
 *  Notes     :
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef included_trace_h
#	include "trace.h"
#endif /* !included_trace_h */
#ifndef TIN_POLICY_H
#	include "policy.h"
#endif /* !TIN_POLICY_H */

#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */

/* GNU libidn */
#if defined(HAVE_IDNA_H) && !defined(_IDNA_H)
#	include <idna.h>
#endif /* HAVE_IDNA_H && !_IDNA_H */
#if defined(HAVE_STRINGPREP_H) && !defined(_STRINGPREP_H)
#	include <stringprep.h>
#endif /* HAVE_STRINGPREP_H && !_STRINGPREP_H */

/* GNU libidn2 */
#if defined(HAVE_LIBIDN2) && defined(HAVE_IDN2_H) && !defined(IDN2_H)
#	include <idn2.h>
#endif /* HAVE_LIBIDN2 && HAVE_IDN2_H && !IDN2_H */

/* JPRS libidnkit */
#if defined(HAVE_IDN_API_H) && !defined(IDN_API_H)
#	include <idn/api.h>
#	if defined(HAVE_IDN_VERSION_H) && !defined(REPRODUCIBLE_BUILD)
#		include <idn/version.h>
#	endif /* HAVE_IDN_VERSION_H && !REPRODUCIBLE_BUILD */
#endif /* HAVE_IDN_API_H && !IDN_API_H */

/* GNU libunistring */
#if defined(HAVE_LIBUNISTRING) && defined(HAVE_UNISTRING_VERSION_H) && !defined(REPRODUCIBLE_BUILD)
#	include <unistring/version.h>
#endif /* HAVE_LIBUNISTRING && HAVE_UNISTRING_VERSION_H && !REPRODUCIBLE_BUILD */

#ifdef NNTPS_ABLE
#	ifdef HAVE_LIB_LIBTLS
#		include <tls.h>
#	else
#		ifdef HAVE_LIB_OPENSSL
#			include <openssl/opensslv.h>
#			include <openssl/crypto.h>
#		else
#			ifdef HAVE_LIB_GNUTLS
#				include <gnutls/gnutls.h>
#			endif /* HAVE_LIB_GNUTLS */
#		endif /* HAVE_LIB_OPENSSL */
#	endif /* HAVE_LIB_LIBTLS */
#endif /* NNTPS_ABLE */

/* ICU */
#if defined(HAVE_LIBICUUC) || defined(USE_ICU_UCSDET)
#	include <unicode/uversion.h>
#	include <unicode/uclean.h>
#endif /* HAVE_LIBICUUC || USE_ICU_UCSDET */

#if defined(USE_ZLIB) && !defined(REPRODUCIBLE_BUILD)
#	include <zlib.h>
#endif /* USE_ZLIB && !REPRODUCIBLE_BUILD */

#ifdef HAVE_LIBURIPARSER
#	include <uriparser/Uri.h>
#else
#	ifdef HAVE_LIBCURL
#		include <curl/curl.h>
#	endif /* HAVE_LIBCURL */
#endif /* HAVE_LIBURIPARSER */

/*
 * defines to control GNKSA-checks behavior:
 * - ENFORCE_RFC1034
 *   require domain name components not to start with a digit
 *
 * - REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
 *   require domain literals to be enclosed in square brackets
 */

/*
 * Local prototypes
 */
static char *strfpath_cp(char *str, const char *tbuf, const char *endp);
static int _strfpath(const char *format, char *str, size_t maxsize, struct t_group *group, t_bool expand_all);
static int gnksa_check_domain(char *domain);
static int gnksa_check_domain_literal(const char *domain);
static int gnksa_check_localpart(const char *localpart);
static int gnksa_dequote_plainphrase(char *realname, char *decoded, int addrtype);
static int strfeditor(const char *editor, int linenum, const char *filename, char *s, size_t maxsize, char *format);
static void make_connection_page(FILE *fp);
static void write_input_history_file(void);
#ifdef CHARSET_CONVERSION
	static t_bool buffer_to_local(char **line, size_t *max_line_len, const char *from_charset, const char *to_charset);
#	ifdef CHARSET_CONVERSION_UCNV
	static t_bool tin_ucnv_buffer_to_local(char **line, size_t *max_line_len, const char *from_charset, const char *to_charset);
#	endif /* CHARSET_CONVERSION_UCNV */
#endif /* CHARSET_CONVERSION */
#if 0 /* currently unused */
	static t_bool stat_article(t_artnum art, const char *group_path);
#endif /* 0 */


/*
 * generate tmp-filename
 */
char *
get_tmpfilename(
	const char *filename)
{
	char *file_tmp;
	int n;
	size_t len;

#ifdef APPEND_PID
	if ((n = snprintf(NULL, 0, "%s.tmp.%ld", filename, (long) process_id)) < 0)
#else
	if ((n = snprintf(NULL, 0, "%s.tmp", filename)) < 0)
#endif /* APPEND_PID */
		return NULL;

	len = (size_t) n + 1;
	file_tmp = my_malloc(len);
#ifdef APPEND_PID
	if (snprintf(file_tmp, len, "%s.tmp.%ld", filename, (long) process_id) < 0)
#else
	if (snprintf(file_tmp, len, "%s.tmp", filename) < 0)
#endif /* APPEND_PID */
	{
		free(file_tmp);
		return NULL;
	}

	return file_tmp;
}


/*
 * append_file instead of rename_file
 * minimum error trapping
 * return 0 on success, errno otherwise.
 */
int
append_file(
	const char *old_filename,
	const char *new_filename)
{
	FILE *fp_old, *fp_new;
	int rval;

	if (!*old_filename || !*new_filename)
		return ENOENT;

	if ((fp_old = tin_fopen(old_filename, "r")) == NULL)
		return errno;

	if ((fp_new = fopen(new_filename, "a")) == NULL) {
		rval = errno;
		perror_message(_(txt_cannot_open), new_filename);
		fclose(fp_old);
		return rval;
	}
	rval = copy_fp(fp_old, fp_new);
	fclose(fp_old);
	fclose(fp_new);
	return rval;
}


#ifndef NDEBUG
_Noreturn void
asfail(
	const char *file,
	int line,
	const char *cond)
{
	my_fprintf(stderr, txt_error_asfail, tin_progname, file, line, cond);
	my_fflush(stderr);

/*
 * create a core dump
 */
#	ifdef HAVE_COREFILE
#		ifdef SIGABRT
		sigdisp(SIGABRT, SIG_DFL);
		kill(process_id, SIGABRT);
#		else
#			ifdef SIGILL
			sigdisp(SIGILL, SIG_DFL);
			kill(process_id, SIGILL);
#			else
#				ifdef SIGIOT
				sigdisp(SIGIOT, SIG_DFL);
				kill(process_id, SIGIOT);
#				endif /* SIGIOT */
#			endif /* SIGILL */
#		endif /* SIGABRT */
#	endif /* HAVE_COREFILE */

	giveup();
}
#endif /* !NDEBUG */


/*
 * Quick copying of files
 *
 * Returns 0 on success and errno otherwise (incl. EPIPE).
 */
int
copy_fp(
	FILE *fp_ip,
	FILE *fp_op)
{
	char buf[8192];
	size_t have, sent;

	errno = 0;	/* To check errno after write, clear it here */

	while ((have = fread(buf, 1, sizeof(buf), fp_ip)) != 0) {
		sent = fwrite(buf, 1, have, fp_op);
		if (sent != have) {
			TRACE(("copy_fp wrote %d of %d:{%.*s}", sent, have, (int) sent, buf));
			if (errno && errno != EPIPE) /* not a broken pipe => more serious error */
				perror_message(_(txt_error_copy_fp));

			return errno;
		}
		TRACE(("copy_fp wrote %d:{%.*s}", sent, (int) sent, buf));
		if (feof(fp_ip) || ferror(fp_ip))
			break;
	}
	return (ferror(fp_ip) ? -1 : 0);
}


/*
 * backup_file(filename, backupname)
 *
 * try to backup filename as backupname. on success backupname has the same
 * permissions as filename.
 *
 * return codes:
 * TRUE  = backup complete or source file was missing
 * FALSE = backup failed
 */
t_bool
backup_file(
	const char *filename,
	const char *backupname)
{
	FILE *fp_in, *fp_out;
	t_bool ret = FALSE;
#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	int fd;
	mode_t mode = (mode_t) (S_IRUSR|S_IWUSR);
	struct stat statbuf;
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	if ((fp_in = tin_fopen(filename, "r")) == NULL)	/* a missing sourcefile is not a real bug */
		return TRUE;

	/* don't follow links when writing backup files - do we really want this? */
	unlink(backupname);
	if ((fp_out = fopen(backupname, "w")) == NULL) {
		fclose(fp_in);
		return ret;
	}

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	if ((fd = fileno(fp_in)) != -1) {
		if (!fstat(fd, &statbuf))
			mode = statbuf.st_mode;
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	if (copy_fp(fp_in, fp_out) == 0)
		ret = TRUE;

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	if ((fd = fileno(fp_out)) != -1) {
#	ifdef HAVE_FCHMOD
		fchmod(fd, mode);
#	else
#		ifdef HAVE_CHMOD
		chmod(backupname, mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
	}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	fclose(fp_out);
	fclose(fp_in);
	return ret;
}


/*
 * copy the body of articles with given file pointers,
 * prefix (= quote_chars), initials of the articles author
 * with_sig is set if the signature should be quoted
 *
 * TODO: - rewrite from scratch, the code is awful!
 *       - handle format-flowed (rewrapping, space stuffing,
 *         force quote compression?, ...)
 */
void
copy_body(
	FILE *fp_ip,
	FILE *fp_op,
	char *prefix,
	const char *initl,
	t_bool raw_data)
{
	char *buf;
	char *buf2;
	char prefixbuf[256];
	char *p = prefixbuf;
	char *q = prefix;
	int i;
	int retcode;
	size_t maxlen = sizeof(prefixbuf) - 1;
	size_t ilen = strlen(initl);
	t_bool initials = FALSE;
	t_bool status_char;
	t_bool status_space;

	/* This is a shortcut for speed reasons: if no prefix (= quote_chars) is given just copy */
	if (!prefix || !*prefix) {
		copy_fp(fp_ip, fp_op);
		return;
	}

	while (maxlen > 0 && *q) {
		if (*q == '%' && *(q + 1) == 'I') {
			if (maxlen < ilen) /* not enough space left for %I expansion */
				break;

			strcpy(p, initl);
			maxlen -= ilen;
			p += ilen;
			q += 2; /* skip over "%I" */
			initials = TRUE;
		} else {
			*p++ = *q++;
			--maxlen;
		}
	}
	*p = '\0';

	/* no QUOTE_COMPRESS with initials */
	if ((tinrc.quote_style & QUOTE_COMPRESS) && !initials) {
		if (prefixbuf[strlen(prefixbuf) - 1] == ' ')
			prefixbuf[strlen(prefixbuf) - 1] = '\0';
	}

	/*
	 * if raw_data is true, the signature is exceptionally quoted, even if
	 * tinrc tells us not to do so. This extraordinary behavior occurs when
	 * replying or following up with the 'raw' message shown.
	 */
	buf2 = my_malloc(1024);
	while ((buf = tin_fgets(fp_ip, FALSE)) != NULL) {
		ilen = strlen(buf);
		if (!(tinrc.quote_style & QUOTE_SIGS) && !strncmp(buf, SIGDASHES, 3) && ilen == 3 && !raw_data)
			break;

		if (initials) { /* initials wanted */
			if (ilen) { /* line is not empty */
				if (strchr(buf, '>')) {
					if (ilen > sizeof(buf2))
						buf2 = my_realloc(buf2, ilen + 1);
					status_space = FALSE;
					status_char = TRUE;
					for (i = 0; buf[i] && (buf[i] != '>'); i++) {
						buf2[i] = buf[i];
						if (buf[i] != ' ')
							status_space = TRUE;
						if ((status_space) && !(isalpha((unsigned char) buf[i]) || buf[i] == '>'))
							status_char = FALSE;
					}
					buf2[i] = '\0';
					if (status_char)	/* already quoted */
						retcode = fprintf(fp_op, "%s>%s\n", buf2, BlankIfNull(strchr(buf, '>')));
					else	/* ... to be quoted ... */
						retcode = fprintf(fp_op, "%s%s\n", prefixbuf, buf);
				} else	/* line was not already quoted (no >) */
					retcode = fprintf(fp_op, "%s%s\n", prefixbuf, buf);
			} else	/* line is empty */
				retcode = fprintf(fp_op, "%s\n", ((tinrc.quote_style & QUOTE_EMPTY) ? prefixbuf : ""));
		} else {		/* no initials in quote_string, just copy */
			if (ilen || (tinrc.quote_style & QUOTE_EMPTY))
				retcode = fprintf(fp_op, "%s%s\n", (buf[0] == '>' ? prefixbuf : prefix), buf);	/* use blank-stripped quote string if line is already quoted */
			else
				retcode = fprintf(fp_op, "\n");
		}
		if (retcode == EOF) {
			perror_message("copy_body() failed"); /* TODO: -> lang.c */
			FreeIfNeeded(buf2);
			return;
		}
	}
	FreeIfNeeded(buf2);
}


/*
 * Lookup 'env' in the environment. If it exists, return its value if non-null,
 * else return 'def'
 */
const char *
get_val(
	const char *env,	/* Environment variable we're looking for */
	const char *def)	/* Default value if no environ value found or null */
{
	const char *ptr;

	return ((ptr = getenv(env)) != NULL ? (*ptr ? ptr : def) : def);
}


/*
 * IMHO it's not tins job to take care about dumb editor backupfiles
 * otherwise BACKUP_FILE_EXT should be configurable via 'M'enu
 */
#define BACKUP_FILE_EXT ".b"
t_bool
invoke_editor(
	const char *filename,
	int lineno,
	const struct t_group *group) /* return value is always ignored */
{
	char buf[PATH_LEN];
	char editor_format[PATH_LEN];
	static char editor[PATH_LEN];
	static t_bool first = TRUE;
	t_bool retcode;
#ifdef BACKUP_FILE_EXT
	char fnameb[PATH_LEN];
#endif /* BACKUP_FILE_EXT */

	if (first) {
		my_strncpy(editor, get_val("VISUAL", get_val("EDITOR", DEFAULT_EDITOR)), sizeof(editor) - 1);
		first = FALSE;
	}

	if (group != NULL)
		my_strncpy(editor_format, (group->attribute->editor_format ? *group->attribute->editor_format : TIN_EDITOR_FMT), sizeof(editor_format) - 1);
	else
		my_strncpy(editor_format, (tinrc.editor_format ? tinrc.editor_format : TIN_EDITOR_FMT), sizeof(editor_format) - 1);

	if (!strfeditor(editor, lineno, filename, buf, sizeof(buf), editor_format))
		sh_format(buf, sizeof(buf), "%s %s", editor, filename);

	cursoron();
	my_flush();
	retcode = invoke_cmd(buf);

#ifdef BACKUP_FILE_EXT
	if (strlen(filename) + strlen(BACKUP_FILE_EXT) < sizeof(fnameb)) {
		STRCPY(fnameb, filename);
		strcat(fnameb, BACKUP_FILE_EXT);
		unlink(fnameb);
	}
#endif /* BACKUP_FILE_EXT */
	return retcode;
}


#ifdef HAVE_ISPELL
t_bool
invoke_ispell(
	const char *nam,
	const struct t_group *group) /* return value is always ignored */
{
	FILE *fp_all, *fp_body, *fp_head;
	char buf[PATH_LEN], nam_body[PATH_LEN], nam_head[PATH_LEN];
	char ispell[PATH_LEN];
	t_bool retcode;

	if (group && group->attribute->ispell && *group->attribute->ispell)
		STRCPY(ispell, *group->attribute->ispell);
	else
		STRCPY(ispell, get_val("ISPELL", PATH_ISPELL));

	/*
	 * Now separating the header and body in two different files so that
	 * the header is not checked by ispell
	 */
#	ifdef HAVE_LONG_FILE_NAMES
	snprintf(nam_body, sizeof(nam_body), "%s%s", nam, ".body");
	snprintf(nam_head, sizeof(nam_head), "%s%s", nam, ".head");
#	else
	snprintf(nam_body, sizeof(nam_body), "%s%s", nam, ".b");
	snprintf(nam_head, sizeof(nam_head), "%s%s", nam, ".h");
#	endif /* HAVE_LONG_FILE_NAMES */

	if ((fp_all = tin_fopen(nam, "r")) == NULL)
		return FALSE;

	if ((fp_head = fopen(nam_head, "w")) == NULL) {
		perror_message(_(txt_cannot_open), nam_head);
		fclose(fp_all);
		return FALSE;
	}

	if ((fp_body = fopen(nam_body, "w")) == NULL) {
		perror_message(_(txt_cannot_open), nam_body);
		fclose(fp_head);
		fclose(fp_all);
		return FALSE;
	}

	while (fgets(buf, (int) sizeof(buf), fp_all) != NULL) {
		fputs(buf, fp_head);
		if (buf[0] == '\n' || buf[0] == '\r') {
			fclose(fp_head);
			fp_head = NULL;
			break;
		}
	}

	if (fp_head)
		fclose(fp_head);

	while (!feof(fp_all) && !ferror(fp_all)) {
		if (fgets(buf, (int) sizeof(buf), fp_all) != NULL)
			fputs(buf, fp_body);
		else
			break;
	}

	fclose(fp_body);
	fclose(fp_all);

	sh_format(buf, sizeof(buf), "%s %s", ispell, nam_body);
	retcode = invoke_cmd(buf);

	if ((errno = append_file(nam_body, nam_head)) != 0) {
		perror_message(_(txt_enter_append), nam_body, nam_head);
		retcode = FALSE;
	}
	unlink(nam_body);
	if ((errno = rename_file(nam_head, nam)) != 0) {
		perror_message(_(txt_rename_error), nam_head, nam);
		retcode = FALSE;
		unlink(nam_head);
	}
	return retcode;
}
#endif /* HAVE_ISPELL */


#ifndef NO_SHELL_ESCAPE
void
shell_escape(
	void)
{
	char *p, *tmp = fmt_string(_(txt_shell_escape), BlankIfNull(tinrc.default_shell_command));
	char shell[LEN];

	if (!prompt_string(tmp, shell, HIST_SHELL_COMMAND)) {
		free(tmp);
		return;
	}
	free(tmp);

	for (p = shell; *p && isspace((unsigned char) *p); p++)
		continue;

	if (*p) {
		FreeIfNeeded(tinrc.default_shell_command);
		tinrc.default_shell_command = my_strdup(p);
	} else {
		my_strncpy(shell, (tinrc.default_shell_command ? tinrc.default_shell_command : (get_val(ENV_VAR_SHELL, DEFAULT_SHELL))), sizeof(shell) - 1);
		p = shell;
	}

	ClearScreen();
	tmp = fmt_string(_(txt_shell_command), p);
	center_line(0, TRUE, tmp);
	free(tmp);
	MoveCursor(INDEX_TOP, 0);

	(void) invoke_cmd(p);

#	ifndef USE_CURSES
	EndWin();
	Raw(FALSE);
#	endif /* !USE_CURSES */
	prompt_continue();
#	ifndef USE_CURSES
	Raw(TRUE);
	InitWin();
#	endif /* !USE_CURSES */

	if (tinrc.draw_arrow)
		ClearScreen();
}


/*
 * shell out, if supported
 */
void
do_shell_escape(
	void)
{
	shell_escape();
	currmenu->redraw();
}
#endif /* !NO_SHELL_ESCAPE */


/*
 * Exits tin cleanly.
 * Has recursion protection - this may happen if the NNTP connection aborts
 * and is not re-established
 */
_Noreturn void
tin_done(
	int ret,
	const char *fmt,
	...)
{
	char *buf = NULL;
	int i;
	signed long int wrote_newsrc_lines;
	static int nested = 0;
	struct t_group *group;
	t_bool ask = TRUE;
	va_list ap;

	if (nested++)
		giveup();

	if (fmt && *fmt) {
		va_start(ap, fmt);
		buf = fmt_message(fmt, ap);
		va_end(ap);
	}

	signal_context = cMain;

#ifdef USE_CURSES
	scrollok(stdscr, TRUE);			/* Allows display of multi-line messages */
#endif /* USE_CURSES */

	/*
	 * check if any groups were read & ask if they should marked read
	 */
	if (tinrc.catchup_read_groups && !cmd_line && !no_write) {
		for (i = 0; i < selmenu.max; i++) {
			group = &active[my_group[i]];
			if (group->read_during_session) {
				if (ask) {
					if (prompt_yn(_(txt_catchup_all_read_groups), FALSE) == 1) {
						ask = FALSE;
						tinrc.thread_articles = THREAD_NONE;	/* speeds up index loading */
					} else
						break;
				}
				wait_message(0, _(txt_catchup_group), group->name);
				grp_mark_read(group, NULL);
			}
		}
	}

	/*
	 * Save the newsrc file. If it fails for some reason, give the user a
	 * chance to try again
	 */
	if (!no_write) {
		i = (read_newsrc_lines <= 0L ? 2 : 4); /* max retries */
		while (--i) {
			wrote_newsrc_lines = write_newsrc();
			if ((wrote_newsrc_lines >= 0L) && (wrote_newsrc_lines >= read_newsrc_lines)) {
				if (/* !batch_mode || */ verbose)
					wait_message(0, _(txt_newsrc_saved), newsrc);
				break;
			}

			if (wrote_newsrc_lines < read_newsrc_lines) {
				/* FIXME: prompt for retry? (i.e. remove break) */
				/* FIXME: translatable/plural-forms */
				wait_message(0, _(txt_warn_newsrc), newsrc,
					(read_newsrc_lines - wrote_newsrc_lines),
					P_(txt_group_sp[0], txt_group_sp[1], read_newsrc_lines - wrote_newsrc_lines),
					OLDNEWSRC_FILE);
				if (!batch_mode)
					prompt_continue();
				break;
			}

			if (i && !batch_mode) {
				if (prompt_yn(_(txt_newsrc_again), TRUE) <= 0)
					break;
			}
		}

		write_input_history_file();

#ifdef HAVE_MH_MAIL_HANDLING
		write_mail_active_file();
#endif /* HAVE_MH_MAIL_HANDLING */

		if (unlink(local_motd_file) == -1) { /* ensure it will be recreated (even if started with -Q) */
			switch (errno) {
				case ENOENT: /* missing file is ok */
					break;

				default:
					perror_message(_(txt_error_unlink), local_motd_file);
					break;
			}
		}
	}

#ifdef XFACE_ABLE
	slrnface_stop();
#endif /* XFACE_ABLE */

	/* Do this sometime after we save the newsrc in case this hangs up for any reason */
	nntp_close((ret == NNTP_ERROR_EXIT));			/* disconnect from NNTP server */

	if (use_nntps)
		tintls_exit();

	free_all_arrays();

	handle_cmdargs(FALSE);

#if defined(HAVE_LIBICUUC)
	u_cleanup();
#endif /* HAVE_LIBICUUC */

	/*
	 * TODO:
	 * why do we make this exception "Terminate gracefully but do
	 * not restore terminal" here?
	 */
#ifdef SIGUSR1
	if (ret != -SIGUSR1) {
#endif /* SIGUSR1 */
#ifdef HAVE_COLOR
#	ifndef USE_CURSES
		reset_screen_attr();
#	endif /* !USE_CURSES */
		use_color = FALSE;
		EndInverse();
#else
		if (!cmd_line)
#endif /* HAVE_COLOR */
		{
			cursoron();
			if (!ret)
				ClearScreen();
		}
		EndWin();
		Raw(FALSE);
#ifdef SIGUSR1
	} else
		ret = SIGUSR1;
#endif /* SIGUSR1 */
#ifdef HAVE_COLOR
#	ifdef USE_CURSES
	free_color_pair_arrays();
#	endif /* USE_CURSES */
#endif /* HAVE_COLOR */
	cleanup_tmp_files();

#ifdef DEBUG
	if (debug > DEBUG_REMOVE) /* special case to remove debug files on exit */
		debug_delete_files();
#endif /* DEBUG */

	if (buf && *buf) {
		my_fputs(buf, stderr);
		my_fputs(cCRLF, stderr);
		my_fflush(stderr);
		free(buf);
	}

	close_msglog();

#ifdef DOALLOC
	no_leaks();	/* free permanent stuff */
	show_alloc();	/* memory-leak testing */
#endif /* DOALLOC */

#ifdef USE_DBMALLOC
	/* force a dump, circumvents a bug in Linux libc */
	{
		extern int malloc_errfd;	/* FIXME */
		malloc_dump(malloc_errfd);
	}
#endif /* USE_DBMALLOC */

	exit(ret);
}


int
my_mkdir(
	const char *path,
	mode_t mode)
{
#ifndef HAVE_MKDIR
	char buf[LEN];
	struct stat sb;

	if (stat(path, &sb) == -1) {
		snprintf(buf, sizeof(buf), "mkdir %s", path); /* redirect stderr to /dev/null? use invoke_cmd()? */
		system(buf);
#	ifdef HAVE_CHMOD
		return chmod(path, mode);
#	else
		(void) mode;
		return 0; /* chmod via system() like for mkdir? */
#	endif /* HAVE_CHMOD */
	} else
		return -1;
#else
	return mkdir(path, mode);
#endif /* !HAVE_MKDIR */
}


/*
 * returns 0 on success and errno otherwise
 */
int
rename_file(
	const char *old_filename,
	const char *new_filename)
{
	FILE *fp_old, *fp_new;
	int rval = 0;
#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
	int fd;
	mode_t mode = (mode_t) (S_IRUSR|S_IWUSR);
	struct stat statbuf;
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

	if (unlink(new_filename) == -1) {
		if (errno == EPERM) { /* TODO: != ENOENT ? */
			rval = errno;
			perror_message(_(txt_error_unlink), new_filename);
			return rval;
		}
	}

#ifdef HAVE_LINK
	if (link(old_filename, new_filename) == -1)
#else
	if (rename(old_filename, new_filename) < 0)
#endif /* HAVE_LINK */
	{
		rval = errno;
		if (errno != EXDEV) { /* ENOENT and co. */
			perror_message(_(txt_rename_error), old_filename, new_filename);
			return rval;
		} else { /* create & copy file across filesystem */
			if ((fp_old = tin_fopen(old_filename, "r")) == NULL)
				return errno;

			if ((fp_new = fopen(new_filename, "w")) == NULL) {
				rval = errno;
				perror_message(_(txt_cannot_open), new_filename);
				fclose(fp_old);
				return rval;
			}

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
			if ((fd = fileno(fp_old)) != -1) {
				if (!fstat(fd, &statbuf))
					mode = statbuf.st_mode;
			}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

			copy_fp(fp_old, fp_new);

#if defined(HAVE_FCHMOD) || defined(HAVE_CHMOD)
			if ((fd = fileno(fp_new)) != -1) {
#	ifdef HAVE_FCHMOD
				fchmod(fd, mode);
#	else
#		ifdef HAVE_CHMOD
				chmod(new_filename, mode);
#		endif /* HAVE_CHMOD */
#	endif /* HAVE_FCHMOD */
			}
#endif /* HAVE_FCHMOD || HAVE_CHMOD */

			fclose(fp_new);
			fclose(fp_old);
			errno = 0;
		}
	}
#ifdef HAVE_LINK
	if (unlink(old_filename) == -1) {
		rval = errno;
		perror_message(_(txt_rename_error), old_filename, new_filename);
	}
#endif /* HAVE_LINK */
	return rval;
}


/*
 * Note that we exit screen/curses mode when invoking
 * external commands
 */
t_bool
invoke_cmd(
	const char *nam)
{
	int ret;
	t_bool save_cmd_line = cmd_line;
#ifndef IGNORE_SYSTEM_STATUS
	t_bool success;
#endif /* !IGNORE_SYSTEM_STATUS */

	if (!save_cmd_line) {
		EndWin();
		Raw(FALSE);
	}
	set_signal_catcher(FALSE);

	TRACE(("called system(%s)", _nc_visbuf(nam)));
	ret = system(nam);
#ifndef USE_SYSTEM_STATUS
	system_status = (ret >= 0 && WIFEXITED(ret)) ? WEXITSTATUS(ret) : 0;
#endif /* !USE_SYSTEM_STATUS */
	TRACE(("return %d (%d)", ret, system_status));

	set_signal_catcher(TRUE);
	if (!save_cmd_line) {
		Raw(TRUE);
		InitWin();
		need_resize = cYes;		/* Flag a redraw */
	}

#ifdef IGNORE_SYSTEM_STATUS
	return TRUE;
#else

	success = (ret == 0);

	if (!success || system_status != 0)
		error_message(2, _(txt_command_failed), nam);

	return success;
#endif /* IGNORE_SYSTEM_STATUS */
}


/*
 * grab file portion of fullpath
 */
void
base_name(
	const char *fullpath,		/* /foo/bar/baz */
	char *file)				/* baz */
{
	char *p = strrchr(fullpath, '/');

	if (p && *(p + 1))
		strcpy(file, p + 1);
	else
		strcpy(file, fullpath);
}


/*
 * grab dir portion of fullpath
 */
void
dir_name(
	const char *fullpath,	/* /foo/bar/baz */
	char *dir)		/* /foo/bar/ */
{
	char *d, *f, *p;

	d = my_strdup(fullpath);
	f = my_strdup(fullpath);
	base_name(d, f);
	if ((p = strrstr(d, f)) != NULL)
		*p = '\0';
	strcpy(dir, d);
	free(f);
	free(d);
}


/*
 * Return TRUE if new mail has arrived
 */
#define MAILDIR_NEW	"new"
t_bool
mail_check(
	const char *mailbox_name)
{
	char *from, *to, p, *cmb, *mb, ecmb[PATH_LEN];
	t_bool in_message = FALSE;
	struct stat buf;

	if (!mailbox_name || !*mailbox_name)
		return FALSE;
	mb = my_strdup(mailbox_name); /* take a copy due to strtok() */

	/*
	 * remove custom messages for now; roughly based on
	 * <https://pubs.opengroup.org/onlinepubs/9699919799/utilities/sh.html>
	 *
	 * TODO: if '\'-quoting is limited to '%' outside the message, there
	 *       is no way to get a ':' into the message (except it would come
	 *       from a variable expansion).
	 */
	from = to = mb;
	while ((p = *from) != '\0') {
		++from;
		if (!in_message) {
			if (p == '\\' && *from == '%') /* really just %? */
				continue;

			if (p == '%' || p == '?') {
				in_message = TRUE;
				continue;
			}

			*to++ = p;
		}
		if (p == ':')
			in_message = FALSE;
	}
	*to = p;

	/* split into elements */
	cmb = strtok(mb, ":");
	while (cmb != NULL) {
		/* expand relative path */
		if (!strfpath(cmb, ecmb, sizeof(ecmb), NULL, FALSE)) {
			free(mb);
			return FALSE;
		} else
			cmb = ecmb;

		if (stat(cmb, &buf) == 0) {
			if (S_ISDIR(buf.st_mode)) { /* maildir setup */
				char *maildir_box;
				size_t maildir_box_len = strlen(cmb) + strlen(MAILDIR_NEW) + 2;
				DIR *dirp;
				DIR_BUF *dp;

				maildir_box = my_malloc(maildir_box_len);
				joinpath(maildir_box, maildir_box_len, cmb, MAILDIR_NEW);

				if (!(dirp = opendir(maildir_box))) {
					free(maildir_box);
					free(mb);
					return FALSE;
				}
				free(maildir_box);
				while ((dp = readdir(dirp)) != NULL) {
					if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, ".."))) {
						CLOSEDIR(dirp);
						free(mb);
						return TRUE;
					}
				}
				CLOSEDIR(dirp);
			} else {
				if (buf.st_atime < buf.st_mtime && buf.st_size > 0) {
					free(mb);
					return TRUE;
				}
			}
		}
		cmb = strtok(NULL, ":");
	}
	free(mb);
	return FALSE;
}


/*
 * Return a pointer into s eliminating any leading Re:'s. Example:
 *
 * 	Re: Reorganization of misc.jobs
 *		^   ^
 * 	Re^2: Reorganization of misc.jobs
 *
 * now also strips trailing (was: ...) (obw)
 */
const char *
eat_re(
	char *s,
	t_bool eat_was)
{
	if (!s || !*s)
		return "";
	else {
		int match;
		REGEX_SIZE *offsets;

		do {
			match = match_regex_ex(s, (REGEX_SIZE) strlen(s), 0, 0, &strip_re_regex);
			offsets = regex_get_ovector_pointer(&strip_re_regex);
			if (match >= 0 && offsets[0] == 0)
				s += offsets[1];
		} while (match >= 0);

		if (eat_was) do {
			match = match_regex_ex(s, (REGEX_SIZE) strlen(s), 0, 0, &strip_was_regex);
			offsets = regex_get_ovector_pointer(&strip_was_regex);
			if (match >= 0 && offsets[0] > 0)
				s[offsets[0]] = '\0';
		} while (match >= 0);
	}
	return s;
}


#if defined(NO_LOCALE) || !defined(MULTIBYTE_ABLE)
int
my_isprint(
	int c)
{
#	ifndef NO_LOCALE
	/* use locale */
	return isprint(c);
#	else
	if (IS_LOCAL_CHARSET("ISO-8859") || IS_LOCAL_CHARSET("ISO8859"))
		return (isprint(c) || (c >= 0xa0 && c <= 0xff));
	else if (IS_LOCAL_CHARSET("ISO-2022"))
		return (isprint(c) || (c == 0x1b));
	else if (IS_LOCAL_CHARSET("Big5"))
		return (isprint(c) || (c >= 0x40 && c <= 0xfe && c != 0x7f));
	else if (IS_LOCAL_CHARSET("EUC-"))
		return 1;
	else /* KOI8-* and UTF-8 */
		return (isprint(c) || (c >= 0x80 && c <= 0xff));
#	endif /* !NO_LOCALE */
}
#endif /* NO_LOCALE || !MULTIBYTE_ABLE */


/*
 * Returns author information
 * thread   if true, assume we're on thread menu and show all author info if
 *          subject not shown
 * art      ptr to article
 * str      ptr in which to return the author. Must be a valid data area
 * len      max length of data to return
 *
 * The data will be null terminated
 */
void
get_author(
	t_bool thread,
	struct t_article *art,
	char *str,
	size_t len)
{
	char *p, *ptr = str;
	int author;
	size_t curr_len, rem_len = len - 1;
	struct t_mailbox *mb = &art->mailbox;

	*ptr = '\0';

	do {
		p = idna_decode(mb->from);

		author = ((thread && !show_subject && curr_group->attribute->show_author == SHOW_FROM_NONE) ? SHOW_FROM_BOTH : curr_group->attribute->show_author);

		switch (author) {
			case SHOW_FROM_ADDR:
				strncpy(ptr, p, rem_len);
				break;

			case SHOW_FROM_NAME:
				strncpy(ptr, (mb->name ? mb->name : p), rem_len);
				break;

			case SHOW_FROM_BOTH:
				if (mb->name) {
					if (CHECK_RFC5322_SPECIALS(mb->name))
						snprintf(ptr, rem_len, "\"%s\" <%s>", mb->name, p);
					else
						snprintf(ptr, rem_len, "%s <%s>", mb->name, p);
				}
				else
					strncpy(ptr, p, rem_len);
				break;

			case SHOW_FROM_NONE:
			default:
				rem_len = 0;
				break;
		}

		free(p);

		/* we stop if there is no room for at least one character after ", " */
		if (rem_len < 5)
			mb = NULL;
		else if ((mb = mb->next)) {
			strcat(ptr, ", ");
			curr_len = strlen(ptr);
			rem_len -= curr_len;
			ptr += curr_len;
		}
	} while (mb);
}


void
toggle_inverse_video(
	void)
{
	if (!(tinrc.inverse_okay = bool_not(tinrc.inverse_okay)))
		tinrc.draw_arrow = TRUE;
#ifndef USE_INVERSE_HACK
#	if 0
	else
		tinrc.draw_arrow = FALSE;
#	endif /* 0 */
#endif /* !USE_INVERSE_HACK */
}


void
show_inverse_video_status(
	void)
{
	info_message((tinrc.inverse_okay ? _(txt_inverse_on) : _(txt_inverse_off)));
}


#ifdef HAVE_COLOR
t_bool
toggle_color(
	void)
{
#	ifdef USE_CURSES
	if (!has_colors()) {
		use_color = FALSE;
		info_message(_(txt_no_colorterm));
		return FALSE;
	}
	if (use_color)
		reset_color();
#	endif /* USE_CURSES */

	if ((use_color = bool_not(use_color))) {
#	ifdef USE_CURSES
		fcol(tinrc.col_normal);
#	endif /* USE_CURSES */
		bcol(tinrc.col_back);
	}
#	ifndef USE_CURSES
	else
		reset_screen_attr();
#	endif /* !USE_CURSES */

	return TRUE;
}


void
show_color_status(
	void)
{
	info_message((use_color ? _(txt_color_on) : _(txt_color_off)));
}
#endif /* HAVE_COLOR */


/*
 * Check for lock file to stop multiple copies of tin -u running and if it
 * does not exist create it so this is the only copy running
 */
void
create_index_lock_file(
	char *the_lock_file)
{
	FILE *fp;
	char buf[64];
	char *cp;
	int err;
	time_t epoch;

	if ((fp = tin_fopen(the_lock_file, "r")) != NULL) {
		err = (fgets(buf, (int) sizeof(buf), fp) == NULL);
		fclose(fp);
		if (!err) {
			for (cp = buf; *cp; cp++) {
				if (!isdigit((unsigned char) *cp) && !isspace((unsigned char) *cp))
					break;
			}
			/* TODO:check errno after s2i()? (EINVAL == malformed lock) */
			error_message(2, "%s: Already started pid=[%d] on %s", tin_progname, s2i(buf, 0, INT_MAX), BlankIfNull(cp));
		} else
			error_message(2, "%s: Already started", tin_progname);
#ifdef DEBUG
		if (debug & DEBUG_MISC) {
			if (!err)
				error_message(0, "Lockfile: %s", the_lock_file);
		}
#endif /* DEBUG */

		free(tin_progname);
		giveup();
	}
	if ((fp = fopen(the_lock_file, "w")) != NULL) {
#ifdef HAVE_FCHMOD
		fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));
#else
#	ifdef HAVE_CHMOD
		chmod(the_lock_file, (mode_t) (S_IRUSR|S_IWUSR));
#	endif /* HAVE_CHMOD */
#endif /* HAVE_FCHMOD */
		(void) time(&epoch);
		fprintf(fp, "%ld  %s\n", (long) process_id, BlankIfNull(str_trim(ctime(&epoch))));
		if ((err = ferror(fp)) || fclose(fp)) {
			error_message(2, _(txt_filesystem_full), the_lock_file);
			if (err) {
				clearerr(fp);
				fclose(fp);
			}
		}
	}
}


/*
 * strfquote() - produce formatted quote string
 *   %A  Articles Email address
 *   %D  Articles Date (uses tinrc.date_format)
 *   %F  Articles Address+Name
 *   %G  Groupname of Article
 *   %M  Articles MessageId
 *   %N  Articles Name of author
 *   %C  First Name of author
 *   %I  Initials of author
 * Return number of characters written (???) or 0 on failure
 */
int
strfquote(
	const char *group,
	int respnum,
	char *s,
	size_t maxsize,
	char *format)
{
	const char *endp;
	char *start = s;
	char tbuf[LEN];
	int i, j;
	t_bool iflag;

	if (s == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strchr(format, '%') == NULL || strlen(format) + 1 >= maxsize)
		return 0;

	endp = s + maxsize;
	for (; *format && s < endp - 1; format++) {
		if (*format != '\\' && *format != '%') {
			*s++ = *format;
			continue;
		}

		tbuf[0] = '\0';

		if (*format == '\\') {
			switch (*++format) {
				case '\0':
					*s++ = '\\';
					goto out;
					/* NOTREACHED */
					break;

				case 'n':	/* linefeed */
					strcpy(tbuf, "\n");
					break;

				case 't':	/* tab */
					strcpy(tbuf, "\t");
					break;

				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}

			if ((i = (int) strlen(tbuf))) {
				if (s + i < endp - 1) {
					strcpy(s, tbuf);
					s += i;
				} else
					return 0;
			}
		}
		if (*format == '%') {
			switch (*++format) {

				case '\0':
					*s++ = '%';
					goto out;
					/* NOTREACHED */
					break;

				case '%':
					*s++ = '%';
					continue;

				case 'A':	/* Articles Email address */
					STRCPY(tbuf, arts[respnum].mailbox.from);
					break;

				case 'C':	/* First Name of author */
					if (arts[respnum].mailbox.name != NULL) {
						STRCPY(tbuf, arts[respnum].mailbox.name);
						if (strchr(tbuf, ' '))
							*(strchr(tbuf, ' ')) = '\0';
					} else {
						STRCPY(tbuf, arts[respnum].mailbox.from);
					}
					break;

				case 'D':	/* Articles Date (reformatted as specified in attributes->date_format) */
					if (!my_strftime(tbuf, LEN - 1, curr_group->attribute->date_format ? BlankIfNull(*curr_group->attribute->date_format) : "", localtime(&arts[respnum].date))) {
						STRCPY(tbuf, BlankIfNull(pgart.hdr.date));
					}
					break;

				case 'F':	/* Articles Address+Name */
					if (arts[respnum].mailbox.name)
						snprintf(tbuf, sizeof(tbuf), "%s <%s>", arts[respnum].mailbox.name, arts[respnum].mailbox.from);
					else {
						STRCPY(tbuf, arts[respnum].mailbox.from);
					}
					break;

				case 'G':	/* Groupname of Article */
					STRCPY(tbuf, group);
					break;

				case 'I':	/* Initials of author */
					STRCPY(tbuf, ((arts[respnum].mailbox.name != NULL) ? arts[respnum].mailbox.name : arts[respnum].mailbox.from));
					j = 0;
					iflag = TRUE;
					for (i = 0; tbuf[i]; i++) {
						if (iflag && tbuf[i] != ' ') {
							tbuf[j++] = tbuf[i];
							iflag = FALSE;
						}
						if (strchr(" ._@", tbuf[i]))
							iflag = TRUE;
					}
					tbuf[j] = '\0';
					break;

				case 'M':	/* Articles Message-ID */
					STRCPY(tbuf, BlankIfNull(pgart.hdr.messageid));
					break;

				case 'N':	/* Articles Name of author */
					STRCPY(tbuf, ((arts[respnum].mailbox.name != NULL) ? arts[respnum].mailbox.name : arts[respnum].mailbox.from));
					break;

				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}

			if ((i = (int) strlen(tbuf))) {
				if (s + i < endp - 1) {
					strcpy(s, tbuf);
					s += i;
				} else
					return 0;
			}
		}
	}
out:
	if (s < endp && *format == '\0') {
		*s = '\0';
		return (int) (s - start);
	} else
		return 0;
}


/*
 * strfeditor() - produce formatted editor string
 *   %E  Editor
 *   %F  Filename
 *   %N  Linenumber
 */
static int
strfeditor(
	const char *editor,
	int linenum,
	const char *filename,
	char *s,
	size_t maxsize,
	char *format)
{
	const char *endp;
	const char *start = s;
	char tbuf[PATH_LEN];
	int i;

	if (s == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strchr(format, '%') == NULL || strlen(format) + 1 >= maxsize)
		return 0;

	endp = s + maxsize;
	for (; *format && s < endp - 1; format++) {
		if (*format != '%') {
			*s++ = *format;
			continue;
		} else {
			tbuf[0] = '\0';

			switch (*++format) {
				case '\0':
					*s++ = '%';
					goto out;
					/* NOTREACHED */
					break;

				case '%':
					*s++ = '%';
					continue;

				case 'E':	/* Editor */
					STRCPY(tbuf, editor);
					break;

				case 'F':	/* Filename */
					STRCPY(tbuf, filename);
					break;

				case 'N':	/* Line number */
					sprintf(tbuf, "%d", linenum);
					break;

				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}

			if ((i = (int) strlen(tbuf))) {
				if (s + i < endp - 1) {
					strcpy(s, tbuf);
					s += i;
				} else
					return 0;
			}
		}
	}
out:
	if (s < endp && *format == '\0') {
		*s = '\0';
		return (int) (s - start);
	} else
		return 0;
}


/*
 * Helper function for strfpath() to copy expanded strings
 * into the output buffer. Return new output buffer or NULL
 * if we overflowed it.
 */
static char *
strfpath_cp(
	char *str,
	const char *tbuf,
	const char *endp)
{
	size_t i;

	if ((i = strlen(tbuf))) {
		if (str + i < endp - 1) {
			strcpy(str, tbuf);
			str += i;
		} else {
			str[0] = '\0';
			return NULL;
		}
	}
	return str;
}


/*
 * strfpath - produce formatted pathname expansion. Handles following forms:
 *   ~/News    -> $HOME/News
 *   ~abc/News -> /home/abc/News
 *   $var/News -> /env/var/News
 *   =file     -> $HOME/Mail/file
 *   =         -> $HOME/Mail/group.name   (shorthand for =%G)
 *   +file     -> savedir/group.name/file
 *   %G        -> group.name              (group.name is a file )
 *   %G/file   -> group.name/file         (group.name is a dir)
 *   %P        -> group/name              (name is a file)
 *   %P/file   -> group/name/file         (name is a dir)
 *
 * Inputs:
 *   format		The string to be converted
 *   str		Return buffer
 *   maxsize	Size of str
 *   group		ptr to current group
 *   expand_all	true if '+' and '=' should be expanded
 * Returns:
 *   0			on error
 *   1			if generated pathname is a mailbox
 *   2			success
 */
static int
_strfpath(
	const char *format,
	char *str,
	size_t maxsize,
	struct t_group *group,
	t_bool expand_all)
{
	const char *endp;
	char *envptr;
	char defbuf[PATH_LEN];
	char tbuf[PATH_LEN];
	const char *startp = format;
	int i;
	const struct passwd *pwd;
	t_bool is_mailbox = FALSE;

	if (str == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strlen(format) + 1 >= maxsize)
		return 0;

	endp = str + maxsize;
	for (; *format && str < endp - 1; format++) {
		/*
		 * If just a normal part of the pathname copy it
		 */
		if (!strchr("~$=+%", *format)) {
			*str++ = *format;
			continue;
		}

		tbuf[0] = '\0';

		switch (*format) {
			case '~':			/* Users or another users homedir */
				switch (*++format) {
					case '/':	/* users homedir */
						joinpath(tbuf, sizeof(tbuf), homedir, "");
						break;

					default:	/* some other users homedir */
						i = 0;
						while (*format && *format != '/')
							tbuf[i++] = *format++;
						tbuf[i] = '\0';
						/*
						 * OK lookup the username in /etc/passwd
						 */
						if ((pwd = getpwnam(tbuf)) == NULL) {
							str[0] = '\0';
							return 0;
						} else
							sprintf(tbuf, "%s/", pwd->pw_dir);
						break;
				}
				if ((str = strfpath_cp(str, tbuf, endp)) == NULL)
					return 0;
				break;

			case '$':	/* Read the envvar and use its value */
				i = 0;
				++format;
				if (*format == '{') {
					++format;
					while (*format && !(strchr("}-", *format)))
						tbuf[i++] = *format++;

					tbuf[i] = '\0';
					i = 0;
					if (*format == '-') {
						++format;
						while (*format && *format != '}')
							defbuf[i++] = *format++;
					}
					defbuf[i] = '\0';
				} else {
					while (*format && *format != '/')
						tbuf[i++] = *format++;

					tbuf[i] = '\0';
					--format;
					defbuf[0] = '\0';
				}
				/*
				 * OK lookup the variable in the shells environment
				 */
				envptr = getenv(tbuf);
				if (envptr == NULL || (*envptr == '\0'))
					STRCPY(tbuf, defbuf);
				else
					STRCPY(tbuf, envptr);
				if ((str = strfpath_cp(str, tbuf, endp)) == NULL)
					return 0;
				else if (*tbuf == '\0') {
					str[0] = '\0';
					return 0;
				}
				break;

			case '=':
				/*
				 * Mailbox name expansion
				 * Only expand if 1st char in format
				 * =dir expands to maildir/dir
				 * =    expands to maildir/groupname
				 */
				if (startp == format && group != NULL && expand_all) {
					char buf[PATH_LEN];

					is_mailbox = TRUE;
					if (strfpath(cmdline.maildir ? cmdline.maildir : (group->attribute->maildir && *group->attribute->maildir) ? *group->attribute->maildir : NULL, buf, sizeof(buf), group, FALSE)) {
						if (*(format + 1) == '\0')				/* Just an = */
							joinpath(tbuf, sizeof(tbuf), buf, group->name);
						else
							joinpath(tbuf, sizeof(tbuf), buf, "");
						if ((str = strfpath_cp(str, tbuf, endp)) == NULL)
							return 0;
					} else {
						str[0] = '\0';
						return 0;
					}
				} else					/* Wasn't the first char in format */
					*str++ = *format;
				break;

			case '+':
				/*
				 * Group name expansion
				 * Only convert if 1st char in format
				 * +file expands to savedir/group.name/file
				 */

				if (startp == format && group != NULL && expand_all) {
					char buf[PATH_LEN];

					/*
					 * Start with the savedir name
					 */
					if (strfpath(cmdline.savedir ? cmdline.savedir : (group->attribute->savedir && *group->attribute->savedir) ? *group->attribute->savedir : NULL, buf, sizeof(buf), group, FALSE)) {
						char tmp[PATH_LEN];
#ifdef HAVE_LONG_FILE_NAMES
						my_strncpy(tmp, group->name, sizeof(tmp) - 1);
#else
						my_strncpy(tmp, group->name, 14);
#endif /* HAVE_LONG_FILE_NAMES */
						joinpath(tbuf, sizeof(tbuf), buf, tmp);	/* Add the group name */
						joinpath(tmp, sizeof(tmp), tbuf, "");
						if ((str = strfpath_cp(str, tmp, endp)) == NULL)
							return 0;
					} else {
						str[0] = '\0';
						return 0;
					}
				} else					/* Wasn't the first char in format */
					*str++ = *format;
				break;

			case '%':	/* Different forms of parsing cmds */
				++format;
				if (group != NULL && *format == 'G') {
					memset(tbuf, 0, sizeof(tbuf));
					STRCPY(tbuf, group->name);
					i = (int) strlen(tbuf);
					if (((str + i) < (endp - 1)) && (i > 0)) {
						strcpy(str, tbuf);
						str += i;
					} else {
						str[0] = '\0';
						return 0;
					}
					break;
				}
				if (group != NULL && *format == 'P') {
					char *pbuf = my_malloc(strlen(group->name) + 2); /* trailing "/\0" */

					make_group_path(group->name, pbuf);
					if ((i = (int) strlen(pbuf)))
						pbuf[i--] = '\0'; /* remove trailing '/' */
					else {
						str[0] = '\0';
						free(pbuf);
						return 0;
					}
					if (((str + i) < (endp - 1)) && (i > 0)) {
						strcpy(str, pbuf);
						free(pbuf);
						str += i;
					} else {
						str[0] = '\0';
						free(pbuf);
						return 0;
					}
					break;
				}
				*str++ = *format;
				break;

			default:
				break;
		}
	}

	if (str < endp && *format == '\0') {
		*str = '\0';
		if (is_mailbox)
			return 1;
		else
			return 2;
	} else {
		str[0] = '\0';
		return 0;
	}
}


/*
 * The real entry point, exists only to expand leading '$'
 */
int
strfpath(
	const char *format,
	char *str,
	size_t maxsize,
	struct t_group *group,
	t_bool expand_all)
{
	/*
	 * Expand any leading env vars first in case they themselves contain
	 * formatting chars
	 */
	if (format && format[0] == '$') {
		char buf[PATH_LEN];

		if (_strfpath(format, buf, sizeof(buf), group, expand_all))
			return (_strfpath(buf, str, maxsize, group, expand_all));
	}

	return (_strfpath(format, str, maxsize, group, expand_all));
}


/*
 * TODO: Properly explain this (quote_areas)
 */
char *
escape_shell_meta(
	const char *source,
	enum quote_enum quote_area)
{
	static char buf[PATH_LEN];
	char *dest = buf;
	int space = sizeof(buf) - 2;

	switch (quote_area) {
		case no_quote:
			while (*source && (space > 0)) {
				if (*source == '\'' || *source == '\\' || *source == '"' ||
					*source == '$' || *source == '`' || *source == '*' ||
					*source == '&' || *source == '|' || *source == '<' ||
					*source == '>' || *source == ';' || *source == '(' ||
					*source == ')') {
					*dest++ = '\\';
					--space;
				}
				*dest++ = *source++;
				--space;
			}
			break;

		case dbl_quote:
			while (*source && (space > 0)) {
				if (*source == '\\' || *source == '"' || *source == '$' ||
					*source == '`') {
					*dest++ = '\\';
					--space;
				}
				*dest++ = *source++;
				--space;
			}
			break;

		case sgl_quote:
			while (*source && (space > 4)) {
				if (*source == '\'') {
					*dest++ = '\'';
					*dest++ = '\\';
					*dest++ = '\'';
					space -= 3;
				}
				*dest++ = *source++;
				--space;
			}
			break;
	}

	*dest = '\0';
	return buf;
}


/*
 * strfmailer() - produce formatted mailer string
 *   %M  Mailer
 *   %F  Filename
 *   %T  To
 *   %S  Subject
 *   %U  User
 * Returns length of produced string (is always ignored currently).
 */
int
strfmailer(
	const char *mail_prog,
	char *subject,	/* FIXME: should be const char */
	char *to, /* FIXME: should be const char */
	const char *filename,
	char *dest,
	size_t maxsize,
	const char *format)
{
	const char *endp;
	const char *start = dest;
	char tbuf[PATH_LEN];
	enum quote_enum quote_area = no_quote;

	/*
	 * safe guards: no destination to write to, no format, no space to
	 * write, or nothing to replace and format string longer than available
	 * space => return without any action
	 */
	if (dest == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strchr(format, '%') == NULL || strlen(format) + 1 >= maxsize)
		return 0;

	/*
	 * walk through format until end of format or end of available space
	 * and replace place holders
	 */
	endp = dest + maxsize;
	for (; *format && dest < endp - 1; format++) {
		/*
		 * take over any character other than '\' and '%' and continue with
		 * next character in format; remember quote area
		 */
		if (*format != '\\' && *format != '%') {
			if (*format == '"' && quote_area != sgl_quote)
				quote_area = (quote_area == dbl_quote ? no_quote : dbl_quote);
			if (*format == '\'' && quote_area != dbl_quote)
				quote_area = (quote_area == sgl_quote ? no_quote : sgl_quote);
			*dest++ = *format;
			continue;
		}

		tbuf[0] = '\0';

		/*
		 * handle sequences introduced by '\':
		 * - "\n" gets line feed (why?)
		 * - '\' followed by NULL gets '\' and leaves loop
		 * - '\' followed by any other character is copied literally and
		 *   shell escaped; if that exceeds the available space, return 0
		 */
		if (*format == '\\') {
			switch (*++format) {
				case '\0':
					*dest++ = '\\';
					goto out;
					/* NOTREACHED */
					break;

				case 'n':	/* linefeed */
					strcpy(tbuf, "\n");
					break;

				default:
					tbuf[0] = '\\';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}
			if (*tbuf) {
				if (sh_format(dest, (size_t) (endp - dest), "%s", tbuf) >= 0)
					dest += strlen(dest);
				else
					return 0;
			}
		}

		/*
		 * handle sequences introduced by '%'
		 * - '%' followed by NULL gets '%' and leaves loop
		 * - '%%' gets '%'
		 * - '%F' expands to filename
		 * - '%M' expands to mailer program
		 * - '%S' expands to subject of message
		 * - '%T' expands to recipient(s) of message
		 * - '%U' expands to userid
		 * - '%' followed by any other character is copied literally
		 */
		if (*format == '%') {
			char *p;
			t_bool ismail = TRUE;
			t_bool escaped = FALSE;

			switch (*++format) {
				case '\0':
					*dest++ = '%';
					goto out;

				case '%':
					*dest++ = '%';
					continue;

				case 'F':	/* Filename */
					STRCPY(tbuf, filename);
					break;

				case 'M':	/* Mailer */
					STRCPY(tbuf, mail_prog);
					break;

				case 'S':	/* Subject */
					/* don't MIME encode Subject if using external mail client */
					if (tinrc.interactive_mailer != INTERACTIVE_NONE)
						strncpy(tbuf, escape_shell_meta(subject, quote_area), sizeof(tbuf) - 1);
					else {
#ifdef CHARSET_CONVERSION
						p = rfc1522_encode(subject, txt_mime_charsets[tinrc.mm_network_charset], ismail);
#else
						p = rfc1522_encode(subject, tinrc.mm_charset, ismail);
#endif /* CHARSET_CONVERSION */
						strncpy(tbuf, escape_shell_meta(p, quote_area), sizeof(tbuf) - 1);
						free(p);
					}
					tbuf[sizeof(tbuf) - 1] = '\0';	/* just in case */
					escaped = TRUE;
					break;

				case 'T':	/* To */
					/* don't MIME encode To if using external mail client */
					if (tinrc.interactive_mailer != INTERACTIVE_NONE)
						strncpy(tbuf, escape_shell_meta(to, quote_area), sizeof(tbuf) - 1);
					else {
#ifdef CHARSET_CONVERSION
						p = rfc1522_encode(to, txt_mime_charsets[tinrc.mm_network_charset], ismail);
#else
						p = rfc1522_encode(to, tinrc.mm_charset, ismail);
#endif /* CHARSET_CONVERSION */
						strncpy(tbuf, escape_shell_meta(p, quote_area), sizeof(tbuf) - 1);
						free(p);
					}
					tbuf[sizeof(tbuf) - 1] = '\0';	/* just in case */
					escaped = TRUE;
					break;

				case 'U':	/* User */
					/* don't MIME encode User if using external mail client */
					if (tinrc.interactive_mailer != INTERACTIVE_NONE)
						STRCPY(tbuf, userid);
					else {
#ifdef CHARSET_CONVERSION
						p = rfc1522_encode(userid, txt_mime_charsets[tinrc.mm_network_charset], ismail);
#else
						p = rfc1522_encode(userid, tinrc.mm_charset, ismail);
#endif /* CHARSET_CONVERSION */
						STRCPY(tbuf, p);
						free(p);
					}
					break;

				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}
			if (*tbuf) {
				if (escaped) {
					if (endp - dest > 0) {
						strncpy(dest, tbuf, (size_t) (endp - dest));
						dest += strlen(dest);
					}
				} else if (sh_format(dest, (size_t) (endp - dest), "%s", tbuf) >= 0) {
					dest += strlen(dest);
				} else
					return 0;
			}
		}
	}
out:
	if (dest < endp && *format == '\0') {
		*dest = '\0';
		return (int) (dest - start);
	} else
		return 0;
}


/*
 * get_initials() - get initial letters of a posters name
 */
int
get_initials(
	struct t_article *art,
	char *s,
	int maxsize) /* return value is always 0 and ignored */
{
	char *ptr = s;
	char tbuf[PATH_LEN];
	int i, j, curr_len, rem_len = maxsize;
	struct t_mailbox *mb = &art->mailbox;
	t_bool iflag;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp, *wbuf;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (s == NULL || maxsize <= 0)
		return 0;

	*ptr = '\0';
	do {
		STRCPY(tbuf, ((mb->name != NULL) ? mb->name : mb->from));
		j = 0;
		iflag = FALSE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wtmp = char2wchar_t(tbuf)) != NULL) {
			wbuf = my_malloc(sizeof(wchar_t) * (size_t) (rem_len + 1));
			for (i = 0; wtmp[i] && j < rem_len; i++) {
				if (iswalpha((wint_t) wtmp[i])) {
					if (!iflag) {
						wbuf[j++] = wtmp[i];
						iflag = TRUE;
					}
				} else
					iflag = FALSE;
			}
			wbuf[j] = (wchar_t) '\0';
			if (wcstombs(tbuf, wbuf, sizeof(tbuf) - 1) != (size_t) -1)
				strcat(ptr, tbuf);
			free(wtmp);
			free(wbuf);
		}
#else
		for (i = 0; tbuf[i] && j < rem_len; i++) {
			if (isalpha((unsigned char) tbuf[i])) {
				if (!iflag) {
					ptr[j++] = tbuf[i];
					iflag = TRUE;
				}
			} else
				iflag = FALSE;
		}
		ptr[j] = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		/* we stop if there is no room for at least one character after ", " */
		if (rem_len < 5)
			mb = NULL;
		else if ((mb = mb->next)) {
			strcat(ptr, ", ");
			curr_len = strwidth(ptr);
			rem_len -= curr_len;
			ptr += curr_len;
		}
	} while (mb);
	return 0;
}


void
get_cwd(
	char *buf)
{
#ifdef HAVE_GETCWD
	if (getcwd(buf, PATH_LEN) == NULL) {
#	ifdef DEBUG
		if (debug & DEBUG_MISC)
			perror_message("getcwd(%s)", buf);
#	endif /* DEBUG */
	}
#else
#	ifdef HAVE_GETWD
	if (getwd(buf) == NULL) {
#		ifdef DEBUG
		if (debug & DEBUG_MISC)
			perror_message("getwd(%s)", buf);
#		endif /* DEBUG */
	}
#	else
	*buf = '\0';
#	endif /* HAVE_GETWD */
#endif /* HAVE_GETCWD */
}


/*
 * Convert a newsgroup name to a newsspool path
 * No effect when reading via NNTP
 */
void
make_group_path(
	const char *name,
	char *path)
{
	while (*name) {
		*path++ = ((*name == '.') ? '/' : *name);
		++name;
	}
	*path++ = '/';
	*path = '\0';
}


/*
 * Given a base pathname & a newsgroup name build an absolute pathname.
 * base_dir = /usr/spool/news
 * group_name = alt.sources
 * group_path = /usr/spool/news/alt/sources
 */
void
make_base_group_path(
	const char *base_dir,
	const char *group_name,
	char *group_path,
	size_t group_path_len)
{
	char *buf = my_malloc(strlen(group_name) + 2); /* trailing "/\0" */

	make_group_path(group_name, buf);
	joinpath(group_path, group_path_len, base_dir, buf);
	free(buf);
}


/*
 * Delete index lock
 */
void
cleanup_tmp_files(
	void)
{
	/*
	 * only required if update_index == TRUE, but update_index is
	 * unknown here
	 */
	if (batch_mode)
		unlink(lock_file);
}


/*
 * returns filesize in bytes
 * -1 in case of an error (file not found, or !S_IFREG)
 */
long /* we use long here as off_t might be unsigned on some systems */
file_size(
	const char *file)
{
	struct stat statbuf;

	return (stat(file, &statbuf) == -1 ? -1L : (S_ISREG(statbuf.st_mode)) ? (long) statbuf.st_size : -1L);
}


/*
 * returns mtime
 * -1 in case of an error (file not found, or !S_IFREG)
 */
long /* we use long (not time_t) here */
file_mtime(
	const char *file)
{
	struct stat statbuf;

	return (stat(file, &statbuf) == -1 ? -1L : (S_ISREG(statbuf.st_mode)) ? (long) statbuf.st_mtime : -1L);
}


/*
 * if pointing to a file (absolute path) return a random line from it
 */
char *
random_organization(
	char *in_org)
{
	FILE *orgfp;
	int nool = 0, sol;
	static char selorg[512];

	if (*in_org != '/')
		return in_org;

	*selorg = '\0';

	if ((orgfp = tin_fopen(in_org, "r")) == NULL)
		return selorg;

	while (fgets(selorg, (int) sizeof(selorg), orgfp))
		++nool;

	if (nool) {
		rewind(orgfp);

		srndm();
		sol = rndm() % nool + 1;
		nool = 0;

		while ((nool != sol) && (fgets(selorg, (int) sizeof(selorg), orgfp)))
			++nool;
	}

	fclose(orgfp);
	return selorg;
}


void
read_input_history_file(
	void)
{
	FILE *fp;
	char *chr;
	char buf[HEADER_LEN];
	int his_w = 0, his_e = 0, his_free = 0;

	if ((fp = tin_fopen(local_input_history_file, "r")) == NULL)
		return;

	if (!batch_mode)
		wait_message(0, _(txt_reading_input_history_file), local_input_history_file);

	/* to be safe ;-) */
	memset((void *) input_history, 0, sizeof(input_history));
	memset((void *) hist_last, 0, sizeof(hist_last));
	memset((void *) hist_pos, 0, sizeof(hist_pos));

	while (fgets(buf, (int) sizeof(buf), fp)) {
		if ((chr = strpbrk(buf, "\n\r")) != NULL)
			*chr = '\0';

		if (*buf)
			input_history[his_w][his_e] = my_strdup(buf);
		else {
			/* empty lines in tin_getline's history buf are stored as NULL pointers */
			input_history[his_w][his_e] = NULL;

			/* get the empty slot in the circular buf */
			if (!his_free)
				his_free = his_e;
		}

		++his_e;
		/* check if next type is reached */
		if (his_e >= HIST_SIZE) {
			hist_pos[his_w] = hist_last[his_w] = his_free;
			his_free = his_e = 0;
			++his_w;
		}
		/* check if end is reached */
		if (his_w > HIST_MAXNUM)
			break;
	}
	fclose(fp);

	if (cmd_line)
		my_printf("\r\n");
}


static void
write_input_history_file(
	void)
{
	FILE *fp;
	char *chr;
	char *file_tmp;
	int his_w, his_e;
	mode_t mask;

	if (no_write)
		return;

	mask = umask((mode_t) (S_IRWXO|S_IRWXG));

	/* generate tmp-filename */
	if ((file_tmp = get_tmpfilename(local_input_history_file)) == NULL)
		return;

	if ((fp = fopen(file_tmp, "w")) == NULL) {
		error_message(2, _(txt_filesystem_full_backup), local_input_history_file);
		/* free memory for tmp-filename */
		free(file_tmp);
		umask(mask);
		return;
	}

	for (his_w = 0; his_w <= HIST_MAXNUM; his_w++) {
		for (his_e = 0; his_e < HIST_SIZE; his_e++) {
			/* write an empty line for empty slots */
			if (input_history[his_w][his_e] == NULL)
				fprintf(fp, "\n");
			else {
				if ((chr = strpbrk(input_history[his_w][his_e], "\n\r")) != NULL)
					*chr = '\0';
				fprintf(fp, "%s\n", input_history[his_w][his_e]);
			}
		}
	}
#ifdef HAVE_FCHMOD
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR)); /* rename_file() preserves mode */
#else
#	ifdef HAVE_CHMOD
	chmod(file_tmp, (mode_t) (S_IRUSR|S_IWUSR)); /* rename_file() preserves mode */
#	endif /* HAVE_CHMOD */
#endif /* HAVE_FCHMOD */

	if ((his_w = ferror(fp)) || fclose(fp)) {
		error_message(2, _(txt_filesystem_full), local_input_history_file);
#ifdef HAVE_CHMOD
		/* fix modes for all pre 1.4.1 local_input_history_file files */
		if (chmod(local_input_history_file, (mode_t) (S_IRUSR|S_IWUSR)) == -1) {
#	ifdef DEBUG
			if (debug & DEBUG_MISC)
				perror_message("chmod(%s, %d)", local_input_history_file, (mode_t) (S_IRUSR|S_IWUSR));
#	endif /* DEBUG */
		}
#endif /* HAVE_CHMOD */
		if (his_w) {
			clearerr(fp);
			fclose(fp);
		}
	} else
		rename_file(file_tmp, local_input_history_file);

	umask(mask);
	free(file_tmp);	/* free memory for tmp-filename */
}


/*
 * quotes wildcards * ? \ [ ] with \
 */
char *
quote_wild(
	char *str)
{
	char *target;
	static char buff[2 * LEN];	/* on the safe side */

	for (target = buff; *str != '\0'; str++) {
		if (tinrc.wildcard) { /* regex */
			/*
			 * quote meta characters ()[]{}\^$*+?.#
			 * replace whitespace with '\s' (pcre)
			 */
			if (*str == '(' || *str == ')' || *str == '[' || *str == ']' || *str == '{' || *str == '}'
			    || *str == '\\' || *str == '^' || *str == '$'
			    || *str == '*' || *str == '+' || *str == '?' || *str == '.'
			    || *str == '#'
			    || *str == ' ' || *str == '\t') {
				*target++ = '\\';
				*target++ = ((*str == ' ' || *str == '\t') ? 's' : *str);
			} else
				*target++ = *str;
		} else {	/* wildmat */
			if (*str == '*' || *str == '\\' || *str == '[' || *str == ']' || *str == '?')
				*target++ = '\\';
			*target++ = *str;
		}
	}
	*target = '\0';
	return buff;
}


/*
 * quotes whitespace in regexps for pcre
 */
char *
quote_wild_whitespace(
	char *str)
{
	char *target;
	static char buff[2 * LEN];	/* on the safe side */

	for (target = buff; *str != '\0'; str++) {
		if (tinrc.wildcard) { /* regex */
			/*
			 * replace whitespace with '\s' (pcre)
			 */
			if (*str == ' ' || *str == '\t') {
				*target++ = '\\';
				*target++ = 's';
			} else
				*target++ = *str;
		} else	/* wildmat */
			*target++ = *str;
	}
	*target = '\0';
	return buff;
}


/*
 * strip_name() removes the realname part from a given e-mail address
 */
void
strip_name(
	const char *from,
	char *address)
{
	char name[HEADER_LEN];

	gnksa_do_check_from(from, address, name);
}


#ifdef CHARSET_CONVERSION
/*
 * converts **input of max-length *input_size *from charset *to charset
 * via iconv() or ucnv*(), may reallocate **input and update *input_size
 * accordingly. returns FALSE on error and TRUE otherwise.
 */
static t_bool
buffer_to_local( /* TODO: rename to something more useful/descriptive */
	char **line,
	size_t *max_line_len,
	const char *from_charset,
	const char *to_charset)
{
	t_bool rval = TRUE;

	/* FIXME: this should default in RFC2046.c to US-ASCII */
	if ((from_charset && *from_charset)) {	/* Content-Type: had a charset parameter */
		/* iconv() might crash on broken multibyte sequences so check them */
		if (charset_compare(from_charset, "UTF-8"))
			(void) utf8_valid(*line);

		if (strcasecmp(from_charset, to_charset)) { /* different charsets */
#	ifdef CHARSET_CONVERSION_ICONV
			char *clocal_charset;
			iconv_t cd0, cd1, cd2;

			clocal_charset = my_strdup((to_charset));
#		ifdef HAVE_ICONV_OPEN_TRANSLIT
			if (tinrc.translit)
				clocal_charset = append_to_string(clocal_charset, "//TRANSLIT");
#		endif /* HAVE_ICONV_OPEN_TRANSLIT */

			/*
			 * TODO: hardcode unknown_ucs4 (0x00 0x00 0x00 0x3f)
			 *       instead of converting it?
			 */
			cd0 = iconv_open("UCS-4", "US-ASCII");
			cd1 = iconv_open("UCS-4", from_charset);
			cd2 = iconv_open(clocal_charset, "UCS-4");
			if (cd0 != (iconv_t) (-1) && cd1 != (iconv_t) (-1) && cd2 != (iconv_t) (-1)) {
				char unknown = '?';
				char *unknown_buf;
				char unknown_ucs4[4];
				char *obuf, *outbuf;
				char *tmpbuf, *tbuf;
				ICONV_CONST char *inbuf;
				ICONV_CONST char *unknown_ascii = &unknown;
				ICONV_CONST char *cur_inbuf;
				int used;
				size_t inbytesleft = 1;
				size_t unknown_bytesleft = 4;
				size_t tmpbytesleft, tsize;
				size_t outbytesleft, osize;
				size_t cur_obl, cur_ibl;
				size_t result;

				unknown_buf = unknown_ucs4;

				/* convert '?' from ASCII to UCS-4 */
				iconv(cd0, &unknown_ascii, &inbytesleft, &unknown_buf, &unknown_bytesleft);

				/* temporarily convert to UCS-4 */
				inbuf = (ICONV_CONST char *) *line;
				inbytesleft = strlen(*line);
				tmpbytesleft = inbytesleft * 4 + 4;	/* should be enough */
				tsize = tmpbytesleft;
				tbuf = (char *) my_malloc(tsize);
				tmpbuf = tbuf;

				do {
					errno = 0;
					result = iconv(cd1, &inbuf, &inbytesleft, &tmpbuf, &tmpbytesleft);
					if (result == (size_t) (-1)) {
						switch (errno) {
							case EILSEQ:
								memcpy(tmpbuf, unknown_ucs4, 4);
								tmpbuf += 4;
								tmpbytesleft -= 4;
								++inbuf;
								--inbytesleft;
								break;

							case E2BIG:
								tbuf = my_realloc(tbuf, tsize * 2);
								tmpbuf = (char *) (tbuf + tsize - tmpbytesleft);
								tmpbytesleft += tsize;
								tsize <<= 1; /* double size */
								break;

							default:
								inbytesleft = 0;
						}
					}
				} while (inbytesleft > 0);

				/* now convert from UCS-4 to local charset */
				inbuf = (ICONV_CONST char *) tbuf;
				inbytesleft = tsize - tmpbytesleft;
				outbytesleft = inbytesleft;
				osize = outbytesleft;
				obuf = (char *) my_malloc(osize + 1);
				outbuf = obuf;

				do {
					/*
					 * save the parameters we need to redo the call of iconv
					 * if we get into the E2BIG case
					 */
					cur_inbuf = inbuf;
					cur_ibl = inbytesleft;
					used = (int) (outbuf - obuf);
					cur_obl = outbytesleft;

					errno = 0;
					result = iconv(cd2, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
					if (result == (size_t) (-1)) {
						switch (errno) {
							case EILSEQ:
								*outbuf = '?';
								++outbuf;
								--outbytesleft;
								inbuf += 4;
								inbytesleft -= 4;
								rval = FALSE;
								break;

							case E2BIG:
								/*
								 * outbuf was too small
								 * As some input could be converted successfully
								 * and we don`t know where the last complete char
								 * ends, redo the last conversion completely.
								 */
								/* resize the output buffer */
								obuf = my_realloc(obuf, osize * 2 + 1);
								outbuf = obuf + used;
								outbytesleft = cur_obl + osize;
								osize <<= 1; /* double size */
								/* reset the other params */
								inbuf = cur_inbuf;
								inbytesleft = cur_ibl;
								break;

							default:
								inbytesleft = 0;
						}
					}
				} while (inbytesleft > 0);

				*outbuf = '\0';
				if (*max_line_len < strlen(obuf)) {
					*max_line_len = strlen(obuf);
					*line = my_realloc(*line, *max_line_len + 1);
				}
				strcpy(*line, obuf);
				iconv_close(cd2);
				iconv_close(cd1);
				iconv_close(cd0);
				free(obuf);
				free(tbuf);
			} else {
				if (cd2 != (iconv_t) (-1))
					iconv_close(cd2);
				if (cd1 != (iconv_t) (-1))
					iconv_close(cd1);
				if (cd0 != (iconv_t) (-1))
					iconv_close(cd0);
				free(clocal_charset);
#		ifdef CHARSET_CONVERSION_UCNV
				return (tin_ucnv_buffer_to_local(line, max_line_len, from_charset, to_charset));
#		else
				return FALSE;
#		endif /* CHARSET_CONVERSION_UCNV */
			}
			free(clocal_charset);
#	else
#		ifdef CHARSET_CONVERSION_UCNV
			return (tin_ucnv_buffer_to_local(line, max_line_len, from_charset, to_charset));
#		else
			return FALSE;
#		endif /* CHARSET_CONVERSION_UCNV */
#	endif /* CHARSET_CONVERSION_ICONV */
		}
	}
	return rval;
}


/*
 * converts **line from local_charset to txt_mime_charsets[mmnwcharset]
 * via iconv() or ucnv*(). may reallocate **line if needed. returns
 * TRUE on success and FALSE on error.
 */
t_bool
buffer_to_network(
	char **line,
	int mmnwcharset)
{
	t_bool conv_success = TRUE;
	if (strcasecmp(txt_mime_charsets[mmnwcharset], tinrc.mm_local_charset)) {
		size_t l = strlen(*line);

#	ifdef CHARSET_CONVERSION_ICONV
		conv_success = buffer_to_local(line, &l, tinrc.mm_local_charset, txt_mime_charsets[mmnwcharset]);
#	else
#		ifdef CHARSET_CONVERSION_UCNV
		conv_success = tin_ucnv_buffer_to_local(line, &l, tinrc.mm_local_charset, txt_mime_charsets[mmnwcharset]);
#		endif /* CHARSET_CONVERSION_UCNV */
#	endif /* CHARSET_CONVERSION_ICONV */
	}
	return conv_success;
}
#endif /* CHARSET_CONVERSION */


char *
buffer_to_ascii(
	char *c)
{
	char *a = c;

	while (*c != '\0') {
		/* reduce to US-ASCII, other non-prints are filtered later */
		if ((unsigned char) *c >= 128)
			*c = '?';
		++c;
	}
	return a;
}


/*
 * do some character set processing
 *
 * this is called for headers, overview data, and article bodies
 * to set non-ASCII characters to '?'
 * (only with MIME_STRICT_CHARSET and !NO_LOCALE or CHARSET_CONVERSION
 * and from_charset=="US-ASCII")
 *
 * **line may get reallocated and *max_line_len updated accordingly.
 */
void
process_charsets(
	char **line,
	size_t *max_line_len,
	const char *from_charset,
	const char *to_charset,
	t_bool conv_tex2iso)
{
	char *p;

#ifdef CHARSET_CONVERSION
	if (strcasecmp(from_charset, "US-ASCII")) {	/* from_charset is NOT US-ASCII */
		if (!buffer_to_local(line, max_line_len, from_charset, iso2asc_supported >= 0 ? "ISO-8859-1" : to_charset))
			buffer_to_ascii(*line);
	} else /* set non-ASCII characters to '?' */
		buffer_to_ascii(*line);
#else
#	if defined(MIME_STRICT_CHARSET) && !defined(NO_LOCALE)
	if ((to_charset && strcasecmp(from_charset, to_charset)) || !strcasecmp(from_charset, "US-ASCII"))
		/* different charsets || network charset is US-ASCII (see below) */
		buffer_to_ascii(*line);
#	else
	(void) to_charset;
#	endif /* MIME_STRICT_CHARSET && !NO_LOCALE */
	/* charset conversion (codepage version) */
#endif /* CHARSET_CONVERSION */

	/*
	 * TEX2ISO conversion should be done before ISO2ASC conversion
	 * to allow TEX2ISO && ISO2ASC, i.e. "a -> auml -> ae
	 */
	if (conv_tex2iso) {
		p = my_strdup(*line);
		convert_tex2iso(p, *line);
		free(p);
	}

	/* iso2asc support */
#ifdef CHARSET_CONVERSION
	if (iso2asc_supported >= 0)
#else
	if (iso2asc_supported >= 0 && !strcasecmp(from_charset, "ISO-8859-1"))
#endif /* CHARSET_CONVERSION */
	{
		p = my_strdup(*line);
		convert_iso2asc(p, line, max_line_len, iso2asc_supported);
		free(p);
		/* we now have (decorated) US-ASCII */
	}
}


/*
 * checking of mail addresses for GNKSA compliance
 *
 * son of RFC 1036:
 *   article         = 1*header separator body
 *   header          = start-line *continuation
 *   start-line      = header-name ":" space [ nonblank-text ] eol
 *   continuation    = space nonblank-text eol
 *   header-name     = 1*name-character *( "-" 1*name-character )
 *   name-character  = letter / digit
 *   letter          = <ASCII letter A-Z or a-z>
 *   digit           = <ASCII digit 0-9>
 *   separator       = eol
 *   body            = *( [ nonblank-text / space ] eol )
 *   eol             = <EOL>
 *   nonblank-text   = [ space ] text-character *( space-or-text )
 *   text-character  = <any ASCII character except NUL (ASCII 0),
 *                       HT (ASCII 9), LF (ASCII 10), CR (ASCII 13),
 *                       or blank (ASCII 32)>
 *   space           = 1*( <HT (ASCII 9)> / <blank (ASCII 32)> )
 *   space-or-text   = space / text-character
 *   encoded-word  = "=?" charset "?" encoding "?" codes "?="
 *   charset       = 1*tag-char
 *   encoding      = 1*tag-char
 *   tag-char      = <ASCII printable character except !()<>@,;:\"[]/?=>
 *   codes         = 1*code-char
 *   code-char     = <ASCII printable character except ?>
 *   From-content  = address [ space "(" paren-phrase ")" ]
 *                 /  [ plain-phrase space ] "<" address ">"
 *   paren-phrase  = 1*( paren-char / space / encoded-word )
 *   paren-char    = <ASCII printable character except ()<>\>
 *   plain-phrase  = plain-word *( space plain-word )
 *   plain-word    = unquoted-word / quoted-word / encoded-word
 *   unquoted-word = 1*unquoted-char
 *   unquoted-char = <ASCII printable character except !()<>@,;:\".[]>
 *   quoted-word   = quote 1*( quoted-char / space ) quote
 *   quote         = <" (ASCII 34)>
 *   quoted-char   = <ASCII printable character except "()<>\>
 *   address       = local-part "@" domain
 *   local-part    = unquoted-word *( "." unquoted-word )
 *   domain        = unquoted-word *( "." unquoted-word )
*/


/*
 * legal domain name components according to RFC 1034
 * includes also '.' as valid separator
 */
static char gnksa_legal_fqdn_chars[256] = {
/*         0 1 2 3  4 5 6 7  8 9 a b  c d e f */
/* 0x00 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x10 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x20 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,1,0,
/* 0x30 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
/* 0x40 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x50 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
/* 0x60 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x70 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
/* 0x80 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x90 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xa0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xb0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xc0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xd0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xe0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xf0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};


/*
 * legal localpart components according to son of RFC 1036
 * includes also '.' as valid separator
 */
static char gnksa_legal_localpart_chars[256] = {
/*         0 1 2 3  4 5 6 7  8 9 a b  c d e f */
/* 0x00 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x10 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x20 */ 0,1,0,1, 1,1,1,1, 0,0,1,1, 0,1,1,1,
/* 0x30 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,1,0,1,
/* 0x40 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x50 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,1,1,
/* 0x60 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x70 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0,
/* 0x80 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x90 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xa0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xb0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xc0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xd0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xe0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xf0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};


/*
 * legal realname characters according to son of RFC 1036
 *
 * we also allow CR & LF for folding
 */
static char gnksa_legal_realname_chars[256] = {
/*         0 1 2 3  4 5 6 7  8 9 a b  c d e f */
/* 0x00 */ 0,0,0,0, 0,0,0,0, 0,0,1,0, 0,1,0,0,
/* 0x10 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x20 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x30 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x40 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x50 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x60 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 0x70 */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0,
/* 0x80 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0x90 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xa0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xb0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xc0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xd0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xe0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 0xf0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};


/*
 * return error message string for given code
 */
const char *
gnksa_strerror(
	int errcode)
{
	const char *message;

	switch (errcode) {
		case GNKSA_INTERNAL_ERROR:
			message = _(txt_error_gnksa_internal);
			break;

		case GNKSA_LANGLE_MISSING:
			message = _(txt_error_gnksa_langle);
			break;

		case GNKSA_LPAREN_MISSING:
			message = _(txt_error_gnksa_lparen);
			break;

		case GNKSA_RPAREN_MISSING:
			message = _(txt_error_gnksa_rparen);
			break;

		case GNKSA_ATSIGN_MISSING:
			message = _(txt_error_gnksa_atsign);
			break;

		case GNKSA_RANGLE_MISSING:
			message = _(txt_error_gnksa_rangle);
			break;

		case GNKSA_SINGLE_DOMAIN:
			message = _(txt_error_gnksa_sgl_domain);
			break;

		case GNKSA_INVALID_DOMAIN:
			message = _(txt_error_gnksa_inv_domain);
			break;

		case GNKSA_ILLEGAL_DOMAIN:
			message = _(txt_error_gnksa_ill_domain);
			break;

		case GNKSA_UNKNOWN_DOMAIN:
			message = _(txt_error_gnksa_unk_domain);
			break;

		case GNKSA_INVALID_FQDN_CHAR:
			message = _(txt_error_gnksa_fqdn);
			break;

		case GNKSA_ZERO_LENGTH_LABEL:
			message = _(txt_error_gnksa_zero);
			break;

		case GNKSA_ILLEGAL_LABEL_LENGTH:
			message = _(txt_error_gnksa_length);
			break;

		case GNKSA_ILLEGAL_LABEL_HYPHEN:
			message = _(txt_error_gnksa_hyphen);
			break;

		case GNKSA_ILLEGAL_LABEL_BEGNUM:
			message = _(txt_error_gnksa_begnum);
			break;

		case GNKSA_BAD_DOMAIN_LITERAL:
			message = _(txt_error_gnksa_bad_lit);
			break;

		case GNKSA_LOCAL_DOMAIN_LITERAL:
			message = _(txt_error_gnksa_local_lit);
			break;

		case GNKSA_RBRACKET_MISSING:
			message = _(txt_error_gnksa_rbracket);
			break;

		case GNKSA_LOCALPART_MISSING:
			message = _(txt_error_gnksa_lp_missing);
			break;

		case GNKSA_INVALID_LOCALPART:
			message = _(txt_error_gnksa_lp_invalid);
			break;

		case GNKSA_ZERO_LENGTH_LOCAL_WORD:
			message = _(txt_error_gnksa_lp_zero);
			break;

		case GNKSA_ILLEGAL_UNQUOTED_CHAR:
			message = _(txt_error_gnksa_rn_unq);
			break;

		case GNKSA_ILLEGAL_QUOTED_CHAR:
			message = _(txt_error_gnksa_rn_qtd);
			break;

		case GNKSA_ILLEGAL_ENCODED_CHAR:
			message = _(txt_error_gnksa_rn_enc);
			break;

		case GNKSA_BAD_ENCODE_SYNTAX:
			message = _(txt_error_gnksa_rn_encsyn);
			break;

		case GNKSA_ILLEGAL_PAREN_CHAR:
			message = _(txt_error_gnksa_rn_paren);
			break;

		case GNKSA_INVALID_REALNAME:
			message = _(txt_error_gnksa_rn_invalid);
			break;

		case GNKSA_MISSING_REALNAME:
			message = _(txt_error_gnksa_rn_missing);
			break;

		case GNKSA_OK:
		default:
			/* shouldn't happen */
			message = "";
			break;
	}

	return message;
}


/*
 * decode realname into displayable string
 * this only does RFC 822 decoding, decoding RFC 2047 encoded parts must
 * be done by another call to the appropriate function
 */
static int
gnksa_dequote_plainphrase(
	char *realname,
	char *decoded,
	int addrtype)
{
	const char *rpos;	/* read position */
	char *wpos;	/* write position */
	int initialstate;	/* initial state */
	int state;	/* current state */

	/* initialize state machine */
	switch (addrtype) {
		case GNKSA_ADDRTYPE_ROUTE:
			initialstate = 0;
			break;

		case GNKSA_ADDRTYPE_OLDSTYLE:
			initialstate = 5;
			break;

		default:
			/* shouldn't happen */
			return GNKSA_INTERNAL_ERROR;
			/* NOTREACHED */
			break;
	}
	state = initialstate;
	rpos = realname;
	wpos = decoded;

	/* decode realname */
	while (*rpos) {
		if (!gnksa_legal_realname_chars[(unsigned char) *rpos])
			return GNKSA_INVALID_REALNAME;

		switch (state) {
			case 0:
				/* in unquoted word, route address style */
				switch (*rpos) {
					case '"':
						state = 1;
						++rpos;
						break;

					case '(':
					case ')':
					case '<':
					case '>':
					case '@':
					case ',':
					case ';':
					case ':':
					case '\\':
					case '.':
					case '[':
					case ']':
						return GNKSA_ILLEGAL_UNQUOTED_CHAR;
						/* NOTREACHED */
						break;

					case '=':
						*(wpos++) = *(rpos++);
						if (*rpos == '?') {
							state = 2;
							*(wpos++) = *(rpos++);
						} else
							state = 0;
						break;

					default:
						state = 0;
						*(wpos++) = *(rpos++);
						break;
				}
				break;

			case 1:
				/* in quoted word */
				switch (*rpos) {
					case '"':
						state = 0;
						++rpos;
						break;

					/*
					 * FIXME: \ is allowed in dquotes as of 5322,
					 * but we need to ensure that \" doesn't end
					 * current state as it currently would.
					 */
					case '\\':
						return GNKSA_ILLEGAL_QUOTED_CHAR;
						/* NOTREACHED */
						break;

					default:
						state = 1;
						*(wpos++) = *(rpos++);
						break;
				}
				break;

			case 2:
				/* in encoded word, charset part */
				switch (*rpos) {
					case '?':
						state = 3;
						*(wpos++) = *(rpos++);
						break;

					case '!':
					case '(':
					case ')':
					case '<':
					case '>':
					case '@':
					case ',':
					case ';':
					case ':':
					case '\\':
					case '"':
					case '[':
					case ']':
					case '/':
					case '=':
						return GNKSA_ILLEGAL_ENCODED_CHAR;
						/* NOTREACHED */
						break;

					default:
						state = 2;
						*(wpos++) = *(rpos++);
						break;
				}
				break;

			case 3:
				/* in encoded word, encoding part */
				switch (*rpos) {
					case '?':
						state = 4;
						*(wpos++) = *(rpos++);
						break;

					case '!':
					case '(':
					case ')':
					case '<':
					case '>':
					case '@':
					case ',':
					case ';':
					case ':':
					case '\\':
					case '"':
					case '[':
					case ']':
					case '/':
					case '=':
						return GNKSA_ILLEGAL_ENCODED_CHAR;
						/* NOTREACHED */
						break;

					default:
						state = 3;
						*(wpos++) = *(rpos++);
						break;
				}
				break;

			case 4:
				/* in encoded word, codes part */
				switch (*rpos) {
					case '?':
						*(wpos++) = *(rpos++);
						if (*rpos == '=') {
							state = initialstate;
							*(wpos++) = *(rpos++);
						} else
							return GNKSA_BAD_ENCODE_SYNTAX;
						break;

					default:
						state = 4;
						*(wpos++) = *(rpos++);
						break;
				}
				break;

			case 5:
				/* in word, old style address */
				switch (*rpos) {
					case '(':
					case ')':
					case '\\':
						return GNKSA_ILLEGAL_PAREN_CHAR;
						/* NOTREACHED */
						break;

					case '=':
						*(wpos++) = *(rpos++);
						if (*rpos == '?') {
							state = 2;
							*(wpos++) = *(rpos++);
						} else
							state = 5;
						break;

					default:
						state = 5;
						*(wpos++) = *(rpos++);
						break;
				}
				break;

			default:
				/* shouldn't happen */
				return GNKSA_INTERNAL_ERROR;
		}
	}

	/* non fatal error is checked last */
	if (!*realname)
		return GNKSA_MISSING_REALNAME;

	/* successful */
	*wpos = '\0';
	return GNKSA_OK;
}


/*
 * check domain literal (IPv4, IPv6)
 */
static int
gnksa_check_domain_literal(
	const char *domain)
{
	char term = '\0';
	unsigned int x1, x2, x3, x4;

	x1 = x2 = x3 = x4 = 666;

	if (*domain == '[') { /* literal bracketed */
#if defined(HAVE_INET_PTON) && defined(AF_INET6)
		/*
		 * academic: what about systenms without AF_INET6?
		 * they would reject IPv6 address (so no change there),
		 * do we need a handrolled parser for those?
		 */
		if (strchr(domain + 1, ':')) {
			char *p;
			int n;
			size_t l = strlen(domain);
			struct in6_addr inaddr;

			if (l < 4 || l > 41) /* "[::]" ... "[acab:cafe:deef:dead:beef:babe:face:feed]" */
				return GNKSA_BAD_DOMAIN_LITERAL;

			if (domain[l - 1] != ']')
				return GNKSA_RBRACKET_MISSING;
			else
				p = my_strndup(domain + 1, l - 2);

			if (inet_pton(AF_INET6, p, &inaddr) != 1)
				n = GNKSA_BAD_DOMAIN_LITERAL;
			else {
#	if 0	/* TODO: filter out link-lokal ... see IPv4 code blow */
				wait_message(2, "IPv6 uncompressed: [%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]",
					inaddr.s6_addr[0],  inaddr.s6_addr[1],
					inaddr.s6_addr[2],  inaddr.s6_addr[3],
					inaddr.s6_addr[4],  inaddr.s6_addr[5],
					inaddr.s6_addr[6],  inaddr.s6_addr[7],
					inaddr.s6_addr[8],  inaddr.s6_addr[9],
					inaddr.s6_addr[10], inaddr.s6_addr[11],
					inaddr.s6_addr[12], inaddr.s6_addr[13],
					inaddr.s6_addr[14], inaddr.s6_addr[15]);
#	endif /* 0*/
				n = GNKSA_OK;
			}

			free(p);
			return n;
		}
#endif /* HAVE_INET_PTON && AF_INET6 */

		if (sscanf(domain, "[%u.%u.%u.%u%c", &x1, &x2, &x3, &x4, &term) != 5)
			return GNKSA_BAD_DOMAIN_LITERAL;

		if (term != ']')
			return GNKSA_BAD_DOMAIN_LITERAL;

	} else { /* literal not bracketed */
#ifdef REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
		return GNKSA_RBRACKET_MISSING;
#else
		if (sscanf(domain, "%u.%u.%u.%u%c", &x1, &x2, &x3, &x4, &term) != 4)	/* there should be no terminating character present */
			return GNKSA_BAD_DOMAIN_LITERAL;
#endif /* REQUIRE_BRACKETS_IN_DOMAIN_LITERAL */
	}

	/* check ip parts for legal range */
	if ((x1 > 255) || (x2 > 255) || (x3 > 255) || (x4 > 255))
		return GNKSA_BAD_DOMAIN_LITERAL;

	/* check for private ip or localhost - see RFC 5735, RFC 5737 */
	/*
	 * TODO: as we 'abuse' disable_gnksa_domain_check to skip TLD
	 *       checking since 20131126 as there are gazillions of new
	 *       TLD showing up, we may reconsider NOT to skip checking
	 *       IPs here.
	 */
	if ((!disable_gnksa_domain_check)
	    && ((x1 == 0)				/* local network */
		|| (x1 == 10)				/* private class A */
		|| (x1 == 127)				/* loopback */
		|| ((x1 == 172) && ((x2 & 0xf0) == 16))	/* private /12 */
		|| ((x1 == 192) && (x2 == 168))		/* private class B */
		|| ((x1 == 192) && (x2 == 0) && (x3 == 2)) /* TEST NET-1 */
		|| ((x1 == 198) && (x2 == 51) && (x3 == 100)) /* TEST NET-2 */
		|| ((x1 == 203) && (x2 == 0) && (x3 == 113)))) /* TEST NET-3 */
		return GNKSA_LOCAL_DOMAIN_LITERAL;

	return GNKSA_OK;
}


static int
gnksa_check_domain(
	char *domain)
{
	char *aux;
	char *last;
	int i;
	int result;

	/* check for domain literal */
	if (*domain == '[') /* check value of domain literal */
		return gnksa_check_domain_literal(domain);

	/* check for leading or trailing dot */
	if ((*domain == '.') || (*(domain + strlen(domain) - 1) == '.'))
		return GNKSA_ZERO_LENGTH_LABEL;

	/* look for TLD start */
	if ((aux = strrchr(domain, '.')) == NULL)
		return GNKSA_SINGLE_DOMAIN;

	++aux;

	/* check existence of toplevel domain */
	switch ((int) strlen(aux)) {
		case 1:
			/* no numeric components allowed */
			if ((*aux >= '0') && (*aux <= '9'))
				return gnksa_check_domain_literal(domain);

			/* single letter TLDs do not exist */
			return GNKSA_ILLEGAL_DOMAIN;
			/* NOTREACHED */
			break;

		case 2:
			/* no numeric components allowed */
			if ((*aux >= '0') && (*aux <= '9')
			    && (*(aux + 1) >= '0') && (*(aux + 1) <= '9'))
				return gnksa_check_domain_literal(domain);

			if ((*aux >= 'a') && (*aux <= 'z')
			    && (*(aux + 1) >= 'a') && (*(aux + 1) <= 'z')) {
				i = ((*aux - 'a') * 26) + (*(aux + 1)) - 'a';
				if (!gnksa_country_codes[i])
					return GNKSA_UNKNOWN_DOMAIN;
			} else
				return GNKSA_ILLEGAL_DOMAIN;
			break;

		case 3:
			/* no numeric components allowed */
			if ((*aux >= '0') && (*aux <= '9')
			    && (*(aux + 1) >= '0') && (*(aux + 1) <= '9')
			    && (*(aux + 2) >= '0') && (*(aux + 2) <= '9'))
				return gnksa_check_domain_literal(domain);
			/* FALLTHROUGH */
		default:
			/* check for valid domains */
			if (!disable_gnksa_domain_check) {
				result = GNKSA_INVALID_DOMAIN;
				for (i = 0; *gnksa_domain_list[i]; i++) {
					if (!strcmp(aux, gnksa_domain_list[i])) {
						result = GNKSA_OK;
						break;
					}
				}
				if (result != GNKSA_OK)
					return result;
			}
			break;
	}

	/* check for illegal labels */
	last = domain;
	for (aux = domain; *aux; aux++) {
		if (*aux == '.') {
			if (aux - last - 1 > 63)
				return GNKSA_ILLEGAL_LABEL_LENGTH;

			if (*(aux + 1) == '.')
				return GNKSA_ZERO_LENGTH_LABEL;

			if ((*(aux + 1) == '-') || (*(aux - 1) == '-'))
				return GNKSA_ILLEGAL_LABEL_HYPHEN;

#ifdef ENFORCE_RFC1034
			if ((*(aux + 1) >= '0') && (*(aux + 1) <= '9'))
				return GNKSA_ILLEGAL_LABEL_BEGNUM;
#endif /* ENFORCE_RFC1034 */
			last = aux;
		}
	}

	/* check for illegal characters in FQDN */
	for (aux = domain; *aux; aux++) {
		if (!gnksa_legal_fqdn_chars[(unsigned char) *aux])
			return GNKSA_INVALID_FQDN_CHAR;
	}

	return GNKSA_OK;
}


/*
 * check localpart of address
 */
static int
gnksa_check_localpart(
	const char *localpart)
{
	const char *aux;

	/* no localpart at all? */
	if (!*localpart)
		return GNKSA_LOCALPART_MISSING;

	/* check for zero-length domain parts */
	if ((*localpart == '.') || (*(localpart + strlen(localpart) - 1) == '.'))
		return GNKSA_ZERO_LENGTH_LOCAL_WORD;

	for (aux = localpart; *aux; aux++) {
		if ((*aux == '.') && (*(aux + 1) == '.'))
			return GNKSA_ZERO_LENGTH_LOCAL_WORD;
		/* check for illegal characters in FQDN */
		if (!gnksa_legal_localpart_chars[(unsigned char) *aux])
			return GNKSA_INVALID_LOCALPART;
	}

	return GNKSA_OK;
}


/*
 * split mail address into realname and address parts
 */
int
gnksa_split_from(
	const char *from,
	char *address,
	char *realname,
	int *addrtype)
{
	char *addr_begin, *addr_end, *ptr;
	char work[HEADER_LEN];

	/* init return variables */
	*address = *realname = '\0';

	/* copy raw address into work area */
	STRCPY(work, from);
	strip_line(work);
	strcpy(address, work);

	if (!*work) {
		*addrtype = GNKSA_ADDRTYPE_OLDSTYLE;
		return GNKSA_ATSIGN_MISSING; /* GNKSA_LPAREN_MISSING */
	}

	/*
	 * RFC5322 allows CFWS after "<" addr-spec ">"
	 * if '>' is present, everything after it (possible comments)
	 * is removed
	 */
	if ((addr_end = strrchr(work, '>'))) {
		ptr = strrchr(work, ')');
		if (!ptr || addr_end > ptr)
			*(addr_end + 1) = '\0';

		ptr = strrchr(work, '(');
		if (!ptr || addr_end < ptr)
			*(addr_end + 1) = '\0';

		str_trim(work);
	}

	addr_end = work + strlen(work) - 1;
	if (*addr_end == '>') {
		/* route-address used */
		*addrtype = GNKSA_ADDRTYPE_ROUTE;

		/* get address part */
		addr_begin = addr_end;
		while (('<' != *addr_begin) && (addr_begin > work))
			--addr_begin;

		if (*addr_begin != '<') /* syntax error in mail address */
			return GNKSA_LANGLE_MISSING;

		*addr_end = *addr_begin = '\0';
		/* copy route address */
		strcpy(address, addr_begin + 1);

		/*
		 * if we allow <> as From: we must disallow <> as Message-ID,
		 * see code in post.c:check_article_to_be_posted()
		 */
#if 0
		if (!strchr(address, '@') && *address) /* check for From: without an @ but allow <> */
#else
		if (!strchr(address, '@')) /* check for From: without an @ */
#endif /* 0 */
			return GNKSA_ATSIGN_MISSING;

		/* missing realname */
		if (addr_begin == work)
			return GNKSA_MISSING_REALNAME;

		/* get realname part */
		addr_end = addr_begin - 1;
		addr_begin = work;

		/* strip surrounding whitespace */
		strip_line(addr_end);
		while ((*addr_begin == ' ') || (*addr_begin == '\t'))
			++addr_begin;

#if 0	/* whitespace only realname */
		strip_line(addr_begin);
		if (!*addr_begin)
			return GNKSA_WHITESPACE_REALNAME;
		else
#endif /* 0 */
		/* copy realname */
		strcpy(realname, addr_begin);
		remove_comments(realname);
		strip_line(realname);
	} else {
		size_t l;

		/* old-style address used */
		*addrtype = GNKSA_ADDRTYPE_OLDSTYLE;

		/* get address part */
		/* skip leading whitespace */
		addr_begin = work;
		while ((*addr_begin == ' ') || (*addr_begin == '\t'))
			++addr_begin;

		if (*addr_begin == '<') {
			*addrtype = GNKSA_ADDRTYPE_ROUTE;
			return GNKSA_RANGLE_MISSING;
		}

		/* scan forward to next whitespace or null */
		l = strlen(addr_begin);
		addr_end = addr_begin;

		while (*addr_end && (*addr_end != ' ') && (*addr_end != '\t'))
			++addr_end;

		*addr_end = '\0';

		/* copy route address */
		strcpy(address, addr_begin);

		/*
		 * if we allow <> as From: we must disallow <> as Message-ID,
		 * see code in post.c:check_article_to_be_posted()
		 */
#if 0
		if (!strchr(address, '@') && *address) /* check for From: without an @ but allow <> */
#else
		if (!strchr(address, '@')) /* check for From: without an @ */
#endif /* 0 */
			return GNKSA_ATSIGN_MISSING;

		if (l == strlen(address))
			return GNKSA_MISSING_REALNAME;

		/* get realname part */
		addr_begin = addr_end + 1;
		addr_end = addr_begin + strlen(addr_begin) - 1;

		/* strip surrounding whitespace */
		strip_line(addr_end);
		while (*addr_begin == ' ' || *addr_begin == '\t')
			++addr_begin;

		/* any realname at all? */
		if (*addr_begin) {
			/* check for parentheses */
			if (*addr_begin != '(')
				return GNKSA_LPAREN_MISSING;

			if (*addr_end != ')')
				return GNKSA_RPAREN_MISSING;

			if ((addr_end > addr_begin + 2) && *(addr_end - 1) == '\\' && *(addr_end - 2) != '\\')
				return GNKSA_RPAREN_MISSING;

			/* copy realname */
			*addr_end = '\0';
			strcpy(realname, addr_begin + 1);
		}
	}

	/* split successful */
	return GNKSA_OK;
}

#define GNKSA_REALNAME_ISSUES(a) ((a == GNKSA_MISSING_REALNAME || a == GNKSA_LPAREN_MISSING || a == GNKSA_RPAREN_MISSING))
/*
 * restrictive check for valid address conforming to RFC 1036, son of RFC 1036
 * and draft-usefor-article-xx.txt
 */
int
gnksa_do_check_from(
	const char *from,
	char *address,
	char *realname)
{
	char *addr_begin;
	char decoded[HEADER_LEN];
	int result, code, addrtype;

	decoded[0] = '\0';

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		debug_print_file("GNKSA", "From:=[%s]", from);
#endif /* DEBUG */

	/* split from */
	code = gnksa_split_from(from, address, realname, &addrtype);
	if (*address == '\0') /* address missing or not extractable */
		return code;

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		debug_print_file("GNKSA", "address=[%s]", address);
#endif /* DEBUG */

	/* parse address */
	addr_begin = strrchr(address, '@');

	if (addr_begin != NULL) {
		/* temporarily terminate string at separator position */
		*addr_begin++ = '\0';

#ifdef DEBUG
		if (debug & DEBUG_MISC)
			debug_print_file("GNKSA", "FQDN=[%s]", addr_begin);
#endif /* DEBUG */

		/* convert FQDN part to lowercase */
		str_lwr(addr_begin);

#ifdef DEBUG
		if (debug & DEBUG_MISC)
			debug_print_file("GNKSA", "str_lwr(FQDN)=[%s]", addr_begin);
#endif /* DEBUG */

		if ((result = gnksa_check_domain(addr_begin)) != GNKSA_OK
		    && (code == GNKSA_OK || GNKSA_REALNAME_ISSUES(code))) /* error detected */
			code = result;

		if ((result = gnksa_check_localpart(address)) != GNKSA_OK
		    && (code == GNKSA_OK || GNKSA_REALNAME_ISSUES(code))) /* error detected */
			code = result;

		/* restore separator character */
		*--addr_begin = '@';
	} else
		code = GNKSA_ATSIGN_MISSING;

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		debug_print_file("GNKSA", "realname=[%s]", realname);
#endif /* DEBUG */

	/* check realname */
	if ((result = gnksa_dequote_plainphrase(realname, decoded, addrtype)) != GNKSA_OK) {
		if (code == GNKSA_OK) /* error detected */
			code = result;
	} else	/* copy dequoted realname to result variable */
		strcpy(realname, decoded);

#ifdef DEBUG
	if (debug & DEBUG_MISC) {
		if (code != GNKSA_OK)
			debug_print_file("GNKSA", "From:=[%s], GNKSA=[%d]\n", from, code);
		else
			debug_print_file("GNKSA", "GNKSA=[%d]\n", code);
	}
#endif /* DEBUG */

	return code;
}


/*
 * check given address
 */
int
gnksa_check_from(
	const char *from)
{
	char address[HEADER_LEN];	/* will be initialised in gnksa_split_from() */
	char realname[HEADER_LEN];	/* which is called by gnksa_do_check_from() */

	return gnksa_do_check_from(from, address, realname);
}


/*
 * parse given address
 * return error code on GNKSA check failure
 */
int
parse_from(
	const char *from,
	char *address,
	char *realname)
{
	return gnksa_do_check_from(from, address, realname);
}


/*
 * Strip trailing blanks, tabs, \r and \n
 */
char *
strip_line(
	char *line)
{
	char *ptr;

	if (!*line)
		return line;

	ptr = line + strlen(line);
	do {
		--ptr;
		if (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')
			*ptr = '\0';
		else
			break;
	} while (ptr > line);

	return line;
}


#if defined(CHARSET_CONVERSION) || (defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE))
/*
 * 'check' a given UTF-8 string and '?'-out illegal sequences
 * TODO: is this check complete?
 *
 * UTF-8           = ASCII / UTF-8-non-ascii
 * ASCII           = %x00-%x7F
 * UTF-8-non-ascii = UTF8-2 / UTF8-3 / UTF8-4
 * UTF8-1          = %x80-BF
 * UTF8-2          = %xC2-DF 1*UTF8-1
 * UTF8-3          = %xE0 %xA0-BF 1*UTF8-1 / %xE1-EC 2*UTF8-1 /
 *                   %xED %x80-9F 1*UTF8-1 / %xEE-EF 2*UTF8-1
 * UTF8-4          = %xF0 %x90-BF 2*UTF8-1 / %xF1-F3 3*UTF8-1 /
 *                   %xF4 %x80-8F 2*UTF8-1
 */
char *
utf8_valid(
	char *line)
{
	char *c;
	unsigned char d, e, f, g;
	unsigned bits;
	int numc;
	t_bool illegal;

	c = line;

	while (*c != '\0') {
		if (!(*c & 0x80)) { /* plain US-ASCII? */
			++c;
			continue;
		}

		numc = 0;
		illegal = FALSE;
		bits = (*c & 0x7c);	/* remove bits 7,1,0 */

		do {
			++numc;
		} while ((bits <<= 1) & 0x80);	/* get sequence length */

		if (c + numc > line + strlen(line)) { /* sequence runs past end of string */
			illegal = TRUE;
			numc = (int) (line + strlen(line) - c);
		}

		if (!illegal) {
			d = (unsigned char) *c;
			e = (unsigned char) *(c + 1);

			switch (numc) {
				case 2:
					/* out of range or sequences which would also fit into 1 byte */
					if (d < 0xc2 || d > 0xdf)
						illegal = TRUE;
					break;

				case 3:
					f = (unsigned char) *(c + 2);
					/* out of range or sequences which would also fit into 2 bytes */
					if (d < 0xe0 || d > 0xef || (d == 0xe0 && e < 0xa0))
						illegal = TRUE;
					/* U+D800 ... U+DBFF, U+DC00 ... U+DFFF (high-surrogates, low-surrogates) */
					if (d == 0xed && e > 0x9f)
						illegal = TRUE;
					/* U+FDD0 ... U+FDEF */
					if (d == 0xef && e == 0xb7 && (f >= 0x90 && f <= 0xaf))
						illegal = TRUE;
					/* U+FFFE, U+FFFF (noncharacters) */
					if (d == 0xef && e == 0xbf && (f == 0xbe || f == 0xbf))
						illegal = TRUE;
					break;

				case 4:
					f = (unsigned char) *(c + 2);
					g = (unsigned char) *(c + 3);
					/* out of range or sequences which would also fit into 3 bytes */
					if (d < 0xf0 || d > 0xf7 || (d == 0xf0 && e < 0x90))
						illegal = TRUE;
					/* largest current used sequence */
					if ((d == 0xf4 && e > 0x8f) || d > 0xf4)
						illegal = TRUE;
					/* Unicode 3.1 noncharacters */
					/* U+1FFFE, U+1FFFF, U+2FFFE, U+2FFFF, U+3FFFE, U+3FFFF; (Unicode 3.1) */
					if (d == 0xf0 && (e == 0x9f || e == 0xaf || e == 0xbf) && f == 0xbf && (g == 0xbe || g == 0xbf))
						illegal = TRUE;
					/* Unicode 3.1 noncharacters */
					/* U+4FFFE, U+4FFFF, U+5FFFE, U+5FFFF, U+6FFFE, U+6FFFF, U+7FFFE, U+7FFFF */
					/* U+8FFFE, U+8FFFF, U+9FFFE, U+9FFFF, U+AFFFE, U+AFFFF, U+BFFFE, U+BFFFF */
					/* U+CFFFE, U+CFFFF, U+DFFFE, U+DFFFF, U+EFFFE, U+EFFFF, U+FFFFE, U+FFFFF */
					if ((d == 0xf1 || d == 0xf2 || d == 0xf3) && (e == 0x8f || e == 0x9f || e == 0xaf || e == 0xbf) && f == 0xbf && (g == 0xbe || g == 0xbf))
						illegal = TRUE;
					/* Unicode 3.1 noncharacters */
					/* U+10FFFE, U+10FFFF */
					if (d == 0xf4 && e == 0x8f && f == 0xbf && (g == 0xbe || g == 0xbf))
						illegal = TRUE;
					break;

#	if 0	/* currently not used, see also check above; RFC 3629 limits UTF-8 to <= U+10FFFF */
				case 5:
					/* out of range or sequences which would also fit into 4 bytes */
					if (d < 0xf8 || d > 0xfb || (d == 0xf8 && e < 0x88))
						illegal = TRUE;
					break;

				case 6:
					/* out of range or sequences which would also fit into 5 bytes */
					if (d < 0xfc || d > 0xfd || (d == 0xfc && e < 0x84))
						illegal = TRUE;
					break;
#	endif /* 0 */

				default:
					/*
					 * with the check for plain US-ASCII above all other sequence
					 * length are illegal.
					 */
					illegal = TRUE;
					break;
			}
		}

		for (d = 1; d < numc && !illegal; d++) {
			e = (unsigned char) *(c + d);
			if (e < 0x80 || e > 0xbf)
				illegal = TRUE;
		}

		if (!illegal)
			c += numc; /* skip over valid sequence */
		else {
			while (numc--) {
				if (*c == '\0' || *c == '\n')
					break;
				if (*c & 0x80)	/* replace 'dangerous' bytes */
					*c = '?';
				++c;
			}
		}
	}
	return line;
}
#endif /* CHARSET_CONVERSION || (MULTIBYTE_ABLE && !NO_LOCALE) */


#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
/*
 * Unicode Standard Annex #14 5.4 "Use of Soft Hyphens"
 * <https://www.unicode.org/reports/tr14/#SoftHyphen> states:
 *
 * "[...] the character U+00AD SOFT HYPHEN (SHY) is an invisible format
 * character that merely indicates a preferred intraword line break
 * position."
 *
 * -> remove SOFT HYPHENs from the given UTF-8 string to prevent
 *    terminal programs from displaying them incorrectly
 */
void
remove_soft_hyphens(
	char *line)
{
	char *buffer;
	size_t len;
	wchar_t *wbuffer, *rptr, *wptr;

	if ((wbuffer = char2wchar_t(line)) != NULL) {
		rptr = wptr = wbuffer;
		while (*rptr) {
			if (*rptr == 0xad)
				++rptr;
			if (*rptr && *rptr != 0xad)
				*wptr++ = *rptr++;
		}
		*wptr = '\0';

		if ((buffer = wchar_t2char(wbuffer)) != NULL) {
			len = strlen(line) + 1;
			strncpy(line, buffer, len);
			line[len - 1] = '\0';
			free(buffer);
		}
		free(wbuffer);
	}
}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */


/*
 * returns freshly allocated mem
 */
char *
idna_decode(
	char *in)
{
	char *out = my_strdup(in);

	/* decoding needed? */
	if (!strstr(in, "xn--"))
		return out;

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	/* IDNA 2008 */
#	if defined(HAVE_LIBIDN2)
	{
		char *t, *s = NULL;
		int rs = IDN2_ICONV_FAIL;
		size_t len;

		if ((t = strrchr(out, '@')))
			++t;
		else
			t = out;

		len = (size_t) (t - out);

		if (len && (rs = idn2_to_unicode_lzlz(t, &s, IDN2_USE_STD3_ASCII_RULES)) == IDN2_OK) {
			char *q = my_strndup(out, len); /* local-part@ */

			free(out);
			q = append_to_string(q, s); /* domain-part */
			FreeIfNeeded(s);
			return q;
		}
#		ifdef DEBUG
		else {
			if (debug & DEBUG_MISC)
				wait_message(2, "idn2_to_unicode_lzlz(%s): %s", t, idn2_strerror(rs));
		}
#		endif /* DEBUG */
		/* IDNA 2008 failed, try again with IDNA 2003 if available */
		FreeIfNeeded(s);
		free(out);
		out = my_strdup(in);
	}
#	endif /* HAVE_LIBIDN2 */

#	if defined(HAVE_LIBIDNKIT) && defined(HAVE_IDN_DECODENAME)
	{
		idn_result_t res;
		char *q, *r;

		if ((q = strrchr(out, '@'))) {
			++q;
			if ((r = strrchr(in, '@')))
				++r;
			else { /* just to make static analyzer happy */
				r = in;
				q = out;
			}
		} else {
			r = in;
			q = out;
		}
		if ((res = idn_decodename(IDN_DECODE_LOOKUP, r, q, out + strlen(out) - q + 1)) == idn_success)
			return out;
		else { /* IDNA 2008 failed, try again with IDNA 2003 if available */
			free(out);
			out = my_strdup(in);
		}
#		ifdef DEBUG
		if (debug & DEBUG_MISC)
			wait_message(2, "idn_decodename(%s): %s", r, idn_result_tostring(res));
#		else
		(void) res;
#		endif /* DEBUG */
	}
#	endif /* HAVE_LIBIDNKIT && HAVE_IDN_DECODENAME */

	/* IDNA 2003 */
#	ifdef HAVE_LIBICUUC
	{
		UChar *src;
		UChar dest[1024];
		UErrorCode err = U_ZERO_ERROR;
		char *s;
#		ifdef HAVE_LIBICUUC_46_API
		UIDNA *uts46;
		UIDNAInfo info = UIDNA_INFO_INITIALIZER;
#		endif /* HAVE_LIBICUUC_46_API */

		if ((s = strrchr(out, '@')))
			++s;
		else
			s = out;

		src = char2UChar(s);
#		ifndef HAVE_LIBICUUC_46_API
		uidna_IDNToUnicode(src, -1, dest, 1023, UIDNA_USE_STD3_RULES, NULL, &err);
#		else
		uts46 = uidna_openUTS46(UIDNA_USE_STD3_RULES, &err);
		uidna_nameToUnicode(uts46, src, u_strlen(src), dest, 1023, &info, &err);
		uidna_close(uts46);
#		endif /* !HAVE_LIBICUUC_46_API */
		free(src);
		if (!(U_FAILURE(err))) {
			char *t;

			*s = '\0'; /* cut off domainpart */
			if ((s = UChar2char(dest)) != NULL) { /* convert domainpart */
				t = my_malloc(strlen(out) + strlen(s) + 1);
				sprintf(t, "%s%s", out, s);
				free(s);
				free(out);
				out = t;
			}
		}
	}
#	else
#		if defined(HAVE_LIBIDN) && defined(HAVE_IDNA_TO_UNICODE_LZLZ)
	if (stringprep_check_version("0.3.0")) {
		char *t, *s = NULL;
		int rs;

		if ((t = strrchr(out, '@')))
			++t;
		else
			t = out;

#			ifdef HAVE_IDNA_USE_STD3_ASCII_RULES
		if ((rs = idna_to_unicode_lzlz(t, &s, IDNA_USE_STD3_ASCII_RULES)) == IDNA_SUCCESS)
#			else
		if ((rs = idna_to_unicode_lzlz(t, &s, 0)) == IDNA_SUCCESS)
#			endif /* HAVE_IDNA_USE_STD3_ASCII_RULES */
			strcpy(t, s);
#			ifdef DEBUG
		else {
			if (debug & DEBUG_MISC) {
#				ifdef HAVE_IDNA_STRERROR
				wait_message(2, "idna_to_unicode_lzlz(%s): %s", t, idna_strerror(rs));
#				else
				wait_message(2, "idna_to_unicode_lzlz(%s): %d", t, rs);
#				endif /* HAVE_IDNA_STRERROR */
			}
		}
#			endif /* DEBUG */
		FreeIfNeeded(s);
	}
#		endif /* HAVE_LIBIDN && HAVE_IDNA_TO_UNICODE_LZLZ */
#	endif /* HAVE_LIBICUUC */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	return out;
}


/*
 * Terminates "from" at first comma that separates mailboxes
 *
 * Mailbox list:
 *   From: alice@example.com (Alice A), bob@example.com (Bob B)
 *     ->: Alice A <alice@example.com>
 *   From: Alice A <alice@example.com>, Bob B <bob@example.com>
 *     ->: Alice A <alice@example.com>
 *   From: "a(,)lice"@example.com (Alice A), bob@example.com (Bob B)
 *     ->: Alice A <"a(,)lice"@example.com>
 * No mailbox list:
 *   From: "Alice A., alice" <alice@example.com>
 *   From: "Alice \" <>, alice" <"ali \" , -ce"@example.com>
 * Broken, no mailbox list:
 *   From: "Alice \" <>, alice" <"ali " , ce@example.com>
 */
char *
split_mailbox_list(
	char *from)
{
	char *ptr = from;
	t_bool at_seen, in_quoted_str, in_quoted_pair, in_addr, in_comment;

	if (!ptr)
		return NULL;

	at_seen = in_quoted_str = in_quoted_pair = in_addr = in_comment = FALSE;

	while (*ptr) {
		switch (*ptr) {
			case '\\':
				in_quoted_pair = bool_not(in_quoted_pair);
				break;

			case '"':
				if (!in_comment && !in_quoted_pair)
					in_quoted_str = bool_not(in_quoted_str);
				else
					in_quoted_pair = FALSE;
				break;

			case '<':
				if (!in_quoted_pair && !in_comment && !in_quoted_str)
					in_addr = TRUE;
				else
					in_quoted_pair = FALSE;
				break;

			case '>':
				if (!in_quoted_pair && !in_comment && !in_quoted_str)
					in_addr = FALSE;
				else
					in_quoted_pair = FALSE;
				break;

			case '(':
				if (!in_addr && !in_quoted_pair && !in_quoted_str)
					in_comment = TRUE;
				else
					in_quoted_pair = FALSE;
				break;

			case ')':
				if (!in_addr && !in_quoted_pair && !in_quoted_str)
					in_comment = FALSE;
				else
					in_quoted_pair = FALSE;
				break;

			case '@':
				if (!in_quoted_pair && !in_comment && !in_quoted_str)
					at_seen = TRUE;
				else
					in_quoted_pair = FALSE;
				break;

			case ',':
				if (at_seen && !(in_quoted_pair || in_quoted_str || in_addr || in_comment)) {
					*ptr = '\0';
					return *++ptr ? ptr : NULL;
				}
				break;

			default:
				in_quoted_pair = FALSE;
				break;
		}
		++ptr;
	}
	return NULL;
}


int
tin_version_info(
	FILE *fp,
	int verb)
{
	int wlines = 0;	/* written lines */
#ifndef REPRODUCIBLE_BUILD
#	ifdef HAVE_LIB_PCRE2
	char *pcre2_version = NULL;
	int pcre2_version_length;
#	endif /* HAVE_LIB_PCRE2 */
#	ifdef HAVE_LIBICUUC
	UVersionInfo version;
	char buf[U_MAX_VERSION_STRING_LENGTH + 1];
#	endif /* HAVE_LIBICUUC */
#else
	(void) verb;
#endif /* REPRODUCIBLE_BUILD */
	/*
	 * TODO: complete list and do some useful grouping;
	 *       split into < 500byte strings;
	 *       show (some info) only in -vV case?
	 */
	static const char *tin_character =
#ifdef DEBUG
			"+DEBUG"
#else
			"-DEBUG"
#endif /* DEBUG */
#ifdef NNTP_ONLY
#	ifdef NNTPS_ABLE
			" +NNTP(S)_ONLY"
#	else
			" +NNTP_ONLY"
#	endif /* NNTPS_ABLE */
#else
#	ifdef NNTP_ABLE
#		ifdef NNTPS_ABLE
			" +NNTP(S)_ABLE"
#		else
			" +NNTP_ABLE -NNTPS_ABLE"
#		endif /* NNTPS_ABLE */
#	else
			" -NNTP_ABLE"
#	endif /* NNTP_ABLE */
#endif /* NNTP_ONLY */
#ifdef NO_POSTING
			" +NO_POSTING"
#else
			" -NO_POSTING"
#endif /* NO_POSTING */
			"\n\t"
#ifdef USE_ZLIB
			"+USE_ZLIB"
#else
			"-USE_ZLIB"
#endif /* USE_ZLIB */
#ifdef ENABLE_IPV6
			" +ENABLE_IPV6"
#else
			" -ENABLE_IPV6"
#endif /* ENABLE_IPV6 */
#ifdef HAVE_COREFILE
			" +HAVE_COREFILE"
#else
			" -HAVE_COREFILE"
#endif /* HAVE_COREFILE */
#ifdef SOCKS
#	ifdef USE_SOCKS5
		"  +USE_SOCKS5"
#	else
		"  +SOCKS"
#	endif /* USE_SOCKS5 */
#endif /* SOCKS */
#ifdef HAVE_FASCIST_NEWSADMIN
			" +HAVE_FASCIST_NEWSADMIN"
#else
			" -HAVE_FASCIST_NEWSADMIN"
#endif /* HAVE_FASCIST_NEWSADMIN */
			"\n\t"
#ifdef NO_SHELL_ESCAPE
			"+NO_SHELL_ESCAPE"
#else
			"-NO_SHELL_ESCAPE"
#endif /* NO_SHELL_ESCAPE */
#ifdef DISABLE_PRINTING
			" +DISABLE_PRINTING"
#else
			" -DISABLE_PRINTING"
#endif /* DISABLE_PRINTING */
#ifdef DONT_HAVE_PIPING
			" +DONT_HAVE_PIPING"
#else
			" -DONT_HAVE_PIPING"
#endif /* DONT_HAVE_PIPING */
#ifdef NO_ETIQUETTE
			" +NO_ETIQUETTE"
#else
			" -NO_ETIQUETTE"
#endif /* NO_ETIQUETTE */
			"\n\t"
#ifdef HAVE_LONG_FILE_NAMES
			"+HAVE_LONG_FILE_NAMES"
#else
			"-HAVE_LONG_FILE_NAMES"
#endif /* HAVE_LONG_FILE_NAMES */
#ifdef APPEND_PID
			" +APPEND_PID"
#else
			" -APPEND_PID"
#endif /* APPEND_PID */
#ifdef HAVE_MH_MAIL_HANDLING
			" +HAVE_MH_MAIL_HANDLING"
#else
			" -HAVE_MH_MAIL_HANDLING"
#endif /* HAVE_MH_MAIL_HANDLING */
#ifdef HAVE_COLOR
			" +HAVE_COLOR"
#else
			" -HAVE_COLOR"
#endif /* HAVE_COLOR */
			"\n\t"
#ifdef HAVE_ISPELL
			"+HAVE_ISPELL"
#else
			"-HAVE_ISPELL"
#endif /* HAVE_ISPELL */
#ifdef HAVE_METAMAIL
			" +HAVE_METAMAIL"
#else
			" -HAVE_METAMAIL"
#endif /* HAVE_METAMAIL */
#ifdef HAVE_PGP
			" +HAVE_PGP"
#else
			" -HAVE_PGP"
#endif /* HAVE_PGP */
#ifdef HAVE_PGPK
			" +HAVE_PGPK"
#else
			" -HAVE_PGPK"
#endif /* HAVE_PGPK */
#ifdef HAVE_GPG
			" +HAVE_GPG"
#else
			" -HAVE_GPG"
#endif /* HAVE_GPG */
			"\n\t"
#ifdef MIME_BREAK_LONG_LINES
			"+MIME_BREAK_LONG_LINES"
#else
			"-MIME_BREAK_LONG_LINES"
#endif /* MIME_BREAK_LONG_LINES */
#ifdef MIME_STRICT_CHARSET
			" +MIME_STRICT_CHARSET"
#else
			" -MIME_STRICT_CHARSET"
#endif /* MIME_STRICT_CHARSET */
			"\n\t"
#ifdef CHARSET_CONVERSION
			"+CHARSET_CONVERSION_{"
#	ifdef CHARSET_CONVERSION_ICONV
			"ICONV"
#		ifdef CHARSET_CONVERSION_UCNV
			","
#		endif /* CHARSET_CONVERSION_UCNV */
#	endif /* CHARSET_CONVERSION_ICONV */
#	ifdef CHARSET_CONVERSION_UCNV
			"UCNV"
#	endif /* CHARSET_CONVERSION_UCNV */
			"}"
#else
			"-CHARSET_CONVERSION"
#endif /* CHARSET_CONVERSION */
#ifdef MULTIBYTE_ABLE
			" +MULTIBYTE_ABLE"
#else
			" -MULTIBYTE_ABLE"
#endif /* MULTIBYTE_ABLE */
#ifdef NO_LOCALE
			" +NO_LOCALE"
#else
			" -NO_LOCALE"
#endif /* NO_LOCALE */
			"\n\t"
#ifdef USE_ICU_UCSDET
			"+USE_ICU_UCSDET"
#else
			"-USE_ICU_UCSDET"
#endif /* USE_ICU_UCSDET */
#ifdef USE_LONG_ARTICLE_NUMBERS
			" +USE_LONG_ARTICLE_NUMBERS"
#else
			" -USE_LONG_ARTICLE_NUMBERS"
#endif /* USE_LONG_ARTICLE_NUMBERS */
			"\n\t"
#ifdef USE_CANLOCK
			"+USE_CANLOCK"
#else
			"-USE_CANLOCK"
#endif /* USE_CANLOCK */
#ifdef EVIL_INSIDE
			" +EVIL_INSIDE"
#else
			" -EVIL_INSIDE"
#endif /* EVIL_INSIDE */
#ifdef FORGERY
			" +FORGERY"
#else
			" -FORGERY"
#endif /* FORGERY */
#ifdef TINC_DNS
			" +TINC_DNS"
#else
			" -TINC_DNS"
#endif /* TINC_DNS */
#ifdef ENFORCE_RFC1034
			" +ENFORCE_RFC1034"
#else
			" -ENFORCE_RFC1034"
#endif /* ENFORCE_RFC1034 */
			"\n\t"
#ifdef REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
			"+REQUIRE_BRACKETS_IN_DOMAIN_LITERAL"
#else
			"-REQUIRE_BRACKETS_IN_DOMAIN_LITERAL"
#endif /* REQUIRE_BRACKETS_IN_DOMAIN_LITERAL */
#ifdef ALLOW_FWS_IN_NEWSGROUPLIST
			" +ALLOW_FWS_IN_NEWSGROUPLIST"
#else
			" -ALLOW_FWS_IN_NEWSGROUPLIST"
#endif /* ALLOW_FWS_IN_NEWSGROUPLIST */
	;

	fprintf(fp, _(txt_tin_version), PRODUCT, VERSION, RELEASEDATE, RELEASENAME);
#if defined(__DATE__) && defined(__TIME__) && !defined(REPRODUCIBLE_BUILD)
	fprintf(fp, " %s %s", __DATE__, __TIME__);
#endif /* __DATE__ && __TIME__ && !REPRODUCIBLE_BUILD */
	fprintf(fp, "\n");
	++wlines;

#ifdef SYSTEM_NAME
	fprintf(fp, "Platform:\n");
	fprintf(fp, "\tOS-Name  = \"%s\"\n", SYSTEM_NAME);
	wlines += 2;
#endif /* SYSTEM_NAME */

#ifdef TIN_CC
	fprintf(fp, "Compiler:\n");
	fprintf(fp, "\tCC       = \"%s\"\n", TIN_CC);
	wlines += 2;
#	if defined(TIN_CFLAGS) && !defined(REPRODUCIBLE_BUILD)
	if (verb) {
		fprintf(fp, "\tCFLAGS   = \"%s\"\n", TIN_CFLAGS);
		++wlines;
	}
#	endif /* TIN_CFLAGS && !REPRODUCIBLE_BUILD */
#	ifdef TIN_CPP
		fprintf(fp, "\tCPP      = \"%s\"\n", TIN_CPP);
		++wlines;
#	endif /* TIN_CPP */
#	if defined(TIN_CFLAGS) && !defined(REPRODUCIBLE_BUILD)
	if (verb) {
		fprintf(fp, "\tCPPFLAGS = \"%s\"\n", TIN_CPPFLAGS);
		++wlines;
	}
#	endif /* TIN_CPPFLAGS && !REPRODUCIBLE_BUILD */
#endif /* TIN_CC */

#ifdef TIN_LD
	fprintf(fp, "Linker and Libraries:\n");
	fprintf(fp, "\tLD       = \"%s\"\n", TIN_LD);
	wlines += 2;
#	if defined(TIN_LDFLAGS) && !defined(REPRODUCIBLE_BUILD)
	if (verb) {
		fprintf(fp, "\tLDFLAGS  = \"%s\"\n", TIN_LDFLAGS);
		++wlines;
	}
#	endif /* TIN_LDFLAGS && !REPRODUCIBLE_BUILD */
#	ifdef TIN_LIBS
		fprintf(fp, "\tLIBS     = \"%s\"\n", TIN_LIBS);
		++wlines;
#	endif /* TIN_LIBS */
#endif /* TIN_LD */

#ifndef REPRODUCIBLE_BUILD
	if (verb) {
#	ifdef NNTPS_ABLE
#		ifdef HAVE_LIB_LIBTLS
		fprintf(fp, "\tTLS      = \"LibreSSL %d\"\n", TLS_API);
#		else
#			ifdef HAVE_LIB_OPENSSL
		fprintf(fp, "\tTLS      = \"%s\"\n", OpenSSL_version(OPENSSL_VERSION));
#			else
#				ifdef HAVE_LIB_GNUTLS
		fprintf(fp, "\tTLS      = \"GnuTLS %s\"\n", gnutls_check_version(NULL));
#				endif /* HAVE_LIB_GNUTLS */
#			endif /* HAVE_LIB_OPENSSL */
#		endif /* HAVE_LIB_LIBTLS */
		++wlines;
#	endif /* NNTPS_ABLE */

#	ifdef HAVE_LIB_PCRE2
		pcre2_version_length = pcre2_config_8(PCRE2_CONFIG_VERSION, NULL);
		if (pcre2_version_length > 0) {
			pcre2_version = my_malloc(pcre2_version_length);
			(void) pcre2_config_8(PCRE2_CONFIG_VERSION, pcre2_version);
		}
		fprintf(fp, "\tPCRE2    = \"%s\"\n", pcre2_version ? pcre2_version : txt_unknown);
		FreeIfNeeded(pcre2_version);
#	else
		fprintf(fp, "\tPCRE     = \"%s\"\n", pcre_version());
#	endif /* HAVE_LIB_PCRE2 */
		++wlines;

#	ifdef USE_CURSES
#		ifdef HAVE_CURSES_VERSION
		fprintf(fp, "\tCURSES   = \"%s\"\n", curses_version());
		++wlines;
#		else
#			if defined(NCURSES_VERSION_MAJOR) && NCURSES_VERSION_MAJOR < 5
		fprintf(fp, "\tCURSES   = \"ncurses %s\"\n", NCURSES_VERSION);
		++wlines;
#			endif /* NCURSES_VERSION_MAJOR && NCURSES_VERSION_MAJOR < 5*/
#			if defined(PDCURSES) && defined(PDC_VERDOT)
		fprintf(fp, "\tCURSES   = \"PDCurses %s\"\n", PDC_VERDOT);
		++wlines;
#			endif /* PDCURSES && PDC_VERDOT */
#		endif /* HAVE_CURSES_VERSION */
#	endif /* USE_CURSES */

#	ifdef USE_GSASL
		fprintf(fp, "\tGNU SASL = \"%s\"\n", gsasl_check_version(NULL));
		++wlines;
#	endif /* USE_GSASL */

#	ifdef HAVE_LIBICUUC
		u_getVersion(version);
		u_versionToString(version, buf);
		fprintf(fp, "\tICU      = \"%s", buf);
#		ifdef USE_ICU_UCSDET
		u_getUnicodeVersion(version);
		u_versionToString(version, buf);
		fprintf(fp, " (Unicode %s)", buf);
#		endif /* USE_ICU_UCSDET */
		fprintf(fp, "\"\n");
		++wlines;
#	endif /* HAVE_LIBICUUC */

#	if defined(HAVE_LIBUNISTRING) && defined(HAVE_UNISTRING_VERSION_H) && (_LIBUNISTRING_VERSION > 0x000009)
		fprintf(fp, "\tUNISTRING= \"%d.%d.%d\"\n", (_LIBUNISTRING_VERSION & 0xff0000) >> 16, (_LIBUNISTRING_VERSION & 0x00ff00) >> 8, _LIBUNISTRING_VERSION & 0x0000ff);
		++wlines;
#	endif /* HAVE_LIBUNISTRING && HAVE_UNISTRING_VERSION_H && _LIBUNISTRING_VERSION > 0x000009 */

#	if defined(HAVE_LIBIDNKIT) && defined (HAVE_IDN_VERSION_H)
		fprintf(fp, "\tIDNKIT   = \"%s\"\n", idn_version_libidn());
		++wlines;
#	endif /* HAVE_LIBIDNKIT && HAVE_IDN_VERSION_H */

#	if defined(HAVE_LIBIDN2) && defined(IDN2_VERSION)
		fprintf(fp, "\tIDN2     = \"%s\"\n", idn2_check_version(NULL));
		++wlines;
#	endif /* HAVE_LIBIDN2 && IDN2_VERSION */

#	if defined(HAVE_LIBIDN) && defined(HAVE_STRINGPREP_H)
		fprintf(fp, "\tIDN      = \"%s\"\n", stringprep_check_version("0.3.0"));
		++wlines;
#	endif /* HAVE_LIBIDN && HAVE_STRINGPREP_H */

#	ifdef USE_ZLIB
#		if defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1020
		fprintf(fp, "\tZLIB     = \"%s\"\n", zlibVersion());
#		else
		fprintf(fp, "\tZLIB     = \"%s\"\n", ZLIB_VERSION);
#endif /* ZLIB_VERNUM && ZLIB_VERNUM >= 0x1020 */
		++wlines;
#	endif /* USE_ZLIB */

#	ifdef HAVE_LIBURIPARSER
		fprintf(fp, "\tURIPARSER= \"%s\"\n", URI_VER_ANSI);
		++wlines;
#	else
#		ifdef HAVE_LIBCURL
		fprintf(fp, "\tCURL     = \"%s\"\n", LIBCURL_VERSION);
		++wlines;
#		endif /* HAVE_LIBCURL */
#	endif /* HAVE_LIBURIPARSER */

#	if defined(USE_CANLOCK) && defined(CL_API_MAJOR) && defined(CL_API_MINOR)
		fprintf(fp, "\tCANLOCK  = \"%d.%d\"\n", CL_API_MAJOR, CL_API_MINOR);
		++wlines;
#	endif /* USE_CANLOCK && CL_API_MAJOR && CL_API_MINOR */
	}
#endif /* REPRODUCIBLE_BUILD */

	fprintf(fp, "Characteristics:\n\t%s\n", tin_character);
	wlines += 11;
	fflush(fp);
	return wlines;
}


void
draw_mark_selected(
	int i)
{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	int j;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	MoveCursor(INDEX2LNUM(i), mark_offset);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	for (j = art_mark_width - wcwidth(tinrc.art_marked_selected); j > 0; --j)
		my_fputc(' ', stdout);
	StartInverse();	/* ToggleInverse() doesn't work correct with ncurses4.x */
	my_fputwc((wint_t) tinrc.art_marked_selected, stdout);
#else
	StartInverse();	/* ToggleInverse() doesn't work correct with ncurses4.x */
	my_fputc(tinrc.art_marked_selected, stdout);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	EndInverse();	/* ToggleInverse() doesn't work correct with ncurses4.x */
}


int
tin_gettime(
	struct t_tintime *tt)
{
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
	static struct timespec cgt;
#endif /* HAVE_CLOCK_GETTIME && CLOCK_REALTIME */
#ifdef HAVE_GETTIMEOFDAY
	static struct timeval gt;
#endif /* HAVE_GETTIMEOFDAY */

#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
	if (clock_gettime(CLOCK_REALTIME, &cgt) == 0) {
		tt->tv_sec = cgt.tv_sec;
		tt->tv_nsec = cgt.tv_nsec;
		return 0;
	}
#endif /* HAVE_CLOCK_GETTIME && CLOCK_REALTIME */
#ifdef HAVE_GETTIMEOFDAY
	if (gettimeofday(&gt, NULL) == 0) {
		tt->tv_sec = gt.tv_sec;
		tt->tv_nsec = gt.tv_usec * 1000;
		return 0;
	}
#endif /* HAVE_GETTIMEOFDAY */
	tt->tv_sec = 0;
	tt->tv_nsec = 0;
	return -1;
}


#if 0
/*
 * Stat a mail/news article to see if it still exists
 */
static t_bool
stat_article(
	t_artnum art,
	const char *group_path)
{
	struct t_group currgrp;

	currgrp = CURR_GROUP;

#	ifdef NNTP_ABLE
	if (read_news_via_nntp && currgrp.type == GROUP_TYPE_NEWS) {
		char buf[NNTP_STRLEN];

		snprintf(buf, sizeof(buf), "STAT %"T_ARTNUM_PFMT, art);
		return (nntp_command(buf, OK_NOTEXT, NULL, 0) != NULL);
	} else
#	endif /* NNTP_ABLE */
	{
		char filename[PATH_LEN];
		struct stat sb;

		joinpath(filename, sizeof(filename), currgrp.spooldir, group_path);
		snprintf(&filename[strlen(filename)], sizeof(filename), "/%"T_ARTNUM_PFMT, art);

		return (stat(filename, &sb) != -1);
	}
}
#endif /* 0 */


/*
 * show connection details
 */
void
show_connection_page(
	void)
{
	FILE *fp;

	if (!(fp = my_tmpfile()))
		return;

	make_connection_page(fp);
	info_pager(fp, _(txt_connection_info), FALSE); /* all other pagers do wrap */
	fclose(fp);
	info_pager(NULL, NULL, TRUE); /* free mem */
}


static void
make_connection_page(
	FILE *fp)
{
	t_bool add_slash;

	if (!read_news_via_nntp)
		fprintf(fp, "%s", _(txt_conninfo_local_spool));
	else {
		if (read_saved_news)
			fprintf(fp, "%s", _(txt_conninfo_saved_news));
#if defined(NNTP_ABLE)
#	if defined(NNTPS_ABLE)
		else {
			if (use_nntps) {
				fprintf(fp, insecure_nntps ? _(txt_conninfo_untrusted) : _(txt_conninfo_trusted), can_post ? _(txt_conninfo_rw) : _(txt_conninfo_ro));
#		ifdef HAVE_LIB_LIBTLS
				fprintf(fp, txt_conninfo_libressl, TLS_API);
#		else
#			ifdef HAVE_LIB_OPENSSL
				fprintf(fp, txt_conninfo_openssl, OpenSSL_version(OPENSSL_VERSION));
#			else
#				ifdef HAVE_LIB_GNUTLS
				fprintf(fp, txt_conninfo_gnutls, gnutls_check_version(NULL));
#				endif /* HAVE_LIB_GNUTLS */
#			endif /* HAVE_LIB_OPENSSL */
#		endif /* HAVE_LIB_LIBTLS */
			} else
#	endif /* NNTPS_ABLE */
			{
				fprintf(fp, _(txt_conninfo_nntp), can_post ? _(txt_conninfo_rw) : _(txt_conninfo_ro));
				if (!can_post && !*domain_name)
					fprintf(fp, "%s\n", _(txt_error_no_domain_name));
			}

			(void) nntp_conninfo(fp);
#	if defined(NNTPS_ABLE)
		}
#	endif /* NNTPS_ABLE */
#endif /* NNTP_ABLE */
	}

	/*
	 * TODO: do we want (#ifdef HAVE_REALPATH) to resolve symlinks
	 *       (or show both like "ls -ld") in the display?
	 */
#ifndef NNTP_ONLY
	if (!read_news_via_nntp && !read_saved_news) {
		fprintf(fp, "%s", txt_conninfo_spool_config);
		add_slash = (spooldir[strlen(spooldir) - 1] != '/');
		fprintf(fp, txt_conninfo_spooldir, spooldir, add_slash ? "/" : "");
		add_slash = (novrootdir[strlen(novrootdir) - 1] != '/');
		fprintf(fp, txt_conninfo_novrootdir, novrootdir, add_slash ? "/" : "");
		fprintf(fp, txt_conninfo_overview_file, novfilename); /* TODO: shows wrong name if using inn >= 2.3.0 nov-style */
		fprintf(fp, txt_conninfo_overview_fmt, overviewfmt_file);
		fprintf(fp, txt_conninfo_newsgroups_file, newsgroups_file);
		fprintf(fp, txt_conninfo_active_file, news_active_file);
		fprintf(fp, txt_conninfo_active_times_file, active_times_file);
		fprintf(fp, txt_conninfo_subscriptions_file, subscriptions_file);
	}
#endif /* !NNTP_ONLY */

	fprintf(fp, "%s", _(txt_conninfo_conf_files));
	/* TODO: hide if empty? */
	fprintf(fp, "tin.defaults       : %s\n", global_defaults_file);

	/* TODO: getext for "local"/"global" translation? */
	fprintf(fp, "attributes (local) : %s\n", local_attributes_file);
	if (*global_attributes_file)
		fprintf(fp, "attributes (global): %s\n", global_attributes_file);

	fprintf(fp, "tinrc (local)      : %s\n", local_config_file);
	if (*global_config_file)
		fprintf(fp, "tinrc (global)     : %s\n", global_config_file);

	fprintf(fp, "serverrc           : %s\n", serverrc_file);

	fprintf(fp, "filter             : %s\n", filter_file);

	if (*keymap_file)
		fprintf(fp, "keymap             : %s\n", keymap_file);

	/* non conf files/dirs */
	fprintf(fp, "\n");
	fprintf(fp, ".newsrc            : %s\n", newsrc);

	/*
	 * "-$NNTPSERVER" is appended in find_nov_file()
	 * which may not haven been called yet (e.g. not
	 * entered a group till now), so index_newsdir may
	 * lack the suffix, fix it for displaying only
	 */
	if (serverrc.cache_overview_files) {
		char *ns = strcasestr(index_newsdir, nntp_server);
		char *idx_nd = my_strdup(index_newsdir);

		if (!ns || strlen(ns) != strlen(nntp_server)) {
			char *srv = my_strdup(nntp_server);

			idx_nd = append_to_string(idx_nd, "-");
			idx_nd = append_to_string(idx_nd, srv);
			free(srv);
		}
		add_slash = (idx_nd[strlen(idx_nd) - 1] != '/');

#ifdef USE_ZLIB
		fprintf(fp, "local overviews %s: %s%s\n", serverrc.compress_overview_files ? "[c]" : "   ", idx_nd, add_slash ? "/" : "");
#else
		fprintf(fp, "local overviews    : %s%s\n", idx_nd, add_slash ? "/" : "");
#endif /* USE_ZLIB */
		free(idx_nd);
    }

#ifdef DEBUG
	{	/* till we have a serverrc-menu */
		const char *e = get_val("TINRC", NULL);
		size_t l = strlen(BlankIfNull(e)) + strlen(BlankIfNull(serverrc.add_cmd_line_opts));

		if (l) {
			fprintf(fp, "\n$TINRC + serverrc  : ");
			if (e && *e && *serverrc.add_cmd_line_opts)
				fprintf(fp, "%s %s", e, serverrc.add_cmd_line_opts);
			else
				fprintf(fp, "%s", (e && *e) ? e : serverrc.add_cmd_line_opts);
			fprintf(fp, "\n");
		}
	}
#endif /* DEBUG */
}


/*
 * pseudo random number generator
 * range 0 to INT_MAX (=RAND_MAX)
 * seed with srndm(void)
 */
int
rndm(
	void)
{
#ifdef HAVE_ARC4RANDOM_UNIFORM
	return (int) (arc4random_uniform(INT_MAX));
#else
#	ifdef HAVE_LRAND48
	return (int) (lrand48() & INT_MAX);
#	else
#		ifdef HAVE_RANDOM
	return (int) (random() & INT_MAX);
#		else
	return rand();
#		endif /* HAVE_RANDOM */
#	endif /* HAVE_LRAND48 */
#endif /* HAVE_ARC4RANDOM_UNIFORM */
}


/* seed rndm() */
void
srndm(
	void)
{
#ifdef HAVE_ARC4RANDOM_UNIFORM
	(void) 0;
#else
	time_t t;

	if ((t = time(NULL)) == (time_t) -1)
		t = (time_t) process_id;
	else {
		if (t >= 1041379200) /* 2003-01-01 00:00:00 GMT */
			t -= 1041379200;
	}
#	ifdef HAVE_LRAND48
	srand48(t);
#	else
#		ifdef HAVE_RANDOM
	srandom((unsigned int) t);
#		else
	srand((unsigned int) t);
#		endif /* HAVE_RANDOM */
#	endif /* HAVE_LRAND48 */
#endif /* HAVE_ARC4RANDOM_UNIFORM */
}


/*
 * opens pathname with mode if it is S_IFREG or S_IFLNK
 * if mode is "r[b]" (read only) its size needs to be > 0L
 * returns FILE* on success or NULL on failure
 * if NULL is returned and errno is 0, it either had a zero size
 * or was neither S_IFDIR, S_IFLNK or S_IFREG.
 *
 * TODO: do something on EISDIR
 */
FILE *
tin_fopen(
	const char *pathname,
	const char *mode)
{
	FILE *fp;
	int serrno = 0;
	struct stat st;

	if (!*pathname || !*mode) {
		serrno = EINVAL;
		goto out;
	}

	if ((fp = fopen(pathname, mode)) != NULL) {
		if (fstat(fileno(fp), &st) != -1) {
			if (S_ISDIR(st.st_mode))
				serrno = errno = EISDIR;
			else {
				if (S_ISREG(st.st_mode)) {
					if (mode[0] == 'r' && (mode[1] == '\0' || (mode[1] == 'b' && mode[2] == '\0')) && st.st_size <= 0L) {
#ifdef DEBUG
						if (debug & DEBUG_MISC)
							error_message(2, "Skipping empty file: %s", pathname);
#endif /* DEBUG */
					} else
						return fp;
				} else
					serrno = errno;
			}
		} else
			serrno = errno;

		if (fclose(fp) != 0) {
			if (!serrno)
				serrno = errno;
		}
	} else
		serrno = errno;

out:
	errno = serrno;
	switch (errno) {
		case 0:
			break;

		case ENOENT: /* a missing file is usually ok */
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				perror_message(_(txt_cannot_open), pathname);
#endif /* DEBUG */
			break;

		case EISDIR:
			/*
			 * TODO: issue an error message here
			 *       this requires some rework elsewhere in
			 *       the code, i.e. "-f /tmp/isdir" would
			 *       lead to multiple messages, most of them
			 *       may confuse the user. at least put the
			 *       _(txt_reading_*) message before the
			 *       tin_fopen() call.
			 */
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				perror_message("Error: tin_fopen(%s)", pathname);
#endif /* DEBUG */
			break;

		default:
			perror_message(_(txt_cannot_open), BlankIfNull(pathname));
			break;
	}
	return NULL;
}


#ifdef CHARSET_CONVERSION
#	ifdef CHARSET_CONVERSION_UCNV
#		if defined(DEBUG) && defined(DEBUG_UCNV)
#			include <unicode/errorcode.h>
#		endif /* DEBUG && DEBUG_UCNV */
/*
 * converts **input of max-length *input_size *from charset *to charset
 * may reallocate **input and updateinput_size accordingly
 * returns FALSE on error and TRUE otherwise
 */
static t_bool
tin_ucnv_buffer_to_local(
	char **line,
	size_t *max_line_len,
	const char *from_charset,
	const char *to_charset)
{
	UErrorCode err = U_ZERO_ERROR;
	UConverter *to_conv, *from_conv;
	UChar *unicode_buffer;
	char *output_buffer;
	char *p;
	const char subc[] = { 0x3f }; /* hardcoded '?' replacement char */
	int quest = 0; /* input/output number of '?' to detect substitutions */
	size_t unicode_length, output_length, ilen;

	from_conv = ucnv_open(from_charset, &err);
	if (U_FAILURE(err)) {
#if defined(DEBUG) && defined(DEBUG_UCNV)
		wait_message(2, "UCNV: %s (%s:%d) %s [-> %s]", u_errorName(err), __FILE__, __LINE__, from_charset, to_charset);
#endif /* DEBUG && DEBUG_UCNV */
		return FALSE;
	}

	to_conv = ucnv_open(to_charset, &err);
	if (U_FAILURE(err)) {
#if defined(DEBUG) && defined(DEBUG_UCNV)
		wait_message(2, "UCNV: %s (%s:%d) [%s ->] %s", u_errorName(err), __FILE__, __LINE__, from_charset, to_charset);
#endif /* DEBUG && DEBUG_UCNV */
		ucnv_close(from_conv);
		return FALSE;
	}

	ucnv_setSubstChars(from_conv, subc, 1, &err); /* switch to ucnv_setSubstString()? */

	ilen = *max_line_len;
	unicode_buffer = my_calloc(ilen, sizeof(UChar));

	p = *line;
	while ((p = strchr(p, '?')) != NULL) { /* TODO: fixme, instead of counting output questionmarks use ucnv_setToUCallBack(...,UCNV_TO_U_CALLBACK_SUBSTITUTE,...) */
		--quest;
		++p;
	}

	do {
		ucnv_reset(from_conv);
		err = U_ZERO_ERROR;
		unicode_length = ucnv_toUChars(from_conv, unicode_buffer, ilen, *line, strlen(*line), &err);
		if (err == U_BUFFER_OVERFLOW_ERROR) {
			ilen = unicode_length + 1;
			free(unicode_buffer);
			unicode_buffer = my_calloc(ilen, sizeof(UChar));
		}
	} while (err == U_BUFFER_OVERFLOW_ERROR);

	if (U_FAILURE(err)) {
#if defined(DEBUG) && defined(DEBUG_UCNV)
		wait_message(2, "UCNV: %s (%s:%d) %s [-> UChar]", u_errorName(err), __FILE__, __LINE__, from_charset);
#endif /* DEBUG && DEBUG_UCNV */
		ucnv_close(from_conv);
		ucnv_close(to_conv);
		free(unicode_buffer);
		return FALSE;
	}

	ucnv_setSubstChars(to_conv, subc, 1, &err); /* switch to ucnv_setSubstString()? */

	ilen = *max_line_len;
	output_buffer = my_calloc(ilen, sizeof(UChar));
	do {
		ucnv_reset(to_conv);
		err = U_ZERO_ERROR;
		output_length = ucnv_fromUChars(to_conv, output_buffer, ilen, unicode_buffer, unicode_length, &err);
		if (err == U_BUFFER_OVERFLOW_ERROR) {
			ilen = output_length + 1;
			free(output_buffer);
			output_buffer = my_calloc(ilen, sizeof(UChar));
		}
	} while (err == U_BUFFER_OVERFLOW_ERROR);

	p = output_buffer;
	while ((p = strchr(p, '?')) != NULL) { /* TODO: fixme, instead of counting output questionmarks use ucnv_setFromUCallBack(..,UCNV_FROM_U_CALLBACK_SUBSTITUTE,..) */
		++quest;
		++p;
	}

	if (U_FAILURE(err)) {
#if defined(DEBUG) && defined(DEBUG_UCNV)
		wait_message(2, "UCNV: %s (%s:%d) [UChar ->] %s", u_errorName(err), __FILE__, __LINE__, to_charset);
#endif /* DEBUG && DEBUG_UCNV */
		ucnv_close(from_conv);
		ucnv_close(to_conv);
		free(unicode_buffer);
		free(output_buffer);
		return FALSE;
	}

	/* copy result */
	free(*line);
	*line = my_strdup(output_buffer);
	*max_line_len = strlen(output_buffer);

	/* cleanup */
	ucnv_close(from_conv);
	ucnv_close(to_conv);
	free(unicode_buffer);
	free(output_buffer);
	return (quest == 0); /* was there a substitution? */
}
#	endif /* CHARSET_CONVERSION_UCNV */
#endif /* CHARSET_CONVERSION */


int
sync_close(
	int fd)
{
	int rc;

#ifdef HAVE_FSYNC
#	ifdef EINTR
fsync_again:
#	endif /*EINTR */
	if ((rc = fsync(fd)) < 0) {
		int serrno;

#	ifdef EINTR
		if (errno == EINTR)
			goto fsync_again;
#	endif /* !EINTR */

		serrno = errno;
#	ifdef HAVE_POSIX_CLOSE
		posix_close(fd, 0);
#	else
		close(fd);
#	endif /* HAVE_POSIX_CLOSE */
		errno = serrno;
		return rc;
	}
#endif /* HAVE_FSYNC */

#ifdef EINTR
close_again:
#endif /* EINTR */
	rc = close(fd);
#ifdef EINTR
	if (rc < 0 && errno == EINTR) {
		struct stat st;

		if (fstat(fd, &st) == 0) /* fd still open? */
			goto close_again;
	}
#endif /* EINTR */

	return rc;
}


const char *
logtime(
	void)
{
#if defined(HAVE_CLOCK_GETTIME) || defined(HAVE_GETTIMEOFDAY)
	static struct t_tintime log_time;
	static char out[40];

	if (tin_gettime(&log_time) == 0) {
		if (my_strftime(out, 39, " [%H:%M:%S.", gmtime(&(log_time.tv_sec)))) {
			snprintf(out + 11, sizeof(out) - 11, "%09ld", log_time.tv_nsec); /* strlen(" [hh:mm:ss.") */
			out[17] = '\0'; /* strlen(" [hh:mm:ss.uuuuuu") */
			strcat(out, "] ");
			return out;
		}
	}
#endif /* HAVE_CLOCK_GETTIME || HAVE_GETTIMEOFDAY */
	return " ";
}
