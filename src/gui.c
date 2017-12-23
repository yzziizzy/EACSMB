

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "window.h"
#include "gui.h"
#include "hash.h"

#include "utilities.h"


VEC(GUIObject*) gui_list; 

HashTable* font_cache; // TextRes*

ShaderProgram* textProg;
ShaderProgram* windowProg;

static GLuint vaoWindow;
static GLuint vboWindow;


GUIObject* guiBaseHitTest(GUIObject* go, Vector2 testPos);


void guiWindowRender(GUIWindow* gw, GameState* gs);
void guiWindowDelete(GUIWindow* gw);

void guiTextRender(GUIText* gt, GameState* gs);
void guiTextDelete(GUIText* gt);



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
	

// handles caching in ram and on disk
static TextRes* getFont(char* name) {
	TextRes* tr;
	char* filename;
	
	// try the cache first
	if(!HT_get(font_cache, name, &tr)) {
		return tr;
	}
	
	// not in the cache
	filename = malloc(strlen(name) + strlen(".sdf") + 1);
	filename[0] = 0;
	strcpy(filename, name);
	strcat(filename, ".sdf");
	
	// try to load a pre-generated sdf file
	tr = LoadSDFFont(filename);
	if(tr == NULL) {
		tr = GenerateSDFFont(name, 16, NULL);
		SaveSDFFont(filename, tr);
	}
	
	HT_set(font_cache, name, tr);
	
	free(filename);
	
	return tr;
}


void gui_Init() {
	
	//init font cache
	font_cache = HT_create(2);
	
	textProg = loadCombinedProgram("guiTextSDF");
	windowProg = loadCombinedProgram("guiWindow");
	
	
	// window VAO
	VAOConfig opts[] = {
		// per vertex
		{2, GL_FLOAT}, // position
		
		{0, 0}
	};
	
	vaoWindow = makeVAO(opts);
	
	
	glBindVertexArray(vaoWindow);
	
	glGenBuffers(1, &vboWindow);
	glBindBuffer(GL_ARRAY_BUFFER, vboWindow);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*4, 0);
	
	Vector2 data[] = {
		{0,0},
		{0,1},
		{1,0},
		{1,1}
	};
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("");
	
	
	
	
}


void guiRender(GUIObject* go, GameState* gs) {
	if(go->h.vt->Render)
		go->h.vt->Render(go, gs);
} 

void guiDelete(GUIObject* go) {
	go->h.deleted = 1;
	
	if(go->h.vt->Delete)
		go->h.vt->Delete(go);
} 

GUIObject* guiHitTest(GUIObject* go, Vector2 testPos) {
	if(go->h.vt->Render)
		return go->h.vt->HitTest(go, testPos);
	
	return guiBaseHitTest(go, testPos);
} 


