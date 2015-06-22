#include <stdio.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include "time.h"
#include "PRecordInfoWindow.h"
#include "ImageData.h"
#include "PZdabFile.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "SnoStr.h"
#include "xsnoed.h"

const short	kCrateTub = 20;
const short	kMarginH = 10;
const short	kMarginV = 5;

char *PRecordInfoWindow::sRecName[kNumHdrRec] = { 
	"RHDR","TRIG","EPED","CAST","CAAC","SOSL"
};
char *PRecordInfoWindow::sRecLabel[kNumHdrRec] = {
	"Run Number:\nStart Time:\nRun Type:\nSources:\nCrate Mask:",
	"Triggers:\nNHIT100L:\nNHIT100M:\nNHIT100H:\nNHIT20:\n"
	    "NHIT20LB:\nESUMLO:\nESUMHI:\nOWLN:\nOWLELO:\nOWLEHI:\n"
	    "Pulser Rate:\nControl Register:\nLockout Width:\nPrescale:\nGTID:",
	"Ped Width:\nCoarse Delay:\nFine Delay:\nQ DAC Setting:\nCrate/Cards:\nType:\nFlag:\nGTID:",
	"Source:\nStatus:\nNum Ropes:\nPosition:\nError:\nOrientation:\n"
	    "Rope ID's:\nRope Lengths:\nRope Errors:",
	"Position:\nRotation:\nRope Lengths:",
	"Source:\nStatus:\nDye Cell:\nFilters:\nN2 Pressure:\nN2 Flow:"
};

static MenuStruct rec_type_menu[] = {
	{ "Run Record (RHDR)",			0, XK_R,kHdrRec_RHDR + 1,  	NULL, 0, MENU_RADIO },
	{ "Trigger Record (TRIG)",		0, XK_T,kHdrRec_TRIG + 1,  	NULL, 0, MENU_RADIO },
	{ "ECA Record (EPED)",			0, XK_E,kHdrRec_EPED + 1,  	NULL, 0, MENU_RADIO },
	{ "Calibration Source (CAST)",	0, XK_C,kHdrRec_CAST + 1,  	NULL, 0, MENU_RADIO },
	{ "AV Position (CAAC)",			0, XK_A,kHdrRec_CAAC + 1,  	NULL, 0, MENU_RADIO },
	{ "Laser Status (SOSL)",		0, XK_L,kHdrRec_SOSL + 1,  	NULL, 0, MENU_RADIO }
};
static MenuStruct rec_main_menu[] = {
	// this label gets set at dynamically
	{ "-------------------------",	0, 0,	0, 	rec_type_menu, 	XtNumber(rec_type_menu), 0 },
};
static char *ropeIDs[] = {
    "Unknown", "North", "South", "East", "West", "Central1", "Central2", "Central3",
    "GuideTube1", "GuideTube2", "GuideTube3", "GuideTube4", "GuideTube5", "GuideTube6",
    "GasUmbilical", "LaserUmbilical", "RotatingUmbilical"
};


//---------------------------------------------------------------------------------
// PRecordInfoWindow constructor
//
PRecordInfoWindow::PRecordInfoWindow(ImageData *data)
			  : PWindow(data)
{
	Widget	w;
	int		n;
	Arg		wargs[16];
	
	mLastIndex = -1;
	mSizeDelta = 1;
	mHasTime = 0;
	mHasGTID = 0;
	
	// initialize the number of lines for each type of displayed record
	for (int i=0; i<kNumHdrRec; ++i) {
	    char *pt = sRecLabel[i];
	    for (n=1; ; ++n) {
	        pt = strchr(pt, '\n');  // look for next line
	        if (!pt) break;
	        ++pt;                   // step to character after newline
	    }
	    mRecLines[i] = n;           // set count of number of lines in this record
	}
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Record Info"); ++n;
	XtSetArg(wargs[n], XmNx, 200); ++n;
	XtSetArg(wargs[n], XmNy, 150); ++n;
	XtSetArg(wargs[n], XmNwidth, 300); ++n;
	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 60); ++n;
	SetShell(CreateShell("recPop",data->toplevel,wargs,n));
	SetMainPane(w = XtCreateManagedWidget( "xsnoedForm", xmFormWidgetClass,GetShell(),NULL,0));
	
	// Create Menubar
	CreateMenu(NULL,rec_main_menu, XtNumber(rec_main_menu),this);
	
	n = 0;
	XtSetArg(wargs[n], XmNleftAttachment,	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNtopAttachment,	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNtopWidget,		mMenu->GetWidget()); ++n;
	XtSetArg(wargs[n], XmNtopOffset,		kMarginV); ++n;
	XtSetArg(wargs[n], XmNleftOffset,		kMarginH); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	mText1 = XtCreateManagedWidget("col1",xmLabelWidgetClass,w,wargs,n);
	
	n = 0;
	XtSetArg(wargs[n], XmNleftAttachment,	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNleftWidget,		mText1); ++n;
	XtSetArg(wargs[n], XmNtopAttachment,	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNtopWidget,		mMenu->GetWidget()); ++n;
	XtSetArg(wargs[n], XmNtopOffset,		kMarginV); ++n;
	XtSetArg(wargs[n], XmNleftOffset,		2 * kMarginH); ++n;
	XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
	mText2 = XtCreateManagedWidget("col2",xmLabelWidgetClass,w,wargs,n);
	
	GetMenu()->SetToggle(data->record_index+1,	 TRUE);

	// set the menu label
	PMenu::SetLabel(GetMenu()->GetMenuList(), rec_type_menu[mData->record_index].name);
	
	data->mSpeaker->AddListener(this);
}

