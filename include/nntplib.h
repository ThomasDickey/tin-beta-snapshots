/*
 *  Project   : tin - a Usenet reader
 *  Module    : nntplib.h
 *  Author    : I.Lea
 *  Created   : 1991-04-01
 *  Updated   : 2005-07-21
 *  Notes     : nntp.h 1.5.11/1.6 with extensions for tin
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


#ifndef NNTPLIB_H
#define NNTPLIB_H 1

#ifndef NNTP_SERVER_FILE
#	define NNTP_SERVER_FILE	"/etc/nntpserver"
#endif /* !NNTP_SERVER_FILE */

#define NNTP_TCP_NAME	"nntp"
#define NNTP_TCP_PORT	"119"

#if 0 /* unused */
/*
 * # seconds after which a read from the NNTP will timeout
 * NB: This is different from the NNTP server timing us out due to inactivity
 */
#	define NNTP_READ_TIMEOUT		30
#endif /* 0 */

/*
 * # times to try and reconnect to server after timeout
 */
#define NNTP_TRY_RECONNECT		2

/*
 * @(#)Header: nntp.h,v 1.81 92/03/12 02:08:31 sob Exp $
 *
 * First digit:
 *
 *	1xx  Informative message
 *	2xx  Command ok
 *	3xx  Command ok so far, continue
 *	4xx  Command was correct, but couldn't be performed
 *	     for some specified reason.
 *	5xx  Command unimplemented, incorrect, or a
 *	     program error has occured.
 *
 * Second digit:
 *
 *	x0x  Connection, setup, miscellaneous
 *	x1x  Newsgroup selection
 *	x2x  Article selection
 *	x3x  Distribution
 *	x4x  Posting
 */
#if 0 /* unused */
#	define	CHAR_INF		'1'
#	define	CHAR_OK		'2'
#	define	CHAR_CONT		'3'
#	define	CHAR_ERR		'4'
#	define	CHAR_FATAL		'5'
#endif /* 0 */

#define	INF_HELP		100	/* Help text on way */
#define	INF_CAPABILITIES	101	/* Capability list follows */
#define	INF_AUTH		180	/* Authorization capabilities */
#define	INF_DEBUG		199	/* Debug output */

#define	OK_CANPOST		200	/* Hello; you can post */
#define	OK_NOPOST		201	/* Hello; you can't post */
#define	OK_EXTENSIONS	202	/* extensions supported follow */
#define	OK_NOIHAVE		202	/* Hello; you can't IHAVE; discussed on the IETF-mailinglist */
#define	OK_SLAVE		202	/* Slave status noted */
#define	OK_NOPOSTIHAVE	203	/* Hello; you can't POST or IHAVE; iscussed on the IETF-mailinglist */
#define	OK_GOODBYE		205	/* Closing connection */
#define	OK_GROUP		211	/* Group selected */
#define	OK_GROUPS		215	/* Newsgroups follow */
#define	OK_MOTD			215	/* News motd follows */

/* obsolete */
#define	OK_XINDEX		218	/* Tin style index follows */

#define	OK_ARTICLE		220	/* Article (head & body) follows */
#define	OK_HEAD			221	/* Head follows */
#define	OK_BODY			222	/* Body follows */
#define	OK_NOTEXT		223	/* No text sent -- stat, next, last */
#define	OK_XOVER		224	/* .overview data follows */
#define	OK_NEWNEWS		230	/* New articles by message-id follow */
#define	OK_NEWGROUPS		231	/* New newsgroups follow */
#define	OK_XFERED		235	/* Article transferred successfully */
#define	OK_POSTED		240	/* Article posted successfully */
#define	OK_AUTHSYS		280	/* Authorization system ok */
#define	OK_AUTH			281	/* Authorization (user/pass) ok */
#define	OK_BIN			282	/* binary data follows */
#define	OK_LIST			282	/* list follows */
#define	OK_SPLIST		283	/* spooldir list follows */
#define	OK_SPSWITCH		284	/* Switching to a different spooldir */
#define	OK_SPNOCHANGE		285	/* Still using same spooldir */
#define	OK_SPLDIRCUR		286	/* Current spooldir */
#define	OK_SPLDIRAVL		287	/* Available spooldir */
#define	OK_SPLDIRERR		288	/* Unavailable spooldir or invalid entry */

#define	CONT_XFER		335	/* Continue to send article */
#define	CONT_POST		340	/* Continue to post article */
#define	NEED_AUTHINFO		380	/* authorization is required */
#define	NEED_AUTHDATA		381	/* <type> authorization data required */

