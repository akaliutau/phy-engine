# Makefile for application.
#####################################################################
# Config.common contains platform and site independent configuration
# parameters and default that can be overriden in Config.site.
#####################################################################
include Config.common
include Config.site

# sources in this directory
CSRCS =  main.c options.c file.c batch.c ipc.c error.c \
	monitor.c canvas.c select.c \
	rendercommon.c render.c renderhook.c \
	tonemapping.c adaptation.c gamma.c lightmetering.c \
	camera.c scene.c vectorlist.c \
	bounds.c material.c materiallist.c \
	splitbsdf.c phong.c texture.c \
	hit.c hitlist.c grid.c geom.c geomlist.c compound.c surface.c \
	vertex.c vertexlist.c patch.c patchlist.c background.c \
	readmgf.c namedvertex.c vertexutils.c cluster.c color.c \
	geometry3d.c topology.c shadowcaching.c shaftculling.c \
	spherical.c potential.c radiance.c raytracing.c packedcolor.c \
	linetracer.c \
	ui_sdk.c ui_defaults.c ui_main.c ui_file.c ui_radiance.c ui_render.c \
	ui_raytracing.c ui_camera.c ui_help.c ui_tonemapping.c ui_config.c \
	ui_canvas.c ui_phong.c ui_dump.c \
	discretesampling.c \
	$(EXTRASRCS)
CXXSRCS = lightlist.C hemirenderer.C stratification.C samplegrid.C ui_pathdebug.C SoftIds.C background_edf.C
SRCS = $(CXXSRCS) $(CSRCS)

# object files in this directory
OBJS = $(CSRCS:.c=.o) $(CXXSRCS:.C=.o)

# Monte Carlo radiosity
MCRDIRS = montecarlo
ifeq ($(includeMonteCarloRadiosity),true)
MCRTARGETS = libmcrad
MCRLIBS = -L./montecarlo -lmcrad

ifeq ($(includeStochasticRelaxationRadiosity),true)
SRRFLAGS = -DSTOCHRELAX
endif

ifeq ($(includeWDRS),true)
WDRSFLAGS = -DWDRS
endif
ifeq ($(includeStochRay),true)
SRRFLAGS = -DSTOCHRAY
endif
ifeq ($(includeRandomWalkRadiosity),true)
RWRADFLAGS = -DRWRAD
endif
MCRFLAGS = $(WDRSFLAGS) $(SRRFLAGS) $(RWRADFLAGS) $(SRRFLAGS)

.PHONY: libmcrad

libmcrad: 
	${MAKE} -C montecarlo all
endif


# The PhotonMap
PHOTONMAPDIRS = photonmapping
ifeq ($(includePhotonMap),true)
PHOTONMAPFLAGS = -DPHOTONMAP
PHOTONMAPTARGETS = libpmap
PHOTONMAPLIBS = -L./photonmapping -lpmap

.PHONY: libpmap

libpmap: 
	${MAKE} -C photonmapping all
endif

# RADFLAGS are extra compilation flags for radiance.c.
# RADLIBS are command line arguments passed to CXX in order to 
# link in the libraries with the world-space radiance methods
# RADTARGETS is the list targets to make for the libraries mentionned
# in RADLIBS.
RADDIRS = $(MCRDIRS) $(PHOTONMAPDIRS) $(YUPDIRS)
RADFLAGS = $(MCRFLAGS) $(PHOTONMAPFLAGS) $(YUPFLAGS)
RADTARGETS = $(MCRTARGETS) $(PHOTONMAPTARGETS) $(YUPTARGETS)
RADLIBS = $(MCRLIBS) $(PHOTONMAPLIBS) $(YUPLIBS)

# Pixel-based radiance computations:

# Stochastic Raytracing and Bidirectional Path Tracing
RAYTRACINGDIRS = raytracing
ifeq ($(includeStochasticRayTracing),true)
SRTFLAGS = -DRT_STOCHASTIC
endif
ifeq ($(includeBidirectionalPathTracing),true)
BIDIRFLAGS = -DRT_BIDIR
endif
RAYTRACINGFLAGS = $(SRTFLAGS) $(BIDIRFLAGS)
RAYTRACINGTARGETS = libraytracing
RAYTRACINGLIBS = -L./raytracing -lraytracing

.PHONY: libraytracing

libraytracing:
	${MAKE} -C raytracing all

# RAYFLAGS are extra compilation flags for raytracing.c
# RAYLIBS determine the libraries to link in for the raytracing methods
# RAYTARGERTS is the list of targets to make the raytracing libs.
RAYDIRS = ${RAYTRACINGDIRS}
RAYFLAGS = $(RAYTRACINGFLAGS)
RAYTARGETS = $(RAYTRACINGTARGETS)
RAYLIBS = $(RAYTRACINGLIBS)


