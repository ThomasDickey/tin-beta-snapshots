dnl Project   : tin - a Usenet reader
dnl Module    : aclocal.m4
dnl Author    : Thomas E. Dickey <dickey@invisible-island.net>
dnl Created   : 1995-08-24
dnl Updated   : 2021-01-03
dnl Notes     :
dnl
dnl Copyright (c) 1995-2021 Thomas E. Dickey <dickey@invisible-island.net>
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl 3. The name of the author may not be used to endorse or promote
dnl    products derived from this software without specific prior written
dnl    permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
dnl OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
dnl DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
dnl DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
dnl GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
dnl WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
dnl NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
dnl SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl
dnl
dnl Macros used in TIN auto-configuration script.
dnl https://invisible-island.net/autoconf/
dnl
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl AC_ISC_POSIX version: 2 updated: 2002/04/17 21:09:56
dnl ------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl isc-posix.m4
dnl ====================
#serial 1
# This test replaces the one in autoconf.
# Currently this macro should have the same name as the autoconf macro
# because gettext's gettext.m4 (distributed in the automake package)
# still uses it.  Otherwise, the use in gettext.m4 makes autoheader
# give these diagnostics:
#   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
#   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])

AC_DEFUN([AC_ISC_POSIX],
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)
dnl ---------------------------------------------------------------------------
dnl AM_GNU_GETTEXT version: 15 updated: 2021/01/02 09:31:20
dnl --------------
dnl Usage: Just like AM_WITH_NLS, which see.
AC_DEFUN([AM_GNU_GETTEXT],
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_CANONICAL_HOST])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl
   AC_REQUIRE([jm_GLIBC21])dnl
   AC_REQUIRE([CF_PROG_CC])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h stddef.h \
