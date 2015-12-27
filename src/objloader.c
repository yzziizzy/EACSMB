
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>


#include "objloader.h"




static void skip_line(FILE* f) {
	while(!feof(f) && fgetc(f) != '\n');
	fgetc(f);
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
	
	
	FILE* f;
	int line = 0;
	int cur_pos = 0;
	int cur_norm = 0;
	int cur_tex = 0;
	int cur_param = 0;
	
	f = fopen(path, "r");
	if(!f) return;
	
	memset(contents, 0, sizeof(OBJContents));
	contents->v.dims = four_d_verts ? 4 : 3;
	
	
	while(!feof(f)) {
		char c;
		int n;
		float fbuf[4];
		
		c = fgetc(f);
		
		// skip comments, usemtl, mtllib, and smoothing lines
		if(c == '#' || c == 'm' || c == 'u' || c == 's') {
			skip_line(f);
			line++;
			continue;
		}
		

		
		// poly group name, unsupported as of now 
		if(c == 'g') {
			skip_line(f);
			line++;
			continue;
		}
		
		if(c == 'f') { // faces; triangles or quads
			//  f v[/t[/n]] v[/t[/n]] v[/t[/n]] [v[/t[/n]]]
			
			int vi, ci;
			int ind[3][4];
			
			// read face data into the index buffer
			for(vi = 0; vi < 4; vi++) {
				n = fscanf(f, "%d", &ind[0][vi]);
				if(!n) break;
				ci = 1;
				while(fgetc(f) == '/' && fscanf(f, "%d", &ind[ci++][vi]));
			}
			
			// all indices are 1-based; convert to 0-based
			// resolve negative (relative) indices to absolute indices 
			contents->f[f_cnt].v  = ind[0][0] - 1;
			contents->f[f_cnt].vn = ind[0][1] - 1;
			contents->f[f_cnt].vt = ind[0][2] - 1;
			
			f_cnt++;
			
			if(vi > 2) { // triangulate the quad
				contents->f[f_cnt++] = ind[0][0] - 1;
				contents->f[f_cnt++] = ind[0][2] - 1;
				contents->f[f_cnt++] = ind[0][3] - 1;
			}
			
		}
		 
		// vertex data
		else if(c == 'v') {
			c = fgetc(f);
			
			fbuf = { 0.0, 0.0, 0.0, 1.0 };
			
			n = fscanf(f, " %f %f %f %f", &fbuf[0], &fbuf[1], &fbuf[2], &fbuf[3]);
			
			if(c == ' ') { // position
				if(n < 3) {
					fprintf(stderr, "OBJ: only %d coordinates for vertex on line %d\n", n, line);
					return;
				}
				
				append_vector(&contents->v, fbuf);
			}
			else if(c == 'n') { // normal
				
				// normalize?
				
				append_vector(&contents->vn, fbuf);
			}
			else if(c == 't') { // texture coordinates
				append_vector(&contents->vt, fbuf);
			}
// 			else if(c == 'p') { // "freeform geometry coordinates"
// 				append_vector(&contents->vp, fbuf);
// 			}
			
			skip_line(f);
			line++;
			continue;
		}
		
		// object name
		else if(c == 'o') {
			contents->o = strscandup(f);
			skip_line(f);
			line++;
			continue;
		}
		
		
	}
}
	




















