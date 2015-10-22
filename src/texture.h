

typedef struct {
	GLuint tex_id;
	short width;
	short height;
	
	
} DataTexture;

typedef struct {
	char* path;
	short width, height;
	uint32_t* data;
} BitmapRGBA8;

DataTexture* loadDataTexture(unsigned char* data, short width, short height);

BitmapRGBA8* readPNG(char* path);











