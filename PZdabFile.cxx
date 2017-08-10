/*
 * File:		PZdabFile.cxx - zdab read functions
 *
 * Author:		P. Harvey (some code from Yuendat Chan's CM_ZDAB.cp)
 *
 * Revisions:	06/26/98 - PH Created
 *				01/29/99 - PH Upgraded to a C++ object
 *				11/26/99 - PH Added ability to read MAST bank records
 *				12/01/99 - PH Generalized to remove MAST-specific knowledge
 *              11/18/04 - PH Fixed reading problem by updating from snobuilder version
 *
 * Notes:		ZDAB external format is big-endian.
 *				ZDAB native format is platform dependent.
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "PZdabFile.h"
#include "CUtils.h"
#include "SnoStr.h"
#include "include/Record_Info.h"

//#define DEBUG_RECORD_HEADERS
//#define DEBUG_EXTENDED_ZDAB


/* constants */
#define BASE_BUFFSIZE		32768UL		// base size of zdab record buffer
#define MAX_BUFFSIZE		0x400000UL	// maximum size of zdab record buffer (4 MB)
		
// the builder won't put out events with NHIT > 10000
// (note that these are possible due to hardware problems)
// but XSNOED can write an event with up to 10240 channels
#define MAX_NHIT			10240

// static member declarations
#ifdef DEBUG_RECORD_HEADERS
int PZdabFile::sVerbose = 1;
#else
int PZdabFile::sVerbose = 0;
#endif


//-------------------------------------------------------------------------------
// PZdabFile constructor
//
PZdabFile::PZdabFile()
{
	mRecBuffsize 	= 0;
	mFile			= NULL;
	mWordOffset		= 0;
	mBlockCount		= 0;
	mRecordCount	= 0;
	mBufferEmpty	= 0;
	mRecBuffer		= 0;
	mRecBuffsize	= 0;		// size of temporary ZDAB buffer
	mBuffPtr32		= 0;
	mBytesRead		= 0;
	mWordsTotal		= 0;
	mBytesTotal		= 0;
	mLastGTID		= 0;
	mLastRecord		= NULL;
}

PZdabFile::~PZdabFile()
{
	Free();
}

void PZdabFile::Free()
{
	if (mRecBuffsize) {
		free(mRecBuffer);
		mRecBuffer = NULL;
		mRecBuffsize = 0;
	}
}

// initialize for reading from zdab file
// returns < 0 on error
int PZdabFile::Init( FILE *inFile )
{
	mFile = inFile;
	if( inFile && !fseek( inFile, 0L, SEEK_SET ) ) {
		mWordOffset = 0;
		mBlockCount = 0;
		mRecordCount= 0;
		mBufferEmpty = 1;
		mLastGTID = 0;
		mLastRecord = NULL;
		// set up zdab record buffer if not already done
		if (!mRecBuffsize) {
			mRecBuffsize = BASE_BUFFSIZE;
			mRecBuffer = (u_int32 *)malloc(mRecBuffsize * sizeof(u_int32));
			if (!mRecBuffer) {
				mRecBuffsize = 0;
				Printf("Out of memory for zdab record buffer!\x07\n");
				return( -1 );
			}
		}
		return( 0 );
	} else {
		return( -1 );
	}
}

