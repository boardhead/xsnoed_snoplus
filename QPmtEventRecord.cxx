////////////////////////////////////////////////////////////
// Facilitates conversion between PmtEventRecords and     //
// QEvents.                                               //
// - if a QCal object is defined, it will be used to fill //
//   in the calibrated PMT values.  Otherwise they will   //
//   be set to zero.                                      //
// - now uses calibrated PmtEventRecords - PH 01/13/99    //
////////////////////////////////////////////////////////////
//*-- Author :	Phil Harvey - 12/8/98

#include "QPmtEventRecord.h"
#include "QEvent.h"
#include "QCal.h"
#include "PZdabFile.h"
#include "QSNO.h"
#include "QMCVX.h"
#include "include/Record_Info.h"
#include "include/NcdDataTypes.h"
#include "TClonesArray.h"

#define MAX_SCOPE_TRACES    8

QMuxScope *tmpMux[MAX_SCOPE_TRACES] = { NULL };

ClassImp(QPmtEventRecord)

QPmtEventRecord::QPmtEventRecord()
{
	// QPmtEventRecord constructor
	if (!tmpMux[0]) {
	    for (int i=0; i<MAX_SCOPE_TRACES; ++i) {
	        tmpMux[i] = new QMuxScope();
	    }
	}
	// initialize member variables
	mBuffer = NULL;
	mPmtRecord = NULL;
	mBufferLength = 0;
	mBlockSize = 0;
	mExtendedFormat = kTRUE;
	mEvent = NULL;

	// calculate time offsets
	struct tm	tms;
	tms.tm_sec = 0;
	tms.tm_min = 0;
	tms.tm_hour = 0;
	tms.tm_mday = 1;
	tms.tm_mon = 0;
	tms.tm_year = 75;
	tms.tm_isdst = 0;
	root_time_zero = mktime(&tms);
	// correct for offset of one day (because Jan 1, 1975 is actually Julian date = 1)
	root_time_zero -= 24 * 60 * 60L;
	/* mktime returns UTC time given local time but sno time zero is UTC */
	/* so we must do a conversion from local to GMT here (no DST because we set isdst to zero) */
#ifdef __MACHTEN__
	root_time_zero -= 5 * 3600L;    // convert to GMT
#else
	root_time_zero -= timezone;     // convert to GMT
#endif

	tms.tm_year = 96;
	sno_time_zero = mktime(&tms);
#ifdef __MACHTEN__
	sno_time_zero -= 5 * 3600L;    // convert to GMT
#else
	sno_time_zero -= timezone;     // convert to GMT
#endif
}

QPmtEventRecord::~QPmtEventRecord()
{
	// QPmtEventRecord destructor
	// - deletes memory allocated to GenericRecordHeader and PmtEventRecord
	delete [] mBuffer;
	delete mEvent;
}

PmtEventRecord * QPmtEventRecord::Convert(QEvent *anEvent)
{
	// Convert from a QEvent to a PmtEventRecord
	// - PmtEventRecord returned will be deleted with this object
	FromQEvent(anEvent);
	return(GetPmtEventRecord());
}

QEvent * QPmtEventRecord::Convert(PmtEventRecord *per)
{
	// Convert from a PmtEventRecord to a QEvent
	// - QEvent returned will be deleted with this object
	if (!mEvent) {
		mEvent = new QEvent();
		if (!mEvent) return(NULL);
	}
	SetPmtEventRecord(per);
	ToQEvent(mEvent);
	return(mEvent);
}

Int_t QPmtEventRecord::GetRecordLen()
{
	// get length of PmtEventRecord
	return mBufferLength - sizeof(GenericRecordHeader);
}

void QPmtEventRecord::SetPmtEventRecord(PmtEventRecord *per)
{
	// Set pointer to PmtEventRecord used in ToQEvent()
	// - deletes current buffer if it exists
	// - byte ordering of input PmtEventRecord must be native
	// - can't call SwapBytes() for event records loaded this way
	
	AllocateBuffer(0);		// free buffer and reset required variables
	mPmtRecord = per;	// save pointer
}

Int_t QPmtEventRecord::GetRunNumber()
{
	// return the run number from the PmtEventRecord
	if (mPmtRecord) return(mPmtRecord->RunNumber);
	else return(0);
}

Int_t QPmtEventRecord::GetEventIndex()
{
	// return the event index number from the PmtEventRecord
	// Note: This is a serial index, NOT the GTID
	if (mPmtRecord) return(mPmtRecord->EvNumber);
	else return(0);
}

Bool_t QPmtEventRecord::AllocateBuffer(Int_t length)
{
	// Allocate buffer for GenericRecordHeader and PmtEventRecord
	// - returns true if allocated OK
	// - sets current PmtEventRecord to point into the buffer
	// - initializes GenericRecordHeader for PmtEventRecord
	// - frees memory and returns false if 'length' is zero
	// - uses existing buffer if length is less than buffer size
	
	if (length==0 || length>mBlockSize) {
		// only reallocate memory if the block must grow
		delete [] mBuffer;
		if (length) {
			mBuffer = new Text_t[length];
		} else {
			mBuffer = NULL;
		}
		if (mBuffer) {
			mBlockSize = length;
		} else {
			mBlockSize = 0;
			mBufferLength = 0;
			mPmtRecord = NULL;
			return(kFALSE);
		}
	}
	// fill in generic record header (do this every time in case it was byte-swapped)
	GenericRecordHeader	*grh = (GenericRecordHeader *)mBuffer;
	grh->RecordID = PMT_RECORD;
	grh->RecordLength = sizeof( aPmtEventRecord );
	grh->RecordVersion = PTK_RECORD_VERSIONS;
	
	// initialize pointer to PMT event record
	mPmtRecord = (PmtEventRecord *)(grh + 1);
	
	mBufferLength = length;	// save current buffer length
	
	return(kTRUE);
}

