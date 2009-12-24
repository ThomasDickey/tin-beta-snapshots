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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "base64.h"
#include "sha1.h"
#include "hmac_sha1.h"
#include "canlock.h"

/* 
 * Return a stripped cancel lock, that is, with the xx: prefix
 * removed, else NULL on failure.
 * type is set to the lock type, else zero on failure.
 */
char *
lock_strip_alpha(char *key, char *type)
{
    char *ret;
    int offset;
    do {
      *type = tolower(*key);
      type++;
      key++;
    } while (*key && *key != ':');
    
    *type = '\0';
    key++;
    ret = strdup (key);
    /* Strip the "Clue-string", no longer part of the lastest
     * draft but could still be present */
    offset = 0;
    while (ret[offset] && ret[offset] != ':')
		offset++;
    ret[offset] = '\0';
    return ret;
}


char *
lock_strip(char *key, char *type)
{
    return lock_strip_alpha(key, type);
}

/* 
 * Generate an SHA1 cancel key.
 * Returns a malloc()'d buffer that the caller will need to free().
 */
char *
sha_key(const unsigned char *secret, size_t seclen,
        const unsigned char *message, size_t msglen)
{
    char
        *cankey[1];
    unsigned char
        *hmacbuff;
    size_t
        keysize;

    hmacbuff = hmac_sha1(secret, seclen, message, msglen);
    if (!hmacbuff)
        return NULL;
    keysize = base64_encode(hmacbuff, SHA_DIGESTSIZE, cankey);
    free ((void *) hmacbuff);
    if (!keysize)
        return NULL;
    *cankey = (char *) realloc((void *) *cankey, keysize + 6);
    if (!*cankey)
        return NULL;
    memmove((void *) (*cankey + 5), (void *) *cankey, keysize + 1);
    strncpy(*cankey, "sha1:", 5);
    return (*cankey);
}

/* 
 * Generate an SHA1 cancel lock.
 * Returns a malloc()'d buffer that the caller will need to free().
 */
char *
sha_lock(const unsigned char *secret, size_t seclen,
         const unsigned char *message, size_t msglen)
{
	char
		*canlock[1],
		*tmp,
		junk[SHA_DIGESTSIZE];
    unsigned char
        *cankey,
        hmacbuff[SHA_DIGESTSIZE];
    size_t
        locksize;
    SHA_CTX
        hash_ctx;

	tmp = sha_key(secret, seclen, message, msglen);
	cankey = (unsigned char *) lock_strip_alpha(tmp, junk);
	free(tmp);
    if (!cankey)
        return NULL;
    if (sha_init(&hash_ctx)) {
    	free(cankey);
        return NULL;
	}
    if (sha_update(&hash_ctx, cankey, strlen((char *) cankey))) {
    	free(cankey);
        return NULL;
	}
	free(cankey);
    if (sha_digest(&hash_ctx, hmacbuff))
        return NULL;
    locksize = base64_encode(hmacbuff, SHA_DIGESTSIZE, canlock);
    if (!locksize)
        return NULL;
    *canlock = (char *) realloc((void *) *canlock, locksize + 6);
    if (!*canlock)
        return NULL;
    memmove((void *) (*canlock + 5), (void *) *canlock, locksize + 1);
    strncpy(*canlock, "sha1:", 5);
    return (*canlock);
}


/* 
 * Verify an SHA cancel key against a cancel lock.
 * Returns 0 on success, nonzero on failure.
 */
int
sha_verify(const char *key, const char *lock)
{
    unsigned char
        binkey[SHA_DIGESTSIZE + 4],
        hmacbuff[SHA_DIGESTSIZE];
    char
        *templock[1];
    size_t
        keysize,
        locksize;
    SHA_CTX
        hash_ctx;


    /* Convert the key back into binary */
    keysize = base64_decode(key, (void *) &binkey);
    if (!keysize)
        return -1;

    if (sha_init(&hash_ctx))
        return -1;
    if (sha_update(&hash_ctx, (unsigned char *)key, strlen(key)))
        return -1;
    if (sha_digest(&hash_ctx, hmacbuff))
        return -1;

    locksize = base64_encode(hmacbuff, SHA_DIGESTSIZE, templock);
    if (!locksize)
        return -1;

    return strcmp(*templock, lock);
}
