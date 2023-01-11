/*
 * COPYRIGHT AND PERMISSION NOTICE
 *
 * Copyright (c) 2017 Dennis Preiser
 * Copyright (c) 2003 G.J. Andruk
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

/* C99 */
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

/* Local */
#include "sha.h"
#include "canlock.h"


/* Map external hash type 'cl_hash_version' to local SHA implementation */
static enum SHAversion which_cl_hash(int which_hash)
{
   switch (which_hash)
   {
      case CL_SHA1:
         return SHA1;
      case CL_SHA224:
         return SHA224;
      case CL_SHA384:
         return SHA384;
      case CL_SHA512:
         return SHA512;

      /* Mandatory algorithm */
      case CL_SHA256:
      default:
         return SHA256;
   }
}


/*
 * Portable replacement for 'strdup()'
 *
 * 'strdup()' requires SUSv2, XSI extension or POSIX.1-2008
 * https://pubs.opengroup.org/onlinepubs/007908799/xsh/strdup.html
 * https://pubs.opengroup.org/onlinepubs/009695399/functions/strdup.html
 * https://pubs.opengroup.org/onlinepubs/9699919799/functions/strdup.html
 *
 * Note:
 * 'malloc()' already gives the correct return (and 'errno') values required by
 * POSIX.
 */
static char *my_strdup(const char *s)
{
   char *res;
   size_t len;

   len = strlen(s);
   res = (char *) malloc(len + (size_t) 1);
   if (NULL != res)
   {
      strcpy(res, s);
   }

   return res;
}


/*
 * Extract a <c-lock-string> or <c-key-string> element respectively from 'key'
 * (that is, with the <scheme>: prefix removed)
 *
 * The <scheme> is written to 'type' on success, else empty string on failure.
 * The caller must ensure that the provided buffer is large enough.
 *
 * Returns a malloc()'d buffer on success, that the caller will need to free().
 * Returns NULL (on failure).
 */
#if ! CL_API_V2
static
#endif  /* CL_API_V2 */
char *lock_strip_alpha(char *key, char *type)
{
   char *ret;
   int offset;

   /* Strip scheme and write it to 'type' */
   do
   {
      *type = (char) tolower((int) (unsigned char) *key);
      type++;
      key++;
   } while (*key && *key != ':');
   *type = '\0';

   key++;  /* Skip colon */

   /* Copy <c-lock-string> or <c-key-string> respectively */
   ret = my_strdup(key);
   if (NULL != ret)
   {
      /*
       * Strip potential trailing <clue-string> element
       * (that is, a :xx suffix is removed).
       *
       * Note:
       * This element was removed from an early draft, but could still be
       * present.
       */
      offset = 0;
      while (ret[offset] && ret[offset] != ':')
         offset++;
      ret[offset] = '\0';
   }
   return ret;
}


#if CL_API_V2
char *lock_strip(char *key, char *type)
{
   return lock_strip_alpha(key, type);
}
#endif  /* CL_API_V2 */


/*
 * Split a <c-lock> or <c-key> element respectively
 *
 * 'input' is split on the first colon into <scheme> and <c-lock-string> or
 * <c-key-string> elements respectively.
 *
 * If scheme is supported, the corresponding ID is returned.
 *
 * A pointer to <c-lock-string> or <c-key-string> respectively is written to
 * the location pointed to by 'klstring' (no memory is allocated, the address
 * points to the memory used for 'input').
 *
 * Returns the hash algorithm ID for scheme (on success).
 * Returns CL_INVALID (on failure, NULL was written to 'klstring' in this case).
 */
int cl_split(char *input, char **klstring)
{
   int hash = CL_INVALID;
   char *scheme, *junk;

   *klstring = strchr(input, (int) ':');
   if (NULL != *klstring)
   {
      (*klstring)++;  /* Skip colon */
      scheme = (char *) malloc(strlen(input) + (size_t) 1);
      if (NULL == scheme) { *klstring = NULL; }
      else
      {
         junk = lock_strip_alpha(input, scheme);
         if (NULL != junk)
         {
            if (!strcmp(scheme, "sha1"))  { hash = CL_SHA1; }
            else if (!strcmp(scheme, "sha224"))  { hash = CL_SHA224; }
            else if (!strcmp(scheme, "sha256"))  { hash = CL_SHA256; }
            else if (!strcmp(scheme, "sha384"))  { hash = CL_SHA384; }
            else if (!strcmp(scheme, "sha512"))  { hash = CL_SHA512; }
            free((void *) junk);
         }
         free((void *) scheme);
      }
   }
   return hash;
}


#if CL_API_V2
/*
 * Generate a SHA1 cancel key
 * Returns a malloc()'d buffer that the caller will need to free() (on success).
 * Returns NULL (on failure).
 */
char *sha_key(const unsigned char *secret, size_t seclen,
        const unsigned char *message, size_t msglen)
{
   return cl_get_key(CL_SHA1, secret, seclen, message, msglen);
}
#endif  /* CL_API_V2 */


/*
 * Generate a cancel key
 * Returns a malloc()'d buffer that the caller will need to free() (on success).
 * Returns NULL (on failure).
 */
