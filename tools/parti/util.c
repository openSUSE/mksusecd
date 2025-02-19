#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "util.h"

opt_t opt;


char *cname(void *buf, int len)
{
  static char name[1024];
  int i;
  char *n;

  if(len > sizeof name - 1) len = sizeof name - 1;

  memcpy(name, buf, len);
  name[len] = 0;

  for(n = name, i = len - 1; i >= 0; i--) {
    if(n[i] == 0 || n[i] == 0x20 || n[i] == 0x09 || n[i] == 0x0a) {
      n[i] = 0;
    }
    else {
      break;
    }
  }

  return name;
}


unsigned read_byte(void *buf)
{
  unsigned char *b = buf;

  return b[0];
}


unsigned read_word_le(void *buf)
{
  unsigned char *b = buf;

  return (b[1] << 8) + b[0];
}


unsigned read_word_be(void *buf)
{
  unsigned char *b = buf;

  return (b[0] << 8) + b[1];
}


unsigned read_dword_le(void *buf)
{
  unsigned char *b = buf;

  return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + b[0];
}


unsigned read_dword_be(void *buf)
{
  unsigned char *b = buf;

  return (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];
}


uint64_t read_qword_le(void *buf)
{
  return ((uint64_t) read_dword_le(buf + 4) << 32) + read_dword_le(buf);
}


uint64_t read_qword_be(void *buf)
{
  return ((uint64_t) read_dword_be(buf) << 32) + read_dword_be(buf + 4);
}


void log_info(const char *format, ...)
{
  if(opt.json) return;

  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
}
