/*
 *  Project   : tin - a Usenet reader
 *  Module    : misc.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 1997-12-31
 *  Notes     :
 *
 * Copyright (c) 1991-2001 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
 *    This product includes software developed by Iain Lea, Rich Skrenta.
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


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef VERSION_H
#	include  "version.h"
#endif /* !VERSION_H */
#ifndef BUGREP_H
#	include  "bugrep.h"
#endif /* !BUGREP_H */
#ifndef included_trace_h
#	include "trace.h"
#endif /* !included_trace_h */
#ifndef TIN_POLICY_H
#	include "policy.h"
#endif /* !TIN_POLICY_H */
#ifndef RFC2046_H
#	include "rfc2046.h"
#endif /* !RFC2046_H */

/*
 * defines to control GNKSA-checks behaviour:
 * - ENFORCE_RFC1034
 *   require domain name components not to start with a digit
 *
 * - REQUIRE_BRACKETS_IN_DOMAIN_LITERAL
 *   require domain literals to be enclosed in square brackets
 */

/*
 * Local prototypes
 */
static int gnksa_check_domain (char *domain);
static int gnksa_check_domain_literal (char *domain);
static int gnksa_check_localpart (const char *localpart);
static int gnksa_dequote_plainphrase (char *realname, char *decoded, int addrtype);
static int gnksa_split_from (const char *from, char *address, char *realname, int *addrtype);
static int strfeditor (char *editor, int linenum, const char *filename, char *s, size_t maxsize, char *format);
static void write_input_history_file (void);
#ifdef LOCAL_CHARSET
	static int to_local (int c);
	static int to_network (int c);
#endif /* LOCAL_CHARSET */

/*
 * generate tmp-filename
 */
char *
get_tmpfilename (
	const char *filename)
{
	char *file_tmp;

	/* alloc memory for tmp-filename */
	file_tmp = (char *) my_malloc (strlen (filename)+5);

	/* generate tmp-filename */
	sprintf (file_tmp, "%s.tmp", filename);
	return file_tmp;
}

/*
 * append_file instead of rename_file
 * minimum error trapping
 */
void
append_file (
	char *old_filename,
	char *new_filename)
{
	FILE *fp_old, *fp_new;

	if ((fp_new = fopen (new_filename, "r")) == (FILE *) 0) {
		perror_message (_(txt_cannot_open), new_filename);
		return;
	}
	if ((fp_old = fopen (old_filename, "a+")) == (FILE *) 0) {
		perror_message (_(txt_cannot_open), old_filename);
		fclose (fp_new);
		return;
	}
	copy_fp (fp_new, fp_old);
	fclose (fp_old);
	fclose (fp_new);
}


void
asfail (
	const char *file,
	int line,
	const char *cond)
{
	my_fprintf (stderr, txt_error_asfail, tin_progname, file, line, cond);
	my_fflush (stderr);

/*
 * create a core dump
 */
#ifdef HAVE_COREFILE
#	ifdef SIGABRT
		sigdisp(SIGABRT, SIG_DFL);
		kill (process_id, SIGABRT);
#	else
#		ifdef SIGILL
			sigdisp(SIGILL, SIG_DFL);
			kill (process_id, SIGILL);
#		else
#			ifdef SIGIOT
				sigdisp(SIGIOT, SIG_DFL);
				kill (process_id, SIGIOT);
#			endif /* SIGIOT */
#		endif /* SIGILL */
#	endif /* SIGABRT */
#endif /* HAVE_COREFILE */

	giveup();
}


/*
 * Quick copying of files
 * Returns FALSE if copy failed. Caller may wish to check for SIGPIPE
 */
t_bool
copy_fp (
	FILE *fp_ip,
	FILE *fp_op)
{
	char buf[8192];
	size_t have, sent;

	while ((have = fread (buf, 1, sizeof(buf), fp_ip)) != 0) {
		sent = fwrite (buf, 1, have, fp_op);
		if (sent != have) {
			TRACE(("copy_fp wrote %d of %d:{%.*s}", sent, have, (int) sent, buf));
			if (!got_sig_pipe) /* !SIGPIPE => more serious error */
				perror_message (_(txt_error_copy_fp));
			return FALSE;
		}
		TRACE(("copy_fp wrote %d:{%.*s}", sent, (int) sent, buf));
	}
	return TRUE;
}


/*
 * backup file
 * Returns FALSE if backup failed or source file does not exists.
 */
t_bool
backup_file (
	const char *filename,
	const char *backupname)
{
	FILE *fp_in, *fp_out;
	t_bool ret = FALSE;

	if ((fp_in = fopen (filename, "r")) == (FILE *) 0)
		return ret;

	/* don't follow links when writing backup files */
	unlink (backupname);
	if ((fp_out = fopen (backupname, "w")) == (FILE *) 0) {
		fclose (fp_in);
		return ret;
	}

	ret = copy_fp (fp_in, fp_out);

	fclose (fp_out);
	fclose (fp_in);
	return ret;
}


/*
 * copy the body of articles with given file pointers,
 * prefix (= quote_chars), initials of the articles author
 * with_sig is set if the signature should be quoted
 *
 * TODO: rewrite from scratch, the code is awfull
 */
void
copy_body (
	FILE *fp_ip,
	FILE *fp_op,
	char *prefix,
	char *initl,
	t_bool with_sig)
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
		copy_fp (fp_ip, fp_op);
		return;
	}

	if (strlen(prefix) > 240) /* truncate and terminate */
		prefix[240] = '\0';

	/* convert %S to %s, for compability reasons only */
	if (strstr(prefix, "%S")) {
		status_char = FALSE;
		for (i = 0; prefix[i]; i++) {
			if ((status_char) && (prefix[i] == 'S'))
				prefix[i] = 's';
			status_char = (prefix[i] == '%');
		}
	}

	if (strstr(prefix, "%s"))
		sprintf(prefixbuf, prefix, initl);
	else {
		/* strip tailing space from quote-char for quoting quoted lines */
		strcpy(prefixbuf, prefix);
		if (prefixbuf[strlen(prefixbuf)-1] == ' ')
			prefixbuf[strlen(prefixbuf)-1] = '\0';
	}

	while (fgets (buf, (int) sizeof(buf), fp_ip) != (char *) 0) {
		if (!with_sig && !strcmp(buf, SIGDASHES))
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
						if ((status_space) && !(isalpha((int)buf[i]) || buf[i] == '>'))
							status_char = FALSE;
					}
					buf2[i] = '\0';
					if (status_char)	/* already quoted */
						retcode = fprintf (fp_op, "%s>%s", buf2, strchr(buf, '>'));
					else	/* ... to be quoted ... */
						retcode = fprintf (fp_op, "%s%s", prefixbuf, buf);
				} else	/* line was not already quoted (no >) */
					retcode = fprintf (fp_op, "%s%s", prefixbuf, buf);
			} else	/* line is empty */
					retcode = fprintf (fp_op, "%s\n", (tinrc.quote_empty_lines ? prefixbuf : ""));
		} else {		/* no initials in quote_string, just copy */
			if ((buf[0] != '\n') || tinrc.quote_empty_lines)
				retcode = fprintf (fp_op, "%s%s", ((buf[0] == '>' || buf[0] == ' ') ? prefixbuf : prefix), buf);  /* use blank-stripped quote string if line is already quoted or beginns with a space */
			else
				retcode = fprintf (fp_op, "\n");
		}
		if (retcode == EOF) {
			perror_message ("copy_body() failed");
			return;
		}
	}
}


/*
 * Lookup 'env' in the environment. If it exists, return its value,
 * else return 'def'
 */
const char *
get_val (
	const char *env,	/* Environment variable we're looking for */
	const char *def)	/* Default value if no environ value found */
{
	const char *ptr;

	return ((ptr = getenv(env)) != (char *) 0 ? ptr : def);
}


