#pragma once

#define  GLEW_STATIC
#include <GL/glew.h>
#include "misc.h"

typedef enum {
  nid_add,
  nid_sub,
  nid_mul,
  nid_div,
  nodeIdCount
} nodeIds;

typedef enum {dt_num, dt_arr, dt_tex} dtype;

typedef struct {
  uint8_t  pei;         // plane element id
  uint8_t  inletCount;  // cached here despite mirroring node definition
  uint16_t nid;         // node id
  uint32_t _unused32_;
} nodeFace;

typedef struct {
  uint8_t  pei;
  uint8_t  type;         // data type
  uint8_t  index;        // 0 is leftmost inlet,1 is second leftmost, etc.
  uint8_t  _unused8_;
  uint32_t conode;       // offset into planeElems of connected node
} inlet;
// There is always one line per inlet, but any number per outlet.
// Moving a node will require looking through all inlets
// to find lines connected to the node being moved.

typedef struct {
  uint8_t  pei;
  uint8_t  numerCount; // this many numerals will follow in planeElem array
  uint8_t  base;       // 2 to 15
  uint8_t  _unused8_;
  float    value;
} numLit;

typedef struct {
  uint8_t  pei;
  uint8_t  value; // 0 to 15
  //uint16_t _unused16_;
  //uint32_t _unused32_;
} numeral;


typedef enum {
  pei_nface,    // node face, followed by inlets, if any
  pei_inlet,    // always follows a nodeface or it's lef-adjacent inlet
  pei_numLit,   // followed by numerals
  pei_numeral   // follows anything that displays a number
} planeElemId;
typedef union {
  planeElemId pei;
  nodeFace    nface;
  inlet       inlet;
  numLit      numLit;
  numeral     numeral;
} planeElem;
