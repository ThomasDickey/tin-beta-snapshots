/*
 *  Project   : tin - a Usenet reader
 *  Module    : inews.c
 *  Author    : I. Lea
 *  Created   : 1992-03-17
 *  Updated   : 2002-04-17
 *  Notes     : NNTP built in version of inews
 *
 * Copyright (c) 1991-2002 Iain Lea <iain@bricbrac.de>
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
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */

#if defined(NNTP_INEWS) && !defined(FORGERY)
#	define PATHMASTER	"not-for-mail"
#endif /* NNTP_INEWS && !FORGERY */


/*
 * local prototypes
 */
#ifdef NNTP_INEWS
	static t_bool submit_inews(char *name, struct t_group *group, char *a_message_id);
#endif /* NNTP_INEWS */
#if defined(NNTP_INEWS) && !defined(FORGERY)
	static int sender_needed(char *from, struct t_group *group, char *sender);
#endif /* NNTP_INEWS && !FORGERY */

#if 0
#	ifdef VMS
#		ifdef MULTINET
#			define netwrite	socket_write
#			include "multinet_root:[multinet.include]netdb.h"
#		else
#			define netwrite	write
#			include <netdb.h>
#		endif /* MULTINET */
#	else
#		ifdef HAVE_NETDB_H
#			include <netdb.h>
#		endif /* HAVE_NETDB_H */
#	endif /* VMS */

#	ifdef HAVE_SYS_SOCKET_H
#		include <sys/socket.h>
#	endif /* HAVE_SYS_SOCKET_H */
#	ifdef HAVE_NETINET_IN_H
#		include <netinet/in.h>
#	endif /* HAVE_NETINET_IN_H */
#endif /* 0 */


/*
 * Submit an article using the NNTP POST command
 */
