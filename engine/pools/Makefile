# POOLS/Makefile
############################################################################

include ../Config.common
include ../Config.site

# See poolsP.h for flags.
# define POOLFLAGS in ../Config.site
# POOLFLAGS = -DPOOL_ALIGN=4

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = pools.c error.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = $(POOLFLAGS)

#default target: compile
all: libpools.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libpools.a: ${OBJECTS}
	$(AR) libpools.a $(OBJECTS) 
	$(RANLIB) libpools.a

# make a clean source tree again
clean: cleangen
	-${RM} libpools.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
