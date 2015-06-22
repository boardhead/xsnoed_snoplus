// File:	QRootFile.h
// Author:	Phil Harvey - 01/05/99

#ifndef __QRootFile__
#define __QRootFile__

#include "TObject.h"
#include "QEventReader.h"

class TFile;
class QTree;

class QRootFile : public TObject, public QEventReader {
public:
	QRootFile();
	virtual ~QRootFile();
	
	virtual Bool_t	Open(char *name=NULL);
	virtual Bool_t	Close();
	virtual Bool_t	Rewind();
	virtual Bool_t	IsOpen()	{ return mFile!=NULL;	}
	virtual const Text_t*	GetName()const	{ return mFileName;		}
	
	virtual QEvent*	GetEvent(Int_t code=0);

	virtual void	SetEventAddress(QEvent *anEvtPtr=NULL);

private:
	Text_t	  *	mFileName;		// name of open file (NULL if not open)
	TFile	  *	mFile;			// pointer to ROOT file object (NULL if not open)
	QTree	  *	mTree;			// pointer to QTree object
	Int_t	 	mEventNum;		// index of next event to read from file
	
	ClassDef(QRootFile,0)	// Reads QEvents from ROOT files
};

#endif
