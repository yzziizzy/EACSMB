



typedef struct {
	Display*     display;
	Window       rootWin;
	Window       clientWin;
	GLXContext   glctx;
	
	XVisualInfo* vi;
	Colormap     colorMap;
	
	XSetWindowAttributes winAttr;
	XWindowAttributes gwa;

	int targetMSAA;
	char* windowTitle;
} XStuff;



XErrorEvent* xLastError;
char xLastErrorStr[1024];
 
int initXWindow(XStuff* xs);
	
	