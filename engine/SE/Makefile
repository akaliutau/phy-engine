# SE/Makefile
#######################################################################
include ../Config.common
include ../Config.site

SUBDIRS = Plyfile Simplify

depend:
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i depend || exit 1 ; } ; done 

all:
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i all || exit 1 ; } ; done 

install:

uninstall:

cleanhere: cleangen

clean: cleanhere
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i clean || exit 1 ; } ; done

distclean: cleanhere
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i distclean || exit 1 ; } ; done

