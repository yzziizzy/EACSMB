
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "decalsCustom.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "pass.h"

#include "c_json/json.h"


static GLuint vao, geomVBO, geomIBO;
static GLuint view_ul, proj_ul;
static ShaderProgram* prog;




static void preFrame(PassFrameParams* pfp, CustomDecalManager* dm);
static void draw(CustomDecalManager* dm, GLuint progID, PassDrawParams* pdp);
static void postFrame(CustomDecalManager* dm);





void initCustomDecals() {
	
	PassDrawable* pd;
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{2, GL_UNSIGNED_SHORT}, // tex
		
		// per instance 
		{4, GL_FLOAT}, // pos1, thickness
		{4, GL_FLOAT}, // pos2, alpha, unused1/2
		{4, GL_FLOAT}, // pos3, unused1
		{4, GL_FLOAT}, // pos4, unused2
		{2, GL_SHORT}, // tex index and tiling info
		
		{0, 0}
	};
	
	vao = makeVAO(opts);

	glexit("custom decals vao");
	
	// shader
	prog = loadCombinedProgram("decalsCustom");
	
	//model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mWorldView");
	proj_ul = glGetUniformLocation(prog->id, "mViewProj");
	//color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("custom decals shader");

	
	// global decal geometry box
	CustomDecalVertex box_vertices[] = {
		//// front face
		//{{-.5, -.5,  .5}, {0,0}},
		//{{-.5,  .5,  .5}, {0,0}},
		//{{ .5,  .5,  .5}, {0,0}},
		//{{ .5, -.5,  .5}, {0,0}},
		//// back face
		//{{-.5, -.5, -.5}, {0,0}},
		//{{-.5,  .5, -.5}, {0,0}},
		//{{ .5,  .5, -.5}, {0,0}},
		//{{ .5, -.5, -.5}, {0,0}},
		// front face
		{{-.5, -.5,  .5}, {3,0}},
		{{-.5,  .5,  .5}, {2,0}},
		{{ .5,  .5,  .5}, {0,0}},
		{{ .5, -.5,  .5}, {1,0}},
		// back face
		{{-.5, -.5, -.5}, {3+4,0}},
		{{-.5,  .5, -.5}, {2+4,0}},
		{{ .5,  .5, -.5}, {0+4,0}},
		{{ .5, -.5, -.5}, {1+4,0}},
	};
	
	/*
	0 - iiiii
	1 - iii
	2 - iiiii
	3 - iiiii
	4 - iiiiii
	5 - iiii
	6 - iiiii
	7 - iii
	*/
	
	unsigned short box_indices[] = {
		
		//0,1,2, 0,2,3,
		//0,0,0,
		
		//0,0,0, 0,0,0,
		//0,0,0, 0,0,0,
		//0,0,0, 0,0,0,
		//0,0,0, 0,0,0,
		//0,0,0, 0,0,0,
		//0,0,0, 0,0,0,
		
		
		4,5,6, 4,6,7, // -z
		0,1,2, 0,2,3, // +z
		0,1,4, 1,4,5, // -x
		2,3,6, 3,6,7, // +x
		0,3,4, 3,4,7, // -y
		1,2,5, 2,5,6, // +y
		
	};
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(geomVBO)) glDeleteBuffers(1, &geomVBO);
	glGenBuffers(1, &geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, geomVBO);
	

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1*3*4 + 4, 0);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, 1*3*4 + 4, 1*3*4);

	glBufferStorage(GL_ARRAY_BUFFER, sizeof(box_vertices), NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	memcpy(buf, box_vertices, sizeof(box_vertices));
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	

	// global decal box gemotry index buffer	
	if(glIsBuffer(geomIBO)) glDeleteBuffers(1, &geomIBO);
	glGenBuffers(1, &geomIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomIBO);
	
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(box_indices), NULL, GL_MAP_WRITE_BIT);

	uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	memcpy(ib, box_indices, sizeof(box_indices));
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	
	
	//pd = Pass_allocDrawable("decal manager");
	//pd->preFrame = preFrame;
	//pd->draw = draw;
	//pd->postFrame = postFrame;
	//pd->prog = prog;
	
	//Pass_registerDrawable(pd);
}




