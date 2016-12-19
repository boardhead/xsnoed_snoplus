#ifndef __ImageData_h__
#define __ImageData_h__

#include <stdio.h>
#include "include/Record_Info.h"
#include "XSnoedResource.h"
#include "PSharedString.h"
#include "matrix.h"
#include "menu.h"
#include "ScopeData.h"

#ifdef FITTR
#include "fitter.h"
#ifndef LOAD_CALIBRATION
#define LOAD_CALIBRATION	// must be defined for fitter to work
#endif
#endif

const short	kMaxNhitCuts	 = 20;		// maximum number of NHIT cuts applied
const short kMaxPmtNcdLogic  = 20;      // maximum number of PMT/NCD's in logic
const short kMaxNcdScopeData = 8;
const short kNumLogAmpParms  = 3;       // number of NCD log amp parameters
const short kMaxCaenChannels = 8;

#define MAX_FNODES		4
#define	NODE_HID		0x01
#define	NODE_OUT		0x02
#define	NODE_MISSED		0x04			// node in cherenkov cone missed intersecting sphere
#define EDGE_HID		0x01
#define FACE_HID		0x01
#define FACE_COL_SHFT	2				// bit shift for face colour

#define MAX_RCON		10				// maximum number of reconstructed events
#define FILELEN			256				// maximum length of file name
#define FORMAT_LEN		512				// maximum length of event label
#define HISTORY_SIZE	101				// cache of recent events
#define FUTURE_SIZE		(HISTORY_SIZE-1)// future buffer is one smaller than history buffer

// special monte carlo vertex flags
#define VERTEX_FLAG_MARK		0x1000	// mark this vertex
#define VERTEX_FLAG_SHOW_NAME	0x2000	// show the interaction name at this vertex
#define VERTEX_FLAG_HIDDEN		0x4000	// do not draw this vertex
#define VERTEX_FLAG_SPECIAL		0x8000	// vertex should be drawn in a special color

// input sources
enum InputSource {
	INPUT_NONE,
	INPUT_ZDAB_FILE,
	INPUT_ROOT_FILE,
	INPUT_ROOT_TREE,
	INPUT_ROOT_EVENT_LIST,
	INPUT_DISPATCHER,
#ifdef XSNOMAN
	INPUT_XSNOMAN,
#endif // XSNOMAN
	NUM_INPUT_SOURCES
};

// bit definitions for status element of Pmt structure
// Note: changes in these definitions must be reflected in ENhitPMT below
enum PmtStatus {
	PMT_OK			= 0x01,
	PMT_NORMAL 		= 0x02,
	PMT_NECK		= 0x08,
	PMT_FECD		= 0x10,
	PMT_LOW_GAIN	= 0x20,
	PMT_OWL			= 0x40,
	PMT_BUTTS		= 0x80
};

enum HitInfoFlags {
	HIT_LGISEL		= 0x01,
	HIT_NORMAL 		= PMT_NORMAL,
	HIT_NECK		= PMT_NECK,
	HIT_FECD		= PMT_FECD,
	HIT_LOW_GAIN	= PMT_LOW_GAIN,
	HIT_OWL			= PMT_OWL,
	HIT_BUTTS		= PMT_BUTTS,
	HIT_PMT_MASK	= HIT_NORMAL | HIT_FECD | HIT_LOW_GAIN | HIT_OWL | HIT_BUTTS | HIT_NECK,
	HIT_SHAPER      = 0x1000,
	HIT_MUX         = 0x2000,
	HIT_SCOPE       = 0x4000,
	HIT_GENERAL     = 0x8000,
	HIT_NCD_MASK    = HIT_SHAPER | HIT_MUX | HIT_SCOPE | HIT_GENERAL,
	HIT_ALL_MASK    = HIT_NCD_MASK | HIT_PMT_MASK,
	HIT_DISCARDED	= 0x100,
	HIT_OVERSCALE	= 0x200,
	HIT_UNDERSCALE	= 0x400,
	HIT_HIDDEN		= 0x800
};

