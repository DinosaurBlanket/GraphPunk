#pragma once

#define  GLEW_STATIC
#include <GL/glew.h>
#include "misc.h"

typedef enum {
  aid_add,
  aid_sub,
  aid_mul,
  aid_div,
  aid_count
} atomIds;
#define rootModId 5000

typedef enum {
  pei_iport,
  pei_oport,
  pei_aface,
  pei_mface,
} planeElemId;
typedef enum {dt_n, dt_b, dt_na, dt_ba} dtype;

typedef struct {
  planeElemId    pei;
  uint8_t        oputCount; // oports are in the same planeElem array following this node
  uint8_t        iputCount; // iports follow the oports
  uint32_t       lineVerts; // offset into plane's vert data, 1 line per iput, both ends of line
  uint32_t       id;        // if id > rootModId, it's a moduleId
  struct module *module;    // this will NULL for all atoms
} vinode;
// a node's ports always follow the node in the planeElems array

typedef struct {
  planeElemId pei;
  dtype       type;
  uint8_t     pos;  // 0 is leftmost port, 1 is 2nd leftmost port, etc.
  uint32_t    node; // offset into planeElems array
  uint32_t    ocon; // for iport, planeElem offset of connected oport, otheriwse 0
} port;

typedef union {
  planeElemId pei;
  port        p;
  vinode      n;
} planeElem;

typedef struct {
  float      rect[4];         // borders of plane
  float      pos[2];          // only used when changing planes
  planeElem *planeElems;
  uint32_t   planeElemCount;
  uint32_t   planeElemCap;
  float     *vertData;        // GL buffer storage, 16 floats(4 verts) per planeElem
  uint32_t  *indxData;        // GL buffer storage,  6 ints per planeElem
  uint32_t   nodeVertsOffset; // float count
  GLuint     vao;
  GLuint     vbo;
  GLuint     ebo;
} plane;
// first verts of every plane are for background
#define backVertsSize 48 // floats, 12 vertices
#define backElemsSize 36 // uint32s
#define initialNodeVertsOffset 256 // floats, leaving that - backVertsSize for lineverts
// background verts are followed by line verts

// Module faces are drawn separately
typedef struct {
  bool paused;
  bool audioSoloed;
  bool audioDisabled;
  bool videoDisabled;
  bool frozen;
} moduleToggles; // for root module, these are all always false
typedef struct {
  uint32_t        id;
  struct module  *parent;
  float           faceRect[4]; // for root module, unused, all 0
  moduleToggles   toggles;
  plane           plane;
  // exnodes
  // specialNodes
  // dataNode data
  // frozen data
} module;
