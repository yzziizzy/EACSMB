


#include "../sexp.h"
#include "../texgen.h"
#include "../dumpImage.h"

#include "../gui_internal.h"

#include "ui.h"




static void tbc_Regen(GUITexBuilderControl* bc) {
	
	
	
	TexGenContext* tgc;
	void* data;
	
	char* prog = "" \
		"(seq " \
		"	(sinewave 30.0 .25)" 
//		"	(worley boxed 4 8 70)" 
// 		"	(perlin .5 6 16 16 100 100)" \
		//"	(blend 1 2 .5)" 
		//"	(chanmux 0 0  1 0  2 0  -1 0)" 
// 		"	(squares 10 .5 #003300 (.2 .4 .1))" 
//  		"	(checkers 10 #663300 (.5 .4 .1))" 
// 		"	(normal_map 0)" \
		""  
		")" \
	;
	
	sexp* sex = sexp_parse(prog);
	bc->op = op_from_sexp(sex);
	sexp_free(sex);
// 	
	tgc = calloc(1, sizeof(*tgc));
	bc->tg = tgc;
	
	tgc->output = calloc(1, sizeof(*tgc->output));  
	
	tgc->output->width = 512;
	tgc->output->height = 512;
	
	HashTable(FloatTex*) storage;
	HT_init(&storage, 4);
	
	//FloatTex* ft = FloatTex_alloc(512, 512, 4);
	
	struct tg_context context;
	
	context.w = 512;
	context.h = 512;
	context.channels = 4;
	
	VEC_INIT(&context.stack);
	context.storage = &storage;
	context.primaryChannel = 0;
	
	//gen_sinewave(&bmp, 0, &opts);
	//gen_perlin(&bmp, 0, &op->perlin);
	run_op(&context, bc->op);
	//gen_sinewave(&bmp, 0, &op->sinewave);
	
	FloatTex* ft = VEC_TAIL(&context.stack);
	
	//HACK: clear out the alpha channel. it's empty for some reason
	for(int x = 0; x < 512; x++) {
		for(int y = 0; y < 512; y++) {
			ft->bmps[3]->data[x + y * 512] = 1.0;
		}
	}
	
	//FloatTex* ft = VEC_ITEM(&context.stack, 4);
	BitmapRGBA8* bmp = FloatTex_ToRGBA8(ft);
	
	
	
	
	// -------- upload texture --------
	
	
	
	GLuint id;
	
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	
	//tgc->output->tex_id = id;
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA8, // internalformat
		512,
		512,
		0,  // border
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // input type
		bmp->data);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	
	if(bc->im->texHandle) {
		glMakeTextureHandleNonResidentARB(bc->im->texHandle);
		// release old texture
		if(bc->texID) glDeleteTextures(1, &bc->texID);
	}
	
	bc->texID = id;
	
	GLuint64 texHandle = glGetTextureHandleARB(id);
	glexit("");
	glMakeTextureHandleResidentARB(texHandle);
	glexit("");
	
	bc->im->texHandle = texHandle;
	printf("%p\n", texHandle);
// 	g_texgenhandle = texHandle;
			
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	// free temporary info
// 	ft;
// 	bmp;
// 	tgc;
}






void guiTexBuilderControlRender(GUITexBuilderControl* bc, PassFrameParams* pfp) {
	GUIHeader_renderChildren(&bc->header, pfp);
// 	guiRender(bc->im, gs, pfp);
}

void guiTexBuilderControlDelete(GUITexBuilderControl* bc) {
	
}


void guiTexBuilderControlResize(GUITexBuilderControl* bc, Vector2 newSz) {
	
// 	guiResize(bc->bg, newSz);
// 	guiResize(bc->im, newSz);
}



