
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
