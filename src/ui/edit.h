#ifndef __EACSMB_ui_edit_h__
#define __EACSMB_ui_edit_h__



// onchange
struct GUIEdit;
typedef void (*GUIEditOnChangeFn)(struct GUIEdit*, void*);


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
	
	GUIEditOnChangeFn onChange;
	void* onChangeData;
	
} GUIEdit;


GUIEdit* GUIEditNew(char* initialValue, Vector2 pos, Vector2 size);




typedef struct GUINumberControl {
	union{
		GUIHeader header;
		GUIEdit ed;
	};
	
	double val;
	
} GUINumberControl;


GUINumberControl* GUINumberControlNew(double initialValue, Vector2 pos, Vector2 size);




#endif // __EACSMB_ui_edit_h__
