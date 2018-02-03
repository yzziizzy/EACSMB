

#include "input.h"



void clearInputState(InputState* st) {
	int i;
	
	for(i = 0; i < 256; i++) {
		st->keyState[i] &= IS_KEYDOWN;
	}
	
	st->clickPos.y = -1;
	st->clickPos.x = -1;
	st->clickButton = 0;
	st->buttonUp = 0;
	st->buttonDown = 0;
	
}





