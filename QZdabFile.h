// File:	QZdabFile.h
// Author:	Phil Harvey - 12/9/98

#ifndef __QZdabFile__
#define __QZdabFile__

#include <stdio.h>
#include "TObject.h"
#include "QEventReader.h"
#include "QPmtEventRecord.h"
#include "PZdabFile.h"

class QEvent;
class PZdabWriter;

class QZdabFile : public TObject , public QEventReader, public PZdabFile {
public:

	QZdabFile();
	virtual	~QZdabFile();
	
	virtual Bool_t	Open(char *filename)	{ return Open(filename, kFALSE); }
	virtual Bool_t	Open(char *filename, Bool_t for_writing);
	virtual Bool_t	Close();
	
	virtual QEvent*	GetEvent(Int_t code=0);
	virtual Bool_t	Write(QEvent *anEvent);
	
	virtual Bool_t	Rewind();
	virtual Bool_t	IsOpen()	{ return mFile!=NULL; }
	virtual const Text_t*	GetName()const	{ return mFileName;	  }
	
	virtual UInt_t* GetZdabBank(char *mBank ) { return NextBank(BankName(mBank)); }
	
	static void SetVerbose(int on)	{ PZdabFile::SetVerbose(on); }

private:
	QPmtEventRecord	mPmtRecord;	// object to convert from PmtEventRecord to QEvent
	Text_t		  *	mFileName;	// name of open file
	PZdabWriter   *	mWriter;	// writer object
	
	ClassDef(QZdabFile,0)		// Reads/writes QEvents from/to ZDAB files
};

#endif
