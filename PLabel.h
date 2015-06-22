/*
** PLabel	- PH 10/15/99
**
** A wrapper to avoid unnecessary screen updates if the label doesn't change
*/
#ifndef __PLABEL_H__
#define __PLABEL_H__

#include <Xm/Xm.h>

class PLabel {
public:
	PLabel(Widget aLabel=NULL);
	virtual ~PLabel();
	
	void		CreateLabel(char *name, Widget parent, ArgList args, Cardinal num_args, int managed=1);
	void		DestroyLabel();

	void		SetString(char *str);
	void		SetStringNow(char *str);
	
	char	  *	GetString()	{ return mString; }
	Widget		GetWidget()	{ return mLabel; }
	
protected:
	void		CreateString(int len);
	void		FreeString();
	int			SaveString(char *str);
	
private:
	Widget		mLabel;
	char	  *	mString;
	int			mStringLen;
};


#endif // __PLABEL_H__
