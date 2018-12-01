

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "window.h"
#include "game.h"
#include "texture.h"
#include "hash.h"
#include "log.h"

#include "utilities.h"

// FontConfig
#include "text/fcfg.h"

// for sdf debugging
#include "dumpImage.h"


VEC(GUIObject*) gui_list; 
VEC(GUIObject*) gui_reap_queue; 


GUIObject* guiBaseHitTest(GUIObject* go, Vector2 testPos);


static void preFrame(PassFrameParams* pfp, GUIManager* gm);
static void draw(GUIManager* gm, GLuint progID, PassDrawParams* pdp);
static void postFrame(GUIManager* gm);




// temp
static void addFont(FontManager* fm, char* name);





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
	
	
	pcalloc(gm->fm);
	gm->fm->oversample = 16;
	addFont(gm->fm, "Helvetica");
// 	addFont(gm->fm, "Impact");
// 	addFont(gm->fm, "Modern");
// 	addFont(gm->fm, "Times New Roman");
// 	addFont(gm->fm, "Century");
// 	addFont(gm->fm, "Courier New");
// 	addFont(gm->fm, "Copperplate");
//	addFont(gm->fm, "Mardian Demo");
//	addFont(gm->fm, "Champignon Medium");
//	addFont(gm->fm, "Lucida Blackletter");
	FontManager_createAtlas(gm->fm);
	
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
	
	//printf("FONT INFO: %f, %f\n", off, wid);
	
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











