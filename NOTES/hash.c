/*
 * hash - Fowler/Noll/Vo hash code library
 *
 * @(#) $Revision: 1.12 $
 * @(#) $Id: hash.c,v 1.12 1999/10/18 20:21:31 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnvhash/RCS/hash.c,v $
 *
 * NOTE: This code is just an example code for using the FNV hash functions.
 *	 The real fnv hash code is found in h32.c and h64.c.
 *
 * See:
 *	http://reality.sgi.com/chongo/fnv/index.html
 *
 * for the main FNV hash home page.
 *
 * Copyright (C) 1987, 1993 Ronald S. Karr and Landon Curt Noll
 * Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
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
 */

/*
 * This code was based on an early copy of the hash code that later 
 * went into Smail3.  Because this code was derived from pre-COPYLEFTed 
 * code, it is not subject to the Smail3 COPYLEFT.  The primary author, 
 * Landon Curt Noll, has attempted to preserve some of the spirit of sharing 
 * that the later Smail3 code had thur the above copyright.
 *
 * NOTE: This code may not be an exact replacement for the Smail3 hash code.
 */

/*
 * hash:
 *	perform a string hashing algorithm functions
 *
 * 	Hash tables are defined by their size.  It is suggested that the
 * 	size be a prime value, say:
 *	    13, 29, 47, 61, 113, 181, 251, 359, 509, 751, 1021, 1511,
 *	    2039, 3079, 4093, 6151, 8179, 12377, 16381, 24571, 32749,
 *	    49139, 65557, 98299, 132049, 180023, 216091, 321367, 521539, ...
 * 	An advantage with the above primes is that they are at last 3 less
 * 	than 2^n, and no closer than 3 away from 3*(2^m) for the larger 
 * 	values of m.  Most malloc systems are most efficient when one allocates
 * 	3 words less than 2^n bytes, i.e., 4*(2^n - 3) bytes.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "hash.h"

#if 0
extern void *malloc();
extern void free();
#endif

/* static functions used in this file */
static long hash_str();
#if !defined(NO_CASE_FOLDING)
static long hash_stric();
static int strcmpic();
#endif /* NO_CASE_FOLDING */
static struct hash *new_hash_element();
static void exit_internal(int level, char *name, char *fmt, ...);

#ifdef STANDALONE
static int debug = DBG_LVL;	/* default debug level */
static void dbug(char *name, char *fmt, ...);	/* print debug output */
#endif


#if !defined(NO_CASE_FOLDING)
#if !defined(SLOW_CASE_CONVERSION)
/*
 * ascii.c: fast ascii operations via direct table lookup
 *
 * convertion to upper or lower case
 */
/*
 * quick and dirty lowercase lookup for ASCII tolower()
 */
static unsigned char lowcase[256] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',0x5b,0x5c,0x5d,0x5e,0x5f,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};
#if 0	/* not needed for this code */
/*
 * quick and dirty uppercase lookup for ASCII toupper()
 */
static unsigned char upcase[256] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
0x60, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',0x7b,0x7c,0x7d,0x7e,0x7f,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};
#endif /* 0 */
#endif /* SLOW_CASE_CONVERSION */
#endif /* NO_CASE_FOLDING */


/*
 * hash_str - perform a 32 bit Fowler/Noll/Vo hash on a string
 *
 * input:
 *	str	- string to hash
 *	mod	- number of hash table entries
 * output:
 *	the slot number on which `str' belongs
 *
 * NOTE: mod should be a prime
 */
static long
hash_str(str, mod)
    register char *str;			/* the string to hash */
    long mod;				/* prime modulus, size of hash table */
{
    register unsigned long val;		/* current hash value */
    register unsigned long c;		/* the current string character */

    /* firewall - bogus case, but be safe anyway */
    if (str == NULL || mod <= 0) {
	return 0;
    }

    /*
     * Fowler/Noll/Vo hash - hash each character in the string
     *
     * The basis of the hash algorithm was taken from an idea
     * sent by Email to the IEEE Posix P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (chongo@toad.com) later improved on their
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * This hash was able to process 234936 words from the web2 dictionary
     * without any 32 bit collisions.
     */
    for (val = 0; c=(unsigned long)(*str); ++str) {
	val *= 16777619;
	val ^= c;
    }
    
    /* our hash value, mod the hash size */
    return (long)(val%(unsigned long)mod);
}


#if !defined(NO_CASE_FOLDING)
/*
 * hash_stric - perform a 32 bit Fowler/Noll/Vo hash on a string 
 *		without regard to case
 *
 * input:
 *	str	- string to hash without regard to case
 *	mod	- number of hash table entries
 * output:
 *	the slot number on which `str' belongs
 *
 * NOTE: mod should be a prime
 */
static long
hash_stric(str, mod)
    register char *str;		/* the string to hash disregarding case */
    long mod;			/* prime modulus, size of hash table */
{
    register long val;			/* current hash value */
    register unsigned long c;		/* the current string character */

    /* firewall - bogus case, but be safe anyway */
    if (str == NULL || mod <= 0) {
	return 0;
    }

    /*
     * Fowler/Noll/Vo hash - hash each character in the string
     *
     * The basis of the hash algorithm was taken from an idea
     * send by Email to the IEEE Posix P1003.2 mailing list from
     * Phong Vo (kpv@research.att.com) and Glenn Fowler (gsf@research.att.com).
     * Landon Curt Noll (chongo@ncd.com) later improved on there
     * algorithm to come up with Fowler/Noll/Vo hash.
     *
     * This hash was able to process 234936 from the web2 dictionary
     * without any 32 bit collisions.
     */
    for (val = 0; c=(unsigned long)(*str); ++str) {
	val *= 16777619;
	val ^= (unsigned long)lowcase[c];
    }
    
    /* our hash value, mod the hash size */
    return (long)(((unsigned long)val)%mod);    
}
#endif /* NO_CASE_FOLDING */


/*
 * walk_hash - walk thru a hash table
 *
 * returns NULL if there is no next element in the hash table
 *
 * input:
 *	cur	- our current location, or NULL to start at the beginning
 *	table	- the table to be walked
 * output:
 *	a pointer to the next element, or NULL if no next element
 *
 * WARNING: results will be unpredictable or fatal if `cur' != NULL, and
 *	    `cur' != `the previous walk location', and `cur' is an element
 *	    that has been either deleted or replaced by another element.
 *	    It should be noted that this `cur' will never be the `previous
 *	    walk location' if our previous call ran off the end of the table.
 */
