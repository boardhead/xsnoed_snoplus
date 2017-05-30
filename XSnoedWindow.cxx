#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <Xm/Form.h>
#include <Xm/ScrollBar.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <X11/StringDefs.h>
#include "XSnoedWindow.h"
#include "XSnoedImage.h"
#include "PResourceManager.h"
#include "PHitInfoWindow.h"
#include "PNCDHitInfoWindow.h"
#include "PRecordInfoWindow.h"
#include "PFitterWindow.h"
#include "PEventInfoWindow.h"
#include "PNCDInfoWindow.h"
#include "PNCDScopeWindow.h"
#include "PNCDImage.h"
#include "PEventControlWindow.h"
#include "PPrintWindow.h"
#include "PColourWindow.h"
#ifdef XSNOMAN
#include "PSnomanWindow.h"
#include "PDamnWindow.h"
#endif
#include "PSettingsWindow.h"
#include "POpticalWindow.h"
#include "PMonteCarloWindow.h"
#include "PAsciiWindow.h"
#include "PAnimationWindow.h"
#include "PDumpDataWindow.h"
#include "PFlatImage.h"
#include "PEventHistogram.h"
#include "PNCDHistogram.h"
#include "PEventTimes.h"
#include "PCrateImage.h"
#include "PMapImage.h"
#include "PScopeImage.h"
#include "PUtils.h"
#include "PZdabFile.h"
#include "xsnoedstream.h"
#include "xsnoed.h"
#include "xsnoed_version.h"
#include "menu.h"
#ifdef ROOT_FILE
#include "TFile.h"
#include "QWater.h"
#include "QRchHist.h"
#include "PSnoDBWindow.h"
#endif
#ifdef SNOPLUS
#include "PCaenWindow.h"
#endif

#ifndef NO_HELP
#include <unistd.h>
#include "mozilla.h"
static char	*sBrowserName  = "netscape";
#endif // NO_HELP

static char *sWaterName[2]          = { "Water", "D2O" };
static ImageData *sLastDeletedData  = 0;

