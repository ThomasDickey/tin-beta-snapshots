/*
 *  Project   : tin - a Usenet reader
 *  Module    : memory.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2002-04-11
 *  Notes     :
 *
 * Copyright (c) 1991-2002 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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
#ifndef RFC2046_H
#	include "rfc2046.h"
#endif /* !RFC2046_H */

/*
 * Dynamic arrays maximum & current sizes
 * num_* values are one past top of used part of array
 */
int max_active = 0;
int num_active = -1;
int max_newnews = 0;
int num_newnews = 0;
int max_art = 0;
int max_save = 0;
int num_save = 0;

/*
 * Dynamic arrays
 */
int *my_group;				/* .newsrc --> active[] */
long *base;				/* base articles for each thread */
struct t_group *active;			/* active newsgroups */
struct t_newnews *newnews;		/* active file sizes on differnet servers */
struct t_article *arts;			/* articles headers in current group */
struct t_save *save;			/* sorts articles before saving them */

/*
 * Local prototypes
 */
static void free_newnews_array (void);
static void free_active_arrays (void);
static void free_if_not_default (char **attrib, char *deflt);


/*
 * Dynamic table management
 * These settings are memory conservative: small initial allocations
 * and a 50% expansion on table overflow. A fast vm system with
 * much memory might want to start with higher initial allocations
 * and a 100% expansion on overflow, especially for the arts[] array.
 */
void
init_alloc (
	void)
{
	/*
	 * active file arrays
	 */
	max_active = get_active_num();
	max_newnews = DEFAULT_NEWNEWS_NUM;

	active = my_malloc (sizeof(*active) * max_active);
	newnews =  my_malloc (sizeof(*newnews) * max_newnews);
	my_group = my_calloc (1, sizeof(int) * max_active);

	/*
	 * article headers array
	 */
	max_art = DEFAULT_ARTICLE_NUM;

	arts = my_malloc (sizeof(*arts) * max_art);
	base = my_malloc (sizeof(long) * max_art);

	/*
	 * save file array
	 */
	max_save = DEFAULT_SAVE_NUM;

	save = my_malloc (sizeof(*save) * max_save);

#ifndef USE_CURSES
	screen = (struct t_screen *) 0;
#endif /* !USE_CURSES */
}


void
expand_art (
	void)
{
	max_art += max_art / 2;		/* increase by 50% */
	arts = my_realloc (arts, sizeof(*arts) * max_art);
	base = my_realloc (base, sizeof(long) * max_art);
}


void
expand_active (
	void)
{
	max_active += max_active / 2;		/* increase by 50% */
	if (active == NULL) {
		active = my_malloc (sizeof (*active) * max_active);
		my_group = my_calloc (1, sizeof(int) * max_active);
	} else {
		active = my_realloc(active, sizeof (*active) * max_active);
		my_group = my_realloc(my_group, sizeof (int) * max_active);
	}
}


void
expand_save (
	void)
{
	max_save += max_save / 2;		/* increase by 50% */
	save = my_realloc(save, sizeof (struct t_save) * max_save);
}


void
expand_newnews (
	void)
{
	max_newnews += max_newnews / 2;			/* increase by 50% */
	newnews = my_realloc(newnews, sizeof(struct t_newnews) * max_newnews);
}


#ifndef USE_CURSES
void
init_screen_array (
	t_bool allocate)
{
	int i;

	if (allocate) {
		screen = my_malloc (sizeof (struct t_screen) * cLINES + 1);

		for (i = 0; i < cLINES; i++)
			screen[i].col = my_malloc ((size_t)(cCOLS + 2));
	} else {
		if (screen != NULL) {
			for (i = 0; i < cLINES; i++)
				FreeAndNull(screen[i].col);

			free (screen);
			screen = (struct t_screen *) 0;
		}
	}
}
#endif /* !USE_CURSES */


void
free_all_arrays (
	void)
{
	hash_reclaim ();

#ifndef USE_CURSES
	init_screen_array (FALSE);
#endif /* !USE_CURSES */

	free_art_array ();
	free_msgids ();

	if (arts != NULL) {
		free (arts);
		arts = (struct t_article *) 0;
	}

	free_filter_array (&glob_filter);
	free_active_arrays ();

#ifdef HAVE_COLOR
	FreeIfNeeded(quote_regex.re);
	FreeIfNeeded(quote_regex.extra);
	FreeIfNeeded(quote_regex2.re);
	FreeIfNeeded(quote_regex2.extra);
	FreeIfNeeded(quote_regex3.re);
	FreeIfNeeded(quote_regex3.extra);
#endif /* HAVE_COLOR */
	FreeIfNeeded(strip_re_regex.re);
	FreeIfNeeded(strip_re_regex.extra);
	FreeIfNeeded(strip_was_regex.re);
	FreeIfNeeded(strip_was_regex.extra);
	FreeIfNeeded(uubegin_regex.re);
	FreeIfNeeded(uubegin_regex.extra);
	FreeIfNeeded(uubody_regex.re);
	FreeIfNeeded(uubody_regex.extra);
	FreeIfNeeded(url_regex.re);
	FreeIfNeeded(url_regex.extra);
	FreeIfNeeded(mail_regex.re);
	FreeIfNeeded(mail_regex.extra);
	FreeIfNeeded(news_regex.re);
	FreeIfNeeded(news_regex.extra);
	FreeIfNeeded(shar_regex.re);
	FreeIfNeeded(shar_regex.extra);

	if (base != NULL) {
		free (base);
		base = (long *) 0;
	}

	if (save != NULL) {
		free_save_array ();
		if (save != NULL) {
			free (save);
			save = (struct t_save *) 0;
		}
	}

	if (newnews != NULL) {
		free_newnews_array ();
		if (newnews != NULL) {
			free (newnews);
			newnews = (struct t_newnews *) 0;
		}
	}

	free_keymaps ();
}