# tool libraries
TOOLDIRS =  accelerators MGF pools math colors geometry lighttransport generics bbox graphicutils mcsampling imageutil PNM tonemapping
TOOLFLAGS =  -I./accelerators -I./pools -I./math -I./lighttransport -I./colors -I./geometry -I./generics -I./bbox -I./imageutil -I./graphicutils
TOOLTARGETS = libaccelerators libmgf libpools libmath libcolors libgeometry libltransport libgdt libbbox libdraw libqmc libimage libtonemap
TOOLLIBS = -L./imageutil -limage -L./MGF -lmgf -L./graphicutils -ldraw -L./mcsampling -lqmc -L./bbox -lbrep -L./generics -lgdt -L./pools -lpools -L./math -lmath -L./colors -lcolors -L./lighttransport -lltransport -L./geometry -lgeometry -L./tonemapping -ltonemap -L./bbox -lbrep -L./accelerators -laccelerators

.PHONY: libaccelerators libmgf libpools libmath libcolors libltransport libgeometry libgdt libbbox libdraw libqmc libimage libtonemap

libmgf: 
	${MAKE} -C MGF all

libpools: 
	${MAKE} -C pools all
	
libaccelerators: 
	${MAKE} -C accelerators all
	
libltransport: 
	${MAKE} -C lighttransport all

libmath: 
	${MAKE} -C math all
	
libcolors: 
	${MAKE} -C colors all
	
libgeometry: 
	${MAKE} -C geometry all
	
libbbox: 
	${MAKE} -C bbox all

libgdt: 
	${MAKE} -C generics all

libdraw: 
	${MAKE} -C graphicutils all

libqmc:
	${MAKE} -C mcsampling all

libimage:
	${MAKE} -C imageutil all

libtonemap:
	${MAKE} -C tonemapping all


SUBDIRS = $(TOOLDIRS) $(RADDIRS) $(RAYDIRS)
LIBTARGETS = $(TOOLTARGETS) $(RADTARGETS) $(RAYTARGETS)

THISDIRLIBS = $(RAYLIBS) $(RADLIBS) $(TOOLLIBS)
THISDIRFLAGS = -I. $(TOOLFLAGS)



# default target: make phy2
all: phy2

list-targets:
	echo "SUBDIRS=" $(SUBDIRS)
	echo "LIBTARGETS=" $(LIBTARGETS)
	echo "THISLIBDIRS=" $(THISLIBDIRS)

# automatically generate source file dependencies

depend_here:
	$(MAKEDEPEND) $(MKDEPFLAGS) $(DRIVERFLAGS) $(SRCS) > deps

depend: render.c ui_defaults.c depend_here
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i depend || exit 1 ; } ; done

# special compilation flags are required to compile render.c
render.o: render.c 
	$(CC) $(CFLAGS) $(DRIVERFLAGS) -c render.c

ui_help.o: ui_help.c
	$(CC) $(CFLAGS) -DPHYHOME=${PHYHOME} -c ui_help.c

radiance.o: radiance.c Config.common
	$(CC) $(CFLAGS) $(RADFLAGS) -c radiance.c

raytracing.o: raytracing.c Config.common
	$(CC) $(CFLAGS) $(RAYFLAGS) -c raytracing.c

# automatically create a C file containing fallback resources from 
# the application defaults in the file 'Phy2'
ui_defaults.c: Phy2
	echo "/* ui_defaults.c: automatically generated from the application defaults file (Phy2)*/" > ui_defaults.c
	echo "" >> ui_defaults.c
	echo "char *fallback_resources[] = {" >> ui_defaults.c
	bash ad2c.sh < Phy2 >> ui_defaults.c
	echo "(char *)0" >> ui_defaults.c
	echo "};" >> ui_defaults.c

# make the executable program 'phy2'
phy2: $(LIBTARGETS) $(OBJS)
	$(CXX) $(LDFLAGS) -o phy2 $(OBJS) $(LIBS)

install:
	-mkdir $(BINDIR)
	$(INSTALL) $(TOPDIR)/phy2 $(BINDIR)

uninstall:
	-$(RM) $(BINDIR)/phy2

# clean up executable and object files
cleanhere: cleangen
	-$(RM) -f ui_defaults.c render.c phy2

clean: cleanhere
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i clean || exit 1 ; } ; done

distclean: cleanhere
	for i in $(SUBDIRS) ; do { $(MAKE) -C $$i distclean || exit 1 ; } ; done
	-$(RM) *.ppm *.mgf *.gz *.tif *.pic

# include the automatically generated source file dependencies
include deps
