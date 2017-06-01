#include <stdlib.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include "PHistImage.h"
#include "PResourceManager.h"
#include "PScale.h"
#include "PImageWindow.h"
#include "ImageData.h"
#include "PUtils.h"

#define HIST_MARGIN_BOTTOM			(25 * GetScaling())
#define HIST_MARGIN_LEFT			(48 * GetScaling())
#define HIST_MARGIN_TOP				(14 * GetScaling())
#define HIST_MARGIN_RIGHT			(24 * GetScaling())
#define HIST_LABEL_Y				(-10 * GetScaling())

const long MIN_LONG = -1 - 0x7fffffffL;
const long MAX_LONG = 0x7fffffffL;

//---------------------------------------------------------------------------------------
// PHistImage constructor
//
PHistImage::PHistImage(PImageWindow *owner, Widget canvas, int createCanvas)
		  : PImageCanvas(owner,canvas,PointerMotionMask|ButtonPressMask|ButtonReleaseMask)
{
	mXScale			= NULL;
	mYScale			= NULL;
	mIsLog			= 0;
	mHistogram		= NULL;
	mOverlay		= NULL;
	mOverlayScale	= 1.0;
	mLabel			= NULL;
	mNumBins		= 0;
	mNumCols		= 0;
	mHistCols		= NULL;
	mYMin           = 0;
	mYMax			= 10;
	mXMin			= 0;
	mXMax			= 10;
	mXScaleFlag		= INTEGER_SCALE;
	mStyle			= kHistStyleBars;
	mGrabFlag		= 0;
	mXMinMin		= 0;
	mXMaxMax		= 10;
	mXMinRng		= 1;
	mOverlayCol     = SCALE_COL3;
	mPlotCol        = TEXT_COL;
	mFixedBins      = 0;
	mCalcObj        = NULL;

	if (!canvas && createCanvas) {
		CreateCanvas("ehCanvas");
	}
}

PHistImage::~PHistImage()
{
	delete mXScale;
	delete mYScale;
	
	// delete histogram, overlay, label string, and colours
	delete [] mHistogram;
	delete [] mOverlay;
	delete [] mLabel;
	delete [] mHistCols;
	
	ImageData *data = mOwner->GetData();
	
	if (data->mScaleHist == this) {
		data->mScaleHist = NULL;
		if (data->mWindow[SCALE_WINDOW]) {
			delete data->mWindow[SCALE_WINDOW];
		}
	}
}

// CreateData - create histogram data (can be used to delete data if numbins=0)
void PHistImage::CreateData(int numbins, int twoD)
{
    int numPix;
    if (twoD) {
        numPix = mHeight + 1 - HIST_MARGIN_BOTTOM - HIST_MARGIN_TOP;
        if (numPix < 0) numPix = 0;
    } else {
        numPix = 0;
    }
	if (numbins != mNumBins || numPix != mNumPix) {
		delete [] mHistogram;
		delete [] mOverlay;
		mHistogram = NULL;  // (mOverlay is reset below)
	}
    if (numbins && !mHistogram) {
        mHistogram = new long[numbins * (numPix ? numPix : 1)];
        // resize overlay if it was created before
        if (mOverlay && numbins!=mNumBins) mOverlay = new long[numbins];
        mNumBins = numbins;
        mNumPix = numPix;
        mNumTraces = 0;
    } else {
        mOverlay = NULL;
    }
    if (!mHistogram) mNumPix = 0;
}

// CreateOverlay - create overlay data the same size as the histogram array
void PHistImage::CreateOverlay(int numbins)
{
	if (numbins != mNumBins) {
		delete [] mHistogram;
		delete [] mOverlay;
		mOverlay = NULL;
		mNumPix = 0;
	}
    if (numbins && !mOverlay) {
        mOverlay = new long[numbins];
        // resize histogram if it was created before
        if (mHistogram && numbins!=mNumBins) mHistogram = new long[numbins];
        mNumBins = numbins;
    } else {
        mHistogram = NULL;
    }
}

void PHistImage::SetIntegerXScale(int is_int)
{
	if (is_int) {
		mXScaleFlag |= INTEGER_SCALE;
	} else {
		mXScaleFlag &= ~INTEGER_SCALE;
	}
	if (mXScale) {
		mXScale->SetInteger(is_int);
	}
}

