

#define TG_DEFINE_REFLECTION


#include "texgen.h"
#include "sexp.h"

#include "perlin.h"
#include "opensimplex.h"



/*

blending:
	sigmoid
	exp/log
	trig
	common polynomials
	by defined input
perlin noise
fractals
	menger sponge
conway's game of life
penrose tiling
voronoi/worley noise

*/


// forward declarations
#define TEXGEN_TYPE_MEMBER(x) static void gen_##x(FloatTex* ft, struct tg_context* context, struct TG_##x* opts);
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER

// dispatch table
typedef void (*genfn)(FloatTex*, struct tg_context*, void*);

genfn generators[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = (genfn)gen_##x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};

static void run_op(FloatTex* out, struct tg_context* context, TexGenOp* op);

static void gen_lerp(FloatTex* ft, struct tg_context* context, struct TG_lerp* opts) {
}
static void gen_rotate(FloatTex* ft, struct tg_context* context, struct TG_rotate* opts) {
}
static void gen_get(FloatTex* ft, struct tg_context* context, struct TG_get* opts) {
}
static void gen_set(FloatTex* ft, struct tg_context* context, struct TG_set* opts) {
}

static void gen_seq(FloatTex* ft, struct tg_context* context, struct TG_seq* opts) {
	int i;
	
	
	for(i = 0; VEC_LEN(&opts->ops); i++) {
		run_op(ft, context, VEC_ITEM(&opts->ops, i));
	}
	
}

static void gen_chanmux(FloatTex* ft, struct tg_context* context, struct TG_chanmux* opts) {
}
static void gen_solid(FloatTex* ft, struct tg_context* context, struct TG_solid* opts) {
	int x, y;
	float r = opts->color.x;
	float g = opts->color.y;
	float b = opts->color.z;
	float a = opts->color.w;
	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			switch(ft->channels) {
				case 4: ft->bmps[3]->data[y * ft->w + x] = a;
				case 3: ft->bmps[2]->data[y * ft->w + x] = b;
				case 2: ft->bmps[1]->data[y * ft->w + x] = g;
				case 1: ft->bmps[0]->data[y * ft->w + x] = r;
			}
		}
	}
}

static void gen_sinewave(FloatTex* ft, struct tg_context* context, struct TG_sinewave* opts) {

	int x,y;
	float scaler = (opts->period * 2 * F_PI) / (float)ft->w;
	float ph = opts->phase * F_2PI / scaler;
	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			float th = fmod(((float)x + ph) * scaler, F_2PI);
			ft->bmps[context->primaryChannel]->data[y * ft->w + x] = ((sin(th) * .5) + .5);
		}
	}
	
}


static void gen_perlin(FloatTex* ft, struct tg_context* context, struct TG_perlin* opts) {
	
	float min = 999999, max = -99999;
	
	printf("Generating new perlin [%d, %d]... \n", ft->w, ft->h);
	int x, y;
	for(y = 0; y < ft->h ; y++) {
		for(x = 0; x < ft->w ; x++) {
			//tb->zs[x + (y * TERR_TEX_SZ)] = sin(x * .1) * .1;
			float f = PerlinNoise_2D(
				opts->offset_x + (x / opts->spread_x), 
				opts->offset_x + (y / opts->spread_y), 
				opts->persistence, 
				opts->octaves
			);
// 			printf("[%d,%d] %f\n", x,y,f);
			f = (f + .7) / 1.4;
			min = fmin(min, f);
			max = fmax(max, f);
			ft->bmps[context->primaryChannel]->data[y * ft->w + x] = f;
		}
	}
	
	printf("min %f max %f\n", min, max);
}

/*
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

*/


