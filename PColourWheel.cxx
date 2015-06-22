#include <math.h>
#include "PColourWheel.h"
#include "PColourWindow.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"
#include "colours.h"

//#define DEBUG_IMAGE		// un-comment this to print debugging info about image

// colour monitor types
enum EColourType {
	kIndexedColour,		// not true colour - use indexed colour
	kTrueColour15,		// 00000000000000000RrrrrGggggBbbbb
	kTrueColour16,		// 0000000000000000RrrrrGgggggBbbbb
	kTrueColour24,		// 00000000RrrrrrrrGgggggggBbbbbbbb
	kTrueColour24rev	// 00000000BbbbbbbbGgggggggRrrrrrrr
};

// number of colours for limited-colour monitors
const int	kNumCols 		= 5;
const int	kTotalNumCols	= kNumCols * kNumCols * kNumCols;
const int	kShadowWidth	= 4;
const int	kMargin			= 8;
const int	kGreySize		= 5;	// radius of grey region in center of wheel

const int	kDirtyWheel		= 0x02;	// the wheel needs redrawing


// ------------------------------------------------------------

PColourWheel::PColourWheel(PImageWindow *owner, Widget canvas, int size)
		  : PImageCanvas(owner,canvas,PointerMotionMask|ButtonPressMask|ButtonReleaseMask)
{
	if (!canvas) {
		CreateCanvas("colourWheel");
	}
	
	mWheelSize 		= size / 2 - kMargin;
	mColours 		= NULL;
	mAllocFlags 	= NULL;
	mImage 			= NULL;
	mDrawLabel 		= 0;			// don't draw the image label
	mIntensity 		= 255;
	mCurX = mCurY 	= mWheelSize + kMargin;
	mDelayedUpdate	= 0;
	mFirstTry		= 1;
	
	// figure out if we can draw directly in RGB colours
	TestColours();
	
	if (mColourType == kIndexedColour) {
		// allocate colours if necessary
		AllocColours();
	}
}

PColourWheel::~PColourWheel()
{
	FreeColours();
	delete [] mColours;
	delete [] mAllocFlags;
	if (mImage) {
		XDestroyImage(mImage);
	}
}

// test colours to determine drawing strategy
// (sets mColourType before returning)
void PColourWheel::TestColours()
{
	Display	  *	dpy  = PResourceManager::sResource.display;
	int			depth = DefaultDepthOfScreen(XtScreen(mCanvas));
	
	mColourType = kIndexedColour;	// use indexed colour by default
	
	if (depth > 8) {
	
		// allocate a single colour to check the pixel RGB bit patterns
		int			scr  = DefaultScreen(dpy);
		Colormap	cmap = DefaultColormap(dpy, scr);
		XColor		tmp_col;
		
		tmp_col.flags = DoRed | DoGreen | DoBlue;
		tmp_col.red   = 0xcccc;
		tmp_col.green = 0x5555;
		tmp_col.blue  = 0x4444;

		if (XAllocColor(dpy, cmap, &tmp_col)) {
			switch (depth) {
				case 15:
				case 16:
					if (tmp_col.pixel == 0x6548) {
						// draw in standard 15-bit true colour mode
						// - pixel bit pattern is 00000000000000000RrrrrGggggBbbbb
						mColourType = kTrueColour15;
					} else if (tmp_col.pixel == 0xcaa8) {
						// draw in standard 16-bit true colour mode
						// - pixel bit pattern is 0000000000000000RrrrrGgggggBbbbb
						mColourType = kTrueColour16;
					}
					break;
				case 24:
				case 32:
					if (tmp_col.pixel == 0xcc5544) {
						// draw in standard 24-bit true colour mode
						// - pixel bit pattern is 00000000RrrrrrrrGgggggggBbbbbbbb
						mColourType = kTrueColour24;
					} else if (tmp_col.pixel == 0x4455cc) {
						// draw in standard 24-bit true colour mode (reversed)
						// - pixel bit pattern is 00000000BbbbbbbbGgggggggRrrrrrrr
						mColourType = kTrueColour24rev;
					}
					break;
			}
			if (mColourType == kIndexedColour) {
				Printf("PColourWheel: Unknown pixel pattern (0x%lx) for %d-bit depth\n",tmp_col.pixel,depth);
				Printf("(will use indexed color for wheel)\n");
				Printf("Please inform Phil Harvey of this warning so\n");
				Printf("support for your hardware can be added to XSNOED.\n");
			}
			// free the colour we allocated
			XFreeColors(dpy, cmap, &tmp_col.pixel, 1, 0);
		}
	}
}

