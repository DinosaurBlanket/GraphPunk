
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "optionsAndErrors.h"
#include "oglTools.h"
#include "../img/peptex.h"


#define fr(i, bound) for (int i = 0; i < (bound); i++)

bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}


#include <time.h>
typedef struct timespec timespec;
// This function assumes latter >= former
void getTimeDelta (timespec *former, timespec *latter, timespec *delta) {
  delta->tv_sec = latter->tv_sec - former->tv_sec;
  if (former->tv_nsec > latter->tv_nsec) { // then we have to carry
    delta->tv_sec--;
    delta->tv_nsec = (1e9 - former->tv_nsec) + latter->tv_nsec;
  }
  else delta->tv_nsec = latter->tv_nsec - former->tv_nsec;
}


typedef struct {float x; float y; float s; float t;} uiVert;


int main(int argc, char *argv[]) {
  float videoSize_p2[2] = {800, 600}; // pixels
	float gridUnit = 32;                // pixels
  float halfVideoSize_u2[2];          // units
  fr(i,2) {halfVideoSize_u2[i] = (videoSize_p2[i]/gridUnit)/2.0f;}
	
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
		videoSize_p2[0],           //int         w,
		videoSize_p2[1],           //int         h,
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
	
  
  float planeCrnrs_u4[4] = {
    floor(-videoSize_p2[0]/gridUnit), ceil(videoSize_p2[1]/gridUnit), // tl
    ceil(videoSize_p2[0]/gridUnit), floor(-videoSize_p2[1]/gridUnit)  // br
  }; // in units
  
  uiVert vertices[] = {
    // inside border
    {planeCrnrs_u4[0]+1, planeCrnrs_u4[1]-1, peptex_ibord_tl_x, peptex_ibord_tl_y}, //  0 tl
    {planeCrnrs_u4[2]-1, planeCrnrs_u4[1]-1, peptex_ibord_tr_x, peptex_ibord_tr_y}, //  1 tr
    {planeCrnrs_u4[2]-1, planeCrnrs_u4[3]+1, peptex_ibord_br_x, peptex_ibord_br_y}, //  2 br
    {planeCrnrs_u4[0]+1, planeCrnrs_u4[3]+1, peptex_ibord_bl_x, peptex_ibord_bl_y}, //  3 bl
    // outside border
    {planeCrnrs_u4[0],   planeCrnrs_u4[1],   peptex_obord_tl_x, peptex_obord_tl_y}, //  4 tl
    {planeCrnrs_u4[2],   planeCrnrs_u4[1],   peptex_obord_tr_x, peptex_obord_tr_y}, //  5 tr
    {planeCrnrs_u4[2],   planeCrnrs_u4[3],   peptex_obord_br_x, peptex_obord_br_y}, //  6 br
    {planeCrnrs_u4[0],   planeCrnrs_u4[3],   peptex_obord_bl_x, peptex_obord_bl_y}, //  7 bl
    // center marker
    {-0.5,  0.5, peptex_cntr_tl_x,  peptex_cntr_tl_y}, //  8 tl
    { 0.5,  0.5, peptex_cntr_tr_x,  peptex_cntr_tr_y}, //  9 tr
    { 0.5, -0.5, peptex_cntr_br_x,  peptex_cntr_br_y}, // 10 br
    {-0.5, -0.5, peptex_cntr_bl_x,  peptex_cntr_bl_y}  // 11 bl
  };
  //uint32_t vertexCount = sizeof(vertices)/sizeof(uiVert);
  uint16_t indices[] = {
    // inside border
    0,1,3, 1,2,3,
    // outside border
    4,5,0, 5,1,0,  5,6,1, 6,2,1,  6,7,2, 7,3,2,  7,4,3, 4,0,3,
    // center marker
    8,9,11, 9,10,11
  };
  uint32_t indexCount = sizeof(indices)/sizeof(uint16_t);
  
  float unitScale_2[2];
  fr(i,2) {unitScale_2[i] = gridUnit/(videoSize_p2[i]/2);}
  
  
  GLuint vao;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glGenVertexArrays(1, &vao);_glec
  glBindVertexArray(vao);_glec
  
  GLuint vbo;
  GLuint ebo;
  glGenBuffers(1, &vbo);_glec
  glGenBuffers(1, &ebo);_glec
  glBindBuffer(GL_ARRAY_BUFFER,         vbo);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(vertices),
    vertices,
    GL_STATIC_DRAW
  );_glec
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, 
    sizeof(indices), 
    indices, 
    GL_STATIC_DRAW
  );_glec
  
  
  
  GLuint shaderProgram = createShaderProgram(
    "src/vert.glsl", 
    "src/frag.glsl", 
    "shaderProgram"
  );
  if (!shaderProgram) return __LINE__;
  glUseProgram(shaderProgram);_glec
  
  
  GLint attr_pos      = glGetAttribLocation(shaderProgram, "pos"  );_glec
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
  
  GLuint tex;
  glGenTextures(1, &tex);_glec
  glBindTexture(GL_TEXTURE_2D, tex);_glec
  {
    SDL_Surface *srfc_ = SDL_LoadBMP("img/peptex.bmp");_sdlec
    SDL_Surface *srfc  = SDL_ConvertSurfaceFormat(
      srfc_, SDL_PIXELFORMAT_ABGR8888, 0
    );_sdlec
    SDL_FreeSurface(srfc_);_sdlec
    
    glTexImage2D(
      GL_TEXTURE_2D,       // GLenum        target
      0,                   // GLint         level
      GL_RGBA,             // GLint         internalformat
      peptex_size_x,       // GLsizei       width
      peptex_size_y,       // GLsizei       height
      0,                   // GLint         border
      GL_RGBA,             // GLenum        format
      GL_UNSIGNED_BYTE,    // GLenum        type
      srfc->pixels         // const GLvoid *data
    );_glec
    
    SDL_FreeSurface(srfc);_sdlec
  }
  glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);_glec
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  
  
  timespec ts_oldFrameStart = {0,0}, ts_newFrameStart = {0,0};
  timespec ts_frameDelta = {0,0};
  #if LOG_TIMING
  timespec ts_compTime = {0,0}, ts_now = {0,0};
  #endif
  clock_gettime(CLOCK_MONOTONIC, &ts_newFrameStart);
  
  // normalized, -1 to 1 for x and y, 0 to 1 for z
  float newCurs_ndc3[3]      = {0};
  float oldCurs_ndc3[3]      = {0};
  float newScrollPos_ndc2[2] = {0};
  float oldScrollPos_ndc2[2] = {0};
  float scrollVel_ndc2[2]    = {0};
  
  // in units, relative to plane center
  float screenCrnrs_u4[4] = {0}; // xyxy, tl br
  //float cursPos_u3[3]   = {0}; // xyz
  
  int curFrame = 0;
  bool running = true;
  
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);_glec
  
	while (running) {
    ts_oldFrameStart = ts_newFrameStart;
    clock_gettime(CLOCK_MONOTONIC, &ts_newFrameStart);
    getTimeDelta(&ts_oldFrameStart, &ts_newFrameStart, &ts_frameDelta);
    #if LOG_TIMING
    printf(
      "ts_frameDelta: %1ld s, %9ld ns\n",
      ts_frameDelta.tv_sec, ts_frameDelta.tv_nsec
    );
    #endif
    
    fr(i,3) {oldCurs_ndc3[i] = newCurs_ndc3[i];}
    bool redraw = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_MOUSEMOTION:
          newCurs_ndc3[0] = event.motion.x;
          newCurs_ndc3[1] = event.motion.y;
          fr(i,2) {
            newCurs_ndc3[i] = 
              (newCurs_ndc3[i] - videoSize_p2[i]/2) / (videoSize_p2[i]/2)
            ;
          }
          newCurs_ndc3[1] *= -1;
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: newCurs_ndc3[2] = 1.0; break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: newCurs_ndc3[2] = 0.0; break;
          }
          break;
      }
    }
    
    fr(i,2) {oldScrollPos_ndc2[i] = newScrollPos_ndc2[i];}
    if (newCurs_ndc3[2]) {
      if (oldCurs_ndc3[2]) {
        fr(i,2) {newScrollPos_ndc2[i] += newCurs_ndc3[i] - oldCurs_ndc3[i];}
      }
      else {fr(i,2) {scrollVel_ndc2[i] = 0;}}
    }
    else {
      if (oldCurs_ndc3[2]) {
        fr(i,2) {scrollVel_ndc2[i] = newCurs_ndc3[i] - oldCurs_ndc3[i];}
      }
      fr(i,2) {newScrollPos_ndc2[i] += scrollVel_ndc2[i];}
    }
    
    
    if (!allEq(newScrollPos_ndc2, oldScrollPos_ndc2, 2)) {
      screenCrnrs_u4[0] = -1/unitScale_2[0] - newScrollPos_ndc2[0]/unitScale_2[0];
      screenCrnrs_u4[1] =  1/unitScale_2[1] - newScrollPos_ndc2[1]/unitScale_2[1];
      screenCrnrs_u4[2] =  1/unitScale_2[0] - newScrollPos_ndc2[0]/unitScale_2[0];
      screenCrnrs_u4[3] = -1/unitScale_2[1] - newScrollPos_ndc2[1]/unitScale_2[1];
      if (screenCrnrs_u4[0] < planeCrnrs_u4[0]) {
        newScrollPos_ndc2[0] = (planeCrnrs_u4[0] + halfVideoSize_u2[0])*-unitScale_2[0];
        scrollVel_ndc2[0] = 0;
      }
      else if (screenCrnrs_u4[2] > planeCrnrs_u4[2]) {
        newScrollPos_ndc2[0] = (planeCrnrs_u4[2] - halfVideoSize_u2[0])*-unitScale_2[0];
        scrollVel_ndc2[0] = 0;
      }
      if (screenCrnrs_u4[1] > planeCrnrs_u4[1]) {
        newScrollPos_ndc2[1] = (planeCrnrs_u4[1] - halfVideoSize_u2[1])*-unitScale_2[1];
        scrollVel_ndc2[1] = 0;
      }
      else if (screenCrnrs_u4[3] < planeCrnrs_u4[3]) {
        newScrollPos_ndc2[1] = (planeCrnrs_u4[3] + halfVideoSize_u2[1])*-unitScale_2[1];
        scrollVel_ndc2[1] = 0;
      }
      glUniform2f(unif_scroll, newScrollPos_ndc2[0], newScrollPos_ndc2[1]);_glec
      redraw = true;
    }
    
    if (redraw) {
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);_glec
      redraw = false;
    }
    
    #if LOG_TIMING
		clock_gettime(CLOCK_MONOTONIC, &ts_now);
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
