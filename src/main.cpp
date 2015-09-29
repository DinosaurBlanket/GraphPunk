//g++ main.cpp -o GraphPunk -Wall -std=c++11
//`sdl2-config --cflags --libs` -lGL -lGLEW

#include <iostream>
using std::cout;
using std::endl;
#define   GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL_opengl.h>
#include <vector>

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "options.h"


#if GLEC_ENABLED
void glec(const int line, const char *file) {
  GLenum GLstatus;
  while ((GLstatus = glGetError()) != GL_NO_ERROR) {
    cout<<"OpenGL error: "<<GLstatus<<" on line "<<line<<" in "<<file<<endl;
  }
}
#define _glec glec(__LINE__, __FILE__);
#else
#define _glec
#endif


#include <string>
using std::string;
#include <sstream>
#include <fstream>
int readFile(const char *path, string &out) {
  std::ifstream fs(path);
  std::stringstream ss;
  ss << fs.rdbuf();
  if (fs.fail()) {cout << "Error reading file: " << path << endl;}
  out = ss.str();
  return out.length();
}


const char *shaderTypeString(GLuint st) {
  switch(st) {
    case GL_VERTEX_SHADER  : return "vertex shader";
    case GL_FRAGMENT_SHADER: return "fragment shader";
  }
  return "unknown shader type";
}

void addShader(
  GLuint  shaderProgram,
  string &shaderText,
  GLenum  shaderType
) {
  GLuint shaderObj = glCreateShader(shaderType);_glec
  GLint shaderTextLen = shaderText.size();
  const char* shaderTexts = shaderText.c_str();
  glShaderSource(shaderObj, 1, &shaderTexts, &shaderTextLen);_glec
  glCompileShader(shaderObj);_glec
  GLint success;
  glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);_glec
  if (!success) {
    GLchar infoLog[1024];
    glGetShaderInfoLog(shaderObj, sizeof(infoLog), NULL, infoLog);_glec
    cout << "Error compiling " << shaderTypeString(shaderType) << endl
    << infoLog << endl;
    exit(__LINE__);
  }
  glAttachShader(shaderProgram, shaderObj);_glec
}

void compileShaders(
  GLuint     &shaderProgram,
  const char *vsPath,
  const char *fsPath
) {
  shaderProgram = glCreateProgram();_glec
  string vs, fs;
  if (!readFile(vsPath, vs)) exit(__LINE__);
  if (!readFile(fsPath, fs)) exit(__LINE__);
  addShader(shaderProgram, vs, GL_VERTEX_SHADER);
  addShader(shaderProgram, fs, GL_FRAGMENT_SHADER);
  GLint success = 0;
  GLchar errorLog[1024] = {0};
  glLinkProgram(shaderProgram);_glec
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);_glec
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);_glec
    cout << "Error linking shader program: " << endl << errorLog << endl;
    exit(__LINE__);
  }
  glValidateProgram(shaderProgram);_glec
  glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);_glec
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);_glec
    cout << "Invalid shader program: " << endl << errorLog << endl;
    exit(__LINE__);
  }
}








//view_map
GLuint VBO;
GLuint IBO;
GLuint shaderProgram;
GLint attr_pos;
GLint attr_color;
GLint unif_transform;
const char *vsPath = "view_map_vs.glsl";
const char *fsPath = "view_map_fs.glsl";
struct vertex {
  vec2     pos;
  uint32_t color;
};
typedef uint16_t indx_t ;
struct vrtxGroup {
  std::vector<vertex> vrtx;
  uint32_t vboStart;
};
vrtxGroup gridLines;
vrtxGroup originSquare;
std::vector<vrtxGroup*> vrtxGroups = {&originSquare, &gridLines};
std::vector<indx_t> indxAll;
uint vrtxCount = 0;
uint indxCount = 0;

