
#include <iostream>
using std::cout;
using std::endl;
#include <iostream>
#include <fstream>


#include "errorsAndLogging.hpp"

void glec(const int line, const char *file) {
  GLenum GLstatus;
  while ((GLstatus = glGetError()) != GL_NO_ERROR) {
    cout<<"OpenGL error: "<<GLstatus<<" on line "<<line<<" in "<<file<<endl;
  }
}
void sdlec(int line, const char *file) {
  const char *error = SDL_GetError();
  if (!error || !error[0]) return;
  cout << "SDL error at line " << line << " in " << file << endl
  << error << endl;
  SDL_ClearError();
  exit(-10);
}



void logVrtxData(
  std::vector<vertex> &vrtxData,
  uint pitch,
  const char *path
) {
#if LOGGING_ENABLED
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
#endif
}
void logIndxData(
  std::vector<indx_t> &indxData,
  uint pitch,
  const char *path
) {
#if LOGGING_ENABLED
  std::ofstream of;
  of.open(path);
  for (uint32_t i = 0; i < indxData.size(); i ++) {
    of << indxData[i] << '\t';
    if (i % pitch == pitch-1) of << endl;
  }
  of.close();
#endif
}


