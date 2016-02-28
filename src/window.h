#ifndef __EACSMB_WINDOW_H__
#define __EACSMB_WINDOW_H__



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


#define IS_KEYDOWN    0x01 // is it down now
#define IS_KEYPRESSED 0x02 // was the key pressed ever
#define IS_CONTROL    0x04
#define IS_SHIFT      0x08
#define IS_ALT        0x10
#define IS_TUX        0x20 // aka "windows key"


enum InputMode {
	CLICK_MODE,
	DRAG_MODE
	
};


typedef struct {
	
	// shitty for now
	Vector2 clickPos;
	Vector2 cursorPos;
	Vector2 cursorPosPixels;
	char clickButton;
	char buttonUp;
	char buttonDown;
	
	unsigned char keyState[256];
	
	enum InputMode mode;
	
} InputState;


XErrorEvent* xLastError;
char xLastErrorStr[1024];
 
int initXWindow(XStuff* xs);

void processEvents(XStuff* xs, InputState* st, int max_events);
void clearInputState(InputState* st);


#endif // __EACSMB_WINDOW_H__
