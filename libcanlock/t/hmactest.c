/*
 * hmac test program
 */

#include <stdio.h>
#include <string.h>
#include "sha1.h"
#include "hmac_sha1.h"

int
main(void)
{
    unsigned char
        *hmachash,
        key1[] = "Jefe",
        message1[] = "what do ya want for nothing?",
        key2[20],
        message2[] = "Hi There",
        key3[80],
        message3[] = "Test Using Larger Than Block-Size Key - Hash Key First",
        key4[80],
        message4[] = "Test Using Larger Than Block-Size Key and Larger "
                     "Than One Block-Size Data",
        key5[] = {  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                    0x19, 0x00
                 },
        message5[50];
    int
        i;


    for (i = 0; i < 20; i++)
        key2[i] = 0x0b;

    for (i = 0; i < 80; i++)
        key3[i] = 0xaa;

    for (i = 0; i < 80; i++)
        key4[i] = 0xaa;

    for (i = 0; i < 50; i++)
        message5[i] = 0xcd;

    printf("Key: %s\n", key1);
    printf("Msg: %s\n", message1);
    hmachash = hmac_sha1(key1, strlen((char *) key1),
                         message1, strlen((char *)message1));
    printf("Expected SHA Digest: %s\n",
           "0xeffcdf6ae5eb2fa2d27416d5f184df9c259a7c79");
    printf("  Actual SHA Digest: 0x");
    for (i = 0; i < SHA_DIGESTSIZE; i++)
        printf("%02x", hmachash[i]);
    putchar('\n');


/********/

    printf("\nKey: 0x0b, len 20 for SHA, 16 for MD5\n");
    printf("Msg: %s\n", message2);


    hmachash = hmac_sha1(key2, 20, message2, strlen((char *) message2));
    printf("Expected SHA Digest: %s\n",
           "0xb617318655057264e28bc0b6fb378c8ef146be00");
    printf("  Actual SHA Digest: 0x");
    for (i = 0; i < SHA_DIGESTSIZE; i++)
        printf("%02x", hmachash[i]);
    putchar('\n');

/********/

    printf("\nKey: 0xaa repeated 80 times\n");
    printf("Msg: %s\n", message3);

    hmachash = hmac_sha1(key3, 80, message3, strlen((char *) message3));
    printf("Expected SHA Digest: %s\n",
           "0xaa4ae5e15272d00e95705637ce8a3b55ed402112");
    printf("  Actual SHA Digest: 0x");
    for (i = 0; i < SHA_DIGESTSIZE; i++)
        printf("%02x", hmachash[i]);
    putchar('\n');

/********/

    printf("\nKey: 0xaa repeated 80 times\n");
    printf("Msg: %s\n", message4);

    hmachash = hmac_sha1(key4, 80, message4, strlen((char *) message4));
    printf("Expected SHA Digest: %s\n",
           "0xe8e99d0f45237d786d6bbaa7965c7808bbff1a91");
    printf("  Actual SHA Digest: 0x");
    for (i = 0; i < SHA_DIGESTSIZE; i++)
        printf("%02x", hmachash[i]);
    putchar('\n');

/********/

    printf("\nKey: 0x");
    for (i = 0; i < 25; i++)
        printf("%02x", key5[i]);

    printf("\nMsg: 0xcd repeated 50 times\n");

    hmachash = hmac_sha1(key5, 25, message5, 50);
    printf("Expected SHA Digest: %s\n",
           "0x4c9007f4026250c6bc8414f9bf50c86c2d7235da");
    printf("  Actual SHA Digest: 0x");
    for (i = 0; i < SHA_DIGESTSIZE; i++)
        printf("%02x", hmachash[i]);
    putchar('\n');

    return 0;
}
