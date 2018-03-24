#ifndef __EACSMB_PERLIN_H__
#define __EACSMB_PERLIN_H__

#include <stdint.h>





float PerlinNoise_2D(float x, float y, float persistence, int octaves);



typedef struct PerlinParams {
	float persistence;
	int octaves;
	uint64_t seed;
	float scale_x;
	float scale_y;
	float offset_x;
	float offset_y;
	float offset_z;
} PerlinParams;


#endif // __EACSMB_PERLIN_H__
