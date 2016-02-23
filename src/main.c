
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "optionsAndErrors.h"
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
  float    corners_gu4[4];
  float    pos_gudc2[2];
  int      depth;
  GLuint   vbo;
  uint32_t vertexCount;
  GLuint   ebo;
  uint32_t indexCount;
  //vinode *vinodes;
} plane; // will eventually also be used for picker pages, not just modules

typedef struct {
  plane p;
  //exnode *exnodes;
  // *specialNodes;
} module;

#define planePadding_gu 12

void correctPlaneCorners(plane *pln, float halfVideoSize_gu2[2]) {
  pln->corners_gu4[0] = floor(-halfVideoSize_gu2[0] - planePadding_gu); // tl
  pln->corners_gu4[1] = ceil ( halfVideoSize_gu2[1] + planePadding_gu); // tr
  pln->corners_gu4[2] = ceil ( halfVideoSize_gu2[0] + planePadding_gu); // bl
  pln->corners_gu4[3] = floor(-halfVideoSize_gu2[1] - planePadding_gu); // br
}
void correctPlaneVertices(plane *pln) {
  if (!pln->vbo) glGenBuffers(1, &pln->vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, pln->vbo);_glec
  uiVert vertices[] = {
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
    }, 
    // center marker
    {-0.5,  0.5, uitex_cntr_tl_x, uitex_cntr_tl_y}, //  8 tl
    { 0.5,  0.5, uitex_cntr_tr_x, uitex_cntr_tr_y}, //  9 tr
    { 0.5, -0.5, uitex_cntr_br_x, uitex_cntr_br_y}, // 10 br
    {-0.5, -0.5, uitex_cntr_bl_x, uitex_cntr_bl_y}  // 11 bl
  };
  pln->vertexCount = sizeof(vertices)/sizeof(uiVert);
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(vertices),
    vertices,
    GL_STATIC_DRAW
  );_glec
  
  if (!pln->ebo) glGenBuffers(1, &pln->ebo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pln->ebo);_glec
  uint16_t indices[] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  pln->indexCount = sizeof(indices)/sizeof(uint16_t);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, 
    sizeof(indices), 
    indices, 
    GL_STATIC_DRAW
  );_glec
}



int main(int argc, char *argv[]) {
  float videoSize_px2[2] = {800, 600}; // pixels
	float gridUnit = 32;                 // pixels
	float unitScale_2[2];
  fr(i,2) {unitScale_2[i] = gridUnit/(videoSize_px2[i]/2.0f);}
  float halfVideoSize_gu2[2];          // grid units
  fr(i,2) {halfVideoSize_gu2[i] = (videoSize_px2[i]/gridUnit)/2.0f;}
  
	SDL_Window    *window    = NULL;
	SDL_GLContext  GLcontext = NULL;
	SDL_Init(SDL_INIT_VIDEO);_sdlec
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	window = SDL_CreateWindow(
		"GraphPunk",               //const char* title,
		SDL_WINDOWPOS_UNDEFINED,   //int         x,
		SDL_WINDOWPOS_UNDEFINED,   //int         y,
		videoSize_px2[0],           //int         w,
		videoSize_px2[1],           //int         h,
		SDL_WINDOW_OPENGL          //Uint32      flags
	);_sdlec
	GLcontext = SDL_GL_CreateContext(window);_sdlec
  SDL_GL_SetSwapInterval(1);_sdlec
  
  glewExperimental = GL_TRUE;
  {
  	GLenum r = glewInit();
    if (r != GLEW_OK) {
      printf("GLEW error: %s\n", glewGetErrorString(r));
      return 1;
    }
    // There's an OpenGL error 1280 here for some reason, just flush it...
    while (glGetError() != GL_NO_ERROR) {};
  }
  //printf("OpenGL version: %s\n\n", glGetString(GL_VERSION));_glec
	
  
  GLuint vao_UI;
  glGenVertexArrays(1, &vao_UI);_glec
  glBindVertexArray(vao_UI);_glec
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  module rootMod = {0};
  correctPlaneCorners(&rootMod.p, halfVideoSize_gu2);
  correctPlaneVertices(&rootMod.p);
  
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
  
  GLuint uiTex;
  texFromBmp(uiTex, uitex_path);
  glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  
  
  plane *curPlane = &rootMod.p;
  
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
  
  glDrawElements(GL_TRIANGLES, curPlane->indexCount, GL_UNSIGNED_SHORT, 0);_glec
  
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
    bool redraw = false;
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
      redraw = true;
    }
    
    if (redraw) {
      glDrawElements(
        GL_TRIANGLES, curPlane->indexCount, GL_UNSIGNED_SHORT, 0
      );_glec
      redraw = false;
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
	
	SDL_GL_DeleteContext(GLcontext);_sdlec
	SDL_Quit();_sdlec
	return 0;
}
