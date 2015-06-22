#ifndef __PRecordInfoWindow_h__
#define __PRecordInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"
#include "PMenu.h"
#include "ImageData.h"


class PRecordInfoWindow : public PWindow, public PListener, public PMenuHandler {
public:
	PRecordInfoWindow(ImageData *data);
	~PRecordInfoWindow();
	
	virtual void	Show();
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *dataPt);
	virtual void	DoMenuCommand(int anID);

private:
	void			ResizeToFit();
	
	Widget			mText1, mText2;
	int				mLastIndex;
	int				mSizeDelta;
	int				mHasTime;
	int				mHasGTID;
	int             mRecLines[kNumHdrRec];
	
	static char *	sRecName[kNumHdrRec];
	static char *	sRecLabel[kNumHdrRec];
	static int		sRecLines[kNumHdrRec];
};


#endif // __PRecordInfoWindow_h__
