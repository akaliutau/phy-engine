# accelerators/Makefile
############################################################################

# use same configuration parameters as in parent directory
include ../Config.common
include ../Config.site


############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = 
CXXSOURCES = balancekdtree.C kdtree.C

OBJECTS = ${SOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I.. -I../math -I../pools -I../geometry



#default target: compile
all: libaccelerators.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libaccelerators.a: ${OBJECTS}
	$(AR) libaccelerators.a $(OBJECTS) 
	$(RANLIB) libaccelerators.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libaccelerators.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
