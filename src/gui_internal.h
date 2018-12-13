#ifndef __EACSMB_gui_internal_h__
#define __EACSMB_gui_internal_h__

// this file is for gui element implementations, not for general outside usage


typedef struct GUIRenderParams {
	Vector2 offset; // from the top left
	Vector2 size; // of the parent's client area
	AABB2 clip;
	float baseZ;
} GUIRenderParams;



void gui_headerInit(GUIHeader* gh, GUIManager* gm, struct gui_vtbl* vt); 
void gui_defaultUpdatePos(GUIObject* go, GUIRenderParams* grp, PassFrameParams* pfp);
Vector2 cui_calcPosGrav(GUIHeader* h, GUIRenderParams* grp);
GUIObject* gui_defaultHitTest(GUIHeader* h, Vector2 testPos);
Vector2 cui_parent2ChildGrav(GUIHeader* child, GUIHeader* parent, Vector2 pt);



GUIUnifiedVertex* GUIManager_checkElemBuffer(GUIManager* gm, int count);
GUIUnifiedVertex* GUIManager_reserveElements(GUIManager* gm, int count);

void GUIHeader_render(GUIHeader* gh, PassFrameParams* pfp);
void GUIHeader_renderChildren(GUIHeader* gh, PassFrameParams* pfp);

void GUIHeader_updatePos(GUIObject* go, GUIRenderParams* grp, PassFrameParams* pfp);

#endif // __EACSMB_gui_internal_h__
