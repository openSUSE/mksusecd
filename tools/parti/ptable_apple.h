#define APPLE_MAGIC     0x504d

typedef struct {
  uint16_t signature;
  uint16_t reserved1;
  uint32_t partitions;
  uint32_t start;
  uint32_t size;
  char name[32];
  char type[32];
  uint32_t data_start;
  uint32_t data_size;
  uint32_t status;
} apple_entry_t;

void dump_apple_ptables(disk_t *disk);
int dump_apple_ptable(disk_t *disk);
