#pragma once

#define uitex_path "img/uitex.bmp"

// in pixels
#define uitex_size_x  512.0f
#define uitex_size_y  512.0f

// in normalized texture coordinates
// global controls
extern const float uitex_gcRect[4];


#define uitex_portW (16.0f/uitex_size_x)
#define uitex_portH  (8.0f/uitex_size_y)
extern const float uitex_inletRects[];

#define uitex_nmrlW (16.0f/uitex_size_x)
#define uitex_nmrlH (16.0f/uitex_size_y)
#define uitex_numLitBackcolor_x  0.0f
#define uitex_numLitBackcolor_y  (64.0f/uitex_size_y)
const float uitex_radixPointRect[4];
const float uitex_negSignRect[4];
const float uitex_nmrlRects[4];
inline void uitex_nmrlRect(float rect[4], const uint32_t n);
const float uitex_base10SignRect[4];
const float uitex_base12SignRect[4];
const float uitex_base16SignRect[4];

extern const float uitex_nodeFaces[];