void
free_art_array (
	void)
{
	register int i;

	for_each_art(i) {
		arts[i].artnum = 0L;
		arts[i].thread = ART_EXPIRED;
		arts[i].inthread = FALSE;
		arts[i].status = ART_UNREAD;
		arts[i].killed = FALSE;
		arts[i].tagged = 0;
		arts[i].selected = FALSE;
		arts[i].date = (time_t) 0;

		FreeAndNull(arts[i].part);
		FreeAndNull(arts[i].patch);
		FreeAndNull(arts[i].xref);

		/* .refs & .msgid are cleared in build_references() */
		arts[i].refs = (char *) '\0';
		arts[i].msgid = (char *) '\0';
	}
}


/*
 * Use this only for attributes that have a fixed default of a static string
 * in tinrc
 */
static void
free_if_not_default (
	char **attrib,
	char *deflt)
{
	/* Can't see how these attribs can be = NULL */
	if (*attrib != NULL && *attrib != deflt) {
		free (*attrib);
		*attrib = (char *) 0;
	}
}


void
free_attributes_array (
	void)
{
	register int i;
	struct t_group *psGrp;

	for_each_group(i) {
		psGrp = &active[i];
		if (psGrp->attribute && !psGrp->attribute->global) {
			free_if_not_default(&psGrp->attribute->maildir, tinrc.maildir);
			free_if_not_default(&psGrp->attribute->savedir, tinrc.savedir);

			FreeAndNull(psGrp->attribute->savefile);

			free_if_not_default(&psGrp->attribute->sigfile, tinrc.sigfile);
			free_if_not_default(&psGrp->attribute->organization, default_organization);

			FreeAndNull(psGrp->attribute->followup_to);

			FreeAndNull(psGrp->attribute->quick_kill_scope);
			FreeAndNull(psGrp->attribute->quick_select_scope);

			FreeAndNull(psGrp->attribute->mailing_list);
			FreeAndNull(psGrp->attribute->x_headers);
			FreeAndNull(psGrp->attribute->x_body);

			free_if_not_default(&psGrp->attribute->from, tinrc.mail_address);
			free_if_not_default(&psGrp->attribute->news_quote_format, tinrc.news_quote_format);
			free_if_not_default(&psGrp->attribute->quote_chars, tinrc.quote_chars);

#ifdef HAVE_ISPELL
			FreeAndNull(psGrp->attribute->ispell);
#endif /* HAVE_ISPELL */

			free (psGrp->attribute);
		}
		psGrp->attribute = (struct t_attribute *) 0;
	}
}


static void
free_active_arrays (
	void)
{
	register int i;

	if (my_group != NULL) {			/* my_group[] */
		free (my_group);
		my_group = (int *) 0;
	}

	if (active != NULL) {		/* active[] */
		for_each_group(i) {
			FreeAndNull(active[i].name);
			FreeAndNull(active[i].description);
			FreeAndNull(active[i].aliasedto);

			if (active[i].type == GROUP_TYPE_MAIL && active[i].spooldir != NULL) {
				free (active[i].spooldir);
				active[i].spooldir = (char *) 0;
			}
			if (active[i].newsrc.xbitmap != 0) {
				free (active[i].newsrc.xbitmap);
				active[i].newsrc.xbitmap = 0;
			}
		}

		free_attributes_array ();

		if (active != NULL) {
			free (active);
			active = (struct t_group *) 0;
		}
	}
	num_active = -1;
}


void
free_save_array (
	void)
{
	int i;

	for (i = 0; i < num_save; i++) {
		FreeAndNull(save[i].path);
		/* file does NOT need to be freed */
		save[i].file = NULL;
		save[i].artptr = NULL;
		save[i].saved = FALSE;
		save[i].is_mailbox = FALSE;
	}
	num_save = 0;
}


static void
free_newnews_array (
	void)
{
	int i;

	for (i = 0; i < num_newnews; i++)
		FreeAndNull(newnews[i].host);

	num_newnews = 0;
}


void *
my_malloc1 (
	const char *file,
	int line,
	size_t size)
{
	void *p;

#ifdef DEBUG
	vDbgPrintMalloc (TRUE, file, line, size);
#endif /* DEBUG */

	if ((p = malloc (size)) == NULL) {
		error_message (txt_out_of_memory, tin_progname, size, file, line);
		giveup();
	}
	return p;
}


/*
 * TODO: add fallback code with malloc(nmemb*size);memset(0,nmemb*size)?
 */
void *
my_calloc1 (
	const char *file,
	int line,
	size_t nmemb,
	size_t size)
{
	void *p;

#ifdef DEBUG
	vDbgPrintMalloc (TRUE, file, line, nmemb * size);
#endif /* DEBUG */

	if ((p = calloc (nmemb, size)) == NULL) {
		error_message (txt_out_of_memory, tin_progname, nmemb * size, file, line);
		giveup();
	}
	return p;
}


void *
my_realloc1 (
	const char *file,
	int line,
	void *p,
	size_t size)
{
#ifdef DEBUG
	vDbgPrintMalloc (FALSE, file, line, size);
#endif /* DEBUG */

	p = ((!p) ? (calloc (1, size)) : realloc (p, size));

	if (p == NULL) {
		error_message (txt_out_of_memory, tin_progname, size, file, line);
		giveup();
	}
	return p;
}
