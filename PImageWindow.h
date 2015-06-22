#ifndef __PImageWindow_h__
#define __PImageWindow_h__

#include <Xm/Xm.h>
#include "PScrollingWindow.h"
#include "PImageCanvas.h"

class PNotifyRaised;

/* PImageWindow class definition */
class PImageWindow : public PScrollingWindow {
	friend class PNotifyRaised;
public:
	PImageWindow(ImageData *data);
	PImageWindow(ImageData *data, Widget shell, Widget mainPane);
	virtual	~PImageWindow();
	
	virtual void	SetShell(Widget w);
	virtual void	Show();
	virtual void	Resize() 		{ }		// called when the canvas is resized
	virtual void	UpdateSelf();
	virtual void	SetToHome(int n=0);     // set image to home position
	virtual char  *	Class()			{ return sImageWindowClass; }
	
	virtual void	SetScrolls();
	
	void			WasRaised();
	
	/* access members */
	void			SetImage(PImageCanvas *image)	{ mImage = image;		}
	PImageCanvas  *	GetImage()						{ return mImage;		}
	
	static int		IsPendingRaise()				{ return sNotifyRaised!=NULL; }
		
protected:
	PImageCanvas  *	mImage;
	int				mPrintable;		// true if window contains a printable image

	static PNotifyRaised	*sNotifyRaised;
	
private:
	void			Initialize();
	
	static void		CirculateWindProc(Widget w, PImageWindow *aWind, XEvent *event);

public:
	static char	  *	sImageWindowClass;
};

#endif // __PImageWindow_h__
