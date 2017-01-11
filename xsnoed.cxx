/*  
** X-windows Sudbury Neutrino Observatory Event Display (XSNOED)
** -------------------------------------------------------------
** 
** Created Jan 13, 1992
** Philip Harvey
** Physics Dept.
** Stirling Hall
** Queen's University
** Kingston, Ontario
** K7L 3N6
**
** This source code, or any part thereof, may be distributed and used
** freely, provided that Philip Harvey be given credit in the program
** documentation for any source code taken from this program.
**
** This code is distributed "as is", with no warranty... blah, blah...
**
** Revision History:
**
** 01/13/92 PH - Created original SNOED for MS-DOS
** 01/20/92 PH - Ported to X windows (called xsnoed - basic X tookit, pre-ANSI C)
** 01/21/92 PH - Ported to Windows 3.1 and added hit display (WSnoed)
** 02/27/92 PH - Ported to X windows in Motif
** 03/10/93 PH - Ported to Macintosh (MacSnoed)
** 06/17/93 PH - Added flat map to Macintosh version
** 06/24/93 PH - Upgraded MacSnoed to C++
** 07/02/93 PH - Added cabling display to MacSnoed
** 07/05/93 PH - Ported additions back to X version (still pre-ANSI C)
** 03/29/94 PS - Added ability to read tube coordinate file. Added pulldown menu for flat map
** 06/09/98 PH - Added ability to listen to dispatched data
** 06/26/98 PH - Added ability to read ZDAB files
** 07/06/98 PH - Added PS projection window back into code. Added new projection types
** 07/29/98 PH - Added CMOS rates display
** 09/01/98 PH - Added ability to read ROOT files
** 09/30/98 PS - Added online fitter
** 10/06/98 PH - Added calibrated data display
** 10/20/98 PH - Added water level display
** 10/29/98 PH - Added optical calibration code
**   --> Revisions after this are documented in xsnoed_version.h
**
** Compiler switches:
**
** SWAP_BYTES	      - this must be defined for little endian systems
** USE_FTIME	      - use this definition if clock_gettime() is not available on the target system
** USE_GETTIMEOFDAY	  - use this definition if clock_gettime() or ftime() are not available
**                      (on Linux this method is best even though ftime() is available because ftime
**                       is only accurate to 1 second while gettimeofday() has sub-second resolution).
** FITTR		      - add online fitter
** LOAD_CALIBRATION   - add data calibration code (automatically enabled with FITTR option)
** NO_DISPATCH        - compile without dispatcher capability
** USE_PTHREAD	      - use threaded dispatcher calls for off-site online monitoring
**                      (dispatcher must be enabled for this to make sense)
** USE_NANOSLEEP      - use nanosleep() for systems without usleep() support
** ROOT_FILE	      - add ability to read ROOT files (ROOT libraries must be available)
** ROOT_APP           - for creating an xsnoed/ROOT executable (xsnoed.root)
** OPTICAL_CAL	      - enables optical calibration code
** OPTICAL_CAL_OUT    - debugging option to output hit times for optical calibration windowing
** PRINT_DRAWS	      - debugging option to print message each time a window is drawn
** STAT_BUG           - option for systems where calling stat() causes a crash.  This happened on one
**                      Linux system -- the symptom is a core dump when xsnoed starts up.
** IGNORE_DISP_ERRORS - Ignore dispatcher errors from check_head() routine.  Around the time we
**                      upgraded to Linux RedHat 6, these errors started appearing for some reason.
** NO_HELP            - disables online help feature for systems that don't support launching netscape
** CHILD_WINDOWS      - makes all windows children of main window -- fixes Linux "dead window" problem,
**						but introduces other windowing problems on some systems.
** NO_FORK            - for systems which don't support the fork() system call (some print features
**                      are lost with this compiler option).
** NO_MAIN            - compiles xsnoed without a main() subroutine (for creating libraries)
** VAX                - defined for VAX systems
** LESSTIF            - defined for Lesstif (shareware Motif) systems
** XSNOMAN            - defined for XSNOMAN build
** NO_FITTER_DAT      - uses default fitter control data instead of reading "fitter.dat"
** DEMO_VERSION		  - xsnoed demo version (allows disabling of some features)
** THICK_SUN		  - draws a double-width sun vector in the 3-D image
*/

//#define SNOSTREAM_DATABASE		/* use SNOSTREAM database for PMT positions (no OWL or low gain tubes) */
#define PETERS_DATABASE				/* use Peter Skensved's database for PMT positions */
//#define NICKS_DATABASE			/* use Nick West's snoman database for PMT positions */
//#define PRINT_DRAWS				/* debugging option to track redundant draws */
//#define DEBUG_MEMORY				/* debugging option to watch for memory leaks in history buffers */
//#define GENERATE_OWL_POSITIONS	/* utility option to write OWL positions to 'database.out' */
//#define DEBUG_EXTRA_DATA			/* for debugging extra hit and event data */

#define	ONLINE_FITTER_NAME			"Online"
#define DATABASE_FILE				"database.dat"
#define NCD_FILE                    "NCDTubeMap.dat"
#define DB_LIST_FILE                "db_list.dat"

#define NOMINAL_TUBE_RADIUS			851.153

#define SNO_LATITUDE				46.4753		// 46¡ 28' 31" N
#define SNO_LONGITUDE				81.2086		// 81¡ 12' 05" W
//#define	LAB_ROTATION				-52.35		// rotation of SNO lab frame wrt true north
#define	LAB_ROTATION				-49.58		// as measured by gyrocompass - PH 10/03/00
#define AV_INNER_RADIUS             600
#define NCD_BOTTOM_OFFSET           25          // cm from AV to bottom of NCD

#define BIG_RUN_NUMBER              2000000000

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <X11/Xatom.h>
#include "xsnoed.h"
#include "xsnoed_version.h"
#include "include/NcdDataTypes.h"
#include "menu.h"
#include "matrix.h"
#include "openfile.h"
#include "calibrate.h"
#include "xsnoedstream.h"
#include "PResourceManager.h"
#include "PImageWindow.h"
#include "PEventHistogram.h"
#include "PScale.h"
#include "PSpeaker.h"
#include "PHitInfoWindow.h"
#include "PEventControlWindow.h"
#include "PEventInfoWindow.h"
#include "PCalSimple.h"
#ifdef TITLES_CAL
#include "PCalTitles.h"
#endif
#ifdef XSNOMAN
#include "PSnomanWindow.h"
#include "xsnoman.h"
#endif
#include "PMonteCarloWindow.h"
#include "POpticalWindow.h"
#include "PZdabFile.h"
#include "PZdabWriter.h"
#include "PUtils.h"
#include "CUtils.h"
#include "XSnoedImage.h"
#include "XSnoedWindow.h"
#include "PCalSimple.h"

#if defined(LOAD_CALIBRATION) && defined(ROOT_FILE)
#include "QPMT.h"
#include "QSimpleCal.h"
#include "QSnoCal.h"
#include "QSnomanCal.h"
#define _ROOT_Rtypes
#define CAL_TIME_THRESH	3600	// calibrations are valid for an hour
#endif

#ifdef ROOT_FILE
#include "QWater.h"
#endif

#ifdef OPTICAL_CAL
#include "oca.h"
#define PROMPT_WINDOW_HALFWIDTH	3.5	/* half width of window for prompt light (ns) */
#define MAX_OCA_STATUS		3
#endif

#define MAX_MSGS			20
#define	NUM_CONE			32
#define	CONE_ANGLE			(41.4*PI/180)
#define VESSEL_RADIUS		600.0			/* cm radius of inside of vessel */

// accept any of the modifier keys as the Alt key
// (Mac keyboard uses Mod2Mask while Linux console uses Mod1Mask,
//  so accept any modifier key just to be safe)
#define AnyModMask			(Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)


/*-------------------------------------------------------------------------------
*/

#ifdef DEBUG_MEMORY
static int alloc_count = 0;
#endif

static char *sWaterName[2] = { "Water", "D2O" };

extern char *sFilePath;	/* alternate search path for data files */

/*---------------------------------------------------------------------------
** Database functions
*/

#ifdef OPTICAL_CAL
/* calculate optical calibration */
OCA *calcOCA(ImageData *data, int source_moved)
{
	if (!data->oca) {
		data->oca = new OCA();
		if (data->oca) {
			FILE *fp = openFile(data->oca_file, "rb",data->file_path);
			if (!fp) {
				Printf("Error opening OCA file %s\n",data->oca_file);
				delete data->oca;	// delete the (useless) object
				data->oca = NULL;
			} else {
				Printf("Loading OCA tables from %s\n",data->oca_file);
				data->oca->Load_Raw_OCA(fp);
				fclose(fp);
				source_moved = 1;
			}
		} else {
			Printf("Error creating OCA object!\n");
		}
	}
	if (data->oca) {
		if (source_moved) {
			data->oca->Set_Ball_Coordinates(data->source_pos[0],data->source_pos[1],data->source_pos[2]);
		}
		if (data->oca->Compute_OCA(data->u_d2o, data->u_acrylic, data->u_h2o)) {
			Printf("Error computing OCA for this source position\n");
		}
	}
	return(data->oca);
}
/* delete optical calibration object */
void deleteOCA(ImageData *data)
{
	if (data->oca) {
		delete data->oca;
		data->oca = (OCA *)0;
		data->missed_window_count = 0;
		data->source_intensity = 0;
		POpticalWindow::UpdateOpticalConstants(data,UPDATE_OSA_INTENSITY);
	}
}

/* set calibrated NHIT (Ratio of observed/expected hits) for this tube */
int setCalibratedNHIT( ImageData *data, HitInfo *hi, int index )
{
	float	occ;
	
	if (data->oca) {
		/* may want to make the status threshold user-settable eventually */
		if (data->oca->Get_Status(hi->index) <= MAX_OCA_STATUS) {
			if ((occ=data->oca->Get_Occupancy(hi->index)) > 0) {
				hi->calibrated = data->source_factor * hi->nhit / occ;
				return(0);
			}
		}
		// set calibrated value to indicate an error
		hi->calibrated = INVALID_CAL;
	} else {
		hi->calibrated = hi->nhit;	// print raw nhits if no OCA available
	}
	return(1);
}
#endif // OPTICAL_CAL

/* set calibrated extra value for this tube */
int setCalibratedExtra(ImageData *data, HitInfo *hi, int index, int dataType)
{
	int	n = dataType - IDM_DISP_EXTRA_FIRST;
	if (n < data->extra_hit_num) {
		hi->calibrated = *((float *)(data->extra_hit_data[n] + 1) + index);
		return(0);
	}
	hi->calibrated = INVALID_CAL;
	return(1);
}

#if defined(LOAD_CALIBRATION) && defined(ROOT_FILE)
/* messy routine that could be optimized for speed eventually if necessary */
static QPMT *getCalibratedPmt(ImageData *data,HitInfo *aHit)
{
	static QPMT *def_pmt=NULL;
	
	/* only calibrate events with valid times */
	if (data->event_time) {
	
		/* calibrate this event if not done already */
		if (data->snoman_counter != data->event_counter) {
			data->snoman_counter = data->event_counter;
			if (!data->root_cal_event) {
				data->root_cal_event = new QEvent;
			}
			root_get_event(data,data->root_cal_event);
			data->root_cal->Calibrate(data->root_cal_event);
		}
		
		/* find the pmt corresponding to aHit */
		HitInfo *hi = data->hits.hit_info;
		for (int i=0; i<data->hits.num_nodes; ++i, ++hi) {
			if (hi->flags & data->bit_mask) continue;	/* only consider unmasked hits */
			if (hi == aHit) {
				return(data->root_cal_event->GetAnyPMT(i));
			}
		}
		Printf("Internal error in getCalibratedPmt() - PMT %d not found!\n",aHit->index);
	}
	if (!def_pmt) {
		def_pmt = new QPMT;
		def_pmt->Sett(INVALID_CAL);
		def_pmt->Sethl(INVALID_CAL);
		def_pmt->Seths(INVALID_CAL);
		def_pmt->Setlx(INVALID_CAL);
	}

	return(def_pmt);
}
#endif

/* returns zero if calibration was done */
int setCalibratedTac( ImageData *data, HitInfo *hi, int index )
{
	if (data->wCalibrated == IDM_PRECALIBRATED) {
		if (data->calHits) {
			hi->calibrated = data->calHits[index].tac;
			return(hi->calibrated == INVALID_CAL);
		}
	} else {
#ifdef LOAD_CALIBRATION
#ifdef ROOT_FILE
		if (data->root_cal) {
			if (data->root_cal->IsA()==QSnomanCal::Class()) {
				QPMT *aPmt = getCalibratedPmt(data,hi);
				hi->calibrated = aPmt->Gett();
				return(0);
			} else {
				QPMT aPmt;
				aPmt.Setn(hi->index);
				aPmt.Setihs(hi->qhs);
				aPmt.Setihl(hi->qhl);
				aPmt.Setilx(hi->qlx);
				aPmt.Setit(hi->tac);
				aPmt.SetCell(hi->cell);
				if (!data->root_cal->Sett(&aPmt)) {
					hi->calibrated = aPmt.Gett();
					return(0);
				}
			}
		}
#else // ROOT_FILE
		if (data->pcal->GetStatus() >= 0) {
			data->pcal->SetChannel(hi->index, hi->cell);
			data->pcal->SetRaw(hi->tac, hi->qhs, hi->qhl, hi->qlx);
			data->pcal->SetRunNumber(data->run_number);
			data->pcal->Calibrate(kCalTac);
			hi->calibrated = data->pcal->GetTac();
			return(hi->calibrated == INVALID_CAL);
		}

#endif // ROOT_FILE
#endif // LOAD_CALIBRATION
	}
	// set calibrated value to indicate an error
	hi->calibrated = INVALID_CAL;
	return(1);
}

int setCalibratedQhl( ImageData *data, HitInfo *hi, int index )
{
	if (data->wCalibrated == IDM_PRECALIBRATED) {
		if (data->calHits) {
			hi->calibrated = data->calHits[index].qhl;
			return(hi->calibrated == INVALID_CAL);
		}
	} else {
#ifdef LOAD_CALIBRATION
#ifdef ROOT_FILE
		if (data->root_cal) {
			if (data->root_cal->IsA()==QSnomanCal::Class()) {
				QPMT *aPmt = getCalibratedPmt(data,hi);
				hi->calibrated = aPmt->Gethl();
				return(0);
			} else {
				QPMT aPmt;
				aPmt.Setn(hi->index);
				aPmt.Setihs(hi->qhs);
				aPmt.Setihl(hi->qhl);
				aPmt.Setilx(hi->qlx);
				aPmt.Setit(hi->tac);
				aPmt.SetCell(hi->cell);
				if (!data->root_cal->Sethl(&aPmt)) {
					hi->calibrated = aPmt.Gethl();
					return(0);
				}
			}
		}
#else // ROOT_FILE
		if (data->pcal->GetStatus() >= 0) {
			data->pcal->SetChannel(hi->index, hi->cell);
			data->pcal->SetRaw(hi->tac, hi->qhs, hi->qhl, hi->qlx);
			data->pcal->SetRunNumber(data->run_number);
			data->pcal->Calibrate(kCalQhl);
			hi->calibrated = data->pcal->GetQhl();
			return(hi->calibrated == INVALID_CAL);
		}
#endif // ROOT_FILE
#endif // LOAD_CALIBRATION
	}
	// set calibrated value to indicate an error
	hi->calibrated = INVALID_CAL;
	return(1);
}

int setCalibratedQhs( ImageData *data, HitInfo *hi, int index )
{
	if (data->wCalibrated == IDM_PRECALIBRATED) {
		if (data->calHits) {
			hi->calibrated = data->calHits[index].qhs;
			return(hi->calibrated == INVALID_CAL);
		}
	} else {
#ifdef LOAD_CALIBRATION
#ifdef ROOT_FILE
		if (data->root_cal) {
			if (data->root_cal->IsA()==QSnomanCal::Class()) {
				QPMT *aPmt = getCalibratedPmt(data,hi);
				hi->calibrated = aPmt->Geths();
				return(0);
			} else {
				QPMT aPmt;
				aPmt.Setn(hi->index);
				aPmt.Setihs(hi->qhs);
				aPmt.Setihl(hi->qhl);
				aPmt.Setilx(hi->qlx);
				aPmt.Setit(hi->tac);
				aPmt.SetCell(hi->cell);
				if (!data->root_cal->Seths(&aPmt)) {
					hi->calibrated = aPmt.Geths();
					return(0);
				}
			}
		}
#else // ROOT_FILE
		if (data->pcal->GetStatus() >= 0) {
			data->pcal->SetChannel(hi->index, hi->cell);
			data->pcal->SetRaw(hi->tac, hi->qhs, hi->qhl, hi->qlx);
			data->pcal->SetRunNumber(data->run_number);
			data->pcal->Calibrate(kCalQhs);
			hi->calibrated = data->pcal->GetQhs();
			return(hi->calibrated == INVALID_CAL);
		}
#endif // ROOT_FILE
#endif // LOAD_CALIBRATION
	}
	// set calibrated value to indicate an error
	hi->calibrated = INVALID_CAL;
	return(1);
}

int setCalibratedQlx( ImageData *data, HitInfo *hi, int index )
{
	if (data->wCalibrated == IDM_PRECALIBRATED) {
		if (data->calHits) {
			hi->calibrated = data->calHits[index].qlx;
			return(hi->calibrated == INVALID_CAL);
		}
	} else {
#ifdef LOAD_CALIBRATION
#ifdef ROOT_FILE
		if (data->root_cal) {
			if (data->root_cal->IsA()==QSnomanCal::Class()) {
				QPMT *aPmt = getCalibratedPmt(data,hi);
				hi->calibrated = aPmt->Getlx();
				return(0);
			} else {
				QPMT aPmt;
				aPmt.Setn(hi->index);
				aPmt.Setihs(hi->qhs);
				aPmt.Setihl(hi->qhl);
				aPmt.Setilx(hi->qlx);
				aPmt.Setit(hi->tac);
				aPmt.SetCell(hi->cell);
				if (!data->root_cal->Setlx(&aPmt)) {
					hi->calibrated = aPmt.Getlx();
					return(0);
				}
			}
		}
#else // ROOT_FILE
		if (data->pcal->GetStatus() >= 0) {
			data->pcal->SetChannel(hi->index, hi->cell);
			data->pcal->SetRaw(hi->tac, hi->qhs, hi->qhl, hi->qlx);
			data->pcal->SetRunNumber(data->run_number);
			data->pcal->Calibrate(kCalQlx);
			hi->calibrated = data->pcal->GetQlx();
			return(hi->calibrated == INVALID_CAL);
		}
#endif // ROOT_FILE
#endif // LOAD_CALIBRATION
	}
	// set calibrated value to indicate an error
	hi->calibrated = INVALID_CAL;
	return(1);
}


/*-------------------------------------------------------------------------------*/

ImageData *xsnoed_create(int load_settings)
{
	char		c[4];
	
	Printf("Version " XSNOED_VERSION
#ifdef LOAD_CALIBRATION
#ifdef SNOPLUS
          " (SNO+)"
#endif
#ifdef FITTR
	 	  " (FITTR)"
#else // FITTR
	 	  " (LOAD_CALIBRATION)"
#endif // FITTR
#endif // LOAD_CALIBRATION
#ifdef ROOT_FILE
		   " (ROOT_FILE)"
#endif // ROOT_FILE
#ifdef USE_PTHREAD
		   " (USE_PTHREAD)"
#endif // USE_PTHREAD
#ifdef OPTICAL_CAL
		   " (OPTICAL_CAL)"
#endif // OPTICAL_CAL
#ifdef NO_DISPATCH
		   " (NO_DISPATCH)"
#endif
#ifdef DEMO_VERSION
		   " (DEMO_VERSION)"
#endif
		   "\n");

	/* test to see if the code was compiled properly for this platform */
	c[0] = 0x01;
	c[1] = 0x02;
	c[2] = 0x03;
	c[3] = 0x04;
	SWAP_INT32(c,1);
	if (*(u_int32 *)c != 0x01020304UL) {
#ifdef SWAP_BYTES
		Printf("Compile error! SWAP_BYTES should NOT have been defined!\n");
#else
		Printf("Compile error! SWAP_BYTES should have been defined!\n");
#endif
	}
/*
** Create main window and menus
*/
	XSnoedWindow *main_window = new XSnoedWindow(load_settings);

	// default is to NOT delete data when xsnoed_create is called
	main_window->GetData()->mDeleteData = 0;
	
	return(main_window->GetData());
}

/* close all windows */
void xsnoed_delete(ImageData *data)
{
	if (data) {
		if (data->mMainWindow) {
			// get XSnoedWindow to delete our data for us
			data->mDeleteData = 1;
			// Close the xsnoed display
			// - deletes all related window objects
			delete data->mMainWindow;
		} else {
			freeData(data);
			delete data;
		}
	}
}

int xsnoed_running(ImageData *data)
{
	if (data && data->mMainWindow!=NULL) {
		return(1);
	} else {
		return(0);
	}
}

void xsnoed_next(ImageData *data, int step)
{
	if (step>0 && !data->history_evt && !data->history_all) {
	
		/* step forward in real time */
		PEventControlWindow::SetEventFilter(data);
		setTriggerFlag(data,TRIGGER_SINGLE);
		
	} else {
	
		/* turn off triggers */
		if (data->trigger_flag != TRIGGER_OFF) {
			setTriggerFlag(data,TRIGGER_OFF);
		}
		
		/* negative is backward in history, so change sign */
		showHistory(data, -step);
	}
}

void xsnoed_set_callback(ImageData *data, void (*aCallback)(void))
{
	data->event_callback = aCallback;
}

void xsnoed_save_event(ImageData *data, char *file_name, unsigned nwriter)
{	
	if (strlen(file_name) >= FILELEN) {
		Printf("Output file name too long!\n");
		return;
	}
	if (nwriter > NUM_WRITERS) {
		Printf("Writer index too large!\n");
		return;
	}
	if (strcmp(data->output_file[nwriter], file_name)) {
		if (data->mZdabWriter[nwriter]) {
			delete data->mZdabWriter[nwriter];
			data->mZdabWriter[nwriter] = NULL;
		}
		strcpy(data->output_file[nwriter], file_name);
	}
	if (file_name[0] == '\0') return;	// don't try to open empty file name
	
	if (!data->mZdabWriter[nwriter]) {
		data->mZdabWriter[nwriter] = new PZdabWriter(data->output_file[nwriter]);
		if (!data->mZdabWriter[nwriter]) {
			Printf("Not enough memory to create ZDAB writer!\n");
			return;
		}
	}
	PmtEventRecord *per = xsnoed_get_event(data);

	if (per) {
		if (!data->mZdabWriter[nwriter]->Write(per)) {
			if (data->hex_id) {
				Printf("Saved event 0x%lx to output zdab file %s\n",
						data->event_id, data->output_file[nwriter]);
			} else {
				Printf("Saved event %ld to output zdab file %s\n",
						data->event_id, data->output_file[nwriter]);
			}
		}
		free(per);
	}
}

// xsnoed_get_event - return PmtEventRecord from currently viewed event
// Note! -- caller must call free() on returned data when done
// (generates extended PmtEventRecord for fits and NCD data only)
PmtEventRecord *xsnoed_get_event(ImageData *data)
{
	int		extended = 1;
	
	int nhits = data->hits.num_nodes;
	u_int32 event_size = sizeof(PmtEventRecord) + nhits*sizeof(FECReadoutData);
	
	if (extended) {
		// add room for extended pmt event record info
		event_size += data->nrcon * (sizeof(FittedEvent) + sizeof(SubFieldHeader));
		
#ifdef SNOPLUS
        if (data->caenData) {
            // get pointer to ncd sub-field header
            u_int32 *caen = data->caenData - 1;
            // get size of field
            event_size += (*caen & SUB_LENGTH_MASK) * sizeof(u_int32);
        }
#else
        if (data->ncdData) {
            // get pointer to ncd sub-field header
            u_int32 *ncd = data->ncdData - 1;
            // get size of field
            event_size += (*ncd & SUB_LENGTH_MASK) * sizeof(u_int32);
		}
#endif
    }
	PmtEventRecord *per = (PmtEventRecord *)malloc(event_size);
	
	if (!per) {
		Printf("Out of memory in xsnoed_get_event()!");
		return(NULL);
	}

	// fill in the PmtEventRecord
	per->PmtEventRecordInfo = PMT_EVR_RECTYPE | PMT_EVR_NOT_MC | PMT_EVR_ZDAB_VER;
	per->DataType   = PMT_EVR_DATA_TYPE;
  	per->RunNumber  = data->run_number;
	per->EvNumber   = 0;
	per->DaqStatus  = data->sub_run;
	per->NPmtHit	= nhits;
	per->CalPckType = PMT_EVR_PCK_TYPE | PMT_EVR_CAL_TYPE;
	
	// copy over the trigger card data
	memcpy(&per->TriggerCardData, data->mtc_word, 6*sizeof(u_int32));
	
	// fill in the PMT hits
	// Note: Currently NOT setting CGT_ES24, CGT_ES16, Cmos_ES16 or MissedCount bits
	HitInfo *hi = data->hits.hit_info;
	u_int32 *fec_word = (u_int32 *)(per + 1);
	for (int i=0; i<nhits; ++i, ++hi) {
//		if (hi->flags & data->bit_mask) continue;	/* only consider unmasked hits */
		*(fec_word++) = (((u_int32)hi->card) << 26) |
				   		(((u_int32)hi->crate) << 21) |
				   		(((u_int32)hi->channel) << 16) |
				   		(hi->gt & 0x0000ffffUL);
		// FEC word 1
		*(fec_word++) = ((hi->flags & HIT_LGISEL) ? (1L << 30) : 0) |
						(((u_int32)hi->qhs ^ 0x800) << 16) |
				   		(((u_int32)hi->cell) << 12) |
				   		(((u_int32)hi->qlx ^ 0x800));
		// FEC word 2
		*(fec_word++) = ((hi->gt & 0x00f00000UL) << 8) |
				   		(((u_int32)hi->tac ^ 0x800) << 16) |
				   		((hi->gt & 0x000f0000UL) >> 4) |
				   		(((u_int32)hi->qhl ^ 0x800));
	}
	
	if (extended) {
		u_int32 *sub_header = &per->CalPckType;
		
		// must set the size of this sub-field before calling AddSubField()
		// (from the sub-field header to the end)
		*sub_header |= ((u_int32 *)((u_int32 *)(per + 1) + nhits * 3) - sub_header);
		
		// add fits
		for (int i=0; i<data->nrcon; ++i) {
			// add new sub-field to extended PmtEventRecord
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_FIT, sizeof(FittedEvent));
			// get pointer to start of fit data
			FittedEvent *theFit = (FittedEvent *)(sub_header + 1);
			RconEvent *rcon = data->rcon + i;
			theFit->x = rcon->pos[0];
			theFit->y = rcon->pos[1];
			theFit->z = rcon->pos[2];
			theFit->u = rcon->dir[0];
			theFit->v = rcon->dir[1];
			theFit->w = rcon->dir[2];
			theFit->time = rcon->time;
			theFit->quality = rcon->chi_squared;
			theFit->npmts = rcon->fit_pmts;
			theFit->spare = 0;
			memcpy(theFit->name, rcon->name, 32);
		}
		
#ifdef SNOPLUS
        // add CAEN trigger sums
        if (data->caenData) {
            u_int32 *caen = data->caenData - 1;
		    u_int32 len = ((*caen & SUB_LENGTH_MASK) - 1) * sizeof(u_int32);
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_CAEN, len);
			u_int32 *caen_out = (u_int32 *)(sub_header + 1);
			memcpy(caen_out, data->caenData, len);
        }
