/*
 * have_ulong64 - Determine if we have a 64 bit unsigned long long
 *
 * usage:
 *	have_ulong64 > longlong.h
 *
 * Not all systems have a 'long long type' so this may not compile on
 * your system.
 *
 * This prog outputs the define:
 *
 *	HAVE_64BIT_LONG_LONG
 *		defined ==> we have a 64 bit unsigned long long
 *		undefined ==> we must simulate a 64 bit unsigned long long
 */
/*
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *	supporting documentation
 *	source copies
 *	source works derived from this source
 *	binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo <was here>	/\../\
 */

/*
 * have the compiler try its hand with unsigned and signed long longs
 */
unsigned long long val = 1099511628211ULL;

int
main(void)
{
	int longlong_bits;	/* bits in a long long */

	/*
	 * ensure that the length of long long val is what we expect
	 */
	if (val == 1099511628211ULL && sizeof(val) == 8) {
		printf("#define HAVE_64BIT_LONG_LONG\t/* yes */\n");
	}

	/* exit(0); */
	return 0;
}
