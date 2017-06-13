//
// File:		PEventInfoWindow.cxx
//
// Description:	Window to display event information in text form
//
// Notes:		11/11/99 - PH To reduce flickering of text items in this window,
//				I'm now calling SetStringNow() instead of SetString().  Previously,
//				the flickering could be so bad that the text could be invisible
//				most of the time.
//
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <time.h>
#include "PEventInfoWindow.h"
#include "PUtils.h"
#include "PSpeaker.h"
#include "PZdabFile.h"
#include "SnoStr.h"
#include "xsnoed.h"
#include "include/NcdDataTypes.h"

const short		kExtraRow			= 9;
const short		kDirtyEventTimes	= 0x02;

//---------------------------------------------------------------------------
// PEventInfoWindow constructor
//
PEventInfoWindow::PEventInfoWindow(ImageData *data)
				: PWindow(data)
{
	Widget	rc1, rc2;
	int		n;
	Arg		wargs[16];

	mTimeZone = 0;
	mNeedFutureEvent = 0;

	data->mSpeaker->AddListener(this);
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Event Info"); ++n;
	XtSetArg(wargs[n], XmNx, 175); ++n;
	XtSetArg(wargs[n], XmNy, 175); ++n;
	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	SetShell(CreateShell("eiPop",data->toplevel,wargs,n));
	SetMainPane(XtCreateManagedWidget("xsnoedForm", xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	rc1 = XtCreateManagedWidget("eiRC1",xmRowColumnWidgetClass,GetMainPane(),wargs,n);
	
	XtCreateManagedWidget("NHIT:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("GTID:",  	 xmLabelWidgetClass,rc1,NULL,0);
#ifdef SNOPLUS
	XtCreateManagedWidget("CAEN Num:",   xmLabelWidgetClass,rc1,NULL,0);
#endif
	if (data->sum)
	    tw_evtnum_label.CreateLabel("Summed:",rc1,NULL,0);
	else
	    tw_evtnum_label.CreateLabel("Evt Num:",rc1,NULL,0);
	XtCreateManagedWidget("Run Num:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Date:",  	 xmLabelWidgetClass,rc1,NULL,0);
	tw_time_label.CreateLabel("Time:", rc1,NULL,0);
	XtCreateManagedWidget("Prev/Next:",  xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Trigger:",  	 xmLabelWidgetClass,rc1,NULL,0);
#ifdef SNOPLUS
	XtCreateManagedWidget("TUBII Trig:", xmLabelWidgetClass,rc1,NULL,0);
#endif
	XtCreateManagedWidget("Pk/Int/Dif:", xmLabelWidgetClass,rc1,NULL,0);

	XtCreateManagedWidget("Normal:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Owl:",  		 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Low Gain:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Neck:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("BUTTS:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("FECD:",  	 xmLabelWidgetClass,rc1,NULL,0);
#ifndef SNOPLUS
	XtCreateManagedWidget("Shaper:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("MUX:",  	     xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Scope:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("General:",  	 xmLabelWidgetClass,rc1,NULL,0);
#endif
#ifdef MTC_HEX
	XtCreateManagedWidget("Word 1:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Word 2:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Word 3:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Word 4:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Word 5:",  	 xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Word 6:",  	 xmLabelWidgetClass,rc1,NULL,0);
#endif	
	n = 0;
	XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNleftWidget, rc1); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment,	XmATTACH_FORM); ++n;
	rc2 = XtCreateManagedWidget("eiRC2",xmRowColumnWidgetClass,GetMainPane(),wargs,n);
	
	tw_nhit		.CreateLabel("nhit", 	 rc2,NULL,0);
	tw_gtid		.CreateLabel("gtid", 	 rc2,NULL,0);
#ifdef SNOPLUS
	tw_caen		.CreateLabel("caen", 	 rc2,NULL,0);
#endif
	tw_evt		.CreateLabel("evtNum", 	 rc2,NULL,0);
	tw_run		.CreateLabel("runNum", 	 rc2,NULL,0);
	tw_date		.CreateLabel("date", 	 rc2,NULL,0);
	tw_time		.CreateLabel("time", 	 rc2,NULL,0);
	tw_diff		.CreateLabel("diff", 	 rc2,NULL,0);
	tw_trig		.CreateLabel("trig", 	 rc2,NULL,0);
#ifdef SNOPLUS
	tw_tubii	.CreateLabel("tubii", 	 rc2,NULL,0);
#endif
	tw_peak		.CreateLabel("peak", 	 rc2,NULL,0);
	
	// must initialize all 'kNumPmtCounts' structures
	tw_pmt_count[0].label.CreateLabel("normal",	 rc2,NULL,0);
	tw_pmt_count[0].index = NHIT_PMT_NORMAL;
	tw_pmt_count[1].label.CreateLabel("owl", 	 rc2,NULL,0);
	tw_pmt_count[1].index = NHIT_PMT_OWL;
	tw_pmt_count[2].label.CreateLabel("low_gain",rc2,NULL,0);
	tw_pmt_count[2].index = NHIT_PMT_LOW_GAIN;
	tw_pmt_count[3].label.CreateLabel("neck", 	 rc2,NULL,0);
	tw_pmt_count[3].index = NHIT_PMT_NECK;
	tw_pmt_count[4].label.CreateLabel("butts", 	 rc2,NULL,0);
	tw_pmt_count[4].index = NHIT_PMT_BUTTS;
	tw_pmt_count[5].label.CreateLabel("fecd", 	 rc2,NULL,0);
	tw_pmt_count[5].index = NHIT_PMT_FECD;
#ifndef SNOPLUS
	tw_pmt_count[6].label.CreateLabel("shaper",  rc2,NULL,0);
	tw_pmt_count[6].index = NHIT_NCD_SHAPER;
	tw_pmt_count[7].label.CreateLabel("mux", 	 rc2,NULL,0);
	tw_pmt_count[7].index = NHIT_NCD_MUX;
	tw_pmt_count[8].label.CreateLabel("scope", 	 rc2,NULL,0);
	tw_pmt_count[8].index = NHIT_NCD_SCOPE;
	tw_pmt_count[9].label.CreateLabel("general", rc2,NULL,0);
	tw_pmt_count[9].index = NHIT_NCD_GENERAL;
#endif
	
#ifdef MTC_HEX
	tw_word[0]	.CreateLabel("word0001", rc2,NULL,0);
	tw_word[1]	.CreateLabel("word0002", rc2,NULL,0);
	tw_word[2]	.CreateLabel("word0003", rc2,NULL,0);
	tw_word[3]	.CreateLabel("word0004", rc2,NULL,0);
	tw_word[4]	.CreateLabel("word0005", rc2,NULL,0);
	tw_word[5]	.CreateLabel("word0006", rc2,NULL,0);
#endif
	// add extra event data
	mFirstExtraRow = kExtraRow;
	SetExtraPanes(rc1, rc2);
}

void PEventInfoWindow::Listen(int message, void *message_data)
{
	switch (message) {
		case kMessageNewEvent:
		case kMessageEventCleared:
		case kMessageTimeFormatChanged:
		case kMessageGTIDFormatChanged:
		case kMessageHitsChanged:
			SetDirty();
			break;
//		case kMessageHistoryChanged:
		case kMessageNextTimeAvailable:
			if (mNeedFutureEvent) {
				SetDirty(kDirtyEventTimes);
			}
			break;
	}
}

void PEventInfoWindow::ShowRelativeTimes(int only_if_future)
{
	char			buff[256];
	ImageData		*data = GetData();
	HistoryEntry	*entry = getCurrentHistoryEntry(data);
	
	if (entry) {
		if (entry->next_time<0 && only_if_future) {
			// return now if we are waiting for a future event
			return;
		}
		// construct string to display relative times
		if (entry->prev_time >= 0) {
			GetRelativeTimeString(buff, entry->prev_time);
		} else {
			strcpy(buff,"?");
		}
		strcat(buff," / ");
		if (entry->next_time >= 0) {
			GetRelativeTimeString(strchr(buff,'\0'), entry->next_time);
			mNeedFutureEvent = 0;
		} else {
			strcat(buff,"?");
			mNeedFutureEvent = 1;
		}
		tw_diff.SetStringNow(buff);
	} else {
		tw_diff.SetStringNow("? / ?");
		mNeedFutureEvent = 0;
	}
}

char *PEventInfoWindow::GetLabelString(int num)
{
	return(GetData()->extra_evt_data[num]->name);
}

// GetRelativeTimeString [static]
int PEventInfoWindow::GetRelativeTimeString(char *buff, double theTime)
{
	int len;
	if (theTime == 0) {
		strcpy(buff,"0");
		len = 0;
	} else if (theTime < 1000e-9) {
		// display in nanoseconds
		len = sprintf(buff,"%.0f ns",theTime / 1e-9);
	} else if (theTime < 1000e-6) {
		// display in microseconds
		if (theTime < 10e-6) {
			len = sprintf(buff,"%.1f us",theTime / 1e-6);
		} else {
			len = sprintf(buff,"%.0f us",theTime / 1e-6);
		}
	} else if (theTime < 1000e-3) {
		// display in milliseconds
		if (theTime < 10e-3) {
			len = sprintf(buff,"%.1f ms",theTime / 1e-3);
		} else {
			len = sprintf(buff,"%.0f ms",theTime / 1e-3);
		}
	} else if (theTime < 120.0) {
		// display in seconds
		if (theTime < 10.0) {
			len = sprintf(buff,"%.1f sec",theTime);
		} else {
			len = sprintf(buff,"%.0f sec",theTime);
		}
	} else if (theTime < 7200.0) {
		// display in minutes
		len = sprintf(buff,"%.0f min",theTime / 60.0);
	} else if (theTime < 172800.0) {
		// display in hours
		len = sprintf(buff,"%.0f hr",theTime / 3600.0);
	} else if (theTime < 63113472.0) {
		// display in days
		len = sprintf(buff,"%.0f day",theTime / 86400.0);
	} else {
		// display in years
		len = sprintf(buff,"%.0f yr",theTime / 31556736.0);
	}
	return(len);
}

// GetTriggerWord [static]
u_int32 PEventInfoWindow::GetTriggerWord(aPmtEventRecord *pmtRecord)
{
    u_int32 *mtc_word, trig_word;
    
    mtc_word = 	(u_int32 *)&pmtRecord->TriggerCardData;
    trig_word = (mtc_word[3] & 0xff000000UL) |
                (mtc_word[4] & 0x0007ffffUL);
    /* add synthetic "NONE" bit if all other bits zero */
    if (!trig_word) {
        if (isOrphan(pmtRecord)) {
            trig_word = TRIGGER_ORPHAN;
        } else {
            trig_word = TRIGGER_NONE;
        }
    }
#ifdef SNOPLUS
    // add synthetic "CAEN" and "TUBII" trigger bits
    if (PZdabFile::GetExtendedData(pmtRecord, SUB_TYPE_CAEN)) {
        trig_word |= TRIGGER_CAEN;
    }
    if (PZdabFile::GetExtendedData(pmtRecord, SUB_TYPE_TUBII)) {
        trig_word |= TRIGGER_TUBII;
    }
#endif
    return(trig_word);
}

// GetTriggerString [static]
// (now puts synthetic bits last)
int PEventInfoWindow::GetTriggerString(char *buff, ImageData *data)
{
	int			i, len=0;
	u_int32		mask, trig_word;
	
	trig_word = data->trig_word;

	if (trig_word) {
        mask = 0x80000000UL;
        for (i=31; i>=0; --i) {
            if (trig_word & mask) {
                if (len) buff[len++] = ',';
                strcpy(buff+len, SnoStr::sXsnoedTrig[i]);
                len += strlen(SnoStr::sXsnoedTrig[i]);
            }
            mask >>= 1;
        }
    } else {
        strcpy(buff, "<none>");
        len = strlen(buff);
    }
	return(len);
}

// GetNhitString [static]
int PEventInfoWindow::GetNhitString(char *buff, ImageData *data)
{
	int	len;
	if (data->event_nhit == (int)data->total_sum_nhit) {
		if (data->event_nhit == data->num_disp) {
			len = sprintf(buff,"%d",(int)data->event_nhit);
		} else {
			len = sprintf(buff,"%d of %d",data->num_disp,(int)data->event_nhit);
		}
	} else {
		if (data->event_nhit == data->num_disp) {
			len = sprintf(buff,"%d (%ld)",(int)data->event_nhit,(long)data->total_sum_nhit);
		} else {
			len = sprintf(buff,"%d of %d (%ld)",data->num_disp,(int)data->event_nhit,(long)data->total_sum_nhit);
		}
	}
	return(len);
}

// UpdateSelf
void PEventInfoWindow::UpdateSelf()
{
	int			i;
	ImageData	*data = GetData();
	char		buff[256];
	struct tm	*tms;

#ifdef PRINT_DRAWS
	Printf("-updateEventInfo\n");
#endif

	if (IsDirty() == kDirtyEventTimes) {
		// only update relative times if only the event times are dirty
		ShowRelativeTimes(1);
		return;
	}
	if (mTimeZone != data->time_zone) {
		mTimeZone = data->time_zone;
		switch (mTimeZone) {
			default:
				tw_time_label.SetStringNow("Time:");
				break;
			case kTimeZoneLocal:
				tw_time_label.SetStringNow("Loc Time:");
				break;
			case kTimeZoneUTC:
				tw_time_label.SetStringNow("UTC Time:");
				break;
		}
	}
	GetNhitString(buff, data);
	tw_nhit.SetStringNow(buff);
	sprintf(buff,data->hex_id ? "0x%.6lx" : "%ld",data->event_id);
	tw_gtid.SetStringNow(buff);
#ifdef SNOPLUS
    u_int32 *caen = data->caenData;
    // (dont show during a sum until we can figure out how to avoid
    // freeing the displayed CAEN data in this mode)
    if (caen && !data->sum) {
	    sprintf(buff,data->hex_id ? "0x%.6x" : "%d",(unsigned)caen[2]);
    } else {
        strcpy(buff, "-");
    }
	tw_caen.SetStringNow(buff);
    if (data->tubiiGT) {
	    sprintf(buff, "0x%.6x", (int)data->tubiiTrig);
	    if (data->event_id != data->tubiiGT) {
	        sprintf(strchr(buff,'\0'), data->hex_id ? " (0x%.6x)" : " (%d)", data->tubiiGT);
	    }
    } else {
        strcpy(buff, "-");
    }
	tw_tubii.SetStringNow(buff);
#endif
	if (data->sum) {
		sprintf(buff,"%ld",(long)data->sum_event_count);
	} else {
		sprintf(buff,"%ld",data->event_num);
	}
	tw_evt.SetStringNow(buff);
	if (data->sub_run < 0) {
		sprintf(buff,"%ld",data->run_number);
	} else {
		sprintf(buff,"%ld_%.3d",data->run_number,data->sub_run);
	}
	tw_run.SetStringNow(buff);
	
	if (data->event_time != 0.0) {
		/* display time in Sudbury, local or GMT time zone */
		tms = getTms(data->event_time, data->time_zone);
		
		sprintf(buff,"%.2d/%.2d/%d",
					 tms->tm_mon+1, tms->tm_mday, tms->tm_year+1900);
		tw_date.SetStringNow(buff);
		
		sprintf(buff,"%.2d:%.2d:%.2d.%.7ld",
					 tms->tm_hour, tms->tm_min, tms->tm_sec,
					 (long)((data->event_time - (long)data->event_time) * 1e7 + 0.5));
		tw_time.SetStringNow(buff);
	} else {
		tw_date.SetStringNow("00/00/0000");
		tw_time.SetStringNow("00:00:00.0000000");
	}
	
	ShowRelativeTimes();
				
	/* display trigger bits */
	GetTriggerString(buff, data);
	tw_trig.SetStringNow(buff);
	
	sprintf(buff,"%d / %d / %d", (int)UNPK_MTC_PEAK(data->mtc_word),
								 (int)UNPK_MTC_INT(data->mtc_word),
								 (int)UNPK_MTC_DIFF(data->mtc_word));
	tw_peak.SetStringNow(buff);
	
	/* update pmt counts */
	int *pmt_counts = getPmtCounts(data);
	int *pmt_extras = getPmtCounts(NULL);
	for (i=0; i<kNumPmtCounts; ++i) {
		int index = tw_pmt_count[i].index;
		int len = sprintf(buff, "%d", pmt_counts[index]);
		// display total hits on this channel
		// (unless displaying CMOS rates where hit_count is used for rates)
		if (pmt_extras[index] && data->wDataType!=IDM_CMOS_RATES) {
			sprintf(buff+len, " (%d)", pmt_counts[index] + pmt_extras[index]);
		}
		tw_pmt_count[i].label.SetStringNow(buff);
	}
#ifdef MTC_HEX
	/* update raw trigger word */
	for (i=0; i<6; ++i) {
		sprintf(buff,"%.8lx",(long)data->mtc_word[i]);
		tw_word[i].SetStringNow(buff);
	}
#endif
	// update the extra event data
	if (SetExtraNum(data->extra_evt_num)) {
#ifdef MTC_HEX
		Widget w = tw_word[0].GetWidget();
#else
		Widget w = tw_pmt_count[kNumPmtCounts-1].label.GetWidget();
#endif
		ResizeToFit(w);
	}
	for (i=0; i<GetExtraNum(); ++i) {
		sprintf(buff, data->extra_evt_fmt[i], data->extra_evt_data[i]->value);
		SetExtraLabel(i, buff);
	}
}


void PEventInfoWindow::SetSum(int sumOn)
{
	if (sumOn)
	    tw_evtnum_label.SetStringNow("Summed:");
	else
	    tw_evtnum_label.SetStringNow("Evt Num:");
}