void PHistImage::CreateScaleWindow()
{
	int			n;
	Position	xpos, ypos;
	Arg			wargs[20];
	char		buff[256], *pt;
	Widget		window, but, w;
	ImageData	*data = mOwner->GetData();
	
	n = 0;
	XtSetArg(wargs[n], XmNx, &xpos); ++n;
	XtSetArg(wargs[n], XmNy, &ypos); ++n;
	if (data->mWindow[SCALE_WINDOW]) {
		if (data->mScaleHist == this) {
			data->mWindow[SCALE_WINDOW]->Raise();	// raise to top
			return;	// already opened scale for this hist
		}
		XtGetValues(data->mWindow[SCALE_WINDOW]->GetShell(), wargs, n);
		// destroy old scale window
		delete data->mWindow[SCALE_WINDOW];
	} else {
  		// center the scale popup on the histogram
		XtGetValues(mOwner->GetShell(), wargs, n);
		xpos += 50;
		ypos += 40;
	}
	data->mScaleHist = this;

	pt = mOwner->GetTitle();
  	sprintf(buff,"%s Scales",pt);

  	n = 0;
	XtSetArg(wargs[n], XmNtitle, buff); ++n;
	XtSetArg(wargs[n], XmNx, xpos); ++n;
	XtSetArg(wargs[n], XmNy, ypos); ++n;
	XtSetArg(wargs[n], XmNminWidth, 456); ++n;
	XtSetArg(wargs[n], XmNminHeight, 155); ++n;
	window = PWindow::CreateShell("scalePop",data->toplevel,wargs,n);
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,window,NULL,0);
	data->mWindow[SCALE_WINDOW] = new PWindow(data,window,w);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 18); ++n;
	XtCreateManagedWidget("X Scale Min:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 115); ++n;
	XtSetArg(wargs[n], XmNy, 13); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	sp_min = XtCreateManagedWidget("ScaleMin",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(sp_min,XmNactivateCallback,(XtCallbackProc)ScaleOKProc,this);
	sprintf(buff,"%g",mXMin);
	setTextString(sp_min,buff);

	n = 0;
	XtSetArg(wargs[n], XmNx, 225); ++n;
	XtSetArg(wargs[n], XmNy, 18); ++n;
	XtCreateManagedWidget("Max:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 270); ++n;
	XtSetArg(wargs[n], XmNy, 13); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	sp_max = XtCreateManagedWidget("ScaleMax",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(sp_max,XmNactivateCallback,(XtCallbackProc)ScaleOKProc,this);
	sprintf(buff,"%g",mXMax);
	setTextString(sp_max,buff);

	n = 0;
	XtSetArg(wargs[n], XmNx, 380); ++n;
	XtSetArg(wargs[n], XmNy, 13); ++n;
	XtSetArg(wargs[n], XmNwidth, 60); ++n;
	but = XtCreateManagedWidget("Full",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)ScaleFullProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 53); ++n;
	XtCreateManagedWidget("Y Scale Min:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 115); ++n;
	XtSetArg(wargs[n], XmNy, 48); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	sp_ymin = XtCreateManagedWidget("YScaleMin",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(sp_ymin,XmNactivateCallback,(XtCallbackProc)ScaleOKProc,this);
	sprintf(buff,"%ld",mYMin);
	setTextString(sp_ymin,buff);

	n = 0;
	XtSetArg(wargs[n], XmNx, 225); ++n;
	XtSetArg(wargs[n], XmNy, 53); ++n;
	XtCreateManagedWidget("Max:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 270); ++n;
	XtSetArg(wargs[n], XmNy, 48); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	sp_ymax = XtCreateManagedWidget("YScaleMax",xmTextWidgetClass,w,wargs,n);
	XtAddCallback(sp_ymax,XmNactivateCallback,(XtCallbackProc)ScaleOKProc,this);
	sprintf(buff,"%ld",mYMax);
	setTextString(sp_ymax,buff);

	n = 0;
	XtSetArg(wargs[n], XmNx, 380); ++n;
	XtSetArg(wargs[n], XmNy, 48); ++n;
	XtSetArg(wargs[n], XmNwidth, 60); ++n;
	but = XtCreateManagedWidget("Auto",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)ScaleAutoProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 370); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 18); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 18); ++n;
	but = XtCreateManagedWidget("OK",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)ScaleOKProc, this);

	n = 0;
	XtSetArg(wargs[n], XmNx, 195); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 7); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 7); ++n;
	but = XtCreateManagedWidget("Apply",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)ApplyProc, this);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtSetArg(wargs[n], XmNmarginLeft, 5); ++n;
	XtSetArg(wargs[n], XmNmarginRight, 5); ++n;
	but = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)CancelProc, window);
    	
	n = 0;
	XtSetArg(wargs[n], XmNx, 140); ++n;
	XtSetArg(wargs[n], XmNy, 83); ++n;
	XtSetArg(wargs[n], XmNset, mIsLog); ++n;
	sp_log = XtCreateManagedWidget("Logarithmic Y Scale",xmToggleButtonWidgetClass,w,wargs,n);