struct hash *
walk_hash(cur, table)
    struct hash *cur;		/* where we are now */
    struct hash_table *table;	/* hash table being walked */
{
    char *_name="walk_hash";
    register struct hash *prev;	/* our previous walk location */
    register long indx;		/* our previous hash slot */
    register long len;		/* the table length */

    /*
     * firewall
     */
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "table is NULL");
    }
    /* fetch these values for faster use */
    prev = table->prev;
    indx = table->indx;
    len = table->len;

    /*
     * find the first hash slot if cur is NULL (due to a restart request)
     */
    if (cur == NULL) {
	/* note that we really don't have a location yet */
	prev = NULL;

	/* find the first slot and return it */
	for (indx=0; indx < len; ++indx) {
	    if (full_slot(table->slot[indx])) {
		/* we found the first entry of the first non-empty chain */
		prev = table->slot[indx];
		break;
	    }
	}

    /*
     * walk from our current location to the next location
     */
    } else {

	/*
	 * if `cur' is not the previous `cur', then find `cur' and
	 * note where our hash table index now resides
	 */
	if (cur != prev) {
	    /* find the hash table index */
	    indx = hash_string(cur->keystr, len, table->flags&HASH_STRCMP);
	    /* if `cur' is an empty slot, panic */
	    if (empty_slot(table->slot[indx])) {
		    exit_internal(HASH_MED, _name, 
		      "<%s> hash slot is empty", cur->keystr);
	    }
	
	    /* walk down the hash table chain looking for our entry */
	    for (prev = table->slot[indx];
		 cur != prev && prev != NULL;
		 prev = next_hash(prev,table)) {
	    }
	}
	/* if `cur' is not in the hash table, error */
	if (prev == NULL) {
	    exit_internal(HASH_MED, _name, 
	      "<%s> is not in table", cur->keystr);
	}

	/*
	 * if we were at the end of a chain, then our successor will
	 * be the start of the next non-empty chain
	 */
	if ((prev = next_hash(prev,table)) == NULL) {
	    /* find the next non-empty chain */
	    for (++indx; indx < len; ++indx) {
		if (full_slot(table->slot[indx])) {
		    /* return first element of this chain */
		    prev = table->slot[indx];
		    break;
	        }
	    }
	}
    }

    /*
     * return the pointer the next element or NULL
     */
    /* remember our location for next time */
    table->prev = hash_addr(prev, table);
    table->indx = indx;
    return table->prev;
}


/*
 * new_hash_element - creat a new hash element
 *
 * return a malloced new hash element with the lengths correctly filled out
 *
 * inputs:
 *	keystr	- the key of this data element
 *	data	- the data to accompany `keystr', or NULL if no data
 *	datalen	- the length of `data' in bytes, or 0 is no data
 *	table	- the hash table which will get this element
 */
static struct hash *
new_hash_element(keystr, data, datalen, table)
    char *keystr;    			/* the keystring to be added */
    char *data;				/* the associated data if any */
    long datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* hash table being added to */
{
    char *_name="new_hash_element";
    struct hash *new;		/* the new slot chain location */
    long keylen;		/* the length of the string, padded */
    long lk;			/* temp var for key length */

    /*
     * firewall - check for bad pointers and values
     */
    if (keystr == NULL || table == NULL) {
	exit_internal(HASH_MED, _name, 
	    "NULL keystring or table");
    }
    lk = keystr_len(keystr);		/* compute padded key length */
    if (lk >= (1L<<BITS_PER_SHORT)) {
	exit_internal(HASH_MED, _name, "key too long");
    }
    keylen = (long)lk;		/* now we know it will fit in an long */
    /* firewall - check against bad data being passed to us */
    if (datalen < 0 || (datalen > 0 && data == NULL) || 
      (long)datalen >= (1L<<BITS_PER_SHORT))  {
	exit_internal(HASH_MED, _name,
	    "bad data passes with: <%s>  datalen: <%d>",
	    keystr, datalen);
    }

    /*
     * malloc the storage
     */
    new = (struct hash *)malloc( hashslot_size(keylen,datalen) );
    /* firewall */
    if (is_odd(new)) {
	exit_internal(HASH_MED, _name,
	      "new_hash_element: malloc returned odd address: %lx",
	      (long)new);
    }

    /*
     * return the prebuild element
     */
    new->succ = NULL;
    new->keylen = keylen;
    strcpy(new->keystr, keystr);
    new->datalen = datalen;
    if (datalen > 0) {
	memcpy(hash_data(new), data, datalen);
    }
    return new;
}


/*
 * free_hash_element - free an old hash element
 *
 * inputs:
 *	cur	- the element which which we will free
 *	search	- non-zero means delete `cur' from table prior to the free
 *	table	- the table on which `cur' resides, or the table to which
 *		  `cur' would have been added
 *
 * WARNING: It is important that the `cur' element NOT be in a hash table
 *	    after the free.  Unpredictable results will happen otherwise.
 *	    If `search' is non-zero, we will first attempt to delete `cur'
 *	    from `table'.  It is a fatal error if `search' is non-zero and
 *	    `cur' is not in `table'.
 *
 * WARNING: It is important that `cur' was individually malloced (perhaps
 *	    by new_hash_element) so that the free of its address will
 *	    be valid.
 */
void
free_hash_element(cur, search, table)
    struct hash *cur;		/* what we will delete */
    int search;			/* non-zero => delete `cur' first */
    struct hash_table *table;	/* table `cur' does/would_have belonged to */
{
    char *_name="free_hash_element";

    /*
     * firewall - check for bad pointers and values
     */
    if (cur == NULL || table == NULL) {
	exit_internal(HASH_MED, _name, 
	  "NULL cur or table");
    }

    /*
     * delete the element first if requested
     */
    if (search != 0 && delete_from_hash(cur->keystr, table) == NULL) {
	exit_internal(HASH_MED, _name,
	  "<%s> not in table",cur->keystr);
    }

    /*
     * free the storage
     */
    free(cur);
    return;
}


/*
 * new_hash_table - creat a new hash table
 *
 * return a malloced new hash table with correctly setup initial pointers
 *
 * input:
 *	tablelen - number of slots in the hash table
 * output:
 *	a pointer to a malloced empty hash table
 */