// NextRecord - get next record in ZDAB file (based on code by Yuen-Dat Chan)
// Returns: pointer to nZDAB record (native format) with trailing data (external format)
nZDAB *PZdabFile::NextRecord()
{
	int				n;
	u_int32			recLength, recType, nb_to_read, nw_count, block_size;
	u_int32			*skip32Ptr, *new_buffer, new_buffsize;
	nZDABPtr 		nzdabPtr;
	CONTROLPtr 		controlPtr;
	PILOTPtr 		pilotPtr;
	pilotHeaderPtr 	pHPtr;
	ZEBRA_ST		daqST;			// Steering block control words (ZEBRA FZ must)
	
	if (!mFile) return(0);
/*
** return the next bank within this physical record if available - PH 12/01/99
** - on any error, drop through to read the next record from file
*/
	if ((nzdabPtr = mLastRecord) != NULL) {
	
		// reset our last record pointer
		mLastRecord = NULL;
		
		// get pointer to first word after bank data (the i/o control word)
		u_int32 *ioControlPtr = (u_int32 *)(nzdabPtr + 1) + nzdabPtr->data_words;
		
		// make sure the ioControlPtr is in the buffer
		if (ioControlPtr < mBuffPtr32) {
		
			// swap the i/o control word
			SWAP_INT32(ioControlPtr,1);
			
			// get length of the next zdab header (lower 16 bits of i/o control word)
			// - for some reason this is actually 3 greater than the actual
			// size of the header which is a minimum of 9 words
			// (not counting the i/o control word as part of the header)
			u_int32 hdrLen = (*ioControlPtr & 0x0000ffffUL);
			
			// make sure the header is at least the minimum size
			if (hdrLen >= 12) {
			
				// get pointer to next bank header
				nzdabPtr = (nZDAB *)(ioControlPtr + hdrLen - 2) - 1;
				
				// make sure the next header is contained in the buffer
				if ((u_int32 *)(nzdabPtr+1) <= mBuffPtr32) {
				
					// swap the next zdab header
					SWAP_INT32(nzdabPtr, NZDAB_WORD_SIZE);
					
					// make sure the data is contained in the buffer
					if ((u_int32 *)(nzdabPtr+1)+nzdabPtr->data_words <= mBuffPtr32) {
					
						if (sVerbose > 1) {
							printf("-- same block --\n");
						}
						// success!! -- we have a good zdab record.
						// save the pointer in mLastRecord and return it
						return(mLastRecord = nzdabPtr);
					}
				}
			}
		}
	}
/*
** loop through Zebra records from file
*/
	while( 1 ) {
	
		if( mBufferEmpty ) {   // need to read in new buffer
		
			n = fread( &daqST, sizeof(daqST), 1, mFile );
			if (n != 1) {
				Printf("Unexpected EOF while reading zdab file!\x07\n");
				return(0);
			}
			SWAP_INT32( &daqST, 8 );
			
			/* check zebra signature - PH 07/03/98 */
			if( daqST.MPR[0] != ZEBRA_SIG0 || daqST.MPR[1] != ZEBRA_SIG1 ||
				daqST.MPR[2] != ZEBRA_SIG2 || daqST.MPR[3] != ZEBRA_SIG3 )
			{
				Printf("Invalid ZEBRA steering block!\x07\n");
				return(0);
			}
				
			if( daqST.MPR[4] & ( ZEBRA_EMERGENCY_STOP | ZEBRA_END_OF_RUN ) ) {
				Printf("ZEBRA EOF after [%ld] blocks and [%ld] records\n",
							 (long)mBlockCount, (long)mRecordCount );
				return(0);
			}
			block_size = daqST.MPR[4] & ZEBRA_BLOCK_SIZE_MASK;  //Phys. rec. length
			
			if( block_size > ZEBRA_BLOCKSIZE ) { 
				Printf("Illegal ZEBRA blocksize\x07\n");
				return(0);
			} else {

				// subtract steering length -> real data length
				// - account for fast blocks (MPR(7)) - PH 07/03/98
				nw_count = block_size * ( 1 + daqST.MPR[7] ) - 8;
			}
			
			if( daqST.MPR[5] != mBlockCount ) {
				Printf("Wrong ZEBRA bank number: %ld (should be %ld)\n",
							(long)daqST.MPR[5], (long)mBlockCount );
			}
			mBlockCount++;

			if( ( mWordsTotal = nw_count + mWordOffset ) > mRecBuffsize ) {
				if (mWordsTotal < BASE_BUFFSIZE) {
					new_buffsize = BASE_BUFFSIZE;
				} else if (mWordsTotal > MAX_BUFFSIZE) {
					Printf("ZDAB record too large! (%ld)  (corrupted file?)\x07\n",
							(long)mWordsTotal);
					return(0);
				} else {
					new_buffsize = mWordsTotal;
				}
				new_buffer = (u_int32 *)malloc(new_buffsize * sizeof(u_int32));
				if (!new_buffer) {
					Printf("Out of memory for ZDAB record buffer!\x07\n");
					return(0);
				}
				// copy any old zdab data into new (larger) buffer
				if (mWordOffset) {
					memcpy(new_buffer, mRecBuffer, mWordOffset * sizeof(u_int32));
				}
				// install new buffer
				Free();	// free old zdab buffer
				mRecBuffer = new_buffer;
				mRecBuffsize = new_buffsize;
			}
			n = fread( mRecBuffer+mWordOffset, sizeof(u_int32), nw_count, mFile );
			if ((u_int32)n != nw_count) {
				if (!n) {
					Printf("Unexpected EOF while reading zdab file!\x07\n");
					return(0);
				}
				nw_count = n;
				mWordsTotal = nw_count + mWordOffset;
			}
			mBuffPtr32 = mRecBuffer;
			mWordOffset = 0;
			mBytesRead = 0;
			mBytesTotal = mWordsTotal * sizeof(u_int32);
			mBufferEmpty = 0;

		} else {	// still has data in buffer, search for zdab banks   

	        // make sure we have enough data for the pilot header
			if( mBytesRead + sizeof(pilotHeader) < mBytesTotal ) {
				if( *mBuffPtr32 == 0 ) {                  //Handle 1 word padding records
					nb_to_read = mBytesRead + sizeof(u_int32); //before swapping ...
					if( nb_to_read <= mBytesTotal ) {
						mBuffPtr32 += 1;
						mBytesRead = nb_to_read;
						continue;
					}
				}

				pHPtr = (pilotHeaderPtr)mBuffPtr32;
				SWAP_INT32( pHPtr, 12 );
				controlPtr = &( pHPtr->control );
				pilotPtr = &( pHPtr->pilot );
				recLength = (u_int32)( controlPtr->length );
				recType = (u_int32)( controlPtr->recordtype );
				
				// handle different record types
				if( recType == 5 ) {
					nb_to_read = mBytesRead + ( recLength + 1 ) * sizeof(u_int32);
					if( nb_to_read <= mBytesTotal ) {						
						mBuffPtr32 += ( recLength + 1 );
						mBytesRead = nb_to_read;
						SWAP_INT32( pHPtr, 12 );     //undo swapping .....
						continue;
					}
				} else if( recType == 2 || recType == 3 || recType == 4 ) {  // normal data
					nb_to_read = mBytesRead + ( recLength + 2 ) * sizeof(u_int32);
					if( nb_to_read <= mBytesTotal ) {
						// calculate pointer to start of zdab event
						// skipping over control and pilot blocks, and pilot6 + pilot9 words
						skip32Ptr = mBuffPtr32 + 12 + pilotPtr->pilot6 + pilotPtr->pilot9;
						
						/* new addition 07/03/98 */
						if( skip32Ptr < mRecBuffer || skip32Ptr >= mRecBuffer+mRecBuffsize ) {
							Printf("Error 1 reading zdab file\x07\n");
							return(0);
						}
						SWAP_INT32( skip32Ptr, 1 );	/* swap zdab offset word */
						
						/* get pointer to start of zdab record header */
						skip32Ptr += ( *skip32Ptr & 0x0000ffff ) - 12 + 1;

						/* range check pointer again */
						if( skip32Ptr < mRecBuffer || skip32Ptr > mRecBuffer+mRecBuffsize-9 ) {
							Printf("Error 2 reading zdab file\x07\n");
							return(0);
						}
						
						nzdabPtr = (nZDAB *)skip32Ptr;
						
						mBuffPtr32 += ( recLength + 2 );	// set up for next location
						mBytesRead = nb_to_read;
						
						// make sure the bank header is contained in our buffer
						if ((u_int32 *)(nzdabPtr+1) > mBuffPtr32) {
							Printf("Error 3 reading zdab file\x07\n");
							return(0);
						}
						SWAP_INT32(nzdabPtr, 9);	// swap the zdab header
						
						// make sure the bank data is contained in our buffer
						if ((u_int32 *)(nzdabPtr+1)+nzdabPtr->data_words > mBuffPtr32) {
							Printf("Error 4 reading zdab file\x07\n");
							return(0);
						}
						
						// keep track of number of physical records found
						++mRecordCount;
						
						// Done! -- save the pointer in mLastRecord and return it
						return(mLastRecord = nzdabPtr);
					}
				} else if( recType == 1 ) {
					nb_to_read = mBytesRead + ( recLength + 2 ) * sizeof(u_int32);
					if( nb_to_read <= mBytesTotal ) {						
						mBuffPtr32 += ( recLength + 2 );
						mBytesRead = nb_to_read;
						SWAP_INT32( pHPtr, 12 );     //undo swapping .....
						continue;
					}
				} else {
					Printf("Unknown record type 0x%lx, length 0x%lx\x07\n",
								(long)recType, (long)recLength );
					return(0);
				}
				// swap back pHPtr because we're going to try again
				SWAP_INT32( pHPtr, 12 );
			}
			// calculate word offset of end of remaining record in buffer
			mWordOffset = ( mBytesTotal - mBytesRead ) / sizeof(u_int32);
			// quit now if our remaining record is too large for the buffer (double check)
			if( (u_int32)(mWordOffset + mBuffPtr32 - mRecBuffer) > mRecBuffsize ) {
				Printf("Record too large!\x07\n");
				return(0);
			}				
			// move remaining data to the beginning of buffer 
			memmove( (char *)mRecBuffer, (char *)mBuffPtr32, mWordOffset * sizeof(u_int32) );	
			mBufferEmpty = 1;
		}								
	}
}


