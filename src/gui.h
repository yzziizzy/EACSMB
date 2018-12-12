#ifndef __EACSMB_GUI_H__
#define __EACSMB_GUI_H__

#include <stdatomic.h>

#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "input.h"
#include "texture.h"
// #include "game.h"
#include "pass.h"
#include "pcBuffer.h"

#include "font.h"


struct GameState;
typedef struct GameState GameState;

struct GUIManager;
typedef struct GUIManager GUIManager;



#define GUI_GRAV_TOP_LEFT      0x00
#define GUI_GRAV_CENTER_LEFT   0x01
#define GUI_GRAV_BOTTOM_LEFT   0x02
#define GUI_GRAV_CENTER_BOTTOM 0x03
#define GUI_GRAV_BOTTOM_RIGHT  0x04
#define GUI_GRAV_CENTER_RIGHT  0x05
#define GUI_GRAV_TOP_RIGHT     0x06
#define GUI_GRAV_CENTER_TOP    0x07
#define GUI_GRAV_CENTER        0x08


struct Color4 {
	uint8_t r,g,b,a;
} __attribute__ ((packed));

struct Color3 {
	uint8_t r,g,b;
} __attribute__ ((packed));


typedef struct GUIUnifiedVertex {
	struct { float l, t, r, b; } pos;
	struct { float l, t, r, b; } clip;
	uint8_t texIndex1, texIndex2, texFade, guiType; 
	struct { uint16_t x, y; } texOffset1, texOffset2;
	struct { uint16_t x, y; } texSize1, texSize2;
	
	struct Color4 fg;
	struct Color4 bg;
	
	float z, alpha, opt1, opt2;
	
} __attribute__ ((packed)) GUIUnifiedVertex;





typedef union GUIObject GUIObject;
struct GUIManager;

struct GUIRenderParams;
typedef struct GUIRenderParams GUIRenderParams;


struct gui_vtbl {
	void (*Render)(GUIObject* go, GUIRenderParams* grp, PassFrameParams* pfp);
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


typedef int  (*GUI_OnClickFn)(GUIEvent* e, Vector2 clickPos);
typedef void (*GUI_OnMouseEnterFn)(GUIEvent* e);
typedef void (*GUI_OnMouseLeaveFn)(GUIEvent* e);



typedef struct GUIHeader {
	struct GUIManager* gm;
	GUIObject* parent;
	struct gui_vtbl* vt;

	// fallback for easy hit testing
	VEC(union GUIObject*) children;
	
	Vector2 topleft; // relative to parent (and window padding)
	Vector2 size; // absolute
	float scale;
	float alpha;
	float z; // relative to the parent
	
	AABB2 hitbox; // in local coordinates
	
	char hidden;
	char deleted;
	char gravity;
	
	
	GUI_OnClickFn onClick;
	GUI_OnMouseEnterFn onMouseEnter;
	GUI_OnMouseLeaveFn onMouseLeave;
	
} GUIHeader;







#include "ui/window.h"
#include "ui/text.h"
#include "ui/simpleWindow.h"
#include "ui/image.h"
#include "ui/edit.h"
#include "ui/columnLayout.h"



union GUIObject {
	GUIHeader h; // legacy
	GUIHeader header;
	GUIText text;
	GUIWindow window;
	GUISimpleWindow simpleWindow;
	GUIImage image;
	GUIColumnLayout columnLayout;
};






/*
The general idea is this:
The gui is held in a big tree. The tree is walked depth-first from the bottom up, resulting in
a mostly-sorted list. A sort is then run on z-index to ensure proper order. The list is then
rendered all at once through a single unified megashader using the geometry stage to expand
points into quads.

*/
typedef struct GUIManager {
	int maxInstances;
	PCBuffer instVB;
	GLuint vao;
	
	int elementCount;
	int elementAlloc;
	GUIUnifiedVertex* elemBuffer;
	
	Vector2i screenSize;
	
	GUIObject* root;
	VEC(GUIObject*) reapQueue; 
	
	FontManager* fm;
	TextureAtlas* ta;
	
	// temp 
	GLuint fontAtlasID;
	GLuint atlasID;
	
} GUIManager;



void GUIManager_init(GUIManager* gm, GlobalSettings* gs);
void GUIManager_initGL(GUIManager* gm, GlobalSettings* gs);
GUIManager* GUIManager_alloc(GlobalSettings* gs);


RenderPass* GUIManager_CreateRenderPass(GUIManager* gm);
PassDrawable* GUIManager_CreateDrawable(GUIManager* gm);





// --------------temp----------------
typedef struct GUITextArea {
	GUIHeader header;
	
	
	char* current;
	
	Vector pos;
	float size;
	
	// align, height, width wrapping
	
	GUIFont* font;
//	TextRenderInfo* strRI;
	
	
} GUITextArea;
// ^^^^^^^^^^^^^temp^^^^^^^^^^^^^^^^








GUIObject* GUIObject_hitTest(GUIObject* go, Vector2 testPos);
GUIObject* GUIManager_hitTest(GUIManager* gm, Vector2 testPos);




// GUIObject* guiHitTest(GUIObject* go, Vector2 testPos);
void guiDelete(GUIObject* go);
// void guiRender(GUIObject* go, GameState* gs, PassFrameParams* pfp);
void guiReap(GUIObject* go);
void guiResize(GUIHeader* gh, Vector2 newSz);
int guiRemoveChild(GUIObject* parent, GUIObject* child);


void guiTriggerClick(GUIEvent* e); 

#define GUIRegisterObject(o, p) GUIRegisterObject_(&(o)->header, (p) ? (&((GUIObject*)(p))->header) : NULL)
void GUIRegisterObject_(GUIHeader* o, GUIHeader* parent);


void guiSetClientSize(GUIObject* go, Vector2 cSize);
Vector2 guiGetClientSize(GUIObject* go);
Vector2 guiRecalcClientSize(GUIObject* go);
void guiAddClient(GUIObject* parent, GUIObject* child);
void guiRemoveClient(GUIObject* parent, GUIObject* child);


#endif // __EACSMB_GUI_H__