// rendering overhead
RenderPass* CustomDecalManager_CreateRenderPass(CustomDecalManager* lm) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = CustomDecalManager_CreateDrawable(lm);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp, prog);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* CustomDecalManager_CreateDrawable(CustomDecalManager* lm) {
	PassDrawable* pd;

	pd = Pass_allocDrawable("CustomDecalManager");
	pd->data = lm;
	pd->preFrame = preFrame;
	pd->draw = (PassDrawFn)draw;
	pd->postFrame = postFrame;
	
	return pd;
}




CustomDecalManager*  CustomDecalManager_alloc(int maxInstances) {
	CustomDecalManager* dm;
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
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 0);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 1*4*4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 2*4*4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 3*4*4);
	
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	
	// texture indices
	glEnableVertexAttribArray(6);
	glVertexAttribIPointer(6, 2, GL_UNSIGNED_SHORT, 4*4*4 + 2*2, 4*4*4);
	glVertexAttribDivisor(6, 1);
	
	
	PCBuffer_finishInit(&dm->instVB);
	
	
	PCBuffer_startInit(
		&dm->indirectCmds, 
		16 * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&dm->indirectCmds);

	
	
	return dm;
}





void CustomDecalManager_readConfigFile(CustomDecalManager* dm, char* configPath) {
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
		CustomDecal* d;
		
		//OBJContents obj;
		
		//ret = json_obj_get_key(tc, "mesh", &val);
		//json_as_string(val, &path);
		
		//loadOBJFile(path, 0, &obj);
		//d = DynamicMeshFromOBJ(&obj);
		pcalloc(d);
		d->name = strdup(key);
		
		
		ret = json_obj_get_key(tc, "texture", &val);
		if(!ret) {
			json_as_string(val, &path);
			
			d->texIndex = TextureManager_reservePath(dm->tm, path);
			printf("cdm: %d %s\n", d->texIndex, path);
		}

#define grab_json_val(str, field, def) \
		d->field = def; \
		if(!json_obj_get_key(tc, str, &val)) { \
			json_as_float(val, &d->field); \
		}

		//grab_json_val("thickness", thickness, 1.0)
		

		int ind = CustomDecalManager_AddDecal(dm, d->name, d);
		printf("CDM added decal %d: %s \n", ind, d->name);
		
	}
	
}


// returns the index of the instance
int CustomDecalManager_AddInstance(CustomDecalManager* dm, int index, const CustomDecalInstance* di) {
	
	CustomDecal* d; 
	
	if(index >= VEC_LEN(&dm->decals)) {
		fprintf(stderr, "decal manager addInstance out of bounds: %d, %d\n", (int)VEC_LEN(&dm->decals), index);
		return -1;
	}
	
	dm->totalInstances++;
	
	//printf("adding instance: %d ", meshIndex);
	d = VEC_ITEM(&dm->decals, index);
	
	VEC_PUSH(&d->instances, *di);
	VEC_TAIL(&d->instances).thickness = d->thickness;
	//VEC_PUSH(&d->instances[1], *di);
	//VEC_INC(&d->instMatrices);
	
	//printf("add instance: %d", mm->totalInstances, VEC_LEN(&msh->instances[0]));
	
	return VEC_LEN(&d->instances);
}

// returns the index of the instance
int CustomDecalManager_lookupName(CustomDecalManager* dm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&dm->lookup, name, &index)) {
		//printf("decal found: %s -> %d\n", name, index);
		return index;
	}
	
	printf("decal not found: %s\n", name);
	return -1;
}


// returns the index if the decal
int CustomDecalManager_AddDecal(CustomDecalManager* dm, char* name, CustomDecal* d) {
	int index;
	
	VEC_PUSH(&dm->decals, d);
	//mm->totalVertices += m->vertexCnt;
	//mm->totalIndices += sm->indexCnt;
	index = VEC_LEN(&dm->decals);
	
	HT_set(&dm->lookup, name, index -1);
	
	return index - 1;
}




