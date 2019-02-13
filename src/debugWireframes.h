#ifndef __EACSMB_debugWirefames_h__
#define __EACSMB_debugWirefames_h__


#include "common_math.h"

#include "pass.h"



void initDebugWireframe();
void resetDebugWireframes();



void debugWF_Line(Vector* p1, Vector* p2, char* color1, char* color2, float width1, float width2);

void debugWF_ProjMatrix(Matrix* m);
void debugWF_AABB(AABB* aabb, char* color, float width);
//void debugWF_Sphere(Sphere* s, char* color, float width);
//void debugWF_Vector(Vector* origin, Vector* dir, float length, char* color, float width);


void renderDebugWireframeLines(PassFrameParams* pfp);






#endif // __EACSMB_debugWirefames_h__