// new font rendering info
static char* defaultCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 `~!@#$%^&*()_+|-=\\{}[]:;<>?,./'\"";

// 16.16 fixed point to float conversion
static float f2f(uint32_t i) {
	return ((double)(i) / 65536.0);
}

// 26.6 fixed point to float conversion
static float f2f26_6(uint32_t i) {
	return ((double)(i) / 64.0);
}


static void blit(
	int src_x, int src_y, int dst_x, int dst_y, int w, int h,
	int src_w, int dst_w, unsigned char* src, unsigned char* dst) {
	
	
	int y, x, s, d;
	
	// this may be upside down...
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			s = ((y + src_y) * src_w) + src_x + x;
			d = ((y + dst_y) * dst_w) + dst_x + x;
			
			dst[d] = src[s];
		}
	}
}

static FT_Library ftLib = NULL;
	

static void checkFTlib() {
	FT_Error err;
	if(!ftLib) {
		err = FT_Init_FreeType(&ftLib);
		if(err) {
			fprintf(stderr, "Could not initialize FreeType library.\n");
			return NULL;
		}
	}
}


static float dist(int a, int b) {
	return a*a + b*b;
}
static float dmin(int a, int b, float d) {
	return fmin(dist(a, b), d);
}

static int boundedOffset(int x, int y, int ox, int oy, int w, int h) {
	int x1 = x + ox;
	int y1 = y + oy;
	if(x1 < 0 || y1 < 0 || x1 >= w || y1 >= h) return -1;
	return x1 + (w * y1);
}

static uint8_t sdfEncode(float d, int inside, float maxDist) {
	int o;
	d = sqrt(d);
	float norm = d / maxDist;
	if(inside) norm = -norm;
	
	o = (norm * 192) + 64;
	
	return o < 0 ? 0 : (o > 255 ? 255 : o);
}

void CalcSDF_Software_(FontGen* fg) {
	
	int searchSize;
	int x, y, ox, oy, sx, sy;
	int dw, dh;
	
	uint8_t* input;
	uint8_t* output;
	
	float d, maxDist;
	
	searchSize = fg->oversample * fg->magnitude;
	maxDist = 0.5 * searchSize;
	dw = fg->rawGlyphSize.x;
	dh = fg->rawGlyphSize.y;
	
	// this is wrong
	fg->sdfGlyphSize.x = floor(((float)(dw/* + (2*fg->magnitude)*/) / (float)(fg->oversample)) + .5);
	fg->sdfGlyphSize.y = floor(((float)(dh/* + (2*fg->magnitude)*/) / (float)(fg->oversample)) + .5); 
	
	fg->sdfGlyph = output = malloc(fg->sdfGlyphSize.x * fg->sdfGlyphSize.y * sizeof(uint8_t));
	input = fg->rawGlyph;
	
	fg->sdfBounds.min.x = 0;
	fg->sdfBounds.max.y =  fg->sdfGlyphSize.y;
	fg->sdfBounds.max.x = fg->sdfGlyphSize.x; 
	fg->sdfBounds.min.y = 0;
	
	// calculate the sdf 
	for(y = 0; y < fg->sdfGlyphSize.y; y++) {
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			int sx = x * fg->oversample;
			int sy = y * fg->oversample;
			//printf(".");
			// value right under the center of the pixel, to determine if we are inside
			// or outside the glyph
			int v = input[sx + (sy * dw)];
			
			d = 999999.99999;
			
			
			for(oy = -searchSize / 2; oy < searchSize; oy++) {
				for(ox = -searchSize / 2; ox < searchSize; ox++) {
					int off = boundedOffset(sx, sy, ox, oy, dw, dh);
					if(off >= 0 && input[off] != v) 
						d = dmin(ox, oy, d);
				}
			}
			
			int q = sdfEncode(d, v, maxDist);
			//printf("%d,%d = %d (%f)\n",x,y,q,d);
			
			output[x + (y * fg->sdfGlyphSize.x)] = q;
		}
	}
	
	
	// find the bounds of the sdf data
	// first rows
	for(y = 0; y < fg->sdfGlyphSize.y; y++) {
		int hasData = 0;
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.x < 255) {
			fg->sdfBounds.min.y = y;
			break;
		}
	}
	for(y = fg->sdfGlyphSize.y - 1; y >= 0; y--) {
		int hasData = 0;
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.x < 255) {
			fg->sdfBounds.max.y = y + 1;
			break;
		}
	}

	for(x = 0; x < fg->sdfGlyphSize.x; x++) {
		int hasData = 0;
		for(y = 0; y < fg->sdfGlyphSize.y; y++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.y < 255) {
			fg->sdfBounds.min.x = x;
			break;
		}
	}
	for(x = fg->sdfGlyphSize.x - 1; x >= 0; x++) {
		int hasData = 0;
		for(y = 0; y < fg->sdfGlyphSize.y; y++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.y < 255) {
			fg->sdfBounds.max.x = x + 1;
			break;
		}
	}
	
	fg->sdfDataSize.x = fg->sdfBounds.max.x - fg->sdfBounds.min.x;
	fg->sdfDataSize.y = fg->sdfBounds.max.y - fg->sdfBounds.min.y;
	
}




static FontGen* addChar(FontManager* fm, FT_Face* ff, int code, int fontSize, char italic, char bold) {
	FontGen* fg;
	FT_Error err;
	FT_GlyphSlot slot;
	
	pcalloc(fg);
	fg->code = code;
	fg->italic = italic;
	fg->bold = bold;
	fg->oversample = fm->oversample;
	fg->magnitude = 8;
	
	int rawSize = fontSize * fm->oversample;
	
	
	err = FT_Set_Pixel_Sizes(*ff, 0, rawSize);
	if(err) {
		fprintf(stderr, "Could not set pixel size to %dpx.\n", rawSize);
		free(fg);
		return;
	}
	
	
	err = FT_Load_Char(*ff, code, FT_LOAD_DEFAULT | FT_LOAD_MONOCHROME);
	
	//f2f(slot->metrics.horiBearingY);
	
	// draw character to freetype's internal buffer and copy it here
	FT_Load_Char(*ff, code, FT_LOAD_RENDER);
	// slot is a pointer
	slot = (*ff)->glyph;
	
	// typographic metrics for later. has nothing to do with sdf generation
	fg->rawAdvance = f2f(slot->linearHoriAdvance); 
	fg->rawBearing.x = f2f26_6(slot->metrics.horiBearingX); 
	fg->rawBearing.y = f2f26_6(slot->metrics.horiBearingY); 
	
	
	// back to sdf generation
	Vector2i rawImgSz = {(slot->metrics.width >> 6), (slot->metrics.height >> 6)};
	
	fg->rawGlyphSize.x = (slot->metrics.width >> 6) + (fg->oversample * fg->magnitude); 
	fg->rawGlyphSize.y = (slot->metrics.height >> 6) + (fg->oversample * fg->magnitude); 
	
	// the raw glyph is copied to the middle of a larger buffer to make the sdf algorithm simpler 
	fg->rawGlyph = calloc(1, sizeof(*fg->rawGlyph) * fg->rawGlyphSize.x * fg->rawGlyphSize.y);
	
	blit(
		0, 0, // src x and y offset for the image
		 (fg->oversample * fg->magnitude * .5), (fg->oversample * fg->magnitude * .5), // dst offset
		rawImgSz.x, rawImgSz.y, // width and height
		slot->bitmap.pitch, fg->rawGlyphSize.x, // src and dst row widths
		slot->bitmap.buffer, // source
		fg->rawGlyph); // destination
	
	
	/// TODO move to multithreaded pass
	CalcSDF_Software_(fg);
	
	// done with the raw data
	free(fg->rawGlyph);
	
	
	/*
	
	printf("raw size: %d, %d\n", fg->rawGlyphSize.x, fg->rawGlyphSize.y);
	printf("sdf size: %d, %d\n", fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	printf("bounds: %f,%f, %f,%f\n",
		fg->sdfBounds.min.x,
		fg->sdfBounds.min.y,
		fg->sdfBounds.max.x,
		fg->sdfBounds.max.y
		   
	);

	writePNG("sdf-raw.png", 1, fg->rawGlyph, fg->rawGlyphSize.x, fg->rawGlyphSize.y);
	writePNG("sdf-pct.png", 1, fg->sdfGlyph, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	*/
	
	return fg;
}


// sorting function for fontgen array
static int gen_comp(const void* aa, const void * bb) {
	FontGen* a = *((FontGen**)aa);
	FontGen* b = *((FontGen**)bb);
	
	if(a->sdfDataSize.y == b->sdfDataSize.y) {
		return b->sdfDataSize.x - a->sdfDataSize.x;
	}
	else {
		return b->sdfDataSize.y - a->sdfDataSize.y;
	}
}



GUIFont* GUIFont_alloc(char* name) {
	GUIFont* f;
	
	pcalloc(f);
	
	f->name = strdup(name);
	f->charsLen = 128;
	f->regular = calloc(1, sizeof(*f->regular) * f->charsLen);
	
	return f;
}


static void addFont(FontManager* fm, char* name) {
	GUIFont* f;
	FT_Error err;
	FT_Face fontFace;
	int len = strlen(defaultCharset);
	
	int fontSize = 8; // pixels
	
	checkFTlib();
	
	// TODO: load font
	char* fontPath = getFontFile(name);
	if(!fontPath) {
		fprintf(stderr, "Could not load font '%s'\n", name);
		return;
	}
	printf("font path: %s: %s\n", name, fontPath);

	err = FT_New_Face(ftLib, fontPath, 0, &fontFace);
	if(err) {
		fprintf(stderr, "Could not access font '%s' at '%'.\n", name, fontPath);
		return;
	}
	
	f = GUIFont_alloc(name);
	
	for(int i = 0; i < len; i++) {
		printf("calc: '%s' %c\n", name, defaultCharset[i]);
		FontGen* fg = addChar(fm, &fontFace, defaultCharset[i], fontSize, 0, 0);
		fg->font = f;
		VEC_PUSH(&fm->gen, fg);
		//addChar(fm, f, ' ', fontSize, 0, 0);
		//addChar(fm, defaultCharset[i], 1, 0);
		//addChar(fm, defaultCharset[i], 0, 1);
		//addChar(fm, defaultCharset[i], 1, 1);
		
		
	}
	
	

}


void FontManager_createAtlas(FontManager* fm) {
	
	qsort(VEC_DATA(&fm->gen), VEC_LEN(&fm->gen), sizeof(VEC_DATA(&fm->gen)), gen_comp);
	
	int totalWidth = 0;
	VEC_EACH(&fm->gen, ind, gen) {
		printf("%c: h: %d, w: %d \n", gen->code, gen->sdfDataSize.y, gen->sdfDataSize.x);
		totalWidth += gen->sdfDataSize.x;
	}
	
	int maxHeight = VEC_ITEM(&fm->gen, 0)->sdfDataSize.y;
	int naiveSize = ceil(sqrt(maxHeight * totalWidth));
	int pot = nextPOT(naiveSize);
	int pot2 = naiveSize / 2;
	
	printf("naive min tex size: %f -> %d (%d)\n", naiveSize, pot, totalWidth);
	
	
	// test the packing
	int row = 0;
	int hext = maxHeight;
	int rowWidth = 0;
// 	int maxh = maxHeight;
	VEC_EACH(&fm->gen, ind, gen) {
		
		if(rowWidth + gen->sdfDataSize.x > pot) {
			row++;
			rowWidth = 0;
			hext += gen->sdfDataSize.y;
		}
		
		rowWidth += gen->sdfDataSize.x;
	}
	
	printf("packing: rows: %d, h extent: %d \n", row, hext);
	if(hext > pot) {
		fprintf(stderr, "character packing overflows texture\n");
		exit(1);
	}
	
	
	// copy the chars into the atlas, cleaning as we go
	uint8_t* texData = fm->atlas = malloc(sizeof(*texData) * pot * pot);
	fm->atlasSize = pot;
	
	// make everything white, the "empty" value
	memset(texData, 255, sizeof(*texData) * pot * pot);
	
	row = 0;
	hext = 0;
	int prevhext = maxHeight;
	rowWidth = 0;
	VEC_EACH(&fm->gen, ind, gen) {
		
		if(rowWidth + gen->sdfDataSize.x > pot) {
			row++;
			rowWidth = 0;
			hext += prevhext;
			prevhext = gen->sdfDataSize.y;
		}
		
		// blit the sdf bitmap data
		blit(
			gen->sdfBounds.min.x, gen->sdfBounds.min.y, // src x and y offset for the image
			rowWidth, hext, // dst offset
			gen->sdfDataSize.x, gen->sdfDataSize.y, // width and height
			gen->sdfGlyphSize.x, pot, // src and dst row widths
			gen->sdfGlyph, // source
			texData); // destination
		
		
		// copy info over to font
		struct charInfo* c = &gen->font->regular[gen->code];
		
		c->texelOffset.x = rowWidth;
		c->texelOffset.y = hext;
		c->texelSize = gen->sdfDataSize;
		c->texNormOffset.x = (float)rowWidth / (float)pot;
		c->texNormOffset.y = (float)hext / (float)pot;
		c->texNormSize.x = (float)gen->sdfDataSize.x / (float)pot;
		c->texNormSize.y = (float)gen->sdfDataSize.y / (float)pot;
		
		// BUG: wrong? needs magnitude?
		c->advance = gen->rawAdvance / (float)gen->oversample;
		c->topLeftOffset.x = (gen->rawBearing.x / (float)gen->oversample) + (float)gen->sdfBounds.min.x;
		c->topLeftOffset.x = (gen->rawBearing.y / (float)gen->oversample) - (float)gen->sdfBounds.min.y;
		
		// advance the write offset
		rowWidth += gen->sdfDataSize.x;
		
		// clean up the FontGen struct
		free(gen->sdfGlyph);
		free(gen);
	}
	
	VEC_FREE(&fm->gen);
	
	
	writePNG("sdf-comp.png", 1, texData, pot, pot);

	
	//exit(1);
	
	
}