#ifdef NNTP_INEWS
static t_bool
submit_inews(
	char *name,
	struct t_group *group,
	char *a_message_id)
{
	FILE *fp;
	char *ptr, *ptr2;
	char from_name[HEADER_LEN];
	char message_id[HEADER_LEN];
	char line[HEADER_LEN];
	int auth_error = 0;
	int respcode;
	t_bool leave_loop = FALSE;
	t_bool id_in_article = FALSE;
	t_bool ret_code = FALSE;
#	ifndef FORGERY
	char sender_hdr[HEADER_LEN];
	int sender = 0;
	t_bool ismail = FALSE;
#	endif /* !FORGERY */
#	ifdef USE_CANLOCK
	t_bool can_lock_in_article = FALSE;
#	endif /* USE_CANLOCK */

	if ((fp = fopen(name, "r")) == NULL) {
		perror_message(_(txt_cannot_open), name);
		return ret_code;
	}

	from_name[0] = '\0';
	message_id[0] = '\0';

	while (fgets(line, (int) sizeof(line), fp) != NULL) {
		if (line[0] != '\n') {
			ptr = strchr(line, ':');
			if (ptr - line == 4 && !strncasecmp(line, "From", 4)) {
				strcpy(from_name, line);
				if ((ptr = strchr(from_name, '\n')))
					*ptr = '\0';
			}
			if (ptr - line == 10 && !strncasecmp(line, "Message-ID", 10)) {
				strcpy(message_id, ptr + 2);
				id_in_article = TRUE;
				if ((ptr = strchr(message_id, '\n')))
					*ptr = '\0';
			}
#	ifdef USE_CANLOCK
			if (ptr - line == 11 && !strncasecmp(line, "Cancel-Lock", 11))
				can_lock_in_article = TRUE;
#	endif /* USE_CANLOCK */
		} else
			break; /* end of headers */
	}

	if ((from_name[0] == '\0') || (from_name[6] == '\0')) {
		/* we could silently add a From: line here if we want to... */
		error_message(_(txt_error_no_from));
		fclose(fp);
		return ret_code;
	}

#	ifndef FORGERY
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
	if (!tinrc.post_8bit_header && GNKSA_OK != gnksa_check_from(from_name + 6)) { /* error in address */
		error_message(_(txt_invalid_from), from_name + 6);
		fclose(fp);
		return ret_code;
	}
#	endif /* !FORGERY */

	do {
		rewind(fp);

#	ifndef FORGERY
		if ((ptr = build_sender()) && (!disable_sender)) {
			sender = sender_needed(from_name + 6, group, ptr);
			switch (sender) {
				case -2: /* can't build Sender: */
					error_message(_(txt_invalid_sender), ptr);
					fclose(fp);
					return ret_code;
					/* NOTREACHED */
					break;

				case -1: /* illegal From: (can't happen as check is done above already) */
					error_message(_(txt_invalid_from), from_name + 6);
					fclose(fp);
					return ret_code;
					/* NOTREACHED */
					break;

				case 1:	/* insert Sender */
					snprintf(sender_hdr, sizeof(sender_hdr), "Sender: %s", ptr);
#		if defined(LOCAL_CHARSET) || defined(MAC_OS_X)
					buffer_to_network(sender_hdr, 0);
#		endif /* LOCAL_CHARSET || MAC_OS_X */
#		ifdef CHARSET_CONVERSION
					buffer_to_network(sender_hdr, group ? group->attribute->mm_network_charset : tinrc.mm_network_charset);
#		endif /* CHARSET_CONVERSION */
					if (!tinrc.post_8bit_header)
						STRCPY(sender_hdr, rfc1522_encode(sender_hdr, group, ismail));
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
		if (nntp_command("POST", CONT_POST, line, sizeof(line)) == NULL) {
			error_message("%s", line);
			fclose(fp);
			return ret_code;
		}

		/*
		 * check article if it contains a Message-ID header
		 * if not scan line if it contains a Message-ID
		 * if it's present: use it.
		 */
		if (message_id[0] == '\0') {
			/* simple syntax check - locate last '<' */
			if ((ptr = strrchr(line, '<')) != NULL) {
				/* search next '>' */
				if ((ptr2 = strchr(ptr, '>')) != NULL) {
					/* terminate string */
					*++ptr2 = '\0';
					/* check for @ and no whitespaces */
					if ((strchr(ptr, '@') != NULL) && (strpbrk(ptr, " \t") == NULL))
						strcpy(message_id, ptr);	/* copy Message-ID */
				}
			}
		}

#	ifndef FORGERY
		/*
		 * Send Path: (and Sender: if needed) headers
		 */
		sprintf(line, "Path: %s", PATHMASTER);
		put_server(line);

		if (sender == 1)
			put_server(sender_hdr);
#	endif /* !FORGERY */

		/*
		 * check if Message-ID comes from the server
		 */
		if (*message_id) {
			if (!id_in_article) {
				sprintf(line, "Message-ID: %s", message_id);
				put_server(line);
			}
#	ifdef USE_CANLOCK
			if (!can_lock_in_article) {
					char lock[1024];
					const char *lptr = (const char *) 0;

					lock[0] = '\0';
					if ((lptr = build_canlock(message_id, get_secret())) != NULL) {
						STRCPY(lock, lptr);
						sprintf(line, "Cancel-Lock: %s", lock);
						put_server(line);
					}
				}
#	endif /* USE_CANLOCK */
		}

		/*
		 * Send article 1 line at a time ending with "."
		 */
		while (fgets(line, (int) sizeof(line), fp) != NULL) {
			/*
			 * Remove linefeed from line
			 */
			if ((ptr = strrchr(line, '\n')) != NULL)
				*ptr = '\0';

			/*
			 * If line starts with a '.' add another '.' to stop truncation
			 */
			if (line[0] == '.')
				u_put_server(".");

#	ifdef USE_CANLOCK
			/* skip any bogus Cancel-Locks */
			if (!strlen(line))
				can_lock_in_article = FALSE;	/* don't touch the body */

			if (can_lock_in_article && !id_in_article) {
				ptr = strchr(line, ':');
				if (ptr - line == 11 && !strncasecmp(line, "Cancel-Lock", 11)) {
					; /* skip line */
				} else {
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

		put_server(".");

		/*
		 * Receive OK_POSTED or ERROR response code from NNTP server
		 * Don't use get_respcode at this point, because then we would not
		 * recognize if posting has failed due to missing authentication in
		 * which case the complete posting has to be resent. Besides, because
		 * of the put_server(".") above a "." would be resent as the last
		 * "command".
		 */
		respcode = get_only_respcode(line, sizeof(line));
		leave_loop = TRUE;

		/*
		 * Don't leave this loop if we only tried once to post and an
		 * authentication request was received. Leave loop on any other
		 * response or any further authentication requests.
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
		error_message("Posting failed(%s)", line);
		return ret_code;
	}

	/*
	 * scan line if it contains a Message-ID
	 */
	{
		/* simple syntax check - locate last '<' */
		if ((ptr = strrchr(line, '<')) != NULL) {
			/* search next '>' */
			if ((ptr2 = strchr(ptr, '>')) != NULL) {
				/* terminate string */
				*++ptr2 = '\0';
				/* check for @ and no whitespaces */
				if ((strchr(ptr, '@') != NULL) && (strpbrk(ptr, " \t") == NULL))
					/* copy Message-ID */
					strcpy(a_message_id, ptr);
			}
		}

#if 0
		if (*message_id && *a_message_id) { /* check if returned ID matches purposed ID */
			if (strcmp(message_id, a_message_id)) {
				; /* shouldn't happen - warn user? */
			}
		}
#endif /* 0 */

		if (*message_id && (id_in_article || !*a_message_id))
			strcpy(a_message_id, message_id);
	}

	ret_code = TRUE;

	return ret_code;
}
#endif /* NNTP_INEWS */

/*
 * Call submit_inews() if using built in inews, else invoke external inews prog
 */
t_bool
submit_news_file(
	char *name,
	struct t_group *group,
	char *a_message_id)
{
	char buf[PATH_LEN];
	char *cp = buf;
	t_bool ret_code;
	t_bool ismail = FALSE;

	a_message_id[0] = '\0';

	checknadd_headers(name);

	/* 7bit ISO-2022-KR is NEVER to be used in Korean news posting. */
#ifdef CHARSET_CONVERSION
	if (!(strcasecmp(txt_mime_charsets[group ? group->attribute->mm_network_charset : tinrc.mm_network_charset], "EUC-KR") || strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_7bit)))
#else
	if (!(strcasecmp(tinrc.mm_charset, "EUC-KR") || strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_7bit)))
#endif /* CHARSET_CONVERSION */
		tinrc.post_mime_encoding = 0;	/* FIXME: txt_8bit */

	rfc15211522_encode(name, txt_mime_encodings[tinrc.post_mime_encoding], group, tinrc.post_8bit_header, ismail);

#ifdef NNTP_INEWS
	if (read_news_via_nntp && !read_saved_news && 0 == strcasecmp(tinrc.inews_prog, "--internal"))
		ret_code = submit_inews(name, group, a_message_id);
	else
#endif /* NNTP_INEWS */
		{
#ifdef M_UNIX
			/* use tinrc.inews_prog or 'inewsdir/inews -h' 'inews -h' */
			if (0 != strcasecmp(tinrc.inews_prog, "--internal"))
				strncpy(buf, tinrc.inews_prog, sizeof(buf) - 1);
			else {
				if (*inewsdir)
					joinpath(buf, inewsdir, "inews -h");
				else
					strcpy(buf, "inews -h");
			}
			cp += strlen(cp);
			sh_format(cp, sizeof(buf) - (cp - buf), " < %s", name);
#else
			make_post_cmd(cp, name);
#endif /* M_UNIX */

			ret_code = invoke_cmd(buf);

#ifdef NNTP_INEWS
			if (!ret_code && read_news_via_nntp && !read_saved_news && 0 != strcasecmp(tinrc.inews_prog, "--internal")) {
				if (prompt_yn(cLINES, _(txt_post_via_builtin_inews), TRUE)) {
					ret_code = submit_inews(name, group, a_message_id);
					if (ret_code) {
						if (prompt_yn(cLINES, _(txt_post_via_builtin_inews_only), TRUE) == 1)
							strcpy(tinrc.inews_prog,"--internal");
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
	char *from,
	struct t_group *group,
	char *sender)
{
	char *from_at_pos;
	char *sender_at_pos;
	char *sender_dot_pos;
	char from_addr[HEADER_LEN];
	char from_name[HEADER_LEN];
	char sender_addr[HEADER_LEN];
	char sender_line[HEADER_LEN];
	char sender_name[HEADER_LEN];

#	ifdef DEBUG
	if (debug == 2) {
		wait_message(3, "sender_needed From:=[%s]", from);
		wait_message(3, "sender_needed Sender:=[%s]", sender);
	}
#	endif /* DEBUG */

	/* split From: line into address & comment */

	gnksa_do_check_from(from, from_addr, from_name);

	snprintf(sender_line, sizeof(sender_line), "Sender: %s", sender);
	if (GNKSA_OK != gnksa_do_check_from(rfc1522_encode(sender_line, group, FALSE) + 8, sender_addr, sender_name))
		return -2;

	from_at_pos = strchr(from_addr, '@');
	sender_at_pos = strchr(sender_addr, '@');
	sender_dot_pos = strchr(sender_at_pos, '.');

	if (strncasecmp(from_addr, sender_addr, (from_at_pos - from_addr)))
		return 1; /* login differs */

	if (strcasecmp(from_at_pos, sender_at_pos) && (strcasecmp(from_at_pos + 1, sender_dot_pos + 1)))
		return 1; /* domainname differs */

	return 0;
}
#endif /* NNTP_INEWS && !FORGERY */
