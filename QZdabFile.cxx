////////////////////////////////////////////////////////////
// Zdab file reader                                       //
// Reads/writes events sequentially from/to zdab files    //
// Converts events to QEvent format                       //
//   Notes:                                               //
//   1) Currently does not convert zdab Monte Carlo.      //
//   2) With SetVerbose() on, debugging information is    //
//   printed out when records other than PMT event        //
//   records are read.                                    //
////////////////////////////////////////////////////////////

//*-- Author :	Phil Harvey - 12/8/98

#include <stdio.h>
#include <string.h>
#include "QZdabFile.h"
#include "QEvent.h"
#include "QPmtEventRecord.h"
#include "PZdabWriter.h"

ClassImp(QZdabFile)

QZdabFile::QZdabFile()
{
	// QZdabFile constructor
	mFileName = NULL;
	mWriter	  = NULL;
}

QZdabFile::~QZdabFile()
{
	// QZdabFile destructor
	
	Close();			// close the file and free memory
}

Bool_t QZdabFile::Open(char *filename, Bool_t for_writing)
{
	// Open a ZDAB file by name
	// - opens file for writing if 'for_writing' is true.
	// - if file exists and is open for writing, events will be appended to end of file.
	
	Bool_t	success = kFALSE;
	
	Close();	// make sure existing file is closed first
	
	if (filename && *filename) {
		// save a copy of the file name
		mFileName = new Text_t[strlen(filename) + 1];
		if (mFileName) {
			strcpy(mFileName, filename);
		}
		if (for_writing) {
			mWriter = new PZdabWriter(filename);
			success = mWriter->IsOpen();
		} else {
			// open the file
			mFile = fopen(filename,"rb");
			Rewind();		// initialize zdab file for reading
			success = (mFile != NULL);
		}
	}
	return(success);
}

Bool_t QZdabFile::Close()
{
	// Close the file and free any allocated memory
	
	Bool_t wasOpen;
	
	if (mWriter) {
		wasOpen = mWriter->IsOpen();
		delete mWriter;
		mWriter = NULL;
	} else {
		wasOpen = (mFile != NULL);
		if (wasOpen) {
			fclose(mFile);
			mFile = NULL;
		}
	}
	
	FreeEvent();			// free our event buffer
	
	if (mFileName) {
		delete mFileName;	// free our filename
		mFileName = NULL;
	}
	
	return(wasOpen);
}

QEvent *QZdabFile::GetEvent(Int_t code)
{
	// Get next event from ZDAB file
	// - returns NULL on error or if no more events in file
	// - can only be called if the file was opened in read mode
	
	if (mWriter) return(NULL);	// can't read from an output file
	
	QEvent *theEvent = GetEventAddress();
	
	if (!theEvent) {
		printf("QZdabFile: Error allocating memory for QEvent\n");
		return(NULL);	// return NULL if event couldn't be allocated
	}
	
	if (!mFile) {
		printf("QZdabFile: No ZDAB file open\n");
		return(NULL);
	}
	
	// get next PmtEventRecord from file
	PmtEventRecord *per = NextPmt();
	
	if (!per) return(NULL);
	
	// convert the PmtEventRecord to a QEvent
	mPmtRecord.SetPmtEventRecord(per);
	mPmtRecord.ToQEvent(theEvent);
	
	return(theEvent);
}

Bool_t QZdabFile::Write(QEvent *anEvent)
{
	// Write an event to the output zdab file
	// - can only be called if the file was opened in write mode.
	// - returns true if the event was written OK
	
	if (!mWriter || !mWriter->IsOpen()) return(kFALSE);
	
	mPmtRecord.FromQEvent(anEvent);		// convert to a PmtEventRecord from a QEvent
	
	return(mWriter->Write(mPmtRecord.GetPmtEventRecord()) == 0);
}

Bool_t QZdabFile::Rewind()
{
	// Rewind to first event in file
	// - returns true if file was rewound successfully
	// - can't rewind an output file
	if (mWriter || !mFile || Init(mFile)<0) {
		return(kFALSE);
	} else {
		return(kTRUE);
	}
}
