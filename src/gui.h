#ifndef __MV_GUI_H__
#define __MV_GUI_H__


#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "game.h"




typedef union GUIObject GUIObject;


struct gui_vtbl {
	void (*Render)(GUIObject* go, GameState* gs);
	void (*Delete)(GUIObject* go);
	GUIObject* (*HitTest)(GUIObject* go, Vector2 testPos);
};


typedef struct GUIHeader {
	Vector2 topleft;
	Vector2 size;
	float scale; // meaning scale, apparently
	float alpha;
	
	AABB2 hitbox;
	
	char hidden;
	char deleted;
	
	struct gui_vtbl* vt;
	
	VEC(union GUIObject*) children;
	
} GUIHeader;


typedef struct GUIText {
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




union GUIObject {
	GUIHeader h;
	GUIText text;
	GUIWindow window;
};


GUIText* guiTextNew(char* str, Vector* pos, float size, char* fontname);
void gui_Init();
void gui_RenderAll(GameState* gs);

GUIObject* guiHitTest(GUIObject* go, Vector2 testPos);
void guiDelete(GUIObject* go);
void guiRender(GUIObject* go, GameState* gs);


void guiTextSetValue(GUIText* gt, char* newval);

GUIWindow* guiWindowNew(Vector* pos, Vector2* size);















#endif // __MV_GUI_H__
