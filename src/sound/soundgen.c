


#include "soundgen.h"

#include "../utilities.h"
#include "../sexp.h"


static void* SoundClip_alloc(int a, int b, int c) { printf("\n!!!!NIH\n"); return NULL; }



static SoundClip* get_source(SoundGenSource* src, SoundGenContext* ctx) {
	if(src->type == 'c') {
		return src->clip;
	}
	else if(src->type == 'i') {
		// TODO: oob check
		return VEC_ITEM(&ctx->stack, src->index);
	}
	else if(src->type == 'n') {
		SoundClip* sc;
		// TODO: error handling
		HT_get(&ctx->clips, src->name, &sc);
		return sc;
	}
	else {
		fprintf(stderr, "Unknown source type in soundgen: '%c'\n", src->type);
		return NULL;
	}
}


// single channel only, mux if you need more
static SoundClip* gen_sine_wave(SoundGenContext* ctx, struct wave_opts* opts) {
	long ns = opts->baseOpts.numSamples;
	
	SoundClip* clip = SoundClip_new(
		opts->baseOpts.channels, 
		opts->baseOpts.sampleRate, 
		ns
	); 
	
	float off = opts->phase;
	float sfreq = opts->frequency / opts->baseOpts.sampleRate;
	int channels = opts->baseOpts.channels;
	
	for(long i = 0; i < ns; i++) {
		float f = sin(fmodf(off + (i * sfreq * F_2PI), F_2PI));
		for(int c = 0; c < channels; c++)
			clip->data[i * channels + c] = f * opts->magnitude; 
	}
	
	return clip;
}




static SoundClip* gen_linear_fade(SoundGenContext* ctx, struct fade_opts* opts) {
	long ns = opts->baseOpts.numSamples;
	
	SoundClip* in = get_source(&opts->source, ctx);
	SoundClip* out = SoundClip_new(
		in->channels,
		in->sampleRate, 
		in->numSamples 
	); 
	
	int chans = in->channels;
	
	if(opts->clampStart) {
		// scale the samples
		for(long i = 0; i < opts->startSample; i++) {
			for(int c = 0; c < chans; c++) {
				out->data[i * chans + c] = in->data[i * chans + c] * opts->startMag;
			}
		}
	}
	else {
		// copy the samples
		memcpy(out->data, in->data, chans * opts->startSample * sizeof(*out->data));
	}
	
	if(opts->type == SG_FADE_LINEAR) {
		double delta = (opts->endMag - opts->startMag) / (opts->endSample - opts->startSample);
		
		long from, to;
		long n, dn;
		
		if(opts->startMag < opts->endMag) {
			n = 0;
			dn = 1;
		}
		else {
			n = ns;
			dn = -1;
		}
		
		for(long i = opts->startSample; i <= opts->endSample; i++, n += dn) {
			for(int c = 0; c < chans; c++) {
				out->data[i * chans + c] += in->data[i * chans + c] * delta * n; 
			}
		}

	}
	else if(opts->type == SG_FADE_SINE) {
		// TODO: implement
	}
	else if(opts->type == SG_FADE_LOG) {
		// TODO: implement
	}
	else {
		fprintf(stderr, "unknown fade type in soundgen: %d\n", opts->type);
	}
	
	// copy/clamp the end
	if(opts->clampEnd) {
		// scale the samples
		for(long i = opts->endSample + 1; i < out->numSamples; i++) {
			for(int c = 0; c < chans; c++) {
				out->data[i * chans + c] = in->data[i * chans + c] * opts->endMag;
			}
		}
	}
	else {
		// copy the samples
		// TODO: fn for this to eliminate errors
// 		memcpy(out->data, in->data, chans * opts->startSample * sizeof(*out->data));
	}
	
	return out;
}



static SoundClip* gen_mux(SoundGenContext* ctx, struct mux_opts* opts) {
	int chans = 0;
	SoundClip* ins[8];
	
	for(int i = 0; i < 8; i++) {
		if(opts->channels[i] > 8) {
			chans = i;
			break;
		}
		
		ins[i] = get_source(&opts->sources[i], ctx);
	}
	
	if(chans < 1) {
		fprintf(stderr, "Insufficient channels in soundgen mux\n");
		return NULL;
	}
	
	
	
	SoundClip* out = SoundClip_alloc(
		opts->baseOpts.numSamples, 
		opts->baseOpts.sampleRate, 
		chans
	); 
	
	
	for(int i = 0; i < ctx->numSamples; i++) {
		for(int c = 0; c < chans; c++) {
			out->data[i * chans + c] = ins[c]->data[i * chans + opts->channels[c]];
		}
	}
	
	return out;
}



