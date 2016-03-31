
#include <stdint.h>
#include "module.h"

uint32_t planeVertDataSize(plane *p) {
  return (backVertsSize + p->lineVertsCap + p->nodeVertsCap)*sizeof(float);
}
uint32_t planeElemDataSize(plane *p) {
  return (backElemsSize + 3*((p->lineVertsCap + p->nodeVertsCap)/2))*sizeof(uint32_t);
}