struct hash_table *
new_hash_table(tablelen, flags)
    long tablelen;		/* number of slots in the hash table */
    int flags;
{
    char *_name="new_hash_table";
    register long i;			/* index */
    struct hash_table *table;		/* the malloced hash table */

    /*
     * firewalls
     */
    if (tablelen <= 0) {
	exit_internal(HASH_MED, _name, 
	  "tablelen: %d", tablelen);
    }
    DEBUG1(HASH_LOW, _name, "tablelen:%d\n",tablelen);

    /*
     * malloc the hash table
     */
    table = (struct hash_table *)malloc(table_size(tablelen));
    /* firewall */
    if (is_odd(table)) {
	exit_internal(HASH_MED, _name,
	    "new_hash_table: malloc returned odd address: %d", (long)table);
    }

    /*
     * initialize the table
     */
    table->len = tablelen;
    table->flags = flags & HASH_FLAGMASK;
    table->prev = NULL;		/* no current walk_hash() location */
    table->indx = 0;		/* no current walk_hash() slot index */
    for (i=0; i < tablelen; i++) {
	table->slot[i] = NULL;
    }
    return table;	/* return our new table */
}


/*
 * free_hash_table - free a hash table and its associated data
 *
 * Free all storage associated with a hash table.
 *
 * NOTE: any malloced elements should be freed prior to calling this routine.
 *
 * input:
 *	table	- the hash table to free
 */
void
free_hash_table(table)
    struct hash_table *table;	/* the hash table to free */
{
    char *_name="free_hash_table";

    /*
     * firewalls
     */
    if (table == NULL ) {
	exit_internal(HASH_MED, _name, "NULL table");
    }
    DEBUG(HASH_LOW, _name,"start\n");

    /*
     * free the table slots
     */
    free(table);
    return;
}


/*
 * add_to_hash - add an element to the a hash table
 *
 * inputs:
 *	keystr	- the key of the data to add
 *	data	- the data to accompany `keystr', or NULL if no data
 *	datalen	- the length of `data' in bytes, or 0 if no data
 *	table	- a pointer to the hash table which is being modified
 * output:
 *	returns ALREADY_HASHED if `keystr' is already in the `table', or
 *	JUST_HASHED if we just added a no key.  The `table' is not modified
 *	if the key is already exists.
 */
int
add_to_hash(keystr, data, datalen, table)
    char *keystr;    			/* the keystring to be added */
    char *data;				/* the associated data if any */
    long datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="add_to_hash";
    register struct hash *cur;		/* the current slot chain location */
    register struct hash *prev;		/* the previous slot chain location */
    register int cmp;			/* -1, 0, or 1 for < = > compare */
    register int caseflag;		/* 0 ==> use strcmp, 1 ==> strcmpic */
    long loc;				/* the hash slot to add onto */
    struct hash *new;			/* the new slot chain location */

    /*
     * firewall - watch for NULLs
     */
    if (keystr == NULL) {
	exit_internal(HASH_MED, _name, "NULL keystr");
    }
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "NULL table");
    }

    /*
     * determine the slot on which this entry is to be added
     */
    caseflag = table->flags & HASH_STRCMP;
    loc = hash_string(keystr, table->len, caseflag);
    DEBUG2(HASH_LOW, _name, "add_to_hash: keystr: <%s> slot: %d\n", 
      keystr, loc);

    /*
     * search the slot chain for our entry
     */
    /* special case for empty slot chains */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "insert in NULL slot\n");
	new = new_hash_element(keystr, data, datalen, table);
	insert_hash(&table->slot[loc], new);
	return JUST_HASHED;
    }

    /* 
     * search the chain
     */
    DEBUG2(HASH_VHIGH, _name, "slot:0x%lx cur:0x%lx\n",
	   (long)table->slot[loc], (long)table->slot[loc]);
    for (prev=NULL, cur=table->slot[loc];
	 cur != NULL; prev=cur, cur=next_hash(cur, table)) {
	/* 
	 * if we found the entry, stop
	 */
	DEBUG2(HASH_VHIGH, _name, "comparing <%s> to <%s>",
	       keystr, cur->keystr);
	if ((cmp = stringcmp(keystr, cur->keystr, caseflag)) == 0) {
	    DEBUG(HASH_MED, _name, "add_to_hash: already hashed\n");
	    return ALREADY_HASHED;

	/* 
	 * we are past the insertion point, insert before here and stop
	 * note if we are inserting at the beginning a a chain or in the middle
	 */
	} else if (cmp < 0) {
	    new = new_hash_element(keystr, data, datalen, table);
	    if (prev == NULL) {
		DEBUG(HASH_MED, _name, "insert at front\n");
		insert_hash(&table->slot[loc], new);  /* insert at beginning */
	    } else {
		DEBUG(HASH_MED, _name, "insert in middle\n");
		insert_hash(prev, new);	/* insert in middle */
	    }
	    return JUST_HASHED;
	}
    }

    /* 
     * case: insertion at the end of the chain
     */
    DEBUG(HASH_MED, _name, "insert at END\n");
    new = new_hash_element(keystr, data, datalen, table);
    insert_hash(prev, new);
    return JUST_HASHED;
}


/*
 * replace_in_hash - replace an existing element in a hash table
 *
 * inputs:
 *	keystr	- the key of the data to replace
 *	data	- the data to accompany `keystr', or NULL if no data
 *	datalen	- the length of `data' in bytes, or 0 if no data
 *	table	- a pointer to the hash table which is being modified
 * output:
 *	returns a pointer to the element that was replaced, or NULL
 *	if no element was replaced due to `keystr' not in `table'
 */
