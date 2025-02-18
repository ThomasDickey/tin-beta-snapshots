/*
 *  Project   : tin - a Usenet reader
 *  Module    : nrctbl.c
 *  Author    : Sven Paulus <sven@tin.org>
 *  Created   : 1996-10-06
 *  Updated   : 2024-12-05
 *  Notes     : This module does the NNTP server name lookup in
 *              ~/.tin/newsrctable and returns the real hostname
 *              and the name of the newsrc file for a given
 *              alias of the server.
 *
 * Copyright (c) 1996-2025 Sven Paulus <sven@tin.org>
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */

#ifdef NNTP_ABLE
/*
 * local prototypes
 */
static void write_newsrctable_file(void);


/*
 * write_newsrctable_file()
 * create newsrctable file in local rc directory
 */
static void
write_newsrctable_file(
	void)
{
	FILE *fp;

	if ((fp = fopen(newsrctable_file, "w")) == NULL)
		return;

	fprintf(fp, _(txt_nrctbl_info), PRODUCT, VERSION);
#ifdef HAVE_FCHMOD
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));
#else
#	ifdef HAVE_CHMOD
	chmod(newsrctable_file, (mode_t) (S_IRUSR|S_IWUSR));
#	endif /* HAVE_CHMOD */
#endif /* HAVE_FCHMOD */
	fclose(fp);
}


/*
 * get_nntpserver()
 * returns the FQDN of NNTP server by looking up a given
 * nickname or alias in the newsrctable
 * ---> extend to allow nameserver-lookups, if search in table
 *      failed
 */
void
get_nntpserver(
	char *nntpserver_name,
	size_t nntpserver_name_len,
	char *nick_name)
{
	FILE *fp;
	char *line_entry;
	char line[LEN];
	char name_found[PATH_LEN];
	int line_entry_counter;
	t_bool found = FALSE;

	if ((fp = tin_fopen(newsrctable_file, "r")) != NULL) {
		while ((fgets(line, sizeof(line), fp) != NULL) && !found) {
			line_entry_counter = 0;

			if (!strchr("# ;", line[0])) {
				while ((line_entry = strtok(line_entry_counter ? NULL : line, " \t\n")) != NULL) {
					++line_entry_counter;

					if (line_entry_counter == 1)
						STRCPY(name_found, line_entry);

					if ((line_entry_counter > 2) && (!strcasecmp(line_entry, nick_name)))
						found = TRUE;
				}
			}
		}
		fclose(fp);
		strncpy(nntpserver_name, (found ? name_found : nick_name), nntpserver_name_len);
	} else {
		write_newsrctable_file();
		strncpy(nntpserver_name, nick_name, nntpserver_name_len);
	}
	nntpserver_name[nntpserver_name_len - 1] = '\0';
}


/*
 * get_newsrcname()
 * get name of newsrc file with given name of nntp server
 * returns TRUE if name was found, FALSE if the search failed
 */
