//
// File:		PResourceManger.cxx
//
// Created:		01/27/99 - P. Harvey
//
// Notes:
//
// As of 02/10/00, the resource manager also provides an interface for keyboard
// translations.  Follow these steps to add a keyboard translation:
//
// a) Get translation table for desired translation:
//
//		XtTranslations my_translations;
//		my_translations = XtParseTranslationTable("<Key>osfUp: XSnoedTranslate(a,b)");
//
//    This should only be done once to avoid memory leaks.  The translation table
//    may be shared among many widgets.  In this example, "a" and "b" are the
//    parameters that will be passed to the callback once installed.
//
// b) Override (or augment) translation for desired widget:
//
//		XtOverrideTranslations(my_widget, my_translations);
//
// c) Listen to PResourceManager::sSpeaker kMessageTranslationCallback messages:
//
//		PResourceManager::sSpeaker->AddListener(this);
//
// d) In the objects's Listen() method, handle kMessageTranslationCallback messages.
//    The message data is a pointer to a TranslationData structure.  First check that
//    it is your widget that generated the callback, then parse the parameters if
//    necessary and perform the desired actions.
//

// ------------------------------------------------------------------------------------------
// Resource Revision History:
//
// 3.1	12/17/99 - PH Made all windows children of main window (changed MC fonts accordingly)
// 3.2	01/17/00 - PH Added warning dialog background colour
// 3.3	01/27/00 - PH Changed MC font specification to make it immune to hierarchy
// 3.4	03/24/00 - PH Added SMALL font and separate font spec for XmText widgets
//
#define MINIMUM_RESOURCE_VERSION	3.5		// minimum version number for valid resource file

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <X11/StringDefs.h>
#include "PResourceManager.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "openfile.h"
#include "calibrate.h"
#include "xsnoed_version.h"
#if defined(USE_ROOT_DISPLAY) && defined(ROOT_SYSTEM) && defined(NO_MAIN)
#include "TGXW.h"
#endif

#define MAX_COLOUR_SEEDS			10
#define MAX_COLOURS					256

const char*	kAutoStr = "!! ====== Lines below are automatically overwritten by xsnoed! =====\n";

// Enumeration data structure
struct SWriteGeoData {
	FILE		*file;
	char		*name;		// resource name
	XrmQuark	quark;		// quark corresponding to resource name
};


static XtResource sResourceList[] = {
 {"resource_version", "ResVer", XtRFloat, sizeof(float), XtOffset(XSnoedResPtr,resource_version),
 		XtRString, (XtPointer)"0"},
 {"tac_min", "TacMin",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,tac_min),
 		XtRString, (XtPointer)"1000"},
 {"tac_max", "TacMax",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,tac_max),
 		XtRString, (XtPointer)"3000"},
 {"qhs_min", "QhsMin",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qhs_min),
 		XtRString, (XtPointer)"300"},
 {"qhs_max", "QhsMax",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qhs_max),
 		XtRString, (XtPointer)"800"},
 {"qhl_min", "QhlMin",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qhl_min),
 		XtRString, (XtPointer)"300"},
 {"qhl_max", "QhlMax",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qhl_max),
 		XtRString, (XtPointer)"800"},
 {"qlx_min", "QlxMin",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qlx_min),
 		XtRString, (XtPointer)"300"},
 {"qlx_max", "QlxMax",		XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qlx_max),
 		XtRString, (XtPointer)"800"},
 {"qhl_qhs_min", "QhlQhsMin",XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qhl_qhs_min),
 		XtRString, (XtPointer)"-400"},
 {"qhl_qhs_max", "QhlQhsMax",XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,qhl_qhs_max),
 		XtRString, (XtPointer)"400"},
 {"cmos_rates_min", "CmosRatesMin",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,cmos_rates_min),
 		XtRString, (XtPointer)"0"},
 {"cmos_rates_max", "CmosRatesMax",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,cmos_rates_max),
 		XtRString, (XtPointer)"4000"},
 {"delta_t_min", "DeltaTMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,delta_t_min),
 		XtRString, (XtPointer)"-50"},
 {"delta_t_max", "DeltaTMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,delta_t_max),
 		XtRString, (XtPointer)"150"},
 {"cal_tac_min", "CalTacMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_tac_min),
 		XtRString, (XtPointer)"-100"},
 {"cal_tac_max", "CalTacMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_tac_max),
 		XtRString, (XtPointer)"300"},
 {"cal_hits_min", "CalHitsMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_nhit_min),
 		XtRString, (XtPointer)"0"},
 {"cal_hits_max", "CalHitsMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_nhit_max),
 		XtRString, (XtPointer)"2"},
 {"cal_qhs_min", "CalQhsMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qhs_min),
 		XtRString, (XtPointer)"-50"},
 {"cal_qhs_max", "CalQhsMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qhs_max),
 		XtRString, (XtPointer)"350"},
 {"cal_qhl_min", "CalQhlMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qhl_min),
 		XtRString, (XtPointer)"-50"},
 {"cal_qhl_max", "CalQhlMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qhl_max),
 		XtRString, (XtPointer)"350"},
 {"cal_qlx_min", "CalQlxMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qlx_min),
 		XtRString, (XtPointer)"-50"},
 {"cal_qlx_max", "CalQlxMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qlx_max),
 		XtRString, (XtPointer)"350"},
 {"cal_qhl_qhs_min", "CalQhlQhsMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qhl_qhs_min),
 		XtRString, (XtPointer)"-200"},
 {"cal_qhl_qhs_max", "CalQhlQhsMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,cal_qhl_qhs_max),
 		XtRString, (XtPointer)"200"},
 {"extra_min", "ExtraMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,extra_min),
 		XtRString, (XtPointer)"-100"},
 {"extra_max", "ExtraMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,extra_max),
 		XtRString, (XtPointer)"100"},
 {"shaper_min", "ShaperMin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,shaper_min),
 		XtRString, (XtPointer)"0"},
 {"shaper_max", "ShaperMax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,shaper_max),
 		XtRString, (XtPointer)"100"},
 {"ncd_scope_xmin", "NcdScopeXmin",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,ncd_scope_xmin),
 		XtRString, (XtPointer)"0"},
 {"ncd_scope_xmax", "NcdScopeXmax",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,ncd_scope_xmax),
 		XtRString, (XtPointer)"15000"},
 {"ncd_scope_ymin", "NcdScopeYmin",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,ncd_scope_ymin),
 		XtRString, (XtPointer)"-100"},
 {"ncd_scope_ymax", "NcdScopeYmax",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,ncd_scope_ymax),
 		XtRString, (XtPointer)"100"},
 {"hist_bins", "HistBins",	XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,hist_bins),
		XtRString, (XtPointer)"50"},
 {"ncd_bins", "NCDBins",	XtRInt,   sizeof(int),	XtOffset(XSnoedResPtr,ncd_bins),
		XtRString, (XtPointer)"50"},
 {"num_cols",	"NumCols",	XtRInt,	  sizeof(int),  XtOffset(XSnoedResPtr,num_cols),
 		XtRString, (XtPointer)"32" },
 {"ves_cols",	"VesCols",	XtRInt,	  sizeof(int),  XtOffset(XSnoedResPtr,ves_cols),
 		XtRString, (XtPointer)"32" },
 {"proj_min",	"ProjMin",	XtRFloat, sizeof(float),XtOffset(XSnoedResPtr,proj.proj_min),
 		XtRString, (XtPointer)"-1.0"},
 {"proj_max",	"ProjMax",	XtRFloat, sizeof(float),XtOffset(XSnoedResPtr,proj.proj_max),
 		XtRString, (XtPointer)"1e10"},
 {"proj_screen","ProjScrn",	XtRFloat, sizeof(float),XtOffset(XSnoedResPtr,proj.proj_screen),
 		XtRString, (XtPointer)"-1.0"},
 {"tube_radius",	"TubeR",	XtRFloat, sizeof(float),XtOffset(XSnoedResPtr,tube_radius),
		XtRString, (XtPointer) "851.153"},
 {"hist_font",	"HistFont",	XtRFontStruct, sizeof(XFontStruct *), XtOffset(XSnoedResPtr,hist_font),
 		XtRString, (XtPointer)"-*-helvetica-medium-r-normal--12-*"},
 {"label_font",	"LabelFont",	XtRFontStruct, sizeof(XFontStruct *), XtOffset(XSnoedResPtr,label_font),
 		XtRString, (XtPointer)"-*-helvetica-medium-r-normal--12-*"},
 {"label_big_font",	"LabelBigFont",	XtRFontStruct, sizeof(XFontStruct *), XtOffset(XSnoedResPtr,label_big_font),
 		XtRString, (XtPointer)"-*-helvetica-medium-r-normal--24-*"},
