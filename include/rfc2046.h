/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2046.h
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 2000-02-18
 *  Updated   : 2024-07-24
 *  Notes     : RFC 2046 MIME article definitions
 *
 * Copyright (c) 2000-2024 Jason Faultless <jason@altarstone.com>
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

#ifndef RFC2046_H
#	define RFC2046_H 1

/* The version of MIME we conform to */
#	define MIME_SUPPORTED_VERSION	"1.0"

/* These must track the array definitions in lang.c; t_part->type */
#	define TYPE_TEXT			0	/* RFC 2046 */
#	define TYPE_MULTIPART		1	/* RFC 2046 */
#	define TYPE_APPLICATION		2	/* RFC 2046 */
#	define TYPE_MESSAGE			3	/* RFC 2046 */
#	define TYPE_IMAGE			4	/* RFC 2046 */
#	define TYPE_AUDIO			5	/* RFC 2046 */
#	define TYPE_VIDEO			6	/* RFC 2046 */
#	define TYPE_FONT			7	/* RFC 8081 */
#	define TYPE_HAPTICS			8	/* draft-ietf-mediaman-haptics-05 */
#	define TYPE_MODEL			9	/* RFC 2077 */
	/* TYPE_EXAMPLE RFC 4735 */

#	define ENCODING_7BIT		0
#	define ENCODING_QP			1
#	define ENCODING_BASE64		2
#	define ENCODING_8BIT		3
#	define ENCODING_BINARY		4
#	define ENCODING_UUE			5
#	define ENCODING_UNKNOWN		6

/* Content-Disposition types (RFC 2183) */
#	define DISP_INLINE			0
#	define DISP_ATTACHMENT		1
#	define DISP_NONE			2

#	define BOUND_NONE		0
#	define BOUND_START		1
#	define BOUND_END		2

#	define FORMAT_FIXED		0
#	define FORMAT_FLOWED	1

#	define MIME_OK							(1 << 0)
#	define MIME_VERSION_MISSING				(1 << 1)
#	define MIME_VERSION_UNSUPPORTED			(1 << 2)
#	define MIME_TYPE_MISSING				(1 << 3)
#	define MIME_TYPE_UNKNOWN				(1 << 4)
#	define MIME_SUBTYPE_MISSING				(1 << 5)
#	define MIME_SUBTYPE_UNKNOWN				(1 << 6)
#	define MIME_CHARSET_MISSING				(1 << 7)
#	define MIME_CHARSET_UNDECLARED			(1 << 8)
#	define MIME_CHARSET_GUESSED				(1 << 9)
#	define MIME_CHARSET_UNSUPPORTED			(1 << 10)
#	define MIME_TRANSFER_ENCODING_MISSING	(1 << 11)
#	define MIME_TRANSFER_ENCODING_UNKNOWN	(1 << 12)
#	define MIME_INIT						(MIME_VERSION_MISSING \
											| MIME_VERSION_UNSUPPORTED \
											| MIME_TYPE_MISSING \
											| MIME_SUBTYPE_MISSING \
											| MIME_CHARSET_MISSING \
											| MIME_TRANSFER_ENCODING_MISSING)

typedef struct hints
{
	unsigned flags:13;	/* Result of mime parsing
						define MIME_OK							no hints, to check if (mime_hints) {}
						define MIME_VERSION_MISSING				mime version missing
						define MIME_VERSION_UNSUPPORTED			mime version != 1.0
						define MIME_TYPE_MISSING				content type missing
						define MIME_TYPE_UNKNOWN				content type unknown
						define MIME_CHARSET_MISSING				charset missing
						define MIME_CHARSET_UNDECLARED			charset missing, undeclared_charset used
						define MIME_CHARSET_GUESSED				charset missing, guessed charset used
						define MIME_CHARSET_UNSUPPORTED			iconv() can't cope with charset
						define MIME_TRANSFER_ENCODING_MISSING	transfer encoding missing
						define MIME_TRANSFER_ENCODING_UNKNOWN	transfer encoding unknown */
	char *type;
	char *subtype;
	char *charset;
	char *encoding;
} t_hints;


