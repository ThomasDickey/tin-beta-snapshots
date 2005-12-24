/*
 *  Project   : tin - a Usenet reader
 *  Module    : post.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2005-12-23
 *  Notes     : mail/post/replyto/followup/repost & cancel articles
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>
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


#ifdef USE_CANLOCK
#	define ADD_CAN_KEY(id) { \
		char key[1024]; \
		const char *kptr = (const char *) 0; \
		key[0] = '\0'; \
		if ((kptr = build_cankey(id, get_secret())) != NULL) { \
			STRCPY(key, kptr); \
			msg_add_header("Cancel-Key", key); \
		} \
	}
	/*
	 * only add lock here if we use an external inews
	 * and generate our own Message-IDs (EVIL_INSIDE)
	 * otherwise inews.c adds the canlock (if possible:
	 * i.e EVIL_INSIDE or server passed id on POST or
	 * user supplied ID by hand) for us!
	 */
#	ifdef EVIL_INSIDE
#		define ADD_CAN_LOCK(id) { \
			char lock[1024]; \
			const char *lptr = (const char *) 0; \
			lock[0] = '\0'; \
			if ((lptr = build_canlock(id, get_secret())) != NULL) { \
				STRCPY(lock, lptr); \
				msg_add_header("Cancel-Lock", lock); \
			} \
		}
#	endif /* EVIL_INSIDE */
#else
#	define ADD_CAN_KEY(id)
#	ifdef EVIL_INSIDE
#		define ADD_CAN_LOCK(id)
#	endif /* EVIL_INSIDE */
#endif /* USE_CANLOCK */

#ifdef EVIL_INSIDE
/* gee! ugly hack - but works */
#	define ADD_MSG_ID_HEADER()	{ \
		char mid[1024]; \
		const char *mptr = (const char *) 0; \
		mid[0] = '\0'; \
		if ((mptr = build_messageid()) != NULL) { \
			STRCPY(mid, mptr); \
			msg_add_header("Message-ID", mid); \
			ADD_CAN_LOCK(mid); \
		} \
	}
#else
#	define ADD_MSG_ID_HEADER()
#endif /* EVIL_INSIDE */

#define MAX_MSG_HEADERS	20	/* shouldn't this be dynamic? */

/* Different posting types for post_loop() */
#define POST_QUICK		0
#define POST_POSTPONED	1
#define POST_NORMAL		2
#define POST_RESPONSE	3
#define POST_REPOST		4

/* When prompting for subject, display no more than 20 characters */
#define DISPLAY_SUBJECT_LEN 20

static int start_line_offset = 1;		/* used by invoke_editor for line no. */

char bug_addr[LEN];			/* address to add send bug reports to */
static char my_distribution[LEN];		/* Distribution: */
static char reply_to[LEN];		/* Reply-To: address */

static struct msg_header {
	char *name;
	char *text;
} msg_headers[MAX_MSG_HEADERS];


/*
 * Local prototypes
 */
static FILE *create_mail_headers(char *filename, const char *suffix, const char *to, const char *subject, struct t_header *extra_hdrs);
static char **split_address_list(const char *addresses, unsigned int *cnt);
static char *backup_article_name(const char *the_article);
static int add_mail_quote(FILE *fp, int respnum);
static int check_article_to_be_posted(const char *the_article, int art_type, struct t_group **group, t_bool art_unchanged);
static int mail_loop(const char *filename, t_function func, char *subject, const char *groupname, const char *prompt, FILE *articlefp);
static int msg_add_x_body(FILE *fp_out, const char *body);
static int msg_write_headers(FILE *fp);
static int post_loop(int type, struct t_group *group, t_function func, const char *posting_msg, int art_type, int offset);
static unsigned int get_recipients(struct t_header *hdr, char *buf, size_t buflen);
static size_t skip_id(const char *id);
static struct t_group *check_moderated(const char *groups, int *art_type, const char *failmsg);
static t_bool address_in_list(const char *addresses, const char *address);
static t_bool append_mail(const char *the_article, const char *addr, const char *the_mailbox);
static t_bool backup_article(const char *the_article);
static t_bool check_for_spamtrap(const char *addr);
static t_bool create_normal_article_headers(struct t_group *group, const char *newsgroups, int art_type);
static t_bool damaged_id(const char *id);
static t_bool fetch_postponed_article(const char tmp_file[], char subject[], char newsgroups[]);
static t_bool insert_from_header(const char *infile);
static t_bool is_crosspost(const char *xref);
static t_bool must_include(const char *id);
static t_bool repair_article(t_function *result, struct t_group *group);
static t_bool submit_mail_file(const char *file, struct t_group *group, FILE *articlefp, t_bool include_text);
static t_function prompt_rejected(void);
static t_function prompt_to_send(const char *subject);
static void add_headers(const char *infile, const char *a_message_id);
static void appendid(char **where, const char **what);
static void find_reply_to_addr(char *from_addr, t_bool parse, struct t_header *hdr);
static void join_references(char *buffer, const char *oldrefs, const char *newref);
static void msg_add_header(const char *name, const char *text);
static void msg_add_x_headers(const char *headers);
static void msg_free_headers(void);
static void msg_init_headers(void);
static void post_postponed_article(int ask, const char *subject, const char *newsgroups);
static void postpone_article(const char *the_article);
static void setup_check_article_screen(int *init);
static void strip_double_ngs(char *ngs_list);
static void update_active_after_posting(char *newsgroups);
static void update_posted_info_file(const char *group, int action, const char *subj, const char *a_message_id);
#ifdef FORGERY
	static void make_path_header(char *line);
#endif /* FORGERY */
#ifdef EVIL_INSIDE
	static const char *build_messageid(void);
	static char *radix32(unsigned long int num);
#endif /* EVIL_INSIDE */
#ifdef USE_CANLOCK
	static const char *build_cankey(const char *messageid, const char *secret);
#endif /* USE_CANLOCK */


static t_function
prompt_to_send(
	const char *subject)
{
	char *smsg;
	char buf[LEN];
	char keyedit[MAXKEYLEN];
	char keyquit[MAXKEYLEN];
	char keysend[MAXKEYLEN];
#ifdef HAVE_ISPELL
	char keyispell[MAXKEYLEN];
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
	char keypgp[MAXKEYLEN];
#endif /* HAVE_PGP_GPG */
	t_function func;

	snprintf(buf, sizeof(buf), _(txt_quit_edit_send),
					printascii(keyquit, func_to_key(GLOBAL_QUIT, post_send_keys)),
					printascii(keyedit, func_to_key(POST_EDIT, post_send_keys)),
#ifdef HAVE_ISPELL
					printascii(keyispell, func_to_key(POST_ISPELL, post_send_keys)),
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
					printascii(keypgp, func_to_key(POST_PGP, post_send_keys)),
#endif /* HAVE_PGP_GPG */
					printascii(keysend, func_to_key(POST_SEND, post_send_keys)));

	func = prompt_slk_response(POST_SEND, post_send_keys, "%s",
				sized_message(&smsg, buf, subject));
	free(smsg);
	return func;
}


static t_function
prompt_rejected(
	void)
{
	char keyedit[MAXKEYLEN], keypostpone[MAXKEYLEN], keyquit[MAXKEYLEN];

/* FIXME (what does this mean?) fix screen pos. */
	Raw(FALSE);
	/* TODO: replace hardcoded key-name in txt_post_error_ask_postpone */
	my_fprintf(stderr, "\n\n%s\n\n", _(txt_post_error_ask_postpone));
	my_fflush(stderr);
	Raw(TRUE);

	return prompt_slk_response(POST_EDIT, post_edit_keys,
				_(txt_quit_edit_postpone),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, post_edit_keys)),
				printascii(keyedit, func_to_key(POST_EDIT, post_edit_keys)),
				printascii(keypostpone, func_to_key(POST_POSTPONE, post_edit_keys)));
}


/*
 * Set up posting specific environment
 */
void
init_postinfo(
	void)
{
	char *ptr;

	/*
	 * check enviroment for REPLYTO
	 */
	reply_to[0] = '\0';
	if ((ptr = getenv("REPLYTO")) != NULL)
		my_strncpy(reply_to, ptr, sizeof(reply_to) - 1);

	/*
	 * check enviroment for DISTRIBUTION
	 */
	my_distribution[0] = '\0';
	if ((ptr = getenv("DISTRIBUTION")) != NULL)
		my_strncpy(my_distribution, ptr, sizeof(my_distribution) - 1);
}


/*
 * TODO: add p'o'stpone function here? would be nice but difficult
 *       as the postpone fetcher looks for articles with correct headers
 */
static t_bool
repair_article(
	t_function *result,
	struct t_group *group)
{
	char keyedit[MAXKEYLEN], keymenu[MAXKEYLEN], keyquit[MAXKEYLEN];
	t_function func;

	func = prompt_slk_response(POST_EDIT, post_edit_ext_keys, _(txt_bad_article),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, post_edit_ext_keys)),
				printascii(keymenu, func_to_key(GLOBAL_OPTION_MENU, post_edit_ext_keys)),
				printascii(keyedit, func_to_key(POST_EDIT, post_edit_ext_keys)));

	*result = func;
	if (func == POST_EDIT) {
		if (invoke_editor(article_name, start_line_offset))
			return TRUE;
	} else if (func == GLOBAL_OPTION_MENU) {
		(void) change_config_file(group); /*OD:*/
		return TRUE;
	}
	return FALSE;
}


/*
 * make a backup copy of ~/TIN_ARTICLE_NAME, this is necessary since
 * submit_news_file adds headers, does q-p conversion etc
 * TODO: why not use BACKUP_FILE_EXT like in misc.c?
 */
static char *
backup_article_name(
	const char *the_article)
{
	static char name[PATH_LEN];

	snprintf(name, sizeof(name), "%s.bak", the_article);
	return name;
}


static t_bool
backup_article(
	const char *the_article)
{
	return backup_file(the_article, backup_article_name(the_article));
}


static void
msg_init_headers(
	void)
{
	int i;

	for (i = 0; i < MAX_MSG_HEADERS; i++) {
		msg_headers[i].name = NULL;
		msg_headers[i].text = NULL;
	}
}


static void
msg_free_headers(
	void)
{
	int i;

	for (i = 0; i < MAX_MSG_HEADERS; i++) {
		FreeAndNull(msg_headers[i].name);
		FreeAndNull(msg_headers[i].text);
	}
}


static void
msg_add_header(
	const char *name,
	const char *text)
{
	const char *p;
	char *ptr;
	char *new_name;
	char *new_text = NULL;
	int i;
	t_bool done = FALSE;

	if (name) {
		/*
		 * Remove : if one is attached to name
		 */
		new_name = my_strdup(name);
		ptr = strchr(new_name, ':');
		if (ptr)
			*ptr = '\0';

		/*
		 * Check if header already exists and if update text
		 */
		for (i = 0; i < MAX_MSG_HEADERS && msg_headers[i].name; i++) {
			if (STRCMPEQ(msg_headers[i].name, new_name)) {
				FreeAndNull(msg_headers[i].text);
				if (text) {
					for (p = text; *p && (*p == ' ' || *p == '\t'); p++)
						;
					new_text = my_strdup(p);
					ptr = strrchr(new_text, '\n');
					if (ptr)
						*ptr = '\0';

					msg_headers[i].text = my_strdup(new_text);
				}
				done = TRUE;
			}
		}

		/*
		 * if header does not exist then add it
		 */
		if (!(done || msg_headers[i].name)) {
			msg_headers[i].name = my_strdup(new_name);
			if (text) {
				for (p = text; *p && (*p == ' ' || *p == '\t'); p++)
					;
				new_text = my_strdup(p);
				ptr = strrchr(new_text, '\n');
				if (ptr)
					*ptr = '\0';

				msg_headers[i].text = my_strdup(new_text);
			}
		}
		FreeIfNeeded(new_name);
		FreeIfNeeded(new_text);
	}
}


static int
msg_write_headers(
	FILE *fp)
{
	int i;
	int wrote = 1;
	char *p;

	for (i = 0; i < MAX_MSG_HEADERS; i++) {
		if (msg_headers[i].name) {
			fprintf(fp, "%s: %s\n", msg_headers[i].name, BlankIfNull(msg_headers[i].text));
			p = msg_headers[i].text;
			do {
				wrote++;
				p = strchr(++p, '\n');
			} while (p);
		}
	}
	fputc('\n', fp);

	return wrote;
}


/* TODO: handle optional Message-ID: field */
t_bool
user_posted_messages(
	void)
{
	FILE *fp;
	char buf[LEN];
	int no_of_lines = 0;
	size_t group_len = 0, i = 0, j, k;
	struct t_posted *posted;

	if ((fp = fopen(posted_info_file, "r")) == NULL) {
		clear_message();
		return FALSE;
	}

	while (fgets(buf, (int) sizeof(buf), fp) != NULL)
		no_of_lines++;

	if (!no_of_lines) {
		fclose(fp);
		info_message(_(txt_no_arts_posted));
		return FALSE;
	}
	rewind(fp);
	posted = my_malloc((no_of_lines + 1) * sizeof(struct t_posted));

	while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
		if (buf[0] == '#' || buf[0] == '\n')
			continue;

		for (j = 0, k = 0; buf[j] != '|' && buf[j] != '\n'; j++)
			if (k < sizeof(posted[i].date) - 1)
				posted[i].date[k++] = buf[j];	/* posted date */

		if (buf[j] == '\n') {
			error_message(_(txt_error_corrupted_file), posted_info_file);
			(void) sleep(1);
			fclose(fp);
			clear_message();
			FreeAndNull(posted);
			return FALSE;
		}
		posted[i].date[k] = '\0';
		posted[i + 1].date[0] = '\0';

		posted[i].action = buf[++j];
		j += 2;

		for (k = 0; buf[j] != '|' && buf[j] != ','; j++) {
			if (k < sizeof(posted[i].group) - 1)
				posted[i].group[k++] = buf[j];
		}
		if (buf[j] == ',') {
			while (buf[j] != '|' && buf[j] != '\n')
				j++;

			if (k > sizeof(posted[i].group) - 5)
				k = sizeof(posted[i].group) - 5;
			posted[i].group[k++] = ',';
			posted[i].group[k++] = '.';
			posted[i].group[k++] = '.';
			posted[i].group[k++] = '.';
		}
		posted[i].group[k] = '\0';
		if (k > group_len)
			group_len = k;
		j++;

		for (k = 0; buf[j] != '\n'; j++) {
			if (k < sizeof(posted[i].subj) - 1)
				posted[i].subj[k++] = buf[j];
		}
		posted[i].subj[k] = '\0';
		i++;
	}
	posted[i].date[0] = '\0';	/* end-marker for display */
	fclose(fp);

	if (!(fp = tmpfile())) {
		FreeAndNull(posted);
		return FALSE;
	}
	for (; i > 0; i--) {
		snprintf(buf, sizeof(buf), "%8s  %c  %-*s  %s",
			posted[i - 1].date, posted[i - 1].action,
			(int) group_len, posted[i - 1].group, posted[i - 1].subj);
		buf[cCOLS - 2] = '\0';
		fprintf(fp, "%s%s", buf, cCRLF);
	}
	FreeAndNull(posted);
	info_pager(fp, _(txt_post_history_menu), TRUE);
	fclose(fp);

	return TRUE;
}


static void
update_posted_info_file(
	const char *group,
	int action,
	const char *subj,
	const char *a_message_id)
{
	FILE *fp;
	char *file_tmp;
	struct tm *pitm;
	time_t epoch;

	if (no_write)
		return;

	file_tmp = get_tmpfilename(posted_info_file);
	if (!backup_file(posted_info_file, file_tmp)) {
		error_message(_(txt_filesystem_full_backup), posted_info_file);
		free(file_tmp);
		return;
	}

	if ((fp = fopen(posted_info_file, "a+")) != NULL) {
		(void) time(&epoch);
		pitm = localtime(&epoch);
		if (*a_message_id)
			fprintf(fp, "%02d-%02d-%02d|%c|%s|%s|%s\n", pitm->tm_mday, pitm->tm_mon + 1, pitm->tm_year % 100, action, BlankIfNull(group), BlankIfNull(subj), a_message_id);
		else
			fprintf(fp, "%02d-%02d-%02d|%c|%s|%s\n", pitm->tm_mday, pitm->tm_mon + 1, pitm->tm_year % 100, action, BlankIfNull(group), BlankIfNull(subj));
		if (ferror(fp) || fclose(fp)) {
			error_message(_(txt_filesystem_full), posted_info_file);
			rename_file(file_tmp, posted_info_file);
		} else
			unlink(file_tmp);
	} else
		rename_file(file_tmp, posted_info_file);

	free(file_tmp);
}


/*
 * appends the content of the_article to the_mailbox, with a From_ line of
 * addr, does mboxo/mboxrd From_ line quoting if needed (!MMDF-style mbox)
 */
static t_bool
append_mail(
	const char *the_article,
	const char *addr,
	const char *the_mailbox)
{
	FILE *fp_in, *fp_out;
	char *bufp;
	char buf[LEN];
	int fd;
	int retrys = 10;	/* maximum lock retrys */
	time_t epoch;
	t_bool mmdf = FALSE;
	t_bool rval = FALSE;

	if (!strcasecmp(txt_mailbox_formats[tinrc.mailbox_format], "MMDF") && the_mailbox != postponed_articles_file)
		mmdf = TRUE;

	if ((fp_in = fopen(the_article, "r")) == NULL)
		return rval;

	if ((fp_out = fopen(the_mailbox, "a+")) != NULL) {
		fd = fileno(fp_out);
		/* TODO: move the retry/error stuff into a function? */
		while (retrys-- && fd_lock(fd, FALSE))
			wait_message(1, _(txt_trying_lock), retrys, the_mailbox);
		if (retrys < 0) {
			wait_message(5, _(txt_error_couldnt_lock), the_mailbox);
			fclose(fp_out);
			fclose(fp_in);
			return rval;
		}
		while (retrys-- && !dot_lock(the_mailbox))
			wait_message(1, _(txt_trying_dotlock), retrys, the_mailbox);
		if (retrys < 0) {
			wait_message(5, _(txt_error_couldnt_dotlock), the_mailbox);
			fd_unlock(fd);
			fclose(fp_out);
			fclose(fp_in);
			return rval;
		}

		if (mmdf)
			fprintf(fp_out, "%s", MMDFHDRTXT);
		else {
			(void) time(&epoch);
			fprintf(fp_out, "From %s %s", addr, ctime(&epoch));
		}
		while (fgets(buf, (int) sizeof(buf), fp_in) != NULL) {
			if (!mmdf) { /* moboxo/mboxrd style From_ quoting required */
				/*
				 * TODO: add Content-Length: header when using MBOXO
				 *       so tin actually write MBOXCL instead of MBOXO?
				 */
				if (tinrc.mailbox_format == 1) { /* MBOXRD */
					/* mboxrd: quote quoted and plain From_ lines in the body */
					bufp = buf;
					while (*bufp == '>')
						bufp++;
					if (strncmp(bufp, "From ", 5) == 0)
						fputc('>', fp_out);
				} else { /* MBOXO (MBOXCL) */
					if (strncmp(buf, "From ", 5) == 0)
						fputc('>', fp_out);
				}
			}
			fputs(buf, fp_out);
		}
		print_art_seperator_line(fp_out, mmdf);

		fflush(fp_out);
		if (fd_unlock(fd) || !dot_unlock(the_mailbox))
			wait_message(4, _(txt_error_cant_unlock), the_mailbox);

		fclose(fp_out);
		rval = TRUE;
	}
	fclose(fp_in);
	return rval;
}


