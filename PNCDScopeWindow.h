//
// File:		PNCDScopeWindow.h
//
// Description:	Window to display NCD event information in text form
//
// Revisions:	08/26/03 - PH Created
//
#ifndef __PNCDScopeWindow_h__
#define __PNCDScopeWindow_h__

#include <Xm/Xm.h>
#include "PImageWindow.h"
#include "PListener.h"
#include "PMenu.h"

const int   kNumNcdScopes = 2;
const int   kNumScopeChannels = 4;

class PNCDScopeImage;

class PNCDScopeWindow : public PImageWindow, public PListener, public PMenuHandler {
public:
	PNCDScopeWindow(ImageData *data);
	virtual ~PNCDScopeWindow();
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);
    virtual void    DoMenuCommand(int anID);
	
    int             IsGoodLogAmpParms(int n);
    void            DoneGrab(PNCDScopeImage *hist);

private:
    void            SetChannels(int chan_mask);
    int             GetSelectedChannels();
    
    Widget          mChannel[kNumScopeChannels];    // channel canvas widgets
	PNCDScopeImage	*mHist[kNumScopeChannels];
	int             mIsCalibrated;
	int             mChannelDisp;
};


#endif // __PNCDScopeWindow_h__
