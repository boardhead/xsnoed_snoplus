#ifndef __TextSpec_h__
#define __TextSpec_h__

#include <X11/Xlib.h>

struct TextSpec {
	XFontStruct	  *	font;
	char		  *	string;
};

#endif // __TextSpec_h__
