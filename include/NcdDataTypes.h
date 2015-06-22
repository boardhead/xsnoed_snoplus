//--------------------------------------------------------------------------------
// Header:	NcdDataTypes
// Purpose:	Describes the format of the NCD raw data.
// Notes:	
// History: 2003-02-11 (mw)  - Original
//			2003-03-25 (jmw) - Revised to include new scope clock record.  Also
//								increased number of bits to 5.  Finally defined
//								idStruct types.
//			2003-03-25 (jfw & mh) - revised, cleaned up naming
//			2003-03-28 (jmw) - Version with 5 bit id, helper masks and two
//								idStructs (ENCD and ONCD) one for the eCPU data
//								and one for the oscilloscope data.
// 			2003-05-21 (jmw) - Added Mux global status register.
//			2004-07-12 (jmw, krr, & mh) - Added General record types
//--------------------------------------------------------------------------------


#ifndef NcdDataTypes_H
#define NcdDataTypes_H

#define kNcdDataTypeMask   0xF8000000			// First 5 bits.
#define kNcdDataTypeMaskA  0xFC000000			// First 6 bits - used by NCDBuilder.
#define kNcdDataMask	   0x07FFFFFF			// Last 27 bits.
#define kNcdDataMaskA      0x03FFFFFF			// Last 26 bits - used by NCDBuilder.

// Types for idStruct
#define kNcdEcpuRecord	    'ENCD'
#define kNcdOscRecord       'ONCD'
#define kNcdRunRecord 		'RNCD'
#define kBuilderMessage		'MBLD'

// Types for hardware records.  
#define kNewRunRecordType		0x00000000		// New run record.
#define kTrigTimeRecordType 	0xF0000000		// NCD trigger card Time record
#define kMuxRecordType 			0xE0000000		// NCD Mux record
#define kScopeRecordType 		0xD0000000		// NCD scope record
#define kGTIDRecordType 		0xC0000000		// NCD trigger card GTID record
#define kScopeTimeRecordType	0xB0000000		// NCD scope Time record
//#define kShGScalRecordType 		0xA0000000		// NCD shaper card - global scaler record
#define kMuxGlobalStatusType	0x90000000		// NCD Mux global record
#define kShaperRecordType 		0x50000000		// NCD shaper card - adc record
#define kScopeGTIDType			0x80000000		// NCD Scope GTID record.
#define kHVType					0x70000000		// NCD HV record.
#define kGeneralDataType		0x60000000      // NCD General record type.

#define kGTIDShaperRecordType   0xA0000000		// NCD GTID record for shaper - used internally by NCDBuilder
												//  Resets this record to kGTIDRecordType.

// Sub types for generic records
#define kUndefined							0x00000000		// Undefined generic type.
#define kNCDModelPulserSettingsRecord		0x00000001		// Pulser settings.
#define kNCDModelLogAmpTask					0x00000002		// Log amp task.
#define kNCDModelLinearityTask				0x00000003		// Linearity task.
#define kNCDModelThresholdTask				0x00000004		// Threshold task.
#define kNCDModelStepPDSTask				0x00000005		// Model step task.
#define kORShaperModelScalers				0x00000006		// Scalers.
#define kORHPPulserModel					0x00000007		// HP Pulser.
#define kORLiveTimeScalers					0x00000008		// Livetime record.
#define kNCDModelCableCheckTask				0x00000009		// Cable check task.
#define kNCDScopeCal                        0x0000000a      // Scope calibration record.

// In the description below GTID1 is associated with the mux and GTID2 is associated with the shaper.

// Types for special eCPU operations (not inserted into data stream)
#define kSKIPRecordType			0x10000000	// skip data, so allocated buffer doesn't role over

//raw data structures for general data type - see end of file for detailed description
// of individual sub types.
//---------- General Data Type ------------------------------------------
// Can have 2 or more long words
// word#1: (Generic data header)
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^----------------------------------- kGeneralDataType                 [bits 27-31]
//       ^^^ ^^^^ ^^^^---------------------- Not used   					  [bits 16-26]
//                     ^^^^ ^^^^ ^^^^ ^^^^-- Size (bytes)					  [bits 0-15]
//
// word #2: (Sub record header)
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^^^^ ^^^^ ^^^^-----------------------Type of generic sub record         [bits 16-31]
//                     ^^^^ ^^^^ ^^^^ ^^^^---Size of generic sub record (bytes) [bits 0-15]
//
// notes:
// 1) If a GTID is included with a particular sub record it will be bits 0-24 of the word
//    immediately following the generic sub record header.
// 2) More than one sub record type can be in a given general data type record.  
//    The way you know that such a multi sub record exists is that the overall
//    size will be more than 4 bytes larger than the sub record size.


