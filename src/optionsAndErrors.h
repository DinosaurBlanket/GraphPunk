
#define CHECK_SDL_ERRORS true
#define CHECK_GL_ERRORS  true
#define LOG_TIMING       false

#if CHECK_SDL_ERRORS
  #include <SDL2/SDL_error.h>
  void sdlec(int line, const char *file) {
    const char *error = SDL_GetError();
    if (!error || !error[0]) return;
  	printf("SDL error at line %i in %s :\n%s", line, file, error);
    SDL_ClearError();
    exit(-10);
  }
  #define _sdlec sdlec(__LINE__, __FILE__);
#else
  #define _sdlec
#endif

#if CHECK_GL_ERRORS
  void glec(const int line, const char *file) {
    GLenum GLstatus;
    while ((GLstatus = glGetError()) != GL_NO_ERROR) {
      printf("OpenGL error: %i on line %i in %s\n", GLstatus, line, file);
    }
  }
  #define _glec glec(__LINE__, __FILE__);
#else
  #define _glec
#endif
