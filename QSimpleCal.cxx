/////////////////////////////////////////////////////////////////////////////////////////////
// Calibrates PMT data using simple calibration constants                                  //
// - shares data with other QSimpleCal objects if they are using the same calibration file   //
/////////////////////////////////////////////////////////////////////////////////////////////

//*-- Author :	Phil Harvey - 12/9/98

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QSimpleCal.h"
#include "QPMT.h"
#include "calibrate.h"
#include "openfile.h"

ClassImp(QSimpleCal)

Text_t	 	QSimpleCal::sDefaultFileName[kMaxNameLen] = "cal_default.dat";
Text_t	 	QSimpleCal::sDefaultDirectory[kMaxNameLen] = "/usr/local/lib/xsnoed";
SharedDB  *	QSimpleCal::sSharedDB = NULL;

QSimpleCal::QSimpleCal()
{
	// QSimpleCal constructor
	
	mDoTimeWalk = kTRUE;	// do time walk correction by default
	mDBase = NULL;
	mFileName[0] = '\0';
	mLoadDBase = kTRUE;
}

QSimpleCal::~QSimpleCal()
{
	// QSimpleCal destructor
	
	RemoveSharedDB(mDBase);	// free database array
}

void QSimpleCal::SetDefaultDirectory(Text_t *aDirectory)
{
	// set default directory for calibration database files
	strcpy(sDefaultDirectory,aDirectory);
}

void QSimpleCal::SetDefaultFileName(Text_t *aFileName)
{
	// set default calibration database file name
	strcpy(sDefaultFileName,aFileName);
}

DBase  *QSimpleCal::AccessSharedDB(Text_t *dbaseFileName)
{
	// Get pointer to shared database if it exists
	// - must call RemoveSharedDB when done with the returned database
	
	for (SharedDB *sdb=sSharedDB; sdb; sdb=sdb->next) {
		if (!strcmp(sdb->name, dbaseFileName)) {
			++sdb->num_owners;	// one more owner
			return(sdb->dbase);	// return the database
		}
		sdb = sdb->next;		// step to next entry
	}
	return(NULL);	// no shared database available
}

void QSimpleCal::AddSharedDB(DBase *aDBase,Text_t *dbaseFileName)
{
	// Add a database to the pool of shared databases
	// - call RemoveSharedDB when done with the database
	// - only call this routine after a failed call to AccessSharedDB()
	
	if (aDBase) {
		SharedDB *sdb = new SharedDB;
		if (sdb) {
			// initialize this entry in the shared DB list
			sdb->dbase = aDBase;
			strcpy(sdb->name,dbaseFileName);
			sdb->num_owners = 1;
			sdb->next = sSharedDB;
			sSharedDB = sdb;
		}
	}
}

void QSimpleCal::RemoveSharedDB(DBase *aDBase)
{
	// Frees memory allocated to databases when all objects are done with them

	if (aDBase) {
		SharedDB	**lastPt = &sSharedDB;
		for (SharedDB *sdb=sSharedDB; sdb; sdb=sdb->next) {
			if (sdb->dbase == aDBase) {
				// decrement owner count and free memory if no more owners
				if (!(--sdb->num_owners)) {
					free(sdb->dbase);		// free database memory
					*lastPt = sdb->next;	// remove from list
					delete sdb;				// free item memory
				}
				break;
			}
			lastPt = &sdb->next;	// remember location of pointer to this entry
		}
	}
}

Int_t QSimpleCal::Init(char *filename)
{
	// Initialize this object
	// - loads constants from database file
	// - uses default database filename if none specified
	// - if this routine is not called before the first database access,
	//   the default database will automatically be loaded
	
	// free existing database
	RemoveSharedDB(mDBase);
	
	mDBase = NULL;			// reset dbase pointer
	mLoadDBase = kTRUE;		// set flag to load new DBase
	
	if (filename) {
		// save the database filename
		strcpy(mFileName, filename);
	} else {
		mFileName[0] = '\0';
	}
	
	// attempt to load the database
	if (LoadDBase() == NULL) return(-1);
	
	return(0);	// database was loaded successfully
}

DBase *QSimpleCal::LoadDBase()
{
	// load database into memory
	// - uses shared database if it has the same name
	// - returns database pointer if successful, NULL otherwise
	// - creates index if none available
	
	if (mLoadDBase) {
	
		Text_t	*name;
		
		// only try once to load the dbase
		mLoadDBase = kFALSE;
		
		// use our name if set, or default name otherwise
		if (mFileName[0]) name = mFileName;
		else name = sDefaultFileName;
		
		// is a shared dbase available with the same name?
		mDBase = AccessSharedDB(name);
		
		if (!mDBase) {
			mDBase = newCalibrationDatabase(name,sDefaultDirectory);
			if (mDBase) {
				// share this database
				AddSharedDB(mDBase,name);
				printf("QSimpleCal: Done loading calibration.\n");
				printf("QSimpleCal: %d PMT's with valid calibrations.\n",mDBase->nvalid);
			} else {
				printf("QSimpleCal: Error loading calibration file %s\n",name);
			}
		}
	}
	return(mDBase);
}

Int_t QSimpleCal::Sett(QPMT *aPmt)
{
	// Sets calibrated time in PMT object

	if (mDBase || LoadDBase()) {
		double val = getCalibratedTac(mDBase, aPmt->Getit(), aPmt->GetCell(),
									  aPmt->Getn(), mDoTimeWalk ? aPmt->Getihs() : -1);

		aPmt->Sett(val);	// set the calibrated time
	
		return(val==INVALID_CAL ? -1 : 0);
	}
	return(QCal::Sett(aPmt));
}


Int_t QSimpleCal::Sethl(QPMT *aPmt)
{
	// Sets calibrated Qhl in PMT object

	if (mDBase || LoadDBase()) {
		double val = getCalibratedQhl(mDBase, aPmt->Getihl(), aPmt->GetCell(),
									  aPmt->Getn());
		
		aPmt->Sethl(val);
		
		return(val==INVALID_CAL ? -1 : 0);
	}
	return(QCal::Sethl(aPmt));
}

Int_t QSimpleCal::Seths(QPMT *aPmt)
{
	// Sets calibrated Qhs in PMT object
	
	if (mDBase || LoadDBase()) {
		double val = getCalibratedQhs(mDBase, aPmt->Getihs(), aPmt->GetCell(),
									  aPmt->Getn());
		
		aPmt->Seths(val);
		
		return(val==INVALID_CAL ? -1 : 0);
	}
	return(QCal::Seths(aPmt));
}

Int_t QSimpleCal::Setlx(QPMT *aPmt)
{
	// Sets calibrated Qlx in PMT object
	
	if (mDBase || LoadDBase()) {
		double val = getCalibratedQlx(mDBase, aPmt->Getilx(), aPmt->GetCell(),
									  aPmt->Getn());
		
		aPmt->Setlx(val);
		
		return(val==INVALID_CAL ? -1 : 0);
	}
	return(QCal::Setlx(aPmt));
}