//raw data structures for new run
//---------- New run record ---------------------------------------------
// three long words
// word #1:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^----------------------------------- kNewRunRecordType                [bits 27-31]
//       ^^^ ^^^^ ^^^^---------------------- Data version					  [bits 16-26]
//                     ^^^^ ^^^^ ^^^^ ^^^^-- Spare 						  	  [bits 0-15]
//
// word #2:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long for run number
//
// word #3:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long containing mac time.

// Helper mask
#define kNewRunVersion 		0x07FF0000	// Mask to obtain version number.

//raw data structures as they are read out of hardware 
//---------- NCD trigger card clock record--------------------------------
// two long words
// word #1:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kTrigTimeRecordType             [bits 27-31]
//       ^----------------------------------- spare                           [bit 26]
//        ^---------------------------------- GTID2							  [bit 25]
//		   ^--------------------------------- GTID1							  [bit 24]
//           ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^--- 24 bits holding upper clock reg [bits 0-23]
//
// word #2:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bits holding lower clock reg
//--------------------------------------------------------------
// Helper masks
#define kClockLatch			0x07000000	// Mask to obtain latch register
#define kClockUpper			0x00FFFFFF	// Mask to obtain upper 24 bits of clock.


//---------- NCD trigger card GTID record--------------------------------
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kGTIDRecordType                 [bits 27-31]
//       ^----------------------------------- spare                           [bits 26]
//        ^---------------------------------- GTID2                           [bits 25]
//         ^--------------------------------- GTID1                           [bits 24]
//           ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^--- 24 bits for gtid                [bits 0-23]
//--------------------------------------------------------------
// Helper masks
#define kGTIDSyncClear		0x04000000	// Mask to obtain syn clear err
#define kGTIDGtid			0x00FFFFFF	// Mask to obtain GTID
#define kGTIDMux			0x01000000  // Mask to obtain if Mux GTID
#define kGTIDShaper			0x02000000	// Mask to obtain if Shaper GTID

//----------Shaper adc record-----------------------------------
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kShaperRecordType               [bits 27-31]
//       ^^^ ^^^^---------------------------- spare                           [bits 20-26]
//                ^^^^----------------------- card number                     [bits 16-19]
//                     ^^^^------------------ chan number                     [bits 12-15]
//                          ^^^^ ^^^^ ^^^^--- 12 bits of adc data             [bits 0-11]
//--------------------------------------------------------------
// Helper masks
#define kShaperCard			0x000F0000	// Mask to obtain shaper card.
#define kShaperChan			0x0000F000	// Mask to obtain shaper chan.
#define kShaperValue		0x00000FFF	// Mask to obtain shaper value.

//----------shaper card global scaler record type --------------------------------
// two long words
// word #1:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kShGScalRecordType              [bits 27-31]
//       ^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ------- spare                           [bits 4-26]
//                                    ^^^^--- board number                    [bits 0-3]
//
// word #2:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^-------------------------------------- spare                           [bits 28-31]
//      ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^--- 28 bits holding scaler value    [bits 0-27]
//--------------------------------------------------------------
// Helper masks
#define kGlobalShaperBoard	0x0000000F	// Mask to obtain board number.
#define kGlobalShaperScaler 0x0FFFFFFF	// Mask to obtain scaler value.

//----------Mux global status record-----------------------------------
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kMuxGlobalStatusType            [bits 27-31]
//       ^^^ ^^^^ ^^^^----------------------- spare                           [bits 16-26]
//                     ^^-------------------- Undefined                       [bits 14-15]
//                       ^^------------------ Scope trigger routing settings  [bits 12-13]
//                                             (A is bit 13)
//                          ^^--------------- Scope fired (A is bit 11)       [bits 10-11]
//		                      ^^------------- Scope busy (A is bit 9)         [bits 8-9]
//                               ^^^^ ^^^^--- 8 Mux fired register            [bits 0-7]
//        
//---------------------------------------------------------------
// Note: The bits 0-15 all come from the Mux Global Status register as one short.
// Helper masks
#define kMuxFired			0x000000FF	// Mask which muxes fired.
#define kMuxAScope			0x00000800  // Mask Scope A fired.
#define kMuxBScope    		0x00000400  // Mask Scope B fired.

