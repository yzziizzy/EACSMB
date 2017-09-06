


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities.h"
#include "common_math.h"

#include "c_json/json.h"
#include "json_gl.h"


#include "meshBuilder.h"



static MeshData* compose(MB_compose_params* params);
static MeshData* process_op(MB_operation* op);
static void append_mesh(MeshData* in, MeshData* out);
static void createCylinder(MB_cylinder_params* params, MeshData* md);


static MeshData* mdcreate() {
	MeshData* md;
	
	md = calloc(1, sizeof(*md));
	CHECK_OOM(md);
	
	VEC_INIT(&md->verts);
	VEC_INIT(&md->indices);
	
	return md;
}


static void append_mesh(MeshData* in, MeshData* out) {
	int i, len;
	int base_vertex = VEC_LEN(&out->verts) - 1;
	
	// copy vertices
	len = VEC_LEN(&in->verts);
	
	for(i = 0; i < len; i++) {
		VEC_PUSH(&out->verts, VEC_ITEM(&in->verts, i));
	}
	
	
	// copy and adjust indices
	len = VEC_LEN(&in->indices);
	
	for(i = 0; i < len; i++) {
		VEC_PUSH(&out->indices, VEC_ITEM(&in->indices, i) + base_vertex );
	}
}


static MeshData* compose(MB_compose_params* params) {
	MeshData* md;
	int i, len;
	
	md = mdcreate();
	len = VEC_LEN(&params->children);
	
	for(i = 0; i < len; i++) {
		MeshData* mdc;
		
		mdc = process_op(&VEC_ITEM(&params->children, i));
		append_mesh(mdc, md);
		
		free(mdc);
	}
	
	return md;
}

static MeshData* process_op(MB_operation* op) {
	MeshData* md;
	int i;
	
	
	if(op->type == MB_OP_COMPOSE) {
		return compose(op);
	}
	else if(op->type == MB_OP_CREATE_CYLINDER) {
		createCylinder(op, md);
	}
	else {
		fprintf(stderr, "!!! MeshBuilder: unimplemented operation.\n");
	}
	
	
}

static void transform(MB_transform_params* params, MeshData* md) {
	
	int i;
	MBVList* l = &md->verts;
	int len = VEC_LEN(l);
	float scale;
/*	 TODO: rewrite with matrices
	// translate position
	if(vMag(&params->position) > 0.00001) {
		for(i = 0; i < len; i++) {
			vAdd(&params->position, &VEC_ITEM(l, i).v, &VEC_ITEM(l, i).v);
		}
	}
	
	// rotation
	if(vMag(&params->direction) > 0.00001 || fabs(params->rotation) > 0.0001 ) {
		fprintf(stderr, "!!! MeshBuilder: transform rotation specified but not implemented.\n");
	}
	
	// scale
	scale = vMag(&params->scale);
	if(scale > 1.00001 || scale < .9999) {
		for(i = 0; i < len; i++) {
			vMul(&params->scale, &VEC_ITEM(l, i).v, &VEC_ITEM(l, i).v);
		}
	}
	*/
	
}



