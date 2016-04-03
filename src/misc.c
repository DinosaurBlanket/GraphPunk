#include "misc.h"

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}

uint32_t nextHighestPO2(const uint32_t n) {
  const uint32_t max = 0xffffffff;
  for (uint32_t i = 1; i <= max; i <<= 1) {
    if (i > n) return i;
  }
  return max;
}
