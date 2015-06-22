/* Routines to write zdab files */
/* Based on C code courtesy of Reda Tafirout */
/* PH - 06/01/99 */

#ifndef __PZdabWriter_h__
#define __PZdabWriter_h__

#include <stdio.h>
#include "PZdabFile.h"

// define some predefined sizes (in words) for an FZ file (exchange format)
#define NWREC		3840    // Physical record
#define NPHREC		8   	// Steering record
#define NLOGIC		2		// Logical record
#define NPILOT		10		// Pilot record
#define NBANK		9		// Bank record
#define NEOR		3		// End of run record
#define NEOZ		6		// End of zebra record

#define MAX_NAMELEN	256

// order of the bank entries in the sBankDef array
enum EBankIndex {
	kZDABindex,
	kMASTindex,
	kRHDRindex,
	kEPEDindex,
	kTRIGindex,
	kSOSLindex,
	kCASTindex,
	kCAACindex,
	NUM_BANKS		//  number of bank definitions in following array
};

// bank definition structure
struct SBankDef {
	u_int32		name;		// hollerith name bank ID
	int			id;			// numerical bank ID
	int			nwords;		// word size of bank structure
	int			nlinks;		// number of links
	u_int32		status;		// bank status
	u_int32		iochar[40];	// i/o characteristic, plus extra i/o control words, plus links
};


// class definition
class PZdabWriter {
public:
	PZdabWriter(char *file_name);
	~PZdabWriter();

	int			IsOpen()		{ return zdaboutput != NULL; }
	
	int			Write(PmtEventRecord *aPmtRecord);
	int			WriteBank(u_int32 *bank_ptr, int index);
	u_int32		GetBytesWritten()	{ return mBytesWritten; }
	char	  *	GetFilename()		{ return zdab_output_file; }
	
	static int	GetIndex(u_int32 bank_name);
	static int	GetBankNWords(int index);

private:
	void		AddRecord(u_int32 *data, u_int32 nwords);
	int			WritePhysicalRecord();
	
	u_int32		mBytesWritten;
	u_int32		mbuf[NWREC];
	u_int32		mpr[NPHREC];
	u_int32		mlr[NLOGIC]; 
	u_int32		mpili[NPILOT+2]; 	// 2 extra words for MAST relocation table entries
	u_int32		mbk[NBANK];
	u_int32 	meor[NEOR];
	u_int32 	meoz[NEOZ];
	u_int32		irec, ipos;
	
	MastRecord	mMastData;

	char		zdab_output_file[MAX_NAMELEN];
	FILE 	 *	zdaboutput;

	static SBankDef sBankDef[NUM_BANKS];	// bank definition structures
};

#endif // __PZdabWriter_h__
