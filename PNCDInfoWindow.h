#ifndef __PNCDInfoWindow_h__
#define __PNCDInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"


class PNCDInfoWindow : public PWindow, public PListener {
public:
	PNCDInfoWindow(ImageData *data);
	virtual ~PNCDInfoWindow();
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);
	
protected:
	virtual char *	GetLabelString(int num);
	
private:
    void            AddString(char *str, int title=1);
    void            ClearString();
    
	PLabel			mNCDLabel;

    XmString        mString;
    int             mDataFlag;
};


#endif // __PNCDInfoWindow_h__