GUIText* guiTextNew(char* str, Vector* pos, float size, char* fontname) {
	
	static struct gui_vtbl static_vt = {
		.Render = guiTextRender,
		.Delete = guiTextDelete,
	};
	
	
	
	
	GUIText* gt;
	
	unsigned int colors[] = {
		0x88FF88FF, INT_MAX
	};
	
	gt = calloc(1, sizeof(*gt));
	CHECK_OOM(gt);
	
	gt->header.vt = &static_vt; 
	
	if(pos) vCopy(pos, &gt->pos);
	gt->size = size;
	
	gt->font = getFont(fontname);
	if(!gt->font) {
		printf(stderr, "Failed to load font: %s\n", fontname);
	}
	
	if(str) {
		gt->current = strdup(str);
		gt->strRI = prepareText(gt->font, str, -1, colors);
	}
	
	
	VEC_PUSH(&gui_list, gt);
	
	return gt;
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


void guiTextSetValue(GUIText* gt, char* newval) {
	unsigned int colors[] = {
		0x88FF88FF, INT_MAX
	};
	
	if(0 != strcmp(newval, gt->current)) {
		if(gt->current) free(gt->current);
		gt->current = strdup(newval);
	
		updateText(gt->strRI, newval, -1, colors);
	}
}


void gui_RenderAll(GameState* gs) {
	int i;
	
	for(i = 0; i < VEC_LEN(&gui_list); i++) {
		
		//guiTextRender(VEC_DATA(&gui_list)[i], gs);
		guiRender(VEC_DATA(&gui_list)[i], gs);
	}
}


void guiTextRender(GUIText* gt, GameState* gs) {
	
	Matrix textProj;
	
	MatrixStack textModel;
	
	msAlloc(3, &textModel);
	
	glUseProgram(textProg->id);
	
	// text stuff
	textProj = IDENT_MATRIX;
	
	mOrtho(0, gs->screen.aspect, 0, 1, -1, 100, &textProj);
	//mScale3f(.5,.5,.5, &textProj);
	
	msIdent(&textModel);
	msScale3f(gt->size * .01, gt->size* .01, gt->size * .01, &textModel);
	msTransv(&gt->pos, &textModel);

	GLuint tp_ul = glGetUniformLocation(textProg->id, "mProj");
	GLuint tm_ul = glGetUniformLocation(textProg->id, "mModel");
	GLuint ts_ul = glGetUniformLocation(textProg->id, "fontTex");
	
	glUniformMatrix4fv(tp_ul, 1, GL_FALSE, textProj.m);
	glUniformMatrix4fv(tm_ul, 1, GL_FALSE, msGetTop(&textModel)->m);
	glexit("text matrix uniforms");

	glDisable(GL_CULL_FACE);
	
	glActiveTexture(GL_TEXTURE0);
	glexit("active texture");

	glUniform1i(ts_ul, 0);
	glexit("text sampler uniform");
// 	glBindTexture(GL_TEXTURE_2D, arial->textureID);
	glBindTexture(GL_TEXTURE_2D, gt->font->textureID); // TODO check null ptr
	glexit("bind texture");
	
	
	glBindVertexArray(gt->strRI->vao);
	glexit("text vao bind");
	
	glBindBuffer(GL_ARRAY_BUFFER, gt->strRI->vbo);
	glexit("text vbo bind");
	glDrawArrays(GL_TRIANGLES, 0, gt->strRI->vertexCnt);
	glexit("text drawing");
}

void guiTextDelete(GUIText* gt) {
	printf("NIH guiTextDelete " __FILE__ ":%d\n", __LINE__);
}





GUIWindow* guiWindowNew(Vector* pos, Vector2* size) {
	
	GUIWindow* gw;
	
	static struct gui_vtbl static_vt = {
		.Render = guiWindowRender,
		.Delete = guiWindowDelete
	};
	
	
	gw = calloc(1, sizeof(*gw));
	CHECK_OOM(gw);
	
	gw->header.vt = & static_vt;
	
	//if(pos) vCopy(pos, &gw->header.pos);
//	gt->size = size;
	
	unsigned int colors[] = {
		0x88FF88FF, INT_MAX
	};
	
	
	VEC_PUSH(&gui_list, gw);
	
	return gw;
}


void guiWindowRender(GUIWindow* gw, GameState* gs) {
	
	Matrix proj = IDENT_MATRIX;
	
	static GLuint proj_ul;
	static GLuint tlx_tly_w_h_ul;
	static GLuint z_alpha__ul;
	static GLuint color_ul;
	
	if(!proj_ul) proj_ul = glGetUniformLocation(windowProg->id, "mProj");
	if(!tlx_tly_w_h_ul) tlx_tly_w_h_ul = glGetUniformLocation(windowProg->id, "tlx_tly_w_h");
	if(!z_alpha__ul) z_alpha__ul = glGetUniformLocation(windowProg->id, "z_alpha_");
	if(!color_ul) color_ul = glGetUniformLocation(windowProg->id, "color");
	
	
	
	
	mOrtho(0, 1, 0, 1, 0, 1, &proj);
	
	
	glUseProgram(windowProg->id);
	glexit("");
	
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj.m);
	glUniform4f(tlx_tly_w_h_ul, .1, .1, .8, .8);
	glUniform4f(z_alpha__ul, -.1, .5, 0, 0); // BUG z is a big messed up; -.1 works but .1 doesn't.
	glUniform3f(color_ul, .1, .5, .1); // BUG z is a big messed up; -.1 works but .1 doesn't.

	glBindVertexArray(vaoWindow);
	glBindBuffer(GL_ARRAY_BUFFER, vboWindow);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("");
}

void guiWindowDelete(GUIWindow* gw) {
	
}