struct hash *
replace_in_hash(keystr, data, datalen, table)
    char *keystr;    			/* the keystring to replace */
    char *data;				/* the associated data if any */
    long datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="replace_in_hash";
    register struct hash *cur;		/* the current slot chain location */
    register struct hash *prev;		/* the previous slot chain location */
    register int cmp;			/* -1, 0, or 1 for < = > compare */
    register int caseflag;		/* 0 ==> use strcmp, 1 ==> strcmpic */
    long loc;				/* the hash slot to add onto */
    struct hash *new;			/* the new slot chain location */

    /*
     * firewall - watch for NULLs
     */
    if (keystr == NULL) {
	exit_internal(HASH_MED, _name, "NULL keystr");
    }
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "NULL table");
    }

    /*
     * determine the slot on which this entry is to be added
     */
    caseflag = table->flags & HASH_STRCMP;
    loc = hash_string(keystr, table->len, caseflag);
    DEBUG2(HASH_LOW, _name, 
      "replace_in_hash: keystr: <%s> slot: %d\n",keystr,loc);

    /*
     * search the slot chain for our entry
     */
    /* special case for empty slow chains */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "slot NULL\n");
	return NULL;	/* no entry to replace */
    }

    /* 
     * search the chain
     */
    for (prev=NULL, cur=table->slot[loc]; cur != NULL; 
      prev=cur, cur=next_hash(cur, table)) {
	/* 
	 * if we found the entry, stop
	 */
	if ((cmp = stringcmp(keystr, cur->keystr, caseflag)) == 0) {
	    new = new_hash_element(keystr, data, datalen, table);
	    if (prev == NULL) {
		DEBUG(HASH_MED, _name, "replaced at front\n");
		/* insert at beginning */
		replace_hash(table->slot[loc], cur, new);
	    } else {
		DEBUG(HASH_MED, _name, "replaced in middle\n");
		replace_hash(prev->succ, cur, new);	/* insert in middle */
	    }
	    return cur;
	/* if we have gone past our entry, stop searching */
        } else if (cmp < 0) {
	    break;
	}
    }

    /* 
     * entry not found, nothing to replace
     */
    DEBUG(HASH_MED, _name, "not found\n");
    return NULL;
}


/*
 * store_in_hash - store an existing element in a hash table
 *
 * inputs:
 *	keystr	- the key of the data to store
 *	data	- the data to accompany `keystr', or NULL if no data
 *	datalen	- the length of `data' in bytes, or 0 if no data
 *	table	- a pointer to the hash table which is being modified
 * output:
 *	returns a pointer to the element that was replaced, or NULL
 *	if no element was replaced.  In any case the element is added.
 */
struct hash *
store_in_hash(keystr, data, datalen, table)
    char *keystr;    			/* the keystring to replace */
    char *data;				/* the associated data if any */
    long datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="store_in_hash";
    register struct hash *cur;		/* the current slot chain location */
    register struct hash *prev;		/* the previous slot chain location */
    register int cmp;			/* -1, 0, or 1 for < = > compare */
    register int caseflag;		/* 0 ==> use strcmp, 1 ==> strcmpic */
    long loc;				/* the hash slot to add onto */
    struct hash *new;			/* the new slot chain location */

    /*
     * firewall - watch for NULLs
     */
    if (keystr == NULL) {
	exit_internal(HASH_MED, _name, "NULL keystr");
    }
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "NULL table");
    }

    /*
     * determine the slot on which this entry is to be added
     */
    caseflag = table->flags & HASH_STRCMP;
    loc = hash_string(keystr, table->len, caseflag);
    DEBUG2(HASH_LOW, _name,
      "store_in_hash: keystr: <%s> loc: %d\n", keystr, loc);

    /*
     * search the slot chain for our entry
     */
    /* special case for empty slow chains */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "insert on NULL slot\n");
	new = new_hash_element(keystr, data, datalen, table);
	insert_hash(&table->slot[loc], new);
	return NULL;
    }

    /* 
     * search the chain
     */
    for (prev=NULL, cur=table->slot[loc]; cur != NULL; 
      prev=cur, cur=next_hash(cur, table)) {
	/* 
	 * if we found the entry, stop
	 */
	if ((cmp = stringcmp(keystr, cur->keystr, caseflag)) == 0) {
	    new = new_hash_element(keystr, data, datalen, table);
	    if (prev == NULL) {
		DEBUG(HASH_MED, _name, "replaced at front\n");
		/* insert at beginning */
		replace_hash(table->slot[loc], cur, new);
	    } else {
		DEBUG(HASH_MED, _name, "replaced in middle\n");
		replace_hash(prev->succ, cur, new);	/* insert in middle */
	    }
	    return cur;

	/* 
	 * we are past the insertion point, insert before here and stop
	 * note if we are inserting at the beginning a a chain or in the middle
	 */
	} else if (cmp < 0) {
	    new = new_hash_element(keystr, data, datalen, table);
	    if (prev == NULL) {
		DEBUG(HASH_MED, _name, "insert at front\n");
		insert_hash(&table->slot[loc], new);  /* insert at beginning */
	    } else {
		DEBUG(HASH_MED, _name, "insert in middle\n");
		insert_hash(&prev->succ, new);	/* insert in middle */
	    }
	    return NULL;
	}
    }

    /* 
     * case: insertion at the end of the chain
     */
    DEBUG(HASH_MED, _name, "insert at END\n");
    new = new_hash_element(keystr, data, datalen, table);
    insert_hash(&prev->succ, new);
    return NULL;
}


/*
 * lookup_in_hash - lookup an element in a hash table and return 
 *		    the associated data
 *
 * inputs:
 *	keystr	- the key of the data to add
 *	data	- pointer to a pointer, or 0 if no data is wanted
 *	datalen	- pointer to the data length, or NULL if no length is wanted
 *	table	- a pointer to the hash table which is being modified
 * output:
 *	returns ALREADY_HASHED if `keystr' is already in the `table', or
 *		NOT_HASHED the key was not found
 *	data	- if `data' was non-NULL points at the key's data or NULL
 *		  no data
 *	datalen	- if `datalen' was non-NULL, points at the length of the
 *		  key's data
 */
int
lookup_in_hash(keystr, data, datalen, table)
    char *keystr;			/* the key to lookup */
    char **data;			/* where to point at data, or NULL */
    long *datalen;			/* where to place data len or NULL */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="lookup_in_hash";
    register struct hash *cur;	/* the slot chain location */
    register int caseflag;	/* 0 ==> use strcmp, 1 ==> strcmpic */
    long loc;			/* the hash slot to add onto */
    int cmp;			/* compare function result */

    /*
     * firewall - watch for NULLs
     */
    if (keystr == NULL) {
	exit_internal(HASH_MED, _name, "NULL keystr");
    }
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "table is NULL");
    }

    /*
     * determine the hash slot to search on
     */
    caseflag = table->flags & HASH_STRCMP;
    loc = hash_string(keystr, table->len, caseflag);
    DEBUG2(HASH_LOW, _name, 
      "lookup_in_hash: keystr: <%s> slot: %d\n",keystr,loc);
    /* watch out for empty chains, there is nothing on them */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "found at slot END\n");
	return NOT_HASHED;
    }

    /* 
     * search the chain
     */
    for (cur=table->slot[loc]; cur != NULL; cur=next_hash(cur, table)) {
	/*
	 * if we found the entry, stop
	 */
	if ((cmp = stringcmp(keystr, cur->keystr, caseflag)) == 0) {
	    DEBUG(HASH_MED, _name, "lookup_in_hash: found\n");
	    /*
	     * fill in the requested args
	     */
	    if (data != NULL) {
		*data = hash_data(cur);	
	    }
	    if (datalen != NULL) {
		*datalen = cur->datalen;
	    }
	    return ALREADY_HASHED;
	/* if we have gone past our entry, stop searching */
        } else if (cmp < 0) {
	    break;
	}
    }

    /* found nothing */
    DEBUG(HASH_MED, _name, "not found\n");
    return NOT_HASHED;
}


