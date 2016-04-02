#pragma once
#include <stdint.h>
#include <stdbool.h>

#define fr(i, bound) for (int i = 0; i < (bound); i++)
bool allEq(const float *l, const float *r, int c);


typedef struct {
  void    *data;
  uint32_t size; // bytes initialized
  uint32_t cap;  // bytes allocated
} mem;

void memAppend(mem *m, uint32_t size);
void memFree(mem *m);
