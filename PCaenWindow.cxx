//
// File:		PCaenWindow.cxx
//
// Description:	Window to display CAEN digitized trigger sums
//
// Revisions:	2012/03/06 - PH Created (borrowed heavily from PNCDScopeWindow.cxx)
//
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <math.h>
#include "PCaenWindow.h"
#include "PNCDScopeImage.h"
#include "ImageData.h"
#include "PSpeaker.h"
#include "PZdabFile.h"
#include "include/NcdDataTypes.h"
#include "CUtils.h"

const int   kDirtyEvent       = 0x02;
const int   kDirtyAll         = 0x04;
const int   kAllCaenChannels  = 0xff;    // mask for all scope channels on

static MenuStruct channels_menu[] = {
	{ "All Channels",0,XK_A,IDM_CAEN_ALL,  NULL, 0, 0 },
	{ "Channels with Data",0, XK_D,IDM_CAEN_AUTO, NULL, 0, 0 },
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Channel 0",	 0, XK_0,IDM_CAEN_0,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 1",	 0, XK_1,IDM_CAEN_1,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 2",	 0, XK_2,IDM_CAEN_2,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 3",	 0, XK_3,IDM_CAEN_3,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 4",	 0, XK_4,IDM_CAEN_4,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 5",	 0, XK_5,IDM_CAEN_5,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 6",	 0, XK_6,IDM_CAEN_6,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Channel 7",	 0, XK_7,IDM_CAEN_7,    NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
};

static MenuStruct caen_main_menu[] = {
	{ "Display",	0, 0,	0, channels_menu, XtNumber(channels_menu), 0 },
};


//---------------------------------------------------------------------------
// PCaenWindow constructor
//
PCaenWindow::PCaenWindow(ImageData *data)
				: PImageWindow(data)
{
	int		n;
	Arg		wargs[20];

	data->mSpeaker->AddListener(this);
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "CAEN Scope"); ++n;
	XtSetArg(wargs[n], XmNx, 400);             ++n;
	XtSetArg(wargs[n], XmNy, 300);             ++n;
	XtSetArg(wargs[n], XmNminWidth, 100);      ++n;
	XtSetArg(wargs[n], XmNminHeight, 100);     ++n;
	SetShell(CreateShell("caenPop",data->toplevel,wargs,n));
	
	n = 0;
	XtSetArg(wargs[n], XmNwidth, 400);         ++n;
	XtSetArg(wargs[n], XmNheight, 600);        ++n;
    XtSetArg(wargs[n], XmNbackground, data->colour[BKG_COL]); ++n;
	SetMainPane(XtCreateManagedWidget("caenMain", xmFormWidgetClass,GetShell(),wargs,n));

    n = 0;
    XtSetArg(wargs[n],XmNmarginHeight,		1); ++n;
    XtSetArg(wargs[n],XmNleftAttachment,	XmATTACH_FORM); ++n;
    XtSetArg(wargs[n],XmNtopAttachment,		XmATTACH_FORM); ++n;
    XtSetArg(wargs[n],XmNrightAttachment,	XmATTACH_FORM); ++n;
    Widget menu = XmCreateMenuBar(GetMainPane(), "xsnoedMenu" , wargs, n);
    XtManageChild(menu);
    CreateMenu(menu, caen_main_menu, XtNumber(caen_main_menu), this);

    // add resource labels to CAEN channels in menu
    for (int i=0; i<kCaenRsrcNum; ++i) {
        if (!strncmp(data->caen_lbl[i], "Channel ", 8)) continue;
        char buff[256];
        sprintf(buff, "Channel %d - %s", i, data->caen_lbl[i]);
        GetMenu()->SetLabel(IDM_CAEN_0+i, buff);
    }

	n = 0;
    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_WIDGET);    ++n;
    XtSetArg(wargs[n], XmNtopWidget,        menu);               ++n;
    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNleftAttachment,   XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNrightAttachment,  XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNbackground,       data->colour[BKG_COL]); ++n;
    Widget form = XtCreateManagedWidget("caenForm", xmFormWidgetClass,GetMainPane(),wargs,n);
    
    // get mask of channels we want to display (and be sure invalid bits aren't set)
    int chan_mask = (data->caen_mask & kAllCaenChannels);
    // display all channels by default if none set
    if (!chan_mask) chan_mask = kAllCaenChannels;

    for (int chan=0; chan<kMaxCaenChannels; ++chan) {
        char buff[128];
        sprintf(buff, "chan%d", chan);
        n = 0;
        XtSetArg(wargs[n], XmNbackground,           data->colour[BKG_COL]); ++n;
        XtSetArg(wargs[n], XmNleftAttachment, 	    XmATTACH_FORM);      ++n;
        XtSetArg(wargs[n], XmNrightAttachment, 	    XmATTACH_FORM);      ++n;
        mChannel[chan] = XtCreateManagedWidget(buff, xmDrawingAreaWidgetClass, form, wargs, n);

        // create histogram
        mHist[chan] = new PNCDScopeImage(this, mChannel[chan], 0);
        mHist[chan]->AllowLabel(FALSE);
        mHist[chan]->SetScaleLimits(0,15000,10);
        mHist[chan]->SetScaleMin(0);
        mHist[chan]->SetScaleMax(1000);
        mHist[chan]->SetCalcObj(this);
        if (chan < kCaenRsrcNum) {
            mHist[chan]->SetLabel(data->caen_lbl[chan]);
            mHist[chan]->SetYMin(data->caen_min[chan]);
            mHist[chan]->SetYMax(data->caen_max[chan]);
        } else {
            sprintf(buff, "Channel %d", chan);
            mHist[chan]->SetLabel(buff);
            mHist[chan]->SetYMin(0);
            mHist[chan]->SetYMax(4096);
        }
        mHist[chan]->SetStyle(kHistStyleSteps);
        mHist[chan]->SetFixedBins();
        mHist[chan]->SetPlotCol(SCOPE1_COL);
        mHist[chan]->SetOverlayCol(SCOPE0_COL);
    }
    // set current channel mask to an invalid entry (with all channels on)
    // to force our channels to be arranged initially
    data->caen_mask = -1;
    // arrange our channel histogram widgets in the window
    SetChannels(chan_mask);
        
    SetDirty(kDirtyAll);
}

