
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



const programFileHeader pretendProgramFileHeader = {
  .nodeDataCount  = 43,
  .planeElemCount = 58
};

const nodeDataOnDisk pretendData[] = { // sub 20 from line number for index
  {.n = nid_output},  // 0
  {.p =   0.0},
  {.p =   0.0},
  {.c =   2},         // 1
  {.n = nid_add},     // 2
  {.p =   0.0},
  {.p =   0.0},
  {.c =   9},         // 3
  {.c =  26},         // 4
  {.n = nid_sub},     // 5
  {.p =   0.0},
  {.p =   0.0},
  {.c =   8},         // 6
  {.c =  17},         // 7
  {.n = nid_numlit_b10w08}, // 8 - 16
  {.p =   0.0},
  {.p =   0.0},
  {.v = 100.0},
  {.n = nid_numlit_b10w08}, // 17 - 25
  {.p =   0.0},
  {.p =   0.0},
  {.v =  50.0},
  {.n = nid_mul},     // 26
  {.p =   0.0},
  {.p =   0.0},
  {.c =  27},         // 27
  {.c =  36},         // 28
  {.n = nid_numlit_b10w08}, // 29 - 37
  {.p =   0.0},
  {.p =   0.0},
  {.v =  25.0},
  {.n = nid_div},     // 38
  {.p =   0.0},
  {.p =   0.0},
  {.c =  37},         // 39
  {.c =  46},         // 40
  {.n = nid_numlit_b10w08}, // 41 - 49
  {.p =   0.0},
  {.p =   0.0},
  {.v =  18.0},
  {.n = nid_numlit_b10w08}, // 50 - 58
  {.p =   0.0},
  {.p =   0.0},
  {.v =   3.0}
};
