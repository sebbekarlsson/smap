#include <smap/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int smap_write_file(const char* filepath, const char* contents) {
  FILE* fp = fopen(filepath, "w+");
  if (!fp) {
    fprintf(stderr, "failed to open `%s` for writing.\n", filepath);
    return 0;
  }

  uint32_t len = strlen(contents ? contents : "");

  fwrite(contents ? contents : "", len, sizeof(char), fp);

  fclose(fp);

  return 1;
}
