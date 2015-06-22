//
// File:		PCalSimple.cxx
//
// Created:		09/13/00 - P. Harvey
//
// Description:	Simple calibration object (with or without Q vs T walk correction)
//
#include <stdio.h>
#include <stdlib.h>
#include "PCalSimple.h"
#include "CUtils.h"
#include "calibrate.h"

PCalSimple::PCalSimple()
{
	mDBase = 0;
	mDoWalk = 1;
}

PCalSimple::~PCalSimple()
{
	Free();
}

void PCalSimple::Init(char *filename, char *path)
{
	Free();		// free old database if it existed
	
	// load calibration constants
	mDBase = newCalibrationDatabase(filename, path);
	
	if (mDBase) {
		Printf("%d PMT's with valid calibrations\n",(int)mDBase->nvalid);
		Printf("Validity range of calibration constants: %d to %d\n",
				(int)mDBase->validity[0], (int)mDBase->validity[2]);
		mStatus = 0;
	} else {
		mStatus = -1;
	}
}

void PCalSimple::Free()
{
	if (mDBase) {
		free(mDBase);
		mDBase = 0;
	}
	mStatus = -1;
}

void PCalSimple::Calibrate(int calFlags)
{
	if (mStatus >= 0) {
		if (calFlags & kCalTac) {
			mCalTac = getCalibratedTac(mDBase, mRawTac, mCell, mIndex, mDoWalk ? mRawQhs : -1);
		}
		if (calFlags & kCalQhs) {
			mCalQhs = getCalibratedQhs(mDBase, mRawQhs, mCell, mIndex);
		}
		if (calFlags & kCalQhl) {
			mCalQhl = getCalibratedQhl(mDBase, mRawQhl, mCell, mIndex);
		}
		if (calFlags & kCalQlx) {
			mCalQlx = getCalibratedQlx(mDBase, mRawQlx, mCell, mIndex);
		}
	} else {
		PCal::Calibrate(calFlags);
	}
}

