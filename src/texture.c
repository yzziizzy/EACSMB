
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <x86intrin.h>

#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "hash.h"
#include "utilities.h"
#include "texture.h"
#include "jpeg.h"
#include "dumpImage.h"

#include <png.h>
#include <setjmp.h>


// something more sophisticated will be needed for tex arrays and view sharing
// a start is a start


static HashTable(TexEntry*) texLookup;

static TextureManager* texman;



Texture* Texture_acquirePath(char* path) {
	struct TexEntry* e;
	Texture* t;
	
	if(strlen(path) == 0) {
		return NULL;
	}
	
	// check the cache
	if(HT_get(&texLookup, path, &e) && e->tex) {
		e->refs++;
		return e->tex;
	}
	
	
	t = loadBitmapTexture(path);
	if(!t) return NULL;
	
	e = malloc(sizeof(*e));
	e->refs = 1;
	e->path = path; // todo: figure out memory management
	e->tex = t;
	
	HT_set(&texLookup, path, e);
	
	return t;
}


void Texture_release(Texture* tex) {
	struct TexEntry* e;
	
	if(strlen(tex->name) == 0) {
		return NULL;
	}
	
		// check the cache
	if(HT_get(&texLookup, tex->name, &e)) {
		if(e->refs > 0) {
			e->refs--;
		}
		else {
			fprintf(stderr, "Texture refs below zero: %s\n", tex->name);
		}
	}
}


