#!/bin/make
#
# hash - makefile for hash tools
#
# @(#) $Revision$
# @(#) $Id$
# @(#) $Source$
#
# Copyright (c) 1997 by Landon Curt Noll.  All Rights Reserved.
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright, this permission notice and text
# this comment, and the disclaimer below appear in all of the following:
#
#       supporting documentation
#       source copies
#       source works derived from this source
#       binaries derived from this source or from derived source
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.


SHELL= /bin/sh
CFLAGS= -O2 -g3
CC= cc
AR= ar
#RANLIB= ranlib
RANLIB= :
LOOK= look
INSTALL=/sbin/install

# where hash.h will be installed
DESTINC= /usr/local/include

# where hash.a will be installed
DESTLIB= /usr/local/lib

TARGETS= hash.a h64.o

all: ${TARGETS}

hash.a: hash.o
	rm -f hash.a
	${AR} rv hash.a hash.o
	${RANLIB} hash.a

install: hash.a hash.h
	rm -f ${DESTLIB}/hash.a
	${INSTALL} -c -m 0644 hash.a ${DESTLIB}
	${RANLIB} ${DESTLIB}/hash.a
	rm -f ${DESTINC}/hash.h
	${INSTALL} -c -m 0644 hash.h ${DESTINC}

clean:
	-rm -f hash.o

clobber: clean
	-rm -f ${TARGETS}