// --------------------------------------------------------------------------------------
// Main menu definitions
//
static MenuStruct extras_menu[] = {
	{ "New Event Display",	0, XK_N,	IDM_NEW_DISPLAY, 	NULL, 0, 0},
	{ "Load Uniform Distribution",0, XK_L,IDM_UNIFORM, 		NULL, 0, 0},
	{ "Dump Record Info",	0, XK_D,	IDM_DUMP_RECORDS,	NULL, 0, MENU_TOGGLE},
};
static MenuStruct echo_main_item[] = {
	{ "Echo Main Display",	0, XK_E,	IDM_ECHO_MAIN,		NULL, 0, MENU_TOGGLE}
};
static MenuStruct settings_menu[] = {
	{ "General...",			0, XK_G,	SETTINGS_WINDOW,	NULL, 0, 0},
	{ "Colors...",			0, XK_C,	COLOUR_WINDOW,		NULL, 0, 0},
};
static MenuStruct file_menu[] = {
#ifndef NO_DISPATCH
	{ "Dispatcher...",		'd', XK_D,	DISP_WINDOW,		NULL, 0, 0},
#endif
	{ "Open File...", 		'o', XK_O,	IDM_LOAD_EVENT,		NULL, 0, 0},
#ifdef ROOT_FILE
	{ "Open Rch File...", 	0, XK_R,	IDM_OPEN_RCH,		NULL, 0, 0},
#endif
	{ "Close",				0, XK_C,	IDM_CLOSE_FILE,		NULL, 0, 0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Print Image...", 	'p', XK_P,	IDM_PRINT_IMAGE,	NULL, 0, 0},
#ifndef NO_FORK
	{ "Print Window...", 	0, XK_W,	IDM_PRINT_WINDOW,	NULL, 0, 0},
#endif // VAX
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
#ifdef XSNOMAN
	{ "SnoMan Control...",	0, XK_M,	SNOMAN_WINDOW,		NULL, 0, 0},
#else
	{ "Event Control...",	0, XK_E,	EVT_NUM_WINDOW,		NULL, 0, 0},
#endif
	{ "Next Event", 		'>', XK_N,	IDM_NEXT_EVENT,		NULL, 0, 0},
	{ "Prev Event",			'<', XK_v,	IDM_PREV_EVENT,		NULL, 0, 0},
	{ "Clear Event",		'l', XK_l,	IDM_CLEAR_EVENT,	NULL, 0, 0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Sum Events",			0, XK_u,	IDM_SUM_EVENT,		NULL, 0, MENU_TOGGLE},
#ifdef FITTR
	{ "Fit Events",			0, XK_i,	IDM_FIT_EVENT,		NULL, 0, MENU_TOGGLE},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Settings",			0, XK_t,	0, settings_menu,	XtNumber(settings_menu), 0},
	{ "Save Settings",		's', XK_S,	IDM_SAVE_SETTINGS,	NULL, 0, 0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "About XSNOED...",  	0, XK_A,	IDM_ABOUT,			NULL, 0, 0},
#ifndef NO_HELP
	{ "User Guide...",		0, XK_G,	IDM_HELP,			NULL, 0, 0},
#endif
	{ "Extras",	   			0, XK_x,	IDM_EXTRAS_MENU, extras_menu, XtNumber(extras_menu), 0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
#ifdef DEMO_VERSION
	{ "Protect", 			0, 0,		IDM_PROTECT,		NULL, 0, 0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
#endif
#ifdef NO_MAIN
	{ "Close Display",		0, 0,		IDM_QUIT,			NULL, 0, 0}
#else
	{ "Quit", 				'q', XK_Q,	IDM_QUIT,			NULL, 0, 0}
#endif
#ifdef XSNOMAN
	,{ "Quit SNOMAN",		0, 0,		IDM_QUIT_SNOMAN,	NULL, 0, 0}
#endif
};
static MenuStruct hit_menu[] = {
	{ "All",	 			0, XK_A,	IDM_CUT_ALL,		NULL, 0, 0},	
	{ "None",	 			0, XK_N,	IDM_CUT_NONE,		NULL, 0, 0},
	{ "PMT Only",	 		0, 0,	    IDM_CUT_PMT_ONLY,	NULL, 0, 0},
#ifndef SNOPLUS
    { "NCD Only",	 		0, 0,	    IDM_CUT_NCD_ONLY,	NULL, 0, 0},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Normal",	 			0, XK_r,	IDM_CUT_NORMAL,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},	
	{ "Owl", 	 			0, XK_O,	IDM_CUT_OWL,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "Low Gain", 			0, XK_L,	IDM_CUT_LOW_GAIN,	NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "Neck", 				0, XK_e,	IDM_CUT_NECK,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "BUTTS", 				0, XK_B,	IDM_CUT_BUTTS,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "FECD", 				0, XK_F,	IDM_CUT_FECD,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
#ifndef SNOPLUS
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "NCD Shaper", 		0, 0,	    IDM_CUT_SHAPER,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "NCD MUX", 		    0, 0,	    IDM_CUT_MUX,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "NCD Scope", 		     0, 0,	    IDM_CUT_SCOPE,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Discarded", 			0, XK_D,	IDM_CUT_DISCARDED,	NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "Underscale",			0, XK_U,	IDM_CUT_UNDERSCALE,	NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
	{ "Overscale",			0, XK_v,	IDM_CUT_OVERSCALE,	NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
};

static MenuStruct display_menu[] = {
	{ "Hits", 				0, XK_H,	0, hit_menu, 		XtNumber(hit_menu),	0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Vessel",				0, XK_V,	IDM_VESSEL,			NULL, 0, MENU_TOGGLE},
	{ "Hit Lines",			0, XK_L,	IDM_HIT_LINES,		NULL, 0, MENU_TOGGLE},
	{ "Fit Lines",			0, XK_i,	IDM_RCON_LINES,		NULL, 0, MENU_TOGGLE},
#ifdef ROOT_FILE
	{ "Water Levels",		0, XK_W,	IDM_WATER_LEVEL,	NULL, 0, MENU_TOGGLE},
#else
	{ "Water Level",		0, XK_W,	IDM_WATER_LEVEL,	NULL, 0, MENU_TOGGLE},
#endif
#ifdef AV_ANCHOR
	{ "AV Anchor Ropes",	0, XK_A,   	IDM_ANCHOR_ROPES,	NULL, 0, MENU_TOGGLE},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "3D Geodesic",		0, XK_G,	IDM_GEODESIC,		NULL, 0, MENU_RADIO | MENU_TOGGLE_ON},
	{ "3D Polar", 			0, XK_P,	IDM_POLAR,			NULL, 0, MENU_RADIO},
	{ "3D No Frame", 		0, XK_N,	IDM_NO_FRAME,		NULL, 0, MENU_RADIO},
	{ "Flat Map",			0, XK_F,	IDM_PANE_FLAT,		NULL, 0, MENU_RADIO},	
	{ "Crate Map",			0, XK_C,	IDM_PANE_CRATE,		NULL, 0, MENU_RADIO},	
	{ "Projection",			0, XK_r,	IDM_PANE_PROJ,		NULL, 0, MENU_RADIO},
#ifndef SNOPLUS
    { "NCD Map",			0, 0,	    IDM_PANE_NCD,		NULL, 0, MENU_RADIO},	
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "White Backgnd",		'b', XK_B,	IDM_WHITE_BKG,		NULL, 0, MENU_TOGGLE},
	{ "Greyscale",			'g', XK_e,	IDM_GREYSCALE,		NULL, 0, MENU_TOGGLE},
};
static MenuStruct move_menu[] = {
	{ "To Home", 			'h', XK_H,	IDM_MOVE_HOME,		NULL, 0, 0},
	{ "To Top", 			0, 0,	    IDM_MOVE_TOP,		NULL, 0, 0},
	{ "To Fit", 			0, XK_T,	IDM_MOVE_VERTEX,	NULL, 0, 0},
	{ "To Sun", 			0, XK_S,	IDM_MOVE_SUN,		NULL, 0, 0},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Auto Fit",			0, XK_A,	IDM_AUTO_VERTEX,	NULL, 0, MENU_TOGGLE},
	{ "Auto Sun",			0, XK_u,	IDM_AUTO_SUN,		NULL, 0, MENU_TOGGLE},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Sphere", 			'e', XK_e,	IDM_MOVE_SPHERE,	NULL, 0, MENU_RADIO | MENU_TOGGLE_ON},
	{ "Fit", 				'f', XK_F,	IDM_MOVE_EVENT,		NULL, 0, MENU_RADIO},
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Next Fit",			'x', XK_x,	IDM_NEXT_VERTEX,	NULL, 0, 0},
};
static MenuStruct data_menu[] = {
#ifdef LOAD_CALIBRATION
	{ "Uncalibrated",		'u', XK_U,	IDM_UNCALIBRATED, 	NULL, 0, MENU_RADIO | MENU_TOGGLE_ON},
	{ "Calibrated",			'c', XK_C,	IDM_CAL_SIMPLE, 	NULL, 0, MENU_RADIO},
	{ "Cal No Walk",		0, XK_N,	IDM_CAL_NO_WALK,	NULL, 0, MENU_RADIO},
#ifdef TITLES_CAL
	{ "Titles Cal",			0, 0,		IDM_CAL_PETER,		NULL, 0, MENU_RADIO},
#endif
#ifdef ROOT_FILE
	{ "SNODB Calibration",	0, XK_B,	IDM_CAL_SNODB, 		NULL, 0, MENU_RADIO},
	{ "SNOMAN Calibration",	0, XK_M,	IDM_CAL_SNOMAN, 	NULL, 0, MENU_RADIO},
	{ "Pre-calibrated",		0, XK_P,	IDM_PRECALIBRATED,	NULL, 0, MENU_RADIO},
#endif
#ifdef XSNOMAN
	{ "Official Calibration",0, XK_O,	IDM_PRECALIBRATED,	NULL, 0, MENU_RADIO},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
#endif
	{ "Tac", 				't', XK_T,	IDM_TAC,			NULL, 0, MENU_RADIO | MENU_TOGGLE_ON},
#ifdef LOAD_CALIBRATION
	{ "Time Diff",			0, XK_D,	IDM_DELTA_T, 		NULL, 0, MENU_RADIO},
#endif
	{ "Qhs", 				'1', XK_s,	IDM_QHS,			NULL, 0, MENU_RADIO},
	{ "Qhl", 				'2', XK_l,	IDM_QHL,			NULL, 0, MENU_RADIO},
	{ "Qlx", 				'3', XK_x,	IDM_QLX,			NULL, 0, MENU_RADIO},
	{ "Qhl-Qhs", 			'4', XK_minus,IDM_QHL_QHS,		NULL, 0, MENU_RADIO},
	{ "Hit Count", 			0, XK_i,	IDM_NHIT,			NULL, 0, MENU_RADIO},
#ifndef SNOPLUS
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "NCD Shaper Value", 	0, 0,	    IDM_NCD_SHAPER_VAL,	NULL, 0, MENU_RADIO | MENU_TOGGLE_ON},
	{ "NCD Shaper Hits", 	0, 0,	    IDM_NCD_SHAPER_HIT,	NULL, 0, MENU_RADIO},
	{ "NCD MUX Hits", 		0, 0,	    IDM_NCD_MUX_HIT,	NULL, 0, MENU_RADIO},
	{ "NCD Scope Hits", 	0, 0,	    IDM_NCD_SCOPE_HIT,	NULL, 0, MENU_RADIO},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "Crate",				0, XK_r,	IDM_DISP_CRATE,		NULL, 0, MENU_RADIO},
	{ "Card",				0, XK_a,	IDM_DISP_CARD,		NULL, 0, MENU_RADIO},
	{ "Channel",			0, XK_h,	IDM_DISP_CHANNEL,	NULL, 0, MENU_RADIO},
	{ "Cell",				0, XK_e,	IDM_DISP_CELL,		NULL, 0, MENU_RADIO},
#ifndef NO_DISPATCH
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "CMOS Rates", 		0, 0,		IDM_CMOS_RATES,		NULL, 0, MENU_RADIO},
#endif
};
static MenuStruct window_menu[] = {
	{ "Record Info",		0, XK_R,	RECORD_INFO_WINDOW,	NULL, 0, 0},
	{ "Event Info",			0, XK_E,	EVT_INFO_WINDOW,	NULL, 0, 0},
	{ "Hit Info",  			0, XK_H,	HIT_INFO_WINDOW,	NULL, 0, 0},
	{ "Histogram", 			0, XK_s,	HIST_WINDOW,		NULL, 0, 0},
	{ "Flat Map",  			0, XK_F,	FLAT_WINDOW,		NULL, 0, 0},
	{ "Crate Map", 			0, XK_C,	CRATE_WINDOW,		NULL, 0, 0},
	{ "Projections",		0, XK_P,	PROJ_WINDOW,		NULL, 0, 0},
#ifndef SNOPLUS
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
	{ "NCD Info",		    0, XK_N,   	NCD_INFO_WINDOW,	NULL, 0, 0},
	{ "NCD Hit",  			0, 0,	    NCD_HIT_INFO_WINDOW,NULL, 0, 0},
	{ "NCD Histogram",      0, 0,       NCD_HIST_WINDOW,    NULL, 0, 0},
	{ "NCD Map",            0, 0,       NCD_MAP_WINDOW,     NULL, 0, 0},
	{ "NCD Scope",			0, 0,	    NCD_SCOPE_WINDOW,	NULL, 0, 0},
#endif
	{ NULL, 				0, 0,		0,					NULL, 0, 0},
#ifdef SNOPLUS
	{ "CAEN Scope",		    0, XK_N,    CAEN_WINDOW,    	NULL, 0, 0},
#else
	{ "Trigger Scope", 		0, 0,     	SCOPE_WINDOW,		NULL, 0, 0},
#endif
#ifdef FITTR
	{ "Fitter",     		0, XK_i,	FITTER_WINDOW,		NULL, 0, 0},
#endif
#ifdef OPTICAL_CAL
	{ "Optical Constants",	0, XK_O,	OPTICAL_WINDOW,		NULL, 0, 0},
#endif
	{ "Monte Carlo",		0, XK_M,	MONTE_CARLO_WINDOW,	NULL, 0, 0},
	{ "Event Times",		0, XK_T,	TIME_WINDOW,		NULL, 0, 0},
	{ "ASCII Output",		0, XK_A,	ASCII_WINDOW,		NULL, 0, 0},
	{ "Animation",			0, 0,	    ANIMATION_WINDOW,	NULL, 0, 0},
	{ "Dump Data",			0, XK_D,	DUMP_DATA_WINDOW,	NULL, 0, 0},
#ifdef ROOT_FILE
	{ "SNODB Viewer",		0, XK_O,	SNODB_WINDOW,		NULL, 0, 0},
	{ "Rch Time",			0, 0,		RCH_TIME_WINDOW,	NULL, 0, 0},
	{ "Rch Qhs",			0, 0,		RCH_QHS_WINDOW,		NULL, 0, 0},
	{ "Rch Qhl",			0, 0,		RCH_QHL_WINDOW,		NULL, 0, 0},
	{ "Rch Qlx",			0, 0,		RCH_QLX_WINDOW,		NULL, 0, 0},
#endif
#ifdef XSNOMAN
	{ "DAMN/DARN Banks",	      	0, 0,		DAMN_WINDOW,	NULL, 0, 0},
#endif
};
static MenuStruct main_menu[] = {
	{ "File", 				0, 0,		0, file_menu, 		XtNumber(file_menu),	0},
	{ "Move",				0, 0,		0, move_menu, 		XtNumber(move_menu),	0},
	{ "Display",			0, 0,		0, display_menu, 	XtNumber(display_menu), 0},
	{ "Data",				0, 0, IDM_DATA_MENU, data_menu, XtNumber(data_menu),	0},
	{ "Windows",			0, 0,		0, window_menu, 	XtNumber(window_menu),	0},
};


//======================================================================================
// XSnoedWindow constructor
//
XSnoedWindow::XSnoedWindow(int load_settings)
			: PImageWindow(NULL)
{
	Arg		wargs[10];
	int		n;
	unsigned int i;
	extern char *g_argv[];
	
	mLabelHeight = 0;
	mLabelDirty = 0;
	mEchoMainDisplay = 0;
	mExtraNum = 0;
	mExtraData = NULL;
	mPrintType = kPrintImage;
	mLabelText[0].font = NULL;
	mLabelText[0].string = NULL;
	mWarnDialog = NULL;
#ifdef DEMO_VERSION
	mPassword = NULL;
#endif
	
	/* initialize necessary member variables */
	filebox = 0;
	aboutbox = 0;

	/* create new imagedata structure */
	ImageData *data = new ImageData;
	if (sLastDeletedData == data) {
	    sLastDeletedData = NULL;
	}
	memset(data,0,sizeof(ImageData));
	mData = data;
/*
** Create top level widget containing a form widget
*/
	data->argv = *g_argv;
	
#ifdef NO_MAIN
	int was_app = 1;
#else	
	int was_app = (PResourceManager::sResource.the_app != 0);
#endif
	/* initialize the application */
	PResourceManager::InitApp();
	
	/* create our top level widget */
	n = 0;
	XtSetArg(wargs[n], XmNx, 30); ++n;
	XtSetArg(wargs[n], XmNy, 30); ++n;
	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	XtSetArg(wargs[n], XmNwidth, 400); ++n;
	XtSetArg(wargs[n], XmNheight, 400); ++n;
#ifdef XSNOMAN
	XtSetArg(wargs[n], XmNtitle, "XED Snoman/Xsnoed Display"); ++n;
#elif NO_MAIN
	XtSetArg(wargs[n], XmNtitle, "QSnoed Object"); ++n;
#elif defined SNOPLUS
	XtSetArg(wargs[n], XmNtitle, "SNO+ Event Display"); ++n;
#else
	XtSetArg(wargs[n], XmNtitle, "SNO Event Display"); ++n;
#endif
	if (was_app) {
		XtSetArg(wargs[n], XmNx, 100); ++n;
		XtSetArg(wargs[n], XmNy, 100); ++n;
		// create transient shell if this isn't the application window
		// (must be called "XSnoed" for XSNOMAN to load resources properly)
		data->toplevel = CreateShell("XSnoed",NULL,wargs,n);
	} else {
		// create application shell
		data->toplevel = XtAppCreateShell(NULL,"XSnoed",applicationShellWidgetClass,
						PResourceManager::sResource.display,wargs,n);
	}

	PResourceManager::InitResources(data->toplevel);
/*
** Initialize our ImageData
*/
	initData(data, load_settings);
	SetShell(data->toplevel);

	/* create main pane */
	SetMainPane(XtCreateManagedWidget("imageForm",xmFormWidgetClass,data->toplevel,NULL,0));
/*
** Create Menubar
*/
	CreateMenu(NULL,main_menu,XtNumber(main_menu),this);
	
	// subsequent main windows do not quit the program, so change the label
	if (was_app) {
		GetMenu()->SetLabel(IDM_QUIT,"Close Display");
		MenuList *ms = GetMenu()->FindMenuItem(IDM_EXTRAS_MENU);
		if (ms) {
			GetMenu()->AddMenuItem(echo_main_item, ms);
		}
	}
	
	SetMainWindow();		// make this a main window

	PImageCanvas *pimage = CreateNewImage();
	pimage->CreateCanvas("canvas", kScrollAllMask);

	/* save a pointer to this main window */
	data->mMainWindow = this;
	
	/* listen to resource changed messages */
	PResourceManager::sSpeaker->AddListener(this);
	
	/* listen for cursor messages */
	data->mSpeaker->AddListener(this);
	
	/* setup menus from our resources */
	if (data->geo)		  SelectMenuItem(IDM_GEODESIC + data->geo);
	if (data->move)		  SelectMenuItem(IDM_MOVE_SPHERE + data->move);
	if (data->calibrated) SelectMenuItem(IDM_UNCALIBRATED + data->calibrated);
	if (data->dataType)	  SelectMenuItem(IDM_TAC + data->dataType);
	
	SetHitMaskMenuToggles();

	GetMenu()->SetToggle(IDM_SUM_EVENT,		 data->sum);
	GetMenu()->SetToggle(IDM_VESSEL,		 data->show_vessel);
	GetMenu()->SetToggle(IDM_HIT_LINES,		 data->hit_lines);
	GetMenu()->SetToggle(IDM_RCON_LINES,	 data->rcon_lines);
	GetMenu()->SetToggle(IDM_WATER_LEVEL,	 data->waterLevel);
	GetMenu()->SetToggle(IDM_ANCHOR_ROPES,	 data->anchor_ropes);
	GetMenu()->SetToggle(IDM_AUTO_VERTEX,	 data->auto_vertex);
	GetMenu()->SetToggle(IDM_AUTO_SUN,		 data->auto_sun);
	GetMenu()->SetToggle(IDM_DUMP_RECORDS,	 data->dump_records);
	
	GetMenu()->SetToggle(IDM_WHITE_BKG,	 	(data->image_col & kWhiteBkg) != 0);
	GetMenu()->SetToggle(IDM_GREYSCALE,	 	(data->image_col & kGreyscale) != 0);
#ifdef FITTR
	GetMenu()->SetToggle(IDM_FIT_EVENT,		 data->autoFit);
#endif

	// set zdab file verbose to agree with our current settings
	PZdabFile::SetVerbose(data->dump_records);

	if (data->show_label) {
		LabelFormatChanged();		// initialize label string
	}
	
	// call necessary functions required by current menu items
	SetupSum(data);
	if (data->show_vessel) sendMessage(data, kMessageVesselChanged);
	if (data->waterLevel) DoWater(0);

	/* show the window */
	Show();
	
	/* open any other windows specified by settings */
	if (sMainWindow == this) {
        int open_windows[2];
        int n, bitnum;
        open_windows[0] = data->open_windows;
        open_windows[1] = data->open_windows2;
        if (open_windows[0] > 0 || open_windows[1] > 0) {
            // open windows by menu item number
            for (i=0; i<XtNumber(window_menu); ++i) {
                n = i / 31; // 31 useable bits in each mask
                if (i >= 2) continue;
                bitnum = i - n * 31;
                if (open_windows[n] & (1UL << bitnum)) {
                    ShowWindow(window_menu[i].id);
                }
            }
        } else if (open_windows[0] < 0 || open_windows[1] < 0) {
            // open windows by window ID
            open_windows[0] = -open_windows[0];
            open_windows[1] = -open_windows[1];
            for (i=1; i<NUM_WINDOWS; ++i) {
                n = i / 31; // 31 useable bits in each mask
                bitnum = i - n * 31;
                if (open_windows[n] & (1UL << bitnum)) {
                    ShowWindow(i);
                }
            }
        }
	}
}

XSnoedWindow::~XSnoedWindow()
{
	if (sMainWindow == this) {
		// only save resources on close of base main window
		SaveResources();

		// this is the base xsnoed window -- delete all other main windows
		while (mNextMainWindow) {
			delete mNextMainWindow;
		}
		// reset the base window pointer
		sMainWindow = NULL;
	} else {
		// this is not the base window -- remove from linked list
		PWindow **xwin_pt = &sMainWindow;
		while (*xwin_pt) {
			if ((*xwin_pt)->mNextMainWindow == this) {
				(*xwin_pt)->mNextMainWindow = mNextMainWindow;	// remove this from list
				break;
			}
			xwin_pt = &(*xwin_pt)->mNextMainWindow;
		}
	}
	
	// free all allocated memory in ImageData struct (except this main window)
	freeData(mData);
	if (mData->mDeleteData) {
	    // set our last deleted data pointer so callbacks won't try to
	    // access this data after we delete it
	    sLastDeletedData = mData;
		int	doQuit = mData->mQuitOnDelete;
		// delete our ImageData structure
		delete mData;
		mData = NULL;	// set to NULL so base class destructors don't access data
		if (doQuit) {
			exit(0);
		}
	}
	delete [] mExtraData;
}

void XSnoedWindow::Listen(int message, void *dataPt)
{
	ImageData	*data = GetData();
	
	switch (message) {
		case kMessageEventCleared:
			mLabelText[0].font = NULL;
			mLabelText[0].string = NULL;
			mLabelFlags = 0;
			break;
		case kMessageNewEvent:
			if (data->show_label) {
				SetLabelDirty();
			}
			break;
		case kMessageTimeFormatChanged:
			// make new label if it shows time or date
			if (data->show_label && (mLabelFlags & (kLabelTime | kLabelDate))) {
				SetLabelDirty();
			}
			break;
		case kMessageGTIDFormatChanged:
			// make new label if it shows GTID
			if (data->show_label && (mLabelFlags & kLabelGTID)) {
				SetLabelDirty();
			}
			break;
		case kMessageAngleFormatChanged:
			if (data->show_label && (mLabelFlags & kLabelSunAngle)) {
				SetLabelDirty();
			}
			break;
		case kMessageFitChanged:
			if (data->show_label && (mLabelFlags & (kLabelFitPos | kLabelFitDir | kLabelFitRadius |
						kLabelFitTime | kLabelFitQuality | kLabelNfit | kLabelSunAngle)))
			{
				SetLabelDirty();
			}
			break;
		case kMessageNextTimeAvailable:
			if (data->show_label && (mLabelFlags & kLabelNextTime)) {
				// set label dirty without sending message to update displays
				// (the display update would be too annoying for this case)
				mLabelDirty = 1;
			}
			break;
		case kMessageHitsChanged:
			if (data->show_label && (mLabelFlags & (kLabelDataType | kLabelDataMin | kLabelDataMax | kLabelNhit))) {
				SetLabelDirty();
			}
			break;
		case kMessageResourceColoursChanged:
			data->image_col = PResourceManager::sResource.image_col;
			GetMenu()->SetToggle(IDM_WHITE_BKG, (data->image_col & kWhiteBkg) != 0);
			GetMenu()->SetToggle(IDM_GREYSCALE, (data->image_col & kGreyscale) != 0);
			sendMessage(data, kMessageColoursChanged);
			break;
#ifdef OPTICAL_CAL
		case kMessageResourceOpticalChanged:
			data->u_acrylic	= PResourceManager::sResource.u_acrylic;
			data->u_d2o		= PResourceManager::sResource.u_d2o;
			data->u_h2o		= PResourceManager::sResource.u_h2o;
			
			/* set values of attenuation items in calibration constants window */
			POpticalWindow::UpdateOpticalConstants(data, UPDATE_OSA_ATTEN);
			
			/* recalculate and redisplay images if necessary */
			if (data->wDataType==IDM_NHIT && data->wCalibrated!=IDM_UNCALIBRATED) {
				calcOCA(data, 0);
				calcCalibratedVals(data);
				sendMessage(data, kMessageOpticalChanged);
			}
			break;
#endif
#ifdef ROOT_FILE
		case kMessageCursorHit:
			root_update_rch_windows(data,data->cursor_hit);
			break;
#endif
		case kMessageNewMainDisplayEvent:
			if (mEchoMainDisplay) {
				addToHistory(data, (PmtEventRecord *)dataPt, HISTORY_ALL);
				xsnoed_event(data, (PmtEventRecord *)dataPt);
			}
			break;
	}
}

PImageCanvas *XSnoedWindow::CreateNewImage(Widget canvas)
{
	PImageCanvas	*pimage;
	
	switch (GetData()->wGeo) {
		case IDM_GEODESIC:
		case IDM_POLAR:
		case IDM_NO_FRAME:
			pimage = new XSnoedImage(this, canvas);
			break;
		case IDM_PANE_FLAT:
			pimage = new PFlatImage(this, canvas);
			break;
		case IDM_PANE_CRATE:
			pimage = new PCrateImage(this, canvas);
			break;
		case IDM_PANE_PROJ:
			pimage = new PMapImage(this, canvas);
			((PMapImage *)pimage)->AddMenuItem();	// add Projection menu
			break;
#ifndef SNOPLUS
        case IDM_PANE_NCD:
			pimage = new PNCDImage(this, canvas);
			break;
#endif
        default:
			pimage = new PImageCanvas(this, canvas);
			break;
	}
	
	return(pimage);
}

// update XSnoedResource's and save to file
void XSnoedWindow::SaveResources(int force_save)
{
	int			i;
	ImageData	*data = mData;
	
	// save main window position
	SaveWindowData();
	
	int open_windows = 0;	// initialize open window mask
	int open_windows2 = 0;
	
	// save all other window positions and calculate open_windows bitmask
	for (i=0; i<NUM_WINDOWS; ++i) {
		if (data->mWindow[i]) {
			// save window data for any windows remaining open
			data->mWindow[i]->SaveWindowData();
			// calculate open_windows (indexed to menu entries)
			if (i<31) {
			    open_windows |= (1 << i);
			} else {
			    open_windows2 |= (1 << (i-31));
			}
		}
	}
		
	// update necessary XSnoedResource settings before writing to file
	data->version			= XSNOED_VERSION;
	data->open_windows		= -open_windows;	// negative to indicate IDM_ indexing
	data->open_windows2     = -open_windows2;
	data->geo				= data->wGeo - IDM_GEODESIC;
	data->move				= data->wMove - IDM_MOVE_SPHERE;
	data->dataType			= data->wDataType - IDM_TAC;
	data->calibrated		= data->wCalibrated - IDM_UNCALIBRATED;
	data->projType			= data->wProjType - IDM_PROJ_RECTANGULAR;
	data->projCoords		= data->wProjCoords - IDM_DETECTOR_COORD;
	data->sizeOption		= data->wSizeOption - IDM_SIZE_FIXED;
	data->shapeOption		= data->wShapeOption - IDM_HIT_SQUARE;
	data->mcTrack			= data->wMCTrack - IDM_MC_ALL_TRACKS;
	data->mcNames			= data->wMCNames - IDM_HIDE_NAMES;
	data->history_cut		= !data->history_all;
	data->label_format_pt	= data->label_format;
	
	// set our resource string pointers to point at our string buffers
	for (i=0; i<NUM_WRITERS; ++i) {
		data->output_file_pt[i] = data->output_file[i];
	}
	for (i=0; i<2; ++i) {
		data->print_string_pt[i] = data->print_string[i];
	}
#ifndef NO_DISPATCH
	data->dispatcher		= GetDispName();
#endif

	// send message indicating we are about to write our settings
	sendMessage(data, kMessageWillWriteSettings);
	
	// save settings to resource file
	PResourceManager::WriteSettings(data, force_save);
	
	// send message indicating we are done writing settings
	sendMessage(data, kMessageWriteSettingsDone);
}

void XSnoedWindow::UpdateSelf()
{
	// count number of displayed hits and save to data->num_disp
	ImageData *data = GetData();
	int	n, num_disp = 0;
	if ((n = data->hits.num_nodes) != 0) {
		HitInfo *hi = data->hits.hit_info;
		long bit_mask = data->bit_mask;
		for (int i=0; i<n; ++i,++hi) {
			if (!(hi->flags & bit_mask)) ++num_disp;
		}
	}
	data->num_disp = num_disp;

	// this must be done after calculating num_disp,
	// so the labels will update correctly - PH 05/23/00
	PImageWindow::UpdateSelf();
}

// UpdateDataMenu - update Data menu to show extra hit data available for this event
// - also updates the value of data->wDataType to be consistent if necessary
void XSnoedWindow::UpdateDataMenu()
{
	int				i;
	ImageData	 *	data = GetData();
	ExtraHitData	cur_data;
	MenuStruct		new_items[2] = {
		{ NULL,		0,0, IDM_DISP_EXTRA_FIRST,	NULL, 0, MENU_TOGGLE},	// for extra data type
		{ NULL, 	0,0, 0,						NULL, 0, 0},			// separator
	};
	
	// check to see if the extra types are unchanged
	// (allow an extra dummy entry if wDataType is set to one past the actual number of types)
	if (mExtraNum==data->extra_hit_num || (mExtraNum==data->extra_hit_num+1 &&
		data->extra_hit_num==data->wDataType-IDM_DISP_EXTRA_FIRST))
	{
		for (i=0; ; ++i) {
			if (i >= mExtraNum) {
				return;		// nothing changed, so nothing to do
			}
			if (i < data->extra_hit_num) {
				// break to re-do menu if any name changed (don't need to check the dummy entry)
				if (strcmp(mExtraData[i].name, data->extra_hit_data[i]->name)) break;
			}
		}
	}
	// first, save name of current data type
	int cur_index = data->wDataType - IDM_DISP_EXTRA_FIRST;
	if (cur_index>=0 && cur_index<mExtraNum) {
		strcpy(cur_data.name, mExtraData[cur_index].name);
	} else {
		cur_data.name[0] = '\0';
	}
	
	// get pointer to Data menu
	MenuList *dataMenu = GetMenu()->FindMenuItem(IDM_DATA_MENU);
	if (!dataMenu) return;

	// next, remove all extra items from the menu
	if (mExtraNum) {
		// also remove separator if there will be no extra types in the new menu
		if (!data->extra_hit_num && data->wDataType<IDM_DISP_EXTRA_FIRST) ++mExtraNum;
		do {
			GetMenu()->RemoveMenuItem(dataMenu);
		} while (--mExtraNum > 0);
	} else {
		// add separator
		GetMenu()->AddMenuItem(new_items+1, dataMenu);
	}
	
	int	new_index = -1;
	
	// re-create the array for the data names
	// (leave one extra location for a dummy entry if necessary)
	delete [] mExtraData;
	mExtraData = new ExtraHitData[data->extra_hit_num + 1];
	mExtraNum = 0;
	
	// add new entries to menu
	for (i=0; i<data->extra_hit_num; ++i) {
		// save the name of this data type
		strcpy(mExtraData[mExtraNum].name, data->extra_hit_data[i]->name);
		new_items[0].name = mExtraData[mExtraNum].name;
		new_items[0].id = IDM_DISP_EXTRA_FIRST + i;
		// is this the currently selected item?
		if (cur_data.name[0] && !strcmp(cur_data.name, new_items[0].name)) {
			new_items[0].flags = MENU_RADIO | MENU_TOGGLE_ON;
			new_index = i;	// remember the index of the current item
		} else {
			new_items[0].flags = MENU_RADIO;
		}
		// add this entry to the menu
		GetMenu()->AddMenuItem(new_items, dataMenu);
		++mExtraNum;
	}
	
	// add dummy entry if necessary
	if (cur_data.name[0] && new_index<0) {
		// save the name of this data type
		strcpy(mExtraData[mExtraNum].name, cur_data.name);
		new_items[0].name = mExtraData[mExtraNum].name;
		new_items[0].id = IDM_DISP_EXTRA_FIRST + data->extra_hit_num;
		new_items[0].flags = MENU_RADIO | MENU_TOGGLE_ON;
		GetMenu()->AddMenuItem(new_items, dataMenu);
		++mExtraNum;
		new_index = data->extra_hit_num;	// set new data type to the dummy entry
	}
	
	// update wDataType to current entry index
	if (new_index >= 0) {
		data->wDataType = new_index + IDM_DISP_EXTRA_FIRST;
	}
}

// LabelFormatChanged - called whenever the image label format changes
// - calculates the label height in pixels (obtained through GetLabelHeight())
// - sets label dirty, forcing rebuilding of label on next GetLabelText() call
void XSnoedWindow::LabelFormatChanged()
{
	XFontStruct	*font;
	char		*label_format = mData->label_format;
	
	mLabelHeight = 0;	// initialize label height
	
	// calculate the label height
	if (*label_format) {
		// initialize pointer to next big-font specification
		char *next_big = strstr(label_format,"%+");
		for (;;) {
			// look for next newline spec
			label_format = strstr(label_format, "%/");
			if (!label_format) {
				// no more newlines -- add height of this line
				if (next_big) font = PResourceManager::sResource.label_big_font;
				else		  font = PResourceManager::sResource.label_font;
				mLabelHeight += font->ascent + font->descent;
				break;
			}
			// skip over newline specification
			label_format += 2;
			
			if (next_big && next_big<label_format) {
				// this line contained a big-font specification
				font = PResourceManager::sResource.label_big_font;
				// look for next big-font spec
				next_big = strstr(label_format,"%+");
			} else {
				font = PResourceManager::sResource.label_font;
			}
			// add the height of this label line
			mLabelHeight += font->ascent + font->descent;
		}
	} else {
		mLabelFlags = 0;
	}
	SetLabelDirty();	// the label needs to be remade
}

int XSnoedWindow::GetPrecision(char *fmt, int def_prec)
{
	unsigned	prec;
	char 		*pt = strchr(fmt,'.');
	
	if (pt) {
		prec = (unsigned)atoi(pt+1);
		if (prec > 9) prec = 9;	// limit to 9 decimal places (must not do more)
	} else {
		prec = def_prec;
	}
	return((int)prec);
}

// SetLabelDirty - Set flag to indicate label needs rebuilding
// - also sends message indicating that the label has changed
void XSnoedWindow::SetLabelDirty()
{
	mLabelDirty = 1;
	sendMessage(mData, kMessageLabelChanged);
}

// is the specified data still valid? (ie. hasn't been deleted)
int XSnoedWindow::IsValidData(ImageData *data)
{
    return(data != sLastDeletedData);
}

// BuildLabelString [static] - make label string for specified event
//
// aTextOut - pointer to TextSpec array of kMaxLabelLines entries (may be NULL)
// aLabelFormat - pointer to label format string
// aBuffer - pointer to buffer to save output string (kLabelSize bytes long)
//
long XSnoedWindow::BuildLabelString(ImageData *data, TextSpec *aTextOut,
									char *aLabelFormat, char *aBuffer)
{
	int				i, len;
	char 		  *	pt = aBuffer;
	char		  *	pt2, *str;
	char		  * last = aBuffer + kLabelSize - 32;
	char 		  *	src = aLabelFormat;
	char 		  *	fmtPt = NULL;
	const short		kFormatSize	= 128;
	char 			format[kFormatSize];
	struct tm	  *	tms = NULL;
	int			  *	pmt_counts = NULL;
	int			  *	pmt_extras = NULL;
	float			firstBin, lastBin;
	double			dtmp,mag;
	RconEvent	  *	rcon;
	HistoryEntry  *	entry;
	long			labelFlags;
	
	// these label formats are in the same order as the ELabelFlags bits
	static char *labelFormats[] = { "rn","gt","ti","da","nh","no","ow","lg",
									"fe","bu","ne","dt","dm","dx","pk","in",
									"df","tr","en","fn","fp","fd","fr","ft",
									"fq","nf","sd","sa","pt","nt","sr",NULL };
	
	labelFlags = 0;	// initialize label flags
	
	// initialize label text for first line
	int lines = 0;
	if (aTextOut) {
		aTextOut[0].font = PResourceManager::sResource.label_font;
		aTextOut[0].string = aBuffer;
	}
	
	for (char ch=*src; ; ch=*(++src)) {
		if (fmtPt) {
			if (!ch) {
				*fmtPt = '\0';
				pt += sprintf(pt,"%s",format);	// add malformed format to string
				break;
			}
			if ((ch>='0' && ch<='9') || ch=='.' || ch=='-') {
				*(fmtPt++) = ch;
				// abort if format is too large
				if (fmtPt - format >= (kFormatSize-5)) {
					*pt = '\0';
					break;	// stop now
				}
			} else if (ch == '%') {	// '%' character specified by "%%"
				*(pt++) = ch;
				fmtPt = NULL;
			} else if (ch == '/') {	// CR specified by "%/"
				// terminate this line since our string is a compound string
				// -- one null-terminated string for each line
				*(pt++) = '\0';
				fmtPt = NULL;
				// check for too many lines
				if (lines >= kMaxLabelLines-2) break;
				// initialize label text for next line
				++lines;
				if (aTextOut) {
					aTextOut[lines].font = PResourceManager::sResource.label_font;
					aTextOut[lines].string = pt;
				}
			} else if (ch == '+') {	// big font specified by "%+"
				if (aTextOut) {
					aTextOut[lines].font = PResourceManager::sResource.label_big_font;
				}
				fmtPt = NULL;
			} else if (!*(++src)) {	// skip second letter of format type
				*(fmtPt++) = ch;
				*fmtPt = '\0';
				pt += sprintf(pt,"%s",format);	// add malformed format to string
				break;
			} else {
				ch = tolower(ch);
				char ch2 = tolower(*src);
				long flag;
				for (i=0; ; ++i) {
					if (!labelFormats[i]) {
						flag = 0;
						break;
					}
					if (labelFormats[i][0]==ch && labelFormats[i][1]==ch2) {
						flag = (1L << i);
						break;
					}
				}
				*fmtPt = '\0';	// null terminate format specification
				labelFlags |= flag;	// update our label flags
				switch (flag) {
					case 0L:
						// add malformed format to string
						pt += sprintf(pt,"%s%c%c",format,ch,ch2);
						break;		// no match
					case kLabelRun:
						pt += sprintf(pt,"%ld",(long)data->run_number);
						break;
					case kLabelGTID:
						if (data->hex_id) {
							len = GetPrecision(format, 6);
							pt += sprintf(pt,"0x%.*lx",len,(long)data->event_id);
						} else {
							pt += sprintf(pt,"%ld",(long)data->event_id);
						}
						break;
					case kLabelTime:
						if (!tms) tms = getTms(data->event_time, data->time_zone);
						if (!data->event_time) {
							tms->tm_hour = tms->tm_min = tms->tm_sec = 0;
						}
						pt += sprintf(pt,"%.2d:%.2d:%.2d",
									tms->tm_hour, tms->tm_min, tms->tm_sec);
						len = GetPrecision(format, 0);
						if (len) {
							double dec = data->event_time - (long)data->event_time;
							for (i=0; i<len; ++i) {
								dec *= 10.0;
							}
							dec += 0.5;	// round up to nearest long
							pt += sprintf(pt,".%.*ld",len,(long)dec);
						}
						if (data->time_zone == kTimeZoneUTC) {
							pt += sprintf(pt," UTC");
						} else if (data->time_zone == kTimeZoneLocal) {
							pt += sprintf(pt," Loc");
						}
						break;
					case kLabelDate:
						if (data->event_time) {
							if (!tms) tms = getTms(data->event_time, data->time_zone);
							pt += sprintf(pt,"%.2d/%.2d/%d",
						 				tms->tm_mon+1, tms->tm_mday, tms->tm_year+1900);
						} else {
							pt += sprintf(pt,"00/00/0000");
						}
						break;
					case kLabelNhit:
						pt += PEventInfoWindow::GetNhitString(pt, data);
						// changed 05/23/00
						// pt += sprintf(pt,"%ld",(long)data->hits.num_nodes);
						break;
					case kLabelNnormal:
						i = NHIT_PMT_NORMAL;
Do_Pmt_Count:	// <-- Yes, this is a goto label (not for the faint of heart!)
						if (!pmt_counts) {
							pmt_counts = getPmtCounts(data);
							pmt_extras = getPmtCounts(NULL);
						}
						// print number of unique channels
						pt += sprintf(pt,"%ld",(long)pmt_counts[i]);
						// print total number of hits in brackets if different from unique channels
						if (pmt_extras[i]) {
							pt += sprintf(pt," (%ld)",(long)(pmt_counts[i] + pmt_extras[i]));
						}
						break;
					case kLabelNowl:
						i = NHIT_PMT_OWL;
						goto Do_Pmt_Count;	// Yes, this is a goto
					case kLabelNlowGain:
						i = NHIT_PMT_LOW_GAIN;
						goto Do_Pmt_Count;	// Yes, this is a goto
					case kLabelNfecd:
						i = NHIT_PMT_FECD;
						goto Do_Pmt_Count;	// Yes, this is a goto
					case kLabelNbutts:
						i = NHIT_PMT_BUTTS;
						goto Do_Pmt_Count;	// Yes, this is a goto
					case kLabelNneck:
						i = NHIT_PMT_NECK;
						goto Do_Pmt_Count;	// Yes, this is a goto
					case kLabelDataType:
						PEventHistogram::GetHistogramLabel(data, pt);
						pt = strchr(pt, '\0');
						break;
					case kLabelDataMin:
						PEventHistogram::GetBins(data, &firstBin, &lastBin);
						len = GetPrecision(format, 1);
						pt += sprintf(pt,"%.*f",len,firstBin);
						break;
					case kLabelDataMax:
						PEventHistogram::GetBins(data, &firstBin, &lastBin);
						len = GetPrecision(format, 1);
						pt += sprintf(pt,"%.*f",len,lastBin);
						break;
					case kLabelPeak:
						pt += sprintf(pt,"%ld",(long)UNPK_MTC_PEAK(data->mtc_word));
						break;
					case kLabelInt:
						pt += sprintf(pt,"%ld",(long)UNPK_MTC_INT(data->mtc_word));
						break;
					case kLabelDiff:
						pt += sprintf(pt,"%ld",(long)UNPK_MTC_DIFF(data->mtc_word));
						break;
					case kLabelTrigger:
						pt += PEventInfoWindow::GetTriggerString(pt, data);
						break;
					case kLabelEvtNum:
						pt += sprintf(pt,"%ld",(long)data->event_num);
						break;
					case kLabelFileName:
						str = data->mEventFile.GetString();
						if (*str) {
							pt2 = strrchr(str,'/');
							if (pt2) str = pt2 + 1;
							pt += sprintf(pt,"%s",str);
						} else {
							pt += sprintf(pt,"<no file>");
						}
						break;
					case kLabelFitPos:
						if (data->nrcon) {
							rcon = data->rcon + data->curcon;
							len = GetPrecision(format,1);
							pt += sprintf(pt,"(%.*f,%.*f,%.*f)",
										len, rcon->pos[0] * data->tube_radius,
										len, rcon->pos[1] * data->tube_radius,
										len, rcon->pos[2] * data->tube_radius);
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelFitDir:
						if (data->nrcon) {
							rcon = data->rcon + data->curcon;
							len = GetPrecision(format, 3);
							pt += sprintf(pt,"(%.*f,%.*f,%.*f)",
										len, rcon->dir[0],
										len, rcon->dir[1],
										len, rcon->dir[2]);
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelFitRadius:
						if (data->nrcon) {
							rcon = data->rcon + data->curcon;
							len = GetPrecision(format, 1);
							pt += sprintf(pt,"%.*f", len, data->tube_radius *
										   sqrt(rcon->pos[0] * rcon->pos[0] +
												rcon->pos[1] * rcon->pos[1] +
												rcon->pos[2] * rcon->pos[2]));
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelFitTime:
						if (data->nrcon) {
							rcon = data->rcon + data->curcon;
							len = GetPrecision(format, 2);
							pt += sprintf(pt,"%.*f", len, rcon->time);
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelFitQuality:
						if (data->nrcon) {
							rcon = data->rcon + data->curcon;
							len = GetPrecision(format, 2);
							pt += sprintf(pt,"%.*f", len, rcon->chi_squared);
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelNfit:
						if (data->nrcon) {
							rcon = data->rcon + data->curcon;
							len = GetPrecision(format, 1);
							pt += sprintf(pt,"%ld", (long)rcon->fit_pmts);
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelSunDir:
						len = GetPrecision(format, 3);
						pt += sprintf(pt,"(%.*f,%.*f,%.*f)",
									len, data->sun_dir.x3,
									len, data->sun_dir.y3,
									len, data->sun_dir.z3);
						break;
					case kLabelSunAngle:
						if (data->nrcon) {
							Node *node = &data->sun_dir;
							rcon = data->rcon + data->curcon;
							mag = vectorLen(node->x3, node->y3, node->z3) *
								  vectorLen(rcon->dir[0], rcon->dir[1], rcon->dir[2]);
							if (mag) {
								dtmp = (node->x3 * rcon->dir[0] + node->y3 * rcon->dir[1] +
										node->z3 * rcon->dir[2]) / mag;
								float angle = acos(dtmp);
								if (data->angle_rad == 1) {
									len = GetPrecision(format, 3);
									pt += sprintf(pt,"%.*f rad", len, angle);
								} else {
									len = GetPrecision(format, 1);
									pt += sprintf(pt,"%.*f\xb0", len, angle * 180 / PI);
								}
							} else {
								pt += sprintf(pt,"-");
							}
						} else {
							pt += sprintf(pt,"<no fit>");
						}
						break;
					case kLabelPrevTime:
					case kLabelNextTime:
						entry = getCurrentHistoryEntry(data);
						if (entry) {
							if (flag == kLabelPrevTime) {
								dtmp = entry->prev_time;
							} else {
								dtmp = entry->next_time;
							}
						} else {
							dtmp = -1;
						}
						if (dtmp < 0) {
							*(pt++) = '?';
						} else {
							pt += PEventInfoWindow::GetRelativeTimeString(pt, dtmp);
						}
						break;
					case kLabelSubRun:
						pt += sprintf(pt,"%d",(int)data->sub_run);
						break;

				}
				fmtPt = NULL;	// no longer in format statement
			}
		} else if (!ch) {
			// we done our format string
			*pt = ch;	// null terminate string
			break;		// done
		} else if (ch == '%') {
			fmtPt = format;
			*(fmtPt++) = ch;
		} else if (pt >= last) {
			// stop now if our label is getting too big
			*pt = '\0';
			break;
		} else {
			*(pt++) = ch;	// copy to label string
		}
	}
	// add terminator for label text
	++lines;
	if (aTextOut) {
		aTextOut[lines].font = NULL;
		aTextOut[lines].string = NULL;
	}
	
	return(labelFlags);		// return label flags
}

// GetLabelText - make label if necessary and return pointer to the label text array
// - array is terminated by a string=NULL entry in the TextSpec array
// (must be called before GetLabelFlags() if label is dirty)
TextSpec * XSnoedWindow::GetLabelText()
{
	// only rebuild label if dirty flag is set
	if (mLabelDirty) {
		mLabelDirty = 0;	// reset label dirty flag
		mLabelFlags = BuildLabelString(mData, mLabelText, mData->label_format, mLabelString);
	}
	return mLabelText;
}

void XSnoedWindow::DoWater(int update_displays)
{
	int			i, j, num_waters=1;
	ImageData	*data = GetData();
	
	if (!data->waterLevel) return;
	
#ifdef ROOT_FILE
	Int_t	idate=0;
	float	ftime=0;
	if (data->root_water) num_waters = 2;
#endif

	for (i=0; i<num_waters; ++i) {
	
		if (data->watercon[i] >= 0) j = data->watercon[i];
		else j = data->nrcon;

		if (j >= MAX_RCON) {
			Printf("Maximum number of fits exceeded\n");
			break;
		} else {
#ifdef ROOT_FILE
			// get water level for this particular event time
			if (data->root_water) {
				if (!i) {
					// get date relative to ROOT time zero
					double ev_time;
					if (data->event_time) ev_time = data->event_time;
					else ev_time = double_time();
					double fdate = (ev_time - data->root_time_zero) / (24 * 60 * 60);
					idate = (Int_t)fdate;
					ftime = (fdate - idate) * (24 * 60 * 60);
					data->water_level[i] = data->root_water->GetH2OLevel(idate,ftime);
				} else {
					data->water_level[i] = data->root_water->GetD2OLevel(idate,ftime);
				}
			}
#endif
			if (data->nrcon <= j) data->nrcon = j + 1;
			if (data->curcon < 0) data->curcon = j;
			data->watercon[i] = j;
			data->rcon[j].pos[0] = 0;
			data->rcon[j].pos[1] = 0;
			data->rcon[j].pos[2] = data->water_level[i] / data->tube_radius;
			data->rcon[j].time = 0;
			data->rcon[j].dir[0] = 0;
			data->rcon[j].dir[1] = 0;
			data->rcon[j].dir[2] = -1;
			data->rcon[j].fit_pmts = 0;
			data->rcon[j].chi_squared = 0;
			data->rcon[j].cone_angle = PI/2;
			data->fit_nhitw[j] = 0;
			// set the fitter name (so we can recognize our fit later)
			strcpy(data->rcon[j].name, sWaterName[i]);
			// calculate the chernkov cone intersection
			setRconNodes( data->rcon + j );

			if (update_displays) {
				// transform the rcon nodes to the current projection
				GetImage()->Transform(data->rcon[j].nodes, data->rcon[j].num_nodes);

				if (i==num_waters-1) {
					// must re-calculate relative hit times whenever rcon changes
					// (only necessary if we are currently displaying time differences)
					if (data->wDataType == IDM_DELTA_T) {
						calcCalibratedVals(data);
					}
					// update the window title (must do this when curcon changes)
					newTitle(data);
				}
			}
		}
	}
	sendMessage(data,kMessageWaterChanged);
}


void XSnoedWindow::AboutXSnoed()
{
	Widget		label_text;
	Arg			warg;
	ImageData *	data = GetData();
	char	  * aboutStr1 = "XSNOED version " XSNOED_VERSION "\n"
							" \n"
							"Sudbury Neutrino Observatory\n"
							"X-Windows Event Display Program\n"
							" \n"
							"Copyright \xa9 1992-2014\n"
							"Philip Harvey, Queen's University\n"
							" \n";
	char	  * aboutStr2 = "Thanks to Peter Skensved, Mark Boulay and\n"
							"Richard Ford for their work on the fitter\n"
							"and calibration code, Bryce Moffat for the\n"
							"optical calibration routine, Yuen-Dat Chan\n"
							"and Reda Tafirout for the ZDAB reader and\n"
							"writer routines, and Nathaniel Tagg for his\n"
							"SNOMAN interface and VAX/HP-UX support.";
#ifndef NO_HELP
	char	  * aboutStr3 = "\n \n"
							"Press the Help button to browse\n"
							"the online XSNOED User Guide.";
#endif

	XtSetArg(warg, XmNtitle, "About XSnoed");
	aboutbox = XmCreateMessageDialog(data->toplevel, "xsnoedAbout", &warg, 1);
	
	XtUnmanageChild(XmMessageBoxGetChild(aboutbox,XmDIALOG_CANCEL_BUTTON));
#ifdef NO_HELP
	XtUnmanageChild(XmMessageBoxGetChild(aboutbox,XmDIALOG_HELP_BUTTON));
#endif

	label_text = XmMessageBoxGetChild(aboutbox,XmDIALOG_MESSAGE_LABEL);
	XtSetArg(warg, XmNalignment,XmALIGNMENT_CENTER);
	XtSetValues(label_text, &warg, 1);

	XmString str1, str2, str3;
	str1 = XmStringCreateLtoR(aboutStr1, XmFONTLIST_DEFAULT_TAG);
	str2 = XmStringCreateLtoR(aboutStr2, "SMALL");
	str3 = XmStringConcat(str1, str2);
	XmStringFree(str1);
	XmStringFree(str2);
#ifndef NO_HELP
	str1 = str3;
	str2 = XmStringCreateLtoR(aboutStr3, XmFONTLIST_DEFAULT_TAG);
	str3 = XmStringConcat(str1, str2);
	XmStringFree(str1);
	XmStringFree(str2);
#endif
	XtSetArg(warg, XmNlabelString, str3);
	XtSetValues(label_text, &warg, 1);
	XmStringFree(str3);

#ifndef NO_HELP
	XtAddCallback(aboutbox, XmNhelpCallback, (XtCallbackProc)HelpProc, aboutbox);
#endif
	XtAddCallback(aboutbox,XtNdestroyCallback,(XtCallbackProc)DestroyDialogProc,&aboutbox);
}

MenuList *XSnoedWindow::GetPopupMenuItem(int id)
{
	MenuList	*ms = mMenu->FindMenuItem(id);
	if (!ms) {
		Printf("Could not find popup menu struct %d!\n",id);
		exit(1);
	}
	return(ms);
}

void XSnoedWindow::SetHitMaskMenuToggles()
{
	ImageData	*data = GetData();
	
	GetMenu()->SetToggle(IDM_CUT_NORMAL,	 !(data->bit_mask & HIT_NORMAL));
	GetMenu()->SetToggle(IDM_CUT_OWL,		 !(data->bit_mask & HIT_OWL));
	GetMenu()->SetToggle(IDM_CUT_LOW_GAIN,	 !(data->bit_mask & HIT_LOW_GAIN));
	GetMenu()->SetToggle(IDM_CUT_NECK,	 	 !(data->bit_mask & HIT_NECK));
	GetMenu()->SetToggle(IDM_CUT_BUTTS,	 	 !(data->bit_mask & HIT_BUTTS));
	GetMenu()->SetToggle(IDM_CUT_FECD,		 !(data->bit_mask & HIT_FECD));
#ifndef SNOPLUS
	GetMenu()->SetToggle(IDM_CUT_SHAPER,	 !(data->bit_mask & HIT_SHAPER));
	GetMenu()->SetToggle(IDM_CUT_MUX,		 !(data->bit_mask & HIT_MUX));
	GetMenu()->SetToggle(IDM_CUT_SCOPE,		 !(data->bit_mask & HIT_SCOPE));
#endif
	GetMenu()->SetToggle(IDM_CUT_DISCARDED,	 !(data->bit_mask & HIT_DISCARDED));
	GetMenu()->SetToggle(IDM_CUT_UNDERSCALE, !(data->bit_mask & HIT_UNDERSCALE));
	GetMenu()->SetToggle(IDM_CUT_OVERSCALE,	 !(data->bit_mask & HIT_OVERSCALE));
}

void XSnoedWindow::SetDumpRecords(int dump_level)
{
	PZdabFile::SetVerbose(dump_level);
	GetData()->dump_records = dump_level;
	GetMenu()->SetToggle(IDM_DUMP_RECORDS, dump_level);
}

// SetupSum [static] - initialize or free necessary variables for sum mode
void XSnoedWindow::SetupSum(ImageData *data)
{
	Arg 	wargs[10];
	
	if (!data->sum) {
		free(data->sum_tac);
		free(data->sum_qhs);
		free(data->sum_qhl);
		free(data->sum_qlx);
        for (int i=0; i<8; ++i) {
            if (data->sum_caen[i]) {
                free(data->sum_caen[i]);
                data->sum_caen[i] = 0;
                data->sum_caen_samples[i] = 0;
            }
        }
		data->sum_tac = data->sum_qhs = data->sum_qhl = data->sum_qlx = 0;
		/* display current event */
		showHistory(data,0);
	} else {
		data->sum_tac = (u_int32 *)XtMalloc(NUM_TOTAL_CHANNELS * sizeof(u_int32));
		data->sum_qhs = (u_int32 *)XtMalloc(NUM_TOTAL_CHANNELS * sizeof(u_int32));
		data->sum_qhl = (u_int32 *)XtMalloc(NUM_TOTAL_CHANNELS * sizeof(u_int32));
		data->sum_qlx = (u_int32 *)XtMalloc(NUM_TOTAL_CHANNELS * sizeof(u_int32));
		if (!data->sum_tac || !data->sum_qhs || !data->sum_qhl || !data->sum_qlx) {
			data->sum = 0;	// reset sum flag
			Printf("Not enough memory to allocate arrays for sum!\n");
			XtSetArg(wargs[0], XmNset, FALSE);
			MenuList *ms = PMenu::GetCurMenuItem();
			XtSetValues(ms->button,wargs,1);
			if (data->sum_tac) { free(data->sum_tac); data->sum_tac = 0; }
			if (data->sum_qhs) { free(data->sum_qhs); data->sum_qhs = 0; }
			if (data->sum_tac) { free(data->sum_tac); data->sum_tac = 0; }
			if (data->sum_qlx) { free(data->sum_qlx); data->sum_qlx = 0; }
			for (int i=0; i<8; ++i) {
			    if (data->sum_caen[i]) {
			        free(data->sum_caen[i]);
			        data->sum_caen[i] = 0;
			        data->sum_caen_samples[i] = 0;
			    }
			}
		} else {
			clearSum(data);
			/* put the currently displayed event into the sum */
			showHistory(data,0);
		}
	}
	if (data->mWindow[EVT_INFO_WINDOW]) {
		((PEventInfoWindow *)data->mWindow[EVT_INFO_WINDOW])->SetSum(data->sum);
	}
}

// ShowWindow - show the specified XSNOED window
void XSnoedWindow::ShowWindow(int id)
{
	// handle window commands
	PWindow	*win = GetData()->mWindow[id];
	
	if (win) {
		win->Raise();	// raise window to top
	} else {
		CreateWindow(id);
	}
}

void XSnoedWindow::WarnQuit()
{
	// open warning dialog
	if (mWarnDialog) {
		XtDestroyWidget(mWarnDialog);
		mWarnDialog = NULL;
	}
	XmString	str;
	Arg			wargs[10];
	int			n;
	str = XmStringCreateLocalized("Really Quit XSNOED?  ");
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Quit XSNOED"); ++n;
	XtSetArg(wargs[n], XmNmessageString, str); ++n;
	XtSetArg(wargs[n], XmNdefaultButtonType, XmDIALOG_OK_BUTTON); ++n;
	mWarnDialog = XmCreateWarningDialog(GetShell(), "xsnoedWarn",wargs,n);
	XmStringFree(str);	// must free the string
	XtUnmanageChild(XmMessageBoxGetChild(mWarnDialog,XmDIALOG_HELP_BUTTON));
/*	// change the "OK" label to "Quit"
	Widget but = XmMessageBoxGetChild(mWarnDialog,XmDIALOG_OK_BUTTON);
	n = 0;
	str = XmStringCreateLocalized("Quit");
	XtSetArg(wargs[n], XmNlabelString, str); ++n;
	XtSetValues(but, wargs, n);
	XmStringFree(str);
*/
	XtAddCallback(mWarnDialog,XmNcancelCallback,(XtCallbackProc)WarnCancelProc,this);
	XtAddCallback(mWarnDialog,XmNokCallback,(XtCallbackProc)WarnOKProc,this);
	XtAddCallback(mWarnDialog,XtNdestroyCallback,(XtCallbackProc)WarnDestroyProc,this);
	XtManageChild(mWarnDialog);
}

//---------------------------------------------------------------------------------
// Assorted callbacks
//
void XSnoedWindow::WarnCancelProc(Widget w, XSnoedWindow *win, caddr_t call_data)
{
	// delete the warning dialog
	XtDestroyWidget(win->mWarnDialog);
	win->mWarnDialog = NULL;
}

void XSnoedWindow::WarnOKProc(Widget w, XSnoedWindow *win, caddr_t call_data)
{
	XtDestroyWidget(win->mWarnDialog);
	win->mWarnDialog = NULL;
	// quit XSNOED
	xsnoed_delete(win->GetData());
}

// the warning dialog was destroyed
void XSnoedWindow::WarnDestroyProc(Widget w, XSnoedWindow *win, caddr_t call_data)
{
	// must verify that it is the current dialog being destroyed
	// (could a delayed callback from one we destroyed ourself already)
	if (win->mWarnDialog == w) {
		win->mWarnDialog = NULL;
	}
}
void XSnoedWindow::FileOK(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data)
{
	XtUnmanageChild(data->mMainWindow->filebox);
	
	char filename[FILELEN];
	strncvtXm(filename, call_data->value, FILELEN);
	data->mEventFile.SetString(filename);
	data->event_num = 0;
	open_event_file(data,0);
	
	if (data->infile) {
		data->mMainWindow->ShowWindow(EVT_NUM_WINDOW);
	} else {
		/* re-open file dialog because the file didn't open */
		XtManageChild(data->mMainWindow->filebox);
	}
	// this causes bad crashes
	//	XtDestroyWidget(data->mMainWindow->filebox);
}

void XSnoedWindow::FileCancel(Widget w, ImageData *data, XmFileSelectionBoxCallbackStruct *call_data)
{
	XtUnmanageChild(data->mMainWindow->filebox);
//	XtDestroyWidget(data->mMainWindow->filebox);
}
void XSnoedWindow::DestroyDialogProc(Widget w, Widget **dialogPtr, caddr_t call_data)
{
	/* must zero our pointer since the dialog is gone */
	*dialogPtr = NULL;
}
#ifndef NO_HELP
void XSnoedWindow::HelpProc(Widget w, Widget dialog, caddr_t call_data)
{
	char	buff[512];
	
	if (dialog) {
		XtUnmanageChild(dialog);
// this worked, but let's keep the dialog around (like with the OK button)
//		XtDestroyWidget(dialog);
	}
	if (!strcmp(sBrowserName,"netscape")) {
		// look for existing netscape and send remote message
		sprintf(buff,"openURL(%s)",PResourceManager::sResource.help_url);
		Display *dpy = PResourceManager::sResource.display;
		mozilla_remote_init_atoms(dpy);
		Window window = mozilla_remote_find_window(dpy);
		if (window) {
			XSelectInput (dpy, window, (PropertyChangeMask|StructureNotifyMask));
	  		if (!mozilla_remote_obtain_lock(dpy, window)) {
				int status = mozilla_remote_command (dpy, window, buff, 1);
				if (status != 6) mozilla_remote_free_lock (dpy, window);
				if (!status) {
					Printf("Netscape already running -- loading XSNOED documentation...\n");
					return;
				}
			}
		}
	}
	
	// fork and exec a new netscape
	if (!fork()) {
		Printf("Running %s...\n",sBrowserName);
		execlp(sBrowserName, sBrowserName, PResourceManager::sResource.help_url, NULL);
		Printf("Error launching %s\x07\n",sBrowserName);
		_exit(1);
	}
}
#endif // NO_HELP
#ifndef NO_DISPATCH
void XSnoedWindow::DisconnectProc(Widget w, ImageData *data, caddr_t call_data)
{
	CloseDispLink();
	delete data->mWindow[DISP_WINDOW];
}
void XSnoedWindow::ConnectProc(Widget w, ImageData *data, caddr_t call_data)
{
	char *str = XmTextGetString(data->mMainWindow->disp_text);
	SetDispName(str);
	XtFree(str);
	
	clearHistoryAll(data);		// clear 'all' history
	handleHeaderRecord(data,NULL,0);	// clear any existing run record
	InitDispLink(data,1);
	delete data->mWindow[DISP_WINDOW];
	if (IsDispConnected(data)) {
		data->mMainWindow->ShowWindow(EVT_NUM_WINDOW);
	}
}
#endif

void XSnoedWindow::CancelProc(Widget w, Widget aShell, caddr_t call_data)
{
	XtDestroyWidget(aShell);
}

//====== Begin DEMO_VERSION code ====================================
#ifdef DEMO_VERSION
// static member declarations
int		XSnoedWindow::sProtect	= 0;
void XSnoedWindow::ProtectMenuItems(MenuList *ms, int on)
{
	// list of menu items enabled in protected mode
	static int enabled_menu_items[] = {
		IDM_GEODESIC,
		IDM_POLAR,
		IDM_NO_FRAME,
		IDM_PANE_FLAT,
		IDM_PANE_CRATE,
		IDM_PANE_PROJ,
		IDM_PANE_NCD,
		IDM_ABOUT,
		IDM_HELP,
		IDM_MOVE_HOME,
		IDM_MOVE_TOP,
		IDM_DATA_MENU,
		IDM_TAC,
		IDM_QHS,
		IDM_QHL,
		IDM_QLX,
		IDM_NEXT_EVENT,
		IDM_PREV_EVENT,
		IDM_VESSEL,
		IDM_PROJ_TO_HOME,
		IDM_PROJ_3D,
		IDM_PROJ_FLAT,
		IDM_PROJ_CRATE,
		IDM_PROJ_RECTANGULAR,
		IDM_PROJ_SINUSOID,
		IDM_PROJ_ELLIPTICAL,
		IDM_PROJ_MOLLWEIDE,
		IDM_PROJ_HAMMER,
		IDM_PROJ_EXTENDED_HAMMER,
		IDM_PROJ_POLAR,
		IDM_PROJ_POLAR_EQUAL,
		IDM_PROJ_DUAL_SINUSOID,
		IDM_PROJ_DUAL_ELLIPTICAL,
		IDM_PROJ_DUAL_MOLLWEIDE,
		IDM_PROJ_DUAL_HAMMER,
		IDM_PROJ_DUAL_POLAR,
		IDM_PROJ_DUAL_POLAR_EQUAL,
		IDM_DETECTOR_COORD,
		IDM_PROTECT,
		0
	};
	
	while (ms) {
		if (ms->id && ms->button) {
			for (int i=0; ; ) {
				// this item was on the list -- leave it alone
				if (ms->id == enabled_menu_items[i]) break;
				if (!enabled_menu_items[++i]) {
					// this item wasn't on the list -- protect or unprotect it
					PMenu::ProtectItem(ms, on);
					break;
				}
			}
		}
		if (ms->sub_menu) {
			// protect necessary items in sub-menu
			ProtectMenuItems(ms->sub_menu, on);
		}
		ms = ms->next;
	}
}
int XSnoedWindow::IsProtected()
{
	if (sProtect) {
		Printf("Sorry - this feature is protected\x07\n");
	}
	return(sProtect);
}
void XSnoedWindow::SetProtect(int on)
{
	int			n;
	Arg			wargs[5];
	ImageData *	data = GetData();
	
	// protect/unprotect required menu items
	ProtectMenuItems(GetMenu()->GetMenuList(), on);
	
	n = 0;
	if (on) {
		Printf("XSNOED features protected for demo mode\n");
		GetMenu()->SetLabel(IDM_PROTECT,"Unprotect");
		XtSetArg(wargs[n], XmNdeleteResponse, XmDO_NOTHING); ++n;
	} else {
		Printf("All XSNOED features unprotected\n");
		GetMenu()->SetLabel(IDM_PROTECT,"Protect");
		XtSetArg(wargs[n], XmNdeleteResponse, XmDESTROY); ++n;
	}
	XtSetValues(GetShell(), wargs, n);
	for (int i=0; i<NUM_WINDOWS; ++i) {
		if (data->mWindow[i]) {
			XtSetValues(data->mWindow[i]->GetShell(), wargs, n);
		}
	}
	sProtect = on;
}
void XSnoedWindow::PasswordOK(Widget w, XSnoedWindow *win, caddr_t call_data)
{
	ImageData *data = win->GetData();
	char *pt1 = win->mPassword;
	
	if (pt1) for (char *pt2=data->password; ; ++pt1, ++pt2) {
		// compare password, ignoring case
		if (toupper(*pt1) != toupper(*pt2)) break;
		if (!*pt1) {
			// password is correct -- unprotect xsnoed
			win->SetProtect(0);
			delete data->mWindow[PASSWORD_WINDOW];
			return;
		}
	}
	setTextString(win->pass_text, "");
	setLabelString(win->pass_label, "Wrong password -- Try again:");
}
// CheckPassword() -- handle password input
void XSnoedWindow::CheckPassword(Widget w, XSnoedWindow *win, caddr_t call_data)
{
	char *passwd = win->mPassword;
	XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *) call_data;

	if (cbs->text->length > 1) {
		cbs->doit = False; // don't allow paste operations --
		return;            // make the user type the password
	}

	if (cbs->startPos < cbs->endPos) {
		// shrink the password by moving endPos... characters forward
		// to the point of deletion
		memmove( passwd + cbs->startPos,
				 passwd + cbs->endPos,
	         	 strlen(passwd) - cbs->endPos + 1 );
         	 
        // then just a delete, not a replace
		if (cbs->text->length == 0)  return;
	}

	if (passwd) {
		// open a hole for the new characters,
		// and insert them in the proper place
		passwd = XtRealloc( passwd, strlen(passwd) + cbs->text->length + 1 );
		memmove( passwd + cbs->startPos + cbs->text->length,
		         passwd + cbs->startPos,
		         strlen(passwd) - cbs->startPos + 1 );
		memcpy( passwd + cbs->startPos, cbs->text->ptr, cbs->text->length );
	} else {
		passwd = XtMalloc( cbs->text->length + 1 );
		strncpy( passwd, cbs->text->ptr, cbs->text->length ); // just the new chars
		passwd[cbs->text->length] = '\0';
	}
	win->mPassword = passwd;

	// display '*' for password characters
	memset( cbs->text->ptr, '*', cbs->text->length );
}
#endif // DEMO_VERSION
//====== End DEMO_VERSION code =====================================


//-----------------------------------------------------------------------------------
// CreateWindow - create a sub window with the specified ID
//
void XSnoedWindow::CreateWindow(int anID)
{
	int			n;
	int			min_width, min_height;
	Arg			wargs[20];
	Widget		window, w;
	ImageData	*data = GetData();
	PImageWindow *pwin = NULL;
	char		buff[128];
#if !defined(NO_DISPATCH) || defined(DEMO_VERSION)
	XmString	str;
	Widget		but;
#endif
	
	switch (anID) {
		
#ifndef XSNOMAN
	  case EVT_NUM_WINDOW:	// Create Event control window
		data->mWindow[anID] = new PEventControlWindow(data);    	
		break;
#endif

#ifdef XSNOMAN
	  case SNOMAN_WINDOW: // Create Snoman interface window
		data->mWindow[anID] = new PSnomanWindow(data);    	
		break;	        
		
	  case DAMN_WINDOW: // Create Snoman DAMN interface window
		data->mWindow[anID] = new PDamnWindow(data);
		break;
#endif

	  case SETTINGS_WINDOW:	// Create Event control window
		data->mWindow[anID] = new PSettingsWindow(data);    	
		break;

	  case HIT_INFO_WINDOW:	// Create Hit info window
	  	data->mWindow[anID] = new PHitInfoWindow(data);
		break;

	  case NCD_HIT_INFO_WINDOW:	// Create Hit info window
	  	data->mWindow[anID] = new PNCDHitInfoWindow(data);
		break;

	  case RECORD_INFO_WINDOW:	// Create Run info window
	  	data->mWindow[anID] = new PRecordInfoWindow(data);
		break;

	  case EVT_INFO_WINDOW:	// Create Event info window
	  	data->mWindow[anID] = new PEventInfoWindow(data);
		break;

#ifdef SNOPLUS
      case CAEN_WINDOW:
	  	data->mWindow[anID] = new PCaenWindow(data);
		break;
#else
	  case NCD_INFO_WINDOW:	// Create NCD info window
	  	data->mWindow[anID] = new PNCDInfoWindow(data);
		break;

	  case NCD_HIST_WINDOW:	// Create NCD histogram window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "NCD Histogram"); ++n;
		XtSetArg(wargs[n], XmNx, 425); ++n;
		XtSetArg(wargs[n], XmNy, 325); ++n;
		XtSetArg(wargs[n], XmNminWidth, 200); ++n;
		XtSetArg(wargs[n], XmNminHeight, 100); ++n;
		window = CreateShell("nhPop",data->toplevel,wargs,n);
		n = 0;
		XtSetArg(wargs[n], XmNwidth, 400); ++n;
		XtSetArg(wargs[n], XmNheight, 250); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install an event histogram in the new window
		(void)new PNCDHistogram(pwin);
		break;

	  case NCD_MAP_WINDOW:	// Create NCD map window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "NCD Map"); ++n;
		XtSetArg(wargs[n], XmNx, 150); ++n;
		XtSetArg(wargs[n], XmNy, 190); ++n;
		XtSetArg(wargs[n], XmNminWidth, 100); ++n;
		XtSetArg(wargs[n], XmNminHeight, 100); ++n;
		window = CreateShell("ncdPop",data->toplevel,wargs,n);
		n = 0;			
		XtSetArg(wargs[n], XmNwidth, 400); ++n;
		XtSetArg(wargs[n], XmNheight, 400); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install an NCD image in the new window
		(void)new PNCDImage(pwin);
		break;

	  case NCD_SCOPE_WINDOW:// Create NCD scope window
	  	data->mWindow[anID] = new PNCDScopeWindow(data);
		break;
#endif // SNOPLUS

#ifdef FITTR
	  case FITTER_WINDOW:	// Create fit window
	  	data->mWindow[anID] = new PFitterWindow(data);
   		break;
#endif

#ifdef OPTICAL_CAL
	  case OPTICAL_WINDOW:	// Create Optical Calibration window
	  	data->mWindow[anID] = new POpticalWindow(data);
	  	break;
#endif

	  case MONTE_CARLO_WINDOW:
	  	data->mWindow[anID] = new PMonteCarloWindow(data);
	  	break;

	  case HIST_WINDOW:	// Create histogram window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "Event Histogram"); ++n;
		XtSetArg(wargs[n], XmNx, 225); ++n;
		XtSetArg(wargs[n], XmNy, 225); ++n;
		XtSetArg(wargs[n], XmNminWidth, 200); ++n;
		XtSetArg(wargs[n], XmNminHeight, 100); ++n;
		window = CreateShell("ehPop",data->toplevel,wargs,n);
		n = 0;
		XtSetArg(wargs[n], XmNwidth, 400); ++n;
		XtSetArg(wargs[n], XmNheight, 250); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install an event histogram in the new window
		(void)new PEventHistogram(pwin);
		break;

	  case SCOPE_WINDOW:	// Create trigger scope window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "Trigger Scope"); ++n;
		XtSetArg(wargs[n], XmNx, 225); ++n;
		XtSetArg(wargs[n], XmNy, 225); ++n;
		XtSetArg(wargs[n], XmNminWidth, 200); ++n;
		XtSetArg(wargs[n], XmNminHeight, 200); ++n;
		window = CreateShell("scopePop",data->toplevel,wargs,n);
		n = 0;
		XtSetArg(wargs[n], XmNwidth,  500); ++n;
		XtSetArg(wargs[n], XmNheight, 450); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// create new Scope Window
		(void)new PScopeImage(pwin);
		//(void)new PEventScope(pwin);
		break;

	  case TIME_WINDOW:	// Create event time window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "Event Times"); ++n;
		XtSetArg(wargs[n], XmNx, 225); ++n;
		XtSetArg(wargs[n], XmNy, 225); ++n;
		XtSetArg(wargs[n], XmNminWidth, 200); ++n;
		XtSetArg(wargs[n], XmNminHeight, 100); ++n;
		window = CreateShell("etPop",data->toplevel,wargs,n);
		n = 0;
		XtSetArg(wargs[n], XmNwidth, 400); ++n;
		XtSetArg(wargs[n], XmNheight, 250); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install an event times histogram in the new window
		(void)new PEventTimes(pwin);
		break;
		
	  case COLOUR_WINDOW: // Create colour window
	  	data->mWindow[anID] = new PColourWindow(data);
	  	break;

	  case ASCII_WINDOW:
	  	data->mWindow[anID] = new PAsciiWindow(data);
	  	break;
	  	
	  case ANIMATION_WINDOW:
	  	data->mWindow[anID] = new PAnimationWindow(data);
	  	break;

	  //added MAH 04/21/00
	  case DUMP_DATA_WINDOW:
	  	data->mWindow[anID] = new PDumpDataWindow(data);
	  	break;

#ifdef ROOT_FILE
	  case SNODB_WINDOW:
	  	data->mWindow[anID] = new PSnoDBWindow(data);
	  	break;
	  case RCH_TIME_WINDOW:
	  	QRchHist::CreateRchWindow(data,"RchTime", anID);
	  	break;
	  case RCH_QHS_WINDOW:
	  	QRchHist::CreateRchWindow(data,"RchQhs", anID);
	  	break;
	  case RCH_QHL_WINDOW:
	  	QRchHist::CreateRchWindow(data,"RchQhl", anID);
	  	break;
	  case RCH_QLX_WINDOW:
	  	QRchHist::CreateRchWindow(data,"RchQlx", anID);
	  	break;
#endif
	  
	  case FLAT_WINDOW:	// Create flat map window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "Flat Map"); ++n;
		XtSetArg(wargs[n], XmNx, 250); ++n;
		XtSetArg(wargs[n], XmNy, 250); ++n;
		XtSetArg(wargs[n], XmNminWidth, 210); ++n;
		XtSetArg(wargs[n], XmNminHeight, 100); ++n;
		window = CreateShell("flatPop",data->toplevel,wargs,n);
		n = 0;			
		XtSetArg(wargs[n], XmNwidth, 800); ++n;
		XtSetArg(wargs[n], XmNheight, 400); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install a flat image in the new window
		(void)new PFlatImage(pwin);
	 	break;

	  case PROJ_WINDOW:	// Create Projection window
	  	sprintf(buff,"%s Projection", data->projName);
		n = 0;
		XtSetArg(wargs[n], XmNtitle, buff); ++n;
		XtSetArg(wargs[n], XmNx, 250); ++n;
		XtSetArg(wargs[n], XmNy, 250); ++n;
		XtSetArg(wargs[n], XmNminWidth, 100); ++n;
		XtSetArg(wargs[n], XmNminHeight, 100); ++n;
		window = CreateShell("tpPop",data->toplevel,wargs,n);
		n = 0;
		XtSetArg(wargs[n], XmNwidth, 600); ++n;
		XtSetArg(wargs[n], XmNheight, 350); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install a map image in the new window
		(void)new PMapImage(pwin);
		break;

	  case CRATE_WINDOW:	// Create crate window
		n = 0;
		XtSetArg(wargs[n], XmNtitle, "Crate Map"); ++n;
		XtSetArg(wargs[n], XmNx, 250); ++n;
		XtSetArg(wargs[n], XmNy, 350); ++n;
		XtSetArg(wargs[n], XmNminWidth, 265); ++n;
		XtSetArg(wargs[n], XmNminHeight, 125); ++n;
		window = CreateShell("cratePop",data->toplevel,wargs,n);
		n = 0;
		XtSetArg(wargs[n], XmNwidth, 800); ++n;
		XtSetArg(wargs[n], XmNheight, 330); ++n;
		w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
		data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
		// install a crate image in the new window
		(void)new PCrateImage(pwin);
	  	break;

#ifndef NO_DISPATCH
	  case DISP_WINDOW:	{// Create Dispatcher window
	  	n = 0;
		XtSetArg(wargs[n], XmNtitle, "Connect to Dispatcher"); ++n;
		XtSetArg(wargs[n], XmNx, 300); ++n;
		XtSetArg(wargs[n], XmNy, 300); ++n;
		XtSetArg(wargs[n], XmNminWidth, 340); ++n;
		XtSetArg(wargs[n], XmNminHeight, 140); ++n;
		window = CreateShell("dispPop",data->toplevel,wargs,n);
		w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,window,NULL,0);
		data->mWindow[anID] = new PWindow(data,window,w);

		n = 0;
		XtSetArg(wargs[n], XmNx, 30); ++n;
		XtSetArg(wargs[n], XmNy, 17); ++n;
		XtSetArg(wargs[n], XmNlabelString, str = XmStringCreate("Dispatcher Name:",XmFONTLIST_DEFAULT_TAG)); ++n;
    	XtCreateManagedWidget("dispname", xmLabelWidgetClass, w, wargs, n);
    	XmStringFree(str);
		n = 0;
		XtSetArg(wargs[n], XmNx, 30); ++n;
		XtSetArg(wargs[n], XmNy, 47); ++n;
		XtSetArg(wargs[n], XmNwidth, 280); ++n;
    	disp_text = XtCreateManagedWidget("disptext", xmTextWidgetClass, w, wargs, n);
		XtAddCallback(disp_text,XmNactivateCallback,(XtCallbackProc)ConnectProc,data);
		setTextString(disp_text, GetDispName());
		n = 0;
		XtSetArg(wargs[n], XmNx, 30); ++n;
		XtSetArg(wargs[n], XmNy, 90); ++n;
		but = XtCreateManagedWidget("Connect",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)ConnectProc, data);
		n = 0;
		XtSetArg(wargs[n], XmNx, 244); ++n;
		XtSetArg(wargs[n], XmNy, 90); ++n;
		XtSetArg(wargs[n], XmNmarginLeft, 3); ++n;
		XtSetArg(wargs[n], XmNmarginRight, 3); ++n;
		but = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)CancelProc, window);
		n = 0;
		XtSetArg(wargs[n], XmNx, 125); ++n;
		XtSetArg(wargs[n], XmNy, 90); ++n;
		but = XtCreateManagedWidget("Disconnect",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)DisconnectProc, data);
    	n = 0;
    	XtSetArg(wargs[n], XmNinitialFocus, disp_text); ++n;
    	XtSetValues(w, wargs, n);
	}	break;
#endif

#ifdef DEMO_VERSION
	  case PASSWORD_WINDOW:	{// Create Password window
	  	// reset password string
	  	if (mPassword) *mPassword = '\0';
	  	n = 0;
		XtSetArg(wargs[n], XmNtitle, "Unprotect"); ++n;
		XtSetArg(wargs[n], XmNx, 300); ++n;
		XtSetArg(wargs[n], XmNy, 300); ++n;
		XtSetArg(wargs[n], XmNminWidth, 340); ++n;
		XtSetArg(wargs[n], XmNminHeight, 140); ++n;
		window = CreateShell("passPop",data->toplevel,wargs,n);
		w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,window,NULL,0);
		data->mWindow[anID] = new PWindow(data,window,w);

		n = 0;
		XtSetArg(wargs[n], XmNx, 30); ++n;
		XtSetArg(wargs[n], XmNy, 17); ++n;
		XtSetArg(wargs[n], XmNlabelString, str = XmStringCreate("Enter Password:",XmFONTLIST_DEFAULT_TAG)); ++n;
    	pass_label = XtCreateManagedWidget("passname", xmLabelWidgetClass, w, wargs, n);
    	XmStringFree(str);
		n = 0;
		XtSetArg(wargs[n], XmNx, 30); ++n;
		XtSetArg(wargs[n], XmNy, 47); ++n;
		XtSetArg(wargs[n], XmNwidth, 280); ++n;
    	pass_text = XtCreateManagedWidget("passtext", xmTextWidgetClass, w, wargs, n);
		XtAddCallback(pass_text, XmNmodifyVerifyCallback, (XtCallbackProc)CheckPassword, this);
		XtAddCallback(pass_text, XmNactivateCallback, (XtCallbackProc)PasswordOK, this);
		// set initial focus to the password text widget
		n = 0;
    	XtSetArg(wargs[n], XmNinitialFocus, pass_text); ++n;
    	XtSetValues(w, wargs, n);
		n = 0;
		XtSetArg(wargs[n], XmNx, 30); ++n;
		XtSetArg(wargs[n], XmNy, 90); ++n;
		XtSetArg(wargs[n], XmNwidth, 70); ++n;
		but = XtCreateManagedWidget("OK",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)PasswordOK, this);
		n = 0;
		XtSetArg(wargs[n], XmNx, 240); ++n;
		XtSetArg(wargs[n], XmNy, 90); ++n;
		XtSetArg(wargs[n], XmNwidth, 70); ++n;
		but = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,w,wargs,n);
    	XtAddCallback(but, XmNactivateCallback, (XtCallbackProc)CancelProc, window);
    	n = 0;
	}	break;
#endif

	  case PRINT_WINDOW:	// Create print window
	  	data->mWindow[anID] = new PPrintWindow(data, (EPrintType)mPrintType);
		break;

	  default:
	  	return;
	}
	if (data->mWindow[anID]) {
		data->mWindow[anID]->Show();		// show the window
		// resize necessary windows
		if (!data->mWindow[anID]->WasResized()) {
			switch (anID) {
				case DISP_WINDOW:
				case PRINT_WINDOW:
				case PASSWORD_WINDOW:
					n = 0;
					XtSetArg(wargs[n], XmNminWidth, &min_width); ++n;
					XtSetArg(wargs[n], XmNminHeight, &min_height); ++n;
					XtGetValues(data->mWindow[anID]->GetShell(), wargs, n);
					data->mWindow[anID]->Resize(min_width, min_height);
					break;
			}
		}
	}
}