#if LOGGING_ENABLED
#include <iostream>
#include <fstream>
void logVrtxData(
  std::vector<vertex> &vrtxData,
  uint pitch,
  const char *path
) {
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < vrtxData.size(); i ++) {
    of <<
    vrtxData[i].pos.x << "\t" <<
    vrtxData[i].pos.y << "\t" <<
    vrtxData[i].pos.z << "\t\t";
    if (i % pitch == pitch-1) of << endl;
  }
  of.close();
}
void logIndxData(
  std::vector<indx_t> &indxData,
  uint pitch,
  const char *path
) {
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < indxData.size(); i ++) {
    of << indxData[i] << '\t';
    if (i % pitch == pitch-1) of << endl;
  }
  of.close();
}
#endif













int main(int argc, char *argv[]) {
	vec2  videoSize = vec2(640, 480);//pixels
  vec4  gridRect  = vec4(-40, 30, 80, 60);//grid units
  float gridUnit  = 16;//pixels
  mat4  scaledTransform;
	
	SDL_Window    *window    = NULL;
	SDL_GLContext  GLcontext = NULL;
	SDL_Init(SDL_INIT_VIDEO);
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
	);
  if (!window)    cout << "SDL error: " << SDL_GetError() << endl;
	GLcontext = SDL_GL_CreateContext(window);
  if (!GLcontext) cout << "SDL error: " << SDL_GetError() << endl;
  
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
  //view_map_init
  {//originSquare
    const uint32_t color = 0xff114400;
    const uint size = 4;
    float corners[size*2] = {-1, 1,   1, 1,  -1,-1,   1,-1};
    originSquare.vrtx.resize(size);
    for (uint i = 0; i < size; i++) {
      originSquare.vrtx[i].pos.x = corners[i*2];
      originSquare.vrtx[i].pos.y = corners[i*2 + 1];
      originSquare.vrtx[i].color = color;
    }
    #if LOGGING_ENABLED
    logVrtxData(originSquare.vrtx, 3, "log/originSquareVrtxData.txt");
    #endif
  }
  {//gridLines
    const float    halfGridLine  = 1.0/gridUnit;//grid units
    const uint32_t gridLineColor = 0xff886622;//ABGR
    const int hrzlGridLineVrtxCount = (gridRect[2] + 1) * 4;
    const int vrtlGridLineVrtxCount = (gridRect[3] + 1) * 4;
    const int gridLineVrtxCount = hrzlGridLineVrtxCount + vrtlGridLineVrtxCount;
    gridLines.vrtx.resize(gridLineVrtxCount);
    int i = 0; float x = 0, y = 0;
    for (; i < hrzlGridLineVrtxCount; i += 4, x++) {
      gridLines.vrtx[i  ].color = gridLineColor;
      gridLines.vrtx[i  ].pos = vec2(
        x+gridRect[0]-halfGridLine, gridRect[1]
      );
      gridLines.vrtx[i+1].color = gridLineColor;
      gridLines.vrtx[i+1].pos = vec2(
        x+gridRect[0]+halfGridLine, gridRect[1]
      );
      gridLines.vrtx[i+2].color = gridLineColor;
      gridLines.vrtx[i+2].pos = vec2(
        x+gridRect[0]-halfGridLine, gridRect[1]-gridRect[3]
      );
      gridLines.vrtx[i+3].color = gridLineColor;
      gridLines.vrtx[i+3].pos = vec2(
        x+gridRect[0]+halfGridLine, gridRect[1]-gridRect[3]
      );
    }
    for (i = hrzlGridLineVrtxCount, y = 0; i < gridLineVrtxCount; i += 4, y++) {
      gridLines.vrtx[i  ].color = gridLineColor;
      gridLines.vrtx[i  ].pos = vec2(
        gridRect[0],             (y-gridRect[1])-halfGridLine
      );
      gridLines.vrtx[i+1].color = gridLineColor;
      gridLines.vrtx[i+1].pos = vec2(
        gridRect[0]+gridRect[2], (y-gridRect[1])-halfGridLine
      );
      gridLines.vrtx[i+2].color = gridLineColor;
      gridLines.vrtx[i+2].pos = vec2(
        gridRect[0],             (y-gridRect[1])+halfGridLine
      );
      gridLines.vrtx[i+3].color = gridLineColor;
      gridLines.vrtx[i+3].pos = vec2(
        gridRect[0]+gridRect[2], (y-gridRect[1])+halfGridLine
      );
    }
    #if LOGGING_ENABLED
    logVrtxData(gridLines.vrtx, 4, "log/gridLinesVrtxData.txt");
    #endif
  }
  scaledTransform = scale(
    mat4(), vec3((gridUnit*2)/videoSize.x, (gridUnit*2)/videoSize.y, 1.0)
  );
  compileShaders(shaderProgram, vsPath, fsPath);
  unif_transform = glGetUniformLocation(shaderProgram, "transform");_glec
  attr_pos   = glGetAttribLocation(shaderProgram, "pos");_glec
  attr_color = glGetAttribLocation(shaderProgram, "color");_glec
  glGenBuffers(1, &VBO);_glec
  glGenBuffers(1, &IBO);_glec
  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);_glec
  for (uint i = 0; i < vrtxGroups.size(); i++) {
    vrtxCount += vrtxGroups[i]->vrtx.size();
    vrtxGroups[i]->vboStart =
      !i ? 0 : vrtxGroups[i-1]->vboStart + vrtxGroups[i-1]->vrtx.size();
  }
  indxCount = vrtxCount * 1.5;
  indxAll.resize(indxCount);
  uint i = 0, v = 0;
  for (; i < indxCount; i += 6, v += 4) {
    indxAll[i  ] = v;
    indxAll[i+1] = v + 1;
    indxAll[i+2] = v + 2;
    indxAll[i+3] = v + 1;
    indxAll[i+4] = v + 2;
    indxAll[i+5] = v + 3;
  }
  #if LOGGING_ENABLED
  logIndxData(indxAll, 12, "log/indxAll.txt");
  #endif
  const int vboSize = sizeof(vertex)*vrtxCount;
  const int iboSize = sizeof(indx_t)*indxCount;
  glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_STREAM_DRAW);_glec
  for (uint i = 0; i < vrtxGroups.size(); i++) {
    glBufferSubData(
      GL_ARRAY_BUFFER,
      sizeof(vertex)*vrtxGroups[i]->vboStart,
      sizeof(vertex)*vrtxGroups[i]->vrtx.size(),
      (const GLvoid*)vrtxGroups[i]->vrtx.data()
    );_glec
  }
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    iboSize,
    indxAll.data(),
    GL_STREAM_DRAW
  );_glec
	
	
	
	
	
	
	
	bool  running  = true;
	//Uint32 timeout = SDL_GetTicks() + runTime;
	while (/*!SDL_TICKS_PASSED(SDL_GetTicks(), timeout) && */running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (
				event.type == SDL_QUIT ||
				(event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
			) {
				running = false;
			}
		}
		
		
		//view_map_draw
	  glClear(GL_COLOR_BUFFER_BIT);_glec
	  glUseProgram(shaderProgram);_glec
	  glUniformMatrix4fv(
	    unif_transform, 1, GL_FALSE, (const GLfloat*)&scaledTransform
	  );_glec
	  glBindBuffer(GL_ARRAY_BUFFER, VBO);_glec
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);_glec
	  glEnableVertexAttribArray(attr_pos);_glec
	  glEnableVertexAttribArray(attr_color);_glec
	  glVertexAttribPointer(
	    attr_pos,   2, GL_FLOAT,         false, 12, (const GLvoid*)0
	  );_glec
	  glVertexAttribPointer(
	    attr_color, 4, GL_UNSIGNED_BYTE, true,  12, (const GLvoid*)8
	  );_glec
	  glDrawElements(
	    GL_TRIANGLES, indxCount, GL_UNSIGNED_SHORT, (const GLvoid*)0
	  );_glec
	  glDisableVertexAttribArray(attr_pos);_glec
	  glDisableVertexAttribArray(attr_color);_glec
		
		
		
		
		
		SDL_GL_SwapWindow(window);
		SDL_Delay(10);
	}
	
	
	SDL_GL_DeleteContext(GLcontext);
	SDL_Quit();
	return 0;
}


