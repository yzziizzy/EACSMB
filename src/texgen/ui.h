#ifndef __EACSMB_texgen_ui_h__
#define __EACSMB_texgen_ui_h__


#include "core.h"
#include "../gui.h"
#include "../gui_internal.h"


typedef struct GUITexBuilderControl {
	GUIHeader header;
	
	GUIWindow* bg;
	GUIImage* im;
	
	GUIGridLayout* controls;
	
	GUIText* ctl_selectedOp;
	
	GUITreeControl* tree;
	
	GUIStructAdjuster* sa;
	
	TexGenContext* tg;
	TexGenOp* op;
	
	InputEventHandler* inputHandlers;
	
	GLuint texID;
	
	// -------
	
	int selectedOpIndex;
	
	GUIDebugAdjuster* da;
	
	
} GUITexBuilderControl;






GUITexBuilderControl* guiTexBuilderControlNew(GUIManager* gm, Vector2 pos, Vector2 size, int zIndex);



extern GUISA_Field* tgsa_fields[];


#endif // __EACSMB_texgen_ui_h__
