//
// File:		PExtraLabels.h
//
// Description:	Mix-in utility class to provide simple mechanism for adding extra
//				pairs of labels to one or two RowColumn parents.
//
// Created:		P. Harvey - 03/16/00
//
#ifndef __PExtraLabels_h__
#define __PExtraLabels_h__

#include <Xm/Xm.h>
#include "PLabel.h"

const short		kMaxExtraLabels	= 16;

class PExtraLabels {
public:
	PExtraLabels();
	virtual ~PExtraLabels();
	
	void			ClearExtraLabels();
	void			SetExtraLabel(int num, char *str);
	int				SetExtraNum(int num);
	void			SetExtraPanes(Widget w1, Widget w2);
	
	int				GetExtraNum()			{ return mExtraNum; }
	Widget			GetExtraWidget(int n)	{ return mExtraValue[n].GetWidget(); }

protected:
	virtual char *	GetLabelString(int num) = 0;
	
	int				mFirstExtraRow;

private:
	PLabel			mExtraLabel[kMaxExtraLabels];
	PLabel			mExtraValue[kMaxExtraLabels];
	int				mExtraNum;
	Widget			mPane1, mPane2;
};


#endif // __PExtraLabels_h__
