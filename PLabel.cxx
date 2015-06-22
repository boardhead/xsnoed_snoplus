/*
** PLabel	- PH 10/15/99
**
** A wrapper to avoid unnecessary screen updates if the label doesn't change
*/
#include <string.h>
#include <Xm/Label.h>
#include "PLabel.h"
#include "PUtils.h"

PLabel::PLabel(Widget aLabel)
{
	mString = NULL;
	mLabel = aLabel;
}

PLabel::~PLabel()
{
	FreeString();
}

// CreateLabel - create XmLabel widget
// - if managed is 0, the widget is created but not managed
void PLabel::CreateLabel(char *name, Widget parent, ArgList args, Cardinal num_args, int managed)
{
	if (managed) {
		mLabel = XtCreateManagedWidget(name,xmLabelWidgetClass,parent,args,num_args);
	} else {
		mLabel = XtCreateWidget(name,xmLabelWidgetClass,parent,args,num_args);
	}
	// the widget name is the initial string, so save it
	SaveString(name);
}

// DestroyLabel - destroy the label widget
void PLabel::DestroyLabel()
{
	if (mLabel) {
		XtDestroyWidget(mLabel);
		mLabel = NULL;
	}
	FreeString();
}

// CreateString - create string and save length of allocated string
void PLabel::CreateString(int len)
{
	mStringLen = len;
	mString = XtMalloc(mStringLen + 1);	// need extra character for NULL terminator
}

// FreeString - free our copy of the string
void PLabel::FreeString()
{
	if (mString) {
		XtFree(mString);
		mString = NULL;
	}
}

// SaveString - save string to our local copy
// - returns zero if string has not changed
int PLabel::SaveString(char *str)
{
	int		len;
	
	if (mString) {
		if (!strcmp(mString, str)) return(0);	// text is the same
		len = strlen(str);
		if (len > mStringLen) {
			// only reallocate string memory if new string is larger
			FreeString();
			CreateString(len);
		}
	} else {
		CreateString(strlen(str));
	}
	if (mString) {
		strcpy(mString, str);
	}
	return(1);
}

// SetString - set the label string (must create a label before this is called!)
void PLabel::SetString(char *str)
{
	if (SaveString(str)) {
		setLabelString(mLabel, str);	// set the XmLabel string
	}
}

// SetStringNow - set the label and draw immediately
// - call SetString() instead of this routine unless you specifically need
//   to update the string before the next time through the event loop
void PLabel::SetStringNow(char *str)
{
	if (SaveString(str)) {
		setLabelString(mLabel, str);
		XmUpdateDisplay(mLabel);
	}
}
