# montecarlo/Makefile
############################################################################

# use same configuration parameters as in directory above 
include ../Config.common
include ../Config.site

# compilation flags specific for this directory
#
# -DIDMCR: include view importance-driven sampling
# -DHOMCR: include higher order radiosity approximations
# -DTESTS: include code for testing in tests.c (for research and debugging - experimental)
#
MCRADCFLAGS = -I..raytracing -I../imageutil -DIDMCR -DHOMCR # -DTESTS

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory

SOURCES = mcrad.c ui_mcrad.c tests.c \
	element.c hierarchy.c ui_hierarchy.c refine.c render.c \
	basis.c basisquad.c basistri.c coefficients.c pushpull.c \
	sample4d.c localline.c nondiff.c \
	stochrelax.c stochjacobi.c ccr.c ui_stochrelax.c \
	randwalk.c tracepath.c ui_randwalk.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = -I. -I.. -I../pools -I../lighttransport -I../colors -I../geometry -I../math -I../generics -I../bbox -I../mcsampling $(MCRADCFLAGS)

#default target: compile
all: libmcrad.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libmcrad.a: ${OBJECTS}
	$(AR) libmcrad.a $(OBJECTS) 
	$(RANLIB) libmcrad.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libmcrad.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
