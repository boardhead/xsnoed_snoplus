#ifndef __XSnoedResource_h__
#define __XSnoedResource_h__

#include <Xm/Xm.h>
#include "colours.h"
#include "menu.h"
#include "PProjImage.h"

// cursor indices
enum ECursorType {
	CURSOR_MOVE_4,
	CURSOR_EXCHANGE,
	CURSOR_MOVE_H,
	CURSOR_MOVE_V,
	CURSOR_XHAIR,
	CURSOR_ARROW_DOWN,
	CURSOR_ARROW_UP,
	NUM_CURSORS
};

enum ETimeZone {
	kTimeZoneSudbury = 0,
	kTimeZoneLocal	 = 1,
	kTimeZoneUTC	 = 2
};

enum EGotoBy {
	kGotoGTID,
	kGotoRun,
	kGotoTime
};

const short     kCaenRsrcNum = 8;               // number of CAEN channels saved

// masks for colour set bits
const short		kWhiteBkg	= 0x01;
const short		kGreyscale	= 0x02;

// definititions for start of hit and vessel colour numbers
#define FIRST_SCALE_COL		(NUM_COLOURS)
#define FIRST_VESSEL_COL	(NUM_COLOURS + PResourceManager::sResource.num_cols)

#define NUM_WRITERS				4				// number of ZDAB writers	
#define MAX_WINDOW_NAME_LEN		32

