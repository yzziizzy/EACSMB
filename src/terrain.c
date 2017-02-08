#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c_json/json.h"
#include "json_gl.h"


#include "utilities.h"
#include "texture.h"
#include "terrain.h"


void vec_resize(void** data, size_t* size, size_t elem_size) {
	void* tmp;
	
	if(*size < 8) *size = 8;
	else *size *= 2;
	
	tmp = realloc(*data, *size * elem_size);
	if(!tmp) {
		fprintf(stderr, "Out of memory in vector resize");
		return;
	}
	
	*data = tmp;
}


void terrain_init() {};

void terrain_initTexInfo(TerrainTexInfo* tti) {
	VEC_INIT(&tti->config);
}



void terrain_readConfigJSON(TerrainTexInfo* tti, char* path) {
	
	json_file_t* jsf;
	
	jsf = json_load_path(path);
	
	
	
	char** a;
	size_t l, i;
	
	int ret;
	struct json_obj* o;
	void* iter;
	char* key, *texName, *tmp;
	struct json_value* v, *tc;
	TerrainTex* tt; 

	
	
	// TODO: error handling
	if(jsf->root->type != JSON_TYPE_OBJ) {
		return;
	}

	
	a = malloc(l * 2 * sizeof(*a));
	if(!a) {
		return;
	}
	
	// do shit
	iter = NULL;
	while(json_obj_next(jsf->root, &iter, &key, &tc)) {
		
		texName = strdup(key);
		tt = calloc(1, sizeof(*tt));
		if(!tt) return; // TODO panic!
		tt->name = texName;
		
		printf("loading tex: %s\n", tt->name);
		
		ret = json_obj_get_key(tc, "diffuse", &v);
		if(!ret || v) { 
			json_as_string(v, &tmp);
			tt->paths.diffuse = strdup(tmp);
			printf("%s, got diffuse: %s \n", tt->name, tt->paths.diffuse);
			tt->featureMask &= TERRAINTEX_DIFFUSE;
		}

		ret = json_obj_get_key(tc, "normal", &v);
		if(ret && !v) {
			json_as_string(v, &tmp);
			tt->paths.normal = strdup(tmp);
			
			tt->featureMask &= TERRAINTEX_NORMAL;
		}
		
		ret = json_obj_get_key(tc, "displacement", &v);
		if(ret && !v) {
			json_as_string(v, &tmp);
			tt->paths.displacement = strdup(tmp);
			
			tt->featureMask &= TERRAINTEX_DISPLACEMENT;
		}
		
		ret = json_obj_get_key(tc, "specular", &v);
		if(ret && !v) {
			json_as_string(v, &tmp);
			tt->paths.specular = strdup(tmp);
			
			tt->featureMask &= TERRAINTEX_SPECULAR;
		}
		
		ret = json_obj_get_key(tc, "reflectivity", &v);
		if(ret && !v) {
			json_as_string(v, &tmp);
			tt->paths.reflectivity = strdup(tmp);
			
			tt->featureMask &= TERRAINTEX_REFLECTIVITY;
		}
		
		VEC_PUSH(&tti->config, tt);
	}
	
// 	*out = a;
// 	*len = l;
	
	
	
	
}


void terrain_loadTextures() {
	
	
}
















