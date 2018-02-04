

#include "input.h"



void clearInputState(InputState* st) {
	int i;
	
	for(i = 0; i < 256; i++) {
		st->keyState[i] &= IS_KEYDOWN;
	}
	
	//st->clickPos.y = -1;
//	st->clickPos.x = -1;
	st->clickButton = 0;
	st->buttonUp = 0;
	st->buttonDown = 0;
	
}



// effective usage with macro:
// InputFocusStack_PushTarget(InputFocusStack* stack, YourType* data, vtable_field_name_in_data);
void _InputFocusStack_PushTarget(InputFocusStack* stack, void* data, ptrdiff_t vtoffset) {
	
	InputFocusTarget t;
	
	InputFocusTarget* t2 = &VEC_TAIL(&stack->stack);
	if(t2->vt->loseFocus) {
		t2->vt->loseFocus(NULL, t2->data);
	}
	
	t.data = data;
	t.vt = (InputEventHandler*)(data + vtoffset);
	
	VEC_PUSH(&stack->stack, t);
	if(t.vt->gainFocus) {
		t.vt->gainFocus(NULL, t.data);
	}
}



void InputFocusStack_RevertTarget(InputFocusStack* stack) {
	
	// notify of losing focus
	InputFocusTarget* t = &VEC_TAIL(&stack->stack);
	if(t->vt->loseFocus) {
		t->vt->loseFocus(NULL, t->data);
	}
	
	VEC_POP(&stack->stack);
	
	// notify of gaining focus
	t = &VEC_TAIL(&stack->stack);
	if(t->vt->gainFocus) {
		t->vt->gainFocus(NULL, t->data);
	}
}


void InputFocusStack_Dispatch(InputFocusStack* stack, InputEvent* ev) {
	
	fprintf(stderr, "\n!!! InputFocusStack_Dispatch not implemented\n\n");
	
}