// Is3d - return TRUE if the specified geometry is 3-dimensional
int XSnoedWindow::Is3d(int geo)
{
	return(geo==IDM_GEODESIC || geo==IDM_POLAR || geo==IDM_NO_FRAME);
}

// CheckMenuCommand - check state of a menu command prior to opening of the menu
int XSnoedWindow::CheckMenuCommand(int anID, int flags)
{
	switch (anID) {
		case IDM_VESSEL:
		case IDM_HIT_LINES:
		case IDM_RCON_LINES:
		case IDM_MOVE_SUN:
		case IDM_MOVE_SPHERE:
		case IDM_MOVE_EVENT:
		case IDM_AUTO_SUN:
		case IDM_AUTO_VERTEX:
		case IDM_MOVE_TOP:
			// can only do these commands with a 3-D geometry
			PMenu::SetEnabled(&flags, Is3d(GetData()->wGeo));
			break;
		case IDM_MOVE_VERTEX:
			// can only move to vertex if we are 3D and have a fit
			PMenu::SetEnabled(&flags, Is3d(GetData()->wGeo) && GetData()->nrcon);
			break;
		case IDM_NEXT_VERTEX:
			// can only do "Next Fit" if we have more than one fit
			PMenu::SetEnabled(&flags, GetData()->nrcon > 1);
			break;
	}
	return(flags);
}


