//
// File:		Record_Info.h
//
// Description:	SNO datastream record definitions
//
// Revisions:   02/21/03 - PH Combined varioius versions of this file so now the exact
//                            same file is used by the builder, SHaRC, xsnoed and QSNO.
//              03/03/03 - PH Changed PmtEventRecord definition so it is now
//                            swapped like a set of 32-bit integers.
//              07/13/04 - PH Added new NCD run mask bits
//
// Notes:       For historical reasons, the structures used by SHaRC can be different
//              from the ones written to the data stream, so be careful which one you
//              are using!
//
#ifndef __RECORD_INFO_H__
#define __RECORD_INFO_H__

#include "include/sno_sys.h"

/*  version numbers...
 *  as of Feb 1997 (from UW) 
 */
#define DAQ_CODE_VERSION            0
#define RUN_RECORD_VERSION          0
#define PMT_RECORD_VERSION          0
#define TRIG_RECORD_VERSION         0
#define PTK_RECORD_VERSIONS			0x50544B30 // 'PTK0'
#define DAQ_RECORD_VERSIONS  		0x44415130 // 'DAQ0' 

#define WORD_SIZE(x)				(sizeof(x)/sizeof(u_int32))

// -------------------------------------------------------------------------------------
//   generic information for every header
//   - preceeds ALL dispatcher records, and all records sent by SHaRC
//   - not written to ZDAB files
//
typedef struct GenericRecordHeader {
  u_int32 RecordID;
  u_int32 RecordLength;   //  length of record to follow, 
                                // NOT INCLUDING generic record header! 
  u_int32 RecordVersion;
} aGenericRecordHeader;

#define HEADERSIZE  sizeof( aGenericRecordHeader ) // 12 bytes generic header.

// -------------------------------------------------------------------------------------
// SHaRC header for all manipulator records
//

// local time structure
typedef struct {
	byte	year;				// last 2 digits of year
	byte	month;				// month of year (1->12)
	byte	day;				// day of month (1->31)
	byte	daylightSavings;	// 0 = standard time, 1 = daylight savings time
    char    timeZone;           // hours from GMT (-11->11) (signed byte!)
	byte	hour;				// hour of day (0->23)
	byte	minute;				// minute of hour (0->59)
	byte	second;				// second of minute (0->59)
	u_int32	microsecond;		// millionths of a second (0->999999)	
} SDateTime;

// header for all manipulator banks
typedef struct {
	u_int32		instanceNumber;
	u_int32		gtid_ref;			// reference GTID for this event
	SDateTime	time;
} SManipHeader;

// -------------------------------------------------------------------------------------
//   MAST record
//
#define MAST_RECORD    0x4d415354 // 'MAST'

// MAST bank structure
typedef struct MastRecord {
	float	currentVersion;		// current SNOMAN version
	float	originalVersion;	// original SNOMAN version
} SBankMAST;

// -------------------------------------------------------------------------------------
//   RHDR record
//
#define RHDR_RECORD    0x52484452 // 'RHDR'  (as written to ZDAB file)
#define RUN_RECORD     0x52554E20 // 'RUN '  (as dispatched and sent by SHaRC)

// RHDR bank structure
typedef struct RunRecord {
  u_int32 Date;
  u_int32 Time;
  u_int32 DAQCodeVersion;
  u_int32 RunNumber;
  u_int32 CalibrationTrialNumber;
  u_int32 SourceMask;	//   which sources in? 
  u_int32 RunMask;		//   run conditions 
#ifdef __MWERKS__
  u_int32 GTCrateMask;  // this run's GT crate mask
#else
  u_int32 GTCrateMsk;   // accomodate misspelling of this for backward compatibility
#endif
  u_int32 FirstGTID;   	// first GTID of this run
  u_int32 ValidGTID;	// first valid GTID (after hardware changes have been made)
  u_int32 Spares[8];	// spares as per nick's suggestion (Thanks Nick!)
} SBankRHDR, aRunRecord;

//   SourceMask... 
#define NO_SRC					0x00000UL
#define ROTATING_SRC            0x00001UL
#define LASER_SRC               0x00002UL
#define SONO_SRC                0x00004UL
#define N16_SRC                 0x00008UL
#define N17_SRC                 0x00010UL
#define NAI_SRC                 0x00020UL
#define LI8_SRC                 0x00040UL
#define PT_SRC                  0x00080UL
#define CF_HI_SRC               0x00100UL
#define CF_LO_SRC               0x00200UL
#define U_SRC                   0x00400UL
#define TH_SRC                  0x00800UL
#define P_LI7_SRC               0x01000UL
#define WATER_SAMPLER           0x02000UL
#define PROP_COUNTER_SRC        0x04000UL
#define SINGLE_NCD_SRC          0x08000UL
#define SELF_CALIB_SRC          0x10000UL
#define Y88_SRC                 0x20000UL

//   RunMask... 
#define NEUTRINO_RUN            0x0001UL
#define SOURCE_RUN              0x0002UL
#define CALIB_RUN               0x0004UL
#define NCD_RUN                 0x0008UL
#define SALT_RUN                0x0010UL
#define POISON_RUN              0x0020UL
#define PARTIAL_FILL_RUN        0x0040UL
#define AIR_FILL_RUN            0x0080UL
#define D2O_RUN                 0x0100UL
#define H2O_RUN                 0x0200UL
#define DCR_ACTIVITY_RUN        0x0400UL
#define TRANSITION_RUN          0x0800UL
#define SOURCE_MOVING_RUN       0x1000UL
#define COMP_COILS_RUN          0x2000UL
#define ECA_RUN                 0x4000UL
#define DIAGNOSTIC_RUN          0x8000UL
#define SUPERNOVA_RUN           0x10000UL
#define MAINTENANCE_RUN         0x20000UL
#define PCA_RUN                 0x40000UL
#define EXPERIMENTAL_RUN        0x80000UL
#define D2O_CIRC_RUN            0x100000UL
#define BUBBLERS_RUN            0x200000UL
#define PMT_OFF_RUN             0x01000000UL
#define NCD_OFF_RUN             0x02000000UL
#define NCD_ECA_RUN             0x04000000UL

