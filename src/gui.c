

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "window.h"
#include "gui.h"
#include "hash.h"
#include "log.h"

#include "utilities.h"


VEC(GUIObject*) gui_list; 
VEC(GUIObject*) gui_reap_queue; 


GUIObject* guiBaseHitTest(GUIObject* go, Vector2 testPos);




/*
	//arial = LoadFont("Arial", 32, NULL);
	glerr("clearing before text program load");
	
	printf("text prog %d\n", textProg->id);
	unsigned int colors[] = {
		0xFF0000FF, 2,
		0x00FF00FF, 4,
		0x0000FFFF, INT_MAX
	};
	
	//strRI = prepareText(arial, "FPS: --", -1, colors);
	strRI = prepareText(arialsdf, "FPS: --", -1, colors);
	
	
		snprintf(frameCounterBuf, 128, "dtime:  %.2fms", sdtime);
// 		snprintf(frameCounterBuf, 128, "dtime:  %.2fms", gs->perfTimes.draw * 1000);
		
		//printf("--->%s\n", frameCounterBuf);
		
		updateText(strRI, frameCounterBuf, -1, fpsColors);
		*/
	



void gui_Window_Init();
void gui_Text_Init();

void gui_Init() {
	
	VEC_INIT(&gui_list);
	VEC_INIT(&gui_reap_queue);
	
	gui_Window_Init();
	gui_Text_Init();
	
	gui_Image_Init("assets/config/guiIcons.json");
}


// add root objects to the root list, record the parent otherwise
void guiRegisterObject_(GUIHeader* o, GUIHeader* parent) {
	
	o->parent = parent;
	
	if(!parent) {
		VEC_PUSH(&gui_list, o);
	}
	else {
		VEC_PUSH(&parent->children, o);
	}
}


void guiRender(GUIObject* go, GameState* gs) {
	if(go->h.hidden || go->h.deleted) return;
	
	if(go->h.vt->Render)
		go->h.vt->Render(go, gs);
} 

void guiDelete(GUIObject* go) {
	go->h.deleted = 1;
	
	VEC_PUSH(&gui_reap_queue, go);
	
	for(int i = 0; i < VEC_LEN(&go->h.children); i++) {
		//guiTextRender(VEC_DATA(&gui_list)[i], gs);
		guiDelete(VEC_ITEM(&go->h.children, i));
	}
	

	
	if(go->h.vt->Delete)
		go->h.vt->Delete(go);
} 

void guiReap(GUIObject* go) {
	if(!go->h.deleted) {
		Log("Attempting to reap non-deleted GUI Object");
		return;
	}
	
	// remove from parent
	guiRemoveChild(go->h.parent, go);
	
	if(go->h.vt->Reap)
		return go->h.vt->Reap(go);
	
}

void guiResize(GUIHeader* gh, Vector2 newSz) {
	if(gh->deleted) return;
	
	if(gh->vt->Resize) {
		gh->vt->Resize((GUIObject*)gh, newSz);
	}
	else {
		gh->size = newSz;
	}
	
	// parents need to resize their children
} 

// NOT SMT SAFE; NO LOCKS
int guiRemoveChild(GUIObject* parent, GUIObject* child) {
	
	if(!parent || !child) return 0;
	
	int i = VEC_FIND(&parent->h.children, child);
	if(i < 0) return 1;
	
	VEC_RM(&parent->h.children, i);
	
	return 0;
}


GUIObject* guiHitTest(GUIObject* go, Vector2 testPos) {

	if(go->h.vt->HitTest)
		return go->h.vt->HitTest(go, testPos);
	
	return guiBaseHitTest(go, testPos);
} 

void guiHeaderInit(GUIHeader* gh) {
	VEC_INIT(&gh->children);
}



GUIObject* guiBaseHitTest(GUIObject* go, Vector2 testPos) {
	GUIHeader* h = &go->h; 

	int in = boxContainsPoint2(&h->hitbox, &testPos);
	if(!in) return NULL;
	
	int i;
	for(i = 0; i < VEC_LEN(&h->children); i++) {
		GUIObject* kid = guiHitTest(VEC_ITEM(&h->children, i), testPos);
		if(kid) return kid;
	}
	
	return go;
}


void gui_RenderAll(GameState* gs) {
	int i;
	
	// TODO: replace with rendering tree once all data is unified
	
	for(i = 0; i < VEC_LEN(&gui_list); i++) {
		//guiTextRender(VEC_DATA(&gui_list)[i], gs);
		guiRender(VEC_DATA(&gui_list)[i], gs);
	}
}



void guiTriggerClick(GUIEvent* e) {
	GUIObject* c = e->currentTarget;
	
	if(!c) return;
	
	if(c->h.onClick)
		c->h.onClick(e);
	
	e->currentTarget = c->h.parent;
	
	guiTriggerClick(e);
}


void guiSetClientSize(GUIObject* go, Vector2 cSize) {
	if(go->h.vt->SetClientSize)
		return go->h.vt->SetClientSize(go, cSize);
}

Vector2 guiGetClientSize(GUIObject* go) {
	if(go->h.vt->GetClientSize)
		return go->h.vt->GetClientSize(go);
	
	return (Vector2){-1,-1};
} 

Vector2 guiRecalcClientSize(GUIObject* go) {
	if(go->h.vt->RecalcClientSize)
		return go->h.vt->RecalcClientSize(go);
	
	return (Vector2){-1,-1};
} 


void guiAddClient(GUIObject* parent, GUIObject* child) {
	if(parent->h.vt->AddClient)
		parent->h.vt->AddClient(parent, child);
} 

void guiRemoveClient(GUIObject* parent, GUIObject* child) {
	if(parent->h.vt->RemoveClient)
		parent->h.vt->RemoveClient(parent, child);
} 

