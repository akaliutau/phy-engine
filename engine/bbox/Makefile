# BBox/Makefile
############################################################################

# use same configuration parameters as CSGtool in parent directory
include ../Config.common
include ../Config.site

# the directory where the pools library is kept (only pools.h is needed)
POOLSDIR = ../pools

# the directory where the Generic Data types library is kept (only 
# Octree.h is needed. 
GDTDIR = ../generics

# specific compile flags for this library
EXTRA_CFLAGS = 

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = solid.c shell.c face.c contour.c edge.c vertex.c \
	  vertex_octree.c euler.c iterate.c error.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = -I$(POOLSDIR) -I$(GDTDIR) $(EXTRA_CFLAGS)



#default target: compile
all: libbrep.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libbrep.a: ${OBJECTS}
	$(AR) libbrep.a $(OBJECTS) 
	$(RANLIB) libbrep.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libbrep.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
