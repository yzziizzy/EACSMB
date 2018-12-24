#ifndef __EACSMB_ui_monitors_h__
#define __EACSMB_ui_monitors_h__


typedef struct GUIFloatMonitor {
	GUIHeader header;
	
	GUIText* text;
	
	char* format;
	float* target;
	
	char* buffer;
	size_t bufferLen;
	
} GUIFloatMonitor;



GUIFloatMonitor* GUIFloatMonitor_new(GUIManager* gm, char* format, float* target);



#endif // __EACSMB_ui_monitors_h__
