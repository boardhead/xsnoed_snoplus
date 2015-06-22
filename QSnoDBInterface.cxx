//
// QSnoDB interface to load values from the SNODB
//
// Created: 11/15/00 - P. Harvey
//

#include <string.h>
#include "QSnoDBInterface.h"
#include "QSnoDB.h"

const int kDataType = 11;
const int kCards	= 16;
const int kChannels	= 32;
const int kSizeDQCR = KDQCR_TABLE + 16 - 1;
const int kSizeDQCH = KDQCH_TABLE + 512 - 1;

ClassImp(QSnoDBInterface)


// interface function
PSnoDBInterface *newQSnoDBInterface(WidgetPtr label)
{
	return(new QSnoDBInterface(label));
}

QSnoDBInterface::QSnoDBInterface()
			   : QSnoCal(""), PSnoDBInterface(NULL)
{
	InitQSnoDBInterface();
}

QSnoDBInterface::QSnoDBInterface(WidgetPtr label)
			   : QSnoCal(""), PSnoDBInterface(label)
{
	InitQSnoDBInterface();
}

QSnoDBInterface::~QSnoDBInterface()
{
	int i;
	for (i=0; i<39; ++i) {
		delete ftslp2[i];
		delete fpdst2[i];
    }
	delete [] ftslp2;
	delete [] fpdst2;
	
	for (i=0; i<19; ++i) {
		delete fdqch[i];
		delete fdqcr[i];
	}
	delete [] fdqch;
	delete [] fdqcr;
}

void QSnoDBInterface::InitQSnoDBInterface()
{
	int i;
	
	// call inherited method to set parameter string
	// so we don't re-create the QSnoDB unnecessarily
	PSnoDBInterface::SetParamString("localhost");
	if (fClient) {
		fClient->SetQuiet(kTRUE);
	}
	ftslp2 = new QBank *[39];
	fpdst2 = new QBank *[39];
	for (i=0; i<39; ++i) {
		ftslp2[i] = 0;
		fpdst2[i] = 0;
	}
	fdqch = new QBank *[19];
	fdqcr = new QBank *[19];
	for (i=0; i<19; ++i) {
		fdqch[i] = 0;
		fdqcr[i] = 0;
	}
}

// SetParamString - set the parameter string (used for the SNODB client address)
void QSnoDBInterface::SetParamString(char *str)
{
	if (strcmp(str,GetParamString())) {
		PSnoDBInterface::SetParamString(str);
		delete fClient;
		fClient = new QSnoDB(GetParamString());
		if (fClient) {
			fClient->SetQuiet(kTRUE);
		}
	}
}