SoundClip* gen_noise(SoundGenContext* ctx, struct noise_opts* opts) {
	uint64_t str; // whatever junk is on the stack is the seed
	int chans = opts->baseOpts.channels;
	
	SoundClip* out = SoundClip_new(
		opts->baseOpts.channels, 
		opts->baseOpts.sampleRate, 
		opts->baseOpts.numSamples
	); 
	
	
	for(int i = 0; i < out->numSamples; i++) {
		for(int c = 0; c < chans; c++) {
			
			// add up a bunch of random numbers to approach gaussian distribution
			float f = pcg_f(&str, 1) + pcg_f(&str, 2) + pcg_f(&str, 3) + 
				pcg_f(&str, 4) + pcg_f(&str, 5) + pcg_f(&str, 6);
			
			f /= 3.0f;
			
			out->data[i * chans + c] = f;
		}
	}
	
	return out;
}


SoundClip* gen_mix(SoundGenContext* ctx, struct mix_opts* opts) {
	int chans = 0;
	int sourceCnt;
	int maxSamples;
	SoundClip* ins[8];
	
	for(int i = 0; i < 8; i++) {
		ins[i] = get_source(&opts->sources[i], ctx);
		if(!ins[i]) {
			sourceCnt = i;
			break;
		}
		
		chans = MAX(chans, ins[i]->channels);
		maxSamples = MAX(maxSamples, ins[i]->numSamples);
	}
	
	SoundClip* out = SoundClip_new(
		chans,
		opts->baseOpts.sampleRate, 
		maxSamples 
	); 
	
	
	for(int s = 0; s < sourceCnt; s++) {
		
		SoundClip* src = ins[s];
		float w = opts->weights[s];
		
		for(int i = 0; i < src->numSamples; i++) {
			for(int c = 0; c < src->channels; c++) {
				out->data[i * chans + c] += src->data[i * src->channels + c] * w;
			}
		}
	}
	
	return out;
}

SoundClip* gen_envelope(SoundGenContext* ctx, struct envelope_opts* opts) {
	
	SoundClip* src = get_source(&opts->source, ctx);
	SoundClip* out = SoundClip_new(
		src->channels, 
		src->sampleRate, 
		src->numSamples
	); 
	
	int s = 0;
	
	float smag = opts->startMag;
	float emag;
	
	for(s = 0; s < 8; ) {
		
		emag = opts->segments[s].endMag;
		float dmag = (emag - smag) / opts->segments[s].numSamples;  
		
		for(int i = 0; i < opts->segments[s].numSamples; i++) {
			float w = smag + (dmag * i);
			
			for(int c = 0; c < src->channels; c++) {
				out->data[i * src->channels + c] = src->data[i * src->channels + c] * w;
			}
		}
		
		s++;
		
		smag = emag;
		emag = opts->segments[s].endMag;
		
		if(opts->segments[s].numSamples < 0) break;
	}
	
	
	return out;
}


// SoundClip* gen_note(SoundGenContext* ctx, struct note_opts* opts) {
// 	
// 	
// 	
// }



SoundGenContext* SoundGenContext_alloc() {
	SoundGenContext* sgc = pcalloc(sgc);
	SoundGenContext_init(sgc);
	return sgc;
}


void SoundGenContext_init(SoundGenContext* sgc) {
	HT_init(&sgc->clips, 4);
}



enum sg_op_type {
	SGO_ARG = 0, // not a true operation
	SGO_SET,
	SGO_WAVE,
	SGO_FADE,
	SGO_DISTORT,
	SGO_REVERB,
	SGO_DELAY,
	SGO_NOISE,
	SGO_MUX,
	SGO_MIX,
	
	SGO_UNKNOWN
};



typedef struct sg_op_node {
	enum sg_op_type type; //
	char* argStr;
	
	VEC(struct sg_op_node*) args;
} sg_op_node;


static sg_op_node* op_new(enum sg_op_type type) {
	sg_op_node* n = pcalloc(n);
	n->type = type;
	return n;
}


