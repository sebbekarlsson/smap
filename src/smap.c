#include <math.h>
#include <smap/smap.h>
#include <smap/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

int smap_load(SourceMap *smap, char *contents) {

  if (!contents) {
    fprintf(stderr, "smap_load: contents is NULL.\n");
    return 0;
  }

//  printf("smap_load: Reading data...\n");
  JSONOptions options = {};
  options.optimized_strings = 1;
  JSON *data = json_parse(contents, &options);
 // printf("smap_load: Done reading data.\n");

  smap->version = maybe_copy_str(json_get_string(data, "version"));
  smap->source_root = maybe_copy_str(json_get_string(data, "sourceRoot"));

 // printf("Extracting information...\n");

  {
    JSONIterator it = json_get_array(data, "names");
    JSON* val = 0;
    smap->names_length = it.length;
    uint32_t i = 0;
    smap->names = (char **)calloc(smap->names_length, sizeof(char *));
    if (!smap->names) {
        fprintf(stderr, "Failed to allocate memory smap->names.\n");
    }
    while ((val = json_iterator_next(&it)) != 0)
    {
      char *value = val->value_str;
        if (!value)
          continue;
        smap->names[i] = maybe_copy_str(value);

        i++;
    }
  }

   {
     JSONIterator it = json_get_array(data, "sourcesContent");

     if (it.length) {
      JSON* val = 0;
      smap->source_contents_length = it.length;
      uint32_t i = 0;
      smap->source_contents = (char **)calloc(smap->source_contents_length, sizeof(char *));
      if (!smap->source_contents) {
          fprintf(stderr, "Failed to allocate memory smap->source_contents.\n");
      }
      while ((val = json_iterator_next(&it)) != 0 && (i < it.length))
      {
        char *value = val->value_str;
          if (!value)
            continue;
          smap->source_contents[i] = maybe_copy_str(value);


          i++;
      }
     }
  }

  {
    JSONIterator it = json_get_array(data, "sources");
    JSON* val = 0;
    smap->sources_length = it.length;
    uint32_t i = 0;
    smap->sources = (char **)calloc(smap->sources_length, sizeof(char *));
    if (!smap->sources) {
      fprintf(stderr, "Failed to allocate memory smap->sources.\n");
    }
    while ((val = json_iterator_next(&it)) != 0)
    {
      char *value = val->value_str;
        if (!value)
          continue;
        smap->sources[i] = maybe_copy_str(value);

        i++;
    }
  }

  smap->file = maybe_copy_str(json_get_string(data, "file"));
  smap->mappings = maybe_copy_str(json_get_string(data, "mappings"));

  //printf("Done extracting information.\n");

  //printf("Decoding mappings...\n");
  if (smap->mappings) {
    smap->mappings_decoded = smap_decode_mappings(smap);
  }
  //printf("Done decoding mappings.\n");

  if (!smap->map)
    smap->map = NEW_MAP();

  VLQDecodeResult *result = smap->mappings_decoded;

  if (result) {
    smap->lines = result->length;
    smap->line_info = (LineInfo **)calloc(result->length + 1, sizeof(LineInfo *));
    if (!smap->line_info) {
      fprintf(stderr, "Failed to allocate smap->line_info.\n");
    }
  } else {
    fprintf(stderr, "result == 0\n");
    return 0;
  }

  //printf("Mapping information...\n");
  for (uint32_t line_nr = 0; line_nr < result->length; line_nr++) {
    //printf("smap_load: line %d / %d\n", line_nr, result->length);
    VLQLine *line = result->lines[line_nr];
    if (!line)
      continue;
    LineInfo *line_info = (LineInfo *)calloc(1, sizeof(LineInfo));
    if (!line_info) {
      fprintf(stderr, "failed to allocate line_info in loop.\nl");
      continue;
    }
    line_info->columns = (LineInfo **)calloc(line->length, sizeof(LineInfo *));
    if (!line_info->columns) {
      fprintf(stderr, "failed to allocate line_info->columns.\n");
      continue;
    }
    line_info->length = line->length;
    smap->line_info[line_nr] = line_info;
    smap->total_length += line->length;
    for (uint32_t seg_nr = 0; seg_nr < line->length; seg_nr++) {
      VLQSegment *segment = line->segments[seg_nr];
      if (!segment) continue;

      //uint32_t v = 0;

      uint32_t original_file =
          segment->original_source; // generated_column + segment->values[1];
      uint32_t original_line_number =
          segment->original_line; // segment->values[2];
      uint32_t original_column = segment->original_column; // segment->values[3];
      uint32_t original_name = segment->length >= 5 ? segment->values[4] : 0;
      uint32_t generated_line = segment->generated_line;
      uint32_t generated_column = segment->generated_column;

      LineInfo *info = calloc(1, sizeof(LineInfo));
      if (!info) {
        fprintf(stderr, "failed to allocate segment info.\n");
      }

      info->generated_column = generated_column;
      info->original_file = original_file;
      info->original_line_number = original_line_number;
      info->original_column = original_column;
      info->original_name = original_name;

      if (smap->sources) {
        info->original_file_str = maybe_copy_str(smap->sources[original_file]);
      }

      if (smap->names && smap->names_length) {
        int kk = original_name =  MAX(0, MIN(smap->names_length-1, original_name));
        info->original_name_str = smap->names[kk];
      }

      line_info->columns[seg_nr] = info;

      char buff[256];
      sprintf(buff, "%d:%d", generated_line, generated_column);
      map_set(smap->map, buff, info);
    }

    char buff[256];
    sprintf(buff, "%d", line_nr);
    map_set(smap->map, buff, line_info);
  }

  // printf("Done mapping information.\n");

  json_free(data);
  vlq_decode_result_free(result);

  return 1;
}
int smap_load_file(SourceMap *smap, const char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    fprintf(stderr, "Could not open file `%s`\n", filepath);
    return 0;
  }

  printf("smap_load_file: reading file...\n");
  char *buffer = NULL;
  size_t len;
  ssize_t bytes_read = getdelim(&buffer, &len, '\0', fp);
  if (!(bytes_read != -1) || buffer == 0) {
    fprintf(stderr, "smap_load_file: Read zero bytes.\n");
    return 0;
  }
  printf("smap_load_file: done reading file.\n");

  return smap_load(smap, buffer);
}

