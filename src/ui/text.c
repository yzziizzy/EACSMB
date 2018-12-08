

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../window.h"
#include "../gui.h"
#include "../gui_internal.h"
#include "../hash.h"

#include "../utilities.h"



static void render(GUIText* gt, AABB2* clip, PassFrameParams* pfp);
static void guiTextDelete(GUIText* gt);


static void writeCharacterGeom(GUIUnifiedVertex* v, struct charInfo* ci, Vector2 off, float sz, float adv, float line);


GUIText* GUIText_new(GUIManager* gm, char* str, char* fontname, float fontSize) {
	
	static struct gui_vtbl static_vt = {
		.Render = render,
		.Delete = guiTextDelete,
	};
	
	
	GUIText* gt;
	pcalloc(gt);
	
// 	gt->header.gm = gm;
	gui_headerInit(&gt->header, gm, &static_vt);
// 	gt->header.vt = &static_vt; 
	
	gt->fontSize = fontSize;
	gt->font = FontManager_findFont(gm->fm, fontname);

	if(str) {
		gt->currentStr = strdup(str);
	}
	
	return gt;
}



static void render(GUIText* gt, AABB2* clip, PassFrameParams* pfp) {
	char* txt = gt->currentStr;
	GUIFont* f = gt->font;
	GUIManager* gm = gt->header.gm;
	
	
	float size = 0.55;
	float hoff = 0;
	float adv = 0;
	
	float spaceadv = f->regular[' '].advance;
	
	for(int n = 0; txt[n] != 0; n++) {
		char c = txt[n];
		
		struct charInfo* ci = &f->regular[c];
		
		
		if(c != ' ') {
			GUIUnifiedVertex* v = GUIManager_checkElemBuffer(gm, 1);
			writeCharacterGeom(v, ci, gt->header.topleft, size, adv, hoff);
			adv += ci->advance * size; // BUG: needs sdfDataSize added in?
			v->fg = (struct Color4){255, 128, 64, 255},
			//v++;
			gm->elementCount++;
		}
		else {
			adv += spaceadv;
		}
		
		
		
	}
}

void guiTextDelete(GUIText* gt) {
	printf("NIH guiTextDelete " __FILE__ ":%d\n", __LINE__);
}


float guiTextGetTextWidth(GUIText* gt, int numChars) {
//	return CalcTextWidth(gt->strRI, numChars);
	return 0;
}


void GUIText_setString(GUIText* gt, char* newval) {
	if(0 != strcmp(newval, gt->currentStr)) {
		if(gt->currentStr) free(gt->currentStr);
		gt->currentStr = strdup(newval);
	}
}









static void writeCharacterGeom(GUIUnifiedVertex* v, struct charInfo* ci, Vector2 off, float sz, float adv, float line) {
	float offx = ci->texNormOffset.x;//TextRes_charTexOffset(gm->font, 'A');
	float offy = ci->texNormOffset.y;//TextRes_charTexOffset(gm->font, 'A');
	float widx = ci->texNormSize.x;//TextRes_charWidth(gm->font, 'A');
	float widy = ci->texNormSize.y;//TextRes_charWidth(gm->font, 'A');
	
	v->pos.t = off.y + line - ci->topLeftOffset.y * sz;
	v->pos.l = off.x + adv + ci->topLeftOffset.x * sz;
	v->pos.b = off.y + line + ci->size.y * sz - ci->topLeftOffset.y * sz;
	v->pos.r = off.x + adv + ci->size.x * sz + ci->topLeftOffset.x * sz;
	
	v->guiType = 1; // text
	
	v->texOffset1.x = offx * 65535.0;
	v->texOffset1.y = offy * 65535.0;
	v->texSize1.x = widx *  65535.0;
	v->texSize1.y = widy * 65535.0;
	v->texIndex1 = ci->texIndex;
}


