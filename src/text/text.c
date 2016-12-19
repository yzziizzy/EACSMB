
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>



#include "../c3dlas/c3dlas.h"
#include "text.h"
#include "fcfg.h"
#include "../EACSMB/src/utilities.h"
#include "../dumpImage.h"



FT_Library ftLib = NULL;

static void makeVertices(TextRenderInfo* tri, unsigned int* colors);


/*
A Note About Charsets:

All characters specified will be packed into a texture then shoved into video ram.
There is a limit to the size of textures. A large font size and large character
set may not fit in the largest texture available. Even if it does, precious vram
is used for every character added. This is not the place to be a unicode twat;
specify only the characters you will actually use. And for God's sake, don't try
to render traditional Chinese at 64pt... you'd use 80+mb of ram.

Also note the kerning information is n^2 in size with respect to strlen(charset).
Currently this information is in system memory but eventually most operations
will be moved to the gpu. Geometry shaders are amazing.

*/

// all keys on a standard US keyboard.
char* defaultCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 `~!@#$%^&*()_+|-=\\{}[]:;<>?,./'\"";

// 16.16 fixed point to float conversion
static float f2f(int i) {
	return (float)(i >> 6);
}


static void blit(
	int src_x, int src_y, int dst_x, int dst_y, int w, int h,
	int src_w, int dst_w, unsigned char* src, unsigned char* dst) {
	
	
	int y, x, s, d;
	
	// this may be upside down...
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			s = ((y + src_y) * src_w) + src_x + x;
			d = ((y + dst_y) * dst_w) + dst_x + x;
			
			dst[d] = src[s];
		}
	}
}




FontInfo* LoadFontInfo(char* fontName) {
	FT_Error err;
	FontInfo* fi;
	char* fontPath;
	
	if(!ftLib) {
		err = FT_Init_FreeType(&ftLib);
		if(err) {
			fprintf(stderr, "Could not initialize FreeType library.\n");
			return NULL;
		}
	}
	
	fontPath = getFontFile(fontName);
	if(!fontPath) {
		fprintf(stderr, "Could not load font '%s'\n", fontName);
		return NULL;
	}
	
	fi = calloc(1, sizeof(FontInfo));
	// if you're out of memory you have bigger problems than error checking...
	
	err = FT_New_Face(ftLib, fontPath, 0, &fi->fontFace);
	if(err) {
		fprintf(stderr, "Could not load font file \"%s\".\n", fontPath);
		free(fi);
		return NULL;
	}
	
	return fi;
}
	
	
	
TextRes* LoadFont(char* fontName, int size, char* chars) {

	FT_Error err;
	FT_GlyphSlot slot;
	FT_Face fontFace; 
	TextRes* res;
	int i, j, charlen, width, h_above, h_below, height, padding, xoffset;
	char* fontPath;
	
	padding = 2;
	
	res = calloc(1, sizeof(TextRes));
	
	res->fontInfo = LoadFontInfo(fontName);
	
	fontFace = res->fontInfo->fontFace;
	
	err = FT_Set_Pixel_Sizes(fontFace, 0, size);
	if(err) {
		fprintf(stderr, "Could not set pixel size to %dpx.\n", size);
		free(res->fontInfo);
		free(res);
		return NULL;
	}
	
	// slot is a pointer
	slot = fontFace->glyph;
	
	if(!chars) chars = defaultCharset;
	res->charSet = strdup(chars);
	
	charlen = strlen(chars);
	res->charLen = charlen;
	
	h_above = 0;
	h_below = 0;
	width = 0;
	height = 0;
	
	// first find how wide of a texture we need
	for(i = 0; i < charlen; i++) {
		int ymin;
		err = FT_Load_Char(fontFace, chars[i], FT_LOAD_DEFAULT);
		
		ymin = slot->metrics.height >> 6;// - f2f(slot->metrics.horiBearingY);
/*
		printf("%c-----\nymin: %d \n", chars[i], ymin);
		printf("bearingY: %d \n", slot->metrics.horiBearingY >> 6);
		printf("width: %d \n", slot->metrics.width >> 6);
		printf("height: %d \n\n", slot->metrics.height >> 6);
*/
		
		width += f2f(slot->metrics.width);
		h_above = MAX(h_above, slot->metrics.horiBearingY >> 6);
		h_below = MAX(h_below, ymin);
	}
	
	
	width += charlen * padding;
	height = h_below + padding + padding;
	
	res->maxHeight = height;
	res->padding = padding / 2;
	
	//TODO: clean up this messy function
//	printf("width: %d, height: %d \n", width, height);

	width = nextPOT(width);
	height = nextPOT(height);
	res->texWidth = width;
	res->texHeight = height; // may not always just be one row
	
	
	res->texture = (unsigned char*)calloc(width * height, 1);
	res->offsets = (unsigned short*)calloc(charlen * sizeof(unsigned short), 1);
	res->charWidths = (unsigned short*)calloc(charlen * sizeof(unsigned short), 1);
	res->valign = (unsigned char*)calloc(charlen * sizeof(unsigned char), 1);
	

	// construct code mapping
	// shitty method for now, improve later
	res->indexLen = 128; // 7 bits for now.
	res->codeIndex = (unsigned char*)calloc(res->indexLen, 1);
	
	// render the glyphs into the texture
	
	xoffset = 0;
	for(i = 0; i < charlen; i++) {
		int paddedw, charHeight, bearingY;
		
		
		err = FT_Load_Char(fontFace, chars[i], FT_LOAD_RENDER);
		
		paddedw = (slot->metrics.width >> 6) + padding;
		bearingY = slot->metrics.horiBearingY >> 6;
		charHeight = slot->metrics.height >> 6;
		
		res->charWidths[i] = paddedw + padding;
		
/*		printf("meh: %d\n", height - charHeight);
		printf("index: %d, char: %c, xoffset: %d, pitch: %d \n", i, chars[i], xoffset, slot->bitmap.pitch);
		printf("m.width: %d, m.height: %d, hbearing: %d, habove: %d \n\n", slot->metrics.width >> 6, slot->metrics.height >> 6, slot->metrics.horiBearingY >> 6, h_above);
*/		blit(
			0, 0, // src x and y offset for the image
			xoffset + padding, padding + (h_above - bearingY), // dst offset
			slot->metrics.width >> 6, slot->metrics.height >> 6, // width and height BUG probably
			slot->bitmap.pitch, width, // src and dst row widths
			slot->bitmap.buffer, // source
			res->texture); // destination
		
		
		res->codeIndex[chars[i]] = i;
		res->offsets[i] = xoffset;
		res->valign[i] = (height - h_above + bearingY - padding);// + (slot->metrics.horiBearingY >> 6);
		xoffset += paddedw;
	}
	
	

	
	// kerning map
	res->kerning = (unsigned char*)malloc(charlen * charlen);
	for(i = 0; i < charlen; i++) {
		FT_UInt left, right;
		FT_Vector k;
		
		left = FT_Get_Char_Index(fontFace, chars[i]);
		
		for(j = 0; j < charlen; j++) {
			
			right = FT_Get_Char_Index(fontFace, chars[j]);
			
			FT_Get_Kerning(fontFace, left, right, FT_KERNING_DEFAULT, &k);
		//	if(k.x != 0) printf("k: (%c%c) %d, %d\n", chars[i],chars[j], k.x, k.x >> 6);
			res->kerning[(i * charlen) + j] = k.x >> 6;
		}
	}
	
	
	
	//////////// opengl stuff ////////////
	
	// TODO: error checking
	glGenTextures(1, &res->textureID);
	glBindTexture(GL_TEXTURE_2D, res->textureID);
	glerr("bind font tex");
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); // no mipmaps for this; it'll get fucked up
	glerr("param font tex");
	
	printf("text width: %d, height: %d \n", width, height);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, res->texture);
	glerr("load font tex");
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	return res;
}


static int bmpsort(GlyphBitmap** a, GlyphBitmap** b) {
	return (*a)->h - (*b)->h;
}

// save a pre-rendered sdf font
void SaveSDFFont(char* path, TextRes* res) {
	FILE* f;
	unsigned short version = 1;
	uint16_t n;
	
	f = fopen(path, "wb");
	
	// write version
	fwrite(&version, 2, 1, f);
	
	// save font info
	
	// sizes
	fwrite(&res->maxWidth, 2, 1, f);
	fwrite(&res->maxHeight, 2, 1, f);
	fwrite(&res->texWidth, 2, 1, f);
	fwrite(&res->texHeight, 2, 1, f);
	fwrite(&res->padding, 2, 1, f);
	fwrite(&res->charLen, 2, 1, f);
	fwrite(&res->indexLen, 2, 1, f);
	
	//charset
	n = res->charLen;
	fwrite(res->charSet, 1, n, f);
	
	fwrite(res->codeIndex, 1, res->indexLen, f);
	fwrite(res->offsets, 2, n, f);
	fwrite(res->kerning, 1, n, f);
	fwrite(res->valign, 1, n, f);
	fwrite(res->charWidths, 2, n, f);
	
	// the data
	fwrite(res->texture, 1, res->texWidth * res->texHeight, f);
	
	
	fclose(f);
}
// load a pre-rendered sdf font
TextRes* LoadSDFFont(char* path) {
	FILE* f;
	TextRes* res;
	unsigned short version;
	int n;
	
	f = fopen(path, "rb");
	if(!f) {
		return NULL;
	}
	
	res = calloc(1, sizeof(TextRes));
	
	fread(&version, 2, 1, f);
	
	// sizes
	fread(&res->maxWidth, 2, 1, f);
	fread(&res->maxHeight, 2, 1, f);
	fread(&res->texWidth, 2, 1, f);
	fread(&res->texHeight, 2, 1, f);
	fread(&res->padding, 2, 1, f);
	fread(&res->charLen, 2, 1, f);
	fread(&res->indexLen, 2, 1, f);
	
	//charset
	n = res->charLen;
	res->charSet = malloc(n);
	fread(res->charSet, 1, n, f);
	
	res->codeIndex = malloc(res->indexLen);
	fread(res->codeIndex, 1, res->indexLen, f);
	
	res->offsets = malloc(2 * n);
	fread(res->offsets, 2, n, f);
	
	res->kerning = malloc(n);
	fread(res->kerning, 1, n, f);
	
	res->valign = malloc(n);
	fread(res->valign, 1, n, f);
	
	res->charWidths = malloc(2 * n);
	fread(res->charWidths, 2, n, f);
	
	// the data
	res->texture = malloc(res->texWidth * res->texHeight);
	fread(res->texture, 1, res->texWidth * res->texHeight, f);
	
	
	fclose(f);
	
	// set up the texture
	glGenTextures(1, &res->textureID);
	glBindTexture(GL_TEXTURE_2D, res->textureID);
	glerr("bind font tex");
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); // no mipmaps for this; it'll get fucked up
	glerr("param font tex");
	
	printf("text width: %d, height: %d \n", res->texWidth, res->texHeight);
// 	printf("text width 3: %d, height: %d \n", width, height);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, res->texWidth, res->texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, res->texture);
	glerr("load font tex");
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	return res;
}

// generate a new sdf font
TextRes* GenerateSDFFont(char* fontName, int size, char* chars) {
	FT_Error err;
	FT_GlyphSlot slot;
	FT_Face fontFace; 
	TextRes* res;
	int i, j, charlen, width, h_above, h_below, height, padding, xoffset, oversample;
	char* fontPath;
	
	padding = 0;
	
	res = calloc(1, sizeof(TextRes));
	
	res->fontInfo = LoadFontInfo(fontName);
	
	fontFace = res->fontInfo->fontFace;
	
	//debug:
	//size = 16;
	oversample = 8; // 16 is better
	
	err = FT_Set_Pixel_Sizes(fontFace, 0, size * oversample);
	if(err) {
		fprintf(stderr, "Could not set pixel size to %dpx.\n", size);
		free(res->fontInfo);
		free(res);
		return NULL;
	}
	
	// slot is a pointer
	slot = fontFace->glyph;
	
	if(!chars) chars = defaultCharset;
	res->charSet = strdup(chars);
	
	charlen = strlen(chars);
	res->charLen = charlen;
	
	h_above = 0;
	h_below = 0;
	width = 4*1024;
	height = 32;
	res->texWidth = width;
	res->texHeight = height; // may not always just be one row
	res->maxHeight = height;
	
	res->texture = (unsigned char*)malloc(width * height);
	memset(res->texture, 0xff, width * height);
	res->offsets = (unsigned short*)calloc(charlen * sizeof(unsigned short), 1);
	res->charWidths = (unsigned short*)calloc(charlen * sizeof(unsigned short), 1);
	res->valign = (unsigned char*)calloc(charlen * sizeof(unsigned char), 1);
	
	GlyphBitmap* gbs = calloc(1, charlen * sizeof(GlyphBitmap));
	
	res->indexLen = 128; // 7 bits for now.
	res->codeIndex = (unsigned char*)calloc(res->indexLen, 1);
	

	int max_h_bearing = 0;
	int max_w = 0;
	int bearingY = 0;
	
	// get character dimensions
	for(i = 0; i < charlen; i++) {
		int ymin;
		err = FT_Load_Char(fontFace, chars[i], FT_LOAD_DEFAULT | FT_LOAD_MONOCHROME);
		
		ymin = slot->metrics.height >> 6;// - f2f(slot->metrics.horiBearingY);

//		printf("%c-----\nymin: %d \n", chars[i], ymin);
//		printf("bearingY: %d \n", slot->metrics.horiBearingY >> 6);
//		printf("width: %d \n", slot->metrics.width >> 6);
//		printf("height: %d \n\n", slot->metrics.height >> 6);

		// this is the opposite value. it will be adjusted in the next loop
		res->valign[i] =  (slot->metrics.horiBearingY >> 6);
		//res->charWidths[i] = (slot->metrics.width >> 6) / oversample;
		max_h_bearing = MAX(res->valign[i], max_h_bearing);
		printf("char: %c yoff: %d \n", chars[i], (slot->metrics.horiBearingY >> 6) / oversample);
		//width += f2f(slot->metrics.width);
		h_above = MAX(h_above, slot->metrics.horiBearingY >> 6);
		h_below = MAX(h_below, ymin);
	}
	
	int tex_width = nextPOT(width);
	

	
	
	// render SDF's
	for(i = 0; i < charlen; i++) {
		gbs[i].oversample = oversample;
		gbs[i].magnitude = 4;
		gbs[i].code = chars[i];
		DrawGlyph(res, &gbs[i]);
		printf("calculating sdf for %d (%c)... ", chars[i], chars[i]);
		
		//writePNG("sdfpct.png", 1, gbs[i].data, gbs[i].dw, gbs[i].dh);
		
		CalcSDF_Software(res, &gbs[i]);

		//writePNG("sdfcalced.png", 1, gbs[i].sdfData, gbs[i].sdfw, gbs[i].sdfh);

		printf("done\n");
		
		res->valign[i] = (max_h_bearing - res->valign[i]) / oversample;
		//exit(2);
		
	}


	// copy sdf's to atlas texture
	GlyphBitmap** charOrdering = malloc(charlen * sizeof(GlyphBitmap*));
	for(i = 0; i < charlen; i++) charOrdering[i] = &gbs[i];
	
	qsort(charOrdering, charlen, sizeof(GlyphBitmap*), bmpsort);
	
	int yoffset = 0;
	xoffset = 0;
	for(i = 0; i < charlen; i++) {
		blit(
			0, 0, // src x and y offset for the image
			xoffset + padding, padding + 0, // dst offset
			gbs[i].sdfw, gbs[i].sdfh, // width and height BUG probably
			gbs[i].sdfw, width, // src and dst row widths
			gbs[i].sdfData, // source
			res->texture); // destination
		
 		res->codeIndex[chars[i]] = i;
		res->charWidths[i] = gbs[i].sdfw;
		res->offsets[i] = xoffset;
		xoffset += gbs[i].sdfw;
	}
	
	writePNG("sdf.png", 1, res->texture, width, height);
	
	// kerning map
	res->kerning = (unsigned char*)malloc(charlen * charlen);
	for(i = 0; i < charlen; i++) {
		FT_UInt left, right;
		FT_Vector k;
		
		left = FT_Get_Char_Index(fontFace, chars[i]);
		
		for(j = 0; j < charlen; j++) {
			
			right = FT_Get_Char_Index(fontFace, chars[j]);
			
			FT_Get_Kerning(fontFace, left, right, FT_KERNING_DEFAULT, &k);
		//	if(k.x != 0) printf("k: (%c%c) %d, %d\n", chars[i],chars[j], k.x, k.x >> 6);
			res->kerning[(i * charlen) + j] = k.x >> 6;
		}
	}
	

	///////////////////////////////////////////////////////////////////
	
		// TODO: error checking
	glGenTextures(1, &res->textureID);
	glBindTexture(GL_TEXTURE_2D, res->textureID);
	glerr("bind font tex");
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); // no mipmaps for this; it'll get fucked up
	glerr("param font tex");
	
	printf("text width: %d, height: %d \n", tex_width, height);
	printf("text width 3: %d, height: %d \n", width, height);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, res->texture);
	glerr("load font tex");
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	free(gbs);
	
	return res;
}

int DrawGlyph(TextRes* res, GlyphBitmap* gb) {
	
	int padding;
	FT_GlyphSlot slot;
	
	padding = gb->oversample * gb->magnitude;
		
	// have freetype draw the character
	FT_Load_Char(res->fontInfo->fontFace, gb->code, FT_LOAD_RENDER);
	
	slot = res->fontInfo->fontFace->glyph;
	
	// this is the size of the original giant character
	gb->w = slot->metrics.width >> 6; 
	gb->h = slot->metrics.height >> 6; 
	
	gb->dw = gb->w + padding + padding;
	gb->dh = gb->h + padding + padding;
// 		
// 	gb->dw = gb->paddedw; //nextPOT(gb->paddedw);
// 	gb->dh = gb->paddedh; //nextPOT(gb->paddedh);
	fprintf(stdout, "glyph bitmap size [%d, %d]\n", gb->dw, gb->dh);

	gb->data = calloc(1, gb->dw * gb->dh * sizeof(uint8_t));
	
		
/*		printf("meh: %d\n", height - charHeight);
		printf("index: %d, char: %c, xoffset: %d, pitch: %d \n", i, chars[i], xoffset, slot->bitmap.pitch);
		printf("m.width: %d, m.height: %d, hbearing: %d, habove: %d \n\n", slot->metrics.width >> 6, slot->metrics.height >> 6, slot->metrics.horiBearingY >> 6, h_above);
*/	blit(
		0, 0, // src x and y offset for the image
		padding, padding, // dst offset
		gb->w, gb->h, // width and height BUG probably
		slot->bitmap.pitch, gb->dw, // src and dst row widths
		slot->bitmap.buffer, // source
		gb->data); // destination

	return 0;
}

static float dist(int a, int b) {
	return a*a + b*b;
}
static float dmin(int a, int b, float d) {
	return fmin(dist(a, b), d);
}

static int boundedOffset(int x, int y, int ox, int oy, int w, int h) {
	int x1 = x + ox;
	int y1 = y + oy;
	if(x1 < 0 || y1 < 0 || x1 >= w || y1 >= h) return -1;
	return x1 + (w * y1);
}

static uint8_t sdfEncode(float d, int inside, float maxDist) {
	int o;
	d = sqrt(d);
	float norm = d / maxDist;
	if(inside) norm = -norm;
	
	o = (norm * 192) + 64;
	
	return o < 0 ? 0 : (o > 255 ? 255 : o);
}

void CalcSDF_Software(TextRes* res, GlyphBitmap* gb) {
	
	int searchSize;
	int x, y, ox, oy, sx, sy;
	int dw, dh;
	
	uint8_t* data;
	uint8_t* output;
	
	float d, maxDist;
	
	searchSize = gb->oversample * gb->magnitude;
	maxDist = 0.5 * searchSize;
	dw = gb->dw;
	dh = gb->dh;
	data = gb->data;
	
	// this is wrong
	gb->sdfw = (dw / (gb->oversample)); 
	gb->sdfh = (dh / (gb->oversample)); 
	
	gb->sdfData = output = calloc(1, gb->sdfw * gb->sdfh * sizeof(uint8_t));
	
	for(y = 0; y < gb->sdfh; y++) {
		for(x = 0; x < gb->sdfw; x++) {
			int sx = x * gb->oversample;
			int sy = y * gb->oversample;
			printf(".");
			// value right under the center of the pixel, to determine if we are inside
			// or outside the glyph
			int v = data[sx + (sy * dw)];
			
			d = 999999.9;
			
			
			for(oy = -searchSize / 2; oy < searchSize; oy++) {
				for(ox = -searchSize / 2; ox < searchSize; ox++) {
					int off = boundedOffset(sx, sy, ox, oy, dw, dh);
					if(off >= 0 && data[off] != v) 
						d = dmin(ox, oy, d);
				}
			}
			
			int q = sdfEncode(d, v, maxDist);
			if(q) { 
// 				gb->sdfdims.left = MIN(gb->sdfdims.left, x);
// 				gb->sdfdims.bottom = MIN(gb->sdfdims.bottom, y);
// 				gb->sdfdims.right = MAX(gb->sdfdims.right, x);
// 				gb->sdfdims.top = MAX(gb->sdfdims.top, y);
			}
			
			output[x + (y * gb->sdfw)] = q;
		}
	}
}





TextRenderInfo* prepareText(TextRes* font, const char* str, int len, unsigned int* colors) {
	
	float offset;
	int v, i;
	TextRenderInfo* tri;
	float uscale, vscale, scale;
	unsigned int color;
	
	unsigned int defaultColor[] = {0xBADA55FF, INT_MAX}; // you have serious problems if the string is longer than INT_MAX
	
	if(!colors)
		colors = &defaultColor[0];
	
	//TODO:
	// normalize uv's
	// investigate and fix kerning, it seems off
	// move VAO to a better spot
	
	
	if(len == -1) len = strlen(str);
	
	//printf("len %d\n", len);
	
	// create vao/vbo
	tri = (TextRenderInfo*)malloc(sizeof(TextRenderInfo));
	tri->vertices = (TextVertex*)malloc(len * 2 * 3 * sizeof(TextVertex));
	tri->font = font;
	tri->text = strdup(str);
	tri->textLen = len;
	
	//move this to a global
	glGenVertexArrays(1, &tri->vao);
	glBindVertexArray(tri->vao);

	// have to create and bind the buffer before specifying its format
	glGenBuffers(1, &tri->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tri->vbo);

	
	// vertex
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)0);
	glerr("pos attrib");
	// uvs
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)(3*4));
	glerr("uv attrib");

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TextVertex), (void*)(5*4));
	glerr("color attrib");

	
	makeVertices(tri, colors);
	
 	glBufferData(GL_ARRAY_BUFFER, tri->vertexCnt * sizeof(TextVertex), tri->vertices, GL_STATIC_DRAW);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(testarr), testarr, GL_STATIC_DRAW);
	glerr("buffering text vertices");
	
	// init shaders elsewhere
	
	return tri;
}


// internal use.
static void makeVertices(TextRenderInfo* tri, unsigned int* colors) {
	float offset, uscale, vscale, scale;
	int v, i;
	char* str;
	TextRes* font;
	unsigned int color;
	
	str = tri->text;
	font = tri->font;
	
	uscale = 1.0 / font->texWidth;
	vscale = 1.0 / font->texHeight;
	scale = 1.0 / font->maxHeight;
	
	offset = 0;
	v = 0;
	for(i = 0; i < tri->textLen; i++) {
		//printf("loop\n");
		float width, valign, kerning;
		float tex_offset, to_next;
		int index, prev;
		
		//increment the colors pointer if we reach the next change point
		if(i == colors[1]) colors += 2;
		
		color = *colors;
		
		
		index = font->codeIndex[str[i]];
		prev = font->codeIndex[str[i-1]];
// 		width = font->kerning[index];
		width = font->charWidths[index] * scale;
		//printf("width: %f\n", width);
		tex_offset = (font->offsets[index] + font->padding) * uscale;
		to_next = (font->offsets[index + 1] + font->padding) * uscale; // bug at end of array
		
		kerning = 0;
		/*
		if(i > 0)
			kerning = (font->kerning[(index * font->charLen) + prev]) * uscale; // bug at end of array
		*/
		offset -= ((font->padding * 2) * vscale);
		offset -= kerning;
		offset -= .15;
		
	
		//printf("kerning: %f\n", kerning);
		
	//	printf("index: %d, char: %c\n", index, str[i]);
	//	printf("offset %f\n", tex_offset);
	//	printf("next o %f\n", (float)to_next * uscale);
	//	printf("uscale %f\n", uscale);
		//printf("valign %d\n", font->valign[index]);
	//	printf("width %f\n\n", width);
	
		valign = font->valign[index] * vscale;
		//tex_offset = 1;

		// add quad, set uv's
		// triangle 1
		tri->vertices[v].x = width + offset;
		tri->vertices[v].y = 1.0 + valign;
		tri->vertices[v].z = 0.0;
		tri->vertices[v].rgba = color;
		tri->vertices[v].u = to_next;
		tri->vertices[v++].v = 1.0;
		
		// top left
		tri->vertices[v].x = width + offset;
		tri->vertices[v].y = 0.0 + valign;
		tri->vertices[v].z = 0.0;
		tri->vertices[v].rgba = color;
		tri->vertices[v].u = to_next;
		tri->vertices[v++].v = 0.0;

		// top right
		tri->vertices[v].x = 0.0 + offset;
		tri->vertices[v].y = 1.0 + valign;
		tri->vertices[v].z = 0.0;
		tri->vertices[v].rgba = color;
		tri->vertices[v].u = tex_offset;
		tri->vertices[v++].v = 1.0;
		
		
		// triangle 2
		tri->vertices[v].x = 0.0 + offset;
		tri->vertices[v].y = 1.0 + valign;
		tri->vertices[v].z = 0.0;
		tri->vertices[v].rgba = color;
		tri->vertices[v].u = tex_offset;
		tri->vertices[v++].v = 1.0;
		
		// top right
		tri->vertices[v].x = width + offset;
		tri->vertices[v].y = 0.0 + valign;
		tri->vertices[v].z = 0.0;
		tri->vertices[v].rgba = color;
		tri->vertices[v].u = to_next;
		tri->vertices[v++].v = 0.0;
		
		// bottom right
		tri->vertices[v].x = 0.0 + offset;
		tri->vertices[v].y = 0.0 + valign;
		tri->vertices[v].z = 0.0;
		tri->vertices[v].rgba = color;
		tri->vertices[v].u = tex_offset;
		tri->vertices[v++].v = 0.0;
		
		
		// move right by kerning amount
		offset += width; //tex_offset;
	}
	
	tri->vertexCnt = v;

}


void FreeFont(TextRes* res) {
	free(res->texture);
	free(res->codeIndex);
	free(res->offsets);
	free(res->valign);
	free(res->kerning);
	free(res->charSet);
	
	free(res);
};
	
	
// super nifty site:
// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
int nextPOT(int in) {
	
	in--;
	in |= in >> 1;
	in |= in >> 2;
	in |= in >> 4;
	in |= in >> 8;
	in |= in >> 16;
	in++;
	
	return in;
}


void updateText(TextRenderInfo* tri, const char* str, int len, unsigned int* colors) {
	
	if(len <= 0) len = strlen(str);
	
	// text hasn't changed
	if(0 == strcmp(str, tri->text)) return;
	
	// need to check if colors changes too, but meh
	
	// reallocate the vertex data buffer if it's too small
	if(tri->vertices && len > tri->textLen) {
		free(tri->vertices);
		tri->vertices = (TextVertex*)malloc(len * 2 * 3 * sizeof(TextVertex));
	}
	
	if(tri->text) free(tri->text);
	tri->text = strdup(str);
	tri->textLen = len;
	
	// i've read that trying to update a vbo when opengl is rendering can force a
	// disasterous gpu<->cpu sync. here the old vbo is released (hopefully async)
	// and the next frame will get the new vbo (all hopefully async). this has not
	// been comparatively tested.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLuint oldvbo = tri->vbo;
	
	glBindVertexArray(tri->vao);
	
	glGenBuffers(1, &tri->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tri->vbo);
	glexit("update text buffer creation");
	
	glDeleteBuffers(1, &oldvbo);
	
	// vertex
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)0);
	glerr("pos attrib");
	// uvs
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)(3*4));
	glerr("uv attrib");

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TextVertex), (void*)(5*4));
	glerr("color attrib");


	
	makeVertices(tri, colors);
	
 	glBufferData(GL_ARRAY_BUFFER, tri->vertexCnt * sizeof(TextVertex), tri->vertices, GL_STATIC_DRAW);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(testarr), testarr, GL_STATIC_DRAW);
	glerr("buffering text vertices");

}

/*
void renderText(TextRenderInfo* tri) {
	glUseProgram(textProg->id);
	
	// text stuff
	textProj = IDENT_MATRIX;
	textModel = IDENT_MATRIX;
	
	mOrtho(0, 1, 0, 1, -1, 100, &textProj);
	//mScale3f(.5,.5,.5, &textProj);
	
	mScale3f(.06, .06, .06, &textModel);
	
	GLuint tp_ul = glGetUniformLocation(textProg->id, "mProj");
	GLuint tm_ul = glGetUniformLocation(textProg->id, "mModel");
	GLuint ts_ul = glGetUniformLocation(textProg->id, "fontTex");
	
	glUniformMatrix4fv(tp_ul, 1, GL_FALSE, textProj.m);
	glUniformMatrix4fv(tm_ul, 1, GL_FALSE, textModel.m);
	glexit("text matrix uniforms");

	glDisable(GL_CULL_FACE);
	
	glActiveTexture(GL_TEXTURE0);
	glexit("active texture");

	glUniform1i(ts_ul, 0);
	glexit("text sampler uniform");
	glBindTexture(GL_TEXTURE_2D, arial->textureID);
	glexit("bind texture");
	
	
	glBindVertexArray(strRI->vao);
	glexit("text vao bind");
	
	glBindBuffer(GL_ARRAY_BUFFER, strRI->vbo);
	glexit("text vbo bind");
	glDrawArrays(GL_TRIANGLES, 0, strRI->vertexCnt);
	glexit("text drawing");
	
	
}
*/


