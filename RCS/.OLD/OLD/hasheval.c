/* $Id$  */
/*
 * hasheval - evaluate a hash system
 *
 * usage:
 *	hasheval [mult] < text
 *
 *	mult	the Fowler/Noll/Vo hash multiplier if FOWLER_NOLL_VO defined
 *	mult	the Fowler/Noll/Vo64 hash multiplier if FOWLER_NOLL_VO64 defined
 *
 * Hash defines:
 *
 *	FOWLER_NOLL_VO64  X <- ((X * 1099511628211) xor b[i]) mod 2^64
 *	FOWLER_NOLL_VO	  X <- ((X * 16777619) xor b[i]) mod 2^32
 *	FOWLER_VO1	  X <- (X * 987654321L + 123456879L + b[i]) mod 2^32
 *	FOWLER_VO2	  X <- (X * 0x63c63cd9 + 0x9c39c33d + b[i]) mod 2^32
 */

#include <stdio.h>
#include <memory.h>
#include <ctype.h>

#define CHUNK 262144		/* number of elements to malloc at a time */

#if defined(FOWLER_NOLL_VO64)
#define HASH_SIZE 64			/* bit size fo hash */
typedef unsigned long long hash;
#else
#define HASH_SIZE 32			/* bit size fo hash */
typedef unsigned long hash;
#endif

hash *hashval = NULL;	/* hash values processed so far */
long hashcnt = 0;		/* number of hash values in use */
long hashmax = 0;		/* malloced size of hashval */

hash mult;			/* Fowler/Noll/Vo multiplier */

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
#if HASH_SIZE == 32
    if (argc == 2) {
	mult = atol(argv[1]);
    } else {
	mult = 16777619;
    }
#else
    if (argc == 2) {
	mult = atoll(argv[1]);
    } else {
	mult = 1099511628211ULL;
    }
#endif

     /*
      * process words on stdin
      */
    while (fgets(buf, BUFSIZ, stdin)) {

	/* process each line */
	buf[BUFSIZ] = '\0';
#if defined(DEBUG)
	printf("str: %s\n", buf);
#endif
	hash_str(buf);
    }

    /*
     * count colisions 
     */
    qsort((char *)hashval, hashcnt, sizeof(hashval[0]), hash_cmp);
    for (i=1, collide=0; i < hashcnt; ++i) {
#if defined(DEBUG)
#if HASH_SIZE == 32
	printf("%d: 0x%08x\n", i-1, hashval[i-1]);
#else
	printf("%d: 0x%016llx\n", i-1, hashval[i-1]);
#endif
#endif
	if (hashval[i-1] == hashval[i]) {
	    ++collide;
	}
    }
#if defined(FOWLER_NOLL_VO)
    printf("Fowler/Noll/Vo %d hashed %d words with %d collision(s)\n", 
	mult, hashcnt, collide);
#elif defined(FOWLER_NOLL_VO64)
    printf("Fowler/Noll/Vo64 %lld hashed %d words with %d collision(s)\n", 
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
    hash *h1;	/* first hash value */
    hash *h2;	/* second hash value */
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
    hash val;		/* current hash value */
    hash c;		/* the current string character */
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
#if defined(FOWLER_NOLL_VO) || defined(FOWLER_NOLL_VO64)
	/* Fowler/Noll/Vo or Fowler/Noll/Vo64 hash */
	/* X <- ((X * magic_prime) xor b[i]) mod 2^x */
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
	hashval = (hash *)malloc(sizeof(hashval[0]) * hashmax);
    } else if (hashcnt >= hashmax) {
	hashmax += CHUNK;
	hashval = (hash *)realloc(hashval, sizeof(hashval[0]) * hashmax);
    }

    /*
     * add hash value to hashval
     */
    hashval[hashcnt++] = val;
    return;
}
