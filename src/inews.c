/*
 *  Project   : tin - a Usenet reader
 *  Module    : inews.c
 *  Author    : I. Lea
 *  Created   : 1992-03-17
 *  Updated   : 2024-12-05
 *  Notes     : NNTP built in version of inews
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>
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
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */


/*
 * local prototypes
 */
#ifdef NNTP_INEWS
	static t_bool submit_inews(const char *name, struct t_group *group, char *a_message_id);
#endif /* NNTP_INEWS */
#if defined(NNTP_INEWS) && !defined(FORGERY)
	static int sender_needed(const char *from, const char *sender, const char *charset);
#endif /* NNTP_INEWS && !FORGERY */


/*
 * Submit an article using the NNTP POST command
 *
 * TODO: remove mailheaders (To, Cc, Bcc, ...)?
 */
#ifdef NNTP_INEWS
static t_bool
submit_inews(
	const char *name,
	struct t_group *group,
	char *a_message_id)
{
	FILE *fp;
	char *line;
	char *ptr, *ptr2;
	char buf[HEADER_LEN];
	char from_name[HEADER_LEN];
	char message_id[HEADER_LEN];
	char response[NNTP_STRLEN];
	int auth_error = 0;
	int respcode;
	t_bool leave_loop;
	t_bool id_in_article = FALSE;
	t_bool ret_code = FALSE;
#	ifndef FORGERY
	char *sender_hdr = my_malloc(HEADER_LEN + 1);
	int charset;
	int sender = 0;
	int err = 0;
	t_bool ismail = FALSE;
#	endif /* !FORGERY */
#	ifdef USE_CANLOCK
	t_bool can_lock_in_article = FALSE;
#	endif /* USE_CANLOCK */

	if ((fp = tin_fopen(name, "r")) == NULL)
		goto out;

	from_name[0] = '\0';
	message_id[0] = '\0';
#	ifndef FORGERY
	sender_hdr[0] = '\0';
#	endif /* !FORGERY */

	while ((line = tin_fgets(fp, TRUE)) != NULL) {
		if (!*line) /* end of headers */
			break;

		if ((ptr = strchr(line, ':'))) {
			if (ptr - line == 4 && !strncasecmp(line, "From", 4)) {
				STRCPY(from_name, line);
			}

			if (ptr - line == 10 && !strncasecmp(line, "Message-ID", 10)) {
				STRCPY(message_id, ptr + 2);
				id_in_article = TRUE;
			}
#	ifdef USE_CANLOCK
			if (ptr - line == 11 && !strncasecmp(line, "Cancel-Lock", 11))
				can_lock_in_article = TRUE;
#	endif /* USE_CANLOCK */
		}
	}

	if ((from_name[0] == '\0') || (from_name[6] == '\0')) {
		/* we could silently add a From: line here if we want to... */
		error_message(2, _(txt_error_no_from));
		fclose(fp);
		goto out;
	}

#	ifndef FORGERY
#		ifdef CHARSET_CONVERSION
	charset = group ? group->attribute->mm_network_charset : tinrc.mm_network_charset;
#		else
	charset = 0;
#		endif /* CHARSET_CONVERSION */
	/*
	 * we should only skip the gnksa_check_from() test if we are going to
	 * post a forged cancel, but inews.c doesn't know anything about the
	 * message type, so we skip the test if FORGERY is set.
	 *
	 * TODO: check at least the local- and domainpart if post_8bit_header
	 *       is set
	 *
	 * check for valid From: line
	 */
	if (check_mailbox_list(from_name, "From:", charset, &err) > 1)
		sender = 1;

	if (err) {
		error_message(2, _(txt_invalid_from), from_name + 6);
		fclose(fp);
		goto out;
	}
#	endif /* !FORGERY */

	do {
		rewind(fp);

#	ifndef FORGERY
		if (!disable_sender && (ptr = build_sender())) {
#		ifdef CHARSET_CONVERSION
			const char *ccharset = group ? txt_mime_charsets[group->attribute->mm_network_charset] : txt_mime_charsets[tinrc.mm_network_charset];
#		else
			const char *ccharset = tinrc.mm_charset;
#		endif /* CHARSET_CONVERSION */

			if (!sender)
				sender = sender_needed(from_name + 6, ptr, ccharset);
			switch (sender) {
				case -2: /* can't build Sender: */
					error_message(2, _(txt_invalid_sender), ptr);
					fclose(fp);
					goto out;
					/* NOTREACHED */
					break;

				case -1: /* illegal From: (can't happen as check is done above already) */
					error_message(2, _(txt_invalid_from), from_name + 6);
					fclose(fp);
					goto out;
					/* NOTREACHED */
					break;

				case 1:	/* insert Sender */
					snprintf(sender_hdr, HEADER_LEN + 1, "Sender: %s", ptr);
#		ifdef CHARSET_CONVERSION
					buffer_to_network(&sender_hdr, charset);
#		endif /* CHARSET_CONVERSION */
					if (!(group ? group->attribute->post_8bit_header : tinrc.post_8bit_header)) {
						char *p = rfc1522_encode(sender_hdr, ccharset, ismail);

						strcpy(sender_hdr, p);
						free(p);
					}
					break;

				case 0: /* no sender needed */
				default:
					break;
			}
		}
#	endif /* !FORGERY */

		/*
		 * Send POST command to NNTP server
		 * Receive CONT_POST or ERROR response code from NNTP server
		 */
		if (nntp_command("POST", CONT_POST, response, sizeof(response)) == NULL) {
			error_message(2, "%s", response);
			fclose(fp);
			goto out;
		}

		/*
		 * check article if it contains a Message-ID header
		 * if not scan response line if it contains a Message-ID
		 * if it's present: use it.
		 */
		if (!*message_id) {
			/* simple syntax check - locate last '<' */
			if ((ptr = strrchr(response, '<')) != NULL) {
				/* search next '>' */
				if ((ptr2 = strchr(ptr, '>')) != NULL) {
					/* terminate string */
					*++ptr2 = '\0';
					/* check for @ and no whitespaces */
					if ((strchr(ptr, '@') != NULL) && (strpbrk(ptr, " \t") == NULL))
						my_strncpy(message_id, ptr, sizeof(message_id) - 1);	/* copy Message-ID */
				}
			}
		}

#	ifndef FORGERY
		/* Send Path: header */
		snprintf(buf, sizeof(buf), "Path: %s", PATHMASTER);
		u_put_server(buf);
		u_put_server("\r\n");

		/* and Sender: (if needed) */
		if (sender == 1 && *sender_hdr) {
			u_put_server(sender_hdr);
			u_put_server("\r\n");
		}
#	endif /* !FORGERY */

		/*
		 * check if Message-ID comes from the server
		 *
		 * IDs must be no longer than 250 chars, but ...
		 */
		if (*message_id && strlen(message_id) <= NNTP_STRLEN) {
			if (!id_in_article) {
				snprintf(buf, sizeof(buf), "Message-ID: %.*s", NNTP_STRLEN, message_id);
				u_put_server(buf);
				u_put_server("\r\n");
			}
#	ifdef USE_CANLOCK
			if (tinrc.cancel_lock_algo && !can_lock_in_article) {
				char lock[1000];
				char *lptr;

				lock[0] = '\0';
				if ((lptr = build_canlock(message_id, get_secret())) != NULL) {
					STRCPY(lock, lptr);
					free(lptr);
					snprintf(buf, sizeof(buf), "Cancel-Lock: %s", lock);
					u_put_server(buf);
					u_put_server("\r\n");
				}
			}
#	endif /* USE_CANLOCK */
		}

		/*
		 * Send article 1 line at a time ending with "."
		 */
		while ((line = tin_fgets(fp, FALSE)) != NULL) {
			/*
			 * If line starts with a '.' add another '.' to stop truncation
			 */
			if (line[0] == '.')
				u_put_server(".");

#	ifdef USE_CANLOCK
			/* skip any bogus Cancel-Locks */
			if (!*line)
				can_lock_in_article = FALSE;	/* don't touch the body */

			if (can_lock_in_article && !id_in_article) {
				ptr = strchr(line, ':');
				if (ptr - line != 11 || strncasecmp(line, "Cancel-Lock", 11)) {
					u_put_server(line);
					u_put_server("\r\n");
				}
				/* TODO: silently add a new Cancel-Lock if message_id is now known? */
			} else
#	endif /* USE_CANLOCK */
			{
				u_put_server(line);
				u_put_server("\r\n");
			}
		}

		u_put_server(".\r\n");
		put_server("", FALSE); /* flush */

		/*
		 * Receive OK_POSTED or ERROR response code from NNTP server
		 * Don't use get_respcode at this point, because then we would not
		 * recognize if posting has failed due to missing authentication in
		 * which case the complete posting has to be resent.
		 */
		respcode = get_only_respcode(response, sizeof(response));
		leave_loop = TRUE;

		/*
		 * Don't leave this loop if we only tried once to post and an
		 * authentication request was received. Leave loop on any other
		 * response or any further authentication requests.
		 *
		 * TODO: add 483 (RFC 3977) support
		 */
		if (((respcode == ERR_NOAUTH) || (respcode == NEED_AUTHINFO)) && (auth_error++ < 1) && (authenticate(nntp_server, userid, FALSE)))
			leave_loop = FALSE;
	} while (!leave_loop);

	fclose(fp);

	/*
	 * FIXME: The displayed message may be wrong if authentication has
	 * failed. (The message will be sth. like "Authentication required"
	 * which is not really wrong but misleading. The problem is that
	 * authenticate() does only return a bool value and not the server
	 * response.)
	 */
	if (respcode != OK_POSTED) {
		error_message(2, _(txt_posting_failed), str_trim(response));
		goto out;
	}

	/*
	 * scan line if it contains a Message-ID
	 */
	/* simple syntax check - locate last '<' */
	if ((ptr = strrchr(response, '<')) != NULL) {
		/* search next '>' */
		if ((ptr2 = strchr(response, '>')) != NULL) {
			/* terminate string */
			*++ptr2 = '\0';
			/* check for @ and no whitespaces */
			if ((strchr(ptr, '@') != NULL) && (strpbrk(ptr, " \t") == NULL))
				strcpy(a_message_id, ptr); /* copy Message-ID */
		}
	}

#	if 0 /* DEBUG */
	if ((debug & DEBUG_NNTP) && verbose > 1) {
		if (*message_id && *a_message_id) { /* check if purposed ID matches returned ID */
			if (strcmp(message_id, a_message_id)) {
				if (id_in_article)
					debug_print_file("NNTP", "Returned Message-ID: %s doesn't match given Message-ID: %s", a_message_id, message_id);
				else
					debug_print_file("NNTP", "Returned Message-ID: %s doesn't match purposed Message-ID: %s", a_message_id, message_id);
			}
		}
	}
#	endif /* 0 */

	if (*message_id && (id_in_article || !*a_message_id))
		strcpy(a_message_id, message_id);

	ret_code = TRUE;

out:
#	ifndef FORGERY
	FreeIfNeeded(sender_hdr);
#	else
	(void) group;
#	endif /* !FORGERY */
	return ret_code;
}
#endif /* NNTP_INEWS */


