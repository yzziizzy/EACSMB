

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../window.h"
#include "../gui.h"
#include "../hash.h"

#include "../utilities.h"


HashTable* font_cache; // TextRes*

ShaderProgram* textProg;

void gui_Text_Init() {
	
	//init font cache
	font_cache = HT_create(2);
	
	textProg = loadCombinedProgram("guiTextSDF");
}






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
		tr = GenerateSDFFont(name, 32, NULL);
		SaveSDFFont(filename, tr);
	}
	
	HT_set(font_cache, name, tr);
	
	free(filename);
	
	return tr;
}



void guiTextRender(GUIText* gt, GameState* gs, PassFrameParams* pfp);
void guiTextDelete(GUIText* gt);


GUIText* guiTextNew(char* str, Vector2 pos, float size, char* fontname) {
	
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
	
	guiHeaderInit(&gt->header);
	gt->header.vt = &static_vt; 
	
	gt->header.topleft = pos;
	gt->size = size;
	
	gt->font = getFont(fontname);
	if(!gt->font) {
		fprintf(stderr, "Failed to load font: %s\n", fontname);
	}
	
	if(str) {
		gt->current = strdup(str);
		gt->strRI = prepareText(gt->font, str, -1, colors);
	}
	
	
	//VEC_PUSH(&gui_list, gt);
	
	return gt;
}



void guiTextRender(GUIText* gt, GameState* gs, PassFrameParams* pfp) {
	
	Matrix textProj;
	Vector v;
	MatrixStack textModel;
	
	msAlloc(3, &textModel);
	
	glUseProgram(textProg->id);
	
	// text stuff
	textProj = IDENT_MATRIX;	
	mOrtho(0, 1, 0, 1, 0, 1, &textProj);
	
	msIdent(&textModel);
	// the text is really big
	msScale3f(gt->size * .01, gt->size * .01, gt->size * .01, &textModel);

	GLuint world_ul = glGetUniformLocation(textProg->id, "world");
	GLuint tp_ul = glGetUniformLocation(textProg->id, "mProj");
	GLuint tm_ul = glGetUniformLocation(textProg->id, "mModel");
	GLuint ts_ul = glGetUniformLocation(textProg->id, "fontTex");
	
	glUniform2fv(world_ul, 1, &gt->header.topleft);
	glUniformMatrix4fv(tp_ul, 1, GL_FALSE, textProj.m);
	glUniformMatrix4fv(tm_ul, 1, GL_FALSE, msGetTop(&textModel)->m);
	glexit("text matrix uniforms");

	glDisable(GL_CULL_FACE);
	
	glActiveTexture(GL_TEXTURE0 + 28);
	glexit("active texture");

	glUniform1i(ts_ul, 28);
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


float guiTextGetTextWidth(GUIText* gt, int numChars) {
	return CalcTextWidth(gt->strRI, numChars);
}


void guiTextSetValue(GUIText* gt, char* newval) {
	unsigned int colors[] = {
		0xddddddFF, INT_MAX
	};
	
	if(0 != strcmp(newval, gt->current)) {
		if(gt->current) free(gt->current);
		gt->current = strdup(newval);
	
		updateText(gt->strRI, newval, -1, colors);
	}
}
