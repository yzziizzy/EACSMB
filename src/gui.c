

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "window.h"
#include "game.h"
#include "texture.h"
#include "hash.h"
#include "log.h"

#include "utilities.h"


VEC(GUIObject*) gui_list; 
VEC(GUIObject*) gui_reap_queue; 


GUIObject* guiBaseHitTest(GUIObject* go, Vector2 testPos);


static void preFrame(PassFrameParams* pfp, GUIManager* gm);
static void draw(GUIManager* gm, GLuint progID, PassDrawParams* pdp);
static void postFrame(GUIManager* gm);


GUIManager* GUIManager_alloc(int maxInstances) {
	GUIManager* gm;
	pcalloc(gm);
	
	GUIManager_init(gm, maxInstances);
	
	return gm;
}


void GUIManager_init(GUIManager* gm, int maxInstances) {
	
	static VAOConfig vaoConfig[] = {
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // top, left, bottom, right
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // tlbr clipping planes
		{0, 4, GL_UNSIGNED_BYTE, 0, GL_FALSE}, // tex indices 1&2, tex fade, gui type
		{0, 4, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex offset 1&2
		{0, 4, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex size 1&2
		
		{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // fg color
		{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // bg color
		
		
		{0, 0, 0}
	};

	
	gm->vao = makeVAO(vaoConfig);
	glBindVertexArray(gm->vao);
	
	int stride = calcVAOStride(0, vaoConfig);

	PCBuffer_startInit(&gm->instVB, maxInstances * stride, GL_ARRAY_BUFFER);
	updateVAO(0, vaoConfig); 
	PCBuffer_finishInit(&gm->instVB);
	
	
	
	// ---- general properties ----
	
	gm->font = LoadSDFFont("Arial.sdf");
	if(!gm->font) {
		fprintf(stderr, "Failed to load font: %s\n", "Arial");
	}
	
	
}

static void preFrame(PassFrameParams* pfp, GUIManager* gm) {
	GUIUnifiedVertex* vmem = PCBuffer_beginWrite(&gm->instVB);
	if(!vmem) {
		printf("attempted to update invalid PCBuffer in GUIManager\n");
		return;
	}
	
	/* just a clipped box
	*vmem = (GUIUnifiedVertex){
		.pos = {50, 10, 900, 700},
		.clip = {150, 110, 800, 600},
		
		.texIndex1 = 0,
		.texIndex2 = 0,
		.texFade = .5,
		.guiType = 0, // window
		
		.texOffset1 = 0,
		.texOffset2 = 0,
		.texSize1 = 0,
		.texSize2 = 0,
		
		.fg = {255, 128, 64, 255},
		.bg = {64, 128, 255, 255},
	};
	*/
	
	float off = TextRes_charTexOffset(gm->font, 'A');
	float wid = TextRes_charWidth(gm->font, 'A');
	*vmem = (GUIUnifiedVertex){
		.pos = {50, 10, 900, 700},
		.clip = {150, 110, 800, 600},
		
		.texIndex1 = 0,
		.texIndex2 = 0,
		.texFade = .5,
		.guiType = 1, // text
		
// 		.texOffset1 = { off * 65535.0, 0},
		.texOffset1 = { off * 65535.0, 0},
		.texOffset2 = {0, 0},
// 		.texSize1 = { wid * 65535.0, 65535},
		.texSize1 = { wid *  65535.0, 65535},
		.texSize2 = {5000, 5000},
		
		.fg = {255, 128, 64, 255},
		.bg = {64, 128, 255, 255},
	};
	
	printf("FONT INFO: %f, %f\n", off, wid);
	
}

static void draw(GUIManager* gm, GLuint progID, PassDrawParams* pdp) {
	size_t offset;
	
// 	if(mdi->uniformSetup) {
// 		(*mdi->uniformSetup)(mdi->data, progID);
// 	}
	GLuint ts_ul = glGetUniformLocation(progID, "fontTex");
	
	glUniform1i(ts_ul, 28);
	glexit("text sampler uniform");
// 	glBindTexture(GL_TEXTURE_2D, arial->textureID);
// 	glBindTexture(GL_TEXTURE_2D, gt->font->textureID); // TODO check null ptr
// 	glexit("bind texture");
	
	// ------- draw --------
	
	glBindVertexArray(gm->vao);
	
	PCBuffer_bind(&gm->instVB);
	offset = PCBuffer_getOffset(&gm->instVB);
	
	
	glDrawArrays(GL_POINTS, offset / sizeof(GUIUnifiedVertex), 1);
	
	glexit("");
}



static void postFrame(GUIManager* gm) {
	PCBuffer_afterDraw(&gm->instVB);
}






RenderPass* GUIManager_CreateRenderPass(GUIManager* gm) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = GUIManager_CreateDrawable(gm);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* GUIManager_CreateDrawable(GUIManager* gm) {
	
	PassDrawable* pd;
	static ShaderProgram* prog = NULL;
	
	if(!prog) {
		prog = loadCombinedProgram("guiUnified");
		glexit("");
	}
	
	
	pd = Pass_allocDrawable("GUIManager");
	pd->data = gm;
	pd->preFrame = preFrame;
	pd->draw = (PassDrawFn)draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	return pd;;
}





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


void guiRender(GUIObject* go, GameState* gs, PassFrameParams* pfp) {
	if(go->h.hidden || go->h.deleted) return;
	
	if(go->h.vt->Render)
		go->h.vt->Render(go, gs, pfp);
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


void gui_RenderAll(GameState* gs, PassFrameParams* pfp) {
	int i;
	
	// TODO: replace with rendering tree once all data is unified
	
	for(i = 0; i < VEC_LEN(&gui_list); i++) {
		//guiTextRender(VEC_DATA(&gui_list)[i], gs);
		guiRender(VEC_DATA(&gui_list)[i], gs, pfp);
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

