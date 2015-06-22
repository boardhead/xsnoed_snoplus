//
// QSnoDB interface to load values from the SNODB
//
// Created: 11/15/00 - P. Harvey
//

#ifndef __QSnoDBInterface_h__
#define __QSnoDBInterface_h__

#include "TObject.h"
#include "PSnoDBInterface.h"
#include "QSnoCal.h"

class QSnoDBInterface : public QSnoCal, public PSnoDBInterface {
public:

	QSnoDBInterface();
	QSnoDBInterface(WidgetPtr label);
	virtual ~QSnoDBInterface();
	
	virtual void	SetParamString(char *str);
	virtual int		GetData(FECReadoutData *pmt, CalibratedPMT *calPmt, ESnoDB_Parameter type,
									u_int32 date, u_int32 time, int count);
	virtual int		GetChannelStatus(FECReadoutData *pmt, u_int32 *status_out,
									u_int32 date, u_int32 time, int count);
	virtual int		GetCardStatus(FECReadoutData *pmt, u_int32 *status_out,
									u_int32 date, u_int32 time, int count);

	ClassDef(QSnoDBInterface,0)		// Interface to SNO DB

protected:
	void			InitQSnoDBInterface();
	
	QBank **ftslp2;		// secondary time calibrations
	QBank **fpdst2;		// secondary charge pedestals
	QBank **fdqch;		// DQCH banks
	QBank **fdqcr;		// DQCR banks
};

#endif // __QSnoDBInterface_h__
