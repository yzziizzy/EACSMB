#ifndef __MV_GUI_H__
#define __MV_GUI_H__


#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "game.h"


enum GUIType {
	GUITYPE_None,
	GUITYPE_Text,
	GUITYPE_Window,
	
	GUITYPE_MAX_VALUE
};


union GUIObject;

typedef struct {
	Vector2 topleft;
	Vector2 size;
	float scale; // meaning scale, apparently
	
	AABB2 hitbox;
	
	char hidden;
	char deleted;
	
	enum GUIType type;
	
	VEC(union GUIObject*) children;
	
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




typedef union GUIObject {
	GUIHeader h;
	GUIText text;
	GUIWindow window;
} GUIObject;


GUIText* guiTextNew(char* str, Vector* pos, float size, char* fontname);
void gui_Init();
void gui_RenderAll(GameState* gs);

GUIObject* guiHitTest(GUIObject* go, Vector2 testPos);


void guiTextRender(GUIText* gt, GameState* gs);
void guiTextDelete(GUIText* gt);
void guiTextSetValue(GUIText* gt, char* newval);


GUIWindow* guiWindowNew(Vector* pos, Vector2* size);

void guiWindowRender(GUIWindow* gw, GameState* gs);
void guiWindowDelete(GUIWindow* gw);














#endif // __MV_GUI_H__
