// File:	QPmtEventRecord.h
// Author:	Phil Harvey - 12/8/98

#ifndef __QPmtEventRecord__
#define __QPmtEventRecord__

#include <time.h>
#include "TObject.h"
#include "QPMTxyz.h"
#include "QEvent.h"

struct PmtEventRecord;
struct GenericRecordHeader;

class QPmtEventRecord : public TObject {
public:
	QPmtEventRecord();
	virtual ~QPmtEventRecord();
	
	QPmtEventRecord &operator=(QEvent &anEvent) { FromQEvent(&anEvent); return(*this); }

	PmtEventRecord*	Convert(QEvent *anEvent);
	QEvent		  *	Convert(PmtEventRecord *per);
	
	void		ToQEvent(QEvent *anEvent);
	void		FromQEvent(QEvent *anEvent);
	void		Load(void *dataPtr, Int_t length);
	void		SwapBytes();
	void		SetPmtEventRecord(PmtEventRecord *per);
	void        UseExtendedFormat(Bool_t on=kTRUE) { mExtendedFormat = on; }
	Int_t		GetRunNumber();
	Int_t		GetEventIndex();
	
	PmtEventRecord*	GetPmtEventRecord()	{ return (PmtEventRecord *)mPmtRecord;	}
	Int_t			GetRecordLen();
	Text_t		  *	GetDataPtr()		{ return mBuffer; 		}
	Int_t			GetDataLen()		{ return mBufferLength;	}
	
	GenericRecordHeader *GetGenericRecordHeader() { return (GenericRecordHeader *)mBuffer; }
	
	Bool_t			AllocateBuffer(Int_t length);

private:
	
	Text_t	*mBuffer;			// pointer to associated PMT event record
	Int_t	mBufferLength;		// length of data in the buffer (bytes)
	Int_t	mBlockSize;			// byte size of memory block allocated for buffer
	time_t	root_time_zero;		// QEvent time zero (Jan 1, 1975)
	time_t	sno_time_zero;		// SNO data time zero (Jan 1, 1996)
	PmtEventRecord	*mPmtRecord;    // pointer to our PMT event record (may be in buffer)
	QEvent	*mEvent;			// event for passing back via Convert();
	Bool_t	mExtendedFormat;	// generate extended PmtEventRecord format (on by default)
	
	ClassDef(QPmtEventRecord,0)	// Converts between PmtEventRecord's and QEvent's
};



#endif
