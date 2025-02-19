CC      = gcc
CFLAGS  = -c -g -O2 -Wall -Wno-pointer-sign
LDFLAGS = -luuid

all: isohybrid

isohdpfx.o: isohdpfx.c
	$(CC) $(CFLAGS) -c $<

isohybrid.o: isohybrid.c isohybrid.h
	$(CC) $(CFLAGS) $<

isohybrid: isohybrid.o isohdpfx.o
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	@rm -f *.o *~ isohybrid
