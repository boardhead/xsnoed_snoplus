/////////////////////////////////////////////////////////////////////////
// A wrapper object for SNO Event Display .                            //
// The QSnoed object can be used to view events in ROOT or ZDAB files, //
// or from a dispatcher.  It also allows viewing of events in QTree's  //
// or TEventList's.  All of the functionality of xsnoed is included.   //
// To use this object from the C interpreter, you must be running a    //
// special version of ROOT that installs a QXsnoedSystem for the       //
// display to work properly.                                           //
// See the Begin_Html<a href="http://manhattan.sno.laurentian.ca/xsnoed/xsnoed.html">xsnoed documentation</a>End_Html for more information about the SNO Event Display //
/////////////////////////////////////////////////////////////////////////
//*-- Author :	Phil Harvey - 11/21/98

#include "QSnoed.h"
#include "QXsnoedSystem.h"
#include "QEvent.h"
#include "QPMT.h"
#include "QTree.h"
#include "QFit.h"
#include "PSpeaker.h"
#include "PEventControlWindow.h"
#include "PEventHistogram.h"
#include "XSnoedWindow.h"
#include "TApplication.h"
#include "TEnv.h"
#include "TError.h"
#include "TException.h"
#include "TEventList.h"
#include "TFile.h"
#include "TH1.h"
#include "xsnoed.h"
#include "xsnoedstream.h"
#include "menu.h"

ClassImp(QSnoed)


//______________________________________________________________________________

class TGXsnoedHandler : public TFileHandler
{
public:
   TGXsnoedHandler(Int_t fd) : TFileHandler(fd, 1) {  }
   Bool_t Notify();
   // Important: don't override ReadNotify()
};

Bool_t TGXsnoedHandler::Notify()
{
	xsnoed_service(0);
    return kFALSE;
}

//______________________________________________________________________________


QSnoed::QSnoed()
{
	Init();
	
	// QSnoed constructor -- opens xsnoed session
	OpenDisplay();
}

QSnoed::QSnoed(ImageData *data)
{
	// QSnoed constructor -- puts QSnoed object wrapper around already started xsnoed session
	Init(data);
}

QSnoed::~QSnoed()
{
	// QSnoed destructor -- closes xsnoed session
	CloseDisplay();
	
	if (mRchFile) delete mRchFile;
	
	if (gSnoed == this) gSnoed = NULL;
}

void QSnoed::Init()
{
	// Initialize necessary QSnoed data members
	mData = NULL;
	mRchFile = NULL;
	mAutoScale = kTRUE;		// auto-scaling on by default
}

void QSnoed::Init(ImageData *data)
{
	// Initialize the QSnoed object from existing xsnoed data
	static int	handler_installed = 0;
	
	Init();	// initialize our data members
	
	mData = data;
	if (!handler_installed) {
		handler_installed = 1;
		if (gSystem->IsA() != QXsnoedSystem::Class()) {
			printf("QSnoed: Warning QXsnoedSystem not installed -- display won't work\n");
		} else {
			// add handler so we will be able to service xsnoed events
			gSystem->AddFileHandler(new TGXsnoedHandler(ConnectionNumber(mData->display)));
		}
	}
	data->mMainWindow->AddListener(this);	// listen for object being deleted
}

void QSnoed::Listen(int message, void *message_data)
{
	// Listen for our main window getting deleted
	switch (message) {
		case kMessageSpeakerDying:
			// our main window object has been deleted - data is now invalid
			mData = NULL;
			break;
	}
}

void QSnoed::OpenDisplay()
{
	// Open the main xsnoed window
	if (!IsOpen()) {
		XSnoedWindow *main_window = new XSnoedWindow;
		ImageData *data = main_window->GetData();
		Init(data);
	}
}

void QSnoed::CloseDisplay()
{
	// Close the xsnoed display
	// - deletes all related window objects
	if (IsOpen()) {
		delete mData->mMainWindow;
	}
}

void QSnoed::View(char *aFilename)
{
	// View events from a file or dispatcher by name.
	// If no filename is given, the dispatcher "localhost" is assumed.
	// - filenames containing "root" are assumed to be ROOT files
	// - otherwise, names are assumed to be ZDAB files
	// - if no matching ZDAB file exists, a dispatcher connection is attempted
	if (!IsOpen()) return;
	if (!aFilename) {
		Clear();
	} else {
		mData->mEventFile.SetString(aFilename);
		open_event_file(mData,1);
	}
}

void QSnoed::View(TFile *aFile)
{
	// View events from a file
	if (!IsOpen()) return;
	if (!aFile) {
		Clear();
	} else {
		View((QTree *)aFile->Get("T"));
	}
}

