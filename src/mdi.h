#ifndef __EACSMB__mdi_h__
#define __EACSMB__mdi_h__


#include "pcBuffer.h"
#include "pass.h"



typedef struct MDIDrawInfo {
	int numToDraw;
	int indexCount;
	int vertexCount;
	
	// for geometry
	void* indices;
	void* vertices;
	
} MDIDrawInfo;




typedef struct MultiDrawIndirect {
	
	VEC(MDIDrawInfo*) meshes;
	
	int indexSize; // sizeof(uint16_t);
	int totalVertices;
	int totalIndices;
	int totalInstances;
	
	int maxInstances;
	
	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	VAOConfig* vaoConfig;
	GLuint vao;
	
	GLenum primMode; // e.g. GL_TRIANGLE_STRIP, GL_PATCHES
	
	// data for persistently mapped instance vbo
	char isIndexed;
	GLuint instVBO;
	GLuint geomVBO;
	GLuint geomIBO;

	void (*instanceSetup)(void* /*data*/, void* /*vmem*/, MDIDrawInfo**, int /*diCount*/, PassFrameParams*);
	void (*uniformSetup)(void* /*data*/, GLuint progID);


	void* data;
	
} MultiDrawIndirect;


MultiDrawIndirect* MultiDrawIndirect_alloc(VAOConfig* vaoConfig, int maxInstances);
void MultiDrawIndirect_init(MultiDrawIndirect* dmi, VAOConfig* vaoConfig, int maxInstances);
void MultiDrawIndirect_updateGeometry(MultiDrawIndirect* mdi);
int MultiDrawIndirect_addMesh(MultiDrawIndirect* mdi, MDIDrawInfo* di);



RenderPass* MultiDrawIndirect_CreateRenderPass(MultiDrawIndirect* m, ShaderProgram* prog); 
PassDrawable* MultiDrawIndirect_CreateDrawable(MultiDrawIndirect* m, ShaderProgram* prog);





#endif // __EACSMB__mdi_h__
