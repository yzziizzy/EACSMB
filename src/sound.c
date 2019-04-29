#include <stdio.h>
#include <stdlib.h>
#include <math.h>



#include <alsa/asoundlib.h>


#include "sound.h"
#include "utilities.h"











SoundManager* SoundManager_alloc() {
	
	SoundManager* sm;
	pcalloc(sm);
	
	HT_init(&sm->clips, 4);
	VEC_INIT(&sm->instances);
	
	sm->sampleRate = 44100;
	sm->deviceName = "default";
	sm->channels = 2;
	sm->bufferLen = 512;
	
	sm->globalVolume = 0.8f;
	
	sm->lastMixTime = -1; // sigil for initialization
	
	// temp
	sm->alsabuf = calloc(1, sizeof(int16_t) * 2 * 4096);
	
	return sm;
}


int SoundManager_reallocBuffer(SoundManager* sm, int newLen) {
	size_t len = sizeof(*sm->buffer) * sm->channels * newLen;
	
	if(sm->buffer) {
		void* tmp = realloc(sm->buffer, len);
		if(!tmp) {
			printf("!!! failed to allocate sound mixer buffer\n");
			return 1;
		}
		sm->buffer = tmp;
	}
	else {
		sm->buffer = calloc(1, len);
	}
	
	sm->bufferLen = newLen;
	
	return 0;
}



