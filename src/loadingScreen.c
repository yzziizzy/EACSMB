
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#include "window.h"
#include "loadingScreen.h"
#include "utilities.h"






void LoadingScreen_init(LoadingScreen* ls) {
	
	ls->time = getCurrentTime();
	ls->done = 0;
	
	
	
	
	// fulls-screen quad
	float vertices[] = {
		-1.0, -1.0,
		-1.0, 1.0,
		1.0, -1.0,
		1.0, 1.0
	};
	glerr("left over error on game init");
	
	glGenVertexArrays(1, &ls->quadVAO);
	glBindVertexArray(ls->quadVAO);
	
	glGenBuffers(1, &ls->quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ls->quadVBO);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, (void*)0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glerr("left over error on game init");
	
	// shaders and uniforms
	ls->prog = loadCombinedProgram("loadingScreen");
	
	ls->ul_timer = glGetUniformLocation(ls->prog->id, "timer");
	ls->ul_resolution = glGetUniformLocation(ls->prog->id, "resolution");
	
}

void LoadingScreen_draw(LoadingScreen* ls, XStuff* xs) {
	
	ls->time = getCurrentTime();
	
	glUseProgram(ls->prog->id);
	
	glProgramUniform1f(ls->prog->id, ls->ul_timer, ls->time);
	glProgramUniform2f(ls->prog->id, ls->ul_resolution, ls->resolution.x, ls->resolution.y);
	
	
	// draw
	//glViewport(0, 0, ls->resolution.x, ls->resolution.y);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindVertexArray(ls->quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ls->quadVBO);
	glexit("quad vbo");
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("quad draw");
	
	
	glXSwapBuffers(xs->display, xs->clientWin);
}

