#ifndef __EACSMB_ui_image_h__
#define __EACSMB_ui_image_h__



#include "../pass.h"




typedef struct GUIImage {
	GUIHeader header;
	
	int texIndex;
	GLuint customTexID;
	
} GUIImage;





GUIImage* guiImageNew(Vector2 pos, Vector2 size, float zIndex, int texIndex);




typedef struct GUIRenderTarget {
	GUIHeader header;
	
	GLuint texID;
	RenderPipeline* rpl;
	
	
} GUIRenderTarget;

GUIRenderTarget* guiRenderTargetNew(Vector2 pos, Vector2 size, RenderPipeline* rpl);


// also initialize GUIRenderTarget data
void gui_Image_Init(char* file);

#endif // __EACSMB_ui_image_h__
