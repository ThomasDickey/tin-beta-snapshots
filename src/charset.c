/*
 *  Project   : tin - a Usenet reader
 *  Module    : charset.c
 *  Author    : M. Kuhn, T. Burmester
 *  Created   : 1993-12-10
 *  Updated   : 1994-02-28
 *  Notes     : ISO to ascii charset conversion routines
 *
 * Copyright (c) 1993-2002 Markus Kuhn <mgk25@cl.cam.ac.uk>
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

/*
 *  Table for the iso2asc convertion...
 *  iso2asc  by  (unrza3@cd4680fs.rrze.uni-erlangen.de)
 *  included by  (root@aspic.han.de)
 */

#define SUB	"?"
#define ISO_EXTRA	0xa0 /* beginning of second range of printable chars */

/*
 * TABSTOP(x) is the column of the character after the TAB
 * at column x. First column is 0, of course.
 */

#define TABSTOP(x)	(((x) - ((x)&7)) + 8)

static constext *const iso2asc[NUM_ISO_TABLES][256-ISO_EXTRA] =
{
	{
	" ","!","c",SUB,SUB,"Y","|",SUB,"\"","(c)","a","<<","-","-","(R)","-",
	" ","+/-","2","3","'","u","P",".",",","1","o",">>"," 1/4"," 1/2"," 3/4","?",
	"A","A","A","A","A","A","AE","C","E","E","E","E","I","I","I","I",
	"D","N","O","O","O","O","O","x","O","U","U","U","U","Y","Th","ss",
	"a","a","a","a","a","a","ae","c","e","e","e","e","i","i","i","i",
	"d","n","o","o","o","o","o",":","o","u","u","u","u","y","th","y"
	},
	{
	" ","!","c",SUB,SUB,"Y","|",SUB,"\"","c","a","<","-","-","R","-",
	" ",SUB,"2","3","'","u","P",".",",","1","o",">",SUB,SUB,SUB,"?",
	"A","A","A","A","A","A","A","C","E","E","E","E","I","I","I","I",
	"D","N","O","O","O","O","O","x","O","U","U","U","U","Y","T","s",
	"a","a","a","a","a","a","a","c","e","e","e","e","i","i","i","i",
	"d","n","o","o","o","o","o",":","o","u","u","u","u","y","t","y"
	},
	{
	" ","!","c",SUB,SUB,"Y","|",SUB,"\"","(c)","a","<<","-","-","(R)","-",
	" ","+/-","2","3","'","u","P",".",",","1","o",">>"," 1/4"," 1/2"," 3/4","?",
	"A","A","A","A","Ae","Aa","AE","C","E","E","E","E","I","I","I","I",
	"D","N","O","O","O","O","Oe","x","Oe","U","U","U","Ue","Y","Th","ss",
	"a","a","a","a","ae","aa","ae","c","e","e","e","e","i","i","i","i",
	"d","n","o","o","o","o","oe",":","oe","u","u","u","ue","y","th","ij"
	},
	{
	" ","!","c",SUB,"$","Y","|",SUB,"\"","(c)","a","<<","-","-","(R)","-",
	" ","+/-","2","3","'","u","P",".",",","1","o",">>"," 1/4"," 1/2"," 3/4","?",
	"A","A","A","A","[","]","[","C","E","@","E","E","I","I","I","I",
	"D","N","O","O","O","O","\\","x","\\","U","U","U","^","Y","Th","ss",
	"a","a","a","a","{","}","{","c","e","`","e","e","i","i","i","i",
	"d","n","o","o","o","o","|",":","|","u","u","u","~","y","th","y"
	},
	{
	"[NS]","[!I]","[Ct]","[Pd]","[Cu]","[Ye]","[BB]","[SE]",
	"[':]","[Co]","[-a]","[<<]","[NO]","[--]","[Rg]","['-]",
	"[DG]","[+-]","[2S]","[3S]","['']","[My]","[PI]","[.M]",
	"[',]","[1S]","[-o]","[>>]","[14]","[12]","[34]","[?I]",
	"[A!]","[A']","[A>]","[A?]","[A:]","[AA]","[AE]","[C,]",
	"[E!]","[E']","[E>]","[E:]","[I!]","[I']","[I>]","[I:]",
	"[D-]","[N?]","[O!]","[O']","[O>]","[O?]","[O:]","[*X]",
	"[O/]","[U!]","[U']","[U>]","[U:]","[Y']","[TH]","[ss]",
	"[a!]","[a']","[a>]","[a?]","[a:]","[aa]","[ae]","[c,]",
	"[e!]","[e']","[e>]","[e:]","[i!]","[i']","[i>]","[i:]",
	"[d-]","[n?]","[o!]","[o']","[o>]","[o?]","[o:]","[-:]",
	"[o/]","[u!]","[u']","[u>]","[u:]","[y']","[th]","[y:]"
	},
	{
	" ","!","c\b|","L\b-","o\bX","Y\b=","|",SUB,
	"\"","(c)","a\b_","<<","-\b,","-","(R)","-",
	" ","+\b_","2","3","'","u","P",".",
	",","1","o\b_",">>"," 1/4"," 1/2"," 3/4","?",
	"A\b`","A\b'","A\b^","A\b~","A\b\"","Aa","AE","C\b,",
	"E\b`","E\b'","E\b^","E\b\"","I\b`","I\b'","I\b^","I\b\"",
	"D\b-","N\b~","O\b`","O\b'","O\b^","O\b~","O\b\"","x",
	"O\b/","U\b`","U\b'","U\b^","U\b\"","Y\b'","Th","ss",
	"a\b`","a\b'","a\b^","a\b~","a\b\"","aa","ae","c\b,",
	"e\b`","e\b'","e\b^","e\b\"","i\b`","i\b'","i\b^","i\b\"",
	"d\b-","n\b~","o\b`","o\b'","o\b^","o\b~","o\b\"","-\b:",
	"o\b/","u\b`","u\b'","u\b^","u\b\"","y\b'","th","y\b\""
	},
	{
	"\377","\255","\233","\234",SUB,"\235","|","\25",
	"\"","(c)","\246","\256","\252","-","(R)","-",
	"\370","\361","\375","3","'","\346","\24","\371",
	",","1","\247","\257","\254","\253"," 3/4","\250",
	"A","A","A","A","\216","\217","\222","\200",
	"E","\220","E","E","I","I","I","I",
	"D","\245","O","O","O","O","\231","x",
	"\355","U","U","U","\232","Y","T","\341",
	"\205","\240","\203","a","\204","\206","\221","\207",
	"\212","\202","\210","\211","\215","\241","\214","\213",
	"d","\244","\225","\242","\223","o","\224","\366",
	"\355","\227","\243","\226","\201","y","t","\230"
	}
};