// -------------------------------------------------------------------------------------
//   TRIG record 
//
#define TRIG_RECORD    0x54524947 // 'TRIG'

// TRIG bank structure
typedef struct TriggerInfo {
  u_int32       TriggerMask;    //  which triggers were set? 
  u_int32       n100lo;            // trigger Threshold settings
  u_int32       n100med;           // these are longs cuz Josh is a weenie.
  u_int32       n100hi;
  u_int32       n20;
  u_int32       n20lb;
  u_int32       esumlo;
  u_int32       esumhi;
  u_int32       owln;
  u_int32       owlelo;
  u_int32       owlehi;
  u_int32       n100lo_zero;    // trigger Threshold zeroes
  u_int32       n100med_zero;
  u_int32       n100hi_zero;
  u_int32       n20_zero;
  u_int32       n20lb_zero;
  u_int32       esumlo_zero;
  u_int32       esumhi_zero;
  u_int32       owln_zero;
  u_int32       owlelo_zero;
  u_int32       owlehi_zero;
  u_int32       PulserRate;        //  MTC local pulser 
  u_int32       ControlRegister;  //  MTC control register status 
  u_int32       reg_LockoutWidth; //  min. time btwn global triggers 
  u_int32       reg_Prescale;     //  how many nhit_100_lo triggers to take 
  u_int32       GTID;             //  to keep track of where I am in the world
} SBankTRIG, aTriggerInfo, *aTriggerInfoPtr;

// TriggerMask...
#define TRIG_NHIT_100_LO        0x00000001
#define TRIG_NHIT_100_MED       0x00000002
#define TRIG_NHIT_100_HI        0x00000004
#define TRIG_NHIT_20            0x00000008
#define TRIG_NHIT_20_LB         0x00000010
#define TRIG_ESUM_LO            0x00000020
#define TRIG_ESUM_HI            0x00000040
#define TRIG_OWLN               0x00000080
#define TRIG_OWLE_LO            0x00000100
#define TRIG_OWLE_HI            0x00000200
#define TRIG_PULSE_GT           0x00000400
#define TRIG_PRESCALE           0x00000800
#define TRIG_PEDESTAL           0x00001000
#define TRIG_PONG               0x00002000
#define TRIG_SYNC               0x00004000
#define TRIG_EXT_ASYNC          0x00008000
#define TRIG_HYDROPHONE         0x00010000
#define TRIG_EXT3               0x00020000
#define TRIG_EXT4               0x00040000
#define TRIG_EXT5               0x00080000
#define TRIG_EXT6               0x00100000
#define TRIG_NCD_SHAPER         0x00200000
#define TRIG_EXT8               0x00400000
#define TRIG_SPECIAL_RAW        0x00800000
#define TRIG_NCD_MUX            0x01000000
#define TRIG_SOFT_GT            0x02000000

// -------------------------------------------------------------------------------------
//   EPED record
//
#define EPED_RECORD    0x45504544 // 'EPED'

// EPED bank structure (as seen by SNOMAN)
typedef struct EpedRecord {
  u_int32 ped_width;
  u_int32 ped_delay_coarse;
  u_int32 ped_delay_fine;
  u_int32 qinj_dacsetting;
  u_int32 halfCrateID;
  u_int32 CalibrationType;
  u_int32 GTID;
  u_int32 Flag;
} SBankEPED;

// EPED record as sent by SHaRC
// (be very careful here!  for backward compatibility this is also
//  defined as EPEDRecord as used by SHaRC.  Not to be confused with EpedRecord.)
typedef struct EPEDRecord {
  u_int32	Time;					// ptk used instead of aDate_Time
  u_int16	CalibrationType;
  byte		halfCrateID; 			// which 1/2 crate is enabled for pedestals
  byte		reg_PedestalWidth;		// width of pedestal pulse for Qinj 
  byte		reg_Ped_GTDel_Coarse;
  byte		reg_Ped_GTDel_Fine;		// pedestal delay for T slopes 
  int16		Qinj_dacsetting;		// DAC setting for Qinj 
  u_int32	MTCD_csr;
  u_int32	GTID;					// GT Id validity range
  u_int32	Flag;					// start/stop flag
  u_int32	RunNumber;				// current run number
  u_int32	Spares[5];
} SHaRC_BankEPED;

// CalibrationType...
#define EPED_Q_SLOPE_RUN        1
#define EPED_T_SLOPE_RUN        2
#define EPED_PED_RUN            3
#define EPED_SLOWPED_RUN        4

// Bit mask in halfCrateID...
#define EPED_FIRST_HALF         (byte) 0
#define EPED_SECOND_HALF        (byte) 0x80U

// Flag...
#define EPED_START_CAL          1ul       // start of cali run
#define EPED_CHANGE_CAL         2ul       // change of same
#define EPED_STOP_CAL           3ul       // stop of same, crate
#define EPED_END_CAL            4ul       // end of run, all crates

// -------------------------------------------------------------------------------------
//   VTHR record 
//
#define VTHR_RECORD	   0x56544852 // 'VTHR'

