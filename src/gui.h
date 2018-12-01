#ifndef __EACSMB_GUI_H__
#define __EACSMB_GUI_H__


#include "common_gl.h"
#include "common_math.h"

#include "text/text.h"
#include "input.h"
// #include "game.h"
#include "pass.h"
#include "pcBuffer.h"


struct GameState;
typedef struct GameState GameState;


struct Color4 {
	uint8_t r,g,b,a;
} __attribute__ ((packed));

struct Color3 {
	uint8_t r,g,b;
} __attribute__ ((packed));


typedef struct GUIUnifiedVertex {
	struct { float t, l, b, r; } pos;
	struct { float t, l, b, r; } clip;
	uint8_t texIndex1, texIndex2, texFade, guiType; 
	struct { uint16_t x, y; } texOffset1, texOffset2;
	struct { uint16_t x, y; } texSize1, texSize2;
	
	struct Color4 fg;
	struct Color4 bg;
	
} __attribute__ ((packed)) GUIUnifiedVertex;





typedef union GUIObject GUIObject;
struct GUIManager;


struct gui_vtbl {
	void (*Render)(GUIObject* go, GameState* gs, PassFrameParams* pfp);
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
	
	struct GUIManager* gm;
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



// ----------- new font rendering info --------------




#include <ft2build.h>
#include FT_FREETYPE_H


struct charInfo {
	int code;
	
	// final output texture coordinates
	Vector2i texelOffset; // from the top left
	Vector2i texelSize; // size of the character data in texels
	Vector2 texNormOffset; // normalized texture coordinates
	Vector2 texNormSize; 
	
	// typographic info
	float advance; // horizonatal distance to advance after this char
	Vector2 topLeftOffset; // offset from the baseline to the top left vertex of the quad
	Vector2 size;
	
};



// generation info for a single character



typedef struct GUIFont {
	
	char* name;
	
	int charsLen;
	struct charInfo* regular;
	struct charInfo* italic;
	struct charInfo* bold;
	
	// TODO: kerning info
	
} GUIFont;


typedef struct FontGen {
	GUIFont* font;
	
	int code;
	char italic;
	char bold;
	
	
	// the raw glyph is oversampled by FontManager.oversample times
	int oversample;
	int magnitude;
	
	// metrics for the raw glyph, in pixels
	uint8_t* rawGlyph;
	Vector2i rawGlyphSize; // size of the raw bitmap

	Vector rawBearing; // distance from the origin to the top left corner of the glyph
	float rawAdvance; // horizontal advance, in pixels
	
	// the sdf is smaller than the raw glyph
	
	// metrics for the sdf glyph, in pixels
	uint8_t* sdfGlyph;
	Vector2i sdfGlyphSize; // size of the sdf bitmap
	AABB2 sdfBounds; // bounding box of the non-empty data in the sdf bitmap, in pixels
	Vector2i sdfDataSize; // size of the non-empty sdf data in the bitmap
	
	Vector sdfBearing; // distance from the origin to the top left corner of the clipped sdf data
	float sdfAdvance; // horizontal advance, in pixels
	
	
	// final texture data
	struct charInfo charinfo;
} FontGen;


typedef struct FontManager {
	HashTable(GUIFont*) fonts;
	
	// SDF generation 
	VEC(FontGen*) gen;
	
	// SDF config
	int oversample;
	int magnitude;
	
	uint8_t* atlas;
	int atlasSize;
	
	
	// temp hacky stuff
	GUIFont* helv;
	
} FontManager;

void FontManager_createAtlas(FontManager* fm);

/*
The general idea is this:
The gui is held in a big tree. The tree is walked depth-first from the bottom up, resulting in
a mostly-sorted list. A sort is then run on z-index to ensure proper order. The list is then
rendered all at once through a single unified megashader using the geometry stage to expand
points into quads.

*/
typedef struct GUIManager {
	PCBuffer instVB;
	GLuint vao;
	
	int elementCount;
	int elementAlloc;
	GUIUnifiedVertex* elemBuffer;
	
	Vector2i screenSize;
	
	GUIObject* root;
	
	FontManager* fm;
	TextRes* font;
	
	// temp 
	GLuint atlasID;
	
} GUIManager;



void GUIManager_init(GUIManager* gm, int maxInstances);
GUIManager* GUIManager_alloc(int maxInstances);

void GUIManager_checkElemBuffer(GUIManager* gm);

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












void gui_Init();
void gui_Image_Init(char* file);

void gui_RenderAll(GameState* gs, PassFrameParams* pfp);

GUIObject* guiHitTest(GUIObject* go, Vector2 testPos);
void guiDelete(GUIObject* go);
void guiRender(GUIObject* go, GameState* gs, PassFrameParams* pfp);
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
