CC      = gcc
CFLAGS  = -g -O2 -fomit-frame-pointer -Wall
LDFLAGS = -ljson-c -luuid -lblkid

VERSION := $(cat VERSION)

CFLAGS  += -DVERSION=\"$(VERSION)\"

PARTI_SRC = disk.c util.c eltorito.c filesystem.c json.c ptable_apple.c ptable_gpt.c ptable_mbr.c zipl.c
PARTI_OBJ = $(PARTI_SRC:.c=.o)
PARTI_H = $(PARTI_SRC:.c=.h)

all: parti

$(PARTI_OBJ) parti.o: %.o: %.c $(PARTI_H)
	$(CC) -c $(CFLAGS) $<

parti: parti.o $(PARTI_OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	rm -f *~ *.o parti
