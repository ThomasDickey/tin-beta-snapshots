/*
 *  Project   : tin - a Usenet reader
 *  Module    : mail.c
 *  Author    : I. Lea
 *  Created   : 1992-10-02
 *  Updated   : 1994-08-11
 *  Notes     : Mail handling routines for creating pseudo newsgroups
 *
 * Copyright (c) 1992-2000 Iain Lea <iain@bricbrac.de>
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
 *    This product includes software developed by Iain Lea.
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

static void read_groups_descriptions (FILE *fp, FILE *fp_save);

/*
 *  Load the mail active file into active[]
 */

#ifdef HAVE_MH_MAIL_HANDLING
void
read_mail_active_file (
	void)
{
	FILE *fp;
	char buf[LEN];
	char my_spooldir[PATH_LEN];
	long count = -1L;
	long min, max;
	struct t_group *ptr;

	if (INTERACTIVE)
		wait_message (0, _(txt_reading_mail_active_file));

	/*
	 * Open the mail active file
	 */
	if ((fp = open_mail_active_fp ("r")) == (FILE *) 0) {
		if (cmd_line)
			my_fputc ('\n', stderr);

		error_message (_(txt_cannot_open), mail_active_file);
		/*
		 * FIXME - maybe do an autoscan of maildir, create & do a reopen ?
		 */
		write_mail_active_file ();
		return;
	}

	while (fgets (buf, (int) sizeof (buf), fp) != (char *) 0) {
		if (!parse_active_line (buf, &max, &min, my_spooldir) || *buf == '\0')
			continue;

		/*
		 * Load mailgroup into group hash table
		 */
		if ((ptr = psGrpAdd (buf)) == NULL)
			continue;

		/*
		 * Load group info. TODO - integrate with active_add()
		 */
		ptr->aliasedto = (char *) 0;
		ptr->description = (char *) 0;
		ptr->spooldir = my_strdup (my_spooldir);
		ptr->moderated = 'y';
		ptr->count = count;
		ptr->xmax = max;
		ptr->xmin = min;
		ptr->type = GROUP_TYPE_MAIL;
		ptr->inrange = FALSE;
		ptr->read_during_session = FALSE;
		ptr->art_was_posted = FALSE;
		ptr->subscribed = FALSE;		/* not in my_group[] yet */
		ptr->newgroup = FALSE;
		ptr->bogus = FALSE;
		ptr->next = -1;			/* hash chaining */
		ptr->newsrc.xbitmap = (t_bitmap *) 0;
		ptr->attribute = (struct t_attribute *) 0;
		ptr->glob_filter = &glob_filter;
		vSetDefaultBitmap (ptr);
	}
	fclose (fp);

	if (INTERACTIVE)
		my_fputs("\n", stdout);
}


/*
 *  Write out mailgroups from active[] to ~/.tin/active.mail
 */

void
write_mail_active_file (
	void)
{
	FILE *fp;
	char acGrpPath[PATH_LEN];
	register int i;
	struct t_group *psGrp;

	if (no_write && file_size (mail_active_file) != -1)
		return;

	vPrintActiveHead (mail_active_file);

	if ((fp = open_mail_active_fp ("a+")) != (FILE *) 0) {
		for (i = 0; i < num_active; i++) {
			psGrp = &active[i];
			if (psGrp->type == GROUP_TYPE_MAIL) {
				vMakeGrpPath (psGrp->spooldir, psGrp->name, acGrpPath);
				vFindArtMaxMin (acGrpPath, &psGrp->xmax, &psGrp->xmin);
				vPrintGrpLine (fp, psGrp->name, psGrp->xmax, psGrp->xmin, psGrp->spooldir);
			}
		}
		fclose (fp);
	}
}


/*
 *  Load the text description from ~/.tin/mailgroups for each mail group into
 *  the active[] array.
 */
void
read_mailgroups_file (
	void)
{
	FILE *fp;

	if (!show_description || save_news || catchup)
		return;

	if ((fp = open_mailgroups_fp ()) != (FILE *) 0) {
		if (INTERACTIVE)
			wait_message (0, _(txt_reading_mailgroups_file));

		read_groups_descriptions (fp, (FILE *) 0);

		fclose (fp);

		if (INTERACTIVE)
			my_fputs("\n", stdout);
	}
}
#endif /* HAVE_MAIL_HANDLING */


/*
 *  Load the text description from NEWSLIBDIR/newsgroups for each group into the
 *  active[] array. Save a copy locally if reading via NNTP to save bandwidth.
 */
