#ifndef __EACSMB_GUI_H__
#define __EACSMB_GUI_H__


#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "input.h"
#include "game.h"




typedef union GUIObject GUIObject;


struct gui_vtbl {
	void (*Render)(GUIObject* go, GameState* gs);
	void (*Delete)(GUIObject* go);
	void (*Reap)(GUIObject* go);
	void (*Resize)(GUIObject* go, Vector2 newSz); // exterior size
	Vector2 (*GetClientSize)(GUIObject* go);
	void    (*SetClientSize)(GUIObject* go, Vector2 cSize); // and force exterior size to match
	Vector2 (*RecalcClientSize)(GUIObject* go); // recalc client size from the client children and call SetClientSize
	GUIObject* (*HitTest)(GUIObject* go, Vector2 testPos);
	
	void (*AddClient)(GUIObject* parent, GUIObject* child);
	void (*RemoveClient)(GUIObject* parent, GUIObject* child);
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
	Vector2 topleft; // relative to parent (and window padding)
	Vector2 size; // absolute
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






#include "ui/window.h"
#include "ui/text.h"
#include "ui/simpleWindow.h"
#include "ui/image.h"
#include "ui/edit.h"


union GUIObject {
	GUIHeader h; // legacy
	GUIHeader header;
	GUIText text;
	GUIWindow window;
	GUISimpleWindow simpleWindow;
	GUIImage image;
};




void gui_Init();
void gui_Image_Init(char* file);

void gui_RenderAll(GameState* gs);

GUIObject* guiHitTest(GUIObject* go, Vector2 testPos);
void guiDelete(GUIObject* go);
void guiRender(GUIObject* go, GameState* gs);
void guiReap(GUIObject* go);
void guiResize(GUIHeader* gh, Vector2 newSz);
int guiRemoveChild(GUIObject* parent, GUIObject* child);


void guiTriggerClick(GUIEvent* e); 

#define guiRegisterObject(o, p) guiRegisterObject_(&(o)->header, p)
void guiRegisterObject_(GUIHeader* o, GUIHeader* parent);

void guiHeaderInit(GUIHeader* gh); 

void guiSetClientSize(GUIObject* go, Vector2 cSize);
Vector2 guiGetClientSize(GUIObject* go);
Vector2 guiRecalcClientSize(GUIObject* go);
void guiAddClient(GUIObject* parent, GUIObject* child);
void guiRemoveClient(GUIObject* parent, GUIObject* child);


#endif // __EACSMB_GUI_H__