static TexGenOp* op_from_sexp(sexp* sex) {
	TexGenOp* op;
	char* name;
	
	op = calloc(1, sizeof(*op));
	
	name = sexp_argAsStr(sex, 0);
	if(strcaseeq(name, "sinewave")) {
		op->type = TEXGEN_TYPE_sinewave;
		op->sinewave.period = sexp_argAsDouble(sex, 1);
		op->sinewave.phase = sexp_argAsDouble(sex, 2);
	}
	else if(strcaseeq(name, "set")) {
		op->type = TEXGEN_TYPE_set;
		op->set.name = strdup(sexp_argAsStr(sex, 1));
	}
	else if(strcaseeq(name, "get")) {
		op->type = TEXGEN_TYPE_get;
		op->get.name = strdup(sexp_argAsStr(sex, 1));
	}
	else if(strcaseeq(name, "perlin")) {
		op->type = TEXGEN_TYPE_perlin;
		op->perlin.persistence = sexp_argAsDouble(sex, 1);
		op->perlin.octaves = sexp_argAsInt(sex, 2);
		op->perlin.spread_x = sexp_argAsDouble(sex, 3);
		op->perlin.spread_y = sexp_argAsDouble(sex, 4);
		op->perlin.offset_x = sexp_argAsDouble(sex, 5);
		op->perlin.offset_y = sexp_argAsDouble(sex, 6);
	}
	else {
		printf("texgen: no such op '%s'\n", sex->str);
	}
	
	
	return op;
}




static void run_op(FloatTex* out, struct tg_context* context, TexGenOp* op) {
	//TexBitmap* out;
	genfn fn;
	
	fn = generators[op->type];
	if(!fn) {
		fprintf(stderr, "texgen: no generator function for type %d\n", op->type);
		return;
	}
	
	fn(out, context, &op->solid); // the exact union member does not matter	
}






static void temptest(GUITexBuilderControl* bc) {
	TexGenContext* tgc;
	TexGenOp* op;
	void* data;
	
	sexp* sex;
	
	//sex = sexp_parse("(sinewave 3.0 .25)");
	sex = sexp_parse("(perlin .1 8 100 100 20 20)");
	op = op_from_sexp(sex);
	sexp_free(sex);
	
	
	tgc = calloc(1, sizeof(*tgc));
	bc->tg = tgc;
	
	tgc->output = calloc(1, sizeof(*tgc->output));  
	
	tgc->output->width = 512;
	tgc->output->height = 512;
	
	FloatTex* ft = FloatTex_alloc(512, 512, 4);
	
	struct tg_context context;
	
	context.primaryChannel = 0;
	
	//gen_sinewave(&bmp, 0, &opts);
	//gen_perlin(&bmp, 0, &op->perlin);
	run_op(ft, &context, op);
	//gen_sinewave(&bmp, 0, &op->sinewave);
	
	
	BitmapRGBA8* bmp = FloatTex_ToRGBA8(ft);
	
	
	GLuint id;
	
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	
	tgc->output->tex_id = id;
	
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
		
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	bc->im->customTexID = id;
	
}










void* TexBitmap_pixelPointer(TexBitmap* bmp, int x, int y) {
	int stride = TexBitmap_pixelStride(bmp);
	return bmp->data8 + (stride * (x + (y * bmp->width)));
}

int TexBitmap_pixelStride(TexBitmap* bmp) {
	return bmp->channels * TexBitmap_componentSize(bmp);
}

int TexBitmap_componentSize(TexBitmap* bmp) {
	switch(bmp->depth) {
		case TEXDEPTH_8: return 1;
		case TEXDEPTH_16: return 2;
		case TEXDEPTH_32: return 4;
		case TEXDEPTH_FLOAT: return 4;
		case TEXDEPTH_DOUBLE: return 8;
	}	
}






FloatBitmap* FloatBitmap_alloc(int width, int height) {
	FloatBitmap* fb;
	pcalloc(fb);
	
	fb->w = width;
	fb->h = height;
	fb->data = calloc(1, sizeof(*fb->data) * width * height);
	
	return fb;
}


FloatTex* FloatTex_alloc(int width, int height, int channels) {
	FloatTex* ft;
	pcalloc(ft);
	
	ft->channels = channels;
	ft->w = width;
	ft->h = height;
	
	for(int i = 0; i < channels; i++) {
		ft->bmps[i] = FloatBitmap_alloc(width, height);
	}
	
	return ft;
}




float FloatTex_texelFetch(FloatTex* ft, int x, int y, int channel) {
	int xx = iclamp(x, 0, ft->w);
	int yy = iclamp(y, 0, ft->h);
	
	if(channel >= ft->channels) return 0.0;
	
	return ft->bmps[channel]->data[xx + (yy * ft->w)];
}