/*
 * IMHO it's not tins job to take care about dumb editor backupfiles
 * otherwise BACKUP_FILE_EXT should be configurable via configure
 * or 'M'enu
 */
#define BACKUP_FILE_EXT ".b"

t_bool
invoke_editor (
	const char *filename,
	int lineno) /* return value is always ignored */
{
	char *my_editor;
	char buf[PATH_LEN], fnameb[PATH_LEN];
	char editor_format[PATH_LEN];
	t_bool retcode;
	static char editor[PATH_LEN];
	static t_bool first = TRUE;

	if (first) {
		my_editor = getenv ("EDITOR");

		my_strncpy (editor, my_editor != NULL ? my_editor : get_val ("VISUAL", DEFAULT_EDITOR), sizeof(editor) - 1);
		first = FALSE;
	}

	my_strncpy (editor_format, (*tinrc.editor_format ? tinrc.editor_format : (tinrc.start_editor_offset ? TIN_EDITOR_FMT_ON : TIN_EDITOR_FMT_OFF)), sizeof(editor_format) - 1);

	if (!strfeditor (editor, lineno, filename, buf, sizeof(buf), editor_format))
		sh_format (buf, sizeof(buf), "%s %s", editor, filename);

	cursoron();
	my_flush();
	retcode = invoke_cmd (buf);

#ifdef BACKUP_FILE_EXT
	strcpy (fnameb, filename);
	strcat (fnameb, BACKUP_FILE_EXT);
	unlink (fnameb);
#endif /* BACKUP_FILE_EXT */
	return retcode;
}


#ifdef HAVE_ISPELL
t_bool
invoke_ispell (
	const char *nam,
	struct t_group *psGrp) /* return value is always ignored */
{
	FILE *fp_all, *fp_body, *fp_head;
	char buf[PATH_LEN], nam_body[100], nam_head[100];
	t_bool retcode;
	char ispell[PATH_LEN];

/*
 * IMHO we don't need an exception for VMS as PATH_ISPELL
 * defaults to ispell (uj 19990617)
 */
/*
#	ifdef VMS
	strcpy (ispell, "ispell");
#	else
*/
	if (psGrp && psGrp->attribute->ispell != (char *) 0)
		STRCPY(ispell, psGrp->attribute->ispell);
	else
		STRCPY(ispell, get_val("ISPELL", PATH_ISPELL));
/*
#	endif *//* VMS */

	/*
	 * Now seperating the header and body in two different files so that
	 * the header is not checked by ispell
	 */
	strncpy (nam_body, nam, 90);
	strcat (nam_body, ".body");

	strncpy (nam_head, nam, 90);
	strcat (nam_head, ".head");

	if ((fp_all = fopen(nam, "r")) == (FILE *) 0) {
		perror_message(_(txt_cannot_open), nam);
		return FALSE;
	}


	if ((fp_head = fopen (nam_head, "w")) == NULL) {
		perror_message(_(txt_cannot_open), nam_head);
		fclose (fp_all);
		return FALSE;
	}

	if ((fp_body = fopen (nam_body, "w")) == NULL) {
		perror_message(_(txt_cannot_open), nam_body);
		fclose (fp_head);
		fclose (fp_all);
		return FALSE;
	}

	while (fgets (buf, (int) sizeof(buf), fp_all) != NULL) {
		fputs (buf, fp_head);
		if (buf[0] == '\n' || buf[0] == '\r') {
			fclose (fp_head);
			break;
		}
	}

	while (fgets(buf, (int) sizeof(buf), fp_all) != NULL)
		fputs(buf, fp_body);

	fclose (fp_body);
	fclose (fp_all);

	sh_format (buf, sizeof(buf), "%s %s", ispell, nam_body);
	retcode = invoke_cmd(buf);

	append_file (nam_head, nam_body);
	unlink (nam_body);
	rename_file (nam_head, nam);

	return retcode;
}
#endif /* HAVE_ISPELL */


#ifndef NO_SHELL_ESCAPE
void
shell_escape (
	void)
{
	char *p;
	char shell[LEN];

	sprintf (mesg, _(txt_shell_escape), tinrc.default_shell_command);

	if (!prompt_string (mesg, shell, HIST_SHELL_COMMAND))
		return;

	for (p = shell; *p && isspace((int)*p); p++)
		continue;

	if (*p)
		my_strncpy (tinrc.default_shell_command, p, sizeof(tinrc.default_shell_command));
	else {
		my_strncpy (shell, (*tinrc.default_shell_command ? tinrc.default_shell_command : (get_val (ENV_VAR_SHELL, DEFAULT_SHELL))), sizeof(shell));
		p = shell;
	}

	ClearScreen ();
	sprintf (mesg, "Shell Command (%s)", p); /* FIXME: -> lang.c */
	center_line (0, TRUE, mesg);
	MoveCursor (INDEX_TOP, 0);

	(void)invoke_cmd(p);

	prompt_continue ();

	if (tinrc.draw_arrow)
		ClearScreen ();
}


/*
 * shell out, if supported
 */
