/*
 * hash - Fowler/Noll/Vo hash code library
 *
 * @(#) $Revision: 1.9 $
 * @(#) $Id: hash.h,v 1.9 1999/10/18 20:21:31 chongo Exp $
 * @(#) $Source: /usr/local/src/cmd/fnvhash/RCS/hash.h,v $
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
 * hash modes
 *
 * NO_CASE_FOLDING:
 *	defined ==> hashing is always case sensitive
 *	undef   ==> hashing without regard it case may be optionally performed
 *
 * SLOW_CASE_CONVERSION:	(ignored if NO_CASE_FOLDING is defined)
 *	defined ==> use Libc tolower() and toupper() conversion
 *	undef   ==> use fast table lookup case conversion
 *
 * MEM_ONLY:
 *	defined ==> disable disk hash ops, chains are pointer only
 *	undef   ==> allow disk hash ops, chains are pointers or arrays
 *
 * STANDALONE:
 *	defined ==> compile as a standalong test routine with a main() function
 *	undef   ==> compile the hash library
 */


/*
 * Derive bit counts extended to other types
 */
#define BITS_PER_CHAR 8
#define BITS_PER_LONG	(sizeof(long)*BITS_PER_CHAR)
#define BITS_PER_INT	(sizeof(int)*BITS_PER_CHAR)
#define BITS_PER_SHORT	(sizeof(short)*BITS_PER_CHAR)
#define BYTES_PER_ALIGN (sizeof(int))


/*
 * pointer - same size as a pointer to a structure
 *
 * Setup the typedef so that `pointer' is the same size as a pointer to
 * a structure.  On most machines, this is a long.  On some machines where
 * long is larger than an int, `pointer' is an unsigned int.  Woe to machines
 * with hardware addresses < 19 bits.  (smail might not even fit in that case!)
 *
 * Assume:
 *	typedef long pointer;
 *	struct foo *p;		<-- some j-random pointer
 *	long v;			<-- same type as the typedef
 *	union offptr {
 *		struct foo *ptr;
 *		pointer addr;
 *	} x;			<-- union of a pointer and the typedef
 *
 * then the following must be true:
 *	x.addr = v;			  implies   v == (pointer)(x.ptr)
 *	x.addr = (pointer)p;		  implies   p == x.ptr
 *	x.addr == (pointer)(x.ptr)
 *	x.ptr  = p;			  implies   p == (struct foo *)(x.addr)
 *	x.ptr  = (struct foo *)v;	  implies   v == x.addr
 *	x.ptr  == (struct foo *)(x.addr)
 */
typedef long pointer;


#if defined(MEM_ONLY)

/*
 * When MEM_ONLY is defined, there is no need to play the game of
 * odd addresses indicating array elements.  We will assume that
 * every address is an even address.  We don't really care if there 
 * are odd addresses.  We just need the macrtos to assume something.
 */
#define is_odd(addr) (0)			/* assume never odd */
#define is_even(addr) (1)			/* assume always even */
#define to_odd(addr) (addr)			/* stub out conversion */
#define to_even(addr) (addr)			/* stub out conversion */

#else /* MEM_ONLY */

/*
 * even bytes and odd bytes
 *
 * The hash system used by hash.c (on disk and in memory) relies on the
 * fact that BYTES_PER_ALIGN (see an arch file) will align things to even
 * byte boundary.  Any hash `pointer' that refers to an odd value (2n+1)
 * is taken to mean an offset rather than a real address.  Note that odd
 * valued pointers are never dereferenced.
 *
 * We need to be able to distinguish between even and odd addresses, and
 * to convert to/from even/odd addresseses.  On most machines this is a
 * trivial marco.  Brain damaged machines with brain damaged segment
 * addressing may have to form their own creative macros.
 *
 * Assume:
 *	#define NULL 0		<-- for the sake of this example
 *	struct foobar *ptr;	<-- ptr points to a BYTES_PER_ALIGN object
 *	pointer addr;		<-- addr is of a BYTES_PER_ALIGN object
 *
 * Then these conversion macros must perform the following:
 *   to_odd() and to_even() do not distroy data:
 *	addr = to_odd(ptr)  ==>  ptr == to_even(addr)
 *	addr = to_even(ptr) ==>  ptr == to_odd(addr)
 *   is_even() and is_odd() are reflexive:
 *	is_odd(addr)  != 0  ==>  is_even(to_even(addr)) == 0
 *	is_odd(ptr)   != 0  ==>  is_even(to_even(ptr)) == 0
 *	is_even(addr) != 0  ==>  is_odd(to_odd(addr)) == 0
 *	is_even(ptr)  != 0  ==>  is_odd(to_odd(ptr)) == 0
 *   NULL is an even pointer:
 *	is_even(NULL) != 0
 */
