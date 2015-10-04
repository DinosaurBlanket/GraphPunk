
#include <vector>
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "options.h"
#include "viewMap.hpp"

#if CHECK_ERRORS_SDL_GL
  void glec(const int line, const char *file);
  #define _glec glec(__LINE__, __FILE__);
  void sdlec(int line, const char *file);
  #define _sdlec sdlec(__LINE__, __FILE__);
#else
  #define _glec
  #define _sdlec
#endif

void logVrtxData(
  std::vector<vertex> &vrtxData,
  uint                 pitch,
  const char          *path
);
void logIndxData(
  std::vector<indx_t> &indxData,
  uint                 pitch,
  const char          *path
);
