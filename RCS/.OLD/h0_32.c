/*
 * h0_32 - 32 bit Fowler/Noll/Vo-0 hash code
 *
 * @(#) $Revision: 3.8 $
 * @(#) $Id: h0_32.c,v 3.8 1999/10/27 01:53:29 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/h0_32.c,v $
 *
 ***
 *
 * This is the original historic FNV algorithm with a 0 offset basis.
 * It is recommended that FNV-1 (with a non-0 offset basis) be used instead.
 *
 ***
 *
 * Fowler/Noll/Vo-0 hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://reality.sgi.com/chongo)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are architected to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://reality.sgi.com/chongo/tech/comp/fnv/
 *
 * for more details as well as other forms of the FNV hash.
 *
 ***
 *
 * Copyright (C) 1999 Landon Curt Noll, all rights reserved.
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
 * http://reality.sgi.com/chongo
 * EMail: chongo_fnv at prime dot engr dot sgi dot com
 *
 * Share and Enjoy!	:-)
 */

#include "fnv0.h"

#define BUF_SIZE (32*1024)	/* number of bytes to hash at a time */


/*
 * This is the original historic FNV algorithm with a 0 offset basis.
 * It is recommended that FNV-1, with a non-0 offset basis be used instead.
 */
static fnv32 virgin = 0;


/*
 * hash_buf - perform a 32 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * returns:
 *	32 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	fnv32 hash_value;
 *
 *	hash_value = fnv0_32_buf(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) fnv0_32_buf(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatenated with buf2.
 */
fnv32
fnv0_32_buf(char *buf, int len, fnv32 *hval)
{
    fnv32 val;			/* current hash value */
    char *buf_end = buf+len;	/* beyond end of hash area */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * FNV-0 hash each octet in the buffer
     */
    while (buf < buf_end) {

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= (fnv32)16777619;

	/* xor the bottom with the current octet */
	val ^= (fnv32)(*buf++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}


/*
 * hash_str - perform a 32 bit Fowler/Noll/Vo hash on a string
 *
 * input:
 *	str	- string to hash
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * returns:
 *	32 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	fnv32 hash_value;
 *
 *	hash_value = fnv0_32_str("the first string", NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of "the first string"
 *	    not counting the final NUL byte.
 *
 *	(void) fnv0_32_str("2nd string", &hash_value);
 *
 *	    The 'hash_value' becomes the hash of "the first string2nd string"
 *	    not counting the final NUL byte.
 */
fnv32
fnv0_32_str(char *str, fnv32 *hval)
{
    fnv32 val;			/* current hash value */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * FNV-0 hash each octet in the buffer
     */
    while (*str) {

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= (fnv32)16777619;

	/* xor the bottom with the current octet */
	val ^= (fnv32)(*str++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}


/*
 * fnv0_32_fd - FNV hash an open filename
 *
 * usage:
 *      fd	- open file descriptor to hash
 *      hash    - hash value to modify or NULL => just return hash value
 *
 * return:
 *      32 bit hash as a static hash type
 */
fnv32
fnv0_32_fd(int fd, fnv32 *hval)
{
    char buf[BUF_SIZE+1];	/* read buffer */
    int readcnt;		/* number of characters written */
    fnv32 val;			/* current hash value */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * hash until EOF
     */
    while ((readcnt = read(fd, buf, BUF_SIZE)) > 0) {
	(void) fnv0_32_buf(buf, readcnt, &val);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}