/*
 * Check the article file for correct header syntax and if there
 * is a blank between the header information and the text.
 *
 * Additionally make **group point to one of the groups we are actually posting to.
 *
 * 1.  Subject header present
 * 2.  Newsgroups header present
 *     From header present
 * 3.  Space after every colon in header
 * 4.  Colon in every header line
 * 5.  Newsgroups line has no spaces, only comma separated
 * 6.  List of newsgroups is presented to user with description
 * 7.  Lines in body that are to long causes a warning to be printed
 * 8.  Group(s) must be listed in the active file
 * 9.  No Sender: header allowed (limit forging) and rejection by
 *     inn servers
 * 10. Check for charset != US-ASCII when using non-7bit-encoding
 * 11. Warn if transfer encoding is base64 or quoted-printable and using
 *     external inews
 * 12. Check that Subject, Newsgroups and if present Followup-To
 *     headers are uniq
 * 13. Display an 'are you sure' message before posting article
 */
#define CA_ERROR_HEADER_LINE_BLANK        0x000001
#define CA_ERROR_MISSING_BODY_SEPERATOT   0x000002
#define CA_ERROR_MISSING_FROM             0x000004
#define CA_ERROR_DUPLICATED_FROM          0x000008
#define CA_ERROR_MISSING_SUBJECT          0x000010
#define CA_ERROR_DUPLICATED_SUBJECT       0x000020
#define CA_ERROR_EMPTY_SUBJECT            0x000040
#define CA_ERROR_MISSING_NEWSGROUPS       0x000080
#define CA_ERROR_DUPLICATED_NEWSGROUPS    0x000100
#define CA_ERROR_EMPTY_NEWSGROUPS         0x000200
#define CA_ERROR_DUPLICATED_FOLLOWUP_TO   0x000400
#define CA_ERROR_BAD_CHARSET              0x000800
#define CA_ERROR_BAD_ENCODING             0x001000
#ifndef FOLLOW_USEFOR_DRAFT
#	define CA_ERROR_SPACE_IN_NEWSGROUPS    0x002000
#	define CA_ERROR_NEWLINE_IN_NEWSGROUPS  0x004000
#	define CA_ERROR_SPACE_IN_FOLLOWUP_TO   0x008000
#	define CA_ERROR_NEWLINE_IN_FOLLOWUP_TO 0x010000
#endif /* !FOLLOW_USEFOR_DRAFT */
#define CA_WARNING_SPACES_ONLY_SUBJECT      0x000001
#define CA_WARNING_RE_WITHOUT_REFERENCES    0x000002
#define CA_WARNING_REFERENCES_WITHOUT_RE    0x000004
#define CA_WARNING_MULTIPLE_SIGDASHES       0x000008
#define CA_WARNING_WRONG_SIGDASHES          0x000010
#define CA_WARNING_LONG_SIGNATURE           0x000020
#define CA_WARNING_ENCODING_EXTERNAL_INEWS  0x000040
#ifdef CHARSET_CONVERSION
#	define CA_WARNING_CHARSET_CONVERSION    0x000080
#endif /* CHARSET_CONVERSION */
#ifdef FOLLOW_USEFOR_DRAFT
#	define CA_WARNING_SPACE_IN_NEWSGROUPS    0x000100
#	define CA_WARNING_NEWLINE_IN_NEWSGROUPS  0x000200
#	define CA_WARNING_SPACE_IN_FOLLOWUP_TO   0x000400
#	define CA_WARNING_NEWLINE_IN_FOLLOWUP_TO 0x000800
#endif /* FOLLOW_USEFOR_DRAFT */

/*
 * TODO: cleanup
 *
 * return values:
 * 	0	article ok
 * 	1	article contains errors
 * 	2	article caused warnings
 */
static int
check_article_to_be_posted(
	const char *the_article,
	int art_type,
	struct t_group **group,
	t_bool art_unchanged)
{
	FILE *fp;
	char *ngptrs[NGLIMIT], *ftngptrs[NGLIMIT];
	char *line, *cp, *cp2;
	char references[HEADER_LEN];
	char subject[HEADER_LEN];
	int cnt = 0;
	int col, i;
	int errors = 0;
	int warnings = 0;
	int init = 1;
	int ngcnt = 0, ftngcnt = 0;
	int oldraw;		/* save previous raw state */
	int saw_sig_dashes = 0;
	int sig_lines = 0;
	int found_followup_to_lines = 0;
	int found_from_lines = 0;
	int found_newsgroups_lines = 0;
	int found_subject_lines = 0;
	int errors_catbp = 0; /* sum of error-codes */
	int warnings_catbp = 0; /* sum of warning-codes */
	size_t nglens[NGLIMIT], ftnglens[NGLIMIT];
	struct t_group *psGrp;
	t_bool end_of_header = FALSE;
	t_bool got_long_line = FALSE;
	t_bool saw_references = FALSE;
	t_bool saw_wrong_sig_dashes = FALSE;
	t_bool mime_7bit = TRUE;
	t_bool mime_usascii = FALSE;
	t_bool contains_8bit = FALSE;
	int must_break_line = 0;
#ifdef CHARSET_CONVERSION
	t_bool charset_conversion_fails = FALSE;
	int mmnwcharset = *group ? (*group)->attribute->mm_network_charset : tinrc.mm_network_charset;
#endif /* CHARSET_CONVERSION */

	if ((fp = fopen(the_article, "r")) == NULL) {
		perror_message(_(txt_cannot_open), the_article);
		return 0;
	}
	oldraw = RawState();	/* save state */

	/* check the header of the article */
	while ((line = tin_fgets(fp, TRUE)) != NULL) {
		cnt++;
		if (!end_of_header && !strlen(line)) { /* end of header reached */
			if (cnt == 1)
				errors_catbp |= CA_ERROR_HEADER_LINE_BLANK;
			end_of_header = TRUE;
			break;
		}

		if (!contains_8bit) {
			for (cp = line; *cp; cp++) {
				if (!isascii(*cp)) {
					contains_8bit = TRUE;
					break;
				}
			}
		}
#ifdef CHARSET_CONVERSION
		/* are all characters in article contained in network_charset? */
		if (strcmp(tinrc.mm_local_charset, txt_mime_charsets[mmnwcharset]) && !charset_conversion_fails) { /* local_charset != network_charset */
			cp = my_malloc(strlen(line) * 4 + 1);
			strcpy(cp, line);
			charset_conversion_fails = !buffer_to_network(cp, mmnwcharset);
			free(cp);
		}
#endif /* CHARSET_CONVERSION */

		if ((cp = strchr(line, ':')) == NULL) {
			setup_check_article_screen(&init);
			StartInverse();
			my_fprintf(stderr, _(txt_error_header_line_colon), cnt, line);
			my_fflush(stderr);
			EndInverse();
			errors++;
			continue;
		}
		if (cp[1] != ' ') {
			setup_check_article_screen(&init);
			StartInverse();
			my_fprintf(stderr, _(txt_error_header_line_space), cnt, line);
			my_fflush(stderr);
			EndInverse();
			errors++;
		}

		if (cp - line == 7 && !strncasecmp(line, "Subject", 7)) {
			found_subject_lines++;
			strncpy(subject, cp + 2, cCOLS - 6);
			subject[cCOLS - 6] = '\0';
		}

#ifndef FORGERY
		if (cp - line == 6 && !strncasecmp(line, "Sender", 6)) {
			setup_check_article_screen(&init);
			StartInverse();
			my_fprintf(stderr, _(txt_error_sender_in_header_not_allowed), cnt);
			my_fflush(stderr);
			EndInverse();
			errors++;
		}
#endif /* !FORGERY */

		if (cp - line == 8 && !strncasecmp(line, "Approved", 8)) {
			if (tinrc.beginner_level) {
				setup_check_article_screen(&init);
				/* StartInverse(); */
				my_fprintf(stderr, _(txt_error_approved)); /* this is only a Warning: */
				my_fflush(stderr);
				/* EndInverse(); */
#ifdef HAVE_FASCIST_NEWSADMIN
				errors++;
#else
				warnings++;
#endif /* HAVE_FASCIST_NEWSADMIN */
			}
#ifdef CHARSET_CONVERSION
			cp2 = rfc1522_encode(line, txt_mime_charsets[mmnwcharset], FALSE);
#else
			cp2 = rfc1522_encode(line, tinrc.mm_charset, FALSE);
#endif /* CHARSET_CONVERSION */
			if (GNKSA_OK != (i = gnksa_check_from(cp2 + (cp - line) + 1))) {
				setup_check_article_screen(&init);
				StartInverse();
				my_fprintf(stderr, _(txt_error_bad_approved), i);
				my_fprintf(stderr, gnksa_strerror(i), i);
				my_fflush(stderr);
				EndInverse();
#ifndef FORGERY
				errors++;
#endif /* !FORGERY */
			}
			free(cp2);
		}

		if (cp - line == 4 && !strncasecmp(line, "From", 4)) {
			found_from_lines++;
#ifdef CHARSET_CONVERSION
			cp2 = rfc1522_encode(line, txt_mime_charsets[mmnwcharset], FALSE);
#else
			cp2 = rfc1522_encode(line, tinrc.mm_charset, FALSE);
#endif /* CHARSET_CONVERSION */
			if (GNKSA_OK != (i = gnksa_check_from(cp2 + (cp - line) + 1))) {
				setup_check_article_screen(&init);
				StartInverse();
				my_fprintf(stderr, _(txt_error_bad_from), i);
				my_fprintf(stderr, gnksa_strerror(i), i);
				my_fflush(stderr);
				EndInverse();
#ifndef FORGERY
				errors++;
#endif /* !FORGERY */
			}
			free(cp2);
		}

		if (cp - line == 8 && !strncasecmp(line, "Reply-To", 8)) {
#ifdef CHARSET_CONVERSION
			cp2 = rfc1522_encode(line, txt_mime_charsets[mmnwcharset], FALSE);
#else
			cp2 = rfc1522_encode(line, tinrc.mm_charset, FALSE);
#endif /* CHARSET_CONVERSION */
			if (GNKSA_OK != (i = gnksa_check_from(cp2 + (cp - line) + 1))) {
				setup_check_article_screen(&init);
				StartInverse();
				my_fprintf(stderr, _(txt_error_bad_replyto), i);
				my_fprintf(stderr, gnksa_strerror(i), i);
				my_fflush(stderr);
				EndInverse();
#ifndef FORGERY
				errors++;
#endif /* !FORGERY */
			}
			free(cp2);
		}

		if (cp - line == 10 && !strncasecmp(line, "Message-ID", 10)) {
#if 0 /* see comment about "<>" in misc.c:gnksa_split_from() */
			char addr[HEADER_LEN], name[HEADER_LEN];
			int type;

			i = gnksa_check_from(cp + 1);
			gnksa_split_from(cp + 1, addr, name, &type);
			if (((GNKSA_OK != i) && (GNKSA_LOCALPART_MISSING > i)) || !*addr) {
#else
			i = gnksa_check_from(cp + 1);
			if ((GNKSA_OK != i) && (GNKSA_LOCALPART_MISSING > i)) {
#endif /* 0 */
				setup_check_article_screen(&init);
				StartInverse();
				my_fprintf(stderr, _(txt_error_bad_msgidfqdn), i);
				my_fprintf(stderr, gnksa_strerror(i), i);
				my_fflush(stderr);
				EndInverse();
#ifndef FORGERY
				errors++;
#endif /* !FORGERY */
			}
		}

		if (cp - line == 10 && !strncasecmp(line, "References", 10)) {
			for (cp = line + 11; *cp == ' '; cp++)
				;
			STRCPY(references, cp);
			if (strlen(references))
				saw_references = TRUE;
		}

		/*
		 * TODO: also check for other illegal chars?
		 *       a 'common' error is to use a semicolon instead of a comma.
		 */
		if (cp - line == 10 && !strncasecmp(line, "Newsgroups", 10)) {
			found_newsgroups_lines++;
			for (cp = line + 11; *cp == ' '; cp++)
				;
			if (strchr(cp, ' ') || strchr(cp, '\t')) {
#ifdef FOLLOW_USEFOR_DRAFT
				warnings_catbp |= CA_WARNING_SPACE_IN_NEWSGROUPS;
#else
				errors_catbp |= CA_ERROR_SPACE_IN_NEWSGROUPS;
#endif /* FOLLOW_USEFOR_DRAFT */
			}
			if (strchr(cp, '\n')) {
#ifdef FOLLOW_USEFOR_DRAFT
				warnings_catbp |= CA_WARNING_NEWLINE_IN_NEWSGROUPS;
#else
				errors_catbp |= CA_ERROR_NEWLINE_IN_NEWSGROUPS;
#endif /* FOLLOW_USEFOR_DRAFT */
				unfold_header(line);
			}

			strip_double_ngs(cp);
			while (*cp) {
				if (!(cp2 = strchr(cp, ',')))
					cp2 = cp + strlen(cp);
				else
					*cp2++ = '\0';
				if (ngcnt < NGLIMIT) {
					nglens[ngcnt] = strlen(cp);
					ngptrs[ngcnt] = my_malloc(nglens[ngcnt] + 1);
					if (!ngptrs[ngcnt]) {
						for (i = 0; i < ngcnt; i++)
							FreeIfNeeded(ngptrs[i]);
						for (i = 0; i < ftngcnt; i++)
							FreeIfNeeded(ftngptrs[i]);
						Raw(oldraw);
						return 1;
					}
					strcpy(ngptrs[ngcnt], cp);
					ngcnt++;
				}
				cp = cp2;
			}
			if (!ngcnt)
				errors_catbp |= CA_ERROR_EMPTY_NEWSGROUPS;
		}

		if (cp - line == 11 && !strncasecmp(line, "Followup-To", 11)) {
			for (cp = line + 12; *cp == ' '; cp++)
				;
			if (strlen(cp)) /* Followup-To not empty */
				found_followup_to_lines++;
			strip_double_ngs(cp);
			if (strchr(cp, ' ') || strchr(cp, '\t')) {
#ifdef FOLLOW_USEFOR_DRAFT
				warnings_catbp |= CA_WARNING_SPACE_IN_FOLLOWUP_TO;
#else
				errors_catbp |= CA_ERROR_SPACE_IN_FOLLOWUP_TO;
#endif /* FOLLOW_USEFOR_DRAFT */
			}
			if (strchr(cp, '\n')) {
#ifdef FOLLOW_USEFOR_DRAFT
				warnings_catbp |= CA_WARNING_NEWLINE_IN_FOLLOWUP_TO;
#else
				errors_catbp |= CA_ERROR_NEWLINE_IN_FOLLOWUP_TO;
#endif /* FOLLOW_USEFOR_DRAFT */
				unfold_header(line);
			}
			while (*cp) {
				if (!(cp2 = strchr(cp, ',')))
					cp2 = cp + strlen(cp);
				else
					*cp2++ = '\0';
				if (ftngcnt < NGLIMIT) {
					ftnglens[ftngcnt] = strlen(cp);
					ftngptrs[ftngcnt] = my_malloc(ftnglens[ftngcnt] + 1);
					if (!ftngptrs[ftngcnt]) {
						/* out of memory? */
						for (i = 0; i < ftngcnt; i++)
							FreeIfNeeded(ftngptrs[i]);
						for (i = 0; i < ngcnt; i++)
							FreeIfNeeded(ngptrs[i]);
						Raw(oldraw);
						return 1;
					}
					strcpy(ftngptrs[ftngcnt], cp);
					ftngcnt++;
				}
				cp = cp2;
			}
		}
	}

	if (subject[0] == '\0')
		errors_catbp |= CA_ERROR_EMPTY_SUBJECT;
	else {
		cp2 = my_strdup(subject);
		if (!strtok(cp2, " \t")) {	/* only blanks in Subject? */
			warnings_catbp |= CA_WARNING_SPACES_ONLY_SUBJECT;
			free(cp2);
		} else {
			free(cp2);
			/* Warn if Subject: begins with "Re: " but there are no References: */
			if (!strncmp(subject, "Re: ", 4) && !saw_references)
				warnings_catbp |= CA_WARNING_RE_WITHOUT_REFERENCES;
			/*
			 * Warn if there are References: but no "Re: " at the beginning of
			 * and no "(was:" in the Subject.
			 */
			if (saw_references && strncmp(subject, "Re: ", 4)) {
				t_bool was_found = FALSE;

				cp2 = subject;
				while (!was_found && (cp2 = strchr(cp2, '(')))
					was_found = (strncmp(++cp2, "was:", 4) == 0);

				if (!was_found)
					warnings_catbp |= CA_WARNING_REFERENCES_WITHOUT_RE;
			}
		}
	}

	if (!found_from_lines)
		errors_catbp |= CA_ERROR_MISSING_FROM;
	else {
		if (found_from_lines > 1)
			errors_catbp |= CA_ERROR_DUPLICATED_FROM;
	}

	if (!found_newsgroups_lines && art_type == GROUP_TYPE_NEWS)
		errors_catbp |= CA_ERROR_MISSING_NEWSGROUPS;

	if (found_newsgroups_lines > 1)
		errors_catbp |= CA_ERROR_DUPLICATED_NEWSGROUPS;

	if (!found_subject_lines)
		errors_catbp |= CA_ERROR_MISSING_SUBJECT;
	else {
		if (found_subject_lines > 1)
			errors_catbp |= CA_ERROR_DUPLICATED_SUBJECT;
	}

	if (found_followup_to_lines > 1)
		errors_catbp |= CA_ERROR_DUPLICATED_FOLLOWUP_TO;

	/*
	 * Check the body of the article for long lines
	 * check if article contains non-7bit-ASCII characters
	 * check if sig is shorter then MAX_SIG_LINES lines
	 */
	while ((line = tin_fgets(fp, FALSE))) {
		cnt++;

		if (saw_sig_dashes || saw_wrong_sig_dashes)
			sig_lines++;

		/* SIGDASHES excluding the terminating \n as tin_fgets strips it */
		if (strlen(line) == 3 && !strncmp(line, SIGDASHES, 3)) {
			saw_wrong_sig_dashes = FALSE;
			saw_sig_dashes++;
			sig_lines = 0;
		}

		/* SIGDASHES excluding the tailing SPACE (and '\n', see comment above) */
		if (strlen(line) == 2 && !strncmp(line, SIGDASHES, 2) && !saw_sig_dashes) {
			saw_wrong_sig_dashes = TRUE;
			sig_lines = 0;
		}

#ifdef CHARSET_CONVERSION
		/* are all characters in article contained in network_charset? */
		if (strcmp(tinrc.mm_local_charset, txt_mime_charsets[mmnwcharset]) && !charset_conversion_fails) { /* local_charset != network_charset */
			cp = my_malloc(strlen(line) * 4 + 1);
			strcpy(cp, line);
			charset_conversion_fails = !buffer_to_network(cp, mmnwcharset);
			free(cp);
		}
#endif /* CHARSET_CONVERSION */

		{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			int num_bytes, wc_width;
			wchar_t wc;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

			col = 0;
			for (cp = line; *cp; ) {
				if (*cp == '\t') {
					col += 8 - (col % 8);
					cp++;
				} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					if ((num_bytes = mbtowc(&wc, cp, MB_CUR_MAX)) != -1) {
						cp += num_bytes;
						if (!contains_8bit && num_bytes > 1)
							contains_8bit = TRUE;
						if (iswprint(wc) && ((wc_width = wcwidth(wc)) != -1))
							col += wc_width;
						else
							col++;
					} else {
						cp++;
						col++;
					}
#else
					if (!contains_8bit && !isascii(*cp))
						contains_8bit = TRUE;
					cp++;
					col++;
#endif /* MULTIBYTE_ABLE && ! NO_LOCALE */
				}
			}
		}
		if (col > MAX_COL && !got_long_line) {
			setup_check_article_screen(&init);
			my_fprintf(stderr, _(txt_warn_art_line_too_long), MAX_COL, cnt, line);
			my_fflush(stderr);
			got_long_line = TRUE;
			warnings++;
		}
		if (strlen(line) > 998 && !must_break_line)
			must_break_line = cnt;
	}

#if 1
/*
 * TODO: cleanup, test me, move to the right location, strings -> lang.c, ...
 */
	if (must_break_line && strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_base64)) {
		setup_check_article_screen(&init);
#	ifdef MIME_BREAK_LONG_LINES
		if (contains_8bit) {
			if (strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_quoted_printable))
				my_fprintf(stderr, _("Line %d is longer than 998 octets and should be folded, but\nencoding is neither set to %s nor to %s\n"), must_break_line, txt_quoted_printable, txt_base64);
		} else
