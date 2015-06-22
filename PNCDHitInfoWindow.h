#ifndef __PNCDHitInfoWindow_h__
#define __PNCDHitInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"


class PNCDHitInfoWindow : public PWindow, public PListener {
public:
	PNCDHitInfoWindow(ImageData *data);
	~PNCDHitInfoWindow();
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);

private:
	void			ClearEntries();
	void			SetHitXY();
	void			ManageXY(int manage);
	void			ResizeToFit();
	
	PLabel          mString, mMuxHits, mShaperHits, mScopeHits;
	PLabel          mShaperVal, mMuxBus, mMuxBox, mMuxChan;
	PLabel          mHV, mShaperSlot, mShaperAddr, mShaperChan;
	PLabel          mScopeChan, mPreamp, mPDSBoard, mPDSChan;
	PLabel          mCounters, mXY[2], mXYLabels[2];
	int				mLastNum;
};


#endif // __PNCDHitInfoWindow_h__
