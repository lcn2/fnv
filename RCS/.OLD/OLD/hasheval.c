/* $Id$  */
/*
 * hasheval - evaluate a hash system
 *
 * usage:
 *	hasheval [mult] < text
 *
 *	mult	the Fowler/Noll/Vo hash multiplier if FOWLER_NOLL_VO defined
 *
 * Hash defines:
 *
 *	FOWLER_NOLL_VO	  X <- ((X * 16777619) xor b[i]) mod 2^32
 *	FOWLER_VO1	  X <- (X * 987654321L + 123456879L + b[i]) mod 2^32
 *	FOWLER_VO2	  X <- (X * 0x63c63cd9 + 0x9c39c33d + b[i]) mod 2^32
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define CHUNK 65536		/* number of elements to malloc at a time */

unsigned long *hashval = NULL;		/* hash values processed so far */
long hashcnt = 0;		/* number of hash values in use */
long hashmax = 0;		/* malloced size of hashval */

#if defined(FOWLER_NOLL_VO)
long mult;			/* Vo/Fowler/Noll multiplier */
#endif /* FOWLER_NOLL_VO */

char alphabet[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'&-.";

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
#if defined(FOWLER_NOLL_VO)
    if (argc == 2) {
	mult = atol(argv[1]);
    } else {
	mult = 16777619;
    }
#endif /* FOWLER_NOLL_VO */

     /*
      * process words on stdin
      */
    while (fgets(buf, BUFSIZ, stdin)) {
	char *p;		/* start of word */
	char *q;		/* beyond end of word */
	char c;			/* char beyond end of word */
	int len;		/* length of word */

	/* firewall */
	buf[BUFSIZ] = '\0';

	/* process each word */
	for (p=buf; *p; p += strcspn(p, alphabet)) {

	    /* process the current word */
	    len = strspn(p, alphabet);
	    if (len > 0) {

		/* terminate word */
		q = p + len;
		c = *q;
		*q = '\0';

		/* hash this word string */
#if defined(DEBUG)
		printf("str: %s\n", p);
#endif
		hash_str(p);

		/* un-terminate word */
		*q = c;
		p = q;
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
#endif
#if defined(FOWLER_NOLL_VO)
    printf("Fowler/Noll/Vo %d hashed %d words with %d collision(s)\n", 
	mult, hashcnt, collide);
#elif defined(FOWLER_VO1)
    printf("Fowler/Vo I hashed %d words with %d collision(s)\n", 
	hashcnt, collide);
#elif defined(FOWLER_VO2)
    printf("Fowler/Vo II hashed %d words with %d collision(s)\n", 
	hashcnt, collide);
#else
    :%$&*&$%: did not define a hash :%$&*&$%:
#endif
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
    char *ostr = str;

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
#if defined(FOWLER_VO1)
	/* Fowler/Vo hash I */
	/* X <- (X * 987654321L + 123456879L + b[i]) mod 2^32 */
	val = val * 987654321L + 123456879L + c;
#endif
#if defined(FOWLER_NOLL_VO)
	/* Fowler/Noll/Vo hash */
	/* X <- ((X * 16777619) xor b[i]) mod 2^32 */
	val *= mult;
	val ^= c;
#endif
#if defined(FOWLER_VO2)
	/* Fowler/Vo hash II */
	/* X <- (X * 0x63c63cd9 + 0x9c39c33d + b[i]) mod 2^32 */
	val = val * 0x63c63cd9 + 0x9c39c33d + c;
#endif
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
