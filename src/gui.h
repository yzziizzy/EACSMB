#ifndef __MV_GUI_H__
#define __MV_GUI_H__


#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "game.h"


typedef struct {
	Vector pos;
	float size;
	
	char hidden;
	char deleted;
	
} GUIHeader;


typedef struct {
	GUIHeader header;
	
	
	char* current;
	
	Vector pos;
	float size;
	
	// align, height, width wrapping
	
	TextRes* font;
	TextRenderInfo* strRI;
	
	
} GUIText;



GUIText* guiTextNew(char* str, Vector* pos, float size, char* fontname);
void gui_Init();
void gui_RenderAll(GameState* gs);

void guiTextRender(GUIText* gt, GameState* gs);
void guiTextDelete(GUIText* gt);
void guiTextSetValue(GUIText* gt, char* newval);
















#endif // __MV_GUI_H__