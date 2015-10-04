
#include <string>

void addShader(
  GLuint       shaderProgram,
  std::string &shaderText,
  GLenum       shaderType
);
void compileShaders(
  GLuint     &shaderProgram,
  const char *vsPath,
  const char *fsPath
);
