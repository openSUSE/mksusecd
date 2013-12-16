GIT2LOG	:= $(shell if [ -x ./git2log ] ; then echo ./git2log --update ; else echo true ; fi)
GITDEPS	:= $(shell [ -d .git ] && echo .git/HEAD .git/refs/heads .git/refs/tags)
VERSION	:= $(shell $(GIT2LOG) --version VERSION ; cat VERSION)
BRANCH	:= $(shell git branch | perl -ne 'print $$_ if s/^\*\s*//')
PREFIX	:= mksusecd-$(VERSION)

all:    changelog
	mkdir -p package
	git archive --prefix=$(PREFIX)/ $(BRANCH) | xz -c > package/$(PREFIX).tar.xz

changelog: $(GITDEPS)
	$(GIT2LOG) --changelog changelog

clean:
	@rm -rf *~ package

