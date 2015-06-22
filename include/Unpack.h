/*
  Unpack.h
  macros and bitmasks to unpack the data in the raw data banks defined in
  Record_Info.h
 */

/* Record ID's */
/* #define RUN_RECORD            'RUN '*/
#define RUN_RECORD               1381322272
/*#define PMT_RECORD		 'PMT ' */
#define PMT_RECORD		 1347245088
#define PEDESTAL_RECORD 	 'PEDE'
/*#define TRIGGER_RECORD 	 'TRIG'*/
#define TRIGGER_RECORD 		 1414678855

/* version numbers...
   as of Feb 1997: */
#define DAQ_CODE_VERSION 				0
#define RUN_RECORD_VERSION 				1
#define PMT_RECORD_VERSION 				1
#define TRIGGER_RECORD_VERSION 			1

/* calibration types, in trigger record */
#define T_PEDESTALS             0x0001
#define T_SLOPES                0x0002
#define Q_PEDESTALS             0x0004
#define Q_SLOPES                0x0008

/* time-zone definitions */
#define SEATTLE	8UL
#define SUDBURY 5UL

/* define the masks
   Run Mask... */
#define NEUTRINO_RUN		0x0001
#define SOURCE_RUN			0x0002
#define CALIB_RUN			0x0004
#define NCD_RUN				0x0008
#define SALT_RUN			0x0010
#define POISON_RUN			0x0020
#define PARTIAL_FILL_RUN	0x0040
#define AIR_FILL_RUN		0x0080
#define D2O_RUN				0x0100
#define H2O_RUN				0x0200
#define MINISNO_RUN			0x0400

/* Source Mask... */
#define ROTATING_SRC		0x00001
#define LASER_SRC			0x00002
#define SONO_SRC			0x00004
#define N16_SRC				0x00008
#define N17_SRC				0x00010
#define NAI_SRC				0x00020
#define LI8_SRC				0x00040
#define PT_SRC				0x00080
#define CF_HI_SRC			0x00100
#define CF_LO_SRC			0x00200
#define U_SRC				0x00400
#define TH_SRC				0x00800
#define P_LI7_SRC			0x01000
#define WATER_SAMPLER		0x02000
#define PROP_COUNTER_SRC	0x04000
#define SINGLE_NCD_SRC		0x08000
#define SELF_CALIB_SRC		0x10000

/* MTC trigger mask, for use with UNPK_MTC_TRIGGER */
#define OWLE_LO				0x00000001
#define OWLE_HI				0x00000002
#define PULSE_GT			0x00000004
#define PRESCALE			0x00000008
#define PEDESTAL			0x00000010
#define PONG				0x00000020
#define SYNC				0x00000040
#define EXT_ASYNC			0x00000080
#define EXT2				0x00000100
#define EXT3				0x00000200
#define EXT4				0x00000400
#define EXT5				0x00000800
#define EXT6				0x00001000
#define EXT7				0x00002000
#define EXT8				0x00004000
#define SPECIAL_RAW			0x00008000
#define NCD					0x00010000
#define SOFT_GT				0x00020000
#define MISS_TRIG			0x00040000
#define NHIT_100_LO			0x00080000
#define NHIT_100_MED		0x00100000
#define NHIT_100_HI			0x00200000
#define NHIT_20				0x00400000
#define NHIT_20_LB			0x00800000
#define ESUM_LO				0x01000000
#define ESUM_HI				0x02000000
#define OWLN				0x04000000

/* MTC trigger error mask, for use with UNPK_MTC_ERROR */
#define TESTGT				0x00000001
#define TEST50				0x00000002
#define TEST10				0x00000004
#define TESTMEM1			0x00000008
#define TESTMEM2			0x00000010
#define SYNCLR16			0x00000020
#define SYNCLR16_WO_TC16	0x00000040
#define SYNCLR24			0x00000080
#define SYNCLR24_WO_TC24	0x00000100
#define SOME_FIFOS_EMPTY	0x00000200
#define SOME_FIFOS_FULL		0x00000400
#define ALL_FIFOS_FULL		0x00000800

