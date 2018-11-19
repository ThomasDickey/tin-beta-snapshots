/*
 *  Project   : tin - a Usenet reader
 *  Module    : misc.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2018-04-04
 *  Notes     :
 *
 * Copyright (c) 1991-2019 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef included_trace_h
#	include "trace.h"
#endif /* !included_trace_h */
#ifndef TIN_POLICY_H
#	include "policy.h"
#endif /* !TIN_POLICY_H */

#if defined(HAVE_IDNA_H) && !defined(_IDNA_H)
#	include <idna.h>
#endif /* HAVE_IDNA_H && !_IDNA_H */
#if defined(HAVE_STRINGPREP_H) && !defined(_STRINGPREP_H)
#	include <stringprep.h>
#endif /* HAVE_STRINGPREP_H && !_STRINGPREP_H */

#if defined(HAVE_IDN_API_H) && !defined(IDN_API_H)
#	include <idn/api.h>
#endif /* HAVE_IDN_API_H && !IDN_API_H */


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
static char *strfpath_cp(char *str, char *tbuf, char *endp);
static int _strfpath(const char *format, char *str, size_t maxsize, struct t_group *group, t_bool expand_all);
static int gnksa_check_domain(char *domain);
static int gnksa_check_domain_literal(char *domain);
static int gnksa_check_localpart(const char *localpart);
static int gnksa_dequote_plainphrase(char *realname, char *decoded, int addrtype);
static int strfeditor(char *editor, int linenum, const char *filename, char *s, size_t maxsize, char *format);
static void write_input_history_file(void);
#ifdef CHARSET_CONVERSION
	static t_bool buffer_to_local(char **line, size_t *max_line_len, const char *network_charset, const char *local_charset);
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

	/* alloc memory for tmp-filename */
	file_tmp = my_malloc(strlen(filename) + 5);

	/* generate tmp-filename */
	sprintf(file_tmp, "%s.tmp", filename);
	return file_tmp;
}


/*
 * append_file instead of rename_file
 * minimum error trapping
 */
void
append_file(
	char *old_filename,
	char *new_filename)
{
	FILE *fp_old, *fp_new;

	if ((fp_old = fopen(old_filename, "r")) == NULL) {
		perror_message(_(txt_cannot_open), old_filename);
		return;
	}
	if ((fp_new = fopen(new_filename, "a")) == NULL) {
		perror_message(_(txt_cannot_open), new_filename);
		fclose(fp_old);
		return;
	}
	copy_fp(fp_old, fp_new);
	fclose(fp_old);
	fclose(fp_new);
}


void
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
#ifdef HAVE_COREFILE
#	ifdef SIGABRT
		sigdisp(SIGABRT, SIG_DFL);
		kill(process_id, SIGABRT);
#	else
#		ifdef SIGILL
			sigdisp(SIGILL, SIG_DFL);
			kill(process_id, SIGILL);
#		else
#			ifdef SIGIOT
				sigdisp(SIGIOT, SIG_DFL);
				kill(process_id, SIGIOT);
#			endif /* SIGIOT */
#		endif /* SIGILL */
#	endif /* SIGABRT */
#endif /* HAVE_COREFILE */

	giveup();
}


/*
 * Quick copying of files
 * Returns FALSE if copy failed. Caller may wish to check for errno == EPIPE.
 */
t_bool
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
			return FALSE;
		}
		TRACE(("copy_fp wrote %d:{%.*s}", sent, (int) sent, buf));
	}
	return TRUE;
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
	int fd;
	mode_t mode = (mode_t) (S_IRUSR|S_IWUSR);
	struct stat statbuf;
	t_bool ret = FALSE;

	if ((fp_in = fopen(filename, "r")) == NULL)	/* a missing sourcefile is not a real bug */
		return TRUE;

	/* don't follow links when writing backup files - do we really want this? */
	unlink(backupname);
	if ((fp_out = fopen(backupname, "w")) == NULL) {
		fclose(fp_in);
		return ret;
	}

	if ((fd = fileno(fp_in)) != -1) {
		if (!fstat(fd, &statbuf))
			mode = statbuf.st_mode;
	}

	ret = copy_fp(fp_in, fp_out);

	if ((fd = fileno(fp_out)) != -1)
		fchmod(fd, mode);

	fclose(fp_out);
	fclose(fp_in);
	return ret;
}


/*
 * copy the body of articles with given file pointers,
 * prefix (= quote_chars), initials of the articles author
 * with_sig is set if the signature should be quoted
 *
 * TODO: rewrite from scratch, the code is awful
 */
void
copy_body(
	FILE *fp_ip,
	FILE *fp_op,
	char *prefix,
	char *initl,
	t_bool raw_data)
{
	char buf[8192];
	char buf2[8192];
	char prefixbuf[256];
	int i;
	int retcode;
	t_bool status_char;
	t_bool status_space;

	/* This is a shortcut for speed reasons: if no prefix (= quote_chars) is given just copy */
	if (!prefix || !*prefix) {
		copy_fp(fp_ip, fp_op);
		return;
	}

	if (strlen(prefix) > 240) /* truncate and terminate */
		prefix[240] = '\0';

	/* convert %S to %s, for compatibility reasons only */
	if (strstr(prefix, "%S")) {
		status_char = FALSE;
		for (i = 0; prefix[i]; i++) {
			if ((status_char) && (prefix[i] == 'S'))
				prefix[i] = 's';
			status_char = (prefix[i] == '%');
		}
	}

	/*
	 * strip trailing space if tinrc.quote_style doesn't have its
	 * QUOTE_COMPRESS flag set
	 */
	if (tinrc.quote_style & QUOTE_COMPRESS) {
		if (strstr(prefix, "%s"))
			snprintf(prefixbuf, sizeof(prefixbuf), prefix, initl);
		else {
			/* strip tailing space from quote-char for quoting quoted lines */
			strcpy(prefixbuf, prefix);
			if (prefixbuf[strlen(prefixbuf) - 1] == ' ')
				prefixbuf[strlen(prefixbuf) - 1] = '\0';
		}
	} else
		snprintf(prefixbuf, sizeof(prefixbuf), prefix, initl);

	/*
	 * if raw_data is true, the signature is exceptionally quoted, even if
	 * tinrc tells us not to do so. This extraordinary behavior occurs when
	 * replying or following up with the 'raw' message shown.
	 */

	while (fgets(buf, (int) sizeof(buf), fp_ip) != NULL) {
		if (!(tinrc.quote_style & QUOTE_SIGS) && !strcmp(buf, SIGDASHES) && !raw_data)
			break;
		if (strstr(prefix, "%s")) { /* initials wanted */
			if (buf[0] != '\n') { /* line is not empty */
				if (strchr(buf, '>')) {
					status_space = FALSE;
					status_char = TRUE;
					for (i = 0; buf[i] && (buf[i] != '>'); i++) {
						buf2[i] = buf[i];
						if (buf[i] != ' ')
							status_space = TRUE;
						if ((status_space) && !(isalpha((int)(unsigned char) buf[i]) || buf[i] == '>'))
							status_char = FALSE;
					}
					buf2[i] = '\0';
					if (status_char)	/* already quoted */
						retcode = fprintf(fp_op, "%s>%s", buf2, BlankIfNull(strchr(buf, '>')));
					else	/* ... to be quoted ... */
						retcode = fprintf(fp_op, "%s%s", prefixbuf, buf);
				} else	/* line was not already quoted (no >) */
					retcode = fprintf(fp_op, "%s%s", prefixbuf, buf);
			} else	/* line is empty */
				retcode = fprintf(fp_op, "%s\n", ((tinrc.quote_style & QUOTE_EMPTY) ? prefixbuf : ""));
		} else {		/* no initials in quote_string, just copy */
			if ((buf[0] != '\n') || (tinrc.quote_style & QUOTE_EMPTY))
				retcode = fprintf(fp_op, "%s%s", (buf[0] == '>' ? prefixbuf : prefix), buf);	/* use blank-stripped quote string if line is already quoted */
			else
				retcode = fprintf(fp_op, "\n");
		}
		if (retcode == EOF) {
			perror_message("copy_body() failed");
			return;
		}
	}
}


