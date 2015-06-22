////////////////////////////////////////////////////////////////////
// Object to read a list of events from files
//
// - Reads event list from input file
// - Each line of the file specifies a new event file to open
//   followed by a list of events to read from that file.
// 
// File format: 	<filename> [<id1>][-[<id2>]]...
//              	SETPATH [<data directory>]
// 
// Example File: 	SETPATH /datadisk/snodata/calib
//				 	SNO_0000002579_000.zdab 411-414 420
//				 	SNO_0000002580_000.zdab 8 10
//				 	SNO_0000002581_000.zdab
//					SETPATH /disk2/phil/testdata
//				 	SNO_0000001698_000.zdab -20 500 510-
//				 	SNO_0000001730_000.zdab 32201
//					SETPATH
//					/disk2/phil/moredata/golden_events.root 195-210
//					/disk2/mgb/snodata/testfile.root 1010
// Notes:
//
// 	1) <filename> may be a ROOT or ZDAB file, or a dispatcher address.
//	2) Event ID's must be in the same order as in the file.
//	3) A new event file is opened for each line in the input.
// 	4) If no list of events is specified, all are read.
//	5) A directory may be specified in the file name, but is
//     overridden by a directory specified in a SETPATH statement
//     or a call to SetPath().
//  6) Multiple SETPATH statements may exist in a file, each path
//     applies to subsequent data file specifications.
//
////////////////////////////////////////////////////////////////////

//*-- Author :	Phil Harvey - 02/25/99

#include <stdlib.h>
#include <string.h>
#include "QListFile.h"
#include "QEventReader.h"
#include "QEvent.h"

const Int_t	ALL_EVENTS	= 0xffffffffL;

ClassImp(QListFile)

QListFile::QListFile()
{
	// QListFile constructor
	mInputFile = NULL;
	mEventReader = NULL;
	*mInputBuffer = '\0';
	*mDataPath = '\0';
}

QListFile::~QListFile()
{
	// QListFile destructor - make sure file is closed
	Close();
}

Bool_t QListFile::Open(Text_t *name)
{
	// open new input file
	Close();	// make sure everything is closed first
	
	if (name) {
		mInputFile = fopen(name,"r");
	}
	return(IsOpen());
}

Bool_t QListFile::Close()
{
	// close input event list file
	Bool_t	was_open = IsOpen();
	
	if (mInputFile) {
		fclose(mInputFile);
		mInputFile = NULL;
	}
	DeleteEventReader();
	*mInputBuffer = '\0';		// reset buffer string
	
	return(was_open);
}

Bool_t QListFile::Rewind()
{
	// rewind to start of event list
	DeleteEventReader();		// delete event reader to force reading of next line in file
	if (mInputFile) {
		fseek(mInputFile,0L,SEEK_SET);	// rewind the input file
	}
	return(IsOpen());
}

void QListFile::DeleteEventReader()
{
	// delete our internal event reader object
	if (mEventReader) {
		delete mEventReader;
		mEventReader = NULL;
	}
}

Bool_t QListFile::NextEventRange()
{
	// Get next range of event id's from input file
	
	Text_t *token = NextToken();
	
	if (token) {
		Text_t *dash = strchr(token,'-');
		if (dash) {
			*dash = '\0';
			if (*token) {
				mFirstEvent = atol(token);
			} else {
				mFirstEvent = ALL_EVENTS;
			}
			if (*(dash + 1)) {
				mLastEvent = atol(dash + 1);
			} else {
				mLastEvent = ALL_EVENTS;	// read all remaining events
			}
		} else {
			mFirstEvent = mLastEvent = atol(token);
		}
		return(kTRUE);	// found another range
	}
	return(kFALSE);
}

void QListFile::SetPath(Text_t *path)
{
	// set path name for data files
	if (!path) {
		*mDataPath = '\0';
	} else if (strlen(path) >= (kMaxPathLen-23)) {	// normal sno file name is 23 chars long
		fprintf(stderr,"QListFile: Path name too long!\n");
	} else {
		strcpy(mDataPath, path);
		Text_t *pt = strchr(mDataPath, '\0');
		if (pt>mDataPath && *(pt-1) != '/') {
			// add trailing '/' to path name
			*(pt++) = '/';
			*pt = '\0';
		}
	}
}

QEvent *QListFile::GetEvent(Int_t code)
{
	// Get next event as specified by input file
	for (;;) {
		if (!mEventReader) {
			if (!mInputFile) return(NULL);
			// read next line from input file
			if (!fgets(mInputBuffer, kInputBufferSize, mInputFile)) {
				return(NULL);
			}
			Text_t *pt;
			Text_t *file_name = NextToken(mInputBuffer);
			if (!file_name) return(NULL);
			
			// parse SETPATH statements
			if (!strcmp(file_name,"SETPATH")) {
				SetPath(NextToken());	// change current path setting
				continue;
			}
			
			if (*mDataPath) {
				// remove path specification from filename if directory is specified
				pt = strrchr(file_name, '/');
				if (pt) file_name = pt + 1;
				// filename will be added to end of path specification
				pt = strchr(mDataPath, '\0');
			} else {
				// filename will be written into start of datapath buffer
				pt = mDataPath;
			}
			if ((Int_t)(pt - mDataPath + strlen(file_name)) >= kMaxPathLen) {
				fprintf(stderr,"QListFile: File name too long!\n");
			} else {
				strcpy(pt,file_name);	// add file name to directory
				mEventReader = CreateEventReader(mDataPath);
				*pt = 0;	// remove name from end of directory for next time
			}
			if (!mEventReader) return(NULL);
			
			// initialize event range (default is entire file)
			mFirstEvent = mLastEvent = ALL_EVENTS;
			NextEventRange();
		}
		// set our event reader to the same address as we are using
		mEventReader->SetEventAddress(GetEventAddress());
		// get the next event
		QEvent *evt = mEventReader->GetEvent();
	
		if (evt) {
			Int_t event_id = evt->GetEvent_id();
			for (;;) {
				if (mFirstEvent != ALL_EVENTS &&
					CompareEventIDs(event_id,mFirstEvent) < 0)
				{
					break;	// keep looking for event
				}
				if (mLastEvent != ALL_EVENTS &&
					CompareEventIDs(event_id,mLastEvent) > 0)
				{
					if (NextEventRange()) continue;	// check event against new range
					// event not in this file -- parse next line of input
					DeleteEventReader();
					break;		// loop back to read next line in file
				}
				return(evt);	// a good event -- RETURN IT
			}
		} else {
			// kill event reader and continue parsing input file
			DeleteEventReader();
		}
	}
}


