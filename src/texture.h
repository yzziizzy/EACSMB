

typedef struct {
	GLuint tex_id;
	short width;
	short height;
	
	
} Texture;

typedef struct {
	char* path;
	short width, height;
	uint32_t* data;
} BitmapRGBA8;

Texture* loadDataTexture(unsigned char* data, short width, short height);

BitmapRGBA8* readPNG(char* path);

Texture* loadBitmapTexture(char* path);









