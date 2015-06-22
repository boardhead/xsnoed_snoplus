//-------------------------
// QListFile.h
//
#ifndef __QListFile_h__
#define __QListFile_h__

#include <stdio.h>
#include "QEventReader.h"
#include "QUtils.h"

const short	kInputBufferSize	= 2048;
const short	kMaxPathLen			= 1024;

class QEvent;

class QListFile : public QEventReader, public QTokenizer {
public:
	QListFile();
	virtual ~QListFile();
	
	virtual Bool_t	Open(Text_t *name=NULL);
	virtual Bool_t	Close();
	virtual Bool_t	IsOpen()		{ return mInputFile != NULL; }
	virtual Bool_t	Rewind();
	virtual QEvent*	GetEvent(Int_t code=0);

	virtual const Text_t*	GetName()const		{ return mInputBuffer;	}
	
	void			SetPath(Text_t *path=NULL);
	
private:
	void			CloseFile();
	Bool_t			NextEventRange();
	void			DeleteEventReader();
	
	FILE		  *	mInputFile;			// input file (list of event files)
	QEventReader  *	mEventReader;		// pointer to the event reader
	Int_t			mFirstEvent;		// GTID of first event to read in current group
	Int_t			mLastEvent;			// GTID of last event in current group
	Text_t			mInputBuffer[kInputBufferSize];	// buffer for reading lines from file
	Text_t			mDataPath[kMaxPathLen];			// path specification for event file

	ClassDef(QListFile,0)	// Read specified events from a list
};

#endif // __QListFile_h__
