#include "misc.h"

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}

#define memMinCap 4

void memAppend(mem *m, uint32_t size) {
  if (m->cap < memMinCap) m->cap = memMinCap;
  m->size += size;
  if (m->size <= m->cap) return;
  while (m->size > m->cap) m->cap *= 2;
  if (!m->data) m->data = malloc(m->size);
  else m->data = realloc(m->data, m->size);
}

void memFree(mem *m) {
  free(m->data);
  m->size = 0;
  m->cap  = 0;
}