/*
 * german tex style to latin1 conversion (by root@aspic, 12/04/93)
 */

#define TEX_SUBST	15
#define SPACES		"                                                                                                         "

static const char *const tex_from[TEX_SUBST] =
{
	"\"a","\\\"a","\"o","\\\"o","\"u","\\\"u","\"A","\\\"A","\"O","\\\"O","\"U","\\\"U","\"s","\\\"s","\\3"
};
static const char *const tex_to[TEX_SUBST] =
{
	"�", "�", "�", "�", "�", "�", "�", "�", "�", "�", "�", "�", "�", "�", "�"
};

/*
 *  Now the conversion function...
 */

void
convert_iso2asc (
	char *iso,
	char **asc_buffer,
	int *max_line_len,
	int t)
{
	constext *p;
	constext *const *tab;
	char *asc;
	int first;	/* flag for first SPACE/TAB after other characters */
	int i, a;	/* column counters in iso and asc */

	asc = *asc_buffer;

	if (iso == 0 || asc == 0)
		return;

	tab = (iso2asc[t] - ISO_EXTRA);
	first = 1;
	i = a = 0;
	while (*iso != '\0') {
		if (*EIGHT_BIT(iso) >= ISO_EXTRA) {
			p = tab[*EIGHT_BIT(iso)];
			iso++, i++;
			first = 1;
			while (*p) {
				*(asc++) = *(p++);
				if ((asc - *asc_buffer) >= *max_line_len) {
					int offset = asc - *asc_buffer;
					*max_line_len += 64;
					*asc_buffer = my_realloc(*asc_buffer, *max_line_len);
					asc = *asc_buffer + offset;
				}
				a++;
			}
		} else {
			if (a > i && ((*iso == ' ') || (*iso == '\t'))) {
				/*
				 * spaces or TABS should be removed
				 */
				if (*iso == ' ') {
					/*
					 * only the first space after a letter must not be removed
					 */
					if (first) {
						*(asc++) = ' ';
						a++;
						first = 0;
					}
					i++;
				} else {	/* here: *iso == '\t' */
					if (a >= TABSTOP(i)) {
						/*
						 * remove TAB or replace it with SPACE if necessary
						 */
						if (first) {
							*(asc++) = ' ';
							a++;
							first = 0;
						}
					} else {
						/*
						 * TAB will correct the column difference
						 */
						*(asc++) = '\t';	/* = *iso */
						a = TABSTOP(a);	/* = TABSTOP(i), because i < a < TABSTOP(i) */
					}
					i = TABSTOP(i);
				}
				iso++;
			} else {
				/*
				 * just copy the characters and advance the column counters
				 */
				if (*iso == '\t') {
					a = i = TABSTOP(i);	/* = TABSTOP(a), because here a = i */
				} else if (*iso == '\b') {
					a--;
					i--;
				} else {
					a++;
					i++;
				}
				*(asc++) = *(iso++);
				first = 1;
			}
		}
		if ((asc - *asc_buffer) >= *max_line_len) {
			int offset = asc - *asc_buffer;
			*max_line_len += 64;
			*asc_buffer = my_realloc(*asc_buffer, *max_line_len);
			asc = *asc_buffer + offset;
		}
	}
	*asc = '\0';

	return;
}


