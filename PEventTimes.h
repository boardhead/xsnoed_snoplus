#ifndef __PEventTimes_h__
#define __PEventTimes_h__

#include "PHistImage.h"


class PEventTimes : public PHistImage {
public:
	PEventTimes(PImageWindow *owner, Widget canvas=0);
	virtual ~PEventTimes();
	
	void			CheckUpdate();
	virtual void	Listen(int message, void *dataPt);

	virtual void	MakeHistogram();
	
	virtual void	DoGrab(float xmin, float xmax);
	virtual void	ResetGrab(int do_update);

	double			GetLastUpdateTime()	{ return mLastUpdateTime; }
	
protected:
	double	mLastUpdateTime;	/* time of last event time plot update */
	
	int		mDirtyPending;
};

#endif // __PEventTimes_h__