char *cl_get_key(int which_hash, const unsigned char *secret, size_t seclen,
                 const unsigned char *message, size_t msglen)
{
   char *cankey[1], *tmp;
   const char *scheme;
   enum SHAversion which_sha;
   size_t keysize, scheme_len;
   uint8_t hmacbuff[USHAMaxHashSize];

   which_sha = which_cl_hash(which_hash);

   /* Ensure that size data from external caller can be represented as 'int' */
   if ((size_t) INT_MAX < msglen || (size_t) INT_MAX < seclen)
      return NULL;

   if (RFC2104Hmac(which_sha, message, (int) msglen, secret, (int) seclen,
                   hmacbuff)
       != shaSuccess)
      return NULL;

   if (!(keysize = base64_encode(hmacbuff, USHAHashSize(which_sha), cankey)))
      return NULL;

   switch (which_sha)
   {
      case SHA1:
         scheme = "sha1:";
         break;
      case SHA224:
         scheme = "sha224:";
         break;
      case SHA256:
         scheme = "sha256:";
         break;
      case SHA384:
         scheme = "sha384:";
         break;
      case SHA512:
         scheme = "sha512:";
         break;
      default:
         return NULL;
   }

   scheme_len = strlen(scheme);

   tmp = (char *) realloc((void *) *cankey, keysize + scheme_len + 1);
   if (NULL != tmp) { *cankey = tmp; }
   else
   {
      free((void *) *cankey);
      return NULL;
   }

   memmove((void *) (*cankey + scheme_len), (void *) *cankey, keysize + 1);
   strncpy(*cankey, scheme, scheme_len);
   return (*cankey);
}


#if CL_API_V2
/*
 * Generate a SHA1 cancel lock
 * Returns a malloc()'d buffer that the caller will need to free() (on success).
 * Returns NULL (on failure).
 */
char *sha_lock(const unsigned char *secret, size_t seclen,
               const unsigned char *message, size_t msglen)
{
   return cl_get_lock(CL_SHA1, secret, seclen, message, msglen);
}
#endif  /* CL_API_V2 */


/*
 * Generate cancel lock
 * Returns a malloc()'d buffer that the caller will need to free() (on success).
 * Returns NULL (on failure).
 */
char *cl_get_lock(int which_hash, const unsigned char *secret, size_t seclen,
                  const unsigned char *message, size_t msglen)
{
   USHAContext hash_ctx;
   char *canlock[1], *tmp, *junk;
   const char *scheme;
   enum SHAversion which_sha;
   size_t hash_size, locksize, scheme_len;
   uint8_t *cankey, hmacbuff[USHAMaxHashSize];

   which_sha = which_cl_hash(which_hash);

   /* The function 'USHAHashSize()' never returns negative values */
   hash_size = (size_t) USHAHashSize(which_sha);

   if (!(tmp = cl_get_key(which_hash, secret, seclen, message, msglen)))
      return NULL;

   if (!(junk = malloc(hash_size + 1)))
   {
      free(tmp);
      return NULL;
   }

   cankey = (unsigned char *) lock_strip_alpha(tmp, junk);

   free(tmp);
   free(junk);

   if (USHAReset(&hash_ctx, which_sha) != shaSuccess)
   {
      free(cankey);
      return NULL;
   }

   if (USHAInput(&hash_ctx, cankey, (unsigned int) strlen((char *) cankey))
       != shaSuccess)
   {
      free(cankey);
      return NULL;
   }

   free(cankey);

   if (USHAResult(&hash_ctx, hmacbuff) != shaSuccess)
      return NULL;

   if (!(locksize = base64_encode(hmacbuff, (int) hash_size, canlock)))
      return NULL;

   switch (which_sha)
   {
      case SHA1:
         scheme = "sha1:";
         break;
      case SHA224:
         scheme = "sha224:";
         break;
      case SHA256:
         scheme = "sha256:";
         break;
      case SHA384:
         scheme = "sha384:";
         break;
      case SHA512:
         scheme = "sha512:";
         break;
      default:
         return NULL;
   }

   scheme_len = strlen(scheme);

   tmp = (char *) realloc((void *) *canlock, locksize + scheme_len + 1);
   if (NULL != tmp) { *canlock = tmp; }
   else
   {
      free((void *) *canlock);
      return NULL;
   }

   memmove((void *) (*canlock + scheme_len), (void *) *canlock, locksize + 1);
   strncpy(*canlock, scheme, scheme_len);
   return (*canlock);
}


#if CL_API_V2
/*
 * Verify a SHA1 cancel key against a cancel lock
 * Returns 0 on success, nonzero on failure.
 */
int sha_verify(const char *key, const char *lock)
{
   return cl_verify(CL_SHA1, key, lock);
}
#endif  /* CL_API_V2 */


/*
 * Verify a cancel key against a cancel lock
 * Returns 0 on success, nonzero on failure.
 */
int cl_verify(int which_hash, const char *key, const char *lock)
{
   int res;
   USHAContext hash_ctx;
   char *templock[1];
   enum SHAversion which_sha;
   size_t key_size, hash_size;
   uint8_t hashbuff[USHAMaxHashSize];

   /* Defeat the fallback to SHA256 default */
   if (CL_INVALID == which_hash)
      return -1;

   /*
    * Ensure that key length is supported
    * Currently the maximum length is for base64 encoded SHA512: 88
    * Theoretical limit: UINT_MAX
    * (must always be representable as 'unsigned int')
    */
   key_size = strlen(key);
   if ((size_t) 88 < key_size)
      return -1;

   which_sha = which_cl_hash(which_hash);

   /* The function 'USHAHashSize()' never returns negative values */
   hash_size = (size_t) USHAHashSize(which_sha);

   if (USHAReset(&hash_ctx, which_sha) != shaSuccess)
      return -1;

   if (USHAInput(&hash_ctx, (const uint8_t *) key, (unsigned int) key_size)
       != shaSuccess)
      return -1;

   if (USHAResult(&hash_ctx, hashbuff) != shaSuccess)
      return -1;

   if (!base64_encode(hashbuff, (int) hash_size, templock))
      return -1;

   res = strcmp(*templock, lock);
   free((void*) *templock);

   return res;
}
