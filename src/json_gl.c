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
			*out = strtol(v->v.str, NULL, 0);
			return 0;
			
		case JSON_TYPE_OBJ: // all invalid
		case JSON_TYPE_ARRAY:
		case JSON_TYPE_COMMENT_SINGLE:
		case JSON_TYPE_COMMENT_MULTI:
		default:
			*out = 0;
			return 1;
	}
}


int json_as_type_gl(struct json_value* v, enum json_type_gl t, void* out) {
	switch(t) {
		
	
	}
}
