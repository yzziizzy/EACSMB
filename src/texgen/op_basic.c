


// squares in a grid
void gen_squares(struct tg_context* context, struct TG_squares* opts) {
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
void gen_checkers(struct tg_context* context, struct TG_checkers* opts) {
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





float FloatTex_sampleGrayscale(FloatTex* ft, int x, int y) {
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


// broken?
void gen_gradient(struct tg_context* context, struct TG_gradient* opts) {
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



void gen_sinewave(struct tg_context* context, struct TG_sinewave* opts) {

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

