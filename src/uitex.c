#include "uitex.h"

// this array must be parallel to nodeIds enum (4 floats per node, xyxy)
const float uitex_nodeFaces[] = {
   64.0f/uitex_size_x, 64.0f/uitex_size_y,  96.0f/uitex_size_x, 80.0f/uitex_size_y, // add
   96.0f/uitex_size_x, 64.0f/uitex_size_y, 128.0f/uitex_size_x, 80.0f/uitex_size_y, // sub
  128.0f/uitex_size_x, 64.0f/uitex_size_y, 160.0f/uitex_size_x, 80.0f/uitex_size_y, // mul
  160.0f/uitex_size_x, 64.0f/uitex_size_y, 192.0f/uitex_size_x, 80.0f/uitex_size_y  // div
  192.0f/uitex_size_x, 64.0f/uitex_size_y, 224.0f/uitex_size_x, 80.0f/uitex_size_y  // output
};

// this array must be parallel to dtype enum (4 floats per inlet, xyxy)
const float uitex_inletRects[] = {
  0.0f,               72.0f/uitex_size_y, 16.0f/uitex_size_x, 80.0f/uitex_size_y, // num
  16.0f/uitex_size_x, 72.0f/uitex_size_y, 32.0f/uitex_size_x, 80.0f/uitex_size_y, // array
  32.0f/uitex_size_x, 72.0f/uitex_size_y, 48.0f/uitex_size_x, 80.0f/uitex_size_y  // packed array
  48.0f/uitex_size_x, 72.0f/uitex_size_y, 64.0f/uitex_size_x, 80.0f/uitex_size_y  // tex
}
