/* $Id$  */
/*
 * hash64 - 64 bit Fowler/Noll/Vo64 hash code
 *
 * Copyright (C) 1997 Landon Curt Noll, all rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose is hereby granted, provided that
 * the above copyright, this permission notice, and the disclaimer
 * below appear in all of the following:
 * 
 *         * supporting documentation
 *         * source copies 
 *         * source works derived from this source
 * 
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
char *fnv64_copyright="@(#) $Revision$ $Date$ : Fowler/Noll/Vo64 hash: Copyright (C) 1997 Landon Curt Noll, all rights reserved. Used in InterNetNews by permission.";


/*
 * 64 bit hash value
 */
#if defined(HAVE_64BIT_LONG_LONG)
typedef unsigned long long hash64;
#else
struct s_hash64 {
    unsigned long w32[2];
};
typedef struct s_hash64 hash64;
#endif


/*
 * hash_buf - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- the hash value to modify
 *
 * retuns:
 *	64 bit hash as a static hash64 structure
 */
hash64
hash_buf(char *buf, int len, hash64 *hval)
{
#if defined(HAVE_64BIT_LONG_LONG)
    unsigned long long val;	/* current hash value */
#else
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
    int j;
#endif
    char c;			/* the current string character */
    int i;

    /*
     * Fowler/Noll/Vo hash - hash each character in the string
     *
     * The basis of the hash algorithm was taken from an idea
     * sent by Email to the IEEE Posix P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (chongo@toad.com) later improved on there
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * The 32 hash was able to process 234936 words from the web2 dictionary
     * without any 32 bit collisions using a constant of 16777619 = 0x1000193.
     *
     * The 64 bit hases uses 1099511628211 = 0x100000001b3 instead.
     */
#if defined(HAVE_64BIT_LONG_LONG)

    /* setup */
    val = 0;

    /* hash each octet of the buffer */
    for (i = 0; i < len; ++i, ++buf) {

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	val *= 1099511628211ULL;

	/* xor the bottom with the current octet */
	val ^= (unsigned long long)(*buf);
    }

    /* convert to hash64 */
    *hval = val;

#else

    /* setup */
    for (i=0; i < 4; ++i) {
	val[i] = 0;
    }

    /* hash each octet of the buffer */
    for (i=0; i < len; ++i, ++buf) {

	/* 
	 * multiply by 1099511628211 mod 2^64 using 32 bit longs 
	 *
	 * Using 1099511628211, we have the following digits base 2^16:
	 *
	 *	0x0	0x100	0x0	0x1b3
	 */
	/* multiply by the lowest order digit base 2^16 */
	tmp[0] = val[0] * 0x1b3;
	tmp[1] = val[1] * 0x1b3;
	tmp[2] = val[2] * 0x1b3;
	tmp[3] = val[3] * 0x1b3;
	/* multiply by the other non-zero digit */
	tmp[2] += val[0] << 8;		/* tmp[2] += val[0] * 0x100 */
	tmp[3] += val[1] << 8;		/* tmp[1] += val[1] * 0x100 */
	/* proapage carries */
	tmp[1] += (tmp[0] >> 16);
	val[0] = tmp[0] & 0xffff;
	tmp[2] += (tmp[1] >> 16);
	val[1] = tmp[1] & 0xffff;
	val[3] += (tmp[2] >> 16);
	val[2] = tmp[2] & 0xffff;
	/* 
	 * Doing a val[3] &= 0xffff; is not really needed since it simply
	 * removes multiples of 2^64.  We can discard these excess bits
	 * outside of the loop when we convert to hash64.
	 */

	/* xor the bottom with the current octet */
	val[0] ^= (unsigned long)(*buf);
    }

    /* convert to hash64 */
    hval->w32[1] = ((val[3]<<16)&0xffff) + val[2];
    hval->w32[0] = val[1]<<16 + val[0];

#endif
    
    /* our hash value */
    return *hval;
}
