#ifndef __xsnoed_h__
#define __xsnoed_h__

#include "ImageData.h"

#ifndef PI
#define	PI				3.14159265358979324
#endif

// uncomment the following line to engage the AV anchor rope tension calculation code
//#define AV_ANCHOR

#ifdef AV_ANCHOR
#define MAX_EDGES		3000
#else
#define MAX_EDGES		350
#endif

#define NECK_RADIUS		75.0			/* cm radius of vessel neck */
#define NECK_BASE		595.2942		/* radius of base of neck */
#define OCA_BUFFER_NUM	7				/* Root window property buffer number for OCA data */

#define NUM_SNO_CRATES		20			/* number of SNO crates */
#define NUM_CRATE_CARDS		16			/* number of FEC cards per SNO crate */
#define	NUM_CARD_CHANNELS	32			/* number of channels per FEC card */
#define NUM_CHANNEL_CELLS	16			/* number of cells per channel */
#define NUM_CRATE_CHANNELS	(NUM_CRATE_CARDS * NUM_CARD_CHANNELS)
#define NUM_TOTAL_CHANNELS	(NUM_SNO_CRATES * NUM_CRATE_CHANNELS)

#define TRIGGER_ORPHAN	0x00080000UL	// synthetic "ORPHAN" trigger bit mask
#define TRIGGER_NONE	0x00100000UL	// synthetic "NONE" trigger bit mask
#define TRIGGER_CAEN    0x00200000UL    // synthetic "CAEN" trigger bit mask (SNO+)
#define TRIGGER_TUBII   0x00400000UL    // synthetic "TUBII" trigger bit mask (SNO+)
#define TRIGGER_NCD     0x00012000UL    // NCD shaper + NCD MUX trigger bits

typedef void (* XSnoedCallbackPtr)(void);

/*------------------------------------------------------------------------------------------*/

struct Projection;

#ifdef  __cplusplus
extern "C" {
#endif

/*
** Functions ....
*/

void    attachHistoryEntry(HistoryEntry *event_buff);
void	calcHitVals(ImageData *data);
void	calcScreen(ImageData *data);
void	calcSunDirection(ImageData *data);
void	calcCalibratedVals(ImageData *data);
int		calibrated_access_begin(ImageData *data);
void	calibrated_access_end(ImageData *data, int old_wCalibrated);
int 	checkEvent(ImageData *data, aPmtEventRecord *pmtRecord);
void	clearEvent(ImageData *data);
void	clearHistory(ImageData *data);
void	clearHistoryAll(ImageData *data);
void	clearSum(ImageData *data);
void	doFit(ImageData *data, int update_displays);
void	freeData(ImageData *data);
void    freeDatabaseList(ImageData *data);
void    freeNcdScopeData(ImageData *data);
void    freeCaenData(ImageData *data);
float	getHitVal(ImageData *data, HitInfo *hi);
float	getHitValPad(ImageData *data, HitInfo *hi);
float   getNcdHitVal(ImageData *data, NCDHit *nh);
void	handleHeaderRecord(ImageData *data, u_int32 *bank_data, u_int32 bank_name);
void	hitSphere(Vector3 pos,Vector3 dir,Node *out,float radius);
void	initData(ImageData *data, int load_settings);
void	initializeCalibrations(ImageData *data);
void    loadDatabaseList(ImageData *data);
void    loadDatabases(ImageData *data, u_int32 run_num);
void    loadNcdDatabase(ImageData *data, char *filename);
void    loadPmtDatabase(ImageData *data, char *filename);
void	loadUniformHits(ImageData *data);
void	monteCarloSetFlags(ImageData *data, int flags, int set);
void	monteCarloProcess(ImageData *data);
void	newTitle(ImageData *data);
void    releaseHistoryEntry(HistoryEntry *event_buff);
HistoryEntry *	removeFutureEvent(ImageData *data);
void	sendMessage(ImageData *data, int message, void *dataPt=NULL);
int		setCalibratedExtra(ImageData *data, HitInfo *hi, int index, int dataType);
int		setCalibratedNHIT(ImageData *data, HitInfo *hi, int index);
int 	setCalibratedQhl(ImageData *data, HitInfo *hi, int index);
int 	setCalibratedQhs(ImageData *data, HitInfo *hi, int index);
int 	setCalibratedQlx(ImageData *data, HitInfo *hi, int index);
int 	setCalibratedTac(ImageData *data, HitInfo *hi, int index);
void	setLabel(ImageData *data, int on);
void	setRconNodes(RconEvent *event);
void	setRconNodesVessel(RconEvent *event, float sphere_radius, float z_cutoff, float cyl_radius);
void	setTriggerFlag(ImageData *data, int theFlag, int end_of_data=0);
int		showHistory(ImageData *data, int incr);
void	toggleWaterLevelDisplay(ImageData *data);
void	usleep_ph(unsigned long usec);

#ifdef LOAD_CALIBRATION
void	setCalibration(ImageData *data,int calType);
#endif

#ifdef OPTICAL_CAL
OCA *	calcOCA(ImageData *data, int source_moved);
void	deleteOCA(ImageData *data);
void	getOpticalParameters(ImageData *data);
#endif

/* xsnoed library interface routines */
ImageData *xsnoed_create(int load_settings);
void	xsnoed_delete(ImageData *data);
int		xsnoed_running(ImageData *data);
int 	xsnoed_filter(ImageData *data, aPmtEventRecord *pmtRecord);
int 	xsnoed_event(ImageData *data, aPmtEventRecord *pmtRecord);
int		xsnoed_replace(ImageData *data, aPmtEventRecord *pmtRecord);
int		xsnoed_event2(ImageData *data, EventInfo *evt, RconEvent *rcon, int nrcon);
void	xsnoed_add_rcons(ImageData *data, RconEvent *rcon, int nrcon, int update_displays);
void	xsnoed_clear(ImageData *data);
void	xsnoed_service(int other_app_events_too);
void	xsnoed_service_all();
void	xsnoed_rates(ImageData *data, int crate, int32 *crate_rates);
void	xsnoed_next(ImageData *data, int step);
void 	xsnoed_set_callback(ImageData *data, XSnoedCallbackPtr);
void	xsnoed_save_event(ImageData *data, char *file_name, unsigned nwriter);
PmtEventRecord *xsnoed_get_event(ImageData *data);

void 	addToHistory(ImageData *data, aPmtEventRecord *pmtRecord, int whichBuff);

#ifdef  __cplusplus
}
#endif

#endif

