

#include "texgen.h"

#include "math.h"





void gen_sinewave(BitmapRGBA8* bmp, int channel, float period) {
	
	int x,y;
	int bytedepth = 4;
	float scaler = (period * 2 * F_PI) / (float)bmp->width;
	
	for(y = 0; y < bmp->height; y++) {
		for(x = 0; x < bmp->width; x++) {
			float th = fmod((float)x * scaler, F_2PI);
			bmp->data[(y * bmp->width + x) * bytedepth + channel] = sin(th) * 256;
		}
	}
	
}




void gen_lerp(BitmapRGBA8* a, BitmapRGBA8* b, float t, int channel) {
	
	int x,y;
	int bytedepth = 4;
	float scaler = (period * 2 * F_PI) / (float)bmp->width;
	
	for(y = 0; y < bmp->height; y++) {
		for(x = 0; x < bmp->width; x++) {
			
			int ad = a->data[(y * bmp->width + x) * bytedepth + channel];
			int bd = b->data[(y * bmp->width + x) * bytedepth + channel];
			b->data[(y * bmp->width + x) * bytedepth + channel] = CLAMP((ad * t) + (bd * (1.0 - t)), 0, 255);
		}
	}
	
}