static void builderKeyUp(InputEvent* ev, GUITexBuilderControl* bc) {
	if(ev->keysym == XK_Escape) {
		
		GUIObject_revertFocus(bc);
		guiDelete(bc);
		//gbcTest = NULL;
	}
	
	if(ev->keysym == XK_Up) {
		bc->selectedOpIndex = (bc->selectedOpIndex - 1 + MAX_TEXGEN_TYPE) % MAX_TEXGEN_TYPE;
		GUIText_setString(bc->ctl_selectedOp, texgen_op_names[bc->selectedOpIndex]);
	} 
	if(ev->keysym == XK_Down) {
		bc->selectedOpIndex = (bc->selectedOpIndex + 1) % MAX_TEXGEN_TYPE;
		GUIText_setString(bc->ctl_selectedOp, texgen_op_names[bc->selectedOpIndex]);
	} 
	
	if(ev->character == 'i') {
		//printf("inserted perlin");
		bc->op = make_op(bc->selectedOpIndex);
		setDefaultsFns[bc->selectedOpIndex](bc->op);
		
		if(bc->sa) GUIObject_delete(bc->sa);
		
		bc->sa = GUIStructAdjuster_new(bc->header.gm, &bc->op->perlin, tgsa_fields[bc->selectedOpIndex]); 
		bc->sa->header.topleft.x = 500;
		
		GUIRegisterObject(bc->sa, bc->bg);
		
	} 

	
	if(ev->character == 'a') { 
		
	}
	if(ev->character == 's') { 
// 		temptest(bc);
	}
	if(ev->keysym == XK_F5) { 
		printf("refreshing texgen\n");
		tbc_Regen(bc);
	}

}


GUITexBuilderControl* guiTexBuilderControlNew(GUIManager* gm, Vector2 pos, Vector2 size, int zIndex) {
	GUITexBuilderControl* bc;
	
	static struct gui_vtbl static_vt = {
		.Render = guiTexBuilderControlRender,
		.Delete = guiTexBuilderControlDelete,
		.Resize = guiTexBuilderControlResize
	};
	
	static InputEventHandler input_vt = {
		.keyUp = builderKeyUp,
	};
	
	
	bc = calloc(1, sizeof(*bc));
	CHECK_OOM(bc);
	
	gui_headerInit(&bc->header, gm, &static_vt);
	bc->header.input_vt = &input_vt;
	
	bc->bg = GUIWindow_new(gm);
	bc->bg->header.topleft = pos;
	bc->bg->header.size = size;
	bc->bg->color = (Vector){.2, .4, .3};
	//bc->bg = guiSimpleWindowNew(
		//(Vector2){pos.x, pos.y}, 
		//(Vector2){size.x, size.y}, 
		//zIndex + .0001
	//);
	
// 	bc->bg->bg->borderWidth = 0;
// 	bc->bg->bg->fadeWidth = 0;
	GUIRegisterObject(bc->bg, &bc->header);
	
	
	
	bc->im = GUIImage_new(gm, NULL);
	bc->im->header.topleft = (Vector2){10, 10}; 
	bc->im->header.size = (Vector2){400, 400}; 
	
	GUIRegisterObject(bc->im, &bc->bg->header);
	
	
	
	bc->tree = GUITreeControl_New(gm);
	bc->tree->header.topleft = (Vector2){20, 350};
	GUIRegisterObject(bc->tree, &bc->bg->header);
	
	GUITreeControl_AppendLabel(bc->tree, NULL, "tree root label", 1);
	
	/*
	bc->controls = GUIGridLayout_new(gs->gui, (Vector2){0,0}, (Vector2){35, 35});
	
	bc->controls->maxCols = 1;
	bc->controls->maxRows = 999999;
	bc->controls->header.gravity = GUI_GRAV_CENTER_BOTTOM;
	for(int i = 0; i < 20; i++) {
		GUIImage* img = GUIImage_new(gs->gui, iconNames[i]);
		img->header.size = (Vector2){30, 30};
		GUIRegisterObject(img, bc->controls);
	}
	
	GUIRegisterObject(bc->controls, &bc->header);
	*/
	
	
	bc->ctl_selectedOp = GUIText_new(gm, "", "Arial", 4.0f);
	bc->ctl_selectedOp->header.topleft = (Vector2){410, 10}; 
	
	GUIRegisterObject(bc->ctl_selectedOp, &bc->bg->header);
	
/*
	bc->da = GUIDebugAdjuster_new(gm, "test: %d", &bc->selectedOpIndex, 'i');
	GUIRegisterObject(bc->da, &bc->bg->header);*/

	
	return bc;
}

