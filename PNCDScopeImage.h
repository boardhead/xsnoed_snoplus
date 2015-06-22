//
// File:		PNCDScopeImage.h
//
// Description:	NCD scope image
//
// Revisions:	09/05/03 - PH Created
//
#ifndef __PNCDScopeImage_h__
#define __PNCDScopeImage_h__

#include "PHistImage.h"

class PNCDScopeImage : public PHistImage
{
public:
	PNCDScopeImage(PImageWindow *owner, Widget canvas=0, int createCanvas=1);
	    
	virtual void	DrawSelf();
    virtual void    DoneGrab();
	
	void            SetCalibrated(int scope, int cal);
	int             IsCalibrated(int scope) { return mIsCalibrated[scope]; }

private:
    int             mIsCalibrated[2];
};

#endif // __PNCDScopeImage_h__
