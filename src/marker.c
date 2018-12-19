
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "marker.h"
// #include "component.h"

#include "utilities.h"
// #include "objloader.h"
#include "shader.h"
#include "pass.h"


#include "c_json/json.h"






static void uniformSetup(MarkerManager* mm, GLuint progID);
static void instanceSetup(MarkerManager* mm, MarkerInstanceShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp);



static ShaderProgram* prog;





MarkerManager* MarkerManager_alloc(GlobalSettings* gs) {
	
	MarkerManager* mm = pcalloc(mm);
	MarkerManager_init(mm, gs);
	
	return mm;
}


void MarkerManager_init(MarkerManager* mm, GlobalSettings* gs) {
	mm->maxInstances = gs->MarkerManager_maxInstances;
	VEC_INIT(&mm->meshes);
	HT_init(&mm->lookup, 4);
	
	static VAOConfig vaoopts[] = {
		// per vertex
		{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	// 	{0, 3, GL_FLOAT, 0, GL_FALSE}, // normal
		{0, 2, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex
		
		// per instance 
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // position and radius
	 	{1, 2, GL_UNSIGNED_SHORT, 1, GL_FALSE}, // texture indices
		
		{0, 0, 0}
	};
	
	mm->mdi = MultiDrawIndirect_alloc(vaoopts, mm->maxInstances, "markerManager");
	mm->mdi->isIndexed = 1;
	mm->mdi->indexSize = 2;
	mm->mdi->primMode = GL_TRIANGLES;
	mm->mdi->uniformSetup = (void*)uniformSetup;
	mm->mdi->instanceSetup = (void*)instanceSetup;
	mm->mdi->data = mm;
}


void MarkerManager_initGL(MarkerManager* mm, GlobalSettings* gs) {
	MultiDrawIndirect_initGL(mm->mdi);
}



// returns the index if the mesh
int MarkerManager_addMesh(MarkerManager* mm, Marker* m, char* name, int segments) {
	int j, i, index;
	
	
	int vertexCount = (segments + 1) * 2;
	MarkerVertex* vertices = calloc(1, sizeof(*vertices) * vertexCount);
	
	float dt = F_2PI / (segments);
	float ds = 1.0 / segments;
	for(i = 0; i <= segments; i++) {
		vertices[i].v = (Vector){cos(i * dt), sin(i * dt), 0};
		vertices[i].t.u = (i * ds) * 65535;
		vertices[i].t.v = 65535;
	} 
	for(i = 0; i <= segments; i++) {
		vertices[segments + 1 + i].v = (Vector){cos(i * dt), sin(i * dt), 1.0};
		vertices[segments + 1 + i].t.u = (i * ds) * 65535;
		vertices[segments + 1 + i].t.v = 0;
	} 
	
	
	int indexCount = segments * 3 * 2;
	short* indices = calloc(1, sizeof(short) * indexCount);
	for(j = 0, i = 0; i < segments; i++) {
		indices[j++] = i;
		indices[j++] = (i + 1) % (segments+1);
		indices[j++] = (segments + 1) + (( i + 1) % (segments+1));
		
		indices[j++] = i;
		indices[j++] = (segments + 1) + ((i + 1) % (segments+1));
		indices[j++] = (segments + 1) + ((i) % (segments+1));
	}
	
	
	
	MDIDrawInfo* di = pcalloc(di);
	
	*di = (MDIDrawInfo){
		.vertices = vertices,
		.vertexCount = vertexCount,
		
		.indices = indices,
		.indexCount = indexCount,
	};
	
	MultiDrawIndirect_addMesh(mm->mdi, di);
	
	VEC_PUSH(&mm->meshes, m);
	index = VEC_LEN(&mm->meshes);
	
	HT_set(&mm->lookup, name, index -1);
	
	return index - 1;
}


void MarkerManager_addInstance(MarkerManager* mm, int index, MarkerInstance* inst) {
	Marker* m;
	
	m = VEC_ITEM(&mm->meshes, index);
	VEC_PUSH(&m->instances, *inst);
}


void MarkerManager_updateGeometry(MarkerManager* mm) {
	MultiDrawIndirect_updateGeometry(mm->mdi);
}



// returns the index of the instance
int MarkerManager_lookupName(MarkerManager* mm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&mm->lookup, name, &index)) {
		printf("marker found: %s -> %d\n", name, index);
		return index;
	}
	printf("marker not found: %s\n", name);
	return -1;
}


void MarkerManager_readConfigFile(MarkerManager* mm, char* configPath) {
	
	int ret;
	struct json_obj* o;
	void* iter;
	char* key, *texName, *tmp;
	struct json_value* v, *tc;
	json_file_t* jsf;
	
	jsf = json_load_path(configPath);
	
	iter = NULL;
	
	while(json_obj_next(jsf->root, &iter, &key, &tc)) {
		json_value_t* val;
		char* name, *path;
		Marker* m;
		
		pcalloc(m);
		
		name = strdup(key);
		
		
		ret = json_obj_get_key(tc, "texture", &val);
		if(!ret) {
			json_as_string(val, &path);
			
			m->texIndex = TextureManager_reservePath(mm->tm, path);
			//printf("-----mm: %d %s\n", m->texIndex, path);
		}
		
		MarkerManager_addMesh(mm, m, name, 24);
	}
}








static void instanceSetup(MarkerManager* mm, MarkerInstanceShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	int i, j;
	int x, y;
	
	//diCount = 1;
	for(j = 0; j < diCount; j++) {
		Marker* m = VEC_ITEM(&mm->meshes, j);
		di[j]->numToDraw = VEC_LEN(&m->instances);
		
		VEC_EACH(&m->instances, i, inst) {
			vmem->pos = inst.pos;//(Vector){10,10,10};
			vmem->radius = 2.0;
			
			vmem->texIndex = m->texIndex;
			vmem->divisor = 5;
			
			vmem++;
		}
		
		di++;
	}
	
	
}





static void uniformSetup(MarkerManager* mm, GLuint progID) {
	// matrices and uniforms
	GLuint tex_ul;

// 	glActiveTexture(GL_TEXTURE0 + 8);
// 	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tm->tex_id);
	
// 	tex_ul = glGetUniformLocation(progID, "sTexture");
// 	glProgramUniform1i(progID, tex_ul, 8);
	glexit("");
}




RenderPass* MarkerManager_CreateRenderPass(MarkerManager* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = MarkerManager_CreateDrawable(m);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* MarkerManager_CreateDrawable(MarkerManager* m) {
	
	if(!prog) {
		prog = loadCombinedProgram("marker");
		glexit("");
	}
	
	return MultiDrawIndirect_CreateDrawable(m->mdi, prog);
}



