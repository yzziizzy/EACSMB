

bin_PROGRAMS = eacsmb

eacsmb_SOURCES = main.c \
	window.c \
	shader.c 

eacsmb_LDADD =  \
	-lm \
	-lX11 -lfreetype \
	-lGL -lGLU -lGLEW 
	 
AM_CFLAGS = "-I/usr/include/freetype2" -g -DLINUX
AM_CXXFLAGS = $(AM_CFLAGS)