#define	ERR_GOODBYE		400	/* Have to hang up for some reason */
#define	ERR_NOGROUP		411	/* No such newsgroup */
#define	ERR_NCING		412	/* Not currently in newsgroup */

#define	ERR_XINDEX		418	/* No tin index for this group */
#define	ERR_XOVERVIEW		419	/* No .overview index for this group */

#define	ERR_NOCRNT		420	/* No current article selected */
#define	ERR_NONEXT		421	/* No next article in this group */
#define	ERR_NOPREV		422	/* No previous article in this group */
#define	ERR_NOARTIG		423	/* No such article in this group */
#define	ERR_NOART		430	/* No such article at all */
#define	ERR_GOTIT		435	/* Already got that article, don't send */
#define	ERR_XFERFAIL		436	/* Transfer failed */
#define	ERR_XFERRJCT		437	/* Article rejected, don't resend */
#define	ERR_NOPOST		440	/* Posting not allowed */
#define	ERR_POSTFAIL		441	/* Posting failed */
#define	ERR_NOAUTH		480	/* authorization required for command */
#define	ERR_AUTHSYS		481	/* Authorization system invalid */
#define	ERR_AUTHREJ		482	/* Authorization data rejected */
#define	ERR_INVALIAS		483	/* Invalid alias on spooldir cmd */
#define	ERR_INVNOSPDIR		484	/* No spooldir file found */

#define	ERR_COMMAND		500	/* Command not recognized */
#define	ERR_CMDSYN		501	/* Command syntax error */
#define	ERR_ACCESS		502	/* Access to server denied */
#define	ERR_FAULT		503	/* Program fault, command not performed */
#define	ERR_MOTD		503	/* No news motd file */
#define	ERR_AUTHBAD		580	/* Authorization Failed */

/*
 * RFC 977 defines this; don't change it.
 */
#define NNTP_STRLEN		512

/*
 * CAPABILITIES
 */
struct t_capabilities {
	unsigned int type:3;			/* none, LIST EXTENSIONS, CAPABILITIES */
	unsigned int version;			/* CAPABILITIES version */
	t_bool mode_reader:1;			/* MODE-READER: "MODE READER" */
	t_bool reader:1;				/* READER: "ARTCILE", "BODY", "LISTGROUP" */
	t_bool post:1;					/* POST */
	t_bool list_active:1;			/* LIST ACTIVE */
	t_bool list_active_times:1;		/* LIST ACTIVE.TIMES, optional */
	t_bool list_distrib_pats:1;		/* LIST DISTRIB.PATS, optional */
	t_bool list_headers:1;			/* LIST HEADERS */
	t_bool list_newsgroups:1;		/* LIST NEWSGROUPS */
	t_bool list_overview_fmt:1;		/* LIST OVERVIEW.FMT */
	t_bool list_motd:1;				/* LIST MOTD, "private" extension */
	t_bool list_subscriptions:1;	/* LIST SUBSCRIPTIONS, "private" extension, RFC 2980 */
	t_bool list_distributions:1;	/* LIST DISTRIBUTIONS, "private" extension, RFC 2980 */
	t_bool list_moderators:1;		/* LIST MODERATORS, "private" extension */
	t_bool xpat:1;					/* XPAT, "private" extension, RFC 2980 */
	t_bool hdr:1;					/* HDR: "HDR", "LIST HEADERS" */
	const char *hdr_cmd;			/* [X]HDR */
	t_bool over:1;					/* OVER: "OVER", "LIST OVERVIEW.FMT" */
	t_bool over_msgid:1;			/* OVER: "OVER mid" */
	const char *over_cmd;			/* [X]OVER */
	t_bool newnews:1;				/* NEWNEWS */
	char *implementation;			/* IMPLEMENTATION */
	t_bool starttls:1;				/* STARTTLS */
	t_bool authinfo_user:1;			/* AUTHINFO USER/PASS */
	t_bool authinfo_sasl:1;			/* AUTHINFO SASL */
#if 0
	t_bool sasl_cram_md5:1;			/* SASL CRAM-MD5 */
	t_bool sasl_digest_md5:1;		/* SASL DIGEST-MD5 */
	t_bool sasl_plain:1;			/* SASL PLAIN */
	t_bool sasl_gssapi:1;			/* SASL GSSAPI */
	t_bool sasl_external:1;			/* SASL EXTERNAL */
	t_bool streaming:1;				/* STREAMING: "MODE STREAM", "CHECK", "TAKETHIS" */
	t_bool ihave:1;					/* IHAVE: "IHAVE" */
#endif /* 0 */
};

#endif /* !NNTPLIB_H */
