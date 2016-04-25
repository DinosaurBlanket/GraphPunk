
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "error.h"
#include "timestamp.h"
#include "ui.h"


int main(int argc, char *argv[]) {
  float videoSize_px2[2] = {800, 600}; // pixels
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  initUi(videoSize_px2);

  timestamp ts_oldFrameStart = {0,0}, ts_newFrameStart = {0,0};
  timestamp ts_frameDelta = {0,0};
  #if LOG_TIMING
  timestamp ts_compTime = {0,0}, ts_now = {0,0};
  #endif
  getTimestamp(&ts_newFrameStart);

  int  curFrame = 0;
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

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_MOUSEMOTION:
          curMove(event.motion.x, event.motion.y);
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:
              clickDn(event.button.x, event.button.y);
              break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:
              clickUp(event.button.x, event.button.y);
              break;
          }
          break;
      }
    }

    perFrame();

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