stdlib.h string.h unistd.h sys/param.h])
   AC_CHECK_FUNCS([feof_unlocked fgets_unlocked getcwd getegid geteuid \
getgid getuid mempcpy munmap putenv setenv setlocale stpcpy strchr strcasecmp \
strdup strtoul tsearch __argz_count __argz_stringify __argz_next])

   AM_ICONV
   AM_LANGINFO_CODESET
   AM_LC_MESSAGES
   AM_WITH_NLS([$1],[$2],[$3],[$4])

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for presentlang in $ALL_LINGUAS; do
         useit=no
         for desiredlang in ${LINGUAS-$ALL_LINGUAS}; do
           # Use the presentlang catalog if desiredlang is
           #   a. equal to presentlang, or
           #   b. a variant of presentlang (because in this case,
           #      presentlang can be used as a fallback for messages
           #      which are not translated in the desiredlang catalog).
           case "$desiredlang" in
             "$presentlang"*) useit=yes;;
           esac
         done
         if test "$useit" = yes; then
           NEW_LINGUAS="$NEW_LINGUAS $presentlang"
         fi
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl Enable libtool support if the surrounding package wishes it.
   INTL_LIBTOOL_SUFFIX_PREFIX=ifelse([$1], use-libtool, [l], [])
   AC_SUBST(INTL_LIBTOOL_SUFFIX_PREFIX)
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_ICONV version: 12 updated: 2007/07/30 19:12:03
dnl --------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl iconv.m4
dnl ====================
dnl serial AM2
dnl
dnl From Bruno Haible.
dnl
dnl ====================
dnl Modified to use CF_FIND_LINKAGE and CF_ADD_SEARCHPATH, to broaden the
dnl range of locations searched.  Retain the same cache-variable naming to
dnl allow reuse with the other gettext macros -Thomas E Dickey
AC_DEFUN([AM_ICONV],
[
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([libiconv-prefix],
[  --with-libiconv-prefix=DIR
                          search for libiconv in DIR/include and DIR/lib], [
    CF_ADD_OPTIONAL_PATH($withval, libiconv)
   ])

  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    CF_FIND_LINKAGE(CF__ICONV_HEAD,
      CF__ICONV_BODY,
      iconv,
      am_cv_func_iconv=yes,
      am_cv_func_iconv=["no, consider installing GNU libiconv"])])

  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])

    AC_CACHE_CHECK([if the declaration of iconv() needs const.],
		   am_cv_proto_iconv_const,[
      AC_TRY_COMPILE(CF__ICONV_HEAD [
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
],[], am_cv_proto_iconv_const=no,
      am_cv_proto_iconv_const=yes)])

    if test "$am_cv_proto_iconv_const" = yes ; then
      am_cv_proto_iconv_arg1="const"
    else
      am_cv_proto_iconv_arg1=""
    fi

    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi

  LIBICONV=
  if test "$cf_cv_find_linkage_iconv" = yes; then
    CF_ADD_INCDIR($cf_cv_header_path_iconv)
    if test -n "$cf_cv_library_file_iconv" ; then
      LIBICONV="-liconv"
      CF_ADD_LIBDIR($cf_cv_library_path_iconv)
    fi
  fi

  AC_SUBST(LIBICONV)
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_LANGINFO_CODESET version: 6 updated: 2021/01/01 16:53:59
dnl -------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl codeset.m4
dnl ====================
dnl serial AM1
dnl
dnl From Bruno Haible.
AC_DEFUN([AM_LANGINFO_CODESET],
[
AC_CACHE_CHECK([for nl_langinfo and CODESET], am_cv_langinfo_codeset,
	[AC_TRY_LINK([#include <langinfo.h>],
	[char* cs = nl_langinfo(CODESET); (void)cs],
	am_cv_langinfo_codeset=yes,
	am_cv_langinfo_codeset=no)
	])
	if test "$am_cv_langinfo_codeset" = yes; then
		AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
		[Define if you have <langinfo.h> and nl_langinfo(CODESET).])
	fi
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_LC_MESSAGES version: 6 updated: 2021/01/02 09:31:20
dnl --------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl lcmessage.m4
dnl ====================
dnl Check whether LC_MESSAGES is available in <locale.h>.
dnl Ulrich Drepper <drepper@cygnus.com>, 1995.
dnl
dnl This file can be copied and used freely without restrictions.  It can
dnl be used in projects which are not available under the GNU General Public
dnl License or the GNU Library General Public License but which still want
dnl to provide support for the GNU gettext functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Library General Public License, and the rest of the GNU
dnl gettext package package is covered by the GNU General Public License.
dnl They are *not* in the public domain.
dnl
dnl serial 2
dnl
AC_DEFUN([AM_LC_MESSAGES],
[if test "$ac_cv_header_locale_h" = yes; then
	AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
		[AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
		am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
	if test "$am_cv_val_LC_MESSAGES" = yes; then
		AC_DEFINE(HAVE_LC_MESSAGES, 1,
		[Define if your <locale.h> file defines LC_MESSAGES.])
	fi
fi])dnl
dnl ---------------------------------------------------------------------------
dnl AM_MULTIBYTE_ABLE version: 10 updated: 2019/12/31 20:39:42
dnl -----------------
dnl
dnl check for required multibyte/widechar functions
dnl Urs Janssen <urs@tin.org> 20021006
dnl Usage: AM_MULTIBYTE_ABLE
AC_DEFUN([AM_MULTIBYTE_ABLE],
[
  AC_CACHE_CHECK([for wide char and multibyte support], am_cv_multibyte_able,
   [AC_TRY_LINK([#include <stdio.h>
#ifdef HAVE_STDLIB_H
#	include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_WCHAR_H
#	include <wchar.h>
#endif /* HAVE_WCHAR_H */
#ifdef HAVE_WCTYPE_H
#	include <wctype.h>
#endif /* HAVE_WCTYPE_H */
],
     [const char icb[5] = {0xa4, 0xa4, 0xa4, 0xe5, 0x00};
      char ocb[5];
      wchar_t wcb[5];
      wchar_t wcb2[5];
      wchar_t format[3];

      putwc(0, 0);
      fputwc(0, 0);
      fwide(0, 0);
      mbtowc(wcb, icb, MB_CUR_MAX);
      mbstowcs(wcb, icb, 5);
      (void) iswalnum((wint_t) wcb[0]);
      (void) iswcntrl((wint_t) wcb[0]);
      (void) iswdigit((wint_t) wcb[0]);
      (void) iswgraph((wint_t) wcb[0]);
      (void) iswprint((wint_t) wcb[0]);
      (void) iswspace((wint_t) wcb[0]);
      (void) towupper((wint_t) wcb[0]);
      /* (void) iswupper((wint_t) wcb[0]); */
      /* (void) towlower((wint_t) wcb[0]); */
      /* (void) iswlower((wint_t) wcb[0]); */
      (void) iswalpha((wint_t) wcb[0]);
      /* (void) iswblank((wint_t) wcb[0]); */
      /* (void) iswpunct((wint_t) wcb[0]); */
      /* (void) iswxdigit((wint_t) wcb[0]); */
      /* (void) iswctype((wint_t) wcb[0], wctype("print")); */
      /* (void) towctranse((wint_t) wcb[0], wctrans("toupper")); */
      (void) wcslen(wcb);
      /* (void) wcsnlen(wcb, 4); */
      wcwidth((wint_t) wcb[0]);
      wcswidth(wcb, 5);
      wcstombs(ocb, wcb, 5);
      wctomb(ocb, wcb[0]);
      wcscat(wcb2, wcb);
      wcscpy(wcb2, wcb);
      mbstowcs(format, "%s", 2);
      swprintf(wcb, 5, format, "test");
      wcsncat(wcb2, wcb, 5);],
     am_cv_multibyte_able=yes,
     [cf_save_LIBS="$LIBS"
      LIBS="-lutf8 $LIBS"
      AC_TRY_LINK([#include <libutf8.h>],
       [const char icb[5] = {0xa4, 0xa4, 0xa4, 0xe5, 0x00};
        char ocb[5];
        wchar_t wcb[5];
        wchar_t wcb2[5];
        wchar_t format[3];

        putwc(0, 0);
        fputwc(0, 0);
        fwide(0, 0);
        mbtowc(wcb, icb, MB_CUR_MAX);
        mbstowcs(wcb, icb, 5);
        (void) iswalnum((wint_t) wcb[0]);
        (void) iswcntrl((wint_t) wcb[0]);
        (void) iswdigit((wint_t) wcb[0]);
        (void) iswgraph((wint_t) wcb[0]);
        (void) iswprint((wint_t) wcb[0]);
        (void) iswspace((wint_t) wcb[0]);
        (void) towupper((wint_t) wcb[0]);
        /* (void) iswupper((wint_t) wcb[0]); */
        /* (void) towlower((wint_t) wcb[0]); */
        /* (void) iswlower((wint_t) wcb[0]); */
        (void) iswalpha((wint_t) wcb[0]);
        /* (void) iswblank((wint_t) wcb[0]); */
        /* (void) iswpunct((wint_t) wcb[0]); */
        /* (void) iswxdigit((wint_t) wcb[0]); */
        /* (void) iswctype((wint_t) wcb[0], wctype("print")); */
        /* (void) towctranse((wint_t) wcb[0], wctrans("toupper")); */
        (void) wcslen(wcb);
        /* (void) wcsnlen(wcb, 4); */
        wcwidth((wint_t) wcb[0]);
        wcswidth(wcb, 5);
        wcstombs(ocb, wcb, 5);
        wctomb(ocb, wcb[0]);
        wcscat(wcb2, wcb);
        wcscpy(wcb2, wcb);
        mbstowcs(format, "%s", 2);
        swprintf(wcb, 5, format, "test");
        wcsncat(wcb2, wcb, 5);],
        [am_cv_multibyte_able=libutf8],
        [am_cv_multibyte_able=no])
      LIBS="$cf_save_LIBS"
   ])
  ])
  if test "$am_cv_multibyte_able" != no; then
    if test "$am_cv_multibyte_able" = libutf8; then
      AC_DEFINE(HAVE_LIBUTF8_H,1,[Define this to 1 if we have header libutf8.h])
      LIBS="-lutf8 $LIBS"
    fi
    AC_DEFINE(MULTIBYTE_ABLE, 1,
      [Define if you have swprintf() and co.])
  fi
])
dnl ---------------------------------------------------------------------------
dnl AM_PATH_PROG_WITH_TEST version: 10 updated: 2021/01/02 09:31:20
dnl ----------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl progtest.m4
dnl ====================
dnl Search path for a program which passes the given test.
dnl Ulrich Drepper <drepper@cygnus.com>, 1996.
dnl
dnl This file can be copied and used freely without restrictions.  It can
dnl be used in projects which are not available under the GNU General Public
dnl License or the GNU Library General Public License but which still want
dnl to provide support for the GNU gettext functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Library General Public License, and the rest of the GNU
dnl gettext package package is covered by the GNU General Public License.
dnl They are *not* in the public domain.
dnl
dnl serial 2
dnl
dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AM_PATH_PROG_WITH_TEST],
[# Extract the first word of "$2", so it can be a program name with args.
AC_REQUIRE([CF_PATHSEP])
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  [[\\/]*|?:[\\/]]*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR}"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_word$ac_exeext" ; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word$ac_exeext"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test ifelse([$4], , [-n "[$]$1"], ["[$]$1" != "$4"]); then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl AM_WITH_NLS version: 30 updated: 2021/01/02 09:31:20
dnl -----------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl gettext.m4
dnl ====================
dnl Macro to add for using GNU gettext.
dnl Ulrich Drepper <drepper@cygnus.com>, 1995.
dnl ====================
dnl Modified to use CF_FIND_LINKAGE and CF_ADD_SEARCHPATH, to broaden the
dnl range of locations searched.  Retain the same cache-variable naming to
dnl allow reuse with the other gettext macros -Thomas E Dickey
dnl ====================
dnl
dnl This file can be copied and used freely without restrictions.  It can
dnl be used in projects which are not available under the GNU General Public
dnl License or the GNU Library General Public License but which still want
dnl to provide support for the GNU gettext functionality.
dnl Please note that the actual code of the GNU gettext library is covered
dnl by the GNU Library General Public License, and the rest of the GNU
dnl gettext package package is covered by the GNU General Public License.
dnl They are *not* in the public domain.
dnl
dnl serial 10
dnl
dnl Usage: AM_WITH_NLS([TOOLSYMBOL], [NEEDSYMBOL], [LIBDIR], [ENABLED]).
dnl If TOOLSYMBOL is specified and is 'use-libtool', then a libtool library
dnl    $(top_builddir)/intl/libintl.la will be created (shared and/or static,
dnl    depending on --{enable,disable}-{shared,static} and on the presence of
dnl    AM-DISABLE-SHARED). Otherwise, a static library
dnl    $(top_builddir)/intl/libintl.a will be created.
dnl If NEEDSYMBOL is specified and is 'need-ngettext', then GNU gettext
dnl    implementations (in libc or libintl) without the ngettext() function
dnl    will be ignored.
dnl LIBDIR is used to find the intl libraries.  If empty,
dnl    the value `$(top_builddir)/intl/' is used.
dnl ENABLED is used to control the default for the related --enable-nls, since
dnl    not all application developers want this feature by default, e.g., lynx.
dnl
dnl The result of the configuration is one of three cases:
dnl 1) GNU gettext, as included in the intl subdirectory, will be compiled
dnl    and used.
dnl    Catalog format: GNU --> install in $(datadir)
dnl    Catalog extension: .mo after installation, .gmo in source tree
dnl 2) GNU gettext has been found in the system's C library.
dnl    Catalog format: GNU --> install in $(datadir)
dnl    Catalog extension: .mo after installation, .gmo in source tree
dnl 3) No internationalization, always use English msgid.
dnl    Catalog format: none
dnl    Catalog extension: none
dnl The use of .gmo is historical (it was needed to avoid overwriting the
dnl GNU format catalogs when building on a platform with an X/Open gettext),
dnl but we keep it in order not to force irrelevant filename changes on the
dnl maintainers.
dnl
AC_DEFUN([AM_WITH_NLS],
[AC_MSG_CHECKING([whether NLS is requested])
  dnl Default is enabled NLS
  ifelse([$4],,[
  AC_ARG_ENABLE(nls,
    [  --disable-nls           do not use Native Language Support],
    USE_NLS=$enableval, USE_NLS=yes)],[
  AC_ARG_ENABLE(nls,
    [  --enable-nls            use Native Language Support],
    USE_NLS=$enableval, USE_NLS=no)])
  AC_MSG_RESULT($USE_NLS)
  AC_SUBST(USE_NLS)

  BUILD_INCLUDED_LIBINTL=no
  USE_INCLUDED_LIBINTL=no
  INTLLIBS=

  dnl If we use NLS figure out what method
  if test "$USE_NLS" = "yes"; then
    dnl We need to process the po/ directory.
    POSUB=po
    AC_DEFINE(ENABLE_NLS, 1,
      [Define to 1 if translation of program messages to the user's native language
 is requested.])
    AC_MSG_CHECKING([whether included gettext is requested])
    AC_ARG_WITH(included-gettext,
      [  --with-included-gettext use the GNU gettext library included here],
      nls_cv_force_use_gnu_gettext=$withval,
      nls_cv_force_use_gnu_gettext=no)
    AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

    nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
    if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
      dnl User does not insist on using GNU NLS library.  Figure out what
      dnl to use.  If GNU gettext is available we use this.  Else we may have
      dnl to fall back to GNU NLS library.
      CATOBJEXT=NONE

      dnl Save these (possibly-set) variables for reference.  If the user
      dnl overrode these to provide full pathnames, then warn if not actually
      dnl GNU gettext, but do not override their values.  Also, if they were
      dnl overridden, suppress the part of the library test which prevents it
      dnl from finding anything other than GNU gettext.  Doing this also
      dnl suppresses a bogus search for the intl library.
      cf_save_msgfmt_path="$MSGFMT"
      cf_save_xgettext_path="$XGETTEXT"

      dnl Search for GNU msgfmt in the PATH.
      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
          [$ac_dir/$ac_word --statistics /dev/null >/dev/null 2>&1], :)
      AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
      AC_SUBST(MSGFMT)

      dnl Search for GNU xgettext in the PATH.
      AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
          [$ac_dir/$ac_word --omit-header /dev/null >/dev/null 2>&1], :)

      cf_save_OPTS_1="$CPPFLAGS"
      if test "x$cf_save_msgfmt_path" = "x$MSGFMT" && \
         test "x$cf_save_xgettext_path" = "x$XGETTEXT" ; then
          CF_ADD_CFLAGS(-DIGNORE_MSGFMT_HACK)
      fi

      cf_save_LIBS_1="$LIBS"
      CF_ADD_LIBS($LIBICONV)

      CF_FIND_LINKAGE(CF__INTL_HEAD,
        CF__INTL_BODY($2),
        intl,
        cf_cv_func_gettext=yes,
        cf_cv_func_gettext=no)

      AC_MSG_CHECKING([for libintl.h and gettext()])
      AC_MSG_RESULT($cf_cv_func_gettext)

      LIBS="$cf_save_LIBS_1"
      CPPFLAGS="$cf_save_OPTS_1"

      if test "$cf_cv_func_gettext" = yes ; then
        AC_DEFINE(HAVE_LIBINTL_H,1,[Define to 1 if we have libintl.h])

        dnl If an already present or preinstalled GNU gettext() is found,
        dnl use it.  But if this macro is used in GNU gettext, and GNU
        dnl gettext is already preinstalled in libintl, we update this
        dnl libintl.  (Cf. the install rule in intl/Makefile.in.)
        if test "$PACKAGE" != gettext; then
          AC_DEFINE(HAVE_GETTEXT, 1,
              [Define if the GNU gettext() function is already present or preinstalled.])

          CF_ADD_INCDIR($cf_cv_header_path_intl)

          if test -n "$cf_cv_library_file_intl" ; then
            dnl If iconv() is in a separate libiconv library, then anyone
            dnl linking with libintl{.a,.so} also needs to link with
            dnl libiconv.
            INTLLIBS="$cf_cv_library_file_intl $LIBICONV"
            CF_ADD_LIBDIR($cf_cv_library_path_intl,INTLLIBS)
          fi

          gt_save_LIBS="$LIBS"
          LIBS="$LIBS $INTLLIBS"
          AC_CHECK_FUNCS(dcgettext)
          LIBS="$gt_save_LIBS"

          CATOBJEXT=.gmo
        fi
      elif test -z "$MSGFMT" || test -z "$XGETTEXT" ; then
        AC_MSG_WARN(disabling NLS feature)
        sed -e /ENABLE_NLS/d confdefs.h >confdefs.tmp
        mv confdefs.tmp confdefs.h
        ALL_LINGUAS=
        CATOBJEXT=.ignored
        MSGFMT=":"
        GMSGFMT=":"
        XGETTEXT=":"
        POSUB=
        BUILD_INCLUDED_LIBINTL=no
        USE_INCLUDED_LIBINTL=no
        USE_NLS=no
        nls_cv_use_gnu_gettext=no
      fi

      if test "$CATOBJEXT" = "NONE"; then
        dnl GNU gettext is not found in the C library.
        dnl Fall back on GNU gettext library.
        nls_cv_use_gnu_gettext=maybe
      fi
    fi

    if test "$nls_cv_use_gnu_gettext" != "no"; then
      CATOBJEXT=.gmo
      if test -f "$srcdir/intl/libintl.h" ; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        BUILD_INCLUDED_LIBINTL=yes
        USE_INCLUDED_LIBINTL=yes
        INTLLIBS="ifelse([$3],[],\$(top_builddir)/intl,[$3])/libintl.ifelse([$1], use-libtool, [l], [])a $LIBICONV"
        LIBS=`echo " $LIBS " | sed -e 's/ -lintl / /' -e 's/^ //' -e 's/ $//'`
      elif test "$nls_cv_use_gnu_gettext" = "yes"; then
        nls_cv_use_gnu_gettext=no
        AC_MSG_WARN(no NLS library is packaged with this application)
      fi
    fi

    dnl Test whether we really found GNU msgfmt.
    if test "$GMSGFMT" != ":"; then
      if $GMSGFMT --statistics /dev/null >/dev/null 2>&1; then
        : ;
      else
        AC_MSG_WARN([found msgfmt program is not GNU msgfmt])
      fi
    fi

    dnl Test whether we really found GNU xgettext.
    if test "$XGETTEXT" != ":"; then
      if $XGETTEXT --omit-header /dev/null >/dev/null 2>&1; then
        : ;
      else
        AC_MSG_WARN([found xgettext program is not GNU xgettext])
      fi
    fi
  fi

  if test "$XGETTEXT" != ":"; then
    AC_OUTPUT_COMMANDS(
     [for ac_file in $CONFIG_FILES; do

        # Support "outfile[:infile[:infile...]]"
        case "$ac_file" in
          *:*) ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
        esac

        # PO directories have a Makefile.in generated from Makefile.inn.
        case "$ac_file" in
        */[Mm]akefile.in)
          # Adjust a relative srcdir.
          ac_dir="`echo "$ac_file"|sed 's%/[^/][^/]*$%%'`"
          ac_dir_suffix="/`echo "$ac_dir"|sed 's%^\./%%'`"
          ac_dots="`echo "$ac_dir_suffix"|sed 's%/[^/]*%../%g'`"
          ac_base="`basename $ac_file .in`"
          # In autoconf-2.13 it is called $ac_given_srcdir.
          # In autoconf-2.50 it is called $srcdir.
          test -n "$ac_given_srcdir" || ac_given_srcdir="$srcdir"

          case "$ac_given_srcdir" in
            .)  top_srcdir=`echo $ac_dots|sed 's%/$%%'` ;;
            /*) top_srcdir="$ac_given_srcdir" ;;
            *)  top_srcdir="$ac_dots$ac_given_srcdir" ;;
          esac

          if test -f "$ac_given_srcdir/$ac_dir/POTFILES.in"; then
            rm -f "$ac_dir/POTFILES"
            test -n "$as_me" && echo "$as_me: creating $ac_dir/POTFILES" || echo "creating $ac_dir/POTFILES"
            sed -e "/^#/d" -e "/^[ 	]*\$/d" -e "s,.*,     $top_srcdir/& \\\\," -e "\$s/\(.*\) \\\\/\1/" < "$ac_given_srcdir/$ac_dir/POTFILES.in" > "$ac_dir/POTFILES"
            test -n "$as_me" && echo "$as_me: creating $ac_dir/$ac_base" || echo "creating $ac_dir/$ac_base"
            sed -e "/POTFILES =/r $ac_dir/POTFILES" "$ac_dir/$ac_base.in" > "$ac_dir/$ac_base"
          fi
          ;;
        esac
      done])

    dnl If this is used in GNU gettext we have to set BUILD_INCLUDED_LIBINTL
    dnl to 'yes' because some of the testsuite requires it.
    if test "$PACKAGE" = gettext; then
      BUILD_INCLUDED_LIBINTL=yes
    fi

    dnl intl/plural.c is generated from intl/plural.y. It requires bison,
    dnl because plural.y uses bison specific features. It requires at least
    dnl bison-1.26 because earlier versions generate a plural.c that doesn't
    dnl compile.
    dnl bison is only needed for the maintainer (who touches plural.y). But in
    dnl order to avoid separate Makefiles or --enable-maintainer-mode, we put
    dnl the rule in general Makefile. Now, some people carelessly touch the
    dnl files or have a broken "make" program, hence the plural.c rule will
    dnl sometimes fire. To avoid an error, defines BISON to ":" if it is not
    dnl present or too old.
    if test "$nls_cv_use_gnu_gettext" = "yes"; then
      AC_CHECK_PROGS([INTLBISON], [bison])
      if test -z "$INTLBISON"; then
        ac_verc_fail=yes
      else
        dnl Found it, now check the version.
        AC_MSG_CHECKING([version of bison])
changequote(<<,>>)dnl
        ac_prog_version=`$INTLBISON --version 2>&1 | sed -n 's/^.*GNU Bison.* \([0-9]*\.[0-9.]*\).*$/\1/p'`
        case "$ac_prog_version" in
          '') ac_prog_version="v. ?.??, bad"; ac_verc_fail=yes;;
          1.2[6-9]*|1.[3-9][0-9]*|[2-9].*)
changequote([,])dnl
             ac_prog_version="$ac_prog_version, ok"; ac_verc_fail=no;;
          *) ac_prog_version="$ac_prog_version, bad"; ac_verc_fail=yes;;
        esac
      AC_MSG_RESULT([$ac_prog_version])
      fi
      if test "$ac_verc_fail" = yes; then
        INTLBISON=:
      fi
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done
  fi

  dnl Make all variables we use known to autoconf.
  AC_SUBST(BUILD_INCLUDED_LIBINTL)
  AC_SUBST(USE_INCLUDED_LIBINTL)
  AC_SUBST(CATALOGS)
  AC_SUBST(CATOBJEXT)
  AC_SUBST(GMOFILES)
  AC_SUBST(INTLLIBS)
  AC_SUBST(INTLOBJS)
  AC_SUBST(POFILES)
  AC_SUBST(POSUB)

  dnl For backward compatibility. Some configure.ins may be using this.
  nls_cv_header_intl=
  nls_cv_header_libgt=

  dnl For backward compatibility. Some Makefiles may be using this.
  DATADIRNAME=share
  AC_SUBST(DATADIRNAME)

  dnl For backward compatibility. Some Makefiles may be using this.
  INSTOBJEXT=.mo
  AC_SUBST(INSTOBJEXT)

  dnl For backward compatibility. Some Makefiles may be using this.
  GENCAT=gencat
  AC_SUBST(GENCAT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_CHECK version: 5 updated: 2014/06/04 19:11:49
dnl ------------------
dnl Conditionally generate script according to whether we're using a given autoconf.
dnl
dnl $1 = version to compare against
dnl $2 = code to use if AC_ACVERSION is at least as high as $1.
dnl $3 = code to use if AC_ACVERSION is older than $1.
define([CF_ACVERSION_CHECK],
[
ifdef([AC_ACVERSION], ,[ifdef([AC_AUTOCONF_VERSION],[m4_copy([AC_AUTOCONF_VERSION],[AC_ACVERSION])],[m4_copy([m4_PACKAGE_VERSION],[AC_ACVERSION])])])dnl
ifdef([m4_version_compare],
[m4_if(m4_version_compare(m4_defn([AC_ACVERSION]), [$1]), -1, [$3], [$2])],
[CF_ACVERSION_COMPARE(
AC_PREREQ_CANON(AC_PREREQ_SPLIT([$1])),
AC_PREREQ_CANON(AC_PREREQ_SPLIT(AC_ACVERSION)), AC_ACVERSION, [$2], [$3])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_COMPARE version: 3 updated: 2012/10/03 18:39:53
dnl --------------------
dnl CF_ACVERSION_COMPARE(MAJOR1, MINOR1, TERNARY1,
dnl                      MAJOR2, MINOR2, TERNARY2,
dnl                      PRINTABLE2, not FOUND, FOUND)
define([CF_ACVERSION_COMPARE],
[ifelse(builtin([eval], [$2 < $5]), 1,
[ifelse([$8], , ,[$8])],
[ifelse([$9], , ,[$9])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 15 updated: 2020/12/31 10:54:15
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl $1 = flags to add
dnl $2 = if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case "$cf_fix_cppflags" in
no)
	case "$cf_add_cflags" in
	-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C)
		case "$cf_add_cflags" in
		-D*)
			cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
				&& test -z "${cf_tst_cflags}" \
				&& cf_fix_cppflags=yes

			if test "$cf_fix_cppflags" = yes ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		*$cf_add_cflags)
			;;
		*)
			case "$cf_add_cflags" in
			-D*)
				cf_tst_cppflags=`echo "x$cf_add_cflags" | sed -e 's/^...//' -e 's/=.*//'`
				CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,$cf_tst_cppflags)
				;;
			esac
			CF_APPEND_TEXT(cf_new_cppflags,$cf_add_cflags)
			;;
		esac
		;;
	*)
		CF_APPEND_TEXT(cf_new_cflags,$cf_add_cflags)
		;;
	esac
	;;
yes)
	CF_APPEND_TEXT(cf_new_extra_cppflags,$cf_add_cflags)

	cf_tst_cflags=`echo "${cf_add_cflags}" |sed -e 's/^[[^"]]*"'\''//'`

	test "x${cf_add_cflags}" != "x${cf_tst_cflags}" \
		&& test -z "${cf_tst_cflags}" \
		&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CF_APPEND_TEXT(CFLAGS,$cf_new_cflags)
fi

if test -n "$cf_new_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CF_APPEND_TEXT(CPPFLAGS,$cf_new_cppflags)
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	CF_APPEND_TEXT(EXTRA_CPPFLAGS,$cf_new_extra_cppflags)
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_INCDIR version: 16 updated: 2020/12/31 20:19:42
dnl -------------
dnl Add an include-directory to $CPPFLAGS.  Don't add /usr/include, since it's
dnl redundant.  We don't normally need to add -I/usr/local/include for gcc,
dnl but old versions (and some misinstalled ones) need that.  To make things
dnl worse, gcc 3.x may give error messages if -I/usr/local/include is added to
dnl the include-path).
AC_DEFUN([CF_ADD_INCDIR],
[
if test -n "$1" ; then
  for cf_add_incdir in $1
  do
	while test "$cf_add_incdir" != /usr/include
	do
	  if test -d "$cf_add_incdir"
	  then
		cf_have_incdir=no
		if test -n "$CFLAGS$CPPFLAGS" ; then
		  # a loop is needed to ensure we can add subdirs of existing dirs
		  for cf_test_incdir in $CFLAGS $CPPFLAGS ; do
			if test ".$cf_test_incdir" = ".-I$cf_add_incdir" ; then
			  cf_have_incdir=yes; break
			fi
		  done
		fi

		if test "$cf_have_incdir" = no ; then
		  if test "$cf_add_incdir" = /usr/local/include ; then
			if test "$GCC" = yes
			then
			  cf_save_CPPFLAGS=$CPPFLAGS
			  CF_APPEND_TEXT(CPPFLAGS,-I$cf_add_incdir)
			  AC_TRY_COMPILE([#include <stdio.h>],
				  [printf("Hello")],
				  [],
				  [cf_have_incdir=yes])
			  CPPFLAGS=$cf_save_CPPFLAGS
			fi
		  fi
		fi

		if test "$cf_have_incdir" = no ; then
		  CF_VERBOSE(adding $cf_add_incdir to include-path)
		  ifelse([$2],,CPPFLAGS,[$2])="$ifelse([$2],,CPPFLAGS,[$2]) -I$cf_add_incdir"

		  cf_top_incdir=`echo "$cf_add_incdir" | sed -e 's%/include/.*$%/include%'`
		  test "$cf_top_incdir" = "$cf_add_incdir" && break
		  cf_add_incdir="$cf_top_incdir"
		else
		  break
		fi
	  else
		break
	  fi
	done
  done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB version: 2 updated: 2010/06/02 05:03:05
dnl ----------
dnl Add a library, used to enforce consistency.
dnl
dnl $1 = library to add, without the "-l"
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIB],[CF_ADD_LIBS(-l$1,ifelse($2,,LIBS,[$2]))])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBDIR version: 11 updated: 2020/12/31 20:19:42
dnl -------------
dnl	Adds to the library-path
dnl
dnl	Some machines have trouble with multiple -L options.
dnl
dnl $1 is the (list of) directory(s) to add
dnl $2 is the optional name of the variable to update (default LDFLAGS)
dnl
AC_DEFUN([CF_ADD_LIBDIR],
[
if test -n "$1" ; then
	for cf_add_libdir in $1
	do
		if test "$cf_add_libdir" = /usr/lib ; then
			:
		elif test -d "$cf_add_libdir"
		then
			cf_have_libdir=no
			if test -n "$LDFLAGS$LIBS" ; then
				# a loop is needed to ensure we can add subdirs of existing dirs
				for cf_test_libdir in $LDFLAGS $LIBS ; do
					if test ".$cf_test_libdir" = ".-L$cf_add_libdir" ; then
						cf_have_libdir=yes; break
					fi
				done
			fi
			if test "$cf_have_libdir" = no ; then
				CF_VERBOSE(adding $cf_add_libdir to library-path)
				ifelse([$2],,LDFLAGS,[$2])="-L$cf_add_libdir $ifelse([$2],,LDFLAGS,[$2])"
			fi
		fi
	done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBS version: 3 updated: 2019/11/02 16:47:33
dnl -----------
dnl Add one or more libraries, used to enforce consistency.  Libraries are
dnl prepended to an existing list, since their dependencies are assumed to
dnl already exist in the list.
dnl
dnl $1 = libraries to add, with the "-l", etc.
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIBS],[
cf_add_libs="[$]ifelse($2,,LIBS,[$2])"
# reverse order
cf_add_0lib=
for cf_add_1lib in $1; do cf_add_0lib="$cf_add_1lib $cf_add_0lib"; done
# filter duplicates
for cf_add_1lib in $cf_add_0lib; do
	for cf_add_2lib in $cf_add_libs; do
		if test "x$cf_add_1lib" = "x$cf_add_2lib"; then
			cf_add_1lib=
			break
		fi
	done
	test -n "$cf_add_1lib" && cf_add_libs="$cf_add_1lib $cf_add_libs"
done
ifelse($2,,LIBS,[$2])="$cf_add_libs"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB_AFTER version: 3 updated: 2013/07/09 21:27:22
dnl ----------------
dnl Add a given library after another, e.g., following the one it satisfies a
dnl dependency for.
dnl
dnl $1 = the first library
dnl $2 = its dependency
AC_DEFUN([CF_ADD_LIB_AFTER],[
CF_VERBOSE(...before $LIBS)
LIBS=`echo "$LIBS" | sed -e "s/[[ 	]][[ 	]]*/ /g" -e "s%$1 %$1 $2 %" -e 's%  % %g'`
CF_VERBOSE(...after  $LIBS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_OPTIONAL_PATH version: 3 updated: 2015/05/10 19:52:14
dnl --------------------
dnl Add an optional search-path to the compile/link variables.
dnl See CF_WITH_PATH
dnl
dnl $1 = shell variable containing the result of --with-XXX=[DIR]
dnl $2 = module to look for.
AC_DEFUN([CF_ADD_OPTIONAL_PATH],[
case "$1" in
no)
	;;
yes)
	;;
*)
	CF_ADD_SEARCHPATH([$1], [AC_MSG_ERROR(cannot find $2 under $1)])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SEARCHPATH version: 6 updated: 2020/12/31 20:19:42
dnl -----------------
dnl Set $CPPFLAGS and $LDFLAGS with the directories given via the parameter.
dnl They can be either the common root of include- and lib-directories, or the
dnl lib-directory (to allow for things like lib64 directories).
dnl See also CF_FIND_LINKAGE.
dnl
dnl $1 is the list of colon-separated directory names to search.
dnl $2 is the action to take if a parameter does not yield a directory.
AC_DEFUN([CF_ADD_SEARCHPATH],
[
AC_REQUIRE([CF_PATHSEP])
for cf_searchpath in `echo "$1" | tr $PATH_SEPARATOR ' '`; do
	if test -d "$cf_searchpath/include" ; then
		CF_ADD_INCDIR($cf_searchpath/include)
	elif test -d "$cf_searchpath/../include" ; then
		CF_ADD_INCDIR($cf_searchpath/../include)
	ifelse([$2],,,[else
$2])
	fi
	if test -d "$cf_searchpath/lib" ; then
		CF_ADD_LIBDIR($cf_searchpath/lib)
	elif test -d "$cf_searchpath" ; then
		CF_ADD_LIBDIR($cf_searchpath)
	ifelse([$2],,,[else
$2])
	fi
done
])
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SUBDIR_PATH version: 5 updated: 2020/12/31 20:19:42
dnl ------------------
dnl Append to a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
dnl $4 = the directory under which we will test for subdirectories
dnl $5 = a directory that we do not want $4 to match
AC_DEFUN([CF_ADD_SUBDIR_PATH],
[
test "x$4" != "x$5" && \
test -d "$4" && \
ifelse([$5],NONE,,[{ test -z "$5" || test "x$5" = xNONE || test "x$4" != "x$5"; } &&]) {
	test -n "$verbose" && echo "	... testing for $3-directories under $4"
	test -d "$4/$3" &&          $1="[$]$1 $4/$3"
	test -d "$4/$3/$2" &&       $1="[$]$1 $4/$3/$2"
	test -d "$4/$3/$2/$3" &&    $1="[$]$1 $4/$3/$2/$3"
	test -d "$4/$2/$3" &&       $1="[$]$1 $4/$2/$3"
	test -d "$4/$2/$3/$2" &&    $1="[$]$1 $4/$2/$3/$2"
}
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_CHECK version: 13 updated: 2012/10/06 11:17:15
dnl ----------------
dnl This was originally adapted from the macros 'fp_PROG_CC_STDC' and
dnl 'fp_C_PROTOTYPES' in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
CF_CC_ENV_FLAGS

AC_CACHE_CHECK(for ${CC:-cc} option to accept ANSI C, cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" \
	"" \
	-qlanglvl=ansi \
	-std1 \
	-Ae \
	"-Aa -D_HPUX_SOURCE" \
	-Xc
do
	CF_ADD_CFLAGS($cf_arg)
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || (__STDC__ != 1)
choke me
#endif
#endif
],[
	int test (int i, double x);
	struct s1 {int (*f) (int a);};
	struct s2 {int (*f) (double a);};],
	[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
CPPFLAGS="$cf_save_CPPFLAGS"
])

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CF_ADD_CFLAGS($cf_cv_ansi_cc)
else
	AC_DEFINE(CC_HAS_PROTOS,1,[Define to 1 if C compiler supports prototypes])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_REQD version: 4 updated: 2008/03/23 14:48:54
dnl ---------------
dnl For programs that must use an ANSI compiler, obtain compiler options that
dnl will make it recognize prototypes.  We'll do preprocessor checks in other
dnl macros, since tools such as unproto can fake prototypes, but only part of
dnl the preprocessor.
AC_DEFUN([CF_ANSI_CC_REQD],
[AC_REQUIRE([CF_ANSI_CC_CHECK])
if test "$cf_cv_ansi_cc" = "no"; then
	AC_MSG_ERROR(
[Your compiler does not appear to recognize prototypes.
You have the following choices:
	a. adjust your compiler options
	b. get an up-to-date compiler
	c. use a wrapper such as unproto])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_APPEND_TEXT version: 1 updated: 2017/02/25 18:58:55
dnl --------------
dnl use this macro for appending text without introducing an extra blank at
dnl the beginning
define([CF_APPEND_TEXT],
[
	test -n "[$]$1" && $1="[$]$1 "
	$1="[$]{$1}$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_ENABLE version: 3 updated: 1999/03/30 17:24:31
dnl -------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 5 updated: 2015/05/10 19:52:14
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE([$1],[$2],[test "$enableval" != ifelse([$5],no,yes,no) && enableval=ifelse([$5],no,no,yes)
	if test "$enableval" != "$5" ; then
ifelse([$3],,[    :]dnl
,[    $3]) ifelse([$4],,,[
	else
		$4])
	fi],[enableval=$5 ifelse([$4],,,[
	$4
])dnl
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_WITH version: 4 updated: 2008/03/23 14:48:54
dnl -----------
dnl Restricted form of AC_ARG_WITH that requires user to specify a value
dnl $1 = option name
dnl $2 = help message
dnl $3 = variable to set with the --with value
dnl $4 = default value, if any, must be constant.
dnl $5 = default value shown for --help if $4 is empty.
AC_DEFUN([CF_ARG_WITH],
[AC_ARG_WITH($1,[$2 ](default: ifelse($4,,ifelse($5,,empty,$5),$4)),,
ifelse($4,,[withval="${$3}"],[withval="${$3-$4}"]))dnl
ifelse($4,,[test -n "$withval" && \
],[test -z "$withval" && withval=no
])dnl
case "$withval" in #(vi
yes)
  AC_MSG_ERROR(expected a value for --with-$1)
  ;; #(vi
no) withval=""
  ;;
esac
$3="$withval"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_AR_FLAGS version: 9 updated: 2021/01/01 13:31:04
dnl -----------
dnl Check for suitable "ar" (archiver) options for updating an archive.
dnl
dnl In particular, handle some obsolete cases where the "-" might be omitted,
dnl as well as a workaround for breakage of make's archive rules by the GNU
dnl binutils "ar" program.
AC_DEFUN([CF_AR_FLAGS],[
AC_REQUIRE([CF_PROG_AR])

AC_CACHE_CHECK(for options to update archives, cf_cv_ar_flags,[
	case "$cf_cv_system_name" in
	*-msvc*)
		cf_cv_ar_flags=''
		cat >mk_static_lib.sh <<-EOF
		#!$SHELL
		MSVC_BIN="[$]AR"
		out="\[$]1"
		shift
		exec \[$]MSVC_BIN -out:"\[$]out" \[$]@
		EOF
		chmod +x mk_static_lib.sh
		AR=`pwd`/mk_static_lib.sh
		;;
	*)
		cf_cv_ar_flags=unknown
		for cf_ar_flags in -curvU -curv curv -crv crv -cqv cqv -rv rv
		do

			# check if $ARFLAGS already contains this choice
			if test "x$ARFLAGS" != "x" ; then
				cf_check_ar_flags=`echo "x$ARFLAGS" | sed -e "s/$cf_ar_flags\$//" -e "s/$cf_ar_flags / /"`
				if test "x$ARFLAGS" != "$cf_check_ar_flags" ; then
					cf_cv_ar_flags=
					break
				fi
			fi

			rm -f "conftest.$ac_cv_objext"
			rm -f conftest.a

			cat >"conftest.$ac_ext" <<EOF
#line __oline__ "configure"
int	testdata[[3]] = { 123, 456, 789 };
EOF
			if AC_TRY_EVAL(ac_compile) ; then
				echo "$AR $ARFLAGS $cf_ar_flags conftest.a conftest.$ac_cv_objext" >&AC_FD_CC
				$AR $ARFLAGS "$cf_ar_flags" conftest.a "conftest.$ac_cv_objext" 2>&AC_FD_CC 1>/dev/null
				if test -f conftest.a ; then
					cf_cv_ar_flags="$cf_ar_flags"
					break
				fi
			else
				CF_VERBOSE(cannot compile test-program)
				break
			fi
		done
		rm -f conftest.a "conftest.$ac_ext" "conftest.$ac_cv_objext"
		;;
	esac
])

if test -n "$ARFLAGS" ; then
	if test -n "$cf_cv_ar_flags" ; then
		ARFLAGS="$ARFLAGS $cf_cv_ar_flags"
	fi
else
	ARFLAGS=$cf_cv_ar_flags
fi

AC_SUBST(ARFLAGS)
])
dnl ---------------------------------------------------------------------------
dnl CF_BUILD_CC version: 9 updated: 2021/01/02 09:31:20
dnl -----------
dnl If we're cross-compiling, allow the user to override the tools and their
dnl options.  The configure script is oriented toward identifying the host
dnl compiler, etc., but we need a build compiler to generate parts of the
dnl source.
dnl
dnl $1 = default for $CPPFLAGS
dnl $2 = default for $LIBS
AC_DEFUN([CF_BUILD_CC],[
CF_ACVERSION_CHECK(2.52,,
	[AC_REQUIRE([CF_PROG_EXT])])
if test "$cross_compiling" = yes ; then

	# defaults that we might want to override
	: ${BUILD_CFLAGS:=''}
	: ${BUILD_CPPFLAGS:='ifelse([$1],,,[$1])'}
	: ${BUILD_LDFLAGS:=''}
	: ${BUILD_LIBS:='ifelse([$2],,,[$2])'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}

	AC_ARG_WITH(build-cc,
		[  --with-build-cc=XXX     the build C compiler ($BUILD_CC)],
		[BUILD_CC="$withval"],
		[AC_CHECK_PROGS(BUILD_CC, [gcc clang c99 c89 cc cl],none)])
	AC_MSG_CHECKING(for native build C compiler)
	AC_MSG_RESULT($BUILD_CC)

	AC_MSG_CHECKING(for native build C preprocessor)
	AC_ARG_WITH(build-cpp,
		[  --with-build-cpp=XXX    the build C preprocessor ($BUILD_CPP)],
		[BUILD_CPP="$withval"],
		[BUILD_CPP='${BUILD_CC} -E'])
	AC_MSG_RESULT($BUILD_CPP)

	AC_MSG_CHECKING(for native build C flags)
	AC_ARG_WITH(build-cflags,
		[  --with-build-cflags=XXX the build C compiler-flags ($BUILD_CFLAGS)],
		[BUILD_CFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CFLAGS)

	AC_MSG_CHECKING(for native build C preprocessor-flags)
	AC_ARG_WITH(build-cppflags,
		[  --with-build-cppflags=XXX the build C preprocessor-flags ($BUILD_CPPFLAGS)],
		[BUILD_CPPFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CPPFLAGS)

	AC_MSG_CHECKING(for native build linker-flags)
	AC_ARG_WITH(build-ldflags,
		[  --with-build-ldflags=XXX the build linker-flags ($BUILD_LDFLAGS)],
		[BUILD_LDFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_LDFLAGS)

	AC_MSG_CHECKING(for native build linker-libraries)
	AC_ARG_WITH(build-libs,
		[  --with-build-libs=XXX   the build libraries (${BUILD_LIBS})],
		[BUILD_LIBS="$withval"])
	AC_MSG_RESULT($BUILD_LIBS)

	# this assumes we're on Unix.
	BUILD_EXEEXT=
	BUILD_OBJEXT=o

	: ${BUILD_CC:='${CC}'}

	if { test "$BUILD_CC" = "$CC" || test "$BUILD_CC" = '${CC}'; } ; then
		AC_MSG_ERROR([Cross-build requires two compilers.
Use --with-build-cc to specify the native compiler.])
	fi

else
	: ${BUILD_CC:='${CC}'}
	: ${BUILD_CPP:='${CPP}'}
	: ${BUILD_CFLAGS:='${CFLAGS}'}
	: ${BUILD_CPPFLAGS:='${CPPFLAGS}'}
	: ${BUILD_LDFLAGS:='${LDFLAGS}'}
	: ${BUILD_LIBS:='${LIBS}'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}
fi

AC_SUBST(BUILD_CC)
AC_SUBST(BUILD_CPP)
AC_SUBST(BUILD_CFLAGS)
AC_SUBST(BUILD_CPPFLAGS)
AC_SUBST(BUILD_LDFLAGS)
AC_SUBST(BUILD_LIBS)
AC_SUBST(BUILD_EXEEXT)
AC_SUBST(BUILD_OBJEXT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUNDLED_CANLOCK version: 1 updated: 2020/01/09 20:03:25
dnl ------------------
dnl Top-level macro for configuring an application with a bundled copy of
dnl the canlock library.
dnl
dnl $1 specifies the top of the directory containing CANLOCK's include/lib dirs.
dnl    That is assigned to $cf_canlock_home, which may be updated.
AC_DEFUN([CF_BUNDLED_CANLOCK],
[
cf_canlock_home=$1
CANDIR_MAKE=
CANLIBS=
CAN_CPPFLAGS=
CAN_MAKEFILE=

case .$cf_canlock_home in #(vi
.no) #(vi
	CF_VERBOSE(using bundled canlock)
	CANLIBS='-L../libcanlock -lcanlock'
	CAN_CPPFLAGS='-I$(top_builddir)/libcanlock/include -I$(top_srcdir)/libcanlock/include'
	CAN_MAKEFILE='libcanlock/Makefile'
	;;
.yes) #(vi
	CF_FIND_CANLOCK(
		cf_canlock_cppflags,
		cf_canlock_libs,
		[
			CANDIR_MAKE='#'
			CANLIBS="$cf_canlock_libs"
			CAN_CPPFLAGS="$cf_canlock_cppflags"
			CF_VERBOSE(using installed canlock)
		],[
			CF_VERBOSE(using bundled canlock because no installed canlock was found)
			CANLIBS='-L../libcanlock -lcanlock'
			CAN_CPPFLAGS='-I$(top_builddir)/libcanlock/include -I$(top_srcdir)/libcanlock/include'
			CAN_MAKEFILE='libcanlock/Makefile'
		])
	;;
.*)
	CF_PATH_SYNTAX(cf_canlock_home)
	CANDIR_MAKE='#'
	CANLIBS="-L${cf_canlock_home}/lib -lcanlock"
	CAN_CPPFLAGS="-I${cf_canlock_home}/include"
	CF_VERBOSE(using installed canlock $cf_canlock_home)
	;;
esac

AC_DEFINE(USE_CANLOCK,1,[Define this to 1 to use Cancel-Locks])

AC_SUBST(CANDIR_MAKE)
AC_SUBST(CANLIBS)
AC_SUBST(CAN_CPPFLAGS)
AC_SUBST(CAN_MAKEFILE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUNDLED_INTL version: 20 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Top-level macro for configuring an application with a bundled copy of
dnl the intl and po directories for gettext.
dnl
dnl $1 specifies either Makefile or makefile, defaulting to the former.
dnl $2 if nonempty sets the option to --enable-nls rather than to --disable-nls
dnl
dnl Sets variables which can be used to substitute in makefiles:
dnl	GT_YES       - "#" comment unless building intl library, otherwise empty
dnl	GT_NO        - "#" comment if building intl library, otherwise empty
dnl	INTLDIR_MAKE - to make ./intl directory
dnl	MSG_DIR_MAKE - to make ./po directory
dnl	SUB_MAKEFILE - list of makefiles in ./intl, ./po directories
dnl
dnl Defines:
dnl	HAVE_LIBGETTEXT_H if we're using ./intl
dnl	NLS_TEXTDOMAIN
dnl
dnl Environment:
dnl	ALL_LINGUAS if set, lists the root names of the ".po" files.
dnl	CONFIG_H assumed to be "config.h"
dnl	PACKAGE must be set, used as default for textdomain
dnl	VERSION may be set, otherwise extract from "VERSION" file.
dnl
AC_DEFUN([CF_BUNDLED_INTL],[
cf_makefile=ifelse($1,,Makefile,$1)

dnl Set of available languages (based on source distribution).  Note that
dnl setting $LINGUAS overrides $ALL_LINGUAS.  Some environments set $LINGUAS
dnl rather than $LC_ALL
test -z "$ALL_LINGUAS" && ALL_LINGUAS=`test -d "$srcdir/po" && cd "$srcdir/po" && echo *.po|sed -e 's/\.po//g' -e 's/*//'`

# Allow override of "config.h" definition:
: ${CONFIG_H:=config.h}
AC_SUBST(CONFIG_H)

if test -z "$PACKAGE" ; then
	AC_MSG_ERROR([[CF_BUNDLED_INTL] used without setting [PACKAGE] variable])
fi

if test -z "$VERSION" ; then
if test -f "$srcdir/VERSION" ; then
	VERSION=`sed -e '2,$d' "$srcdir/VERSION" |cut -f1`
else
	VERSION=unknown
fi
fi
AC_SUBST(VERSION)

AM_GNU_GETTEXT(,,,[$2])

if test "$USE_NLS" = yes ; then
	AC_ARG_WITH(textdomain,
		[  --with-textdomain=PKG   NLS text-domain (default is package name)],
		[NLS_TEXTDOMAIN=$withval],
		[NLS_TEXTDOMAIN=$PACKAGE])
	AC_DEFINE_UNQUOTED(NLS_TEXTDOMAIN,"$NLS_TEXTDOMAIN",[Define to the nls textdomain value])
	AC_SUBST(NLS_TEXTDOMAIN)
fi

INTLDIR_MAKE=
MSG_DIR_MAKE=
SUB_MAKEFILE=

dnl this updates SUB_MAKEFILE and MSG_DIR_MAKE:
CF_OUR_MESSAGES($1)

if test "$USE_INCLUDED_LIBINTL" = yes ; then
	if test "$nls_cv_force_use_gnu_gettext" = yes ; then
		:
	elif test "$nls_cv_use_gnu_gettext" = yes ; then
		:
	else
		INTLDIR_MAKE="#"
	fi
	if test -z "$INTLDIR_MAKE"; then
		AC_DEFINE(HAVE_LIBGETTEXT_H,1,[Define to 1 if we have libgettext.h])
		for cf_makefile in \
			$srcdir/intl/Makefile.in \
			$srcdir/intl/makefile.in
		do
			if test -f "$cf_makefile" ; then
				SUB_MAKEFILE="$SUB_MAKEFILE `echo \"${cf_makefile}\"|sed -e 's,^'$srcdir/',,' -e 's/\.in$//'`:${cf_makefile}"
				break
			fi
		done
	fi
else
	INTLDIR_MAKE="#"
	if test "$USE_NLS" = yes ; then
		AC_CHECK_HEADERS(libintl.h)
	fi
fi

if test -z "$INTLDIR_MAKE" ; then
	CF_APPEND_TEXT(CPPFLAGS,-I../intl)
fi

dnl FIXME:  we use this in lynx (the alternative is a spurious dependency upon
dnl GNU make)
if test "$BUILD_INCLUDED_LIBINTL" = yes ; then
	GT_YES="#"
	GT_NO=
else
	GT_YES=
	GT_NO="#"
fi

AC_SUBST(INTLDIR_MAKE)
AC_SUBST(MSG_DIR_MAKE)
AC_SUBST(GT_YES)
AC_SUBST(GT_NO)

dnl FIXME:  the underlying AM_GNU_GETTEXT macro either needs some fixes or a
dnl little documentation.  It doesn't define anything so that we can ifdef our
dnl own code, except ENABLE_NLS, which is too vague to be of any use.

if test "$USE_INCLUDED_LIBINTL" = yes ; then
	if test "$nls_cv_force_use_gnu_gettext" = yes ; then
		AC_DEFINE(HAVE_GETTEXT,1,[Define to 1 if we have gettext function])
	elif test "$nls_cv_use_gnu_gettext" = yes ; then
		AC_DEFINE(HAVE_GETTEXT,1,[Define to 1 if we have gettext function])
	fi
	if test -n "$nls_cv_header_intl" ; then
		AC_DEFINE(HAVE_LIBINTL_H,1,[Define to 1 if we have header-file for libintl])
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUNDLED_PCRE version: 4 updated: 2009/12/24 04:20:51
dnl ---------------
dnl Top-level macro for configuring an application with a bundled copy of
dnl the pcre library.
dnl
dnl $1 specifies the top of the directory containing PCRE's include, lib dirs.
dnl    That is assigned to $cf_pcre_home, which may be updated.
AC_DEFUN([CF_BUNDLED_PCRE],
[
cf_pcre_home=$1
PCREDIR_MAKE=
PCREDIR_LIBS=
PCREDIR_CPPFLAGS=

case .$cf_pcre_home in #(vi
.no) #(vi
	# setup to compile the bundled PCRE:
#	. $srcdir/pcre/version.sh
	AC_SUBST(PCRE_MAJOR)
	AC_SUBST(PCRE_MINOR)
	AC_SUBST(PCRE_DATE)
	AC_SUBST(PCRE_DEFINES)
	;;
.yes) #(vi
	CF_FIND_PCRE(
		cf_pcre_cppflags,
		cf_pcre_libs,
		[
			PCREDIR_MAKE='#'
			PCREDIR_LIBS="$cf_pcre_libs"
			PCREDIR_CPPFLAGS="$cf_pcre_cppflags"
		],[
			CF_VERBOSE(using bundled pcre because no installed pcre was found)
			AC_SUBST(PCRE_MAJOR)
			AC_SUBST(PCRE_MINOR)
			AC_SUBST(PCRE_DATE)
			AC_SUBST(PCRE_DEFINES)
			cf_pcre_home=no
		])
	;;
.*)
	CF_PATH_SYNTAX(cf_pcre_home)
	PCREDIR_MAKE='#'
	PCREDIR_LIBS="-L${cf_pcre_home}/lib -lpcre"
	PCREDIR_CPPFLAGS="-I${cf_pcre_home}/include"
	;;
esac

AC_SUBST(PCREDIR_MAKE)
AC_SUBST(PCREDIR_LIBS)
AC_SUBST(PCREDIR_CPPFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CC_ENV_FLAGS version: 10 updated: 2020/12/31 18:40:20
dnl ---------------
dnl Check for user's environment-breakage by stuffing CFLAGS/CPPFLAGS content
dnl into CC.  This will not help with broken scripts that wrap the compiler
dnl with options, but eliminates a more common category of user confusion.
dnl
dnl In particular, it addresses the problem of being able to run the C
dnl preprocessor in a consistent manner.
dnl
dnl Caveat: this also disallows blanks in the pathname for the compiler, but
dnl the nuisance of having inconsistent settings for compiler and preprocessor
dnl outweighs that limitation.
AC_DEFUN([CF_CC_ENV_FLAGS],
[
# This should have been defined by AC_PROG_CC
: "${CC:=cc}"

AC_MSG_CHECKING(\$CFLAGS variable)
case "x$CFLAGS" in
*-[[IUD]]*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CFLAGS variable to hold CPPFLAGS options)
	cf_flags="$CFLAGS"
	CFLAGS=
	for cf_arg in $cf_flags
	do
		CF_ADD_CFLAGS($cf_arg)
	done
	;;
*)
	AC_MSG_RESULT(ok)
	;;
esac

AC_MSG_CHECKING(\$CC variable)
case "$CC" in
*[[\ \	]]-*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment uses the CC variable to hold CFLAGS/CPPFLAGS options)
	# humor him...
	cf_prog=`echo "$CC" | sed -e 's/	/ /g' -e 's/[[ ]]* / /g' -e 's/[[ ]]*[[ ]]-[[^ ]].*//'`
	cf_flags=`echo "$CC" | ${AWK:-awk} -v prog="$cf_prog" '{ printf("%s", [substr]([$]0,1+length(prog))); }'`
	CC="$cf_prog"
	for cf_arg in $cf_flags
	do
		case "x$cf_arg" in
		x-[[IUDfgOW]]*)
			CF_ADD_CFLAGS($cf_arg)
			;;
		*)
			CC="$CC $cf_arg"
			;;
		esac
	done
	CF_VERBOSE(resulting CC: '$CC')
	CF_VERBOSE(resulting CFLAGS: '$CFLAGS')
	CF_VERBOSE(resulting CPPFLAGS: '$CPPFLAGS')
	;;
*)
	AC_MSG_RESULT(ok)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_1_DECL version: 5 updated: 2001/07/22 21:09:16
dnl ---------------
dnl Check for missing declarations in the system headers (adapted from vile).
dnl
dnl CHECK_DECL_FLAG and CHECK_DECL_HDRS must be set in configure.in
AC_DEFUN([CF_CHECK_1_DECL],
[
AC_MSG_CHECKING([for missing "$1" extern])
AC_CACHE_VAL([cf_cv_func_$1],[
CF_MSG_LOG([for missing "$1" external])
cf_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $CHECK_DECL_FLAG"
AC_TRY_LINK([
$CHECK_DECL_HDRS

#undef $1
struct zowie { int a; double b; struct zowie *c; char d; };
extern struct zowie *$1();
],
[
],
[if test -n "$CHECK_DECL_HDRS" ; then
# try to work around system headers which are infested with non-standard syntax
CF_UPPER(cf_1_up,$1)
AC_TRY_COMPILE([
#define DECL_${cf_1_up}
$CHECK_DECL_HDRS
],[long x = 0],
[eval 'cf_cv_func_'$1'=yes'],
[eval 'cf_cv_func_'$1'=no'])
else
eval 'cf_cv_func_'$1'=yes'
fi
],
[eval 'cf_cv_func_'$1'=no'])
CFLAGS="$cf_save_CFLAGS"
])
eval 'cf_result=$cf_cv_func_'$1
AC_MSG_RESULT($cf_result)
test $cf_result = yes && AC_DEFINE_UNQUOTED(DECL_$2)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 13 updated: 2020/12/31 10:54:15
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f "$srcdir/config.guess" || test -f "$ac_aux_dir/config.guess" ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name",[Define to the system name.])
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CFLAGS version: 4 updated: 2021/01/02 19:22:58
dnl ---------------
dnl Conditionally add to $CFLAGS and $CPPFLAGS values which are derived from
dnl a build-configuration such as imake.  These have the pitfall that they
dnl often contain compiler-specific options which we cannot use, mixed with
dnl preprocessor options that we usually can.
AC_DEFUN([CF_CHECK_CFLAGS],
[
CF_VERBOSE(checking additions to CFLAGS)
cf_check_cflags="$CFLAGS"
cf_check_cppflags="$CPPFLAGS"
CF_ADD_CFLAGS($1,yes)
if test "x$cf_check_cflags" != "x$CFLAGS" ; then
AC_TRY_LINK([#include <stdio.h>],[printf("Hello world");],,
	[CF_VERBOSE(test-compile failed.  Undoing change to \$CFLAGS)
	 if test "x$cf_check_cppflags" != "x$CPPFLAGS" ; then
		 CF_VERBOSE(but keeping change to \$CPPFLAGS)
	 fi
	 CFLAGS="$cf_check_cflags"])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_DECL version: 2 updated: 1997/08/28 23:57:55
dnl -------------
AC_DEFUN([CF_CHECK_DECL],
[for ac_func in $1
do
CF_UPPER(ac_tr_func,$ac_func)
CF_CHECK_1_DECL(${ac_func}, ${ac_tr_func})dnl
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_ERRNO version: 13 updated: 2020/03/10 18:53:47
dnl --------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>, e.g.,
dnl the 'errno' variable.  Define a DECL_xxx symbol if we must declare it
dnl ourselves.
dnl
dnl $1 = the name to check
dnl $2 = the assumed type
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_CACHE_CHECK(if external $1 is declared, cf_cv_dcl_$1,[
	AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
	ifelse([$2],,int,[$2]) x = (ifelse([$2],,int,[$2])) $1; (void)x,
	[cf_cv_dcl_$1=yes],
	[cf_cv_dcl_$1=no])
])

if test "$cf_cv_dcl_$1" = no ; then
	CF_UPPER(cf_result,decl_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,ifelse([$2],,int,[$2]))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_EXTERN_DATA version: 4 updated: 2015/04/18 08:56:57
dnl --------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it's real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_CACHE_CHECK(if external $1 exists, cf_cv_have_$1,[
	AC_TRY_LINK([
#undef $1
extern $2 $1;
],
	[$1 = 2],
	[cf_cv_have_$1=yes],
	[cf_cv_have_$1=no])
])

if test "$cf_cv_have_$1" = yes ; then
	CF_UPPER(cf_result,have_$1)
	AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_FD_SET version: 5 updated: 2012/10/06 11:17:15
dnl ---------------
dnl Check if the fd_set type and corresponding macros are defined.
AC_DEFUN([CF_CHECK_FD_SET],
[
AC_REQUIRE([CF_TYPE_FD_SET])
AC_CACHE_CHECK([for fd_set macros],cf_cv_macros_fd_set,[
AC_TRY_COMPILE([
#include <sys/types.h>
#if USE_SYS_SELECT_H
# include <sys/select.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#  ifdef TIME_WITH_SYS_TIME
#   include <time.h>
#  endif
# else
#  include <time.h>
# endif
#endif
],[
	fd_set read_bits;
	FD_ZERO(&read_bits);
	FD_SET(0, &read_bits);],
	[cf_cv_macros_fd_set=yes],
	[cf_cv_macros_fd_set=no])])
test $cf_cv_macros_fd_set = yes && AC_DEFINE(HAVE_TYPE_FD_SET,1,[Define to 1 if type fd_set is declared])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_HEADERS version: 3 updated: 2000/11/03 11:14:19
dnl ----------------
dnl AC_CHECK_HEADERS(sys/socket.h) fails on OS/2 EMX because it demands that
dnl <sys/types.h> be included first.
dnl
dnl Also <sys/dir.h> and <sys/dirent.h> and <sys/stat.h>, but we normally do
dnl not do our own tests via AC_CHECK_HEADERS for those.
AC_DEFUN([CF_CHECK_HEADERS],[
for cf_hdr in $1
do
	AC_MSG_CHECKING(for $cf_hdr)
	AC_TRY_CPP([
#include <sys/types.h>
#include <$cf_hdr>
],[cf_found=yes],[cf_found=no])
AC_MSG_RESULT($cf_found)
if test $cf_found = yes ; then
	CF_UPPER(cf_tr_hdr,$cf_hdr)
	AC_DEFINE_UNQUOTED(HAVE_${cf_tr_hdr})
fi
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_IPV6 version: 3 updated: 2004/01/22 17:38:22
dnl -------------
dnl Check for IPV6 configuration.
AC_DEFUN([CF_CHECK_IPV6],[
CF_FIND_IPV6_TYPE
CF_FIND_IPV6_LIBS

CF_FUNC_GETADDRINFO

if test "$cf_cv_getaddrinfo" != "yes"; then
	if test "$cf_cv_ipv6type" != "linux"; then
		AC_MSG_WARN(
[You must get working getaddrinfo() function,
or you can specify "--disable-ipv6"])
	else
		AC_MSG_WARN(
[The getaddrinfo() implementation on your system seems be buggy.
You should upgrade your system library to the newest version
of GNU C library (aka glibc).])
	fi
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_NESTED_PARAMS version: 3 updated: 2019/12/31 20:39:42
dnl ----------------------
dnl Check if the compiler allows nested parameter lists (some don't)
AC_DEFUN([CF_CHECK_NESTED_PARAMS],
[
AC_MSG_CHECKING([if nested parameters work])
AC_CACHE_VAL(cf_cv_nested_params,[
	AC_TRY_COMPILE([],
	[extern void (*sigdisp(int sig, void (*func)(int sig)))(int sig)],
	[cf_cv_nested_params=yes],
	[cf_cv_nested_params=no])
])
AC_MSG_RESULT($cf_cv_nested_params)
test $cf_cv_nested_params = yes && AC_DEFINE(HAVE_NESTED_PARAMS,1,[Define this to 1 if the compiler allows nested parameter lists])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CLANG_COMPILER version: 8 updated: 2021/01/01 13:31:04
dnl -----------------
dnl Check if the given compiler is really clang.  clang's C driver defines
dnl __GNUC__ (fooling the configure script into setting $GCC to yes) but does
dnl not ignore some gcc options.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = CLANG_COMPILER (default)
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_CLANG_COMPILER],[
ifelse([$2],,CLANG_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	AC_MSG_CHECKING(if this is really Clang ifelse([$1],GXX,C++,C) compiler)
	cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
	AC_TRY_COMPILE([],[
#ifdef __clang__
#else
make an error
#endif
],[ifelse([$2],,CLANG_COMPILER,[$2])=yes
],[])
	ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
	AC_MSG_RESULT($ifelse([$2],,CLANG_COMPILER,[$2]))
fi

CLANG_VERSION=none

if test "x$ifelse([$2],,CLANG_COMPILER,[$2])" = "xyes" ; then
	case "$CC" in
	c[[1-9]][[0-9]]|*/c[[1-9]][[0-9]])
		AC_MSG_WARN(replacing broken compiler alias $CC)
		CFLAGS="$CFLAGS -std=`echo "$CC" | sed -e 's%.*/%%'`"
		CC=clang
		;;
	esac

	AC_MSG_CHECKING(version of $CC)
	CLANG_VERSION="`$CC --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(CLANG[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$CLANG_VERSION" && CLANG_VERSION=unknown
	AC_MSG_RESULT($CLANG_VERSION)

	for cf_clang_opt in \
		-Qunused-arguments \
		-Wno-error=implicit-function-declaration
	do
		AC_MSG_CHECKING(if option $cf_clang_opt works)
		cf_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $cf_clang_opt"
		AC_TRY_LINK([
			#include <stdio.h>],[
			printf("hello!\\n");],[
			cf_clang_optok=yes],[
			cf_clang_optok=no])
		AC_MSG_RESULT($cf_clang_optok)
		CFLAGS="$cf_save_CFLAGS"
		if test "$cf_clang_optok" = yes; then
			CF_VERBOSE(adding option $cf_clang_opt)
			CF_APPEND_TEXT(CFLAGS,$cf_clang_opt)
		fi
	done
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_COLOR_CURSES version: 9 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if curses supports color.  (Note that while SVr3 curses supports
dnl color, it does this differently from SVr4 curses; more work would be needed
dnl to accommodate SVr3).
dnl
AC_DEFUN([CF_COLOR_CURSES],
[
AC_MSG_CHECKING(if curses supports color attributes)
AC_CACHE_VAL(cf_cv_color_curses,[
	AC_TRY_LINK([
#include <${cf_cv_ncurses_header:-curses.h}>
],
	[chtype x = COLOR_BLUE;
	 has_colors();
	 start_color();
#ifndef NCURSES_BROKEN
	 wbkgd(curscr, getbkgd(stdscr)); /* X/Open XPG4 aka SVr4 Curses */
#endif
	],
	[cf_cv_color_curses=yes],
	[cf_cv_color_curses=no])
	])
AC_MSG_RESULT($cf_cv_color_curses)
if test "$cf_cv_color_curses" = yes ; then
	AC_DEFINE(COLOR_CURSES,1,[Define to 1 if if curses supports color attributes])
	test ".$cf_cv_ncurses_broken" != .yes && AC_DEFINE(HAVE_GETBKGD,1,[Define to 1 if curses has getbkgd function])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_COMPTYPE version: 3 updated: 2019/12/31 20:39:42
dnl -----------
dnl Check if the compiler uses 'void *' for qsort's compare function parameters
dnl (i.e., it's an ANSI prototype).
AC_DEFUN([CF_COMPTYPE],
[
AC_MSG_CHECKING([for ANSI qsort])
AC_CACHE_VAL(cf_cv_comptype,[
	AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif],
	[extern int compare(const void *, const void *);
	 char *foo = "string";
	 qsort(foo, sizeof(foo)/sizeof(*foo), sizeof(*foo), compare)],
	[cf_cv_comptype=yes],
	[cf_cv_comptype=no])
])
AC_MSG_RESULT($cf_cv_comptype)
if test $cf_cv_comptype = yes; then
	AC_DEFINE(HAVE_COMPTYPE_VOID,1,[Define this to 1 if qsort uses void*])
else
	AC_DEFINE(HAVE_COMPTYPE_CHAR,1,[Define this to 1 if qsort uses char*])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CONST_X_STRING version: 6 updated: 2021/01/01 13:31:04
dnl -----------------
dnl The X11R4-X11R6 Xt specification uses an ambiguous String type for most
dnl character-strings.
dnl
dnl It is ambiguous because the specification accommodated the pre-ANSI
dnl compilers bundled by more than one vendor in lieu of providing a standard C
dnl compiler other than by costly add-ons.  Because of this, the specification
dnl did not take into account the use of const for telling the compiler that
dnl string literals would be in readonly memory.
dnl
dnl As a workaround, one could (starting with X11R5) define XTSTRINGDEFINES, to
dnl let the compiler decide how to represent Xt's strings which were #define'd.
dnl That does not solve the problem of using the block of Xt's strings which
dnl are compiled into the library (and is less efficient than one might want).
dnl
dnl Xt specification 7 introduces the _CONST_X_STRING symbol which is used both
dnl when compiling the library and compiling using the library, to tell the
dnl compiler that String is const.
AC_DEFUN([CF_CONST_X_STRING],
[
AC_REQUIRE([AC_PATH_XTRA])

CF_SAVE_XTRA_FLAGS([CF_CONST_X_STRING])

AC_TRY_COMPILE(
[
#include <stdlib.h>
#include <X11/Intrinsic.h>
],
[String foo = malloc(1); (void)foo],[

AC_CACHE_CHECK(for X11/Xt const-feature,cf_cv_const_x_string,[
	AC_TRY_COMPILE(
		[
#define _CONST_X_STRING	/* X11R7.8 (perhaps) */
#undef  XTSTRINGDEFINES	/* X11R5 and later */
#include <stdlib.h>
#include <X11/Intrinsic.h>
		],[String foo = malloc(1); *foo = 0],[
			cf_cv_const_x_string=no
		],[
			cf_cv_const_x_string=yes
		])
])

CF_RESTORE_XTRA_FLAGS([CF_CONST_X_STRING])

case "$cf_cv_const_x_string" in
no)
	CF_APPEND_TEXT(CPPFLAGS,-DXTSTRINGDEFINES)
	;;
*)
	CF_APPEND_TEXT(CPPFLAGS,-D_CONST_X_STRING)
	;;
esac

])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_COREFILE version: 5 updated: 2019/12/31 20:39:42
dnl -----------
dnl Check if the application can dump core (for debugging).
AC_DEFUN([CF_COREFILE],
[
AC_MSG_CHECKING([if application can dump core])
AC_CACHE_VAL(cf_cv_corefile,[
	AC_TRY_RUN([
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
int found()
{
	struct stat sb;
	return ((stat("core", &sb) == 0			/* UNIX */
	   ||    stat("conftest.core", &sb) == 0	/* FreeBSD */
		)
		&& ((sb.st_mode & S_IFMT) == S_IFREG));
}
int main()
{
#ifdef __amiga__
/* Nicholas d'Alterio (nagd@ic.ac.uk) reports that the check for ability to
 * core dump causes the machine to crash - reason unknown (gcc 2.7.2)
 */
	${cf_cv_main_return:-return}(1);
#else
	int	pid, status;
	if (found())
		unlink("core");
	if (found())
		${cf_cv_main_return:-return}(1);
	if ((pid = fork()) != 0) {
		while (wait(&status) <= 0)
			;
	} else {
		abort();	/* this will dump core, if anything will */
	}
	if (found()) {
		unlink("core");
		${cf_cv_main_return:-return}(0);
	}
	${cf_cv_main_return:-return}(1);
#endif
}],
	[cf_cv_corefile=yes],
	[cf_cv_corefile=no],
	[cf_cv_corefile=unknown])])
AC_MSG_RESULT($cf_cv_corefile)
test $cf_cv_corefile = yes && AC_DEFINE(HAVE_COREFILE,1,[Define this to 1 if the application can dump core])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CPP_CONCATS version: 3 updated: 2019/12/31 20:39:42
dnl --------------
dnl Test for ANSI token expansion (used in 'assert').
AC_DEFUN([CF_CPP_CONCATS],
[
AC_MSG_CHECKING([for ansi token concatenation])
AC_CACHE_VAL(cf_cv_cpp_concats,[
	AC_TRY_COMPILE([
#define concat(a,b) a ## b],
	[char *firstlast = "y", *s = concat(first,last)],
	[cf_cv_cpp_concats=yes],
	[cf_cv_cpp_concats=no])
])
AC_MSG_RESULT($cf_cv_cpp_concats)
test $cf_cv_cpp_concats = yes && AC_DEFINE(CPP_DOES_CONCAT,1,[Define this to 1 for ansi token concatenation])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CPP_EXPANDS version: 3 updated: 2019/12/31 20:39:42
dnl --------------
dnl Test for ANSI token expansion (used in 'assert').
AC_DEFUN([CF_CPP_EXPANDS],
[
AC_MSG_CHECKING([for ansi token expansion/substitution])
AC_CACHE_VAL(cf_cv_cpp_expands,[
	AC_TRY_COMPILE([
#define string(n) #n],
	[char *s = string(token)],
	[cf_cv_cpp_expands=yes],
	[cf_cv_cpp_expands=no])
])
AC_MSG_RESULT($cf_cv_cpp_expands)
test $cf_cv_cpp_expands = yes && AC_DEFINE(CPP_DOES_EXPAND,1,[Define this to 1 for ansi token expansion/substitution])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_CONFIG version: 2 updated: 2006/10/29 11:06:27
dnl ----------------
dnl Tie together the configure-script macros for curses.  It may be ncurses,
dnl but unless asked, we do not make a special search for ncurses.  However,
dnl still check for the ncurses version number, for use in other macros.
AC_DEFUN([CF_CURSES_CONFIG],
[
CF_CURSES_CPPFLAGS
CF_NCURSES_VERSION
CF_CURSES_LIBS
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_CPPFLAGS version: 14 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Look for the curses headers.
AC_DEFUN([CF_CURSES_CPPFLAGS],[

AC_CACHE_CHECK(for extra include directories,cf_cv_curses_incdir,[
cf_cv_curses_incdir=no
case "$host_os" in
hpux10.*)
	if test "x$cf_cv_screen" = "xcurses_colr"
	then
		test -d /usr/include/curses_colr && \
		cf_cv_curses_incdir="-I/usr/include/curses_colr"
	fi
	;;
sunos3*|sunos4*)
	if test "x$cf_cv_screen" = "xcurses_5lib"
	then
		test -d /usr/5lib && \
		test -d /usr/5include && \
		cf_cv_curses_incdir="-I/usr/5include"
	fi
	;;
esac
])
if test "$cf_cv_curses_incdir" != no
then
	CF_APPEND_TEXT(CPPFLAGS,$cf_cv_curses_incdir)
fi

CF_CURSES_HEADER
CF_TERM_HEADER
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_FUNCS version: 20 updated: 2020/12/31 20:19:42
dnl ---------------
dnl Curses-functions are a little complicated, since a lot of them are macros.
AC_DEFUN([CF_CURSES_FUNCS],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_REQUIRE([CF_XOPEN_CURSES])
AC_REQUIRE([CF_CURSES_TERM_H])
AC_REQUIRE([CF_CURSES_UNCTRL_H])
for cf_func in $1
do
	CF_UPPER(cf_tr_func,$cf_func)
	AC_MSG_CHECKING(for ${cf_func})
	CF_MSG_LOG(${cf_func})
	AC_CACHE_VAL(cf_cv_func_$cf_func,[
		eval cf_result='$ac_cv_func_'$cf_func
		if test ".$cf_result" != ".no"; then
			AC_TRY_LINK(CF__CURSES_HEAD,
			[
#ifndef ${cf_func}
long foo = (long)(&${cf_func});
fprintf(stderr, "testing linkage of $cf_func:%p\\n", (void *)foo);
if (foo + 1234L > 5678L)
	${cf_cv_main_return:-return}(foo != 0);
#endif
			],
			[cf_result=yes],
			[cf_result=no])
		fi
		eval 'cf_cv_func_'$cf_func'="$cf_result"'
	])
	# use the computed/retrieved cache-value:
	eval 'cf_result=$cf_cv_func_'$cf_func
	AC_MSG_RESULT($cf_result)
	if test "$cf_result" != no; then
		AC_DEFINE_UNQUOTED(HAVE_${cf_tr_func})
	fi
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_HEADER version: 5 updated: 2015/04/23 20:35:30
dnl ----------------
dnl Find a "curses" header file, e.g,. "curses.h", or one of the more common
dnl variations of ncurses' installs.
dnl
dnl $1 = ncurses when looking for ncurses, or is empty
AC_DEFUN([CF_CURSES_HEADER],[
AC_CACHE_CHECK(if we have identified curses headers,cf_cv_ncurses_header,[
cf_cv_ncurses_header=none
for cf_header in \
	ncurses.h ifelse($1,,,[$1/ncurses.h]) \
	curses.h ifelse($1,,,[$1/curses.h]) ifelse($1,,[ncurses/ncurses.h ncurses/curses.h])
do
AC_TRY_COMPILE([#include <${cf_header}>],
	[initscr(); tgoto("?", 0,0)],
	[cf_cv_ncurses_header=$cf_header; break],[])
done
])

if test "$cf_cv_ncurses_header" = none ; then
	AC_MSG_ERROR(No curses header-files found)
fi

# cheat, to get the right #define's for HAVE_NCURSES_H, etc.
AC_CHECK_HEADERS($cf_cv_ncurses_header)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_LIBS version: 44 updated: 2021/01/02 09:31:20
dnl --------------
dnl Look for the curses libraries.  Older curses implementations may require
dnl termcap/termlib to be linked as well.  Call CF_CURSES_CPPFLAGS first.
AC_DEFUN([CF_CURSES_LIBS],[

AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_MSG_CHECKING(if we have identified curses libraries)
AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
	[initscr(); tgoto("?", 0,0)],
	cf_result=yes,
	cf_result=no)
AC_MSG_RESULT($cf_result)

if test "$cf_result" = no ; then
case "$host_os" in
freebsd*)
	AC_CHECK_LIB(mytinfo,tgoto,[CF_ADD_LIBS(-lmytinfo)])
	;;
hpux10.*)
	# Looking at HPUX 10.20, the Hcurses library is the oldest (1997), cur_colr
	# next (1998), and xcurses "newer" (2000).  There is no header file for
	# Hcurses; the subdirectory curses_colr has the headers (curses.h and
	# term.h) for cur_colr
	if test "x$cf_cv_screen" = "xcurses_colr"
	then
		AC_CHECK_LIB(cur_colr,initscr,[
			CF_ADD_LIBS(-lcur_colr)
			ac_cv_func_initscr=yes
			],[
		AC_CHECK_LIB(Hcurses,initscr,[
			# HP's header uses __HP_CURSES, but user claims _HP_CURSES.
			CF_ADD_LIBS(-lHcurses)
			CF_APPEND_TEXT(CPPFLAGS,-D__HP_CURSES -D_HP_CURSES)
			ac_cv_func_initscr=yes
			])])
	fi
	;;
linux*)
	case `arch 2>/dev/null` in
	x86_64)
		if test -d /lib64
		then
			CF_ADD_LIBDIR(/lib64)
		else
			CF_ADD_LIBDIR(/lib)
		fi
		;;
	*)
		CF_ADD_LIBDIR(/lib)
		;;
	esac
	;;
sunos3*|sunos4*)
	if test "x$cf_cv_screen" = "xcurses_5lib"
	then
		if test -d /usr/5lib ; then
			CF_ADD_LIBDIR(/usr/5lib)
			CF_ADD_LIBS(-lcurses -ltermcap)
		fi
	fi
	ac_cv_func_initscr=yes
	;;
esac

if test ".$ac_cv_func_initscr" != .yes ; then
	cf_save_LIBS="$LIBS"

	if test ".${cf_cv_ncurses_version:-no}" != .no
	then
		cf_check_list="ncurses curses cursesX"
	else
		cf_check_list="cursesX curses ncurses"
	fi

	# Check for library containing tgoto.  Do this before curses library
	# because it may be needed to link the test-case for initscr.
	if test "x$cf_term_lib" = x
	then
		AC_CHECK_FUNC(tgoto,[cf_term_lib=predefined],[
			for cf_term_lib in $cf_check_list otermcap termcap tinfo termlib unknown
			do
				AC_CHECK_LIB($cf_term_lib,tgoto,[
					: "${cf_nculib_root:=$cf_term_lib}"
					break
				])
			done
		])
	fi

	# Check for library containing initscr
	test "$cf_term_lib" != predefined && test "$cf_term_lib" != unknown && LIBS="-l$cf_term_lib $cf_save_LIBS"
	if test "x$cf_curs_lib" = x
	then
		for cf_curs_lib in $cf_check_list xcurses jcurses pdcurses unknown
		do
			LIBS="-l$cf_curs_lib $cf_save_LIBS"
			if test "$cf_term_lib" = unknown || test "$cf_term_lib" = "$cf_curs_lib" ; then
				AC_MSG_CHECKING(if we can link with $cf_curs_lib library)
				AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
					[initscr()],
					[cf_result=yes],
					[cf_result=no])
				AC_MSG_RESULT($cf_result)
				test "$cf_result" = yes && break
			elif test "$cf_curs_lib" = "$cf_term_lib" ; then
				cf_result=no
			elif test "$cf_term_lib" != predefined ; then
				AC_MSG_CHECKING(if we need both $cf_curs_lib and $cf_term_lib libraries)
				AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
					[initscr(); tgoto((char *)0, 0, 0);],
					[cf_result=no],
					[
					LIBS="-l$cf_curs_lib -l$cf_term_lib $cf_save_LIBS"
					AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
						[initscr()],
						[cf_result=yes],
						[cf_result=error])
					])
				AC_MSG_RESULT($cf_result)
				test "$cf_result" != error && break
			fi
		done
	fi
	test "$cf_curs_lib" = unknown && AC_MSG_ERROR(no curses library found)
fi
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_TERMCAP version: 12 updated: 2015/05/15 19:42:24
dnl -----------------
dnl Check if we should include <curses.h> to pick up prototypes for termcap
dnl functions.  On terminfo systems, these are normally declared in <curses.h>,
dnl but may be in <term.h>.  We check for termcap.h as an alternate, but it
dnl isn't standard (usually associated with GNU termcap).
dnl
dnl The 'tgoto()' function is declared in both terminfo and termcap.
dnl
dnl See CF_TYPE_OUTCHAR for more details.
AC_DEFUN([CF_CURSES_TERMCAP],
[
AC_REQUIRE([CF_CURSES_TERM_H])
AC_CACHE_CHECK(if we should include curses.h or termcap.h, cf_cv_need_curses_h,[
cf_save_CPPFLAGS="$CPPFLAGS"
cf_cv_need_curses_h=no

for cf_t_opts in "" "NEED_TERMCAP_H"
do
for cf_c_opts in "" "NEED_CURSES_H"
do

    CPPFLAGS="$cf_save_CPPFLAGS $CHECK_DECL_FLAG"
    test -n "$cf_c_opts" && CPPFLAGS="$CPPFLAGS -D$cf_c_opts"
    test -n "$cf_t_opts" && CPPFLAGS="$CPPFLAGS -D$cf_t_opts"

    AC_TRY_LINK([/* $cf_c_opts $cf_t_opts */
$CHECK_DECL_HDRS],
	[char *x = (char *)tgoto("")],
	[test "$cf_cv_need_curses_h" = no && {
	     cf_cv_need_curses_h=maybe
	     cf_ok_c_opts=$cf_c_opts
	     cf_ok_t_opts=$cf_t_opts
	}],
	[echo "Recompiling with corrected call (C:$cf_c_opts, T:$cf_t_opts)" >&AC_FD_CC
	AC_TRY_LINK([
$CHECK_DECL_HDRS],
	[char *x = (char *)tgoto("",0,0)],
	[cf_cv_need_curses_h=yes
	 cf_ok_c_opts=$cf_c_opts
	 cf_ok_t_opts=$cf_t_opts])])

	CPPFLAGS="$cf_save_CPPFLAGS"
	test "$cf_cv_need_curses_h" = yes && break
done
	test "$cf_cv_need_curses_h" = yes && break
done

if test "$cf_cv_need_curses_h" != no ; then
	echo "Curses/termcap test = $cf_cv_need_curses_h (C:$cf_ok_c_opts, T:$cf_ok_t_opts)" >&AC_FD_CC
	if test -n "$cf_ok_c_opts" ; then
		if test -n "$cf_ok_t_opts" ; then
			cf_cv_need_curses_h=both
		else
			cf_cv_need_curses_h=curses.h
		fi
	elif test -n "$cf_ok_t_opts" ; then
		cf_cv_need_curses_h=termcap.h
	elif test "$cf_cv_term_header" != no ; then
		cf_cv_need_curses_h=term.h
	else
		cf_cv_need_curses_h=no
	fi
fi
])

case $cf_cv_need_curses_h in
both)
	AC_DEFINE_UNQUOTED(NEED_CURSES_H,1,[Define to 1 if we must include curses.h])
	AC_DEFINE_UNQUOTED(NEED_TERMCAP_H,1,[Define to 1 if we must include termcap.h])
	;;
curses.h)
	AC_DEFINE_UNQUOTED(NEED_CURSES_H,1,[Define to 1 if we must include curses.h])
	;;
term.h)
	AC_DEFINE_UNQUOTED(NEED_TERM_H,1,[Define to 1 if we must include term.h])
	;;
termcap.h)
	AC_DEFINE_UNQUOTED(NEED_TERMCAP_H,1,[Define to 1 if we must include termcap.h])
	;;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_TERM_H version: 15 updated: 2021/01/02 09:31:20
dnl ----------------
dnl SVr4 curses should have term.h as well (where it puts the definitions of
dnl the low-level interface).  This may not be true in old/broken implementations,
dnl as well as in misconfigured systems (e.g., gcc configured for Solaris 2.4
dnl running with Solaris 2.5.1).
AC_DEFUN([CF_CURSES_TERM_H],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl

AC_CACHE_CHECK(for term.h, cf_cv_term_header,[

# If we found <ncurses/curses.h>, look for <ncurses/term.h>, but always look
# for <term.h> if we do not find the variant.

cf_header_list="term.h ncurses/term.h ncursesw/term.h"

case "${cf_cv_ncurses_header:-curses.h}" in
*/*)
	cf_header_item=`echo "${cf_cv_ncurses_header:-curses.h}" | sed -e 's%\..*%%' -e 's%/.*%/%'`term.h
	cf_header_list="$cf_header_item $cf_header_list"
	;;
esac

for cf_header in $cf_header_list
do
	AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <${cf_header}>],
	[WINDOW *x; (void)x],
	[cf_cv_term_header=$cf_header
	 break],
	[cf_cv_term_header=no])
done

case "$cf_cv_term_header" in
no)
	# If curses is ncurses, some packagers still mess it up by trying to make
	# us use GNU termcap.  This handles the most common case.
	for cf_header in ncurses/term.h ncursesw/term.h
	do
		AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#ifdef NCURSES_VERSION
#include <${cf_header}>
#else
make an error
#endif],
			[WINDOW *x; (void)x],
			[cf_cv_term_header=$cf_header
			 break],
			[cf_cv_term_header=no])
	done
	;;
esac
])

case "$cf_cv_term_header" in
term.h)
	AC_DEFINE(HAVE_TERM_H,1,[Define to 1 if we have term.h])
	;;
ncurses/term.h)
	AC_DEFINE(HAVE_NCURSES_TERM_H,1,[Define to 1 if we have ncurses/term.h])
	;;
ncursesw/term.h)
	AC_DEFINE(HAVE_NCURSESW_TERM_H,1,[Define to 1 if we have ncursesw/term.h])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CURSES_UNCTRL_H version: 8 updated: 2021/01/02 09:31:20
dnl ------------------
dnl Any X/Open curses implementation must have unctrl.h, but ncurses packages
dnl may put it in a subdirectory (along with ncurses' other headers, of
dnl course).  Packages which put the headers in inconsistent locations are
dnl broken).
AC_DEFUN([CF_CURSES_UNCTRL_H],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl

AC_CACHE_CHECK(for unctrl.h, cf_cv_unctrl_header,[

# If we found <ncurses/curses.h>, look for <ncurses/unctrl.h>, but always look
# for <unctrl.h> if we do not find the variant.

cf_header_list="unctrl.h ncurses/unctrl.h ncursesw/unctrl.h"

case "${cf_cv_ncurses_header:-curses.h}" in
*/*)
	cf_header_item=`echo "${cf_cv_ncurses_header:-curses.h}" | sed -e 's%\..*%%' -e 's%/.*%/%'`unctrl.h
	cf_header_list="$cf_header_item $cf_header_list"
	;;
esac

for cf_header in $cf_header_list
do
	AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <${cf_header}>],
	[WINDOW *x; (void)x],
	[cf_cv_unctrl_header=$cf_header
	 break],
	[cf_cv_unctrl_header=no])
done
])

case "$cf_cv_unctrl_header" in
no)
	AC_MSG_WARN(unctrl.h header not found)
	;;
esac

case "$cf_cv_unctrl_header" in
unctrl.h)
	AC_DEFINE(HAVE_UNCTRL_H,1,[Define to 1 if we have unctrl.h])
	;;
ncurses/unctrl.h)
	AC_DEFINE(HAVE_NCURSES_UNCTRL_H,1,[Define to 1 if we have ncurses/unctrl.h])
	;;
ncursesw/unctrl.h)
	AC_DEFINE(HAVE_NCURSESW_UNCTRL_H,1,[Define to 1 if we have ncursesw/unctrl.h])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DEFAULT_SHELL version: 7 updated: 2019/12/31 20:39:42
dnl ----------------
dnl Look for a Bourne-shell compatible program from a list that we know about:
dnl	ash	Almquist Shell (sh based)
dnl	bash	Bourne Again Shell (sh, ksh based)
dnl	dash	Debian Almquist Shell (sh based)
dnl	jsh	Job Control Bourne Shell (sh based)
dnl	keysh	Key Shell (ksh based)
dnl	ksh	Korn Shell (sh based)
dnl	mksh	MirBSD Korn shell (pdksh based)
dnl	pdksh	Public-domain ksh
dnl	sh	Bourne Shell or POSIX Shell
dnl	zsh	Z Shell (sh, ksh based)
dnl On BSD systems look for a C Shell compatible program:
dnl	csh	C Shell
dnl	tcsh	TENEX C Shell (csh based)
AC_DEFUN([CF_DEFAULT_SHELL],
[
AC_MSG_CHECKING(for the default shell program)
cf_shell_progs="ifelse($1,,sh,[$1])"
if test -z "$cf_shell_progs" ; then
	cf_shell_progs="sh ksh bash zsh pdksh mksh jsh keysh ash dash"
	# TIN preferred default shell for BSD systems is csh. Others are sh.
	AC_TRY_COMPILE([
#include <sys/params.h>],[
#if (defined(BSD) && (BSD >= 199103))
#else
make an error
#endif
],[$cf_shell_progs="csh tcsh $cf_shell_progs"])
fi
CF_MSG_LOG(paths of shell programs: $cf_shell_progs)
if test -s /etc/shells && test `egrep -c -v '^(#| |    |$)' /etc/shells` -gt 0; then
	CF_MSG_LOG(/etc/shells)
	for cf_prog in $cf_shell_progs
	do
		case $cf_prog in
			/*)
				cf_pattern="^"$cf_prog"$"
				;;
			*/*)
				AC_MSG_ERROR(Program name must be absolute or filename: $cf_prog)
				;;
			*)
				cf_pattern="/"$cf_prog"$"
				;;
		esac
		cf_path=`egrep $cf_pattern /etc/shells 2>/dev/null`
		if test -n "$cf_path"
		then
			for cf_shell in $cf_path
			do
				if test -f "$cf_shell"
				then
					DEFAULT_SHELL="$cf_shell"
					break
				fi
			done
		fi
		if test -n "$DEFAULT_SHELL"
		then
			break
		fi
	done
	AC_MSG_RESULT($DEFAULT_SHELL)
else
	CF_MSG_LOG($PATH)
AC_PATH_PROGS(DEFAULT_SHELL,
	$cf_shell_progs,,
	$PATH:/bin:/usr/bin:/usr/xpg4/bin:/bin/posix:/usr/bin/posix:/usr/old/bin:/usr/local/bin)
fi
if test -z "$DEFAULT_SHELL" ; then
	AC_MSG_WARN(
Cannot find the default shell you specified: $cf_shell_progs)
	if test -f /bin/false ; then
		AC_MSG_WARN(Using /bin/false instead)
		DEFAULT_SHELL=/bin/false
	else
		AC_MSG_ERROR(Cannot use /bin/false because it does not exist)
	fi
fi
AC_DEFINE_UNQUOTED(DEFAULT_SHELL,"$DEFAULT_SHELL",[Define this to the default shell-program])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DEFINE_STRING version: 3 updated: 2019/12/31 20:39:42
dnl ----------------
dnl Define a string which may contain escaped quotes or backslashes
dnl $1 = symbol to define
dnl $2 = the information we want to quote
AC_DEFUN([CF_DEFINE_STRING],
[
cf_define=`echo $2|sed -e 's/\\\\/\\\\134/g' -e 's/^[[ 	]]\\+//' -e 's/[[ 	]]\\+$//' -e 's/"/\\\\042/g'`
AC_DEFINE_UNQUOTED($1, "$cf_define", [Define a value for $1])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DIRNAME version: 5 updated: 2020/12/31 20:19:42
dnl ----------
dnl "dirname" is not portable, so we fake it with a shell script.
AC_DEFUN([CF_DIRNAME],[$1=`echo "$2" | sed -e 's%/[[^/]]*$%%'`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 13 updated: 2015/04/18 08:56:57
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          do not display "compiling" commands],
	[
	ECHO_LT='--silent'
	ECHO_LD='@echo linking [$]@;'
	RULE_CC='@echo compiling [$]<'
	SHOW_CC='@echo compiling [$]@'
	ECHO_CC='@'
],[
	ECHO_LT=''
	ECHO_LD=''
	RULE_CC=''
	SHOW_CC=''
	ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ERRNO version: 5 updated: 1997/11/30 12:44:39
dnl --------
dnl Check if 'errno' is declared in <errno.h>
AC_DEFUN([CF_ERRNO],
[
CF_CHECK_ERRNO(errno)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_CANLOCK version: 1 updated: 2020/01/09 20:03:25
dnl ---------------
dnl Look for CANLOCK, to use instead of a bundled copy of CANLOCK.
dnl
dnl $1 = variable to set with CANLOCK's CPPFLAGS
dnl $2 = variable to set with CANLOCK's LIBS
dnl $3 = action to take on success
dnl $4 = action to take on failure
AC_DEFUN([CF_FIND_CANLOCK],[
AC_REQUIRE([CF_PKG_CONFIG])

cf_save_CFLAGS="$CFLAGS"
cf_save_LIBS="$LIBS"
cf_find_CANLOCK=yes

CF_TRY_PKG_CONFIG(libcanlock3,,[
	CF_TRY_PKG_CONFIG(libcanlock,,[
		cf_pkgconfig_incs=
		cf_pkgconfig_libs=
		for cf_canlock_lib in canlock3 canlock
		do
			AC_CHECK_LIB($cf_canlock_lib,cl_clear_secret,[
				cf_pkgconfig_libs="-l$cf_canlock_lib"
				break
			])
		done
		if test -z "$cf_pkgconfig_libs" ; then
			cf_find_CANLOCK=no
		else
			AC_CHECK_HEADERS(libcanlock-3/canlock.h canlock.h)
		fi
		])
	])

CFLAGS="$cf_save_CFLAGS"
LIBS="$cf_save_LIBS"

$1="$cf_pkgconfig_incs"
$2="$cf_pkgconfig_libs"

if test "$cf_find_CANLOCK" = yes; then
	$3
else
	$4
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_IPV6_LIBS version: 9 updated: 2021/01/02 09:31:20
dnl -----------------
dnl Based on the IPV6 stack type, look for the corresponding library.
AC_DEFUN([CF_FIND_IPV6_LIBS],[
AC_REQUIRE([CF_FIND_IPV6_TYPE])

cf_ipv6lib=none
cf_ipv6dir=none

AC_MSG_CHECKING(for IPv6 library if required)
case "$cf_cv_ipv6type" in
solaris)
	;;
inria)
	;;
kame)
	dnl http://www.kame.net/
	cf_ipv6lib=inet6
	cf_ipv6dir=v6
	;;
linux-glibc)
	;;
linux-libinet6)
	dnl http://www.v6.linux.or.jp/
	cf_ipv6lib=inet6
	cf_ipv6dir=inet6
	;;
toshiba)
	cf_ipv6lib=inet6
	cf_ipv6dir=v6
	;;
v6d)
	cf_ipv6lib=v6
	cf_ipv6dir=v6
	;;
zeta)
	cf_ipv6lib=inet6
	cf_ipv6dir=v6
	;;
esac
AC_MSG_RESULT($cf_ipv6lib)

if test "$cf_ipv6lib" != "none"; then

	AC_TRY_LINK([
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip6.h>],
	[getaddrinfo(0, 0, 0, 0)],,[
	CF_HEADER_PATH(cf_search,$cf_ipv6dir)
	for cf_incdir in $cf_search
	do
		cf_header=$cf_incdir/netinet/ip6.h
		if test -f "$cf_header"
		then
			CF_ADD_INCDIR($cf_incdir)
			test -n "$verbose" && echo "	... found $cf_header" 1>&AC_FD_MSG
			break
		fi
		test -n "$verbose" && echo "	... tested $cf_header" 1>&AC_FD_MSG
	done
	])

	CF_FIND_LIBRARY([$cf_ipv6lib],[$cf_ipv6dir],[
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip6.h>],
	[getaddrinfo(0, 0, 0, 0)],
	[getaddrinfo],
	noexit)
	if test "$cf_found_library" = no ; then
		AC_MSG_ERROR(
[No $cf_ipv6lib library found, cannot continue.  You must fetch lib$cf_ipv6lib.a
from an appropriate IPv6 kit and compile beforehand.])
	fi
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_IPV6_TYPE version: 7 updated: 2021/01/02 09:31:20
dnl -----------------
AC_DEFUN([CF_FIND_IPV6_TYPE],[
AC_CACHE_CHECK(ipv6 stack type, cf_cv_ipv6type, [
cf_cv_ipv6type=unknown
for i in solaris inria kame linux-glibc linux-libinet6 toshiba v6d zeta
do
	case "$i" in
	solaris)
		if test "SunOS" = "`uname -s`"
		then
		  if test -f /usr/include/netinet/ip6.h
		  then
			cf_cv_ipv6type=$i
		  fi
		fi
		;;
	inria)
		dnl http://www.kame.net/
		AC_EGREP_CPP(yes, [
#include <netinet/in.h>
#ifdef IPV6_INRIA_VERSION
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	kame)
		dnl http://www.kame.net/
		AC_EGREP_CPP(yes, [
#include <netinet/in.h>
#ifdef __KAME__
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	linux-glibc)
		dnl http://www.v6.linux.or.jp/
		AC_EGREP_CPP(yes, [
#include <features.h>
#if defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	linux-libinet6)
		dnl http://www.v6.linux.or.jp/
		if test -d /usr/inet6
		then
			cf_cv_ipv6type=$i
		elif test -f /usr/include/netinet/ip6.h
		then
			cf_cv_ipv6type=$i
		fi
		;;
	toshiba)
		AC_EGREP_CPP(yes, [
#include <sys/param.h>
#ifdef _TOSHIBA_INET6
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	v6d)
		AC_EGREP_CPP(yes, [
#include </usr/local/v6/include/sys/v6config.h>
#ifdef __V6D__
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	zeta)
		AC_EGREP_CPP(yes, [
#include <sys/param.h>
#ifdef _ZETA_MINAMI_INET6
yes
#endif],	[cf_cv_ipv6type=$i])
		;;
	esac
	if test "$cf_cv_ipv6type" != "unknown"; then
		break
	fi
done
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LIBRARY version: 11 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Look for a non-standard library, given parameters for AC_TRY_LINK.  We
dnl prefer a standard location, and use -L options only if we do not find the
dnl library in the standard library location(s).
dnl	$1 = library name
dnl	$2 = library class, usually the same as library name
dnl	$3 = includes
dnl	$4 = code fragment to compile/link
dnl	$5 = corresponding function-name
dnl	$6 = flag, nonnull if failure should not cause an error-exit
dnl
dnl Sets the variable "$cf_libdir" as a side-effect, so we can see if we had
dnl to use a -L option.
AC_DEFUN([CF_FIND_LIBRARY],
[
	eval 'cf_cv_have_lib_'"$1"'=no'
	cf_libdir=""
	AC_CHECK_FUNC($5,
		eval 'cf_cv_have_lib_'"$1"'=yes',[
		cf_save_LIBS="$LIBS"
		AC_MSG_CHECKING(for $5 in -l$1)
		LIBS="-l$1 $LIBS"
		AC_TRY_LINK([$3],[$4],
			[AC_MSG_RESULT(yes)
			 eval 'cf_cv_have_lib_'"$1"'=yes'
			],
			[AC_MSG_RESULT(no)
			CF_LIBRARY_PATH(cf_search,$2)
			for cf_libdir in $cf_search
			do
				AC_MSG_CHECKING(for -l$1 in $cf_libdir)
				LIBS="-L$cf_libdir -l$1 $cf_save_LIBS"
				AC_TRY_LINK([$3],[$4],
					[AC_MSG_RESULT(yes)
			 		 eval 'cf_cv_have_lib_'"$1"'=yes'
					 break],
					[AC_MSG_RESULT(no)
					 LIBS="$cf_save_LIBS"])
			done
			])
		])
eval 'cf_found_library="[$]cf_cv_have_lib_'"$1"\"
ifelse($6,,[
if test "$cf_found_library" = no ; then
	AC_MSG_ERROR(Cannot link $1 library)
fi
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LINKAGE version: 22 updated: 2020/12/31 20:19:42
dnl ---------------
dnl Find a library (specifically the linkage used in the code fragment),
dnl searching for it if it is not already in the library path.
dnl See also CF_ADD_SEARCHPATH.
dnl
dnl Parameters (4-on are optional):
dnl     $1 = headers for library entrypoint
dnl     $2 = code fragment for library entrypoint
dnl     $3 = the library name without the "-l" option or ".so" suffix.
dnl     $4 = action to perform if successful (default: update CPPFLAGS, etc)
dnl     $5 = action to perform if not successful
dnl     $6 = module name, if not the same as the library name
dnl     $7 = extra libraries
dnl
dnl Sets these variables:
dnl     $cf_cv_find_linkage_$3 - yes/no according to whether linkage is found
dnl     $cf_cv_header_path_$3 - include-directory if needed
dnl     $cf_cv_library_path_$3 - library-directory if needed
dnl     $cf_cv_library_file_$3 - library-file if needed, e.g., -l$3
AC_DEFUN([CF_FIND_LINKAGE],[

# If the linkage is not already in the $CPPFLAGS/$LDFLAGS configuration, these
# will be set on completion of the AC_TRY_LINK below.
cf_cv_header_path_$3=
cf_cv_library_path_$3=

CF_MSG_LOG([Starting [FIND_LINKAGE]($3,$6)])

cf_save_LIBS="$LIBS"

AC_TRY_LINK([$1],[$2],[
	cf_cv_find_linkage_$3=yes
	cf_cv_header_path_$3=/usr/include
	cf_cv_library_path_$3=/usr/lib
],[

LIBS="-l$3 $7 $cf_save_LIBS"

AC_TRY_LINK([$1],[$2],[
	cf_cv_find_linkage_$3=yes
	cf_cv_header_path_$3=/usr/include
	cf_cv_library_path_$3=/usr/lib
	cf_cv_library_file_$3="-l$3"
],[
	cf_cv_find_linkage_$3=no
	LIBS="$cf_save_LIBS"

	CF_VERBOSE(find linkage for $3 library)
	CF_MSG_LOG([Searching for headers in [FIND_LINKAGE]($3,$6)])

	cf_save_CPPFLAGS="$CPPFLAGS"
	cf_test_CPPFLAGS="$CPPFLAGS"

	CF_HEADER_PATH(cf_search,ifelse([$6],,[$3],[$6]))
	for cf_cv_header_path_$3 in $cf_search
	do
		if test -d "$cf_cv_header_path_$3" ; then
			CF_VERBOSE(... testing $cf_cv_header_path_$3)
			CPPFLAGS="$cf_save_CPPFLAGS"
			CF_APPEND_TEXT(CPPFLAGS,-I$cf_cv_header_path_$3)
			AC_TRY_COMPILE([$1],[$2],[
				CF_VERBOSE(... found $3 headers in $cf_cv_header_path_$3)
				cf_cv_find_linkage_$3=maybe
				cf_test_CPPFLAGS="$CPPFLAGS"
				break],[
				CPPFLAGS="$cf_save_CPPFLAGS"
				])
		fi
	done

	if test "$cf_cv_find_linkage_$3" = maybe ; then

		CF_MSG_LOG([Searching for $3 library in [FIND_LINKAGE]($3,$6)])

		cf_save_LIBS="$LIBS"
		cf_save_LDFLAGS="$LDFLAGS"

		ifelse([$6],,,[
		CPPFLAGS="$cf_test_CPPFLAGS"
		LIBS="-l$3 $7 $cf_save_LIBS"
		AC_TRY_LINK([$1],[$2],[
			CF_VERBOSE(... found $3 library in system)
			cf_cv_find_linkage_$3=yes])
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"
			])

		if test "$cf_cv_find_linkage_$3" != yes ; then
			CF_LIBRARY_PATH(cf_search,$3)
			for cf_cv_library_path_$3 in $cf_search
			do
				if test -d "$cf_cv_library_path_$3" ; then
					CF_VERBOSE(... testing $cf_cv_library_path_$3)
					CPPFLAGS="$cf_test_CPPFLAGS"
					LIBS="-l$3 $7 $cf_save_LIBS"
					LDFLAGS="$cf_save_LDFLAGS -L$cf_cv_library_path_$3"
					AC_TRY_LINK([$1],[$2],[
					CF_VERBOSE(... found $3 library in $cf_cv_library_path_$3)
					cf_cv_find_linkage_$3=yes
					cf_cv_library_file_$3="-l$3"
					break],[
					CPPFLAGS="$cf_save_CPPFLAGS"
					LIBS="$cf_save_LIBS"
					LDFLAGS="$cf_save_LDFLAGS"
					])
				fi
			done
			CPPFLAGS="$cf_save_CPPFLAGS"
			LDFLAGS="$cf_save_LDFLAGS"
		fi

	else
		cf_cv_find_linkage_$3=no
	fi
	],$7)
])

LIBS="$cf_save_LIBS"

if test "$cf_cv_find_linkage_$3" = yes ; then
ifelse([$4],,[
	CF_ADD_INCDIR($cf_cv_header_path_$3)
	CF_ADD_LIBDIR($cf_cv_library_path_$3)
	CF_ADD_LIB($3)
],[$4])
else
ifelse([$5],,AC_MSG_WARN(Cannot find $3 library),[$5])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIND_PCRE version: 1 updated: 2020/01/01 16:29:19
dnl ------------
dnl Look for PCRE, to use instead of a bundled copy of PCRE.
dnl
dnl $1 = variable to set with PCRE's CPPFLAGS
dnl $2 = variable to set with PCRE's LIBS
dnl $3 = action to take on success
dnl $4 = action to take on failure
AC_DEFUN([CF_FIND_PCRE],[
AC_REQUIRE([CF_PKG_CONFIG])

cf_save_CFLAGS="$CFLAGS"
cf_save_LIBS="$LIBS"
cf_find_PCRE=yes

CF_TRY_PKG_CONFIG(libpcre2,,[
	CF_TRY_PKG_CONFIG(libpcre,,[
		AC_CHECK_LIB(pcre,pcre_compile,[
			CF_ADD_LIB(pcre)
			AC_CHECK_LIB(pcre2-posix,regcomp,[
				CF_ADD_LIB(pcre2-posix)],[
					AC_CHECK_LIB(pcreposix,regcomp,[
						CF_ADD_LIB(pcreposix)
					],[
						cf_find_PCRE=no
					])
				])
			],[
				cf_find_PCRE=no
			])
		])
	])

CFLAGS="$cf_save_CFLAGS"
LIBS="$cf_save_LIBS"

$1="$cf_pkgconfig_incs"
$2="$cf_pkgconfig_libs"

if test "$cf_find_PCRE" = yes; then
	$3
else
	$4
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FIX_SLASHES version: 2 updated: 2001/05/27 21:36:02
dnl --------------
dnl OS/2 and Cygwin ports may pick up backslashes in pathnames, which are not
dnl usable in quoted strings.  Fix them.
dnl	$1=fixed($2)
AC_DEFUN([CF_FIX_SLASHES],
[
case $cf_cv_system_name in #(vi
os2*|cygwin*)
	$1=`echo "[$]$1" | sed -e 's%\\\\%/%g'`
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_ALLOCA version: 2 updated: 2009/01/06 19:35:17
dnl --------------
dnl workaround for bison 1.875c (compound breakage in Linux stdlib.h and
dnl bison's output make bison try to use alloca()).
AC_DEFUN([CF_FUNC_ALLOCA],
[
AC_FUNC_ALLOCA

case $host_os in
linux*|gnu*)
	# workaround for bison 1.875c (compound breakage in Linux stdlib.h
	# and bison's output make bison try to use alloca()).
	if test -z "$GCC" ; then
		CPPFLAGS="$CPPFLAGS -DYYSTACK_USE_ALLOCA=0"
		ALLOCA=""
	elif test "$INTEL_COMPILER" = yes ; then
		CPPFLAGS="$CPPFLAGS -DYYSTACK_USE_ALLOCA=0"
		ALLOCA=""
	fi
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_FORK version: 4 updated: 2010/10/23 15:52:32
dnl ------------
dnl Check if 'fork()' is available, and working.  Amiga (and possibly other
dnl machines) have a non-working 'fork()' entrypoint.
AC_DEFUN([CF_FUNC_FORK],
[AC_MSG_CHECKING([for fork])
AC_CACHE_VAL(cf_cv_func_fork,[
AC_TRY_RUN([
int main()
{
	if (fork() < 0)
		${cf_cv_main_return:-return}(1);
	${cf_cv_main_return:-return}(0);
}],	[cf_cv_func_fork=yes],
	[cf_cv_func_fork=no],
	[cf_cv_func_fork=unknown])
])dnl
AC_MSG_RESULT($cf_cv_func_fork)
test $cf_cv_func_fork = yes && AC_DEFINE(HAVE_FORK)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_GETADDRINFO version: 9 updated: 2017/05/10 18:31:29
dnl -------------------
dnl Look for a working version of getaddrinfo(), for IPV6 support.
AC_DEFUN([CF_FUNC_GETADDRINFO],[
AC_CACHE_CHECK(working getaddrinfo, cf_cv_getaddrinfo,[
AC_TRY_RUN([
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define expect(a,b) if (strcmp(a,b) != 0) goto bad

int main(void)
{
   int passive, gaierr, inet4 = 0, inet6 = 0;
   struct addrinfo hints, *ai, *aitop;
   char straddr[INET6_ADDRSTRLEN], strport[16];

   for (passive = 0; passive <= 1; passive++) {
     memset(&hints, 0, sizeof(hints));
     hints.ai_family = AF_UNSPEC;
     hints.ai_flags = passive ? AI_PASSIVE : 0;
     hints.ai_socktype = SOCK_STREAM;
     if ((gaierr = getaddrinfo(NULL, "54321", &hints, &aitop)) != 0) {
       (void)gai_strerror(gaierr);
       goto bad;
     }
     for (ai = aitop; ai; ai = ai->ai_next) {
       if (ai->ai_addr == NULL ||
           ai->ai_addrlen == 0 ||
           getnameinfo(ai->ai_addr, ai->ai_addrlen,
                       straddr, sizeof(straddr), strport, sizeof(strport),
                       NI_NUMERICHOST|NI_NUMERICSERV) != 0) {
         goto bad;
       }
       switch (ai->ai_family) {
       case AF_INET:
         expect(strport, "54321");
         if (passive) {
           expect(straddr, "0.0.0.0");
         } else {
           expect(straddr, "127.0.0.1");
         }
         inet4++;
         break;
       case AF_INET6:
         expect(strport, "54321");
         if (passive) {
           expect(straddr, "::");
         } else {
           expect(straddr, "::1");
         }
         inet6++;
         break;
       case AF_UNSPEC:
         goto bad;
         break;
       default:
         /* another family support? */
         break;
       }
     }
   }

   if (!(inet4 == 0 || inet4 == 2))
     goto bad;
   if (!(inet6 == 0 || inet6 == 2))
     goto bad;

   if (aitop)
     freeaddrinfo(aitop);
   ${cf_cv_main_return:-return}(0);

  bad:
   if (aitop)
     freeaddrinfo(aitop);
   ${cf_cv_main_return:-return}(1);
}
],
[cf_cv_getaddrinfo=yes],
[cf_cv_getaddrinfo=no],
[cf_cv_getaddrinfo=unknown])
])
if test "$cf_cv_getaddrinfo" = yes ; then
	AC_DEFINE(HAVE_GAI_STRERROR,1,[Define to 1 if we have gai_strerror function])
	AC_DEFINE(HAVE_GETADDRINFO,1,[Define to 1 if we have getaddrinfo function])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_MEMMOVE version: 9 updated: 2017/01/21 11:06:25
dnl ---------------
dnl Check for memmove, or a bcopy that can handle overlapping copy.  If neither
dnl is found, add our own version of memmove to the list of objects.
AC_DEFUN([CF_FUNC_MEMMOVE],
[
AC_CHECK_FUNC(memmove,,[
AC_CHECK_FUNC(bcopy,[
	AC_CACHE_CHECK(if bcopy does overlapping moves,cf_cv_good_bcopy,[
		AC_TRY_RUN([
int main(void) {
	static char data[] = "abcdefghijklmnopqrstuwwxyz";
	char temp[40];
	bcopy(data, temp, sizeof(data));
	bcopy(temp+10, temp, 15);
	bcopy(temp+5, temp+15, 10);
	${cf_cv_main_return:-return} (strcmp(temp, "klmnopqrstuwwxypqrstuwwxyz"));
}
		],
		[cf_cv_good_bcopy=yes],
		[cf_cv_good_bcopy=no],
		[cf_cv_good_bcopy=unknown])
		])
	],[cf_cv_good_bcopy=no])
	if test "$cf_cv_good_bcopy" = yes ; then
		AC_DEFINE(USE_OK_BCOPY,1,[Define to 1 to use bcopy when memmove is unavailable])
	else
		AC_DEFINE(USE_MY_MEMMOVE,1,[Define to 1 to use replacement function when memmove is unavailable])
	fi
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_SYSTEM version: 6 updated: 2019/12/31 20:39:42
dnl --------------
dnl Check if the 'system()' function returns a usable status, or if not, try
dnl to use the status returned by a SIGCHLD.
AC_DEFUN([CF_FUNC_SYSTEM],
[
AC_REQUIRE([CF_UNION_WAIT])
AC_MSG_CHECKING(if the system function returns usable child-status)
AC_CACHE_VAL(cf_cv_system_status,[
	AC_TRY_RUN([
#include <stdio.h>
#include <signal.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

RETSIGTYPE signal_handler (int sig)
{
#if HAVE_TYPE_UNIONWAIT
	union wait wait_status;
#else
	int wait_status = 1;
#endif
	int system_status;
	wait (&wait_status);
	system_status = WEXITSTATUS(wait_status); /* should be nonzero */
	${cf_cv_main_return:-return}(system_status != 23);
}

int main()
{
	/* this looks weird, but apparently the SIGCHLD gets there first on
	 * machines where 'system()' doesn't return a usable code, so ...
	 */
	signal (SIGCHLD, signal_handler);
	system("exit 23");
	${cf_cv_main_return:-return}(1);
}
],
	[cf_cv_system_status=no],
	[AC_TRY_RUN(
	[int main() { ${cf_cv_main_return:-return}(system("exit 23") != (23 << 8)); }],
	[cf_cv_system_status=yes],
	[cf_cv_system_status=unknown],
	[cf_cv_system_status=unknown])],
	[cf_cv_system_status=unknown])
])
AC_MSG_RESULT($cf_cv_system_status)
test $cf_cv_system_status = no && AC_DEFINE(USE_SYSTEM_STATUS,1,[Define this to 1 if the system function returns usable exit-status])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 8 updated: 2019/09/07 13:38:36
dnl --------------
dnl Find version of gcc, and (because icc/clang pretend to be gcc without being
dnl compatible), attempt to determine if icc/clang is actually used.
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(GCC[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)
CF_CLANG_COMPILER(GCC,CLANG_COMPILER,CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 41 updated: 2021/01/01 16:53:59
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Winline (usually not worthwhile)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
if test "x$have_x" = xyes; then CF_CONST_X_STRING fi
cat > "conftest.$ac_ext" <<EOF
#line __oline__ "${as_me:-configure}"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF
if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #279: controlling expression is constant

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall"
	for cf_opt in \
		wd1419 \
		wd1683 \
		wd1684 \
		wd193 \
		wd593 \
		wd279 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
elif test "$GCC" = yes && test "$GCC_VERSION" != "unknown"
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	cf_gcc_warnings="Wignored-qualifiers Wlogical-op Wvarargs"
	test "x$CLANG_COMPILER" = xyes && cf_gcc_warnings=
	for cf_opt in W Wall \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Wdeclaration-after-statement \
		Wextra \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef Wno-inline $cf_gcc_warnings $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case "$cf_opt" in
			Winline)
				case "$GCC_VERSION" in
				[[34]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			Wpointer-arith)
				case "$GCC_VERSION" in
				[[12]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -rf ./conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 10 updated: 2018/12/10 20:09:41
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
AC_DEFUN([CF_GNU_SOURCE],
[
cf_gnu_xopen_source=ifelse($1,,500,$1)

AC_CACHE_CHECK(if this is the GNU C library,cf_cv_gnu_library,[
AC_TRY_COMPILE([#include <sys/types.h>],[
	#if __GLIBC__ > 0 && __GLIBC_MINOR__ >= 0
		return 0;
	#elif __NEWLIB__ > 0 && __NEWLIB_MINOR__ >= 0
		return 0;
	#else
	#	error not GNU C library
	#endif],
	[cf_cv_gnu_library=yes],
	[cf_cv_gnu_library=no])
])

if test x$cf_cv_gnu_library = xyes; then

	# With glibc 2.19 (13 years after this check was begun), _DEFAULT_SOURCE
	# was changed to help a little.  newlib incorporated the change about 4
	# years later.
	AC_CACHE_CHECK(if _DEFAULT_SOURCE can be used as a basis,cf_cv_gnu_library_219,[
		cf_save="$CPPFLAGS"
		CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 19) || (__GLIBC__ > 2)
				return 0;
			#elif (__NEWLIB__ == 2 && __NEWLIB_MINOR__ >= 4) || (__GLIBC__ > 3)
				return 0;
			#else
			#	error GNU C library __GLIBC__.__GLIBC_MINOR__ is too old
			#endif],
			[cf_cv_gnu_library_219=yes],
			[cf_cv_gnu_library_219=no])
		CPPFLAGS="$cf_save"
	])

	if test "x$cf_cv_gnu_library_219" = xyes; then
		cf_save="$CPPFLAGS"
		AC_CACHE_CHECK(if _XOPEN_SOURCE=$cf_gnu_xopen_source works with _DEFAULT_SOURCE,cf_cv_gnu_dftsrc_219,[
			CF_ADD_CFLAGS(-D_DEFAULT_SOURCE -D_XOPEN_SOURCE=$cf_gnu_xopen_source)
			AC_TRY_COMPILE([
				#include <limits.h>
				#include <sys/types.h>
				],[
				#if (_XOPEN_SOURCE >= $cf_gnu_xopen_source) && (MB_LEN_MAX > 1)
					return 0;
				#else
				#	error GNU C library is too old
				#endif],
				[cf_cv_gnu_dftsrc_219=yes],
				[cf_cv_gnu_dftsrc_219=no])
			])
		test "x$cf_cv_gnu_dftsrc_219" = "xyes" || CPPFLAGS="$cf_save"
	else
		cf_cv_gnu_dftsrc_219=maybe
	fi

	if test "x$cf_cv_gnu_dftsrc_219" != xyes; then

		AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
		AC_TRY_COMPILE([#include <sys/types.h>],[
			#ifndef _XOPEN_SOURCE
			#error	expected _XOPEN_SOURCE to be defined
			#endif],
			[cf_cv_gnu_source=no],
			[cf_save="$CPPFLAGS"
			 CF_ADD_CFLAGS(-D_GNU_SOURCE)
			 AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _XOPEN_SOURCE
				#error	expected _XOPEN_SOURCE to be undefined
				#endif],
				[cf_cv_gnu_source=no],
				[cf_cv_gnu_source=yes])
			CPPFLAGS="$cf_save"
			])
		])

		if test "$cf_cv_gnu_source" = yes
		then
		AC_CACHE_CHECK(if we should also define _DEFAULT_SOURCE,cf_cv_default_source,[
			CF_APPEND_TEXT(CPPFLAGS,-D_GNU_SOURCE)
			AC_TRY_COMPILE([#include <sys/types.h>],[
				#ifdef _DEFAULT_SOURCE
				#error	expected _DEFAULT_SOURCE to be undefined
				#endif],
				[cf_cv_default_source=no],
				[cf_cv_default_source=yes])
			])
			if test "$cf_cv_default_source" = yes
			then
				CF_APPEND_TEXT(CPPFLAGS,-D_DEFAULT_SOURCE)
			fi
		fi
	fi

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HEADER_PATH version: 15 updated: 2021/01/01 13:31:04
dnl --------------
dnl Construct a search-list of directories for a nonstandard header-file
dnl
dnl Parameters
dnl	$1 = the variable to return as result
dnl	$2 = the package name
AC_DEFUN([CF_HEADER_PATH],
[
$1=

# collect the current set of include-directories from compiler flags
cf_header_path_list=""
if test -n "${CFLAGS}${CPPFLAGS}" ; then
	for cf_header_path in $CPPFLAGS $CFLAGS
	do
		case "$cf_header_path" in
		-I*)
			cf_header_path=`echo ".$cf_header_path" |sed -e 's/^...//' -e 's,/include$,,'`
			CF_ADD_SUBDIR_PATH($1,$2,include,$cf_header_path,NONE)
			cf_header_path_list="$cf_header_path_list [$]$1"
			;;
		esac
	done
fi

# add the variations for the package we are looking for
CF_SUBDIR_PATH($1,$2,include)

test "$includedir" != NONE && \
test "$includedir" != "/usr/include" && \
test -d "$includedir" && {
	test -d "$includedir" &&    $1="[$]$1 $includedir"
	test -d "$includedir/$2" && $1="[$]$1 $includedir/$2"
}

test "$oldincludedir" != NONE && \
test "$oldincludedir" != "/usr/include" && \
test -d "$oldincludedir" && {
	test -d "$oldincludedir"    && $1="[$]$1 $oldincludedir"
	test -d "$oldincludedir/$2" && $1="[$]$1 $oldincludedir/$2"
}

$1="[$]$1 $cf_header_path_list"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 8 updated: 2021/01/01 16:53:59
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
AC_REQUIRE([AC_CANONICAL_HOST])
ifelse([$2],,INTEL_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	case "$host_os" in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse([$1],GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
		ifelse([$3],,CFLAGS,[$3])="$ifelse([$3],,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse([$2],,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147"
],[])
		ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse([$2],,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIBRARY_PATH version: 11 updated: 2021/01/01 13:31:04
dnl ---------------
dnl Construct a search-list of directories for a nonstandard library-file
dnl
dnl Parameters
dnl	$1 = the variable to return as result
dnl	$2 = the package name
AC_DEFUN([CF_LIBRARY_PATH],
[
$1=
cf_library_path_list=""
if test -n "${LDFLAGS}${LIBS}" ; then
	for cf_library_path in $LDFLAGS $LIBS
	do
		case "$cf_library_path" in
		-L*)
			cf_library_path=`echo ".$cf_library_path" |sed -e 's/^...//' -e 's,/lib$,,'`
			CF_ADD_SUBDIR_PATH($1,$2,lib,$cf_library_path,NONE)
			cf_library_path_list="$cf_library_path_list [$]$1"
			;;
		esac
	done
fi

CF_SUBDIR_PATH($1,$2,lib)

$1="$cf_library_path_list [$]$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIB_PREFIX version: 14 updated: 2021/01/01 13:31:04
dnl -------------
dnl Compute the library-prefix for the given host system
dnl $1 = variable to set
define([CF_LIB_PREFIX],
[
	case "$cf_cv_system_name" in
	OS/2*|os2*)
		if test "$DFT_LWR_MODEL" = libtool; then
			LIB_PREFIX='lib'
		else
			LIB_PREFIX=''
		fi
		;;
	*-msvc*)
		LIB_PREFIX=''
		;;
	*)	LIB_PREFIX='lib'
		;;
	esac
ifelse($1,,,[$1=$LIB_PREFIX])
	AC_SUBST(LIB_PREFIX)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LOCKFILE version: 2 updated: 2001/05/26 12:41:02
dnl -----------
dnl Check for functions that we can use for file-locking.  Simply checking for
dnl existence is not sufficient, since fcntl, for example, uses definitions
dnl that are not in every version of the header.
AC_DEFUN([CF_LOCKFILE],
[
AC_CHECK_HEADERS(unistd.h fcntl.h sys/file.h)
AC_CHECK_FUNC(fcntl)
AC_CHECK_FUNC(lockf)
AC_CHECK_FUNC(flock,,,[
	CF_RECHECK_FUNC(flock,bsd,cf_cv_lockfile,[AC_MSG_WARN(no lock function found)])])

AC_CACHE_CHECK(for file-locking functions,cf_cv_lockfile,[
cf_cv_lockfile=
for cf_lock in fcntl lockf flock
do
if eval 'test ${ac_cv_func_'$cf_lock'+set} = set'; then
	case $cf_lock in #(vi
	fcntl) #(vi
		AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
],[
	struct flock flk;
	int block = 1;
	int fd = 0;
	int rc;

	flk.l_type = F_WRLCK;
	flk.l_whence = SEEK_SET;
	flk.l_start = 0;
	flk.l_len = 0;
	rc = fcntl(fd, block ? F_SETLKW : F_SETLK, &flk)
	&& fcntl(fd, F_GETLK, &flk)
	&& fcntl(fd, F_SETLK, &flk);
	],,continue)
		;;
	lockf) #(vi
		AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
],[
	int block = 1, fd = 0;
	int ret = lockf(fd, block ? F_LOCK : F_TLOCK, 0L)
	 && lockf(fd, F_TEST, 0L)
	 && lockf(fd, F_ULOCK, 0L);
	],,[continue])
		;;
	flock)
		AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
],[
	int block = 1, fd = 0;
	int ret = flock(fd, (LOCK_EX|LOCK_NB))
	 && flock(fd, LOCK_UN)
	 && flock(fd, block ? LOCK_EX : (LOCK_EX | LOCK_NB));
	],,[continue])
		;;
	esac
	cf_cv_lockfile="$cf_cv_lockfile $cf_lock"
fi
done
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKEFLAGS version: 19 updated: 2020/12/31 20:19:42
dnl ------------
dnl Some 'make' programs support ${MAKEFLAGS}, some ${MFLAGS}, to pass 'make'
dnl options to lower-levels.  It's very useful for "make -n" -- if we have it.
dnl (GNU 'make' does both, something POSIX 'make', which happens to make the
dnl ${MAKEFLAGS} variable incompatible because it adds the assignments :-)
AC_DEFUN([CF_MAKEFLAGS],
[
AC_CACHE_CHECK(for makeflags variable, cf_cv_makeflags,[
	cf_cv_makeflags=''
	for cf_option in '-${MAKEFLAGS}' '${MFLAGS}'
	do
		cat >cf_makeflags.tmp <<CF_EOF
SHELL = $SHELL
all :
	@ echo '.$cf_option'
CF_EOF
		cf_result=`${MAKE:-make} -k -f cf_makeflags.tmp 2>/dev/null | fgrep -v "ing directory" | sed -e 's,[[ 	]]*$,,'`
		case "$cf_result" in
		.*k|.*kw)
			cf_result="`${MAKE:-make} -k -f cf_makeflags.tmp CC=cc 2>/dev/null`"
			case "$cf_result" in
			.*CC=*)	cf_cv_makeflags=
				;;
			*)	cf_cv_makeflags=$cf_option
				;;
			esac
			break
			;;
		.-)
			;;
		*)
			CF_MSG_LOG(given option \"$cf_option\", no match \"$cf_result\")
			;;
		esac
	done
	rm -f cf_makeflags.tmp
])

AC_SUBST(cf_cv_makeflags)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 5 updated: 2010/10/23 15:52:32
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me:-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_BROKEN version: 8 updated: 2012/11/08 20:57:52
dnl -----------------
dnl Check for pre-1.9.9g ncurses (among other problems, the most obvious is
dnl that color combinations don't work).
AC_DEFUN([CF_NCURSES_BROKEN],
[
AC_REQUIRE([CF_NCURSES_VERSION])
if test "$cf_cv_ncurses_version" != no ; then
AC_MSG_CHECKING(for obsolete/broken version of ncurses)
AC_CACHE_VAL(cf_cv_ncurses_broken,[
AC_TRY_COMPILE([
#include <${cf_cv_ncurses_header:-curses.h}>],[
#if defined(NCURSES_VERSION) && defined(wgetbkgd)
	make an error
#else
	int x = 1
#endif
],
	[cf_cv_ncurses_broken=no],
	[cf_cv_ncurses_broken=yes])
])
AC_MSG_RESULT($cf_cv_ncurses_broken)
if test "$cf_cv_ncurses_broken" = yes ; then
	AC_MSG_WARN(hmm... you should get an up-to-date version of ncurses)
	AC_DEFINE(NCURSES_BROKEN,1,[Define to 1 if you have an obsolete version of ncurses])
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CC_CHECK version: 5 updated: 2020/12/31 20:19:42
dnl -------------------
dnl Check if we can compile with ncurses' header file
dnl $1 is the cache variable to set
dnl $2 is the header-file to include
dnl $3 is the root name (ncurses or ncursesw)
AC_DEFUN([CF_NCURSES_CC_CHECK],[
	AC_TRY_COMPILE([
]ifelse($3,ncursesw,[
#define _XOPEN_SOURCE_EXTENDED
#undef  HAVE_LIBUTF8_H	/* in case we used CF_UTF8_LIB */
#define HAVE_LIBUTF8_H	/* to force ncurses' header file to use cchar_t */
])[
#include <$2>],[
#ifdef NCURSES_VERSION
]ifelse($3,ncursesw,[
#ifndef WACS_BSSB
	make an error
#endif
])[
printf("%s\\n", NCURSES_VERSION);
#else
#ifdef __NCURSES_H
printf("old\\n");
#else
	make an error
#endif
#endif
	]
	,[$1=$2]
	,[$1=no])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CONFIG version: 26 updated: 2021/01/03 08:05:37
dnl -----------------
dnl Tie together the configure-script macros for ncurses, preferring these in
dnl order:
dnl a) ".pc" files for pkg-config, using $NCURSES_CONFIG_PKG
dnl b) the "-config" script from ncurses, using $NCURSES_CONFIG
dnl c) just plain libraries
dnl
dnl $1 is the root library name (default: "ncurses")
AC_DEFUN([CF_NCURSES_CONFIG],[
AC_REQUIRE([CF_PKG_CONFIG])
cf_ncuconfig_root=ifelse($1,,ncurses,$1)
cf_have_ncuconfig=no

if test "x${PKG_CONFIG:=none}" != xnone; then
	AC_MSG_CHECKING(pkg-config for $cf_ncuconfig_root)
	if "$PKG_CONFIG" --exists $cf_ncuconfig_root ; then
		AC_MSG_RESULT(yes)

		AC_MSG_CHECKING(if the $cf_ncuconfig_root package files work)
		cf_have_ncuconfig=unknown

		cf_save_CFLAGS="$CFLAGS"
		cf_save_CPPFLAGS="$CPPFLAGS"
		cf_save_LIBS="$LIBS"

		cf_pkg_cflags="`$PKG_CONFIG --cflags $cf_ncuconfig_root`"
		cf_pkg_libs="`$PKG_CONFIG --libs $cf_ncuconfig_root`"

		# while -W for passing linker flags is prevalent, it is not "standard".
		# At least one wrapper for c89/c99 (in Apple's xcode) has its own
		# incompatible _and_ non-standard -W option which gives an error.  Work
		# around that pitfall.
		case "x${CC}@@${cf_pkg_libs}@${cf_pkg_cflags}" in
		x*c[[89]]9@@*-W*)
			CF_ADD_CFLAGS($cf_pkg_cflags)
			CF_ADD_LIBS($cf_pkg_libs)

			AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
				[initscr(); mousemask(0,0); tigetstr((char *)0);],
				[AC_TRY_RUN([#include <${cf_cv_ncurses_header:-curses.h}>
					int main(void)
					{ char *xx = curses_version(); return (xx == 0); }],
					[cf_test_ncuconfig=yes],
					[cf_test_ncuconfig=no],
					[cf_test_ncuconfig=maybe])],
				[cf_test_ncuconfig=no])

			CFLAGS="$cf_save_CFLAGS"
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"

			if test "x$cf_test_ncuconfig" != xyes; then
				cf_temp=`echo "x$cf_pkg_cflags" | sed -e s/^x// -e 's/-W[[^ 	]]*//g'`
				cf_pkg_cflags="$cf_temp"
				cf_temp=`echo "x$cf_pkg_libs" | sed -e s/^x// -e 's/-W[[^ 	]]*//g'`
				cf_pkg_libs="$cf_temp"
			fi
			;;
		esac

		CF_ADD_CFLAGS($cf_pkg_cflags)
		CF_ADD_LIBS($cf_pkg_libs)

		AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
			[initscr(); mousemask(0,0); tigetstr((char *)0);],
			[AC_TRY_RUN([#include <${cf_cv_ncurses_header:-curses.h}>
				int main(void)
				{ char *xx = curses_version(); return (xx == 0); }],
				[cf_have_ncuconfig=yes],
				[cf_have_ncuconfig=no],
				[cf_have_ncuconfig=maybe])],
			[cf_have_ncuconfig=no])
		AC_MSG_RESULT($cf_have_ncuconfig)
		test "$cf_have_ncuconfig" = maybe && cf_have_ncuconfig=yes
		if test "$cf_have_ncuconfig" != "yes"
		then
			CPPFLAGS="$cf_save_CPPFLAGS"
			LIBS="$cf_save_LIBS"
			NCURSES_CONFIG_PKG=none
		else
			AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])
			NCURSES_CONFIG_PKG=$cf_ncuconfig_root
			CF_TERM_HEADER
		fi

	else
		AC_MSG_RESULT(no)
		NCURSES_CONFIG_PKG=none
	fi
else
	NCURSES_CONFIG_PKG=none
fi

if test "x$cf_have_ncuconfig" = "xno"; then
	cf_ncurses_config="${cf_ncuconfig_root}${NCURSES_CONFIG_SUFFIX}-config"; echo "Looking for ${cf_ncurses_config}"

	CF_ACVERSION_CHECK(2.52,
		[AC_CHECK_TOOLS(NCURSES_CONFIG, ${cf_ncurses_config} ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}5-config, none)],
		[AC_PATH_PROGS(NCURSES_CONFIG,  ${cf_ncurses_config} ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}6-config ${cf_ncuconfig_root}5-config, none)])

	if test "$NCURSES_CONFIG" != none ; then

		CF_ADD_CFLAGS(`$NCURSES_CONFIG --cflags`)
		CF_ADD_LIBS(`$NCURSES_CONFIG --libs`)

		# even with config script, some packages use no-override for curses.h
		CF_CURSES_HEADER(ifelse($1,,ncurses,$1))

		dnl like CF_NCURSES_CPPFLAGS
		AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])

		dnl like CF_NCURSES_LIBS
		CF_UPPER(cf_nculib_ROOT,HAVE_LIB$cf_ncuconfig_root)
		AC_DEFINE_UNQUOTED($cf_nculib_ROOT)

		dnl like CF_NCURSES_VERSION
		cf_cv_ncurses_version="`$NCURSES_CONFIG --version`"

	else

		CF_NCURSES_CPPFLAGS(ifelse($1,,ncurses,$1))
		CF_NCURSES_LIBS(ifelse($1,,ncurses,$1))

	fi
else
	NCURSES_CONFIG=none
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_CPPFLAGS version: 22 updated: 2021/01/02 09:31:20
dnl -------------------
dnl Look for the SVr4 curses clone 'ncurses' in the standard places, adjusting
dnl the CPPFLAGS variable so we can include its header.
dnl
dnl The header files may be installed as either curses.h, or ncurses.h (would
dnl be obsolete, except that some packagers prefer this name to distinguish it
dnl from a "native" curses implementation).  If not installed for overwrite,
dnl the curses.h file would be in an ncurses subdirectory (e.g.,
dnl /usr/include/ncurses), but someone may have installed overwriting the
dnl vendor's curses.  Only very old versions (pre-1.9.2d, the first autoconf'd
dnl version) of ncurses don't define either __NCURSES_H or NCURSES_VERSION in
dnl the header.
dnl
dnl If the installer has set $CFLAGS or $CPPFLAGS so that the ncurses header
dnl is already in the include-path, don't even bother with this, since we cannot
dnl easily determine which file it is.  In this case, it has to be <curses.h>.
dnl
dnl The optional parameter gives the root name of the library, in case it is
dnl not installed as the default curses library.  That is how the
dnl wide-character version of ncurses is installed.
AC_DEFUN([CF_NCURSES_CPPFLAGS],
[AC_REQUIRE([CF_WITH_CURSES_DIR])

AC_PROVIDE([CF_CURSES_CPPFLAGS])dnl
cf_ncuhdr_root=ifelse($1,,ncurses,$1)

test -n "$cf_cv_curses_dir" && \
test "$cf_cv_curses_dir" != "no" && { \
  CF_ADD_INCDIR($cf_cv_curses_dir/include/$cf_ncuhdr_root)
}

AC_CACHE_CHECK(for $cf_ncuhdr_root header in include-path, cf_cv_ncurses_h,[
	cf_header_list="$cf_ncuhdr_root/curses.h $cf_ncuhdr_root/ncurses.h"
	{ test "$cf_ncuhdr_root" = ncurses || test "$cf_ncuhdr_root" = ncursesw; } && cf_header_list="$cf_header_list curses.h ncurses.h"
	for cf_header in $cf_header_list
	do
		CF_NCURSES_CC_CHECK(cf_cv_ncurses_h,$cf_header,$1)
		test "$cf_cv_ncurses_h" != no && break
	done
])

CF_NCURSES_HEADER
CF_TERM_HEADER

# some applications need this, but should check for NCURSES_VERSION
AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])

CF_NCURSES_VERSION
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_HEADER version: 6 updated: 2021/01/02 09:31:20
dnl -----------------
dnl Find a "curses" header file, e.g,. "curses.h", or one of the more common
dnl variations of ncurses' installs.
dnl
dnl See also CF_CURSES_HEADER, which sets the same cache variable.
AC_DEFUN([CF_NCURSES_HEADER],[

if test "$cf_cv_ncurses_h" != no ; then
	cf_cv_ncurses_header=$cf_cv_ncurses_h
else

AC_CACHE_CHECK(for $cf_ncuhdr_root include-path, cf_cv_ncurses_h2,[
	test -n "$verbose" && echo
	CF_HEADER_PATH(cf_search,$cf_ncuhdr_root)
	test -n "$verbose" && echo "search path $cf_search"
	cf_save2_CPPFLAGS="$CPPFLAGS"
	for cf_incdir in $cf_search
	do
		CF_ADD_INCDIR($cf_incdir)
		for cf_header in \
			ncurses.h \
			curses.h
		do
			CF_NCURSES_CC_CHECK(cf_cv_ncurses_h2,$cf_header,$1)
			if test "$cf_cv_ncurses_h2" != no ; then
				cf_cv_ncurses_h2=$cf_incdir/$cf_header
				test -n "$verbose" && echo $ac_n "	... found $ac_c" 1>&AC_FD_MSG
				break
			fi
			test -n "$verbose" && echo "	... tested $cf_incdir/$cf_header" 1>&AC_FD_MSG
		done
		CPPFLAGS="$cf_save2_CPPFLAGS"
		test "$cf_cv_ncurses_h2" != no && break
	done
	test "$cf_cv_ncurses_h2" = no && AC_MSG_ERROR(not found)
	])

	CF_DIRNAME(cf_1st_incdir,$cf_cv_ncurses_h2)
	cf_cv_ncurses_header="`basename "$cf_cv_ncurses_h2"`"
	if test "`basename "$cf_1st_incdir"`" = "$cf_ncuhdr_root" ; then
		cf_cv_ncurses_header="$cf_ncuhdr_root/$cf_cv_ncurses_header"
	fi
	CF_ADD_INCDIR($cf_1st_incdir)

fi

# Set definitions to allow ifdef'ing for ncurses.h

case "$cf_cv_ncurses_header" in
*ncurses.h)
	AC_DEFINE(HAVE_NCURSES_H,1,[Define to 1 if we have ncurses.h])
	;;
esac

case "$cf_cv_ncurses_header" in
ncurses/curses.h|ncurses/ncurses.h)
	AC_DEFINE(HAVE_NCURSES_NCURSES_H,1,[Define to 1 if we have ncurses/ncurses.h])
	;;
ncursesw/curses.h|ncursesw/ncurses.h)
	AC_DEFINE(HAVE_NCURSESW_NCURSES_H,1,[Define to 1 if we have ncursesw/ncurses.h])
	;;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_LIBS version: 20 updated: 2021/01/03 08:05:37
dnl ---------------
dnl Look for the ncurses library.  This is a little complicated on Linux,
dnl because it may be linked with the gpm (general purpose mouse) library.
dnl Some distributions have gpm linked with (bsd) curses, which makes it
dnl unusable with ncurses.  However, we don't want to link with gpm unless
dnl ncurses has a dependency, since gpm is normally set up as a shared library,
dnl and the linker will record a dependency.
dnl
dnl The optional parameter gives the root name of the library, in case it is
dnl not installed as the default curses library.  That is how the
dnl wide-character version of ncurses is installed.
AC_DEFUN([CF_NCURSES_LIBS],
[AC_REQUIRE([CF_NCURSES_CPPFLAGS])

cf_nculib_root=ifelse($1,,ncurses,$1)
	# This works, except for the special case where we find gpm, but
	# ncurses is in a nonstandard location via $LIBS, and we really want
	# to link gpm.
cf_ncurses_LIBS=""
cf_ncurses_SAVE="$LIBS"
AC_CHECK_LIB(gpm,Gpm_Open,
	[AC_CHECK_LIB(gpm,initscr,
		[LIBS="$cf_ncurses_SAVE"],
		[cf_ncurses_LIBS="-lgpm"])])

case "$host_os" in
freebsd*)
	# This is only necessary if you are linking against an obsolete
	# version of ncurses (but it should do no harm, since it's static).
	if test "$cf_nculib_root" = ncurses ; then
		AC_CHECK_LIB(mytinfo,tgoto,[cf_ncurses_LIBS="-lmytinfo $cf_ncurses_LIBS"])
	fi
	;;
esac

CF_ADD_LIBS($cf_ncurses_LIBS)

if test -n "$cf_cv_curses_dir" && test "$cf_cv_curses_dir" != "no"
then
	CF_ADD_LIBS(-l$cf_nculib_root)
else
	CF_FIND_LIBRARY($cf_nculib_root,$cf_nculib_root,
		[#include <${cf_cv_ncurses_header:-curses.h}>],
		[initscr()],
		initscr)
fi

if test -n "$cf_ncurses_LIBS" ; then
	AC_MSG_CHECKING(if we can link $cf_nculib_root without $cf_ncurses_LIBS)
	cf_ncurses_SAVE="$LIBS"
	for p in $cf_ncurses_LIBS ; do
		q=`echo "$LIBS" | sed -e "s%$p %%" -e "s%$p$%%"`
		if test "$q" != "$LIBS" ; then
			LIBS="$q"
		fi
	done
	AC_TRY_LINK([#include <${cf_cv_ncurses_header:-curses.h}>],
		[initscr(); mousemask(0,0); tigetstr((char *)0);],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_RESULT(no)
		 LIBS="$cf_ncurses_SAVE"])
fi

CF_UPPER(cf_nculib_ROOT,HAVE_LIB$cf_nculib_root)
AC_DEFINE_UNQUOTED($cf_nculib_ROOT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_VERSION version: 16 updated: 2020/12/31 20:19:42
dnl ------------------
dnl Check for the version of ncurses, to aid in reporting bugs, etc.
dnl Call CF_CURSES_CPPFLAGS first, or CF_NCURSES_CPPFLAGS.  We don't use
dnl AC_REQUIRE since that does not work with the shell's if/then/else/fi.
AC_DEFUN([CF_NCURSES_VERSION],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_CACHE_CHECK(for ncurses version, cf_cv_ncurses_version,[
	cf_cv_ncurses_version=no
	cf_tempfile=out$$
	rm -f "$cf_tempfile"
	AC_TRY_RUN([
#include <${cf_cv_ncurses_header:-curses.h}>
#include <stdio.h>
int main(void)
{
	FILE *fp = fopen("$cf_tempfile", "w");
#ifdef NCURSES_VERSION
# ifdef NCURSES_VERSION_PATCH
	fprintf(fp, "%s.%d\\n", NCURSES_VERSION, NCURSES_VERSION_PATCH);
# else
	fprintf(fp, "%s\\n", NCURSES_VERSION);
# endif
#else
# ifdef __NCURSES_H
	fprintf(fp, "old\\n");
# else
	make an error
# endif
#endif
	${cf_cv_main_return:-return}(0);
}],[
	cf_cv_ncurses_version=`cat $cf_tempfile`],,[

	# This will not work if the preprocessor splits the line after the
	# Autoconf token.  The 'unproto' program does that.
	cat > "conftest.$ac_ext" <<EOF
#include <${cf_cv_ncurses_header:-curses.h}>
#undef Autoconf
#ifdef NCURSES_VERSION
Autoconf NCURSES_VERSION
#else
#ifdef __NCURSES_H
Autoconf "old"
#endif
;
#endif
EOF
	cf_try="$ac_cpp conftest.$ac_ext 2>&AC_FD_CC | grep '^Autoconf ' >conftest.out"
	AC_TRY_EVAL(cf_try)
	if test -f conftest.out ; then
		cf_out=`sed -e 's%^Autoconf %%' -e 's%^[[^"]]*"%%' -e 's%".*%%' conftest.out`
		test -n "$cf_out" && cf_cv_ncurses_version="$cf_out"
		rm -f conftest.out
	fi
])
	rm -f "$cf_tempfile"
])
test "$cf_cv_ncurses_version" = no || AC_DEFINE(NCURSES,1,[Define to 1 if we are using ncurses headers/libraries])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NETLIBS version: 12 updated: 2021/01/02 09:31:20
dnl ----------
dnl After checking for functions in the default $LIBS, make a further check
dnl for the functions that are netlib-related (these aren't always in the
dnl libc, etc., and have to be handled specially because there are conflicting
dnl and broken implementations.
dnl Common library requirements (in order):
dnl	-lresolv -lsocket -lnsl
dnl	-lnsl -lsocket
dnl	-lsocket
dnl	-lbsd
dnl	-lnetwork
AC_DEFUN([CF_NETLIBS],[
cf_test_netlibs=no

AC_MSG_CHECKING(for network libraries)

AC_CACHE_VAL(cf_cv_netlibs,[
AC_MSG_RESULT(working...)

cf_cv_netlibs=""
cf_test_netlibs=yes

case "$host_os" in
mingw*)
	AC_CHECK_HEADERS( windows.h winsock.h winsock2.h )

	if test "$ac_cv_header_winsock2_h" = "yes" ; then
		cf_winsock_lib="-lws2_32"
	elif test "$ac_cv_header_winsock_h" = "yes" ; then
		cf_winsock_lib="-lwsock32"
	fi

	cf_save_LIBS="$LIBS"
	CF_ADD_LIBS($cf_winsock_lib)

	AC_TRY_LINK([
#ifdef HAVE_WINDOWS_H
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#else
#ifdef HAVE_WINSOCK_H
#include <winsock.h>
#endif
#endif
#endif
],[
	char buffer[1024];
	gethostname(buffer, sizeof(buffer));],
	[cf_cv_netlibs="$cf_winsock_lib $cf_cv_netlibs"],
	[AC_MSG_ERROR(Cannot link against winsock library)])

	LIBS="$cf_save_LIBS"
	;;
*)
	AC_CHECK_FUNCS(gethostname,,[
		CF_RECHECK_FUNC(gethostname,nsl,cf_cv_netlibs,[
			CF_RECHECK_FUNC(gethostname,socket,cf_cv_netlibs)])])

	AC_CHECK_LIB(inet, main, cf_cv_netlibs="-linet $cf_cv_netlibs")

	if test "$ac_cv_func_lsocket" != no ; then
	AC_CHECK_FUNCS(socket,,[
		CF_RECHECK_FUNC(socket,socket,cf_cv_netlibs,[
			CF_RECHECK_FUNC(socket,bsd,cf_cv_netlibs)])])
	fi

	AC_CHECK_FUNCS(gethostbyname,,[
		CF_RECHECK_FUNC(gethostbyname,nsl,cf_cv_netlibs)])

	AC_CHECK_FUNCS(inet_ntoa,,[
		CF_RECHECK_FUNC(inet_ntoa,nsl,cf_cv_netlibs)])

	AC_CHECK_FUNCS(gethostbyname,,[
		CF_RECHECK_FUNC(gethostbyname,network,cf_cv_netlibs)])

	AC_CHECK_FUNCS(strcasecmp,,[
		CF_RECHECK_FUNC(strcasecmp,resolv,cf_cv_netlibs)])
	;;
esac
])

case "$cf_cv_netlibs" in
*ws2_32*)
	AC_DEFINE(USE_WINSOCK2_H,1,[Define to 1 if we should include winsock2.h])
	;;
esac

CF_ADD_LIBS($cf_cv_netlibs)
test "$cf_test_netlibs" = no && echo "$cf_cv_netlibs" >&AC_FD_MSG
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NO_LEAKS_OPTION version: 7 updated: 2020/12/31 18:40:20
dnl ------------------
dnl see CF_WITH_NO_LEAKS
AC_DEFUN([CF_NO_LEAKS_OPTION],[
AC_MSG_CHECKING(if you want to use $1 for testing)
AC_ARG_WITH($1,
	[$2],
	[AC_DEFINE_UNQUOTED($3,1,"Define to 1 if you want to use $1 for testing.")ifelse([$4],,[
	 $4
])
	: "${with_cflags:=-g}"
	: "${with_no_leaks:=yes}"
	 with_$1=yes],
	[with_$1=])
AC_MSG_RESULT(${with_$1:-no})

case ".$with_cflags" in
.*-g*)
	case .$CFLAGS in
	.*-g*)
		;;
	*)
		CF_ADD_CFLAGS([-g])
		;;
	esac
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_OUR_MESSAGES version: 8 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Check if we use the messages included with this program
dnl
dnl $1 specifies either Makefile or makefile, defaulting to the former.
dnl
dnl Sets variables which can be used to substitute in makefiles:
dnl	MSG_DIR_MAKE - to make ./po directory
dnl	SUB_MAKEFILE - makefile in ./po directory (see CF_BUNDLED_INTL)
dnl
AC_DEFUN([CF_OUR_MESSAGES],
[
cf_makefile=ifelse($1,,Makefile,$1)

use_our_messages=no
if test "$USE_NLS" = yes ; then
if test -d "$srcdir/po" ; then
AC_MSG_CHECKING(if we should use included message-library)
	AC_ARG_ENABLE(included-msgs,
	[  --disable-included-msgs use included messages, for i18n support],
	[use_our_messages=$enableval],
	[use_our_messages=yes])
fi
AC_MSG_RESULT($use_our_messages)
fi

MSG_DIR_MAKE="#"
if test "$use_our_messages" = yes
then
	SUB_MAKEFILE="$SUB_MAKEFILE po/$cf_makefile.in:$srcdir/po/$cf_makefile.inn"
	MSG_DIR_MAKE=
fi

AC_SUBST(MSG_DIR_MAKE)
AC_SUBST(SUB_MAKEFILE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATHSEP version: 8 updated: 2021/01/01 13:31:04
dnl ----------
dnl Provide a value for the $PATH and similar separator (or amend the value
dnl as provided in autoconf 2.5x).
AC_DEFUN([CF_PATHSEP],
[
	AC_MSG_CHECKING(for PATH separator)
	case "$cf_cv_system_name" in
	os2*)	PATH_SEPARATOR=';'  ;;
	*)	${PATH_SEPARATOR:=':'}  ;;
	esac
ifelse([$1],,,[$1=$PATH_SEPARATOR])
	AC_SUBST(PATH_SEPARATOR)
	AC_MSG_RESULT($PATH_SEPARATOR)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_EDITOR version: 6 updated: 2019/12/31 20:39:42
dnl --------------
dnl Look for the default editor (vi)
AC_DEFUN([CF_PATH_EDITOR],
[
AC_MSG_CHECKING(for default editor)
CF_ARG_WITH(editor,
    [  --with-editor=PROG      specify editor],
    [DEFAULT_EDITOR],,vi)
if test -z "$DEFAULT_EDITOR" ; then
    if test -n "$EDITOR" ; then
    	DEFAULT_EDITOR="$EDITOR"
    elif test -n "$VISUAL" ; then
    	DEFAULT_EDITOR="$VISUAL"
    else
	AC_PATH_PROG(DEFAULT_EDITOR,vi,vi,$PATH:/usr/bin:/usr/ucb)
    fi
fi
AC_MSG_RESULT($DEFAULT_EDITOR)
AC_DEFINE_UNQUOTED(DEFAULT_EDITOR,"$DEFAULT_EDITOR",[Define this to the default editor])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_MAILBOX version: 5 updated: 2019/12/31 20:39:42
dnl ---------------
dnl Look for the directory that contains incoming mail.  I would check for an
dnl actual mail-file, to verify this, but that is not always easy to arrange.
AC_DEFUN([CF_PATH_MAILBOX],
[
AC_MSG_CHECKING(for incoming-mail directory)
CF_ARG_WITH(mailbox,
    [  --with-mailbox=DIR      directory for incoming mailboxes],
    [DEFAULT_MAILBOX])
if test -z "$DEFAULT_MAILBOX" ; then
for cf_dir in \
	/var/spool/mail \
	/usr/spool/mail \
	/var/mail \
	/usr/mail \
	/mail
    do
    	if test -d $cf_dir ; then
	    DEFAULT_MAILBOX=$cf_dir
	    break
	fi
    done
fi
if test -n "$DEFAULT_MAILBOX" ; then
	AC_DEFINE_UNQUOTED(DEFAULT_MAILBOX,"$DEFAULT_MAILBOX",[Define this to the directory for incoming mailboxes])
else
	DEFAULT_MAILBOX=none
fi
AC_MSG_RESULT($DEFAULT_MAILBOX)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_MAILER version: 7 updated: 2019/12/31 20:39:42
dnl --------------
dnl Look for the program that sends outgoing mail.
AC_DEFUN([CF_PATH_MAILER],
[
AC_PATH_PROG(DEFAULT_MAILER,sendmail,,$PATH:/usr/sbin:/usr/lib)
CF_ARG_WITH(mailer,
     [  --with-mailer=PROG      specify default mailer-program],
     [DEFAULT_MAILER],,mailx)
if test -z "$DEFAULT_MAILER" ; then
AC_PATH_PROG(DEFAULT_MAILER,mailx,,$PATH:/usr/bin)
fi
if test -z "$DEFAULT_MAILER" ; then
AC_PATH_PROG(DEFAULT_MAILER,mail,,$PATH:/usr/bin)
fi
AC_MSG_CHECKING(for default mailer)
if test -n "$DEFAULT_MAILER" ; then
	CF_FIX_SLASHES(ac_cv_path_DEFAULT_MAILER)
	CF_FIX_SLASHES(DEFAULT_MAILER)
	AC_DEFINE_UNQUOTED(DEFAULT_MAILER,"$DEFAULT_MAILER",[Define this to the default mailer-program])
else
	DEFAULT_MAILER=none
fi
AC_MSG_RESULT($DEFAULT_MAILER)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_SYNTAX version: 18 updated: 2020/12/31 18:40:20
dnl --------------
dnl Check the argument to see that it looks like a pathname.  Rewrite it if it
dnl begins with one of the prefix/exec_prefix variables, and then again if the
dnl result begins with 'NONE'.  This is necessary to work around autoconf's
dnl delayed evaluation of those symbols.
AC_DEFUN([CF_PATH_SYNTAX],[
if test "x$prefix" != xNONE; then
	cf_path_syntax="$prefix"
else
	cf_path_syntax="$ac_default_prefix"
fi

case ".[$]$1" in
.\[$]\(*\)*|.\'*\'*)
	;;
..|./*|.\\*)
	;;
.[[a-zA-Z]]:[[\\/]]*) # OS/2 EMX
	;;
.\[$]\{*prefix\}*|.\[$]\{*dir\}*)
	eval $1="[$]$1"
	case ".[$]$1" in
	.NONE/*)
		$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
		;;
	esac
	;;
.no|.NONE/*)
	$1=`echo "[$]$1" | sed -e s%NONE%$cf_path_syntax%`
	;;
*)
	ifelse([$2],,[AC_MSG_ERROR([expected a pathname, not \"[$]$1\"])],$2)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PDCURSES_X11 version: 15 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Configure for PDCurses' X11 library
AC_DEFUN([CF_PDCURSES_X11],[
AC_REQUIRE([CF_X_ATHENA])

CF_ACVERSION_CHECK(2.52,
	[AC_CHECK_TOOLS(XCURSES_CONFIG, xcurses-config, none)],
	[AC_PATH_PROGS(XCURSES_CONFIG, xcurses-config, none)])

if test "$XCURSES_CONFIG" != none ; then

CF_ADD_CFLAGS(`$XCURSES_CONFIG --cflags`)
CF_ADD_LIBS(`$XCURSES_CONFIG --libs`)

cf_cv_lib_XCurses=yes

else

LDFLAGS="$LDFLAGS $X_LIBS"
CF_CHECK_CFLAGS($X_CFLAGS)
AC_CHECK_LIB(X11,XOpenDisplay,
	[CF_ADD_LIBS(-lX11)],,
	[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])
AC_CACHE_CHECK(for XCurses library,cf_cv_lib_XCurses,[
CF_ADD_LIBS(-lXCurses)
AC_TRY_LINK([
#include <xcurses.h>
char *XCursesProgramName = "test";
],[XCursesExit();],
[cf_cv_lib_XCurses=yes],
[cf_cv_lib_XCurses=no])
])

fi

if test "$cf_cv_lib_XCurses" = yes ; then
	AC_DEFINE(UNIX,1,[Define to 1 if using PDCurses on Unix])
	AC_DEFINE(XCURSES,1,[Define to 1 if using PDCurses on Unix])
	AC_CHECK_HEADER(xcurses.h, AC_DEFINE(HAVE_XCURSES,1,[Define to 1 if using PDCurses on Unix]))
else
	AC_MSG_ERROR(Cannot link with XCurses)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PKG_CONFIG version: 11 updated: 2021/01/01 13:31:04
dnl -------------
dnl Check for the package-config program, unless disabled by command-line.
AC_DEFUN([CF_PKG_CONFIG],
[
AC_MSG_CHECKING(if you want to use pkg-config)
AC_ARG_WITH(pkg-config,
	[  --with-pkg-config{=path} enable/disable use of pkg-config],
	[cf_pkg_config=$withval],
	[cf_pkg_config=yes])
AC_MSG_RESULT($cf_pkg_config)

case "$cf_pkg_config" in
no)
	PKG_CONFIG=none
	;;
yes)
	CF_ACVERSION_CHECK(2.52,
		[AC_PATH_TOOL(PKG_CONFIG, pkg-config, none)],
		[AC_PATH_PROG(PKG_CONFIG, pkg-config, none)])
	;;
*)
	PKG_CONFIG=$withval
	;;
esac

test -z "$PKG_CONFIG" && PKG_CONFIG=none
if test "$PKG_CONFIG" != none ; then
	CF_PATH_SYNTAX(PKG_CONFIG)
elif test "x$cf_pkg_config" != xno ; then
	AC_MSG_WARN(pkg-config is not installed)
fi

AC_SUBST(PKG_CONFIG)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 11 updated: 2018/12/31 20:46:17
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[AC_REQUIRE([CF_POSIX_VISIBLE])dnl

if test "$cf_cv_posix_visible" = no; then

cf_POSIX_C_SOURCE=ifelse([$1],,199506L,[$1])

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in
	 .[[12]]??*)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 .2)
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 .*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
make an error
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,$cf_cv_posix_c_source)
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	CF_ADD_CFLAGS($cf_cv_posix_c_source)
fi

fi # cf_cv_posix_visible

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_JC version: 4 updated: 2019/12/31 20:39:42
dnl -----------
dnl Check if we have POSIX-style job control (i.e., sigaction), or if we must
dnl use the signal functions.  Use AC_CHECK_FUNCS(sigaction) first.
AC_DEFUN([CF_POSIX_JC],[

AC_REQUIRE([AC_TYPE_SIGNAL])
AC_REQUIRE([CF_SIG_ARGS])

if test "$ac_cv_func_sigaction" = yes; then

AC_CACHE_CHECK(whether sigaction needs _POSIX_SOURCE,cf_cv_sigact_bad,[
AC_TRY_COMPILE([
#include <sys/types.h>
#include <signal.h>],[struct sigaction act],
  [cf_cv_sigact_bad=no],[cf_cv_sigact_bad=yes])
])

test "$cf_cv_sigact_bad" = yes && AC_DEFINE(SVR4_ACTION,1,[Define this to 1 if sigaction needs _POSIX_SOURCE])

AC_CACHE_CHECK(if we have sigaction/related functions,cf_cv_sigaction_funcs,[
AC_TRY_LINK([
#ifdef SVR4_ACTION
#define _POSIX_SOURCE
#endif
#include <sys/types.h>
#include <signal.h>],[
    RETSIGTYPE (*func)(SIG_ARGS) = SIG_IGN;
    struct sigaction sa, osa;
    sa.sa_handler = func;
    sa.sa_flags = 0;
    sigemptyset (&sa.sa_mask);
    sigaction (SIGBUS,&sa,&osa);],
    [cf_cv_sigaction_funcs=yes],
    [cf_cv_sigaction_funcs=no])])

test "$cf_cv_sigaction_funcs" = yes && AC_DEFINE(HAVE_POSIX_JC,1,[Define this to 1 if we have POSIX-style job-control functions])

fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_VISIBLE version: 1 updated: 2018/12/31 20:46:17
dnl ----------------
dnl POSIX documents test-macros which an application may set before any system
dnl headers are included to make features available.
dnl
dnl Some BSD platforms (originally FreeBSD, but copied by a few others)
dnl diverged from POSIX in 2002 by setting symbols which make all of the most
dnl recent features visible in the system header files unless the application
dnl overrides the corresponding test-macros.  Doing that introduces portability
dnl problems.
dnl
dnl This macro makes a special check for the symbols used for this, to avoid a
dnl conflicting definition.
AC_DEFUN([CF_POSIX_VISIBLE],
[
AC_CACHE_CHECK(if the POSIX test-macros are already defined,cf_cv_posix_visible,[
AC_TRY_COMPILE([#include <stdio.h>],[
#if defined(__POSIX_VISIBLE) && ((__POSIX_VISIBLE - 0L) > 0) \
	&& defined(__XSI_VISIBLE) && ((__XSI_VISIBLE - 0L) > 0) \
	&& defined(__BSD_VISIBLE) && ((__BSD_VISIBLE - 0L) > 0) \
	&& defined(__ISO_C_VISIBLE) && ((__ISO_C_VISIBLE - 0L) > 0)
#error conflicting symbols found
#endif
],[cf_cv_posix_visible=no],[cf_cv_posix_visible=yes])
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_AR version: 1 updated: 2009/01/01 20:15:22
dnl ----------
dnl Check for archiver "ar".
AC_DEFUN([CF_PROG_AR],[
AC_CHECK_TOOL(AR, ar, ar)
])
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC version: 5 updated: 2019/12/31 08:53:54
dnl ----------
dnl standard check for CC, plus followup sanity checks
dnl $1 = optional parameter to pass to AC_PROG_CC to specify compiler name
AC_DEFUN([CF_PROG_CC],[
CF_ACVERSION_CHECK(2.53,
	[AC_MSG_WARN(this will incorrectly handle gnatgcc choice)
	 AC_REQUIRE([AC_PROG_CC])],
	[])
ifelse($1,,[AC_PROG_CC],[AC_PROG_CC($1)])
CF_GCC_VERSION
CF_ACVERSION_CHECK(2.52,
	[AC_PROG_CC_STDC],
	[CF_ANSI_CC_REQD])
CF_CC_ENV_FLAGS
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_EXT version: 15 updated: 2021/01/02 09:31:20
dnl -----------
dnl Compute $PROG_EXT, used for non-Unix ports, such as OS/2 EMX.
AC_DEFUN([CF_PROG_EXT],
[
AC_REQUIRE([CF_CHECK_CACHE])
case "$cf_cv_system_name" in
os2*)
	CFLAGS="$CFLAGS -Zmt"
	CF_APPEND_TEXT(CPPFLAGS,-D__ST_MT_ERRNO__)
	CXXFLAGS="$CXXFLAGS -Zmt"
	# autoconf's macro sets -Zexe and suffix both, which conflict:w
	LDFLAGS="$LDFLAGS -Zmt -Zcrtdll"
	ac_cv_exeext=.exe
	;;
esac

AC_EXEEXT
AC_OBJEXT

PROG_EXT="$EXEEXT"
AC_SUBST(PROG_EXT)
test -n "$PROG_EXT" && AC_DEFINE_UNQUOTED(PROG_EXT,"$PROG_EXT",[Define to the program extension (normally blank)])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_SUM_R version: 3 updated: 2019/12/31 20:39:42
dnl -------------
dnl See if sum can take -r
AC_DEFUN([CF_PROG_SUM_R],
[
if test $ac_cv_path_PATH_SUM
then
AC_MSG_CHECKING([if $ac_cv_path_PATH_SUM takes -r])
AC_CACHE_VAL(ac_cv_prog_sum_r,[
if AC_TRY_COMMAND($ac_cv_path_PATH_SUM -r config.log 1>&AC_FD_CC)
then
	ac_cv_prog_sum_r=yes
else
	ac_cv_prog_sum_r=no
fi
])
if test $ac_cv_prog_sum_r = yes; then
	AC_DEFINE(SUM_TAKES_DASH_R,1,[Define this if the sum command support -r option])
	AC_DEFINE_UNQUOTED(PATH_SUM_R, "$ac_cv_path_PATH_SUM -r", [Define this to the sum command, with -r option if supported])
else
	AC_DEFINE_UNQUOTED(PATH_SUM_R, "$ac_cv_path_PATH_SUM")
fi
AC_MSG_RESULT($ac_cv_prog_sum_r)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_YACC version: 2 updated: 1997/12/16 11:20:41
dnl ------------
dnl A better version of AC_PROC_YACC, verifies that we'll only choose bison if
dnl we'll be able to compile with it.  Bison uses alloca, which isn't all that
dnl portable.
AC_DEFUN([CF_PROG_YACC],
[
AC_REQUIRE([AC_PROG_CC])
AC_CACHE_VAL(cf_cv_prog_YACC,[
if test -n "$YACC" ; then
  cf_cv_prog_YACC="$YACC" # Let the user override the test.
else
cat >conftest.y <<EOF
%{
void yyerror(s) char *s; { }
%}
%token	NUMBER
%%
time	: NUMBER ':' NUMBER
	;
%%
int yylex() { return NUMBER; }
int main() { return yyparse(); }
EOF
  for cf_prog in 'bison -y' byacc yacc
  do
    rm -f y.tab.[ch]
    AC_MSG_CHECKING(for $cf_prog)
    cf_command="$cf_prog conftest.y"
    cf_result=no
    if AC_TRY_EVAL(cf_command) && test -s y.tab.c ; then
      mv y.tab.c conftest.c
      rm -f y.tab.h
      if test "$cf_prog" = 'bison -y' ; then
        if AC_TRY_EVAL(ac_link) && test -s conftest ; then
          cf_result=yes
        fi
      else
        cf_result=yes
      fi
    fi
    AC_MSG_RESULT($cf_result)
    if test $cf_result = yes ; then
      cf_cv_prog_YACC="$cf_prog"
      break
    fi
  done
fi
])
YACC="$cf_cv_prog_YACC"
AC_SUBST(YACC)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PW_GECOS version: 3 updated: 2019/12/31 20:39:42
dnl -----------
dnl Check if the passwd-struct defines the '.pw_gecos' member (useful
dnl in decoding user names).
AC_DEFUN([CF_PW_GECOS],
[
AC_CACHE_CHECK([for passwd.pw_gecos], cf_cv_pw_gecos,[
	AC_TRY_COMPILE([
#include <pwd.h>
],[
	struct passwd foo;
	char bar = foo.pw_gecos],
	[cf_cv_pw_gecos=yes],
	[cf_cv_pw_gecos=no])])
test $cf_cv_pw_gecos = no && AC_DEFINE(DONT_HAVE_PW_GECOS,1,[Define this to 1 if passwd struct has .pw_gecos])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RECHECK_FUNC version: 3 updated: 2000/10/18 19:29:13
dnl ---------------
dnl Re-check on a function to see if we can pick it up by adding a library.
dnl	$1 = function to check
dnl	$2 = library to check in
dnl	$3 = environment to update (e.g., $LIBS)
dnl	$4 = what to do if this fails
dnl
dnl This uses 'unset' if the shell happens to support it, but leaves the
dnl configuration variable set to 'unknown' if not.  This is a little better
dnl than the normal autoconf test, which gives misleading results if a test
dnl for the function is made (e.g., with AC_CHECK_FUNC) after this macro is
dnl used (autoconf does not distinguish between a null token and one that is
dnl set to 'no').
AC_DEFUN([CF_RECHECK_FUNC],[
AC_CHECK_LIB($2,$1,[
	CF_UPPER(cf_tr_func,$1)
	AC_DEFINE_UNQUOTED(HAVE_$cf_tr_func)
	ac_cv_func_$1=yes
	if test "$cf_used_lib_$2" != yes ; then cf_used_lib_$2=yes; $3="-l$2 [$]$3"; fi],[
	ac_cv_func_$1=unknown
	unset ac_cv_func_$1 2>/dev/null
	$4],
	[[$]$3])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 3 updated: 2010/01/09 11:05:50
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[$]//g'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RESTORE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:47:45
dnl ---------------------
dnl Restore flags saved in CF_SAVE_XTRA_FLAGS
dnl $1 = name of current macro
define([CF_RESTORE_XTRA_FLAGS],
[
LIBS="$cf_save_LIBS_$1"
CFLAGS="$cf_save_CFLAGS_$1"
CPPFLAGS="$cf_save_CPPFLAGS_$1"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SAVE_XTRA_FLAGS version: 1 updated: 2020/01/11 16:46:44
dnl ------------------
dnl Use this macro to save CFLAGS/CPPFLAGS/LIBS before checks against X headers
dnl and libraries which do not update those variables.
dnl
dnl $1 = name of current macro
define([CF_SAVE_XTRA_FLAGS],
[
cf_save_LIBS_$1="$LIBS"
cf_save_CFLAGS_$1="$CFLAGS"
cf_save_CPPFLAGS_$1="$CPPFLAGS"
LIBS="$LIBS ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS}"
for cf_X_CFLAGS in $X_CFLAGS
do
	case "x$cf_X_CFLAGS" in
	x-[[IUD]]*)
		CPPFLAGS="$CPPFLAGS $cf_X_CFLAGS"
		;;
	*)
		CFLAGS="$CFLAGS $cf_X_CFLAGS"
		;;
	esac
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIGWINCH version: 6 updated: 2021/01/01 13:31:04
dnl -----------
dnl Use this macro after CF_XOPEN_SOURCE, but do not require it (not all
dnl programs need this test).
dnl
dnl This is really a Mac OS X 10.4.3 workaround.  Defining _POSIX_C_SOURCE
dnl forces SIGWINCH to be undefined (breaks xterm, ncurses).  Oddly, the struct
dnl winsize declaration is left alone - we may revisit this if Apple choose to
dnl break that part of the interface as well.
AC_DEFUN([CF_SIGWINCH],
[
AC_CACHE_CHECK(if SIGWINCH is defined,cf_cv_define_sigwinch,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=yes],
	[AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH; (void)x],
	[cf_cv_define_sigwinch=maybe],
	[cf_cv_define_sigwinch=no])
])
])

if test "$cf_cv_define_sigwinch" = maybe ; then
AC_CACHE_CHECK(for actual SIGWINCH definition,cf_cv_fixup_sigwinch,[
cf_cv_fixup_sigwinch=unknown
cf_sigwinch=32
while test "$cf_sigwinch" != 1
do
	AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[
#if SIGWINCH != $cf_sigwinch
make an error
#endif
int x = SIGWINCH; (void)x],
	[cf_cv_fixup_sigwinch=$cf_sigwinch
	 break])

cf_sigwinch="`expr "$cf_sigwinch" - 1`"
done
])

	if test "$cf_cv_fixup_sigwinch" != unknown ; then
		CPPFLAGS="$CPPFLAGS -DSIGWINCH=$cf_cv_fixup_sigwinch"
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIG_ARGS version: 3 updated: 2019/12/31 20:39:42
dnl -----------
dnl Check for systems that have signal-handlers prototyped with one argument
dnl versus those with more than one argument, define the symbol SIG_ARGS to
dnl match.  (If it's empty, that's ok too).
AC_DEFUN([CF_SIG_ARGS],
[
AC_MSG_CHECKING([declaration of signal arguments])
AC_CACHE_VAL(cf_cv_sig_args,[
cf_cv_sig_args=
for cf_test in "int sig" "int sig, ..."
do
	AC_TRY_COMPILE([
#include <signal.h>],
	[extern RETSIGTYPE catch($cf_test); signal(SIGINT, catch)],
	[cf_cv_sig_args="$cf_test";break])
done
])
AC_MSG_RESULT($cf_cv_sig_args)
AC_DEFINE_UNQUOTED(SIG_ARGS,$cf_cv_sig_args,[Define this to 1 if signal handlers are prototyped with more than one argument])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIG_CONST version: 5 updated: 2019/12/31 20:39:42
dnl ------------
dnl Check for systems where the special signal constants aren't prototyped
dnl (there's a lot of them, and the compiler can generate a lot of warning
dnl messages that make it hard to pick out genuine errors).
AC_DEFUN([CF_SIG_CONST],
[
AC_REQUIRE([CF_SIG_ARGS])
AC_MSG_CHECKING([for redefinable signal constants])
AC_CACHE_VAL(cf_cv_sig_const,[
cf_cv_sig_const=no
if test -n "$cf_cv_sig_args"; then
	cf_test=`echo $cf_cv_sig_args|sed -e s/sig//`
	AC_TRY_RUN([
#define NEW_DFL	((RETSIGTYPE (*)($cf_test))0)
#define NEW_IGN	((RETSIGTYPE (*)($cf_test))1)
#define NEW_ERR	((RETSIGTYPE (*)($cf_test))-1)

#include <signal.h>

int main()
{
	if (NEW_DFL != SIG_DFL
	 || NEW_IGN != SIG_IGN
	 || NEW_ERR != SIG_ERR
	 /* at least one system won't let me redefine these! */
#undef SIG_DFL
#undef SIG_IGN
#undef SIG_ERR
#define SIG_DFL NEW_DFL
#define SIG_IGN NEW_IGN
#define SIG_ERR NEW_ERR
	 || NEW_DFL != SIG_DFL)
	 	${cf_cv_main_return:-return}(1);
	signal(SIGINT, SIG_DFL);
	${cf_cv_main_return:-return}(0);
}],
	[cf_cv_sig_const=yes],
	[cf_cv_sig_const=no],
	[cf_cv_sig_const=unknown])
fi
])
AC_MSG_RESULT($cf_cv_sig_const)
test "$cf_cv_sig_const" = yes && AC_DEFINE(DECL_SIG_CONST,1,[Define this to 1 for redefinable signal constants])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIZECHANGE version: 17 updated: 2021/01/01 13:31:04
dnl -------------
dnl Check for definitions & structures needed for window size-changing
dnl
dnl https://stackoverflow.com/questions/18878141/difference-between-structures-ttysize-and-winsize/50769952#50769952
AC_DEFUN([CF_SIZECHANGE],
[
AC_REQUIRE([CF_STRUCT_TERMIOS])
AC_CACHE_CHECK(declaration of size-change, cf_cv_sizechange,[
	cf_cv_sizechange=unknown
	cf_save_CPPFLAGS="$CPPFLAGS"

for cf_opts in "" "NEED_PTEM_H"
do

	CPPFLAGS="$cf_save_CPPFLAGS"
	if test -n "$cf_opts"
	then
		CF_APPEND_TEXT(CPPFLAGS,-D$cf_opts)
	fi
	AC_TRY_COMPILE([#include <sys/types.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#else
#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif
#endif

#ifdef NEED_PTEM_H
/* This is a workaround for SCO:  they neglected to define struct winsize in
 * termios.h -- it's only in termio.h and ptem.h
 */
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
],[
#ifdef TIOCGSIZE
	struct ttysize win;	/* SunOS 3.0... */
	int y = win.ts_lines = 2;
	int x = win.ts_cols = 1;
	(void)y;
	(void)x;
#else
#ifdef TIOCGWINSZ
	struct winsize win;	/* everything else */
	int y = win.ws_row = 2;
	int x = win.ws_col = 1;
	(void)y;
	(void)x;
#else
	no TIOCGSIZE or TIOCGWINSZ
#endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */
	],
	[cf_cv_sizechange=yes],
	[cf_cv_sizechange=no])

	CPPFLAGS="$cf_save_CPPFLAGS"
	if test "$cf_cv_sizechange" = yes ; then
		echo "size-change succeeded ($cf_opts)" >&AC_FD_CC
		test -n "$cf_opts" && cf_cv_sizechange="$cf_opts"
		break
	fi
done
])
if test "$cf_cv_sizechange" != no ; then
	AC_DEFINE(HAVE_SIZECHANGE,1,[Define to 1 if sizechange declarations are provided])
	case "$cf_cv_sizechange" in
	NEED*)
		AC_DEFINE_UNQUOTED($cf_cv_sizechange )
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SOCKS version: 9 updated: 2012/11/08 20:57:52
dnl --------
dnl Check for socks library
dnl $1 = the [optional] directory in which the library may be found
AC_DEFUN([CF_SOCKS],[
  CF_ADD_OPTIONAL_PATH($1, [socks library])
  CF_FIND_LINKAGE([
#include <stdio.h>
],[
      Raccept((char *)0)
],
      socks)

  if test "x$cf_cv_find_linkage_socks" = "xyes" ; then
    AC_DEFINE(SOCKS,1,[Define to 1 if we are using socks library])

    AC_DEFINE(accept,Raccept,[Define to override function name if using socks library])
    AC_DEFINE(bind,Rbind,[Define to override function name if using socks library])
    AC_DEFINE(connect,Rconnect,[Define to override function name if using socks library])
    AC_DEFINE(getpeername,Rgetpeername,[Define to override function name if using socks library])
    AC_DEFINE(getsockname,Rgetsockname,[Define to override function name if using socks library])
    AC_DEFINE(listen,Rlisten,[Define to override function name if using socks library])
    AC_DEFINE(recvfrom,Rrecvfrom,[Define to override function name if using socks library])
    AC_DEFINE(select,Rselect,[Define to override function name if using socks library])
  else
    AC_MSG_ERROR(cannot link with socks library)
  fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SOCKS5 version: 12 updated: 2012/11/08 20:57:52
dnl ---------
dnl Check for socks5 configuration
dnl $1 = the [optional] directory in which the library may be found
AC_DEFUN([CF_SOCKS5],[
  CF_ADD_OPTIONAL_PATH($1, [socks5 library])

CF_ADD_LIBS(-lsocks5)

AC_DEFINE(USE_SOCKS5,1,[Define to 1 if we are using socks5 library])
AC_DEFINE(SOCKS,1,[Define to 1 if we are using socks library])

AC_MSG_CHECKING(if the socks library uses socks4 prefix)
cf_use_socks4=error
AC_TRY_LINK([
#include <socks.h>],[
	Rinit((char *)0)],
	[AC_DEFINE(USE_SOCKS4_PREFIX,1,[Define to 1 if socks library uses socks4 prefix])
	 cf_use_socks4=yes],
	[AC_TRY_LINK([#include <socks.h>],
		[SOCKSinit((char *)0)],
		[cf_use_socks4=no],
		[AC_MSG_ERROR(Cannot link with socks5 library)])])
AC_MSG_RESULT($cf_use_socks4)

if test "$cf_use_socks4" = "yes" ; then
	AC_DEFINE(accept,Raccept)
	AC_DEFINE(bind,Rbind)
	AC_DEFINE(connect,Rconnect)
	AC_DEFINE(getpeername,Rgetpeername)
	AC_DEFINE(getsockname,Rgetsockname)
	AC_DEFINE(listen,Rlisten)
	AC_DEFINE(recvfrom,Rrecvfrom)
	AC_DEFINE(select,Rselect)
else
	AC_DEFINE(accept,SOCKSaccept)
	AC_DEFINE(getpeername,SOCKSgetpeername)
	AC_DEFINE(getsockname,SOCKSgetsockname)
	AC_DEFINE(recvfrom,SOCKSrecvfrom)
fi

AC_MSG_CHECKING(if socks5p.h is available)
AC_TRY_COMPILE([
#define INCLUDE_PROTOTYPES
#include <socks.h>],[
	init((char *)0)],
	[cf_use_socks5p_h=yes],
	[cf_use_socks5p_h=no])
AC_MSG_RESULT($cf_use_socks5p_h)

test "$cf_use_socks5p_h" = yes && AC_DEFINE(INCLUDE_PROTOTYPES,1,[Define to 1 if needed to declare prototypes in socks headers])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRERROR version: 2 updated: 2001/07/11 09:34:49
dnl -----------
dnl Check for strerror(), or it is not found, for the related data.  POSIX
dnl requires strerror(), so only old systems such as SunOS lack it.
AC_DEFUN([CF_STRERROR],[
AC_CHECK_FUNCS(strerror, AC_DEFINE(HAVE_STRERROR),[CF_SYS_ERRLIST])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRUCT_TERMIOS version: 11 updated: 2020/03/19 20:46:13
dnl -----------------
dnl Some machines require _POSIX_SOURCE to completely define struct termios.
AC_DEFUN([CF_STRUCT_TERMIOS],[
AC_REQUIRE([CF_XOPEN_SOURCE])

AC_CHECK_HEADERS( \
termio.h \
termios.h \
unistd.h \
sys/ioctl.h \
sys/termio.h \
)

if test "$ac_cv_header_termios_h" = yes ; then
	case "$CFLAGS $CPPFLAGS" in
	*-D_POSIX_SOURCE*)
		termios_bad=dunno ;;
	*)	termios_bad=maybe ;;
	esac
	if test "$termios_bad" = maybe ; then
	AC_MSG_CHECKING(whether termios.h needs _POSIX_SOURCE)
	AC_TRY_COMPILE([#include <termios.h>],
		[struct termios foo; int x = foo.c_iflag = 1; (void)x],
		termios_bad=no, [
		AC_TRY_COMPILE([
#define _POSIX_SOURCE
#include <termios.h>],
			[struct termios foo; int x = foo.c_iflag = 2; (void)x],
			termios_bad=unknown,
			termios_bad=yes AC_DEFINE(_POSIX_SOURCE,1,[Define to 1 if we must define _POSIX_SOURCE]))
			])
	AC_MSG_RESULT($termios_bad)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBDIR_PATH version: 7 updated: 2014/12/04 04:33:06
dnl --------------
dnl Construct a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
AC_DEFUN([CF_SUBDIR_PATH],
[
$1=

CF_ADD_SUBDIR_PATH($1,$2,$3,$prefix,NONE)

for cf_subdir_prefix in \
	/usr \
	/usr/local \
	/usr/pkg \
	/opt \
	/opt/local \
	[$]HOME
do
	CF_ADD_SUBDIR_PATH($1,$2,$3,$cf_subdir_prefix,$prefix)
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_ERRLIST version: 6 updated: 2001/12/30 13:03:23
dnl --------------
dnl Check for declaration of sys_nerr and sys_errlist in one of stdio.h and
dnl errno.h.  Declaration of sys_errlist on BSD4.4 interferes with our
dnl declaration.  Reported by Keith Bostic.
AC_DEFUN([CF_SYS_ERRLIST],
[
    CF_CHECK_ERRNO(sys_nerr)
    CF_CHECK_ERRNO(sys_errlist)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_SELECT_TIMEVAL version: 5 updated: 2019/12/31 20:39:42
dnl ---------------------
dnl Check if there is a conflict between <sys/select.h> and <sys/time.h>.
dnl This is known to be a problem with SCO.
AC_DEFUN([CF_SYS_SELECT_TIMEVAL],
[
AC_MSG_CHECKING(if sys/time.h conflicts with sys/select.h)
AC_CACHE_VAL(cf_cv_sys_select_timeval,[
for cf_opts in no yes
do
AC_TRY_COMPILE([
#define yes 1
#define no 0
#if $cf_opts
#define timeval fake_timeval
#endif
#include <sys/types.h>
#ifdef TIME_WITH_SYS_TIME
#	include <sys/time.h>
#	include <time.h>
#else
#	ifdef HAVE_SYS_TIME_H
#		include <sys/time.h>
#	else
#		include <time.h>
#	endif
#endif
#undef timeval
#ifdef HAVE_SYS_SELECT_H
#	include <sys/select.h>
#endif
],[struct timeval foo],
	[cf_cv_sys_select_timeval=$cf_opts
	 break],
	[cf_cv_sys_select_timeval=no])
done
])
AC_MSG_RESULT($cf_cv_sys_select_timeval)
test $cf_cv_sys_select_timeval = yes && AC_DEFINE(NEED_TIMEVAL_FIX,1,[Define this to 1 if sys/time.h conflicts with sys/select.h])
])
dnl ---------------------------------------------------------------------------
dnl CF_TERMCAP_LIBS version: 15 updated: 2015/04/15 19:08:48
dnl ---------------
dnl Look for termcap libraries, or the equivalent in terminfo.
dnl
dnl The optional parameter may be "ncurses", "ncursesw".
AC_DEFUN([CF_TERMCAP_LIBS],
[
AC_CACHE_VAL(cf_cv_termlib,[
cf_cv_termlib=none
AC_TRY_LINK([],[char *x=(char*)tgoto("",0,0)],
[AC_TRY_LINK([],[int x=tigetstr("")],
	[cf_cv_termlib=terminfo],
	[cf_cv_termlib=termcap])
	CF_VERBOSE(using functions in predefined $cf_cv_termlib LIBS)
],[
ifelse([$1],,,[
case "$1" in
ncurses*)
	CF_NCURSES_CONFIG($1)
	cf_cv_termlib=terminfo
	;;
esac
])
if test "$cf_cv_termlib" = none; then
	# FreeBSD's linker gives bogus results for AC_CHECK_LIB, saying that
	# tgetstr lives in -lcurses when it is only an unsatisfied extern.
	cf_save_LIBS="$LIBS"
	for cf_lib in tinfo curses ncurses termlib termcap
	do
		LIBS="-l$cf_lib $cf_save_LIBS"
		for cf_func in tigetstr tgetstr
		do
			AC_MSG_CHECKING(for $cf_func in -l$cf_lib)
			AC_TRY_LINK([],[int x=$cf_func("")],[cf_result=yes],[cf_result=no])
			AC_MSG_RESULT($cf_result)
			if test "$cf_result" = yes ; then
				if test "$cf_func" = tigetstr ; then
					cf_cv_termlib=terminfo
				else
					cf_cv_termlib=termcap
				fi
				break
			fi
		done
		test "$cf_result" = yes && break
	done
	test "$cf_result" = no && LIBS="$cf_save_LIBS"
fi
if test "$cf_cv_termlib" = none; then
	# allow curses library for broken AIX system.
	AC_CHECK_LIB(curses, initscr, [CF_ADD_LIBS(-lcurses)])
	AC_CHECK_LIB(termcap, tgoto, [CF_ADD_LIBS(-ltermcap) cf_cv_termlib=termcap])
fi
])
if test "$cf_cv_termlib" = none; then
	AC_MSG_WARN([Cannot find -ltermlib, -lcurses, or -ltermcap])
fi
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERMIOS version: 3 updated: 2019/12/31 20:39:42
dnl ----------
dnl See if we can link with the termios functions tcsetattr/tcgetattr
AC_DEFUN([CF_TERMIOS],
[
AC_MSG_CHECKING([for nonconflicting termios.h])
AC_CACHE_VAL(cf_cv_use_termios_h,[
	AC_TRY_LINK([
#ifdef HAVE_IOCTL_H
#	include <ioctl.h>
#else
#	ifdef HAVE_SYS_IOCTL_H
#		include <sys/ioctl.h>
#	endif
#endif

#if !defined(sun) || !defined(NL0)
#include <termios.h>
#endif
],[
	struct termios save_tty;
	(void) tcsetattr (0, TCSANOW, &save_tty);
	(void) tcgetattr (0, &save_tty)],
	[cf_cv_use_termios_h=yes],
	[cf_cv_use_termios_h=no])
])
AC_MSG_RESULT($cf_cv_use_termios_h)
if test $cf_cv_use_termios_h = yes; then
	AC_DEFINE(HAVE_TERMIOS_H,1,[Define this to 1 if we have header termios.h])
	AC_DEFINE(HAVE_TCGETATTR,1,[Define this to 1 if we have function tcgetattr])
	AC_DEFINE(HAVE_TCSETATTR,1,[Define this to 1 if we have function tcsetattr])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TERM_HEADER version: 6 updated: 2021/01/02 09:31:20
dnl --------------
dnl Look for term.h, which is part of X/Open curses.  It defines the interface
dnl to terminfo database.  Usually it is in the same include-path as curses.h,
dnl but some packagers change this, breaking various applications.
AC_DEFUN([CF_TERM_HEADER],[
AC_CACHE_CHECK(for terminfo header, cf_cv_term_header,[
case "${cf_cv_ncurses_header}" in
*/ncurses.h|*/ncursesw.h)
	cf_term_header=`echo "$cf_cv_ncurses_header" | sed -e 's%ncurses[[^.]]*\.h$%term.h%'`
	;;
*)
	cf_term_header=term.h
	;;
esac

for cf_test in $cf_term_header "ncurses/term.h" "ncursesw/term.h"
do
AC_TRY_COMPILE([#include <stdio.h>
#include <${cf_cv_ncurses_header:-curses.h}>
#include <$cf_test>
],[int x = auto_left_margin; (void)x],[
	cf_cv_term_header="$cf_test"],[
	cf_cv_term_header=unknown
	])
	test "$cf_cv_term_header" != unknown && break
done
])

# Set definitions to allow ifdef'ing to accommodate subdirectories

case "$cf_cv_term_header" in
*term.h)
	AC_DEFINE(HAVE_TERM_H,1,[Define to 1 if we have term.h])
	;;
esac

case "$cf_cv_term_header" in
ncurses/term.h)
	AC_DEFINE(HAVE_NCURSES_TERM_H,1,[Define to 1 if we have ncurses/term.h])
	;;
ncursesw/term.h)
	AC_DEFINE(HAVE_NCURSESW_TERM_H,1,[Define to 1 if we have ncursesw/term.h])
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TIOCGWINSZ version: 6 updated: 2019/12/31 20:39:42
dnl -------------
dnl On some systems ioctl(fd, TIOCGWINSZ, &size) will always return {0,0} until
dnl ioctl(fd, TIOCSWINSZ, &size) is called to explicitly set the size of the
dnl screen.
dnl
dnl Attempt to determine if we're on such a system by running a test-program.
dnl This won't work, of course, if the configure script is run in batch mode,
dnl since we've got to have access to the terminal.
dnl
dnl CHECK_DECL_FLAG and CHECK_DECL_HDRS must be set in configure.in
AC_DEFUN([CF_TIOCGWINSZ],
[
AC_MSG_CHECKING([for working TIOCGWINSZ])
AC_CACHE_VAL(cf_cv_use_tiocgwinsz,[
	cf_save_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -D__CPROTO__ $CHECK_DECL_FLAG"
	AC_TRY_RUN([
$CHECK_DECL_HDRS
int main()
{
	int fd;
	for (fd = 0; fd <= 2; fd++) {	/* try in/out/err in case redirected */
#ifdef TIOCGSIZE
		struct ttysize size;
		if (ioctl (0, TIOCGSIZE, &size) == 0
		 && size.ts_lines > 0
		 && size.ts_cols > 0)
			${cf_cv_main_return:-return}(0);
#else
		struct winsize size;
		if (ioctl(0, TIOCGWINSZ, &size) == 0
		 && size.ws_row > 0
		 && size.ws_col > 0)
			${cf_cv_main_return:-return}(0);
#endif
	}
	${cf_cv_main_return:-return}(0);	/* we cannot guarantee this is run interactively */
}],
		[cf_cv_use_tiocgwinsz=yes],
		[cf_cv_use_tiocgwinsz=no],
		[cf_cv_use_tiocgwinsz=unknown])
		rm -f autoconf.h
		CFLAGS="$cf_save_CFLAGS"])
AC_MSG_RESULT($cf_cv_use_tiocgwinsz)
test $cf_cv_use_tiocgwinsz != yes && AC_DEFINE(DONT_HAVE_SIGWINCH,1,[Define this to 1 for working TIOCGWINSZ])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TM_GMTOFF version: 4 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check if the tm-struct defines the '.tm_gmtoff' member (useful in decoding
dnl dates).
AC_DEFUN([CF_TM_GMTOFF],
[
AC_MSG_CHECKING([for tm.tm_gmtoff])
AC_CACHE_VAL(cf_cv_tm_gmtoff,[
	AC_TRY_COMPILE([
#ifdef TIME_WITH_SYS_TIME
#	include <sys/time.h>
#	include <time.h>
#else
#	ifdef HAVE_SYS_TIME_H
#		include <sys/time.h>
#	else
#		include <time.h>
#	endif
#endif
],[
	struct tm foo;
	long bar = foo.tm_gmtoff],
	[cf_cv_tm_gmtoff=yes],
	[cf_cv_tm_gmtoff=no])])
AC_MSG_RESULT($cf_cv_tm_gmtoff)
test "$cf_cv_tm_gmtoff" = no && AC_DEFINE(DONT_HAVE_TM_GMTOFF,1,[Define to 1 if the tm-struct defines .tm_gmtoff member])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TRIM_X_LIBS version: 3 updated: 2015/04/12 15:39:00
dnl --------------
dnl Trim extra base X libraries added as a workaround for inconsistent library
dnl dependencies returned by "new" pkg-config files.
AC_DEFUN([CF_TRIM_X_LIBS],[
	for cf_trim_lib in Xmu Xt X11
	do
		case "$LIBS" in
		*-l$cf_trim_lib\ *-l$cf_trim_lib*)
			LIBS=`echo "$LIBS " | sed -e 's/  / /g' -e 's%-l'"$cf_trim_lib"' %%' -e 's/ $//'`
			CF_VERBOSE(..trimmed $LIBS)
			;;
		esac
	done
])
dnl ---------------------------------------------------------------------------
dnl CF_TRY_PKG_CONFIG version: 6 updated: 2020/12/31 10:54:15
dnl -----------------
dnl This is a simple wrapper to use for pkg-config, for libraries which may be
dnl available in that form.
dnl
dnl $1 = package name, which may be a shell variable
dnl $2 = extra logic to use, if any, after updating CFLAGS and LIBS
dnl $3 = logic to use if pkg-config does not have the package
AC_DEFUN([CF_TRY_PKG_CONFIG],[
AC_REQUIRE([CF_PKG_CONFIG])

if test "$PKG_CONFIG" != none && "$PKG_CONFIG" --exists "$1"; then
	CF_VERBOSE(found package $1)
	cf_pkgconfig_incs="`$PKG_CONFIG --cflags "$1" 2>/dev/null`"
	cf_pkgconfig_libs="`$PKG_CONFIG --libs   "$1" 2>/dev/null`"
	CF_VERBOSE(package $1 CFLAGS: $cf_pkgconfig_incs)
	CF_VERBOSE(package $1 LIBS: $cf_pkgconfig_libs)
	CF_ADD_CFLAGS($cf_pkgconfig_incs)
	CF_ADD_LIBS($cf_pkgconfig_libs)
	ifelse([$2],,:,[$2])
else
	cf_pkgconfig_incs=
	cf_pkgconfig_libs=
	ifelse([$3],,:,[$3])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TRY_XOPEN_SOURCE version: 2 updated: 2018/06/20 20:23:13
dnl -------------------
dnl If _XOPEN_SOURCE is not defined in the compile environment, check if we
dnl can define it successfully.
AC_DEFUN([CF_TRY_XOPEN_SOURCE],[
AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE([
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CF_APPEND_TEXT(CPPFLAGS,-D_XOPEN_SOURCE=$cf_XOPEN_SOURCE)
	 AC_TRY_COMPILE([
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
	CPPFLAGS="$cf_save"
	])
])

if test "$cf_cv_xopen_source" != no ; then
	CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
	CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
	cf_temp_xopen_source="-D_XOPEN_SOURCE=$cf_cv_xopen_source"
	CF_ADD_CFLAGS($cf_temp_xopen_source)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_FD_SET version: 6 updated: 2020/03/10 18:53:47
dnl --------------
dnl Check for the declaration of fd_set.  Some platforms declare it in
dnl <sys/types.h>, and some in <sys/select.h>, which requires <sys/types.h>.
dnl Finally, if we are using this for an X application, Xpoll.h may include
dnl <sys/select.h>, so we don't want to do it twice.
AC_DEFUN([CF_TYPE_FD_SET],
[
AC_CHECK_HEADERS(X11/Xpoll.h)

AC_CACHE_CHECK(for declaration of fd_set,cf_cv_type_fd_set,
	[CF_MSG_LOG(sys/types alone)
AC_TRY_COMPILE([
#include <sys/types.h>],
	[fd_set x; (void)x],
	[cf_cv_type_fd_set=sys/types.h],
	[CF_MSG_LOG(X11/Xpoll.h)
AC_TRY_COMPILE([
#ifdef HAVE_X11_XPOLL_H
#include <X11/Xpoll.h>
#endif],
	[fd_set x; (void)x],
	[cf_cv_type_fd_set=X11/Xpoll.h],
	[CF_MSG_LOG(sys/select.h)
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/select.h>],
	[fd_set x; (void)x],
	[cf_cv_type_fd_set=sys/select.h],
	[cf_cv_type_fd_set=unknown])])])])
if test $cf_cv_type_fd_set = sys/select.h ; then
	AC_DEFINE(USE_SYS_SELECT_H,1,[Define to 1 to include sys/select.h to declare fd_set])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_OUTCHAR version: 15 updated: 2015/05/15 19:42:24
dnl ---------------
dnl Check for return and param type of 3rd -- OutChar() -- param of tputs().
dnl
dnl For this check, and for CF_CURSES_TERMCAP, the $CHECK_DECL_HDRS variable
dnl must point to a header file containing this (or equivalent):
dnl
dnl	#ifdef NEED_CURSES_H
dnl	# ifdef HAVE_NCURSES_NCURSES_H
dnl	#  include <ncurses/ncurses.h>
dnl	# else
dnl	#  ifdef HAVE_NCURSES_H
dnl	#   include <ncurses.h>
dnl	#  else
dnl	#   include <curses.h>
dnl	#  endif
dnl	# endif
dnl	#endif
dnl
dnl	#ifdef HAVE_NCURSES_TERM_H
dnl	#  include <ncurses/term.h>
dnl	#else
dnl	# ifdef HAVE_TERM_H
dnl	#  include <term.h>
dnl	# endif
dnl	#endif
dnl
dnl	#if NEED_TERMCAP_H
dnl	# include <termcap.h>
dnl	#endif
dnl
AC_DEFUN([CF_TYPE_OUTCHAR],
[
AC_REQUIRE([CF_CURSES_TERMCAP])

AC_CACHE_CHECK(declaration of tputs 3rd param, cf_cv_type_outchar,[

cf_cv_type_outchar="int OutChar(int)"
cf_cv_found=no
cf_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $CHECK_DECL_FLAG"

for P in int void; do
for Q in int void; do
for R in int char; do
for S in "" const; do
	CF_MSG_LOG(loop variables [P:[$]P, Q:[$]Q, R:[$]R, S:[$]S])
	AC_TRY_COMPILE([$CHECK_DECL_HDRS],
	[extern $Q OutChar($R);
	extern $P tputs ($S char *string, int nlines, $Q (*_f)($R));
	tputs("", 1, OutChar)],
	[cf_cv_type_outchar="$Q OutChar($R)"
	 cf_cv_found=yes
	 break])
done
	test $cf_cv_found = yes && break
done
	test $cf_cv_found = yes && break
done
	test $cf_cv_found = yes && break
done
])

case $cf_cv_type_outchar in
int*)
	AC_DEFINE(OUTC_RETURN,1,[Define to 1 if tputs outc function returns a value])
	;;
esac
case $cf_cv_type_outchar in
*char*)
	AC_DEFINE(OUTC_ARGS,char c,[Define to actual type to override tputs outc parameter type])
	;;
esac

CPPFLAGS="$cf_save_CPPFLAGS"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_SIGACTION version: 4 updated: 2012/10/06 17:56:13
dnl -----------------
dnl
AC_DEFUN([CF_TYPE_SIGACTION],
[
AC_MSG_CHECKING([for type sigaction_t])
AC_CACHE_VAL(cf_cv_type_sigaction,[
	AC_TRY_COMPILE([
#include <signal.h>],
		[sigaction_t x],
		[cf_cv_type_sigaction=yes],
		[cf_cv_type_sigaction=no])])
AC_MSG_RESULT($cf_cv_type_sigaction)
test "$cf_cv_type_sigaction" = yes && AC_DEFINE(HAVE_TYPE_SIGACTION,1,[Define to 1 if we have the sigaction_t type])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UNION_WAIT version: 8 updated: 2021/01/02 09:31:20
dnl -------------
dnl Check to see if the BSD-style union wait is declared.  Some platforms may
dnl use this, though it is deprecated in favor of the 'int' type in Posix.
dnl Some vendors provide a bogus implementation that declares union wait, but
dnl uses the 'int' type instead; we try to spot these by checking for the
dnl associated macros.
dnl
dnl Ahem.  Some implementers cast the status value to an int*, as an attempt to
dnl use the macros for either union wait or int.  So we do a check compile to
dnl see if the macros are defined and apply to an int.
dnl
dnl Sets: $cf_cv_type_unionwait
dnl Defines: HAVE_TYPE_UNIONWAIT
AC_DEFUN([CF_UNION_WAIT],
[
AC_REQUIRE([CF_WAIT_HEADERS])
AC_MSG_CHECKING([for union wait])
AC_CACHE_VAL(cf_cv_type_unionwait,[
	AC_TRY_LINK($cf_wait_headers,
	[int x;
	 int y = WEXITSTATUS(x);
	 int z = WTERMSIG(x);
	 wait(&x);
	 (void)x;
	 (void)y;
	 (void)z;
	],
	[cf_cv_type_unionwait=no
	 echo compiles ok w/o union wait 1>&AC_FD_CC
	],[
	AC_TRY_LINK($cf_wait_headers,
	[union wait x;
#ifdef WEXITSTATUS
	 int y = WEXITSTATUS(x);
#endif
#ifdef WTERMSIG
	 int z = WTERMSIG(x);
#endif
	 wait(&x);
	 (void)x;
#ifdef WEXITSTATUS
	 (void)y;
#endif
#ifdef WTERMSIG
	 (void)z;
#endif
	],
	[cf_cv_type_unionwait=yes
	 echo compiles ok with union wait and possibly macros too 1>&AC_FD_CC
	],
	[cf_cv_type_unionwait=no])])])
AC_MSG_RESULT($cf_cv_type_unionwait)
test "$cf_cv_type_unionwait" = yes && AC_DEFINE(HAVE_TYPE_UNIONWAIT,1,[Define to 1 if type unionwait is declared])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VA_COPY version: 6 updated: 2018/12/04 18:14:25
dnl ----------
dnl check for va_copy, part of stdarg.h starting with ISO C 1999.
dnl Also, workaround for glibc's __va_copy, by checking for both.
dnl Finally, try to accommodate pre-ISO C 1999 headers.
AC_DEFUN([CF_VA_COPY],[
AC_CACHE_CHECK(for va_copy, cf_cv_have_va_copy,[
AC_TRY_LINK([
#include <stdarg.h>
],[
	static va_list dst;
	static va_list src;
	va_copy(dst, src)],
	cf_cv_have_va_copy=yes,
	cf_cv_have_va_copy=no)])

if test "$cf_cv_have_va_copy" = yes;
then
	AC_DEFINE(HAVE_VA_COPY,1,[Define to 1 if we have va_copy])
else # !cf_cv_have_va_copy

AC_CACHE_CHECK(for __va_copy, cf_cv_have___va_copy,[
AC_TRY_LINK([
#include <stdarg.h>
],[
	static va_list dst;
	static va_list src;
	__va_copy(dst, src)],
	cf_cv_have___va_copy=yes,
	cf_cv_have___va_copy=no)])

if test "$cf_cv_have___va_copy" = yes
then
	AC_DEFINE(HAVE___VA_COPY,1,[Define to 1 if we have __va_copy])
else # !cf_cv_have___va_copy

AC_CACHE_CHECK(for __builtin_va_copy, cf_cv_have___builtin_va_copy,[
AC_TRY_LINK([
#include <stdarg.h>
],[
	static va_list dst;
	static va_list src;
	__builtin_va_copy(dst, src)],
	cf_cv_have___builtin_va_copy=yes,
	cf_cv_have___builtin_va_copy=no)])

test "$cf_cv_have___builtin_va_copy" = yes &&
	AC_DEFINE(HAVE___BUILTIN_VA_COPY,1,[Define to 1 if we have __builtin_va_copy])

fi # cf_cv_have___va_copy

fi # cf_cv_have_va_copy

case "${cf_cv_have_va_copy}${cf_cv_have___va_copy}${cf_cv_have___builtin_va_copy}" in
*yes*)
	;;

*)
	AC_CACHE_CHECK(if we can simply copy va_list, cf_cv_pointer_va_list,[
AC_TRY_LINK([
#include <stdarg.h>
],[
	va_list dst;
	va_list src;
	dst = src],
	cf_cv_pointer_va_list=yes,
	cf_cv_pointer_va_list=no)])

	if test "$cf_cv_pointer_va_list" = no
	then
		AC_CACHE_CHECK(if we can copy va_list indirectly, cf_cv_array_va_list,[
AC_TRY_LINK([
#include <stdarg.h>
],[
	va_list dst;
	va_list src;
	*dst = *src],
			cf_cv_array_va_list=yes,
			cf_cv_array_va_list=no)])
		test "$cf_cv_array_va_list" = yes && AC_DEFINE(ARRAY_VA_LIST,1,[Define to 1 if we can copy va_list indirectly])
	fi
	;;
esac
])
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 3 updated: 2007/07/29 09:55:12
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
CF_MSG_LOG([$1])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WAIT_HEADERS version: 3 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Build up an expression $cf_wait_headers with the header files needed to
dnl compile against the prototypes for 'wait()', 'waitpid()', etc.  Assume it's
dnl Posix, which uses <sys/types.h> and <sys/wait.h>, but allow SVr4 variation
dnl with <wait.h>.
AC_DEFUN([CF_WAIT_HEADERS],
[
AC_HAVE_HEADERS(sys/wait.h)
cf_wait_headers="#include <sys/types.h>
"
if test "$ac_cv_header_sys_wait_h" = yes; then
cf_wait_headers="$cf_wait_headers
#include <sys/wait.h>
"
else
AC_HAVE_HEADERS(wait.h)
AC_HAVE_HEADERS(waitstatus.h)
if test "$ac_cv_header_wait_h" = yes; then
cf_wait_headers="$cf_wait_headers
#include <wait.h>
"
fi
if test "$ac_cv_header_waitstatus_h" = yes; then
cf_wait_headers="$cf_wait_headers
#include <waitstatus.h>
"
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_CURSES_DIR version: 4 updated: 2021/01/02 19:22:58
dnl ------------------
dnl Wrapper for AC_ARG_WITH to specify directory under which to look for curses
dnl libraries.
AC_DEFUN([CF_WITH_CURSES_DIR],[

AC_MSG_CHECKING(for specific curses-directory)
AC_ARG_WITH(curses-dir,
	[  --with-curses-dir=DIR   directory in which (n)curses is installed],
	[cf_cv_curses_dir=$withval],
	[cf_cv_curses_dir=no])
AC_MSG_RESULT($cf_cv_curses_dir)

if test -n "$cf_cv_curses_dir" && test "$cf_cv_curses_dir" != "no"
then
	CF_PATH_SYNTAX(withval)
	if test -d "$cf_cv_curses_dir"
	then
		CF_ADD_INCDIR($cf_cv_curses_dir/include)
		CF_ADD_LIBDIR($cf_cv_curses_dir/lib)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DBMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ----------------
dnl Configure-option for dbmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DBMALLOC],[
CF_NO_LEAKS_OPTION(dbmalloc,
	[  --with-dbmalloc         test: use Conor Cahill's dbmalloc library],
	[USE_DBMALLOC])

if test "$with_dbmalloc" = yes ; then
	AC_CHECK_HEADER(dbmalloc.h,
		[AC_CHECK_LIB(dbmalloc,[debug_malloc]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DFTENV version: 6 updated: 2019/12/31 20:39:42
dnl --------------
dnl Wrapper for AC_ARG_WITH to inherit/override an environment variable's
dnl "#define" in the compile.
dnl $1 = option name
dnl $2 = help-message
dnl $3 = name of variable to inherit/override
dnl $4 = default value of variable, if any
AC_DEFUN([CF_WITH_DFTENV],
[AC_ARG_WITH($1,[$2 ](default: ifelse($4,,empty,$4)),,
ifelse($4,,[withval="${$3}"],[withval="${$3-$4}"]))
case "$withval" in #(vi
yes|no)
  AC_MSG_ERROR(expected a value for --with-$1)
  ;;
esac
$3="$withval"
AC_DEFINE_UNQUOTED($3,"[$]$3",[Define $2])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ---------------
dnl Configure-option for dmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DMALLOC],[
CF_NO_LEAKS_OPTION(dmalloc,
	[  --with-dmalloc          test: use Gray Watson's dmalloc library],
	[USE_DMALLOC])

if test "$with_dmalloc" = yes ; then
	AC_CHECK_HEADER(dmalloc.h,
		[AC_CHECK_LIB(dmalloc,[dmalloc_debug]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PATH version: 11 updated: 2012/09/29 15:04:19
dnl ------------
dnl Wrapper for AC_ARG_WITH to ensure that user supplies a pathname, not just
dnl defaulting to yes/no.
dnl
dnl $1 = option name
dnl $2 = help-text
dnl $3 = environment variable to set
dnl $4 = default value, shown in the help-message, must be a constant
dnl $5 = default value, if it's an expression & cannot be in the help-message
dnl
AC_DEFUN([CF_WITH_PATH],
[AC_ARG_WITH($1,[$2 ](default: ifelse([$4],,empty,[$4])),,
ifelse([$4],,[withval="${$3}"],[withval="${$3:-ifelse([$5],,[$4],[$5])}"]))dnl
if ifelse([$5],,true,[test -n "$5"]) ; then
CF_PATH_SYNTAX(withval)
fi
eval $3="$withval"
AC_SUBST($3)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PROGRAM version: 7 updated: 2019/12/31 20:39:42
dnl ---------------
dnl Wrapper for AC_PATH_PROG, with command-line option.
dnl Params:
dnl $1 = program name
dnl $2 = help-string (I'd use format, but someone's disable it in autoconf)
dnl $3 = caller-supplied default if no --with option is given.  If this is
dnl      blank, the macro uses AC_PATH_PROG.
AC_DEFUN([CF_WITH_PROGRAM],
[dnl
define([cf_path_name], PATH_[]translit($1, [a-z], [A-Z]))dnl
define([cf_have_name], HAVE_[]translit($1, [a-z], [A-Z]))dnl
AC_ARG_WITH($1,[$2],ifelse($3,,
[case "$withval" in #(vi
  yes[)]
   AC_MSG_ERROR(expected a value for --with-$1)
   ;; #(vi
  no[)]
   ;; #(vi
  *[)]
   # user supplied option-value for "--with-$1=path"
   AC_MSG_CHECKING(for $1)
   ac_cv_path_]cf_path_name[="$withval"
   AC_DEFINE_UNQUOTED(cf_path_name,"$withval",[Define this to the pathname for $1])dnl
   AC_DEFINE(cf_have_name,1,[Define this to 1 if the $1 program exists])dnl
   AC_MSG_RESULT($withval)
   ;;
 esac],[$3]),[
  # user did not specify "--with-$1"; do automatic check
  AC_PATH_PROG(cf_path_name,$1)
  if test -n "$cf_path_name"; then
    AC_DEFINE_UNQUOTED(cf_path_name,"$cf_path_name")dnl
    AC_DEFINE(cf_have_name)dnl
  fi
])dnl
undefine([cf_path_name])undefine([cf_have_name])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_VALUE version: 4 updated: 2019/12/31 20:39:42
dnl -------------
dnl Wrapper for AC_ARG_WITH to ensure that if the user supplies a value, it is
dnl not simply defaulting to yes/no.  Empty strings are ok if the macro is
dnl invoked without a default value
dnl $1 = option name
dnl $2 = help-message
dnl $3 = variable to inherit/override
dnl $4 = default value, if any.
AC_DEFUN([CF_WITH_VALUE],
[CF_ARG_WITH($1,[$2],[$3],[$4])
 AC_DEFINE_UNQUOTED($3,"$withval",[Define a value for $1])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_CURSES version: 16 updated: 2021/01/02 09:31:20
dnl ---------------
dnl Test if we should define X/Open source for curses, needed on Digital Unix
dnl 4.x, to see the extended functions, but breaks on IRIX 6.x.
dnl
dnl The getbegyx() check is needed for HPUX, which omits legacy macros such
dnl as getbegy().  The latter is better design, but the former is standard.
AC_DEFUN([CF_XOPEN_CURSES],
[
AC_REQUIRE([CF_CURSES_CPPFLAGS])dnl
AC_CACHE_CHECK(definition to turn on extended curses functions,cf_cv_need_xopen_extension,[
cf_cv_need_xopen_extension=unknown
AC_TRY_LINK([
#include <stdlib.h>
#include <${cf_cv_ncurses_header:-curses.h}>],[
#if defined(NCURSES_VERSION_PATCH)
#if (NCURSES_VERSION_PATCH < 20100501) && (NCURSES_VERSION_PATCH >= 20100403)
	make an error
#endif
#endif
#ifdef NCURSES_VERSION
	cchar_t check;
	int check2 = curs_set((int)sizeof(check));
#endif
	long x = winnstr(stdscr, "", 0);
	int x1, y1;
#ifdef NCURSES_VERSION
	(void)check2;
#endif
	getbegyx(stdscr, y1, x1);
	(void)x;
	(void)y1;
	(void)x1;
	],
	[cf_cv_need_xopen_extension=none],
	[
	for cf_try_xopen_extension in _XOPEN_SOURCE_EXTENDED NCURSES_WIDECHAR
	do
		AC_TRY_LINK([
#define $cf_try_xopen_extension 1
#include <stdlib.h>
#include <${cf_cv_ncurses_header:-curses.h}>],[
#ifdef NCURSES_VERSION
		cchar_t check;
		int check2 = curs_set((int)sizeof(check));
#endif
		long x = winnstr(stdscr, "", 0);
		int x1, y1;
		getbegyx(stdscr, y1, x1);
#ifdef NCURSES_VERSION
		(void)check2;
#endif
		(void)x;
		(void)y1;
		(void)x1;
		],
		[cf_cv_need_xopen_extension=$cf_try_xopen_extension; break])
	done
	])
])

case "$cf_cv_need_xopen_extension" in
*_*)
	CF_APPEND_TEXT(CPPFLAGS,-D$cf_cv_need_xopen_extension)
	;;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 57 updated: 2021/01/01 16:53:59
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_XOPEN_SOURCE],[
AC_REQUIRE([AC_CANONICAL_HOST])
AC_REQUIRE([CF_POSIX_VISIBLE])

if test "$cf_cv_posix_visible" = no; then

cf_XOPEN_SOURCE=ifelse([$1],,500,[$1])
cf_POSIX_C_SOURCE=ifelse([$2],,199506L,[$2])
cf_xopen_source=

case "$host_os" in
aix[[4-7]]*)
	cf_xopen_source="-D_ALL_SOURCE"
	;;
msys)
	cf_XOPEN_SOURCE=600
	;;
darwin[[0-8]].*)
	cf_xopen_source="-D_APPLE_C_SOURCE"
	;;
darwin*)
	cf_xopen_source="-D_DARWIN_C_SOURCE"
	cf_XOPEN_SOURCE=
	;;
freebsd*|dragonfly*|midnightbsd*)
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	cf_xopen_source="-D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
hpux11*)
	cf_xopen_source="-D_HPUX_SOURCE -D_XOPEN_SOURCE=500"
	;;
hpux*)
	cf_xopen_source="-D_HPUX_SOURCE"
	;;
irix[[56]].*)
	cf_xopen_source="-D_SGI_SOURCE"
	cf_XOPEN_SOURCE=
	;;
linux*|uclinux*|gnu*|mint*|k*bsd*-gnu|cygwin)
	CF_GNU_SOURCE($cf_XOPEN_SOURCE)
	;;
minix*)
	cf_xopen_source="-D_NETBSD_SOURCE" # POSIX.1-2001 features are ifdef'd with this...
	;;
mirbsd*)
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <sys/select.h> and other headers which use u_int / u_short types
	cf_XOPEN_SOURCE=
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
netbsd*)
	cf_xopen_source="-D_NETBSD_SOURCE" # setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
openbsd[[4-9]]*)
	# setting _XOPEN_SOURCE lower than 500 breaks g++ compile with wchar.h, needed for ncursesw
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=600
	;;
openbsd*)
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
osf[[45]]*)
	cf_xopen_source="-D_OSF_SOURCE"
	;;
nto-qnx*)
	cf_xopen_source="-D_QNX_SOURCE"
	;;
sco*)
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
solaris2.*)
	cf_xopen_source="-D__EXTENSIONS__"
	cf_cv_xopen_source=broken
	;;
sysv4.2uw2.*) # Novell/SCO UnixWare 2.x (tested on 2.1.2)
	cf_XOPEN_SOURCE=
	cf_POSIX_C_SOURCE=
	;;
*)
	CF_TRY_XOPEN_SOURCE
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
esac

if test -n "$cf_xopen_source" ; then
	CF_ADD_CFLAGS($cf_xopen_source,true)
fi

dnl In anything but the default case, we may have system-specific setting
dnl which is still not guaranteed to provide all of the entrypoints that
dnl _XOPEN_SOURCE would yield.
if test -n "$cf_XOPEN_SOURCE" && test -z "$cf_cv_xopen_source" ; then
	AC_MSG_CHECKING(if _XOPEN_SOURCE really is set)
	AC_TRY_COMPILE([#include <stdlib.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_XOPEN_SOURCE_set=yes],
	[cf_XOPEN_SOURCE_set=no])
	AC_MSG_RESULT($cf_XOPEN_SOURCE_set)
	if test "$cf_XOPEN_SOURCE_set" = yes
	then
		AC_TRY_COMPILE([#include <stdlib.h>],[
#if (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
make an error
#endif],
		[cf_XOPEN_SOURCE_set_ok=yes],
		[cf_XOPEN_SOURCE_set_ok=no])
		if test "$cf_XOPEN_SOURCE_set_ok" = no
		then
			AC_MSG_WARN(_XOPEN_SOURCE is lower than requested)
		fi
	else
		CF_TRY_XOPEN_SOURCE
	fi
fi
fi # cf_cv_posix_visible
])
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA version: 24 updated: 2020/03/10 18:53:47
dnl -----------
dnl Check for Xaw (Athena) libraries
dnl
dnl Sets $cf_x_athena according to the flavor of Xaw which is used.
AC_DEFUN([CF_X_ATHENA],
[
cf_x_athena=${cf_x_athena:-Xaw}

AC_MSG_CHECKING(if you want to link with Xaw 3d library)
withval=
AC_ARG_WITH(Xaw3d,
	[  --with-Xaw3d            link with Xaw 3d library])
if test "$withval" = yes ; then
	cf_x_athena=Xaw3d
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with Xaw 3d xft library)
withval=
AC_ARG_WITH(Xaw3dxft,
	[  --with-Xaw3dxft         link with Xaw 3d xft library])
if test "$withval" = yes ; then
	cf_x_athena=Xaw3dxft
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with neXT Athena library)
withval=
AC_ARG_WITH(neXtaw,
	[  --with-neXtaw           link with neXT Athena library])
if test "$withval" = yes ; then
	cf_x_athena=neXtaw
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if you want to link with Athena-Plus library)
withval=
AC_ARG_WITH(XawPlus,
	[  --with-XawPlus          link with Athena-Plus library])
if test "$withval" = yes ; then
	cf_x_athena=XawPlus
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

cf_x_athena_lib=""

if test "$PKG_CONFIG" != none ; then
	cf_athena_list=
	test "$cf_x_athena" = Xaw && cf_athena_list="xaw8 xaw7 xaw6"
	for cf_athena_pkg in \
		$cf_athena_list \
		${cf_x_athena} \
		${cf_x_athena}-devel \
		lib${cf_x_athena} \
		lib${cf_x_athena}-devel
	do
		CF_TRY_PKG_CONFIG($cf_athena_pkg,[
			cf_x_athena_lib="$cf_pkgconfig_libs"
			CF_UPPER(cf_x_athena_LIBS,HAVE_LIB_$cf_x_athena)
			AC_DEFINE_UNQUOTED($cf_x_athena_LIBS)

			CF_TRIM_X_LIBS

AC_CACHE_CHECK(for usable $cf_x_athena/Xmu package,cf_cv_xaw_compat,[
AC_TRY_LINK([
#include <X11/Xmu/CharSet.h>
],[
int check = XmuCompareISOLatin1("big", "small");
(void)check;
],[cf_cv_xaw_compat=yes],[cf_cv_xaw_compat=no])])

			if test "$cf_cv_xaw_compat" = no
			then
				# workaround for broken ".pc" files...
				case "$cf_x_athena_lib" in
				*-lXmu*)
					;;
				*)
					CF_VERBOSE(work around broken package)
					cf_save_xmu="$LIBS"
					cf_first_lib=`echo "$cf_save_xmu" | sed -e 's/^[ ][ ]*//' -e 's/ .*//'`
					CF_TRY_PKG_CONFIG(xmu,[
							LIBS="$cf_save_xmu"
							CF_ADD_LIB_AFTER($cf_first_lib,$cf_pkgconfig_libs)
						],[
							CF_ADD_LIB_AFTER($cf_first_lib,-lXmu)
						])
					CF_TRIM_X_LIBS
					;;
				esac
			fi

			break])
	done
fi

if test -z "$cf_x_athena_lib" ; then
	CF_X_EXT
	CF_X_TOOLKIT
	CF_X_ATHENA_CPPFLAGS($cf_x_athena)
	CF_X_ATHENA_LIBS($cf_x_athena)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA_CPPFLAGS version: 9 updated: 2020/12/31 10:54:15
dnl --------------------
dnl Normally invoked by CF_X_ATHENA, with $1 set to the appropriate flavor of
dnl the Athena widgets, e.g., Xaw, Xaw3d, neXtaw.
AC_DEFUN([CF_X_ATHENA_CPPFLAGS],
[
AC_REQUIRE([AC_PATH_XTRA])
cf_x_athena_root=ifelse([$1],,Xaw,[$1])
cf_x_athena_inc=""

for cf_path in default \
	/usr/contrib/X11R6 \
	/usr/contrib/X11R5 \
	/usr/lib/X11R5 \
	/usr/local
do
	if test -z "$cf_x_athena_inc" ; then
		CF_SAVE_XTRA_FLAGS([CF_X_ATHENA_CPPFLAGS])
		cf_test=X11/$cf_x_athena_root/SimpleMenu.h
		if test "$cf_path" != default ; then
			CF_APPEND_TEXT(CPPFLAGS,-I$cf_path/include)
			AC_MSG_CHECKING(for $cf_test in $cf_path)
		else
			AC_MSG_CHECKING(for $cf_test)
		fi
		AC_TRY_COMPILE([
#include <X11/Intrinsic.h>
#include <$cf_test>],[],
			[cf_result=yes],
			[cf_result=no])
		AC_MSG_RESULT($cf_result)
		CF_RESTORE_XTRA_FLAGS([CF_X_ATHENA_CPPFLAGS])
		if test "$cf_result" = yes ; then
			test "$cf_path"  = default && cf_x_athena_inc=default
			test "$cf_path" != default && cf_x_athena_inc="$cf_path/include"
			break
		fi
	fi
done

if test -z "$cf_x_athena_inc" ; then
	AC_MSG_WARN([Unable to find Athena header files])
elif test "$cf_x_athena_inc" != default ; then
	CF_APPEND_TEXT(CPPFLAGS,-I$cf_x_athena_inc)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_X_ATHENA_LIBS version: 13 updated: 2020/01/11 18:16:10
dnl ----------------
dnl Normally invoked by CF_X_ATHENA, with $1 set to the appropriate flavor of
dnl the Athena widgets, e.g., Xaw, Xaw3d, neXtaw.
AC_DEFUN([CF_X_ATHENA_LIBS],
[AC_REQUIRE([CF_X_TOOLKIT])
cf_x_athena_root=ifelse([$1],,Xaw,[$1])
cf_x_athena_lib=""

for cf_path in default \
	/usr/contrib/X11R6 \
	/usr/contrib/X11R5 \
	/usr/lib/X11R5 \
	/usr/local
do
	for cf_lib in \
		${cf_x_athena_root} \
		${cf_x_athena_root}7 \
		${cf_x_athena_root}6
	do
	for cf_libs in \
		"-l$cf_lib -lXmu" \
		"-l$cf_lib -lXpm -lXmu" \
		"-l${cf_lib}_s -lXmu_s"
	do
		test -n "$cf_x_athena_lib" && break

		CF_SAVE_XTRA_FLAGS([CF_X_ATHENA_LIBS])
		cf_test=XawSimpleMenuAddGlobalActions
		test "$cf_path" != default && cf_libs="-L$cf_path/lib $cf_libs"
		CF_ADD_LIBS($cf_libs)
		AC_MSG_CHECKING(for $cf_test in $cf_libs)
		AC_TRY_LINK([
#include <X11/Intrinsic.h>
#include <X11/$cf_x_athena_root/SimpleMenu.h>
],[
$cf_test((XtAppContext) 0)],
			[cf_result=yes],
			[cf_result=no])
		AC_MSG_RESULT($cf_result)
		CF_RESTORE_XTRA_FLAGS([CF_X_ATHENA_LIBS])

		if test "$cf_result" = yes ; then
			cf_x_athena_lib="$cf_libs"
			break
		fi
	done # cf_libs
		test -n "$cf_x_athena_lib" && break
	done # cf_lib
done

if test -z "$cf_x_athena_lib" ; then
	AC_MSG_ERROR(
[Unable to successfully link Athena library (-l$cf_x_athena_root) with test program])
fi

CF_ADD_LIBS($cf_x_athena_lib)
CF_UPPER(cf_x_athena_LIBS,HAVE_LIB_$cf_x_athena)
AC_DEFINE_UNQUOTED($cf_x_athena_LIBS)
])
dnl ---------------------------------------------------------------------------
dnl CF_X_EXT version: 3 updated: 2010/06/02 05:03:05
dnl --------
AC_DEFUN([CF_X_EXT],[
CF_TRY_PKG_CONFIG(Xext,,[
	AC_CHECK_LIB(Xext,XextCreateExtension,
		[CF_ADD_LIB(Xext)])])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_X_TOOLKIT version: 26 updated: 2021/01/02 09:31:20
dnl ------------
dnl Check for X Toolkit libraries
AC_DEFUN([CF_X_TOOLKIT],
[
AC_REQUIRE([AC_PATH_XTRA])
AC_REQUIRE([CF_CHECK_CACHE])

# OSX is schizoid about who owns /usr/X11 (old) versus /opt/X11 (new) (and
# in some cases has installed dummy files in the former, other cases replaced
# it with a link to the new location).  This complicates the configure script.
# Check for that pitfall, and recover using pkg-config
#
# If none of these are set, the configuration is almost certainly broken.
if test -z "${X_CFLAGS}${X_PRE_LIBS}${X_LIBS}${X_EXTRA_LIBS}"
then
	CF_TRY_PKG_CONFIG(x11,,[AC_MSG_WARN(unable to find X11 library)])
	CF_TRY_PKG_CONFIG(ice,,[AC_MSG_WARN(unable to find ICE library)])
	CF_TRY_PKG_CONFIG(sm,,[AC_MSG_WARN(unable to find SM library)])
	CF_TRY_PKG_CONFIG(xt,,[AC_MSG_WARN(unable to find Xt library)])
else
	LIBS="$X_PRE_LIBS $LIBS $X_EXTRA_LIBS"
fi

cf_have_X_LIBS=no

CF_TRY_PKG_CONFIG(xt,[

	case "x$LIBS" in
	*-lX11*)
		;;
	*)
# we have an "xt" package, but it may omit Xt's dependency on X11
AC_CACHE_CHECK(for usable X dependency,cf_cv_xt_x11_compat,[
AC_TRY_LINK([
#include <X11/Xlib.h>
],[
	int rc1 = XDrawLine((Display*) 0, (Drawable) 0, (GC) 0, 0, 0, 0, 0);
	int rc2 = XClearWindow((Display*) 0, (Window) 0);
	int rc3 = XMoveWindow((Display*) 0, (Window) 0, 0, 0);
	int rc4 = XMoveResizeWindow((Display*)0, (Window)0, 0, 0, 0, 0);
],[cf_cv_xt_x11_compat=yes],[cf_cv_xt_x11_compat=no])])
		if test "$cf_cv_xt_x11_compat" = no
		then
			CF_VERBOSE(work around broken X11 dependency)
			# 2010/11/19 - good enough until a working Xt on Xcb is delivered.
			CF_TRY_PKG_CONFIG(x11,,[CF_ADD_LIB_AFTER(-lXt,-lX11)])
		fi
		;;
	esac

AC_CACHE_CHECK(for usable X Toolkit package,cf_cv_xt_ice_compat,[
AC_TRY_LINK([
#include <X11/Shell.h>
],[int num = IceConnectionNumber(0); (void) num
],[cf_cv_xt_ice_compat=yes],[cf_cv_xt_ice_compat=no])])

	if test "$cf_cv_xt_ice_compat" = no
	then
		# workaround for broken ".pc" files used for X Toolkit.
		case "x$X_PRE_LIBS" in
		*-lICE*)
			case "x$LIBS" in
			*-lICE*)
				;;
			*)
				CF_VERBOSE(work around broken ICE dependency)
				CF_TRY_PKG_CONFIG(ice,
					[CF_TRY_PKG_CONFIG(sm)],
					[CF_ADD_LIB_AFTER(-lXt,$X_PRE_LIBS)])
				;;
			esac
			;;
		esac
	fi

	cf_have_X_LIBS=yes
],[

	LDFLAGS="$X_LIBS $LDFLAGS"
	CF_CHECK_CFLAGS($X_CFLAGS)

	AC_CHECK_FUNC(XOpenDisplay,,[
	AC_CHECK_LIB(X11,XOpenDisplay,
		[CF_ADD_LIB(X11)])])

	AC_CHECK_FUNC(XtAppInitialize,,[
	AC_CHECK_LIB(Xt, XtAppInitialize,
		[AC_DEFINE(HAVE_LIBXT,1,[Define to 1 if we can compile with the Xt library])
		 cf_have_X_LIBS=Xt
		 LIBS="-lXt $LIBS"])])
])

if test "$cf_have_X_LIBS" = no ; then
	AC_MSG_WARN(
[Unable to successfully link X Toolkit library (-lXt) with
test program.  You will have to check and add the proper libraries by hand
to makefile.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF__CURSES_HEAD version: 2 updated: 2010/10/23 15:54:49
dnl ---------------
dnl Define a reusable chunk which includes <curses.h> and <term.h> when they
dnl are both available.
define([CF__CURSES_HEAD],[
#ifdef HAVE_XCURSES
#include <xcurses.h>
char * XCursesProgramName = "test";
#else
#include <${cf_cv_ncurses_header:-curses.h}>
#if defined(NCURSES_VERSION) && defined(HAVE_NCURSESW_TERM_H)
#include <ncursesw/term.h>
#elif defined(NCURSES_VERSION) && defined(HAVE_NCURSES_TERM_H)
#include <ncurses/term.h>
#elif defined(HAVE_TERM_H)
#include <term.h>
#endif
#endif
])
dnl ---------------------------------------------------------------------------
dnl CF__ICONV_BODY version: 2 updated: 2007/07/26 17:35:47
dnl --------------
dnl Test-code needed for iconv compile-checks
define([CF__ICONV_BODY],[
	iconv_t cd = iconv_open("","");
	iconv(cd,NULL,NULL,NULL,NULL);
	iconv_close(cd);]
)dnl
dnl ---------------------------------------------------------------------------
dnl CF__ICONV_HEAD version: 1 updated: 2007/07/26 15:57:03
dnl --------------
dnl Header-files needed for iconv compile-checks
define([CF__ICONV_HEAD],[
#include <stdlib.h>
#include <iconv.h>]
)dnl
dnl ---------------------------------------------------------------------------
dnl CF__INTL_BODY version: 3 updated: 2017/07/10 20:13:33
dnl -------------
dnl Test-code needed for libintl compile-checks
dnl $1 = parameter 2 from AM_WITH_NLS
define([CF__INTL_BODY],[
	bindtextdomain ("", "");
	return (int) gettext ("")
			ifelse([$1], need-ngettext, [ + (int) ngettext ("", "", 0)], [])
#ifndef IGNORE_MSGFMT_HACK
			[ + _nl_msg_cat_cntr]
#endif
])
dnl ---------------------------------------------------------------------------
dnl CF__INTL_HEAD version: 1 updated: 2007/07/26 17:35:47
dnl -------------
dnl Header-files needed for libintl compile-checks
define([CF__INTL_HEAD],[
#include <libintl.h>
extern int _nl_msg_cat_cntr;
])dnl
dnl ---------------------------------------------------------------------------
dnl jm_GLIBC21 version: 4 updated: 2015/05/10 19:52:14
dnl ----------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl glibc21.m4
dnl ====================
dnl serial 2
dnl
dnl Test for the GNU C Library, version 2.1 or newer.
dnl From Bruno Haible.
AC_DEFUN([jm_GLIBC21],
[
AC_CACHE_CHECK(whether we are using the GNU C Library 2.1 or newer,
	ac_cv_gnu_library_2_1,
	[AC_EGREP_CPP([Lucky GNU user],
	[
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1) || (__GLIBC__ > 2)
  Lucky GNU user
 #endif
#endif
	],
	ac_cv_gnu_library_2_1=yes,
	ac_cv_gnu_library_2_1=no)])
	AC_SUBST(GLIBC21)
	GLIBC21="$ac_cv_gnu_library_2_1"
])