enum InteractionMask {
	IM_SPECTRAL_REFL = 1,	// don't use bit zero so we can use bit numbers in menu
	IM_DIFFUSE_REFL,
	IM_DIFFUSE_TRANS,
	IM_RAYLEIGH_SCAT,
	IM_PMT_BOUNCE,
	IM_REACHED_PMT
};

// Note: some code relies on the order of the HistoryType enums
enum HistoryType {
	HISTORY_VIEWED,
	HISTORY_ALL,
	HISTORY_FUTURE
};
	
enum ENhitOp {
	NHIT_EQUAL_TO		= 0x01,
	NHIT_GREATER_THAN	= 0x02,
	NHIT_GT_OR_EQ		= NHIT_GREATER_THAN | NHIT_EQUAL_TO,
	NHIT_LESS_THAN		= 0x04,
	NHIT_LT_OR_EQ		= NHIT_LESS_THAN | NHIT_EQUAL_TO,
	NHIT_NOT_EQUAL_TO	= 0x06,
	NHIT_BAD_OP			= 0x07
};

// these correspond to the bit numbers of the PmtStatus types
// (except for 0, which means any type)
enum ENhitPMT {
	NHIT_PMT_ANY		= 0,
	NHIT_PMT_NORMAL 	= 1,
	NHIT_PMT_NECK		= 3,
	NHIT_PMT_FECD		= 4,
	NHIT_PMT_LOW_GAIN	= 5,
	NHIT_PMT_OWL		= 6,
	NHIT_PMT_BUTTS		= 7,
	// the following are not PMT's, but it is convenient to put the NCD stuff here
#ifdef SNOPLUS
	NHIT_LAST_USED      = NHIT_PMT_BUTTS, // the last used bit
#else
	NHIT_NCD_SHAPER     = 8,
	NHIT_NCD_MUX        = 9,
	NHIT_NCD_SCOPE      = 10,
	NHIT_NCD_GENERAL    = 11,
	NHIT_LAST_USED      = NHIT_NCD_GENERAL, // the last used bit
#endif
	// bit masks for PMT and NCD types
	NHIT_PMT_MASK       = 0x00fa,
	NHIT_NCD_MASK       = 0x0f00
};

enum EPmtNcdLogic {
    kPmtNcd_Want,                   // want to display this PMT
    kPmtNcd_Need,                   // need this PMT/NCD in event or it won't be displayed
    kPmtNcd_Refuse,                 // don't display any event with this PMT/NCD
    kPmtNcd_Always,                 // always display events with this PMT/NCD
    kPmtNcdLogicMax
};

// indices for record header array
enum {
	kHdrRec_RHDR,
	kHdrRec_TRIG,
	kHdrRec_EPED,
	kHdrRec_CAST,
	kHdrRec_CAAC,
	kHdrRec_SOSL,
	kNumHdrRec
};

// structure used in event history buffers
struct HistoryEntry {
	int32	usage_count;			// number of owners of this history entry
	float	prev_time;				// time (sec) to previous event (or -1 if time unknown)
	float	next_time;				// time (sec) to next event (or -1 if time unknown)
	PSharedString	filename;		// file name for this event
	// the extended PmtEventRecord follows...
};

struct NhitLogic {
	int			nhit;				// nhit threshold
	ENhitOp		op;					// nhit operator
	ENhitPMT	type;				// hit type
	short		or_index;			// index to logic entry for 'or' condition
};

struct PmtNcdLogic {
    int          num;               // PMT barcode or NCD index
    EPmtNcdLogic logic;             // our logic for this PMT/NCD
};

struct Point3 {
	float		x,y,z;
};

// Note: If this structure changes, the format of the binary database
// file is affected, so the magic number in initData() must be changed.
struct Pmt {
	float		x,y,z,r;			// normallized xyz position and radius
	short int	panel;				// Panel number
	char		cell;				// Cell
	unsigned char	status;			// Tube status
	short int	tube;				// PMT barcode
};

