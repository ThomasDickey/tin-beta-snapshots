/*
 *  Project   : tin - a Usenet reader
 *  Module    : tin.h
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2023-02-06
 *  Notes     : #include files, #defines & struct's
 *
 * Copyright (c) 1997-2023 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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


/*
 * OS specific doda's
 */

#ifndef TIN_H
#define TIN_H 1

#ifdef HAVE_CONFIG_H
#	ifndef TIN_AUTOCONF_H
#		include <autoconf.h>	/* FIXME: normally we use 'config.h' */
#	endif /* !TIN_AUTOCONF_H */
#else
#	ifndef HAVE_CONFDEFS_H
#		error "configure run missing"
#	endif /* !HAVE_CONFDEFS_H */
#endif /* HAVE_CONFIG_H */


/*
 * this causes trouble on Linux (forces nameserver lookups even for local
 * connections - this is an (unwanted) feature of getaddrinfo) see also
 * nntplib.c
 */
/* IPv6 support */
#if defined(HAVE_GETADDRINFO) && defined(HAVE_GAI_STRERROR) && defined(ENABLE_IPV6)
#	define INET6
#endif /* HAVE_GETADDRINFO && HAVE_GAI_STRERROR && ENABLE_IPV6 */


/*
 * Native Language Support.
 */
#ifndef NO_LOCALE
#	ifdef HAVE_LOCALE_H
#		include <locale.h>
#	endif /* HAVE_LOCALE_H */
#	ifndef HAVE_SETLOCALE
#		define setlocale(Category, Locale) /* empty */
#	endif /* !HAVE_SETLOCALE */
#endif /* !NO_LOCALE */

#define N_(Str) Str

#if defined(ENABLE_NLS) && !defined(__BUILD__)
#	ifdef HAVE_LIBINTL_H
#		include <libintl.h>
#	endif /* HAVE_LIBINTL_H */
#	define _(Text)	gettext(Text)
#else
#	undef bindtextdomain
#	define bindtextdomain(Domain, Directory) /* empty */
#	undef textdomain
#	define textdomain(Domain) /* empty */
#	define _(Text) Text
#endif /* ENABLE_NLS && !__BUILD__ */

#ifndef LOCALEDIR
#	define LOCALEDIR "/usr/share/locale"
#endif /* !LOCALEDIR */

#if defined(__amiga__) || defined(__amiga)
#	define SMALL_MEMORY_MACHINE
#endif /* __amiga__ || __amiga */

#include <signal.h>

enum context { cMain, cArt, cAttachment, cAttrib, cConfig, cFilter, cGroup, cInfopager, cPage, cPOSTED, cPost, cPostCancel, cPostFup, cReconnect, cScope, cSelect, cThread, cURL };
enum icontext { cNone, cGetline, cPromptCONT, cPromptSLK, cPromptYN };
enum resizer { cNo, cYes, cRedraw };
enum rc_state { RC_IGNORE, RC_UPGRADE, RC_DOWNGRADE, RC_ERROR };

#include <stdio.h>
#ifdef HAVE_ERRNO_H
#	include <errno.h>
#else
#	ifdef HAVE_SYS_ERRNO_H
#		include <sys/errno.h>
/*
	#	else
	#		error "No errno.h or sys/errno.h found"
*/
#	endif /* HAVE_SYS_ERRNO_H */
#endif /* HAVE_ERRNO_H */
#if !defined(errno)
#	ifdef DECL_ERRNO
		extern int errno;
#	endif /* DECL_ERRNO */
#endif /* !errno */
#if !defined(EHOSTUNREACH)
#	define EHOSTUNREACH 113
#endif /* !EHOSTUNREACH */

#ifdef HAVE_STDDEF_H
#	include <stddef.h>
#endif /* HAVE_STDDEF_H */
#ifdef HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_STAT_H
#	include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef TIME_WITH_SYS_TIME
#	include <sys/time.h>
#	include <time.h>
#else
#	ifdef HAVE_SYS_TIME_H
#		include <sys/time.h>
#	else
#		include <time.h>
#	endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

#ifdef HAVE_SYS_TIMES_H
#	include <sys/times.h>
#endif /* HAVE_SYS_TIMES_H */

#if defined(HAVE_LIBC_H) && defined(__NeXT__)
#	include <libc.h>
#endif /* HAVE_LIBC_H && __NeXT__ */

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_PWD_H
#	include <pwd.h>
#endif /* HAVE_PWD_H */

#ifdef HAVE_SYS_PARAM_H
#	include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#include <ctype.h>

#ifdef HAVE_STDLIB_H
#	include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include <stdarg.h>

#ifdef HAVE_GETOPT_H
#	include <getopt.h>
#endif /* HAVE_GETOPT_H */

#if defined(ENABLE_LONG_ARTICLE_NUMBERS) && !defined(SMALL_MEMORY_MACHINE)
/*
 * defines and typedefs for 64 bit article numbers
 *
 * TODO: what if !CPP_DOES_CONCAT
 *       add configure check for PRIdLEAST64
 *       add configure check for SCNdLEAST64
 *       add configure check for INT64_MAX, LLONG_MAX, LONG_MAX
 */
#	if defined(HAVE_INT_LEAST64_T) && !defined(HAVE_INTTYPES_H)
#		undef HAVE_INT_LEAST64_T
#	endif /* HAVE_INT_LEAST64_T && !HAVE_INTTYPES_H */
#	if defined(HAVE_INT_LEAST64_T) || defined(HAVE_LONG_LONG) || defined(quad_t)
#		if defined(HAVE_ATOLL) || defined(HAVE_ATOQ)
#			ifdef HAVE_STRTOLL
#				define USE_LONG_ARTICLE_NUMBERS 1
#			endif /* HAVE_STRTOLL */
#		endif /* HAVE_ATOLL || HAVE_ATOQ */
#	endif /* HAVE_INT_LEAST64_T || HAVE_LONG_LONG || quad_t */
#	ifdef HAVE_STDINT_H
#		include <stdint.h>
#	endif	/* HAVE_STDINT_H */
#endif /* ENABLE_LONG_ARTICLE_NUMBERS && !SMALL_MEMORY_MACHINE */
#ifdef USE_LONG_ARTICLE_NUMBERS
#	if defined(HAVE_INT_LEAST64_T) && defined(HAVE_INT64_C)
#		include <inttypes.h>
		typedef int_least64_t t_artnum;
#		define T_ARTNUM_PFMT PRIdLEAST64
#		define T_ARTNUM_SFMT SCNdLEAST64
#		define T_ARTNUM_CONST(v) INT64_C(v)
#		define ARTNUM_MAX INT64_MAX
#	else
		typedef long long t_artnum;
#		define T_ARTNUM_PFMT "lld"
#		define T_ARTNUM_SFMT "lld"
#		define T_ARTNUM_CONST(v) v ## LL
#		define ARTNUM_MAX LLONG_MAX
#	endif /* HAVE_INT_LEAST64_T && HAVE_INT64_C */
#	ifdef HAVE_ATOLL
#		define atoartnum atoll
#	else
#		define atoartnum atoq
#	endif /* HAVE_ATOLL */
#	define strtoartnum strtoll
#else
	typedef long t_artnum;
#	define T_ARTNUM_PFMT "ld"
#	define T_ARTNUM_SFMT "ld"
#	define T_ARTNUM_CONST(v) v ## L
#	define ARTNUM_MAX LONG_MAX
#	define atoartnum atol
#	define strtoartnum strtol
#endif /* USE_LONG_ARTICLE_NUMBERS */

/*
 * FIXME: make this autoconf
 */
#ifndef __QNX__
#	ifdef HAVE_STRING_H
#		include <string.h>
#	else
#		ifdef HAVE_STRINGS_H
#			include <strings.h>
#		endif /* HAVE_STRINGS_H */
#	endif /* HAVE_STRING_H */
#else
#	ifdef HAVE_STRING_H
#		include <string.h>
#	endif /* HAVE_STRING_H */
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif /* HAVE_STRINGS_H */
#endif /* !__QNX__ */

/*
 * FIXME: make this autoconf
 */
#ifdef SEIUX
#	include <bsd/sys/time.h>
#	include <bsd/sys/signal.h>
#	include <bsd/sys/types.h>
#	include <posix/unistd.h>
#	include <bsd/netinet/in.h>
#endif /* SEIUX */
/* *-next-nextstep3 && *-next-openstep4 */
#ifdef __NeXT__
#	undef HAVE_SYS_UTSNAME_H
#	undef HAVE_UNAME
#	undef WEXITSTATUS
#	undef WIFEXITED
#	define IGNORE_SYSTEM_STATUS 1
#	define HAVE_SYS_WAIT_H 1
#	define NO_LOCKING 1
#endif /* __NeXT__ */
/* m68k-sun-sunos3.5 */
#if defined(sun) || defined(__sun) && (!defined(__SVR4) || !defined(__svr4__)) && defined(BSD) && BSD < 199306
#	define IGNORE_SYSTEM_STATUS 1
#endif /* sun|| __sun && (!__SVR4 || __svr4__) && BSD && BSD < 199306 */

#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_SYS_IOCTL_H
#	include <sys/ioctl.h>
/* We don't need/use these, and they cause redefinition errors with SunOS 4.x
 * when we include termio.h or termios.h
 */
#	if defined(sun) && !defined(__svr4)
#		undef NL0
#		undef NL1
#		undef CR0
#		undef CR1
#		undef CR2
#		undef CR3
#		undef TAB0
#		undef TAB1
#		undef TAB2
#		undef XTABS
#		undef BS0
#		undef BS1
#		undef FF0
#		undef FF1
#		undef ECHO
#		undef NOFLSH
#		undef TOSTOP
#		undef FLUSHO
#		undef PENDIN
#	endif /* sun && !__svr4 */
#endif /* HAVE_SYS_IOCTL_H */

#ifdef HAVE_PROTOTYPES_H
#	include <prototypes.h>
#endif /* HAVE_PROTOTYPES_H */

#ifdef HAVE_SYS_UTSNAME_H
#	include <sys/utsname.h>
#endif /* HAVE_SYS_UTSNAME_H */

/*
 * Needed for catching child processes
 */
#ifdef HAVE_SYS_WAIT_H
#	include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#ifndef WEXITSTATUS
#	define WEXITSTATUS(status)	((int) (((status) >> 8) & 0xFF))
#endif /* !WEXITSTATUS */

#ifndef WIFEXITED
#	define WIFEXITED(status)	((int) (((status) & 0xFF) == 0))
#endif /* !WIFEXITED */

/*
 * Needed for timeout in user abort of indexing a group (BSD & SYSV variaties)
 */
#ifdef HAVE_SYS_SELECT_H
#	ifdef NEED_TIMEVAL_FIX
#		define timeval fake_timeval
#		include <sys/select.h>
#		undef timeval
#	else
#		include <sys/select.h>
#	endif /* NEED_TIMEVAL_FIX */
#endif /* HAVE_SYS_SELECT_H */

#if defined(HAVE_STROPTS_H) && !defined(__dietlibc__)
#	include <stropts.h>
#endif /* HAVE_STROPTS_H && !__dietlibc__ */

#ifdef HAVE_POLL_H
#	include <poll.h>
#else
#	ifdef HAVE_SYS_POLL_H
#		include <sys/poll.h>
#	endif /* HAVE_SYS_POLL_H */
#endif /* HAVE_POLL_H */

/*
 * Directory handling code
 */
#ifdef HAVE_DIRENT_H
#	include <dirent.h>
/* #	define NAMLEN(dirent) strlen((dirent)->d_name) */
#else
#	define dirent direct
/* #	define NAMLEN(dirent) (dirent)->d_namlen */
#	ifdef HAVE_SYS_NDIR_H
#		include <sys/ndir.h>
#	endif /* HAVE_SYS_NDIR_H */
#	ifdef HAVE_SYS_DIR_H
#		include <sys/dir.h>
#	endif /* HAVE_SYS_DIR_H */
#	ifdef HAVE_NDIR_H
#		include <ndir.h>
#	endif /* HAVE_NDIR_H */
#endif /* HAVE_DIRENT_H */

#ifndef DIR_BUF
#	define DIR_BUF struct dirent
#endif /* !DIR_BUF */

#ifndef HAVE_UNLINK
#	define unlink(file)	remove(file)
#endif /* !HAVE_UNLINK */

/*
 * If native OS hasn't defined STDIN_FILENO be a smartass and do it
 */
#if !defined(STDIN_FILENO)
#	define STDIN_FILENO	0
#endif /* !STDIN_FILENO */

/*
 * include <paths.h> if available to define _PATH_TMP
 */
