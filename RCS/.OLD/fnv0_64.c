/*
 * fnv_64 - 64 bit Fowler/Noll/Vo hash of a string or rile
 *
 * @(#) $Revision: 3.5 $
 * @(#) $Id: fnv_64.c,v 3.5 1999/10/24 00:54:47 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv_64.c,v $
 *
 * usage:
 *	fnv64 [-b bcnt] [-m] [-s arg] [-v] [arg ...]
 *	fnv_64 [-b bcnt] [-m] [-s arg] [-v] [arg ...]
 *
 *	-b bcnt	  mask off all but the lower bcnt bits (default: 32)
 *	-m	  multiple hashes, one per line for each arg
 *	-s	  hash arg as a string (ignoring terminating NUL bytes)
 *	-v	  verbose mode, print arg after hash
 *	arg	  string (if -s was given) or filename (default stdin)
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
 * http://reality.sgi.com/chongo
 * chongo_fnv@prime.engr.sgi.com
 *
 * Share and Enjoy!	:-)
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fnv.h"
#include "longlong.h"

#define WIDTH 64	/* bit width of hash */

static char *usage = "usage: %s [-b bcnt] [-s arg] [arg ...]\n";
static char *program;	/* our name */


int
main(int argc, char *argv[])
{
    char buf[BUFSIZ+1];	/* read buffer */
    fnv64 hval;		/* current hash value */
    int s_flag = 0;	/* 1 => -s was given, hash args as strings */
    int b_flag = WIDTH;	/* -b flag value */
    fnv64 bmask;	/* mask to apply to output */
    extern int optind;	/* argv index of the next argument to be processed */
    int fd;		/* open file to process */
    int i;

    /*
     * parse args
     */
    program = argv[0];
    while ((i = getopt(argc, argv, "b:s")) != -1) {
	switch (i) {
	case 'b':	/* bcnt bit mask count */
	    b_flag = atoi(optarg);
	    break;
	case 's':	/* hash args as strings */
	    s_flag = 1;
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
	hval = fnv_64_str(argv[optind], NULL);

	/* hash any other strings */
	for (i=optind+1; i < argc; ++i) {
	    fnv_64_str(argv[i], &hval);
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
	    hval = fnv_64_fd(0, NULL);

	} else {

	    /* case: open, hash and close the 1st file */
	    fd = open(argv[optind], O_RDONLY);
	    if (fd < 0) {
		fprintf(stderr, "%s: unable to open file: %s\n",
			program, argv[optind]);
		exit(4);
	    }
	    hval = fnv_64_fd(fd, NULL);
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
		exit(5);
	    }
	    (void) fnv_64_fd(fd, &hval);
	    close(fd);
	}
    }

    /*
     * report hash and exit
     */
#if defined(HAVE_64BIT_LONG_LONG)
    printf("0x%016llx\n", hval & bmask);
#else
    printf("0x%08lx%08lx\n",
	   hval.w32[1] & bmask.w32[1], 
	   hval.w32[0] & bmask.w32[0]);
#endif
    return 0;	/* exit(0); */
}
