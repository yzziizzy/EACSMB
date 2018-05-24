#ifndef __EACSMB_opensimplex_h__
#define __EACSMB_opensimplex_h__

#include <stdint.h>





typedef struct {
	int64_t seed;
	
	int width, height;
	
	int16_t perm[256];
	int16_t permGradIndex3D[256];
} OpenSimplexNoise;


typedef struct {
	double divisor;
	float blendWeight;
} OpenSimplexOctave;

typedef struct {
	int w, h;
	int offsetX, offsetY; // not yet implemented
	OpenSimplexOctave* octaves;
} OpenSimplexParams;


void OpenSimplex_init(OpenSimplexNoise* osn, int64_t seed, int width, int height);

float* OpenSimplex_GenNoise2D(OpenSimplexNoise* osn, OpenSimplexParams* params);

void OpenSimplex_init2D(OpenSimplexNoise* osn, int64_t seed);
void OpenSimplex_init3D(OpenSimplexNoise* osn, int64_t seed);



#endif // __EACSMB_opensimplex_h__
