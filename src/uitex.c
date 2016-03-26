#include "uitex.h"

// this array must be parallel to atomIDs enum (4 floats per atom, xyxy)
const float uitex_atomFaces[] = {
   96.0f/uitex_size_x, 64.0f/uitex_size_y, 128.0f/uitex_size_x, 80.0f/uitex_size_y, // add
  128.0f/uitex_size_x, 64.0f/uitex_size_y, 160.0f/uitex_size_x, 80.0f/uitex_size_y, // sub
  160.0f/uitex_size_x, 64.0f/uitex_size_y, 192.0f/uitex_size_x, 80.0f/uitex_size_y, // mul
  192.0f/uitex_size_x, 64.0f/uitex_size_y, 224.0f/uitex_size_x, 80.0f/uitex_size_y  // div
};
