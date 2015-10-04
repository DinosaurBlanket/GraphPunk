
#include <iostream>
using std::cout;
using std::endl;
#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <vector>

#include "errorsAndLogging.hpp"
#include "viewMap.hpp"


int main(int argc, char *argv[]) {
	vec2 videoSize = vec2(1280, 800);//pixels
	
	SDL_Window    *window    = NULL;
	SDL_GLContext  GLcontext = NULL;
	SDL_Init(SDL_INIT_VIDEO);_sdlec
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	window = SDL_CreateWindow(
		"GraphPunk",               //const char* title,
		SDL_WINDOWPOS_UNDEFINED,   //int         x,
		SDL_WINDOWPOS_UNDEFINED,   //int         y,
		videoSize.x,               //int         w,
		videoSize.y,               //int         h,
		SDL_WINDOW_OPENGL          //Uint32      flags
	);_sdlec
	GLcontext = SDL_GL_CreateContext(window);_sdlec
  SDL_GL_SetSwapInterval(1);_sdlec
  
  glewExperimental = GL_TRUE;
  {
  	GLenum r = glewInit();
    if (r != GLEW_OK) {
      cout << "Error: " << endl << glewGetErrorString(r) << endl;
      return 1;
    }
  }
  
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;_glec
  glClearColor(0.0,0.2,0.3,1.0);_glec
	
  
  scrollable scroll;
  initMap(scroll, videoSize);
  
  float cursPress  = 0;
  float pCursPress = 0;
  vec2  cursPos;
  vec2  pCursPos;
  int  curFrame = 0;
  bool running = true;
	while (running) {
    pCursPress = cursPress;
    pCursPos   = cursPos;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_MOUSEMOTION:
          cursPos = vec2(event.motion.x, event.motion.y);
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: cursPress = 1; break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT: cursPress = 0; break;
          }
          break;
      }
    }
    scroll.advance(cursPress, pCursPress, cursPos, pCursPos);
    if (scroll.hasMoved() || !curFrame) {
  		drawMap(scroll);
  		SDL_GL_SwapWindow(window);_sdlec
    }
		SDL_Delay(16);
    curFrame++;
	}
	
	SDL_GL_DeleteContext(GLcontext);_sdlec
	SDL_Quit();_sdlec
	return 0;
}