/*
 * Lookup 'env' in the environment. If it exists, return its value,
 * else return 'def'
 */
const char *
get_val(
	const char *env,	/* Environment variable we're looking for */
	const char *def)	/* Default value if no environ value found */
{
	const char *ptr;

	return ((ptr = getenv(env)) != NULL ? ptr : def);
}


/*
 * IMHO it's not tins job to take care about dumb editor backupfiles
 * otherwise BACKUP_FILE_EXT should be configurable via configure
 * or 'M'enu
 */
#define BACKUP_FILE_EXT ".b"
t_bool
invoke_editor(
	const char *filename,
	int lineno,
	struct t_group *group) /* return value is always ignored */
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
		my_strncpy(editor_format, (*group->attribute->editor_format ? group->attribute->editor_format : (group->attribute->start_editor_offset ? TIN_EDITOR_FMT_ON : TIN_EDITOR_FMT_OFF)), sizeof(editor_format) - 1);
	else
		my_strncpy(editor_format, (*tinrc.editor_format ? tinrc.editor_format : (tinrc.start_editor_offset ? TIN_EDITOR_FMT_ON : TIN_EDITOR_FMT_OFF)), sizeof(editor_format) - 1);

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
	struct t_group *group) /* return value is always ignored */
{
	FILE *fp_all, *fp_body, *fp_head;
	char buf[PATH_LEN], nam_body[PATH_LEN], nam_head[PATH_LEN];
	char ispell[PATH_LEN];
	t_bool retcode;

	if (group && group->attribute->ispell != NULL)
		STRCPY(ispell, group->attribute->ispell);
	else
		STRCPY(ispell, get_val("ISPELL", PATH_ISPELL));

	/*
	 * Now separating the header and body in two different files so that
	 * the header is not checked by ispell
	 */
#ifdef HAVE_LONG_FILE_NAMES
	snprintf(nam_body, sizeof(nam_body), "%s%s", nam, ".body");
	snprintf(nam_head, sizeof(nam_head), "%s%s", nam, ".head");
#else
	snprintf(nam_body, sizeof(nam_body), "%s%s", nam, ".b");
	snprintf(nam_head, sizeof(nam_head), "%s%s", nam, ".h");
#endif /* HAVE_LONG_FILE_NAMES */

	if ((fp_all = fopen(nam, "r")) == NULL) {
		perror_message(_(txt_cannot_open), nam);
		return FALSE;
	}

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

	while (fgets(buf, (int) sizeof(buf), fp_all) != NULL)
		fputs(buf, fp_body);

	fclose(fp_body);
	fclose(fp_all);

	sh_format(buf, sizeof(buf), "%s %s", ispell, nam_body);
	retcode = invoke_cmd(buf);

	append_file(nam_body, nam_head);
	unlink(nam_body);
	rename_file(nam_head, nam);
	return retcode;
}
#endif /* HAVE_ISPELL */


#ifndef NO_SHELL_ESCAPE
void
shell_escape(
	void)
{
	char *p, *tmp;
	char shell[LEN];

	tmp = fmt_string(_(txt_shell_escape), tinrc.default_shell_command);

	if (!prompt_string(tmp, shell, HIST_SHELL_COMMAND)) {
		free(tmp);
		return;
	}
	free(tmp);

	for (p = shell; *p && isspace((int) *p); p++)
		continue;

	if (*p)
		my_strncpy(tinrc.default_shell_command, p, sizeof(tinrc.default_shell_command) - 1);
	else {
		my_strncpy(shell, (*tinrc.default_shell_command ? tinrc.default_shell_command : (get_val(ENV_VAR_SHELL, DEFAULT_SHELL))), sizeof(shell) - 1);
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
void
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
		i = 3; /* max retries */
		while (i--) {
			wrote_newsrc_lines = write_newsrc();
			if ((wrote_newsrc_lines >= 0L) && (wrote_newsrc_lines >= read_newsrc_lines)) {
				if (!batch_mode || verbose)
					wait_message(0, _(txt_newsrc_saved));
				break;
			}

			if (wrote_newsrc_lines < read_newsrc_lines) {
				/* FIXME: prompt for retry? (i.e. remove break) */
				wait_message(0, _(txt_warn_newsrc), newsrc,
					(read_newsrc_lines - wrote_newsrc_lines),
					PLURAL(read_newsrc_lines - wrote_newsrc_lines, txt_group),
					OLDNEWSRC_FILE);
				if (!batch_mode)
					prompt_continue();
				break;
			}

			if (!batch_mode) {
				if (prompt_yn(_(txt_newsrc_again), TRUE) <= 0)
					break;
			}
		}

		write_input_history_file();

#ifdef HAVE_MH_MAIL_HANDLING
		write_mail_active_file();
#endif /* HAVE_MH_MAIL_HANDLING */
	}

#ifdef XFACE_ABLE
	slrnface_stop();
#endif /* XFACE_ABLE */

	/* Do this sometime after we save the newsrc in case this hangs up for any reason */
	if (ret != NNTP_ERROR_EXIT)
		nntp_close();			/* disconnect from NNTP server */

	free_all_arrays();

	/* TODO: why do we make this exception here? */
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

	if (buf && *buf) {
		my_fputs(buf, stderr);
		my_fputs(cCRLF, stderr);
		my_fflush(stderr);
		free(buf);
	}

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
	char *path,
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
		return 0; /* chmod via system() like for mkdir? */
#	endif /* HAVE_CHMOD */
	} else
		return -1;
#else
		return mkdir(path, mode);
#endif /* !HAVE_MKDIR */
}


#ifdef M_UNIX
void
rename_file(
	const char *old_filename,
	const char *new_filename)
{
	FILE *fp_old, *fp_new;
	int fd;
	mode_t mode = (mode_t) (S_IRUSR|S_IWUSR);
	struct stat statbuf;

	unlink(new_filename);

#	ifdef HAVE_LINK
	if (link(old_filename, new_filename) == -1)
#	else
	if (rename(old_filename, new_filename) < 0)
#	endif /* HAVE_LINK */
	{
		if (errno == EXDEV) {	/* create & copy file across filesystem */
			if ((fp_old = fopen(old_filename, "r")) == NULL) {
				perror_message(_(txt_cannot_open), old_filename);
				return;
			}
			if ((fp_new = fopen(new_filename, "w")) == NULL) {
				perror_message(_(txt_cannot_open), new_filename);
				fclose(fp_old);
				return;
			}

			if ((fd = fileno(fp_old)) != -1) {
				if (!fstat(fd, &statbuf))
					mode = statbuf.st_mode;
			}

			copy_fp(fp_old, fp_new);

			if ((fd = fileno(fp_new)) != -1)
				fchmod(fd, mode);

			fclose(fp_new);
			fclose(fp_old);
			errno = 0;
		} else {
			perror_message(_(txt_rename_error), old_filename, new_filename);
			return;
		}
	}
#	ifdef HAVE_LINK
	if (unlink(old_filename) == -1) {
		perror_message(_(txt_rename_error), old_filename, new_filename);
		return;
	}
#	endif /* HAVE_LINK */
}
#endif /* M_UNIX */


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
#endif /* IGNORE_SYSTEM_STATUS */

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


void
draw_percent_mark(
	long cur_num,
	long max_num)
{
	char buf[32]; /* should be big enough */
	int len;

	if (NOTESLINES <= 0)
		return;

	if (cur_num <= 0 && max_num <= 0)
		return;

	clear_message();
	snprintf(buf, sizeof(buf), "%s(%d%%) [%ld/%ld]", _(txt_more), (int) (cur_num * 100 / max_num), cur_num, max_num);
	len = strwidth(buf);
	MoveCursor(cLINES, cCOLS - len - (1 + BLANK_PAGE_COLS));
#ifdef HAVE_COLOR
	fcol(tinrc.col_normal);
#endif /* HAVE_COLOR */
	StartInverse();
	my_fputs(buf, stdout);
	EndInverse();
	my_flush();
}


