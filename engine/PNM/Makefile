# PNM/Makefile
############################################################################

# use same configuration parameters as in directory above 
include ../Config.common
include ../Config.site

# directory where the POOLS library header file pools.h can be found.
POOLSDIR = ../pools

############################################################################
# No need to change anything below this line
############################################################################
# the source files in this directory
SOURCES = common.c pnmread.c pnmwrite.c error.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = -I${POOLSDIR}



#default target: compile
all: libpnm.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libpnm.a: ${OBJECTS}
	$(AR) libpnm.a $(OBJECTS) 
	$(RANLIB) libpnm.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libpnm.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