//----------Mux data record-----------------------------------
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kMuxRecordType                  [bits 27-31]
//       ^^^-^------------------------------- mux bus                         [bits 23-26] 
//            ^^----------------------------- Scope trigger routing settings  [bits 21-22]
//                                             (A is bit 22)
//              ^_^-------------------------- Scope fired (A is bit 20)       [bits 19-20]
//		           ^^------------------------ Scope busy (A is bit 18)        [bits 17-18]
//                   ^ ^^^^------------------ spare                           [bits 12-16]
//                          ^^^^ ^^^^ ^^^^--- 12 bit chan hit patten          [bits 0-11]
//        
//---------------------------------------------------------------
// ** Scope 0 fired when bit 20 is set and scope 1 fired when bit 19 is set.
// Helper masks
#define kMuxBox				0x07800000	// Mask to get mux box number.
#define kMuxScope0			0x00100000	// Mask for scope0.
#define kMuxScope1 			0x00080000 	// Mask for scope1.
#define kMuxScope			0X00180000  // Mask for scopes.
#define kMuxHitPattern		0x00000FFF	// Mask to get hit pattern.

// raw data structures as they come off the scope (via mac)
//----------Scope clock record--------------------------------
// two long words
// word #1:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kScopeTimeRecordType            [bits 27-31]
//       ^^^--------------------------------- Channel (0-3)         		  [bits 24-26]
//           ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^--- 24 bits holding upper clock reg [bits 0-23]
//
// word #2:
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bits holding lower clock reg
//--------------------------------------------------------------
// See mask for NCD trigger clock

//----------Scope data record-----------------------------------
// n words
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^ ^------------------------------------ kScopeRecordType                [bits 27-31]
//       ^^^ ^------------------------------- scope                           [bits 23-26]
//            ^^^ ^-------------------------- chan                            [bits 19-22]
//                 ^^^------------------------spare                           [bits 16-18]
//                     ^^^^ ^^^^ ^^^^ ^^^^----number of bytes to follow       [bits 0-15]                  
// n bytes follow.....
//---------------------------------------------------------------
// Helper Masks
#define kScopeId			0x07800000	// Mask to get scope id.
#define kScopeChannel		0x00780000	// Mask to get channel.
#define kScopeSize			0x0000FFFF	// Mask to get number of bytes that follow.


//----------skiped CB record--------------------------------
// note that this record is never shipped, it is for internal use only.
// 0000 0000 0000 0000 0000 0000 0000 0000   32 bit unsigned long
// ^^^^-^------------------------------------ kSKIPRecordType                 [bits 27-31]
//       ^^^ ^^^^ ^^^^----------------------- unused                          [bits 16-26]
//                     ^^^^ ^^^^ ^^^^ ^^^^--- 16 bits holding skipped register size
//																			  [bits 0-15]
//----------------------------------------------------------


//----------Sub record types--------------------------------
// Sub types of generic records
//
// Pulser Settings
// Composed of 5 pieces of data: GTID, waveform, amplitude, burst rate and width of pulser (after header)
//
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Global Trigger ID
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Pulser waveform (int)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Pulser amplitude (float)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Pulser burst rate (float)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Pulser width (float)

// Tasks
// Composed of 2 pieces of data: GTID, stop/start (after header which says which task)
//
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Global Trigger ID
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
// --------------------------------------^--- 1==Started, 0==Stopped

// Scaler records
// Composed of global scalers (1 per card) and channel scalers (as many as enabled)
//
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Global Scaler Header
// -----^^^^--------------------------------- Crate Number
// ----------^^^^---------------------------- Card Number
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Global Scaler Value (32 bits)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Scaler Value 
// -----^^^^--------------------------------- Crate number
// ----------^^^^---------------------------- Card number
// ---------------^^^^----------------------- Channel number
// --------------------^^^^ ^^^^ ^^^^ ^^^^--- Scaler Value
// Scaler Values continue until the total length is used up.

// Livetime record
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Global Trigger ID
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Status word
// -----------------^^----------------------- 0=endrun,1=startrun,3=middlerun
// -------------------------^^^^------------- Crate
// ---------------------------------^ ^^^^--- Slot
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   High bytes of livetime counters
// ^^^^ ^^^^ -------------------------------- High 8 bits of trigger3 livetime
// ----------^^^^ ^^^^ ---------------------- High 8 bits of trigger2 livetime
// --------------------^^^^ ^^^^ ------------ High 8 bits of trigger1 livetime
// ------------------------------^^^^ ^^^^--- High 8 bits of total livetime
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Low 32 bits of total livetime
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Low 32 bits of MUX livetime
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Low 32 bits of shaper livetime
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Low 32 bits of scope livetime

// Scope calibration record
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Scope ID
// ------------------------------^^^^ ------- Scope number
// -----------------------------------^^^^--- Scope channel
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Logamp parameter A (float)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Logamp parameter B (float)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Logamp parameter C (float)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Preamp RC factor (float)
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx   Electronics delay time (float)

#endif
