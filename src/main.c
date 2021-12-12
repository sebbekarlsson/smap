#include <smap/smap.h>
#include <stdio.h>


int main(int argc, char* argv[]) {
  SourceMap smap = {};

  if (argc < 2) {
    fprintf(stderr, "Please specify input file.\n");
    return 1;
  }

  smap_load_file(&smap, argv[1]);

  printf("Map total length: %d\n", smap.total_length);

  for (int i = 0; i < 132; i++) {
    for (int j = 0; j < 80; j++) {
      LineInfo* info = smap_get_info_at(&smap, i, j);

      if (info) {
        printf(" %s (%d:%d) -> (%d:%d)\n", info->original_file_str, info->original_line_number, info->original_column, i, j);
      }
    }
  }

  smap_free(&smap);

  return 0;
}
