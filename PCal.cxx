//
// File:		PCal.cxx
//
// Created:		09/13/00 - P. Harvey
//
// Description:	Base class for calibration objects
//
#include "PCal.h"
#include "include/Record_Info.h"

PCal::PCal()
{
	mStatus		= -1;
	mRunNumber	= 0xffffffffUL;
}

// Set - set member variables from PMT bundle
void PCal::Set(u_int32 *pmt_bundle)
{
	mCrate		= UNPK_CRATE_ID(pmt_bundle);
	mCard		= UNPK_BOARD_ID(pmt_bundle);
	mChannel	= UNPK_CHANNEL_ID(pmt_bundle);
	mCell		= UNPK_CELL_ID(pmt_bundle);
	mIndex  	= mCrate * 512 + mCard * 32 + mChannel;
	mRawTac		= UNPK_TAC(pmt_bundle);
	mRawQhs		= UNPK_QHS(pmt_bundle);
	mRawQhl		= UNPK_QHL(pmt_bundle);
	mRawQlx		= UNPK_QLX(pmt_bundle);
}

void PCal::SetChannel(int cr, int ca, int ch, int cell)
{
	mCrate		= cr;
	mCard		= ca;
	mChannel	= ch;
	mCell		= cell;
	mIndex		= cr * 512 + ca * 32 + ch;
}

void PCal::SetChannel(int index, int cell)
{
	mIndex		= index;
	mCrate		= index / 512;
	index	   -= mCrate * 512;
	mCard		= index / 32;
	mChannel	= index - mCard * 32;
	mCell		= cell;
}

void PCal::SetRaw(int tac, int qhs, int qhl, int qlx)
{
	mRawTac		= tac;
	mRawQhs		= qhs;
	mRawQhl		= qhl;
	mRawQlx		= qlx;
}

// Calibrate - calculate calibrated data values
//
// This virtual function must be overridden by derived classes
// to perform the necessary calibration and set the value of
// the calibrated data members.  The implementation is free
// to set the values of more calibrated member variables than
// indicated by the flags, but it must at minimum set the
// specified variables.  If a calibration can not be done,
// the variables should be set to -9999.
//
void PCal::Calibrate(int calFlags)
{
	mCalTac = mCalQhs = mCalQhl = mCalQlx = -9999;
}
