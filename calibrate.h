#ifndef __CALIBRATE_H__
#define __CALIBRATE_H__

#define MAGIC_NUMBER	0x1b2cd3e5UL
#define INVALID_CAL		-9999		/* data value indicating invalid calibration */

/*  Please read notes at the bottom of this file ... */

typedef struct {
	float tac_offset;
	float tac_slope;
	float qhl_offset;
	float qhl_slope;
	float qhs_offset;
	float qhs_slope;
	float qlx_offset;
	float qlx_slope;
} Cmos;

typedef struct DBasePMT {
	short int ccc;  /*Calibration is invalid if most significant bit is set  ...*/
	Cmos cmos[16];
} dBasePMT;

typedef struct DBase {
	unsigned long	magic_number;	/* magic number to identify database file (0x1b2cd3e4) */
	int				version;		/* database version number */
	int				nentries;		/* number of PMT entries */
	int				nvalid;			/* number of PMT's with valid calibrations */
	int				validity[4];	/* validity range (start date, start time, end date, end time) */
	float			walk[2][10];	/* time walk correction constants (version 1) */
	float			walk2[400];		/* time walk correction constants (version 2) */
	float			walk3[2];		/* time walk correction constants for high charge (version 2) */
	dBasePMT		pmt[1];			/* constants for first PMT (nentries-1 others to follow) */
} dBase;

/* default database file name */
#define TITLES_FILE "cal_default.dat" //used

#define TSLP_DEBUG_FILE "tslp_debug_file.dat"

#define NENTRIES 9728


#define NCRATES 19
#define TITLES_HDR_SIZE 30 /* SNOMAN currently uses 30 words for the header */
#define TSLP_HDR_WORDS 31 + 2 /* NT and NP plus up to 31 time samples */
/* Each sample requires 12 bits plus one flag word so ... */
/* The following constants are only approximate - they are based on ~19 timesamplings
over a 400 ns period */
#define TSLP_EARLY 3
#define TSLP_LATE 2

#define TSLP_CURL_FAIL 2 /* Allow for up to 2 failures in tac-curl region */

#ifdef SWAP_BYTES
#define BANK_PDST 0x54534450
#define BANK_TSLP 0x504c5354
#else
#define BANK_PDST 0x50445354
#define BANK_TSLP 0x54534c50
#endif


#define PECA_TITLES_FILE "peca_default.dat"
#define TECA_TITLES_FILE "teca_default.dat"
#define CAL_TITLES_FILE "cal_titles_default.dat"



dBase *loadPecaCalibration( FILE *fd, int nentries );
dBase *loadTecaCalibration( FILE *fd, dBase *dbase, int nentries ); 
dBase *loadType1Calibration( FILE *fp, int nentries );


#define CAL_TITLES_TIME_OFFSET 44 /* Just in case we would want to shift things */


#define CAL_TIME_OFFSET   138   /* THIS SHOULD EVENTUALLY COME FROM CALIBRATION																	 FILE!!!! */



dBase *newCalibrationDatabase(char *name, char *defaultDirectory, int mode = 0 );
double getCalibratedTac(dBase *dbase, int tac, int cell, int num, int qhs);
double getCalibratedQhs(dBase *dbase, int qhs, int cell, int num);
double getCalibratedQhl(dBase *dbase, int qhs, int cell, int num);
double getCalibratedQlx(dBase *dbase, int qhs, int cell, int num);


#endif

