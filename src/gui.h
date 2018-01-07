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
	void (*Reap)(GUIObject* go);
	void (*Resize)(GUIObject* go, Vector2 newSz);
	GUIObject* (*HitTest)(GUIObject* go, Vector2 testPos);
};



typedef struct GUIEvent {
	double eventTime;
	Vector2 eventPos;
	GUIObject* originalTarget;
	GUIObject* currentTarget;
	
} GUIEvent;


typedef int  (*GUI_OnClickFn)(GUIEvent* e);
typedef void (*GUI_OnMouseEnterFn)(GUIEvent* e);
typedef void (*GUI_OnMouseLeaveFn)(GUIEvent* e);


typedef struct GUIHeader {
	Vector2 topleft;
	Vector2 size;
	float scale; // meaning scale, apparently
	float alpha;
	float z;
	
	AABB2 hitbox;
	
	char hidden;
	char deleted;
	
	VEC(union GUIObject*) children;
	GUIObject* parent;
	
	struct gui_vtbl* vt;
	
	GUI_OnClickFn onClick;
	GUI_OnMouseEnterFn onMouseEnter;
	GUI_OnMouseLeaveFn onMouseLeave;
	
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
	
// 	uint32_t color;
	Vector color;
	
	float zindex;
	
} GUIWindow;



#include "ui/simpleWindow.h"
#include "ui/image.h"


union GUIObject {
	GUIHeader h; // legacy
	GUIHeader header;
	GUIText text;
	GUIWindow window;
	GUISimpleWindow simpleWindow;
	GUIImage image;
};




GUIText* guiTextNew(char* str, Vector* pos, float size, char* fontname);
void gui_Init();
void gui_Image_Init(char* file);

void gui_RenderAll(GameState* gs);

GUIObject* guiHitTest(GUIObject* go, Vector2 testPos);
void guiDelete(GUIObject* go);
void guiRender(GUIObject* go, GameState* gs);
void guiReap(GUIObject* go);
void guiResize(GUIHeader* gh, Vector2 newSz);
int guiRemoveChild(GUIObject* parent, GUIObject* child);

void guiTextSetValue(GUIText* gt, char* newval);

GUIWindow* guiWindowNew(Vector2 pos, Vector2 size, float zIndex);

void guiTriggerClick(GUIEvent* e); 

#define guiRegisterObject(o, p) guiRegisterObject_(&(o)->header, p)
void guiRegisterObject_(GUIHeader* o, GUIHeader* parent);





#endif // __MV_GUI_H__
