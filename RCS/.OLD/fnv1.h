/*
 * fnv1 - Fowler/Noll/Vo-0 hash code
 *
 * @(#) $Revision: 3.6 $
 * @(#) $Id: fnv1.h,v 3.6 1999/10/29 00:42:35 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv1.h,v $
 *
 ***
 *
 * This is the original historic FNV-1 algorithm with a 0 offset basis.
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

#if !defined(__FNV1_H__)
#define __FNV1_H__


/*
 * 32 bit FNV-1 hash type
 */
typedef unsigned long Fnv32_t;


/*
 * 32 bit FNV-1 non-0 initial basis
 *
 * The FNV-1 initial basis is the FNV-0 hash of the following 32 octets:
 *
 *              chongo <Landon Curt Noll> /\../\
 *
 * Note that the \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 */
#define FNV_32_INIT ((Fnv32_t)0x811c9dc5)


/*
 * determine how 64 bit unsigned values are constructed
 */
#include "longlong.h"


/*
 * 64 bit FNV-1 hash type
 */
#if defined(HAVE_64BIT_LONG_LONG)
typedef unsigned long long Fnv64_t;
#else
typedef struct {
    unsigned long w32[2];
} Fnv64_t;
#endif


/*
 * 32 bit FNV-1 non-0 initial basis
 *
 * The FNV-1 initial basis is the FNV-0 hash of the following 32 octets:
 *
 *              chongo <Landon Curt Noll> /\../\
 *
 * Note that the \'s above are not back-slashing escape characters.
 * They are literal ASCII  backslash 0x5c characters.
 */
#if defined(HAVE_64BIT_LONG_LONG)
#define FNV_64_INIT ((Fnv64_t)0xcbf29ce484222325ULL)
#else
extern const Fnv64_t fnv_64_init;
#define FNV_64_INIT (fnv_64_init)
#endif


/*
 * external functions
 */
extern Fnv32_t fnv1_32_buf(void *buf, size_t len, Fnv32_t hval);
extern Fnv32_t fnv1_32_str(char *buf, Fnv32_t hval);
extern Fnv64_t fnv1_64_buf(void *buf, size_t len, Fnv64_t hval);
extern Fnv64_t fnv1_64_str(char *buf, Fnv64_t hval);


#endif /* __FNV1_H__ */