void CustomDecalManager_updateMatrices(CustomDecalManager* dm, PassFrameParams* pfp) {
	int decal_index, i;
	
	
	
	CustomDecalInstance* vmem = PCBuffer_beginWrite(&dm->instVB);
	
	if(!vmem) {
		printf("attempted to update invalid dynamic mesh manager\n");
		return;
	}

	
	for(decal_index = 0; decal_index < VEC_LEN(&dm->decals); decal_index++) {
		CustomDecal* d = VEC_ITEM(&dm->decals, decal_index);
		d->numToDraw = 0;
		
		// TODO make instances switch per frame
		for(i = 0; i < VEC_LEN(&d->instances); i++) {
			CustomDecalInstance* di = &VEC_ITEM(&d->instances, i);
			
			float dist = vDist(&di->pos1, &pfp->dp->eyePos);
				
		//	printf("d %f -- ", d);
		//	printf("%f, %f, %f -- ", di->pos.x, di->pos.y, di->pos.z);
		//	printf("%f, %f, %f\n", pfp->dp->eyePos.x, pfp->dp->eyePos.y, pfp->dp->eyePos.z);
			
			//if(dist > 500) continue;
			d->numToDraw++;
			
			// only write sequentially. random access is very bad.
			*vmem = *di;
			
			vmem++;
		}
		//vmem += VEC_LEN(&dm->instances);
		//printf("num to draw %d\n", d->numToDraw);
	}
	
}








// -------------------------------------------------------------------


static void preFrame(PassFrameParams* pfp, CustomDecalManager* dm) {
	
	CustomDecalManager_updateMatrices(dm, pfp);

	
	
	DrawElementsIndirectCommand* cmds = PCBuffer_beginWrite(&dm->indirectCmds);
	
	int index_offset = 0;
	int instance_offset = 0;
	int decal_index;
	for(decal_index = 0; decal_index < VEC_LEN(&dm->decals); decal_index++) {
		CustomDecal* d = VEC_ITEM(&dm->decals, decal_index);
			
		cmds[decal_index].firstIndex = index_offset; // offset of this mesh into the instances
		cmds[decal_index].count = 36;//dm->indexCnt; // number of polys
		
		// offset into instanced vertex attributes
		cmds[decal_index].baseInstance = (dm->maxInstances * ((dm->instVB.nextRegion) % PC_BUFFER_DEPTH)) + instance_offset; 
		// number of instances
		cmds[decal_index].instanceCount = d->numToDraw; //VEC_LEN(&dm->instances[0]); 
	//printf("instances %d %d %d %d  \n", mesh_index, dm->indexCnt, VEC_LEN(&dm->instances[0]), instance_offset );
		cmds[decal_index].baseVertex = 0;
		
		index_offset += 36;//dm->indexCnt;// * sizeof(DynamicMeshVertex);//dm->indexCnt;
		instance_offset += VEC_LEN(&d->instances);
	}
}


void draw(CustomDecalManager* dm, GLuint progID, PassDrawParams* pdp) {
	
	GLuint tex_ul;

glexit("");
	glUseProgram(prog->id);
	glexit("");

	
	// TODO: move to intermediate initialization stage
	glActiveTexture(GL_TEXTURE0 + 9);
glexit("");
	glBindTexture(GL_TEXTURE_2D_ARRAY, dm->tm->tex_id);
	//printf("tex id: %d\n", dm->tm->tex_id);
glexit("");
	
	tex_ul = glGetUniformLocation(prog->id, "sTexture");
glexit("");
	glProgramUniform1i(prog->id, tex_ul, 9);
	glexit("");


	glActiveTexture(GL_TEXTURE0 + 23);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, dm->dtex);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 23);
	

	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &pdp->mWorldView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &pdp->mViewProj->m);
	
	

	//printf("instance count %d, %d\n", cmds[0].instanceCount, cmds[0].count);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, geomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomIBO);
	
	PCBuffer_bind(&dm->instVB);	
	PCBuffer_bind(&dm->indirectCmds);
	
	// there's just one mesh for decals atm
 	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, 1,/*VEC_LEN(&dm->decals),*/ 0);
	glexit("multidrawelementsindirect");
	
}



static void postFrame(CustomDecalManager* dm) {
	PCBuffer_afterDraw(&dm->instVB);
	PCBuffer_afterDraw(&dm->indirectCmds);
}
