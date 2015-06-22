//
// File:		PNCDInfoWindow.cxx
//
// Description:	Window to display NCD event information in text form
//
// Revisions:	06/26/03 - PH Created
//
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <time.h>
#include "PNCDInfoWindow.h"
#include "PUtils.h"
#include "PSpeaker.h"
#include "SnoStr.h"
#include "xsnoed.h"
#include "include/NcdDataTypes.h"

enum EDataFlag
{
    kDataUnknown,
    kNoData,
    kHasData
};

const short     kMaxUnknownRecords      = 20;
const unsigned  kNumGenericSubRecords   = 10;

static char *sGenericSubRecord[kNumGenericSubRecords] = {
                "Undefined",
                "NCD model pulser settings",
                "NCD model log amp task",
                "NCD model linearity",
                "NCD model threshold",
                "NCD model step PDS",
                "Orca shaper model scalers",
                "Orca HP pulser model",
                "Orca live time scalers",
                "NCD scope calibration"
            };

static char *sLiveTimeScalerStatus[] = { "Endrun","Startrun","Invalid","Middlerun" };


//---------------------------------------------------------------------------
// PNCDInfoWindow constructor
//
PNCDInfoWindow::PNCDInfoWindow(ImageData *data)
				: PWindow(data)
{
	int		n;
	Arg		wargs[16];

	mString = NULL;

	data->mSpeaker->AddListener(this);
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "NCD Info"); ++n;
	XtSetArg(wargs[n], XmNx, 250); ++n;
	XtSetArg(wargs[n], XmNy, 175); ++n;
	XtSetArg(wargs[n], XmNwidth, 250); ++n;
	XtSetArg(wargs[n], XmNheight, 400); ++n;
	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	SetShell(CreateShell("ncdInfoPop",data->toplevel,wargs,n));
	
	n = 0;
	XtSetArg(wargs[n], XmNheight, 3000); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_POSITION); ++n;
	SetMainPane(XtCreateManagedWidget("xsnoedForm", xmFormWidgetClass,GetShell(),wargs,n));

    n = 0;
	XtSetArg(wargs[n], XmNalignment,		XmALIGNMENT_BEGINNING); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, XmATTACH_POSITION); ++n;
	mNCDLabel.CreateLabel("ncd_info", GetMainPane(),wargs,n);
	
	mDataFlag = kDataUnknown;
}

PNCDInfoWindow::~PNCDInfoWindow()
{
    ClearString();
}

void PNCDInfoWindow::Listen(int message, void *message_data)
{
	switch (message) {
		case kMessageNewEvent:
		case kMessageEventCleared:
		case kMessageTimeFormatChanged:
		case kMessageGTIDFormatChanged:
			SetDirty();
			break;
	}
}

void PNCDInfoWindow::ClearString()
{
    if (mString) {
        XmStringFree(mString);
        mString = NULL;
    }
}

void PNCDInfoWindow::AddString(char *str, int title)
{
	XmString str1 = XmStringCreateLtoR(str, (char *)(title ? XmFONTLIST_DEFAULT_TAG : "FIXED"));
	
	if (mString) {
	    XmString str2 = XmStringConcat(mString, str1);
	    XmStringFree(mString);
    	XmStringFree(str1);
	    mString = str2;
	} else {
	    mString = str1;
	}
}

char *PNCDInfoWindow::GetLabelString(int num)
{
	return(GetData()->extra_evt_data[num]->name);
}

