#ifndef __PUtils_h__
#define __PUtils_h__

#include "CUtils.h"
#include <Xm/Xm.h>

void	setLabelString(Widget label, char *string);
char *	getLabelString(Widget label);
void	setTextString(Widget text, char *string);
int 	strncvtXm(char *out,XmString in,int n);
void	setToggle(Widget toggle, int on);

#endif // __PUtils_h__