void QPmtEventRecord::Load(void *dataPtr, Int_t length)
{
	// load raw event into buffer (GenericRecordHeader plus PmtEventRecord)
	if (AllocateBuffer(length)) {
		memcpy(mBuffer, dataPtr, length);
	}
}

void QPmtEventRecord::SwapBytes()
{
	// byte swap the data if required for the host architecture
	// - only works for PmtEventRecords stored in local buffer
#ifdef SWAP_BYTES
	if (!mBuffer) return;	// nothing to do if no buffer
	
	// swap the GenericRecordHeader
	SWAP_INT32(mBuffer, 3);
  
	// The start of the PMT record carries the global event data.
	PmtEventRecord *per = (aPmtEventRecord *)(mBuffer + sizeof(GenericRecordHeader));
	
	// swap the PmtEventRecord
	SWAP_PMT_RECORD(per);

	// swap the FEC data
	// Also swaps any calibrated data sub-fields
	// BUT OTHER SUB_FIELDS WILL BE MESSED UP -- EVENTUALLY THESE MUST BE HANDLED
	// (may be very tricky to run through sub-fields because we don't know
	//  if the data is swapped or not)
	int	pmtWords = (mBufferLength - sizeof(GenericRecordHeader)
					- sizeof(PmtEventRecord)) / sizeof(u_int32);

	if (pmtWords > 0) {
		// Now swap all PMT bundles
		SWAP_INT32( per+1, pmtWords );
	}

#endif
}

