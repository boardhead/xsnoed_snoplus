#include <string.h>
#include "PResourceManager.h"
#include "PColourPicker.h"
#include "PImageWindow.h"
#include "PColourWindow.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"
#include "colours.h"

const int	kMargin 	= 10;
const int	kNcolumns 	= 3;
const int	kNrows 		= (NUM_COLOURS + kNcolumns - 1) / kNcolumns;
const int	kH 			= 16;
const int	kW 			= 30;
const int	kTextWid 	= 90;
const int 	kTextMargin = 8;

const int 	kBothSets	= 2;	// flag for FreeColours to free both sets

// last selected colour
int PColourPicker::sLastColourNum	= -1;

static char	*sColourName[] = {
	"Background", "Labels & Text", "PSUP Back", "PSUP Front", "Vessel Dark",
		"Vessel Light", "3-D Axes", "Current Fit", "Other Fits", "Underscale",
	"0% Scale", "25% Scale", "50% Scale", "75% Scale", "100% Scale",
	    "Overscale", "Discarded Hit", "Map Grids", "Sun Vector", "Water Level",
	"NCD Tubes", "NCD Scope", "CAEN Scope", "MC Neutrino", "MC Electron",
	    "MC Neutron", "MC Gamma", "MC Photon", "MC Special", "MC Other"
};


// ------------------------------------------------------------

PColourPicker::PColourPicker(PImageWindow *owner, Widget canvas)
		  : PImageCanvas(owner,canvas,PointerMotionMask|ButtonPressMask|ButtonReleaseMask)
{
	if (!canvas) {
		CreateCanvas("colourPicker");
	}
	// set the font for our drawable (must be done after creating canvas)
	SetFont(PResourceManager::sResource.hist_font);
	
	mDrawLabel 	= 0;			// don't draw the image label
	mCurrentSet = -1;			// initialize to invalid colour set number
	mAllocErr	= 0;

	// allocate our colour arrays (save both colour sets)
	mColours = new XColor[2*NUM_COLOURS];
	mRevertColours = new XColor[2*NUM_COLOURS];
	mAllocFlags = new char[2*NUM_COLOURS];
	if (!mColours || !mRevertColours || !mAllocFlags) quit("Out of memory");
	
	// copy both current colour sets into our working array
	for (int i=0; i<2*NUM_COLOURS; ++i) {
		memcpy(mColours+i, PResourceManager::GetColour(i), sizeof(XColor));
	}
	
	// save a copy of these colours in our revert array
	memcpy(mRevertColours, mColours, 2*NUM_COLOURS*sizeof(XColor));
	
	// zero our colour allocated flags
	memset(mAllocFlags, 0, 2*NUM_COLOURS);
	
	SetCurrentColours();

	// intialize sLastColourNum if not done already
	if (sLastColourNum < 0) {
		sLastColourNum = (mCurrentSet ? BKG_COL : TEXT_COL);
	}
	// initialize current colour number
	mColourNum = sLastColourNum;
}

PColourPicker::~PColourPicker()
{
	FreeColours(kBothSets);
	
	delete [] mColours;
	delete [] mRevertColours;
	delete [] mAllocFlags;
}

void PColourPicker::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageColoursChanged:
			if (SetCurrentColours()) {
				((PColourWindow *)mOwner)->PickerColourChanged();
			}
			break;
	}
}

// FreeColours - free allocated colours in one or both colour sets
void PColourPicker::FreeColours(int flags)
{
	Display	  *	dpy  = PResourceManager::sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);
	
	// determine colours to free:
	// flags = 0 - free 1st colour set (0 to NUM_COLOURS-1)
	// flags = 1 - free 2nd colour set (NUM_COLOURS to 2*NUM_COLOURS-1)
	// flags = 2 - free both colour sets (0 to 2*NUM_COLOURS-1)
	int 		firstCol = (flags==1 ? NUM_COLOURS : 0);
	int			lastCol  = (flags==0 ? NUM_COLOURS : 2*NUM_COLOURS);

	// free colours individually
	for (int i=firstCol; i<lastCol; ++i) {
		if (mAllocFlags[i]) {
			XFreeColors(dpy, cmap, &mColours[i].pixel, 1, 0);
			mAllocFlags[i] = 0;
		}
	}
}

// Set the current colour set
// - returns non-zero and updates display if the colour set changed
int PColourPicker::SetCurrentColours()
{
	int newSet = PResourceManager::sResource.image_col & kWhiteBkg ? 1 : 0;
	
	if (mCurrentSet != newSet) {
		mCurrentSet = newSet;
		SetDirty(kDirtyPix);	// our pixmap requires redrawing
		return(1);
	} else {
		return(0);
	}
}

