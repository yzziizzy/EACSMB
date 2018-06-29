#ifndef __EACSMB_EMITTER_H__
#define __EACSMB_EMITTER_H__

#include "common_gl.h"
#include "common_math.h"

#include "ds.h"

#include "pass.h"



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
	
	VEC(EmitterSprite) sprite;
	VEC(EmitterInstance) instances;
	
} Emitter;






void emitter_update_vbo(Emitter* e); 
void emitterAddInstance(Emitter* e, EmitterInstance* ei); 
Emitter* makeEmitter(); 
void initEmitters();


PassDrawable* Emitter_CreateDrawable(Emitter* e);
RenderPass* Emitter_CreateRenderPass(Emitter* e);

#endif // __EACSMB_EMITTER_H__
