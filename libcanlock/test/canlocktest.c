/* =============================================================================
 * Copyright (c) 2017-2018 Michael Baeuerle
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

/* Test program for new API available since version 3 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "canlock.h"


#define BUFFSIZE 128


/* ========================================================================== */
/* Get <scheme> element */

static const char *check_scheme(cl_hash_version hash)
{
   const char *scheme;

   switch (hash)
   {
      case CL_SHA1:
         scheme = "sha1";
         break;
      case CL_SHA224:
         scheme = "sha224";
         break;
      case CL_SHA256:
         scheme = "sha256";
         break;
      case CL_SHA384:
         scheme = "sha384";
         break;
      case CL_SHA512:
         scheme = "sha512";
         break;
      default:
         /* Not supported */
         return NULL;
   }

   return scheme;
}


/* ========================================================================== */
/* Check whether Cancel-Key matches Cancel-Lock */

static int checker(cl_hash_version hash, char *key, char *lock)
{
   int res = -1;
   cl_hash_version hash_key, hash_lock;
   char *string_key, *string_lock;
   const char *scheme;

   printf("%s\n%s,%s\n", "Check Cancel-Key,Cancel-Lock:", key, lock);

   hash_key = cl_split(key, &string_key);
   hash_lock = cl_split(lock, &string_lock);

   if (hash_key && hash_lock)
   {
      /* Check whether <scheme> matches */
      if (hash_key == hash_lock)
      {
         if (!cl_verify(hash, string_key, string_lock))
         {
            printf("\nGOOD\n");
            res = 0;
         }
         else
            printf("\nBAD\n");
      }
      else
         printf("\nBAD: Scheme mismatch (%s/%s)\n",
                check_scheme(hash_key), check_scheme(hash_lock));
   }
   else
      printf("\nBAD: Scheme not supported\n");

   return res;
}


/* ========================================================================== */
/* Test program for new API from version 3 */

