/*
 * hash32 - 32 bit Fowler/Noll/Vo hash code
 *
 * @(#) $Revision: 2.10 $
 * @(#) $Id: h32.c,v 2.10 1999/10/19 06:15:45 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/lib/libfnv/RCS/h32.c,v $
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
 * hash_buf - perform a 32 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * retuns:
 *	32 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completeion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	hash32 hash_value;
 *
 *	hash_value = hash32_buf(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) hash32_buf(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatinated with buf2.
 */
hash32
hash32_buf(char *buf, int len, hash32 *hval)
{
    hash32 val;			/* current hash value */
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
     * See:
     *		http://reality.sgi.com/chongo/tech/comp/fnv/index.html
     *
     * for the most up to date copy of this code and the FNV hash home page.
     */
    val = (hval ? *hval : (hash32)0);
    while (buf < buf_end) {

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= (hash32)16777619;

	/* xor the bottom with the current octet */
	val ^= (hash32)(*buf++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* our hash value */
    return *hval;
}


/*
 * hash_str - perform a 32 bit Fowler/Noll/Vo hash on a string
 *
 * input:
 *	str	- string to hash
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * retuns:
 *	32 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completeion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	hash32 hash_value;
 *
 *	hash_value = hash32_str("the first string", NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of "the first string"
 *	    not counting the final NUL byte.
 *
 *	(void) hash32_str("2nd string", &hash_value);
 *
 *	    The 'hash_value' becomes the hash of "the first string2nd string"
 *	    not counting the final NUL byte.
 */
hash32
hash32_str(char *str, hash32 *hval)
{
    hash32 val;			/* current hash value */

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
     * See:
     *		http://reality.sgi.com/chongo/tech/comp/fnv/index.html
     *
     * for the most up to date copy of this code and the FNV hash home page.
     */
    val = (hval ? *hval : (hash32)0);
    while (*str) {

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= (hash32)16777619;

	/* xor the bottom with the current octet */
	val ^= (hash32)(*str++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* our hash value */
    return *hval;
}
