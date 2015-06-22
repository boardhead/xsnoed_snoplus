#include <stdio.h>
#include <stdlib.h>
#include <X11/StringDefs.h>
#include <Xm/DrawingA.h>
#include "PImageCanvas.h"
#include "PImageWindow.h"
#include "PHitInfoWindow.h"
#include "PResourceManager.h"
#include "PDrawXPixmap.h"
#include "PDrawPostscriptFile.h"
#include "PMenu.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"

const short	kPrintScaling		= 10;	// coordinate scaling for printed images
const short kLabelClickMargin	= 8;


//----------------------------------------------------------------------------------------------
// PImageCanvas constructor
//
PImageCanvas::PImageCanvas(PImageWindow *owner, Widget canvas, EventMask eventMask)
{
	ImageData	*data = owner->GetData();
	
	mDpy			= data->display;
	mOwner			= owner;
	mWidth  		= 0;
	mHeight 		= 0;
	mCanvasWidth	= 0;
	mCanvasHeight	= 0;
	mDrawable 		= 0;
	mCanvas 		= NULL;
	mEventMask 		= eventMask;
	mDrawLabel		= 1;
	mAllowLabel     = 1;
	mLabelText		= NULL;
	mLabelHeight	= 0;
	mDirty			= kDirtyPix;

	SetCanvas(canvas);
	
	owner->SetImage(this);
	
	data->mSpeaker->AddListener(this);
}

PImageCanvas::~PImageCanvas()
{
	if (mDrawable) delete mDrawable;
	
	if (mCanvas) {
		XtRemoveCallback(mCanvas, XmNexposeCallback, (XtCallbackProc)CanvasExposeProc, this);
		XtRemoveCallback(mCanvas, XmNresizeCallback, (XtCallbackProc)CanvasResizeProc, this);
		
		if (mEventMask) {
			XtRemoveEventHandler(mCanvas, mEventMask, FALSE, (XtEventHandler)CanvasMotionProc, this);
		}
	}
}

void PImageCanvas::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageLabelChanged:
			if (mDrawLabel) {
				SetDirty();
			}
			break;
	}
}
	

// SetDirty - mark the image as needing redrawing
void PImageCanvas::SetDirty(int flag)
{
	mDirty |= flag;				// set our dirty flags
	mOwner->SetDirty(flag);		// set our owner's dirty flag
}

/* must be called AFTER widget is realized */
void PImageCanvas::SetCursor(int type)
{
	/* define the cursor for our image */
	XDefineCursor(mDpy, XtWindow(mCanvas), PResourceManager::sResource.cursor[type]);
}

void PImageCanvas::SetCursorForPos(int x, int y)
{
	if (IsInLabel(x,y)) {
		if (IsLabelOn()) {
			SetCursor(CURSOR_ARROW_DOWN);
		} else {
			SetCursor(CURSOR_ARROW_UP);
		}
	} else {
		XUndefineCursor(mDpy, XtWindow(mCanvas));
	}
}

void PImageCanvas::SetCanvas(Widget canvas)
{
	mCanvas = canvas;
	
	if (canvas) {
		mDpy = XtDisplay(canvas);
		
		if (mDrawable) delete mDrawable;
		mDrawable = new PDrawXPixmap(mDpy, mOwner->GetData()->gc,
									 DefaultDepthOfScreen(XtScreen(canvas)), canvas);
		
		XtAddCallback(canvas, XmNexposeCallback, (XtCallbackProc)CanvasExposeProc, this);
		XtAddCallback(canvas, XmNresizeCallback, (XtCallbackProc)CanvasResizeProc, this);
		
		if (mEventMask) {
			XtAddEventHandler(canvas, mEventMask, FALSE, (XtEventHandler)CanvasMotionProc, this);
		}
	}
}

