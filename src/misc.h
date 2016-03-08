
#define fr(i, bound) for (int i = 0; i < (bound); i++)
bool allEq(const float *l, const float *r, int c);

typedef struct {float x; float y; float s; float t;} uiVert;
typedef struct {
  GLuint    vbo;
  uint32_t  vCount; // number of vertex elements
  uint32_t  vCap;   // maximum number of elements the buffer can hold
  GLuint    ibo;
  uint32_t  iCount;
  uint32_t  iCap;
} vertGroup;
void drawVertGroup(vertGroup *vg);


typedef struct {
  float       corners_gu4[4]; // xyxy, bottom left and top right
  float       pos_gudc2[2];
  vertGroup   vg;
  //uiVert     *lineVertData
  //uiVert     *nodeVertData
  //int       depth; // in module tree
  //vinode *vinodes;
} plane;

typedef struct {
  plane p;
  //exnode *exnodes;
  // *specialNodes;
} module;

void resetPlaneCorners(plane *pln, float halfVideoSize_gu2[2]);
void initPlane(plane *pln, float halfVideoSize_gu2[2]);

// global control stuff
typedef struct {
  bool paused;
  bool auMuted;
  bool auSoloed;
  bool moveBranch;
  bool locked;
  bool Saving;
  vertGroup vg;
} GlblCtrls;
