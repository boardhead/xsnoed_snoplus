//
// File:		PSnoDBWindow.h
//
// Description:	SNODB database viewer control window
//
// Created:		11/15/00 - P. Harvey
//

#ifndef __PSnoDBWindow_h__
#define __PSnoDBWindow_h__

#include "PWindow.h"
#include "PMenu.h"
#include "PSnoDBInterface.h"


class PSnoDBWindow : public PWindow, public PMenuHandler {
public:
	PSnoDBWindow(ImageData *data);
	~PSnoDBWindow();
	
	void			ViewSnoDB(int code);
	virtual void	DoMenuCommand(int anID);
	
private:
	static void		LoadProc(Widget w,PSnoDBWindow *db_win, caddr_t call_data);
	static void		DiffProc(Widget w,PSnoDBWindow *db_win, caddr_t call_data);
	static void     CancelProc(Widget w, Widget aShell, caddr_t call_data);	
	
	PSnoDBInterface	  *	mSnoDBInterface;
	ESnoDB_Parameter	mDataType;
	
	int				mRawData[kSnoDB_ParameterMax];
	Widget			mMessageLabel, mParamText, mRawDataText, mCellText;
	Widget			mDateText1, mTimeText1, mDateText2, mTimeText2;
};

#endif // __PSnoDBWindow_h__
