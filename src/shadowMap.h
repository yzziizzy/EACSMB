#ifndef __EACSMB__shadowMap_h__
#define __EACSMB__shadowMap_h__


#include "pass.h"


typedef struct ShadowMap {
	Vector2i size;
	
	GLuint depthTex;
	
	Framebuffer fb;
	RenderPipeline* rpipe;
	
	Vector2 clipPlanes;
	
	Matrix mWorldLight;
	
} ShadowMap;



struct GameState;
typedef struct GameState GameState;

ShadowMap* ShadowMap_alloc();
void ShadowMap_init(ShadowMap* sm);
void ShadowMap_SetupFBOs(ShadowMap* sm);
void ShadowMap_Render(ShadowMap* sm, PassFrameParams* cameraPFP, Vector* lightPos);

void ShadowMap_addPass(ShadowMap* sm, RenderPass* pass);







#endif // __EACSMB__shadowMap_h__
