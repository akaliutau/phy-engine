# Makefile for MGF parser library
##############################################################################
include ../Config.common
include ../Config.site

# compile flags specific for this directory */
MGFFLAGS = -Dregister= -Dlint

##############################################################################
OBJS = parser.o context.o xf.o object.o lookup.o badarg.o words.o fvect.o
SRCS = ${OBJS:.o=.c}

THISDIRFLAGS = $(MGFFLAGS)


all: libmgf.a

depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SRCS) > deps	

libmgf.a:	$(OBJS)
	$(AR) libmgf.a $(OBJS)
	$(RANLIB) libmgf.a

install:

uninstall:

clean: cleangen
	rm -f *.o libmgf.a mgfilt *~ deps 
	$(TOUCH) deps

distclean: clean

##############################################################################
