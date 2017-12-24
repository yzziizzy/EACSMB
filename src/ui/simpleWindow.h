#ifndef __EACSMB_ui_simpleWindow_h__
#define __EACSMB_ui_simpleWindow_h__


#include "../gui.h"



typedef struct {
	GUIHeader header;
	
	GUIWindow* bg;
	GUIWindow* titlebar;
	GUIWindow* closebutton;
	
	GUIText* titleText;
	
	char* title;
	
} GUISimpleWindow;




GUISimpleWindow* guiSimpleWindowNew(Vector2 pos, Vector2 size, float zIndex);






#endif // __EACSMB_ui_simpleWindow_h__