#define is_odd(addr) ((pointer)(addr)&0x1)	/* non-zero if odd pointer */
#define is_even(addr) (! is_odd(addr))		/* non-zero if even pointer */
#define to_odd(addr) ((pointer)(addr)|0x1)	/* from even or odd to odd */
#define to_even(addr) ((pointer)(addr)&(~0x1))	/* from even or odd to even */

/*
 * set_ptr(ptr,addr) places the numeric value of `addr' into the pointer `ptr'.
 * That is, we borrow the pointer `ptr' to store numeric data.
 *
 * get_ptr(ptr) converts a pointer `ptr' back into the value which was stored
 * into it by set_ptr().
 */
#define get_ptr(ptr)      ( *(pointer *)&(ptr) )
#define set_ptr(ptr,addr) ( get_ptr(ptr) = (pointer)(addr) )

#endif /* MEM_ONLY */


/*
 * case conversion macros
 */
#ifdef FAST_CASE_CONVERSION
# undef tolower
# define tolower(c) (lowcase[c])	/* quick table lookup for lower case */
# define lowercase(c) (tolower(c))	/* lowercase uppercase chars */
# undef toupper
# define toupper(c) (upcase[c])		/* quick table lookup for upper case */
# define uppercase(c) (toupper(c))	/* uppercase lowercase chars */
#else /* FAST_CASE_CONVERSION */
/* slower Libc way of case conversion */
# define lowercase(c) (islower(c) ? (char)(c) : (char)tolower(c))
# define uppercase(c) (isupper(c) ? (char)(c) : (char)toupper(c))
#endif /* FAST_CASE_CONVERSION */


/*
 * return the integer offset from the start of a given structure type
 * to a given tag.
 */
#define OFFSET(type, tag) \
    (long)((char *)(&((struct type *)0)->tag) - (char *)(struct type *)0)


/*
 * debug macros
 */
#define HASH_LOW 20
#define HASH_MED 40
#define HASH_HIGH 60
#define HASH_VHIGH 80
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
# ifndef DBG_LVL
#  define DBG_LVL HASH_LOW
# endif
#else	/*STANDALONE*/
# define DEBUG(d,n,m)
# define DEBUG1(d,n,m,a)
# define DEBUG2(d,n,m,a,b)
# define DEBUG3(d,n,m,a,b,c)
# define DEBUG4(d,n,m,a,b,c,e)
#endif /*STANDALONE*/


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
    int flags;		/* see flags section, default == 0 */
    long indx;		/* the walk_hash() slot location */
    long len;		/* the number of hash slots in this table */
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

/* hash flags */
#define HASH_DEFAULT	0x00000000	/* use strcmpic, everything in memory */
#define HASH_STRCMP	0x00000001	/* 0 ==> use strcmpic, 1 ==> a != A */
#define HASH_FLAGMASK	0x00000001	/* logical and of valid flags */


/*
 * hash:
 *	variable length hash table structure found in hash slot chains
 *
 * #if defined(MEM_ONLY)
 *
 * Hash slow chains, when MEM_ONLY is defined, are simply pointer
 * chains that are NULL terminated.
 *
 * #else (* MEM_ONLY *)
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
 *    The next entry is hash_len(cur) bytes beyond ``cur''
 *
 *  hash entries in queue form:
 *    is_even(cur->succ) is true
 *    cur->succ == NULL ==> end of chain
 *
 * #endif (* MEM_ONLY *)
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
#define hash_align(val) (((long)(val)+(BYTES_PER_ALIGN-1))&~(BYTES_PER_ALIGN-1))
/* correctly padded length of the key string - given the key string */
#define keystr_len(keystring) \
  hash_align(strlen((char *)(keystring))+1)
/* hash slot size in bytes - given lengths of the padded key string and data */
#define hashslot_size(keystrlen,datalen) \
 (hash_align(OFFSET(hash, keystr[0]) + \
	     hash_align((long)(keystrlen)+1) + \
	     hash_align((long)(datalen))))
/* hash slot length in bytes - given a ``struct hash'' element pointer */
#define hash_len(cur) \
 (OFFSET(hash, keystr[0]) + \
  (long)(((struct hash *)(cur))->keylen) + \
  hash_align((((struct hash *)(cur))->datalen)))

/* pointer to hash data - given a ``struct hash'' element pointer */
#define hash_data(cur) \
 ((((struct hash *)(cur))->datalen > 0) ? \
  ((char *)(((struct hash *)(cur))->keystr+((struct hash *)(cur))->keylen)) :\
  ((char *)NULL))