void PColourWheel::AllocColours()
{
	Display	  *	dpy  = PResourceManager::sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);
	XColor		tmp_col;
	
	tmp_col.flags = DoRed | DoGreen | DoBlue;
	
	if (!mColours) {
		// create arrays for colours and alloc flags
		mColours = new Pixel[kTotalNumCols];
		mAllocFlags = new char[kTotalNumCols];
		if (!mColours || !mAllocFlags) quit("Out of memory");
		memset(mAllocFlags, 0, kTotalNumCols);
	} else {
		// free allocated X colours (but not arrays)
		FreeColours();
	}
	int count=0;
	int i = 0;
	// allocate X colours
	for (int r=0; r<kNumCols; ++r) {
		for (int g=0; g<kNumCols; ++g) {
			for (int b=0; b<kNumCols; ++b) {
				tmp_col.red   = r * 65535L / (kNumCols - 1);
				tmp_col.green = g * 65535L / (kNumCols - 1);
				tmp_col.blue  = b * 65535L / (kNumCols - 1);
				tmp_col.pixel = 0;
				if (XAllocColor(dpy, cmap, &tmp_col)) {
					mAllocFlags[i] = 1;	// allocated successfully
					mColours[i] = tmp_col.pixel;
					++count;
				} else {
					mAllocFlags[i] = 0;
					mColours[i] = PResourceManager::sResource.white_col;
				}
				++i;
			}
		}
	}
	if (count != kTotalNumCols) {
		Printf("%d colors could not be allocated for color wheel\n", kTotalNumCols-count);
	}
	// install this colour map into our drawable
	mDrawable->SetColourMap(mColours);
}

void PColourWheel::FreeColours()
{
	if (!mColours) return;

	int			i;
	Display	  *	dpy  = PResourceManager::sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);
	
	// are all colours allocated?
	for (i=0; i<kTotalNumCols; ++i) {
		if (!mAllocFlags[i]) break;
	}
	if (i == kTotalNumCols) {
		// free all at once
		XFreeColors(dpy, cmap, mColours, kTotalNumCols, 0);
		memset(mAllocFlags, 0, kTotalNumCols);
	} else {
		// free individually
		for (i=0; i<kTotalNumCols; ++i) {
			if (mAllocFlags[i]) {
				XFreeColors(dpy, cmap, mColours+i, 1, 0);
				mAllocFlags[i] = 0;
			}
		}
	}
}


void PColourWheel::HandleEvents(XEvent *event)
{
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
				SetCursorPos(event->xbutton.x, event->xbutton.y);
				Draw();
				CursorMoved();
				((PColourWindow *)mOwner)->WheelColourChanging();
			}
			break;
		case ButtonRelease:
			XUngrabPointer(mDpy, CurrentTime);
			sPressed = 0;
			CursorMoved();	// calculate our new colour RGB values
			((PColourWindow *)mOwner)->WheelColourChanged();
			break;
	}
}

