#!/bin/make
#
# hash - makefile for hash tools
#
# @(#) $Revision: 2.8 $
# @(#) $Id: Makefile,v 2.8 1999/10/23 09:45:28 chongo Exp chongo $
# @(#) $Source: /usr/local/src/lib/libfnv/RCS/Makefile,v $
#
# See:
#	http://reality.sgi.com/chongo/tech/comp/fnv/index.html
#
# for the most up to date copy of this code and the FNV hash home page.
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

# make tools
#
SHELL= /bin/sh
CFLAGS= -g3
#CFLAGS= -O2 -g3
CC= cc
AR= ar
TAR= tar
EGREP= egrep
GZIP_BIN= gzip
INSTALL= install

# If your system needs ranlib use:
#	RANLIB= ranlib
# otherwise use:
#	RANLIB= :
#
#RANLIB= ranlib
RANLIB= :

# where to install things
#
DESTBIN= /usr/local/bin
DESTLIB= /usr/local/lib
DESTINC= /usr/local/include
# NOTE: Lines with WWW in them are removed from the shipped Makefile
WWW= /usr/local/ns-home/docs/chongo/src/fnv

# what to build
#
SRC= h32.c h64.c have_ulong64.c fnv32.c fnv64.c
HSRC= longlong.h fnv.h
ALL= ${SRC} fnv.h Makefile README
PROGS= fnv32 fnv64
LIBS= libfnv.a
TARGETS= ${LIBS} ${PROGS}

# default rule
#
all: ${TARGETS}

# things to build
#
h32.o: h32.c longlong.h fnv.h
	${CC} ${CFLAGS} h32.c -c

h64.o: h64.c longlong.h fnv.h
	${CC} ${CFLAGS} h64.c -c

libfnv.a: h32.o h64.o
	rm -f $@
	${AR} rv $@ h32.o h64.o
	${RANLIB} $@

fnv32.o: fnv32.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv32.c -c

fnv32: fnv32.o libfnv.a
	${CC} fnv32.o libfnv.a -o fnv32

fnv64.o: fnv64.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv64.c -c

fnv64: fnv64.o libfnv.a
	${CC} fnv64.o libfnv.a -o fnv64

longlong.h: have_ulong64.c Makefile
	-@rm -f have_ulong64 have_ulong64.o ll_tmp longlong.h
	@echo 'forming longlong.h'
	@echo '/*' > longlong.h
	@echo ' * DO NOT EDIT -- generated by the Makefile' >> longlong.h
	@echo ' */' >> longlong.h
	@echo '' >> longlong.h
	@echo '#if !defined(__LONGLONG_H__)' >> longlong.h
	@echo '#define __LONGLONG_H__' >> longlong.h
	@echo '' >> longlong.h
	@echo '/* do we have/want to use a long long type? */' >> longlong.h
	-@rm -f have_ulong64.o have_ulong64
	-@${CC} ${CFLAGS} have_ulong64.c -c 2>/dev/null; true
	-@${CC} ${CFLAGS} have_ulong64.o -o have_ulong64 2>/dev/null; true
	-@${SHELL} -c "./have_ulong64 > ll_tmp 2>/dev/null" \
	    >/dev/null 2>&1; true
	-@if [ -s ll_tmp ]; then \
	    cat ll_tmp >> longlong.h; \
	else \
	    echo '#undef HAVE_64BIT_LONG_LONG	/* no */' >> longlong.h; \
	fi
	@echo '' >> longlong.h
	@echo '#endif /* !__LONGLONG_H__ */' >> longlong.h
	-@rm -f have_ulong64 have_ulong64.o ll_tmp
	@echo 'longlong.h formed'

# utilities
#
install: libfnv.a
	-@if [ -d "${DESTBIN}" ]; then \
	    echo "	mkdir -p ${DESTBIN}"; \
	    mkdir -p ${DESTBIN}; \
	fi
	-@if [ -d "${DESTLIB}" ]; then \
	    echo "	mkdir -p ${DESTLIB}"; \
	    mkdir -p ${DESTLIB}; \
	fi
	-@if [ -d "${DESTINC}" ]; then \
	    echo "	mkdir -p ${DESTINC}"; \
	    mkdir -p ${DESTINC}; \
	fi
	${INSTALL} -m 0755 ${PROGS} ${DESTLIB}
	${INSTALL} -m 0644 ${LIBS} ${DESTLIB}
	${RANLIB} ${DESTLIB}/libfnv.a
	${INSTALL} -m 0644 ${HSRC} ${DESTINC}
	# NOTE: Lines with WWW in them are removed from the shipped Makefile
	-if [ -d ${WWW} ]; then \
	    rm -f Makefile.ship			# WWW; \
	    ${EGREP} -v WWW Makefile > Makefile.ship 	# WWW; \
	    rm -f Makefile.save			# WWW; \
	    ln Makefile Makefile.save		# WWW; \
	    cp -f Makefile.ship Makefile	# WWW; \
	    ${TAR} -cf - ${ALL} | ${GZIP_BIN} --best > fnv_hash.tar.gz # WWW; \
	    ${INSTALL} -m 0644 fnv_hash.tar.gz ${ALL} ${WWW}; \
	    mv -f Makefile.save Makefile	# WWW; \
	    rm -f fnv_hash.tar.gz Makefile.ship	# WWW; \
	fi					# WWW

clean:
	-rm -f have_ulong64 have_ulong64.o ll_tmp longlong.h
	-rm -f h32.o h64.o fnv32.o fnv64.o
	-rm -f fnv_hash.tar.gz Makefile.ship	# WWW

clobber: clean
	-rm -f ${TARGETS}