// Apply current colours to image for the displayed colour set only
void PColourPicker::ApplyCurrentColours()
{
	int 	offset = mCurrentSet * NUM_COLOURS;
	Pixel *	colours = PResourceManager::sResource.colset[0];
	int		changed = 0;
	
	for (int i=offset; i<NUM_COLOURS+offset; ++i) {
		if (mAllocFlags[i] && colours[i]!=mColours[i].pixel) {
			// update resource manager colour array
			PResourceManager::SetColour(i, mColours+i);
			// the resource manager now owns the colour
			mAllocFlags[i] = 0;
			// set changed flag to force re-loading of colour set
			changed = 1;
		}
	}
	if (changed) {
		int oldSet = PResourceManager::sResource.image_col;
		// force a change of the colour set
		PResourceManager::sResource.image_col = -1;
		// we want to ignore the message this will send
		// (we don't need to be redrawn -- we already have the current colours
		Ignore(1);
		PResourceManager::SetColours(oldSet);
		Ignore(0);
	}
}

// RevertColours - revert current colour set back to original values
// - returns non-zero if any colours were changed
int PColourPicker::RevertColours()
{
	Display	  *	dpy  = PResourceManager::sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);
	int 		offset = mCurrentSet * NUM_COLOURS;
	int			changed = 0;
	int			errCount = 0;
	
	// first, free all of the colours we had previously allocated in this set
	FreeColours(mCurrentSet);
	
	// then change back all of the colours that we modified
	for (int i=offset; i<NUM_COLOURS+offset; ++i) {
		// change back our working colour if it differs from the revert colour
		if (memcmp(mColours+i, mRevertColours+i, sizeof(XColor))) {
			changed |= 0x01;	// the working colours changed
			// copy the original colour back into the working array
			memcpy(mColours+i, mRevertColours+i, sizeof(XColor));
		}
		// revert the current image colour if it was changed
		XColor *res_col = PResourceManager::GetColour(i);
		if (memcmp(res_col, mColours+i, sizeof(XColor))) {
			changed |= 0x02;	// the resource colours changed
			// re-allocate the original colour
			// Note: Do NOT set the alloc flags because we will be
			//       promptly handing it off to the resource manager!
			if (XAllocColor(dpy, cmap, mColours+i)) {
				// install the colour in the resource manager
				// (it now owns the colour and will free it when done with it)
				PResourceManager::SetColour(i, mColours+i);
			} else {
				++errCount;
			}
		}
	}
	// must redraw ourself if our working colours changed
	if (changed & 0x01) {
		SetDirty(kDirtyPix);
	}
	// did we change the image (resource) colours ?
	if (changed & 0x02) {
		// force a change of the colour set
		int oldSet = PResourceManager::sResource.image_col;
		PResourceManager::sResource.image_col = -1;
		PResourceManager::SetColours(oldSet);
	}
	if (errCount) {
		Printf("Error allocating colors -- couldn't revert %d colors\x07\n",errCount);
	}
	return(changed);
}

void PColourPicker::SetColourRGB(int *col3)
{
	int r = col3[0] | (col3[0] << 8);
	int g = col3[1] | (col3[1] << 8);
	int b = col3[2] | (col3[2] << 8);
	
	int i = mColourNum + mCurrentSet * NUM_COLOURS;
	XColor *xcol = mColours + i;

	if (r!=xcol->red || g!=xcol->green || b!=xcol->blue) {
	
		Display	  *	dpy  = PResourceManager::sResource.display;
		int			scr  = DefaultScreen(dpy);
		Colormap	cmap = DefaultColormap(dpy, scr);
		
		// free current colour if we allocated it
		if (mAllocFlags[i]) {
			XFreeColors(dpy, cmap, &xcol->pixel, 1, 0);
			mAllocFlags[i] = 0;
		}
		
		// allocate new colour
		xcol->red = r;
		xcol->green = g;
		xcol->blue = b;
		if (XAllocColor(dpy, cmap, xcol)) {
			mAllocFlags[i] = 1;
			// draw the single colour immediately
			int x,y;
			GetXY(mColourNum,&x,&y);
			GC gc = PResourceManager::sResource.gc;
			XSetForeground(dpy,gc,xcol->pixel);
			XFillRectangle(dpy,XtWindow(mCanvas),gc,x+2,y+2,kW-3,kH-3);
			// quietly set our pixmap dirty so it will be redrawn when next used
			mDirty |= kDirtyPix;
		} else {
			if (!mAllocErr) {
				mAllocErr = 1;
				Printf("Error allocating color -- color can not be changed!\x07\n");
			}
		}
	}
}

