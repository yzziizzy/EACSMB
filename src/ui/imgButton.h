#ifndef __EACSMB_ui_imgButton_h__
#define __EACSMB_ui_imgButton_h__



typedef struct GUIImageButton {
	GUIHeader header;
	
// 	int imgIndex;
	
	//GUIWindow* bg;
	GUIImage* img;
	
} GUIImageButton;


GUIImageButton* GUIImageButton_New(GUIManager* gm, Vector2 size, char* imgName);




#endif // __EACSMB_ui_imgButton_h__
