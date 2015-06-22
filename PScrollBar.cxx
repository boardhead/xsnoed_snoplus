#include <Xm/ScrollBar.h>
#include "PScrollBar.h"
#include "PScrollingWindow.h"

const int	kScrollIncrement	= 100;

// scrollBar callback
static void scrollBarProc(Widget w, PScrollBar *aScroll, XmScrollBarCallbackStruct *call_data)
{
	if (aScroll->GetHandler()) {
		aScroll->GetHandler()->ScrollValueChanged(aScroll->GetType(), call_data->value);
	}
}

// ------------------------------------------------------------------------------------------------------------
// PScrollBar constructor
//
PScrollBar::PScrollBar(Widget container, EScrollBar bar, char *name, Arg *aWargs, int an, PScrollHandler *handler)
{
	int		n;
	Arg		wargs[10];
	
	mType = bar;
	mHandler = handler;
	mScrollWidget = XtCreateManagedWidget(name, xmScrollBarWidgetClass, container, aWargs, an);
	
	// override scrollbar range to set all scrollbars to 0 -> kScrollMax
	n = 0;
	XtSetArg(wargs[n], XmNminimum, 0); ++n;
	XtSetArg(wargs[n], XmNmaximum, kScrollMax + kSliderSize); ++n;
	XtSetArg(wargs[n], XmNsliderSize, kSliderSize); ++n;
	XtSetArg(wargs[n], XmNpageIncrement, kSliderSize); ++n;
	XtSetArg(wargs[n], XmNincrement, kScrollIncrement); ++n;
	XtSetValues(mScrollWidget, wargs, n);

	XtAddCallback(mScrollWidget, XmNvalueChangedCallback, (XtCallbackProc)scrollBarProc, this);
	XtAddCallback(mScrollWidget, XmNdragCallback, (XtCallbackProc)scrollBarProc, this);
}

PScrollBar::~PScrollBar()
{
}
