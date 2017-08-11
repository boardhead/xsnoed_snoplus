#ifndef __PEventInfoWindow_h__
#define __PEventInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PExtraLabels.h"
#include "PListener.h"
#include "PLabel.h"
#include "include/Record_Info.h"

struct SPmtCount {
	PLabel		label;	// label for PMT count
	int			index;	// index for PMT type
};

#ifdef SNOPLUS
const int kNumPmtCounts	= 6;
#else
const int kNumPmtCounts	= 10;
#endif

class PEventInfoWindow : public PWindow, public PExtraLabels, public PListener {
public:
	PEventInfoWindow(ImageData *data);
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);
	
	void			SetSum(int sumOn);
	void			ShowRelativeTimes(int only_if_future=0);

	static u_int32	GetTriggerWord(aPmtEventRecord *pmtRecord);
	static int		GetRelativeTimeString(char *buff, double theTime);
	static int		GetTriggerString(char *buff, ImageData *data);
	static int		GetNhitString(char *buff, ImageData *data);
	
protected:
	virtual char *	GetLabelString(int num);
	
private:
	PLabel			tw_nhit, tw_gtid, tw_run, tw_evt;
	PLabel			tw_time, tw_diff, tw_date, tw_trig, tw_peak;
#ifdef SNOPLUS
    PLabel          tw_caen, tw_tubii, tw_fecd;
#endif
	PLabel			tw_evtnum_label, tw_time_label;
	SPmtCount		tw_pmt_count[kNumPmtCounts];
#ifdef MTC_HEX
	PLabel			tw_word[6];
#endif
	
	int				mTimeZone;
	int				mNeedFutureEvent;
};


#endif // __PEventInfoWindow_h__