// get pointer to next PmtEventRecord in zdab file
// Returns: pointer to PmtEventRecord (native format) or NULL on error or EOF
PmtEventRecord *PZdabFile::NextPmt()
{
	nZDABPtr 		nzdabPtr;
	PmtEventRecord*	pmtRecord = NULL;
	
	do {
		nzdabPtr = NextRecord();
		if (!nzdabPtr) break;	// give up if no more records in file
		pmtRecord = GetPmtRecord(nzdabPtr);
	} while (!pmtRecord);		// loop until we find a pmt record
	
	return(pmtRecord);
}


// NextBank - Return the next specified zdab bank from file
// Returns: pointer to bank data (native format) or NULL if no more matching banks
u_int32 *PZdabFile::NextBank(u_int32 bank_name)
{
	u_int32 *dataPt = NULL;
	  
	do {
		nZDAB *nzdabPtr = NextRecord();
		if (!nzdabPtr) break;
		dataPt = GetBank(nzdabPtr, bank_name);
	} while (!dataPt);

	return(dataPt);
}


// GetPmtRecord - analyze this nZDAB record
// Accepts: pointer to nZDAB record (external format)
// Returns: pointer to the PmtEventRecord (native format) if it has one. 
//          Otherwise, returns NULL
// Swaps: PmtEventRecord to native format
PmtEventRecord *PZdabFile::GetPmtRecord(nZDAB *nzdabPtr)
{
	PmtEventRecord *pmtEventPtr;
	
	// test the bank name
	if (nzdabPtr->bank_name == ZDAB_RECORD) { // extract 'ZDAB' PMT records
	
		if (sVerbose > 2) {
			DumpHex(nzdabPtr);
		}
		pmtEventPtr = (PmtEventRecord *)(nzdabPtr + 1);
		
		SWAP_PMT_RECORD( pmtEventPtr );		// swap the PmtEventRecord into native format

		int npmt = pmtEventPtr->NPmtHit;
		
		if (npmt > MAX_NHIT) {
		
			Printf("Read error: Bad ZDAB -- %d pmt hit!\x07\n", npmt );
			pmtEventPtr = NULL;	// not a valid PmtEventRecord
			
		}  else {
		
			// swap the hit data
			SWAP_INT32( pmtEventPtr + 1, 3 * npmt );
#ifdef DEBUG_EXTENDED_ZDAB
			static int count = 0;
			Printf("ZDAB %2d) %d hits\n",++count,npmt);
#endif			
			// swap the sub-fields
			u_int32	*sub_header = &pmtEventPtr->CalPckType;
			while (*sub_header & SUB_NOT_LAST) {
				sub_header += (*sub_header & SUB_LENGTH_MASK);
				SWAP_INT32( sub_header, 1 );	// swap the sub-field header
				// get number of data words (-1 because we don't want to include header size)
#if defined(SWAP_BYTES) || defined(DEBUG_EXTENDED_ZDAB)
				u_int32 data_words = (*sub_header & SUB_LENGTH_MASK) - 1;
#endif
#ifdef DEBUG_EXTENDED_ZDAB
				Printf("  Sub-field %d - %d words\n", (int)(*sub_header >> SUB_TYPE_BITNUM),(int)data_words);
#endif			
				SWAP_INT32( sub_header+1, data_words );
			}
			
			// Disable the extended PmtEventRecord feature:
			// - make sure sub-field flag is reset
			// (must do this because we have a non-standard use for this bit)
//			pmtEventPtr->CalPckType &= ~SUB_NOT_LAST;

			// keep track of last valid event GTID
			u_int32 gtid = pmtEventPtr->TriggerCardData.BcGT;
			if (gtid) mLastGTID = gtid;
		}
   							
	} else {
		
		if (sVerbose) {
			if (sVerbose > 1) {
				DumpHex(nzdabPtr);
			} else {
				DumpRecord(nzdabPtr, mLastGTID);
			}
		}
		pmtEventPtr = NULL;	// not a valid PmtEventRecord
	}
	return(pmtEventPtr);	// return the PmtEventRecord (or NULL if not a PmtRecord)
}


