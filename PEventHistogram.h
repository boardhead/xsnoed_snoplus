#ifndef __PEventHistogram_h__
#define __PEventHistogram_h__

#include "PHistImage.h"

struct ImageData;

class PEventHistogram : public PHistImage {
public:
	PEventHistogram(PImageWindow *owner, Widget canvas=0);
	virtual ~PEventHistogram();
	
	virtual void	Listen(int message, void *dataPt);
	virtual void	MakeHistogram();
	virtual void	SetHistogramLabel();
	
	virtual void	DoGrab(float xmin, float xmax);
	virtual void	DoneGrab();
	virtual void	ResetGrab(int do_update);
	
	virtual void	SetScaleLimits();

	static long		GetBins(ImageData *data, float *first_pt, float *last_pt);
	static void		SetBins(ImageData *data, float first, float last);
	static void		GetLimits(ImageData *data, float *min_pt, float *max_pt, float *min_rng);
	
	static void		SetMaxCalScale(float maxT=1e3, float maxQ=1e4, float minRng=0.01);
	
	static PHistImage *GetEventHistogram(ImageData *data);
	static void		GetHistogramLabel(ImageData *data, char *buff);
	
private:
	static float	sMaxCalTime;
	static float	sMaxCalCharge;
	static float	sMinRangeFloat;
	static int		sIsAutoScale;
};

#endif // __PEventHistogram_h__