void
do_shell_escape (
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
tin_done (
	int ret)
{
	static int nested;
	register int i;
	t_bool ask = TRUE;
	struct t_group *group;
	signed long int wrote_newsrc_lines = -1;

	if (nested++)
		giveup();

	signal_context = cMain;

#ifdef USE_CURSES
	scrollok (stdscr, TRUE);			/* Allows display of multi-line messages */
#endif /* USE_CURSES */

	/*
	 * check if any groups were read & ask if they should marked read
	 */
	if (tinrc.catchup_read_groups && !cmd_line && !no_write) {
		for (i = 0; i < selmenu.max; i++) {
			group = &active[my_group[i]];
			if (group->read_during_session) {
				if (ask) {
					if (prompt_yn (cLINES, _(txt_catchup_all_read_groups), FALSE) == 1) {
						ask = FALSE;
						tinrc.thread_articles = THREAD_NONE;	/* speeds up index loading */
					} else
						break;
				}
				wait_message (0, "Catchup %s...", group->name);	/* FIXME: -> lang.c */
				grp_mark_read (group, NULL);
			}
		}
	}

	/*
	 * Save the newsrc file. If it fails for some reason, give the user a
	 * chance to try again
	 */
	if (!no_write) {
		forever {
			if (((wrote_newsrc_lines = write_newsrc ()) >= 0L) && (wrote_newsrc_lines >= read_newsrc_lines)) {
				my_fputs(_(txt_newsrc_saved), stdout);
				break;
			}

			if (wrote_newsrc_lines < read_newsrc_lines) {
				/* FIXME: prompt for retry? (i.e. remove break) */
				wait_message(0, _(txt_warn_newsrc), newsrc,
					(read_newsrc_lines - wrote_newsrc_lines),
					PLURAL(read_newsrc_lines - wrote_newsrc_lines, txt_group),
					OLDNEWSRC_FILE);
				prompt_continue();
				break;
			}

			if (!prompt_yn (cLINES, _(txt_newsrc_again), TRUE))
				break;
		}

		write_input_history_file ();
#if 0 /* FIXME */
		write_attributes_file (local_attributes_file);
#endif /* 0 */

#ifdef HAVE_MH_MAIL_HANDLING
		write_mail_active_file ();
#endif /* HAVE_MH_MAIL_HANDLING */
	}

	/* Do this sometime after we save the newsrc in case this hangs up for any reason */
	if (ret != NNTP_ERROR_EXIT)
		nntp_close ();			/* disconnect from NNTP server */

	free_all_arrays ();
#ifdef SIGUSR1
	if (ret != -SIGUSR1) {
#endif /* SIGUSR1 */
#ifdef HAVE_COLOR
		use_color = FALSE;
		EndInverse();
#else
		if (!cmd_line)
#endif /* HAVE_COLOR */
		{
			cursoron();
			if (!ret)
				ClearScreen ();
		}
		EndWin ();
		Raw (FALSE);
#ifdef SIGUSR1
	} else {
		ret = SIGUSR1;
	}
#endif /* SIGUSR1 */
	cleanup_tmp_files ();

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

#ifdef VMS
	if (!ret)
		ret = 1;
	vms_close_stdin (); /* free resources used by ReadCh */
#endif /* VMS */

	exit (ret);
}


/*
 * strip_double_ngs ()
 * Strip duplicate newsgroups from within a given list of comma
 * separated groups
 *
 * 14-Jun-'96 Sven Paulus <sven@oops.sub.de>
 *
 */
void
strip_double_ngs (
	char *ngs_list)
{
	char *ptr;			/* start of next (outer) newsgroup */
	char *ptr2;			/* temporary pointer */
	char ngroup1[HEADER_LEN];	/* outer newsgroup to compare */
	char ngroup2[HEADER_LEN];	/* inner newsgroup to compare */
	char cmplist[HEADER_LEN];	/* last loops output */
	char newlist[HEADER_LEN];	/* the newly generated list without */
										/* any duplicates of the first nwsg */
	int ncnt1;			/* counter for the first newsgroup */
	int ncnt2;			/* counter for the second newsgroup */
	t_bool over1;		/* TRUE when the outer loop is over */
	t_bool over2;		/* TRUE when the inner loop is over */

	/* shortcut, check if there is only 1 group */
	if (strchr(ngs_list, ',') != (char *) 0) {
		over1 = FALSE;
		ncnt1 = 0;
		strcpy(newlist, ngs_list);		/* make a "working copy" */
		ptr = newlist;						/* the next outer newsg. is the 1st */

		while (!over1) {
			ncnt1++;							/* inc. outer counter */
			strcpy(cmplist, newlist);	/* duplicate groups for inner loop */
			ptr2 = strchr(ptr, ',');	/* search "," ... */
			if (ptr2 != (char *) 0) {	/* if found ... */
				*ptr2 = '\0';
				strcpy(ngroup1, ptr);	/* chop off first outer newsgroup */
				ptr = ptr2 + 1;			/* pointer points to next newsgr. */
			} else {							/* ... if not: last group */
				over1 = TRUE;				/* wow, everything is done after . */
				strcpy(ngroup1, ptr);	/* ... this last outer newsgroup */
			}

			over2 = FALSE;
			ncnt2 = 0;

			/*
			 * now compare with each inner newsgroup on the list,
			 * which is behind the momentary outer newsgroup
			 * if it is different from the outer newsgroup, append
			 * to list, strip double-commas
			 */

			while (!over2) {
				ncnt2++;
				strcpy(ngroup2, cmplist);
				ptr2 = strchr(ngroup2, ',');
				if (ptr2 != (char *) 0) {
					strcpy(cmplist, ptr2+1);
					*ptr2 = '\0';
				} else
					over2 = TRUE;

				if ((ncnt2 > ncnt1) && (strcasecmp(ngroup1, ngroup2)) && (strlen(ngroup2) != 0)) {
					strcat(newlist, ",");
					strcat(newlist, ngroup2);
				}
			}
		}
		strcpy(ngs_list, newlist);	/* move string to its real location */
	}
}


int
my_mkdir (
	char *path,
	mode_t mode)
{
#ifndef HAVE_MKDIR
	char buf[LEN];
	struct stat sb;

	sprintf(buf, "mkdir %s", path); /* redirect stderr to /dev/null ? */
	if (stat (path, &sb) == -1) {
		system (buf);
		return chmod (path, mode);
	} else
		return -1;
#else
#	if defined(M_OS2) || defined(WIN32)
		return mkdir (path);
#	else
		return mkdir (path, mode);
#	endif /* M_OS2 || WIN32 */
#endif /* !HAVE_MKDIR */
}


int
my_chdir (
	char *path)
{
	int retcode;

	retcode = chdir (path);

#ifdef M_OS2
	if (*path && path[1] == ':') {
		_chdrive (toupper((unsigned char)path[0]) - 'A' + 1);
	}
#endif /* M_OS2 */

	return retcode;
}


#ifdef M_UNIX
void
rename_file (
	const char *old_filename,
	const char *new_filename)
{
	FILE *fp_old, *fp_new;

	unlink (new_filename);

#	ifdef HAVE_LINK
	if (link (old_filename, new_filename) == -1)
#	else
	if (rename (old_filename, new_filename) < 0)
#	endif /* HAVE_LINK */
	{
		if (errno == EXDEV) {	/* create & copy file across filesystem */
			if ((fp_old = fopen (old_filename, "r")) == (FILE *) 0) {
				perror_message (_(txt_cannot_open), old_filename);
				return;
			}
			if ((fp_new = fopen (new_filename, "w")) == (FILE *) 0) {
				perror_message (_(txt_cannot_open), new_filename);
				fclose (fp_old);
				return;
			}
			copy_fp (fp_old, fp_new);
			fclose (fp_new);
			fclose (fp_old);
			errno = 0;
		} else {
			perror_message (_(txt_rename_error), old_filename, new_filename);
			return;
		}
	}
#	ifdef HAVE_LINK
	if (unlink (old_filename) == -1) {
		perror_message (_(txt_rename_error), old_filename, new_filename);
		return;
	}
#	endif /* HAVE_LINK */
}
#endif /* M_UNIX */


#ifdef VMS
void
rename_file (
	char *old_filename,
	char *new_filename)
{
	char new_filename_vms[1024];

	if (!strchr(strchr(new_filename, ']') ? strchr(new_filename, ']') : new_filename, '.')) {
		/* without final dot the new filename is not as tin expects */
		if (strlen(new_filename) >= sizeof(new_filename_vms)) {
			perror_message ("length of %s is too large", new_filename);
			return;
		}
		strcpy(new_filename_vms, new_filename);
		strcat(new_filename_vms, ".");
		new_filename = &new_filename_vms[0];
	}

	if (rename(old_filename, new_filename))
		perror_message (_(txt_rename_error), old_filename, new_filename);
}
#endif /* VMS */


#ifdef M_AMIGA
/*
 * AmigaOS now has links. Better not to use them as not everybody has new ROMS
 */
void
rename_file (
	char *old_filename,
	char *new_filename)
{
	char buf[1024];

	unlink (new_filename);
	if (rename (old_filename, new_filename) == EOF)
		perror_message (_(txt_rename_error), old_filename, new_filename);

	return;
}
#endif /* M_AMIGA */


t_bool
invoke_cmd (
	const char *nam)
{
	int ret;
	t_bool save_cmd_line = cmd_line;

	if (!save_cmd_line) {
		EndWin ();
		Raw (FALSE);
	}
	set_signal_catcher (FALSE);

	TRACE(("called system(%s)", _nc_visbuf(nam)));
#ifdef USE_SYSTEM_STATUS
	system(nam);
	ret = system_status;
#else
	ret = system (nam);
#endif /* USE_SYSTEM_STATUS */
	TRACE(("return %d", ret));

	set_signal_catcher (TRUE);
	if (!save_cmd_line) {
		Raw (TRUE);
		InitWin ();
		need_resize = cYes;		/* Flag a redraw */
	}

	if (ret != 0)
		error_message (_(txt_command_failed), nam);

#ifdef VMS
	return ret != 0;
#else
	return ret == 0;
#endif /* VMS */
}


void
draw_percent_mark (
	long cur_num,
	long max_num)
{
	char buf[32]; /* should be big enough */
	int percent;

	if (NOTESLINES <= 0)
		return;

	if (cur_num <= 0 && max_num <= 0)
		return;

	clear_message();
	percent = (int) (cur_num * 100 / max_num);
	snprintf (buf, sizeof (buf) - 1, "%s(%d%%) [%ld/%ld]", _(txt_more), percent, cur_num, max_num);
	MoveCursor (cLINES, (cCOLS - (int) strlen (buf)) - (1 + BLANK_PAGE_COLS));
	StartInverse ();
	my_fputs (buf, stdout);
	my_flush ();
	EndInverse ();
}


void
base_name (
	char *fullpath,		/* argv[0] */
	char *program)		/* tin_progname is returned */
{
	size_t i;
#ifdef VMS
	char *cp;
#endif /* VMS */

	strcpy (program, fullpath);

	for (i = strlen (fullpath)-1; i; i--) {
#ifndef VMS
		if (fullpath[i] == SEPDIR)
#else
		if (fullpath[i] == ']')
#endif /* !VMS */
		{
			strcpy (program, fullpath+(i+1));
			break;
		}
	}
#ifdef M_OS2
	str_lwr (program);
#endif /* M_OS2 */
#ifdef VMS
	if ((cp = strrchr(program, '.')) != 0)
		*cp = '\0';
#endif /* VMS */
}


/*
 *  Return TRUE if new mail has arrived
 */
t_bool
mail_check (
	void)
{
#ifndef WIN32 /* No unified mail transport on WIN32 */
	const char *mailbox_name;
	struct stat buf;
#	ifdef M_AMIGA
	static long mbox_size = 0;
#	endif /* M_AMIGA */

	mailbox_name = get_val ("MAIL", mailbox);

#	ifdef M_AMIGA
	/*
	 * Since AmigaDOS does not distinguish between atime and mtime
	 * we have to find some other way to figure out if the mailbox
	 * was modified (to bad that Iain removed the mail_setup() and
	 * mail_check() scheme used prior to 1.30 260694 which worked also
	 * on AmigaDOS). (R. Luebke 10.7.1994)
	 */

	/* this is only a first try, but it seems to work :) */

	if (mailbox_name != 0) {
		if (stat (mailbox_name, &buf) >= 0) {
			if (buf.st_size > 0) {
				if (buf.st_size > mbox_size) {
					mbox_size = buf.st_size;
					return TRUE;
				} else
					/*
					 * at this point we have to calculate how much the
					 * mailbox has to grow until we say "new mail"
					 * Unfortunately, some MUAs write status information
					 * back to to the users mailbox. This is a size increase
					 * and would result in "new mail" if we only look for some
					 * size increase. The mbox_size calculation below works
					 * for me for some time now (I use AmigaELM).
					 * Probably there is a better method, if you know one
					 * you are welcome... :-)
					 * I think a constant offset is more accurate today,
					 * 1k is the average size of mail-headers alone in each
					 * message I receive. (obw)
					 */
					mbox_size = buf.st_size + 1024;
			} else
				mbox_size = 0;
		}
	}
#	else
	if (mailbox_name != 0 && stat (mailbox_name, &buf) >= 0 && buf.st_atime < buf.st_mtime && buf.st_size > 0)
		return TRUE;
#	endif /* M_AMIGA */
#endif /* !WIN32 */
	return FALSE;
}

#if 0
/*
 * Returns the user name and E-mail address of the user
 *
 * Written by ahd 15 July 1989
 * Borrowed from UUPC/extended with some mods by nms
 * Rewritten from scratch by Th. Quinot, 1997-01-03
 */

#	ifdef lint
static int once;
#		define ONCE while(once)
#	else
#		define ONCE while(0)
#	endif /* lint */

#	define APPEND_TO(dest, src) do { \
	(void) sprintf ((dest), "%s", (src)); \
	(dest) = strchr((dest), '\0'); \
	} ONCE
