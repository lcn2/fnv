/*
 * fnv1 - Fowler/Noll/Vo-1 hash code
 *
 * @(#) $Revision: 3.4 $
 * @(#) $Id: fnv1.h,v 3.4 1999/10/27 01:53:29 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv1.h,v $
 *
 ***
 *
 * This is the FNV-1 algorithm with a non-0 offset basis which is very
 * similar to the historic FNV-0 algorithm and identical in speed.
 *
 ***
 *
 * Fowler/Noll/Vo-1 hash
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

#if !defined(__FNV1_H__)
#define __FNV1_H__


/*
 * 32 bit hash value
 */
typedef unsigned long fnv32;


/*
 * determine how 64 bit unsigned values are represented
 */
#include "longlong.h"


/*
 * 64 bit hash value
 */
#if defined(HAVE_64BIT_LONG_LONG)
typedef unsigned long long fnv64;
#else
struct s_fnv64 {
    unsigned long w32[2];
};
typedef struct s_fnv64 fnv64;
#endif


/*
 * external functions
 */
extern fnv32 fnv1_32_buf(char *buf, int len, fnv32 *hval);
extern fnv32 fnv1_32_str(char *buf, fnv32 *hval);
extern fnv32 fnv1_32_fd(int fd, fnv32 *hval);
extern fnv64 fnv1_64_buf(char *buf, int len, fnv64 *hval);
extern fnv64 fnv1_64_str(char *buf, fnv64 *hval);
extern fnv64 fnv1_64_fd(int fd, fnv64 *hval);


#endif /* __FNV1_H__ */