#ifdef HAVE_PATHS_H
#	include <paths.h>
#endif /* HAVE_PATHS_H */
#ifndef _PATH_TMP
#	define _PATH_TMP	"/tmp/"
#endif /* _PATH_TMP */

/*
 * If OS misses the isascii() function
 */
#if !defined(HAVE_ISASCII) && !defined(isascii)
#	define isascii(c) (!((c) & ~0177))
#endif /* !HAVE_ISASCII && !isascii */


/*
 * any pgp/gpp support possible and wanted
 * sort out possible conflicts: gpg is preferred over pgp5 over pgp
 */
#if defined(HAVE_PGP) || defined(HAVE_PGPK) || defined(HAVE_GPG)
#	define HAVE_PGP_GPG 1
#	if defined(HAVE_PGP) && defined(HAVE_PGPK)
#		undef HAVE_PGP
#	endif /* HAVE_PGP && HAVE_PGPK */
#	if defined(HAVE_PGPK) && defined(HAVE_GPG)
#		undef HAVE_PGPK
#	endif /* HAVE_PGPK && HAVE_GPG */
#	if defined(HAVE_PGP) && defined(HAVE_GPG)
#		undef HAVE_PGP
#	endif /* HAVE_PGP && HAVE_GPG */
#endif /* HAVE_PGP || HAVE_PGPK || HAVE_GPG */

/*
 * slrnface requires some things
 */
#if defined(HAVE_SLRNFACE) && defined(HAVE_MKFIFO) && defined(HAVE_FORK) && defined(HAVE_EXECLP) && defined(HAVE_WAITPID) && !defined(DONT_HAVE_PIPING) && !defined(X_DISPLAY_MISSING)
#	define XFACE_ABLE
#endif /* HAVE_SLRNFACE && HAVE_MKFIFO && HAVE_FORK && HAVE_EXECLP && HAVE_WAITPID && !DONT_HAVE_PIPING && !X_DISPLAY_MISSING */

/*
 * Setup support for reading from NNTP
 */
#if defined(NNTP_ABLE) || defined(NNTP_ONLY)
#	ifndef NNTP_ABLE
#		define NNTP_ABLE	1
#	endif /* !NNTP_ABLE */
#	ifndef NNTP_INEWS
#		define NNTP_INEWS	1
#	endif /* !NNTP_INEWS */
#else
#	ifdef XHDR_XREF
#		undef XHDR_XREF
#	endif /* XHDR_XREF */
#	ifdef BROKEN_LISTGROUP
#		undef BROKEN_LISTGROUP
#	endif /* BROKEN_LISTGROUP */
#endif /* NNTP_ABLE || NNTP_ONLY */

#define FAKE_NNTP_FP		(FILE *) 9999

/*
 * Max time between the first character of a VT terminal escape sequence
 * for special keys and the following characters to arrive (msec)
 */
#define SECOND_CHARACTER_DELAY	200

/*
 * Maximum time (seconds) for a VT terminal escape sequence
 */
#define VT_ESCAPE_TIMEOUT	1

/*
 * Determine machine configuration for external programs & directories
 */
#if defined(BSD)
/*
 * To catch 4.3 Net2 code base or newer
 * (e.g. FreeBSD 1.x, 4.3/Reno, NetBSD 0.9, 386BSD, BSD/386 1.1 and below).
 * use
 * #if (defined(BSD) && (BSD >= 199103))
 *
 * To detect if the code is being compiled on a 4.4 code base or newer
 * (e.g. FreeBSD 2.x, 4.4, NetBSD 1.0, BSD/386 2.0 or above).
 * use
 * #if (defined(BSD) && (BSD >= 199306))
 *
 * (defined in <sys/param.h>)
 */
#	ifndef HAVE_MEMCMP
#		define memcmp(s1, s2, n)	bcmp(s2, s1, n)
#	endif /* !HAVE_MEMCMP */
#	ifndef HAVE_MEMCPY
#		define memcpy(s1, s2, n)	bcopy(s2, s1, n)
#	endif /* !HAVE_MEMCPY */
#	ifndef HAVE_MEMSET
#		define memset(s1, s2, n)	bfill(s1, n, s2)
#	endif /* !HAVE_MEMSET */
#	ifndef HAVE_STRCHR
#		define strchr(str, ch)	index(str, ch)
#	endif /* !HAVE_STRCHR */
#	ifndef HAVE_STRRCHR
#		define strrchr(str, ch)	rindex(str, ch)
#	endif /* !HAVE_STRRCHR */
#	if defined(__386BSD__) || defined(__bsdi__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#		define DEFAULT_PRINTER	"/usr/bin/lpr"
#		define DEFAULT_SUM	"/usr/bin/cksum -o 1 <" /* use tailing <, otherwise get filename output too */
#	else
#		define DEFAULT_PRINTER	"/usr/ucb/lpr"
#		define DEFAULT_SUM	"sum"
#	endif /* __386BSD__ || __bsdi__ || __NetBSD__ || __FreeBSD__ || __OpenBSD__ */
#	ifdef DGUX
#		define USE_INVERSE_HACK
#	endif /* DGUX */
#	ifdef pyr
#		define DEFAULT_MAILER	"/usr/.ucbucb/mail"
#	endif /* pyr */
#else /* !BSD */
#	ifdef linux
#		define DEFAULT_PRINTER	"/usr/bin/lpr"
#	endif /* linux */
#	ifdef QNX42
#		ifndef DEFAULT_EDITOR
#			define DEFAULT_EDITOR	"/bin/vedit"
#		endif /* !DEFAULT_EDITOR */
#	endif /* QNX42 */
#	ifdef _AIX
#		define DEFAULT_PRINTER	"/bin/lp"
#		define READ_CHAR_HACK
#	endif /* _AIX */
#	ifdef sinix
#		define DEFAULT_PRINTER	"/bin/lpr"
#	endif /* sinix */
#	ifdef sysV68
#		define DEFAULT_MAILER	"/bin/rmail"
#	endif /* sysV68 */
#	ifdef UNIXPC
#		define DEFAULT_MAILER	"/bin/rmail"
#	endif /* UNIXPC */

#	ifndef DEFAULT_PRINTER
#		define DEFAULT_PRINTER	"/usr/bin/lp"
#	endif /* !DEFAULT_PRINTER */
#	ifndef PATH_SUM
#		define DEFAULT_SUM	"sum -r"
#	endif /* !PATH_SUM */
#endif /* BSD */

/*
 * fallback values
 */
#ifndef DEFAULT_EDITOR
#	define DEFAULT_EDITOR	"/usr/bin/vi"
#endif /* !DEFAULT_EDITOR */
#ifndef DEFAULT_MAILER
#	define DEFAULT_MAILER	"/usr/lib/sendmail"
#endif /* !DEFAULT_MAILER */
#ifndef DEFAULT_MAILBOX
#	define DEFAULT_MAILBOX	"/usr/spool/mail"
#endif /* !DEFAULT_MAILBOX */


/* FIXME: remove absolute-paths! */
/*
 * Miscellaneous program-paths
 */
#ifndef PATH_ISPELL
#	define PATH_ISPELL	"ispell"
#endif /* !PATH_ISPELL */

#ifndef PATH_METAMAIL	/* only unset if !HAVE_METAMAIL */
#	define PATH_METAMAIL	"metamail"
#endif /* !PATH_METAMAIL */
#define METAMAIL_CMD		PATH_METAMAIL" -e -p -m \"tin\""

#define INTERNAL_CMD	"--internal"

/*
 * Fix up the 'sum' path and parameter for './configure'd systems
 */
#ifdef PATH_SUM
#	ifdef DEFAULT_SUM
#		undef DEFAULT_SUM
#	endif /* DEFAULT_SUM */
#	ifdef SUM_TAKES_DASH_R
#		define DEFAULT_SUM PATH_SUM_R
#	else
#		define DEFAULT_SUM PATH_SUM
#	endif /* SUM_TAKES_DASH_R */
#endif /* PATH_SUM */

#ifdef HAVE_LONG_FILE_NAMES
#	define PATH_PART	"part"
#	define PATH_PATCH	"patch"
#	define INDEX_LOCK	"tin.%.256s.LCK"
#else
#	define PATH_PART	""
#	define PATH_PATCH	"p"
#	define INDEX_LOCK	"%.10s.LCK"
#endif /* HAVE_LONG_FILE_NAMES */

/* inews.c:submit_inews() and save.c:save_and_process_art() */
#define PATHMASTER 	"not-for-mail"

/*
 * How often should the active file be reread for new news
 */
#ifndef REREAD_ACTIVE_FILE_SECS
#	define REREAD_ACTIVE_FILE_SECS 1200	/* seconds (20 mins) */
#endif /* !REREAD_ACTIVE_FILE_SECS */

/*
 * Initial sizes of internal arrays for small (<4MB) & large memory machines
 */
#ifdef SMALL_MEMORY_MACHINE
#	define DEFAULT_ARTICLE_NUM	600
#	define DEFAULT_SAVE_NUM	10
#else
#	define DEFAULT_ARTICLE_NUM	1200
#	define DEFAULT_SAVE_NUM	30
#endif /* SMALL_MEMORY_MACHINE */
#define DEFAULT_ACTIVE_NUM	1800
#define DEFAULT_NEWNEWS_NUM	5
#define DEFAULT_MAPKEYS_NUM 100	/* ~remappable keys per level (avoid massiv reallocs) */
#define DEFAULT_SCOPE_NUM	8

#define RCDIR	".tin"
#define INDEX_MAILDIR	".mail"
#define INDEX_NEWSDIR	".news"
#define INDEX_SAVEDIR	".save"

#define ACTIVE_FILE	"active"
#define ACTIVE_MAIL_FILE	"active.mail"
#define ACTIVE_SAVE_FILE	"active.save"
#define ACTIVE_TIMES_FILE	"active.times"
#define ATTRIBUTES_FILE	"attributes"
#define CONFIG_FILE	"tinrc"
#define SERVERCONFIG_FILE	"serverrc"
#define DEFAULT_MAILDIR	"Mail"
#define DEFAULT_SAVEDIR	"News"
#define DEFAULT_URL_HANDLER "url_handler.pl"
/* Prefixes saved attachments with no set filename */
#define SAVEFILE_PREFIX		"unknown"


/* MMDF-mailbox separator */
#ifndef MMDFHDRTXT
#	define MMDFHDRTXT "\01\01\01\01\n"
#endif /* MMDFHDRTXT */


/*
 * all regexps are extended -> # must be quoted!
 */
#ifdef HAVE_COLOR
/* case insensitive */
#	define DEFAULT_QUOTE_REGEX	"^\\s{0,3}(?:[\\]{}>|:)]|\\w{1,3}[>|])(?!-)"
#	define DEFAULT_QUOTE_REGEX2	"^\\s{0,3}(?:(?:[\\]{}>|:)]|\\w{1,3}[>|])\\s*){2}(?!-[})>])"
#	define DEFAULT_QUOTE_REGEX3	"^\\s{0,3}(?:(?:[\\]{}>|:)]|\\w{1,3}[>|])\\s*){3}"
#endif /* HAVE_COLOR */

/* case insensitive */
#if 0 /* single words only */
#	define DEFAULT_SLASHES_REGEX	"(?:^|(?<=\\s))/[^\\s/]+/(?:(?=[,.!?;]?\\s)|$)"
#	define DEFAULT_STARS_REGEX	"(?:^|(?<=\\s))\\*[^\\s*]+\\*(?:(?=[,.!?;]?\\s)|$)"
#	define DEFAULT_UNDERSCORES_REGEX	"(?:^|(?<=\\s))_[^\\s_]+_(?:(?=[,.!?;]?\\s)|$)"
#	define DEFAULT_STROKES_REGEX	"(?:^|(?<=\\s))-[^-\\s]+-(?:(?=[,.!?;]?\\s)|$)"
#else /* multiple words */
#	define DEFAULT_SLASHES_REGEX	"(?:^|(?<=\\s))/(?(?=[^-*/_\\s][^/\\s])[^-*/_\\s][^/]*[^-*/_\\s]|[^/\\s])/(?:(?=[,.!?;]?\\s)|$)"
#	define DEFAULT_STARS_REGEX	"(?:^|(?<=\\s))\\*(?(?=[^-*/_\\s][^*\\s])[^-*/_\\s][^*]*[^-*/_\\s]|[^*\\s])\\*(?:(?=[,.!?;]?\\s)|$)"
#	define DEFAULT_UNDERSCORES_REGEX	"(?:^|(?<=\\s))_(?(?=[^-*/_\\s][^_\\s])[^-*/_\\s][^_]*[^-*/_\\s]|[^_\\s])_(?:(?=[,.!?;]?\\s)|$)"
#	define DEFAULT_STROKES_REGEX	"(?:^|(?<=\\s))-(?(?=[^-*/_\\s][^-\\s])[^-*/_\\s][^-]*[^-*/_\\s]|[^-\\s])-(?:(?=[,.!?;]?\\s)|$)"
#endif /* 0 */

