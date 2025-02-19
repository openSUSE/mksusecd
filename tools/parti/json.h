#include <json-c/json.h>

void json_init();
void json_done();
void json_print();
json_object *json_object_new_format(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
