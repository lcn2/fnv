/* $Id$  */
/*
 * hasheval - evaluate a hash system
 *
 * usage:
 *	hasheval mult < text
 *
 *	mult	the Fowler/Noll/Vo hash multiplier   
 *
 * Try a mutliplier of 16777619 with /usr/share/dict/web2.
 */

#include <stdio.h>
#include <ctype.h>

#define CHUNK 65536		/* number of elements to malloc at a time */

unsigned long *hashval = NULL;		/* hash values processed so far */
long hashcnt = 0;		/* number of hash values in use */
long hashmax = 0;		/* malloced size of hashval */

long mult;			/* Vo/Fowler/Noll multiplier */

extern void *malloc();
extern void *realloc();

int hash_cmp();		/* compare two hash values */
void hash_str();	/* add hash of string to the hashval table */

main(argc, argv)
    int argc;		/* arg count */
    char *argv[];	/* the args */
{
    char buf[BUFSIZ+1];		/* input buffer */
    int collide;		/* number of collisions */
    int i;

     /*
      * parse args 
      */
    if (argc != 2) {
	fprintf(stderr, "usage: %s mult < web\n", argv[0]);
    }
    mult = atol(argv[1]);

     /*
      * process words on stdin
      */
    while (fgets(buf, BUFSIZ, stdin)) {
	char *p;
	char *q;

	/* firewall */
	buf[BUFSIZ] = '\0';

	/* process each word */
	for (p=buf, q=p; *p; ++q) {
	    char c;

	    /* non-alphanul terminates word */
	    if (*q == '\0' || !isascii(*q) || !isalnum(*q)) {
		
		/* terminate word */
		c = *q;
		*q = '\0';

		/* hash this word string */
		hash_str(p);

		/* find start of next word, if it exists */
		for (p=q; *p && (!isascii(*p) || !isalnum(*p)); ++p) {
		}
		q = p;
	    }
	}
    }

    /*
     * count colisions 
     */
    qsort((char *)hashval, hashcnt, sizeof(hashval[0]), hash_cmp);
    for (i=1, collide=0; i < hashcnt; ++i) {
#if defined(DEBUG)
	printf("%d: 0x%08x\n", i-1, hashval[i-1]);
#endif
	if (hashval[i-1] == hashval[i]) {
	    ++collide;
	}
    }
#if defined(DEBUG)
    printf("%d: 0x%08x\n", hashcnt-1, hashval[hashcnt-1]);
    printf("hashed %d words with %d collision(s)\n", hashcnt, collide);
#endif
    if (collide <= 2) {
	printf("Vo/Fowler/Noll for %d hashed %d words with %d collision(s)\n", 
	    mult, hashcnt, collide);
    }
    exit(0);
}

/*
 * hash_cmp - compate two hash values for qsort
 *
 * given:
 *	h1	first hash value
 *	h2	second hash value
 *
 * return:
 *	0	h1 == h2
 *	-1	h1 < h2
 *	1	h1 > h2
 */
int
hash_cmp(h1, h2)
    unsigned long *h1;		/* first hash value */
    unsigned long *h2;		/* second hash value */
{
    return ((*h1 < *h2) ? -1 : ((*h1 == *h2) ? 0 : 1));
}

/* 
 * hash_str - add hash of string to the hashval table 
 *
 * given:
 *	str	the string to hash
 *
 * Adds the hash value to the hashval table.  If needed, hash_val
 * will malloc or realloc hashval.
 */
void 
hash_str(str)
    register char *str;		/* the string to hash */
{
    register unsigned long val;		/* current hash value */
    register unsigned long c;		/* the current string character */

    /*
     * Fowler/Noll hash - hash each character in the string
     *
     * The basis of the hash algorithm was examined Glenn Fowler and
     * Landon Curt Noll (gsf@research.att.com) during the IEEE Posix 1989
     * Florida standards meeting.
     */
    for (val = 0; c=(unsigned long)(*str); ++str) {
#if 0
	/* Fowler/Noll hash */
	val ^= (val<<5) ^ c;
	val = (val & 0x1) ? ~val : val;
#endif
#if 0
	/* Fowler hash */
	val = ((val<<3) ^ (val>>2) ^ 1) + c;
	val = (val<0) ? ~val : val;
#endif
#if 0
	/* K. Phong Vo and Glenn Fowler hash */
	val = val * 987654321L + 123456879L + c;
#endif
	/* Fowler/Noll/Vo hash */
	val *= mult;
	val ^= c;
    }

    /*
     * malloc or realloc the hashval table if needed
     */
    if (hashval == NULL) {
	hashmax = CHUNK;
	hashval = (long *)malloc(sizeof(hashval[0]) * hashmax);
    } else if (hashcnt >= hashmax) {
	hashmax += CHUNK;
	hashval = (long *)realloc(hashval, sizeof(hashval[0]) * hashmax);
    }

    /*
     * add hash value to hashval
     */
    hashval[hashcnt++] = val;
    return;
}