void QPmtEventRecord::ToQEvent(QEvent *anEvent)
{
	// Fill out entries in specified QEvent from this PmtEventRecord
	// - uses global gCal object to calculate calibrated values

        int		 i, num;

	anEvent->Clear(0);	// clear event

	// nothing more to do if we don't have a PmtEventRecord to work from
	if (!mPmtRecord) return;

	u_int32 *mtc_word = (u_int32 *)&(mPmtRecord->TriggerCardData);

	// fill in event information
	double	theTime = (mPmtRecord->TriggerCardData.Bc10_2 * (double) 4294967296.0 +
					   mPmtRecord->TriggerCardData.Bc10_1) * 1e-7;

	double	time_50MHz = mtc_word[2] * (double)2048.0 + (mtc_word[1] >> 21);

	Int_t	snoDays = (Int_t)(theTime / (24 * 60 * 60L)); // days since SNO time zero
	Int_t	npmts = mPmtRecord->NPmtHit;

	anEvent->SetEvent_id(mPmtRecord->TriggerCardData.BcGT);
	anEvent->SetTrig_type(((mtc_word[3] & 0xff000000UL) >> 24) |
			   			  ((mtc_word[4] & 0x0007ffffUL) << 8));
	anEvent->SetNhits(npmts);
	anEvent->SetnPBUNs(npmts);
	if (theTime) {
		anEvent->SetJulianDate(snoDays + (Int_t)((sno_time_zero - root_time_zero + 0.5) / (24 * 60 * 60L)));
		anEvent->SetUT1((Int_t)(theTime - snoDays * (double)(24 * 60 * 60)));
		anEvent->SetUT2((Int_t)((theTime - (unsigned long)theTime) * 1e9));
	} else {
		anEvent->SetJulianDate(0);
		anEvent->SetUT1(0);
		anEvent->SetUT2(0);
	}
	anEvent->SetNph(0.0);	// <-- how is Nph derived????? !!!!!
	anEvent->SetGtr_time(time_50MHz / ((double)1e-9 * 50e6));

	// set analog trigger measurements
	anEvent->SetEsumPeak((Int_t)UNPK_MTC_PEAK(mtc_word));
	anEvent->SetEsumInt((Int_t)UNPK_MTC_INT(mtc_word));
	anEvent->SetEsumDiff((Int_t)UNPK_MTC_DIFF(mtc_word));

	// set run number and event index
	anEvent->SetRun(mPmtRecord->RunNumber);
	anEvent->SetEventIndex(mPmtRecord->EvNumber);

	// add hits
	u_int32 *thePmtHits = (u_int32 *)(mPmtRecord + 1);

	// initialize necessary variables before scanning sub-fields
	anEvent->SetBit(kCalibrated, kFALSE);
	CalibratedPMT	*theCalHits = NULL;
	MonteCarloHeader *monteCarloData = NULL;
	FittedEvent		*theFit = NULL;
	Int_t			numFits = 0;
	u_int32 *       ncdData = NULL;
	u_int32 *       ncdEnd;

	// run through the sub-fields, looking for additional information
	u_int32	*sub_header = &mPmtRecord->CalPckType;
	while (*sub_header & SUB_NOT_LAST) {
		sub_header += (*sub_header & SUB_LENGTH_MASK);
		switch (*sub_header >> SUB_TYPE_BITNUM) {
			case SUB_TYPE_CALIBRATED:
				anEvent->SetBit(kCalibrated, kTRUE);
				theCalHits = (CalibratedPMT *)(sub_header + 1);
				break;
			case SUB_TYPE_MONTE_CARLO:
				monteCarloData = (MonteCarloHeader *)(sub_header + 1);
				break;
			case SUB_TYPE_FIT:
				theFit = (FittedEvent *)(sub_header + 1);
				numFits = ( (*sub_header & SUB_LENGTH_MASK) * sizeof(u_int32)
							- sizeof(SubFieldHeader) ) / sizeof(FittedEvent);
				break;
		    case SUB_TYPE_NCD:
		        ncdData = sub_header + 1;
                ncdEnd = sub_header + (*sub_header & SUB_LENGTH_MASK);
		        break;
			default:
				// ignore unrecognized fields
				break;
		}
	}

	// fill in fitted event data
	if (theFit) {
		QFit qfit;
		for (int i=0; i<numFits; ++i, ++theFit) {
			qfit.SetX(theFit->x);
			qfit.SetY(theFit->y);
			qfit.SetZ(theFit->x);
			qfit.SetU(theFit->u);
			qfit.SetV(theFit->v);
			qfit.SetW(theFit->w);
			qfit.SetTime(theFit->time);
			qfit.SetQualityOfFit(theFit->quality);
			qfit.SetNumPMTsUsed(theFit->npmts);
			qfit.SetName(theFit->name);
			anEvent->AddQFIT(&qfit);	// add the fit to the event
		}
	}

	// fill in event monte carlo information
	if (monteCarloData) {
		QMCVX mcvx;
		num = monteCarloData->nVertices;
		MonteCarloVertex *theVertex = (MonteCarloVertex *)(monteCarloData + 1);
		for (i=0; i<num; ++i, ++theVertex) {
			mcvx.SetEnergy(theVertex->energy);
			mcvx.SetX(theVertex->x);
			mcvx.SetY(theVertex->y);
			mcvx.SetZ(theVertex->z);
			mcvx.SetU(theVertex->u);
			mcvx.SetV(theVertex->v);
			mcvx.SetW(theVertex->w);
			mcvx.SetIDP(theVertex->particle);
//			mcvx.SetBit(kReflected, (theVertex->flags&0x01) != 0);
			mcvx.SetINC(theVertex->int_code);
			mcvx.SetIndex(theVertex->parent);
			mcvx.SetTime(theVertex->time);
			anEvent->AddQMCVX(&mcvx);	// add monte carlo vertex to event
		}
	}

	QPMT aPmt;

	// fill in event PMT hits
	for (i=0; i<npmts; ++i,thePmtHits+=3) {

		aPmt.Setn(((UNPK_CRATE_ID(thePmtHits) * 16) + UNPK_BOARD_ID(thePmtHits)) * 32 +
					UNPK_CHANNEL_ID(thePmtHits));
		aPmt.SetCell(UNPK_CELL_ID(thePmtHits));
                aPmt.SetStatus(0);		// status
		aPmt.Setihl(UNPK_QHL(thePmtHits));
		aPmt.Setihs(UNPK_QHS(thePmtHits));
		aPmt.Setilx(UNPK_QLX(thePmtHits));
		aPmt.Setit(UNPK_TAC(thePmtHits));
		if (theCalHits) {
			aPmt.Sethl(theCalHits->qhl);	// calibrated qhl
			aPmt.Seths(theCalHits->qhs);	// calibrated qhs
			aPmt.Setlx(theCalHits->qlx);	// calibrated qlx
			aPmt.Sett(theCalHits->tac);	// calibrated tac
			++theCalHits;
		} else {
			aPmt.Sethl(0);	// calibrated qhl
			aPmt.Seths(0);	// calibrated qhs
			aPmt.Setlx(0);	// calibrated qlx
			aPmt.Sett(0);	// calibrated tac
		}

                aPmt.SetCMOSStatus(       UNPK_CELL_ID     (thePmtHits)  //bits 0-3
                                    | ( (UNPK_CGT_ES_16   (thePmtHits)<<4)&0x010 )
                                    | ( (UNPK_CGT_ES_24   (thePmtHits)<<5)&0x020 )
                                    | ( (UNPK_MISSED_COUNT(thePmtHits)<<6)&0x040 )
                                    | ( (UNPK_NC_CC       (thePmtHits)<<7)&0x080 )
                                    | ( (UNPK_LGI_SELECT  (thePmtHits)<<8)&0x100 )
                                    | ( (UNPK_CMOS_ES_16  (thePmtHits)<<9)&0x200 )  );


		anEvent->Add(&aPmt);	// add this PMT to the event
	}
	// fill in calibrated values if calibration not done and PCA object is available
	if (!theCalHits && gCal) {
		gCal->Calibrate(anEvent);
		anEvent->SetBit(kCalibrated, kTRUE);
	}
	// fill in the ncd information
	if (ncdData) {
	    u_int32 g_mux=0;
	    int n_mux=0, n_muxg=0, n_muxf=0, n_part=0;
	    int r_scope=0, n_scope=0;
	    double t_scope=0;
	    for (u_int32 *ncd=ncdData; ncd<ncdEnd; ++ncd) switch (*ncd & kNcdDataTypeMask) {
            case kNewRunRecordType:
                break;
            case kTrigTimeRecordType:
                anEvent->SetNCDClock1(*ncd & kClockUpper);          // Upper ( 24 ) Clock bits
                anEvent->SetNCDLatchReg((*ncd & kClockLatch) >> 24);// Latch Register ( 3 bits )
                ++ncd;
                anEvent->SetNCDClock2(*ncd);                        // Lower Clock bits
/*                if (um->jday < DELTA_JDAY + 365.25) {               // Jan 1, 1997  
                    printf( "Rootify : Rebuilding timestamp from NCD clock for event %d\n", ev_num );
                    clock2Stime( *p & 0xffffff, *p, &s_tim );
                    anEvent->SetJulianDate( s_tim.jday );
                    anEvent->SetUT1( s_tim.ut1 );
                    anEvent->SetUT2( s_tim.ut2 );
                }
*/
                break;
            case kGTIDRecordType:
                anEvent->SetNCDGTID(*ncd & kGTIDGtid);
                anEvent->SetNCDSync((*ncd & kGTIDSyncClear) >> 26);
//                if (um->jday < DELTA_JDAY + 364.25) {              // Jan 1, 1997
//                    printf( "Rootify : Replacing MTC GTID with NCD GTID [%d] for event %d\n", *p & 0xffffff, ev_num );
//                    anEvent->SetEvent_id( *p & 0xffffff );
//                }
                break;
            case kMuxRecordType: {
// removed - PH 07/19/04
//            	int scope_bits[] = { 0, 2, 1, 3 };  // Remapped scope bits
                ++n_mux;
                int card = (*ncd & kMuxBox) >> 23;                  // Mux Bus Number ( 4 bits wide )
                if( !g_mux ) {
                    n_part++;                                       // Move data to partial  chain
                }
                if( !( ( g_mux & 0xff ) & ( 1 << card ) ) ) n_part++; // Check mux bus number - move data to partial chain if not set
                QMuxScope *qmux = anEvent->AddMuxScope();
                qmux->SetStatusWord( n_part ? 0x2 : 0x3 );
                qmux->SetMuxChannelMask( *ncd & kMuxHitPattern );
// this looks wrong to me - PH 07/19/04
//                int n = scope_bits[ (*ncd & kMuxScope) >> 21 ];     // These bits are reversed ...
//                qmux->SetScopeNumber(n / 2);                        // Scope A : 0 / Scope B : 1
                qmux->SetScopeNumber((*ncd & kMuxScope1) ? 1 : 0);  // PH 07/19/04
                qmux->SetMuxBusNumber(card);
                qmux->SetMuxBoxNumber(-1);                          // Default is `undefined'
                qmux->SetNCDStringNumber(-1);
                qmux->SetMuxChannel(-1);
                int n;
                for (n=0; n<12; n++ ) {
                    if( ( *ncd & kMuxHitPattern ) & ( 1 << n ) ) break; // Find first mux channel that fired
                }
                if( n < 12 ) {                                              
                    if( !(((kMuxHitPattern << (n + 1)) & *ncd ) & kMuxHitPattern ) ) {  // Is it unique ?
                        qmux->SetMuxChannel(n);
                      }
                } else {
//                    printf( "Rootify : No Mux bits [0x%x] set at event %d !\n", *p, ev_num );
                }
                qmux->SetSizeOfScopeTrace(-1);                      // Put `undefined' values in these to avoid confusion
                qmux->SetLatchRegisterID(r_scope);                  // Should be -1
                qmux->SetClockRegister(t_scope);                    // Should be `-1'
                qmux->SetScopeChannel(-1);
                qmux->SetGlobalMuxRegister(g_mux);                  // - and save a sane copy of the global mux register
            }   break;
            case kScopeRecordType: {
                QMuxScope *qmux = NULL;
                ++n_scope;
                int n = (*ncd & kScopeId) >> 23;                    // Scope number
                if ( !g_mux || !(g_mux & (1 << (n + 10))) ) {
                    // Scope record without associated global mux record or mux bits
                    qmux = anEvent->AddMuxScope();
                    qmux->SetStatusWord(0x0);
                    qmux->SetScopeNumber(-1);
                    n_part++;                                       // Move data to partial muxscope chain
                }
                int ch = (*ncd & kScopeChannel) >> 19;              // Scope channel
                int words = ((*ncd & kScopeSize) + sizeof(u_int32) - 1) / sizeof(u_int32);
                char *scopeData = (char *) (ncd + 1);                      // Save address of scope trace
                //
                // check on scope triggered bits ??????
                //
                qmux = NULL;                                        // Search for first mux record without a scope
                for (int j=0; j<anEvent->GetnMuxScopes(); ++j) {
                    qmux = anEvent->GetMuxScope(j);
                    if (qmux->GetStatusWord() & 0x4) {
                        if ( (qmux->GetScopeNumber() == n) && (qmux->GetScopeChannel() == ch) ) {
//                            printf( "Rootify : Duplicate Scope [%d]/Channel [%d] Mux record at event %d\n", m, k, ev_num );
                        }
                    } else {
                        break;
                    }
                }
                if (!qmux) {
//                    printf( "Rootify : Stray Scope record [0x%x] at event %d\n", *p, ev_num );
                    qmux = anEvent->AddMuxScope();
                    qmux->SetStatusWord(0x0);
                    qmux->SetScopeNumber(-1);
                    n_part++;                                       // Move data to partial chain
                }
                if (qmux->GetScopeNumber() != n) {
//                    printf( "Rootify : Scope number mismatch at event %d !\n", ev_num );
                    n_part++;
                }
                qmux->SetStatusWord( qmux->GetStatusWord() | 0x4 );
                qmux->SetClockRegister(t_scope);
                qmux->SetSizeOfScopeTrace(words);
                qmux->SetLatchRegisterID(r_scope);
                qmux->SetScopeChannel(ch);
                // must un-swap the scope data
                SWAP_INT32(scopeData, words);
                qmux->GetValues()->Set(sizeof(u_int32) * words, scopeData);
                qmux->SetLogAmpParameter_a(85.0);           // Fill with default values for now
                qmux->SetLogAmpParameter_b(-0.1);            // Eventually we will stuff these 
                qmux->SetLogOffset(0.0);          // from NCLA banks (or the new kScopeCal
                qmux->SetLogAmpPreampRCFactor(50000);          // record if it exists)
                qmux->SetScopeOffset(56.0);
		ncd += words;
            }   break;
            case kScopeTimeRecordType:
                r_scope = (*ncd & kClockLatch) >> 24;       // Latch register ( 3 bits wide )
                t_scope = (double) 4294967296.0 * (*ncd & kClockUpper) + ncd[1];
                ++ncd;      // 2 words long
                break;
//            case kShGScalRecordType: {
//                QGlobalShaper *qglobalshaper = anEvent->AddGlobalShaper();
//                qglobalshaper->SetBoardInfo(*ncd & 0xf);            // 4 bits of board info
//                qglobalshaper->SetGlobalScaler(ncd[1] & 0xfffffff); // 28 bits of scaler info
//                ++ncd;      // 2 words long
//            }   break;
            case kMuxGlobalStatusType: {
                ++n_muxg;
//                if (g_mux) {
//                   printf( "Rootify : Error : Extra Mux Global Record [0x%x] at event %d - replacing active one [0x%x] !\n",
//                                    *p, ev_num, g_mux );
//                }
                int m = *ncd & kMuxFired;                   // Mux Fired bits
                g_mux = m | ((*ncd & 0xaa00) >> 1) | ((*ncd & 0x5500) << 1); 
                // Save Global Mux register ( reverse scope bits ! )
                while (m) {                                 // Count Mux Fired bits
                    if( m & 0x1 ) ++n_muxf;
                    m >>= 1;
                }
                m = (g_mux & 0x3000) >> 2;
//                if ( ( m & g_mux ) != m ) {
//                    printf( "Rootify : Inconsistent scope trigger/fired bits [0x%x] for events %d\n", g_mux, ev_num );
//                }
            }   break;
            case kShaperRecordType: {
                QADC *qadc = anEvent->AddShaper();
                qadc->SetADCCharge(*ncd & kShaperValue);          // 12 bits of Charge ADC
                int card = (*ncd & kShaperCard) >> 16;
                int chan = (*ncd & kShaperChan) >> 12;
                qadc->SetShaperChannelNumber(chan); 
                qadc->SetShaperSlotNumber(card);
		        qadc->SetShaperHardwareAddress(-1);
                qadc->SetNCDStringNumber(-1);
            }   break;
            case kGeneralDataType: {
                u_int32 *gen_end = ncd + (*ncd & 0xffff) / sizeof(u_int32);
                ++ncd;  // skip over general record header
                while (ncd < gen_end) {
                    switch (*ncd >> 16) {
                        case kNCDScopeCal: {
                            // Note: this assumes that this record comes after the MUX and scope records
                            int ch = ncd[1] & 0x0f;
                            int n = (ncd[1] & 0xf0) >> 4;
                            for (int j=0; j<anEvent->GetnMuxScopes(); ++j) {
                                QMuxScope *qmux = anEvent->GetMuxScope(j);
                                if ( (qmux->GetScopeNumber() == n) && (qmux->GetScopeChannel() == ch) ) {
                                    qmux->SetLogAmpParameter_a(*(float *)(ncd+2));
                                    qmux->SetLogAmpParameter_b(*(float *)(ncd+3));
                                    qmux->SetLogOffset(*(float *)(ncd+4));
                                    qmux->SetLogAmpPreampRCFactor(*(float *)(ncd+5));
                                    qmux->SetLogAmpElecDelayTime(*(float *)(ncd+6));
                                    qmux->SetScopeOffset(*(float *)(ncd+7));
                                }
                            }
                        } break;
                    }
                    // step to next generic sub-record
                    if (!(*ncd & 0xffff)) {
//                        printf("Bad NCD general sub-record!\n");
                        break;
                    }
                    ncd += (*ncd & 0xffff) / sizeof(u_int32);
                }
                ncd = gen_end;  // make sure we arrive at end of record
            }   break;
            default:
                break;
	    }
	    // finish associating data
//        if (n_muxf != n_mux) {
//            printf( "Rootify : Number of Mux records [%d] differ from number of bits set in Global Mux Register [%d] at event %d !\n", n_mux, n_muxf, ev_num );
//        }
        if (n_muxf < n_scope) {
//            printf( "Rootify : Found %d Mux records and %d Scope records at event %d !\n", n_mux, n_scope, ev_num );
        } else if (n_scope < n_muxf) {
            n_part++;                                                 // Force check for partial records
        }
        if (n_part) {
            int n = anEvent->GetnMuxScopes();
            if (n > MAX_SCOPE_TRACES) {
//                printf( "Rootify : Too many scope traces [%d] - increase MAX_SCOPE_TRACES !\n", l );
//                return( 1 );
            } else if (n) {
                int j;
                for (j=0; j<n; ++j) {
                    *tmpMux[j] = *anEvent->GetMuxScope(j);   // Use `copy' function to move data to temporary 
                }
                anEvent->GetMuxScopes()->Delete();          // Reset pointers and
                anEvent->SetnMuxScopes(0);                  // counters ...
                for (j=0; j<n; ++j) {
                    if ( (tmpMux[j]->GetStatusWord() & 0x7) == 0x7 ) { // Divide them up ...
                        anEvent->AddMuxScope(tmpMux[j]);
                    } else {
                        anEvent->AddPartialMuxScope(tmpMux[j]);
                    }
                }
            } else {
//                printf( "Rootify : 0 MuxScopes !\n" );    // This should never happen ..........
//                return( 1 );
            }
        }
	}
}

