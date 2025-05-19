/*
 *  Project   : tin - a Usenet reader
 *  Module    : memory.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2025-05-14
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


/*
 * Dynamic arrays maximum (initialized in init_alloc()) & current sizes
 * num_* values are one past top of used part of array
 */
int max_active;
int num_active = -1;
int max_newnews;
int num_newnews = 0;
int max_art;
int max_base;
int max_save;
int num_save = 0;
int max_scope;
int num_scope = -1;

/*
 * Dynamic arrays
 */
int *my_group;				/* .newsrc --> active[] */
t_artnum *base;				/* base articles for each thread */
struct t_group *active;			/* active newsgroups */
struct t_scope *scopes = NULL;	/* attributes stores in .tin/attributes */
struct t_newnews *newnews;		/* active file sizes on different servers */
struct t_article *arts;			/* articles headers in current group */
struct t_save *save;			/* sorts articles before saving them */

/*
 * Local prototypes
 */
static void free_active_arrays(void);
static void free_attributes(struct t_attribute *attributes);
static void free_cmdline_args(void);
static void free_scopes_arrays(void);
static void free_tinrc_strings(void);
static void free_newnews_array(void);
static void free_if_not_default(char ***attrib, char **deflt);
static void free_input_history(void);


/*
 * Dynamic table management
 * These settings are memory conservative: small initial allocations
 * and a 50% expansion on table overflow. A fast vm system with
 * much memory might want to start with higher initial allocations
 * and a 100% expansion on overflow, especially for the arts[] array.
 */
void
init_alloc(
	void)
{
	/*
	 * active file arrays
	 */
	max_active = DEFAULT_ACTIVE_NUM;
	max_newnews = DEFAULT_NEWNEWS_NUM;

	active = my_malloc(sizeof(*active) * (size_t) max_active);
	newnews = my_malloc(sizeof(*newnews) * (size_t) max_newnews);
	my_group = my_calloc((size_t) max_active, sizeof(int));

	/*
	 * article headers array
	 */
	max_art = DEFAULT_ARTICLE_NUM;
	max_base = DEFAULT_ARTICLE_NUM;

	arts = my_calloc((size_t) max_art, sizeof(*arts));
	base = my_malloc(sizeof(t_artnum) * (size_t) max_base);

	ofmt = my_calloc(9, sizeof(*ofmt));	/* initial number of overview fields */

	/*
	 * save file array
	 */
	max_save = DEFAULT_SAVE_NUM;

	save = my_malloc(sizeof(*save) * (size_t) max_save);

	/*
	 * scope array
	 */
	max_scope = DEFAULT_SCOPE_NUM;
	expand_scope();

#ifndef USE_CURSES
	screen = (struct t_screen *) 0;
#endif /* !USE_CURSES */
}


void
expand_art(
	void)
{
	int i = max_art;

	max_art += max_art >> 1;		/* increase by 50% */
	arts = my_realloc(arts, sizeof(*arts) * (size_t) max_art);
		/*
		 * memset(&arts[i].artnum, 0, (max_art - i - 1) * sizeof(*arts));
		 * seems to be not faster at all
		 */
	for (; i < max_art; i++) {
		arts[i].subject = arts[i].xref = arts[i].path = arts[i].refs = arts[i].msgid = arts[i].mailbox.from = arts[i].mailbox.name = NULL;
		arts[i].mailbox.next = NULL;
	}
}


void
expand_active(
	void)
{
	max_active += max_active >> 1;		/* increase by 50% */
	if (active == NULL) {
		active = my_malloc(sizeof(*active) * (size_t) max_active);
		my_group = my_calloc((size_t) max_active, sizeof(int));
	} else {
		active = my_realloc(active, sizeof(*active) * (size_t) max_active);
		my_group = my_realloc(my_group, sizeof(int) * (size_t) max_active);
	}
}


void
expand_base(
	void)
{
	max_base += max_base >> 1;		/* increase by 50% */
	base = my_realloc(base, sizeof(t_artnum) * (size_t) max_base);
}


void
expand_save(
	void)
{
	max_save += max_save >> 1;		/* increase by 50% */
	save = my_realloc(save, sizeof(struct t_save) * (size_t) max_save);
}


