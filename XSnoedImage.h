#ifndef __XSnoedImage_h__
#define __XSnoedImage_h__
 
#include "PProjImage.h"
#include "xsnoed.h"

enum XSnoedImageDirtyFlags {
	kDirtyHits		= 0x02,
	kDirtyFit		= 0x04,
	kDirtyVessel	= 0x08,
	kDirtyFrame		= 0x10,
	kDirtyAxes		= 0x20,
	kDirtySun		= 0x40,
	
	kDirtyAll		= 0xfe
};

class XSnoedImage : public PProjImage
{
public:
	XSnoedImage(PImageWindow *owner, Widget canvas=0);
	virtual ~XSnoedImage();
			
	virtual void	Resize();
	virtual void	DrawSelf();
	virtual void	HandleEvents(XEvent *event)	;
	virtual void	Transform(Node *node, int num_nodes) { transform(node, &mProj, num_nodes); }
	virtual void	TransformHits();
	virtual int     FindNearestNcd();
	virtual void	Listen(int message, void *dataPt);
	
	virtual void	SetScrolls();
	virtual void	ScrollValueChanged(EScrollBar bar, int value);

	virtual void	SetToHome(int n=0);
	void			SetToSun();
	void			SetToVertex();
	
private:
	void			CalcGrab2(int x,int y);
	void			CalcGrab3(int x,int y);
	void			CalcVesselShading();
	int				FindMonteCarloVertex(int forced=0);
	void			RotationChanged();
	void			SetHitSize();
	
	Polyhedron			mFrame;					// detector frame information
	Polyhedron			mVessel;				// vessel geometry
	WireFrame			mAxes;					// coordinate axes
	MonteCarloVertex  *	mMCVertex;				// last MC vertex cursor-ed
	float				mSpinAngle;				// 'Josh' bar spin angle
	double				mMaxMagAtan;			// arctan of maximum magnification
	float				mHitSize;				// last used size of PMT hexagon
	Node				mTubeNode[6];			// nodes for PMT hexagons
	float				mGrabX,mGrabY,mGrabZ;	// 3-D mouse cursor coordinates for grab
};


#endif // __XSnoedImage_h__
