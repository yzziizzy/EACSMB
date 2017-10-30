#ifndef __EACSMB_WATERPLANE_H__
#define __EACSMB_WATERPLANE_H__

#include "common_gl.h"
#include "common_math.h"

#include "ds.h"

// #include "pcBuffer.h"
// #include "texture.h"


typedef struct {
	unsigned short u, v;
} UVPair;

struct WaterPlaneVertex {
	Vector pos;
	UVPair t;
};





typedef struct WaterPlane {
	
	
	// always GL_TRIANGLES
	struct WaterPlaneVertex* vertices;
	int vertexCnt;
	
	GLuint vbo;
	
	
} WaterPlane;


void initWaterPlane();
void drawWaterPlane(WaterPlane* wp, Matrix* view, Matrix* proj);
void WaterPlane_create(WaterPlane* wp, float size, Vector* pos);

#endif // __EACSMB_WATERPLANE_H__