#	endif /* MIME_BREAK_LONG_LINES */
		{
			if (!strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_quoted_printable))
				my_fprintf(stderr, _("Line %d is longer than 998 octets, and should be folded, but\nencoding is set to %s without enabling MIME_BREAK_LONG_LINES or\nposting doesn't contain any 8bit chars and thus folding won't happen\n"), must_break_line, txt_quoted_printable);
			else
				my_fprintf(stderr, _("Line %d is longer than 998 octets, and should be folded, but\nencoding is not set to %s\n"), must_break_line, txt_base64);
		}
		my_fflush(stderr);
		warnings++;
	}
#endif /* 1 */

	if (saw_sig_dashes > 1)
		warnings_catbp |= CA_WARNING_MULTIPLE_SIGDASHES;

	if (saw_wrong_sig_dashes)
		warnings_catbp |= CA_WARNING_WRONG_SIGDASHES;

	if (sig_lines > MAX_SIG_LINES) {
		warnings_catbp |= CA_WARNING_LONG_SIGNATURE;
#ifdef HAVE_FASCIST_NEWSADMIN
		errors++;
#endif /* HAVE_FASCIST_NEWSADMIN */
	}

#ifdef CHARSET_CONVERSION
	if (charset_conversion_fails)
		warnings_catbp |= CA_WARNING_CHARSET_CONVERSION;
#endif /* CHARSET_CONVERSION */

	if (!end_of_header)
		errors_catbp |= CA_ERROR_MISSING_BODY_SEPERATOT;

	/*
	 * check for MIME Content-Type and Content-Transfer-Encoding
	 *
	 * If the user has modified the Newsgroups-header **group might not
	 * point to the correct newsgroup any more.
	 * Take first group in Newsgroups-header to pass it along to
	 * submit_news_file et.al. to use it for group-attributes, or if there is
	 * no Newsgroups:-header (mailing_list) stay with given group.
	 *
	 * Is this correct for crosspostings?
	 */
	if (ngcnt)
		*group = group_find(ngptrs[0]);

	/*
	 * check for known 7bit charsets
	 */
	for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
#ifdef CHARSET_CONVERSION
		if (!strcasecmp(txt_mime_charsets[mmnwcharset], txt_mime_7bit_charsets[i]))
#else
		if (!strcasecmp(tinrc.mm_charset, txt_mime_7bit_charsets[i]))
#endif /* CHARSET_CONVERSION */
		{
			mime_usascii = TRUE;
			break;
		}
	}
	if (strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], "7bit"))
		mime_7bit = FALSE;
	if (contains_8bit && mime_usascii)
#ifndef CHARSET_CONVERSION
		errors_catbp |= CA_ERROR_BAD_CHARSET;
#else /* we catch this case later on again */
		warnings_catbp |= CA_WARNING_CHARSET_CONVERSION;
#endif /* !CHARSET_CONVERSION */

	if (contains_8bit && mime_7bit)
		errors_catbp |= CA_ERROR_BAD_ENCODING;

	/*
	 * Warn when poster is using a non-plain encoding such as quoted-printable
	 * or base64 and external inews because if that external inews appends a
	 * signature it will not be encoded. We might additionally check if there's
	 * a file named ~/.signature and skip the warning if it is not present.
	 */
	if (((tinrc.post_mime_encoding == MIME_ENCODING_QP) || (tinrc.post_mime_encoding == MIME_ENCODING_BASE64)) && 0 != strcasecmp(tinrc.inews_prog, INTERNAL_CMD))
		warnings_catbp |= CA_WARNING_ENCODING_EXTERNAL_INEWS;

	/* give most error messages */
	if (errors_catbp) {
		setup_check_article_screen(&init);
		StartInverse();

		/* missing headers */
		if (errors_catbp & CA_ERROR_HEADER_LINE_BLANK)
			my_fprintf(stderr, _(txt_error_header_line_blank));
		if (errors_catbp & CA_ERROR_MISSING_BODY_SEPERATOT)
			my_fprintf(stderr, _(txt_error_header_and_body_not_separate));
		if (errors_catbp & CA_ERROR_MISSING_FROM)
			my_fprintf(stderr, _(txt_error_header_line_missing), "From");
		if (errors_catbp & CA_ERROR_MISSING_SUBJECT)
			my_fprintf(stderr, _(txt_error_header_line_missing), "Subject");
		if (errors_catbp & CA_ERROR_MISSING_NEWSGROUPS)
			my_fprintf(stderr, _(txt_error_header_line_missing), "Newsgroups");

		/* dublicated headers */
		if (errors_catbp & CA_ERROR_DUPLICATED_FROM)
			my_fprintf(stderr, _(txt_error_header_duplicate), found_from_lines, "From");
		if (errors_catbp & CA_ERROR_DUPLICATED_SUBJECT)
			my_fprintf(stderr, _(txt_error_header_duplicate), found_subject_lines, "Subject");
		if (errors_catbp & CA_ERROR_DUPLICATED_NEWSGROUPS)
			my_fprintf(stderr, _(txt_error_header_duplicate), found_newsgroups_lines, "Newsgroups");
		if (errors_catbp & CA_ERROR_DUPLICATED_FOLLOWUP_TO)
			my_fprintf(stderr, _(txt_error_header_duplicate), found_followup_to_lines, "Followup-To");

		/* empty headers */
		if (errors_catbp & CA_ERROR_EMPTY_SUBJECT)
			my_fprintf(stderr, _(txt_error_header_line_empty), "Subject");
		if (errors_catbp & CA_ERROR_EMPTY_NEWSGROUPS)
			my_fprintf(stderr, _(txt_error_header_line_empty), "Newsgroups");

#ifndef FOLLOW_USEFOR_DRAFT
		/* illegal space in headers */
		if (errors_catbp & CA_ERROR_SPACE_IN_NEWSGROUPS)
			my_fprintf(stderr, _(txt_error_header_line_comma), "Newsgroups");
		if (errors_catbp & CA_ERROR_SPACE_IN_FOLLOWUP_TO)
			my_fprintf(stderr, _(txt_error_header_line_comma), "Followup-To");

		/* illegal newline in headers */
		if (errors_catbp & CA_ERROR_NEWLINE_IN_NEWSGROUPS)
			my_fprintf(stderr, _(txt_error_header_line_groups_contd), "Newsgroups");
		if (errors_catbp & CA_ERROR_NEWLINE_IN_FOLLOWUP_TO)
			my_fprintf(stderr, _(txt_error_header_line_groups_contd), "Followup-To");
#endif /* !FOLLOW_USEFOR_DRAFT */

		/* encoding/charset trouble */
		if (errors_catbp & CA_ERROR_BAD_CHARSET)
			my_fprintf(stderr, _(txt_error_header_line_bad_charset));
		if (errors_catbp & CA_ERROR_BAD_ENCODING)
			my_fprintf(stderr, _(txt_error_header_line_bad_encoding));

		my_fflush(stderr);
		EndInverse();
		errors += errors_catbp;
	}

	/* give most warnings */
	if (warnings_catbp) {
		setup_check_article_screen(&init);

		if (warnings_catbp & CA_WARNING_SPACES_ONLY_SUBJECT)
			my_fprintf(stderr, _(txt_warn_blank_subject));
		if (warnings_catbp & CA_WARNING_RE_WITHOUT_REFERENCES)
			my_fprintf(stderr, _(txt_warn_re_but_no_references));
		if (warnings_catbp & CA_WARNING_REFERENCES_WITHOUT_RE)
			my_fprintf(stderr, _(txt_warn_references_but_no_re));

#ifdef FOLLOW_USEFOR_DRAFT /* TODO: give useful warning */
		if (warnings_catbp & CA_WARNING_SPACE_IN_NEWSGROUPS)
			my_fprintf(stderr, _(txt_warn_header_line_comma), "Newsgroups");
		if (warnings_catbp & CA_WARNING_SPACE_IN_FOLLOWUP_TO)
			my_fprintf(stderr, _(txt_warn_header_line_comma), "Followup-To");
		if (warnings_catbp & CA_WARNING_NEWLINE_IN_NEWSGROUPS)
			my_fprintf(stderr, _(txt_warn_header_line_groups_contd), "Newsgroups");
		if (warnings_catbp & CA_WARNING_NEWLINE_IN_FOLLOWUP_TO)
			my_fprintf(stderr, _(txt_warn_header_line_groups_contd), "Followup-To");
#endif /* FOLLOW_USEFOR_DRAFT */

		if (warnings_catbp & CA_WARNING_MULTIPLE_SIGDASHES)
			my_fprintf(stderr, _(txt_warn_multiple_sigs), saw_sig_dashes);
		if (warnings_catbp & CA_WARNING_WRONG_SIGDASHES)
			my_fprintf(stderr, _(txt_warn_wrong_sig_format));
		if (warnings_catbp & CA_WARNING_LONG_SIGNATURE)
			my_fprintf(stderr, _(txt_warn_sig_too_long), MAX_SIG_LINES);

		if (warnings_catbp & CA_WARNING_ENCODING_EXTERNAL_INEWS)
			my_fprintf(stderr, _(txt_warn_encoding_and_external_inews));

#ifdef CHARSET_CONVERSION
		if (warnings_catbp & CA_WARNING_CHARSET_CONVERSION)
			my_fprintf(stderr, _(txt_warn_charset_conversion), tinrc.mm_local_charset, txt_mime_charsets[mmnwcharset]);
#endif /* CHARSET_CONVERSION */

		my_fflush(stderr);
		warnings += warnings_catbp;
	}

	if (ngcnt && !errors) {
		/*
		 * Print a note about each newsgroup
		 */
		setup_check_article_screen(&init);
		if (art_unchanged)
			my_fprintf(stderr, _(txt_warn_article_unchanged));
		my_fprintf(stderr, _(txt_art_newsgroups), subject, PLURAL(ngcnt, txt_newsgroup));
		for (i = 0; i < ngcnt; i++) {
			if ((psGrp = group_find(ngptrs[i])))
				my_fprintf(stderr, "  %s\t %s\n", ngptrs[i], BlankIfNull(psGrp->description));
			else {
#ifdef HAVE_FASCIST_NEWSADMIN
				StartInverse();
				errors++;
				my_fprintf(stderr, _(txt_error_not_valid_newsgroup), ngptrs[i]);
				my_fflush(stderr);
				EndInverse();
#else
				my_fprintf(stderr, (!list_active ? /* did we read the whole active file? */ _(txt_warn_not_in_newsrc) : _(txt_warn_not_valid_newsgroup)), ngptrs[i]);
				warnings++;
#endif /* HAVE_FASCIST_NEWSADMIN */
			}
		}
		if (!found_followup_to_lines && ngcnt > 1 && !errors) {
#ifdef HAVE_FASCIST_NEWSADMIN
			StartInverse();
			my_fprintf(stderr, _(txt_error_missing_followup_to), ngcnt);
			my_fflush(stderr);
			EndInverse();
			errors++;
#else
			my_fprintf(stderr, _(txt_warn_missing_followup_to), ngcnt);
			warnings++;
#endif /* HAVE_FASCIST_NEWSADMIN */
		}

		if (ftngcnt && !errors) {
			if (ftngcnt > 1) {
#ifdef HAVE_FASCIST_NEWSADMIN
				StartInverse();
				my_fprintf(stderr, _(txt_error_followup_to_several_groups));
				my_fflush(stderr);
				EndInverse();
				errors++;
#else
				my_fprintf(stderr, _(txt_warn_followup_to_several_groups));
				warnings++;
#endif /* HAVE_FASCIST_NEWSADMIN */
			}
#ifdef HAVE_FASCIST_NEWSADMIN
			if (!errors) {
#endif /* HAVE_FASCIST_NEWSADMIN */
				my_fprintf(stderr, _(txt_followup_newsgroups), PLURAL(ftngcnt, txt_newsgroup));
				for (i = 0; i < ftngcnt; i++) {
					if ((psGrp = group_find(ftngptrs[i])))
						my_fprintf(stderr, "  %s\t %s\n", ftngptrs[i], BlankIfNull(psGrp->description));
					else {
						if (STRCMPEQ("poster", ftngptrs[i]))
							my_fprintf(stderr, _(txt_followup_poster), ftngptrs[i]);
						else {
#ifdef HAVE_FASCIST_NEWSADMIN
							StartInverse();
							my_fprintf(stderr, _(txt_error_not_valid_newsgroup), ftngptrs[i]);
							my_fflush(stderr);
							EndInverse();
							errors++;
#else
							my_fprintf(stderr, (!list_active ? /* did we read the whole active file? */ _(txt_warn_not_in_newsrc) : _(txt_warn_not_valid_newsgroup)), ftngptrs[i]);
							warnings++;
#endif /* HAVE_FASCIST_NEWSADMIN */
						}
					}
				}
#ifdef HAVE_FASCIST_NEWSADMIN
			}
#endif /* HAVE_FASCIST_NEWSADMIN */
		}

#ifndef NO_ETIQUETTE
		if (tinrc.beginner_level)
			my_fprintf(stderr, _(txt_warn_posting_etiquette));
#endif /* !NO_ETIQUETTE */
		my_fflush(stderr);
	}
	fclose(fp);

	Raw(oldraw);		/* restore raw/unraw state */

	/* free memory */
	for (i = 0; i < ngcnt; i++)
		FreeIfNeeded(ngptrs[i]);
	for (i = 0; i < ftngcnt; i++)
		FreeIfNeeded(ftngptrs[i]);

	return (errors ? 1 : warnings ? 2 : 0);
}


static void
setup_check_article_screen(
	int *init)
{
	if (*init) {
		ClearScreen();
		center_line(0, TRUE, _(txt_check_article));
		MoveCursor(INDEX_TOP, 0);
		Raw(FALSE);
		*init = 0;
	}
}


/*
 * edit/present an article, perform spell/PGP etc., operations if required
 * submit the article and perform all necessary backend processing
 */
static int
post_loop(
	int type,				/* type of posting */
	struct t_group *group,
	t_function func,
	const char *posting_msg, /* displayed just prior to article submission */
	int art_type,			/* news, mail etc. */
	int offset)				/* editor start offset */
{
	char a_message_id[HEADER_LEN];	/* Message-ID of the article if known */
	int ret_code = POSTED_NONE;
	int i = 1;
	long artchanged;		/* artchanged work was not done in post_postponed_article */
	struct t_group *ogroup = curr_group;
	t_bool art_unchanged;

	a_message_id[0] = '\0';

	forever {
post_article_loop:
		art_unchanged = FALSE;
		switch (func) {
			case POST_EDIT:
				/*
				 * This was VERY different in repost_article Code existed to
				 * recheck subject and restart editor, but is not enabled
				 */
				artchanged = file_mtime(article_name);
				if (!invoke_editor(article_name, offset)) {
					if (file_size(article_name) > 0L) {
						if (artchanged != file_mtime(article_name)) {
							unlink(backup_article_name(article_name));
							rename_file(article_name, dead_article);
							if (tinrc.keep_dead_articles)
								append_file(dead_articles, dead_article);
						}
					}
					goto post_article_postponed;
				}
				ret_code = POSTED_REDRAW;

				/* This might be erroneous with posting postponed */
				if (file_size(article_name) > 0L) {
					if (artchanged == file_mtime(article_name))
						art_unchanged = TRUE;
					while ((i = check_article_to_be_posted(article_name, art_type, &group, art_unchanged)) == 1 && repair_article(&func, group))
						;
					if (func == POST_EDIT || func == GLOBAL_OPTION_MENU)
						break;
				}
				/* FALLTHROUGH */

			case GLOBAL_QUIT:
			case GLOBAL_ABORT:
				if (tinrc.unlink_article) {
#if 0 /* usefull? */
					if (tinrc.keep_dead_articles)
						append_file(dead_articles, dead_article);
#endif /* 0 */
					unlink(article_name);
				}
				clear_message();
				return ret_code;

			case GLOBAL_OPTION_MENU:
				(void) change_config_file(group);
				while ((i = check_article_to_be_posted(article_name, art_type, &group, art_unchanged) == 1) && repair_article(&func, group))
					;
				break;

#ifdef HAVE_ISPELL
			case POST_ISPELL:
				invoke_ispell(article_name, group);
				ret_code = POSTED_REDRAW; /* not all versions did this */
				break;
#endif /* HAVE_ISPELL */

#ifdef HAVE_PGP_GPG
			case POST_PGP:
				invoke_pgp_news(article_name);
				break;
#endif /* HAVE_PGP_GPG */

			case GLOBAL_POST:
				wait_message(0, posting_msg);
				backup_article(article_name);

				/* Functions that didn't handle mail didn't do this */
				if (art_type == GROUP_TYPE_NEWS) {
					if (submit_news_file(article_name, group, a_message_id))
						ret_code = POSTED_OK;
				} else {
					if (submit_mail_file(article_name, group, NULL, FALSE)) /* mailing_list */
						ret_code = POSTED_OK;
				}

				if (ret_code == POSTED_OK) {
					unlink(backup_article_name(article_name));
					wait_message(2, _(txt_art_posted), *a_message_id ? a_message_id : "");
					goto post_article_done;
				} else {
					if ((func = prompt_rejected()) == POST_POSTPONE)
						/* reuse clean copy which didn't get modified by submit_news_file() */
						postpone_article(backup_article_name(article_name));
					else if (func == POST_EDIT) {
						/* replace modified article with clean backup */
						rename_file(backup_article_name(article_name), article_name);
						goto post_article_loop;
					} else {
						unlink(backup_article_name(article_name));
						rename_file(article_name, dead_article);
						if (tinrc.keep_dead_articles)
							append_file(dead_articles, dead_article);
						wait_message(2, _(txt_art_rejected), dead_article);
					}
				return ret_code;
				}

			case POST_POSTPONE:
				postpone_article(article_name);
				goto post_article_postponed;

			default:
				break;
		}
		if (type != POST_REPOST) {
			char keyedit[MAXKEYLEN], keypost[MAXKEYLEN];
			char keypostpone[MAXKEYLEN], keyquit[MAXKEYLEN];
			char keymenu[MAXKEYLEN];
#ifdef HAVE_ISPELL
			char keyispell[MAXKEYLEN];
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
			char keypgp[MAXKEYLEN];
#endif /* HAVE_PGP_GPG */

			func = prompt_slk_response((i ? POST_EDIT : art_unchanged ? POST_POSTPONE : GLOBAL_POST),
					post_post_keys, _(txt_quit_edit_post),
					printascii(keyquit, func_to_key(GLOBAL_QUIT, post_post_keys)),
					printascii(keyedit, func_to_key(POST_EDIT, post_post_keys)),
#ifdef HAVE_ISPELL
					printascii(keyispell, func_to_key(POST_ISPELL, post_post_keys)),
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
					printascii(keypgp, func_to_key(POST_PGP, post_post_keys)),
#endif /* HAVE_PGP_GPG */
					printascii(keymenu, func_to_key(GLOBAL_OPTION_MENU, post_post_keys)),
					printascii(keypost, func_to_key(GLOBAL_POST, post_post_keys)),
					printascii(keypostpone, func_to_key(POST_POSTPONE, post_post_keys)));
		} else {
			char *smsg;
			char buf[LEN];
			char keyedit[MAXKEYLEN], keypost[MAXKEYLEN];
			char keypostpone[MAXKEYLEN], keyquit[MAXKEYLEN];
			char keymenu[MAXKEYLEN];
#ifdef HAVE_ISPELL
			char keyispell[MAXKEYLEN];
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
			char keypgp[MAXKEYLEN];
#endif /* HAVE_PGP_GPG */

			snprintf(buf, sizeof(buf), _(txt_quit_edit_xpost),
					printascii(keyquit, func_to_key(GLOBAL_QUIT, post_post_keys)),
					printascii(keyedit, func_to_key(POST_EDIT, post_post_keys)),
#ifdef HAVE_ISPELL
					printascii(keyispell, func_to_key(POST_ISPELL, post_post_keys)),
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
					printascii(keypgp, func_to_key(POST_PGP, post_post_keys)),
#endif /* HAVE_PGP_GPG */
					printascii(keymenu, func_to_key(GLOBAL_OPTION_MENU, post_post_keys)),
					printascii(keypost, func_to_key(GLOBAL_POST, post_post_keys)),
					printascii(keypostpone, func_to_key(POST_POSTPONE, post_post_keys)));

			/* Superfluous force_command stuff not used in current code */
			func = ( /* force_command ? ch_default : */ prompt_slk_response(func,
						post_post_keys, "%s", sized_message(&smsg, buf,
						"" /* TODO: was note_h.subj */ )));
			free(smsg);
		}
	}

post_article_done:
	if (ret_code == POSTED_OK) {
		FILE *art_fp;
		struct t_header header;

		memset(&header, 0, sizeof(struct t_header));

		if ((art_fp = fopen(article_name, "r")) == NULL)
			perror_message(_(txt_cannot_open), article_name);
		else {
			curr_group = group;
			parse_rfc822_headers(&header, art_fp, NULL);
			fclose(art_fp);
		}

		if (art_type == GROUP_TYPE_NEWS) {
			if (header.newsgroups) {
				update_active_after_posting(header.newsgroups);
				/* In POST_RESPONSE, this was copied from note_h.newsgroups if !followup to poster */
				my_strncpy(tinrc.default_post_newsgroups, header.newsgroups, sizeof(tinrc.default_post_newsgroups) - 1);
			}
		}

		if (header.subj) {
			char tag;
			/*
			 * When crossposting postponed articles we currently do not add
			 * autoselect since we don't know which group the article was
			 * actually in
			 * FIXME: This logic is faithful to the original, but awful
			 */
			if (art_type == GROUP_TYPE_NEWS && tinrc.add_posted_to_filter && (type == POST_QUICK || type == POST_POSTPONED || type == POST_NORMAL)) {
				if ((group = group_find(header.newsgroups)) && (type != POST_POSTPONED || (type == POST_POSTPONED && !strchr(header.newsgroups, ',')))) {
					quick_filter_select_posted_art(group, header.subj, a_message_id);
					if (type == POST_QUICK || (type == POST_POSTPONED && post_postponed_and_exit))
						write_filter_file(filter_file);
				}
			}

			switch (type) {
				case POST_POSTPONED:
					tag = (header.references) ? 'f' : 'w';
					break;

				case POST_RESPONSE:
					tag = 'f';
					break;

				case POST_REPOST:
					tag = 'x';
					break;

				case POST_NORMAL:
				case POST_QUICK:
				default:
					tag = 'w';
					break;
			}

			switch (art_type) {
				case GROUP_TYPE_NEWS:
					update_posted_info_file(header.newsgroups, tag, header.subj, a_message_id);
					break;

				case GROUP_TYPE_MAIL:
					update_posted_info_file(header.to, tag, header.subj, "");
					break;

				default:
					break;
			}

			my_strncpy(tinrc.default_post_subject, header.subj, sizeof(tinrc.default_post_subject) - 1);
		}

		if (*tinrc.posted_articles_file && type != POST_REPOST) {
			char a_mailbox[LEN];
			char posted_msgs_file[PATH_LEN];

			joinpath(posted_msgs_file, tinrc.maildir, tinrc.posted_articles_file);
			/*
			 * log Message-ID if given in a_message_id,
			 * add Date:, remove empty headers
			 */
			add_headers(article_name, a_message_id);
			if (!strfpath(posted_msgs_file, a_mailbox, sizeof(a_mailbox), group))
				STRCPY(a_mailbox, posted_msgs_file);
			if (!append_mail(article_name, userid, a_mailbox)) {
				/* TODO: error message */
			}
		}
		free_and_init_header(&header);
	}

post_article_postponed:
	curr_group = ogroup;
	if (tinrc.unlink_article)
		unlink(article_name);

	return ret_code;
}