/*
 * Call submit_inews() if using built in inews, else invoke external inews
 * prog
 */
t_bool
submit_news_file(
	char *name,
	struct t_group *group,
	char *a_message_id)
{
	char buf[PATH_LEN];
	char *cp = buf;
	char *fcc = checknadd_headers(name, group);
	t_bool ret_code;
	t_bool ismail = FALSE;

	FreeIfNeeded(fcc); /* we don't use it at the moment */

	a_message_id[0] = '\0';

	rfc15211522_encode(name, txt_mime_encodings[(group ? group->attribute->post_mime_encoding : tinrc.post_mime_encoding)], group, (group ? group->attribute->post_8bit_header : tinrc.post_8bit_header), ismail);

#ifdef NNTP_INEWS
	if (read_news_via_nntp && !read_saved_news && !strcasecmp(tinrc.inews_prog, INTERNAL_CMD))
		ret_code = submit_inews(name, group, a_message_id);
	else
#endif /* NNTP_INEWS */
	{
		/* use tinrc.inews_prog or 'inewsdir/inews -h' 'inews -h' */
		if (strcasecmp(tinrc.inews_prog, INTERNAL_CMD))
			STRCPY(buf, tinrc.inews_prog);
		else {
			if (*inewsdir)
				joinpath(buf, sizeof(buf), inewsdir, "inews -h");
			else
				strcpy(buf, "inews -h");
		}
		cp += strlen(cp);
		sh_format(cp, sizeof(buf) - (size_t) (cp - buf), " < %s", name);

		ret_code = invoke_cmd(buf);

#ifdef NNTP_INEWS
		if (!ret_code && read_news_via_nntp && !read_saved_news && strcasecmp(tinrc.inews_prog, INTERNAL_CMD)) {
			if (prompt_yn(_(txt_post_via_builtin_inews), TRUE)) {
				if ((ret_code = submit_inews(name, group, a_message_id)) == TRUE) {
					if (prompt_yn(_(txt_post_via_builtin_inews_only), TRUE) == 1) {
						FreeIfNeeded(tinrc.inews_prog);
						tinrc.inews_prog = my_strdup(INTERNAL_CMD);
					}
				}
			}
		}
#endif /* NNTP_INEWS */
	}
	return ret_code;
}