/* XSnoedResource structure definition */
struct XSnoedResource {
	float			resource_version;			// resource file version number
	int				tac_min, tac_max;			// range for raw TAC colour scale
	int				qhs_min, qhs_max;			// range for raw Qhs colour scale
	int				qhl_min, qhl_max;			// range for raw Qhl colour scale
	int				qlx_min, qlx_max;			// range for raw Qlx colour scale
	int				qhl_qhs_min, qhl_qhs_max;	// range for Qhl-Qhs colour scale
	int				cmos_rates_min;				// minimum for CMOS rates colour scale
	int				cmos_rates_max;				// maximum for CMOS rates colour scale
 	float			delta_t_min, delta_t_max;	// range for Delta T colour scale
 	float			cal_tac_min, cal_tac_max;	// range for calibrated TAC colour scale
 	float			cal_nhit_min, cal_nhit_max;	// range for calibrated Hit Count colour scale
 	float			cal_qhs_min, cal_qhs_max;	// range for calibrated Qhs colour scale
 	float			cal_qhl_min, cal_qhl_max;	// range for calibrated Qhl colour scale
 	float			cal_qlx_min, cal_qlx_max;	// range for calibrated Qlx colour scale
 	float			cal_qhl_qhs_min;			// minimum for calibrated Qhl-Qhs colour scale
 	float			cal_qhl_qhs_max;			// maximum for calibrated Qhl-Qhs colour scale
 	float			extra_min, extra_max;		// range for extra hit data colour scale
 	float			shaper_min, shaper_max;		// range for NCD shaper value colour scale
 	float           ncd_scope_xmin,ncd_scope_xmax;// range for NCD scope x axis
 	int             ncd_scope_ymin,ncd_scope_ymax;// range for NCD scope y axis
	int				hist_bins;					// number of histogram bins
	int             ncd_bins;                   // number of NCD histogram bins
	int				open_windows;				// bit mask for windows to open at startup
	int             open_windows2;              // bit mask for windows 32-63
	Pixel			colour[NUM_COLOURS];		// sys colour numbers for drawing
	Pixel			colset[2][NUM_COLOURS];		// colour sets
	Pixel		  *	scale_col;					// scale colours
	Pixel		  *	vessel_col;					// vessel colours
	Pixel			black_col;					// constant black colour
	Pixel			white_col;					// constant white colour
	int				num_cols;					// number of colours in hit colour scale
	int				ves_cols;					// number of colours in vessel colour scale
 	Projection		proj;						// current projection
 	float			time_interval;				// time interval for displayed events
	float			tube_radius;				// nominal PMT radius
	XFontStruct	  *	hist_font;					// font for histograms
	XFontStruct	  *	label_font;					// font for image labels
	XFontStruct	  *	label_big_font;				// big label font
	float			light_speed;				// speed of light (cm/ns)
	float			water_level[2];				// light and heavy water levels (cm)
	int				history_cut;				// flag to display cut history
	int				hex_id;						// flag to display event ID's in hex
	int				time_zone;					// time zone: 0=local, 1=UTC, 2=Sudbury
	int				angle_rad;					// flag to display angles in radians
	int				hit_xyz;					// flag to display hit xyz coordinates
	int				log_scale;					// flag for log histogram scale
	float			hit_size;					// hit size multiplier
	float           ncd_size;                   // ncd size multiplier
	char		  * calibration_file;			// pointer to calibration filename
	char		  *	calcmd_file;				// pointer to calibration command file
 	char		  *	oca_file;					// pointer to OCA filename
 	char		  *	file_path;					// pointer to alternate search path
 	char		  *	snodb_server;				// pointer to SNODB server IP
 	char		  *	snoman_server;				// pointer to SNOMAN server IP
 	char		  *	help_url;					// pointer to help URL string
 	char		  *	output_file_pt[NUM_WRITERS];// pointer to output zdab filenames
 	char		  *	print_string_pt[2];			// pointer to print command/filename
 	char		  *	label_format_pt;			// pointer to event label format string
	Cursor			cursor[NUM_CURSORS];		// cursors
	XtAppContext	the_app;					// the application context
	Display		  *	display;					// the X display
	GC				gc;							// the X graphics context
#ifdef OPTICAL_CAL
	float			u_acrylic;					// attenuation coefficient for acrylic
	float			u_h2o;						// attenuation coefficient for light water
	float			u_d2o;						// attenuation coefficient for heavy water
#endif
	int				geo;						// index for geometry menu item
	int				move;						// index for move menu item
	int				dataType;					// index for data type menu item
	int				calibrated;					// index for calibration menu item
	int				projType;					// index for projection menu item
	int				mcNames;					// flag to display Monte Carlo names
	int				mcTrack;					// index display Monte Carlo track menu
	int				mcParticle;					// bitmask for Monte Carlo particles to display
	int				sum;						// flag for event summing
	int				autoFit;					// flag for automatic fitting
	int				waterLevel;					// flag to display water level
	int				bit_mask;					// bitmask for displayed hits
	int				hit_lines;					// flag to display hit lines
	int				rcon_lines;					// flag to display fit lines
	int				show_vessel;				// flag to display acrylic vessel
	int				anchor_ropes;				// flag to display AV anchor ropes
	int				dump_records;				// flag to dump records to console
	int				image_col;					// index for image colour scheme
 	int				print_to;					// print destination (0=printer, 1=file)
	int				print_col;					// flag for print colours
	int				print_label;				// flag to print labels
	int				show_label;					// flag to show event label
	int				auto_vertex;				// flag to automatically move to fit vertex
	int				auto_sun;					// flag to automatically move to sun direction
	int				load_trigger_settings;		// non-zero to load trigger settings on startup
	char		  *	nhit_logic;					// threshold logic string
	char		  *	trigger_mask;				// trigger mask string
	int				goto_run_num;				// flag to goto by run number instead of event ID
	int				projCoords;					// index for projection coordinates menu item
	int				sizeOption;					// index for hit size menu item
	int				shapeOption;				// index for hit shape menu item
	int				record_index;				// index for record type displayed
	char		  *	version;					// xsnoed version that wrote the resources
	char		  *	dispatcher;					// dispatcher name
	int				save_config;				// flag to save settings on quit
	char		  *	ascii_label;				// pointer to default ASCII window label string
	int				ascii_label_allocated;		// non-zero if label string was allocated
	char		  *	ascii_file;					// pointer to default ASCII window file name
	int				ascii_file_allocated;		// non-zero if file name string was allocated
	int				ascii_auto;					// flag for ascii window auto-next feature
	char		  *	dump_file;					// pointer to default ASCII dump window file name
	int				dump_file_allocated;		// non-zero if file name string was allocated
	int             ncd_scope_cal;              // index for ncd scope calibration menu item
	int             ncd_scope_chan;             // index for ncd scope channel menu item
	int             ncd_scope_mask;             // mask of scope channels currently displayed
	int             caen_mask;                  // mask of CAEN channels displayed
	int             caen_auto;                  // auto-scaling option for CAEN window
	int             caen_log;                   // log colour scale for CAEN sum
	int             caen_min[kCaenRsrcNum];     // CAEN scale minimums
	int             caen_max[kCaenRsrcNum];     // CAEN scale maximums
	char          * caen_lbl[kCaenRsrcNum];     // CAEN trace labels
#ifdef DEMO_VERSION
	char		  *	password;					// password for demo version
#endif
};

typedef XSnoedResource *XSnoedResPtr;

#endif // __XSnoedResource_h__
