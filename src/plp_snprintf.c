/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/

/*
 * Overview:
 *
 * This version of snprintf was developed originally for printing
 * on a motley collection of specialized hardware that had NO IO
 * library.  Due to contractual restrictions,  a clean room implementation
 * of the printf() code had to be developed.
 *
 * The method chosen for printf was to be as paranoid as possible,
 * as these platforms had NO memory protection,  and very small
 * address spaces.  This made it possible to try to print
 * very long strings, i.e. - all of memory, very easily.  To guard
 * against this,  all printing was done via a buffer, generous enough
 * to hold strings,  but small enough to protect against overruns,
 * etc.
 *
 * Strangely enough,  this proved to be of immense importance when
 * SPRINTFing to a buffer on a stack...  The rest,  of course,  is
 * well known,  as buffer overruns in the stack are a common way to
 * do horrible things to operating systems, security, etc etc.
 *
 * This version of snprintf is VERY limited by modern standards.
 *
 * COPYRIGHT AND TERMS OF USE:
 *
 * You may use, copy, distribute, or otherwise incorporate this software
 * and documentation into any product or other item,  provided that
 * the copyright in the documentation and source code as well as the
 * source code generated constant strings in the object, executable
 * or other code remain in place and are present in executable modules
 * or objects.
 *
 * You may modify this code as appropriate to your usage; however the
 * modified version must be identified by changing the various source
 * and object code identification strings as is appropriately noted
 * in the source code.
 *
 * The next include line is expected to work in conjunction with the
 * GNU CONFIGURE utility.  You  should define the following macros
 * appropriately:
 *
 * HAVE_STDARG_H - if the <stdargs.h> include file is available
 * HAVE_VARARG_H - if the <varargs.h> include file is available
 *
 * HAVE_STRERROR - if the strerror() routine is available.
 *   If it is not available, then examine the lines containing
 *   the tests below.  You may need to fiddle with HAVE_SYS_NERR
 *   and  HAVE_SYS_NERR_DEF to make compilation work correctly.
 *        HAVE_SYS_NERR
 *        HAVE_SYS_NERR_DEF
 *
 * HAVE_QUAD_T   - if the quad_t type is defined
 * HAVE_LONG_LONG  - if the long long type is defined
 *
 *   If you are using the GNU configure (autoconf) facility, add the
 *   following line to the configure.in file, to force checking for the
 *   quad_t and long long  data types:
 *
 *     AC_CHECK_TYPE(quad_t,NONE)
 *
 * 	dnl test to see if long long is defined
 *
 * 	AC_MSG_CHECKING(checking for long long)
 * 	AC_TRY_COMPILE([
 * 	#include <sys/types.h>
 * 	],[long long x; x = 0],
 * 	ac_cv_long_long=yes, ac_cv_long_long=no)
 * 	AC_MSG_RESULT($ac_cv_long_long)
 * 	if test $ac_cv_long_long = yes; then
 * 	  AC_DEFINE(HAVE_LONG_LONG)
 * 	fi
 *
 *
 *   Add the following lines to the acconfig.h in the correct
 *   position.
 *
 *     / * Define if quad_t is NOT present on the system  * /
 *     #undef quad_t
 *
 *     / * Define if long long is present on the system  * /
 *     #undef HAVE_LONG_LONG
 *
 *   When you run configure, if quad_t is NOT defined, the config.h
 *   file will have in it:
 *     #define quad_t NONE
 *
 *   If it is defined, the config.h file will have
 *   / * #undef quad_t * /
 *
 *   If long long is defined, you will have:
 *     #define HAVE_LONG_LONG
 *
 *    This code is then used in the source code to enable quad quad
 *   and long long support.
 *
 */


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */

#if !defined(HAVE_SNPRINTF) && !defined(HAVE_VSNPRINTF)

#	if !defined(PLP_SNPRINTF_H) && !defined(TIN_H)
#		include "plp_snprintf.h"
#	endif /* !PLP_SNPRINTF_H && !TIN_H */

