#ifndef __PNCDHistogram_h__
#define __PNCDHistogram_h__

#include "PHistImage.h"

struct ImageData;

class PNCDHistogram : public PHistImage {
public:
	PNCDHistogram(PImageWindow *owner, Widget canvas=0);
	virtual ~PNCDHistogram();
	
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
	
	static PHistImage *GetNCDHistogram(ImageData *data);
	static void		GetHistogramLabel(ImageData *data, char *buff);
	
private:
	static float	sMinRangeFloat;
	static int		sIsAutoScale;
};

#endif // __PNCDHistogram_h__
