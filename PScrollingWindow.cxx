#include "PScrollingWindow.h"

PScrollingWindow::PScrollingWindow(ImageData *data)
				: PWindow(data)
{
	for (int i=0; i<kNumScrollBars; ++i) {
		mScrollBar[i] = NULL;
	}
}

PScrollingWindow::~PScrollingWindow()
{
	for (int i=0; i<kNumScrollBars; ++i) {
		delete mScrollBar[i];
	}
}


// --------------------------------------------------------------------------------
// New ScrollBar
//
// Create new scroll bar in window
//
void PScrollingWindow::NewScrollBar(EScrollBar bar, char *name, Arg *wargs, int n)
{
	delete mScrollBar[bar];
	
	mScrollBar[bar] = new PScrollBar(GetMainPane(),bar,name,wargs,n);
}

void PScrollingWindow::SetScrollValue(EScrollBar bar, int value, int do_callback)
{
	Arg		wargs[1];
	
	if (!GetScrollBar(bar)) return;
	
	if (value < 0) value = 0;
	if (value > kScrollMax) value = kScrollMax;
	
	XtSetArg(wargs[0], XmNvalue, value);
	XtSetValues(GetScrollBar(bar)->GetWidget(),wargs,1);

	if (do_callback) {
		GetScrollBar(bar)->GetHandler()->ScrollValueChanged(bar,value);
	}
}


int	PScrollingWindow::GetScrollValue(EScrollBar bar)
{
	Arg		wargs[1];
	int		value;
	
	if (!GetScrollBar(bar)) return(0);
	
	XtSetArg(wargs[0], XmNvalue, &value);
	XtGetValues(GetScrollBar(bar)->GetWidget(),wargs,1);
	
	return(value);
}

// SetScrollHandler - set handler for existing scrollbars
void PScrollingWindow::SetScrollHandler(PScrollHandler *hand)
{
	for (int i=0; i<kNumScrollBars; ++i) {
		if (mScrollBar[i]) {
			mScrollBar[i]->SetHandler(hand);
		}
	}
}