PRecordInfoWindow::~PRecordInfoWindow()
{
}

void PRecordInfoWindow::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageNewHeaderRecord:
			// force an update if we are displaying the record type that changed
			if (*(int *)dataPt == mLastIndex) {
				SetDirty();
			}
			break;
		case kMessageTimeFormatChanged:
			if (mHasTime) SetDirty();
			break;
		case kMessageGTIDFormatChanged:
			if (mHasGTID) SetDirty();
			break;
	}
}

void PRecordInfoWindow::ResizeToFit()
{
	int n;
	Arg wargs[10];
	
	n = 0;
	Dimension textHeight, textTop;
	XtSetArg(wargs[n], XmNheight, &textHeight);  ++n;
	XtSetArg(wargs[n], XmNy, &textTop);  ++n;
	XtGetValues(mText1, wargs, n);
	n = 0;
	Dimension width;
	XtSetArg(wargs[n], XmNwidth, &width); ++n;
	XtGetValues(GetShell(), wargs, n);
	// use a trick here -- change the width by a single pixel
	// to force resizing of the menu buttons to fit the new text
	Resize(width+mSizeDelta, textHeight+textTop+kMarginV);
	mSizeDelta *= -1;
}

void PRecordInfoWindow::Show()
{
	PWindow::Show();	// let the base class do the work
	
	if (!WasResized()) {
		ResizeToFit();
	}
}