/*
 * delete_from_hash - delete an element in the hash table
 *
 * inputs:
 *	keystr	- the key of the data to add
 *	table	- a pointer to the hash table which is being modified
 * output:
 *	returns a pointer to the element that was deleted, or NULL
 *	if no element was deleted due to `keystr' not in `table'
 */
struct hash *
delete_from_hash(keystr, table)
    char *keystr;			/* the key to lookup */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="delete_from_hash";
    register struct hash *cur;		/* the slot chain location */
    register struct hash *prev;		/* previous element */
    register int caseflag;		/* 0 ==> use strcmp, 1 ==> strcmpic */
    long loc;			/* the hash slot to add onto */
    int cmp;			/* compare function result */

    /*
     * firewall - watch for NULLs
     */
    if (keystr == NULL) {
	exit_internal(HASH_MED, _name, "keystr is NULL");
    }
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "table is NULL");
    }

    /*
     * determine the hash slot to search on
     */
    caseflag = table->flags & HASH_STRCMP;
    loc = hash_string(keystr, table->len, caseflag);
    DEBUG2(HASH_LOW, _name, 
      "delete_from_hash: keystr: <%s> loc: %d\n",keystr,loc);
    /* watch out for empty chains, there is nothing on them */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "EMPTY slot\n");
	return NULL;	/* key is not in the table */
    }

    /* 
     * search the chain for the element to delete
     */
    for (prev=NULL, cur=table->slot[loc]; cur != NULL; 
      prev=cur, cur=next_hash(cur, table)) {
	/* 
	 * if we found the entry, stop
	 */
	if ((cmp = stringcmp(keystr, cur->keystr, caseflag)) == 0) {
	    if (prev == NULL) {
		DEBUG(HASH_MED, _name, "delete at front\n");
		/* delete at the beginning */
		delete_hash(&table->slot[loc], cur);
	    } else {
		DEBUG(HASH_MED, _name, "delete in middle\n");
		delete_hash(&prev->succ, cur);	/* delete in middle */
	    }
	    return cur;
	/*
	 * if we have gone past the entry, stop looking
	 */
        } else if (cmp < 0) {
	    DEBUG(HASH_MED, _name, "past spot\n");
	    break;
        }
    }

    /* found nothing */
    DEBUG(HASH_MED, _name, "not found\n");
    return NULL;		/* nothing was deleted */
}


#if !defined(MEM_ONLY)
/*
 * write_hash_table - write a hash table to a stream
 *
 * input:
 *	tab	- the hash table to write
 *	stream	- the stream on which to write
 */
void
write_hash_table(table, stream)
    struct hash_table *table;	/* the table to write */
    FILE *stream;		/* the stream on which to write table */
{
    char *_name="write_hash_table";
    register long i;		/* index */
    long tab_loc;		/* file location of hash_table start */
    long loc;			/* current location */
    long size;			/* size of the current element */
    long offset;                /* current offset within the file */
    long disc_succ;             /* cur->succ as written to disc */
    struct hash *cur;		/* the current hash element */
    struct hash_table *tab;	/* disk copy of table */

    /*
     * firewalls
     */
    if (table == NULL || stream == NULL) {
	exit_internal(HASH_MED, _name, "write_hash_table: NULL arguments");    
    }
    if (table->len <= 0) {
	exit_internal(HASH_MED, _name, 
	  "write_hash_table: table length: %d", table->len);
    }
    DEBUG1(HASH_LOW, _name, "write_hash_table: size: %d\n", table->len);

    /*
     * allocate a temporary disk copy of the hash table
     */
    tab = (struct hash_table *)malloc(table_size(table->len));
    tab->len = table->len;		/* preserve table length */
    tab->flags = table->flags;		/* preserve flags - XXX all bits ??? */
    tab->prev = NULL;			/* clear walking location */
    tab->indx = 0;			/* clear walking slot index */

    /*
     * skip the hash table block
     */
    tab_loc = ftell(stream);
    if (fseek(stream, (long)table_size(table->len), 1) < 0) {
	exit_internal(HASH_MED, _name, 
	  "write_hash_table: bad skip of %d over table",
	    table_size(table->len));
    }

    /*
     * write out each hash table chain
     */
    for (i=0; i < table->len; i++) {

	/* don't write out chains that don't exist */
	if (empty_slot(table->slot[i])) {
		/* slot is empty, deal with it quickly */
		set_ptr(tab->slot[i], (long)NULL);
		continue;
	}

	/* note starting offset of hash chain */
	offset = ftell(stream) - tab_loc;
	if (is_odd(offset)) {
		exit_internal(HASH_MED, _name, 
		  "write_hash_table: slot %d offset is odd:%ld",
		      i, offset);
	}
	set_ptr(tab->slot[i], to_odd(offset));

	/* write up to the last chain element */
	for (cur=table->slot[i]; cur != NULL; cur=next_hash(cur, table)) {
	    /* compute the current element length */
	    size = hash_len(cur);
	    if (is_odd(size)) {
		    exit_internal(HASH_MED, _name,
			  "write_hash_table: size is odd:%ld for <%s>",
			  offset, cur->keystr);
	    }
	    offset += size;		/* note file movement */
	    /* write the hash element disk successor element */
	    disc_succ = (cur->succ == NULL) ? 0 : to_odd(offset);
	    if (fwrite((char *)&disc_succ, sizeof(disc_succ), 1, stream) != 1) {
		exit_internal(HASH_MED, _name,
		      "write_hash_table: bad succ write <%s> on slot %d",
		      cur->keystr, i);
	    }
	    /* write the rest of the hash element data */
	    if (fwrite((char *)cur+sizeof(cur->succ), size-sizeof(cur->succ),
		       1, stream) != 1) {
		exit_internal(HASH_MED, _name,
		      "write_hash_table: bad write <%s> on slot %d",
		      cur->keystr, i);
	    }
	}
    }

    /*
     * write the hash table back in its place and return to our
     * current position
     */
    loc = ftell(stream);	/* remember our current location */
    if (fseek(stream, (long)tab_loc, 0) < 0) {
	exit_internal(HASH_MED, _name, 
	    "write_hash_table: bad skip back to table at %d",
	    tab_loc);
    }
    if (fwrite((char *)tab, table_size(tab->len), 1, stream) != 1) {
	exit_internal(HASH_MED, _name, "write_hash_table: bad table write");
    }
    if (fseek(stream, (long)loc, 0) < 0) {
	exit_internal(HASH_MED, _name, 
	    "write_hash_table: bad end seek to %d", loc);
    }

    /*
     * free the temporary disk copy of the hash table
     */
    free((char *)tab);
    return;
}
#endif /* MEM_ONLY */


