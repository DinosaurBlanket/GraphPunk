
#define  GLEW_STATIC
#include <GL/glew.h>

GLuint createShaderProgram(
  const char *vertPath,
  const char *fragPath,
  const char *progName
);

void texFromBmp(const char *bmpPath);