/*
 * Parse the list of newsgroups. For each, check group flag status. If it is
 * possible to post to the group and the user agrees, then keep going. Return
 * pointer to the first group in the list (the posting code needs this)
 * Any one failure => return NULL
 */
static struct t_group *
check_moderated(
	const char *groups,
	int *art_type,
	const char *failmsg)
{
	char *groupname;
	char newsgroups[HEADER_LEN];
	struct t_group *group;
	struct t_group *first_group = NULL;
	int vnum = 0, bnum = 0;

	/* Take copy - strtok() modifies its args */
	STRCPY(newsgroups, groups);

	groupname = strtok(newsgroups, ",");

	do {
		vnum++; /* number of newsgroups */

		if (!(group = group_find(groupname))) {
			bnum++;	/* number of bogus groups */
			continue;
		}

		if (!first_group)				/* Save ptr to the 1st group */
			first_group = group;

		/*
		 * Testing for !attribute here is a useful check for other brokenness
		 * Generally only bogus groups should have no attributes
		 */
		if (group->bogus) {
			error_message(_(txt_group_bogus), groupname);
			return NULL;
		}

		if (group->attribute->mailing_list != NULL)
			*art_type = GROUP_TYPE_MAIL;

		if (!can_post && *art_type == GROUP_TYPE_NEWS) {
			info_message(_(txt_cannot_post));
			return NULL;
		}

		if (group->moderated == 'x' || group->moderated == 'n') {
			error_message(_(txt_cannot_post_group), group->name);
			return NULL;
		}

		if (group->moderated == 'm') {
			char *prompt = fmt_string(_(txt_group_is_moderated), groupname);
			if (prompt_yn(prompt, TRUE) != 1) {
/*				Raw(FALSE); */
				error_message(failmsg);
				free(prompt);
				return NULL;
			}
			free(prompt);
		}
	} while ((groupname = strtok(NULL, ",")) != NULL);

	if (vnum > bnum)
		return first_group;
	else {
		error_message(_(txt_not_in_active_file), groupname);
		return NULL;
	}
}


/*
 * Build the standard headers used by quick_post_article() and post_article()
 * Return TRUE or FALSE if things went wrong - there seems to be little
 * error checking possible in here
 */
static t_bool
create_normal_article_headers(
	struct t_group *group,
	const char *newsgroups,
	int art_type)
{
	FILE *fp;
	char from_name[HEADER_LEN];
#ifdef FORGERY
	char tmp[HEADER_LEN];
#endif /* FORGERY */
	char *prompt, *tmp2;

	/* Get subject for posting article - Limit the display if needed */
	tmp2 = strunc(tinrc.default_post_subject, DISPLAY_SUBJECT_LEN);

	prompt = fmt_string(_(txt_post_subject), tmp2);

	if (!(prompt_string_default(prompt, tinrc.default_post_subject, _(txt_no_subject), HIST_POST_SUBJECT))) {
		free(prompt);
		free(tmp2);
		return FALSE;
	}
	free(prompt);
	free(tmp2);

	if ((fp = fopen(article_name, "w")) == NULL) {
		perror_message(_(txt_cannot_open), article_name);
		return FALSE;
	}

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	get_from_name(from_name, group);
#ifdef FORGERY
	make_path_header(tmp);
	msg_add_header("Path", tmp);
#endif /* FORGERY */
	msg_add_header("From", from_name);
	msg_add_header("Subject", tinrc.default_post_subject);

	if (art_type == GROUP_TYPE_MAIL)
		msg_add_header("To", group->attribute->mailing_list);
	else {
		msg_add_header("Newsgroups", newsgroups);
		ADD_MSG_ID_HEADER();
	}

	if (group->attribute->followup_to != NULL && art_type == GROUP_TYPE_NEWS)
		msg_add_header("Followup-To", group->attribute->followup_to);
	else {
		if (tinrc.prompt_followupto)
			msg_add_header("Followup-To", "");
	}

	if (*reply_to)
		msg_add_header("Reply-To", reply_to);

	if (group->attribute->organization != NULL)
		msg_add_header("Organization", random_organization(group->attribute->organization));

	if (*my_distribution && art_type == GROUP_TYPE_NEWS)
		msg_add_header("Distribution", my_distribution);

	msg_add_header("Summary", "");
	msg_add_header("Keywords", "");

	msg_add_x_headers(group->attribute->x_headers);

	start_line_offset = msg_write_headers(fp) + 1;
	fprintf(fp, "\n");			/* add a newline to keep vi from bitching */
	msg_free_headers();

	start_line_offset += msg_add_x_body(fp, group->attribute->x_body);

	msg_write_signature(fp, FALSE, group);
	fclose(fp);
	cursoron();
	return TRUE;
}


/*
 * Quick post an article (not a followup)
 */
void
quick_post_article(
	t_bool postponed_only)
{
	char buf[HEADER_LEN];
	int art_type = GROUP_TYPE_NEWS;
	struct t_group *group;

	msg_init_headers();
	ClearScreen();

	/*
	 * check for postponed articles first
	 * first param is whether to ask the user if they want to do it or not.
	 * it's opposite to the command line switch.
	 * second param is whether to assume yes to all which is the same as
	 * the command line switch.
	 */

	if (pickup_postponed_articles(!postponed_only, postponed_only) || postponed_only)
		return;

	/*
	 * Get groupname
	 */
	snprintf(buf, sizeof(buf), _(txt_post_newsgroups), tinrc.default_post_newsgroups);
	if (!(prompt_string_default(buf, tinrc.default_post_newsgroups, _(txt_no_newsgroups), HIST_POST_NEWSGROUPS)))
		return;

	/*
	 * Strip double newsgroups
	 */
	strip_double_ngs(tinrc.default_post_newsgroups);

	/*
	 * Check/see if any of the newsgroups are not postable.
	 */
	if ((group = check_moderated(tinrc.default_post_newsgroups, &art_type, _(txt_exiting))) == NULL)
		return;

	if (!create_normal_article_headers(group, tinrc.default_post_newsgroups, art_type))
		return;

	post_loop(POST_QUICK, group, POST_EDIT, _(txt_posting), art_type, start_line_offset);
}


/*
 *  Post an article that is already written (for postponed articles)
 */
static void
post_postponed_article(
	int ask,
	const char *subject,
	const char *newsgroups)
{
	char *ng;
	char *p;
	char buf[LEN];

	if (!can_post) {
		info_message(_(txt_cannot_post));
		return;
	}

	ng = my_strdup(newsgroups);
	if ((p = strchr(ng, ',')) != NULL)
		*p = '\0';

	snprintf(buf, sizeof(buf), _("Posting: %.*s ..."), cCOLS - 14, subject); /* TODO: -> lang.c, use strunc() */
	post_loop(POST_POSTPONED, group_find(ng), (ask ? POST_EDIT : GLOBAL_POST), buf, GROUP_TYPE_NEWS, 0);
	free(ng);
	return;
}


/*
 * count how many articles are in postponed.articles. Essentially,
 * we count '^From ' lines
 */
int
count_postponed_articles(
	void)
{
	FILE *fp = fopen(postponed_articles_file, "r");
	char line[HEADER_LEN];
	int count = 0;

	if (!fp)
		return 0;

	while (fgets(line, (int) sizeof(line), fp)) {
		if (strncmp(line, "From ", 5) == 0)
			count++;
	}
	fclose(fp);
	return count;
}


/*
 * Copy the first postponed article and remove it from the postponed file
 */
static t_bool
fetch_postponed_article(
	const char tmp_file[],
	char subject[],
	char newsgroups[])
{
	FILE *in, *out;
	FILE *tmp;
	char *bufp = NULL;
	char postponed_tmp[PATH_LEN];
	char line[HEADER_LEN];
	t_bool first_article;
	t_bool prev_line_nl;
	t_bool anything_left;

	snprintf(postponed_tmp, sizeof(postponed_tmp), "%s_", postponed_articles_file);
	in = fopen(postponed_articles_file, "r");
	out = fopen(tmp_file, "w");
	tmp = fopen(postponed_tmp, "w");

	if (in == NULL || out == NULL || tmp == NULL) {
		if (in)
			fclose(in);
		if (out)
			fclose(out);
		if (tmp)
			fclose(tmp);
		return FALSE;
	}

	fgets(line, (int) sizeof(line), in);

	if (strncmp(line, "From ", 5) != 0) {
		fclose(in);
		fclose(out);
		fclose(tmp);
		return FALSE;
	}

	first_article = TRUE;
	prev_line_nl = FALSE;
	anything_left = FALSE;

	/*
	 * we have one minor problem with copying the article, we have added
	 * a newline at the end of the article and we have to remove that,
	 * but we don't know we are on the last line until we read the next
	 * line containing "From "
	 */

	while (fgets(line, (int) sizeof(line), in) != NULL) {
		if (tinrc.mailbox_format == 1)
			bufp = line;
		if (strncmp(line, "From ", 5) == 0)
			first_article = FALSE;
		if (first_article) {
			match_string(line, "Newsgroups: ", newsgroups, HEADER_LEN);
			match_string(line, "Subject: ", subject, HEADER_LEN);

			if (prev_line_nl)
				fputc('\n', out);

			if (strlen(line) && line[strlen(line) - 1] == '\n') {
				prev_line_nl = TRUE;
				line[strlen(line) - 1] = '\0';
			} else
				prev_line_nl = FALSE;

			/* unquote quoted From_ lines */
			if (tinrc.mailbox_format == 1) {
				while (*bufp == '>')
					bufp++;
				if (strncmp(bufp, "From ", 5) == 0)
					fputs(line + 1, out);
				else
					fputs(line, out);
			} else {
				if (strncmp(line, ">From ", 6) == 0)
					fputs(line + 1, out);
				else
					fputs(line, out);
			}
		} else {
			fputs(line, tmp);
			anything_left = TRUE;
		}
	}

	fclose(in);
	fclose(out);
	fclose(tmp);

	unlink(postponed_articles_file);

	if (anything_left)
		rename_file(postponed_tmp, postponed_articles_file);
	else
		unlink(postponed_tmp);

	return TRUE;
}


/* pick up any postponed articles and ask if the user wants to use them */
t_bool
pickup_postponed_articles(
	t_bool ask,
	t_bool all)
{
	char newsgroups[HEADER_LEN];
	char subject[HEADER_LEN];
	char question[HEADER_LEN];
	int count = count_postponed_articles();
	int i;
	t_function func;

	if (!count) {
		if (!ask)
			info_message(_(txt_info_nopostponed));
		return FALSE;
	}

	snprintf(question, sizeof(question), _(txt_prompt_see_postponed), count);

	if (ask && prompt_yn(question, TRUE) != 1)
		return FALSE;

	for (i = 0; i < count; i++) {
		if (!fetch_postponed_article(article_name, subject, newsgroups))
			return TRUE;

		if (!all) {
			char *smsg;
			char buf[LEN];
			char keyall[MAXKEYLEN], keyno[MAXKEYLEN], keyoverride[MAXKEYLEN];
			char keyquit[MAXKEYLEN], keyyes[MAXKEYLEN];

			snprintf(buf, sizeof(buf), _(txt_postpone_repost),
					printascii(keyyes, func_to_key(PROMPT_YES, post_postpone_keys)),
					printascii(keyoverride, func_to_key(POSTPONE_OVERRIDE, post_postpone_keys)),
					printascii(keyall, func_to_key(POSTPONE_ALL, post_postpone_keys)),
					printascii(keyno, func_to_key(PROMPT_NO, post_postpone_keys)),
					printascii(keyquit, func_to_key(GLOBAL_QUIT, post_postpone_keys)));

			func = prompt_slk_response(PROMPT_YES, post_postpone_keys,
					"%s", sized_message(&smsg, buf, subject));
			free(smsg);

			if (func == POSTPONE_ALL)
				all = TRUE;
		}

		/* No else here since all changes in previous if */
		if (all)
			func = POSTPONE_OVERRIDE;

		switch (func) {
			case PROMPT_YES:
			case POSTPONE_OVERRIDE:
				post_postponed_article(func == PROMPT_YES, subject, newsgroups);
				Raw(TRUE);
				break;

			case PROMPT_NO:
			case GLOBAL_QUIT:
			case GLOBAL_ABORT:
				if (!append_mail(article_name, userid, postponed_articles_file)) {
					/* TODO: : error -message */
				}
				unlink(article_name);
				if (func != PROMPT_NO)
					return TRUE;
				break;

			default:
				break;
		}
	}
	return TRUE;
}


static void
postpone_article(
	const char *the_article)
{
	wait_message(3, _(txt_info_do_postpone));
	if (!append_mail(the_article, userid, postponed_articles_file)) {
		/* TODO: error-message */
	}
}


/*
 * Post an original article (not a followup)
 */
t_bool
post_article(
	const char *groupname)
{
	int art_type = GROUP_TYPE_NEWS;
	struct t_group *group;
	t_bool redraw_screen = FALSE;

	msg_init_headers();

	/*
	 * Check that we can post to all the groups we want to
	 */
	if ((group = check_moderated(groupname, &art_type, "")) == NULL)
		return redraw_screen;

	if (!create_normal_article_headers(group, groupname, art_type))
		return redraw_screen;

	return (post_loop(POST_NORMAL, group, POST_EDIT, _(txt_posting), art_type, start_line_offset) != POSTED_NONE);
}


/*
 * yeah, right, that's from the same Chris who is telling Jason he's
 * doing obfuscated C :-)
 */
static void
appendid(
	char **where,
	const char **what)
{
	char *oldpos;

	oldpos = *where;
	while (**what && **what != '<')
		(*what)++;
	if (**what) {
		while (**what && **what != '>' && !isspace((unsigned char) **what))
			*(*where)++ = *(*what)++;
		if (**what != '>')
			*where = oldpos;
		else {
			(*what)++;
			*(*where)++ = '>';
		}
	}
}


/*
 * check given Message-ID for "_-_@" which (should) indicate(s)
 * a Subject: change
 */
static t_bool
must_include(
	const char *id)
{
	while (*id && *id != '<')
		id++;
	while (*id && *id != '>') {
		if (*++id != '_')
			continue;
		if (*++id != '-')
			continue;
		if (*++id != '_')
			continue;
		if (*++id == '@')
			return TRUE;
	}
	return FALSE;
}


static size_t
skip_id(
	const char *id)
{
	size_t skipped = 0;
	while (id[skipped] && isspace((unsigned char) id[skipped]))
		skipped++;
	if (id[skipped]) {
		while (id[skipped] && !isspace((unsigned char) id[skipped]))
			skipped++;
	}
	return skipped;
}


static t_bool
damaged_id(
	const char *id)
{
	while (*id && isspace((unsigned char) *id))
		id++;
	if (*id != '<')
		return 1;
	while (isascii((unsigned char) *id) && isgraph((unsigned char) *id) && !iscntrl((unsigned char) *id) && *id != '>')
		id++;
	if (*id != '>')
		return TRUE;
	return FALSE;
}


/*
 * A real crossposting test had to run on Newsgroups but we only have Xref in
 * t_article, so we use this.
 */
static t_bool
is_crosspost(
	const char *xref)
{
	int count = 0;

	for (; *xref; xref++)
		if (*xref == ':')
			count++;

	return (count >= 2) ? TRUE : FALSE;
}


/*
 * with folding there would not be a length limit, but currently we don't do
 * folding and some of our code has a 2048 byte limit.  also there are
 * several newsservers out there which do have some length limit, so
 * shortening to 998 is a good idea.
 */
#define MAXREFSIZE 998


/*
 * TODO: if we have the art[x] that we are following up to, then
 *       get_references(art[x].refptr) will give us the new refs line
 */
