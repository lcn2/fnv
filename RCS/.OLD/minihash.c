/* $Id$  */
/*
 * hash - hash code library
 */

/*
 * Copyright (C) 1987, 1993 Ronald S. Karr and Landon Curt Noll
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice, and the
 * disclaimer below appear in all of the following:
 * 
 *         * supporting documentation
 *         * source copies 
 *         * source works derived from this source
 *         * binaries derived from this source or from derived source
 * 
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
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
#include <malloc.h>
#include "hash.h"

/* static functions used in this file */
static int hash_str();
static struct hash *new_hash_element();

/* predeclare functions */
int add_to_hash();
int lookup_in_hash();
struct hash_table *new_hash_table();
struct hash *delete_from_hash();
struct hash *replace_in_hash();
struct hash *store_in_hash();
struct hash *walk_hash();
void free_hash_element();
void free_hash_table();
void exit_internal();

#ifdef STANDALONE
int debug = DBG_LVL;	/* default debug level */
void dbug();		/* print debug output */
#endif

/*
 * hash_str - convert a trying into a hash value
 *
 * We build the hash value one character at a time by repeating the following
 * steps for each character:
 *
 *	1) The previous value is shifted up (by HASH_UP_SHIFT) and the
 *	   current character is added
 *	2) any upper level excess bits are fetched (by masking with
 *	   {H,L}_HASH_MASK) and xor-ed into bits near the bottom
 *	3) the upper level excess bits are cleared
 *
 * In the end, the hash value is taken modulo `mod' to produce a slot number.
 *
 * input:
 *	str	- string to hash
 *	mod	- number of hash table entries
 * output:
 *	the slot number on which `str' belongs
 *
 * NOTE: For more optimal hashing of smaller hash tables (entries < HASH_LEVEL)
 *       we L_ value constants.  This gives us a faster hash fold.  For larger
 *	 hash tables, this is not needed so H_ value constants are used.
 *
 * NOTE: mod should be a prime <= ~H_HASH_MASK
 */
