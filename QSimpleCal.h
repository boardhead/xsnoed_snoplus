// File:	QSimpleCal.h
// Author:	Phil Harvey - 12/9/98

#ifndef __QSimpleCal__
#define __QSimpleCal__

#include "QCal.h"

const int	kMaxNameLen		= 256;

struct DBase;

struct SharedDB {
	DBase		*dbase;
	Text_t		name[kMaxNameLen];
	Int_t		num_owners;
	SharedDB	*next;
};


class QSimpleCal : public QCal {
public:

	QSimpleCal();
	virtual ~QSimpleCal();
	
	virtual Int_t	Init(char *filename=NULL);
	
	virtual Int_t	Sett(QPMT *aPmt);
	virtual Int_t	Sethl(QPMT *aPmt);
	virtual Int_t	Seths(QPMT *aPmt);
	virtual Int_t	Setlx(QPMT *aPmt);
	
	void			DoTimeWalk(Bool_t doIt)	{ mDoTimeWalk = doIt; }
	
	static void		SetDefaultDirectory(Text_t *aDirectory);
	static void		SetDefaultFileName(Text_t *aFileName);
	
private:
	DBase		  * LoadDBase();
	
	static DBase  *	AccessSharedDB(Text_t *dbaseFileName);
	static void		AddSharedDB(DBase *aDBase,Text_t *dbaseFileName);
	static void		RemoveSharedDB(DBase *aDBase);
	
	Bool_t			mDoTimeWalk; 			// true to do Q vs T correction (true is default)
	
	DBase		  *	mDBase;					// pointer to calibration database
	Bool_t			mLoadDBase;				// flag set to load the database
	Text_t			mFileName[kMaxNameLen];	// calibration database file name
	
	static Text_t	sDefaultFileName[kMaxNameLen];	// default calibration data file name
	static Text_t	sDefaultDirectory[kMaxNameLen];	// default calibration file directory

	static SharedDB	*sSharedDB;				// linked list of databases shared by these objects
	
	ClassDef(QSimpleCal,0)	// PMT calibration using simple linear constants
};

#endif
