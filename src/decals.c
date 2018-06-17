
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "decals.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "pass.h"

#include "c_json/json.h"


static GLuint vao, geomVBO;
static GLuint view_ul, proj_ul;
static ShaderProgram* prog;




static void preFrame(PassFrameParams* pfp, DecalManager* dm);
static void draw(PassDrawParams* pdp, GLuint progID, DecalManager* dm);
static void postFrame(DecalManager* dm);





void initDecals() {
	
	PassDrawable* pd;
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{2, GL_UNSIGNED_SHORT}, // tex
		
		// per instance 
		{4, GL_FLOAT}, // pos, size
		{4, GL_FLOAT}, // rot, alpha, unused1/2
		{2, GL_SHORT}, // tex index and tiling info
		
		{0, 0}
	};
	
	vao = makeVAO(opts);

	glexit("decals vao");
	
	// shader
	prog = loadCombinedProgram("decals");
	
	//model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	//color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("decals shader");

	
	// global decal geometry box
	Vector box_vertices[] = {
		// front face
		{-.5, -.5,  .5},
		{-.5,  .5,  .5},
		{ .5,  .5,  .5},
		{ .5, -.5,  .5},
		// back face
		{-.5, -.5, -.5},
		{-.5,  .5, -.5},
		{ .5,  .5, -.5},
		{ .5, -.5, -.5}
	};
	
	unsigned short box_indices[] = {
		0,1,2, 2,3,0,
		4,5,6, 6,7,4,
		0,1,4, 1,4,5,
		3,4,7, 3,0,4,
		2,5,6, 0,2,5,
		2,3,6, 3,6,7,
	};
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(geomVBO)) glDeleteBuffers(1, &geomVBO);
	glGenBuffers(1, &geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, geomVBO);
	

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1*3*4 + 4, 0);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, 1*3*4 + 4, 1*3*4);

	glBufferStorage(GL_ARRAY_BUFFER, sizeof(box_vertices), NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	memcpy(buf, box_vertices, sizeof(box_vertices));
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	// global decal box gemotry index buffer	
	if(glIsBuffer(mm->geomIBO)) glDeleteBuffers(1, &mm->geomIBO);
	glGenBuffers(1, &mm->geomIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->geomIBO);
	
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(box_indices), NULL, GL_MAP_WRITE_BIT);

	uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	memcpy(ib, box_indices, sizeof(box_indices));
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	
	
	pd = Pass_allocDrawable("decal manager");
	pd->preFrame = preFrame;
	pd->draw = draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	Pass_registerDrawable(pd);
}








DecalManager*  DecalManager_alloc(int maxInstances) {
	DecalManager* dm;
	GLbitfield flags;
	size_t vbo_size;
	
	
	dm = calloc(1, sizeof(*dm));

	VEC_INIT(&dm->decals);
	HT_init(&dm->lookup, 6);
	//HT_init(&mm->textureLookup, 6);
	
	dm->maxInstances = maxInstances;
	
	glBindVertexArray(vao);
	
	
	// per-instance attributes
	PCBuffer_startInit(&dm->instVB, dm->maxInstances * sizeof(Matrix), GL_ARRAY_BUFFER);

	// position matrix 	
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4*4*2 + 2*2, 0);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*2 + 2*2, 1*4*4);
	
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	
	// texture indices
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 2, GL_UNSIGNED_SHORT, 4*4*2 + 2*2, 2*4*4);
	glVertexAttribDivisor(4, 1);
	
	
	PCBuffer_finishInit(&dm->instVB);
	
	
	PCBuffer_startInit(
		&dm->indirectCmds, 
		16 * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&dm->indirectCmds);

	
	
	return dm;
}





