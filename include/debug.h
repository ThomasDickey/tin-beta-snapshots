/*
 *  Project   : tin - a Usenet reader
 *  Module    : debug.h
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   :
 *  Updated   : 2024-11-14
 *  Notes     :
 *
 * Copyright (c) 2007-2025 Urs Janssen <urs@tin.org>
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


#ifndef DEBUG_H
#	define DEBUG_H 1

#	define DEBUG_NNTP	0x01	/*   1, "NNTP", "NNTPS" */
#	define DEBUG_FILTER	0x02	/*   2, "FILTER", "ARTS"  */
#	define DEBUG_NEWSRC	0x04	/*   4, "NEWSRC", "BITMAP" */
#	define DEBUG_REFS	0x08	/*   8, "THREADING", "REFS" */
#	define DEBUG_MEM	0x10	/*  16, "MEMORY", "MALLOC" */
#	define DEBUG_ATTRIB	0x20	/*  32, "ATTRIBUTES" */
#	define DEBUG_MISC	0x40	/*  64, "MISC", "GNKSA", "ACTIVE" */
#	define DEBUG_ALL	0x7f	/* 127, "ALL", "EVERYTHING" */
#	define DEBUG_REMOVE	0x80	/* 128, "REMOVE", "DELETE" */


#	if 0 /* this is very noisy */
#		define DEBUG_IO(x)	fprintf x
#	else
#		define DEBUG_IO(x)	/* nothing */
#	endif /* 0 */
#endif /* !DEBUG_H */
