
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "error.h"
#include "oglTools.h"
#include "../img/uitex.h"
#include "timestamp.h"


#define fr(i, bound) for (int i = 0; i < (bound); i++)

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}



typedef struct {float x; float y; float s; float t;} uiVert;
typedef struct {
  GLuint    vbo;
  uint32_t  vCount; // number of vertex elements
  uint32_t  vCap;   // maximum number of elements the buffer can hold
  GLuint    ibo;
  uint32_t  iCount;
  uint32_t  iCap;
} vertGroup;
void drawVertGroup(vertGroup *vg) {
  glBindBuffer(GL_ARRAY_BUFFER,         vg->vbo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vg->ibo);_glec
  glDrawElements(GL_TRIANGLES, vg->iCount, GL_UNSIGNED_INT, 0);_glec
}

typedef struct {
  float       corners_gu4[4];
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

void putUnitSquareVerts(
  uiVert     *dest,
  const float bl_xy_gu[2], // grid units
  const float bl_st_nt[2], // normalized texture coordinates
  float       texOffset_nt
) {
  // tl
  dest[0].x = bl_xy_gu[0];
  dest[0].y = bl_xy_gu[1] + 1.0;
  dest[0].s = bl_st_nt[0];
  dest[0].t = bl_st_nt[1] + texOffset_nt;
  // tr
  dest[1].x = bl_xy_gu[0] + 1.0;
  dest[1].y = bl_xy_gu[1] + 1.0;
  dest[1].s = bl_st_nt[0] + texOffset_nt;
  dest[1].t = bl_st_nt[1] + texOffset_nt;
  // br
  dest[2].x = bl_xy_gu[0] + 1.0;
  dest[2].y = bl_xy_gu[1];
  dest[2].s = bl_st_nt[0] + texOffset_nt;
  dest[2].t = bl_st_nt[1];
  // bl
  dest[3].x = bl_xy_gu[0];
  dest[3].y = bl_xy_gu[1];
  dest[3].s = bl_st_nt[0];
  dest[3].t = bl_st_nt[1];
}

const float planePadding_gu = 12;

void resetPlaneCorners(plane *pln, float halfVideoSize_gu2[2]) {
  pln->corners_gu4[0] = floor(-halfVideoSize_gu2[0] - planePadding_gu); // tl
  pln->corners_gu4[1] = ceil ( halfVideoSize_gu2[1] + planePadding_gu); // tr
  pln->corners_gu4[2] = ceil ( halfVideoSize_gu2[0] + planePadding_gu); // bl
  pln->corners_gu4[3] = floor(-halfVideoSize_gu2[1] - planePadding_gu); // br
  uiVert backVerts[8] = {
    // inside border
    {    // 0 tl
      pln->corners_gu4[0]+1, pln->corners_gu4[1]-1, 
      uitex_ibord_tl_x, uitex_ibord_tl_y
    }, { // 1 tr
      pln->corners_gu4[2]-1, pln->corners_gu4[1]-1, 
      uitex_ibord_tr_x, uitex_ibord_tr_y
    }, { // 2 br
      pln->corners_gu4[2]-1, pln->corners_gu4[3]+1, 
      uitex_ibord_br_x, uitex_ibord_br_y
    }, { // 3 bl
      pln->corners_gu4[0]+1, pln->corners_gu4[3]+1, 
      uitex_ibord_bl_x, uitex_ibord_bl_y
    },
    // outside border
    {    // 4 tl
      pln->corners_gu4[0], pln->corners_gu4[1], 
      uitex_obord_tl_x, uitex_obord_tl_y
    }, { // 5 tr
      pln->corners_gu4[2], pln->corners_gu4[1], 
      uitex_obord_tr_x, uitex_obord_tr_y
    }, { // 6 br
      pln->corners_gu4[2], pln->corners_gu4[3], 
      uitex_obord_br_x, uitex_obord_br_y
    }, { // 7 bl
      pln->corners_gu4[0], pln->corners_gu4[3], 
      uitex_obord_bl_x, uitex_obord_bl_y
    }
  };
  glBindBuffer(GL_ARRAY_BUFFER, pln->vg.vbo);_glec
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(backVerts), backVerts);_glec
}

