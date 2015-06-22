///////////////////////////////////////////////////////////////////////////////////////////
// ROOT interface to dispatcher functions for accessing the real time SNO event stream.  //	
// Also provides conversion of events into QEvent format.                                //
// - if the user kills a dispatcher call via ctrl-C, QDispatch will automatically        //
//   disconnect and reconnect on the next call to avoid generating broken pipe errors.   //
///////////////////////////////////////////////////////////////////////////////////////////

//*-- Author :	Phil Harvey - 12/8/98

#ifndef NO_DISPATCH

#include <unistd.h>
#include <string.h>
#include "QDispatch.h"
#include "QEvent.h"
#include "QRunHeader.h"
#include "QTriggerBank.h"
#include "QPmtEventRecord.h"
#include "include/dispatch.h"
#include "include/Record_Info.h"

ClassImp(QDispatch)

QDispatch	*gDispatch = NULL;

Int_t	QDispatch::sRecursionCount = 0;
	

QDispatch::QDispatch()
{
	// QDispatch constructor
	
	strcpy(mHostName,"localhost");
	// null terminate for the case where the max length name is set
	mHostName[kMaxHostNameSize-1] = 0;
	mRunNumber = 0;
	mEventIndex = 0;
}


QDispatch::QDispatch(Int_t bufferSize) : QBufferedEventReader(bufferSize)
{
	// QDispatch constructor
	
	strcpy(mHostName,"localhost");
	// null terminate for the case where the max length name is set
	mHostName[kMaxHostNameSize-1] = 0;
	mRunNumber = 0;
	mEventIndex = 0;
}


QDispatch::~QDispatch()
{
	// QDispatch destructor
	
	if (Close()) {	// disconnect if necessary
		printf("QDispatch: Disconnected from %s\n",mHostName);
	}
	// reset gDispatch global if it was 'this'
	if (gDispatch == this) gDispatch = NULL;
}

#ifdef USE_NANOSLEEP
static void usleep2(unsigned long usec)
{
	struct timespec ts;
	ts.tv_sec = usec / 1000000UL;
	ts.tv_nsec = (usec - ts.tv_sec * 1000000UL) * 1000;
	nanosleep(&ts,NULL);
}
#else
#define usleep2(a) usleep(a)
#endif


Bool_t QDispatch::Open(char *hostname,char *records)
{
	// Connect to dispatcher
	// - returns true and sets gDispatch to this if successfully connected
	
	if (IsOpen()) {
		printf("QDispatch: Disconnecting from %s\n",mHostName);
		Close();		// make sure we are disconnected
		usleep2(500000);		// wait a bit after disconnecting (0.5 sec)
	}
	
	// save our new hostname (if not null)
	if (hostname != NULL) {
		strncpy(mHostName, hostname, kMaxHostNameSize-1);
	}
	
	printf("QDispatch: Connecting to dispatcher %s tags: %s\n",mHostName,records);
	Int_t rc = init_disp_link(mHostName,records);
	
	if (rc >= 0) {
		mIsConnected = kTRUE;
		send_me_always();
		my_id("QDISPATCH");
		// set global dispatch object to this so we will be automatically
		// disconnected when the user exits ROOT
		gDispatch = this;
		sRecursionCount = 0;	// reset recursion count on new connection
	} else {
		printf("QDispatch: Error connecting to dispatcher %s\n",mHostName);
	}
	
	return(mIsConnected);
}

Bool_t QDispatch::Close()
{
	// Disconnect from dispatcher
	// - returns true if we were previously connected to the dispatcher
	
	Bool_t wasConnected = mIsConnected;
	
	if (IsOpen()) {
		drop_connection();
		mIsConnected = kFALSE;
	}
	
	return(wasConnected);
}

Bool_t QDispatch::PutString(char *tag, char *string)
{
	// Send a string to the dispatcher with the specified tag
	return PutData(tag, string, strlen(string));
}

Bool_t QDispatch::PutData(char *tag, void *data, Int_t datalen)
{
	// Send data to the dispatcher with the specified tag
	Bool_t	isOK;
	
	++sRecursionCount;
	Int_t rc = put_fulldata(tag,data,datalen);
	--sRecursionCount;
	if (rc <= 0) {
		printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
		Close();
		isOK = kFALSE;
	} else {
		isOK = kTRUE;
	}
	return(isOK);
}

Bool_t QDispatch::PutEvent(QEvent *anEvent)
{
	// Send a QEvent to the dispatcher
	Bool_t	rtnVal = kFALSE;
	
	if (anEvent) {
		if (!mIsConnected || sRecursionCount) {
			Open();
		}
		if (mIsConnected) {
			mPmtRecord.FromQEvent(anEvent);
			mPmtRecord.SwapBytes();	// must swap bytes before dispatching
			Text_t *ptr = mPmtRecord.GetDataPtr();
			if (ptr) {
				rtnVal = PutData("RAWDATA",ptr,mPmtRecord.GetDataLen());
			} else {
				printf("QDispatch: Error converting QEvent\n");
			}
		} else {
			printf("QDispatch: Dispatcher not connected\n");
		}
	}
	return(rtnVal);
}

