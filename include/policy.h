/*
 *  Project   : tin - a Usenet reader
 *  Module    : policy.h
 *  Author    : R.Doeblitz
 *  Created   : 1999-01-12
 *  Updated   :
 *  Notes     : #defines and static data for policy configuration
 *
 * Copyright (c) 1999-2000 Ralf Doeblitz <doeblitz@gmx.de>
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
 *    This product includes software developed by Ralf Doeblitz.
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


/*
 * tables for TLD search in misc.c, gnksa domain checking
 */

#ifndef TIN_POLICY_H
#	define TIN_POLICY_H 1
/*
 * known two letter country codes
 */
static char gnksa_country_codes[26*26] = {
/*      A B C D E  F G H I J  K L M N O  P Q R S T  U V W X Y Z */
/* A */ 0,0,1,1,1, 1,1,0,1,0, 0,1,1,1,1, 0,1,1,1,1, 1,0,1,0,0,1,
/* B */ 1,1,0,1,1, 1,1,1,1,1, 0,0,1,1,1, 0,0,1,1,1, 0,1,1,0,1,1,
/* C */ 1,0,1,1,0, 1,1,1,1,0, 1,1,1,1,1, 0,0,1,0,0, 1,1,0,1,1,1,
/* D */ 0,0,0,0,1, 0,0,0,0,1, 1,0,1,0,1, 0,0,0,0,0, 0,0,0,0,0,1,
/* E */ 0,0,1,0,1, 0,1,1,0,0, 0,0,0,0,0, 0,0,1,1,1, 0,0,0,0,0,0,
/* F */ 0,0,0,0,0, 0,0,0,1,1, 1,0,1,0,1, 0,0,1,0,0, 0,0,0,1,0,0,
/* G */ 1,1,0,1,1, 1,1,1,1,0, 0,1,1,1,0, 1,1,1,1,1, 1,0,1,0,1,0,
/* H */ 0,0,0,0,0, 0,0,0,0,0, 1,0,1,1,0, 0,0,1,0,1, 1,0,0,0,0,0,
/* I */ 0,0,0,1,1, 0,0,0,0,0, 0,1,1,1,1, 0,1,1,1,1, 0,0,0,0,0,0,
/* J */ 0,0,0,0,1, 0,0,0,0,0, 0,0,1,0,1, 1,0,0,0,0, 0,0,0,0,0,0,
/* K */ 0,0,0,0,1, 0,1,0,1,0, 0,0,1,1,0, 1,0,1,0,0, 0,0,1,0,1,1,
/* L */ 1,1,1,0,0, 0,0,0,1,0, 1,0,0,0,0, 0,0,1,1,1, 1,1,0,0,1,0,
/* M */ 1,0,1,1,0, 0,1,1,0,0, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1,1,
/* N */ 1,0,1,0,1, 1,1,0,1,0, 0,1,0,0,1, 1,0,1,0,0, 1,0,0,0,0,1,
/* O */ 0,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0,0,
/* P */ 1,0,0,0,1, 1,1,1,0,0, 1,1,1,1,0, 0,0,1,0,1, 0,0,1,0,1,0,
/* Q */ 1,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,0,
/* R */ 0,0,0,0,1, 0,0,0,0,0, 0,0,0,0,1, 0,0,0,0,0, 1,0,1,0,0,0,
/* S */ 1,1,1,1,1, 0,1,1,1,1, 1,1,1,1,1, 0,0,1,0,1, 1,1,0,0,1,1,
/* T */ 0,0,1,1,0, 1,1,1,0,1, 1,0,1,1,1, 1,0,1,0,1, 0,1,1,0,0,1,
/* U */ 1,0,0,0,0, 0,1,0,0,0, 1,0,1,0,0, 0,0,0,1,0, 0,0,0,0,1,1,
/* V */ 1,0,1,0,1, 0,1,0,1,0, 0,0,0,1,0, 0,0,0,0,0, 1,0,0,0,0,0,
/* W */ 0,0,0,0,0, 1,0,0,0,0, 0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0,0,
/* X */ 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,0,
/* Y */ 0,0,0,0,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,1, 1,0,0,0,0,0,
/* Z */ 1,0,0,0,0, 0,0,0,0,0, 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0,1
/*      A B C D E  F G H I J  K L M N O  P Q R S T  U V W X Y Z */
};

/*
 * valid domains with 3 or more characters
 *
 * later add: nom, rec, web, arts, firm, info, shop
 */
static const char *gnksa_domain_list[] = {
	"com",
	"edu",
	"gov",
	"int",
	"mil",
	"net",
	"org",
	"arpa",
	"uucp",
	"bitnet",
#	if 0
	/* the new domain names, not yet valid */
	"nom",
	"rec",
	"web",
	"arts",
	"firm",
	"info",
	"shop",
#	endif /* 0 */
	/* the next four are defined in RFC 2606 */
	"invalid",
#	if 0
	/* but three of them shoudn't be used on usenet */
	"test",
	"example",
	"localhost",
#	endif /* 0 */
#	ifdef TINC_DNS
	"bofh",
#	endif /* TINC_DNS */
	/* sentinel */
	""
};
#endif /* !TIN_POLICY_H */

/* end of file */
