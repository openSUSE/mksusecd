#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/fs.h>   /* BLKGETSIZE64 */

#include "util.h"
#include "disk.h" 

extern json_object *json_root;

unsigned disk_list_size;
disk_t *disk_list;

int disk_read(disk_t *disk, void *buffer, uint64_t block_nr, unsigned count)
{
  unsigned factor = disk->block_size / disk->chunk_size;

  // fprintf(stderr, "read: %llu - %u (factor = %u)\n", (unsigned long long) block_nr, count, factor);

  count *= factor;
  block_nr *= factor;

  for(unsigned u = 0; u < count; u++, block_nr++, buffer += disk->chunk_size) {
    // fprintf(stderr, "read request: disk %u, addr %08"PRIx64"\n", disk->index, block_nr * disk->chunk_size);
    if(!disk_cache_read(disk, buffer, block_nr)) {
      int err = disk_read_single(disk, buffer, block_nr);
      if(err) return err;
      disk_cache_store(disk, buffer, block_nr);
    }
  }

  return 0;
}


int disk_read_single(disk_t *disk, void *buffer, uint64_t block_nr)
{
  if(disk->fd == -1) {
    // fprintf(stderr, "cache miss: disk %u, addr %08"PRIx64"\n", disk->index, block_nr * disk->chunk_size);
    memset(buffer, 0, disk->chunk_size);

    return 0;
  }

  // fprintf(stderr, "read: %llu[%llu]\n", (unsigned long long) block_nr, (unsigned long long) disk->size);

  off_t offset = block_nr * disk->chunk_size;

  if(lseek(disk->fd, offset, SEEK_SET) != offset) {
    fprintf(stderr, "sector %"PRIu64" not found\n", block_nr);

    return 2;
  }

  if(read(disk->fd, buffer, disk->chunk_size) != disk->chunk_size) {
    fprintf(stderr, "error reading sector %"PRIu64"\n", block_nr);

    return 3;
  }

  return 0;
}


int disk_cache_read(disk_t *disk, void *buffer, uint64_t chunk_nr)
{
  for(disk_data_t *disk_data = disk->data; disk_data; disk_data = disk_data->next) {
    if(disk_data->chunk_nr == chunk_nr) {
      memcpy(buffer, disk_data->data, disk->chunk_size);
      return 1;
    }
  }

  return 0;
}


void disk_cache_store(disk_t *disk, void *buffer, uint64_t chunk_nr)
{
  // fprintf(stderr, "cache store: disk %u, addr %08"PRIx64"\n", disk->index, chunk_nr * disk->chunk_size);
  disk_data_t *disk_data = calloc(1, sizeof *disk_data);

  disk_data->chunk_nr = chunk_nr;
  disk_data->data = malloc(disk->chunk_size);
  memcpy(disk_data->data, buffer, disk->chunk_size);

  disk_data->next = disk->data;
  disk->data = disk_data;
}


int disk_export(disk_t *disk, char *file_name)
{
  FILE *f = stdout;

  if(strcmp(file_name, "-")) {
    f = fopen(file_name, "a");
    if(!f) {
      perror(file_name);
      return 1;
    }
  }

  fprintf(f, "# disk %u, size = %"PRIu64"\n", disk->index, disk->size_in_bytes);

  uint64_t chunk_nr = 0;
  disk_data_t *disk_data;

  do {
    disk_data = disk_cache_search(disk, &chunk_nr);
    if(disk_data) {
      disk_cache_dump(disk, disk_data, f);
    }
  }
  while(chunk_nr != UINT64_MAX);

  if(f != stdout) fclose(f);

  return 0;
}


int disk_to_fd(disk_t *disk, uint64_t offset)
{
  uint64_t chunk_nr = 0;
  disk_data_t *disk_data;

  int fd = syscall(SYS_memfd_create, "", 0);

  if(fd == -1) return 0;

  do {
    disk_data = disk_cache_search(disk, &chunk_nr);
    if(disk_data) {
      if(disk_data->chunk_nr * disk->chunk_size >= offset) {
        lseek(fd, disk_data->chunk_nr * disk->chunk_size - offset, SEEK_SET);
        write(fd, disk_data->data, disk->chunk_size);
      }
    }
  }
  while(chunk_nr != UINT64_MAX);

  lseek(fd, 0, SEEK_SET);

  return fd;
}


void disk_cache_dump(disk_t *disk, disk_data_t *disk_data, FILE *file)
{
  uint8_t all_zeros[16] = {};
  uint8_t *data = disk_data->data;

  uint64_t max_addr = disk->size_in_bytes - 1;
  unsigned address_digits = 0;
  while(max_addr >>= 4) address_digits++;
  if(address_digits < 4) address_digits = 4;

  for(unsigned u = 0; u < disk->chunk_size; u += 16) {
    if(memcmp(data + u, &all_zeros, 16)) {
      fprintf(file, "%0*"PRIx64" ", address_digits, disk_data->chunk_nr * disk->chunk_size + u);
      for(unsigned u1 = 0; u1 < 16; u1++) {
        fprintf(file, " %02x", data[u + u1]);
      }
      fprintf(file, "  ");
      for(unsigned u1 = 0; u1 < 16; u1++) {
        fprintf(file, "%c", data[u + u1] >= 32 && data[u + u1] < 0x7f ? data[u + u1] : '.');
      }
      fprintf(file, "\n");
    }
  }
}