#	define RTRIM(whatbuf, whatp) do { (whatp)--; \
	while ((whatp) >= (whatbuf) && \
	(*(whatp) == ' ')) \
	*((whatp)--) = '\0'; } ONCE
#	define LTRIM(whatbuf, whatp) for ((whatp) = (whatbuf); \
	(whatp) && (*(whatp) == ' '); \
	(whatp)++)
#	define TRIM(whatbuf, whatp) do { RTRIM ((whatbuf), (whatp)); \
	LTRIM ((whatbuf), (whatp)); \
	} ONCE

void
parse_from (
	char *addr,
	char *addrspec,
	char *comment)
{
	char atom_buf[HEADER_LEN];
	char quoted_buf[HEADER_LEN];

	char *atom_p = atom_buf;
	char *quoted_p = quoted_buf;

	char asbuf[HEADER_LEN];
	char cmtbuf[HEADER_LEN];

	char *ap = addr,
		 *asp = asbuf,
		*cmtp = cmtbuf;
	unsigned int state = 0;
/*
 * 0 = fundamental, 1 = in quotes, 2 = escaped in quotes,
 * 3 = in angle brackets, 4 = in parentheses
 */

	unsigned int plevel = 0;
	/* Parentheses nesting level */

	unsigned int atom_type = 0;
	/* 0 = unknown, 1 = address */

	*asp = *cmtp = '\0';
	for (; *ap; ap++) {
		switch (state) {
			case 0 :
				switch (*ap) {
					case '\"' :
						*atom_p = '\0';
						quoted_p = quoted_buf;
						*(quoted_p++) = '\"';
						state = 1;
						break;
					case '<' :
						*atom_p = '\0';
						APPEND_TO (cmtp, atom_buf);
						atom_p = atom_buf;
						atom_type = 0;
						asp = asbuf;
						state = 3;
						break;
					case '(' :
						*atom_p = '\0';
						APPEND_TO (asp, atom_buf);
						atom_p = atom_buf;
						atom_type = 0;
						plevel++;
						state = 4;
						break;
					case ' ' : case '\t' :
						if (atom_type == 1) {
							*atom_p = '\0';
							APPEND_TO (asp, atom_buf);
							atom_p = atom_buf;
							atom_type = 0;
						} else
							*(atom_p++) = *ap;
						break;
					default :
						*(atom_p++) = *ap;
						break;
				}
				break;
			case 1 :
				if (*ap == '\"') {
					switch (*(ap + 1)) {
						case '@' : case '%' :
							*(quoted_p++) = '\"'; *quoted_p = '\0';
							APPEND_TO (asp, quoted_buf);
							APPEND_TO (cmtp, atom_buf);
							atom_type = 1;
							break;
						default :
							*quoted_p = '\0';
							APPEND_TO (asp, atom_buf);
							APPEND_TO (cmtp, quoted_buf + 1);
							break;
					}
					state = 0;
					break;
				} else if (*ap == '\\')
					state = 2;
				*(quoted_p++) = *ap;
				break;
			case 2 :
				*(quoted_p++) = *ap;
				state = 1;
				break;
			case 3 :
				if (*ap == '>') {
					*asp = '\0';
					state = 0;
				} else
					*(asp++) = *ap;
				break;
			case 4 :
				switch (*ap) {
					case ')' :
						if (!--plevel) {
							*cmtp = '\0';
							state = 0;
						} else
							*(cmtp++) = *ap;
						break;
					case '(' :
						plevel++;
						nobreak; /* FALLTHROUGH */
					default :
						*(cmtp++) = *ap;
					break;
				}
				break;
			default :
				/* Does not happen. */
				goto FATAL;
		}
	}
	*cmtp = *asp = *atom_p = '\0';
	if (!state) {
		if ((atom_type == 1) || !*asbuf) {
			APPEND_TO (asp, atom_buf);
		} else {
			APPEND_TO (cmtp, atom_buf);
		}
	}
	/* Address specifier */
	TRIM (asbuf, asp);
	/* Comment */
	TRIM (cmtbuf, cmtp);
	strcpy (addrspec, asp);
	strcpy (comment, cmtp);
	return;
FATAL:
	strcpy(addrspec, "error@hell");
	*comment = '\0';
}
#	undef APPEND_TO
#	undef RTRIM
#	undef LTRIM
#	undef TRIM

#endif /* 0 */