// VTHR bank structure
typedef struct VThresholdRecord {
  u_int32        GTID;	  //  GTID 
  u_int32        ncrates; // number of crates = 19 for SNODAQ
  u_int32        null2;   // these are longs cuz Josh is a weenie.
  u_int32        null3;
  u_int32        null4;
  u_int32        null5;
  u_int32        null6;
  u_int32        null7;
  u_int32        null8;
  u_int32        null9;
  unsigned char	 theData[19*16*32];
} SBankVTHR;

// -------------------------------------------------------------------------------------
//   CAST record
//
#define CAST_RECORD		0x43415354	// 'CAST'
#define CLST_RECORD		0x434C5354	// 'CLST' - obsolete

#define	kMaxManipulatorRopes	6

// manipulator rope structure
typedef struct ManipRopeStatus {
	u_int32		ropeID;				// EManipulatorRopeID
	float		length;
	float		targetLength;
	float		velocity;
	float		tension;
	float		encoderError;
	float		spares[2];
} aManipRopeStatus;

//  CAST bank structure
typedef struct ManipStatus {
	u_int32			sourceID;
	u_int32			status;
	u_int32			numRopes;
	float			position[3];	// x, y, z position
	float			destination[3];
	float			positionError;
	float			obsoletePosErr[3];
    float			orientation;	// source orientation (0=unknown, 1=north...)
    float           spare;
	u_int32			tail;
	aManipRopeStatus	ropeStatus[kMaxManipulatorRopes];
	float			spare2[23];
} SBankCAST;

//  SHaRC CAST bank structure
typedef struct {
	u_int32			sourceID;
	u_int32			status;
	u_int32			numRopes;
	float			position[3];	// x, y, z position
	float			destination[3];
	float			positionError;
	float			obsoletePosErr[3];
    float			orientation;	// source orientation (0=unknown, 1=north...)
    float           spare;
	u_int32			tail;
	aManipRopeStatus	ropeStatus[4];
} SHaRC_BankCAST;

// status...
enum EManipulatorStatus {
	kStoppedNone = 1,
	kStoppedLowTension,
	kStoppedHighTension,
	kStoppedEndpoint,
	kStoppedStuck,
	kStoppedNetForce,
	kStoppedAxisError,
	kMovingDirect,
	kMovingIdle,
	kMovingAbort
};
	
// ropeID...
enum EManipulatorRopeID {
	NorthRope = 1,
	SouthRope,
	EastRope,
	WestRope,
	CentralRope1,
	CentralRope2,
	CentralRope3,
	GuideTubeRope1,
	GuideTubeRope2,
	GuideTubeRope3,
	GuideTubeRope4,
	GuideTubeRope5,
	GuideTubeRope6,
	GasUmbilical,
	LaserUmbilical,
	RotatingUmbilical
};

// -------------------------------------------------------------------------------------
//   CAAC record
//
#define CAAC_RECORD		0x43414143	// 'CAAC'
#define CLAC_RECORD		0x434C4143	// 'CLAC' - obsolete

// CAAC bank structure
typedef struct AVStatus {
	float		position[3];		// x, y, z
	float		rotation[3];		// roll, pitch, yaw
	float		ropeLength[7];		// seven sense rope lengths
	float		spare[3];
} SBankCAAC;

// -------------------------------------------------------------------------------------
//   SOSA record
//
#define SOSA_RECORD		0x534F5341	// 'SOSA'

// SOSA bank structure
typedef struct AcceleratorStatus {
	u_int32		sourceID;
	u_int32		acceleratorOn;
	float		targetVoltage;
	float		targetCurrent;
	float		anodeVoltage;
	float		anodeCurrent;
	float		getterVoltage;
	float		getterCurrent;
	float		direction[3];
	float		spare;
} SBankSOSA;

// -------------------------------------------------------------------------------------
//   SOSG record
//
#define SOSG_RECORD		0x534F5347	// 'SOSG'

// SOSG bank structure
typedef struct GasStatus {
	u_int32		sourceID;
	u_int32		dtGeneratorOn;
	float		targetVerticalPosition;
	float		pressure1;
	float		pressure2;
	float		pressure3;
	float		pressure4;
	float		pressure5;
	float		pressure6;
	float		vacuum1;
	float		vacuum2;
	float		carbonDioxideFlowRate;
	float		heliumFlowRate;
	float		oxygenFlowRate;
	float		fastNeutronFlux;
	float		neutronRadiationMonitor;
	float		pmtVoltage;
	float		wireVoltage;
	float		direction[3];
	float		spare[2];
} SBankSOSG;

// -------------------------------------------------------------------------------------
//   SOSL record
//
#define SOSL_RECORD		0x534F534C	// 'SOSL'

// SOSL bank structure
typedef struct LaserStatus {
	u_int32		sourceID;
	u_int32		status;
	u_int32		dyeCellNumber;
	u_int32		filterWheel1Position;
	u_int32		filterWheel2Position;
	u_int32		pmtVoltage;
	float		pressure1;
	float		pressure2;
	float		nitrogenFlowRate;
	float		direction[3];
	float		spare;
} SBankSOSL;

// -------------------------------------------------------------------------------------
//   SOSR record
//
#define SOSR_RECORD		0x534F5352	// 'SOSR'

// SOSR bank structure
typedef struct RotatingStatus {
	u_int32		sourceID;
	float		direction[3];
} SBankSOSR;


// -------------------------------------------------------------------------------------
//   TASK record
//
#define TASK_RECORD     0x5441534B // 'TASK'

// task record  -- Task running record
typedef struct TASKRecord {
    u_int32 Time;           // ptk used instead of aDate_Time
    u_int32 TasksRunning;   //true if any task is running, false if none running.
    u_int32 GTID;
    u_int32 RunNumber;
    u_int32 Spares[5];
} SBankTASK;