struct HitInfo {
	long		gt;					// global trigger ID
	long		nhit;				// nhit counter
	short		qhs;				// high gain short integrate charge ADC value
	short		qhl;				// high gain long integrate charge ADC value
	short		qlx;				// low gain charge ADC value
	short		tac;				// TAC ADC value
	short		hit_val;			// colour index for drawing this hit
	short		index;				// index of tube in position array
	char		crate;				// electronics crate number (0-19)
	char		card;				// electronics FEC card number (0-15)
	char		channel;			// electronics FEC channel number (0-31)
	char		cell;				// CMOS cell number (0-15)
	float		calibrated;			// calibrated value (charge,time,time diff...)
	short		flags;				// hit info flags
};

struct EventInfo {
	u_int32		gt;					// global trigger ID
	u_int32		runNumber;			// run number
	u_int32		evtNumber;			// event number
	double		time10mhz;			// 10 MHz timer value converted to double
	short		nhit;				// number of tubes hit
	HitInfo		*hitInfo;			// pointer to hit information array (size=nhit)
};

struct Node {
	float		x3,y3,z3;			// physical sphere coordinates (radius=1)
	float		xr,yr,zr;			// rotated physical coordinates
	int			x,y;				// screen coordinates after rotating
	int			flags;				// flag for x,y outside limits
};

struct Face {
	int			num_nodes;
	Node		*nodes[MAX_FNODES];
	Point3		norm;				// unit vector normal to face
	int			flags;
};

struct Edge {
	Node		*n1, *n2;			// nodes for endpoints of edge
	Face		*f1, *f2;			// associated faces
	int			flags;				// used to indicate a hidden line
};

struct WireFrame {
	int			num_nodes;
	Node		*nodes;
	int			num_edges;
	Edge		*edges;
};

struct Polyhedron {
	int			num_nodes;
	Node		*nodes;
	int			num_edges;
	Edge		*edges;
	int			num_faces;
	Face		*faces;
	float		radius;
};

/*
** Reconstructed event
** Nodes:	0	- event position
**			1	- event vector
**			2->	- ordered Cherenkov cone nodes
*/
struct RconEvent {
	int			num_nodes;			// number of nodes for drawing
	Node		*nodes;				// array of nodes for drawing
	Vector3		pos;				// normalized reconstructed event location
	Vector3		dir;				// reconstructed event direction
	float		cone_angle;			// cone angle for display
	float		time;				// time for reconstructed event
	float		chi_squared;		// fit quality
	int			fit_pmts;			// number of pmts used in fit
	char		name[32];			// fitter name
};

struct TubeHits {
	int			num_nodes;
	Node		*nodes;
	HitInfo		*hit_info;			// corresponding array of hit information
};

struct DatabaseFile {
    int         type;               // 0=PMT, 1=NCD (EDBFileType)
    char        filename[512];      // database file name
    u_int32     first_run;          // first valid run number for this database
    u_int32     last_run;           // last valid run number for this database
};

enum EDBFileType {
    kPmtDB  = 0,
    kNcdDB  = 1,
    kNumDBs
};

