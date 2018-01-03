#ifndef __EACSMB_ui_image_h__
#define __EACSMB_ui_image_h__





typedef struct GUIImage {
	GUIHeader header;
	
	int texIndex;
	GLuint customTexID;
	
} GUIImage;





GUIImage* guiImageNew(Vector2 pos, Vector2 size, float zIndex, int texIndex);






#endif // __EACSMB_ui_image_h__
