/*
 *  Project   : tin - a Usenet reader
 *  Module    : nrctbl.c
 *  Author    : Sven Paulus <sven@tin.org>
 *  Created   : 1996-10-06
 *  Updated   : 2005-07-02
 *  Notes     : This module does the NNTP server name lookup in
 *              ~/.tin/newsrctable and returns the real hostname
 *              and the name of the newsrc file for a given
 *              alias of the server.
 *
 * Copyright (c) 1996-2006 Sven Paulus <sven@tin.org>
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */


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

	if ((fp = fopen(local_newsrctable_file, "w")) == NULL)
		return;

	fprintf(fp, _(txt_nrctbl_info), PRODUCT, VERSION);
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));
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
	char *nick_name)
{
	FILE *fp;
	char *line_entry;
	char line[LEN];
	char name_found[PATH_LEN];
	int line_entry_counter;
	t_bool found = FALSE;

	if ((fp = fopen(local_newsrctable_file, "r")) != NULL) {
		while ((fgets(line, sizeof(line), fp) != NULL) && !found) {
			line_entry_counter = 0;

			if (!strchr("# ;", line[0])) {
				while ((line_entry = strtok(line_entry_counter ? NULL : line, " \t\n")) != NULL) {
					line_entry_counter++;

					if (line_entry_counter == 1)
						strcpy(name_found, line_entry);

					if ((line_entry_counter > 2) && (!strcasecmp(line_entry, nick_name)))
						found = TRUE;
				}
			}
		}
		fclose(fp);
		strcpy(nntpserver_name, (found ? name_found : nick_name));
	} else {
		write_newsrctable_file();
		strcpy(nntpserver_name, nick_name);
	}
}


/*
 * get_newsrcname()
 * get name of newsrc file with given name of nntp server
 * returns TRUE if name was found, FALSE if the search failed
 */
int
get_newsrcname(
	char *newsrc_name,
	const char *nntpserver_name) /* return value is always ignored */
{
	FILE *fp;
	char *line_entry;
	char line[LEN];
	char name_found[PATH_LEN];
	int line_entry_counter;
	int found = 0;
	t_bool do_cpy = FALSE;

	if ((fp = fopen(local_newsrctable_file, "r")) != NULL) {
		while ((fgets(line, (int) sizeof(line), fp) != NULL) && (found != 1)) {
			line_entry_counter = 0;

			if (!strchr("# ;", line[0])) {
				while ((line_entry = strtok(line_entry_counter ? NULL : line, " \t\n")) != NULL) {
					line_entry_counter++;

					if ((line_entry_counter == 1) && (!strcasecmp(line_entry, nntpserver_name))) {
						found = 1;
						do_cpy = TRUE;
					}

					if ((line_entry_counter == 1) && ((!strcasecmp(line_entry, "default")) || (!strcmp(line_entry, "*")))) {
						found = 2;
						do_cpy = TRUE;
					}
					if (do_cpy && (line_entry_counter == 2)) {
						strcpy(name_found, line_entry);
						do_cpy = FALSE;
					}
				}
			}
		}
		fclose(fp);
		if (found) {
			char dir[PATH_LEN];
			char tmp_newsrc[PATH_LEN];
			int error = 0;

			if (!strfpath(name_found, tmp_newsrc, sizeof(tmp_newsrc), NULL)) {
					my_fprintf(stderr, _("couldn't expand %s\n"), name_found); /* TODO: -> lang.c */
					error = 1;
			} else {
				if (tmp_newsrc[0] == '/')
					(void) strcpy(newsrc_name, tmp_newsrc);
				else
					joinpath(newsrc_name, homedir, tmp_newsrc);
			}
			(void) strcpy(dir, newsrc_name);
			if (strchr(dir, '/'))
				*strrchr(dir, '/') = (char) 0;

			if (!error) {
			/* FIXME - write a global permssion check routine */
				if (access(dir, X_OK)) {
					my_fprintf(stderr, _(txt_error_no_enter_permission), dir);
					error = 1;
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
						printf("%s%c\b", _(txt_nrctbl_create), default_ch);
					} else
						printf("%s%c\b", _(txt_nrctbl_default), default_ch);

					if ((ch = (char) ReadCh()) == '\r' || ch == '\n')
						ch = default_ch;
				} while (ch != ESC && ch != 'a' && ch != 'c' && ch != 'd' && ch != 'q');
				printf("%c\n", ch);

				/* NOTE: these keys can not be remapped */
				switch (ch) {
					case 'c':
						/* FIXME this doesn't check if we could create the file */
						return TRUE;

					case 'd':
						joinpath(newsrc_name, homedir, ".newsrc");
						return TRUE;

					case 'a':
						/*
						 * FIXME this (e.g. the location of the alternativce name)
						 * is not documented in the man page
						 */
						snprintf(name_found, sizeof(name_found), ".newsrc-%s", nntpserver_name);
						joinpath(newsrc_name, homedir, name_found);
						return TRUE;

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
