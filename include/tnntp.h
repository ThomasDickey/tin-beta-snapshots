/*
 *  Project   : tin - a Usenet reader
 *  Module    : tnntp.h
 *  Author    : Thomas Dickey <dickey@invisible-island.net>
 *  Created   : 1997-03-05
 *  Updated   : 2003-09-19
 *  Notes     : #include files, #defines & struct's
 *
 * Copyright (c) 1997-2006 Thomas Dickey <dickey@invisible-island.net>
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


#ifndef TNNTP_H
#define TNNTP_H 1

#if defined(VMS) && defined(SOCKETSHR_TCP)
#	ifdef __GNUC__
#		define __SOCKET_TYPEDEFS
#	endif /* __GNUC__ */
#	include <netdb.h>
#	include <in.h>
#	include <inet.h>
#	include <socket.h>
#	define s_printf	si_fprintf
#	define s_fdopen	si_fdopen
#	define s_flush	si_fflush
#	define s_fclose	si_fclose
#	define s_gets	si_fgets
#	define s_close	si_close
#	define s_puts	si_fputs
#	define s_dup
#	define s_init()	(1)
#	define s_end()
#else
#	define s_printf	fprintf
#	define s_fdopen	fdopen
#	define s_flush	fflush
#	define s_fclose	fclose
#	define s_gets	fgets
#	if 0 /* __BEOS__ port in progress */
#		ifdef HAVE_CLOSESOCKET
#			define s_close closesocket
#		else
#			define s_close	close
#		endif /* HAVE_CLOSESOCKET */
#		ifdef __BEOS__
#			define s_puts(s,fd)	write(fileno(fd),s,strlen(s))
#		else
#			define s_puts(s,fd)	fputs(s,fd)
#		endif /* __BEOS__ */
#	else
#		define s_close close
#		define s_puts	fputs
#	endif /* 0 */
#	define s_dup		dup
#	define s_init()	(1)
#	define s_end()
#endif /* VMS && SOCKETSHR_TCP */

#if defined(NNTP_ABLE) || defined(HAVE_GETHOSTBYNAME)
#	ifdef HAVE_NETDB_H
#		include <netdb.h>
#	endif /* HAVE_NETDB_H */
#	define IPPORT_NNTP ((unsigned short) 119)
#	ifdef TLI
#		ifdef HAVE_FCNTL_H
#			include	<fcntl.h>
#		endif /* HAVE_FCNTL_H */
#		include	<tiuser.h>
#		ifdef HAVE_STROPTS_H
#			include	<stropts.h>
#		endif /* HAVE_STROPTS_H */
#		ifdef HAVE_NET_SOCKET_H /* __BEOS__ fd_* macros */
#			include <net/socket.h>
#		else
#			ifdef HAVE_SYS_SOCKET_H
#				include	<sys/socket.h>
#			else
#				ifdef HAVE_SOCKET_H
#					include <socket.h>
#				endif /* HAVE_SOCKET_H */
#			endif /* HAVE_SYS_SOCKET_H */
#		endif /* HAVE_NET_SOCKET_H */
#		ifdef HAVE_NETINET_IN_H
#			include	<netinet/in.h>
#		endif /* HAVE_NETINET_IN_H */
#	else
#		ifdef VMS
#			ifdef MULTINET
#				include "MULTINET_ROOT:[multinet.include]errno.h"
#				include "MULTINET_ROOT:[multinet.include]netdb.h"
#				include "MULTINET_ROOT:[multinet.include.vms]inetiodef.h"
#				include "MULTINET_ROOT:[multinet.include.sys]socket.h"
#				include "MULTINET_ROOT:[multinet.include.netinet]in.h"
#				define netopen	socket_open
#				define netread	socket_read
#				define netwrite socket_write
#				define netclose socket_close
#			else
#				ifdef UCX
#					include <errno.h>
#					include <iodef.h>
#					include <in.h>
#					include <socket.h>
#					define netopen	open
#					define netread	read
#					define netwrite	write
#					define netclose	close
#				endif /* UCX */
#			endif /* MULTINET */
#		else
#			ifdef HAVE_SYS_SOCKET_H
#				include <sys/socket.h>
#			else
#				ifdef HAVE_SOCKET_H
#					include <socket.h>
#				endif /* HAVE_SOCKET_H */
#			endif /* HAVE_SYS_SOCKET_H */
#			ifdef HAVE_NETINET_IN_H
#				include <netinet/in.h>
#			endif /* HAVE_NETINET_IN_H */
#			ifdef HAVE_NETLIB_H
#				include <netlib.h>
#			endif /* HAVE_NETLIB_H */
#			ifdef HAVE_ARPA_INET_H
#				include <arpa/inet.h>
#			endif /* HAVE_ARPA_INET_H */
#		endif /* VMS */
#	endif /* TLI */

#	ifdef EXCELAN
		extern int connect (int, struct sockaddr *);
		extern unsigned short htons (unsigned short);
		extern unsigned long rhost (char **);
		extern int rresvport (int);
		extern int socket (int, struct sockproto *, struct sockaddr_in *, int);
#	endif /* EXCELAN */

#	ifdef DECNET
#		include <netdnet/dn.h>
#		include <netdnet/dnetdb.h>
#	endif /* DECNET */

#endif /* NNTP_ABLE || HAVE_GETHOSTBYNAME */

#if defined(HOST_NAME_MAX) && !defined(MAXHOSTNAMELEN)
#	define MAXHOSTNAMELEN	HOST_NAME_MAX
#endif /* HOST_NAME_MAX && !MAXHOSTNAMELEN */
#ifndef MAXHOSTNAMELEN
#	define MAXHOSTNAMELEN 255
#endif /* !MAXHOSTNAMELEN */

#ifndef SOCKS
#	ifdef DECL_CONNECT
	extern int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
#	endif /* DECL_CONNECT */
#endif /* !SOCKS */

#ifdef DECL_INET_NTOA
	extern char *inet_ntoa (struct in_addr);
#endif /* DECL_INET_NTOA */

#endif /* TNNTP_H */
