/*
 * COPYRIGHT AND PERMISSION NOTICE
 *
 * Copyright (c) 2003 G.J. Andruk
 * Copyright (c) 2017 Dennis Preiser
 * Copyright (c) 2018 Michael Baeuerle
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

#ifndef _CANLOCK_H
#define _CANLOCK_H


/* ========================================================================== */
/* Include header files */

#include <stddef.h>  /* For size_t */


/* ========================================================================== */
/* Constants */

/* libcanlock API major version */
#define CL_API_MAJOR  3

/* libcanlock API minor version */
#define CL_API_MINOR  0

/* libcanlock legacy API V2 emulation available if nonzero */
#define CL_API_V2  1

/* Supported hash algorithms for <scheme> according to RFC 8315 */
#define CL_INVALID  0
#define CL_SHA1     1         /* IANA registered name: "sha1" */
#define CL_SHA224   2         /* IANA registered name: "sha224" */
#define CL_SHA256   3         /* IANA registered name: "sha256" */
#define CL_SHA384   4         /* IANA registered name: "sha384" */
#define CL_SHA512   5         /* IANA registered name: "sha512" */


/* ========================================================================== */
/* Type definitions  */

typedef int cl_hash_version;


/* ========================================================================== */
/* New API (available since version 3.0.0) */

extern char *cl_get_key(cl_hash_version which_hash,
                        const unsigned char *secret, size_t seclen,
                        const unsigned char *message, size_t msglen);
extern char *cl_get_lock(cl_hash_version which_hash,
                         const unsigned char *secret, size_t seclen,
                         const unsigned char *message, size_t msglen);
extern int cl_split(char *input, char **klstring);
extern int cl_verify(cl_hash_version which_hash,
                     const char *key, const char *lock);
extern int cl_clear_secret(void *sec, size_t sec_size, size_t buf_size);


#if CL_API_V2
/* ========================================================================== */
/* Old API (Version 2b emulation) provided for backward compatibility */

extern char *lock_strip_alpha(char *key, char *type);
extern char *lock_strip(char *key, char *type);

/* Wrappers around new cl_(get_key|get_lock|verify) with hash set to SHA1 */
extern char *sha_key(const unsigned char *secret, size_t seclen,
                     const unsigned char *message, size_t msglen);
extern char *sha_lock(const unsigned char *secret, size_t seclen,
                      const unsigned char *message, size_t msglen);
extern int sha_verify(const char *key, const char *lock);
#endif  /* CL_API_V2 */


#endif  /* _CANLOCK_H */

/* EOF */
