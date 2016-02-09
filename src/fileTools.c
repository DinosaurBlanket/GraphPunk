
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

int getFileSize(const char *path) {
  struct stat st;
  stat(path, &st);
  return st.st_size;
}


int stringFromFile(const char *restrict path, char *dest, uint32_t maxWrite) {
  if (!dest || !path || maxWrite < 1) return 0;
  FILE *fp = fopen(path, "r");
  if (!fp) {
    printf("error: could not open file \"%s\"\n", path);
    return 0;
  }
  int c, i = 0;
  for (; i < maxWrite; i++) {
    c = fgetc(fp);
    if (c == EOF) {
      dest[i] = '\0';
      break;
    }
    dest[i] = c;
  }
  fclose(fp);
  return i;
}