PCaenWindow::~PCaenWindow()
{
    for (int i=0; i<kMaxCaenChannels; ++i) {
        delete mHist[i];
        mHist[i] = NULL;
    }
    SetImage(NULL);     // make sure base class doesn't re-delete our image
}

void PCaenWindow::Listen(int message, void *message_data)
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

void PCaenWindow::DoMenuCommand(int anID)
{
	switch (anID) {
		case IDM_CAEN_ALL:
		    SetChannels(kAllCaenChannels);
		    SetDirty(kDirtyAll);
		    break;
		case IDM_CAEN_AUTO:
		    SetChannels(GetActiveChannels());
		    SetDirty(kDirtyAll);
		    break;
		case IDM_CAEN_0:
		case IDM_CAEN_1:
		case IDM_CAEN_2:
		case IDM_CAEN_3:
		case IDM_CAEN_4:
		case IDM_CAEN_5:
		case IDM_CAEN_6:
		case IDM_CAEN_7:
		    SetChannels(GetData()->caen_mask ^ (1 << (anID - IDM_CAEN_0)));
		    SetDirty(kDirtyAll);
		    break;
    }
}

void PCaenWindow::DoneGrab(PNCDScopeImage *)
{
    // update current Y scale settings
    ImageData *data = GetData();
    for (int chan=0; chan<kMaxCaenChannels && chan<kCaenRsrcNum; ++chan) {
        data->caen_min[chan] = mHist[chan]->GetYMin();
        data->caen_max[chan] = mHist[chan]->GetYMax();
    }
}

// get mask for active scope channels
int PCaenWindow::GetActiveChannels()
{
    int chan_mask = 0;

    // set our channel(s) to be displayed
    for (int chan=0; chan<kMaxCaenChannels; ++chan) {
        // get bitmask of channels with data
        if (mHist[chan]->GetDataPt() || mHist[chan]->GetOverlayPt()) {
            chan_mask |= (1 << chan);
        }
    }
    return(chan_mask);
}

// set our displayed channels
void PCaenWindow::SetChannels(int chan_mask)
{
    int       n, chan;
	Arg		  wargs[20];
	ImageData *data = GetData();

    if (chan_mask && chan_mask != data->caen_mask) {
        // count the number of channels displayed
        int chan_count = 0;
        for (chan=0; chan<kMaxCaenChannels; ++chan) {
            if (chan_mask & (1 << chan)) {
                ++chan_count;
            }
        }
        // make necessary attachments and remap displayed channels
        float height = 100.0 / chan_count;
        int count = 0;
        for (chan=0; chan<kMaxCaenChannels; ++chan) {
            // toggle on/off our channel displays
            GetMenu()->SetToggle(IDM_CAEN_0 + chan, chan_mask & (1 << chan));
            if (chan_mask & (1 << chan)) {
                n = 0;
                if (count) {
                    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_POSITION);      ++n;
                    XtSetArg(wargs[n], XmNtopPosition, 	    int(count*height+0.5));  ++n;
                } else {
                    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_WIDGET);        ++n;
                    XtSetArg(wargs[n], XmNtopWidget,        GetMenu()->GetWidget()); ++n;
                }
                if (count < chan_count-1) {
                    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_POSITION);      ++n;
                    XtSetArg(wargs[n], XmNbottomPosition, 	int((count+1)*height+0.5)); ++n;
                } else {
                    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);          ++n;
                }
                XtSetValues(mChannel[chan], wargs, n);
                ++count;
            }
        }
        // map necessary channels
        for (chan=0; chan<kMaxCaenChannels; ++chan) {
            // only map/unmap channels that have changed
            if ((chan_mask ^ data->caen_mask) & (1 << chan)) {
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
        for (chan=0; chan<kMaxCaenChannels; ++chan) {
            if (chan_mask & (1 << chan)) {
                SetImage(mHist[chan]);
                break;
            }
        }
        data->caen_mask = chan_mask;
    }
}

