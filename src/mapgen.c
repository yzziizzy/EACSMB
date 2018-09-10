#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"

#include "utilities.h"
#include "map.h"
#include "perlin.h"
#include "opensimplex.h"








//BezierSpline2* genRiverSpline(float w, float h) {
	//BezierSpline2* sp;
	
	//pcalloc(sp);
	
	
	//BezierSplineSegment2* seg;
	
	
	
	
//}




void erode(MapLayer* input, float strength) {
	
	
}





void Mapgen_v1(MapLayer* ml) {
	
	int x, y;
	//FILE* f;
	
	
	// overall tilt
	
	// install a river
	//BezierSpline2
	
	
	// install some mountains
	
	
	
	
	

	OpenSimplexNoise osn;
	OpenSimplex_init(&osn, 6456, ml->w, ml->h);
	
	float div = .5;
	float top = 1.0;
	
	OpenSimplexOctave octs[] = {
		{1, top * pow(div, 0)},
		{2, top * pow(div, 1)},
		{4, top * pow(div, 2)},
		{8, top * pow(div, 3)},
		{16, top * pow(div, 4)},
		{32, top * pow(div, 5)},
		{64, top * pow(div, 6)},
		{128, top * pow(div, 7)},
		{256, top * pow(div, 8)},
		{-1, -1}
	};
	OpenSimplexParams params = {
		ml->w, ml->h,
		1024*512, 1024*512,
		octs
	};
	
	float* data = OpenSimplex_GenNoise2D(&osn, &params);
	
	ml->data.f = malloc(sizeof(*ml->data.f) * ml->w * ml->h);
	
	for(y = 0; y < ml->h ; y++) {
		for(x = 0; x < ml->w ; x++) {
			float f = data[x + (y * ml->w)];
			float ff = fabs(1-f) + 1;
			ff *= 1.5;
			//float fff = ff * ff;
			//ml->data.f[x + (y * ml->w)] = pow(ff, 4); // make it super steep
			ml->data.f[x + (y * ml->w)] = pow(ff, 2.2);
		}
	}
	
	free(data);
	
	
	
}










// only supports float layers
void MapGen_water(MapInfo* mi, ShaderProgram* prog) {
	
	static int waterIndex = 0;
	
	waterIndex = (waterIndex + 1) % 2;
	
	glUseProgram(prog->id);
	
	glUniform1i(glGetUniformLocation(prog->id, "waterIndex"), waterIndex);
	
	
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->terrainTex);
	GLuint hmul = glGetUniformLocation(prog->id, "sHeightMap");
	glProgramUniform1i(prog->id, hmul, 4);
	
	
	
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindImageTexture(5, mi->terrainTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	GLuint hmul2 = glGetUniformLocation(prog->id, "sOut");
	glProgramUniform1i(prog->id, hmul2, 5);

	glActiveTexture(GL_TEXTURE0 + 6);
	glBindImageTexture(6, mi->terrainTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG8);
	GLuint hmul3 = glGetUniformLocation(prog->id, "sVel");
	glProgramUniform1i(prog->id, hmul3, 6);
	
	glDispatchCompute(mi->block->w, mi->block->h, 1);
	
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	
	//glMapBuffer();
	
	
}





void MapGen_erode(MapInfo* mi, ShaderProgram* prog) {

	glUseProgram(prog->id);

	
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->terrainTex);
	GLuint hmul = glGetUniformLocation(prog->id, "sHeightMap");
	glProgramUniform1i(prog->id, hmul, 4);
	
	glActiveTexture(GL_TEXTURE0 + 7);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->terrainTex);
	GLuint hmul4 = glGetUniformLocation(prog->id, "sVel");
	glProgramUniform1i(prog->id, hmul4, 7);
	
	
	
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindImageTexture(5, mi->terrainTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	GLuint hmul2 = glGetUniformLocation(prog->id, "iOut");
	glProgramUniform1i(prog->id, hmul2, 5);

	glActiveTexture(GL_TEXTURE0 + 6);
	glBindImageTexture(6, mi->terrainTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG8);
	GLuint hmul3 = glGetUniformLocation(prog->id, "iVel");
	glProgramUniform1i(prog->id, hmul3, 6);
	
	glDispatchCompute(mi->block->w, mi->block->h, 1);
}



















void MapGen_initWaterVelTex(MapInfo* mi) {
	glGenTextures(1, &mi->block->waterVelTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->block->waterVelTex);
	glexit("failed to create map textures b");
	
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glexit("failed to create map textures a");
	
	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
		1,  // mips, flat
		GL_RG8,
		mi->block->w, mi->block->h,
		1); // layers: water velocity
	
	glexit("failed to create map textures");
	printf("created terrain tex\n");

}






