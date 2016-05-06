
#include "nodeDef.h"

void getNodeDef(nodeDef *def, nodeId id) {
  switch(id) {
    case nid_add:
    case nid_sub:
    case nid_mul:
    case nid_div:
      def->outType      = dt_num;
      def->inletCount   = 2;
      def->inTypes[0]   = dt_num;
      def->inTypes[1]   = dt_num;
      def->extraPECount = 0;
      def->ndodCount    = 4;
      return;
    case nid_numlit8:
      def->outType      = dt_num;
      def->inletCount   = 0;
      def->extraPECount = 8; // 6 for numerals and radix point, 1 for number-base divider, 1 for base numeral
      def->ndodCount    = 3;
      return;
    case nid_output:
      def->outType      = dt_noOut;
      def->inletCount   = 1;
      def->inTypes[0]   = dt_num;
      def->extraPECount = 0;
      def->ndodCount    = 3;
      return;
    default:_SHOULD_NOT_BE_HERE_;
  }
}
