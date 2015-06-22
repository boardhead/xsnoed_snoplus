#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>
#include <Xm/BulletinB.h>
#include <Xm/Form.h>
#include <stdio.h>
#include "QRchHist.h"
#include "PImageWindow.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "xsnoedstream.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"

char *QRchHist::sRchName[] = { "fRchTime", "fRchQhs", "fRchQhl", "fRchQlx" };
	

static MenuStruct rch_file_menu[] = {
	{ "Open Overlay...",	0, 0,	IDM_OPEN_OVERLAY_RCH, 	NULL, 0, 0 },
	{ "Close Overlay",		0, 0,	IDM_CLOSE_OVERLAY_RCH,  NULL, 0, 0 },
};
static MenuStruct rch_main_menu[] = {
	{ "File",			0, 0, 0, rch_file_menu, XtNumber(rch_file_menu), 0 },
};


QRchHist::QRchHist(PImageWindow *owner, int anID, Widget canvas)
		: PHistImage(owner,canvas,0)
{
	mRchIndex = anID - RCH_TIME_WINDOW;
	mStyle = kHistStyleSteps;
	mRchOverlayFile	= NULL;

	ImageData *data = owner->GetData();
	
	mXMinMin	= -10000;
	mXMaxMax	= 10000;
	mDataMax	= 0;
	mOverlayCol	= SCALE_COL3;
	
	switch (anID) {
		case RCH_TIME_WINDOW:
			mXMin = data->cal_tac_min;
			mXMax = data->cal_tac_max;
			mXMinMin = -1000;
			mXMaxMax = 1000;
			break;
		case RCH_QHS_WINDOW:
			mXMin = data->cal_qhs_min;
			mXMax = data->cal_qhs_max;
			break;
		case RCH_QHL_WINDOW:
			mXMin = data->cal_qhl_min;
			mXMax = data->cal_qhl_max;
			break;
		case RCH_QLX_WINDOW:
			mXMin = data->cal_qlx_min;
			mXMax = data->cal_qlx_max;
			break;
	}
	if (data->rch_pmtn >= 0) {
		ReadRchData();
	} else {
		SetLabel(sRchName[anID-RCH_TIME_WINDOW]+1);
	}
	if (!canvas) {
		owner->CreateMenu(NULL,rch_main_menu, XtNumber(rch_main_menu),this);
		CreateCanvas("rchCanvas");
	}
	data->mSpeaker->AddListener(this);
}

QRchHist::~QRchHist()
{
	ImageData *data = mOwner->GetData();
	
	if (data->rch_filebox && data->root_rch_hist==this) {
		XtUnmanageChild(data->rch_filebox);
	}
	delete [] mOverlay;
	mOverlay = NULL;
	delete mRchOverlayFile;
}

void QRchHist::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageCursorHit:
			SetDirty();
			break;
		default:
			PHistImage::Listen(message, dataPt);
			break;
	}
}

void QRchHist::DoGrab(float xmin, float xmax)
{
	mXMin = xmin;
	mXMax = xmax;
	CheckScaleRange();
	ReadRchData();
}


void QRchHist::MakeHistogram()
{
	mOverlayScale = 1.0;
	
	if (!(mGrabFlag & GRAB_Y)) {

		// calculate the overlay scaling
		if (mOverlay) {
			long *ovr = mOverlay;
			long *last = ovr + mOverlayBins;
			long datamax = 0;
			while (ovr < last) {
				if (*ovr > datamax) datamax = *ovr;
				++ovr;
			}
			if (datamax) {
				if (!mDataMax) {
					// scale to overlay if no main data
					mDataMax = datamax;
				} else {
					// scale overlay to same maximum if not grabbing Y
					mOverlayScale = mDataMax / (double)datamax;
				}
			}
		}
		// set the plot Y maximum
		long ymax = (long)(mDataMax * 1.2);
		if (ymax < 10) ymax = 10;
		SetYMax(ymax);
	}
}