#if !defined(NO_CASE_FOLDING)
/*
 * strcmpic - case independent strcmp function
 */
static int
strcmpic(s1, s2)
    register char *s1, *s2;		/* strings to be compared */
{
    register int c1, c2;		/* temp */

    while (*s1 && *s2) {
#if defined(SLOW_CASE_CONVERSION)
	if (isupper(c1 = *s1++)) {
		c1 = tolower(c1);
	}
	if (isupper(c2 = *s2++)) {
		c2 = tolower(c2);
	}
	if (c1 != c2) {
		return c1-c2;		/* strings are not equal */
	}
#else /* SLOW_CASE_CONVERSION */
	if ((c1 = (int)tolower(*s1++)) != (c2 = (int)tolower(*s2++))) {
	    return c1-c2;
	}
#endif /* SLOW_CASE_CONVERSION */
    }

    /*
     * one or both chars must be `\0'.  If only one is `\0', then
     * the other string is longer.
     */
    return (int)((*s1)-(*s2));
}
#endif /* NO_CASE_FOLDING */


/*
 * find_hash - find an element in a hash table and return a
 *		    pointer to its associated data
 *
 * inputs:
 *	key	- the key of the data to lookup
 *	datalen - pointer to length of data found or NULL
 *	table	- a pointer to the hash table which is being modified
 * output:
 *	returns pointer to data, or NULL if not found
 *	        if datalen is non-NULL, returns length of data
 */
char *
find_hash(key, datalen, table)
    char *key;			/* the key to lookup */
    long *datalen;		/* length of data,  NULL ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="find_hash";
    register struct hash *cur;	/* the slot chain location */
    long loc;			/* the hash slot to add onto */
    int cmp;			/* compare function result */

    /*
     * firewall - watch for NULLs
     */
    if (key == NULL) {
	exit_internal(HASH_MED, _name, "NULL key");
    }
    if (table == NULL) {
	exit_internal(HASH_MED, _name, "table is NULL");
    }

    /*
     * determine the hash slot to search on
     */
    loc = hash_str(key, table->len);
    DEBUG2(HASH_LOW, _name, "key: <%s> slot: %d\n",
      key, loc);
    /* watch out for empty chains, there is nothing on them */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "found at slot END\n");
	if (datalen != NULL) {
	    *datalen = 0;
	}
	return NULL;
    }

    /* 
     * search the chain
     */
    for (cur=table->slot[loc]; cur != NULL; cur=next_hash(cur,table)) {
	/*
	 * if we found the entry, stop
	 */
	if ((cmp = strcmp(key, cur->keystr)) == 0) {
	    DEBUG(HASH_MED, _name, "found\n");
	    if (datalen != NULL) {
		*datalen = cur->datalen;
	    }
	    return hash_data(cur);	
	/* if we have gone past our entry, stop searching */
        } else if (cmp < 0) {
	    break;
	}
    }

    /* found nothing */
    DEBUG(HASH_MED, _name, "not found\n");
    if (datalen != NULL) {
	*datalen = 0;
    }
    return NULL;
}


/*VARARGS2*/
/*
 * level	error exit level
 * name		routine name
 * fmt		printf(3) format
 * ...		arguments for printf
 */
static void
exit_internal(int level, char *name, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)printf("INTERNAL: level=%d %s: ", level, name); 
    (void)vprintf(fmt, ap);
    putchar('\n');			/* fatal messages not \n terminated */
    va_end(ap);
    fflush(stdout);

    exit(1);
}


#ifdef STANDALONE

#include <sys/types.h>
#include <sys/stat.h>

#define TABLE_LEN 3 /* use only a few slots to stress the chain code */
#define INPUT_SIZE (70*1024)	/* max input line */

char *tempname = "/tmp/hashtestXXXXXX";	/* tempory hash file name */

