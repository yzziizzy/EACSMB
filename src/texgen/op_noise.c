
void gen_perlin(struct tg_context* context, struct TG_perlin* opts) {
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
float wrapdist(Vector2 pos, Vector2 p, float w, float h) {
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


float closest(Vector2* points, int plen, Vector2 pos, float w, float h) {
	int i;
	float cdist = 9999999999;

	for(i = 0; i < plen; i++) {
		float d = wrapdist(pos, *(points + i), w, h);
		cdist = fmin(cdist, d);
	}
	
	return cdist;
}

float second_closest(Vector2* points, int plen, Vector2 pos, float w, float h) {
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


void gen_worley(struct tg_context* context, struct TG_worley* opts) {
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