void QRchHist::ReadRchData()
{
	int			i, firstbin, lastbin, nbins;
	long		datamax = 0;
	ImageData 	*data = mOwner->GetData();
	TH2F		*th2;
	TH1D		*th1;
	TAxis		*axis;
	
	int pmtn = data->rch_pmtn;
	
	if (pmtn < 0) return;
			
	int xmin = (int)GetScaleMin();
	int xmax = (int)GetScaleMax();

	if (data->root_rch_file) {
		th2 = (TH2F *)data->root_rch_file->Get(sRchName[mRchIndex]);
		if (th2) {
			th1 = th2->ProjectionX("th1", pmtn+1, pmtn+1);
			axis = th1->GetXaxis();
			
			// make sure we don't go outside our Rch histogram
			mXMinMin = axis->GetBinCenter(axis->FindBin(-10000));
			mXMaxMax = axis->GetBinCenter(axis->FindBin(10000));
			CheckScaleRange();
			
			firstbin = axis->FindBin(xmin);
			lastbin = axis->FindBin(xmax);
			nbins = lastbin - firstbin;
			
			CreateData(nbins);	// create the histogram data array
			
			long *dataPt = GetDataPt();
			
			if (dataPt) {
		
				for (i=firstbin; i<lastbin; ++i) {
					long n = (long)th1->GetBinContent(i);
					if (n > datamax) datamax = n;
					dataPt[i-firstbin] = n;
				}
				mDataMax = datamax;
			}
			delete th1;
		}
	} else {
		if (mHistogram) {
			delete [] mHistogram;
			mHistogram = NULL;
		}
		mDataMax = 0;
	}

	if (mRchOverlayFile) {
		th2 = (TH2F *)mRchOverlayFile->Get(sRchName[mRchIndex]);
		if (th2) {
			th1 = th2->ProjectionX("th1", pmtn+1, pmtn+1);
			axis = th1->GetXaxis();

			mXMinMin = axis->GetBinCenter(axis->FindBin(-10000));
			mXMaxMax = axis->GetBinCenter(axis->FindBin(10000));
			CheckScaleRange();
			
			firstbin = axis->FindBin(xmin);
			lastbin = axis->FindBin(xmax);
			nbins = lastbin - firstbin;

			if (!mOverlay || mOverlayBins!=nbins) {
				delete [] mOverlay;
				mOverlay = NULL;
				if (!mHistogram) {
					mNumBins = nbins;
				}
				if (mNumBins == nbins) {
					mOverlay = new long[nbins];
					mOverlayBins = nbins;
				}
			}
			if (mOverlay) {
				for (i=firstbin; i<lastbin; ++i) {
					mOverlay[i-firstbin] = (long)th1->GetBinContent(i);
				}
			}
			delete th1;
		} else {
			delete [] mOverlay;
			mOverlay = NULL;
		}
	}
	
	// set the histogram label
	char buff[128];
	int crate = pmtn / 512;
	int card = (pmtn - crate * 512) / 32;
	int channel = pmtn - crate * 512 - card * 32;
	sprintf(buff,"%d/%d/%d - %s",crate,card,channel,sRchName[mRchIndex]+1);
	SetLabel(buff);
}

void QRchHist::RchFileOK(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data)
{
	char			buff[512];
	
	XtUnmanageChild(data->rch_filebox);
	
	strncvtXm(buff, call_data->value, 512);
	TFile *theFile = new TFile(buff);
	if (!theFile) {
		Printf("Error opening Rch file: %s\n",buff);
		/* re-open file dialog because the file didn't open */
		XtManageChild(data->rch_filebox);
		return;
	}
	
	QRchHist *hist = data->root_rch_hist;
	if (hist) {
		Printf("Opened %s overlay: %s\n",sRchName[hist->mRchIndex]+1,buff);
		if (hist->mRchOverlayFile) delete hist->mRchOverlayFile;
		hist->mRchOverlayFile = theFile;
		hist->ReadRchData();
		hist->Draw();
	} else {
		Printf("Opened Rch file: %s\n",buff);
		if (data->owns_rch_file) {
			delete data->root_rch_file;
		}
		data->owns_rch_file = 1;		// we are responsible for deleting this file
		data->root_rch_file = theFile;
		// update all Rch windows
		int pmtn = data->rch_pmtn;
		data->rch_pmtn = -1;
		root_update_rch_windows(data, pmtn);
	}
}

