# mcsampling/Makefile
############################################################################

# use same configuration parameters as in the parent directory
include ../Config.common
include ../Config.site

# specific compile flags for this library
QMC_CFLAGS = 

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = faure.c halton.c nied31.c nied63.c scrambledhalton.c sobol.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = -I.. $(QMC_CFLAGS)



#default target: compile
all: libqmc.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libqmc.a: ${OBJECTS}
	$(AR) libqmc.a $(OBJECTS) 
	$(RANLIB) libqmc.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libqmc.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
