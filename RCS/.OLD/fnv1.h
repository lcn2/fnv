/*
 * fnv - Fowler/Noll/Vo hash code
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
 * chongo_fnv@prime.engr.sgi.com
 *
 * Share and Enjoy!
 */

#if !defined(__FNV_H__)
#define __FNV_H__


/*
 * 32 bit hash value
 */
typedef unsigned long hash32;


/*
 * determine how 64 bit unsigned values are represented
 */
#include "longlong.h"


/*
 * 64 bit hash value
 */
#if defined(HAVE_64BIT_LONG_LONG)
typedef unsigned long long hash64;
#else
struct s_hash64 {
    unsigned long w32[2];
};
typedef struct s_hash64 hash64;
#endif


/*
 * external functions
 */
extern hash32 hash32_buf(char *buf, int len, hash32 *hval);
extern hash32 hash32_str(char *buf, hash32 *hval);
extern hash64 hash64_buf(char *buf, int len, hash64 *hval);
extern hash64 hash64_str(char *buf, hash64 *hval);


#endif /* __FNV_H__ */