void PColourPicker::GetColourRGB(int *col3)
{
	XColor *xcol = mColours + mColourNum + mCurrentSet * NUM_COLOURS;
	
	col3[0] = xcol->red   >> 8;
	col3[1] = xcol->green >> 8;
	col3[2] = xcol->blue  >> 8;
}

void PColourPicker::AfterDrawing()
{
	// draw border around current colour
	Display   *	dpy = XtDisplay(mCanvas);
	GC 			gc = PResourceManager::sResource.gc;
	
	XSetForeground(dpy, gc, PResourceManager::sResource.black_col);

	// draw box around current color selection
	int x, y;
	GetXY(mColourNum, &x, &y);
	XDrawRectangle(dpy,XtWindow(mCanvas),gc,x-1,y-1,kW+2,kH+2);
	XDrawRectangle(dpy,XtWindow(mCanvas),gc,x-2,y-2,kW+4,kH+4);
	
	// underline text for this colour
	char *name = sColourName[mColourNum];
	int width = XTextWidth(mDrawable->GetFont(), name, strlen(name));
	x += kW + kTextMargin;
	y += kH - 1;
	XDrawLine(dpy,XtWindow(mCanvas),gc,x,y,x+width,y);
}

// GetXY - get X-Y position for given colour number
void PColourPicker::GetXY(int colNum, int *x, int *y)
{
	*x = (colNum / kNrows) * (kW + kMargin + kTextMargin + kTextWid) + kMargin;
	*y = (colNum % kNrows) * (kH + kMargin) + kMargin;
}

void PColourPicker::HandleEvents(XEvent *event)
{
	int			x, y, i, j, colNum;
	static int	sPressed = 0;
	
	switch (event->type) {
		case ButtonPress:
			sPressed = 1;
			XGrabPointer(mDpy, XtWindow(mCanvas),0,
						 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
						 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
			// fall through!
		case MotionNotify:
			if (sPressed) {
				colNum = sLastColourNum;	// initialize colour number
				x = event->xbutton.x;
				y = event->xbutton.y;
				if (x>=0 && y>=kMargin/2) {
					// determine colour number of selected entry
					i = x / (kW + kMargin + kTextMargin + kTextWid);
					j = (y - kMargin/2) / (kH + kMargin);
					if (i<kNcolumns && j<kNrows) {
						colNum = j + i * kNrows;
					}
				}
				if (mColourNum != colNum) {
					mAllocErr = 0;	// reset error so a new message will be posted
					mColourNum = colNum;
					// draw ourself immediately (will not draw pixmap unless necessary)
					Draw();		
				}
			}
			break;
		case ButtonRelease:
			XUngrabPointer(mDpy, CurrentTime);
			sPressed = 0;
			if (sLastColourNum != mColourNum) {
				sLastColourNum = mColourNum;	// save last selected colour number
				((PColourWindow *)mOwner)->PickerColourChanged();
			}
			break;
	}
}

void PColourPicker::SetColourNumber(int colNum)
{
	mColourNum = colNum;
	mAllocErr = 0;
	SetDirty();
}

/*
** Draw colour picker image
*/
void PColourPicker::DrawSelf()
{
	// don't redraw unless we have to
	if (!IsDirtyPix()) return;

#ifdef PRINT_DRAWS
	Printf("draw Colour Picker\n");
#endif
	// clear the area
	Arg warg;
	XColor col;
	XtSetArg(warg, XmNbackground, &col);
	XtGetValues(mOwner->GetMainPane(), &warg, 1);
	mDrawable->SetForegroundPixel(col.pixel);
	FillRectangle(0, 0, mWidth, mHeight);
	
	int offset = mCurrentSet * NUM_COLOURS;
	
	// draw a bunch of rectangles
	for (int i=0; i<NUM_COLOURS; ++i) {
		int x,y;
		GetXY(i,&x,&y);
		mDrawable->SetForegroundPixel(PResourceManager::sResource.black_col);
		mDrawable->DrawRectangle(x,y,kW,kH);
		DrawString(x+kW+kTextMargin, y+kH/2, sColourName[i], kTextAlignMiddleLeft);
		mDrawable->SetForegroundPixel(PResourceManager::sResource.white_col);
		mDrawable->DrawRectangle(x+1,y+1,kW-2,kH-2);
		mDrawable->SetForegroundPixel(mColours[i+offset].pixel);
		FillRectangle(x+2,y+2,kW-3,kH-3);
	}
}
