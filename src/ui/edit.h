#ifndef __EACSMB_ui_edit_h__
#define __EACSMB_ui_edit_h__





typedef struct GUIEdit {
	GUIHeader header;
	InputEventHandler* inputHandlers;
	
	
	char* buf;
	int buflen;
	int textlen;
	int cursorpos; // in characters
	
// 	float fontSize;
	float blinkRate;
	float cursorOffset; // in screen units
	
	// offsets, text align
	
	GUIWindow* bg;
	GUIText* textControl;
	GUIWindow* cursor; // just a thin window
	
} GUIEdit;


GUIEdit* GUIEditNew(char* initialValue, Vector2 pos, Vector2 size);


#endif // __EACSMB_ui_edit_h__
