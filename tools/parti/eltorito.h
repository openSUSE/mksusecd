typedef union {
  struct {
    uint8_t header_id;
    uint8_t reserved[31];
  } any;

  struct {
    uint8_t header_id;
    uint8_t platform_id;
    uint16_t reserved1;
    char name[24];
    uint16_t crc;
    uint16_t magic;
  } validation;

  struct {
    uint8_t header_id;
    uint8_t info;
    uint8_t reserved[30];
  } extension;

  struct {
    uint8_t header_id;
    uint8_t media;
    uint16_t load_segment;
    uint8_t system;
    uint8_t reserved;
    uint16_t size;
    uint32_t start;
    uint8_t criteria;
    char name[19];
  } entry;

  struct {
    uint8_t header_id;
    uint8_t platform_id;
    uint16_t entries;
    char name[28];
  } section;
} eltorito_t;


void dump_eltorito(disk_t *disk);
