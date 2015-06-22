
// File:    QBufferedEventReader.h
// Author:  Jeremy Sylvestre - June/02

#ifndef __QBUFFEREDEVENTREADER_H__
#define __QBUFFEREDEVENTREADER_H__

//#include "TObject.h"
#include "QEventReader.h"
#include "QRecordBuffer.h"

class QEvent;

class QBufferedEventReader : /*public TObject,*/ public QEventReader
{
private:
  void Init();

public:
  QBufferedEventReader();
  QBufferedEventReader(Int_t size);
  virtual ~QBufferedEventReader();

  Bool_t  CreateBuffer(Int_t size);

  Int_t   BufferEvents(Int_t num = 1, Int_t waitForIt = 1);
  QEvent* MoveToNextEvent();
  void    ClearBuffer() { mRecBuf->DeleteAllRecords(); }

  Int_t GetBufferSize()          {if(mRecBuf) return mRecBuf->GetBufferSize();           return 0;}
  Int_t GetNRecords()            {if(mRecBuf) return mRecBuf->GetNRecords();             return 0;}
  Int_t GetCurrentEventPosition(){if(mRecBuf) return mRecBuf->GetCurrentEventPosition(); return 0;}
  Int_t GetNumFutureEvents()     {if(mRecBuf) return mRecBuf->GetNumFutureEvents();      return 0;}
  Int_t GetNumPastEvents()       {if(mRecBuf) return mRecBuf->GetNumPastEvents();        return 0;}

  Int_t GetMinPastEvents() { return mMinPastEvents; }

  QEvent* GetEventAt(Int_t offset, Int_t waitForIt = 1);
  QSNO*   GetRecordAt(Int_t offset, UInt_t* type);
  UInt_t  GetRecordTypeAt(Int_t offset);

  void SetMinPastEvents(Int_t mpe) {if(mpe<0) mMinPastEvents = 0; else mMinPastEvents = mpe;}
  void SetBufferingOfRunRecords (Bool_t setting) { mBufferRunRecs  = setting; }
  void SetBufferingOfTrigRecords(Bool_t setting) { mBufferTrigRecs = setting; }

  ClassDef(QBufferedEventReader,0)   // class to buffer SNO data and provide random access

private:
  QRecordBuffer* mRecBuf;   // The data stream buffer.

  Int_t mMinPastEvents;  // Minimum number of "past" events to keep in the buffer. default = 0.
                         // If this is too large (with "large" being measured
                         //   against the size of your buffer), you're gonna
                         //   have trouble buffering events.

  Bool_t mBufferRunRecs;  // flag - whether to buffer run records. default = true
  Bool_t mBufferTrigRecs;  // flag - whether to buffer trig records. default = true

};


#endif
