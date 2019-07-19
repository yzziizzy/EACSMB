#ifndef __EACSMB_jpeg_h__


#include <stdint.h>

#include "texture.h"



int writeJPEG_RGBA(char* path, int width, int height, void* data, uint32_t alphaColor, int quality);
int writeJPEG_RGB(char* path, int width, int height, void* data, int quality);



TexBitmap* readJPEG_RGBA(char* path, uint32_t alphaValue);
TexBitmap* readJPEG_R(char* path);



#endif // __EACSMB_jpeg_h__