// UpdateSelf
void PNCDInfoWindow::UpdateSelf()
{
	ImageData	*data = GetData();
	const int   kBuffSize = 8192;
	char		buff[kBuffSize];
	char        *pt;
	unsigned    card, chan, mask;
	int         unknown_count = 0;

#ifdef PRINT_DRAWS
	Printf("-updateNCDInfo\n");
#endif

	if (data->ncdData) {
	    mDataFlag = kHasData;
	    ClearString();
	    // get pointer to ncd sub-field header
	    u_int32 *ncd = data->ncdData - 1;
	    // get pointer to end of the NCD record
	    u_int32 *ncd_end = ncd + (*ncd & SUB_LENGTH_MASK);
	    sprintf(buff, "  Record size:    %ld\n", (*ncd & SUB_LENGTH_MASK));
	    AddString(buff, 0);
	    // increment ncd pointer to next record
	    while (++ncd < ncd_end) {
	        int len = 0;
	        switch (*ncd & kNcdDataTypeMask) {
	            case kSKIPRecordType:
	                AddString("Skip record\n");
	                break;
	            case kNewRunRecordType:
	                AddString("New run record\n");
	                len += sprintf(buff+len,"  Data version:   %d\n", (*ncd & kNewRunVersion) >> 16);
	                len += sprintf(buff+len, "  Run number:     %ld\n", (long)ncd[1]);
	                len += sprintf(buff+len, "  Mac time:       %ld\n", (long)ncd[2]);
	                ncd += 2;   // 3 words long
	                break;
	            case kTrigTimeRecordType:
	                AddString("Trig time record\n");
	                len += sprintf(buff+len, "  Latch register: %d\n", (*ncd & kClockLatch) >> 24);
	                len += sprintf(buff+len, "  Clock:          %.0f\n", (double) 4294967296.0 * (*ncd & kClockUpper) + ncd[1]);
	                ++ncd;      // 2 words long
	                break;
	            case kGTIDRecordType:
	                AddString("GTID record\n");
	                len += sprintf(buff+len, "  Synclear err:   %d\n", (*ncd & kGTIDSyncClear) >> 26);
	                if (data->hex_id) {
	                    len += sprintf(buff+len, "  GTID:           0x%x\n", (*ncd & kGTIDGtid));
	                } else {
	                    len += sprintf(buff+len, "  GTID:           %ld\n", (long)(*ncd & kGTIDGtid));
	                }
	                break;
	            case kMuxRecordType:
	                AddString("Mux record\n");
                    card = (*ncd & kMuxBox) >> 23;
                    mask = (*ncd & kMuxHitPattern);
	                len += sprintf(buff+len, "  Mux bus:        %d\n", card);
	                len += sprintf(buff+len, "  Scope 0 fired:  %d\n", (*ncd & kMuxScope0) >> 20);
	                len += sprintf(buff+len, "  Scope 1 fired:  %d\n", (*ncd & kMuxScope1) >> 19);
	                len += sprintf(buff+len, "  Hit pattern:    0x%x\n", mask);
	                len += sprintf(buff+len, "  String names:   ");
                    if (card < kNumMuxBuses) {
                        int count = 0;
                        for (chan=0; chan<kNumMuxChannels; ++chan) {
                            if (mask & (1 << chan)) {
                                int n = data->ncdMuxLookup[card][chan];
                                pt = data->ncdMap[n].label;
                                if (!pt[0]) pt = "?";
                                len += sprintf(buff+len, "%s%s", (count ? "," : ""), pt);
                                ++count;
                            }
                        }
                        len += sprintf(buff+len, "\n");
                    } else {
                        len += sprintf(buff+len, "(bad bus number)\n");
                    }
	                break;
	            case kScopeRecordType:
	                AddString("Scope data record\n");
	                len += sprintf(buff+len, "  Scope ID:       %d\n",(*ncd & kScopeId) >> 23);
	                len += sprintf(buff+len, "  Channel:        %d\n",(*ncd & kScopeChannel) >> 19);
	                len += sprintf(buff+len, "  Size:           %d\n",*ncd & kScopeSize);
	                ncd += (*ncd & kScopeSize) / sizeof(u_int32);   // scope data
	                break;
	            case kScopeTimeRecordType:
	                AddString("Scope time record\n");
	                len += sprintf(buff+len, "  Channel:        %d\n", (*ncd & kClockLatch) >> 24);
	                len += sprintf(buff+len, "  Clock:          %.0f\n", (double) 4294967296.0 * (*ncd & kClockUpper) + ncd[1]);
	                ++ncd;      // 2 words long
	                break;
// not used
//	            case kShGScalRecordType:
//	                AddString("Shaper card global scaler record\n");
//	                len += sprintf(buff+len, "  Board number:   %d\n", (ncd[0] & kGlobalShaperBoard));
//	                len += sprintf(buff+len, "  Scalar value:   %d\n", (ncd[1] & kGlobalShaperScaler));
//	                ++ncd;      // 2 words long
//	                break;
	            case kMuxGlobalStatusType:
	                AddString("Mux Global Status\n");
	                len += sprintf(buff+len, "  Scope 0 fired:  %d\n", (*ncd & kMuxAScope) >> 11);
	                len += sprintf(buff+len, "  Scope 1 fired:  %d\n", (*ncd & kMuxBScope) >> 10);
	                len += sprintf(buff+len, "  Mux mask:       0x%x\n", (*ncd & kMuxFired));
	                break;
	            case kShaperRecordType:
	                AddString("Shaper record\n");
                    card = (*ncd & kShaperCard) >> 16;
                    chan = (*ncd & kShaperChan) >> 12;
	                len += sprintf(buff+len, "  Shaper slot:    %d\n", card);
	                len += sprintf(buff+len, "  Shaper channel: %d\n", chan);
	                len += sprintf(buff+len, "  Shaper value:   %d\n", (*ncd & kShaperValue));
	                pt = NULL;
                    if (card < kNumShaperSlots && chan < kNumShaperChannels) {
                        int n = data->ncdShaperLookup[card][chan];
                        pt = data->ncdMap[n].label;
                        if (pt[0]) {
	                        len += sprintf(buff+len, "  Shaper HW addr: 0x%x\n", data->ncdMap[n].shaper_addr);
	                    } else {
                            pt = "(unknown)";
                        }
                    } else {
                        pt = "(bad card/channel)";
                    }
                    len += sprintf(buff+len, "  String name:    %s\n", pt);
	                break;
	            case kScopeGTIDType:
	                AddString("Scope GTID record\n");
	                break;
	            case kHVType:
	                AddString("HV record\n");
	                break;
                case kGeneralDataType: {
                    AddString("General record\n");
                    int words = ((*ncd & 0xffff) + sizeof(u_int32) - 1) / sizeof(u_int32);
                    len += sprintf(buff+len, "  Length:         %d bytes\n", (int)(words * sizeof(u_int32)));
                    u_int32 *rec_end = ncd + words;
                    while (ncd<rec_end-1) {
                        ++ncd;  // step to start of next sub-record
                        int subwords = ((*ncd & 0xffff) + sizeof(u_int32) - 1) / sizeof(u_int32);
                        unsigned subtype = (*ncd >> 16);
                        char *typestr;
                        if (subtype < kNumGenericSubRecords) {
                            typestr = sGenericSubRecord[subtype];
                        } else {
                            typestr = "Unknown";
                        }
                        len += sprintf(buff+len, "  Subrecord type: %s (%d)\n", typestr, subtype);
                        switch (subtype) {
                            case kNCDModelPulserSettingsRecord:
                                if (data->hex_id) {
                                    len += sprintf(buff+len, "    GTID:         0x%lx\n", (long)ncd[1]);
                                } else {
                                    len += sprintf(buff+len, "    GTID:         %ld\n", (long)ncd[1]);
                                }
                                len += sprintf(buff+len, "    Waveform:     %ld\n", (long)ncd[2]);
                                len += sprintf(buff+len, "    Amplitude:    %g\n", *(float *)(ncd+3));
                                len += sprintf(buff+len, "    Burst rate:   %g\n", *(float *)(ncd+4));
                                len += sprintf(buff+len, "    Width:        %g\n", *(float *)(ncd+5));
                                break;
                            case kNCDModelLogAmpTask:
                            case kNCDModelLinearityTask:
                            case kNCDModelThresholdTask:
                            case kNCDModelStepPDSTask:
                                if (data->hex_id) {
                                    len += sprintf(buff+len, "    GTID:         0x%lx\n", (long)ncd[1]);
                                } else {
                                    len += sprintf(buff+len, "    GTID:         %ld\n", (long)ncd[1]);
                                }
                                len += sprintf(buff+len, "    Status:       %s\n",
                                               (ncd[2] & 0x01) ? "Started" : "Stopped");
                                break;
                            case kORLiveTimeScalers:
                                if (data->hex_id) {
                                    len += sprintf(buff+len, "    GTID:         0x%lx\n", (long)ncd[1]);
                                } else {
                                    len += sprintf(buff+len, "    GTID:         %ld\n", (long)ncd[1]);
                                }
                                len += sprintf(buff+len, "    Status:       %s\n", 
                                               sLiveTimeScalerStatus[(ncd[2]>>16) & 0x03]);
                                len += sprintf(buff+len, "    Crate:        %d\n", (int)((ncd[2]>>8) & 0x0f));
                                len += sprintf(buff+len, "    Slot:         %d\n", (int)(ncd[2] & 0x1f));
                                len += sprintf(buff+len, "    Total live:   %.0lf\n", 
                                               (double)4294967296.0 * (ncd[3] & 0xff) + ncd[4]);
                                len += sprintf(buff+len, "    MUX live:     %.0lf\n", 
                                               (double)4294967296.0 * ((ncd[3] >> 8) & 0xff) + ncd[5]);
                                len += sprintf(buff+len, "    Shaper live:  %.0lf\n", 
                                               (double)4294967296.0 * ((ncd[3] >> 16) & 0xff) + ncd[6]);
                                len += sprintf(buff+len, "    Scope live:   %.0lf\n", 
                                               (double)4294967296.0 * ((ncd[3] >> 24) & 0xff) + ncd[7]);
                                break;
                            case kNCDScopeCal:
                                len += sprintf(buff+len, "    Scope:        %d\n", (int)((ncd[1]>>4) & 0x0f));
                                len += sprintf(buff+len, "    Channel:      %d\n", (int)(ncd[1] & 0x0f));
                                len += sprintf(buff+len, "    LogAmp A:     %g\n", *(float *)(ncd + 2));
                                len += sprintf(buff+len, "    LogAmp B:     %g\n", *(float *)(ncd + 3));
                                len += sprintf(buff+len, "    LogAmp C:     %g\n", *(float *)(ncd + 4));
                                len += sprintf(buff+len, "    Preamp RC:    %g\n", *(float *)(ncd + 5));
                                len += sprintf(buff+len, "    Delay Time:   %g\n", *(float *)(ncd + 6));
                                break;
                            case kORShaperModelScalers:
                            case kORHPPulserModel:
                            default:
                                len += sprintf(buff+len, "    Length:       %d bytes\n",
                                               (int)(subwords * sizeof(u_int32)));
                                if (subwords <= 10) {
                                    for (int i=1; i<subwords; ++i) {
                                        len += sprintf(buff+len,"    Word %d:       0x%.8lx\n",i, (long)ncd[i]);
                                    }
                                }
                                break;
                        }
                        ncd += subwords - 1;    // step to just before next sub-record
                    }
                }   break;
	            default:
	                sprintf(buff,"Unknown record 0x%x\n", *ncd & kNcdDataTypeMask);
	                AddString(buff);
	                ++unknown_count;
	                break;
	        }
	        if (len) {
	            AddString(buff, 0);
	        }
	        if (unknown_count > kMaxUnknownRecords) {
	            AddString("Too many unknown records!\n");
	            AddString("-- Printout Truncated --\n");
	            break;
	        }
	    }
	} else if (mDataFlag != kNoData) {
        mDataFlag = kNoData;
		ClearString();
	    AddString("[no NCD data]");
	}
	// update the text
	Arg warg;
	XtSetArg(warg, XmNlabelString, mString);
	XtSetValues(mNCDLabel.GetWidget(), &warg, 1);
	
	// patch - resize the main pane to get the label to recalculate its size
	static int size = 3010;
	size ^= 0x01;
	XtSetArg(warg, XmNheight, size);
	XtSetValues(GetMainPane(), &warg, 1);
}