//	XtAddCallback(sp_log,XmNvalueChangedCallback,(XtCallbackProc)ScaleOKProc, this);

	data->mWindow[SCALE_WINDOW]->Show();		// show the window
}

void PHistImage::SetScaleLimits(float min, float max, float min_rng)
{
	mXMinMin = min;
	mXMaxMax = max;
	mXMinRng = min_rng;
}

void PHistImage::SetLabel(char *str)
{
	if (mLabel) delete [] mLabel;
	int len = strlen(str);
	mLabel = new char[len+1];
	if (mLabel) {
		strcpy(mLabel, str);
	}
}

void PHistImage::UpdateScaleInfo()
{
	char		buff[128];
	Arg			wargs[1];
	ImageData	*data = mOwner->GetData();
	
	if (!data->mWindow[SCALE_WINDOW]) return;
	
	if (data->mScaleHist == this) {
	
		sprintf(buff,"%g",mXMin);
		setTextString(sp_min,buff);
	
		sprintf(buff,"%g",mXMax);
		setTextString(sp_max,buff);
	
		sprintf(buff,"%ld",mYMin);
		setTextString(sp_ymin,buff);
	
		sprintf(buff,"%ld",mYMax);
		setTextString(sp_ymax,buff);
	
		XtSetArg(wargs[0], XmNset, mIsLog);
		XtSetValues(sp_log,wargs,1);
	}
}

void PHistImage::SetLog(int on)
{
	if (mIsLog != on) {
		mIsLog = on;
		// must re-create y scale if log changes
		delete mYScale;
		mYScale = NULL;
	}
}

void PHistImage::SetCursorForPos(int x, int y)
{
	if (IsInLabel(x, y)) {
		PImageCanvas::SetCursorForPos(x,y);
	} else if (y >= mHeight - HIST_MARGIN_BOTTOM) {
		SetCursor(CURSOR_MOVE_H);
	} else if (x <= HIST_MARGIN_LEFT) {
		SetCursor(CURSOR_MOVE_V);
	} else {
		SetCursor(CURSOR_MOVE_4);
	}
}