/* light speed in cm per ns (in water !)*/
 {"light_speed", "LightS", XtRFloat, sizeof(float), XtOffset(XSnoedResPtr,light_speed),
		XtRString, (XtPointer) "22.49006" },
 {"water_level", "WaterLevel", XtRFloat, sizeof(float), XtOffset(XSnoedResPtr,water_level[0]),
	 	XtRString, (XtPointer) "0" },
 {"cal_file","CalFile",XtRString,sizeof(String),XtOffset(XSnoedResPtr,calibration_file),
 		XtRString, (XtPointer) TITLES_FILE },
 {"calcmd_file","CalCmdFile",XtRString,sizeof(String),XtOffset(XSnoedResPtr,calcmd_file),
 		XtRString, (XtPointer) "calcmd_default.dat" },
 {"oca_file","OCAFile",XtRString,sizeof(String),XtOffset(XSnoedResPtr,oca_file),
 		XtRString, (XtPointer) "oca_visual.bin" },
 {"file_path","FilePath",XtRString,sizeof(String),XtOffset(XSnoedResPtr,file_path),
		XtRString, (XtPointer) "/usr/local/ph:/usr/local/xsnoed:/usr/local/lib/xsnoed:/usr/share/xsnoed" },
 {"snodb_server","SnodbServer",XtRString,sizeof(String),XtOffset(XSnoedResPtr,snodb_server),
 		XtRString, (XtPointer) "localhost" },
 {"snoman_server","SnomanServer",XtRString,sizeof(String),XtOffset(XSnoedResPtr,snoman_server),
 		XtRString, (XtPointer) "localhost" },
 {"help_url","HelpURL",XtRString,sizeof(String),XtOffset(XSnoedResPtr,help_url),
 		XtRString, (XtPointer) "http://snodaq.phy.queensu.ca/xsnoed/xsnoed.html" },
#ifdef DEMO_VERSION
 {"password","Password",XtRString,sizeof(String),XtOffset(XSnoedResPtr,password),
 		XtRString, (XtPointer) "xsnoed_expert" },
#endif
 {"black_col",	"BlackCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,black_col),
 		XtRString, (XtPointer)"Black"},
 {"white_col",	"WhiteCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,white_col),
 		XtRString, (XtPointer)"White"},
 {"load_trigger_settings","LoadTrigSet",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,load_trigger_settings),
 		XtRString, (XtPointer) "0" },