// -------------------------------------------------------------------------------------
//   ZDAB record (event record)
//
#define ZDAB_RECORD    0x5a444142 // 'ZDAB'  (as written to ZDAB file)
#define PMT_RECORD     0x504d5420 // 'PMT '  (as dispatched and sent by SHaRC)

/*
 * pmt record -- variable-length record containing npmt*3 LW of FEC data.
 * note: Generic Header length indicates only length of the pmt header, and
 * DOES NOT include the hits! 
 *
 * Also includes the following info, from ref_packer_zdab_pmt.f
 *  
 *      -> BEWARE:  HERE, MSB is 32 ! ! ! ! <-
 *  Also, word number below is SNOMAN WORD NUMBER ONLY!
 *
 *  o  Event Header Record (one per event trigger):
 *                                 Number
 *     Name        WORD  LSB Pos.  of Bits       Description
 *     RECORD_TYPE   1      26       7       Record type (e.g. PMT, NCD, etc).
 *     MC_FLAG       1      25       1       0=Real event, 1= MC event.
 *     VER_NUM       1      17       8       ZDAB_PMT format number number.
 *     DATA_TYPE     1       1      16       Run Type (see id_run_types.doc).
 *     RUN_NUMBER    2       1      32       Run number.
 *     EV_NUMBER     3       1      32       Event number in this Run.
 *     DAQ_STATUS    4      17      16       DAQ status flags.
 *     NHITS         4       1      16       Number of fired PMT's.
 *     PCK_TYPE      5      29       4       MC packing type:
 *                                           0= PMT info only
 *                                           1= 0 plus source bank info
 *                                           2= 1 plus jitter/cerenkov history
 *     CAL_TYPE      5      25       4       MC Calibration type:
 *                                           0= simple calibration constants
 *                                           1= full calibration constants
 *
 *     ---- extended PmtEventRecord format (PH 02/25/99) ----
 *
 *     SUB_FIELD     5      24       1       0= No sub fields
 *                                           1= Sub field to follow
 *     FIELD_LEN     5       1      23       Offset in 4-byte words to first sub field
 *                                           (May be zero if no sub fields)
 *                                           
 */

// put flags here, shifted to correct position.  See above.
#define PMT_EVR_RECTYPE         ( 0xA << 9 ) // see SNOMAN Docs for these
#define PMT_EVR_NOT_MC          ( 0x0UL << 8 )
#define PMT_EVR_ZDAB_VER        ( 23UL << 0  )
#define PMT_EVR_DATA_TYPE       0xB 
#define PMT_EVR_DAQ_STAT        0xA 
#define PMT_EVR_PCK_TYPE        ( 0x00UL << 28 )
#define PMT_MC_PCK_TYPE         ( 0x01UL << 28 )
#define PMT_JITTER_PCK_TYPE     ( 0x02UL << 28 )
#define PMT_EVR_CAL_TYPE        ( 0x01UL << 24 )

#define PCK_TYPE_MASK           ( 0x0fUL << 28 )
#define CAL_TYPE_MASK           ( 0x0fUL << 24 )

#ifdef SWAP_BYTES

//   FEC data as read out in 96-bit structure 
typedef struct FECReadoutData {
  //   word 1 (starts from LSB): 
  unsigned GTID1                :16; //   lower 16 bits 
  unsigned ChannelID            :5;
  unsigned CrateID              :5;
  unsigned BoardID              :4;
  unsigned CGT_ES16             :1;
  unsigned CGT_ES24             :1;

  //   word 2: 
  unsigned Qlx                  :11;
  unsigned SignQlx              :1;
  unsigned CellID               :4;
  unsigned Qhs                  :11;
  unsigned SignQhs              :1;
  unsigned MissedCount          :1;
  unsigned NC_CC                :1;
  unsigned LGI_Select           :1;
  unsigned Cmos_ES16            :1;

  //   word 3            : 
  unsigned Qhl                  :11;
  unsigned SignQhl              :1;
  unsigned GTID2                :4;        //   bits 17-20 
  unsigned TAC                  :11;
  unsigned SignTAC              :1;
  unsigned GTID3                :4;        //   bits 21-24 

} aFECReadoutData, *aFECReadoutDataPtr;

//   Master Trigger Card data 
typedef struct MTCReadoutData {
  //   word 0 
  u_int32 Bc10_1          :32;
  //   word 1 
  u_int32 Bc10_2          :21;
  u_int32 Bc50_1          :11;
  //   word 2 
  u_int32 Bc50_2          :32;
  //   word 3 
  u_int32 BcGT            :24; //   LSB 
  unsigned Nhit_100_Lo          :1;
  unsigned Nhit_100_Med         :1;
  unsigned Nhit_100_Hi          :1;
  unsigned Nhit_20              :1;
  unsigned Nhit_20_LB           :1;
  unsigned ESum_Lo              :1;
  unsigned ESum_Hi              :1;
  unsigned Owln                 :1; //   MSB 

  //   word 4 
  unsigned Owle_Lo              :1;
  unsigned Owle_Hi              :1;
  unsigned Pulse_GT             :1;
  unsigned Prescale             :1;
  unsigned Pedestal             :1;
  unsigned Pong                 :1;
  unsigned Sync                 :1;
  unsigned Ext_Async            :1;
  unsigned Hydrophone           :1;
  unsigned Ext_3                :1;
  unsigned Ext_4                :1;
  unsigned Ext_5                :1;
  unsigned Ext_6                :1;
  unsigned NCD_Shaper           :1;
  unsigned Ext_8                :1;
  unsigned Special_Raw          :1;
  unsigned NCD_Mux              :1;
  unsigned Soft_GT              :1;
  unsigned Miss_Trig            :1;
  unsigned Peak                 :10;
  unsigned Diff_1               :3;

  //  word 5 
  unsigned Diff_2               :7;
  unsigned Int                  :10;
  unsigned TestGT               :1;
  unsigned Test50               :1;
  unsigned Test10               :1;
  unsigned TestMem1             :1;
  unsigned TestMem2             :1;
  unsigned SynClr16             :1;
  unsigned SynClr16_wo_TC16     :1;
  unsigned SynClr24             :1;
  unsigned SynClr24_wo_TC24     :1;
  unsigned FIFOsNotAllEmpty     :1;
  unsigned FIFOsNotAllFull      :1;
  unsigned FIFOsAllFull         :1;
  unsigned Unused1              :1;
  unsigned Unused2              :1;
  unsigned Unused3              :1;

} aMTCReadoutData, *aMTCReadoutDataPtr;