void SoundManager_start(SoundManager* sm) {
	snd_pcm_hw_params_t* hw_params;
	int err;
	
	// openthe device
	if((err = snd_pcm_open(&sm->playback_handle, sm->deviceName, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to open pcm device '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}

	// set up hw_params
	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to allocate hw_params for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}
	if((err = snd_pcm_hw_params_any(sm->playback_handle, hw_params)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to init hw_params for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}
	
	// device access
	if((err = snd_pcm_hw_params_set_access(sm->playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to set access type for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}

	// set format and sample rate
	if((err = snd_pcm_hw_params_set_format(sm->playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to set sample format for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}
	if((err = snd_pcm_hw_params_set_rate_near(sm->playback_handle, hw_params, &sm->sampleRate, 0)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to set sample rate for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}
	if((err = snd_pcm_hw_params_set_channels(sm->playback_handle, hw_params, sm->channels)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to set channel count for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}

	// configure device
	if((err = snd_pcm_hw_params(sm->playback_handle, hw_params)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed to set hw params for '%s' (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}
	
	snd_pcm_hw_params_free(hw_params);
	
	if((err = snd_pcm_prepare(sm->playback_handle)) < 0) {
		fprintf(stderr, "!!![ALSA] Failed prepare device '%s' for playback (%s)\n", sm->deviceName, snd_strerror(err));
		return;
	}
	
	
	// set up the internal software mixer
	if(SoundManager_reallocBuffer(sm, sm->bufferLen)) {
		return;
	}
	
	
	// read to go
	sm->status = 1;
}



void SoundManager_addClip(SoundManager* sm, SoundClip* sc, char* name) {
	if(!sc) return;
	HT_set(&sm->clips, name, sc);
}
void SoundManager_addInstance(SoundManager* sm, SoundInstance* si) {
	if(!si) return;
	VEC_PUSH(&sm->instances, si);
}

void SoundManager_addClipInstance(SoundManager* sm, char* clipName, SoundInstance* si) {
	SoundClip* sc;
	if(HT_get(&sm->clips, clipName, &sc)) return;
	si->clip = sc;
	
	SoundManager_addInstance(sm, si);
}


// start index for the current timestamp
static double getCurrentBufferIndex(SoundManager* sm) {
	
}



// this function assumes the clip overlaps the buffer
static void mixClip(SoundManager* sm, SoundInstance* si) {
	SoundClip* sc = si->clip;
	
	
	
#if defined(EACSMB_USE_SIMD)
	
	
#else
	int start = ;
	
	for(int i; i <  ) {
		sm->buffer[i]
		
	}
	
	
#endif
/*	
	// update write pointer
	if(si->flags & SOUNDFLAG_LOOP) {
		si->lastSampleWritten = (si->lastSampleWritten + n) % sc->numSamples;
	}
	else {
		si->lastSampleWritten = (si->lastSampleWritten + n);
	}
	*/
} 



void SoundManager_tick(SoundManager* sm, double newGlobalTime) {
	if(sm->lastMixTime == -1) { // sigil for initialization
		sm->lastMixTime = newGlobalTime;
	}
	
	int buflen = MIN(4096, snd_pcm_avail_update(sm->playback_handle));
	
	// clear the buffer
	for(int i = 0; i < buflen; i++) {
		sm->alsabuf[i * 2 + 0] = 0;
		sm->alsabuf[i * 2 + 1] = 0;
	}
	
	// mix in the sound
	VEC_EACH(&sm->instances, ind, si) {
		SoundClip* sc = si->clip;
		int n;
		
		// skip clips queued in the future
		if(si->globalStartTime > newGlobalTime) continue;
		
		// just in case a finished one slips through
		if(si->lastSampleWritten >= sc->numSamples) continue;
		
		// determine how many samples to write
		if(si->flags & SOUNDFLAG_LOOP) {
			n = buflen;
		}
		else {
			n = MIN(buflen, sc->numSamples - si->lastSampleWritten);
		}
		
		// TODO: mix into float intermediate buffer first
		// add to buffer
		for(int i = 0; i < n; i++) {
			for(int c = 0; c < 2; c++) {
				sm->alsabuf[i * 2 + c] = sc->data[((si->lastSampleWritten + i) % sc->numSamples) * 2 + c] * 32768;
			}
		}
	
		// update write pointer
		if(si->flags & SOUNDFLAG_LOOP) {
			si->lastSampleWritten = (si->lastSampleWritten + n) % sc->numSamples;
		}
		else {
			si->lastSampleWritten = (si->lastSampleWritten + n);
		}
	}
	
	// send the buffer to alsa
	snd_pcm_writei(sm->playback_handle, sm->alsabuf, buflen);
	
	
	// delete finished instances
	VEC_EACH(&sm->instances, ind, si) {
		if(si->lastSampleWritten >= si->clip->numSamples) {
			VEC_RM(&sm->instances, ind);
			free(si);
		}
	}
	
}


void SoundManager_readConfigFile(SoundManager* sm, char* path) {
	json_file_t* jsf;
	
	jsf = json_load_path(path);
	SoundManager_readConfigJSON(sm, jsf->root);
	
	// TODO: free json file
}



void SoundManager_readConfigJSON(SoundManager* sm, json_value_t* jo) {
	json_value_t* v, *j_sys;  
	
	if(jo->type != JSON_TYPE_OBJ) {
		printf("invalid sound config format\n");
		return;
	}
	
	
	sm->channels = json_obj_get_int(jo, "channels", sm->channels);
	sm->bufferLen = json_obj_get_int(jo, "bufferLength", sm->bufferLen);
	sm->sampleRate = json_obj_get_int(jo, "sampleRate", sm->sampleRate);
	
	// for when multiple audio API's are supported 
	//sm->channels = json_obj_get_string(jo, "default", sm->channels);
	
	// try to grab alsa information
	if(json_obj_get_key(jo, "alsa", &j_sys)) {
		printf("found alsa config info\n");
		
		// BUG : memleak, strdup of empty string, restrudup of default value
		//sm->deviceName = strdup(json_obj_get_string(jo, "default", sm->deviceName));
		
// 		sm-> = json_obj_get_int(jo, "bitDepth", sm->);
// 		sm->sampleRate = json_obj_get_int(jo, "rate", sm->sampleRate);
	}
	
	// TODO: pulse, JACK, etc
	
}

void SoundManager_shutdown(SoundManager* sm) {
	snd_pcm_close(sm->playback_handle);
}





static int samplesInRange(int rate, double start, double stop) {
	double d = stop - start;
	d *= rate;
	
	return d;
}

static int samplesBufferAvailable(SoundManager* sm) {
	
	
}


static void mix_to_buffers(SoundManager* sm, double toFuture) {
	int i;
	int numSamplesToMix = samplesInRange(sm->sampleRate, sm->lastMixTime, sm->lastMixTime + toFuture);
	
	int startIndex = sm->bufferWriteHead;
	
	for(i = 0; i < numSamplesToMix; i++) {
		
		
	}
	
	
}




SoundClip* SoundClip_fromWAV(char* path) {
	size_t flen;
	int datalen;
	
	char* riff = readFileRaw(path, &flen);
	
	//check the riff header
	if(0 != strncmp(riff, "RIFF", 4)) {
		printf("!!! '%s' is not a valid WAV file\n", path);
		goto FAIL_1;
	}
	
	unsigned int rtlen = *((uint32_t*)(riff + 4));
	printf("riff len: %d\n", rtlen );
	
	if(0 != strncmp(riff + 8, "WAVE", 4)) {
		printf("!!! '%s' is not a valid WAV file.\n", path);
		goto FAIL_1;
	}
	
	char* cursor = riff + 8 + 4;
	char* chunkBase = cursor;
	// read fmt chunk
	
	if(0 != strncmp(cursor, "fmt ", 4)) {
		printf("!!! '%s' is missing mandatory WAV fmt chunk\n", path);
		goto FAIL_1;
	}
	cursor += 4;
	
	unsigned int fmtlen = *((uint32_t*)(cursor));
	cursor += 4;
	
	struct {
		int16_t FormatTag; // 1 == pcm, 3 == 32-bit float pcm
		uint16_t Channels;
		uint32_t SamplesPerSec;
		uint32_t AvgBytesPerSec;
		uint16_t BlockAlign;
		uint16_t BitsPerSample;
	}* fmt = cursor;
	
	if(fmt->FormatTag != 1 && fmt->FormatTag != 3) {
		printf("!!! WAVE Format %d is not supported\n", (int)fmt->FormatTag);
		goto FAIL_1;
	}
	
	SoundClip* sc = pcalloc(sc);
	
	
	
	// next chunk
	chunkBase += fmtlen + 8;
	
	// find the data chunk
	while(cursor < riff + rtlen) {
		if(0 == strncmp(chunkBase, "data", 4)) 
			goto FOUND_DATA;
		
		chunkBase += *((uint32_t*)(chunkBase + 4)) + 8;
	}
	// ran out of file
	printf("!!! EOF while looking for WAVE data chunk\n");
	goto FAIL_2;
FOUND_DATA:
	
	datalen = *((uint32_t*)(chunkBase + 4));
	cursor = chunkBase + 8;
	
	int lindex, rindex;
	lindex = 0;
	rindex = sc->channels == 1 ? 0 : 1;
	
	// grab the data
	if(fmt->FormatTag == 3) { //32-bit float
		sc->numSamples = datalen / fmt->BlockAlign;
		sc->channels = 2;// fmt->Channels;  pin it to 2 for now
		sc->sampleRate = fmt->SamplesPerSec;
		sc->length = (float)sc->numSamples / (float)sc->sampleRate;
		sc->data = calloc(1, sc->numSamples * sc->channels * sizeof(*sc->data));
		
		for(int i = 0; i < sc->numSamples; i++) {
// 			for(int c = 0; c < fmt->Channels; c++) {
				sc->data[i * sc->channels + 0] = *((float*)(cursor + (i * fmt->BlockAlign) + (lindex * sizeof(float))));
				sc->data[i * sc->channels + 1] = *((float*)(cursor + (i * fmt->BlockAlign) + (rindex * sizeof(float))));
// 			}
		}
		
	}
	else if(fmt->FormatTag == 1) { // integer
		printf("!!! integer WAV not implemented. use 32bit float\n");
		goto FAIL_2;
	}
	
	
	free(riff);
	
	return sc;
	
FAIL_2:
	free(sc);
FAIL_1:
	if(riff) free(riff);
	
	return NULL;
}




void SoundClip_resample(SoundClip* sc, int newRate) {
	if(newRate == sc->sampleRate) return;
	
	int newNumSamples = ceil(sc->length * newRate);
	float* newData = calloc(1, newNumSamples * sc->channels * sizeof(*newData));
	
	if(newRate > sc->sampleRate) { // up-sampling
		
		float rateDiff = (float)newRate / (float)sc->sampleRate;
		
		for(int i = 0; i < newNumSamples; i++) {
			for(int c = 0; c < sc->channels; c++) {
				// indices
				int li = floor(i * rateDiff);
				int hi = li + 1;
				
				// samples
				float l = sc->data[li * sc->channels + c];
				float h = sc->data[hi * sc->channels + c];
				
				// weights
				float hw = i * rateDiff;
				hw = hw - (long)hw; // fract
				float lw = 1 - hw;
				
				float f = ((l * lw) + (h * hw)) / 2;
				newData[i * sc->channels + c] = f;
			}
		}
	}
	else { // downsampling, need fancy averages
		printf("!!! downsampling is not implemented\n");
		free(newData);
		return;
	}
	
	sc->sampleRate = newRate;
	sc->numSamples = newNumSamples;
	free(sc->data);
	sc->data = newData;
	sc->length = (float)sc->numSamples / (float)sc->sampleRate;
}



SoundClip* SoundClip_new(int channels, int sampleRate, uint64_t numSamples) {
	SoundClip* sc = pcalloc(sc);
	
	// aligned and rounded up to 16 bytes for AVX
	int64_t align = 16;
	
	// only works when align is a power of two
	int64_t sz = (numSamples + align - 1) & -align;
	
	sc->data = aligned_alloc(align, sz * channels * sizeof(*sc->data));
	sc->numSamples = sz;
	sc->sampleRate = sampleRate;
	sc->channels = channels;
	
	sc->length = (float)numSamples / (float)sampleRate;
	sc->defaultVolume = 1.0f;
	
	return sc;
}


SoundClip* SoundClip_like(SoundClip* proto) {
	return SoundClip_new(proto->channels, proto->sampleRate, proto->numSamples);
}