/*
** ----------------- the following resources saved with settings -----------------
*/
 // Note: "version" is the name of the first resource saved
 {"version","Version",XtRString,sizeof(String),XtOffset(XSnoedResPtr,version),
 		XtRString, (XtPointer) XSNOED_VERSION },
 {"open_windows","OpenWindows",	XtRInt,	  sizeof(int),  XtOffset(XSnoedResPtr,open_windows),
 		XtRString, (XtPointer)"0" },
 {"open_windows2","OpenWindows2",XtRInt,  sizeof(int),  XtOffset(XSnoedResPtr,open_windows2),
 		XtRString, (XtPointer)"0" },
 {"hex_id", "HexID", XtRInt, sizeof(int), XtOffset(XSnoedResPtr,hex_id),
	 	XtRString, (XtPointer) "0" },
 {"time_zone", "TimeZone", XtRInt, sizeof(int), XtOffset(XSnoedResPtr,time_zone),
	 	XtRString, (XtPointer) "0" },
 {"angle_rad", "AngleRad", XtRInt, sizeof(int), XtOffset(XSnoedResPtr,angle_rad),
	 	XtRString, (XtPointer) "0" },
 {"hit_xyz", "HitXYZ", XtRInt, sizeof(int), XtOffset(XSnoedResPtr,hit_xyz),
	 	XtRString, (XtPointer) "0" },
 {"log_scale", "LogScale", XtRInt, sizeof(int), XtOffset(XSnoedResPtr,log_scale),
	 	XtRString, (XtPointer) "0" },
 {"hit_size", "HitSize", XtRFloat, sizeof(float), XtOffset(XSnoedResPtr,hit_size),
	 	XtRString, (XtPointer) "1.0" },
 {"ncd_size", "NCDSize", XtRFloat, sizeof(float), XtOffset(XSnoedResPtr,ncd_size),
	 	XtRString, (XtPointer) "2.5" },
 {"history_cut", "HistoryCut", XtRInt, sizeof(int), XtOffset(XSnoedResPtr,history_cut),
	 	XtRString, (XtPointer) "1" },
 {"save_config","SaveConfig",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,save_config),
 		XtRString, (XtPointer) "0" },
 {"geo","Geo",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,geo),
 		XtRString, (XtPointer) "0" },
 {"move","Move",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,move),
 		XtRString, (XtPointer) "0" },
 {"dataType","DataType",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,dataType),
 		XtRString, (XtPointer) "0" },
 {"calibrated","Calibrated",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,calibrated),
 		XtRString, (XtPointer) "0" },
 {"projType","ProjType",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,projType),
 		XtRString, (XtPointer) "0" },
 {"projCoords","ProjCoords",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,projCoords),
 		XtRString, (XtPointer) "0" },
 {"sizeOption","SizeOption",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,sizeOption),
 		XtRString, (XtPointer) "0" },
 {"shapeOption","ShapeOption",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,shapeOption),
 		XtRString, (XtPointer) "0" },
 {"record_index","RecordIndex",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,record_index),
 		XtRString, (XtPointer) "0" },
 {"sum","Sum",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,sum),
 		XtRString, (XtPointer) "0" },
 {"autoFit","AutoFit",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,autoFit),
 		XtRString, (XtPointer) "0" },
 {"waterLevel","WaterLevel",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,waterLevel),
 		XtRString, (XtPointer) "0" },
 {"mcNames","MCNames",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,mcNames),
 		XtRString, (XtPointer) "0" },
 {"mcTrack","MCTrack",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,mcTrack),
 		XtRString, (XtPointer) "0" },
 {"mcParticle","MCParticle",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,mcParticle),
 		XtRString, (XtPointer) "1" },
 {"bit_mask","BitMask",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,bit_mask),
 		XtRString, (XtPointer) "0" },
 {"hit_lines","HitLines",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,hit_lines),
 		XtRString, (XtPointer) "0" },
 {"rcon_lines","RconLines",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,rcon_lines),
 		XtRString, (XtPointer) "0" },
 {"show_vessel","ShowVessel",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,show_vessel),
 		XtRString, (XtPointer) "0" },
 {"anchor_ropes","AnchorRopes",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,anchor_ropes),
 		XtRString, (XtPointer) "0" },
 {"auto_vertex","AutoVertex",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,auto_vertex),
 		XtRString, (XtPointer) "0" },
 {"auto_sun","AutoSun",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,auto_sun),
 		XtRString, (XtPointer) "0" },
 {"time_interval","TimeInterval",XtRFloat,sizeof(float),XtOffset(XSnoedResPtr,time_interval),
 		XtRString, (XtPointer) "1.0" },
 {"nhit_logic","NhitLogic",XtRString,sizeof(String),XtOffset(XSnoedResPtr,nhit_logic),
 		XtRString, (XtPointer) "0" },
 {"trigger_mask","TriggerMask",XtRString,sizeof(String),XtOffset(XSnoedResPtr,trigger_mask),
 		XtRString, (XtPointer) "<any>" },
 {"goto_run","GotoRun",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,goto_run_num),
 		XtRString, (XtPointer) "0" },
 {"dump_records","DumpRecords",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,dump_records),
 		XtRString, (XtPointer) "0" },
 {"image_col","ImageCol",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,image_col),
 		XtRString, (XtPointer) "0" },
 {"print_to","PrintTo",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,print_to),
 		XtRString, (XtPointer) "0" },
 {"print_col","PrintCol",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,print_col),
 		XtRString, (XtPointer) "1" },
 {"print_label","PrintLabel",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,print_label),
 		XtRString, (XtPointer) "1" },
 {"show_label","ShowLabel",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,show_label),
 		XtRString, (XtPointer) "0" },
 {"label_format","LabelFormat",XtRString,sizeof(String),XtOffset(XSnoedResPtr,label_format_pt),
 		XtRString, (XtPointer) "Run: %rn  GTID: %gt" },
 {"outfile1","Outfile1",XtRString,sizeof(String),XtOffset(XSnoedResPtr,output_file_pt[0]),
 		XtRString, (XtPointer) "xsnoed_out1.zdab" },
 {"outfile2","Outfile2",XtRString,sizeof(String),XtOffset(XSnoedResPtr,output_file_pt[1]),
 		XtRString, (XtPointer) "xsnoed_out2.zdab" },
 {"outfile3","Outfile3",XtRString,sizeof(String),XtOffset(XSnoedResPtr,output_file_pt[2]),
 		XtRString, (XtPointer) "xsnoed_out3.zdab" },
 {"outfile4","Outfile4",XtRString,sizeof(String),XtOffset(XSnoedResPtr,output_file_pt[3]),
 		XtRString, (XtPointer) "xsnoed_out4.zdab" },
 {"dispatcher","Dispatcher",XtRString,sizeof(String),XtOffset(XSnoedResPtr,dispatcher),
 		XtRString, (XtPointer) "localhost" },
 {"print_command","PrintCommand",XtRString,sizeof(String),XtOffset(XSnoedResPtr,print_string_pt[0]),
 		XtRString, (XtPointer) "lpr " },
 {"print_filename","PrintFilename",XtRString,sizeof(String),XtOffset(XSnoedResPtr,print_string_pt[1]),
 		XtRString, (XtPointer) "xsnoed_image.ps" },
 {"ascii_label","AsciiLabel",XtRString,sizeof(String),XtOffset(XSnoedResPtr,ascii_label),
 		XtRString, (XtPointer) "Run: %rn GTID: %gt" },
 {"ascii_file","AsciiFile",XtRString,sizeof(String),XtOffset(XSnoedResPtr,ascii_file),
 		XtRString, (XtPointer) "xsnoed_events.txt" },
 {"ascii_auto","AsciiAuto",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,ascii_auto),
 		XtRString, (XtPointer) "0" },
 {"dump_file","DumpFile",XtRString,sizeof(String),XtOffset(XSnoedResPtr,dump_file),
 		XtRString, (XtPointer) "xsnoed_dump.txt" },
 {"ncd_scope_cal","NcdScopeCal",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,ncd_scope_cal),
 		XtRString, (XtPointer) "0" },
 {"ncd_scope_chan","NcdScopeChan",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,ncd_scope_chan),
 		XtRString, (XtPointer) "0" },
 {"ncd_scope_mask","NcdScopeMask",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,ncd_scope_mask),
 		XtRString, (XtPointer) "15" },
 {"caen_mask","CaenMask",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_mask),
 		XtRString, (XtPointer) "255" },
 {"caen_auto","CaenAuto",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_auto),
 		XtRString, (XtPointer) "0" },
 {"caen_log","CaenLog",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_log),
 		XtRString, (XtPointer) "0" },
 {"caen0_min","Caen0Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[0]),
 		XtRString, (XtPointer) "0" },
 {"caen0_max","Caen0Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[0]),
 		XtRString, (XtPointer) "4096" },
 {"caen0_label","Caen0Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[0]),
 		XtRString, (XtPointer) "Channel 0" },
 {"caen1_min","Caen1Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[1]),
 		XtRString, (XtPointer) "0" },
 {"caen1_max","Caen1Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[1]),
 		XtRString, (XtPointer) "4096" },
 {"caen1_label","Caen1Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[1]),
 		XtRString, (XtPointer) "Channel 1" },
 {"caen2_min","Caen2Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[2]),
 		XtRString, (XtPointer) "0" },
 {"caen2_max","Caen2Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[2]),
 		XtRString, (XtPointer) "4096" },
 {"caen2_label","Caen2Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[2]),
 		XtRString, (XtPointer) "Channel 2" },
 {"caen3_min","Caen3Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[3]),
 		XtRString, (XtPointer) "0" },
 {"caen3_max","Caen3Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[3]),
 		XtRString, (XtPointer) "4096" },
 {"caen3_label","Caen3Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[3]),
 		XtRString, (XtPointer) "Channel 3" },
 {"caen4_min","Caen4Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[4]),
 		XtRString, (XtPointer) "0" },
 {"caen4_max","Caen4Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[4]),
 		XtRString, (XtPointer) "4096" },
 {"caen4_label","Caen4Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[4]),
 		XtRString, (XtPointer) "Channel 4" },
 {"caen5_min","Caen5Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[5]),
 		XtRString, (XtPointer) "0" },
 {"caen5_max","Caen5Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[5]),
 		XtRString, (XtPointer) "4096" },
 {"caen5_label","Caen5Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[5]),
 		XtRString, (XtPointer) "Channel 5" },
 {"caen6_min","Caen6Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[6]),
 		XtRString, (XtPointer) "0" },
 {"caen6_max","Caen6Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[6]),
 		XtRString, (XtPointer) "4096" },
 {"caen6_label","Caen6Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[6]),
 		XtRString, (XtPointer) "Channel 6" },
 {"caen7_min","Caen7Min",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_min[7]),
 		XtRString, (XtPointer) "0" },
 {"caen7_max","Caen7Max",XtRInt,sizeof(int),XtOffset(XSnoedResPtr,caen_max[7]),
 		XtRString, (XtPointer) "4096" },
 {"caen7_label","Caen7Label",XtRString,sizeof(String),XtOffset(XSnoedResPtr,caen_lbl[7]),
 		XtRString, (XtPointer) "Channel 7" },

 // colours
 {"bkg_col",	"BkgCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][BKG_COL]),
 		XtRString, (XtPointer)"Black"},
 {"text_col",	"TextCol",  XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][TEXT_COL]),
 		XtRString, (XtPointer)"White"},
 {"hid_col",	"HidCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][HID_COL]),
 		XtRString, (XtPointer)"Grey40"},
 {"frame_col",	"FrameCol", XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][FRAME_COL]),
 		XtRString, (XtPointer)"White"},
 {"vdark_col","VDarkCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][VDARK_COL]),
 		XtRString, (XtPointer)"Grey40"},
 {"vlit_col","VLitCol",		XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][VLIT_COL]),
 		XtRString, (XtPointer)"Grey80"},
 {"axes_col",	"AxesCol",  XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][AXES_COL]),
 		XtRString, (XtPointer)"ForestGreen"},
 {"curcon_col",	"CurconCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][CURCON_COL]),
 		XtRString, (XtPointer)"Red"},
 {"rcon_col",	"RconCol",  XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][RCON_COL]),
 		XtRString, (XtPointer)"Green"},
 {"scale_under","ScaleUnder",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_UNDER]),
 		XtRString, (XtPointer)"SkyBlue3"},
 {"scale_col0",	"ScaleCol0",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_COL0]),
 		XtRString, (XtPointer)"RoyalBlue1"},
 {"scale_col1",	"ScaleCol1",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_COL1]),
 		XtRString, (XtPointer)"LimeGreen"},
 {"scale_col2",	"ScaleCol2",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_COL2]),
 		XtRString, (XtPointer)"Yellow"},
 {"scale_col3",	"ScaleCol3",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_COL3]),
 		XtRString, (XtPointer)"Orange"},
 {"scale_col4",	"ScaleCol4",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_COL4]),
 		XtRString, (XtPointer)"Red"},
 {"scale_over",	"ScaleOver",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCALE_OVER]),
 		XtRString, (XtPointer)"Pink1"},
 {"discarded_col","DiscardCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][DISCARDED_COL]),
 		XtRString, (XtPointer)"Grey30"},
 {"grid_col",	"GridCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][GRID_COL]),
 		XtRString, (XtPointer)"Grey"},
 {"sun_col",	"SunCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SUN_COL]),
 		XtRString, (XtPointer)"Goldenrod1"},
 {"water_col",	"WaterCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][WATER_COL]),
 		XtRString, (XtPointer)"RoyalBlue"},
 {"ncd_col",	"NcdCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][NCD_COL]),
 		XtRString, (XtPointer)"MediumOrchid1"},
 {"scope0_col",	"Scope0Col",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCOPE0_COL]),
 		XtRString, (XtPointer)"SkyBlue3"},
 {"scope1_col",	"Scope1Col",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][SCOPE1_COL]),
 		XtRString, (XtPointer)"Chartreuse2"},
 {"mc_neutrino_col","MCNeutrinoCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_NEUTRINO_COL]),
 		XtRString, (XtPointer)"Aquamarine"},
 {"mc_electron_col","MCElectronCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_ELECTRON_COL]),
 		XtRString, (XtPointer)"Orange"},
 {"mc_neutron_col",	"MCNeutronCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_NEUTRON_COL]),
 		XtRString, (XtPointer)"Coral"},
 {"mc_gamma_col",	"MCGammaCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_GAMMA_COL]),
 		XtRString, (XtPointer)"HotPink"},
 {"mc_photon_col",	"MCPhotonCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_PHOTON_COL]),
 		XtRString, (XtPointer)"IndianRed"},
 {"mc_special_col",	"MCSpecialCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_SPECIAL_COL]),
 		XtRString, (XtPointer)"OrangeRed"},
 {"mc_other_col",	"MCOtherCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[0][MC_OTHER_COL]),
 		XtRString, (XtPointer)"Chartreuse2"},
 {"alt_bkg_col",	"AltBkgCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][BKG_COL]),
 		XtRString, (XtPointer)"White"},
 {"alt_text_col",	"AltTextCol",  XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][TEXT_COL]),
 		XtRString, (XtPointer)"Black"},
 {"alt_hid_col",	"AltHidCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][HID_COL]),
 		XtRString, (XtPointer)"Grey35"},
 {"alt_frame_col",	"AltFrameCol", XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][FRAME_COL]),
 		XtRString, (XtPointer)"Black"},
 {"alt_vdark_col","AltVDarkCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][VDARK_COL]),
 		XtRString, (XtPointer)"Grey40"},
 {"alt_vlit_col","AltVLitCol",		XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][VLIT_COL]),
 		XtRString, (XtPointer)"Grey80"},
 {"alt_axes_col",	"AltAxesCol",  XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][AXES_COL]),
 		XtRString, (XtPointer)"ForestGreen"},
 {"alt_curcon_col",	"AltCurconCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][CURCON_COL]),
 		XtRString, (XtPointer)"Red"},
 {"alt_rcon_col",	"AltRconCol",  XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][RCON_COL]),
 		XtRString, (XtPointer)"SpringGreen"},
 {"alt_scale_under","AltScaleUnder",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_UNDER]),
 		XtRString, (XtPointer)"SkyBlue3"},
 {"alt_scale_col0",	"AltScaleCol0",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_COL0]),
 		XtRString, (XtPointer)"RoyalBlue3"},
 {"alt_scale_col1",	"AltScaleCol1",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_COL1]),
 		XtRString, (XtPointer)"SeaGreen"},
 {"alt_scale_col2",	"AltScaleCol2",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_COL2]),
 		XtRString, (XtPointer)"Goldenrod3"},
 {"alt_scale_col3",	"AltScaleCol3",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_COL3]),
 		XtRString, (XtPointer)"Orange"},
 {"alt_scale_col4",	"AltScaleCol4",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_COL4]),
 		XtRString, (XtPointer)"Red"},
 {"alt_scale_over",	"AltScaleOver",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCALE_OVER]),
 		XtRString, (XtPointer)"Pink3"},
 {"alt_discarded_col","AltDiscardCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][DISCARDED_COL]),
 		XtRString, (XtPointer)"Grey80"},
 {"alt_grid_col",	"AltGridCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][GRID_COL]),
 		XtRString, (XtPointer)"Grey"},
 {"alt_sun_col",	"AltSunCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SUN_COL]),
 		XtRString, (XtPointer)"Goldenrod2"},
 {"alt_water_col",	"AltWaterCol",	XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][WATER_COL]),
 		XtRString, (XtPointer)"RoyalBlue3"},
 {"alt_ncd_col",	"AltNcdCol",   XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][NCD_COL]),
 		XtRString, (XtPointer)"MediumOrchid1"},
 {"alt_scope0_col",	"AltScope0Col",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCOPE0_COL]),
 		XtRString, (XtPointer)"SkyBlue3"},
 {"alt_scope1_col",	"AltScope1Col",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][SCOPE1_COL]),
 		XtRString, (XtPointer)"Chartreuse2"},
 {"alt_mc_neutrino_col","AltMCNeutrinoCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_NEUTRINO_COL]),
 		XtRString, (XtPointer)"Aquamarine"},
 {"alt_mc_electron_col","AltMCElectronCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_ELECTRON_COL]),
 		XtRString, (XtPointer)"Orange"},
 {"alt_mc_neutron_col","AltMCNeutronCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_NEUTRON_COL]),
 		XtRString, (XtPointer)"Coral"},
 {"alt_mc_gamma_col","AltMCGammaCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_GAMMA_COL]),
 		XtRString, (XtPointer)"HotPink"},
 {"alt_mc_photon_col","AltMCPhotonCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_PHOTON_COL]),
 		XtRString, (XtPointer)"IndianRed"},
 {"alt_mc_special_col","AltMCSpecialCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_SPECIAL_COL]),
 		XtRString, (XtPointer)"OrangeRed"},
 {"alt_mc_other_col","AltMCOtherCol",XtRPixel, sizeof(Pixel),XtOffset(XSnoedResPtr,colset[1][MC_OTHER_COL]),
 		XtRString, (XtPointer)"Chartreuse2"}
};


