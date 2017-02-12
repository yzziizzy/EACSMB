

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






