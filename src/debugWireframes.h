#ifndef __EACSMB_debugWirefames_h__
#define __EACSMB_debugWirefames_h__


#include "common_math.h"

#include "pass.h"



void initDebugWireframe();
void resetDebugWireframes();



void debugWFAddLine(Vector* p1, Vector* p2, char* color1, char* color2, float width1, float width2);

void debugWFProjMatrix(Matrix* m);


void renderDebugWireframeLines(PassFrameParams* pfp);






#endif // __EACSMB_debugWirefames_h__
