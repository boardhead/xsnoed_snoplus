//
// File:	PColourWindow.cxx
//
// Created:	03/02/00 - Phil Harvey
//
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/DrawingA.h>
#include <Xm/Separator.h>
#include "PColourWindow.h"
#include "PColourWheel.h"
#include "PColourPicker.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "ImageData.h"

const int	kPickerHeight = 16 * (NUM_COLOURS + 2) / 3;

#define WIN_WIDTH 	425
#define WIN_HEIGHT	(360 + kPickerHeight)
#define RGB_YPOS	18

PColourWindow::PColourWindow(ImageData *data)
			 : PImageWindow(data)
{
	int		n;
	Arg		wargs[20];
	Widget	w, but, canvas;
	
	mPrintable = 0;		// not printable
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "XSNOED Colors"); ++n;
	XtSetArg(wargs[n], XmNx, 320); ++n;
	XtSetArg(wargs[n], XmNy, 100); ++n;
	XtSetArg(wargs[n], XmNminWidth, WIN_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WIN_HEIGHT); ++n;
	SetShell(CreateShell("colPop",data->toplevel,wargs,n));
	
	n = 0;			
	XtSetArg(wargs[n], XmNwidth, WIN_WIDTH); ++n;
	XtSetArg(wargs[n], XmNheight, WIN_HEIGHT); ++n;
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),wargs,n);
	SetMainPane(w);
	
	n = 0;
	XtSetArg(wargs[n], XmNy, 242); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtCreateManagedWidget("sep",xmSeparatorWidgetClass,w,wargs,n);

	// create colour picker
	n = 0;
	XtSetArg(wargs[n], XmNx,	10);  ++n;
	XtSetArg(wargs[n], XmNy,	249);  ++n;
	XtSetArg(wargs[n], XmNwidth, 400); ++n;
	XtSetArg(wargs[n], XmNheight, 100 + kPickerHeight); ++n;
	canvas = XtCreateManagedWidget("colourPicker", xmDrawingAreaWidgetClass, w, wargs, n);
	mColourPicker = new PColourPicker(this, canvas);

	// create colour wheel (done after colour picker so it becomes our default ImageCanvas)
	n = 0;
	XtSetArg(wargs[n], XmNx,	12);  ++n;
	XtSetArg(wargs[n], XmNy,	11);  ++n;
	XtSetArg(wargs[n], XmNwidth, 178); ++n;
	XtSetArg(wargs[n], XmNheight, 178); ++n;
	canvas = XtCreateManagedWidget("colourWheel", xmDrawingAreaWidgetClass, w, wargs, n);
	// install a colour wheel in the window
	// Note: this must be done _after_ installing the colourPicker, because
	// we want this to be our default image canvas (to set the cursor, etc)
	mColourWheel = new PColourWheel(this, canvas, 178);

	n = 0;
	XtSetArg(wargs[n], XmNx, 194); ++n;
	XtSetArg(wargs[n], XmNy, 15); ++n;
	XtCreateManagedWidget("Intensity", xmLabelWidgetClass, w, wargs, n);

	n = 0;
#ifdef LESSTIF
	XtSetArg(wargs[n], XmNx, 180); ++n;
#else
	XtSetArg(wargs[n], XmNx, 199); ++n;
#endif
	XtSetArg(wargs[n], XmNy, 40); ++n;
	XtSetArg(wargs[n], XmNorientation, XmVERTICAL); ++n;
	XtSetArg(wargs[n], XmNheight, 150); ++n;
	XtSetArg(wargs[n], XmNwidth, 20); ++n;
	XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
	XtSetArg(wargs[n], XmNminimum, 0); ++n;
	// the slider scale is set to give even values when sliding...
	XtSetArg(wargs[n], XmNmaximum, 255); ++n;
	XtSetArg(wargs[n], XmNvalue, 255); ++n;
	mSlide = XtCreateManagedWidget("slide",xmScaleWidgetClass,w,wargs,n);
	XtAddCallback(mSlide,XmNvalueChangedCallback,(XtCallbackProc)IntensityProc,this);
	XtAddCallback(mSlide,XmNdragCallback,(XtCallbackProc)ScaleDragProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 20); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	but = XtCreateManagedWidget("OK",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)OkProc, this);
	// set initial focus to this button
	n = 0;
	XtSetArg(wargs[n], XmNinitialFocus, but); ++n;
	XtSetValues(w, wargs, n);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 120); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	but = XtCreateManagedWidget("Apply",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)ApplyProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 220); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	but = XtCreateManagedWidget("Revert",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)RevertProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 320); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	but = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)CancelProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 300); ++n;
	XtSetArg(wargs[n], XmNy, 15); ++n;
	XtCreateManagedWidget("Components", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 262); ++n;
	XtSetArg(wargs[n], XmNy, 36+RGB_YPOS); ++n;
	XtSetArg(wargs[n], XmNwidth, 50); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_END); ++n;
	XtCreateManagedWidget("Red:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 320); ++n;
	XtSetArg(wargs[n], XmNy, 30+RGB_YPOS); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	mText[0] = XtCreateManagedWidget("red", xmTextWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 262); ++n;
	XtSetArg(wargs[n], XmNy, 86+RGB_YPOS); ++n;
	XtSetArg(wargs[n], XmNwidth, 50); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_END); ++n;
	XtCreateManagedWidget("Green:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 320); ++n;
	XtSetArg(wargs[n], XmNy, 80+RGB_YPOS); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	mText[1] = XtCreateManagedWidget("green", xmTextWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 262); ++n;
	XtSetArg(wargs[n], XmNy, 136+RGB_YPOS); ++n;
	XtSetArg(wargs[n], XmNwidth, 50); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_END); ++n;
	XtCreateManagedWidget("Blue:", xmLabelWidgetClass, w, wargs, n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 320); ++n;
	XtSetArg(wargs[n], XmNy, 130+RGB_YPOS); ++n;
	XtSetArg(wargs[n], XmNwidth, 80); ++n;
	XtSetArg(wargs[n], XmNheight, 32); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 3); ++n;
	mText[2] = XtCreateManagedWidget("blue", xmTextWidgetClass, w, wargs, n);

	// add the callbacks to the text widgets
	for (int i=0; i<3; ++i) {
		XtAddCallback(mText[i],XmNactivateCallback,(XtCallbackProc)TextProc,this);
		XtAddCallback(mText[i],XmNlosingFocusCallback,(XtCallbackProc)TextProc, this);
	}
	
	// set our initial colour
	PickerColourChanged();
}

