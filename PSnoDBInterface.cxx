//
// Virtual base class for interface to load values from the SNODB
//
// Created: 11/15/00 - P. Harvey
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <Xm/Xm.h>
#include "PSnoDBInterface.h"
#include "PUtils.h"

PSnoDBInterface::PSnoDBInterface(WidgetPtr label)
{
	mMessageLabel = label;
	mParamString = NULL;
}

PSnoDBInterface::~PSnoDBInterface()
{
	delete [] mParamString;
}

// GetParamString - return parameter string
// (guaranteed NOT to return a NULL string)
char *PSnoDBInterface::GetParamString()
{
	if (mParamString) {
		return mParamString;
	} else {
		return "";
	}
}

// SetParamString - set parameter string
void PSnoDBInterface::SetParamString(char *str)
{
	delete [] mParamString;
	if (str) {
		int nbytes = strlen(str) + 1;
		mParamString = new char[nbytes];
		if (mParamString) {
			memcpy(mParamString, str, nbytes);
		}
	} else {
		mParamString = NULL;
	}
}

void PSnoDBInterface::Message(char *fmt, ...)
{
	va_list	varArgList;
	char	buff[1024];
	
	va_start(varArgList,fmt);
	vsprintf(buff,fmt,varArgList);
	va_end(varArgList);

	if (mMessageLabel) {
		setLabelString((Widget)mMessageLabel, buff);
		XmUpdateDisplay((Widget)mMessageLabel);	// update the label now
	} else {
		printf("%s\n",buff);
	}
}