void
convert_tex2iso (
	char *from,
	char *to)
{
	int i, ex;
	size_t spaces = 0; /* spaces to add */
	size_t len, col = 0;	/* length of from, col counter */
	size_t subst_len;

	*to = '\0';
	len = strlen (from);

	while (col < len) {
		i = ex = 0;
		while ((i < TEX_SUBST) && !ex) {
			subst_len = strlen (tex_from[i]);
			if (!strncmp (from + col, tex_from[i], subst_len)) {
				strcat (to, tex_to[i]);
				spaces += subst_len - 1;
				col += subst_len - 1;
				ex = 1;
			}
			i++;
		}
		if (!ex)
			strncat (to, from + col, 1);
		if (from[col] == ' ') {
			strncat (to, SPACES, spaces);
			spaces = 0;
		}

		col++;
	}
}

/*
 * Check for german TeX encoding in file open on fp
 */
t_bool
is_art_tex_encoded (
	FILE *fp)
{
	char line[LEN];
	int i, len;
	t_bool body = FALSE;

	rewind (fp);

	while (fgets (line, (int) sizeof(line), fp) != NULL) {
		if (line[0] == '\n' && !body)
			body = TRUE;
		else if (!body)
			continue;

		i = 0;

		while (line[i++] == ' ')
			;	/* search for first non blank */

		i--;

		if (!isalnum((unsigned char) line[i]))
			continue;	/* quoting char */

		len = strlen(line) - 1;
		for (i = 1; i < len; i++) {
			if (((line[i] == '\\') || (line[i] == '\"')) &&
							(isalnum((unsigned char) line[i - 1])) &&
							(isalnum((unsigned char) line[i + 1])))
				return TRUE;
		}
	}

	return FALSE;
}


/*
 * Replace all non printable characters by '?'
 */
void
convert_to_printable (
	char *buf)
{
	unsigned char *c;
#ifdef ENABLE_MBLEN
	int t_len = 0;
#endif /* ENABLE_MBLEN */

	for (c = (unsigned char *) buf; *c; c++) {
#ifdef ENABLE_MBLEN
		if (!my_isprint(*c) && (t_len = mblen((const char *) c, MAX(2,MB_CUR_MAX))) <= 1)
			*c = '?';
		while (--t_len > 0)
			c++;
#else
		if (!my_isprint(*c))
			*c = '?';
#endif /* ENABLE_MBLEN */
	}
}


/*
 * Same as convert_to_printable() but allows Backspace (ASCII 8), TAB (ASCII
 * 9), and FormFeed (ASCII 12) according to son of RFC 1036 section 4.4;
 * LineFeed (ASCII 10) and CarriageReturn (ASCII 13) are allowed, too.
 */
void
convert_body2printable (
	char *buf)
{
	unsigned char *c;
#ifdef ENABLE_MBLEN
	int t_len = 0;
#endif /* ENABLE_MBLEN */

	for (c = (unsigned char *)buf; *c; c++) {
#ifdef ENABLE_MBLEN
		if (!(my_isprint(*c) || *c == 8 || *c == 9 || *c == 10 || *c == 12 || *c == 13) && (t_len = mblen((const char *) c, MAX(2,MB_CUR_MAX))) <= 1)
			*c = '?';
		while (--t_len > 0)
			c++;
#else
		if (!(my_isprint(*c) || *c == 8 || *c == 9 || *c == 10 || *c == 12 || *c == 13))
			*c = '?';
#endif /* ENABLE_MBLEN */
	}
}