static int
hash_str(str, mod)
    register char *str;			/* the string to hash */
    int mod;				/* prime modulus, size of hash table */
{
    register unsigned long val;		/* current hash value */
    register unsigned long excess;	/* upper/excess bits in val */
    register unsigned long c;		/* the current string character */

    /* firewall - bogus case, but be safe anyway */
    if (str == NULL) {
	return 0;
    }

    /*
     * hash each character in the string
     *
     * if our mod is small enough, then use L_ value constants so that
     * strings can fold into themselves faster.
     */
    if (mod < HASH_LEVEL) {
	/* hash each character using the L_ values */
	for (val = 0; c=(unsigned long)*str; ++str) {
	    val = (val << HASH_UP_SHIFT) + c;
	    val ^= ((excess = (val&L_HASH_MASK)) >> L_DOWN_SHIFT);
	    val ^= excess;
	}
    } else {
	/* hash each character using the H_ values */
	for (val = 0; c=(unsigned long)*str; ++str) {
	    val = (val << HASH_UP_SHIFT) + c;
	    val ^= ((excess = (val&H_HASH_MASK)) >> H_DOWN_SHIFT);
	    val ^= excess;
	}
    }
    
    return (int)(val%mod);    /* our hash value, mod the hash size */
}

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
    register int indx;		/* our previous hash slot */
    register int len;		/* the table length */

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
	    indx = hash_str(cur->keystr, len);
	    /* if `cur' is an empty slot, error */
	    if (empty_slot(table->slot[indx])) {
		    exit_internal(HASH_MED, _name, 
		      "<%s> hash slot is empty", cur->keystr);
	    }
	
	    /* walk down the hash table chain looking for our entry */
	    for (prev = table->slot[indx];
		 cur != prev && prev != NULL;
		 prev = next_hash(prev)) {
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
	if ((prev = next_hash(prev)) == NULL) {
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
    table->prev = prev;
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
    int datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* hash table being added to */
{
    char *_name="new_hash_element";
    struct hash *new;		/* the new slot chain location */
    int keylen;			/* the length of the string, padded */
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
    keylen = (int)lk;		/* now we know it will fit in an int */
    /* firewall - check against bad data being passed to us */
    if (datalen < 0 || (datalen > 0 && data == NULL) || 
      (int)datalen >= (1L<<BITS_PER_SHORT))  {
	exit_internal(HASH_MED, _name,
	    "bad data passes with: <%s>  datalen: <%d>",
	    keystr, datalen);
    }

    /*
     * malloc the storage
     */
    new = (struct hash *)malloc( hashslot_size(keylen,datalen) );

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
 * Frees a hash table element according to the life of the hash table.
 * Removes the hash table element if it in the hash table unless explicitly
 * told that the element is not in the table.
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
 *	life - the alloc block to which this is to be associated, or NULL
 *	       meaning the permanent block
 * output:
 *	a pointer to a malloced empty hash table
 */
struct hash_table *
new_hash_table(tablelen, life)
    int tablelen;		/* number of slots in the hash table */
    struct block *life;		/* is the hash table permanent or temporary */
{
    char *_name="new_hash_table";
    register int i;			/* index */
    struct hash_table *table;		/* the malloced hash table */

    /*
     * firewalls
     */
    if (tablelen <= 0) {
	exit_internal(HASH_MED, _name, 
	  "tablelen: %d", tablelen);
    }
    DEBUG2(HASH_LOW, _name, "tablelen:%d life:%d\n",tablelen,life);

    /*
     * malloc the hash table
     */
    table = (struct hash_table *)malloc(table_size(tablelen));

    /*
     * initialize the table
     */
    table->len = tablelen;
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
    struct hash *cur;		/* current element to delete */

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
    int datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="add_to_hash";
    register struct hash *cur;		/* the current slot chain location */
    register struct hash *prev;		/* the previous slot chain location */
    register int cmp;			/* -1, 0, or 1 for < = > compare */
    int loc;				/* the hash slot to add onto */
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
    loc = hash_str(keystr, table->len);
    DEBUG2(HASH_LOW, _name, "keystr: <%s> slot: %d\n", 
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
	 cur != NULL; prev=cur, cur=next_hash(cur)) {
	/* 
	 * if we found the entry, stop
	 */
	DEBUG2(HASH_VHIGH, _name, "comparing <%s> to <%s>",
	       keystr, cur->keystr);
	if ((cmp = strcmp(keystr, cur->keystr)) == 0) {
	    DEBUG(HASH_MED, _name, "already hashed\n");
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
    int datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="replace_in_hash";
    register struct hash *cur;		/* the current slot chain location */
    register struct hash *prev;		/* the previous slot chain location */
    register int cmp;			/* -1, 0, or 1 for < = > compare */
    int loc;				/* the hash slot to add onto */
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
    loc = hash_str(keystr, table->len);
    DEBUG2(HASH_LOW, _name, "keystr: <%s> slot: %d\n",keystr,loc);

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
      prev=cur, cur=next_hash(cur)) {
	/* 
	 * if we found the entry, stop
	 */
	if ((cmp = strcmp(keystr, cur->keystr)) == 0) {
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
    int datalen;			/* length of data,  0 ==> no data */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="store_in_hash";
    register struct hash *cur;		/* the current slot chain location */
    register struct hash *prev;		/* the previous slot chain location */
    register int cmp;			/* -1, 0, or 1 for < = > compare */
    int loc;				/* the hash slot to add onto */
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
    loc = hash_str(keystr, table->len);
    DEBUG2(HASH_LOW, _name, "keystr: <%s> loc: %d\n", 
      keystr, loc);

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
      prev=cur, cur=next_hash(cur)) {
	/* 
	 * if we found the entry, stop
	 */
	if ((cmp = strcmp(keystr, cur->keystr)) == 0) {
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
    int *datalen;			/* where to place data len or NULL */
    struct hash_table *table;		/* the hash table to add it to */
{
    char *_name="lookup_in_hash";
    register struct hash *cur;	/* the slot chain location */
    int loc;			/* the hash slot to add onto */
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
    loc = hash_str(keystr, table->len);
    DEBUG2(HASH_LOW, _name, "keystr: <%s> slot: %d\n",
      keystr, loc);
    /* watch out for empty chains, there is nothing on them */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "found at slot END\n");
	return NOT_HASHED;
    }

    /* 
     * search the chain
     */
    for (cur=table->slot[loc]; cur != NULL; cur=next_hash(cur)) {
	/*
	 * if we found the entry, stop
	 */
	if ((cmp = strcmp(keystr, cur->keystr)) == 0) {
	    DEBUG(HASH_MED, _name, "found\n");
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
    int loc;			/* the hash slot to add onto */
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
    loc = hash_str(keystr, table->len);
    DEBUG2(HASH_LOW, _name, "keystr: <%s> loc: %d\n", 
      keystr, loc);
    /* watch out for empty chains, there is nothing on them */
    if (empty_slot(table->slot[loc])) {
	DEBUG(HASH_MED, _name, "EMPTY slot\n");
	return NULL;	/* key is not in the table */
    }

    /* 
     * search the chain for the element to delete
     */
    for (prev=NULL, cur=table->slot[loc]; cur != NULL; 
      prev=cur, cur=next_hash(cur)) {
	/* 
	 * if we found the entry, stop
	 */
	if ((cmp = strcmp(keystr, cur->keystr)) == 0) {
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

#ifdef STANDALONE

#include "varargs.h"
#include <sys/types.h>
#include <sys/stat.h>

#define TABLE_LEN 3 /* use only a few slots to stress the chain code */
#define INPUT_SIZE (70*1024)	/* max input line */

void
main(argc, argv)
    int argc;	/* arg count */
    char *argv[]; 	/* args */
{
    char *_name="main";
    int i;			/* index */
    char buf[INPUT_SIZE+1];	/* the input buffer for stdin args */
    struct hash *cur;		/* pointer to walk the table */
    struct hash_table *table;	/* our allocated table */
    void test();		/* test an with an element */
    void dump_hash_table();	/* dump the contents of the hash table */

    /*
     * establish debug level
     */
    DEBUG(HASH_LOW, _name, "start\n");
    if (argc > 1) {
	debug = atoi(argv[1]);
	--argc;
	++argv;
    }
    DEBUG1(HASH_LOW, _name, "debug level: %d\n", debug);

    /*
     * setup a hash table
     */
    table = new_hash_table(TABLE_LEN, (struct block *)NULL);

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
	}
    }

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
    exit(0);
}

/*
 * perform various tests on a string
 */
void
test(buf, table)
    char *buf;		/* the key to add */
    struct hash_table *table;	/* our allocated table */
{
    char *_name="test";
    register struct hash *cur;	/* the current hash entry */
    char *data;			/* the data we stored */
    int datalen;		/* length of data */
    int buflen;			/* length of the buffer string + NULL */
    int i;			/* index */

    /* test the add function */
    buflen = strlen(buf)+1;
    i = add_to_hash(buf, buf, buflen, table);
    DEBUG2(HASH_LOW, _name, "<%s> add: %d\n", buf, i);

    /* test the lookup function */
    DEBUG1(HASH_MED, _name, "<%s> lookup\n", buf);
    if (lookup_in_hash(buf, &data, &datalen, table) != ALREADY_HASHED) {
	exit_internal(HASH_MED, _name,
	  "add_to_hash lost <%s>", buf);
    } else if (strcmp(buf, data) != 0 || buflen != datalen) {
	exit_internal(HASH_MED, _name,
	  "add_to_hash lookup <%s> != <%s>", buf, data);
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
	  "delete_from_hash unable to delete <%s>", buf);
    } else if (strcmp(buf, hash_data(cur)) != 0 ||
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
	  "store_in_hash lost store #2 of <%s>", buf);
    } else if (strcmp(buf, hash_data(cur)) != 0 ||
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
	  "replace_in_hash lost <%s>", buf);
    } else if (strcmp(buf, hash_data(cur)) != 0 ||
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
    int i;				/* index */
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
	for (cur=table->slot[i]; cur != NULL; cur=next_hash(cur)) {
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

/*VARARGS2*/
void
exit_internal(level, name, fmt, va_alist)
    int level;			/* error exit level */
    char *name;			/* routine name */
    char *fmt;			/* printf(3) format */
    va_dcl                      /* arguments for printf */
{
    va_list ap;

    va_start(ap);
    (void)printf("INTERNAL: level=%d %s: ", level, name); 
    (void)vprintf(fmt, ap);
    putchar('\n');			/* fatal messages not \n terminated */
    va_end(ap);
    fflush(stdout);

    exit(1);
}

/*
 * dbug - trace standalone stub for trace/debug output
 */
/*VARARGS2*/
void
dbug(name, fmt, va_alist)
    char *name;			/* routine name */
    char *fmt;			/* printf(3) format */
    va_dcl                      /* arguments for printf */
{
    va_list ap;

    va_start(ap);
    (void)printf("DEBUG %s: ", name); 
    (void)vprintf(fmt, ap);
    va_end(ap);
    fflush(stdout);
}

#endif	/* STANDALONE */
