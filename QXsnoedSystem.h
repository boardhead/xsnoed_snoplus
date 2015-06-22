// File:	QXSnoedSystem.h
// Author:	Phil Harvey - 11/26/98

#ifndef __QXsnoedSystem__
#define __QXsnoedSystem__

#include "TUnixSystem.h"
#include "PSpeaker.h"

/* QXsnoedSystem class definition */
class QXsnoedSystem : public TUnixSystem, public PSpeaker {
public:
				 QXsnoedSystem();
	virtual 	~QXsnoedSystem();
	
	virtual void DispatchOneEvent(Bool_t pendingOnly);
	virtual int  UnixSelect(UInt_t nfds, TFdSet *readready, TFdSet *writeready, Long_t timeout);
	
	void		 SetExitRootWithXsnoed(Bool_t doExit) { mExitRootWithXsnoed = doExit; }
	
private:
	Bool_t		 mExitRootWithXsnoed;	// if set, exits root on xsnoed quit
	
	ClassDef(QXsnoedSystem,1)	// Integrates xsnoed GUI into ROOT system
};
	
#endif
