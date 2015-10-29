#ifndef __ui_h__
#define __ui_h__



#define LIST(x) \
	struct x; \
	typedef struct x##List { \
		struct x* d; \
		struct x##List* next; \
	} x##List;

#define LISTU(x) \
	union x; \
	typedef struct x##List { \
		union x* d; \
		struct x##List* next; \
	} x##List;




LIST(UIWindow);
LISTU(UIWin);

union UIWin;


typedef struct UIWindow {
	unsigned int type;
	AABB2 box;
	float z;
	struct UIWinList* kids;
	union UIWin* parent;
} UIWindow;



typedef struct UISolidWin {
	UIWindow win;
	unsigned int bgColor;
	AABB2 padding;
} UISolidWin;





typedef struct UITextWin {
	UIWindow win;
	char* text;
	int textLen;
	TextRes* font;
} UITextWin;








typedef union UIWin {
	struct {
		unsigned int type;
		AABB2 box;
		float z;
		struct UIWinList* kids;
		union UIWin* parent;
	};
	
	UIWindow _win;
	UITextWin _textWin;
	
	
	
	
} UIWin;



extern UIWindow uiRootWin;




#endif // __ui_h__