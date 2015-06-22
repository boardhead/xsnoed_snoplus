#ifndef __PProjImage_h__
#define __PProjImage_h__

#include <Xm/Xm.h>
#include "PImageCanvas.h"
#include "matrix.h"

#define THIN_LINE_WIDTH		0.15
#define THICK_LINE_WIDTH	0.5

// default event mask for projection images
const EventMask kProjImageEvents = PointerMotionMask | ButtonPressMask |
								   ButtonReleaseMask | LeaveWindowMask;
								   
enum EAngleFlags {
	kAngleTheta = 0x01,
	kAnglePhi	= 0x02,
	kAngleGamma	= 0x04,
	kAngleAll	= kAngleTheta | kAnglePhi | kAngleGamma
};

struct Projection {
	Matrix3		rot;				/* rotation matrix */
	Matrix3		inv;				/* inverse rotation matrix */
	Vector3		pt;					/* projection point */
	int			xscl;				/* pixel radius of unit sphere (x) */
	int			yscl;				/* pixel radius of unit sphere (y) */
	float		mag;				/* magnification factor for image */
	int			xcen,ycen;			/* center of image in window coordinates */
	int			xsiz,ysiz;			/* width and height of drawing window */
	int			inside;				/* flag for projection point inside detectors */
	int			in_ves;				/* flag for inside vessel */
	int			neck_first;			/* flag to draw neck first */
	float		proj_min;			/* minimum z position of projection point */
	float		proj_max;			/* maximum z position of projection point */
	float		proj_screen;		/* z position of projection screen */
	int			proj_type;			/* integer specifying type of projection */
	float		theta;				/* projection theta rotation (optional) */
	float		phi;				/* projection phi rotation (optional) */
	float		gamma;				/* projection gamma rotation (optional) */
};

struct Node;

/* class definition */
class PProjImage : public PImageCanvas {
public:
	PProjImage(PImageWindow *owner, Widget canvas, EventMask eventMask=kProjImageEvents);
	virtual ~PProjImage();

	virtual void	Resize();
	virtual void	HandleEvents(XEvent *event);
	virtual void	Transform(Node *node, int num_nodes) { }
	virtual void	TransformHits();
	virtual void	SetScrolls();
	virtual void	ScrollValueChanged(EScrollBar bar, int value);
	virtual void	SetToHome(int n=0);
	virtual void	DrawAngles(int horiz=0, int angleFlags=kAngleTheta|kAnglePhi);
	virtual void	Listen(int message, void *data);
	
	Projection	  *	GetProj()		    { return &mProj;	}

	virtual int		FindNearestHit();
	virtual int     FindNearestNcd()    { return 0; }
	long			HiddenHitMask();

protected:
	
	int				HandleButton3(XEvent *event);
	
	Projection		mProj;			/* projection */
	float			mImageSizeX;	/* half width of image (user units) */
	float			mImageSizeY;	/* half height of image (user units) */
	int				mScaleProportional;	/* non-zero if scale must be the same in X and Y */
	int				mMarginPix;		/* pixel margin outside image */
	float			mMarginFactor;	/* factor to be applied to image size for additional margin */
	float			mMinMagAtan;	/* atan() of minimum magnification */
	float			mDiffMagAtan;	/* atan(max_mag) - atan(min_mag) */
	int				mInvisibleHits;	/* mask for hit types not displayed */
};


#endif // __PProjImage_h__
