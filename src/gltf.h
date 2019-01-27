#ifndef __EACSMB_gltf_h__
#define __EACSMB_gltf_h__


#include "ds.h"
#include "hash.h"

#include "c_json/json.h"


enum {
	GLTF_TYPE_SCALAR = 1,
	GLTF_TYPE_VEC2,
	GLTF_TYPE_VEC3,
	GLTF_TYPE_VEC4,
	GLTF_TYPE_MAT2,
	GLTF_TYPE_MAT3,
	GLTF_TYPE_MAT4,
};




typedef struct gltf_buffer {
	char* uri;
	size_t length;
	
	void* data;
} gltf_buffer;


typedef struct gltf_bufferView {
	size_t length;
	size_t offset;
	int target;
	
	gltf_buffer* buffer;
} gltf_bufferView;


typedef struct gltf_accessor {
	gltf_bufferView* bufferView;
	
	GLuint compType;
	int count;
	
	int type;
	
} gltf_accessor;

// samplers are just the texture properties
typedef struct gltf_sampler {
	GLuint minFilter;
	GLuint magFilter;
	GLuint wrapS;
	GLuint wrapT;
} gltf_sampler;


typedef struct gltf_image {
	char* uri;
	// -- or --
	gltf_bufferView* bufferView;
	char* mimeType;
} gltf_image;


typedef struct gltf_texture {
	gltf_image* source;
	gltf_sampler* sampler;
} gltf_texture;


typedef struct gltf_material {
	char* name;
	
	// TODO: finish
	
} gltf_material;


typedef struct gltf_primitive {
	GLuint mode;
	gltf_accessor* indices;
	
	gltf_accessor* position;
	gltf_accessor* normal;
	gltf_accessor* tangent;
	gltf_accessor* color0;
	gltf_accessor* weights0;
	gltf_accessor* joints0;
	gltf_accessor* texCoord0;
	gltf_accessor* texCoord1;
	
} gltf_primitive;


typedef struct gltf_mesh {
	char* name;
	
	VEC(gltf_primitive*) primitives;
	
} gltf_mesh;



typedef struct gltf_file {
	char* path;
	
	VEC(gltf_accessor*) accessors;
	VEC(gltf_bufferView*) bufferViews;
	VEC(gltf_buffer*) buffers;
	VEC(gltf_image*) images;
	VEC(gltf_sampler*) samplers;
	VEC(gltf_texture*) textures;
	VEC(gltf_material*) materials;
	VEC(gltf_mesh*) meshes;
	
	HashTable(int) meshLookup;
	
} gltf_file;




void gltf_loadBuffer(gltf_buffer* b);
void gltf_readBufferView(gltf_bufferView* bv, void* buffer);
void gltf_readAccessor(gltf_accessor* acc, void* buffer);
void gltf_readImage(gltf_image* img, void* buffer);


gltf_file* gltf_parsejson(json_value_t* root);
gltf_file* gltf_loadFile(char* path);
void gltf_free(gltf_file* gf);




#endif // __EACSMB_gltf_h__
