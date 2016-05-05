#pragma once

#define uitex_path "img/uitex.bmp"

// in pixels
#define uitex_size_x  512.0f
#define uitex_size_y  512.0f

// in normalized texture coordinates
// global controls
#define uitex_gc_bl_x             0.0f
#define uitex_gc_bl_y             0.0f
#define uitex_gc_tr_x          (320.0f/uitex_size_x)
#define uitex_gc_tr_y           (64.0f/uitex_size_y)


extern const float uitex_atomFaces[];
extern const float uitex_inletRects[];