void
expand_scope(
	void)
{
	if ((scopes == NULL) || (num_scope < 0)) {
		if (scopes == NULL)
			scopes = my_malloc(sizeof(*scopes) * (size_t) max_scope);
		num_scope = 0;
	} else {
		max_scope += max_scope >> 1;	/* increase by 50% */
		scopes = my_realloc(scopes, sizeof(*scopes) * (size_t) max_scope);
	}
}


void
expand_newnews(
	void)
{
	max_newnews += max_newnews >> 1;			/* increase by 50% */
	newnews = my_realloc(newnews, sizeof(struct t_newnews) * (size_t) max_newnews);
}


#ifndef USE_CURSES
void
init_screen_array(
	t_bool allocate)
{
	int i;

	if (allocate) {
		screen = my_malloc(sizeof(struct t_screen) * (size_t) cLINES + 1);

		for (i = 0; i < cLINES; i++) {
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			screen[i].col = my_malloc((size_t) cCOLS * MB_CUR_MAX + 2);
#	else
			screen[i].col = my_malloc((size_t) cCOLS + 2);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}
	} else {
		if (screen != NULL) {
			for (i = 0; i < cLINES; i++)
				FreeAndNull(screen[i].col);

			FreeAndNull(screen);
		}
	}
}
#endif /* !USE_CURSES */


void
free_all_arrays(
	void)
{
	hash_reclaim();

#ifndef USE_CURSES
	if (!batch_mode)
		init_screen_array(FALSE);
#endif /* !USE_CURSES */

	free_art_array();
	free_msgids();
	FreeAndNull(arts);
	free_filter_array(&glob_filter);
	free_active_arrays();
	free_scopes_arrays();
	free_tinrc_strings();

#ifdef HAVE_COLOR
	regex_cache_destroy(&quote_regex);
	regex_cache_destroy(&quote_regex2);
	regex_cache_destroy(&quote_regex3);
	regex_cache_destroy(&extquote_regex);
#endif /* HAVE_COLOR */
	regex_cache_destroy(&slashes_regex);
	regex_cache_destroy(&stars_regex);
	regex_cache_destroy(&strokes_regex);
	regex_cache_destroy(&underscores_regex);
	regex_cache_destroy(&strip_re_regex);
	regex_cache_destroy(&strip_was_regex);
	regex_cache_destroy(&uubegin_regex);
	regex_cache_destroy(&uubody_regex);
	regex_cache_destroy(&xxbody_regex);
	regex_cache_destroy(&verbatim_begin_regex);
	regex_cache_destroy(&verbatim_end_regex);
	regex_cache_destroy(&yencbegin_regex);
	regex_cache_destroy(&yencpart_regex);
	regex_cache_destroy(&yencend_regex);
	regex_cache_destroy(&url_regex);
	regex_cache_destroy(&mail_regex);
	regex_cache_destroy(&news_regex);
	regex_cache_destroy(&shar_regex);
	regex_cache_destroy(&shar_end_regex);
	regex_cache_destroy(&hideline_regex);

	if (!batch_mode) {
		free_keymaps();
		free_input_history();
	}

	FreeAndNull(base);

	if (save != NULL) {
		free_save_array();
		FreeAndNull(save);
	}

	if (newnews != NULL) {
		free_newnews_array();
		FreeAndNull(newnews);
	}

	FreeAndNull(nntp_caps.headers_range);
	FreeAndNull(nntp_caps.headers_id);
	FreeAndNull(nntp_caps.implementation);
	FreeAndNull(nntp_caps.sasl_mechs);
	FreeAndNull(nntp_caps.sasl_mech_used);

	if (ofmt) { /* ofmt might not be allocated yet on early abort */
		int i;

		for (i = 0; ofmt[i].name; i++)
			free(ofmt[i].name);
		free(ofmt);
	}

	tin_fgets(NULL, FALSE);
	rfc1522_decode(NULL);

	free(backup_article_name);
	free_cmdline_args();
	free(tin_progname);
}


