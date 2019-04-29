#ifndef __EACSMB_sound_soundgen_h__
#define __EACSMB_sound_soundgen_h__

#include "../sound.h"

#include "ds.h"
#include "hash.h"




typedef struct SoundGenSource {
	char type; // 'c', 'i', 'n', '0'=empty
	union {
		SoundClip* clip;
		int index;
		char* name;
	};
} SoundGenSource;



struct base_sound_opts {
	long numSamples;
	int sampleRate;
	int channels;
};


enum SoundGenWaveType {
	SG_WAVE_SINE,
	SG_WAVE_SQUARE,
	SG_WAVE_TRIANGLE,
	SG_WAVE_SAWTOOTH,
};

struct wave_opts {
	struct base_sound_opts baseOpts;
	enum SoundGenWaveType type;
	float phase;
	float magnitude;
	float frequency;
};


enum SoundGenFadeType {
	SG_FADE_LINEAR,
	SG_FADE_SINE,
	SG_FADE_LOG,
};

struct fade_opts {
	struct base_sound_opts baseOpts;
	SoundGenSource source;
	enum SoundGenFadeType type;
	long startSample;
	float startMag;
	long endSample;
	float endMag;
	char clampStart;
	char clampEnd;
};



enum SoundGenDistortType {
	SG_DISTORT_CLIP,
	SG_DISTORT_BITCRUSH,
};

struct distort_opts {
	struct base_sound_opts baseOpts;
	SoundGenSource source;
	enum SoundGenDistortType type;
	
};

struct reverb_opts {
	struct base_sound_opts baseOpts;
	SoundGenSource source;
};

struct delay_opts {
	struct base_sound_opts baseOpts;
	SoundGenSource source;
	long delaySamples;
};


enum SoundGenNoiseType {
	SG_NOISE_WHITE,
};

struct noise_opts {
	struct base_sound_opts baseOpts;
	enum SoundGenNoiseType type;
};


struct mux_opts {
	struct base_sound_opts baseOpts;
	SoundGenSource sources[8];
	unsigned char channels[8]; // 128 for no output channel
};

// mix all channels straight down, according tot he weights
struct mix_opts {
	struct base_sound_opts baseOpts;
	SoundGenSource sources[8];
	float weights[8];
};



typedef struct SoundGenContext { 
	
	HashTable(SoundClip*) clips;
	VEC(SoundClip*) stack;
	
	int sampleRate;
	long numSamples;
	
	
} SoundGenContext;




SoundGenContext* SoundGenContext_alloc();
void SoundGenContext_init(SoundGenContext* sgc);

SoundClip* SoundGen_genTest();






#endif // __EACSMB_sound_soundgen_h__
