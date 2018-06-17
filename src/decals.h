#ifndef __EACSMB_decals_h__
#define __EACSMB_decals_h__


#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"





typedef struct DecalVertex {
	Vector v;
	struct {
		unsigned short u, v;
	} t;
} DecalVertex;


typedef struct DecalInstance {
	Vector pos; // shouldn't this only be 2 components?
	float size;
	
	float rot;
	float alpha;
	float unused1, unused2;
	
	unsigned short texIndex;
	unsigned short tileInfo; // 0 = clamp to transparent, 1 = tilex, 2 = tiley, 3 = tile both
	
}  __attribute__ ((packed)) DecalInstance;

typedef struct Decal {
	int texIndex;
	
	float size;
	VEC(DecalInstance) instances;
	int numToDraw; // todo: move
} Decal;


typedef struct DecalManager {
	
	VEC(Decal*) decals;
	HashTable(int) lookup;

	int maxInstances;

	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	// data for persistently mapped instance vbo
	GLuint instVBO;
	GLuint geomVBO;
	GLuint geomIBO;
	
	TextureManager* tm;


} DecalManager;




#endif // __EACSMB_decals_h__
