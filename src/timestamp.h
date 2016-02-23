
#include <time.h>
typedef struct timespec timestamp;

int getTimestamp(timestamp *ts);

// This function assumes latter >= former
void getTimeDelta (timestamp *former, timestamp *latter, timestamp *delta);
