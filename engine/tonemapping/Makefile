# tonemapping/Makefile
############################################################################

# use same configuration parameters as in directory above 
include ../Config.common
include ../Config.site

# compilation flags specific for this directory
TMCFLAGS = 

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory

SOURCES = dummy.c lightness.c trwf.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = -I. -I.. -I../pools -I../colors -I../geometry -I../math -I../lighttransport -I../generics -I../bbox -I../mcsampling $(TMCFLAGS)

#default target: compile
all: libtonemap.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libtonemap.a: ${OBJECTS}
	$(AR) libtonemap.a $(OBJECTS)
	$(RANLIB) libtonemap.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libtonemap.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
