
// File:    QRecordBuffer.h
// Author:  Jeremy Sylvestre - June/02

#ifndef __QRECORDBUFFER_H__
#define __QRECORDBUFFER_H__

#include "TObject.h"

class QSNO;
class QEvent;

struct RecBufferEntry {
  QSNO*  Data;
  UInt_t Type;
};

class QRecordBuffer : public TObject
{
private:
  void  Init();
  Int_t GetIndex(Int_t position); 
                                 
public:
  QRecordBuffer();
  QRecordBuffer(Int_t size);
  virtual ~QRecordBuffer();
  
  void    DeleteAllRecords();
  Bool_t  SetBufferSize(Int_t size);
  void    PushRecord(QSNO* rec, UInt_t type);
  QEvent* MoveToNextEvent();

  QEvent* GetEventAt(Int_t offset);
  QSNO*   GetRecordAt(Int_t offset, UInt_t* type);
  UInt_t  GetRecordTypeAt(Int_t offset);
  Int_t   GetNRecords() { return mNRecords; }
  Int_t   GetBufferSize() { return mBufferSize; }
  Int_t   GetCurrentEventPosition() { return mCurrentEventPos; }
  Int_t   GetEventPosition(Int_t offset);
  Int_t   GetNumFutureEvents();
  Int_t   GetNumPastEvents();

  ClassDef(QRecordBuffer,0)   // class to buffer SNO data and provide random access

private:
  RecBufferEntry* mRecords;  // array of RecBufferEntry structs (circular buffer scheme)

  Int_t mBufferSize;  // size of the buffer
  Int_t mNRecords;  // number of records in the buffer
  Int_t mBufferStartPtr;  // offset into mRecords array against which all "positions" are calculated
  Int_t mCurrentEventPos;  // position of "current" event in buffer
    
};


#endif