#else
		// add ncd data
		if (data->ncdData) {
            u_int32 *ncd = data->ncdData - 1;
		    u_int32 len = ((*ncd & SUB_LENGTH_MASK) - 1) * sizeof(u_int32);
			PZdabFile::AddSubField(&sub_header, SUB_TYPE_NCD, len);
			u_int32 *ncd_out = (u_int32 *)(sub_header + 1);
			memcpy(ncd_out, data->ncdData, len);
		}
#endif
	}
	return(per);
}

/* set time for event in ImageData structure, and do */
/* any other necessary initialization for the new event time */
static void setEventTime(ImageData *data, double aTime)
{
	data->event_time = aTime;
#if defined(LOAD_CALIBRATION) && defined(ROOT_FILE)
	if (data->root_cal && data->root_cal->IsA()==QSnoCal::Class()) {
		if (aTime) {
			if (fabs(aTime - data->root_cal_time) > CAL_TIME_THRESH) {
				// make the calibration valid for the new event time
				time_t iTime = (time_t)aTime;
				struct tm *tms = gmtime(&iTime);
				Int_t date2 = (((Int_t)tms->tm_year + 1900) * 100 + tms->tm_mon + 1) * 100 + tms->tm_mday;
				Int_t time2 = (((Int_t)tms->tm_hour * 100 + tms->tm_min) * 100 + tms->tm_sec) * 100 +
								(Int_t)((aTime - iTime) * 100);
				((QSnoCal *)data->root_cal)->MakeValid(date2,time2);
				data->root_cal_time = aTime;
			}
		}
	}
#endif
}

void setRconNodesVessel(RconEvent *event, float sphere_radius, float z_cutoff, float cyl_radius)
{
	int		i, no_dir;
	Vector3	v1,v2;
	Matrix3	evt_rot;
	float	tz;
	float	theta,thinc;
	float	cp = cos(event->cone_angle);
	float	sp = sin(event->cone_angle);

	if (event->num_nodes) {
		free(event->nodes);
		event->num_nodes = 0;
	}
	/* make sure direction is a unit vector */
	no_dir = unitVector(event->dir);
	
	if (no_dir) {
		event->num_nodes = 1;
	} else {
		event->num_nodes = 2 + NUM_CONE;
	}
	event->nodes = (Node *)XtMalloc(event->num_nodes * sizeof(Node));
	if (!event->nodes) {
		event->num_nodes = 0;
		return;	
	}
/*
** First node is event location, second node is calculated
** position of intersection with sphere:
*/
	event->nodes[0].x3 = event->pos[0];
	event->nodes[0].y3 = event->pos[1];
	event->nodes[0].z3 = event->pos[2];
	
	if (no_dir) return;	/* nothing more to do if we don't have a direction vector */
	
	/* calculate intesection of event line with sphere */
	hitSphere(event->pos,event->dir,event->nodes+1,1.0);
/*
** Now calculate position of Cherenkov cone and add to nodes array
** beginning at nodes[2].  Start by getting rotation matrix to bring
** the vector (0,0,1) to align with the event direction.
** We must ensure that rcon_dir is indeed a unit vector before
** using it to obtain the alignment rotation matrix.
** (Round-off errors can kill us here!)
*/
	unitVector(event->dir);
	getRotAlign(event->dir,evt_rot);

	thinc = 2*PI / NUM_CONE;
	
	if (event->nodes[0].z3 < z_cutoff) {
		for (i=2,theta=0; i<event->num_nodes; ++i,theta+=thinc) {
			v1[0] = cos(theta) * sp;
			v1[1] = sin(theta) * sp;
			v1[2] = cp;
			vectorMult(evt_rot,v1,v2);
			hitSphere(event->pos,v2,event->nodes+i,sphere_radius);
		}
	} else {
		tz = event->pos[2];
		event->pos[2] = 0;
		for (i=2,theta=0; i<event->num_nodes; ++i,theta+=thinc) {
			v1[0] = cos(theta) * sp;
			v1[1] = sin(theta) * sp;
			v1[2] = cp;
			vectorMult(evt_rot,v1,v2);
			hitSphere(event->pos,v2,event->nodes+i,cyl_radius);
			event->nodes[i].z3 += tz;
		}
		event->pos[2] = tz;
	}
}

void setRconNodes(RconEvent *event)
{
	/* set nodes of cherenkov cone */
	if (!strcmp(event->name,sWaterName[1])) {
		event->pos[0] = event->pos[1] = 0;
		setRconNodesVessel(event,
						   VESSEL_RADIUS / NOMINAL_TUBE_RADIUS,
						   NECK_BASE / NOMINAL_TUBE_RADIUS,
						   NECK_RADIUS / NOMINAL_TUBE_RADIUS );
	} else {
		setRconNodesVessel(event, 1.0, 1e20, 10.0);
	}
}

float getNcdHitVal(ImageData *data, NCDHit *nh)
{
    float val;
    switch (data->wNCDType) {
        case IDM_NCD_SHAPER_VAL:
            if (nh->shaper_count) {
                val = nh->shaper_value / (float)nh->shaper_count;
            } else {
                val = 0;
            }
            break;
        case IDM_NCD_MUX_HIT:
            val = nh->mux_count;
            break;
        case IDM_NCD_SHAPER_HIT:
            val = nh->shaper_count;
            break;
        case IDM_NCD_SCOPE_HIT:
            val = nh->scope_count;
            break;
        default:
            val = 0;
            break;
    }
    return(val);
}

// get hit value for currently displayed parameter
float getHitVal(ImageData *data, HitInfo *hi)
{
	float val;
	
	switch (data->wDataType) {
		case IDM_TAC:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				val = hi->calibrated;
			} else {
				val = hi->tac;
			}
			break;
		case IDM_QHS:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				val = hi->calibrated;
			} else {
				val = hi->qhs;
			}
			break;
		case IDM_QHL:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				val = hi->calibrated;
			} else {
				val = hi->qhl;
			}
			break;
		case IDM_QLX:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				val = hi->calibrated;
			} else {
				val = hi->qlx;
			}
			break;
		case IDM_QHL_QHS:
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				val = hi->calibrated;
			} else {
				val = hi->qhl - hi->qhs;
			}
			break;
		case IDM_NHIT:
#ifdef OPTICAL_CAL
			if (data->wCalibrated != IDM_UNCALIBRATED) {
				val = hi->calibrated;
				break;
			}
#endif
			// fall through!
		case IDM_CMOS_RATES:	/* cmos rates use NHIT entry of hit info */
			val = hi->nhit;
			break;
		case IDM_DISP_CRATE:
			val = hi->crate;
			break;
		case IDM_DISP_CARD:
			val = hi->card;
			break;
		case IDM_DISP_CHANNEL:
			val = hi->channel;
			break;
		case IDM_DISP_CELL:
			val = hi->cell;
			break;
		default:
			val = hi->calibrated;
			break;
	}
	return(val);
}

// getHitValPad - get hit value, padding with +0.5 for integer data types
float getHitValPad(ImageData *data, HitInfo *hi)
{
	float	val = getHitVal(data, hi);
	
	if (isIntegerDataType(data)) {
		val += 0.5;
	}
	return(val);
}

// calculate the colours corresponding to hit values for display
void calcHitVals(ImageData *data)
{
	int		i;
	float	val, first, last, range;
	long	ncols;
	HitInfo	*hi;
	int		n;

	hi = data->hits.hit_info;
	n  = data->hits.num_nodes;
	
	PEventHistogram::GetBins(data, &first, &last);
	range = last - first;
/*
** Calculate colour indices for each hit
*/
	ncols = data->num_cols - 2;
	for (i=0; i<n; ++i,++hi) {
		if (hi->flags & HIT_DISCARDED) {
			hi->hit_val = (int)ncols + 2;
			continue;
		}
		// calculate scaled hit value
		val = ncols * (getHitValPad(data, hi) - first) / range;

		// reset over/underscale flags
		hi->flags &= ~(HIT_OVERSCALE|HIT_UNDERSCALE);
		if (val < 0) {
			val = -1;
			hi->flags |= HIT_UNDERSCALE;	// set underscale flag
		} else if (val >= ncols) {
			val = ncols;
			hi->flags |= HIT_OVERSCALE;		// set overscale flag
		}
		hi->hit_val = (int)val + 1;
	}
}

void monteCarloSetFlags(ImageData *data, int flags, int set)
{
	if (data->monteCarlo) {
		MonteCarloVertex *first = (MonteCarloVertex *)(data->monteCarlo + 1);
		MonteCarloVertex *last = first + data->monteCarlo->nVertices;
		MonteCarloVertex *vertex;
		if (set) {
			for (vertex=first; vertex<last; ++vertex) {
				vertex->flags |= flags;
			}
		} else {
			flags = ~flags;
			for (vertex=first; vertex<last; ++vertex) {
				vertex->flags &= flags;
			}
		}
	}
}

struct DaughterList {
	int index;
	DaughterList *next;
};
static DaughterList	**sDaughterList  = NULL;
static MonteCarloVertex	*sVertexList = NULL;

static void monteCarloShowDaughter(DaughterList *daughter)
{
	while (daughter) {
		MonteCarloVertex *vertex = sVertexList + daughter->index;
		vertex->flags &= ~VERTEX_FLAG_HIDDEN;	// show this daughter
		// show all daughters of this vertex
		monteCarloShowDaughter(sDaughterList[daughter->index]);
		daughter = daughter->next;	// step to next daughter
	}
}

/* process monte carlo to set the appropriate flags */
void monteCarloProcess(ImageData *data)
{
	int					i, n, numVertices;
	MonteCarloVertex	*vertex, *first, *last, *v;
	
	if (!data->monteCarlo) return;
	
	numVertices = data->monteCarlo->nVertices;
	first = (MonteCarloVertex *)(data->monteCarlo + 1);
	last = first + numVertices;
	
	switch (data->wMCTrack) {
		case IDM_MC_ALL_TRACKS:
			// reset all hidden flags
			monteCarloSetFlags(data, VERTEX_FLAG_HIDDEN, FALSE);
			break;
		case IDM_MC_SINGLE_TRACK:
			// reset all HIDDEN, SPECIAL and MARK flags
			monteCarloSetFlags(data, VERTEX_FLAG_SPECIAL | VERTEX_FLAG_MARK | VERTEX_FLAG_HIDDEN, FALSE);
			return;	// don't hide masked particles
		case IDM_MC_SELECTED: {
			// first, find all the daughters for every track
			DaughterList *daughterList;
			daughterList = (DaughterList *)malloc(numVertices * (sizeof(DaughterList) + sizeof(DaughterList *)));
			if (!daughterList) break;
			DaughterList **firstDaughterPt = (DaughterList **)(daughterList + numVertices);
			DaughterList *nextEntry = daughterList;
			memset(firstDaughterPt, 0, numVertices * sizeof(DaughterList *));
			for (vertex=first,i=0; vertex<last; ++vertex,++i) {
				vertex->flags |= VERTEX_FLAG_HIDDEN;	// hide all tracks while we are at it
				n = vertex->parent;
				if (n<0 || n>=numVertices) continue;
				// insert into start of linked daughter list
				nextEntry->next = firstDaughterPt[n];
				nextEntry->index = i;
				firstDaughterPt[n] = nextEntry;
				++nextEntry;
			}
			// now show all reflected tracks
			for (vertex=first,i=0; vertex<last; ++vertex,++i) {
				int	flags;
				int int_code = (vertex->int_code / 1000) % 1000;
				switch (int_code) {
					case 201:	// spectral reflection
						flags = (1<<IM_SPECTRAL_REFL);
						break;
					case 202:	// diffuse reflection
						flags = (1<<IM_DIFFUSE_REFL);
						break;
					case 204:	// diffuse transmission
						flags = (1<<IM_DIFFUSE_TRANS);
						break;
					case 303:	// Rayleigh scatter
						flags = (1<<IM_RAYLEIGH_SCAT);
						break;
					case 305:	// bounce
						flags = (1<<IM_PMT_BOUNCE);
						break;
					case 402:	// reached PMT
						flags = (1<<IM_REACHED_PMT);
						break;
					default:
						continue;
				}
				if (!(flags & data->mcFlags)) continue;
				v = vertex;
				// show all vertices leading to this
				for (;;) {
					v->flags &= ~VERTEX_FLAG_HIDDEN;	// show it
					n = v->parent;
					if (n<0 || n>=numVertices) break;
					v = first + n;
				}
				// show all daughters
				sVertexList = first;
				sDaughterList = firstDaughterPt;
				monteCarloShowDaughter(firstDaughterPt[i]);
			}
			free(daughterList);
		}	break;
	}
	/* now hide masked particles */
	if (data->mcParticle) {
		for (vertex=first; vertex<last; ++vertex) {
			if (data->mcParticle & PMonteCarloWindow::ParticleMask(vertex)) {
				vertex->flags |= VERTEX_FLAG_HIDDEN;	// hide masked vertex
			}
		}
	}
}

void setTriggerFlag(ImageData *data, int theFlag, int end_of_data)
{
	int		oldFlag = data->trigger_flag;
	
	data->trigger_flag = theFlag;

	sendMessage(data, kMessageTriggerChanged);
	
	/* show last summed event */
	if (oldFlag==TRIGGER_CONTINUOUS && data->sum && data->sum_changed) {
		xsnoed_event(data, (aPmtEventRecord *)0);
	}
	
	if (!end_of_data) {
		/* set/reset throw out flag as necessary */
		ActivateTrigger(data);
	
		/* start handling events immediately */
		HandleEvents(data);
	}
}

/* set calibration type (only for calibrated version) */
#if defined(LOAD_CALIBRATION)
void setCalibration(ImageData *data,int calType)
{
	if (calType == IDM_PRECALIBRATED) {
		data->real_cal_scale = data->precal_real;
	} else if (calType==IDM_CAL_SIMPLE || calType==IDM_CAL_NO_WALK || 
#ifdef TITLES_CAL
		calType==IDM_CAL_PETER ||
#endif
		calType==IDM_UNCALIBRATED)
	{
		data->real_cal_scale = 0;
	} else {
		data->real_cal_scale = 1;
	}
#ifndef ROOT_FILE

	// initialize necessary calibration object
	switch (calType) {
		case IDM_CAL_SIMPLE:
			data->pcal_simple->SetWalk(1);
			if (data->pcal_simple->GetStatus() < 0) {
				data->pcal_simple->Init(data->calibration_file, data->file_path);
			}
#ifdef TITLES_CAL
			data->pcal_titles->Free();	// make sure titles banks are freed
#endif
			data->pcal = data->pcal_simple;
			break;
		case IDM_CAL_NO_WALK:
			data->pcal_simple->SetWalk(0);
			if (data->pcal_simple->GetStatus() < 0) {
				data->pcal_simple->Init(data->calibration_file, data->file_path);
			}
#ifdef TITLES_CAL
			data->pcal_titles->Free();	// make sure titles banks are freed
#endif
			data->pcal = data->pcal_simple;
			break;
#ifdef TITLES_CAL
		case IDM_CAL_PETER:
			data->pcal = data->pcal_titles;
			if (data->pcal_titles->GetStatus() < 0) {
				data->pcal_titles->Init(data->calcmd_file);
			}
			break;
#endif
	}

#else // !ROOT_FILE

	QCal	*oldCal;
	
	switch(calType) {
		case IDM_UNCALIBRATED:
			delete data->root_cal;
			data->root_cal = NULL;
			delete data->root_cal2;
			data->root_cal2 = NULL;
			break;
		case IDM_CAL_SIMPLE:
		case IDM_CAL_NO_WALK:
			if (!data->root_cal || data->root_cal->IsA()!=QSimpleCal::Class()) {
				if (!data->root_cal2 || data->root_cal2->IsA()!=QSimpleCal::Class()) {
					// save old calibration to make comparisons faster
					delete data->root_cal2;
					data->root_cal2 = data->root_cal;
					data->root_cal = NULL;
					// initialize default calibration file and path
					QSimpleCal::SetDefaultFileName(data->calibration_file);
					QSimpleCal::SetDefaultDirectory(data->file_path);
					oldCal = gCal;		// don't want to change global PCA object
					data->root_cal = new QSimpleCal;
					gCal = oldCal;		// restore original PCA object
					if (!data->root_cal) quit("Out of memory in setCalibration()");
					if (((QSimpleCal *)data->root_cal)->Init()) {
						// error initializing QSimpleCal object
						delete data->root_cal;
						data->root_cal = new QCal;	// use dummy calibrator
					}
				} else {
					// swap the calibration objects
					oldCal = data->root_cal;
					data->root_cal = data->root_cal2;
					data->root_cal2 = oldCal;
				}
			}
			// set time walk flag according to type of calibration
			((QSimpleCal *)data->root_cal)->DoTimeWalk(calType==IDM_CAL_SIMPLE);
			break;
		case IDM_CAL_SNODB:
			if (!data->root_cal || data->root_cal->IsA()!=QSnoCal::Class()) {
				if (!data->root_cal2 || data->root_cal2->IsA()!=QSnoCal::Class()) {
					// save old calibration to make comparisons faster
					delete data->root_cal2;
					data->root_cal2 = data->root_cal;
					data->root_cal = NULL;
					// create new snoman calibration object
					oldCal = gCal;		// don't want to change global PCA object
					data->root_cal = new QSnoCal("read");
					gCal = oldCal;		// restore original PCA object
					if (!data->root_cal) quit("Out of memory in setCalibration()");
/*					Printf("Connecting to SNODB server %s...\n",data->snodb_server);
					((QSnoCal *)data->root_cal)->SetServer(data->snodb_server);
					if (((QSnoCal *)data->root_cal)->PingServer()) {
						Printf("Error connecting to SNODB server!\n");
						delete data->root_cal;
						data->root_cal = new QCal;	// use dummy calibrator
					} else {
						Printf("Connected OK to SNODB server\n");
*/
						// load calibrations valid for this event time
						data->root_cal_time = 0;	// reset old calibration time
						setEventTime(data,data->event_time);
//					}
				} else {
					// swap the calibration objects
					oldCal = data->root_cal;
					data->root_cal = data->root_cal2;
					data->root_cal2 = oldCal;
				}
			}
			break;
		case IDM_CAL_SNOMAN:
			if (!data->root_cal || data->root_cal->IsA()!=QSnomanCal::Class()) {
				if (!data->root_cal2 || data->root_cal2->IsA()!=QSnomanCal::Class()) {
					// save old calibration to make comparisons faster
					delete data->root_cal2;
					data->root_cal2 = data->root_cal;
					data->root_cal = NULL;
					// create new snoman calibration object
					oldCal = gCal;		// don't want to change global PCA object
					data->root_cal = new QSnomanCal;
					gCal = oldCal;		// restore original PCA object
					if (!data->root_cal) quit("Out of memory in setCalibration()");
					Printf("Connecting to SNOMAN server %s...\n",data->snoman_server);
					((QSnomanCal *)data->root_cal)->SetServer(data->snoman_server);
					if (((QSnomanCal *)data->root_cal)->PingServer()) {
						Printf("Couldn't connect with SNOMAN server\n");
						delete data->root_cal;
						data->root_cal = new QCal;	// use dummy calibrator
					} else {
						Printf("Connected OK to SNOMAN server\n");
					}
				} else {
					// swap the calibration objects
					oldCal = data->root_cal;
					data->root_cal = data->root_cal2;
					data->root_cal2 = oldCal;
				}
			}
			break;
	}
#endif // ROOT_FILE
}
#endif // LOAD_CALIBRATION

// loads simple calibration file and fitter file
// First looks for a ".dat.bin" binary calibration file if
// the calibration filename ends in ".dat".  If not found, it
// will load the ".dat" (ascii) file, then create a ".dat.bin" file.
void initializeCalibrations( ImageData *data )
{
#ifdef LOAD_CALIBRATION

	if (!(data->init_cal & 0x01) &&
		// don't load calibration if precalibrated data available and used
		(data->wCalibrated != IDM_PRECALIBRATED))
	{
		data->init_cal |= 0x01;
#ifndef ROOT_FILE
// try without this
//		data->pcal->Init(data->calibration_file, data->file_path);
#endif // !ROOT_FILE
	}
	if (!(data->init_cal & 0x02)) {
		data->init_cal |= 0x02;
#ifdef FITTR
#ifndef NO_FITTER_DAT
		FILE *fp = openFile(FITTER_CONTROL_FILE, "r",data->file_path);
		if (fp) {
			data->pfit = initFitter( fp, data->light_speed );
			if (data->pfit) {
				Printf("Loaded fitter data from %s\n", getOpenFileName());
			} else {
				Printf("Error initializing fitter from %s\n",getOpenFileName());
			}
			fclose(fp);
		} else {
			Printf("Could not find fitter file %s\n",FITTER_CONTROL_FILE);
		}
#endif // NO_FITTER_DAT
		if (!data->pfit) {
			Printf("Using default fitter data\n");
			data->pfit = initFitter( NULL, data->light_speed );
		}
#endif // FITTR
	}
#ifdef ROOT_FILE
	if (!data->root_cal) {
		if (data->wCalibrated == IDM_UNCALIBRATED) {
			// we must be doing a fit with an uncalibrated display to get here
			// so load the simple calibration (the default) to do the fit
			setCalibration(data,IDM_CAL_SIMPLE);
		} else {
			setCalibration(data,data->wCalibrated);
		}
	}
#endif // ROOT_FILE
#endif // LOAD_CALIBRATION
}
		

