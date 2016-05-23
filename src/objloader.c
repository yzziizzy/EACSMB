
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "utilities.h"
#include "objloader.h"


static int sgrabfloat(char** s, float* f) {
	int num = 0;
	
	if(sscanf(*s, " %f%n", f, &num) > 0) {
		*s += num;
		return 1;
	}
	
	return 0;
}

// all real indices are 1-based. zero means not present.
static int readFaceVertex(char** s, int* v, int* vn, int* vt) {
	//  8/1/2
	// pos/normal/tex
	
	int n, cs;
	
	*v = 0;
	*vn = 0;
	*vt = 0;
	//printf(*s);
	n = sscanf(*s, " %d%n", v, &cs);
	*s += cs;
	if(**s != '/') {
		printf("broken face vertex - 1  '%.3s' \n", *s);
	}
	if(n < 1) {
		printf("missing face vertex position\n");
	}
	cs = 0;
	
	(*s)++;
	n = sscanf(*s, "%d%n", vn, &cs);
	*s += cs;
	if(**s != '/') {
		printf("broken face vertex - 2 %d  '%.3s' \n", n, *s);
	}
	if(n < 1) {
		printf("no face vertex normal\n");
	}
	cs = 0;
	
	(*s)++;
	if(isspace(**s)) return;
	n = sscanf(*s, "%d%n", vt, &cs);
	*s += cs;
	if(n < 1) {
		printf("no face vertex texture\n");
	}
	
}


// all real indices are 1-based. zero means not present.
static int readFace(char** s, int* v, int* vn, int* vt) {
	
	if(**s == 'f') {
		(*s)++;
		//printf(*s);
		readFaceVertex(s, v, vn, vt);
		readFaceVertex(s, v + 1, vn + 1, vt + 1);
		readFaceVertex(s, v + 2, vn + 2, vt + 2);
	}
	
}




static void skip_line(char** s) {
	if(!*s) return;
	while(**s && **s != '\n') (*s)++; 
	while(**s && **s == '\n') (*s)++; 
}

// kinda shitty limited version. object names longer than 1023 chars are too fucking long anyway.
static char* strscandup(FILE* f) {
	char buf[1024];
	
	buf[0] = 0;
	
	fscanf(f, "%s", buf);
	return strdup(buf);
}

static inline void append_vector(OBJDataBuffer* c, float* fs) {
	
	if(!c->buf) {
		c->sz = 64;
		c->buf = malloc(c->sz * c->dims * sizeof(float));
	}
	
	if(c->cnt >= c->sz) {
		c->sz *= 2;
		c->buf = realloc(c->buf, c->sz * c->dims * sizeof(float));
	}
	
	memcpy(&c->buf[c->cnt * c->dims], fs, c->dims * sizeof(float));
	
	c->cnt += c->dims;
}


