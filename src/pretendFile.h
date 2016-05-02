
#include "nodeDef.h"

//0 output
//1   add  : 200
//2     sub  : 50
//3       100
//4       50
//5     mul  : 150
//6       25
//7       div  : 6
//8         18
//9         3



programFileHeader pretendProgramFileHeader = {43};

nodeDataOnDisk pretendData[] = { // sub 20 from line number for index
  {.n = nid_output},
  {.p =   0.0},
  {.p =   0.0},
  {.c =   4},
  {.n = nid_add},
  {.p =   0.0},
  {.p =   0.0},
  {.c =   9},
  {.c =  22},
  {.n = nid_sub},
  {.p =   0.0},
  {.p =   0.0},
  {.c =  14},
  {.c =  18},
  {.n = nid_numlit7},
  {.p =   0.0},
  {.p =   0.0},
  {.v = 100.0},
  {.n = nid_numlit7},
  {.p =   0.0},
  {.p =   0.0},
  {.v =  50.0},
  {.n = nid_mul},
  {.p =   0.0},
  {.p =   0.0},
  {.c =  27},
  {.c =  31},
  {.n = nid_numlit7},
  {.p =   0.0},
  {.p =   0.0},
  {.v =  25.0},
  {.n = nid_div},
  {.p =   0.0},
  {.p =   0.0},
  {.c =  36},
  {.c =  40},
  {.n = nid_numlit7},
  {.p =   0.0},
  {.p =   0.0},
  {.v =  18.0},
  {.n = nid_numlit7},
  {.p =   0.0},
  {.p =   0.0},
  {.v =   3.0}
};
