#ifndef __EACSMB_ui_image_h__
#define __EACSMB_ui_image_h__





typedef struct GUIImage {
	GUIHeader header;
	
	GUIWindow* bg;
	GUIWindow* titlebar;
	GUIWindow* closebutton;
	
	GUIText* titleText;
	
	char* title;
	
} GUIImage;





GUIImage* guiImageNew(Vector2 pos, Vector2 size, float zIndex);






#endif // __EACSMB_ui_image_h__
