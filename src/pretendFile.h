
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
  .nodeDataCount  = 44,
  .planeElemCount = 69
};

const nodeDataOnDisk pretendData[] = { // sub 20 from line number for index
  {.n = nid_output},  // 0,1
  {.p =    0.0},
  {.p =  200.0},
  {.c =    2},        // 2
  {.n = nid_add},     // 3,4
  {.p =    0.0},
  {.p =  150.0},
  {.c =    9},        // 5
  {.c =   26},        // 6
  {.n = nid_sub},     // 7,8
  {.p = -140.0},
  {.p =   80.0},
  {.c =    8},         // 9
  {.c =   17},         // 10
  {.n = nid_numlit_b10w08}, // 11 - 20
  {.p =  -300.0},
  {.p =     0.0},
  {.v =  100.0},
  {.n = nid_numlit_b10w08}, // 21 - 30
  {.p = -140.0},
  {.p =    0.0},
  {.v =   50.0},
  {.n = nid_mul},     // 31,32
  {.p =  140.0},
  {.p =   80.0},
  {.c =  27},         // 33
  {.c =  36},         // 34
  {.n = nid_numlit_b10w08}, // 35 - 44
  {.p =   40.0},
  {.p =    0.0},
  {.v =  25.0},
  {.n = nid_div},     // 45,46
  {.p = 200.0},
  {.p =   0.0},
  {.c =  37},         // 47
  {.c =  46},         // 48
  {.n = nid_numlit_b10w08}, // 49 - 58
  {.p =   60.0},
  {.p = -100.0},
  {.v =  18.0},
  {.n = nid_numlit_b10w08}, // 59 - 68
  {.p =  250.0},
  {.p = -100.0},
  {.v =   3.0}
};
