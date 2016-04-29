#pragma once

typedef enum {
  nid_output,
  nid_add,
  nid_sub,
  nid_mul,
  nid_div,
  nid_numlit,
  nodeIdCount
} nodeIds;

typedef enum {dt_num, dt_arr, dt_par, dt_tex } dtype;
typedef num_t double
typedef struct {float   *data, uint32_t w, uint32_t h, uint16_t d} arr_t;
typedef struct {uint8_t *data, uint32_t w, uint32_t h, uint16_t d} par_t;
typedef struct {GLuint   tex,  uint32_t w, uint32_t h} dt_tex; // in pixels


typedef union {
  uint32_t       n; // nodeId
  float          x; // x position, always follows nodeId
  float          y; // y position, always follows x position
  uint32_t       c; // index of child nodeId, one per inlet, follows y position
  float          v; // literal number value, follows y position of numlit
} nodeDataOnDisk;

typedef struct {
  uint32_t  nodeDataCount;
} programFileHeader;


#define maxInletCount 30

typedef struct {
  uint8_t  outType;
  uint8_t  inletCount;
  uint8_t  inTypes[maxInletCount];
} nodeDef;
// face size should be taken from uitex_nodeFaces

void getNodeDef(nodeDef *def, nodeId id) {
  switch(id) {
    case nid_add:
      def.outType    = dt_num;
      def.inletCount = 2;
      def.inTypes[0] = dt_num;
      def.inTypes[1] = dt_num;
      return;
    default:
      
  }
}
