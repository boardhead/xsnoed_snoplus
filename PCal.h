//
// File:		PCal.h
//
// Created:		09/11/00 - P. Harvey
//
// Description:	Base class for calibration objects
//
// Notes:	1)	To use this class, follow these steps:
//
//				 a)	Call Init() to load the calibration constants from file.
//					Init should load the constants then set the value of mStatus.
//					A negative status code indicates that an error occured and
//					that calibration is not possible.  The status should be
//					verified with GetStatus() before proceeding.
//
//				 b)	Call Set(), or SetChannel() and SetRaw(), to initialize the
//					raw channel variables.
//
//				 c)	Call Calibrate() with the flags indicating which variables
//					you want to calibrate.
//
//				 d)	Call GetTac(), GetQhs(), GetQhl() and GetQlx() to get the
//					calibrated data.  The result is undefined if you attempt
//					to retrieve a value that wasn't specified in the cal flags.
//
#ifndef __PCal_h__
#define __PCal_h__

// include files
#include "include/sno_sys.h"

// constants
enum ECalFlags {
	kCalNone		= 0,
	kCalTac			= 0x01,
	kCalQhs			= 0x02,
	kCalQhl			= 0x04,
	kCalQlx			= 0x08,
	kCalAll			= kCalTac | kCalQhs | kCalQhl | kCalQlx
};


// class definition
class PCal {
public:
					PCal();
	virtual			~PCal()							{ }
	
	virtual void	Init(char *file, char *path=0)	{ }
	virtual void	Free()							{ }

	virtual void	Calibrate(int calFlags=kCalAll);
	
	void			Set(u_int32 *pmt_bundle);		// sets channel ID and raw data values
	
	void			SetRunNumber(u_int32 run)		{ mRunNumber = run; }
	void			SetChannel(int cr, int ca, int ch, int cell);
	void			SetChannel(int index, int cell);
	void			SetRaw(int tac, int qhs, int qhl, int qlx);
	
	int				GetCrate()						{ return mCrate;	}
	int				GetCard()						{ return mCard;		}
	int				GetChannel()					{ return mChannel;	}
	int				GetCell()						{ return mCell;		}
	int				GetIndex()						{ return mIndex;	}
	int				GetStatus()						{ return mStatus;	}
	
	int				GetRawTac()						{ return mRawTac;	}
	int				GetRawQhs()						{ return mRawQhs;	}
	int				GetRawQhl()						{ return mRawQhl;	}
	int				GetRawQlx()						{ return mRawQlx;	}
	
	double			GetTac()						{ return mCalTac;	}
	double			GetQhs()						{ return mCalQhs;	}
	double			GetQhl()						{ return mCalQhl;	}
	double			GetQlx()						{ return mCalQlx;	}

protected:
	// channel identification
	int				mCrate;			// crate number
	int				mCard;			// FEC card (slot) number
	int				mChannel;		// FEC channel
	int				mCell;			// CMOS cell number
	int				mIndex;			// crate * 512 + card * 32 + channel
	
	// raw data values
	int				mRawTac;		// raw TAC ADC reading
	int				mRawQhs;		// raw high gain short integrate charge ADC reading
	int				mRawQhl;		// raw high gain long integrate charge ADC reading
	int				mRawQlx;		// raw low gain charge ADC reading
	
	// calibrated data values
	double			mCalTac;		// calibrated time
	double			mCalQhs;		// calibrated Qhs
	double			mCalQhl;		// calibrated Qhl
	double			mCalQlx;		// calibrates Qlx
	
	// calibrator status
	int				mStatus;		// >= 0 if calibrator is functional
	u_int32			mRunNumber;		// run number
};


#endif // __PCal_h__
