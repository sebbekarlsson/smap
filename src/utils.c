#include <smap/utils.h>
#include <string.h>

char* maybe_copy_str(char* v) {
  if (!v) return 0;
  return strdup(v);
}
