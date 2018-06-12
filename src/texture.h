#ifndef __EACSMB_texture_h__
#define __EACSMB_texture_h__


typedef struct {
	GLuint tex_id;
	short width;
	short height;
	
	char* name;
	
} Texture;

typedef struct {
	char* path;
	short width, height;
	uint32_t* data;
} BitmapRGBA8;


enum TextureDepth {
	TEXDEPTH_8,
	TEXDEPTH_16,
	TEXDEPTH_32,
	TEXDEPTH_FLOAT,
	TEXDEPTH_DOUBLE,
	TEXDEPTH_MAXVALUE
};


typedef struct TexBitmap {
	short channels;
	enum TextureDepth depth;
	unsigned int width, height;
	
	union {
		uint8_t* data8;
		uint16_t* data16;
		uint32_t* data32;
		float* fdata;
		double* ddata;
	};
	
} TexBitmap;


typedef struct TexArray {
	unsigned short width, height;
	int depth;
	
	GLuint tex_id;
} TexArray;


Texture* loadDataTexture(unsigned char* data, short width, short height);

BitmapRGBA8* readPNG(char* path);
int readPNG2(char* path, BitmapRGBA8* bmp);

Texture* loadBitmapTexture(char* path);


TexArray* loadTexArray(char** files);


//Texture* Texture_acquireCustom(char* name);
Texture* Texture_acquirePath(char* path);
void Texture_release(Texture* tex);


void initTextures();

TexBitmap* TexBitmap_create(int w, int h, enum TextureDepth d, int channels); 


#endif // __EACSMB_texture_h__