#else // SWAP_BYTES

//   FEC data as read out in 96-bit structure 
typedef struct FECReadoutData {
  //   word 1 (starts from MSB): 
  unsigned CGT_ES24             :1;
  unsigned CGT_ES16             :1;
  unsigned BoardID              :4;
  unsigned CrateID              :5;
  unsigned ChannelID            :5;
  unsigned GTID1                :16; //   lower 16 bits 
  //   word 2: 
  unsigned Cmos_ES16            :1;
  unsigned LGI_Select           :1;
  unsigned NC_CC                :1;
  unsigned MissedCount          :1;
  unsigned SignQhs              :1;
  unsigned Qhs                  :11;
  unsigned CellID               :4;
  unsigned SignQlx              :1;
  unsigned Qlx                  :11;
  //   word 3            : 
  unsigned GTID3                :4;        //   bits 21-24 
  unsigned SignTAC              :1;
  unsigned TAC                  :11;
  unsigned GTID2                :4;        //   bits 17-20 
  unsigned SignQhl              :1;
  unsigned Qhl                  :11;
} aFECReadoutData, *aFECReadoutDataPtr;

//   Master Trigger Card data 
typedef struct MTCReadoutData {
  //   word 0 
  u_int32 Bc10_1          :32;
  //   word 1 
  u_int32 Bc50_1          :11;
  u_int32 Bc10_2          :21;
  //   word 2 
  u_int32 Bc50_2          :32;
  //   word 3 
  unsigned Owln                 :1; //   MSB 
  unsigned ESum_Hi              :1;
  unsigned ESum_Lo              :1;
  unsigned Nhit_20_LB           :1;
  unsigned Nhit_20              :1;
  unsigned Nhit_100_Hi          :1;
  unsigned Nhit_100_Med         :1;
  unsigned Nhit_100_Lo          :1;
  u_int32 BcGT            :24; //   LSB 
  //   word 4 
  unsigned Diff_1               :3;
  unsigned Peak                 :10;
  unsigned Miss_Trig            :1;
  unsigned Soft_GT              :1;
  unsigned NCD_Mux              :1;
  unsigned Special_Raw          :1;
  unsigned Ext_8                :1;
  unsigned NCD_Shaper           :1;
  unsigned Ext_6                :1;
  unsigned Ext_5                :1;
  unsigned Ext_4                :1;
  unsigned Ext_3                :1;
  unsigned Hydrophone           :1;
  unsigned Ext_Async            :1;
  unsigned Sync                 :1;
  unsigned Pong                 :1;
  unsigned Pedestal             :1;
  unsigned Prescale             :1;
  unsigned Pulse_GT             :1;
  unsigned Owle_Hi              :1;
  unsigned Owle_Lo              :1;
  //  word 5 
  unsigned Unused3              :1;
  unsigned Unused2              :1;
  unsigned Unused1              :1;
  unsigned FIFOsAllFull         :1;
  unsigned FIFOsNotAllFull      :1;
  unsigned FIFOsNotAllEmpty     :1;
  unsigned SynClr24_wo_TC24     :1;
  unsigned SynClr24             :1;
  unsigned SynClr16_wo_TC16     :1;
  unsigned SynClr16             :1;
  unsigned TestMem2             :1;
  unsigned TestMem1             :1;
  unsigned Test10               :1;
  unsigned Test50               :1;
  unsigned TestGT               :1;
  unsigned Int                  :10;
  unsigned Diff_2               :7;
} aMTCReadoutData, *aMTCReadoutDataPtr;

#endif // SWAP_BYTES

// ZDAB bank structure
typedef struct PmtEventRecord {
#ifdef SWAP_BYTES
  u_int16 DataType;
  u_int16 PmtEventRecordInfo;
#else
  u_int16 PmtEventRecordInfo;
  u_int16 DataType;
#endif
  u_int32 RunNumber;
  u_int32 EvNumber;
#ifdef SWAP_BYTES
  u_int16 NPmtHit;
  u_int16 DaqStatus; // Now used to store sub-run number - PH
#else
  u_int16 DaqStatus; // Now used to store sub-run number - PH
  u_int16 NPmtHit;
#endif
  u_int32 CalPckType;  // lower 24 bits are now used for extended PmtEventRecord flags
  aMTCReadoutData TriggerCardData;    //   6 LW of MTC data 
                                      //   FECReadoutData follows directly. 
} SBankZDAB, aPmtEventRecord;