void QSnoed::View(QTree *aTree)
{
	// View events from a QTree
	if (!IsOpen()) return;
	if (!aTree) {
		Clear();
	} else {
		root_open_tree(mData,aTree);
	}
}

void QSnoed::View(TEventList *aList, QTree *aTree)
{
	// View events from a TEventList in a QTree
	// If no QTree is specified, the currently viewed tree is assumed
	if (!IsOpen()) return;
	if (!aList) {
		Clear();
	} else {
		root_open_event_list(mData,aList,aTree);
	}
}

void QSnoed::View(QEvent *anEvent)
{
	// View an event
	if (!IsOpen()) return;
	
	if (!anEvent) {
		Clear();
	} else {
		aPmtEventRecord *per = root_load_qevent(anEvent);
		
		if (per) {
			// show the event
			addToHistory(mData,per, 1);	// add to history
			xsnoed_event(mData,per);
		}
	}
}

void QSnoed::View(TH1 *hist_qhl, TH1 *hist_tac, TH1 *hist_qhs, TH1 *hist_qlx)
{
	// View a 1-D histogram as Qhl in crate/card/channel ordering
	// - xsnoed color scale is automatically scaled to the data range for valid PMT's.
	// - dataType values are 1-Tac, 2-Qhs, 3-Qhl (the default), 4-Qlx
	// - taken from code by Aksel Hallin with mods by Bryce Moffat
	Int_t		i, j;
	QEvent		event;
	QPMT		*pmt;
	Int_t		Nhits = 0;
	const int	kNumHists = 4;
	TH1*		hist[kNumHists];
	TAxis*		axis[kNumHists];
	Float_t		minval[kNumHists];
	Float_t		maxval[kNumHists];
	
	// put histogram pointers into an array for convenience
	hist[0] = hist_qhl;
	hist[1] = hist_tac;
	hist[2] = hist_qhs;
	hist[3] = hist_qlx;

	for (i=0; i<kNumHists; ++i) {
		if (hist[i]) {
			// get number of bins from the first available histogram
			if (!Nhits) {
				Nhits = hist[i]->GetNbinsX();
				if (Nhits >= 20*512) {
					Nhits = 20*512 - 1;
				}
			}
			axis[i] = hist[i]->GetXaxis();	// get axis object for this histogram
			// initialize min/max values
			minval[i] = 1e9;
			maxval[i] = -1e9;
		}
	}
	if (!Nhits) return;	// nothing to do

	// initialize necessary elements of the event
	event.SetEvent_id(0);
	event.SetGtr_time((Double_t)0);
	event.SetNph(0);
	event.SetTrig_type(1);
	event.SetNhits(Nhits);
	event.SetBit(kCalibrated);
	
	gSNO->GetPMTxyz();  // Ensure we have a QPMTxyz database loaded

	// add PMT's to the event, with data values corresponding to histogram bin values
	for (j=0; j<Nhits; j++) {
	
		// add new PMT to the event, and initialize required elements
		pmt = event.AddQPMT();
		pmt->SetCell(1);
		pmt->Setn((Int_t)j);
		pmt->SetStatusBit(3);

		// set calibrated data values from histograms
		for (i=0; i<kNumHists; ++i) {
			Float_t value;
			if (!hist[i]) {
				value = 0;
			} else {
				value = hist[i]->GetBinContent(axis[i]->FindBin(j));
			}
			switch (i) {
				case 0:
					pmt->Sethl(value);
					break;
				case 1:
					pmt->Sett(value);
					break;
				case 2:
					pmt->Seths(value);
			  		break;
				case 3:
					pmt->Setlx(value);
					break;
			}
			// calculate value ranges for valid PMT's
			if (!gPMTxyz->IsInvalidPMT(j)) {
				if (value > maxval[i]) maxval[i] = value;
				if (value < minval[i]) minval[i] = value;
			}
		}
	}

	// expand maximum calibrated scale limits
	PEventHistogram::SetMaxCalScale(1e5, 1e5, 1e-5);

	// force XSnoed to view the appropriate data type
	if (mData && mData->mMainWindow) {	// better safe than sorry
	
		// set xsnoed to display precalibrated data
		mData->mMainWindow->SelectMenuItem(IDM_PRECALIBRATED);
		
		// loop backwards so we will end up displaying the first histogram passed
		for (i=kNumHists-1; i>=0; --i) {
			if (!hist[i]) continue;
			// set xsnoed to display the appropriate dataType
			switch (i) {
				case 0:
					SelectMenuItem(IDM_QHL);
					break;
				case 1:
					SelectMenuItem(IDM_TAC);
					break;
				case 2:
					SelectMenuItem(IDM_QHS);
					break;
				case 3:
					SelectMenuItem(IDM_QLX);
					break;
			}
			// set the histogram scales
			if (mAutoScale && minval[i]<maxval[i]) {
				SetScale(minval[i], maxval[i], kFALSE);	// set scale without updating display
			}
		}
	}
	
	View(&event);	// finally, view the constructed event
}

