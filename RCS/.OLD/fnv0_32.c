/*
 * fnv0_32 - 32 bit Fowler/Noll/Vo-0 hash of a string or rile
 *
 * @(#) $Revision: 3.11 $
 * @(#) $Id: fnv0_32.c,v 3.11 1999/10/27 05:36:12 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv0_32.c,v $
 *
 ***
 *
 * This is the original historic FNV algorithm with a 0 offset basis.
 * It is recommended that FNV-1 (with a non-0 offset basis) be used instead.
 *
 * usage:
 *	fnv032 [-b bcnt] [-m [-v]] [-s arg] [arg ...]
 *	fnv0_32 [-b bcnt] [-m [-v]] [-s arg] [arg ...]
 *
 *	-b bcnt	  mask off all but the lower bcnt bits (default: 32)
 *	-m	  multiple hashes, one per line for each arg
 *	-s	  hash arg as a string (ignoring terminating NUL bytes)
 *	-v	  verbose mode, print arg after hash (implies -m)
 *	arg	  string (if -s was given) or filename (default stdin)
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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fnv0.h"

#define WIDTH 32	/* bit width of hash */

#define BUF_SIZE (32*1024)	/* number of bytes to hash at a time */

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
print_fnv(Fnv32_t hval, Fnv32_t mask, int verbose, char *arg)
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
    char buf[BUF_SIZE+1];	/* read buffer */
    int readcnt;		/* number of characters written */
    Fnv32_t hval;	/* current hash value */
    int s_flag = 0;	/* 1 => -s was given, hash args as strings */
    int m_flag = 0;	/* 1 => print multiple hashes, one per arg */
    int v_flag = 0;	/* 1 => verbose hash print */
    int b_flag = WIDTH;	/* -b flag value */
    Fnv32_t bmask;	/* mask to apply to output */
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
	bmask = (Fnv32_t)0xffffffff;
    } else {
	bmask = (Fnv32_t)((1 << b_flag) - 1);
    }

    /* 
     * start with the FNV-0 initial basis
     */
    hval = fnv_32_init;

    /*
     * string hashing
     */
    if (s_flag) {

	/* hash any other strings */
	for (i=optind; i < argc; ++i) {
	    hval = fnv0_32_str(argv[i], hval);
	    if (m_flag) {
		print_fnv(hval, bmask, v_flag, argv[i]);
	    }
	}


    /*
     * file hashing
     */
    } else {

	/*
	 * case: process only stdin
	 */
	if (optind >= argc) {

	    /* case: process only stdin */
	    hval = fnv0_32_fd(0, hval);
	    if (m_flag) {
		print_fnv(hval, bmask, v_flag, "(stdin)");
	    }

	} else {

	    /*
	     * process any other files
	     */
	    for (i=optind; i < argc; ++i) {

		/* open the file */
		fd = open(argv[i], O_RDONLY);
		if (fd < 0) {
		    fprintf(stderr, "%s: unable to open file: %s\n",
			    program, argv[i]);
		    exit(4);
		}

		/*  hash the file */
		while ((readcnt = read(fd, buf, BUF_SIZE)) > 0) {
		    hval = fnv0_32_buf(buf, readcnt, hval);
		}

		/* finish processing the file */
		if (m_flag) {
		    print_fnv(hval, bmask, v_flag, argv[i]);
		}
		close(fd);
	    }
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