// global init fn
void initTextures() {
	HT_init(&texLookup, 5);
}


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
	png_byte colorType;
	png_byte bitDepth;
	
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
	
	
	// coerce the fileinto 8-bit RGBA        
	
	bitDepth = png_get_bit_depth(readStruct, infoStruct);
	colorType = png_get_color_type(readStruct, infoStruct);
	
	if(colorType == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(readStruct);
	}
	if(colorType == PNG_COLOR_TYPE_GRAY) {
		png_set_expand_gray_1_2_4_to_8(readStruct);
		png_set_expand(readStruct);
	}
	
	if(bitDepth == 16) {
		png_set_strip_16(readStruct);
	}
	if(bitDepth < 8) {
		png_set_packing(readStruct);
	}
	if(png_get_valid(readStruct, infoStruct, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(readStruct);
	}
	if(colorType == PNG_COLOR_TYPE_GRAY) {
		png_set_gray_to_rgb(readStruct);
		png_set_expand(readStruct);
	}		
	if(colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(readStruct);
	}

// 	if(colorType > PNG_COLOR_TYPE_PALETTE) {
//  	png_color_16 background = {.red= 255, .green = 255, .blue = 255};
//  	png_set_background(readStruct, &background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
// 	}
//  	png_set_filler(readStruct, 0x00, PNG_FILLER_AFTER);
	
//	printf("interlace: %d\n", png_set_interlace_handling(readStruct));
	
	png_read_update_info(readStruct, infoStruct);
	
	
	// read the data
	bmp->data = (uint32_t*)malloc(bmp->width * bmp->height * sizeof(uint32_t));
	
	rowPtrs = (png_bytep*)malloc(sizeof(png_bytep) * bmp->height);
	
	for(i = 0; i < bmp->height; i++) {
		rowPtrs[i] = (png_bytep)(bmp->data + (bmp->width * i));
	}

	png_read_image(readStruct, rowPtrs);

	
	free(rowPtrs);
	png_destroy_read_struct(&readStruct, (png_infopp)0, (png_infopp)0);

	fclose(f);
	
//	printf("Loaded \"%s\".\n", path);
	
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
	dt->name = path;

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




static uint32_t average(BitmapRGBA8* bmp, Vector2 min, Vector2 max) {
	int w = bmp->width;
	int xmin, xmax, ymin, ymax;
	int x, y;
	
	union {
		uint32_t u;
		uint8_t b[4];
	} u;
	
	xmin = MAX(0, floor(min.x));
	xmax = MIN(bmp->width, ceil(max.x));
	ymin = MAX(0, floor(min.y));
	ymax = MIN(bmp->height, ceil(max.y));
		
	float r = 0.0;
	float g = 0.0;
	float b = 0.0;
	float a = 0.0; 
	
	for(y = ymin; y < ymax; y++) {  
		for(x = xmin; x < xmax; x++) {  
			u.u = bmp->data[(int)(ceil(x) + (w * ceil(y)))];	
			
			r += (float)u.b[0]; // these may not correspond properly to the component
			g += (float)u.b[1]; //   but it doesn't matter as long as everything is
			b += (float)u.b[2]; //   consistent across the function
			a += (float)u.b[3];
		}
	}
	
	float totalWeight = (xmax - xmin) * (ymax - ymin);

	r /= totalWeight;
	g /= totalWeight;
	b /= totalWeight;
	a /= totalWeight;
	
	u.b[0] = iclamp(r, 0, 255);
	u.b[1] = iclamp(g, 0, 255);
	u.b[2] = iclamp(b, 0, 255);
	u.b[3] = iclamp(a, 0, 255);
	
	return u.u;
}



static BitmapRGBA8* resample(BitmapRGBA8* in, Vector2i outSz) {
	int ox, oy;
	float scaleFactor = in->width / outSz.x;
	BitmapRGBA8* out;
	
	out = calloc(1, sizeof(out));
	out->width = outSz.x;
	out->height = outSz.y;
	out->data = malloc(out->width * out->height * sizeof(*out->data));
	
	// really shitty algorithm
	for(oy = 0; oy < outSz.y; oy++) {
		for(ox = 0; ox < outSz.x; ox++) {
			
			out->data[ox + (oy * outSz.x)] = average(
				in, 
				(Vector2){ox * scaleFactor, oy * scaleFactor},
				(Vector2){ox * (scaleFactor + 1), oy * (scaleFactor + 1)}
			);
		}
	}
	
	return out;
}




// nearest resizing

static uint32_t bmpsample(BitmapRGBA8* b, Vector2i p) {
	return b->data[iclamp(p.x, 0, b->width) + (iclamp(p.y, 0, b->height) * b->width)];
}

static uint32_t bmpsamplef(BitmapRGBA8* b, Vector2 p) {
	Vector2i pi = {floor(p.x + .5), floor(p.y + .5)};
	return bmpsample(b, pi);
}

static uint32_t bmpsamplescale(BitmapRGBA8* b, Vector2i p, float scale) {
	Vector2i pi = {floor((p.x * scale) + .5), floor((p.y * scale) + .5)};
	return bmpsample(b, pi);
}


static BitmapRGBA8* nearestRescale(BitmapRGBA8* in, Vector2i outSz) {
	int ox, oy;
	float scaleFactor = (float)outSz.x / (float)in->width;
	BitmapRGBA8* out;
	
	out = calloc(1, sizeof(out));
	out->width = outSz.x;
	out->height = outSz.y;
	out->data = malloc(out->width * out->height * sizeof(*out->data));
	
	for(oy = 0; oy < outSz.y; oy++) {
		for(ox = 0; ox < outSz.x; ox++) {
			out->data[ox + (oy * outSz.x)] = bmpsamplescale(in, (Vector2i){ox, oy}, 1.0f / scaleFactor);
		}
	}
	
	return out;
}



// linear rescaling

static void unpack8(uint32_t v, uint32_t* r, uint32_t* g, uint32_t* b, uint32_t* a) {
	union {
		uint32_t n;
		uint8_t b[4];
	} u;
	
	u.n = v;
	*r = u.b[0];
	*g = u.b[1];
	*b = u.b[2];
	*a = u.b[3];
}
static void unpackadd8(uint32_t v, uint32_t* o) {
	union {
		uint32_t n;
		uint8_t b[4];
	} u;
	
	u.n = v;
	o[0] += u.b[0];
	o[1] += u.b[1];
	o[2] += u.b[2];
	o[3] += u.b[3];
}

static uint32_t repackdiv8(uint32_t* o, float divisor) {
	union {
		uint32_t n;
		uint8_t b[4];
	} u;
	
	u.b[0] = (float)o[0] / divisor;
	u.b[1] = (float)o[1] / divisor;
	u.b[2] = (float)o[2] / divisor;
	u.b[3] = (float)o[3] / divisor;
	
	return u.n;
}



static uint32_t bmplinsamplescale(BitmapRGBA8* b, Vector2i p, int scale) {
	int x, y;
	float invscale = (float)scale;
	Vector2i p0 = {p.x * invscale, p.y * invscale};
	
	uint32_t acc[4] = {0,0,0,0};
	
	for(y = 0; y < scale; y++) {
		for(x = 0; x < scale; x++) {
			unpackadd8(bmpsample(b, (Vector2i){p0.x + x, p0.y + y}), acc);
		}
	}
	
	return repackdiv8(acc, scale * scale);
}


static BitmapRGBA8* linearDownscale_2(BitmapRGBA8* in); 
static BitmapRGBA8* linearDownscale_4(BitmapRGBA8* in); 
static BitmapRGBA8* linearDownscale_8(BitmapRGBA8* in); 

static BitmapRGBA8* linearDownscale(BitmapRGBA8* in, Vector2i outSz) {
	int ox, oy;
	int scaleFactor = (float)in->width / (float)outSz.x;
	BitmapRGBA8* out;
	
	// check for certain optimized power-of-two versions
	switch(scaleFactor) {
		case 2: return linearDownscale_2(in);
		case 4: return linearDownscale_4(in);
		case 8: return linearDownscale_8(in);
	}
	
	
	out = calloc(1, sizeof(out));
	out->width = outSz.x;
	out->height = outSz.y;
	out->data = malloc(out->width * out->height * sizeof(*out->data));
	
	for(oy = 0; oy < outSz.y; oy++) {
		for(ox = 0; ox < outSz.x; ox++) {
			out->data[ox + (oy * outSz.x)] = bmplinsamplescale(in, (Vector2i){ox, oy}, scaleFactor);
		}
	}
	
	return out;
}

// scale by 1/2
static BitmapRGBA8* linearDownscale_2(BitmapRGBA8* in) {

	int ix, iy;
	int ox, oy;
	BitmapRGBA8* out;
	
	out = calloc(1, sizeof(out));
	out->width = in->width / 2;
	out->height = in->height / 2;
	out->data = malloc(out->width * out->height * sizeof(*out->data));
	
#if defined(EACSMB_USE_SIMD) && defined(EACSMB_HAVE_AVX2)
	// two columns of two pixels at a time, side by side
	// requires in->width to be a multiple of 2
	for(oy = iy = 0; iy < in->height; iy += 2, oy++) {
		for(ox = ix = 0; ix < in->width; ix += 2, ox++) {
			// load and add two rows of two pixels
			__m256i top_i = _mm256_cvtepu8_epi32(*(__m128i*)(in->data + ix + (iy * in->width)));
			__m256i bot_i = _mm256_cvtepu8_epi32(*(__m128i*)(in->data + ix + ((iy + 1) * in->width)));
			__m256i row_i = _mm256_add_epi32(top_i, bot_i);
			
			// horizontal add of the adjacent summed pixels
			__m256i row2_i = _mm256_permute2x128_si256(row_i, row_i, _MM_SHUFFLE(0,1,0,1));
			        row_i = _mm256_add_epi32(row2_i, row_i);
			__m128i pix_i = _mm256_extracti128_si256(row_i, 0);
			// pix_i's low 64 bit have the summed pixel
			
			// divide by 4
			pix_i = _mm_srai_epi32(pix_i, 2);
			
			// chop down to char and store
			pix_i = _mm_packus_epi32(pix_i, pix_i);
			__m64 p2 = _mm_movepi64_pi64(pix_i);
			      p2 = _m_packuswb(p2, p2);
			
			out->data[ox + oy * out->width] = _mm_cvtsi64_si32(p2);
		}
	}
#else
	for(oy = iy = 0; iy < in->height; iy += 2, oy++) {
		for(ox = ix = 0; ix < in->width; ix += 2, ox++) {
			out->data[ox + (oy * out->width)] = bmplinsamplescale(in, (Vector2i){ox, oy}, 2);
		}
	}
#endif
	
	return out;
}


// scale by 1/4 
static BitmapRGBA8* linearDownscale_4(BitmapRGBA8* in) {

	int ix, iy;
	int ox, oy;
	BitmapRGBA8* out;
	
	out = calloc(1, sizeof(out));
	out->width = in->width / 4;
	out->height = in->height / 4;
	out->data = malloc(out->width * out->height * sizeof(*out->data));
	
#if defined(EACSMB_USE_SIMD) && defined(EACSMB_HAVE_AVX2)
	// four columns of 8 rows of pixels
	// requires in->width to be a multiple of 2
	for(oy = iy = 0; iy < in->height; iy += 4, oy++) {
		for(ox = ix = 0; ix < in->width; ix += 4, ox++) {
			// load and add 4 rows of 4 pixels
			__m256i r1_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + (iy * in->width)));
			__m256i r2_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 1) * in->width)));
			__m256i r3_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 2) * in->width)));
			__m256i r4_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 3) * in->width)));
			__m256i row_i = _mm256_add_epi16(r1_i, r2_i);
			        row_i = _mm256_add_epi16(row_i, r3_i);
			        row_i = _mm256_add_epi16(row_i, r4_i);
			// row_i is now a vertical sum of 4 columns of pixels
			
			// horizontal add of the adjacent four summed pixels
			__m128i pix2_hi_i = _mm256_extracti128_si256(row_i, 1);
			__m128i pix2_lo_i = _mm256_extracti128_si256(row_i, 0);
			__m128i pix_2_i = _mm_add_epi16(pix2_hi_i, pix2_lo_i);
			// pix_2_i is one hadd, being two pixels remaining
			__m128i pix_2_i_2 = _mm_shuffle_epi32(pix_2_i, _MM_SHUFFLE(3,4,0,0));
			__m128i pix_i = _mm_add_epi16(pix_2_i, pix_2_i_2);
			// pix_i's low 64 bit have the summed pixel
			
			// divide by 16
			pix_i = _mm_srai_epi16(pix_i, 4);
			
			// chop down to char and store
			__m64 p2 = _mm_movepi64_pi64(pix_i);
			      p2 = _m_packuswb(p2, p2);
			out->data[ox + oy * out->width] = _mm_cvtsi64_si32(p2);
		}
	}