//--------------------------------------------------------------------------------------
// DoMenuCommand
//
void XSnoedWindow::DoMenuCommand(int anID)
{
	int			n;
	Arg			wargs[10];
	ImageData	*data = GetData();
	PHistImage	*hist;
				
	if (anID < NUM_WINDOWS) {	// Is this a window menu command?
	
		ShowWindow(anID);		// Yes -- show the window
		
#ifdef ROOT_FILE
		// Open the "Open Rch File" dialog when we show any Rch Window
		// if an Rch file isn't already open
		if (!data->root_rch_file && anID>=RCH_TIME_WINDOW && anID<=RCH_QLX_WINDOW) {
			// show the "Open Rch File" dialog
			QRchHist::OpenRchFile(data, NULL);
		}
#endif

	} else switch (anID) {
	
		case IDM_NEW_DISPLAY:
			(void)new XSnoedWindow;
			break;
			
		case IDM_UNIFORM:
			loadUniformHits(data);
			break;
			
		case IDM_DUMP_RECORDS:
			data->dump_records = !data->dump_records;
			PZdabFile::SetVerbose(data->dump_records);
			break;
			
		case IDM_ABOUT:
			if (aboutbox) {
				XtUnmanageChild(aboutbox);
			} else {
				AboutXSnoed();
			}
			XtManageChild(aboutbox);
			break;
			
		case IDM_LOAD_EVENT:
		{
			Widget	file_sel_box;
			XmString	title,dir_mask;
			
			if (!filebox) {
				n = 0;
				XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS);	++n;
				XtSetArg(wargs[n], XmNtitle, "Open Event File"); ++n;
				filebox = XmCreateBulletinBoardDialog(data->toplevel, "xsnoedFileBox", wargs, n);
				
				title = XmStringCreateLtoR("Open event file...", XmFONTLIST_DEFAULT_TAG);
				dir_mask = XmStringCreateLtoR("*", XmFONTLIST_DEFAULT_TAG);
				n = 0;
				XtSetArg(wargs[n], XmNdirMask, dir_mask);	++n;
				XtSetArg(wargs[n], XmNfilterLabelString, title);	++n;
				file_sel_box = XmCreateFileSelectionBox(filebox, "selectionbox", wargs, n);
				
				XtUnmanageChild(XmFileSelectionBoxGetChild(file_sel_box,XmDIALOG_HELP_BUTTON));
				
				XtAddCallback(file_sel_box, XmNokCallback, (XtCallbackProc)FileOK, data);
				XtAddCallback(file_sel_box, XmNcancelCallback, (XtCallbackProc)FileCancel, data);
				XmStringFree(title);
				XmStringFree(dir_mask);
				XtManageChild(file_sel_box);
				XtAddCallback(filebox,XtNdestroyCallback,(XtCallbackProc)DestroyDialogProc,&filebox);
				
			} else if (XtIsManaged(filebox)) {
			
				/* unmanage then manage again to raise window to top */
				XtUnmanageChild(filebox);
		//		XRaiseWindow(data->display,XtWindow(filebox));
		//		return;
			}
			XtManageChild(filebox);
		} break;
		
#ifdef ROOT_FILE
		case IDM_OPEN_RCH:
			QRchHist::OpenRchFile(data, NULL);
			break;
#endif
		case IDM_CLOSE_FILE:
			close_event_file(data);
#ifdef ROOT_FILE
			if (data->owns_rch_file) {
				data->owns_rch_file = 0;
				delete data->root_rch_file;
			}
			data->root_rch_file = NULL;
#endif
			break;
			
// no Print Window feature if we can't fork a process
#ifndef NO_FORK
		case IDM_PRINT_WINDOW:
			mPrintType = kPrintWindow;
			if (data->mWindow[PRINT_WINDOW]) {
				((PPrintWindow *)data->mWindow[PRINT_WINDOW])->SetPrintType((EPrintType)mPrintType);
			} else {
				CreateWindow(PRINT_WINDOW);
			}
			break;
#endif // NO_FORK
			
		case IDM_PRINT_IMAGE:
			mPrintType = kPrintImage;
			if (data->mWindow[PRINT_WINDOW]) {
				((PPrintWindow *)data->mWindow[PRINT_WINDOW])->SetPrintType((EPrintType)mPrintType);
			} else {
				CreateWindow(PRINT_WINDOW);
			}
			break;
			
		case IDM_SAVE_SETTINGS:
			data->mMainWindow->SaveResources(1);
			break;
			
		case IDM_NEXT_EVENT:
			xsnoed_next(data,1);
			break;
			
		case IDM_PREV_EVENT:
			xsnoed_next(data,-1);
			break;
			
		case IDM_CLEAR_EVENT:
			xsnoed_clear(data);
        	handleHeaderRecord(data,NULL,0);	// clear run records too
			break;

		case IDM_SUM_EVENT:
			data->sum ^= 1;
			SetupSum(data);
			break;
		
		case IDM_FIT_EVENT:
			data->autoFit ^= 1;
			if (data->autoFit) {
				doFit(data, 1);
			}
			break;

#ifndef NO_HELP			
		case IDM_HELP:
			HelpProc(NULL,NULL,NULL);
			break;
#endif

		case IDM_QUIT:
			if (PMenu::WasAccelerator()) {
				WarnQuit();
			} else {
				xsnoed_delete(data);
			}
			break;
			
#ifdef XSNOMAN
		case IDM_QUIT_SNOMAN:
			xsnoman_quit_confirm(data);
			break;
#endif
		case IDM_CUT_ALL:
			if ((data->bit_mask & HIT_ALL_MASK) != 0) {
				data->bit_mask &= ~HIT_ALL_MASK;
				SetHitMaskMenuToggles();
				sendMessage(data, kMessageHitsChanged);
			}
			break;
			
		case IDM_CUT_NONE:
			if ((data->bit_mask & HIT_ALL_MASK) != HIT_ALL_MASK) {
				data->bit_mask |= HIT_ALL_MASK;
				SetHitMaskMenuToggles();
				sendMessage(data, kMessageHitsChanged);
			}
			break;
		
		case IDM_CUT_PMT_ONLY:
			if ((data->bit_mask & HIT_ALL_MASK) != HIT_NCD_MASK) {
				data->bit_mask |= HIT_NCD_MASK;
				data->bit_mask &= ~HIT_PMT_MASK;
				SetHitMaskMenuToggles();
				sendMessage(data, kMessageHitsChanged);
			}
			break;
			
#ifndef SNONPLUS
        case IDM_CUT_NCD_ONLY:
			if ((data->bit_mask & HIT_ALL_MASK) != HIT_PMT_MASK) {
				data->bit_mask |= HIT_PMT_MASK;
				data->bit_mask &= ~HIT_NCD_MASK;
				SetHitMaskMenuToggles();
				sendMessage(data, kMessageHitsChanged);
			}
			break;
#endif

		case IDM_CUT_NORMAL:
			data->bit_mask ^= HIT_NORMAL;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_OWL:
			data->bit_mask ^= HIT_OWL;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_LOW_GAIN:
			data->bit_mask ^= HIT_LOW_GAIN;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_FECD:
			data->bit_mask ^= HIT_FECD;
			sendMessage(data, kMessageHitsChanged);
			break;
			
#ifndef SNOPLUS
		case IDM_CUT_SHAPER:
			data->bit_mask ^= HIT_SHAPER;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_MUX:
			data->bit_mask ^= HIT_MUX;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_SCOPE:
			data->bit_mask ^= HIT_SCOPE;
			sendMessage(data, kMessageHitsChanged);
			break;
#endif

		case IDM_CUT_BUTTS:
			data->bit_mask ^= HIT_BUTTS;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_NECK:
			data->bit_mask ^= HIT_NECK;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_DISCARDED:
			data->bit_mask ^= HIT_DISCARDED;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_CUT_OVERSCALE:
			data->bit_mask ^= HIT_OVERSCALE;
			sendMessage(data, kMessageHitsChanged);
			break;
		
		case IDM_CUT_UNDERSCALE:
			data->bit_mask ^= HIT_UNDERSCALE;
			sendMessage(data, kMessageHitsChanged);
			break;
			
		case IDM_VESSEL:
			data->show_vessel ^= 1;
			sendMessage(data, kMessageVesselChanged);
			break;
		
		case IDM_HIT_LINES:
			data->hit_lines ^= 1;
			sendMessage(data, kMessageHitLinesChanged);
			break;

		case IDM_RCON_LINES:
			data->rcon_lines ^= 1;
			sendMessage(data, kMessageFitLinesChanged);
			break;
			
		case IDM_WATER_LEVEL:
			toggleWaterLevelDisplay(data);
			break;
			
		case IDM_ANCHOR_ROPES:
		    data->anchor_ropes ^= 1;
			sendMessage(data, kMessageVesselChanged);
			break;
			
		case IDM_GEODESIC:
		case IDM_POLAR:
		case IDM_NO_FRAME:
		case IDM_PANE_FLAT:
		case IDM_PANE_CRATE:
		case IDM_PANE_PROJ:
		case IDM_PANE_NCD:
		{
			int oldGeo = data->wGeo;
			int was3d = Is3d(data->wGeo);
			
			if (GetMenu()->UpdateTogglePair(&data->wGeo)) {
			
				int is3d = Is3d(data->wGeo);
				
				if (is3d && was3d) {
					sendMessage(data, kMessageGeometryChanged);
				} else {
					if (oldGeo == IDM_PANE_PROJ) {
						GetMenu()->RemoveMenuItem();	// remove Projection menu
					}
					PImageCanvas *pimage = GetImage();
					Widget canvas = pimage->GetCanvas();
					delete pimage;
					SetScrollHandler(CreateNewImage(canvas));
					SetScrolls();
				}
				SetDirty();
			}
		} break;

		case IDM_WHITE_BKG:
			PResourceManager::SetColours(data->image_col ^ kWhiteBkg);
			break;
			
		case IDM_GREYSCALE:
			PResourceManager::SetColours(data->image_col ^ kGreyscale);
			break;
			
		case IDM_MOVE_HOME:
			SetToHome();
			break;
			
		case IDM_MOVE_TOP:
			SetToHome(1);
			break;
			
		case IDM_MOVE_VERTEX:
			sendMessage(data, kMessageSetToVertex);
			SetDirty();
			break;
			
		case IDM_MOVE_SUN:
			sendMessage(data, kMessageSetToSun);
			SetDirty();
			break;
	
		case IDM_AUTO_VERTEX:
			data->auto_vertex ^= 1;
			if (data->auto_vertex && data->auto_sun) {
				GetMenu()->SetToggle(IDM_AUTO_SUN, 0);
				data->auto_sun = 0;
			}
			if (data->auto_vertex && data->nrcon) {
				sendMessage(data, kMessageSetToVertex);
				SetDirty();
			}
			break;
			
		case IDM_AUTO_SUN:
			data->auto_sun ^= 1;
			if (data->auto_sun) {
				if (data->auto_vertex) {
					GetMenu()->SetToggle(IDM_AUTO_VERTEX, 0);
					data->auto_vertex = 0;
				}
				sendMessage(data, kMessageSetToSun);
				SetDirty();
			}
			break;
			
		case IDM_MOVE_SPHERE:
		case IDM_MOVE_EVENT:
			PMenu::UpdateTogglePair(&data->wMove);
			break;
		
		case IDM_NEXT_VERTEX:
			if (data->nrcon > 1) {
				if (++data->curcon >= data->nrcon) data->curcon = 0;
				// must re-calculate time differences only if they are being displayed
				if (data->wDataType == IDM_DELTA_T) {
					calcCalibratedVals(data);
				}
				if (data->auto_vertex) {
					sendMessage(data, kMessageSetToVertex);
					SetDirty();
				}
				newTitle(data);
				sendMessage(data, kMessageFitChanged);
			}
			break;
			
		default:
			// break if not an extra hit data type
			if (anID<IDM_DISP_EXTRA_FIRST || anID>IDM_DISP_EXTRA_LAST) {
				Printf("Unknown menu command ID - %d\n",anID);
				break;
			}
			// Drop through! (extra hit data types...)
		case IDM_TAC:
		case IDM_DELTA_T:
		case IDM_QHS:
		case IDM_QHL:
		case IDM_QLX:
		case IDM_QHL_QHS:
		case IDM_NHIT:
		case IDM_DISP_CRATE:
		case IDM_DISP_CARD:
		case IDM_DISP_CHANNEL:
		case IDM_DISP_CELL:
		case IDM_CMOS_RATES:
		{
			int			mode_change = 0;
			
			/* clear variables if switching to or from rates display */
			if (anID==IDM_CMOS_RATES || data->wDataType==IDM_CMOS_RATES) {
				mode_change = 1;
			}
			if (PMenu::UpdateTogglePair(&data->wDataType)) {
#ifdef OPTICAL_CAL
				if (data->wDataType==IDM_NHIT && data->wCalibrated!=IDM_UNCALIBRATED) {
					calcOCA(data, 1);
				} else {
					deleteOCA(data);
				}
#endif
				char *str = PMenu::GetLabel((MenuList *)NULL);
				if (str) {
					XtFree(data->dispName);
					data->dispName = str;
				}
				
				// must reset histogram grab to allow scale to change
				hist = PEventHistogram::GetEventHistogram(data);
				if (hist) hist->ResetGrab(0);
				
				if (mode_change) {
					/* send message indicating that the mode changed */
					sendMessage(data,kMessageDataModeChanged);
					if (anID == IDM_CMOS_RATES) {
						/* allocate cmos rates array */
						if (!data->cmos_rates) {
							data->cmos_rates = (int32 *)XtMalloc(NUM_TOTAL_CHANNELS * sizeof(int32));
							if (!data->cmos_rates) quit("Not enough memory!\n");
							memset(data->cmos_rates, -1, NUM_TOTAL_CHANNELS * sizeof(int32));
						}
					} else {
						/* free cmos rates array */
						if (data->cmos_rates) {
							free(data->cmos_rates);
							data->cmos_rates = 0;
						}
					}
					/* must re-display the event from scratch */
					xsnoed_event(data, (aPmtEventRecord *)0);
				} else {
					/* re-calculate hit values and redisplay all images */
					calcCalibratedVals(data);
					calcHitVals(data);
					sendMessage(data,kMessagePMTDataTypeChanged);
					sendMessage(data,kMessageHitsChanged);
				}
			}
		} break;
		
#ifndef SNOPLUS
        case IDM_NCD_SHAPER_VAL:
		case IDM_NCD_SHAPER_HIT:
		case IDM_NCD_MUX_HIT:
		case IDM_NCD_SCOPE_HIT:
		    if (PMenu::UpdateTogglePair(&data->wNCDType)) {
				sendMessage(data,kMessageNCDDataTypeChanged);
			    sendMessage(data,kMessageHitsChanged);
		    }
		    break;
#endif

		case IDM_UNCALIBRATED:
		case IDM_PRECALIBRATED:
		case IDM_CAL_SIMPLE:
		case IDM_CAL_NO_WALK:
#ifdef TITLES_CAL
		case IDM_CAL_PETER:
#endif
#ifdef ROOT_FILE
		case IDM_CAL_SNODB:
		case IDM_CAL_SNOMAN:
#endif
			/* toggle between calibrated and uncalibrated display */
#ifdef LOAD_CALIBRATION
			if (PMenu::UpdateTogglePair(&data->wCalibrated)) {
				setCalibration(data,data->wCalibrated);
			}
			
			// must reset histogram grab to allow scale to change
			hist = PEventHistogram::GetEventHistogram(data);
			if (hist) hist->ResetGrab(0);
				
#ifdef OPTICAL_CAL
			if (data->wDataType==IDM_NHIT && data->wCalibrated!=IDM_UNCALIBRATED) {
				calcOCA(data, 1);
			} else {
				deleteOCA(data);
			}
#endif
			calcCalibratedVals(data);
			calcHitVals(data);
			sendMessage(data, kMessageCalibrationChanged);
			sendMessage(data, kMessageHitsChanged);
#endif
			break;
#ifdef DEMO_VERSION
		case IDM_PROTECT:
			if (sProtect) {
				// open password dialog
				if (data->mWindow[PASSWORD_WINDOW]) {
					data->mWindow[PASSWORD_WINDOW]->Raise();
				} else {
					CreateWindow(PASSWORD_WINDOW);
				}
			} else {
				// turn on protect mode
				SetProtect(1);
			}
			break;
#endif
		case IDM_ECHO_MAIN:
			mEchoMainDisplay ^= 1;
			if (mEchoMainDisplay) {
				// show event currently in main display
				ImageData *main_data = ((XSnoedWindow *)PWindow::sMainWindow)->GetData();
				PmtEventRecord *pmtRecord = getHistoryEvent(main_data, 0);
				if (pmtRecord) {
					Listen(kMessageNewMainDisplayEvent, pmtRecord);
				}
			}
			break;
	}
}





