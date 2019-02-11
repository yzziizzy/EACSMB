

#define TG_DEFINE_REFLECTION


#include "texgen.h"
#include "sexp.h"

#include "perlin.h"
#include "opensimplex.h"

#include "gui_internal.h"




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


static char* texgen_op_names[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = #x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};






static void tg_context_push(tg_context* c, FloatTex* ft) {
	VEC_PUSH(&c->stack, ft);
}

static FloatTex* FloatTex_fromContext(tg_context* c) {
	return FloatTex_alloc(c->w, c->h, c->channels);
}

static FloatTex* tg_context_index(tg_context* context, int i) {
	return VEC_ITEM(&context->stack, VEC_LEN(&context->stack) - 1 - i);
}

// forward declarations
#define TEXGEN_TYPE_MEMBER(x) static void gen_##x(struct tg_context* context, struct TG_##x* opts);
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER

// dispatch table
typedef void (*genfn)(struct tg_context*, void*);

genfn generators[] = {
#define TEXGEN_TYPE_MEMBER(x) [TEXGEN_TYPE_##x] = (genfn)gen_##x,
	TEXGEN_TYPE_LIST
#undef TEXGEN_TYPE_MEMBER
};

static void run_op(struct tg_context* context, TexGenOp* op);

static void gen_lerp(struct tg_context* context, struct TG_lerp* opts) {
	FloatTex* ft = VEC_TAIL(&context->stack);
	
	
	
	
	tg_context_push(context, ft);
}

static void gen_rotate(struct tg_context* context, struct TG_rotate* opts) {
}

static void gen_get(struct tg_context* context, struct TG_get* opts) {
	FloatTex* ft;
	//HT_get(context->storage, opts->name, &ft);
	VEC_PUSH(&context->stack, ft);
}

static void gen_set(struct tg_context* context, struct TG_set* opts) {
	
	FloatTex* ft = VEC_TAIL(&context->stack);
	//HT_set(context->storage, opts->name, ft); 
}


static void gen_seq(struct tg_context* context, struct TG_seq* opts) {
	int i;
	
	
	for(i = 0; i < VEC_LEN(&opts->ops); i++) {
		run_op(context, VEC_ITEM(&opts->ops, i));
	}
	
}

static void gen_chanmux(struct tg_context* context, struct TG_chanmux* opts) {
	FloatTex* ft = FloatTex_fromContext(context);
	
	int depth = VEC_LEN(&context->stack) - 1;
	
	if(opts->r_i > -1) { 
		memcpy(
			ft->bmps[0]->data,
			VEC_ITEM(&context->stack, depth - opts->r_i)->bmps[opts->r_c]->data, 
			context->w * context->h * sizeof(float)
		);
	}
	
	if(opts->g_i > -1) {
		memcpy(
			ft->bmps[1]->data,
			VEC_ITEM(&context->stack, depth - opts->g_i)->bmps[opts->g_c]->data, 
			context->w * context->h * sizeof(float)
		);
	}
	
	if(opts->b_i > -1) {
		memcpy(
			ft->bmps[2]->data,
			VEC_ITEM(&context->stack, depth - opts->b_i)->bmps[opts->b_c]->data,  
			context->w * context->h * sizeof(float)
		);
	}
	
	if(opts->a_i > -1) {
		memcpy(
			ft->bmps[3]->data,
			VEC_ITEM(&context->stack, depth - opts->a_i)->bmps[opts->a_c]->data, 
			context->w * context->h * sizeof(float)
		);
	}
	
	
	
	tg_context_push(context, ft);
}

// squares in a grid
static void gen_squares(struct tg_context* context, struct TG_squares* opts) {
	int x, y;
	float grid = opts->grid;
	float size = opts->size;
	Vector4 color = opts->color;
	Vector4 bg = opts->background;
	
	FloatTex* ft = FloatTex_fromContext(context);
	
	float stripe = (ft->w / grid);
	float ratio = (stripe * size) / 2;
	
	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			Vector4* c = &bg;
			
			float yy = fmod(y, stripe);
			float xx = fmod(x, stripe);
			if(
				(yy > ratio && yy < stripe - ratio)
				&& (xx > ratio && xx < stripe - ratio)
			) { // inside square
				c = &color;
			}
			
			switch(ft->channels) {
				case 4: ft->bmps[3]->data[y * ft->w + x] = c->w;
				case 3: ft->bmps[2]->data[y * ft->w + x] = c->z;
				case 2: ft->bmps[1]->data[y * ft->w + x] = c->y;
				case 1: ft->bmps[0]->data[y * ft->w + x] = c->x;
			}
		}
	}
	
	tg_context_push(context, ft);
}