/*
 * returnvalues:  1 = Sender needed
 *                0 = no Sender needed
 *               -1 = error (no '.' and/or '@' in From) [unused]
 *               -2 = error (no '.' and/or '@' in Sender)
 */
#if defined(NNTP_INEWS) && !defined(FORGERY)
static int
sender_needed(
	const char *from,
	const char *sender,
	const char *charset)
{
	const char *from_at_pos;
	const char *sender_at_pos;
	const char *sender_dot_pos;
	char *p;
	char from_addr[HEADER_LEN];
	char sender_addr[HEADER_LEN];
	char sender_line[HEADER_LEN];
	char sender_name[HEADER_LEN];
	int r;

#	ifdef DEBUG
	if (debug & DEBUG_MISC) {
		wait_message(3, "sender_needed From:=[%s]", from);
		wait_message(3, "sender_needed Sender:=[%s]", sender);
	}
#	endif /* DEBUG */

	/* extract address */
	strip_name(from, from_addr);

	snprintf(sender_line, sizeof(sender_line), "Sender: %s", sender);

	p = rfc1522_encode(sender_line, charset, FALSE);
	r = gnksa_do_check_from(p + 8, sender_addr, sender_name);
	if (r > GNKSA_OK && r < GNKSA_MISSING_REALNAME) {
		free(p);
		return -2;
	}
	free(p);

	from_at_pos = strchr(from_addr, '@');
	if ((sender_at_pos = strchr(sender_addr, '@')))
		sender_dot_pos = strchr(sender_at_pos, '.');
	else /* this case is caught by the gnksa_do_check_from() code above; anyway ... */
		return -2;

	if (from_at_pos == NULL || sender_dot_pos == NULL) /* as we build From and check Sender above this shouldn't happen at all */
		return -2;

	if (strncasecmp(from_addr, sender_addr, (size_t) (from_at_pos - from_addr)))
		return 1; /* login differs */

	if (strcasecmp(from_at_pos, sender_at_pos) && (strcasecmp(from_at_pos + 1, sender_dot_pos + 1)))
		return 1; /* domainname differs */

	return 0;
}
#endif /* NNTP_INEWS && !FORGERY */
