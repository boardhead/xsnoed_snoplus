// File:	QDispatch.h
// Author:	Phil Harvey - 12/8/98

#ifndef NO_DISPATCH

#ifndef __QDispatch__
#define __QDispatch__

#include "TObject.h"
#include "QBufferedEventReader.h"
#include "QPmtEventRecord.h"

#define QDISP_MAXBUFSIZE 130000

const short	kMaxHostNameSize = 128;

class QEvent;

class QDispatch : public TObject, public QBufferedEventReader
{
protected:
        void SwapInt32_Buffer(Int_t num);

public:
	QDispatch();
        QDispatch(Int_t bufferSize);
	virtual	~QDispatch();

	virtual Bool_t	Open(char *hostname=NULL) { return Open(hostname,"w RAWDATA w RECHDR"); }
	virtual Bool_t	Open(char *hostname, char *records);
	virtual Bool_t	Close();
	virtual Bool_t	IsOpen()		{ return mIsConnected;  }
	virtual const Text_t*	GetName()const		{ return mHostName;		}

	Bool_t			PutData(char *tag, void *data, Int_t datalen);
	Bool_t			PutString(char *tag, char *string);

	Int_t           GetData(char *tag, void *data, Int_t buffsize, Int_t waitForIt=1);

	Int_t			GetRunNumber()	{ return mRunNumber;	}
	Int_t			GetEventIndex()	{ return mEventIndex;	}

	virtual QEvent *GetEvent(Int_t waitForIt=1);
	virtual Bool_t	PutEvent(QEvent *anEvent);

    virtual QSNO*   GetRecord(UInt_t* type, Int_t waitForIt=1);

protected:
    Text_t  mDispBuf[QDISP_MAXBUFSIZE];
	Bool_t	mIsConnected;				// true if we are currently connected to dispatcher
	Text_t	mHostName[kMaxHostNameSize];// name of dispatcher (default = "localhost")
	QPmtEventRecord	mPmtRecord;			// object to convert from PmtEventRecord to QEvent
	Int_t	mRunNumber;					// run number of last event received
	Int_t	mEventIndex;				// index number of last event received
	
	static Int_t	sRecursionCount;	// count to tell if we are recursively calling dispatch routine
										// (may happen if the user ctrl-C's out of a function)

	ClassDef(QDispatch,0)		// Provides access to dispatched SNO events
};

extern QDispatch	*gDispatch;

#endif

#endif // NO_DISPATCH