/* MTC monitoring info, for use with UNPK_MTC_MONITOR */
#define PEAK_MASK               0x000003FF
#define DIFF_MASK               0x000FFC00
#define INT_MASK                0x3FF00000

/* unpacking "routines" by definition! Looks awful, but is fast...
 "a" in the following is a pointer to 3 longwords as read out from the FEC32.
*/
#define UNPK_MISSED_COUNT(a) ( (*(a+1) >> 28) & 0x1 )
#define UNPK_NC_CC(a)		( (*(a+1) >> 29) & 0x1 )
#define UNPK_LGI_SELECT(a)	( (*(a+1) >> 30) & 0x1 )
#define UNPK_CMOS_ES_16(a)	( (*(a+1) >> 31) & 0x1 )
#define UNPK_CGT_ES_16(a)	( (*(a) >> 30) & 0x1 )
#define UNPK_CGT_ES_24(a)	( (*(a) >> 31) & 0x1 )
#define UNPK_FEC_ERROR(a)     ( ( ( *(a) >> 24 ) | (*(a+1) >> 28 ) ) & 0x3F )

#define UNPK_QLX(a)			( (*(a+1) & 0x00000800) == 0x800 ? (*(a+1) & 0x000007FF) : (*(a+1) & 0x000007FF) + 2048 )
#define UNPK_QHS(a)			( ((*(a+1) >>16) & 0x00000800) == 0x800 ? ((*(a+1) >>16) & 0x000007FF) : ((*(a+1) >>16) & 0x000007FF) + 2048 )
#define UNPK_QHL(a)			( (*(a+2) & 0x00000800)  == 0x800 ? (*(a+2) & 0x000007FF) : (*(a+2) & 0x000007FF) + 2048 )
#define UNPK_TAC(a)			( ((*(a+2) >>16) & 0x00000800)  == 0x800 ? ((*(a+2) >>16) & 0x000007FF) : ((*(a+2) >>16) & 0x000007FF) + 2048 )
#define UNPK_CELL_ID(a) 	( (*(a+1) >> 12) & 0x0000000F )
#define UNPK_CHANNEL_ID(a) 	( (*(a) >> 16) & 0x0000001F )
#define UNPK_BOARD_ID(a) 	( (*(a) >> 26) & 0x0000000F )
#define UNPK_CRATE_ID(a) 	( (*(a) >> 21) & 0x0000001F )
#define UNPK_FEC_GT_ID(a)	( *(a) & 0x0000FFFF ) | ( (*(a+2) << 4) & 0x000F0000 ) | ( (*(a+2) >> 8) & 0x00F00000 )

#define UNPK_MTC_BC50_1(a)		( (*(a+1) >> 11) & 0x7FF )
#define UNPK_MTC_BC50_2(a)		( *(a+2) )
#define UNPK_MTC_BC10_1(a)		( *(a) )
#define UNPK_MTC_BC10_2(a)		( *(a+1) & 0x1FFFFF )
#define UNPK_MTC_GT_ID(a)		( *(a+3) & 0x00FFFFFF)

#define UNPK_MTC_MONITOR(a)  ( ( ( *(a+4) >> 19 ) && 0x1FFF ) || ( ( *(a+5) << 13 ) && 0x3FFFE000 ) )
/* unpack the Trigger Word (lowest 27 bits are significant) */
#define UNPK_MTC_TRIGGER(a)     ( ( ( *(a+3) >> 5 ) & 0x7F80000 ) || ( *(a+4) & 0x7FFFF ) )
/* unpack the Trigger Error bits (lowest 14 bits signficant) */
#define UNPK_MTC_ERROR(a) 		( ( *(a+5) >> 17 ) & 0x3FFF )

