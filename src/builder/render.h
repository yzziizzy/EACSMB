#ifndef __EACSMB_builder_render_h__
#define __EACSMB_builder_render_h__








#include "../meshBuilder.h" 
#include "../dynamicMesh.h" 
#include "../shader.h" 



typedef struct RenderParams {
	
	Vector w_eyePos;
	Vector w_eyeDir;
	
	Vector2i fboSize;
	
	
	Matrix* mWorldView;
	Matrix* mViewProj;
	
	// inverse
	Matrix* mViewWorld;
	Matrix* mProjView;
	
	
	//time
	
} RenderParams;




typedef struct BuilderRenderable {
	void (*render)(void* data, RenderParams* rp);
	void* data;
	
	//shader here?
} BuilderRenderable;


typedef struct BuilderPass {
	
	char clearColor;
	char clearDepth;
	
	char fboIndex;
	// fbo config, texture bindings, etc
	
	ShaderProgram* prog;
	
	GLuint diffuseUL;
	GLuint normalsUL;
	GLuint lightingUL;
	GLuint depthUL;
	
	//FBOConfig* gbufConfig;
	
	VEC(BuilderRenderable) renderables;
	
} BuilderPass;






typedef struct BuilderPipeline {
	DynamicMeshManager dmm;
	
	Vector2i viewSz;
	
	// fbo's
	GLuint* backingTextures;
	GLuint gbuf;
	GLuint sbuf; // output buffer
	
	Framebuffer fbos[2];
	
	VEC(BuilderPass*) passes;
	
} BuilderPipeline;



void BuilderPipeline_renderAll(BuilderPiperline* bp, RenderParams* rp);




#endif // __EACSMB_builder_render_h__
