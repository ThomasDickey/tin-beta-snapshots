/*
 *  Project   : tin - a Usenet reader
 *  Module    : version.h
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 1999-11-13
 *  Notes     :
 *
 * Copyright (c) 1991-2000 Iain Lea <iain@bricbrac.de>
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


#ifndef VERSION_H
#	define VERSION_H	1

#	define PRODUCT		"tin"
#	define VERSION		"1.5.2"
#	define RELEASEDATE	"20000119"
#	define RELEASENAME	"Sumerland"
#	define TINRC_VERSION	"1.2"

#	ifdef M_AMIGA
#		define OSNAME	"AMIGA"
#		define AMIVER	VERSION
#	endif /* M_AMIGA */

#	ifdef M_OS2
#		define OSNAME	"OS/2"
#	endif /* M_OS2 */

#	ifdef M_UNIX
#		if !defined(__amiga)
#			define OSNAME	"UNIX"
#		else
#			define OSNAME	"AMIGA"
#		endif /* !__amiga */
#	endif /* M_UNIX */

#	ifdef WIN32
#		define OSNAME	"Windows/NT"
#	endif /* WIN32 */

#	ifndef OSNAME
#		define OSNAME	"Unknown"
#	endif /* !OSNAME */

#endif /* !VERSION_H */