t_bool
get_newsrcname(
	char *newsrc_name,
	size_t newsrc_name_len,
	const char *nntpserver_name) /* return value is always ignored */
{
	FILE *fp;

	if ((fp = tin_fopen(newsrctable_file, "r")) != NULL) {
		char line[LEN];
		char name_found[PATH_LEN] = { '\0' };
		char *ns = NULL, *nsp = NULL, *p, *q;
		char *line_entry;
		int found = 0;
		int line_entry_counter;
		size_t l = strlen(nntpserver_name) + 8; /* []:65535\0 */
		t_bool do_cpy = FALSE;

		if ((p = strchr(nntpserver_name, ':')) != NULL) {
			if ((q = strrchr(nntpserver_name, ':')) != NULL) {
				if (p != q) {
					ns = my_malloc(l);
					nsp = my_malloc(l);
					sprintf(ns, "[%s]", nntpserver_name);
					sprintf(nsp, "[%s]:%u", nntpserver_name, nntp_tcp_port);
				}
			}
		}
		if (!ns)
			ns = my_strdup(nntpserver_name);
		if (!nsp) {
			/* add [] and/or port if required */
			nsp = my_malloc(l);
			sprintf(nsp, "%s:%u", nntpserver_name, nntp_tcp_port);
		}

		while ((fgets(line, (int) sizeof(line), fp) != NULL) && (found != 1)) {
			if (strchr("# ;", line[0])) /* comment? */
				continue;

			line_entry_counter = 0;
			while ((line_entry = strtok(line_entry_counter ? NULL : line, " \t\n")) != NULL) {
				++line_entry_counter;

				if ((line_entry_counter == 1) && ((!strcasecmp(line_entry,nsp)) || (!strcasecmp(line_entry, ns)))) {
					found = 1;
					do_cpy = TRUE;
				}

				if (!found && (!strcasecmp(line_entry, "default") || (*line_entry == '*' && *(line_entry + 1) == '\0'))) {
					found = 2;
					do_cpy = TRUE;
				}

				if (do_cpy && (line_entry_counter == 2)) {
					STRCPY(name_found, line_entry);
					do_cpy = FALSE;
				}
			}
		}
		free(ns);
		free(nsp);
		fclose(fp);
		if (found) {
			char dir[PATH_LEN] = { '\0' };
			char tmp_newsrc[PATH_LEN];
			int error = 0;

			if (!strfpath(name_found, tmp_newsrc, sizeof(tmp_newsrc), NULL, FALSE)) {
				my_fprintf(stderr, _(txt_error_couldnt_expand), name_found);
				error = 1;
			} else {
				if (tmp_newsrc[0] == '/') {
					(void) strncpy(newsrc_name, tmp_newsrc, newsrc_name_len);
					newsrc_name[newsrc_name_len - 1] = '\0';
				} else
					joinpath(newsrc_name, newsrc_name_len, homedir, tmp_newsrc);

				STRCPY(dir, newsrc_name);
				if ((line_entry = strrchr(dir, '/'))) { /* dirname(newsrc_name) */
					while (line_entry > &dir[0] && *line_entry == '/')
						*line_entry-- = '\0';
				}

				if (access(dir, X_OK)) {
					if (errno != ENOENT) {
						my_fprintf(stderr, _(txt_error_no_enter_permission), dir);
						error = 1;
					} else {
						my_fprintf(stderr, _(txt_error_no_such_file), dir);
						error = 2;
					}
				} else if (access(newsrc_name, F_OK)) {
					my_fprintf(stderr, _(txt_error_no_such_file), newsrc_name);
					error = 2;
				} else if (access(dir, R_OK)) {
					my_fprintf(stderr, _(txt_error_no_read_permission), dir);
					error = 1;
				} else if (access(newsrc_name, R_OK)) {
					my_fprintf(stderr, _(txt_error_no_read_permission), newsrc_name);
					error = 1;
				} else if (access(dir, W_OK)) {
					my_fprintf(stderr, _(txt_error_no_write_permission), dir);
					error = 1;
				} else if (access(newsrc_name, W_OK)) {
					my_fprintf(stderr, _(txt_error_no_write_permission), newsrc_name);
					error = 1;
				}
			}

			if (error) {
				char ch, default_ch = 'a';

				do {
					/* very ugly code, but curses is not initialized yet */
					if (error >= 2) {
						default_ch = 'c';
						my_printf("%s%c\b", _(txt_nrctbl_create), default_ch);
					} else
						my_printf("%s%c\b", _(txt_nrctbl_default), default_ch);

					if ((ch = (char) ReadCh()) == '\r' || ch == '\n')
						ch = default_ch;
				} while (ch != ESC && ch != 'a' && ch != 'c' && ch != 'd' && ch != 'q');
				my_printf("%c\n", ch);

				/* NOTE: these keys can not be remapped */
				switch (ch) {
					case 'c':
						if (*dir) {
							errno = 0;
							(void) my_mkdir(dir, (mode_t) (S_IRWXU));
							if (!errno || errno == EEXIST)
								return TRUE;
							my_fprintf(stderr, "mkdir(%s): Error: %s\n", dir, strerror(errno));
						}

						no_write = TRUE;
						tin_done(EXIT_FAILURE, _(txt_cannot_create), *dir ? dir : "NULL");
						/* keep lint quiet: */
						/* NOTREACHED */
						break;

					case 'd':
						joinpath(newsrc_name, newsrc_name_len, homedir, ".newsrc");
						return TRUE;
						/* keep lint quiet: */
						/* NOTREACHED */
						break;

					case 'a':
						/*
						 * FIXME this (e.g. the location of the alternative name)
						 * is not documented in the man page
						 */
						snprintf(name_found, sizeof(name_found), ".newsrc-%s", nntpserver_name);
						joinpath(newsrc_name, newsrc_name_len, homedir, name_found);
						return TRUE;
						/* keep lint quiet: */
						/* NOTREACHED */
						break;

					case 'q':
						exit(EXIT_SUCCESS);
						/* keep lint quiet: */
						/* FALLTHROUGH */

					case ESC:
					default:
						return TRUE;
				}
			}
			return TRUE;
		}
	} else
		write_newsrctable_file();

	return FALSE;
}
#else
static void no_newsrctable(void);	/* proto-type */
static void
no_newsrctable(	/* ANSI C requires non-empty source file */
	void)
{
}
#endif /* NNTP_ABLE */