/* case sensitive && ^-anchored */
#define DEFAULT_STRIP_RE_REGEX	"(?:R[eE](?:\\^\\d+|\\[\\d+\\])?|A[wW]|Odp|Sv):\\s"
/* case sensitive */
#define DEFAULT_STRIP_WAS_REGEX	"(?:(?<=\\S)|\\s)\\((?:[Ww]a[rs]|[Bb]y[l\\xb3]o):.*\\)\\s*$"
#define DEFAULT_U8_STRIP_WAS_REGEX	"(?:(?<=\\S)|\\s)\\((?:[Ww]a[rs]|[Bb]y[l\\x{0142}]o):.*\\)\\s*$"
/*
 * overkill regexp for balanced '()':
 * #define DEFAULT_STRIP_WAS_REGEX	"(?:(?<=\\S)|\\s)\\((?:[Ww]a[rs]|[Bb]y[l\xb3]o):(?:(?:[^)(])*(?:\\([^)(]*\\))*)+\\)\\s*$"
 */

/* case sensitive & ^-anchored */
#define UUBEGIN_REGEX	"begin\\s\\s?[0-7]{3,4}\\s+"
/* case sensitive & ^-anchored */
#define UUBODY_REGEX	"(?:`|.[\\x20-\\x60]{1,61})$"

/* case sensitive & ^-anchored */
#define SHAR_REGEX	"\\#(?:!\\s?(?:/usr)?/bin/sh|\\s?(?i)this\\sis\\sa\\sshell\\sarchive)"

/* slrn verbatim marks, case sensitive & ^-anchored */
#define DEFAULT_VERBATIM_BEGIN_REGEX	"#v\\+\\s$"
#define DEFAULT_VERBATIM_END_REGEX	"#v-\\s$"

/* quoted text from external sources */
#define DEFAULT_EXTQUOTE_REGEX "^\\|\\s"

/*
 * URL related regexs:
 * add TELNET (RFC 4248), WAIS (RFC 4156), IMAP (RFC 2192), NFS (RFC 2224)
 *     LDAP (RFC 2255), POP (RFC 2384), ...
 * add IPv6 (RFC 3986) support
 */
/*
 * case insensitive
 * TODO: - split out ftp (only ftp allows username:passwd@, RFC 1738)?
 *       - test IDNA (RFC 3490) case
 *       - adjust to follow RFC 3986 (section 2.3)
 */
#define URL_REGEX	"\\b(?:https?|ftp|gopher)://(?:[^:@/\\s]*(?::[^:@/\\s]*)?@)?(?:(?:(?:[^\\W_](?:(?:-|[^\\W_]){0,61}(?<!---)[^\\W_])?|xn--[^\\W_](?:-(?!-)|[^\\W_]){1,57}[^\\W_])\\.)+[a-z]{2,18}\\.?|localhost|(?:(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?)|\\[(?:(?:[0-9A-F]{0,4}:){1,7}[0-9A-F]{1,4}|(?:[0-9A-F]{0,4}:){1,3}(?:(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?))\\])(?::\\d+)?(?:[-a-zA-Z0-9()@:%_\\+.~#?&\\/=\\$,]*)"
/*
 * case insensitive
 * TOFO: check against RFC 6068
 */
#define MAIL_REGEX	"\\b(?:mailto:(?:[-\\w$.+!*'(),;/?:@&=]|%[\\da-f]{2})+)(?<!\\))"
/*
 * case insensitive
 * TODO: check against RFC 5538
 */
#define NEWS_REGEX "\\b(?:s?news|nntp):(?:(?:(?://(?:(?:[^\\W_](?:(?:-|[^\\W_]){0,61}(?<!---)[^\\W_])?|xn--[^\\W_](?:-(?!-)|[^\\W_]){1,57}[^\\W_])\\.)+[a-z]{2,14}\\.?|localhost|(?:(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?))(?::\\d+)?(?(?=[/])[^()\\^\\[\\]{}\\|\\x00-\\x1f\\x7f\\s\"<>'\\\\:,;]+|$))|[^\\^\\[\\]{}\\|\\x00-\\x1f\\x7f\\s<>\"():,;\\\\'/]+)\\b"

#if 0 /* not implemented */
/*
 * case insensitive
 */
#	define TELNET_REGEX	"\\btelnet://(?:[^:@/]*(?::[^:@/]*)?@)?(?:(?:[^\\W_](?:(?:-|[^\\W_]){0,61}(?<!---)[^\\W_])?\\.)+[a-z]{2,14}\\.?||localhost|(?:(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(?:2[0-4]\\d|25[0-5]|[01]?\\d\\d?))(?::\\d+)?/?"
#endif /* 0 */


#define FILTER_FILE	"filter"
#define INPUT_HISTORY_FILE	".inputhistory"
#ifdef HAVE_MH_MAIL_HANDLING
#	define MAILGROUPS_FILE	"mailgroups"
#endif /* HAVE_MH_MAIL_HANDLING */
#define NEWSRC_FILE	".newsrc"
#define NEWSRCTABLE_FILE	"newsrctable"
/* ifdef APPEND_PID (default) NEWNEWSRC_FILE will be .newnewsrc<pid> */
#define NEWNEWSRC_FILE	".newnewsrc"
#define OLDNEWSRC_FILE	".oldnewsrc"
#ifndef OVERVIEW_FILE
#	define OVERVIEW_FILE	".overview"
#endif /* !OVERVIEW_FILE */
#define OVERVIEW_FMT	"overview.fmt"
#define POSTED_FILE	"posted"
#define POSTPONED_FILE	"postponed.articles"
#define SUBSCRIPTIONS_FILE	"subscriptions"
#define NEWSGROUPS_FILE	"newsgroups"
#define KEYMAP_FILE	"keymap"

#define SIGDASHES "-- \n"

#ifndef BOOL_H
#	include "bool.h"
#endif /* BOOL_H */

/* Philip Hazel's Perl regular expressions library */
#ifdef HAVE_LIB_PCRE2

#define PCRE2_CODE_UNIT_WIDTH 0
#include <pcre2.h>

#define REGEX_CASELESS PCRE2_CASELESS
#define REGEX_ANCHORED PCRE2_ANCHORED
#define REGEX_NOTEMPTY PCRE2_NOTEMPTY

#define REGEX_ERROR_NOMATCH PCRE2_ERROR_NOMATCH

#define REGEX_OPTIONS uint32_t
#define REGEX_NOFFSET uint32_t
#define REGEX_SIZE size_t

#else /* HAVE_LIB_PCRE2 */

#include <pcre.h>

#define REGEX_CASELESS PCRE_CASELESS
#define REGEX_ANCHORED PCRE_ANCHORED
#define REGEX_NOTEMPTY PCRE_NOTEMPTY

#define REGEX_ERROR_NOMATCH PCRE_ERROR_NOMATCH

#define REGEX_OPTIONS int
#define REGEX_NOFFSET int
#define REGEX_SIZE int

#endif /* HAVE_LIB_PCRE2 */

#ifdef HAVE_ICONV
#	define CHARSET_CONVERSION 1
#	ifdef HAVE_ICONV_H
#		include <iconv.h>
#	endif /* HAVE_ICONV_H */
#endif /* HAVE_ICONV */

#ifdef HAVE_LANGINFO_H
#	include <langinfo.h>
#else
#	ifdef HAVE_NL_TYPES_H
#		include <nl_types.h>
#	endif /* HAVE_NL_TYPES_H */
#endif /* HAVE_LANGINFO_H */
#ifndef HAVE_NL_ITEM
	typedef int nl_item;
#endif /* HAVE_NL_ITEM */
#ifndef CODESET
#	define CODESET ((nl_item) 1)
#endif /* CODESET */

#ifdef CHARSET_CONVERSION
#	define IS_LOCAL_CHARSET(c)	(!strncasecmp(tinrc.mm_local_charset, c, strlen(c)))
#else
#	define IS_LOCAL_CHARSET(c)	(!strncasecmp(tinrc.mm_charset, c, strlen(c)))
#endif /* CHARSET_CONVERSION */

/* TODO: move up to the 'right' place */
#ifdef HAVE_SYS_FILE_H
#	include <sys/file.h>
#endif /* HAVE_SYS_FILE_H */

#ifdef HAVE_LIBUTF8_H
#	include <libutf8.h>
#else
/*
 * order is important here:
 * - Solaris 2.5.1 requires wchar.h before wctype.h
 * - Tru64 with Desktop Toolkit C requires stdio.h before wchar.h
 * - BSD/OS 4.0.1 requires stddef.h, stdio.h and time.h before wchar.h
 */
#	ifdef HAVE_WCHAR_H
#		include <wchar.h>
#	endif /* HAVE_WCHAR_H */
#	ifdef HAVE_WCTYPE_H
#		include <wctype.h>
#	endif /* HAVE_WCTYPE_H */
#endif /* HAVE_LIBUTF8_H */

#ifndef MAX
#	define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif /* !MAX */
#ifndef MIN
#	define MIN(a,b)	(((a) > (b)) ? (b) : (a))
#endif /* !MIN */

#ifndef forever
/*@notfunction@*/
#	define forever	for(;;)
#endif /* !forever */

/*
 * safe strcpy into fixed-legth buffer
 */
#if 1
#	define STRCPY(dst, src)	(strncpy(dst, src, sizeof(dst) - 1), dst[sizeof(dst) - 1] = '\0')
#else
#	define STRCPY(dst, src)	(*(dst) = '\0', strncat(dst, src, sizeof(dst) - 1))
#endif /* 1 */

#define STRCMPEQ(s1, s2)	(strcmp((s1), (s2)) == 0)
#define STRNCMPEQ(s1, s2, n)	(strncmp((s1), (s2), n) == 0)
#define STRNCASECMPEQ(s1, s2, n)	(strncasecmp((s1), (s2), n) == 0)

/*
 * PATH_LEN    = max. path length (incl. terminating '\0')
 * NAME_LEN    = max. filename length (not incl. terminating '\0')
 * LEN         =
 * HEADER_LEN  = max. size of a news/mail header-line
 * NEWSRC_LINE =
 */

#ifdef PATH_MAX
#	define PATH_LEN	PATH_MAX
#else
#	ifdef MAXPATHLEN
#		define PATH_LEN	MAXPATHLEN
#	else
#		ifdef _POSIX_PATH_MAX
#			define PATH_LEN	_POSIX_PATH_MAX
#		else
#			define PATH_LEN	255
#		endif /* _POSIX_PATH_MAX */
#	endif /* MAXPATHLEN */
#endif /* PATH_MAX */
#ifdef HAVE_LONG_FILE_NAMES
#	ifdef NAME_MAX
#		define NAME_LEN	NAME_MAX
#	else
#		ifdef _POSIX_NAME_MAX
#			define NAME_LEN	_POSIX_NAME_MAX
#		else
#			define NAME_LEN	14
#		endif /* _POSIX_NAME_MAX */
#	endif /* NAME_MAX */
#else
#	define NAME_LEN	14
#endif /* HAVE_LONG_FILE_NAMES */
#define LEN	1024

#define NEWSRC_LINE	8192
#define HEADER_LEN	1024
#define IMF_LINE_LEN 998 /* RFC 5322 2.1.1 */

#define TABLE_SIZE	1409

#define MODULO_COUNT_NUM	50

#define DAY	(60*60*24)		/* Seconds in a day */

#define ctrl(c)	((c) & 0x1F)

#ifndef DEFAULT_ISO2ASC
#	define DEFAULT_ISO2ASC	"-1 "	/* ISO -> ASCII charset conversion */
#endif /* !DEFAULT_ISO2ASC */

