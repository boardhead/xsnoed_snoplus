///////////////////////////////////////////////////////////////////////////////
//
//  Allows buffering of and random access to data from the SNO data stream.
//
//
//    ----------------------
//    QBUFFEREDEVENTREADER:
//    ----------------------
//
//    Note the distinction between event records and non-event records in the
//    following documentation -- all buffering actions are performed with respect
//    to the number of events in the buffer, and non-event records are buffered
//    as a side-effect of these actions. You can specify which types of non-event
//    records are buffered (subject to which non-event record types are actually
//    handled by the code -- currently only RUN and TRIG records).
//
//    The QBufferedEventReader maintains a circular buffer of records received from
//    the dispatcher.  It keeps two pointers into the buffer.  One points at the
//    "current" event -- all records before this are consider to be in the "past",
//    and all records after this are considered to be in the "future". The second
//    pointer keeps track of the oldest record in the buffer -- this is where the
//    snake eats its tail.
//
//    The current-event-pointer can only be manipulated through the
//    ::MoveToNextEvent() method.  All other event/record offsets are measured
//    against this pointer.
//
//    If you make the QBufferedEventReader buffer more records, either by explicitly
//    telling it to or by a requesting an event sufficiently in the future that it
//    is not in the buffer, it will buffer records until the desired number of
//    "future" events is in the buffer, throwing out old records as necessary.
//    However, there is a setting to keep a certain minimum number of "past" events
//    in the buffer -- if this is set greater than zero, then the
//    QBufferedEventReader may not always be able to buffer as many "future" events
//    as you ask it to.  Even if this setting is set to zero, the "current" event
//    is never discarded, so if you have a small buffer and ask for an event far
//    into the future, you'll probably get NULL back.
//
//    Below is a sample event loop that looks for a burst of events -- 10 events in
//    10 milliseconds -- inside a buffer of 100 records.
//
//    {
//
//
//      QDispatch* disp = new QDispatch(100);
//
//      disp->SetMinPastEvents(10);  // we'd like at least 10 past events in the
//                                   // buffer at all times
//
//      disp->BufferEvents(10,1);    // buffer 10 events, the second argument
//                                   // means "wait-for-it"
//
//      QEvent* current_event;
//
//      current_event = disp->MoveToNextEvent();  // you need to do one of these to
//                                                // move the current-event-pointer
//                                                // into the record buffer
//
//      QEvent* ev1,ev2;
//      while(1){
//
//      // look for a burst in a 10-event window around the current event
//        for(Int_t i=0;i<10;i++){
//          ev1 = disp->GetEventAt(-9+i);
//          ev2 = disp->GetEventAt(0+i);
//          if(ev1 && ev2){
//            if((ev1->GetEvent_id() != 0) && (ev2->GetEvent_id() != 0)){
//              // ignore orphans
//              Double_t diff = ev2->GetGtr_time() - ev1->GetGtr_time(); // 50MHz
//              if(diff < 1e7){
//                HandleBurst(disp,current_event) // some routine to
//                                                // process the burst
//                break;
//              }
//            }
//          }
//        }
//
//        // before moving to next event we need to handle any
//        // non-event records in between current and next events
//        Bool_t handling_nonevents = kTRUE;
//        Int_t offset = 0;
//        while(handling_nonevents){
//          // note: this loop can potentially become an infinite one
//          //       if your buffer is too small and your minimum number
//          //       of past events kept is too large and there are a lot
//          //       of non-event records coming through
//          UInt_t rec_type;
//          QSNO* rec = disp->GetRecordAt(++offset, &rec_type);
//          if(rec = NULL){
//            disp->BufferEvents(10,1);
//          } else {
//            if(rec_type == PMT_RECORD){
//              handling_nonevents = kFALSE;
//            } else {
//              HandleNonPMTRecord(rec,rec_type);  // some routine to process record
//            }
//          }
//        }
//
//        // move to next event
//        current_event = disp->MoveToNextEvent();
//
//      }
//
//    }
//
//
//    -------------------------------------------------------
//    ADDING RECORD TYPES TO QBUFFEREDEVENTREADER/QDISPATCH:
//    -------------------------------------------------------
//
//    (1) Create a definition (record ID and struct) for your record in
//        $QSNO_ROOT/include/include/Record_Info.h if one doesn't already exist.
//
//    (2) Create a QSNO class under the qtree module corresponding to your record
//        if one doesn't already exist.  (Use QRunHeader and QTriggerBank as
//        templates).  Your class must be dervied from the QSNO base class.
//
//    (3) Add code to the QDispatch::GetRecord() function to deal with the new
//        record type.
//
//    (4) If you want to be able to buffer this record type, you'll have to add
//        code (in numerous places) to the QBufferedEventReader and QRecordBuffer
//        classes to handle the new record type.  It should be fairly obvious from
//        the existing code where modifications are needed -- just look for code
//        that shuffles around QRunRecord,QTriggerBank,QEvent objects.
//
//
///////////////////////////////////////////////////////////////////////////////

//*-- Author :  Jeremy Sylvest - June/02

#include <stdio.h>
#include "QEvent.h"
//#include "QRunHeader.h"
//#include "QTriggerBank.h"
#include "QBufferedEventReader.h"
#include "include/Record_Info.h"


