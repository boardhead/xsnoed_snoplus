/*
** class PEventHistogram
**
** This is a very special PHistImage object because the histogram
** scale for this object manifests itself as the hit colour for all
** image windows.  As such, some of these methods are declared static
** to allow calculation of the histogram scale even in the absence
** of an instantiation of this class.
*/
#include <math.h>
#include "PEventHistogram.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PScale.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"
#include "menu.h"

#define MIN_HIST_RANGE				5
#define MIN_HIST_BINS				5


// values for converting calibrated histogram scales
// (approx averages from B.Frati's distributions)
#define SPE_QHS	20	// channels for Qhs <spe> centroid
#define SPE_QHL	40	// channels for Qhl <spe> centroid
#define SPE_QLX	2	// channels for Qlx <spe> centroid

float 	PEventHistogram::sMaxCalTime 	= 1000.0;
float 	PEventHistogram::sMaxCalCharge	= 10000.0;
float	PEventHistogram::sMinRangeFloat	= 0.01;
int		PEventHistogram::sIsAutoScale	= 0;


//---------------------------------------------------------------------------------------
// PEventHistogram constructor
//
PEventHistogram::PEventHistogram(PImageWindow *owner, Widget canvas)
		  	   : PHistImage(owner,canvas)
{
	ImageData *data = owner->GetData();
	
	mNumCols = data->num_cols;
	mHistCols = new int[mNumCols];
	mOverlayCol = NUM_COLOURS + mNumCols;
	mIsLog = data->log_scale;	// restore log scale setting
	if (!mHistCols) quit("No memory for allocating colour array!");
	for (int i=0; i<mNumCols; ++i) {
		mHistCols[i] = NUM_COLOURS + i;
	}
}

PEventHistogram::~PEventHistogram()
{
	if (mGrabFlag) {
		ResetGrab(1);
	}
}

