# raytracing/Makefile 
############################################################################

# use same configuration parameters as in directory above 
include ../Config.common
include ../Config.site

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
ifeq ($(includeStochasticRayTracing),true)
RTSCXXSRCS = rtstochastic.C rtsoptions.C
RTSCSRCS = ui_rtstochastic.c ui_raymatting.c
endif
ifeq ($(includeBidirectionalPathTracing),true)
BIDIRCXXSRCS = bidirpath.C bidiroptions.C densitybuffer.C densitykernel.C
BIDIRCSRCS = ui_bidirpath.c
endif

CSRCS = $(RTSCSRCS) $(BIDIRCSRCS)

CXXSRCS = pathnode.C bipath.C raytools.C sampler.C eyesampler.C \
	pixelsampler.C lightsampler.C screenbuffer.C screeniterate.C \
	bsdfsampler.C specularsampler.C samplertools.C lightdirsampler.C \
	flagchain.C spar.C bsdfcompsampler.C photonmapsampler.C \
	raycasting.C raymatting.C pixelfilter.C rmoptions.C \
	$(RTSCXXSRCS) $(BIDIRCXXSRCS)

SOURCES = $(CXXSRCS) $(CSRCS)

OBJECTS = ${CXXSRCS:.C=.o} ${CSRCS:.c=.o}

THISDIRFLAGS = -I. -I.. -I../pools -I../math -I../lighttransport -I../geometry -I../generics -I../bbox -I../imageutil -I../mcsampling -I../photonmapping -I../graphicutils

#default target: compile
all: libraytracing.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libraytracing.a: ${OBJECTS}
	$(AR) libraytracing.a $(OBJECTS) 
	$(RANLIB) libraytracing.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libraytracing.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
