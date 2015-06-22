/*
 * File:		zdab_file.h - zdab read functions header file
 *
 * Author:		P. Harvey
 *
 * Revisions:	06/26/98 - PH Created
 */
#ifndef __PZdabFile_h__
#define __PZdabFile_h__

#include <stdio.h>
#include "include/Record_Info.h"

#ifdef SWAP_BYTES
#define SWAP_INT32(a,b)	swap_bytes((char *)(a),(b), sizeof(int32))
#define SWAP_INT16(a,b)	swap_bytes((char *)(a),(b), sizeof(int16))
#define SWAP_FLOAT(a,b) swap_bytes((char *)(a),(b), sizeof(float))
#define SWAP_DOUBLE(a,b) swap_bytes((char *)(a),(b), sizeof(double))
#define SWAP_PMT_RECORD(a) swap_bytes((char *)(a), sizeof(PmtEventRecord)/sizeof(int32), sizeof(int32))
#else
#define SWAP_INT32(a,b)
#define SWAP_INT16(a,b)
#define SWAP_FLOAT(a,b)
#define SWAP_DOUBLE(a,b)
#define SWAP_PMT_RECORD(a)
#endif

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/* definitions require for ZDAB/ZEBRA */

#define ZEBRA_BLOCKSIZE 		3840		// maximum size of zebra record (32-bit words)

/* flag bits for steering block MPR[4] - PH 07/03/98 */
#define ZEBRA_EMERGENCY_STOP	0x80000000UL
#define ZEBRA_END_OF_RUN		0x40000000UL
#define ZEBRA_START_OF_RUN		0x20000000UL
#define ZEBRA_BLOCK_SIZE_MASK	0x00ffffffUL

#define ZEBRA_SIG0				0x0123cdefUL
#define ZEBRA_SIG1				0x80708070UL
#define ZEBRA_SIG2				0x4321abcdUL
#define ZEBRA_SIG3				0x80618061UL


typedef struct nZDAB{
	u_int32	next_bank; 		// next bank
	u_int32	supp_bank; 		// supp bank
	u_int32	supp_link;		// supp link
	u_int32	bank_number;	// numerical bank-id
	u_int32	bank_name;		// hollerith name bank-id
	u_int32	total_links;	// total number of links
	u_int32	struct_links;	// number of structural links
	u_int32	data_words;		// number of datawords
	u_int32	status;			// status

} nZDAB, *nZDABPtr;

#define NZDAB_WORD_SIZE		(sizeof(nZDAB) / sizeof(u_int32))

typedef struct ZEBRA_ST {	// Steering clock 
	u_int32	MPR[8];			//	Physical record control words
	// PH 07/03/98
	// 0 - 0x0123cdef
	// 1 - 0x80708070
	// 2 - 0x4321abcd
	// 3 - 0x80618061
	// 4 - zebra bank size in 32-bit words (usually 0xf00 = 3840)
	// 5 - bank number in file
	// 6 - offset in 32-bit words to the start of the control record in this bank
	// 7 - number of fast blocks to follow (don't contain steering blocks)
} ZEBRA_ST;	
	
typedef struct CONTROL {
	u_int32	length;
	u_int32	recordtype;
} CONTROL, *CONTROLPtr;
	
typedef struct PILOT{
	u_int32	pilot0;
	u_int32	pilot1;
	u_int32	pilot2;
	u_int32	pilot3;
	u_int32	pilot4;
	u_int32	pilot5;
	u_int32	pilot6;
	u_int32	pilot7;
	u_int32	pilot8;
	u_int32	pilot9;
} PILOT, *PILOTPtr;

typedef struct MTC{
	u_int32	mtc0;
	u_int32	mtc1;
	u_int32	mtc2;
	u_int32	mtc3;
	u_int32	mtc4;
	u_int32	mtc5;
} MTC, *MTCPtr;	
	
typedef  struct pilotHeader{
	CONTROL   control;              //  2 u_int32 : Record Length Control
	PILOT     pilot;				// 10 u_int32 : Pilot (ZEBRA)
} pilotHeader, *pilotHeaderPtr;

class PackedCharArray {
public:
    PackedCharArray(char *data)      { mData = data; }

#ifdef SWAP_BYTES
    char    Get(int index)           { return(mData[index ^ 0x03]); }
    void    Set(int index, char val) { mData[index ^ 0x03] = val;   }
#else
    char    Get(int index)           { return(mData[index]); }
    void    Set(int index, char val) { mData[index] = val;   }
#endif

private:
    char    *mData;
};

//-------------------------------------------------------------------------


/* class definition */
class PZdabFile {
public:
							PZdabFile();
	virtual 				~PZdabFile();
	
	int						Init(FILE *inFile);
	void					Free();
	
	// return next nZDAB record from file
	nZDAB				  *	NextRecord();
	
	// return next specified data type from file
	PmtEventRecord		  *	NextPmt();
	u_int32				  *	NextBank(u_int32 bank_name);
	
	// extract various records from nZDAB record
	PmtEventRecord		  *	GetPmtRecord(nZDAB *nzdabPtr);
	static u_int32		  *	GetBank(nZDAB *nzdabPtr, u_int32 bank_name=0);
	
	static u_int32			GetSize(PmtEventRecord *pmtRecord);
	static u_int32        * GetExtendedData(PmtEventRecord *pmtRecord, int subType);
	static u_int32        * GetNcdData(PmtEventRecord *pmtRecord)
	                            { return GetExtendedData(pmtRecord, SUB_TYPE_NCD); }
	
	static void				DumpRecord(u_int32 *bankData, int bankSize, u_int32 bankName, u_int32 lastGTID=0);
	static void				DumpRecord(nZDAB *nzdabPtr, u_int32 lastGTID=0);
	static void				DumpHex(nZDAB *nzdabPtr, int numToPrint=16);
	
	static int				GetVerbose()			{ return sVerbose; }
	static void				SetVerbose(int on)		{ sVerbose=on; }
	
	static u_int32			BankName(char *bank_name_string);
	static char			  *	BankNameString(u_int32 bank_name);

	static void				AddSubField(u_int32 **io_sub_header_pt, int sub_type, u_int32 numBytes);
	
protected:
	FILE		  *	mFile;

private:

	u_int32			mWordOffset;
	u_int32			mBlockCount, mRecordCount;
	int				mBufferEmpty;
	u_int32		  *	mRecBuffer;
	u_int32			mRecBuffsize;		// size of temporary ZDAB buffer
	u_int32		  *	mBuffPtr32;
	u_int32			mBytesRead, mWordsTotal, mBytesTotal;
	u_int32			mLastGTID;
	nZDAB		  *	mLastRecord;
	
	static int		sVerbose;		// 0=off, 1=dump records, 2=hex dump non-zdab, 3=hex dump all
};


extern "C" {
	int	zdab_get_subrun(char *filename);
	int zdab_set_subrun(char *filename, int subrun);
	long zdab_get_run(char *filename);
	long zdab_set_run(char *filename, long run);
    void swap_bytes(char *valPt, int num, int size);
	void swap_PmtRecord(aPmtEventRecord *aPmtRecord);
}

// PmtEventRecord utility functions
double	get50MHzTime(PmtEventRecord *pmtRecord);
double	get50MHzTimeMax();
int		isOrphan(PmtEventRecord *pmtRecord);



#endif // __PZdabFile_h__

