

#include "texgen.h"

#include "math.h"








static void gen_sinewave(TexBitmap* bmp, int channel, struct TG_sinewave* opts) {

	int x,y;
	int bytedepth = 4;
	float scaler = (opts->period * 2 * F_PI) / (float)bmp->width;
	float ph = opts->phase * F_2PI / scaler;
	unsigned char* d = bmp->data8;
	
	for(y = 0; y < bmp->height; y++) {
		for(x = 0; x < bmp->width; x++) {
			float th = fmod(((float)x + ph) * scaler, F_2PI);
			d[(y * bmp->width + x) * bytedepth + channel] = ((sin(th) * .5) + .5) * 256;
		}
	}
	
}




static void gen_lerp(BitmapRGBA8* a, BitmapRGBA8* b, int channel_a, int channel_b, float t) {
	
	int x,y;
	int bytedepth = 4;
	unsigned char* da = a->data;
	unsigned char* db = b->data;
	
	
	for(y = 0; y < a->height; y++) {
		for(x = 0; x < a->width; x++) {
			
			int ad = da[(y * a->width + x) * bytedepth + channel_a];
			int bd = db[(y * b->width + x) * bytedepth + channel_b];
			db[(y * b->width + x) * bytedepth + channel_b] = iclamp((ad * t) + (bd * (1.0 - t)), 0, 255);
		}
	}
	
}






static void temptest(GUITexBuilderControl* bc) {
	TexGen* tg;
	void* data;
	
	
	tg = calloc(1, sizeof(*tg));
	bc->tg = tg;
	
	tg->output = calloc(1, sizeof(*tg->output));  
	
	tg->output->width = 512;
	tg->output->height = 512;
	
	data = malloc(512 * 512 * 4);
	TexBitmap bmp;
	bmp.data8 = data;
	bmp.width = 512;
	bmp.height = 512;
	
	struct TG_sinewave opts;
	opts.period = 2.0;
	opts.phase = .25;
	gen_sinewave(&bmp, 0, &opts);
	
	GLuint id;
	
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	
	tg->output->tex_id = id;
	
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
		data);
	
	glGenerateMipmap(GL_TEXTURE_2D);
		
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	bc->im->customTexID = id;
	
}






void guiTexBuilderControlRender(GUITexBuilderControl* bc, GameState* gs) {
	guiRender(bc->bg, gs);
	guiRender(bc->im, gs);
}

void guiTexBuilderControlDelete(GUITexBuilderControl* bc) {
	
}


void guiTexBuilderControlResize(GUITexBuilderControl* bc, Vector2 newSz) {
	
	guiResize(bc->bg, newSz);
	guiResize(bc->im, newSz);
}



static void builderKeyUp(InputEvent* ev, GUITexBuilderControl* bc) {
	if(ev->keysym == XK_Escape) {
		
		guiDelete(bc);
		//gbcTest = NULL;
	}
	
	if(ev->keysym == XK_Left) {
		//bc->yaw = fmod(bc->yaw + .01, F_2PI);
	} 

	
	if(ev->character == 's') { 
		temptest(bc);
	}

}


GUITexBuilderControl* guiTexBuilderControlNew(Vector2 pos, Vector2 size, int zIndex) {
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
	
	guiHeaderInit(&bc->header);
	bc->header.vt = &static_vt;
	bc->inputHandlers = &input_vt;
	
	bc->header.hitbox.min.x = pos.x;
	bc->header.hitbox.min.y = pos.y;
	bc->header.hitbox.max.x = pos.x + size.x;
	bc->header.hitbox.max.y = pos.y + size.y;
	
	bc->header.topleft = pos;
	bc->header.size = size;
	bc->header.z = 0;
	
	
	bc->bg = guiSimpleWindowNew(
		(Vector2){pos.x, pos.y}, 
		(Vector2){size.x, size.y}, 
		zIndex + .0001
	);
	guiRegisterObject(bc->bg, &bc->header);
	
	
	
	bc->im = guiImageNew(
		(Vector2){pos.x + .01, pos.y + .025}, 
		(Vector2){size.x - .02, size.y - .035}, 
		zIndex + .001, 
		-1
	);
	guiRegisterObject(bc->im, &bc->header);
	bc->im->customTexID = 17;
	
	
	return bc;
}



struct texopt_param {
	char* name;
	ptrdiff_t struct_offset;
	int type;
	union {
		double f;
		int64_t i;
	} range_min, range_max, def_value;
	
	
};

// for sine wave
struct texopt_param opt_params[] = {
	{
		.name = "Period",
		.struct_offset = offsetof(struct TG_sinewave, period),
		.type = 1,
		.range_min = { .f = 0.00001 },
		.range_max = { .f = 1000000.0 },
		.def_value = { .f = 2.0 },
	},
	{
		.name = "Phase",
		.struct_offset = offsetof(struct TG_sinewave, phase),
		.type = 1,
		.range_min = { .f = 0.0 },
		.range_max = { .f = 1.0 },
		.def_value = { .f = 0.5 },
	},
	
};


typedef struct GUITBCOptsControl {
	VEC(GUIText*) texts;
	VEC(GUIEdit*) edits;
	
	
	
} GUITBCOptsControl;


struct tbc_opt_param {
	struct texopt_param* param;
	TexGenOp* op;
};

static void tbcOnChange(GUIEdit* ed, struct tbc_opt_param* p) {
	struct texopt_param* param = p->param;
	TexGenOp* op = p->op;
	double d_val;
	
	switch(op->type) {
		case 0:// TODO: real enums
			//d_val = GUIEditGetDoubleVal(ed);
			// TODO: clamp val to limits
			*((float*)(op + param->struct_offset)) = d_val;
			break;
	}
}



GUITBCOptsControl* guiTBCOptsNew(TexGenOp* op, struct texopt_param* params, int paramLen, Vector2 pos, Vector2 size, int zIndex) {
	
	int i;
	GUITBCOptsControl* c;
	GUIEdit* ed;
	struct tbc_opt_param* p;
	float ypos;
	
	
	c = calloc(1, sizeof(*c));
	
	
	for(i = 0; i < paramLen; i++) {
		struct texopt_param* param = params + i;
		
		ed = GUIEditNew("0", pos, size); // TODO: positioning
		ed->onChange = tbcOnChange;
		
		p = calloc(1, sizeof(*p));
		p->param = param;
		p->op = op;
		ed->onChangeData = p;
		
		VEC_PUSH(&c->edits, ed);
	}
	
	
}


