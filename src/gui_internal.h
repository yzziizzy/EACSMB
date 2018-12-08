#ifndef __EACSMB_gui_internal_h__
#define __EACSMB_gui_internal_h__

// this file is for gui element implementations, not for general outside usage


typedef struct GUIRenderParams {
	Vector2 offset; // from the top left
	AABB2 clip;
} GUIRenderParams;



void gui_headerInit(GUIHeader* gh, GUIManager* gm, struct gui_vtbl* vt); 

GUIUnifiedVertex* GUIManager_checkElemBuffer(GUIManager* gm, int count);
GUIUnifiedVertex* GUIManager_reserveElements(GUIManager* gm, int count);

void GUIHeader_render(GUIHeader* gh, AABB2* clip, PassFrameParams* pfp);
void GUIHeader_renderChildren(GUIHeader* gh, AABB2* clip, PassFrameParams* pfp);



#endif // __EACSMB_gui_internal_h__
