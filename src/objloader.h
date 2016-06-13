#ifndef __EACSMB_OBJLOADER_H__
#define __EACSMB_OBJLOADER_H__




typedef struct OBJDataBuffer {
	float* buf;
	int cnt, sz, dims;
} OBJDataBuffer;


typedef struct OBJVertex { // Face vertex
	int v, vt, vn;
} OBJVertex;


typedef struct OBJContents {
	
	OBJDataBuffer v, vn, vt;
	
	// only supports one mesh atm
	OBJVertex* f;
	int fv_cnt;
	int fv_sz;
	
	char* o;
} OBJContents;



void loadOBJFile(char* path, int four_d_verts, OBJContents* contents);
Mesh* OBJtoMesh(OBJContents* obj);

















#endif // __EACSMB_OBJLOADER_H__
