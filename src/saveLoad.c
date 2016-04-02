
#include <stdlib.h>
#include <stdint.h>
#include "module.h"

//on disk there is:
//  a folder of module files named after their moduleID,
//  a list of moduleIDs that are in use,
//  maybe a file containing global state such as the last module they were in


typedef struct {
  uint32_t      moduleId;
  uint32_t      parentId;
  uint32_t      planeElemCount;
  float         planePos[2];
  moduleToggles toggles;
} diskModuleHeader;
//There are two chunks of data following the header, 
//  the planeElems(nodes and ports), same as in-memory
//  the positions(xy, bl) of each planeElem, 1/8 of what's in-memory(vertData)



// temorary declarations for data that will later come from a file
#define lastModuleId 4040 // arbitrary
const diskModuleHeader lastModuleHeader = {
  .moduleId       = lastModuleId,
  .parentId       = 0,
  .planeElemCount = 4,
  .planePos       = {0,0}
};
const planeElem lastModulePlaneElems[4] = {
  {
    .n.pei       = pei_aface,
    .n.oputCount = 1,
    .n.iputCount = 2,
    .n.lineVerts = 0,
    .n.id        = aid_add,
    .n.module    = NULL
  },
  {
    .p.pei  = pei_oport,
    .p.type = dt_n,
    .p.node = 0,
    .p.pos  = 0
  },
  {
    .p.pei  = pei_iport,
    .p.type = dt_n,
    .p.node = 0,
    .p.pos  = 0
  },
  {
    .p.pei  = pei_iport,
    .p.type = dt_n,
    .p.node = 0,
    .p.pos  = 1
  }
};
const float lastModulePositions[] = {
  16,16,
  16,32,
  16, 0,
  32, 0
};





mem     allModules;
module *lastModule  = NULL;


module *loadModule(uint32_t moduleId) {
  
}

void saveLoadInit(void) {
  // get the lastModuleId...
  lastModule = loadModule(lastModuleId);
}

module *getLastModule(void) {
  return lastModule;
}

void save(void) {}

void saveLoadExit(void) {
  memFree(&allModules);
}
