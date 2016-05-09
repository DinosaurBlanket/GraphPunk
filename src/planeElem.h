#pragma once

#include nodeDef.h

// every planeElem is a just a rectangle in vertData

typedef struct {
  uint8_t  pei;         // plane element id
  uint8_t  inletCount;  // cached here despite mirroring node definition
  uint16_t nid;         // node id
  uint32_t _unused32_1;
} nodeBase;
// the types, and perhaps other data are looked up from the node id

typedef struct {
  uint8_t  pei;
  uint8_t  type;         // data type
  uint8_t  index;        // 0 is leftmost inlet,1 is second leftmost, etc.
  uint8_t  _unused8_3;
  uint32_t conode;       // offset into planeElems of connected node
} inlet;
// There is always one line per inlet, but any number per outlet.
// Moving a node will require looking through all inlets
// to find lines connected to the node being moved.

typedef struct {
  uint8_t  pei;
  uint8_t  width; // this many additional planeElems will follow
  uint8_t  base;       // 2 to 15
  uint8_t  _unused8_3;
  float    value;
} numLit;


typedef enum {
  nmric_negative = 16,
  nmric_radix,
  nmric_base10,
  nmric_base12,
  nmric_base16
} numerics;
typedef struct {
  uint8_t  pei;
  uint8_t  _unused8_1;
  uint16_t _unused16_1;
  uint8_t  value; // numeral value (0-15), or numerics value
  //uint8_t  _unused8_5;
  //uint16_t _unused16_3;
} numeric;


typedef enum {
  pei_nface,    // node face, followed by inlets, if any
  pei_inlet,    // always follows a nodeface or it's lef-adjacent inlet
  pei_numLit,   // followed by numerals
  pei_numeric   // follows anything that displays a number
} planeElemId;
typedef union {
  planeElemId pei;
  nodeFace    nface;
  inlet       inlet;
  numLit      numLit;
  numeric     numeric;
} planeElem;
