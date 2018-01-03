#ifndef __EACSMB_TERRAIN_H__
#define __EACSMB_TERRAIN_H__

#include "ds.h"
#include "hash.h"



#define TERRAINTEX_DIFFUSE       0x0001
#define TERRAINTEX_NORMAL        0x0002
#define TERRAINTEX_DISPLACEMENT  0x0004
#define TERRAINTEX_SPECULAR      0x0008
#define TERRAINTEX_REFLECTIVITY  0x0010

#define TERRAINTEX_ALL_FEATURES \
	( TERRAINTEX_DIFFUSE \
	| TERRAINTEX_NORMAL \
	| TERRAINTEX_DISPLACEMENT \
	| TERRAINTEX_SPECULAR \
	| TERRAINTEX_REFLECTIVITY \
	)



typedef struct TerrainTex {
	char* name;
	uint32_t index;
	uint16_t width, height;
	
	uint64_t featureMask;
	
	struct {
		char* diffuse;
		char* normal;
		char* displacement;
		char* specular;
		char* reflectivity;
	} paths;

	
	
} TerrainTex;


typedef struct TerrainTexInfo {
	
	VEC(TerrainTex*) config;
	HashTable* nameLookup;
	
	// needed: shader-accessible layout information
	
	struct {
		GLuint tex_id;
		int depth;
	} diffuse, normal, displacement, specular, reflectivity;
	
	GLuint infoId;
	
	
	
} TerrainTexInfo;


void terrain_init();

void terrain_readConfigJSON(TerrainTexInfo* tti, char* path);





#endif // __EACSMB_TERRAIN_H__