static void
join_references(
	char *buffer,
	const char *oldrefs,
	const char *newref)
{
	/*
	 * First of all: shortening references is a VERY BAD IDEA.
	 * Nevertheless, current software usually has restrictions in
	 * header length (their programmers seem to misinterpret RFC821
	 * as valid for news, and the command length limit of RFC977
	 * as valid for headers)
	 *
	 * construct a new references line, then trim it if necessary
	 *
	 * do some sanity cleanups: remove damaged ids, make
	 * sure there is space between ids (tabs and commas are stripped)
	 *
	 * note that we're not doing strict son of RFC 1036 here: we don't
	 * take any precautions to keep the last three message ids, but
	 * it's not very likely that MAXREFSIZE chars can't hold at least
	 * 4 refs
	 */
	char *b, *c, *d;
	const char *e;
	int space = 0;

	b = my_malloc(strlen(oldrefs) + strlen(newref) + 64);
	c = b;
	e = oldrefs;

	while (*e) {
		if (*e == ' ') {
			space++, *c++ = ' ', e++;	/* keep existing spaces */
			continue;
		} else if (*e != '<') {		/* strip everything besides spaces and */
			e++;	/* message-ids */
			continue;
		}
		if (damaged_id(e)) {	/* remove damaged message ids and mark
					   the gap if that's not already done */
			e += skip_id(e);
			while (space < 3)
				space++, *c++ = ' ';

			continue;
		}
		if (!space)
			*c++ = ' ';
		else
			space = 0;
		appendid(&c, &e);
	}
	while (space)
		c--, space--;	/* remove superfluous space at the end */
	*c++ = ' ';
	appendid(&c, &newref);
	*c = 0;

	/* now see if we need to remove ids */
	while (strlen(b) > (MAXREFSIZE - strlen("References: ") - 2)) {
		c = b;
		c += skip_id(c);	/* keep the first one */
		while (*c && must_include(c))
			c += skip_id(c); /* skip those marked with _-_ */
		d = c;
		c += skip_id(c);	/* ditch one */
		*d++ = ' ';
		*d++ = ' ';
		*d++ = ' ';	/* and mark this appropriately */
		while (*c == ' ')
			c++;
		strcpy(d, c);
	}

	strcpy(buffer, b);
	free(b);
	return;

	/*
	 * son of RFC 1036 says:
	 * Followup agents SHOULD not shorten References  headers.   If
	 * it  is absolutely necessary to shorten the header, as a des-
	 * perate last resort, a followup agent MAY do this by deleting
	 * some  of  the  message IDs.  However, it MUST not delete the
	 * first message ID, the last three message IDs (including that
	 * of  the immediate precursor), or any message ID mentioned in
	 * the body of the followup.  If it is possible  for  the  fol-
	 * lowup agent to determine the Subject content of the articles
	 * identified in the References header, it MUST not delete  the
	 * message  ID of any article where the Subject content changed
	 * (other than by prepending of a back  reference).   The  fol-
	 * lowup  agent MUST not delete any message ID whose local part
	 * ends with "_-_" (underscore (ASCII 95), hyphen  (ASCII  45),
	 * underscore);  followup  agents are urged to use this form to
	 * mark subject changes, and to avoid using it otherwise.
	 * [...]
	 * When a References header is shortened, at least three blanks
	 * SHOULD be left between adjacent message IDs  at  each  point
	 * where  deletions  were  made.  Software preparing new Refer-
	 * ences headers SHOULD preserve multiple blanks in older  Ref-
	 * erences content.
	 */
}


int /* return code is currently ignored! */
post_response(
	const char *groupname,
	int respnum,
	t_bool copy_text,
	t_bool with_headers,
	t_bool raw_data)
{
	FILE *fp;
	char *ptr;
	char bigbuf[HEADER_LEN];
	char buf[HEADER_LEN];
	char from_name[HEADER_LEN];
	char initials[64];
	int art_type = GROUP_TYPE_NEWS;
	int ret_code = POSTED_NONE;
	struct t_group *group;
	struct t_header note_h = pgart.hdr;
	t_bool use_followup_to = TRUE;
#ifdef FORGERY
	char line[HEADER_LEN];
#endif /* FORGERY */
	t_function func;

	msg_init_headers();
	wait_message(0, _(txt_post_a_followup));

	/*
	 * Remove duplicates in Newsgroups and Followup-To line
	 */
	strip_double_ngs(note_h.newsgroups);
	if (note_h.followup)
		strip_double_ngs(note_h.followup);

	if (note_h.followup && STRCMPEQ(note_h.followup, "poster")) {
		char keymail[MAXKEYLEN], keypost[MAXKEYLEN], keyquit[MAXKEYLEN];

/*		clear_message(); */
		func = prompt_slk_response(PAGE_MAIL, post_mail_fup_keys, _(txt_resp_to_poster),
				printascii(keymail, func_to_key(POST_MAIL, post_mail_fup_keys)),
				printascii(keypost, func_to_key(GLOBAL_POST, post_mail_fup_keys)),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, post_mail_fup_keys)));
		switch (func) {
			case GLOBAL_POST:
				use_followup_to = FALSE;
				break;

			case GLOBAL_QUIT:
			case GLOBAL_ABORT:
				return ret_code;

			case POST_MAIL:
				return mail_to_author(groupname, respnum, copy_text, with_headers, FALSE);

			default:
				break;
		}
	} else if (note_h.followup && strcmp(note_h.followup, groupname) != 0
			&& strcmp(note_h.followup, note_h.newsgroups) != 0) {
		char keyignore[MAXKEYLEN], keypost[MAXKEYLEN], keyquit[MAXKEYLEN];

		/*
		 * note that comparing newsgroups and followup-to isn't
		 * really correct, since the order of the newsgroups may be
		 * different, but testing that also isn't really worth
		 * it. The main culprit for the duplication is tin <=1.22, BTW.
		 */
		MoveCursor(cLINES / 2, 0);
		CleartoEOS();
		center_line((cLINES / 2) + 2, TRUE, _(txt_resp_redirect));
		MoveCursor((cLINES / 2) + 4, 0);

		my_fputs("    ", stdout);
		/*
		 * TODO: check if any valid groups are in the Followup-To:-line
		 *       and if not inform the user and use Newsgroups: instead
		 */
		ptr = note_h.followup;
		while (*ptr) {
			if (*ptr != ',')
				my_fputc(*ptr, stdout);
			else {
				my_fputs(cCRLF, stdout);
				my_fputs("    ", stdout);
			}
			ptr++;
		}
		my_flush();

		func = prompt_slk_response(GLOBAL_POST, post_ignore_fupto_keys,
				_(txt_prompt_fup_ignore),
				printascii(keypost, func_to_key(GLOBAL_POST, post_ignore_fupto_keys)),
				printascii(keyignore, func_to_key(POST_IGNORE_FUPTO, post_ignore_fupto_keys)),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, post_ignore_fupto_keys)));
		switch (func) {
			case GLOBAL_QUIT:
			case GLOBAL_ABORT:
				return ret_code;

			case POST_IGNORE_FUPTO:
				use_followup_to = FALSE;
				break;

			case GLOBAL_POST:
			default:
				break;
		}
	}

	if ((fp = fopen(article_name, "w")) == NULL) {
		perror_message(_(txt_cannot_open), article_name);
		return ret_code;
	}

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	group = group_find(groupname);
	get_from_name(from_name, group);
#ifdef FORGERY
	make_path_header(line);
	msg_add_header("Path", line);
#endif /* FORGERY */
	msg_add_header("From", from_name);

	ptr = my_strdup(note_h.subj);
	snprintf(bigbuf, sizeof(bigbuf), "Re: %s", eat_re(ptr, TRUE));
	msg_add_header("Subject", bigbuf);
	free(ptr);

	if (group && group->attribute->x_comment_to && note_h.from)
		msg_add_header("X-Comment-To", note_h.from);
	if (note_h.followup && use_followup_to) {
		msg_add_header("Newsgroups", note_h.followup);
		if (tinrc.prompt_followupto)
			msg_add_header("Followup-To", (strchr(note_h.followup, ',') != NULL) ? note_h.followup : "");
	} else {
		if (group && group->attribute->mailing_list) {
			msg_add_header("To", group->attribute->mailing_list);
			art_type = GROUP_TYPE_MAIL;
		} else {
			msg_add_header("Newsgroups", note_h.newsgroups);
			if (tinrc.prompt_followupto)
				msg_add_header("Followup-To", (strchr(note_h.newsgroups, ',') != NULL) ? note_h.newsgroups : "");
			if (group && group->attribute->followup_to != NULL)
				msg_add_header("Followup-To", group->attribute->followup_to);
			else {
				if ((ptr = strchr(note_h.newsgroups, ',')))
					msg_add_header("Followup-To", note_h.newsgroups);
			}
		}
	}

	/*
	 * Append to References: line if its already there
	 */
	if (note_h.references) {
		join_references(bigbuf, note_h.references, note_h.messageid);
		msg_add_header("References", bigbuf);
	} else
		msg_add_header("References", note_h.messageid);

	if (group && group->attribute->organization != NULL)
		msg_add_header("Organization", random_organization(group->attribute->organization));

	if (*reply_to)
		msg_add_header("Reply-To", reply_to);

	if (art_type != GROUP_TYPE_MAIL) {
		ADD_MSG_ID_HEADER();
		if (note_h.distrib)
			msg_add_header("Distribution", note_h.distrib);
		else if (*my_distribution)
			msg_add_header("Distribution", my_distribution);
	}

	msg_add_x_headers(group->attribute->x_headers);

	start_line_offset = msg_write_headers(fp) + 1;
	msg_free_headers();
	start_line_offset += msg_add_x_body(fp, group->attribute->x_body);

	if (copy_text) {
		if (arts[respnum].xref && is_crosspost(arts[respnum].xref)) {
			if (strfquote(group->name, respnum, buf, sizeof(buf), tinrc.xpost_quote_format))
				fprintf(fp, "%s\n", buf);
		} else if (strfquote(groupname, respnum, buf, sizeof(buf), (group && group->attribute->news_quote_format != NULL) ? group->attribute->news_quote_format : tinrc.news_quote_format))
			fprintf(fp, "%s\n", buf);
		start_line_offset++;

		/*
		 * check if tinrc.xpost_quote_format or tinrc.news_quote_format
		 * is longer than 1 line and correct start_line_offset
		 */
		for (ptr = buf; *ptr; ptr++) {
			if (*ptr == '\n')
				++start_line_offset;
		}

		get_initials(respnum, initials, sizeof(initials));

		if (raw_data) /* rewind raw article if needed */
			fseek(pgart.raw, 0L, SEEK_SET);

		if (with_headers && raw_data)
			copy_body(pgart.raw, fp, (group ? group->attribute->quote_chars : tinrc.quote_chars), initials, TRUE);
		else {
			if (raw_data) {
				long offset = 0L;
				char buffer[8192];

				/* skip headers + header/body separator */
				while (fgets(buffer, (int) sizeof(buffer), pgart.raw) != NULL) {
					offset += strlen(buffer);
					if (buffer[0] == '\n' || buffer[0] == '\r')
						break;
				}
				fseek(pgart.raw, offset, SEEK_SET);
				copy_body(pgart.raw, fp, (group ? group->attribute->quote_chars : tinrc.quote_chars), initials, TRUE);
			} else { /* cooked art */
				resize_article(FALSE, &pgart);
				if (with_headers) {
					/*
					 * unfortunately this includes only those headers
					 * mentioned in news_headers_to_display as article
					 * cooking 'hides' all other headers
					 */
					fseek(pgart.cooked, 0L, SEEK_SET); /* rewind cooked art */
				} else { /* without headers */
					int i = 0;

					while (pgart.cookl[i].flags & C_HEADER) /* skip headers in cooked art if any */
						i++;

					if (i) /* cooked art contained any headers, so skip also the header/body seperator */
						i++;

					fseek(pgart.cooked, pgart.cookl[i].offset, SEEK_SET); /* skip headers and header/body separator */
				}
				copy_body(pgart.cooked, fp, (group ? group->attribute->quote_chars : tinrc.quote_chars), initials, FALSE);
			}
		}
	} else /* !copy_text */
		fprintf(fp, "\n");	/* add a newline to keep vi from bitching */

	msg_write_signature(fp, FALSE, group);
	fclose(fp);

	resize_article(TRUE, &pgart);	/* rebreak long lines */
	if (raw_data)	/* we've been in raw mode, reenter it */
		toggle_raw(group);

	return (post_loop(POST_RESPONSE, group, POST_EDIT, _(txt_posting), art_type, start_line_offset));
}


/*
 * Generates the basic header for a mailed article
 * Returns an open fp or NULL if article couldn't be created
 * The name of the temp. article file is written to 'filename'
 * If extra_hdrs is defined, then additional headers are added, see the code
 */
static FILE *
create_mail_headers(
	char *filename,
	const char *suffix,
	const char *to,
	const char *subject,
	struct t_header *extra_hdrs)
{
	FILE *fp;

	msg_init_headers();
	joinpath(filename, homedir, suffix);

	/* TODO: why do we exclude VMS here but nowhere else? */
#if defined(APPEND_PID) && !defined(VMS)
	snprintf(filename + strlen(filename), PATH_LEN - strlen(filename), ".%d", (int) process_id);
#endif /* APPEND_PID && !VMS */

	if ((fp = fopen(filename, "w")) == NULL) {
		perror_message(_(txt_cannot_open), filename);
		return NULL;
	}

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	if ((INTERACTIVE_NONE == tinrc.interactive_mailer) || (INTERACTIVE_WITH_HEADERS == tinrc.interactive_mailer)) {	/* tin should include headers for editing */
		char from_buf[HEADER_LEN];
		char *from_address;

		if (curr_group && curr_group->attribute && curr_group->attribute->from && strlen(curr_group->attribute->from))
			from_address = curr_group->attribute->from;
		else /* i.e. called from select.c without any groups */
			from_address = tinrc.mail_address;

		if ((from_address == NULL) || !strlen(from_address)) {
			get_from_name(from_buf, (struct t_group *) 0);
			from_address = &from_buf[0];
		} /* from_address is now always a valid pointer to a string */

		if (strlen(from_address))
			msg_add_header("From", from_address);

		msg_add_header("To", to);
		msg_add_header("Subject", subject);

		if (*reply_to)
			msg_add_header("Reply-To", reply_to);

		/*
		 * Only add own address if it is not already there.
		 *
		 * Note: get_recipients() strips out duplicated addresses later, but
		 * only for displaying; the MTA has to deal with it. They shouldn't be
		 * put in the file in the first place, so we don't do it.
		 */
		if (!address_in_list(to, strlen(from_address) ? from_address : userid)) {
			if (tinrc.auto_cc)
				msg_add_header("Cc", strlen(from_address) ? from_address : userid);

			if (tinrc.auto_bcc)
				msg_add_header("Bcc", strlen(from_address) ? from_address : userid);
		}

		if (curr_group && curr_group->attribute && curr_group->attribute->fcc && strlen(curr_group->attribute->fcc))
			msg_add_header("Fcc", curr_group->attribute->fcc);

		if (*default_organization)
			msg_add_header("Organization", random_organization(default_organization));

		if (extra_hdrs) {
			/*
			 * Write Message-ID as In-Reply-To to the mail
			 */
			msg_add_header("In-Reply-To", extra_hdrs->messageid);

			/*
			 * Rewrite Newsgroups: as X-Newsgroups: as RFC 822 doesn't define it.
			 */
			strip_double_ngs(extra_hdrs->newsgroups);
			msg_add_header("X-Newsgroups", extra_hdrs->newsgroups);
		}

		if (curr_group && curr_group->attribute && curr_group->attribute->x_headers && strlen(curr_group->attribute->x_headers))
			msg_add_x_headers(curr_group->attribute->x_headers);
	}
	start_line_offset = msg_write_headers(fp) + 1;
	msg_free_headers();

	return fp;
}


/*
 * Handle editing/spellcheck/PGP etc., operations on a mail article
 * Submit/abort the article as required and return POSTED_{NONE,REDRAW,OK}
 * Replaces core of mail_to_someone(), mail_bug_report(), mail_to_author()
 */
static int
mail_loop(
	const char *filename,		/* Temp. filename being used */
	t_function func,		/* default function */
	char *subject,
	const char *groupname,		/* Newsgroup we are posting from */
	const char *prompt,			/* If set, used for final query before posting */
	FILE *articlefp)
{
	FILE *fp;
	int ret = POSTED_NONE;
	long artchanged;
	struct t_header hdr;
	struct t_group *group = (struct t_group *) 0;
	t_bool is_changed = FALSE;
#ifdef HAVE_PGP_GPG
	char mail_to[HEADER_LEN];
#endif /* HAVE_PGP_GPG */

	if (groupname)
		group = group_find(groupname);

	forever {
		switch (func) {
			case POST_EDIT:
				artchanged = file_mtime(filename);

				if (!(invoke_editor(filename, start_line_offset)))
					return ret;

				ret = POSTED_REDRAW;
				if (((artchanged == file_mtime(filename)) && (prompt_yn(_(txt_prompt_unchanged_mail), TRUE) > 0)) || (file_size(filename) <= 0L)) {
					clear_message();
					return ret;
				}

				if (artchanged != file_mtime(filename))
					is_changed = TRUE;

				if (!(fp = fopen(filename, "r"))) { /* Oops */
					clear_message();
					return ret;
				}
				parse_rfc822_headers(&hdr, fp, NULL);
				fclose(fp);
				if (hdr.subj) {
					strncpy(subject, hdr.subj, HEADER_LEN - 1);
					subject[HEADER_LEN - 1] = '\0';
				} else
					error_message(_(txt_error_header_line_missing), "Subject");
				if (!hdr.to && !hdr.cc && !hdr.bcc)
					error_message(_(txt_error_header_line_missing), "To");
				break;

#ifdef HAVE_ISPELL
			case POST_ISPELL:
				invoke_ispell(filename, group);
/*				ret = POSTED_REDRAW; TODO: is this needed, not that REDRAW does not imply OK */
				break;
#endif /* HAVE_ISPELL */

#ifdef HAVE_PGP_GPG
			case POST_PGP:
				if (!(fp = fopen(filename, "r"))) { /* Oops */
					clear_message();
					return ret;
				}
				parse_rfc822_headers(&hdr, fp, NULL);
				fclose(fp);
				if (get_recipients(&hdr, mail_to, sizeof(mail_to) - 1))
					invoke_pgp_mail(filename, mail_to);
				else
					error_message(_(txt_error_header_line_missing), "To");
				break;
#endif /* HAVE_PGP_GPG */

			case GLOBAL_QUIT:
			case GLOBAL_ABORT:
				clear_message();
				return ret;

			case POST_SEND:
				{
					t_bool confirm = TRUE;

					if (prompt) {
						clear_message();
						if (prompt_yn(prompt, FALSE) != 1)
							confirm = FALSE;
					}

					if (confirm && submit_mail_file(filename, group, articlefp, is_changed)) {
						info_message(_(txt_articles_mailed), 1, _(txt_article_singular));
						return POSTED_OK;
					}
				}
				return ret;
				/* NOTREACHED */
				break;

			default:
				break;
		}
		func = prompt_to_send(subject);
	}

	/* NOTREACHED */
	return ret;
}


/*
 * Add the mail_quote_format string to 'fp', return the number of lines of text
 * added to the file
 */
static int
add_mail_quote(
	FILE *fp,
	int respnum)
{
	char *s;
	char buf[HEADER_LEN];
	int line_count = 0;

	if (strfquote(CURR_GROUP.name, respnum, buf, sizeof(buf), tinrc.mail_quote_format)) {
		fprintf(fp, "%s\n", buf);
		line_count++;

		for (s = buf; *s; s++) {
			if (*s == '\n')
				++line_count;
		}
	}
	return line_count;
}


/*
 * Return a POSTED_* code
 */