void
main(argc, argv)
    int argc;	/* arg count */
    char *argv[]; 	/* args */
{
    char *_name="main";
    long i;			/* index */
    char buf[INPUT_SIZE+1];	/* the input buffer for stdin args */
    struct hash *cur;		/* pointer to walk the table */
    struct hash_table *table;	/* our allocated table */
#if !defined(NO_CASE_FOLDING)
    struct hash_table *tableic;	/* allocated table without regard to case */
#endif /* NO_CASE_FOLDING */
    void test();		/* test an with an element */
    void dump_hash_table();	/* dump the contents of the hash table */
#if !defined(MEM_ONLY)
    void hash_file_test();	/* perform hash file testing */
#endif /* MEM_ONLY */

    /*
     * establish debug level
     */
    DEBUG(HASH_LOW, _name, "main: start\n");
    if (argc > 1 && strncmp(argv[1], "-d", 2) == 0) {
	    /* we have a debug level, use it */
	    if (argv[1][2] != '\0') {
		    debug = atoi(&argv[1][2]);
		    --argc;
		    ++argv;
	    } else if (argc > 2) {
		    debug = atoi(argv[2]);
		    argc -= 2;
		    argv += 2;
	    }
    }
    DEBUG1(HASH_LOW, _name, "debug level: %d\n", debug);

    /*
     * setup a hash table
     */
    table = new_hash_table(TABLE_LEN, HASH_STRCMP);
#if !defined(NO_CASE_FOLDING)
    tableic = new_hash_table(TABLE_LEN, 0);
#endif /* NO_CASE_FOLDING */

    /*
     * special case: no args means read one arg per line
     */
    if (argc == 1) {
	while(fgets(buf, INPUT_SIZE, stdin) != NULL) {
	    i = strlen(buf);
	    buf[i-1] = '\0';
	    DEBUG1(HASH_LOW, _name,
	      "testing <%s> -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n", buf);
	    test(buf, table);
	    if ( debug >= HASH_HIGH ) {
		dump_hash_table(table);
	    }
#if !defined(NO_CASE_FOLDING)
	    DEBUG1(HASH_LOW, _name,
		   "main: testing ignore case <%s> -*-*-*-*-*-*-*-*-*-\n", buf);
	    test(buf, tableic);
	    if ( debug >= HASH_HIGH ) {
		dump_hash_table(tableic);
	    }
#endif /* NO_CASE_FOLDING */
	}

    /*
     * hash each argument
     */
    } else {
	for (i=1; i < argc; ++i) {
	    DEBUG1(HASH_LOW, _name,
	      "testing <%s> -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n", buf);
	    test(argv[i], table);
	    if ( debug >= HASH_HIGH ) {
		dump_hash_table(table);
	    }
#if !defined(NO_CASE_FOLDING)
	    DEBUG1(HASH_LOW, _name,
		   "main: testing ignore case <%s> -*-*-*-*-*-*-*-*-*-\n", buf);
	    test(argv[i], tableic);
	    if ( debug >= HASH_HIGH ) {
		dump_hash_table(tableic);
	    }
#endif /* NO_CASE_FOLDING */
	}
    }

    /*
     * test the file operations
     */
#if !defined(MEM_ONLY)
    DEBUG(HASH_LOW, _name, "main: hash_file_test\n");
    hash_file_test(table);
#if !defined(NO_CASE_FOLDING)
    DEBUG(HASH_LOW, _name, "main: hash_file_test ignore case\n");
    hash_file_test(tableic);
#endif /* NO_CASE_FOLDING */
#endif /* MEM_ONLY */

    /*
     * final cleanup
     */
    DEBUG(HASH_LOW, _name, "free memory\n");
    /* free the hash table elements */
    for (cur=walk_hash((struct hash *)NULL,table);
	 cur != NULL;
	 cur=walk_hash(cur,table))
    {
	free_hash_element(cur, 0, table); /* free without deletion */
    }
    free_hash_table(table);	/* free up the memory */
#if !defined(NO_CASE_FOLDING)
    DEBUG(HASH_LOW, _name, "main: free memory ignore case\n");
    for (cur=walk_hash((struct hash *)NULL,tableic);
	 cur != NULL;
	 cur=walk_hash(cur,tableic))
    {
	free_hash_element(cur, 0, tableic); /* free without deletion */
    }
    free_hash_table(tableic);	/* free up the memory */
#endif /* NO_CASE_FOLDING */
    DEBUG(HASH_LOW, _name, "main: end\n");
    exit(0);
}


/*
 * perform various tests on a string
 */
void
test(buf, table)
    char *buf;			/* the key to add */
    struct hash_table *table;	/* our allocated table */
{
    char *_name="test";
    register struct hash *cur;	/* the current hash entry */
    char *data;			/* the data we stored */
    long datalen;		/* length of data */
    long buflen;		/* length of the buffer string + NULL */
    long i;			/* index */
    int caseflag = 0;		/* 0 ==> use strcmp, 1 ==> strcmpic */

    /* test the add function */
    buflen = strlen(buf)+1;
    i = add_to_hash(buf, buf, buflen, table);
    DEBUG2(HASH_LOW, _name, "<%s> add: %d\n", buf, i);

    /* test the lookup function */
    DEBUG1(HASH_MED, _name, "<%s> lookup\n", buf);
    if (lookup_in_hash(buf, &data, &datalen, table) != ALREADY_HASHED) {
	exit_internal(HASH_MED, _name,
	      "test: add_to_hash lost <%s>", buf);
    } else if (stringcmp(buf, data, caseflag) != 0 || buflen != datalen) {
	exit_internal(HASH_MED, _name,
	      "test: add_to_hash lookup <%s> != <%s>", buf, data);
    }
    i = add_to_hash(buf, buf, buflen, table);
    if (i != ALREADY_HASHED) {
	exit_internal(HASH_MED, _name, 
	  "add_to_hash returned: %d for #2\n", i);
    }

    /* test the delete function */
    DEBUG1(HASH_MED, _name, "<%s> delete\n", buf);
    cur = delete_from_hash(buf, table);	/* delete something that exists */
    if (cur == NULL) {
	exit_internal(HASH_MED, _name,
	      "test: delete_from_hash unable to delete <%s>", buf);
    } else if (stringcmp(buf, hash_data(cur), caseflag) != 0 ||
	       buflen != cur->datalen) {
	exit_internal(HASH_MED, _name,
	  "delete_from_hash mis delete of <%s>", buf);
    } else {
	free_hash_element(cur, 0, table); /* free up memory */
    }
    DEBUG1(HASH_MED, _name, "<%s> empty delete\n", buf);
    cur = delete_from_hash(buf, table);	/* delete a nothing */
    if (cur != NULL) {
	exit_internal(HASH_MED, _name,
	  "delete_from_hash ghost delete #2 of <%s>", buf);
    }

    /* test the store function */
    DEBUG1(HASH_MED, _name, "<%s> store\n", buf);
    cur = store_in_hash(buf, buf, buflen, table);
    if (cur != NULL) {
	exit_internal(HASH_MED, _name,
	  "store_in_hash ghost store of <%s>", buf);
    }
    DEBUG1(HASH_MED, _name, "<%s> store #2\n", buf);
    cur = store_in_hash(buf, buf, buflen, table);
    if (cur == NULL) {
	exit_internal(HASH_MED, _name,
	      "test: store_in_hash lost store #2 of <%s>", buf);
    } else if (stringcmp(buf, hash_data(cur), caseflag) != 0 ||
	       buflen != cur->datalen) {
	exit_internal(HASH_MED, _name,
	  "store_in_hash mis store #2 of <%s>", buf);
    } else {
	free_hash_element(cur, 0, table); /* free up memory */
    }
    