// GetBank - get specified type to bank data (or any type if bank_name is 0)
// Accepts: pointer to nZDAB record (native format) with bank data (external format)
// Returns: pointer to bank data (native format)
// Swaps: data to native format if specified type
u_int32 *PZdabFile::GetBank(nZDAB *nzdabPtr, u_int32 bank_name)
{
	u_int32	*dataPt;
	
	// look for specified bank type
	if (!bank_name || nzdabPtr->bank_name==bank_name) {
		// get pointer to bank data
		dataPt = (u_int32 *)(nzdabPtr + 1);
		// swap the bank data
		SWAP_INT32(dataPt, nzdabPtr->data_words);
	} else {
		dataPt = NULL;		// not the specified type of bank
	}
	// return pointer to the data
	return(dataPt);
}


// BankName - convert string to bank name (native format)
// - string must be 4 characters long (this is not validated)
u_int32 PZdabFile::BankName(char *bank_name_string)
{
	return( ((u_int32)bank_name_string[0] << 24) | 
			((u_int32)bank_name_string[1] << 16) |
			((u_int32)bank_name_string[2] <<  8) |
			((u_int32)bank_name_string[3]) );
}


// BankNameString - convert from bank name (native format) to string
char *PZdabFile::BankNameString(u_int32 bank_name)
{
	static char	rtnString[5];
	
	for (int i=0; i<4; ++i) {
		char ch = (bank_name >> (8 * (3 - i))) & 0xff;
		if (ch >= ' ') {
			rtnString[i] = ch;
		} else {
			rtnString[i] = ' ';	// substitute space for non-printable characters
		}
	}
	rtnString[4] = '\0';	// null terminate the string
	return(rtnString);
}