void
free_art_array(
	void)
{
	int i;

	for_each_art(i) {
		arts[i].artnum = T_ARTNUM_CONST(0);
		arts[i].date = (time_t) 0;
		FreeAndNull(arts[i].xref);
		free_mailbox_list(arts[i].mailbox.next);

		/*
		 * .refs & .msgid are usually free()d in build_references()
		 * nevertheless we try to free() it here in case tin_done()
		 * was called before build_references()
		 */
		FreeAndNull(arts[i].refs);
		FreeAndNull(arts[i].msgid);

		arts[i].tagged = 0;
		arts[i].thread = ART_EXPIRED;
		arts[i].prev = ART_NORMAL;
		arts[i].status = ART_UNREAD;
		arts[i].killed = ART_NOTKILLED;
		arts[i].selected = FALSE;
	}
}


/*
 * Use this for attributes that have a default in tinrc
 * otherwise use (char **) 0 as deflt
 */
static void
free_if_not_default(
	char ***attrib,
	char **deflt)
{
	if (*attrib && *attrib != deflt) {
		FreeAndNull(**attrib);
		FreeAndNull(*attrib);
	}
}


/*
 * Free memory of one attributes struct only
 */
static void
free_attributes(
	struct t_attribute *attributes)
{
	free_if_not_default(&attributes->group_format, &tinrc.group_format);
	free_if_not_default(&attributes->thread_format, &tinrc.thread_format);
	free_if_not_default(&attributes->date_format, &tinrc.date_format);
	free_if_not_default(&attributes->editor_format, &tinrc.editor_format);
	free_if_not_default(&attributes->fcc, (char **) 0);
	free_if_not_default(&attributes->from, &tinrc.mail_address);
	free_if_not_default(&attributes->followup_to, (char **) 0);
#ifdef HAVE_ISPELL
	free_if_not_default(&attributes->ispell, (char **) 0);
#endif /* HAVE_ISPELL */
	free_if_not_default(&attributes->maildir, &tinrc.maildir);
	free_if_not_default(&attributes->mailing_list, (char **) 0);
	free_if_not_default(&attributes->mime_types_to_save, &default_mime_types_to_save);
	free_if_not_default(&attributes->news_headers_to_display, &tinrc.news_headers_to_display);
	free_if_not_default(&attributes->news_headers_to_not_display, &tinrc.news_headers_to_not_display);
	if (attributes->headers_to_display) {
		if (attributes->headers_to_display->header)
			FreeIfNeeded(*attributes->headers_to_display->header);
		FreeAndNull(attributes->headers_to_display->header);
		free(attributes->headers_to_display);
		attributes->headers_to_display = NULL;
	}
	if (attributes->headers_to_not_display) {
		if (attributes->headers_to_not_display->header)
			FreeIfNeeded(*attributes->headers_to_not_display->header);
		FreeAndNull(attributes->headers_to_not_display->header);
		free(attributes->headers_to_not_display);
		attributes->headers_to_not_display = NULL;
	}
	free_if_not_default(&attributes->news_quote_format, &tinrc.news_quote_format);
	free_if_not_default(&attributes->organization, &default_organization);
	free_if_not_default(&attributes->quick_kill_scope, &default_filter_kill_global);
	free_if_not_default(&attributes->quick_select_scope, &default_filter_select_global);
	free_if_not_default(&attributes->quote_chars, &tinrc.quote_chars);
	free_if_not_default(&attributes->savedir, &tinrc.savedir);
	free_if_not_default(&attributes->savefile, (char **) 0);
	free_if_not_default(&attributes->sigfile, &tinrc.sigfile);
#ifdef CHARSET_CONVERSION
	free_if_not_default(&attributes->undeclared_charset, (char **) 0);
#endif /* CHARSET_CONVERSION */
	free_if_not_default(&attributes->x_headers, (char **) 0);
	free_if_not_default(&attributes->x_body, (char **) 0);
}

