


#include "vegetation.h"





static void bind_uniform(struct uniform_binding* ub, ShaderProgram* s) {
	// to make initialization easier
	if(ub->count == 0) {
		ub->count = 1;
	}
	
	switch(ub->type) {
		case UT_INT:
			switch(ub->size) {
				case 1: glProgramUniform1iv(s->id, ub->location, ub->count, (GLint*)ub->data); break;
				case 2: glProgramUniform2iv(s->id, ub->location, ub->count, (GLint*)ub->data); break;
				case 3: glProgramUniform3iv(s->id, ub->location, ub->count, (GLint*)ub->data); break;
				case 4: glProgramUniform4iv(s->id, ub->location, ub->count, (GLint*)ub->data); break;
					
				default:
					fprintf(stderr, "!!! invalid size in bind_uniform: %d\n", ub->size);
					break;
			}
			break;
			
		case UT_UINT:
			switch(ub->size) {
				case 1: glProgramUniform1uiv(s->id, ub->location, ub->count, (GLuint*)ub->data); break;
				case 2: glProgramUniform2uiv(s->id, ub->location, ub->count, (GLuint*)ub->data); break;
				case 3: glProgramUniform3uiv(s->id, ub->location, ub->count, (GLuint*)ub->data); break;
				case 4: glProgramUniform4uiv(s->id, ub->location, ub->count, (GLuint*)ub->data); break;
					
				default:
					fprintf(stderr, "!!! invalid size in bind_uniform: %d\n", ub->size);
					break;
			}
			break;
			
		case UT_FLOAT:
			switch(ub->size) {
				case 1: glProgramUniform1fv(s->id, ub->location, ub->count, (GLfloat*)ub->data); break;
				case 2: glProgramUniform2fv(s->id, ub->location, ub->count, (GLfloat*)ub->data); break;
				case 3: glProgramUniform3fv(s->id, ub->location, ub->count, (GLfloat*)ub->data); break;
				case 4: glProgramUniform4fv(s->id, ub->location, ub->count, (GLfloat*)ub->data); break;
					
				default:
					fprintf(stderr, "!!! invalid size in bind_uniform: %d\n", ub->size);
					break;
			}
			break;
			
		//case UT_DOUBLE: // ext support required, i think. not used by this program atm.
			
		case UT_MATRIX_2x2:
			glProgramUniformMatrix2v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_2x3:
			glProgramUniformMatrix2x3v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_2x4:
			glProgramUniformMatrix2x4v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_3x2:
			glProgramUniformMatrix3x2v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_3x3:
			glProgramUniformMatrix3v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_3x4:
			glProgramUniformMatrix3x4v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_4x2:
			glProgramUniformMatrix4x2v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_4x3:
			glProgramUniformMatrix4x3v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
			
		case UT_MATRIX_4x4:
			glProgramUniformMatrix4v(s->id, ub->location, ub->count, ub->transpose, (GLfloat*)ub->data);
			break;
		
		default:
			fprintf(stderr, "!!! unknown type in bind_uniform: %d\n", ub->type);
			break;
	}
}


static void bind_all_uniforms(Drawable* d) {
	for(int i = 0; i < VEC_LEN(&d->uniforms); i++) {
		bind_uniform(&VEC_ITEM(&d->uniforms, i), d->prog);
	}
}


static int attrib_size(struct vert_attrib_info* a) {
	return attrib_type_size(a) * a->count;
}


static int attrib_type_size(struct vert_attrib_info* a) {
	int type_size = 0;
	switch(a->type) {
		case GL_DOUBLE: 
			type_size = 8; 
			break;
		case GL_FLOAT: 
			type_size = 4; 
			break;
		case GL_SHORT: 
		case GL_UNSIGNED_SHORT: 
			type_size = 2; 
			break;
		case GL_BYTE: 
		case GL_UNSIGNED_BYTE: 
			type_size = 1; 
			break;
		case GL_MATRIX_EXT: // abused for this function. does not conflict with anything
			type_size = 4*16; 
			break;
		default:
			fprintf(stderr, "Unsupported VAO type\n");
			exit(2);
	}
	
	return type_size;
}


static int calc_vbo_stride(struct vertex_buffer_info* vbi) {
	int stride = 0;
	for(int i = 0; i < VEC_LEN(&vbi->attribs); i++) {
		struct vert_attrib_info* a = &VEC_ITEM(&vbi->attribs, i);
		a->size = attrib_size(a);
		stride += a->size;
	}
	
	return stride;
}


static void bind_vbo_for_write(struct vertex_buffer_info* vbi) {
	
	if(vbi->stride <= 0) {
		vbi->stride = calc_vbo_stride(vbi);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, vbi->id);
	
	int offset = 0;
	for(int i = 0; i < VEC_LEN(&vbi->attribs); i++) {
		struct vert_attrib_info* a = &VEC_ITEM(&vbi->attribs, i);
		
		glEnableVertexAttribArray(a->index);
		
		switch(a->type) {
			case GL_INT:
			case GL_UNSIGNED_INT:
			case GL_SHORT: 
			case GL_UNSIGNED_SHORT: 
			case GL_BYTE: 
			case GL_UNSIGNED_BYTE: 
				glVertexAttribIPointer(a->index, a->count, a->type, a->normalized, vbi->stride, offset);
				break;
				
			case GL_DOUBLE:
				glVertexAttribLPointer(a->index, a->count, a->type, a->normalized, vbi->stride, offset);
				break;
				
			case GL_FLOAT:
			case GL_MATRIX_EXT:
			default:
				glVertexAttribPointer(a->index, a->count, a->type, a->normalized, vbi->stride, offset);
				break;
		}
		
		glVertexAttribDivisor(a->index, a->divisor);
		
		offset += a->size;
	}
	
}

static void bind_all_vbos_for_draw(Drawable* d) {
	for(int i = 0; i < VEC_LEN(&d->vbos); i++) {
		glBindBuffer(GL_ARRAY_BUFFER, VEC_ITEM(&d->vbos, i).id);
	}
}





void Drawable_Init(Drawable* d) {
	VEC_INIT(d->vbos);
	VEC_INIT(d->uniforms);
}


void Drawable_Draw(Drawable* d) {
	
	glUseProgram(d->prog->id);
	
	bind_all_uniforms(d);
	
	glBindVertexArray(d->vao);
	bind_all_vbos_for_draw(d);
	
	if(!d->indexType) {
		glDrawArrays(d->primType, d->firstIndex, d->drawCnt);
	}
	else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->indexBuffer_id);
		glDrawElementsInstanced(d->primType, d->drawCnt, d->indexType, 0, d->instCnt);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void Drawable_AddVBO(Drawable* d, struct vert_attrib_info* att) {
	
	struct vertex_buffer_info* vbi;
	
	VEC_INC(&d->vbos);
	vbi = &VEC_LAST(&d->vbos);
	
	VEC_INIT(&vbi->attribs);
	
	for(int i = 0; att[i].type != 0; i++) {
		VEC_PUSH(&vbi->attribs, att[i]);
	}
}




void Vegetation_Init(Vegetation* v) {
	
	v->d = calloc(1, sizeof(*v->d));
	Drawable_Draw(v->d);
	
	Drawable_AddVBO(&v->d, (struct vert_attrib_info[]){
		{GL_FLOAT, 0, 4, GL_FALSE, 0},
		{GL_FLOAT, 1, 4, GL_FALSE, 0},
		{0},
	})
	&v->d->vbos
			 
			 
			 
	
}