// MCHeader - monte carlo event data if CAL_TYPE = 1 or 2 (one per event)
typedef struct MCHeader {
  u_int16 mcVersion;    // SNOMAN MC version number
  u_int16 nVertices;    // number of source vertices
  u_int32 mcEvNumber;   // MC event number
  u_int32 julianDate;   // Julian Date (01/01/75 = 1)
  u_int32 sec;          // universal time seconds
  u_int32 nsec;         // universal time nanoseconds
  u_int16 Seed1;        // first random number seed
  u_int16 Seed2;        // second random number seed
  u_int32 randNum;      // random number used
} aMCHeader;

// MCData - monte carlo data (one per source vertex + hanging tracks)
typedef struct MCData {
  u_int32 intType;      // interaction type
  u_int16 xPos;         // vertex X position (32768 = 1000.0 cm)
  u_int16 yPos;         // vertex Y position
  u_int16 zPos;         // vertex X position
  byte    nTracks;      // number of tracks for this vertex
  byte    vClass;       // vertex class, (Pre)Source=(1)0
  u_int32 time0;        // first word of double prec time
  u_int32 time1;        // second word of double prec time
  u_int16 pType;        // particle type
  u_int16 pEnergy0;     // first word of 32-bit energy (note: NOT on 32-bit boundary!)
  u_int16 pEnergy1;     // second word of 32-bit energy
  u_int16 xDir;         // track direction x-cosine (32768 = 1.0)
  u_int16 yDir;         // track direction x-cosine
  u_int16 zDir;         // track direction x-cosine
} aMCData;

// MCJitter - jitter/cherenkov history if CAL_TYPE = 2 (one per PMT)
typedef struct MCJitter {
  u_int32 jitterData;
} aMCJitter;
 
/* ................... extended PmtEventRecord format (PH 02/25/99) ................... */

#define SUB_NOT_LAST            ( 0x00800000UL )  // bit set indicates another sub-field follows
#define SUB_LENGTH_MASK         ( 0x007fffffUL )  // mask for offset to next sub-field
#define SUB_TYPE_BITNUM         24                // bit position of sub-field type

/* sub-field ID numbers */
#define SUB_TYPE_CALIBRATED     8UL     // calibrated sub-field type
#define SUB_TYPE_MONTE_CARLO    9UL     // monte carlo sub-field
#define SUB_TYPE_FIT           10UL     // fitted event sub-field
#define SUB_TYPE_HIT_DATA	   11UL		// extra floating-point hit data
#define SUB_TYPE_EVENT_DATA    12UL		// extra floating-point event data
#define SUB_TYPE_PACKED_TSLH   13UL     // packed time since last hit
#define SUB_TYPE_CAL_FLAGS     14UL     // calibrated data flags (one 32 bit word)
#define SUB_TYPE_HCA_QUEENS    15UL     // Queen's HCA calibration
#define SUB_TYPE_NCD		   16UL     // NCD data record
#define SUB_TYPE_CAEN          32UL     // SNO+ CAEN data
#define SUB_TYPE_TUBII         33UL     // SNO+ TUBII trigger word

/* sub-field header */
typedef struct SubFieldHeader {
  u_int32 flags;        // bits  0-22 = size of this sub-field in 4-byte words (including header)
                        // bit  23    = 0-no more sub-fields, 1=another sub-field follows
                        // bits 24-31 = ID number for this sub-field
} aSubFieldHeader;

/* sub-field ID number 8 - calibrated PMT data */
/* -9999 in any of these fields means that no calibration was available. */
/* Order and number of these correspond exactly to hits in original PmtEventRecord. */
typedef struct CalibratedPMT {
    float    tac;
    float    qhs;
    float    qhl;
    float    qlx;
} aCalibratedPMT;

/* sub-field ID number 9 - monte carlo header */
typedef struct MCHeader   MonteCarloHeader;

/* vertex structure for monte-carlo data */
typedef struct MonteCarloVertex {
    float    x,y,z;     // vertex location in cm
    float    u,v,w;     // track direction cosines
    double   time;      // event time
    float    energy;    // energy of particle for outgoing track
    u_int32  int_code;  // vertex interaction code
    u_int16  flags;     // special vertex flags (upper 8 bits reserved for display)
    u_int16  particle;  // outgoing track particle ID
    int32    parent;    // index of parent vertex in list (-ve if no parent)
} aMonteCarloVertex;

/* sub-field ID number 10 - fitted event data */
typedef struct FittedEvent {
    float    x,y,z;     // fitted event position in cm
    float    u,v,w;     // fitted event direction cosines
    float    time;      // fitted event time
    float    quality;   // quality of fit
    u_int16  npmts;     // number of PMT's fit
    u_int16  spare;     // extra fit-dependent data
    char     name[32];  // fitter identification string (NULL terminated)
} aFittedEvent;

/* sub-field ID number 11 - extra floating point hit data */
#define DATA_NAME_LEN	24

typedef struct ExtraHitData {
	char	 name[DATA_NAME_LEN];	// null-terminated data name (see Note 1)
	// -- followed by NPmtHit float values in the same order as PMT hits
} aExtraHitData;

// Note 1) The extra hit/event data name may have an optional format specifier.
// The format specifier is standard C printf style for a floating point number
// (ie. "%.2f"), and must immediately follow the null terminator of the name.

/* sub-field ID number 12 - extra floating point event data */
typedef struct ExtraEventData {
	char	name[DATA_NAME_LEN];	// null-terminated data name (see Note 1)
	float	value;	// data value
} aExtraEventData;

/* sub-field ID number 13 - packed TSLH */

/* sub-field ID number 14 - calibration flags (one 32-bit word) */
#define CAL_FLAG_QSLOPE		0x0001	// flag for charge slopes applied

/* sub-field ID number 15 - Queen's HCA calibration */

/* sub-field ID number 16 - NCD data record (see NcdDataTypes.h) */

