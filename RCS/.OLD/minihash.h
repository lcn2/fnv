/* $Id$  */
/*
 * hash - definitions for the support of hash.c
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
 * architecture constants
 */
#define BITS_PER_BYTE 8
#define BITS_PER_SHORT (sizeof(short)*BITS_PER_BYTE)
#define BITS_PER_LONG (sizeof(long)*BITS_PER_BYTE)
#define BYTES_PER_ALIGN (sizeof(int))

/*
 * return the integer offset from the start of a given structure type
 * to a given tag.
 */
#define OFFSET(type, tag) \
    (int)((char *)(&((struct type *)0)->tag) - (char *)(struct type *)0)
/*
 * debug macros
 */
#ifdef STANDALONE
# define DEBUG(d,n,m) \
     {						\
	 if (d <= debug) {			\
		dbug(n,m);			\
	 }					\
     }
# define DEBUG1(d,n,m,a) \
    {						\
	if (d <= debug) {			\
		dbug(n,m,a);			\
	}					\
    }
# define DEBUG2(d,n,m,a,b) \
    {						\
	if (d <= debug) {			\
		dbug(n,m,a,b);			\
	}					\
    }
# define DEBUG3(d,n,m,a,b,c) \
    {						\
	if (d <= debug) {			\
		dbug(n,m,a,b,c);		\
	}					\
    }
# define DEBUG4(d,n,m,a,b,c,e) \
    {						\
	if (d <= debug) {			\
		dbug(n,m,a,b,c,e);		\
	}					\
    }
#else	/*STANDALONE*/
# define DEBUG(d,n,m)
# define DEBUG1(d,n,m,a)
# define DEBUG2(d,n,m,a,b)
# define DEBUG3(d,n,m,a,b,c)
# define DEBUG4(d,n,m,a,b,c,e)
# define DBG_LVL 0
#endif /*STANDALONE*/
#define HASH_LOW 20
#define HASH_MED 40
#define HASH_HIGH 60
#define HASH_VHIGH 80
#ifndef DBG_LVL
# define DBG_LVL HASH_LOW
#endif

/*
 * hash function values
 */
/* shift the hash up HASH_UP_SHIFT bits before adding the next character */
#define HASH_UP_SHIFT (3)
/* if hash mod < (1<<HASH_LEVEL_SHIFT), use L_ hash values, otherwise H_ */
#define HASH_LEVEL_SHIFT (17)
#define HASH_LEVEL (1<<HASH_LEVEL_SHIFT)
/* for hash sizes where the hash mod < HASH_LEVEL */
#define L_MASK_SHIFT (HASH_LEVEL_SHIFT)
#define L_HASH_MASK (~((1<<L_MASK_SHIFT)-1))     /* mask of excess bits */
#define L_DOWN_SHIFT (L_MASK_SHIFT-1)		 /* shift down excess */
/* for hash sizes where the hash mod >= HASH_LEVEL */
#define H_MASK_SHIFT (BITS_PER_LONG-HASH_UP_SHIFT-2)
#define H_HASH_MASK (~((1<<H_MASK_SHIFT)-1))	 /* mask of excess bits */
#define H_DOWN_SHIFT (H_MASK_SHIFT-2)		 /* shift down excess */

/*
 * what add_to_hash(), lookup_in_hash(), ... return
 */
#define NOT_HASHED (-1)		/* the key was not hashed */
#define JUST_HASHED (0)		/* the key was just hashed */
#define ALREADY_HASHED (1)	/* the key has been already hashed */

/*
 * hash_table - hash slots which map integers to mixed chains of hash elements
 *
 * A hash table consists of a ``struct hash_table'' and related hash slot
 * chains of ``struct hash''.  A hash table contains the number of slots,
 * and that number of slot pointers.  Each slot points to a slot chain
 * of ``struct hash'' elements.  Hash slot chains are kept in sorted order.
 *
 * The function hash_str() maps a string the index of one of the hash table
 * slot pointers.  A slot that does not have a chain has the value NULL.
 */
struct hash_table {
    int len;		/* the number of hash slots in this table */
    int indx;		/* the walk_hash() slot location */
    struct hash *prev;	/* the walk_hash() current element location */
    struct hash *slot[1];	/* ``len'' consecutive slot chain pointers */
};
/* hash table entry size in bytes - given the number of slots */
#define table_size(len) \
    (((len)*sizeof(struct hash *)) + OFFSET(hash_table, slot[0]))
/* return TRUE if the hash table slot is empty, not-TRUE otherwise */
#define empty_slot(slot) ((struct hash *)(slot) == NULL)
/* return FALSE if the hash table slot is empty, not-FALSE otherwise */
#define full_slot(slot) ((struct hash *)(slot) != NULL)