static XrmOptionDescRec command_line_options[XtNumber(sResourceList)];


XSnoedResource	PResourceManager::sResource;
PSpeaker	  *	PResourceManager::sSpeaker;
int				PResourceManager::sResourceFileSaveConfig = 0;	// value of save_config in resource file
char		  *	PResourceManager::sAllocFlags = NULL;
char			PResourceManager::sSettingsFilename[] = "~/.XSnoed";	// settings file name
XColor			PResourceManager::sColours[2 * NUM_COLOURS];
char			PResourceManager::sColoursAllocated[2 * NUM_COLOURS] = { 0 };
int				PResourceManager::sWindowOffsetX = 0;
int				PResourceManager::sWindowOffsetY = 0;

extern char *progpath;

#ifdef VAX
static char *	xsnoed_class = "XSnoed.dat";
#else
static char *	xsnoed_class = "XSnoed";
#endif

/* globals */
int		g_argc = 1;
char **	g_argv = &progpath;

/*
static int myHandler(Display *disp, XErrorEvent *err)
{
	printf("X error caught! (set debug breakpoint here)\n");
	exit(1);
}*/

//----------------------------------------------------------------------------------------------

/* initialize xsnoed X application */
void PResourceManager::InitApp()
{
	if (!sSpeaker) {
		sSpeaker = new PSpeaker;
	}
	// initialize the_app, display and gc
	if (!sResource.the_app) {
	
		XtToolkitInitialize();
		sResource.the_app = XtCreateApplicationContext();
		
#if defined(USE_ROOT_DISPLAY) && defined(ROOT_SYSTEM) && defined(NO_MAIN)
		// this is close, but events aren't getting sent properly from ROOT...
		sResource.display = (Display *)gGXW->GetDisplay();
		if (!sResource.display) {
			quit("Display not initialized by ROOT!");
		}
#else
		sResource.display = XOpenDisplay(NULL);
		if (!sResource.display) {
			Printf("Could not initialize default display.\n");
			quit("Is your DISPLAY environment variable set?");
		}
#endif
		// initialize the Xt display
		XtDisplayInitialize(sResource.the_app, sResource.display, "xsnoed", xsnoed_class,
                              GetCommandLineOptions(), GetNumOptions(), &g_argc, g_argv);

		sResource.gc = XCreateGC(sResource.display,DefaultRootWindow(sResource.display),0,NULL);

		// add our keyboard translation action
		static XtActionsRec actions[] = { { "XSnoedTranslate", &TranslationCallback } };
		XtAppAddActions(sResource.the_app, actions, XtNumber(actions));
		
		// add our error handler
//		XSetErrorHandler(myHandler);
		
		// load our settings
		LoadSettings();
	}
}

/* InitResources - initialize xsnoed resources */
void PResourceManager::InitResources(Widget toplevel)
{
	static int	sCursorID[NUM_CURSORS] = { 52, 50, 108, 116, 68, 106, 114 };//34 };  changed to arrow from crosshairs - PH 05/19/99
	static int	initDone = 0;
	
	if (!initDone) {
		initDone = 1;

		XtAddConverter(XtRString, XtRFloat, Str2floatXm, NULL, 0);

		// create temporary resources because XtGetApplicationResources kills the resource list!
		unsigned num = XtNumber(sResourceList);
		XtResource *temp_resources = new XtResource[num];
		if (!temp_resources) quit("out of memory!");
		memcpy(temp_resources, sResourceList, sizeof(sResourceList));
		XtGetApplicationResources(toplevel,&sResource,temp_resources,num,NULL,0);
		delete [] temp_resources;
		
/*		// get integer version number for xsnoed that wrote the resources
		char *pt = sResource.version;
		int version = 0;
		for (int i=0; ;) {
			version += atoi(pt);
			if (++i >= 3) break;
			pt = strchr(pt,'.');
			if (!pt) break;
			++pt;
			version *= 100;
		}
*/
		// initialize value of save_config from file
		sResourceFileSaveConfig = sResource.save_config;
		
		/* create cursors */
		for (int n=0; n<NUM_CURSORS; ++n) {
			sResource.cursor[n] = XCreateFontCursor(sResource.display, sCursorID[n]);
		}
		/* range check our colour set */
		if ((unsigned)sResource.image_col >= 4) {
			sResource.image_col = 0;
		}
		/* create array for colour allocation flags */
		int totalCols = NUM_COLOURS + sResource.num_cols + sResource.ves_cols;
		sAllocFlags = new char[totalCols];
		if (!sAllocFlags) quit("Out of memory");
		memset(sAllocFlags, 0, totalCols);
		
		/* get our RGB colours */
		for (int i=0; i<2; ++i) {
			for (int j=0; j<NUM_COLOURS; ++j) {
				sColours[i*NUM_COLOURS + j].pixel = sResource.colset[i][j];
			}
		}
		Display	  *	dpy  = sResource.display;
		int			scr  = DefaultScreen(dpy);
		Colormap	cmap = DefaultColormap(dpy, scr);
		XQueryColors(dpy,cmap,sColours,2*NUM_COLOURS);
		
		/* copy current colours into working array */
		CopyColours();
		/* allocate our colours */
		AllocColours(sResource.num_cols,&sResource.scale_col,SCALE_UNDER, 7, 1, 1);
		AllocColours(sResource.ves_cols,&sResource.vessel_col,VDARK_COL, 2, 0, 0);
		
#ifdef OPTICAL_CAL
		/* initialize optical constants to reasonable values */
		sResource.u_acrylic = 0.025;
		sResource.u_d2o		= 0.0005;
		sResource.u_h2o		= 0.0005;
		/* listen to property changed events for optical parameters */
		XSelectInput(sResource.display, DefaultRootWindow(sResource.display), PropertyChangeMask);
		/* get current optical parameters from this X session property */
		GetOpticalParameters();
#endif
	}
}

