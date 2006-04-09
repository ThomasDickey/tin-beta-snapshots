/*
 * includes/defines/declarations which are already done by tin.h
 * but might be needed if you use plp_snprintf() in other programms
 */

#ifndef PLP_SNPRINTF_H
#	define PLP_SNPRINTF_H 1

#	ifdef HAVE_CONFIG_H
#		include "config.h"
#	endif /* HAVE_CONFIG_H */
#	include <sys/types.h>
#	include <ctype.h>
#	include <stdlib.h>
#	ifdef HAVE_STRING_H
#		include <string.h>
#	endif /* HAVE_STRING_H */
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif /* HAVE_STRINGS_H */
#	include <stdio.h>

/* For testing, define these values */
#	ifdef TEST
#		define HAVE_QUAD_T 1
#		define HAVE_STDARG_H 1
#	endif /* TEST */

	extern int errno;
#endif /* PLP_SNPRINTF_H */