void DecalManager_readConfigFile(DecalManager* dm, char* configPath) {
	int ret;
	struct json_obj* o;
	void* iter;
	char* key, *texName, *tmp;
	struct json_value* v, *tc;
	json_file_t* jsf;
	
	
	jsf = json_load_path(configPath);
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "textures", &tex);
	
	
	iter = NULL;
	
	while(json_obj_next(jsf->root, &iter, &key, &tc)) {
		json_value_t* val;
		char* path;
		DynamicMesh* dm;
		
		OBJContents obj;
		
		ret = json_obj_get_key(tc, "mesh", &val);
		json_as_string(val, &path);
		
		loadOBJFile(path, 0, &obj);
		dm = DynamicMeshFromOBJ(&obj);
		dm->name = strdup(key);
		
		
		ret = json_obj_get_key(tc, "texture", &val);
		if(!ret) {
			json_as_string(val, &path);
			
			dm->texIndex = TextureManager_reservePath(mm->tm, path);
			printf("dmm: %d %s\n", dm->texIndex, path);
		}

#define grab_json_val(str, field, def) \
		dm->field = def; \
		if(!json_obj_get_key(tc, str, &val)) { \
			json_as_float(val, &dm->field); \
		}

		grab_json_val("scale", defaultScale, 1.0)
		grab_json_val("rotDegX", defaultRotX, 0.0)
		grab_json_val("rotDegY", defaultRotY, 0.0)
		grab_json_val("rotDegZ", defaultRotZ, 0.0)
		
		// radians are not easy to edit in a config file, so it's in degrees
		dm->defaultRotX *= F_PI / 180.0;  
		dm->defaultRotY *= F_PI / 180.0;  
		dm->defaultRotZ *= F_PI / 180.0;  
		

		int ind = dynamicMeshManager_addMesh(mm, dm->name, dm);
		printf("DM added mesh %d: %s \n", ind, dm->name);
		
	}
	
}


// returns the index of the instance
int DecalManager_addInstance(DecalManager* dm, int index, const DecalInstance* di) {
	
	Decal* d; 
	
	if(index >= VEC_LEN(&dm->decals)) {
		fprintf(stderr, "decal manager addInstance out of bounds: %d, %d\n", (int)VEC_LEN(&dm->decals), index);
		return -1;
	}
	
	dm->totalInstances++;
	
	//printf("adding instance: %d ", meshIndex);
	d = VEC_ITEM(&dm->decals, index);
	VEC_PUSH(&d->instances, *di);
//	VEC_PUSH(&d->instances[1], *di);
	//VEC_INC(&d->instMatrices);
	
	//printf("add instance: %d", mm->totalInstances, VEC_LEN(&msh->instances[0]));
	
	return VEC_LEN(&d->instances);
}

// returns the index of the instance
int DecalManager_lookupName(DecalManager* dm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&dm->lookup, name, &index)) {
		//printf("decal found: %s -> %d\n", name, index);
		return index;
	}
	
	printf("decal not found: %s\n", name);
	return -1;
}


// returns the index if the decal
int DecalManager_addDecal(DecalManager* dm, char* name, Decal* d) {
	int index;
	
	VEC_PUSH(&dm->decals, d);
	//mm->totalVertices += m->vertexCnt;
	//mm->totalIndices += sm->indexCnt;
	index = VEC_LEN(&dm->decals);
	
	HT_set(&dm->lookup, name, index -1);
	
	return index - 1;
}




