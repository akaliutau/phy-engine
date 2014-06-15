############;-*-fundamental-*-###############################################
# 
#               Makefile for PLY library and simple applications
#               ================================================
#
#############################################################################


# These are the names of various programs used during "making".
# Pick whichever flavor of C you like with the CC variable.
include ../../Config.common
include ../../Config.site

# Useful paths
PROJ            = ..

# INCLUDE supplies paths to header files.
INCLUDE		= -I.

# CFLAGS is all of the compiler options.  Put any extras here.
THISDIRFLAGS	=  $(INCLUDE)

# LDFLAGS contains all linker flags, which is mostly just paths to libraries.
THISDIRLDFLAGS	=

# These are the libraries linked to the program.  example: -lm
THISDIRLIBS	=
#-lmalloc

#------------------------------ Lists of Files -------------------------------#

# Groups of source files
PLY_SRCS = plybound.c				\
	   plycomps.c				\
	   plyconvert.c				\
	   plycrunch.c				\
	   plyfile.c				\
	   plyflip.c				\
	   plyindex.c				\
	   plymark.c				\
	   plynormals.c				\
	   plyorient.c				\
	   plyremove.c				\
	   plyremoveface.c			\
	   plyremovevert.c			\
	   plyshared.c				\
	   plyskel.c				\
	   plytriangulate.c			\
	   plyunshared.c			\
	   plyvertepsilons.c			\
	   plyxform.c				

#All executables
EXECUTABLES       = plybound				\
	   	    plycomps				\
	   	    plycrunch				\
	   	    plycurvaturecolor			\
	   	    plycurvatures			\
	   	    plyflip				\
	   	    plyindex				\
	   	    plymark				\
	   	    plynormals				\
	   	    plyorient				\
	   	    plyremove				\
	   	    plyremoveface			\
	   	    plyremovevert			\
	   	    plyshared				\
	   	    plyskel				\
	   	    plytriangulate			\
	   	    plyunshared				\
	   	    plyvertepsilons			\
	   	    plyxform				


#The compiled library
LIB = libply.a

#--------------------------------- Main Rules --------------------------------#

#IMPORTANT - the object files must come BEFORE the libraries!!!

all: $(LIB) $(EXECUTABLES)

libply.a: plyfile.c ply.h
	$(CC) $(CFLAGS) -o plyfile.o -c plyfile.c
	$(AR) libply.a plyfile.o 
	$(RANLIB) libply.a
	-$(RM) plyfile.o
	$(CP) libply.a ../lib

ply2ascii: plyconvert.c $(LIB) $(MAKEFILE)
# AFS on HPs won't overwrite executable, so remove
	-$(RM) $@    
	$(CC) $(CFLAGS) -o $@ plyconvert.c -DWRITE_ASCII $(LIB) $(LIBS)

ply2binary: plyconvert.c $(LIB) $(MAKEFILE)
# AFS on HPs won't overwrite executable, so remove first
	-$(RM) $@    
	$(CC) $(CFLAGS) -o $@ plyconvert.c -DWRITE_BINARY $(LIB) $(LIBS)


$(EXECUTABLES) : $(LIB)
# AFS on HPs won't overwrite executable, so remove first
	-$(RM) $@    
	$(CC) $(CFLAGS) -o $@ $@.c $(LIB) $(LIBS)

#--------------------------------- Handy Rules -------------------------------#

# Remove any intermediate files.
clean : cleangen
	-$(RM) $(EXECUTABLES) $(LIB)

distclean: clean

# Strip and move the executables to the /bin directory.
install:
	strip $(EXECUTABLES); mv $(EXECUTABLES) $(PROJ)/bin
	mv $(LIB) $(PROJ)/lib
	$(RM) $(PROJ)/include/ply.h; cp ply.h $(PROJ)/include

#-----------------------------Dependency Rules--------------------------------#

depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(PLY_SRCS) >> deps

include deps
