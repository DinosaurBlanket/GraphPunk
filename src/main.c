
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "optionsAndErrors.h"
#include "oglTools.h"


#define fr(i, bound) for (int i = 0; i < (bound); i++)
bool allEq(const float *l, const float *r, int c) {
  fr(i,c) {if (l[i] != r[i]) return false;}
  return true;
}
//bool any(const float *a, int c) {fr(i,c) {if(a[i]) return true;} return false;}


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


typedef struct {float p[2]; uint8_t c[4];} uiVert;


int main(int argc, char *argv[]) {
  float videoSize[2] = {1280, 800}; // pixels
	float gridUnit = 16; // pixels
	
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
		videoSize[0],              //int         w,
		videoSize[1],              //int         h,
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
	
  
  float plnSz[4] = {-60, 60, 60, -60}; // in units, XYXY, top-left and bot-right
  uiVert vertices[] = {
    // inside border
    {.p = {plnSz[0]+1, plnSz[1]-1}, .c = {0, 0xFF, 0, 0xFF}}, //  0 tl
    {.p = {plnSz[2]-1, plnSz[1]-1}, .c = {0, 0xFF, 0, 0xFF}}, //  1 tr
    {.p = {plnSz[2]-1, plnSz[3]+1}, .c = {0, 0xFF, 0, 0xFF}}, //  2 br
    {.p = {plnSz[0]+1, plnSz[3]+1}, .c = {0, 0xFF, 0, 0xFF}}, //  3 bl
    // outside border
    {.p = {plnSz[0],   plnSz[1]},   .c = {0xFF, 0, 0, 0xFF}}, //  4 tl
    {.p = {plnSz[2],   plnSz[1]},   .c = {0xFF, 0, 0, 0xFF}}, //  5 tr
    {.p = {plnSz[2],   plnSz[3]},   .c = {0xFF, 0, 0, 0xFF}}, //  6 br
    {.p = {plnSz[0],   plnSz[3]},   .c = {0xFF, 0, 0, 0xFF}}, //  7 bl
    // center marker
    {.p = { 0.0f,  1.0f},           .c = {0, 0, 0xFF, 0xFF}}, //  8 t
    {.p = { 1.0f,  0.0f},           .c = {0, 0, 0xFF, 0xFF}}, //  9 r
    {.p = { 0.0f, -1.0f},           .c = {0, 0, 0xFF, 0xFF}}, // 10 b
    {.p = {-1.0f,  0.0f},           .c = {0, 0, 0xFF, 0xFF}}  // 11 l
  };
  //uint32_t vertexCount = sizeof(vertices)/sizeof(uiVert);
  uint16_t indices[] = {
    // plane inside border
    0,1,3, 1,2,3,
    // border
    4,5,0, 5,1,0,  5,6,1, 1,6,2,  3,6,7, 3,2,6,  4,3,7, 4,0,3,
    // center marker
    8,9,11, 11,10,9
  };
  uint32_t indexCount = sizeof(indices)/sizeof(uint16_t);
  
  float unitScale[2];
  fr(i,2) {unitScale[i] = gridUnit/(videoSize[i]/2);}
  
  
  
  GLuint vao;
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
  
  
  GLint attr_pos   = glGetAttribLocation(shaderProgram, "pos"  );_glec
  GLint attr_color = glGetAttribLocation(shaderProgram, "color");_glec
  glEnableVertexAttribArray(attr_pos  );_glec
  glEnableVertexAttribArray(attr_color);_glec
  glVertexAttribPointer(
    attr_pos,   2, GL_FLOAT,         GL_FALSE, 12, (const GLvoid*)0
  );_glec
  glVertexAttribPointer(
    attr_color, 4, GL_UNSIGNED_BYTE, GL_TRUE,  12, (const GLvoid*)8
  );_glec
  
  
  GLint unif_unitScale = glGetUniformLocation(shaderProgram, "unitScale");
  glUniform2f(unif_unitScale, unitScale[0], unitScale[1]);
  GLint unif_scroll = glGetUniformLocation(shaderProgram, "scroll");
  glUniform2f(unif_scroll, 0, 0);
  
  timespec ts_oldFrameStart = {0,0}, ts_newFrameStart = {0,0};
  timespec ts_frameDelta = {0,0};
  #if LOG_TIMING
  timespec ts_compTime = {0,0}, ts_now = {0,0};
  #endif
  clock_gettime(CLOCK_MONOTONIC, &ts_newFrameStart);
  
  // normalized, -1 to 1 for x and y, 0 to 1 for z
  float newCurs[3]   = {0};
  float oldCurs[3]   = {0};
  float newScrollPos[2] = {0};
  float oldScrollPos[2] = {0};
  float inertia[2]   = {0};
  
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
    
    fr(i,3) {oldCurs[i] = newCurs[i];}
    bool redraw = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_MOUSEMOTION:
          newCurs[0] = event.motion.x;
          newCurs[1] = event.motion.y;
          fr(i,2) {newCurs[i] = (newCurs[i]-videoSize[i]/2)/(videoSize[i]/2);}
          newCurs[1] *= -1;
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: newCurs[2] = 1.0; break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: newCurs[2] = 0.0; break;
          }
          break;
      }
    }
    
    fr(i,2) {oldScrollPos[i] = newScrollPos[i];}
    if (newCurs[2]) {
      if (oldCurs[2]) {fr(i,2) {newScrollPos[i] += newCurs[i] - oldCurs[i];}}
      else {fr(i,2) {inertia[i] = 0;}}
    }
    else {
      if (oldCurs[2]) {fr(i,2) {inertia[i] = newCurs[i] - oldCurs[i];}}
      fr(i,2) {newScrollPos[i] += inertia[i];}
    }
    
    if (!allEq(newScrollPos, oldScrollPos, 3)) {
      glUniform2f(unif_scroll, newScrollPos[0], newScrollPos[1]);_glec
      redraw = true;
    }
    
    if (redraw) {
      glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);_glec
      redraw = false;
    }
    
    #if LOG_TIMING
		clock_gettime(CLOCK_MONOTONIC, &ts_now);
    getTimeDelta(&ts_newFrameStart, &ts_now, &ts_compTime);
    printf(
      "ts_compTime: %3ld s, %9ld ns\n",
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
