#ifndef __EACSMB_TERRAIN_H__
#define __EACSMB_TERRAIN_H__


#define VEC(t) \
struct vector { \
	size_t len, alloc; \
	t* data; \
}

#define VEC_INIT(x) \
do { \
	(x)->data = NULL; \
	(x)->len = 0; \
	(x)->alloc = 0; \
} while(0)

#define VEC_LEN(x) (x)->len
#define VEC_ALLOC(x) (x)->alloc
#define VEC_DATA(x) (x)->data

//  

#define VEC_GROW(x) vec_resize(&VEC_DATA(x), &VEC_ALLOC(x), sizeof(*VEC_DATA(x)))

#define VEC_CHECK(x) \
do { \
	if(VEC_LEN(x) >= VEC_ALLOC(x)) { \
		VEC_GROW(x); \
	} \
} while(0)

#define VEC_PUSH(x, e) \
do { \
	VEC_CHECK(x); \
	VEC_DATA(x)[VEC_LEN(x)] = (e); \
	VEC_LEN(x)++; \
} while(0)

#define VEC_PEEK(x) VEC_DATA(x)[VEC_LEN(x) - 1]

#define VEC_POP(x, e) \
do { \
	VEC_CHECK(x); \
	(e) = VEC_PEEK(x); \
	VEC_LEN(x)--; \
} while(0)





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
	
	
	
} TerrainTexInfo;


void terrain_init();

void terrain_readConfigJSON(TerrainTexInfo* tti, char* path);





#endif // __EACSMB_TERRAIN_H__