/*
 *  Return a pointer into s eliminating any leading Re:'s.  Example:
 *
 *	  Re: Reorganization of misc.jobs
 *	  ^   ^
 *    Re^2: Reorganization of misc.jobs
 *
 *  now also strips trailing (was: ...) (obw)
 */
char *
eat_re (
	char *s,
	t_bool eat_was)
{
	int data, slen;
	int offsets[6];
	int size_offsets = sizeof(offsets)/sizeof(int);

	do {
		slen = strlen(s);
		data = pcre_exec(strip_re_regex.re, strip_re_regex.extra, s, slen, 0, 0, offsets, size_offsets);
		if (offsets[0] == 0)
			s += offsets[1];
	} while (data > 0);

	if (eat_was) do {
		slen = strlen(s);
		data = pcre_exec(strip_was_regex.re, strip_was_regex.extra, s, slen, 0, 0, offsets, size_offsets);
		if (offsets[0] > 0)
			s[offsets[0]] = '\0';
	} while (data > 0);

	return s;
}


int
my_isprint (
	int c)
{
#ifndef NO_LOCALE
	/* use locale */
	return isprint(c);
#else
#	ifdef LOCAL_CHARSET
		/* use some conversation table */
		return (isprint(c) || (c >= 0x80 && c <= 0xff));
#	else
		/* assume iso-8859-1 */
		return (isprint(c) || (c >= 0xa0 && c <= 0xff));
#	endif /* LOCAL_CHARSET */
#endif /* !NO_LOCALE */
}


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
get_author (
	t_bool thread,
	struct t_article *art,
	char *str, size_t len)
{
	int author;

	author = ((thread && !show_subject) ? SHOW_FROM_BOTH : CURR_GROUP.attribute->show_author);

	switch (author) {
		case SHOW_FROM_NONE:
			str[0] = '\0';
			break;
		case SHOW_FROM_ADDR:
			strncpy (str, art->from, len);
			break;
		case SHOW_FROM_NAME:
			strncpy (str, (art->name ? art->name : art->from), len);
			break;
		case SHOW_FROM_BOTH:
			if (art->name)
				snprintf (str, len, "%s <%s>", art->name, art->from);
			else
				strncpy (str, art->from, len);
			break;
		default:
			break;
	}

	*(str + len) = '\0';				/* NULL terminate */
}


void
toggle_inverse_video (
	void)
{
	tinrc.inverse_okay = !tinrc.inverse_okay;
	if (tinrc.inverse_okay) {
#ifndef USE_INVERSE_HACK
		tinrc.draw_arrow = FALSE;
#endif /* !USE_INVERSE_HACK */
	} else {
		tinrc.draw_arrow = TRUE;
	}
}


void
show_inverse_video_status (
	void)
{
		info_message ((tinrc.inverse_okay ? _(txt_inverse_on) : _(txt_inverse_off)));
}


#ifdef HAVE_COLOR
t_bool
toggle_color (
	void)
{
#	ifdef USE_CURSES
	if (!has_colors()) {
		use_color = FALSE;
		info_message (_(txt_no_colorterm));
		return FALSE;
	} else
#	endif /* USE_CURSES */
		use_color = !use_color;

	return TRUE;
}


void
show_color_status (
	void)
{
	info_message ((use_color ? _(txt_color_on) : _(txt_color_off)));
}
#endif /* HAVE_COLOR */


/* moved from art.c */
#ifdef WIN32
	/* Don't want the overhead of windows.h */
	int kbhit(void);
#endif /* WIN32 */


/*
 * Check for lock file to stop multiple copies of tind or tin -U running
 * and if it does not exist create it so this is the only copy running
 */
void
create_index_lock_file (
	char *the_lock_file)
{
	char buf[64];
	FILE *fp;
	time_t epoch;
	struct stat sb;

	if (stat (the_lock_file, &sb) == 0) {
		if ((fp = fopen (the_lock_file, "r")) != (FILE *) 0) {
			fgets (buf, (int) sizeof(buf), fp);
			fclose (fp);
			error_message ("\n%s: Already started pid=[%d] on %s", tin_progname, atoi(buf), buf+8);
			giveup();
		}
	} else {
		if ((fp = fopen (the_lock_file, "w")) != (FILE *) 0) {
			(void) time (&epoch);
			fprintf (fp, "%6d  %s\n", (int) process_id, ctime (&epoch));
			if (ferror (fp) || fclose (fp))
				error_message (_(txt_filesystem_full), the_lock_file);
			else
				chmod (the_lock_file, (mode_t)(S_IRUSR|S_IWUSR));
		}
	}
}


/*
 * strfquote() - produce formatted quote string
 *   %A  Articles Email address
 *   %D  Articles Date
 *   %F  Articles Address+Name
 *   %G  Groupname of Article
 *   %M  Articles MessageId
 *   %N  Articles Name of author
 *   %C  First Name of author
 *   %I  Initials of author
 * Return number of characters written (???) or 0 on failure
 */
