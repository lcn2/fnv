#!/bin/make
#
# hash - makefile for FNV hash tools
#
# @(#) $Revision: 5.1 $
# @(#) $Id: Makefile,v 5.1 2009/06/30 09:01:38 chongo Exp $
# @(#) $Source: /usr/local/src/cmd/fnv/RCS/Makefile,v $
#
# See:
#	http://www.isthe.com/chongo/tech/comp/fnv/index.html
#
# for the most up to date copy of this code and the FNV hash home page.
#
# Please do not copyright this code.  This code is in the public domain.
#
# LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
# EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# By:
#	chongo <Landon Curt Noll> /\oo/\
#	http://www.isthe.com/chongo/
#
# Share and Enjoy!	:-)

# make tools
#
SHELL= /bin/sh
CFLAGS= -O3 -g3
#CFLAGS= -O2 -g3
#CC= cc
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
WWW= /usr/local/src/cmd/fnv/NOTES/www

# what to build
#
SRC=	hash_32.c hash_32a.c hash_64.c hash_64a.c \
	fnv32.c fnv64.c \
	have_ulong64.c test_fnv.c
NO64BIT_SRC= no64bit_fnv64.c no64bit_hash_64.c \
	no64bit_hash_64a.c no64bit_test_fnv.c
HSRC=	fnv.h \
	longlong.h
ALL=	${SRC} ${HSRC} \
	README Makefile
PROGS=	fnv032 fnv064 fnv132 fnv164 fnv1a32 fnv1a64
OBSOLETE_PROGS=	fnv0_32 fnv0_64 fnv1_32 fnv1_64 fnv1a_32 fnv1a_64
NO64BIT_PROGS= no64bit_fnv064 no64bit_fnv164 no64bit_fnv1a64
LIBS=	libfnv.a
LIBOBJ=	hash_32.o hash_64.o hash_32a.o hash_64a.o test_fnv.o
NO64BIT_OBJ= no64bit_fnv64.o no64bit_hash_64.o \
	no64bit_hash_64a.o no64bit_test_fnv.o
OTHEROBJ= fnv32.o fnv64.o
TARGETS= ${LIBOBJ} ${LIBS} ${PROGS}

# default rule
#
all: ${TARGETS}

# things to build
#
hash_32.o: hash_32.c longlong.h fnv.h
	${CC} ${CFLAGS} hash_32.c -c

hash_64.o: hash_64.c longlong.h fnv.h
	${CC} ${CFLAGS} hash_64.c -c

hash_32a.o: hash_32a.c longlong.h fnv.h
	${CC} ${CFLAGS} hash_32a.c -c

hash_64a.o: hash_64a.c longlong.h fnv.h
	${CC} ${CFLAGS} hash_64a.c -c

test_fnv.o: test_fnv.c longlong.h fnv.h
	${CC} ${CFLAGS} test_fnv.c -c

fnv32.o: fnv32.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv32.c -c

fnv032: fnv32.o libfnv.a
	${CC} fnv32.o libfnv.a -o fnv032

fnv64.o: fnv64.c longlong.h fnv.h
	${CC} ${CFLAGS} fnv64.c -c

fnv064: fnv64.o libfnv.a
	${CC} fnv64.o libfnv.a -o fnv064

libfnv.a: ${LIBOBJ}
	rm -f $@
	${AR} rv $@ ${LIBOBJ}
	${RANLIB} $@

fnv132: fnv032
	-rm -f $@
	-cp -f $? $@

fnv1a32: fnv032
	-rm -f $@
	-cp -f $? $@

fnv164: fnv064
	-rm -f $@
	-cp -f $? $@

fnv1a64: fnv064
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
	@echo '/*' >> longlong.h
	@echo ' * NO64BIT_LONG_LONG undef HAVE_64BIT_LONG_LONG' >> longlong.h
	@echo ' */' >> longlong.h
	@echo '#if defined(NO64BIT_LONG_LONG)' >> longlong.h
	@echo '#undef HAVE_64BIT_LONG_LONG' >> longlong.h
	@echo '#endif /* NO64BIT_LONG_LONG */' >> longlong.h
	@echo '' >> longlong.h
	@echo '#endif /* !__LONGLONG_H__ */' >> longlong.h
	-@rm -f have_ulong64 have_ulong64.o ll_tmp
	@echo 'longlong.h formed'

# utilities
#
install: all
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
	@# NOTE: Lines with WWW in them are removed from the shipped Makefile
	-@if [ -d ${WWW} ]; then \
	    echo "rm -f Makefile.ship";			: WWW; \
	    rm -f Makefile.ship;			: WWW; \
	    echo "${EGREP} -v WWW Makefile > Makefile.ship"; 	: WWW; \
	    ${EGREP} -v WWW Makefile > Makefile.ship; 	: WWW; \
	    echo "rm -f Makefile.save";			: WWW; \
	    rm -f Makefile.save;			: WWW; \
	    echo "ln Makefile Makefile.save";		: WWW; \
	    ln Makefile Makefile.save;			: WWW; \
	    echo "cp -f Makefile.ship Makefile";	: WWW; \
	    cp -f Makefile.ship Makefile;		: WWW; \
	    echo "${TAR} -cf - ${ALL} | ${GZIP_BIN} --best > fnv_hash.tar.gz";: WWW; \
	    ${TAR} -cf - ${ALL} | ${GZIP_BIN} --best > fnv_hash.tar.gz;: WWW; \
	    echo "${INSTALL} -m 0644 fnv_hash.tar.gz ${ALL} ${WWW}"; \
	    ${INSTALL} -m 0644 fnv_hash.tar.gz ${ALL} ${WWW}; \
	    echo "mv -f Makefile.save Makefile";	: WWW; \
	    mv -f Makefile.save Makefile;		: WWW; \
	    echo "rm -f fnv_hash.tar.gz Makefile.ship";	: WWW; \
	    rm -f fnv_hash.tar.gz Makefile.ship;	: WWW; \
	fi;						: WWW
	@# remove osolete programs
	for i in ${OBSOLETE_PROGS}; do \
	    if [ -f "${DESTBIN}/$$i" ]; then \
	        echo "rm -f ${DESTBIN}/$$i"; \
		rm -f "${DESTBIN}/$$i"; \
	    fi; \
	done