static void
free_tinrc_strings(
		void)
{
	FreeAndNull(tinrc.editor_format);
	FreeAndNull(tinrc.mailer_format);
	FreeAndNull(tinrc.default_goto_group);
	FreeAndNull(tinrc.default_mail_address);
#ifndef DONT_HAVE_PIPING
	FreeAndNull(tinrc.default_pipe_command);
#endif /* DONT_HAVE_PIPING */
	FreeAndNull(tinrc.default_post_newsgroups);
	FreeAndNull(tinrc.default_post_subject);
#ifndef DISABLE_PRINTING
	FreeAndNull(tinrc.printer);
#endif /* DISABLE_PRINTING */
	FreeAndNull(tinrc.default_range_group);
	FreeAndNull(tinrc.default_range_select);
	FreeAndNull(tinrc.default_range_thread);
	FreeAndNull(tinrc.default_pattern);
	FreeAndNull(tinrc.default_repost_group);
	FreeAndNull(tinrc.default_save_file);
	FreeAndNull(tinrc.default_search_art);
	FreeAndNull(tinrc.default_search_author);
	FreeAndNull(tinrc.default_search_config);
	FreeAndNull(tinrc.default_search_group);
	FreeAndNull(tinrc.default_search_subject);
	FreeAndNull(tinrc.default_select_pattern);
	FreeAndNull(tinrc.default_shell_command);
	FreeAndNull(tinrc.maildir);
	FreeAndNull(tinrc.mail_address);
	FreeAndNull(tinrc.mail_quote_format);
	FreeAndNull(tinrc.metamail_prog);
#ifndef CHARSET_CONVERSION
	FreeAndNull(tinrc.mm_charset);
#endif /* !CHARSET_CONVERSION */
	FreeAndNull(tinrc.mm_local_charset);
	FreeAndNull(tinrc.news_headers_to_display);
	FreeAndNull(tinrc.news_headers_to_not_display);
	FreeAndNull(tinrc.news_quote_format);
	FreeAndNull(tinrc.quote_chars);
#ifdef HAVE_COLOR
	FreeAndNull(tinrc.quote_regex);
	FreeAndNull(tinrc.quote_regex2);
	FreeAndNull(tinrc.quote_regex3);
	FreeAndNull(tinrc.extquote_regex);
#endif /* HAVE_COLOR */
	FreeAndNull(tinrc.slashes_regex);
	FreeAndNull(tinrc.stars_regex);
	FreeAndNull(tinrc.underscores_regex);
	FreeAndNull(tinrc.strokes_regex);
	FreeAndNull(tinrc.strip_re_regex);
	FreeAndNull(tinrc.strip_was_regex);
	FreeAndNull(tinrc.verbatim_begin_regex);
	FreeAndNull(tinrc.verbatim_end_regex);
	FreeAndNull(tinrc.hideline_regex);
	FreeAndNull(tinrc.savedir);
	FreeAndNull(tinrc.sigfile);
	FreeAndNull(tinrc.spamtrap_warning_addresses);
	FreeAndNull(tinrc.url_handler);
	FreeAndNull(tinrc.posted_articles_file);
	FreeAndNull(tinrc.inews_prog);
	FreeAndNull(tinrc.select_format);
	FreeAndNull(tinrc.group_format);
	FreeAndNull(tinrc.thread_format);
	FreeAndNull(tinrc.attachment_format);
	FreeAndNull(tinrc.page_mime_format);
	FreeAndNull(tinrc.page_uue_format);
	FreeAndNull(tinrc.page_yenc_format);
	FreeAndNull(tinrc.date_format);
	FreeAndNull(tinrc.xpost_quote_format);
#	ifdef NNTPS_ABLE
		FreeAndNull(tinrc.tls_ca_cert_file);
#	endif /* NNTPS_ABLE */
}


static void
free_cmdline_args(
	void)
{
	FreeAndNull(cmdline.maildir);
	FreeAndNull(cmdline.nntpserver);
	FreeAndNull(cmdline.savedir);
	FreeAndNull(cmdline.msgid);
}


void
free_scope(
	int num)
{
	struct t_scope *scope;

	scope = &scopes[num];
	FreeAndNull(scope->scope);
	free_attributes(scope->attribute);
	free(scope->attribute);
	scope->attribute = (struct t_attribute *) 0;
	free(scope->state);
	scope->state = (struct t_attribute_state *) 0;
}


static void
free_scopes_arrays(
	void)
{
	while (num_scope > 0)
		free_scope(--num_scope);
	FreeAndNull(scopes);
	num_scope = -1;
	FreeAndNull(default_filter_kill_global);
	FreeAndNull(default_filter_select_global);
	FreeAndNull(default_organization);
	FreeAndNull(default_mime_types_to_save);
}


