#include "PEventTimes.h"
#include "ImageData.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PZdabFile.h"
#include "CUtils.h"
#include <math.h>

const short	 kNumHistBins	= 50;
const double kUpdateTime	= 0.5;


//---------------------------------------------------------------------------------------
// PEventHistogram constructor
//
PEventTimes::PEventTimes(PImageWindow *owner, Widget canvas)
		   : PHistImage(owner,canvas)
{
	ImageData *data = owner->GetData();
	
	SetLabel("Uncut Event Time Distribution (sec)");
	
	mDirtyPending = 0;
	mNumCols = 3;
	mHistCols = new int[mNumCols];
	mHistCols[0] = NUM_COLOURS;
	mHistCols[1] = NUM_COLOURS + data->num_cols/2 + 1;
	mHistCols[2] = NUM_COLOURS + data->num_cols - 1;
	mOverlayCol = NUM_COLOURS + data->num_cols - 2;
	mXMin = 0;
	mXMax = 100;
	mHistogram = new long[kNumHistBins];
	mOverlay = new long[kNumHistBins];
	mNumBins = kNumHistBins;
	mXScaleFlag = 0;
	mLastUpdateTime = 0;
	mXMinMin = -1000;
	mXMaxMax = 1000;
	mXMinRng = 10e-9;
	
	data->mEventTimes = this;
	
	data->mSpeaker->AddListener(this);
}

PEventTimes::~PEventTimes()
{
	mOwner->GetData()->mEventTimes = NULL;
}

void PEventTimes::Listen(int message, void *dataPt)
{
	ImageData *data;
	
	switch (message) {
		case kMessageHistoryChanged:
			data = mOwner->GetData();
			if (data->history_size[HISTORY_FUTURE] == FUTURE_SIZE) {
				// draw event times histogram if our history has filled
				SetDirty();
			} else {
				mDirtyPending = 1;	// wait for a while before updating
			}
			break;
			
		case kMessageHistoryChangeEnd:
		case kMessageNewEvent:
			SetDirty();
			break;
			
		case kMessageHistoryWillClear:
			data = mOwner->GetData();
			// set the dirty flag for the event times
			if (data->history_size[HISTORY_FUTURE] || data->history_size[HISTORY_ALL]) {
				SetDirty();
			}
			break;
			
	    case kMessageColoursChanged:
	        SetDirty();
	        break;

		default:
			PHistImage::Listen(message, dataPt);
			break;
	}
}

void PEventTimes::DoGrab(float xmin, float xmax)
{
	if (mGrabFlag & GRAB_X) {
		mXMin = xmin;
		mXMax = xmax;
		CheckScaleRange();
	}
}

void PEventTimes::ResetGrab(int do_update)
{
	if (do_update) SetDirty();
	PHistImage::ResetGrab(do_update);
}

/* update if we are dirty and it is time to update */
void PEventTimes::CheckUpdate()
{
	if (mDirtyPending && double_time()-mLastUpdateTime>kUpdateTime) {
		SetDirty();	// time to do a redraw
	}
}