PColourWindow::~PColourWindow()
{
	// Note: mColourWindow will be deleted by the base class
	//       since it is the 'mImage'.
	delete mColourPicker;
}

void PColourWindow::UpdateSelf()
{
	// draw the colour wheel if necessary
	if (mColourWheel->IsDirty()) {
		mColourWheel->Draw();
	}
	
	// draw colour picker too
	if (mColourPicker->IsDirty()) {
		mColourPicker->Draw();
	}
}

// set current color from current picker colour
void PColourWindow::PickerColourChanged()
{
	int col3[3];
	
	// get RGB of current picker colour
	mColourPicker->GetColourRGB(col3);
	
	// set current wheel colour
	mColourWheel->SetColourRGB(col3);
	
	// update text and slider for current colour
	UpdateText();
	UpdateSliderPosition();
}

// WheelColourChanging - animate necessary items while wheel colour is changing
void PColourWindow::WheelColourChanging()
{
	int col3[3];
	mColourWheel->GetColourRGB(col3);
	mColourPicker->SetColourRGB(col3);
}

// WheelColourChanged - colour is done changing, update all required items
void PColourWindow::WheelColourChanged()
{
	UpdateText();
}

void PColourWindow::UpdateSliderPosition()
{
	Arg warg;
	XtSetArg(warg, XmNvalue, mColourWheel->GetIntensity());
	XtSetValues(mSlide, &warg, 1);
}

void PColourWindow::UpdateText()
{
	char	buff[32];
	int		col3[3];
	
	mColourWheel->GetColourRGB(col3);
	for (int i=0; i<3; ++i) {
		sprintf(buff,"%d", col3[i]);
		setTextString(mText[i], buff);
	}
}


void PColourWindow::SetIntensity(int val)
{
	// set colour wheel intensity
	mColourWheel->SetIntensity(val);
	UpdateSliderPosition();
}

void PColourWindow::IntensityProc(Widget w, PColourWindow *cwin, XmScaleCallbackStruct *call_data)
{
	PColourWheel *cwheel = (PColourWheel *)cwin->mImage;
	cwheel->SetIntensity(call_data->value, 1);	// set intensity
	cwin->WheelColourChanging();
	// done changing our colour wheel
	cwin->WheelColourChanged();
	// update the colour wheel now if it wasn't done dynamically
	cwheel->AnimateDone();
}

void PColourWindow::ScaleDragProc(Widget w, PColourWindow *cwin, XmScaleCallbackStruct *call_data)
{
	PColourWheel *cwheel = (PColourWheel *)cwin->mImage;
	// only update colour wheel for fast animation
	cwheel->SetIntensity(call_data->value, 1);
	// the wheel colour is changing -- do required animation
	cwin->WheelColourChanging();
}

void PColourWindow::OkProc(Widget w, PColourWindow *cwin, caddr_t call_data)
{
	TextProc(NULL, cwin, NULL);	// do this in case text was changed
	cwin->mColourPicker->ApplyCurrentColours();
	delete(cwin);
}

void PColourWindow::ApplyProc(Widget w, PColourWindow *cwin, caddr_t call_data)
{
	TextProc(NULL, cwin, NULL);	// do this in case text was changed
	cwin->mColourPicker->ApplyCurrentColours();
}

// revert to last applied colours
void PColourWindow::RevertProc(Widget w, PColourWindow *cwin, caddr_t call_data)
{
	if (cwin->mColourPicker->RevertColours()) {
		cwin->PickerColourChanged();
	}
}

void PColourWindow::CancelProc(Widget w, PColourWindow *cwin, caddr_t call_data)
{
	if (cwin->mColourPicker->RevertColours()) {
		cwin->PickerColourChanged();
	}
	delete(cwin);
}

void PColourWindow::TextProc(Widget w, PColourWindow *cwin, caddr_t call_data)
{
	int	col3[3];
	int oldCol[3];
	
	for (int i=0; i<3; ++i) {
		char *str = XmTextGetString(cwin->mText[i]);
		col3[i] = atoi(str);
		if (col3[i] < 0) col3[i] = 0;
		else if (col3[i] > 255) col3[i] = 255;
		XtFree(str);	// must free the string
	}
	cwin->mColourWheel->GetColourRGB(oldCol);
	if (memcmp(oldCol, col3, sizeof(col3))) {
		cwin->mColourWheel->SetColourRGB(col3);
		cwin->mColourPicker->SetColourRGB(col3);
		cwin->UpdateSliderPosition();
	}
	cwin->UpdateText();		// update the text in case an entry wasn't in standard format
}

