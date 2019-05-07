#ifndef __EACSMB_sound_utils_h__
#define __EACSMB_sound_utils_h__


#include "../sound.h"



typedef struct SoundKernel {
	float* data;
	int length;
	int start; // start offset, allowing for negative indices
} SoundKernel;

SoundKernel* SoundKernel_new(int length, int start);


// mix src into dest equally
void SoundClip_mixEq(SoundClip* dest, SoundClip* src, uint64_t destOffset, uint64_t srcOffset);

// mix src into dest amplified
void SoundClip_mixWeighted(SoundClip* dest, SoundClip* src, float srcW, uint64_t destOffset, uint64_t srcOffset);

// change volume in-place
void SoundClip_amplify(SoundClip* clip, float amount);

// find the sample with the largest absolute value on any channel 
float SoundClip_getPeak(SoundClip* clip);

// find the average absolute sample value on all channel 
float SoundClip_getAverage(SoundClip* clip);

// does what it says 
float SoundClip_stripDCOffset(SoundClip* clip);

// clip everything above a certain volume using atctan
float SoundClip_softClip(SoundClip* clip, float pct);

// returns a new clip which is the input convolved by the kernel. the input is unchanged
SoundClip* SoundClip_convolve(SoundClip* in, SoundKernel* k);

// convolve one kernel with another and return a new result
SoundKernel* SoundKernel_convolve(SoundKernel* a, SoundKernel* b);

// fill part of a clip with white noise
void SoundClip_genNoise(SoundClip* clip, unsigned long startSample, unsigned long endSample, float volume);



// TODO: channel mixing


typedef struct SoundClipFFT {
	float* imag; // sines
	float* real; // cosines
	int length;
} SoundClipFFT; 


#endif // __EACSMB_sound_utils_h__
