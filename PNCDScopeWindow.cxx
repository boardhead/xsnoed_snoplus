//
// File:		PNCDScopeWindow.cxx
//
// Description:	Window to display NCD scope information
//
// Revisions:	08/26/03 - PH Created
//
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <math.h>
#include "PNCDScopeWindow.h"
#include "PNCDScopeImage.h"
#include "ImageData.h"
#include "PSpeaker.h"
#include "PZdabFile.h"
#include "include/NcdDataTypes.h"
#include "CUtils.h"

const int   kDirtyEvent          = 0x02;
const int   kDirtyAll            = 0x04;
const float kBadCalibrationValue = -9999.;
const int   kAllScopeChannelsOn  = 0x0f;    // mask for all scope channels on

static MenuStruct channels_menu[] = {
	{ "All Channels",0,XK_A,IDM_CHANNEL_ALL,  NULL, 0, MENU_RADIO | MENU_TOGGLE_ON },
	{ "Channel 0",	 0, XK_0,IDM_CHANNEL_0,    NULL, 0, MENU_RADIO },
	{ "Channel 1",	 0, XK_1,IDM_CHANNEL_1,    NULL, 0, MENU_RADIO },
	{ "Channel 2",	 0, XK_2,IDM_CHANNEL_2,    NULL, 0, MENU_RADIO },
	{ "Channel 3",	 0, XK_3,IDM_CHANNEL_3,    NULL, 0, MENU_RADIO },
	{ "Channels with Data",0, XK_D,IDM_CHANNEL_AUTO, NULL, 0, MENU_RADIO },
};

#ifdef ROOT_FILE
static MenuStruct scope_data_menu[] = {
	{ "Raw",	    0, XK_R,IDM_LOGAMP_RAW,   NULL, 0, MENU_RADIO | MENU_TOGGLE_ON },
	{ "AntiLog",	0, XK_A,IDM_LOGAMP_CAL,   NULL, 0, MENU_RADIO },
};
#endif

static MenuStruct scope_main_menu[] = {
	{ "Display",	0, 0,	0, channels_menu, XtNumber(channels_menu), 0 },
#ifdef ROOT_FILE
	{ "Data",		0, 0,	0, scope_data_menu, XtNumber(scope_data_menu), 0 },
#endif
};


