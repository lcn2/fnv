/*
 * fnv_32 - 32 bit Fowler/Noll/Vo hash of a string or rile
 *
 * @(#) $Revision: 3.6 $
 * @(#) $Id: fnv_32.c,v 3.6 1999/10/24 01:08:54 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv_32.c,v $
 *
 * usage:
 *	fnv32 [-b bcnt] [-m [-v]] [-s arg] [arg ...]
 *	fnv_32 [-b bcnt] [-m [-v]] [-s arg] [arg ...]
 *
 *	-b bcnt	  mask off all but the lower bcnt bits (default: 32)
 *	-m	  multiple hashes, one per line for each arg
 *	-s	  hash arg as a string (ignoring terminating NUL bytes)
 *	-v	  verbose mode, print arg after hash (implies -m)
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

#define WIDTH 32	/* bit width of hash */

static char *usage = "usage: %s [-b bcnt] [-m [-v]] [-s arg] [arg ...]\n";
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
print_fnv(fnv32 hval, fnv32 mask, int verbose, char *arg)
{
    if (verbose) {
	printf("0x%08lx %s\n", hval & mask, arg);
    } else {
	printf("0x%08lx\n", hval & mask);
    }
}


/*
 * main - the main function
 *
 * see usage above for details.
 */
int
main(int argc, char *argv[])
{
    char buf[BUFSIZ+1];	/* read buffer */
    fnv32 hval;		/* current hash value */
    int s_flag = 0;	/* 1 => -s was given, hash args as strings */
    int m_flag = 0;	/* 1 => print multiple hashes, one per arg */
    int v_flag = 0;	/* 1 => verbose hash print */
    int b_flag = WIDTH;	/* -b flag value */
    fnv32 bmask;	/* mask to apply to output */
    extern int optind;	/* argv index of the next argument to be processed */
    extern char *optarg;/* option argument */
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
    if (b_flag == WIDTH) {
	bmask = (fnv32)0xffffffff;
    } else {
	bmask = (fnv32)((1 << b_flag) - 1);
    }

    /*
     * string hashing
     */
    if (s_flag) {

	/* hash the 1st string */
	hval = fnv_32_str(argv[optind], NULL);
	if (m_flag) {
	    print_fnv(hval, bmask, v_flag, argv[optind]);
	}

	/* hash any other strings */
	for (i=optind+1; i < argc; ++i) {
	    if (m_flag) {
		hval = fnv_32_str(argv[i], NULL);
		print_fnv(hval, bmask, v_flag, argv[i]);
	    } else {
		fnv_32_str(argv[i], &hval);
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
	    hval = fnv_32_fd(0, NULL);
	    if (m_flag) {
		print_fnv(hval, bmask, v_flag, "-");
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
		hval = fnv_32_fd(fd, NULL);
		print_fnv(hval, bmask, v_flag, argv[optind]);
	    } else {
		(void) fnv_32_fd(fd, &hval);
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
		hval = fnv_32_fd(fd, NULL);
		print_fnv(hval, bmask, v_flag, argv[i]);
	    } else {
		(void) fnv_32_fd(fd, &hval);
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
