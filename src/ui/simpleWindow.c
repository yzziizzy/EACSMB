
#include "stdlib.h"
#include "string.h"



#include "simpleWindow.h"





static int closeClick(GUIEvent* e) {
	
	GUISimpleWindow* sw;
	
	sw = (GUISimpleWindow*)e->currentTarget;
	
	if(e->originalTarget == sw->closebutton) {
		printf("close button clicked\n");
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
	
	float titleBarHeight = .01;
	
	static struct gui_vtbl static_vt = {
		.Render = guiSimpleWindowRender,
		.Delete = guiSimpleWindowDelete
	};
	
	
	sw = calloc(1, sizeof(*sw));
	CHECK_OOM(sw);
	
	guiHeaderInit(&sw->header);
	sw->header.vt = &static_vt;
	
	
	sw->bg = guiWindowNew(pos, size, zIndex);
	sw->bg->color = (Vector){0.1, 0.9, 0.1};
	guiRegisterObject(sw->bg, sw);
	
	sw->titlebar = guiWindowNew((Vector2){pos.x, pos.y}, (Vector2){size.x, titleBarHeight}, zIndex + .0001);
	sw->titlebar->color = (Vector){0.9, 0.1, .9};
	guiRegisterObject(sw->titlebar, sw);
	
	sw->closebutton = guiWindowNew((Vector2){pos.x + size.x - .01, pos.y + .001}, (Vector2){.009, .009}, zIndex + .0002);
	sw->closebutton->color = (Vector){0.9, 0.1, 0.1};
	guiRegisterObject(sw->closebutton, sw);
	
	
	sw->header.onClick = closeClick;
	
	
	
	return sw;
}