/* sub-field ID nubmer 32 - CAEN trigger sum data */
#define UNPK_CAEN_MAGIC(a)          ( (*(a) >> 28) & 0x0000000f )
#define UNPK_CAEN_WORD_COUNT(a)     ( *(a) & 0x0fffffff )
#define UNPK_CAEN_CHANNEL_MASK(a)   ( *((a)+1) & 0x000000ff )
#define UNPK_CAEN_PATTERN(a)        ( (*((a)+1) >> 8) & 0x0000ffff )
#define UNPK_CAEN_PACK_FLAG(a)      ( (*((a)+1) >> 24) & 0x00000001 )
#define UNPK_CAEN_BOARD_ID(a)       ( *((a)+1) >> 25 )
#define UNPK_CAEN_SYNC16(a)         ( (*((a)+1) >> 15) & 0x00000001 )
#define UNPK_CAEN_SYNC24(a)         ( (*((a)+1) >> 16) & 0x00000001 )
#define UNPK_CAEN_EVENT_COUNT(a)    ( *((a)+2) & 0x00ffffff )
#define UNPK_CAEN_TRIGGER_TIME(a)   ( *((a)+3) )

/* sub-field ID number 33 - TUBii record */
typedef struct TubiiRecord {
    u_int32     TrigWord;
    u_int32     GTID;
} aTubiiRecord;

/* .............................. End extended format .............................. */

/*
 *  unpacking "routines" by definition! Looks awful, but is fast...
 *  "a" in the following is a pointer to 3 longwords as read out from
 *  the FEC32.  from SNODAQ distribution
 */
#define UNPK_MISSED_COUNT(a)    ( (*((a)+1) >> 28) & 0x1 )
#define UNPK_NC_CC(a)           ( (*((a)+1) >> 29) & 0x1 )
#define UNPK_LGI_SELECT(a)      ( (*((a)+1) >> 30) & 0x1 )
#define UNPK_CMOS_ES_16(a)      ( (*((a)+1) >> 31) & 0x1 )
#define UNPK_CGT_ES_16(a)       ( (*(a) >> 30) & 0x1 )
#define UNPK_CGT_ES_24(a)       ( (*(a) >> 31) & 0x1 )
#define UNPK_QLX(a)             ( (  *((a)+1)        & 0x00000fff) ^ 0x00000800 )
#define UNPK_QHS(a)             ( ( (*((a)+1) >> 16) & 0x00000fff) ^ 0x00000800 )
#define UNPK_QHL(a)             ( (  *((a)+2)        & 0x00000fff) ^ 0x00000800 )
#define UNPK_TAC(a)             ( ( (*((a)+2) >> 16) & 0x00000fff) ^ 0x00000800 )

#define UNPK_CELL_ID(a)         ( (*((a)+1) >> 12) & 0x0000000F )
#define UNPK_CHANNEL_ID(a)      ( (*(a) >> 16) & 0x0000001F )
#define UNPK_BOARD_ID(a)        ( (*(a) >> 26) & 0x0000000F )
#define UNPK_CRATE_ID(a)        ( (*(a) >> 21) & 0x0000001F )
#define UNPK_FEC_GT24_ID(a)     ( (  * (a)          & 0x0000FFFF ) | \
                                  ( (*((a)+2) << 4) & 0x000F0000 ) | \
                                  ( (*((a)+2) >> 8) & 0x00F00000 ) )
#define UNPK_FEC_GT16_ID(a)     ( *(a) & 0x0000FFFF ) 
#define UNPK_FEC_GT8_ID(a)      ( ( (*((a)+2) >> 24) & 0x000000F0 ) | \
                                  ( (*((a)+2) >> 12) & 0x0000000F )

#define UNPK_FEC_GT_ID(a)       ( (  * (a)          & 0x0000FFFF ) | \
                                  ( (*((a)+2) << 4) & 0x000F0000 ) | \
                                  ( (*((a)+2) >> 8) & 0x00F00000 ) )

// unpacking trigger words by definitions...
// unpacking "routines" by definition! Looks awful, but is fast...
// "a" in the following is a pointer to 6 longwords as read out from the MTC.
#define UNPK_MTC_BC50_1(a)		( (*((a)+1) >> 21) & 0x7FF )
#define UNPK_MTC_BC50_2(a)		( *((a)+2) )
#define UNPK_MTC_BC10_1(a)		( *( a ) )
#define UNPK_MTC_BC10_2(a)		( *((a)+1) & 0x1FFFFF )
#define UNPK_MTC_GT_ID(a)		( *((a)+3) & 0x00FFFFFF )
#define UNPK_MTC_DIFF(a)		(((*((a)+4) >> 29) & 0x007) | ((*((a)+5) << 3) & 0x3F8))
#define UNPK_MTC_INT(a)			( (*((a)+5) >>  7) & 0x3FF )
#define UNPK_MTC_PEAK(a)		( (*((a)+4) >> 19) & 0x3FF )
#define UNPK_NHIT_100_LO(a)		( *((a)+3) & 0x01000000 )
#define UNPK_NHIT_100_MED(a)	( *((a)+3) & 0x02000000 )
#define UNPK_NHIT_100_HI(a)		( *((a)+3) & 0x04000000 )
#define UNPK_NHIT_20(a)			( *((a)+3) & 0x08000000 )
#define UNPK_NHIT_20_LB(a)		( *((a)+3) & 0x10000000 )
#define UNPK_ESUM_LO(a)			( *((a)+3) & 0x20000000 )
#define UNPK_ESUM_HI(a)			( *((a)+3) & 0x40000000 )
#define UNPK_OWLN(a)			( *((a)+3) & 0x80000000 )
#define UNPK_OWLE_LO(a)			( *((a)+4) & 0x00000001 )
#define UNPK_OWLE_HI(a)			( *((a)+4) & 0x00000002 )
#define UNPK_PULSE_GT(a)		( *((a)+4) & 0x00000004 )
#define UNPK_PRESCALE(a)		( *((a)+4) & 0x00000008 )
#define UNPK_PONG(a)			( *((a)+4) & 0x00000010 )
#define UNPK_SYNC(a)			( *((a)+4) & 0x00000020 )
#define UNPK_EXT_ASYNC(a)		( *((a)+4) & 0x00000040 )
#define UNPK_EXT_2(a)			( *((a)+4) & 0x00000080 )
#define UNPK_EXT_3(a)			( *((a)+4) & 0x00000100 )
#define UNPK_EXT_4(a)			( *((a)+4) & 0x00000200 )
#define UNPK_EXT_5(a)			( *((a)+4) & 0x00000400 )
#define UNPK_EXT_6(a)			( *((a)+4) & 0x00000800 )
#define UNPK_EXT_7(a)			( *((a)+4) & 0x00001000 )
#define UNPK_EXT_8(a)			( *((a)+4) & 0x00002000 )
#define UNPK_SPECIAL_RAW(a)		( *((a)+4) & 0x00004000 )
#define UNPK_NCD(a)				( *((a)+4) & 0x00008000 )
#define UNPK_SOFT_GT(a)			( *((a)+4) & 0x00010000 )
#define UNPK_MISS_TRIG(a)		( *((a)+4) & 0x00020000 )

