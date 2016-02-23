
#include "timestamp.h"

int getTimestamp(timestamp *ts) {return clock_gettime(CLOCK_MONOTONIC, ts);}

// This function assumes latter >= former
void getTimeDelta (timestamp *former, timestamp *latter, timestamp *delta) {
  delta->tv_sec = latter->tv_sec - former->tv_sec;
  if (former->tv_nsec > latter->tv_nsec) { // then we have to carry
    delta->tv_sec--;
    delta->tv_nsec = (1e9 - former->tv_nsec) + latter->tv_nsec;
  }
  else delta->tv_nsec = latter->tv_nsec - former->tv_nsec;
}
