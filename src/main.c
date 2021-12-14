#include <smap/smap.h>
#include <smap/io.h>
#include <smap/macros.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <b64/b64.h>


int main(int argc, char* argv[]) {
  SourceMap smap = {};

  if (argc < 2) {
    fprintf(stderr, "Please specify input file.\n");
    return 1;
  }

  smap_load_file(&smap, argv[1]);

  printf("Map total length: %d\n", smap.total_length);
  printf("Nr sources: %d\n", smap.sources_length);

  FILE* fp = fopen("map.csv", "w+");

 const char* head = "o_file;o_line;o_col;s_row;s_col\n";
  fwrite(head, strlen(head), sizeof(char), fp);
  for (int i = 0; i < smap.lines; i++) {
    printf("Writing line %d / %d\n", i, smap.lines);
    LineInfo* line = smap.line_info[i];
    if (!line) continue;
    for (int j = 0; j < line->length; j++) {
      LineInfo* info = line->columns[j];//smap_get_info_at(&smap, i, j);

      if (!info) continue;

        const char* template = "%s;%d;%d;%d;%d\n";
        char*  file_str = OR(info->original_file_str, "");
        file_str = b64_encode(file_str);
        char buffline[PATH_MAX*16];
        fwrite(buffline, strlen(buffline), sizeof(char), fp);
        sprintf(buffline, template, file_str, info->original_line_number, info->original_column, i, j);


        if (file_str) {
          free(file_str);
        }
    }
  }


  fclose(fp);
  smap_free(&smap);

  return 0;
}