int
mail_to_someone(
	const char *address,
	t_bool confirm_to_mail,
	t_openartinfo *artinfo,
	const struct t_group *group)
{
	FILE *fp;
	char nam[HEADER_LEN];
	char subject[HEADER_LEN];
	int ret_code = POSTED_NONE;
	struct t_header note_h = artinfo->hdr;
	t_bool mime_forward = group->attribute->mime_forward;
	t_function func = POST_SEND;

	clear_message();
	snprintf(subject, sizeof(subject), "(fwd) %s\n", note_h.subj);

	/*
	 * don't add extra headers in the mail_to_someone() case as we include
	 * the full original headers in either the body of the mail or a separate
	 * message/rfc822 MIME part.
	 */
	if ((fp = create_mail_headers(nam, TIN_LETTER_NAME, address, subject, NULL)) == NULL)
		return ret_code;

	if (!mime_forward || INTERACTIVE_NONE != tinrc.interactive_mailer) {
		rewind(artinfo->raw);
		fprintf(fp, _(txt_forwarded));
		copy_fp(artinfo->raw, fp);
		fprintf(fp, _(txt_forwarded_end));
	}

	if (INTERACTIVE_NONE == tinrc.interactive_mailer)
		msg_write_signature(fp, TRUE, &CURR_GROUP);

	fclose(fp);

	if (INTERACTIVE_NONE != tinrc.interactive_mailer) {	/* user wants to use his own mailreader */
		char buf[HEADER_LEN];
		char *p;

		ret_code = POSTED_REDRAW;
		subject[strlen(subject) - 1] = '\0'; /* cut trailing '\n' */
		p = my_strdup(address); /* FIXME: strfmailer() won't take const arg 3 */
		strfmailer(mailer, subject, p, nam, buf, sizeof(buf), tinrc.mailer_format);
		free(p);
		if (invoke_cmd(buf))
			ret_code = POSTED_OK;
	} else {
		if (confirm_to_mail)
			func = prompt_to_send(subject);
		ret_code = mail_loop(nam, func, subject, group ? group->name : NULL, NULL, mime_forward ? artinfo->raw : NULL);
	}

	if (tinrc.unlink_article)
		unlink(nam);

	return ret_code;
}


t_bool
mail_bug_report(
	void) /* FIXME: return value is always ignored */
{
	FILE *fp;
	const char *domain;
	char buf[LEN], nam[PATH_LEN];
	char tmesg[LEN];
	char subject[HEADER_LEN];
	t_bool ret_code = FALSE;

	wait_message(0, _(txt_mail_bug_report));
	snprintf(subject, sizeof(subject), "BUG REPORT %s\n", page_header);

	if ((fp = create_mail_headers(nam, ".bugreport", bug_addr, subject, NULL)) == NULL)
		return FALSE;

	start_line_offset += tin_version_info(fp);
#if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
#	ifdef _AIX
	fprintf(fp, "BOX1 : %s %s.%s", system_info.sysname, system_info.version, system_info.release);
#	else
#		ifdef SEIUX
	fprintf(fp, "BOX1 : %s %s", system_info.version, system_info.release);
#		else
	fprintf(fp, "BOX1 : %s %s (%s)", system_info.sysname, system_info.release, system_info.machine);
#		endif /* SEIUX */
#	endif /* _AIX */
#else
	fprintf(fp, "BOX1 : Please enter the following information: Machine+OS");
#endif /* HAVE_SYS_UTSNAME_H && HAVE_UNAME */

#ifdef DOMAIN_NAME
	domain = DOMAIN_NAME;
#else
	domain = "";
#endif /* DOMAIN_NAME */

	fprintf(fp, "\nCFG1 : active=%d, arts=%d, reread=%d, nntp_xover=%s\n",
		DEFAULT_ACTIVE_NUM,
		DEFAULT_ARTICLE_NUM,
		tinrc.reread_active_file_secs,
		bool_unparse(nntp_caps.over_cmd != NULL));
	fprintf(fp, "CFG2 : debug=%d, threading=%d\n", debug, tinrc.thread_articles);
	fprintf(fp, "CFG3 : domain=[%s]\n", BlankIfNull(domain));
	start_line_offset += 4;

	if (*bug_nntpserver1) {
		fprintf(fp, "NNTP1: %s\n", bug_nntpserver1);
		start_line_offset++;
	}
	if (*bug_nntpserver2) {
		fprintf(fp, "NNTP2: %s\n", bug_nntpserver2);
		start_line_offset++;
	}
	if (nntp_caps.implementation){
		fprintf(fp, "IMPLE: %s\n", nntp_caps.implementation);
		start_line_offset++;
	}
	fprintf(fp, "\nPlease enter _detailed_ bug report, gripe or comment:\n\n");
	start_line_offset += 2;

	if (INTERACTIVE_NONE == tinrc.interactive_mailer)
		msg_write_signature(fp, TRUE, (selmenu.curr == -1) ? NULL : &CURR_GROUP);

	fclose(fp);

	if (INTERACTIVE_NONE != tinrc.interactive_mailer) {	/* user wants to use his own mailreader */
		subject[strlen(subject) - 1] = '\0';	/* cut trailing '\n' */
		strfmailer(mailer, subject, bug_addr, nam, buf, sizeof(buf), tinrc.mailer_format);
		if (invoke_cmd(buf))
			ret_code = TRUE;
	} else {
		snprintf(tmesg, sizeof(tmesg), _(txt_mail_bug_report_confirm), bug_addr);
		ret_code = mail_loop(nam, POST_EDIT, subject, NULL, tmesg, NULL) ? TRUE : FALSE;
	}

	unlink(nam);
	return ret_code;
}


int /* return value is always ignored */
mail_to_author(
	const char *group,
	int respnum,
	t_bool copy_text,
	t_bool with_headers,
	t_bool raw_data)
{
	FILE *fp;
	char *p, *q;
	char from_addr[HEADER_LEN];
	char nam[100];
	char subject[HEADER_LEN];
	char initials[64];
	int ret_code = POSTED_NONE;
	int i;
	struct t_header note_h = pgart.hdr;

	wait_message(0, _(txt_reply_to_author));
	find_reply_to_addr(from_addr, FALSE, &pgart.hdr);

	i = gnksa_check_from(from_addr);

	/* TODO: make gnksa error level configurable */
	if (check_for_spamtrap(from_addr) || (i > GNKSA_OK && i < GNKSA_ILLEGAL_UNQUOTED_CHAR)) {
		char keyabort[MAXKEYLEN], keycont[MAXKEYLEN];
		t_function func;

		func = prompt_slk_response(POST_CONTINUE, post_continue_keys,
				_(txt_warn_suspicious_mail),
				printascii(keycont, func_to_key(POST_CONTINUE, post_continue_keys)),
				printascii(keyabort, func_to_key(POST_ABORT, post_continue_keys)));
		switch (func) {
			case POST_ABORT:
			case GLOBAL_ABORT:
				clear_message();
				return ret_code;

			case POST_CONTINUE:
				break;

			/* the user wants to continue anyway, so we do nothing special here */
			default:
				break;
		}
	}

	q = p = my_strdup(note_h.subj);
	snprintf(subject, sizeof(subject), "Re: %s\n", eat_re(p, TRUE));
	free(q);

	/*
	 * add extra headers in the mail_to_author() case as we don't include the
	 * full original headers in the body of the mail
	 */
	if ((fp = create_mail_headers(nam, TIN_LETTER_NAME, from_addr, subject, &note_h)) == NULL)
		return ret_code;

	if (copy_text) {
		start_line_offset += add_mail_quote(fp, respnum);
		get_initials(respnum, initials, sizeof(initials));

		if (raw_data) /* rewind raw article if needed */
			fseek(pgart.raw, 0L, SEEK_SET);

		if (with_headers && raw_data)
			copy_body(pgart.raw, fp, tinrc.quote_chars, initials, TRUE);
		else {
			if (raw_data) { /* raw data && !with_headers */
				long offset = 0L;
				char buffer[8192];

				/* skip headers + header/body separator */
				while (fgets(buffer, (int) sizeof(buffer), pgart.raw) != NULL) {
					offset += strlen(buffer);
					if (buffer[0] == '\n' || buffer[0] == '\r')
						break;
				}
				fseek(pgart.raw, offset, SEEK_SET);
				copy_body(pgart.raw, fp, tinrc.quote_chars, initials, TRUE);
			} else { /* cooked art */
				resize_article(FALSE, &pgart);
				if (with_headers) {
					/*
					 * unfortunately this includes only those headers
					 * mentioned in news_headers_to_display as article
					 * cooking 'hides' all other headers
					 */
					fseek(pgart.cooked, 0L, SEEK_SET);
				} else { /* without headers */
					i = 0;
					while (pgart.cookl[i].flags & C_HEADER) /* skip headers in cooked art if any */
						i++;
					if (i) /* cooked art contained any headers, so skip also the header/body seperator */
						i++;
					fseek(pgart.cooked, pgart.cookl[i].offset, SEEK_SET);
				}
				copy_body(pgart.cooked, fp, tinrc.quote_chars, initials, FALSE);
			}
		}
	} else /* !copy_text */
		fprintf(fp, "\n");	/* add a newline to keep vi from bitching */

	if (INTERACTIVE_NONE == tinrc.interactive_mailer)
		msg_write_signature(fp, TRUE, &CURR_GROUP);

	fclose(fp);

	{
		char mail_to[HEADER_LEN];

		find_reply_to_addr(mail_to, TRUE, &pgart.hdr);

		if (INTERACTIVE_NONE != tinrc.interactive_mailer) {	/* user wants to use his own mailreader for reply */
			char buf[HEADER_LEN];

			subject[strlen(subject) - 1] = '\0'; /* cut trailing '\n' */
			strfmailer(mailer, subject, mail_to, nam, buf, sizeof(buf), tinrc.mailer_format);
			if (invoke_cmd(buf))
				ret_code = POSTED_OK;
		} else
			ret_code = mail_loop(nam, POST_EDIT, subject, group, NULL, NULL);

		/*
		 * If interactive_mailer!=NONE and the user changed the subject in his
		 * mailreader, the entry generated here is wrong, strictly speaking.
		 * But since we don't have a chance to get the final subject back from
		 * the mailer I think this is the best solution. -dn, 2000-03-16
		 */
		/*
		 * same with mail_to, if user changes To: in the editor tin
		 * doesn't notice it and logs the original value.
		 */
		if (ret_code == POSTED_OK)
			update_posted_info_file(mail_to, 'r', subject, ""); /* TODO: update_posted_info_file elsewhere? */
	}

	if (tinrc.unlink_article)
		unlink(nam);

	resize_article(TRUE, &pgart);	/* rebreak long lines */

	if (raw_data)	/* we've been in raw mode */
		toggle_raw(group_find(group));

	return ret_code;
}


/*
 * compare the given e-mail address with a list of components
 * in tinrc.spamtrap_warning_addresses
 */
static t_bool
check_for_spamtrap(
	const char *addr)
{
	char *env;
	char *ptr;
	char *tmp;

	if (!strlen(tinrc.spamtrap_warning_addresses) || !addr || !*addr)
		return FALSE;

	tmp = env = my_strdup(tinrc.spamtrap_warning_addresses);

	while (strlen(tmp)) {
		ptr = strchr(tmp, ',');
		if (ptr != NULL)
			*ptr = '\0';
		if (strcasestr(addr, tmp)) {
			free(env);
			return TRUE;
		}
		tmp += strlen(tmp);
		if (ptr != NULL)
			tmp++;
	}
	free(env);
	return FALSE;
}


t_bool
cancel_article(
	struct t_group *group,
	struct t_article *art,
	int respnum)
{
	FILE *fp;
	char buf[HEADER_LEN];
	char cancel[HEADER_LEN];
	char from_name[HEADER_LEN];
	char a_message_id[HEADER_LEN];
#ifdef FORGERY
	char line[HEADER_LEN];
	t_bool author = TRUE;
#else
	char user_name[128];
	char full_name[128];
#endif /* FORGERY */
	int init = 1;
	int oldraw;
	struct t_header note_h = pgart.hdr, hdr;
	t_bool redraw_screen = FALSE;
	t_function func;
	t_function default_func = POST_CANCEL;

	msg_init_headers();

	/*
	 * Check if news / mail / save group
	 */
	if (group->type == GROUP_TYPE_MAIL || group->type == GROUP_TYPE_SAVE) {
		grp_del_mail_art(art);
		return FALSE;
	}
	get_from_name(from_name, group);
#ifdef FORGERY
	make_path_header(line);
#else
	get_user_info(user_name, full_name);
#endif /* FORGERY */

#ifdef DEBUG
	if (debug == 2)
		error_message("From=[%s]  Cancel=[%s]", art->from, from_name);
#endif /* DEBUG */

	if (!strcasestr(from_name, art->from)) {
#ifdef FORGERY
		author = FALSE;
#else
		wait_message(3, _(txt_art_cannot_cancel));
		return redraw_screen;
#endif /* FORGERY */
	} else {
		char *smsg;
		char buff[LEN];
		char keycancel[MAXKEYLEN], keyquit[MAXKEYLEN], keysupersede[MAXKEYLEN];

		snprintf(buff, sizeof(buff), _(txt_cancel_article),
				printascii(keycancel, func_to_key(POST_CANCEL, post_delete_keys)),
				printascii(keysupersede, func_to_key(POST_SUPERSEDE, post_delete_keys)),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, post_delete_keys)));

		func = prompt_slk_response(default_func, post_delete_keys,
						"%s", sized_message(&smsg, buff, art->subject));
		free(smsg);

		switch (func) {
			case POST_CANCEL:
				break;

			case POST_SUPERSEDE:
				repost_article(note_h.newsgroups, respnum, TRUE, &pgart);
				return TRUE; /* force screen redraw */

			default:
				return redraw_screen;
		}
	}

	clear_message();

	joinpath(cancel, homedir, TIN_CANCEL_NAME);
#ifdef APPEND_PID
	snprintf(cancel + strlen(cancel), sizeof(cancel) - strlen(cancel), ".%d", (int) process_id);
#endif /* APPEND_PID */
	if ((fp = fopen(cancel, "w")) == NULL) {
		perror_message(_(txt_cannot_open), cancel);
		return redraw_screen;
	}

	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

#ifdef FORGERY
	if (!author) {
		char line2[HEADER_LEN];

		snprintf(line2, sizeof(line2), "cyberspam!%s", line);
		msg_add_header("Path", line2);
		msg_add_header("From", from_name);
		msg_add_header("Sender", note_h.from);
		snprintf(line, sizeof(line), "<cancel.%s", note_h.messageid + 1);
		msg_add_header("Message-ID", line);
		msg_add_header("X-Cancelled-By", from_name);
		/*
		 * Original Subject is includet in the body but some
		 * stupid bots like it in the header as well
		 */
		msg_add_header("X-Orig-Subject", note_h.subj);
	} else {
		msg_add_header("Path", line);
		if (art->name)
			snprintf(line, sizeof(line), "%s <%s>", art->name, art->from);
		else
			snprintf(line, sizeof(line), "<%s>", art->from);
		msg_add_header("From", line);
		ADD_CAN_KEY(note_h.messageid);
	}
#else
	msg_add_header("From", from_name);
	ADD_MSG_ID_HEADER();
	ADD_CAN_KEY(note_h.messageid);
#endif /* FORGERY */
	snprintf(buf, sizeof(buf), "cmsg cancel %s", note_h.messageid);
	msg_add_header("Subject", buf);

	/*
	 * remove duplicates from Newsgroups header
	 */
	strip_double_ngs(note_h.newsgroups);
	msg_add_header("Newsgroups", note_h.newsgroups);
	if (tinrc.prompt_followupto)
		msg_add_header("Followup-To", "");
	snprintf(buf, sizeof(buf), "cancel %s", note_h.messageid);
	msg_add_header("Control", buf);

	/* TODO: does this catch x-posts to moderated groups? */
	if (group->moderated == 'm')
		msg_add_header("Approved", from_name);

	if (group && group->attribute->organization != NULL)
		msg_add_header("Organization", random_organization(group->attribute->organization));

	if (note_h.distrib)
		msg_add_header("Distribution", note_h.distrib);
	else if (*my_distribution)
		msg_add_header("Distribution", my_distribution);

	/* some ppl. like X-Headers: in cancels */
	msg_add_x_headers(group->attribute->x_headers);

	start_line_offset = msg_write_headers(fp);
	msg_free_headers();

#ifdef FORGERY
	if (author)
		fprintf(fp, txt_article_cancelled);
	else {
		rewind(pgart.raw);
		copy_fp(pgart.raw, fp);
	}
	fclose(fp);
	invoke_editor(cancel, start_line_offset);
	redraw_screen = TRUE;
#else
	fprintf(fp, txt_article_cancelled);
	fclose(fp);
#endif /* FORGERY */

	oldraw = RawState();
	setup_check_article_screen(&init);

#ifdef FORGERY
	if (!author) {
		my_fprintf(stderr, _(txt_warn_cancel_forgery));
		my_fprintf(stderr, "From: %s\n", BlankIfNull(note_h.from));
	} else
#endif /* FORGERY */
	my_fprintf(stderr, _(txt_warn_cancel));

	my_fprintf(stderr, "Subject: %s\n", BlankIfNull(note_h.subj));
	my_fprintf(stderr, "Date: %s\n", BlankIfNull(note_h.date));
	my_fprintf(stderr, "Message-ID: %s\n", BlankIfNull(note_h.messageid));
	my_fprintf(stderr, "Newsgroups: %s\n", BlankIfNull(note_h.newsgroups));
	Raw(oldraw);

	if (!(fp = fopen(cancel, "r"))) {
		/* Oops */
		unlink(cancel);
		clear_message();
		return redraw_screen;
	}
	parse_rfc822_headers(&hdr, fp, NULL);
	fclose(fp);

	forever {
		{
			char *smsg;
			char buff[LEN];
			char keycancel[MAXKEYLEN], keyedit[MAXKEYLEN], keyquit[MAXKEYLEN];

			snprintf(buff, sizeof(buff), _(txt_quit_cancel),
					printascii(keyedit, func_to_key(POST_EDIT, post_cancel_keys)),
					printascii(keyquit, func_to_key(GLOBAL_QUIT, post_cancel_keys)),
					printascii(keycancel, func_to_key(POST_CANCEL, post_cancel_keys)));

			func = prompt_slk_response(default_func, post_cancel_keys, "%s", sized_message(&smsg, buff, note_h.subj));
			free(smsg);
		}

		switch (func) {
			case POST_EDIT:
				invoke_editor(cancel, start_line_offset);
				if (!(fp = fopen(cancel, "r"))) {
					/* Oops */
					unlink(cancel);
					clear_message();
					return redraw_screen;
				}
				parse_rfc822_headers(&hdr, fp, NULL);
				fclose(fp);
				break;

			case POST_CANCEL:
				wait_message(1, _(txt_cancelling_art));
				if (submit_news_file(cancel, group, a_message_id)) {
					info_message(_(txt_art_cancel));
					if (hdr.subj)
						update_posted_info_file(group->name, 'd', hdr.subj, a_message_id);
					else
						error_message(_(txt_error_header_line_missing), "Subject");
					unlink(cancel);
					return redraw_screen;
				}
				break;

			case GLOBAL_QUIT:
			case GLOBAL_ABORT:
				unlink(cancel);
				clear_message();
				return redraw_screen;
				/* NOTREACHED */
				break;

			default:
				break;
		}
	}
	/* NOTREACHED */
	return redraw_screen;
}


#ifndef FORGERY
#	define FromSameUser	(strcasestr(from_name, arts[respnum].from))
#	define NotSuperseding	(!supersede || (!FromSameUser))
#	define Superseding	(supersede && FromSameUser)
#else
#	define NotSuperseding	(!supersede)
#	define Superseding	(supersede)
#endif /* !FORGERY */

/*
 * Repost an already existing article to another group (ie. local group)
 */