void
read_newsgroups_file (
	void)
{
	FILE *fp;
	FILE *fp_save = (FILE *) 0;

	if (!show_description || save_news || catchup)
		return;

	if ((fp = open_newsgroups_fp ()) != (FILE *) 0) {
		if (INTERACTIVE)
			wait_message (0, _(txt_reading_newsgroups_file));


		if (read_news_via_nntp && !read_local_newsgroups_file && !no_write)
			fp_save = fopen (local_newsgroups_file, "w" FOPEN_OPTS);

		read_groups_descriptions (fp, fp_save);

		if (fp_save != (FILE *) 0) {
			fclose (fp_save);
			read_local_newsgroups_file = TRUE;
		}

		TIN_FCLOSE (fp);

		if (INTERACTIVE)
			my_fputs("\n", stdout);
	}
}


/*
 *  Read groups descriptions from opened file & make local backup copy
 *  of all groups that don't have a 'x' in the active file moderated
 *  field & if reading groups of type GROUP_TYPE_NEWS.
 *  Aborting this early won't have any adverse affects, just some missing
 *  descriptions.
 */
static void
read_groups_descriptions (
	FILE *fp,
	FILE *fp_save)
{
	char *ptr;
	char *p, *q;
	char group[PATH_LEN];
	int count = 0;
	struct t_group *psGrp;

	while ((ptr = tin_fgets (fp, FALSE)) != (char *) 0) {
		if (*ptr == '#' || *ptr == '\0')
			continue;

/*
 *  This was moved from below and simplified.  I can't test here for the
 *  type of group being read, because that requires having found the
 *  group in the active file, and that truncates the local copy of the
 *  newsgroups file to only subscribed-to groups when tin is called with
 *  the "-q" option.
 */
		if ((fp_save != (FILE *) 0) && read_news_via_nntp && !read_local_newsgroups_file)
			fprintf (fp_save, "%s\n", ptr);

		for (p = ptr, q = group ; *p && *p != ' ' && *p != '\t' ; p++, q++)
			*q = *p;

		*q = '\0';

		while (*p == '\t' || *p == ' ')
			p++;

		psGrp = psGrpFind (group);

		if (psGrp != (struct t_group *) 0 && psGrp->description == (char *) 0) {
			q = p;
			while ((q = strchr (q, '\t')) != (char *) 0)
				*q = ' ';

			psGrp->description = my_strdup (p);

#	if 0 /* not useful for cache_overview_files */
			if (psGrp->type == GROUP_TYPE_NEWS) {
				if (fp_save != (FILE *) 0 && read_news_via_nntp && !read_local_newsgroups_file)
					fprintf (fp_save, "%s\n", ptr);
			}
#	endif /* 0 */
		}

		if (++count % 100 == 0)
			spin_cursor ();

	}
}


void
vPrintActiveHead (
	char *pcActiveFile)
{
	FILE *hFp;

	if (no_write && file_size (pcActiveFile) != -1)
		return;

	if ((hFp = fopen (pcActiveFile, "w")) != (FILE *) 0) {
		/* FIXME: -> lang.c */
		fprintf (hFp, _("# [Mail/Save] active file. Format is like news active file:\n"));
		fprintf (hFp, _("#   groupname  max.artnum  min.artnum  /dir\n"));
		fprintf (hFp, _("# The 4th field is the basedir (ie. ~/Mail or ~/News)\n#\n"));
		fclose (hFp);
	}
}


void
vFindArtMaxMin (
	char *pcGrpPath,
	long *plArtMax,
	long *plArtMin)
{
	DIR *tDirFile;
	DIR_BUF *tFile;
	long lArtNum;

	*plArtMin = *plArtMax = 0L;

	if (access (pcGrpPath, R_OK) != 0) {
		*plArtMin = 1L;
		return;
	}

	tDirFile = opendir (pcGrpPath);
	if (tDirFile != (DIR *) 0) {
		while ((tFile = readdir (tDirFile)) != (DIR_BUF *) 0) {
			lArtNum = atol (tFile->d_name);
			if (lArtNum >= 1) {
				if (lArtNum > *plArtMax) {
					*plArtMax = lArtNum;
					if (*plArtMin == 0)
						*plArtMin = lArtNum;
				} else if (lArtNum < *plArtMin)
					*plArtMin = lArtNum;
			}
		}
		CLOSEDIR(tDirFile);
	}
	if (*plArtMin == 0)
		*plArtMin = 1;
}


void
vPrintGrpLine (
	FILE *hFp,
	char *pcGrpName,
	long lArtMax,
	long lArtMin,
	char *pcBaseDir)
{
	fprintf (hFp, "%s %05ld %05ld %s\n",
		pcGrpName, lArtMax, lArtMin, pcBaseDir);
}