float FloatTex_sample(FloatTex* ft, float x, float y, int channel) {
	
	float xf = floor(ft->w * x);
	float xc = ceil(ft->w * x);
	float yf = floor(ft->h * y);
	float yc = ceil(ft->h * y);
	
	float xfyf = FloatTex_texelFetch(ft, xf, yf, channel);
	float xfyc = FloatTex_texelFetch(ft, xf, yc, channel);
	float xcyf = FloatTex_texelFetch(ft, xc, yf, channel);
	float xcyc = FloatTex_texelFetch(ft, xc, yc, channel);
	
	// TODO BUG: check the order here
	float l_yf = flerp(xfyf, xcyf, xc - x);
	float l_yc = flerp(xfyc, xcyc, xc - x);
	return flerp(l_yf, l_yc, yc - y);
}


uint32_t floats_to_uin32(float r, float g, float b, float a) {
	union {
		uint8_t b[4];
		uint32_t n;
	} u;
	
	u.b[0] = iclamp(round(r * 255), 0, 255); 
	u.b[1] = iclamp(round(g * 255), 0, 255); 
	u.b[2] = iclamp(round(b * 255), 0, 255); 
	u.b[3] = iclamp(round(a * 255), 0, 255); 
	
	return u.n;
}

BitmapRGBA8* FloatTex_ToRGBA8(FloatTex* ft) {
	
	BitmapRGBA8* bmp;
	pcalloc(bmp);
	bmp->width = ft->w;
	bmp->height = ft->h;
	bmp->data = malloc(ft->w * ft->h * 4);
	
	int x, y;
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			float r = 0, g = 0, b = 0, a = 0;
			switch(ft->channels) {
				case 4: a = FloatTex_texelFetch(ft, x, y, 3);
				case 3: b = FloatTex_texelFetch(ft, x, y, 2);
				case 2: g = FloatTex_texelFetch(ft, x, y, 1);
				case 1: r = FloatTex_texelFetch(ft, x, y, 0);
			}
			
			bmp->data[x + (y * ft->w)] = floats_to_uin32(r, g, b, a);
		}
	}
	
	return bmp;
}

/*
static void str_multi_split(char* input, char* break_chars, char*** output, size_t* out_len) {
	size_t alloc_sz = 8;
	size_t ol = 0;
	char** out;
	char* s, *e;
	char beginning = 1;
	
	if(!input || !break_chars) {
		*output = NULL;
		*out_len = 0;
		return;
	}
	
	out = malloc(sizeof(*out) * alloc_sz);
	s = input;
	
	while(*s) {
		e = strpbrk(s, break_chars);
		
		if(beginning) {
			 s = e;
			 continue;
		}
		
		s = e;
		ol++;
	}
	
	*output = out;
	*out_len = ol;
} 




static void parseConfig(char* path) {
	
	size_t len;
	char* source = readFile(path, &len);
	
	
	
	
	
} 





*/


void guiTexBuilderControlRender(GUITexBuilderControl* bc, GameState* gs, PassFrameParams* pfp) {
	guiRender(bc->bg, gs, pfp);
	guiRender(bc->im, gs, pfp);
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
	
// 	bc->bg->bg->borderWidth = 0;
// 	bc->bg->bg->fadeWidth = 0;
	guiRegisterObject(bc->bg, &bc->header);
	
	
	
	bc->im = guiImageNew(
		(Vector2){pos.x + .01, pos.y + .025}, 
		(Vector2){size.x - .02, size.y - .035}, 
		zIndex + .001, 
		-1
	);
	guiRegisterObject(bc->im, &bc->header);
	bc->im->customTexID = 17;
	
	guiAddClient(bc->bg, bc->im);
	Vector2 bgsz = guiRecalcClientSize(bc->bg);
	printf("simplewindosz: %f, %f \n", bgsz.x, bgsz.y);
	guiResize(bc->bg, bgsz);
	
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
	struct tg_reflect* param;
	TexGenOp* op;
};

void tbcOnChange(GUIEdit* ed, struct tbc_opt_param* p) {
	struct texopt_param* param = p->param;
	TexGenOp* op = p->op;
	double d_val;
	
	switch(op->type) {
		case 0:// TODO: real enums
			d_val = guiEditGetDouble(ed);
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
		
		// TODO: text label
		
		ed = GUIEditNew("0", pos, size); // TODO: positioning
		ed->numType = 2;
		ed->onChange = tbcOnChange;
		guiEditSetDouble(ed, param->def_value.f);
		
		p = calloc(1, sizeof(*p));
		p->param = param;
		p->op = op;
		ed->onChangeData = p;
		
		VEC_PUSH(&c->edits, ed);
	}
	
	
}


