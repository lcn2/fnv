/* $Id$  */
/*
 * hash32 - 32 bit Fowler/Noll/Vo hash code
 *
 * Copyright (C) 1997 Landon Curt Noll, all rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice, and the
 * disclaimer below appear in all of the following:
 * 
 *         * supporting documentation
 *         * source copies 
 *         * source works derived from this source
 *         * binaries derived from this source or from derived source
 * 
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
char *fnv_copyright=""@(#) $Revision$ $Date$ : Fowler/Noll/Vo hash: Copyright (C) 1997 Landon Curt Noll, all rights reserved. Used in InterNetNews by permission.";


/*
 * 32 bit hash value
 */
typedef unsigned long hash;


/*
 * hash_buf - perform a 32 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- the hash value to modify
 *
 * retuns:
 *	32 bit hash as a static hash type
 */
hash
hash_buf(char *buf, int len, hash *hval)
{
    unsigned long val;		/* current hash value */
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
     */
    /* setup */
    val = 0;

    /* hash each octet of the buffer */
    for (i = 0; i < len; ++i, ++buf) {

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= 16777619;

	/* xor the bottom with the current octet */
	val ^= (unsigned long)(*buf);
    }

    /* convert to hash */
    *hval = val;

    /* our hash value */
    return *hval;
}