// checkerboard
static void gen_checkers(struct tg_context* context, struct TG_checkers* opts) {
	int x, y;
	float grid = opts->grid;
	Vector4 color = opts->color;
	Vector4 bg = opts->background;
	
	FloatTex* ft = FloatTex_fromContext(context);
	
	float stripe = (ft->w / grid);
	float half = stripe / 2;
	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			Vector4* c = &bg;
			
			float yy = fmod(y, stripe);
			float xx = fmod(x, stripe);
			if((yy > half) ^ (xx > half)) { // inside square
				c = &color;
			}
			
			switch(ft->channels) {
				case 4: ft->bmps[3]->data[y * ft->w + x] = c->w;
				case 3: ft->bmps[2]->data[y * ft->w + x] = c->z;
				case 2: ft->bmps[1]->data[y * ft->w + x] = c->y;
				case 1: ft->bmps[0]->data[y * ft->w + x] = c->x;
			}
		}
	}
	
	tg_context_push(context, ft);
}





static float FloatTex_sampleGrayscale(FloatTex* ft, int x, int y) {
	float t = 0;
	int n = 0;
	
// 	x = iclamp(x, 0, ft->w);
// 	y = iclamp(y, 0, ft->h);
	x = x % ft->w;
	y = y % ft->h;
	
	switch(ft->channels) {
		case 4: t += ft->bmps[3]->data[y * ft->w + x]; n++;
		case 3: t += ft->bmps[2]->data[y * ft->w + x]; n++;
		case 2: t += ft->bmps[1]->data[y * ft->w + x]; n++;
		case 1: t += ft->bmps[0]->data[y * ft->w + x]; n++;
	}
	
	return t / n;
}


// generate a normal map based on pixel brightness
static void gen_normal_map(struct tg_context* context, struct TG_normal_map* opts) {
	int x, y;
	
	FloatTex* ft = FloatTex_fromContext(context);
	
	int depth = VEC_LEN(&context->stack) - 1;
	FloatTex* src = VEC_ITEM(&context->stack, iclamp(depth - opts->index, 0, depth));
	
	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			
			float d_x = FloatTex_sampleGrayscale(src, x + 1, y) - FloatTex_sampleGrayscale(src, x - 1, y);
			float d_y = FloatTex_sampleGrayscale(src, x, y + 1) - FloatTex_sampleGrayscale(src, x, y - 1);
			
			Vector norm = {-d_x, -d_y, 1.0};
			vNorm(&norm, &norm);
			
			ft->bmps[0]->data[y * ft->w + x] = norm.x * .5 +.5;
			ft->bmps[1]->data[y * ft->w + x] = norm.y * .5 +.5;
			ft->bmps[2]->data[y * ft->w + x] = norm.z * .5 +.5;
			ft->bmps[3]->data[y * ft->w + x] = 1.0;
		}
	}
	
	tg_context_push(context, ft);
}

// changes the context
static void gen_context(struct tg_context* context, struct TG_context* opts) {

}


static void gen_solid(struct tg_context* context, struct TG_solid* opts) {
	int x, y;
	float r = opts->color.x;
	float g = opts->color.y;
	float b = opts->color.z;
	float a = opts->color.w;
	
	FloatTex* ft = FloatTex_fromContext(context);
	
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
	
	tg_context_push(context, ft);
}

