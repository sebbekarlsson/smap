#ifndef SMAP_H
#define SMAP_H
#include <json/json.h>
#include <VLQ/VLQ.h>
#include <hashmap/map.h>

// {
// "version":3,
// "file":"app.js",
// "sourceRoot":"",
// "sources":["../src/app.ts"],
// "names":[],
//
// "mappings":";AAAA,OAAO,CAAC,GAAG,CAAC,OAAO,CAAC,CAAC"
// }


typedef struct SOURCE_MAP_MAPPINGS_STRUCT {
  uint32_t generated_column;
  uint32_t original_file;
  uint32_t original_line_number;
  uint32_t original_column;
  uint32_t original_name;

  char* generated_column_str;
  char* original_file_str;
  char* original_line_number_str;
  char* original_column_str;
  char* original_name_str;

  struct SOURCE_MAP_MAPPINGS_STRUCT** columns;
  uint32_t length;
} LineInfo;

typedef struct SOURCE_MAP_STRUCT {
  char* version;
  char* file;
  char* source_root;
  char** sources;
  uint32_t sources_length;
  char** names;
  uint32_t names_length;
  char* mappings;
  VLQDecodeResult* mappings_decoded;
  LineInfo** line_info;
  uint32_t lines;
  uint32_t  total_length;
  map_T* map;
} SourceMap;


VLQDecodeResult*  smap_decode_mappings(SourceMap* source_map);

int smap_load(SourceMap* smap, char* contents);
int smap_load_file(SourceMap* smap, const char* filepath);

LineInfo* smap_get_info_at(SourceMap* smap, uint32_t line, uint32_t col);

void smap_line_info_free(LineInfo* info);
void smap_free(SourceMap* smap);

#endif
