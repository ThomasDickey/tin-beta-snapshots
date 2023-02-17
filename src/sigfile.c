/*
 *  Project   : tin - a Usenet reader
 *  Module    : sigfile.c
 *  Author    : M. Gleason & I. Lea
 *  Created   : 1992-10-17
 *  Updated   : 2021-07-26
 *  Notes     : Generate random signature for posting/mailing etc.
 *
 * Copyright (c) 1992-2023 Mike Gleason
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

#define MAXLOOPS 1000

#define CURRENTDIR "."

static char sigfile[PATH_LEN];

static FILE *open_random_sig(char *sigdir);
static int thrashdir(char *sigdir);


void
msg_write_signature(
	FILE *fp,
	t_bool include_dot_signature,
	struct t_group *thisgroup)
{
	FILE *fixfp;
	FILE *sigfp;
	char cwd[PATH_LEN];
	char path[PATH_LEN];
	char pathfixed[PATH_LEN];

#ifdef NNTP_INEWS
	if (read_news_via_nntp && !strcasecmp(tinrc.inews_prog, INTERNAL_CMD))
		include_dot_signature = TRUE;
#endif /* NNTP_INEWS */

	if (thisgroup && !thisgroup->bogus) {
		if (!strcmp(thisgroup->attribute->sigfile, "--none"))
			return;

		/*
		 * TODO: handle DONT_HAVE_PIPING case
		 *       use strfpath()?
		 */
#ifndef DONT_HAVE_PIPING
		if (thisgroup->attribute->sigfile[0] == '!') {
			FILE *pipe_fp;
			char *sigcmd, *sigattr, *ptr;
			char cmd[PATH_LEN];

			fprintf(fp, "\n%s", thisgroup->attribute->sigdashes ? SIGDASHES : "");
			sigattr = thisgroup->attribute->sigfile + 1;

			if ((ptr = strstr(sigattr, "%G"))) {
				char *to, *grpname;
				int cnt = 1;

				/* check if %G occurs more than once */
				while (strstr(++ptr, "%G"))
					++cnt;

				/* sigattr - (cnt * '%G') + (cnt * '"groupname"') + '\0' */
				sigcmd = my_malloc(strlen(sigattr) + (size_t) cnt * strlen(thisgroup->name) + 1);
				to = sigcmd;

				while (*sigattr) {
					if (*sigattr == '%' && *(sigattr + 1) == 'G') {
						grpname = thisgroup->name;
						*to++ = '"';
						while (*grpname)
							*to++ = *grpname++;
						*to++ = '"';
						sigattr += 2;
					} else
						*to++ = *sigattr++;
				}

				*to = '\0';
			} else {
				sigcmd = my_malloc(strlen(sigattr) + 1);
				sprintf(sigcmd, "%s", sigattr);
			}

			if ((pipe_fp = popen(sigcmd, "r")) != NULL) {
				while (fgets(cmd, PATH_LEN, pipe_fp))
					fputs(cmd, fp);
				pclose(pipe_fp);
			} /* else issue an error-message? */
			free(sigcmd);

			return;
		}
#endif /* !DONT_HAVE_PIPING */
		get_cwd(cwd);

		if (!strfpath(thisgroup->attribute->sigfile, path, sizeof(path), thisgroup, FALSE)) {
			if (!strfpath(tinrc.sigfile, path, sizeof(path), thisgroup, FALSE))
				joinpath(path, sizeof(path), homedir, ".Sig");
		}

		/*
		 * Check to see if sigfile is a directory & if it is
		 * generate a random signature from sigs in sigdir. If
		 * the file path/.sigfixed or ~/.sigfixed exists (fixed
		 * part of random sig) then read it in first and append
		 * the random sig part onto the end.
		 */
		if ((sigfp = open_random_sig(path)) != NULL) {
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				error_message(2, "USING random sig=[%s]", sigfile);
#endif /* DEBUG */
			fprintf(fp, "\n%s", thisgroup->attribute->sigdashes ? SIGDASHES : "");
			joinpath(pathfixed, sizeof(pathfixed), path, ".sigfixed");
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				error_message(2, "TRYING fixed sig=[%s]", pathfixed);
#endif /* DEBUG */
			if ((fixfp = fopen(pathfixed, "r")) != NULL) {
				copy_fp(fixfp, fp);
				fclose(fixfp);
			} else {
				joinpath(pathfixed, sizeof(pathfixed), homedir, ".sigfixed");
#ifdef DEBUG
				if (debug & DEBUG_MISC)
					error_message(2, "TRYING fixed sig=[%s]", pathfixed);
#endif /* DEBUG */
				if ((fixfp = fopen(pathfixed, "r")) != NULL) {
					copy_fp(fixfp, fp);
					fclose(fixfp);
				}
			}
			copy_fp(sigfp, fp);
			fclose(sigfp);
			if (chdir(cwd) == -1) {
#ifdef DEBUG
				int e = errno;
				if (debug & DEBUG_MISC)
					error_message(2, "chdir(%s): Error: %s", cwd, strerror(e));
#endif /* DEBUG */
			}
			return;
		}

		if ((sigfp = fopen(path, "r")) != NULL) {
			fprintf(fp, "\n%s", thisgroup->attribute->sigdashes ? SIGDASHES : "");
			copy_fp(sigfp, fp);
			fclose(sigfp);
			return;
		}

		/*
		 * Use ~/.signature as a last resort, but only if mailing or
		 * using internal inews (external inews appends it automagically).
		 */
		if ((sigfp = fopen(default_signature, "r")) != NULL) {
			if (include_dot_signature) {
				fprintf(fp, "\n%s", thisgroup->attribute->sigdashes ? SIGDASHES : "");
				copy_fp(sigfp, fp);
			}
			fclose(sigfp);
		}
	}
}