/*
 * grab file portion of fullpath
 */
void
base_name(
	const char *fullpath,		/* /foo/bar/baz */
	char *file)				/* baz */
{
	size_t i;

	strcpy(file, fullpath);

	for (i = strlen(fullpath) - 1; i; i--) {
		if (fullpath[i] == DIRSEP) {
			strcpy(file, fullpath + i + 1);
			break;
		}
	}
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
 *
 * TODO: why not cache the mailbox_name?
 */
#define MAILDIR_NEW	"new"
t_bool
mail_check(
	void)
{
	const char *mailbox_name;
	struct stat buf;

	mailbox_name = get_val("MAIL", mailbox);

	if (mailbox_name != NULL && stat(mailbox_name, &buf) >= 0) {
		if ((int) (buf.st_mode & S_IFMT) == (int) S_IFDIR) { /* maildir setup */
			char *maildir_box;
			size_t maildir_box_len = strlen(mailbox_name) + strlen(MAILDIR_NEW) + 2;
			DIR *dirp;
			DIR_BUF *dp;

			maildir_box = my_malloc(maildir_box_len);
			joinpath(maildir_box, maildir_box_len, mailbox_name, MAILDIR_NEW);

			if (!(dirp = opendir(maildir_box))) {
				free(maildir_box);
				return FALSE;
			}
			free(maildir_box);
			while ((dp = readdir(dirp)) != NULL) {
				if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, ".."))) {
					CLOSEDIR(dirp);
					return TRUE;
				}
			}
			CLOSEDIR(dirp);
		} else {
			if (buf.st_atime < buf.st_mtime && buf.st_size > 0)
				return TRUE;
		}
	}
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
	int data;
	int offsets[6];
	int size_offsets = ARRAY_SIZE(offsets);

	if (!s || !*s)
		return "<No subject>"; /* also used in art.c:parse_headers() */

	do {
		data = pcre_exec(strip_re_regex.re, strip_re_regex.extra, s, strlen(s), 0, 0, offsets, size_offsets);
		if (offsets[0] == 0)
			s += offsets[1];
	} while (data >= 0);

	if (eat_was) do {
		data = pcre_exec(strip_was_regex.re, strip_was_regex.extra, s, strlen(s), 0, 0, offsets, size_offsets);
		if (offsets[0] > 0)
			s[offsets[0]] = '\0';
	} while (data >= 0);

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
	if (IS_LOCAL_CHARSET("ISO-8859"))
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
	char *p = idna_decode(art->from);
	int author;

	author = ((thread && !show_subject && curr_group->attribute->show_author == SHOW_FROM_NONE) ? SHOW_FROM_BOTH : curr_group->attribute->show_author);

	switch (author) {
		case SHOW_FROM_ADDR:
			strncpy(str, p, len);
			break;

		case SHOW_FROM_NAME:
			strncpy(str, (art->name ? art->name : p), len);
			break;

		case SHOW_FROM_BOTH:
			if (art->name)
				snprintf(str, len, "%s <%s>", art->name, p);
			else
				strncpy(str, p, len);
			break;

		case SHOW_FROM_NONE:
		default:
			len = 0;
			break;
	}

	free(p);
	*(str + len) = '\0';				/* NULL terminate */
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
	use_color = bool_not(use_color);

	if (use_color) {
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
 *
 * FIXME: get rid of hardcoded pid-length as pid_t might be long
 */
void
create_index_lock_file(
	char *the_lock_file)
{
	FILE *fp;
	char buf[64];
	int err;
	time_t epoch;

	if ((fp = fopen(the_lock_file, "r")) != NULL) {
		err = (fgets(buf, (int) sizeof(buf), fp) == NULL);
		fclose(fp);
		error_message(2, "\n%s: Already started pid=[%d] on %s", tin_progname, err ? 0 : atoi(buf), err ? "-" : buf + 8);
		free(tin_progname);
		giveup();
	}
	if ((fp = fopen(the_lock_file, "w")) != NULL) {
		fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));
		(void) time(&epoch);
		fprintf(fp, "%6d  %s\n", (int) process_id, ctime(&epoch));
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
	char *endp;
	char *start = s;
	char tbuf[LEN];
	int i, j;
	t_bool iflag;

	if (s == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strchr(format, '%') == NULL && strlen(format) + 1 >= maxsize)
		return 0;

	endp = s + maxsize;
	for (; *format && s < endp - 1; format++) {
		tbuf[0] = '\0';

		if (*format != '\\' && *format != '%') {
			*s++ = *format;
			continue;
		}

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
			i = strlen(tbuf);
			if (i) {
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
					STRCPY(tbuf, arts[respnum].from);
					break;

				case 'C':	/* First Name of author */
					if (arts[respnum].name != NULL) {
						STRCPY(tbuf, arts[respnum].name);
						if (strchr(tbuf, ' '))
							*(strchr(tbuf, ' ')) = '\0';
					} else {
						STRCPY(tbuf, arts[respnum].from);
					}
					break;

				case 'D':	/* Articles Date (reformatted as specified in attributes->date_format) */
					if (!my_strftime(tbuf, LEN - 1, curr_group->attribute->date_format, localtime(&arts[this_resp].date))) {
						STRCPY(tbuf, BlankIfNull(pgart.hdr.date));
					}
					break;

				case 'F':	/* Articles Address+Name */
					if (arts[respnum].name)
						snprintf(tbuf, sizeof(tbuf), "%s <%s>", arts[respnum].name, arts[respnum].from);
					else {
						STRCPY(tbuf, arts[respnum].from);
					}
					break;

				case 'G':	/* Groupname of Article */
					STRCPY(tbuf, group);
					break;

				case 'I':	/* Initials of author */
					STRCPY(tbuf, ((arts[respnum].name != NULL) ? arts[respnum].name : arts[respnum].from));
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
					STRCPY(tbuf, ((arts[respnum].name != NULL) ? arts[respnum].name : arts[respnum].from));
					break;

				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}
			i = strlen(tbuf);
			if (i) {
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
		return (s - start);
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
	char *editor,
	int linenum,
	const char *filename,
	char *s,
	size_t maxsize,
	char *format)
{
	char *endp;
	char *start = s;
	char tbuf[PATH_LEN];
	int i;

	if (s == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strchr(format, '%') == NULL && strlen(format) + 1 >= maxsize)
		return 0;

	endp = s + maxsize;
	for (; *format && s < endp - 1; format++) {
		tbuf[0] = '\0';

		if (*format != '\\' && *format != '%') {
			*s++ = *format;
			continue;
		}

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

				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}
			i = strlen(tbuf);
			if (i) {
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
			i = strlen(tbuf);
			if (i) {
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
		return (s - start);
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
	char *tbuf,
	char *endp)
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
 *   ~abc/News -> /usr/abc/News
 *   $var/News -> /env/var/News
 *   =file     -> $HOME/Mail/file
 *   =         -> $HOME/Mail/group.name
 *   +file     -> savedir/group.name/file
 *
 * Interestingly, %G is not documented as such and apparently unused
 *   ~/News/%G -> $HOME/News/group.name
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
 *
 * TODO: add %X (Article number), %M (Message-ID)?
 */
static int
_strfpath(
	const char *format,
	char *str,
	size_t maxsize,
	struct t_group *group,
	t_bool expand_all)
{
	char *endp;
	char *envptr;
	char defbuf[PATH_LEN];
	char tbuf[PATH_LEN];
	const char *startp = format;
	int i;
	struct passwd *pwd;
	t_bool is_mailbox = FALSE;

	if (str == NULL || format == NULL || maxsize == 0)
		return 0;

	if (strlen(format) + 1 >= maxsize)
		return 0;

	endp = str + maxsize;
	for (; *format && str < endp - 1; format++) {
		tbuf[0] = '\0';

		/*
		 * If just a normal part of the pathname copy it
		 */
		if (!strchr("~$=+%", *format)) {
			*str++ = *format;
			continue;
		}

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
				format++;
				if (*format && *format == '{') {
					format++;
					while (*format && !(strchr("}-", *format)))
						tbuf[i++] = *format++;
					tbuf[i] = '\0';
					i = 0;
					if (*format && *format == '-') {
						format++;
						while (*format && *format != '}')
							defbuf[i++] = *format++;
					}
					defbuf[i] = '\0';
				} else {
					while (*format && *format != '/')
						tbuf[i++] = *format++;
					tbuf[i] = '\0';
					format--;
					defbuf[0] = '\0';
				}
				/*
				 * OK lookup the variable in the shells environment
				 */
				envptr = getenv(tbuf);
				if (envptr == NULL || (*envptr == '\0'))
					strncpy(tbuf, defbuf, sizeof(tbuf) - 1);
				else
					strncpy(tbuf, envptr, sizeof(tbuf) - 1);
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
					if (strfpath((cmdline.args & CMDLINE_MAILDIR) ? cmdline.maildir : group->attribute->maildir, buf, sizeof(buf), group, FALSE)) {
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
					if (strfpath((cmdline.args & CMDLINE_SAVEDIR) ? cmdline.savedir : group->attribute->savedir, buf, sizeof(buf), group, FALSE)) {
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
				format++;
				if (*format && *format == 'G' && group != NULL) {
					memset(tbuf, 0, sizeof(tbuf));
					STRCPY(tbuf, group->name);
					i = strlen(tbuf);
					if (((str + i) < (endp - 1)) && (i > 0)) {
						strcpy(str, tbuf);
						str += i;
						break;
					} else {
						str[0] = '\0';
						return 0;
					}
				} else
					*str++ = *format;
				/* FALLTHROUGH */
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
	if (format[0] == '$') {
		char buf[PATH_LEN];

		if (_strfpath(format, buf, sizeof(buf), group, expand_all))
			return (_strfpath(buf, str, maxsize, group, expand_all));
	}

	return (_strfpath(format, str, maxsize, group, expand_all));
}


/*
 * TODO: Properly explain this
 */
char *
escape_shell_meta(
	const char *source,
	int quote_area)
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
					space--;
				}
				*dest++ = *source++;
				space--;
			}
			break;

		case dbl_quote:
			while (*source && (space > 0)) {
				if (*source == '\\' || *source == '"' || *source == '$' ||
					*source == '`') {
					*dest++ = '\\';
					space--;
				}
				*dest++ = *source++;
				space--;
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
				space--;
			}
			break;

		default:
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
	char *endp;
	char *start = dest;
	char tbuf[PATH_LEN];
	int quote_area = no_quote;

	/*
	 * safe guards: no destination to write to, no format, no space to
	 * write, or nothing to replace and format string longer than available
	 * space => return without any action
	 */
	if (dest == NULL || format == NULL || maxsize == 0)
		return 0;

	/*
	 * TODO: shouldn't we better check for no % OR format > maxsize?
	 *       as no replacement doesn't make sense (hardcoded To, Subject
	 *       and filename) and the resulting string usually is longer after
	 *       replacements were done (nobody uses enough %% to make the
	 *       result shorter than the input).
	 */
	if (strchr(format, '%') == NULL && strlen(format) + 1 >= maxsize)
		return 0;

	/*
	 * walk through format until end of format or end of available space
	 * and replace place holders
	 */
	endp = dest + maxsize;
	for (; *format && dest < endp - 1; format++) {
		tbuf[0] = '\0';

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

		/*
		 * handle sequences introduced by '\':
		 * - "\n" gets line feed
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
				if (sh_format(dest, endp - dest, "%s", tbuf) >= 0)
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
					if (INTERACTIVE_NONE != tinrc.interactive_mailer)
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
					if (INTERACTIVE_NONE != tinrc.interactive_mailer)
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
					if (INTERACTIVE_NONE != tinrc.interactive_mailer)
						strncpy(tbuf, userid, sizeof(tbuf) - 1);
					else {
#ifdef CHARSET_CONVERSION
						p = rfc1522_encode(userid, txt_mime_charsets[tinrc.mm_network_charset], ismail);
#else
						p = rfc1522_encode(userid, tinrc.mm_charset, ismail);
#endif /* CHARSET_CONVERSION */
						strncpy(tbuf, p, sizeof(tbuf) - 1);
						free(p);
					}
					tbuf[sizeof(tbuf) - 1] = '\0';	/* just in case */
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
						strncpy(dest, tbuf, endp - dest);
						dest += strlen(dest);
					}
				} else if (sh_format(dest, endp - dest, "%s", tbuf) >= 0) {
					dest += strlen(dest);
				} else
					return 0;
			}
		}
	}