void DecalManager_updateMatrices(DecalManager* dmm, PassFrameParams* pfp) {
	int mesh_index, i;
	
	
	
	DecalInstance* vmem = PCBuffer_beginWrite(&dmm->instVB);
	
	if(!vmem) {
		printf("attempted to update invalid dynamic mesh manager\n");
		return;
	}

	
	for(decal_index = 0; decal_index < VEC_LEN(&dm->meshes); decal_index++) {
		Decal* d = VEC_ITEM(&dm->meshes, decal_index);
		d->numToDraw = 0;
		
		// TODO make instances switch per frame
		for(i = 0; i < VEC_LEN(&dm->instances[0]); i++) {
			DecalInstance* di = &VEC_ITEM(&d->instances[0], i);
			
			float d = vDist(&di->pos, &pfp->dp->eyePos);
				
		//	printf("d %f -- ", d);
		//	printf("%f, %f, %f -- ", dmi->pos.x, dmi->pos.y, dmi->pos.z);
		//	printf("%f, %f, %f\n", pfp->dp->eyePos.x, pfp->dp->eyePos.y, pfp->dp->eyePos.z);
			
			if(d > 500) continue;
			dm->numToDraw++;
			
			mTransv(&di->pos, &m);
			
			// HACK
			Vector noise;
			vRandom(&(Vector){-1,-1,-1}, &(Vector){1,1,1}, &noise);
			//mTransv(&noise, &m);
			
			// z-up hack for now
			mRot3f(1, 0, 0, F_PI / 2, &m);
			mRot3f(1, 0, 0, dm->defaultRotX, &m);
			mRot3f(0, 1, 0, dm->defaultRotY, &m);
			mRot3f(0, 0, 1, dm->defaultRotZ, &m);
			mScale3f(dm->defaultScale, dm->defaultScale, dm->defaultScale, &m);
			
			//mScalev(&dmi->scale, &m);
			
			// only write sequentially. random access is very bad.
			vmem->m = m;
			vmem->diffuseIndex = dm->texIndex;
			vmem->normalIndex = 0;
			
			vmem++;
		}
		//vmem += VEC_LEN(&dm->instances);
	//	printf("num to draw %d\n", dm->numToDraw);
	}
	
}








// -------------------------------------------------------------------


static void preFrame(PassFrameParams* pfp, DecalManager* dm) {
	
	dynamicMeshManager_updateMatrices(mm, pfp);

	
	
	DrawArraysIndirectCommand* cmds = PCBuffer_beginWrite(&mm->indirectCmds);
	
	int index_offset = 0;
	int instance_offset = 0;
	int decal_index;
	for(decal_index = 0; decal_index < VEC_LEN(&mm->meshes); decal_index++) {
		DynamicMesh* dm = VEC_ITEM(&mm->meshes, decal_index);
			
		cmds[mesh_index].first = index_offset; // offset of this mesh into the instances
		cmds[mesh_index].count = dm->indexCnt; // number of polys
		
		// offset into instanced vertex attributes
		cmds[mesh_index].baseInstance = (mm->maxInstances * ((mm->instVB.nextRegion) % PC_BUFFER_DEPTH)) + instance_offset; 
		// number of instances
		cmds[mesh_index].instanceCount = dm->numToDraw; //VEC_LEN(&dm->instances[0]); 
	//printf("instances %d %d %d %d  \n", mesh_index, dm->indexCnt, VEC_LEN(&dm->instances[0]), instance_offset );
		
		index_offset += dm->indexCnt;// * sizeof(DynamicMeshVertex);//dm->indexCnt;
		instance_offset += VEC_LEN(&dm->instances[0]);
	}
}


void dynamicMeshManager_draw(DynamicMeshManager* mm, PassFrameParams* pfp) {
	
	GLuint tex_ul;

	
	// TODO: move to intermediate initialization stage
	glActiveTexture(GL_TEXTURE0 + 9);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mm->tm->tex_id);
	
	tex_ul = glGetUniformLocation(prog->id, "sTexture");
	glProgramUniform1i(prog->id, tex_ul, 9);
	
	glUseProgram(prog->id);
	glexit("");

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &pfp->dp->mWorldView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &pfp->dp->mViewProj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	

	//printf("instance count %d, %d\n", cmds[0].instanceCount, cmds[0].count);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->geomIBO);
	
	
	PCBuffer_bind(&mm->instVB);
	
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	

	
	PCBuffer_bind(&dm->indirectCmds);
	
	// there's just one mesh for decals atm
 	glMultiDrawArraysIndirect(GL_TRIANGLES, 0, 1/*VEC_LEN(&dm->decals)*/, 0);
	glexit("multidrawarraysindirect");
	
	PCBuffer_afterDraw(&dm->instVB);
	PCBuffer_afterDraw(&dm->indirectCmds);
	
}



tatic void postFrame(DecalManager* mm) {
	PCBuffer_afterDraw(&dm->instVB);
	PCBuffer_afterDraw(&dm->indirectCmds);
}