int
repost_article(
	const char *groupname,
	int respnum,
	t_bool supersede,
	t_openartinfo *artinfo)
{
	FILE *fp;
	char buf[HEADER_LEN];
	char from_name[HEADER_LEN];
	char full_name[128];
	char user_name[128];
	int art_type = GROUP_TYPE_NEWS;
	int ret_code = POSTED_NONE;
	struct t_group *group;
	struct t_header note_h = artinfo->hdr;
	t_bool force_command = FALSE;
#	ifdef FORGERY
	char line[HEADER_LEN];
#	endif /* FORGERY */
	t_function func, default_func = GLOBAL_POST;

	msg_init_headers();

	/*
	 * remove duplicates from Newsgroups header
	 */
	strip_double_ngs(note_h.newsgroups);

	/*
	 * Check if any of the newsgroups are moderated.
	 */
	if ((group = check_moderated(groupname, &art_type, _(txt_art_not_posted))) == NULL)
		return ret_code;

	if ((fp = fopen(article_name, "w")) == NULL) {
		perror_message(_(txt_cannot_open), article_name);
		return ret_code;
	}
	fchmod(fileno(fp), (mode_t) (S_IRUSR|S_IWUSR));

	if (supersede) {
		get_user_info(user_name, full_name);
		get_from_name(from_name, group);
#	ifndef FORGERY
		if (FromSameUser)
#	endif /* !FORGERY */
		{
#	ifdef FORGERY
			make_path_header(line);
			msg_add_header("Path", line);

			msg_add_header("From", (note_h.from ? note_h.from : from_name));

			find_reply_to_addr(line, FALSE, &artinfo->hdr);
			if (*line)
				msg_add_header("Reply-To", line);

			msg_add_header("X-Superseded-By", from_name);

			if (note_h.org)
				msg_add_header("Organization", note_h.org);

			snprintf(line, sizeof(line), "<supersede.%s", note_h.messageid + 1);
			msg_add_header("Message-ID", line);
			/* ADD_CAN_KEY(note_h.messageid); */ /* should we add key here? */
#	else
			msg_add_header("From", from_name);
			if (*reply_to)
				msg_add_header("Reply-To", reply_to);
			ADD_MSG_ID_HEADER();
			ADD_CAN_KEY(note_h.messageid);
#	endif /* !FORGERY */
			msg_add_header("Supersedes", note_h.messageid);

			if (note_h.followup)
				msg_add_header("Followup-To", note_h.followup);

			if (note_h.keywords)
				msg_add_header("Keywords", note_h.keywords);

			if (note_h.summary)
				msg_add_header("Summary", note_h.summary);

			if (note_h.distrib)
				msg_add_header("Distribution", note_h.distrib);
		}
	} else { /* !supersede */
		get_user_info(user_name, full_name);
		get_from_name(from_name, group);
		msg_add_header("From", from_name);
		if (*reply_to)
			msg_add_header("Reply-To", reply_to);
	}
	msg_add_header("Subject", note_h.subj);
	msg_add_header("Newsgroups", groupname);
	ADD_MSG_ID_HEADER();

	if (note_h.references) {
		join_references(buf, note_h.references, (NotSuperseding ? note_h.messageid : ""));
		msg_add_header("References", buf);
	}
	if (NotSuperseding) {
		if (group->attribute->organization != NULL)
			msg_add_header("Organization", random_organization(group->attribute->organization));
		else if (*default_organization)
			msg_add_header("Organization", random_organization(default_organization));

		if (*reply_to)
			msg_add_header("Reply-To", reply_to);

		if (*my_distribution)
			msg_add_header("Distribution", my_distribution);

	} else {
		if (note_h.org)
			msg_add_header("Organization", note_h.org);
		else {
			if (group->attribute->organization != NULL)
				msg_add_header("Organization", random_organization(group->attribute->organization));
			else if (*default_organization)
				msg_add_header("Organization", random_organization(default_organization));
		}
	}

	/*
	 * some ppl. like X-Headers: in reposts
	 * X-Headers got lost on supersede, re-add
	 */
	msg_add_x_headers(group->attribute->x_headers);

	start_line_offset = msg_write_headers(fp) + 1;
	msg_free_headers();

	if (NotSuperseding) {
		fprintf(fp, "[ %-72s ]\n", _(txt_article_reposted));
		/*
		 * all string lengths are calculated to a maximum line length
		 * of 76 characters, this should look ok (sven@tin.org)
		 */
		if (note_h.from)
			fprintf(fp, "[ From: %-66s ]\n", note_h.from);
		if (note_h.subj)
			fprintf(fp, "[ Subject: %-63s ]\n", note_h.subj);
		if (note_h.newsgroups)
			fprintf(fp, "[ Newsgroups: %-60s ]\n", note_h.newsgroups);
		if (note_h.messageid)
			fprintf(fp, "[ Message-ID: %-60s ]\n\n", note_h.messageid);
	}

	{
		int i = 0;
		while (artinfo->cookl[i].flags & C_HEADER) /* skip headers in cooked art if any */
			i++;
		if (i) /* cooked art contained any headers, so skip also the header/body seperator */
			i++;
		fseek(artinfo->cooked, artinfo->cookl[i].offset, SEEK_SET);
		copy_fp(artinfo->cooked, fp);
	}

	/* only append signature when NOT superseding own articles */
	if (NotSuperseding && tinrc.signature_repost)
		msg_write_signature(fp, FALSE, group);

	fclose(fp);

	/*
	 * on supersede change default-key
	 *
	 * FIXME: this is only useful when entering the editor.
	 * After leaving the editor it should be GLOBAL_POST
	 */
	if (Superseding) {
		default_func = POST_EDIT;
		force_command = TRUE;
	}

	func = default_func;
	if (!force_command) {
		char *smsg;
		char buff[LEN];
		char keyedit[MAXKEYLEN], keypost[MAXKEYLEN];
		char keypostpone[MAXKEYLEN], keyquit[MAXKEYLEN];
		char keymenu[MAXKEYLEN];
#ifdef HAVE_ISPELL
		char keyispell[MAXKEYLEN];
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
		char keypgp[MAXKEYLEN];
#endif /* HAVE_PGP_GPG */

		snprintf(buff, sizeof(buff), _(txt_quit_edit_xpost),
				printascii(keyquit, func_to_key(GLOBAL_QUIT, post_post_keys)),
				printascii(keyedit, func_to_key(POST_EDIT, post_post_keys)),
#ifdef HAVE_ISPELL
				printascii(keyispell, func_to_key(POST_ISPELL, post_post_keys)),
#endif /* HAVE_ISPELL */
#ifdef HAVE_PGP_GPG
				printascii(keypgp, func_to_key(POST_PGP, post_post_keys)),
#endif /* HAVE_PGP_GPG */
				printascii(keymenu, func_to_key(GLOBAL_OPTION_MENU, post_post_keys)),
				printascii(keypost, func_to_key(GLOBAL_POST, post_post_keys)),
				printascii(keypostpone, func_to_key(POST_POSTPONE, post_post_keys)));

		func = prompt_slk_response(default_func, post_post_keys,
			"%s", sized_message(&smsg, buff, note_h.subj));
		free(smsg);
	}
	return (post_loop(POST_REPOST, group, func, (Superseding ? _(txt_superseding_art) : _(txt_repost_an_article)), art_type, start_line_offset));
}


static void
msg_add_x_headers(
	const char *headers)
{
	FILE *fp = NULL;
	char *ptr;
	char file[PATH_LEN];
	char line[HEADER_LEN];
	char **x_hdrs = NULL;
	int num_x_hdrs = 0;
	t_bool a_pipe = FALSE;
	int i;

	if (!headers)
		return;

	if (headers[0] != '/' && headers[0] != '~' && headers[0] != '!') {
		strcpy(line, headers);
		if ((ptr = strchr(line, ':')) != NULL) {
			*ptr++ = '\0';
			if (*ptr == ' ' || *ptr == '\t') {
				msg_add_header(line, ptr);
				return;
			}
		}
	} else {
		/*
		 * without this else a "x_headers=name" without a ':' would be
		 * treated as a filename in the current dir - IMHO not very useful
		 */
		if (!strfpath(headers, file, sizeof(file), &CURR_GROUP))
			strcpy(file, headers);

#ifndef DONT_HAVE_PIPING
		if (file[0] == '!') {
			if ((fp = popen(file + 1, "r")) == NULL)
				return;
			else
				a_pipe = TRUE;
		}
#endif /* !DONT_HAVE_PIPING */
		if (!a_pipe && ((fp = fopen(file, "r")) == NULL))
			return;

		while (fgets(line, (int) sizeof(line), fp) != NULL) {
			if (line[0] != '\n' && line[0] != '#') {
				if (line[0] != ' ' && line[0] != '\t') {
					x_hdrs = my_realloc(x_hdrs, (num_x_hdrs + 1) * sizeof(char *));
					x_hdrs[num_x_hdrs] = my_malloc(strlen(line) + 1);
					strcpy(x_hdrs[num_x_hdrs++], line);
				} else {
					if (!num_x_hdrs) /* folded line, but no previous header */
						continue;
					i = strlen(x_hdrs[num_x_hdrs - 1]);
					x_hdrs[num_x_hdrs - 1] = my_realloc(x_hdrs[num_x_hdrs - 1], i + strlen(line) + 1);
					strcpy(x_hdrs[num_x_hdrs - 1] + i, line);
				}
			}
		}

		if (num_x_hdrs) {
			for (i = 0; i < num_x_hdrs; i++) {
				if ((ptr = strchr(x_hdrs[i], ':')) != NULL) {
					*ptr++ = '\0';
					msg_add_header(x_hdrs[i], ptr);
				}
				free(x_hdrs[i]);
			}
			free(x_hdrs);
		}

		if (a_pipe)
			pclose(fp);
		else
			fclose(fp);
	}
}


/*
 * Add an x_body attribute to an article if it exists.
 * Can be a piece of text or the name of a file to append
 * Returns the # of lines appended.
 */
static int
msg_add_x_body(
	FILE *fp_out,
	const char *body)
{
	FILE *fp;
	char *ptr;
	char file[PATH_LEN];
	char line[HEADER_LEN];
	int wrote = 0;

	if (!body)
		return 0;

	if (body[0] != '/' && body[0] != '~') { /* FIXME: Unix'ism */
		strncpy(line, body, sizeof(line) - 1);
		if ((ptr = strrchr(line, '\n')) != NULL)
			*ptr = '\0';

		fprintf(fp_out, "%s\n", line);
		wrote++;
	} else {
		if (!strfpath(body, file, sizeof(file), &CURR_GROUP))
			strcpy(file, body);

		if ((fp = fopen(file, "r")) != NULL) {
			while (fgets(line, (int) sizeof(line), fp) != NULL) {
				fputs(line, fp_out);
				wrote++;
			}
			fclose(fp);
		}
	}
	if (wrote > 1) {
		fputc('\n', fp_out);
		wrote++;
	}
	return wrote;
}


/*
 * Add the User-Agent header after the other headers
 * Strip duplicate newsgroups. Only write followup header if it differs
 * from the newsgroups headers.
 * Remove Fcc header and return pointer to it. (Must be freed by
 * invoking function.)
 */
char *
checknadd_headers(
	const char *infile)
{
	FILE *fp_in, *fp_out;
	char *fcc = NULL;
	char *l;
	char *ptr;
	char newsgroups[HEADER_LEN];
	char line[HEADER_LEN];
	char outfile[PATH_LEN];

	newsgroups[0] = '\0';

	if ((fp_in = fopen(infile, "r")) == NULL)
		return NULL;

#ifdef VMS
	snprintf(outfile, sizeof(outfile), "%s-%d", infile, (int) process_id);
#else
	snprintf(outfile, sizeof(outfile), "%s.%d", infile, (int) process_id);
#endif /* VMS */

	if ((fp_out = fopen(outfile, "w")) == NULL) {
		fclose(fp_in);
		return NULL;
	}

	while ((l = tin_fgets(fp_in, TRUE)) != NULL) {
		if (l[0] == '\0') /* end of headers */
			break;

		if ((ptr = parse_header(l, "Newsgroups", FALSE, FALSE))) {
			strip_double_ngs(ptr);
			STRCPY(newsgroups, ptr);
			snprintf(line, sizeof(line), "Newsgroups: %s\n", newsgroups);
			fputs(line, fp_out);
		} else if ((ptr = parse_header(l, "Followup-To", FALSE, FALSE))) {
			strip_double_ngs(ptr);
			/*
			 * Only write followup header if not blank or followups != newsgroups
			 */
			if (*ptr && strcasecmp(newsgroups, ptr)) {
				snprintf(line, sizeof(line), "Followup-To: %s\n", ptr);
				fputs(line, fp_out);
			}
		} else if ((ptr = parse_header(l, "Fcc", FALSE, FALSE))) {
			fcc = my_strdup(ptr);
		} else if ((ptr = strchr(l, ':')) != NULL) { /* valid header? */
			if (strlen(ptr) > 3) /* skip empty headers ": \n\0" */
				fprintf(fp_out, "%s\n", l);
		}
	} /* end of headers */

	if (tinrc.advertising) {	/* Add after other headers */
		char suffix[HEADER_LEN];

		suffix[0] = '\0';
#if defined(HAVE_SYS_UTSNAME_H) && defined(HAVE_UNAME)
		if (*system_info.release) {
#	ifdef _AIX
		snprintf(suffix, sizeof(suffix), " (%s/%s.%s)",
			system_info.sysname, system_info.version, system_info.release);
#	else
#		ifdef SEIUX
			snprintf(suffix, sizeof(suffix), " (%s/%s)",
				system_info.version, system_info.release);
#		else
			snprintf(suffix, sizeof(suffix), " (%s/%s (%s))",
				system_info.sysname, system_info.release, system_info.machine);
#		endif /* SEIUX */
#	endif /* _AIX */
		}
#endif /* HAVE_SYS_UTSNAME_H && HAVE_UNAME */
#ifdef SYSTEM_NAME
		if (!*suffix) {
			if (strlen(SYSTEM_NAME))
				snprintf(suffix, sizeof(suffix), " (%s)", SYSTEM_NAME);
		}
#endif /* SYSTEM_NAME */

		fprintf(fp_out, "User-Agent: %s/%s-%s (\"%s\") (%s)%s\n",
			PRODUCT, VERSION, RELEASEDATE, RELEASENAME, OSNAME, suffix);
	}

	fputs("\n", fp_out); /* header/body seperator */

	while ((l = tin_fgets(fp_in, FALSE)) != NULL)
		fprintf(fp_out, "%s\n", l);
	fclose(fp_out);
	fclose(fp_in);
	rename_file(outfile, infile);
	return fcc;
}


static t_bool
insert_from_header(
	const char *infile)
{
	FILE *fp_in, *fp_out;
	char *line;
	char *p;
	char from_name[HEADER_LEN];
	char outfile[PATH_LEN];
	t_bool from_found = FALSE;
	t_bool in_header = TRUE;

	if ((fp_in = fopen(infile, "r")) != NULL) {
#	ifdef VMS
		snprintf(outfile, sizeof(outfile), "%s-%d", infile, (int) process_id);
#	else
		snprintf(outfile, sizeof(outfile), "%s.%d", infile, (int) process_id);
#	endif /* VMS */
		if ((fp_out = fopen(outfile, "w")) != NULL) {
			strcpy(from_name, "From: ");
			if (*tinrc.mail_address)
				strncat(from_name, tinrc.mail_address, sizeof(from_name) - 7);
			else
				get_from_name(from_name + 6, (struct t_group *) 0);

#	ifdef DEBUG
			if (debug == 2)
				wait_message(2, "insert_from_header [%s]", from_name + 6);
#	endif /* DEBUG */

			while ((line = tin_fgets(fp_in, in_header)) != NULL) {
				if (in_header && !strncasecmp(line, "From: ", 6)) {
					char from_buff[HEADER_LEN];

					from_found = TRUE;
					STRCPY(from_buff, line + 6);
					unfold_header(from_buff);

					/* Check the From: line */
					/*
					 * insert_from_header() is only called
					 * from submit_mail_file() so the 3rd
					 * arg should perhaps be TRUE
					 */
#	ifdef CHARSET_CONVERSION
					p = rfc1522_encode(from_buff, txt_mime_charsets[tinrc.mm_network_charset], FALSE);
#	else
					p = rfc1522_encode(from_buff, tinrc.mm_charset, FALSE);
#	endif /* CHARSET_CONVERSION */
					if (GNKSA_OK != gnksa_check_from(p)) { /* error in address */
						error_message(_(txt_invalid_from), from_buff);
						free(p);
						unlink(outfile);
						fclose(fp_out);
						fclose(fp_in);
						return FALSE;
					}
					free(p);
				}
				if (*line == '\0' && in_header) {
					if (!from_found) {
						/* Check the From: line */
#	ifdef CHARSET_CONVERSION
						p = rfc1522_encode(from_name, txt_mime_charsets[tinrc.mm_network_charset], FALSE);
#	else
						p = rfc1522_encode(from_name, tinrc.mm_charset, FALSE);
#	endif /* CHARSET_CONVERSION */
						if (GNKSA_OK != gnksa_check_from(p + 6)) { /* error in address */
							error_message(_(txt_invalid_from), from_name + 6);
							free(p);
							unlink(outfile);
							fclose(fp_out);
							fclose(fp_in);
							return FALSE;
						}
						free(p);
						fprintf(fp_out, "%s\n", from_name);
					}
					in_header = FALSE;
				}
				fprintf(fp_out, "%s\n", line);
			}

			fclose(fp_out);
			fclose(fp_in);
			rename_file(outfile, infile);

			return TRUE;
		}
	}
	return FALSE;
}


/*
 * Copy the appropriate reply-to address
 * from Reply-To (or From as a fallback) into 'from_addr'
 * If 'parse' is set, full syntax validation is performed and
 * the address portion is split off.
 */
static void
find_reply_to_addr(
	char *from_addr,
	t_bool parse,
	struct t_header *hdr)
{
	char fname[HEADER_LEN];
	const char *ptr;

	/*
	 * we shouldn't see any articles without a From: (and a Reply-To:) line,
	 * but for the rare case a fallback to '<>' is better than to crash.
	 */
	ptr = (hdr->replyto) ? hdr->replyto : (hdr->from ? hdr->from : "<>");

	/*
	 * We do this to save a redundant strcpy when we don't want to parse
	 */
	if (parse) {
#if 1
		/* TODO: Return code ignored? */
		parse_from(ptr, from_addr, fname);
#else
		/* Or should we decode full_addr? */
		parse_from(ptr, temp, fname);
		strcpy(full_addr, rfc1522_decode(tmp));
#endif /* 1 */
	} else
		strcpy(from_addr, ptr);
}


/*
 * If any arts have been posted by the user reread the active
 * file so that they are shown in the unread articles number
 * for each group at the group selection level.
 */
t_bool
reread_active_after_posting(
	void)
{
	int i;
	long old_min;
	long old_max;
	struct t_group *group;
	t_bool modified = FALSE;

	if (reread_active_for_posted_arts) {
		reread_active_for_posted_arts = FALSE;

		for_each_group(i) {
			if ((group = &active[i])) {
				if (group->subscribed && group->art_was_posted) {
					group->art_was_posted = FALSE;

					wait_message(0, _(txt_group_rereading), group->name);
					old_min = group->xmin;
					old_max = group->xmax;
					group_get_art_info(group->spooldir, group->name, group->type, &group->count, &group->xmax, &group->xmin);

					if (group->newsrc.num_unread > group->count) {
#ifdef DEBUG
						my_printf(cCRLF "Unread WRONG grp=[%s] unread=[%ld] count=[%ld]",
							group->name, group->newsrc.num_unread, group->count);
						my_flush();
#endif /* DEBUG */
						group->newsrc.num_unread = group->count;
					}
					if (group->xmin != old_min || group->xmax != old_max) {
#ifdef DEBUG
						my_printf(cCRLF "Min/Max DIFF grp=[%s] old=[%ld-%ld] new=[%ld-%ld]",
							group->name, old_min, old_max, group->xmin, group->xmax);
						my_flush();
#endif /* DEBUG */
						expand_bitmap(group, 0);
						modified = TRUE;
					}
					clear_message();
				}
			}
		}
	}
	return modified;
}


