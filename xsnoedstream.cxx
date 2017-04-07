/* xsnoedstream.c -
**   Contact: Phil Harvey, Queen's University
**   This file contains remnants of snostream.C by Charles Duba
**
**   XSNOED event handling code
 */

#define NO_SIGINT		// until I get this working right - PH 12/07/99
//#define DUMP_SCOPE_DATA   // for debugging the scope data

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>
// added for the HostToNetworkLong function
#include <netinet/in.h>
#include "menu.h"
#include "xsnoed.h"
#include "xsnoedstream.h"
#include "PZdabFile.h"
#include "PListener.h"
#include "PUtils.h"
#include "include/Record_Info.h"
#include "include/cmosdata.h"
#include "XSnoedWindow.h"
#include "PEventControlWindow.h"
#include "PResourceManager.h"
#include "PEventTimes.h"
#include "openfile.h"

#ifndef NO_DISPATCH
#include "include/dispatch.h"
#endif

// -NT
#ifdef XSNOMAN
#include "xsnoman.h"
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/* includes, defines and variables for reading root files */
#ifdef ROOT_FILE
#include "QSnoed.h"
#include "QDispatch.h"
#ifdef R__UNIX
#include "QXsnoedSystem.h"
#endif
#include "QEvent.h"
#include "QTree.h"
#include "QPmtEventRecord.h"
#include "QRchHist.h"
#include "TEnv.h"
#include "TROOT.h"
#include "TFile.h"
#include "TRint.h"
#define ROOT_FILE_FLAG		((FILE *)0xffff1234UL)		// flag for root file
static int32			root_event_num = -1;
static TFile 		  *	root_file_ptr = NULL;
static QTree		  *	root_tree = NULL;
static TEventList	  *	root_event_list = NULL;
static QEvent		  * root_event = NULL;
static QPmtEventRecord*	root_pmtRecord = NULL;
#endif
char test_buff[100];

/* constants */
#ifndef NO_DISPATCH
#ifdef SNOPLUS
#define DISP_BUFSIZ			200000		// dispatcher receive buffer size (large enough for NHIT 10000 + 8 CAEN traces)
#else
#define DISP_BUFSIZ			300000		// dispatcher receive buffer size (large enough for NHIT 10000 + 8 NCD scope traces)
#endif
#define MAX_DATALESS		50			// max 50 loops without data, then start being nice
#define PACKETS_PER_LOOP	100			// number of dispatcher packets to read per loop
#define NICE_WAIT_MS		100			// wait time (ms) between polls while dataless
#define STATUS_CHECK_TIME	10			// seconds of receiving nothing before we send a status message
#define STATUS_DEAD_TIME	30			// seconds before we declare dispatcher dead
#define STATUS_COUNT		4			// number of times we think the dispatcher is dead before trying to reconnect
#define RETRY_CONNECT_TIME	10			// seconds between retrying dispatcher connection
#define DISP_NBUF           PACKETS_PER_LOOP + 1 // number of dispatcher read-ahead buffers
#endif

#if defined(USE_PTHREAD) && !defined(NO_DISPATCH)
#include <pthread.h>
enum {
	THREAD_CMD_NULL,
	THREAD_CMD_CONNECT,
	THREAD_CMD_DISCONNECT,
	THREAD_CMD_GET_DATA,
	THREAD_CMD_DIE
};

#ifndef NO_SIGINT
static int do_quit = 0;
#endif
static pthread_t tid;
static int	thread_running = 0;
static int	thread_command = THREAD_CMD_NULL;
static int	thread_command_count = 0;
static int 	thread_response_count = 0;
static pthread_mutex_t command_mutex = PTHREAD_MUTEX_INITIALIZER;
void * do_dispatcher_thread(void *arg);
int get_data_threaded(void);
int do_thread_command(int cmd);
int thread_command_complete(void);
#endif

void ProcessEvents(ImageData *data, int theMode, long time_interval_ms);

// --NT 7/11/2000
static void do_auto_rotate(ImageData *data);
static float auto_rotate_period = 15.0;

/* static variables */
#ifndef ROOT_SYSTEM
static XtWorkProcId	work_proc_id;
#endif

#ifndef NO_DISPATCH
static char         disp_buf_next[DISP_NBUF][DISP_BUFSIZ];
static char       * disp_buf = disp_buf_next[0];
static char			disp_hostname[256] = "localhost";
static int 			statuscheck, statusdata,dispconnectstatus;
static char 		disp_tag[TAGSIZE+1];
static char         disp_tag_next[DISP_NBUF][TAGSIZE+1];// set by dispatcher thread, cleared by main thread
static int			disp_nbytes;
static int          disp_nbytes_next[DISP_NBUF] = { 0 };
static int          disp_head = 0;          // next dispatcher buffer to fill (set by dispatcher thread)
static int          disp_tail = 0;          // next dispatcher buffer to read out (set by main thread)
static time_t		status_send_time = 0;
static time_t		last_recv_time = 0;
static int			isConnected = 0;
static time_t		retryTime = 0;
static int			retryConnection = 0;
static u_int32		dataless_count = 0;
static ImageData  *	dispatcher_data = NULL;
static u_int32		disp_last_gtid = 0;
static int			brokenPipe = 0;
#endif

#if !defined(NO_MAIN) && !defined(ROOT_APP)
static char *		good_prog_path = NULL;
#endif

#ifdef ROOT_FILE
void create_root_system();
#endif

extern int		g_argc;
extern char **	g_argv;

#if !defined(NO_MAIN) && !defined(ROOT_APP)
static void cleanup(void)
{
	// make sure our main window is deleted properly
	if (PWindow::sMainWindow) delete PWindow::sMainWindow;
	
	if (good_prog_path) delete [] good_prog_path;
}
#ifndef NO_SIGINT
static void sigIntAction(int sig)
{
	do_quit = 1;
	if (XSnoedWindow::sMainWindow) {
		// send dummy event to break out of event loop
		ImageData *data = XSnoedWindow::sMainWindow->GetData();
		XClientMessageEvent dummy;
		dummy.type = ClientMessage;
		dummy.format = 8;
		XSendEvent(data->display, XtWindow(data->toplevel), FALSE, 0, (XEvent *)&dummy);
	}
}
#endif

/* handle SIGPIPE errors */
#ifndef NO_SIGPIPE
static void sigPipeAction(int sig)
{
#ifndef NO_DISPATCH
	brokenPipe = 1;
#else
	Printf("Broken pipe\n");
#endif
}
#endif

/* main program */
int main(int argc, char *argv[])
{
	int		i;
	int		load_settings = 1;
#ifdef DEMO_VERSION
	int		is_protected = 0;
#endif
	
	extern char *progname;
	extern char *progpath;
	
	g_argc = argc;
	g_argv = argv;
	
	atexit(&cleanup);	// make sure our cleanup routine is called when we quit

#ifndef NO_SIGPIPE
    // Set up signal handler for SIGPIPE
	struct sigaction act1;
	sigemptyset( &act1.sa_mask );
	act1.sa_flags = SA_SIGINFO;
	act1.sa_handler =  sigPipeAction;
	if ( sigaction( SIGPIPE, &act1, (struct sigaction *) NULL ) < 0 ) {
		Printf("Error installing SIGPIPE handler\n");
	}
#endif

#ifndef NO_SIGINT
    // Set up signal handler for SIGINT so that we can clean up the EB
	struct sigaction act;
	sigemptyset( &act.sa_mask );
	act.sa_flags = SA_SIGINFO;
	act.sa_handler =  sigIntAction;
	if ( sigaction( SIGINT, &act, (struct sigaction *) NULL ) < 0 ) {
		Printf("Error installing SIGINT handler\n");
	}
#endif
	
	/* get pointer to program name */
	progpath = argv[0];
	progname = strrchr(argv[0],'/');
	if (progname) ++progname;
	else {
		progname = argv[0];
		// find path for the executable - PH 10/20/99
		// (if it was located through PATH, it may not have the full path in the filename)
		FILE *fp = openFile(progname,"r",NULL);
		if (fp) {
			good_prog_path = new char[strlen(getOpenFileName())+1];
			if (good_prog_path) {
				strcpy(good_prog_path, getOpenFileName());
				progpath = good_prog_path;
			}
			fclose(fp);
		}
	}
	// look for command line options
	int auto_rotate = 0;
	for (i=1; i<g_argc; ++i) {
		if (g_argv[i][0] == '-' && g_argv[i][1]!='\0' && g_argv[i][2]=='\0') {
			switch (g_argv[i][1]) {
				case 'n':	// don't load settings from resources
					load_settings = 0;
					break;
				case 's':	// specify alternate settings filename
					// set settings filename
					if (i+1<g_argc && g_argv[i+1][0] != '-') {
						PResourceManager::SetSettingsFilename(g_argv[++i]);
					}
					break;

				case 'R':
					auto_rotate = 1;
					if (i+1<g_argc && g_argv[i+1][0] != '-') {
						sscanf(g_argv[++i],"%f",&auto_rotate_period);
					}
					if(auto_rotate_period<=1.0) auto_rotate_period = 1.0;
					break;

#ifdef DEMO_VERSION
				case 'p':	// start in protected mode
					is_protected = 1;
					break;
#endif
			}
		}
	}
  	ImageData *data = xsnoed_create(load_settings);
  	data->mDeleteData = 1;
  	data->mQuitOnDelete = 1;
#ifndef NO_DISPATCH  
  	dispconnectstatus = -1;
#endif
#ifdef DEMO_VERSION
	if (is_protected) data->mMainWindow->SetProtect(1);
#endif

	// look for filename in command line options
	// (must be done after xsnoed_create() so the resource
	//  settings will be removed from the command line options)
	int dump_level = -1;
	int do_open_file = 0;
	for (i=1; i<g_argc; ++i) {
		if (g_argv[i][0] == '-') {
			switch (g_argv[i][1]) {
				case 's':
				case 'R':
					++i;	// skip additional argument
					break;
				case 'd':
					if (g_argv[i][2]>='0' && g_argv[i][2]<='9') {
						// set record dump
						dump_level = g_argv[i][2] - '0';
					} else {
						dump_level = 1;	// default to level 1
					}
					break;
			}
		} else if (!do_open_file) {
			// doesn't start with "-", must be the name of a file to open
			data->mEventFile.SetString(g_argv[i]);
			// set flag to open this file later
			do_open_file = 1;
		}
	}
	// set record dump level
	if (dump_level >= 0) {
		data->mMainWindow->SetDumpRecords(dump_level);
		Printf("Dump Records level set to %d\n",dump_level);
	}
	// open event file if specified
	if (do_open_file) {
		open_event_file(data,1);
		// open event control window if we were successful
		if (data->infile
#ifndef NO_DISPATCH
			|| (data==dispatcher_data && isConnected)
#endif
		) {
			if (load_settings && !data->mWindow[EVT_NUM_WINDOW]) {
				data->mMainWindow->ShowWindow(EVT_NUM_WINDOW);
			}
		}
	}
	

	// Start up autorotate sequence
	if (auto_rotate) do_auto_rotate(data);

#ifdef ROOT_FILE
	// start ROOT
	do_root(data);
#else	
	while (xsnoed_running(data)) {
		xsnoed_service(1);
#ifndef NO_SIGINT
		if (do_quit) break;
#endif
	}
#endif
}
#endif // NO_MAIN
  
  
/* work proc callback - used to read events */
#ifdef ROOT_SYSTEM
class XSnoedstreamListener : public PListener {
public:
	XSnoedstreamListener(ImageData *data) : mData(data) { }
	virtual void	Listen(int message, void *message_data);
	ImageData		*mData;
};
void XSnoedstreamListener::Listen(int message, void *message_data)
{
	if (message == kMessageWorkProc) {
		if (mData) {
			// remove listener until next time
			((QXsnoedSystem *)gSystem)->RemoveListener(this);
			// handle work proc ourselves
			if (mData->process_mode & PROCESS_WORK_PROC) {
				mData->process_mode &= ~PROCESS_WORK_PROC;
				HandleEvents(mData);
			}
		}
	}
}
#else
static Boolean WorkProcCallback(ImageData *data)
{
#ifdef LESSTIF
	if (!(data->process_mode & PROCESS_WORK_PROC)) return(TRUE);
#endif
	/* reset process mode because our callback is only called once */
	data->process_mode &= ~PROCESS_WORK_PROC;
	
	HandleEvents(data);

	return(TRUE);	// remove work proc if it exists
}
#endif

