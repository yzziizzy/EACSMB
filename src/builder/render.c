

#include "render.h" 
#include "builder.h" 
#include "game.h" 



/*

	MeshData* md;
	MB_operation* root;
	
	
	root = read_json("assets/models/test.json");
	if(!root) {
		printf("failed to read test json file\n");
		exit(1);
	}
	
	md = process_op(root);
	if(!md) {
		printf("failed to process mesh operations \n");
		exit(1);
	}
*/




void updateMeshManager() {
	/*

	sm = calloc(1, sizeof(*sm));
	CHECK_OOM(sm);

	sm->vertexCnt = VEC_LEN(&md->verts);
	sm->indexCnt = VEC_LEN(&md->indices);
	sm->indexWidth = 2;
	
	printf("verts: %d, indices: %d \n", sm->vertexCnt, sm->indexCnt);
	
	
	sm->vertices = malloc(sizeof(*sm->vertices) * sm->vertexCnt);
	CHECK_OOM(sm->vertices);
	sm->indices.w16 = malloc(sizeof(*sm->indices.w16) * sm->indexCnt);
	CHECK_OOM(sm->indices.w16);

	for(i = 0; i < sm->vertexCnt; i++) sm->vertices[i] = *((StaticMeshVertex*)&VEC_ITEM(&md->verts, i));
	for(i = 0; i < sm->indexCnt; i++) sm->indices.w16[i] = VEC_ITEM(&md->indices, i);
	
	
	
	StaticMesh_updateBuffers(sm);
	*/
}
void guiBuilderControlRender(GUIBuilderControl* bc, GameState* gs, PassFrameParams* pfp) {
// 	bc->rt->texID = RenderPipeline_getOutputTexture(bc->rpipe);
// 	
// 	guiRender(bc->rt, gs, pfp);
// 	
// 	if(bc->ed)
// 		guiRender(bc->ed, gs, pfp);
	
	//printf("sdf\n");
}

void guiBuilderControlDelete(GUIBuilderControl* rt) {
	//RenderPipeline_destroy(rt->rpl);
	//free(rt->rpl);
}


void guiBuilderControlResize(GUIBuilderControl* bc, Vector2 newSz) {
	
	guiResize(bc->bg, newSz);
//	guiResize(bc->rt, newSz);
}

void geom_pass_render(MeshManager* mm, PassDrawable* pd, PassDrawParams* dp) {
	
	meshManager_draw(mm, dp->mWorldView, dp->mViewProj);
	
}


static void builderKeyUp(InputEvent* ev, GUIBuilderControl* bc) {
	if(ev->keysym == XK_Escape) {
		RemovePrePass("mesh_builder_control");
		guiDelete(bc);
		//gbcTest = NULL;
	}
	
	if(ev->keysym == XK_Left) {
		bc->yaw = fmod(bc->yaw + .01, F_2PI);
	} 
	if(ev->keysym == XK_Right) {
		bc->yaw = fmod(bc->yaw - .01, F_2PI);
	} 
	if(ev->keysym == XK_Up) {
		bc->pitch = fmod(bc->pitch + .01, F_2PI);
	} 
	if(ev->keysym == XK_Down) {
		bc->pitch = fmod(bc->pitch - .01, F_2PI);
	} 
	
	if(ev->character == 's') { // sphere
		
		
		
		MeshBuilder_LoadJSON(bc->mb, "assets/models/test.json");
		
	}
	if(ev->character == 'c') { // reroot with a compose
	//	bc->ed = GUIEditNew("i", (Vector2){.5,.5}, (Vector2){.2,.045});
		
		//guiRegisterObject(bc->ed, &bc->bg->header);
	//	InputFocusStack_PushTarget(bc->inputHandlers->stack, bc->ed, inputHandlers);
		
		
		
	}
}


static void pipeline_render(GUIBuilderControl* bc, PassFrameParams* pfp) {
	PassDrawParams rp;
	
	Matrix view = IDENT_MATRIX;
	Matrix proj = IDENT_MATRIX;
	
	mPerspective(60, 1, .01, 1000, &proj);

	mRot3f(0,1,0, bc->yaw, &view);
	mRot3f(1,0,0, bc->pitch, &view);
	mRot3f(0,0,1, bc->roll, &view);
	mTrans3f(0, 0, -10, &view);
	
	// y-up to z-up rotation
	mRot3f(1, 0, 0, F_PI_2, &view);
	mScale3f(1, 1, -1, &view);
	
	//rp.fboSize = (Vector2i){300,300};
	rp.mWorldView = &view; //msGetTop(&gs->view);
	rp.mViewProj = &proj; //msGetTop(&gs->proj);
	
	
	RenderPipeline_renderAll(bc->rpipe, &rp);
}


