/*
 *  Project   : tin - a Usenet reader
 *  Module    : version.h
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2003-09-19
 *  Notes     :
 *
 * Copyright (c) 1991-2004 Iain Lea <iain@bricbrac.de>
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


#ifndef VERSION_H
#	define VERSION_H	1

#	define PRODUCT		"tin"
#	ifndef TIN_AUTOCONF_H
#		define VERSION		"1.7.4"
#	endif /* !TIN_AUTOCONF_H */
#	define RELEASEDATE	"20040111"
#	define RELEASENAME	"Taransay"
/* config-file versions - must by dotted triples */
#	define TINRC_VERSION	"1.3.7"
#	define ATTRIBUTES_VERSION	"1.0.2"
#	define FILTER_VERSION	"1.0.0"
#	define SERVERCONFIG_VERSION	"1.0.0"

#	ifdef VMS
#		define OSNAME	"VMS"
#	endif /* VMS */

#	ifdef M_UNIX
#		ifdef __BEOS__
#			define OSNAME	"BeOS"
#		endif /* __BEOS__ */
#		ifdef __CYGWIN__
#			ifdef __WINNT__ /* also defined on Win95 *sigh* */
#				define OSNAME	"Windows/NT"
#			endif /*__WINNT__ */
#		endif /* __CYGWIN__ */
#		ifndef OSNAME
#			define OSNAME	"UNIX"
#		endif /* !OSNAME */
#	endif /* M_UNIX */

#	ifndef OSNAME
#		define OSNAME	"Unknown"
#	endif /* !OSNAME */

#endif /* !VERSION_H */
