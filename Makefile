#!/bin/make
#
# hash - makefile for hash tools

SHELL= /bin/sh
CFLAGS= -O2 -g3
CC= cc
AR= ar
#RANLIB= ranlib
RANLIB= :
LOOK= look
INSTALL=/usr/bin/install

# where hash.h will be installed
DESTINC= /usr/local/include

# where hash.a will be installed
DESTLIB= /usr/local/lib

# where the dictionary is found
#WORDS= /usr/share/lib/dict/words
WORDS= ./Message.ID

TRIALS= fowler.vo1 fowler.vo2 fowler.noll.vo
TESTS= ${TRIALS} besthash standalone
TARGETS= hash.a

all: ${TARGETS}

hash.a: hash.o
	rm -f hash.a
	${AR} rv hash.a hash.o
	${RANLIB} hash.a

test: ${TESTS} words #standalone stand

words: ${WORDS}
	-@for i in ${TRIALS}; do \
	    echo "$$i < ${WORDS}"; \
	    ./$$i < ${WORDS}; \
	done

stand: standalone
	grep '^ya' ${WORDS} | ./standalone

standalone: hash.c hash.h
	${CC} ${CFLAGS} hash.c -DFOWLER_NOLL_VO -DSTANDALONE -o standalone

fowler.vo1: hasheval.c
	${CC} ${CFLAGS} hasheval.c -DFOWLER_VO1 -o fowler.vo1

fowler.vo2: hasheval.c
	${CC} ${CFLAGS} hasheval.c -DFOWLER_VO2 -o fowler.vo2

fowler.noll.vo: hasheval.c
	${CC} ${CFLAGS} hasheval.c -DFOWLER_NOLL_VO -o fowler.noll.vo

besthash: besthash.sh
	rm -f besthash
	cp -p besthash.sh besthash
	chmod +x besthash

install: hash.a hash.h
	rm -f ${DESTLIB}/hash.a
	${INSTALL} -c -m 0644 hash.a ${DESTLIB}
	${RANLIB} ${DESTLIB}/hash.a
	rm -f ${DESTINC}/hash.h
	${INSTALL} -c -m 0644 hash.h ${DESTINC}

clean:
	-rm -f hash.o hasheval.o

clobber: clean
	-rm -f ${TARGETS} ${TESTS}