static sg_op_node* opFromStr(const char* s) {
	sg_op_node* n = op_new(0);
	
	if(strcaseeq(s, "set")) n->type = SGO_SET;
	else if(strcaseeq(s, "wave")) n->type = SGO_WAVE;
	else if(strcaseeq(s, "sine")) {
		n->type = SGO_WAVE;
// 		VEC_PUSH(&n->args, "SINE");
	}
	else if(strcaseeq(s, "fade")) n->type = SGO_FADE;
	else if(strcaseeq(s, "distort")) n->type = SGO_DISTORT;
	else if(strcaseeq(s, "reverb")) n->type = SGO_REVERB;
	else if(strcaseeq(s, "delay")) n->type = SGO_DELAY;
	else if(strcaseeq(s, "noise")) n->type = SGO_NOISE;
	else if(strcaseeq(s, "mux")) n->type = SGO_MUX;
	else if(strcaseeq(s, "mix")) n->type = SGO_MIX;
	else n->type = SGO_UNKNOWN;
	
	return n;
}




static sg_op_node* arg_from_sexp(sexp* sex) {
	sg_op_node* op = op_new(SGO_ARG);
	
	if(sex->type == 1) {
		op->argStr = sex->str;
		return op;
	}
	
	VEC_EACH(&sex->args, ai, a) {
		VEC_PUSH(&op->args, arg_from_sexp(a));
	}
	
	return op;
}


static sg_op_node* op_from_sexp(sexp* sex) {
	sg_op_node* op;
	char* name;
	
	if(sex->brace == '[') { //straight args
		return arg_from_sexp(sex);
	}
	else if(sex->brace == '(') { // function 
		
		name = sexp_argAsStr(sex, 0);
		
		op = opFromStr(name);
	}
	
	
	return op;
} 







void SoundGen_FromString(char* source) {
	sexp* s = sexp_parse(source);
	
	
	
}




SoundClip* SoundGen_genTest() {
	SoundGenContext* ctx = SoundGenContext_alloc();
	
	struct wave_opts opts = {
		.baseOpts = {
			.numSamples = 44100 * 5,
			.sampleRate = 44100,
			.channels = 2,
		},
		.type = SG_WAVE_SINE,
		.phase = 0,
		.magnitude = .4,
		.frequency = 260,
	};
	
	
	SoundClip* sine = gen_sine_wave(ctx, &opts);
	
	struct fade_opts fopts = {
		.baseOpts = opts.baseOpts,
		.source = {
			.type = 'c',
			.clip = sine,
		},
		.type = SG_FADE_LINEAR,
		.startSample = 0,
		.startMag = 1.0,
		.endSample = 44100 * 5,
		.endMag = 0,
		0,0
	};
	
	SoundClip* faded = gen_linear_fade(ctx, &fopts);
	
	
	struct noise_opts nopts = {
		.baseOpts = opts.baseOpts,
		.type = SG_NOISE_WHITE,
	};
	
	SoundClip* noise = gen_noise(ctx, &nopts);
	
	
	struct mix_opts mopts = {
		.baseOpts = opts.baseOpts,
		.sources = {
			{ .type = 'c', .clip = sine },
			{ .type = 'c', .clip = noise },
			{NULL},
		},
		.weights = { .7, .08},
	};
	
	SoundClip* mixed = gen_mix(ctx, &mopts);
	
	
	
	return mixed;
}



static float noteHDist(char n, int octave, char sharpFlat) {
	int n1;
	switch(n) {
		case 'A': n1 =   0; break; 
		case 'B': n1 =   2; break;
		case 'C': n1 = -10; break;
		case 'D': n1 =  -8; break;
		case 'E': n1 =  -6; break;
		case 'F': n1 =  -4; break;
		case 'G': n1 =  -2; break;
	}
	
	if(sharpFlat == 'b') n--;
	else if(sharpFlat == '#') n++;
	
	return ((octave - 4) * 12) + n1;  
}


// A4 at 440hz.
static float hdFreq(float halfDist) {
	return 440.0 * powf(_12root2, halfDist);
}


static float noteFreq(char n, int octave, char sharpFlat) {
	float n2 = noteHDist(n, octave, sharpFlat);  
	return hdFreq(n2);
}
