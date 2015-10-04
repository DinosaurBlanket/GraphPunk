#pragma once

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "scrollable.hpp"

struct vertex {
  vec2     pos;
  uint32_t color;
};
typedef uint16_t indx_t ;

void initMap(scrollable &scroll, vec2 videoSize);

void drawMap(scrollable &scroll);



