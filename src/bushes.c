 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "common_gl.h"
#include "common_math.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "bushes.h"









BushConfig* BushConfig_alloc() {
	BushConfig* bc = pcalloc(bc);
	bc->numPanes = -1;
	return bc;
}






BushModel* BushModel_FromConfig(BushConfig* bc) {
		size_t l, i;
	
	int ret;
	struct json_value* tex_o;
	struct json_obj* o;
	void* iter;
	char* key, *texName, *tmp;
	struct json_value* v, *tc;
	BushModel* bm;
	
	/*
		// TODO: error handling
	if(jsf->root->type != JSON_TYPE_OBJ) {
		printf("invalid terrain tex config format\n");
		return;
	}

	
	json_obj_get_key(jsf->root, "types", &tex_o);
	if(tex_o->type != JSON_TYPE_OBJ) {
		printf("invalid terrain tex config format (2)\n");
		return;
	}
	
	
	iter = NULL;
	while(json_obj_next(tex_o, &iter, &key, &tc)) {
		char* path;
		
		bm = pcalloc(bm);
		
// 		st->name = strdup(key);
// 		st->diffuse = -1;
// 		st->normal = -1;
// 		VEC_PUSH(&mi->surfaceTypes, st);
		
		if(tc->type == JSON_TYPE_OBJ) {
			json_obj_get_key(tc, "diffuse", &v);
			if(v->type == JSON_TYPE_STRING) {
				json_as_string(v, &path);
// 				st->diffuse = TextureManager_reservePath(mi->tm, path);
			}
			else {
				fprintf(stderr, "!!! Terrain type '%s' missing diffuse texture.\n", key);
			}
			
			json_obj_get_key(tc, "normal", &v);
			if(v->type == JSON_TYPE_STRING) {
				json_as_string(v, &path);
// 				st->normal = TextureManager_reservePath(mi->tm, path);
			}
		}
		else {
			
			json_as_string(tc, &path);
			
// 			st->diffuse = TextureManager_reservePath(mi->tm, path);
		}
		
		
	}
	
	*/
	
	return bm;
}



static void addQuad(BushModel* bm, Vector center, Vector2 size, float rotation, float tilt) {
	
	float hwidth = size.y / 2.0;
	
	float cr = cos(rotation);
	float sr = sin(rotation);
	float ct = cos(tilt);
	float st = sin(tilt);
	
	Vector a = {hwidth * cr, hwidth * sr, 0};
	Vector b = {-a.x, -b.y, 0};
	
	Vector a2 = {
		a.x * size.y * st,
		a.y * size.y * st,
		ct * size.y
	};
	
	Vector b2 = {-a2.x, -a2.y, a2.z};
	
	
	Vector norm;
	vCross(&a, &b, &norm);
	vNorm(&norm, &norm);
	
	int base_index = VEC_LEN(&bm->vertices);
	
	VEC_PUSH(&bm->vertices, ((Vertex_PNT){ p: a,  n: norm, t: {u: 0.0, v: 0.0} }));
	VEC_PUSH(&bm->vertices, ((Vertex_PNT){ p: b,  n: norm, t: {u: 1.0, v: 0.0} }));
	VEC_PUSH(&bm->vertices, ((Vertex_PNT){ p: b2, n: norm, t: {u: 1.0, v: 1.0} }));
	VEC_PUSH(&bm->vertices, ((Vertex_PNT){ p: a2, n: norm, t: {u: 0.0, v: 1.0} }));
	
	VEC_PUSH(&bm->indices, base_index + 0);
	VEC_PUSH(&bm->indices, base_index + 1);
	VEC_PUSH(&bm->indices, base_index + 2);
	
	VEC_PUSH(&bm->indices, base_index + 0);
	VEC_PUSH(&bm->indices, base_index + 2);
	VEC_PUSH(&bm->indices, base_index + 3);
}




BushModel* BushModel_GenCrop(int rows, float width, float length) {
	
	BushModel* bm = pcalloc(bm);
	
	
	float row_w = width / rows;
	float half_row = row_w / 2.0;
	float h_center = length / 2.0;
	
	for(int i = 0; i < rows; i++) {
		addQuad(bm, 
			(Vector){i * row_w + half_row, h_center, 0},
			(Vector2){length, 1.0},
			0.0, 0.0
		);
	}
	
	
	return bm;
}

