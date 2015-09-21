#pragma once

#include <glm/glm.hpp>

void view_map_init(glm::vec4 gridRect);
void view_map_scrollZoom(glm::vec3 pos);
void view_map_resizeMap();
void view_map_addPatch();
void view_map_remPatch();
void view_map_repPatch();
void view_map_draw();
