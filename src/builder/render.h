#ifndef __EACSMB_builder_render_h__
#define __EACSMB_builder_render_h__


#include "../ds.h"


#include "builder.h" 
#include "../staticMesh.h" 
#include "../shader.h" 
#include "../pass.h" 
#include "../game.h" 




typedef struct GUIBuilderControl {
	GUIHeader header;
	
	GUISimpleWindow* bg;
	GUIRenderTarget* rt;
	RenderPipeline* rpipe;
	
	GUIEdit* ed;
	
	MeshBuilder* mb;
	MeshManager* mm;
	
	InputEventHandler* inputHandlers;
	
	float yaw;
	float pitch;
	float roll;
	
	
} GUIBuilderControl;




GUIBuilderControl* guiBuilderControlNew(Vector2 pos, Vector2 size, int zIndex);








#endif // __EACSMB_builder_render_h__