/*
** NCD stuff
*/
const unsigned kMaxNCDs        = 80;   // maximum number of NCD detectors
const unsigned kNCDLabelLen    = 8;
const unsigned kSegmentNameLen = 16;
const unsigned kPreampLen      = 16;
const unsigned kNumMuxBuses    = 8;
const unsigned kNumMuxChannels = 12;
const unsigned kNumShaperSlots = 21;
const unsigned kNumShaperChannels = 8;
struct NCDMap {
    short       string_number;
    char        label[kNCDLabelLen];
    float       x,y,len;            // x,y position and length of string
    unsigned    mux_bus;            // mux bus number (0-7)
    unsigned    mux_boxnum;         // mux box number ?
    unsigned    mux_channel;        // mux channel (0-11)
    unsigned    hv_supply;          // HV supply number (0-7)
    unsigned    shaper_slot;        // shaper ADC slot number (0-20)
    unsigned    shaper_addr;        // shaper ADC HW address ?
    unsigned    shaper_channel;     // shaper ADC channel (0-7)
    unsigned    scope_channel;      // scope channel (1-4)
    char        preamp[kPreampLen];
    unsigned    num_segments;       // number of tube segments (3 or 4)
    unsigned    pds_board;
    unsigned    pds_channel;
    char        segment_name[4][kSegmentNameLen];
    Node        nodes[2];           // nodes for endpoints
};
struct NCDHit {
    u_int32     mux_count;
    u_int32     shaper_count;
    u_int32     scope_count;
    u_int32     shaper_value;
    int         flags;              // hit flags for this NCD
};

#ifdef OPTICAL_CAL
#ifdef  __cplusplus
class OCA;
#else
typedef void *OCA;
#endif
#endif

#ifdef ROOT_FILE
#ifdef  __cplusplus
class QWater;
class QCal;
class QEvent;
class QRchHist;
class TFile;
#else
typedef void *QWater;
typedef void *QCal;
typedef void *QEvent;
#endif
#endif


class XSnoedWindow;
class PWindow;
class PSpeaker;
class XSnoedstreamListener;
class PZdabFile;
class PZdabWriter;
class PProjImage;
class PEventTimes;
class PHistImage;
class PScopeImage;
class PCal;
class PCalSimple;
#ifdef TITLES_CAL
class PCalTitles;
#endif
struct MenuStruct;

struct ImageData : XSnoedResource {
	XSnoedWindow  *	mMainWindow;		// main xsnoed window
	PWindow		  * mWindow[NUM_WINDOWS];// xsnoed windows
	PSpeaker	  *	mSpeaker;			// broadcasts messages to listeners
	PEventTimes	  *	mEventTimes;		// event times histogram image
	MenuStruct	  *	main_menu;			// pointer to menu struct
	PProjImage	  *	mLastImage;			// last projection image to transform hits
	PProjImage	  *	mCursorImage;		// last image to be cursor'd in
	PZdabFile	  *	mZdabFile;			// zdab file object
	PZdabWriter	  *	mZdabWriter[NUM_WRITERS];// zdab writer objects
	PHistImage	  *	mScaleHist;			// histogram image currently being scaled
	PScopeImage	  *	mScaleScope;		// scope image currently being scaled
	u_int32		  *	mHdrRec[kNumHdrRec];// header records
	int				mDeleteData;		// flag to delete data when main window deleted
	int				mQuitOnDelete;		// quit application when main window deleted

	Widget			toplevel;			// top level XSnoed widget
	Polyhedron		geod;				// geodesic frame info
	TubeHits		hits;				// tube hit information
	
	NCDMap          ncdMap[kMaxNCDs];   // NCD tube map information
	int             numNcds;            // number of NCDs in map
	NCDHit          ncdHit[kMaxNCDs+1]; // NCD hit information
	int             numNcdHit;          // number of NCDs hit
	u_int32         general_count;      // count of general NCD records
	// lookup ncd number (index in ncdMap) based on shaper or MUX channels
	int             ncdShaperLookup[kNumShaperSlots][kNumShaperChannels];
	int             ncdMuxLookup[kNumMuxBuses][kNumMuxChannels];
	