out:
	if (dest < endp && *format == '\0') {
		*dest = '\0';
		return (dest - start);
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
	int maxsize) /* return value is always ignored */
{
	char tbuf[PATH_LEN];
	int i, j = 0;
	t_bool iflag = FALSE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wtmp, *wbuf;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (s == NULL || maxsize <= 0)
		return 0;

	STRCPY(tbuf, ((art->name != NULL) ? art->name : art->from));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	if ((wtmp = char2wchar_t(tbuf)) != NULL) {
		wbuf = my_malloc(sizeof(wchar_t) * (maxsize + 1));
		for (i = 0; wtmp[i] && j < maxsize; i++) {
			if (iswalpha((wint_t) wtmp[i])) {
				if (!iflag) {
					wbuf[j++] = wtmp[i];
					iflag = TRUE;
				}
			} else
				iflag = FALSE;
		}
		wbuf[j] = (wchar_t) '\0';
		s[0] = '\0';
		if (wcstombs(tbuf, wbuf, sizeof(tbuf) - 1) != (size_t) -1)
			strcat(s, tbuf);
		free(wtmp);
		free(wbuf);
	}
#else
	for (i = 0; tbuf[i] && j < maxsize; i++) {
		if (isalpha((int)(unsigned char) tbuf[i])) {
			if (!iflag) {
				s[j++] = tbuf[i];
				iflag = TRUE;
			}
		} else
			iflag = FALSE;
	}
	s[j] = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	return 0;
}


