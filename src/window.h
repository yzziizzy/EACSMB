#ifndef __EACSMB_WINDOW_H__
#define __EACSMB_WINDOW_H__

#include <X11/X.h>
#include <X11/Xlib.h>

#include "common_gl.h"
#include "input.h"


typedef struct XStuff {
	Display*     display;
	Window       rootWin;
	Window       clientWin;
	GLXContext   glctx;
	
	XVisualInfo* vi;
	Colormap     colorMap;
	
	XWindowAttributes winAttr;
	
	void (*onExpose)(struct XStuff*, void*);
	void* onExposeData;

	int targetMSAA;
	char* windowTitle;
	
	
	Bool ready;
} XStuff;





XErrorEvent* xLastError;
char xLastErrorStr[1024];
 
int initXWindow(XStuff* xs);

void processEvents(XStuff* xs, InputState* st, int max_events);



#endif // __EACSMB_WINDOW_H__