/*
 * Given a base pathname & a newsgroup name build an absolute pathname.
 * base = /usr/spool/news
 * newsgroup = alt.sources
 * absolute path = /usr/spool/news/alt/sources
 */
void
vMakeGrpPath (
	char *pcBaseDir,
	char *pcGrpName,
	char *pcGrpPath)
{
	char *pcPtr;

	joinpath (pcGrpPath, pcBaseDir, pcGrpName);

	pcPtr = pcGrpPath + strlen (pcBaseDir);
	while ((pcPtr = strchr (pcPtr, '.')) != (char *) 0)
		*pcPtr = '/';
}


/*
 * Given an absolute pathname & a base pathname build a newsgroup name
 * base = /usr/spool/news
 * absolute path = /usr/spool/news/alt/sources
 * newsgroup = alt.sources
 */
void
vMakeGrpName (
	char *pcBaseDir,
	char *pcGrpName,
	char *pcGrpPath)
{
	char *pcPtrBase;
	char *pcPtrName;
	char *pcPtrPath;

	pcPtrBase = pcBaseDir;
	pcPtrPath = pcGrpPath;

	while (*pcPtrBase && (*pcPtrBase == *pcPtrPath)) {
		pcPtrBase++;
		pcPtrPath++;
	}
	strcpy (pcGrpName, ++pcPtrPath);

	pcPtrName = pcGrpName;
	while ((pcPtrName = strchr (pcPtrName, '/')) != (char *) 0)
		*pcPtrName = '.';
}


void
vGrpDelMailArt (
	struct t_article *psArt)
{

	if (psArt->delete_it) {
		art_mark_undeleted (psArt);
		info_message (_(txt_art_undeleted));
	} else {
		art_mark_deleted (psArt);
		info_message (_(txt_art_deleted));
	}
}


void
vGrpDelMailArts (
	struct t_group *psGrp)
{
	char acArtFile[PATH_LEN];
	char acGrpPath[PATH_LEN];
	int iNum;
	struct t_article *psArt;
#if 0 /* see comment below */
	t_bool bUpdateIndexFile = FALSE;
#endif /* 0 */

	if (psGrp->type == GROUP_TYPE_MAIL || psGrp->type == GROUP_TYPE_SAVE) {
		wait_message (1, (psGrp->type == GROUP_TYPE_MAIL) ? _(txt_processing_mail_arts) : _(txt_processing_saved_arts));
		vMakeGrpPath (psGrp->spooldir, psGrp->name, acGrpPath);
		for (iNum = 0; iNum < top_art; iNum++) {
			psArt = &arts[iNum];
			if (psArt->delete_it) {
				sprintf (acArtFile, "%s/%ld", acGrpPath, psArt->artnum);
				unlink (acArtFile);
				psArt->thread = ART_EXPIRED;
#if 0 /* see comment below */
				bUpdateIndexFile = TRUE;
#endif /* 0 */
			}
		}

#if 0
/*
 * current tin's build_references() is changed to free msgid and refs,
 * therefore we cannot call vWriteNovFile after it. I simply commented
 * out this codes, NovFile will update at next time.
 */
/*
 * MAYBE also check if min / max article was deleted. If so then update
 * the active[] entry for the group and rewrite the mail.active file
 */
		if (bUpdateIndexFile)
			vWriteNovFile (psGrp);
#endif /* 0 */
	}
}


/* TODO test this function */
t_bool
iArtEdit (
	struct t_group *psGrp,
	struct t_article *psArt)
{
	FILE *fpin, *fpout;
	char acArtFile[PATH_LEN];
	char acTmpFile[PATH_LEN];
	t_bool ret = FALSE;

	/*
	 * Check if news / mail group
	 */
	if (psGrp->type == GROUP_TYPE_NEWS)
		return FALSE;

	vMakeGrpPath (psGrp->spooldir, psGrp->name, acTmpFile);
	sprintf (acArtFile, "%s/%ld", acTmpFile, psArt->artnum);
	sprintf (acTmpFile, "%s%d.art", TMPDIR, (int) process_id);

	if ((fpin = fopen (acArtFile, "r")) != NULL) {
		if ((fpout = fopen (acTmpFile, "w")) != NULL) {
			if (copy_fp (fpin, fpout)) {
				if (invoke_editor (acTmpFile, 1)) {
					rename_file (acTmpFile, acArtFile);
					ret = TRUE;
				} else
					unlink (acTmpFile);
			}
			fclose(fpout);
		}
		fclose(fpin);
	}
	return ret;
}