#ifndef DEFAULT_COMMENT
#	define DEFAULT_COMMENT	"> "	/* used when by follow-ups & replies */
#endif /* !DEFAULT_COMMENT */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
#	define T_CHAR_FMT "lc"
#else
#	define T_CHAR_FMT "c"
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#ifndef ART_MARK_UNREAD
#	define ART_MARK_UNREAD	'+'	/* used to show that an art is unread */
#endif /* !ART_MARK_UNREAD */
#ifndef ART_MARK_RETURN
#	define ART_MARK_RETURN	'-'	/* used to show that an art will return */
#endif /* !ART_MARK_RETURN */
#ifndef ART_MARK_SELECTED
#	define ART_MARK_SELECTED	'*'	/* used to show that an art was auto selected */
#endif /* !ART_MARK_SELECTED */
#ifndef ART_MARK_RECENT
#	define ART_MARK_RECENT	'o'	/* used to show that an art is fresh */
#endif /* !ART_MARK_RECENT */
#ifndef ART_MARK_READ
#	define ART_MARK_READ	' '	/* used to show that an art was not read or seen */
#endif /* !ART_MARK_READ */
#ifndef ART_MARK_READ_SELECTED
#	define ART_MARK_READ_SELECTED ':'	/* used to show that a read art is hot (kill_level >0) */
#endif /* !ART_MARK_READ_SELECTED */
#ifndef ART_MARK_KILLED
#	define ART_MARK_KILLED 'K'		/* art has been killed (kill_level >0) */
#endif /* !ART_MARK_KILLED */
#ifndef ART_MARK_DELETED
#	define ART_MARK_DELETED	'D'	/* art has been marked for deletion (mailgroup) */
#endif /* !ART_MARK_DELETED */
#ifndef MARK_INRANGE
#	define MARK_INRANGE	'#'	/* group/art within a range (# command) */
#endif /* !MARK_INRANGE */


/*
 * Unicode chars for thread and attachment tree and cursor (instead of "|+`->")
 * only available when IS_LOCAL_CHARSET("UTF-8")
 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	/*
	 * For CURSOR_ARROW and CURSOR_HORIZ tinrc.utf8_graphics ? :
	 * is handled inside screen.c:draw_arrow_mark().
	 */
#	define CURSOR_ARROW		(wint_t) 0x25B6
#	define CURSOR_HORIZ		(wint_t) 0x2500
#	define TREE_ARROW		(wchar_t) (tinrc.utf8_graphics ? 0x25B6 : '>')
#	define TREE_ARROW_WRAP	(wchar_t) (tinrc.utf8_graphics ? 0x25B7 : '>')
#	define TREE_BLANK		(wchar_t) ' '
#	define TREE_HORIZ		(wchar_t) (tinrc.utf8_graphics ? 0x2500 : '-')
#	define TREE_UP_RIGHT	(wchar_t) (tinrc.utf8_graphics ? 0x2514 : '`')
#	define TREE_VERT		(wchar_t) (tinrc.utf8_graphics ? 0x2502 : '|')
#	define TREE_VERT_RIGHT	(wchar_t) (tinrc.utf8_graphics ? 0x251C : '+')
#else
	/*
	 * No need for CURSOR_ARROW and CURSOR_HORIZ here. This is handled inside
	 * screen.c:draw_arrow_mark().
	 */
#	define TREE_ARROW		'>'
#	define TREE_ARROW_WRAP	'>'
#	define TREE_BLANK		' '
#	define TREE_HORIZ		'-'
#	define TREE_UP_RIGHT	'`'
#	define TREE_VERT		'|'
#	define TREE_VERT_RIGHT	'+'
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

#ifdef USE_INVERSE_HACK
#	define BLANK_PAGE_COLS	2
#else
#	define BLANK_PAGE_COLS	0
#endif /* USE_INVERSE_HACK */

/*
 * Return values for tin_errno
 */
#define TIN_ABORT		1			/* User requested abort or timeout */

#define TINRC_CONFIRM_ACTION	(tinrc.confirm_choice == 1 || tinrc.confirm_choice == 4 || tinrc.confirm_choice == 5 || tinrc.confirm_choice == 7)
#define TINRC_CONFIRM_TO_QUIT	(tinrc.confirm_choice == 3 || tinrc.confirm_choice == 4 || tinrc.confirm_choice == 6 || tinrc.confirm_choice == 7)
#define TINRC_CONFIRM_SELECT	(tinrc.confirm_choice == 2 || tinrc.confirm_choice == 5 || tinrc.confirm_choice == 6 || tinrc.confirm_choice == 7)
#define TINRC_CONFIRM_MAX	7

/*
 * defines for tinrc.auto_cc_bcc
 */
#define AUTO_CC					1
#define AUTO_BCC				2
#define AUTO_CC_BCC				3

/*
 * defines for tinrc.goto_next_unread
 */
#define NUM_GOTO_NEXT_UNREAD	4
#define GOTO_NEXT_UNREAD_PGDN	1
#define GOTO_NEXT_UNREAD_TAB	2

/*
 * defines for tinrc.trim_article_body
 */
#define NUM_TRIM_ARTICLE_BODY	8
#define SKIP_LEADING			1
#define SKIP_TRAILING			2
#define COMPACT_MULTIPLE		4

/*
 * defines for tinrc.show_help_mail_sign
 */
enum {
	SHOW_SIGN_NONE = 0,
	SHOW_SIGN_HELP,
	SHOW_SIGN_MAIL,
	SHOW_SIGN_BOTH
};

/*
 * MIME Encodings
 */
enum {
	MIME_ENCODING_8BIT = 0,
	MIME_ENCODING_BASE64,
	MIME_ENCODING_QP,
	MIME_ENCODING_7BIT
};

/*
 * Number of charset-traslation tables (iso2asci)
 */
#define NUM_ISO_TABLES	7

/*
 * Maximum permissible colour number
 */
#define MAX_COLOR	15
#define MAX_BACKCOLOR	7

/*
 * Count of available attributes for highlighting
 */
#define MAX_ATTR	6

/*
 * Maximal permissible word mark type
 * 0 = nothing, 1 = mark, 2 = space
 */
#define MAX_MARK	2

/* Line number (starting at 0) of 1st non-header data on the screen */
/* ie, size of header */
#define INDEX_TOP	2
#define INDEX2LNUM(i)	(INDEX_TOP + (i) - currmenu->first)
#ifndef USE_CURSES
#	define INDEX2SNUM(i)	((i) - currmenu->first)
#endif /* !USE_CURSES */

#define GROUP_MATCH(s1, pat, case_s)		(wildmat(s1, pat, case_s))

#define REGEX_FMT (tinrc.wildcard ? "%s" : "*%s*")

#define IGNORE_ART(i)	((tinrc.kill_level != KILL_THREAD && arts[i].killed) || (arts[i].thread == ART_EXPIRED))
/* only used for threading */
#define IGNORE_ART_THREAD(i)	(arts[i].thread != ART_UNTHREADED || (tinrc.kill_level == KILL_NOTHREAD && arts[i].killed))

/*
 * Is this part text/plain?
 */
#define IS_PLAINTEXT(x) \
			(x->type == TYPE_TEXT && strcasecmp("plain", x->subtype) == 0)

/* TRUE if basenote has responses */
#define HAS_FOLLOWUPS(i)	(arts[base[i]].thread >= 0)

/*
 * Only close off our stream when reading on local spool
 */
#ifdef NNTP_ABLE
#	define TIN_FCLOSE(x)	if (x != FAKE_NNTP_FP) fclose(x)
#else
#	define TIN_FCLOSE(x)	fclose(x)
#endif /* NNTP_ABLE */

/*
 * Often used macro to point to the group we are currently in
 */
#define CURR_GROUP	(active[my_group[selmenu.curr]])

/*
 * Defines an unread group
 */
#define UNREAD_GROUP(i)		(!active[my_group[i]].bogus && active[my_group[i]].newsrc.num_unread > 0)

/*
 * Expands to singular/plural version of string
 */
#define PLURAL(x,y)			((x == 1) ? _(y##_singular) : _(y##_plural))

/*
 * News/Mail group types
 */
#define GROUP_TYPE_MAIL	0
#define GROUP_TYPE_NEWS	1
#define GROUP_TYPE_SAVE	2	/* saved news, read with tin -R */

/*
 * used by get_arrow_key()
 */
#	define KEYMAP_UNKNOWN		0
#	define KEYMAP_UP		1
#	define KEYMAP_DOWN		2
#	define KEYMAP_LEFT		3
#	define KEYMAP_RIGHT		4
#	define KEYMAP_PAGE_UP		5
#	define KEYMAP_PAGE_DOWN	6
#	define KEYMAP_HOME		7
#	define KEYMAP_END		8
#	define KEYMAP_DEL		9
#	define KEYMAP_INS		10
#	define KEYMAP_MOUSE		11


/*
 * used in main.c, curses.c and signal.c
 * it's useless trying to run tin interactively below these sizes
 * (values acquired by testing ;-) )
 */
#define MIN_LINES_ON_TERMINAL		 8
#define MIN_COLUMNS_ON_TERMINAL		40


/*
 * indicate given cmd-line options
 */
#define CMDLINE_GETART_LIMIT	1
#define CMDLINE_MAILDIR			2
#define CMDLINE_NNTPSERVER		4
#define CMDLINE_SAVEDIR			8
#define CMDLINE_USE_COLOR		16


/*
 * used by feed_articles() & show_mini_help() & quick_filter & add_filter_rule
 */
#define SELECT_LEVEL	1
#define GROUP_LEVEL	2
#define THREAD_LEVEL	3
#define PAGE_LEVEL	4
#define INFO_PAGER	5
#define SCOPE_LEVEL	6
#define CONFIG_LEVEL	7
#define ATTRIB_LEVEL	8
#define ATTACHMENT_LEVEL	9
#define URL_LEVEL	10
#define POSTED_LEVEL	11

#define MINI_HELP_LINES		5

#define FEED_MAIL		1
#define FEED_PIPE		2
#define FEED_PRINT		3
#define FEED_SAVE		4
#define FEED_AUTOSAVE	5
#define FEED_REPOST		6
#define FEED_MARK_READ		7
#define FEED_MARK_UNREAD	8


/*
 * Threading strategies available
 */
#define THREAD_NONE		0
#define THREAD_SUBJ		1
#define THREAD_REFS		2
#define THREAD_BOTH		3
#define THREAD_MULTI		4
#define THREAD_PERC		5

#define THREAD_MAX		THREAD_PERC

#define THREAD_PERC_DEFAULT	75

/*
 * Values for show_author
 */
#define SHOW_FROM_NONE		0
#define SHOW_FROM_ADDR		1
#define SHOW_FROM_NAME		2
#define SHOW_FROM_BOTH		3

/*
 * Values for thread_score
 */
#define THREAD_SCORE_MAX	0
#define THREAD_SCORE_SUM	1
#define THREAD_SCORE_WEIGHT	2

/*
 * Values for interactive_mailer
 */
enum {
	INTERACTIVE_NONE = 0,
	INTERACTIVE_WITH_HEADERS,
	INTERACTIVE_WITHOUT_HEADERS
};

/*
 * used in feed.c & save.c
 */
#define POST_PROC_NO		0
#define POST_PROC_SHAR		1
#define POST_PROC_YES		2

/*
 * used in art.c
 * sort types on arts[] array
 */
#define SORT_ARTICLES_BY_NOTHING		0
#define SORT_ARTICLES_BY_SUBJ_DESCEND	1
#define SORT_ARTICLES_BY_SUBJ_ASCEND	2
#define SORT_ARTICLES_BY_FROM_DESCEND	3
#define SORT_ARTICLES_BY_FROM_ASCEND	4
#define SORT_ARTICLES_BY_DATE_DESCEND	5
#define SORT_ARTICLES_BY_DATE_ASCEND	6
#define SORT_ARTICLES_BY_SCORE_DESCEND	7
#define SORT_ARTICLES_BY_SCORE_ASCEND	8
#define SORT_ARTICLES_BY_LINES_DESCEND	9
#define SORT_ARTICLES_BY_LINES_ASCEND	10

/*
 * used in art.c
 * sort types on base[] array
 */
#define SORT_THREADS_BY_NOTHING			0
#define SORT_THREADS_BY_SCORE_DESCEND	1
#define SORT_THREADS_BY_SCORE_ASCEND	2
#define SORT_THREADS_BY_LAST_POSTING_DATE_DESCEND	3
#define SORT_THREADS_BY_LAST_POSTING_DATE_ASCEND	4

/*
 * Different values of strip_bogus - the ways to handle bogus groups
 */
#define BOGUS_KEEP		0	/* not used */
#define BOGUS_REMOVE		1
#define BOGUS_SHOW		2

/*
 * Different extents to which we can hide killed articles
 */
#define KILL_UNREAD		0		/* Kill only unread articles */
#define KILL_THREAD		1		/* Kill all articles and show as K */
#define KILL_NOTHREAD	2		/* Kill all articles, never show them */

/*
 * Various types of quoting behaviour
 */
#define QUOTE_COMPRESS	1		/* Compress quotes */
#define QUOTE_SIGS		2		/* Quote signatures */
#define QUOTE_EMPTY		4		/* Quote empty lines */


/*
 * used in save.c/main.c
 */