	u_int32       * ncdData;            // ncd data for the event (or NULL if no NCD data)
#ifdef SNOPLUS
	u_int32       * caenData;           // CAEN digitized trigger data (or NULL)
#endif
	CalibratedPMT *	calHits;			// calibrated data for each hit (NULL if no pre-calibration)
	ExtraHitData  *	extra_hit_data[MAX_EXTRA_NUM];// extra data for each hit (NULL if no extra data)
	char		  *	extra_hit_fmt[MAX_EXTRA_NUM];// format for extra hit data (NULL defaults to "%g")
	int				extra_hit_num;		// number of extra hit data types for this event
	ExtraEventData*	extra_evt_data[MAX_EXTRA_NUM];// extra event data (NULL if no extra data)
	char		  *	extra_evt_fmt[MAX_EXTRA_NUM];// format for extra event data (NULL defaults to "%g")
	int				extra_evt_num;		// number of extra event data types for this event
	MonteCarloHeader *monteCarlo;		// monte carlo data for currently displayed event
	RconEvent		rcon[MAX_RCON];		// reconstruction events
	int				nrcon;				// number of reconstruction nodes
	int				curcon;				// current rcon
	int				watercon[2];		// water level rcons (0=H2O, 1=D2O)
	Node			sun_dir;			// direction to sun
	long			last_calc_time;		// time of last calculated sun direction
	int				num_disp;			// number of displayed hits
	u_int32			mtc_word[6];		// MTC trigger word
	u_int32         trig_word;          // trigger word (incl. synthetic bits)
	int       		fit_nhitw[MAX_RCON];// fitter nhitw parameters
	float			angle_conv;			// conversion from radians to displayed angle units
	Vector3			source_pos;			// OCA source position
	float			source_intensity;	// OCA source intensity
	float			source_factor;		// factor to scale calibrated hits display
#ifdef OPTICAL_CAL
	OCA			  *	oca;				// Optical Calibration object
	u_int32			missed_window_count;// number of tubes that missed the prompt window in sum
#endif
	Pmt			  *	tube_coordinates;	// array of tube position information
	int				num_tube_coordinates;// number of tubes in array

	// menu ID (IDM_) variables
	int				wGeo;				// code for geometry type
	int				wMove;				// flag for movement
	int				wDataType;			// flag for displayed data type
	int				wNCDType;			// flag for displayed NCD data type
	int				wCalibrated;		// flag for calibrated display
	int				wProjType;			// current projection type
	int				wMCTrack;			// Monte Carlo track display type
	int				wMCNames;			// Monte Carlo track names
	int				wProjCoords;		// current projection coordinates
	int				wSizeOption;		// current projection hit size option
	int				wShapeOption;		// current projection hit shape option

	int				mcFlags;			// monte carlo track display flags
	int				mcHighlight;		// flag to highlight current MC track
	char		  *	projName;			// name of current projection
	char		  *	dispName;			// name of display type
	int				cursor_hit;			// hit index for current cursor location
	int				cursor_ncd;			// ncd index for current cursor location
	PSharedString	mEventFile;			// name of currently open event file
	PSharedString	mDispFile;			// file name for currently displayed event
	char			output_file[NUM_WRITERS][FILELEN];// name of output event files
	char			print_string[2][FILELEN];// print command(0)/filename(1) strings
	char			label_format[FORMAT_LEN];// format of event label
	FILE		  *	infile;				// input file (or NULL if none open)
	double			sno_time_zero;		// zero time for 10 MHz clock
	long			event_num;			// number of event in file
	long			event_id;			// event global trigger ID
	int				non_seq_file;		// flag indicates that events in file are non-sequential
	int				require_rewind;		// flag indicates that file needs rewinding before next get_event
	long			last_seq_gtid;		// id of last event read from file (reset on rewind)
	double			event_time;			// time of event from 10 MHz counter
	int				trigger_bitmask;	// trigger bitmask
	int				trigger_bits_on;	// trigger bits which must be set
	int				trigger_bits_off;	// trigger bits which must be zero
	int				trigger_bits_always;// always show events with these bits set
	char			trigger_mask_buff[256];// trigger mask string
	int				nhit_logic_num;		// number of nhit cuts used
	int				nhit_pmt_mask;		// mask for specific tubes used in logic
	NhitLogic		nhit_logic_array[kMaxNhitCuts];// nhit logic array
	char			nhit_logic_buff[256];// nhit logic string
	int             pmt_logic_num;  // number of PMT's or NCD's in logic
	PmtNcdLogic     pmt_logic_array[kMaxPmtNcdLogic];
	int             ncd_logic_num;  // number of PMT's or NCD's in logic
	PmtNcdLogic     ncd_logic_array[kMaxPmtNcdLogic];
	char            pmt_ncd_logic_buff[256];
	char		  *	argv;				// command line to run program
	
