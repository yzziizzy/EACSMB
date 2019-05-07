

// #include "../common_gl.h" // shouldn't need this, but utilities.h wants it
#include "../utilities.h"
#include "utils.h"







SoundKernel* SoundKernel_new(int length, int start) {
	SoundKernel* k = pcalloc(k);
	
	k->length = length;
	k->start = start;
	k->data = calloc(1, k->length * sizeof(k->data));
	
	return k;
}



// mix src into dest equally
void SoundClip_mixEq(SoundClip* dest, SoundClip* src, uint64_t destOffset, uint64_t srcOffset) {
	if(!dest || !src) return;
	
	int samples = MIN(dest->numSamples - destOffset, src->numSamples - srcOffset);
	int chans = MIN(dest->channels, src->channels);
	
	for(int i = 0; i < samples; i++) {
		for(int c = 0; c < chans; c++) {
			dest->data[(i + destOffset) * dest->channels + c] += src->data[(i + srcOffset) * src->channels + c];
		}
	}
}



// mix src into dest amplified
void SoundClip_mixWeighted(SoundClip* dest, SoundClip* src, float srcW, uint64_t destOffset, uint64_t srcOffset) {
	if(!dest || !src) return;
	
	int samples = MIN(dest->numSamples - destOffset, src->numSamples - srcOffset);
	int chans = MIN(dest->channels, src->channels);
	
	for(int i = 0; i < samples; i++) {
		for(int c = 0; c < chans; c++) {
			float d = dest->data[(i + destOffset) * dest->channels + c];
			float s = src->data[(i + srcOffset) * src->channels + c] * srcW;
			dest->data[(i + destOffset) * dest->channels + c] = d + s;
		}
	}
}

// change volume in-place
void SoundClip_amplify(SoundClip* clip, float amount) {
	if(!clip) return;
	
	for(int i = 0; i < clip->numSamples; i++) {
		for(int c = 0; c < clip->channels; c++) {
			clip->data[i * clip->channels + c] *= amount;
		}
	}
}




// find the sample with the largest absolute value on any channel 
float SoundClip_getPeak(SoundClip* clip) {
	if(!clip) return 0.0f;
	
	float max = 0.0;
	for(int i = 0; i < clip->numSamples; i++) {
		for(int c = 0; c < clip->channels; c++) {
			float f = clip->data[i * clip->channels + c];
			max = fmax(max, fabs(f));
		}
	}
	
	return max;
}

// find the average absolute sample value on all channel 
float SoundClip_getAverage(SoundClip* clip) {
	if(!clip || clip->numSamples == 0 || clip->channels == 0) return 0.0f;
	
	double avg = 0.0;
	for(int i = 0; i < clip->numSamples; i++) {
		for(int c = 0; c < clip->channels; c++) {
			float f = clip->data[i * clip->channels + c];
			avg += fabs(f);
		}
	}
	
	return avg / ((double)clip->numSamples * (double)clip->channels);
}


// does what it says 
float SoundClip_stripDCOffset(SoundClip* clip) {
	if(!clip || clip->numSamples == 0 || clip->channels == 0) return 0.0f;
	
	double avg = 0.0;
	for(int i = 0; i < clip->numSamples; i++) {
		for(int c = 0; c < clip->channels; c++) {
			float f = clip->data[i * clip->channels + c];
			avg += abs(f);
		}
	}
	
	avg /= ((double)clip->numSamples * (double)clip->channels);
	
	float fa = avg;
	
	for(int i = 0; i < clip->numSamples; i++) {
		for(int c = 0; c < clip->channels; c++) {
			clip->data[i * clip->channels + c] -= fa;
		}
	}
}


// clip everything above a certain volume using atctan
float SoundClip_softClip(SoundClip* clip, float pct) {
	float omp = 1.0f - pct;
	for(int i = 0; i < clip->numSamples; i++) {
		for(int c = 0; c < clip->channels; c++) {
			float f = clip->data[i * clip->channels + c];
			
			if(f > pct) f = pct + atanf(f - pct) * omp / F_PI_2; 
			
			clip->data[i * clip->channels + c] = f;
		}
	}
}