clean:
	-rm -f have_ulong64 have_ulong64.o ll_tmp ll_tmp2 longlong.h
	-rm -f ${LIBOBJ}
	-rm -f ${OTHEROBJ}
	-rm -f fnv_hash.tar.gz Makefile.ship	# WWW

clobber: clean
	-rm -f ${TARGETS}
	-rm -f ${OBSOLETE_PROGS} lltmp lltmp2 ll_tmp
	-rm -f ${NO64BIT_SRC}
	-rm -f ${NO64BIT_OBJ}
	-rm -f ${NO64BIT_PROGS}
	-rm -f vector.c

check: ${PROGS}
	@echo -n "FNV-0 32 bit tests: "
	@./fnv032 -t 1 -v
	@echo -n "FNV-1 32 bit tests: "
	@./fnv132 -t 1 -v
	@echo -n "FNV-1a 32 bit tests: "
	@./fnv1a32 -t 1 -v
	@echo -n "FNV-0 64 bit tests: "
	@./fnv064 -t 1 -v
	@echo -n "FNV-1 64 bit tests: "
	@./fnv164 -t 1 -v
	@echo -n "FNV-1a 64 bit tests: "
	@./fnv1a64 -t 1 -v

###############################
# generate test vector source #
###############################

no64bit_fnv64.c: fnv64.c
	-rm -f $@
	-cp -f $? $@

no64bit_hash_64.c: hash_64.c
	-rm -f $@
	-cp -f $? $@

no64bit_hash_64a.c: hash_64a.c
	-rm -f $@
	-cp -f $? $@

no64bit_test_fnv.c: test_fnv.c
	-rm -f $@
	-cp -f $? $@

no64bit_fnv64.o: no64bit_fnv64.c longlong.h fnv.h
	${CC} ${CFLAGS} -DNO64BIT_LONG_LONG no64bit_fnv64.c -c

no64bit_hash_64.o: no64bit_hash_64.c longlong.h fnv.h
	${CC} ${CFLAGS} -DNO64BIT_LONG_LONG no64bit_hash_64.c -c

no64bit_hash_64a.o: no64bit_hash_64a.c longlong.h fnv.h
	${CC} ${CFLAGS} -DNO64BIT_LONG_LONG no64bit_hash_64a.c -c

no64bit_test_fnv.o: no64bit_test_fnv.c longlong.h fnv.h
	${CC} ${CFLAGS} -DNO64BIT_LONG_LONG no64bit_test_fnv.c -c

no64bit_fnv064: no64bit_fnv64.o no64bit_hash_64.o \
		no64bit_hash_64a.o no64bit_test_fnv.o
	${CC} ${CFLAGS} no64bit_fnv64.o no64bit_hash_64.o \
		        no64bit_hash_64a.o no64bit_test_fnv.o -o $@

no64bit_fnv164: no64bit_fnv064
	-rm -f $@
	-cp -f $? $@

no64bit_fnv1a64: no64bit_fnv064
	-rm -f $@
	-cp -f $? $@

vector.c: ${PROGS} ${NO64BIT_PROGS}
	-rm -f $@
	echo '/* start of output generated by make $@ */' >> $@
	echo '' >> $@
	#@
	echo '/* FNV-0 32 bit test vectors */' >> $@
	./fnv032 -t 0 >> $@
	echo '' >> $@
	#@
	echo '/* FNV-1 32 bit test vectors */' >> $@
	./fnv132 -t 0 >> $@
	echo '' >> $@
	#@
	echo '/* FNV-1a 32 bit test vectors */' >> $@
	./fnv1a32 -t 0 >> $@
	echo '' >> $@
	#@
	echo '/* FNV-0 64 bit test vectors */' >> $@
	echo '#if defined(HAVE_64BIT_LONG_LONG)' >> $@
	./fnv064 -t 0 >> $@
	echo '#else /* HAVE_64BIT_LONG_LONG */' >> $@
	./no64bit_fnv064 -t 0 >> $@
	echo '#endif /* HAVE_64BIT_LONG_LONG */' >> $@
	echo '' >> $@
	#@
	echo '/* FNV-1 64 bit test vectors */' >> $@
	echo '#if defined(HAVE_64BIT_LONG_LONG)' >> $@
	./fnv164 -t 0 >> $@
	echo '#else /* HAVE_64BIT_LONG_LONG */' >> $@
	./no64bit_fnv164 -t 0 >> $@
	echo '#endif /* HAVE_64BIT_LONG_LONG */' >> $@
	echo '' >> $@
	#@
	echo '/* FNV-1a 64 bit test vectors */' >> $@
	echo '#if defined(HAVE_64BIT_LONG_LONG)' >> $@
	./fnv1a64 -t 0 >> $@
	echo '#else /* HAVE_64BIT_LONG_LONG */' >> $@
	./no64bit_fnv1a64 -t 0 >> $@
	echo '#endif /* HAVE_64BIT_LONG_LONG */' >> $@
	echo '' >> $@
	#@
	echo '/* end of output generated by make $@ */' >> $@
