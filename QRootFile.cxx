/////////////////////////////////////////////////////////////////
// QEventReader derived object to read QEvents from ROOT files //
/////////////////////////////////////////////////////////////////

//*-- Author :	Phil Harvey - 01/05/99
#include <string.h>
#include "QRootFile.h"
#include "QTree.h"
#include "TFile.h"

ClassImp(QRootFile)

QRootFile::QRootFile()
{
	// QRootFile constructor
	mFileName = NULL;
	mFile = NULL;
	mTree = NULL;
	mEventNum = 0;
}

QRootFile::~QRootFile()
{
	// QRootFile destructor
	// - deletes ROOT file object if it exists
	Close();
}

Bool_t QRootFile::Open(char *name)
{
	// Open ROOT file for reading QEvents
	
	Close();	// close old file if it exists
	
	if (name) {
		// save a copy of the file name
		mFileName = new Text_t[strlen(name) + 1];
		if (mFileName) {
			strcpy(mFileName, name);
		}
		// Create new ROOT file for reading
		mFile = new TFile(name,"READ");
		if (mFile && mFile->IsOpen()) {
			/* set QTree pointer */
			mTree = (QTree *)mFile->Get("T");
			if (mTree) {
				// set address for loading events
				GetEventAddress();	// make sure event is allocated
				mTree->SetBranchAddress("Events",&mEvent);
				// initialize read index
				mEventNum = 0;
			} else {
				// no success - delete file object
				delete mFile;
				mFile = NULL;
			}
		} else if (mFile) {
			// clean up if file couldn't be opened properly
			delete mFile;
			mFile = NULL;
		}
	}
	return(IsOpen());
}

Bool_t QRootFile::Close()
{
	// Close ROOT file
	// - returns kTRUE if it was previously open
	if (mFile) {
		delete mFile;
		mFile = NULL;
		mTree = NULL;
		return(kTRUE);
	} else {
		return(kFALSE);
	}
	if (mFileName) {
		delete mFileName;
		mFileName = NULL;
	}
}

QEvent *QRootFile::GetEvent(Int_t code)
{
	// Get next event from file
	// - returns NULL if no more events in file
	
	if (!IsOpen()) return(NULL);

	// attempt to get the next event from the tree
	if (mTree->GetEvent(mEventNum++) > 0) {
		return(GetEventAddress());
	} else {
		return(NULL);	// event not found
	}
}
	
Bool_t QRootFile::Rewind()
{
	// Rewind to start of file
	mEventNum = 0;
	return(IsOpen());
}

void QRootFile::SetEventAddress(QEvent *anEvtPtr)
{
	// Set event address for loading of QEvents
	QEventReader::SetEventAddress(anEvtPtr);
	
	if (IsOpen()) {
		// set event address for tree if it exists
		GetEventAddress();	// make sure event is allocated
		mTree->SetBranchAddress("Events",&mEvent);
	}
}


