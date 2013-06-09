#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <libgen.h>

#include "aes.h"
#include "utility.h"
#include "genrand.h"

#define PATH_URANDOM    "/dev/urandom"

#define PAYLOAD_SZ  16
#define KEY_SZ      16

aes_context  ctx;
unsigned char ctr[PAYLOAD_SZ];

/* Increment 128 bit counter.
 * NOTE: we are not concerned with endianness here since the counter is
 * just sixteen bytes of payload to AES and outside of this function isn't 
 * operated upon numerically.
 */
void incr128(unsigned char *val)
{
    unsigned long long *t = (unsigned long long *)val;

    //assert(sizeof(unsigned long long) == 8);
    //assert(PAYLOAD_SZ == 16);

    if (++t[0] == 0)
        ++t[1];
}

/* Copy 'buflen' bytes of raw randomness into 'buf'.
 */
void genrandraw(unsigned char *buf, int buflen)
{
    int fd, n = 0;

    if ((fd = open(PATH_URANDOM, O_RDONLY)) >= 0)
    {
        n = Utility::ReadData(fd, buf, buflen);

        (void)close(fd);
    }

    // Use rand in case reading from urandom failed
    if (fd < 0  || n <= 0)
    {
        for (n = 0; n < buflen; n++)
            buf[n] = rand();
    }
}

/* Pick new (random) key and counter values.
 */
int churnrand()
{
    unsigned char key[KEY_SZ];

    genrandraw(ctr, PAYLOAD_SZ);
    genrandraw(key, KEY_SZ);

    if (aes_set_key(&ctx, key, KEY_SZ*8) != 0)
    {
        //fprintf(stderr, "aes_set_key error\n");
        //exit(1);
        return 1;
    }

    return 0;
}

/* Initialize the module.
 */
int initrand()
{
    struct timeval tv;

    if (access(PATH_URANDOM, R_OK) < 0)
    {
        if (gettimeofday(&tv, NULL) < 0)
        {
            //fprintf(stderr, "gettimeofday: %s\n", strerror(errno));
            //exit(1);
            return 1;
        }
        srand(tv.tv_usec);
    }

    if (churnrand() != 0)
        return 1;

    return 0;
}

/* Fill buf with random data.
 */
void genrand(char *buf, int buflen)
{
    int i;
    unsigned char out[PAYLOAD_SZ];
    int cpylen = PAYLOAD_SZ;

    for (i = 0; i < buflen; i += cpylen)
    {
        aes_encrypt(&ctx, ctr, out);
        incr128(ctr);

        if (cpylen > buflen - i)
            cpylen = buflen - i;

        memcpy(&buf[i], out, cpylen);
    }

    //assert(i == buflen);
}