void PHistImage::HandleEvents(XEvent *event)
{
	int				x1,y1,x2,y2,dx,dy,doUpdate;
	double			diff, newMax, newMin, val;
	static int		posX, posY;
	static double	grabX, grabY;
	static int		didDrag;
	static int		wasChanged;
	static int		isExpanding, isExpandingY;
	static double	scaleMin, scaleMax;
	static long     yscaleMin, yscaleMax;
	static int		lastGrabFlag;
	static int		x0, y0;					// X,Y scale pixel origin
	static double	scaleMin0, scaleMax0;	// X scale min/max at start of grab
	static long     yscaleMin0, yscaleMax0; // Y scale min/max at start of grab

	// get the histogram boundaries
	if (!mXScale || !mYScale) return;
	
	y1 = HIST_MARGIN_TOP;
	y2 = mHeight - HIST_MARGIN_BOTTOM;
	x1 = HIST_MARGIN_LEFT;
	x2 = mWidth - HIST_MARGIN_RIGHT;
	
	switch (event->type) {
		case ButtonPress:
			if (!(mGrabFlag & GRABS_ACTIVE)) {
				lastGrabFlag = mGrabFlag;
				wasChanged = 0;
				didDrag = 0;
				posX = event->xbutton.x;
				posY = event->xbutton.y;
				if (IsInLabel(posX, posY)) {
					ShowLabel(!IsLabelOn());
					SetCursorForPos(posX, posY);
					break;
				} else if (posY >= mHeight - HIST_MARGIN_BOTTOM) {
					mGrabFlag |= GRAB_X_ACTIVE | GRAB_X;	// grab X only
				} else if (posX <= HIST_MARGIN_LEFT) {
					mGrabFlag |= GRAB_Y_ACTIVE | GRAB_Y;	// grab Y only
				} else {
					mGrabFlag |= GRABS_ACTIVE | GRAB_X | GRAB_Y;	// grab both
				}
				SetCursorForPos(posX, posY);
				XGrabPointer(mDpy, XtWindow(mCanvas),0,
							 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
							 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
				if (mGrabFlag & GRAB_X_ACTIVE) {
					/* get X grab location */
					grabX = mXScale->GetVal(posX);
					x0 = mXScale->GetPix(0);
					isExpanding = (mXScale->GetMinVal() == 0);
					// don't expand if we click right on the zero
					if (isExpanding && x0-posX<8 && x0-posX>-8) isExpanding = 0;
					// shift scale if we click using right mouse button (button3)
					if (isExpanding && event->xbutton.button==Button3) isExpanding = 0;
				}
				if (mGrabFlag & GRAB_Y_ACTIVE) {
					/* get Y grab location */
					grabY = mYScale->GetVal(posY);
					y0 = mYScale->GetPix(0);
					isExpandingY = 1;
					if (mIsLog) {
						/* want log of grab point for a log scale */
						grabY = log(grabY + 1);
					} else {
                        // don't expand if we click right on the zero
                        if (isExpandingY && y0-posY<8 && y0-posY>-8) isExpandingY = 0;
                        // shift scale if we click using right mouse button (button3)
                        if (isExpandingY && event->xbutton.button==Button3) isExpandingY = 0;
					}
				}
				/* save initial X scale range */
				scaleMin = scaleMin0 = mXScale->GetMinVal();
				scaleMax = scaleMax0 = mXScale->GetMaxVal();
				yscaleMin = yscaleMin0 = (long)mYScale->GetMinVal();
				yscaleMax = yscaleMax0 = (long)mYScale->GetMaxVal();
			}
			break;
		case ButtonRelease:
			if (mGrabFlag & GRABS_ACTIVE) {
				if (wasChanged) {
					// update the scale window
					UpdateScaleInfo();
					// inform derived classes that the scale has changed
					DoneGrab();
				} else if (!didDrag) {
					if ((mGrabFlag & GRABS_ACTIVE) == GRAB_X_ACTIVE ||
						(mGrabFlag & GRABS_ACTIVE) == GRAB_Y_ACTIVE)
					{
						mGrabFlag = lastGrabFlag;	// restore original grab flag
						CreateScaleWindow();
					} else if (mGrabFlag) {
						// reset the grab flags (allowing derived classes to rescale if necessary)
						ResetGrab(1);
					}
				}
				mGrabFlag &= ~GRABS_ACTIVE;
				XUngrabPointer(mDpy, CurrentTime);
			}
			break;
		case MotionNotify:
			if (mGrabFlag & GRABS_ACTIVE) {
				if (!didDrag) {
					dx = posX - event->xbutton.x;
					dy = posY - event->xbutton.y;
					// must surpass motion threshold before changing scale
					if (dx>-4 && dx<4 && dy>-4 && dy<4) break;
					didDrag = 1;
				}
				doUpdate = 0;
				if (mGrabFlag & GRAB_X_ACTIVE) {	// change X scale?
//					val = mXScale->GetVal(event->xbutton.x);
					SetScaleLimits();
					// expand scale if it starts at zero
					if (isExpanding) {
						if (event->xbutton.x > x0) {
							newMax = scaleMax0 * (posX-x0) / (event->xbutton.x-x0);
							if (newMax > mXMaxMax) {
								newMax = mXMaxMax;
							} else if (newMax < mXMinRng) {
								newMax = mXMinRng;
							}
						} else {
							newMax = mXMaxMax;
						}
						if (mXScaleFlag & INTEGER_SCALE) {
							// round to nearest integer for integer scales
							newMax = (newMax>=0 ? (long)(newMax+0.5) : (long)(newMax-0.5));
						}
						if (scaleMax != newMax) {
							scaleMax = newMax;
							DoGrab(scaleMin, scaleMax);
							doUpdate = 1;
						}
					} else {	// else slide scale
						diff = mXScale->GetRelVal(event->xbutton.x-posX);
						if (mXScaleFlag & INTEGER_SCALE) {
							// round off for integer scales
							diff = (diff>=0 ? (long)(diff + 0.5) : (long)(diff - 0.5));
						}
						// range check
						if (scaleMin0 - diff < mXMinMin) {
							diff = scaleMin0 - mXMinMin;
						} else if (scaleMax0 - diff > mXMaxMax) {
							diff = scaleMax0 - mXMaxMax;
						}
						newMax = scaleMax0 - diff;
						if (scaleMax != newMax) {
							scaleMax = newMax;
							scaleMin = scaleMin0 - diff;
							DoGrab(scaleMin, scaleMax);
							doUpdate = 1;
						}
					}
				}
				if (mGrabFlag & GRAB_Y_ACTIVE) {	// change Y scale?
					if (mIsLog) {
						// expand log scale
						if (event->xbutton.y < y2) {
							val = (y2-y1) * grabY / (event->xbutton.y-y2);
							if (val > 20) {
								newMax = MAX_LONG;
							} else {
								newMax = exp(val) + 1;
							}
						} else {
							newMax = MAX_LONG;
						}
						newMin = 0;
					} else if (isExpandingY) {
					    // is origin closer to scale minimum?
					    double newRng;
                        if ((y0 - event->xbutton.y) * grabY > 0) {
                            newRng = (yscaleMax0 - yscaleMin0) * (posY-y0) / (double)(event->xbutton.y-y0);
					    } else {
					        newRng = MAX_LONG - (double)MIN_LONG;
					    }
					    if (newRng < 10) newRng = 10;   // set minimum range

					    newMax = (y0 - y1) * newRng / (double)(y2 - y1);
					    if (newMax > MAX_LONG) {
					        newMax = MAX_LONG;
					        newRng = newMax * (y2 - y1) / (double)(y0 - y1);
					    }
					    newMin = (y0 - y2) * newRng / (y2 - y1);
					    if (newMin < MIN_LONG) {
					        newMin = MIN_LONG;
					        newRng = newMin * (y2 - y1) / (double)(y0 - y2);
					        newMax = (y0 - y1) * newRng / (double)(y2 - y1);
					    }
                        // round to nearest integer (y scale always integer)
                        newMin = (newMin>=0 ? (long)(newMin+0.5) : (long)(newMin-0.5));
                        newMax = (newMax>=0 ? (long)(newMax+0.5) : (long)(newMax-0.5));
	                } else { // else slide scale
						diff = mYScale->GetRelVal(posY-event->xbutton.y);
                        // round off for integer scales
                        diff = (diff>=0 ? (long)(diff + 0.5) : (long)(diff - 0.5));
						newMax = yscaleMax0 - diff;
                        newMin = (double)yscaleMin + (newMax - yscaleMax);
				    }
                    if (mYMax != newMax || mYMin != newMin) {
                        DoGrabY(newMin, newMax);
                        doUpdate = 1;
                    }
				}
				// redraw histogram if either scale changed
				if (doUpdate) {
					wasChanged = 1;
					SetDirty(kDirtyHistCalc);
				}
			} else {
				SetCursorForPos(event->xbutton.x, event->xbutton.y);
			}
			break;
	}
}

void PHistImage::DoGrab(float xmin, float xmax)
{
	mXMin = xmin;
	mXMax = xmax;
	CheckScaleRange();
}

void PHistImage::DoGrabY(double newMin, double newMax)
{
    // range check new scale
    if (newMax - newMin < 10) {
        newMax = newMin + 10;
    }
    if (newMax > MAX_LONG) {
        newMax = MAX_LONG;
        if (newMax - newMin < 10) {
            newMin = newMax - 10;
        }
    }
    if (newMin < MIN_LONG) {
        newMin = MIN_LONG;
        if (newMax - newMin < 10) {
            newMax = newMin + 10;
        }
    }
    // do update only if scale maximum was changed
    mYMin = (long)newMin;
    mYMax = (long)newMax;
}

/*
** Draw histogram
*/
void PHistImage::DrawSelf()
{
	int				i,j,n,x,y,dx,dy;
	int				lastx, lasty;
	int				x1,y1,x2,y2;
	long			counts;
	
#ifdef PRINT_DRAWS
	Printf("drawHist\n");
#endif

	MakeHistogram();
	SetHistogramLabel();

	PImageCanvas::DrawSelf();
	
	y1 = HIST_MARGIN_TOP;
	y2 = mHeight - HIST_MARGIN_BOTTOM;
	x1 = HIST_MARGIN_LEFT;
	x2 = mWidth - HIST_MARGIN_RIGHT;

	// start drawing
	SetForeground(TEXT_COL);
	SetFont(PResourceManager::sResource.hist_font);
	
	/* create scales if necessary */
	if (!mXScale) {
		mXScale = new PScale(mDrawable,GetFont(),x1,y2,x2+1,y2,mXScaleFlag);
		if (!mXScale) return;
	}
	if (!mYScale) {
		mYScale = new PScale(mDrawable,GetFont(),x1,y1,x1,y2,
									  INTEGER_SCALE | (mIsLog ? LOG_SCALE : 0));
		if (!mYScale) return;
	}
	/* set the scale ranges (also draws them) */
	mXScale->SetRng(GetScaleMin(),GetScaleMax());
	mYScale->SetRng(GetYMin(),GetYMax());

    /* calculate histogram data if necessary */
    if ((IsDirty() & kDirtyHistCalc) && mCalcObj) {
        mCalcObj->DoCalc(this);
    }

	if (mNumBins) {

		int nbin, noffset;
		
		if (mFixedBins) {
		    noffset = (long)GetScaleMin();
		    if (noffset < 0) noffset = 0;
		    nbin = (long)(GetScaleMax() + 0.5) - noffset;
		    if (nbin > mNumBins) nbin = mNumBins;
	    } else {
		    noffset = 0;
		    nbin = mNumBins;
		}
		dx = (x2-x1) / nbin - GetScaling();
		if (dx < GetScaling()) dx = GetScaling();			/* limit minimum bin size */
		else if (dx == GetScaling()) dx = 2*GetScaling();

        if (mStyle == kHistStyle2D) {

            // draw 2-dimensional histogram
            lastx = x1;
            int ncols = mOwner->GetData()->num_cols;
            SetForeground(FIRST_SCALE_COL);
            unsigned col = 0;
            XSegment **spp = new XSegment*[ncols];
            memset(spp, 0, ncols * sizeof(XSegment*));
            int *nseg = new int[ncols];
            memset(nseg, 0, ncols * sizeof(int));
            unsigned long max = mCalcObj ? mCalcObj->GetMaxVal() : mNumTraces;
            int defCol = mNumTraces > 1 ? ncols - 1 : 0;
            if (mNumPix && mHistogram) {
                const int kSegMax = 100;
                for (i=0; i<nbin; ++i) {
                    unsigned long *dat = (unsigned long *)mHistogram + (i + noffset) * mNumPix;
                    x = x1 + ((i+1)*(x2-x1)+nbin/2)/nbin + 1;
                    for (j=0; j<mNumPix; ++j) {
                        if (!dat[j]) continue;
                        y = y2 - j;
                        if (dat[j] == 1) {
                            col = 0;
                        } else if (max) { // (may be 0 if all points are offscale)
                            col = (dat[j] * ncols) / max;
                            if (col >= ncols) col = ncols - 1;
                        } else {
                            col = defCol;
                        }
                        if (!spp[col]) {
                            spp[col] = new XSegment[kSegMax];
                            if (!spp[col]) break;
                        }
                        if (nseg[col] >= kSegMax) {
                            SetForeground(FIRST_SCALE_COL + col);
                            DrawSegments(spp[col], kSegMax);
                            nseg[col] = 0;
                        }
                        XSegment *sp = spp[col] + nseg[col]++;
                        sp->x1 = lastx;
                        sp->x2 = x;
                        sp->y1 = sp->y2 = y;
                    }
                    lastx = x;
                }
                for (col=0; col<ncols; ++col) {
                    if (nseg[col]) {
                        SetForeground(FIRST_SCALE_COL + col);
                        DrawSegments(spp[col], nseg[col]);
                        delete [] spp[col];
                    }
                }
                delete [] nseg;
                delete [] spp;
            }
            SetForeground(TEXT_COL);

		} else if (mStyle == kHistStyleBars) {
		
			if (mHistogram) {
				for (i=0; i<nbin; ++i) {

					if (!(counts = mHistogram[i+noffset])) continue;
					
					x = x1 + (i*(x2-x1)+nbin/2)/nbin + 1;
					
					/* draw discarded hits */
					if (mOverlay && mOverlay[i+noffset]) {
						y = mYScale->GetPix(counts);
						if (y == y2) --y;
						SetForeground(mOverlayCol);
						FillRectangle(x, y, dx, y2-y);
						if (!(counts -= mOverlay[i+noffset])) continue;
					}
					/* draw underscale and overscale bars */
					if (i==0) {
						y = mYScale->GetPix(counts);
						if (y == y2) --y;
						dy = y2 - y;
						if (dy < 0) { dy = -dy; y = y2; }
						if (mHistCols) SetForeground(mHistCols[0]);
						FillRectangle(x, y, dx, dy);
						if (!(counts -= mUnderscale)) continue;
					} else if (i == nbin - 1) {
						y = mYScale->GetPix(counts);
						if (y == y2) --y;
						dy = y2 - y;
						if (dy < 0) { dy = -dy; y = y2; }
						if (mHistCols) SetForeground(mHistCols[mNumCols-1]);
						FillRectangle(x, y, dx, dy);
						if (!(counts -= mOverscale)) continue;
					}
					
					y = mYScale->GetPix(counts);
					if (y == y2) --y;	/* don't let bars disappear unless they are truly zero */
					dy = y2 - y;
					if (dy < 0) { dy = -dy; y = y2; }
					if (mHistCols) {
						n = 1 + i * (mNumCols-3)/(nbin-1);
						SetForeground(mHistCols[n]);
					}
					FillRectangle(x, y, dx, dy);
				}
			}
			
		} else {
			
	        XSegment *sp, *segments = new XSegment[nbin * 2];
	        if (!segments) {
	            printf("memory low\n");
	            return;
	        }
			if (mOverlay) {
				SetForeground(mOverlayCol);
				sp = segments;
				lastx = x1;
				lasty = y1;
				for (i=0; i<nbin; ++i) {
					counts = mOverlay[i+noffset];
					x = x1 + ((i+1)*(x2-x1)+nbin/2)/nbin + 1;
					y = mYScale->GetPix(counts * mOverlayScale);
					if (y==y2 && counts) --y;	// don't let bars disappear unless they are truly zero
					// range-check y position
					if (y < y1 - 1) y = y1 - 1;
					if (y > y2 + 1) y = y2 + 1;
					if (i && y != lasty) {
					    sp->x1 = lastx;  sp->y1 = lasty;
					    sp->x2 = lastx;  sp->y2 = y;
					    ++sp;
					}
					if (x != lastx) {
					    sp->x1 = lastx; sp->y1 = y;
					    sp->x2 = x;     sp->y2 = y;
					    ++sp;
					}
					lastx = x;
					lasty = y;
				}
				DrawSegments(segments, sp-segments);
				SetForeground(TEXT_COL);
			}
			if (mHistogram) {
				SetForeground(mPlotCol);
				sp = segments;
				lastx = x1;
				lasty = y1;
				for (i=0; i<nbin; ++i) {
					counts = mHistogram[i+noffset];
					x = x1 + ((i+1)*(x2-x1)+nbin/2)/nbin + 1;
					y = mYScale->GetPix(counts);
					if (y==y2 && counts) --y;	// don't let bars disappear unless they are truly zero
					// range-check y position
					if (y < y1 - 1) y = y1 - 1;
					if (y > y2 + 1) y = y2 + 1;
					if (i && y != lasty) {
					    sp->x1 = lastx;  sp->y1 = lasty;
					    sp->x2 = lastx;  sp->y2 = y;
					    ++sp;
					}
					if (x != lastx) {
					    sp->x1 = lastx; sp->y1 = y;
					    sp->x2 = x;     sp->y2 = y;
					    ++sp;
					}
					lastx = x;
					lasty = y;
				}
				DrawSegments(segments, sp-segments);
				SetForeground(TEXT_COL);
			}
			delete [] segments;
		}
	}
/*
** Draw histogram label
*/
	if (mLabel) {
		SetForeground(TEXT_COL);
		SetFont(PResourceManager::sResource.hist_font);
		DrawString(x2,y1+HIST_LABEL_Y,mLabel,kTextAlignTopRight);
	}
}

void PHistImage::Resize()
{
	if (mXScale) {
		delete mXScale;
		mXScale = NULL;
	}
	if (mYScale) {
		delete mYScale;
		mYScale = NULL;
	}
	SetDirty(kDirtyHistCalc);
}

// this must only be called when the scale window is open
void PHistImage::ReadScaleValues()
{
	Arg			wargs[1];
	Boolean		isLog = FALSE;
	float		xmin, xmax;
	long        ymin, ymax;
	char		*str;

	xmin = atof(str = XmTextGetString(sp_min));
	XtFree(str);
	xmax = atof(str = XmTextGetString(sp_max));
	XtFree(str);
	ymin = atol(str = XmTextGetString(sp_ymin));
	XtFree(str);
	ymax = atol(str = XmTextGetString(sp_ymax));
	XtFree(str);
	
	XtSetArg(wargs[0], XmNset, &isLog);
	XtGetValues(sp_log, wargs, 1);
	
	SetLog(isLog);

	// check scale range
	SetScaleLimits();
	mGrabFlag |= GRAB_X_ACTIVE | GRAB_X;
	DoGrab(xmin, xmax);
	DoGrabY(ymin, ymax);
	mGrabFlag &= ~GRABS_ACTIVE;
	SetDirty(kDirtyHistCalc);
	DoneGrab();
	UpdateScaleInfo();
}

int PHistImage::GetPix(long val)
{
    return mHeight - HIST_MARGIN_BOTTOM - mYScale->GetPix((double)val);
}

void PHistImage::CheckScaleRange()
{
	if (mXMin < mXMinMin) {
		mXMin = mXMinMin;
	}
	if (mXMax > mXMaxMax) {
		mXMax = mXMaxMax;
	}
	if (mXMax - mXMin < mXMinRng) {
		mXMax = mXMin + mXMinRng;
		if (mXMax > mXMaxMax) {
			mXMax = mXMaxMax;
			mXMin = mXMax - mXMinRng;
		}
	}
}


void PHistImage::ScaleFullProc(Widget w, PHistImage *hist, caddr_t call_data)
{
	char buff[256];

	hist->SetScaleLimits();
	sprintf(buff,"%g",hist->mXMinMin);
	setTextString(hist->sp_min,buff);

	sprintf(buff,"%g",hist->mXMaxMax);
	setTextString(hist->sp_max,buff);
}

void PHistImage::ScaleAutoProc(Widget w, PHistImage *hist, caddr_t call_data)
{
	char buff[256];
	int min=0x7fffffff, max=0, counts;

	if (hist->mNumBins) {
		int noffset, nbin;
		if (hist->mFixedBins) {
		    noffset = (long)hist->GetScaleMin();
		    if (noffset < 0) noffset = 0;
		    nbin = (long)(hist->GetScaleMax() + 0.5) - noffset;
		    if (nbin > hist->mNumBins) nbin = hist->mNumBins;
	    } else {
		    noffset = 0;
		    nbin = hist->mNumBins;
		}
		if (hist->mHistogram) {
		    if (!hist->mCalcObj || !hist->mCalcObj->GetRange(hist, noffset, nbin, &min, &max)) {
                for (int i=0; i<nbin; ++i) {
                    counts = hist->mHistogram[i+noffset];
                    if (max < counts) max = counts;
                    if (min > counts) min = counts;
                }
            }
		}
		if (hist->mOverlay) {
			for (int i=0; i<nbin; ++i) {
				counts = hist->mOverlay[i+noffset];
				if (max < counts) max = counts;
				if (min > counts) min = counts;
			}
		}
		if (hist->mHistogram || hist->mOverlay) {
			int rng = max - min;
			if (rng < 10) {
				min = max - 9;
				if (min < 0) min = 0;
				max = min + 10;
			} else {
				int pad = rng / 10;
				min -= pad;
				if (min < 0) min = 0;
				max += pad;
			}
			sprintf(buff,"%ld",(long)min);
			setTextString(hist->sp_ymin,buff);
		
			sprintf(buff,"%ld",(long)max);
			setTextString(hist->sp_ymax,buff);
		}
	}
}

void PHistImage::ScaleOKProc(Widget w, PHistImage *hist, caddr_t call_data)
{
	ImageData *data = hist->mOwner->GetData();
	
	hist->ReadScaleValues();
	if (data->mWindow[SCALE_WINDOW]) {
		delete data->mWindow[SCALE_WINDOW];
	}
}

void PHistImage::ApplyProc(Widget w, PHistImage *hist, caddr_t call_data)
{
	hist->ReadScaleValues();
}

void PHistImage::CancelProc(Widget w, Widget aShell, caddr_t call_data)
{
	XtDestroyWidget(aShell);
}

