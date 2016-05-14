#pragma once

#include <stdint.h>
#define  GLEW_STATIC
#include <GL/glew.h>

#define fingerUnit  16 // pixels

typedef enum {
  nid_add,
  nid_sub,
  nid_mul,
  nid_div,
  nid_output,
  nid_numlit_b10w08,
  nodeIdCount
} nodeIds;

typedef enum {dt_num, dt_arr, dt_par, dt_tex, dt_noOut} dtype;
typedef double num_t;
typedef struct {float   *data; uint32_t w; uint32_t h; uint16_t d;} arr_t;
typedef struct {uint8_t *data; uint32_t w; uint32_t h; uint16_t d;} par_t;
typedef struct {GLuint   tex;  uint32_t w; uint32_t h;} tex_t; // in pixels


typedef union {
  uint32_t       n; // nodeId
  float          p; // x, y positions, always follow nodeId
  uint32_t       c; // planeElem index of child nodeId, one per inlet, follows y position
  float          v; // literal number value, follows y position of numlit
} nodeDataOnDisk;
#define ndodChildStart 3
#define ndodNumLitValStart 3

typedef struct {
  uint32_t  nodeDataCount;
  uint32_t  planeElemCount;
} programFileHeader;


#define maxInletCount 28

typedef struct {
  uint16_t size[2];      // in pixels
  uint8_t  extraPECount; // extra planeElem count
  uint8_t  ndodCount;    // jump to next nodeId in nodeDataOnDisk array
  uint8_t  outType;
  uint8_t  inletCount;
  uint8_t  inTypes[maxInletCount];
  uint8_t  inletPos[maxInletCount]; // from left, in inlet widths
} nodeDef;
// face size should be taken from uitex_nodeFaces

void getNodeDef(nodeDef *def, uint32_t nodeId);
