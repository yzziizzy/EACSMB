#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "common_gl.h"

#include "ds.h"

#include "utilities.h"
#include "shader.h"


#include "debugWireframes.h"





typedef struct debugWireframeLine {
	Vector p1, p2;
	uint32_t color1, color2;
	float width1, width2;
} debugWireframeLine;

typedef struct debugWireframePoint {
	Vector p;
	uint32_t color;
	float size;
} debugWireframePoint;


static VEC(debugWireframeLine) debugLines;
static VEC(debugWireframePoint) debugPoints;



static GLuint debugVaoLines;
static GLuint debugVaoPoints;
static ShaderProgram* debugProgLines;
static ShaderProgram* debugProgPoints;

static GLuint wv_lines_ul, vp_lines_ul;
static GLuint wv_points_ul, vp_points_ul;


// VAO definitions
static VAOConfig vao_opts_lines[] = {
	// per vertex
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // color
	
	{0, 0, 0}
};

static VAOConfig vao_opts_points[] = {
	// per vertex
	{0, 4, GL_FLOAT, 0, GL_FALSE}, // position, size
	{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // color
	{0, 0, 0}
};


struct debugLineShader {
	Vector p;
	uint32_t color;
};

struct debugPointShader {
	Vector p;
	uint32_t color;
};




void initDebugWireframe() {
	VEC_INIT(&debugLines);
	VEC_INIT(&debugPoints);
	
	debugVaoLines = makeVAO(vao_opts_lines);
	debugVaoPoints = makeVAO(vao_opts_points);
	
	debugProgLines = loadCombinedProgram("debugWireframeLines");
	debugProgPoints = loadCombinedProgram("debugWireframePoints");
	
	wv_lines_ul = glGetUniformLocation(debugProgLines->id, "mWorldView");
	vp_lines_ul = glGetUniformLocation(debugProgLines->id, "mViewProj");
	
	wv_points_ul= glGetUniformLocation(debugProgPoints->id, "mWorldView");
	vp_points_ul = glGetUniformLocation(debugProgPoints->id, "mViewProj");
	
	glProgramUniform1i(debugProgLines->id, glGetUniformLocation(debugProgLines->id, "sDepth"), 2);
	glProgramUniform1i(debugProgPoints->id, glGetUniformLocation(debugProgPoints->id, "sDepth"), 2);
}


void resetDebugWireframes() {
	VEC_TRUNC(&debugLines);
	VEC_TRUNC(&debugPoints);
}







void renderDebugWireframeLines(PassFrameParams* pfp) {
	GLuint vbo;
	
	if(0 == VEC_LEN(&debugLines)) return;
	
	glBindVertexArray(debugVaoLines);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	
	int stride = calcVAOStride(0, vao_opts_lines);
	updateVAO(0, vao_opts_lines); 
	
	glBufferStorage(GL_ARRAY_BUFFER, stride * VEC_LEN(&debugLines) * 2, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	struct debugLineShader* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	
	VEC_EACH(&debugLines, i, line) {
		buf[i*2  ].p = line.p1;
		buf[i*2  ].color = line.color1;
		buf[i*2+1].p = line.p2;
		buf[i*2+1].color = line.color2;
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glexit("");
	
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glUseProgram(debugProgLines->id);
	glUniformMatrix4fv(wv_lines_ul, 1, GL_FALSE, &pfp->dp->mWorldView->m);
	glUniformMatrix4fv(vp_lines_ul, 1, GL_FALSE, &pfp->dp->mViewProj->m);

	glexit("");
// 	glBindVertexArray(vao);
	
// 	glDrawArrays(GL_LINES, 0, 12 * 2);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, VEC_LEN(&debugLines) * 2);
	glexit("debug box draw");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);
	glexit("");
}



// TODO: parse color


void debugWFAddLine(Vector* p1, Vector* p2, char* color1, char* color2, float width1, float width2) {
	VEC_PUSH(&debugLines, ((debugWireframeLine){
		.p1 = *p1,
		.p2 = *p2,
		.color1 = 0xffffffff,
		.color2 = 0xffffffff,
		.width1 = width1,
		.width2 = width2,
	}));
}



void debugWFProjMatrix(Matrix* m) {
	
	Frustum f;
	
	static char* green = "green"; 
	static char* red = "red"; 
	
	frustumFromMatrix(m, &f);
	
	// near plane
	debugWFAddLine(&f.points[0], &f.points[1], green, green, 1, 1);
	debugWFAddLine(&f.points[0], &f.points[2], green, green, 1, 1);
	debugWFAddLine(&f.points[3], &f.points[1], green, green, 1, 1);
	debugWFAddLine(&f.points[3], &f.points[2], green, green, 1, 1);
	
	// far plane
	debugWFAddLine(&f.points[4], &f.points[5], red, red, 1, 1);
	debugWFAddLine(&f.points[4], &f.points[6], red, red, 1, 1);
	debugWFAddLine(&f.points[7], &f.points[5], red, red, 1, 1);
	debugWFAddLine(&f.points[7], &f.points[6], red, red, 1, 1);
	
	// connecting lines
	debugWFAddLine(&f.points[0], &f.points[4], green, red, 1, 1);
	debugWFAddLine(&f.points[1], &f.points[5], green, red, 1, 1);
	debugWFAddLine(&f.points[2], &f.points[6], green, red, 1, 1);
	debugWFAddLine(&f.points[3], &f.points[7], green, red, 1, 1);
}