/*
 * Linked list of parameter/value pairs
 * Used for params attached to a content line
 */
typedef struct param
{
	char *name;
	char *value;
	char *charset;
	int part;
	t_bool encoded;
	t_bool enc_fallback;
	struct param *next;
} t_param;


/*
 * Describes the properties of an article or article attachment
 * We reuse this to describe uuencoded sections
 */
typedef struct part
{
	unsigned type:4;		/* Content major type */
	unsigned encoding:3;	/* Transfer encoding */
	unsigned format:1;		/* Format=Fixed/Flowed */
	unsigned disposition:2; /* Content-Disposition=inline/attachment */
	char *subtype;			/* Content subtype */
	char *description;		/* Content-Description */
	char *language;			/* Content-Language RFC 3282 */
	t_param *params;		/* List of Content-Type parameters */
	long offset;			/* offset in article of the text of attachment */
	unsigned long bytes;	/* part size in bytes */
	int line_count;			/* # lines in this part */
	int depth;				/* For multipart within multipart */
	t_hints mime_hints;
	struct part *uue;		/* UUencoded section information */
	struct part *next;		/* next part */
} t_part;


/*
 * Used in save.c to build a list of attachments to be displayed
 *
 * TODO: move somewhere else?
 */
typedef struct partlist {
	t_part *part;
	struct partlist *next;
	int tagged;
} t_partl;


/*
 * RFC 822 compliant header with RFC 2045 MIME extensions
 */
struct t_header
{
	char *from;				/* From: */
	char *to;				/* To: */
	char *cc;				/* Cc: */
	char *bcc;				/* Bcc: */
	char *date;				/* Date: */
	char *subj;				/* Subject: */
	char *org;				/* Organization: */
	char *replyto;			/* Reply-To: */
	char *newsgroups;		/* Newsgroups: */
	char *messageid;		/* Message-ID: */
	char *references;		/* References: */
	char *distrib;			/* Distribution: */
	char *keywords;			/* Keywords: */
	char *summary;			/* Summary: */
	char *followup;			/* Followup-To: */
	char *ftnto;			/* Old X-Comment-To: (Used by FIDO) */
	char *xface;			/* X-Face: */
	t_bool mime:1;			/* Is Mime-Version: defined - TODO: change to version number */
	t_part *ext;			/* Extended Mime header information */
};


/* flags for lineinfo.flags */
/* Primary colours */
#	define C_HEADER		0x0001
#	define C_BODY		0x0002
#	define C_SIG		0x0004
#	define C_ATTACH		0x0008
#	define C_UUE		0x0010

/* Secondary flags */
#	define C_QUOTE1	0x0020
#	define C_QUOTE2	0x0040
#	define C_QUOTE3	0x0080

#	define C_URL			0x0100	/* Contains http|ftp|gopher: */
#	define C_MAIL			0x0200	/* Contains mailto: */
#	define C_NEWS			0x0400	/* Contains news|nntp: */
#	define C_CTRLL			0x0800	/* Contains ^L */
#	define C_VERBATIM		0x1000	/* Verbatim block */
#	ifdef HAVE_COLOR
#		define C_EXTQUOTE	0x2000	/* Quoted text from external sources */
#	endif /* HAVE_COLOR */


typedef struct lineinfo
{
	long offset;			/* Offset of this line */
	int flags;				/* Info about this line */
} t_lineinfo;


/*
 * Oddball collection of information about the open article
 */
typedef struct openartinfo
{
	struct t_header hdr;	/* Structural overview of the article */
	t_bool tex2iso;			/* TRUE if TeX encoding present */
	int cooked_lines;		/* # lines in cooked t_lineinfo */
	FILE *raw;				/* the actual data streams */
	FILE *cooked;
	FILE *log;
	t_lineinfo *rawl;		/* info about the data streams */
	t_lineinfo *cookl;
} t_openartinfo;

#endif /* !RFC2046_H */
