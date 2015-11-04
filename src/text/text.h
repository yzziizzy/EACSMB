
#include <ft2build.h>
#include FT_FREETYPE_H



typedef struct {
	FT_Face fontFace;
	char* charSet;
	unsigned char* texture;
	unsigned char* codeIndex;
	short charLen;
	short indexLen;
	unsigned short* offsets;  // texture offsets
	unsigned char* kerning;
	unsigned char* valign; // vertical offset for gutters
	unsigned short* charWidths; // quad widths
	
	short maxWidth, maxHeight;
	short padding;
	
	GLuint textureID;
	short texWidth;
	short texHeight;
	
} TextRes;


typedef struct {
	float x,y,z;
	float u,v;
	unsigned int rgba;
} TextVertex;

typedef struct {
	TextVertex* vertices;
	int vertexCnt;
	
	char* text;
	int textLen;
	
	GLuint vao; // need to move vao to font? global?
	GLuint vbo;
	
	TextRes* font;
	
} TextRenderInfo;



// this function is rather expensive. it rebinds textures.
TextRes* LoadFont(char* path, int size, char* chars);


TextRenderInfo* prepareText(TextRes* font, const char* str, int len, unsigned int* colors);

void FreeFont(TextRes* res);

void updateText(TextRenderInfo* tri, const char* str, int len, unsigned int* colors);










