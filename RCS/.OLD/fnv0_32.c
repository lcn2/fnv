/*
 * fnv32 - 32 bit Fowler/Noll/Vo hash of a string or rile
 *
 * @(#) $Revision: 3.1 $
 * @(#) $Id: fnv32.c,v 3.1 1999/10/23 09:59:04 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/lib/libfnv/RCS/fnv32.c,v $
 *
 * usage:
 *	fnv32 [-s arg] [arg ...]
 *
 *	-s	hash arg as a string (ignoring terminating NUL bytes)
 *	arg	string (if -s was given) or filename (default stdin)
 *
 * See:
 *	http://reality.sgi.com/chongo/tech/comp/fnv/index.html
 *
 * for the most up to date copy of this code and the FNV hash home page.
 *
 * Copyright (C) 1997 Landon Curt Noll, all rights reserved.
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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fnv.h"

static char *usage = "usage: %s [-s arg] [arg ...]\n";
static char *program;	/* our name */


int
main(int argc, char *argv[])
{
    char buf[BUFSIZ+1];	/* read buffer */
    fnv32 hval;		/* current hash value */
    int s_flag = 0;	/* 1 => -s was given, hash args as strings */
    extern int optind;	/* argv index of the next argument to be processed */
    int fd;		/* open file to process */
    int i;

    /*
     * parse args
     */
    program = argv[0];
    while ((i = getopt(argc, argv, "s")) != -1) {
	switch (i) {
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

    /*
     * string hashing
     */
    if (s_flag) {

	/* hash the 1st string */
	hval = fnv32_str(argv[optind], NULL);

	/* hash any other strings */
	for (i=optind+1; i < argc; ++i) {
	    fnv32_str(argv[i], &hval);
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
	    hval = fnv32_fd(0, NULL);

	} else {

	    /* case: open, hash and close the 1st file */
	    fd = open(argv[optind], O_RDONLY);
	    if (fd < 0) {
		fprintf(stderr, "%s: unable to open file: %s\n",
			program, argv[optind]);
		exit(3);
	    }
	    hval = fnv32_fd(fd, NULL);
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
	    (void) fnv32_fd(fd, &hval);
	    close(fd);
	}
    }

    /*
     * report hash and exit
     */
    printf("0x%08lx\n", hval);
    return 0;	/* exit(0); */
}
