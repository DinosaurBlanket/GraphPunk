#pragma once

#include <glm/vec2.hpp>
using glm::vec2;

void view_map_init();
void view_map_scroll(
  float cursPress,
  float pCursPress,
  vec2  cursPos,
  vec2  pCursPos
);
void view_map_resizeMap();
void view_map_addPatch();
void view_map_remPatch();
void view_map_repPatch();
void view_map_draw();
void displayGCB();
