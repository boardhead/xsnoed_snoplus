/*
** X-window offscreen drawable - PH 09/22/99
*/
#include <stdio.h>
#include <string.h>
#include "PDrawXPixmap.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "colours.h"

PDrawXPixmap::PDrawXPixmap(Display *dpy, GC gc, int depth, Widget w)
{
	mDpy = dpy;
	mGC	 = gc;
	mDepth = depth;
	mAltWidget = w;	// widget to draw into if we can't create pixmap
	mPix = 0;
	mWidth = 0;
	mHeight = 0;
}

PDrawXPixmap::~PDrawXPixmap()
{
	FreePixmap();
}

//---------------------------------------------------------------------------------------
// BeginDrawing
//
int PDrawXPixmap::BeginDrawing(int width, int height)
{
	int		sizeChanged = 0;
	
	if (mWidth != width || mHeight != height) {
		// free old pixmap since our size changed
		FreePixmap();
		sizeChanged = 1;
	}
	if (!mPix) {
		if (!width) return(0);
		mPix = XCreatePixmap(mDpy, DefaultRootWindow(mDpy), width, height, mDepth); 
		if (mPix) {
			mDrawable = mPix;
		} else {
			if (sizeChanged) {
				Printf("No memory for pixmap!\x07\n");
			}
			if (mAltWidget && XtWindow(mAltWidget)) {
				mDrawable = XtWindow(mAltWidget);
			} else {
				return(0);
			}
		}
		mWidth = width;
		mHeight = height;
	}
	return(1);
}

void PDrawXPixmap::EndDrawing()
{
}

void PDrawXPixmap::FreePixmap()
{
	if (mPix) {
		XFreePixmap(mDpy,mPix);
		mPix = 0;
		mWidth = 0;
		mHeight = 0;
		mDrawable = 0;
	}
}

int PDrawXPixmap::HasPixmap()
{
	return(mPix != 0);
}

void PDrawXPixmap::SetForeground(int col_num)
{
	if (mColours) {
		XSetForeground(mDpy, mGC, mColours[col_num]);
	} else {
		if (col_num < NUM_COLOURS) {
			XSetForeground(mDpy, mGC, PResourceManager::sResource.colour[col_num]);
		} else if ((col_num-=NUM_COLOURS) < PResourceManager::sResource.num_cols) {
			XSetForeground(mDpy, mGC, PResourceManager::sResource.scale_col[col_num]);
		} else if ((col_num-=PResourceManager::sResource.num_cols) < PResourceManager::sResource.ves_cols) {
			XSetForeground(mDpy, mGC, PResourceManager::sResource.vessel_col[col_num]);
		}
	}
}

void PDrawXPixmap::SetForegroundPixel(Pixel pixel)
{
	XSetForeground(mDpy, mGC, pixel);
}

void PDrawXPixmap::SetFont(XFontStruct *font)
{
	PDrawable::SetFont(font);
	XSetFont(mDpy, mGC, font->fid);
}

void PDrawXPixmap::SetLineWidth(float width)
{
	XSetLineAttributes(mDpy, mGC, (int)width, LineSolid, CapButt, JoinMiter);
}

void PDrawXPixmap::DrawRectangle(int x,int y,int w,int h)
{
	XDrawRectangle(mDpy, mDrawable, mGC, x, y, w, h);
}

void PDrawXPixmap::FillRectangle(int x,int y,int w,int h)
{
	XFillRectangle(mDpy, mDrawable, mGC, x, y, w, h);
}

void PDrawXPixmap::DrawSegments(XSegment *segments, int num)
{
	XDrawSegments(mDpy,mDrawable,mGC,segments,num);
}

void PDrawXPixmap::DrawPoint(int x, int y)
{
	XDrawPoint(mDpy,mDrawable,mGC,x,y);
}

void PDrawXPixmap::DrawLine(int x1,int y1,int x2,int y2)
{
	XDrawLine(mDpy,mDrawable,mGC,x1,y1,x2,y2);
}

void PDrawXPixmap::FillPolygon(XPoint *point, int num)
{
	XFillPolygon(mDpy,mDrawable,mGC,point,num, Convex, CoordModeOrigin);
}

void PDrawXPixmap::DrawString(int x, int y, char *str,ETextAlign_q align)
{
	int len = strlen(str);
	
	if (GetFont()) {
		switch (align % 3) {
			case 0:		// left
				break;
			case 1:		// right
				x -= XTextWidth(GetFont(), str, len) / 2;
				break;
			case 2:		// center
				x -= XTextWidth(GetFont(), str, len);
				break;
		}
		switch (align / 3) {
			case 0:		// top
				y += GetFont()->ascent;
				break;
			case 1:		// middle
				y += GetFont()->ascent / 2;
				break;
			case 2:		// bottom
				break;
		}
	}
	XDrawString(mDpy,mDrawable,mGC,x,y,str,len);
}

void PDrawXPixmap::DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
	XDrawArc(mDpy,mDrawable,mGC,cx-rx, cy-ry, 2*rx, 2*ry, (int)(ang1 * 64), (int)(ang2 * 64));
}

void PDrawXPixmap::FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
	XFillArc(mDpy,mDrawable,mGC,cx-rx, cy-ry, 2*rx+1, 2*ry+1, (int)(ang1 * 64), (int)(ang2 * 64));
}

void PDrawXPixmap::PutImage(XImage *image, int dest_x, int dest_y)
{
	XPutImage(mDpy,mDrawable,mGC,image,0,0,dest_x,dest_y,image->width,image->height);
}

XImage*	PDrawXPixmap::GetImage(int x, int y, int width, int height)
{
	return(XGetImage(mDpy,mDrawable,x,y,width,height,AllPlanes,ZPixmap));
}

int PDrawXPixmap::CopyArea(int x,int y,int w,int h,Window dest)
{
	if (mPix) {
		XCopyArea(mDpy,mPix,dest,mGC,x,y,w,h,x,y);
		return(1);
	} else {
		return(0);
	}
}
