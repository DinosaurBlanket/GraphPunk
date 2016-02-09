
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "fileTools.h"
#include "optionsAndErrors.h"

GLuint createShaderProgram(
  const char *vertPath, 
  const char *fragPath, 
  const char *progName
) {
  int vertSourceSize, fragSourceSize, textBufSize = 1024;
  
  vertSourceSize = getFileSize(vertPath);
  if (!vertSourceSize) {
    printf("error: could not open file \"%s\"\n", vertPath);
    return 0;
  }
  if (vertSourceSize > textBufSize) textBufSize = vertSourceSize + 1;
  
  fragSourceSize = getFileSize(fragPath);
  if (!fragSourceSize) {
    printf("error: could not open file \"%s\"\n", fragPath);
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
