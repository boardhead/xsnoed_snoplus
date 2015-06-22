//
// File:		PSharedString.cxx
//
// Created:		06/09/00 - P. Harvey
//
// Description:	A string class to allow sharing of a commonly allocated string
//
// Usage:	1)	Set the string initially with SetString(char *)
//			2)	Make copies of a string by calling SetString(PSharedString *)
//
// Notes:		The Construct() and Destruct() methods are provided in case
//				the regular C++ constructor/destructor needs to be bypassed.
//

#include <string.h>
#include "PSharedString.h"

void PSharedString::Release()
{
	// delete the string when the allocation count reaches zero
	if (mString && --mString->alloc_count == 0) {
		delete [] (char *)mString;
	}
	mString = (SSharedString *)0;
}

void PSharedString::SetString(char *str)
{
	Release();	// first, release our old string
	
	if (str) {
		int size = strlen(str) + 1;
		mString = (SSharedString *)(new char[sizeof(SSharedString) + size]);
		if (mString) {
			mString->alloc_count = 1;
			mString->size = size;
			strcpy((char *)(mString + 1), str);
		}
	}
}

// share a string with another PSharedString object
void PSharedString::SetString(PSharedString &multiStr)
{
	Release();	// first, release our old string
	
	if (multiStr.mString) {
		mString = multiStr.mString;
		++mString->alloc_count;
	}
}

char * PSharedString::GetString()
{
	if (mString) {
		return((char *)(mString + 1));
	} else {
		return((char *)"");		// never return a NULL (return empty string instead)
	}
}
