//
// File:	PColourWindow.h
//
// Created:	03/02/00 - Phil Harvey
//
#include "PImageWindow.h"

class PColourPicker;
class PColourWheel;

class PColourWindow : public PImageWindow
{
public:

	PColourWindow(ImageData *data);
	~PColourWindow();
	
	virtual void 	UpdateSelf();
	
	void			WheelColourChanging();
	void			WheelColourChanged();
	void			PickerColourChanged();
	void			SetIntensity(int val);

private:
	void			UpdateSliderPosition();
	void			UpdateText();
	
	static void		IntensityProc(Widget w, PColourWindow *cwin, XmScaleCallbackStruct *call_data);
	static void		ScaleDragProc(Widget w, PColourWindow *cwin, XmScaleCallbackStruct *call_data);
	static void		OkProc(Widget w, PColourWindow *cwin, caddr_t call_data);
	static void		ApplyProc(Widget w, PColourWindow *cwin, caddr_t call_data);
	static void		RevertProc(Widget w, PColourWindow *cwin, caddr_t call_data);
	static void		CancelProc(Widget w, PColourWindow *cwin, caddr_t call_data);
	static void		TextProc(Widget w, PColourWindow *cwin, caddr_t call_data);
	
	PColourPicker *	mColourPicker;
	PColourWheel  *	mColourWheel;
	
	Widget			mText[3], mSlide;
};
