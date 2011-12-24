/*
 *  Project   : tin - a Usenet reader
 *  Module    : debug.h
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   :
 *  Updated   : 2008-12-12
 *  Notes     :
 *
 * Copyright (c) 2007-2012 Urs Janssen <urs@tin.org>
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


#ifndef DEBUG_H
#	define DEBUG_H 1

#	ifdef NNTP_ABLE
#		define DEBUG_NNTP	0x01	/* 1 */
#	else
#		define DEBUG_NNTP	0x00	/* disabled */
#	endif /* NNTP_ABLE */
#	define DEBUG_FILTER	0x02	/* 2 */
#	define DEBUG_NEWSRC	0x04	/* 4 */
#	define DEBUG_REFS	0x08	/* 8 */
#	define DEBUG_MEM	0x10	/* 16 */
#	define DEBUG_ATTRIB	0x20	/* 32 */
#	define DEBUG_MISC	0x40	/* 64 */
#	define DEBUG_ALL	0x7f	/* 127 */

#	if 0 /* this is very noisy */
#		define DEBUG_IO(x)	fprintf x
#	else
#		define DEBUG_IO(x)	/* nothing */
#	endif /* 0 */
#endif /* !DEBUG_H */
