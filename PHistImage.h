#ifndef __PHistImage_h__
#define __PHistImage_h__

#include "PImageCanvas.h"

enum {
	GRAB_X			= 0x01,
	GRAB_Y			= 0x02,
	GRAB_X_ACTIVE	= 0x10,
	GRAB_Y_ACTIVE	= 0x20,
	GRABS_ACTIVE	= (GRAB_X_ACTIVE | GRAB_Y_ACTIVE)
};

enum EHistStyle {
	kHistStyleBars,
	kHistStyleSteps,
	kHistStyle2D
};

struct ImageData;
class PScale;
class PHistImage;

const int kDirtyHistCalc = 0x80;  // indicates the 2D hist may require calculating

class PHistCalc {
public:
    virtual void    DoCalc(PHistImage *hist) { };
    virtual int     GetRange(PHistImage *hist, int noffset, int nbin, int *min, int *max) { return 0; }
    
    unsigned long   GetMaxVal()     { return mMaxVal; }

protected:
    unsigned long   mMaxVal;
};

class PHistImage : public PImageCanvas {
public:
	PHistImage(PImageWindow *owner, Widget canvas=0, int createCanvas=1);
	virtual ~PHistImage();
	
	virtual void	DrawSelf();
	virtual void	Resize();
	virtual void	HandleEvents(XEvent *event);
	virtual void	SetCursorForPos(int x, int y);
	
	virtual void	DoGrab(float xmin, float xmax);
	virtual void    DoGrabY(double ymin, double ymax);
	virtual void	DoneGrab()					{ }
	
	virtual void	ResetGrab(int do_update)	{ mGrabFlag = 0; }

	virtual void	MakeHistogram()				{ }
	virtual void	SetHistogramLabel() 		{ }
	
	void			UpdateScaleInfo();
	void			CreateScaleWindow();
	
	void			SetLabel(char *str);
	char		  *	GetLabel()					{ return mLabel; }
	
	virtual float	GetScaleMin()				{ return mXMin; }
	virtual float	GetScaleMax()				{ return mXMax; }
	
	virtual void	SetScaleMin(float xmin)		{ mXMin = xmin; }
	virtual void	SetScaleMax(float xmax)		{ mXMax = xmax; }
	
	long            GetYMin()                   { return mYMin; }
	long			GetYMax()			        { return mYMax; }
	
	void            SetYMin(long ymin)          { mYMin = ymin; }
	void			SetYMax(long ymax)			{ mYMax = ymax; }
	
	void			SetUnderscale(long num)		{ mUnderscale = num; }
	void			SetOverscale(long num)		{ mOverscale = num; }
	void			SetStyle(EHistStyle style)	{ mStyle = style; }
	void            SetNumTraces(long num)      { mNumTraces = num; }
	void			SetLog(int on);
	void            SetCalcObj(PHistCalc *obj)  { mCalcObj = obj; }
	
	void			CreateData(int numbins, int twoD=0);
	void            CreateOverlay(int numbins);
	void            SetFixedBins(int on=1)      { mFixedBins = on; }
	long		  *	GetDataPt()					{ return mHistogram; }
	long          * GetOverlayPt()              { return mOverlay; }
	int				GetNumBins()				{ return mNumBins; }
	int             GetNumPix()                 { return mNumPix; }
	int				GetGrabFlag()				{ return mGrabFlag; }
	int             GetPix(long val);
	long            GetNumTraces()              { return mNumTraces; }
	
	virtual void	SetScaleLimits()			{ }
	void			SetScaleLimits(float min, float max, float min_rng);
	void			SetIntegerXScale(int is_int);
	int             CalcAutoScale(int *minPt, int *maxPt);
	void            SetAutoScale(int on)        { mAutoScale = on; }

	void            SetPlotCol(int col)         { mPlotCol = col; }
	void            SetOverlayCol(int col)      { mOverlayCol = col; }
	
protected:
	void			ReadScaleValues();
	void			CheckScaleRange();

	long		  *	mHistogram;		// pointer to histogram array
	long		  *	mOverlay;		// pointer to overlay array
	double			mOverlayScale;	// scaling factor for overlay plot
	long			mOverscale;		// number of overscale entries
	long			mUnderscale;	// number of underscale entries
	int				mNumBins;		// number of histogram bins
	int             mNumPix;        // number of pixels for 2D plot
	long            mNumTraces;     // number of traces for 2D plot
	int			  *	mHistCols;		// colours for drawing histogram (underscale,regular,overscale)
	int             mPlotCol;       // plot color for kHistStyleLines
	int				mOverlayCol;	// pixel value for overlay colour
	int				mNumCols;		// number of histogram colours
	char		  *	mLabel;			// histogram label
	PScale		  *	mXScale;		// pointer to X scale object
	PScale		  *	mYScale;		// pointer to Y scale object
	float			mXMin;			// x scale minimum
	float			mXMax;			// x scale maximum
	long			mYMin;			// y scale minimum
	long			mYMax;			// y scale maximum (set by MakeHistogram())
	float			mXMinMin;		// minimum x scale minimum
	float			mXMaxMax;		// maximum x scale maximum
	float			mXMinRng;		// minimum x scale range
	Boolean			mIsLog;			// true if Y scale is logarithmic
	int				mXScaleFlag;	// flag for x scale type
	EHistStyle		mStyle;			// style for drawing histogram
	int				mGrabFlag;		// flag for current grab
	int             mFixedBins;     // false if data is rebinned on x scale change
	Widget			sp_log, sp_min, sp_max;	// widgets for scale window
	Widget          sp_ymin, sp_ymax;
	PHistCalc     * mCalcObj;       // object used to recalculate 2D histogram
	int             mAutoScale;     // flag to automatically scale when drawing
	
private:
	static void		ScaleAutoProc(Widget w, PHistImage *hist, caddr_t call_data);
	static void		ScaleFullProc(Widget w, PHistImage *hist, caddr_t call_data);
	static void		ScaleOKProc(Widget w, PHistImage *hist, caddr_t call_data);
	static void		ApplyProc(Widget w, PHistImage *hist, caddr_t call_data);
	static void		CancelProc(Widget w, Widget aShell, caddr_t call_data);

};

#endif // __PHistImage_h__