/* do_auto_rotate -- used as a callback  -- NT*/
static void do_auto_rotate(ImageData *data)
{
	const double kIncrementTime = 0.05;  // Update the display every so often

    if (!XSnoedWindow::IsValidData(data)) return;

	double curtime = double_time();

	// Absolute angle (in fraction of 2 pi) that we want to rotate to
	double rot_angle = fmod(curtime,(double) auto_rotate_period)/auto_rotate_period;
	//printf("Time is %f --> Rotate to %f, period = %f",curtime,rot_angle,auto_rotate_period);

	// If we have a 3d display, update it.
	if (data->mMainWindow) {
		PProjImage* image = (PProjImage*) data->mMainWindow->GetImage();
		if (image && image->GetProj()->proj_type==IDM_PROJ_3D) {
			//printf("--> scroll to %f",kScrollMax*rot_angle);
			image->ScrollValueChanged(kScrollBottom,(int)(kScrollMax*rot_angle));
			image->SetScrolls();
			PWindow::HandleUpdates();
		}
	}
	//printf("\n");
      
	// Setup another callback proc to get it right.
	long time_interval_ms =(long)(kIncrementTime * 1000.);
	XtAppAddTimeOut(data->the_app, time_interval_ms, (XtTimerCallbackProc)do_auto_rotate, data);
}



/* time out callback -- used to display events */
static void TimeOutCallback(ImageData *data)
{
    if (!XSnoedWindow::IsValidData(data)) return;
#ifdef LESSTIF
	if (!(data->process_mode & PROCESS_TIME_OUT)) return;
#endif
	/* reset process mode because our callback is only called once */
	data->process_mode &= ~PROCESS_TIME_OUT;

	HandleEvents(data);
}


/* throw out callback -- used to stop throwing out dispatched events */
static void ThrowOutCallback(ImageData *data)
{
    if (!XSnoedWindow::IsValidData(data)) return;
#ifdef LESSTIF
	if (!(data->process_mode & PROCESS_THROW_OUT)) return;
#endif
	/* reset process mode because our callback is only called once */
	data->process_mode &= ~PROCESS_THROW_OUT;

	/* re-activate trigger */
	/* (only if trigger is continuous -- user could have pressed 'Single' */
	/*  after this callback was registered) */
	if (data->trigger_flag == TRIGGER_CONTINUOUS) {
		/* - will look for a good event in future buffer and display the last one that exists */
		/* - also resets throw_out_data flag if necessary */
		ActivateTrigger(data);
	}
}

// Kick - kick our callbacks to patch linux bug
//
// - for some reason we can miss callbacks after a window is destroyed in linux.
//   This call is designed to patch this problem.
//
#ifdef LESSTIF
void Kick(ImageData *data)
{
#ifndef ROOT_SYSTEM
	if (data->process_mode & PROCESS_WORK_PROC) {
		WorkProcCallback(data);
	}
#endif
	if (data->process_mode & PROCESS_TIME_OUT) {
		TimeOutCallback(data);
	}
	if (data->process_mode & PROCESS_THROW_OUT) {
		ThrowOutCallback(data);
	}
}
#endif

/*
** ProcessEvents() - initialize callbacks to start processing events from file or dispatcher
*/
void ProcessEvents(ImageData *data, int theMode, long time_interval_ms)
{
	XtIntervalId id;
	
	switch (theMode) {
		case PROCESS_WORK_PROC:
			/* start work proc if not already running */
			if (!(data->process_mode & PROCESS_WORK_PROC)) {
				data->process_mode |= PROCESS_WORK_PROC;
#ifndef ROOT_SYSTEM
				work_proc_id = XtAppAddWorkProc(data->the_app, (XtWorkProc)WorkProcCallback, data);
#else
				if (gSystem->IsA() == QXsnoedSystem::Class()) {
					if (!data->mWorkListener) {
						data->mWorkListener = new XSnoedstreamListener(data);
					}
					((QXsnoedSystem *)gSystem)->AddListener(data->mWorkListener);
				}
#endif
			}
			break;
		case PROCESS_TIME_OUT:
			/* add timeout if there is a trigger and we are idle */
			if ((data->process_mode & (PROCESS_WORK_PROC | PROCESS_TIME_OUT)) == PROCESS_IDLE) {
	 			data->process_mode |= PROCESS_TIME_OUT;
				if (!time_interval_ms) time_interval_ms = (long)(data->time_interval * 1000);
			 	id = XtAppAddTimeOut(data->the_app, time_interval_ms, 
			 					(XtTimerCallbackProc)TimeOutCallback, data);
	   	 	}
	   	 	break;
	   	case PROCESS_THROW_OUT:
	   		if (!(data->process_mode & PROCESS_THROW_OUT)) {
	 			data->process_mode |= PROCESS_THROW_OUT;
				if (!time_interval_ms) time_interval_ms = (long)(data->time_interval * 1000);
			 	id = XtAppAddTimeOut(data->the_app, time_interval_ms, 
			 					(XtTimerCallbackProc)ThrowOutCallback, data);
	   		}
	   		break;
	}
}

/* an active trigger has been enabled.  Scan future buffer immediately */
/* Returns non-zero if an event was displayed */
void ActivateTrigger(ImageData *data)
{
	if (data->trigger_flag) {
		int	first, last, incr;
		if (data->trigger_flag==TRIGGER_SINGLE || data->infile) {
			// search forward through future events for next event
			first = 0;
			last = data->history_size[HISTORY_FUTURE];
			incr = 1;
		} else {
			// search backward from end of future events to find last event
			// (this is necessary when viewing live data to avoid getting too
			// far behind -- tried removing but lead to problems, PH 07/10/00)
			first = data->history_size[HISTORY_FUTURE] - 1;
			last = -1;
			incr = -1;
		}
		for (int i=first; i!=last; i+=incr) {
			HistoryEntry *entry = data->history_buff[HISTORY_FUTURE][i];
			if (!entry) continue;
			PmtEventRecord *thePmtRecord = (PmtEventRecord *)(entry + 1);
			if (checkEvent(data, thePmtRecord)) {
				/* we found an event we want to display... */
				/* shift events up to and including this one into 'all' buffer */
				do {
					entry = data->history_buff[HISTORY_FUTURE][0];
					if (!entry) {
						Printf("Ouch! NULL event in real-time future buffer!\n");
						continue;
					}
					thePmtRecord = (PmtEventRecord *)(entry + 1);
					addToHistory(data, thePmtRecord, HISTORY_ALL);	// add to 'all' history
					// put appropriate events into viewed history, even
					// if we didn't view them
					if (incr<0 && i!=0 && checkEvent(data, thePmtRecord)) {
						// add to viewed history
						addToHistory(data, thePmtRecord, HISTORY_VIEWED);
					}
				} while (--i >= 0);
				
				if (xsnoed_event(data, thePmtRecord)) {
					/* re-arm our callback if we are still throwing out data */
					/* (only if in continous mode -- otherwise throw out indefinitely) */
					if (data->throw_out_data && data->trigger_flag==TRIGGER_CONTINUOUS) {
						ProcessEvents(data,PROCESS_THROW_OUT, 0);
					}
					// return now and don't reset the throw_out_data flag
					return;
				}
				break;
			}
		}
		data->throw_out_data = 0;	// reset throw_out flag to look for next event
	} else {
		// set throw_out flag so we will save (but not view) incoming live data
		data->throw_out_data = 1;
	}
	return;
}

/* check to see if it is time for a display */
static void CheckDisplayTime(ImageData *data,int display_required)
{
	if (data->display_time) {
		/* display events only occasionally while summing */
		if (double_time() >= data->display_time) {
			/* update the display */
			xsnoed_event(data, (aPmtEventRecord *)0);
			/* reset the display time */
			if (display_required) {
				/* get time again here because display can take forever on a slow net connection! */
				data->display_time = double_time() + data->time_interval;
			} else {
				/* no more displays required */
				data->display_time = 0;
			}
		}
	} else if (display_required) {
		/* set time for next display */
		data->display_time = double_time() + data->time_interval;
	}
	// check to see if event times histogram needs updating if it exists
	if (data->mEventTimes) {
		data->mEventTimes->CheckUpdate();
	}
}

#ifdef ROOT_FILE // -------------------------------------------------------------------------

/* create root system if necessary */
/* also intializes root_event */
void create_root_system()
{
	extern void  InitGui();
	static VoidFuncPtr_t initfuncs[] = { InitGui, 0 };
	
	if (!gROOT) {
		Printf("Initializing ROOT\n");
		gROOT = ::new TROOT("theRoot","xsnoed_root",initfuncs);
	}
	if (!root_event) {
		root_event = new QEvent();
	}
}

/* load a ROOT event for purposes of display */
aPmtEventRecord *root_load_qevent(QEvent *anEvent)
{
	// root_pmtRecord was a static object, but for some reason quitting root
	// could double-delete the object, so now I allocate it dynamically here. - PH 01/14/99
	if (!root_pmtRecord) {
		root_pmtRecord = new QPmtEventRecord;
		if (!root_pmtRecord) {
			Printf("Error creating QPmtEventRecord (out of memory?)\n");
			exit(1);
		}
	}
	// convert QEvent into a PmtEventRecord
	root_pmtRecord->FromQEvent(anEvent);
	
	return(root_pmtRecord->GetPmtEventRecord());
}

char *old_branch_address = NULL;
Bool_t old_auto_delete = false;

static void set_tree_state()
{
	if (root_tree) {
		TBranch *branch = root_tree->GetBranch("Events");
		if (branch) {
			old_branch_address = branch->GetAddress();
			old_auto_delete = branch->IsAutoDelete();
			branch->SetAddress(&root_event);
			branch->SetAutoDelete(FALSE);
			return;
		}
	}
	printf("NO BRANCH!\n");
	old_branch_address = NULL;
	old_auto_delete = false;
}
static void restore_tree_state()
{
	if (root_tree) {
		TBranch *branch = root_tree->GetBranch("Events");
		if (branch) {
			branch->SetAddress(old_branch_address);
			branch->SetAutoDelete(old_auto_delete);
		}
	}
}

aPmtEventRecord *root_load_event(int anEvent)
{
	aPmtEventRecord *per = NULL;
	root_event_num = anEvent;		// set the ROOT event number
	
	create_root_system();	// init root system (creates root_event)
	
	if (root_event_list) {
		// translate event number from list entry index
		anEvent = root_event_list->GetEntry(anEvent);
	}
	
	if (root_tree) {
		// load next event from tree
		set_tree_state();
		if (root_tree->GetEvent(anEvent) > 0) {
			per = root_load_qevent(root_event);
		}
		restore_tree_state();
	} else {
		Printf("Cannot load event by index with a QTree\n");
	}
	return(per);
}

