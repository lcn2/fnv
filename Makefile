#!/bin/make
#
# hash - makefile for hash tools
#
# @(#) $Revision: 3.5 $
# @(#) $Id: Makefile,v 3.5 1999/10/23 13:14:40 chongo Exp chongo $
# @(#) $Source: /usr/local/src/cmd/fnv/RCS/Makefile,v $
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
#CFLAGS= -g3
CFLAGS= -O2 -g3
#CFLAGS= -g3 -DZERO_BASED
#CFLAGS= -O2 -g3 -DZERO_BASED
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
SRC= h_32.c h_64.c h1_32.c h1_64.c have_ulong64.c \
	fnv_32.c fnv_64.c fnv1_32.c fnv1_64.c
HSRC= longlong.h fnv.h
ALL= ${SRC} fnv.h Makefile README
PROGS= fnv_32 fnv_64 fnv1_32 fnv1_64 fnv132 fnv164 fnv32 fnv64
LIBS= libfnv.a
TARGETS= ${LIBS} ${PROGS}

# default rule
#
all: ${TARGETS}

# things to build
#
h_32.o: h_32.c longlong.h fnv.h
	${CC} ${CFLAGS} h_32.c -c

h_64.o: h_64.c longlong.h fnv.h
	${CC} ${CFLAGS} h_64.c -c

h1_32.o: h1_32.c longlong.h fnv.h
	${CC} ${CFLAGS} h1_32.c -c

h1_64.o: h1_64.c longlong.h fnv.h
	${CC} ${CFLAGS} h1_64.c -c

libfnv.a: h_32.o h_64.o h1_32.o h1_64.o
	rm -f $@
	${AR} rv $@ h_32.o h_64.o h1_32.o h1_64.o
	${RANLIB} $@

fnv_32.o: fnv_32.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv_32.c -c

fnv_32: fnv_32.o libfnv.a
	${CC} fnv_32.o libfnv.a -o fnv_32

fnv_64.o: fnv_64.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv_64.c -c

fnv_64: fnv_64.o libfnv.a
	${CC} fnv_64.o libfnv.a -o fnv_64

fnv1_32.o: fnv1_32.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv1_32.c -c

fnv1_32: fnv1_32.o libfnv.a
	${CC} fnv1_32.o libfnv.a -o fnv1_32

fnv1_64.o: fnv1_64.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv1_64.c -c

fnv1_64: fnv1_64.o libfnv.a
	${CC} fnv1_64.o libfnv.a -o fnv1_64

fnv132: fnv1_32
	-rm -f $@
	-cp -f $? $@

fnv164: fnv1_64
	-rm -f $@
	-cp -f $? $@

fnv32: fnv_32
	-rm -f $@
	-cp -f $? $@

fnv64: fnv_64
	-rm -f $@
	-cp -f $? $@

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
	${INSTALL} -m 0755 ${PROGS} ${DESTBIN}
	${INSTALL} -m 0644 ${LIBS} ${DESTLIB}
	${RANLIB} ${DESTLIB}/libfnv.a
	${INSTALL} -m 0644 ${HSRC} ${DESTINC}
	# NOTE: Lines with WWW in them are removed from the shipped Makefile
	-if [ -d ${WWW} ]; then \
	    rm -f Makefile.ship;			: WWW; \
	    ${EGREP} -v WWW Makefile > Makefile.ship; 	: WWW; \
	    rm -f Makefile.save;			: WWW; \
	    ln Makefile Makefile.save;			: WWW; \
	    cp -f Makefile.ship Makefile;		: WWW; \
	    ${TAR} -cf - ${ALL} | ${GZIP_BIN} --best > fnv_hash.tar.gz;: WWW; \
	    ${INSTALL} -m 0644 fnv_hash.tar.gz ${ALL} ${WWW}; \
	    mv -f Makefile.save Makefile;		: WWW; \
	    rm -f fnv_hash.tar.gz Makefile.ship;	: WWW; \
	fi;						: WWW

