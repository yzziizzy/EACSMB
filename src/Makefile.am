

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.c \
	hash.c \
	MurmurHash3.c \
	utilities.c \
	config.c \
	dumpImage.c \
	objloader.c \
	staticMesh.c \
	uniformBuffer.c \
	fbo.c \
	road.c \
	window.c \
	shader.c \
	game.c \
	renderLoop.c \
	gui.c \
	ds.c \
	cursor.c \
	scene.c \
	world.c \
	itemLoader.c \
	emitter.c \
	terrain.c \
	map.c \
	sim.c \
	texture.c \
	perlin.c \
	opensimplex.c \
	json_gl.c \
	c_json/json.c \
	c3dlas/c3dlas.c \
	c3dlas/meshgen.c \
	text/text.c \
	text/fcfg.c

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype -lfontconfig -lpng \
	-lGL -lGLU -lGLEW
	 
AM_CFLAGS = "-I/usr/include/freetype2" -g -DLINUX -Wno-unused-result
AM_CXXFLAGS = $(AM_CFLAGS)



