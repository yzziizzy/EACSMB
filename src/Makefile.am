

bin_PROGRAMS = eacsmb

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
	font.c \
	game.c \
	gui.c \
	hash.c \
	input.c \
	itemLoader.c \
	json_gl.c \
	lighting.c \
	loadingScreen.c \
	log.c \
	mdi.c \
	map.c \
	mapgen.c \
	marker.c \
	MurmurHash3.c \
	objloader.c \
	opensimplex.c \
	ordered_hash.c \
	pass.c \
	path.c \
	pcBuffer.c \
	perlin.c \
	pipe.c \
	renderLoop.c \
	road.c \
	scene.c \
	settings.c \
	sexp.c \
	shader.c \
	shadowMap.c \
	sim.c \
	sound.c \
	staticMesh.c \
	text/fcfg.c \
	text/text.c \
	texgen.c \
	texture.c \
	textureAtlas.c \
	ui/columnLayout.c \
	ui/edit.c \
	ui/image.c \
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
	-lGL -lGLU -lGLEW \
	-lasound
	 
AM_CFLAGS = "-I/usr/include/freetype2" -std=gnu11 -g -DLINUX \
	-pthread \
	-Wno-unused-result \
	-Werror-implicit-function-declaration
	
AM_CXXFLAGS = $(AM_CFLAGS)