// GetData - return specified data for each pmt in array
// returns: number of banks loaded, or -1 on error
int QSnoDBInterface::GetData(FECReadoutData *pmt, CalibratedPMT *calPmt, ESnoDB_Parameter type,
						      u_int32 date, u_int32 time, int count)
{
	int		i;
	int		numLoaded = 0;
	char	banksLoaded[39];	// must be one larger than needed due to FORTRAN indexing! doh!
	
	if (!fpdst) {
		Message("Memory Error");
		return(-1);
	}
	memset(banksLoaded, 0, sizeof(banksLoaded));
	
	// loop over all PMT hits
	while (count) {
	
		// extract parameters for this PMT hit
		int cr1 = pmt->CrateID + 1;
		int ca1 = pmt->BoardID + 1;
		int ch1 = pmt->ChannelID + 1;
		int ce1 = pmt->CellID + 1;
		int ca2 = ca1>kCards/2 ? ca1-kCards/2 : ca1;
		int halfCrate = 2*(cr1-1) + (ca1 - 1)/8 + 1;
	  	u_int32 offset;
	  	QBank *tmpBank;
	  	int doLoad;
	  	
	 	if (!banksLoaded[halfCrate]) {
	 		doLoad = 1;
	 		if (type == kSnoDB_CalibrateTime) {
				// load time slope banks (necessary only if we are calibrating times)
				if (ftslp[halfCrate] && ftslp[halfCrate]->IsValid(date,time,kDataType)) {
					doLoad = 0;					// no need to load bank
				} else {
					// swap the primary and secondary banks
					tmpBank = ftslp[halfCrate];
					ftslp[halfCrate] = ftslp2[halfCrate];
					ftslp2[halfCrate] = tmpBank;
					// test the secondary bank for validity
					if (ftslp[halfCrate] && ftslp[halfCrate]->IsValid(date,time,kDataType)) {
						doLoad = 0;
					}
				}
				if (doLoad) {
					Message("Loading TSLP %d (%ld %ld)...",halfCrate,(long)date,(long)time);
					delete ftslp[halfCrate];	// delete existing bank
					ftslp[halfCrate] = fClient->GetBank("TSLP",halfCrate,date,time);
					if (ftslp[halfCrate]) {
						++numLoaded;
					} else if (!fClient->IsServerOK()) {
						Message("SNODB server %s not responding!",GetParamString());
						return(-1);
					}
				}
			} else {
				// load pedestal bank
				if (fpdst[halfCrate] && fpdst[halfCrate]->IsValid(date,time,kDataType)) {
					doLoad = 0;
				} else {
					// swap the primary and secondary banks
					tmpBank = fpdst[halfCrate];
					fpdst[halfCrate] = fpdst2[halfCrate];
					fpdst2[halfCrate] = tmpBank;
					if (fpdst[halfCrate] && fpdst[halfCrate]->IsValid(date,time,kDataType)) {
						doLoad = 0;
					}
				}
				if (doLoad) {
					Message("Loading PDST %d (%ld %ld)...",halfCrate,(long)date,(long)time);
					delete fpdst[halfCrate];
					fpdst[halfCrate] = fClient->GetBank("PDST",halfCrate,date,time);
					if (fpdst[halfCrate]) {
						++numLoaded;
					} else if (!fClient->IsServerOK()) {
						Message("SNODB server %s not responding!",GetParamString());
						return(-1);
					}
				}
			}
			banksLoaded[halfCrate] = 1;
		}
	  	switch (type) {
	  	
	  		case kSnoDB_CalibrateTime: {
	  			QPMT qPmt;
				qPmt.Setn(((pmt->CrateID * 16) + pmt->BoardID) * 32 + pmt->ChannelID);	  			
				qPmt.SetCell(pmt->CellID);
	  			qPmt.Setihs(0);
	  			qPmt.Setihl(0);
	  			qPmt.Setilx(0);
	  			qPmt.Setit(UNPK_TAC((u_int32 *)pmt));
	  			DoECA(&qPmt);
	  			calPmt->qhs = qPmt.Geths();
	  			calPmt->qhl = qPmt.Gethl();
	  			calPmt->qlx = qPmt.Getlx();
	  			calPmt->tac = qPmt.Gett();
	  		} break;
	  		
	  		case kSnoDB_CalibrateCharge: {
	  			QPMT qPmt;
				qPmt.Setn(((pmt->CrateID * 16) + pmt->BoardID) * 32 + pmt->ChannelID);	  			
				qPmt.SetCell(pmt->CellID);
	  			qPmt.Setihs(UNPK_QHS((u_int32 *)pmt));
	  			qPmt.Setihl(UNPK_QHL((u_int32 *)pmt));
	  			qPmt.Setilx(UNPK_QLX((u_int32 *)pmt));
	  			qPmt.Setit(0);
	  			DoECA(&qPmt);
	  			calPmt->qhs = qPmt.Geths();
	  			calPmt->qhl = qPmt.Gethl();
	  			calPmt->qlx = qPmt.Getlx();
	  			calPmt->tac = qPmt.Gett();
	  		} break;
	  		
	  		case kSnoDB_ChargePedestal: {
			  	offset = 5 * ((ce1-1)*(kChannels*kCards/2)+(ch1-1)*(kCards/2)+(ca2-1)) + 1;
			  	float *calPt = &calPmt->qhs;
				if (fpdst[halfCrate] && fpdst[halfCrate]->IsData()) {
				  	for (i=1; i<=3; ++i) {
				  		// copy pedestal value into calibrated charge for qhs, qhl and qlx
						*(calPt++) = (float)fpdst[halfCrate]->icons(offset + i);
					}
				} else {
				  	for (i=1; i<=3; ++i) {
				  		// no data available
						*(calPt++) = -9999.;
					}
				}
			} break;
			
			default:
				calPmt->qhs = calPmt->qhl = calPmt->qlx = calPmt->tac = 0;
				break;
		}
		--count;
		++pmt;
		++calPmt;
	}
	return(numLoaded);
}


