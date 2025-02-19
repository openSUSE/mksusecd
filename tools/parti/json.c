#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <json-c/json.h>

#include "json.h"
#include "util.h"

json_object *json_root;


void json_init()
{
  json_root = json_object_new_array();
}


void json_done()
{
  if(json_root) json_object_put(json_root);

  json_root = 0;
}


void json_print()
{
  if(!opt.json) return;

  json_object *json_obj = json_root;

  if(json_object_array_length(json_obj) == 1) json_obj = json_object_array_get_idx(json_obj, 0);

  int flags = JSON_C_TO_STRING_PRETTY + JSON_C_TO_STRING_NOSLASHESCAPE + JSON_C_TO_STRING_SPACED;

  json_object_to_fd(STDOUT_FILENO, json_obj, flags);

  printf("\n");
}


json_object *json_object_new_format(const char *format, ...)
{
  char *str = 0;

  va_list args;
  va_start(args, format);
  vasprintf(&str, format, args);
  va_end(args);

  json_object *json = json_object_new_string(str);

  free(str);

  return json;
}