Int_t QDispatch::GetData(char *tag, void *databuffer, Int_t buffsize, Int_t waitForIt)
{
	// Get packet tag and data from dispatcher
	// - This call will block until an event is available if waitForIt is true
	// - If an error occurs, this routine automatically disconnects from
	//   the dispatcher and returns 0
	// - Returns number of bytes in databuffer

	Int_t	rc;

	if (!mIsConnected || sRecursionCount) {
		if (!Open()) return(0);
	}
	int disp_nbytes;
	char disp_tag[TAGSIZE+1];

    ++sRecursionCount;	// counter indicates if the user aborted a dispatcher call with ctrl-C

	if (waitForIt) {
		rc = wait_head(disp_tag, &disp_nbytes);
	} else {
		rc = check_head(disp_tag, &disp_nbytes);
		if (!rc) {
		   --sRecursionCount;
		   return(0);	     // return immediately if no data
		}
	}

	if (rc > 0) {
	    if (disp_nbytes <= buffsize) {
            rc = get_data(databuffer,disp_nbytes);
        } else {
            rc = -1000;
        }
	}
	--sRecursionCount;

    if (rc < 0) {
        printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
        Close();
        return(0);
    }
    disp_tag[TAGSIZE] = '\0';
    strcpy(tag, disp_tag);
    return disp_nbytes;
}

QEvent *QDispatch::GetEvent(Int_t waitForIt)
{
	// Get next event from dispatcher
	// - This call will block until an event is available if waitForIt is true
	// - If an error occurs, this routine automatically disconnects from
	//   the dispatcher and returns NULL
	
	Int_t	rc;
	QEvent *theEvent = GetEventAddress();
	
	if (!theEvent) {
		printf("QDispatch: Error allocating memory for QEvent\n");
		return(NULL);	// return NULL if event couldn't be allocated
	}

	if (!mIsConnected || sRecursionCount) {
		if (!Open()) return(NULL);
	}

	int disp_nbytes;
	char disp_tag[TAGSIZE+1];

	++sRecursionCount;	// counter indicates if the user aborted a dispatcher call with ctrl-C

	if (waitForIt) {
		rc = wait_head(disp_tag, &disp_nbytes);
	} else {
		rc = check_head(disp_tag, &disp_nbytes);
		if (!rc) {
			--sRecursionCount;
			return(NULL);	// return immediately if no data
		}
	}
	if (rc > 0) {
		if (!strcmp(disp_tag,"RAWDATA")) {
			// make sure we have memory allocated
			if (mPmtRecord.AllocateBuffer(disp_nbytes)) {
				rc = get_data(mPmtRecord.GetDataPtr(), disp_nbytes);
			} else {
				rc = -1000;
			}
                } else if(!strcmp(disp_tag,"RECHDR")) {
                  // ignore record headers
                        rc = -9999;
		} else {
			printf("QDispatch: Unknown dispatcher tag: %s!\n",disp_tag);
			rc = -1001;
		}
	}
	--sRecursionCount;

	if (rc < 0) {
                if(rc > -9000){
                        printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
		        Close();
                }
		return(NULL);
	}
	
	// must do byte swapping before converting to a QEvent
	mPmtRecord.SwapBytes();
	
	// convert data to QEvent format
	mPmtRecord.ToQEvent(theEvent);
	
	// save the run number and event index
	mRunNumber = mPmtRecord.GetRunNumber();
	mEventIndex = mPmtRecord.GetEventIndex();
	
	return(theEvent);
}


