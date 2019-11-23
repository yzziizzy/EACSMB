



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

static int hexDigit(char c) {
	if(c >= '0' && c <= '9') {
		return c - '0';
	}
	else if(c >= 'a' && c <= 'f') {
		return 10 + (c - 'a');
	}
	else if(c >= 'A' && c <= 'F') {
		return 10 + (c - 'A');
	}
	return 0;
}

static double nibbleHexNorm(char* s) {
	if(s[0] == '\0' || s[1] == '\0') return 0.0;
	double d = (hexDigit(s[0]) * 16.0) + hexDigit(s[1]);
	return d / 256.0;
}

Vector4 sexp_argAsColor(sexp* x, int argn) {
	int i;
	union {
		Vector4 c;
		float f[4];
	} u;
	
	u.c.x = 0.0;
	u.c.y = 0.0;
	u.c.z = 0.0;
	u.c.w = 1.0; // default alpha is 1.0 

	if(VEC_LEN(&x->args) < argn) return u.c;
	sexp* arg = VEC_ITEM(&x->args, argn);
	
	if(arg->type == 0) { // it's an s-expression
		for(i = 0; i < VEC_LEN(&arg->args); i++) { 
			u.f[i] = sexp_argAsDouble(arg, i);
		}
	}
	else { // it's a literal
		// throw away any leading BS
		char* s = arg->str;
		char* e = arg->str + strlen(arg->str);
		if(s[0] == '#') s++;
		if(s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
		
		for(i = 0; i < 4 && s < e; i++) {
			u.f[i] = nibbleHexNorm(s);
			s += 2;
		}
	}
	
	printf("color: %f,%f,%f,%f\n", u.c.x, u.c.y, u.c.z, u.c.w);
	
	return u.c;
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

void FloatBitmap_free(FloatBitmap* bmp) {
	if(bmp->data) free(bmp->data);
	bmp->data = NULL;
//	free(bmp);
}
void FloatTex_free(FloatTex* ft) {
	FloatBitmap_free(ft->bmps[0]);
	FloatBitmap_free(ft->bmps[1]);
	FloatBitmap_free(ft->bmps[2]);
	FloatBitmap_free(ft->bmps[3]);
	free(ft->bmps[0]);
	free(ft->bmps[1]);
	free(ft->bmps[2]);
	free(ft->bmps[3]);
	
//	free(ft);
}

FloatTex* FloatTex_similar(FloatTex* orig) {
	return FloatTex_alloc(orig->w, orig->h, orig->channels);
}

FloatTex* FloatTex_copy(FloatTex* orig) {
	FloatTex* new = FloatTex_similar(orig);
	
	for(int i = 0; i < new->channels; i++) {
		memcpy(new->bmps[i]->data, orig->bmps[i]->data, new->w * new->h * sizeof(*(orig->bmps[i]->data)));
	}
	
	return new;
}


float FloatTex_texelFetch(FloatTex* ft, int x, int y, int channel) {
	int xx = iclamp(x, 0, ft->w);
	int yy = iclamp(y, 0, ft->h);
	
	if(channel >= ft->channels) return 0.0;
	
	return ft->bmps[channel]->data[xx + (yy * ft->w)];
}



float FloatTex_sample(FloatTex* ft, float x, float y, int channel) {
	
	float xf = fmod(floor(ft->w * x), ft->w);
	float xc = fmod(ceil(ft->w * x), ft->w);
	float yf = fmod(floor(ft->h * y), ft->h);
	float yc = fmod(ceil(ft->h * y), ft->h);
	
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
// 			d_val = guiEditGetDouble(ed);
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
		
// 		ed = GUIEdit_New("0", pos, size); // TODO: positioning
// 		ed->numType = 2;
// 		ed->onChange = tbcOnChange;
// 		guiEditSetDouble(ed, param->def_value.f);
		
// 		p = calloc(1, sizeof(*p));
// 		p->param = param;
// 		p->op = op;
// 		ed->onChangeData = p;
		
// 		VEC_PUSH(&c->edits, ed);
	}
	
	
}


