/*
 * h2_64 - 64 bit Fowler/Noll/Vo-2 hash code
 *
 * @(#) $Revision: 3.11 $
 * @(#) $Id: h2_64.c,v 3.11 1999/10/29 07:39:03 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnv/TWO/RCS/h2_64.c,v $
 *
 ***
 *
 * This FNV-2 algorithm is under development and has not been fully
 * tested.  Use of this algorithm is not recommended at this time.
 *
 ***
 *
 * Fowler/Noll/Vo-2 hash
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
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *	chongo <Landon Curt Noll> /\oo/\
 *	http://reality.sgi.com/chongo
 *	EMail: chongo_fnv at prime dot engr dot sgi dot com
 *
 * Share and Enjoy!	:-)
 */

#include "fnv2.h"

#define BUF_SIZE (32*1024)	/* number of bytes to hash at a time */


/*
 * This FNV-2 algorithm is under development and has not been fully
 * tested.  Use of this algorithm is not recommended at this time.
 *
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
 */
#if defined(HAVE_64BIT_LONG_LONG)
static Fnv64_t virgin = 0xcbf29ce484222325ULL;
#else
static Fnv64_t virgin = { 0x84222325, 0xcbf29ce4 };
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
 *	Fnv64_t hash_value;
 *
 *	hash_value = fnv2_64_buf(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) fnv2_64_buf(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatenated with buf2.
 */
Fnv64_t
fnv2_64_buf(char *buf, int len, Fnv64_t *hval)
{
#if !defined(HAVE_64BIT_LONG_LONG)
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
#endif
    Fnv64_t ret;			/* 64 bit return value */
    char *buf_end = buf+len;	/* beyond end of hash area */

    /*
     * FNV-2 hash each octet in the buffer
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

	/* FNV-2 pre-increments on every octet */
	++ret;

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	ret *= (Fnv64_t)1099511628211ULL;

	/* xor the bottom with the current octet */
	ret ^= (Fnv64_t)(*buf++);
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
	/* FNV-2 pre-increments on every octet */
	++tmp[0];
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
	 * outside of the loop when we convert to Fnv64_t.
	 */

	/* xor the bottom with the current octet */
	val[0] ^= (unsigned long)(*buf++);
    }

    /* convert to Fnv64_t */
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
 *	Fnv64_t hash_value;
 *
 *	hash_value = fnv2_64_str(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) fnv2_64_str(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatenated with buf2.
 */
Fnv64_t
fnv2_64_str(char *str, Fnv64_t *hval)
{
#if !defined(HAVE_64BIT_LONG_LONG)
    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */
#endif
    Fnv64_t ret;			/* 64 bit return value */

    /*
     * FNV-2 hash each octet in the buffer
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

	/* FNV-2 pre-increments on every octet */
	++ret;

	/* multiply by 1099511628211ULL mod 2^64 using 64 bit longs */
	ret *= (Fnv64_t)1099511628211ULL;

	/* xor the bottom with the current octet */
	ret ^= (Fnv64_t)(0x100 | *str++);
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
	 * outside of the loop when we convert to Fnv64_t.
	 */

	/* xor the bottom with the current octet */
	val[0] ^= (unsigned long)(0x100 | *str++);
    }

    /* convert to Fnv64_t */
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
 * fnv2_64_fd - FNV hash an open filename
 *
 * usage:
 *      fd	- open file descriptor to hash
 *      hash    - hash value to modify or NULL => just return hash value
 *
 * return:
 *      64 bit hash as a static hash type
 */
Fnv64_t
fnv2_64_fd(int fd, Fnv64_t *hval)
{
    char buf[BUF_SIZE+1];	/* read buffer */
    int readcnt;		/* number of characters written */
    Fnv64_t val;			/* current hash value */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * hash until EOF
     */
    while ((readcnt = read(fd, buf, BUF_SIZE)) > 0) {
	(void) fnv2_64_buf(buf, readcnt, &val);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}
