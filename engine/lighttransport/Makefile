# lighttransport/Makefile
############################################################################

# use same configuration parameters as in parent directory
include ../Config.common
include ../Config.site

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = brdf.c bsdf.c btdf.c edf.c xxdf.c
CXXSOURCES = 

OBJECTS = ${SOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I.. -I../pools -I../math -I../geometry 



#default target: compile
all: libltransport.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libltransport.a: ${OBJECTS}
	$(AR) libltransport.a $(OBJECTS) 
	$(RANLIB) libltransport.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libltransport.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
