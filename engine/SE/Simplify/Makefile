############;-*-fundamental-*-###############################################
# 
#               Makefile for Simplification Code
#               ================================
#
#############################################################################

include ../../Config.common
include ../../Config.site

# Useful paths
PROJ            = ..

# INCLUDE supplies paths to header files.
INCLUDE		= -I. -I$(PROJ)/include

# extra compiler options for this directory
THISDIRFLAGS	=  $(INCLUDE)

# extra linker flags (which is mostly just paths to libraries) for this directory
THISDIRLDFLAGS	=

# extra libraries linked to the program(s) in this directory
THISDIRLIBS	= -L$(PROJ)/lib -lply  -lm

#------------------------------ Lists of Files -------------------------------#

# Groups of source files
SIMPLIFY_SRCS = candidate.c			\
                fill_hole.c                     \
                geometry.c			\
       		intersect.c  			\
       		octree.c     			\
       		offset.c			\
		plysimplify.c			\
                removal_queue.c			\
	        simplify.c			\
                tubes.c				\
                vertex_removal.c

# Groups of object files
SIMPLIFY_OBJS = $(SIMPLIFY_SRCS:.c=.o)

#All executables
EXECUTABLES     = plysimplify


#--------------------------------- Main Rules --------------------------------#

#IMPORTANT - the object files must come BEFORE the libraries!!!
all: $(EXECUTABLES)

plysimplify: $(SIMPLIFY_OBJS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) $(SIMPLIFY_OBJS)  -o $@ $(LIBS)


#--------------------------------- Handy Rules -------------------------------#

# Remove any intermediate files.
clean : cleangen
	-$(RM) $(EXECUTABLES)

distclean: clean

# Strip and move the executables to the /bin directory.
install:
	strip $(EXECUTABLES); mv $(EXECUTABLES) $(PROJ)/bin

#-----------------------------Dependency Rules--------------------------------#

depend:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(SIMPLIFY_SRCS) >> deps

