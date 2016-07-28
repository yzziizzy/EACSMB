#ifndef __EACSMB_EMITTER_H__
#define __EACSMB_EMITTER_H__


// grouped by attribute
typedef struct  __attribute__ ((__packed__)) EmitterInstance {
	Vector pos; 
	float scale;
	
	float start_time, lifespan, unused[2];
	
} EmitterInstance;

typedef struct EmitterSprite {
	Vector start_pos;
	float phys_fn_index;
	
	Vector start_vel;
	float spawn_delay;
	
	Vector start_acc;
	float lifetime;
	
	float size, spin, growth_rate, randomness;
	
} EmitterSprite;

typedef struct Emitter {
	int particleNum;
	GLuint points_vbo; // just to annoy you
	GLuint instance_vbo;
	
	EmitterSprite* sprite;
	EmitterInstance* instances;
	
} Emitter;

















#endif // __EACSMB_EMITTER_H__