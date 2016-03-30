
#include <stdint.h>
#define  GLEW_STATIC
#include <GL/glew.h>

//on disk there is:
//  a folder of module files named after their moduleID,
//  a list of moduleIDs that are in use,
//  maybe a file containing state such as the last module they were in?


typedef struct {
  uint32_t moduleID;
  uint32_t planeElemCount;
  float    planePos[2];
} diskModuleHeader;
//There are two chunks of data following the header, 
//  the planeElems(nodes and ports), same as in-memory
//  the rects(xyxy, bl-tr) of each planeElem, 1/4 of whats in-memory(vertData)