// GetCanvasSize - get size of our canvas
// - returns non-zero (width of canvas) if canvas is realized
int PImageCanvas::GetCanvasSize()
{
	// return zero if our widget isn't available or isn't realized
	if (!mCanvas || !XtIsRealized(mCanvas)) return(0);

	// get the current size of the canvas
	Arg			wargs[10];
	Dimension	width,height;
	XtSetArg(wargs[0],XtNwidth, &width);
	XtSetArg(wargs[1],XtNheight,&height);
	XtGetValues(mCanvas,wargs,2);

	if (mCanvasWidth!=width || mCanvasHeight!=height) {
		ImageData	*data = mOwner->GetData();
		mCanvasWidth = mWidth = width;
		mCanvasHeight = mHeight = height;
		if (data->show_label) {
			mHeight -= GetScaling() * mLabelHeight;
			// handle case where mHeight is negative (remember, this is unsigned math)
			if (mHeight > height) mHeight = 0;
		}
		mDirty |= kDirtyPix;	// flag indicates pixmap requires redrawing
		Resize();				// call this whenever the image size changes
	}
	return((int)width);
}

void PImageCanvas::CanvasExposeProc(Widget w, PImageCanvas *anImage, XmDrawingAreaCallbackStruct *call_data)
{
	XExposeEvent  *	event = (XExposeEvent *)call_data->event;

	if (!anImage->mCanvasWidth) {
		anImage->GetCanvasSize();
	}
	// if pixmap is available and doesn't require drawing, copy the exposed area to screen
	if (!anImage->IsDirty() &&
		 anImage->mDrawable->CopyArea(event->x,event->y,event->width,event->height,XtWindow(anImage->mCanvas)))
	{
		if (event->count == 0) {
			// call this routine after last copy to screen
			anImage->AfterDrawing();
		}
	} else if (event->count == 0) {	// only draw on last expose event
		// pixmap is dirty or unavailable -- draw the image from scratch
		anImage->Draw();
	}
}

void PImageCanvas::CanvasResizeProc(Widget w, PImageCanvas *anImage, XmDrawingAreaCallbackStruct *call_data)
{
	int wasDrawn = (anImage->mCanvasWidth != 0);

	if (anImage->GetCanvasSize()) {
		if (wasDrawn) {
			// clear the canvas on the screen until we can redraw it
			XClearArea(anImage->mDpy,XtWindow(w),0,0,0,0,TRUE);
		}
	}
}

void PImageCanvas::CanvasMotionProc(Widget w, PImageCanvas *anImage, XEvent *event)
{
	// are we waiting for a window to be selected?
	if (PImageWindow::IsPendingRaise()) {
		// must look for button presses too, in case the window manager
		// doesn't raise the window on a button click
		if (event->type == ButtonPress) {
			anImage->mOwner->WasRaised();
		}
	} else {
		// handle events normally
		anImage->HandleEvents(event);
	}
}

// Print - print image to postscript file
int PImageCanvas::Print(char *filename, int flags)
{
	int	printOK = 0;
	
	PDrawPostscriptFile	drawable(filename, (flags & kPrintLandscape));
	
	PDrawable *oldDrawable = mDrawable;
	int oldWidth = mWidth;
	int oldHeight = mHeight;
	int oldCanvasWidth = mCanvasWidth;
	int oldCanvasHeight = mCanvasHeight;
	
	mDrawable = &drawable;
	
	// change scaling to improve printed resolution
	drawable.SetScaling(kPrintScaling);
	mWidth *= kPrintScaling;
	mHeight *= kPrintScaling;
	mCanvasWidth *= kPrintScaling;
	mCanvasHeight *= kPrintScaling;
	Resize();		// call this whenever the image size changes
	
	// draw image to postscript file
	if (drawable.BeginDrawing(mCanvasWidth,mCanvasHeight)) {
		Prepare();
		DrawSelf();
		drawable.EndDrawing();
		printOK = 1;
	}
	
	// restore original settings
	mDrawable = oldDrawable;
	mWidth = oldWidth;
	mHeight = oldHeight;
	mCanvasWidth = oldCanvasWidth;
	mCanvasHeight = oldCanvasHeight;
	Resize();		// call this whenever the image size changes
	
	return(printOK);
}