void PRecordInfoWindow::UpdateSelf()
{
	int			i, n;
	u_int32		t32;
	char		*pt, buff[1024];
	ImageData	*data = mData;
	int			index = data->record_index;
	
	mHasTime = 0;
	mHasGTID = 0;
	
	if (mLastIndex != index) {
		mLastIndex = index;
		setLabelString(mText1, sRecLabel[index]);
		// resize window to fix new text height if we are visible
		if (IsVisible()) {
			ResizeToFit();
		}
	}
	if (!data->mHdrRec[index]) {
		strcpy(buff,"-");
		for (i=1; i<mRecLines[index]; ++i) {
			strcat(buff,"\n-");
		}
	} else {
	
		pt = buff;
	
		switch (index) {
	
			case kHdrRec_RHDR: {
				SBankRHDR *rhdr = (SBankRHDR *)data->mHdrRec[index];
				pt += sprintf(pt,"%lu\n",(unsigned long)rhdr->RunNumber);
				
				u_int32 tmp = rhdr->Date;
				long year = tmp / 10000UL;
				tmp -= year * 10000UL;
				long month = tmp / 100UL;
				tmp -= month * 100UL;
				long day = tmp;
				tmp = rhdr->Time;
				long hour = tmp / 1000000UL;
				tmp -= hour * 1000000UL;
				long min = tmp / 10000UL;
				tmp -= min * 10000UL;
				long sec = tmp / 100UL;
				tmp -= sec * 100UL;
				long frac = tmp;
		
				// convert to our time zone
				struct tm tms;
				tms.tm_sec = sec;
				tms.tm_min = min;
				tms.tm_hour = hour;
				tms.tm_mday = day;
				tms.tm_mon = month - 1;
				tms.tm_year = year - 1900;
				tms.tm_isdst = 0;
				time_t theTime = mktime(&tms);
#ifdef __MACHTEN__
				theTime -= 5 * 3600L;	// convert to GMT
#else
				theTime	-= timezone;	// convert to GMT
#endif
		
				struct tm *tm2 = getTms((double)theTime, mData->time_zone);
		
				pt += sprintf(pt,"%.2ld/%.2ld/%ld %.2ld:%.2ld:%.2ld.%.2ld",
						(long)(tm2->tm_mon + 1), (long)tm2->tm_mday, (long)(tm2->tm_year + 1900),
						(long)tm2->tm_hour, (long)tm2->tm_min, (long)tm2->tm_sec, (long)frac);
				if (mData->time_zone == kTimeZoneLocal) {
					pt += sprintf(pt," Loc\n");
				} else if (mData->time_zone == kTimeZoneUTC) {
					pt += sprintf(pt," UTC\n");
				} else {
					pt += sprintf(pt,"\n");
				}
				
				n = 0;
				tmp = rhdr->RunMask;
				if (tmp) {
					pt += SnoStr::GetList(pt, SnoStr::sRunType, tmp);
					pt += sprintf(pt,"\n");
				} else {
					pt += sprintf(pt,"<none>\n");
				}
				
				n = 0;
				tmp = rhdr->SourceMask;
				if (tmp) {
					pt += SnoStr::GetList(pt, SnoStr::sSourceMask, tmp);
					pt += sprintf(pt,"\n");
				} else {
					pt += sprintf(pt,"<none>\n");
				}
				
				n = 0;
				tmp = rhdr->GTCrateMsk;
				if (tmp) {
					int first_crate = -1;
					for (i=0; i<32; ++i) {
						// is this crate masked in?
						if (tmp & (1UL << i)) {
							// is next crate on too?
							if (i<31 && (tmp&(1UL<<(i+1))) && i!=kCrateTub && (i+1)!=kCrateTub) {
								// yes: don't print it yet
								if (first_crate < 0) first_crate = i;
							} else {
								// print comma separator if necessary
								if (n) {
									pt += sprintf(pt, ",");
								}
								if (first_crate >= 0) {
									// print first crate of range
									pt += sprintf(pt,"%d-",first_crate);
									first_crate = -1;
								}
								// print the crate name
								if (i==kCrateTub) {	// is it the TUB?
									pt += sprintf(pt,"Tub");
								} else {
									pt += sprintf(pt,"%d",i);
								}
								++n;
							}
						}
					}
				} else {
					pt += sprintf(pt,"<none>");
				}
				mHasTime = 1;
			} break;
			
			case kHdrRec_TRIG: {
				SBankTRIG *trig = (SBankTRIG *)data->mHdrRec[index];
				// the trigger bits are different in the TRIG record
				n = 0;
				t32 = trig->TriggerMask;
				for (i=0; i<32; ++i) {
					if (t32 & (1UL << i)) {
						pt += sprintf(pt,"%s%s", n?",":"", SnoStr::sTrigMask[i]);
						++n;
					}
				}
				// print thresholds and zeros for all 10 triggers
				pt += sprintf(pt, "\n");
				for (i=0; i<10; ++i) {
					pt += sprintf(pt, "%4ld, z=%4ld (%+4ld)\n",
								(long)((&trig->n100lo)[i]), (long)((&trig->n100lo_zero)[i]),
								(long)((&trig->n100lo)[i] - (&trig->n100lo_zero)[i]));
				}
				pt += sprintf(pt, "%ld\n", (long)trig->PulserRate);
				pt += sprintf(pt, "%ld\n", (long)trig->ControlRegister);
				pt += sprintf(pt, "%ld\n", (long)trig->reg_LockoutWidth);
				pt += sprintf(pt, "%ld\n", (long)trig->reg_Prescale);
				if (data->hex_id) {
					pt += sprintf(pt, "0x%.6lx", (long)trig->GTID);
				} else {
					pt += sprintf(pt, "%ld", (long)trig->GTID);
				}
				mHasGTID = 1;
			} break;
			
			case kHdrRec_EPED: {
				SBankEPED *eped = (SBankEPED *)data->mHdrRec[index];
				pt += sprintf(pt,"%ld\n",(long)eped->ped_width);
				pt += sprintf(pt,"%ld\n",(long)eped->ped_delay_coarse);
				pt += sprintf(pt,"%ld\n",(long)eped->ped_delay_fine);
				pt += sprintf(pt,"%ld\n",(long)eped->qinj_dacsetting);
				pt += sprintf(pt,"%d / %s\n",(int)eped->halfCrateID & 0x7f,
							(eped->halfCrateID & EPED_SECOND_HALF) ? "8-15" : "0-7");
				if ((unsigned)eped->CalibrationType < kNumEPEDTypes) {
					pt += sprintf(pt,"%s\n",SnoStr::sEPEDType[eped->CalibrationType]);
				} else {
					pt += sprintf(pt,"unknown (0x%lx)\n",(long)eped->CalibrationType);
				}
				if ((unsigned)eped->Flag < kNumEPEDFlags) {
					pt += sprintf(pt,"%s\n",SnoStr::sEPEDFlag[eped->Flag]);
				} else {
					pt += sprintf(pt,"unknown (0x%lx)\n",(long)eped->Flag);
				}
				if (data->hex_id) {
					pt += sprintf(pt, "0x%.6lx", (long)eped->GTID);
				} else {
					pt += sprintf(pt, "%ld", (long)eped->GTID);
				}
				mHasGTID = 1;
			} break;
			
			case kHdrRec_CAST: {
				SBankCAST *cast = (SBankCAST *)data->mHdrRec[index];
				pt += sprintf(pt,"%ld\n",(long)cast->sourceID);
				if ((unsigned)cast->status < kNumManipStatus) {
					pt += sprintf(pt,"%s\n",SnoStr::sManipStatus[cast->status]);
				} else {
					pt += sprintf(pt,"0x%lx\n",(long)cast->status);
				}
				pt += sprintf(pt,"%ld\n",(long)cast->numRopes);
				pt += sprintf(pt,"(%.1f,%.1f,%.1f)\n",cast->position[0],cast->position[1],cast->position[2]);
				pt += sprintf(pt,"%.1f\n",cast->positionError);
				unsigned orientation = (unsigned)cast->orientation;
				if (orientation < kNumSourceOrientations) {
				    pt += sprintf(pt,"%s\n", SnoStr::sSourceOrientation[orientation]);
				} else {
				    pt += sprintf(pt,"unknown (%d)\n", orientation);
				}
				char *comma = "";
				for (i=0; i<(int)cast->numRopes; ++i) {
				    long id = (long)cast->ropeStatus[i].ropeID;
				    if (id > 0 && id < (long)(sizeof(ropeIDs) / sizeof(char *))) {
				        pt += sprintf(pt,"%s%s",comma,ropeIDs[id]);
				    } else {
				        pt += sprintf(pt,"%s%ld",comma,(long)cast->ropeStatus[i].ropeID);
				    }
				    comma = ",";
				}
				pt += sprintf(pt,"\n");
				comma = "";
				for (i=0; i<(int)cast->numRopes; ++i) {
				    pt += sprintf(pt,"%s%.1f",comma,cast->ropeStatus[i].length);
				    comma = ",";
				}
				pt += sprintf(pt,"\n");
				comma = "";
				for (i=0; i<(int)cast->numRopes; ++i) {
				    pt += sprintf(pt,"%s%.1f",comma,cast->ropeStatus[i].encoderError);
				    comma = ",";
				}
			} break;
			
			case kHdrRec_CAAC: {
				SBankCAAC *caac = (SBankCAAC *)data->mHdrRec[index];
				pt += sprintf(pt,"(%.1f,%.1f,%.1f)\n",caac->position[0],caac->position[1],caac->position[2]);
				pt += sprintf(pt,"(%.4f,%.4f,%.4f)\n",caac->rotation[0],caac->rotation[1],caac->rotation[2]);
				pt += sprintf(pt,"%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f",
							caac->ropeLength[0], caac->ropeLength[1], caac->ropeLength[2], caac->ropeLength[3], 
							caac->ropeLength[4], caac->ropeLength[5], caac->ropeLength[6]);
			} break;
				
			case kHdrRec_SOSL: {
				SBankSOSL *sosl = (SBankSOSL *)data->mHdrRec[index];
				pt += sprintf(pt,"%ld\n",(long)sosl->sourceID);
				pt += sprintf(pt,"0x%lx\n",(long)sosl->status);
				pt += sprintf(pt,"%ld\n",(long)sosl->dyeCellNumber);
				pt += sprintf(pt,"%ld, %ld\n",(long)sosl->filterWheel1Position,(long)sosl->filterWheel2Position);
				pt += sprintf(pt,"%.3f\n",sosl->pressure1);
				pt += sprintf(pt,"%.3f",sosl->nitrogenFlowRate);
			} break;
		}
	}
	setLabelString(mText2, buff);
}

void PRecordInfoWindow::DoMenuCommand(int anID)
{
	int		index;
	
	switch (anID) {
		case kHdrRec_RHDR+1:
		case kHdrRec_TRIG+1:
		case kHdrRec_EPED+1:
		case kHdrRec_CAST+1:
		case kHdrRec_CAAC+1:
		case kHdrRec_SOSL+1:
			index = mData->record_index + 1;
			PMenu::UpdateTogglePair(&index);
			mData->record_index = index - 1;
			PMenu::SetLabel(GetMenu()->GetMenuList(), rec_type_menu[index-1].name);
			SetDirty();
			break;
	}
}