/* From QGlobals.h
  KDQCH_TABLE             = 11,
  KDQCH_B_PMT_CABLE       =  0,
  KDQCH_B_PMTIC_RESISTOR  =  1,
  KDQCH_B_SEQUENCER       =  2,
  KDQCH_B_100NS           =  3,
  KDQCH_B_20NS            =  4,
  KDQCH_B_75OHM           =  5,
  KDQCH_B_QINJ            =  6,
  KDQCH_B_N100            =  7,
  KDQCH_B_N20             =  8,
  KDQCH_B_NOT_OP          =  9,
  KDQCH_B_BAD             = 10,
  KDQCH_B_VTHR            = 16,
  KDQCH_B_VTHR_ZERO       = 24 
*/
// GetChannelStatus - returns array of channel status words
// returns: number of banks loaded, or -1 on error
int QSnoDBInterface::GetChannelStatus(FECReadoutData *pmt, u_int32 *status_out,
									  u_int32 date, u_int32 time, int count)
{
	int		numLoaded = 0;
	char	banksLoaded[19];
	
	if (!fdqch) {
		Message("Memory Error");
		return(-1);
	}
	memset(banksLoaded, 0, sizeof(banksLoaded));
	
	while (count) {
		int crate	= pmt->CrateID;
		int card	= pmt->BoardID;
		int channel	= pmt->ChannelID;
		
	 	if (!banksLoaded[crate]) {
			// load time slope banks (necessary only if we are calibrating times)
			if (!fdqch[crate] || !fdqch[crate]->IsValid(date,time,kDataType)) {
				// Note: DQXX bank number start at 0!!! (not 1)
				int nbank = crate;
				Message("Loading DQCH %d (%ld %ld)...",nbank,(long)date,(long)time);
				delete fdqch[crate];	// delete existing bank
				fdqch[crate] = fClient->GetBank("DQCH",nbank,date,time);
				if (!fdqch[crate] && !fClient->IsServerOK()) {
					Message("SNODB server %s not responding!",GetParamString());
					return(-1);
				}
				++numLoaded;
			}
			banksLoaded[crate] = 1;
		}
		// Note: IsData() doesn't work for bank 0!!
		if (fdqch[crate] && fdqch[crate]->GetData()->GetSize()>=kSizeDQCH) {
			*status_out = fdqch[crate]->icons(KDQCH_TABLE + 32*card + channel);
		} else {
			*status_out = 0xffffffff;
		}
		--count;
		++pmt;
		++status_out;
	}
	return(numLoaded);
}


/* From QGlobals.h
 KDQCR_TABLE       = 11,
 KDQCR_B_CRATE     =  0,
 KDQCR_B_MB        =  1,
 KDQCR_B_PMTIC     =  2,
 KDQCR_B_DAQ       =  3,
 KDQCR_B_DC        =  4,
 KDQCR_B_SLOT_OP   =  8,
 KDQCR_B_GT        =  9,
 KDQCR_B_CR_ONLINE = 10,
 KDQCR_B_CR_HV     = 11,
 KDQCR_B_RELAY     = 12,
 KDQCR_B_HV        = 16
*/
// GetCardStatus - returns array of card status words
// returns: number of banks loaded, or -1 on error
int QSnoDBInterface::GetCardStatus(FECReadoutData *pmt, u_int32 *status_out,
								   u_int32 date, u_int32 time, int count)
{
	int		numLoaded = 0;
	char	banksLoaded[19];
	
	if (!fdqcr) {
		Message("Memory Error");
		return(-1);
	}
	memset(banksLoaded, 0, sizeof(banksLoaded));
	
	while (count) {
		int crate	= pmt->CrateID;
		int card	= pmt->BoardID;
		int dc		= pmt->ChannelID / 8;
		
	 	if (!banksLoaded[crate]) {
			// load time slope banks (necessary only if we are calibrating times)
			if (!fdqcr[crate] || !fdqcr[crate]->IsValid(date,time,kDataType)) {
				// Note: DQXX bank number start at 0!!! (not 1)
				int nbank = crate;
				Message("Loading DQCR %d (%ld %ld)...",nbank,(long)date,(long)time);
				delete fdqcr[crate];	// delete existing bank
				fdqcr[crate] = fClient->GetBank("DQCR",nbank,date,time);
				if (!fdqcr[crate] && !fClient->IsServerOK()) {
					Message("SNODB server %s not responding!",GetParamString());
					return(-1);
				}
				++numLoaded;
			}
			banksLoaded[crate] = 1;
		}
		// Note: IsData() doesn't work for bank 0!!
		if (fdqcr[crate] && fdqcr[crate]->GetData()->GetSize()>=kSizeDQCR) {
			u_int32 flags = fdqcr[crate]->icons(KDQCR_TABLE + card);
			// set all DC flags to the value for this dc
			if (flags & (0x01 << (KDQCR_B_DC + dc))) {
				flags |= (0x0f << KDQCR_B_DC);
			} else {
				flags &= ~(0x0f << KDQCR_B_DC);
			}
			// set all relay flags to the value for this dc
			if (flags & (0x01 << (KDQCR_B_RELAY + dc))) {
				flags |= (0x0f << KDQCR_B_RELAY);
			} else {
				flags &= ~(0x0f << KDQCR_B_RELAY);
			}
			*status_out = flags;
		} else {
			*status_out = 0xffffffff;
		}
		--count;
		++pmt;
		++status_out;
	}
	return(numLoaded);
}

