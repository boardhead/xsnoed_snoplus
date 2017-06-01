//
// File:		PCaenWindow.h
//
// Description:	Window to display CAEN digitized trigger sums
//
// Revisions:	2012/03/06 - PH Created
//
#ifndef __PCaenWindow_h__
#define __PCaenWindow_h__

#include <Xm/Xm.h>
#include "PImageWindow.h"
#include "PListener.h"
#include "PMenu.h"
#include "ImageData.h"
#include "PHistImage.h"

class PNCDScopeImage;

class PCaenWindow : public PImageWindow, public PListener, public PMenuHandler, public PHistCalc {
public:
	PCaenWindow(ImageData *data);
	virtual ~PCaenWindow();
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);
    virtual void    DoMenuCommand(int anID);
	
    void            DoneGrab(PNCDScopeImage *hist);
    
    virtual void    DoCalc(PHistImage *hist);
    virtual int     GetRange(PHistImage *hist, int noffset, int nbin, int *min, int *max);

private:
    void            SetChannels(int chan_mask);
    int             GetActiveChannels();
    
    Widget          mChannel[kMaxCaenChannels];    // channel canvas widgets
	PNCDScopeImage	*mHist[kMaxCaenChannels];
};


#endif // __PCaenWindow_h__
