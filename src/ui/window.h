#ifndef __EACSMB_ui_window_h__
#define __EACSMB_ui_window_h__



typedef struct GUIWindow {
	GUIHeader header;
	
// 	Vector2 size;
	Vector2 clientSize;
	struct {
		float top, left, bottom, right;
	} padding;
	
// 	uint32_t color;
	Vector color;
	Vector4 borderColor;
	float borderWidth;
	float fadeWidth;
	
	float zindex;
	
	VEC(union GUIObject*) clients;
	
	
} GUIWindow;




GUIWindow* guiWindowNew(Vector2 pos, Vector2 size, float zIndex);




#endif // __EACSMB_ui_window_h__