void calcCalibratedVals(ImageData *data)
{
	int		i;
	float	xr,yr,zr,tr;
	float	len;
	HitInfo	*hi;
	int		n;
	Node	*node;
#ifdef OPTICAL_CAL
	int		num;
	float	sum;
#endif

	hi = data->hits.hit_info;
	n  = data->hits.num_nodes;

	switch (data->wDataType) {
		case IDM_DELTA_T:
			break;
#ifdef OPTICAL_CAL
		case IDM_NHIT:
			if (data->wCalibrated == IDM_UNCALIBRATED) return;
			break;
#endif
		case IDM_TAC:
		case IDM_QHS:
		case IDM_QHL:
		case IDM_QLX:
		case IDM_QHL_QHS:
			// no need to calculate this if not displaying calibrated data
			if (data->wCalibrated == IDM_UNCALIBRATED) return;
			break;
			
		default:
			// can set set extra calibrated values now (don't need calibration constants)
			if (data->wDataType>=IDM_DISP_EXTRA_FIRST && data->wDataType<=IDM_DISP_EXTRA_LAST) {
				for (i=0; i<n; ++i, ++hi) {
					setCalibratedExtra(data,hi,i,data->wDataType);
				}
			}
			return;
	}
	
	initializeCalibrations(data);

#if !defined(ROOT_FILE) && !defined(LOAD_CALIBRATION)
	if (data->wCalibrated != IDM_PRECALIBRATED) {
	
		for (i=0; i<n; ++i,++hi) {
			// set calibrated value to indicate an error
			hi->calibrated = INVALID_CAL;
		}
		Printf("Calibrations unavailable\n");
		
	} else {
#endif
	
		switch (data->wDataType) {
			case IDM_DELTA_T:
				node = data->hits.nodes;
				if (!data->nrcon) {
					xr = 0;
					yr = 0;
					zr = 0;
					tr = 0;
				} else {
					xr = data->rcon[data->curcon].pos[0];
					yr = data->rcon[data->curcon].pos[1];
					zr = data->rcon[data->curcon].pos[2];
					tr = data->rcon[data->curcon].time;
				}
				for (i=0; i<n; ++i,++hi,++node) {
					setCalibratedTac(data,hi,i);
					float r = data->tube_coordinates[hi->index].r;
					// Calculate time differences from the straight line path to the reconstructed point (xr,yr,zr)
					len  = vectorLen(node->x3*data->tube_radius - xr*r,
									 node->y3*data->tube_radius - yr*r,
									 node->z3*data->tube_radius - zr*r);
					hi->calibrated -= tr + len / data->light_speed;
				}
				break;
#ifdef OPTICAL_CAL
			case IDM_NHIT:
				if (data->oca) {
					data->source_factor = 1;
					for (i=0; i<n; ++i,++hi) {
						setCalibratedNHIT(data,hi,i);
					}
					/* calculate average value */
					sum = 0;
					num = 0;
					for (i=0,hi=data->hits.hit_info; i<n; ++i,++hi) {
						if (hi->calibrated) {
							sum += hi->calibrated;
							++num;
						}
					}
					/* normalize to an average of 1.0 */
					if (num) {
						tr = num / sum;
						for (i=0,hi=data->hits.hit_info; i<n; ++i,++hi) {
							hi->calibrated *= tr;
						}
						data->source_factor = tr;
						if (data->sum) {
							data->source_intensity = 1 / (tr * data->sum_event_count);
						} else {
							data->source_intensity = 1 / tr;
						}
					} else {
						data->source_intensity = 0;
						data->source_factor = 0;
					}	
					// update source intensity
					POpticalWindow::UpdateOpticalConstants(data,UPDATE_OSA_INTENSITY);
				} else {
					for (i=0; i<n; ++i,++hi) {
						hi->calibrated = hi->nhit;
					}
				}
				break;
#endif // OPTICAL_CAL
			case IDM_TAC:
				for (i=0; i<n; ++i,++hi) {
					setCalibratedTac(data,hi,i);
				}
				break;
			case IDM_QHS:
				for (i=0; i<n; ++i,++hi) {
					setCalibratedQhs(data,hi,i);
				}
				break;
			case IDM_QHL:
				for (i=0; i<n; ++i,++hi) {
					setCalibratedQhl(data,hi,i);
				}
				break;
			case IDM_QLX:
				for (i=0; i<n; ++i,++hi) {
					setCalibratedQlx(data,hi,i);
				}
				break;
			case IDM_QHL_QHS:
				for (i=0; i<n; ++i,++hi) {
					setCalibratedQhs(data,hi,i);
					tr = hi->calibrated;
					setCalibratedQhl(data,hi,i);
					hi->calibrated -= tr;
				}
				break;
		}
#if !defined(ROOT_FILE) && !defined(LOAD_CALIBRATION)
	}
#endif

	/* must re-calculate hit values */
	calcHitVals(data);
}

// Send message to windows and update
void sendMessage(ImageData *data, int message, void *dataPt)
{
	data->mSpeaker->Speak(message, dataPt);
}


// Calculate direction to sun in the SNO lab frame for the given time
// - uses SNO_LATITUDE, SNO_LONGITUDE and LAB_ROTATION constants to specify SNO lab frame
// - non-accurate method assumes circular orbit, and is good to about 5 degrees
// - accurate method is more time consuming, but is good to about 0.03 degrees
static void calcSunDirection(Vector3 sun_dir, time_t the_time, int useAccurateMethod=0)
{
	Vector3			d1;
	double			theta;
	const double	to_rad = PI / 180.0;

	if (useAccurateMethod) {
		// calculate direction to sun using elliptical earth orbit - PH 06/16/00
		// (ref: http://hotel04.ausys.se/pausch/comp/ppcomp.html)
		// (verification data: http://aa.usno.navy.mil/AA/data/docs/AltAz.html)
		static time_t tzero = 0;
		if (!tzero) {	
			struct tm tm2;
			tm2.tm_sec = 0;
			tm2.tm_min = 0;
			tm2.tm_hour = 0;		// correct for EST (no DST)
			tm2.tm_mday = 0;		// Yes, this is January zero!
			tm2.tm_mon = 0;
			tm2.tm_year = 2000 - 1900;
			tm2.tm_isdst = 0;
			tzero = mktime(&tm2);	// time zero for astronomical constants
			// correct for timezone to get tzero in UTC
#ifdef __MACHTEN__
			tzero -= 5 * 3600L;	// convert to GMT
#else
			tzero -= timezone;	// convert to GMT
#endif
		}
		// the number of days since Jan 0, 2000 (time zero for astronomical constants)
		double d = (the_time - tzero) / (24 * 3600.0);
		// the obliquity of the ecliptic
		double ecl = (23.4393 - 3.563E-7 * d) * to_rad;
		// the argument of perihelion
		double w = (282.9404 + 4.70935E-5 * d) * to_rad;
		// the eccentricity of earth's orbit
		double e = 0.016709 - 1.151E-9 * d;
		// the mean anomaly
		double ma = (356.0470 + 0.9856002585 * d) * to_rad;
		// the eccentric anomaly
		double ea = ma + e * sin(ma) * (1.0 + e * cos(ma));
		
		// the direction to the sun relative to the major axis of the elliptical orbit
		double xv = cos(ea) - e;
		double yv = sqrt(1.0 - e*e) * sin(ea);

		// the true anomaly (angle between position and perihelion)
		double v = atan2(yv, xv);

		// the Sun's true longitude in ecliptic rectangular geocentric coordinates
		double sun_longitude = v + w;

		// calculate the direction to sun including earth tilt
		// (x is east, y is north, z is away from the earth)
		double xs = cos(sun_longitude);
		double ys = sin(sun_longitude);
		d1[0] = ys * cos(ecl);
		d1[1] = ys * sin(ecl);
		d1[2] = xs;

		// calculate rotation of the earth about it's axis for this time of day
		// (k0 is empirically generated to agree with US Navy data - PH 06/16/00)
		double k0 = 0.27499;				// rotational position at time zero
		double k1 = 1.0 + 1.0 / 365.2425;	// rotational period of the earth in days
		double spin = k0 + k1 * d;			// spin of the earth (number of revolutions)
		// compute the spin angle (reducing to the range -2*PI to 2*PI)
		theta = (spin - (long)spin) * 2 * PI;

	} else {
	
		double	tilt;
		double	max_tilt = 23.5 * to_rad;		/* tilt of earth's axis */
		double	summer_solstice = 172.0;		/* day of year where earth tilt is maximum */
	
		// break down the time to get the time of day
		struct tm *tms = gmtime(&the_time);
		
		/* start with the direction to the sun at midnight from a location */
		/* on the prime meridian at the equator, including seasonal tilt */
		/* (z is up, y is north, x is east) */
		theta = (2. * PI) * (summer_solstice - tms->tm_yday) / 365.0;
		tilt = max_tilt * cos(theta);
		d1[0] = 0;
		d1[1] = sin(tilt);
		d1[2] = -cos(tilt);

		/* calculate earth rotation for this time of day */
		theta = (PI / 12.) * (tms->tm_hour + tms->tm_min / 60.0);
	}
	
	/* now move the detector to our longitude and latitude, */
   	/* rotating the earth to the appropriate time of day, */
	/* and finally rotate the y axis to construction north */
	/* alpha) about y axis, beta) about new x axis, gamma) about new z axis */
	double sno_longitude = SNO_LONGITUDE * to_rad;
	double sno_latitude = SNO_LATITUDE * to_rad;
	double true_north = LAB_ROTATION * to_rad;	/* true north (CCW from detector Y axis) */
	
	Matrix3 rot;
	getRotMatrixYXZ(rot, sno_longitude-theta, sno_latitude, true_north);

	// do the rotation, returning the result in 'sun_dir'
	vectorMult(rot,d1,sun_dir);
}

// Set sun direction in ImageData structure to correspond with event_time
void calcSunDirection(ImageData *data)
{
	time_t	the_time;
	long	this_calc_time;

	// get event time
	the_time = (time_t)data->event_time;
	
	// use current time if time is NULL in ImageData structure
	if (!the_time) the_time = (time_t)double_time();
	
	// don't recalculate the sun direction if the time is within 60 seconds
	this_calc_time = the_time / 60;
	
	if (data->last_calc_time != this_calc_time) {
		data->last_calc_time = this_calc_time;
		
		// get the sun direction for this time
		Vector3 sun_dir;
		calcSunDirection(sun_dir, the_time, 0);
		
		// set the sun direction in the ImageData structure
		data->sun_dir.x3 = sun_dir[0];
		data->sun_dir.y3 = sun_dir[1];
		data->sun_dir.z3 = sun_dir[2];
	
		if (data->mMainWindow) {
			// transform the sun direction by the current projection
			data->mMainWindow->GetImage()->Transform(&data->sun_dir, 1);
		}
		// inform listeners that the sun has moved
		sendMessage(data, kMessageSunMoved);
	}
}

static int verify_tube_coordinates(ImageData *data, int n)
{
//	unsigned int	cr, sl, ch, t;
	int				rtnVal = 0;
//	static int		msg_count = 0;
	
	if (!(data->tube_coordinates[n].status & PMT_OK)) {
/*		if (msg_count < MAX_MSGS) {
			++rtnVal;
			cr = n / (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			t = n - cr * (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			sl = t / NUM_CARD_CHANNELS;
			ch = t - sl * NUM_CARD_CHANNELS;
			Printf("Unmapped tube (crate %d card %d channel %d) - remapped to (0,0,1)\n",
					cr,sl,ch);
			if (++msg_count == MAX_MSGS) {
				Printf("Maximum number of error messages exceeded\n");
			}

		}
*/		/* put tube at top of detector */
		data->tube_coordinates[n].x = 0;
		data->tube_coordinates[n].y = 0;
		data->tube_coordinates[n].z = 1;
		data->tube_coordinates[n].r = 0;
		data->tube_coordinates[n].status |= (PMT_OK | PMT_FECD);
	}
	return(rtnVal);
}

/*
** xsnoed_add_rcons - add reconstructions to the event display
*/
void xsnoed_add_rcons(ImageData *data, RconEvent *rcon, int nrcon, int update_displays)
{
	int		i;
	
	// limit the total number of reconstructed points displayed
	if (nrcon + data->nrcon > MAX_RCON) {
		nrcon = MAX_RCON - data->nrcon;
		Printf("Too many fits\n");
	}
	
	for (i=0; i<nrcon; ++i) {
		/* copy reconstructed event into our ImageData */
		/* normalize coordinates to the tube radius */
		/* Note: Must NOT change num_nodes or nodes pointer in data->rcon!! */
		data->rcon[data->nrcon].pos[0] = rcon[i].pos[0] / data->tube_radius;
		data->rcon[data->nrcon].pos[1] = rcon[i].pos[1] / data->tube_radius;
		data->rcon[data->nrcon].pos[2] = rcon[i].pos[2] / data->tube_radius;
		data->rcon[data->nrcon].dir[0] = rcon[i].dir[0];
		data->rcon[data->nrcon].dir[1] = rcon[i].dir[1];
		data->rcon[data->nrcon].dir[2] = rcon[i].dir[2];
		data->rcon[data->nrcon].cone_angle = CONE_ANGLE;
		data->rcon[data->nrcon].time = rcon[i].time;
		data->rcon[data->nrcon].chi_squared = rcon[i].chi_squared;
		data->rcon[data->nrcon].fit_pmts = rcon[i].fit_pmts;
		memcpy(data->rcon[data->nrcon].name, rcon[i].name, 32);

		/* update the current rcon and the water rcon indices */
		if (!strcmp(rcon[i].name,sWaterName[0])) {
			data->rcon[data->nrcon].cone_angle = PI/2;
			data->watercon[0] = data->nrcon;
			if (data->curcon < 0) data->curcon = data->nrcon;
		} else if (!strcmp(rcon[i].name,sWaterName[1])) {
			data->rcon[data->nrcon].cone_angle = PI/2;
			data->watercon[1] = data->nrcon;
			if (data->curcon < 0) data->curcon = data->nrcon;
		} else {
			data->rcon[data->nrcon].cone_angle = CONE_ANGLE;
			data->curcon = data->nrcon;
		}
		/* set nodes of cherenkov cone */
		setRconNodes(data->rcon + data->nrcon);
		/* increment the rcon counter */
		data->nrcon++;
	}
	
	sendMessage(data, kMessageFitChanged);
	
	if (update_displays) {
		// must re-calculate relative hit times whenever rcon changes
		// (only necessary if we are currently displaying time differences)
		if (data->wDataType == IDM_DELTA_T) {
			calcCalibratedVals(data);
		}
		if (data->auto_vertex) {
			// move to the new fit vertex
			sendMessage(data, kMessageSetToVertex);
		} else {
			// transform the rcon nodes to the current projection
			data->mMainWindow->GetImage()->Transform(data->rcon[data->curcon].nodes,data->rcon[data->curcon].num_nodes);
		}
		// update the window title (must do this when curcon changes)
		newTitle(data);
	}
}

