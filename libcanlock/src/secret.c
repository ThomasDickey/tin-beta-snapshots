/* ========================================================================== */
/* Copyright (c) 2018 Michael Baeuerle
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY
 * SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */

/* C11 (if available) */
#define __STDC_WANT_LIB_EXT1__ 1  /*!< Request for Annex K */

/* GNU autoconf */
#include <config.h>

/* C99 */
#include <string.h>

/* Local */
#include "canlock.h"


/* ========================================================================== */
/*! \brief Overwrite secret data in memory
 *
 * \param[in] sec       Pointer to secret data
 * \param[in] sec_size  Size of secret data
 * \param[in] buf_size  Size of buffer
 *
 * \attention
 * The default implementation uses \c memset() and is not crytographically
 * secure. A smart compiler may optimize this function away completely.
 * If no better function of the OS was available, this situation is indicated
 * with a return value of 1.
 *
 * \note
 * Even if the OS in general supports explicit overwriting of memory, on a
 * modern machine with Cache, Swap, etc. it is not possible to securely
 * overwrite all copies that such subsystems may have created (at least it
 * is not possible in a portable way).
 *
 * \return
 * - 0 on success
 * - 1 if only \c memset() was available
 * - -1 on error
 */

int cl_clear_secret(void *sec, size_t sec_size, size_t buf_size)
{
   int  res = -1;

   if (NULL != sec)
   {
/*
 * C11 Annex K specifies "__STDC_LIB_EXT1__" for detection:
 * #if defined(__STDC_LIB_EXT1__)
 * We use the check result from autoconf instead
 */
#ifdef HAVE_MEMSET_S
      /* Standard solution using C11 Annex K */
      res = (int) memset_s(sec, buf_size, 0, sec_size);
      if (res)
      {
         /* Nonzero return value indicates a constraint violation */
         res = -1;
      }
#else  /* HAVE_MEMSET_S */
      if (sec_size <= buf_size)
      {
         /* OS specific functions should be called here, if available */
#  ifdef HAVE_EXPLICIT_MEMSET
         /* NetBSD has 'explicit_memset()' since version 7.0 */
         explicit_memset(sec, 0, sec_size);
         res = 0;
#  elif defined(HAVE_EXPLICIT_BZERO)
         /* OpenBSD has 'explicit_bzero()' since version 5.5 */
         /* GNU libc has 'explicit_bzero()' since version 2.25 */
         explicit_bzero(sec, sec_size);
         res = 0;
#  else  /* HAVE_EXPLICIT_MEMSET */
         /*
          * There seems to be no portable way to enforce memory access in C99.
          * But there is a chance that the optimizer is not smart enough and
          * 'memset()' will do the job.
          */
         memset(sec, 0, sec_size);
         res = 1;
#  endif
      }
#endif  /* HAVE_MEMSET_S */
   }

   return (res);
}


/* EOF */
