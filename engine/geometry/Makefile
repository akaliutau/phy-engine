# geometry/Makefile
############################################################################

# use same configuration parameters as in parent directory
include ../Config.common
include ../Config.site

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = vector.c vector2d.c transform.c

CXXSOURCES = 

OBJECTS = ${SOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I.. -I../pools -I../math


#default target: compile
all: libgeometry.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libgeometry.a: ${OBJECTS}
	$(AR) libgeometry.a $(OBJECTS) 
	$(RANLIB) libgeometry.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libgeometry.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
