#include "merry_helpers.h"

void merry_LITTLE_ENDIAN_to_BIG_ENDIAN(MerryHostMemLayout *le) {
  MerryHostMemLayout be;
  be.bytes.b0 = le->bytes.b7;
  be.bytes.b1 = le->bytes.b6;
  be.bytes.b2 = le->bytes.b5;
  be.bytes.b3 = le->bytes.b4;
  be.bytes.b4 = le->bytes.b3;
  be.bytes.b5 = le->bytes.b2;
  be.bytes.b6 = le->bytes.b1;
  be.bytes.b7 = le->bytes.b0;
  le->whole_word = be.whole_word;
}
