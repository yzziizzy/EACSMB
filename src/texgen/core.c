


#include "texgen.h"



#include "core.h"


// static void gen_normal_map(struct tg_context* context, struct TG_normal_map* opts);
// static void gen_blend(struct tg_context* context, struct TG_blend* opts);
// static void gen_chanmux(struct tg_context* context, struct TG_chanmux* opts) ;
// static void gen_seq(struct tg_context* context, struct TG_seq* opts);
// static void gen_set(struct tg_context* context, struct TG_set* opts) ;
// static void gen_get(struct tg_context* context, struct TG_get* opts) ;
// static void gen_context(struct tg_context* context, struct TG_context* opts);
// static void gen_rotate(struct tg_context* context, struct TG_rotate* opts) ;
// static void gen_solid(struct tg_context* context, struct TG_solid* opts);
// static void gen_lerp(struct tg_context* context, struct TG_lerp* opts);
// static void gen_sinewave(struct tg_context* context, struct TG_sinewave* opts);
// static void gen_gradient(struct tg_context* context, struct TG_gradient* opts)
// 
// 


static void tg_context_push(tg_context* c, FloatTex* ft) {
	VEC_PUSH(&c->stack, ft);
}

static FloatTex* FloatTex_fromContext(tg_context* c) {
	return FloatTex_alloc(c->w, c->h, c->channels);
}

static FloatTex* tg_context_index(tg_context* context, int i) {
	return VEC_ITEM(&context->stack, VEC_LEN(&context->stack) - 1 - i);
}



#include "op_util.c"
#include "op_basic.c"
#include "op_noise.c"
#include "op_normalMap.c"



TexGenOp*  make_op(int type) {
	TexGenOp* op = calloc(1, sizeof(*op));
	op->type = type;
	return op;
}

TexGenOp* op_from_sexp(sexp* sex) {
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




void run_op(struct tg_context* context, TexGenOp* op) {
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
	
	printf("stack depth: %d\n", &context.stack);
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






