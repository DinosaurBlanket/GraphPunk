

typedef enum {
  pei_iport,
  pei_oport,
  pei_aface,
  pei_mface,
  pei_mhandle
} planeElemId;
typedef enum {dt_n, dt_b, dt_na, dt_ba} dtype;

typedef enum {
  aid_add,
  aid_sub,
  aid_mul,
  aid_div,
  aid_count
} atomIDs;
#define atomIdModuleIdBoundary 4000


typedef struct {
  planeElemId    pei;
  uint8_t        iputCount; // ports are in the same planeElem array following this node
  uint8_t        oputCount; // ports are in the same planeElem array following this node
  uint32_t       id;        // if id > atomIdModuleIdBoundary, it's a moduleId
  struct module *module;    // if NULL, it's not a module
} vinode;
// a node's ports always follow the node in the planeElems array

typedef struct {
  planeElemId pei;
  dtype       type;
  uint32_t    node; // offset into plane's planeElems array
  uint8_t     pos;  // 0 is leftmost port, 1 is 2nd leftmost port, etc.
} portBase;
typedef struct {portBase b;} oport;
typedef struct {
  portBase b;
  uint32_t lineVerts; // offset into plane's vert data, both ends of line
  uint32_t oput;      // offset into plane's planeElems
} iport;

typedef union {oport o; iport i; vinode n; planeElemId pei;} planeElem;

typedef struct {
  float      rect[4];
  float      pos[2];        // only used when changing planes
  planeElem *planeElems;    // malloced, parallel with vertData
  uint32_t   planeElemCount;
  uint32_t   planeElemCap;
  GLuint     vao;
  GLuint     vbo;
  GLuint     ebo;
  float     *vertData;
  uint32_t  *indxData;
  uint32_t   lineVertsSize; // in elements (floats)
  uint32_t   lineVertsCap;  // in elements (floats)
  uint32_t   nodeVertsSize; // in elements (ints), a rect for each planeElem
  uint32_t   nodeVertsCap;  // in elements (ints), a rect for each planeElem
} plane;
// first verts of every plane are for background
#define backVertsSize 48 // in elements, 12 vertices, 48 floats
#define backElemsSize 36 // in elements (ints)
uint32_t planeVertDataSize(plane *p) {
  return (backVertsSize + p->lineVertsCap + p->nodeVertsCap)*sizeof(float);
}
uint32_t planeElemDataSize(plane *p) {
  return (backElemsSize + 3*((p->lineVertsCap + p->nodeVertsCap)/2))*sizeof(uint32_t);
}
// Module faces are drawn separately
typedef struct {
  uint32_t        moduleId;
  struct module  *parent;
  plane           plane;
  //exnode *exnodes;
  // *specialNodes;
} module;
