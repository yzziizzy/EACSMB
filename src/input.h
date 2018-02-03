#ifndef __EACSMB__input_h__
#define __EACSMB__input_h__

#include "ds.h"
#include "common_math.h"


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

enum InputEventType {
	EVENT_KEYDOWN,
	EVENT_KEYUP,
	EVENT_KEYPRESSED
	
};

typedef struct {
	char type; // 0 = kb, 1 = mouse
	
	unsigned int keycode;
	unsigned int click_x, click_y;
	
	double time;
	
} InputEvent;


typedef struct {
	
	// shitty for now
	Vector2 clickPos;
	Vector2 cursorPos;
	Vector2 cursorPosPixels;
	Vector2 cursorPosInv;
	char clickButton;
	char buttonUp;
	char buttonDown;
	
	unsigned char keyState[256];
	double keyStateChanged[256];
	
	VEC(InputEvent*) events;
	
	enum InputMode mode;
	
} InputState;


void clearInputState(InputState* st);



#endif // __EACSMB__input_h__
