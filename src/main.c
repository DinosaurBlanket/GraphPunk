
#include <stdio.h>
#include <stdint.h>

#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "error.h"
#include "oglTools.h"
#include "../img/uitex.h"
#include "timestamp.h"
#include "misc.h"



int main(int argc, char *argv[]) {
  float videoSize_px2[2] = {800, 600}; // pixels
	float gridUnit = uitex_guSize_px;
	float unitScale_2[2];
  fr(i,2) {unitScale_2[i] = gridUnit/(videoSize_px2[i]/2.0f);}
  float halfVideoSize_gu2[2];          // grid units
  fr(i,2) {halfVideoSize_gu2[i] = (videoSize_px2[i]/gridUnit)/2.0f;}
  
  
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
  
  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  //vertGroup glorolsVerts;
  //initGlorolsVerts(&glorolsVerts, halfVideoSize_gu2);
  
  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  
  //GLuint vao_UI;
  //glGenVertexArrays(1, &vao_UI);_glec
  //glBindVertexArray(vao_UI);_glec
  
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
  float screenCrnrs_gu4[4]  = {0}; // xyxy, bl tr, relative to plane center
  
  //bool paused;
  //bool auMuted;
  //bool auSoloed;
  //bool moveBranch;
  //bool locked;
  //bool Saving;
  
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
      screenCrnrs_gu4[1] = newScrollPos_gu2[1]-halfVideoSize_gu2[1];
      screenCrnrs_gu4[2] = newScrollPos_gu2[0]+halfVideoSize_gu2[0];
      screenCrnrs_gu4[3] = newScrollPos_gu2[1]+halfVideoSize_gu2[1];
      fr(i,2) {
        if (screenCrnrs_gu4[i] < curPlane->corners_gu4[i]) {
          newScrollPos_gu2[i] = curPlane->corners_gu4[i]+halfVideoSize_gu2[i];
          scrollVel_gu2[i] = 0;
        }
        else if (screenCrnrs_gu4[i+2] > curPlane->corners_gu4[i+2]) {
          newScrollPos_gu2[i] = curPlane->corners_gu4[i+2]-halfVideoSize_gu2[i];
          scrollVel_gu2[i] = 0;
        }
      }
      printf("plane\n");
      glUniform2f(unif_scroll, newScrollPos_gu2[0], newScrollPos_gu2[1]);_glec
      drawVertGroup(&curPlane->vg);
      //printf("glorols\n");
      //glUniform2f(unif_scroll, 0.0, 0.0);_glec
      //drawVertGroup(&glorolsVerts);
      printf("end draw\n");
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
