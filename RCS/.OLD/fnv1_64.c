/*
 * fnv1_64 - 64 bit Fowler/Noll/Vo-1 hash of a string or rile
 *
 * @(#) $Revision: 3.9 $
 * @(#) $Id: fnv1_64.c,v 3.9 1999/10/27 01:53:29 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv1_64.c,v $
 *
 ***
 *
 * This is the FNV-1 algorithm with a non-0 offset basis which is very
 * similar to the historic FNV-0 algorithm and identical in speed.
 *
 * usage:
 *	fnv164 [-b bcnt] [-m [-v]] [-s arg] [arg ...]
 *	fnv1_64 [-b bcnt] [-m [-v]] [-s arg] [arg ...]
 *
 *	-b bcnt	  mask off all but the lower bcnt bits (default: 32)
 *	-m	  multiple hashes, one per line for each arg
 *	-s	  hash arg as a string (ignoring terminating NUL bytes)
 *	-v	  verbose mode, print arg after hash (implies -m)
 *	arg	  string (if -s was given) or filename (default stdin)
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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fnv1.h"
#include "longlong.h"

#define WIDTH 64	/* bit width of hash */

static char *usage = "usage: %s [-b bcnt] [-s arg] [arg ...]\n";
static char *program;	/* our name */


/*
 * print_fnv - print an FNV hash
 *
 * given:
 *	hval	  the hash value to print
 *	mask	  lower bit mask
 *	verbose	  1 => print arg with hash
 *	arg	  string or filename arg
 */
static void
print_fnv(fnv64 hval, fnv64 mask, int verbose, char *arg)
{
#if defined(HAVE_64BIT_LONG_LONG)
    if (verbose) {
	printf("0x%016llx %s\n", hval & mask, arg);
    } else {
	printf("0x%016llx\n", hval & mask);
    }
#else
    if (verbose) {
	printf("0x%08lx%08lx %s\n",
	       hval.w32[1] & mask.w32[1],
	       hval.w32[0] & mask.w32[0],
	       arg);
    } else {
	printf("0x%08lx%08lx\n",
	       hval.w32[1] & mask.w32[1],
	       hval.w32[0] & mask.w32[0]);
    }
#endif
}


int
main(int argc, char *argv[])
{
    char buf[BUFSIZ+1];	/* read buffer */
    fnv64 hval;		/* current hash value */
    int s_flag = 0;	/* 1 => -s was given, hash args as strings */
    int m_flag = 0;	/* 1 => print multiple hashes, one per arg */
    int v_flag = 0;	/* 1 => verbose hash print */
    int b_flag = WIDTH;	/* -b flag value */
    fnv64 bmask;	/* mask to apply to output */
    extern int optind;	/* argv index of the next argument to be processed */
    int fd;		/* open file to process */
    int i;

    /*
     * parse args
     */
    program = argv[0];
    while ((i = getopt(argc, argv, "b:msv")) != -1) {
	switch (i) {
	case 'b':	/* bcnt bit mask count */
	    b_flag = atoi(optarg);
	    break;
	case 'm':	/* print multiple hashes, one per arg */
	    m_flag = 1;
	    break;
	case 's':	/* hash args as strings */
	    s_flag = 1;
	    break;
	case 'v':	/* verbose hash print */
	    m_flag = 1;
	    v_flag = 1;
	    break;
	default:
	    fprintf(stderr, usage, program);
	    exit(1);
	}
    }
    /* -s requires at least 1 arg */
    if (s_flag && optind >= argc) {
	fprintf(stderr, usage, program);
	exit(2);
    }
    /* limit -b values */
    if (b_flag < 0 || b_flag > WIDTH) {
	fprintf(stderr, "%s: -b bcnt: %d must be >= 0 and < %d\n",
		program, b_flag, WIDTH);
	exit(3);
    }
#if defined(HAVE_64BIT_LONG_LONG)
    if (b_flag == WIDTH) {
	bmask = (fnv64)0xffffffffffffffffULL;
    } else {
	bmask = (fnv64)((1ULL << b_flag) - 1ULL);
    }
#else
    if (b_flag == WIDTH) {
	bmask.w32[0] = 0xffffffffUL;
	bmask.w32[1] = 0xffffffffUL;
    } else if (b_flag >= WIDTH/2) {
	bmask.w32[0] = 0xffffffffUL;
	bmask.w32[1] = ((1UL << (b_flag-(WIDTH/2))) - 1UL);
    } else {
	bmask.w32[0] = ((1UL << b_flag) - 1UL);
	bmask.w32[1] = 0UL;
    }
#endif

    /*
     * string hashing
     */
    if (s_flag) {

	/* hash the 1st string */
	hval = fnv1_64_str(argv[optind], NULL);
	if (m_flag) {
	    print_fnv(hval, bmask, v_flag, argv[optind]);
	}

	/* hash any other strings */
	for (i=optind+1; i < argc; ++i) {
	    if (m_flag) {
		print_fnv(fnv1_64_str(argv[i], NULL), bmask, v_flag, argv[i]);
	    } else {
		fnv1_64_str(argv[i], &hval);
	    }
	}


    /*
     * file hashing
     */
    } else {

	/*
	 * process the first file
	 */
	if (optind >= argc) {

	    /* case: process only stdin */
	    hval = fnv1_64_fd(0, NULL);
	    if (m_flag) {
		print_fnv(hval, bmask, v_flag, "(stdin)");
	    }

	} else {

	    /* case: open, hash and close the 1st file */
	    fd = open(argv[optind], O_RDONLY);
	    if (fd < 0) {
		fprintf(stderr, "%s: unable to open file: %s\n",
			program, argv[optind]);
		exit(4);
	    }
	    if (m_flag) {
		print_fnv(fnv1_64_fd(fd, NULL), bmask, v_flag, argv[optind]);
	    } else {
		hval = fnv1_64_fd(fd, NULL);
	    }
	    close(fd);
	}

	/*
	 * process any other files
	 */
	for (i=optind+1; i < argc; ++i) {

	    /* open, hash and close the next file */
	    fd = open(argv[i], O_RDONLY);
	    if (fd < 0) {
		fprintf(stderr, "%s: unable to open file: %s\n",
			program, argv[i]);
		exit(4);
	    }
	    if (m_flag) {
		print_fnv(fnv1_64_fd(fd, NULL), bmask, v_flag, argv[i]);
	    } else {
		(void) fnv1_64_fd(fd, &hval);
	    }
	    close(fd);
	}
    }

    /*
     * report hash and exit
     */
    if (!m_flag) {
	print_fnv(hval, bmask, v_flag, "");
    }
    return 0;	/* exit(0); */
}
