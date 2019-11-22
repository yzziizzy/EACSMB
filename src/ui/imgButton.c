
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "../gui.h"
#include "../gui_internal.h"



static void render(GUIImageButton* w, GameState* gs, PassFrameParams* pfp) {
	
	GUIHeader_renderChildren(&w->header, pfp);
}

static void delete(GUIImageButton* ib) {

}
static void resize(GUIImageButton* go, Vector2 newSz) {
	guiResize(&go->img->header, newSz);
}

GUIImageButton* GUIImageButton_New(GUIManager* gm, Vector2 size, char* imgName) {
	
	GUIImageButton* w;
	
	
	static struct gui_vtbl static_vt = {
		.Render = (void*)render,
		.Resize = (void*)resize,
	};
	
	static InputEventHandler input_vt = {
		//.keyText = recieveText,
		//.keyDown = keyDown,
	};
	
	w = calloc(1, sizeof(*w));
	CHECK_OOM(w);
	
	gui_headerInit(&w->header, gm, &static_vt);
	w->header.input_vt = &input_vt;
	
	w->header.size = size;
// 	w->header.z = zIndex;
	
	w->img = GUIImage_new(gm, imgName);
	GUIRegisterObject(w->img, &w->header);
	
	
	
	return w;
}