void loadOBJFile(char* path, int four_d_verts, OBJContents* contents) {
	
	
	char* f;
	int line = 0;
	int cur_pos = 0;
	int cur_norm = 0;
	int cur_tex = 0;
	int cur_param = 0;
	
	int current_vertex_index = 0;
	
	f = readFile(path, NULL);
	if(!f) return;
	
	memset(contents, 0, sizeof(OBJContents));
	contents->v.dims = four_d_verts ? 4 : 3;
	
	//HACk BUG 
	contents->f = malloc(sizeof(OBJVertex) * 1337);
	
	while(*f) {
		char c;
		int n, j;
		float fbuf[4] = { 0.0, 0.0, 0.0, 1.0 };
		
		//printf("looping \n");
		c = *f;
		f++;
		printf("-%d-%c- ", line, c);
		
		// skip comments, usemtl, mtllib, and smoothing lines
		if(c == '#' || c == 'm' || c == 'u' || c == 's') {
			printf("useless line '%c'\n", c);
			skip_line(&f);
			line++;
			continue;
		}
		

		
		// poly group name, unsupported as of now 
		if(c == 'g') {
			printf("polygroup \n");
			skip_line(&f);
			line++;
			continue;
		}
		
		if(c == 'f') { // faces; triangles or quads
			//  f v[/t[/n]] v[/t[/n]] v[/t[/n]] [v[/t[/n]]]
			int V[3];
			int Vn[3];
			int Vt[3];
			
			int vi, ci;
			
			int i;
			
			readFace(&f, V, Vn, Vt);
			
			for(i = 0; i < 3; i++) {
				if(V[i] < 0) V[i] = current_vertex_index + 1 + V[i];
				else V[i] = V[i] == 0 ? 0 : -V[i];
			}
			
			for(i = 0; i < 3; i++) {
				if(Vn[i] < 0) Vn[i] = current_vertex_index + 1 + Vn[i];
				else Vn[i] = Vn[i] == 0 ? 0 : -Vn[i];
			}
			
			for(i = 0; i < 3; i++) {
				if(Vt[i] < 0) Vt[i] = current_vertex_index + 1 + Vt[i];
				else Vt[i] = Vt[i] == 0 ? 0 : -Vt[i];
			}
			
			// all indices are 1-based; convert to 0-based
			// resolve negative (relative) indices to absolute indices 
			contents->f[contents->fv_cnt].v  = V[0];
			contents->f[contents->fv_cnt].vn = Vn[0];
			contents->f[contents->fv_cnt].vt = Vt[0];
			contents->fv_cnt++;
			contents->f[contents->fv_cnt].v  = V[0];
			contents->f[contents->fv_cnt].vn = Vn[0];
			contents->f[contents->fv_cnt].vt = Vt[0];
			contents->fv_cnt++;
			contents->f[contents->fv_cnt].v  = V[0];
			contents->f[contents->fv_cnt].vn = Vn[0];
			contents->f[contents->fv_cnt].vt = Vt[0];
			contents->fv_cnt++;
			printf("found face #%d [] \n", contents->fv_cnt/3);
			
			
			
			printf(" face \n");
			skip_line(&f);
			line++;
			continue;
			
			/*
			if(vi > 2) { // triangulate the quad
				contents->f[f_cnt++] = ind[0][0] - 1;
				contents->f[f_cnt++] = ind[0][2] - 1;
				contents->f[f_cnt++] = ind[0][3] - 1;
			}
			*/
		}
		 
		// vertex data
		else if(c == 'v') {
			c = *f;
			f++;
			printf(" next c '%c' \n ", c);
			
			//fbuf ;
			n = 0;
			
			skip_line(&f);
			line++;
			continue;
			
			
			sgrabfloat(&f, &fbuf);
			/*
			char chars = 0; l
			do {
				j = sscanf(f, " %f%n", &fbuf[j++], &chars);
				f += chars;
			} while(j);
			*/
			if(c == ' ') { // position
				if(n < 3) {
					fprintf(stderr, "OBJ: only %d coordinates for vertex on line %d\n", n, line);
					return;
				}
				
				
				
				append_vector(&contents->v, fbuf);
				
				printf("found vertex #%d [%f, %f, %f] \n", contents->v.cnt/contents->v.dims, fbuf[0], fbuf[1], fbuf[2]);
			}
			else if(c == 'n') { // normal
				
				// normalize?
				
				append_vector(&contents->vn, fbuf);
				printf("found normal #%d [] \n", contents->vn.cnt);
			}
			else if(c == 't') { // texture coordinates
				append_vector(&contents->vt, fbuf);
				printf("found texcoords #%d [] \n", contents->vt.cnt);
			}
// 			else if(c == 'p') { // "freeform geometry coordinates"
// 				append_vector(&contents->vp, fbuf);
// 			}
			else {
				printf("found unknown v '%c'\n", c); 
			}
			
			
			
			skip_line(&f);
			line++;
			continue;
		}
		
		// object name
		else if(c == 'o') {
			printf("object name\n");
			contents->o = strlndup(f);
			skip_line(&f);
			line++;
			continue;
		}
		else {
			printf("nothin\n");
			
		}
		
		
		
	}
}
	




















