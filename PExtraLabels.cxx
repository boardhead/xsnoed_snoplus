//
// File:		PExtraLabels.cxx
//
// Description:	Mix-in utility class to provide simple mechanism for adding extra
//				pairs of labels to one or two RowColumn parents.
//
// Created:		P. Harvey - 03/16/00
//

#include <stdio.h>
#include "PExtraLabels.h"

PExtraLabels::PExtraLabels()
{
	mExtraNum = 0;
	mFirstExtraRow = 0;
	mPane1 = NULL;
	mPane2 = NULL;
}

PExtraLabels::~PExtraLabels()
{
}

void PExtraLabels::SetExtraPanes(Widget w1, Widget w2)
{
	mPane1 = w1;
	mPane2 = w2;
}

void PExtraLabels::ClearExtraLabels()
{
	for (int i=0; i<mExtraNum; ++i) {
		mExtraValue[i].SetString("-");
	}
}

void PExtraLabels::SetExtraLabel(int num, char *str)
{
	mExtraValue[num].SetString(str);
}

// SetExtraNum - set the number of extra labels
// - returns non-zero if the number of extra labels was changed
int PExtraLabels::SetExtraNum(int newNum)
{
	int			i, pos, rows, count, num;
	char		buff[32];
	Arg			wargs[1];
	Widget		widgets[2*kMaxExtraLabels];
	int			changed = 0;

	if (!mPane1) return(changed);
	
	// calculate new number of extra data fields to display
	if (newNum > kMaxExtraLabels) newNum = kMaxExtraLabels;
	
	// set the label strings of labels that already exist
	for (i=0; i<mExtraNum && i<newNum; ++i) {
		sprintf(buff,"%s:",GetLabelString(i));
		mExtraLabel[i].SetString(buff);
	}
	
	if (mExtraNum > newNum) {
		// remove unnecessary labels
		count = 0;
		num = mExtraNum - newNum;	// number of widget pairs to remove
		for (i=mExtraNum-1; i>=newNum; --i) {
			widgets[count] = mExtraLabel[i].GetWidget();
			widgets[count+num] = mExtraValue[i].GetWidget();
			++count;
		}
		if (mPane1 == mPane2) {
			// unmanage all widgets at once (to reduce flickering)
			XtUnmanageChildren(widgets, 2*count);
		} else {
			XtUnmanageChildren(widgets, count);
			XtUnmanageChildren(widgets+num, count);
		}
		// destroy the widgets
		do {
			--mExtraNum;
			mExtraLabel[mExtraNum].DestroyLabel();
			mExtraValue[mExtraNum].DestroyLabel();
		} while (mExtraNum > newNum);
		
		changed = 1;
		
	} else if (mExtraNum<newNum && mExtraNum<kMaxExtraLabels) {
		// add necessary labels
		count = 0;
		rows = mFirstExtraRow + newNum;
		if (newNum > kMaxExtraLabels) {
			newNum = kMaxExtraLabels;	// limit the maximum number of labels
		}
		num = newNum - mExtraNum;		// number of widget pairs to add
		do {
			pos = mFirstExtraRow + mExtraNum;
			if (mPane1 == mPane2) {
				// I must admit, I don't understand this positioning,
				// but it works by trial and error, so here it is
				XtSetArg(wargs[0], XmNpositionIndex, pos + rows + 1);
			} else {
				XtSetArg(wargs[0], XmNpositionIndex, pos);
			}
			mExtraValue[mExtraNum].CreateLabel("-", mPane2, wargs, 1, 0);
			widgets[count] = mExtraValue[mExtraNum].GetWidget();
			XtSetArg(wargs[0], XmNpositionIndex, pos);
			sprintf(buff,"%s:",GetLabelString(mExtraNum));
			mExtraLabel[mExtraNum].CreateLabel(buff, mPane1, wargs, 1, 0);
			widgets[count+num] = mExtraLabel[mExtraNum].GetWidget();
			++count;	// count pairs of widgets added
			++mExtraNum;
			++rows;
		} while (mExtraNum < newNum);
		
		// manage all widgets at once (to reduce flickering)
		if (mPane1 == mPane2) {
			XtManageChildren(widgets, 2*count);
		} else {
			// must manage children separately if they have different parents
			XtManageChildren(widgets, count);
			XtManageChildren(widgets+num, count);
		}
		
		changed = 1;
	}
	return(changed);
}