static GLuint vao, geomVBO, geomIBO;
static GLuint view_ul, proj_ul;
static ShaderProgram* prog;

// static void preFrame(PassFrameParams* pfp, CustomDecalManager* dm);
// static void draw(CustomDecalManager* dm, GLuint progID, PassDrawParams* pdp);
// static void postFrame(CustomDecalManager* dm);

static void uniformSetup(BushManager* mm, GLuint progID);
static void instanceSetup(BushManager* mm, BushInstanceShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp);

void BushManager_updateGeometry(BushManager* mm) {
	MultiDrawIndirect_updateGeometry(mm->mdi);
}

// VAO
static VAOConfig vao_opts[] = {
	// per vertex
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // normal
	{0, 2, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex
	
	// per instance 
	{1, 1, GL_MATRIX_EXT, 1, GL_FALSE}, // model-world matrix
	{1, 4, GL_UNSIGNED_SHORT, 1, GL_FALSE}, // texture indices: diffuse, normal, metallic, roughness
	
	{0, 0, 0}
};


// this is called from _initGL which is run on one thread
static int bushes_are_initialized = 0;
static void initBushes() {
	
	bushes_are_initialized = 1;
	
	// shader
	prog = loadCombinedProgram("bushes");
	
// 	model_ul = glGetUniformLocation(prog->id, "mModel");
// 	view_ul = glGetUniformLocation(prog->id, "mView");
// 	proj_ul = glGetUniformLocation(prog->id, "mProj");
// 	color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("bushes shader");
}



static void instanceSetup(BushManager* mm, BushInstanceShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	int i, j;
	int x, y;
	
	//diCount = 1;
	for(j = 0; j < diCount; j++) {
		BushModel* m = VEC_ITEM(&mm->meshes, j);
		di[j]->numToDraw = VEC_LEN(&m->instances);
		
		VEC_EACH(&m->instances, i, inst) {
// 			vmem->pos = inst.pos;//(Vector){10,10,10};
// 			vmem->radius = 2.0;
// 			
// 			vmem->texIndex = m->texIndex;
// 			vmem->divisor = 5;
			
			vmem++;
		}
		
		di++;
	}
	
	
}





static void uniformSetup(BushManager* mm, GLuint progID) {
	// matrices and uniforms
	GLuint tex_ul;

// 	glActiveTexture(GL_TEXTURE0 + 8);
// 	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tm->tex_id);
	
// 	tex_ul = glGetUniformLocation(progID, "sTexture");
// 	glProgramUniform1i(progID, tex_ul, 8);
	glexit("");
}





BushManager* BushManager_alloc(GlobalSettings* gs) {
	BushManager* bm = pcalloc(bm);
	BushManager_init(bm, gs);
	return bm;
}

void BushManager_init(BushManager* bmm, GlobalSettings* gs) {
	
	bmm->mdi = MultiDrawIndirect_alloc(vao_opts, gs->BushManager_maxInstances, "bushManager");
	bmm->mdi->isIndexed = 1;
	bmm->mdi->indexSize = 2;
	bmm->mdi->primMode = GL_TRIANGLES;
	bmm->mdi->uniformSetup = (void*)uniformSetup;
	bmm->mdi->instanceSetup = (void*)instanceSetup;
	bmm->mdi->data = bmm;
}

void BushManager_initGL(BushManager* bmm, GlobalSettings* gs) {
	
	if(!bushes_are_initialized) initBushes();
	
	MultiDrawIndirect_initGL(bmm->mdi);
}



RenderPass* BushManager_CreateRenderPass(BushManager* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = BushManager_CreateDrawable(m);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	
	return rp;
}


PassDrawable* BushManager_CreateDrawable(BushManager* m) {
	
	if(!prog) {
		prog = loadCombinedProgram("bushes");
		glexit("");
	}
	
	return MultiDrawIndirect_CreateDrawable(m->mdi, prog);
}



