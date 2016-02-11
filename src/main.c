
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
  float videoSize[2] = {1280, 800}; // pixels
	float gridUnit = 32; // pixels
	
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
	
  
  float plnSz[4] = {-30, 30, 30, -30}; // in units, XYXY, top-left and bot-right
  uiVert vertices[] = {
    // inside border
    {plnSz[0]+1, plnSz[1]-1, plnSz[0]+1, plnSz[1]-1}, //  0 tl
    {plnSz[2]-1, plnSz[1]-1, plnSz[2]-1, plnSz[1]-1}, //  1 tr
    {plnSz[2]-1, plnSz[3]+1, plnSz[2]-1, plnSz[3]+1}, //  2 br
    {plnSz[0]+1, plnSz[3]+1, plnSz[0]+1, plnSz[3]+1}, //  3 bl
    // outside border
    {plnSz[0],   plnSz[1],   plnSz[0]-1, plnSz[1]+1}, //  4 tl
    {plnSz[2],   plnSz[1],   plnSz[2]+1, plnSz[1]+1}, //  5 tr
    {plnSz[2],   plnSz[3],   plnSz[2]+1, plnSz[3]-1}, //  6 br
    {plnSz[0],   plnSz[3],   plnSz[0]-1, plnSz[3]-1}, //  7 bl
  };
  //uint32_t vertexCount = sizeof(vertices)/sizeof(uiVert);
  uint16_t indices[] = {
    // plane inside border
    0,1,3, 1,2,3,
    // border
    4,5,0, 5,1,0,  5,6,1, 1,6,2,  3,6,7, 3,2,6,  4,3,7, 4,0,3,
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
  glUniform2f(unif_unitScale, unitScale[0], unitScale[1]);
  GLint unif_scroll = glGetUniformLocation(shaderProgram, "scroll");
  glUniform2f(unif_scroll, 0, 0);
  
  GLuint tex;
  glGenTextures(1, &tex);_glec
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);_glec
  {
    SDL_Surface *srfc = SDL_LoadBMP("img/planeWallpaperMarkedRGB.bmp");_sdlec
    
    //printf("srfc->w: %d, srfc->h: %d\n", srfc->w, srfc->h);
    //fr(r, srfc->h) {
    //  fr(c, srfc->w) {
    //    uint8_t *pixel = (uint8_t*)srfc->pixels + (srfc->w*r + c)*3;
    //    printf("%3d %3d %3d - ", *pixel, *(pixel+1), *(pixel+2));
    //  }
    //  puts("");
    //}
    
    glTexImage2D(
      GL_TEXTURE_2D,       // GLenum        target
      0,                   // GLint         level
      GL_RGB,              // GLint         internalformat
      gridUnit,            // GLsizei       width
      gridUnit,            // GLsizei       height
      0,                   // GLint         border
      GL_RGB,              // GLenum        format
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
