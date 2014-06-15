# graphicutils/Makefile
############################################################################

# use same configuration parameters as in the parent directory
include ../Config.common
include ../Config.site

############################################################################
# No need to change anything below here
############################################################################
# the source files in this directory
SOURCES = draw.c poly_scan.c poly_scan_flat.c poly_scan_z.c poly_clip.c \
	  line_scan.c line_clip.c
OBJECTS = ${SOURCES:.c=.o}

THISDIRFLAGS = -I.. -I../pools -I../math -I../geometry


#default target: compile
all: libdraw.a

# generate source file dependencies
depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SOURCES) > deps

# compile and link
libdraw.a: ${OBJECTS}
	$(AR) libdraw.a $(OBJECTS) 
	$(RANLIB) libdraw.a

install:

uninstall:

# make a clean source tree again
clean: cleangen
	-${RM} libdraw.a *.o *~ core deps
	${TOUCH} deps

distclean: clean

###############################################################################
# dependencies --- automatically generated with make depend !!
###############################################################################
include deps