void QPmtEventRecord::FromQEvent(QEvent *anEvent)
{
	// Fill out entries in this PmtEventRecord from the specified QEvent
	// - if anEvent is NULL, free memory allocated to buffer
	// - allocates new buffer for and sets current PmtEventRecord to point into the buffer

	Int_t	i, num, npmts, length;
	Int_t   ncd_length = 0;
	QPMT 	*aPmt;
    QMuxScope *qmux;
	CalibratedPMT *calPmt = NULL;

	if (anEvent) {

		npmts  = anEvent->GetnAny();

		// calculate required buffer length
		length = sizeof(GenericRecordHeader) + sizeof(PmtEventRecord) +
				  npmts * sizeof(FECReadoutData);

        if (mExtendedFormat) {
		    // add room for calibrated data if available
#ifdef UNRELIABLE_QEVENT_CALIBRATED_FLAG
		    if (1)
#else
		    if (anEvent->IsCalibrated())
#endif
            {
                length += npmts * sizeof(CalibratedPMT) + sizeof(SubFieldHeader);
            }
            // add room for monte carlo data if available
            if (anEvent->GetnMCVXs()) {
                length += anEvent->GetnMCVXs() * sizeof(MonteCarloVertex) +
                          sizeof(MonteCarloHeader) + sizeof(SubFieldHeader);
            }
            // add room for fits
            if (anEvent->GetnFITs()) {
                length += anEvent->GetnFITs() * (sizeof(FittedEvent) + sizeof(SubFieldHeader));
            }
            // add room for event energies
            if (anEvent->GetnRSPs()) {
                length += anEvent->GetnRSPs() * (sizeof(ExtraEventData) + sizeof(SubFieldHeader));
            }
            // add room for NCD data
            if (anEvent->GetnShapers() || anEvent->GetnMuxScopes() || anEvent->GetnPartialMuxScopes()) {
                // kGTIDRecordType
                ncd_length += sizeof(u_int32);
                // kTrigTimeRecordType [2 words]
//                ncd_length += 2 * sizeof(u_int32);
                if (anEvent->GetnShapers()) {
                    // kShaperRecordType
                    ncd_length += anEvent->GetnShapers() * sizeof(u_int32);
                }
                if (anEvent->GetnMuxScopes() || anEvent->GetnPartialMuxScopes()) {
                    // kMuxGlobalStatusType
                    ncd_length += sizeof(u_int32);
                    // kMuxRecordType
                    ncd_length += (anEvent->GetnMuxScopes() + anEvent->GetnPartialMuxScopes()) * sizeof(u_int32);
                    // add scope records
                    if (anEvent->GetnMuxScopes()) {
                        // kScopeTimeRecordType [2 words]
                        ncd_length += 2 * sizeof(u_int32);
                        for (i=0; i<anEvent->GetnMuxScopes(); ++i) {
                            qmux = anEvent->GetMuxScope(i);
                            int words = qmux->GetSizeOfScopeTrace();
                            // kScopeRecordType [one word + scope data]
                            ncd_length += (1 + words) * sizeof(u_int32);
                        }
                        // kNCDScopeCal (generic records with 7 words of data plus headers)
                        ncd_length += (1 + anEvent->GetnMuxScopes() * 8) * sizeof(u_int32);
                    }
                }
                // kShGScalRecordType [2 words]
//                ncd_length += anEvent->GetNumGlobalShapers() * sizeof(u_int32);
                // add size of extended NCD record to total event length
                length += ncd_length + sizeof(SubFieldHeader);
            }
        }
	} else {
		// force de-allocation of existing buffer
		npmts = length = 0;
	}

	if (!AllocateBuffer(length)) return;	// return if no buffer was allocated

	/* get pointer to first MTC and FEC words */
	u_int32 *mtc_word = (u_int32 *)&(mPmtRecord->TriggerCardData);
	u_int32 *fec_word = (u_int32 *)(mPmtRecord + 1);
	mPmtRecord->RunNumber = anEvent->GetRun();
	mPmtRecord->EvNumber = anEvent->GetEventIndex();
	mPmtRecord->NPmtHit = npmts;

	mPmtRecord->PmtEventRecordInfo = PMT_EVR_RECTYPE | PMT_EVR_NOT_MC | PMT_EVR_ZDAB_VER ;
	mPmtRecord->DataType = PMT_EVR_DATA_TYPE;
	mPmtRecord->DaqStatus = 0; // now used to store sub-run number (was PMT_EVR_DAQ_STAT)
	mPmtRecord->CalPckType = PMT_EVR_PCK_TYPE | PMT_EVR_CAL_TYPE;

	if (mExtendedFormat) {	// ==== Start of sub-field extensions - PH 02/25/99
	
		u_int32 *sub_header = &mPmtRecord->CalPckType;
		
		// must set the size of this sub-field before calling AddSubField()
		// (from the sub-field header to the end)
		*sub_header |= ((u_int32 *)(fec_word + npmts * 3) - sub_header);
		
#ifdef UNRELIABLE_QEVENT_CALIBRATED_FLAG
		if (1)
#else
		// add calibrated hit information
		if (anEvent->IsCalibrated())
#endif
		{
			// add new sub-field to extended PmtEventRecord
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_CALIBRATED, npmts * sizeof(CalibratedPMT));
			// get pointer to start of calibrated data for later
			calPmt = (CalibratedPMT *)(sub_header + 1);
		}
		
		// add monte carlo data
		if ((num = anEvent->GetnMCVXs()) != 0) {
			// add new sub-field to extended PmtEventRecord
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_MONTE_CARLO,
							sizeof(MonteCarloHeader) + num * sizeof(MonteCarloVertex));
			// get pointer to start of monte-carlo data and vertices
			MonteCarloHeader *monteCarloData = (MonteCarloHeader *)(sub_header + 1);
			MonteCarloVertex *theVertex = (MonteCarloVertex *)(monteCarloData + 1);
			// fill in the monte carlo data values
			monteCarloData->nVertices = num;
			for (i=0; i<num; ++i, ++theVertex) {
				QMCVX *mcvx = (QMCVX *)anEvent->GetQMCVXs()->At(i);
				theVertex->energy = mcvx->GetEnergy();
				theVertex->x = mcvx->GetX();
				theVertex->y = mcvx->GetY();
				theVertex->z = mcvx->GetZ();
				theVertex->u = mcvx->GetU();
				theVertex->v = mcvx->GetV();
				theVertex->w = mcvx->GetW();
				theVertex->particle = mcvx->GetIDP();
//				theVertex->flags = mcvx->TestBit(kReflected) ? 0 : 0x01;
				theVertex->int_code = mcvx->GetINC();
				theVertex->parent = mcvx->GetIndex();
				theVertex->time = mcvx->GetTime();
			}
		}
		
		// add all available fits
		num = anEvent->GetnFITs();
		for (i=0; i<num; ++i) {
			// add new sub-field to extended PmtEventRecord
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_FIT, sizeof(FittedEvent));
			// get pointer to start of fit data
			FittedEvent *theFit = (FittedEvent *)(sub_header + 1);
			QFit *qfit = (QFit *)anEvent->GetQFITs()->At(i);
			theFit->x = qfit->GetX();
			theFit->y = qfit->GetY();
			theFit->z = qfit->GetZ();
			theFit->u = qfit->GetU();
			theFit->v = qfit->GetV();
			theFit->w = qfit->GetW();
			theFit->time = qfit->GetTime();
			theFit->quality = qfit->GetQualityOfFit();
			theFit->npmts = qfit->GetNumPMTsUsed();
			theFit->spare = 0;
			const char *pt = qfit->GetName();
			char buff[256];
			if (!pt) pt = "<null>";
			sprintf(buff,"%s-%d", pt, (int)qfit->GetIndex());
			memset(theFit->name, 0, 32);	// initialize the name to all zeros
			strncpy(theFit->name,buff,31);	// copy fit name (31 chars max)
		}
		
		// add energy calibration from first available QRSP
		num = anEvent->GetnRSPs();
		for (i=0; i<num; ++i) {
			QRSP *rsp = anEvent->GetRSP(i);
			if (!rsp) continue;
			// add new sub-field to extended PmtEventRecord
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_EVENT_DATA, sizeof(ExtraEventData));
			// get pointer to ExtraEventData structure
			ExtraEventData *eventData = (ExtraEventData *)(sub_header + 1);
			// fill in the value
			eventData->value = rsp->GetEnergy();
			// initialize the name to null
			memset(eventData->name, 0, DATA_NAME_LEN);
			// fill in the name
			int len;
			if (i) {
				len = sprintf(eventData->name, "Energy%d", i+1);
			} else {
				len = sprintf(eventData->name, "Energy");
			}
			// add the format specification (following the name string)
			strcpy(eventData->name+len+1, "%.2f");
		}
		
		// add NCD data
        if (ncd_length) {
            // Note: must be very careful that we have accounted for everything
            // that we will add below when we calculated ncd_length above!
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_NCD, ncd_length);
			u_int32 *ncd = sub_header + 1;
            
            // add kGTIDRecordType record
            *(ncd++) = kGTIDRecordType |
                       (anEvent->GetNCDGTID() & kGTIDGtid) |
                       ((anEvent->GetNCDSync() << 26) & kGTIDSyncClear);
            
            // add kTrigTimeRecordType record