// returns a new clip which is the input convolved by the kernel. the input is unchanged
SoundClip* SoundClip_convolve(SoundClip* in, SoundKernel* k) {
	
	uint64_t ns = k->length + in->numSamples - 1;
	SoundClip* out = SoundClip_new(in->channels, in->sampleRate, ns); 
	
	for(uint64_t s = 0; s < in->numSamples; s++) {
		for(int c = 0; c < in->channels; c++) {
			float f = 0;
			for(int i = k->length - 1; i >= 0; i--) {
				f += in->data[(s + i) * in->channels + c] * k->data[i];
			}
			out->data[s * in->channels + c] = f;
		}
	} 
	
	return out;
}

// convolve one kernel with another and return a new result
SoundKernel* SoundKernel_convolve(SoundKernel* a, SoundKernel* b) {
	// TODO BUG: figure out correct start value
	int ns = a->length + b->length - 1;
	SoundKernel* out = SoundKernel_new(ns, a->start + b->start);
	
	for(uint64_t s = 0; s < a->length; s++) {
		float f = 0;
		for(int i = b->length - 1; i >= 0; i--) {
			f += a->data[s] * b->data[i];
		}
		out->data[s] = f;
	}
	
	return out;
}


// fill part of a clip with white noise
void SoundClip_genNoise(SoundClip* clip, unsigned long startSample, unsigned long endSample, float volume) {
	if(!clip) return;
	
	uint64_t str; // whatever junk is on the stack is the seed
	
	long start = MIN(startSample, clip->numSamples);
	long end = MIN(endSample, clip->numSamples);
	
	for(int i = start; i < end; i++) {
		for(int c = 0; c < clip->channels; c++) {
			
			// add up a bunch of random numbers to approach gaussian distribution
			float f = pcg_f(&str, 1) + pcg_f(&str, 2) + pcg_f(&str, 3) + 
				pcg_f(&str, 4) + pcg_f(&str, 5) + pcg_f(&str, 6);
			
			f /= 3.0f;
			
			clip->data[i * clip->channels + c] = f * volume;
		}
	}
}


static float fft_Wr(float n, float b) {
	return cos(F_2PI * n / b);
}
static float fft_Wi(float n, float b) {
	return sin(F_2PI * n / b);
}



static void fft_bfly(SoundClipFFT* fft, int offset, int stage) {
	
	int pot = 1 << stage; 
	int n = fft->length / pot;
	
	if(n == 0) return;
	
	for(int i = 0; i < n; i++) {
		float wr = fft_Wr(n, pot);
		float wi = fft_Wi(n, pot);
	
		// BUG somethign is wrong with this math, but the internet is 
		//   conflicting and confusing on the matter 
		float r0 = fft->real[offset + i];
		float r1 = fft->real[offset + i + n];
		float i0 = fft->imag[offset + i];
		float i1 = fft->imag[offset + i + n];
		
		
		
		fft->real[offset + i]     = r0 + wr * r1;
		fft->real[offset + i + n] = r0 - wr * r1;
		fft->imag[offset + i]     = i0 + wi * i1;
		fft->imag[offset + i + n] = i0 - wi * i1;
	}
	
	
	
}



// the length MUST be a power of two
SoundClipFFT* SoundClip_FFT(SoundClip* clip, int channel) {
	SoundClipFFT* fft;
	fft = pcalloc(fft);
	
	int len = clip->numSamples;
	int pot = __builtin_clz(len) + 1;
	
	fft->length = len;
	fft->imag = calloc(1, len * sizeof(*fft->imag)); // imaginary
	fft->real = malloc(len * sizeof(*fft->real)); // real
	
	// copy the requested channel into the real array
	for(int i = 0; i < len; i++) {
		int ir = bitReverse32(i) >> (32 - pot);
		fft->real[i] = clip->data[i * clip->channels + channel];
	}
	
	fft_bfly(fft, 0, 1);
	
	return fft;
} 

