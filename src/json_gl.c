#include <stdlib.h> 
#include <stdio.h> 
#include <stdint.h> 

#include "c_json/json.h"
#include "json_gl.h" 
#include "hash.h" 

static HashTable* lookup;

struct enum_data {
	char* k;
	GLenum v;
};

#define gl_enum_data(str, macro, val) {str, val},
// #define gl_enum_data(str, macro, val) {str, macro},
static struct enum_data enum_values_for_gl[] = {
	#include "gl_enum_data.c" 
	{0,0}
};



void json_gl_init_lookup() {
	struct enum_data* d;
	
	if(lookup) return;
	lookup = HT_create(13); // 8192; there are 5671 enums
	
	d = enum_values_for_gl;
	
	fprintf(stderr, "initializing GLenum lookup...");
	while(d->k) {
		HT_set(lookup, d->k, d->v);
		d++;
	}
	fprintf(stderr, " done.\n");
}




int json_as_GLenum(struct json_value* v, GLenum* out) {
	
	switch(v->type) { // actual type
		case JSON_TYPE_UNDEFINED:
		case JSON_TYPE_NULL:
			*out = 0;
			return 1;
			
		case JSON_TYPE_INT: // treat it as a literal value
			*out = v->v.integer;
			return 0;
			
		case JSON_TYPE_DOUBLE:
			*out = v->v.dbl;  // treat it as a literal value, converted to int
			return 0;
			
		case JSON_TYPE_STRING: // look up the enum
			return HT_get(lookup, v->v.str, out);
			
			
		case JSON_TYPE_OBJ: // all invalid
		case JSON_TYPE_ARRAY:
		case JSON_TYPE_COMMENT_SINGLE:
		case JSON_TYPE_COMMENT_MULTI:
		default:
			*out = 0;
			return 1;
	}
}


static float index_as_float(json_value_t* obj, int index, float def) {
	json_value_t* v;
	float f;
	int ret;
	
	if(!json_as_float(v, &f)) {
		return def;
	}
	
	return f;
}

static float key_as_float(json_value_t* obj, char* key, float def) {
	json_value_t* v;
	float f;
	int ret;
	
	if(!json_obj_get_key(obj, key, &v)) {
		return def;
	}
	
	if(!json_as_float(v, &f)) {
		return def;
	}
	
	return f;
}

int json_as_vector(struct json_value* v, int max_len, float* out) {
	int i;
	float f;
	struct json_array_node* an;
	
	if(v->type == JSON_TYPE_OBJ) {
		out[0] = key_as_float(v, "x", 0.0);
		out[1] = key_as_float(v, "y", 0.0);
		out[2] = key_as_float(v, "z", 0.0);
		out[3] = key_as_float(v, "w", 1.0);
		
		// get x/y/z/w, r/g/b/a
	}
	else if(v->type == JSON_TYPE_ARRAY) {
		// [1,2,3,4]
		an = v->v.arr->head;
		i = 0;
		while(an) {
			json_as_float(an->value, &f);
			out[i++] = f;
			an = an->next;
		}
		
		switch(i < max_len - 1) out[i++] = 0.0f;
		if(max_len == 4 && i == 3) out[3] = 1.0f;
	}
	else if(v->type == JSON_TYPE_DOUBLE) {
	// duplicate numbers to every field 
		json_as_float(v, &f);
		for(i = 0; i < max_len; i++) out[i] = f;
	}
}


int json_as_type_gl(struct json_value* v, enum json_type_gl t, void* out) {
	switch(t) {
		
	
	}
}