clean:
	-rm -f have_ulong64 have_ulong64.o ll_tmp ll_tmp2 longlong.h
	-rm -f h_32.o h_64.o fnv_32.o fnv_64.o 
	-rm -f h1_32.o h1_64.o fnv1_32.o fnv1_64.o
	-rm -f fnv_hash.tar.gz Makefile.ship	# WWW

clobber: clean
	-rm -f ${TARGETS}

check: all
	@echo "	These tests should not print an un-indented hash value:"
	@echo
	@echo "	fnv_32 -s hi == 0x6977223c"
	-@./fnv_32 -s hi | ${EGREP} -v 0x6977223c; /bin/true
	@echo "	fnv_32 -s hello == 0xb6fa7167"
	-@./fnv_32 -s hello | ${EGREP} -v 0xb6fa7167; /bin/true
	@echo "	fnv_32 -s curds and whey == 0x08ebf912"
	-@./fnv_32 -s curds and whey | ${EGREP} -v 0x08ebf912; /bin/true
	@echo "	fnv_64 -s hi == 0x08326007b4eb2b9c"
	-@./fnv_64 -s hi | ${EGREP} -v 0x08326007b4eb2b9c; /bin/true
	@echo "	fnv_64 -s hello == 0x7b495389bdbdd4c7"
	-@./fnv_64 -s hello | ${EGREP} -v 0x7b495389bdbdd4c7; /bin/true
	@echo "	fnv_64 -s curds and whey == 0x795ad7b2d9a7dc72"
	-@./fnv_64 -s curds and whey | ${EGREP} -v 0x795ad7b2d9a7dc72; /bin/true
	-@rm -f ll_tmp ll_tmp2
	-@echo line 1 > ll_tmp
	-@echo line 2 >> ll_tmp
	-@echo line 3 >> ll_tmp
	@echo "	fnv_32 < ll_tmp == 0xb8cbeb33"
	-@./fnv_32 < ll_tmp | ${EGREP} -v 0xb8cbeb33; /bin/true
	@echo "	fnv_64 < ll_tmp == 0x79dcad190c3291b3"
	-@./fnv_64 < ll_tmp | ${EGREP} -v 0x79dcad190c3291b3; /bin/true
	@echo "	fnv_32 ll_tmp == 0xb8cbeb33"
	-@./fnv_32 ll_tmp | ${EGREP} -v 0xb8cbeb33; /bin/true
	@echo "	fnv_64 ll_tmp == 0x79dcad190c3291b3"
	-@./fnv_64 ll_tmp | ${EGREP} -v 0x79dcad190c3291b3; /bin/true
	@echo "	fnv_32 ll_tmp ll_tmp == 0x08ae22e5"
	-@./fnv_32 ll_tmp ll_tmp | ${EGREP} -v 0x08ae22e5; /bin/true
	@echo "	fnv_64 ll_tmp ll_tmp == 0x10a587a0ff68f305"
	-@./fnv_64 ll_tmp ll_tmp | ${EGREP} -v 0x10a587a0ff68f305; /bin/true
	-@dd if=/dev/zero bs=1024k count=4 of=ll_tmp2 >/dev/null 2>&1
	@echo "	fnv_32 ll_tmp ll_tmp2 == 0x23cbeb33"
	-@./fnv_32 ll_tmp ll_tmp2 | ${EGREP} -v 0x23cbeb33; /bin/true
	@echo "	fnv_64 ll_tmp ll_tmp2 == 0xcafae19b7f3291b3"
	-@./fnv_64 ll_tmp ll_tmp2 | ${EGREP} -v 0xcafae19b7f3291b3; /bin/true
	@echo "	fnv_32 ll_tmp2 ll_tmp2 == 0x1b1c9dc5"
	-@./fnv_32 ll_tmp2 ll_tmp2 | ${EGREP} -v 0x1b1c9dc5; /bin/true
	@echo "	fnv_64 ll_tmp2 ll_tmp2 == 0x637a2df54e222325"
	-@./fnv_64 ll_tmp2 ll_tmp2 | ${EGREP} -v 0x637a2df54e222325; /bin/true
	-@rm -f ll_tmp ll_tmp2
	@echo
	@echo "	All done!"
