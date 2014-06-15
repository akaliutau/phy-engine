# generics/Makefile
############################################################################

# use same configuration parameters as in parent directory
include ../Config.common
include ../Config.site

# the directory where the pools library is kept (only pools.h is needed)
POOLSDIR = ../pools

# specific compile flags for this library
GDTFLAGS = 

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = List.c DList.c Bintree.c Octree.c Array.c error.c
CXXSOURCES = CSList.C

OBJECTS = ${SOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I$(POOLSDIR) $(GDTFLAGS)



#default target: compile
all: libgdt.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libgdt.a: ${OBJECTS}
	$(AR) libgdt.a $(OBJECTS) 
	$(RANLIB) libgdt.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libgdt.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