int
strfquote (
	const char *group,
	int respnum,
	char *s,
	size_t maxsize,
	char *format)
{
	char *endp = s + maxsize;
	char *start = s;
	int i, j;
	t_bool iflag;

	if (s == (char *) 0 || format == (char *) 0 || maxsize == 0)
		return 0;

	if (strchr (format, '%') == (char *) 0 && strlen (format) + 1 >= maxsize)
		return 0;

	for (; *format && s < endp - 1; format++) {
		char tbuf[LEN];

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
					strcpy (tbuf, "\n");
					break;
				case 't':	/* tab */
					strcpy (tbuf, "\t");
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
					strcpy (s, tbuf);
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
					if (arts[respnum].name != (char *) 0) {
						STRCPY(tbuf, arts[respnum].name);
						if (strrchr (arts[respnum].name, ' '))
							*(strrchr (tbuf, ' ')) = '\0';
					} else {
						STRCPY(tbuf, arts[respnum].from);
					}
					break;

				case 'D':	/* Articles Date */
					STRCPY(tbuf, BlankIfNull(pgart.hdr.date));
					break;

				case 'F':	/* Articles Address+Name */
					if (arts[respnum].name)
						snprintf (tbuf, sizeof(tbuf) - 1 , "%s <%s>", arts[respnum].name, arts[respnum].from);
					else {
						STRCPY(tbuf, arts[respnum].from);
					}
					break;

				case 'G':	/* Groupname of Article */
					STRCPY(tbuf, group);
					break;

				case 'I':	/* Initials of author */
					STRCPY(tbuf, ((arts[respnum].name != (char *) 0) ? arts[respnum].name : arts[respnum].from));
					j = 0;
					iflag = TRUE;
					for (i = 0; tbuf[i]; i++) {
						if (iflag) {
							tbuf[j++] = tbuf[i];
							iflag = FALSE;
						}
						if (strchr(" ._@", tbuf[i]))
							iflag = TRUE;
					}
					tbuf[j] = '\0';
					break;

				case 'M':	/* Articles MessageId */
					STRCPY(tbuf, BlankIfNull(pgart.hdr.messageid));
					break;

				case 'N':	/* Articles Name of author */
					strcpy (tbuf, ((arts[respnum].name != (char *) 0) ? arts[respnum].name : arts[respnum].from));
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
					strcpy (s, tbuf);
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
strfeditor (
	char *editor,
	int linenum,
	const char *filename,
	char *s,
	size_t maxsize,
	char *format)
{
	char *endp = s + maxsize;
	char *start = s;
	char tbuf[PATH_LEN];
	int i;

	if (s == (char *) 0 || format == (char *) 0 || maxsize == 0)
		return 0;

	if (strchr (format, '%') == (char *) 0 && strlen (format) + 1 >= maxsize)
		return 0;

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
					strcpy (tbuf, "\n");
					break;
#ifdef WIN32
				case '\"':
					strcpy (tbuf, "\\\"");
					break;
#endif /* WIN32 */
				default:
					tbuf[0] = '%';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}
			i = strlen(tbuf);
			if (i) {
				if (s + i < endp - 1) {
					strcpy (s, tbuf);
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
					sprintf (tbuf, "%d", linenum);
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
					strcpy (s, tbuf);
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

	if ((i = strlen (tbuf))) {
		if (str + i < endp - 1) {
			strcpy (str, tbuf);
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
 *   +file     -> tinrc.savedir/group.name/file
 *
 * Interestingly, %G is not documented as such and apparently unused
 *   ~/News/%G -> $HOME/News/group.name
 *
 * FIXME:	allow =%G expansion
 *			(write all articles to a single file named $HOME/Mail/group.name)
 *			What's with this ? '=' on its own already does this
 *
 * Inputs:
 *   format		The string to be converted
 *   str		Return buffer
 *   maxsize	Size of str
 *   group		ptr to current group
 * Returns:
 *   0			on error
 *   1			if generated pathname is a mailbox
 *   2			success
 */
static int
_strfpath (
	const char *format,
	char *str,
	size_t maxsize,
	struct t_group *group)
{
	char *endp = str + maxsize;
	const char *startp = format;
	char defbuf[PATH_LEN];
	char *envptr;
	int i;
#ifndef M_AMIGA
	struct passwd *pwd;
#endif /* !M_AMIGA */
	t_bool is_mailbox = FALSE;

	if (str == (char *) 0 || format == (char *) 0 || maxsize == 0)
		return 0;

	if (strlen (format) + 1 >= maxsize)
		return 0;

	for (; *format && str < endp - 1; format++) {
		char tbuf[PATH_LEN];

		tbuf[0] = '\0';

		/*
		 * If just a normal part of the pathname copy it
		 */
#ifdef VMS
		if (!strchr ("~=+%", *format))
#else
		if (!strchr ("~$=+%", *format))
#endif /* VMS */
		{
			*str++ = *format;
			continue;
		}

		switch (*format) {
			case '~':			/* Users or another users homedir */
				switch (*++format) {
					case '/':	/* users homedir */
						joinpath (tbuf, homedir, "");
						break;
					default:	/* some other users homedir */
#ifndef M_AMIGA
						i = 0;
						while (*format && *format != '/')
							tbuf[i++] = *format++;
						tbuf[i] = '\0';
						/*
						 * OK lookup the username in /etc/passwd
						 */
						if ((pwd = getpwnam (tbuf)) == NULL) {
							str[0] = '\0';
							return 0;
						} else
							sprintf (tbuf, "%s/", pwd->pw_dir);
#else
						/* Amiga has no other users */
						return 0;
						/* NOTREACHED */
#endif /* !M_AMIGA */
						break;
				}
				if ((str = strfpath_cp(str, tbuf, endp)) == NULL)
					return 0;
				break;
#ifndef VMS
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
				envptr = getenv (tbuf);
				if (envptr == (char *) 0 || (*envptr == '\0'))
					strncpy (tbuf, defbuf, sizeof(tbuf)-1);
				else
					strncpy (tbuf, envptr, sizeof(tbuf)-1);
				if ((str = strfpath_cp(str, tbuf, endp)) == NULL)
					return 0;
				else if (*tbuf == '\0') {
					str[0] = '\0';
					return 0;
				}
				break;
#endif /* !VMS */
			case '=':
				/*
				 * Mailbox name expansion
				 * Only expand if 1st char in format
				 * =dir expands to maildir/dir
				 * =    expands to maildir/groupname
				 */
				is_mailbox = TRUE;

				if (startp == format && group != NULL) {
					char buf[PATH_LEN];

					if (strfpath (group->attribute->maildir, buf, sizeof(buf), group)) {
						if (*(format+1) == '\0')				/* Just an = */
							joinpath (tbuf, buf, group->name);
						else
							joinpath (tbuf, buf, "");
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
				 */
				if (startp == format && group != NULL) {
					char buf[PATH_LEN];

					/*
					 * Start with the savedir name
					 */
					if (strfpath (group->attribute->savedir, buf, sizeof(buf), group)) {
						char tmp[PATH_LEN];
#ifdef HAVE_LONG_FILE_NAMES
						my_strncpy (tmp, group->name, sizeof(tmp));
#else
						my_strncpy (tmp, group->name, 14);
#endif /* HAVE_LONG_FILE_NAMES */
						JOINPATH(tbuf, buf, tmp);	/* Add the group name */
						joinpath (tmp, tbuf, "");
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
				if (*format && *format == 'G') {
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
				nobreak; /* FALLTHROUGH */
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
strfpath (
	const char *format,
	char *str,
	size_t maxsize,
	struct t_group *group)
{
	/*
	 * Expand any leading env vars first in case they themselves contain
	 * formatting chars
	 */
	if (format[0] == '$') {
		char buf[PATH_LEN];

		if (_strfpath (format, buf, sizeof (buf), group))
			return (_strfpath (buf, str, maxsize, group));
	}

	return (_strfpath (format, str, maxsize, group));
}


enum quote_enum {
	no_quote = 0,
	dbl_quote,
	sgl_quote
};


char *
escape_shell_meta (
	char *source,
	int quote_area)
{
	static char buf[PATH_LEN];
	char *dest = buf;

	switch (quote_area) {
		case no_quote:
			while (*source) {
				if (*source == '\'' || *source == '\\' || *source == '"' ||
					*source == '$' || *source == '`' || *source == '*' ||
					*source == '&' || *source == '|' || *source == '<' ||
					*source == '>' || *source == ';' || *source == '(' ||
					*source == ')')
					*dest++ = '\\';
				*dest++ = *source++;
			}
			break;

		case dbl_quote:
			while (*source) {
				if (*source == '\\' || *source == '"' || *source == '$' ||
					*source == '`')
					*dest++ = '\\';
				*dest++ = *source++;
			}
			break;

		case sgl_quote:
			while (*source) {
				if (*source == '\'') {
					*dest++ = '\'';
					*dest++ = '\\';
					*dest++ = '\'';
				}
				*dest++ = *source++;
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
strfmailer (
	char *the_mailer,
	char *subject,
	char *to,
	const char *filename,
	char *dest,
	size_t maxsize,
	char *format)
{
	char *endp = dest + maxsize;
	char *start = dest;
	char tbuf[PATH_LEN];
	int quote_area = no_quote;

	/*
	 * safe guards: no destination to write to, no format, no space to
	 * write, or nothing to replace and format string longer than available
	 * space => return without any action
	 */
	if (dest == (char *) 0 || format == (char *) 0 || maxsize == 0)
		return 0;

	if (strchr (format, '%') == (char *) 0 && strlen (format) + 1 >= maxsize)
		return 0;

	/*
	 * walk through format until end of format or end of available space
	 * and replace place holders
	 */
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
					strcpy (tbuf, "\n");
					break;
				default:
					tbuf[0] = '\\';
					tbuf[1] = *format;
					tbuf[2] = '\0';
					break;
			}
			if (*tbuf) {
				if (sh_format (dest, endp - dest, "%s", tbuf) >= 0)
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
		 * - '%M' expands to mailer
		 * - '%S' expands to subject of message
		 * - '%T' expands to recipient(s) of message
		 * - '%U' expands to userid
		 * - '%' followed by any other character is copied literally
		 */
		if (*format == '%') {
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
					STRCPY(tbuf, the_mailer);
					break;
				case 'S':	/* Subject */
					/* don't MIME encode Subject if using external mail client */
					if (tinrc.use_mailreader_i)
						strncpy (tbuf, escape_shell_meta (subject, quote_area), sizeof(tbuf));
					else
						strncpy (tbuf, escape_shell_meta (rfc1522_encode (subject, ismail), quote_area), sizeof(tbuf));
					tbuf[sizeof(tbuf) - 1] = '\0';	/* just in case */
					escaped = TRUE;
					break;
				case 'T':	/* To */
					/* don't MIME encode To if using external mail client */
					if (tinrc.use_mailreader_i)
						strncpy (tbuf, escape_shell_meta (to, quote_area), sizeof(tbuf));
					else
						strncpy (tbuf, escape_shell_meta (rfc1522_encode (to, ismail), quote_area), sizeof(tbuf));
					tbuf[sizeof(tbuf) - 1] = '\0';	/* just in case */
					escaped = TRUE;
					break;
				case 'U':	/* User */
					/* don't MIME encode User if using external mail client */
					if (tinrc.use_mailreader_i)
						strncpy (tbuf, userid, sizeof(tbuf));
					else
						strncpy (tbuf, rfc1522_encode (userid, ismail), sizeof(tbuf));
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
				} else if (sh_format (dest, endp - dest, "%s", tbuf) >= 0) {
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
get_initials (
	int respnum,
	char *s,
	int maxsize) /* return value is always ignored */
{
	char tbuf[PATH_LEN];
	int i, j;
	t_bool iflag;

	if (s == (char *) 0 || maxsize == 0)
		return 0;

	strcpy (tbuf, ((arts[respnum].name != (char *) 0) ? arts[respnum].name : arts[respnum].from));

	iflag = FALSE;
	j = 0;
	for (i = 0; tbuf[i] && j < maxsize-1; i++) {
		if (isalpha((int)tbuf[i])) {
			if (!iflag) {
				s[j++] = tbuf[i];
				iflag = TRUE;
			}
		} else
			iflag = FALSE;
	}
	s[j] = '\0';
	return 0;
}


void get_cwd (
	char *buf)
{
#ifdef HAVE_GETCWD
	getcwd (buf, PATH_LEN);
#else
	getwd (buf);
#endif /* HAVE_GETCWD */
}


/*
 * Convert a newsgroup name to a newsspool path
 * No effect when reading via NNTP
 */
void
make_group_path (
	char *name,
	char *path)
{
#ifdef VMS
	sprintf(path, "[%s]", name);
#else
	while (*name) {
		*path = ((*name == '.') ? '/' : *name);
		name++;
		path++;
	}
	*path = '\0';
#endif /* VMS */
}


/*
 * Delete tmp index & local newsgroups file
 */
void
cleanup_tmp_files (
	void)
{
	char acNovFile[PATH_LEN];

	if (read_news_via_nntp && xover_supported && !tinrc.cache_overview_files) {
		sprintf (acNovFile, "%s%d.idx", TMPDIR, (int) process_id);
		unlink (acNovFile);
	}

	if (!tinrc.cache_overview_files)
		unlink (local_newsgroups_file);

	/*
	 * Even though batch_mode is turned off with -U, the child still has it set
	 */
	if (batch_mode)
		unlink (lock_file);
}

#ifndef M_UNIX
void
make_post_process_cmd (
	char *cmd,
	char *dir,
	char *file)
{
	char buf[LEN];
	char currentdir[PATH_LEN];

	get_cwd (currentdir);
	my_chdir (dir);
#	ifdef M_OS2
	backslash (file);
#	endif /* M_OS2 */
	sh_format (buf, sizeof(buf), cmd, file);
	invoke_cmd (buf);
	my_chdir (currentdir);
}
#endif /*! M_UNIX */


/*
 * returns filesize in bytes
 * -1 in case of an error (file not found, or !S_IFREG)
 */
long /* we use long here as off_t might be unsigned on some systems */
file_size (
	const char *file)
{
	struct stat statbuf;

	return (stat (file, &statbuf) == -1 ? -1L : (S_ISREG(statbuf.st_mode)) ? (long) statbuf.st_size : -1L);
}

/*
 * returns mtime
 * -1 in case of an error (file not found, or !S_IFREG)
 */
long /* we use long (not time_t) here for FILE_CHANGED() macro */
file_mtime (
	const char *file)
{
	struct stat statbuf;

	return (stat (file, &statbuf) == -1 ? -1L : (S_ISREG(statbuf.st_mode)) ? (long) statbuf.st_mtime : -1L);
}


void
vPrintBugAddress (
	void)
{
	my_fprintf (stderr, _("%s %s %s (\"%s\") [%s]: send a DETAILED bug report to %s\n"),
		tin_progname, VERSION, RELEASEDATE, RELEASENAME, OSNAME, bug_addr);
	my_fflush (stderr);
}


char *
random_organization (
	char *in_org)
{
	FILE *orgfp;
	int nool = 0, sol;
	static char selorg[512];

	*selorg = '\0';

	if (*in_org != '/')
		return in_org;

	srand ((unsigned int) time(NULL));

	if ((orgfp = fopen(in_org, "r")) == NULL)
		return selorg;

	/* count lines */
	while (fgets(selorg, (int) sizeof(selorg), orgfp))
		nool++;

	rewind (orgfp);
	sol = rand () % nool + 1;
	nool = 0;
	while ((nool != sol) && (fgets(selorg, (int) sizeof(selorg), orgfp)))
		nool++;

	fclose(orgfp);

	return selorg;
}


void
read_input_history_file (
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
		wait_message (0, _(txt_reading_input_history_file));

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
			hist_last[his_w] = his_free;
			hist_pos[his_w] = hist_last[his_w];
			his_free = his_e = 0;
			his_w++;
		}
		/* check if end is reached */
		if (his_w > HIST_MAXNUM)
			break;
	}
	fclose(fp);

	if (cmd_line)
		printf ("\r\n");
}


static void
write_input_history_file (
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
	file_tmp = get_tmpfilename (local_input_history_file);

	if ((fp = fopen(file_tmp, "w")) == NULL) {
		error_message (_(txt_filesystem_full_backup), local_input_history_file);
		/* free memory for tmp-filename */
		free (file_tmp);
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

	if (ferror (fp) || fclose (fp))
		error_message (_(txt_filesystem_full), local_input_history_file);
	else
		rename_file (file_tmp, local_input_history_file);
	umask(mask);
	/* fix modes for all pre 1.4.1 local_input_history_file files */
	chmod (local_input_history_file, (mode_t)(S_IRUSR|S_IWUSR));
	/* free memory for tmp-filename */
	free (file_tmp);
}


/*
 * quotes wildcards * ? \ [ ] with \
 */
char *
quote_wild (
	char *str)
{
	char *target;
	static char buff[2*LEN];	/* on the safe side */

	for (target = buff; *str != '\0'; str++) {
		if (tinrc.wildcard) { /* regex */
			/*
			 * quote meta characters ()[]{}\^$*+?.
			 * replace whitespace with '\s' (pcre)
			 */
			if (*str == '(' || *str == ')' || *str == '[' || *str == ']' || *str == '{' || *str == '}'
			    || *str == '\\' || *str == '^' || *str == '$'
			    || *str == '*' || *str == '+' || *str == '?' || *str == '.'
			    || *str == ' ' || *str == '\t') {
				*target++ = '\\';
				*target++ = ((*str == ' ' || *str == '\t')? 's' : *str);
			} else
				*target++ = *str;
		} else {	/* wildmat */
			if (*str == '*' || *str == '\\' || *str == '[' || *str == ']' || *str == '?')
				*target++ = '\\';
			*target++ = *str;
		}
	}
	*target = '\0';
	return (buff);
}


/*
 * quotes whitespace in regexps for pcre
 */
char *
quote_wild_whitespace (
	char *str)
{
	char *target;
	static char buff[2*LEN];	/* on the safe side */

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
	return (buff);
}



/*
 * strip_address () removes the address part from a given e-mail address
 */
void
strip_address (
	char *the_address,
	char *stripped_address)
{
	char *end_pos;
	char *start_pos;

	if (strchr(the_address, '@') != (char *) 0) {
		if ((end_pos = strchr(the_address,'<')) == (char *) 0) {
			if ((start_pos = strchr(the_address, ' ')) == (char *) 0)
				strcpy (stripped_address, the_address);
			else {
				strcpy (stripped_address, start_pos + 2);
				if (stripped_address[strlen(stripped_address) - 1] == ')')
					stripped_address[strlen(stripped_address) - 1] = '\0';
			}
		} else
			if (end_pos > the_address)
				strncpy (stripped_address, the_address, end_pos - the_address - 1);
			else
				strcpy (stripped_address, the_address);
	} else {
		if (the_address[0] == '(')
			strcpy (stripped_address, the_address + 1);
		else
			strcpy (stripped_address, the_address);
		if (stripped_address[strlen(stripped_address) - 1] == ')')
			stripped_address[strlen(stripped_address) - 1] = '\0';
	}
}


/*
 * strip_name () removes the realname part from a given e-mail address
 */
void
strip_name (
	char *the_address,
	char *stripped_address)
{
	char *end_pos;
	char *start_pos;

	/* skip realname in address */
	if ((start_pos = strchr (the_address, '<')) == (char *) 0) {
		/* address in user@domain (realname) syntax or realname is missing */
		strcpy (stripped_address, the_address);
		start_pos = stripped_address;
		if ((end_pos = strchr (start_pos, ' ')) == (char *) 0)
			end_pos = start_pos+strlen(start_pos);
	} else {
		start_pos++; /* skip '<' */
		strcpy (stripped_address, start_pos);
		start_pos = stripped_address;
		if ((end_pos = strchr (start_pos, '>')) == (char *) 0)
			end_pos = start_pos+strlen(start_pos); /* skip '>' */
	}
	*(end_pos) = '\0';
}


#ifdef LOCAL_CHARSET
/*
 * convert between local and network charset (e.g. NeXT and latin1)
 */

#	define CHARNUM 256
#	define BAD (-1)

/* use the appropriate conversion tables */
#	if LOCAL_CHARSET == 437
#		include "l1_ibm437.tab"
#		include "ibm437_l1.tab"
#	else
#		if LOCAL_CHARSET == 850
#			include "l1_ibm850.tab"
#			include "ibm850_l1.tab"
#		else
#			include "l1_next.tab"
#			include "next_l1.tab"
#		endif /* 850 */
#	endif /* 437 */

static int
to_local (
	int c)
{
	if (use_local_charset) {
		c = c_network_local[(unsigned char)c];
		if (c == BAD)
			return '?';
		else
			return c;
	} else
		return c;
}

void
buffer_to_local (
	char *b)
{
	for(; *b; b++)
		*b = to_local(*b);
}

static int
to_network (
	int c)
{
	if (use_local_charset) {
		c = c_local_network[(unsigned char) c];
		if (c == BAD)
			return '?';
		else
			return c;
	} else
		return c;
}

void
buffer_to_network (
	char *b)
{
	for(; *b; b++)
		*b = to_network(*b);
}
#endif /* LOCAL_CHARSET */


/*
 * checking of mail adresses for GNKSA compliance
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
/* 0x20 */ 0,0,0,1, 1,1,1,1, 0,0,1,1, 0,1,1,1,
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
 */
static char gnksa_legal_realname_chars[256] = {
/*         0 1 2 3  4 5 6 7  8 9 a b  c d e f */
/* 0x00 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
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
gnksa_strerror (
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
gnksa_dequote_plainphrase (
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
		if (!gnksa_legal_realname_chars[(int) *rpos])
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
gnksa_check_domain_literal (
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

	/* check for private ip or localhost */
	if ((!disable_gnksa_domain_check)
	    && ((0 == x1)				/* local network */
		|| (10 == x1)				/* private class A */
		|| ((172 == x1) && (16 == (x2 & 0xf0)))	/* private class B */
		|| ((192 == x1) && (168 == x2))		/* private class C */
		|| (127 == x1)))			/* localhost */
		return GNKSA_LOCAL_DOMAIN_LITERAL;


	return GNKSA_OK;
}


static int
gnksa_check_domain (
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
	if (('.' == *domain) || ('.' == *(domain+strlen(domain)-1)))
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
			nobreak; /* FALLTHROUGH */
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
		if (!gnksa_legal_fqdn_chars[(int) *aux])
			return GNKSA_INVALID_FQDN_CHAR;
	}

	return GNKSA_OK;
}


/*
 * check localpart of address
 */
static int
gnksa_check_localpart (
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
		if (!gnksa_legal_localpart_chars[(int) *aux])
			return GNKSA_INVALID_LOCALPART;
	}

	return GNKSA_OK;
}


/*
 * split mail address into realname and address parts
 */
static int
gnksa_split_from (
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
	strncpy(work, from, 998);
	work[998] = '\0';
	work[999] = '\0';

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

	/* split successful */
	return GNKSA_OK;
}


/*
 * restrictive check for valid address conforming to RFC 1036, son of RFC 1036
 * and draft-usefor-article-xx.txt
 */
int
gnksa_do_check_from (
	const char *from,
	char *address,
	char *realname)
{
	char *addr_begin;
	char *aux;
	char decoded[HEADER_LEN];
	int result = 0;
	int code;
	int addrtype;

	decoded[0] = '\0';

#ifdef DEBUG
	if (debug == 2)
		wait_message (0, "From:=[%s]", from);
#endif /* DEBUG */

	/* split from */
	code = gnksa_split_from(from, address, realname, &addrtype);
	if ('\0' == *address) /* address missing or not extractable */
		return code;

#ifdef DEBUG
	if (debug == 2)
		wait_message (0, "address=[%s]", address);
#endif /* DEBUG */

	/* parse address */
	addr_begin = strrchr(address, '@');
	if (NULL == addr_begin) {
		if (GNKSA_OK == code)
			code = result;
	} else {
		/* temporarily terminate string at separator position */
		*addr_begin++ = '\0';

#ifdef DEBUG
		if (debug == 2)
			wait_message (0, "FQDN=[%s]", addr_begin);
#endif /* DEBUG */

		/* convert FQDN part to lowercase */
		for (aux = addr_begin; *aux; aux++)
			*aux = tolower((int)*aux);

		if (GNKSA_OK != (result = gnksa_check_domain(addr_begin))
		    && (GNKSA_OK == code)) /* error detected */
			code = result;

		if (GNKSA_OK != (result = gnksa_check_localpart(address))
		    && (GNKSA_OK == code)) /* error detected */
			code = result;

		/* restore separator character */
		*--addr_begin= '@';
	}

#ifdef DEBUG
	if (debug == 2)
		wait_message (0, "realname=[%s]", realname);
#endif /* DEBUG */

	/* check realname */
	if (GNKSA_OK != (result = gnksa_dequote_plainphrase(realname, decoded, addrtype))) {
		if (GNKSA_OK == code) /* error detected */
			code = result;
	} else	/* copy dequoted realname to result variable */
		strcpy(realname, decoded);

#ifdef DEBUG
	if (debug == 2) {
		if (GNKSA_OK != code)
			wait_message (3, "From:=[%s], GNKSA=[%d]", from, code);
		else
			wait_message (0, "GNKSA=[%d]", code);
	}
#endif /* DEBUG */

	return code;
}


/*
 * check given address
 */
int
gnksa_check_from (
	char *from)
{
	char address[HEADER_LEN];	/* will be initialised in gnksa_split_from () */
	char realname[HEADER_LEN];	/* which is called by gnksa_do_check_from() */

	return gnksa_do_check_from(from, address, realname);
}


#if 1
/*
 * parse given address
 * return error code on GNKSA check failure
 */
int
parse_from (
	const char *from,
	char *address,
	char *realname)
{
	return gnksa_do_check_from(from, address, realname);
}
#endif /* 1 */


/*
 * Strip trailing blanks, tabs, \r and \n
 */
void
strip_line (
	char *line)
{
	char *ptr = line + strlen(line) - 1;

	while ((ptr >= line) && (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n'))
		ptr--;

	*++ptr = '\0';
}