/*
 * KEEP THIS STRING - MODIFY AT THE END WITH YOUR REVISIONS
 * i.e. - the LOCAL REVISIONS part is for your use
 */

static const char *_id = "plp_snprintf V1999.02.20 Copyright Patrick Powell 1988-1999 <papowell@astart.com> \
@Id: plp_snprintf.c,v 1.4 1999/02/20 17:44:16 papowell Exp papowell @\
 LOCAL REVISIONS: tin 1.5.2-01";

/* varargs declarations: */

#ifdef HAVE_STDARGS
#	undef HAVE_STDARGS    /* let's hope that works everywhere (mj) */
#endif /* HAVE_STDARGS */
#ifdef VA_LOCAL_DECL
#	undef VA_LOCAL_DECL
#endif /* VA_LOCAL_DECL */
#ifdef VA_START
#	undef VA_START
#endif /* VA_START */
#ifdef VA_SHIFT
#	undef VA_SHIFT
#endif /* VA_SHIFT */
#ifdef VA_END
#	undef VA_END
#endif /* VA_END */

#ifdef HAVE_STDARG_H
#	include <stdarg.h>
#	define HAVE_STDARGS    /* let's hope that works everywhere (mj) */
#	define VA_LOCAL_DECL   va_list ap;
#	define VA_START(f)     va_start(ap, f)
#	define VA_SHIFT(v,t)	;	/* no-op for ANSI */
#	define VA_END          va_end(ap)
#else
#	ifdef HAVE_VARARGS_H
#		include <varargs.h>
#		undef HAVE_STDARGS
#		define VA_LOCAL_DECL   va_list ap;
#		define VA_START(f)     va_start(ap)		/* f is ignored! */
#		define VA_SHIFT(v,t)	v = va_arg(ap,t)
#		define VA_END		va_end(ap)
#	else
XX ** NO VARARGS ** XX
#	endif /* HAVE_VARARGS_H */
#endif /* HAVE_STDARG_H */

#if 0
/* the dreaded QUAD_T strikes again... */
#ifndef HAVE_QUAD_T
#	if defined(quad_t) && qaud_t == NONE
#		define HAVE_QUAD_T 0
#	else
#		define HAVE_QUAD_T 1
#	endif /* quad_t && qaud_t== NONE */
#endif /* HAVE_QUAD_T */
#endif /* 0 */

#if defined(HAVE_QUAD_T) && !defined(HAVE_LONG_LONG)
  ERROR you need long long
#endif
#ifdef HAVE_QUAD_T
   /* suspender and belts on this one */
   const union { quad_t t; long long v; } x;
#endif

union value {
#if defined(HAVE_QUAD_T) || defined(HAVE_LONG_LONG)
	long long value;
#else
	long value;
#endif
	double dvalue;
};

#ifdef cval
#	undef cval
#endif /* cval */
#define cval(s) (*((unsigned const char *)s))


static char *plp_Errormsg(int err);
static void plp_strcat(char *dest, const char *src);
static void dopr(char *buffer, const char *format, va_list args);
static void fmtstr(const char *value, int ljust, int len, int precision);
static void fmtnum(union value *value, int plp_base, int dosign, int ljust, int len, int zpad);
static void fmtdouble(int fmt, double value, int ljust, int len, int zpad, int precision);
static void dostr(const char *);
static void dopr_outch(int c);

static char *output;
static char *end;

int visible_control = 1;


int plp_vsnprintf(char *str, size_t count, const char *fmt, va_list args)
{
	str[0] = 0;
	end = str+count-1;
	dopr( str, fmt, args );
	if( count != 0 )
		end[0] = 0;
	return(strlen(str));
}