void QSnoed::View(Int_t anEventNum)
{
	// View event with the specified index
	if (!IsOpen()) return;
	
	aPmtEventRecord *per = root_load_event(anEventNum);
	
	if (per) {
		// show the event
		addToHistory(mData,per, 1);	// add to history
		xsnoed_event(mData,per);
	}
}

void QSnoed::Goto(Int_t aGTID)
{
	// View event with the specified GTID
	if (!IsOpen()) return;
	load_event(mData, aGTID, kGotoGTID);
}

void QSnoed::SetAutoScale(Bool_t on)
{
	// Turn on or off auto-scaling for QSnoed::View(TH1*,TH1*,TH1*,TH1*)
	mAutoScale = on;
}

void QSnoed::Cut(Text_t *aCutString)
{
	// Apply specified cuts to currently displayed QTree
	// Clear existing cuts if aCutString is NULL or empty
	
	static TEventList	*sEvt = NULL;
	
	if (!IsOpen()) return;

	// delete old event list if it exists
	if (sEvt) {
		delete sEvt;
		sEvt = NULL;
	}
	
	// get currently displayed QTree
	QTree *T = GetTree();
	
	// if no cuts, show whole tree
	if (!aCutString || !aCutString[0]) {
		if (T) View(T);
	} else if (T) {
		// create event list
		sEvt = new TEventList("QSnoedEventList");
		if (sEvt) {
			// do the cut
			T->Draw(">>QSnoedEventList",aCutString);
			printf("QSnoed: %d matching events\n",sEvt->GetN());
			View(sEvt);
		} else {
			printf("QSnoed: Out of memory for event list!\n");
		}
	} else {
		printf("QSnoed: No QTree currently open\n");
	}
}

void QSnoed::CloseFile()
{
	// Close any open files or dispatcher links
	if (!IsOpen()) return;
	close_event_file(mData);
}

void QSnoed::Add(QFit *aFit)
{
	// Add a fit to the display
	RconEvent	rcon;
	
	if (!IsOpen()) return;
	root_set_rcon(&rcon, aFit);
	
	// add reconstruction and update display only if we aren't in our callback
	// (if we are in the callback, it will get updated automatically later)
	xsnoed_add_rcons(mData, &rcon, 1, !mData->in_callback);
}

void QSnoed::SetEventCallback(QSnoedCallbackPtr aCallback)
{
	// Set callback to be made when a new event is displayed
	if (!IsOpen()) return;
	xsnoed_set_callback(mData, (XSnoedCallbackPtr)aCallback);
}

TFile *QSnoed::GetFile()
{
	// Get pointer to currently viewed file
	if (!IsOpen()) return(NULL);
	return(root_get_file());
}

QTree *QSnoed::GetTree()
{
	// Get pointer to currently viewed tree
	if (!IsOpen()) return(NULL);
	return(root_get_tree());
}

TEventList *QSnoed::GetEventList()
{
	// Get pointer to currently viewed event list
	if (!IsOpen()) return(NULL);
	return(root_get_event_list());
}

QEvent *QSnoed::GetEvent()
{
	// Get pointer to currently viewed event
	// Bonus: If the currently viewed event is not from a ROOT file
	// (i.e. from the dispatcher or a ZDAB file), this routine converts
	// the event into a QEvent before returning the pointer.
	if (!IsOpen()) return(NULL);
	return(root_get_event(mData,NULL));
}

int QSnoed::SelectMenuItem(int item_id, int window_id)
{
	// Select a specified menu item in a specified window
	// - defaults to window_id=0 for the main window
	// - see menu.h for menu item ID's and window ID's
	// - return codes:  0 = item selected OK
	//                 -1 = item not found in menu
	//                 -2 = window has no menu
	//                 -3 = main xsnoed window is not open
	//                 -4 = window ID out of range
	//                 -5 = window is not open
	PWindow *theWindow;
	
	if (!mData) return(-3);		// no image data
	
	if (window_id) {
		if (window_id<0 || window_id>=NUM_WINDOWS) {
			return(-4);			// invalid window ID
		}
		theWindow = mData->mWindow[window_id];
	} else {
		theWindow = mData->mMainWindow;
	}
	if (!theWindow) return(-5);	// window isn't open
	
	return(theWindow->SelectMenuItem(item_id));
}

