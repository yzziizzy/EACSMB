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
	Matrix* mWorldProj;
	
	// inverse
	Matrix* mViewWorld;
	Matrix* mProjView;
	Matrix* mProjWorld;
	
	Vector eyeVec;
	Vector eyePos;
	Vector sunVec;
	
	Vector vEyePos;
	Vector vLookDir;
	Vector vEyeSun;
	Vector vSunPos;
	
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



typedef struct DrawTimer {
	char timerLen; // size of history buffer, max 16
	char timerNext;
	char timerUsed;
	char timerHistIndex;
	GLuint timerStartIDs[5];
	GLuint timerEndIDs[5];
	float timerHistory[16];
	
	// results values 
	float timerAvg;
	float timerMin;
	float timerMax;
} DrawTimer;




struct PassDrawable;

typedef void (*PassDrawFn)(void* data, struct PassDrawable* drawable, PassDrawParams* dp);


typedef struct PassDrawable {
	
	char* name;
	
	GLuint diffuseUL;
	GLuint normalsUL;
	GLuint lightingUL;
	GLuint depthUL;
	
	// forward matrices
	GLuint ul_mWorldView;
	GLuint ul_mViewProj;
	GLuint ul_mWorldProj;
	
	// inverse matrices
	GLuint ul_mViewWorld;
	GLuint ul_mProjView;
	GLuint ul_mProjWorld;
	
	GLuint ul_timeSeconds;
	GLuint ul_timeFractional;
	
	GLuint ul_targetSize;
	
	ShaderProgram* prog; 
	void* data;
	
	// where uniform buffers would be set up
	void (*preFrame)(PassFrameParams*, void*);
	
	// might be called many times
	PassDrawFn draw;
	
	// where circular buffers are rotated
	void (*postFrame)(void*);
	
	DrawTimer timer;
} PassDrawable;




typedef struct RenderPass {
	
	char clearColor;
	char clearDepth;
	
	char fboIndex;
	// fbo config, texture bindings, etc
	
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

void RenderPass_init(RenderPass* pass); 
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





void DrawTimer_Init(DrawTimer* pd);
void DrawTimer_Start(DrawTimer* pd);
void DrawTimer_End(DrawTimer* pd);






#endif // __EACSMB_pass_h__
