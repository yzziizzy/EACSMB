#ifndef __objloader_h__ 
#define __objloader_h__ 




typedef struct OBJDataBuffer {
	float* buf;
	int cnt, sz, dims;
} OBJDataBuffer;


typedef struct OBJContents {
	
	OBJDataBuffer v, vn, vt, vp;
	
	int* f;
	int* fn;
	int* ft;
	int* fp;
	
	char* o;
} OBJContents;





















#endif 