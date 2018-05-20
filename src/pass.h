#ifndef __EACSMB_pass_h__
#define __EACSMB_pass_h__


#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "shader.h"
#include "fbo.h"


// TODO: check alignment for ubo
typedef struct PassDrawParams {
	Matrix* mWorldView;
	Matrix* mViewProj;
	
	// inverse
	Matrix* mViewWorld;
	Matrix* mProjView;
	
	Vector eyeVec;
	Vector eyePos;
	Vector sunVec;
	
	float timeSeconds;
	float timeFractional;
	float spinner;
	
	Vector2i targetSize;
} PassDrawParams;

typedef struct PassFrameParams {
	PassDrawParams* dp;
	
	double timeElapsed; // time since last frame
	double gameTime; // gets paused, persisted on save, etc
	double wallTime; // from the first frame rendered this session
} PassFrameParams;




struct PassDrawable;

typedef void (*PassDrawFn)(void* data, struct PassDrawable* drawable, PassDrawParams* dp);


typedef struct PassDrawable {
	
	char* name;
	
	ShaderProgram* prog; 
	void* data;
	
	// where uniform buffers would be set up
	void (*preFrame)(PassFrameParams*, void*);
	
	// might be called many times
	PassDrawFn draw;
	
	// where circular buffers are rotated
	void (*postFrame)(void*);
	
} PassDrawable;




typedef struct RenderPass {
	
	char clearColor;
	char clearDepth;
	
	char fboIndex;
	// fbo config, texture bindings, etc
	
	ShaderProgram* prog;
	
	// texture ul's to read from
	GLuint diffuseUL;
	GLuint normalsUL;
	GLuint lightingUL;
	GLuint depthUL;
	
	
	VEC(PassDrawable*) drawables;
	
} RenderPass;



typedef struct RenderPipeline {
	
	Vector2i viewSz;
	
	Vector4 clearColor;
	
	// fbo's
	GLuint* backingTextures;
	Framebuffer fbos[2];
	
	VEC(RenderPass*) passes;
	
} RenderPipeline;


void initRenderPipeline();


int RenderPass_addDrawable(RenderPass* rp, PassDrawable* d);
PassDrawable* Pass_allocDrawable(char* name);

void RenderPass_init(RenderPass* pass, ShaderProgram* prog); 
void RenderPipeline_addShadingPass(RenderPipeline* rpipe, char* shaderName); 

void RenderPipeline_renderAll(RenderPipeline* rp, PassDrawParams* pdp);
void RenderPass_renderAll(RenderPass* pass, PassDrawParams* pdp);
void RenderPass_preFrameAll(RenderPass* pass, PassFrameParams* pfp);
void RenderPass_postFrameAll(RenderPass* pass);


void RenderPipeline_init(RenderPipeline* rp);
void RenderPipeline_rebuildFBOs(RenderPipeline* rp, Vector2i sz);
void RenderPipeline_destroy(RenderPipeline* rp);

GLuint RenderPipeline_getOutputTexture(RenderPipeline* rp);


// temporary hacky code
typedef void (*pass_callback)(void* data, PassFrameParams* pfp);
void RegisterPrePass(pass_callback cb, void* data, char* name);
void RemovePrePass(char* name);
void RenderAllPrePasses(PassFrameParams* pfp);











#endif // __EACSMB_pass_h__
