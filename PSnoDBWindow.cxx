//
// File:		PSnoDBWindow.cxx
//
// Description:	SNODB database viewer control window
//
// Created:		11/15/00 - P. Harvey
//
#include <stdlib.h>
#include <time.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include "PSnoDBWindow.h"
#include "PSnoDBInterface.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"
#include "PZdabFile.h"
#include "PUtils.h"

#define WINDOW_WIDTH		400
#define WINDOW_HEIGHT		260

static MenuStruct type_menu[] = {
	{ "Calibrated Times",	0, 0,	kSnoDB_CalibrateTime, 		NULL, 0, 0 },
	{ "Calibrated Charges",	0, 0,	kSnoDB_CalibrateCharge, 	NULL, 0, 0 },
	{ "Charge Pedestals",	0, 0,	kSnoDB_ChargePedestal, 		NULL, 0, 0 },
};


PSnoDBWindow::PSnoDBWindow(ImageData *data)
			: PWindow(data)
{
	int		n;
	Arg		wargs[10];
	Widget	w, but;
	
	mDataType = kSnoDB_CalibrateTime;
	mRawData[kSnoDB_CalibrateTime] = 2250;
	mRawData[kSnoDB_CalibrateCharge] = 620;
	mRawData[kSnoDB_ChargePedestal] = -1;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "SNODB Viewer"); ++n;
	XtSetArg(wargs[n], XmNx, 50); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, WINDOW_WIDTH); ++n;
	XtSetArg(wargs[n], XmNminHeight, WINDOW_HEIGHT); ++n;
	SetShell(CreateShell("snodbPop",data->toplevel,wargs,n));
	SetMainPane(w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 220); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	but = XtCreateManagedWidget("Load (1)",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)LoadProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNy, 220); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNleftWidget, but); ++n;
	XtSetArg(wargs[n], XmNleftOffset, 16); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	but = XtCreateManagedWidget("Diff (1-2)",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)DiffProc,this);

	n = 0;
	XtSetArg(wargs[n], XmNy, 220); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNwidth, 100); ++n;
	but = XtCreateManagedWidget("Done",xmPushButtonWidgetClass,w,wargs,n);
	XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)CancelProc,GetShell());

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 8); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	mMessageLabel = XtCreateManagedWidget("message_text",xmLabelWidgetClass,w,wargs,n);
	setLabelString(mMessageLabel, "Ready");
	
	n = 0;
	XtSetArg(wargs[n], XmNy, 33); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtCreateManagedWidget("sep1",xmSeparatorWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 45); ++n;
	XtCreateManagedWidget("SNODB Server:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
	XtSetArg(wargs[n], XmNy, 40); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mParamText = XtCreateManagedWidget("server",xmTextWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 80); ++n;
	XtCreateManagedWidget("1)   Date:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 90); ++n;
	XtSetArg(wargs[n], XmNy, 75); ++n;
	XtSetArg(wargs[n], XmNwidth, 120); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mDateText1 = XtCreateManagedWidget("date1",xmTextWidgetClass,w,wargs,n);
	setTextString(mDateText1, "20000101");
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 215); ++n;
	XtSetArg(wargs[n], XmNy, 80); ++n;
	XtCreateManagedWidget("Time:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 267); ++n;
	XtSetArg(wargs[n], XmNy, 75); ++n;
	XtSetArg(wargs[n], XmNwidth, 120); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mTimeText1 = XtCreateManagedWidget("time1",xmTextWidgetClass,w,wargs,n);
	setTextString(mTimeText1,"00000");
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtCreateManagedWidget("2)   Date:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 90); ++n;
	XtSetArg(wargs[n], XmNy, 110); ++n;
	XtSetArg(wargs[n], XmNwidth, 120); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mDateText2 = XtCreateManagedWidget("date2",xmTextWidgetClass,w,wargs,n);
	setTextString(mDateText2, "20000601");
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 215); ++n;
	XtSetArg(wargs[n], XmNy, 115); ++n;
	XtCreateManagedWidget("Time:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 267); ++n;
	XtSetArg(wargs[n], XmNy, 110); ++n;
	XtSetArg(wargs[n], XmNwidth, 120); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mTimeText2 = XtCreateManagedWidget("time2",xmTextWidgetClass,w,wargs,n);
	setTextString(mTimeText2,"00000");

	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 150); ++n;
	XtCreateManagedWidget("Raw Data Value:", xmLabelWidgetClass,w,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNx, 140); ++n;
	XtSetArg(wargs[n], XmNy, 145); ++n;
	XtSetArg(wargs[n], XmNwidth, 60); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mRawDataText = XtCreateManagedWidget("raw",xmTextWidgetClass,w,wargs,n);
	setTextString(mRawDataText, "2250");

	n = 0;
	XtSetArg(wargs[n], XmNx, 202); ++n;
	XtSetArg(wargs[n], XmNy, 150); ++n;
	XtCreateManagedWidget("CMOS Cell Num:", xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNy, 145); ++n;
	XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
	XtSetArg(wargs[n], XmNwidth, 60); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
	mCellText = XtCreateManagedWidget("cell",xmTextWidgetClass,w,wargs,n);
	setTextString(mCellText, "0");
	
	n = 0;
	XtSetArg(wargs[n], XmNx, 16); ++n;
	XtSetArg(wargs[n], XmNy, 185); ++n;
	XtCreateManagedWidget("Parameter:", xmLabelWidgetClass,w,wargs,n);

	// create 'type' option menu
	Widget sub_menu = XmCreatePulldownMenu(w, "optionMenu", NULL, 0);
	CreateMenu(sub_menu, type_menu, XtNumber(type_menu), this);
	
	// create option menu widget
	n = 0;
	XtSetArg(wargs[n], XmNx, 100); ++n;
	XtSetArg(wargs[n], XmNy, 179); ++n;
	XtSetArg(wargs[n], XmNmarginHeight, 0); ++n;
	XtSetArg(wargs[n], XmNmarginWidth, 0); ++n;
	XtSetArg(wargs[n], XmNsubMenuId, sub_menu); ++n;
	Widget option_menu = XmCreateOptionMenu(w, "type_opt", wargs, n);
	XtManageChild(option_menu);
	
#ifdef ROOT_FILE
	PSnoDBInterface *newQSnoDBInterface(WidgetPtr label);
	mSnoDBInterface = newQSnoDBInterface((WidgetPtr)mMessageLabel);
	setTextString(mParamText, mSnoDBInterface->GetParamString());
#else
	mSnoDBInterface = NULL;
#endif

}


PSnoDBWindow::~PSnoDBWindow()
{
	delete mSnoDBInterface;
}

// Retrieve - get data from SNO Database
// (mSnoDBInterface must be non-NULL to call this routine)
void PSnoDBWindow::ViewSnoDB(int code)
{
	if (!mSnoDBInterface) {
		setLabelString(mMessageLabel, "Internal error -- No SNODB interface object!");
		return;
	}
	mSnoDBInterface->Message("Working...");
	
	ImageData *data = GetData();
	
	char *str;
	int i, cr, ca, ch;
	int nhits = 19 * 512;
	int size = nhits * (sizeof(FECReadoutData) + sizeof(CalibratedPMT))
	                  + sizeof(PmtEventRecord) + 2 * sizeof(SubFieldHeader) + sizeof(u_int32);
	char *buff = new char[size];
	if (!buff) {
		setLabelString(mMessageLabel, "Out of memory!");
		return;
	}
	memset(buff,0,size);
	
	// get cell number
	str = XmTextGetString(mCellText);
	int cell = atoi(str);
	XtFree(str);
	
	if (cell<0 || cell>15) {
		setLabelString(mMessageLabel, "Cell out of range!  Must be between 0 and 15.");
		delete [] buff;
		return;
	}
	
	PmtEventRecord *per = (PmtEventRecord *)buff;
	
	// get date and time
	str = XmTextGetString(mDateText1);
	u_int32 date1 = atol(str);
	XtFree(str);
	
	str = XmTextGetString(mTimeText1);
	u_int32 time1 = atol(str);
	XtFree(str);

	// set the time of the event
	struct tm	tms;
	tms.tm_sec = time1 % 100;
	tms.tm_min = (time1 / 100) % 100;
	tms.tm_hour = (time1 / 10000);
	tms.tm_mday = date1 % 100;
	tms.tm_mon = ((date1 / 100) % 100) - 1;
	tms.tm_year = (date1 / 10000) - 1900;
	tms.tm_isdst = 0;
	double event_time = mktime(&tms);
	/* mktime returns UTC time given local time but sno time zero is UTC */
	/* so we must do a conversion from local to GMT here (no DST because we set isdst to zero) */
#ifdef __MACHTEN__
	event_time -= 5 * 3600L;	// convert to GMT
#else
	event_time	-= timezone;	// convert to GMT
#endif

	double time_10mhz = (event_time - data->sno_time_zero) * 1e7 + 0.5;
	u_int32 hi10mhz = (u_int32)(time_10mhz / 4294967296.0);

	// set the 10MHz time in the MTC
	per->TriggerCardData.Bc10_1 = (u_int32)(time_10mhz - hi10mhz * 4294967296.0);
	per->TriggerCardData.Bc10_2 = hi10mhz;
//
// fill in the necessary elements of the raw PMT bundles
//
	// fill in crate/card/channel/cell
	FECReadoutData *pmt = (FECReadoutData *)(per + 1);
	for (cr=0; cr<19; ++cr) {
		for (ca=0; ca<16; ++ca) {
			for (ch=0; ch<32; ++ch) {
				pmt->CrateID = cr;
				pmt->BoardID = ca;
				pmt->ChannelID = ch;
				pmt->CellID = cell;
				++pmt;
			}
		}
	}
	pmt = (FECReadoutData *)(per + 1);	// rewind pmt pointer to first PMT
	
	// fill in raw data if necessary
	if (mDataType != kSnoDB_ChargePedestal) {
		str = XmTextGetString(mRawDataText);
		// get raw value (must toggle upper bit in FECReadoutData structure)
		int raw = (u_int32)atoi(str);
		XtFree(str);
		for (i=0; i<nhits; ++i) {
			pmt->Qhs = pmt->Qhl = pmt->Qlx = pmt->TAC = raw;
			// set complement of sign bit
			if (!(raw & 0x800)) {
				pmt->SignQhs = pmt->SignQhl = pmt->SignQlx = pmt->SignTAC = 1;
			}
			++pmt;
		}
		pmt = (FECReadoutData *)(per + 1);	// rewind pmt pointer to first PMT
	}
//
// fill in the calibrated data
//
	u_int32 *sub_header = &per->CalPckType;

	// must set the size of this sub-field before calling AddSubField()
	// (from the sub-field header to the end)
	*sub_header |= ((u_int32 *)((u_int32 *)(per + 1) + nhits * 3) - sub_header);

	// add calibrated data flags to indicate that qslope is not applied
	PZdabFile::AddSubField(&sub_header, SUB_TYPE_CAL_FLAGS, sizeof(u_int32));
	
	// add new sub-field to extended PmtEventRecord
	PZdabFile::AddSubField(&sub_header, SUB_TYPE_CALIBRATED, nhits * sizeof(CalibratedPMT));
	
    // get pointer to start of calibrated data
    CalibratedPMT *calPmt = (CalibratedPMT *)(sub_header + 1);

	// set SNODB interface parameter string
	str = XmTextGetString(mParamText);
	mSnoDBInterface->SetParamString(str);
	XtFree(str);

	int totLoaded = 0;
	CalibratedPMT *calPmt2 = NULL;
	u_int32 date2, time2;
	if (code) {
		calPmt2 = new CalibratedPMT[512];
		if (!calPmt2) {
			setLabelString(mMessageLabel, "Out of memory!");
			delete [] buff;
			return;
		}
		memset(calPmt2, 0, 512*sizeof(CalibratedPMT));
	
		// get 2nd date and time
		str = XmTextGetString(mDateText2);
		date2 = atol(str);
		XtFree(str);
	
		str = XmTextGetString(mTimeText2);
		time2 = atol(str);
		XtFree(str);
	}
	
	// help the user by setting the appropriate displayed data type
	data->mMainWindow->SelectMenuItem(IDM_PRECALIBRATED);
	switch (mDataType) {
		case kSnoDB_CalibrateTime:
			if (data->wCalibrated != IDM_TAC) {
				data->mMainWindow->SelectMenuItem(IDM_TAC);
			}
			break;
		case kSnoDB_CalibrateCharge:
		case kSnoDB_ChargePedestal:
			if (data->wCalibrated<IDM_QHS || data->wCalibrated>IDM_QLX) {
				data->mMainWindow->SelectMenuItem(IDM_QHS);
			}
			break;
		default:
			break;
	}

	const double	kUpdateTime = 2;		// update time in secs
	double  		next_display_time = double_time() + kUpdateTime;
	int				displayed = 0;
	
	// loop over all crates
	for (cr=0; cr<19; ++cr) {
		int offset = cr * 512;
		// get data from SNODB
		int numLoaded = mSnoDBInterface->GetData(pmt+offset, calPmt+offset, mDataType, date1, time1, 512);
	
		if (numLoaded < 0) {
			delete [] buff;
			delete [] calPmt2;
			return;
		}
		totLoaded += numLoaded;
		if (code) {
	
			// load 2nd set of constants
			numLoaded += mSnoDBInterface->GetData(pmt+offset, calPmt2, mDataType, date2, time2, 512);
			
			// take the difference
			for (i=0; i<512; ++i) {
				calPmt[i+offset].tac -= calPmt2[i].tac;
				calPmt[i+offset].qhs -= calPmt2[i].qhs;
				calPmt[i+offset].qhl -= calPmt2[i].qhl;
				calPmt[i+offset].qlx -= calPmt2[i].qlx;
			}
		}
		// set nhits to include this crate
		per->NPmtHit = (cr + 1) * 512;
		
		// update the display periodically if this is taking a long time
		if (cr==18 || double_time()>next_display_time) {
			if (!displayed) {
				displayed = 1;
				addToHistory(data, per, HISTORY_ALL);
				xsnoed_event(data, per);
			} else {
				xsnoed_replace(data, per);
			}
			// update all windows now
			PWindow::HandleUpdates();
			next_display_time = double_time() + kUpdateTime;
		}
	}
	delete [] calPmt2;
	delete [] buff;		// delete our event buffer
	
	if (totLoaded >= 0) {
		mSnoDBInterface->Message("Done (loaded %d banks)", totLoaded);
	}
}

void PSnoDBWindow::LoadProc(Widget w,PSnoDBWindow *db_win, caddr_t call_data)
{
	db_win->ViewSnoDB(0);
}

void PSnoDBWindow::DiffProc(Widget w,PSnoDBWindow *db_win, caddr_t call_data)
{
	db_win->ViewSnoDB(1);
}

void PSnoDBWindow::CancelProc(Widget w, Widget aShell, caddr_t call_data)
{
	XtDestroyWidget(aShell);
}

void PSnoDBWindow::DoMenuCommand(int anID)
{
	char *str = XmTextGetString(mRawDataText);
	// get raw value (must toggle upper bit in FECReadoutData structure)
	int raw = (u_int32)atoi(str);
	XtFree(str);
	mRawData[mDataType] = raw;			// save old raw value
	char buff[32];
	if (anID == kSnoDB_ChargePedestal) {
		strcpy(buff,"n/a");
	} else {
		sprintf(buff,"%d",mRawData[anID]);
	}
	setTextString(mRawDataText, buff);	// set new value in text edit
	
	// set the parameter type
	mDataType = (ESnoDB_Parameter)anID;
}

