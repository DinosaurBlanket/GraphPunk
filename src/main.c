
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#define CHECK_SDL_GL_ERRORS true

#if CHECK_SDL_GL_ERRORS
  void glec(const int line, const char *file) {
    GLenum GLstatus;
    while ((GLstatus = glGetError()) != GL_NO_ERROR) {
      printf("OpenGL error: %i on line %i in %s\n", GLstatus, line, file);
    }
  }
  #define _glec glec(__LINE__, __FILE__);
  void sdlec(int line, const char *file) {
    const char *error = SDL_GetError();
    if (!error || !error[0]) return;
  	printf("SDL error at line %i in %s :\n%s", line, file, error);
    SDL_ClearError();
    exit(-10);
  }
  #define _sdlec sdlec(__LINE__, __FILE__);
#else
  #define _glec
  #define _sdlec
#endif


#include <sys/stat.h>
// caller must free the pointer returned by this function
char *stringFromFile(const char *restrict path) {
  FILE *fp = fopen(path, "r");
  if(!fp) {
    printf("error: could not open file: %s\n", path);
    return NULL;
  }
  uint32_t size;
  {
    struct stat st;
    int fd = fileno(fp);
    fstat(fd, &st);
    size = st.st_size;
  }
  char *out = malloc(size);
  int c;
  for (int32_t i = 0;; i++) {
    c = fgetc(fp);
    if (c == EOF || i == size-1) {
      out[i] = '\0';
      break;
    }
    out[i] = c;
  }
  fclose(fp);
  return out;
}

void shaderSourceFromFile(const char *restrict path, GLuint shader) {
  char *source = stringFromFile(path);
  glShaderSource(shader, 1, (const GLchar * const*)&source, NULL);_glec
  free(source);
}


int main(int argc, char *argv[]) {
	uint32_t videoSizeX = 1280;//pixels
	uint32_t videoSizeY =  800;
	
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
		videoSizeX,                //int         w,
		videoSizeY,                //int         h,
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
  printf("OpenGL version: %s\n", glGetString(GL_VERSION));_glec
	
  
  GLuint vao;
  glGenVertexArrays(1, &vao);_glec
  glBindVertexArray(vao);_glec
  
  glClearColor(0.0,0.2,0.3,1.0);_glec
  
  float vertices[] = {
    +0.0f, +0.5f,
    +0.5f, -0.5f,
    -0.5f, -0.5f
  };
  GLuint vbo;
  glGenBuffers(1, &vbo);_glec
  glBindBuffer(GL_ARRAY_BUFFER, vbo);_glec
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(vertices),
    vertices,
    GL_STATIC_DRAW
  );_glec
  
  
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);_glec
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);_glec
  shaderSourceFromFile("src/vert.glsl", vertShader);
  shaderSourceFromFile("src/frag.glsl", fragShader);
  glCompileShader(vertShader);_glec
  glCompileShader(fragShader);_glec
  
  GLuint shaderProgram = glCreateProgram();_glec
  glAttachShader(shaderProgram, vertShader);_glec
  glAttachShader(shaderProgram, fragShader);_glec
  glLinkProgram(shaderProgram);_glec
  glUseProgram(shaderProgram);_glec
  
  GLint attrib_pos = glGetAttribLocation(shaderProgram, "position");_glec
  glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);_glec
  glEnableVertexAttribArray(attrib_pos);_glec
  
  
  int curFrame = 0;
  bool running = true;
	while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: running = false; break;
        case SDL_MOUSEMOTION:
          break;
        case SDL_MOUSEBUTTONDOWN:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:  break;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:  break;
          }
          break;
      }
    }
  	
    glClear(GL_COLOR_BUFFER_BIT);_glec
		glDrawArrays(GL_TRIANGLES, 0, 3);_glec
		
		
		SDL_GL_SwapWindow(window);_sdlec
		
		SDL_Delay(16);
    curFrame++;
	}
	
	SDL_GL_DeleteContext(GLcontext);_sdlec
	SDL_Quit();_sdlec
	return 0;
}
