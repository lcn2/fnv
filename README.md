# fnv

Fowler/Noll/Vo hash

* The fnv1a32, fnv1a64 implement the recommended FNV-1a hash.

* The fnv132, fnv164 implement the common FNV-1 hash.

* The fnv032, fnv064 implement the historic FNV-0 hash (used to generate FNV-1a and FNV-1 offset basis only).


# To install

```sh
make clobber all test
sudo make install clobber
```


# Fowler/Noll/Vo hash

The basis of this hash algorithm was taken from an idea sent
as reviewer comments to the IEEE POSIX P1003.2 committee by:

[Phong Vo](http://www.research.att.com/info/kpv)
[Glenn Fowler](http://www.research.att.com/~gsf/)

In a subsequent ballot round [Landon Curt Noll](http://www.isthe.com/chongo)
improved on their algorithm.  Some people tried this hash and found that it
worked rather well.  In an email message to Landon, they named it the
`Fowler/Noll/Vo` or FNV hash.

FNV hashes are designed to be fast while maintaining a low
collision rate. The FNV speed allows one to quickly hash lots
of data while maintaining a reasonable collision rate.  See
<http://www.isthe.com/chongo/tech/comp/fnv/index.html>
for more details as well as other forms of the FNV hash.
Comments, questions, bug fixes and suggestions welcome at
the address given in the above URL.


# FNV hash utility

Two hash utilities (32 bit and 64 bit) are provided:

```
fnv1a64 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [-v] [arg ...]
fnv1a32 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [-v] [arg ...]

fnv164 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [-v] [arg ...]
fnv132 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [-v] [arg ...]

fnv064 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [-v] [arg ...]
fnv032 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [-v] [arg ...]

-h         print help and exit
-v         verbose mode, print arg after hash (implies -m)
-V         print version and exit

-b bcnt   mask off all but the lower bcnt bits (default: 32)
-m        multiple hashes, one per line for each arg
-s        hash arg as a string (ignoring terminating NUL bytes)
-t code   0 ==> generate test vectors, 1 ==> test FNV hash

arg       string (if -s was given) or filename (default stdin)
```

* The fnv1a32, fnv1a64 implement the recommended FNV-1a hash.

* The fnv132, fnv164 implement the common FNV-1 hash.

* The fnv032, fnv064 implement the historic FNV-0 hash (used to generate FNV-1a and FNV-1 offset basis only).

To test FNV hashes, try:

```sh
/usr/local/bin/fnv1a64 -t 1 -v
/usr/local/bin/fnv1a32 -t 1 -v

/usr/local/bin/fnv164 -t 1 -v
/usr/local/bin/fnv132 -t 1 -v

/usr/local/bin/fnv032 -t 1 -v
/usr/local/bin/fnv064 -t 1 -v
```

If you are compiling, try:

```sh
make check
```


# FNV hash library

The libfnv.a library implements both a 32 bit and a 64 bit FNV hash
on collections of bytes, a NUL terminated strings or on an open file
descriptor.

Here is the 32 bit FNV 1 hash:

```c
Fnv32_t fnv_32_buf(void *buf, int len, Fnv32_t hval);       /* byte buf */
Fnv32_t fnv_32_str(char *string, Fnv32_t hval);             /* string */
```

Here is the 32 bit FNV 1a hash:

```c
Fnv32_t fnv_32a_buf(void *buf, int len, Fnv32_t hval);      /* byte buf */
Fnv32_t fnv_32a_str(char *string, Fnv32_t hval);            /* string */
```

Here is the 64 bit FNV 1 hash:

```c
Fnv64_t fnv_64_buf(void *buf, int len, Fnv64_t hval);       /* byte buf */
Fnv64_t fnv_64_str(char *string, Fnv64_t hval);             /* string */
```

Here is the 64 bit FNV 1a hash:

```c
Fnv64_t fnv_64a_buf(void *buf, int len, Fnv64_t hval);      /* byte buf */
Fnv64_t fnv_64a_str(char *string, Fnv64_t hval);            /* string */
```

On the first call to a hash function, one must supply the initial basis
that is appropriate for the hash in question:

FNV-1a:

```c
FNV1A_32_INIT               /* 32 bit FNV-1a initial basis */
FNV1A_64_INIT               /* 64 bit FNV-1a initial basis */
```

FNV-1:

```c
FNV1_32_INIT                /* 32 bit FNV-1 initial basis */
FNV1_64_INIT                /* 64 bit FNV-1 initial basis */
```

FNV-0:

```c
FNV0_32_INIT                /* 32 bit FNV-0 initial basis */
FNV0_64_INIT                /* 64 bit FNV-0 initial basis */
```

For example to perform a 64 bit FNV-1 hash:

```c
#include "fnv.h"

Fnv64_t hash_val;

hash_val = fnv_64_str("a string", FNV1_64_INIT);
hash_val = fnv_64_str("more string", hash_val);
```

produces the same final hash value as:

```c
hash_val = fnv_64_str("a stringmore string", FNV1_64_INIT);
```

NOTE: If one used `FNV0_64_INIT` instead of `FNV1_64_INIT` one would get the
historic FNV-0 hash instead of the recommended FNV-1a hash.

To perform a 32 bit FNV-1 hash:

```c
#include "fnv.h"

Fnv32_t hash_val;

hash_val = fnv_32_buf(buf, length_of_buf, FNV1_32_INIT);
hash_val = fnv_32_str("more data", hash_val);
```

To perform a 64 bit FNV-1a hash:

```c
#include "fnv.h"

Fnv64_t hash_val;

hash_val = fnv_64a_buf(buf, length_of_buf, FNV1_64_INIT);
hash_val = fnv_64a_str("more data", hash_val);
```


# To use


## FNV-1a 64-bit

Recommended FNV-1a 64-bit hash:

```sh
/usr/local/bin/fnv1a64 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [arg ...]

    -h         print help and exit
    -v         verbose mode, print arg after hash (implies -m)
    -V         print version and exit

    -b bcnt    mask off all but the lower bcnt bits (default 64)
    -m         multiple hashes, one per line for each arg
    -s arg     hash arg as a string (ignoring terminating NUL bytes)
    -t code    test hash code: (0 ==> generate test vectors
                                1 ==> validate against FNV test vectors)

    arg        string (if -s was given) or filename (default stdin)

Exit codes:
    0         all OK
    2         -h and help string printed or -V and version string printed
    3         command line error
    4         error on opening or reading file
 >= 10        test suite error
 >= 20        internal error

fnv1a64 version: 5.0.5 2025-04-13
```


## FNV-1a 32-bit

Recommended FNV-1a 32-bit hash:

```sh
/usr/local/bin/fnv1a32 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [arg ...]

    -h         print help and exit
    -v         verbose mode, print arg after hash (implies -m)
    -V         print version and exit

    -b bcnt    mask off all but the lower bcnt bits (default 32)
    -m         multiple hashes, one per line for each arg
    -s arg     hash arg as a string (ignoring terminating NUL bytes)
    -t code    test hash code: (0 ==> generate test vectors
                                1 ==> validate against FNV test vectors)

    arg        string (if -s was given) or filename (default stdin)

Exit codes:
    0         all OK
    2         -h and help string printed or -V and version string printed
    3         command line error
    4         error on opening or reading file
 >= 10        test suite error
 >= 20        internal error

fnv1a32 version: 5.0.5 2025-04-13
```


## FNV-1 64-bit

Common FNV-1 64-bit hash:

```sh
/usr/local/bin/fnv164 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [arg ...]

    -h         print help and exit
    -v         verbose mode, print arg after hash (implies -m)
    -V         print version and exit

    -b bcnt    mask off all but the lower bcnt bits (default 64)
    -m         multiple hashes, one per line for each arg
    -s arg     hash arg as a string (ignoring terminating NUL bytes)
    -t code    test hash code: (0 ==> generate test vectors
                                1 ==> validate against FNV test vectors)

    arg        string (if -s was given) or filename (default stdin)

Exit codes:
    0         all OK
    2         -h and help string printed or -V and version string printed
    3         command line error
    4         error on opening or reading file
 >= 10        test suite error
 >= 20        internal error

fnv164 version: 5.0.5 2025-04-13
```


## FNV-1 32-bit

Common FNV-1 32-bit hash:

```sh
/usr/local/bin/fnv132 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [arg ...]

    -h         print help and exit
    -v         verbose mode, print arg after hash (implies -m)
    -V         print version and exit

    -b bcnt    mask off all but the lower bcnt bits (default 32)
    -m         multiple hashes, one per line for each arg
    -s arg     hash arg as a string (ignoring terminating NUL bytes)
    -t code    test hash code: (0 ==> generate test vectors
                                1 ==> validate against FNV test vectors)

    arg        string (if -s was given) or filename (default stdin)

Exit codes:
    0         all OK
    2         -h and help string printed or -V and version string printed
    3         command line error
    4         error on opening or reading file
 >= 10        test suite error
 >= 20        internal error

fnv132 version: 5.0.5 2025-04-13
```


## FNV-0 64-bit

Historic FNV-0 64-bit hash (used to generate FNV-1a and FNV-1 offset basis
only):

```sh
/usr/local/bin/fnv064 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [arg ...]

    -h         print help and exit
    -v         verbose mode, print arg after hash (implies -m)
    -V         print version and exit

    -b bcnt    mask off all but the lower bcnt bits (default 64)
    -m         multiple hashes, one per line for each arg
    -s arg     hash arg as a string (ignoring terminating NUL bytes)
    -t code    test hash code: (0 ==> generate test vectors
                                1 ==> validate against FNV test vectors)

    arg        string (if -s was given) or filename (default stdin)

Exit codes:
    0         all OK
    2         -h and help string printed or -V and version string printed
    3         command line error
    4         error on opening or reading file
 >= 10        test suite error
 >= 20        internal error

NOTE: Programs that begin with fnv0 implement the FNV-0 hash.
      The FNV-0 hash is historic FNV algorithm that is now deprecated.

fnv064 version: 5.0.5 2025-04-13
```


## FNV-0 32-bit

Historic FNV-0 32-bit hash (used to generate FNV-1a and FNV-1 offset basis
only):

```sh
/usr/local/bin/fnv032 [-h] [-v] [-V] [-b bcnt] [-m] [-s arg] [-t code] [arg ...]

    -h         print help and exit
    -v         verbose mode, print arg after hash (implies -m)
    -V         print version and exit

    -b bcnt    mask off all but the lower bcnt bits (default 32)
    -m         multiple hashes, one per line for each arg
    -s arg     hash arg as a string (ignoring terminating NUL bytes)
    -t code    test hash code: (0 ==> generate test vectors
                                1 ==> validate against FNV test vectors)

    arg        string (if -s was given) or filename (default stdin)

Exit codes:
    0         all OK
    2         -h and help string printed or -V and version string printed
    3         command line error
    4         error on opening or reading file
 >= 10        test suite error
 >= 20        internal error

NOTE: Programs that begin with fnv0 implement the FNV-0 hash.
      The FNV-0 hash is historic FNV algorithm that is now deprecated.

fnv032 version: 5.0.5 2025-04-13
```


# Reporting Security Issues

To report a security issue, please visit "[Reporting Security Issues](https://github.com/lcn2/fnv/security/policy)".