	int				event_shown;		// non-zero if there is an event displayed
	int				trigger_flag;		// trigger flag (continuous/single/off)
	int				event_nhit;			// original number of hits in this event
	long			run_number;			// run number for event
	int				sub_run;			// sub-run number
    int      		last_cur_x;			// last cursor x location
    int				last_cur_y;			// last cursor y location
	Point3 		  *	remap_data;			// remapped coordinates for flat map tubes
	char 		  *	remap_done;			// array of flags for each PMT indicating the remap was done
	int				history_all;		// flag to display all events in history (instead of viewed only)
	int				history_evt;		// index of currently viewed event in history
	HistoryEntry  *	history_buff[3][HISTORY_SIZE];// history of viewed/all/future events
	HistoryEntry  *	last_non_orphan_history_entry;// pointer to last non-orphan entry into history buffer
	int				history_size[3];	// number of events in each history buffer
	char		  *	future_msg[HISTORY_SIZE];// messages for events in future buffer
	int				was_history;		// flag to indicate we were viewing events from the history
//	int				lost_future;		// flag set when a future event is lost due to full future buffer
	int				sum_changed;		// sum has been changed, but not displayed yet
	double			sum_time;			// time of last event entered into sum
	u_int32			sum_run_number;		// run number of last event summed
	int				sum_sub_run;		// sub-run number of last event summed
	PSharedString	sum_filename;		// filename for last event of sum
	u_int32			sum_event_id;		// event id of last event summed
	u_int32			sum_mtc_word[6];	// mtc word from last event summed
	u_int32         sum_trig_word;      // trig word from last event summed
	u_int32			sum_event_count;	// number of events summed
	u_int32			sum_nhit_count;		// total number of tubes hit
	u_int32		  *	sum_nhit;			// allocated all the time (also used for int32 cmos rates)
	u_int32		  *	sum_tac;			// only allocated while wSum = 1
	u_int32		  *	sum_qhs;			// only allocated while wSum = 1
	u_int32		  *	sum_qhl;			// only allocated while wSum = 1
	u_int32		  *	sum_qlx;			// only allocated while wSum = 1
	u_int32       * sum_ncdData;        // ncd data of last event summed
    u_int32       * sum_caenData;       // summed trigger sums
	u_int16       * caen_data[kMaxCaenChannels];  // raw scope data arrays
	int             caen_size;                    // size of scope data arrays
	u_int32         caenChannelMask;    // stuff in CAEN data...
	u_int32         caenPattern;
	u_int32         caenEventCount;
	u_int32         caenClock;	
    u_int32         tubiiTrig;
    u_int32         tubiiGT;
	HistoryEntry  * sum_event;          // history entry for last event summed
	int             reset_sum;          // flag to reset sum
	double          reset_sum_time;     // reset sum time in minutes
	double          last_reset_time;    // time of last reset
	u_int32			max_sum_nhit;		// maximum hits for any tube in sum
	u_int32			total_sum_nhit;		// total number of summed tubes hit
	u_int32			max_sum_tac;		// maximum summed tac for any tube
	u_int32			max_sum_qhs;		// maximum summed qhs for any tube
	u_int32			max_sum_qhl;		// maximum summed qhl for any tube
	u_int32			max_sum_qlx;		// maximum summed qlx for any tube
	
	char *          ncd_scope_data[kMaxNcdScopeData];  // raw scope data arrays
	float           ncd_log_amp[kMaxNcdScopeData][kNumLogAmpParms];
	long            ncd_scope_size;                    // size of scope data arrays
	
