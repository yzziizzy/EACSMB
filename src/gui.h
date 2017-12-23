#ifndef __MV_GUI_H__
#define __MV_GUI_H__


#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "game.h"


typedef struct {
	Vector pos;
	float size; // meaning scale, apparently
	
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

typedef struct GUIWindow {
	GUIHeader header;
	
	Vector2 size;
	
	uint32_t color;
	
	float zindex;
	
} GUIWindow;




GUIText* guiTextNew(char* str, Vector* pos, float size, char* fontname);
void gui_Init();
void gui_RenderAll(GameState* gs);

void guiTextRender(GUIText* gt, GameState* gs);
void guiTextDelete(GUIText* gt);
void guiTextSetValue(GUIText* gt, char* newval);


GUIWindow* guiWindowNew(Vector* pos, Vector2* size);

void guiWindowRender(GUIWindow* gw, GameState* gs);
void guiWindowDelete(GUIWindow* gw);














#endif // __MV_GUI_H__