/*
** show the event
** Note: if data->sum is non-zero, the event is summed and not shown
**		 if the trigger is continuous -- set event_buff to NULL to show summed events
** Returns non-zero if event was drawn.
*/
static int showEvent(ImageData *data, HistoryEntry *event_buff)
{
	int				i, n, bad_hits=0;
	char			*pt;
	unsigned int	cr, sl, ch, t;
	u_int32			*thePmtHits, sum;
	int32			*cmos_rates_pt, cmos_rate;
	static int		msg_count = 0;
	HitInfo			*hit_info;
	double			ev_time;
	PmtEventRecord *pmtRecord;
#ifdef OPTICAL_CAL
	HitInfo			hit;
	int				doWindow;

	if (data->oca && data->wDataType==IDM_NHIT && data->wCalibrated!=IDM_UNCALIBRATED) {
		doWindow = 1;
	} else {
		doWindow = 0;
	}
#endif
    // clear sum automatically at regular intervals if specified
    if (data->sum && data->reset_sum &&
        data->last_reset_time + data->reset_sum_time * 60 < double_time())
    {
        clearSum(data);
    }
	// get pointer to the PMT event record
	if (event_buff) {
		pmtRecord = (PmtEventRecord *)(event_buff + 1);
        // load the proper databases for this event
        loadDatabases(data, pmtRecord->RunNumber);
	} else {
		pmtRecord = NULL;
	}

	// send message if this is the main display indicating we have a new event
	if (data->mMainWindow==PWindow::sMainWindow && pmtRecord!=NULL) {
		PResourceManager::sSpeaker->Speak(kMessageNewMainDisplayEvent, pmtRecord);
	}
	
	if (data->trigger_flag == TRIGGER_SINGLE) {
	
 		/* disarm the trigger */
		data->trigger_flag = TRIGGER_OFF;
		
		/* update the event filter popup */
		sendMessage(data, kMessageTriggerChanged);
		
		/* set flag to throw out any events after single trigger */
		data->throw_out_data = 1;
	}
#ifdef SNOPLUS
/*
** Handle the CAEN data
*/
	u_int32 *caen = NULL;
    if (pmtRecord) {
        if (!data->sum) {
            data->caenChannelMask = 0;
            data->caenPattern = 0;
            data->caenEventCount = 0;
            data->caenClock = 0;
            data->tubiiTrig = 0;
            data->tubiiGT = 0;
        }
        TubiiRecord *tubii = (TubiiRecord *)PZdabFile::GetExtendedData(pmtRecord, SUB_TYPE_TUBII);
        if (tubii) {
            data->tubiiTrig = tubii->TrigWord;
            data->tubiiGT = tubii->GTID;
        }
        caen = PZdabFile::GetExtendedData(pmtRecord, SUB_TYPE_CAEN);
        if (caen) {
            u_int32 len = (*(caen - 1) & SUB_LENGTH_MASK) - 1;
            if (len != UNPK_CAEN_WORD_COUNT(caen)) {
                printf("Invalid CAEN length (%d words in a %d word extended record)\n",
                    (int)UNPK_CAEN_WORD_COUNT(caen), (int)len);
                freeCaenData(data);
            } else {
                data->caenChannelMask = UNPK_CAEN_CHANNEL_MASK(caen);
                data->caenPattern     = UNPK_CAEN_PATTERN(caen);
                data->caenEventCount  = UNPK_CAEN_EVENT_COUNT(caen);
                data->caenClock       = UNPK_CAEN_TRIGGER_TIME(caen);
                // count the number of channels active
                int num_chan = 0;
                for (int i=0; i<kMaxCaenChannels; ++i) {
                    if (data->caenChannelMask & (1 << i)) ++num_chan;
                }
                if (num_chan) {
                    // length of each trace (in 32-bit words)
                    int trace_words = (UNPK_CAEN_WORD_COUNT(caen) - 4) / num_chan;
                    // number of samples in each trace (12-bit sample in 16-bit word)
                    int trace_samples = trace_words * 2;
                    if (data->caen_size != trace_samples) {
                        freeCaenData(data);
                        data->caen_size = trace_samples;
                    }
                    for (int i=0, n=0; i<kMaxCaenChannels; ++i) {
                        u_int16 *trace = data->caen_data[i];
                        if (!(data->caenChannelMask & (1 << i))) {
                            if (trace) {
                                delete trace;
                                data->caen_data[i] = NULL;
                            }
                            continue;
                        }
                        if (!trace) {
                            // allocate memory for this trace
                            trace = new u_int16[trace_samples];
                            if (!trace) {
                                printf("Out of memory for CAEN trace\n");
                                break;
                            }
                            data->caen_data[i] = trace;
                        }
                        u_int32 *wordPt = caen + 4 + n * trace_words;
                        for (int j=0; j<trace_samples; ++wordPt) {
                            trace[j++] = *(wordPt) & 0x0000ffff;
                            trace[j++] = *(wordPt) >> 16;
                        }
                        ++n;
                    }
                } else {
                    freeCaenData(data);
                }
            }
        } else {
            freeCaenData(data);
        }
    }
#else
/*
** Handle the NCD data
*/
	u_int32 *ncdData = NULL;
    if (pmtRecord) {
        // start from zero if not summing
        NCDHit *ncdHit = data->ncdHit;
        if (!data->sum) {
            data->numNcdHit = 0;
            data->general_count = 0;
            memset(ncdHit, 0, sizeof(data->ncdHit));
            freeNcdScopeData(data);
        }
        ncdData = PZdabFile::GetNcdData(pmtRecord);
        if (ncdData) {
            // get pointer to ncd sub-field header
            u_int32 *ncd = ncdData - 1;
            // get pointer to end of the NCD record
            u_int32 *ncd_end = ncd + (*ncd & SUB_LENGTH_MASK);
            // increment ncd pointer to next record
            int unknown_count = 0;
            const short kMaxUnknownRecords  = 20;
            int card,chan,mask;
            // remember our MUX scope hits
            struct {
                int ncd_num;
                int scope_num;
            } mux_scope[kMaxNCDs];
            int mux_scopes = 0;
            memset(mux_scope,0,sizeof(mux_scope));
            // remember our scope hit pattern
            int scope_hit[2];
            memset(scope_hit,0,sizeof(scope_hit));
            int scope_count = 0;
            // parse our NCD data
            while (++ncd < ncd_end) {
                switch (*ncd & kNcdDataTypeMask) {
                    case kShaperRecordType:
                        ++data->numNcdHit;
                        card = (*ncd & kShaperCard) >> 16;
                        chan = (*ncd & kShaperChan) >> 12;
                        if ((unsigned)card >= kNumShaperSlots) break;
                        if ((unsigned)chan >= kNumShaperChannels) break;
                        n = data->ncdShaperLookup[card][chan];
                        ++ncdHit[n].shaper_count;
                        ncdHit[n].flags |= HIT_SHAPER;
                        ncdHit[n].shaper_value += (*ncd & kShaperValue);
                        break;
                    case kMuxRecordType:
                        ++data->numNcdHit;
                        card = (*ncd & kMuxBox) >> 23;
                        mask = (*ncd & kMuxHitPattern);
                        if ((unsigned)card >= kNumMuxBuses) break;
                        for (chan=0; chan<(int)kNumMuxChannels; ++chan) {
                            if (mask & (1 << chan)) {
                                n = data->ncdMuxLookup[card][chan];
                                ++ncdHit[n].mux_count;
                                ncdHit[n].flags |= HIT_MUX;
                                // remember which scope should have fired for this mux
                                if ((*ncd & kMuxScope) && (mux_scopes < (int)kMaxNCDs)) {
                                    mux_scope[mux_scopes].ncd_num = n;
                                    if (*ncd & kMuxScope0) {
                                        mux_scope[mux_scopes].scope_num = 0;
                                    } else {
                                        mux_scope[mux_scopes].scope_num = 1;
                                    }
                                    ++mux_scopes;
                                }
                            }
                        }
                        break;
                    case kScopeRecordType:
                        ++data->numNcdHit;
                        chan = (*ncd & kScopeChannel) >> 19;
                        n = (*ncd & kScopeId) >> 23;
                        if (n < 2 && chan < 4) {
                            // remember scope channels hit
                            scope_hit[n] |= (1 << chan);
                            long data_size = *ncd & kScopeSize;
                            if (data_size < 130000) {
                                char *pt;
                                int num = n * 4 + chan;
                                if (data->ncd_scope_size == data_size) {
                                    pt = data->ncd_scope_data[num];
                                } else {
                                    freeNcdScopeData(data);
                                    pt = NULL;
                                }
                                // allocate memory for scope data if necessary
                                if (!pt) {
                                    pt = new char[data_size];
                                    if (pt) {
                                        data->ncd_scope_data[num] = pt;
                                        data->ncd_scope_size = data_size;
                                    } else {
                                        printf("Out of memory for NCD scope data\n");
                                    }
                                }
                                if (pt) {
                                    // save the raw scope data
                                    memcpy(pt, (char *)(ncd + 1), data_size);
                                    // reset log amp parameters for this scope
                                    // Note: I am assuming here that the kNCDScopeCal record
                                    // comes after kScopeRecordType or else we zero the current
                                    // calibration.
                                    memset(data->ncd_log_amp[num], 0, sizeof(data->ncd_log_amp[num]));
                                }
                            } else {
                                printf("Bad scope data size %ld\n", data_size);
                            }
                        } else {
                            printf("Bad scope data (scope %d channel %d)\n", n, chan);
                        }
                        ncd += (*ncd & kScopeSize) / sizeof(u_int32);   // scope data
                        ++scope_count;
                        break;
                    case kMuxGlobalStatusType:
                    case kSKIPRecordType:
                    case kGTIDRecordType:
                    case kScopeGTIDType:
                    case kHVType:
                        break;
                    case kTrigTimeRecordType:
                    case kScopeTimeRecordType:
//                    case kShGScalRecordType:
                        ++ncd;      // 2 words long
                        break;
                    case kNewRunRecordType:
                        ncd += 2;   // 3 words long
                        break;
                    case kGeneralDataType: {
                        u_int32 *gen_end = ncd + (*ncd & 0xffff) / sizeof(u_int32);   // variable length
                        ++ncd;  // skip over general record header
                        while (ncd < gen_end) {
                            switch (*ncd >> 16) {
                                case kNCDScopeCal: {
                                    n = (ncd[1] & 0xf0) >> 4;   // scope number
                                    ch = ncd[1] & 0x0f;         // scope channel
                                    if (n < 2 && ch < 4) {
                                        int num = n * 4 + ch;
                                        for (i=0; i<kNumLogAmpParms; ++i) {
                                            // save log amp parameters for this scope
                                            data->ncd_log_amp[num][i] = *(float *)(ncd + 2 + i);
                                        }
                                    }
                                }   break;
                            }
                            // step to next generic sub-record
                            if (!(*ncd & 0xffff)) {
                                printf("Bad NCD general sub-record!\n");
                                break;
                            }
                            ncd += (*ncd & 0xffff) / sizeof(u_int32);
                        }
                        ncd = gen_end;  // make sure we arrive at end of record
                        ++data->general_count;
                    }   break;
                    default:
                        ++unknown_count;
                        break;
                }
                if (unknown_count > kMaxUnknownRecords) break;
            }
            // now associate muxes with scopes
            NCDMap *ncd_map = data->ncdMap;
            int scope_found = 0;
            for (i=0; i<mux_scopes; ++i) {
                n = mux_scope[i].ncd_num;   // ncd index
                chan = ncd_map[n].scope_channel;    // channel of scope that should have fired
                // check to see if scope really fired
                if (scope_hit[mux_scope[i].scope_num] & (1 << chan)) {
                    ++ncdHit[n].scope_count;
                    ncdHit[n].flags |= HIT_SCOPE;
                    ++scope_found;
                }
            }
            // account for missing scope traces
            if (scope_found < scope_count) {
                ncdHit[data->numNcds].scope_count += (scope_count - scope_found);
                ncdHit[data->numNcds].flags |= HIT_SCOPE;
            }
        }
    }
#endif // SNOPLUS
/*
** add this event to the sum and display the sum
*/
	if (data->sum && pmtRecord) {
	
		/* count the number of events we summed */
		++data->sum_event_count;
		data->sum_changed = 1;		/* set flag indicating sum has changed */
		
		/* save necessary event information */
		data->sum_run_number = pmtRecord->RunNumber;
		data->sum_sub_run = pmtRecord->DaqStatus;
		data->sum_event_id  = pmtRecord->TriggerCardData.BcGT;
		data->sum_time = ((double) 4294967296.0 * pmtRecord->TriggerCardData.Bc10_2 + 
							 pmtRecord->TriggerCardData.Bc10_1) * 1e-7;
		data->sum_filename.SetString(event_buff->filename);
		if (data->sum_time) data->sum_time += data->sno_time_zero;
		memcpy(data->sum_mtc_word, &pmtRecord->TriggerCardData, 6*sizeof(u_int32));
		releaseHistoryEntry(data->sum_event);
		// make sure the event doesn't disappear from the history
		attachHistoryEntry(data->sum_event = event_buff);
		// it is OK now to store a pointer to the NCD data
#ifdef SNOPLUS
        data->sum_caenData = caen;
#else
		data->sum_ncdData = ncdData;
#endif
        data->sum_trig_word = PEventInfoWindow::GetTriggerWord(pmtRecord);
        
		/* get pointer to PMT data */
		thePmtHits = (u_int32 *)(pmtRecord + 1);
#ifdef OPTICAL_CAL_OUT
FILE *fp=fopen("oca_window.out","ab");
#endif
#if defined(OPTICAL_CAL) && defined(ROOT_FILE)
		if (doWindow) {
			if (data->root_cal && data->root_cal->IsA()==QSnomanCal::Class()) {
Printf("Sorry, can't yet window OCA with SNOMAN calibration\n");
/*
				// calculate calibration constants for event if SNOMAN calibration
				if (!data->root_cal_event) data->root_cal_event = new QEvent;
				QPmtEventRecord rec;
				rec.SetPmtEventRecord(pmtRecord);
				rec.ToQEvent(data->root_cal_event);
				data->event_counter++;
*/
			}
		}
#endif
		for (i=0; i<pmtRecord->NPmtHit; ++i,thePmtHits+=3) {
			
			cr = UNPK_CRATE_ID(thePmtHits);
			sl = UNPK_BOARD_ID(thePmtHits);
			ch = UNPK_CHANNEL_ID(thePmtHits);

			if (cr>=NUM_SNO_CRATES || sl>=NUM_CRATE_CARDS || ch>=NUM_CARD_CHANNELS) {
				if (msg_count < MAX_MSGS) {
					Printf("Error reading FEC data (crate %d card %d channel %d) %.8lx %.8lx %.8lx\n",
							cr,sl,ch,(long)thePmtHits[0],(long)thePmtHits[1],(long)thePmtHits[2]);
					if (++msg_count == MAX_MSGS) {
						Printf("Maximum number of error messages exceeded\n");
					}
				}
				// skip this hit
				++bad_hits;
				continue;
			}
			n = (cr * NUM_CRATE_CARDS + sl) * NUM_CARD_CHANNELS + ch;
#ifdef OPTICAL_CAL
			if (doWindow) {
#ifdef ROOT_FILE
				if (data->root_cal && data->root_cal->IsA()==QSnomanCal::Class()) {
					hit.calibrated = data->oca->Get_T0(n);	// TEMPORARILY DISABLE WINDOWING!
				} else {
#endif
					hit.index = n;
					hit.cell = UNPK_CELL_ID(thePmtHits);
					hit.tac = UNPK_TAC(thePmtHits);
					hit.qhl = UNPK_QHL(thePmtHits);
					setCalibratedTac(data,&hit,i);
#ifdef ROOT_FILE
				}
#endif

#ifdef OPTICAL_CAL_OUT
				fprintf(fp,"%g\n",data->oca->Get_T0(n)-hit.calibrated);
#endif
				if (fabs(data->oca->Get_T0(n) - hit.calibrated) > PROMPT_WINDOW_HALFWIDTH) {
					++data->missed_window_count;
					continue;
				}
			}
#endif // OPTICAL_CAL
			if (data->max_sum_nhit < (sum = ++data->sum_nhit[n])) {
				data->max_sum_nhit = sum;
			}
			++data->total_sum_nhit;	/* increment total number of tubes hit */
			
			/* keep track of number of tubes in the sum */
			if (sum == 1) ++data->sum_nhit_count;
			
			/* sum all other ADC values, keeping track of maximums */
			sum = (data->sum_tac[n] += UNPK_TAC(thePmtHits));
			if (data->max_sum_tac < sum) data->max_sum_tac = sum;
			sum = (data->sum_qhs[n] += UNPK_QHS(thePmtHits));
			if (data->max_sum_qhs < sum) data->max_sum_qhs = sum;
			sum = (data->sum_qhl[n] += UNPK_QHL(thePmtHits));
			if (data->max_sum_qhl < sum) data->max_sum_qhl = sum;
			sum = (data->sum_qlx[n] += UNPK_QLX(thePmtHits));
			if (data->max_sum_qlx < sum) data->max_sum_qlx = sum;
		}
#ifdef OPTICAL_CAL_OUT
fclose(fp);
#endif
		/* do not display summed event on continuous triggers */
		if (data->trigger_flag == TRIGGER_CONTINUOUS) return(0);
	}
/*
** Update the event structure
*/
	clearEvent(data);
	
	if (!pmtRecord && data->wDataType!=IDM_CMOS_RATES && !data->sum) {
		/* take pmtRecord from the history */
		if (data->history_evt >= 0) {
			event_buff = data->history_buff[data->history_all][data->history_evt];
		} else {
			event_buff = data->history_buff[HISTORY_FUTURE][-1-data->history_evt];
		}
		/* set pmtRecord pointer */
		if (event_buff) {
			pmtRecord = (PmtEventRecord *)(event_buff + 1);
#ifdef SNOPLUS
			caen = PZdabFile::GetExtendedData(pmtRecord, SUB_TYPE_CAEN);
#else
			ncdData = PZdabFile::GetNcdData(pmtRecord);
#endif
		}
	}
		
	if (pmtRecord) {
#ifdef SNOPLUS
	    data->caenData = caen;
#else
	    data->ncdData = ncdData;
#endif
		memcpy(data->mtc_word, &pmtRecord->TriggerCardData, 6*sizeof(u_int32));
		data->trig_word = PEventInfoWindow::GetTriggerWord(pmtRecord);
		data->run_number = pmtRecord->RunNumber;
		data->sub_run = pmtRecord->DaqStatus;
		data->event_id  = pmtRecord->TriggerCardData.BcGT;
		// set currently displayed filename for this event
		data->mDispFile.SetString(event_buff->filename);
	
		ev_time = ((double) 4294967296.0 * pmtRecord->TriggerCardData.Bc10_2 + 
    							 pmtRecord->TriggerCardData.Bc10_1) * 1e-7;
    	// add offset to time zero if the time was valid
    	if (ev_time) ev_time += data->sno_time_zero;
    	setEventTime(data,ev_time);
    	data->mMainWindow->DoWater(0);

		data->event_num = pmtRecord->EvNumber;
		calcSunDirection(data);
	}
/*
** Update the nodes and hit information
*/
	/* display CMOS rates data */
	if (data->wDataType == IDM_CMOS_RATES) {
	
    	setEventTime(data,double_time());
     	data->mMainWindow->DoWater(0);
   	
		data->sum_changed = 0;	/* reset changed flag since we are displaying sum now */

		/* count the number of channels with valid rates */
		n = 0;
		cmos_rates_pt = data->cmos_rates;
		for (i=0; i<NUM_TOTAL_CHANNELS; ++i) {
			if (*(cmos_rates_pt++) >= 0) ++n;
		}
		
		/* fill in hit information */
		data->event_counter++;
		data->hits.num_nodes = data->event_nhit = n;
		data->hits.nodes     = (Node *)   XtMalloc(n * sizeof(Node));
		data->hits.hit_info  = (HitInfo *)XtMalloc(n * sizeof(HitInfo));
		
		if (!data->hits.nodes || !data->hits.hit_info) quit("Out of memory");
		
		hit_info = data->hits.hit_info;
		cmos_rates_pt = data->cmos_rates;
		for (i=0,n=0; i<data->hits.num_nodes; ++i,++n) {
			/* look for next non-zero entry in sum */
			for (;;) {
				if (n >= NUM_TOTAL_CHANNELS) quit("bug in showEvent()");
				cmos_rate = *(cmos_rates_pt++);
				if (cmos_rate >= 0) break;
				++n;
			}
			bad_hits += verify_tube_coordinates(data,n);

			cr = n / (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			t = n - cr * (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			sl = t / NUM_CARD_CHANNELS;
			ch = t - sl * NUM_CARD_CHANNELS;
			hit_info->index = n;
			hit_info->gt = 0;
			hit_info->tac = 0;
			hit_info->qhs = 0;
			hit_info->qhl = 0;
			hit_info->qlx = 0;
			hit_info->nhit = cmos_rate;
			hit_info->crate = cr;
			hit_info->card = sl;
			hit_info->channel = ch;
			hit_info->cell = 0;
			hit_info->flags = data->tube_coordinates[n].status & HIT_PMT_MASK;
			
			data->hits.nodes[i].x3 = data->tube_coordinates[n].x;
			data->hits.nodes[i].y3 = data->tube_coordinates[n].y;
			data->hits.nodes[i].z3 = data->tube_coordinates[n].z;
			++hit_info;
		}

	/* display summed data */
	} else if (data->sum) {
	
#ifdef SNOPLUS
	    data->caenData = data->sum_caenData;
#else
        data->ncdData = data->sum_ncdData;
#endif
		data->sum_changed = 0;	/* reset changed flag since we are displaying sum now */
		setEventTime(data,data->sum_time);	/* set event time */
    	data->mMainWindow->DoWater(0);
		data->run_number = data->sum_run_number;
		data->sub_run = data->sum_sub_run;
		data->event_id = data->sum_event_id;
		memcpy(data->mtc_word, data->sum_mtc_word, 6*sizeof(u_int32));
		data->trig_word = data->sum_trig_word;
		data->mDispFile.SetString(data->sum_filename);
		
		/* fill in hit information */
		
		data->event_counter++;
		data->hits.num_nodes = data->event_nhit = data->sum_nhit_count;
		data->hits.nodes     = (Node *)   XtMalloc(data->hits.num_nodes*sizeof(Node));
		data->hits.hit_info  = (HitInfo *)XtMalloc(data->hits.num_nodes*sizeof(HitInfo));
		
		if (!data->hits.nodes || !data->hits.hit_info) quit("Out of memory");
		
		hit_info = data->hits.hit_info;
		for (i=0,n=0; i<data->hits.num_nodes; ++i,++n) {
			/* look for next non-zero entry in sum */
			for (;;) {
				if (n >= NUM_TOTAL_CHANNELS) quit("bug in showEvent()");
				if ((sum = data->sum_nhit[n]) != 0) break;
				++n;
			}
			bad_hits += verify_tube_coordinates(data,n);

			cr = n / (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			t = n - cr * (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			sl = t / NUM_CARD_CHANNELS;
			ch = t - sl * NUM_CARD_CHANNELS;
			hit_info->index = n;
			hit_info->gt = 0;
			hit_info->tac = data->sum_tac[n] / sum;
			hit_info->qhs = data->sum_qhs[n] / sum;
			hit_info->qhl = data->sum_qhl[n] / sum;
			hit_info->qlx = data->sum_qlx[n] / sum;
			hit_info->nhit = sum;
			hit_info->crate = cr;
			hit_info->card = sl;
			hit_info->channel = ch;
			hit_info->cell = 0;
			hit_info->flags = data->tube_coordinates[n].status & HIT_PMT_MASK;
			
			data->hits.nodes[i].x3 = data->tube_coordinates[n].x;
			data->hits.nodes[i].y3 = data->tube_coordinates[n].y;
			data->hits.nodes[i].z3 = data->tube_coordinates[n].z;
			++hit_info;
		}
			
	/* display normal PMT records */	
	} else if (pmtRecord) {

		/* clear nhit sum */
		memset(data->sum_nhit, 0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
		data->max_sum_nhit = 0;
		data->total_sum_nhit = 0;
#ifdef OPTICAL_CAL
		data->missed_window_count = 0;
#endif
		// initialize necessary variables before scanning sub-fields
		data->calHits = NULL;
		data->extra_hit_num = 0;
		data->extra_evt_num = 0;
		data->monteCarlo = NULL;
		
		FittedEvent *theFit = NULL;
		int numFits = 0;
		
		data->precal_real = 1;	// assume precalibrated data is real
		
		// run through the sub-fields, looking for additional information
		u_int32	*sub_header = &pmtRecord->CalPckType;
		while (*sub_header & SUB_NOT_LAST) {
			sub_header += (*sub_header & SUB_LENGTH_MASK);
			switch (*sub_header >> SUB_TYPE_BITNUM) {
				case SUB_TYPE_HIT_DATA:
					// extra hit data is available
					if (data->extra_hit_num < MAX_EXTRA_NUM) {
						ExtraHitData *hitData = (ExtraHitData *)(sub_header + 1);
						data->extra_hit_data[data->extra_hit_num] = hitData;
						// be sure name is null terminated
						hitData->name[DATA_NAME_LEN-1] = '\0';
						// extract the format information if available
						pt = strchr(hitData->name, '\0') + 1;
						// note the shortest possible format spec is 3 chars ("%g\0")
						if (pt > hitData->name+DATA_NAME_LEN-3 || *pt!='%') {
							pt = "%g";	// default to "%g" format
						}
						data->extra_hit_fmt[data->extra_hit_num] = pt;	// set the format string
						// increment extra hit data count
						++data->extra_hit_num;
					}
					break;
				case SUB_TYPE_EVENT_DATA:
					// extra event data is available
					if (data->extra_evt_num < MAX_EXTRA_NUM) {
						ExtraEventData *evtData = (ExtraEventData *)(sub_header + 1);
						data->extra_evt_data[data->extra_evt_num] = evtData;
						// be sure name is null terminated
						evtData->name[DATA_NAME_LEN-1] = '\0';
						// extract the format information if available
						pt = strchr(evtData->name, '\0') + 1;
						// note the shortest possible format spec is 3 chars ("%g\0")
						if (pt > evtData->name+DATA_NAME_LEN-3 || *pt!='%') {
							pt = "%g";	// default to "%g" format
						}
						data->extra_evt_fmt[data->extra_evt_num] = pt;	// set the format string
						// increment extra event data count
						++data->extra_evt_num;
					}
					break;
				case SUB_TYPE_CALIBRATED:
					// pre-calibrated data is available
					data->calHits = (CalibratedPMT *)(sub_header + 1);
					break;
				case SUB_TYPE_CAL_FLAGS:
					if (! (*(u_int32 *)(sub_header + 1) & CAL_FLAG_QSLOPE)) {
						// set real precalibrated data flag to zero if the
						// calibration didn't have a charge slope applied
						data->precal_real = 0;
					}
					break;
				case SUB_TYPE_MONTE_CARLO:
					data->monteCarlo = (MonteCarloHeader *)(sub_header + 1);
					monteCarloProcess(data);
					break;
				case SUB_TYPE_FIT:
					theFit = (FittedEvent *)(sub_header + 1);
					numFits = ( (*sub_header & SUB_LENGTH_MASK) * sizeof(u_int32)
								- sizeof(SubFieldHeader) ) / sizeof(FittedEvent);
					// add all fits in this record to list of xsnoed reconstructed events
					if (numFits) {
						// limit the total number of reconstructed points displayed
						if (numFits + data->nrcon > MAX_RCON) {
							numFits = MAX_RCON - data->nrcon;
							Printf("Too many fits\n");
						}
						
						for (i=0; i<numFits; ++i, ++theFit) {
							/* copy reconstructed event into our ImageData */
							/* normalize coordinates to the tube radius */
							/* Note: Must NOT change num_nodes or nodes pointer in data->rcon!! */
							data->rcon[data->nrcon].pos[0] = theFit->x / data->tube_radius;
							data->rcon[data->nrcon].pos[1] = theFit->y / data->tube_radius;
							data->rcon[data->nrcon].pos[2] = theFit->z / data->tube_radius;
							data->rcon[data->nrcon].dir[0] = theFit->u;
							data->rcon[data->nrcon].dir[1] = theFit->v;
							data->rcon[data->nrcon].dir[2] = theFit->w;
							data->rcon[data->nrcon].cone_angle = CONE_ANGLE;
							data->rcon[data->nrcon].time = theFit->time;
							data->rcon[data->nrcon].chi_squared = theFit->quality;
							data->rcon[data->nrcon].fit_pmts = theFit->npmts;
							memcpy(data->rcon[data->nrcon].name, theFit->name, 32);
							/* set the current rcon to this */
							data->curcon = data->nrcon;
							/* set nodes of cherenkov cone */
							setRconNodes(data->rcon + data->nrcon);
							/* increment the rcon counter */
							data->nrcon++;
						}
					}
					break;
				default:
					// ignore unrecognized fields
					break;
			}
		}
#ifdef DEBUG_EXTRA_DATA
		static int count = 0;
		struct {
			ExtraHitData data;
			float data_values[10240];
		} hit_data1 = { { "Test Long One" }, { 100 }},
		  hit_data2 = { { "Test Two" }, { 200 }};
		ExtraEventData evt_data1 = { { "Evt data 1" }, 100 };
		ExtraEventData evt_data2 = { { "More event data" }, 200 };
		switch (++count) {
			case 1:
				data->extra_hit_data[data->extra_hit_num] = &hit_data1.data;
				data->extra_hit_fmt[data->extra_hit_num] = "%.1f";
				++data->extra_hit_num;
				data->extra_hit_data[data->extra_hit_num] = &hit_data2.data;
				data->extra_hit_fmt[data->extra_hit_num] = "%g";
				++data->extra_hit_num;
				data->extra_evt_data[data->extra_evt_num] = &evt_data1;
				data->extra_evt_fmt[data->extra_evt_num] = "%.2f";
				++data->extra_evt_num;
				data->extra_evt_data[data->extra_evt_num] = &evt_data2;
				data->extra_evt_fmt[data->extra_evt_num] = "%g";
				++data->extra_evt_num;
				break;
			case 2:
				data->extra_hit_data[data->extra_hit_num] = &hit_data2.data;
				data->extra_hit_fmt[data->extra_hit_num] = "%g";
				++data->extra_hit_num;
				data->extra_hit_data[data->extra_hit_num] = &hit_data1.data;
				data->extra_hit_fmt[data->extra_hit_num] = "%.1f";
				++data->extra_hit_num;
				data->extra_evt_data[data->extra_evt_num] = &evt_data2;
				data->extra_evt_fmt[data->extra_evt_num] = "%g";
				++data->extra_evt_num;
				data->extra_evt_data[data->extra_evt_num] = &evt_data1;
				data->extra_evt_fmt[data->extra_evt_num] = "%.2f";
				++data->extra_evt_num;
				break;
			case 3:
				data->extra_hit_data[data->extra_hit_num] = &hit_data1.data;
				data->extra_hit_fmt[data->extra_hit_num] = "%.1f";
				++data->extra_hit_num;
				data->extra_evt_data[data->extra_evt_num] = &evt_data1;
				data->extra_evt_fmt[data->extra_evt_num] = "%.2f";
				++data->extra_evt_num;
				break;
			default:
				count = 0;
				break;
		}
#endif
		// update data menu and set current data type for display
		data->mMainWindow->UpdateDataMenu();
/*
** get the rest of the event information
*/
		data->event_counter++;
		data->hits.num_nodes = data->event_nhit = pmtRecord->NPmtHit;
		data->hits.nodes     = (Node *)   XtMalloc(data->hits.num_nodes*sizeof(Node));
		data->hits.hit_info  = (HitInfo *)XtMalloc(data->hits.num_nodes*sizeof(HitInfo));
		if (!data->hits.nodes || !data->hits.hit_info) quit("Out of memory");
		
		/* get pointer to PMT data */
		thePmtHits = (u_int32 *)(pmtRecord + 1);
		hit_info = data->hits.hit_info;
		for (i=0; i<data->hits.num_nodes; thePmtHits+=3) {
			
			cr = UNPK_CRATE_ID(thePmtHits);
			sl = UNPK_BOARD_ID(thePmtHits);
			ch = UNPK_CHANNEL_ID(thePmtHits);

			if (cr>=NUM_SNO_CRATES || sl>=NUM_CRATE_CARDS || ch>=NUM_CARD_CHANNELS) {
				if (msg_count < MAX_MSGS) {
					Printf("Error reading FEC data (crate %d card %d channel %d) %.8lx %.8lx %.8lx\n",
							cr,sl,ch,(long)thePmtHits[0],(long)thePmtHits[1],(long)thePmtHits[2]);
					if (++msg_count == MAX_MSGS) {
						Printf("Maximum number of error messages exceeded\n");
					}
				}
				// skip this hit
				--data->hits.num_nodes;
				++bad_hits;
				continue;
			}
			n = (cr * NUM_CRATE_CARDS + sl) * NUM_CARD_CHANNELS + ch;
			
			/* increment hit counter and keep track of maximum hits per tube */
			if (data->max_sum_nhit < (sum = ++data->sum_nhit[n])) {
				data->max_sum_nhit = sum;
			}
			++data->total_sum_nhit;
			
			/* don't add double hits to the display node list */
			if (sum > 1) {
				--data->hits.num_nodes;
				continue;
			}
			
			bad_hits += verify_tube_coordinates(data,n);

			hit_info->index = n;
			hit_info->gt = UNPK_FEC_GT24_ID(thePmtHits);
			hit_info->tac = UNPK_TAC(thePmtHits);
			hit_info->qhs = UNPK_QHS(thePmtHits);
			hit_info->qhl = UNPK_QHL(thePmtHits);
			hit_info->qlx = UNPK_QLX(thePmtHits);
			hit_info->nhit = sum;
			hit_info->crate = cr;
			hit_info->card = sl;
			hit_info->channel = ch;
			hit_info->cell = UNPK_CELL_ID(thePmtHits);
			hit_info->flags = data->tube_coordinates[n].status & HIT_PMT_MASK;
			if (UNPK_LGI_SELECT(thePmtHits)) {
				hit_info->flags |= HIT_LGISEL;
			}

			data->hits.nodes[i].x3 = data->tube_coordinates[n].x;
			data->hits.nodes[i].y3 = data->tube_coordinates[n].y;
			data->hits.nodes[i].z3 = data->tube_coordinates[n].z;
			
			++i;
			++hit_info;
		}
		/* must re-run through events and update hit counts */
		/* if we had any double hits (i.e. max_sum_nhit is > 1) */
		if (data->max_sum_nhit > 1) {
			hit_info = data->hits.hit_info;
			for (i=0; i<data->hits.num_nodes; ++i,++hit_info) {
				hit_info->nhit = data->sum_nhit[hit_info->index];
			}
		}
		/* apply the auto fit if selected */
		if (data->autoFit) {
			doFit(data, 0);		// do the fit without updating displays
		}
	}
	if (bad_hits) {
		Printf("%d bad hits in event %ld\n", bad_hits, data->event_id);
	}
	
	// do callback if it exists before displaying new event
	if (data->event_callback) {
		data->in_callback = 1;
		data->event_callback();
		data->in_callback = 0;
	}

	data->event_shown = 1;
	newTitle(data);				/* update main window title */
	
	/* calculate the calibrated values if necessary */
	calcCalibratedVals(data);
	
	/* calculate the hit colour indices */
	calcHitVals(data);
/*
** update the displays
*/
	if (data->auto_vertex && data->nrcon) {
		sendMessage(data, kMessageSetToVertex);
	} else if (data->auto_sun) {
		sendMessage(data, kMessageSetToSun);
	}
	sendMessage(data, kMessageNewEvent);
	
	return(1);
}


void clearEvent(ImageData *data)
{
	int		i;
	
	if (data->hits.num_nodes) {
		free(data->hits.nodes);
		free(data->hits.hit_info);
		data->hits.num_nodes = 0;
		data->event_nhit = 0;
		data->event_counter++;
	}
	for (i=0; i<data->nrcon; ++i) {
		if (data->rcon[i].num_nodes) {
			free(data->rcon[i].nodes);
			data->rcon[i].num_nodes = 0;
		}
	}
	data->nrcon  = 0;
	data->curcon = -1;
	data->watercon[0] = -1;
	data->watercon[1] = -1;
	data->cursor_hit = -1;
	data->cursor_ncd = -1;
	
	memset(data->mtc_word, 0, 6*sizeof(u_int32));
	data->trig_word = 0;
	data->run_number = 0;
	data->sub_run = -1;
	data->event_id = 0;
	data->calHits = NULL;
	data->ncdData = NULL;
#ifdef SNOPLUS
    data->caenData = NULL;
#endif
	data->extra_hit_num = 0;		// reset counter for available extra hit data
	data->extra_evt_num = 0;		// reset counter for available extra event data
	data->monteCarlo = NULL;
	setEventTime(data,0);
	data->event_num = 0;
	data->display_time = 0;
	data->event_shown = 0;
	data->mDispFile.Release();		// release displayed event filename
/*
	// reset manual scaling if not currently dragging scales
	PHistImage *hist = PEventHistogram::GetEventHistogram(data);
	if (hist && !(hist->GetGrabFlag() & GRAB_ACTIVE)) {
		hist->ResetGrab();
	}
*/	
	if (data->mSpeaker) {
		sendMessage(data, kMessageEventCleared);
	}
}


/* clear event sum variables */
void clearSum(ImageData *data)
{
	memset(data->sum_nhit, 0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
	data->max_sum_nhit = 0;
	data->total_sum_nhit = 0;
	data->last_reset_time = double_time();
#ifdef OPTICAL_CAL
	data->missed_window_count = 0;
#endif
	
	if (data->sum) {
		memset(data->sum_tac,  0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
		memset(data->sum_qhs,  0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
		memset(data->sum_qhl,  0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
		memset(data->sum_qlx,  0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
		data->max_sum_tac  = 0;
		data->max_sum_qhs  = 0;
		data->max_sum_qhl  = 0;
		data->max_sum_qlx  = 0;
		data->sum_event_count = 0;
		data->sum_nhit_count = 0;
		data->sum_changed = 0;
		data->sum_time = 0;
		data->sum_run_number = 0;
		data->sum_sub_run = -1;
		data->sum_event_id = 0;
		data->sum_filename.Release();
		releaseHistoryEntry(data->sum_event);
		data->sum_event = NULL;
		data->sum_ncdData = NULL;
#ifdef SNOPLUS
        data->sum_caenData = NULL;
#endif
		memset(data->sum_mtc_word, 0, 6*sizeof(u_int32));
		data->sum_trig_word = 0;
	}
	if (data->cmos_rates) {
		memset(data->cmos_rates, -1, NUM_TOTAL_CHANNELS * sizeof(int32));
	}
	if (data->numNcdHit) {
	    data->numNcdHit = 0;
	    NCDHit *ncdHit = data->ncdHit;
	    memset(ncdHit,0,sizeof(data->ncdHit));
	}
	data->general_count = 0;    // clear count of general NCD records
#ifndef SNOPLUS
	freeNcdScopeData(data);
#endif
}


/* remove event from 'future' history buffer */
/* and display any future messages */
HistoryEntry *removeFutureEvent(ImageData *data)
{
	HistoryEntry	*event_buff;
	int				num = data->history_size[HISTORY_FUTURE];
	
	if (num != 0) {
	
		/* get the event from our 'future' buffer */
		event_buff = data->history_buff[HISTORY_FUTURE][0];
		
		/* remove the first event from the buffer */
		if (--data->history_size[HISTORY_FUTURE] > 0) {
			/* move all events down by one in future buffer */
			memmove(data->history_buff[HISTORY_FUTURE], data->history_buff[HISTORY_FUTURE]+1,
					data->history_size[HISTORY_FUTURE] * sizeof(void*));
		}
		/* set last entry to NULL */
		data->history_buff[HISTORY_FUTURE][data->history_size[HISTORY_FUTURE]] = NULL;
		
		/* handle future messages */
		if (data->future_msg[0]) {
			printf("%s",data->future_msg[0]);
			delete [] data->future_msg[0];
		}
		// move the future messages down one
		memmove(data->future_msg, data->future_msg+1, (num-1) * sizeof(char *));
		data->future_msg[num-1] = NULL;
		
	} else {
		event_buff = NULL;
	}
		
	return(event_buff);
}

/* handleHeaderRecord - handle run records read from file or dispatcher */
/* (if bank_name == 0, clear existing records from memory) */
void handleHeaderRecord(ImageData *data, u_int32 *bank_data, u_int32 bank_name)
{
	int			i;
	unsigned	size;
	
	switch (bank_name) {
		case 0:		// flag to clear all records
			for (i=0; i<kNumHdrRec; ++i) {
				if (data->mHdrRec[i]) {
					delete [] data->mHdrRec[i];
					data->mHdrRec[i] = NULL;
					sendMessage(data, kMessageNewHeaderRecord, &i);
				}
			}
			return;	// all done, so return from here
			
		case RUN_RECORD:
		case RHDR_RECORD:
			i = kHdrRec_RHDR;
			size = sizeof(SBankRHDR);
			break;
		case TRIG_RECORD:
			i = kHdrRec_TRIG;
			size = sizeof(SBankTRIG);
			break;
		case EPED_RECORD:
			i = kHdrRec_EPED;
			size = sizeof(SBankEPED);
			break;
		case CAST_RECORD:
			i = kHdrRec_CAST;
			size = sizeof(SBankCAST);
			break;
		case CAAC_RECORD:
			i = kHdrRec_CAAC;
			size = sizeof(SBankCAAC);
			break;
		case SOSL_RECORD:
			i = kHdrRec_SOSL;
			size = sizeof(SBankSOSL);
			break;
		default:
			return;
	}
	if (!data->mHdrRec[i]) {
		data->mHdrRec[i] = (u_int32 *)new char[size];
		if (!data->mHdrRec[i]) return;	// give up if no memory for record
	}
	// save the new bank data
	memcpy(data->mHdrRec[i], bank_data, size);
	
	// send a message indicating the data has changed
	sendMessage(data, kMessageNewHeaderRecord, &i);
}

/* add event to history */
/* whichBuff = 0 - HISTORY_VIEWED (viewed history) */
/*		     = 1 - HISTORY_ALL    (history of all events) */
/*		     = 2 - HISTORY_FUTURE ('history' of future events) */
/* Note: viewed history events are assumed to be the last events added to the 'all' history */
/*
** history record:
**		1) int32 - usage counter
**		2) PmtEventRecord (size rounded up to nearest int32)
*/
void addToHistory(ImageData *data, aPmtEventRecord *pmtRecord, int whichBuff)
{
	int				is_new_entry;
	double			time_diff;
	HistoryEntry	*event_buff;
	u_int32			event_size;
	
	if (!pmtRecord) {		/* may be nil if we are summing */
		if (whichBuff==HISTORY_FUTURE && data->history_size[whichBuff]<FUTURE_SIZE) {
			// a 'null' in future buffer indicates EOF
			data->history_size[HISTORY_FUTURE]++;
			// send message so the Event Times window knows there is no more data
			sendMessage(data, kMessageHistoryChangeEnd);
		}
		return;
	}
	
	/* check to see if this event is intended for 'all' history */
	/* If so, we only need to allocate new memory if it is not a 'future' event */
	switch (whichBuff) {
		case HISTORY_VIEWED:
			is_new_entry = 0;
			break;
		case HISTORY_ALL:
			/* entry is only new if it doesn't come from the 'future' buffer */
			if (data->history_size[HISTORY_FUTURE] != 0) {
				/* look at next event from 'future' buffer */
				event_buff = data->history_buff[HISTORY_FUTURE][0];
				/* is it the same event? */
				if (pmtRecord == (PmtEventRecord *)(event_buff + 1)) {
					/* yes.  This is not a new entry */
					is_new_entry = 0;
				} else {
					/* no. We must move all 'future' events into the 'all' history */
					/* to keep the buffer events sequential */
					while (data->history_size[HISTORY_FUTURE]) {
						HistoryEntry *entry = data->history_buff[HISTORY_FUTURE][0];
						if (!entry) {
							Printf("Ouch! NULL entry in future buffer during add to 'all' history!\n");
							break;
						}
						/* move the first future event into the 'all' history */
						addToHistory(data, (PmtEventRecord *)(entry + 1), HISTORY_ALL);
					}
					is_new_entry = 1;
				}
			} else {
				is_new_entry = 1;
			}
			break;
		case HISTORY_FUTURE:
			is_new_entry = 1;
			break;
		default:
			quit("Invalid history buffer number");
			return;
	} 
	
	/* change size of history buffer to accomodate this entry */
	if (whichBuff==HISTORY_FUTURE) {
		if (data->history_size[HISTORY_FUTURE] < FUTURE_SIZE) {
			/* increment our history count */
			data->history_size[whichBuff]++;
		} else {
//			data->lost_future = 1;
			return;
		}
	} else if (data->history_size[whichBuff] < HISTORY_SIZE) {
		/* increment our history count */
		data->history_size[whichBuff]++;
	} else {
		/* delete last entry from full buffer */
		event_buff = data->history_buff[whichBuff][FUTURE_SIZE];
        // be sure to reset last non-orphan pointer if necessary
        // so it doesn't point into deleted memory
        if (data->last_non_orphan_history_entry == event_buff) {
            data->last_non_orphan_history_entry = NULL;
        }
        releaseHistoryEntry(event_buff);
	}
	
	if (is_new_entry) {

		event_size = PZdabFile::GetSize(pmtRecord);
		
		/* round up to nearest event int32 */
		event_size = (event_size + (sizeof(int32)-1)) & ~(sizeof(int32)-1);
		/* allocate room for event plus HistoryEntry structure */
		event_buff = (HistoryEntry *)malloc(sizeof(HistoryEntry) + event_size);
		if (!event_buff) {
			Printf("Out of memory for event buffer!\n");
			return;
		}
		// must properly construct the PMultiString object since we
		// didn't allocate memory with the C++ 'new' operator
		event_buff->filename.Construct();
#ifdef DEBUG_MEMORY
		++alloc_count;
#endif
		/* initialize memory usage counter (used once to start) */
		event_buff->usage_count = 1;
		/* initialize next event time to -1 (indicates we don't know the time) */
		event_buff->next_time = -1;
		/* set filename for this event (shared allocation with current event filename) */
		event_buff->filename.SetString(data->mEventFile);
		/* calculate event time differences only for non-orphan events */
		if (!isOrphan(pmtRecord)) {
			/* do we have a valid event to compare this with? */
			if (data->last_non_orphan_history_entry) {
				/* calculate time difference */
				time_diff = get50MHzTime(pmtRecord) -
							get50MHzTime((PmtEventRecord *)(data->last_non_orphan_history_entry + 1));
				// handle wrap of 50MHz clock
				if (time_diff < 0) time_diff += get50MHzTimeMax();
				// fill in this event's 'prev_time' and last event's 'next_time'
				event_buff->prev_time = data->last_non_orphan_history_entry->next_time = (float)time_diff;
				// check to see if we are currently viewing the last non-orphan event
				HistoryEntry *viewedEntry = getCurrentHistoryEntry(data);
				if (viewedEntry == data->last_non_orphan_history_entry) {
					// we are viewing the last non-orphan event, so send message
					// to indicate the 'next' time is now available for display
					sendMessage(data, kMessageNextTimeAvailable);
				}
			} else {
				event_buff->prev_time = -1;			// don't know time to previous event
			}
			data->last_non_orphan_history_entry = event_buff;	// update pointer to last history entry
		} else {
			event_buff->prev_time = -1;
		}
		
		/* copy event into buffer */
		memcpy(event_buff+1, pmtRecord, event_size);

		/* inform necessary objects that the history has changed */
		sendMessage(data, kMessageHistoryChanged);
		
		if (whichBuff == HISTORY_FUTURE) {
		
			/* put event at end of future buffer */
			data->history_buff[HISTORY_FUTURE][data->history_size[HISTORY_FUTURE]-1] = event_buff;

			return;	// done
		}
			
	} else if (whichBuff == HISTORY_ALL) {
		/* remove first event from future buffer */
		event_buff = removeFutureEvent(data);
		/* our history buffers have changed */
		sendMessage(data, kMessageHistoryChanged);
	} else {
		/* copy last filtered event into viewed history */
		event_buff = data->history_buff[HISTORY_ALL][0];
		if (event_buff) {
		    attachHistoryEntry(event_buff);    // attach to this event
		} else {
			Printf("Ouch! NULL event into 'viewed' history\n");
		}
	}
	/* make room in history buffer for new event pointer */
	memmove(data->history_buff[whichBuff]+1, data->history_buff[whichBuff], FUTURE_SIZE * sizeof(void*));
	
	/* add new pointer to start of history buffer */
	data->history_buff[whichBuff][0] = event_buff;
	
#ifdef DEBUG_MEMORY
	static time_t next_time = 0;
	time_t this_time = time(NULL);
	if (this_time >= next_time) {
		next_time = this_time + 10;
		int num_allocated = 0;
		for (int i=0; i<=HISTORY_FUTURE; ++i) {
			int num = (i==HISTORY_FUTURE ? FUTURE_SIZE : HISTORY_SIZE);
			for (int j=0; j<num; ++j) {
				event_buff = data->history_buff[i][j];
				if (event_buff) {
					// only count events in viewed history if not duplicated in 'all' history
					if (i!=HISTORY_VIEWED || event_buff->usage_count==1) {
						++num_allocated;
					}
				}
			}
		}
		Printf("%d allocated.  %d accounted%s\n", alloc_count, num_allocated,
				alloc_count == num_allocated ? "." : "!!!!!!" );
	}
#endif
}

// attach to an event to prevent it from being deleted
void attachHistoryEntry(HistoryEntry *event_buff)
{
    if (event_buff) ++event_buff->usage_count;
}

// release an event, allowing it to be deleted if nobody else is using it
void releaseHistoryEntry(HistoryEntry *event_buff)
{
    if (event_buff && --event_buff->usage_count==0) {
        event_buff->filename.Destruct();	// must destruct the PMultiString object
        free(event_buff);
#ifdef DEBUG_MEMORY
        --alloc_count;
#endif
    }
}

// clear the 'all' and 'future' history buffers
void clearHistoryAll(ImageData *data)
{
	int				num, whichBuff;
	
	// send message indicating the history will be cleared
	sendMessage(data, kMessageHistoryWillClear);
	
	// reset last history entry
	data->last_non_orphan_history_entry = NULL;
	
	// free the future event messages first
	for (num=0; num<data->history_size[HISTORY_FUTURE]; ++num) {
		if (data->future_msg[num]) {
			delete [] data->future_msg[num];
			data->future_msg[num] = NULL;
		}
	}
	// clear 'future' and 'all' history
	for (whichBuff=HISTORY_FUTURE; ; ) {
		for (num=0; num<data->history_size[whichBuff]; ++num) {
			releaseHistoryEntry(data->history_buff[whichBuff][num]);
			data->history_buff[whichBuff][num] = 0;
		}
		data->history_size[whichBuff] = 0;
		if (whichBuff == HISTORY_FUTURE) {
			whichBuff = HISTORY_ALL;
		} else {
			break;
		}
	}
}

// clear the 'viewed' and 'all' history buffers
void clearHistory(ImageData *data)
{
	int				num, whichBuff;
	
	// reset last history entry
	data->last_non_orphan_history_entry = NULL;
	
	for (whichBuff=HISTORY_VIEWED; ; ) {
		for (num=0; num<data->history_size[whichBuff]; ++num) {
			releaseHistoryEntry(data->history_buff[whichBuff][num]);
			data->history_buff[whichBuff][num] = 0;
		}
		data->history_size[whichBuff] = 0;
		if (whichBuff == HISTORY_VIEWED) {
			whichBuff = HISTORY_ALL;
		} else {
			break;
		}
	}
}

/*
** show event from history
*/             
int showHistory(ImageData *data, int incr)
{
	HistoryEntry	*event_buff;
	int				hnum = data->history_evt + incr;
	
	/* we don't keep a history of CMOS rates */
	if (data->wDataType == IDM_CMOS_RATES) return(0);
	
	if (hnum >= 0) {
		if (hnum >= data->history_size[data->history_all]) {
			hnum = data->history_size[data->history_all] - 1;
		}
		if (hnum < 0) hnum = 0;	/* must do this in case 'all' buffer is empty */
	} else if (data->history_all) {
		// view future events if going forward in 'all' history
		if (-hnum > data->history_size[HISTORY_FUTURE]) {
			hnum = -data->history_size[HISTORY_FUTURE];
		}
		if (hnum<0 && data->history_buff[HISTORY_FUTURE][-1-hnum]==NULL) {
			++hnum;	// don't try to view NULL entry in future buffer (eof marker)
		}
	} else {
		hnum = 0;
	}
	
	if (incr && data->history_evt==hnum) {
		Printf("At %s of %s history\x07\n",
				incr>0 ? "start" : "end",
				data->history_all ? "uncut" : "cut");
		return(0);	// do nothing if an increment was unsuccessful
	}
	if (data->history_evt != hnum) {
		data->history_evt = hnum;
		sendMessage(data, kMessageHistoryEventChanged);
		data->was_history = 1;
	}
		
	if (hnum >= 0) {
		// show event from 'all' or 'viewed' history buffer
		event_buff = data->history_buff[data->history_all][hnum];
	} else {
		// show event from 'future' history buffer (indicated by negative history number)
		event_buff = data->history_buff[HISTORY_FUTURE][-1-hnum];
	}
	
	if (event_buff) {
		showEvent(data, event_buff);
		return(1);	// event was shown OK
	}
	return(0);
}

void usleep_ph(unsigned long usec)
{
#ifdef USE_NANOSLEEP
	struct timespec ts;
	ts.tv_sec = usec / 1000000UL;
	ts.tv_nsec = (usec - ts.tv_sec * 1000000UL) * 1000;
	nanosleep(&ts,NULL);
#else
	usleep(usec);
#endif
}


/* filter event (return true if it should be displayed) */
int xsnoed_filter(ImageData *data, aPmtEventRecord *pmtRecord)
{
	// add to 'future' history if we are throwing out data
	if (data->throw_out_data) {
		addToHistory(data, pmtRecord, HISTORY_FUTURE);
		
		return(0);
	}
	
	addToHistory(data, pmtRecord, HISTORY_ALL);	// add to 'all' history
	
	/* return zero if trigger not armed */
	if (!data->trigger_flag) return(0);
	
	return(checkEvent(data, pmtRecord));
}

/* return true if event should be displayed */
int checkEvent(ImageData *data, aPmtEventRecord *pmtRecord)
{
	const short		kFirstBit = NHIT_PMT_NORMAL;	// PMT_NORMAL is the lowest bit
	const short		kLastBit = NHIT_LAST_USED;      // last used NHIT bit
	int				i, j, n, flags;
	int				pmt_counts[kLastBit + 1];       // count types of PMT's hit
	int				satisfy_nhit[kMaxNhitCuts];
	int             logic_num;
	unsigned int	cr, sl, ch;
	u_int32 		trig_word, trigger_bitmask, mask;
	u_int32			*thePmtHits;
	int             is_pmt_ncd_logic;
	int             count_logic[kPmtNcdLogicMax];
	
	// initialize local PMT/NCD logic variables
	if (data->pmt_logic_num || data->ncd_logic_num) {
	    is_pmt_ncd_logic = 1;
	    memset(count_logic, 0, sizeof(count_logic));
	} else {
	    is_pmt_ncd_logic = 0;
	}
/*
** check trigger word
*/
	if ((trigger_bitmask = data->trigger_bitmask) != 0) {
		/* get trigger word for this event */
		trig_word = PEventInfoWindow::GetTriggerWord(pmtRecord);
		/* return success if it matches our 'always show' trigger mask */
		if (trig_word & data->trigger_bits_always) return(1);
		/* remove 'always' bits from trigger criterion */
		if ((trigger_bitmask &= ~data->trigger_bits_always) != 0) {
			/* toggle trigger word bits that we require to be off */
			trig_word ^= data->trigger_bits_off;
			/* check for bits that must be on */
			mask = data->trigger_bits_on | data->trigger_bits_off;
			if ((trig_word & mask) != mask) return(0);
			/* check for don't care bits */
			if (!(trig_word & trigger_bitmask)) return(0);
		}
	}
	memset(pmt_counts, 0, sizeof(pmt_counts));  // initialize the counters
	loadDatabases(data, pmtRecord->RunNumber);  // be sure the proper databases are loaded
/* 
** check nhit criterion
*/
	// count PMT types if necessary
	logic_num = data->pmt_logic_num;
	if ((data->nhit_pmt_mask & NHIT_PMT_MASK) || logic_num) {
	
		/* get pointer to PMT data */
		thePmtHits = (u_int32 *)(pmtRecord + 1);
		
		for (i=0; i<pmtRecord->NPmtHit; ++i,thePmtHits+=3) {
			
			cr = UNPK_CRATE_ID(thePmtHits);
			sl = UNPK_BOARD_ID(thePmtHits);
			ch = UNPK_CHANNEL_ID(thePmtHits);

			if (cr>=NUM_SNO_CRATES || sl>=NUM_CRATE_CARDS || ch>=NUM_CARD_CHANNELS) {
				// skip this hit
				continue;
			}
			n = (cr * NUM_CRATE_CARDS + sl) * NUM_CARD_CHANNELS + ch;
			
			flags = data->tube_coordinates[n].status;
			
			// count number of each tube type
			for (j=kFirstBit; j<=kLastBit; ++j) {
				if (flags & (1 << j)) ++pmt_counts[j];
			}
			// count our PMT logic matches if necessary
			if (logic_num) {
			    for (j=0; j<logic_num; ++j) {
			        if (n == data->pmt_logic_array[j].num) {
			            ++count_logic[data->pmt_logic_array[j].logic];
			        }
			    }
			}
		}
	}
#ifndef SNOPLUS
	// count NCD types if necessary
	int  ncd_counts[kMaxNCDs + 1];       // count individual NCD's hit
	logic_num = data->ncd_logic_num;
	if ((data->nhit_pmt_mask & NHIT_NCD_MASK) || logic_num) {
	    memset(ncd_counts,0,sizeof(ncd_counts));
        u_int32	*sub_header = &pmtRecord->CalPckType;
        while (*sub_header & SUB_NOT_LAST) {
            sub_header += (*sub_header & SUB_LENGTH_MASK);
            if ((*sub_header >> SUB_TYPE_BITNUM) == SUB_TYPE_NCD) {
                // get pointer to ncd sub-field header
                u_int32 *ncd = sub_header;
                // get pointer to end of the NCD record
                u_int32 *ncd_end = sub_header + (*sub_header & SUB_LENGTH_MASK);
                // increment ncd pointer to next record
                int unknown_count = 0;
                const short kMaxUnknownRecords  = 20;
                while (++ncd < ncd_end) {
                    switch (*ncd & kNcdDataTypeMask) {
                        case kShaperRecordType: {
                            int card = (*ncd & kShaperCard) >> 16;
                            int chan = (*ncd & kShaperChan) >> 12;
                            n = data->ncdShaperLookup[card][chan];
                            ++ncd_counts[n];
                            ++pmt_counts[NHIT_NCD_SHAPER];
                            // count our NCD logic matches if necessary
                            if (logic_num) {
                                for (j=0; j<logic_num; ++j) {
                                    if (n == data->ncd_logic_array[j].num) {
                                        ++count_logic[data->ncd_logic_array[j].logic];
                                    }
                                }
                            }
                        }   break;
                        case kMuxRecordType: {
                            int mask = (*ncd & kMuxHitPattern);
                            int card = (*ncd & kMuxBox) >> 23;
                            for (int chan=0; chan<(int)kNumMuxChannels; ++chan) {
                                if (mask & (1 << chan)) {
                                    n = data->ncdMuxLookup[card][chan];
                                    ++ncd_counts[n];
                                    ++pmt_counts[NHIT_NCD_MUX];
                                    // count our NCD logic matches if necessary
                                    if (logic_num) {
                                        for (j=0; j<logic_num; ++j) {
                                            if (n == data->ncd_logic_array[j].num) {
                                                ++count_logic[data->ncd_logic_array[j].logic];
                                            }
                                        }
                                    }
                                }
                            }
                        }   break;
                        case kScopeRecordType:
                            ++pmt_counts[NHIT_NCD_SCOPE];
                            ncd += (*ncd & kScopeSize) / sizeof(u_int32);   // scope data
                            break;
                        case kMuxGlobalStatusType:
                        case kSKIPRecordType:
                        case kGTIDRecordType:
                        case kScopeGTIDType:
                        case kHVType:
                            break;
                        case kTrigTimeRecordType:
                        case kScopeTimeRecordType:
//                        case kShGScalRecordType:
                            ++ncd;      // 2 words long
                            break;
                        case kNewRunRecordType:
                            ncd += 2;   // 3 words long
                            break;
                        case kGeneralDataType:
                            ncd += (*ncd & 0xffff) / sizeof(u_int32);   // variable length
                            ++pmt_counts[NHIT_NCD_GENERAL];
                            break;
                        default:
                            ++unknown_count;
                            break;
                    }
                    if (unknown_count > kMaxUnknownRecords) {
                        printf("Too many unknown NCD records!\n");
                        break;
                    }
                }
            }
        }
	}
#endif // SNOPLUS

    // handle our PMT/NCD logic
    if (is_pmt_ncd_logic) {
	    if (count_logic[kPmtNcd_Always]) return(1); // always display these
        // sum our required logic counts
        int req_logic[kPmtNcdLogicMax];
        memset(req_logic, 0, sizeof(req_logic));
        int req_count = 0;
        for (i=0; i<data->pmt_logic_num; ++i) {
            ++req_logic[data->pmt_logic_array[i].logic];
            ++req_count;
        }
        for (i=0; i<data->ncd_logic_num; ++i) {
            ++req_logic[data->ncd_logic_array[i].logic];
            ++req_count;
        }
        if (req_count > req_logic[kPmtNcd_Always]) {
    	    if (count_logic[kPmtNcd_Refuse]) return(0); // never display these
            // do we have all needed PMT/NCD's?
            if (count_logic[kPmtNcd_Need] < req_logic[kPmtNcd_Need]) return(0);
            // no match if we have nothing that is interesting
            if (!(count_logic[kPmtNcd_Want] || req_logic[kPmtNcd_Need] || req_logic[kPmtNcd_Refuse])) return(0);
        }
    }
	
	// fill in bit count entry for 'any' PMT type
	pmt_counts[0] = pmtRecord->NPmtHit;

	// check NHIT logic
	NhitLogic *logicPt = data->nhit_logic_array;
	for (i=0; i<data->nhit_logic_num; ++i,++logicPt) {
		switch (logicPt->op) {
			case NHIT_EQUAL_TO:
				if (pmt_counts[logicPt->type] == logicPt->nhit) {
					satisfy_nhit[i] = 1;
					continue;
				}
				break;
			case NHIT_GREATER_THAN:
				if (pmt_counts[logicPt->type] > logicPt->nhit) {
					satisfy_nhit[i] = 1;
					continue;
				}
				break;
			case NHIT_GT_OR_EQ:
				if (pmt_counts[logicPt->type] >= logicPt->nhit) {
					satisfy_nhit[i] = 1;
					continue;
				}
				break;
			case NHIT_LESS_THAN:
				if (pmt_counts[logicPt->type] < logicPt->nhit) {
					satisfy_nhit[i] = 1;
					continue;
				}
				break;
			case NHIT_LT_OR_EQ:
				if (pmt_counts[logicPt->type] <= logicPt->nhit) {
					satisfy_nhit[i] = 1;
					continue;
				}
				break;
			case NHIT_NOT_EQUAL_TO:
				if (pmt_counts[logicPt->type] != logicPt->nhit) {
					satisfy_nhit[i] = 1;
					continue;
				}
				break;
			case NHIT_BAD_OP:
				break;
		}
		// we fail if this is not part of a logical 'or'
		// (in which case the 'or' pointer points to itself
		if (logicPt->or_index == i) return(0);	// failed cut
		
		// we failed this test... was it part of a logical 'or' ?
		int	or_index = logicPt->or_index;
		
		// have we finished all parts of this 'or' ?
		if (or_index < i) {
			// yes: were any of the other parts satisfied?
			for (;;) {
				if (satisfy_nhit[or_index]) {
					// passed another part of the 'or' so we are OK
					break;
				}
				// step to next index
				or_index = data->nhit_logic_array[or_index].or_index;
				// failed cut if no more parts of this 'or'
				if (or_index >= i) return(0);	// failed cut
			}
		}
		// set the 'failed' condition for this test
		satisfy_nhit[i] = 0;
	}
	
	return(1);	// passes all cuts
}

/* main XSnoed event display call */
/* - records the event into the history buffer */
/* Note: this must be called after xsnoed_filter or addToHistory(x,x,HISTORY_ALL) */
/*       or replaceInHistory() unless pmtRecord is null */
int xsnoed_event(ImageData *data, aPmtEventRecord *pmtRecord)
{
	HistoryEntry *event_buff;

	addToHistory(data, pmtRecord, HISTORY_VIEWED);	// add to viewed history
	
	/* reset history event since we are showing a new one now */
	if (data->was_history) {
		PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
		if (pe_win) {
			pe_win->UpdateHistoryLabel(0);
		}
#ifdef XSNOMAN
		PSnomanWindow *sm_win = (PSnomanWindow *)data->mWindow[SNOMAN_WINDOW];
		if (sm_win) {
			sm_win->UpdateHistory(0);
		}
#endif		
		data->was_history = 0;
		data->history_evt = 0;
	}
	
	if (pmtRecord) {
		// display event from history (in case the caller deletes the incoming pmtRecord)
		event_buff = data->history_buff[HISTORY_VIEWED][0];
	} else {
		event_buff = NULL;
	}
	return(showEvent(data, event_buff));
}

// replace currently viewed event in history buffer and show new event
// (pmtRecord must not be NULL)
int xsnoed_replace(ImageData *data, aPmtEventRecord *pmtRecord)
{
	HistoryEntry	*event_buff;
	HistoryEntry	*old_entry = getCurrentHistoryEntry(data);
	
	if (old_entry) {
	
		/* create new buffer for event */
		u_int32 event_size = sizeof(aPmtEventRecord) + 12 * pmtRecord->NPmtHit;
		
		/* make room for sub-headers */
		u_int32	*sub_header = &pmtRecord->CalPckType;
		while (*sub_header & SUB_NOT_LAST) {
			sub_header += (*sub_header & SUB_LENGTH_MASK);
			event_size += (*sub_header & SUB_LENGTH_MASK) * sizeof(u_int32);
		}
		/* round up to nearest event int32 */
		event_size = (event_size + (sizeof(int32)-1)) & ~(sizeof(int32)-1);
		/* allocate room for event plus HistoryEntry structure */
		event_buff = (HistoryEntry *)malloc(sizeof(HistoryEntry) + event_size);
		
		if (!event_buff) {
			Printf("Out of memory for event buffer!\n");
			return(0);
		}
		/* copy the original history entry */
		memcpy(event_buff, old_entry, sizeof(HistoryEntry));
		/* copy event into buffer */
		memcpy(event_buff+1, pmtRecord, event_size);
	
		/* replace all pointers to the old entry */
		if (data->last_non_orphan_history_entry == old_entry) {
			data->last_non_orphan_history_entry = event_buff;
		} 
		for (int whichBuff=HISTORY_VIEWED; whichBuff<=HISTORY_FUTURE; ++whichBuff) {
			for (int num=0; num<data->history_size[whichBuff]; ++num) {
				if (data->history_buff[whichBuff][num] == old_entry) {
					data->history_buff[whichBuff][num] = event_buff;
				}
			}
		}
		if (data->sum_event == old_entry) {
		    data->sum_event = event_buff;
		    data->sum_ncdData = NULL;   // just in case
#ifdef SNOPLUS
            data->sum_caenData = NULL;
#endif
		}
		
		/* release the old entry memory */
		free(old_entry);
		
		/* view the new event */
		return(showEvent(data, event_buff));
	}
	return(0);	// we weren't looking at a history entry
}

// dispatchEvent - dispatch the X event (PH 03/25/00)
// - handles menu accelerator key events internally
static void dispatchEvent(XEvent *event)
{
	const int		kBuffSize = 10;
	char 			buff[kBuffSize];
	KeySym			ks;
	XComposeStatus	cs;
	static int 		altPressed = 0;
	
	switch (event->type) {
		case KeyPress:
			XLookupString(&event->xkey, buff, kBuffSize, &ks, &cs);
			switch (ks) {
				case XK_Alt_L:
					altPressed |= 0x01;
					break;
				case XK_Alt_R:
					altPressed |= 0x02;
					break;
				default:
					// handle as an accelerator if alt is pressed
					if (event->xkey.state & AnyModMask &&	// any modifier pressed?
						altPressed &&						// was it Alt?
						PWindow::sMainWindow &&				// main window initialized?
						PWindow::sMainWindow->GetMenu() &&	// main window menu initialized?
						// do the accelerator
						PWindow::sMainWindow->GetMenu()->DoAccelerator(ks))
					{
						// return now since the event was an accelerator
						// and we handled it internally
						return;
					}
					break;
			}
			break;
		case KeyRelease:
			XLookupString(&event->xkey, buff, kBuffSize, &ks, &cs);
			switch (ks) {
				case XK_Alt_L:
					altPressed &= ~0x01;
					break;
				case XK_Alt_R:
					altPressed &= ~0x02;
					break;
			}
			break;
		case ConfigureNotify:
			// check window offset at every configure event
			// - This is odd I know, but it seems to work with different X clients
			// - The problem was that some X clients will move the window after it
			//   is created.  So we monitor the configureNotify messages and capture
			//   the first one to figure out how the client places the windows.
			if (PWindow::sMainWindow &&
				event->xconfigure.window == XtWindow(PWindow::sMainWindow->GetShell()))
			{
				PWindow::sMainWindow->CheckWindowOffset(event->xconfigure.border_width);
			}
			break;
	}
	// give the event to X
	XtDispatchEvent(event);
}

void xsnoed_service_all()
{
	XEvent			theEvent;
	long 			pending;
	XtAppContext	the_app = PResourceManager::sResource.the_app;
	
	if (!the_app) return;

#ifdef OPTICAL_CAL
	Display *display = PResourceManager::sResource.display;
#endif

	while ((pending = XtAppPending(the_app)) != 0) {
		if (pending & XtIMXEvent) {
			// only call XtAppNextEvent if we have a real X event
			// because it will service timers before getting the event,
			// so will block on XtIMTimer events
			XtAppNextEvent(the_app, &theEvent);
#ifdef OPTICAL_CAL
			if (theEvent.type == PropertyNotify &&
				theEvent.xany.window==DefaultRootWindow(display) &&
				theEvent.xproperty.atom == XA_CUT_BUFFER7)
			{
				if (theEvent.xproperty.state == PropertyNewValue) {
					PResourceManager::GetOpticalParameters();
				}
			} else
#endif
			dispatchEvent(&theEvent);	// dispatch the event
		} else {
			// process a single timer (or other type of) event
			XtAppProcessEvent(the_app,pending);
		}
		// update windows now if necessary
		PWindow::HandleUpdates();
	}
}

void xsnoed_service(int service_all)
{
	XEvent			theEvent;
	XtAppContext	the_app = PResourceManager::sResource.the_app;

	if (!the_app) return;
	if (service_all || XtAppPending(the_app)) {
		XtAppNextEvent(the_app, &theEvent);
#ifdef OPTICAL_CAL
		if (theEvent.type == PropertyNotify &&
			theEvent.xany.window==DefaultRootWindow(PResourceManager::sResource.display) &&
			theEvent.xproperty.atom == XA_CUT_BUFFER7)
		{
			if (theEvent.xproperty.state == PropertyNewValue) {
				PResourceManager::GetOpticalParameters();
			}
		} else
#endif
		// dispatch the event
		dispatchEvent(&theEvent);
		
		// update windows now if necessary
		PWindow::HandleUpdates();
	}
}

/*
** xsnoed API interface to display event in generic format
** - does not record the event in the history buffer
*/
int xsnoed_event2(ImageData *data, EventInfo *evt, RconEvent *rcon, int nrcon)
{
	int				i, n, bad_hits;
	unsigned int	cr, sl, ch;
	u_int32			sum;
	static int		msg_count = 0;
	HitInfo			*hit_info, *src_hits;
	
	clearEvent(data);
	loadDatabases(data, evt->runNumber);  // load proper databases for this event
	
	/* clear nhit sum */
	memset(data->sum_nhit, 0, NUM_TOTAL_CHANNELS * sizeof(u_int32));
	data->max_sum_nhit = 0;
	data->total_sum_nhit = 0;
#ifdef OPTICAL_CAL
	data->missed_window_count = 0;
#endif

	data->event_counter++;
	data->hits.num_nodes = data->event_nhit = evt->nhit;
	data->hits.nodes     = (Node *)   XtMalloc(data->hits.num_nodes*sizeof(Node));
	data->hits.hit_info  = (HitInfo *)XtMalloc(data->hits.num_nodes*sizeof(HitInfo));
	if (!data->hits.nodes || !data->hits.hit_info) quit("Out of memory");
	
	memset(data->mtc_word, 0, 6*sizeof(u_int32));
	data->trig_word = 0;
	data->run_number = evt->runNumber;
	data->sub_run = -1;
	data->event_id  = evt->gt;
	setEventTime(data, evt->time10mhz * 1e-7 + data->sno_time_zero);
    data->mMainWindow->DoWater(0);
	data->event_num = evt->evtNumber;
	
	data->event_shown = 1;
	newTitle(data);								/* add filename to window caption bar */
	calcSunDirection(data);
	
	/* read reconstructed points */
	xsnoed_add_rcons(data, rcon, nrcon, 0);
		
	/* transfer hit info */
	bad_hits = 0;
	hit_info = data->hits.hit_info;
	src_hits = evt->hitInfo;
	for (i=0; i<data->hits.num_nodes; ++src_hits) {
		
		cr = src_hits->crate;
		sl = src_hits->card;
		ch = src_hits->channel;

		if (cr>=NUM_SNO_CRATES || sl>=NUM_CRATE_CARDS || ch>=NUM_CARD_CHANNELS) {
			if (msg_count < MAX_MSGS) {
				Printf("Error reading HitInfo data (crate %d card %d channel %d)\n",
						cr,sl,ch);
				if (++msg_count == MAX_MSGS) {
					Printf("Maximum number of error messages exceeded\n");
				}
			}
			// skip this hit
			--data->hits.num_nodes;
			++bad_hits;
			continue;
		}
		n = (cr * NUM_CRATE_CARDS + sl) * NUM_CARD_CHANNELS + ch;

		bad_hits += verify_tube_coordinates(data,n);

		/* increment hit counter and keep track of maximum hits per tube */
		if (data->max_sum_nhit < (sum = ++data->sum_nhit[n])) {
			data->max_sum_nhit = sum;
		}
		++data->total_sum_nhit;
		
		/* don't add double hits to the display node list */
		if (sum > 1) {
			--data->hits.num_nodes;
			continue;
		}
		
		/* save the hit information */
		memcpy(hit_info, src_hits, sizeof(HitInfo));
		hit_info->index = n;
		hit_info->nhit = sum;

		data->hits.nodes[i].x3 = data->tube_coordinates[n].x;
		data->hits.nodes[i].y3 = data->tube_coordinates[n].y;
		data->hits.nodes[i].z3 = data->tube_coordinates[n].z;
		
		++i;
		++hit_info;
	}
	/* must re-run through events and update hit counts */
	/* if we had any double hits (i.e. max_sum_nhit is > 1) */
	if (data->max_sum_nhit > 1) {
		hit_info = data->hits.hit_info;
		for (i=0; i<data->hits.num_nodes; ++i,++hit_info) {
			hit_info->nhit = data->sum_nhit[hit_info->index];
		}
	}
	if (bad_hits) {
		if (msg_count < MAX_MSGS) {
			Printf("%d bad hits in event %ld\n", bad_hits, data->event_id);
			if (++msg_count == MAX_MSGS) {
				Printf("Maximum number of error messages exceeded\n");
			}
		}
	}
	
	/* calculate the hit colour indices */
	calcHitVals(data);
/*
** update the displays
*/
	if (data->auto_vertex && data->nrcon) {
		sendMessage(data, kMessageSetToVertex);
	} else if (data->auto_sun) {
		sendMessage(data, kMessageSetToSun);
	}

	sendMessage(data, kMessageNewEvent);
	
	return(1);
}

/* clear event display */
void xsnoed_clear(ImageData *data)
{
	clearSum(data);
	clearEvent(data);
#ifdef SNOPLUS
    freeCaenData(data);
#endif
	data->mMainWindow->DoWater(0);
//	clearHistory(data);
	calcSunDirection(data);
	newTitle(data);
	sendMessage(data, kMessageNewEvent);
}

/* put crate rates information into xsnoed data */
void xsnoed_rates(ImageData *data, int crate, int32 *crate_rates)
{
	int		i;
	int32	*rate_pt = data->cmos_rates + crate * NUM_CRATE_CHANNELS;
	int32	theRate;
	
	for (i=0; i<NUM_CRATE_CHANNELS; ++i) {
		theRate = *(crate_rates++);
		if (theRate >= 0) *rate_pt = theRate;
		++rate_pt;
	}
	/* set flag indicating that our data has changed */
	data->sum_changed = 1;
}

void setLabel(ImageData *data, int on)
{
	if (data->show_label != on) {
		data->show_label = on;
		if (on) {
			data->mMainWindow->LabelFormatChanged();
		} else {
			sendMessage(data, kMessageLabelChanged);
		}
		// also send a show-label-changed message
		sendMessage(data, kMessageShowLabelChanged);
	}
}

/*-------------------------------------------------------------------------------
*/
/*
** Calculate intersection with sphere of specified radius
**
** Inputs:	pos - position of point
**			dir - UNIT direction vector
*/
void hitSphere(Vector3 pos,Vector3 dir,Node *out,float radius)
{
	float		x,y,z,dx,dy,dz,t;
	float		ct,st,r,r2;

	x  = pos[0];
	y  = pos[1];
	z  = pos[2];
	dx = dir[0];
	dy = dir[1];
	dz = dir[2];
	r  = sqrt(dy*dy + dz*dz);
	r2 = radius * radius;
/*
** Rotate about x axis to make dy=0
** After this rotation, (dx,dy,dz) = (dx0,0,sqrt(1-dx0*dx0)).
*/
	if (r) {
		ct = dz/r;
		st = dy/r;
		dz = dy*st + dz*ct;
		t  =  y*ct -  z*st;
		z  =  y*st +  z*ct;
		y  = t;
/*
** Rotate about y axis to make dx = 0
** After this rotation, (dx,dy,dz) = (0,0,1)
** (Note that ct=dz and st=dx)
*/
		t = x*dz -  z*dx;
		z = x*dx +  z*dz;
		x = t;
	}
/*
** Now x,y represent the intersection point on the x,y plane of the rotated frame.
** Points with z = +-sqrt(r2-x*x-y*y) are on the surface of the sphere.
** No intersection if ((x*x+y*y)>r2), otherwise the point of exit
** has z = sqrt(r2 - (x*x+y*y)).
*/
	t = r2 - (x*x + y*y);
/*
** Calculate distance to intersection point.
** Note that z must be positive, and z(intersection) = sqrt(t).
** So distance to intersection point = sqrt(t) - z.
*/
	if (t >= 0) {
		r = sqrt(t) - z;
		if (r < 0) {
			r = 10;
			out->flags |= NODE_MISSED;
		} else {
			out->flags &= ~NODE_MISSED;
		}
	} else {
		r = 10;
		out->flags |= NODE_MISSED;
	}

	out->x3 = pos[0] + r * dir[0];
	out->y3 = pos[1] + r * dir[1];
	out->z3 = pos[2] + r * dir[2];
}

/*-------------------------------------------------------------------------------
*/
void newTitle(ImageData *data)
{
	char	buff[200], buf2[10], buf3[32];
	char	*pt;
	char	*str = data->mDispFile.GetString();
	
	if (data->event_shown) {
		if (str[0]) {
			pt = strrchr(str,'/');
			if (pt) str = pt+1;						/* get rid of path from filename */
		} else {
			sprintf(buf3, "Run %ld", data->run_number);
			str = buf3;
		}
	} else {
		str = (char *)0;
	}
	pt = data->mMainWindow->GetTitle();
	
	strncpy(buff,pt,150);
	pt = strchr(buff,'[');						/* find start of filename */
	if (pt > buff+1) --pt;	
	else pt = buff + strlen(buff);				/* pt points to end of title */
/*
** add filename and event number to title
*/
	if (str) {
		if (data->curcon >= 0) sprintf(buf2,".%d",data->curcon);
		else buf2[0] = 0;
		sprintf(pt,data->hex_id ? " [%s:0x%lx%s]" : " [%s:%ld%s]",str,data->event_id,buf2);
	} else *pt = 0;
		
	data->mMainWindow->SetTitle(buff);		/* set new title */
 
	PEventControlWindow::UpdateEventNumber(data);
}

/*---------------------------------------------------------------------------------
** Routines to initialize data structure and read database files
*/
static char *delim = " \n\r\t";
static char *bad_fmt = "Bad format tube coordinate file";

static void readTubeCoordinates(ImageData *data, int index)
{
	char	*pt;
	float	x,y,z,r,d;
	int		cr,sl,ch,t;
	
	if (!(pt = strtok(NULL,delim))) quit(bad_fmt);
	// these ::'s are necessary to resolve overloading ambiguity
	// between atof() and std::atof() when compiling on surf - PH 03/04/02
	x = ::atof(pt);
	if (!(pt = strtok(NULL,delim))) quit(bad_fmt);
	y = ::atof(pt);
	if (!(pt = strtok(NULL,delim))) quit(bad_fmt);
	z = ::atof(pt);
	r = vectorLen(x,y,z);
/*
cr = index / (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
t = index - cr * (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
sl = t / NUM_CARD_CHANNELS;
ch = t - sl * NUM_CARD_CHANNELS;
Printf("%2d %2d %2d (%.1f %.1f %.1f) %.1f\n",cr,sl,ch,x,y,z,r);
*/
	if (!(data->tube_coordinates[index].status & PMT_NORMAL)) {
		// non-normal pmts get normalized differently!
		r = data->tube_radius;
	}
	x /= r;
	y /= r;
	z /= r;
	
	if (data->tube_coordinates[index].status & PMT_OK) {
		d = vectorLen(data->tube_coordinates[index].x - x,
					  data->tube_coordinates[index].y - y,
					  data->tube_coordinates[index].z - z);

		if (d > 0.01) {	/* print error if out by greater than 8.5 cm */
			cr = index / (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			t = index - cr * (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
			sl = t / NUM_CARD_CHANNELS;
			ch = t - sl * NUM_CARD_CHANNELS;
			Printf("Crate %d card %d channel %d position inconsistency!  Was (%.0f,%.0f,%.0f) Now (%.0f,%.0f,%.0f)\n",
			 		cr,sl,ch,
			 		data->tube_coordinates[index].x*data->tube_coordinates[index].r,
			 		data->tube_coordinates[index].y*data->tube_coordinates[index].r,
			 		data->tube_coordinates[index].z*data->tube_coordinates[index].r,
			 		x*r,y*r,z*r);
		}
	}
				  
	data->tube_coordinates[index].x = x;
	data->tube_coordinates[index].y = y;
	data->tube_coordinates[index].z = z;
	data->tube_coordinates[index].r = r;
	
	data->tube_coordinates[index].status |= PMT_OK;	/* indicates position was found */
}

#ifdef GENERATE_OWL_POSITIONS
// this is the routine that calculated the OWL locations for the database
void generate_owl_positions(ImageData *data)
{
	int		i;
	char	buff[512];
	Edge	*edge;
	Node	*node, *n2;
	int		ne, nn;
	double	max_cross_z, cross_z;
	float	x,y,z,dx,dy,dz,len,x0=0,y0=0,z0=0,xt,yt,zt,x2,y2;
	FILE	*fp1, *fp2;
	
	fp1 = fopen(DATABASE_FILE,"r");
	if (!fp1) quit("can't open " DATABASE_FILE);
	fp2 = fopen("database.out","w");
	for (i=0; i<1+751+9522; ++i) {
		fgets(buff,512,fp1);
		fputs(buff,fp2);
	}
	
	nn = data->geod.num_nodes;
	ne = data->geod.num_edges;
	for (int i=0; i<91; ++i) {
		fgets(buff,512,fp1);
		int node_num = atoi(buff+26);
		if (node_num<2 || node_num>92) quit("bad node number");
		node = data->geod.nodes + node_num - 1;
		x = node->x3;
		y = node->y3;
		z = node->z3;
		len = vectorLen(x,y,z);
		x /= len;
		y /= len;
		z /= len;
		if (node_num==48 || node_num==68 || node_num==79) {
			max_cross_z = 2;
		} else {
			max_cross_z = -2;
		}
		edge = data->geod.edges;
		for (int j=0; j<ne; ++j, ++edge) {
			if (node == edge->n1) {
				n2 = edge->n2;
			} else if (node == edge->n2) {
				n2 = edge->n1;
			} else {
				continue;
			}
			x2 = (x + n2->x3) / 2;
			y2 = (y + n2->y3) / 2;
			len = sqrt(x2*x2 + y2*y2);
			x2 /= len;
			y2 /= len;
			dx = n2->x3 - x;
			dy = n2->y3 - y;
			dz = n2->z3 - z;
			len = vectorLen(dx,dy,dz);
			dx /= len;
			dy /= len;
			dz /= len;
			cross_z = x2 * dy - y2 * dx;
//printf("%g %g %g -- %g\n",dx,dy,dz,cross_z);
			if (node_num==48 || node_num==68) {
				if (max_cross_z >= cross_z) {
					max_cross_z = cross_z;
					x0 = dx;
					y0 = dy;
					z0 = dz;
				}
			} else if (node_num == 79) {
				// south pole OWL is on southern strut
				if (max_cross_z >= dy) {
					max_cross_z = dy;
					x0 = dx;
					y0 = dy;
					z0 = dz;
				}
			} else {
				if (max_cross_z <= cross_z) {
					max_cross_z = cross_z;
					x0 = dx;
					y0 = dy;
					z0 = dz;
				}
			}
		}
		if (fabs(max_cross_z) > 1) {
			printf("node %d - cross_z = %g\n",node_num,max_cross_z);
			quit("generate owl failure");
		}
		xt = node->x3 * data->tube_radius;
		yt = node->y3 * data->tube_radius;
		zt = node->z3 * data->tube_radius;
//printf("%g %g %g\n",x,y,z);
		printf("Node %d (%.1f,%.1f,%.1f) owl (%.1f,%.1f,%.1f)\n",
				node_num, xt, yt, zt,
				xt + 50 * x0 + 35 * x,
				yt + 50 * y0 + 35 * y,
				zt + 50 * z0 + 35 * z);
		sprintf(strchr(buff,0)-1, " %6.1f %6.1f %6.1f\n",
				xt + 50 * x0 + 35 * x,
				yt + 50 * y0 + 35 * y,
				zt + 50 * z0 + 35 * z);
		fputs(buff,fp2);

	}
	while (fgets(buff,512,fp1)) {
		fputs(buff,fp2);
	}
	fclose(fp1);
	fclose(fp2);
	quit("done");
}
#endif

void initData(ImageData *data, int load_settings)
{
    int         i;
	struct tm	tms;
	char		*msg;
	
	/* create speaker object */
	data->mSpeaker = new PSpeaker;
	data->mZdabFile = new PZdabFile;
// uncomment the following line to debug ZDAB files
//	data->mZdabFile->SetVerbose(1);
	data->mDeleteData = 1;		// delete by default
#if defined(LOAD_CALIBRATION) && !defined(ROOT_FILE)
	data->pcal_simple = new PCalSimple();
#ifdef TITLES_CAL
	data->pcal_titles = new PCalTitles();
#endif
	data->pcal = data->pcal_simple;
#endif
    data->db_count = 0;
    data->db_run_num = (u_int32)-1;
    data->db_list = NULL;
    data->cur_db[kPmtDB] = (DatabaseFile *)-1;
#ifndef SNOPLUS
    data->cur_db[kNcdDB] = (DatabaseFile *)-1;
#endif
/*
** Initialize ImageData from resources
*/
	// initialize ImageData from resources
	memcpy(data, &PResourceManager::sResource, sizeof(XSnoedResource));

	// range check menu items
	if ((unsigned)data->geo > IMAX_GEO)					  data->geo = 0;
	if ((unsigned)data->move > IMAX_MOVE)			 	  data->move = 0;
	if ((unsigned)data->calibrated > IMAX_CALIBRATED)     data->calibrated = 0;
	if ((unsigned)data->dataType > IMAX_DATATYPE)		  data->dataType = 0;
	if ((unsigned)data->projType > IMAX_PROJTYPE)	   	  data->projType = 0;
	if ((unsigned)data->projCoords > IMAX_PROJCOORDS)     data->projCoords = 0;
	if ((unsigned)data->sizeOption > IMAX_SIZEOPTION)     data->sizeOption = 0;
	if ((unsigned)data->shapeOption > IMAX_SHAPEOPTION)   data->shapeOption = 0;
	if ((unsigned)data->mcTrack > IMAX_MCTRACK) 		  data->mcTrack = 0;
	if ((unsigned)data->ncd_scope_cal > IMAX_SCOPE_CAL)   data->ncd_scope_cal = 0;
	if ((unsigned)data->ncd_scope_chan > IMAX_SCOPE_CHAN) data->ncd_scope_chan = 0;
	
	// range check other necessary resources
	if ((unsigned)data->print_to >= 2)	data->print_to = 0;

	// initialize necessary ImageData elements
	/* -- deemed "confusing", so reset the following settings to zero */
	data->sum				= 0;
	data->bit_mask			= HIT_HIDDEN;
	data->auto_vertex		= 0;
	data->auto_sun			= 0;
	data->move				= 0;
	data->projCoords		= 0;

	// reset other settings if not loading settings
	if (!load_settings) {
		data->geo 			= 0;
		data->move 			= 0;
		data->calibrated 	= 0;
		data->dataType 		= 0;
		data->projType 		= 0;
		data->projCoords 	= 0;
		data->sizeOption 	= 0;
		data->shapeOption 	= 0;
		data->mcTrack 		= 0;
		data->show_vessel	= 0;
		data->rcon_lines	= 0;
		data->hit_lines		= 0;
		data->anchor_ropes  = 0;
		data->waterLevel    = 0;
		data->goto_run_num	= 0;
		data->open_windows 	= 0;
		data->open_windows2 = 0;
		data->history_cut 	= 1;
		data->hex_id 		= 0;
		data->time_zone 	= 0;
		data->angle_rad 	= 0;
		data->hit_xyz 		= 0;
		data->log_scale		= 0;
		data->hit_size		= 1;
		data->save_config	= 0;
		data->autoFit		= 0;
		data->ncd_scope_cal = 0;
		data->ncd_scope_chan= 0;
		data->ncd_scope_mask= 0;
	}
#if defined(NO_MAIN) && !defined(XSNOMAN)
	// don't open other windows if not a standalone program - PH 12/22/99
	// unless it is XSNOMAN - PH 03/25/00
	data->open_windows = 0;
	data->open_windows2 = 0;
#endif
	
	data->wGeo				= IDM_GEODESIC;		// set later
	data->wMove				= IDM_MOVE_SPHERE;	// set later
	data->wCalibrated		= IDM_UNCALIBRATED;	// set later
	data->wDataType			= IDM_TAC;			// set later
	data->wNCDType          = IDM_NCD_SHAPER_VAL; // set later
	data->wProjType			= data->projType + IDM_PROJ_RECTANGULAR;
	data->wProjCoords		= data->projCoords + IDM_DETECTOR_COORD;
	data->wSizeOption		= data->sizeOption + IDM_SIZE_FIXED;
	data->wShapeOption		= data->shapeOption + IDM_HIT_SQUARE;
	data->wMCTrack	 		= data->mcTrack + IDM_MC_ALL_TRACKS;
	data->wMCNames			= data->mcNames + IDM_HIDE_NAMES;
	data->mcFlags	 		= (1<<IM_REACHED_PMT);
	data->dispName	 		= XtMalloc(20);
	strcpy(data->dispName, "Tac");
	data->projName			= XtMalloc(20);
	strcpy(data->projName, "Rectangular");
	data->curcon	 		= -1;
	data->watercon[0]		= -1;
	data->watercon[1]		= -1;
	data->cursor_hit 		= -1;
	data->cursor_ncd 		= -1;
	data->sub_run			= -1;
	data->angle_conv 		= 180 / PI;
	data->mcHighlight		= 1;
	data->reset_sum_time    = 10;
	
	// set the trigger logic variables
	if (data->load_trigger_settings) {
		PEventControlWindow::SetNhitLogic(data, data->nhit_logic);
		PEventControlWindow::SetTriggerMaskLogic(data, data->trigger_mask);
	}
	
	// must turn on water level display through this call so necessary objects are created
	if (data->waterLevel) {
		data->waterLevel = 0;
		toggleWaterLevelDisplay(data);
	}
	// make sure that both auto_vertex and auto_sun are not enabled simultaneously
	if (data->auto_vertex && data->auto_sun) {
		data->auto_sun = 0;
	}

#ifndef NO_DISPATCH
	SetDispName(data->dispatcher);
#endif

	sFilePath = data->file_path;
	
	// copy resource strings into our buffers
	for (i=0; i<NUM_WRITERS; ++i) {
		strncpy(data->output_file[i], data->output_file_pt[i], FILELEN);
		data->output_file[i][FILELEN-1] = '\0';
	}
	for (i=0; i<2; ++i) {
		strncpy(data->print_string[i], data->print_string_pt[i], FILELEN);
		data->print_string[i][FILELEN-1] = '\0';
	}
	strncpy(data->label_format, data->label_format_pt, FORMAT_LEN);
	data->label_format[FORMAT_LEN-1] = '\0';

	tms.tm_sec = 0;
	tms.tm_min = 0;
	tms.tm_hour = 0;
	tms.tm_mday = 1;
	tms.tm_mon = 0;
#ifdef SNOPLUS
    tms.tm_year = 110;  // SNO+ time zero is Jan 1, 2010
#else
	tms.tm_year = 96;   // SNO time zero is Jan 1, 1996
#endif
	tms.tm_isdst = 0;
	data->sno_time_zero = mktime(&tms);
	/* mktime returns UTC time given local time but sno time zero is UTC */
	/* so we must do a conversion from local to GMT here (no DST because we set isdst to zero) */
#ifdef __MACHTEN__
	data->sno_time_zero -= 5 * 3600L; // patch for MachTen version
#else
	data->sno_time_zero -= timezone;
#endif

#ifdef ROOT_FILE
	tms.tm_year = 75;
	data->root_time_zero = mktime(&tms);
	// correct for offset of one day (because Jan 1, 1975 is actually Julian date = 1)
	data->root_time_zero -= 24.0 * 60.0 * 60.0;
#ifdef __MACHTEN__
	data->root_time_zero -= 5 * 3600L; // patch for MachTen version
#else
	data->root_time_zero -= timezone;
#endif
	data->rch_pmtn = -1;
#endif

	// -NT: Initialize snoman flags
	data->input_source = INPUT_NONE;
#ifdef XSNOMAN
	data->snoman_has_data = FALSE;
	data->exit_to_snoman = FALSE;
#endif // XSNOMAN
/*
** Initialize event filter parameters
*/
	data->trigger_flag = TRIGGER_OFF;
/*
** Initialize sun direction
*/
	calcSunDirection(data);
/*
** load geodesic geometry (must be done before loading tube coordinates)
*/
	msg = loadGeometry(&data->geod,IDM_GEODESIC,data->argv,data->tube_radius);
#ifdef GENERATE_OWL_POSITIONS
	generate_owl_positions(data);
#endif
	if (msg) quit(msg);	
/*
** Load PMT/NCD coordinates from files
*/
    loadDatabaseList(data);
    loadDatabases(data, 0);     // load default databases
}

// load list of PMT/NCD databases - PH 09/16/04
void loadDatabaseList(ImageData *data)
{
    FILE *fp;
    char name[FILENAME_LEN];
    char buff[512];
    
	fp = openFile(DB_LIST_FILE,"r",data->file_path);
	if (fp) {
	    strcpy(name, getOpenFileName());
	    // count the number of lines in the file
	    int lines = 0;
	    while (fgets(buff, 256, fp)) {
	        ++lines;
	    }
	    fseek(fp, 0L, 0);   // rewind the file
	    // allocate memory for the maximum number of database files
	    data->db_list = new DatabaseFile[lines];
	    data->db_count = 0;
	    char *delim = " \t\n\r";
	    for (int i=0; i<lines; ++i) {
	        if (!fgets(buff, 256, fp)) break;
	        char *pt;
	        if (!(pt = strtok(buff, delim))) {
	            Printf("Error 1 in line %d of %s\n", i+1, DB_LIST_FILE);
	            continue;
	        }
	        if (!strcmp(pt, "PMT")) {
	            data->db_list[data->db_count].type = kPmtDB;
	        } else if (!strcmp(pt, "NCD")) {
	            data->db_list[data->db_count].type = kNcdDB;
	        } else {
	            Printf("Missing type in line %d of %s\n", i+1, DB_LIST_FILE);
	            continue;
	        }
	        if (!(pt = strtok(NULL, delim)) || 
	            !(data->db_list[data->db_count].first_run=atol(pt)))
	        {
	            Printf("Missing start run number in line %d of %s\n", i+1, DB_LIST_FILE);
	            continue;
	        }
	        if (!(pt = strtok(NULL, delim)) || 
	            !(data->db_list[data->db_count].last_run=atol(pt)))
	        {
	            Printf("Missing end run number in line %d of %s\n", i+1, DB_LIST_FILE);
	            continue;
	        }
	        if (!(pt = strtok(NULL, delim))) {
	            Printf("Missing file name in line %d of %s\n", i+1, DB_LIST_FILE);
	            continue;
            }
            char *dst = data->db_list[data->db_count].filename;
            if (pt[0] != '/') {
                // set same directory as list file
                // unless an absolute directory is specified
                if (strrchr(name, '/')) {
                    strcpy(dst,name);
                    dst = strrchr(dst, '/') + 1;
                }
            }
            strcpy(dst, pt);    // add filename
            ++data->db_count;   // increment the count of DB file entries
	    }
	    fclose(fp);
	    Printf("Read database list %s with %d entries\n", name, data->db_count);
    } else {
        Printf("Database list %s not found -- using default databases\n", DB_LIST_FILE);
    }
}

// free list of PMT/NCD databases - PH 09/16/04
void freeDatabaseList(ImageData *data)
{
    delete [] data->db_list;
    data->db_count = 0;
    data->db_run_num = (u_int32)-1;
    data->db_list = NULL;
    data->cur_db[kPmtDB] = (DatabaseFile *)-1;
#ifndef SNOPLUS
    data->cur_db[kNcdDB] = (DatabaseFile *)-1;
#endif
}

// load PMT and NCD databases for specified run - PH 09/16/04
// Note: If run is zero, load default databases only if none loaded already
void loadDatabases(ImageData *data, u_int32 run_num)
{
    // nothing to do if we have already loaded the databases for this run
    if (run_num == data->db_run_num) return;
    
    DatabaseFile *new_db[kNumDBs];
    new_db[kPmtDB] = NULL;
#ifndef SNOPLUS
    new_db[kNcdDB] = NULL;
#endif

    if (run_num) {
        // find the proper databases for this run number
        DatabaseFile *last_db = data->db_list + data->db_count;
        for (DatabaseFile *db=data->db_list; db<last_db; ++db) {
            // continue if this db isn't in the run range
            if (run_num < db->first_run || run_num > db->last_run) continue;
            // continue if we already found a db of this type
            if (new_db[db->type]) continue;
            // take the first DB we find that matches the run number
            new_db[db->type] = db;          // save pointer to current DB of this type
            if (new_db[1 - db->type]) break;// all done if found both types
        }
    } else {
        // use current DB if loaded, otherwise default DB
        for (int type=0; type<kNumDBs; ++type) {
            if (data->cur_db[type] != (DatabaseFile *)-1) {
                new_db[type] = data->cur_db[type];
            }
        }
    }
    char *filename;
    if (data->cur_db[kPmtDB] != new_db[kPmtDB]) {
        // must load new PMT database file
        if (new_db[kPmtDB]) {
            filename = new_db[kPmtDB]->filename;
        } else {
            filename = DATABASE_FILE;   // default to original database file
        }
        loadPmtDatabase(data, filename);
        data->cur_db[kPmtDB] = new_db[kPmtDB];
    }
#ifndef SNOPLUS
    if (data->cur_db[kNcdDB] != new_db[kNcdDB]) {
        // must load new NCD database file
        if (new_db[kNcdDB]) {
            filename = new_db[kNcdDB]->filename;
        } else {
            filename = NCD_FILE;        // default to original database file
        }
        loadNcdDatabase(data, filename);
        data->cur_db[kNcdDB] = new_db[kNcdDB];
    }
#endif
    data->db_run_num = run_num;         // save run number of current databases
}

static void checkTube(ImageData *data, int index, int ipmt, char *type)
{
    if (data->tube_coordinates[index].status) {
        int cr = index / (16 * 32);
        int sl = (index - cr * 16 * 32) / 32;
        int ch = index - (cr * 16 + sl) * 32;
        char pmt1[16],pmt2[16];
        strcpy(pmt1,int2bc(ipmt,'P'));
        strcpy(pmt2,int2bc(data->tube_coordinates[index].tube,'P'));
        printf("WARNING DB conflict: %s tube %s overwrites %s in %d/%d/%d\n",
               type, pmt1, pmt2, cr, sl, ch);
    }
}

void loadPmtDatabase(ImageData *data, char *filename)
{
	int			nowl, nlg, npan, npmt, nbut=0, neck=0;
	int			ipan, ipmt;
	int			i,j,n,l;
	unsigned	cr,sl,ch,index;
	char		*pt, buff[512];
	FILE		*fp;
	int			got_binary = 0;
	int			create_binary = 1;
#ifdef NO_OWL_POSITIONS
	float		r;
	Node		*nod;
#endif	

	data->num_tube_coordinates = NUM_TOTAL_CHANNELS;

    if (data->remap_data) XtFree((char *)data->remap_data);
	data->remap_data = (Point3 *)XtMalloc(data->num_tube_coordinates * sizeof(Point3));
    if (data->remap_done) XtFree((char *)data->remap_done);
	data->remap_done = (char *)XtMalloc(data->num_tube_coordinates * sizeof(char));
	if (!data->remap_data || !data->remap_done) {
	  quit("Not enough memory");
	}
	memset(data->remap_done, 0, data->num_tube_coordinates * sizeof(char));

    if (data->tube_coordinates) XtFree((char *)data->tube_coordinates);
	data->tube_coordinates = (Pmt *)XtMalloc(data->num_tube_coordinates * sizeof(Pmt));
	if (!data->tube_coordinates) {
		quit("No memory for tube coordinates");
	}
	memset(data->tube_coordinates, 0, data->num_tube_coordinates * sizeof(Pmt));
	// initialize PMT barcodes to invalid number
	for (i=0; i<data->num_tube_coordinates; ++i) {
	    data->tube_coordinates[i].tube = -1;
	}

	if (!data->sum_nhit) {
	    data->sum_nhit = (u_int32 *)XtMalloc(NUM_TOTAL_CHANNELS * sizeof(u_int32));
	}
	if (!data->sum_nhit) {
		quit("No memory for NHIT sum");
	}
	
#ifdef SNOSTREAM_DATABASE

	Printf("Reading file pmt_position.dat...\n");
	fp = openFile("pmt_position.dat","r",data->file_path);
	if (!fp) quit("Can't find file: pmt_position.dat");

	count = 0;
	while (fgets(buff, 256, fp)) {
	
		/* get crate, card, channel numbers */
		if (!(pt = strtok(buff,delim))) quit(bad_fmt);
		cr = atoi(pt);
		if (cr >= NUM_SNO_CRATES) continue;
		if (!(pt = strtok(NULL,delim))) quit(bad_fmt);
		sl = atoi(pt);
		if (sl >= NUM_CRATE_CARDS) continue;
		if (!(pt = strtok(NULL,delim))) quit(bad_fmt);
		ch = atoi(pt);
		if (ch >= NUM_CARD_CHANNELS) continue;
		
		if ((3==cr)||(13==cr)||(17==cr)||(18==cr)) {
			sl=((31-sl)&017);
		}
		else sl=(16-sl);
 		ch=(32-ch);

		index = (cr * NUM_CRATE_CARDS + sl) * NUM_CARD_CHANNELS + ch;

		data->tube_coordinates[index].status = PMT_NORMAL;
		
		readTubeCoordinates(data,index);
		++count;
	}
	fclose(fp);
	Printf("%d tube positions read\n",count);
#endif // SNOSTREAM_DATABASE

#ifdef NICKS_DATABASE
	Printf("Reading file snoman_pmt.dat...\n");
	fp = openFile("snoman_pmt.dat","r",data->file_path);
	if (!fp) quit("Can't find file: snoman_pmt.dat");

	count = 0;
	while (fgets(buff, 256, fp)) {
		if (!(pt = strtok(buff,delim))) quit(bad_fmt);
		index = atoi(pt);
		if (!(pt = strtok(NULL,delim))) quit(bad_fmt);
		n = atoi(pt);
		if (n < 0) continue;	/* -999 means no mapping */
		if (index >= NUM_TOTAL_CHANNELS) quit(bad_fmt);
		if (n <= 9599) {
			data->tube_coordinates[index].status = PMT_NORMAL;
		} else if (n <= 9699) {
			data->tube_coordinates[index].status = PMT_OWL;
		} else if (n <= 9799) {
			data->tube_coordinates[index].status = PMT_LOW_GAIN;
		} else {
			continue;
		}
		readTubeCoordinates(data,index);
		++count;
	}
	fclose(fp);
	Printf("%d tube positions read\n",count);
#endif // NICKS_DATABASE

#ifdef PETERS_DATABASE

/* this code is a pile of poo, and needs to be re-written */

	//** for parsing Peter's file DATABASE.DAT
  fp = openAltFile( filename,"rb",data->file_path,".bin");
  
//  const long kDatabaseMagicNumber = 0x237ad2fa;	... number for files up to version 3.8.3
  const long kDatabaseMagicNumber = 0x237ad2fc;
  
  struct {
  	long magic_number;
  	long date;
  	long time;
  } database_header;
  
  if (fp) {
  	if (fread(&database_header, sizeof(database_header), 1, fp) == 1 &&
  		database_header.magic_number == kDatabaseMagicNumber)
  	{
  		unsigned num = fread(data->tube_coordinates,sizeof(Pmt),data->num_tube_coordinates,fp);
	  	if (num == (unsigned)data->num_tube_coordinates) {
			got_binary = 1;
			Printf("Loaded PMT database %s\n",getOpenFileName());
	  	} else {
	  		Printf("Error reading binary database file %s\n", getOpenFileName());
            // must reset tube data to get rid of any garbage
            memset(data->tube_coordinates, 0, data->num_tube_coordinates * sizeof(Pmt));
            // initialize PMT barcodes to invalid number
            for (i=0; i<data->num_tube_coordinates; ++i) {
                data->tube_coordinates[i].tube = -1;
            }
	  	}
	} else {
		Printf("Invalid format binary database file %s\n", getOpenFileName());
	}
	fclose(fp);
	if (!got_binary) {
		create_binary = 0;	// don't overwrite invalid format bad binary file
		Printf("--> Will load ASCII version of database file\n");
	}
  }
  if (!got_binary) {
	fp = openFile( filename, "r",data->file_path );
	if( !fp ) {
	    sprintf(buff,"Can't find file: %s", filename);
	    quit(buff);
	}
	Printf("Reading file %s...\n", filename);
	
	fgets( buff, 256, fp );
	pt = strstr( buff, "DATE=" );
	database_header.date = atol( pt + 5 );
	pt = strstr( buff, "TIME=" );
	database_header.time = atoi( pt + 5 ) * 3600L;	// hours
	pt = strchr( pt, ':' );
	database_header.time += atoi( pt + 1 ) * 60;	// minutes
	pt = strchr( pt+1, ':' );
	database_header.time += atoi( pt + 1 );			// seconds
	database_header.magic_number = kDatabaseMagicNumber;
	pt = strstr( buff, "NPAN=" );
	npan = atoi( pt + 5 );        // may want to make these global ...
	pt = strstr( buff, "NPMT=" );
	npmt = atoi( pt + 5 );
	pt = strstr( buff, "NOWL=" );
	nowl = atoi( pt + 5 );
	pt = strstr( buff, "NLG=" );
	nlg = atoi( pt + 4 );
	pt = strstr( buff, "NBUT=" );
	if (pt) nbut = atoi( pt + 5);
	pt = strstr( buff, "NECK=" );
	if (pt) neck = atoi( pt + 5);

	for( l=0; l<npan; ++l ) {
		fgets(buff, 256, fp);
		pt = strtok(buff, delim);
		if (!pt) quit(bad_fmt);
		if (*pt == 'H') {
			ipan = atoi( pt + 1 );
			pt = strtok(NULL, delim);
			if (!pt) quit(bad_fmt);
			n = atoi(pt);
			for (i=0; i<n; ++i) {
				if (!fgets(buff, 256, fp)) quit(bad_fmt);
				if( strstr( buff, "---- ---- ---- ---- 0x10" ) ) continue; //Skip missing PMTs
				pt = strtok(buff,delim);    // skip cable ID
				pt = strtok( NULL, delim ); // get PMT ID
				ipmt = bc2int( pt, 'P' );
				for (j=0; j<3; ++j) pt = strtok(NULL, delim);
				if (!pt) quit(bad_fmt);
				cr = pt[0] - 'A';
				sl = 15 - pt[1] + 'A';  //Normal crates
				if( cr == 3 || cr == 13 || cr > 16 ) {
				  if( pt[1] == 'P' ) {
				    sl = 15;
				  } else {
				    --sl;
				  }
				}
				if( cr == 17 && sl == 15 ) {
				  cr = 18;  //Special case of crate R, slot P
				  sl = 0;
				}
				ch = 32 - (pt[2] - '0') * 10 - (pt[3] - '0');

				if (cr<20 && sl<16 && ch<32) {
					index = (cr * 16 + sl) * 32 + ch;
					pt = strtok( NULL, delim );
					pt += 2;
		            checkTube(data,index,ipmt,"PMT");
					data->tube_coordinates[index].status = PMT_NORMAL |
						( ( ( *pt - '0' ) * 0x10 + ( *(pt + 1 ) - '0' ) ) & 0xf8 );
					data->tube_coordinates[index].panel = ipan;
					data->tube_coordinates[index].cell = i+1;
					data->tube_coordinates[index].tube = ipmt;

					readTubeCoordinates(data,index); //PMT_OK flag set here
				}
			}
		}
	}
	/* add OWL tubes */
	for( l=0; l<nowl; ++l ) {
		fgets( buff, 256, fp );
		strtok( buff, delim );  /* skip cable ID */
		pt = strtok( NULL, delim );	/* get PMT ID */
		ipmt = bc2int( pt, 'P' );
		for( j=0; j<3; ++j ) pt = strtok( NULL, delim );	/* get electronics channel */
		cr = pt[0] - 'A';
		sl = 15 - pt[1] + 'A';  //Normal crates
		if( cr == 3 || cr == 13 || cr > 16 ) {
	  		if( pt[1] == 'P' ) {
				sl = 15;
	    	} else {
	      		--sl;
	    	}
		}
		if( cr == 17 && sl == 15 ) {
			cr = 18;  //Special case of crate R, slot P
			sl = 0;
	  	}
	  	ch = 32 - (pt[2] - '0') * 10 - (pt[3] - '0');
	  	
	  	/* get node number */
	  	pt = strtok(NULL, delim);
	  	if (!pt || *pt!='N') quit(bad_fmt);
	  	n = atoi(pt+1);

		if (cr<20 && sl<16 && ch<32 && n>1 && n<93) {  //Node numbers run from 2 to 92
	  		index = ( cr * 16 + sl ) * 32 + ch;
		    checkTube(data,index,ipmt,"OWL");
	  		/* geodesic geometry must be loaded BEFORE this file */
	  		/* because we use the geodesic node location for the OWL tube coordinate here */
			pt = strtok( NULL, delim );
			pt += 2;
			data->tube_coordinates[index].tube = ipmt;
			data->tube_coordinates[index].panel = n; //Put node number here 
			data->tube_coordinates[index].cell = 0;
#ifdef NO_OWL_POSITIONS
			nod = data->geod.nodes + n - 1;	// must subtract one for node index in array
			r = vectorLen(nod->x3,nod->y3,nod->z3);
			data->tube_coordinates[index].status = PMT_OK |
				( ( ( *pt - '0' ) * 0x10 + ( *(pt + 1 ) - '0' ) ) & 0xf8 );
			data->tube_coordinates[index].x = nod->x3 / r;
			data->tube_coordinates[index].y = nod->y3 / r;
			data->tube_coordinates[index].z = nod->z3 / r;
			data->tube_coordinates[index].r = data->geod.radius * data->tube_radius;
#else
			data->tube_coordinates[index].status = 
				( ( ( *pt - '0' ) * 0x10 + ( *(pt + 1 ) - '0' ) ) & 0xf8 );
			readTubeCoordinates(data, index);
#endif
	  	} else {
			quit( bad_fmt );
	  	}
	}
	/* add low gain tubes */
	for( l=0; l<nlg; ++l ) {
	  fgets( buff, 256, fp );
	  pt = strtok( buff, delim );
	  pt = strtok( NULL, delim );
	  ipmt = bc2int( pt, 'P' );
	  for( j=0; j<3; ++j ) pt = strtok( NULL, delim );
	  cr = pt[0] - 'A';
	  sl = 15 - pt[1] + 'A';  //Normal crates
	  if( cr == 3 || cr == 13 || cr > 16 ) {
	    if( pt[1] == 'P' ) {
	      sl = 15;
	    } else {
	      --sl;
	    }
	  }
	  if( cr == 17 && sl == 15 ) {
	    cr = 18;  //Special case of crate R, slot P
	    sl = 0;
	  }
	  ch = 32 - (pt[2] - '0') * 10 - (pt[3] - '0');

	  index = ( cr * 16 + sl ) * 32 + ch;
	  for( i=0; i<NUM_TOTAL_CHANNELS; ++i ) {
	    if( ipmt == data->tube_coordinates[i].tube ) break;
	  }
	  if( i < NUM_TOTAL_CHANNELS ) {
		checkTube(data,index,ipmt,"Low gain");
        pt = strtok( NULL, delim );	// skip panel-cell number
        pt = strtok( NULL, delim );
        pt += 2;
        data->tube_coordinates[index].status = PMT_OK |
            ( ( ( *pt - '0' ) * 0x10 + ( *(pt + 1 ) - '0' ) ) & 0xf8 );
	    data->tube_coordinates[index].x = data->tube_coordinates[i].x;
	    data->tube_coordinates[index].y = data->tube_coordinates[i].y;
	    data->tube_coordinates[index].z = data->tube_coordinates[i].z;
	    data->tube_coordinates[index].r = data->tube_coordinates[i].r;
	    data->tube_coordinates[index].tube = ipmt;
	    data->tube_coordinates[index].panel = data->tube_coordinates[i].panel;
	    data->tube_coordinates[index].cell = data->tube_coordinates[i].cell;
	  } else {
	    quit( bad_fmt );
	  }
	}
	/* add BUTTS tubes */
	for (l=0; l<nbut; ++l) {
		fgets( buff, 256, fp );
		pt = strtok( buff, delim );	// read butt ID
		pt = strtok( NULL, delim );	// read tube ID
		ipmt = bc2int( pt, 'P' );
		pt = strtok( NULL, delim );	// read channel
		cr = pt[0] - 'A';
		sl = 15 - pt[1] + 'A';  //Normal crates
		if( cr == 3 || cr == 13 || cr > 16 ) {
	  		if( pt[1] == 'P' ) {
				sl = 15;
	    	} else {
	      		--sl;
	    	}
		}
		if( cr == 17 && sl == 15 ) {
			cr = 18;  //Special case of crate R, slot P
			sl = 0;
	  	}
	  	ch = 32 - (pt[2] - '0') * 10 - (pt[3] - '0');
		index = ( cr * 16 + sl ) * 32 + ch;

		pt = strtok( NULL, delim );	// read status
		pt += 2;
		
		checkTube(data,index,ipmt,"BUTTS");
		data->tube_coordinates[index].panel = 0;
		data->tube_coordinates[index].cell = 0;
		data->tube_coordinates[index].tube = ipmt;
		data->tube_coordinates[index].status = PMT_BUTTS;
//			( ( ( *pt - '0' ) * 0x10 + ( *(pt + 1 ) - '0' ) ) & 0xf8 );

		readTubeCoordinates(data,index); //PMT_OK flag set here
	}
	/* add neck tubes */
	for (l=0; l<neck; ++l) {
		fgets( buff, 256, fp );
		pt = strtok( buff, delim );	// read cable ID
		pt = strtok( NULL, delim );	// read tube ID
		ipmt = bc2int( pt, 'P' );
		pt = strtok( NULL, delim );	// read channel
		cr = pt[0] - 'A';
		sl = 15 - pt[1] + 'A';      //Normal crates
		if( cr == 3 || cr == 13 || cr > 16 ) {
	  		if( pt[1] == 'P' ) {
				sl = 15;
	    	} else {
	      		--sl;
	    	}
		}
		if( cr == 17 && sl == 15 ) {
			cr = 18;  //Special case of crate R, slot P
			sl = 0;
	  	}
	  	ch = 32 - (pt[2] - '0') * 10 - (pt[3] - '0');
		index = ( cr * 16 + sl ) * 32 + ch;

		pt = strtok( NULL, delim );	// read status
		pt += 2;
		checkTube(data,index,ipmt,"Neck");
		data->tube_coordinates[index].panel = 0;
		data->tube_coordinates[index].cell = 0;
		data->tube_coordinates[index].tube = ipmt;
		data->tube_coordinates[index].status = PMT_NECK;
//			( ( ( *pt - '0' ) * 0x10 + ( *(pt + 1 ) - '0' ) ) & 0xf8 );

		readTubeCoordinates(data,index); //PMT_OK flag set here
	}
	fclose(fp);
	Printf("Read %d standard, %d OWL, %d Low Gain, %d BUTTS and %d Neck PMT entries\n",
			npmt, nowl, nlg, nbut, neck );
	
	if (create_binary) {
		// write binary version of .dat file with extension .dat.bin
		fp = createAltFile(getOpenFileName(),"wb",".bin");
		if (fp) {
			fwrite(&database_header,sizeof(database_header),1,fp);
	  		fwrite(data->tube_coordinates,sizeof(Pmt),data->num_tube_coordinates,fp);
			fclose(fp);
			Printf("Created binary database file %s\n",getOpenFileName());
		}
	}
  }
//  Printf("PMT database date stamp: %ld\n",database_header.date);
	
#endif // PETERS_DATABASE

/*
	fp = fopen("database.txt","w");
	for (cr=0,n=0;cr<19;++cr) {
		for (sl=0;sl<16;++sl) {
			for (ch=0;ch<32;++ch) {
				fprintf(fp,"%2d %2d %2d (%6.1f %6.1f %6.1f) %2x %4d %4d %2d %2d\n",
						cr,sl,ch,
						data->tube_coordinates[n].x * data->tube_radius,
						data->tube_coordinates[n].y * data->tube_radius,
						data->tube_coordinates[n].z * data->tube_radius,
						data->tube_coordinates[n].status,
						data->tube_coordinates[n].tube,
						data->tube_coordinates[n].index,
						data->tube_coordinates[n].panel,
						data->tube_coordinates[n].cell);
				++n;
			}
		}
	}
	fclose(fp);
*/
}

//
// load NCD tube map
//
#ifdef SNOPLUS
void freeCaenData(ImageData *data)
{
    if (data->caen_size) {
        for (int i=0; i<kMaxCaenChannels; ++i) {
            delete data->caen_data[i];
            data->caen_data[i] = NULL;
        }
        data->caen_size = 0;
    }
}

#else

void loadNcdDatabase(ImageData *data, char *filename)
{
    int  i,j,n,line=0;
    int  err = 0;
    char *pt,buff[512];
    
	FILE *fp = openFile( filename, "r",data->file_path );
	// initialize all lookup elements to -1
	memset(data->ncdShaperLookup,-1,sizeof(data->ncdShaperLookup));
	memset(data->ncdMuxLookup,-1,sizeof(data->ncdMuxLookup));
	// clear NCD map
	memset(data->ncdMap,0,sizeof(data->ncdMap));
	if (fp) {
	    n = 0;
	    char *del = ", \t\n";
	    while (fgets(buff, 256, fp)) {
	        ++line;
	        pt = buff;
	        while (isspace(*pt)) ++pt;
	        if (*pt=='#' || *pt=='\0') continue;    // ignore blank/comment lines
	        pt = strtok(pt,del);            // string number
	        data->ncdMap[n].string_number = atoi(pt);
	        if (!(pt=strtok(NULL,del))) { err=1; break; }   // x pos
	        data->ncdMap[n].x = atof(pt);
	        if (!(pt=strtok(NULL,del))) { err=2; break; }   // y pos
	        data->ncdMap[n].y = atof(pt);
	        if (!(pt=strtok(NULL,del))) { err=3; break; }   // string name
	        strncpy(data->ncdMap[n].label, pt, kNCDLabelLen-1);
	        if (!(pt=strtok(NULL,del))) { err=5; break; }   // mux bus
	        data->ncdMap[n].mux_bus = atoi(pt);
	        if (data->ncdMap[n].mux_bus >= kNumMuxBuses) { err=6; break; }
	        if (!(pt=strtok(NULL,del))) { err=4; break; }   // mux box number 
	        data->ncdMap[n].mux_boxnum = atoi(pt);
	        if (!(pt=strtok(NULL,del))) { err=7; break; }   // mux channel
	        data->ncdMap[n].mux_channel = atoi(pt);
	        if (data->ncdMap[n].mux_channel >= kNumMuxChannels) { err=8; break; }
	        if (!(pt=strtok(NULL,del))) { err=9; break; }   // hv supply number
	        data->ncdMap[n].hv_supply = atoi(pt);
	        if (!(pt=strtok(NULL,del))) { err=10; break; }  // shaper slot
	        data->ncdMap[n].shaper_slot = atoi(pt);
	        if (data->ncdMap[n].shaper_slot >= kNumShaperSlots) { err=11; break; }
	        if (!(pt=strtok(NULL,del))) { err=12; break; }  // shaper hw address
	        if (sscanf(pt,"0x%x",&data->ncdMap[n].shaper_addr) != 1) { err=12; break; }
	        if (!(pt=strtok(NULL,del))) { err=12; break; }  // shaper channel
	        data->ncdMap[n].shaper_channel = atoi(pt);
	        if (data->ncdMap[n].shaper_channel >= kNumShaperChannels) { err=13; break; }
	        if (!(pt=strtok(NULL,del))) { err=14; break; }  // scope channel
	        data->ncdMap[n].scope_channel = atoi(pt);
	        if (!(pt=strtok(NULL,del))) { err=15; break; }  // preamp
	        strncpy(data->ncdMap[n].preamp, pt, kPreampLen-1);
	        if (!(pt=strtok(NULL,del))) { err=15; break; }  // PDS board
	        data->ncdMap[n].pds_board = atoi(pt);
	        if (!(pt=strtok(NULL,del))) { err=15; break; }  // PDS channel
	        data->ncdMap[n].pds_channel = atoi(pt);
	        if (!(pt=strtok(NULL,del))) { err=15; break; }
	        data->ncdMap[n].num_segments = atoi(pt);
	        if (data->ncdMap[n].num_segments<1 || data->ncdMap[n].num_segments>4) {
	            err = 16;
	            break;
	        }
	        for (i=0; i<(int)data->ncdMap[n].num_segments; ++i) {
	            if (!(pt=strtok(NULL,del))) { err=17; break; }
	            strncpy(data->ncdMap[n].segment_name[i], pt, kSegmentNameLen-1);
	        }
	        // fill in NCD nodes
	        float x = data->ncdMap[n].x / data->tube_radius;
	        float y = data->ncdMap[n].y / data->tube_radius;
	        data->ncdMap[n].nodes[0].x3 = data->ncdMap[n].nodes[1].x3 = x;
	        data->ncdMap[n].nodes[0].y3 = data->ncdMap[n].nodes[1].y3 = y;
	        float v = AV_INNER_RADIUS / data->tube_radius;
	        float z = -sqrt(v*v-x*x-y*y);
	        data->ncdMap[n].nodes[0].z3 = z + NCD_BOTTOM_OFFSET / data->tube_radius;
	        data->ncdMap[n].nodes[1].z3 = -data->ncdMap[n].nodes[0].z3;
	        
	        if (n >= (int)kMaxNCDs) { err=18; break; }
	        
	        // fill in shaper/mux lookups
	        if (data->ncdShaperLookup[data->ncdMap[n].shaper_slot][data->ncdMap[n].shaper_channel] < 0) {
	            data->ncdShaperLookup[data->ncdMap[n].shaper_slot][data->ncdMap[n].shaper_channel] = n;
	        } else {
	            err = 19;
	        }
	        if (data->ncdMuxLookup[data->ncdMap[n].mux_bus][data->ncdMap[n].mux_channel] < 0) {
	            data->ncdMuxLookup[data->ncdMap[n].mux_bus][data->ncdMap[n].mux_channel] = n;
	        } else {
	            err = 20;
	        }
	        ++n;
	    }
	    if (!err) {
	        data->numNcds = n;
	        // point all undefined mux/shaper channels to the last ncd entry
	        for (i=0; i<(int)kNumMuxBuses; ++i) {
	            for (j=0; j<(int)kNumMuxChannels; ++j) {
	                if (data->ncdMuxLookup[i][j] < 0) {
	                    data->ncdMuxLookup[i][j] = n;
	                }
	            }
	        }
	        for (i=0; i<(int)kNumShaperSlots; ++i) {
	            for (j=0; j<(int)kNumShaperChannels; ++j) {
	                if (data->ncdShaperLookup[i][j] < 0) {
	                    data->ncdShaperLookup[i][j] = n;
	                }
	            }
	        }
	    }
	} else {
	    err = -1;
	}
	if (err) {
	    Printf("Error %d in line %d of %s\n", err, line, filename);
	} else {
	    Printf("Loaded NCD database %s\n",getOpenFileName());
	}
}

void freeNcdScopeData(ImageData *data)
{
    if (data->ncd_scope_size) {
        for (int i=0; i<kMaxNcdScopeData; ++i) {
            delete data->ncd_scope_data[i];
            data->ncd_scope_data[i] = NULL;
        }
        data->ncd_scope_size = 0;
        // reset our log amp parameters
        memset(data->ncd_log_amp, 0, sizeof(data->ncd_log_amp));
    }
}
#endif // SNOPLUS

/* free all memory allocated in ImageData structure */
/* (except the main window object and the ImageData itself) */
void freeData(ImageData *data)
{
	int		i;
	// delete all our windows
	for (i=0; i<NUM_WINDOWS; ++i) {
		if (data->mWindow[i]) {
			delete data->mWindow[i];
		}
	}
	
	for (i=0; i<NUM_WRITERS; ++i) {
		delete data->mZdabWriter[i];
		data->mZdabWriter[i] = NULL;
	}
	
	freeDatabaseList(data);
	
#ifdef SNOPLUS
    freeCaenData(data);
#else
    freeNcdScopeData(data);
#endif

	// clear the displayed hits and reconstructions
	clearEvent(data);
	
	// forget the event history
	clearHistory(data);
	
	// forget the 'all' and 'future' histories
	clearHistoryAll(data);
	
	// close any open files or dispatcher connections
	close_event_file(data);
	
	if (data->cmos_rates) {
		free(data->cmos_rates);
		data->cmos_rates = 0;
	}
	if (data->sum) {
		data->sum = 0;
		free(data->sum_tac);
		free(data->sum_qhs);
		free(data->sum_qhl);
		free(data->sum_qlx);
	}
	XtFree((char *)data->tube_coordinates);	data->tube_coordinates = NULL;
	XtFree((char *)data->remap_data);		data->remap_data = NULL;
	XtFree((char *)data->remap_done);		data->remap_done = NULL;
	XtFree((char *)data->sum_nhit);			data->sum_nhit = NULL;
	XtFree(data->projName);
	XtFree(data->dispName);
	
	if (data->ascii_label_allocated) {
		data->ascii_label_allocated = 0;
		XtFree(data->ascii_label);
		data->ascii_label = NULL;
	}
	if (data->ascii_file_allocated) {
		data->ascii_file_allocated = 0;
		XtFree(data->ascii_file);
		data->ascii_file = NULL;
	}
	
	freePoly(&data->geod);
	
#ifdef ROOT_FILE
	delete data->root_cal_event;
	data->root_cal_event = NULL;
	
	if (data->owns_rch_file) {
		delete data->root_rch_file;
		data->owns_rch_file = 0;
		data->root_rch_file = NULL;
	}
#endif

#ifdef LOAD_CALIBRATION
#ifdef ROOT_FILE
	delete data->root_cal;
	data->root_cal = NULL;
	delete data->root_cal2;
	data->root_cal2 = NULL;
	delete data->root_water;
	data->root_water = NULL;
	data->waterLevel = 0;
#else // ROOT_FILE
	delete data->pcal_simple;
	data->pcal_simple = NULL;
#ifdef TITLES_CAL
	delete data->pcal_titles;
	data->pcal_titles = NULL;
#endif // TITLES_CAL
	data->pcal = NULL;
#endif // ROOT_FILE
#ifdef FITTR
	if (data->pfit) {
		free(data->pfit);
		data->pfit = NULL;
	}
	data->init_cal = 0;
#endif // FITTR
#endif // LOAD_CALIBRATION

#ifdef OPTICAL_CAL
	delete data->oca;
	data->oca = NULL;
#endif

#ifdef ROOT_SYSTEM
	root_free_data(data);
#endif

#if XSNOMAN
	xsnoman_delete();
#endif
	delete data->mSpeaker;
	data->mSpeaker = NULL;
	
	delete data->mZdabFile;
	data->mZdabFile = NULL;
}


/* turn on or off the water level display */
void toggleWaterLevelDisplay(ImageData *data)
{
	data->waterLevel ^= 1;
	if (data->waterLevel) {
#ifdef ROOT_FILE
		if (!data->root_water) {
			data->root_water = new QWater;
		}
#endif
		if (data->mMainWindow) {
			data->mMainWindow->DoWater(1);
		}
	} else {
#ifdef ROOT_FILE
		if (data->root_water) {
			delete data->root_water;
			data->root_water = NULL;
		}
#endif
	}
	sendMessage(data, kMessageWaterChanged);
}

int calibrated_access_begin(ImageData *data)
{
	int old_wCalibrated = data->wCalibrated;	/* save calibrated flag */
	
	/* use the simple calibration by default if no pre-calibrated data available */
	if (old_wCalibrated==IDM_PRECALIBRATED && !data->calHits) {
		data->wCalibrated = IDM_CAL_SIMPLE;
	}

	initializeCalibrations(data);	// make sure calibrations are initialized
	
	return(old_wCalibrated);		// return old flag
}

void calibrated_access_end(ImageData *data, int old_wCalibrated)
{
	data->wCalibrated = old_wCalibrated;	// restore original calibrated flag
}

void doFit(ImageData *data, int update_displays)
{
#ifdef FITTR
	int j, jtubes;
	Vertex* vertex;
	HitInfo *hi;
	Fit *fit;
	float oldCal;
	int step = 1;
	
	int	old_wCalibrated = calibrated_access_begin(data);
	
	if (!data->pfit) {
		if (update_displays) Printf("Fitter unavailable\n");
		calibrated_access_end(data, old_wCalibrated);
		return;
	}
	jtubes = data->hits.num_nodes;
	// fit a maximum of 400 tubes
	while (jtubes/step > 400) {
		++step;
	}
	jtubes /= step;
	
	if( !( vertex = (Vertex *)malloc( jtubes * sizeof(Vertex) ) ) ) {
		Printf("Cannot allocate %d bytes for fitter\n", jtubes * sizeof(Vertex) );
		quit("Fatal error");
	}

	hi = data->hits.hit_info;
	for( j=0; j<jtubes; ++j, hi+=step ) {
		float r = data->tube_coordinates[hi->index].r;
		int index = j * step;
		vertex[j].x = data->hits.nodes[index].x3 * r;
		vertex[j].y = data->hits.nodes[index].y3 * r;
		vertex[j].z = data->hits.nodes[index].z3 * r;
		// ignore non-normal and discarded tubes
		if( (hi->flags & (HIT_PMT_MASK | HIT_DISCARDED)) == HIT_NORMAL ) {
			vertex[j].stat = ( (unsigned int)hi->flags << 16 ) | hi->index;
			oldCal = hi->calibrated;
			if (!setCalibratedTac( data, hi, index )) {
				vertex[j].t = hi->calibrated;
				hi->calibrated = oldCal;
				continue;
			}
			hi->calibrated = oldCal;
		}
		vertex[j].stat |= 0x8000;
		vertex[j].t= 0.;
	}
	/* can now return wCalibrated (since setCalibrated...() calls are finished) */
	calibrated_access_end(data, old_wCalibrated);

	if( jtubes > 3 ) {
		fit = fitter( data->pfit, jtubes, vertex );

		/* this needs some work */

		if (fit && fit->nhitw > 0) {
			if (update_displays) {
/*				Printf("x,y,z,t = %f,%f,%f,%f   u,v,w = %f,%f,%f \nchi^2 = %f  nhit = %d nhitw = %d\n",
						fit->vertex.x, fit->vertex.y, fit->vertex.z, fit->vertex.t,
						fit->dirvec.u, fit->dirvec.v, fit->dirvec.w,
						fit->chi, fit->nhit, fit->nhitw );
*/			
#ifdef PETER_S
				Printf("jpmt,jevnt,jstart,istat = %d,%d,%d,%d\n",
						fit->jpmt, fit->jevnt, fit->jstart, fit->istat );
#endif // PETER_S
			}

			for (j=0; j<data->nrcon; ++j) {
				if (!strcmp(data->rcon[j].name, ONLINE_FITTER_NAME)) break;
			}
			if (j >= MAX_RCON) {
				if (update_displays) Printf("Maximum number of fits exceeded\n");
			} else {
				if (data->nrcon <= j) data->nrcon = j + 1;
				data->curcon = j;
				data->rcon[j].pos[0] = fit->vertex.x / data->tube_radius;
				data->rcon[j].pos[1] = fit->vertex.y / data->tube_radius;
				data->rcon[j].pos[2] = fit->vertex.z / data->tube_radius;
				data->rcon[j].time = fit->vertex.t;
				data->rcon[j].dir[0] = fit->dirvec.u;
				data->rcon[j].dir[1] = fit->dirvec.v;
				data->rcon[j].dir[2] = fit->dirvec.w;
				data->rcon[j].fit_pmts = fit->nhit;
				data->rcon[j].chi_squared = fit->chi;
				data->rcon[j].cone_angle = CONE_ANGLE;
				data->fit_nhitw[j] = fit->nhitw;
				// set the fitter name (so we can recognize our fit later)
				strcpy(data->rcon[j].name, ONLINE_FITTER_NAME);
				// calculate the chernkov cone intersection
				setRconNodes( data->rcon + j );
				
				if (update_displays) {
					// must re-calculate relative hit times whenever rcon changes
					// (only necessary if we are currently displaying time differences)
					if (data->wDataType == IDM_DELTA_T) {
						calcCalibratedVals(data);
					}
					if (data->auto_vertex) {
						// move to the new fit vertex
						sendMessage(data, kMessageSetToVertex);
						data->mMainWindow->SetDirty();
					} else {
						// transform the rcon nodes to the current projection
						data->mMainWindow->GetImage()->Transform(data->rcon[j].nodes, data->rcon[j].num_nodes);
					}
					// update the window title (must do this when curcon changes)
					newTitle(data);
					sendMessage(data,kMessageFitChanged);
				} else {
					sendMessage(data, kMessageFitChanged);
				}
			}
		} else if (update_displays) {
			Printf("Can't fit event %ld\n",(long)data->event_id);
		}
		if (fit) free( fit );
	}
	free( vertex );
#endif // FITTR
}


/* load NHIT=1 into all tubes */
void loadUniformHits(ImageData *data)
{
	int		i, n, cr, sl, ch, t;
	HitInfo	*hit_info;
	
	clearEvent(data);
	
	data->sum_nhit_count = NUM_TOTAL_CHANNELS;
	data->total_sum_nhit = NUM_TOTAL_CHANNELS;
	data->sum_event_count = 1;
	data->sum_changed = 0;	/* reset changed flag since we are displaying sum now */
	setEventTime(data,double_time());	/* set event time */
	data->mMainWindow->DoWater(0);
	data->run_number = 0;
	data->sub_run = -1;
	data->event_id = 0;
	memset(data->mtc_word, 0, 6*sizeof(u_int32));
	data->trig_word = 0;
	
	/* fill in hit information */
	data->event_counter++;
	data->hits.num_nodes = data->event_nhit = data->sum_nhit_count;
	data->hits.nodes     = (Node *)   XtMalloc(data->hits.num_nodes*sizeof(Node));
	data->hits.hit_info  = (HitInfo *)XtMalloc(data->hits.num_nodes*sizeof(HitInfo));
	
	if (!data->hits.nodes || !data->hits.hit_info) quit("Out of memory");
	
	hit_info = data->hits.hit_info;
	for (i=0,n=0; i<data->hits.num_nodes; ++i,++n) {
		verify_tube_coordinates(data,n);
		data->sum_nhit[n] = 1;
		cr = n / (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
		t = n - cr * (NUM_CRATE_CARDS * NUM_CARD_CHANNELS);
		sl = t / NUM_CARD_CHANNELS;
		ch = t - sl * NUM_CARD_CHANNELS;
		hit_info->index = n;
		hit_info->gt = 0;
		hit_info->tac = 1500;
		hit_info->qhs = 750;
		hit_info->qhl = 750;
		hit_info->qlx = 750;
		hit_info->nhit = 1;
		hit_info->crate = cr;
		hit_info->card = sl;
		hit_info->channel = ch;
		hit_info->cell = 0;
		hit_info->flags = data->tube_coordinates[n].status & HIT_PMT_MASK;
		
		data->hits.nodes[i].x3 = data->tube_coordinates[n].x;
		data->hits.nodes[i].y3 = data->tube_coordinates[n].y;
		data->hits.nodes[i].z3 = data->tube_coordinates[n].z;
		++hit_info;
	}
	data->numNcdHit = 0;
	data->general_count = 0;
	for (i=0; i<data->numNcds; ++i) {
	    ++data->numNcdHit;
	    data->ncdHit[i].mux_count = 1;
	    data->ncdHit[i].shaper_count = 1;
	    data->ncdHit[i].scope_count = 1;
	    data->ncdHit[i].shaper_value = 100;
	    data->ncdHit[i].flags = HIT_SHAPER | HIT_MUX | HIT_SCOPE;
	}
	memset(data->ncdHit+data->numNcds, 0, sizeof(NCDHit));
	
	data->event_shown = 1;
	newTitle(data);				/* update main window title */
	
	/* calculate the calibrated values if necessary */
	calcCalibratedVals(data);
	
	/* calculate the hit colour indices */
	calcHitVals(data);
/*
** update the displays
*/
	sendMessage(data, kMessageNewEvent);
}