void PZdabFile::DumpRecord(u_int32 *bankData, int bankSize, u_int32 bankName, u_int32 lastGTID)
{
	char	*pt, buff[256];
	
	// swap the bank data
	SWAP_INT32(bankData, bankSize);
	
	switch (bankName) {
	
		case EPED_RECORD: {
			SBankEPED *eped = (EpedRecord *)bankData;
			char *type_str;
			char buff1[16];
			switch (eped->CalibrationType) {
				case EPED_T_SLOPE_RUN:
					type_str = "TSLOPE";
					break;
				case EPED_PED_RUN:
					type_str = "PED";
					break;
				case EPED_Q_SLOPE_RUN:
					type_str = "QSLOPE";
					break;
				default:
					sprintf(buff1,"0x%lx",(long)eped->CalibrationType);
					type_str = buff1;
					break;
			}
			char *flag_str;
			char buff2[16];
			switch (eped->Flag) {
				case EPED_START_CAL:
					flag_str = "STRT";
					break;
				case EPED_CHANGE_CAL:
					flag_str = "CHNG";
					break;
				case EPED_STOP_CAL:
					flag_str = "STOP";
					break;
				case EPED_END_CAL:
					flag_str = "END ";
					break;
				default:
					sprintf(buff2,"0x%lx",(long)eped->Flag);
					flag_str = buff2;
					break;
			}
			Printf("EPED at GT %.8ld: %s-%s cr=%-2ld sl=%s Wid=%-3ld C-Dly=%-3ld F-Dly=%-3ld Q-Inj=%-3ld GTID=%.8ld\n",
					(long)lastGTID, type_str, flag_str, (long)(eped->halfCrateID & 0x7f),
					(eped->halfCrateID & EPED_SECOND_HALF) ? "8-15" : "0-7 " ,
					(long)eped->ped_width,(long)eped->ped_delay_coarse,
					(long)eped->ped_delay_fine,(long)eped->qinj_dacsetting,
					(long)eped->GTID);
		} break;
		
		case RHDR_RECORD:
		case RUN_RECORD: {
			SBankRHDR *rhdr = (RunRecord *)bankData;
			SnoStr::GetList(buff, SnoStr::sRunType, rhdr->RunMask);
			Printf("RHDR at GT %.8ld: Run=%ld (%s) - Valid=%.8ld GTID=%.8ld\n", (long)lastGTID,
			        (long)rhdr->RunNumber, buff, (long)rhdr->ValidGTID, (long)rhdr->FirstGTID);
		} break;
		
		case TRIG_RECORD: {
			SBankTRIG *trig = (TriggerInfo *)bankData;
			SnoStr::GetList(buff, SnoStr::sTrigMask, trig->TriggerMask);
			Printf("TRIG at GT %.8ld: %s - GTID=%.8ld\n", (long)lastGTID,
					buff, (long)trig->GTID);
		} break;
		
		case CAST_RECORD:
		case CLST_RECORD: {
			SBankCAST *cast = (ManipStatus *)bankData;
			if ((unsigned)cast->status < kNumManipStatus) {
				pt = SnoStr::sManipStatus[cast->status];
			} else {
				sprintf(buff,"Status=0x%lx",(long)cast->status);
				pt = buff;
			}
			Printf("%s at GT %.8ld: ID=%u.%u %s Pos=(%.1f,%.1f,%.1f)\n",
					BankNameString(bankName), (long)lastGTID, (cast->sourceID >> 16),
					(cast->sourceID & 0xffff),pt,cast->position[0],cast->position[1],cast->position[2]);
		} break;
		
		case CAAC_RECORD:
		case CLAC_RECORD: {
			SBankCAAC *caac = (AVStatus *)bankData;
			Printf("%s at GT %.8ld: Pos=(%.1f,%.1f,%.1f) Rot=(%.2f,%.2f,%.2f)\n",
					BankNameString(bankName), (long)lastGTID,
					caac->position[0], caac->position[1], caac->position[2],
					caac->rotation[0], caac->rotation[1], caac->rotation[2]);
		} break;
		
		case SOSL_RECORD: {
			SBankSOSL *sosl = (LaserStatus *)bankData;
			Printf("SOSL at GT %.8ld: Status=0x%lx Dye=%ld Filter1=%ld Filter2=%ld\n",
					(long)lastGTID,
					(long)sosl->status, (long)sosl->dyeCellNumber,
					(long)sosl->filterWheel1Position, (long)sosl->filterWheel2Position);
		} break;
		
		case MAST_RECORD: {
			SBankMAST *mast = (MastRecord *)bankData;
			Printf("MAST at GT %.8ld: Version=%.4f Original=%.4f\n",
					(long)lastGTID, mast->currentVersion, mast->originalVersion);
		} break;
		
		default: {
			Printf("%s at GT %.8ld\n", 
					BankNameString(bankName), (long)lastGTID);
		} break;
		
	}
	
	// we must swap back the data
	SWAP_INT32(bankData, bankSize);
}

