

bin_PROGRAMS = eacsmb

# add modscan to the list below for automatic scanning
#  it kills rebuild performance
eacsmb_SOURCES = main.c \
	btree.c \
	builder/render.c \
	builder/core.c \
	builder/builder.c \
	building.c \
	bushes.c \
	c3dlas/c3dlas.c \
	c3dlas/meshgen.c \
	c_json/json.c \
	component.c \
	cursor.c \
	debugWireframes.c \
	decals.c \
	decalsCustom.c \
	ds.c \
	dumpImage.c \
	dynamicMesh.c \
	emitter.c \
	fbo.c \
	fcfg.c \
	font.c \
	game.c \
	gltf.c \
	gui.c \
	hash.c \
	input.c \
	itemLoader.c \
	jpeg.c \
	json_gl.c \
	lighting.c \
	loadingScreen.c \
	log.c \
	mdi.c \
	map.c \
	mapgen.c \
	marker.c \
	mempool.c \
	MurmurHash3.c \
	objloader.c \
	opensimplex.c \
	ordered_hash.c \
	pass.c \
	path.c \
	pcBuffer.c \
	perlin.c \
	pipe.c \
	qsort_r.c \
	qtree.c \
	renderLoop.c \
	road.c \
	scene.c \
	settings.c \
	sexp.c \
	shader.c \
	shadowMap.c \
	sim.c \
	sound.c \
	sound/soundgen.c \
	sound/utils.c \
	sound/vorbis.c \
	texgen.c \
	texture.c \
	textureAtlas.c \
	ui/animations/pulse.c \
	ui/columnLayout.c \
	ui/configLoader.c \
	ui/debugAdjuster.c \
	ui/edit.c \
	ui/gridLayout.c \
	ui/image.c \
	ui/monitors.c \
	ui/scrollWindow.c \
	ui/simpleWindow.c \
	ui/structAdjuster.c \
	ui/text.c \
	ui/tree.c \
	ui/window.c \
	uniformBuffer.c \
	utilities.c \
	waterPlane.c \
	window.c \
	world.c \
	worldEdit.c 

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype -lfontconfig -lpng \
	-lGL -lGLU -lGLEW 
	
# -ffast-math but without reciprocal approximations
AM_CFLAGS = "-I/usr/include/freetype2" -std=gnu11 -g -DLINUX \
	-DEACSMB_USE_SIMD \
	-DEACSMB_HAVE_SSE4 \
	-DEACSMB_HAVE_SSE41 \
	-DEACSMB_HAVE_AVX \
	-DEACSMB_HAVE_AVX2 \
	-msse4.1 -mavx -mavx2\
	-fno-math-errno \
	-fexcess-precision=fast \
	-fno-signed-zeros -fno-trapping-math -fassociative-math \
	-ffinite-math-only -fno-rounding-math \
	-fno-signaling-nans \
	-include "../config.h" \
	-include "./global_include.h" \
	-pthread \
	-Wno-unused-result \
	-Werror-implicit-function-declaration \
	-Wno-discarded-qualifiers
	
AM_CXXFLAGS = $(AM_CFLAGS)