//---------------------------------------------------------------------------------------
// Draw
//
void PImageCanvas::Draw()
{
	// get our canvas dimensions if not done yet
	if (!mCanvasWidth) {
		if (!GetCanvasSize()) return;	// return if widget not realized
	}
	if (mDrawable->BeginDrawing(mCanvasWidth, mCanvasHeight)) {
		Prepare();
		// set dirty pixmap flag too if we have arrived here without a pixmap
		if (!mDrawable->HasPixmap()) {
			mDirty |= kDirtyPix;
		}
		DrawSelf();
		mDrawable->EndDrawing();
		// copy the image to the screen
		mDrawable->CopyArea(0,0,mCanvasWidth,mCanvasHeight,XtWindow(mCanvas));
		// reset all dirty flags since we have just successfully drawn ourself
		mDirty = 0;
		AfterDrawing();		// call this after any drawing to screen
	}
}

//---------------------------------------------------------------------------------------
// DrawLabel
//
void PImageCanvas::DrawLabel(int x,int y,ETextAlign_q align)
{
	if (mLabelText && mLabelHeight) {
		SetForeground(TEXT_COL);
		y -= GetScaling() * mLabelHeight;
		for (TextSpec *ts=mLabelText; ts->string; ++ts) {
			SetFont(ts->font);
			y += GetScaling() * ts->font->ascent;
			DrawString(x, y, ts->string, align);
			y += GetScaling() * ts->font->descent;
		}
	}
}

//---------------------------------------------------------------------------------------
// Prepare - prepare to draw into canvas
//
void PImageCanvas::Prepare()
{
	// resize drawing area according to current label size
	ImageData *data = mOwner->GetData();
	int newHeight;
	if (mDrawLabel && data->show_label) {
		XSnoedWindow *mainWindow = mOwner->GetData()->mMainWindow;
		mLabelText = mainWindow->GetLabelText();
		newHeight = mainWindow->GetLabelHeight();
	} else {
		mLabelText = NULL;
		newHeight = 0;
	}
	// get current number of lines in label
	if (mLabelHeight != newHeight) {
		// calculate new image height
		mHeight = mCanvasHeight - GetScaling() * newHeight;
		if (mHeight > mCanvasHeight) mHeight = 0;
		mLabelHeight = newHeight;
		Resize();	// call this whenever the image size changes
	}
}


//---------------------------------------------------------------------------------------
// DrawSelf
//
void PImageCanvas::DrawSelf()
{
	// clear the canvas with the background colour */
	SetForeground(BKG_COL);
	FillRectangle(0, 0, mCanvasWidth, mCanvasHeight);
	
	if (mLabelText) {
		DrawLabel(mCanvasWidth/2, mCanvasHeight-2*GetScaling(), kTextAlignBottomCenter);
	}
}

int PImageCanvas::IsInLabel(int x, int y)
{
    if (mAllowLabel) {
	    return(mOwner->GetData()->show_label && (y > mCanvasHeight - mLabelHeight - kLabelClickMargin));
    } else {
        return(0);
    }
}

void PImageCanvas::ShowLabel(int on)
{
	if (mDrawLabel != on) {
		mDrawLabel = on;
		if (mCanvas && XtIsRealized(mCanvas)) {
			Prepare();	// do this now so any subsequent cursor changes will use new configuration
			SetDirty();	// redraw ourself
		}
	}
}

void PImageCanvas::AllowLabel(int on)
{
    mAllowLabel = on;
    if (!on) ShowLabel(on);
}