/*
 * hash:
 *	variable length hash table structure found in hash slot chains
 *
 * Hash slot chains may be a mixture of arrays of ``struct hash'' elements
 * and linked lists of ``struct hash'' elements.  The reason for this mixture
 * is that some of the data is pre-constructed on disk by programs that
 * have no knowledge addresses, and thus simply stack data elements one after
 * another in an array.  Then again, some of the data is malloced and inserted 
 * into lists at run time, and thus must be linked in by pointers.
 *
 * To deal with this problem, the following methods are used:
 *
 *  hash entries in array form:
 *    is_odd(cur->succ) is true
 *    The next entry is hash_len(cur) bytes beyond `cur'
 *
 *  hash entries in queue form:
 *    is_even(cur->succ) is true
 *    cur->succ == NULL ==> end of chain
 *
 * A hash slot chain consists of a set of ``struct hash'' elements whose key
 * strings mapped onto the same hash table slot index.  All hash slot chain
 * elements are kept in sorted order as defined by memcmp().  (smail needs
 * to compare/hash without regard to case, so it uses strcmpic() instead)
 *
 * The location ``element.keystr'' is the starting location of the key string.
 * The length of the key string is ``element.keylen''.  Each key string must
 * be NULL byte terminated.
 *
 * One can may optionally associate data with the element.  The length of
 * this data is found in ``element.datalen''.  Extra bytes may be added to
 * pad the ``element'' to a BYTES_PER_ALIGN byte length.  The first byte of
 * the starting data is located at ``element.keystr[element.keylen]''.
 */
struct hash {
    /* NOTE: succ must be the first element */
    struct hash *succ; 	/* pointer to next hash entry, or NULL */
    short keylen;	/* length of key string + '\0' + word boundary pad */
    short datalen;	/* length in bytes of the data beyond the key string */
    char keystr[1];	/* padded key string, optionally followed by data */
};

/* hash_align aligns objects on optimal addresses */
#define hash_align(val) (((int)(val)+(BYTES_PER_ALIGN-1))&~(BYTES_PER_ALIGN-1))
/* correctly padded length of the key string - given the key string */
#define keystr_len(keystring) \
  hash_align(strlen((char *)(keystring))+1)
/* hash slot size in bytes - given lengths of the padded key string and data */
#define hashslot_size(keystrlen,datalen) \
 (hash_align(OFFSET(hash, keystr[0]) + \
	     hash_align((int)(keystrlen)+1) + \
	     hash_align((int)(datalen))))
/* hash slot length in bytes - given a ``struct hash'' element pointer */
#define hash_len(cur) \
 (OFFSET(hash, keystr[0]) + \
  (int)(((struct hash *)(cur))->keylen) + \
  hash_align((((struct hash *)(cur))->datalen)))

/* pointer to hash data - given a ``struct hash'' element pointer */
#define hash_data(cur) \
 ((((struct hash *)(cur))->datalen > 0) ? \
  ((char *)(((struct hash *)(cur))->keystr+((struct hash *)(cur))->keylen)) :\
  ((char *)NULL))

/*
 * insert_hash - insert an element before our current location in a slot chain
 *
 * Insert an element after an element (or hash slot head) in a chain.  We
 * pass `prev', a pointer to the `struct hash'-pointer that refers to 
 * our current chain location.  Our job is to have `prev' point to the
 * new element and our new element point to the current chain location.
 *
 * args:
 *	prev	- struct hash **
 *		  the entry before the place of insertion.  This pointer
 *		  may actually be the hash table slot pointer.
 *	item	- struct hash *
 *		  the item to insert
 */
#define insert_hash(prev, item) { \
    (struct hash *)(item)->succ = *((struct hash **)(prev)); \
    *(struct hash **)(prev) = (struct hash *)(item); \
}

/*
 * delete_hash - delete an element in the hash chain
 *
 * Given two ``struct hash'' elements `prev' and `item', delete_hash() will
 * remove `item' from the hash slot chain.
 *
 * input:
 *	prev	- struct hash **
 *		  pointer to the previous chain's forward pointer.  This
 *		  pointer may actually be the hash hash table slot pointer.
 *		  We will delete the element to which this pointer points.
 *	cur	- struct hash *
 *		  the `next' pointer of the item to delete
 */

#define delete_hash(prev, cur) { \
  (*(struct hash **)(prev)) = (struct hash *)(cur)->succ; \
}

/*
 * replace_hash - replace an element in the hash chain
 *
 * Replace the element referred by `cur' and pointer at by `prev' with the
 * entry `item'
 *
 * input:
 *	prev	- struct hash *
 *		  the previous chain's forward pointer.  This pointer may
 *		  actually be the hash table slot pointer.  We will replace
 *		  the element to which this pointer points at.
 *	cur	- struct hash *
 *		  the element being replaced (i.e., deleted)
 *	item	- struct hash *
 *		  the element which is replacing `cur'
 */
#define replace_hash(prev, cur, item) { \
    (struct hash *)(item)->succ = (struct hash *)(cur)->succ; \
    (struct hash *)(prev) = (struct hash *)(item); \
}

/*
 * next_hash - return the next element in a hash slot chain
 *
 * returns NULL if the next element was beyond the end of the chain
 *
 * input:
 *      cur     - struct hash *
 *                our current location
 * output:
 *      a pointer to the next element, or NULL if no next element
 */
#define next_hash(cur) ((struct hash *)(cur)->succ)
