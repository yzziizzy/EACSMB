
#include <math.h>
#include "perlin.h"



// ancient shitty algorithm. when i have time to piss away i'll bother learning how simplex noise works.

static float Noise(int x, int y) {
	long n;
	n = x + y * 57;
	n = (n << 13) ^ n;
	return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);    
}



static float Interpolate(float a, float b, float x) {
	float f = (1.0 - cos(x * 3.1415927)) * 0.5;
	return  a * (1.0 - f) + b * f;
}

static float SmoothedNoise(float x, float y) {
	float corners = (Noise(x-1, y-1) + Noise(x+1, y-1) + Noise(x-1, y+1) + Noise(x+1, y+1)) / 16;
	float sides = (Noise(x-1, y) + Noise(x+1, y) + Noise(x, y-1) + Noise(x, y+1)) /  8;
	float center = Noise(x, y) / 4;
	return corners + sides + center;
}

static float InterpolatedNoise_1(float x, float y) {
	float integer_X, integer_Y, fractional_X, fractional_Y;
	float v1, v2, v3, v4, i1, i2;
	
	integer_X = floor(x);
	fractional_X = x - integer_X;

	integer_Y = floor(y);
	fractional_Y = y - integer_Y;

	v1 = SmoothedNoise(integer_X,     integer_Y);
	v2 = SmoothedNoise(integer_X + 1, integer_Y);
	v3 = SmoothedNoise(integer_X,     integer_Y + 1);
	v4 = SmoothedNoise(integer_X + 1, integer_Y + 1);

	i1 = Interpolate(v1, v2, fractional_X);
	i2 = Interpolate(v3, v4, fractional_X);

	return Interpolate(i1, i2, fractional_Y);
}


float PerlinNoise_2D(float x, float y, float persistence, int octaves) {
	float total, p, frequency, amplitude;
	int i;
	
	total = 0;
	octaves--;

	for(i = 0; i < octaves; i++) {
		frequency = 2 * i;
		amplitude = persistence * i;
		total += InterpolatedNoise_1(x * frequency, y * frequency) * amplitude;
	}

	return total;

}




















