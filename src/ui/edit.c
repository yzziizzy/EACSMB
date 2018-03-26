

#include "../gui.h"




static void insertChar(GUIEdit* ed, char c);
static void updateTextControl(GUIEdit* ed);
static void fireOnchange(GUIEdit* ed);




void guiEditRender(GUIEdit* ed, GameState* gs) {
	
	guiRender(ed->bg, gs);
	guiRender(ed->textControl, gs);
	
	// TODO: cycle blink here
	guiRender(ed->cursor, gs);
}

void guiEditDelete(GUIEdit* sw) {
	
	
	
	
}



static void recieveText(InputEvent* ev, GUIEdit* ed) {
	insertChar(ed, ev->character);
	updateTextControl(ed);
}



GUIEdit* GUIEditNew(char* initialValue, Vector2 pos, Vector2 size) {
	
	GUIEdit* ed;
	
	
	static struct gui_vtbl static_vt = {
		.Render = (void*)guiEditRender,
		.Delete = (void*)guiEditDelete
	};
		
	static InputEventHandler input_vt = {
		.keyText = recieveText,
	};
	
	ed = calloc(1, sizeof(*ed));
	CHECK_OOM(ed);
	
	guiHeaderInit(&ed->header);
	ed->header.vt = &static_vt;
	ed->inputHandlers = &input_vt;
	
	ed->header.hitbox.min.x = pos.x;
	ed->header.hitbox.min.y = pos.y;
	ed->header.hitbox.max.x = pos.x + size.x;
	ed->header.hitbox.max.y = pos.y + size.y;
	
	ed->blinkRate = 1.5;
	
	if(initialValue) {
		ed->textlen = strlen(initialValue);
		ed->buflen = nextPOT(ed->textlen + 1);
		ed->buf = malloc(ed->buflen);
		strcpy(ed->buf, initialValue);
	}
	else {
		ed->textlen = 0;
		ed->buflen = 16;
		ed->buf = malloc(16);
		ed->buf[0] = 0;
	}
	
	ed->cursorpos = ed->textlen;
	
	ed->bg = guiWindowNew(pos, size, 1);
	ed->bg->color = (Vector){0.1, 0.1, 0.1};
	ed->bg->borderColor = (Vector4){1.0, .7, .3, 1.0};
	guiRegisterObject(ed->bg, &ed->header);
	
	// TODO: fix size and pos of cursor
	ed->cursor = guiWindowNew(pos, (Vector2){.003, size.y}, 1);
	ed->cursor->color = (Vector){1.0, 1.0, 1.0};
	guiRegisterObject(ed->cursor, &ed->bg->header);
	
	ed->textControl = guiTextNew(initialValue, pos, 6.0f, "Arial");
	ed->textControl->header.size.x = .5;
	guiRegisterObject(ed->textControl, &ed->bg->header);

	
	
	return ed;
}


static void growBuffer(GUIEdit* ed, int extra) {
	ed->buflen = nextPOT(ed->textlen + extra + 1);
	ed->buf = realloc(ed->buf, ed->buflen);
}

static void checkBuffer(GUIEdit* ed, int minlen) {
	if(ed->buflen < minlen + 1) {
		growBuffer(ed, ed->buflen - minlen + 1);
	}
}

// at the cursor. does not move the cursor.
static void insertChar(GUIEdit* ed, char c) { 
	checkBuffer(ed, ed->textlen + 1);
	
	char* e = ed->buf + ed->textlen + 1; // copy the null terminator too
	while(e >= ed->buf + ed->cursorpos) {
		*e = *(e - 1);
		e--;
	}
	
	ed->textlen++;
	*e = c;
}

static void updateTextControl(GUIEdit* ed) {
	guiTextSetValue(ed->textControl, ed->buf);
	
	// get new cursor pos
	ed->cursorOffset = guiTextGetTextWidth(ed->textControl, ed->cursorpos);
	
	fireOnchange(ed);
}

// just changes the text value. triggers nothing.
static void setText(GUIEdit* ed, char* s) {
	int len = strlen(s);
	checkBuffer(ed, len);
	
	strcpy(ed->buf, s);
	
	// TODO: check and adjust cursor pos if text shrank
}

static void fireOnchange(GUIEdit* ed) {
	if(ed->onChange) {
		(*ed->onChange)(ed, ed->onChangeData);
	}
}



void guiEditSetText(GUIEdit* ed, char* text) {
	setText(ed, text);
}




void guiEditSetInt(GUIEdit* ed, int64_t ival) {
	fprintf(stderr, "FIXME: GUIEditSetInt\n");
	guiEditSetDouble(ed, ival);
}

void guiEditSetDouble(GUIEdit* ed, double dval) {
	char txtVal[64]; 
	
	ed->numVal = dval;
	
	gcvt(dval, 6, txtVal);
	guiEditSetText(ed, txtVal);
}


static int updateDval(GUIEdit* ed) {
	char* end = NULL;
	
	double d = strtod(ed->buf, &end);
	if(ed->buf == end) { // conversion failed
		ed->numVal = 0.0;
		return 1;
	}
	
	ed->numVal = d;
	return 0;
}


double guiEditGetDouble(GUIEdit* ed) {
	// TODO: cache this value
	updateDval(ed);
	return ed->numVal;
}



