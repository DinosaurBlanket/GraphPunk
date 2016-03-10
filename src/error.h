#pragma once

#define CHECK_SDL_ERRORS 1
#define CHECK_GL_ERRORS  1
#define ENABLE_VSYNC     1
#define PRINT_GL_VERSION 0

#if CHECK_SDL_ERRORS
  void sdlec(int line, const char *file);
  #define _sdlec sdlec(__LINE__, __FILE__);
#else
  #define _sdlec
#endif

#if CHECK_GL_ERRORS
  void glec(const int line, const char *file);
  #define _glec glec(__LINE__, __FILE__);
#else
  #define _glec
#endif

#include "misc.h"
void printVerts(const uiVert *verts, int count);