void
get_cwd(
	char *buf)
{
#ifdef HAVE_GETCWD
	getcwd(buf, PATH_LEN);
#else
#	ifdef HAVE_GETWD
	getwd(buf);
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
		*path = ((*name == '.') ? '/' : *name);
		name++;
		path++;
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
	char buf[LEN];

	make_group_path(group_name, buf);
	joinpath(group_path, group_path_len, base_dir, buf);
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


#ifndef M_UNIX
void
make_post_process_cmd(
	char *cmd,
	char *dir,
	char *file)
{
	char buf[LEN];
	char currentdir[PATH_LEN];

	get_cwd(currentdir);
	chdir(dir);
	sh_format(buf, sizeof(buf), cmd, file);
	invoke_cmd(buf);
	chdir(currentdir);
}
#endif /* !M_UNIX */


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
 * TODO: this seems to be unix specific, avoid !M_UNIX calls or
 *       add code for other OS
 *       this feature also isn't documented anywhere
 */
char *
random_organization(
	char *in_org)
{
	FILE *orgfp;
	int nool = 0, sol;
	static char selorg[512];

	*selorg = '\0';

	if (*in_org != '/')	/* M_UNIXism?! */
		return in_org;

	srand((unsigned int) time(NULL));

	if ((orgfp = fopen(in_org, "r")) == NULL)
		return selorg;

	/* count lines */
	while (fgets(selorg, (int) sizeof(selorg), orgfp))
		nool++;

	if (!nool) {
		fclose(orgfp);
		return selorg;
	}

	rewind(orgfp);
	sol = rand() % nool + 1;
	nool = 0;
	while ((nool != sol) && (fgets(selorg, (int) sizeof(selorg), orgfp)))
		nool++;

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

	/* this is usually .tin/.inputhistory */
	if ((fp = fopen(local_input_history_file, "r")) == NULL)
		return;

	if (!batch_mode)
		wait_message(0, _(txt_reading_input_history_file));

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

		his_e++;
		/* check if next type is reached */
		if (his_e >= HIST_SIZE) {
			hist_pos[his_w] = hist_last[his_w] = his_free;
			his_free = his_e = 0;
			his_w++;
		}
		/* check if end is reached */
		if (his_w > HIST_MAXNUM)
			break;
	}
	fclose(fp);

	if (cmd_line)
		printf("\r\n");
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
	file_tmp = get_tmpfilename(local_input_history_file);

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

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR)); /* rename_file() preserves mode */

	if ((his_w = ferror(fp)) || fclose(fp)) {
		error_message(2, _(txt_filesystem_full), local_input_history_file);
#ifdef HAVE_CHMOD
		/* fix modes for all pre 1.4.1 local_input_history_file files */
		chmod(local_input_history_file, (mode_t) (S_IRUSR|S_IWUSR));
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
static t_bool
buffer_to_local(
	char **line,
	size_t *max_line_len,
	const char *network_charset,
	const char *local_charset)
{
	/* FIXME: this should default in RFC2046.c to US-ASCII */
	if ((network_charset && *network_charset)) {	/* Content-Type: had a charset parameter */
		if (strcasecmp(network_charset, local_charset)) { /* different charsets */
			char *clocal_charset;
			iconv_t cd0, cd1, cd2;

			clocal_charset = my_malloc(strlen(local_charset) + strlen("//TRANSLIT") + 1);
			strcpy(clocal_charset, local_charset);
#	ifdef HAVE_ICONV_OPEN_TRANSLIT
			if (tinrc.translit)
				strcat(clocal_charset, "//TRANSLIT");
#	endif /* HAVE_ICONV_OPEN_TRANSLIT */

			/* iconv() might crash on broken multibyte sequences so check them */
			if (!strcasecmp(network_charset, "UTF-8") || !strcasecmp(network_charset, "utf8"))
				(void) utf8_valid(*line);

			/*
			 * TODO: hardcode unknown_ucs4 (0x00 0x00 0x00 0x3f)
			 *       instead of converting it?
			 */
			cd0 = iconv_open("UCS-4", "US-ASCII");
			cd1 = iconv_open("UCS-4", network_charset);
			cd2 = iconv_open(clocal_charset, "UCS-4");
			if (cd0 != (iconv_t) (-1) && cd1 != (iconv_t) (-1) && cd2 != (iconv_t) (-1)) {
				ICONV_CONST char *inbuf;
				char unknown = '?';
				ICONV_CONST char *unknown_ascii = &unknown;
				char *unknown_buf;
				char unknown_ucs4[4];
				char *obuf, *outbuf;
				char *tmpbuf, *tbuf;
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
				tbuf = my_malloc(tsize);
				tmpbuf = (char *) tbuf;

				do {
					errno = 0;
					result = iconv(cd1, &inbuf, &inbytesleft, &tmpbuf, &tmpbytesleft);
					if (result == (size_t) (-1)) {
						switch (errno) {
							case EILSEQ:
								memcpy(tmpbuf, unknown_ucs4, 4);
								tmpbuf += 4;
								tmpbytesleft -= 4;
								inbuf++;
								inbytesleft--;
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
				obuf = my_malloc(osize + 1);
				outbuf = (char *) obuf;

				do {
					/*
					 * save the parameters we need to redo the call of iconv
					 * if we get into the E2BIG case
					 */
					cur_inbuf = inbuf;
					cur_ibl = inbytesleft;
					used = outbuf - obuf;
					cur_obl = outbytesleft;

					errno = 0;
					result = iconv(cd2, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
					if (result == (size_t) (-1)) {
						switch (errno) {
							case EILSEQ:
								**&outbuf = '?';
								outbuf++;
								outbytesleft--;
								inbuf += 4;
								inbytesleft -= 4;
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

				**&outbuf = '\0';
				if (*max_line_len < strlen(obuf) + 1) {
					*max_line_len = strlen(obuf) + 1;
					*line = my_realloc(*line, *max_line_len);
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
				return FALSE;
			}
			free(clocal_charset);
		}
	}
	return TRUE;
}


/* convert from local_charset to txt_mime_charsets[mmnwcharset] */
t_bool
buffer_to_network(
	char *line,
	int mmnwcharset)
{
	char *obuf;
	char *outbuf;
	ICONV_CONST char *inbuf;
	iconv_t cd;
	size_t result, osize;
	size_t inbytesleft, outbytesleft;
	t_bool conv_success = TRUE;

	if (strcasecmp(txt_mime_charsets[mmnwcharset], tinrc.mm_local_charset)) {
		if ((cd = iconv_open(txt_mime_charsets[mmnwcharset], tinrc.mm_local_charset)) != (iconv_t) (-1)) {
			inbytesleft = strlen(line);
			inbuf = (char *) line;
			outbytesleft = 1 + inbytesleft * 4;
			osize = outbytesleft;
			obuf = my_malloc(osize + 1);
			outbuf = (char *) obuf;

			do {
				errno = 0;
				result = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
				if (result == (size_t) (-1)) {
					switch (errno) {
						case EILSEQ:
							/* TODO: only one '?' for each multibyte sequence ? */
							**&outbuf = '?';
							outbuf++;
							inbuf++;
							inbytesleft--;
							conv_success = FALSE;
							break;

						case E2BIG:
							obuf = my_realloc(obuf, osize * 2);
							outbuf = (char *) (obuf + osize - outbytesleft);
							outbytesleft += osize;
							osize <<= 1; /* double size */
							break;

						default:	/* EINVAL */
							inbytesleft = 0;
							conv_success = FALSE;
					}
				}
			} while (inbytesleft > 0);

			**&outbuf = '\0';
			strcpy(line, obuf); /* FIXME: here we assume that line is big enough to hold obuf */
			free(obuf);
			iconv_close(cd);
		}
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
		c++;
	}
	return a;
}


/*
 * do some character set processing
 *
 * this is called for headers, overview data, and article bodies
 * to set non-ASCII characters to '?'
 * (only with MIME_STRICT_CHARSET and !NO_LOCALE or CHARSET_CONVERSION
 * and network_charset=="US-ASCII")
 */
void
process_charsets(
	char **line,
	size_t *max_line_len,
	const char *network_charset,
	const char *local_charset,
	t_bool conv_tex2iso)
{
	char *p;

#ifdef CHARSET_CONVERSION
	if (strcasecmp(network_charset, "US-ASCII")) {	/* network_charset is NOT US-ASCII */
		if (iso2asc_supported >= 0)
			p = my_strdup("ISO-8859-1");
		else
			p = my_strdup(local_charset);
		if (!buffer_to_local(line, max_line_len, network_charset, p))
			buffer_to_ascii(*line);
		free(p);
	} else /* set non-ASCII characters to '?' */
		buffer_to_ascii(*line);
#else
#	if defined(MIME_STRICT_CHARSET) && !defined(NO_LOCALE)
	if ((local_charset && strcasecmp(network_charset, local_charset)) || !strcasecmp(network_charset, "US-ASCII"))
		/* different charsets || network charset is US-ASCII (see below) */
		buffer_to_ascii(*line);
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
	if (iso2asc_supported >= 0 && !strcasecmp(network_charset, "ISO-8859-1"))
#endif /* CHARSET_CONVERSION */
	{
		p = my_strdup(*line);
		convert_iso2asc(p, line, max_line_len, iso2asc_supported);
		free(p);
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
			message = txt_error_gnksa_langle;
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
 * this only does RFC822 decoding, decoding RFC2047 encoded parts must
 * be done by another call to the appropriate function
 */
static int
gnksa_dequote_plainphrase(
	char *realname,
	char *decoded,
	int addrtype)
{
	char *rpos;	/* read position */
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
						rpos++;
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
					case '.':
					case '[':
					case ']':
						return GNKSA_ILLEGAL_UNQUOTED_CHAR;
						/* NOTREACHED */
						break;

					case '=':
						*(wpos++) = *(rpos++);
						if ('?' == *rpos) {
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
						rpos++;
						break;

					case '(':
					case ')':
					case '<':
					case '>':
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
						if ('=' == *rpos) {
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
					case '<':
					case '>':
					case '\\':
						return GNKSA_ILLEGAL_PAREN_CHAR;
						/* NOTREACHED */
						break;

					case '=':
						*(wpos++) = *(rpos++);
						if ('?' == *rpos) {
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

	/* successful */
	*wpos = '\0';
	return GNKSA_OK;
}


/*
 * check domain literal
 */
static int
gnksa_check_domain_literal(
	char *domain)
{
	char term;
	int n;
	unsigned int x1, x2, x3, x4;

	/* parse domain literal into ip number */
	x1 = x2 = x3 = x4 = 666;
	term = '\0';

	if ('[' == *domain) { /* literal bracketed */
		n = sscanf(domain, "[%u.%u.%u.%u%c", &x1, &x2, &x3, &x4, &term);
		if (5 != n)
			return GNKSA_BAD_DOMAIN_LITERAL;

		if (']' != term)
			return GNKSA_BAD_DOMAIN_LITERAL;

	} else { /* literal not bracketed */
#ifdef REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
		return GNKSA_RBRACKET_MISSING;
#else
		n = sscanf(domain, "%u.%u.%u.%u%c", &x1, &x2, &x3, &x4, &term);
		/* there should be no terminating character present */
		if (4 != n)
			return GNKSA_BAD_DOMAIN_LITERAL;
#endif /* REQUIRE_BRACKETS_IN_DOMAIN_LITERAL */
	}

	/* check ip parts for legal range */
	if ((255 < x1) || (255 < x2) || (255 < x3) || (255 < x4))
		return GNKSA_BAD_DOMAIN_LITERAL;

	/* check for private ip or localhost - see RFC 5735, RFC 5737 */
	if ((!disable_gnksa_domain_check)
	    && ((0 == x1)				/* local network */
		|| (10 == x1)				/* private class A */
		|| ((172 == x1) && (16 == (x2 & 0xf0)))	/* private /12 */
		|| ((192 == x1) && (168 == x2))		/* private class B */
		|| ((192 == x1) && (0 == x2) && (2 == x3)) /* TEST NET-1 */
		|| ((198 == x1) && (51 == x2) && (100 == x3)) /* TEST NET-2 */
		|| ((203 == x1) && (0 == x2) && (113 == x3)) /* TEST NET-3 */
		|| (127 == x1)))			/* loopback */
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
	if ('[' == *domain) /* check value of domain literal */
		return gnksa_check_domain_literal(domain);

	/* check for leading or trailing dot */
	if (('.' == *domain) || ('.' == *(domain + strlen(domain) - 1)))
		return GNKSA_ZERO_LENGTH_LABEL;

	/* look for TLD start */
	aux = strrchr(domain, '.');
	if (NULL == aux)
		return GNKSA_SINGLE_DOMAIN;

	aux++;

	/* check existence of toplevel domain */
	switch ((int) strlen(aux)) {
		case 1:
			/* no numeric components allowed */
			if (('0' <= *aux) && ('9' >= *aux))
				return gnksa_check_domain_literal(domain);

			/* single letter TLDs do not exist */
			return GNKSA_ILLEGAL_DOMAIN;
			/* NOTREACHED */
			break;

		case 2:
			/* no numeric components allowed */
			if (('0' <= *aux) && ('9' >= *aux)
			    && ('0' <= *(aux + 1)) && ('9' >= *(aux + 1)))
				return gnksa_check_domain_literal(domain);

			if (('a' <= *aux) && ('z' >= *aux)
			    && ('a' <= *(aux + 1)) && ('z' >= *(aux + 1))) {
				i = ((*aux - 'a') * 26) + (*(aux + 1)) - 'a';
				if (!gnksa_country_codes[i])
					return GNKSA_UNKNOWN_DOMAIN;
			} else
				return GNKSA_ILLEGAL_DOMAIN;
			break;

		case 3:
			/* no numeric components allowed */
			if (('0' <= *aux) && ('9' >= *aux)
			    && ('0' <= *(aux + 1)) && ('9' >= *(aux + 1))
			    && ('0' <= *(aux + 2)) && ('9' >= *(aux + 2)))
				return gnksa_check_domain_literal(domain);
			/* FALLTHROUGH */
		default:
			/* check for valid domains */
			result = GNKSA_INVALID_DOMAIN;
			for (i = 0; *gnksa_domain_list[i]; i++) {
				if (!strcmp(aux, gnksa_domain_list[i]))
					result = GNKSA_OK;
			}
			if (disable_gnksa_domain_check)
				result = GNKSA_OK;
			if (GNKSA_OK != result)
				return result;
			break;
	}

	/* check for illegal labels */
	last = domain;
	for (aux = domain; *aux; aux++) {
		if ('.' == *aux) {
			if (aux - last - 1 > 63)
				return GNKSA_ILLEGAL_LABEL_LENGTH;

			if ('.' == *(aux + 1))
				return GNKSA_ZERO_LENGTH_LABEL;

			if (('-' == *(aux + 1)) || ('-' == *(aux - 1)))
				return GNKSA_ILLEGAL_LABEL_HYPHEN;

#ifdef ENFORCE_RFC1034
			if (('0' <= *(aux + 1)) && ('9' >= *(aux + 1)))
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
	if (('.' == *localpart) || ('.' == *(localpart + strlen(localpart) -1)))
		return GNKSA_ZERO_LENGTH_LOCAL_WORD;

	for (aux = localpart; *aux; aux++) {
		if (('.' == *aux) && ('.' == *(aux + 1)))
			return GNKSA_ZERO_LENGTH_LOCAL_WORD;
	}

	/* check for illegal characters in FQDN */
	for (aux = localpart; *aux; aux++) {
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
	char *addr_begin;
	char *addr_end;
	char work[HEADER_LEN];

	/* init return variables */
	*address = *realname = '\0';

	/* copy raw address into work area */
	strncpy(work, from, HEADER_LEN - 2);
	work[HEADER_LEN - 2] = '\0';
	work[HEADER_LEN - 1] = '\0';

	/* skip trailing whitespace */
	addr_end = work + strlen(work) - 1;
	while (addr_end >= work && (' ' == *addr_end || '\t' == *addr_end))
		addr_end--;

	if (addr_end < work) {
		*addrtype = GNKSA_ADDRTYPE_OLDSTYLE;
		return GNKSA_LPAREN_MISSING;
	}

	*(addr_end + 1) = '\0';
	*(addr_end + 2) = '\0';

	if ('>' == *addr_end) {
		/* route-address used */
		*addrtype = GNKSA_ADDRTYPE_ROUTE;

		/* get address part */
		addr_begin = addr_end;
		while (('<' != *addr_begin) && (addr_begin > work))
			addr_begin--;

		if ('<' != *addr_begin) /* syntax error in mail address */
			return GNKSA_LANGLE_MISSING;

		/* copy route address */
		*addr_end = *addr_begin = '\0';
		strcpy(address, addr_begin + 1);

		/* get realname part */
		addr_end = addr_begin - 1;
		addr_begin = work;

		/* strip surrounding whitespace */
		while (addr_end >= work && (' ' == *addr_end || '\t' == *addr_end))
			addr_end--;

		while ((' ' == *addr_begin) || ('\t' == *addr_begin))
			addr_begin++;

		*++addr_end = '\0';
		/* copy realname */
		strcpy(realname, addr_begin);
	} else {
		/* old-style address used */
		*addrtype = GNKSA_ADDRTYPE_OLDSTYLE;

		/* get address part */
		/* skip leading whitespace */
		addr_begin = work;
		while ((' ' == *addr_begin) || ('\t' == *addr_begin))
			addr_begin++;

		/* scan forward to next whitespace or null */
		addr_end = addr_begin;
		while ((' ' != *addr_end) && ('\t' != *addr_end) && (*addr_end))
			addr_end++;

		*addr_end = '\0';
		/* copy route address */
		strcpy(address, addr_begin);

		/* get realname part */
		addr_begin = addr_end + 1;
		addr_end = addr_begin + strlen(addr_begin) -1;
		/* strip surrounding whitespace */
		while ((' ' == *addr_end) || ('\t' == *addr_end))
			addr_end--;

		while ((' ' == *addr_begin) || ('\t' == *addr_begin))
			addr_begin++;

		/* any realname at all? */
		if (*addr_begin) {
			/* check for parentheses */
			if ('(' != *addr_begin)
				return GNKSA_LPAREN_MISSING;

			if (')' != *addr_end)
				return GNKSA_RPAREN_MISSING;

			/* copy realname */
			*addr_end = '\0';
			strcpy(realname, addr_begin + 1);
		}
	}

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

	/* split successful */
	return GNKSA_OK;
}


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
		wait_message(0, "From:=[%s]", from);
#endif /* DEBUG */

	/* split from */
	code = gnksa_split_from(from, address, realname, &addrtype);
	if ('\0' == *address) /* address missing or not extractable */
		return code;

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		wait_message(0, "address=[%s]", address);
#endif /* DEBUG */

	/* parse address */
	addr_begin = strrchr(address, '@');

	if (NULL != addr_begin) {
		/* temporarily terminate string at separator position */
		*addr_begin++ = '\0';

#ifdef DEBUG
		if (debug & DEBUG_MISC)
			wait_message(0, "FQDN=[%s]", addr_begin);
#endif /* DEBUG */

		/* convert FQDN part to lowercase */
		str_lwr(addr_begin);

		if (GNKSA_OK != (result = gnksa_check_domain(addr_begin))
		    && (GNKSA_OK == code)) /* error detected */
			code = result;

		if (GNKSA_OK != (result = gnksa_check_localpart(address))
		    && (GNKSA_OK == code)) /* error detected */
			code = result;

		/* restore separator character */
		*--addr_begin = '@';
	}

#ifdef DEBUG
	if (debug & DEBUG_MISC)
		wait_message(0, "realname=[%s]", realname);
#endif /* DEBUG */

	/* check realname */
	if (GNKSA_OK != (result = gnksa_dequote_plainphrase(realname, decoded, addrtype))) {
		if (GNKSA_OK == code) /* error detected */
			code = result;
	} else	/* copy dequoted realname to result variable */
		strcpy(realname, decoded);

#ifdef DEBUG
	if (debug & DEBUG_MISC) { /* TODO: dump to a file instead of wait_message() */
		if (GNKSA_OK != code)
			wait_message(2, "From:=[%s], GNKSA=[%d]", from, code);
		else
			wait_message(0, "GNKSA=[%d]", code);
	}
#endif /* DEBUG */

	return code;
}


/*
 * check given address
 */
int
gnksa_check_from(
	char *from)
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
	char *ptr = line + strlen(line) - 1;

	while ((ptr >= line) && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n'))
		ptr--;

	*++ptr = '\0';

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
	int numc;
	t_bool illegal;

	c = line;

	while (*c != '\0') {
		if (!(*c & 0x80)) { /* plain US-ASCII? */
			c++;
			continue;
		}

		numc = 0;
		illegal = FALSE;
		d = (*c & 0x7c);	/* remove bits 7,1,0 */

		do {
			numc++;
		} while ((d <<= 1) & 0x80);	/* get sequence length */

		if (c + numc > line + strlen(line)) { /* sequence runs past end of string */
			illegal = TRUE;
			numc = line + strlen(line) - c;
		}

		if (!illegal) {
			d = *c;
			e = *(c + 1);

			switch (numc) {
				case 2:
					/* out of range or sequences which would also fit into 1 byte */
					if (d < 0xc2 || d > 0xdf)
						illegal = TRUE;
					break;

				case 3:
					f = *(c + 2);
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
					f = *(c + 2);
					g = *(c + 3);
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

#	if 0	/* currently not used, see also check above */
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
			e = *(c + d);
			if (e < 0x80 || e > 0xbf || e == '\0' || e == '\n')
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
				c++;
			}
		}
	}
	return line;
}
#endif /* CHARSET_CONVERSION || (MULTIBYTE_ABLE && !NO_LOCALE) */


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
#	if defined(HAVE_LIBIDNKIT) && defined(HAVE_IDN_DECODENAME)
	{
		idn_result_t res;
		char *q, *r = NULL;

		if ((q = strrchr(out, '@'))) {
			q++;
			r = strrchr(in, '@');
			r++;
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
#ifdef HAVE_LIBICUUC_46_API
		UIDNA *uts46;
		UIDNAInfo info = UIDNA_INFO_INITIALIZER;
#endif /* HAVE_LIBICUUC_46_API */

		if ((s = strrchr(out, '@')))
			s++;
		else
			s = out;

		src = char2UChar(s);
#ifndef HAVE_LIBICUUC_46_API
		uidna_IDNToUnicode(src, -1, dest, 1023, UIDNA_USE_STD3_RULES, NULL, &err);
#else
		uts46 = uidna_openUTS46(UIDNA_USE_STD3_RULES, &err);
		uidna_nameToUnicode(uts46, src, u_strlen(src), dest, 1023, &info, &err);
		uidna_close(uts46);
#endif /* !HAVE_LIBICUUC_46_API */
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
		int rs = IDNA_SUCCESS;

		if ((t = strrchr(out, '@')))
			t++;
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
			if (debug & DEBUG_MISC)
#				ifdef HAVE_IDNA_STRERROR
				wait_message(2, "idna_to_unicode_lzlz(%s): %s", t, idna_strerror(rs));
#				else
				wait_message(2, "idna_to_unicode_lzlz(%s): %d", t, rs);
#				endif /* HAVE_IDNA_STRERROR */
		}
#			endif /* DEBUG */
		FreeIfNeeded(s);
	}
#		endif /* HAVE_LIBIDN && HAVE_IDNA_TO_UNICODE_LZLZ */
#	endif /* HAVE_LIBICUUC */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	return out;
}


int
tin_version_info(
	FILE *fp)
{
	int wlines = 0;	/* written lines */

#if defined(__DATE__) && defined(__TIME__)
	fprintf(fp, _("Version: %s %s release %s (\"%s\") %s %s\n"),
		PRODUCT, VERSION, RELEASEDATE, RELEASENAME, __DATE__, __TIME__);
#else
	fprintf(fp, _("Version: %s %s release %s (\"%s\")\n"),
		PRODUCT, VERSION, RELEASEDATE, RELEASENAME);
#endif /* __DATE__ && __TIME__ */
	wlines++;

#ifdef SYSTEM_NAME
	fprintf(fp, "Platform:\n");
	fprintf(fp, "\tOS-Name  = \"%s\"\n", SYSTEM_NAME);
	wlines += 2;
#endif /* SYSTEM_NAME */

#ifdef TIN_CC
	fprintf(fp, "Compiler:\n");
	fprintf(fp, "\tCC       = \"%s\"\n", TIN_CC);
	wlines += 2;
#	ifdef TIN_CFLAGS
		fprintf(fp, "\tCFLAGS   = \"%s\"\n", TIN_CFLAGS);
		wlines++;
#	endif /* TIN_CFLAGS */
#	ifdef TIN_CPP
		fprintf(fp, "\tCPP      = \"%s\"\n", TIN_CPP);
		wlines++;
#	endif /* TIN_CPP */
#	ifdef TIN_CPPFLAGS
		fprintf(fp, "\tCPPFLAGS = \"%s\"\n", TIN_CPPFLAGS);
		wlines++;
#	endif /* TIN_CPPFLAGS */
#endif /* TIN_CC */

#ifdef TIN_LD
	fprintf(fp, "Linker and Libraries:\n");
	fprintf(fp, "\tLD       = \"%s\"\n", TIN_LD);
	wlines += 2;
#	ifdef TIN_LDFLAGS
		fprintf(fp, "\tLDFLAGS  = \"%s\"\n", TIN_LDFLAGS);
		wlines++;
#	endif /* TIN_LDFLAGS */
#	ifdef TIN_LIBS
		fprintf(fp, "\tLIBS     = \"%s\"\n", TIN_LIBS);
		wlines++;
#	endif /* TIN_LIBS */
#endif /* TIN_LD */
	fprintf(fp, "\tPCRE     = \"%s\"\n", pcre_version());
	wlines++;

	fprintf(fp, "Characteristics:\n\t"
/* TODO: complete list and do some useful grouping; show only in -vV case? */
#ifdef DEBUG
			"+DEBUG "
#else
			"-DEBUG "
#endif /* DEBUG */
#ifdef NNTP_ONLY
			"+NNTP_ONLY "
#else
#	ifdef NNTP_ABLE
			"+NNTP_ABLE "
#	else
			"-NNTP_ABLE "
#	endif /* NNTP_ABLE */
#endif /* NNTP_ONLY */
#ifdef NO_POSTING
			"+NO_POSTING "
#else
			"-NO_POSTING "
#endif /* NO_POSTING */
#ifdef BROKEN_LISTGROUP
			"+BROKEN_LISTGROUP "
#else
			"-BROKEN_LISTGROUP "
#endif /* BROKEN_LISTGROUP */
#ifdef XHDR_XREF
			"+XHDR_XREF"
#else
			"-XHDR_XREF"
#endif /* XHDR_XREF */
			"\n\t"
#ifdef HAVE_FASCIST_NEWSADMIN
			"+HAVE_FASCIST_NEWSADMIN "
#else
			"-HAVE_FASCIST_NEWSADMIN "
#endif /* HAVE_FASCIST_NEWSADMIN */
#ifdef ENABLE_IPV6
			"+ENABLE_IPV6 "
#else
			"-ENABLE_IPV6 "
#endif /* ENABLE_IPV6 */
#ifdef HAVE_COREFILE
			"+HAVE_COREFILE"
#else
			"-HAVE_COREFILE"
#endif /* HAVE_COREFILE */
			"\n\t"
#ifdef NO_SHELL_ESCAPE
			"+NO_SHELL_ESCAPE "
#else
			"-NO_SHELL_ESCAPE "
#endif /* NO_SHELL_ESCAPE */
#ifdef DISABLE_PRINTING
			"+DISABLE_PRINTING "
#else
			"-DISABLE_PRINTING "
#endif /* DISABLE_PRINTING */
#ifdef DONT_HAVE_PIPING
			"+DONT_HAVE_PIPING "
#else
			"-DONT_HAVE_PIPING "
#endif /* DONT_HAVE_PIPING */
#ifdef NO_ETIQUETTE
			"+NO_ETIQUETTE"
#else
			"-NO_ETIQUETTE"
#endif /* NO_ETIQUETTE */
			"\n\t"
#ifdef HAVE_LONG_FILE_NAMES
			"+HAVE_LONG_FILE_NAMES "
#else
			"-HAVE_LONG_FILE_NAMES "
#endif /* HAVE_LONG_FILE_NAMES */
#ifdef APPEND_PID
			"+APPEND_PID "
#else
			"-APPEND_PID "
#endif /* APPEND_PID */
#ifdef HAVE_MH_MAIL_HANDLING
			"+HAVE_MH_MAIL_HANDLING"
#else
			"-HAVE_MH_MAIL_HANDLING"
#endif /* HAVE_MH_MAIL_HANDLING */
			"\n\t"
#ifdef HAVE_ISPELL
			"+HAVE_ISPELL "
#else
			"-HAVE_ISPELL "
#endif /* HAVE_ISPELL */
#ifdef HAVE_METAMAIL
			"+HAVE_METAMAIL "
#else
			"-HAVE_METAMAIL "
#endif /* HAVE_METAMAIL */
#ifdef HAVE_SUM
			"+HAVE_SUM"
#else
			"-HAVE_SUM"
#endif /* HAVE_SUM */
			"\n\t"
#ifdef HAVE_COLOR
			"+HAVE_COLOR "
#else
			"-HAVE_COLOR "
#endif /* HAVE_COLOR */
#ifdef HAVE_PGP
			"+HAVE_PGP "
#else
			"-HAVE_PGP "
#endif /* HAVE_PGP */
#ifdef HAVE_PGPK
			"+HAVE_PGPK "
#else
			"-HAVE_PGPK "
#endif /* HAVE_PGPK */
#ifdef HAVE_GPG
			"+HAVE_GPG"
#else
			"-HAVE_GPG"
#endif /* HAVE_GPG */
			"\n\t"
#ifdef MIME_BREAK_LONG_LINES
			"+MIME_BREAK_LONG_LINES "
#else
			"-MIME_BREAK_LONG_LINES "
#endif /* MIME_BREAK_LONG_LINES */
#ifdef MIME_STRICT_CHARSET
			"+MIME_STRICT_CHARSET "
#else
			"-MIME_STRICT_CHARSET "
#endif /* MIME_STRICT_CHARSET */
#ifdef CHARSET_CONVERSION
			"+CHARSET_CONVERSION"
#else
			"-CHARSET_CONVERSION"
#endif /* CHARSET_CONVERSION */
			"\n\t"
#ifdef MULTIBYTE_ABLE
			"+MULTIBYTE_ABLE "
#else
			"-MULTIBYTE_ABLE "
#endif /* MULTIBYTE_ABLE */
#ifdef NO_LOCALE
			"+NO_LOCALE "
#else
			"-NO_LOCALE "
#endif /* NO_LOCALE */
#ifdef USE_LONG_ARTICLE_NUMBERS
			"+USE_LONG_ARTICLE_NUMBERS"
#else
			"-USE_LONG_ARTICLE_NUMBERS"
#endif /* USE_LONG_ARTICLE_NUMBERS */
			"\n\t"
#ifdef USE_CANLOCK
			"+USE_CANLOCK "
#else
			"-USE_CANLOCK "
#endif /* USE_CANLOCK */
#ifdef EVIL_INSIDE
			"+EVIL_INSIDE "
#else
			"-EVIL_INSIDE "
#endif /* EVIL_INSIDE */
#ifdef FORGERY
			"+FORGERY "
#else
			"-FORGERY "
#endif /* FORGERY */
#ifdef TINC_DNS
			"+TINC_DNS "
#else
			"-TINC_DNS "
#endif /* TINC_DNS */
#ifdef ENFORCE_RFC1034
			"+ENFORCE_RFC1034"
#else
			"-ENFORCE_RFC1034"
#endif /* ENFORCE_RFC1034 */
			"\n\t"
#ifdef REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
			"+REQUIRE_BRACKETS_IN_DOMAIN_LITERAL "
#else
			"-REQUIRE_BRACKETS_IN_DOMAIN_LITERAL "
#endif /* REQUIRE_BRACKETS_IN_DOMAIN_LITERAL */
#ifdef FOLLOW_USEFOR_DRAFT
			"+FOLLOW_USEFOR_DRAFT"
#else
			"-FOLLOW_USEFOR_DRAFT"
#endif /* FOLLOW_USEFOR_DRAFT */
			"\n");
	wlines += 11;
	fflush(fp);
	return wlines;
}


void
draw_mark_selected(
	int i)
{
	MoveCursor(INDEX2LNUM(i), mark_offset);
	StartInverse();	/* ToggleInverse() doesn't work correct with ncurses4.x */
	my_fputc(tinrc.art_marked_selected, stdout);
	EndInverse();	/* ToggleInverse() doesn't work correct with ncurses4.x */
}


int
tin_gettime(
	struct t_tintime *tt)
{
#ifdef HAVE_CLOCK_GETTIME
	static struct timespec cgt;
#endif /* HAVE_CLOCK_GETTIME */
#ifdef HAVE_GETTIMEOFDAY
	static struct timeval gt;
#endif /* HAVE_GETTIMEOFDAY */

#ifdef HAVE_CLOCK_GETTIME
	if (clock_gettime(CLOCK_REALTIME, &cgt) == 0) {
		tt->tv_sec = cgt.tv_sec;
		tt->tv_nsec = cgt.tv_nsec;
		return 0;
	}
#endif /* HAVE_CLOCK_GETTIME */
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
