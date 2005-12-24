/*
 *  Project   : tin - a Usenet reader
 *  Module    : bugrep.h
 *  Author    : Urs Janssen <urs@tin.org>
 *  Created   : 1997-07-30
 *  Updated   : 2003-09-19
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


#ifndef BUGREP_H
#	define BUGREP_H	1

#	ifdef M_UNIX
#		define BUG_REPORT_ADDRESS	"tin-bugs@tin.org"
#	endif /* M_UNIX */

#	ifdef VMS
#		define BUG_REPORT_ADDRESS	"stenns@vw.tci.uni-hannover.de"
#	endif /* VMS */

/* fallback */
#	ifndef BUG_REPORT_ADDRESS
#		define BUG_REPORT_ADDRESS	"tin-bugs@tin.org"
#	endif /* BUG_REPORT_ADDRESS */

#endif /* !BUGREP_H */