//---------------------------------------------------------------------------
// PNCDScopeWindow constructor
//
PNCDScopeWindow::PNCDScopeWindow(ImageData *data)
				: PImageWindow(data)
{
	int		n;
	Arg		wargs[20];

    mIsCalibrated = IDM_LOGAMP_RAW;
    mChannelDisp  = IDM_CHANNEL_ALL;
    
	data->mSpeaker->AddListener(this);
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "NCD Scope"); ++n;
	XtSetArg(wargs[n], XmNx, 400);             ++n;
	XtSetArg(wargs[n], XmNy, 300);             ++n;
	XtSetArg(wargs[n], XmNminWidth, 100);      ++n;
	XtSetArg(wargs[n], XmNminHeight, 100);     ++n;
	SetShell(CreateShell("ncdScopePop",data->toplevel,wargs,n));
	
	n = 0;
	XtSetArg(wargs[n], XmNwidth, 400);         ++n;
	XtSetArg(wargs[n], XmNheight, 600);        ++n;
    XtSetArg(wargs[n], XmNbackground, data->colour[BKG_COL]); ++n;
	SetMainPane(XtCreateManagedWidget("scopeForm", xmFormWidgetClass,GetShell(),wargs,n));

    n = 0;
    XtSetArg(wargs[n],XmNmarginHeight,		1); ++n;
    XtSetArg(wargs[n],XmNleftAttachment,	XmATTACH_FORM); ++n;
    XtSetArg(wargs[n],XmNtopAttachment,		XmATTACH_FORM); ++n;
    XtSetArg(wargs[n],XmNrightAttachment,	XmATTACH_FORM); ++n;
    Widget menu = XmCreateMenuBar(GetMainPane(), "xsnoedMenu" , wargs, n);
    XtManageChild(menu);
    CreateMenu(menu, scope_main_menu, XtNumber(scope_main_menu), this);

	n = 0;
    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_WIDGET);    ++n;
    XtSetArg(wargs[n], XmNtopWidget,        menu);               ++n;
    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNleftAttachment,   XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNrightAttachment,  XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNbackground,       data->colour[BKG_COL]); ++n;
    Widget form = XtCreateManagedWidget("scopeForm", xmFormWidgetClass,GetMainPane(),wargs,n);
    
    for (int chan=0; chan<kNumScopeChannels; ++chan) {
        char buff[128];
        sprintf(buff, "chan%d", chan);
        n = 0;
        XtSetArg(wargs[n], XmNbackground,           data->colour[BKG_COL]); ++n;
        XtSetArg(wargs[n], XmNleftAttachment, 	    XmATTACH_FORM);      ++n;
        XtSetArg(wargs[n], XmNrightAttachment, 	    XmATTACH_FORM);      ++n;
        mChannel[chan] = XtCreateManagedWidget(buff, xmDrawingAreaWidgetClass, form, wargs, n);

        // create histogram
        mHist[chan] = new PNCDScopeImage(this, mChannel[chan], 0);
        sprintf(buff, "Channel %d", chan);
        mHist[chan]->SetLabel(buff);
        mHist[chan]->AllowLabel(FALSE);
        mHist[chan]->SetScaleLimits(0,15000,10);
        mHist[chan]->SetScaleMin(data->ncd_scope_xmin);
        mHist[chan]->SetScaleMax(data->ncd_scope_xmax);
        mHist[chan]->SetYMin(data->ncd_scope_ymin);
        mHist[chan]->SetYMax(data->ncd_scope_ymax);
        mHist[chan]->SetStyle(kHistStyleSteps);
        mHist[chan]->SetFixedBins();
        mHist[chan]->SetPlotCol(SCOPE0_COL);
        mHist[chan]->SetOverlayCol(SCOPE1_COL);
    }
    // get mask of channels we want to display (and be sure invalid bits aren't set)
    int chan_mask = (data->ncd_scope_mask & kAllScopeChannelsOn);
    // display all channels by default if none set
    if (!chan_mask) chan_mask = kAllScopeChannelsOn;
    // set current channel mask to an invalid entry (with all channels on)
    // to force our channels to be arranged initially
    data->ncd_scope_mask = -1;
    // arrange our channel histogram widgets in the window
    SetChannels(chan_mask);
        
    // select our current menu items
    if (data->ncd_scope_chan) SelectMenuItem(IDM_CHANNEL_ALL + data->ncd_scope_chan);
#ifdef ROOT_FILE
    if (data->ncd_scope_cal)  SelectMenuItem(IDM_LOGAMP_RAW + data->ncd_scope_cal);
#endif

    SetDirty(kDirtyAll);
}

PNCDScopeWindow::~PNCDScopeWindow()
{
    for (int i=0; i<kNumScopeChannels; ++i) {
        delete mHist[i];
        mHist[i] = NULL;
    }
    SetImage(NULL);     // make sure base class doesn't re-delete our image
}

void PNCDScopeWindow::Listen(int message, void *message_data)
{
	switch (message) {
		case kMessageNewEvent:
		case kMessageEventCleared:
			SetDirty(kDirtyEvent);
			break;
		case kMessageColoursChanged:
			SetDirty(kDirtyAll);
			break;
	}
}

void PNCDScopeWindow::DoMenuCommand(int anID)
{
	switch (anID) {
		case IDM_LOGAMP_RAW:
		case IDM_LOGAMP_CAL:
		    PMenu::UpdateTogglePair(&mIsCalibrated);
		    GetData()->ncd_scope_cal = mIsCalibrated - IDM_LOGAMP_RAW;
		    SetDirty(kDirtyAll);
		    break;
		case IDM_CHANNEL_ALL:
		case IDM_CHANNEL_0:
		case IDM_CHANNEL_1:
		case IDM_CHANNEL_2:
		case IDM_CHANNEL_3:
		case IDM_CHANNEL_AUTO:
		    PMenu::UpdateTogglePair(&mChannelDisp);
		    GetData()->ncd_scope_chan = mChannelDisp - IDM_CHANNEL_ALL;
		    SetChannels(GetSelectedChannels());
		    SetDirty(kDirtyAll);
		    break;
    }
}