XrmOptionDescRec *PResourceManager::GetCommandLineOptions()
{
	static int	init_options = 0;
/*
** Initialize our command line options so all our resources
** will have command-line equivalents
*/
	if (!init_options) {
		init_options = 1;
		for (int i=0; i<(int)XtNumber(sResourceList); ++i) {
			int len = strlen(sResourceList[i].resource_name);
			char *pt = (char *)XtMalloc(len * 2 + 4);
			if (!pt) quit("Out of memory for command line options");
			pt[0] = '-';
			strcpy(pt+1, sResourceList[i].resource_name);
			pt[len+2] = '*';
			strcpy(pt+len+3, pt+1);
			command_line_options[i].option = pt;					// has form "-resource_type"
			command_line_options[i].specifier = pt+len+2;			// has form "*resource_type"
			command_line_options[i].argKind = XrmoptionSepArg;
			command_line_options[i].value = NULL;
		}
	}
	return(command_line_options);
}

int PResourceManager::GetNumOptions()
{
	return(XtNumber(sResourceList));
}

void PResourceManager::Str2floatXm(XrmValue *args, Cardinal *nargs, XrmValue *fromVal, XrmValue *toVal)
{
	static float	result;
	if (*nargs != 0) XtWarning("String to Float conversion needs no args");

	if (sscanf((char *)fromVal->addr,"%f",&result) == 1) {
		toVal->size = sizeof(float);
		toVal->addr = (caddr_t)&result;
	} else {
		XtStringConversionWarning((char *)fromVal->addr,"Float");
	}
}

// read resource version from a line of the resource file
// - returns 0 if line doesn't specify a version number
float PResourceManager::ReadResourceVersion(char *buff)
{
	static char *resource_label = "resource_version:";
			
	char *pt = strstr(buff,resource_label);
	
	if (pt) {
		pt = strtok(pt+strlen(resource_label), " \t\n\r");
		if (pt) {
			return(atof(pt));
		}
	}
	return(0.0);
}

// SetSettingsFilename - set name for settings file
// - name may start with "~/", which will be converted by GetSettingsFilename()
void PResourceManager::SetSettingsFilename(char *name)
{
	if (strlen(name) < (unsigned)kMaxSettingsFilenameLen) {
		strcpy(sSettingsFilename, name);
	}
}

// GetSettingsFilename - get full pathname of settings file
// - will convert leading "~/" to home directory name
// - returns non-zero if name obtained OK
int PResourceManager::GetSettingsFilename(char *outName)
{
	if (!memcmp(sSettingsFilename, "~/", 2)) {
		// name starts with "~/" -- convert to home filename
		char *home = getenv("HOME");
		if (!home) return(0);	// error: can't get home directory name
	
		strcpy(outName, home);	// return home directory name
		strcat(outName, sSettingsFilename + 1);	// add "/<filename>"
	} else {
		strcpy(outName, sSettingsFilename);
	}
	
	return(1);
}

// LoadSettings - load our resources from the settings file
void PResourceManager::LoadSettings()
{
	// first make sure the settings file exists
	if (VerifySettingsFile()) {

		// read in the resources from our preferences
		// and merge into the current database - PH 11/11/99
		char settings_filename[256];
		if (GetSettingsFilename(settings_filename)) {
			XrmDatabase home_db = XrmGetFileDatabase(settings_filename);
			if (home_db) {
				XrmDatabase cur_db = XtDatabase(sResource.display);
				if (cur_db) {
					// note: the following call will destroy home_db
					XrmMergeDatabases(home_db, &cur_db);
				} else {
					XrmDestroyDatabase(home_db);
				}
			}
		}
	}
}

// VerifySettingsFile -- Make sure we have a good settings file in our home directory
// Note that mData is not initialized when this routine is called, so don't use it!
// Return non zero if a settings file exists
int PResourceManager::VerifySettingsFile()
{
	const int	kBuffSize = 256;
	char		settings_filename[kBuffSize];
	char		oldset_filename[kBuffSize];
	char		buff[kBuffSize];
	FILE		*source_file, *dest_file, *old_file=NULL;
	int			out_of_date = 0;
	int			do_rename = 0;
	float		old_version = 0.0;

	if (!GetSettingsFilename(settings_filename)) {
		Printf("Can't locate home directory, so can't find settings file!\n");
		return(0); // nothing to do if we can't find our home directory
	}
	dest_file = fopen(settings_filename,"r");
	
	if (dest_file) {
		// check resource version number
		while (fgets(buff,kBuffSize,dest_file)) {
			old_version = ReadResourceVersion(buff);
			if (old_version != 0.0) break;
		}
		fclose(dest_file);
		
		if (old_version > MINIMUM_RESOURCE_VERSION-0.000001) {
			// good file already exists -- nothing to do
			return(1);
		}
		out_of_date = 1;
		Printf("Settings file %s is out of date (version %.1f)\n", settings_filename, old_version);
		
		// make full pathname to rename old settings file
		strcpy(oldset_filename, settings_filename);
		strcat(oldset_filename, "~");
		
		do_rename = 1;

	} else {
		Printf("Settings file %s not found\n", settings_filename);
	}
	// first try to read from the XSnoed.resource file
	source_file = openFile("XSnoed.resource","r",NULL);
	if (!source_file) {
		source_file = openFile("XSnoed","r",NULL);
    }
	if (!source_file) {
		Printf("Can't find source XSnoed.resource resource file%s\x07\n",
		       out_of_date ? " to update settings file" : "");
		// return non-zero if an out of date file is available
		return(out_of_date);	// nothing to do if we can't find it
	}
	// version number must be first line in a standard XSnoed file
	// - make sure version is good
	if (fgets(buff,kBuffSize,source_file)) {
		float new_version = ReadResourceVersion(buff);
		if (new_version < MINIMUM_RESOURCE_VERSION-0.000001) {
			Printf("Source resource file %s is%s out of date (version %.1f)\n", 
					xsnoed_class, (out_of_date ? " also" : ""), new_version);
			Printf("Current resource version is %.1f\n", (float)MINIMUM_RESOURCE_VERSION);
			Printf("Can't update settings file %s\n", settings_filename);
			Printf("Warning: Using out-of-date resources!\n");
			fclose(source_file);
			return(1);
		}
		fseek(source_file, 0L, SEEK_SET);	// rewind source file to beginning
		
	} else {
	
		Printf("Error reading from source resource file %s\n", xsnoed_class);
		fclose(source_file);
		return(0);
	}
	
	// rename old version of resource file (default ~/.XSnoed) if it existed
	if (do_rename) {
		Printf("Moving old settings %s to %s...\n", settings_filename, oldset_filename);
		if (rename(settings_filename, oldset_filename)) {
			Printf("Error renaming settings file\x07\n");
			return(0);
		}
		
		old_file = fopen(oldset_filename, "r");	// re-open the old file
		
		if (old_file) {
			// look for auto string
			int found_auto_str = 0;
			while (fgets(buff,kBuffSize,old_file)) {
				if (!strcmp(buff,kAutoStr)) {
					found_auto_str = 1;
					break;
				}
			}
			if (!found_auto_str) {
				fclose(old_file);	// no settings to copy over... close the old file
				old_file = NULL;
			}
		} else {
			Printf("Error opening old settings file %s\n", oldset_filename);
		}
	}
	
	// create resource file (default ~/.XSnoed)
	dest_file = fopen(settings_filename,"w");
	if (dest_file) {
		Printf("Copying source resource file %s to %s...\n", getOpenFileName(), settings_filename);
		int write_err = 0;
		while (fgets(buff,kBuffSize,source_file)) {
			if (fputs(buff,dest_file) == EOF) {
				write_err = 1;	// copy across all xsnoed resources
				break;
			}
		}
		if (old_file) {
			Printf("Preserving original settings from %s...\n", oldset_filename);
			if (fputs("\n", dest_file) == EOF) write_err = 1;
			if (fputs(kAutoStr, dest_file) == EOF) write_err = 1;
/*
** Translate resources from old versions here...
*/
			// translate window position resources from version 3.2
			if (fabs(old_version - 3.2) < 0.000001) {
				while (fgets(buff,kBuffSize,old_file)) {
					char *pt = strstr(buff,"geometry:");
					const short kPrefixLen = 7;	// length of "xsnoed." string
					// take "xsnoed." off the geometry for all windows but the main
					if (pt && pt-buff>kPrefixLen && !memcmp(buff,"xsnoed.",kPrefixLen)) {
						if (fputs(buff+kPrefixLen, dest_file) == EOF) write_err = 1;
					} else {
						if (fputs(buff, dest_file) == EOF) write_err = 1;
					}
				}
			} else {
				while (fgets(buff,kBuffSize,old_file)) {
					if (fputs(buff, dest_file) == EOF) write_err = 1;
				}
			}
			fclose(old_file);
		}
		fclose(dest_file);
		if (write_err) {
			// erase the file if we couldn't write to it properly
			unlink(settings_filename);
			Printf("Error writing to %s (disk full?)\x07\n", settings_filename);
		} else {
			Printf("Done %s settings file\n", out_of_date ? "updating" : "creating");
		}
	} else {
		Printf("Error creating %s\x07\n", settings_filename);
		if (old_file) fclose(old_file);
	}
	fclose(source_file);	// close the source file
	
	return(1);
}

