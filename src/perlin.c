
#include <math.h>
#include "perlin.h"

#include <x86intrin.h>


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
	
	// horribly broken
// #if defined(EACSMB_USE_SIMD)
// 		return PerlinNoise_2D_SIMD(x, y, persistence, octaves);
	
// #else
	total = 0;
	octaves--;

	for(i = 0; i < octaves; i++) {
		frequency = 2 * i;
		amplitude = persistence * i;
		total += InterpolatedNoise_1(x * frequency, y * frequency) * amplitude;
	}
	
	return total;
// #endif

}



// packed i32's in, packed floats out
static __m256 Noise_m256i(__m256i x, __m256i y) {
	__m256i n;
	__m256i fiftyseven = _mm256_set1_epi32(57);
	
	n = _mm256_add_epi32(x, _mm256_mullo_epi32(y, fiftyseven));
	n = _mm256_xor_si256(_mm256_slli_epi32(n, 13), n);
	
	__m256i n2 = _mm256_mullo_epi32(n, n);
	        n2 = _mm256_mullo_epi32(n2, _mm256_set1_epi32(15731));
	        n2 = _mm256_add_epi32(n2, _mm256_set1_epi32(789221));
	        n2 = _mm256_mullo_epi32(n2, n);
	        n2 = _mm256_and_si256(n2, _mm256_set1_epi32(0x7fffffff));
	
	__m256 o = _mm256_cvtepi32_ps(n2);
	       o = _mm256_div_ps(o, _mm256_set1_ps(1073741824.0f));
	       o = _mm256_sub_ps(_mm256_set1_ps(1.0f), o);
	
	return o;
}

static float SmoothedNoise_SIMD(float x, float y) {
	__m256i x_ = _mm256_set1_epi32((int)x);
	__m256i y_ = _mm256_set1_epi32((int)y);
	
	//                              |   corners   |    sides    |
	__m256i x_off = _mm256_set_epi32(1, -1,  1, -1, 1, -1, 0,  0);
	__m256i y_off = _mm256_set_epi32(1,  1, -1, -1, 0,  0, 1, -1);
	
	x_ = _mm256_add_epi32(x_, x_off);
	y_ = _mm256_add_epi32(y_, y_off);
	
	__m256 shell = Noise_m256i(x_, y_);
	
	__m256 divisors = _mm256_set_ps(16.0f,16.0f,16.0f,16.0f, 8.0,8.0,8.0,8.0);
	shell = _mm256_div_ps(shell, divisors);
	
	float center = Noise(x, y) / 4.0f;
	
	// hadd
	__m128 a = _mm256_extractf128_ps(shell, 0);
	__m128 b = _mm256_extractf128_ps(shell, 1);
	a = _mm_add_ps(a, b);
	b = _mm_movehdup_ps(a);
	a = _mm_add_ps(a, b);
	a = _mm_movehl_ps(a, b);
	a = _mm_add_ss(a, b);
	
	return _mm_cvtss_f32(a) + center;
}



/*
static __m128 Interpolate_SIMD(__m128 a, __m128 b, __m128 x) {
	__m128 one = _mm_set1_ps(1.0f);
	
	// f = (1.0 - cos(x * 3.1415927)) * 0.5;
	__m128 f = _mm_mul_ps(_mm_sub_ps(
		one,
		_mm_cos_ps(_mm_mul_ps(x, _mm_set1_ps(3.14159265358979324f)))
	), _mm_set1_ps(0.5f));
	
	
	//return  a * (1.0 - f) + b * f;
	return _mm_add_ps(
		_mm_mul_ps(a, _mm_sub_ps(one, f)),
		_mm_mul_ps(b, f)
	);
}
*/
		
static float InterpolatedNoise_1_SIMD(float x, float y) {
	float integer_X, integer_Y, fractional_X, fractional_Y;
	float v1, v2, v3, v4, i1, i2;
	
	integer_X = floor(x);
	fractional_X = x - integer_X;

	integer_Y = floor(y);
	fractional_Y = y - integer_Y;

	v1 = SmoothedNoise_SIMD(integer_X,     integer_Y);
	v2 = SmoothedNoise_SIMD(integer_X + 1, integer_Y);
	v3 = SmoothedNoise_SIMD(integer_X,     integer_Y + 1);
	v4 = SmoothedNoise_SIMD(integer_X + 1, integer_Y + 1);

	i1 = Interpolate(v1, v2, fractional_X);
	i2 = Interpolate(v3, v4, fractional_X);

	return Interpolate(i1, i2, fractional_Y);
}


float PerlinNoise_2D_SIMD(float x, float y, float persistence, int octaves) {
	float total, p, frequency, amplitude;
	int i;
	
	for(i = 0; i < octaves; i++) {
		frequency = 2 * i;
		amplitude = persistence * i;
		total += InterpolatedNoise_1_SIMD(x * frequency, y * frequency) * amplitude;
	}
}









