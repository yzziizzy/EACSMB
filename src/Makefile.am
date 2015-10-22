

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.c \
	window.c \
	shader.c \
	game.c \
	texture.c \
	c3dlas/c3dlas.c \
	text/text.c

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype -lpng \
	-lGL -lGLU -lGLEW 
	 
AM_CFLAGS = "-I/usr/include/freetype2" -g -DLINUX -Wno-unused-result -O0
AM_CXXFLAGS = $(AM_CFLAGS)



