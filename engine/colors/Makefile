# colors/Makefile
############################################################################

# use same configuration parameters as in parent directory
include ../Config.common
include ../Config.site


############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = cie.c
CXXSOURCES = 

OBJECTS = ${SOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I../math



#default target: compile
all: libcolors.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libcolors.a: ${OBJECTS}
	$(AR) libcolors.a $(OBJECTS) 
	$(RANLIB) libcolors.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libcolors.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