int main(void)
{
   char *c_key = NULL, *c_lock = NULL;
   const char sec[] = "ExampleSecret";
   const char sec2[] = "AnotherSecret";
   const char mid[] = "<12345@mid.example>";
   const char uid[] = "JaneDoe";
   char *uid_mid = NULL;
   int rv;
   int failed = 0;

   /* First test (SHA256 without UID) */
   printf("Test 1 (SHA256 without UID)\n\n");
   printf("Secret    : %s\n", sec);
   printf("Message-ID: %s\n", mid);
   c_key = cl_get_key(CL_SHA256, (const unsigned char *) sec, strlen(sec),
                      (const unsigned char *) mid, strlen(mid));
   c_lock = cl_get_lock(CL_SHA256, (const unsigned char *) sec, strlen(sec),
                        (const unsigned char *) mid, strlen(mid));
   printf("\n");
   printf("%s\n%s,%s\n\n", "Expected Cancel-Key,Cancel-Lock:",
          "sha256:qv1VXHYiCGjkX/N1nhfYKcAeUn8bCVhrWhoKuBSnpMA=",
          "sha256:s/pmK/3grrz++29ce2/mQydzJuc7iqHn1nqcJiQTPMc=");
   if (strcmp(c_key, "sha256:qv1VXHYiCGjkX/N1nhfYKcAeUn8bCVhrWhoKuBSnpMA="))
   {
      printf("BAD: Cancel-Key not as expected (%s)\n", c_key);
      failed = 1;
   }
   else
   {
      if (strcmp(c_lock, "sha256:s/pmK/3grrz++29ce2/mQydzJuc7iqHn1nqcJiQTPMc="))
      {
         printf("BAD: Cancel-Lock not as expected (%s)\n", c_lock);
         failed = 1;
      }
      else
      {
         rv = checker(CL_SHA256, c_key, c_lock);
         if (rv) failed = 1;
      }
   }
   free((void *) c_lock);
   free((void *) c_key);
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Second test (SHA256 with UID) */
   printf("Test 2 (SHA256 with UID)\n\n");
   printf("Secret    : %s\n", sec2);
   printf("User-ID   : %s\n", uid);
   printf("Message-ID: %s\n", mid);
   uid_mid = (char *) malloc(strlen(uid) + strlen(mid) + (size_t) 1);
   strcpy(uid_mid, uid);
   strcat(uid_mid, mid);
   c_key = cl_get_key(CL_SHA256, (const unsigned char *) sec2, strlen(sec2),
                      (const unsigned char *) uid_mid, strlen(uid_mid));
   c_lock = cl_get_lock(CL_SHA256, (const unsigned char *) sec2, strlen(sec2),
                        (const unsigned char *) uid_mid, strlen(uid_mid));
   free((void *) uid_mid);
   printf("\n");
   printf("%s\n%s,%s\n\n", "Expected Cancel-Key,Cancel-Lock:",
          "sha256:yM0ep490Fzt83CLYYAytm3S2HasHhYG4LAeAlmuSEys=",
          "sha256:NSBTz7BfcQFTCen+U4lQ0VS8VIlZao2b8mxD/xJaaeE=");

   if (strcmp(c_key, "sha256:yM0ep490Fzt83CLYYAytm3S2HasHhYG4LAeAlmuSEys="))
   {
      printf("BAD: Cancel-Key not as expected (%s)\n", c_key);
      failed = 1;
   }
   else
   {
      if (strcmp(c_lock, "sha256:NSBTz7BfcQFTCen+U4lQ0VS8VIlZao2b8mxD/xJaaeE="))
      {
         printf("BAD: Cancel-Lock not as expected (%s)\n", c_lock);
         failed = 1;
      }
      else
      {
         rv = checker(CL_SHA256, c_key, c_lock);
         if (rv) failed = 1;
      }
   }
   free((void *) c_lock);
   free((void *) c_key);
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 3 (Check SHA256) */
   printf("Test 3 (Check SHA256)\n\n");
   c_key = "shA256:sSkDke97Dh78/d+Diu1i3dQ2Fp/EMK3xE2GfEqZlvK8=";
   c_lock = "sHa256:RrKLp7YCQc9T8HmgSbxwIDlnCDWsgy1awqtiDuhedRo=";
   rv = checker(CL_SHA256, c_key, c_lock);
   if (rv) failed = 1;
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 4 (Check SHA1) */
   printf("Test 4 (Check SHA1)\n\n");
   c_key = "ShA1:aaaBBBcccDDDeeeFFF";
   c_lock = "sha1:bNXHc6ohSmeHaRHHW56BIWZJt+4=";
   rv = checker(CL_SHA1, c_key, c_lock);
   if (rv) failed = 1;
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 5 (Check SHA1 with <clue-string> element) */
   printf("Test 5 (Check SHA1 with <clue-string> element)\n\n");
#if 0  /* Skip because this requires V2 legacy API */
   c_key = "ShA1:aaaBBBcccDDDeeeFFF:bN";
   c_lock = "sha1:bNXHc6ohSmeHaRHHW56BIWZJt+4=";
   rv = checker(CL_SHA1, c_key, c_lock);
   if (rv) failed = 1;
#endif
   printf("SKIP: <clue-string> is obsolete since >20 years\n");
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 6 (Check SHA512) */
   printf("Test 6 (Check SHA512)\n\n");
   c_key = "sha512:ryoikFW3wKefmYr+zDzKn16ngNf1eYbZ0DN+3yqCbkid3HxU5K99G7RcNEx1UxiL3ZQfwg1+TDhH96D+tCcXGQ==";
   c_lock = "sha512:Hq6MQ2JMzGf56agcqYPEMnoWHbQMSAG0eE0ABHgktP8cKL6/A4bvydjUAa0h7sHUU8vdfWXK7eUYG/pnDxgitg==";
   rv = checker(CL_SHA512, c_key, c_lock);
   if (rv) failed = 1;
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 7 (Check SHA256 with wrong key) */
   printf("Test 7 (Check SHA256 with wrong key)\n\n");
   c_key = "shA256:sSkDke97Dh78/d+Diu1i3dQ2Fp/EMK3xE2GfEqZlvK9=";
   c_lock = "sHa256:RrKLp7YCQc9T8HmgSbxwIDlnCDWsgy1awqtiDuhedRo=";
   rv = checker(CL_SHA256, c_key, c_lock);
   /* BAD is expected and the correct result for this test */
   printf("(Note: BAD is expected and the correct result for this test)\n");
   if (!rv) failed = 1;
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 8 (Check with unknown <scheme>) */
   printf("Test 8 (Check with unknown <scheme>)\n\n");
   c_key = "shA257:sSkDke97Dh78/d+Diu1i3dQ2Fp/EMK3xE2GfEqZlvK8=";
   c_lock = "sHa256:RrKLp7YCQc9T8HmgSbxwIDlnCDWsgy1awqtiDuhedRo=";
   rv = checker(CL_SHA256, c_key, c_lock);
   /* BAD is expected and the correct result for this test */
   printf("(Note: BAD is expected and the correct result for this test)\n");
   if (!rv) failed = 1;
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Test 9 (Check with <scheme> mismatch) */
   printf("Test 9 (Check with <scheme> mismatch)\n\n");
   c_key = "shA256:sSkDke97Dh78/d+Diu1i3dQ2Fp/EMK3xE2GfEqZlvK8=";
   c_lock = "sha1:bNXHc6ohSmeHaRHHW56BIWZJt+4=";
   rv = checker(CL_SHA256, c_key, c_lock);
   /* BAD is expected and the correct result for this test */
   printf("(Note: BAD is expected and the correct result for this test)\n");
   if (!rv) failed = 1;
   printf("\n----------------------------------------"
          "----------------------------------------\n\n");

   /* Check for success */
   if (!failed) exit(EXIT_SUCCESS);
   exit(EXIT_FAILURE);
}


/* EOF */
