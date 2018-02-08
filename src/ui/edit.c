

#include "../gui.h"





void guiEditRender(GUIEdit* ed, GameState* gs) {
	
	guiRender(ed->bg, gs);
	guiRender(ed->textControl, gs);
	
	// TODO: cycle blink here
	guiRender(ed->cursor, gs);
}

void guiEditDelete(GUIEdit* sw) {
	
	
	
	
}



GUIEdit* GUIEditNew(char* initialValue, Vector2 pos, Vector2 size) {
	
	GUIEdit* ed;
	
	
	static struct gui_vtbl static_vt = {
		.Render = guiEditRender,
		.Delete = guiEditDelete
	};
	
	ed = calloc(1, sizeof(*ed));
	CHECK_OOM(ed);
	
	guiHeaderInit(&ed->header);
	ed->header.vt = &static_vt;
	
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
	ed->bg->color = (Vector){0.1, 0.9, 0.1};
	guiRegisterObject(ed->bg, &ed->header);
	
	// TODO: fix size and pos of cursor
	ed->cursor = guiWindowNew(pos, size, 1);
	ed->cursor->color = (Vector){0.0, 0.0, 0.0};
	guiRegisterObject(ed->cursor, &ed->bg->header);
	
	ed->textControl = guiTextNew(initialValue, &(Vector){8.0,2.0,0.0}, 6.0f, "Arial");
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
	while(e > ed->buf + ed->cursorpos) {
		*e = *(e - 1);
		e--;
	}
	
	*e = c;
}

static void updateTextControl(GUIEdit* ed) {
	guiTextSetValue(ed->textControl, ed->buf);
	
	// get new cursor pos
	ed->cursorOffset = guiTextGetTextWidth(ed->textControl, ed->cursorpos);
	
}


static void setText(GUIEdit* ed, char* s) {
	
	printf("gui edit set text not implemented\n");
}














