
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "../gui.h"

#include "hash.h"
#include "shader.h"
#include "texture.h"
#include "c_json/json.h"






HashTable(int) image_names;
TexArray* image_textures;

GLuint vaoImage, vboImage;

ShaderProgram* imageProg;
ShaderProgram* rtProg;


void gui_Image_Init(char* file) {
	
	char** paths; 
	int len;
	
	void* iter;
	char* texName;
	json_value_t* j_texPath;
	json_file_t* jsf;
	
	HT_init(&image_names, 4);
	
	jsf = json_load_path(file);
	
	len = json_obj_length(jsf->root);
	paths = malloc(sizeof(*paths) * len + 1);
	paths[len] = 0;
	
	iter = NULL; 
	int i = 0;
	while(json_obj_next(jsf->root, &iter, &texName, &j_texPath)) {
		
		char* name = strdup(texName);
		json_as_string(j_texPath, paths + i);
		
		HT_set(&image_names, name, i);
		
		i++;
	}
	
	// 128x128 is hardcoded for now
	image_textures = loadTexArray(paths);
	
	for(i = 0; i < len; i++) free(paths[i]);
	free(paths);
	
	
	// BUG: double free error somewhere in here
	//json_free(jsf->root);
	//free(jsf);
	
	
	
	// gl stuff
	imageProg = loadCombinedProgram("guiImage");
	rtProg = loadCombinedProgram("guiRenderTarget");
	
	
	// image VAO
	VAOConfig opts[] = {
		// per vertex
		{2, GL_FLOAT}, // position/tex coords
		
		{0, 0}
	};
	
	vaoImage = makeVAO(opts);
	
	
	glBindVertexArray(vaoImage);
	
	glGenBuffers(1, &vboImage);
	glBindBuffer(GL_ARRAY_BUFFER, vboImage);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*4, 0);
	
	Vector2 data[] = {
		{0,0},
		{0,1},
		{1,0},
		{1,1}
	};
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("");
	
	
	
}






void guiImageRender(GUIImage* im, GameState* gs) {
	
	Matrix proj = IDENT_MATRIX;
	
	static GLuint proj_ul;
	static GLuint tlx_tly_w_h_ul;
	static GLuint z_alpha__ul;
	static GLuint texIndex_ul;
	static GLuint sTexture_ul;
	static GLuint sCustomTexture_ul;
	//static GLuint color_ul;
	
	if(!proj_ul) proj_ul = glGetUniformLocation(imageProg->id, "mProj");
	if(!tlx_tly_w_h_ul) tlx_tly_w_h_ul = glGetUniformLocation(imageProg->id, "tlx_tly_w_h");
	if(!z_alpha__ul) z_alpha__ul = glGetUniformLocation(imageProg->id, "z_alpha_");
	if(!texIndex_ul) texIndex_ul = glGetUniformLocation(imageProg->id, "texIndex");
	if(!sTexture_ul) sTexture_ul = glGetUniformLocation(imageProg->id, "sTexture");
	if(!sCustomTexture_ul) sCustomTexture_ul = glGetUniformLocation(imageProg->id, "sCustomTexture");
	//if(!color_ul) color_ul = glGetUniformLocation(imageProg->id, "color");
	
	
	
	mOrtho(0, 1, 0, 1, 0, 1, &proj);
	
	
	glUseProgram(imageProg->id);
	glexit("");
	
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj.m);
	glUniform4f(tlx_tly_w_h_ul, 
		im->header.topleft.x, 
		im->header.topleft.y, 
		im->header.size.x, 
		im->header.size.y 
	);
	glUniform4f(z_alpha__ul, -.1, 1, 0, 0); // BUG z is a big messed up; -.1 works but .1 doesn't.
	
	glProgramUniform1i(imageProg->id, texIndex_ul, im->texIndex);
	if(im->texIndex == -1) {
		glProgramUniform1i(imageProg->id, sCustomTexture_ul, 30);
	}
	glProgramUniform1i(imageProg->id, sTexture_ul, 31);
	
	//glActiveTexture(GL_TEXTURE0 + 31);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, image_textures->tex_id);
	
	glActiveTexture(GL_TEXTURE0 + 30);
	glBindTexture(GL_TEXTURE_2D, im->customTexID);
	
	//glUniform3f(color_ul, gw->color.x, gw->color.y, gw->color.z); // BUG z is a big messed up; -.1 works but .1 doesn't.

	glBindVertexArray(vaoImage);
	glBindBuffer(GL_ARRAY_BUFFER, vboImage);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("");
}

