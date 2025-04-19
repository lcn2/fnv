/*
 * hash_64 - 64 bit Fowler/Noll/Vo-0 hash code
 *
 ***
 *
 * For the most up to date copy of this code, see:
 *
 *	https://github.com/lcn2/fnv
 *
 * For more information on the FNV hash, see:
 *
 *	http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 ***
 *
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.
 *
 ***
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org>
 *
 ***
 *
 * Author:
 *
 * chongo (Landon Curt Noll) /\oo/\
 *
 * http://www.isthe.com/chongo/index.html
 * https://github.com/lcn2
 *
 * Share and enjoy!  :-)
 */


#include <stdlib.h>
#include "fnv.h"


/*
 * FNV-0 defines the initial basis to be zero
 */
#if !defined(HAVE_64BIT_LONG_LONG)
const Fnv64_t fnv0_64_init = { 0UL, 0UL };
#endif /* ! HAVE_64BIT_LONG_LONG */


/*
 * FNV-1 defines the initial basis to be non-zero
 */
#if !defined(HAVE_64BIT_LONG_LONG)
const Fnv64_t fnv1_64_init = { 0x84222325UL, 0xcbf29ce4UL };
#endif /* ! HAVE_64BIT_LONG_LONG */


/*
 * 64 bit magic FNV-0 and FNV-1 prime
 */
#if defined(HAVE_64BIT_LONG_LONG)
#define FNV_64_PRIME ((Fnv64_t)0x100000001b3ULL)
#else /* HAVE_64BIT_LONG_LONG */
#define FNV_64_PRIME_LOW ((unsigned long)0x1b3)	/* lower bits of FNV prime */
#define FNV_64_PRIME_SHIFT (8)		/* top FNV prime shift above 2^32 */
#endif /* HAVE_64BIT_LONG_LONG */


/*
 * fnv_64_buf - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- previous hash value or 0 if first call
 *
 * returns:
 *	64 bit hash as a static hash type
 *
 * NOTE: To use the 64 bit FNV-0 historic hash, use FNV0_64_INIT as the hval
 *	 argument on the first call to either fnv_64_buf() or fnv_64_str().
 *
 * NOTE: To use the recommended 64 bit FNV-1 hash, use FNV1_64_INIT as the hval
 *	 argument on the first call to either fnv_64_buf() or fnv_64_str().
 */
Fnv64_t
fnv_64_buf(void *buf, size_t len, Fnv64_t hval)
{
    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

#if defined(HAVE_64BIT_LONG_LONG)

    /*
     * FNV-1 hash each octet of the buffer
     */
    while (bp < be) {

	/* multiply by the 64 bit FNV magic prime mod 2^64 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_64_PRIME;
#else /* NO_FNV_GCC_OPTIMIZATION */
	hval += (hval << 1) + (hval << 4) + (hval << 5) +
		(hval << 7) + (hval << 8) + (hval << 40);
#endif /* NO_FNV_GCC_OPTIMIZATION */

	/* xor the bottom with the current octet */
	hval ^= (Fnv64_t)*bp++;
    }

#else /* HAVE_64BIT_LONG_LONG */

    unsigned long val[4];			/* hash value in base 2^16 */
    unsigned long tmp[4];			/* tmp 64 bit value */

    /*
     * Convert Fnv64_t hval into a base 2^16 array
     */
    val[0] = hval.w32[0];
    val[1] = (val[0] >> 16);
    val[0] &= 0xffff;
    val[2] = hval.w32[1];
    val[3] = (val[2] >> 16);
    val[2] &= 0xffff;

    /*
     * FNV-1 hash each octet of the buffer
     */
    while (bp < be) {

	/*
	 * multiply by the 64 bit FNV magic prime mod 2^64
	 *
	 * Using 0x100000001b3 we have the following digits base 2^16:
	 *
	 *	0x0	0x100	0x0	0x1b3
	 *
	 * which is the same as:
	 *
	 *	0x0	1<<FNV_64_PRIME_SHIFT	0x0	FNV_64_PRIME_LOW
	 */
	/* multiply by the lowest order digit base 2^16 */
	tmp[0] = val[0] * FNV_64_PRIME_LOW;
	tmp[1] = val[1] * FNV_64_PRIME_LOW;
	tmp[2] = val[2] * FNV_64_PRIME_LOW;
	tmp[3] = val[3] * FNV_64_PRIME_LOW;
	/* multiply by the other non-zero digit */
	tmp[2] += val[0] << FNV_64_PRIME_SHIFT;	/* tmp[2] += val[0] * 0x100 */
	tmp[3] += val[1] << FNV_64_PRIME_SHIFT;	/* tmp[3] += val[1] * 0x100 */
	/* propagate carries */
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
	val[0] ^= (unsigned long)*bp++;
    }

    /*
     * Convert base 2^16 array back into an Fnv64_t
     */
    hval.w32[1] = ((val[3]<<16) | val[2]);
    hval.w32[0] = ((val[1]<<16) | val[0]);

#endif /* HAVE_64BIT_LONG_LONG */

    /* return our new hash value */
    return hval;
}


