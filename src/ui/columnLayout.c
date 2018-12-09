
#include "stdlib.h"
#include "string.h"




#include "../gui.h"
#include "../gui_internal.h"





static void render(GUIColumnLayout* cl, GUIRenderParams* grp, PassFrameParams* pfp);





GUIColumnLayout* GUIColumnLayout_new(GUIManager* gm, Vector2 pos, float spacing, float zIndex) {
	
	GUIColumnLayout* w;
	
	static struct gui_vtbl static_vt = {
		.Render = render,
	};
	
	
	w = calloc(1, sizeof(*w));
	CHECK_OOM(w);
	
	gui_headerInit(&w->header, gm, &static_vt);
	
	w->spacing = spacing;
	
	// empty layout has no size
	w->header.hitbox.min.x = 0;
	w->header.hitbox.min.y = 0;
	w->header.hitbox.max.x = 0;
	w->header.hitbox.max.y = 0;
	
	w->header.topleft = pos;
	
	return w;
}



// TODO: re-sort







static void render(GUIColumnLayout* cl, GUIRenderParams* grp, PassFrameParams* pfp) {
	
// 	guiRender(ed->bg, gs, pfp);
// 	guiRender(ed->textControl, gs, pfp);
	
	// TODO: fix fencepost issue with spacing
	// TODO: figure out better phase for size calculation
	float total_h = 0.0;
	float max_w = 0.0;
	VEC_EACH(&cl->header.children, i, child) { 
		total_h += cl->spacing + child->h.size.y;
		max_w = fmax(max_w, child->h.size.x);
	}
	
	cl->header.size.y = total_h;
	cl->header.size.x = max_w;
	
	Vector2 tl = cui_calcPosGrav(&cl->header, grp);
	
	// columnlayout works by spoofing the renderparams supplied to each child
	total_h = 0.0;
	VEC_EACH(&cl->header.children, i, child) { 
		total_h += cl->spacing;
		
		GUIRenderParams grp2 = {
			.clip = grp->clip,
			.size = child->h.size,//{child, total_h},
			.offset = {
				.x = tl.x,
				.y = tl.y + total_h 
			}
		};
		
/*		
		child->h.topleft.x = tl.x;
		child->h.topleft.y = tl.y + total_h;*/
		
		GUIHeader_render(child, &grp2, pfp);
		
		total_h += child->h.size.y;
	}
	
// 	int n = VEC_LEN(&cl->header);
	
	
	
}

void guiColumnLayoutDelete(GUIColumnLayout* o) {
	
}










