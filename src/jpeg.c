#include <stdlib.h>
#include <stdio.h>
#include <string.h>

   

#include "jpeg.h"        



#ifndef HAVE_JPEG
int writeJPEG_RGB(char* path, int width, int height, void* data, int quality) {
	printf("No libJPEG support compiled in.\n");
}
int writeJPEG_RGBA(char* path, int width, int height, void* data, uint32_t alphaColor, int quality) {
	printf("No libJPEG support compiled in.\n");
}



BitmapRGBA8* readJPEG_RGBA(char* path, uint32_t alphaValue) {
	printf("No libJPEG support compiled in.\n");
}


#else



#include <jpeglib.h>


// quality is 1..100
int writeJPEG_RGB(char* path, int width, int height, void* data, int quality) {
	
	struct jpeg_compress_struct ci;
	struct jpeg_error_mgr err;
	FILE* f;
	JSAMPROW* rows; 
	
	f = fopen(path, "wb");
	if(f == NULL) {
		return 1;
	}
	
	
	ci.err = jpeg_std_error(&err);
	jpeg_create_compress(&ci);

	jpeg_stdio_dest(&ci, f);
	
	ci.image_width = width;
	ci.image_height = height;
	ci.input_components = 3;
	ci.in_color_space = JCS_RGB; 
	
	jpeg_set_defaults(&ci);
	
	
	
	jpeg_set_quality(&ci, (quality > 100 ? 100 : (quality < 1 ? 1 : quality)), 1);
	
	jpeg_start_compress(&ci, 1);
	
	rows = malloc(sizeof(*rows) * height);

	// set up the pointers and write the image
	for(int i = 0; i < height; i++) {
		rows[i] = data + (i * (width * 3));
	}

	jpeg_write_scanlines(&ci, rows, height);
	
	
	// finish up
	jpeg_finish_compress(&ci);
	
	fclose(f);
	free(rows);
	
	jpeg_destroy_compress(&ci);
	
	return 0;
}




// quality is 1..100
int writeJPEG_RGBA(char* path, int width, int height, void* data, uint32_t alphaColor, int quality) {
	
	struct jpeg_compress_struct ci;
	struct jpeg_error_mgr err;
	FILE* f;
	JSAMPROW row;
	float a_r, a_b, a_g;
	
	f = fopen(path, "wb");
	if(f == NULL) {
		return 1;
	}
	
	ci.err = jpeg_std_error(&err);
	jpeg_create_compress(&ci);

	jpeg_stdio_dest(&ci, f);
	
	ci.image_width = width;
	ci.image_height = height;
	ci.input_components = 3;
	ci.in_color_space = JCS_RGB; 
	
	jpeg_set_defaults(&ci);
	
	
	
	jpeg_set_quality(&ci, (quality > 100 ? 100 : (quality < 1 ? 1 : quality)), 1);
	
	jpeg_start_compress(&ci, 1);

	row = malloc(width * 3 * sizeof(*row));
	a_r = (alphaColor & 0xff000000) >> 24;
	a_g = (alphaColor & 0x00ff0000) >> 16;
	a_b = (alphaColor & 0x0000ff00) >> 8;
	
	JSAMPARRAY arr = &row;
	
	// convert and write one row at a time
	for(int i = 0; i < height; i++) {
		
		// blend in the alpha color
		for(int x = 0; x < width; x++) {
			union {
				uint8_t b[4];
				uint32_t n;
			} u;
			u.n = ((uint32_t*)data)[i * width + x];
			
			if(u.b[3] == 255) {
				row[(x * 3) + 0] = u.b[0];
				row[(x * 3) + 1] = u.b[1];
				row[(x * 3) + 2] = u.b[2];
			}
			else {
				float a = (float)u.b[3] / 256.0;
				float oma = 1.0 - a;
				
				row[(x * 3) + 0] = (float)u.b[0] * a + a_r * oma; 
				row[(x * 3) + 1] = (float)u.b[1] * a + a_g * oma; 
				row[(x * 3) + 2] = (float)u.b[2] * a + a_b * oma; 
			}
		}
		
		
		jpeg_write_scanlines(&ci, arr, 1);
	}

	
	
	// finish up
	jpeg_finish_compress(&ci);
	
	fclose(f);
	free(row);
	
	jpeg_destroy_compress(&ci);
	
	return 0;
}