#define CHECK_ANY_NEWS		0
#define START_ANY_NEWS		1
#define MAIL_ANY_NEWS		2
#define SAVE_ANY_NEWS		3


/*
 * used in post.c
 */
#define POSTED_NONE		0			/* Article wasn't posted */
#define POSTED_REDRAW		1			/* redraw needed in any case */
#define POSTED_OK		2			/* posted normally */


/*
 * used in pager
 */
#define UUE_NO			0		/* Don't hide uue data */
#define UUE_YES			1		/* Hide uue data */
#define UUE_ALL			2		/* Hide uue data harder */

/*
 * used in misc.c/rfc1524.c
 */
enum quote_enum {
	no_quote = 0,
	dbl_quote,
	sgl_quote
};


/*
 * Values used in show_article_by_msid
 */
enum {
	LOOKUP_OK           = 0,
	LOOKUP_FAILED       = -1,
	LOOKUP_QUIT         = -2,
	LOOKUP_UNAVAIL      = -3,
	LOOKUP_REPLY        = -4,
	LOOKUP_ART_UNAVAIL  = -5,
	LOOKUP_NO_LAST      = -6
};

/*
 * index_point variable values used throughout tin
 */

/*
 * -1 is kind of overloaded as an error from which_thread() and other functions
 * where we wish to return to the next level up
 */
enum {
	GRP_RETSELECT	= -1,	/* Pager 'T' command only -> return to selection screen */
	GRP_QUIT		= -2,	/* Set by 'Q' when coming all the way out */
	GRP_NEXTUNREAD	= -3,	/* (After catchup) goto next unread item */
	GRP_NEXT		= -4,	/* (After catchup) move to next item */
	GRP_ARTUNAVAIL	= -5,	/* show_page() Article is unavailable */
	GRP_ARTABORT	= -6,	/* show_page() User aborted article read */
	GRP_KILLED		= -7,	/* ?? Thread was killed at pager level */
	GRP_GOTOTHREAD	= -8,	/* show_page() only. Goto thread menu */
	GRP_ENTER		= -9,	/* New group is set, spin in read_groups() */
	GRP_EXIT		= -10	/* Normal return to higher level */
};

#ifndef EXIT_SUCCESS
#	define EXIT_SUCCESS	0	/* Successful exit status */
#endif /* !EXIT_SUCCESS */

#ifndef EXIT_FAILURE
#	define EXIT_FAILURE	1	/* Failing exit status */
#endif /* !EXIT_FAILURE */

#define NEWS_AVAIL_EXIT 2
#define NNTP_ERROR_EXIT	3

/*
 * Assertion verifier
 */
#ifdef assert
#	undef assert
#endif /* assert */
#ifdef NDEBUG
#	define assert(p)        ((void) 0)
#else
#	ifdef CPP_DOES_EXPAND
#		define assert(p)	if(!(p)) asfail(__FILE__, __LINE__, #p); else (void) 0;
#	else
#		define assert(p)	if(!(p)) asfail(__FILE__, __LINE__, "p"); else (void) 0;
#	endif /* CPP_DOES_EXPAND */
#endif /* NDEBUG */

#define ESC	27

/*
 * filter entries expire after DEFAULT_FILTER_DAYS
 */
#define DEFAULT_FILTER_DAYS		28

/*
 * art.thread
 */
#define ART_UNTHREADED	-1
#define ART_EXPIRED		-2

/*
 * Where does this belong?? It is overloaded
 */
#define ART_NORMAL		-1

/*
 * art.status
 */
#define ART_READ		0
#define ART_UNREAD		1
#define ART_WILL_RETURN		2
#define ART_UNAVAILABLE		-1 /* Also used by msgid.article */

/*
 * art.killed
 */
#define ART_NOTKILLED		0
#define ART_KILLED		1
#define ART_KILLED_UNREAD	2

/*
 * Additionally used for user aborts in art_open()
 */
#define ART_ABORT		-2

/*
 * used by t_group & my_group[]
 */
#define UNSUBSCRIBED	'!'
#define SUBSCRIBED	':'

/* Converts subscription status to char for .newsrc */
#define SUB_CHAR(x)	(x ? SUBSCRIBED : UNSUBSCRIBED)
/* Converts .newsrc subscription char to boolean */
#define SUB_BOOL(x)	(x == SUBSCRIBED)

/*
 * filter_type used in struct t_filter
 */
#define SCORE_MAX		10000

#define FILTER_SUBJ_CASE_SENSITIVE		0
#define FILTER_SUBJ_CASE_IGNORE		1
#define FILTER_FROM_CASE_SENSITIVE		2
#define FILTER_FROM_CASE_IGNORE		3
#define FILTER_MSGID		4
#define FILTER_MSGID_LAST	5
#define FILTER_MSGID_ONLY	6
#define FILTER_REFS_ONLY	7
#define FILTER_LINES		8

#define FILTER_LINES_NO		0
#define FILTER_LINES_EQ		1
#define FILTER_LINES_LT		2
#define FILTER_LINES_GT		3

/*
 * default format strings for selection, group, thread level
 * and the date display in the page header.
 * Don't change without adjusting rc_update() and the like accordingly!
 */
#define DEFAULT_SELECT_FORMAT	"%f %n %U  %G  %d"
#define DEFAULT_GROUP_FORMAT	"%n %m %R %L  %s  %F"
#define DEFAULT_THREAD_FORMAT	"%n %m  [%L]  %T  %F"
#define DEFAULT_DATE_FORMAT		"%a, %d %b %Y %H:%M:%S"

/*
 * unicode normalization
 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
#	ifdef HAVE_LIBICUUC
#		define HAVE_UNICODE_NORMALIZATION 3
#	else
#		ifdef HAVE_LIBUNISTRING
#			define HAVE_UNICODE_NORMALIZATION 2
#		else
#			if defined(HAVE_LIBIDN) && defined(HAVE_STRINGPREP_H)
#				define HAVE_UNICODE_NORMALIZATION 1
#			endif /* HAVE_LIBIDN && HAVE_STRINGPREP_H */
#		endif /* HAVE_LIBUNISTRING */
#	endif /* HAVE_LIBICUUC */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

/*
 * normalization forms, we prefer NFC (see RFC 6532) if possible
 */
#ifdef HAVE_UNICODE_NORMALIZATION
enum {
	NORMALIZE_NONE = 0,
	NORMALIZE_NFKC = 1,
#	if (HAVE_UNICODE_NORMALIZATION >= 2)
	NORMALIZE_NFKD = 2,
	NORMALIZE_NFC = 3,
	NORMALIZE_NFD = 4
#		ifdef HAVE_UNICODE_UNORM2_H
	, NORMALIZE_NFKC_CF = 5
#		endif /* HAVE_UNICODE_UNORM2_H */
#	endif /* HAVE_UNICODE_NORMALIZATION >= 2 */
};
#	if (HAVE_UNICODE_NORMALIZATION >= 2)
#		ifdef HAVE_UNICODE_UNORM2_H
#			define NORMALIZE_MAX NORMALIZE_NFKC_CF
#		else
#			define NORMALIZE_MAX NORMALIZE_NFD
#		endif /* HAVE_UNICODE_UNORM2_H */
#		define DEFAULT_NORMALIZE NORMALIZE_NFC
#	else
#		define NORMALIZE_MAX NORMALIZE_NFKC
#		define DEFAULT_NORMALIZE NORMALIZE_NFKC
#	endif /* HAVE_UNICODE_NORMALIZATION >= 2 */
#endif /* HAVE_UNICODE_NORMALIZATION */

#ifdef HAVE_LIBICUUC
#	ifdef HAVE_UNICODE_USTRING_H
#		include <unicode/ustring.h>
#	endif /* HAVE_UNICODE_USTRING_H */
#	ifdef HAVE_UNICODE_UNORM2_H
#		include <unicode/unorm2.h>
#	else
#		ifdef HAVE_UNICODE_UNORM_H
#			include <unicode/unorm.h>
#		endif /* HAVE_UNICODE_UNORM_H */
#	endif /* HAVE_UNICODE_UNORM2_H */
#	ifdef HAVE_UNICODE_UIDNA_H
#		include <unicode/uidna.h>
#	endif /* HAVE_UNICODE_UIDNA_H */
#	ifdef HAVE_UNICODE_UBIDI_H
#		include <unicode/ubidi.h>
#	endif /* HAVE_UNICODE_UBIDI_H */
#endif /* HAVE_LIBICUUC */


/*
 * used in checking article header before posting
 */
#define MAX_COL		78	/* Max. line length before issuing a warning */
#define MAX_SIG_LINES	4	/* Max. num. of signature lines before warning */

typedef unsigned char	t_bitmap;

/*
 * Keys for add_msgid()
 */
#define REF_REF				1		/* Add a ref->ref entry */
#define MSGID_REF			2		/* Add a msgid->ref entry */

/*
 * Size of hash tables for hash_str() and hash_msgid()
 * Make sure it's prime!
 */
#ifdef SMALL_MEMORY_MACHINE
#	define HASHNODE_TABLE_SIZE 2411
#	define MSGID_HASH_SIZE	2609
#else
#	define HASHNODE_TABLE_SIZE 222199
#	define MSGID_HASH_SIZE	222199
#endif /* MSGID_HASH_SIZE */

/*
 * cmd-line options
 */
struct t_cmdlineopts {
	int getart_limit;			/* getart_limit */
	char maildir[PATH_LEN];		/* maildir */
	char nntpserver[PATH_LEN];	/* nntpserver */
	char savedir[PATH_LEN];		/* savedir */
	unsigned int args:5;		/* given options */
};

/*
 * struct t_msgid - message id
 */
struct t_msgid {
	struct t_msgid *next;		/* Next in hash chain */
	struct t_msgid *parent;		/* Message-id followed up to */
	struct t_msgid *sibling;	/* Next followup to parent */
	struct t_msgid *child;		/* First followup to this article */
	int article;			/* index in arts[] or ART_NORMAL */
	char txt[1];			/* The actual msgid */
};

/*
 * struct t_article - article header
 *
 * article.thread:
 * the next article in thread
 *	-1  (ART_UNTHREADED) article exists but is not (yet) threaded
 *	-2  (ART_EXPIRED) article has expired (wasn't found in search of spool
 *	    directory for the group)
 *	>=0 points to another arts[] (struct t_article)
 *
 * article.prev:
 *	the previous article in thread
 *	-1  (ART_NORMAL) initial default, first (no previous) article in thread
 *	>=0 points to the previous arts[] (struct t_article)
 */
struct t_article {
	t_artnum artnum;		/* Article number in spool directory for group */
	char *subject;			/* Subject: line from mail header */
	char *from;			/* From: line from mail header (address) */
	char *name;			/* From: line from mail header (full name) */
	char *xref;			/* Xref: cross posted article reference line */
	char *path;			/* Path: line */
	/* NB: The msgid and refs are only retained until the reference tree is built */
	char *msgid;			/* Message-ID: unique message identifier */
	char *refs;			/* References: article reference id's */
	struct t_msgid *refptr;		/* Pointer to us in the reference tree */
	time_t date;			/* Date: line from header in seconds */
	int line_count;			/* Lines: number of lines in article */
	int gnksa_code;			/* From: line from mail header (GNKSA error code) */
	int tagged;			/* 0 = not tagged, >0 = tagged */
	int thread;
	int prev;
	int score;			/* score article has reached after filtering */
	unsigned int status:2;	/* 0 = read, 1 = unread, 2 = will return */
	unsigned int killed:2;	/* 0 = not killed, 1 = killed, 2 = killed unread */
	t_bool zombie:1;	/* 1 = was alive (unread) before 'X' command */
	t_bool delete_it:1;	/* 1 = delete art when leaving group [mail group] */
	t_bool selected:1;	/* FALSE = not selected, TRUE = selected */
	t_bool inrange:1;	/* TRUE = article selected via # range command */
	t_bool matched:1;	/* TRUE = article matched regex in feed.c */
	t_bool keep_in_base:1;	/* TRUE = keep (read) article in base[] (show_only_unread_arts) */
	t_bool multipart_subj:1;	/* TRUE = subject looks like multipart subject */
};


#ifdef NNTP_ABLE
/*
 * struct t_article_range - holds ranges of article numbers to perform actions on parts of arts[]
 */
struct t_article_range {
	t_artnum start;
	t_artnum end;
	t_artnum cnt;
	struct t_article_range *next;
};
#endif /* NNTP_ABLE */


/*
 * struct t_newsheader - holds an array of which news headers to [not] display
 */
struct t_newsheader {
	char **header;	/* array of which headers to [not] display */
	int num;		/* number of headers in array header */
};