void initPlane(plane *pln, float halfVideoSize_gu2[2]) {
  pln->vg.vCap = 128;
  pln->vg.iCap = pln->vg.vCap/2;
  glGenBuffers(1, &pln->vg.vbo);_glec
  glGenBuffers(1, &pln->vg.ibo);_glec
  glBindBuffer(GL_ARRAY_BUFFER,         pln->vg.vbo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pln->vg.ibo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,               // GLenum        target
    pln->vg.vCap*sizeof(uiVert),   // GLsizeiptr    size
    0,                             // const GLvoid *data
    GL_STATIC_DRAW                 // GLenum        usage​
  );_glec
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    pln->vg.iCap*sizeof(uint32_t),
    0,
    GL_STATIC_DRAW
  );_glec
  
  pln->vg.vCount = 12;
  resetPlaneCorners(pln, halfVideoSize_gu2);
  uiVert centerVerts[4];
  const float centerVerts_bl_xy_gu[2] = {-0.5, -0.5};
  const float centerVerts_bl_st_nt[2] = {uitex_cntr_bl_x, uitex_cntr_bl_y};
  putUnitSquareVerts(
    centerVerts,
    centerVerts_bl_xy_gu,
    centerVerts_bl_st_nt,
    uitex_guSize_nt
  );
  glBufferSubData(
    GL_ARRAY_BUFFER,
    8*sizeof(uiVert),
    sizeof(centerVerts),
    centerVerts
  );_glec
  
  uint32_t backInd[] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  pln->vg.iCount = sizeof(backInd)/sizeof(uint32_t);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(backInd), backInd);_glec
}



// global control stuff
struct gc {
  bool paused;
  bool auMuted;
  bool auSoloed;
  bool moveBranch;
  bool locked;
  bool Saving;
  GLuint   vbo;
  GLuint   ebo;
  uint32_t vertexCount;
  uint32_t indexCount;
};