void PNCDScopeWindow::DoneGrab(PNCDScopeImage *hist)
{
    ImageData *data = GetData();

    if (mChannelDisp == IDM_CHANNEL_AUTO) {
        // set our channels once our grab is done
		SetChannels(GetSelectedChannels());
    }
    // update current histogram scales
    data->ncd_scope_xmin = hist->GetScaleMin();
    data->ncd_scope_xmax = hist->GetScaleMax();
    data->ncd_scope_ymin = hist->GetYMin();
    data->ncd_scope_ymax = hist->GetYMax();
    
    // set scales of other channels
    for (int chan=0; chan<kNumScopeChannels; ++chan) {
        // adjust scales of all histograms if any one of them changes
        if (mHist[chan] != hist &&
           (mHist[chan]->GetScaleMin() != hist->GetScaleMin() ||
            mHist[chan]->GetScaleMax() != hist->GetScaleMax() ||
            mHist[chan]->GetYMin() != hist->GetYMin() ||
            mHist[chan]->GetYMax() != hist->GetYMax()))
        {
            mHist[chan]->SetScaleMin(hist->GetScaleMin());
            mHist[chan]->SetScaleMax(hist->GetScaleMax());
            mHist[chan]->SetYMin(hist->GetYMin());
            mHist[chan]->SetYMax(hist->GetYMax());
            mHist[chan]->SetDirty();
        }
    }
}

// get mask for selected scope channels
int PNCDScopeWindow::GetSelectedChannels()
{
    int chan_mask = 0;

    // set our channel(s) to be displayed
    if (mChannelDisp == IDM_CHANNEL_AUTO) {
        for (int chan=0; chan<kNumScopeChannels; ++chan) {
            // get bitmask of channels with data
            if (mHist[chan]->GetDataPt() || mHist[chan]->GetOverlayPt()) {
                chan_mask |= (1 << chan);
            }
        }
    } else if (mChannelDisp == IDM_CHANNEL_ALL) {
        chan_mask = kAllScopeChannelsOn;
    } else {    // IDM_CHANNEL_0, etc...
        chan_mask = (1 << (mChannelDisp - IDM_CHANNEL_0));
    }
    return(chan_mask);
}

// set our displayed channels
void PNCDScopeWindow::SetChannels(int chan_mask)
{
    int       n, chan;
	Arg		  wargs[20];
	ImageData *data = GetData();

    if (chan_mask && chan_mask != data->ncd_scope_mask) {
        // count the number of channels displayed
        int chan_count = 0;
        for (chan=0; chan<kNumScopeChannels; ++chan) {
            if (chan_mask & (1 << chan)) {
                ++chan_count;
            }
        }
        // make necessary attachments and remap displayed channels
        int height = 100 / chan_count;
        int count = 0;
        for (chan=0; chan<kNumScopeChannels; ++chan) {
            if (chan_mask & (1 << chan)) {
                n = 0;
                if (count) {
                    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_POSITION);      ++n;
                    XtSetArg(wargs[n], XmNtopPosition, 	    count * height);         ++n;
                } else {
                    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_WIDGET);        ++n;
                    XtSetArg(wargs[n], XmNtopWidget,        GetMenu()->GetWidget()); ++n;
                }
                if (count < chan_count-1) {
                    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_POSITION);      ++n;
                    XtSetArg(wargs[n], XmNbottomPosition, 	(count + 1) * height);   ++n;
                } else {
                    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);          ++n;
                }
                XtSetValues(mChannel[chan], wargs, n);
                ++count;
            }
        }
        // map necessary channels
        for (chan=0; chan<kNumScopeChannels; ++chan) {
            // only map/unmap channels that have changed
            if ((chan_mask ^ data->ncd_scope_mask) & (1 << chan)) {
                // must be careful not to map a widget before it is realized
                if (XtIsRealized(mChannel[chan])) {
                    if (chan_mask & (1 << chan)) {
                        // show this channel
                        XtSetMappedWhenManaged(mChannel[chan], True);
                    } else {
                        // hide this channel
                        XtSetMappedWhenManaged(mChannel[chan], False);
                    }
                } else {
                    n = 0;
                    if (chan_mask & (1 << chan)) {
                        XtSetArg(wargs[n], XmNmappedWhenManaged, TRUE); ++n;
                    } else {
                        XtSetArg(wargs[n], XmNmappedWhenManaged, FALSE); ++n;
                    }
                    XtSetValues(mChannel[chan], wargs, n);
                }
            }
        }
        // set our canvas to the first displayed channel (for Print Image...)
        for (chan=0; chan<kNumScopeChannels; ++chan) {
            if (chan_mask & (1 << chan)) {
                SetImage(mHist[chan]);
                break;
            }
        }
        data->ncd_scope_mask = chan_mask;
    }
}

