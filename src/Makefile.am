

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.c \
	utilities.c \
	window.c \
	shader.c \
	game.c \
	ui.c \
	config.c \
	cursor.c \
	scene.c \
	map.c \
	texture.c \
	perlin.c \
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



