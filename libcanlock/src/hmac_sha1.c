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
#include "sha1.h"
#include "hmac_sha1.h"

#define ipad 0x36
#define opad 0x5c

/* 
 * Encode a string using HMAC - see RFC-2104 for details.
 * Returns the MAC, or NULL on error.
 * Caller needs to free() non-NULL return values..
 */
unsigned char *
hmac_sha1(
            const unsigned char *K,   /* key */
            int Klen,           /* and it size */
            const unsigned char *T,   /* text to encode */
            int Tlen            /* and its size */
        )
{
    SHA_CTX
        hash_ctx;
    unsigned char
        keyin[SHA_DATASIZE],
        *step2,
        step4[SHA_DIGESTSIZE],
        step5[SHA_DATASIZE + SHA_DIGESTSIZE],
        *hmac_out,
        *c;
    int
        i,
        j;

    if (sha_init(&hash_ctx))
        return NULL;

    /* If the key is bigger than SHA_DATASIZE we need to hash it. */
    if (Klen > SHA_DATASIZE) {
        if (sha_update(&hash_ctx, K, Klen))
            return NULL;
        if (sha_digest(&hash_ctx, keyin))
            return NULL;
        Klen = SHA_DIGESTSIZE;
    }
    else
        memcpy(keyin, K, Klen);

    step2 = (unsigned char *) malloc(Tlen + SHA_DATASIZE);

    c = keyin;
    for (i = 0; i < Klen; i++) {
        step2[i] = *c ^ ipad;
        step5[i] = *c ^ opad;
        c++;
    }
    for (j = i; j < SHA_DATASIZE; j++) {
        step2[j] = ipad;
        step5[j] = opad;
    }
    
    memcpy(&step2[SHA_DATASIZE], T, Tlen);

    if (sha_init(&hash_ctx)) {
    	free(step2);
        return NULL;
	}
    if (sha_update(&hash_ctx, step2, SHA_DATASIZE + Tlen)) {
    	free(step2);
        return NULL;
	}
	free(step2);
    if (sha_digest(&hash_ctx, step4))
        return NULL;

    memcpy(&step5[SHA_DATASIZE], step4, SHA_DIGESTSIZE);

    hmac_out = (unsigned char *) malloc(SHA_DIGESTSIZE);
    if (!hmac_out)
        return NULL;

    if (sha_init(&hash_ctx)) {
    	free(hmac_out);
        return NULL;
	}
    if (sha_update(&hash_ctx, step5, SHA_DATASIZE + SHA_DIGESTSIZE)) {
    	free(hmac_out);
        return NULL;
	}
    if (sha_digest(&hash_ctx, hmac_out)) {
    	free(hmac_out);
        return NULL;
	}

    return hmac_out;
}
