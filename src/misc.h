
#define fr(i, bound) for (int i = 0; i < (bound); i++)
bool allEq(const float *l, const float *r, int c);

typedef struct {float x; float y; float s; float t;} uiVert;

#define lineVertOffset 12 // first verts of every plane are for background
#define lineElemOffset 36 // first verts of every plane are for background
typedef struct {
  float    corners_gu4[4]; // xyxy, bottom left and top right
  float    pos_gudc2[2];
  GLuint   vao;
  GLuint   vbo;
  GLuint   ebo;
  uint32_t lineCount;
  uint32_t lineCap;
  uint32_t nodeCount;
  uint32_t nodeCap;
  //int    depth; // in module tree
  //vinode *vinodes; // vinodes keep track of their data location in vao
} plane;
void printVerts(const uiVert *verts, int count);

typedef struct {
  plane p;
  //exnode *exnodes;
  // *specialNodes;
} module;

void mapTexRectToVerts(
  uiVert     *destVerts,
  const float destCorners_gu[4], // grid units
  const float srcCorners_nt[4]   // normalized texture coordinates
);

void resetPlaneCorners(plane *pln, float halfVideoSize_gu2[2]);
void initPlane(
  plane *pln,
  GLuint shader,
  GLuint tex,
  float  halfVideoSize_gu2[2]
);
//void setRectElems(uint32_t *elems, uint32_t rectCount);