// DumpRecord - print information about specified record
// Accepts: pointer to nZDAB record (native format) with trailing data (external format)
// Swaps: nothing
void PZdabFile::DumpRecord(nZDAB *nzdabPtr, u_int32 lastGTID)
{
	DumpRecord((u_int32 *)(nzdabPtr + 1), nzdabPtr->data_words, nzdabPtr->bank_name, lastGTID);
}

void PZdabFile::DumpHex(nZDAB *nzdabPtr, int numToPrint)
{
	int		i;
	int		nl = (int)nzdabPtr->total_links;
	int		nio;	// extra i/o characteristic words
	u_int32 *tp = (u_int32 *)(nzdabPtr);
#ifdef SWAP_BYTES
	u_int32 *tp2 = (u_int32 *)(nzdabPtr + 1);
#endif
	
	SWAP_INT32(tp2,numToPrint);	// swap bytes
	
	// print i/o control words
	// look for first word
	for (nio=0; nio<17; ++nio) {
		if ((int)(*(tp-nl-nio-1) & 0x0000ffffUL) == (12 + nio + nl)) break;
	}
	if (nio < 17) {
		printf("%2d i/o control words:", nio + 1);
		if (nio) SWAP_INT32(tp-nl-nio, nio);
		for (i=0; i<=nio; ++i) {
			printf(" %.8lx",(long)*(tp-nl-nio-1+i));
		}
		if (nio) SWAP_INT32(tp-nl-nio, nio);
		printf("\n");
#ifdef DEBUG_RELOCATION_TABLE
		if (nzdabPtr->bank_name == MAST_RECORD) {
			// print relocation table information for MAST bank
			SWAP_INT32(tp-nl-nio-3, 2);
			printf("Relocation table: %.8lx %.8lx\n",*(tp-nl-nio-3),*(tp-nl-nio-2));
			SWAP_INT32(tp-nl-nio-3, 2);
		}
#endif
	} else {
		printf("*** Error searching for i/o control start!\n");
	}
	
	// print links
	printf("%2d links:",nl);
	if (nl) {
		SWAP_INT32(tp-nl, nl);
		for (i=0; i<nl; ++i) {
			printf(" %ld",(long)*(tp-nl+i));
		}
		SWAP_INT32(tp-nl, nl);
	}
	printf("\n");
	for (i=0; i<numToPrint; ++i) {
		printf("%2d) %s 0x%.8lx %10lu %20g\n",i,BankNameString(tp[i]),
			   (long)tp[i],(long)tp[i],*(float *)(tp+i));
	}
	printf("\n");

	SWAP_INT32(tp2,numToPrint);	// swap back again
}