/*
 * Use these macros to quiet gcc warnings when assigning into a bitfield.
 */
#define CAST_MASK(value,bits)	(((1U << (bits)) - 1) & (unsigned) (value))
#define CAST_BOOL(value)	CAST_MASK(value, 1)
#define CAST_BITS(value,bits)	CAST_MASK(value, BITS_OF(bits))
#define BITS_OF(bits)		BITS_OF_ ## bits

#define BoolField(value)	unsigned value:1
#define IntField(value)	unsigned value:BITS_OF(value)

#define BITS_OF_auto_cc_bcc		2
#define BITS_OF_mail_mime_encoding	2
#define BITS_OF_mm_network_charset	6
#define BITS_OF_post_mime_encoding	2
#define BITS_OF_post_process_type	2
#define BITS_OF_quick_kill_header	3
#define BITS_OF_quick_select_header	3
#define BITS_OF_show_author		2
#define BITS_OF_sort_article_type	4
#define BITS_OF_sort_threads_type	3
#define BITS_OF_thread_articles	3
#define BITS_OF_thread_perc		7
#define BITS_OF_trim_article_body	3

/*
 * struct t_attribute - configurable attributes on a per group basis
 */
struct t_attribute {
	char *maildir;				/* mail dir if other than ~/Mail */
	char *savedir;				/* save dir if other than ~/News */
	char *savefile;				/* save articles to specified file */
	char *sigfile;				/* sig file if other than ~/.Sig */
	char *group_format;			/* format string for group level */
	char *thread_format;		/* format string for thread level */
	char *date_format;			/* format string for the date display */
	char *editor_format;		/* editor + parameters  %E +%N %F */
	char *organization;			/* organization name */
	char *fcc;					/* Fcc folder for mail */
	char *followup_to;			/* where posts should be redirected */
	char *quick_kill_scope;		/* quick filter kill scope */
	char *quick_select_scope;	/* quick filter select scope */
	char *mailing_list;			/* mail list email address */
	char *news_headers_to_display;	/* which headers to display */
	char *news_headers_to_not_display;	/* which headers to not display */
	char *x_headers;			/* extra headers for message header */
	char *x_body;				/* boilerplate text for message body */
	char *from;					/* from line */
	char *news_quote_format;	/* another way to begin a posting format */
	char *quote_chars;			/* string to precede quoted text on each line */
	char *mime_types_to_save;	/* MIME content major/minors we want to save */
#ifdef HAVE_ISPELL
	char *ispell;				/* path to ispell and options */
#endif /* HAVE_ISPELL */
#ifdef CHARSET_CONVERSION
	char *undeclared_charset;		/* charset of articles without MIME charset declaration */
	IntField(mm_network_charset);		/* network charset */
#endif /* CHARSET_CONVERSION */
	struct t_newsheader *headers_to_display;	/* array of which headers to display */
	struct t_newsheader *headers_to_not_display;	/* array of which headers to not display */
	BoolField(global);			/* global/group specific */
	IntField(quick_kill_header);	/* quick filter kill header */
	BoolField(quick_kill_expire);	/* quick filter kill limited/unlimited time */
	BoolField(quick_kill_case);		/* quick filter kill case sensitive? */
	IntField(quick_select_header);	/* quick filter select header */
	BoolField(quick_select_expire);	/* quick filter select limited/unlimited time */
	BoolField(quick_select_case);	/* quick filter select case sensitive? */
	BoolField(add_posted_to_filter);	/* add posted articles to filter */
	BoolField(advertising);			/* add User-Agent: -header */
	BoolField(alternative_handling);	/* skip multipart/alternative parts */
	BoolField(ask_for_metamail);	/* ask before using MIME viewer */
	IntField(auto_cc_bcc);			/* add your name to cc/bcc automatically */
	BoolField(auto_list_thread);	/* list thread when entering it using right arrow */
	BoolField(auto_select);			/* 0=show all unread, 1='X' just hot arts */
	BoolField(batch_save);			/* 0=none, 1=save -S/mail -M */
	BoolField(delete_tmp_files);	/* 0=leave, 1=delete */
	BoolField(group_catchup_on_exit);	/* ask if read groups are to be marked read */
	BoolField(mail_8bit_header);	/* allow 8bit chars. in header of mail message */
	IntField(mail_mime_encoding);
	BoolField(mark_ignore_tags);	/* Ignore tags for GROUP_MARK_THREAD_READ/THREAD_MARK_ARTICLE_READ */
	BoolField(mark_saved_read);		/* mark saved article/thread as read */
	BoolField(pos_first_unread);	/* position cursor at first/last unread article */
	BoolField(post_8bit_header);	/* allow 8bit chars. in header when posting to newsgroup */
	IntField(post_mime_encoding);
	BoolField(post_process_view);	/* set TRUE to invoke mailcap viewer app */
#ifndef DISABLE_PRINTING
	BoolField(print_header);		/* print all of mail header or just Subject: & From lines */
#endif /* !DISABLE_PRINTING */
	BoolField(process_only_unread);	/* save/print//mail/pipe unread/all articles */
	BoolField(prompt_followupto);	/* display empty Followup-To header in editor */
	BoolField(show_only_unread_arts);	/* 0=all, 1=only unread */
	BoolField(sigdashes);			/* set TRUE to prepend every signature with dashes */
	BoolField(signature_repost);	/* set TRUE to add signature when reposting articles */
	IntField(thread_articles);			/* 0=unthread, 1=subject, 2=refs, 3=both, 4=multipart, 5=percentage */
	BoolField(thread_catchup_on_exit);	/* catchup thread with left arrow key or not */
	IntField(thread_perc);			/* percentage threading threshold */
	IntField(show_author);			/* 0=none, 1=name, 2=addr, 3=both */
	BoolField(show_signatures);		/* 0=none, 1=show signatures */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	BoolField(suppress_soft_hyphens);	/* set TRUE to remove soft hyphens (U+00AD) from articles */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	IntField(trim_article_body);	/* 0=Don't trim article body, 1=Skip leading blank lines,
						2=Skip trailing blank lines, 3=Skip leading and trailing blank lines,
						4=Compact multiple blank lines between textblocks,
						5=Compact multiple blank lines between textblocks and skip leading blank lines,
						6=Compact multiple blank lines between textblocks and skip trailing blank lines,
						7=Compact multiple blank lines between textblocks and skip leading and trailing
						  blank lines */
	BoolField(verbatim_handling);	/* 0=none, 1=detect verbatim blocks */
#ifdef HAVE_COLOR
	BoolField(extquote_handling);		/* 0=none, 1=detect quoted text from external sources */
#endif /* HAVE_COLOR */
	BoolField(wrap_on_next_unread);	/* Wrap around threads when searching next unread article */
	IntField(sort_article_type);		/* 0=none, 1=subj descend, 2=subj ascend,
						   3=from descend, 4=from ascend,
						   5=date descend, 6=date ascend,
						   7=score descend, 8=score ascend */
	IntField(sort_threads_type);	/* 0=none, 1=score descend, 2=score ascend,
						   3=last posting date descend, 4=last posting date ascend */
	IntField(post_process_type);	/* 0=none, 1=shar, 2=uudecode */
	BoolField(x_comment_to);	/* insert X-Comment-To: in Followup */
	BoolField(tex2iso_conv);	/* Convert TeX2ISO */
	BoolField(mime_forward);	/* forward articles as attachment or inline */
};

/*
 * struct t_attribute_state - holds additional information
 * about numeric attributes within a scope
 */
struct t_attribute_state {
	BoolField(add_posted_to_filter);
	BoolField(advertising);
	BoolField(alternative_handling);
	BoolField(ask_for_metamail);
	BoolField(auto_cc_bcc);
	BoolField(auto_list_thread);
	BoolField(auto_select);
	BoolField(batch_save);
	BoolField(date_format);
	BoolField(delete_tmp_files);
	BoolField(editor_format);
	BoolField(fcc);
	BoolField(followup_to);
	BoolField(from);
	BoolField(group_catchup_on_exit);
	BoolField(group_format);
#ifdef HAVE_ISPELL
	BoolField(ispell);
#endif /* HAVE_ISPELL */
	BoolField(mail_8bit_header);
	BoolField(mail_mime_encoding);
	BoolField(maildir);
	BoolField(mailing_list);
	BoolField(mark_ignore_tags);
	BoolField(mark_saved_read);
	BoolField(mime_forward);
	BoolField(mime_types_to_save);
	BoolField(news_headers_to_display);
	BoolField(news_headers_to_not_display);
	BoolField(news_quote_format);
	BoolField(organization);
	BoolField(pos_first_unread);
	BoolField(post_8bit_header);
	BoolField(post_mime_encoding);
	BoolField(post_process_view);
	BoolField(post_process_type);
#ifndef DISABLE_PRINTING
	BoolField(print_header);
#endif /* !DISABLE_PRINTING */
	BoolField(process_only_unread);
	BoolField(prompt_followupto);
	BoolField(quick_kill_case);
	BoolField(quick_kill_expire);
	BoolField(quick_kill_header);
	BoolField(quick_kill_scope);
	BoolField(quick_select_case);
	BoolField(quick_select_expire);
	BoolField(quick_select_header);
	BoolField(quick_select_scope);
	BoolField(quote_chars);
	BoolField(savedir);
	BoolField(savefile);
	BoolField(show_author);
	BoolField(show_only_unread_arts);
	BoolField(show_signatures);
	BoolField(sigdashes);
	BoolField(sigfile);
	BoolField(signature_repost);
	BoolField(sort_article_type);
	BoolField(sort_threads_type);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	BoolField(suppress_soft_hyphens);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	BoolField(tex2iso_conv);
	BoolField(thread_articles);
	BoolField(thread_catchup_on_exit);
	BoolField(thread_format);
	BoolField(thread_perc);
	BoolField(trim_article_body);
#ifdef CHARSET_CONVERSION
	BoolField(undeclared_charset);
	BoolField(mm_network_charset);
#endif /* CHARSET_CONVERSION */
	BoolField(verbatim_handling);
#ifdef HAVE_COLOR
	BoolField(extquote_handling);
#endif /* HAVE_COLOR */
	BoolField(wrap_on_next_unread);
	BoolField(x_body);
	BoolField(x_comment_to);
	BoolField(x_headers);
};

/*
 * struct t_scope
 */
struct t_scope {
	char *scope;				/* scope for these group attributes */
	struct t_attribute *attribute;	/* the attributes itself */
	struct t_attribute_state *state;	/* additional information about numeric attributes */
	BoolField(global);			/* TRUE for scopes from global_attributes_file */
};

/*
 * struct t_newsrc - newsrc related info.
 */
struct t_newsrc {
	t_bool present:1;		/* update newsrc? */
	t_artnum num_unread;		/* unread articles in group */
	t_artnum xmax;			/* newsrc max */
	t_artnum xmin;			/* newsrc min */
	t_artnum xbitlen;			/* bitmap length (max-min+1) */
	t_bitmap *xbitmap;	/* bitmap read/unread (max-min+1+7)/8 */
};

/*
 * struct t_group - newsgroup info from active file
 */
struct t_group {
	char *name;			/* newsgroup/mailbox name */
	char *aliasedto;		/* =new.group in active file, NULL if not */
	char *description;	/* text from NEWSLIBDIR/newsgroups file */
	char *spooldir;		/* groups spool directory */
	char moderated;		/* state of group moderation */
	t_artnum count;		/* article number count */
	t_artnum xmax;		/* max. article number */
	t_artnum xmin;		/* min. article number */
	unsigned int type:4;		/* grouptype - newsgroup/mailbox/savebox */
	t_bool inrange:1;		/* TRUE if group selected via # range command */
	t_bool read_during_session:1;	/* TRUE if group entered during session */
	t_bool art_was_posted:1;	/* TRUE if art was posted to group */
	t_bool subscribed:1;		/* TRUE if subscribed to group */
	t_bool newgroup:1;		/* TRUE if group was new this session */
	t_bool bogus:1;			/* TRUE if group is not in active list */
	int next;			/* next active entry in hash chain */
	struct t_newsrc newsrc;		/* newsrc bitmap specific info. */
	struct t_attribute *attribute;	/* group specific attributes */
	struct t_filters *glob_filter;	/* points to filter array */
};

/*
 * used in hashstr.c
 */
struct t_hashnode {
	struct t_hashnode *next;	/* chain for spillover */
	int aptr;			/* used in subject threading */
	char txt[1];			/* stub for the string data, \0 terminated */
};

/*
 * used for variable screen layout
 *
 * holds a preparsed format string, a date format string and some
 * precalculated length
 */
