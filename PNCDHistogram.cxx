/*
** class PNCDHistogram
**
** This is a very special PHistImage object because the histogram
** scale for this object manifests itself as the hit colour for all
** NCD windows.  As such, some of these methods are declared static
** to allow calculation of the histogram scale even in the absence
** of an instantiation of this class.
*/
#include <math.h>
#include "PNCDHistogram.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PScale.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"
#include "menu.h"

#define MIN_HIST_RANGE				5
#define MIN_HIST_BINS				5


float	PNCDHistogram::sMinRangeFloat	= 0.01;
int		PNCDHistogram::sIsAutoScale	= 0;

const u_int32 kShaperMax = 100000;

//---------------------------------------------------------------------------------------
// PNCDHistogram constructor
//
PNCDHistogram::PNCDHistogram(PImageWindow *owner, Widget canvas)
		  	   : PHistImage(owner,canvas)
{
	ImageData *data = owner->GetData();
	
	mNumCols = data->num_cols;
	mHistCols = new int[mNumCols];
	if (!mHistCols) quit("No memory for allocating colour array!");
	for (int i=0; i<mNumCols; ++i) {
		mHistCols[i] = NUM_COLOURS + i;
	}
}

PNCDHistogram::~PNCDHistogram()
{
	if (mGrabFlag) {
		ResetGrab(1);
	}
}

void PNCDHistogram::Listen(int message, void *dataPt)
{
	switch (message) {
	    case kMessageNCDDataTypeChanged:
	        // must reset the grab so we will recalculate hist scale
	        PHistImage::ResetGrab(0);
	        break;
		case kMessageNewEvent:
		case kMessageEventCleared:
		case kMessageColoursChanged:
		case kMessageHitsChanged:
			SetDirty();
			break;
		default:
			PImageCanvas::Listen(message, dataPt);
			break;
	}
}

PHistImage* PNCDHistogram::GetNCDHistogram(ImageData *data)
{
	if (data->mWindow[NCD_HIST_WINDOW]) {
		return (PHistImage *)((PImageWindow *)data->mWindow[NCD_HIST_WINDOW])->GetImage();
	} else {
		return NULL;
	}
}

/*
** Make histogram and set x and y scale ranges
*/
void PNCDHistogram::MakeHistogram()
{
	ImageData	*data = mOwner->GetData();
	int			i,n,num,slab;
	long		max;
	NCDHit		*nh  = data->ncdHit;
	long		bit_mask;
	long		nbin = data->ncd_bins;
	float		val, first, last, range;
	int			incr;
	
	mUnderscale = 0;
	mOverscale = 0;
	
	// set the X scale to integer if the data type is integer
	SetIntegerXScale(data->wNCDType != IDM_NCD_SHAPER_VAL);
	
	/* get histogram bins */
	nbin = GetBins(data,&first, &last);
	range = last - first;
/*
** Calculate and draw histogram
*/
	if (mHistogram && nbin!=mNumBins) {
		delete [] mHistogram;
		mHistogram = NULL;
	}
	if (!mHistogram) {
		// allocate histogram array
		mHistogram = new long[nbin];
		if (!mHistogram) {
			Printf("Out of memory for histogram\n");
			return;
		}
		mNumBins = nbin;				// set number of bins
	}
	memset(mHistogram, 0, nbin * sizeof(long));
	
	num = data->numNcds;
	max = 0;
	incr = 1;
	
	bit_mask = data->bit_mask;

	for (i=0; i<num; ++i, ++nh) {
	
	    if (!(nh->flags & ~bit_mask)) continue;
	    
		/* calculate bin number */
		// MAY WANT TO ROUND UP HERE!!!!
		val = (getNcdHitVal(data, nh) - first) * nbin / range;
 
 		// convert val to an integral bin number
		if (val < 0) {
			n = 0;
			mUnderscale += incr;
		} else if (val >= nbin) {
			n = nbin - 1;
			mOverscale += incr;
		} else {
			n = (int)val;
		}
		if ((mHistogram[n] += incr) > max) max = mHistogram[n];
	}
	/* calculate a nice even maximum value for the y axis */
	if (!(mGrabFlag & GRAB_Y)) {
		slab = 5;
		while (1) {
			if (max/slab < 10) break;
			slab *= 2;
			if (max/slab < 10) break;
			slab *= 5;
		}
		mYMax = (max/slab + 1) * slab;
		if (mYMax < 10) mYMax = 10;
		mYMin = 0;
	}
	// save the x scale range
	mXMin = first;
	mXMax = last;
}

/*
** Set histogram label string
*/
void PNCDHistogram::SetHistogramLabel()
{
	ImageData	*data = mOwner->GetData();

	if (!mLabel) {
		mLabel = new char[128];
		if (!mLabel) return;
	}
	GetHistogramLabel(data, mLabel);
}

void PNCDHistogram::GetHistogramLabel(ImageData *data, char *buff)
{
	switch (data->wNCDType) {
	    case IDM_NCD_SHAPER_VAL:
	        strcpy(buff, "NCD Shaper Value");
	        break;
        case IDM_NCD_MUX_HIT:
            strcpy(buff, "NCD MUX Hits");
            break;
        case IDM_NCD_SHAPER_HIT:
            strcpy(buff, "NCD Shaper Hits");
            break;
        case IDM_NCD_SCOPE_HIT:
            strcpy(buff, "NCD Scope Hits");
            break;
    }
}