void PEventHistogram::Listen(int message, void *dataPt)
{
	switch (message) {
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

PHistImage* PEventHistogram::GetEventHistogram(ImageData *data)
{
	if (data->mWindow[HIST_WINDOW]) {
		return (PHistImage *)((PImageWindow *)data->mWindow[HIST_WINDOW])->GetImage();
	} else {
		return NULL;
	}
}

/*
** Make histogram and set x and y scale ranges
*/
void PEventHistogram::MakeHistogram()
{
	ImageData	*data = mOwner->GetData();
	int			i,n,num,slab;
	long		max;
	HitInfo		*hi  = data->hits.hit_info;
	long		bit_mask;
	long		nbin = data->hist_bins;
	float		val, first, last, range;
	int			incr;
	
	mUnderscale = 0;
	mOverscale = 0;
	
	// set the X scale to integer if the data type is integer
	SetIntegerXScale(isIntegerDataType(data));
	
	/* get histogram bins */
	nbin = GetBins(data,&first, &last);
	range = last - first;
/*
** Calculate and draw histogram
*/
	if (mHistogram && nbin!=mNumBins) {
		delete [] mHistogram;
		delete [] mOverlay;
		mHistogram = NULL;
		mOverlay = NULL;
	}
	if (!mHistogram || !mOverlay) {
	    if (mHistogram) delete [] mHistogram;
	    if (mOverlay) delete [] mOverlay;
		// allocate histogram and overlay arrays
		mHistogram = new long[nbin];
		mOverlay = new long[nbin];
		if (!mHistogram || !mOverlay) {
			Printf("Out of memory for histogram\n");
			return;
		}
		mNumBins = nbin;				// set number of bins
	}
	memset(mHistogram, 0, nbin * sizeof(long));
	memset(mOverlay, 0, nbin * sizeof(long));
	
	num = data->hits.num_nodes;
	max = 0;
	incr = 1;
	
	// decide for ourselves whether the hit is under/overscale
	// because we may be in the process of changing the scale
	bit_mask = data->bit_mask & ~(HIT_UNDERSCALE | HIT_OVERSCALE);
	
	for (i=0; i<num; ++i, ++hi) {
	
		if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */

		/* calculate bin number  */
		val = (getHitValPad(data, hi) - first) * nbin / range;
 
 		// convert val to an integral bin number
		if (val < 0) {
			// ignore underscale hits if masked out
			if (data->bit_mask & HIT_UNDERSCALE) continue;
			n = 0;
			if (!(hi->flags & HIT_DISCARDED)) mUnderscale += incr;
		} else if (val >= nbin) {
			// ignore overscale hits if masked out
			if (data->bit_mask & HIT_OVERSCALE) continue;
			n = nbin - 1;
			if (!(hi->flags & HIT_DISCARDED)) mOverscale += incr;
		} else {
			n = (int)val;
		}
		if ((mHistogram[n] += incr) > max) max = mHistogram[n];
		// keep track of discarded hits in each bin
		if (hi->flags & HIT_DISCARDED) mOverlay[n] += incr;
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
void PEventHistogram::SetHistogramLabel()
{
	ImageData	*data = mOwner->GetData();

	if (!mLabel) {
		mLabel = new char[128];
		if (!mLabel) return;
	}
	GetHistogramLabel(data, mLabel);
}

void PEventHistogram::GetHistogramLabel(ImageData *data, char *buff)
{
	strcpy(buff,data->dispName);
	switch (data->wDataType) {
		case IDM_DELTA_T:
			if (data->nrcon) strcat(buff," (ns from fit)");
			else strcat(buff," (ns from center)");
			break;
#ifdef OPTICAL_CAL
		case IDM_NHIT:
			if (data->oca) strcat(buff," Ratio");
			break;
#endif
		case IDM_TAC:
			if (data->wCalibrated != IDM_UNCALIBRATED) strcat(buff," (ns)");
			else strcat(buff," (raw)");
			break;
		case IDM_QHS:
		case IDM_QHL:
		case IDM_QLX:
		case IDM_QHL_QHS:
			if (data->real_cal_scale) strcat(buff," (pe)");
			else if (data->wCalibrated != IDM_UNCALIBRATED) strcat(buff," (ped corr)");
			else strcat(buff," (raw)");
			break;
		default:
			break;
	}
}

void PEventHistogram::DoGrab(float xmin, float xmax)
{
	mXMin = xmin;
	mXMax = xmax;
	CheckScaleRange();
}

// called after a grab is completed
void PEventHistogram::DoneGrab()
{
	ImageData *data = mOwner->GetData();
	
	data->log_scale = mIsLog;	// keep log scale setting current
	
	// must re-calculate hit values and redraw images if colour scale changed
	SetBins(data, mXMin, mXMax);

	calcHitVals(data);
	sendMessage(data,kMessageHitsChanged);
}

void PEventHistogram::ResetGrab(int do_update)
{
	// let the base class reset the grab flag
	PHistImage::ResetGrab(do_update);
	
	if (do_update) {
		// reset colour scale to defaults if this is an auto-scale
		if (sIsAutoScale) {
			ImageData *data = mOwner->GetData();
			SetBins(data, mXMin, mXMax);
			calcHitVals(data);
			sendMessage(data,kMessageHitsChanged);
		} else {
			// otherwise just redraw the histogram
			SetDirty();
		}
	}
}

/* get histogram bin parameters */
long PEventHistogram::GetBins(ImageData *data, float *first_pt, float *last_pt)
{
	long		nbin;
	float		first, last, range;
	long		factor;
	PHistImage	*hist = GetEventHistogram(data);
	
	/* handle manual scales */
	if (hist && (hist->GetGrabFlag() & GRAB_X)) {
		*first_pt = first = hist->GetScaleMin();
		*last_pt  = last  = hist->GetScaleMax();
		nbin = hist->GetNumBins();
		range = last - first;
		// must recalculate number of bins for some datatypes
		switch (data->wDataType) {
			case IDM_NHIT:
#ifdef OPTICAL_CAL
				if (data->oca) break;
#endif
				nbin = (long)range;
				if (nbin < data->hist_bins) {
					if (nbin < 5) nbin = 5;
				} else {
					/* limit the total number of bins */
					while (nbin > data->hist_bins) {
						nbin /= 2;
					}
				}
				break;
			case IDM_DISP_CRATE:
			case IDM_DISP_CARD:
			case IDM_DISP_CHANNEL:
			case IDM_DISP_CELL:
				nbin = (long)range;
				break;
		}
		return(nbin);
	}
	nbin = data->hist_bins;
	sIsAutoScale = 0;
	
	/* get histogram scales */
	switch (data->wDataType) {
		case IDM_TAC:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
 				first = data->cal_tac_min;
 				last  = data->cal_tac_max;
 			} else {
 				first = data->tac_min;
 				last  = data->tac_max;
 			}
			break;
		case IDM_QHS:
			if (data->real_cal_scale) {
				first = data->cal_qhs_min / SPE_QHS;
	 			last  = data->cal_qhs_max / SPE_QHS;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				first = data->cal_qhs_min;
	 			last  = data->cal_qhs_max;
	 		} else {
				first = data->qhs_min;
	 			last  = data->qhs_max;
	 		}
			break;
		case IDM_QHL:
			if (data->real_cal_scale) {
				first = data->cal_qhl_min / SPE_QHL;
	 			last  = data->cal_qhl_max / SPE_QHL;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				first = data->cal_qhl_min;
	 			last  = data->cal_qhl_max;
	 		} else {
				first = data->qhl_min;
	 			last  = data->qhl_max;
	 		}
			break;
		case IDM_QLX:
			if (data->real_cal_scale) {
				first = data->cal_qlx_min / SPE_QLX;
	 			last  = data->cal_qlx_max / SPE_QLX;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				first = data->cal_qlx_min;
	 			last  = data->cal_qlx_max;
	 		} else {
				first = data->qlx_min;
	 			last  = data->qlx_max;
	 		}
			break;
		case IDM_QHL_QHS:
			if (data->real_cal_scale) {
				first = data->cal_qhl_qhs_min / SPE_QHS;
	 			last  = data->cal_qhl_qhs_max / SPE_QHS;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				first = data->cal_qhl_qhs_min;
	 			last  = data->cal_qhl_qhs_max;
	 		} else {
				first = data->qhl_qhs_min;
	 			last  = data->qhl_qhs_max;
	 		}
			break;
		case IDM_NHIT:
#ifdef OPTICAL_CAL
			if (data->oca) {
				first = data->cal_nhit_min;
				last = data->cal_nhit_max;
				break;
			}
#endif
			first = 0;
			/* set range to the max number of hits per tube */
			last = data->max_sum_nhit + 1;
			if (last < MIN_HIST_BINS) last = MIN_HIST_BINS;
			/* limit the total number of bins */
			factor = 1;
			for (nbin=(int)(last-first); nbin>data->hist_bins; nbin/=2) { 
				factor *= 2;
			}
			/* adjust last bin to avoid integer binning problems */
			last = first + nbin * factor;
			sIsAutoScale = 1;
			break;
		case IDM_CMOS_RATES:
			first = data->cmos_rates_min;
			last  = data->cmos_rates_max;
			break;
		case IDM_DISP_CRATE:
			first = 0;
			last = nbin = NUM_SNO_CRATES;
			sIsAutoScale = 1;
			break;
		case IDM_DISP_CARD:
			first = 0;
			last = nbin = NUM_CRATE_CARDS;
			sIsAutoScale = 1;
			break;
		case IDM_DISP_CHANNEL:
			first = 0;
			last = nbin = NUM_CARD_CHANNELS;
			sIsAutoScale = 1;
			break;
		case IDM_DISP_CELL:
			first = 0;
			last = nbin = NUM_CHANNEL_CELLS;
			sIsAutoScale = 1;
			break;
		case IDM_DELTA_T:
      		first = data->delta_t_min;
			last = data->delta_t_max;
			break;
		default:
			first = data->extra_min;
			last = data->extra_max;
			sIsAutoScale = 0;
			break;
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
void PEventHistogram::SetBins(ImageData *data, float first, float last)
{
	/* set histogram scales */
	switch (data->wDataType) {
		case IDM_TAC:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
 				data->cal_tac_min = first;
 				data->cal_tac_max = last;
 			} else {
 				data->tac_min = (int)first;
 				data->tac_max = (int)last;
 			}
			break;
		case IDM_QHS:
			if (data->real_cal_scale) {
				data->cal_qhs_min = first * SPE_QHS;
	 			data->cal_qhs_max = last * SPE_QHS;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				data->cal_qhs_min = first;
	 			data->cal_qhs_max = last;
	 		} else {
				data->qhs_min = (int)first;
	 			data->qhs_max = (int)last;
	 		}
			break;
		case IDM_QHL:
			if (data->real_cal_scale) {
				data->cal_qhl_min = first * SPE_QHL;
	 			data->cal_qhl_max = last * SPE_QHL;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				data->cal_qhl_min = first;
	 			data->cal_qhl_max = last;
	 		} else {
				data->qhl_min = (int)first;
	 			data->qhl_max = (int)last;
	 		}
			break;
		case IDM_QLX:
			if (data->real_cal_scale) {
				data->cal_qlx_min = first * SPE_QLX;
	 			data->cal_qlx_max = last * SPE_QLX;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				data->cal_qlx_min = first;
	 			data->cal_qlx_max = last;
	 		} else {
				data->qlx_min = (int)first;
	 			data->qlx_max = (int)last;
	 		}
			break;
		case IDM_QHL_QHS:
			if (data->real_cal_scale) {
				data->cal_qhl_qhs_min = first * SPE_QHS;
	 			data->cal_qhl_qhs_max = last * SPE_QHS;
			} else if (data->wCalibrated != IDM_UNCALIBRATED) {
				data->cal_qhl_qhs_min = first;
	 			data->cal_qhl_qhs_max = last;
	 		} else {
				data->qhl_qhs_min = (int)first;
	 			data->qhl_qhs_max = (int)last;
	 		}
			break;
 		case IDM_DELTA_T:
     		data->delta_t_min = first;
			data->delta_t_max = last;
			break;
		case IDM_NHIT:
#ifdef OPTICAL_CAL
			if (data->oca) {
				data->cal_nhit_min = first;
				data->cal_nhit_max = last;
				break;
			}
#endif
			// fall through!
		case IDM_DISP_CRATE:
		case IDM_DISP_CARD:
		case IDM_DISP_CHANNEL:
		case IDM_DISP_CELL:
			break;	// do nothing for now (autoscaling)
		case IDM_CMOS_RATES:
			data->cmos_rates_min = (int)first;
			data->cmos_rates_max = (int)last;
			break;
		default:
			data->extra_min = first;
			data->extra_max = last;
			break;
	}
}
 
void PEventHistogram::SetScaleLimits()
{
	float min, max, min_rng;
	
	GetLimits(mOwner->GetData(), &min, &max, &min_rng);
	
	mXMinMin = min;
	mXMaxMax = max;
	mXMinRng = min_rng;
}

void PEventHistogram::SetMaxCalScale(float maxT, float maxQ, float minRng)
{
	sMaxCalTime = maxT;
	sMaxCalCharge = maxQ;
	sMinRangeFloat = minRng;
}

/* get histogram maximum range */
void PEventHistogram::GetLimits(ImageData *data,float *min_pt, float *max_pt, float *min_rng)
{
	float		xmin, xmax, rmin;
	
	rmin = MIN_HIST_RANGE;	// default minimum range
	
	/* get histogram scales */
	switch (data->wDataType) {
		case IDM_TAC:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				xmin = -sMaxCalTime;
				xmax = sMaxCalTime;
				rmin = sMinRangeFloat;
 			} else {
 				xmin = 0;
 				xmax = 4096;
 			}
			break;
		case IDM_QHS:
		case IDM_QHL:
		case IDM_QLX:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				xmin = -sMaxCalCharge;
				xmax = sMaxCalCharge;
				rmin = sMinRangeFloat;
	 		} else {
				xmin = 0;
				xmax = 4096;
	 		}
			break;
		case IDM_QHL_QHS:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				xmin = -sMaxCalCharge;
				xmax = sMaxCalCharge;
				rmin = sMinRangeFloat;
	 		} else {
				xmin = -4096;
				xmax = 4096;
	 		}
			break;
		case IDM_NHIT:
#ifdef OPTICAL_CAL
			if (data->oca) {
				xmin = 0;
				xmax = 100;
				rmin = sMinRangeFloat;
				break;
			}
#endif
			xmin = 0;
			/* set range to the max number of hits per tube */
			xmax = data->max_sum_nhit + 1;
			if (xmax < 5) xmax = 5;
			break;
		case IDM_CMOS_RATES:
			xmin = 0;
			xmax = 10e6;
			break;
		case IDM_DISP_CRATE:
			xmin = 0;
			xmax = NUM_SNO_CRATES;
			break;
		case IDM_DISP_CARD:
			xmin = 0;
			xmax = NUM_CRATE_CARDS;
			break;
		case IDM_DISP_CHANNEL:
			xmin = 0;
			xmax = NUM_CARD_CHANNELS;
			break;
		case IDM_DISP_CELL:
			xmin = 0;
			xmax = NUM_CHANNEL_CELLS;
			break;
		case IDM_DELTA_T:
      		xmin = -sMaxCalTime;
      		xmax = sMaxCalTime;
			rmin = sMinRangeFloat;
			break;
		default:
			xmin = -1e6;
			xmax = 1e6;
			rmin = sMinRangeFloat;
			break;
	}
	*min_pt = xmin;
	*max_pt = xmax;
	*min_rng = rmin;
}
 