// Calculate maximum colour values from current cursor location
void PColourWheel::CursorMoved()
{
	float 	p3 = PI / 3;
	float 	p3i = 3 / PI;
	float	x = mCurX - mWheelSize - kMargin;
	float	y = mCurY - mWheelSize - kMargin;
	float	r2b = x * x + y * y;
	float 	ang = atan2(y, -x);
	float 	f = (sqrt(r2b) - kGreySize) / (mWheelSize - kGreySize);
	if (f > 1.0) f = 1.0;
	else if (f < 0.0) f = 0.0;
	float 	pang = ang;	// positive angle
	if (pang < 0) pang += 2 * PI;
	float t;
	// calculate red component
	t = fabs(pang - PI) * p3i - 1.0;
	if (t < 0) t = 0;
	else if (t > 1.0) t = 1.0;
	float fr = (1.0 - f * t) * 255.0;
	// calculate green component
	t = fabs(ang + p3) * p3i - 1.0;
	if (t < 0) t = 0;
	else if (t > 1.0) t = 1.0;
	float fg = (1.0 - f * t) * 255.0;
	// calculate blue component
	t = fabs(ang - p3) * p3i - 1.0;
	if (t < 0) t = 0;
	else if (t > 1.0) t = 1.0;
	float fb = (1.0 - f * t) * 255.0;
	
	mMaxColour[0] = (int)(fr + 0.5);
	mMaxColour[1] = (int)(fg + 0.5);
	mMaxColour[2] = (int)(fb + 0.5);
}

void PColourWheel::SetIntensity(int val, int fastAnimate)
{
	if (mIntensity != val) {
		mIntensity = val;
		if (fastAnimate) {
			if (mImage) {
				// we have an image, so fast updates are possible
				// - update the wheel immediately
				DrawTheWheel();
				Draw();
			} else {
				// fast animation isn't available because we don't have
				// an image -- so delay the update until later
				mDelayedUpdate = 1;
			}
		} else {
			// update the wheel normally
			SetDirty(kDirtyWheel);	// force the wheel to be redrawn
		}
	}
}

void PColourWheel::AnimateDone()
{
	if (mDelayedUpdate) {
		mDelayedUpdate = 0;
		SetDirty(kDirtyWheel);
	}
}

void PColourWheel::GetColourRGB(int *col3)
{
	for (int i=0; i<3; ++i) {
		col3[i] = (int)(mMaxColour[i] * mIntensity / 255.0 + 0.5);
	}
}

void PColourWheel::SetColourRGB(int *col3)
{
	int i;
	
	// get colour component min/max values
	int minVal = col3[0];
	int maxVal = col3[0];
	for (i=1; i<3; ++i) {
		if (minVal > col3[i]) minVal = col3[i];
		if (maxVal < col3[i]) maxVal = col3[i];
	}

	// set intensity
	SetIntensity(maxVal);
	
	// set the max colour values
	if (maxVal > 0) {
		for (i=0; i<3; ++i) {
			mMaxColour[i] = col3[i] * 255.0 / maxVal;
		}
	} else {
		for (i=0; i<3; ++i) {
			mMaxColour[i] = 255;
		}
	}
	
	// set the cursor position
	int x,y;
	x = y = mWheelSize + kMargin;
	if (maxVal != minVal) {
		// calculate the cursor position from the RGB values
		float scale = maxVal - minVal;
		float radius = kGreySize + (mWheelSize - kGreySize) * (maxVal - minVal) / maxVal;
		float angle=0;
		for (i=0; i<3; ++i) {
			if (col3[i] == minVal) {
				int i1 = (i + 1) % 3;
				int i2 = (i + 2) % 3;
				if (col3[i1] == maxVal) {
					angle = 2.0 * i1 + (col3[i2] - minVal) / scale;
				} else {
					angle = 2.0 * (i1 + 1) - (col3[i1] - minVal) / scale;
				}
				break;
			}
		}
		x = (int)(x + radius * cos(angle * PI / 3) + 0.5);
		y = (int)(y - radius * sin(angle * PI / 3) + 0.5);
	}
	SetCursorPos(x, y);
	SetDirty();
}

