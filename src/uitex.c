#include "uitex.h"

const float uitex_gcRect[4] = {0, 0, 320.0/uitex_size_x, 64.0/uitex_size_y};


// this array must be parallel to dtype enum (4 floats per inlet, xyxy)
const float uitex_inletRects[] = {
  0.0,               72.0/uitex_size_y, 16.0/uitex_size_x, 80.0/uitex_size_y, // num
  16.0/uitex_size_x, 72.0/uitex_size_y, 32.0/uitex_size_x, 80.0/uitex_size_y, // array
  32.0/uitex_size_x, 72.0/uitex_size_y, 48.0/uitex_size_x, 80.0/uitex_size_y, // packed array
  48.0/uitex_size_x, 72.0/uitex_size_y, 64.0/uitex_size_x, 80.0/uitex_size_y  // tex
};

const float uitex_radixPointRect[4] = {
  64.0/uitex_size_x, 64.0/uitex_size_y,
  80.0/uitex_size_x, 80.0/uitex_size_y
};
const float uitex_negSignRect[4] {
  80.0/uitex_size_x, 64.0/uitex_size_y,
  96.0/uitex_size_x, 80.0/uitex_size_y
};
const float uitex_nmrlRects[4] {
   96.0/uitex_size_x, 64.0/uitex_size_y,
  352.0/uitex_size_x, 80.0/uitex_size_y
};
const float uitex_base10SignRect[4] {
  352.0/uitex_size_x, 64.0/uitex_size_y,
  368.0/uitex_size_x, 80.0/uitex_size_y
};
const float uitex_base12SignRect[4] {
  368.0/uitex_size_x, 64.0/uitex_size_y,
  384.0/uitex_size_x, 80.0/uitex_size_y
};
const float uitex_base16SignRect[4] {
  384.0/uitex_size_x, 64.0/uitex_size_y,
  400.0/uitex_size_x, 80.0/uitex_size_y
};
inline void uitex_nmrlRect(float rect[4], const uint32_t n) {
  rect[0] = uitex_nmrlRect[0] + uitex_nmrlW*n;
  rect[1] = uitex_nmrlRect[1];
  rect[2] = uitex_nmrlRect[2] + uitex_nmrlW*n + uitex_nmrlW;
  rect[3] = uitex_nmrlRect[3];
}


// this array must be parallel to nodeIds enum (4 floats per node, xyxy)
const float uitex_nodeFaces[] = {
    0.0/uitex_size_x, 80.0/uitex_size_y,  32.0/uitex_size_x, 96.0/uitex_size_y, // add
   32.0/uitex_size_x, 80.0/uitex_size_y,  64.0/uitex_size_x, 96.0/uitex_size_y, // sub
   64.0/uitex_size_x, 80.0/uitex_size_y,  96.0/uitex_size_x, 96.0/uitex_size_y, // mul
   96.0/uitex_size_x, 80.0/uitex_size_y, 128.0/uitex_size_x, 96.0/uitex_size_y, // div
  128.0/uitex_size_x, 80.0/uitex_size_y, 160.0/uitex_size_x, 96.0/uitex_size_y  // output
};