int PNCDScopeWindow::IsGoodLogAmpParms(int n)
{
    if (n >= kMaxNcdScopeData) return(0);
    
    ImageData   *data = GetData();
    float       *parms = data->ncd_log_amp[n];

    for (int i=0; i<kNumLogAmpParms; ++i) {
        if (parms[i] == 0 || parms[i] == kBadCalibrationValue) return(0);
    }
    return(1);
}

// UpdateSelf
void PNCDScopeWindow::UpdateSelf()
{
	ImageData	*data = GetData();

#ifdef PRINT_DRAWS
	Printf("-updateNCDScope\n");
#endif

    int chan;
    
    // load new event if necessary
    if (IsDirty() & (kDirtyEvent | kDirtyAll)) {
        // delete old data if it exists
        for (chan=0; chan<kNumScopeChannels; ++chan) {
            if (mHist[chan]->GetDataPt() || mHist[chan]->GetOverlayPt()) {
                mHist[chan]->CreateData(0);
                mHist[chan]->SetDirty();
            } else if (IsDirty() & kDirtyAll) {
                mHist[chan]->SetDirty();
            }
        }
        if (data->ncd_scope_size) {
            long data_size = data->ncd_scope_size;
            for (int scope=0; scope<kNumNcdScopes; ++scope) {
                for (chan=0; chan<kNumScopeChannels; ++chan) {
                    int n = chan + scope * kNumScopeChannels;
                    if (!data->ncd_scope_data[n]) continue;
                    long *pt;
                    if (scope) {
                        mHist[chan]->CreateOverlay(data_size);
                        pt = mHist[chan]->GetOverlayPt();
                    } else {
                        mHist[chan]->CreateData(data_size);
                        pt = mHist[chan]->GetDataPt();
                    }
                    if (pt) {
                        PackedCharArray scopeData(data->ncd_scope_data[n]);
                        if (mIsCalibrated == IDM_LOGAMP_CAL && IsGoodLogAmpParms(n)) {
                            mHist[chan]->SetCalibrated(scope, 1);
                            float *parms = data->ncd_log_amp[n];
                            for (int i=0; i<data_size; ++i) {
                                // antilog and multiply by 1e3 to bring to a reasonable integer scale
                                pt[i] = (long)(1e3 * parms[1] * (pow(10.0, (double)((scopeData.Get(i) - parms[2]) / parms[0])) - 1.0));
                            }
                        } else {
                            mHist[chan]->SetCalibrated(scope, 0);   // not displaying antilogged data
                            for (int i=0; i<data_size; ++i) {
                                pt[i] = scopeData.Get(i);
                            }
                        }
                        mHist[chan]->SetScaleLimits(0,data_size,10);
                        mHist[chan]->SetDirty();
                    }
                }
            }
        }
        // set channels if auto and no grabs are active
        if (mChannelDisp == IDM_CHANNEL_AUTO) {
            for (chan=0; chan<kNumScopeChannels; ++chan) {
                if (mHist[chan]->GetGrabFlag() & GRABS_ACTIVE) break;
            }
            if (chan == kNumScopeChannels) SetChannels(GetSelectedChannels());
        }
    }
    // Update necessary histograms
    for (chan=0; chan<kNumScopeChannels; ++chan) {
        // update this channel if required and visible
        if (mHist[chan]->IsDirty() && (data->ncd_scope_mask & (1<<chan))) {
            mHist[chan]->Draw();
        }
    }
}