/* VARARGS3 */
#ifdef HAVE_STDARGS
int plp_snprintf (char *str,size_t count,const char *fmt,...)
#else
int plp_snprintf (va_alist) va_dcl
#endif
{
#ifndef HAVE_STDARGS
    char *str;
	size_t count;
    char *fmt;
#endif
    VA_LOCAL_DECL

    VA_START (fmt);
    VA_SHIFT (str, char *);
    VA_SHIFT (count, size_t );
    VA_SHIFT (fmt, char *);
    (void) plp_vsnprintf ( str, count, fmt, ap);
    VA_END;
	return( strlen( str ) );
}

static void dopr( char *buffer, const char *format, va_list args )
{
	int ch;
	union value value;
	int longflag = 0;
	int quadflag = 0;
	char *strvalue;
	int ljust;
	int len;
	int zpad;
	int precision;
	int set_precision;
	double dval;
	int err = errno;
	int plp_base = 0;
	int signed_val = 0;

	output = buffer;
	while( (ch = *format++) ){
		switch( ch ){
		case '%':
			longflag = quadflag =
			ljust = len = zpad = plp_base = signed_val = 0;
			precision = -1; set_precision = 0;
		nextch:
			ch = *format++;
			switch( ch ){
			case 0:
				dostr( "**end of format**" );
				return;
			case '-': ljust = 1; goto nextch;
			case '.': set_precision = 1; precision = 0; goto nextch;
			case '*': len = va_arg( args, int ); goto nextch;
			case '0': /* set zero padding if len not set */
				if(len==0 && set_precision == 0 ) zpad = '0';
				/* FALLTHROUGH */
			case '1': /* FALLTHROUGH */
			case '2': /* FALLTHROUGH */
			case '3': /* FALLTHROUGH */
			case '4': /* FALLTHROUGH */
			case '5': /* FALLTHROUGH */
			case '6': /* FALLTHROUGH */
			case '7': /* FALLTHROUGH */
			case '8': /* FALLTHROUGH */
			case '9':
				if( set_precision ){
					precision = precision*10 + ch - '0';
				} else {
					len = len*10 + ch - '0';
				}
				goto nextch;
			case 'l': ++longflag; goto nextch;
			case 'q': quadflag = 1; goto nextch;
			case 'u': case 'U':
				if( plp_base == 0 ){ plp_base = 10; signed_val = 0; }
				/* FALLTHROUGH */
			case 'o': case 'O':
				if( plp_base == 0 ){ plp_base = 8; signed_val = 0; }
				/* FALLTHROUGH */
			case 'd': case 'D':
				if( plp_base == 0 ){ plp_base = 10; signed_val = 1; }
				/* FALLTHROUGH */
			case 'x':
				if( plp_base == 0 ){ plp_base = 16; signed_val = 0; }
				/* FALLTHROUGH */
			case 'X':
				if( plp_base == 0 ){ plp_base = -16; signed_val = 0; }
				if( quadflag || longflag > 1 ){
#if defined(HAVE_LONG_LONG)
					if( signed_val ){
					value.value = va_arg( args, long long );
					} else {
					value.value = va_arg( args, unsigned long long );
					}
#else
					if( signed_val ){
					value.value = va_arg( args, long );
					} else {
					value.value = va_arg( args, unsigned long );
					}
#endif
				} else if( longflag ){
					if( signed_val ){
						value.value = va_arg( args, long );
					} else {
						value.value = va_arg( args, unsigned long );
					}
				} else {
					if( signed_val ){
						value.value = va_arg( args, int );
					} else {
						value.value = va_arg( args, unsigned int );
					}
				}
				fmtnum( &value,plp_base,signed_val, ljust, len, zpad );
				break;
			case 's':
				strvalue = va_arg( args, char *);
				fmtstr(strvalue, ljust, len, precision);
				break;
			case 'c':
				ch = va_arg( args, int );
				{ char b[2];
					int vsb = visible_control;
					b[0] = ch;
					b[1] = 0;
					visible_control = 0;
					fmtstr(b, ljust, len, precision);
					visible_control = vsb;
				}
				break;
			case 'f': case 'g': case 'e':
				dval = va_arg( args, double );
				fmtdouble(ch, dval, ljust, len, zpad, precision);
				break;
			case 'm':
				fmtstr(plp_Errormsg(err), ljust, len, precision);
				break;
			case '%': dopr_outch( ch ); continue;
			default:
				dostr(  "???????" );
			}
			longflag = 0;
			break;
		default:
			dopr_outch( ch );
			break;
		}
	}
	*output = 0;
}