    /* test the replace function */
    DEBUG1(HASH_MED, _name, "<%s> replace_in_hash\n", buf);
    cur = replace_in_hash(buf, buf, buflen, table);
    if (cur == NULL) {
	exit_internal(HASH_MED, _name,
	      "test: replace_in_hash lost <%s>", buf);
    } else if (stringcmp(buf, hash_data(cur), caseflag) != 0 ||
	       buflen != cur->datalen) {
	exit_internal(HASH_MED, _name,
	  "replace_in_hash mis replace of <%s>", buf);
    } else {
	free_hash_element(cur, 0, table); /* free up memory */
    }
    DEBUG1(HASH_MED, _name, "<%s> replace_in_hash empty\n", buf);
    cur = delete_from_hash(buf, table);	/* delete something that exists */
    if (cur == NULL) {
	exit_internal(HASH_MED, _name,
	  "delete_from_hash unable to delete #3 <%s>", buf);
    } else {
	free_hash_element(cur, 0, table); /* free up memory */
    }
    cur = replace_in_hash(buf, buf, buflen, table);
    if (cur != NULL) {
	exit_internal(HASH_MED, _name,
	  "replace_in_hash ghost replace of <%s>", buf);
    } else {
	/* put it back for keeps */
	cur = store_in_hash(buf, buf, buflen, table);
	if (cur != NULL) {
	    exit_internal(HASH_MED, _name,
	      "store_in_hash store #3 lost <%s>", buf);
        }
    }
}


/*
 * dump_hash_table - dump the hash table
 */
void
dump_hash_table(table)
    struct hash_table *table;	/* our allocated table */
{
    char *_name="dump_hash_table";
    long i;			/* index */
    struct hash *cur;		/* the current hash entry */

    /*
     * check the hash chains
     */
    for (i=0; i < TABLE_LEN; ++i) {
	DEBUG1(HASH_HIGH, _name, "slot[%d]:", i);
	/* check for empty slots */
	if (empty_slot(table->slot[i])) {
	    DEBUG(HASH_HIGH, _name, "Empty\n");
	    continue;
	}
	for (cur=table->slot[i]; cur != NULL; cur=next_hash(cur, table)) {
	    if (cur->keylen == 0) {
		DEBUG2(HASH_HIGH, _name, " 0x%lx: <%s> :EOC ",
		   (long)cur, cur->keystr);
	    } else {
		DEBUG3(HASH_HIGH, _name, " 0x%lx: <%s> :0x%lx ",
		   (long)cur, cur->keystr, (long)cur->succ);
	    }
	}
	DEBUG(HASH_HIGH, _name, "\n");
    }
}


#if !defined(MEM_ONLY)
/*
 * hash_file_test - test the hash table file operations
 */
void
hash_file_test(table)
    struct hash_table *table;		/* the hash table to operate on */
{
    char *_name="hash_file_test";
    char *filename;		/* the file to operate on */
    struct stat buf;		/* file status buffer */
    struct hash *cur;		/* current location in hash table */
    struct hash *cur2;		/* current location in hash table2 */
    struct hash_table *table2;	/* the hash table to operate on */
    int caseflag;		/* 0 ==> use strcmp, 1 ==> strcmpic */
    char *template;		/* template for mktemp */
    FILE *stream;		/* the file stream */
    char *mktemp();		/* form a temp filename */

    /*
     * open up a file to perform hash table operations into
     */
    template = (char *)malloc(strlen(tempname)+1);
    strcpy(template, tempname);
    filename = mktemp(template);
    DEBUG1(HASH_LOW, _name, "hash_file_test: using <%s>\n", filename);
    stream = fopen(filename, "w");
    if (stream == NULL) {
	exit_internal(HASH_MED, _name, 
	  "hash_file_test: can not creat <%s>", filename);
    }

    /*
     * write out the hash table
     */
    write_hash_table(table, stream);
    if (fclose(stream) != 0) {
	exit_internal(HASH_MED, _name, "hash_file_test: can not fclose\n");
    }

    /*
     * reread the hash table into memory
     */
    DEBUG1(HASH_MED, _name, "hash_file_test: rereading <%s>\n", filename);
    stream = fopen(filename, "r");
    if (stream == NULL) {
	exit_internal(HASH_MED, _name, 
	  "hash_file_test: can not reopen <%s>", filename);
    }
    if (fstat(fileno(stream), &buf) < 0) {
	exit_internal(HASH_MED, _name, 
	  "hash_file_test: can not stat <%s>", filename);
    }
    table2 = (struct hash_table *)malloc(buf.st_size * sizeof(char));
    if (fread((char *)table2,sizeof(char),buf.st_size,stream) != buf.st_size) {
	exit_internal(HASH_MED, _name, 
	  "hash_file_test: can not reread <%s>", filename);
    }

    /*
     * walk thru each hash table and verify string/data
     */
    caseflag = table->flags & HASH_STRCMP;
    DEBUG(HASH_MED, _name, "hash_file_test: verify\n");
    for (cur=walk_hash((struct hash *)NULL,table),
	 	cur2=walk_hash((struct hash *)NULL,table2);
	 cur != NULL || cur2 != NULL;
	 cur=walk_hash(cur,table), cur2=walk_hash(cur2,table2))
    {
	    /* compare cur and cur2 */
	    if (stringcmp(cur->keystr,cur2->keystr,caseflag) != 0) {
		exit_internal(HASH_MED, _name,
		      "hash_file_test: key mismatch: <%s> != <%s>\n",
		      cur->keystr, cur2->keystr);
	    }
	    if (cur->datalen != cur2->datalen) {
		exit_internal(HASH_MED, _name,
		      "hash_file_test: key mismatch: %d != %d\n",
		      cur->datalen, cur2->datalen);
	    }
	    if (memcmp(hash_data(cur), hash_data(cur2), cur->datalen) != 0) {
		exit_internal(HASH_MED, _name,
		      "hash_file_test: data mismatch between <%s> and <%s>\n",
		      cur->keystr, cur2->keystr);
	    }
    }

    /*
     * cleanup
     */
    DEBUG(HASH_MED, _name, "hash_file_test: cleanup\n");
    free(table2);
}
#endif /* MEM_ONLY */


/*
 * dbug - trace standalone stub for trace/debug output
 *
 * name		routine name
 * fmt		printf(3) format
 * ...		arguments for printf
 */
/*VARARGS2*/
static void
dbug(char *name, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)printf("DEBUG %s: ", name); 
    (void)vprintf(fmt, ap);
    va_end(ap);
    fflush(stdout);
}

#endif	/* STANDALONE */
