#ifndef __PMapImage_h__
#define __PMapImage_h__

#include "PProjImage.h"
#include "PMenu.h"
#include "matrix.h"

struct MenuStruct;

class PMapImage : public PProjImage, public PMenuHandler {
public:
	PMapImage(PImageWindow *owner, Widget canvas=0);
	virtual ~PMapImage();
	
	virtual void	DrawSelf();
	
	virtual void	Listen(int message, void *dataPt);
	virtual void	SetScrolls();
	virtual void	ScrollValueChanged(EScrollBar bar, int value);

	virtual void	TransformHits();

	virtual void	DoMenuCommand(int anID);
	
	void			AddMenuItem();
	void			SetProjection(int proj_type);
	void			TransformHits(Vector3 vec, Matrix3 rot1);
	void			SetFitRelative(int rel);
	int				IsFitRelative()		{ return mRelativeToFit; }
	
private:
	XSegment	  *	AddProjLine(XSegment *sp,XSegment *segments,Node *n0,Node *n1,
						   		Vector3 vec,Matrix3 rot1,Projection *proj,int nsplit);
						   		
	int				mRelativeToFit;		// non-zero if plot relative to fit
	int				mSizeOption;		// size menu option
	int				mShapeOption;		// shape menu option
	int				mProjType;			// projection type
	int				mSplitThreshold;	// maximum times to split a line for drawing fit
};

#endif // __PMapImage_h__