/*
 * Format '%[-]len[.precision]s'
 * -   = left justify (ljust)
 * len = minimum length
 * precision = numbers of chars in string to use
 */
static void
fmtstr(const char *value, int ljust, int len, int precision)
{
	int padlen, slen, i, c;	/* amount to pad */

	if( value == 0 ){
		value = "<NULL>";
	}
	if( precision > 0 ){
		slen = precision;
	} else {
		/* cheap slen so you do not have library call */
		for( slen = i = 0; (c=cval(value+i)); ++i ){
			if( visible_control && iscntrl( c ) && !isspace(c) ){
				++slen;
			}
			++slen;
		}
	}
	padlen = len - slen;
	if( padlen < 0 ) padlen = 0;
	if( ljust ) padlen = -padlen;
	while( padlen > 0 ) {
		dopr_outch( ' ' );
		--padlen;
	}
	/* output characters */
	for( i = 0; (c = cval(value+i)); ++i ){
		if( visible_control && iscntrl( c ) && !isspace( c ) ){
			dopr_outch('^');
			c = ('@' | (c & 0x1F));
		}
		dopr_outch(c);
	}
	while( padlen < 0 ) {
		dopr_outch( ' ' );
		++padlen;
	}
}

static void
fmtnum(  union value *value, int plp_base, int dosign, int ljust,
	int len, int zpad )
{
	int signvalue = 0;
#ifdef HAVE_LONG_LONG
	unsigned long long uvalue;
#else
	unsigned long uvalue;
#endif /* HAVE_LONG_LONG */
	char convert[64];
	int place = 0;
	int padlen = 0;	/* amount to pad */
	int caps = 0;

	/* fprintf(stderr,"value 0x%x, plp_base %d, dosign %d, ljust %d, len %d, zpad %d\n",
		value, plp_base, dosign, ljust, len, zpad );/ **/
	uvalue = value->value;
	if( dosign ){
		if( value->value < 0 ) {
			signvalue = '-';
			uvalue = -value->value;
		}
	}
	if( plp_base < 0 ){
		caps = 1;
		plp_base = -plp_base;
	}
	do{
		convert[place++] =
			(caps? "0123456789ABCDEF":"0123456789abcdef")
			 [uvalue % (unsigned)plp_base  ];
		uvalue = (uvalue / (unsigned)plp_base );
	}while(uvalue);
	convert[place] = 0;
	padlen = len - place;
	if( padlen < 0 ) padlen = 0;
	if( ljust ) padlen = -padlen;
	/* fprintf( stderr, "str '%s', place %d, sign %c, padlen %d\n",
		convert,place,signvalue,padlen); / **/
	if( zpad && padlen > 0 ){
		if( signvalue ){
			dopr_outch( signvalue );
			--padlen;
			signvalue = 0;
		}
		while( padlen > 0 ){
			dopr_outch( zpad );
			--padlen;
		}
	}
	while( padlen > 0 ) {
		dopr_outch( ' ' );
		--padlen;
	}
	if( signvalue ) dopr_outch( signvalue );
	while( place > 0 ) dopr_outch( convert[--place] );
	while( padlen < 0 ){
		dopr_outch( ' ' );
		++padlen;
	}
}

static void
plp_strcat(char *dest, const char *src )
{
	if( dest && src ){
		dest += strlen(dest);
		strcpy(dest,src);
	}
}

