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

/*
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

*/
void terrain_init() {
	
	
	
};

void terrain_initTexInfo(TerrainTexInfo* tti) {
	VEC_INIT(&tti->config);
	tti->nameLookup = HT_create(5);
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
		
		// TODO: check for minimum features
		
		HT_set(tti->nameLookup, texName, VEC_LEN(&tti->config));
		VEC_PUSH(&tti->config, tt);
	}
	
	
}


static void loadTexData(TerrainTex* tt) {
	
	
}


void terrain_loadTextures(TerrainTexInfo* tti) {
	
	//	TexArray* ta;
	int len, i;
	char* source, *s;
	
	int w, h;
	
	
	//len = ptrlen(files);
	len = VEC_LEN(&tti->config);
	
	//bmps = malloc(sizeof(BitmapRGBA8*) * len);
	//ta = calloc(sizeof(TexArray), 1);
	
	w = 256;
	h = 256;
	
	/*
	for(i = 0; i < len; i++) {
		//bmps[i] = readPNG(files[i]);
		
		if(!bmps[1]) {
			//printf("Failed to load %s\n", files[i]);
			continue;
		}
		
		w = MAX(w, bmps[i]->width);
		h = MAX(h, bmps[i]->height);
	}*/
	
//	ta->width = w;
//	ta->height = h;
//	ta->depth = len;
	
	printf("len: %d\n", len);
	
	
	glGenTextures(1, &tti->diffuse.tex_id);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tti->diffuse.tex_id);
	glexit("failed to create texture array 1");
	
// 		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_FALSE);
	glexit("failed to create texture array 2");

	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glexit("failed to create texture array 3");
	
	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
		1,  // mips, flat
		GL_RGBA8,
		w, h,
		len); // layers
	
	glexit("failed to create texture array 4");
	
	
	for(i = 0; i < len; i++) {
		//if(!bmps[i]) continue;
		
		BitmapRGBA8 bmp;
		char buf[512];
		buf[0] = 0;
		strcpy(buf, "assets/textures/");
		strcat(buf, VEC_DATA(&tti->config)[i]->paths.diffuse);
		
		if(readPNG2(buf, &bmp)) {
			printf("failed to load file\n");
			return;
		}
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
			0,  // mip level, 0 = base, no mipmap,
			0, 0, i,// offset
			w, h,
			1,
			GL_RGBA,  // format
			GL_UNSIGNED_BYTE, // input type
			bmp.data);
		glexit("could not load tex array slice");
		
		free(bmp.data);
	}
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
//	free(bmps);
	
	//return ta;
}
















