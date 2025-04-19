/*
 * have_ulong64 - Determine if we have a 64 bit unsigned long long
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


/*
 * have the compiler try its hand with unsigned and signed long longs
 */
#if ! defined(NO64BIT_LONG_LONG)
unsigned long long val = 1099511628211ULL;
#endif /* NO64BIT_LONG_LONG */

int
main(void)
{
	/*
	 * ensure that the length of long long val is what we expect
	 */
#if defined(NO64BIT_LONG_LONG)
	printf("#undef HAVE_64BIT_LONG_LONG\t/* no */\n");
#else /* NO64BIT_LONG_LONG */
	if (val == 1099511628211ULL && sizeof(val) == 8) {
		printf("#define HAVE_64BIT_LONG_LONG\t/* yes */\n");
	}
#endif /* NO64BIT_LONG_LONG */

	/* exit(0); */
	return 0;
}