static FILE *
open_random_sig(
	char *sigdir)
{
	srand((unsigned int) time(NULL));

	if (chdir(sigdir) == 0) {
		if (thrashdir(sigdir) || !*sigfile) {
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				error_message(2, "NO sigfile=[%s]", sigfile);
#endif /* DEBUG */
			return (FILE *) 0;
		} else {
#ifdef DEBUG
			if (debug & DEBUG_MISC)
				error_message(2, "sigfile=[%s]", sigfile);
#endif /* DEBUG */
			return fopen(sigfile, "r");
		}
	}

	return (FILE *) 0;
}


static int
thrashdir(
	char *sigdir)
{
	DIR *dirp;
	DIR_BUF *dp;
	char *cwd;
	int safeguard, recurse;
	int c = 0, numentries = 0, pick;
	struct stat st;

	sigfile[0] = '\0';

	if ((dirp = opendir(CURRENTDIR)) == NULL)
		return 1;

	while (readdir(dirp) != NULL)
		numentries++;

	/*
	 * consider "." and ".." non-entries
	 * consider all entries starting with "." non-entries
	 */
	if (numentries < 3) {
		CLOSEDIR(dirp);
		return -1;
	}

	cwd = my_malloc(PATH_LEN);

	get_cwd(cwd);
	recurse = strcmp(cwd, sigdir);

	/*
	 * If we are using the root sig directory, we don't want
	 * to recurse, or else we might use a custom sig intended
	 * for a specific newsgroup (and not this one).
	 */
	for (safeguard = 0, dp = NULL; safeguard < MAXLOOPS && dp == NULL; safeguard++) {
#ifdef DEBUG
		if (debug & DEBUG_MISC)
			error_message(2, "sig loop=[%d] recurse=[%d]", safeguard, recurse);
#endif /* DEBUG */
#ifdef HAVE_REWINDDIR
		rewinddir(dirp);
#else
		CLOSEDIR(dirp);
		if ((dirp = opendir(CURRENTDIR)) == NULL) {
			free(cwd);
			return 1;
		}
#endif /* HAVE_REWINDDIR */
		pick = rand() % numentries + 1;
		while (--pick >= 0) {
			if ((dp = readdir(dirp)) == NULL)
				break;
		}
		if (dp != NULL) {	/* if we could open the dir entry */
			if (!strcmp(dp->d_name, CURRENTDIR) || (dp->d_name[0] == '.'))
				dp = NULL;
			else {	/* if we have a non-dot entry */
				if (stat(dp->d_name, &st) == -1) {
					CLOSEDIR(dirp);
					free(cwd);
					return 1;
				}
				if (S_ISDIR(st.st_mode)) {
					if (recurse) {
						/*
						 * do subdirectories
						 */
						if ((chdir(dp->d_name) < 0) || ((c = thrashdir(sigdir)) == 1)) {
							CLOSEDIR(dirp);
							free(cwd);
							return 1;
						}
						if (c == -1) {
							/*
							 * the one we picked was an
							 * empty dir so try again.
							 */
							dp = NULL;
							if (chdir(cwd) == -1) {
#ifdef DEBUG
								int e = errno;
								if (debug & DEBUG_MISC)
									error_message(2, "chdir(%s): Error: %s", cwd, strerror(e));
#endif /* DEBUG */
							}
						}
					} else
						dp = NULL;
				} else {	/* end dir; we have a file */
					get_cwd(sigfile);
					strcat(sigfile, "/");
					strcat(sigfile, dp->d_name);
#ifdef DEBUG
					if (debug & DEBUG_MISC)
						error_message(2, "Found a file=[%s]", sigfile);
#endif /* DEBUG */
				}
			}
		}
	}
	free(cwd);
#ifdef DEBUG
	if (debug & DEBUG_MISC)
		error_message(2, "return 0: sigfile=[%s]", sigfile);
#endif /* DEBUG */
	CLOSEDIR(dirp);

	return 0;
}