static void createTetrahedron(float size, MeshData* md) {
	MeshBuilderVertex vert;
	
	int start_vertex = VEC_LEN(&md->verts);
	
	
	vert = (MeshBuilderVertex){
		.v = {0, 0, 0},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	vert = (MeshBuilderVertex){
		.v = {size, 0, 0},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	vert = (MeshBuilderVertex){
		.v = {0, size, 0},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	vert = (MeshBuilderVertex){
		.v = {0, 0, size},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	// bottom triangle
	VEC_PUSH(&md->indices, start_vertex + 0);
	VEC_PUSH(&md->indices, start_vertex + 1);
	VEC_PUSH(&md->indices, start_vertex + 2);
	
	// hyp face
	VEC_PUSH(&md->indices, start_vertex + 1);
	VEC_PUSH(&md->indices, start_vertex + 2);
	VEC_PUSH(&md->indices, start_vertex + 3);

	// vert faces
	VEC_PUSH(&md->indices, start_vertex + 0);
	VEC_PUSH(&md->indices, start_vertex + 2);
	VEC_PUSH(&md->indices, start_vertex + 3);
	
	VEC_PUSH(&md->indices, start_vertex + 0);
	VEC_PUSH(&md->indices, start_vertex + 1);
	VEC_PUSH(&md->indices, start_vertex + 3);
	
}


static void createCylinder(MB_cylinder_params* params, MeshData* md) {
	int i, j;
	MeshBuilderVertex vert;
	
	// save for constructing the indices.
	int start_vertex = VEC_LEN(&md->verts) - 1;
	
	int sections = params->linear_segments;
	float radius = params->radius;
	float length = params->length;
	
	// create vertices
	for(j = 0; j < 2; j++) {
		for(i = 0; i < sections; i++) {
			float theta = (i * D_2PI) / sections;
			
			vert.v.y = j * length;
			vert.v.x = cos(theta) * radius;
			vert.v.z = sin(theta) * radius;
			
			vert.n.y = 0;
			vert.n.x = cos(theta);
			vert.n.z = sin(theta);
			
			vert.t.u = j * (65535 / 1);//(j / length) * 65536; 
			vert.t.v = i * (65535 / sections); //(i / sections) * 65536; 
			
			VEC_PUSH(&md->verts, vert);
		}
	}
	
	// fill in indices
	int ring1 = start_vertex, ring2 = start_vertex + sections;
	for(i = 0; i < sections; i++) {
		int r1v1 = ring1 + i;
		int r1v2 = ring1 + ((i + 1) % sections);
		int r2v1 = ring2 + i;
		int r2v2 = ring2 + ((i + 1) % sections);
		
		// triangle 1
		VEC_PUSH(&md->indices, r1v1);
		VEC_PUSH(&md->indices, r2v1);
		VEC_PUSH(&md->indices, r1v2);
		
		// triangle 2
		VEC_PUSH(&md->indices, r1v2);
		VEC_PUSH(&md->indices, r2v1);
		VEC_PUSH(&md->indices, r2v2);
	}
	
	if(params->cap_min || params->cap_max) {
		fprintf(stderr, "!!! MeshBuilder: cylinder end caps specified but not implemented.\n");
	}
}














/////// json parsing ////////

static MB_operation* handle_obj(json_value_t* obj);

static struct {char* name; int code;} op_lookup[] = {
	{"none", MB_OP_NONE},
	{"compose", MB_OP_COMPOSE},
	{"cylinder", MB_OP_CREATE_CYLINDER},
	{"cube", MB_OP_CREATE_CUBE},
	{"sphere", MB_OP_CREATE_SPHERE},
	{"pyramid", MB_OP_CREATE_PYRAMID}
};

static int lookup_name(char* n) {
	int i;
	int len = sizeof(op_lookup) / sizeof(op_lookup[0]);
	for(i = 0; i < len; i++) {
		if(0 == strcmp(n, op_lookup[i].name)) return op_lookup[i].code;
	}
	return MB_OP_NONE;
}





static MB_operation* handle_cylinder(json_value_t* obj) {
	MB_cylinder_params* params;
	json_value_t* v;
	int64_t n;
	
	params = calloc(1, sizeof(*params));
	CHECK_OOM(params);
	
	json_obj_get_key(obj, "length", &v);
	json_as_float(v, &params->length);
	
	json_obj_get_key(obj, "radius", &v);
	json_as_float(v, &params->length);

	json_obj_get_key(obj, "linear_segments", &v);
	json_as_int(v, &n);
	params->linear_segments = n;
	
	json_obj_get_key(obj, "radial_segments", &v);
	json_as_int(v, &n);
	params->radial_segments = n;
	
	// TODO: cap_min, cap_max
	
	return params;
}


static MB_operation* handle_arr(json_value_t* arr) {
	json_array_node_t* n;
	MB_compose_params* params;
	
	params = calloc(1, sizeof(*params));
	CHECK_OOM(params);
	
	n = arr->v.arr->head;
	while(n) {
		json_value_t* v;
		MB_operation* mbop;
		
		v = n->value;
		
		mbop = handle_obj(v);
		if(mbop) {
			VEC_PUSH(&params->children, *mbop);
		}
		
		
		n = n->next;
	}
	
	return params;
}

static MB_operation* handle_compose(json_value_t* obj) {
	// this is just a wrapper for handle_arr, which is the implicit version
	json_value_t* v;
	json_obj_get_key(obj, "children", &v);
	
	return handle_arr(v);
}


static MB_operation* handle_obj(json_value_t* obj) {
	enum MB_op_type type;
	char* tname;
	json_value_t* v;
	
	static MB_operation* (*fntable[])(json_value_t*) = {
		NULL,
		handle_compose,
		handle_cylinder
	};
	
	if(obj->type == JSON_TYPE_ARRAY) {
		return handle_arr(obj);
	}
	
	// peek the op and hand off to the appropriate handler
	json_obj_get_key(obj, "op", &v);
	json_as_string(v, &tname);
	type = lookup_name(tname);
	
	if(type == MB_OP_NONE) {
		fprintf(stderr, "!!! MeshBuilder: json parser: unknown operation '%s'\n", tname);
		return NULL;
	}
	
	return fntable[type](obj);
}

static MB_operation* read_json(char* path) {
	
	json_file_t* jsf;
	
	jsf = json_load_path(path);
	if(jsf) {
		fprintf(stderr, "!!! MeshBuilder: could not read file '%s'", path);
		return NULL;
	}
	
	return handle_obj(jsf->root);
	
	// TODO: json cleanup
}


void meshBuilder_test() {
	
	
	
	
	
}