TexBitmap* readJPEG_RGBA(char* path, uint32_t alphaValue) {

	struct jpeg_decompress_struct dci;
	struct jpeg_error_mgr err;
	FILE* f;
	float a_r, a_b, a_g;
	
	f = fopen(path, "rb");
	if(f == NULL) {
		return NULL;
	}
	
	
	dci.err = jpeg_std_error(&err);
	jpeg_create_decompress(&dci);

	jpeg_stdio_src(&dci, f);
	
	jpeg_read_header(&dci, 1);
	
	// output parameters must be set before calling start_decompress()
	
	// set to a sane colorspace
	dci.out_color_space = JCS_RGB;
	dci.out_color_components = 3;
	dci.output_components = 3;
	
	TexBitmap* bmp = calloc(1, sizeof(*bmp));
	bmp->width = dci.image_width;
	bmp->height = dci.image_height;
	bmp->depth = TEXDEPTH_8;
	bmp->channels = 4;
	bmp->path = strdup(path);
	
	unsigned char* ch = bmp->data8 = calloc(1, sizeof(*bmp->data8) * 4 * bmp->width * bmp->height);
	
	jpeg_start_decompress(&dci);
	
	JSAMPROW row = (JSAMPROW)malloc(sizeof(JSAMPLE) * bmp->width * 3);
	JSAMPROW* buf = &row;
	
	while (dci.output_scanline < dci.output_height) {
		int n = jpeg_read_scanlines(&dci, buf, 1);
		
		if(n) {
			// output_scanline is the /next/ one, which has been advanced at this point
			int i = (dci.output_scanline - 1) * bmp->width * 4;
			for(int x = 0; x < bmp->width; x++) {
				ch[i + x * 4 + 0] = row[x * 3 + 0];
				ch[i + x * 4 + 1] = row[x * 3 + 1];
				ch[i + x * 4 + 2] = row[x * 3 + 2];
				ch[i + x * 4 + 3] = 255;//alphaValue;
			}
		}
		
	}   

	jpeg_finish_decompress(&dci);
	jpeg_destroy_decompress(&dci);
	
	
	fclose(f);
	free(row);
	
	return bmp;
}


TexBitmap* readJPEG_R(char* path) {

	struct jpeg_decompress_struct dci;
	struct jpeg_error_mgr err;
	FILE* f;
	float a_r, a_b, a_g;
	
	f = fopen(path, "rb");
	if(f == NULL) {
		return NULL;
	}
	
	
	dci.err = jpeg_std_error(&err);
	jpeg_create_decompress(&dci);

	jpeg_stdio_src(&dci, f);
	
	jpeg_read_header(&dci, 1);
	
	// output parameters must be set before calling start_decompress()
	
	// set to a sane colorspace
	dci.out_color_space = JCS_GRAYSCALE;
	dci.out_color_components = 1;
	dci.output_components = 1;
	
	TexBitmap* bmp = calloc(1, sizeof(*bmp));
	bmp->width = dci.image_width;
	bmp->height = dci.image_height;
	bmp->depth = TEXDEPTH_8;
	bmp->channels = 1;
	bmp->path = strdup(path);
	
	unsigned char* ch = bmp->data8 = calloc(1, sizeof(*bmp->data8) * bmp->width * bmp->height);
	
	jpeg_start_decompress(&dci);
	
	JSAMPROW row = (JSAMPROW)malloc(sizeof(JSAMPLE) * bmp->width);
	JSAMPROW* buf = &row;
	
	while (dci.output_scanline < dci.output_height) {
		int n = jpeg_read_scanlines(&dci, buf, 1);
		
		if(n) {
			// output_scanline is the /next/ one, which has been advanced at this point
			int i = (dci.output_scanline - 1) * bmp->width;
			for(int x = 0; x < bmp->width; x++) {
				ch[i + x] = row[x];
			}
		}
		
	}   

	jpeg_finish_decompress(&dci);
	jpeg_destroy_decompress(&dci);
	
	
	fclose(f);
	free(row);
	
	return bmp;
}

#endif


