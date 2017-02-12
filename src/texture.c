
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
	int ret;
	
	BitmapRGBA8* b = (BitmapRGBA8*)calloc(sizeof(BitmapRGBA8), 1);
	if(!b) return NULL;
	
	ret = readPNG2(path, b);
	if(ret) {
		if(b->data) free(b->data);
		free(b);
		return NULL;
	}
	
	return b;
}

int readPNG2(char* path, BitmapRGBA8* bmp) {
	
	FILE* f;
	png_byte sig[8];
	png_bytep* rowPtrs;
	int i;
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open \"%s\" (readPNG).\n", path);
		return 1;
	}
	
	fread(sig, 1, 8, f);
	
	if(png_sig_cmp(sig, 0, 8)) {
		fprintf(stderr, "\"%s\" is not a valid PNG file.\n", path);
		fclose(f);
		return 1;
	}
	
	png_structp readStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!readStruct) {
		fprintf(stderr, "Failed to load \"%s\". readPNG Error 1.\n", path);
		fclose(f);
		return 1;
	}
	
	png_infop infoStruct = png_create_info_struct(readStruct);
	if (!infoStruct) {
		fprintf(stderr, "Failed to load \"%s\". readPNG Error 2.\n", path);
		png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);
		fclose(f);
		return 1;
	};
	
	
	bmp->path = path;
	
	// exceptions are evil. the alternative with libPNG is a bit worse. ever heard of return codes libPNG devs?
	if (setjmp(png_jmpbuf(readStruct))) {
		
		fprintf(stderr, "Failed to load \"%s\". readPNG Error 3.\n", path);
		png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);
		
		if(bmp->data) free(bmp->data);
		free(bmp);
		fclose(f);
		return 1;
	}
	
	
	png_init_io(readStruct, f);
	png_set_sig_bytes(readStruct, 8);

	png_read_info(readStruct, infoStruct);

	bmp->width = png_get_image_width(readStruct, infoStruct);
	bmp->height = png_get_image_height(readStruct, infoStruct);
	
	png_read_update_info(readStruct, infoStruct);
	
	bmp->data = (uint32_t*)malloc(bmp->width * bmp->height * sizeof(uint32_t));
	
	rowPtrs = (png_bytep*)malloc(sizeof(png_bytep) * bmp->height);
	
	for(i = 0; i < bmp->height; i++) {
		rowPtrs[i] = (png_bytep)(bmp->data + (bmp->width * i));
	}

	png_read_image(readStruct, rowPtrs);

	
	free(rowPtrs);
	png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);

	fclose(f);
	
	printf("Loaded \"%s\".\n", path);
	
	return 0;
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
	

	glGenTextures(1, &dt->tex_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dt->tex_id);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
	
	glGenerateMipmap(GL_TEXTURE_2D);
		
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glexit("failed to load texture");
	return dt;
}


// actually, the argument is a void**, but the compiler complains. stupid standards...
size_t ptrlen(const void* a) {
	size_t i = 0;
	void** b = (void**)a;
	
	while(*b++) i++;
	return i;
}



TexArray* loadTexArray(char** files) {
	
	TexArray* ta;
	int len, i;
	char* source, *s;
	BitmapRGBA8** bmps;
	int w, h;
	
	
	len = ptrlen(files);
	
	bmps = malloc(sizeof(BitmapRGBA8*) * len);
	ta = calloc(sizeof(TexArray), 1);
	
	w = 0;
	h = 0;
	
	for(i = 0; i < len; i++) {
		bmps[i] = readPNG(files[i]);
		
		if(!bmps[1]) {
			printf("Failed to load %s\n", files[i]);
			continue;
		}
		
		w = MAX(w, bmps[i]->width);
		h = MAX(h, bmps[i]->height);
	}
	
	ta->width = w;
	ta->height = h;
	ta->depth = len;
	
	printf("len: %d\n", len);
	
	
	glGenTextures(1, &ta->tex_id);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ta->tex_id);
	glexit("failed to create texture array 1");
	
// 		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_FALSE);
	glexit("failed to create texture array 2");

	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glexit("failed to create texture array 3");
	
	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
		1,  // mips, flat
		GL_RGBA8,
		w, h,
		len); // layers
	
	glexit("failed to create texture array 4");
	
	
	for(i = 0; i < len; i++) {
		if(!bmps[i]) continue;
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
			0,  // mip level, 0 = base, no mipmap,
			0, 0, i,// offset
			w, h,
			1,
			GL_RGBA,  // format
			GL_UNSIGNED_BYTE, // input type
			bmps[i]->data);
		glexit("could not load tex array slice");
		
		free(bmps[i]->data);
		free(bmps[i]);
	}
	
	free(bmps);
	
	return ta;
}

