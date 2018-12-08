#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../gui.h"
#include "../gui_internal.h"
#include "../utilities.h"





void guiWindowDelete(GUIWindow* gw);
// Vector2 guiWindowGetClientSize(GUIObject* go);
// void guiWindowSetClientSize(GUIObject* go, Vector2 cSize);
// Vector2 guiWindowRecalcClientSize(GUIObject* go);
void guiWindowAddClient(GUIObject* parent, GUIObject* child);
void guiWindowRemoveClient(GUIObject* parent, GUIObject* child);


static void render(GUIWindow* gw, AABB2* clip, PassFrameParams* pfp);




GUIWindow* GUIWindow_new(GUIManager* gm) {
	
	GUIWindow* gw;
	
	static struct gui_vtbl static_vt = {
		.Render = render,
		.Delete = guiWindowDelete,
// 		.GetClientSize = guiWindowGetClientSize,
// 		.SetClientSize = guiWindowSetClientSize,
// 		.RecalcClientSize = guiWindowRecalcClientSize,
		.AddClient = guiWindowAddClient,
		.RemoveClient = guiWindowRemoveClient,
	};
	
	
	pcalloc(gw);
	gui_headerInit(&gw->header, gm, &static_vt);
/*	
	gw->header.topleft = pos;
	gw->header.size = size;
	gw->header.z = zIndex;
	
	gw->header.hitbox.min.x = pos.x;
	gw->header.hitbox.min.y = pos.y;
	gw->header.hitbox.max.x = pos.x + size.x;
	gw->header.hitbox.max.y = pos.y + size.y;
	*/

	//if(pos) vCopy(pos, &gw->header.pos);
//	gt->size = size;
	
// 	unsigned int colors[] = {
// 		0x88FF88FF, INT_MAX
// 	};
	
	//VEC_PUSH(&gui_list, gw);
	
	return gw;
}


static void render(GUIWindow* gw, AABB2* clip, PassFrameParams* pfp) {
	
	if(gw->header.hidden || gw->header.deleted) return;
	
	// TODO: clip calculations
	
	GUIUnifiedVertex* v = GUIManager_reserveElements(gw->header.gm, 1);
	
	*v = (GUIUnifiedVertex){
		.pos = {gw->header.topleft.x, gw->header.topleft.y,
			gw->header.topleft.x + gw->header.size.x, gw->header.topleft.y + gw->header.size.y},
		.clip = {150, 110, 800, 600},
		
		.texIndex1 = 0,
		.texIndex2 = 0,
		.texFade = .5,
		.guiType = 0, // window
		
		.texOffset1 = 0,
		.texOffset2 = 0,
		.texSize1 = 0,
		.texSize2 = 0,
		
		.fg = {255, 128, 64, 255}, // TODO: border color
		.bg = {64, 128, 255, 255}, // TODO: color
		
		.z = gw->header.z,
		.alpha = gw->header.alpha,
	};
	
	
	GUIHeader_renderChildren(&gw->header, clip, pfp);
}

void guiWindowDelete(GUIWindow* gw) {
	
}










Vector2 GUIWindow_getClientSize(GUIObject* go) {
	GUIWindow* w = &go->window;
	return w->clientSize;
}

void GUIWindow_setClientSize(GUIObject* go, Vector2 cSize) {
	GUIHeader* h = &go->header;
	GUIWindow* w = &go->window;
	w->clientSize = cSize;
// 	h->size.x = cSize.x + w->padding.left + w->padding.right;
// 	h->size.y = cSize.y + w->padding.bottom + w->padding.top;
	
	// TODO: trigger resize event
}

// recalculate client size based on client children sizes and positions
Vector2 guiWindowRecalcClientSize(GUIObject* go) {
	GUIWindow* w = &go->window;
	int i;
	Vector2 max = {0, 0}; 
	
// 	for(i = 0; i < VEC_LEN(&w->clients); i++) {
// 		GUIHeader* h = (GUIHeader*)VEC_ITEM(&w->clients, i);
// 		
// 		max.x = fmax(max.x, h->topleft.x + h->size.x);
// 		max.y = fmax(max.y, h->topleft.y + h->size.y);
// 	}
	
	return max;
}

void guiWindowAddClient(GUIObject* parent, GUIObject* child) {
	GUIWindow* w = &parent->window;
	
// 	int i = VEC_FIND(&w->clients, child);
// 	if(i < 0) VEC_PUSH(&w->clients, child);
};

void guiWindowRemoveClient(GUIObject* parent, GUIObject* child) {
	GUIWindow* w = &parent->window;
	
// 	int i = VEC_FIND(&w->clients, child);
// 	if(i <= 0) VEC_RM(&w->clients, i);
};