void guiImageDelete(GUIImage* im) {
	
	
	
	
}



GUIImage* guiImageNew(Vector2 pos, Vector2 size, float zIndex, int texIndex) {
	
	GUIImage* im;
	
	float tbh = .03; // titleBarHeight
	
	static struct gui_vtbl static_vt = {
		.Render = guiImageRender,
		.Delete = guiImageDelete
	};
	
	
	im = calloc(1, sizeof(*im));
	CHECK_OOM(im);
	
	guiHeaderInit(&im->header);
	im->header.vt = &static_vt;
	
	im->header.hitbox.min.x = pos.x;
	im->header.hitbox.min.y = pos.y;
	im->header.hitbox.max.x = pos.x + size.x;
	im->header.hitbox.max.y = pos.y + size.y;
	
	im->header.topleft = pos;
	im->header.size = size;
	im->header.z = zIndex;
	
	im->texIndex = texIndex;
	im->customTexID = 0;
	
	return im;
}































void guiRenderTargetRender(GUIRenderTarget* im, GameState* gs) {
	
	Matrix proj = IDENT_MATRIX;
	
	static GLuint proj_ul;
	static GLuint tlx_tly_w_h_ul;
	static GLuint z_alpha__ul;
	static GLuint sTexture_ul;
	//static GLuint color_ul;
	
	if(!proj_ul) proj_ul = glGetUniformLocation(rtProg->id, "mProj");
	if(!tlx_tly_w_h_ul) tlx_tly_w_h_ul = glGetUniformLocation(rtProg->id, "tlx_tly_w_h");
	if(!z_alpha__ul) z_alpha__ul = glGetUniformLocation(rtProg->id, "z_alpha_");
	if(!sTexture_ul) sTexture_ul = glGetUniformLocation(rtProg->id, "sTexture");
	//if(!color_ul) color_ul = glGetUniformLocation(rtProg->id, "color");
	
	if(im->texID == 0) return;
	
	mOrtho(0, 1, 0, 1, 0, 1, &proj);
	
	glUseProgram(rtProg->id);
	glexit("");
	
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj.m);
	glUniform4f(tlx_tly_w_h_ul, 
		im->header.topleft.x, 
		im->header.topleft.y, 
		im->header.size.x, 
		im->header.size.y 
	);
	glUniform4f(z_alpha__ul, -.1, 1, 0, 0); // BUG z is a big messed up; -.1 works but .1 doesn't.
	
	glProgramUniform1i(rtProg->id, sTexture_ul, 30);
	
	glActiveTexture(GL_TEXTURE0 + 30);
	glBindTexture(GL_TEXTURE_2D, im->texID);
	
	glBindVertexArray(vaoImage);
	glBindBuffer(GL_ARRAY_BUFFER, vboImage);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("");
}

void guiRenderTargetDelete(GUIRenderTarget* rt) {
	RenderPipeline_destroy(rt->rpl);
	free(rt->rpl);
}

void guiRenderTargetResize(GUIRenderTarget* rt, Vector2 newSz) {
	
// 	RenderPipeline_rebuildFBOs(rt->rpl, (Vector2i){newSz.x, newSz.y});
	
	printf("hack. need to get real pizel size here\n");
	
	RenderPipeline_rebuildFBOs(rt->rpl, (Vector2i){newSz.x * 600, newSz.y * 600});
}








GUIRenderTarget* guiRenderTargetNew(Vector2 pos, Vector2 size, RenderPipeline* rpl) {
	
	GUIRenderTarget* im;
	
	float tbh = .03; // titleBarHeight
	
	static struct gui_vtbl static_vt = {
		.Render = guiRenderTargetRender,
		.Delete = guiRenderTargetDelete,
		.Resize = guiRenderTargetResize
	};
	
	
	im = calloc(1, sizeof(*im));
	CHECK_OOM(im);
	
	guiHeaderInit(&im->header);
	im->header.vt = &static_vt;
	
	im->header.hitbox.min.x = pos.x;
	im->header.hitbox.min.y = pos.y;
	im->header.hitbox.max.x = pos.x + size.x;
	im->header.hitbox.max.y = pos.y + size.y;
	
	im->header.topleft = pos;
	im->header.size = size;
	im->header.z = 0;
	
	im->texID = 0;
	im->rpl = rpl;
	
	return im;
}