static void
fmtdouble( int fmt, double value, int ljust, int len, int zpad, int precision )
{
	char convert[128];
	char fmts[128];
	int l;

	if( len == 0 )
		len = 10;
	if( len > (int) (sizeof(convert) - 20) )
		len = sizeof(convert) - 20;
	if( precision > (int) sizeof(convert) - 20 )
		precision = sizeof(convert) - 20;
	if( precision > len )
		precision = len;
	strcpy( fmts, "%" );
	if( ljust )
		plp_strcat(fmts, "-" );
	if( zpad )
		plp_strcat(fmts, "0" );
	if( len )
		sprintf( fmts+strlen(fmts), "%d", len );
	if( precision > 0 )
		sprintf( fmts+strlen(fmts), ".%d", precision );
	l = strlen( fmts );
	fmts[l] = fmt;
	fmts[l+1] = 0;
	/* this is easier than trying to do the portable dtostr */
	sprintf( convert, fmts, value );
	dostr( convert );
}

static void dostr( const char *str )
{
	while(*str) dopr_outch(*str++);
}

static void dopr_outch( int c )
{
	if( end == 0 || output < end ){
		*output++ = c;
	}
}


/****************************************************************************
 * static char *plp_errormsg( int err )
 *  returns a printable form of the
 *  errormessage corresponding to the valie of err.
 *  This is the poor man's version of sperror(), not available on all systems
 *  Patrick Powell Tue Apr 11 08:05:05 PDT 1995
 ****************************************************************************/
/****************************************************************************/
#if !defined(HAVE_STRERROR)

# if defined(HAVE_SYS_NERR)
#  if !defined(HAVE_SYS_NERR_DEF)
     extern int sys_nerr;
#  endif
#  define num_errors    (sys_nerr)
# else
#  define num_errors    (-1)            /* always use "errno=%d" */
# endif

# if defined(HAVE_SYS_ERRLIST)
#  if !defined(HAVE_SYS_ERRLIST_DEF)
     extern const char *const sys_errlist[];
#  endif
# else
#  undef  num_errors
#  define num_errors   (-1)            /* always use "errno=%d" */
# endif

#endif

static char * plp_Errormsg ( int err )
{
    char *cp;

#if defined(HAVE_STRERROR)
	cp = (void *)strerror(err);
#else
# if defined(HAVE_SYS_ERRLIST)
    if (err >= 0 && err < num_errors) {
		cp = (void *)sys_errlist[err];
    } else
# endif
	{
		static char msgbuf[32];     /* holds "errno=%d". */
		/* SAFE use of sprintf */
		(void) sprintf (msgbuf, "errno=%d", err);
		cp = msgbuf;
    }
#endif
    return (cp);
}

#if defined(TEST)
#include <stdio.h>
int main( void )
{
	char buffer[128];
	char *t;
	char *test1 = "01234";
	errno = 1;
	plp_snprintf( buffer, sizeof(buffer), (t="errno '%m'")); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%s"), test1 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%12s"), test1 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%-12s"), test1 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%12.2s"), test1 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%-12.2s"), test1 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%g"), 1.25 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%g"), 1.2345 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%12g"), 1.25 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%12.2g"), 1.25 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%0*d"), 6, 1 ); printf( "%s = '%s'\n", t, buffer );
#if defined(HAVE_LONG_LONG)
	plp_snprintf( buffer, sizeof(buffer), (t = "%llx"), 1, 2, 3, 4 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%llx"), (long long)1, (long long)2 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%qx"), 1, 2, 3, 4 ); printf( "%s = '%s'\n", t, buffer );
	plp_snprintf( buffer, sizeof(buffer), (t = "%qx"), (quad_t)1, (quad_t)2 ); printf( "%s = '%s'\n", t, buffer );
#endif
	return(0);
}
#endif
#endif /* !HAVE_SNPRINTF && !HAVE_VSNPRINTF */
