/*
 * h_64 - 64 bit Fowler/Noll/Vo hash code
 *
 * @(#) $Revision: 3.4 $
 * @(#) $Id: h_64.c,v 3.4 1999/10/24 00:05:01 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/h_64.c,v $
 *
 * See:
 *	http://reality.sgi.com/chongo/tech/comp/fnv/index.html
 *
 * for the most up to date copy of this code and the FNV hash home page.
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
 * chongo_fnv@prime.engr.sgi.com
 *
 * Share and Enjoy!	:-)
 */

#include "fnv.h"

#define BUF_SIZE (32*1024)	/* number of bytes to hash at a time */


/*
 * We start the hash at a non-zero value at the beginning so that
 * hashing blocks of data with all 0 bits do not map onto the same
 * 0 hash value.  The virgin value that we use below is the hash value
 * that we would get from following 32 ASCII characters:
 *
 *		chongo <Landon Curt Noll> /\../\
 *
 * Note that the \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 *
 * The effect of this virgin initial value is the same as starting
 * with 0 and pre-pending those 32 characters onto the data being
 * hashed.
 *
 * Yes, even with this non-zero virgin value there is a set of data
 * that will result in a zero hash value.  Worse, appending any
 * about of zero bytes will continue to produce a zero hash value.
 * But that would happen with any initial value so long as the
 * hash of the initial was the `inverse' of the virgin prefix string.
 *
 * But then again for any hash function, there exists sets of data
 * which that the hash of every member is the same value.  That is
 * life with many to few mapping functions.  All we do here is to
 * prevent sets whose members consist of 0 or more bytes of 0's from
 * being such an awkward set.
 *
 * And yes, someone can figure out what the magic 'inverse' of the
 * 32 ASCII character are ... but this hash function is NOT intended
 * to be a cryptographic hash function, just a fast and reasonably
 * good hash function.
 */
#if defined(ZERO_BASED)
# if defined(HAVE_64BIT_LONG_LONG)
static fnv64 virgin = 0;
# else
static fnv64 virgin = { 0, 0 };
# endif
#else
# if defined(HAVE_64BIT_LONG_LONG)
static fnv64 virgin = 0xcbf29ce484222325;
# else
static fnv64 virgin = { 0x84222325, 0xcbf29ce4 };
# endif
#endif


/*
 * hash_buf - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * returns:
 *	64 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	fnv64 hash_value;
 *
 *	hash_value = fnv_64_buf(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) fnv_64_buf(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatenated with buf2.
 */
fnv64
fnv_64_buf(char *buf, int len, fnv64 *hval)
{
#if !defined(HAVE_64BIT_LONG_LONG)
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
#endif
    fnv64 ret;			/* 64 bit return value */
    char *buf_end = buf+len;	/* beyond end of hash area */

    /*
     * Fowler/Noll/Vo hash - hash each character in the buffer
     *
     * The basis of the hash algorithm was taken from an idea
     * sent by Email to the IEEE POSIX P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (http://reality.sgi.com/chongo) later improved on their
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * The 32 hash was able to process 234936 words from the web2 dictionary
     * without any 32 bit collisions using a constant of 16777619 = 0x1000193.
     *
     * The 64 bit hash uses 1099511628211 = 0x100000001b3 instead.
     *
     * See:
     *		http://reality.sgi.com/chongo/tech/comp/fnv/index.html
     *
     * !for the most up to date copy of this code and the FNV hash home page.
     *
     */
#if defined(HAVE_64BIT_LONG_LONG)

    /*
     * load or initialize hash value
     */
    ret = (hval ? *hval : virgin);

    /*
     * hash each octet of the buffer
     */
    while (buf < buf_end) {

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	ret *= (fnv64)1099511628211ULL;

	/* xor the bottom with the current octet */
	ret ^= (fnv64)(*buf++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = ret;
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
	val[0] = virgin.w32[0];
	val[1] = (val[0] >> 16);
	val[0] &= 0xffff;
	val[2] = virgin.w32[1];
	val[3] = (val[2] >> 16);
	val[2] &= 0xffff;
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
	val[3] = tmp[3] + (tmp[2] >> 16);
	val[2] = tmp[2] & 0xffff;
	/*
	 * Doing a val[3] &= 0xffff; is not really needed since it simply
	 * removes multiples of 2^64.  We can discard these excess bits
	 * outside of the loop when we convert to fnv64.
	 */

	/* xor the bottom with the current octet */
	val[0] ^= (unsigned long)(*buf++);
    }

    /* convert to fnv64 */
    ret.w32[1] = ((val[3]<<16) | val[2]);
    ret.w32[0] = ((val[1]<<16) | val[0]);

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = ret;
    }
#endif

    /* return our new hash value */
    return ret;
}