/*
 * fnv_64_str - perform a 64 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	hval	- previous hash value or 0 if first call
 *
 * returns:
 *	64 bit hash as a static hash type
 *
 * NOTE: To use the 64 bit FNV-0 historic hash, use FNV0_64_INIT as the hval
 *	 argument on the first call to either fnv_64_buf() or fnv_64_str().
 *
 * NOTE: To use the recommended 64 bit FNV-1 hash, use FNV1_64_INIT as the hval
 *	 argument on the first call to either fnv_64_buf() or fnv_64_str().
 */
Fnv64_t
fnv_64_str(char *str, Fnv64_t hval)
{
    unsigned char *s = (unsigned char *)str;	/* unsigned string */

#if defined(HAVE_64BIT_LONG_LONG)

    /*
     * FNV-1 hash each octet of the string
     */
    while (*s) {

	/* multiply by the 64 bit FNV magic prime mod 2^64 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_64_PRIME;
#else /* NO_FNV_GCC_OPTIMIZATION */
	hval += (hval << 1) + (hval << 4) + (hval << 5) +
		(hval << 7) + (hval << 8) + (hval << 40);
#endif /* NO_FNV_GCC_OPTIMIZATION */

	/* xor the bottom with the current octet */
	hval ^= (Fnv64_t)*s++;
    }

#else /* !HAVE_64BIT_LONG_LONG */

    unsigned long val[4];	/* hash value in base 2^16 */
    unsigned long tmp[4];	/* tmp 64 bit value */

    /*
     * Convert Fnv64_t hval into a base 2^16 array
     */
    val[0] = hval.w32[0];
    val[1] = (val[0] >> 16);
    val[0] &= 0xffff;
    val[2] = hval.w32[1];
    val[3] = (val[2] >> 16);
    val[2] &= 0xffff;

    /*
     * FNV-1 hash each octet of the string
     */
    while (*s) {

	/*
	 * multiply by the 64 bit FNV magic prime mod 2^64
	 *
	 * Using 1099511628211, we have the following digits base 2^16:
	 *
	 *	0x0	0x100	0x0	0x1b3
	 *
	 * which is the same as:
	 *
	 *	0x0	1<<FNV_64_PRIME_SHIFT	0x0	FNV_64_PRIME_LOW
	 */
	/* multiply by the lowest order digit base 2^16 */
	tmp[0] = val[0] * FNV_64_PRIME_LOW;
	tmp[1] = val[1] * FNV_64_PRIME_LOW;
	tmp[2] = val[2] * FNV_64_PRIME_LOW;
	tmp[3] = val[3] * FNV_64_PRIME_LOW;
	/* multiply by the other non-zero digit */
	tmp[2] += val[0] << FNV_64_PRIME_SHIFT;	/* tmp[2] += val[0] * 0x100 */
	tmp[3] += val[1] << FNV_64_PRIME_SHIFT;	/* tmp[3] += val[1] * 0x100 */
	/* propagate carries */
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
	val[0] ^= (unsigned long)(*s++);
    }

    /*
     * Convert base 2^16 array back into an Fnv64_t
     */
    hval.w32[1] = ((val[3]<<16) | val[2]);
    hval.w32[0] = ((val[1]<<16) | val[0]);

#endif /* !HAVE_64BIT_LONG_LONG */

    /* return our new hash value */
    return hval;
}