static void gen_gradient(struct tg_context* context, struct TG_gradient* opts) {
	FloatTex* ft = FloatTex_fromContext(context);
	FloatTex* b = VEC_ITEM(&context->stack, VEC_LEN(&context->stack) - opts->index);
	
	int x, y;	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			Vector4 c;
			float f = FloatTex_texelFetch(b, x, y, opts->channel);
			
			vLerp4(&opts->color1, &opts->color2, f, &c);
			ft->bmps[0]->data[y * ft->w + x] = c.x;
			ft->bmps[1]->data[y * ft->w + x] = c.y;
			ft->bmps[2]->data[y * ft->w + x] = c.z;
			ft->bmps[3]->data[y * ft->w + x] = c.w;
		}
	}
}

static void gen_sinewave(struct tg_context* context, struct TG_sinewave* opts) {

	FloatTex* ft = FloatTex_fromContext(context);
	
	int x,y;
	float scaler = (opts->period * 2 * F_PI) / (float)ft->w;
	float ph = opts->phase * F_2PI / scaler;
	
	for(y = 0; y < ft->h; y++) {
		for(x = 0; x < ft->w; x++) {
			float th = fmod(((float)x + ph) * scaler, F_2PI);
			ft->bmps[context->primaryChannel]->data[y * ft->w + x] = ((sin(th) * .5) + .5);
		}
	}
	
	tg_context_push(context, ft);
}



static void gen_perlin(struct tg_context* context, struct TG_perlin* opts) {
	FloatTex* ft = FloatTex_fromContext(context);
	
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
	
	tg_context_push(context, ft);
}


// VERY slow
static float wrapdist(Vector2 pos, Vector2 p, float w, float h) {
	float d1 = vDist2(&pos, &(Vector2){p.x + w, p.y});
	float d2 = vDist2(&pos, &(Vector2){p.x - w, p.y});
	float d3 = vDist2(&pos, &(Vector2){p.x, p.y + h});
	float d4 = vDist2(&pos, &(Vector2){p.x, p.y - h});
	float d5 = vDist2(&pos, &(Vector2){p.x + w, p.y + h});
	float d6 = vDist2(&pos, &(Vector2){p.x - w, p.y + h});
	float d7 = vDist2(&pos, &(Vector2){p.x + w, p.y - h});
	float d8 = vDist2(&pos, &(Vector2){p.x - w, p.y - h});
	float d = vDist2(&pos, &p);
	
	return fmin(d, fmin(fmin(fmin(d1, d2), fmin(d3, d4)), fmin(fmin(d5, d6), fmin(d7, d8)))); 
}


static float closest(Vector2* points, int plen, Vector2 pos, float w, float h) {
	int i;
	float cdist = 9999999999;

	for(i = 0; i < plen; i++) {
		float d = wrapdist(pos, *(points + i), w, h);
		cdist = fmin(cdist, d);
	}
	
	return cdist;
}

static float second_closest(Vector2* points, int plen, Vector2 pos, float w, float h) {
	int i;
	float cdist = 9999999999;
	float cdist2 = 9999999999;

	for(i = 0; i < plen; i++) {
		float d = wrapdist(pos, *(points + i), w, h);
		if(d < cdist) {
			cdist2 = cdist;
			cdist = d;
		}
		else if(d < cdist2) {
			cdist2 = d;
		}
	}
	
	return cdist2;
}


