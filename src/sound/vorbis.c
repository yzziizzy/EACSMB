#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vorbis.h"

#include "../utilities.h"
#include "../sound.h"

#define HAS_LIBVORBISFILE 1
#if HAS_LIBVORBISFILE


#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>




struct SoundClip* SoundClip_fromVorbis(char* path) {
	SoundClip* sc;
	OggVorbis_File vf;
	vorbis_info* vi;
	int section = 0;
	
	if(0 != ov_fopen(path, &vf)) {
		return NULL;
	}
	   
	pcalloc(sc);
	
	// this memory apparently does not need to be cleaned up
	vi = ov_info(&vf, -1);
	
	sc->channels = 2; // force two channels atm
	sc->sampleRate = vi->rate; // files with weird rates are resampled later
	
	sc->numSamples = ov_pcm_total(&vf, -1);
	sc->length = (float)sc->numSamples / (float)sc->sampleRate;
	
	sc->data = malloc(sc->numSamples * sizeof(*sc->data) * sc->channels);
	
	int lindex, rindex;
	switch(vi->channels) {
		case 1: lindex = 0; rindex = 0; break;
		case 2: lindex = 0; rindex = 1; break;
		case 3: lindex = 0; rindex = 2; break;
		case 4: lindex = 0; rindex = 1; break;
		case 5: lindex = 0; rindex = 2; break;
		case 6: lindex = 0; rindex = 2; break;
		case 7: lindex = 0; rindex = 2; break;
		default: lindex = 0; rindex = 2; break;
	}
	
	// read the data
	float** buf;
	long offset = 0;
	
	while(1) {
		int ret = ov_read_float(&vf, &buf, 1024, &section);
		if(ret == 0) break;
		if(ret == OV_HOLE) continue;
		if(ret == OV_EBADLINK || ret == OV_EINVAL) goto BAIL;
		
		float* lbuf = buf[lindex];
		float* rbuf = buf[rindex];
		
		for(int i = 0; i < ret; i++) {
			sc->data[((i + offset) * sc->channels) + 0] = lbuf[i];
			sc->data[((i + offset) * sc->channels) + 1] = rbuf[i];
		}
		
		offset += ret;
	}
	
	ov_clear(&vf);
	
	return sc;
	
	
BAIL:
	free(sc->data);
	free(sc);
	ov_clear(&vf);
	
	return NULL;
}


#else // HAS_LIBVORBISFILE

struct SoundClip* SoundClip_loadVorbis(char* path) {
	fprintf(stderr, "Attempted to load an OGG/Vorbis file without OGG/Vorbis support compiled in. (%s)\n", path);
	return NULL;
}

#endif




