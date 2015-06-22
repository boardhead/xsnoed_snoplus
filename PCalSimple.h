//
// File:		PCalSimple.h
//
// Created:		09/13/00 - P. Harvey
//
// Description:	Simple calibration object
//
#ifndef __PCalSimple_h__
#define __PCalSimple_h__

#include "PCal.h"

struct DBase;

class PCalSimple : public PCal {
public:
					PCalSimple();
					~PCalSimple();
					
	virtual void	Init(char *filename, char *path=0);
	virtual void	Free();
	virtual void	Calibrate(int calFlags=kCalAll);
	
	void			SetWalk(int doWalk)				{ mDoWalk = doWalk; }
	
private:
	DBase		  *	mDBase;			// database pointer
	int				mDoWalk;		// non-zero if walk correction is performed
};

#endif // __PCalSimple_h__