static void gen_worley(struct tg_context* context, struct TG_worley* opts) {
	int x, y, i, p;
	int pCnt = opts->num_points;
	Vector2* points;
	
	FloatTex* ft = FloatTex_fromContext(context);
	
	// point generation
	if(strcaseeq(opts->algorithm, "random")) {
		// generate random points
		points = malloc(sizeof(*points) * pCnt);
		for(x = 0; x < pCnt; x++) {
			points[x].x = frand(0, ft->w); 
			points[x].y = frand(0, ft->h); 
		}
	}
	else if(strcaseeq(opts->algorithm, "boxed")) {
		Vector2 boxSide = {
			(float)ft->w / (float)opts->boxes,
			(float)ft->h / (float)opts->boxes
		};
		
		pCnt *= opts->boxes * opts->boxes;
		points = malloc(sizeof(*points) * pCnt);
		
		p = 0;
		for(y = 0; y < opts->boxes; y++) {
			for(x = 0; x < opts->boxes; x++) {
				for(i = 0; i < opts->num_points; i++) {
					points[p].x = frand(x * boxSide.x, (x + 1) * boxSide.x); 
					points[p].y = frand(y * boxSide.y, (y + 1) * boxSide.y); 
					p++; 
				}
			}
		}
				
	}
	else if(strcaseeq(opts->algorithm, "sample")) {
		fprintf(stderr, "Worley sample algorithm not implemented\n");
	}
	else {
		fprintf(stderr, "No such worley algorithm\n");
	}
	
	
	for(y = 0; y < ft->h ; y++) {
		for(x = 0; x < ft->w ; x++) {
			float f = closest(points, pCnt, (Vector2){x, y}, ft->w, ft->h);
			ft->bmps[context->primaryChannel]->data[y * ft->w + x] = f / opts->divisor;
		}
	}
	
	
	free(points);
	tg_context_push(context, ft);
}


#define pixel(ft, x, y, c) ((ft)->bmps[(c)]->data[((y) * (ft)->w) + (x)])

