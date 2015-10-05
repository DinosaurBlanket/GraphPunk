#pragma once

#include <glm/glm.hpp>
using glm::vec2;

class scrollable {
  float accel;
  float bumper;
  vec2  size;
  vec2  boundary;
  vec2  vel;
  vec2  pos;
  vec2  pPos;
  vec2  overBounds;
  vec2  pOverBounds;
  vec2  winSize;
  uint  lastAdvanced;//milliseconds
  vec2  posBR();
  vec2  pPosBR();
  void  resetBoundary();
public:
  void init(
    const float accelIn,
    const vec2  sizeIn,
    const vec2  initPos,
    const vec2  winSizeIn
  );
  void advance(float cursPress, float pCursPress, vec2 cursPos,vec2 pCursPos);
  vec2 getSize();
  void resize(vec2 newSize);
  vec2 getWinSize();
  vec2 getPos();
  bool hasMoved();
};
