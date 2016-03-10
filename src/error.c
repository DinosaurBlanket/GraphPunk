
#include <SDL2/SDL_error.h>
void sdlec(int line, const char *file) {
  const char *error = SDL_GetError();
  if (!error || !error[0]) return;
  printf("SDL error at line %i in %s :\n%s\n", line, file, error);
  SDL_ClearError();
  exit(-10);
}

#include <GL/glew.h>
void glec(const int line, const char *file) {
  GLenum GLstatus;
  while ((GLstatus = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL error: %i on line %i in %s\n", GLstatus, line, file);
  }
}

#include "misc.h"
void printVerts(const uiVert *verts, int count) {
  fr(i,count) {
    printf(
      "%2i: vert pos, gu: %4.2f, %4.2f  -  tex pos, nt: %6.5f, %6.5f\n",
      i, verts[i].x, verts[i].y, verts[i].s, verts[i].t
    );
  }
}
