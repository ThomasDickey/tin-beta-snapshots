/*
 *  Project   : tin - a Usenet reader
 *  Module    : rfc2046.h
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 2000-02-18
 *  Updated   : 2003-05-06
 *  Notes     : rfc2046 MIME article definitions
 *
 * Copyright (c) 2000-2006 Jason Faultless <jason@altarstone.com>
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

#ifndef RFC2046_H
#	define RFC2046_H 1

/* The version of MIME we conform to */
#	define MIME_SUPPORTED_VERSION	"1.0"

/* These must track the array definitions in lang.c */
#	define NUM_CONTENT_TYPES	7
#	define NUM_ENCODINGS		6

#	define TYPE_TEXT			0
#	define TYPE_MULTIPART		1
#	define TYPE_APPLICATION		2
#	define TYPE_MESSAGE			3

#	define ENCODING_7BIT		0
#	define ENCODING_QP			1
#	define ENCODING_BASE64		2
#	define ENCODING_8BIT		3
#	define ENCODING_BINARY		4
#	define ENCODING_UUE			5

#	define DISP_INLINE			0
#	define DISP_ATTACH			1

#	define BOUND_NONE		0
#	define BOUND_START		1
#	define BOUND_END		2


/*
 * Linked list of parameter/value pairs
 * Used for params attached to a content line
 */
typedef struct param
{
	char *name;
	char *value;
	struct param *next;
} t_param;


/*
 * Describes the properties of an article or article attachment
 * We re-use this to describe uuencoded sections
 */
typedef struct part
{
	unsigned type:3;		/* Content major type */
	unsigned encoding:3;	/* Transfer encoding */
#	if 0
	unsigned disposition:1;
#	endif /* 0 */
	char *subtype;			/* Content subtype */
	char *description;		/* Content-Description */
	t_param *params;		/* List of Content-Type parameters */
	long offset;			/* offset in article of the text of attachment */
	int line_count;			/* # lines in this part */
	int depth;				/* For multipart within multipart */
	struct part *uue;		/* UUencoded section information */
	struct part *next;		/* next part */
} t_part;


/*
 * RFC822 compliant header with RFC2045 MIME extensions
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
#	define C_HEADER		0x001
#	define C_BODY		0x002
#	define C_SIG		0x004
#	define C_ATTACH		0x008
#	define C_UUE		0x010

/* Secondary flags */
#	define C_QUOTE1	0x020
#	define C_QUOTE2	0x040
#	define C_QUOTE3	0x080

#	define C_URL		0x100	/* Contains http|ftp|gopher: */
#	define C_MAIL		0x200	/* Contains mailto: */
#	define C_NEWS		0x400	/* Contains news|nntp: */
#	define C_CTRLL		0x800	/* Contains ^L */


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
	t_lineinfo *rawl;		/* info about the data streams */
	t_lineinfo *cookl;
} t_openartinfo;

#endif /* !RFC2046_H */
