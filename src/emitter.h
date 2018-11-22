#ifndef __EACSMB_EMITTER_H__
#define __EACSMB_EMITTER_H__

#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"
#include "mdi.h"



// grouped by attribute
typedef struct  __attribute__ ((__packed__)) EmitterInstance {
	Vector pos; 
	float scale;
	
	float start_time, lifespan, unused[2];
	
} EmitterInstance;

// need: fade in/out time 
// refactor time for offset and cooldown
//           |---------------------lifetime----------------------------|
// --offset--|--spawn-delay--|--fade-in--|--<calculated>--|--fade-out--|
typedef struct EmitterSprite {
	Vector start_pos;
	float offset;
	
	Vector start_vel;
	float spawn_delay;
	
	Vector start_acc;
	float lifetime;
	
	float size, spin, growth_rate, randomness;
	
	float fade_in, fade_out, unallocated_1, unallocated_2;
	
} EmitterSprite;



typedef struct Emitter {
	int particleNum;
	int instanceNum;
	GLuint points_vbo; // just to annoy you
	GLuint instance_vbo;
	
	VEC(EmitterSprite) sprites;
	VEC(EmitterInstance) instances;
	
} Emitter;


typedef struct EmitterManager {
	
	VEC(Emitter*) emitters;
	HashTable(int) lookup;
	
	TextureManager* tm;
	MultiDrawIndirect* mdi;
	
} EmitterManager;




// void emitter_update_vbo(Emitter* e); 
// void emitterAddInstance(Emitter* e, EmitterInstance* ei); 
// Emitter* makeEmitter(); 
// void initEmitters();






EmitterManager* EmitterManager_alloc(int maxInstances);
int EmitterManager_addEmitter(EmitterManager* em, Emitter* e, char* name);
void EmitterManager_addInstance(EmitterManager* em, int index, EmitterInstance* inst);
void EmitterManager_updateGeometry(EmitterManager* em);
int EmitterManager_lookupName(EmitterManager* em, char* name);



PassDrawable* EmitterManager_CreateDrawable(EmitterManager* em);
RenderPass* EmitterManager_CreateRenderPass(EmitterManager* em);



#endif // __EACSMB_EMITTER_H__
