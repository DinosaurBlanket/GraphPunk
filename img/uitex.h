#define uitex_path "img/uitex.bmp"

// in pixels
#define uitex_size_x     512.0f
#define uitex_size_y     512.0f
#define uitex_guSize_px   16.0f

// in normalized texture coordinates
#define uitex_guSize_nt   (uitex_guSize_px/uitex_size_x)

// background
#define uitex_ibord_tl_x  ( 1.0f/uitex_size_x)
#define uitex_ibord_tl_y  (34.0f/uitex_size_y)
#define uitex_ibord_tr_x  ( 2.0f/uitex_size_x)
#define uitex_ibord_tr_y  (34.0f/uitex_size_y)
#define uitex_ibord_br_x  ( 2.0f/uitex_size_x)
#define uitex_ibord_br_y  (33.0f/uitex_size_y)
#define uitex_ibord_bl_x  ( 1.0f/uitex_size_x)
#define uitex_ibord_bl_y  (33.0f/uitex_size_y)
#define uitex_obord_tl_x    0.0f
#define uitex_obord_tl_y  (35.0f/uitex_size_y)
#define uitex_obord_tr_x  ( 3.0f/uitex_size_x)
#define uitex_obord_tr_y  (35.0f/uitex_size_y)
#define uitex_obord_br_x  ( 3.0f/uitex_size_x)
#define uitex_obord_br_y  (32.0f/uitex_size_y)
#define uitex_obord_bl_x    0.0f
#define uitex_obord_bl_y  (32.0f/uitex_size_y)
#define uitex_cntr_bl_x     0.0f
#define uitex_cntr_bl_y     0.0f
#define uitex_cntr_tr_x   (32.0f/uitex_size_x)
#define uitex_cntr_tr_y   (32.0f/uitex_size_y)

// global controls
#define uitex_gc_buttonSide       (uitex_guSize_nt*2.0f)
#define uitex_gc_play_bl_x        (32.0f/uitex_size_x)
#define uitex_gc_play_bl_y          0.0f
#define uitex_gc_step_bl_x        (64.0f/uitex_size_x)
#define uitex_gc_step_bl_y          0.0f
#define uitex_gc_unmuted_bl_x     (96.0f/uitex_size_x)
#define uitex_gc_unmuted_bl_y       0.0f
#define uitex_gc_unsoloed_bl_x   (128.0f/uitex_size_x)
#define uitex_gc_unsoloed_bl_y      0.0f
#define uitex_gc_moveNode_bl_x   (160.0f/uitex_size_x)
#define uitex_gc_moveNode_bl_y     0.0f
#define uitex_gc_unLock_bl_x     (192.0f/uitex_size_x)
#define uitex_gc_unLock_bl_y        0.0f
#define uitex_gc_up_bl_x         (224.0f/uitex_size_x)
#define uitex_gc_up_bl_y            0.0f
#define uitex_gc_top_bl_x        (256.0f/uitex_size_x)
#define uitex_gc_top_bl_y           0.0f
#define uitex_gc_back_bl_x       (288.0f/uitex_size_x)
#define uitex_gc_back_bl_y          0.0f
#define uitex_gc_forward_bl_x    (320.0f/uitex_size_x)
#define uitex_gc_forward_bl_y       0.0f
#define uitex_gc_save_bl_x       (352.0f/uitex_size_x)
#define uitex_gc_save_bl_y          0.0f

// ports
#define uitex_port_num_bl_x       (32.0f/uitex_size_x)
#define uitex_port_num_bl_y       (64.0f/uitex_size_y)
#define uitex_port_numArray_bl_x  (48.0f/uitex_size_x)
#define uitex_port_numArray_bl_y  (64.0f/uitex_size_y)
#define uitex_port_byte_bl_x      (64.0f/uitex_size_x)
#define uitex_port_byte_bl_y      (64.0f/uitex_size_y)
#define uitex_port_byteArray_bl_x (80.0f/uitex_size_x)
#define uitex_port_byteArray_bl_y (64.0f/uitex_size_y)

// atom faces
#define uitex_aface_add_bl_x     ( 96.0f/uitex_size_x)
#define uitex_aface_add_bl_y     ( 64.0f/uitex_size_y)
#define uitex_aface_add_tr_x     (128.0f/uitex_size_x)
#define uitex_aface_add_tr_y     ( 80.0f/uitex_size_y)
#define uitex_aface_sub_bl_x     (128.0f/uitex_size_x)
#define uitex_aface_sub_bl_y     ( 64.0f/uitex_size_y)
#define uitex_aface_sub_tr_x     (160.0f/uitex_size_x)
#define uitex_aface_sub_tr_y     ( 80.0f/uitex_size_y)
#define uitex_aface_mul_bl_x     (160.0f/uitex_size_x)
#define uitex_aface_mul_bl_y     ( 64.0f/uitex_size_y)
#define uitex_aface_mul_tr_x     (192.0f/uitex_size_x)
#define uitex_aface_mul_tr_y     ( 80.0f/uitex_size_y)
#define uitex_aface_div_bl_x     (192.0f/uitex_size_x)
#define uitex_aface_div_bl_y     ( 64.0f/uitex_size_y)
#define uitex_aface_div_tr_x     (224.0f/uitex_size_x)
#define uitex_aface_div_tr_y     ( 80.0f/uitex_size_y)
