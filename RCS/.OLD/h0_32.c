/*
 * h0_32 - 32 bit Fowler/Noll/Vo-0 hash code
 *
 * @(#) $Revision: 3.9 $
 * @(#) $Id: h0_32.c,v 3.9 1999/10/27 05:36:12 chongo Exp chongo $
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

#include <stdlib.h>
#include "fnv0.h"


/*
 * FNV-0 defines the initial basis to be 0
 *
 * This is the original historic FNV algorithm with a 0 offset basis.
 * It is recommended that FNV-1, with a non-0 offset basis be used instead.
 */
const Fnv32_t fnv_32_init = (Fnv32_t)0;


/* 
 * 32 bit magic FNV-0 prime 
 */
#define FNV_32_PRIME ((Fnv32_t)0x01000193)	


/*
 * hash_buf - perform a 32 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 *	buf	- start of buffer to hash
 *	len	- length of buffer in octets
 *	hval	- previous hash value or 0 if first call
 *
 * returns:
 *	32 bit hash as a static hash type
 */
Fnv32_t
fnv0_32_buf(void *buf, size_t len, Fnv32_t hval)
{
    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

    /*
     * FNV-0 hash each octet in the buffer
     */
    while (bp < be) {

	/* multiply by FNV_32_PRIME mod 2^32 */
	hval *= FNV_32_PRIME;

	/* xor the bottom with the current octet */
	hval ^= (Fnv32_t)*bp++;
    }

    /* return our new hash value */
    return hval;
}


/*
 * hash_str - perform a 32 bit Fowler/Noll/Vo hash on a string
 *
 * input:
 *	str	- string to hash
 *	hval	- previous hash value or 0 if first call
 *
 * returns:
 *	32 bit hash as a static hash type
 */
Fnv32_t
fnv0_32_str(char *str, Fnv32_t hval)
{
    /*
     * FNV-0 hash each octet in the buffer
     */
    while (*str) {

	/* multiply by 16777619 mod 2^32 */
	hval *= FNV_32_PRIME;

	/* xor the bottom with the current octet */
	hval ^= (Fnv32_t)*str++;
    }

    /* return our new hash value */
    return hval;
}
