#include <stdint.h>
#include "uitex.h"

const float uitex_gcRect[4] = {0, 0, 320, 64};


// this array must be parallel to dtype enum (4 floats per inlet, xyxy)
const float uitex_inletRects[] = {
   0, 72, 16, 80, // num
  16, 72, 32, 80, // array
  32, 72, 48, 80, // packed array
  48, 72, 64, 80  // tex
};

const float uitex_radixPointRect[4] = { 64, 64,  80, 80};
const float uitex_negSignRect[4]    = { 80, 64,  96, 80};
const float uitex_nmrlRects[4]      = { 96, 64, 352, 80};
const float uitex_base10SignRect[4] = {352, 64, 368, 80};
const float uitex_base12SignRect[4] = {368, 64, 384, 80};
const float uitex_base16SignRect[4] = {384, 64, 400, 80};
inline void uitex_nmrlRect(float rect[4], const uint32_t n) {
  rect[0] = uitex_nmrlRects[0] + uitex_nmrlW*n;
  rect[1] = uitex_nmrlRects[1];
  rect[2] = uitex_nmrlRects[2] + uitex_nmrlW*n + uitex_nmrlW;
  rect[3] = uitex_nmrlRects[3];
}

// this array must be parallel to nodeIds enum (4 floats per node, xyxy)
const float uitex_nodeFaces[] = {
    0, 80,  32, 96, // add
   32, 80,  64, 96, // sub
   64, 80,  96, 96, // mul
   96, 80, 128, 96, // div
  128, 80, 160, 96  // output
};