static void
free_active_arrays(
	void)
{
	FreeAndNull(my_group);	/* my_group[] */

	if (active != NULL) {	/* active[] */
		int i;

		for_each_group(i) {
			FreeAndNull(active[i].name);
			FreeAndNull(active[i].description);
			FreeAndNull(active[i].aliasedto);
			if (active[i].type == GROUP_TYPE_MAIL || active[i].type == GROUP_TYPE_SAVE) {
				FreeAndNull(active[i].spooldir);
			}
			FreeAndNull(active[i].newsrc.xbitmap);
			if (active[i].attribute && !active[i].attribute->global) {
				free(active[i].attribute);
				active[i].attribute = (struct t_attribute *) 0;
			}
		}
		FreeAndNull(active);
	}
	num_active = -1;
}


void
free_save_array(
	void)
{
	int i;

	for (i = 0; i < num_save; i++) {
		FreeAndNull(save[i].path);
		/* file does NOT need to be freed */
		save[i].file = NULL;
		save[i].mailbox = FALSE;
	}
	num_save = 0;
}


static void
free_newnews_array(
	void)
{
	int i;

	for (i = 0; i < num_newnews; i++)
		FreeAndNull(newnews[i].host);

	num_newnews = 0;
}


static void
free_input_history(
	void)
{
	int his_w, his_e;

	for (his_w = 0; his_w <= HIST_MAXNUM; his_w++) {
		for (his_e = 0; his_e < HIST_SIZE; his_e++) {
			FreeIfNeeded(input_history[his_w][his_e]);
		}
	}
}


void *
my_malloc1(
	const char *file,
	int line,
	size_t size)
{
	void *p;

#ifdef DEBUG
	if (debug & DEBUG_MEM)
		debug_print_malloc(TRUE, file, line, size);
#endif /* DEBUG */

	if ((p = malloc(size)) == NULL) {
		error_message(2, txt_out_of_memory, tin_progname, (unsigned long) size, file, line);
		giveup();
	}
	return p;
}


/*
 * TODO: add fallback code with malloc(nmemb*size);memset(0,nmemb*size)?
 */
void *
my_calloc1(
	const char *file,
	int line,
	size_t nmemb,
	size_t size)
{
	void *p;

#ifdef DEBUG
	if (debug & DEBUG_MEM)
		debug_print_malloc(TRUE, file, line, nmemb * size);
#endif /* DEBUG */

	if ((p = calloc(nmemb, size)) == NULL) {
		error_message(2, txt_out_of_memory, tin_progname, (unsigned long) (nmemb * size), file, line);
		giveup();
	}
	return p;
}


/*
 * TODO: add fallback code with
 *        q = malloc(nmemb * size); memmove(q, p, size); free(p)?
 *
 * for size = 0 and p != NULL, we do what glibc does and
 * ignore ISO C 23  7.24.3.7 ("undefined behavior").
 */
void *
my_realloc1(
	const char *file,
	int line,
	void *p,
	size_t size)
{
#ifdef DEBUG
	if (debug & DEBUG_MEM)
		debug_print_malloc(FALSE, file, line, size);
#endif /* DEBUG */

	if (!size) {
		if (p)
			free(p);

		return NULL;
	}

	if (p) {
		void *q = realloc(p, size);

		if (q != NULL)
			p = q;
		else {
			free(p);
			p = NULL;
		}
	} else
		p = malloc(size);

	if (p == NULL) {
		error_message(2, txt_out_of_memory, tin_progname, (unsigned long) size, file, line);
		giveup();
	}
	return p;
}


#if !defined(HAVE_MEMMOVE) && !defined(HAVE_BCOPY)
void
my_memmove(
	void *dest,
	const void *src,
	size_t n)
{
	char *d = (char *) dest;
	const char *c = (const char *) src;

	if (c < d && d < c + n) {
		d += n;
		c += n;
		while (n--)
			*--d = *--c;
	} else {
		while (n--)
			*d++ = *c++;
	}
}
#endif /* !HAVE_MEMMOVE && !HAVE_BCOPY */
