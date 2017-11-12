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






#endif // __EACSMB_texture_h__



