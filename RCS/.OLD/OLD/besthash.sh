#!/bin/sh
# $Id$ 
#
# besthash - look for the best Vo/Fowler/Noll hash multipler

/usr/games/primes 16777216 | while read prime; do
    ./fowler.noll.vo $prime < /usr/share/lib/dict/words
    sync
done
