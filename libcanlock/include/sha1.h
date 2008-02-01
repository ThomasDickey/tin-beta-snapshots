#ifndef _SHA1_H_
#define _SHA1_H_

/* The SHA block size and message digest sizes, in bytes */

#define SHA_DATASIZE    64
#define SHA_DATALEN     16
#define SHA_DIGESTSIZE  20
#define SHA_DIGESTLEN    5
/* The structure for storing SHA info */

#include <stdint.h>

typedef struct sha_ctx {
  uint32_t digest[SHA_DIGESTLEN];  /* Message digest */
  uint32_t count_l, count_h;       /* 64-bit block count */
  uint8_t block[SHA_DATASIZE];     /* SHA data buffer */
  int index;                             /* index into buffer */
  int finalized;
} SHA_CTX;

int sha_init(struct sha_ctx *ctx);
int sha_update(struct sha_ctx *ctx, const uint8_t *buffer, uint32_t len);
void sha_final(struct sha_ctx *ctx);
int sha_digest(struct sha_ctx *ctx, uint8_t *s);
void sha_copy(struct sha_ctx *dest, struct sha_ctx *src);

#if 1

#ifndef EXTRACT_UCHAR
#define EXTRACT_UCHAR(p)  (*(unsigned char *)(p))
#endif

#define STRING2INT(s) ((((((EXTRACT_UCHAR(s) << 8)    \
			 | EXTRACT_UCHAR(s+1)) << 8)  \
			 | EXTRACT_UCHAR(s+2)) << 8)  \
			 | EXTRACT_UCHAR(s+3))
#else
uint32_t STRING2INT(uint8_t *s)
{
  uint32_t r;
  int i;
  
  for (i = 0, r = 0; i < 4; i++, s++)
    r = (r << 8) | *s;
  return r;
}
#endif

#endif
