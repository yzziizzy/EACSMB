#ifndef __EACSMB_LIGHTING_H__
#define __EACSMB_LIGHTING_H__

#include "common_gl.h"
#include "common_math.h"

#include "pcBuffer.h"
#include "pass.h"


enum LightType {
	LIGHT_TYPE_POINT = 0,
	LIGHT_TYPE_SPOT,
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_DIRECTIONAL_BOX,
	LIGHT_TYPE_SPRITE,
	
	NUM_LIGHT_TYPES
};




// grouped by attribute
typedef struct  __attribute__ ((__packed__)) LightInstance {
	Vector pos; 
	float constant;
	
	Vector direction;
	float linear;
	
	Vector color;
	float quadratic;
	
	float cutoff_angle, exponent;
	float radius, unused2;
	
} LightInstance;


typedef struct __attribute__ ((__packed__)) LightVertex {
	Vector pos;
	float category;
} LightVertex;

typedef struct LightTypeInfo {
	enum LightType type;
	
	LightVertex* sprite; // mesh info
	LightInstance* instances;
	
} LightTypeInfo;


typedef struct LightingInfo {
	
	
	// vbos
	GLuint mesh_vbo[NUM_LIGHT_TYPES];
	GLuint instance_vbo[NUM_LIGHT_TYPES];
	GLuint indices_bo[NUM_LIGHT_TYPES];
	
	LightTypeInfo lightInfo[NUM_LIGHT_TYPES];
	
} LightingInfo;


typedef struct LightManager {
	
	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	int maxInstances;
	
	GLuint instVBO;
	GLuint geomVBO;
	GLuint geomIBO;
	
	VEC(LightInstance) lights;
	
	
	// temp
	GLuint dtex;
	
} LightManager;



void initLighting();


void LightManager_Init(LightManager* lm);
void LightManager_AddPointLight(LightManager* lm, Vector pos, float radius, float intensity);
RenderPass* LightManager_CreateRenderPass(LightManager* lm);


#endif // __EACSMB_LIGHTING_H__
