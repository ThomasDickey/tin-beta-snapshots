/*
 *  Project   : tin - a Usenet reader
 *  Module    : inews.c
 *  Author    : I. Lea
 *  Created   : 1992-03-17
 *  Updated   : 1997-12-31
 *  Notes     : NNTP builtin version of inews
 *
 * Copyright (c) 1991-2001 Iain Lea <iain@bricbrac.de>
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
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */


/*
 * local prototypes
 */
#ifdef NNTP_INEWS
	static t_bool submit_inews (char *name, char *a_message_id);
#endif /* NNTP_INEWS */

#if defined(NNTP_INEWS) && !defined(FORGERY)
	static int sender_needed (char * from, char * sender);
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

#define PATHMASTER	"not-for-mail"

/*
 * Submit an article using the NNTP POST command
 */
#ifdef NNTP_INEWS
static t_bool
submit_inews (
	char *name,
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

	if ((fp = fopen (name, "r")) == (FILE *) 0) {
		perror_message (_(txt_cannot_open), name);
		return ret_code;
	}

	from_name[0] = '\0';
	message_id[0] = '\0';

	while (fgets (line, (int) sizeof (line), fp) != NULL) {
		if (line[0] != '\n') {
			ptr = strchr (line, ':');
			if (ptr - line == 4 && !strncasecmp (line, "From", 4)) {
				strcpy(from_name, line);
				if ((ptr = strchr(from_name, '\n')))
					*ptr = '\0';
			}
			if (ptr - line == 10 && !strncasecmp (line, "Message-ID", 10)) {
				strcpy(message_id, ptr+2);
				id_in_article = TRUE;
				if ((ptr = strchr(message_id, '\n')))
					*ptr = '\0';
			}
		} else
			break; /* end of headers */
	}

	if ((from_name[0] == '\0') || (from_name[6] == '\0')) {
		/* we could silently add a From: line here if we want to... */
		error_message (_(txt_error_no_from));
		fclose (fp);
		return ret_code;
	}

#ifndef FORGERY
	/*
	 * we should only skip the gnksa_check_from() test if we are going to post
	 * a forged cancel, but inews.c doesn't know anything about the message
	 * type, so we skip the test if FORGERY is set
	 *
	 * check for valid From: line
	 * this will be done once again in sender_needed!?!
	 */
	if (GNKSA_OK != gnksa_check_from(rfc1522_encode(from_name, FALSE) + 6)) { /* error in address */
		error_message (_(txt_invalid_from), from_name + 6);
		fclose (fp);
		return ret_code;
	}
#endif /* !FORGERY */

	do {
		rewind(fp);


#	ifndef FORGERY
		if ((ptr = build_sender()) && (!disable_sender)) {
			sender = sender_needed (from_name + 6, ptr);
			switch (sender) {
				case -2: /* can't build Sender: */
					error_message (_(txt_invalid_sender), ptr);
					fclose (fp);
					return ret_code;
					/* NOTREACHED */
					break;

				case -1: /* illegal From: (can't happen as check is done above already) */
					error_message (_(txt_invalid_from), from_name + 6);
					fclose (fp);
					return ret_code;
					/* NOTREACHED */
					break;

				case 1:	/* insert Sender */
					snprintf (sender_hdr, sizeof(sender_hdr), "Sender: %s", ptr);
					STRCPY (sender_hdr, rfc1522_encode (sender_hdr, ismail));
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
		if (nntp_command("POST", CONT_POST, line) == NULL) {
			error_message ("%s", line);
			fclose (fp);
			return ret_code;
		}

		/*
		 * check article if it contains a Message-ID header
		 * if not scan line if it contains a Message-ID
		 * if it's present: use it.
		 */
		if (message_id[0] == '\0') {

			/* simple syntax check - locate last '<' */
			if ((ptr = strrchr (line, '<')) != (char *) 0) {
				/* search next '>' */
				if ((ptr2 = strchr (ptr, '>')) != (char *) 0) {
					/* terminate string */
					*++ptr2 = '\0';
					/* check for @ and no whitespaces */
					if ((strchr(ptr, '@') != (char *) 0) && (strpbrk(ptr, " \t") == (char *) 0))
						/* copy Message-ID */
						strcpy(message_id, ptr);
				}
			}
		}

#	ifndef FORGERY
		/*
		 * Send Path: (and Sender: if needed) headers
		 */
		sprintf (line, "Path: %s", PATHMASTER);
		put_server (line);

		if (sender == 1)
			put_server (sender_hdr);
#	endif /* !FORGERY */

		/*
		 * check if Message-ID comes from the server
		 */
		if (*message_id) {
			if (!id_in_article) {
				sprintf (line, "Message-ID: %s", message_id);
				put_server (line);
			}
#	ifdef USE_CANLOCK
			/* create a Cancel-Lock: */
			{
				char lock[1024];
				const char *lptr = (const char *) 0;

				lock[0] = '\0';
				if ((lptr = build_canlock(message_id, get_secret())) != (const char *) 0) {
					STRCPY(lock, lptr);
					sprintf (line, "Cancel-Lock: %s", lock);
					put_server (line);
				}
			}
#	endif /* USE_CANLOCK */
		}

		/*
		 * Send article 1 line at a time ending with "."
		 */
		while (fgets (line, (int) sizeof (line), fp) != (char *) 0) {
			/*
			 * Remove linefeed from line
			 */
			if ((ptr = strrchr (line, '\n')) != (char *) 0)
				*ptr = '\0';

			/*
			 * If line starts with a '.' add another '.' to stop truncation
			 */
			if (line[0] == '.')
				u_put_server (".");

			u_put_server (line);
			u_put_server ("\r\n");
		}

		put_server (".");

		/*
		 * Receive OK_POSTED or ERROR response code from NNTP server
		 * Don't use get_respcode at this point, because then we would not
		 * recognize if posting has failed due to missing authentication in
		 * which case the complete posting has to be resent. Besides, because
		 * of the put_server(".") above a "." would be resent as the last
		 * "command".
		 */
		respcode = get_only_respcode (line);
		leave_loop = TRUE;

		/*
		 * Don't leave this loop if we only tried once to post and an
		 * authentication request was received. Leave loop on any other
		 * response or any further authentication requests.
		 */
		if (((respcode == ERR_NOAUTH) || (respcode == NEED_AUTHINFO)) && (auth_error++ < 1) && (authenticate (nntp_server, userid, FALSE)))
			leave_loop = FALSE;
	} while (!leave_loop);

	fclose (fp);

	/*
	 * FIXME: The displayed message may be wrong if authentication has
	 * failed.  (The message will be sth. like "Authentication required"
	 * which is not really wrong but misleading. The problem is that
	 * authenticate() does only return a bool value and not the server
	 * response.)
	 */
	if (respcode != OK_POSTED) {
		error_message ("Posting failed (%s)", line);
		return ret_code;
	}

	/*
	 * scan line if it contains a Message-ID
	 */
	{
		/* simple syntax check - locate last '<' */
		if ((ptr = strrchr (line, '<')) != (char *) 0) {
			/* search next '>' */
			if ((ptr2 = strchr (ptr, '>')) != (char *) 0) {
				/* terminate string */
				*++ptr2 = '\0';
				/* check for @ and no whitespaces */
				if ((strchr(ptr, '@') != (char *) 0) && (strpbrk(ptr, " \t") == (char *) 0))
					/* copy Message-ID */
					strcpy(a_message_id, ptr);
			}
		}

#if 0
		if (*message_id && *a_message_id) { /* check if returned ID matches purposed ID */
			if (strcmp(message_id, a_message_id))
				; /* shouldn't happen - warn user? */
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
 * Call submit_inews() if using builtin inews, else invoke external inews prog
 */
t_bool
submit_news_file (
	char *name,
	char *a_message_id)
{
	char buf[LEN];
	char *cp = buf;
	t_bool ret_code;
	t_bool ismail = FALSE;

	a_message_id[0] = '\0';

	checknadd_headers (name);

	/* 7bit ISO-2022-KR is NEVER to be used in Korean news posting. */
	if (!(strcasecmp(tinrc.mm_charset, "euc-kr") || strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_7bit)))
		tinrc.post_mime_encoding = 0;	/* FIXME: txt_8bit */

	rfc15211522_encode(name, txt_mime_encodings[tinrc.post_mime_encoding], tinrc.post_8bit_header, ismail);

#ifdef NNTP_INEWS
	if (read_news_via_nntp && !read_saved_news && tinrc.use_builtin_inews) {
		ret_code = submit_inews (name, a_message_id);
	} else
#endif /* NNTP_INEWS */
		{
#ifdef M_UNIX
		if (*inewsdir) {
			strcpy (buf, inewsdir);
			strcat (buf, "/");
			cp += strlen (cp);
		}
		sh_format (cp, sizeof(buf) - (cp - buf), "inews -h < %s", name);
#else
		make_post_cmd (cp, name);
#endif /* M_UNIX */

		ret_code = invoke_cmd (buf);
#ifdef NNTP_INEWS
		if (!ret_code && read_news_via_nntp && !read_saved_news && !tinrc.use_builtin_inews) {
			if (prompt_yn(cLINES, _(txt_post_via_builtin_inews), TRUE)) {
				ret_code = submit_inews (name, a_message_id);
				if (ret_code) {
					tinrc.use_builtin_inews = prompt_yn(cLINES, _(txt_post_via_builtin_inews_only), TRUE);
				}
			}
		}
#endif /* NNTP_INEWS */
	}

	return ret_code;
}


/*
 * FIXME: do _real_ RFC822-parsing - currently this is a quick hack
 *        to cover the most usual cases...
 *
 * returnvalues:  1 = Sender needed
 *                0 = no Sender needed
 *               -1 = error (no '.' and/or '@' in From)
 *               -2 = error (no '.' and/or '@' in Sender)
 */
#if defined(NNTP_INEWS) && !defined(FORGERY)
static int sender_needed (
	char *from,
	char *sender)
{
	char *from_at_pos;
	char *sender_at_pos;
	char *sender_dot_pos;
	char from_addr[HEADER_LEN];
	char from_line[HEADER_LEN];
	char from_name[HEADER_LEN];
	char sender_addr[HEADER_LEN];
	char sender_line[HEADER_LEN];
	char sender_name[HEADER_LEN];

#ifdef DEBUG
	if (debug == 2)
		wait_message (0, "sender_needed From:=[%s]", from);
#endif /* DEBUG */

	snprintf (from_line, sizeof(from_line), "From: %s", from);
	if (GNKSA_OK != gnksa_do_check_from(rfc1522_encode(from_line, FALSE) + 6, from_addr, from_name))
		return -1;

#ifdef DEBUG
	if (debug == 2)
		wait_message (0, "sender_needed Sender:=[%s]", sender);
#endif /* DEBUG */

	snprintf (sender_line, sizeof(sender_line), "Sender: %s", sender);
	if (GNKSA_OK != gnksa_do_check_from(rfc1522_encode(sender_line, FALSE) + 8, sender_addr, sender_name))
		return -2;

	from_at_pos = strchr(from_addr, '@');
	sender_at_pos = strchr(sender_addr, '@');
	sender_dot_pos = strchr (sender_at_pos, '.');

	if (strncasecmp(from_addr, sender_addr, (from_at_pos - from_addr)))
		return 1; /* login differs */

	if (strcasecmp(from_at_pos, sender_at_pos) && (strcasecmp (from_at_pos + 1, sender_dot_pos + 1)))
		return 1; /* domainname differs */

	return 0;
}
#endif /* NNTP_INEWS && !FORGERY */