GUIBuilderControl* guiBuilderControlNew(Vector2 pos, Vector2 size, int zIndex) {
	/*
	GUIBuilderControl* bc;
	
	static struct gui_vtbl static_vt = {
		.Render = guiBuilderControlRender,
		.Delete = guiBuilderControlDelete,
		.Resize = guiBuilderControlResize
	};
	
	static InputEventHandler input_vt = {
		.keyUp = builderKeyUp,
	};
	
	
	bc = calloc(1, sizeof(*bc));
	CHECK_OOM(bc);
	
	gui_headerInit(&bc->header);
	bc->header.vt = &static_vt;
	bc->inputHandlers = &input_vt;
	
	bc->header.hitbox.min.x = pos.x;
	bc->header.hitbox.min.y = pos.y;
	bc->header.hitbox.max.x = pos.x + size.x;
	bc->header.hitbox.max.y = pos.y + size.y;
	
	bc->header.topleft = pos;
	bc->header.size = size;
	bc->header.z = 0;
	
	
	bc->bg = guiSimpleWindowNew(
		(Vector2){pos.x, pos.y}, 
		(Vector2){size.x, size.y}, 
		zIndex + .0001
	);
	//bc->bg = (Vector){0.9, 0.1, .9};
	guiRegisterObject(bc->bg, &bc->header);
	
	
	// set up the pipeline
	RenderPipeline* rpipe = calloc(1, sizeof(*rpipe));
	RenderPipeline_init(rpipe);
	bc->rpipe = rpipe;
	
	
	///-------------
	
	enum {
		DIFFUSE = 0,
		NORMAL,
		LIGHTING,
		DEPTH,
		OUTPUT,
		DEPTH2
	};

	FBOTexConfig texcfg[] = {
		{ GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE }, // diffuse
		{ GL_RGB, GL_RGB, GL_UNSIGNED_BYTE }, // normals
		{ GL_RGB16F, GL_RGB, GL_HALF_FLOAT }, // lighting
		{ GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT }, // depth
		{ GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }, // output
		{ GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT }, // depth2
		{ 0, 0, 0}
	};
		
	RenderPipelineFBOConfig gbuf[] = {
		{GL_COLOR_ATTACHMENT0, DIFFUSE },
		{GL_COLOR_ATTACHMENT1, NORMAL },
		{GL_COLOR_ATTACHMENT2, LIGHTING },
		{GL_DEPTH_ATTACHMENT, DEPTH },
		{0, -1}
	};
	
	RenderPipelineFBOConfig hbuf[] = {
		{GL_COLOR_ATTACHMENT0, OUTPUT },
		{GL_DEPTH_ATTACHMENT, DEPTH2 },
		{0, -1}
	};
	
	RenderPipeline_setFBOTexConfig(rpipe, texcfg);
	RenderPipeline_setFBOConfig(rpipe, gbuf, "gbuffer");
	RenderPipeline_setFBOConfig(rpipe, hbuf, "shading");
	
	///-------------
	
	
	RenderPass* pass;
	
	// geometry pass
	pass = calloc(1, sizeof(*pass));
	pass->clearColor = 1;
	pass->clearDepth = 1;
	
	pass->fboIndex = 0;
	
	RenderPass_init(pass);
	// loadCombinedProgram("dynamicMeshInstanced")
	
	bc->mm = meshManager_alloc();
	//meshManager_readConfigFile(bc->mm, "assets/config/models.json");
	//meshManager_updateGeometry(bc->mm);
	StaticMeshInstance smi[] = {
		{
			{1,1,4}, 2.5,
			{1, 0, 0}, 0.0,
			.9, 0,0,0
		}
	};
	//meshManager_addInstance(bc->mm, 0, &smi[0]);
	//meshManager_updateInstances(bc->mm);
	
	PassDrawable* br1 = Pass_allocDrawable("builder dmm");
	br1->draw = geom_pass_render;
	br1->data = bc->mm;
	br1->prog = loadCombinedProgram("dynamicMeshInstanced");
	
	VEC_PUSH(&pass->drawables, br1);
	
	VEC_PUSH(&rpipe->passes, pass);
	
	
	RenderPipeline_addShadingPass(rpipe, "builderShading"); 
	
	rpipe->clearColor = (Vector4){0.1, 0.1, 0.1, 1};
	
	//giTest->texIndex = -1;
	//Builder_initFBOs(rpipe);
	
	RegisterPrePass((pass_callback)pipeline_render, bc, "mesh_builder_control");
	
	
	bc->rt = guiRenderTargetNew(pos, size, rpipe);
	guiRegisterObject(bc->rt, &bc->bg->header);
	
	bc->yaw = 0.0;
	bc->pitch = 0.0;
	bc->roll = 0.0;
	
	
	bc->mb = MeshBuilder_alloc();
	bc->mb->mm = bc->mm;
	
	return bc;
	*/
}