// utility to create image canvas plus specified scrollbars
void PImageCanvas::CreateCanvas(char *name, int scrollBarMask)
{
	int				n;
	Arg				wargs[16];
	unsigned char	attach;
	Widget			topWidget;
	
	if (mOwner->GetMenu()) {
		topWidget = mOwner->GetMenu()->GetWidget();
	} else {
		topWidget = 0;
	}
	// get right attachment of top widget
	if (topWidget) {
		n = 0;
		XtSetArg(wargs[n], XmNrightAttachment, &attach);  ++n;
		XtGetValues(topWidget, wargs, n);
	}
	
	if (scrollBarMask & kScrollLeftMask) {
		n = 0;
		if (topWidget) {
			XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_WIDGET);  ++n;
			XtSetArg(wargs[n], XmNtopWidget, topWidget); ++n;
		} else {
			XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM);  ++n;
		}
		XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM);  ++n;
		XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);  ++n;
		XtSetArg(wargs[n], XmNwidth, 15); ++n;
		XtSetArg(wargs[n], XmNorientation, XmVERTICAL);  ++n;
		mOwner->NewScrollBar(kScrollLeft,"xsnoedScroll1",wargs,n);
	}
	if (scrollBarMask & kScrollRightMask) {
		n = 0;
		if (topWidget && attach==XmATTACH_FORM) {
			XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_WIDGET);  ++n;
			XtSetArg(wargs[n], XmNtopWidget, topWidget); ++n;
		} else {
			XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM);  ++n;
		}
		XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);  ++n;
		XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM);  ++n;
		XtSetArg(wargs[n], XmNwidth, 15); ++n;
		XtSetArg(wargs[n], XmNorientation, XmVERTICAL);  ++n;
		mOwner->NewScrollBar(kScrollRight,"xsnoedScroll2",wargs,n);
	}
	if (scrollBarMask & kScrollBottomMask) {
		n = 0;
		if (scrollBarMask & kScrollLeftMask) {
			XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET);  ++n;
			XtSetArg(wargs[n], XmNleftWidget, mOwner->GetScrollBar(kScrollLeft)->GetWidget()); ++n;
		} else {
			XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM);  ++n;
		}
		if (scrollBarMask & kScrollRightMask) {
			XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_WIDGET);  ++n;
			XtSetArg(wargs[n], XmNrightWidget, mOwner->GetScrollBar(kScrollRight)->GetWidget()); ++n;
		} else {
			XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM);  ++n;
		}
		XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);  ++n;
		XtSetArg(wargs[n], XmNheight, 15); ++n;
		XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL);  ++n;
		mOwner->NewScrollBar(kScrollBottom,"xsnoedScroll3",wargs,n);
	}
	
	// finally create canvas
	n = 0;
	if (topWidget && attach==XmATTACH_FORM) {
		XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_WIDGET);  ++n;
		XtSetArg(wargs[n], XmNtopWidget, topWidget);  ++n;
	} else {
		// attach top to form if no top widget, or if top widget doesn't span form
		XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_FORM);  ++n;
	}
	if (scrollBarMask & kScrollLeftMask) {
		XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET);  ++n;
		XtSetArg(wargs[n], XmNleftWidget, mOwner->GetScrollBar(kScrollLeft)->GetWidget()); ++n;
	} else {
		XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM);  ++n;
	}
	if (scrollBarMask & kScrollBottomMask) {
		XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_WIDGET);  ++n;
		XtSetArg(wargs[n], XmNbottomWidget, mOwner->GetScrollBar(kScrollBottom)->GetWidget()); ++n;
	} else {
		XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);  ++n;
	}
	if (scrollBarMask & kScrollRightMask) {
		XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_WIDGET);  ++n;
		XtSetArg(wargs[n], XmNrightWidget, mOwner->GetScrollBar(kScrollRight)->GetWidget()); ++n;
	} else {
		XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM);  ++n;
	}
	Widget canvas = XtCreateManagedWidget(name, xmDrawingAreaWidgetClass, mOwner->GetMainPane(), wargs, n);
	
	SetCanvas(canvas);

	/* set up this as the default scrollbar handler */
	mOwner->SetScrollHandler(this);
	SetScrolls();
}