/*
 * hash_str - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	hval	- hash value to modify or NULL => just return hash value
 *
 * returns:
 *	64 bit hash as a static hash type
 *	*hval is also set to the returned hash value if it was non-NULL
 *
 * NOTE: If hval is NULL, this routine starts with a 0 hash value and
 * 	 returns the hash value.  If hval is non-NULL, that what it points
 *	 to as used as the previous hash value and on completion becomes
 *	 the new hash value as well as returning the new hash value.
 *
 * Example:
 *	fnv64 hash_value;
 *
 *	hash_value = fnv_64_str(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) fnv_64_str(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatenated with buf2.
 */
fnv64
fnv_64_str(char *str, fnv64 *hval)
{
#if !defined(HAVE_64BIT_LONG_LONG)
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
#endif
    fnv64 ret;			/* 64 bit return value */

    /*
     * Fowler/Noll/Vo hash - hash each character in the string
     *
     * The basis of the hash algorithm was taken from an idea
     * sent by Email to the IEEE POSIX P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (http://reality.sgi.com/chongo) later improved on their
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * The 32 hash was able to process 234936 words from the web2 dictionary
     * without any 32 bit collisions using a constant of 16777619 = 0x1000193.
     *
     * The 64 bit hash uses 1099511628211 = 0x100000001b3 instead.
     *
     * See:
     *		http://reality.sgi.com/chongo/tech/comp/fnv/index.html
     *
     * for the most up to date copy of this code and the FNV hash home page.
     *
     */
#if defined(HAVE_64BIT_LONG_LONG)

    /*
     * load or initialize hash value
     */
    ret = (hval ? *hval : virgin);

    /*
     * hash each octet of the buffer
     */
    while (*str) {

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	ret *= (fnv64)1099511628211ULL;

	/* xor the bottom with the current octet */
	ret ^= (fnv64)(*str++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = ret;
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
	val[0] = virgin.w32[0];
	val[1] = (val[0] >> 16);
	val[0] &= 0xffff;
	val[2] = virgin.w32[1];
	val[3] = (val[2] >> 16);
	val[2] &= 0xffff;
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
	val[3] = tmp[3] + (tmp[2] >> 16);
	val[2] = tmp[2] & 0xffff;
	/*
	 * Doing a val[3] &= 0xffff; is not really needed since it simply
	 * removes multiples of 2^64.  We can discard these excess bits
	 * outside of the loop when we convert to fnv64.
	 */

	/* xor the bottom with the current octet */
	val[0] ^= (unsigned long)(*str++);
    }

    /* convert to fnv64 */
    ret.w32[1] = ((val[3]<<16) | val[2]);
    ret.w32[0] = ((val[1]<<16) | val[0]);

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = ret;
    }
#endif

    /* return our new hash value */
    return ret;
}


/*
 * fnv_64_fd - FNV hash an open filename
 *
 * usage:
 *      fd	- open file descriptor to hash
 *      hash    - hash value to modify or NULL => just return hash value
 *
 * return:
 *      64 bit hash as a static hash type
 */
fnv64
fnv_64_fd(int fd, fnv64 *hval)
{
    char buf[BUF_SIZE+1];	/* read buffer */
    int readcnt;		/* number of characters written */
    fnv64 val;			/* current hash value */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * hash until EOF
     */
    while ((readcnt = read(fd, buf, BUF_SIZE)) > 0) {
	(void) fnv_64_buf(buf, readcnt, &val);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}