/*
 * stringcmp - compare two strings
 *
 * Some hash tables compare strings without regard to case while
 * others treat case as significant.  Returns <0, ==0, >0 if str1
 * is less than, equal to, or greater than str2.
 *
 * #if defined(NO_CASE_FOLDING)
 *
 * args:
 *	str1	- char *
 *		  first string to compare
 *	str2	- char *
 *		  second string to compare
 *	strcase	- int
 *		  this value is ignored when NO_CASE_FOLDING is defined
 *
 * #else (* NO_CASE_FOLDING *)
 *
 * args:
 *	str1	- char *
 *		  first string to compare
 *	str2	- char *
 *		  second string to compare
 *	strcase	- int
 *		  case == 0 ==> use strcmp,  == 1 ==> use strcmpic
 *
 * #endif (* NO_CASE_FOLDING *)
 */
#if defined(NO_CASE_FOLDING)
# define stringcmp(str1, str2, strcase) (strcmp(str1, str2))
#else /* NO_CASE_FOLDING */
# define stringcmp(str1, str2, strcase) \
    (strcase ? strcmpic(str1, str2) : strcmp(str1, str2))
#endif /* NO_CASE_FOLDING */


/*
 * hash_string - hash string with or without regard to case
 *
 * #if defined(NO_CASE_FOLDING)
 *
 * args:
 *	str	- char *
 *		  the string to hash
 *	mod	- long
 *		  prime modulus used in hashing
 *	strcase	- int
 *		  this value is ignored when NO_CASE_FOLDING is defined
 *
 * #else (* NO_CASE_FOLDING *)
 *
 * args:
 *	str	- char *
 *		  the string to hash
 *	mod	- long
 *		  prime modulus used in hashing
 *	strcase	- int
 *		  == 0 ==> hash where a == A, == 1 ==> hash where a != A
 *
 * #endif (* NO_CASE_FOLDING *)
 */
#if defined(NO_CASE_FOLDING)
# define hash_string(str, mod, strcase) (hash_str(str, mod))
#else /* NO_CASE_FOLDING */
# define hash_string(str, mod, strcase) \
    (strcase ? hash_stric(str, mod) : hash_str(str, mod))
#endif /* NO_CASE_FOLDING */


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
    ((struct hash *)(item))->succ = *((struct hash **)(prev)); \
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
  (*(struct hash **)(prev)) = ((struct hash *)(cur))->succ; \
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
    ((struct hash *)(item))->succ = ((struct hash *)(cur))->succ; \
    (prev) = (struct hash *)(item); \
}


/*
 * hash_addr - return memory address of a 'succ' value
 *
 * #if defined(MEM_ONLY)
 *
 * The 'succ', when MEM_ONLY is defined, is always a queue pointer.
 *
 * #else (* MEM_ONLY *)
 * 
 * If 'succ' is an array pointer form, hash_addr() will convert it to
 * a memory address, otherwise the queue pointer is returned.
 *
 * #endif (* MEM_ONLY *)
 *
 * input:
 *	aqval	- struct hash *
 *		  the array or queue value tp be converted to a pointer
 *	table	- struct hash_table *
 *		  the hash table holding ``cur''
 * output:
 *	a pointer to the object reference by succ
 */
#if defined(MEM_ONLY)
# define hash_addr(aqval, table) ((struct hash *)(aqval))
#else /* MEM_ONLY */
# define hash_addr(aqval, table) \
   (is_odd((struct hash *)(aqval)) ? \
     (struct hash *)((char *)(table) + to_even((struct hash *)(aqval))) :\
     (struct hash *)(aqval))
#endif /* MEM_ONLY */


/*
 * next_hash - return the next element in a hash slot chain
 *
 * returns NULL if the next element was beyond the end of the chain
 *
 * input:
 *	cur	- struct hash *
 *		  our current location
 *	table	- struct hash_table *
 *		  the hash table holding ``cur''
 * output:
 *	a pointer to the next element, or NULL if no next element
 */
#define next_hash(cur, table) hash_addr(cur->succ, table)


/* 
 * predeclare functions 
 */
struct hash *walk_hash();
void free_hash_element();
struct hash_table *new_hash_table();
void free_hash_table();
int add_to_hash();
struct hash *replace_in_hash();
struct hash *store_in_hash();
int lookup_in_hash();
struct hash *delete_from_hash();
#if !defined(MEM_ONLY)
void write_hash_table();
#endif
char *find_hash();