//            *(ncd++) = kTrigTimeRecordType | 
//                       (anEvent->GetNCDClock1() & kClockUpper) |
//                       ((anEvent->GetNCDLatchReg() << 24) & kClockLatch);
//            *(ncd++) = anEvent->GetNCDClock2();
            
            // add kShaperRecordType records
            for (i=0; i<anEvent->GetnShapers(); ++i) {
                QADC *qadc = anEvent->GetShaper(i);
                *(ncd++) = kShaperRecordType |
                           (qadc->GetADCCharge() & kShaperValue) |
                           ((qadc->GetShaperChannelNumber() << 12) & kShaperChan) |
                           ((qadc->GetShaperSlotNumber() << 16) & kShaperCard);
            }
            if (anEvent->GetnMuxScopes() || anEvent->GetnPartialMuxScopes()) {
                if (anEvent->GetnMuxScopes()) {
                    qmux = anEvent->GetMuxScope(0);
                } else {
                    qmux = anEvent->GetPartialMuxScope(0);
                }
                u_int32 g_mux = qmux->GetGlobalMuxRegister();
                // add kMuxGlobalStatusType record
                *(ncd++) = kMuxGlobalStatusType |
                           (g_mux & kMuxFired) |
                           ((g_mux << 1) & 0xaa00) |
                           ((g_mux >> 1) & 0x5500);
                // add kMuxRecordType records
                for (i=0; i<anEvent->GetnMuxScopes(); ++i) {
                    qmux = anEvent->GetMuxScope(i);
                    *(ncd++) = kMuxRecordType |
                               ((qmux->GetMuxBusNumber() << 23) & kMuxBox) |
                               (qmux->GetScopeNumber() ? kMuxScope1 : kMuxScope0) |
                               (qmux->GetMuxChannelMask() & kMuxHitPattern);
                }
                for (i=0; i<anEvent->GetnPartialMuxScopes(); ++i) {
                    qmux = anEvent->GetPartialMuxScope(i);
                    *(ncd++) = kMuxRecordType |
                               ((qmux->GetMuxBusNumber() << 23) & kMuxBox) |
                               (qmux->GetScopeNumber() ? kMuxScope1 : kMuxScope0) |
                               (qmux->GetMuxChannelMask() & kMuxHitPattern);
                }
                // add kScopeTimeRecordType record
                if (anEvent->GetnMuxScopes()) {
                    qmux = anEvent->GetMuxScope(0);
                    int r_scope = qmux->GetLatchRegisterID();
                    double t_scope = qmux->GetClockRegister();
                    u_int32 clock_upper = (u_int32)(t_scope / 4294967296.0);
                    *(ncd++) = kScopeTimeRecordType |
                               ((r_scope << 24) & kClockLatch) |
                               (clock_upper & kClockUpper);
                    *(ncd++) = (u_int32)(t_scope - clock_upper * 4294967296.0 + 0.5);
                }
                // add kScopeRecordType records
                for (i=0; i<anEvent->GetnMuxScopes(); ++i) {
                    qmux = anEvent->GetMuxScope(i);
                    int words = qmux->GetSizeOfScopeTrace();
                    // kScopeRecordType [one word + scope data]
                    *(ncd++) = kScopeRecordType |
                               ((qmux->GetScopeChannel() << 19) & kScopeChannel) |
                               ((qmux->GetScopeNumber() << 23) & kScopeId) |
                               ((words * sizeof(u_int32)) & kScopeSize);
                    memcpy(ncd, qmux->GetValues()->GetArray(), words * sizeof(u_int32));
                    // must swap the scope data
                    SWAP_INT32(ncd, words);
                    ncd += words;
                }
                // add kNCDScopeCal records
                if (anEvent->GetnMuxScopes()) {
                    *(ncd++) = kGeneralDataType | ((1 + anEvent->GetnMuxScopes() * 7) * sizeof(u_int32));
                    for (i=0; i<anEvent->GetnMuxScopes(); ++i) {
                        qmux = anEvent->GetMuxScope(i);
                        *(ncd++) = (kNCDScopeCal << 16) | (7 * sizeof(u_int32));
                        *(ncd++) = qmux->GetScopeChannel() | (qmux->GetScopeNumber() << 4);
                        *((float *)ncd++) = qmux->GetLogAmpParameter_a();
                        *((float *)ncd++) = qmux->GetLogAmpParameter_b();
                        *((float *)ncd++) = qmux->GetLogOffset();
                        *((float *)ncd++) = qmux->GetLogAmpPreampRCFactor();
                        *((float *)ncd++) = qmux->GetLogAmpElecDelayTime();
                        *((float *)ncd++) = qmux->GetScopeOffset();
                    }
                }
            }
            Int_t filled_length = (Int_t)((ncd - sub_header - 1) * sizeof(u_int32));
            if (ncd_length != filled_length) {
                printf("NCD length error! Reserved %d bytes but filled %d!\n",
                        (int)ncd_length, (int)filled_length);
                if (filled_length > ncd_length) {
                    printf("WARNING: This has resulted an a memory overrun! You should restart ROOT!\n");
                }
            }
        }

	} // ==== End of sub-field extensions

	u_int32 gtid = anEvent->GetEvent_id();
	u_int32 trigger = anEvent->GetTrig_type();

	// add 0.5 to avoid round-off errors when we convert to integer
	// also, be careful to add UT1 and UT2 AFTER converting to SNO time to avoid loss in precision
	double time_10mhz = ((anEvent->GetJulianDate() * (double)(24 * 60 * 60) + (root_time_zero - sno_time_zero)) +
						 anEvent->GetUT1() + anEvent->GetUT2() * (double)(1e-9) ) * 1e7 + 0.5;

	double time_50mhz = anEvent->GetGtr_time() * ((double)1e-9 * 50e6) + 0.5;
	
	u_int32 hi10mhz = (u_int32)(time_10mhz / 4294967296.0);
	u_int32 hi50mhz = (u_int32)(time_50mhz / 2048.0);
	
	u_int32 peak = anEvent->GetEsumPeak() & 0x03ff;
	u_int32 inte = anEvent->GetEsumInt()  & 0x03ff;
	u_int32 diff = anEvent->GetEsumDiff() & 0x03ff;

	// MTC word 0
	*(mtc_word++) = (u_int32)(time_10mhz - hi10mhz * 4294967296.0);
	// MTC word 1
	*(mtc_word++) = ((u_int32)(time_50mhz - hi50mhz * 2048.0) << 21) | hi10mhz;
	// MTC word 2
	*(mtc_word++) = hi50mhz;
 		// MTC word 3
	*(mtc_word++) = ((trigger & 0x000000ffUL) << 24) | gtid;
	// MTC word 4
	*(mtc_word++) = ((trigger & 0x07ffff00UL) >> 8) | (peak << 19) | (diff << 29);
	// MTC word 5
	*(mtc_word++) = (inte << 7) | (diff >> 3);

	for (i=0; i<npmts; ++i) {

		aPmt = anEvent->GetAnyPMT(i);

		// FEC word 0
		*(fec_word++) = (((u_int32)aPmt->GetCard()) << 26) |
				   		(((u_int32)aPmt->GetCrate()) << 21) |
				   		(((u_int32)aPmt->GetChannel()) << 16) |
				   		(gtid & 0x0000ffffUL);
		// FEC word 1
		*(fec_word++) = (((u_int32)aPmt->Getihs() ^ 0x800) << 16) |
				   		(((u_int32)aPmt->GetCell()) << 12) |
				   		(((u_int32)aPmt->Getilx() ^ 0x800));
		// FEC word 2
		*(fec_word++) = ((gtid & 0x00f00000UL) << 8) |
				   		(((u_int32)aPmt->Getit() ^ 0x800) << 16) |
				   		((gtid & 0x000f0000UL) >> 4) |
				   		(((u_int32)aPmt->Getihl() ^ 0x800));

		/* fill in calibrated PmtEventRecord entries if available */
		if (calPmt) {
			calPmt->tac = aPmt->Gett();
			calPmt->qhs = aPmt->Geths();
			calPmt->qhl = aPmt->Gethl();
			calPmt->qlx = aPmt->Getlx();
			++calPmt;
		}
	}
}
