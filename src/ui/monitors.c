#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../gui.h"
#include "../gui_internal.h"
#include "../utilities.h"





static void render(GUIFloatMonitor* gfm, PassFrameParams* pfp);

static void updateText(GUIFloatMonitor* gfm);



GUIFloatMonitor* GUIFloatMonitor_new(GUIManager* gm, char* format, float* target) {
	GUIFloatMonitor* gfm;
	
	static struct gui_vtbl static_vt = {
		.Render = render,
	};
	
	
	pcalloc(gfm);
	gui_headerInit(&gfm->header, gm, &static_vt);
	
	gfm->target = target;
	gfm->format = format;
	gfm->bufferLen = 0;
	
	
	gfm->text = GUIText_new(gm, "", "Arial", 3.0f);
	updateText(gfm);
	
	GUIRegisterObject(gfm->text, gfm);
	
	return gfm;
}

/*
static void updatePos(GUIWindow* gw, GUIRenderParams* grp, PassFrameParams* pfp) {
	Vector2 tl = cui_calcPosGrav(&gw->header, grp);
}*/

static void render(GUIFloatMonitor* gfm, PassFrameParams* pfp) {
	
	if(gfm->header.hidden || gfm->header.deleted) return;
	
	updateText(gfm);
	
	GUIHeader_renderChildren(&gfm->header, pfp);
}




static void updateText(GUIFloatMonitor* gfm) {
	
	float val = *gfm->target;
	
	size_t len = snprintf(NULL, 0, gfm->format, val) + 1; 
	
	if(gfm->bufferLen < len) {
		gfm->bufferLen = len + 32;
		gfm->buffer = realloc(gfm->buffer, gfm->bufferLen);
	}
	
	snprintf(gfm->buffer, gfm->bufferLen, gfm->format, val);
	
	GUIText_setString(gfm->text, gfm->buffer);
}




