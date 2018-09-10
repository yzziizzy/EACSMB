

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.c \
	builder/render.c \
	builder/core.c \
	builder/builder.c \
	building.c \
	c3dlas/c3dlas.c \
	c3dlas/meshgen.c \
	c_json/json.c \
	component.c \
	config.c \
	cursor.c \
	decals.c \
	decalsCustom.c \
	ds.c \
	dumpImage.c \
	dynamicMesh.c \
	emitter.c \
	fbo.c \
	game.c \
	gui.c \
	hash.c \
	input.c \
	itemLoader.c \
	json_gl.c \
	lighting.c \
	log.c \
	map.c \
	mapgen.c \
	mdi.c \
	MurmurHash3.c \
	objloader.c \
	opensimplex.c \
	pass.c \
	path.c \
	pcBuffer.c \
	perlin.c \
	pipe.c \
	renderLoop.c \
	road.c \
	scene.c \
	sexp.c \
	shader.c \
	shadowMap.c \
	sim.c \
	staticMesh.c \
	text/fcfg.c \
	text/text.c \
	texgen.c \
	texture.c \
	ui/image.c \
	ui/edit.c \
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
	-Wno-unused-result \
	-Werror-implicit-function-declaration
	
AM_CXXFLAGS = $(AM_CFLAGS)