QSNO* QDispatch::GetRecord(UInt_t* type, Int_t waitForIt)
{
	// Get next record from dispatcher
	// - This call will block until a record is available if waitForIt is true
	// - If an error occurs, this routine automatically disconnects from
	//   the dispatcher and returns NULL
	// - The record type (see $QSNO_ROOT/include/include/Record_Info.h) is
	//   returned in "type"

	Int_t	rc;

    QSNO* theRec = NULL;
    UInt_t recType = 0;
	
	if (!mIsConnected || sRecursionCount) {
		if (!Open()) return(NULL);
	}
	
	int disp_nbytes;
	char disp_tag[TAGSIZE+1];
				
    Int_t loop = 1;
    
    while(loop){

        loop = 0;

    	++sRecursionCount;	// counter indicates if the user aborted a dispatcher call with ctrl-C
	
    	if (waitForIt) {
    		rc = wait_head(disp_tag, &disp_nbytes);
    	} else {
    		rc = check_head(disp_tag, &disp_nbytes);
    		if (!rc) {
    			--sRecursionCount;
    			return(NULL);	// return immediately if no data
    		}
    	}
    	if (rc > 0) { 
    		if (( !strcmp(disp_tag,"RAWDATA")) || (!strcmp(disp_tag,"RECHDR")) ){
                if( disp_nbytes <= QDISP_MAXBUFSIZE ){
                    rc = get_data(mDispBuf,disp_nbytes);
                } else {
                    rc = -1000;
                }
    		} else {
    			printf("QDispatch: Unknown dispatcher tag: %s!\n",disp_tag);
    			rc = -1001;
    		}
    	}
    	--sRecursionCount;
	
    	if (rc < 0) {
    		printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
    		Close();
    		return(NULL);
    	}
	
        this->SwapInt32_Buffer(3);
        aGenericRecordHeader* theHeader = (aGenericRecordHeader*) mDispBuf;
        UInt_t recID = theHeader->RecordID;
        this->SwapInt32_Buffer(3);

        unsigned int dnb = (unsigned int) disp_nbytes;

        if(recID == RUN_RECORD){

            QRunHeader* theRHead = GetRunHeaderAddress();
            if(!theRHead){
        		printf("QDispatch: Error allocating memory for QRunHeader, data lost\n");
                rc = -2000;
            } else {
                if(dnb < sizeof(aGenericRecordHeader) + sizeof(aRunRecord)){
                  printf("QDispatch: Odd record size received, dropping data\n");
                  rc = -2001;
                } else {
                    this->SwapInt32_Buffer(disp_nbytes/4);
                    Text_t* runRecStart = mDispBuf + sizeof(aGenericRecordHeader);
                    theRHead->Set((UInt_t*) runRecStart);
                    recType = RUN_RECORD;
                    theRec = (QSNO*) theRHead;
                    mRunNumber = theRHead->GetRun();
                }
            }
        	if (rc < 0) {
    	    	printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
    		    Close();
        		return(NULL);
    	    }
    
        } else if(recID == TRIG_RECORD) {

            QTriggerBank* theTrgBnk = GetTriggerBankAddress();
            if(!theTrgBnk){
        		printf("QDispatch: Error allocating memory for QTriggerBank, data lost\n");
                rc = -3000;
            } else {
                if(dnb < sizeof(aGenericRecordHeader) + sizeof(aTriggerInfo)){
                  printf("QDispatch: Odd record size received, dropping data\n");
                  rc = -3001;
                } else {
                    this->SwapInt32_Buffer(disp_nbytes/4);
                    Text_t* trigRecStart = mDispBuf + sizeof(aGenericRecordHeader);
                    theTrgBnk->Set((Int_t*) trigRecStart);
                    recType = TRIG_RECORD;
                    theRec = (QSNO*) theTrgBnk;
                }
            }
        	if (rc < 0) {
    	    	printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
    		    Close();
        		return(NULL);
    	    }

        } else if(recID == PMT_RECORD) {

        	QEvent *theEvent = GetEventAddress();
    		if (!theEvent) {
    	    	printf("QDispatch: Error allocating memory for QEvent, data lost\n");
                rc = -4000;
        	} else {
    			// make sure we have memory allocated
    			if (mPmtRecord.AllocateBuffer(disp_nbytes)) {
                    mPmtRecord.Load(mDispBuf, disp_nbytes);
   
                	// must do byte swapping before converting to a QEvent
                	mPmtRecord.SwapBytes();
	
                	// convert data to QEvent format
                	mPmtRecord.ToQEvent(theEvent);
	
                	// save the run number and event index
                	mRunNumber = mPmtRecord.GetRunNumber();
                	mEventIndex = mPmtRecord.GetEventIndex();
                
                    recType = PMT_RECORD;
                    theRec = (QSNO*) theEvent;
    			} else {
    				rc = -4001;
    			}
            }
        	if (rc < 0) {
    	    	printf("QDispatch: Dispatcher error %d -- Disconnected\n",rc);
    		    Close();
        		return(NULL);
    	    }

        } else {
            loop = waitForIt;
        }
  
    }

    if(type) *type = recType;	
	return(theRec);

}


void QDispatch::SwapInt32_Buffer(Int_t num)
{
#ifdef SWAP_BYTES
  Char_t  cTemp;
  Char_t*  cPtr;
  for(Int_t j = 0; j < num; j++){
    cPtr = mDispBuf + 4*j;
    cTemp = *cPtr;
    *cPtr = *(cPtr+3);
    *(cPtr+3) = cTemp;
    cTemp = *(cPtr+1);
    *(cPtr+1) = *(cPtr+2);
    *(cPtr+2) = cTemp;
  }
#endif // SWAP_BYTES
}


#endif // NO_DISPATCH

