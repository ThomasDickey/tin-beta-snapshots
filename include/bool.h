/*
 *  Project   : tin - a Usenet reader
 *  Module    : bool.h
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   :
 *  Updated   : 2003-05-16
 *  Notes     :
 *
 * Copyright (c) 1997-2006 Urs Janssen <urs@tin.org>
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


#ifndef BOOL_H
#	define BOOL_H 1

#	if 0
/*
 * This is the correct way, but causes problems on some systems
 * e.g. SuSE-7.3 (IA-32)
 */
#		ifndef __cplusplus
#			ifdef HAVE_STDBOOL_H
#				include <stdbool.h>
#				ifdef _Bool
	typefdef _Bool t_bool;
#				endif /* _Bool */
#			endif /* HAVE_STDBOOL_H */
#		endif /* __cplusplus */
#		ifndef FALSE
#			define FALSE 0
#		endif /* !FALSE */
#		ifndef TRUE
#			define TRUE (!FALSE)
#		endif /* !TRUE */
#		ifndef t_bool
	typedef unsigned t_bool;	/* don't make this a char or short! */
#		endif /* t_bool */

#	else

#		ifndef FALSE
#			define FALSE 0
#		endif /* !FALSE */

#		ifndef TRUE
#			define TRUE (!FALSE)
#		endif /* !TRUE */

	typedef unsigned t_bool;	/* don't make this a char or short! */

#	endif /* 0 */

	extern /*@unused@*/ t_bool bool_equal (t_bool, t_bool) /*@*/ ;
#	define bool_equal(a,b) ((a) ? (b) : !(b))

	extern /*@unused@*/ t_bool bool_not (t_bool) /*@*/ ;
#	define bool_not(b) ((b) ? FALSE : TRUE)

	extern /*@unused@*/ /*@observer@*/ char *bool_unparse (t_bool) /*@*/ ;
#	define bool_unparse(b) ((b) ? "TRUE" : "FALSE")

#endif /* !BOOL_H */