void PEventTimes::MakeHistogram()
{
	int				i, n1, n2, cur_event_num, zero_index=0;
	int				num_entries, buff_num;
	long			n, ymax;
	double			t, start_time, end_time, zero_time=0, x_rng;
	HistoryEntry	*entry;
	ImageData		*data = mOwner->GetData();
	
	// reset pending dirty since we are drawing now
	mDirtyPending = 0;
	
	memset(mHistogram, 0, kNumHistBins * sizeof(long));
	memset(mOverlay, 0, kNumHistBins * sizeof(long));
	mOverscale = 0;
	mUnderscale = 0;
	ymax = 0;
	mLastUpdateTime = double_time();
	
	n1 = data->history_size[HISTORY_ALL];
	
	/* find first timestamped (non-orphan) event in history buffer */
	for (i=0; i<n1; ++i) {
		entry = data->history_buff[HISTORY_ALL][i];
		t = get50MHzTime((PmtEventRecord *)(entry + 1));
		if (t) {
			zero_time = t;
			zero_index = i;	// save index of zero-time event
			break;
		}
	}
	// history buffer has no event with valid time stamp
	// - don't bother to draw histogram in this case
	if (i>= n1) return;
	
	n2 = data->history_size[HISTORY_FUTURE];
	
	if (mGrabFlag & GRAB_X) {
		start_time = mXMin + zero_time;
		end_time = mXMax + zero_time;
		x_rng = end_time - start_time;
	} else {
		start_time = zero_time;	// initialize start time
		// search for earliest time in 'all' history buffer
		for (i=n1-1; i>zero_index; --i) {
			entry = data->history_buff[HISTORY_ALL][i];
			if (!entry) continue;
			t = get50MHzTime((PmtEventRecord *)(entry + 1));
			if (!t) continue;
			if (t > zero_time) {
				t -= get50MHzTimeMax();		// make wrap-free wrt zero time
			}
			start_time = t;
			break;
		}
		// search for latest time in 'future' history buffer
		end_time = zero_time;	// initialize end time
		for (i=n2-1; i>=0; --i) {
			entry = data->history_buff[HISTORY_FUTURE][i];
			if (!entry) continue;
			t = get50MHzTime((PmtEventRecord *)(entry + 1));
			if (!t) continue;
			if (t < zero_time) {
				t += get50MHzTimeMax();	// make wrap-free wrt zero time
			}
			end_time = t;
			break;
		}
		
		// calculate time range for histogram x scale	
		x_rng = end_time - start_time;
		
		// bring minimum time range out to 200 ns
		if (x_rng < 200e-9) {
			start_time -= 100e-9 - (x_rng/2);
			end_time += 100e-9 - (x_rng/2);
			x_rng = end_time - start_time;
		}
		// make scales relative to latest event time
		mXMin = start_time - zero_time;
		mXMax = end_time - zero_time;
	}
	
	cur_event_num = getCurrentEventIndex(data);
	
	// initialize to number of entries for 'all' buffer
	num_entries = n1;

	// loop over 'all' and 'future' history buffers
	for (buff_num=1; ; ) {
	
		// bin events in history buffer
		for (i=0; i<num_entries; ++i) {
		
			// get 50MHz time for this event
			entry = data->history_buff[buff_num][i];
			if (!entry) continue;
			t = get50MHzTime((PmtEventRecord *)(entry + 1));
			if (!t) continue;	// ignore orphans
			
			// make wrap-free with respect to zero time
			if (i < zero_index) {
				// event is later that zero time
				if (t < zero_time) {
					t += get50MHzTimeMax();
				}
			} else {
				// event is earlier than (or equal to) zero time
				if (t > zero_time) {
					t -= get50MHzTimeMax();
				}
			}
			n = (long)((t - start_time) * (kNumHistBins - 1) / x_rng + 0.5);
			if (n < 0) {
				if (i != cur_event_num) ++mUnderscale;
				n = 0;
			} else if (n >= kNumHistBins) {
				if (i != cur_event_num) ++mOverscale;
				n = kNumHistBins - 1;
			}
			if (++mHistogram[n] > ymax) {
				ymax = mHistogram[n];
			}
			if (i==cur_event_num) ++mOverlay[n];
		}
		if (++buff_num > 2) break;	// increment buffer number and break if done
		
		// shift cur_event_num for future events (which were negative history_evt number)
		cur_event_num = -1 - cur_event_num;
		
		// shift zero_index so all events in history are 
		// (events are always later that zero time since they are in the future)
		zero_index = 9999;
		
		// change to number of entries for 'future' buffer
		num_entries = n2;
	}
	
	// set y-scale maximum unless the y scale is currently grabbed
	if (!(mGrabFlag & GRAB_Y)) {
		if (ymax < 10) ymax = 10;
		mYMax = ymax;
	}
}

