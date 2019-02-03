

bin_PROGRAMS = eacsmb

# add modscan to the list below for automatic scanning
#  it kills rebuild performance
eacsmb_SOURCES = main.c \
	btree.c \
	builder/render.c \
	builder/core.c \
	builder/builder.c \
	building.c \
	c3dlas/c3dlas.c \
	c3dlas/meshgen.c \
	c_json/json.c \
	component.c \
	cursor.c \
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
	sound/vorbis.c \
	staticMesh.c \
	texgen.c \
	texture.c \
	textureAtlas.c \
	ui/animations/pulse.c \
	ui/columnLayout.c \
	ui/configLoader.c \
	ui/edit.c \
	ui/gridLayout.c \
	ui/image.c \
	ui/monitors.c \
	ui/scrollWindow.c \
	ui/simpleWindow.c \
	ui/text.c \
	ui/window.c \
	uniformBuffer.c \
	utilities.c \
	waterPlane.c \
	window.c \
	world.c 

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype -lfontconfig -lpng \
	-lGL -lGLU -lGLEW 
	 
AM_CFLAGS = "-I/usr/include/freetype2" -std=gnu11 -g -DLINUX \
	-msse4.1 -mavx \
	-include "../config.h" \
	-pthread \
	-Wno-unused-result \
	-Werror-implicit-function-declaration
	
AM_CXXFLAGS = $(AM_CFLAGS)




