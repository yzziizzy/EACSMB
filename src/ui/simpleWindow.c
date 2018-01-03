
#include "stdlib.h"
#include "string.h"



#include "../gui.h"





static int closeClick(GUIEvent* e) {
	
	GUISimpleWindow* sw;
	

	sw = (GUISimpleWindow*)e->currentTarget;
	
	if(e->originalTarget == sw->closebutton) {
		
		sw->header.hidden = 1;
		guiDelete(sw);
	}
	
	//TODO no further bubbling
	return 0;
}



void guiSimpleWindowRender(GUISimpleWindow* sw, GameState* gs) {
	
	guiRender(sw->bg, gs);
	guiRender(sw->titlebar, gs);
	guiRender(sw->closebutton, gs);
}

void guiSimpleWindowDelete(GUISimpleWindow* sw) {
	
	
	
	
}



GUISimpleWindow* guiSimpleWindowNew(Vector2 pos, Vector2 size, float zIndex) {
	
	GUISimpleWindow* sw;
	
	float tbh = .03; // titleBarHeight
	
	static struct gui_vtbl static_vt = {
		.Render = guiSimpleWindowRender,
		.Delete = guiSimpleWindowDelete
	};
	
	
	sw = calloc(1, sizeof(*sw));
	CHECK_OOM(sw);
	
	guiHeaderInit(&sw->header);
	sw->header.vt = &static_vt;
	
	sw->header.hitbox.min.x = pos.x;
	sw->header.hitbox.min.y = pos.y;
	sw->header.hitbox.max.x = pos.x + size.x;
	sw->header.hitbox.max.y = pos.y + size.y;
	
	sw->bg = guiWindowNew(pos, size, zIndex);
	sw->bg->color = (Vector){0.1, 0.9, 0.1};
	guiRegisterObject(sw->bg, &sw->header);
	
	sw->titlebar = guiWindowNew(
		(Vector2){pos.x, pos.y}, 
		(Vector2){size.x, tbh}, 
		zIndex + .0001
	);
	sw->titlebar->color = (Vector){0.9, 0.1, .9};
	guiRegisterObject(sw->titlebar, &sw->bg->header);
	
	sw->closebutton = guiWindowNew(
		(Vector2){pos.x + size.x - tbh, pos.y + tbh * .05}, 
		(Vector2){tbh * 0.9, tbh * 0.9},
		zIndex + .0002
	);
	sw->closebutton->color = (Vector){0.9, 0.1, 0.1};
	guiRegisterObject(sw->closebutton, &sw->titlebar->header);
	
	
	sw->header.onClick = closeClick;
	
	
	
	return sw;
}