/* fill in entries of an RconEvent given a QFit */
void root_set_rcon(RconEvent *rcon, QFit *aFit)
{
	if (aFit) {
		rcon->pos[0] = aFit->GetX();
		rcon->pos[1] = aFit->GetY();
		rcon->pos[2] = aFit->GetZ();
		rcon->dir[0] = aFit->GetU();
		rcon->dir[1] = aFit->GetV();
		rcon->dir[2] = aFit->GetW();
		rcon->time = aFit->GetTime();
		rcon->chi_squared = aFit->GetQualityOfFit();
		rcon->fit_pmts = aFit->GetNumPMTsUsed();
		const char *pt = aFit->GetName();
		if (!pt) pt = "<null>";
		strncpy(rcon->name,pt,31);	// copy fit name
		rcon->name[31] = 0;	// make sure it's terminated
	} else {
		memset(rcon, 0, sizeof(RconEvent));
	}
}

TFile *root_get_file(void)
{
	return(root_file_ptr);
}
QTree *root_get_tree(void)
{
	return(root_tree);
}
TEventList *root_get_event_list(void)
{
	return(root_event_list);
}
QEvent *root_get_event(ImageData *data, QEvent *anEvent)
{
/*
** convert displayed event into a QEvent
** Note: even if we are displaying a root event, we must still do this conversion
** because the user may be displaying an event other than the last QEvent loaded.
** - event is calibrated if anEvent=NULL, uncalibrated otherwise
*/
	int		 i, do_cal;
		
	create_root_system();	// make sure the root system is initialized
	
	if (!anEvent) {
		anEvent = root_event;
		do_cal = 1;
	} else {
		do_cal = 0;
	}
	anEvent->Clear(0);	// clear event
	
	// fill in event information
	double	theTime = data->event_time;
	if (theTime) {
		theTime = theTime - data->root_time_zero;
	}
	double time_50MHz = data->mtc_word[2] * (double)2048.0 +
					  (data->mtc_word[1] >> 21);
					  
	anEvent->SetEvent_id(data->event_id);
	anEvent->SetTrig_type(((data->mtc_word[3] & 0xff000000UL) >> 24) |
			   			  ((data->mtc_word[4] & 0x0007ffffUL) << 8));
	anEvent->SetNhits(data->event_nhit);
	anEvent->SetnPBUNs(data->hits.num_nodes);
	anEvent->SetJulianDate((Int_t)(theTime / (24 * 60 * 60)));
	anEvent->SetUT1((Int_t)(theTime - anEvent->GetJulianDate() * (double)(24 * 60 * 60)));
	anEvent->SetUT2((Int_t)((theTime - (unsigned long)theTime) * 1e9));
	anEvent->SetNph(0);	// <-- how is Nph derived????? !!!!!
	anEvent->SetGtr_time(time_50MHz / ((double)1e-9 * 50e6));
	anEvent->SetRun(data->run_number);
	anEvent->SetEventIndex(data->event_num);
	anEvent->SetBit(kCalibrated,kTRUE);
	anEvent->SetEsumPeak((Int_t)(data->mtc_word[4] >> 19) & 0x03ff);
	anEvent->SetEsumInt((Int_t)(data->mtc_word[5] >> 7) & 0x03ff);
	anEvent->SetEsumDiff((Int_t)((data->mtc_word[4] >> 29) | (data->mtc_word[5] << 3)) & 0x03ff);
	
	// add hits
	QPMT aPmt;
	HitInfo *hi = data->hits.hit_info;
	for (i=0; i<data->hits.num_nodes; ++i, ++hi) {
		if (hi->flags & data->bit_mask) continue;	/* only consider unmasked hits */
		aPmt.Setn(((hi->crate * 16) + hi->card) * 32 + hi->channel);
		aPmt.SetCell(hi->cell);
		aPmt.SetStatus(0);
		aPmt.Setihl(hi->qhl);
		aPmt.Setihs(hi->qhs);
		aPmt.Setilx(hi->qlx);
		aPmt.Setit(hi->tac);
		if (do_cal) {
			double tmp = hi->calibrated;	// save current calibrated value
			setCalibratedQhl(data,hi,i);
			aPmt.Sethl(hi->calibrated);
			setCalibratedQhs(data,hi,i);
			aPmt.Seths(hi->calibrated);
			setCalibratedQlx(data,hi,i);
			aPmt.Setlx(hi->calibrated);
			setCalibratedTac(data,hi,i);
			aPmt.Sett(hi->calibrated);
			hi->calibrated = tmp;			// restore original calibrated value
		} else {
			aPmt.Sethl(0);	// calibrated qhl
			aPmt.Seths(0);	// calibrated qhs
			aPmt.Setlx(0);	// calibrated qlx
			aPmt.Sett(0);	// calibrated tac
		}
		// set discarded flag for hit
		aPmt.SetBit(0, hi->flags&HIT_DISCARDED ? kTRUE : kFALSE);
		anEvent->Add(&aPmt);	// add this PMT to the event
	}
	// add reconstructions
	for (i=0; i<data->nrcon; ++i) {
		QFit qfit;
		RconEvent *rcon = data->rcon + i;
		qfit.SetX(rcon->pos[0] * data->tube_radius);
		qfit.SetY(rcon->pos[1] * data->tube_radius);
		qfit.SetZ(rcon->pos[2] * data->tube_radius);
		qfit.SetU(rcon->dir[0]);
		qfit.SetV(rcon->dir[1]);
		qfit.SetW(rcon->dir[2]);
		qfit.SetTime(rcon->time);
		qfit.SetQualityOfFit(rcon->chi_squared);
		qfit.SetNumPMTsUsed(rcon->fit_pmts);
		qfit.SetName(rcon->name);
		anEvent->AddQFIT(&qfit);
	}

	return(anEvent);
}
void root_free_data(ImageData *data)
{
	delete data->mWorkListener;
	data->mWorkListener = NULL;
}
#endif // ROOT_FILE -------------------------------------------------------------------

