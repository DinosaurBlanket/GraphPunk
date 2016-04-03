
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
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
const diskModuleHeader lastModHeader = {
  .moduleId       = rootModId,
  .parentId       = 0,
  .planeElemCount = 4,
  .planePos       = {0,0}
};
const planeElem lastModPlaneElems[4] = {
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
const float lastModPlaneElemPositions[] = {
  16,16,
  16,32,
  16, 0,
  32, 0
};







module root = {
  .id       = rootModId,
  .parent   = NULL,
  .toggles  = {0},
  .faceRect = {0}
};

module *loadModules(uint32_t id) {
  module *m;
  if (id != rootModId) {
    // m = malloc module
    // load module data from disk
    // initialize id, parent, toggles, faceRect
  }
  else m = &root;
  
  // initialize some critical plane data
  m->plane.planeElemCount = lastModHeader.planeElemCount;
  if (!lastModHeader.planeElemCount) {
    m->plane.planeElemCap = 0;
    m->plane.planeElems = NULL;
    return m;
  }
  m->plane.planeElemCap = nextHighestPO2(lastModHeader.planeElemCount);
  m->plane.planeElems = malloc(m->plane.planeElemCap*sizeof(planeElem));
  fr(i, m->plane.planeElemCount) {
    m->plane.planeElems[i] = lastModPlaneElems[i];
    if (m->plane.planeElems[i].pei == pei_mface) {
      m->plane.planeElems[i].n.module = (struct module*)loadModules(m->plane.planeElems[i].n.id); // why the hell do i need to cast this??
    }
    // fill vert data from lastModPlaneElemPositions
    
  }
  return m;
}

// returns module you were in you last closed ...for now that will be root
module *saveLoadInit(void) {
  loadModules(rootModId);
  return &root;
}

void save(module *curModule) {}

void saveLoadExit(module *curModule) {
  // traverse module tree and free from bottom up
}