// WritePaddedLabel - write resource label to file, padding with tab characters
void PResourceManager::WritePaddedLabel(FILE *fp, char *object_name, char *res_name)
{
	const char*	tabChars = "\t\t\t\t";
	
	int len = fprintf(fp,"%s.%s:",object_name, res_name);
	
	// pad with tabs up to the 4th tab stop
	int numTabs = 4 - len / 8;
	if (numTabs < 1) numTabs = 1;
	fprintf(fp,"%.*s",numTabs,tabChars);
}

// return the full resource name for a widget
void PResourceManager::GetResourceName(Widget w, char *buff)
{
	Widget	parent = XtParent(w);
	if (parent) {
		GetResourceName(parent,buff);
		strcat(buff,".");
	} else {
		buff[0] = '\0';
	}
	strcat(buff,XtName(w));
}

// SetWindowOffset - set offset from window resource position to actual window position
void PResourceManager::SetWindowOffset(int dx, int dy)
{
	sWindowOffsetX = dx;
	sWindowOffsetY = dy;
}

// GetWindowGeometry - get window geometry from the current resource database
// - returns non-zero if geometry was found
int PResourceManager::GetWindowGeometry(char *name, SWindowGeometry *geo)
{
	XrmDatabase		theDatabase = XtDatabase(sResource.display);
	char 			*strType;
	char			fullName[256];
	XrmValue		theValue;
	
	sprintf(fullName,"%s.geometry",name);
	
	if (XrmGetResource(theDatabase, fullName, fullName, &strType, &theValue)) {
		// make sure we got a string type back
		if (!strcmp(strType, XtRString)) {
			// parse geometry specification
			char *str = (char *)theValue.addr;
			char *pt1, *pt2, *pt3;
			pt1 = strchr(str, 'x');
			if (pt1) {
				pt2 = strchr(++pt1, '+');
				if (pt2) {
					pt3 = strchr(++pt2, '+');
					if (pt3) {
						++pt3;
						// return geometry values
						geo->width = atoi(str);
						geo->height = atoi(pt1);
						geo->x = atoi(pt2);
						geo->y = atoi(pt3);
						return(1);	// success!
					}
				}
			}
		} else {
			Printf("Oops - geometry resource isn't of type 'String'\n");
		}
	}
	return(0);	// geometry not obtained
}

// GetWindowGeometry - get window geometry from database for specified widget
int PResourceManager::GetWindowGeometry(Widget w, SWindowGeometry *geo)
{
	char res_name[256];
	GetResourceName(w, res_name);
	return(GetWindowGeometry(res_name, geo));
}

// SetWindowGeometry - set window geometry in current resource database
void PResourceManager::SetWindowGeometry(char *name, SWindowGeometry *geo)
{
	char		specifier[256];
	char		value_str[256];
	XrmDatabase	theDatabase = XtDatabase(sResource.display);
	
	sprintf(specifier,"%s.geometry",name);
	sprintf(value_str,"%dx%d+%d+%d",(int)geo->width,(int)geo->height,
				(int)geo->x - sWindowOffsetX,
				(int)geo->y - sWindowOffsetY);
	
	XrmPutStringResource(&theDatabase, specifier, value_str);
}

// WriteGeoProc - enumerate database to write window geometries
// - this routine is made extern "C" instead of a static member
//   because of a brain-dead compiler that gives warnings otherwise - PH 05/08/00
int WriteGeoProc(XrmDatabase *database, XrmBindingList bindings,
				 XrmQuarkList quarks, XrmRepresentation *type,
				 XrmValue *value, XPointer theData)
{
	int		n;
	FILE	*fp = ((SWriteGeoData *)theData)->file;
	char	res_name[256];
	
	// find last quark in list
	for (n=0; quarks[n]!=NULLQUARK; ++n) { }
	
	// only consider quarks matching the specification
	if (n>=2 && quarks[n-1] == ((SWriteGeoData *)theData)->quark) {
	
		char *object_name = XrmQuarkToString(quarks[0]);
		
		// only write xsnoed object geometries
#ifdef CHILD_WINDOWS
		if (!strcmp(object_name, "xsnoed"))
#else
		if (strcmp(object_name, "XSnoed"))
#endif
		{
			// construct resource name
			res_name[0] = '\0';
			for (int i=0; ;) {
				strcat(res_name,XrmQuarkToString(quarks[i]));
				if (++i >= n-1) break;
				strcat(res_name,".");
			}
			
			// write resource name to file
			PResourceManager::WritePaddedLabel(fp, res_name, ((SWriteGeoData *)theData)->name);
			
			// make sure the resource value is a string
			if (type[0] == XrmStringToQuark(XtRString)) {
				// write resource value to file
				fprintf(fp, "%s\n", (char *)value->addr);
			} else {
				Printf("Uh oh - geometry resource isn't of type 'String'\n");
/*
				// convert to a string and write to file
				char buff[256];
				XrmValue theValue;
				theValue.size = 256;
				theValue.addr = buff;
				XtConvertAndStore(NULL, XrmQuarkToString(type[0]), value, XtRString, &theValue);
				fprintf(fp, "%s\n", buff);
*/
			}
		}
	}
	return(0);	// return false to continue iterating through resources
}

// WriteWindowGeometries - write all window geometry resources to file
void PResourceManager::WriteWindowGeometries(FILE *dest_file)
{
	XrmDatabase		theDatabase = XtDatabase(sResource.display);
	XrmQuark		quarks[1];
	SWriteGeoData	theData;
	
	quarks[0] = NULLQUARK;
	theData.file = dest_file;
	theData.name = "geometry";
	theData.quark = XrmStringToQuark(theData.name);
	
	XrmEnumerateDatabase(theDatabase, quarks, quarks, XrmEnumAllLevels, WriteGeoProc, (XPointer)&theData);
}

