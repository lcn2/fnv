#!/bin/make
#
# hash - makefile for hash tools

SHELL= /bin/sh
CFLAGS= -O
CC= gcc

TARGETS= fowler.vo1 fowler.vo2 fowler.noll.vo

all: ${TARGETS}

test: words web2

words: /usr/dict/words
	-@for i in ${TARGETS}; do \
	    echo "$$i < /usr/dict/words"; \
	    $$i < /usr/dict/words; \
	done

web2: /usr/share/dict/web2
	-@for i in ${TARGETS}; do \
	    echo "$$i < /usr/share/dict/web2"; \
	    $$i < /usr/share/dict/web2; \
	done

fowler.vo1: hasheval.c
	${CC} ${CFLAGS} hasheval.c -DFOWLER_VO1 -o fowler.vo1

fowler.vo2: hasheval.c
	${CC} ${CFLAGS} hasheval.c -DFOWLER_VO2 -o fowler.vo2

fowler.noll.vo: hasheval.c
	${CC} ${CFLAGS} hasheval.c -DFOWLER_NOLL_VO -o fowler.noll.vo

clean:
	-rm -f *.o

clobber: clean
	-rm -f ${TARGETS}