int main(int argc, char *argv[]) {
  float videoSize_px2[2] = {800, 600}; // pixels
	float gridUnit = 32;                 // pixels
	float unitScale_2[2];
  fr(i,2) {unitScale_2[i] = gridUnit/(videoSize_px2[i]/2.0f);}
  float halfVideoSize_gu2[2];          // grid units
  fr(i,2) {halfVideoSize_gu2[i] = (videoSize_px2[i]/gridUnit)/2.0f;}
  
  
	//initVideo(window, GlContext, videoSize_px2[0], videoSize_px2[1]);
  SDL_Window   *window    = NULL;
  SDL_GLContext GlContext = NULL;
  SDL_Init(SDL_INIT_VIDEO);_sdlec
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  window = SDL_CreateWindow(
    "GraphPunk",               //const char* title,
    SDL_WINDOWPOS_UNDEFINED,   //int         x,
    SDL_WINDOWPOS_UNDEFINED,   //int         y,
    videoSize_px2[0],          //int         w,
    videoSize_px2[1],          //int         h,
    SDL_WINDOW_OPENGL          //Uint32      flags
  );_sdlec
  GlContext = SDL_GL_CreateContext(window);_sdlec
  SDL_GL_SetSwapInterval(ENABLE_VSYNC);_sdlec
  
  glewExperimental = GL_TRUE;
  {
    GLenum r = glewInit();
    if (r != GLEW_OK) {
      printf("GLEW error: %s\n", glewGetErrorString(r));
      exit(1);
    }
    // There's an OpenGL error 1280 here for some reason, just flush it...
    while (glGetError() != GL_NO_ERROR) {};
  }
  #if PRINT_GL_VERSION
  printf("OpenGL version: %s\n\n", glGetString(GL_VERSION));_glec
  #endif
  
  module rootMod = {0};
  plane *curPlane = &rootMod.p;
  initPlane(curPlane, halfVideoSize_gu2);
  
  
  GLuint vao_UI;
  glGenVertexArrays(1, &vao_UI);_glec
  glBindVertexArray(vao_UI);_glec
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  GLuint shaderProgram = createShaderProgram(
    "src/vert.glsl", 
    "src/frag.glsl", 
    "shaderProgram"
  );
  if (!shaderProgram) return __LINE__;
  glUseProgram(shaderProgram);_glec
  
  GLint attr_pos      = glGetAttribLocation(shaderProgram, "pos");_glec
  GLint attr_texCoord = glGetAttribLocation(shaderProgram, "texCoord");_glec
  glEnableVertexAttribArray(attr_pos  );_glec
  glEnableVertexAttribArray(attr_texCoord);_glec
  glVertexAttribPointer(
    attr_pos,      2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)0
  );_glec
  glVertexAttribPointer(
    attr_texCoord, 2, GL_FLOAT, GL_FALSE, 16, (const GLvoid*)8
  );_glec
  
  GLint unif_unitScale = glGetUniformLocation(shaderProgram, "unitScale");
  glUniform2f(unif_unitScale, unitScale_2[0], unitScale_2[1]);
  GLint unif_scroll = glGetUniformLocation(shaderProgram, "scroll");
  glUniform2f(unif_scroll, 0, 0);
  
  GLuint uiTex = 0;
  texFromBmp(uiTex, uitex_path);
  glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  
  drawVertGroup(&curPlane->vg);
  
  
  float newCursAbs_gu3[3]   = {0}; // cursor state relative to screen
  float oldCursAbs_gu3[3]   = {0};
  float newScrollPos_gu2[2] = {0}; // plane center to screen center difference
  float oldScrollPos_gu2[2] = {0};
  float scrollVel_gu2[2]    = {0};
  float screenCrnrs_gu4[4]  = {0}; // xyxy, ltrb, relative to plane center
  
  
  timestamp ts_oldFrameStart = {0,0}, ts_newFrameStart = {0,0};
  timestamp ts_frameDelta = {0,0};
  #if LOG_TIMING
  timestamp ts_compTime = {0,0}, ts_now = {0,0};
  #endif
  getTimestamp(&ts_newFrameStart);
  
  int curFrame = 0;
  bool running = true;
  
  
	while (running) {
    ts_oldFrameStart = ts_newFrameStart;
    getTimestamp(&ts_newFrameStart);
    getTimeDelta(&ts_oldFrameStart, &ts_newFrameStart, &ts_frameDelta);
    #if LOG_TIMING
    printf(
      "ts_frameDelta: %1ld s, %9ld ns\n",
      ts_frameDelta.tv_sec, ts_frameDelta.tv_nsec
    );
    #endif
    
    fr(i,3) {oldCursAbs_gu3[i] = newCursAbs_gu3[i];}
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_MOUSEMOTION:
          newCursAbs_gu3[0] =  event.motion.x/gridUnit - halfVideoSize_gu2[0];
          newCursAbs_gu3[1] = -event.motion.y/gridUnit + halfVideoSize_gu2[1];
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: newCursAbs_gu3[2] = 1.0f; break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: newCursAbs_gu3[2] = 0; break;
          }
          break;
      }
    }
    
    fr(i,2) {oldScrollPos_gu2[i] = newScrollPos_gu2[i];}
    if (newCursAbs_gu3[2]) {
      if (oldCursAbs_gu3[2]) {
        fr(i,2) {newScrollPos_gu2[i] += newCursAbs_gu3[i] - oldCursAbs_gu3[i];}
      }
      else {fr(i,2) {scrollVel_gu2[i] = 0;}}
    }
    else {
      if (oldCursAbs_gu3[2]) {
        fr(i,2) {scrollVel_gu2[i] = newCursAbs_gu3[i] - oldCursAbs_gu3[i];}
      }
      fr(i,2) {newScrollPos_gu2[i] += scrollVel_gu2[i];}
    }
    
    
    if (!allEq(newScrollPos_gu2, oldScrollPos_gu2, 2)) {
      screenCrnrs_gu4[0] = newScrollPos_gu2[0]-halfVideoSize_gu2[0];
      screenCrnrs_gu4[1] = newScrollPos_gu2[1]+halfVideoSize_gu2[1];
      screenCrnrs_gu4[2] = newScrollPos_gu2[0]+halfVideoSize_gu2[0];
      screenCrnrs_gu4[3] = newScrollPos_gu2[1]-halfVideoSize_gu2[1];
      if (screenCrnrs_gu4[0] < curPlane->corners_gu4[0]) {
        newScrollPos_gu2[0] = curPlane->corners_gu4[0] + halfVideoSize_gu2[0];
        scrollVel_gu2[0] = 0;
      }
      else if (screenCrnrs_gu4[2] > curPlane->corners_gu4[2]) {
        newScrollPos_gu2[0] = curPlane->corners_gu4[2] - halfVideoSize_gu2[0];
        scrollVel_gu2[0] = 0;
      }
      if (screenCrnrs_gu4[1] > curPlane->corners_gu4[1]) {
        newScrollPos_gu2[1] = curPlane->corners_gu4[1] - halfVideoSize_gu2[1];
        scrollVel_gu2[1] = 0;
      }
      else if (screenCrnrs_gu4[3] < curPlane->corners_gu4[3]) {
        newScrollPos_gu2[1] = curPlane->corners_gu4[3] + halfVideoSize_gu2[1];
        scrollVel_gu2[1] = 0;
      }
      glUniform2f(unif_scroll, newScrollPos_gu2[0], newScrollPos_gu2[1]);_glec
      drawVertGroup(&curPlane->vg);
    }
    
    
    #if LOG_TIMING
		getTimestamp(&ts_now);
    getTimeDelta(&ts_newFrameStart, &ts_now, &ts_compTime);
    printf("ts_compTime: %3ld s, %9ld ns\n",
      ts_compTime.tv_sec, ts_compTime.tv_nsec
    );
    #endif
    
		SDL_GL_SwapWindow(window);_sdlec
    curFrame++;
	}
	
	SDL_GL_DeleteContext(GlContext);_sdlec
	SDL_Quit();_sdlec
	return 0;
}
