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
// inlets
#define uitex_inlet_num_bl_x      0.0f
#define uitex_inlet_num_bl_y    (72.0f/uitex_size_y)
#define uitex_inlet_num_tr_x    (16.0f/uitex_size_x)
#define uitex_inlet_num_tr_y    (80.0f/uitex_size_y)
#define uitex_inlet_array_bl_x  (16.0f/uitex_size_x)
#define uitex_inlet_array_bl_y  (72.0f/uitex_size_y)
#define uitex_inlet_array_tr_x  (32.0f/uitex_size_x)
#define uitex_inlet_array_tr_y  (80.0f/uitex_size_y)
#define uitex_inlet_tex_bl_x    (32.0f/uitex_size_x)
#define uitex_inlet_tex_bl_y    (72.0f/uitex_size_y)
#define uitex_inlet_tex_tr_x    (48.0f/uitex_size_x)
#define uitex_inlet_tex_tr_y    (80.0f/uitex_size_y)

extern const float uitex_atomFaces[];
