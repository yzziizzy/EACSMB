

// generate a normal map based on pixel brightness
void gen_normal_map(struct tg_context* context, struct TG_normal_map* opts) {
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

