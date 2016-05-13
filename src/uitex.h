#pragma once

#define uitex_path "img/uitex.bmp"

#define uitex_clearR 0.1f
#define uitex_clearG 0.1f
#define uitex_clearB 0.1f
#define uitex_clearA 1.0f


// in pixels
#define uitex_size_x  512.0f
#define uitex_size_y  512.0f

// global controls
extern const float uitex_gcRect[4];


#define uitex_borderColor_x  0.0f
#define uitex_borderColor_y 64.0f

#define uitex_portW 16.0f
#define uitex_portH  8.0f
extern const float uitex_inletRects[];

#define uitex_nmrlW  16.0f
#define uitex_nmrlH  16.0f
#define uitex_numLitBackcolor_x  0.0f
#define uitex_numLitBackcolor_y 64.0f
const float uitex_radixPointRect[4];
const float uitex_negSignRect[4];
const float uitex_nmrlRects[4];
void uitex_nmrlRect(float rect[4], const uint32_t n);
const float uitex_base10SignRect[4];
const float uitex_base12SignRect[4];
const float uitex_base16SignRect[4];

extern const float uitex_nodeFaces[];
