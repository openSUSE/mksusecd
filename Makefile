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

all: changelog isohybrid parti

isohybrid:
	@make -C tools/isohybrid

parti:
	@make -C tools/parti

archive: changelog
	@if [ ! -d .git ] ; then echo no git repo ; false ; fi
	mkdir -p package
	git archive --prefix=$(PREFIX)/ $(BRANCH) > package/$(PREFIX).tar
	tar -r -f package/$(PREFIX).tar --mode=0664 --owner=root --group=root --mtime="`git show -s --format=%ci`" --transform='s:^:$(PREFIX)/:' VERSION changelog
	xz -f package/$(PREFIX).tar

changelog: $(GITDEPS)
	$(GIT2LOG) --changelog changelog

install: isohybrid parti doc
	@cp mksusecd mksusecd.tmp
	@perl -pi -e 's/0\.0/$(VERSION)/ if /VERSION = /' mksusecd.tmp
	@perl -pi -e 's#"(.*)"#"$(LIBDIR)"# if /LIBEXECDIR = /' mksusecd.tmp
	@cp verifymedia verifymedia.tmp
	@perl -pi -e 's/0\.0/$(VERSION)/ if /VERSION = /' verifymedia.tmp
	@perl -pi -e 's#"(.*)"#"$(LIBDIR)"# if /LIBEXECDIR = /' verifymedia.tmp
	@cp isozipl isozipl.tmp
	@perl -pi -e 's/0\.0/$(VERSION)/ if /VERSION = /' isozipl.tmp
	install -m 755 -D mksusecd.tmp $(DESTDIR)$(BINDIR)/mksusecd
	install -m 755 -D verifymedia.tmp $(DESTDIR)$(BINDIR)/verifymedia
	install -m 755 -D isozipl.tmp $(DESTDIR)$(BINDIR)/isozipl
	install -m 755 -D tools/isohybrid/isohybrid $(DESTDIR)$(LIBDIR)/mksusecd/isohybrid
	install -m 755 -D tools/parti/parti $(DESTDIR)$(LIBDIR)/mksusecd/parti
	@rm -f mksusecd.tmp verifymedia.tmp isozipl.tmp

doc:
	@if [ -x /usr/bin/asciidoctor ] ; then \
	  asciidoctor -b manpage -a version=$(VERSION) mksusecd_man.adoc ;\
	  asciidoctor -b manpage -a version=$(VERSION) verifymedia_man.adoc ;\
	else \
	  a2x -f manpage -a version=$(VERSION) mksusecd_man.adoc ;\
	  a2x -f manpage -a version=$(VERSION) verifymedia_man.adoc ;\
	fi
# a2x -f docbook -a version=$(VERSION) mksusecd_man.adoc
# dblatex mksusecd_man.xml

clean:
	@make -C tools/isohybrid clean
	@make -C tools/parti clean
	@rm -f *.o *~ *.tmp */*~ mksusecd{.1,_man.xml,_man.pdf} verifymedia{.1,_man.xml,_man.pdf}
	@rm -rf package