void QRchHist::RchFileCancel(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data)
{
	XtUnmanageChild(data->rch_filebox);
}

void QRchHist::DestroyRchFileboxProc(Widget w, ImageData *data, caddr_t call_data)
{
	data->rch_filebox = NULL;
}

void QRchHist::OpenRchFile(ImageData *data, QRchHist *hist)
{
	int			n;
	char		buff[128];
	Arg			wargs[10];
	XmString	title, dir_mask;
	
	data->root_rch_hist = hist;	// save pointer to Rch hist that owns the file selection box
	
	if (data->rch_filebox && XtIsManaged(data->rch_filebox)) {
		XtUnmanageChild(data->rch_filebox);
	}
	if (hist) {
		sprintf(buff,"Open %s Overlay...",sRchName[hist->mRchIndex]+1);
	} else {
		strcpy(buff,"Open Rch File...");
	}
	title = XmStringCreateLtoR(buff, XmFONTLIST_DEFAULT_TAG);
	
	if (data->rch_filebox) {
		n = 0;
		XtSetArg(wargs[n], XmNfilterLabelString, title);	++n;
		XtSetValues(data->rch_selbox, wargs, n);
	} else {
		n = 0;
		XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS);	++n;
		XtSetArg(wargs[n], XmNtitle, "Open Rch File"); ++n;
		data->rch_filebox = XmCreateBulletinBoardDialog(data->toplevel, "xsnoedFileBox", wargs, n);
		
		dir_mask = XmStringCreateLtoR("*.rch", XmFONTLIST_DEFAULT_TAG);
		n = 0;
		XtSetArg(wargs[n], XmNdirMask, dir_mask);	++n;
		XtSetArg(wargs[n], XmNfilterLabelString, title);	++n;
		data->rch_selbox = XmCreateFileSelectionBox(data->rch_filebox, "selbox", wargs, n);
		
		XtUnmanageChild(XmFileSelectionBoxGetChild(data->rch_selbox,XmDIALOG_HELP_BUTTON));
		
		XtAddCallback(data->rch_selbox, XmNokCallback, (XtCallbackProc)RchFileOK, data);
		XtAddCallback(data->rch_selbox, XmNcancelCallback, (XtCallbackProc)RchFileCancel, data);
		XmStringFree(dir_mask);
		XtManageChild(data->rch_selbox);
		XtAddCallback(data->rch_filebox,XtNdestroyCallback,(XtCallbackProc)DestroyRchFileboxProc,data);
		
	}
	XmStringFree(title);
	
	XtManageChild(data->rch_filebox);
}

void QRchHist::CreateRchWindow(ImageData *data, char *name, int anID)
{
	int			n, dx;
	Arg			wargs[10];
	Widget		window, w;
	
	dx = (anID-RCH_TIME_WINDOW) * 20;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, name); ++n;
	XtSetArg(wargs[n], XmNx, 350+dx); ++n;
	XtSetArg(wargs[n], XmNy, 300+dx); ++n;
	XtSetArg(wargs[n], XmNminWidth, 200); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	window = PWindow::CreateShell(name,data->toplevel,wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNwidth, 400); ++n;
	XtSetArg(wargs[n], XmNheight, 250); ++n;
	w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);

	PImageWindow *pwin = new PImageWindow(data,window,w);
	
	(void)new QRchHist(pwin, anID);
	
	data->mWindow[anID] = pwin;
}


void QRchHist::DoMenuCommand(int anID)
{
	switch (anID) {
		case IDM_OPEN_OVERLAY_RCH:
			OpenRchFile(mOwner->GetData(), this);
			break;
			
		case IDM_CLOSE_OVERLAY_RCH:
			if (mRchOverlayFile) {
				delete mRchOverlayFile;
				mRchOverlayFile = NULL;
				delete [] mOverlay;
				mOverlay = NULL;
				Draw();
			}
			break;
	}
}
