#include <stdio.h>
#include "PImageWindow.h"
#include "PImageCanvas.h"
#include "PResourceManager.h"
#include "PNotifyRaised.h"
#include "xsnoed.h"

char		  *	PImageWindow::sImageWindowClass = "PImageWindow";
PNotifyRaised *	PImageWindow::sNotifyRaised 	= 0;


//------------------------------------------------------------------------------------
// PImageWindow constructor
//
PImageWindow::PImageWindow(ImageData *data)
			: PScrollingWindow(data)
{
	Initialize();
}

PImageWindow::PImageWindow(ImageData *data, Widget shell, Widget mainPane)
			: PScrollingWindow(data)
{
	Initialize();
	SetShell(shell);
	SetMainPane(mainPane);
}

void PImageWindow::Initialize()
{
	mImage = NULL;
	mPrintable = 1;		// by default, image windows are printable
}

PImageWindow::~PImageWindow()
{
	if (mImage) {
		delete mImage;
	}
}

void PImageWindow::SetShell(Widget w)
{
	PWindow::SetShell(w);
	
	if (w) {
		/* handle circulate events for printing */
// THIS IS WHAT CAUSED THE ENDLESS RESIZE PROBLEM!!!! - PH 01/13/03
//		XtAddEventHandler(w, StructureNotifyMask|SubstructureNotifyMask, FALSE, (XtEventHandler)CirculateWindProc, this);
		XtAddRawEventHandler(w, StructureNotifyMask|SubstructureNotifyMask, FALSE, (XtEventHandler)CirculateWindProc, this);
	}
}

void PImageWindow::Show()
{
	SetScrolls();		// make sure our scrollbars are consistent
	
	PWindow::Show();	// let base class show the window
	
	/* define the cursor for our image */
	/* (must be done AFTER window is realized) */
	mImage->SetCursor(CURSOR_XHAIR);
}

void PImageWindow::UpdateSelf()
{
	if (mImage->IsDirty()) {
		mImage->Draw();
	}
}

void PImageWindow::SetScrolls()
{
	if (mImage) mImage->SetScrolls();
}

void PImageWindow::SetToHome(int n)
{
	if (mImage) {
		mImage->SetToHome(n);	// tell the image to move to the home position
		SetScrolls();			// make scrollbars consistent with this position
		SetDirty();				// inform of change in image data
	}
}

void PImageWindow::WasRaised()
{
	// send notification to sNotifyRaised that this window was raised
	if (sNotifyRaised && mPrintable) {
		// inform interested window of the window that was raised
		sNotifyRaised->NotifyRaised(this);
		// reset the flag
		sNotifyRaised = NULL;
	}
}

void PImageWindow::CirculateWindProc(Widget w, PImageWindow *aWind, XEvent *event)
{
	if (event->type == ConfigureNotify &&
		event->xconfigure.window == XtWindow(aWind->GetShell()))
	{
		aWind->WasRaised();
	}
}
	


