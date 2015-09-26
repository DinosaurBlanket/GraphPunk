#pragma once

#include "GL_util.hpp"

void view_map_init(vec4 gridRect, vec2 screenSize, float gridUnit);
void view_map_scrollZoom(vec3 pos);
void view_map_resizeMap();
void view_map_addPatch();
void view_map_remPatch();
void view_map_repPatch();
void view_map_draw();
void displayGCB();
