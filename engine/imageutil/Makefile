# imageutil/Makefile
############################################################################

# use same configuration parameters as in the parent directory
include ../Config.common
include ../Config.site

# specific compile flags for this library
# -DNO_TIFF: to avoid inclusion of the TIFF output driver
# -DOLD_TIFF: define this if your TIFF library is pre v3.4beta037 (august 1997)
IMAGE_CFLAGS =

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
CSOURCES = dkcolor.c
CXXSOURCES = image.C ppm.C tiff.C pic.C
SOURCES = $(CSOURCES) $(CXXSOURCES)
OBJECTS = ${CSOURCES:.c=.o} ${CXXSOURCES:.C=.o}

THISDIRFLAGS = -I.. -I../colors $(TIFFLAGS) $(IMAGE_CFLAGS)



#default target: compile
all: libimage.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libimage.a: ${OBJECTS}
	$(AR) libimage.a $(OBJECTS) 
	$(RANLIB) libimage.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libimage.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