disk_data_t *disk_cache_search(disk_t *disk, uint64_t *chunk_nr)
{
  disk_data_t *disk_data_found = NULL;
  uint64_t next_chunk_nr = UINT64_MAX;

  if(*chunk_nr == next_chunk_nr) return NULL;

  for(disk_data_t *disk_data = disk->data; disk_data; disk_data = disk_data->next) {
    if(disk_data->chunk_nr == *chunk_nr) {
      disk_data_found = disk_data;
    }
    if(disk_data->chunk_nr > *chunk_nr && disk_data->chunk_nr < next_chunk_nr) {
      next_chunk_nr = disk_data->chunk_nr;
    }
  }

  *chunk_nr = next_chunk_nr;

  return disk_data_found;
}


void disk_add_to_list(disk_t *disk)
{
  json_object *json;

  disk_list = reallocarray(disk_list, disk_list_size + 1, sizeof *disk_list);
  disk_list[disk_list_size] = *disk;
  disk_list[disk_list_size].index = disk_list_size;
  disk_list[disk_list_size].json_disk =
    disk_list[disk_list_size].json_current = json = json_object_new_object();
  json_object_array_add(json_root, json);

  disk_list_size++;

  log_info("%s: %"PRIu64" bytes\n", disk->name, disk->size_in_bytes);

  json_object *json_device = json_object_new_object();

  json_object_object_add(json, "device", json_device);
  json_object_object_add(json_device, "file_name", json_object_new_string(disk->name));
  json_object_object_add(json_device, "block_size", json_object_new_int(disk->block_size));
  json_object_object_add(json_device, "size", json_object_new_int64(disk->size_in_bytes / disk->block_size));
}


void disk_init(char *file_name)
{
  struct stat sbuf;
  disk_t disk = { .chunk_size = 512, .block_size = 512 };

  disk.fd = open(file_name, O_RDONLY | O_LARGEFILE);

  if(disk.fd == -1) {
    perror(file_name);
    exit(1);
  }

  disk.name = strdup(file_name);

  if(!fstat(disk.fd, &sbuf)) disk.size_in_bytes = sbuf.st_size;
  if(!disk.size_in_bytes && ioctl(disk.fd, BLKGETSIZE64, &disk.size_in_bytes)) disk.size_in_bytes = 0;

  disk_add_to_list(&disk);
}


void disk_import(char *file_name)
{
  FILE *file = fopen(file_name, "r");

  if(!file) {
    perror(file_name);
    exit(1);
  }

  char *line = NULL;
  size_t line_len = 0;
  unsigned line_nr = 0;

  disk_t disk = { .fd = -1 };

  uint8_t chunk[512];
  uint64_t current_chunk_nr = UINT64_MAX;

  while(getline(&line, &line_len, file) > 0) {
    line_nr++;
    unsigned index;
    uint64_t size;
    uint64_t addr;
    uint8_t line_data[16];
    if(sscanf(line, "# disk %u, size = %"SCNu64"", &index, &size) == 2) {
      if(disk.name) {
        if(current_chunk_nr != UINT64_MAX) disk_cache_store(&disk, chunk, current_chunk_nr);
        disk_add_to_list(&disk);
        disk = (disk_t) { .index = disk_list_size };
        current_chunk_nr = UINT64_MAX;
      }
      asprintf(&disk.name, "%s#%u", file_name, index);
      disk.size_in_bytes = size;
      disk.chunk_size = disk.block_size = 512;
    }
    else if(
      sscanf(line,
        "%"SCNx64" %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx",
        &addr,
        line_data, line_data + 1, line_data + 2, line_data + 3,
        line_data + 4, line_data + 5, line_data + 6, line_data + 7,
        line_data + 8, line_data + 9, line_data + 10, line_data + 11,
        line_data + 12, line_data + 13, line_data + 14, line_data + 15
      ) == 17 &&
      !(addr & 0xf) &&
      addr <= disk.size_in_bytes + 16
    ) {
      // fprintf(stderr, "XXX %08"PRIx64" %02x %02x\n", addr, line_data[0], line_data[1]);
      uint64_t chunk_nr = addr / disk.chunk_size;
      // fprintf(stderr, "ZZZ chunk_nr %"PRIu64", current_chunk_nr %"PRIu64"\n", chunk_nr, current_chunk_nr);
      if(chunk_nr != current_chunk_nr) {
        if(current_chunk_nr != UINT64_MAX) disk_cache_store(&disk, chunk, current_chunk_nr);
        current_chunk_nr = chunk_nr;
        memset(chunk, 0, sizeof chunk);
      }
      memcpy(chunk + (addr % disk.chunk_size), line_data, 16);
    }
    else {
      fprintf(stderr, "%s: line %u: invalid import data: %s\n", file_name, line_nr, line);
      exit(1);
    }
  }

  free(line);

  if(disk.name) {
    if(current_chunk_nr != UINT64_MAX) disk_cache_store(&disk, chunk, current_chunk_nr);
    disk_add_to_list(&disk);
  }
}
