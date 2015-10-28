
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "utilities.h"
#include "texture.h"

#include <png.h>
#include <setjmp.h>






Texture* loadDataTexture(unsigned char* data, short width, short height) {

	
	Texture* dt;
	
	dt = (Texture*)malloc(sizeof(Texture));
	dt->width = width;
	dt->height = height;
	
	
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &dt->tex_id);
	glBindTexture(GL_TEXTURE_2D, dt->tex_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RED, // internalformat
		width,
		height,
		0,  // border
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // input type
		data);
	
	
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return dt;
}


BitmapRGBA8* readPNG(char* path) {
	
	FILE* f;
	png_byte sig[8];
	BitmapRGBA8* b;
	png_bytep* rowPtrs;
	int i;
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open \"%s\" (readPNG).\n", path);
		return NULL;
	}
	
	fread(sig, 1, 8, f);
	
	if(png_sig_cmp(sig, 0, 8)) {
		fprintf(stderr, "\"%s\" is not a valid PNG file.\n", path);
		fclose(f);
		return NULL;
	}
	
	png_structp readStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!readStruct) {
		fprintf(stderr, "Failed to load \"%s\". readPNG Error 1.\n", path);
		fclose(f);
		return NULL;
	}
	
	png_infop infoStruct = png_create_info_struct(readStruct);
	if (!infoStruct) {
		fprintf(stderr, "Failed to load \"%s\". readPNG Error 2.\n", path);
		png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);
		fclose(f);
		return NULL;
	};
	
	
	b = (BitmapRGBA8*)calloc(sizeof(BitmapRGBA8), 1);
	b->path = path;
	
	// exceptions are evil. the alternative with libPNG is a bit worse. ever heard of return codes libPNG devs?
	if (setjmp(png_jmpbuf(readStruct))) {
		
		fprintf(stderr, "Failed to load \"%s\". readPNG Error 3.\n", path);
		png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);
		
		if(b->data) free(b->data);
		free(b);
		fclose(f);
		return NULL;
	}
	
	
	png_init_io(readStruct, f);
	png_set_sig_bytes(readStruct, 8);

	png_read_info(readStruct, infoStruct);

	b->width = png_get_image_width(readStruct, infoStruct);
	b->height = png_get_image_height(readStruct, infoStruct);
	
	png_read_update_info(readStruct, infoStruct);
	
	b->data = (uint32_t*)malloc(b->width * b->height * sizeof(uint32_t));
	
	rowPtrs = (png_bytep*)malloc(sizeof(png_bytep) * b->height);
	
	for(i = 0; i < b->height; i++)
		rowPtrs[i] = (png_bytep*)(b->data + (b->width * i));

	png_read_image(readStruct, rowPtrs);

	
	free(rowPtrs);
	png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);

	fclose(f);
	
	printf("Loaded \"%s\".\n", path);
	
	return b;
}


Texture* loadBitmapTexture(char* path) {

	BitmapRGBA8* png; 
	
	
	
	png = readPNG(path);
	if(!png) {
		fprintf(stderr, "could not load texture %s \n", path);
		return NULL;
	}
	
	Texture* dt;
	
	dt = (Texture*)malloc(sizeof(Texture));
	dt->width = png->width;
	dt->height = png->height;
	
	
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &dt->tex_id);
	glBindTexture(GL_TEXTURE_2D, dt->tex_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA8, // internalformat
		png->width,
		png->height,
		0,  // border
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // input type
		png->data);
	
	
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return dt;
}

