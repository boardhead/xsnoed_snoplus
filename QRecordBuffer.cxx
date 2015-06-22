///////////////////////////////////////////////////////////////////////////////
//
//  Allows buffering of and random access to data from the SNO data stream.
//  See QBufferedEventReader for a full description
//
///////////////////////////////////////////////////////////////////////////////

//*-- Author :  Jeremy Sylvest - June/02

#include <stdio.h>
#include "QSNO.h"
#include "QEvent.h"
#include "QRunHeader.h"
#include "QTriggerBank.h"
#include "QRecordBuffer.h"
#include "include/Record_Info.h"


ClassImp(QRecordBuffer)


QRecordBuffer::QRecordBuffer()
{
  // constructor
  this->Init();
}


QRecordBuffer::QRecordBuffer(Int_t size)
{
  // constructor with buffer size
  this->Init();
  this->SetBufferSize(size);
}


QRecordBuffer::~QRecordBuffer()
{
  // destructor
  this->DeleteAllRecords();
  if(mRecords) delete [] mRecords;
}


void QRecordBuffer::Init()
{
  // initialize
  mRecords         = NULL;
  mBufferSize      =    0;
  mNRecords        =    0;
  mCurrentEventPos =   -1;
  mBufferStartPtr  =    0;
}


void QRecordBuffer::DeleteAllRecords()
{
  // clear out the buffer
  if(mRecords){
    for(Int_t i = 0; i < mBufferSize; i++){
      if(mRecords[i].Data) delete mRecords[i].Data;
    }
  }
  mNRecords        =  0;
  mCurrentEventPos = -1;
  mBufferStartPtr  =  0;
}


Bool_t QRecordBuffer::SetBufferSize(Int_t size)
{
  // Warning: You will lose all data in the buffer if you call this method
  //          more than once (including the call from
  //          ::QRecordBuffer(Int_t size)
  this->DeleteAllRecords();
  if(mRecords) delete [] mRecords;
  if(size > 0){
    mRecords = new RecBufferEntry[size];
    if(mRecords){
      mBufferSize = size;
      for(Int_t i = 0; i < mBufferSize; i++){
        mRecords[i].Data = NULL;
        mRecords[i].Type = 0;
      }
    }
  } else {
    mBufferSize = 0;
  }
  mNRecords        =  0;
  mCurrentEventPos = -1;
  mBufferStartPtr  =  0;
  return mRecords;
}


void QRecordBuffer::PushRecord(QSNO* rec, UInt_t type)
{
  // Push a record onto the buffer, dropping the oldest record if necessary.
  // This method should be easy to modify to handle more record types when/if
  //   associated QSno objects for the different record types are coded up.
  //   --> see the docs for QBufferedEventReader for a full description

  QSNO* qTemp = NULL;
  if(mRecords){
    switch(type){
      case(RUN_RECORD)  :   qTemp = new QRunHeader;    break;
      case(TRIG_RECORD) :   qTemp = new QTriggerBank;  break;
      case(PMT_RECORD)  :   qTemp = new QEvent;        break;
    }
    if(!qTemp){
      printf("QRecordBuffer: mem alloc error, record lost\n");
    } else {

      if(mNRecords == mBufferSize){
        if(mRecords[mBufferStartPtr].Data) delete mRecords[mBufferStartPtr].Data;
        ++mBufferStartPtr %= mBufferSize;
        mNRecords--;
        mCurrentEventPos--;
        if(mCurrentEventPos < -1) mCurrentEventPos = -1;
      }

      QRunHeader*   qrunrec   = NULL;
      QTriggerBank* qtrigbank = NULL;
      QEvent*       qev       = NULL;
      QRunHeader*   qRRTemp;
      QTriggerBank* qTBTemp;
      QEvent*       qEVTemp;
    
      switch(type){
        case RUN_RECORD :
          qrunrec = (QRunHeader*) rec;
          qRRTemp = (QRunHeader*) qTemp;
          *qRRTemp = *qrunrec;
          break;
        case TRIG_RECORD:
          qtrigbank = (QTriggerBank*) rec;
          qTBTemp   = (QTriggerBank*) qTemp;
          *qTBTemp = *qtrigbank;
          break;
        case PMT_RECORD :
          qev     = (QEvent*) rec;
          qEVTemp = (QEvent*) qTemp;
          *qEVTemp = *qev;
          break;
      }
      
      Int_t newRecPtr = mBufferStartPtr - (mBufferSize - mNRecords);
      if(newRecPtr < 0) newRecPtr += mBufferSize;
      mRecords[newRecPtr].Data   = qTemp;
      mRecords[newRecPtr].Type = type;
      mNRecords++;
    }
  }

}


