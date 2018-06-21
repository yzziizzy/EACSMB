#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../gui.h"
#include "../utilities.h"


ShaderProgram* windowProg;

static GLuint vaoWindow;
static GLuint vboWindow;



void gui_Window_Init() {

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

void guiWindowRender(GUIWindow* gw, GameState* gs, PassFrameParams* pfp);
void guiWindowDelete(GUIWindow* gw);
Vector2 guiWindowGetClientSize(GUIObject* go);
void guiWindowSetClientSize(GUIObject* go, Vector2 cSize);
Vector2 guiWindowRecalcClientSize(GUIObject* go);
void guiWindowAddClient(GUIObject* parent, GUIObject* child);
void guiWindowRemoveClient(GUIObject* parent, GUIObject* child);







GUIWindow* guiWindowNew(Vector2 pos, Vector2 size, float zIndex) {
	
	GUIWindow* gw;
	
	static struct gui_vtbl static_vt = {
		.Render = guiWindowRender,
		.Delete = guiWindowDelete,
		.GetClientSize = guiWindowGetClientSize,
		.SetClientSize = guiWindowSetClientSize,
		.RecalcClientSize = guiWindowRecalcClientSize,
		.AddClient = guiWindowAddClient,
		.RemoveClient = guiWindowRemoveClient,
	};
	
	
	gw = calloc(1, sizeof(*gw));
	CHECK_OOM(gw);
	
	guiHeaderInit(&gw->header);
	gw->header.vt = &static_vt;
	
	gw->header.topleft = pos;
	gw->header.size = size;
	gw->header.z = zIndex;
	
	gw->header.hitbox.min.x = pos.x;
	gw->header.hitbox.min.y = pos.y;
	gw->header.hitbox.max.x = pos.x + size.x;
	gw->header.hitbox.max.y = pos.y + size.y;
	
	//if(pos) vCopy(pos, &gw->header.pos);
//	gt->size = size;
	
// 	unsigned int colors[] = {
// 		0x88FF88FF, INT_MAX
// 	};
	
	gw->borderWidth = 0.05;
	gw->fadeWidth = 0.02;
	
	//VEC_PUSH(&gui_list, gw);
	
	return gw;
}


void guiWindowRender(GUIWindow* gw, GameState* gs, PassFrameParams* pfp) {
	
	Matrix proj = IDENT_MATRIX;
	
	static GLuint proj_ul;
	static GLuint tlx_tly_w_h_ul;
	static GLuint z_alpha_borderWidth_ul;
	static GLuint color_ul;
	static GLuint border_ul;
	
	if(!proj_ul) proj_ul = glGetUniformLocation(windowProg->id, "mProj");
	if(!tlx_tly_w_h_ul) tlx_tly_w_h_ul = glGetUniformLocation(windowProg->id, "tlx_tly_w_h");
	if(!z_alpha_borderWidth_ul) z_alpha_borderWidth_ul = glGetUniformLocation(windowProg->id, "z_alpha_borderWidth_fadeWidth");
	if(!color_ul) color_ul = glGetUniformLocation(windowProg->id, "color");
	if(!border_ul) border_ul = glGetUniformLocation(windowProg->id, "border");
	
	
	
	
	mOrtho(0, 1, 0, 1, 0, 1, &proj);
	
	
	glUseProgram(windowProg->id);
	glexit("");
	
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj.m);
	glUniform4f(tlx_tly_w_h_ul, 
		gw->header.topleft.x, 
		gw->header.topleft.y, 
		gw->header.size.x, 
		gw->header.size.y 
	);
	glUniform4f(z_alpha_borderWidth_ul, -.1, .5, gw->borderWidth, gw->fadeWidth); // BUG z is a big messed up; -.1 works but .1 doesn't.
	glUniform3f(color_ul, gw->color.x, gw->color.y, gw->color.z); // BUG z is a big messed up; -.1 works but .1 doesn't.
	glUniform4fv(border_ul, 1, &gw->borderColor);

	glBindVertexArray(vaoWindow);
	glBindBuffer(GL_ARRAY_BUFFER, vboWindow);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("");
}

void guiWindowDelete(GUIWindow* gw) {
	
}










Vector2 guiWindowGetClientSize(GUIObject* go) {
	GUIWindow* w = &go->window;
	return w->clientSize;
}

void guiWindowSetClientSize(GUIObject* go, Vector2 cSize) {
	GUIHeader* h = &go->header;
	GUIWindow* w = &go->window;
	w->clientSize = cSize;
	h->size.x = cSize.x + w->padding.left + w->padding.right;
	h->size.y = cSize.y + w->padding.bottom + w->padding.top;
	
	// TODO: trigger resize event
}

// recalculate client size based on client children sizes and positions
Vector2 guiWindowRecalcClientSize(GUIObject* go) {
	GUIWindow* w = &go->window;
	int i;
	Vector2 max = {0, 0}; 
	
	for(i = 0; i < VEC_LEN(&w->clients); i++) {
		GUIHeader* h = (GUIHeader*)VEC_ITEM(&w->clients, i);
		
		max.x = fmax(max.x, h->topleft.x + h->size.x);
		max.y = fmax(max.y, h->topleft.y + h->size.y);
	}
	
	return max;
}

void guiWindowAddClient(GUIObject* parent, GUIObject* child) {
	GUIWindow* w = &parent->window;
	
	int i = VEC_FIND(&w->clients, child);
	if(i < 0) VEC_PUSH(&w->clients, child);
};

void guiWindowRemoveClient(GUIObject* parent, GUIObject* child) {
	GUIWindow* w = &parent->window;
	
	int i = VEC_FIND(&w->clients, child);
	if(i <= 0) VEC_RM(&w->clients, i);
};