/* handle incoming events */
void HandleEvents(ImageData *data)
{
	int			n_future;
#ifndef NO_DISPATCH	
    int			i, rc;
#ifndef USE_PTHREAD
    int			statusout,position;
    time_t 		cur_time;
#endif
#endif
	int			processFlag = PROCESS_TIME_OUT;
	aPmtEventRecord *thePmtRecord;

#ifdef XSNOMAN
	// -NT: Flag for more data if 'next' button pressed.
	if (data->input_source == INPUT_XSNOMAN) {
	  
	  if( data->trigger_flag == TRIGGER_OFF ) return;

	  // Check to see if we've already burned the current data to the screen:
		if(!data->snoman_has_data) {  
	   
			// We have, so set the flag to return to snoman on the next event-loop pass in
			// run_xsnoman_until_needy_... we're needy now.
			data->exit_to_snoman = TRUE;

			// Re-arm the processor to load up the event when ready.
			processFlag = PROCESS_WORK_PROC;
			ProcessEvents(data,processFlag,0);
			return;
	    
		} else {
		    // We have data!
		    
		    // Burn the data:
		    thePmtRecord = get_next_event(data);
		    
		    // Set flag for burned data:
		    data->snoman_has_data = FALSE;
		    
		    // This is the same as file input, except that we don't want to read ahead
		    // by setting the throw_out_data flag:
		    
		    if (thePmtRecord) {

				if(data->snoman_replace_mode) {
			
					xsnoed_replace(data, thePmtRecord); 
		      
				} else {  
			
					if (xsnoed_filter(data, thePmtRecord)) {
						// display event (or sum it if in sum mode)
						if (!xsnoed_event(data, thePmtRecord)) {
			  				// display the sum at regular intervals
							CheckDisplayTime(data,TRUE);
						}
					}
		      
		    	}
		    } else {
				Printf("No more SNOMAN events\x07\n");
		      
				// turn off our trigger
				// (set the end_of_data flag so it doesn't try to continue reading into future buffer)
				setTriggerFlag(data,TRIGGER_OFF,1);
		    }
		    
		    // Re-arm processor only if we're running continuously.
		    if(data-> trigger_flag == TRIGGER_CONTINUOUS) ProcessEvents(data,processFlag, 0);
	
		    CheckDisplayTime(data,FALSE);
		    
		    return;
		}
	} // End INPUT_XSNOMAN
	// -NT ends
#endif // XSNOMAN

#ifndef NO_DISPATCH	
	if (retryTime && data==dispatcher_data) {
		if (retryTime < time(NULL)) {
			InitDispLink(data,1);
		}
		if (!isConnected) {
			// poll again after waiting a while
			ProcessEvents(data,PROCESS_TIME_OUT, NICE_WAIT_MS);
			CheckDisplayTime(data,FALSE);
			return;
		}
	}
#endif
	
	n_future = data->history_size[HISTORY_FUTURE];
	if (data->trigger_flag == TRIGGER_OFF) {
		if (!data->throw_out_data || n_future>=FUTURE_SIZE ||
			// stop reading ahead if our future buffer has read to the end of the file
			(n_future>0 && data->history_buff[HISTORY_FUTURE][n_future-1]==NULL))
		{
			/* return without installing work proc if our trigger is off */
			return;
		}
	}
	
	/* handle the case where we are getting input from file */
	if (data->infile) {
	
		// read next event from file
		if (data->throw_out_data) {
			if (n_future >= FUTURE_SIZE) {
			
				Printf("Ouch! History buffer full on read from file!\n");
				
			} else if (put_next_event_into_future(data)) {
			
				if (n_future < HISTORY_SIZE-2) {
					// fill future buffer as quickly as possible
	 				processFlag = PROCESS_WORK_PROC;
	 			}
			}
	 		thePmtRecord = NULL;	// set to null because we don't want to display anything
		} else {
			// read next event from file
			thePmtRecord = get_next_event(data);
		}

		if (thePmtRecord) {
			if (xsnoed_filter(data, thePmtRecord)) {
				/* display event (or sum it if in sum mode) */
 				if (!xsnoed_event(data, thePmtRecord)) {
	 				/* display the sum at regular intervals */
	 				CheckDisplayTime(data,TRUE);
 				} else if (!data->throw_out_data
#ifdef XSNOMAN
						 && (data->input_source != INPUT_XSNOMAN)
#endif // XSNOMAN
				) {
	 				data->throw_out_data = 1;	// start throwing out data now
 					ProcessEvents(data,PROCESS_THROW_OUT, 0);
				}
 			}
  			// read as quickly as possible to find next event or fill future buffers
 			processFlag = PROCESS_WORK_PROC;
 			
  		} else if (!data->throw_out_data) {
  			Printf("No more events in file\x07\n");
   			
//  			rewind_event_file(data);

   			// turn off our trigger
   			// (set the end_of_data flag so it doesn't try to continue reading into future buffer)
   			setTriggerFlag(data,TRIGGER_OFF,1);
   		}
 		// re-arm our processor
		ProcessEvents(data,processFlag, 0);
		
		CheckDisplayTime(data,FALSE);
		return;
	}

#ifndef NO_DISPATCH	
	/* exit now if we aren't connected to a dispatcher */
	if (!isConnected || data!=dispatcher_data) {
		/* make sure displays are updated before leaving */
		CheckDisplayTime(data,FALSE);
		return;
	}
	
	++dataless_count;		// increment our dataless counter (will be reset to zero if we get data)
	
    // loop over max number of packets to receive per callback
    for (i=0; i<PACKETS_PER_LOOP; ++i) {

		// Check with dispatcher to see if any events with a tag matching 
		// the ones xsnoed signed up for are being sent.  If so, rc!=0, 
		// and we need to read in what data is waiting in the 
		// appropriate formats.
#ifdef USE_PTHREAD

		rc = get_data_threaded();
		
		if (rc > 0) {
#else
		rc = check_head(disp_tag, &disp_nbytes);

		if (rc > 0) { 

			rc = get_data(disp_buf, DISP_BUFSIZ);

			// if we have a bad read, let's set the (dispatcher) 
			// statuscheck to 'bad' and get out of the subloop to 
			// start up the dispatcher-reconnect cycle.
			if( rc < 0 ) {
				statuscheck = -1;
				statusdata = 0;
		    	status_send_time = 1;
				Printf("---BAD READ--- Attempting to reconnect\n");
				InitDispLink(data,1);
				CheckDisplayTime(data,FALSE);
				return;
			}
			
			// we got some data, so the dispatcher must be OK
			if (status_send_time) {
				// print "OK" message if we started decrementing status check
				if (statuscheck != STATUS_COUNT) {
					Printf("+++DISPATCHER OK+++\n");
					statuscheck = STATUS_COUNT;
				}
				// reset status send time
				status_send_time = 0;
			}

#endif
			last_recv_time = time(NULL);
			
			// Print an error and ignore the packet if it exceeds the maximum size
			// (The packet was truncated when we read it,
			//  so if it wasn't bad before it certainly is now...)
			if ( disp_nbytes > DISP_BUFSIZ ) {
			    Printf("Event too large!! -- buffer overrun!\n");
			    continue;
			}

			//  If Dispatcher has 'RAWDATA' ie- event built data--
			// go to the rawdata decode loop.
			if (!strcmp(disp_tag, "RAWDATA")) 
			{
				dataless_count = 0;
				// we must keep reading all dispatched data furiously so we
				// don't get too far behind -- the dispatcher will heavily buffer
				// the data and we will be viewing the distant past.
				// So, only process events if we are not throwing out data in this mode.
				if (data->wDataType!=IDM_CMOS_RATES && decode_rawdata(data)) {
					// start throwing out data from now on
					data->throw_out_data = 1;
					// set time out which will reset throw_out_data
					ProcessEvents(data,PROCESS_THROW_OUT, 0);
					// note:  will continue on with work proc now...
				}
			} 	
			  
			// 'CMOSDATA' is data read in by DAQ and sent down the 
			// stream which includes CMOS rates for each channel in a crate
			else if (!strcmp(disp_tag, "CMOSDATA")) 
			{
				dataless_count = 0;
				if (data->wDataType == IDM_CMOS_RATES) {
					decode_cmos(data);
				}
			}
			
			// 'STATUS' tags indicate a monitoring status event-- 
			// currently only includes the heartbeat of a few monitoring 
			// tools... I use it here to determine the livelyhood of the 
			// dispatched data stream.
			else if (!strcmp(disp_tag, "STATUS")) 
			{
				decode_status();
			}    

			// 'RECHDR' tags indicate a record header
			else if (!strcmp(disp_tag, "RECHDR"))
			{
				decode_rechdr(data);
			}
			else if (!strcmp(disp_tag, "SCOPEDAT"))
			{
				dataless_count = 0;
				decode_scopedat(data);
			}
			
		} else {	// we didn't have an event waiting for us from dispatcher...

#ifndef USE_PTHREAD
			// Did we have a dispatcher communication problem?
			if( rc < 0 ) {
		  		// If so, set the dispatcher status-bit to 'pooched'
		  		statuscheck = -1;
		    	statusdata = 0;
		    	status_send_time = 1;
			}

			// Since we don't have data now either, we check the system time.
			cur_time = time('\0');
			
			if (status_send_time) {
				if (cur_time - status_send_time >= STATUS_DEAD_TIME) {
					Printf("***DISPATCHER LINK NOT RESPONDING***\n");
					status_send_time = 0;
					if (--statuscheck < 0) {
						rc=whereis(disp_hostname,"XSNOED",NULL,0);
						if (rc<1) {
					  		// If the willful-disconnect flag is on, we've 
					  		// disconnected as per request of the operator, 
					  		// so we write this on the display and continue.
					  		if (dispconnectstatus==-1) {
					    		CloseDispLink();
					  		} else {
					  			// If not, then we are disconnected without consent, 
					  			// and we will try to reconnect until the operator 
					  			// specifically requests a cancel.
					    		Printf("***ATTEMPTING TO RECONNECT***\n");
					    		InitDispLink(data,1);
					  		}
						}
					}
				}
			}
			// has it been more than N seconds since we have seen some data?
			if (!status_send_time &&
				cur_time - last_recv_time >= STATUS_CHECK_TIME)
			{
				// Send out a heartbeat to dispatcher to verify its existance
				statusout=2;
				position = 0;

				rc = put_data ("STATUS",&statusout,4,&position);

				status_send_time = cur_time;
			}
#endif
			if (dataless_count > MAX_DATALESS) {
				CheckDisplayTime(data,FALSE);
				// be nice if we haven't seen data in a while
				ProcessEvents(data,PROCESS_TIME_OUT, NICE_WAIT_MS);
				return;
			}
			break;   // no data, so stop polling dispatcher for a while
		}
    }
	CheckDisplayTime(data,FALSE);
	
    // poll again as soon as possible
    ProcessEvents(data,PROCESS_WORK_PROC, 0);
#endif // NO_DISPATCH
}


#ifndef NO_DISPATCH // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Decode_status
// A program which decodes the 'STATUS' tag from the dispatcher-- currently,
// rather minimal...
void decode_status ( void )
{
  SWAP_INT32(disp_buf, 1);
  
  // copy the buffer into a global variable, and carry the message back home.
  statusdata=disp_buf[3];

}


// Program decode_rawdata
// Takes dispatcher data under the tag 'RAWDATA' and interfaces it to 
// xsnoed, shipping off the relevant pieces to the proper areas.
int decode_rawdata(ImageData *data)
{
	int	did_display = 0;
  	aGenericRecordHeader *theHeader;
	aPmtEventRecord *thePmtRecord; 
	char *buffer = disp_buf;
	u_int32 the_gtid;

	SWAP_INT32(buffer, 3);

	// Read the generic record header, which tells us what type of
	// standard record is included.
	theHeader = (aGenericRecordHeader *)buffer;

	// Increment the buffer pointer beyond the header to the data.
	buffer += sizeof( aGenericRecordHeader );

	// Depending on what kind of information is said to be included
	// in this data structure...
	switch (theHeader->RecordID) {

		// If we have a run_record, that is, a record detailing the type of
		// run and the conditions of the detector...
		case( RUN_RECORD):

			break;


		// If we have a standard event record...
		case( PMT_RECORD): {

			// check to see if this is going into 'future' history buffer
			// and make sure there is enough room in buffer if it is.
			if (data->throw_out_data && data->history_size[HISTORY_FUTURE]>=FUTURE_SIZE) {
//				data->lost_future = 1;	// lost a future event due to full buffer
				break;
			}
	
		    // The start of the PMT record carries the global event data.
		    thePmtRecord = (aPmtEventRecord *)buffer;
		    
#ifdef SWAP_BYTES		    
		    // swap the structure
		    SWAP_PMT_RECORD(thePmtRecord);
		
		    // the actual PMT data is contained after the global data.
		    buffer += sizeof( aPmtEventRecord );
		
			// byte-swap all PMT hits
			SWAP_INT32( (unsigned long *)buffer, 3 * thePmtRecord->NPmtHit ); 
		
			// swap the sub-fields
			u_int32	*sub_header = &thePmtRecord->CalPckType;
			while (*sub_header & SUB_NOT_LAST) {
				sub_header += (*sub_header & SUB_LENGTH_MASK);
				SWAP_INT32( sub_header, 1 );	// swap the sub-field header
				// swap the data (-1 because we don't want to include header size)
				SWAP_INT32( sub_header+1, (*sub_header & SUB_LENGTH_MASK) - 1 );
			}
#endif
			// save last dispatched non-orphan GTID
			the_gtid = thePmtRecord->TriggerCardData.BcGT;
			if (the_gtid) disp_last_gtid = the_gtid;
			
			// pass the event to xsnoed
			if (xsnoed_filter(data, thePmtRecord)) {
				did_display = xsnoed_event(data, thePmtRecord);
				if (!did_display) {
					CheckDisplayTime(data,TRUE);
				}
			}
			
		}  	break; // End of case PMT_RECORD
 

		// If the data is carrying trigger card information...
		case( TRIG_RECORD):

			// ALL contents disabled due to lack of use.
			break;

   
    	// If the data contained is an elect. pedestal...
  		case( EPED_RECORD):
    
    		break;
    

    	// If the data type has never been seen or heard of before.
  		default:
    		Printf("Unknown record type %d passed in RAWDATA.\n", theHeader->RecordID);
    		break;
	} //END of record ID switch.

	return(did_display);
}

// Decode "RECHDR" packets
void decode_rechdr(ImageData *data)
{
  	aGenericRecordHeader *theHeader;
	char 				 *buffer = disp_buf;

	SWAP_INT32(buffer, 3);	// swap the generic record header

	// Read the generic record header, which tells us what type of
	// standard record is included.
	theHeader = (aGenericRecordHeader *)buffer;

	// Increment the buffer pointer beyond the header to the data.
	buffer += sizeof( aGenericRecordHeader );
	
	if (PZdabFile::GetVerbose()) {
		PZdabFile::DumpRecord((u_int32 *)(theHeader + 1), theHeader->RecordLength,
								theHeader->RecordID, disp_last_gtid);
	}
	// swap the record into native format
	SWAP_INT32(buffer, theHeader->RecordLength / sizeof(u_int32));
	
	// handle the header record
	handleHeaderRecord(data, (u_int32 *)buffer, theHeader->RecordID);
}

void decode_scopedat(ImageData *data)
{
    // put a copy of the data in the ImageData structure 
    // (use memcpy instead of assignment because the floating point values
    // may not yet make sense if loaded into a floating point register)
    if (sizeof(ScopeData) == SCOPE_DATA_SIZE) {
        memcpy(&data->scopeData, disp_buf, sizeof(ScopeData));
    } else {
        // copy all elements individually since alignment is different
        // (this is a quick hack, and should be fixed more elegantly - PH 10/16/03)
        int n = 0;
        ScopeData *sd = &data->scopeData;
        memcpy(&sd->acquireStopafter[0], disp_buf+n, 32); n+=32;
        memcpy(&sd->acquireState, disp_buf+n, sizeof(int)); n+=sizeof(int);
        memcpy(&sd->acquireMode[0], disp_buf+n, 32); n+=32;
        memcpy(&sd->acquireNumenv, disp_buf+n, sizeof(int)); n+=sizeof(int);
        memcpy(&sd->acquireNumavg, disp_buf+n, sizeof(int)); n+=sizeof(int);
    
       // channel
        memcpy(&sd->voltSteps[0], disp_buf+n, 256*sizeof(double)); n+=256*sizeof(double);
        memcpy(&sd->numVoltSteps, disp_buf+n, sizeof(int)); n+=sizeof(int);
        memcpy(&sd->channelScale[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(double)); n+=SCOPEMAXCHANNELS*sizeof(double);
        memcpy(&sd->channelPosition[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(double)); n+=SCOPEMAXCHANNELS*sizeof(double);
        memcpy(&sd->channelOffset[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(double)); n+=SCOPEMAXCHANNELS*sizeof(double);
        memcpy(&sd->channelCoupling[0][0], disp_buf+n, SCOPEMAXCHANNELS*32); n+=SCOPEMAXCHANNELS*32;
        memcpy(&sd->channelBandwidth[0][0], disp_buf+n, SCOPEMAXCHANNELS*32); n+=SCOPEMAXCHANNELS*32;
    
       // time
        memcpy(&sd->timeScale, disp_buf+n, sizeof(double)); n+=sizeof(double);
    
       // trigger
        memcpy(&sd->triggerMode[0], disp_buf+n, 32); n+=32;
        memcpy(&sd->triggerType[0], disp_buf+n, 32); n+=32;
        memcpy(&sd->triggerLevel, disp_buf+n, sizeof(double)); n+=sizeof(double);
        memcpy(&sd->triggerHoldoff, disp_buf+n, sizeof(double)); n+=sizeof(double);
        memcpy(&sd->triggerSource[0], disp_buf+n, 32); n+=32;
        memcpy(&sd->triggerCoupling[0], disp_buf+n, 32); n+=32;
        memcpy(&sd->triggerSlope, disp_buf+n, sizeof(int)); n+=sizeof(int);
        memcpy(&sd->triggerSlopeString[0], disp_buf+n, 32); n+=32;
       
        memcpy(&sd->channelFlag[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
        memcpy(&sd->activeChannels[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
        memcpy(&sd->numberOfActiveChannels, disp_buf+n, sizeof(int)); n+=sizeof(int);
    
       
       // === Waveform data ===
    
        memcpy(&sd->vDiv[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(float)); n+=SCOPEMAXCHANNELS*sizeof(float);
        memcpy(&sd->tDiv[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(float)); n+=SCOPEMAXCHANNELS*sizeof(float);
    
        memcpy(&sd->size[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
        memcpy(&sd->wave[0][0], disp_buf+n, SCOPEMAXCHANNELS*WAVEFORMSIZE); n+=SCOPEMAXCHANNELS*WAVEFORMSIZE;
    
       // conversion constants
        memcpy(&sd->npoints[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
        memcpy(&sd->yoff[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(float)); n+=SCOPEMAXCHANNELS*sizeof(float);
        memcpy(&sd->ymult[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(float)); n+=SCOPEMAXCHANNELS*sizeof(float);
        memcpy(&sd->xincr[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(float)); n+=SCOPEMAXCHANNELS*sizeof(float);
        memcpy(&sd->pointoff[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
    
        memcpy(&sd->nvchperdiv[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
        memcpy(&sd->ntchperdiv[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
    
        memcpy(&sd->channel[0], disp_buf+n, SCOPEMAXCHANNELS*sizeof(int)); n+=SCOPEMAXCHANNELS*sizeof(int);
        memcpy(&sd->eventNumber, disp_buf+n, sizeof(int)); n+=sizeof(int);
        if (n != SCOPE_DATA_SIZE) printf("Eeek!  Scope data wrong size (%d)\n", n);
    }
#ifdef SWAP_BYTES
    // get a pointer to the ScopeData data
    ScopeData *scopeData = &data->scopeData;
    
    SWAP_INT32(&scopeData->acquireState, 1);
    SWAP_INT32(&scopeData->acquireNumenv, 1);
    SWAP_INT32(&scopeData->acquireNumavg, 1);
    SWAP_DOUBLE(scopeData->voltSteps, 256);
    SWAP_INT32(&scopeData->numVoltSteps, 1);
    SWAP_DOUBLE(scopeData->channelScale, SCOPEMAXCHANNELS);
    SWAP_DOUBLE(scopeData->channelPosition, SCOPEMAXCHANNELS);
    SWAP_DOUBLE(scopeData->channelOffset, SCOPEMAXCHANNELS);
    SWAP_DOUBLE(&scopeData->timeScale, 1);
    SWAP_DOUBLE(&scopeData->triggerLevel, 1);
    SWAP_DOUBLE(&scopeData->triggerHoldoff, 1);
    SWAP_INT32(&scopeData->triggerSlope, 1);
    SWAP_INT32(&scopeData->channelFlag, SCOPEMAXCHANNELS);
    SWAP_INT32(&scopeData->activeChannels, SCOPEMAXCHANNELS);
    SWAP_INT32(&scopeData->numberOfActiveChannels, 1);
    SWAP_FLOAT(scopeData->vDiv, SCOPEMAXCHANNELS);
    SWAP_FLOAT(scopeData->tDiv, SCOPEMAXCHANNELS);
    SWAP_INT32(scopeData->size, SCOPEMAXCHANNELS);
    SWAP_INT32(scopeData->npoints, SCOPEMAXCHANNELS);
    SWAP_FLOAT(scopeData->yoff, SCOPEMAXCHANNELS);
    SWAP_FLOAT(scopeData->ymult, SCOPEMAXCHANNELS);
    SWAP_FLOAT(scopeData->xincr, SCOPEMAXCHANNELS);
    SWAP_INT32(scopeData->pointoff, SCOPEMAXCHANNELS);
    SWAP_INT32(scopeData->nvchperdiv, SCOPEMAXCHANNELS);
    SWAP_INT32(scopeData->ntchperdiv, SCOPEMAXCHANNELS);
    SWAP_INT32(scopeData->channel, SCOPEMAXCHANNELS);
    SWAP_INT32(&scopeData->eventNumber, 1);
   
#endif

    // send message indicating new scope data is available
    data->mSpeaker->Speak(kMessageNewScopeData);
    
#ifdef DUMP_SCOPE_DATA
      
    int ich;
    int i;

    printf("  decode_scopedat()\n");
    printf("    disp_tag    =<%s>\n",disp_tag);
    printf("    disp_nbytes = %d\n",disp_nbytes);
    
    // *****************************************
    // Print out vars
    
    // aquire
    printf("   // aquire variables\n");
    printf("   char scopeData.acquireStopafter: <%s>\n", data->scopeData.acquireStopafter);
    printf("   int  scopeData.acquireState:     <%d>\n", data->scopeData.acquireState);
    printf("   char scopeData.acquireMode:      <%s>\n", data->scopeData.acquireMode);
    printf("   int  scopeData.acquireNumenv:    <%d>\n", data->scopeData.acquireNumenv);
    printf("   int  scopeData.acquireNumavg:    <%d>\n", data->scopeData.acquireNumavg);
    
    // channel
    printf("\n   // channel variables\n");
    printf("   double scopeData.voltSteps[1]:        <%lf>\n", data->scopeData.voltSteps[1]);
    printf("   int    scopeData.numVoltSteps:        <%d>\n",  data->scopeData.numVoltSteps);
    for(i=0;i<SCOPEMAXCHANNELS;i++){
      printf("   double scopeData.channelScale[%d]:     <%lf>\n",i, data->scopeData.channelScale[i]);
      printf("   double scopeData.channelPosition[%d]:  <%lf>\n",i, data->scopeData.channelPosition[i]);
      printf("   double scopeData.channelOffset[%d]:    <%lf>\n",i, data->scopeData.channelOffset[i]);
      printf("   char   scopeData.channelCoupling[%d]:  <%s>\n", i, data->scopeData.channelCoupling[i]);
      printf("   char   scopeData.channelBandwidth[%d]: <%s>\n", i, data->scopeData.channelBandwidth[i]);
    } 
    // time
    printf("\n   // time variables\n");
    printf("   double scopeData.timeScale:  <%lf>\n", data->scopeData.timeScale);
    
    // trigger
    printf("\n   // trigger variables\n");
    printf("   char   scopeData.triggerMode:        <%s>\n",  data->scopeData.triggerMode);
    printf("   char   scopeData.triggerType:        <%s>\n",  data->scopeData.triggerType);
    printf("   double scopeData.triggerLevel:       <%lf>\n", data->scopeData.triggerLevel);
    printf("   double scopeData.triggerHoldoff:     <%lf>\n", data->scopeData.triggerHoldoff);
    printf("   char   scopeData.triggerSource:      <%s>\n",  data->scopeData.triggerSource);
    printf("   char   scopeData.triggerCoupling:    <%s>\n",  data->scopeData.triggerCoupling);
    printf("   int    scopeData.triggerSlope:       <%d>\n",  data->scopeData.triggerSlope);
    printf("   char   scopeData.triggerSlopeString: <%s>\n",  data->scopeData.triggerSlopeString);
    
    // more channel stuff
    printf("\n   // more channel stuff\n");
    for(i=0;i<SCOPEMAXCHANNELS;i++){
      printf("   int scopeData.channelFlag[%d]:         <%d>\n", i,data->scopeData.channelFlag[i]);
    }
    for(i=0;i<SCOPEMAXCHANNELS;i++){
      printf("   int scopeData.activeChannels[%d]:      <%d>\n", i,data->scopeData.activeChannels[i]);
    }
    printf("   int scopeData.numberOfActiveChannels: <%d>\n", data->scopeData.numberOfActiveChannels);
    
    // waveform vars
    printf("\n   // Waveform variables\n");
    for (i=0; i < data->scopeData.numberOfActiveChannels; i++) {
        ich = data->scopeData.activeChannels[i]-1;
        printf("   float scopeData.vDiv[%d]:           <%lf>\n",ich, data->scopeData.vDiv[ich]);
        printf("   float scopeData.tDiv[%d]:           <%lf>\n",ich, data->scopeData.tDiv[ich]);
        
        printf("   int scopeData.size[%d]:             <%d>\n", ich, data->scopeData.size[ich]);
        
        printf("   int   scopeData.npoints[%d]:        <%d> \n",  ich, data->scopeData.npoints[ich]);
        printf("   float scopeData.yoff[%d]:           <%lf>\n",  ich, data->scopeData.yoff[ich]);
        printf("   float scopeData.ymult[%d]:          <%lf>\n",  ich, data->scopeData.ymult[ich]);
        printf("   float scopeData.xincr[%d]:          <%lf>\n",  ich, data->scopeData.xincr[ich]);
        printf("   int   scopeData.pointoff[%d]:       <%d> \n",  ich, data->scopeData.pointoff[ich]);
        
        printf("   int scopeData.nvchperdiv[%d]:       <%d> \n",  ich, data->scopeData.nvchperdiv[ich]);
        printf("   int scopeData.ntchperdiv[%d]:       <%d> \n",  ich, data->scopeData.ntchperdiv[ich]);
        printf("   int scopeData.channel[%d]:          <%d> \n",  ich, data->scopeData.channel[ich]);
        printf("   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");         
    }
    
    printf("   int data->scopeData.eventNumber:  <%d>\n", data->scopeData.eventNumber);
    printf("\n-------------------------------\n");
      
#endif // DUMP_SCOPE_DATA

} // end decode_scopedat()





// Program decode_cmos
// Takes cmos data broadcasted by DAQ and processes.
void decode_cmos(ImageData *data)
{
  int		i,j,crate;
  float		*datap;
  int32		rates[NUM_CRATE_CARDS][NUM_CARD_CHANNELS];
  unsigned long *lbufp;
  struct CmosCrateHeader *ccheader;
  struct CmosBoardHeader *cbheader;
  struct CmosChannelData *ccdata;

  // assign a pointer to the address of the dispatcher-passed buffer.
  lbufp = (unsigned long *) &disp_buf;
  
  SWAP_INT32(lbufp, 1);	// swap the CmosCrateHeader structure (bit packed 32 bit word)

  // The first chunk of data of CMOSDATA should be the crateheader, so fill it.
  ccheader = (struct CmosCrateHeader *)lbufp;

  // Which crate has info that is being sent?
  crate = (int)ccheader->CrateID;
  
  // do nothing if crate out of range
  if ((unsigned)crate >= NUM_SNO_CRATES) return;
  
  // Index the pointer past the crate header.
  ++lbufp;

  // If the datatype entry is == 1, then we have partial channel rate data.
  if (ccheader->DataType == 1) {

		// do nothing if data for no boards
		if (!ccheader->BoardMask) return;
		
		// initialize rates to -1 for this crate
		memset(rates,-1,sizeof(rates));
		
		// For every pontential card in the crate...
		for (i=0; i<16; i++) {

			// If the data claims to have data for this card/board...
			if (((ccheader->BoardMask)>>i) & 0x01) {

				SWAP_INT32(lbufp, 1);	// swap the CMOS board header
		      
				// Set the board pointer to the proper location...
				cbheader = (struct CmosBoardHeader *)lbufp;
				
				++lbufp;

				// for every potential channel on the board...
				for (j=0; j<32; j++) {

					// If the data claims to have info for that channel...
					if (((cbheader->ChannelMask) >> j) & 0x01) {

						// swap the rate data
						SWAP_INT32(lbufp, 1);

						// Take the information from that channel
						ccdata = (struct CmosChannelData *)lbufp;

						// And copy the data into the CMOS rate array.
						rates[i][j] = (int32)ccdata->rate;

						// Finally, step on to the next channel
						++lbufp;
						
					}  // End of active channel routine
					
				} // End of total channel loop.

			} // End of active board routine.
			
		} // End of total board loop.
      

	// If ccheader->DataType is 2, then we have a full crate of data.
	} else if (ccheader->DataType == 2) {
      
		datap = (float *)lbufp;
		
		SWAP_INT32(datap, NUM_CARD_CHANNELS*NUM_CRATE_CARDS);
		
		for (i=0; i<NUM_CRATE_CARDS; i++) {
			for (j=0; j<NUM_CARD_CHANNELS; j++){
				rates[i][j] = (u_int32) *(datap++);
			}
		}
		
	} else {
	
		return;		// no good data
	}
	
	// send the rates to xsnoed
	xsnoed_rates(data, crate, &rates[0][0]);
	
	// display rates in due time
	CheckDisplayTime(data,TRUE);
}

// close the dispatcher connection and reset the necessary variables
static void CloseDispatcher(void)
{
	if (isConnected) {
		if (dispatcher_data && dispatcher_data->input_source==INPUT_DISPATCHER) {
			dispatcher_data->input_source = INPUT_NONE;
		} else {
			Printf("Internal error in CloseDispatcher()\n");
		}
	}
	dispconnectstatus = -1;
	drop_connection();
	isConnected = 0;
}

// CloseDispLink_Now
// Close dispatcher link now
static void CloseDispLink_Now(void)
{
	if (isConnected) {
		CloseDispatcher();
		Printf("---DISPATCHER DISCONNECTED---\n");
	} else if (retryTime) {
		Printf("---GIVING UP ON DISPATCHER CONNECTION---\n");
	}
	retryTime = 0;
}

#ifdef USE_PTHREAD
// Tell dispatcher thread to close link
static void CloseDispLink_InThread()
{
	if (isConnected) {
		Printf("Closing dispatcher link...\n");
		/* wait until we can send our disconnect message */
		while (do_thread_command(THREAD_CMD_DISCONNECT)) { }
		/* wait until our disconnect is done */
		while (!thread_command_complete()) { }
		if (isConnected) {
			Printf("Error closing dispatcher link\n");
		}
	} else {
		// print any messages and reset necessary variables
		CloseDispLink_Now();
	}
	retryTime = 0;
}
#endif

// CloseDispLink
// Closes link to dispatcher
// (use this routine from foreground thread)
void CloseDispLink(void)
{
#ifdef USE_PTHREAD
	// close the dispatcher link
	CloseDispLink_InThread();
	// kill the thread
	if (thread_running) {
		/* send DIE command */
		while (do_thread_command(THREAD_CMD_DIE)) { }
		/* wait until thread is dead */
		while (!thread_command_complete()) { }
		if (thread_running) {
			Printf("Error stopping dispatcher thread!\n");
//		} else {
//			Printf("Dispatcher thread stopped\n");
		}
	}
#else
	CloseDispLink_Now();
#endif
}

// InitDispLink_Now
// Tries to connect Xsnoed to a dispatcher on the specified client.
static void InitDispLink_Now(ImageData *data)
{
  int rc;

  // Drop any current connections to any dispatchers.
  if (isConnected) {
  	CloseDispLink_Now();
  	sleep(1);	// wait a bit after disconnecting before reconnecting
  }

  // Try to connect, with 'w' flags (data receipt verification NOT nec)
  rc = init_disp_link(disp_hostname,"w RAWDATA w CMOSDATA w STATUS w RECHDR w SCOPEDAT"); 

  // If the attempt to connect has come across with a positive response...
  if( rc >= 0 ) {
    Printf("+++DISPATCHER CONNECTED+++\n");
    data->mEventFile.Release();	/* no input file */
    isConnected = 1;
    data->input_source = INPUT_DISPATCHER;
	dataless_count = 0;
	disp_last_gtid = 0;	// reset last GTID
    statuscheck = STATUS_COUNT;
	dispconnectstatus = 0;
	last_recv_time = time(NULL);
	status_send_time = 0;
    // Tell the dispatcher that the program will always be ready for
    // any information that it has the time to send.
    rc = send_me_always();

    // Tell the dispatcher that I like to be called 'XSNOED'
    rc = my_id("XSNOED");
    
    retryTime = 0;
  
  } else if (retryConnection) {
  	if (!retryTime) {
  		Printf("***COULDN'T CONNECT TO %s - Will keep trying***\n",disp_hostname);
  	}
  	// set flag to try reconnecting later
  	retryTime = time(NULL) + RETRY_CONNECT_TIME;
  } else {
  	Printf("***COULDN'T CONNECT TO %s***\n",disp_hostname);
  }

}// End InitDispLink_Now

// InitDispLink
// Inits link to dispatcher
// (use this routine from foreground thread)
void InitDispLink(ImageData *data,int keep_trying)
{
	if (isConnected && dispatcher_data!=data) {
		// transfer connection to this ImageData object
		if (dispatcher_data) {
			data->input_source = dispatcher_data->input_source;
			dispatcher_data->input_source = INPUT_NONE;
		}
	}
	dispatcher_data = data;
	retryConnection = keep_trying;
	
#ifdef USE_PTHREAD
	pthread_attr_t attr;
	
	// close event file or dispatcher connection if open
	// Note: only closes dispatcher connection if opened by this ImageData object
	close_event_file(data);
	
	// start the dispatcher thread if necessary
	if (!thread_running) {
		thread_running = 1;
		thread_command = THREAD_CMD_NULL;
		thread_response_count = thread_command_count;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
		pthread_create(&tid, &attr, &do_dispatcher_thread, data);
//		Printf("Dispatcher thread started\n");
	}
	
	// must close link in case opened by another ImageData object
	if (isConnected) {
		CloseDispLink_InThread();
	}
	retryTime = 0;	// make sure retry time is reset
	
	Printf("Opening dispatcher link to %s...\n",disp_hostname);
	/* wait until we can send our disconnect message */
	while (do_thread_command(THREAD_CMD_CONNECT)) { }
	/* wait until our connect is done */
	while (!thread_command_complete()) { }
//	if (!isConnected) {
//		Printf("Error opening dispatcher link\n");
//	} else {
//		Printf("Dispatcher link opened\n");
//	}
#else
	// close event file if open
	close_event_file(data);
	InitDispLink_Now(data);
#endif
  
  	if (isConnected) {
		// start continuous trigger by default when open connection
		setTriggerFlag(data,TRIGGER_CONTINUOUS);
	}

    // add our work proc to receive the dispatcher data
    ProcessEvents(data,PROCESS_TIME_OUT, 0);
}


// IsDispConnected
// returns non-zero if the dispatcher is connected
int IsDispConnected(ImageData *data)
{
	return(isConnected && dispatcher_data==data);
}

#endif // NO_DISPATCH ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef ROOT_FILE
void root_open_tree(ImageData *data,QTree *aTree)
{
	if (root_file_ptr && (aTree == root_tree)) {
		// zero file pointer to prevent QTree file from being closed
		data->infile = 0;
		// reset the event list if it existed
		root_event_list = NULL;
	} else {
		// close existing file if open (must do this before setting root_tree)
		close_event_file(data);
		// save QTree pointer
		root_tree = aTree;
	}
	// set the file name
	data->mEventFile.SetString("QTree");
	
	// open the tree
	open_event_file(data,0);
}
void root_open_event_list(ImageData *data,TEventList *aList, QTree *aTree)
{
	if (!aList || !aList->GetN()) {
		Printf("Event list is empty\n");
	} else {
		// check to see if the specified Tree is from our currently open file
		if (root_file_ptr && aTree==root_tree) {
			// zero aTree so we don't close our file
			aTree = NULL;
		}
		// close existing file if aTree is specified
		if (aTree) {
			// new QTree is specified -- close old file
			close_event_file(data);
			// save new QTree pointer
			root_tree = aTree;
		} else if (!root_tree) {
			Printf("Must specify QTree to view a TEventList\n");
			return;
		} else {
			Printf("Using existing QTree for event list\n");
			// zero file pointer to prevent QTree file from being closed
			data->infile = 0;
		}
		
		// save TEventList pointer
		root_event_list = aList;
		
		// set the file name
		data->mEventFile.SetString("TEventList");
		
		// open the list
		open_event_file(data,0);
	}
}
#endif

#ifdef ROOT_FILE
void root_update_rch_windows(ImageData *data, int hit_num)
{
	if (hit_num < 0) return;
	
	int pmtn = data->hits.hit_info[hit_num].index;
	
	if (data->rch_pmtn != pmtn) {
		data->rch_pmtn = pmtn;
		for (int i=RCH_TIME_WINDOW; i<=RCH_QLX_WINDOW; ++i) {
			if (!data->mWindow[i]) continue;
			QRchHist *hist = (QRchHist *)((PImageWindow *)data->mWindow[i])->GetImage();
			hist->ReadRchData();
			hist->Draw();
		}
	}
}
#endif

/* 
** open_event_file - open zdab or root event file specified by data->mEventFile
*/
void open_event_file(ImageData *data, int try_dispatch)
{
	char *file_type = "zdab file";
	
	// close event file and dispatcher link if they are open
	close_event_file(data);
	
	// clear 'all' history
	clearHistoryAll(data);
	
	char *filename = data->mEventFile.GetString();
	char *pt = strrchr(filename,'/');
	if (pt) ++pt;
	else pt = filename;

#ifdef ROOT_FILE
	if (strstr(pt,"root")) {
		// initialize root if not done already
		create_root_system();
		file_type = "root file";
		root_file_ptr = new TFile(filename,"READ");
		if (root_file_ptr->IsOpen()) {
			/* set QTree pointer */
			root_tree = (QTree *)root_file_ptr->Get("T");
			if (root_tree) {
				data->input_source = INPUT_ROOT_FILE;
				data->infile = ROOT_FILE_FLAG;
				root_event_num = -1;
			} else {
				Printf("No QTree in file!\n");
			}
		} else {
			delete root_file_ptr;
			root_file_ptr = NULL;
		}
	} else if (root_event_list || root_tree) {
		create_root_system();
		file_type = "root object";
		// we are opening a root TEventList or QTree
		data->infile = ROOT_FILE_FLAG;
		root_event_num = -1;
		if (root_event_list) {
			data->input_source = INPUT_ROOT_EVENT_LIST;
		} else {
			data->input_source = INPUT_ROOT_TREE;
		}
	} else {
#endif
		data->infile = fopen(filename,"rb");
		if (data->infile) {
			data->mZdabFile->Init(data->infile);
			data->input_source = INPUT_ZDAB_FILE;
		}
#ifdef ROOT_FILE
	}
#endif
	
	if (data->infile) {
		Printf("Opened %s: %s\n",file_type,filename);
    	// initialize non sequential event counter
    	data->non_seq_file = 0;
    	data->require_rewind = 0;	// reset rewind flag
		data->last_seq_gtid = 0;	// reset last sequential event id
   		// clear old events from display
 //   	xsnoed_clear(data);
		setTriggerFlag(data,TRIGGER_SINGLE);
    	// add our work proc to load the first event
    	ProcessEvents(data,PROCESS_TIME_OUT, 0);
    	
	} else if (strstr(pt,"zdab") || strstr(pt,"root") || !try_dispatch) {
		Printf("Error opening %s %s\x07\n",file_type,filename);
	} else {
#ifndef NO_DISPATCH
		SetDispName(filename);
		InitDispLink(data,0);
#endif
	}
}

/*
** close_event_file - close zdab or root event file
*/
void close_event_file(ImageData *data)
{
	// close dispatcher link if open
#ifndef NO_DISPATCH
	if (data==dispatcher_data) {
		if (isConnected) CloseDispLink();
		retryTime = 0;
	}
#endif

	// close file if open
	if (data->infile) {
#ifdef ROOT_FILE
		if (data->infile == ROOT_FILE_FLAG) {
			if (root_file_ptr) {
				delete root_file_ptr;
				Printf("Closed root file\n");
			} else if (root_event_list) {
				Printf("Closed root Event List\n");
			} else {
				Printf("Closed root QTree\n");
			}
			root_file_ptr = NULL;
			root_tree = NULL;
			root_event_list = NULL;
			if (root_pmtRecord) {
				root_pmtRecord->SetPmtEventRecord(NULL);	// free pmtRecord buffer
			}

		} else {
#endif
			fclose(data->infile);
			Printf("Closed zdab file\n");
#ifdef ROOT_FILE
		}
#endif
		data->infile = 0;
		data->input_source = INPUT_NONE;
	}
	handleHeaderRecord(data,NULL,0);	// clear any existing run record
}

/*
** rewind_event_file - rewind event file to start
*/
void rewind_event_file(ImageData *data)
{
	FILE	*fp;
	int		subrun;
	int		print_msg = 1;
	
	if (data->infile) {
#ifdef ROOT_FILE
		if (data->infile == ROOT_FILE_FLAG) {
			root_event_num = -1;
		} else {
#endif
			subrun = zdab_get_subrun(data->mEventFile.GetString());
			if (subrun > 0) {
				/* try to open subrun number 000 */
				char filename[FILELEN];
				strcpy(filename, data->mEventFile.GetString());
				zdab_set_subrun(filename, 0);
				fp = fopen(filename,"rb");
				if (fp) {
					/* change file name */
					data->mEventFile.SetString(filename);
					/* success! */
					Printf("Opened zdab file: %s\n", filename);
					/* close the old file */
					fclose(data->infile);
					/* make the new file current */
					data->infile = fp;
					/* don't print message if we just printed "open file" message */
					print_msg = 0;
				}
			}
			/* rewind to start of file and init zdab for reading */
	   		data->mZdabFile->Init(data->infile);
#ifdef ROOT_FILE
	   	}
#endif
   	}
	if (print_msg) Printf("Rewind to start of event file\n");
	
	data->last_seq_gtid = 0;	// reset last sequential event id
	data->require_rewind = 0;	// reset rewind flag
	
	// our 'future' history is no longer valid -- clear it.
	clearHistoryAll(data);
}

/*
** xsnoed.root main()
*/
#ifdef ROOT_APP
int main(int argc, char *argv[])
{
	extern char *progname;
	extern char *progpath;
	
	g_argc = argc;
	g_argv = argv;
	
	/* get pointer to program name */
	progpath = g_argv[0];
	progname = strrchr(g_argv[0],'/');
	if (progname) ++progname;
	else progname = g_argv[0];

	create_root_system();
	
#ifdef R__UNIX
	gSystem = new QXsnoedSystem;	// install XSnoed system
#endif

    TRint *theApp = new TRint("XSnoed ROOT", &g_argc, g_argv);

    gSnoed = new QSnoed;

	printf("Use gSnoed (global QSnoed object) to interact with the Event Display\n");

    // Init Intrinsics, build all windows, and enter event loop
    theApp->Run(TRUE);
	
	printf("\n");
	
	delete gDispatch;	// delete global dispatcher to avoid pipe errors

    printf("\ndone!\n");
}
#endif

#ifdef ROOT_FILE
void do_root(ImageData *data)
{
	int argc = 0;
	
	create_root_system();
	
#ifdef R__UNIX
	if (gSystem->IsA() != QXsnoedSystem::Class()) {
		QXsnoedSystem *sys = new QXsnoedSystem;	// install XSnoed system
		// we want ROOT to quit when we select "Quit" from the xsnoed menu
		sys->SetExitRootWithXsnoed(TRUE);
		gSystem = sys;
	}
#endif

    TRint *theApp = new TRint("XSnoed ROOT", &argc, &data->argv);
    
    gSnoed = new QSnoed(data);

	printf("Use gSnoed (global QSnoed object) to interact with the Event Display\n");
	
    // Init Intrinsics, build all windows, and enter event loop
    theApp->Run(FALSE);
}
#endif

/*
** load_event -  go to event with specified GTID/Run/Time in the event file
*/
void load_event(ImageData *data, long target, int loadBy)
{
	long				current, diff;
	int					rewound = 0;
	int					found_good = 0;
//	const long			kOneDay = 24 * 3600L;	// a day's worth of seconds
	
	data->throw_out_data = 0;
	
	aPmtEventRecord *thePmtRecord = 0;	// set to zero to avoid compiler warning on Sun
	
	if (!data->infile) return;

	/* get the currently display event ID or run number */
	switch (loadBy) {
		case kGotoGTID:
			current = data->event_id;
			break;
		case kGotoRun:
			current = data->run_number;
			break;
		case kGotoTime:
			current = (long)data->event_time;
			break;
		default:
			return;
	}
	
	/* do nothing if already loaded desired event */
	if (target == current) return;
	
#ifdef ROOT_FILE
	if (data->infile == ROOT_FILE_FLAG) {
		root_tree->GetTree()->SetBranchStatus("*",0);
		root_tree->GetTree()->SetBranchStatus("Event_id",1);
		set_tree_state();
		
		switch (loadBy) {
			case kGotoGTID:
				// only use sequential event logic for sequential files
				if (!data->non_seq_file) {
					// does the desired GTID come before the currently displayed event?
					diff = target - current;
					if (diff < 0) diff += 0x01000000L;
					if (diff > 0x00800000L) {
						rewind_event_file(data);	// yes: rewind file
					}
				}
				break;
			case kGotoTime:
				// add date to target if it is time only
// must account for time zones to make this work....
//				if (current && target<=2*kOneDay) {
//					target += (long)(current / kOneDay) * kOneDay;
//				}
				// fall through
			case kGotoRun:
				diff = target - current;
				if (diff < 0) {
					rewind_event_file(data);
				}
				break;
		}
		
		for (int i=0; ; ++i) {
			if (root_tree->GetEvent(++root_event_num) > 0) {
				switch (loadBy) {
					case kGotoGTID:
						current = root_event->GetEvent_id();
						break;
					case kGotoRun:
						current = root_event->GetRun();
						break;
					case kGotoTime:
						current = (long)((root_event->GetJulianDate() * (double)(24 * 60 * 60) + 
								  data->root_time_zero) + root_event->GetUT1() + 0.5);
						break;
				}
				if ((loadBy==kGotoGTID && data->non_seq_file) ? 
					 current == target : current >= target)
				{
					clearHistoryAll(data);	// 'all' history is now invalid
					root_tree->GetTree()->SetBranchStatus("*",1);
					thePmtRecord = root_load_event(root_event_num);
					if (!thePmtRecord) {
						Printf("internal error loading PMT info\n");
					} else {
						addToHistory(data, thePmtRecord, HISTORY_ALL);
						xsnoed_event(data, thePmtRecord);
					}
					restore_tree_state();	// restore original event address
					return;
				}
			} else {
				Printf("Event not found\x07\n");
				root_event_num = -1;
				root_tree->GetTree()->SetBranchStatus("*",1);
				restore_tree_state();		// restore original event address
				return;
			}
		}
	}
#endif
	while (1) {
	
		if (!data->throw_out_data) {
		
			switch (loadBy) {
				case kGotoGTID:
					/* calculate GTID difference to desired event */
					if (current != 0) {
						diff = target - current;
						if (!data->non_seq_file) {
							if (diff < 0) diff += 0x01000000L;	/* make it positive */
						
							/* do we have our first good gtid? */
							if (!found_good) {
								found_good = 1;
								/* rewind file if it is too far away */
								if (diff > 0x00800000L) {
									rewind_event_file(data);
									rewound = 1;
									diff = 1;	/* force loading of another event */
								}
							}
						} else {
							if (diff) diff = 1;	/* force loading of another event if not exactly what we want */
						}
					} else if (target == 0) {
						/* this event is an orphan, and we are looking for an orphan */
						diff = 0;
					} else {
						/* this event is an orphan, but we are looking for something else */
						diff = 1;	/* force loading of another event */
					}
					
					/* display event if we have arrived at or passed the desired one */
					if (diff==0 || diff>0x00800000L) {
						/* we found our event to display! */
						data->throw_out_data = 1;	// throw out events after this one
						/* view the event */
						xsnoed_event(data, thePmtRecord);
					}
					break;
					
				case kGotoRun:
					diff = target - current;
					if (diff<0 && !rewound) {
						rewind_event_file(data);
						rewound = 1;
					} else if (diff <= 0) {
						data->throw_out_data = 1;	// throw out events after this one
						/* view the event */
						xsnoed_event(data, thePmtRecord);
					}
					break;
					
				case kGotoTime:
					/* calculate time difference to desired event */
					if (current != 0) {
						diff = target - current;
					
						/* do we have our first good time? */
						if (!found_good) {
							found_good = 1;
							/* add date if no date in target time */
// must account for time zones to make this work....
//							if (target < 2*kOneDay) {
//								target += (long)(current / kOneDay) * kOneDay;
//								diff = target - current;
//							}
							/* rewind file if it is too far away */
							if (diff < 0) {
								rewind_event_file(data);
								rewound = 1;
								diff = 1;	/* force loading of another event */
							}
						}
					} else if (target == 0) {
						/* this event is an orphan, and we are looking for an orphan */
						diff = 0;
					} else {
						/* this event is an orphan, but we are looking for something else */
						diff = 1;	/* force loading of another event */
					}
					
					/* display event if we have arrived at or passed the desired one */
					if (diff <= 0) {
						/* we found our event to display! */
						data->throw_out_data = 1;	// throw out events after this one
						/* view the event */
						xsnoed_event(data, thePmtRecord);
					}
					break;
			}
		}
		
		/* load events into future buffer if throwing out */
		if (data->throw_out_data) {
			if (!put_next_event_into_future(data)) break;
			if (data->history_size[HISTORY_FUTURE] >= FUTURE_SIZE) {
				break;	// stop when 'future' history is full
			}
		} else {
			thePmtRecord = get_next_event(data);
			if (!thePmtRecord) {
				/* rewind the file if we haven't found any good events yet */
				if (!found_good && !rewound) {
					rewind_event_file(data);
					rewound = 1;
					thePmtRecord = get_next_event(data);
				}
				if (!thePmtRecord) {
					if (!data->throw_out_data) {
						Printf("Event not found\x07\n");
					}
					break;
				}
			}
			// add event to 'all' history buffer (MUST do this before displaying it)
			addToHistory(data, thePmtRecord, HISTORY_ALL);

			/* set gtid or run number of last loaded event for next time through loop */
			switch (loadBy) {
				case kGotoGTID:
					current = thePmtRecord->TriggerCardData.BcGT;
					break;
				case kGotoRun:
					current = thePmtRecord->RunNumber;
					break;
				case kGotoTime: {
					double ev_time;
					ev_time = ((double) 4294967296.0 * thePmtRecord->TriggerCardData.Bc10_2 + 
	    							 thePmtRecord->TriggerCardData.Bc10_1) * 1e-7;
	    			// add offset to time zero if the time was valid
	    			if (ev_time) ev_time += data->sno_time_zero;
	    			current = (long)ev_time;
				}	break;
			}
		}
	}
}

/* get next event into future buffer */
int put_next_event_into_future(ImageData *data)
{
	int			n_future = data->history_size[HISTORY_FUTURE];
	const short kBuffSize = 512;
	char 		buff[kBuffSize];
	
	if (n_future && !data->history_buff[HISTORY_FUTURE][n_future-1]) {
		return(0);	// can't add to future after a null event
	} else {
		SetPrintfOutput(buff, kBuffSize);
		PmtEventRecord *thePmtRecord = get_next_event(data,0);	// get next physical event
		SetPrintfOutput(NULL,0);
		
		if (*buff) {
			if (data->future_msg[n_future]) {
				Printf("Lost message: %s",data->future_msg[n_future]);
				delete [] data->future_msg[n_future];
			}
			data->future_msg[n_future] = new char[strlen(buff) + 1];
			if (data->future_msg[n_future]) {
				strcpy(data->future_msg[n_future], buff);
			}
		}
		addToHistory(data, thePmtRecord, HISTORY_FUTURE);
		
		return(thePmtRecord != NULL);
	}
}

// getNextPmt - get next pmt record from file
// - decode other interesting banks at the same time
static aPmtEventRecord *getNextPmt(ImageData *data)
{
	nZDABPtr 		nzdabPtr;
	u_int32		  *	dataPt;
	PmtEventRecord*	pmtRecord = NULL;
	PZdabFile	  *	zdabFile = data->mZdabFile;

	for (;;) {
		nzdabPtr = zdabFile->NextRecord();
		if (!nzdabPtr) break;	// give up if no more records in file
		
		// handle PmtEventRecords
		pmtRecord = zdabFile->GetPmtRecord(nzdabPtr);
		if (pmtRecord) break;
		
		// handle other records
		dataPt = PZdabFile::GetBank(nzdabPtr);
		if (dataPt) handleHeaderRecord(data, dataPt, nzdabPtr->bank_name);

	} // loop until we find a pmt record
	
	return(pmtRecord);
}

/*
** get_next_event - return pointer to next pmt event record in file
*/
aPmtEventRecord *get_next_event(ImageData *data, int use_future)
{
	aPmtEventRecord *thePmtRecord = NULL;
	FILE	*fp;
	int		subrun, num;
	

	// -NT: dispatch to XSNOMAN event loader in xsnoman.cxx
#ifdef XSNOMAN  
	if (data->input_source == INPUT_XSNOMAN) {
	  thePmtRecord = xsnoman_load_event();
	  return (thePmtRecord);
	}
#endif // XSNOMAN
	  
	if (data->require_rewind) {
		/* rewind to start of file */
		rewind_event_file(data);
	} else if (use_future && (num = data->history_size[HISTORY_FUTURE]) != 0) {	// is a future event available?
		// get next event from 'future' history buffer
		if (data->history_buff[HISTORY_FUTURE][0]) {
			thePmtRecord = (PmtEventRecord *)(data->history_buff[HISTORY_FUTURE][0] + 1);
		} else {
			removeFutureEvent(data);
			data->require_rewind = 1;
		}
		return(thePmtRecord);
	}
	
	for (;;) {
#ifdef ROOT_FILE
		if (data->infile == ROOT_FILE_FLAG) {
			
			thePmtRecord = root_load_event(root_event_num+1);
			if (!thePmtRecord) {
				root_event_num = -1;
			}
			
		} else {
#endif
			thePmtRecord = getNextPmt(data);
			if (!thePmtRecord) {
				/* try to open next subrun */
				subrun = zdab_get_subrun(data->mEventFile.GetString());
				if (subrun>=0 && subrun<999) {
					/* try to open next subrun */
					char filename[FILELEN];
					strcpy(filename, data->mEventFile.GetString());
					zdab_set_subrun(filename, subrun+1);
					fp = fopen(filename,"rb");
					if (fp) {
						/* change name of currently open file */
						data->mEventFile.SetString(filename);
						/* success! */
						Printf("Opened zdab file: %s\n", filename);
						/* close the old file */
						fclose(data->infile);
						/* make the new file current */
						data->infile = fp;
						/* try again */
						data->mZdabFile->Init(fp);
						thePmtRecord = getNextPmt(data);
						if (thePmtRecord) break; // got our event
					}
				}
				data->last_seq_gtid = 0;
				// set flag so we rewind on next call
				if (use_future) {
					data->require_rewind = 1;
				}
			}
#ifdef ROOT_FILE
		}
#endif
		break;	// didn't want to loop anyway
	}
	// keep track of gtid of last event in a sequential file
	if (thePmtRecord && !data->non_seq_file) {
		long gtid = thePmtRecord->TriggerCardData.BcGT;
		// ignore orphans (GTID 0)
		if (gtid != 0) {
			if (data->last_seq_gtid) {
				long diff = gtid - data->last_seq_gtid;
				if (diff < 0) diff += 0x01000000L;
				if (diff > 0x00800000L) {
					data->non_seq_file = 1;
					Printf("Events in this file are non-sequential -- 'Goto GTID' will search for exact match.\n");
				}
			}
			data->last_seq_gtid = gtid;
		}
	}
	return(thePmtRecord);
}

#ifndef NO_DISPATCH // --------------
// SetDispName
void SetDispName(char *name)
{
	strcpy(disp_hostname,name);
}
// GetDispName
char *GetDispName(void)
{
	return(disp_hostname);
}
#endif // NO_DISPATCH ----------------

//=====================================================================
#if defined(USE_PTHREAD) && !defined(NO_DISPATCH)	

int do_thread_command(int cmd)
{
	if (thread_command_complete()) {
		pthread_mutex_lock(&command_mutex);
		++thread_command_count;
		thread_command = cmd;
		pthread_mutex_unlock(&command_mutex);
		return(0);
	} else {
		return(-1);
	}
}

int thread_command_complete()
{
	bool result;
	pthread_mutex_lock(&command_mutex);
	result = thread_command_count == thread_response_count;
	pthread_mutex_unlock(&command_mutex);
	return result;
}

int get_data_threaded()
{
    // start the thread reading more data if our buffers are not already full
    if (!disp_nbytes_next[disp_head]) {
		do_thread_command(THREAD_CMD_GET_DATA);
    }
    // clear this buffer to allow the dispatcher thread to use it again
    if (disp_nbytes_next[disp_tail] == -1) {
        disp_nbytes_next[disp_tail] = 0;
        if (++disp_tail >= DISP_NBUF) disp_tail = 0;
    }
    if (disp_nbytes_next[disp_tail]) {
        disp_buf = disp_buf_next[disp_tail];
        disp_nbytes = disp_nbytes_next[disp_tail];
        strcpy(disp_tag, disp_tag_next[disp_tail]);
        disp_nbytes_next[disp_tail] = -1;   // mark this buffer as "in use"
		return(disp_nbytes);
	} else {
		return(0);
	}
}

/* dispatcher thread process */
/* for the threaded version, ALL dispatcher calls are made from this routine */
void * do_dispatcher_thread(void *arg)
{
	int		rc, nbytes;
    int		statusout, position;
    time_t 	cur_time;
    ImageData *data = (ImageData *)arg;

	for (;;) {
		if (thread_command_complete()) {
			usleep_ph(10000);	// sleep 0.01 sec while waiting for next command
			continue;
		}
		switch (thread_command) {
			case THREAD_CMD_NULL:
				break;
			case THREAD_CMD_CONNECT:
				InitDispLink_Now(data);
				break;
			case THREAD_CMD_DISCONNECT:
				CloseDispLink_Now();
				break;
			case THREAD_CMD_GET_DATA:
				// first, handle broken pipes
				if (brokenPipe) {
					brokenPipe = 0;
					Printf("---BROKEN PIPE--- Attempting to reconnect\n");
					if (isConnected) {
						CloseDispatcher();
					}
		    		retryConnection = 1;
		    		InitDispLink_Now(data);
		    		break;
				}
				if (disp_nbytes_next[disp_head]) {
				    break;  // our buffers are full, so stop reading
				}
				rc = check_head(disp_tag_next[disp_head], &nbytes);
				if (rc > 0) { 
					
					rc = get_data(disp_buf_next[disp_head], DISP_BUFSIZ);

					// if we have a bad read, let's set the (dispatcher) 
					// statuscheck to 'bad' and get out of the subloop to 
					// start up the dispatcher-reconnect cycle.
					if( rc < 0 ) {
						statuscheck = -1;
						statusdata = 0;
				    	status_send_time = 1;
						Printf("---BAD READ--- Attempting to reconnect\n");
						if (isConnected) {
							CloseDispatcher();
						}
						retryConnection = 1;
						InitDispLink_Now(data);
					} else {
						// we got some data, so the dispatcher must be OK
						if (status_send_time) {
							// print "OK" message if we started decrementing status check
							if (statuscheck != STATUS_COUNT) {
								Printf("+++DISPATCHER OK+++\n");
								statuscheck = STATUS_COUNT;
							}
							// reset status send time
							status_send_time = 0;
						}
						if (nbytes) {
						    // setting the data size makes this visible to the main thread
						    disp_nbytes_next[disp_head] = nbytes;
						    if (++disp_head >= DISP_NBUF) disp_head = 0;
						}
						continue;   // continue reading ahead to fill next buffer
					}
				} else {
					// Did we have a dispatcher communication problem?
					if( rc < 0 ) {
#ifdef IGNORE_DISP_ERRORS
						rc = 0;
#else
				  		// If so, set the dispatcher status-bit to 'pooched'
				  		if (statuscheck >= 0) {
				  			statuscheck = -1;
				  		}
				    	statusdata = 0;
				    	status_send_time = 1;
#endif		
					}
					// Since we don't have data now either, we check the system time.
					cur_time = time(NULL);
					
					if (status_send_time) {
						if (cur_time - status_send_time >= STATUS_DEAD_TIME) {
							if (statuscheck >= -1) {
								Printf("***DISPATCHER POLLING ERROR***\n");
							}
							status_send_time = 0;
							if (--statuscheck < 0) {
								rc=whereis(disp_hostname,"XSNOED",NULL,0);
								if (rc<1) {
							  		// If the willful-disconnect flag is on, we've 
							  		// disconnected as per request of the operator, 
							  		// so we write this on the display and continue.
							  		if (dispconnectstatus==-1) {
							    		CloseDispLink_Now();
							  		} else {
							  			// If not, then we are disconnected without consent, 
							  			// and we will try to reconnect until the operator 
							  			// specifically requests a cancel.
							    		Printf("***ATTEMPTING TO RECONNECT***\n");
										if (isConnected) {
											CloseDispatcher();
										}
							    		retryConnection = 1;
							    		InitDispLink_Now(data);
							  		}
								}
								if (statuscheck < -10) statuscheck = -10;
							}
						}
					}
					// has it been more than N seconds since we have seen some data?
					if (!status_send_time &&
						cur_time - last_recv_time >= STATUS_CHECK_TIME)
					{
						// Send out a heartbeat to dispatcher to verify its existence
						statusout=2;
						position = 0;
		
						rc = put_data ("STATUS",&statusout,4,&position);
		
						status_send_time = cur_time;
					}
				}
				break;
			case THREAD_CMD_DIE:
				pthread_mutex_lock(&command_mutex);
				thread_running = 0;
				thread_response_count = thread_command_count;
				pthread_mutex_unlock(&command_mutex);
				return(NULL);
		}
		pthread_mutex_lock(&command_mutex);
 		thread_response_count = thread_command_count;
		pthread_mutex_unlock(&command_mutex);
	}
}

#endif
//==================================================================================