QEvent* QRecordBuffer::MoveToNextEvent()
{
  // This method moves mCurrentEventPos to the next event record in the buffer
  //   and returns a pointer to that event.
  // If there are no more event records in the buffer, mCurrentEventPos is
  //   left where it was and returns NULL.

  QEvent* theEvent = NULL;

  if(mRecords){
    Int_t aPtr;
    for(Int_t i = mCurrentEventPos + 1; i < mNRecords; i++){
      aPtr = GetIndex(i);
      if(mRecords[aPtr].Type == PMT_RECORD){
        mCurrentEventPos = i;
        theEvent = (QEvent*) mRecords[aPtr].Data;
        break;
      }
    }
  }

  return theEvent;

}


QEvent* QRecordBuffer::GetEventAt(Int_t offset)
{
  // This method will return the event at an offset of "offset" events from
  //   the event referenced by mCurrentEventPos.
  // ie. It "collapses" the buffer by ignoring all non-PMT records and then
  //   finds the corresponding event at the offset position.
  // If no such event exists, NULL is returned.
  // Returns NULL if MoveToNextEvent has not been called at least once.

  QEvent* theEvent = NULL;

  if(mRecords){

    if(mCurrentEventPos >= 0){

      if(offset == 0){
        theEvent = (QEvent*) mRecords[GetIndex(mCurrentEventPos)].Data;
      } else {
        Int_t effective_offset = 0;
        Int_t step = (offset > 0) ? 1 : -1;
        Int_t aPtr;
        for(Int_t i = mCurrentEventPos + step; (i>=0) && (i<mNRecords); i+=step){
          aPtr = GetIndex(i);
          if(mRecords[aPtr].Type == PMT_RECORD) effective_offset += step;
          if(effective_offset == offset){
            theEvent = (QEvent*) mRecords[aPtr].Data;
          }
        }
      }

    }

  }

  return theEvent;

}


QSNO* QRecordBuffer::GetRecordAt(Int_t offset, UInt_t* type)
{
  // This method will return the record at the specified offset with respect
  //   to the mCurrentEventPos, and will return the type of the record in the
  //   UInt_t referenced by "type".
  // If the offset is outside the bounds of the buffer, NULL and 0 are
  //   returned.

  QSNO* theRecord = NULL;
  UInt_t theRecType = 0;

  if(mRecords){
    Int_t recPtr = mCurrentEventPos + offset;
    if( (recPtr >=0) && (recPtr < mNRecords) ){
      theRecord  = mRecords[GetIndex(recPtr)].Data;
      theRecType = mRecords[GetIndex(recPtr)].Type;
    }
  }

  if(type) *type = theRecType;
  return theRecord;
}


UInt_t QRecordBuffer::GetRecordTypeAt(Int_t offset)
{
  // Similar to ::GetRecordAt() method, but only returns the
  //   record type.

  UInt_t theRecType = 0;

  if(mRecords){
    Int_t recPtr = mCurrentEventPos + offset;
    if( (recPtr >=0) && (recPtr < mNRecords) ){
      theRecType = mRecords[GetIndex(recPtr)].Type;
    }
  }

  return theRecType;

}


Int_t QRecordBuffer::GetEventPosition(Int_t offset)
{
  // This method will return the position in the buffer of the event at an
  //   offset of "offset" events from the event referenced by mCurrentEventPos
  // ie. It "collapses" the buffer by ignoring all non-PMT records and then
  //   finds the corresponding event at the offset position.
  // It will return -1 if no such event exists.

  Int_t rv = -1;

  if(mRecords){

    if(mCurrentEventPos >= 0){

      if(offset == 0){
        rv = mCurrentEventPos;
      } else {
        Int_t effective_offset = 0;
        Int_t step = (offset > 0) ? 1 : -1;
        for(Int_t i = mCurrentEventPos + step; (i>=0) && (i<mNRecords); i+=step){
          if(mRecords[GetIndex(i)].Type == PMT_RECORD) effective_offset += step;
          if(effective_offset == offset){
            rv = i;
          }
        }
      }

    }

  }

  return rv;
}


Int_t QRecordBuffer::GetNumFutureEvents()
{
  // Returns the number of buffered events that appear at a position greater
  //   than mCurrentEventPos.
  Int_t nevents = 0;
  if(mRecords){
    for(Int_t i = mCurrentEventPos + 1; i < mNRecords; i++){
      if(mRecords[GetIndex(i)].Type == PMT_RECORD) nevents++;
    }
  }
  return nevents;
}


Int_t QRecordBuffer::GetNumPastEvents()
{
  // Returns the number of buffered events that appear at a position less than
  //   mCurrentEventPos.
  Int_t nevents = 0;
  if(mRecords){
    for(Int_t i = 0; i < mCurrentEventPos; i++){
      if(mRecords[GetIndex(i)].Type == PMT_RECORD) nevents++;
    }
  }
  return nevents;
}

Int_t QRecordBuffer::GetIndex(Int_t position)
{
  // Return an array index given a buffer position
  return ( (position + mBufferStartPtr)%mBufferSize );
}
