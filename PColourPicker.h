#ifndef __PColourPicker_h__
#define __PColourPicker_h__

#include "PImageCanvas.h"


class PColourPicker : public PImageCanvas {
public:
	PColourPicker(PImageWindow *owner, Widget canvas=0);
	virtual ~PColourPicker();
	
	virtual void	DrawSelf();
	virtual void	AfterDrawing();
	virtual void	Listen(int message, void *dataPt);
	virtual void	HandleEvents(XEvent *event);
	
	void			ApplyCurrentColours();
	int				RevertColours();

	void			SetColourRGB(int *col3);
	void			GetColourRGB(int *col3);
	void			SetColourNumber(int colNum);
	int				GetColourNumber()  { return mColourNum; }
	
private:
	void			LoadCurrentColours();
	void			FreeColours(int flags);
	int				SetCurrentColours();
	void			GetXY(int colNum, int *x, int *y);
	
	int				mColourNum;		// current colour number
	XColor		  *	mColours;
	XColor		  *	mRevertColours;	// revert colours for both sets (2 * NUM_COLOURS)
	char		  *	mAllocFlags;
	int				mCurrentSet;	// current colour set (0 or 1)
	int				mAllocErr;
	
	static int		sLastColourNum;
};

#endif // __PColourPicker_h__