void PNCDHistogram::DoGrab(float xmin, float xmax)
{
	mXMin = xmin;
	mXMax = xmax;
	CheckScaleRange();
}

// called after a grab is completed
void PNCDHistogram::DoneGrab()
{
	ImageData *data = mOwner->GetData();
	
	data->log_scale = mIsLog;	// keep log scale setting current
	
	// must re-calculate hit values and redraw images if colour scale changed
	SetBins(data, mXMin, mXMax);

//	calcHitVals(data);
	sendMessage(data,kMessageHitsChanged);
}

void PNCDHistogram::ResetGrab(int do_update)
{
	// let the base class reset the grab flag
	PHistImage::ResetGrab(do_update);
	
	if (do_update) {
		// reset colour scale to defaults if this is an auto-scale
		if (sIsAutoScale) {
			ImageData *data = mOwner->GetData();
			SetBins(data, mXMin, mXMax);
//			calcHitVals(data);
			sendMessage(data,kMessageHitsChanged);
		} else {
			// otherwise just redraw the histogram
			SetDirty();
		}
	}
}

/* get histogram bin parameters */
long PNCDHistogram::GetBins(ImageData *data, float *first_pt, float *last_pt)
{
	long		nbin;
	float		first, last, range;
	PHistImage	*hist = GetNCDHistogram(data);

	/* handle manual scales */
	if (hist && (hist->GetGrabFlag() & GRAB_X)) {
		*first_pt = first = hist->GetScaleMin();
		*last_pt  = last  = hist->GetScaleMax();
		nbin = hist->GetNumBins();
		range = last - first;
		// must recalculate number of bins for some datatypes
		switch (data->wNCDType) {
		    case IDM_NCD_MUX_HIT:
		    case IDM_NCD_SHAPER_HIT:
		    case IDM_NCD_SCOPE_HIT:
				nbin = (long)range;
				if (nbin < data->ncd_bins) {
					if (nbin < 5) nbin = 5;
				} else {
					/* limit the total number of bins */
					while (nbin > data->ncd_bins) {
						nbin /= 2;
					}
				}
				break;
		}
		return(nbin);
	}
	nbin = data->ncd_bins;
	sIsAutoScale = 0;
	
	/* get histogram scales */
	switch (data->wNCDType) {
	    case IDM_NCD_SHAPER_VAL:
 			first = data->shaper_min;
 			last = data->shaper_max;
			break;
		default: {
        	NCDHit  *nh = data->ncdHit;
        	int     num = data->numNcds;
        	first = last = 0;
		    for (int i=0; i<num; ++i, ++nh) {
		        float val = getNcdHitVal(data, nh);
		        if (val > last) last = val;
		    }
		    ++last; // set max scale to one beyond maximum
		    if (last < 10) last = 10;
		    sIsAutoScale = 1;
            nbin = (long)last;
            // recalculate number of bins for integer datatypes
            if (nbin < data->ncd_bins) {
                if (nbin < 5) nbin = 5;
            } else {
                /* limit the total number of bins */
                while (nbin > data->ncd_bins) {
                    nbin /= 2;
                }
            }
		}   break;
	}
	// update current scale limits if the event histogram
	if (hist && (hist->GetScaleMin()!=first || hist->GetScaleMax()!=last)) {
		hist->SetScaleMin(first);
		hist->SetScaleMax(last);
		hist->UpdateScaleInfo();
	}

	*first_pt = first;
	*last_pt  = last;
	
	return(nbin);
}
 

/* set histogram range */
void PNCDHistogram::SetBins(ImageData *data, float first, float last)
{
	/* set histogram scales */
	switch (data->wNCDType) {
        case IDM_NCD_SHAPER_VAL:
            data->shaper_min = first;
            data->shaper_max = last;
            break;
	}
}
 
void PNCDHistogram::SetScaleLimits()
{
	float min, max, min_rng;
	
	GetLimits(mOwner->GetData(), &min, &max, &min_rng);
	
	mXMinMin = min;
	mXMaxMax = max;
	mXMinRng = min_rng;
}

/* get histogram maximum range */
void PNCDHistogram::GetLimits(ImageData *data,float *min_pt, float *max_pt, float *min_rng)
{
	float		xmin, xmax, rmin;
	
	rmin = MIN_HIST_RANGE;	// default minimum range
	
	/* get histogram scales */
	switch (data->wNCDType) {
	    case IDM_NCD_SHAPER_VAL:
	        xmin = 0;
	        xmax = kShaperMax;
	        rmin = 10;
	        break;
	    case IDM_NCD_MUX_HIT:
	    case IDM_NCD_SHAPER_HIT:
	    case IDM_NCD_SCOPE_HIT: {
        	NCDHit  *nh = data->ncdHit;
        	int     num = data->numNcds;
        	xmin = xmax = 0;
		    for (int i=0; i<num; ++i, ++nh) {
		        float val = getNcdHitVal(data, nh);
		        if (val > xmax) xmax = val;
		    }
		    ++xmax; // allow scale to be set one beyond maximum
		    if (xmax < 10) xmax = 10;
		}   break;
	}
	*min_pt = xmin;
	*max_pt = xmax;
	*min_rng = rmin;
}
 



