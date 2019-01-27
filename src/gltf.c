#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "utilities.h"

#include "gltf.h"



#include "json_gl.h"






gltf_file* gltf_loadFile(char* path) {
	json_file_t* jsf;
	
	
	
	jsf = json_load_path(path);
	return gltf_parsejson(jsf->root);
}


gltf_file* gltf_parsejson(json_value_t* root) {
	gltf_file* gf;
	struct json_array_node* link;
	int ret, index;
	json_value_t* cat_j;
	
	if(root->type != JSON_TYPE_OBJ) {
		return NULL;
	}
	
	pcalloc(gf);
	
	
	// buffers first
	json_obj_get_key(root, "buffers", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		
		gltf_buffer* buf = pcalloc(buf);
		buf->uri = json_obj_key_as_string(val, "uri");
		buf->length = json_obj_get_int(val, "byteLength", -1);
		
		VEC_PUSH(&gf->buffers, buf);
		
		link = link->next;
	}
	
	
	// buffer views next
	json_obj_get_key(root, "bufferViews", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		
		gltf_bufferView* bufv = pcalloc(bufv);
		index = json_obj_get_int(val, "buffer", -1);
		bufv->length = json_obj_get_int(val, "byteLength", -1);
		bufv->offset = json_obj_get_int(val, "byteOffset", 0);
		bufv->target = json_obj_get_int(val, "target", -1);
		bufv->buffer = VEC_ITEM(&gf->buffers, index);
		
		VEC_PUSH(&gf->bufferViews, bufv);
		
		link = link->next;
	}
	
	
	// accessors
	json_obj_get_key(root, "accessors", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		char* typename;
		
		gltf_accessor* acc = pcalloc(acc);
		index = json_obj_get_int(val, "bufferView", -1);
		acc->count = json_obj_get_int(val, "count", -1);
		acc->compType = json_obj_get_int(val, "componentType", -1);
		typename = json_obj_key_as_string(val, "type");
		if(0 == strcmp(typename, "SCALAR")) acc->type = GLTF_TYPE_SCALAR;
		else if(0 == strcmp(typename, "VEC2")) acc->type = GLTF_TYPE_VEC2;
		else if(0 == strcmp(typename, "VEC3")) acc->type = GLTF_TYPE_VEC3;
		else if(0 == strcmp(typename, "VEC4")) acc->type = GLTF_TYPE_VEC4;
		else if(0 == strcmp(typename, "MAT2")) acc->type = GLTF_TYPE_MAT2;
		else if(0 == strcmp(typename, "MAT3")) acc->type = GLTF_TYPE_MAT3;
		else if(0 == strcmp(typename, "MAT4")) acc->type = GLTF_TYPE_MAT4;
		else {
			acc->type = 0;
		}
		
		free(typename);
		acc->bufferView = VEC_ITEM(&gf->bufferViews, index);
		
		VEC_PUSH(&gf->accessors, acc);
		
		link = link->next;
	}
	
	// images
	json_obj_get_key(root, "images", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		
		gltf_image* im = pcalloc(im);
// 		index = json_obj_get_int(jo, "bufferView", -1);
		
		// TODO finish
		
		VEC_PUSH(&gf->images, im);
		
		link = link->next;
	}
	
	// samplers
	json_obj_get_key(root, "samplers", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		
		gltf_sampler* s = pcalloc(s);
 		s->minFilter = json_obj_get_int(val, "minFilter", GL_NEAREST);
 		s->magFilter = json_obj_get_int(val, "maxFilter", GL_NEAREST);
 		s->wrapS = json_obj_get_int(val, "wrapS", GL_REPEAT);
 		s->wrapT = json_obj_get_int(val, "wrapT", GL_REPEAT);
		
		VEC_PUSH(&gf->samplers, s);
		
		link = link->next;
	}
	
	// textures
	json_obj_get_key(root, "textures", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		
		gltf_texture* tex = pcalloc(tex);
 		index = json_obj_get_int(val, "source", -1);
		tex->source = VEC_ITEM(&gf->images, index);
 		index = json_obj_get_int(val, "sampler", -1);
		tex->sampler = VEC_ITEM(&gf->samplers, index);
		
		VEC_PUSH(&gf->textures, tex);
		
		link = link->next;
	}
	
	// materials
	json_obj_get_key(root, "materials", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		
		gltf_material* mat = pcalloc(mat);
		mat->name = json_obj_key_as_string(val, "name");
		
		// TODO: Fix
		
		VEC_PUSH(&gf->materials, mat);
		
		link = link->next;
	}
	
	// meshes
	json_obj_get_key(root, "meshes", &cat_j);
	link = cat_j->v.arr->head;
	while(link) {
		json_value_t* val = link->value;
		json_value_t* prims_j;
		
		
		gltf_mesh* mesh = pcalloc(mesh);
		mesh->name = json_obj_key_as_string(val, "name");
		
		json_obj_get_key(val, "meshes", &prims_j);
		struct json_array_node* plink = prims_j->v.arr->head;
		while(plink) {
			json_value_t* pval = link->value;
			json_value_t* attribs, *aval;
			int64_t i;
			
			gltf_primitive* prim = pcalloc(prim);
		
			index = json_obj_get_int(pval, "indices", -1);
			prim->indices = VEC_ITEM(&gf->accessors, index);
			
			json_obj_get_key(pval, "attributes", &attribs);
			
				json_obj_get_key(attribs, "POSITION", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->position = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "NORMAL", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->normal = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "TANGENT", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->tangent = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "COLOR_0", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->color0 = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "TEXCOORD_0", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->texCoord0 = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "TEXCOORD_1", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->texCoord1 = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "JOINTS_0", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->joints0 = VEC_ITEM(&gf->accessors, i);
				}
			
				json_obj_get_key(attribs, "WEIGHTS_0", &aval);
				if(aval) {
					json_as_int(aval, &i);
					prim->weights0 = VEC_ITEM(&gf->accessors, i);
				}
			
			
			VEC_PUSH(&mesh->primitives, prim);
			
			plink = plink->next;
		}
		
		VEC_PUSH(&gf->meshes, mesh);
		
		link = link->next;
	}
	
	
	// animations
	
	// skins
	
	// nodes
	
	
	return gf;
}