// WriteSettings - write resources to file
void PResourceManager::WriteSettings(XSnoedResource *res, int force_save)
{
	int			i;
	int			write_err = 0;
	int			replacing = 0;
	int			found_auto_str = 0;
	char	  *	pt;
	const int	kBuffSize = 256;
	char		settings_filename[kBuffSize];
	char		temp_filename[kBuffSize];
	char		buff[kBuffSize];
	FILE	  *	source_file, *temp_file;

	if (res->save_config) {
		force_save = 1;		// must write to file if save_config is set in these resources
	} else if (!force_save && !sResourceFileSaveConfig) {
		// nothing to do if we don't need to save the configuration
		// and save_config is already zero in the resource file.
		return;
	}
	
	if (!GetSettingsFilename(settings_filename)) {
		if (force_save) {
			Printf("HOME environment variable not set -- can't save settings\n");
		}
		return;
	}
	
	// open output file
	strcpy(temp_filename,settings_filename);
	strcat(temp_filename,"_tmp");
	temp_file = fopen(temp_filename,"w");
	if (!temp_file) {
		Printf("Error creating temporary resource file %s\x07\n",temp_filename);
		return;	// nothing to do if we can't write file
	}
	
	// open source resource file (default ~/.XSnoed)
	source_file = fopen(settings_filename,"r");
	
	// look for end of file (if it was opened)
	if (source_file) {
		replacing = 1;	// we will replace this file later
		while (fgets(buff,kBuffSize,source_file)) {
			if (!strcmp(buff,kAutoStr)) {
				found_auto_str = 1;
				break;
			}
			if (fputs(buff,temp_file) == EOF) write_err = 1;	// copy across other xsnoed resources
		}
	}
	if (!found_auto_str) {
		// write a blank line before the auto string
		if (fputs("\n",temp_file) == EOF) write_err = 1;
	}
	if (fputs(kAutoStr, temp_file) == EOF) write_err = 1;
	
	// do we really want to save the configuration?
	if (force_save) {

		// write a blank line
		if (fputs("\n",temp_file) == EOF) write_err = 1;
		
		// write all window positions to file
		WriteWindowGeometries(temp_file);
		
		// look for the "version" resource in the list
		for (i=0; i<(int)XtNumber(sResourceList); ++i) {
			if (!strcmp(sResourceList[i].resource_name, "version")) break;
		}
		// write all resources from "version" onwards to the output file
		for (; i<(int)XtNumber(sResourceList); ++i) {
		
			WritePaddedLabel(temp_file, xsnoed_class, sResourceList[i].resource_name);
			
			char *res_pt = (char *)res + sResourceList[i].resource_offset;
			
			if (!strcmp(sResourceList[i].resource_type, XtRString)) {
				fprintf(temp_file,"%s\n", *(char **)res_pt);
			} else if (!strcmp(sResourceList[i].resource_type, XtRInt)) {
				fprintf(temp_file,"%d\n", *(int *)res_pt);
			} else if (!strcmp(sResourceList[i].resource_type, XtRFloat)) {
				fprintf(temp_file,"%.2f\n", *(float *)res_pt);
			} else if (!strcmp(sResourceList[i].resource_type, XtRPixel)) {
				// look up pixel value in our colours
				int index = (Pixel *)res_pt - res->colset[0];
				if (index>=0 && index<2*NUM_COLOURS) {
					// print the colour value
					fprintf(temp_file,"rgb:%.2x/%.2x/%.2x\n",
							(int)(sColours[index].red >> 8),
							(int)(sColours[index].green >> 8),
							(int)(sColours[index].blue >> 8));
				} else {
					// bad index -- set the colour to black
					fprintf(temp_file,"Black\n");
				}
			} else {
				fprintf(temp_file,"0\n");
				Printf("Unrecognized resource type!\n");
			}
		}

		if (fputs("\n",temp_file) == EOF) write_err = 1;	// write a trailing blank line
		
	} else {

		int found_save_config = 0;
		
		// don't save the configuration -- just copy it over except for save_config
		if (source_file) {
			while (fgets(buff,kBuffSize,source_file)) {
				pt = strstr(buff,".save_config:");
				if (pt) {
					pt = strchr(buff, '\n');
					if (pt && *(pt-1)=='0') {
						// save_config is already zero.. no need to continue copying file
						fclose(source_file);
						fclose(temp_file);
						unlink(temp_filename);	// erase temporary file
						return;
					}
					found_save_config = 1;
					WritePaddedLabel(temp_file, xsnoed_class, "save_config");
					fprintf(temp_file,"%d\n", res->save_config);
				} else {
					if (fputs(buff,temp_file) == EOF) write_err = 1;	// copy the old line
				}
			}
		}
		if (!found_save_config) {
			if (fputs("\n",temp_file) == EOF) write_err = 1;
			WritePaddedLabel(temp_file, xsnoed_class, "save_config");
			fprintf(temp_file,"%d\n", res->save_config);
			if (fputs("\n",temp_file) == EOF) write_err = 1;	// write a trailing blank line
		}
	}
	// close the files
	fclose(temp_file);
	if (source_file) fclose(source_file);
	
	// update state of save_config in resource file
	sResourceFileSaveConfig = res->save_config;
	
	if (write_err) {
		Printf("Error writing to temporary resource file %s\x07\n",temp_filename);
		unlink(temp_filename);	// erase temporary file
	} else {
		// rename the file we wrote
		if (rename(temp_filename, settings_filename)) {
			if (replacing) {
				Printf("Error replacing resource file %s\x07\n",settings_filename);
			} else {
				Printf("Error creating resource file %s\x07\n",settings_filename);
			}
			unlink(temp_filename);		// erase temporary file
		} else if (force_save) {
			Printf("Settings saved to %s\n",settings_filename);
		}
	}
}

void PResourceManager::AllocColours(int num, Pixel **col, int first, int nseeds,
									int overscale, int extras)
{
	int				i, j, cells, no_color = 0;
	float			t;
	XColor			col_seeds[MAX_COLOUR_SEEDS];
	XColor			tmp_cols[MAX_COLOURS];
	Display		  *	dpy  = sResource.display;
	int				scr  = DefaultScreen(dpy);
	Colormap		cmap = DefaultColormap(dpy, scr);
	char		  *	alloc_flags=0;
	
	if (col == &sResource.scale_col) {
		alloc_flags = sAllocFlags + NUM_COLOURS;
	} else if (col == &sResource.vessel_col) {
		alloc_flags = sAllocFlags + NUM_COLOURS + sResource.num_cols;
	} else {
		quit("AllocColours error");
	}
	
	if (nseeds < (overscale ? 4 : 2)) quit("Too few colour seeds!");
	if (nseeds > MAX_COLOUR_SEEDS) quit("Too many colour seeds!");
	if (num > MAX_COLOURS) quit("Too many colours!");
	
	/* try to allocate read/write cells in colormap */
	if (!*col) {
		*col = (Pixel *)XtMalloc((num + extras) * sizeof(Pixel));
		if (!*col) quit("Not enough memory for colour allocation");
		cells = XAllocColorCells(dpy, cmap, TRUE, NULL, 0, *col, num);
	} else {
		cells = 0;
	}
	
	for (i=0; i<nseeds; ++i) {
		col_seeds[i].pixel = sResource.colour[first+i];
	}
	XQueryColors(dpy,cmap,col_seeds,nseeds);
	
	for (i=0; i<num; ++i) {
		tmp_cols[i].flags = DoRed | DoGreen | DoBlue;
		// calculate base seed number for this index
		if (overscale) {
			if (i == 0) {
				j = 0;
				t = 0.0;
			} else if (i == num-1) {
				j = nseeds - 2;
				t = 1.0;
			} else {
				t = 1 + (i - 1) * (nseeds - 3) / (float)(num - 3);
				j = (int)t;
				if (j > nseeds-3) j = nseeds - 3;
				t -= j;
			}
		} else {
			t = i * (nseeds - 1) / (float)(num - 1);
			j = (int)t;
			if (j > nseeds-2) j = nseeds - 2;
			t -= j;
		}
		// interpolate the colour value
		tmp_cols[i].red   = (unsigned short)(col_seeds[j].red   * (1-t) + col_seeds[j+1].red   * t);
		tmp_cols[i].green = (unsigned short)(col_seeds[j].green * (1-t) + col_seeds[j+1].green * t);
		tmp_cols[i].blue  = (unsigned short)(col_seeds[j].blue  * (1-t) + col_seeds[j+1].blue  * t);

		if (cells) {
			/* must store colour into the new cells we allocated */
			tmp_cols[i].pixel = (*col)[i];
			alloc_flags[i] = 1;
		} else if (XAllocColor(dpy, cmap, tmp_cols + i)) {
			/* allocated a read-only cell because we couldn't get read-write */
			(*col)[i] = tmp_cols[i].pixel;
			alloc_flags[i] = 1;
		} else {
			/* can't get this colour -- use foreground colour */
			(*col)[i] = sResource.colour[FRAME_COL];
			no_color = 1;
			alloc_flags[i] = 0;
		}
	}
	if (cells) {
		XStoreColors(dpy, cmap, tmp_cols, num);
	}
	if (no_color) Printf("Couldn't allocate colours\n");
	
	// add extra colours (copy pixel value directly into end of colour list)
	for (i=0; i<extras; ++i) {
		(*col)[num + i] = sResource.colour[first + nseeds + i];
	}
}

void PResourceManager::FreeAllocatedColours(Pixel *col, int num)
{
	int			i;
	Display	  *	dpy  = sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);
	char	  *	alloc_flags=0;

	if (col == sResource.colour) {
		alloc_flags = sAllocFlags;
	} else if (col == sResource.scale_col) {
		alloc_flags = sAllocFlags + NUM_COLOURS;
	} else if (col == sResource.vessel_col) {
		alloc_flags = sAllocFlags + NUM_COLOURS + sResource.num_cols;
	} else {
		quit("Error in FreeAllocatedColours");
	}
	
	for (i=0; i<num; ++i) {
		if (!alloc_flags[i]) break;
	}
	
	// were all of the colours allocated?
	if (i == num) {
		// free all at once
		XFreeColors(dpy, cmap, col, num, 0);
		memset(alloc_flags, 0, num);
	} else {
		// free individually
		for (i=0; i<num; ++i) {
			if (alloc_flags[i]) {
				XFreeColors(dpy, cmap, col+i, 1, 0);
				alloc_flags[i] = 0;
			}
		}
	}
}

