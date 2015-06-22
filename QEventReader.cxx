//////////////////////////////////////////////
// Base class for objects that read QEvents //
//////////////////////////////////////////////

//*-- Author :	Phil Harvey - 12/9/98

#include <stdio.h>
#include <string.h>
#include "QEventReader.h"
#include "QEvent.h"
#include "QRunHeader.h"
#include "QTriggerBank.h"

ClassImp(QEventReader)

QEventReader::QEventReader()
{
	// QEventReader constructor
	
	mEventAllocated = kFALSE;
	mEvent    = NULL;
        mRHead    = NULL;
        mTrigBank = NULL;
}

QEventReader::~QEventReader()
{
	// QEventReader destructor

//	FreeEvent();			// free allocated memory
        this->FreeRecords();
}

void QEventReader::FreeEvent()
{
	// Free any memory allocated by this object
	if (mEventAllocated) {
		mEventAllocated = kFALSE;
		delete mEvent;
		mEvent = NULL;
	}
}

void QEventReader::FreeRecords()
{
        // Free any memory allocated by this object
        this->FreeEvent();
        if(mRHead) delete mRHead;
        mRHead = NULL;
        if(mTrigBank) delete mTrigBank;
        mTrigBank = NULL;
}

void QEventReader::SetEventAddress(QEvent *anEvtPtr)
{
	// Set address for event
	// Events received via GetEvent() will be stored here
	// - if no event address is set, QEventReader will create an event
	//   for return via GetEvent(), and will delete this event when destructed.

	if (mEventAllocated) {
		mEventAllocated = kFALSE;
		delete mEvent;
	}
	mEvent = anEvtPtr;
}

QEvent *QEventReader::GetEventAddress()
{
	// Get address for event
	// - will allocate a QEvent if not already allocated
	// - if allocated, the QEvent will be deleted when the QEventReader object is deleted

	if (!mEvent) {
		mEvent = new QEvent;
		mEventAllocated = kTRUE;
	}
	return(mEvent);
}

QRunHeader* QEventReader::GetRunHeaderAddress()
{
	// Get address for run header
	// - will allocate a QRunHeader if not already allocated
	// - if allocated, the QRunHeader will be deleted when the QEventReader object is deleted
        if(!mRHead) mRHead = new QRunHeader;
        return(mRHead);
}

QTriggerBank* QEventReader::GetTriggerBankAddress()
{
	// Get address for trigger bank
	// - will allocate a QTriggerBank if not already allocated
	// - if allocated, the QTriggerBank will be deleted when the QEventReader object is deleted
        if(!mTrigBank) mTrigBank = new QTriggerBank;
        return(mTrigBank);
}


#ifndef NO_EVENT_READER_CREATE
#ifndef NO_DISPATCH
#include "QDispatch.h"
#endif
#include "QZdabFile.h"
#include "QRootFile.h"
QEventReader *CreateEventReader(char *name)
{
	// Create a QEventReader of the appropriate type:
	// QZdabFile - created if file exists and name contains "zdab"
	// QRootFile - created if file exists and name doesn't contain "zdab"
	// QDispatch - created if no file exists and name is a valid dispatcher IP addr
	// Otherwise returns NULL.
	// - Calls Open() for the newly created object.
	// - The caller is responsible for deleting the object created.

	QEventReader	*evRead = NULL;

	if (name) {
		// get pointer to start of filename (ignore path string if it exists)
		char *namestart = strrchr(name,'/');
		if (!namestart) namestart = name;
		else namestart++;

		FILE *fp = fopen(name,"r");
		if (fp) {
			fclose(fp);
			if (strstr(namestart,"zdab")) {
				evRead = new QZdabFile;
			} else {
				evRead = new QRootFile;
			}
		} else {
#ifndef NO_DISPATCH
			evRead = new QDispatch;
#endif
		}
		if (!evRead->Open(name)) {
			delete evRead;
			evRead = NULL;
		}
	}
	return(evRead);
}
#endif