void gltf_loadBuffer(gltf_buffer* b) {
	if(b->data) return;
	
	b->data = malloc(b->length);
	
	FILE* f;
	
	f = fopen(b->uri, "rb");
	fread(b->data, 1, b->length, f);
	fclose(f);
	
}

void gltf_readBufferView(gltf_bufferView* bv, void* buffer) {
	gltf_loadBuffer(bv->buffer);
	memcpy(buffer, bv->buffer->data + bv->offset, bv->length);
}

void gltf_readAccessor(gltf_accessor* acc, void* buffer) {
	gltf_readBufferView(acc->bufferView, buffer);
}

void gltf_readImage(gltf_image* img, void* buffer) {
// 	gltf_readBufferView(acc->bufferView, buffer);
}

void gltf_free(gltf_file* gf) {
	VEC_EACH(&gf->accessors, ai, a) { free(a); }
	VEC_EACH(&gf->bufferViews, ai, a) { free(a); }
	VEC_EACH(&gf->buffers, ai, a) { 
		if(a->uri) free(a->uri);
		if(a->data) free(a->data);
		free(a); 
	}
	VEC_EACH(&gf->samplers, ai, a) { free(a); }
	VEC_EACH(&gf->textures, ai, a) { free(a); }
	VEC_EACH(&gf->images, ai, a) { 
		if(a->uri) free(a->uri);
		free(a); 
	}
	VEC_EACH(&gf->materials, ai, a) { 
		if(a->name) free(a->name);
		free(a); 
	}
	VEC_EACH(&gf->meshes, ai, a) { 
		VEC_EACH(&a->primitives, pi, p) {
			free(p);
		}
		free(a); 
	}
	
	// TODO: free hash table
	
}