/*
 * If posting was successful parse the Newgroups: line and set a flag in each
 * posted to newsgroup for later processing to update num of unread articles
 */
static void
update_active_after_posting(
	char *newsgroups)
{
	char *src, *dst;
	char groupname[HEADER_LEN];
	struct t_group *group;

	/*
	 * remove duplicates from Newsgroups header
	 */
	strip_double_ngs(newsgroups);

	src = newsgroups;
	dst = groupname;

	while (*src) {
		if (*src != ' ')
			*dst = *src;

		src++;
		if (*dst == ',' || *dst == '\0') {
			*dst = '\0';
			group = group_find(groupname);
			if (group != NULL && group->subscribed) {
				reread_active_for_posted_arts = TRUE;
				group->art_was_posted = TRUE;
			}
			dst = groupname;
		} else
			dst++;
	}
}


static t_bool
submit_mail_file(
	const char *file,
	struct t_group *group,
	FILE *articlefp,
	t_bool include_text)
{
	FILE *fp;
	char *fcc;
	char buf[HEADER_LEN];
	char mail_to[HEADER_LEN];
	struct t_header hdr;
	t_bool mailed = FALSE;
#ifdef VMS
	char subject[HEADER_LEN];
#endif /* VMS */

	fcc = checknadd_headers(file);

	if (insert_from_header(file)) {
		if ((fp = fopen(file, "r"))) {
			parse_rfc822_headers(&hdr, fp, NULL);
			fclose(fp);
			if (get_recipients(&hdr, mail_to, sizeof(mail_to) - 1)) {
				wait_message(0, _(txt_mailing_to), mail_to);

				/* Use group-attribute for mailing_list */

				if (articlefp != NULL)
					compose_mail_mime_forwarded(file, articlefp, include_text, group);
				else /* text/plain */
					compose_mail_text_plain(file, group);

				strfmailer(mailer, hdr.subj, mail_to, file, buf, sizeof(buf), tinrc.mailer_format);

#ifdef VMS /* quick hack! M.St. 29.01.98 */
				{
					char *transport;

					/* TODO: document env var */
					transport = getenv("MAIL$INTERNET_TRANSPORT");
					snprintf(buf, sizeof(buf), "mail/subject=\"%s\" %s %s%%\"%s\"", subject, file, *transport ? transport : "smtp", mail_to);
				}
#endif /* VMS */
				if (invoke_cmd(buf))
					mailed = TRUE;
			} else
				error_message(_(txt_error_header_line_missing), "To");
		}
	}
	if (NULL != fcc) {
		if (mailed && strlen(fcc)) {
			char a_mailbox[PATH_LEN];

			if (0 == strfpath(fcc, a_mailbox, sizeof(a_mailbox), group))
				STRCPY(a_mailbox, fcc);
			if (!append_mail(file, userid, a_mailbox)) {
				/* TODO: error handling */
			}
		}
		FreeIfNeeded(fcc);
	}
	return mailed;
}


#ifdef FORGERY
static void
make_path_header(
	char *line)
{
	char full_name[128];
	char user_name[128];

	get_user_info(user_name, full_name);
	sprintf(line, "%s!%s", domain_name, user_name);
	return;
}
#endif /* FORGERY */


/*
 * Splits a list of e-mail addresses according to RFC 2822 into separate
 * strings containing one address each. Returns an array of pointers to
 * those strings. Side effects: changes the value of cnt to the number of
 * addresses found.
 *
 * You must free each of the strings separately. You must free the array you
 * got returned.
 */
static char **
split_address_list(
	const char *addresses,
	unsigned int *cnt)
{
	char **argv = NULL;
	char *addr;
	const char *start, *end, *curr;
	size_t len = 0, addr_len = 0;
	unsigned int argc = 0, dquotes = 0, parens = 0;

	if (!addresses) {
		*cnt = 0;
		return NULL;
	}

	len = strlen(addresses);
	curr = addresses;

	while (len > 0) {
		/* skip white space at beginning */
		while (len && isspace((int) *curr)) {
			curr++;
			len--;
		}
		if (len == 0)
			break;

		/* new address starts here */
		/*
		 * Commas are the separator between addresses. But quoted-string areas
		 * (text between double quotation marks) and comment areas (text
		 * between braces) have a special meaning where a comma is not to be
		 * treated as a separator. Inside those areas there may be
		 * quoted-pairs (backslash followed by a character that now doesn't
		 * have any special meaning). Comments may be nested, too,
		 * quoted-strings cannot.
		 */
		start = curr;
		while (len && (*curr != ',')) {
			switch (*curr) {
				case '\"':
					/* quoted-string area */
					curr++;
					len--;
					dquotes++;
					while (len && dquotes) {
						switch (*curr) {
							case '\"':
								/* end of quoted-string */
								dquotes--;
								break;

							case '\\':
								/* quoted-pair: ignore next char */
								if (len > 1) {
									curr++;
									len--;
								}
								break;

							default:
								/* nothing special, just step over it */
								break;
						}
						curr++;
						len--;
					}
					break;

				case '(':
					/* comment area */
					curr++;
					len--;
					parens++;
					while (len && parens) {
						switch (*curr) {
							case '(':
								/* comments may be nested */
								parens++;
								break;

							case ')':
								parens--;
								break;

							case '\\':
								/* quoted-pair: ignore next char */
								if (len > 1) {
									curr++;
									len--;
								}
								break;

							default:
								/* nothing special, just step over it */
								break;
						}
						curr++;
						len--;
					}
					break;

				default:
					/* nothing special, just step over it */
					break;
			}
			if (len > 0) {
				/* avoid going after end of addresses (may occur in broken address lists) */
				curr++;
				len--;
			}
		}
		/* end of address */
		end = curr;
		if (end > start) {
			end--;
			while ((end > start) && isspace((int) *end)) end--;	/* skip trailing white space */
			if (!isspace((int) *end))
				end++;
			addr_len = end - start;
			if (addr_len > 0) {
				addr = my_malloc(addr_len + 1);
				strncpy(addr, start, addr_len);
				addr[addr_len] = '\0';
				argc++;
				argv = my_realloc(argv, argc * sizeof(char *));
				argv[argc - 1] = addr;
			}
		}
		if (len > 0) {
			curr++;
			len--;
		}
	}
	/* end of buffer, end of addresses. now return array of strings */
	*cnt = argc;
	return argv;
}


/*
 * Returns TRUE if address is in addresses, FALSE otherwise. Only e-mail
 * addresses are of interest here, comments and quoted-strings are ignored.
 */
static t_bool
address_in_list(
	const char *addresses,
	const char *address)
{
	char **addr_list;
	char *curr_address = NULL, *this_address;
	t_bool found = FALSE;
	unsigned int num_addr = 0, i;

	if ((addresses == NULL) || (address == NULL))
		return FALSE;

	addr_list = split_address_list(addresses, &num_addr);
	if (num_addr == 0)
		return FALSE;

	this_address = my_malloc(strlen(address) + 1);
	strip_name(address, this_address);

	for (i = 0; i < num_addr; i++) {
		curr_address = my_realloc(curr_address, strlen(addr_list[i]) + 1);
		strip_name(addr_list[i], curr_address);
		if (!strcasecmp(curr_address, this_address))
			found = TRUE;
		FreeIfNeeded(addr_list[i]);
	}
	FreeIfNeeded(addr_list);
	FreeIfNeeded(curr_address);
	FreeIfNeeded(this_address);

	return found;
}


/*
 * Gets all recipient addresses in header, i.e. contents of To:, Cc: and
 * Bcc:, but strips out duplicates. Returns number of recipients found.
 * Side effects: changes content of buf to space separated list of recipient
 * addresses
 */
static unsigned int
get_recipients(
	struct t_header *hdr,
	char *buf,
	size_t buflen)
{
	char **to_addresses, **cc_addresses, **bcc_addresses, **all_addresses;
	char *dest, *src;
	unsigned int num_to = 0, num_cc = 0, num_bcc = 0, num_all, j = 0, i;

	/* get individual e-mail addresses from To, Cc and Bcc headers */
	to_addresses = split_address_list(hdr->to, &num_to);
	cc_addresses = split_address_list(hdr->cc, &num_cc);
	bcc_addresses = split_address_list(hdr->bcc, &num_bcc);

	if (!(num_all = num_to + num_cc + num_bcc))
		return 0;

	all_addresses = my_malloc(num_all * sizeof(char *));
	for (i = 0; i < num_to; i++, j++) {
		all_addresses[j] = my_malloc(strlen(to_addresses[i]) + 1);
		strip_name(to_addresses[i], all_addresses[j]);
	}
	for (i = 0; i < num_cc; i++, j++) {
		all_addresses[j] = my_malloc(strlen(cc_addresses[i]) + 1);
		strip_name(cc_addresses[i], all_addresses[j]);
	}
	for (i = 0; i < num_bcc; i++, j++) {
		all_addresses[j] = my_malloc(strlen(bcc_addresses[i]) + 1);
		strip_name(bcc_addresses[i], all_addresses[j]);
	}

	/* strip double addresses */
	for (i = 0; i < (num_all - 1); i++) {
		if (!all_addresses[i])
			continue;
		for (j = i + 1; j < num_all; j++) {
			if (!all_addresses[j])
				continue;
			if (!strcasecmp(all_addresses[i], all_addresses[j]))
				FreeAndNull(all_addresses[j]);
		}
	}
	/* build list of space separated e-mail addresses */
	dest = buf;
	for (i = 0; i < num_all; i++) {
		if (all_addresses[i]) {
			for (src = all_addresses[i]; (*src && buflen); src++, dest++) {
				*dest = *src;
				buflen--;
			}
			if (buflen > 0) {
				*dest++ = ' ';
				buflen--;
			}
		}
	}
	if (dest > buf)
		*--dest = '\0';

	/* free all allocated memory */
	for (i = 0; i < num_to; i++)
		FreeIfNeeded(to_addresses[i]);
	FreeIfNeeded(to_addresses);
	for (i = 0; i < num_cc; i++)
		FreeIfNeeded(cc_addresses[i]);
	FreeIfNeeded(cc_addresses);
	for (i = 0; i < num_bcc; i++)
		FreeIfNeeded(bcc_addresses[i]);
	FreeIfNeeded(bcc_addresses);
	for (i = 0; i < num_all; i++)
		FreeIfNeeded(all_addresses[i]);
	FreeIfNeeded(all_addresses);

	return num_all;
}


#ifdef EVIL_INSIDE
/*
 * build_messageid()
 * returns *(<Message-ID>)
 */
static const char *
build_messageid(
	void)
{
	int i;
	static char buf[1024]; /* Message-IDs are limited to 998-12+CRLF octets */
	static unsigned long int seqnum = 0; /* we'd use a counter in tinrc */
	time_t t = time(NULL);

	if (t >= 1041379200) /* 2003-01-01 00:00:00 GMT */
		t -= 1041379200;
	else
		return '\0';

	snprintf(buf, sizeof(buf), "<%sT", radix32(seqnum++));
	strcat(buf, radix32(t));
	strcat(buf, "I");
	strcat(buf, radix32(process_id));

#	ifndef FORGERY
	{
	/*
	 * Message ID format as suggested in
	 * draft-ietf-usefor-msg-id-alt-00, 2.1.3
	 * based on login name and FQDN
	 */
		static char buf2[1024];

		strip_name(build_sender(), buf2);
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "N%s%%%s>", radix32(getuid()), buf2);
	}
#	else
	/*
	 * Message ID format as suggested in
	 * draft-ietf-usefor-msg-id-alt-00, 2.1.1
	 * based on the host's FQDN
	 */
	snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "N%s@%s>", radix32(getuid()), get_fqdn(get_host_name()));
#	endif /* !FORGERY */

	i = gnksa_check_from(buf);
	if ((GNKSA_OK != i) && (GNKSA_LOCALPART_MISSING > i))
		buf[0] = '\0';
	return buf;
}
#endif /* EVIL_INSIDE */


/* TODO: move to canlock.c */
#ifdef USE_CANLOCK
/*
 * build_canlock(messageid, secret)
 * returns *(cancel-lock) or NULL
 */
const char *
build_canlock(
	const char *messageid,
	const char *secret)
{
	if ((messageid == NULL) || (secret == NULL))
		return ((const char *) 0);
	else
		/*
		 * sha_lock should be
		 * const char *sha_lock(const char *, size_t, const char *, size_t)
		 * but unfortunately is
		 * unsigned char *sha_lock(char *, size_t, char *, size_t)
		 * -> cast as cast can
		 */
		return (const char *) (sha_lock((char *) secret, strlen(secret), (char *) messageid, strlen(messageid)));
}


/*
 * build_cankey(messageid, secret)
 * returns *(cancel-key) or NULL
 */
static const char *
build_cankey(
	const char *messageid,
	const char *secret)
{
	if ((messageid == NULL) || (secret == NULL))
		return ((const char *) 0);
	else
		/*
		 * sha_key should be
		 * const char *sha_key(const char *, size_t, const char *, size_t)
		 * but unfortunately is
		 * unsigned char *sha_key(char *, size_t, char *, size_t)
		 * -> cast as cast can
		 */
		return (const char *) (sha_key((char *) secret, strlen(secret), (char *) messageid, strlen(messageid)));
}


/*
 * get_secret()
 * returns *(secret) or NULL
 */
#	ifdef VMS
#		define SECRET_FILE "cancelsecret."
#	else
#		define SECRET_FILE ".cancelsecret"
#	endif /* VMS */
char *
get_secret(
	void)
{
	FILE *fp_secret;
	char *ptr;
	char path_secret[PATH_LEN];
	static char cancel_secret[HEADER_LEN];

	cancel_secret[0] = '\0';
	joinpath(path_secret, homedir, SECRET_FILE);
	if ((fp_secret = fopen(path_secret, "r")) == NULL) {
#	ifdef DEBUG
		/* TODO: prompt for secret manually here? */
		my_fprintf(stderr, _(txt_cannot_open), path_secret);
		my_fflush(stderr);
		sleep(2);
#	endif /* DEBUG */
		return NULL;
	} else {
		(void) fread(cancel_secret, HEADER_LEN - 1, 1, fp_secret);
		fclose(fp_secret);
	}

	cancel_secret[HEADER_LEN - 1] = '\0';

	if ((ptr = strchr(cancel_secret, '\n')))
		*ptr = '\0';

	return cancel_secret;
}
#endif /* USE_CANLOCK */


/*
 * adds Message-ID- and Date-Header to infile, removes empty headers
 */
static void
add_headers(
	const char *infile,
	const char *a_message_id)
{
	FILE *fp_in;
	char *line;
	char outfile[PATH_LEN];
	int fd_out;
	t_bool inhdrs = TRUE, writesuccess = TRUE;
	t_bool addmid = TRUE;
	t_bool adddate = TRUE;

	if (!(*a_message_id))
		addmid = FALSE;

	if ((fp_in = fopen(infile, "r")) == NULL)
		return;

	if ((fd_out = my_tmpfile(outfile, sizeof(outfile) - 1, TRUE, homedir)) == -1) {
		fclose(fp_in);
		return;
	}

	while ((line = tin_fgets(fp_in, inhdrs)) != NULL) {
		if (inhdrs) {
			if (strlen(line) == 0) {			/* End of headers */
				inhdrs = FALSE;
				if (addmid) {
					char msgidbuf[HEADER_LEN];

					snprintf(msgidbuf, sizeof(msgidbuf), "Message-ID: %s\n", a_message_id);
					if (write(fd_out, msgidbuf, strlen(msgidbuf)) == (ssize_t) -1) /* abort on write errors */ {
						writesuccess = FALSE;
						break;
					}
				}
				if (adddate) {
					time_t epoch;
					struct tm *gmdate;
					char dateheader[50];
#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
					char *old_lc_all = NULL, *old_lc_time = NULL;

					/* Unlocalized date-header */
					if (getenv("LC_ALL") != NULL) {
						old_lc_all = my_strdup(setlocale(LC_ALL, NULL));
						setlocale(LC_ALL, "POSIX");
					} else {
						old_lc_time = my_strdup(setlocale(LC_TIME, NULL));
						setlocale(LC_TIME, "POSIX");
					}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */

					(void) time(&epoch);
					gmdate = gmtime(&epoch); /* my_strftime has no %z or %Z */
					my_strftime(dateheader, sizeof(dateheader) - 1, "Date: %a, %d %b %Y %H:%M:%S -0000\n", gmdate);

#if defined(HAVE_SETLOCALE) && !defined(NO_LOCALE)
					/* change back LC_* */
					if (old_lc_all != NULL) {
						setlocale(LC_ALL, old_lc_all);
						free(old_lc_all);
					} else if (old_lc_time != NULL) {
						setlocale(LC_TIME, old_lc_time);
						free(old_lc_time);
					}
#endif /* HAVE_SETLOCALE && !NO_LOCALE */

					if (write(fd_out, dateheader, strlen(dateheader)) == (ssize_t) -1) /* abort on write errors */ {
						writesuccess = FALSE;
						break;
					}
				}
			} else {
				char *cp;
				t_bool emptyhdr = TRUE;

				/*
				 * check_article_to_be_posted takes care that we have at
				 * least ": " in and "\n" at the end of every (unfolded) header
				 * line
				 */
				for (cp = strchr(line, ':'), cp++; *cp; cp++) {
					if (!isspace((int) *cp)) {
						emptyhdr = FALSE;
						break;
					}
				}
				if (emptyhdr)
					continue;

				if (STRNCASECMPEQ(line, "Message-ID: <", sizeof("Message-ID: <") - 1)) /* Article already contains a Message-ID Header */
					addmid = FALSE;
				else if (STRNCASECMPEQ(line, "Date: ", sizeof("Date: ") - 1)) /* Article already contains a Date Header */
					adddate = FALSE;
			}
		}
		if ((write(fd_out, line, strlen(line)) == (ssize_t) -1) || (write(fd_out, "\n", 1) == (ssize_t) -1)) /* abort on write errors */ {
			writesuccess = FALSE;
			break;
		}
	}

	close(fd_out);
	fclose(fp_in);
	if (writesuccess)
		rename_file(outfile, infile);
	else
		unlink(outfile);
}


#ifdef EVIL_INSIDE
static char *
radix32(
	unsigned long int num)
{
	static const char ralphabet[] = "0123456789abcdefghijklmnopqrstuv";
	static char tmp[20];
	char *ptr;

	ptr = tmp + sizeof(tmp) - 1;
	*ptr-- = '\0';
	if (num) {
		for (; num; num >>= 5)
			*ptr-- = ralphabet[(int) (num & 0x1f)];
	} else
		*ptr-- = ralphabet[0];
	return ++ptr;
}
#endif /* EVIL_INSIDE */


/*
 * Strip duplicate newsgroups from within a given list of comma separated
 * groups
 * 14-Jun-'96 Sven Paulus <sven@oops.sub.de>
 */
static void
strip_double_ngs(
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
	if (strchr(ngs_list, ',') != NULL) {
		over1 = FALSE;
		ncnt1 = 0;
		strcpy(newlist, ngs_list);		/* make a "working copy" */
		ptr = newlist;						/* the next outer newsg. is the 1st */

		while (!over1) {
			ncnt1++;							/* inc. outer counter */
			strcpy(cmplist, newlist);	/* duplicate groups for inner loop */
			ptr2 = strchr(ptr, ',');	/* search "," ... */
			if (ptr2 != NULL) {	/* if found ... */
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
				if (ptr2 != NULL) {
					strcpy(cmplist, ptr2 + 1);
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
