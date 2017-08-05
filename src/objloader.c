
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"

#include "utilities.h"
#include "objloader.h"


// returns status code
int strtol_2(char* s, int* charsRead, int* value) {
	char* end;
	int val;
	
	if(charsRead) *charsRead = 0;
	
	errno = 0;
	val = strtol(s, &end, 10);
	if(errno != 0) return errno; // read error
	if(s == end) return -1; // no characters read
	
	// success
	if(charsRead) *charsRead = end - s;
	if(value) *value = val;
	
	return 0;
}



int parseFaceVertex(char** s, int* info) {
	char* end;
	int val;
	int err, chars_read;
	
	while(**s && (**s == ' ' || **s == '\t' || **s == '\r')) (*s)++;
	if(**s == '\n') return 1;
	
	info[3] = 0;
	
	// first  
	err = strtol_2(*s, &chars_read, &val);
	if(err) return 1;
	*s += chars_read ;
	info[0] = val;
	info[3]++;
	
	if(**s != '/') return 0;
	(*s)++;
	
	err = strtol_2(*s, &chars_read, &val);
	if(err) return 0;
	*s += chars_read ;
	info[1] = val;
	info[3]++;
	
	if(**s != '/') return 0;
	(*s)++;
	
	err = strtol_2(*s, &chars_read, &val);
	if(err) return 0;
	*s += chars_read ;
	info[2] = val;
	info[3]++;
	
	return 0;
}



