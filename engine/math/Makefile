# math/Makefile
############################################################################

# use same configuration parameters as in parent directory
include ../Config.common
include ../Config.site

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = jacobian.c volume.c

CXXSOURCES = 

OBJECTS = ${SOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I../pools


#default target: compile
all: libmath.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libmath.a: ${OBJECTS}
	$(AR) libmath.a $(OBJECTS) 
	$(RANLIB) libmath.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libmath.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
