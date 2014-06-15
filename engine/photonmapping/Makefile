# photonmapping/Makefile 
############################################################################

# use same configuration parameters as in directory above 
include ../Config.common
include ../Config.site

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
CSRCS = 
CXXSRCS = pmap.C photonmap.C screensampler.C photon.C \
	pmapoptions.C ui_pmap.C importancemap.C photonkdtree.C pmapimportance.C

SOURCES = $(CXXSRCS) $(CSRCS)

OBJECTS = ${CXXSRCS:.C=.o} ${CSRCS:.c=.o}

THISDIRFLAGS = -I. -I.. -I../accelerators -I../pools -I../lighttransport -I../colors -I../geometry -I../generics -I../bbox -I../raytracing -I../imageutil

#default target: compile
all: libpmap.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libpmap.a: ${OBJECTS}
	$(AR) libpmap.a $(OBJECTS) 
	$(RANLIB) libpmap.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libpmap.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
