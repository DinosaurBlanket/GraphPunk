#pragma once
#include <stdint.h>
#include <stdbool.h>

#define fr(i, bound) for (int i = 0; i < (bound); i++)
bool allEq(const float *l, const float *r, int c);

uint32_t nextHighestPO2(const uint32_t n);
