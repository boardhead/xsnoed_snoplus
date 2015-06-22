//
// File:		PSharedString.h
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

struct SSharedString {
	int		alloc_count;	// number of times string is allocated
	int		size;			// size of the string to follow (bytes incl. null term)
	// the string follows ...
};

class PSharedString {
public:
					PSharedString()		{ Construct(); }
					~PSharedString()		{ Destruct(); }
	
	void			Construct()			{ mString = (SSharedString *)0; }
	void			Destruct()			{ Release(); }
	
	void			SetString(char *str);
	void			SetString(PSharedString &aSharedString);
	
	char *			GetString();
	
	void			Release();
	
private:
	SSharedString	*mString;
};