#else
	for(oy = iy = 0; iy < in->height; iy += 4, oy++) {
		for(ox = ix = 0; ix < in->width; ix += 4, ox++) {
			out->data[ox + (oy * out->width)] = bmplinsamplescale(in, (Vector2i){ox, oy}, 4);
		}
	}
#endif
	
	return out;
}



// scale by 1/8
static BitmapRGBA8* linearDownscale_8(BitmapRGBA8* in) {

	int ix, iy;
	int ox, oy;
	BitmapRGBA8* out;
	
	out = calloc(1, sizeof(out));
	out->width = in->width / 8;
	out->height = in->height / 8;
	size_t s = out->width * out->height * sizeof(*out->data);
	void* p = malloc(s);
	out->data = p;
	
#if defined(EACSMB_USE_SIMD) && defined(EACSMB_HAVE_AVX2)
	// two columns of four columns of 8 rows of pixels
	// requires in->width to be a multiple of 8
	for(oy = iy = 0; iy < in->height; iy += 8, oy++) {
		for(ox = ix = 0; ix < in->width; ix += 8, ox++) {
			// load and add 8 rows of 4 pixels
			__m256i r1_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 0) * in->width)));
			__m256i r2_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 1) * in->width)));
			__m256i r3_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 2) * in->width)));
			__m256i r4_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 3) * in->width)));
			__m256i r5_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 4) * in->width)));
			__m256i r6_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 5) * in->width)));
			__m256i r7_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 6) * in->width)));
			__m256i r8_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + ((iy + 7) * in->width)));
			__m256i rowa_i = _mm256_add_epi16(r1_i, r2_i);
			__m256i rowb_i = _mm256_add_epi16(r3_i, r4_i);
			__m256i rowc_i = _mm256_add_epi16(r5_i, r6_i);
			__m256i rowd_i = _mm256_add_epi16(r7_i, r8_i);
			        rowa_i = _mm256_add_epi16(rowa_i, rowb_i);
			        rowc_i = _mm256_add_epi16(rowc_i, rowd_i);
			__m256i row_i = _mm256_add_epi16(rowa_i, rowc_i);
			// rowa_i is now a vertical sum of 4 columns of pixels
			
			// load and add 8 rows of the next 4 horizontal pixels 
			r1_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 0) * in->width)));
			r2_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 1) * in->width)));
			r3_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 2) * in->width)));
			r4_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 3) * in->width)));
			r5_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 4) * in->width)));
			r6_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 5) * in->width)));
			r7_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 6) * in->width)));
			r8_i = _mm256_cvtepu8_epi16(*(__m128i*)(in->data + ix + 4 + ((iy + 7) * in->width)));
			rowa_i = _mm256_add_epi16(r1_i, r2_i);
			rowb_i = _mm256_add_epi16(r3_i, r4_i);
			rowc_i = _mm256_add_epi16(r5_i, r6_i);
			rowd_i = _mm256_add_epi16(r7_i, r8_i);
			rowa_i = _mm256_add_epi16(rowa_i, rowb_i);
			rowc_i = _mm256_add_epi16(rowc_i, rowd_i);
			row_i = _mm256_add_epi16(row_i, rowa_i);
			row_i = _mm256_add_epi16(row_i, rowc_i);
			// row_i is now now the sum of two sums of 4 columns
			
			// horizontal add of the adjacent four summed pixels
			__m128i pix2_hi_i = _mm256_extracti128_si256(row_i, 1);
			__m128i pix2_lo_i = _mm256_extracti128_si256(row_i, 0);
			__m128i pix_2_i = _mm_add_epi16(pix2_hi_i, pix2_lo_i);
			// pix_2_i is one hadd, being two pixels remaining
			__m128i pix_2_i_2 = _mm_shuffle_epi32(pix_2_i, _MM_SHUFFLE(3,4,0,0));
			__m128i pix_i = _mm_add_epi16(pix_2_i, pix_2_i_2);
			// pix_i's low 64 bit have the summed pixel
			
			// divide by 64
			pix_i = _mm_srai_epi16(pix_i, 6);
			
			// chop down to char and store
			__m64 p2 = _mm_movepi64_pi64(pix_i);
			p2 = _m_packuswb(p2, p2);
			out->data[ox + oy * out->width] = _mm_cvtsi64_si32(p2);
		}
	}
