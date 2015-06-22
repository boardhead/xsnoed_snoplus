#ifndef __PImageCanvas_h__
#define __PImageCanvas_h__

#include <Xm/Xm.h>

#include "PScrollBar.h"
#include "PListener.h"
#include "PDrawable.h"
#include "TextSpec.h"

// masks for CreateCanvas() routine
enum {
	kScrollLeftMask	= 1 << kScrollLeft,
	kScrollRightMask = 1 << kScrollRight,
	kScrollBottomMask = 1 << kScrollBottom,
	kScrollAllMask = kScrollLeftMask | kScrollRightMask | kScrollBottomMask
};

enum EPrintFlags {
	kPrintLandscape = 0x01
};

enum EDirtyFlags {
	kDirtyNormal	= 0x0001,
	kDirtyPix		= 0x8000
};

class PImageWindow;
struct Node;

class PImageCanvas : public PScrollHandler, public PListener {
public:
	PImageCanvas(PImageWindow *owner, Widget canvas, EventMask eventMask=0);
	virtual ~PImageCanvas();
	
	void			CreateCanvas(char *name, int scrollBarMask=0);
	void			SetCanvas(Widget canvas);
	Widget			GetCanvas()		{ return mCanvas;	}
	
	void			Draw();					// called to draw image in canvas and copy to screen
	void			Prepare();
	void			SetCursor(int type);
	void			DrawLabel(int x,int y,ETextAlign_q align);

	int				IsInLabel(int x, int y);
	void			ShowLabel(int on);
	void            AllowLabel(int on);
	int				IsLabelOn()		{ return mDrawLabel; }
	
	virtual void	Listen(int message, void *dataPt);
	virtual int		Print(char *filename, int flags=0);	// print image to postscript file
	virtual void	DrawSelf();				// override by derived types to perform drawing
	virtual void	AfterDrawing()	{ }		// called after any drawing to screen
	virtual void	Resize()		{ SetDirty(); }		// called before drawing if canvas was resized
	virtual void	SetScrolls()	{ }		// set scrollbars of owner window
	virtual void	SetToHome(int n=0) { }		// set image to home position
	
	virtual void	HandleEvents(XEvent *event) { }
	virtual void	SetCursorForPos(int x, int y);
	virtual void	Transform(Node *node, int num_nodes) { }
	virtual void	TransformHits() 	{ sLastTransformHits = this; }

	void			SetDirty(int flag=kDirtyNormal);
	int				IsDirty()			{ return mDirty;			 }
	int				IsDirtyPix()		{ return mDirty & kDirtyPix; }
	PImageWindow  *	GetOwner()			{ return mOwner;			 }

	// convenience functions for accessing mDrawable
	int				GetScaling()							  { return mDrawable->GetScaling(); }
	void			SetForeground(int col_num) 				  { mDrawable->SetForeground(col_num); }
	void			SetFont(XFontStruct *font)				  { mDrawable->SetFont(font); }
	XFontStruct	  *	GetFont()								  { return mDrawable->GetFont(); }
	void			SetLineWidth(float width)				  { mDrawable->SetLineWidth(width); }
	void			SetLineType(ELineType type)				  { mDrawable->SetLineType(type); }
	void			DrawSegments(XSegment *segments, int num) { mDrawable->DrawSegments(segments,num); }
	void			DrawLine(int x1,int y1,int x2,int y2) 	  { mDrawable->DrawLine(x1,y1,x2,y2); }
	void			FillRectangle(int x,int y,int w, int h)	  { mDrawable->FillRectangle(x,y,w,h); }
	void			FillPolygon(XPoint *point, int num) 	  { mDrawable->FillPolygon(point,num); }
	void			DrawString(int x, int y, char *str,ETextAlign_q align)
															  { mDrawable->DrawString(x,y,str,align); }
	void			DrawArc(int cx,int cy,int rx,int ry,float ang1=0.0,float ang2=360.0)
															  { mDrawable->DrawArc(cx,cy,rx,ry,ang1,ang2); }
	void			FillArc(int cx,int cy,int rx,int ry,float ang1=0.0,float ang2=360.0)
															  { mDrawable->FillArc(cx,cy,rx,ry,ang1,ang2); }
protected:
	int				GetCanvasSize();
	
	PImageWindow  *	mOwner;				// owner window
	Display		  *	mDpy;				// pointer to X window display
	PDrawable	  *	mDrawable;			// pointer to drawable object
	Widget			mCanvas;			// X drawing canvas
	Dimension		mWidth;				// image width
	Dimension		mHeight;			// image height
	EventMask		mEventMask;			// mask for events accepted by canvas
	int				mDrawLabel;			// flag true if label should be drawn
	TextSpec	  *	mLabelText;			// pointer to label string spec (or NULL if no label)
	int				mLabelHeight;		// pixel height for label
	int             mAllowLabel;        // non-zero if label is allowed
	
	int				mDirty;				// flag set if need redrawing

private:
	Dimension		mCanvasWidth;		// full width of canvas (incl. label region)
	Dimension		mCanvasHeight;		// full height of canvas (incl. label region)
	
	static void CanvasResizeProc(Widget w, PImageCanvas *anImage, XmDrawingAreaCallbackStruct *call_data);
	static void CanvasExposeProc(Widget w, PImageCanvas *anImage, XmDrawingAreaCallbackStruct *call_data);
	static void CanvasMotionProc(Widget w, PImageCanvas *anImage, XEvent *event);
	
public:
	PImageCanvas  *	sLastTransformHits;
};

#endif // __PImageCanvas_h__