// calculate range for 2D histogram (return 1 if OK)
int PCaenWindow::GetRange(PHistImage *hist, int *min, int *max)
{
    for (int chan=0; chan<kMaxCaenChannels; ++chan) {
        if (hist != mHist[chan]) continue;
        ImageData	*data = GetData();
        u_int32 *src = data->sum_caen[chan];
        if (!src) continue;
        long caen_size = data->sum_caen_samples[chan];
        for (int i=0; i<caen_size; ++i) {
            u_int32 *pt = src + i * 4096;
            for (int j=0; j<4096; ++j) {
                if (pt[j]) {
                    if (*min > j) *min = j;
                    break;
                }
            }
            for (int j=4095; j>=0; --j) {
                if (pt[j]) {
                    if (*max < j) *max = j;
                    break;
                }
            }
        }
        return 1;
    }
    return 0;
}

// recalculate 2D histogram
void PCaenWindow::DoCalc(PHistImage *hist)
{
    for (int chan=0; chan<kMaxCaenChannels; ++chan) {
        if (hist != mHist[chan]) continue;
        ImageData	*data = GetData();
        u_int32 *src = data->sum_caen[chan];
        if (!src) continue;
        long caen_size = data->sum_caen_samples[chan];
        hist->CreateData(caen_size, 1);  // make 2D data
        unsigned long *dst = (unsigned long *)hist->GetDataPt();
        if (!dst) continue;
        long numPix = hist->GetNumPix();
        unsigned long max = 0;
        memset(dst, 0, numPix * caen_size * sizeof(long));
        for (int i=0; i<caen_size; ++i) {
            for (int j=0; j<4096; ++j) {
                u_int32 val = src[i * 4096 + j];
                if (!val) continue;
                int pix = hist->GetPix(j);
                if (pix <= 0) {
                    dst[i * numPix] += val;
                } else if (pix >= numPix) {
                    dst[i * numPix + numPix - 1] += val;
                } else {
                    unsigned long tmp = (dst[i * numPix + pix] += val);
                    if (max < tmp) max = tmp;
                }
            }
        }
        mMaxVal = max;
        hist->SetNumTraces(data->sum_caen_count[chan]);
        hist->SetScaleLimits(0,caen_size,10);
        break;
    }
}

// UpdateSelf
void PCaenWindow::UpdateSelf()
{
	ImageData	*data = GetData();

#ifdef PRINT_DRAWS
	Printf("-updateCaenWindow\n");
#endif

    int chan;
    
    // load new event if necessary
    if (IsDirty() & (kDirtyEvent | kDirtyAll)) {
        // delete old data if it exists
        for (chan=0; chan<kMaxCaenChannels; ++chan) {
            if (mHist[chan]->GetDataPt() || mHist[chan]->GetOverlayPt()) {
                mHist[chan]->CreateData(0);
                mHist[chan]->SetDirty();
            } else if (IsDirty() & kDirtyAll) {
                mHist[chan]->SetDirty();
            }
            mHist[chan]->SetStyle(data->sum ? kHistStyle2D : kHistStyleSteps);
        }
        long caen_size = data->caen_size;
        if (caen_size || data->sum) {
            for (chan=0; chan<kMaxCaenChannels; ++chan) {
                if (data->sum) caen_size = data->sum_caen_samples[chan];
                PNCDScopeImage *hist = mHist[chan];
                if (hist->GetScaleMax() > caen_size) {
                    hist->SetScaleMax(caen_size);
                }
                if (data->sum_caen[chan]) {
                    hist->SetDirty(kDirtyHistCalc);  // histogram requires calculating
                } else {
                    u_int16 *caen = data->caen_data[chan];
                    if (!caen) continue;
                    hist->CreateData(caen_size);
                    long *pt = hist->GetDataPt();
                    if (pt) {
                        hist->SetCalibrated(0, 0);   // not displaying antilogged data
                        for (int i=0; i<caen_size; ++i) {
                            pt[i] = caen[i];
                        }
                        hist->SetScaleLimits(0,caen_size,10);
                        hist->SetDirty();
                    }
                }
            }
        }
    }
    // Update necessary histograms
    for (chan=0; chan<kMaxCaenChannels; ++chan) {
        // update this channel if required and visible
        if (mHist[chan]->IsDirty() && (data->caen_mask & (1<<chan))) {
            mHist[chan]->Draw();
        }
    }
}

