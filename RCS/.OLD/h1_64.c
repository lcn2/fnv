/*
 * hash64 - 64 bit Fowler/Noll/Vo64 hash code
 *
 * @(#) $Revision: 2.2 $
 * @(#) $Id: h64.c,v 2.2 1999/10/19 06:15:45 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/lib/libfnv/RCS/h64.c,v $
 *
 * See:
 *	http://reality.sgi.com/chongo/tech/comp/fnv/index.html
 *
 * for the most up to date copy of this code and the FNV hash home page.
 *
 * Copyright (C) 1997 Landon Curt Noll, all rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *       supporting documentation
 *       source copies
 *       source works derived from this source
 *       binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo <Landon Curt Noll> /\oo/\
 * chongo_fnv@prime.engr.sgi.com
 *
 * Share and Enjoy!
 */

#include "fnv.h"


/*
 * hash_buf - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * retuns:
 *	64 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completeion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	hash64 hash_value;
 *
 *	hash_value = hash64_buf(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) hash64_buf(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatinated with buf2.
 */
hash64
hash64_buf(char *buf, int len, hash64 *hval)
{
#if defined(HAVE_64BIT_LONG_LONG)
    unsigned long long val;	/* current hash value */
#else
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
#endif
    char *buf_end = buf+len;	/* beyond end of hash area */

    /*
     * Fowler/Noll/Vo hash - hash each character in the buffer
     *
     * The basis of the hash algorithm was taken from an idea
     * sent by Email to the IEEE Posix P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (chongo@toad.com) later improved on their
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * The 32 hash was able to process 234936 words from the web2 dictionary
     * without any 32 bit collisions using a constant of 16777619 = 0x1000193.
     *
     * The 64 bit hases uses 1099511628211 = 0x100000001b3 instead.
     *
     * See:
     *		http://reality.sgi.com/chongo/tech/comp/fnv/index.html
     *
     * for the most up to date copy of this code and the FNV hash home page.
     *
     */
#if defined(HAVE_64BIT_LONG_LONG)

    /* 
     * setup the initial hash value 
     */
    val = (hval ? *hval : (hash64)0);

    /* 
     * hash each octet of the buffer 
     */
    while (buf < buf_end) {

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	val *= (hash64)1099511628211ULL;

	/* xor the bottom with the current octet */
	val ^= (hash64)(*buf++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

#else

    /*
     * setup the initial hash value
     */
    if (hval) {
	val[0] = hval->w32[0];
	val[1] = (val[0] >> 16);
	val[0] &= 0xffff;
	val[2] = hval->w32[1];
	val[3] = (val[2] >> 16);
	val[2] &= 0xffff;
    } else {
	val[0] = val[1] = val[2] = val[3] = (unsigned long)0;
    }

    /* 
     * hash each octet of the buffer 
     */
    while (buf < buf_end) {

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
	val[0] ^= (unsigned long)(*buf++);
    }

    /* convert to hash64 */
    if (hval) {
	hval->w32[1] = ((val[3]<<16)&0xffff) + val[2];
	hval->w32[0] = val[1]<<16 + val[0];
    }

#endif

    /* our hash value */
    return *hval;
}


/*
 * hash_str - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * retuns:
 *	64 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completeion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	hash64 hash_value;
 *
 *	hash_value = hash64_str(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) hash64_str(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatinated with buf2.
 */
hash64
hash64_str(char *str, hash64 *hval)
{
#if defined(HAVE_64BIT_LONG_LONG)
    unsigned long long val;	/* current hash value */
#else
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
#endif

    /*
     * Fowler/Noll/Vo hash - hash each character in the string
     *
     * The basis of the hash algorithm was taken from an idea
     * sent by Email to the IEEE Posix P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (chongo@toad.com) later improved on their
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * The 32 hash was able to process 234936 words from the web2 dictionary
     * without any 32 bit collisions using a constant of 16777619 = 0x1000193.
     *
     * The 64 bit hases uses 1099511628211 = 0x100000001b3 instead.
     *
     * See:
     *		http://reality.sgi.com/chongo/tech/comp/fnv/index.html
     *
     * for the most up to date copy of this code and the FNV hash home page.
     *
     */
#if defined(HAVE_64BIT_LONG_LONG)

    /* 
     * setup the initial hash value 
     */
    val = (hval ? *hval : (hash64)0);

    /* 
     * hash each octet of the buffer 
     */
    while (*str) {

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	val *= (hash64)1099511628211ULL;

	/* xor the bottom with the current octet */
	val ^= (hash64)(*str++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

#else

    /*
     * setup the initial hash value
     */
    if (hval) {
	val[0] = hval->w32[0];
	val[1] = (val[0] >> 16);
	val[0] &= 0xffff;
	val[2] = hval->w32[1];
	val[3] = (val[2] >> 16);
	val[2] &= 0xffff;
    } else {
	val[0] = val[1] = val[2] = val[3] = (unsigned long)0;
    }

    /* 
     * hash each octet of the buffer 
     */
    while (*str) {

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
	val[0] ^= (unsigned long)(*str++);
    }

    /* convert to hash64 */
    if (hval) {
	hval->w32[1] = ((val[3]<<16)&0xffff) + val[2];
	hval->w32[0] = val[1]<<16 + val[0];
    }

#endif

    /* our hash value */
    return *hval;
}
