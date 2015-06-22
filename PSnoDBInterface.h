//
// Virtual base class for interface to load values from the SNODB
//
// Created: 11/15/00 - P. Harvey
//

#ifndef __PSnoDBInterface_h__
#define __PSnoDBInterface_h__

#include "include/Record_Info.h"

enum ESnoDB_Parameter {
	kSnoDB_CalibrateTime = 1,
	kSnoDB_CalibrateCharge,
	kSnoDB_ChargePedestal,
	kSnoDB_ParameterMax
};

typedef void * WidgetPtr;

class PSnoDBInterface {
public:
				PSnoDBInterface(WidgetPtr label);
	virtual		~PSnoDBInterface();
	
	virtual void SetParamString(char *str);
	virtual int  GetData(FECReadoutData *pmt, CalibratedPMT *calPmt, ESnoDB_Parameter type,
						 u_int32 date, u_int32 time, int count) = 0;
						 
	char	  *	GetParamString();
	void 		SetMessageLabel(WidgetPtr label)	{ mMessageLabel = label; }
	
	void Message(char *fmt, ...);
	
private:
	WidgetPtr	mMessageLabel;
	char	  *	mParamString;
};

#endif // __PSnoDBInterface_h__