void PColourWheel::SetCursorPos(int x, int y)
{
	if (mCurX != x || mCurY != y) {
		// limit xy to inside the wheel
		int tx = x - mWheelSize - kMargin;
		int ty = y - mWheelSize - kMargin;
		int r2 = tx * tx + ty * ty;
		if (r2 > mWheelSize * mWheelSize) {
			float f = mWheelSize / sqrt((float)r2);
			x = (int)(tx * f + mWheelSize + kMargin + 0.5);
			y = (int)(ty * f + mWheelSize + kMargin + 0.5);
			if (mCurX==x && mCurY==y) return;
		}
		mCurX = x;
		mCurY = y;
	}
}

void PColourWheel::AfterDrawing()
{
	int 		x = mCurX;
	int 		y = mCurY;
	Display   *	dpy = XtDisplay(mCanvas);
	GC 			gc = mOwner->GetData()->gc;
	XSegment	seg[4];

	// draw the colour cursor
	for (int i=-1; i<=1; ++i) {
		int n = 0;
		seg[n].x1 = x-8;	seg[n].y1 = y+i;
		seg[n].x2 = x-2;	seg[n].y2 = y+i;
		++n;
		seg[n].x1 = x+2;	seg[n].y1 = y+i;
		seg[n].x2 = x+8;	seg[n].y2 = y+i;
		++n;
		seg[n].x1 = x+i;	seg[n].y1 = y-8;
		seg[n].x2 = x+i;	seg[n].y2 = y-2;
		++n;
		seg[n].x1 = x+i;	seg[n].y1 = y+2;
		seg[n].x2 = x+i;	seg[n].y2 = y+8;
		++n;
		if (i) {
			XSetForeground(dpy, gc, PResourceManager::sResource.white_col);
		} else {
			XSetForeground(dpy, gc, PResourceManager::sResource.black_col);
		}
		XDrawSegments(dpy,XtWindow(mCanvas),gc,seg,n);
	}
}

void PColourWheel::DrawSelf()
{
	// do nothing if we don't need to redraw the wheel
	if (IsDirty() & (kDirtyPix | kDirtyWheel)) {
		DrawTheWheel();
	}
}

