#include <stdint.h>

int getFileSize(const char *path);
int stringFromFile(const char *restrict path, char *dest, uint32_t maxWrite);