void loadOBJFile(char* path, int four_d_verts, OBJContents* contents) {
	
	
	char* f, *raw;
	int line = 0;
	int cur_pos = 0;
	int cur_norm = 0;
	int cur_tex = 0;
	int cur_param = 0;
	
	int current_vertex_index = 0;
	
	raw = readFile(path, NULL);
	if(!raw) return;
	
	memset(contents, 0, sizeof(OBJContents));
	
	int numVertices = 0;
	int numNormals = 0;
	int numTexCoords = 0;
	int numFaces = 0;
	

	f = raw;
	while(*f) {
		char c;
		
		c = *f;
		f++;
		
		if(c == 'f') { // faces
			//numFaces++;
// 			count number of flip-flops between space and not, subtract two
			{
				int in_letters = 0;
				while(*f && f++) {
					if(in_letters && isspace(*f)) {
						in_letters = 0;
						//printf("face");
						numFaces++;
					}
					if(!in_letters && !isspace(*f)) {
						in_letters = 1;
					}
					
					if(*f == '\n') break;
					//printf("x");
				}
				//printf("-\n");
			//while(*f++ != '\n');
			}
		}
		else if(c == 'v') { 
			c = *f;
			if(c == ' ') numVertices++;
			else if(c == 'n') numNormals++;
			else if(c == 't') numTexCoords++;
		}
		
		while(*f && *f != '\n') f++;
		f++;
	}
	
	printf("Vertices: %d\n", numVertices);
	printf("Normals: %d\n", numNormals);
	printf("TexCoords: %d\n", numTexCoords);
	printf("Faces: %d\n", numFaces);
	
	Vector* vertices;
	Vector* normals;
	Vector2* texCoords;
	OBJVertex* faces;
	
	int vc = 0;
	int nc = 0;
	int tc = 0;
	int fc = 0;
	
	vertices = malloc(numVertices * sizeof(Vector));
	normals = malloc(numNormals * sizeof(Vector));
	texCoords = malloc(numTexCoords * sizeof(Vector2));
	// double the size in case of quads
	faces = malloc(numFaces * 6 * sizeof(OBJVertex));
	
	
	f = raw;
	while(*f) {
		char c;
		int chars_read, n, j;
		int fd[3][4];
		int vertex_count;
		int ret;
		int nn = 0;
		
		//printf("looping \n");
		c = *f;
		f++;
		
		if(c == 'f') { // faces. currently only triangles and quite hacky
			//  f v[/t[/n]] v[/t[/n]] v[/t[/n]] [v[/t[/n]]]
			
			//printf("parseface \n");
			
			ret = parseFaceVertex(&f, &fd[0]); // the center vertex
			//if(!ret) 
			//printf("ret 1: %d, ", ret);
			ret = parseFaceVertex(&f, &fd[1]); // the next vertex
			//printf("ret 2: %d, ", ret);
			
			do {
				
				ret = parseFaceVertex(&f, &fd[2]);
				//printf("ret 3: %d \n", ret);
				//printf("parseface( %d )\n", ret);
				if(ret) break;
				
				//parseFaceVertex(&f, &fd[3]);
				
				/*
				printf("%d/%d/%d[%d] %d/%d/%d[%d] %d/%d/%d[%d] \n",
					fd[0][0], fd[0][1], fd[0][2], fd[0][3],
					fd[1][0], fd[1][1], fd[1][2], fd[1][3],
					fd[2][0], fd[2][1], fd[2][2], fd[2][3]);
				*/
					   
				//printf("fc %d %d\n", fc, fd[0][0]);
				vCopy(  &vertices[fd[0][0]-1], &faces[fc].v);
				vCopy2(&texCoords[fd[0][1]-1], &faces[fc].t);
				vCopy(   &normals[fd[0][2]-1], &faces[fc].n);
				fc++;
				
				vCopy(  &vertices[fd[1][0]-1], &faces[fc].v);
				vCopy2(&texCoords[fd[1][1]-1], &faces[fc].t);
				vCopy(   &normals[fd[1][2]-1], &faces[fc].n);
				fc++;
				
				vCopy(  &vertices[fd[2][0]-1], &faces[fc].v);
				vCopy2(&texCoords[fd[2][1]-1], &faces[fc].t);
				vCopy(   &normals[fd[2][2]-1], &faces[fc].n);
				fc++;
				//printf("fc %d\n", fc);
				// shift the last vertex
				
				memcpy(&fd[1], &fd[2], sizeof(fd[2]));
			} while(1);
			

		}
		else if(c == 'v') {
			
			c = *f;
			f++;
			if(c == ' ') {
				chars_read = 0;
				n = sscanf(f, 
					" %f %f %f%n",
					&vertices[vc].x,
					&vertices[vc].y,
					&vertices[vc].z,
					&chars_read);
				
				if(n < 3) {
					fprintf(stderr, "error reading vertex\n");
				}
				
				//printf("got vertex: %f %f %f ",vertices[vc].x,vertices[vc].y,vertices[vc].z);
				
				f += chars_read-1;
				//printf("'%.5s' \n", (f-2));
				vc++;
			}
			else if(c == 'n') {
				chars_read = 0;
				n = sscanf(f, 
					" %f %f %f%n",
					&normals[nc].x,
					&normals[nc].y,
					&normals[nc].z,
					&chars_read);
				
				if(n < 3) {
					fprintf(stderr, "error reading normal\n");
				}
				
				f += chars_read-1;
				nc++;
			}
			else if(c == 't') {
				chars_read = 0;
				n = sscanf(f, 
					" %f %f%n",
					&texCoords[tc].x,
					&texCoords[tc].y,
					&chars_read);
				
				if(n < 2) {
					fprintf(stderr, "error reading tex coord\n");
				}
				
				//printf("got tex coords: %f %f\n", texCoords[tc].x, texCoords[tc].y);
				
				f += chars_read-1;
				tc++;
			}
			
			f++;
			//printf("x");
		} 
		
		// skip to the end
		while(*f && *f != '\n') f++;
		f++;
	}
	
	
	contents->faces = faces;
	contents->faceCnt = fc  > 0 ? fc / 3 : 0;
}




/*

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

static void append_vector(OBJDataBuffer* c, float* fs) {
	
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


void old_loadOBJFile(char* path, int four_d_verts, OBJContents* contents) {
	
	
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
	
	
	// count everything
	
	// alloc everything
	
	// read each thing into the mesh
	
	
	
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
			* /
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
			* /
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
*/
/*
Mesh* OBJtoMesh(OBJContents* obj) {
	
	Mesh* m;
	int i;
	
	m = calloc(1, sizeof(Mesh));
	
	// vertex arrays are the number of unique vertices. obj's do not enforce this.
	// all vertices are created new for simplicity sake
	m->szVertices = obj->fv_cnt;
	m->vertexCnt = obj->fv_cnt;
	m->indices = malloc(sizeof(obj->fv_cnt) * sizeof(unsigned short));
	m->vertices = malloc(sizeof(MeshVertex) * obj->fv_cnt);
	
	for(i = 0; i < obj->fv_cnt; i++) {
		vCopy(&obj->v.buf[obj->f[i].v], &m->vertices[i].v); 
		vCopy(&obj->vn.buf[obj->f[i].vn], &m->vertices[i].n); 
		m->vertices[i].t.u = obj->vn.buf[obj->f[i].vt]; 
		m->vertices[i].t.v = obj->vn.buf[obj->f[i].vt+1]; 
		m->indices[i] = i;
	}
	
	
	
	return m;
}

*/
















