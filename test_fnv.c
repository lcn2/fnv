/*
 * test_fnv - FNV test suite
 *
 * @(#) $Revision: 1.5 $
 * @(#) $Id: fnv32.c,v 1.5 2001/05/30 15:34:30 chongo Exp chongo $
 * @(#) $Source: /usr/local/src/cmd/fnv/RCS/fnv32.c,v $
 *
 ***
 *
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 ***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *	chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!	:-)
 */

#include <stdio.h>
#include "fnv.h"

#define LEN(x) (sizeof(x)-1)
/* TEST macro does not include trailing NUL byte in the test vector */
#define TEST(x) {x, LEN(x)}
/* TEST0 macro includes the trailing NUL byte in the test vector */
#define TEST0(x) {x, sizeof(x)}
/* REPEAT500 - repeat a string 500 times */
#define R500(x) R100(x)R100(x)R100(x)R100(x)R100(x)
#define R100(x) R10(x)R10(x)R10(x)R10(x)R10(x)R10(x)R10(x)R10(x)R10(x)R10(x)
#define R10(x) x x x x x x x x x x

/*
 * FNV test vectors
 */
struct test_vector test[] = {
  TEST(""), TEST0(""),
  TEST("a"), TEST0("a"),
  TEST("b"), TEST0("b"),
  TEST("c"), TEST0("c"),
  TEST("\xff\x00\x00\x01"), TEST("\x01\x00\x00\xff"),
  TEST("\xff\x00\x00\x02"), TEST("\x02\x00\x00\xff"),
  TEST("\xff\x00\x00\x03"), TEST("\x03\x00\x00\xff"),
  TEST("\xff\x00\x00\x04"), TEST("\x04\x00\x00\xff"),
  TEST("\x40\x51\x4e\x44"), TEST("\x44\x4e\x51\x40"),
  TEST("\x40\x51\x4e\x4a"), TEST("\x4a\x4e\x51\x40"),
  TEST("\x40\x51\x4e\x54"), TEST("\x54\x4e\x51\x40"),
  TEST("127.0.0.1"), TEST0("127.0.0.1"),
  TEST("127.0.0.2"), TEST0("127.0.0.2"),
  TEST("127.0.0.3"), TEST0("127.0.0.3"),
  TEST("64.81.78.68"), TEST0("64.81.78.68"),
  TEST("64.81.78.74"), TEST0("64.81.78.74"),
  TEST("64.81.78.84"), TEST0("64.81.78.84"),
  TEST("chongo"), TEST0("chongo"),
  TEST("feedface"), TEST0("feedface"),
  TEST("feedfacedaffdeed"), TEST0("feedfacedaffdeed"),
  TEST("feedfacedeadbeef"), TEST0("feedfacedeadbeef"),
  TEST("chongo was here"), TEST0("chongo was here"),
  TEST("chongo <Landon Curt Noll> /\\../\\"),
  TEST0("chongo <Landon Curt Noll> /\\../\\"),
  TEST("chongo (Landon Curt Noll) /\\../\\"),
  TEST0("chongo (Landon Curt Noll) /\\../\\"),
  TEST("http://antwrp.gsfc.nasa.gov/apod/astropix.html"),
  TEST("http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash"),
  TEST("http://epod.usra.edu/"),
  TEST("http://exoplanet.eu/"),
  TEST("http://hvo.wr.usgs.gov/cam3/"),
  TEST("http://hvo.wr.usgs.gov/kilauea/update/deformation.html"),
  TEST("http://hvo.wr.usgs.gov/kilauea/update/images.html"),
  TEST("http://hvo.wr.usgs.gov/kilauea/update/maps.html"),
  TEST("http://hvo.wr.usgs.gov/volcanowatch/current_issue.html"),
  TEST("http://neo.jpl.nasa.gov/risk/"),
  TEST("http://norvig.com/21-days.html"),
  TEST("http://primes.utm.edu/curios/home.php"),
  TEST("http://slashdot.org/"),
  TEST("http://tux.wr.usgs.gov/Maps/155.25-19.5.html"),
  TEST("http://volcano.wr.usgs.gov/kilaueastatus.php"),
  TEST("http://www.avo.alaska.edu/activity/Redoubt.php"),
  TEST("http://www.dilbert.com/fast/"),
  TEST("http://www.fourmilab.ch/gravitation/orbits/"),
  TEST("http://www.fpoa.net/"),
  TEST("http://www.ioccc.org/index.html"),
  TEST("http://www.isthe.com/cgi-bin/number.cgi"),
  TEST("http://www.isthe.com/chongo/bio.html"),
  TEST("http://www.isthe.com/chongo/index.html"),
  TEST("http://www.isthe.com/chongo/src/calc/lucas-calc"),
  TEST("http://www.isthe.com/chongo/tech/astro/venus2004.html"),
  TEST("http://www.isthe.com/chongo/tech/astro/vita.html"),
  TEST("http://www.isthe.com/chongo/tech/comp/c/expert.html"),
  TEST("http://www.isthe.com/chongo/tech/comp/calc/index.html"),
  TEST("http://www.isthe.com/chongo/tech/comp/fnv/index.html"),
  TEST("http://www.isthe.com/chongo/tech/math/number/howhigh.html"),
  TEST("http://www.isthe.com/chongo/tech/math/number/number.html"),
  TEST("http://www.isthe.com/chongo/tech/math/prime/mersenne.html"),
  TEST("http://www.isthe.com/chongo/tech/math/prime/mersenne.html#largest"),
  TEST("http://www.lavarnd.org/cgi-bin/corpspeak.cgi"),
  TEST("http://www.lavarnd.org/cgi-bin/haiku.cgi"),
  TEST("http://www.lavarnd.org/cgi-bin/rand-none.cgi"),
  TEST("http://www.lavarnd.org/cgi-bin/randdist.cgi"),
  TEST("http://www.lavarnd.org/index.html"),
  TEST("http://www.lavarnd.org/what/nist-test.html"),
  TEST("http://www.macosxhints.com/"),
  TEST("http://www.mellis.com/"),
  TEST("http://www.nature.nps.gov/air/webcams/parks/havoso2alert/havoalert.cfm"),
  TEST("http://www.nature.nps.gov/air/webcams/parks/havoso2alert/timelines_24.cfm"),
  TEST("http://www.paulnoll.com/"),
  TEST("http://www.pepysdiary.com/"),
  TEST("http://www.sciencenews.org/index/home/activity/view"),
  TEST("http://www.skyandtelescope.com/"),
  TEST("http://www.sput.nl/~rob/sirius.html"),
  TEST("http://www.systemexperts.com/"),
  TEST("http://www.tq-international.com/phpBB3/index.php"),
  TEST("http://www.travelquesttours.com/index.htm"),
  TEST("http://www.wunderground.com/global/stations/89606.html"),
  TEST(R10("21701")),
  TEST(R10("M21701")),
  TEST(R10("2^21701-1")),
  TEST(R10("\x54\xc5")),
  TEST(R10("\xc5\x54")),
  TEST(R10("23209")),
  TEST(R10("M23209")),
  TEST(R10("2^23209-1")),
  TEST(R10("\x5a\xa9")),
  TEST(R10("\xa9\x5a")),
  TEST(R10("391581216093")),
  TEST(R10("391581*2^216093-1")),
  TEST(R10("\x05\xf9\x9d\x03\x4c\x81")),
  TEST(R10("FEDCBA9876543210")),
  TEST(R10("\xfe\xdc\xba\x98\x76\x54\x32\x10")),
  TEST(R10("EFCDAB8967452301")),
  TEST(R10("\xef\xcd\xab\x89\x67\x45\x23\x01")),
  TEST(R10("0123456789ABCDEF")),
  TEST(R10("\x01\x23\x45\x67\x89\xab\xcd\xef")),
  TEST(R10("1032547698BADCFE")),
  TEST(R10("\x10\x32\x54\x76\x98\xba\xdc\xfe")),
  TEST(R500("\x00")),
  TEST(R500("\x07")),
  TEST(R500("~")),
  TEST(R500("\x7f")),
  {NULL, 0}	/* MUST BE LAST */
};