struct t_fmt {
	char str[LEN];
	char date_str[LEN];
	size_t len_date;		/* %D Date */
	size_t len_date_max;
	size_t len_grpdesc;		/* %d newsgroup description */
	size_t len_from;		/* %F From */
	size_t len_grpname;		/* %G groupname */
	size_t len_grpname_dsc;
	size_t len_grpname_max;
	size_t len_initials;	/* %I initials */
	size_t len_linenumber;	/* %n linenumber on screen */
	size_t len_linecnt;		/* %L line count (article) */
	size_t len_msgid;		/* %M message-id */
	size_t len_respcnt;		/* %R count, number of responses */
	size_t len_score;		/* %S score */
	size_t len_subj;		/* %s subject */
	size_t len_ucnt;		/* %U unread count */
	size_t flags_offset;
	size_t mark_offset;
	size_t ucnt_offset;
	t_bool show_grpdesc;
	t_bool d_before_f;
	t_bool g_before_f;
	t_bool d_before_u;
	t_bool g_before_u;
};

/*
 * used in filter.c
 *
 * Create 2 filter arrays - global & local. Local will be part of group_t
 * structure and will have priority over global filter. Should help to
 * speed kill/selecting within a group. The long value number that is in
 * ~/.tin/kill will be replaced by group name so that it is more human
 * readable and that if hash routine is changed it will still work.
 *
 * Add time period to filter_t struct to allow timed kills & auto-selection
 * Default kill & select time 28 days. Store as a long and compare when
 * loading against present time. If time secs is passed set flag to save
 * filter file and don't load expired entry. Renamed to filter because of
 * future directions in adding other retrieval methods to present kill &
 * auto selection.
 *
 * Also separate kill/select screen to allow ^K=kill ^A=auto-select
 */
struct t_filters {
	int max;
	int num;
	struct t_filter *filter;
};

/*
 * struct t_filter_comment: allow multiple comment-lines in filter-file.
 */
struct t_filter_comment {
	char *text;			/* One line of comment. */
	struct t_filter_comment *next;	/* points to next comment-entry */
};

/*
 * struct t_filter - local & global filtering (ie. kill & auto-selection)
 */
struct t_filter {
	struct t_filter_comment *comment;
	char *scope;			/* NULL='*' (all groups) or 'comp.os.*' */
	char *subj;			/* Subject: line */
	char *from;			/* From: line */
	char *msgid;			/* Message-ID: line */
	char *xref;			/* groups in xref line */
	char *path;			/* server in path line */
	struct t_filter *next;		/* next rule valid in group */
	time_t time;			/* expire time in seconds */
	int lines_num;			/* Lines: line */
	int gnksa_num;			/* GNKSA code */
	int score;			/* score to give if rule matches */
	char lines_cmp;			/* Lines compare <> */
	char gnksa_cmp;			/* GNKSA compare <> */
	unsigned int inscope:4;		/* if group matches scope e.g. 'comp.os.*' */
	unsigned int icase:2;		/* Case sensitive filtering */
	unsigned int fullref:4;		/* use full references or last entry only */
};

/*
 * struct t_filter_rule - provides parameters to build filter rule from
 */
struct t_filter_rule {
	struct t_filter_comment *comment;
	char text[PATH_LEN];
	char scope[PATH_LEN];
	int counter;
	int icase;
	int fullref;
	int lines_cmp;
	int lines_num;
	int score;
	int expire_time;
	t_bool from_ok:1;
	t_bool lines_ok:1;
	t_bool msgid_ok:1;
	t_bool subj_ok:1;
	t_bool check_string:1;
};

/*
 * Filter cache structure using Philip Hazel's Perl regular expression
 * library (see pcre/pcre.[ch] for details)
 */
#ifdef HAVE_LIB_PCRE2

struct regex_cache {
	pcre2_code_8 *re;
	pcre2_match_data_8 *match;
};

#define REGEX_CACHE_INITIALIZER { NULL, NULL }

#else /* HAVE_LIB_PCRE2 */

struct regex_cache {
	pcre *re;
	pcre_extra *extra;
	int *ovector;
	int ovecalloc;	/* number of allocated integers */
	int ovecmax;	/* max valid pairs in ovector, i.e. max capturecount */
	int oveccount;	/* valid capture count from last regex_exec */
};

#define REGEX_CACHE_INITIALIZER { NULL, NULL, NULL, 0, 0, 0 }

#endif /* HAVE_LIB_PCRE2 */

struct t_save {
	char *path;
	char *file;					/* ptr to file part of *path */
	t_bool mailbox:1;			/* Set if path is a mailbox */
};

#ifndef USE_CURSES
struct t_screen {
	char *col;
};
#endif /* !USE_CURSES */

typedef struct posted {
	char date[10];
	char group[80];
	char action;
	char subj[120];
	char mid[256];
	struct posted *next;
} t_posted;

struct t_art_stat {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t art_mark;		/* mark to use for this thread - not used for groups */
#else
	char art_mark;		/* mark to use for this thread - not used for groups */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	int total;		/* total article count */
	int unread;		/* number of unread articles (does not include seen) arts */
	int seen;		/* number of seen articles (ART_WILL_RETURN) */
	int deleted;		/* number of articles marked for deletion (mailgroups) */
	int inrange;		/* number of articles marked in a range */
	int selected_total;	/* total selected count */
	int selected_unread;	/* selected and unread */
	int selected_seen;	/* selected and seen */
	int killed;		/* killed */
	int score;		/* maximum score */
	int multipart_total; /* 0=not multipart, >0 = number of articles in the multipart */
	int multipart_have; /* number of articles we actually have found */
	int multipart_compare_len; /* length of subject which contains non-specific multipart info */
	time_t time;		/* latest time */
};


/*
 * Used for detecting changes in active file size on different news servers
 */
struct t_newnews {
	char *host;
	time_t time;
};

typedef const char constext;

/*
 * Defines text strings used by a tinrc variable
 */
struct opttxt {
	constext *help;					/* Helptext on Option Menu */
	constext *opt;					/* Text on body of Option Menu screen */
	constext *tinrc;				/* (optional) Text written with variable to tinrc file */
};

/*
 * Used for building option menu
 */
struct t_option {
	int var_type;		/* type of variable (see tincfg.h) */
	int var_index;		/* index in corresponding table */
	int *variable;		/* ptr to variable to change */
	constext **opt_list;	/* ptr to list entries if OPT_LIST */
	struct opttxt *txt;	/* ptr to information/help on option */
};

/*
 * Multipart article detection
 */
typedef struct {
	char *subject;
	int subject_compare_len;
	int part_number;
	int total;
	int arts_index;
} MultiPartInfo;


/*
 * Key information about current menu screen
 */
typedef struct {
	int curr;					/* Current cursor pos (cur_groupnum, index_point, thread_index_point) */
	int max;					/* Max # on current menu (group_top, top_base, top_thread) */
	int first;					/* First # on current menu */
	void (*redraw) (void);		/* Redraw function */
	void (*draw_arrow) (void);	/* Arrow draw */
	void (*draw_item) (int item);	/* draw the specified item */
} t_menu;


/*
 * Packet of data needed to enter pager
 */
typedef struct {
	int art;
	t_bool ignore_unavail:1;
} t_pagerinfo;


/*
 * Time functions.
 */
typedef struct _TIMEINFO {
	time_t time;
	long usec;
	long tzone;
} TIMEINFO;


struct t_tintime {
	time_t tv_sec; /* seconds */
	long tv_nsec; /* nanoseconds */
};


/*
 * mailcap fields
 * the x-token field is missing, we would need something like
 * struct t_xtoken { char *xtoken; t_xtoken *next; } for that ...
 */
typedef struct {
	char *type;		/* content-type, mandatory */
	char *command;	/* view-command, mandatory */
	char *compose;
	char *composetyped;
	char *description;
	char *edit;
	char *nametemplate;
	char *print;
	char *test;
	char *x11bitmap;
	int textualnewlines;
	t_bool needsterminal:1;
	t_bool copiousoutput:1;
} t_mailcap;


typedef struct urllist {
	char *url;
	struct urllist *next;
} t_url;


/*
 * Determine signal return type
 */
#ifndef RETSIGTYPE
#	define RETSIGTYPE void
#endif /* !RETSIGTYPE */

/*
 * Determine qsort compare type
 */
#ifdef HAVE_COMPTYPE_VOID
#	ifdef __STDC__
		typedef const void *t_comptype;
#	else
		typedef void *t_comptype;
#	endif /* __STDC__ */
#else
#	ifdef HAVE_COMPTYPE_CHAR
		typedef char *t_comptype;
#	endif /* HAVE_COMPTYPE_CHAR */
#endif /* HAVE_COMPTYPE_VOID */

/* Define a matching function pointer type */
typedef int (*t_compfunc)(t_comptype, t_comptype);
typedef void (*t_sortfunc)(void *, size_t, size_t, t_compfunc);

#define _CDECL

/* set to (void)heapsort or qsort */
#ifndef USE_HEAPSORT
#	define tin_sort qsort
#else
#	define MAX_SORT_FUNCS 1
#endif /* !USE_HEAPSORT */

/* Separator between dir part of path & the filename */
#define DIRSEP	'/'

/*
 * mouse buttons for use in xterm
 */
#define MOUSE_BUTTON_1		0
#define MOUSE_BUTTON_2		1
#define MOUSE_BUTTON_3		2


#define REDIRECT_OUTPUT		"> /dev/null 2>&1"
#define REDIRECT_PGP_OUTPUT		"> /dev/null"
#define ENV_VAR_MAILER		"MAILER"
#define ENV_VAR_SHELL		"SHELL"
#define TIN_EDITOR_FMT		"%E +%N %F"
#define MAILER_FORMAT		"%M -oi -t < %F"
#ifdef HAVE_KEY_PREFIX
#	define KEY_PREFIX		0x8f: case 0x9b
#endif /* HAVE_KEY_PREFIX */

#if !defined(S_ISDIR)
#	define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif /* !S_ISDIR */

#if !defined(S_ISREG)
#	define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#endif /* !S_ISREG */

#ifndef S_IRWXU /* should be defined in <sys/stat.h> */
#	define S_IRWXU	0000700	/* read, write, execute permission (owner) */
#	define S_IRUSR	0000400	/* read permission (owner) */
#	define S_IWUSR	0000200	/* write permission (owner) */
#	define S_IXUSR	0000100	/* execute permission (owner) */

#	define S_IRWXG	0000070	/* read, write, execute permission (group) */
#	define S_IRGRP	0000040	/* read permission (group) */
#	define S_IWGRP	0000020	/* write permission (group) */
#	define S_IXGRP	0000010	/* execute permission (group) */

#	define S_IRWXO	0000007	/* read, write, execute permission (other) */
#	define S_IROTH	0000004	/* read permission (other) */
#	define S_IWOTH	0000002	/* write permission (other) */
#	define S_IXOTH	0000001	/* execute permission (other) */
#endif /* !S_IRWXU */

/* the next two are needed for fcc 1.0 on linux */
#ifndef S_IFMT
#	define S_IFMT	0xF000	/* type of file */
#endif /* S_IFMT */
#ifndef S_IFREG
#	define S_IFREG	0x8000	/* regular */
#endif /* S_IFREG */

#ifndef S_IRWXUGO
#	define S_IRWXUGO	(S_IRWXU|S_IRWXG|S_IRWXO)	/* read, write, execute permission (all) */
#	define S_IRUGO	(S_IRUSR|S_IRGRP|S_IROTH)	/* read permission (all) */
#	define S_IWUGO	(S_IWUSR|S_IWGRP|S_IWOTH)	/* write permission (all) */
#	define S_IXUGO	(S_IXUSR|S_IXGRP|S_IXOTH)	/* execute permission (all) */
#endif /* !S_IRWXUGO */

#ifndef S_ISVTX
#	define S_ISVTX 0
#endif /* !S_ISVTX */

#ifdef DONT_HAVE_PIPING
#	define TIN_PRINTFILE "tinprint%d.tmp"
#endif /* DONT_HAVE_PIPING */

/*
 * Defines for access()
 */
#ifndef R_OK
#	define R_OK	4	/* Test for Read permission */
#endif /* !R_OK */
#ifndef W_OK
#	define W_OK	2	/* Test for Write permission */
#endif /* !W_OK */
#ifndef X_OK
#	define X_OK	1	/* Test for eXecute permission */
#endif /* !X_OK */
#ifndef F_OK
#	define F_OK	0	/* Test for existence of File */
#endif /* !F_OK */

