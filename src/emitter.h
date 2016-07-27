#ifndef __EACSMB_EMITTER_H__
#define __EACSMB_EMITTER_H__


// grouped by attribute
typedef struct  __attribute__ ((__packed__)) EmitterInstance {
	Vector pos; 
	float scale;
	
	float start_time, lifespan, unused[2];
	
} EmitterInstance;

typedef struct Emitter {
	int particleNum;
	GLuint points_vbo; // just to annoy you
	GLuint instance_vbo;
	
	EmitterInstance* instances;
	
} Emitter;

















#endif // __EACSMB_EMITTER_H__