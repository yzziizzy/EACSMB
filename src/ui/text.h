#ifndef __EACSMB_ui_text_h__
#define __EACSMB_ui_text_h__


typedef struct GUIText {
	GUIHeader header;
	
	
	char* current;
	
	Vector pos;
	float size;
	
	// align, height, width wrapping
	
	TextRes* font;
	TextRenderInfo* strRI;
	
	
} GUIText;



GUIText* guiTextNew(char* str, Vector2 pos, float size, char* fontname);

void guiTextSetValue(GUIText* gt, char* newval);
float guiTextGetTextWidth(GUIText* gt, int numChars);



#endif // __EACSMB_ui_text_h__