void QSnoed::SetMenuItemText(int item_id, char *str, int window_id)
{
	// Set the text of a specified menu item in a specified window
	// - defaults to window_id=0 for the main window
	// - see menu.h for menu item ID's and window ID's
	if (mData) {
		PWindow *theWindow;
		if (window_id) {
			if (window_id>=0 && window_id<NUM_WINDOWS) {
				theWindow = mData->mWindow[window_id];
			} else {
				theWindow = NULL;
			}
		} else {
			theWindow = mData->mMainWindow;
		}
		if (theWindow) {
			theWindow->SetMenuItemText(item_id, str);
		}
	}
}

int QSnoed::PrintImage(char *filename, int flags, int window_id)
{
    // Print image to postscript file
	// - defaults to window_id=0 for the main window
	// - see menu.h for window ID's
	// - return codes:  0 = image printed OK
	//                 -1 = error writing to file
	//                 -2 = window has no image
	//                 -3 = main xsnoed window is not open
	//                 -4 = window ID out of range
	//                 -5 = window is not open
	PWindow *theWindow;
	
	if (!mData) return(-3);		// no image data
	
	if (window_id) {
		if (window_id<0 || window_id>=NUM_WINDOWS) {
			return(-4);			// invalid window ID
		}
		theWindow = mData->mWindow[window_id];
	} else {
		theWindow = mData->mMainWindow;
	}
	if (!theWindow) return(-5);	// window isn't open
	
	if (theWindow->Class() != PImageWindow::sImageWindowClass) {
	    return(-2);             // not an image window
	}
	PImageCanvas *theImage = ((PImageWindow *)theWindow)->GetImage();
	if (!theImage) {
	    return(-2);             // no image
	}
	if (!theImage->Print(filename, flags)) {
	    return(-1);             // error printing to file
	}
	return(0);
}

void QSnoed::SetScale(float minval, float maxval, Bool_t doUpdate)
{
	// Set event histogram range and hit color scale
	if (mData) {
		PEventHistogram::SetBins(mData, minval, maxval);
// why was I doing this?  - removed 11/18/99
//		calcCalibratedVals(mData);
		if (doUpdate) {
			calcHitVals(mData);
			sendMessage(mData, kMessageHitsChanged);
		}
	}
}

void QSnoed::Service()
{
	// Service all xsnoed gui events
	// (shouldn't have to call this if QXsnoedSystem is installed correctly)
	if (!IsOpen()) return;
	xsnoed_service_all();
}

void QSnoed::Sun()
{
	// Rotate 3-D display to the Sun direction
	if (!IsOpen()) return;
	sendMessage(mData, kMessageSetToSun);
	mData->mMainWindow->SetDirty();
}

void QSnoed::Home()
{
	// Rotate 3-D display to the Home position
	if (!IsOpen()) return;
	mData->mMainWindow->SetToHome();
}

void QSnoed::Clear()
{
	// Clear the current event
	if (!IsOpen()) return;
	xsnoed_clear(mData);
}

void QSnoed::Next()
{
	// View the next event in QTree, file, or from dispatcher
	if (!IsOpen()) return;
	xsnoed_next(mData,1);
}

void QSnoed::Prev()
{
	// View the previous event (from xsnoed history buffer)
	if (!IsOpen()) return;
	xsnoed_next(mData,-1);
}

void QSnoed::Cont(float sec)
{
	// View events continuously with the specified time interval in seconds
	if (!IsOpen()) return;
	if (sec > 0) {
		mData->time_interval = sec;
	}
	PEventControlWindow::SetEventFilter(mData);
	setTriggerFlag(mData,TRIGGER_CONTINUOUS);
}

void QSnoed::Stop()
{
	// Stop continuous display of events -- started by Cont()
	if (!IsOpen()) return;
	setTriggerFlag(mData,TRIGGER_OFF);
}

//temporary solution, added by mgb
void QSnoed::SetCal(QCal *cal)
{
	if (!IsOpen()) return;
    delete mData->root_cal;
    mData->root_cal = cal;
}

void QSnoed::SetRch(TFile *aFile)
{
	// Set Rch file for singles histogram display
	if (mRchFile) {
		// erase any Rch file we created ourselves
		delete mRchFile;
		mRchFile = NULL;
	}
	mData->owns_rch_file = 0;
	mData->root_rch_file = aFile;
	int pmtn = mData->rch_pmtn;
	mData->rch_pmtn = -1;
	root_update_rch_windows(mData,pmtn);
}

void QSnoed::SetRch(char *filename)
{
	// Set Rch file by name
	TFile *f = new TFile(filename);
	SetRch(f);
	mRchFile = f;	// save pointer so we will delete it later
}
