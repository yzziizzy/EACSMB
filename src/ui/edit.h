#ifndef __EACSMB_ui_edit_h__
#define __EACSMB_ui_edit_h__



typedef struct GUIEdit {
	GUIHeader header;
	InputEventHandler* inputHandlers;
	
	
	VEC(char) text;
	
	float blinkRate;
	
	// offsets, text align
	
	GUIText bg;
	GUIText textControl;
	GUIWindow cursor; // just a thin window
	
} GUIEdit;



#endif // __EACSMB_ui_edit_h__
