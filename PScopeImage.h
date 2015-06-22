#ifndef __PScopeImage_h__
#define __PScopeImage_h__

#include "PImageCanvas.h"
#include "ImageData.h"

// TEMPORARY!!! (In order to use the GRAB_X, GRAB_Y enum)
#include "PHistImage.h"

#include "PMenu.h"
#include "math.h"

/*
enum {
	GRAB_X			= 0x01,
	GRAB_Y			= 0x02,
	GRAB_X_ACTIVE	= 0x10,
	GRAB_Y_ACTIVE	= 0x20,
	GRABS_ACTIVE	= (GRAB_X_ACTIVE | GRAB_Y_ACTIVE)
};
*/
enum EScopeStyle {
	kScopeStyleBars,
	kScopeStyleSteps
};

class PScale;
struct ImageData;

class PScopeImage : public PImageCanvas, public PMenuHandler
{
public:
	PScopeImage(PImageWindow *owner, Widget canvas=0, int createCanvas=1);
	virtual ~PScopeImage();
	
	virtual void	DrawSelf();
	virtual void	Resize();
	
	// NEW ++++++++
	virtual void	Listen(int message, void *dataPt);
	
	//virtual void	HandleEvents(XEvent *event);
	virtual void	SetCursorForPos(int x, int y);
	
	virtual void	DoGrab(float xmin, float xmax) { }
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
	
	void			SetYMax(long ymax)			{ mYMax = ymax; }
	
	void			SetUnderscale(long num)		{ mUnderscale = num; }
	void			SetOverscale(long num)		{ mOverscale = num; }
	void			SetStyle(EScopeStyle style)	{ mStyle = style; }
	void			SetLog(int on);
	
	void			CreateData(int numbins);
	long		  *	GetDataPt()					{ return mHistogram; }
	int				GetNumBins()				{ return mNumBins; }
	int				GetGrabFlag()				{ return mGrabFlag; }
	
	virtual void	SetScaleLimits()			{ }
	void			SetScaleLimits(float min, float max, float min_rng);
	void			SetIntegerXScale(int is_int);
	
	
	void			DrawGrid();
	void			DrawRunStop();
	void			DrawTriggerMode();
	void			DrawTriggerType();
	void			DrawTriggerCoupling();
	void			DrawTriggerHoldoff();
	void			DrawCh1();
	void			DrawCh2();
	void			DrawVpd(int ch);
	void			DrawTpd();
	void			DrawTriggerSource();
	void			DrawTriggerSlope();
	void			DrawTriggerLevel();
	void			DrawEventNumber();
	void			DrawTrigger();
	int			DisplayDiv2Y(double ydiv);
	void			DrawWaveform(int ch, int ind);
	void			ComputeLogs(int npoints);
	void			Volts2String(double vpd, char *str);
	void			Time2String(double t,char *str);
	
	virtual void		DoMenuCommand(int anID);
	
	Widget			GetFilebox() { return mFilebox; }
	
	
protected:
	void			ReadScaleValues();
	void			CheckScaleRange();

	long		  *	mHistogram;		// pointer to histogram array
	long		  *	mOverlay;		// pointer to overlay array
	double			mOverlayScale;	// scaling factor for overlay plot
	long			mOverscale;		// number of overscale entries
	long			mUnderscale;	// number of underscale entries
	int				mNumBins;		// number of histogram bins
	int			  *	mHistCols;		// colours for drawing histogram (underscale,regular,overscale)
	int				mOverlayCol;	// pixel value for overlay colour
	int				mNumCols;		// number of histogram colours
	char		  *	mLabel;			// histogram label
	PScale		  *	mXScale;		// pointer to X scale object
	PScale		  *	mYScale;		// pointer to Y scale object
	float			mXMin;			// x scale minimum
	float			mXMax;			// x scale maximum
	long			mYMax;			// y scale maximum (set by MakeHistogram())
	float			mXMinMin;		// minimum x scale minimum
	float			mXMaxMax;		// maximum x scale maximum
	float			mXMinRng;		// minimum x scale range
	Boolean			mIsLog;			// true if Y scale is logarithmic
	int				mXScaleFlag;	// flag for x scale type
	EScopeStyle		mStyle;			// style for drawing histogram
	int				mGrabFlag;		// flag for current grab
	Widget			sp_log, sp_min, sp_max;	// widgets for scale window
	
	ImageData		*mData;
	int			mRunStop;
	float 			mX[WAVEFORMSIZE],mY[WAVEFORMSIZE];
	//int			init_y_pos, /*ch_decrement, */prev_wfsize;
	//bool			firstTimeThrough;
	Widget			mFilebox;
	
private:
	static void		ScaleOKProc(Widget w, PScopeImage *hist, caddr_t call_data);
	static void		ApplyProc(Widget w, PScopeImage *hist, caddr_t call_data);
	static void		CancelProc(Widget w, Widget aShell, caddr_t call_data);	

	void			OpenFileBox();
	static void		FileOK(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data);
	static void		FileCancel(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data);
	static void		DestroyDialogProc(Widget w, Widget **dialogPtr, caddr_t call_data);
	//void			ShowWindow(int id);
};

#endif // __PScopeImage_h__
