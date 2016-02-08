
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define  GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
typedef unsigned int uint;

#define CHECK_SDL_GL_ERRORS true
#define LOG_TIMING          true

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
int getFileSize(const char *restrict path) {
  struct stat st;
  stat(path, &st);
  return st.st_size;
}

void print_CouldNotOpenFile(const char *path) {
  printf("error: could not open file \"%s\"\n", path);
}
int stringFromFile(const char *restrict path, char *dest, uint32_t maxWrite) {
  if (!dest || !path || maxWrite < 1) return 0;
  FILE *fp = fopen(path, "r");
  if (!fp) {
    print_CouldNotOpenFile(path);
    return 0;
  }
  int c, i = 0;
  for (; i < maxWrite; i++) {
    c = fgetc(fp);
    if (c == EOF) {
      dest[i] = '\0';
      break;
    }
    dest[i] = c;
  }
  fclose(fp);
  return i;
}


GLuint createShaderProgram(
  const char *vertPath, 
  const char *fragPath, 
  const char *progName
) {
  int vertSourceSize, fragSourceSize, textBufSize = 1024;
  
  vertSourceSize = getFileSize(vertPath);
  if (!vertSourceSize) {
    print_CouldNotOpenFile(vertPath);
    return 0;
  }
  if (vertSourceSize > textBufSize) textBufSize = vertSourceSize + 1;
  
  fragSourceSize = getFileSize(fragPath);
  if (!fragSourceSize) {
    print_CouldNotOpenFile(fragPath);
    return 0;
  }
  if (fragSourceSize > textBufSize) textBufSize = fragSourceSize + 1;
  
  char *textBuf = malloc(textBufSize);
  GLint success;
  const char *compileErrorString = "error compiling shader \"%s\":\n%s\n";
  
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);_glec
  stringFromFile(vertPath, textBuf, textBufSize);
  glShaderSource(vertShader, 1, (const GLchar * const*)&textBuf, NULL);_glec
  glCompileShader(vertShader);_glec
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);_glec
  if (!success) {
    glGetShaderInfoLog(vertShader, textBufSize, NULL, textBuf);_glec
    printf(compileErrorString, vertPath, textBuf);
    return 0;
  }
  
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);_glec
  stringFromFile(fragPath, textBuf, textBufSize);
  glShaderSource(fragShader, 1, (const GLchar * const*)&textBuf, NULL);_glec
  glCompileShader(fragShader);_glec
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);_glec
  if (!success) {
    glGetShaderInfoLog(fragShader, textBufSize, NULL, textBuf);_glec
    printf(compileErrorString, fragPath, textBuf);
    return 0;
  }
  
  GLuint shaderProgram = glCreateProgram();_glec
  glAttachShader(shaderProgram, vertShader);_glec
  glAttachShader(shaderProgram, fragShader);_glec
  glLinkProgram(shaderProgram);_glec
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);_glec
  if (!success) {
    glGetShaderInfoLog(shaderProgram, textBufSize, NULL, textBuf);_glec
    printf("error linking shader program \"%s\":\n%s\n", progName, textBuf);
    return 0;
  }
  glValidateProgram(shaderProgram);_glec
  glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);_glec
  if (!success) {
    glGetShaderInfoLog(shaderProgram, textBufSize, NULL, textBuf);_glec
    printf("error: invalid shader program \"%s\":\n%s\n", progName, textBuf);
    return 0;
  }
  
  free(textBuf);
  return shaderProgram;
}


typedef struct {float p[2]; uint8_t c[4];} uiVert;

#define fr(i, bound) for (int i = 0; i < (bound); i++)

void ncopy(float *d, const float *s, int c) {fr(i,c) {d[i] = s[i];}}
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





int main(int argc, char *argv[]) {
	uint32_t videoSizeX   = 1280;//pixels
	uint32_t videoSizeY   =  800;
  float    videoSize[2] = {videoSizeX, videoSizeY};
	uint32_t gridUnit     =   16;
	
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
  //printf("OpenGL version: %s\n\n", glGetString(GL_VERSION));_glec
	
  
  
  uiVert vertices[] = {
    // plane
    {.p = {-120.0f,  120.0f}, .c = {0, 0xFF, 0, 0xFF}}, // 0
    {.p = { 120.0f,  120.0f}, .c = {0, 0xFF, 0, 0xFF}}, // 1
    {.p = { 120.0f, -120.0f}, .c = {0, 0xFF, 0, 0xFF}}, // 2
    {.p = {-120.0f, -120.0f}, .c = {0, 0xFF, 0, 0xFF}}, // 3
    // center marker
    {.p = { 0.0f,  1.0f},     .c = {0, 0, 0xFF, 0xFF}}, // 4
    {.p = { 1.0f,  0.0f},     .c = {0, 0, 0xFF, 0xFF}}, // 5
    {.p = { 0.0f, -1.0f},     .c = {0, 0, 0xFF, 0xFF}}, // 6
    {.p = {-1.0f,  0.0f},     .c = {0, 0, 0xFF, 0xFF}}  // 7
  };
  //uint32_t vertexCount = sizeof(vertices)/sizeof(uiVert);
  uint16_t indices[] = {
    // plane
    0,1,3, 1,2,3,
    // center marker
    4,5,7, 5,6,7
  };
  uint32_t indexCount = sizeof(indices)/sizeof(uint16_t);
  
  float unitScale[2] = {
    (float)(gridUnit*2)/(float)videoSizeX,
    (float)(gridUnit*2)/(float)videoSizeY
  };
  
  
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
    
    ncopy(oldCurs, newCurs, 3);
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
    
    ncopy(oldScrollPos, newScrollPos, 2);
    if (newCurs[2]) {
      if (oldCurs[2]) {
        fr(i,2) {newScrollPos[i] += newCurs[i] - oldCurs[i];}
      }
      else {
        fr(i,2) {inertia[i] = 0;}
      }
    }
    else {
      if (oldCurs[2]) {
        fr(i,2) {inertia[i] = newCurs[i] - oldCurs[i];}
      }
      fr(i,2) {newScrollPos[i] += inertia[i];}
    }
    
    if (!allEq(newScrollPos, oldScrollPos, 3)) {
      glUniform2f(unif_scroll, newScrollPos[0], newScrollPos[1]);
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
