CC      = gcc
CFLAGS  = -c -g -O2 -Wall -Wno-pointer-sign
LDFLAGS = -luuid

GIT2LOG	:= $(shell if [ -x ./git2log ] ; then echo ./git2log --update ; else echo true ; fi)
GITDEPS	:= $(shell [ -d .git ] && echo .git/HEAD .git/refs/heads .git/refs/tags)
VERSION	:= $(shell $(GIT2LOG) --version VERSION ; cat VERSION)
BRANCH	:= $(shell [ -d .git ] && git branch | perl -ne 'print $$_ if s/^\*\s*//')
PREFIX	:= mksusecd-$(VERSION)
BINDIR	 = /usr/bin
LIBDIR	 = /usr/lib

all: changelog isohybrid

isohdpfx.o: isohdpfx.c
	$(CC) $(CFLAGS) -c $<

isohybrid.o: isohybrid.c isohybrid.h
	$(CC) $(CFLAGS) $<

isohybrid: isohybrid.o isohdpfx.o
	$(CC) $^ $(LDFLAGS) -o $@

archive: changelog
	@if [ ! -d .git ] ; then echo no git repo ; false ; fi
	mkdir -p package
	git archive --prefix=$(PREFIX)/ $(BRANCH) > package/$(PREFIX).tar
	tar -r -f package/$(PREFIX).tar --mode=0664 --owner=root --group=root --mtime="`git show -s --format=%ci`" --transform='s:^:$(PREFIX)/:' VERSION changelog
	xz -f package/$(PREFIX).tar

changelog: $(GITDEPS)
	$(GIT2LOG) --changelog changelog

install: isohybrid
	@cp mksusecd mksusecd.tmp
	@perl -pi -e 's/0\.0/$(VERSION)/ if /VERSION = /' mksusecd.tmp
	@perl -pi -e 's#"(.*)"#"$(LIBDIR)"# if /LIBEXECDIR = /' mksusecd.tmp
	@cp isozipl isozipl.tmp
	@perl -pi -e 's/0\.0/$(VERSION)/ if /VERSION = /' isozipl.tmp
	install -m 755 -D mksusecd.tmp $(DESTDIR)$(BINDIR)/mksusecd
	install -m 755 -D isozipl.tmp $(DESTDIR)$(BINDIR)/isozipl
	install -m 755 -D isohybrid $(DESTDIR)$(LIBDIR)/mksusecd/isohybrid
	@rm -f mksusecd.tmp isozipl.tmp

clean:
	@rm -f *.o isohybrid
	@rm -rf *~ package

