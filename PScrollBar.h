#ifndef __PScrollBar_h__
#define __PScrollBar_h__

#include <Xm/Xm.h>

const int	kScrollMax	= 10000;	// maximum value for all scrollbars
const int	kSliderSize	= 1000;		// size of slider

enum EScrollBar {
	kScrollLeft,
	kScrollRight,
	kScrollBottom,
	kNumScrollBars
};

class PScrollingWindow;

class PScrollHandler {
public:
	PScrollHandler() { }
    virtual ~PScrollHandler() { }
	
	virtual void		ScrollValueChanged(EScrollBar bar, int value) { }
};


class PScrollBar {
public:
	PScrollBar(Widget container, EScrollBar bar, char *name, Arg *wargs, int n, PScrollHandler *handler=NULL);
	virtual ~PScrollBar();
	
	Widget				GetWidget()							{ return mScrollWidget; }
	EScrollBar			GetType()							{ return mType;			}
	PScrollHandler	  *	GetHandler()						{ return mHandler;		}
	void				SetHandler(PScrollHandler *hand)	{ mHandler = hand;		}
	
private:
	Widget				mScrollWidget;
	EScrollBar			mType;
	PScrollHandler	  *	mHandler;
};

#endif // __PScrollBar_h__