ClassImp(QBufferedEventReader)


QBufferedEventReader::QBufferedEventReader()
{
  // constructor
  this->Init();
}


QBufferedEventReader::QBufferedEventReader(Int_t size)
{
  // constructor with size of buffer to create
  this->Init();
  this->CreateBuffer(size);
}


QBufferedEventReader::~QBufferedEventReader()
{
  // destructor
  if(mRecBuf) delete mRecBuf;
}


void QBufferedEventReader::Init()
{
  // initialize
  mRecBuf         =  NULL;
  mBufferRunRecs  = kTRUE;
  mBufferTrigRecs = kTRUE;
  mMinPastEvents  =     0;
}


Bool_t QBufferedEventReader::CreateBuffer(Int_t size)
{
  // Create the buffer with a given size
  // Warning: You will lose all data in the buffer if you call this method
  //          more than once, or if you used the constructor that takes a
  //          buffer-size integer argument and then call this method.
  //
  if(mRecBuf) delete mRecBuf;
  mRecBuf = new QRecordBuffer(size);
  return mRecBuf;
}


Int_t QBufferedEventReader::BufferEvents(Int_t num, Int_t waitForIt)
{
  // This method will ask the lower level implementation
  //   (e.g. QDispatch) for data records until the specified number of
  //   PMT records have been buffered, buffering all other records received
  //   as well.
  // Method will block until the required number of events is found if
  //   waitForIt is true.
  // Method returns the number of events actually pushed onto the buffer. If
  //   the return value is less than "num", it means the method returned early
  //   because (a) the buffer is full and the maximum number of "past" records
  //   dropped off the other end of the buffer was reached (as determined by
  //   the mMinPastEvents setting; OR (b) the lower level implementation of
  //   ::GetRecord() returned a NULL pointer, probably meaning the data source
  //   has no new records waiting.

  Int_t rv = 0;

  if(mRecBuf){

    // figure out how many records we can safely push onto the buffer
    Int_t lastSafeEvPos = mRecBuf->GetEventPosition(-1*mMinPastEvents);
    Int_t bufSpace      = (mRecBuf->GetBufferSize() - mRecBuf->GetNRecords())
                            + ( (lastSafeEvPos>0) ? lastSafeEvPos : 0 );

    Bool_t noRecs = kFALSE;
    while( (!noRecs) && (rv < num) && (bufSpace > 0) ){
      UInt_t type = 0;
      QSNO* rec = this->GetRecord(&type,waitForIt);
      if(rec){
        switch(type){
          case RUN_RECORD  :
            if(mBufferRunRecs){
              mRecBuf->PushRecord(rec,type);
              bufSpace--;
            }
            break;
          case TRIG_RECORD :
            if(mBufferTrigRecs){
              mRecBuf->PushRecord(rec,type);
              bufSpace--;
            }
            break;
          case PMT_RECORD  :
            mRecBuf->PushRecord(rec,type);
            bufSpace--;
            rv++;
            break;
        }
      } else {
        noRecs = kTRUE;
      }
    }
  }

  return rv;
}


QEvent* QBufferedEventReader::MoveToNextEvent()
{
  // Returns the next event and sets the data stream buffer's
  //   "current event" pointer to point to this event.
  // If this method returns NULL you need to call ::BufferEvents.
  // Before calling this method you should check for and handle
  //   any/all non-PMT records that are buffered between the current
  //   and the next event.
  QEvent* theEvent = NULL;

  if(mRecBuf){
    theEvent = mRecBuf->MoveToNextEvent();
  }
  
  return theEvent;
}


QEvent* QBufferedEventReader::GetEventAt(Int_t offset, Int_t waitForIt)
{
  // Returns event at position "offset" with respect to the "current" event.
  //   ( See QRecordBuffer::GetEventAt() )
  // If offset>0 and offset > number of "future" events buffered, method attempts
  //   to buffer new records until enough "future" events are buffered.
  
  QEvent* theEvent = NULL;
  if(mRecBuf){
    theEvent = mRecBuf->GetEventAt(offset);
    if((!theEvent) && (offset > 0)){
      Int_t numNewEventsNeeded = offset - mRecBuf->GetNumFutureEvents();
      if(this->BufferEvents(numNewEventsNeeded,waitForIt) >= numNewEventsNeeded){
        theEvent = mRecBuf->GetEventAt(offset);
      }
    }
  }
  return theEvent;
}


QSNO* QBufferedEventReader::GetRecordAt(Int_t offset, UInt_t* type)
{
  // Returns record at position "offset" with respect to the "current" event.
  // However, unlike ::GetEventAt(), method does not attempt to buffer new records
  //   if offset>0 and offset > number of "future" records buffered.

  QSNO* theRec = NULL;
  UInt_t recType = 0;

  if(mRecBuf) theRec = mRecBuf->GetRecordAt(offset, &recType);

  if(type) *type = recType;
  return theRec;
}


UInt_t QBufferedEventReader::GetRecordTypeAt(Int_t offset)
{
  // Similar to ::GetRecordAt(), but only returns the type of record.
  UInt_t recType = 0;
  if(mRecBuf) recType = mRecBuf->GetRecordTypeAt(offset);
  return recType;
}