// SetColour - set individual colour entry
// Note: the colour must be allocated externally and becomes the
// property of the resource manager after used in this call
// Also note that num is between 0 and NUM_COLOURS*2-1
void PResourceManager::SetColour(int num, XColor *xcol)
{
	Display	  *	dpy  = sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);

	if (sColoursAllocated[num]) {
		XFreeColors(dpy, cmap, &sColours[num].pixel, 1, 0);
	}
	// copy the XColor structure
	memcpy(sColours+num, xcol, sizeof(XColor));
	// update the associated pixel value in our lookup array
	// (index into the 2-D array as if it were 1-dimensional)
	sResource.colset[0][num] = xcol->pixel;
	
	// set flag indicating we own this colour
	sColoursAllocated[num] = 1;
}

/* free allocated colours */
void PResourceManager::FreeColours()
{
	// deallocate grey colours if necessary
	FreeAllocatedColours(sResource.colour, NUM_COLOURS);
	// free scale colours
	FreeAllocatedColours(sResource.scale_col, sResource.num_cols);
	// free vessel colours
	FreeAllocatedColours(sResource.vessel_col, sResource.ves_cols);
}

/* change colour schemes */
void PResourceManager::SetColours(int colourSet)
{
	if (sResource.image_col != colourSet) {
		
		FreeColours();	// must free old colours before setting sResource.image_col

		sResource.image_col = colourSet;

		CopyColours();
		AllocColours(sResource.num_cols,&sResource.scale_col,SCALE_UNDER, 7, 1, 1);
		AllocColours(sResource.ves_cols,&sResource.vessel_col,VDARK_COL, 2, 0, 0);
		
		sSpeaker->Speak(kMessageResourceColoursChanged);
	}
}

/* copy current colour set into working colours */
/* Note: this routine should only be called when all allocated colours are freed */
/*       (otherwise colour leaks will occur) */
void PResourceManager::CopyColours()
{
	int			i;
	Display	  *	dpy  = sResource.display;
	int			scr  = DefaultScreen(dpy);
	Colormap	cmap = DefaultColormap(dpy, scr);

	if (!(sResource.image_col & kGreyscale)) {
		// colour scale
		memcpy(sResource.colour, sResource.colset[sResource.image_col], NUM_COLOURS*sizeof(Pixel));
	} else {
		// greyscale
		int set = (sResource.image_col & kWhiteBkg) ? 1 : 0;
										  
		// make temporary array to load colour values
		XColor *tmp_cols = new XColor[NUM_COLOURS];
		
		if (tmp_cols) {
			// convert colours to greyscale values
#ifdef GREYSCALE_INTENSITY
			// use intensity model to convert to greyscale
			for (i=0; i<NUM_COLOURS; ++i) {
				int k = i + set * NUM_COLOURS;
				unsigned short	val;
				if (set) {
					// take average for white background
					val = (unsigned short)((sColours[k].red
								 + (u_int32)sColours[k].green
								 + (u_int32)sColours[k].blue) / 3);
				} else {
					// take maximum for black background
					val = sColours[k].red;
					if (val < sColours[k].green) {
						val = sColours[k].green;
					}
					if (val < sColours[k].blue) {
						val = sColours[k].blue;
					}
				}
				tmp_cols[i].red   = val;
				tmp_cols[i].green = val;
				tmp_cols[i].blue  = val;
				tmp_cols[i].flags = DoRed | DoGreen | DoBlue;
			}
#else
			// use luminance model to convert to greyscale
			for (i=0; i<NUM_COLOURS; ++i) {
				int k = i + set * NUM_COLOURS;
				unsigned short	val;
/* this is the old NTSC standard
				// use luminance weighting (r=0.299, g=0.587, b=0.114)
				val = (unsigned short)(((u_int32)sColours[k].red   * 299 +
									    (u_int32)sColours[k].green * 587 +
									    (u_int32)sColours[k].blue  * 114 + 500) / 1000);
*/
				// (this is better for newer monitors)
				// use luminance weighting (r=0.2125, g=0.7154, b=0.0721)
				val = (unsigned short)(((u_int32)sColours[k].red   * 2125 +
									    (u_int32)sColours[k].green * 7154 +
									    (u_int32)sColours[k].blue  *  721 + 5000) / 10000);
				tmp_cols[i].red   = val;
				tmp_cols[i].green = val;
				tmp_cols[i].blue  = val;
				tmp_cols[i].flags = DoRed | DoGreen | DoBlue;
			}
#endif
			// change the scale colours to linear greyscale
			const int kNumCol = SCALE_OVER - SCALE_UNDER + 1;
			// intensity mapping for greyscale scale colours
			static unsigned short grey[2][kNumCol] = {
				// under,     0%,    25%,    50%,    75%,   100%,   over
				{ 0x5555, 0x6666, 0x8888, 0xaaaa, 0xcccc, 0xeeee, 0xffff },	// black bkg
				{ 0xdddd, 0xbbbb, 0x9999, 0x7777, 0x5555, 0x3333, 0x0000 }	// white bkg
			};
			// substitute the scale colours
			for (i=0; i<kNumCol; ++i) {
				int n = i + SCALE_UNDER;
				tmp_cols[n].red = tmp_cols[n].green = tmp_cols[n].blue = grey[set][i];
			}
			// allocate the grey colours
			for (i=0; i<NUM_COLOURS; ++i) {
				if (XAllocColor(dpy, cmap, tmp_cols + i)) {
					sResource.colour[i] = tmp_cols[i].pixel;
					sAllocFlags[i] = 1;	// set flag for colour allocated
				} else {
					// couldn't allocate the grey -- default to original color
					sResource.colour[i] = sResource.colset[set][i];
				}
			}
			delete tmp_cols;
		} else {
			Printf("Not enough memory to copy colours!");
		}
	}
}

void PResourceManager::SetApp(XtAppContext anApp, Display *dpy, GC gc)
{
	sResource.the_app = anApp;
	sResource.display = dpy;
	sResource.gc = gc;
}

#ifdef OPTICAL_CAL
/* retrieve optical parameters stored in the property of the root X window */
void PResourceManager::GetOpticalParameters()
{
	char	*pt;
	int		len;
	char	buff[512];
	float	u_acrylic, u_h2o, u_d2o;
	
	pt = XFetchBuffer(sResource.display, &len, OCA_BUFFER_NUM);
	
	if (pt) {
		if (len > 511) len = 511;
		memcpy(buff,pt,len);
		XFree(pt);
		buff[len] = 0;
	
		pt = buff;
		if (memcmp(pt,"Acrylic:",8)) return;
		u_acrylic = atof(pt+8);
		pt = strchr(pt,'\n');
		if (!pt) return;
		if (memcmp(++pt,"H2O:",4)) return;
		u_h2o = atof(pt+4);
		pt = strchr(pt,'\n');
		if (!pt) return;
		if (memcmp(++pt,"D2O:",4)) return;
		u_d2o = atof(pt+4);
		
		/* did the parameters change? */
		if (sResource.u_acrylic != u_acrylic ||
			sResource.u_h2o != u_h2o ||
			sResource.u_d2o != u_d2o)
		{
			sResource.u_acrylic = u_acrylic;
			sResource.u_h2o = u_h2o;
			sResource.u_d2o = u_d2o;
			/* inform anyone who is listening about the optical parameter change */
			sSpeaker->Speak(kMessageResourceOpticalChanged);
		}
	}
}
#endif


// TranslationCallback - callback for keyboard translations
// - this translation utility is for use by any widget requiring translations
void PResourceManager::TranslationCallback(Widget w, XEvent *ev, String *params, Cardinal *num_params)
{
	TranslationData	transData;
	
	transData.widget	 = w;
	transData.event	 	 = ev;
	transData.params	 = params;
	transData.num_params = *num_params;
	
	// inform all objects of this translation (via global resource speaker)
	sSpeaker->Speak(kMessageTranslationCallback, &transData);
}


/*
**  The following code is intended for debugging only - it is never called 
**  by anything.
*/
void PResourceManager::ListResources(Widget w)
{
	WidgetClass wclass;
	static XtResourceList resource_list;
	static unsigned int resource_items = 0;
	unsigned int i;

	wclass = XtClass( w);

	XtGetResourceList( wclass, &resource_list, &resource_items);

	if( resource_items != 0) {

		for ( i =0; i < resource_items; ++i ) {

			printf("Entry %d\nresource_name   = %s\n", i, resource_list->resource_name);
			printf("resource_class  = %s\n", resource_list->resource_class);
			printf("resource_type   = %s\n", resource_list->resource_type);
			printf("resource_size   = 0x%x\n", resource_list->resource_size);
			printf("resource_offset = 0x%x\n", resource_list->resource_offset);
			printf("default_type    = %s\n", resource_list->default_type);
			printf("default_addr    = 0x%lx\n", (long)resource_list->default_addr);
			++resource_list;

		}

	}
}