	int             db_count;           // number of db's in list
	u_int32         db_run_num;         // run number for loaded databases
	DatabaseFile  * db_list;            // list of database files
	DatabaseFile  * cur_db[kNumDBs];    // current PMT/NCD DB file (NULL=default DB, -1=no DB loaded)

	int32		  *	cmos_rates;			// cmos rates array
	int				init_cal;			// flag indicates we attempted to init fit
	int				real_cal_scale;		// flag to scale histogram for real calibrated data
	int				precal_real;		// flag set if precalibrated charge data is in PE
	
	void		( *	event_callback )( void );// pointer to callback made before displaying new event
	int				in_callback;		// flag non-zero if we are currently in our callback
	
	double			display_time;		// user variable cleared when event is displayed

	unsigned long	event_counter;		// counter used to determine if event is new
	unsigned long	snoman_counter;		// last event calibrated with QSnomanCal

	int				process_mode;		// event processing mode flags
	int				throw_out_data;		// flag to ignore incoming data

	int				input_source;		// InputSource source for event data
	
	ScopeData		scopeData;          // Latest data from trigger scope
#ifdef ROOT_SYSTEM
	XSnoedstreamListener* mWorkListener;// listener for work proc messages (ROOT_SYSTEM only)
#endif

#ifdef LOAD_CALIBRATION
#ifdef ROOT_FILE
	QCal		  *	root_cal;			// active ROOT calibration object
	QCal		  *	root_cal2;			// dormant ROOT calibration object
	double			root_cal_time;		// time of last calibration
#else // ROOT_FILE
	PCal		  *	pcal;				// pointer to current calibration object
	PCalSimple	  *	pcal_simple;
#ifdef TITLES_CAL
	PCalTitles	  *	pcal_titles;
#endif // TITLES_CAL
#endif // ROOT_FILE
#endif // LOAD_CALIBRATION

#ifdef ROOT_FILE
	QWater		  *	root_water;			// ROOT object to get water level information
	QEvent		  *	root_cal_event;		// last root event for snoman calibration
	double			root_time_zero;		// time zero for ROOT Julian date
	TFile		  *	root_rch_file;		// root Rch file
	int				owns_rch_file;		// non-zero if we own the rch file
	Widget			rch_filebox;		// rch file open dialog box
	Widget			rch_selbox;			// rch file selection box
	QRchHist	  *	root_rch_hist;		// rch histogram that owns the filebox
	int				rch_pmtn;			// PMT number for rch displays
#endif // ROOT_FILE

#ifdef FITTR
	fitControl	  *	pfit;				// fitter control data
#endif // FITTR

#ifdef XSNOMAN
	int				snoman_has_data;	// SNOMAN needs to supply more data if false.
	int				exit_to_snoman;		// Flag to return to SNOMAN on next pass.
	int				snoman_replace_mode;// If true, event is replaced instead of xsnoed_event'edd
	int				snoman_current_run;
	int				snoman_current_gtid;
#endif // XSNOMAN
};

struct Projection;

// functions
int		getCurrentEventIndex(ImageData *data);
int		isIntegerDataType(ImageData *data);
PmtEventRecord *getHistoryEvent(ImageData *data, int index);
HistoryEntry *getCurrentHistoryEntry(ImageData *data);
struct tm *getTms(double aTime, int time_zone);
time_t	getTime(struct tm *tms, int time_zone);
int *	getPmtCounts(ImageData *data);

// ImageData routines
void	freePoly(Polyhedron *poly);
void	freeWireFrame(WireFrame *frame);
void	initNodes(WireFrame *fm, Point3 *pt, int num);
void	initEdges(WireFrame *fm, int *n1, int *n2, int num);
char *	loadGeometry(Polyhedron *poly, int geo, char *argv, float tube_rad);
void	transform(Node *node, Projection *pp, int num);
void	transformPoly(Polyhedron *poly, Projection *pp);

int		isRealFit(ImageData *data);		// returns non-zero if current fit exists and is not water

#endif // __ImageData_h__
