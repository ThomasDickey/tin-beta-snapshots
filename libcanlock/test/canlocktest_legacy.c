/*
 * COPYRIGHT AND PERMISSION NOTICE
 *
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

/*
 * Test program for legacy API from version 2
 *
 * This program doesn't really do anything but lightly exercise all the
 * library functions, so you can make sure it all compiled correctly.
 * Everything's kept simple so that you can also see how they would be
 * called in a real application.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "canlock.h"


#define BUFFSIZE 512


static int checker(char *key, char *lock)
{
   int res = -1;
   char *rawkey, *rawlock;
   char keytype[BUFFSIZE], locktype[BUFFSIZE];

   printf("L %s K %s ", lock, key);

   rawkey = lock_strip_alpha(key, keytype);
   rawlock = lock_strip_alpha(lock, locktype);

   if (!strcmp(keytype, locktype))
   {
      if (!strcmp(keytype, "sha1"))
      {
         if (!sha_verify(rawkey, rawlock))
         {
            printf(" GOOD\n");
            res = 0;
         }
         else
            printf(" BAD\n");
      }
      else
         printf(" SCHEME NOT SUPPORTED\n");
   }
   else
      printf(" SCHEME MISMATCH %s/%s\n", keytype, locktype);

   return res;
}


int main(void)
{
   char cankey[256], canlock[256], *lkey, *llock;
   unsigned char secret[] = "fluffy",
      message[] = "<lkr905851929.22670@meow.invalid>";
   int rv, rv2;
   int failed = 0;

   printf("Secret %s\n", secret);
   printf("Message %s\n", message);

   llock = sha_lock(secret, strlen((char *) secret),
                    message, strlen((char *) message));
   lkey = sha_key(secret, strlen((char *) secret),
                  message, strlen((char *)message));

   printf("%s%s %s\n", "SHA Expect Lock/Key:\n",
          "L sha1:ScU1gyAi9bd/aFEOyzg4m99lwXs=",
          "K sha1:C1Me/4n0l/V778Ih3J2UnhAoHrA=");

   rv = checker(lkey, llock);
   free((void *) llock);
   free((void *) lkey);
   printf("---\n");
   if (rv) failed = 1;

/*********/

   printf("Testing against usefor cancel lock draft 01 samples...\n");

   sprintf(canlock, "%s", "sha1:bNXHc6ohSmeHaRHHW56BIWZJt+4=");
   sprintf(cankey, "%s", "sha1:aaaBBBcccDDDeeeFFF");
   rv = checker(cankey, canlock);
   printf("---above should have been GOOD---\n");
   if (rv) failed = 1;

/*********/

   sprintf(canlock, "%s", "SHA1:H7/zsCUemvbvSDyARDaMs6AQu5s=");
   sprintf(cankey, "%s", "sha1:chW8hNeDx3iNUsGBU6/ezDk88P4=");
   rv = checker(cankey, canlock);

   sprintf(canlock, "%s", "SHA1:H7/zsCUemvbvSDyARDaMs6AQu5s=");
   sprintf(cankey, "%s", "sha1:4srkWaRIzvK51ArAP:Hc");
   rv2 = checker(cankey, canlock);
   printf("---above should have been GOOD and BAD---\n");
   if (rv || !rv2) failed = 1;

/*********/

   sprintf(canlock, "%s", "sha1:JyEBL4w9/abCBuzCxMIE/E73GM4=");
   sprintf(cankey, "%s", "sha1:K4rkWRjRcXmIzvK51ArAP:Jy");
   rv = checker(cankey, canlock);

   sprintf(canlock, "%s", "sha1:2Bmg+zWaY1noRiCdy8k3IapwSDU=");
   sprintf(cankey, "%s", "sha1:K4rkWRjRcXmIzvK51ArAP:Jy");
   rv2 = checker(cankey, canlock);
   printf("---above should have been GOOD and BAD---\n");
   if (rv || !rv2) failed = 1;
   printf("\n");

   /* Check for success */
   if (!failed) exit(EXIT_SUCCESS);
   exit(EXIT_FAILURE);
}