#else
	for(oy = iy = 0; iy < in->height; iy += 8, oy++) {
		for(ox = ix = 0; ix < in->width; ix += 8, ox++) {
			out->data[ox + (oy * out->width)] = bmplinsamplescale(in, (Vector2i){ox, oy}, 8);
		}
	}
#endif
	
	return out;
}

// scale up by 2
static BitmapRGBA8* linearUpscale_2(BitmapRGBA8* in) {

	int ix, iy;
	int ox, oy;
	BitmapRGBA8* out;
	
	out = calloc(1, sizeof(out));
	out->width = in->width * 2;
	out->height = in->height * 2;
	out->data = malloc(out->width * out->height * sizeof(*out->data));
	
#if defined(EACSMB_USE_SIMD) && defined(EACSMB_HAVE_AVX2)
	
	// contributions from each pixel in the middle blocks
	__m256i co_tl = _mm256_set_epi16(
		9, 9, 9, 9, // tl
		3, 3, 3, 3, // tr
		3, 3, 3, 3, // bl
		1, 1, 1, 1 // br
	);
	__m256i co_tr = _mm256_set_epi16(
		3, 3, 3, 3,
		9, 9, 9, 9,
		1, 1, 1, 1,
		3, 3, 3, 3
	);
	__m256i co_bl = _mm256_set_epi16(
		3, 3, 3, 3,
		1, 1, 1, 1,
		9, 9, 9, 9,
		3, 3, 3, 3
	);
	__m256i co_br = _mm256_set_epi16(
		1, 1, 1, 1,
		3, 3, 3, 3,
		3, 3, 3, 3,
		9, 9, 9, 9
	);
	
	// contributions for each pixel in the end columns
	__m128i co_top = _mm_set_epi16(3,3,3,3, 1,1,1,1);
	__m128i co_bot = _mm_set_epi16(1,1,1,1, 3,3,3,3);
	
	
	// first to the top and bottom rows, as they are special
	
	// the corner pixels are just copied
	out->data[0] = in->data[0];
	out->data[out->width] = in->data[in->width];
	out->data[out->width * (out->height - 1)] = in->data[in->width * (in->height - 1)];
	out->data[out->width * (out->height)] = in->data[in->width * (in->height)];
	
	for(ox = 1, ix = 0; ix < in->width; ix++, ox += 2) {
		// TODO
	}
	
	
	// now the middle rows
	for(oy = 1, iy = 0; iy <= in->height; iy++, oy += 2) {
		
		// the first two pixels are special
		__m128i out_mm = _mm_setzero_si128();
		
		__m128i t = _mm_set1_epi32(in->data[(iy + 0) * in->width]);
		__m128i b = _mm_set1_epi32(in->data[(iy + 1) * in->width]);
		        t = _mm_cvtepu8_epi16(t);
		        b = _mm_cvtepu8_epi16(b);
		
		out_mm = _mm_add_epi16(out_mm, _mm_mullo_epi16(t, co_top));
		out_mm = _mm_add_epi16(out_mm, _mm_mullo_epi16(b, co_bot));
		
		out_mm = _mm_packus_epi16(out_mm, out_mm);
		
		union {
			uint32_t dw[2];
			uint64_t qw;
		} u;
		u.qw = _mm_cvtsi128_si64(out_mm);
		out->data[(oy + 0) * out->width] = u.dw[0];
		out->data[(oy + 1) * out->width] = u.dw[1];
		
		// now the middle pixels
		for(ox = 1, ix = 0; ix < in->width; ix++, ox += 2) {
			
			// out->data[tl] = (9tl + 3tr + 3bl + br ) / 16; 
			
			__m256i out_pix = _mm256_setzero_si256();
			
			// grab and convert the top and bottom rows  
			__m128i top = _mm_cvtepu8_epi16(*(__m128i*)(in->data + ix + 0 + ((iy + 0) * in->width)));
			__m128i bot = _mm_cvtepu8_epi16(*(__m128i*)(in->data + ix + 0 + ((iy + 1) * in->width)));
			
			__m256i tr_v = _mm256_broadcastq_epi64(top);
					top = _mm_unpackhi_epi64(top, top);
			__m256i tl_v = _mm256_broadcastq_epi64(top);
			__m256i br_v = _mm256_broadcastq_epi64(bot);
					bot = _mm_unpackhi_epi64(bot, bot);
			__m256i bl_v = _mm256_broadcastq_epi64(bot);
			
			// add up contributions
			out_pix = _mm256_add_epi16(out_pix, _mm256_mullo_epi16(tl_v, co_tl));
			out_pix = _mm256_add_epi16(out_pix, _mm256_mullo_epi16(tr_v, co_tr));
			out_pix = _mm256_add_epi16(out_pix, _mm256_mullo_epi16(bl_v, co_bl));
			out_pix = _mm256_add_epi16(out_pix, _mm256_mullo_epi16(br_v, co_br));
			
			// divide by 16
			out_pix = _mm256_srai_epi16(out_pix, 6); 
			
			
			// convert and save
			__m128i packed = _mm_packus_epi16(
				_mm256_extracti128_si256(out_pix, 1), // top 
				_mm256_extracti128_si256(out_pix, 0) // bottom
			);
			
			// two packed pixels are stored at once
			_mm_storel_epi64(&out->data[ox + 0 + (oy + 1) * out->width], packed);
			packed = _mm_unpackhi_epi64(packed, packed);
			_mm_storel_epi64(&out->data[ox + 0 + (oy + 0) * out->width], packed);
		}
		
		// do the last two vertical pixels
		// they are also speciall
		out_mm = _mm_setzero_si128();
		
		t = _mm_set1_epi32(in->data[ix + (iy + 0) * in->width]);
		b = _mm_set1_epi32(in->data[ix + (iy + 1) * in->width]);
		t = _mm_cvtepu8_epi16(t);
		b = _mm_cvtepu8_epi16(b);
		
		out_mm = _mm_add_epi16(out_mm, _mm_mullo_epi16(t, co_top));
		out_mm = _mm_add_epi16(out_mm, _mm_mullo_epi16(b, co_bot));
		
		out_mm = _mm_packus_epi16(out_mm, out_mm);
		
		u.qw = _mm_cvtsi128_si64(out_mm);
		out->data[ox + (oy + 0) * out->width] = u.dw[0];
		out->data[ox + (oy + 1) * out->width] = u.dw[1];
		
	}
	
#else

#endif
	
	return out;
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



static int depth_bytes[] = {
	[TEXDEPTH_8] = 1,
	[TEXDEPTH_16] = 2,
	[TEXDEPTH_32] = 4,
	[TEXDEPTH_FLOAT] = 4,
	[TEXDEPTH_DOUBLE] = 8,
};

TexBitmap* TexBitmap_create(int w, int h, enum TextureDepth d, int channels) {
	int dbytes;
	TexBitmap* bmp;
	
	if(d >= TEXDEPTH_MAXVALUE) return NULL; 
	dbytes = depth_bytes[d];
	
	bmp = calloc(1, sizeof(*bmp));	
	
	bmp->data8 = malloc(w * h * channels * dbytes);
	bmp->width = w;
	bmp->height = h;
	bmp->channels = channels;
	
	return bmp;
}




TextureManager* TextureManager_alloc(int channels) {
	TextureManager* tm;
	
	tm = calloc(1, sizeof(*tm));
	CHECK_OOM(tm);
	
	TextureManager_init(tm, channels);
	
	return tm;
}


void TextureManager_init(TextureManager* tm, int channels) {
	HT_init(&tm->texLookup, 4);
	VEC_INIT(&tm->texEntries);
	tm->mipLevels = 1; // no mipmap by default
	tm->channels = channels;
}


	
int TextureManager_reservePath(TextureManager* tm, char* path) {
	
	int ret;
	int64_t index;
	char* pathc;
	
	if(!HT_getInt(&tm->texLookup, path, &index)) {
		// path already reserved, increase refs
		TexEntry* tep = &VEC_ITEM(&tm->texEntries, index);
		
		tep->refs++;
		return index;
	}
	
	// new texture
	
	pathc = strdup(path);
	
	TexEntry te = {
		.tex = NULL,
		.refs = 1,
		.path = pathc,
	};
	
	index = VEC_LEN(&tm->texEntries);
	VEC_PUSH(&tm->texEntries, te);
	
	HT_setInt(&tm->texLookup, pathc, index);
	
	return index;
}



// read texture files and push into gpu
int TextureManager_loadAll(TextureManager* tm, Vector2i targetRes) {
	int i, depth;
	
	if(targetRes.x * targetRes.y <= 0) {
		fprintf(stderr, "TextureManager: Target res has zero area\n");
		return 1;
	}
	
	tm->targetRes = targetRes;
	depth = VEC_LEN(&tm->texEntries);
	
	if(depth <= 0) {
		fprintf(stderr, "TextureManager: depth is zero (no textures reserved)\n");
		return 2;
	}
	
	
	glGenTextures(1, &tm->tex_id);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tm->tex_id);
	glexit("failed to create texture array 1");
	printf("texman array %d\n", tm->tex_id); 
// 		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_FALSE);
	glexit("failed to create texture array 2");

	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glexit("failed to create texture array 3");
	
	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	GLenum type, format;
	switch(tm->channels) {
		case 1: type = GL_R8; format = GL_RED; break;  
		case 2: type = GL_RG8; format = GL_RG; break;  
		case 3: type = GL_RGB8; format = GL_RGB; break;
		default:
		case 4: type = GL_RGBA8; format = GL_RGBA; break;
	}
	
	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
		tm->mipLevels,  // mips, flat
		type,
		targetRes.x, targetRes.y,
		depth); // layers
	
	glexit("failed to create texture array 4");
	
	
	for(i = 0; i < VEC_LEN(&tm->texEntries); i++) {
		TexEntry* te = &VEC_ITEM(&tm->texEntries, i);
		BitmapRGBA8* bmp;
		
		if(te->path[0] == '$') {
			// texgen
			bmp = TexGen_Generate(te->path + 1, targetRes);
			
		}
		else {
			// load from file
			
			// check extension
			char* ext = pathExt(te->path);
			
			if(0 == strcasecmp(ext, "png")) {
				bmp = readPNG(te->path);
			}
			else if (0 == strcasecmp(ext, "jpg") || 0 == strcasecmp(ext, "jpeg")) {
				bmp = readJPEG_RGBA(te->path, 0);
			}
			else {
				fprintf(stderr, "Unknown texture format: '%s'\n", ext);
			}
				
			if(!bmp) {
				printf("TextureManager: Failed to load %s\n", te->path);
				continue;
			}
			
			if(bmp->width != targetRes.x || bmp->height != targetRes.y) {
				printf("resizing %s to %d,%d\n", te->path, targetRes.x, targetRes.y);
				//BitmapRGBA8* tmp = resample(bmp, targetRes);
				BitmapRGBA8* tmp;
				if(bmp->width > targetRes.x) {
					tmp = linearDownscale(bmp, targetRes);
				}
				else {
					tmp = nearestRescale(bmp, targetRes);
				}
				free(bmp->data);
				free(bmp);
				
				bmp = tmp;
			}
		}
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
			0,  // mip level, 0 = base, no mipmap,
			0, 0, i,// offset
			targetRes.x, targetRes.y,
			1,
			GL_RGBA,  // format
			GL_UNSIGNED_BYTE, // input type
			bmp->data);
		glexit("could not load tex array slice");
		
		free(bmp->data);
		free(bmp);
		
	}
	
	return 0;
}




