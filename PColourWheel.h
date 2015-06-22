#ifndef __PColourWheel_h__
#define __PColourWheel_h__

#include "PImageCanvas.h"


class PColourWheel : public PImageCanvas {
public:
	PColourWheel(PImageWindow *owner, Widget canvas=0, int size=100);
	virtual ~PColourWheel();
	
	virtual void	DrawSelf();
	virtual void	AfterDrawing();
	virtual void	HandleEvents(XEvent *event);
	
	void			SetIntensity(int val, int fastAnimate=0);
	void			AnimateDone();

	void			SetColourRGB(int *col3);
	void			GetColourRGB(int *col3);
	
	int				GetIntensity()	{ return mIntensity; }

private:
	void			SetCursorPos(int x, int y);
	void			CursorMoved();
	void			DrawTheWheel();
	
	void			TestColours();
	void			AllocColours();
	void			FreeColours();
	
	Pixel		  *	mColours;
	char		  *	mAllocFlags;
	int				mIntensity;
	int				mCurX, mCurY;
	int				mWheelSize;
	int				mColourType;
	float			mMaxColour[3];
	XImage		  *	mImage;
	int				mFirstTry;
	int				mDelayedUpdate;
};

#endif // __PColourWheel_h__