//
// AddSubField - utility routine to add sub-field to PmtEventRecord
//
// io_sub_header_pt - pointer to pointer to sub-field header
//                    (Note: previous field size must be set before call!)
// sub_type - identification number for new sub-field
// numBytes - size of sub-field in bytes (not including sub-field header)
//
void PZdabFile::AddSubField(u_int32 **io_sub_header_pt, int sub_type, u_int32 numBytes)
{
	u_int32		*sub_header = *io_sub_header_pt;
	
	// set flag in previous sub-field header indicating another one follows
	*sub_header |= SUB_NOT_LAST;
	// move sub_header pointer to the new sub-field header
	sub_header += (*sub_header & SUB_LENGTH_MASK);
	// initialize new sub-field type
	*sub_header = (sub_type << SUB_TYPE_BITNUM);
	// set the size of this sub-field (round up to an even number of u_int32's)
	*sub_header |= ((numBytes + sizeof(SubFieldHeader) + sizeof(u_int32) - 1) / sizeof(u_int32));
	
	// return updated sub_header
	*io_sub_header_pt = sub_header;
}

// getExtendedData [static]
// get pointer to specified extended data in PMT record
u_int32 *PZdabFile::GetExtendedData(PmtEventRecord *pmtRecord, int subType)
{
    u_int32 *theData = NULL;
    u_int32	*sub_header = &pmtRecord->CalPckType;
    while (*sub_header & SUB_NOT_LAST) {
        sub_header += (*sub_header & SUB_LENGTH_MASK);
        if ((int)(*sub_header >> SUB_TYPE_BITNUM) == subType) {
            theData = (u_int32 *)(sub_header + 1);
            break;
        }
    }
    return(theData);
}