static void gen_blend(struct tg_context* context, struct TG_blend* opts) {
	int x, y;
	FloatTex* a = tg_context_index(context, opts->a_index);
	FloatTex* b = tg_context_index(context, opts->b_index);
	FloatTex* ft = FloatTex_fromContext(context);
	
	
	for(y = 0; y < ft->h ; y++) {
		for(x = 0; x < ft->w ; x++) {
			pixel(ft, x, y, 0) = flerp(pixel(a, x, y, 0), pixel(b, x, y, 0), opts->t);
			pixel(ft, x, y, 1) = flerp(pixel(a, x, y, 1), pixel(b, x, y, 1), opts->t);
			pixel(ft, x, y, 2) = flerp(pixel(a, x, y, 2), pixel(b, x, y, 2), opts->t);
			pixel(ft, x, y, 3) = flerp(pixel(a, x, y, 3), pixel(b, x, y, 3), opts->t);
		}
	}
	
	tg_context_push(context, ft);
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

static Vector4 sexp_argAsColor(sexp* x, int argn) {
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

static TexGenOp*  make_op(int type) {
	TexGenOp* op = calloc(1, sizeof(*op));
	op->type = type;
	return op;
}

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
	else if(strcaseeq(name, "context")) {
		op->type = TEXGEN_TYPE_context;
		op->context.primaryChannel = sexp_argAsInt(sex, 1);
		op->context.op = op_from_sexp(sexp_argAsSexp(sex, 2));
	}
	else if(strcaseeq(name, "seq")) {
		op->type = TEXGEN_TYPE_seq;
		VEC_INIT(&op->seq.ops);
		for(int i = 0; i < VEC_LEN(&sex->args) - 1; i++) {
			
			VEC_PUSH(&op->seq.ops, op_from_sexp(sexp_argAsSexp(sex, 1 + i)));
		}
	}
	else if(strcaseeq(name, "set")) {
		op->type = TEXGEN_TYPE_set;
		op->set.name = strdup(sexp_argAsStr(sex, 1));
		op->set.op = op_from_sexp(sexp_argAsSexp(sex, 2));
	}
	else if(strcaseeq(name, "get")) {
		op->type = TEXGEN_TYPE_get;
		op->get.name = strdup(sexp_argAsStr(sex, 1));
	}
	else if(strcaseeq(name, "squares")) {
		op->type = TEXGEN_TYPE_squares;
		op->squares.grid = sexp_argAsDouble(sex, 1);
		op->squares.size = sexp_argAsDouble(sex, 2);
		op->squares.background = sexp_argAsColor(sex, 3);
		op->squares.color = sexp_argAsColor(sex, 4);
	}
	else if(strcaseeq(name, "normal_map")) {
		op->type = TEXGEN_TYPE_normal_map;
		op->normal_map.index = sexp_argAsInt(sex, 1);
	}
	else if(strcaseeq(name, "checkers")) {
		op->type = TEXGEN_TYPE_checkers;
		op->checkers.grid = sexp_argAsDouble(sex, 1);
		op->checkers.background = sexp_argAsColor(sex, 2);
		op->checkers.color = sexp_argAsColor(sex, 3);
	}
	else if(strcaseeq(name, "blend")) {
		op->type = TEXGEN_TYPE_blend;
		op->blend.a_index = sexp_argAsInt(sex, 1);
		op->blend.b_index = sexp_argAsInt(sex, 2);
		op->blend.t = sexp_argAsDouble(sex, 3);
	}
	else if(strcaseeq(name, "chanmux")) {
		op->type = TEXGEN_TYPE_chanmux;
		op->chanmux.r_i = sexp_argAsInt(sex, 1);
		op->chanmux.r_c = sexp_argAsInt(sex, 2);
		op->chanmux.g_i = sexp_argAsInt(sex, 3);
		op->chanmux.g_c = sexp_argAsInt(sex, 4);
		op->chanmux.b_i = sexp_argAsInt(sex, 5);
		op->chanmux.b_c = sexp_argAsInt(sex, 6);
		op->chanmux.a_i = sexp_argAsInt(sex, 7);
		op->chanmux.a_c = sexp_argAsInt(sex, 8);
	}
	else if(strcaseeq(name, "worley")) {
		op->type = TEXGEN_TYPE_worley;
		op->worley.algorithm = strdup(sexp_argAsStr(sex, 1));
		
		if(strcaseeq(op->worley.algorithm, "random")) {
			op->worley.num_points = sexp_argAsInt(sex, 2);
			op->worley.divisor = sexp_argAsDouble(sex, 3);
		}
		else if(strcaseeq(op->worley.algorithm, "boxed")) {
			op->worley.num_points = sexp_argAsInt(sex, 2);
			op->worley.boxes = sexp_argAsInt(sex, 3);
			op->worley.divisor = sexp_argAsDouble(sex, 4);
		}
		else if(strcaseeq(op->worley.algorithm, "sample")) {
			op->worley.sample_index = sexp_argAsInt(sex, 2);
			op->worley.sample_channel = sexp_argAsInt(sex, 3);
			op->worley.sample_thresh = sexp_argAsDouble(sex, 4);
			op->worley.divisor = sexp_argAsDouble(sex, 5);
		}
		else {
			fprintf(stderr, "Unknown worley noise algorithm: %s\n", op->worley.algorithm);
		}
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




static void run_op(struct tg_context* context, TexGenOp* op) {
	//TexBitmap* out;
	genfn fn;
	printf("op\n");
	fn = generators[op->type];
	if(!fn) {
		fprintf(stderr, "texgen: no generator function for type %d\n", op->type);
		return;
	}
	
	fn(context, &op->solid); // the exact union member does not matter	
}





BitmapRGBA8* TexGen_Generate(char* source, Vector2i size) {
	TexGenContext* tgc;
	TexGenOp* op;
	sexp* sex;
	struct tg_context context;

	
	sex = sexp_parse(source);
	op = op_from_sexp(sex);
	sexp_free(sex);
	
	
	HashTable(FloatTex*) storage;
	HT_init(&storage, 4);
	
	context.w = size.x;
	context.h = size.y;
	context.channels = 4;
	context.storage = &storage;
	context.primaryChannel = 0;
	VEC_INIT(&context.stack);

	run_op(&context, op);
	
	FloatTex* ft = VEC_TAIL(&context.stack);
	BitmapRGBA8* bmp = FloatTex_ToRGBA8(ft);
	
	HT_destroy(&storage, 0);
	
	VEC_EACH(&context.stack, i, f) {
		FloatTex_free(f);
		free(f);
	}
	
	VEC_FREE(&context.stack);
	

	return bmp;
}




static void temptest(GUITexBuilderControl* bc) {
	TexGenContext* tgc;
	TexGenOp* op;
	void* data;
	
	sexp* sex;
	
	char* prog = "" \
		"(seq " \
 		"	(sinewave 30.0 .25)" \
//		"	(worley boxed 4 8 70)" 
		"	(perlin .5 6 16 16 100 100)" \
		//"	(blend 1 2 .5)" 
		//"	(chanmux 0 0  1 0  2 0  -1 0)" 
 		"	(squares 10 .5 #003300 (.2 .4 .1))" 
//  		"	(checkers 10 #663300 (.5 .4 .1))" 
		"	(normal_map 0)" \
		"" \
		")" \
	;
	
	//sex = sexp_parse("(seq (sinewave 3.0 .25) (sinewave 3.0 .25))");
	//sex = sexp_parse("(perlin .1 8 100 100 20 20)");
	sex = sexp_parse(prog);
	bc->op = op_from_sexp(sex);
	sexp_free(sex);
	
	/*
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
	run_op(&context, op);
	//gen_sinewave(&bmp, 0, &op->sinewave);
	
	FloatTex* ft = VEC_TAIL(&context.stack);
	//FloatTex* ft = VEC_ITEM(&context.stack, 4);
	BitmapRGBA8* bmp = FloatTex_ToRGBA8(ft);
	
	
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

	
	GLuint64 texHandle = glGetTextureHandleARB(id);
	glexit("");
	glMakeTextureHandleResidentARB(texHandle);
	glexit("");
	
	bc->im->texHandle = texHandle;
	printf("%p\n", texHandle);
	g_texgenhandle = texHandle;
			
	glBindTexture(GL_TEXTURE_2D, 0);*/
	
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


static void tbc_Regen(GUITexBuilderControl* bc) {
	
	
	
	TexGenContext* tgc;
	void* data;
	
	char* prog = "" \
		"(seq " \
// 		"	(sinewave 30.0 .25)" 
//		"	(worley boxed 4 8 70)" 
		"	(perlin .5 6 16 16 100 100)" \
		//"	(blend 1 2 .5)" 
		//"	(chanmux 0 0  1 0  2 0  -1 0)" 
// 		"	(squares 10 .5 #003300 (.2 .4 .1))" 
//  		"	(checkers 10 #663300 (.5 .4 .1))" 
		"	(normal_map 0)" \
		""  \
		")" \
	;
	;
	sexp* sex = sexp_parse(prog);
	TexGenOp* op = op_from_sexp(sex);
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
	run_op(&context, op);
	//gen_sinewave(&bmp, 0, &op->sinewave);
	
	FloatTex* ft = VEC_TAIL(&context.stack);
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
	GUIHeader_render(bc->bg, pfp);
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
		printf("inserted perlin");
		bc->op = make_op(TEXGEN_TYPE_perlin);
// 		bc->op = make_op(bc->selectedOpIndex);
		bc->op->perlin.persistence = .5;
		bc->op->perlin.octaves = 6;
		bc->op->perlin.spread_x = 16;
		bc->op->perlin.spread_y = 16;
		bc->op->perlin.offset_x = 100;
		bc->op->perlin.offset_y = 100;
		
		GUIStructAdjuster* sa = GUIStructAdjuster_new(bc->header.gm, &bc->op->perlin, TG_perlin_structAdjusterFields); 
		
		
		GUIRegisterObject(sa, bc->bg);
		
	} 

	
	if(ev->character == 'a') { 
		
	}
	if(ev->character == 's') { 
		//temptest(bc);
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
	bc->im->header.topleft = (Vector2){0, 0}; 
	bc->im->header.size =(Vector2){400, 400}; 
	
	GUIRegisterObject(bc->im, &bc->bg->header);
		
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
	
	
	bc->da = GUIDebugAdjuster_new(gm, "test: %d", &bc->selectedOpIndex, 'i');
	GUIRegisterObject(bc->da, &bc->bg->header);

	
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


