#ifndef __EACSMB_jpeg_h__


#include <stdint.h>




int writeJPEG_RGBA(char* path, int width, int height, void* data, uint32_t alphaColor, int quality);
int writeJPEG_RGB(char* path, int width, int height, void* data, int quality);



#endif // __EACSMB_jpeg_h__
