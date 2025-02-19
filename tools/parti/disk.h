#include <json-c/json.h>

typedef struct disk_data_s {
  struct disk_data_s *next;
  uint64_t chunk_nr;
  uint8_t *data;
} disk_data_t;

typedef struct {
  char *name;
  int fd;
  unsigned index;
  unsigned heads;
  unsigned sectors;
  unsigned cylinders;
  uint64_t size_in_bytes;
  unsigned chunk_size;
  unsigned block_size;
  unsigned grub_used:1;
  unsigned isolinux_used:1;
  disk_data_t *data;
  json_object *json_disk;
  json_object *json_current;
} disk_t;

extern unsigned disk_list_size;
extern disk_t *disk_list;

int disk_read(disk_t *disk, void *buf, uint64_t sector, unsigned cnt);
int disk_read_single(disk_t *disk, void *buffer, uint64_t block_nr);

int disk_cache_read(disk_t *disk, void *buffer, uint64_t chunk_nr);
void disk_cache_dump(disk_t *disk, disk_data_t *disk_data, FILE *file);
void disk_cache_store(disk_t *disk, void *buffer, uint64_t chunk_nr);
disk_data_t *disk_cache_search(disk_t *disk, uint64_t *chunk_nr);

int disk_export(disk_t *disk, char *file_name);
int disk_to_fd(disk_t *disk, uint64_t offset);
void disk_add_to_list(disk_t *disk);
void disk_init(char *file_name);
void disk_import(char *file_name);