VLQDecodeResult *smap_decode_mappings(SourceMap *source_map) {
  if (!source_map) {
    fprintf(stderr, "source_map = NULL.\n");
    return 0;
  }
  if (!source_map->mappings) {
    fprintf(stderr, "mappings = NULL.\n");
    return 0;
  }

  VLQDecodeResult *result =
      (VLQDecodeResult *)calloc(1, sizeof(VLQDecodeResult));
  vlq_decode_into(result, source_map->mappings);

  return result;
}

LineInfo *smap_get_info_at(SourceMap *smap, uint32_t line, uint32_t col) {
  char buff[256];
  // line -= 1;
  // col -= 1;
  sprintf(buff, "%d:%d", line, col);
  LineInfo *info = (LineInfo *)map_get_value(smap->map, buff);

  if (!info && line < smap->lines) {
    LineInfo *line_info = smap->line_info[line];

    if (line_info && col < line_info->length) {
      info = line_info->columns[col];
    }
  }
  return info;
}

LineInfo *smap_get_info_at_line(SourceMap *smap, uint32_t line) {
  char buff[256];
  sprintf(buff, "%d", line);
  LineInfo *info = (LineInfo *)map_get_value(smap->map, buff);
  return info;
}


void smap_line_info_free(LineInfo* info) {
  if(!info) return;
  if (info->columns) {
    for (uint32_t i = 0; i < info->length; i++) {
      smap_line_info_free(info->columns[i]);
    }

    free(info->columns);
  }

  free(info);
}
void smap_free(SourceMap* smap) {
  if (!smap) return;
  if (smap->line_info) {
    for (uint32_t i = 0; i < smap->lines; i++) {
      smap_line_info_free(smap->line_info[i]);
    }
    free(smap->line_info);
  }

  if (smap->map) {
    map_free(smap->map);
  }

  if (smap->mappings) free(smap->mappings);

  if (smap->mappings_decoded) {
    vlq_decode_result_free(smap->mappings_decoded);
  }

  if (smap->sources) {
    for (uint32_t i = 0; i < smap->sources_length; i++) {
      free(smap->sources[i]);
    }
    free(smap->sources);
  }

  if (smap->names) {
    for (uint32_t i = 0; i < smap->names_length; i++) {
      free(smap->names[i]);
    }
    free(smap->names);
  }

  if (smap->source_root) free(smap->source_root);

  if (smap->version) free(smap->version);

  if (smap->file) free(smap->file);
}