// unpack Trigger Error Mask...
#define UNPK_TESTGT(a)				( *((a)+5) & 0x00020000 )
#define UNPK_TEST50(a)				( *((a)+5) & 0x00040000 )
#define UNPK_TEST10(a)				( *((a)+5) & 0x00080000 )
#define	UNPK_TESTMEM1(a)			( *((a)+5) & 0x00100000 )
#define	UNPK_TESTMEM2(a)			( *((a)+5) & 0x00200000 )
#define	UNPK_SYNCLR16(a)			( *((a)+5) & 0x00400000 )
#define	UNPK_SYNCLR16_WO_TC16(a)	( *((a)+5) & 0x00800000 )
#define	UNPK_SYNCLR24(a)			( *((a)+5) & 0x01000000 )
#define	UNPK_SYNCLR24_WO_TC24(a)	( *((a)+5) & 0x02000000 )
#define	UNPK_SOME_FIFOS_EMPTY(a)	( *((a)+5) & 0x04000000 )
#define	UNPK_SOME_FIFOS_FULL(a)		( *((a)+5) & 0x08000000 )
#define	UNPK_ALL_FIFOS_FULL(a)		( *((a)+5) & 0x10000000 )

// unpack the Trigger Word (lowest 27 bits are significant)
#define UNPK_MTC_TRIGGER(a)     ( ( ( *((a)+3) >> 5 ) & 0x7F80000 ) | ( *((a)+4) & 0x7FFFF ) )
// unpack the Trigger Error bits (lowest 14 bits signficant)
#define UNPK_MTC_ERROR(a) 		( ( *((a)+5) >> 17 ) & 0x3FFF )


// -------------------------------------------------------------------------------------

// other records, not defined in this file
#define CRON_RECORD	   0x43524F4E // 'CRON'
#define VRLY_RECORD	   0x56524C59 // 'VRLY'
#define MSEI_RECORD    0x4D534549 // 'MSEI'

#ifdef __MWERKS__
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// extra code for SHaRC version only

typedef struct MTCReadoutWords {
    u_int32 Word[6];
} aMTCReadoutWords, *aMTCReadoutWordsPtr;

typedef struct FECReadoutWords {
    u_int32 Word[3];
} aFECReadoutWords, *aFECReadoutWordsPtr;

// (be very careful here!  for backward compatibility this is also
//  defined as EpedRecord as used by everyone else.  Not to be confused with EPEDRecord.)
typedef struct EPEDRecord  SHaRC_BankEPED;

// SHaRC uses short forms of trigger bit definitions
#define NHIT_100_LO        TRIG_NHIT_100_LO
#define NHIT_100_MED       TRIG_NHIT_100_MED
#define NHIT_100_HI        TRIG_NHIT_100_HI
#define NHIT_20            TRIG_NHIT_20
#define NHIT_20_LB         TRIG_NHIT_20_LB
#define ESUM_LO            TRIG_ESUM_LO
#define ESUM_HI            TRIG_ESUM_HI
#define OWLN               TRIG_OWLN
#define OWLE_LO            TRIG_OWLE_LO
#define OWLE_HI            TRIG_OWLE_HI
#define PULSE_GT           TRIG_PULSE_GT
#define PRESCALE           TRIG_PRESCALE
#define PEDESTAL           TRIG_PEDESTAL
#define PONG               TRIG_PONG
#define SYNC               TRIG_SYNC
#define EXT_ASYNC          TRIG_EXT_ASYNC
#define HYDROPHONE         TRIG_HYDROPHONE
#define EXT3               TRIG_EXT3
#define EXT4               TRIG_EXT4
#define EXT5               TRIG_EXT5
#define EXT6               TRIG_EXT6
#define EXT7               TRIG_EXT7
#define EXT8               TRIG_EXT8
#define SPECIAL_RAW        TRIG_SPECIAL_RAW
#define NCD                TRIG_NCD_MUX
#define SOFT_GT            TRIG_SOFT_GT

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif // __MWERKS__


#endif //  __RECORD_INFO_H__