// GetSize - get the size of a PMT event record (including sub-fields)
// pmtRecord - pointer to pmt record in native format
u_int32 PZdabFile::GetSize(PmtEventRecord *pmtRecord)
{
	/* create new buffer for event */
	u_int32 event_size = sizeof(aPmtEventRecord) + 12 * pmtRecord->NPmtHit;
		
	/* make room for sub-headers */
	u_int32	*sub_header = &pmtRecord->CalPckType;
	while (*sub_header & SUB_NOT_LAST) {
		sub_header += (*sub_header & SUB_LENGTH_MASK);
		event_size += (*sub_header & SUB_LENGTH_MASK) * sizeof(u_int32);
	}
	return(event_size);
}


// byte-swap array of numbers - PH 09/02/03
void swap_bytes(char *valPt, int num, int size)
{
    for (int n=0; n<num; ++n) {
        char *pt1 = valPt;
        char *pt2 = valPt + size - 1;
        do {
            char tmp = *pt1;
            *pt1 = *pt2;
            *pt2 = tmp;
        } while (++pt1 < --pt2);
        valPt += size;
    }
}

/* return subrun number (-ve if filename doesn't conform to standard) */
int	zdab_get_subrun(char *filename)
{
	int		i;
	char	*pt = strstr(filename, ".zdab");
	if (!pt || pt-filename<4) return(-1);
	pt -= 4;
	/* verify the file name format (must end in _###.zdab) */
	if (pt[0] != '_') return(-1);
	for (i=1; i<4; ++i) {
		if (pt[i]<'0' || pt[i]>'9') return(-1);
	}
	/* filename is good, return the number */
	return(atoi(pt+1));
}

/* Set sub-run number in filename */
/* Note: before calling this routine, you should verify */
/* that the filename is in the proper format by prior call */
/* to zdab_get_subrun() */
int zdab_set_subrun(char *filename, int subrun)
{
	char	*pt;
	char	buff[32];
	
	if (subrun<0 || subrun>999) return(-1);
	pt = strstr(filename,".zdab");
	if (!pt || pt-filename<4) return(-1);
	sprintf(buff,"%.3d",subrun);
	memcpy(pt-3, buff, 3);
	return(subrun);
}

/* return run number (-ve if filename doesn't conform to standard) */
long zdab_get_run(char *filename)
{
	int		i;
	char	*pt = strstr(filename, ".zdab");
	if (!pt || pt-filename<18) return(-1);
	pt -= 14;
	/* verify the file name format (must start with SNO_##########) */
	if (memcmp(pt-4,"SNO_",4)) return(-1);
	for (i=0; i<10; ++i) {
		if (pt[i]<'0' || pt[i]>'9') return(-1);
	}
	/* filename is good, return the number */
	return(atol(pt));
}

/* Set run number in filename */
/* Note: before calling this routine, you should verify */
/* that the filename is in the proper format by prior call */
/* to zdab_get_run() */
long zdab_set_run(char *filename, long run)
{
	char	*pt;
	char	buff[32];
	
	if (run<0) return(-1);
	pt = strstr(filename,".zdab");
	if (!pt || pt-filename<18) return(-1);
	sprintf(buff,"%.10ld",run);
	memcpy(pt-14, buff, 10);
	return(run);
}

// get 50 MHz time in sec from PMT event record
double get50MHzTime(PmtEventRecord *pmtRecord)
{
	return(((double) 2048.0 * pmtRecord->TriggerCardData.Bc50_2 + 
    						  pmtRecord->TriggerCardData.Bc50_1) * 2e-8);
}

// get maximum value for 50 MHz clock (in sec)
double get50MHzTimeMax()
{
	return(4096.0 * 0x80000000UL * 2e-8);
}

// isOrphan - returns non-zero if the specified event is an orphan
int isOrphan(PmtEventRecord *pmtRecord)
{
	u_int32 *mtc_data = (u_int32 *)&pmtRecord->TriggerCardData;
	
	for (int i=0; i<6; ++i) {
		if (*mtc_data != 0) return(0);
		++mtc_data;
	}
	return(1);
}