/*
** Draw colour picker image
*/
void PColourWheel::DrawTheWheel()
{
#ifdef PRINT_DRAWS
	Printf("draw Colour Wheel\n");
#endif
	int wheelX = mWheelSize + kMargin;
	int wheelY = mWheelSize + kMargin;
	int imageSize = mWheelSize * 2 - 1;
	float brightness = mIntensity / 255.0;
	float maxCol;
	if (mColourType == kIndexedColour) {
		// indexed colour
		maxCol = (kNumCols - 1) * brightness;
	} else {
		// true colour - maximum is 0xffff
		maxCol = 65535 * brightness;
	}
	
	// do we need to draw into a newly created pixmap?
	if (IsDirtyPix()) {
/*
** draw the constant background components of the image into the drawable
*/
		// clear the area
		Arg warg;
		Pixel pixel;
		XtSetArg(warg, XmNbackground, &pixel);
		XtGetValues(mOwner->GetMainPane(), &warg, 1);
		mDrawable->SetForegroundPixel(pixel);
		FillRectangle(0, 0, mWidth, mHeight);
		// draw the shadow
		XtSetArg(warg, XmNbottomShadowColor, &pixel);
		XtGetValues(mOwner->GetMainPane(), &warg, 1);
		mDrawable->SetForegroundPixel(pixel);
		FillArc(wheelX+kShadowWidth,wheelY+kShadowWidth,mWheelSize,mWheelSize);
		// create our client-side image if we haven't already done so
		if (mFirstTry) {
			mFirstTry = 0;	// only try to create image once
			// create the image from the drawing we just did
			mImage = mDrawable->GetImage(kMargin+1, kMargin+1, imageSize, imageSize);
			if (!mImage) {
				Printf("PColourWheel: Error creating image\n");
#ifdef DEBUG_IMAGE
			} else {
				Printf("byte order=%d  bit order=%d  pad=%d\n",
					mImage->byte_order,mImage->bitmap_bit_order,mImage->bitmap_pad);
				Printf("depth=%d bits/pix=%d r=%lx g=%lx b=%lx\n",
					mImage->depth,mImage->bits_per_pixel,
					mImage->red_mask,mImage->green_mask,mImage->blue_mask);
#endif
			}
		}
	}

	// draw the colour wheel
	int cen = mWheelSize - 1;	// the center pixel
	int r2max = mWheelSize * mWheelSize;	// the maximum radius we will draw (squared)
	float p3 = PI / 3;
	float p3i = 1 / p3;
	float er, eg, eb;	// colour errors for dithering
	er = eg = eb = 0.5; // initialize remainders for dithering
	for (int j=0; j<imageSize; ++j) {
		int y = j - cen;
		for (int i=0; i<imageSize; ++i) {
			int x = i - cen;
			int r2 = x * x + y * y;
			if (r2 > r2max) continue;
			float ang = atan2((float)y, (float)-x);  // get colour angle (-pi -> pi)
			// calculate distance fraction of full radius
			float f = (sqrt((float)r2) - kGreySize) / (float)(mWheelSize - kGreySize);
			if (f < 0) f = 0;
			float pang = ang;	// positive angle
			if (pang < 0) pang += 2 * PI;
			// calculate red component
			float t = fabs(pang - PI) * p3i - 1.0;
			if (t < 0) t = 0;
			else if (t > 1.0) t = 1.0;
			float fr = (1.0 - f * t) * maxCol;
			// calculate green component
			t = fabs(ang + p3) * p3i - 1.0;
			if (t < 0) t = 0;
			else if (t > 1.0) t = 1.0;
			float fg = (1.0 - f * t) * maxCol;
			// calculate blue component
			t = fabs(ang - p3) * p3i - 1.0;
			if (t < 0) t = 0;
			else if (t > 1.0) t = 1.0;
			float fb = (1.0 - f * t) * maxCol;
			// convert to integer RGB components
			int r = (int)(fr + 0.5);
			int g = (int)(fg + 0.5);
			int b = (int)(fb + 0.5);
			// calculate corresponding pixel value for drawing
			Pixel thePixel=0;
			switch (mColourType) {
				case kIndexedColour:
					// we are using our allocated colour map
					// -- dither the colours
					r = (int)(fr += er);
					g = (int)(fg += eg);
					b = (int)(fb += eb);
					er = fr - r;
					eg = fg - g;
					eb = fb - b;
					thePixel = mColours[((r * kNumCols) + g) * kNumCols + b];
					break;
				case kTrueColour15:
					thePixel = ((r & 0xf800) >> 1) | ((g & 0xf800) >> 6) | ((b & 0xf800) >> 11);
					break;
				case kTrueColour16:
					thePixel = (r & 0xf800) | ((g & 0xfc00) >> 5) | ((b & 0xf800) >> 11);
					break;
				case kTrueColour24:
					thePixel = ((r & 0xff00) << 8) | (g & 0xff00) | ((b & 0xff00) >> 8);
					break;
				case kTrueColour24rev:
					thePixel = ((r & 0xff00) >> 8) | (g & 0xff00) | ((b & 0xff00) << 8);
					break;
			}
			// set this pixel in the image or pixmap
			if (mImage) {
				XPutPixel(mImage, i, j, thePixel);
			} else {
				mDrawable->SetForegroundPixel(thePixel);
				mDrawable->DrawPoint(i+kMargin+1, j+kMargin+1);
			}
		}
	}

	// if we have an image, put it into the pixmap
	if (mImage) {
		mDrawable->PutImage(mImage, kMargin+1, kMargin+1);
	}
			  
	// finally, draw circle around colour wheel
	mDrawable->SetForegroundPixel(PResourceManager::sResource.black_col);
	DrawArc(wheelX,wheelY,mWheelSize,mWheelSize);
}