/* Various function redefinitions */
#if defined(USE_DBMALLOC) || defined(USE_DMALLOC)
#	define my_malloc(size)	malloc((size_t) (size))
#	define my_calloc(nmemb, size)	calloc((nmemb), (size_t) (size))
#	define my_realloc(ptr, size)	realloc((ptr), (size_t) (size))
#else
#	define my_malloc(size)	my_malloc1(__FILE__, __LINE__, (size_t) (size))
#	define my_calloc(nmemb, size)	my_calloc1(__FILE__, __LINE__, (nmemb), (size_t) (size))
#	define my_realloc(ptr, size)	my_realloc1(__FILE__, __LINE__, (ptr), (size_t) (size))
#endif /* USE_DBMALLOC || USE_DMALLOC */

#define ARRAY_SIZE(array)	((int) (sizeof(array) / sizeof(array[0])))

#define FreeIfNeeded(p)	if (p != NULL) free((void *) p)
#define FreeAndNull(p)	if (p != NULL) { free((void *) p); p = NULL; }

#define BlankIfNull(p)	((p) ? (p) : "")

#define my_group_find(x)	add_my_group(x, FALSE, FALSE)
#define my_group_add(x, y)	add_my_group(x, TRUE, y)
#define for_each_group(x)	for (x = 0; x < num_active; x++)
#define for_each_art(x)		for (x = 0; x < top_art; x++)
#define for_each_art_in_thread(x, y)	for (x = (int) base[y]; x >= 0; x = arts[x].thread)

/*
 * Cast for the (few!) places where we need to examine 8-bit characters w/o
 * sign-extension, and a corresponding test-macro.
 */
#define EIGHT_BIT(ptr)	(unsigned char *)ptr
#define is_EIGHT_BIT(p)	((*EIGHT_BIT(p) < 32 && !isspace((int)*p)) || *EIGHT_BIT(p) > 127)

/*
 * function prototypes & extern definitions
 */

#ifndef SIG_ARGS
#	if defined(__STDC__)
#		define SIG_ARGS	int sig
#	endif /* __STDC__ */
#endif /* !SIG_ARGS */

#ifndef __LCLINT__ /* lclint doesn't like it */
/* stifle complaints about not-a-prototype from gcc */
#	ifdef DECL_SIG_CONST
#		undef	SIG_DFL
#		define SIG_DFL	(void (*)(SIG_ARGS))0
#		undef	SIG_IGN
#		define SIG_IGN	(void (*)(SIG_ARGS))1
#		undef	SIG_ERR
#		define SIG_ERR	(void (*)(SIG_ARGS))-1
#	endif /* DECL_SIG_CONST */
#endif /* !__LCLINT__ */

/*
 * tputs() function-param
 */
#ifdef OUTC_RETURN
#	define OUTC_RETTYPE	int
#else
#	define OUTC_RETTYPE	void
#endif /* OUTC_RETURN */

#ifndef OUTC_ARGS
#	define OUTC_ARGS	int c
#endif /* !OUTC_ARGS */

#if __STDC__ || defined(__cplusplus)
#	define OUTC_FUNCTION(func)	OUTC_RETTYPE func (OUTC_ARGS)
#else
#	define OUTC_FUNCTION(func)	OUTC_RETTYPE func (c) int c;
#endif /* __STDC__ || __cplusplus */

typedef OUTC_RETTYPE (*OutcPtr) (OUTC_ARGS);
typedef FILE TCP;

#ifndef EXTERN_H
#	include "extern.h"
#endif /* !EXTERN_H */
#ifndef TINRC_H
#	include "tinrc.h"
#endif /* !TINRC_H */
#ifndef NNTPLIB_H
#	include "nntplib.h"
#endif /* !NNTPLIB_H */

/*
 * RFC 1521 / RFC 1522 interface
 */
typedef void (*BodyPtr) (char *, FILE *, int);

#ifdef USE_DBMALLOC
#	undef strchr
#	undef strrchr
#	include <dbmalloc.h> /* dbmalloc 1.4 */
#endif /* USE_DBMALLOC */

#ifdef USE_DMALLOC
#	include <dmalloc.h>
#	define DMALLOC_FUNC_CHECK
#	ifdef HAVE_STRDUP
#		define my_strdup(s) strdup((s))
#	endif /* HAVE_STRDUP */
#endif /* USE_DMALLOC */

#ifdef DOALLOC
	extern char *doalloc(char *, size_t);
	extern char *docalloc(size_t, size_t);
	extern void	dofree(char *);
#	undef malloc
#	undef realloc
#	undef calloc
#	undef free
#	define malloc(n)	doalloc((char *) 0, n)
#	define realloc		doalloc
#	define calloc		docalloc
#	define free		dofree
	extern void	fail_alloc(char *, char *);
	extern void	Trace(char *, ...);
	extern void	Elapsed(char *);
	extern void	WalkBack(void);
	extern void	show_alloc(void);
	extern void	no_leaks(void);
#endif /* DOALLOC */

/* define some standard places to look for a tin.defaults file */
#define TIN_DEFAULTS_BUILTIN "/etc/opt/tin","/etc/tin","/etc","/usr/local/lib/tin","/usr/local/lib","/usr/local/etc/tin","/usr/local/etc","/usr/lib/tin","/usr/lib",NULL
#ifdef TIN_DEFAULTS_DIR
#	define TIN_DEFAULTS TIN_DEFAULTS_DIR,TIN_DEFAULTS_BUILTIN
#else
#	define TIN_DEFAULTS TIN_DEFAULTS_BUILTIN
#endif /* TIN_DEFAULTS_DIR */

/*
 * We force this include-ordering since socks.h contains redefinitions of
 * functions that probably are prototyped via other includes. The socks.h
 * definitions have to be included everywhere, since they're making wrappers
 * for the stdio functions as well as the network functions.
 */
#ifdef USE_SOCKS5
#	define SOCKS
#	include <socks.h>
/* socks.h doesn't define prototypes for use */
extern size_t read(int, char *, size_t);
extern int dup(int);
extern int close(int);
extern int fprintf(FILE *, const char *, ...);
extern int fclose(FILE *);
extern struct tm *localtime(time_t *);
#endif /* USE_SOCKS5 */

#ifdef SETVBUF_REVERSED
#	define SETVBUF(stream, buf, mode, size)	setvbuf(stream, mode, buf, size)
#else
#	define SETVBUF(stream, buf, mode, size)	setvbuf(stream, buf, mode, size)
#endif /* SETVBUF_REVERSED */

#ifdef CLOSEDIR_VOID
#	define CLOSEDIR(DIR)	closedir(DIR)
#else
#	define CLOSEDIR(DIR)	if (closedir(DIR)) error_message(2, "closedir() failed: %s %d", __FILE__, __LINE__)
#endif /* CLOSEDIR_VOID */

#ifdef HAVE_GETTIMEOFDAY
#	ifndef GETTIMEOFDAY_2ARGS
#		define gettimeofday(a,b) gettimeofday(a)
#	endif /* GETTIMEOFDAY_2ARGS */
#endif /* HAVE_GETTIMEOFDAY */

/* libmss */
#ifdef MSS
#	ifdef strdup
#		undef strdup
#	endif /* strdup */
#	include <mss.h>
#	undef my_malloc
#	undef my_realloc
#	undef my_calloc
#	define my_malloc(size)	malloc((size_t)(size))
#	define my_realloc(ptr, size)	realloc((ptr), (size_t) (size))
#	define my_calloc(nmemb, size) calloc((nmemb), (size_t) (size))
#endif /* MSS */

/* libcanlock */
#ifdef USE_CANLOCK
#	ifdef HAVE_LIBCANLOCK_3_CANLOCK_H
#		include <libcanlock-3/canlock.h>
#	else
#		include <canlock.h>
#	endif /* HAVE_LIBCANLOCK_3_CANLOCK_H */
#endif /* USE_CANLOCK */

/* gsasl */
#ifdef USE_SASL
#	include <gsasl.h>
#endif /* USE_SASL */

/*
 * adapted from ncurses curses.priv.h:
 * If we have va_copy(), use it for assigning va_list's.
 */
#if defined(HAVE___VA_COPY)
#	define begin_va_copy(dst,src)	__va_copy(dst, src)
#	define end_va_copy(dst)	va_end(dst)
#else
#	if defined(va_copy) || defined(HAVE_VA_COPY)
#		define begin_va_copy(dst,src)	va_copy(dst, src)
#		define end_va_copy(dst)	va_end(dst)
#	else
#		if defined(HAVE___BUILTIN_VA_COPY)
#			define begin_va_copy(dst,src)	__builtin_va_copy(dst, src)
#			define end_va_copy(dst)	va_end(dst)
#		else
#			if defined(ARRAY_VA_LIST)
#				define begin_va_copy(dst,src)	*(dst) = *(src)
#				define end_va_copy(dst)	/* nothing */
#			else
#				define begin_va_copy(dst,src)	(dst) = (src)
#				define end_va_copy(dst)	/* nothing */
#			endif /* ARRAY_VA_LIST */
#		endif /* HAVE___BUILTIN_VA_COPY */
#	endif /* va_copy || HAVE_VA_COPY */
#endif /* HAVE___VA_COPY */

/* snprintf(), vsnprintf() */
#ifndef HAVE_SNPRINTF
#	define snprintf	plp_snprintf
#endif /* HAVE_SNPRINTF */
#ifndef HAVE_VSNPRINTF
#	define vsnprintf	plp_vsnprintf
#endif /* HAVE_VSNPRINTF */

#ifdef HAVE_MEMMOVE
#	define my_memmove memmove
#else
#	ifdef HAVE_BCOPY
#		define my_memmove(d,s,n) bcopy(s,d,n)
	/* else memory.c:my_memmove() */
#	endif /* HAVE_BCOPY */
#endif /* HAVE_MEMMOVE */

/* gcc-specific attributes */
#if defined(__GNUC__) && !defined(__cplusplus) && !defined(__APPLE_CC__) && !defined(__NeXT__)
#	if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 6)
#		define UNUSED(x) x __attribute__((unused))
#	else
#		define UNUSED(x) x
#	endif /* __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 6) */
#else
#	define UNUSED(x) x
#endif /* __GNUC__ && !__cplusplus && !__APPLE_CC__ && !__NeXT__ */

/* #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L */
#ifdef STDC_NORETURN
#	ifdef HAVE_STDNORETURN_H
#		include <stdnoreturn.h>
#	endif /* HAVE_STDNORETURN_H */
#else
#	undef _Noreturn
	/*
	 * gcc says "The attribute noreturn is not implemented in GCC
	 * versions earlier than 2.5" but gcc 2.7.2.3 on m68k-sony-newsos
	 * or mips-dec-osf1 still doesn't know __attribute__((noreturn)).
	 * __GNUC__ > 2 should be fine
	 */
#	if defined(__GNUC__) && !defined(__cplusplus) && !defined(__APPLE_CC__) && !defined(__NeXT__)
#		if __GNUC__ > 2
#			define _Noreturn __attribute__((noreturn))
#		else
#			define _Noreturn /**/
#		endif /* __GNUC__ > 2 */
#	else
#		define _Noreturn /**/
#	endif /* __GNUC__ && !__cplusplus && !__APPLE_CC__ && !__NeXT__ */
#endif /* STDC_NORETURN */

#ifndef __CPROTO__
#	ifndef PROTO_H
#		include "proto.h"
#	endif /* !PROTO_H */
#endif /* !__CPROTO__ */

/* init_selfinfo() needs MM_CHARSET */
#ifndef MM_CHARSET
#	define MM_CHARSET "US-ASCII"
#endif /* !MM_CHARSET */


#if !defined(SEEK_SET)
#	define SEEK_SET 0L
#endif /* !SEEK_SET */

#if !defined(EOF)
#	define EOF -1
#endif /* !EOF */

/* various filenames used by tin */
#define TIN_ARTICLE_NAME	".article"
#define TIN_CANCEL_NAME	".cancel"
#define TIN_LETTER_NAME	".letter"
#define TIN_BUGREPORT_NAME	".bugreport"

/* read_news_active_file() / open_newsgroups_fp() */
#ifndef DISABLE_PIPELINING
#	define PIPELINE_LIMIT 45
#else
#	define PIPELINE_LIMIT 1
#endif /* DISABLE_PIPELINING */

#ifndef DEBUG_H
#	include "debug.h"
#endif /* !DEBUG_H */

struct t_overview_fmt {
	char *name;
	enum f_type type;
};

struct t_version {
	enum rc_state state;
	int file_version;		/* rc_majorv * 10000 + rc_minorv * 100 + rc_subv */
/*	int current_version;*/	/* c_majorv * 10000 + c_minorv * 100 + c_subv */
};

#endif /* !TIN_H */
