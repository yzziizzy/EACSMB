
// changes the context
void gen_context(struct tg_context* context, struct TG_context* opts) {

}


void gen_solid(struct tg_context* context, struct TG_solid* opts) {
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



void gen_lerp(struct tg_context* context, struct TG_lerp* opts) {
	FloatTex* ft = VEC_TAIL(&context->stack);
	
	
	
	
	tg_context_push(context, ft);
}

void gen_rotate(struct tg_context* context, struct TG_rotate* opts) {
}

void gen_get(struct tg_context* context, struct TG_get* opts) {
	FloatTex* ft;
	//HT_get(context->storage, opts->name, &ft);
	VEC_PUSH(&context->stack, ft);
}

void gen_set(struct tg_context* context, struct TG_set* opts) {
	
	FloatTex* ft = VEC_TAIL(&context->stack);
	//HT_set(context->storage, opts->name, ft); 
}


void gen_seq(struct tg_context* context, struct TG_seq* opts) {
	int i;
	
	
	for(i = 0; i < VEC_LEN(&opts->ops); i++) {
		run_op(context, VEC_ITEM(&opts->ops, i));
	}
	
}

void gen_chanmux(struct tg_context* context, struct TG_chanmux* opts) {
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




#define pixel(ft, x, y, c) ((ft)->bmps[(c)]->data[((y) * (ft)->w) + (x)])

void gen_blend(struct tg_context* context, struct TG_blend* opts) {
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

