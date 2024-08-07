#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */

#ifndef _PORTABLE_SNPRINTF_H_
#	define _PORTABLE_SNPRINTF_H_

#	define PORTABLE_SNPRINTF_VERSION_MAJOR 2
#	define PORTABLE_SNPRINTF_VERSION_MINOR 2

#	ifdef HAVE_SNPRINTF
    /* #include <stdio.h> */
#		ifdef SNPRINTF_BROKEN
#			define PREFER_PORTABLE_SNPRINTF
#		endif /* SNPRINTF_BROKEN */
#	else
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
extern int vsnprintf(char *, size_t, const char *, va_list);
#	endif /* HAVE_SNPRINTF */

#	if defined(HAVE_SNPRINTF) && defined(PREFER_PORTABLE_SNPRINTF)
extern int portable_snprintf(char *str, size_t str_m, const char *fmt, /*args*/ ...);
extern int portable_vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
#	define snprintf  portable_snprintf
#	define vsnprintf portable_vsnprintf
#	endif /* HAVE_SNPRINTF && PREFER_PORTABLE_SNPRINTF */

#	if !defined(HAVE_SNPRINTF) || defined(PREFER_PORTABLE_SNPRINTF)
#		ifdef NEED_ASPRINTF
extern int asprintf (char **ptr, const char *fmt, /*args*/ ...);
#		endif /* NEED_ASPRINTF */
#		ifdef NEED_VASPRINTF
extern int vasprintf (char **ptr, const char *fmt, va_list ap);
#		endif /* NEED_VASPRINTF */
#		ifdef NEED_ASNPRINTF
extern int asnprintf (char **ptr, size_t str_m, const char *fmt, /*args*/ ...);
#		endif /* NEED_ASNPRINTF */
#		ifdef NEED_VASNPRINTF
extern int vasnprintf(char **ptr, size_t str_m, const char *fmt, va_list ap);
#		endif /* NEED_VASNPRINTF */
#	endif /* !HAVE_SNPRINTF || PREFER_PORTABLE_SNPRINTF */

#endif /* _PORTABLE_SNPRINTF_H_ */
