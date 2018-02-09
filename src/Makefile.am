

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.c \
	builder/render.c \
	builder/core.c \
	builder/builder.c \
	c3dlas/c3dlas.c \
	c3dlas/meshgen.c \
	c_json/json.c \
	config.c \
	cursor.c \
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
	log.c \
	map.c \
	MurmurHash3.c \
	objloader.c \
	opensimplex.c \
	pass.c \
	pcBuffer.c \
	perlin.c \
	pipe.c \
	renderLoop.c \
	road.c \
	scene.c \
	shader.c \
	sim.c \
	staticMesh.c \
	terrain.c \
	text/fcfg.c \
	text/text.c \
	texture.c \
	ui/image.c \
	ui/edit.c \
	ui/simpleWindow.c \
	uniformBuffer.c \
	utilities.c \
	waterPlane.c \
	window.c \
	world.c 

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype -lfontconfig -lpng \
	-lGL -lGLU -lGLEW
	 
AM_CFLAGS = "-I/usr/include/freetype2" -std=gnu11 -g -DLINUX -Wno-unused-result
AM_CXXFLAGS = $(AM_CFLAGS)



