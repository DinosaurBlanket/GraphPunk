
#include "nodeDef.h"

void getNodeDef(nodeDef *def, nodeId id) {
  switch(id) {
    case nid_add:
    case nid_sub:
    case nid_mul:
    case nid_div:
      def.outType      = dt_num;
      def.inletCount   = 2;
      def.inTypes[0]   = dt_num;
      def.inTypes[1]   = dt_num;
      def.extraPECount = 0;
      return;
    case nid_numlit7:
      def.outType      = dt_num;
      def.inletCount   = 0;
      def.extraPECount = 8; // 7 numerals + base numeral
      return;
    default:_SHOULD_NOT_BE_HERE_;
  }
}
