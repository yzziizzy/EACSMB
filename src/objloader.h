#ifndef __EACSMB_OBJLOADER_H__
#define __EACSMB_OBJLOADER_H__




typedef struct OBJDataBuffer {
	float* buf;
	int cnt, sz, dims;
} OBJDataBuffer;


typedef struct OBJFace {
	int v, vt, vn;
} OBJFace;


typedef struct OBJContents {
	
	OBJDataBuffer v, vn, vt;
	
	OBJFace* f;
	int f_cnt;
	int f_sz;
	
	char* o;
} OBJContents;





















#endif // __EACSMB_OBJLOADER_H__