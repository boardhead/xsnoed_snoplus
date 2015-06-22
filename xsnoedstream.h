#ifndef _XSNOEDSTREAM_H_
#define _XSNOEDSTREAM_H_

#include "xsnoed.h"
#include "include/Record_Info.h"

enum {
	PROCESS_IDLE 		= 0,			// data processing is idle
	PROCESS_TIME_OUT 	= 0x01,			// data processing waiting for timeout callback
	PROCESS_WORK_PROC	= 0x02,			// data processing in work proc
	PROCESS_THROW_OUT	= 0x04			// callback to stop throwing out data from dispatcher
};

#ifdef  __cplusplus
extern "C" {
#endif

void decode_status ( void );
int decode_rawdata( ImageData *data );
void decode_cmos( ImageData *data );
void decode_rechdr( ImageData *data );
void decode_scopedat( ImageData *data );
void InitDispLink(ImageData *data,int keep_trying);

void CloseDispLink(void);
int IsDispConnected(ImageData *data);
void SetDispName(char *name);
char *GetDispName(void);
void HandleEvents(ImageData *data);
void ActivateTrigger(ImageData *data);

#ifdef LESSTIF
void Kick(ImageData *data);
#endif

void open_event_file(ImageData *data, int try_dispatch);
void close_event_file(ImageData *data);
void rewind_event_file(ImageData *data);
aPmtEventRecord *get_next_event(ImageData *data, int use_future=1);
void load_event(ImageData *data, long target, int loadBy=kGotoGTID);
int put_next_event_into_future(ImageData *data);

#ifdef ROOT_FILE
void 			do_root(ImageData *data);
#ifdef __cplusplus
class QEvent;
class QTree;
class QEvent;
class QFit;
class TFile;
class TEventList;
void 			 root_open_tree(ImageData *data,QTree *aTree);
void			 root_open_event_list(ImageData *data,TEventList *aList, QTree *aTree);
aPmtEventRecord *root_load_event(int anEvent);
aPmtEventRecord *root_load_qevent(QEvent *anEvent);
void 			 root_set_rcon(RconEvent *rcon, QFit *aFit);
TFile 			*root_get_file(void);
QTree 			*root_get_tree(void);
TEventList 		*root_get_event_list(void);
QEvent			*root_get_event(ImageData *data, QEvent *anEvent);
void			root_update_rch_windows(ImageData *data, int hit_num);
void			root_free_data(ImageData *data);
#endif
#endif

#ifdef  __cplusplus
}
#endif

#endif
