// File:	QEventReader.h
// Author:	Phil Harvey - 12/9/98

#ifndef __QEventReader__
#define __QEventReader__

#include "Rtypes.h"

class QEvent;
class QRunHeader;
class QTriggerBank;
class QSNO;

class QEventReader {
public:

	QEventReader();
	virtual	~QEventReader();
	
	virtual Bool_t	Open(char *)	{ return kFALSE; }
	virtual Bool_t	Close()					{ return kFALSE; }
	virtual Bool_t	IsOpen()				{ return kFALSE; }
	virtual Bool_t	Rewind()				{ return kFALSE; }
	virtual const Text_t*	GetName()const				{ return NULL;	 }
	
	virtual QEvent*	GetEvent(Int_t=0 )	{ return NULL;	 }
        virtual Int_t   GetBuffer(Int_t , Text_t **, char **) { return 0; }
        virtual QSNO*   GetRecord(UInt_t* type,Int_t) { if(type) *type=0; return NULL; }

	virtual void	SetEventAddress(QEvent *anEvtPtr=NULL);
	virtual QEvent*	GetEventAddress();

        virtual QRunHeader*   GetRunHeaderAddress();
        virtual QTriggerBank* GetTriggerBankAddress();

	virtual void	FreeEvent();
        virtual void    FreeRecords();


// Buffering virtual functions
        virtual Bool_t  CreateBuffer(Int_t) { return kFALSE; }
        virtual Int_t   BufferEvents(Int_t ,Int_t ) { return 0; }
        virtual QEvent* MoveToNextEvent() { return NULL; }
        virtual void    ClearBuffer() {}
        virtual QEvent* GetEventAt(Int_t ,Int_t ) { return NULL; }
        virtual QSNO*   GetRecordAt(Int_t,UInt_t* type) { if(type) *type=0; return NULL; }
        virtual UInt_t  GetRecordTypeAt(Int_t) { return 0; }
        virtual Int_t   GetBufferSize() { return 0; }
        virtual Int_t   GetNRecords()   { return 0; }
        virtual Int_t   GetCurrentEventPosition() { return -1; }
        virtual Int_t   GetNumFutureEvents() { return 0; }
        virtual Int_t   GetNumPastEvents() { return 0; }
        virtual Int_t   GetMinPastEvents() { return 0; }
        virtual void    SetMinPastEvents(Int_t ) {}
        virtual void    SetBufferingOfRunRecords(Bool_t) {}
        virtual void    SetBufferingOfTrigRecords(Bool_t) {}


protected:
	QEvent	     *mEvent;			// event pointer for GetEvent() & GetRecord()
        QRunHeader   *mRHead;           // run header pointer for GetRecord()
        QTriggerBank *mTrigBank;        // trigger bank pointer for GetRecord()

private:
	Bool_t	mEventAllocated;	// true if we created the event (then we must delete it)

	ClassDef(QEventReader,0)	// Virtual base class for objects that read QEvents
};

#ifndef NO_EVENT_READER_CREATE
QEventReader	*CreateEventReader(char *name);
#endif


#endif
