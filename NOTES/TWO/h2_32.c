/*
 * h2_32 - 32 bit Fowler/Noll/Vo-2 hash code
 *
 * @(#) $Revision: 3.9 $
 * @(#) $Id: h2_32.c,v 3.9 1999/10/29 07:39:03 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnv/TWO/RCS/h2_32.c,v $
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
static Fnv32_t virgin = 0x811c9dc5;


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
 *	Fnv32_t hash_value;
 *
 *	hash_value = fnv2_32_buf(buf, len, NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of the 'buf' buffer.
 *
 *	(void) fnv2_32_buf(buf2, len2, &hash_value);
 *
 *	    The 'hash_value' becomes the hash of buf concatenated with buf2.
 */
Fnv32_t
fnv2_32_buf(char *buf, int len, Fnv32_t *hval)
{
    Fnv32_t val;			/* current hash value */
    char *buf_end = buf+len;	/* beyond end of hash area */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * FNV-2 hash each octet in the buffer
     */
    while (buf < buf_end) {

	/* FNV-2 pre-increments on every octet */
	++val;

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= (Fnv32_t)16777619;

	/* xor the bottom with the current octet */
	val ^= (Fnv32_t)(*buf++);
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
 *	Fnv32_t hash_value;
 *
 *	hash_value = fnv2_32_str("the first string", NULL);
 *
 *	    The 'hash_value' becomes the FNV hash of "the first string"
 *	    not counting the final NUL byte.
 *
 *	(void) fnv2_32_str("2nd string", &hash_value);
 *
 *	    The 'hash_value' becomes the hash of "the first string2nd string"
 *	    not counting the final NUL byte.
 */
Fnv32_t
fnv2_32_str(char *str, Fnv32_t *hval)
{
    Fnv32_t val;			/* current hash value */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * FNV-2 hash each octet in the buffer
     */
    while (*str) {

	/* FNV-2 pre-increments on every octet */
	++val;

	/* multiply by 16777619 mod 2^32 using 32 bit longs */
	val *= (Fnv32_t)16777619;

	/* xor the bottom with the current octet */
	val ^= (Fnv32_t)(*str++);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}


/*
 * fnv2_32_fd - FNV hash an open filename
 *
 * usage:
 *      fd	- open file descriptor to hash
 *      hash    - hash value to modify or NULL => just return hash value
 *
 * return:
 *      32 bit hash as a static hash type
 */
Fnv32_t
fnv2_32_fd(int fd, Fnv32_t *hval)
{
    char buf[BUF_SIZE+1];	/* read buffer */
    int readcnt;		/* number of characters written */
    Fnv32_t val;			/* current hash value */

    /*
     * load or initialize hash value
     */
    val = (hval ? *hval : virgin);

    /*
     * hash until EOF
     */
    while ((readcnt = read(fd, buf, BUF_SIZE)) > 0) {
	(void) fnv2_32_buf(buf, readcnt, &val);
    }

    /* save the hash if we were given a non-NULL initial hash value */
    if (hval) {
	*hval = val;
    }

    /* return our new hash value */
    return val;
}
