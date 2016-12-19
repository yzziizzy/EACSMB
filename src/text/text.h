
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	FT_Face fontFace;
	
	
} FontInfo;

typedef struct {
	int code;
	int fontSize;
	int oversample, magnitude;
	
	// size of the original giant character in the font
	int w, h;
	int paddedw, paddedh; // deprecated
	
	// need kerning and offset data
	
	int dw, dh; // size of the oversampled padded glyph
	int sdfw, sdfh; // size of the calculated sdf image
	uint8_t* data;
	uint8_t* sdfData;
	
	struct {
		int left, top;
		int right, bottom;
	} sdfdims;
	
} GlyphBitmap;

typedef struct {
	FontInfo* fontInfo;
	char* charSet;
	uint8_t* texture;
	uint8_t* codeIndex;
	uint16_t charLen;
	uint16_t indexLen;
	uint16_t* offsets;  // texture offsets
	uint8_t* kerning;
	uint8_t* valign; // vertical offset for gutters
	uint16_t* charWidths; // quad widths
	
	uint16_t maxWidth, maxHeight;
	uint16_t padding;
	
	GLuint textureID;
	uint16_t texWidth;
	uint16_t texHeight;
	
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


// handles fontconfig and freetype loading
FontInfo* LoadFontInfo(char* fontName);

// this function is rather expensive. it rebinds textures.
TextRes* LoadFont(char* path, int size, char* chars);



TextRenderInfo* prepareText(TextRes* font, const char* str, int len, unsigned int* colors);

void FreeFont(TextRes* res);

void updateText(TextRenderInfo* tri, const char* str, int len, unsigned int* colors);



TextRes* LoadSDFFont(char* path);

TextRes* GenerateSDFFont(char* fontName, int size, char* chars);





