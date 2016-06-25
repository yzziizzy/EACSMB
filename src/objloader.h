#ifndef __EACSMB_OBJLOADER_H__
#define __EACSMB_OBJLOADER_H__




typedef struct OBJDataBuffer {
	float* buf;
	int cnt, sz, dims;
} OBJDataBuffer;


typedef struct OBJVertex { // Face vertex
	Vector v, n;
	Vector2 t;
} OBJVertex;


typedef struct OBJContents {
	
	// only supports one mesh atm
	OBJVertex* faces;
	int faceCnt;
	
	char* o;
} OBJContents;



void loadOBJFile(char* path, int four_d_verts, OBJContents* contents);

















#endif // __EACSMB_OBJLOADER_H__
